
#ifndef MYDB_PAGE_H
#define MYDB_PAGE_H


#include "MyDB_Table.h"
#include <string>
#include <memory>


using namespace std;
class MyDB_Page;
typedef shared_ptr <MyDB_Page> MyDB_PagePtr;

class MyDB_BufferManager;

class MyDB_Page {
public:
    // manage data
    void *getBytes();
    void *wroteBytes();
    void *setData(void *bytes, size_t bytes_num);
    
    // manage reference num
    void increaseRefNum();
    void decreaseRefNum();
    
    MyDB_Page(MyDB_TablePtr table_ptr, size_t pos, MyDB_BufferManager &bf_mgr);
    ~MyDB_Page();
    
private:
    friend class MyDB_BufferManager;
    
    int ref_num;
    void *bytes;
    bool is_dirty;
    MyDB_BufferManager& bf_mgr;
    MyDB_TablePtr table_ptr;
    size_t pos;
    
};


#endif