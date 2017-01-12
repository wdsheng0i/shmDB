/*
    页内处理类，增删改查
    插入和删除一条记录要涉及页内链的转化
    增删改要涉及改位图
*/
#ifndef PAGEHANDLE_H
#define PAGEHANDLE_H
#include<vector>
#include"ShmStruct.h"

class PageHandle
{
public:
    PageHandle();
    virtual ~PageHandle();

    PageNode * pageSearch(SHMInfo* shmInfo,Table *  tablePtr,vector <vector <string> > &vec, int ipageNodeSize,PageInfo *pageInfo);      //页查找函数
    PageNode * pageInsert(SHMInfo* shmInfo,Table *  tablePtr,vector <vector <string> > &vec, int ipageNodeSize);//页插入函数
    PageNode * pageDelete(SHMInfo* shmInfo,Table *  tablePtr,vector <vector <string> > &vec, int ipageNodeSize,PageInfo*pageInfo);     //页删除函数
    PageNode * pageUpdate(SHMInfo* shmInfo,Table *  tablePtr,vector <vector <string> > &vec, int ipageNodeSize,PageInfo *pageInfo);   // 页更新函数
    //查找frominput中某字段值，即v【2】中某字段值
	string look(std::vector<std::vector<string > > v,string name);    
};

#endif // PAGEHANDLE_H
