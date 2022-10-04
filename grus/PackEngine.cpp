#include "PackEngine.h"
#include "Jit_Object.h"
#include "datetime.h"
#include "jitclassproxy.h"
#include "jitclassinfo.h"
#include "jitmgr.h"

typedef void (*SERIALIZE_STUB)(GalaxyJitPtr self, unsigned long long streamId, bool InputOrOutput);


PackEngine::PackEngine()
{
}

PackEngine::~PackEngine()
{
}

bool PackEngine::Pack(GrusStream& stream,std::vector<PyObject*>& pInObs)
{
	bool bOK = true;
	for (auto pObIn : pInObs)
	{
		PyJit::Object obj(pObIn, true);
		bOK = DoPack(stream, obj);
		obj.Empty();
		if (!bOK)
		{
			break;
		}
	}
	return bOK;
}
bool PackEngine::Pack(GrusStream& stream, std::vector<GalaxyJitPtr>& pInObs)
{
	bool bOK = true;
	for (auto pObIn : pInObs)
	{
		PyJit::Object obj(pObIn, true);
		bOK = DoPack(stream, obj);
		obj.Empty();
		if (!bOK)
		{
			break;
		}
	}
	return bOK;
}


bool PackEngine::DoPack(GrusStream& stream, PyJit::Object& obj)
{
	bool bRet = true;
	std::string objType = obj.GetType();
	if (objType == "NULL" || objType == "NoneType")
	{
		stream << 'N';
	}
	else if (objType == "dict")
	{
		stream << 'd';
		PyJit::Dict dict = obj;
		auto items = dict.ToList();
		auto item_cnt = items.GetCount();
		for (int i = 0; i <item_cnt; i++)
		{
			auto item = (PyJit::Object)items[i];
			PyJit::Object o_key = (PyJit::Object)item[0];
			PyJit::Object o_value = (PyJit::Object)item[1];
			bRet = DoPack(stream, o_key);
			if (!bRet)
			{
				break;
			}
			bRet = DoPack(stream, o_value);
			if (!bRet)
			{
				break;
			}
		}
		stream << endChar;
	}
	else if (objType == "tuple")
	{
		stream << 't';
		int cnt = (int)obj.GetCount();
		for (int i = 0; i < cnt; i++)
		{
			PyJit::Object item = (PyJit::Object)obj[i];
			bRet = DoPack(stream, item);
			if (!bRet)
			{
				break;
			}
		}
		stream << endChar;
	}
	else if (objType == "list")
	{
		stream << 'l';
		int cnt = (int)obj.GetCount();
		for (int i = 0; i < cnt; i++)
		{
			PyJit::Object item = (PyJit::Object)obj[i];
			bRet = DoPack(stream, item);
			if (!bRet)
			{
				break;
			}
		}
		stream << endChar;
	}
	else if (objType == "set")
	{
		stream << 'S';
		PyJit::Object builtins(PyEval_GetBuiltins(), true);
		PyJit::Object liOb = (PyJit::Object)builtins["list"](obj);
		int cnt = (int)liOb.GetCount();
		for (int i = 0; i < cnt; i++)
		{
			PyJit::Object item = (PyJit::Object)liOb[i];
			bRet = DoPack(stream, item);
			if (!bRet)
			{
				break;
			}
		}
		stream << endChar;
	}
	else if (objType == "frozenset")
	{
		stream << 'R';
		PyJit::Object builtins(PyEval_GetBuiltins(), true);
		PyJit::Object liOb = (PyJit::Object)builtins["list"](obj);
		int cnt = (int)liOb.GetCount();
		for (int i = 0; i < cnt; i++)
		{
			PyJit::Object item = (PyJit::Object)liOb[i];
			bRet = DoPack(stream, item);
			if (!bRet)
			{
				break;
			}
		}
		stream << endChar;
	}
	else if (objType == "range")
	{
		stream << 'r';
		PyJit::Object builtins(PyEval_GetBuiltins(),true);
		PyJit::Object func = builtins["getattr"];
		int start = (int)(PyJit::Object)func(obj,"start");
		int stop = (int)(PyJit::Object)func(obj,"stop");
		int step = (int)(PyJit::Object)func(obj,"step");
		stream << start;
		stream << stop;
		stream << step;
	}
	else if (objType == "bool")
	{
		stream << 'b';
		stream << (bool)obj;
	}
	else if (objType == "int")
	{
		stream << 'i';
		stream << (int)obj;
	}
	else if (objType == "float")
	{
		stream << 'f';
		stream << (double)obj;
	}
	else if (objType == "complex")
	{
		stream << 'c';
		PyObject* pOb = (PyObject*)obj.ref();
		Py_complex comp = PyComplex_AsCComplex(pOb);
		stream << comp.real;
		stream << comp.imag;
	}
	else if (objType == "str")
	{
		stream << 's';
		stream << (std::string)obj;
	}
	else if (objType == "bytes")
	{
		stream << 'B';
		long long size = PyBytes_Size((PyObject*)obj.ref());
		char* data = PyBytes_AsString((PyObject*)obj.ref());
		stream << size;
		stream.append(data, size);
	}
	else if (objType == "bytearray")
	{
		stream << 'A';
		long long size = PyByteArray_Size((PyObject*)obj.ref());
		char* data = PyByteArray_AsString((PyObject*)obj.ref());
		stream << size;
		stream.append(data, size);
	}
	else if (objType == "datetime.datetime")
	{
		int y = PyDateTime_GET_YEAR((PyObject*)obj.ref());
		int M = PyDateTime_GET_MONTH((PyObject*)obj.ref());
		int d = PyDateTime_GET_DAY((PyObject*)obj.ref());
		int h = PyDateTime_DATE_GET_HOUR((PyObject*)obj.ref());
		int m = PyDateTime_DATE_GET_MINUTE((PyObject*)obj.ref());
		int s = PyDateTime_DATE_GET_SECOND((PyObject*)obj.ref());
		int ms = PyDateTime_DATE_GET_MICROSECOND((PyObject*)obj.ref());
		char fold = PyDateTime_DATE_GET_FOLD((PyObject*)obj.ref());
		//PyObject* tz = PyDateTime_DATE_GET_TZINFO((PyObject*)obj.ref());
		stream << 'T';
		stream << y << M << d << h << m << s << ms << fold;
	}
	else if (objType == "numpy.ndarray")
	{
		bRet = DoNumpyPack(stream, obj);
	}
	else
	{
		bRet = DoCustomPack(objType, stream, obj);
	}
	return bRet;
}

