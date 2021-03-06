#include "TableHandle.h"
#include"ShmStruct.h"
#include<stdlib.h>


extern vector<vector<vector<string> > > alltables;
char fileName[40] = "ShmDBFile.fs";
DiskFileHandle diskFileUtil2;
//构造函数
TableHandle::TableHandle()
{
}

//析构函数
TableHandle::~TableHandle()
{
}

//接收读取到的配置文件中的参数创建表(实例化一个表对象在表信息区，)
bool TableHandle::createTable(void *const Ptr,vector <vector <string> >  &vec)        //传内存首地址
{
    Table tempTable;

    strcpy( tempTable.sTableName,vec[0][0].c_str());//表名
    tempTable.iTableID= atoi(vec[1][0].c_str());//表ID
    strcpy( tempTable.cPrimaryKey,vec[2][0].c_str());   //接收表信息
    tempTable.iColumnCounts= vec.size()-3;//表列数

    for(int i=0; i<vec.size()-3; i++)   //接收列信息
    {
        strcpy (tempTable.tColumn[i].sName,vec[i+3][0].c_str());
        tempTable.tColumn[i].iPos=atoi(vec[i+3][1].c_str());
        tempTable.tColumn[i].iDataType=atoi(vec[i+3][2].c_str());
        tempTable.tColumn[i].iColumnLen=atoi(vec[i+3][3].c_str());

        tempTable.iDataSize=tempTable.iDataSize+tempTable.tColumn[i].iColumnLen;//计算数据长度
    }
   // cout<<"  表信息区该表位置 "<<Ptr+sizeof(SHMInfo)+sizeof(Table)*tempTable.iTableID<<endl;
  //  cout<<"  SHMInfo头大小 "<<sizeof(SHMInfo)<<endl;
  //  cout<<"  Table头大小 "<<sizeof(Table)<<endl;
//cout<<"  PageInfo头大小 "<<sizeof(PageInfo)<<endl;
   // cout<<"  iTableID******"<<tempTable.iTableID<<endl;
  //  cout<<"  创建表时内存首地址 "<<Ptr<<endl;

    memcpy((Ptr+sizeof(SHMInfo)+sizeof(Table)*tempTable.iTableID),&tempTable,sizeof(Table));        //地址计算
   //不改位图直接调用磁盘写第0页
   SHMInfo *shmInfo=(SHMInfo*)Ptr;
   diskFileUtil2.diskFileWritePage(fileName,Ptr,0,shmInfo->iChildSize);
   // shmInfo->bitmap.set(0,1);//改位图
    cout<<"  表创建成功 "<<endl;
    return true;
}

//删除表(删除表信息区的对象，表所属页删空（用0覆盖），归还给空页链管理，重连空页链)
bool TableHandle::dropTable(void * const Ptr,vector <vector <string> > &vec)
{
    Table tempTable;
    strcpy( tempTable.sTableName,vec[0][0].c_str());
    //cout<<"  要删除的表名***********"<<tempTable.sTableName<<endl;
    tempTable.iTableID= atoi(vec[1][0].c_str());
    cout<<"  要删除的表id "<<tempTable.iTableID<<endl;
    memset((Ptr+sizeof(SHMInfo)+sizeof(Table)*(tempTable.iTableID)),0,sizeof(Table));//该表的信息区重置为0

    SHMInfo *shmInfo=(SHMInfo*)Ptr;
    //不改位图直接调用磁盘写第0页
    diskFileUtil2.diskFileWritePage(fileName,Ptr,0,shmInfo->iChildSize);
    // shmInfo->bitmap.set(0,1);//改位图
    PageInfo *pageInfo=NULL;
    void *str=Ptr+(shmInfo->iChildSize);//第一个数据页

        //好像不需要遍历所有页，只遍历该表两条链上的页就行？？
    for(str; str<Ptr+(shmInfo->iShmSize); str=str+(shmInfo->iChildSize))  ////循环遍历内存中所有的页，根据m_iTableID所属表ID把这些页的除页号外全重置为0，重新链到shm的空链上
    {
        //cout<<"  tablehandle回收该表的页归还给内存 "<<endl;
        pageInfo=(PageInfo *)str;
        if(strcmp(pageInfo->sTableName,tempTable.sTableName)==0)
        {
            memset(((void*)pageInfo)+sizeof(PageInfo),0,((shmInfo->iChildSize)-sizeof(PageInfo)));//把该页除页头信息外重置为0
            shmInfo->bitmap.set(pageInfo->iPageID,1);//改位图
            pageInfo->iNextPageID=shmInfo->iEmptyPageOffset;     //该页的下一页指向内存现在的首空
            shmInfo->iEmptyPageOffset=pageInfo->iPageID;     //内存首空指向该页
        }
    }
    //从alltables中删除此table
    for(int i=0;i<alltables.size();++i){
        if(strcmp(tempTable.sTableName,alltables[i][0][0].c_str())==0){
            if(i <= alltables.size())
            {
             alltables.erase(alltables.begin() + i);
            }
        }
    }
    return true;
}

