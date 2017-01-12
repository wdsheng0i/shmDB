#include <iostream>
#include "BitMap.h"
using namespace std;

BitMap::BitMap()
{
    int i = 0;
    while(i != iMapCount)
    {
        MapArray[i] = 0;
        ++i;
    }
}

BitMap::~BitMap()
{
}

bool BitMap::JudgeRange(unsigned int site)
{
    if((site<0)||(site>iMapSize-1))
    {
        cout << "Error!Beyond the range of bitmap!" <<endl;
        return true;
    }
    else
    {
        return false;
    }
}

int BitMap::at(unsigned int site)
{
    if(JudgeRange(site))
    {
        return -1;
    }
    unsigned int count = site/32;
    unsigned int result = MapArray[count] & (1 << (31-site));
    if(0 == result)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

bool BitMap::set(unsigned int site,unsigned int bit)
{
    if(JudgeRange(site))
    {
        return false;
    }
    unsigned int count = site/32;
    if(0 == bit)
    {
        MapArray[count] = MapArray[count] & (~(1 << (31-site)));
    }
    else
    {
        MapArray[count] = MapArray[count] | (1 << (31-site));
    }
    if(bit == at(site))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void BitMap::show(unsigned int site)
{
    if(JudgeRange(site))
    {
        return;
    }
    unsigned int i = 0;
    unsigned int count = site/32;
    cout << count*32 << "--" << count*32+31 << ':' << endl;
    while(i != 32)
    {
        if((i != 0)&&(0 == i%8))
        {
            cout << ' ';
        }
        cout << at(i+count*32);
        ++i;
    }
    cout << endl;
}

void BitMap::showPage(unsigned int site)
{
    cout << site << ':' << at(site) << endl;
}

void BitMap::showAll()
{
    int i = 0;
    while(i != iMapCount)
    {
        show(32*i);
        ++i;
    }
}
