#include "jitmgr.h"
#include "jitfunc.h"
#include "jitfuncinfo.h"
#include "jitclassproxy.h"
#include "jitclassinfo.h"
#include "jitlib.h"
#include "Jit_Object.h"
#include "GrusJitHost.h"
#include <iostream>
#include <fstream>
#include <sstream> 
#include "JSON.h"
#include "JSONValue.h"
#include <algorithm>
#include "md5.h"
#include "buildsystem.h"
#if !(WIN32)
#include <uuid/uuid.h>
#endif

JitHost* g_pHost = nullptr;

JITManager::JITManager()
{
	g_pHost = &GrusJitHost::I();
}

JITManager::~JITManager()
{

}

void* JITManager::QueryOrCreateClassObject(void* selfofcaller, 
	const char* class_name, void* pNativeObj)
{
	PyJitClassProxy* pOutProxy = nullptr;
	PyJitClassProxy* pSelfProxy = (PyJitClassProxy*)selfofcaller;
	JitClassInfo* classInfo = pSelfProxy->classInfo;
	if (classInfo && classInfo->Name() == class_name)
	{
		pOutProxy = classInfo->QueryInstance(pNativeObj);
	}
	else
	{
		if (classInfo)
		{
			JitLib* pLib = classInfo->Lib();

		}
	}
	if (pOutProxy)
	{
		Py_IncRef((PyObject*)pOutProxy);
	}
	return GalaxyJitPtr(pOutProxy);
}

bool JITManager::SearchVC_Compiler(std::string& vc_init_file)
{
	std::string vc_rootPath = "C:\\Program Files\\Microsoft Visual Studio";
	bool bFind = SearchVC_CompilerWithPath(vc_rootPath, vc_init_file);
	if(!bFind)
	{
		vc_rootPath = "C:\\Program Files (x86)\\Microsoft Visual Studio";
		bFind = SearchVC_CompilerWithPath(vc_rootPath, vc_init_file);
	}
	return bFind;
}
bool JITManager::SearchVC_CompilerWithPath(std::string root,std::string& vc_init_file)
{
	std::vector<std::string> subfolders;
	std::vector<std::string> files;
	dir(root, subfolders, files);
	std::vector<int> versions;
	for (auto i : subfolders)
	{
		auto v = std::stoi(i);
		if (v >= 2017 && v <= 3000)
		{
			versions.push_back(v);
		}
	}
	std::sort(versions.begin(), versions.end());
	subfolders.clear();
	files.clear();
	//search from highest version
	bool bFind = false;
	for (int i = (int)versions.size() - 1; i >= 0; i--)
	{
		std::string path_version = root + "\\" + std::to_string(versions[i]);
		dir(path_version,subfolders, files);
		for (auto j : subfolders)
		{
			std::string search_file = path_version + "\\" + j+"\\VC\\Auxiliary\\Build\\vcvarsall.bat";
			if (exists(search_file))
			{
				vc_init_file = search_file;
				bFind = true;
				break;
			}
		}
		if (bFind)
		{
			break;
		}
	}
	return bFind;
}
void JITManager::Init()
{
#if (WIN32) && (USING_MSVC)
	std::string json_config_file = mModuleRootFolder + "/Config.json";
	std::wifstream configFile(json_config_file);
	std::wstring strConfig;
	if (configFile.is_open())
	{
		std::wstringstream strStream;
		strStream << configFile.rdbuf();
		configFile.close();
		strConfig = strStream.str();
	}
	else
	{
		strConfig = L"{}";
	}
	bool bFind_Compiler_Path = false;
	JSONValue* valRoot = JSON::Parse(strConfig.c_str());
	if (valRoot != nullptr && valRoot->IsObject())
	{
		auto root = valRoot->AsObject();
		if (root.find(L"Compiler_Path") != root.end()
			&& root[L"Compiler_Path"]->IsString())
		{
			mCompiler_Path = ws2s(root[L"Compiler_Path"]->AsString());
			bFind_Compiler_Path = true;
		}
	}
	if (!bFind_Compiler_Path)
	{
		std::string vc_init_file;
		bFind_Compiler_Path = SearchVC_Compiler(vc_init_file);
		if (bFind_Compiler_Path)
		{
			mCompiler_Path = vc_init_file;
			if (valRoot != nullptr && valRoot->IsObject())
			{
				auto root = valRoot->AsObject();
				root[L"Compiler_Path"] = new JSONValue(s2ws(mCompiler_Path));
				auto val = new JSONValue(root);
				std::wstring wstrJson = val->Stringify(true);
				std::string strJson = ws2s(wstrJson);
				std::ofstream file(json_config_file);
				if (file.is_open())
				{
					file.write((const char*)strJson.c_str(), strJson.length() * sizeof(char));
					file.close();
				}
			}
		}
	}
	if (valRoot)
	{
		delete valRoot;
	}
#endif
}