bool PackEngine::DoUnpack(GrusStream& stream, PyJit::Object& obj, UnpackAction& act)
{
	bool bRet = true;
	char type = 0;
	stream >> type;
	if (type == endChar)
	{
		act = UnpackAction::back_to_parent;
		return true;
	}
	else if (type == '$')
	{
		bRet = DoCustomUnpack(stream, obj);
		return bRet;
	}
	else if (type == 'N')
	{
		obj.Empty();
		return true;
	}
	switch (type)
	{
	case 'd':
	{
		PyJit::Dict dict;
		while (true)
		{
			PyJit::Object key, value;
			UnpackAction act = UnpackAction::None;
			bRet = DoUnpack(stream, key, act);
			if (!bRet || act == UnpackAction::back_to_parent)
			{
				break;
			}
			act = UnpackAction::None;
			bRet = DoUnpack(stream, value, act);
			if (!bRet || act == UnpackAction::back_to_parent)
			{
				break;
			}
			if (!key.IsNull() && !value.IsNull())
			{
				PyDict_SetItem((PyObject*)dict.ref(),
					(PyObject*)(GalaxyJitPtr)key, 
					(PyObject*)(GalaxyJitPtr)value);
			}
		}
		obj = dict;
	}
	break;
	case 't':
	{
		std::vector<GalaxyJitPtr> store;
		while (true)
		{
			PyJit::Object item;
			UnpackAction act = UnpackAction::None;
			bRet = DoUnpack(stream, item, act);
			if (!bRet || act == UnpackAction::back_to_parent)
			{
				break;
			}
			if (!item.IsNull())
			{
				store.push_back((GalaxyJitPtr)item);
			}
		}
		if (bRet)
		{
			PyObject* tuple = PyTuple_New(store.size());
			for (int i = 0; i < (int)store.size(); i++)
			{
				PyTuple_SetItem(tuple, i, (PyObject*)store[i]);
			}
			obj = tuple;
		}
	}
	break;
	case 'l':
	{
		PyObject* list = PyList_New(0);
		while (true)
		{
			PyJit::Object item;
			UnpackAction act = UnpackAction::None;
			bRet = DoUnpack(stream, item, act);
			if (!bRet || act == UnpackAction::back_to_parent)
			{
				break;
			}
			if (!item.IsNull())
			{
				PyList_Append(list, (PyObject*)(GalaxyJitPtr)item);
			}
		}
		obj = list;
	}
	break;
	case 'S':
	{
		PyObject* set = PySet_New(0);
		while (true)
		{
			PyJit::Object item;
			UnpackAction act = UnpackAction::None;
			bRet = DoUnpack(stream, item, act);
			if (!bRet || act == UnpackAction::back_to_parent)
			{
				break;
			}
			if (!item.IsNull())
			{
				PySet_Add(set, (PyObject*)(GalaxyJitPtr)item);
			}
		}
		obj = set;
	}
	break;
	case 'R'://frozenset
	{
		PyObject* set = PySet_New(0);
		while (true)
		{
			PyJit::Object item;
			UnpackAction act = UnpackAction::None;
			bRet = DoUnpack(stream, item, act);
			if (!bRet || act == UnpackAction::back_to_parent)
			{
				break;
			}
			if (!item.IsNull())
			{
				PySet_Add(set, (PyObject*)(GalaxyJitPtr)item);
			}
		}
		PyJit::Object builtins(PyEval_GetBuiltins(), true);
		obj = builtins["frozenset"](set);
	}
	break;
	case 'r'://range
	{
		int start, stop, step;
		stream >> start;
		stream >> stop;
		stream >> step;
		PyJit::Object builtins(PyEval_GetBuiltins(), true);
		obj = builtins["range"](start, stop, step);
	}
	break;
	case 'b':
	{
		bool v = false;
		stream >> v;
		obj = v;
	}
	break;
	case 'i':
	{
		int v = 0;
		stream >> v;
		obj = v;
	}
	break;
	case 'f':
	{
		double v = 0;
		stream >> v;
		obj = v;
	}
	break;
	case 'c':
	{
		double real = 0;
		stream >> real;
		double mag = 0;
		stream >> mag;
		obj = PyComplex_FromDoubles(real, mag);
	}
	break;
	case 's':
	{
		std::string v;
		stream >> v;
		obj = v;
	}
	break;
	case 'B'://Bytes
	{
		long long size = 0;
		stream >> size;
		obj = PyBytes_FromStringAndSize(nullptr, size);
		char* buf = PyBytes_AsString((PyObject*)obj.ref());
		stream.CopyTo(buf, size);
	}
	break;
	case 'A'://bytearray
	{
		long long size = 0;
		stream >> size;
		obj = PyByteArray_FromStringAndSize(nullptr, size);
		char* buf = PyByteArray_AsString((PyObject*)obj.ref());
		stream.CopyTo(buf, size);
	}
	break;
	case 'T'://datetime
	{
		PyDateTime_IMPORT;
		int y, M, d, h, m, s, ms;
		char fold;
		stream >> y >> M >> d >> h >> m >> s >> ms >> fold;
		obj = PyDateTime_FromDateAndTimeAndFold(y, M, d, h, m, s, ms, fold);
	}
	break;
	case 'M'://numpy
		bRet = DoNumpyUnpack(stream, obj);
	break;
	default:
		break;
	}
	return bRet;
}