//给表分配空页页并初始化页内分块, 然后重连内存空页
void  TableHandle::pageInit(void *const ptr,Table *  tablePtr)
{
    SHMInfo *shmInfo=(SHMInfo*)ptr;
    PageInfo pageInfo;
    PageInfo * pagePtr=NULL;
    if(shmInfo->iEmptyPageOffset == -1)
    {
        cout<<"  内存中已无可用空页 "<<endl;
    }
    else
    {
        pagePtr=(PageInfo*)(ptr+(shmInfo->iEmptyPageOffset)*(pageInfo.iPageSize));//找到要分配的内存首空页地址
        shmInfo->iEmptyPageOffset=pagePtr->iNextPageID;//内存首空页指向下一页（重连内存空页链）
        pagePtr->iNextPageID=  tablePtr->iFreePageID;//该页插入表空页链
        tablePtr->iFreePageID=pagePtr->iPageID;//表头空页偏移指向该页

        strcpy((pagePtr->sTableName),(tablePtr->sTableName));//标记该页属于此表
        //初始化表内块
        int iPageNodeSize = tablePtr->iDataSize+sizeof(PageNode);
        PageNode tempPageNodeHead;
        int tempPageNodeNum = pageInfo.iPageSize/iPageNodeSize;
        int tempPageNodeHeadSize = sizeof( PageNode);

        for(int i=1; i<tempPageNodeNum; i++)  //循环，将块头拷贝至应在的地址位置
        {
            tempPageNodeHead.iPageNodeID= i;
            if(i != tempPageNodeNum-1)
            {
                tempPageNodeHead.iNextNode = i + 1;//置i+1,表示初始状态，第i块的下一个空块为i+1块
            }
            else
            {
                tempPageNodeHead.iNextNode = -1;//置-1，表示已到空块链结尾
            }
            memcpy(((void*)pagePtr)+i*iPageNodeSize,&tempPageNodeHead,tempPageNodeHeadSize);//块头初始化
        }
    }
}

//接收参数匹配操作
void TableHandle::option(void *const ptr,vector <vector <string> > &vec)
{
    char tempStr[MAX_NAME_LEN]={0};
    strcpy (tempStr,vec[1][0].c_str());//拿到操作（insert/delete/update/select）
    if (strcmp(tempStr,"insert")==0)
    {
        shmInsert(ptr,vec);
    }
    else if (strcmp(tempStr,"select*")==0)
    {
        shmSearch(ptr,vec);
    }
   else if (strcmp(tempStr,"update")==0)
    {
        shmUpdate(ptr,vec);
    }
   else if (strcmp(tempStr,"delete")==0)
    {
        shmDelete(ptr,vec);
    }
   else if (strcmp(tempStr,"drop")==0)
    {
        dropTable(ptr,vec);
    }
    else if(strcmp(tempStr,"desc")==0){
        descTable(vec);
    }
    else if(strcmp(tempStr,"show")==0){
        show();
    }
    else{
        cout<<"  tablehandle.cpp-unknown operation"<<endl;
    }
}
void TableHandle::descTable(vector <vector <string> > &vec){
    for(int i=0;i<alltables.size();++i){
        if(vec[0][0].compare(alltables[i][0][0])==0){
            cout<<"  tableName: "<<alltables[i][0][0]<<"  tableId: "<<alltables[i][1][0]<<"  key: "<<alltables[i][2][0]<<endl;
            for(int n=3;n<alltables[i].size();++n){
                for(int m=0;m<4;m+=4){
                cout<<"  "<<alltables[i][n][m]<<" "<<alltables[i][n][m+1]<<" ";
                cout<<alltables[i][n][m+2]<<" "<<alltables[i][n][m+3]<<endl;
                }
            }
        }
    }
}

