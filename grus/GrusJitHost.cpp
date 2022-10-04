#include "GrusJitHost.h"
#include "jitlib.h"
#include "utility.h"
#include "jitclassproxy.h"
#include "jitmgr.h"
#include "PackEngine.h"
#include "jitclassinfo.h"
#include "BlockStream.h"

//each cpp file has to call this line
extern "C"
{
#include "numpy/ndarrayobject.h"
#include "numpy/arrayobject.h"
}


static void LoadNumpy()
{
	if (PyArray_API == NULL)
	{
		_import_array();
	}
}
#define SURE_NUMPY_API() LoadNumpy()

GrusJitHost::GrusJitHost()
{

}
GrusJitHost::~GrusJitHost()
{

}
int GrusJitHost::AddModule(void* context, const char* moduleName)
{
	JitLib* pLib = (JitLib*)context;
	return pLib->AddModule(moduleName);
}
void GrusJitHost::AddFunc(int moduleIndex,void* context, const char* hash, const char* funcName, void* funcPtr)
{
	JitLib* pLib = (JitLib*)context;
	pLib->AddFuncPtr(moduleIndex,funcName, hash,funcPtr);
}

void GrusJitHost::AddClass(int moduleIndex, void* context, const char* hash, 
	const char* className, int propNum, int methodNum,
	const char* classMemberNames[], int memberNum,
	const unsigned long long classStubFuncs[], int stubNum)
{
	std::vector<std::string> aryClassMemberNames(classMemberNames,
		classMemberNames+ memberNum);
	std::vector<unsigned long long> aryClassStubFuncs(classStubFuncs, classStubFuncs+ stubNum);
	JitLib* pLib = (JitLib*)context;
	pLib->AddClassPtr(moduleIndex, className, hash, propNum, methodNum, 
		aryClassMemberNames, aryClassStubFuncs);
}

int GrusJitHost::to_int(GalaxyJitPtr pVar)
{
	return (int)PyLong_AsLong((PyObject*)pVar);
}

GalaxyJitPtr GrusJitHost::from_int(int val)
{
	PyObject* pOb = PyLong_FromLong(val);
	return (GalaxyJitPtr)pOb;
}

long long GrusJitHost::to_longlong(GalaxyJitPtr pVar)
{
	return PyLong_AsLongLong((PyObject*)pVar);

}

GalaxyJitPtr GrusJitHost::from_longlong(long long val)
{
	PyObject* pOb = PyLong_FromLongLong(val);
	return (GalaxyJitPtr)pOb;
}

double GrusJitHost::to_double(GalaxyJitPtr pVar)
{
	return (double)PyFloat_AsDouble((PyObject*)pVar);
}

GalaxyJitPtr GrusJitHost::from_double(double val)
{
	return (GalaxyJitPtr)PyFloat_FromDouble((double)val);
}

bool GrusJitHost::IsNone(GalaxyJitPtr obj)
{
	return ((PyObject*)obj == Py_None);
}

float GrusJitHost::to_float(GalaxyJitPtr pVar)
{
	return (float)PyFloat_AsDouble((PyObject*)pVar);
}

GalaxyJitPtr GrusJitHost::from_float(float val)
{
	return (GalaxyJitPtr)PyFloat_FromDouble((double)val);
}

const char* GrusJitHost::to_str(GalaxyJitPtr pVar)
{
	std::string str =  ConvertFromObject((PyObject*)pVar);
	char* newStr = new char[str.size()+1];
	memcpy(newStr, str.c_str(), str.size()+1);
	return newStr;
}

GalaxyJitPtr GrusJitHost::from_str(const char* val)
{
	return (GalaxyJitPtr*)PyUnicode_FromString(val);
}

long long GrusJitHost::GetCount(GalaxyJitPtr objs)
{
	PyObject* pOb = (PyObject*)objs;
	if (pOb == nullptr)
	{
		return 0;
	}
	long long cnt = 1;
	if (PyTuple_Check(pOb))
	{
		cnt = PyTuple_GET_SIZE(pOb);
	}
	else if (PyDict_Check(pOb))
	{
		cnt = PyDict_GET_SIZE(pOb);
	}
	else if (PyList_Check(pOb))
	{
		cnt = PyList_GET_SIZE(pOb);
	}
	else if (PyByteArray_Check(pOb))
	{
		cnt = PyByteArray_Size(pOb);
	}
	return cnt;
}