//real function call from python side
static PyObject*
JitProxy(PyObject* self, PyObject* args, PyObject* kwargs)
{
	PyJitFunc* pJitFunc = (PyJitFunc*)self;
	return pJitFunc->funcInfo->Call(args,kwargs);
}

static PyObject*
JitWrapper(PyObject* self, PyObject* args)
{
	PyJitFunc* pJitFunc = (PyJitFunc*)self;
	PyObject* obFunc = PyTuple_GetItem(args, 0);
	JitFuncInfo* pFuncInfo = pJitFunc->funcInfo;
	try
	{
		PyJit::Object funcObj(obFunc);

		std::string funcName;
		ClassFuncInfo funcInfo;
		std::string funcBody;

		funcName = (std::string)funcObj["__name__"];
		std::string wholecode = (std::string)PyJit::Object()["inspect.getsource"](funcObj);
		pFuncInfo->ParseFuncInfo(wholecode, funcInfo,funcBody);
		pFuncInfo->SetFuncHead(funcInfo);
		std::string wholespec;
		pFuncInfo->BuildRelativeFileSpec(wholespec);
		ParsePythonFunctionCode(wholecode, funcBody);
		wholespec += wholecode;
		std::string hash = md5(wholespec);

		pFuncInfo->SetCode(funcBody);
		pFuncInfo->SetName(funcName);
		pFuncInfo->SetHash(hash);
	}
	catch (...)
	{
		PyErr_Print();
	}

	static PyMethodDef def{ "jit_proxy",
		(PyCFunction)JitProxy, METH_VARARGS | METH_KEYWORDS,
		"proxy for jit function" };

	Py_INCREF(self);
	PyObject* retFunc = PyCFunction_New(&def, self);

	return retFunc;
}

