#ifndef _JIT_CLASS_PROXY_H
#define _JIT_CLASS_PROXY_H


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

#define add_key_class_type "__pyjit_class__"


class JitFuncInfo;
class JitClassInfo;
class JitLib;
typedef struct _JitClassType_
{
	PyObject_HEAD;
	PyTypeObject* JitClassProxyType;
	JitFuncInfo* WrapperFuncInfo = nullptr;
	JitClassInfo* classInfo = nullptr;
	int ModuleIndex =0;
	JitLib* Lib = nullptr;
}JitClassType;

JitClassType* NewJitClassFactoryType();
void BuildJitClass(JitClassType* pClassType,JitClassInfo* pClassInfo);

typedef struct _PyJitClassProxy_
{
	PyObject_HEAD;
	JitClassInfo* classInfo = nullptr;
	void* classInstance = nullptr;
} PyJitClassProxy;

PyJitClassProxy* NewPyJitClassProxy(JitClassInfo* pClassInfo, PyTypeObject* type, PyObject* args);

#endif // _JIT_CLASS_PROXY_H
