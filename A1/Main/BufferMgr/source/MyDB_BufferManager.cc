
#ifndef BUFFER_MGR_C
#define BUFFER_MGR_C

#include "MyDB_BufferManager.h"
#include <string>
#include <fcntl.h>
#include <iostream>
#include "MyDB_Page.h"
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <utility>

using namespace std;

//find if the pair exists in the hashtable and return the pageHandle of that page
MyDB_PageHandle MyDB_BufferManager :: getPage (MyDB_TablePtr indexTable, long pageNum) {
	if (openFile.find(indexTable) != openFile.end()) {
		int status = open (indexPage->getStorageLoc ().c_str (), O_CREAT | O_RDWR, 0666);
		openFile[indexTable] = status;
	}
	pair <MyDB_TablePtr, long> indexPage = make_pair (indexTable, pageNum);	
	if (pageMap.find(indexPage) == pageMap.end()) {
		MyDB_PagePtr pagePtr = make_shared <MyDB_Page> (indexTable, pageNum, *this);
		pageMap[indexPage] = pagePtr;
		return make_shared <MyDB_PageHandleBase> (pagePtr);
	}
	return make_shared <MyDB_PageHandleBase> (pageMap[indexPage]);
}
//find anonymous page
MyDB_PageHandle MyDB_BufferManager :: getPage () {
	if (openFile.find(nullptr) != openFile.end()) {
		int status = open (targetFile.c_str (), O_CREAT | O_RDWR, 0666);
		openFile[nullptr] = status;
	}
	size_t pos = availablePositions.top ();
	availablePositions.pop ();
	if (availablePositions.size () == 0) {
		availablePositions.push (pos + 1);
	}
	MyDB_PagePtr pagePtr = make_shared <MyDB_Page> (nullptr, pos, *this);
	pair <MyDB_TablePtr, long> indexPage = make_pair (nullptr, pos);
	pageMap [indexPage] = pagePtr;
	return make_shared <MyDB_PageHandleBase> (pagePtr);	
}

//pop up a pageHandle from the beginning of the list
void MyDB_BufferManager :: popPage () {
	auto t = handleList.begin();
	auto pageHandle = *t;
	if (pageHandle -> page -> isDirty) {
		lseek (openFile[pageHandle -> page -> myTable], pageHandle -> page -> pos * pageSize, SEEK_SET);
		write (openFile[pageHandle -> page -> myTable], pageHandle -> page -> bytes, pageSize);
        fsync(openFile[pageHandle -> page -> myTable]);
		pageHandle -> page -> isDirty = false;
	}
	handleMap.erase(pageHandle);
	handleList.erase(t);
	availableRam.push_back (pageHandle -> page -> bytes);
	pageHandle -> page -> bytes = nullptr;
}

void MyDB_BufferManager :: erasePage (MyDB_Page& targetPage) {
	pair <MyDB_TablePtr, long> indexPage = make_pair (targetPage.myTable, targetPage.pos);
	if (pageMap.find (indexPage) != pageMap.end()) {
		auto pagePtr = pageMap[indexPage];
		MyDB_PageHandle pageHandle = make_shared <MyDB_PageHandleBase> (pagePtr);
		if (targetPage.bytes != nullptr && handleMap.find (pageHandle) == handleMap.end()) {
			unpin (pageHandle);	
			return;
		}
		//erase from LRU
		if (handleMap.find (pageHandle) != handleMap.end()) {
			auto it = handleMap.find (pageHandle);
			auto page = *it;
		    handleMap.erase (page);
		    handleList.erase(it);
		}
		auto page = pageMap.find (indexPage);
		pageMap.erase (page); 
	}
}

void MyDB_BufferManager :: visitPage (MyDB_Page &targetPage) {
	pair <MyDB_TablePtr, long> indexPage = make_pair (targetPage.myTable, targetPage.pos);
	auto pagePtr = pageMap [indexPage];

	MyDB_PageHandle pageHandle = make_shared <MyDB_PageHandleBase> (pagePtr);
	if (handleMap.find (pageHandle) == handleMap.end()) {
		handleList.erase(handleMap[pageHandle]);
		handleList.insert(handleList.begin(), pageHandle);
		handleMap[pageHandle] = handleList.begin();
	} else if (pagePtr -> bytes == nullptr) {	
		if (availableRam.size () == 0)
			kickOutPage ();
		if (availableRam.size () == 0) {
			cout << "There is no RAM available\n";
			exit (1);
		}
		pagePtr -> bytes = availableRam.back(); 
		pagePtr -> numBytes = pageSize;
		availableRam.pop_back ();
		lseek (openFile[pagePtr -> myTable], pagePtr -> pos * pageSize, SEEK_SET);
		read (openFile[pagePtr -> myTable], pagePtr -> bytes, pageSize);
		handleMap.insert (pageHandle);
	}
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage (MyDB_TablePtr indexTable, long pageNum) {
	if (openFile.find(indexTable) != openFile.end()) {
		int status = open (indexPage->getStorageLoc ().c_str (), O_CREAT | O_RDWR, 0666);
		openFile[indexTable] = status;
	}	
	pair <MyDB_TablePtr, long> indexPage = make_pair (indexTable, pageNum);
	if (pageMap.find (indexPage) == pageMap.end()) {
		if (availableRam.size () == 0)
			kickOutPage ();
		if (availableRam.size () == 0) 
			return nullptr;

		MyDB_PagePtr pagePtr = make_shared <MyDB_Page> (indexTable, pageNum, *this);
		pagePtr -> bytes = availableRam.back();
		pagePtr -> numBytes = pageSize;
		availableRam.pop_back ();
		pageMap [indexPage] = pagePtr;
		return make_shared <MyDB_PageHandleBase> (pagePtr);
	}

	return make_shared <MyDB_PageHandleBase> (allPages [whichPage]);
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage () {
	if (availableRam.size () == 0)
		kickOutPage ();
	if (availableRam.size () == 0) 
		return nullptr;

	MyDB_PageHandle pageHandle = getPage ();
	pageHandle -> page -> bytes = availableRam.back();
	pageHandle -> page -> numBytes = pageSize;
	availableRam.pop_back ();

	return returnVal;	
}

void MyDB_BufferManager :: unpin (MyDB_PageHandle targetHandle) {
	if (handleMap.find (targetHandle) != handleMap.end()) {
		handleMap.erase (targetHandle);
		handleList.insert(handleList.begin(), targetHandle);
		handleMap[targetHandle] = handleList.begin();
	}
}

MyDB_BufferManager :: MyDB_BufferManager (size_t size, size_t numPages, string file) {
	pageSize =size;
	targetFile = filr;
	// create all of the RAM
	for (size_t i = 0; i < numPages; i++) {
		availableRam.push_back (malloc (pageSizeIn));
	}	
	availablePositions.push (0);
}

MyDB_BufferManager :: ~MyDB_BufferManager () {
    unordered_map < pair <MyDB_TablePtr, size_t>, MyDB_PagePtr> emptyPageMap;
	pageMap = emptyMap;
	unordered_map <MyDB_PageHandle, list<MyDB_PageHandle> :: iterator> emptyHandleMap;
	handleMap = emptyHandleMap;
	list <MyDB_PageHandle> emptyHandleList;
	handleList = emptyHandleMap;
	for (auto ram : availableRam) {
		free (ram);
	}
	for (auto f : openFile) {
		close (f.second);
	}
	unlink (tempFile.c_str ());
}
	
#endif