void TableHandle::show(){
    cout<<"  有如下表"<<endl;
    for(int i=0;i<alltables.size();++i){
        cout<<"  "<<alltables[i][0][0]<<endl;
    }
}
//表层查询处理，内部调用页内的对应函数
void TableHandle::shmSearch(void *const ptr,vector <vector <string> > &vec)
{
    Table searchTable;
    bool isTable=false;//判断表是否存在
    strcpy( searchTable.sTableName,vec[0][0].c_str());//
    SHMInfo *shmInfo=(SHMInfo*)ptr;
    void*tableAdr= ptr+sizeof(SHMInfo);//得到表信息区的首地址
    void *PageAdr=ptr+(shmInfo->iChildSize);//第一页的地址
    PageNode*retPageNode=NULL;
    for(tableAdr; tableAdr<(ptr+shmInfo->iChildSize); tableAdr=tableAdr+sizeof(Table))  //遍历表信息区
    {
        //cout<<"  tablehandle-开始遍历某页"<<endl;
        Table*tablePtr=(Table*)tableAdr;
        if(strcmp(tablePtr->sTableName,searchTable.sTableName)==0)
        {
            //cout<<"  tablehandle-表名"<<tablePtr->sTableName<<endl;
            isTable=true;
            int ipageNodeSize  = tablePtr->iDataSize+sizeof(PageNode);//计算出该表数据大小
            if(tablePtr->iFreePageID== -2)      //表中尚无 空页页时
            {
                cout<<"  tablehandle查询失败，表中无数据"<<endl;
            }
            else
            {
                //循环遍历内存中所有的页，根据sTableName所属表名找到所有页
                for(PageAdr; PageAdr<(ptr+(shmInfo->iShmSize)); PageAdr=(PageAdr+(shmInfo->iChildSize)))
                {
                    PageInfo* pageInfo=(PageInfo *)PageAdr;
                    if(strcmp(pageInfo->sTableName,tablePtr->sTableName)==0)
                    {
                        //cout<<"  tablehandle-该表的页号"<<pageInfo->iPageID<<endl;
                        retPageNode =  pageUtil.pageSearch(shmInfo,tablePtr,vec, ipageNodeSize,pageInfo);  //调用页内的insert函数,由tablePtr->iFreePageID->iEmptyPageNodeOffset,插入数据
                    }
                }
                if(retPageNode!=NULL)
                {
                    //cout<<"retPageNode"<<;
                    cout<<"  tablehandle-INFO：查询成功 "<<endl;
                }
                else
                {
                    cout<<"  tablehandle-查询结束，未找到该数据 "<<endl;
                }
            }
        }
    }
    if(isTable==false)
    {
        cout<<"  tablehandle-要查询的表不存在 "<<endl;
    }
}

