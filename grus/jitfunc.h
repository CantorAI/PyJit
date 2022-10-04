#ifndef _JIT_FUNC_H
#define _JIT_FUNC_H


#include <string>

#ifdef _DEBUG
#undef _DEBUG
extern "C"
{
#include "Python.h"
#include "numpy/ndarrayobject.h"
}
#define _DEBUG
#else
extern "C"
{
#include "Python.h"
#include "numpy/ndarrayobject.h"
}
#endif

PyTypeObject* GetJitFuncType();

class JitFuncInfo;
typedef struct _PyJitFunc_
{
	PyObject_HEAD;
	JitFuncInfo* funcInfo = nullptr;
} PyJitFunc;

PyJitFunc* NewPyJitFunc();
void DestroyPyJitFunc(PyJitFunc* self);

#endif // _JIT_FUNC_H
