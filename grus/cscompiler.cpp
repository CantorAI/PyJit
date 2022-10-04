#include "cscompiler.h"
#include "jitlib.h"
#include "jitfuncinfo.h"
#include "jitmgr.h"
#include <iostream>
#include <fstream>

CsCompiler::CsCompiler()
{
}

CsCompiler::~CsCompiler()
{
}

bool CsCompiler::Init(std::string& libFileName)
{
	std::string strJitFolder = mJitLib->Path() + Path_Sep + "_jit_";
	mLibFileName = strJitFolder + Path_Sep_S
		+ "bin" + Path_Sep_S + mJitLib->LibFileName() + ShareLibExt;
	return true;
}

bool CsCompiler::BuildCode(int moduleIndex,std::string strJitFolder,
	std::vector<std::string>& srcs, std::vector<std::string>& exports)
{
	std::string allcode =
		"";
	std::string stubFileHead = "";
	std::string allStubCode = stubFileHead;
	bool needToGenFuncCodeFile = false;
	for (auto it : mJitLib->FuncMap(moduleIndex))
	{
		JitFuncInfo* pFuncInfo = it.second.pFuncInfo;
		if (pFuncInfo->Lang() != LangType::cs)
		{
			continue;
		}
		FuncParseInfo funcInfo;
		if (!TranslateCode(pFuncInfo->Name(), pFuncInfo->Code(), funcInfo))
		{
			continue;
		}
		//TODO: add code here to generate the cs files
		//
		std::string funcCode;
		std::string stubCode;
		if (!BuildCodeFiles(pFuncInfo->Name(), funcInfo, funcCode, stubCode))
		{
			continue;
		}

		bool isExternImpl = pFuncInfo->IsExternImpl();
		std::string& funcName = pFuncInfo->Name();
		//TODO:
		//
		if (isExternImpl)
		{
			std::vector<std::string> impl_srcs = pFuncInfo->GetExternImplFileName();
			for (std::string& ss : impl_srcs)
			{
				//check if s is full path with prefix '/' or '\\'
				if (ss[0] != Path_Sep)
				{
#if (!WIN32)
					ReplaceAll(ss, "\\", "/");
#endif
					std::string strFileName = mJitLib->Path() + Path_Sep_S + ss;
					srcs.push_back(strFileName);
				}
			}
		}
		else
		{
			allcode += funcCode;
			needToGenFuncCodeFile = true;
		}
		allStubCode += stubCode;
		//SPRINTF(funcLine, online_len, funcLineTemp,
		//	pFuncInfo->Hash().c_str(),
		//	funcName.c_str(), funcName.c_str());
		//init_func_lines += funcLine;
	}

	//Write out code
	if (needToGenFuncCodeFile)
	{
		std::string strJitCs = strJitFolder + Path_Sep
			+ mJitLib->LibFileName() + ".cs";
		std::ofstream file(strJitCs);
		if (!file.is_open())
		{
			return false;
		}
		file.write((const char*)allcode.c_str(), allcode.length() * sizeof(char));
		file.close();

		srcs.push_back(strJitCs);
	}
	//Write out stub code
	std::string strJitStubCs = strJitFolder + Path_Sep
		+ mJitLib->LibFileName() + ".stub.cs";
	std::ofstream sfile(strJitStubCs);
	if (!sfile.is_open())
	{
		return false;
	}
	sfile.write((const char*)allStubCode.c_str(), allStubCode.length() * sizeof(char));
	sfile.close();

	srcs.push_back(strJitStubCs);

	return false;
}

