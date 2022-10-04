#pragma once

#include "compiler.h"

class JitHost;
class CsCompiler :
	public JitCompiler
{
public:
	CsCompiler();
	~CsCompiler();

	// Inherited via JitCompiler
	virtual bool Init(std::string& libFileName) override;
	virtual bool BuildCode(int moduleIndex, std::string strJitFolder,
		std::vector<std::string>& srcs, std::vector<std::string>& exports) override;
	virtual bool CompileAndLink(std::string strJitFolder,
		std::vector<std::string> srcs, std::vector<std::string> exports) override;
	virtual bool LoadLib(const std::string& libFileName, JitHost* pHost) override;
	virtual void UnloadLib() override;
protected:
	bool BuildCodeFiles(std::string& funcName, FuncParseInfo& info, std::string& code, std::string& stubCode);
	std::string MapDataType(std::string type);

};