//表层插入处理，内部调用页内的对应函数，插入在在 空链首页找空块插入数据 ！！！！！！！****插入要检查主键是否已存在****！！！！！！！
void TableHandle::shmInsert(void *const ptr ,vector <vector <string> >& vec)
{
    //cout<<"  tablehandle-插入数据时ptr内存首地址"<<ptr<<endl;
    Table tempTable;
    bool isTable=false;
    strcpy( tempTable.sTableName,vec[0][0].c_str());//取表名
    SHMInfo *shmInfo=(SHMInfo*)ptr;
    void*tableAdr= ptr+sizeof(SHMInfo);//得到表信息区的首地址
    //cout<<"  tablehandle-tableAdr表信息区首地址"<<tableAdr<<endl;
    for(tableAdr; tableAdr<(ptr+shmInfo->iChildSize); tableAdr=tableAdr+sizeof(Table))  //遍历表信息区
    {
        //先判断要插入的表是否有页？如果有，调用页内insert函数
        //若没有页，先给该表分配shm的空页链的第一个空页，然后对该页分块
        Table*tablePtr=(Table*)tableAdr;
        if(strcmp(tablePtr->sTableName,tempTable.sTableName)==0)
        {
            //cout<<"  tablehandle-表名**********"<<tablePtr->sTableName<<endl;
            //cout<<"  tablehandle-student表地址***************"<<tablePtr<<endl;

            isTable=true;
            int ipageNodeSize  = tablePtr->iDataSize+sizeof(PageNode);//表的数据大小+块头大小=块大小
            if(tablePtr->iFreePageID== -2)      //表中尚无页时
            {
                cout<<"  tablehandle-表中尚无页，准备给表空页，并初始化页 "<<endl;
                pageInit(ptr,tablePtr);  //分配一个空页并初始化页内块大小
                cout<<"  tablehandle-已分给该表一页，并完成初始化页 "<<endl;
            }

                //查找vec中的主键
                string keyName=tablePtr->cPrimaryKey;
                string keyValue=pageUtil.look(vec,keyName);
                //判断主键唯一
                vector <vector <string> > newVec(vec);
                newVec[2].clear();//清除原查询条件
                newVec[2].push_back(keyName);
                newVec[2].push_back("=");
                newVec[2].push_back(keyValue);
                PageNode* p=NULL;
                //遍历该表所有页，判断主键是否已存在
                //循环遍历内存中所有的页，根据sTableName所属表名找到所有页
                for(void *PageAdr=ptr+(shmInfo->iChildSize); PageAdr<(ptr+(shmInfo->iShmSize)); PageAdr=(PageAdr+(shmInfo->iChildSize)))
                {
                    PageInfo* pageInfo=(PageInfo *)PageAdr;
                    if(strcmp(pageInfo->sTableName,tablePtr->sTableName)==0)
                    {
                        p= pageUtil.pageSearch(shmInfo,tablePtr,newVec, ipageNodeSize,pageInfo);
                        if(p!=NULL){break;}
                    }
                }
                if(p!=NULL){
                    cout<<p<<endl;
                cout<<"  此主键已存在，插入失败"<<endl;
                break;
                 }
                else{
                    pageUtil.pageInsert(shmInfo,tablePtr,vec,  ipageNodeSize);  //调用页内的insert函数,由tablePtr->iFreePageID->iEmptyPageNodeOffset,插入数据
                    break;
                }
        }
    }
    if(isTable==false)
    {
        cout<<"  tablehandle-要插入的表不存在 "<<endl;
    }
}

