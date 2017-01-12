/*
    共享锁类的定义
*/
#ifndef TMUTEX_H
#define TMUTEX_H

class TMutex
{
public:
    TMutex();  //如果在构造的时候就初始化bFlag=true
    ~TMutex();

    int Create();             //创建锁
    int Destroy();            //销毁锁

    int Lock(bool bFlag);     //加锁，这里给个参数，如果不加锁Flag = false
    int UnLock(bool bFlag);   //解锁，这里给个参数，如果不加锁Flag = false
    //上面函数的非0返回值都是错误的，可以通过这个接口来获取错误原因
    const char* GetErrMsg(int iErrno);

private:
//        pthread_mutex_t     mutex;//????
//        pthread_mutexattr_t mattr;//???
    bool bIsCreate;
};

#endif // TMUTEX_H
