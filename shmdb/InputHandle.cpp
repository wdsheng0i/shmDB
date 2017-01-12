#include "ShmHandle.h"
#include "DiskFileHandle.h"
#include"PageHandle.h"
#include"TableHandle.h"
#include <iostream>
#include <cstring>
#include <cctype>
#include<cstdio>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <cstdlib>
#include <unistd.h>

#define shmKey 31313
using namespace std;

#include <algorithm>
#include <functional>
#include "xml.h"

//using namespace std;
extern vector<vector<vector<string> > > alltables;
extern bool loadallXML();//xml.h的方法
extern vector<string>  getNameKey(string tablename);
//extern std::vector<Column> getColumns(string tablename);
int selectlen=6;//select的长度
int slength=11;//select*from的长度
int dlength=10;//deletefrom的长度
int ulength=6;//update的长度
int ilength=10;//insertinto的长度
int itable;//table在vector中的位置
string datatype="";//column 类型
int datalen=0;//column 长度
string number ="number";
//用户输入解析
vector<vector<string> > frominput;
//判断是否空格
bool iswhite(char ch)
{
    return isspace(ch);
}
//去除空格
string erasespace(string &str)
{
    str.erase(remove_if(str.begin(),str.end(),ptr_fun(iswhite)),str.end());
    return str;
}
//分割字符串,注意：当字符串为空时，也会返回一个空字符串
void mysplit(std::string& s, std::string& delim,std::vector< std::string >& ret)
{
    size_t last = 0;
    size_t index=s.find_first_of(delim,last);
    while (index!=std::string::npos)
    {
        ret.push_back(s.substr(last,index-last));
        last=index+1;
        index=s.find_first_of(delim,last);
    }
    if (index-last>0)
    {
        ret.push_back(s.substr(last,index-last));
    }
}
//判断表中是否存在此字段,存在就读取出字段类型
bool iscolumn(string column)
{
    for(int i=3; i<alltables[itable].size(); ++i)
    {
        if(column.compare(alltables[itable][i][0])==0)
        {
            datatype=alltables[itable][i][2];
            datalen=atoi(alltables[itable][i][3].c_str());
            return true;
        }
    }
    cerr<<"unknown column "<<column<<endl;
    throw 1;
    return false;
}
//判断是否是数字
bool isnumber(const char* cstr)
{
    if (NULL == cstr || cstr[0] == 0)
    {
        return false;
    }

    int len = strlen(cstr);
    int pos = 0;
    if (cstr[0] == '-' || cstr[0] == '+')
    {
        if (len <= 1)
        {
            return false;
        }
        pos++;
    }

    while (pos < len)
    {
        if (cstr[pos] < '0' || cstr[pos] > '9')
        {
            return false;
        }
        pos++;
    }
    return true;
}
//字段类型和长度校验
bool istype(string zhi)
{
    if(zhi.size()>datalen)
    {

        cerr<<zhi<<" is too long"<<endl;
        throw 1;
        return false;
    }
    if(datatype.compare(number)==0)
    {
        if(!isnumber(zhi.c_str()))
        {

            cerr<<zhi<<" is not number"<<endl;
            throw 1;
            return false;
        }
    }
    return true;
}


//处理分割后的字符串
void jxstr(string &str,vector<string> &v)
{
    int eq=str.find("=");
    int bt=str.find(">");
    int lt=str.find("<");
    int like=str.find("~");//模糊匹配~
    if(eq>-1)
    {
        string column=str.substr(0,eq);
        string zhi = str.substr(eq+1);
        if(!iscolumn(column))
        {
            return ;
        }
        if(!istype(zhi))
        {
            return;
        }
        v.push_back(column);
        v.push_back("=");
        v.push_back(zhi);
        /*cout<<".size "<<v.size()<<endl;
        cout<<column<<" "<<"= "<<zhi<<endl;*/
    }
    else if(bt>-1)
    {
        string column=str.substr(0,bt);
        string zhi = str.substr(bt+1);
        if(!iscolumn(column))
        {
            return ;
        }
        if(!istype(zhi))
        {
            return;
        }
        v.push_back(column);
        v.push_back(">");
        v.push_back(zhi);
        //cout<<column<<" "<<"> "<<zhi<<endl;

    }
    else if(lt>-1)
    {
        string column=str.substr(0,lt);
        string zhi = str.substr(lt+1);
        if(!iscolumn(column))
        {
            return ;
        }
        if(!istype(zhi))
        {
            return;
        }
        v.push_back(column);
        v.push_back("<");
        v.push_back(zhi);
        //cout<<column<<" "<<"< "<<zhi<<endl;

    }
    else if(like>-1)
    {
        string column=str.substr(0,like);
        string zhi = str.substr(like+1);
        if(!iscolumn(column))
        {
            return ;
        }
        if(!istype(zhi))
        {
            return;
        }
        v.push_back(column);
        v.push_back("~");
        v.push_back(zhi);
        //cout<<column<<" "<<"~ "<<zhi<<endl;

    }
    else
    {

        cerr<<"  error:input error"<<endl;
        throw 1;
        return;
    }
}