// 删除涉及链的转化   表层删除处理，内部调用页内的对应函数
void TableHandle::shmDelete(void *const ptr,vector <vector <string> > &vec)
{
    //cout<<"  tablehandle-删除数据时ptr内存首地址"<<ptr<<endl;
    Table tempTable;
    bool isTable=false;
    strcpy( tempTable.sTableName,vec[0][0].c_str());
    SHMInfo *shmInfo=(SHMInfo*)ptr;
    void*tableAdr= ptr+sizeof(SHMInfo);//得到表信息区的首地址

    //cout<<"  tableAdr表信息区首地址***************"<<tableAdr<<endl;

    for(tableAdr; tableAdr<(ptr+shmInfo->iChildSize); tableAdr=tableAdr+sizeof(Table))  //遍历表信息区
    {
        Table*tablePtr=(Table*)tableAdr;
        if(strcmp(tablePtr->sTableName,tempTable.sTableName)==0)
        {

            isTable=true;
            int ipageNodeSize  = tablePtr->iDataSize+sizeof(PageNode);
            if(tablePtr->iFreePageID== -2)      //表中尚无页时
            {
                cout<<"  删除失败，表中无数据 "<<endl;
            }
            else
            {
                int tempPageID ;
                PageNode* retPageNode = NULL;
                PageInfo* delPageAdr = NULL;
                //============================查找满链，满链上删数据可能会造成页状态改变，页位置改变===========================================================================
                tempPageID = tablePtr->iFullPageID;//删除操作先从满链开始搜寻数据，首页页号赋值为满偏移
                if(tempPageID == -1)
                {
                    cout<<"  表满链无数据，检查空链 "<<endl;
                }
                else    //首页位置保存在内存首部，所以分开考虑，不能加入do while循环
                {
                    delPageAdr = ( PageInfo*)(ptr + (shmInfo->iChildSize)*tempPageID);//根据页号拿地址
                    retPageNode = pageUtil.pageDelete(shmInfo,tablePtr,vec, ipageNodeSize,delPageAdr);  //此处调用页处理类的对应方法
                    if(retPageNode!=NULL)   //成功删除得到删除项所在地址，temp_item_ptr不为空
                    {
                        cout<<"  页内，删除成功 "<<endl;
                        fullToEmpty(shmInfo,tablePtr,delPageAdr);//删除成功需检查页状态，更新满链空链,函数返回
                    }
                    if(delPageAdr->iNextPageID == -1)
                    {
                        cout<<"  表满链只有一页，查找结束 "<<endl;
                    }
                    else
                    {
                        do   //需要循环检测,使用do while,先操作查找，再判断偏移是否为-1
                        {
                            tempPageID = delPageAdr->iNextPageID ;             //更新页号，页地址
                            delPageAdr =  ( PageInfo*)(ptr + (shmInfo->iChildSize)*tempPageID);
                            retPageNode = pageUtil.pageDelete(shmInfo,tablePtr,vec, ipageNodeSize,delPageAdr);  //此处调用页处理类的对应方法
                            if(retPageNode!=NULL)   //成功删除得到删除项所在地址，temp_item_ptr不为空
                            {
                                cout<<"  页内，删除成功 "<<endl;
                                fullToEmpty(shmInfo,tablePtr,delPageAdr);//删除成功需检查页状态，更新满链空链,函数返回
                            }
                        }
                        while(delPageAdr->iNextPageID == -1);
                    }
                }
//=========================================查找空链,空链上删数据，页状态不变，页位置不变======================================================================
                tempPageID = tablePtr->iFreePageID;//删除操作再从空链搜寻数据，首页页号赋值为空偏移
                if(tempPageID == -2)
                {
                    cout<<"  表中无空链,检查结束 "<<endl;
                }
                else
                {
                    delPageAdr =  ( PageInfo*)(ptr + (shmInfo->iChildSize)*tempPageID);
                    retPageNode = pageUtil.pageDelete(shmInfo,tablePtr,vec, ipageNodeSize,delPageAdr);  //此处调用页处理类的对应方法
                    if(retPageNode!=NULL)   //成功删除得到删除项所在地址，temp_item_ptr不为空
                    {
                        cout<<"  页内，删除成功 "<<endl;
                        //页内做change_list_empty(ptr,temp_pagenum);//删除成功需检查页状态，更新满链空链,函数返回
                    }
                    if(delPageAdr->iNextPageID == -2)
                    {
                        cout<<"  表空链只有一页，查找结束 "<<endl;
                    }
                    else
                    {
                        do   //需要循环检测,使用do while,先操作查找，再判断偏移是否为-1
                        {
                            tempPageID = delPageAdr->iNextPageID ;             //更新页号，页地址
                            delPageAdr =  ( PageInfo*)(ptr + (shmInfo->iChildSize)*tempPageID);
                            retPageNode = pageUtil.pageDelete(shmInfo,tablePtr,vec, ipageNodeSize,delPageAdr);  //此处调用页处理类的对应方法
                            if(retPageNode!=NULL)   //成功删除得到删除项所在地址，temp_item_ptr不为空
                            {
                                cout<<"  页内，删除成功 "<<endl;
                            }
                        }
                        while(delPageAdr->iNextPageID == -1);
                    }
                }
                if(retPageNode == NULL)
                {
                    cout<<"  删除失败，找不到数据 "<<endl;
                }
            }
        }
    }
    if(isTable==false)
    {
        cout<<"  要删除的表不存在 "<<endl;
    }
}