//$ is used to identify which is a customized Object
bool PackEngine::DoCustomPack(std::string& objType,
	GrusStream& stream, PyJit::Object& obj)
{
	const int online_len = 2000;
	char line[online_len];

	PyObject* pOb = (PyObject*)obj.ref();
	auto type = Py_TYPE(pOb);
	JitClassType* pClassType = (JitClassType*)PyDict_GetItem(type->tp_dict, PyUnicode_FromString(add_key_class_type));
	if (pClassType)
	{
		JitClassInfo* pClassInfo = pClassType->classInfo;
		if (pClassInfo->support_serialization())
		{
			void* func = pClassInfo->SerializStub();
			if (func)
			{
				SERIALIZE_STUB serialize_stub = (SERIALIZE_STUB)func;
				std::string nm = pClassInfo->GetFullNameSpace();
				std::string fullTypeName = "PyJit::" + nm + "." + objType;
				bool exist = false;
				int id = QueryOrCreateShortName(fullTypeName, exist);
				std::string outType;
				if (exist)
				{
					SPRINTF(line, online_len, "$=%d",id);
					outType = line;
				}
				else
				{
					SPRINTF(line, online_len, "$%s=%d", fullTypeName.c_str(),id);
					outType = line;
				}
				stream << outType;
				blockIndex objBlockSize_pos = stream.GetPos();
				STREAM_SIZE objBlockSize = 0;//will update after call serialize_stub
				stream << objBlockSize;
				STREAM_SIZE before_size = stream.CalcSize(stream.GetPos());
				serialize_stub((GalaxyJitPtr)obj, stream.GetKey(), false);
				if (stream.CanBeOverrideMode())
				{
					blockIndex afterPos = stream.GetPos();
					STREAM_SIZE after_size = stream.CalcSize(afterPos);
					objBlockSize = after_size - before_size;
					//update
					stream.SetOverrideMode(true);
					stream.SetPos(objBlockSize_pos);
					stream << objBlockSize;
					stream.SetOverrideMode(false);
					//move back to end of Stream
					stream.SetPos(afterPos);
				}
			}
		}
	}
	else
	{//Non-PyJit Object, use Pack Handler
		if (m_PackHandler)
		{
			bool exist = false;
			int id = QueryOrCreateShortName(objType, exist);
			std::string outType;
			if (exist)
			{
				SPRINTF(line, online_len, "$=%d", id);
				outType = line;
			}
			else
			{
				SPRINTF(line, online_len, "$%s=%d", objType.c_str(), id);
				outType = line;
			}
			stream << outType;
			blockIndex objBlockSize_pos = stream.GetPos();
			STREAM_SIZE objBlockSize = 0;//will update after call serialize_stub
			stream << objBlockSize;
			STREAM_SIZE before_size = stream.CalcSize(stream.GetPos());
			m_PackHandler->Pack(objType, stream, obj);
			if (stream.CanBeOverrideMode())
			{//update
				blockIndex afterPos = stream.GetPos();
				STREAM_SIZE after_size = stream.CalcSize(afterPos);
				objBlockSize = after_size - before_size;
				stream.SetOverrideMode(true);
				stream.SetPos(objBlockSize_pos);
				stream << objBlockSize;
				stream.SetOverrideMode(false);
				//move back to end of Stream
				stream.SetPos(afterPos);
			}
		}
	}
	return true;
}


