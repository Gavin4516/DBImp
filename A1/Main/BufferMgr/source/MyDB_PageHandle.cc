
#ifndef PAGE_HANDLE_C
#define PAGE_HANDLE_C

#include <memory>
#include "MyDB_PageHandle.h"

void *MyDB_PageHandleBase :: getBytes () {
	return page_ptr->getBytes ();
}

void MyDB_PageHandleBase :: wroteBytes () {
    page_ptr->wroteBytes ();
}

MyDB_PageHandleBase :: MyDB_PageHandleBase (MyDB_PagePtr page_ptr) {
    this->page_ptr = page_ptr;
    this->page_ptr->increaseRefNum ();
}

MyDB_PageHandleBase :: ~MyDB_PageHandleBase () {
    page_ptr->decreaseRefNum ();
}

#endif