//解析(字段 条件 值 and)类型的字符串
void jxand(const string  &str1,vector<string> &v)
{
    string str=str1;//截取字符串,传过来的是const,转一下
    string strbegin;
    int i=str.find("and");
    while(i>-1)
    {
        strbegin=str.substr(0,i);
        str=str.substr(i+3);
        i=str.find("and");
        jxstr(strbegin,v);
    }
    jxstr(str,v);
}

//判断表存在否
bool istable(string &str)
{
    for(int i=0; i<alltables.size(); ++i)
    {
        if(str.compare(alltables[i][0][0])==0)
        {
            itable=i;
            return true;
        }
    }

    cerr<<"  unknown table "<<str<<endl;
    throw 1;
    return false;
}

/*//判断表名表id是否存在alltables中
//#include<stdlib.h>
*/


//解析
void jiexi( string &str,vector<vector<string> > &frominput)
{
    string table;
    int iwhere;//where在输入串的位置
    vector<string> tablename;
    vector<string> operation;
    vector<string> afterwhere;//存放where之后的条件
    vector<string> afterset;//存放set之后的条件
    iwhere=str.find("where");
    if(strcmp(str.substr(0,slength).c_str(),"select*from")==0)
    {
        table=str.substr(slength,iwhere-slength);//取得表名
        if(!istable(table))
        {
            return;
        }
        tablename.push_back(table);
        operation.push_back("select*");
    }
    else if(strcmp(str.substr(0,selectlen).c_str(),"select")==0)
    {
        int ifrom=str.find("from");
        table=str.substr(ifrom+4,iwhere-ifrom-4);//取得表名
        if(!istable(table))
        {
            return;
        }
        tablename.push_back(table);
        operation.push_back("select");
        string afterselect=str.substr(selectlen,ifrom-selectlen);
        string delimit=",";
        //将要展示字段存入afterset
        mysplit(afterselect,delimit,afterset);
        //校验是否存在此列名
        for (int i = 0; i < afterset.size(); ++i)
        {
            iscolumn(afterset[i]);
        }

    }
    else if(strcmp(str.substr(0,dlength).c_str(),"deletefrom")==0)
    {
        table=str.substr(dlength,iwhere-dlength);//取得表名
        if(!istable(table))
        {
            return;
        }
        tablename.push_back(table);
        operation.push_back("delete");
    }
    else if(strcmp(str.substr(0,ulength).c_str(),"update")==0)
    {
        table=str.substr(ulength,str.find("set")-ulength);//取得表名
        if(!istable(table))
        {
            return;
        }
        tablename.push_back(table);
        operation.push_back("update");
        //解析set之后where之前的字符串
        int set=str.find("set");
        jxand(str.substr(set+3,iwhere-set-3).c_str(),afterset);
    }
    else if(strcmp(str.substr(0,ilength).c_str(),"insertinto")==0)
    {
        table=str.substr(ilength,str.find("(")-ilength);//取得表名
        if(!istable(table))
        {
            return;
        }
        tablename.push_back(table);
        operation.push_back("insert");
        jxand(str.substr(str.find("(")+1).c_str(),afterwhere);
    }
    //desc
    else if(strcmp(str.substr(0,4).c_str(),"desc")==0)
    {
        table=str.substr(4,iwhere);
        if(!istable(table))
        {
            return;
        }
        tablename.push_back(table);
        operation.push_back("desc");
    }
    //drop
    else if(strcmp(str.substr(0,4).c_str(),"drop")==0)
    {
        table=str.substr(4,iwhere);
        if(!istable(table))
        {
            return;
        }
        tablename.push_back(table);
        operation.push_back("drop");
    }
    //create
    else if(strcmp(str.substr(0,6).c_str(),"create")==0)
    {
        operation.push_back("create");
    }
    else if(strcmp(str.substr(0,4).c_str(),"show")==0)
    {
        operation.push_back("show");
    }
    //showTableInfo
    else if(strcmp(str.substr(0,13).c_str(),"showTableInfo")==0)
    {
        table=str.substr(13,iwhere);
        if(!istable(table))
        {
            return;
        }
        tablename.push_back(table);
        operation.push_back("showTableInfo");
    }
    //showPageInfo
    else if(strcmp(str.substr(0,12).c_str(),"showPageInfo")==0)
    {
        string pageId=str.substr(12,iwhere);
        if(!isnumber(pageId.c_str()))
        {
            cerr<<pageId<<" is not a number"<<endl;
            return;
        }
        tablename.push_back(pageId);
        operation.push_back("showPageInfo");
    }
    //showShmInfo
    else if(strcmp(str.substr(0,11).c_str(),"showShmInfo")==0)
    {
        operation.push_back("showShmInfo");
    }
    else if(strcmp(str.substr(0,1).c_str(),"h")==0||strcmp(str.substr(0,4).c_str(),"help")==0)
    {
        cout<<"  show"<<endl;
        cout<<"  desc tablename"<<endl;
        cout<<"  drop tablename"<<endl;
        cout<<"  create"<<endl;
       /* cout<<"  showTableInfo tablename"<<endl;
        cout<<"  showPageInfo pageId"<<endl;
        cout<<"  showShmInfo"<<endl;*/
        cout<<"  select * from tablename where columnname = value and ..."<<endl;
        //cout<<"  select columnname , columnname , ...from tablename where ... "<<endl;
        cout<<"  update tablename set columnname = value and columnname =value where ..."<<endl;
        cout<<"  delete from tablename where columnname  = value and ..."<<endl;
        cout<<"  insert into tablename (columnname = value and ..."<<endl;
        throw 1;
        return;
    }
    else
    {
        cerr<<"  error:input error,wrong syntax"<<endl;
        cerr<<"  input h or help for help"<<endl;
        throw 1;
        return;
    }
    //先判断table，在判断where后的字段，之前出现bug
    if(iwhere>-1)//处理where之后的字符串
    {
        jxand(str.substr(iwhere+5).c_str(),afterwhere);
    }
    frominput.push_back(tablename);
    frominput.push_back(operation);
    frominput.push_back(afterwhere);
    frominput.push_back(afterset);
}

