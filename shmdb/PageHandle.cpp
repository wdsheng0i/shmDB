#include "PageHandle.h"
#include"TableHandle.h"
#include <exception> 

TableHandle tableHandleUtil;

//存下查询结果集
std::vector<PageNode *> fromSearch;

//构造函数
PageHandle::PageHandle()
{
}

//析构函数
PageHandle::~PageHandle()
{
}

string PageHandle::look(std::vector<std::vector<string > > v,string name){
    for(int i=0;i<v[2].size();i+=3){
        if(name.compare(v[2][i])==0){
            return v[2][i+2];
        }
    }
    return NULL;
}

//页内查询，只按单一属性查找，返回一个记录（）
PageNode * PageHandle::pageSearch(SHMInfo* shmInfo,Table *  tablePtr,vector <vector <string> > &vec, int ipageNodeSize,PageInfo*pageInfo )

{
    //cout<< "pagesearch"<<endl;
     //cout<<" ipageNodeSize"<<ipageNodeSize<<"  node大小"<<sizeof(PageNode)<<endl;
    int pageNodeSize=sizeof(PageNode);
    //查询前清空fromSearch
    fromSearch.clear();
    char  searchColName[MAX_NAME_LEN]= {0};
    char searchColValue[MAX_NAME_LEN]= {0};
    PageNode*searchPageNode=((PageNode*)((void *)pageInfo+pageInfo->iFullPageNodeOffset*ipageNodeSize));
    //cout<<"  searchPageNode"<<searchPageNode<<"  ifull"<<pageInfo->iFullPageNodeOffset<<endl;
    //cout<<"  查询pageinfo"<<pageInfo<<"  iPagenodesize"<<ipageNodeSize<<" offset"<<pageInfo->iFullPageNodeOffset<<endl;
    //遍历每条记录 //((PageNode*)((void*)pageInfo+pageInfo->iPageSize))pageInfo->iFullPageNodeOffset
    for(searchPageNode;searchPageNode>0;searchPageNode=(PageNode*)((void*)pageInfo+ipageNodeSize*searchPageNode->iNextNode)){
      //  cout<<"  searchPageNode"<<searchPageNode<<endl;
        bool isFind=true;
        for(int i=0;i<vec[2].size();i+=3){
            int colOffset=0;
            char tempStr[MAX_NAME_LEN]= {0};
            //列名
            strcpy (searchColName,vec[2][i].c_str());
            //值
            strcpy (searchColValue,vec[2][i+2].c_str());
            for(int i=0; i<tablePtr->iColumnCounts; i++){
                if(strcmp(tablePtr->tColumn[i].sName,searchColName)!=0){
                    colOffset+=tablePtr->tColumn[i].iColumnLen;
                }
                else {
                     memcpy(tempStr,((void*)searchPageNode+(colOffset+pageNodeSize)),tablePtr->tColumn[i].iColumnLen);
                    break;
                }
            }
            if(strcmp(tempStr,searchColValue)!=0){
                isFind=false;
                break;
            } 
             
        }
        if(isFind){
            //cout<<"  找到，输出结果>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"<<endl;
            int offset=0;
            for(int i=0; i<tablePtr->iColumnCounts; i++)
            { 
            cout<<"  "<<(tablePtr->tColumn[i].sName)<<":"<<(char*)((void*)searchPageNode+pageNodeSize+offset)<<"  ";
            offset+=(tablePtr->tColumn[i].iColumnLen);
            }
            cout<<endl;
            fromSearch.push_back((PageNode*)searchPageNode);
        }
        if(searchPageNode->iNextNode==-1){
            break;
        }
    }
    if(fromSearch.size()>0){
        return fromSearch[fromSearch.size()-1];
    }
    else{
        return NULL;
    }
}

