#include "jitclassproxy.h"
#include "utility.h"
#include "jitclassinfo.h"


static PyObject*
JitClassProxy_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	PyJit::Object oargs(args);

	//pClassType is  Borrowed reference, no need to dec_ref
	JitClassType* pClassType = (JitClassType*)PyDict_GetItem(type->tp_dict, PyUnicode_FromString(add_key_class_type));
	JitClassInfo* pClassInfo = pClassType->classInfo;
	JitLib* pLib = pClassInfo->Lib();
	if (pClassInfo->CheckNeedBuild())
	{
		std::vector<std::string> out_files;
		pLib->GenAndBuild(out_files);
	}
	PyJitClassProxy* self = NewPyJitClassProxy(pClassInfo, type,args);
	//printf("JitClassProxy_new,class:%s,native:0x%x\n", self->classInfo->Name().c_str(), self->classInstance);

	return (PyObject*)self;
}
static int
JitClassProxy_init(PyJitClassProxy* self, PyObject* args, PyObject* kwds)
{
	return 0;
}
static void JitClassProxy_dealloc(PyJitClassProxy* self)
{
	JitClassInfo* pClassInfo = self->classInfo;
	//printf("JitClassProxy_dealloc,class:%s,native:0x%x\n", pClassInfo->Name().c_str(), self->classInstance);
	if (pClassInfo->DeallocStub() != nullptr)
	{
		typedef void (*CLASS_DEALLOC)(void* classInstance);
		CLASS_DEALLOC deallocFunc = (CLASS_DEALLOC)pClassInfo->DeallocStub();
		deallocFunc(self->classInstance);
		pClassInfo->RemoveInstance(self->classInstance);
		self->classInstance = nullptr;
	}
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyTypeObject JitClassProxyType = {
	PyObject_HEAD_INIT(&PyType_Type)
	"JitClassProxy",             /*tp_name*/
	sizeof(PyJitClassProxy),             /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)JitClassProxy_dealloc, /*tp_dealloc*/
	0,/*tp_vectorcall_offset*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_as_async*/

	0,                         /*tp_repr*/

	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/

	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/

	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
	"JitClassProxy objects",           /* tp_doc */

	0,		               /* tp_traverse */
	0,		               /* tp_clear */
	0,		               /* tp_richcompare */
	0,		               /* tp_weaklistoffset */
	0,		               /* tp_iter */
	0,		               /* tp_iternext */
	0,             /* tp_methods */
	0,             /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)JitClassProxy_init,      /* tp_init */
	0,                         /* tp_alloc */
	JitClassProxy_new,                 /* tp_new */
	0,//tp_free
	0,//tp_is_gc
	0,//tp_bases
	0,//tp_mro
	0,//tp_cache
	0,//tp_subclasses
	0,//tp_weaklist
	0,//tp_del
	0,//tp_version_tag
	0,//tp_finalize
	0,//tp_vectorcall
};

PyJitClassProxy* NewPyJitClassProxy(JitClassInfo* pClassInfo,PyTypeObject* type,PyObject* args)
{
	//Update Property Get/Set and Methods
	if (!pClassInfo->TypeMethodSetGet_Assigned())
	{
		int pid = 0;
		for (VarInfo& varInfo : pClassInfo->Props())
		{
			type->tp_getset[pid].get = (getter)varInfo.getter;
			type->tp_getset[pid].set = (setter)varInfo.setter;
			pid++;
		}
		int mid = 0;
		for (auto& funcInfo : pClassInfo->Funcs())
		{
			type->tp_methods[mid].ml_meth = (PyCFunction)funcInfo.stubfunc;
			mid++;
		}
		pClassInfo->setTypeMethodSetGet_Assigned(true);
	}
	auto self = (PyJitClassProxy*)type->tp_alloc(type, 0);
	self->classInfo = pClassInfo;
	if (pClassInfo->NewStub() != nullptr)
	{
		typedef void* (*CLASS_NEW)(GalaxyJitPtr self, GalaxyJitPtr vars);
		CLASS_NEW newclassFunc = (CLASS_NEW)pClassInfo->NewStub();
		self->classInstance = newclassFunc(self, args);
		pClassInfo->AddInstance(self->classInstance, self);
	}
	return self;
}

/**********************JitClassFactory*****************/

