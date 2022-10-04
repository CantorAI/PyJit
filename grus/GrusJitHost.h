#pragma once
#include "singleton.h"
#include "Jit_Host.h"
#include "Locker.h"
#include <unordered_map>

class GrusStream;
class GrusJitHost :
	public JitHost,
	public Singleton<GrusJitHost>
{
public:
	GrusJitHost();
	~GrusJitHost();


	// Inherited via JitHost
	virtual int AddModule(void* context, const char* moduleName) override;
	virtual void AddFunc(int moduleIndex,void* context, const char* hash, const char* funcName, void* funcPtr) override;
	virtual void AddClass(int moduleIndex, void* context, const char* hash, 
		const char* className, int propNum, int methodNum,
		const char* classMemberNames[], int memberNum,
		const unsigned long long classStubFuncs[], int stubNum) override;
	virtual int to_int(GalaxyJitPtr pVar) override;

	virtual GalaxyJitPtr from_int(int val) override;

	virtual float to_float(GalaxyJitPtr pVar) override;

	virtual GalaxyJitPtr from_float(float val) override;

	virtual const char* to_str(GalaxyJitPtr pVar) override;

	virtual GalaxyJitPtr from_str(const char* val) override;
	virtual GalaxyJitPtr Get(GalaxyJitPtr objs, int idx) override;
	virtual int Set(GalaxyJitPtr objs, int idx, GalaxyJitPtr val) override;
	virtual void Free(const char* sz) override;

	virtual GalaxyJitPtr Get(GalaxyJitPtr objs, const char* key) override;

	virtual GalaxyJitPtr Call(GalaxyJitPtr obj, int argNum, GalaxyJitPtr* args) override;
	virtual GalaxyJitPtr Call(GalaxyJitPtr obj, int argNum, GalaxyJitPtr* args, GalaxyJitPtr kwargs) override;
	virtual GalaxyJitPtr Call(GalaxyJitPtr obj, GalaxyJitPtr args, GalaxyJitPtr kwargs) override;
	virtual bool ContainKey(GalaxyJitPtr container, GalaxyJitPtr key) override;
	virtual bool KVSet(GalaxyJitPtr container, GalaxyJitPtr key, GalaxyJitPtr val) override;

	virtual GalaxyJitPtr NewTuple (long long size) override;
	virtual GalaxyJitPtr NewList(long long size) override;
	virtual GalaxyJitPtr NewDict() override;
	virtual GalaxyJitPtr NewArray(int nd, unsigned long long* dims, int itemDataType) override;
	virtual GalaxyJitPtr Import(const char* key) override;

	virtual void Release(GalaxyJitPtr obj) override;

	virtual int AddRef(GalaxyJitPtr obj) override;

	virtual void* GetDataPtr(GalaxyJitPtr obj) override;

	virtual long long GetCount(GalaxyJitPtr objs) override;

	virtual bool GetDataDesc(GalaxyJitPtr obj, 
		int& itemDataType, int& itemSize,
		std::vector<unsigned long long>& dims, 
		std::vector<unsigned long long>& strides) override;

	virtual long long to_longlong(GalaxyJitPtr pVar) override;
	virtual GalaxyJitPtr from_longlong(long long val) override;
	virtual double to_double(GalaxyJitPtr pVar) override;
	virtual GalaxyJitPtr from_double(double val) override;
	virtual bool IsNone(GalaxyJitPtr obj) override;
	virtual bool IsDict(GalaxyJitPtr obj) override;
	virtual bool DictContain(GalaxyJitPtr dict,std::string& strKey) override;
	virtual bool IsArray(GalaxyJitPtr obj) override;
	virtual bool IsList(GalaxyJitPtr obj) override;
	virtual GalaxyJitPtr GetDictKeys(GalaxyJitPtr obj) override;
	virtual void* GetClassProxyNative(GalaxyJitPtr classProxyObj) override;
	virtual GalaxyJitPtr QueryOrCreate(GalaxyJitPtr selfofcaller, const char* class_name, void* pNativeObj) override;
	virtual const char* GetObjectType(GalaxyJitPtr obj) override;
	virtual GalaxyJitPtr GetDictItems(GalaxyJitPtr dict) override;
	virtual bool StreamWrite(unsigned long long streamId, char* data, long long size) override;
	virtual bool StreamRead(unsigned long long streamId, char* data, long long size) override;
	virtual bool StreamWriteChar(unsigned long long streamId, char ch) override;
	virtual bool StreamReadChar(unsigned long long streamId, char& ch) override;
	virtual bool StreamWriteString(unsigned long long streamId, std::string& str) override;
	virtual bool StreamReadString(unsigned long long streamId, std::string& str) override;
	virtual GalaxyJitPtr GetPyNone() override;
	virtual GalaxyJitPtr CreateJitObject(void* lib,
		const char* moduleName,
		const char* objTypeName,
		GalaxyJitPtr args);
public:
	unsigned long long RegisterStream(GrusStream* pStream);
	void UnregisterStream(unsigned long long key);
private:
	GrusStream* GetStream(unsigned long long id);
	std::unordered_map<unsigned long long, GrusStream*> m_runningStreams;
	Locker m_streamLocker;
	// Inherited via JitHost
	virtual bool ParseModule(GalaxyJitPtr pModule) override;
	virtual bool PackTo(GalaxyJitPtr obj, JitStream* pStream) override;
	virtual GalaxyJitPtr UnpackFrom(JitStream* pStream) override;
	virtual GalaxyJitPtr Pack(std::vector<GalaxyJitPtr> objList) override;
	virtual bool Unpack(GalaxyJitPtr byteArray, std::vector<GalaxyJitPtr>& objList) override;
	virtual GalaxyJitPtr CreateByteArray(const char* buf, long long size) override;
};
