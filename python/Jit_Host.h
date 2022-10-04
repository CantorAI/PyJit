#ifndef _JIT_HOST_H_
#define _JIT_HOST_H_

#include <vector>
#include <string>

#if (WIN32)
#define JIT_EXPORT __declspec(dllexport) 
#else
#define JIT_EXPORT
#include <string.h>
#endif

//all funcation,if return GalaxyJitPtr will hold a new reference
typedef void* GalaxyJitPtr;

struct blockInfo
{
	char* buf;
	long long block_size;
	long long data_size;
};

class JitStream
{
public:
	virtual void Refresh() = 0;
	virtual int BlockNum() = 0;
	virtual blockInfo& GetBlockInfo(int index) = 0;
	virtual bool NewBlock() = 0;
	virtual bool MoveToNextBlock() = 0;
};

class JitHost
{
public:
	virtual int AddModule(void* context,const char* moduleName) = 0;
	virtual void AddFunc(int moduleIndex,void* context, const char* hash,const char* funcName, void* funcPtr) = 0;
	virtual void AddClass(int moduleIndex, void* context, const char* hash, 
		const char* className, int propNum,int methodNum,
		const char* classMemberNames[],int memberNum,
		const unsigned long long classStubFuncs[],int stubNum) = 0;
	virtual int to_int(GalaxyJitPtr pVar) = 0;
	virtual GalaxyJitPtr from_int(int val) = 0;
	virtual long long to_longlong(GalaxyJitPtr pVar) = 0;
	virtual GalaxyJitPtr from_longlong(long long val) = 0;
	virtual float to_float(GalaxyJitPtr pVar) = 0;
	virtual GalaxyJitPtr from_float(float val) = 0;
	virtual double to_double(GalaxyJitPtr pVar) = 0;
	virtual GalaxyJitPtr from_double(double val) = 0;
	virtual const char* to_str(GalaxyJitPtr pVar) = 0;
	virtual GalaxyJitPtr from_str(const char* val) = 0;

	virtual long long GetCount(GalaxyJitPtr objs) = 0;
	virtual GalaxyJitPtr Get(GalaxyJitPtr objs, int idx) = 0;
	virtual int Set(GalaxyJitPtr objs, int idx, GalaxyJitPtr val) = 0;
	virtual GalaxyJitPtr Get(GalaxyJitPtr objs, const char* key) = 0;
	virtual bool ContainKey(GalaxyJitPtr container, GalaxyJitPtr key) = 0;
	virtual bool KVSet(GalaxyJitPtr container, GalaxyJitPtr key, GalaxyJitPtr val) = 0;
	virtual void Free(const char* sz) = 0;
	virtual int AddRef(GalaxyJitPtr obj) = 0;
	virtual void Release(GalaxyJitPtr obj) = 0;
	virtual GalaxyJitPtr Call(GalaxyJitPtr obj, int argNum, GalaxyJitPtr* args) = 0;
	virtual GalaxyJitPtr Call(GalaxyJitPtr obj, int argNum, GalaxyJitPtr* args, GalaxyJitPtr kwargs) = 0;
	virtual GalaxyJitPtr Call(GalaxyJitPtr obj, GalaxyJitPtr args, GalaxyJitPtr kwargs) = 0;
	virtual GalaxyJitPtr NewList(long long size) = 0;
	virtual GalaxyJitPtr NewTuple(long long size) = 0;
	virtual GalaxyJitPtr NewDict() = 0;
	virtual GalaxyJitPtr NewArray(int nd, unsigned long long* dims, int itemDataType) = 0;
	virtual void* GetDataPtr(GalaxyJitPtr obj) = 0;
	virtual bool GetDataDesc(GalaxyJitPtr obj, 
		int& itemDataType,int& itemSize,
		std::vector<unsigned long long>& dims,
		std::vector<unsigned long long>& strides) = 0;
	virtual GalaxyJitPtr Import(const char* key) = 0;
	virtual bool IsNone(GalaxyJitPtr obj) = 0;
	virtual bool IsDict(GalaxyJitPtr obj) = 0;
	virtual bool IsList(GalaxyJitPtr obj) = 0;
	virtual bool IsArray(GalaxyJitPtr obj) = 0;
	virtual GalaxyJitPtr GetDictKeys(GalaxyJitPtr obj) = 0;
	virtual void* GetClassProxyNative(GalaxyJitPtr classProxyObj) = 0;
	virtual GalaxyJitPtr QueryOrCreate(GalaxyJitPtr selfofcaller,const char* class_name, void* pNativeObj) = 0;
	virtual const char* GetObjectType(GalaxyJitPtr obj) = 0;
	virtual GalaxyJitPtr GetDictItems(GalaxyJitPtr dict) = 0;
	virtual bool DictContain(GalaxyJitPtr dict,std::string& strKey) = 0;
	virtual bool StreamWrite(unsigned long long streamId, char* data, long long size) = 0;
	virtual bool StreamRead(unsigned long long streamId, char* data, long long size) = 0;
	virtual bool StreamWriteChar(unsigned long long streamId, char ch) = 0;
	virtual bool StreamReadChar(unsigned long long streamId, char& ch) = 0;
	virtual bool StreamWriteString(unsigned long long streamId,std::string& str) = 0;
	virtual bool StreamReadString(unsigned long long streamId, std::string& str) = 0;
	virtual GalaxyJitPtr GetPyNone() = 0;
	virtual GalaxyJitPtr CreateJitObject(void* lib,
		const char* moduleName, 
		const char* objTypeName,
		GalaxyJitPtr args) = 0;
	virtual bool ParseModule(GalaxyJitPtr pModule) = 0;
	virtual bool PackTo(GalaxyJitPtr obj,JitStream* pStream) = 0;
	virtual GalaxyJitPtr UnpackFrom(JitStream* pStream) = 0;
	virtual GalaxyJitPtr Pack(std::vector<GalaxyJitPtr> objList) = 0;
	virtual bool Unpack(GalaxyJitPtr byteArray,std::vector<GalaxyJitPtr>& objList) = 0;
	virtual GalaxyJitPtr CreateByteArray(const char* buf, long long size) = 0;
};

extern JitHost* g_pHost;

#endif//_JIT_HOST_H_

