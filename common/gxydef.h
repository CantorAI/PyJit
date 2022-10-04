#ifndef _GXY_DEF_H_
#define _GXY_DEF_H_


typedef void* PasWaitHandle;

#if (WIN32)
#include <Windows.h>
#define MS_SLEEP(t) Sleep(t)
#define US_SLEEP(t) Sleep(t/1000)
#else
#include <unistd.h>
#include <sys/time.h>
#define MS_SLEEP(t)  usleep((t)*1000)
#define US_SLEEP(t) usleep(t)
#endif


enum class DataFrameType
{
    Framework_Type_Begin = 1,


    Framework_Local_Rpc,
    Framework_Local_Rpc_Ack,

    Framework_Type_End,

    MAX_PASCALID = 10000,
};

#if (WIN32)
#define SPRINTF sprintf_s
#define Path_Sep_S "\\"
#define Path_Sep '\\'
#define ShareLibExt ".dll"
#else
#define SPRINTF snprintf
#define Path_Sep_S "/"
#define Path_Sep '/'
#define ShareLibExt ".so"
#endif

#endif //_GXY_DEF_H_