int PackEngine::QueryOrCreateShortName(std::string& fullname, bool& exist)
{
	int id = 0;
	auto it = m_shortNameMap.find(fullname);
	if (it != m_shortNameMap.end())
	{
		id = it->second;
		exist = true;
	}
	else
	{
		id = (int)(m_shortNameMap.size()+1);//start from 1
		m_shortNameMap.emplace(std::make_pair(fullname, id));
		exist = false;
	}
	return id;
}

bool PackEngine::DoCustomUnpack(GrusStream& stream, PyJit::Object& obj)
{
	std::string objType;
	STREAM_SIZE objSize = 0;
	stream >> objType;
	stream >> objSize;
	if (objType.size() > 0 && objType[0] == '=')
	{
		int id = 0;
#if (WIN32)
		sscanf_s(objType.c_str(), "=%d",&id);
#else
		sscanf(objType.c_str(), "=%d", &id);
#endif
		auto it = m_shortNameFromIdMap.find(id);
		if (it != m_shortNameFromIdMap.end())
		{
			objType = it->second;
		}
	}
	else
	{
		auto pos = objType.rfind('=');
		if (pos != std::string::npos)
		{
			std::string strId = objType.substr(pos + 1);
			int id = 0;
#if (WIN32)
			sscanf_s(strId.c_str(), "%d", &id);
#else
			sscanf(strId.c_str(), "%d", &id);
#endif
			objType = objType.substr(0, pos);
			m_shortNameFromIdMap.emplace(std::make_pair(id, objType));
		}
	}
	auto pos = objType.find("PyJit::");
	if (pos != std::string::npos)
	{
		objType = objType.substr(pos + 7);
		JitClassInfo* pClassInfo = JITManager::I().FindClassFromNameSpace(objType);
		if (pClassInfo != nullptr && pClassInfo->support_serialization())
		{
			void* func = pClassInfo->SerializStub();
			if (func)
			{
				PyObject* args = PyTuple_New(0);
				PyJitClassProxy* self = NewPyJitClassProxy(pClassInfo,
					pClassInfo->GetJitClassType()->JitClassProxyType, args);
				Py_DecRef(args);
				SERIALIZE_STUB serialize_stub = (SERIALIZE_STUB)func;
				serialize_stub(self, stream.GetKey(), true);
				obj = self;
			}
		}
		else
		{//skip
			stream.Skip(objSize);
		}
	}
	else
	{
		if (m_PackHandler)
		{
			m_PackHandler->Unpack(objType, stream, obj);
		}
		else
		{
			stream.Skip(objSize);
		}
	}
	return true;
}