void JITManager::ParseKWArgs(JitLib* pLib, LangType langType,JitFuncInfo* funcInfo, PyObject* kwargs)
{
	const char* libBase_Var_Debug = "debug";
	const char* libBase_Var_Include = "include";
	const char* class_name_tag = "classname";
	const char* include_dir_tag = "include_dir";
	const char* lib_dir_tag = "lib_dir";

	PyObject* key, * value;
	Py_ssize_t pos = 0;
	while (PyDict_Next(kwargs, &pos, &key, &value))
	{
		std::string strKey = PyUnicode_AsUTF8(key);
		std::string strKeyLower = strKey;
		std::transform(strKeyLower.begin(),
			strKeyLower.end(), strKeyLower.begin(), ::tolower);
		if (strKeyLower.compare(libBase_Var_Debug) == 0)
		{
			bool bDebug = PyObject_IsTrue(value);
			pLib->SetDebug(bDebug);
			continue;
		}
		else if (strKeyLower.compare(class_name_tag) == 0)
		{
			std::string className = (std::string)PyJit::Object(value, true);
			funcInfo->SetClassName(className);
			continue;
		}
		else if (strKeyLower.compare(include_dir_tag) == 0)
		{
			if (PyList_Check(value))
			{
				auto dirs = PyJit::Object(value, true);
				int cnt = (int)dirs.GetCount();
				for (int i = 0; i < cnt; i++)
				{
					std::string dir = (std::string)dirs[i];
					pLib->AddIncludeDir(dir);
				}
			}
			else if (PyUnicode_Check(value))
			{
				std::string dir = (std::string)PyJit::Object(value, true);
				pLib->AddIncludeDir(dir);
			}
			continue;
		}
		else if (strKeyLower.compare(libBase_Var_Include) == 0)
		{
			if (PyList_Check(value))
			{
				auto headers = PyJit::Object(value,true);
				int cnt = (int)headers.GetCount();
				for (int i = 0; i < cnt; i++)
				{
					std::string header = (std::string)headers[i];
					funcInfo->AddInclude(header);
					pLib->AddIncludeFile(langType, header);
				}
			}
			else if (PyUnicode_Check(value))
			{
				auto oheader = PyJit::Object(value,true);
				std::string header = (std::string)oheader;
				funcInfo->AddInclude(header);
				pLib->AddIncludeFile(langType, header);
			}
			continue;
		}
		funcInfo->AddCfg(strKey, PyJit::Object(value,true));
	}
}
void JITManager::ParsePyModuleInfo(PyObject* kwargs,std::string& strModuleFileName,
	std::string& strLangType, std::string& strLibName)
{
	const char* langKeyTag = "lang";
	const char* libNameTag = "libname";

	strLangType = "cpp";//default is cpp
	if (kwargs != nullptr)
	{
		PyObject* key, * value;
		Py_ssize_t pos = 0;
		while (PyDict_Next(kwargs, &pos, &key, &value))
		{
			std::string strKey = PyUnicode_AsUTF8(key);
			std::string strKeyLower = strKey;
			std::transform(strKeyLower.begin(),
				strKeyLower.end(), strKeyLower.begin(), ::tolower);
			if (strKeyLower.compare(langKeyTag) == 0)
			{
				strLangType = (std::string)PyJit::Object(value);
				continue;
			}
			else if (strKeyLower.compare(libNameTag) == 0)
			{
				strLibName = (std::string)PyJit::Object(value);
			}
		}
	}
	PyJit::Object locals(PyEval_GetLocals(), true);
	PyJit::Object thisFile = (PyJit::Object)locals["__file__"];
	strModuleFileName = (std::string)PyJit::Object()["os.path.abspath"](thisFile);
}
PyObject* JITManager::CreateFuncWrapper(PyObject* args, PyObject* kwargs)
{
	std::string strModuleFileName;
	std::string strLangType;
	std::string strLibName;
	ParsePyModuleInfo(kwargs, strModuleFileName, strLangType, strLibName);

	int nModuleIndex = 0;
	JitLib* pLib = GetOrCreateLib(true,strModuleFileName, strLibName ,&nModuleIndex);

	PyJitFunc* pJitFunc = NewPyJitFunc();
	pJitFunc->funcInfo->SetLib(pLib);
	pJitFunc->funcInfo->SetModuleIndex(nModuleIndex);
	LangType langType = pJitFunc->funcInfo->SetLang(strLangType);
	
	pLib->LoadLib();

	if (kwargs != nullptr)
	{
		ParseKWArgs(pLib, langType, pJitFunc->funcInfo, kwargs);
	}

	PyObject* retFunc = nullptr;
	static PyMethodDef def{ "JitWrapper",
		JitWrapper, METH_VARARGS,
		"JitWrapper" };

	retFunc = PyCFunction_New(&def, (PyObject*)pJitFunc);
	return retFunc;
}

static PyObject*
JitWrapper_Class(PyObject* self, PyObject* args)
{
	JitClassType* pJitClassType = (JitClassType*)self;
	PyObject* obClass = PyTuple_GetItem(args, 0);
	PyJit::Object classObj(obClass,true);
	std::string className = (std::string)classObj["__name__"];

	JitClassInfo* pClassInfo = nullptr;
	JitLib* pLib = pJitClassType->Lib;
	int nModuleIndex = pJitClassType->ModuleIndex;
	pClassInfo =(JitClassInfo*)pLib->QueryFuncOrClassInfo(nModuleIndex, className);
	if (pClassInfo == nullptr)
	{
		pClassInfo = new JitClassInfo();
		pClassInfo->SetLib(pLib);
		pClassInfo->SetModuleIndex(nModuleIndex);
	}
	pClassInfo->CopyNecessaryPropsFromWrapperFuncInfo(pJitClassType->WrapperFuncInfo);
	pClassInfo->SetJitClassType(pJitClassType);
	pJitClassType->classInfo = pClassInfo;
	try
	{
		std::string fingerprints;
		pClassInfo->BuildRelativeFileSpec(fingerprints);
		/*
			we used inspect's getsource to get class code, but
			failed, also will cause refcount issue here for classObj,
			so change to use combine all information from BuildClassInfo
			to calc hash
		*/
		//build class
		pClassInfo->BuildClassInfo(obClass, className, fingerprints);
		std::string hash = md5(fingerprints);
		//calc hash first to make sure if code changed or not
		pClassInfo->SetHash(hash);
	}
	catch (...)
	{
		PyErr_Print();
	}
	pClassInfo->SetName(className);
	
	BuildJitClass(pJitClassType, pClassInfo);

	pLib->LoadLib();

	PyObject* jitClassProxyType = (PyObject*)pJitClassType->JitClassProxyType;
	Py_INCREF(jitClassProxyType);
	PyModule_AddObject(JITManager::I().GetThisModule(), className.c_str(), jitClassProxyType);

	Py_INCREF(jitClassProxyType);
	return jitClassProxyType;

}

