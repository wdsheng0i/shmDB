#include "ShmHandle.h"
#include <iostream>
#include<iomanip>
#include <string.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include<unistd.h>
#include <signal.h>
#include <stdio.h>

extern vector<vector<vector<string> > > alltables;
SHMInfo shmInfo;//作为基本数据，拷贝至内存，不可调用此对象的bitmap等非确定元素
PageInfo pageInfo;//
void *shmAddr = NULL;
DiskFileHandle diskFileUtil;
//PageHandle pageUtil;

//构造函数
ShmHandle::ShmHandle()
{
}

//析构函数
ShmHandle::~ShmHandle()
{
}

//创建共享内存并建立对应的磁盘文件
bool ShmHandle::shmCreat(char* diskfilename, int shmsize,int shmKey )
{
    //以下代码为依据shmKey创建共享内存，如果共享内存已存在，依据shmKey返回shmId;
    int shmId = shmget(shmKey, shmsize, 0666 | IPC_CREAT);      //加|IPC_EXCL如果共享内存已存在返回错误,失败返回-1
    if (shmId != -1)
    {
        cout<<"*****INFO:SHM INIT*****"<<endl;
        shmAddr = shmat(shmId,NULL,0);
        bool bReadFlag = diskFileUtil.diskFileReadPage(shmAddr,0,shmInfo.iChildSize,diskfilename);//创建内存后先读磁盘文件是否存在
        if(bReadFlag)
        {
            cout<<"*****INFO:文件存在，准备页加载...*****"<<endl;
         //   for(int i=1; i<shmInfo.iShmSize/shmInfo.iChildSize; i++)
         for(int i=0; i<shmInfo.iShmSize/shmInfo.iChildSize; i++)//从第0页开始全部加载进内存
            {
                diskFileUtil.diskFileReadPage(shmAddr,i,shmInfo.iChildSize,diskfilename);
            }
            cout<<"*****INFO:页加载结束*****"<<endl;
            cout<<"  从内存初始化alltables"<<endl;
           Table* table;
           int onceZero=0;
           for(table=(Table*)(shmAddr+sizeof(shmInfo));(void*)table<shmAddr+shmInfo.iChildSize;table=(Table*)((void*)table+sizeof(Table))){
            //只允许一次table->iTableID是零的情况，再次出现说明是空内存
            //不过也有bug,若第一次插表，没插入id为0的表，则
            //此时从内存初始化的alltables里仍有id为0的表，虽然全空，也导致后面不能插入id为0的表
            if(table->iTableID==0){
                ++onceZero;
                if(onceZero>1) {break; }
            }
            std::vector<std::vector<string> > tab;
            std::vector<string>  tabName;
            tabName.push_back(table->sTableName);
            cout<<"  初始化alltables"<<table->sTableName<<" "<<table->iTableID<<endl;
            tab.push_back(tabName);
            std::vector<string>  tabId;
            char tableId[50];
            sprintf(tableId,"%d",table->iTableID);
            tabId.push_back(tableId);
            tab.push_back(tabId);
            std::vector<string>  tabKey;
            tabKey.push_back(table->cPrimaryKey);
            tab.push_back(tabKey);
              for(int i=0;i<table->iColumnCounts;++i){
                std::vector<string> tabCol;
                tabCol.push_back(table->tColumn[i].sName);
                char ipos1[50];
                sprintf(ipos1,"%d",table->tColumn[i].iPos);
                tabCol.push_back(ipos1);
                char dataType[50];
                sprintf(dataType,"%d",table->tColumn[i].iDataType);
                tabCol.push_back(dataType);
                char iColumnLen[50] ;
                sprintf(iColumnLen,"%d",table->tColumn[i].iColumnLen);
                tabCol.push_back(iColumnLen);
                tab.push_back(tabCol);
              }
              alltables.push_back(tab);
              //cout<<"  size"<<alltables.size()<<endl;
           }
        }
        else
        {
            cout<<"*****INFO:初始化内存块 *****"<<endl;
            memset(shmAddr,0,shmsize);//内存初始化全为0
            memcpy(shmAddr,&shmInfo,sizeof(struct SHMInfo));//把内存信息拷到内存头部
            shmControl(shmAddr,shmInfo.iShmSize,shmInfo.iChildSize);//初始化分页编号
            diskFileUtil.diskFileWrite(diskfilename,shmAddr,shmInfo.iShmSize);//创建文件并把整块内存写入文件
            cout<<"*****INFO:创建文件，并将内存块写入*****"<<endl;
        }
        shmdt(shmAddr);
        cout<<"*****INFO:SHM INIT FINISH*****"<<endl;
    }
    else
    {
        cout << "INFO:Creat Shm Failed!,Return."<< endl;
        return false;
    }
    return true;
}

//初始化共享内存，分页编号串起来成为初始空链
bool ShmHandle::shmControl(void *ptr, int shmsize, int pagesize )
{
    cout << "*****INFO:内存分页开始*****"<<endl;
    PageInfo tempPageHead;
    int tempPageNum = shmsize/pagesize;
    int tempPageHeadSize = sizeof( PageInfo);

    /*
    *整理页头，准备拷贝 ,多余的准备工作，只是显示的变现出拷贝前整理页头
    PageInfo结构体对象有默认的和下面相同的初始值
    */
    tempPageHead.pageStateFlag = 0;
    tempPageHead.iUsedPageNodeNum  = 0;
    tempPageHead.iPageSize = pagesize;
    tempPageHead.iEmptyPageNodeOffset = 1;

    for(int i=1; i<tempPageNum; i++)  //循环，将页头拷贝至页应在的地址位置 从第一页开始
    {
        tempPageHead.iPageID = i;
        if(i != tempPageNum-1)
        {
            tempPageHead.iNextPageID = i + 1;//置i+1,表示初始状态，第i页的下一个空页为i+1页
        }
        else
        {
            tempPageHead.iNextPageID = -1;//置-1，表示已到空链结尾
        }
        memcpy(ptr+i*pagesize,&tempPageHead,tempPageHeadSize);//页头初始化 地址从第一页开始，第0页时内存信息区+table信息区，不初始化，暂未编号
    }
    cout << "*****INFO:分页完成*****"<<endl;
    return true;
}

