#ifndef _PAS_COMMON_H_
#define _PAS_COMMON_H_

#include <string.h>
#include <time.h>
#include <cctype>
#include <codecvt>
#include <locale>
#include <thread>

#if (WIN32)
#include <Windows.h>
#else
#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif
#endif

static int64_t getCurMilliTimeStamp()
{
#if (WIN32)
    return (int64_t)GetTickCount64();
#else
   struct timeval tv;
   gettimeofday(&tv, NULL);

   return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}

static unsigned long GetPID()
{
    unsigned long processId = 0;
#if (WIN32)
    processId = GetCurrentProcessId();
#else
#include <sys/types.h>
#include <unistd.h>
    processId = getpid();
#endif
    return processId;
}


static unsigned long GetThreadID()
{
    unsigned long tid = 0;
#if (WIN32)
    tid = ::GetCurrentThreadId();
#else
#include <sys/types.h>
#include <unistd.h>
    tid = gettid();
#endif
    return tid;
}
#endif //_PAS_COMMON_H_