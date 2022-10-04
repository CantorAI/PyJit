#ifndef __gthread_h__
#define __gthread_h__


class GThread
{
public:
    bool Start();
    virtual void Stop();
    void WaitToEnd();
public:
    virtual void run() =0;
private:
    void* mThreadHandle =nullptr;
};

#endif //!__gthread_h__