GalaxyJitPtr GrusJitHost::Get(GalaxyJitPtr objs, int idx)
{
	PyObject* pOb = (PyObject*)objs;
	PyObject* pRetOb = Py_None;
	if (PyTuple_Check(pOb))
	{
		auto size = PyTuple_Size(pOb);
		if (idx < size)
		{
			pRetOb = PyTuple_GetItem(pOb, idx);
		}
	}
	else if (PyList_Check(pOb))
	{
		auto size = PyList_Size(pOb);
		if (idx < size)
		{
			pRetOb = PyList_GetItem(pOb, idx);
		}
	}
	else if (PyDict_Check(pOb))
	{
		PyObject* pObKey = PyLong_FromLong(idx);
		pRetOb = PyDict_GetItem(pOb, pObKey);
		Py_DecRef(pObKey);
	}
	//all come from Borrowed reference,so add one
	Py_IncRef(pRetOb);
	return (GalaxyJitPtr)pRetOb;
}

int GrusJitHost::Set(GalaxyJitPtr objs, int idx, GalaxyJitPtr val)
{	
	PyObject* pOb = (PyObject*)objs;
	int retVal = 0;
	if (PyList_Check(pOb))
	{
		retVal = PyList_SetItem(pOb, idx, (PyObject*)val);
	}
	else if (PyTuple_Check(pOb))
	{
		retVal = PyTuple_SetItem(pOb, idx, (PyObject*)val);
	}
	else if (PyDict_Check(pOb))
	{
		PyObject* pObKey = PyLong_FromLong(idx);
		retVal = PyDict_SetItem(pOb,pObKey, (PyObject*)val);
		Py_DecRef(pObKey);
	}
	return retVal;
}

void GrusJitHost::Free(const char* sz)
{
	delete sz;
}

GalaxyJitPtr GrusJitHost::Get(GalaxyJitPtr objs, const char* key)
{
	PyObject* pOb = (PyObject*)objs;
	PyObject* pRetOb = Py_None;
	if (pOb != nullptr && PyDict_Check(pOb))
	{
		PyObject* pObKey = PyUnicode_FromString(key);
		pRetOb = PyDict_GetItem(pOb, pObKey);//Borrowed reference
		Py_IncRef(pRetOb);
		Py_DecRef(pObKey);
	}
	else
	{
		if (strchr(key,'.') == nullptr)
		{
			if (pOb == nullptr)
			{
				pRetOb = PyImport_ImportModule(key);//New reference
			}
			else
			{
				pRetOb = PyObject_GetAttrString(pOb, key);//New reference
			}
		}
		else
		{
			std::string strKey(key);
			auto keys = split(strKey, '.');
			int i = 0;
			if (pOb)
			{
				Py_IncRef(pOb);
			}
			else
			{
				pOb = PyImport_ImportModule(keys[0].c_str());
				if (pOb == nullptr)
				{
					PyErr_PrintEx(0);
				}
				i = 1;
			}
			for (;i<(int)keys.size();i++)
			{
				auto k = keys[i];
				//New reference
				pRetOb = PyObject_GetAttrString(pOb, k.c_str());
				if (pRetOb != nullptr)
				{
					Py_DecRef(pOb);
					pOb = pRetOb;
				}
				else
				{
					Py_DecRef(pOb);
					break;
				}
			}
		}
	}
	if (pRetOb == nullptr)
	{
		pRetOb = Py_None;
		Py_IncRef(pRetOb);
	}
	return (GalaxyJitPtr)pRetOb;
}

