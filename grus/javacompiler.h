#pragma once

#include "compiler.h"

class JitHost;
class JavaCompiler :
	public JitCompiler
{
public:
	JavaCompiler();
	~JavaCompiler();

	// Inherited via JitCompiler
	virtual bool Init(std::string& libFileName) override;
	virtual bool BuildCode(int moduleIndex,std::string strJitFolder,
		std::vector<std::string>& srcs, std::vector<std::string>& exports) override;
	virtual bool CompileAndLink(std::string strJitFolder,
		std::vector<std::string> srcs, std::vector<std::string> exports) override;
	virtual bool LoadLib(const std::string& libFileName, JitHost* pHost) override;
	virtual void UnloadLib() override;
};