static void JitClassFactory_dealloc(JitClassType* self)
{
	if (self->JitClassProxyType)
	{
		if (self->JitClassProxyType->tp_getset)
		{
			delete[] self->JitClassProxyType->tp_getset;
			self->JitClassProxyType->tp_getset = nullptr;
		}
		if (self->JitClassProxyType->tp_methods)
		{
			delete[] self->JitClassProxyType->tp_methods;
			self->JitClassProxyType->tp_methods = nullptr;
		}
		delete self->JitClassProxyType;
		self->JitClassProxyType = nullptr;
	}
	if (self->WrapperFuncInfo)
	{
		delete self->WrapperFuncInfo;
		self->WrapperFuncInfo = nullptr;
	}
	if (self->classInfo)
	{
		delete self->classInfo;
		self->classInfo = nullptr;
	}
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyTypeObject JitClassFactoryType = {
	PyObject_HEAD_INIT(&PyType_Type)
	"JitClassFactory",             /*tp_name*/
	sizeof(JitClassType),             /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)JitClassFactory_dealloc, /*tp_dealloc*/
	0,/*tp_vectorcall_offset*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_as_async*/

	0,                         /*tp_repr*/

	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/

	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/

	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
	"JitClassFactory objects",           /* tp_doc */

	0,		               /* tp_traverse */
	0,		               /* tp_clear */
	0,		               /* tp_richcompare */
	0,		               /* tp_weaklistoffset */
	0,		               /* tp_iter */
	0,		               /* tp_iternext */
	0,             /* tp_methods */
	0,             /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	0,      /* tp_init */
	0,                         /* tp_alloc */
	0,                 /* tp_new */
	0,//tp_free
	0,//tp_is_gc
	0,//tp_bases
	0,//tp_mro
	0,//tp_cache
	0,//tp_subclasses
	0,//tp_weaklist
	0,//tp_del
	0,//tp_version_tag
	0,//tp_finalize
	0,//tp_vectorcall
};

static bool __JitClassFactoryType_Prepared = false;
void PrepareJitClassFactoryType()
{
	PyType_Ready(& JitClassFactoryType);
	__JitClassFactoryType_Prepared = true;
}

JitClassType* NewJitClassFactoryType()
{
	if (!__JitClassFactoryType_Prepared)
	{
		PrepareJitClassFactoryType();
	}
	JitClassType* pType = (JitClassType*)JitClassFactoryType.tp_alloc(&JitClassFactoryType, 0);
	PyTypeObject* proxyType = new PyTypeObject();
	pType->JitClassProxyType = proxyType;
	*pType->JitClassProxyType = JitClassProxyType;
	pType->WrapperFuncInfo = new JitFuncInfo();

	return pType;
}
static PyObject*
JitClass___reduce__(PyJitClassProxy* self)
{
	PyObject* pRet = PyTuple_New(0);
	return pRet;
}
static PyObject*
JitClass___getnewargs__(PyJitClassProxy* self)
{
	PyObject* pRet = PyTuple_New(0);
	//PyTuple_SetItem(pRet, 0, PyLong_FromLong(self->status));
	return pRet;
}

/* Un-pickle the object */
static PyObject*
JitClass___setstate__(PyJitClassProxy* self, PyObject* state)
{

	return PyLong_FromLong(1);
}

/* Pickle the object */
static PyObject*
JitClass___getstate__(PyJitClassProxy* self, PyObject* Py_UNUSED(ignored))
{
	/*PyObject* ret = Py_BuildValue("{sssisi}",
		"taskInstanceId", self->taskInstanceId.c_str(),
		"status", self->status,
		PICKLE_VERSION_KEY, PICKLE_VERSION);*/

		/*PyObject* ret = Py_BuildValue("{sisi}",
				"status", self->status,
				PICKLE_VERSION_KEY, PICKLE_VERSION);*/

				/*PyObject* ret = PyDict_New();
				PyDict_SetItem(ret,
					Py_BuildValue("s","taskInstanceId"),
					Py_BuildValue("s",self->taskInstanceId.c_str()));*/
	PyObject* ret = PyTuple_New(1);
	PyTuple_SetItem(ret, 0, PyLong_FromLong(1));
	return ret;
}
void BuildJitClass(JitClassType* pClassType, JitClassInfo* pClassInfo)
{
	//Props
	auto& props = pClassInfo->Props();
	int propCount = (int)props.size();
	if (propCount > 0)
	{
		PyGetSetDef* propDef = new PyGetSetDef[propCount + 1];
		int pid = 0;
		for (auto& varInfo : props)
		{
			auto& def = propDef[pid];
			def = { varInfo.name.c_str(),nullptr,nullptr,varInfo.name.c_str(),nullptr };
			pid++;
		}
		//NULL End
		propDef[propCount] ={ nullptr, nullptr,nullptr,nullptr,nullptr };
		pClassType->JitClassProxyType->tp_getset = propDef;
	}

	//Methods
	std::vector<ClassFuncInfo>& funcInfos = pClassInfo->Funcs();
	int funcCount = (int)funcInfos.size();
	if (funcCount > 0)
	{
		PyMethodDef* methods = new PyMethodDef[funcCount + 1];
		int mid = 0;
		for (ClassFuncInfo& funcInfo : funcInfos)
		{
			PyMethodDef& def = methods[mid];
			def.ml_name = funcInfo.name.c_str();//memory holds by funcInfos
			def.ml_meth = nullptr;
			def.ml_flags = METH_VARARGS;
			def.ml_doc = funcInfo.name.c_str();
			mid++;
		}
		//NULL End
		methods[funcCount] = { NULL, NULL, 0, NULL };
		pClassType->JitClassProxyType->tp_methods = methods;
	}

	pClassType->JitClassProxyType->tp_name = pClassInfo->Name().c_str();
	if (PyType_Ready(pClassType->JitClassProxyType) < 0)
	{
		printf("error\r\n");
	}
	Py_INCREF(pClassType);
	PyDict_SetItem(pClassType->JitClassProxyType->tp_dict, PyUnicode_FromString(add_key_class_type),(PyObject*)pClassType);
}