GalaxyJitPtr GrusJitHost::Call(GalaxyJitPtr obj, int argNum, GalaxyJitPtr* args)
{
	PyObject* pRetOb = Py_None;
	PyObject* pCallOb = (PyObject*)obj;
	if (!PyCallable_Check(pCallOb))
	{
		Py_IncRef(pRetOb);
		return pRetOb;
	}
	PyObject* pObArgs = PyTuple_New(argNum);
	for (int i = 0; i < argNum; i++)
	{
		PyObject* arg = (PyObject*)args[i];//args already hold one refcount
		Py_IncRef(arg);
		PyTuple_SetItem(pObArgs,i, arg);
	}
	pRetOb = PyObject_CallObject(pCallOb, pObArgs);
	Py_DECREF(pObArgs);

	return (GalaxyJitPtr)pRetOb;
}
GalaxyJitPtr GrusJitHost::Call(GalaxyJitPtr obj, GalaxyJitPtr args, GalaxyJitPtr kwargs)
{
	PyObject* pRetOb = Py_None;
	PyObject* pCallOb = (PyObject*)obj;
	if (!PyCallable_Check(pCallOb))
	{
		Py_IncRef(pRetOb);
		return pRetOb;
	}
	pRetOb = PyObject_Call(pCallOb,(PyObject*)args, (PyObject*)kwargs);
	return (GalaxyJitPtr)pRetOb;
}
GalaxyJitPtr GrusJitHost::Call(GalaxyJitPtr obj, int argNum, GalaxyJitPtr* args, GalaxyJitPtr kwargs)
{
	PyObject* pRetOb = Py_None;
	PyObject* pCallOb = (PyObject*)obj;
	if (!PyCallable_Check(pCallOb))
	{
		Py_IncRef(pRetOb);
		return pRetOb;
	}
	PyObject* pObArgs = PyTuple_New(argNum);
	for (int i = 0; i < argNum; i++)
	{
		PyObject* arg = (PyObject*)args[i];//args already hold one refcount
		Py_IncRef(arg);//?
		PyTuple_SetItem(pObArgs, i, arg);
	}
	PyObject* pkwargs = (PyObject*)kwargs;

	pRetOb = PyObject_Call(pCallOb, pObArgs, pkwargs);
	Py_DECREF(pObArgs);

	return (GalaxyJitPtr)pRetOb;
}

bool GrusJitHost::ContainKey(GalaxyJitPtr container, GalaxyJitPtr key)
{
	bool bOK = false;
	if (PyDict_Check((PyObject*)container))
	{
		bOK = PyDict_Contains((PyObject*)container, (PyObject*)key);
	}
	return bOK;
}

bool GrusJitHost::KVSet(GalaxyJitPtr container, GalaxyJitPtr key, GalaxyJitPtr val)
{
	bool bOK = false;
	if (PyDict_Check((PyObject*)container))
	{
		bOK = PyDict_SetItem((PyObject*)container, (PyObject*)key, (PyObject*)val);
	}
	return bOK;
}
GalaxyJitPtr GrusJitHost::NewTuple(long long size)
{
	return (GalaxyJitPtr)PyTuple_New(size);
}
GalaxyJitPtr GrusJitHost::NewList(long long size)
{
	return (GalaxyJitPtr)PyList_New(size);
}

GalaxyJitPtr GrusJitHost::NewDict()
{
	return (GalaxyJitPtr)PyDict_New();
}

GalaxyJitPtr GrusJitHost::NewArray(int nd, unsigned long long* dims, int itemDataType)
{
	SURE_NUMPY_API();

	auto aryData = (PyArrayObject*)PyArray_SimpleNew(
		nd,
		(npy_intp *)dims,
		itemDataType);
	return (GalaxyJitPtr)aryData;
}

GalaxyJitPtr GrusJitHost::Import(const char* key)
{
	return (GalaxyJitPtr)PyImport_ImportModule(key);
}

void GrusJitHost::Release(GalaxyJitPtr obj)
{
	PyObject* pOb = (PyObject*)obj;
	Py_DecRef(pOb);
}

int GrusJitHost::AddRef(GalaxyJitPtr obj)
{
	PyObject* pOb = (PyObject*)obj;
	Py_IncRef(pOb);
	return (int)pOb->ob_refcnt;
}

void* GrusJitHost::GetDataPtr(GalaxyJitPtr obj)
{
	SURE_NUMPY_API();
	PyObject* pOb = (PyObject*)obj;
	if (PyArray_Check(pOb))
	{
		PyArrayObject* pArrayOb = (PyArrayObject*)pOb;
		return PyArray_DATA(pArrayOb);
	}
	else if (PyByteArray_Check(pOb))
	{
		return (void*)PyByteArray_AsString(pOb);
	}
	return nullptr;
}

