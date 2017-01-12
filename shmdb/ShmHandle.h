/*
        共享内存操作类的定义
*/
#ifndef SHMHANDLE_H
#define SHMHANDLE_H
#include"ShmStruct.h"
#include "DiskFileHandle.h"
#include"PageHandle.h"
#define PID_MAX 100

class ShmHandle
{
public:
    ShmHandle();
    virtual ~ShmHandle();

    bool shmCreat(char* diskfilename, int shmsize,int shmKey );//加入参数int shmKey,去除共享内存指针，创建函数只需要返回内存快标识ID
    bool shmAttach(void*& ptr, const int shmKey); //链接，ptr保存shmat返回值
    bool shmControl(void *ptr,int shmsize, int pagesize );//完成初始化
    bool shmDelAttach(void*& ptr); //断开链接
    bool shmDestroy( const int shmfileid); //删共享内存
    bool showShmInfo(void *const ptr);
    int numsCount(void *const ptr);
    bool showPageInfo(void *const ptr,int pagenum,int pagesize );//展示页头信息

    void *ptr; //记录共享内存首地址
    void *shmAddr;
    int shmID; //记录共享内存ID
    SHMInfo shmInfo; //记录共享内存块信息的结构体，同时保存为内存块的第一页
   // PageHandle pageUtil;
};

#endif // SHMHANDLE_H