//l进程链接共享内存并记录其id及链接进程数目
bool ShmHandle::shmAttach(void*& ptr, const int shmKey)
{
    //cout << "*****AttachShm Start*****"<< shmfile_key<<endl;
    int shmId = shmget(shmKey, 0, 0666 | IPC_CREAT);//大小为0，只获得已存在的共享内存
    ptr = shmat(shmId,NULL,0);//失败返回-1
    if ((long int)ptr != -1)  //ptr在32位系统下，指针长度4字节，64位系统8字节，此处转型为long int防止丢失精度
    {
        SHMInfo *temp  = ( SHMInfo *)ptr;
        if(temp->pid_array[0]<PID_MAX)
        {
            temp->pid_array[0] =  temp->pid_array[0] +1;
            temp->pid_array[temp->pid_array[0]] = getpid();
            cout<<"连接>>进程NO:"<<temp->pid_array[0]<<" 进程ID："<< temp->pid_array[temp->pid_array[0]] <<endl;
            cout << "*****INFO:内存链接成功！******" << endl;
        }
        else
        {
            cout<<"*****INFO:达到最大连接量,不可再连接*****"<<endl;
            shmdt(ptr);
            return false;
        }
    }
    else
    {
        cout << "INFO:Shm Attach Failed!, Return."<< endl;
        return false;
    }
    return true;
}

//断开进程链接
bool ShmHandle::shmDelAttach(void*& ptr)
{
    SHMInfo *temp  = ( SHMInfo *)ptr;
    cout<<"断开<<进程NO:"<<temp->pid_array[0]<<" 进程ID："<< temp->pid_array[temp->pid_array[0]] <<endl;
    temp->pid_array[temp->pid_array[0]] = 0;
    temp->pid_array[0] =  temp->pid_array[0] -1;
    bool shmdtState = shmdt(ptr);//成功返回0
    if(!shmdtState)
    {
        cout << "INFO:Shm Delete Success!"<< endl;
    }
    else
    {
        cout << "INFO:Shm Delete Failed!, Return."<< endl;
        return false;
    }
    ptr = NULL;
    return true;
}

//销毁进程
bool ShmHandle::shmDestroy(const int shmKey)
{
    void *ptr = NULL;
    shmAttach(ptr,shmKey);
    SHMInfo* tempPtr = ( SHMInfo *)ptr;
    for(int i=0; i<tempPtr->pid_array[0]-1; i++)   //杀死除自己的所有进程
    {
        kill(tempPtr->pid_array[i+1],SIGKILL);
    }
    shmDelAttach(ptr);
    int shmId = shmget(shmKey, 0, 0666 | IPC_CREAT);
    bool shmdtState = shmctl(shmId,IPC_RMID,NULL);//成功返回0
    if(!shmdtState)
    {
        cout << "INFO:Shm Destroy Success!"<< endl;
    }
    else
    {
        cout << "INFO:Shm Destroy Failed!, Return."<< endl;
        return false;
    }
    ptr = NULL;
    return true;
}

//页展示
bool ShmHandle::showPageInfo(void *const ptr,int pagenum,int pagesize )//展示页头信息
{
    PageInfo *tempPageInfo;
    tempPageInfo = ( PageInfo *)(ptr + pagenum*pagesize);
    cout<<endl<<"**********This is the page information!*******"<<endl;
    cout<<"PageNum:"<<tempPageInfo->iPageID<<"  PageState:"<<tempPageInfo->pageStateFlag<<endl;
    cout<<"ItemEmputyOffset:"<<tempPageInfo->iEmptyPageNodeOffset<<" UsedBlock:"<<tempPageInfo->iUsedPageNodeNum<<endl;
//   cout<<"PageEmputyOffset:"<<tempPageInfo->iEmptyPageOffset<<"  PageFullOffset:"<<tempPageInfo->iFullPageOffset<<endl<<endl;
    return true;
}

//内存信息展示
bool ShmHandle::showShmInfo(void *const ptr)
{
    SHMInfo *tempShmInfo;
    int num = numsCount(ptr);
    tempShmInfo = ( SHMInfo *)ptr;
    cout<<endl<<"**********This is the Share Memory information!*******"<<endl;
    cout<<"ShmSize(M):"<<tempShmInfo->iShmSize/1024/1024<<" FulledPages:"<<tempShmInfo->iUsedPageNum<<endl;
    cout<<"TotalItems:"<<num<<"    Percent(%):"<<setiosflags(ios::fixed)<<setprecision(2)<<num*100.0*pageInfo.iChildSize/shmInfo.iShmSize<<endl;
    cout<<"EmputyOffset:"<<tempShmInfo->iEmptyPageOffset<<endl;
    return true;
}

//计数
int ShmHandle::numsCount(void *const ptr)
{
    int pagesMax = shmInfo.iShmSize/shmInfo.iChildSize;
    int itemCount  = 0;
    PageInfo* pageInfoPtr = NULL;
    for(int i=1; i<pagesMax; i++)
    {
        pageInfoPtr = ( PageInfo* )(ptr + i*shmInfo.iChildSize);
        itemCount += pageInfoPtr->iUsedPageNodeNum;
    }
    return itemCount;
}
