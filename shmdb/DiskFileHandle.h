/*
读写磁盘文件类
*/
#ifndef DISKFILEHANDLE_H
#define DISKFILEHANDLE_H
#include "ShmStruct.h"
#include <iostream>
#include <string>

using namespace std;

class DiskFileHandle
{
public:
    DiskFileHandle();
    virtual ~DiskFileHandle();

    bool diskFileRead(void *shmptr, int shmsize,  const char *filename);        //读文件写入共享内存
    bool diskFileReadPage(void *shmptr, int pagenum, int pagesize, const char *filename);       //读对应页写入共享内存
    bool diskFileWrite(const char *filename, const void *shmptr, int shmsize);      //读内存块写入文件
    bool diskFileWritePage( const char *filename,const void *shmptr, int pagenum, int pagesize);        //读内存页写入文件
};

#endif // DISKFILEHANDLE_H