//页内插入，插入在在空链首页找空块插入数据 ！！！！！！！****插入要检查主键是否已存在****！！！！！！！
PageNode * PageHandle::pageInsert(SHMInfo* shmInfo,Table *  tablePtr,vector <vector <string> > &vec, int ipageNodeSize)       //找到该页空块链的第一块 ，插入数据
{
   
    PageInfo*tempPage= (PageInfo*)((void*)shmInfo+((tablePtr->iFreePageID)*(shmInfo->iChildSize)));  //找到空页地址
    PageNode*emptyPageNodePtr =  (PageNode*)((void*)tempPage+(tempPage->iEmptyPageNodeOffset)*ipageNodeSize) ;////找到空块地址
    PageNode* insertPageNodePtr =(PageNode*) ((void*)emptyPageNodePtr+sizeof(PageNode));//  找到空块地址+PageNode头大小=插入数据的具体地址
    //cout<<"  pageInfo"<<tempPage<<"  ipageNodeSize"<<ipageNodeSize<<"  offset"<<tempPage->iEmptyPageNodeOffset<<endl;
        //原插入未按照xml定义的顺序，空的应该空出来，buggggggggggggggg
       /* for(int m = 0,i = 0; m<vec[2].size(); m = m+3,i++)
        {
            char  tempStr[MAX_NAME_LEN]= {0};
            strcpy (tempStr,vec[2][m+2].c_str());
            cout<<"  插入到"<<insertPageNodePtr<<endl;
            //cout<<"  tempStr"<<tempStr<<endl;
            memcpy(insertPageNodePtr,tempStr, tablePtr->tColumn[i].iColumnLen);
            //char tiaoshi[MAX_NAME_LEN]={0};       
            //memcpy(tiaoshi,insertPageNodePtr,tablePtr->tColumn[i].iColumnLen);
            //cout<<"  tiaoshi "<<tiaoshi<<"    modeptr "<<insertPageNodePtr<<endl;
            
            insertPageNodePtr=(PageNode*)((void*)insertPageNodePtr+tablePtr->tColumn[i].iColumnLen);      
        }*/
      
        for(int i=0,m=0;i<tablePtr->iColumnCounts&&m<vec[2].size();++i,m+=3){
            char  tempStr[MAX_NAME_LEN]= {0};
            strcpy (tempStr,vec[2][m+2].c_str());
            //cout<<" tempStr"<<tempStr<<endl;
            char tempName[MAX_NAME_LEN]={0};
            strcpy(tempName,vec[2][m].c_str());
            //cout<<" tempName"<<tempName<<endl;
            int colIndex;
            //根据列名得到列号
            for(int n=0;n<tablePtr->iColumnCounts;++n){
                if(strcmp(tempName,tablePtr->tColumn[n].sName)==0){
                    colIndex=n;
                    break;
                }
            }
            for(int n=0;n<colIndex;++n){
                insertPageNodePtr=(PageNode*)((void*)insertPageNodePtr+tablePtr->tColumn[n].iColumnLen);
            }
            //cout<<"  插入地址"<<insertPageNodePtr<<"  值"<<tempStr<<" index"<<colIndex<<endl;
            memcpy(insertPageNodePtr,tempStr, tablePtr->tColumn[colIndex].iColumnLen);
            //插入完insertPageNodePtr置回
            insertPageNodePtr =(PageNode*) ((void*)emptyPageNodePtr+sizeof(PageNode));
        }  

        cout<<"  插入成功"<<endl;
        shmInfo->bitmap.set(tempPage->iPageID,1);//改位图
        tempPage-> iUsedPageNodeNum ++;//页已用块+1
        tablePtr->iCounts++;//表中记录数+1
        if( emptyPageNodePtr->iNextNode == -1)  //插入数据后就判断要分新页否，表的页链转化（空->满）
        {
            cout<<"  该页已用完，正重新申请空页"<<endl;
            tempPage->pageStateFlag= -1;//满页改状态
            if(shmInfo->iEmptyPageOffset == -1)
            {
                cout<<"  内存中已无可用空页"<<endl;
            }
            else
            {
                //插入节点达到满页时，页状态改变 空->满链上
                tempPage->iNextPageID= tablePtr->iFullPageID;//挪到表的满页链上
                tablePtr->iFullPageID= tempPage->iPageID;//插入表中满页链头
                tableHandleUtil.pageInit(shmInfo,tablePtr);//初始化新分配的页
                cout<<"  已重新分给该表一页，并完成初始化页"<<endl;
            }
        }
        else
        {   
            cout<<"  修改块链"<<endl;
            //插入数据后此页扔未满可用时
            tempPage->iEmptyPageNodeOffset = emptyPageNodePtr->iNextNode;//该页空偏指向下一块
            emptyPageNodePtr->iNextNode= tempPage->iFullPageNodeOffset;//挪到页的满块链上
            tempPage->iFullPageNodeOffset= emptyPageNodePtr->iPageNodeID;//插入页中满块链头
        }
         //返回插入的地址
        return emptyPageNodePtr;
    
}