int main()
{
    cout<<"*****INFO:主进程启动*****"<<endl;
    ShmHandle shmUtil;
    DiskFileHandle diskFileUtil;
    PageHandle pageUtil;
    TableHandle tableUtil;
    cout<<"*****INFO:内存大小*****"<<shmUtil.shmInfo.iShmSize<<endl;
    cout<<"*****INFO:页大小*****"<<shmUtil.shmInfo.iChildSize<<endl;
    char fileName[40] = "ShmDBFile.fs";
    pid_t pid;

    shmUtil.shmCreat(fileName, shmUtil.shmInfo.iShmSize,shmKey );
    shmUtil.shmAttach(shmUtil.shmAddr,shmKey);
    shmUtil.showShmInfo(shmUtil.shmAddr);

    string input;//用户输入
    vector<vector<string> > frominput;   // 此处只提供保存解析值的结构体对象可以根据增删改查操作,自定义遍历
    //loadallXML();
    //tableUtil.createTable(shmUtil.shmAddr,alltables[0]);

    if((pid = fork()) < 0)
    {
        cout << "Fork is fault,please restart the program!" << endl;
        return 1;
    }
    //fork返回值是０，以下是子进程的运行代码，也就是CheckPoint模块
    void *ptr = shmUtil.shmAddr;
    SHMInfo *pointer = ( SHMInfo *) ptr;
    if(0 == pid)
    {
        while(1)
        {
            for(int i = 0; i != 8192; ++i)
            {
                if(1 == pointer->bitmap.at(i))
                {
                    //   cout<<"INFO: Checking Bitmap"<<endl;
                    //     pointer->bitmap.show(i);
                    pointer->bitmap.set(i,0);
                    diskFileUtil.diskFileWritePage(fileName,shmUtil.shmAddr,0,shmUtil.shmInfo.iChildSize);
                    diskFileUtil.diskFileWritePage(fileName,shmUtil.shmAddr,i,shmUtil.shmInfo.iChildSize);
                //    diskFileUtil.diskFileWrite(fileName,shmUtil.shmAddr,shmUtil.shmInfo.iChildSize);
                    if(i==0)
                    {
                        cout<<"INFO: CheckPoint 子进程退出"<<endl;
                        exit(EXIT_SUCCESS);
                    }
                }
            }
            sleep(5);
        }
    }
    //fork返回值大于０，以下是主进程的运行代码
    if(pid > 0)
    {
        while(1)
        {
            frominput.clear();//输入前,清空之前解析的数据
            cout<<"shmdb>>";
            getline(cin,input);
            erasespace(input);//去除所有空格
            try
            {
                jiexi(input,frominput);//解析时以对字符类型和长度进行校验

                char tempStr[MAX_NAME_LEN]= {0};
                strcpy (tempStr,frominput[1][0].c_str());//拿到操作（create/drop/insert/delete/update/select）
                if (strcmp(tempStr,"create")==0)
                {
                    bool isSuccess=loadallXML();
                    if(isSuccess){
                        tableUtil.createTable(shmUtil.shmAddr,alltables[alltables.size()-1]);//创建表
                    }
                }
                else
                {
                    tableUtil.option(shmUtil.shmAddr,frominput);//操作
                }
            }
            catch(int)
            {
                continue;//发生输入异常就不再执行下面操作
            }
        }
    }
    cout<<"INFO: BitMap第0位，置1，通知CheckPoint进程，写内存第0页并退出"<<endl;
    pointer->bitmap.set(0,1);
    shmUtil.shmDelAttach(shmUtil.shmAddr);//销毁测试
    return  0;
}
