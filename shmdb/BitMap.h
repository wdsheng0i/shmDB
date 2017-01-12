/*
    位图类，用于记录脏页供checkpoint扫描
*/

#ifndef BITMAP_H
#define BITMAP_H
#include <iostream>

using namespace std;
const int iShmSize = 128*1024*1024;     //共享内存大小
const int iPageSize = 256*64;       //页的大小
const int iMapSize = iShmSize/iPageSize;        //位图的大小
const int iMapCount = iMapSize/32;      //位图需要多少个unsigned int 型的数据来表

class BitMap
{
private:
    unsigned int MapArray[iMapCount];

public:
    BitMap();       //构造函数
    ~BitMap();      //析构函数

    bool JudgeRange(unsigned int site);
    int at(unsigned int site);      //返回第site位的值
    bool set(unsigned int site,unsigned int bit);       //将site位设置成bit值（bit为0或1）
    void show(unsigned int site);
    void showPage(unsigned int site);      //打印位图
    void showAll();
};

#endif // BITMAP_H