PyObject* JITManager::Register(PyObject* args, PyObject* kwargs)
{
	PyJit::Object oArgs(args, true);
	std::string strModuleFileName;
	if (oArgs.GetCount() >= 1)
	{
		strModuleFileName = (std::string)oArgs[0];
	}
	std::string strLangType;
	std::string strLibName;

	JitLib* pLib = GetOrCreateLib(false, strModuleFileName, strLibName);
	if (pLib)
	{
		pLib->SetHaveCppFunc(true,strModuleFileName);
		pLib->LoadLib();
	}

	unsigned long long ptr = (unsigned long long)(void*)g_pHost;
	unsigned long long lib = (unsigned long long)(void*)pLib;
	PyObject* pRetOb = PyTuple_New(2);
	PyTuple_SetItem(pRetOb, 0, PyLong_FromLongLong(ptr));
	PyTuple_SetItem(pRetOb, 1, PyLong_FromLongLong(lib));
	return pRetOb;
}

PyObject* JITManager::CreateClassWrapper(PyObject* args, PyObject* kwargs)
{
	std::string strModuleFileName;
	std::string strLangType;
	std::string strLibName;
	ParsePyModuleInfo(kwargs, strModuleFileName, strLangType, strLibName);

	int nModuleIndex = 0;
	JitLib* pLib = GetOrCreateLib(true, strModuleFileName, strLibName, &nModuleIndex);

	JitClassType* pJitClassType = NewJitClassFactoryType();
	pJitClassType->ModuleIndex = nModuleIndex;
	pJitClassType->Lib = pLib;
	JitFuncInfo* pWrapperFuncInfo = pJitClassType->WrapperFuncInfo;
	pWrapperFuncInfo->SetLib(pLib);
	LangType langType = pWrapperFuncInfo->SetLang(strLangType);

	if (kwargs != nullptr)
	{
		ParseKWArgs(pLib, langType, pWrapperFuncInfo, kwargs);
	}

	PyObject* retFunc = nullptr;

	static PyMethodDef def_class{ "JitWrapperClass",
		JitWrapper_Class, METH_VARARGS,
		"JitWrapperClass" };

	retFunc = PyCFunction_New(&def_class, (PyObject*)pJitClassType);
	return retFunc;
}

