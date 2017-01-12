/*
结构体声明：===>具体内存大小，页大小有待商榷<===
PS;共享内存（一块）=管理区（第0页）+数据区（第1页开始）
        管理区=SHMInfo区+表信息区=一页大小（第0页）------>>数据区分页编号时从第1页开始，防止覆盖第0 页的内存信息和表结构信息
        页=页头（PageInfo）+数据块区------>>删除表归还页给SHM时不要把页头（PageInfo）置为0 ，里面有记录页号
        块=块头（PageNode）+一条记录的数据大小（列长度和）------>>删除一条记录时不要把块头重置为0，里面记录有块号
        链：便于查找`定位`分配
        SHM中有一条链=空链------>>用于给表分配页，删除表时回收页
        表中有两条链=满链+空链------>>插入删除时转化(达到满页/设定的阈值)
        页中有两条链=满链+空链------>>插入删除一条记录时就转化
*/
#ifndef SHMSTRUCT_H
#define SHMSTRUCT_H
#include"BitMap.h"
#include <string.h>
#include"TMutex.h"

#define PID_MAX 100
#define MAX_SHM_ID 100
#define MAX_NAME_LEN 50
#define MAX_PATH_NAME_LEN 100
#define MAX_COLUMN_COUNTS 50//???多少


using namespace std;

//数据库的整体信息
struct  mdbDSN
{
    char sVersion[MAX_NAME_LEN]; 	  //版本号
    char sName[MAX_NAME_LEN];    //名称

    int iTableCounts;            //表个数
    int iMutexCounts;            //锁个数, 目前暂固定为10000。

    int iShmCounts;           //共享内存的个数
    int iShmID[MAX_SHM_ID];   //最多有100个共享内存段
    int iShmKey[MAX_SHM_ID];  //ShmID会随着系统的变化而变化而Key是唯一的

    int iProcAddr;            //进程信息的偏移量
    int iProcCounts;          //进程数

    int iPageMutexAddr;      //页锁管理区偏移量

    int iUserTableAddr;       //用户管理区偏移量
    int iTableSpaceAddr;      //用户表空间偏移量

    char sDataStore[MAX_PATH_NAME_LEN];      //文件影像的位置
    int  iPermSize;               //只有在表信息不确定的情况下，才需要设定

    int iLeftAdd;               //剩余空间偏移量
    TMutex tMutex;            //管理区共享锁
};

//共享内存块信息
struct SHMInfo
{
    int iShmID ;      //共享内存编号，用于寻址,共享内存的唯一编号即为SHMID(同一key分配得到的SHMID，在重置系统之后发生变化，存储SHMID与文件中无意义，反而易于产生歧义)
    int iShmSize = 16*1024*1024 ;         //共享内存大小32M
    int iChildSize = 15000;       //页大小32K
    int iFullPageOffset = -1 ;       //满页链首偏移，内存只关心空满链的链首在哪
    int iEmptyPageOffset  = 1;       //空页链首偏移
    int iUsedPageNum  = 0;       //SHM已用页数
    int pid_array[PID_MAX+1] = {0};     //存储连接共享内存进程的进程号,0号位存储已有多少个进程连接。
    BitMap bitmap;       //实例化一个1k的bitmap
};

//页信息
struct PageInfo
{
    int iPageID  = 0;          //页号
    int iPageSize  = 15000;       //页大小
    int iChildSize;        //子块大小=计算出来的列长度和+块节点头信息大小（没用到好像）
    int iNextPageID;      //下一个页号，
    int iEmptyPageNodeOffset = 1 ;  //空块链首偏移，页只关心空满块的链首在哪
    int iFullPageNodeOffset = -1;     ////man块链首偏移，页只关心空满块的链首在哪
    int pageStateFlag  = 0;     // 页状态标识，初始为0可用，全满-1，由满转空部分空闲可用 (-1  ->  1)
    int iUsedPageNodeNum = 0 ;      //页内已用记录块数

    int iDataOffSet;     //数据的偏移量=页首地址-页头信息大小
    //int m_iTableID;//当前属于哪个表ID
     char sTableName[MAX_NAME_LEN]={0};//当前页属于哪个表
};

//页内块节点结构体：页面内部的页节点（记录块）
struct PageNode
{
    int iPageNodeID = 0;         //块号(由块号*块大小->每一块地址)
    int iPageNodeSize;       //块大小
    int iDataSize;       //数据大小
    int iDataOffSet;       // 数据偏移=块首地址-块头信息大小
    int  iNextNode;      //下一个节点ID，用于链
    int iPageID;        //所属页号
};
/*
     RowID我们仿照Oracle的做法，定义一个RowID，通过RowID我们可以确定某个数据的具体位置，
     RowID在某个表空间中必须是唯一的（当然多个表空间很可能有相同的RowID）。
    */
struct RowID        //？？
{
    int iPageID;    //所属页号
    int  iNum;       //在页中的第几个数据
};

//列信息定义
struct Column
{
    char sName[MAX_NAME_LEN];       //列名称
    int  iDataType;           //列的数据类型
    //char  sDataType[MAX_NAME_LEN];
    int  iColumnLen; 	        //列的数据长度
    int  iPos;                //列的位置??，如果前面有VarChar类型，则值为-1
};

//表结构信息
struct Table
{
     char sTableName[MAX_NAME_LEN];       //表名
    //char*sTableName;
    int iTableID;//表ID
    char cPrimaryKey[MAX_NAME_LEN];//主键
    int iColumnCounts;                //列的个数
    Column tColumn[MAX_COLUMN_COUNTS];      //列信息
//   char sTableSpace[MAX_NAME_LEN];         //所属表空间
    int iCounts = 0;          //记录数
    int iFullPages = 0;        //已满页数
    int iFullPageID = -1;       //已满页的第一个页(满页链首偏移，表只关心空满链的链首在哪  通过表名找到页地址，之后的页通过链找到
    int iFreePages = 0;        //自由页数
    int iFreePageID = -2;      //自由页的第一个页(空页链首偏移，表只关心空满链的链首在哪
    TMutex tTableMutex;   //表的共享锁
    int iDataSize = 0;       //一条数据大小
};

//    表空间节点类的定义
struct TSNode
{
    int iPageStart;     //起始页号
    int iPageEnd;     //结束页号
    int iShmID;         //所属共享内存ID
    int  iOffSet;    //起始页在共享内存中的偏移量
    int iNext;       //下一个节点偏移量
};

// 表空间类的定义
struct TableSpace
{
    char sName[MAX_NAME_LEN];      //表空间名称
    int  iPageSize;        //页大小，单位为K，默认为4
    int  iRequestCounts;   //一次申请的页面数，默认为10万
    int  iFullPages;       //已满页数
    int  iFreePages;       //自由页数
    int  iEmptyPages;     //空闲页数
    int  iTotalPages;      //总的页数

    TSNode tNode;        //节点信息(一个内存块作为一个节点)？？？
    int  iEmptyPageID;       //空闲页的第一个页
    TMutex tEmptyMutex;      //空闲页的共享锁
};

#endif // SHMSTRUCT_H
