#ifndef Grus_IMPL_H
#define Grus_IMPL_H

#include "grus.h"


//trick for win32 compile to avoid using pythonnn_d.lib
#ifdef _DEBUG
#undef _DEBUG
extern "C"
{
#include "Python.h"
}
#define _DEBUG
#else
extern "C"
{
#include "Python.h"
}
#endif
extern PyMethodDef RootMethods[];

#endif // Grus_IMPL_H
