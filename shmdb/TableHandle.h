/*
    表处理类，表的创建/删除，表一层的增删改查
    插入和删除（达到一页满/转化阈值）要涉及表内链的转化
*/
#ifndef TABLEHANDLE_H
#define TABLEHANDLE_H
#include<vector>
#include"DiskFileHandle.h"
#include"PageHandle.h"
#include"ShmStruct.h"

class TableHandle
{
public:
    TableHandle();
    virtual ~TableHandle();

    bool createTable(void *const ptr,vector <vector <string> > &vec);     //接收读取到的配置文件中的参数创建表
    bool dropTable(void * const Ptr,vector <vector <string> > &vec);       //删除表
    void pageInit(void *  const ptr,Table *  tablePtr);
    void option(void *const ptr,vector <vector <string> > &vec);//接收参数匹配操作
    void shmInsert(void *const ptr ,vector <vector <string> > &vec);        //表插入函数
    void shmDelete(void *const ptr,vector <vector <string> > &vec);        //表删除函数
    void shmUpdate(void *const ptr,vector <vector <string> > &vec);        //表更新函数
    void shmSearch(void *const ptr,vector <vector <string> > &vec);        //表查询函数
    void fullToEmpty(SHMInfo* shmInfo,Table *  tablePtr,PageInfo*pageInfo);//删除满页中节点时，页状态改变 满->空
    void descTable(vector <vector <string> > &vec);
    void show();
     void showTableInfo(void *const ptr,vector <vector <string> > &vec );

    PageHandle pageUtil;

};

#endif // TABLEHANDLE_H