bool CsCompiler::CompileAndLink(std::string strJitFolder,
	std::vector<std::string> srcs, std::vector<std::string> exports)
{
	int Debug = mJitLib->IsBuildWithDebug() ? 1 : 0;
	std::string binPath = (std::string)PyJit::Object()["os.path.join"](strJitFolder, "bin");
	auto M_ccompiler = PyJit::Object::Import("distutils.ccompiler");
	auto compiler = M_ccompiler["new_compiler"]();
	auto M_sysconfig = PyJit::Object::Import("distutils.sysconfig");
	M_sysconfig["customize_compiler"](compiler);
	GalaxyJitPtr* pp;
	pp = new GalaxyJitPtr[1];
	pp[0] = PyJit::Object(mJitLib->LibFileName());
	std::string binFileName =
		(std::string)((PyJit::Object)compiler["shared_object_filename"]).Call(1, pp,
			PyJit::Object(std::map <std::string, PyJit::Object>
	{
		{"output_dir", PyJit::Object(binPath)},
	}));
	PyJit::Object sources(srcs);
	std::string includePath = JITManager::I().GetPath();
	std::map <std::string, PyJit::Object> kwargs_
	{
		  { "output_dir", PyJit::Object("obj") },
		  { "include_dirs",PyJit::Object(std::vector<std::string>{includePath})},
		  {"debug",PyJit::Object(Debug)}
	};
	PyJit::Object kwargs(kwargs_);
	pp = new GalaxyJitPtr[1];
	pp[0] = sources;
	//pp will be deleted by .Call
	auto objects = ((PyJit::Object)compiler["compile"]).Call(1, pp, kwargs);
	pp = new GalaxyJitPtr[2];
	pp[0] = objects;
	pp[1] = PyJit::Object(binFileName);
	PyJit::Object(compiler["link_shared_object"]).Call(2, pp, PyJit::Object(
		std::map <std::string, PyJit::Object>{
			{"debug", PyJit::Object(Debug)},
			{ "output_dir", PyJit::Object("bin") },
			{ "target_lang",PyJit::Object("cs") }//,
			//{ "export_symbols",PyJit::Object(std::vector<std::string>{"InitJitLib"}) }
	}));
	return false;
}

bool CsCompiler::LoadLib(const std::string& libFileName, JitHost* pHost)
{
	return false;
}

void CsCompiler::UnloadLib()
{
}

bool CsCompiler::BuildCodeFiles(std::string& funcName,
	FuncParseInfo& info, std::string& code,
	std::string& stubCode)
{
	const int online_len = 2000;

	std::string param_def;
	std::string param_in_stub;
	char line[online_len];
	for (int i = 0; i < info.parameters.size(); i++)
	{
		auto it0 = info.parameters[i];
		auto mappedDataType = MapDataType(it0.second);
		param_def += "\t" + mappedDataType + " " + it0.first;
		SPRINTF(line, online_len, "\t\t(%s)objs[%d]", mappedDataType.c_str(), i);
		param_in_stub += line;

		if (i < (info.parameters.size() - 1))
		{
			param_def += ",";
			param_in_stub += ",";
		}
		param_def += "\n";
		param_in_stub += "\n";
	}
	std::string retType = MapDataType(info.retType);
	std::string funcHead;
	//SPRINTF(line, online_len, "inline %s %s\n(\n", retType.c_str(), funcName.c_str());
	//TODO:if add inline, can't debug, so delete it, because use the O2 to compile
	SPRINTF(line, online_len, "%s %s\n(\n", retType.c_str(), funcName.c_str());
	funcHead = line;
	funcHead += param_def;
	funcHead += ")";
	std::string decl_func = "extern " + funcHead;
	ReplaceAll(decl_func, "\n", "");
	ReplaceAll(decl_func, "\t", " ");

	funcHead += "\n{";
	std::string funcAll = funcHead + info.body +
		"}\n";

	std::string funcStubHead;
	SPRINTF(line, online_len,
		"GalaxyJitPtr %s_stub(GalaxyJitPtr vars)\n{\n"
		"\t%s;\n"
		"\tPyJit::Object objs(vars);\n"
		"\treturn PyJit::Object(%s(\n",
		funcName.c_str(),
		decl_func.c_str(),
		funcName.c_str()
	);
	funcStubHead = line;
	std::string funcStubAll = funcStubHead
		+ param_in_stub
		+ "\t\t)\n\t);\n}\n";

	code = funcAll;
	stubCode = funcStubAll;
	return true;
}
std::string CsCompiler::MapDataType(std::string type)
{
	std::string strRetType;
	if (type == "")
	{
		strRetType = "PyJit::Object";
	}
	else if (type == "int")
	{
		strRetType = "int";
	}
	else if (type == "float")
	{
		strRetType = "float";
	}
	else if (type == "str")
	{
		strRetType = "std::string";
	}
	return strRetType;
}