PyObject* JITManager::Generate(PyObject* args, PyObject* kwargs)
{
#if 0
	DWORD procId = GetCurrentProcessId();
	char info[1000];
	sprintf_s(info, 1000, "Attach to python process %d to debug,"
		"after attach,don't forget to close this Message Box"
		"to make program continue!", procId);
	MessageBox(NULL, info, "PyJit Native Debug", MB_OK);
#endif
	PyJit::Object os = PyJit::Object::Import("os");
	std::string curModuleFolder = GetCallerPyModuleFolder();
	const char* outPath = nullptr;
	std::string outputFolder;//keep here to hold memory for 
	if (kwargs)
	{
		PyJit::Object owargs(kwargs, true);
		PyJit::Object objOutputFolder = owargs["out"];
		outputFolder = (std::string)objOutputFolder;
		outputFolder = curModuleFolder + outputFolder;
		bool bExsits = (bool)os["path.isdir"](outputFolder);
		if (!bExsits)
		{
			os["makedirs"](outputFolder);
		}
		outPath = outputFolder.c_str();
	}
	std::vector<std::string> PyFiles;
	PyJit::Object oArgs(args,true);
	if (oArgs.GetCount() >= 1)
	{
		PyJit::Object src = (PyJit::Object)oArgs[0];
		if (src.IsList())
		{
			int cnt = (int)src.GetCount();
			for (int i = 0; i < cnt; i++)
			{
				std::string src_item = (std::string)src[i];
				PyFiles.push_back(src_item);
			}
		}
		else
		{
			std::string strPyFiles = (std::string)src;
			PyFiles = split(strPyFiles, ';');
		}
		PyFiles = BuildSystem::I().ParseFolder(curModuleFolder,PyFiles);
	}
	PyJit::Object sys = PyJit::Object::Import("sys");
	std::string cwd = (std::string)os["getcwd"]();
	//change
	os["chdir"](curModuleFolder);
	std::vector<std::string> out_files;
	for (auto file : PyFiles)
	{
		PyJit::Object fullPath = os["path.abspath"](file);
		std::string strModuleFileName = (std::string)fullPath;
		std::string baseName = (std::string)os["path.basename"](fullPath);
		auto pos = baseName.rfind('.');
		if (pos != baseName.npos)
		{
			baseName = baseName.substr(0, pos);
		}
		PyJit::Object folderName = os["path.dirname"](fullPath);
		sys["path.insert"](0, folderName);
		PyJit::Object::Import(baseName.c_str());
		sys["path.remove"](folderName);
		JitLib* pLib = GetOrCreateLib(true,strModuleFileName, "");
		if (pLib)
		{
			std::vector<std::string> files0;
			pLib->GenAndBuild(files0,true, outPath);
			for (auto it = files0.begin(); it != files0.end(); it++)
			{
				std::string outFile = *it;
				//outFile = GetRelativePath(outFile, curModuleFolder);
				out_files.push_back(outFile);
			}
		}
	}
	//restore
	os["chdir"](cwd);
	return (PyObject*)(GalaxyJitPtr)PyJit::Object(out_files);
}

PyObject* JITManager::Generate_UID(PyObject* args, PyObject* kwargs)
{
	std::string strGuid;
#if (WIN32)
	GUID gid;
	CoCreateGuid(&gid);
	char szGuid[128];
	sprintf_s(szGuid, "%08X%04X%04X%X%X%X%X%X%X%X%X",
		gid.Data1, gid.Data2, gid.Data3,
		gid.Data4[0], gid.Data4[1], gid.Data4[2], gid.Data4[3],
		gid.Data4[4], gid.Data4[5], gid.Data4[6], gid.Data4[7]);
	//StringFromGUID2(gid, wGuid, 128);
	//std::wstring wstrGuid(wGuid);
	strGuid = szGuid;// ws2s(wstrGuid);
#else
	uuid_t uuid;
	uuid_generate_time_safe(uuid);
	char szGuid[128];
	uuid_unparse(uuid, szGuid);

	strGuid = szGuid;
#endif

	return PyUnicode_FromString(strGuid.c_str());
}

void JITManager::Init(PyObject* args, PyObject* kwargs)
{
	PyJit::Object locals(PyEval_GetLocals(), true);
	PyJit::Object thisFile = (PyJit::Object)locals["__file__"];
	std::string  strModuleFileName = (std::string)PyJit::Object()["os.path.abspath"](thisFile);

	if (kwargs != nullptr)
	{
		PyJit::Object kw(kwargs);
		std::string fullLibName;
		std::string libPath = (std::string)kw["libpath"];
		std::string libName = (std::string)kw["libname"];
		//TODO: process libName
		if (libName.empty())
		{
			int moduleIndex = 0;
			JitLib* pLib = GetOrCreateLib(true, strModuleFileName, libName ,&moduleIndex);
			pLib->LoadLib();
		}
		else
		{
			if (libPath.find(":") != std::string::npos || libPath[0] == Path_Sep)
			{//absloute path
				fullLibName = libPath + Path_Sep_S + libName + ShareLibExt;
			}
			else
			{
				auto pos = strModuleFileName.rfind(Path_Sep);
				if (pos != std::string::npos)
				{
					std::string modulePath = strModuleFileName.substr(0, pos + 1);
					libPath = modulePath + libPath;
				}
				fullLibName = libPath + Path_Sep_S + libName + ShareLibExt;
			}
			int moduleIndex = 0;
			JitLib* pLib = GetOrCreateLib(true, strModuleFileName, libName ,&moduleIndex);
			if (pLib)
			{
				pLib->LoadLib(fullLibName);
			}
		}
	}
}