//表层更新处理，内部调用页内的对应函数
void TableHandle:: shmUpdate(void *const ptr,vector <vector <string> > &vec)
{
    //cout<<"  更新数据时ptr内存首地址 "<<ptr<<endl;

    Table searchTable;
    bool isTable=false;
    strcpy( searchTable.sTableName,vec[0][0].c_str());//
    SHMInfo *shmInfo=(SHMInfo*)ptr;
    void*tableAdr= ptr+sizeof(SHMInfo);//得到表信息区的首地址
    void *PageAdr=ptr+(shmInfo->iChildSize);//第一页的地址
    PageNode*retPageNode=NULL;

    //cout<<"  tableAdr表信息区首地址 "<<tableAdr<<endl;

    for(tableAdr; tableAdr<(ptr+shmInfo->iChildSize); tableAdr=tableAdr+sizeof(Table))  //遍历表信息区
    {
        Table*tablePtr=(Table*)tableAdr;
        if(strcmp(tablePtr->sTableName,searchTable.sTableName)==0)
        {
            isTable=true;
            int  ipageNodeSize  = tablePtr->iDataSize+sizeof(PageNode);//计算出该表数据大小
            if(tablePtr->iFreePageID== -2)      //表中尚无页时
            {
                cout<<"  更新失败，表中无数据！*****"<<endl;
            }
            else
            {
                //循环遍历内存中所有的页，根据sTableName所属表名找到所有页
                for(PageAdr; PageAdr<ptr+(shmInfo->iShmSize); PageAdr=PageAdr+(shmInfo->iChildSize))
                {
                    PageInfo* pageInfo=(PageInfo *)PageAdr;
                    if(strcmp(pageInfo->sTableName,tablePtr->sTableName)==0)
                    {
                        retPageNode =  pageUtil.pageUpdate(shmInfo,tablePtr,vec, ipageNodeSize,pageInfo);  //调用页内的insert函数,由tablePtr->iFreePageID->iEmptyPageNodeOffset,插入数据
        
                    }
                }
                if(retPageNode!=NULL)
                {
                    //cout<<"retPageNode"<<;
                    cout<<"  更新成功 "<<endl;
                }
                else
                {
                    cout<<"  更新失败，未找到该数据 "<<endl;
                }
            }
        }
    }
    if(isTable==false)
    {
        cout<<"  要更新的表不存在 "<<endl;
    }
}

//删除满页中节点时，页状态改变 满->空
void TableHandle::fullToEmpty(SHMInfo* shmInfo,Table *  tablePtr,PageInfo*pageInfo)
{
    if(pageInfo->pageStateFlag==1) //满页调用页删除函数后状态改变
    {
        void *temppageInfo=((void*)shmInfo+pageInfo->iPageSize);//第一页地址
        //遍历所有页，找到该页上一页
        for(temppageInfo ; temppageInfo<((void*)shmInfo+shmInfo->iShmSize); temppageInfo=temppageInfo+pageInfo->iPageSize)//遍历所有页
        {
            PageInfo*temppageaddr= (PageInfo*) temppageInfo;
            if(temppageaddr->iNextPageID==pageInfo->iPageID)
            {
                temppageaddr->iNextPageID=pageInfo->iNextPageID;//重连原表满页链
                pageInfo->iNextPageID=tablePtr->iFreePageID;//插到表的空页链首
                tablePtr->iFreePageID=pageInfo->iPageID;
            }
        }
    }
}

//展示表信息
void TableHandle::showTableInfo(void *const ptr, vector <vector <string> > &vec)
{
    Table tempTable;
    strcpy( tempTable.sTableName,vec[0][0].c_str());
    bool isTable = false;
    SHMInfo *shmInfo=(SHMInfo*)ptr;
    void*tableAdr= ptr+sizeof(SHMInfo);//得到表信息区的首地址
    for(tableAdr; tableAdr<(ptr+shmInfo->iChildSize); tableAdr=tableAdr+sizeof(Table))  //遍历表信息区
    {
        Table*tablePtr=(Table*)tableAdr;
        if(strcmp(tablePtr->sTableName,tempTable.sTableName)==0)
        {
            isTable=true;
            cout<<endl<<"  Table information!********"<<endl;
            cout<<"  TableName: "<<tablePtr->sTableName<<"  PrimaryKey: "<<tablePtr->cPrimaryKey<<endl;
            cout<<"  ColumnCounts: "<<tablePtr->iColumnCounts<<"    Counts: "<<tablePtr->iCounts<<endl;
            cout<<"  iFreePageID: "<<tablePtr->iFreePageID<<"    iFullPageID: "<<tablePtr->iFullPageID<<endl;
            cout<<"  iTableID: "<<tablePtr->iTableID<<endl;
        }
    }
    if(isTable==false)
    {
        cout<<"  要展示的表不存在*****"<<endl;
    }
}
