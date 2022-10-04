#ifndef RPCCALL_H
#define RPCCALL_H


class GalaxyRpcCall
{
public:
    virtual bool HostCreate(unsigned long long key,int InBufSize,int outBufSize) =0;
    virtual bool ClientConnect(int key,int InBufSize,int outBufSize, int timeoutMS) =0;
    virtual bool WriteIn(unsigned int index,
                 char* data,int size,
                 char* data2,int size2,
                 char* data3, int size3,
                 unsigned int callType)=0;
    virtual bool WriteOut(unsigned int index,
                  char* data,int size,
                  char* data2,int size2,
                  char* data3, int size3,
                  unsigned int callType)=0;
    virtual bool ReadIn(unsigned int& index, unsigned int& callType, char* pdata, int* pSize) = 0;
    virtual bool ReadOut(unsigned int& index, unsigned int& callType, char* pdata, int* pSize) = 0;
    virtual void Close() = 0;
    virtual void Reset() = 0;
};
#endif // RPCCALL_H