static PyObject*
cfunction_vectorcall_FASTCALL_KEYWORDS(
	PyObject* func, PyObject* const* args, size_t nargsf, PyObject* kwnames)
{
	Py_ssize_t nargs = PyVectorcall_NARGS(nargsf);
	auto pConvertFunc = PyCFunction_GET_FUNCTION(func);
	_PyCFunctionFastWithKeywords meth = (_PyCFunctionFastWithKeywords)pConvertFunc;
	if (meth == NULL) {
		return NULL;
	}
	PyObject* result = meth(PyCFunction_GET_SELF(func), args, nargs, kwnames);
	return result;

}


JitLib* JITManager::GetOrCreateLib(bool addModule, std::string callerPyFile, std::string strLibName,int* pModuleIndex)
{
	//split into path and file name
	//create JitLib per Path
	std::string path;
	std::string filename;
	auto pos = callerPyFile.rfind("/");
	if (pos == std::string::npos)
	{
		pos = callerPyFile.rfind("\\");
	}
	if (pos != std::string::npos)
	{
		path = callerPyFile.substr(0, pos);
		filename = callerPyFile.substr(pos + 1);
	}
	else
	{
		//just in case, but should never be here
		filename = callerPyFile;
	}
	pos = filename.rfind('.');
	//remove ext name
	if (pos != std::string::npos)
	{
		filename = filename.substr(0,pos);
	}
	JitLib* pLib = nullptr;
	//make lib name and path
	std::string libKey = path;
	if (strLibName.size() == 0)//empty string means use folder name as lib name
	{
		auto pos = path.rfind("/");
		if (pos == std::string::npos)
		{
			pos = path.rfind("\\");
		}
		if (pos != std::string::npos)
		{
			strLibName = path.substr(pos + 1);
		}
	}
	else if(strLibName == "self")//change to use module name
	{
		strLibName = filename;
		libKey = path + Path_Sep + strLibName;
	}
	else
	{
		libKey = path + Path_Sep + strLibName;
	}
	auto it = mJitlibs.find(path);
	if (it != mJitlibs.end())
	{
		pLib = it->second;
	}
	else
	{
		pLib = new JitLib(path, strLibName);
		mJitlibs.emplace(std::make_pair(path, pLib));
	}
	if (addModule)
	{
		int idx = pLib->AddModule(filename);
		if (pModuleIndex)
		{
			*pModuleIndex = idx;
		}
	}
	return pLib;
}
//if same name but diffrent folder?
JitLib* JITManager::SearchLibBySharedLibName(std::string name)
{
	JitLib* pLib = nullptr;
	for (auto it : mJitlibs)
	{
		if (it.second->LibFileName() == name)
		{
			pLib = it.second;
			break;
		}
	}
	return pLib;
}

JitClassInfo* JITManager::FindClassFromNameSpace(std::string nm)
{
	auto list = split(nm, '.');
	if (list.size() != 3)
	{
		return nullptr;
	}
	JitLib* pLib = SearchLibBySharedLibName(list[0]);
	if (pLib == nullptr)
	{
		return nullptr;
	}
	return pLib->FindClassFromNameSpace(list[1], list[2]);
}

PyObject* JITManager::FindClassProxyType(std::string className)
{
	PyObject* pOb = PyModule_GetDict(mThisModule);//Borrowed reference
	if (pOb == nullptr)
	{
		return nullptr;
	}
	PyJit::Dict dict = (PyJit::Dict)pOb;//Dict will addref
	PyJit::Object classProxyType = dict[className.c_str()];
	return (PyObject*)(GalaxyJitPtr)classProxyType;
}