//页内删除
PageNode * PageHandle::pageDelete(SHMInfo* shmInfo,Table *  tablePtr,vector <vector <string> > &vec, int ipageNodeSize,PageInfo*pageInfo)
{
    PageNode*delPageNode=NULL;
    void*searchPageNode=NULL;
    pageSearch( shmInfo, tablePtr,vec, ipageNodeSize,pageInfo);
    for(int i=0;i<fromSearch.size();++i){
        delPageNode=fromSearch[i];
        if(delPageNode!=NULL)
            {
        memset((void*)delPageNode+sizeof(PageNode),0,ipageNodeSize-sizeof(PageNode));//该块数据区置为0
        shmInfo->bitmap.set(pageInfo->iPageID,1);//改位图
        pageInfo-> iUsedPageNodeNum --;//页已用块-1
        tablePtr->iCounts -- ;//表中记录数-1
        //遍历页查找要删节点的上一个节点
        for(searchPageNode = ((void*)pageInfo+ipageNodeSize); searchPageNode<((void*)pageInfo+pageInfo->iPageSize); searchPageNode=searchPageNode+ipageNodeSize)
            {
            PageNode* searchPageNodeaddr=(PageNode*)searchPageNode;
            if(searchPageNodeaddr->iNextNode== delPageNode->iPageNodeID) //找到要删节点B的上一个节点A
            {
                searchPageNodeaddr->iNextNode=delPageNode->iNextNode;//  重连满块链（A->Bd的next）
                delPageNode->iNextNode=pageInfo->iEmptyPageNodeOffset;
                pageInfo->iEmptyPageNodeOffset=delPageNode->iPageNodeID;//该块插到空块链首
            }
            }
        if((double)(pageInfo-> iUsedPageNodeNum)/(pageInfo->iPageSize/ipageNodeSize)<0.8)  //阈值临时改为0.99方便测试
            {
            pageInfo->pageStateFlag=1;//满页变为可用页
            }
            }
    }
    return delPageNode;
}

//页内更新，只按单一属性查找，set一个属性，返回一个记录
PageNode * PageHandle::pageUpdate(SHMInfo* shmInfo,Table *  tablePtr,vector <vector <string> > &vec, int ipageNodeSize,PageInfo*pageInfo)
{
    PageNode*updatePageNode=NULL;
    pageSearch( shmInfo, tablePtr,vec, ipageNodeSize,pageInfo);
    for(int i=0;i<fromSearch.size();++i){
        updatePageNode=fromSearch[i];
        if(updatePageNode!=NULL)
        {   
            for(int i=0;i<vec[3].size();i+=3){
                char  setColName[MAX_NAME_LEN]= {0};
                char setColValue[MAX_NAME_LEN]= {0};
                strcpy (setColName,vec[3][i].c_str());//拿到字段的名字->定位到第几个字段
                strcpy (setColValue,vec[3][i+2].c_str());//拿到字段的值
                int setColOffset=sizeof(PageNode);
                for(int i=0; i<tablePtr->iColumnCounts; i++)
                {
                    if(strcmp(tablePtr->tColumn[i].sName,setColName)!=0)
                    {
                    setColOffset=setColOffset+tablePtr->tColumn[i].iColumnLen;//找到要更新的列位置   
                    }
                    else
                       {
                    memcpy((void*)updatePageNode+setColOffset,setColValue, tablePtr->tColumn[i].iColumnLen);
                    shmInfo->bitmap.set(pageInfo->iPageID,1);//改位图  
                    }
                }
            }
            
        }
    }
     return updatePageNode;  
}
