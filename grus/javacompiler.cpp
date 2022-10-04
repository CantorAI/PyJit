#include "javacompiler.h"
#include "jitlib.h"
#include "jitfuncinfo.h"
#include "jitmgr.h"

JavaCompiler::JavaCompiler()
{
}

JavaCompiler::~JavaCompiler()
{
}

bool JavaCompiler::Init(std::string& libFileName)
{
	std::string strJitFolder = mJitLib->Path() + Path_Sep + "_jit_";
	mLibFileName = strJitFolder + Path_Sep_S
		+ "bin" + Path_Sep_S + mJitLib->LibFileName() + ".jar";
	return true;
}

bool JavaCompiler::BuildCode(int moduleIndex,std::string strJitFolder,
	std::vector<std::string>& srcs, std::vector<std::string>& exports)
{
	return false;
}

bool JavaCompiler::CompileAndLink(std::string strJitFolder,
	std::vector<std::string> srcs, std::vector<std::string> exports)
{
	return false;
}

bool JavaCompiler::LoadLib(const std::string& libFileName, JitHost* pHost)
{
	return false;
}

void JavaCompiler::UnloadLib()
{
}