bool GrusJitHost::GetDataDesc(GalaxyJitPtr obj,
	int& itemDataType, int& itemSize,
	std::vector<unsigned long long>& dims,
	std::vector<unsigned long long>& strides)
{
	SURE_NUMPY_API();
	PyObject* pOb = (PyObject*)obj;
	if (PyArray_Check(pOb))
	{
		PyArrayObject* pArrayOb = (PyArrayObject*)pOb;
		PyArray_Descr* pDesc = PyArray_DTYPE(pArrayOb);
		itemSize = (int)PyArray_ITEMSIZE(pArrayOb);
		itemDataType = pDesc->type_num;
		for (int i = 0; i < PyArray_NDIM(pArrayOb); i++)
		{
			dims.push_back(PyArray_DIM(pArrayOb,i));
			strides.push_back(PyArray_STRIDE(pArrayOb,i));
		}
		return true;
	}
	else
	{
		return false;
	}
}
bool GrusJitHost::IsDict(GalaxyJitPtr obj)
{
	return PyDict_Check((PyObject*)obj);
}
bool GrusJitHost::DictContain(GalaxyJitPtr dict,
	std::string& strKey)
{
	return PyDict_Check((PyObject*)dict) &&
		PyDict_Contains((PyObject*)dict, (PyObject*)PyJit::Object(strKey).ref());
}
bool GrusJitHost::IsArray(GalaxyJitPtr obj)
{
	SURE_NUMPY_API();
	return PyArray_Check((PyObject*)obj);
}
bool GrusJitHost::IsList(GalaxyJitPtr obj)
{
	return PyList_Check((PyObject*)obj);
}
GalaxyJitPtr GrusJitHost::GetDictKeys(GalaxyJitPtr obj)
{
	return PyDict_Keys((PyObject*)obj);
}

void* GrusJitHost::GetClassProxyNative(GalaxyJitPtr classProxyObj)
{
	PyJitClassProxy* pClassProxy = (PyJitClassProxy*)classProxyObj;
	return pClassProxy->classInstance;
}

GalaxyJitPtr GrusJitHost::QueryOrCreate(GalaxyJitPtr selfofcaller, 
	const char* class_name, void* pNativeObj)
{
	return JITManager::I().QueryOrCreateClassObject(selfofcaller, class_name, pNativeObj);
}

const char* GrusJitHost::GetObjectType(GalaxyJitPtr obj)
{
	PyObject* pOb = (PyObject*)obj;
	return Py_TYPE(pOb)->tp_name;
}

GalaxyJitPtr GrusJitHost::GetDictItems(GalaxyJitPtr dict)
{
	return PyDict_Items((PyObject*)dict);
}

bool GrusJitHost::StreamWrite(unsigned long long streamId, char* data, long long size)
{
	auto pStream = GetStream(streamId);
	if (pStream == nullptr)
	{
		return false;
	}
	return pStream->append(data, size);
}

bool GrusJitHost::StreamRead(unsigned long long streamId, char* data, long long size)
{
	auto pStream = GetStream(streamId);
	if (pStream == nullptr)
	{
		return false;
	}
	return pStream->CopyTo(data, size);
}

bool GrusJitHost::StreamWriteChar(unsigned long long streamId, char ch)
{
	auto pStream = GetStream(streamId);
	if (pStream == nullptr)
	{
		return false;
	}
	return pStream->appendchar(ch);
}

bool GrusJitHost::StreamReadChar(unsigned long long streamId, char& ch)
{
	auto pStream = GetStream(streamId);
	if (pStream == nullptr)
	{
		return false;
	}
	return pStream->fetchchar(ch);
}

bool GrusJitHost::StreamWriteString(unsigned long long streamId, std::string& str)
{
	auto pStream = GetStream(streamId);
	if (pStream == nullptr)
	{
		return false;
	}
	return pStream->append((char*)str.c_str(),str.size()+1);
}

bool GrusJitHost::StreamReadString(unsigned long long streamId, std::string& str)
{
	auto pStream = GetStream(streamId);
	if (pStream == nullptr)
	{
		return false;
	}
	return pStream->fetchstring(str);
}

GalaxyJitPtr GrusJitHost::GetPyNone()
{
	PyObject* pOb = Py_None;
	Py_IncRef(pOb);
	return pOb;
}

