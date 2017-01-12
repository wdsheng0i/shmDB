#include "tinyxml.h"
#include <iostream>
#include <string>
#include <vector>
#include<stdlib.h>


using namespace std;

string xmls[]={"table.xml"};
vector<vector<vector<string> > > alltables;

//解析某个xml
bool loadXML(string xmlname)
{
    bool isExist=false;
    vector<vector<string> > table;
    vector<string> tablename;
    vector<string> tableid;
    vector<string> primarykey;
    TiXmlDocument doc;
    if(!doc.LoadFile(xmlname.c_str()))
    {
        cerr << doc.ErrorDesc() << endl;
        return false;
    }

    TiXmlElement* root = doc.FirstChildElement();
    if(root == NULL)
    {
        cerr << "Failed to load file: No root element." << endl;
        doc.Clear();
        return false;
    }

    for(TiXmlElement* elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
    {
        string elemName = elem->Value();//取节点名字
        //cout<<"遍历到节点"<<elemName<<endl;
        const char* attr;
        if(strcmp(elemName.c_str(),"table-name")==0)
        {
            attr = elem->Attribute("value");//取节点属性值
            //cout<<"表名"<<attr<<endl;
            for(int n=0;n<alltables.size();++n){
                if(strcmp(attr,alltables[n][0][0].c_str())==0){
                    isExist=true;
                } 
            }
            if(isExist){
                cerr<<"  失败，此表名已存在!!!"<<attr<<endl;
                break;
            }
            tablename.push_back(attr);
            table.push_back(tablename);
        }
         else if(strcmp(elemName.c_str(),"table-id")==0)
        {
        attr = elem->Attribute("value");//取节点属性值
        for(int n=0;n<alltables.size();++n){
            if(strcmp(attr,alltables[n][1][0].c_str())==0){
                  isExist=true;
               
            }
             if(isExist){
               cerr<<"  失败，此表id已存在!!!"<<attr<<endl;
                break;
            }
        }
        //cout<<"表id"<<attr<<endl;
        tableid.push_back(attr);
        table.push_back(tableid);
        }
         else if(strcmp(elemName.c_str(),"primary-key")==0)
        {
        attr = elem->Attribute("value");//取节点属性值
        //cout<<"zhujian"<<attr<<endl;
        primarykey.push_back(attr);
        table.push_back(primarykey);
        }

        else{
            vector<string> column;
            attr=elem->Attribute("name");
            column.push_back(attr);// cout<<attr<<endl;
            attr=elem->Attribute("column-pos");
            column.push_back(attr);//cout<<attr<<endl;
            attr=elem->Attribute("data-type");
            column.push_back(attr);//cout<<attr<<endl;
            attr=elem->Attribute("data-len");
            column.push_back(attr);//cout<<attr<<endl;
            table.push_back(column);
        }
    }
    doc.Clear();
    if(!isExist){
        alltables.push_back(table);
        return true;
    }
    return false;
}

//遍历解析数组中所有xml
bool loadallXML()
{
    int xmlslength=sizeof(xmls)/sizeof(xmls[0]);
    for(int i=0;i<xmlslength;++i){
        return loadXML(xmls[i]);
    }
}