/*
	Using numpy.save and load from ByteIO to impl
*/
bool PackEngine::DoNumpyPack(GrusStream& stream, PyJit::Object& obj)
{
	PyJit::Object np = PyJit::Object::Import("numpy");
	PyJit::Object io = PyJit::Object::Import("io");
	PyJit::Object ioBuf = (PyJit::Object)io["BytesIO"]();
	np["save"](ioBuf, obj);
	PyJit::Object bytes = (PyJit::Object)ioBuf["getvalue"]();
	long long size = PyBytes_Size((PyObject*)bytes.ref());
	char* data = PyBytes_AsString((PyObject*)bytes.ref());
	stream << 'M';
	stream << size;
	stream.append(data, size);

	ioBuf["close"]();
	return true;
}
bool PackEngine::DoNumpyUnpack(GrusStream& stream, PyJit::Object& obj)
{
	long long size = 0;
	stream >> size;
	PyJit::Object bytes = PyBytes_FromStringAndSize(nullptr, size);
	char* buf = PyBytes_AsString((PyObject*)bytes.ref());
	stream.CopyTo(buf, size);

	PyJit::Object np = PyJit::Object::Import("numpy");
	PyJit::Object io = PyJit::Object::Import("io");
	PyJit::Object ioBuf = (PyJit::Object)io["BytesIO"](bytes);
	obj = np["load"](ioBuf);
	ioBuf["close"]();
	return true;
}
