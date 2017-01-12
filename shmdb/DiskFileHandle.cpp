#include "DiskFileHandle.h"
#include<stdio.h>//C file

//构造函数
DiskFileHandle::DiskFileHandle()
{
}

//析构函数
DiskFileHandle::~DiskFileHandle()
{
}

//整块读
bool DiskFileHandle::diskFileRead(void *shmptr, int shmsize, const char * filename)
{
    FILE *fp;
    fp=fopen(filename,"rb");
    if(NULL==fp)
    {
        cout<<"INFO:file doesnot exist!"<<endl;
        return false;
    }
    fread(shmptr,shmsize,1,fp);
    fclose(fp);
    return true;
}

//整块写
bool DiskFileHandle::diskFileWrite(const char *filename, const void *shmptr, int shmsize)
{
    FILE *fp;
    fp=fopen(filename,"wb");
    if(NULL==fp)
    {
        cout<<"INFO:file open failed!"<<endl;
        return false;
    }
    fwrite(shmptr,shmsize,1,fp);
    fclose(fp);
    return true;
}

//按页读
bool DiskFileHandle::diskFileReadPage(void *shmptr, int pagenum, int pagesize,const char *filename)
{
    FILE *fp;
    fp=fopen(filename,"rb+");
    if(NULL==fp)
    {
        cout<<"*****INFO:文件不存在*****"<<endl;
        return false;
    }

    fseek(fp,pagenum*pagesize,0);

    fread((shmptr+(pagenum*pagesize)),pagesize,1,fp);
    fclose(fp);
    return true;
}

//按页写  文件名，内存首地址，页数，页大小
bool DiskFileHandle::diskFileWritePage( const char *filename,const void *shmptr, int pagenum, int pagesize)
{
    FILE *fp;
    fp=fopen(filename,"rb+");
    if(NULL==fp)
    {
        cout<<"INFO:file open failed!";
        return false;
    }

    fseek(fp,pagenum*pagesize,SEEK_SET);

    fwrite((shmptr+(pagenum*pagesize)),pagesize,1,fp);
    fclose(fp);
    return true;
}
