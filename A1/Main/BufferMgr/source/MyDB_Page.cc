
#ifndef PAGE_C
#define PAGE_C

#include <memory>
#include "MyDB_Page.h"
#include "MyDB_Table.h"
#include "MyDB_BufferManager.h"


void *MyDB_Page :: getBytes () {
	
}

void MyDB_Page :: wroteBytes () {
    is_dirty = true;
}

void MyDB_Page :: increaseRefNum () {
    ref_num++;
}

void MyDB_Page :: decreaseRefNum () {
    ref_num--;
    
}


MyDB_Page :: MyDB_Page (MyDB_TablePtr table_ptr, size_t pos, MyDB_BufferManager &bf_mgr) {
    this->table_ptr = table_ptr;
    this->pos = pos;
    this->bf_mgr = bf_mgr;
    bytes = nullptr;
    is_dirty = false;
    ref_num = 0;
}

MyDB_Page :: ~MyDB_Page () {
}

#endif