GalaxyJitPtr GrusJitHost::CreateJitObject(void* lib, 
	const char* moduleName, 
	const char* objTypeName,
	GalaxyJitPtr args)
{
	JitLib* pJitLib = (JitLib*)lib;
	std::string strModuleName(moduleName);
	std::string strObjTypeName(objTypeName);
	JitClassInfo* pClassInfo = pJitLib->FindClassFromNameSpace(
		strModuleName, strObjTypeName);
	if (pClassInfo)
	{
		return NewPyJitClassProxy(pClassInfo,
			pClassInfo->GetJitClassType()->JitClassProxyType,
			(PyObject*)args);
	}
	else
	{
		return nullptr;
	}
}

unsigned long long GrusJitHost::RegisterStream(GrusStream* pStream)
{
	unsigned long long key = (unsigned long long)pStream;
	m_streamLocker.Lock();
	auto it = m_runningStreams.find(key);
	if (it == m_runningStreams.end())
	{
		m_runningStreams.emplace(std::make_pair(key, pStream));
	}
	m_streamLocker.Unlock();
	return key;
}

void GrusJitHost::UnregisterStream(unsigned long long key)
{
	m_streamLocker.Lock();
	auto it = m_runningStreams.find(key);
	if (it != m_runningStreams.end())
	{
		m_runningStreams.erase(it);
	}
	m_streamLocker.Unlock();
}

GrusStream* GrusJitHost::GetStream(unsigned long long id)
{
	GrusStream* pStream = nullptr;
	m_streamLocker.Lock();
	auto it = m_runningStreams.find(id);
	if (it != m_runningStreams.end())
	{
		pStream = it->second;
	}
	m_streamLocker.Unlock();
	return pStream;
}

bool GrusJitHost::ParseModule(GalaxyJitPtr pModule)
{
	JitClassInfo* pClassInfo = new JitClassInfo();
	PyJit::Object objModule(pModule, true);
	std::string moduleName = (std::string)objModule["__name__"];
	std::string fingerprints;
	pClassInfo->BuildClassInfo((PyObject*)pModule, moduleName, fingerprints);

	return true;
}

bool GrusJitHost::PackTo(GalaxyJitPtr obj, JitStream* pStream)
{
	GrusStream streamProvider;
	streamProvider.SetProvider(pStream);
	PackEngine pe;
	std::vector<PyObject*> objs{ (PyObject*)obj };
	bool bOK = pe.Pack(streamProvider, objs);
	return bOK;
}

GalaxyJitPtr GrusJitHost::UnpackFrom(JitStream* pStream)
{
	PyObject* pOutOb = nullptr;
	GrusStream streamProvider;
	streamProvider.SetProvider(pStream);
	PackEngine pe;
	auto outList = pe.Unpack<PyObject*>(streamProvider);
	if (outList.size() > 0)
	{//only support one Object,but if the stream has more than one object, release here
		//to avoid memory leak
		pOutOb = outList[0];
		for (int i = 1; i < (int)outList.size(); i++)
		{
			if (outList[i])
			{
				Py_DecRef(outList[i]);
			}
		}
	}
	return GalaxyJitPtr(pOutOb);
}

GalaxyJitPtr GrusJitHost::Pack(std::vector<GalaxyJitPtr> objList)
{
	PyObject* pStreamOb = nullptr;
	BlockStream stream;
	PackEngine pe;
	bool bOK = pe.Pack(stream, objList);
	if (bOK)
	{
		STREAM_SIZE size = stream.Size();
		pStreamOb = PyByteArray_FromStringAndSize(nullptr, size);
		char* buf = PyByteArray_AsString(pStreamOb);//get the buffer from pStreamOb
		stream.FullCopyTo(buf, size);
	}

	return pStreamOb;
}

bool GrusJitHost::Unpack(GalaxyJitPtr byteArray, std::vector<GalaxyJitPtr>& objList)
{
	PyObject* pByteArray = (PyObject*)byteArray;
	STREAM_SIZE size = PyByteArray_Size(pByteArray);
	char* pData = PyByteArray_AsString(pByteArray);
	BlockStream stream(pData, size, false);
	PackEngine pe;
	objList = pe.Unpack<GalaxyJitPtr>(stream);
	return true;
}

GalaxyJitPtr GrusJitHost::CreateByteArray(const char* buf, long long size)
{
	return (GalaxyJitPtr)PyByteArray_FromStringAndSize(buf,size);
}
