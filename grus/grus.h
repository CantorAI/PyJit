#ifndef Grus_H
#define Grus_H

#if (WIN32)
#include <windows.h>
#define Grus_EXPORT __declspec(dllexport) 
#else
#define Grus_EXPORT
#endif


#endif // Grus_H
