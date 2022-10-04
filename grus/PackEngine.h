#pragma once

#include "grus_impl.h"
#include "Jit_Object.h"

#include "GrusStream.h"

enum class UnpackAction
{
    None,
    back_to_parent,
    dict_add_key,
    dict_set_value,
};

class PackEngine;
class PackHandler
{
public:
    virtual bool Pack(std::string& objType, GrusStream& stream, PyJit::Object& obj)=0;
    virtual bool Unpack(std::string& objType,GrusStream& stream, PyJit::Object& obj)=0;

    void SetPackEngine(PackEngine* p)
    {
        m_PackEngine = p;
    }
protected:
    PackEngine* m_PackEngine;
};
class PackEngine
{
public:
	PackEngine();
	~PackEngine();
    void SetPackHandler(PackHandler* p)
    {
        m_PackHandler = p;
        m_PackHandler->SetPackEngine(this);
    }
	bool Pack(GrusStream& stream,std::vector<PyObject*>& pInObs);
    bool Pack(GrusStream& stream, std::vector<GalaxyJitPtr>& pInObs);
    template<typename T>
    std::vector<T> Unpack(GrusStream& stream)
    {
        std::vector<T> outObList;
        while (!stream.IsEOS())
        {
            PyJit::Object outObj;
            UnpackAction act = UnpackAction::None;
            bool bRet = DoUnpack(stream, outObj, act);
            if (bRet)
            {
                GalaxyJitPtr ptr = (GalaxyJitPtr)outObj;
                outObList.push_back((T)(PyObject*)ptr);
            }
            outObj.Empty();
        }
        return outObList;
    }
protected:
    const char endChar = 0;
    bool DoPack(GrusStream& stream,PyJit::Object& obj);
    bool DoUnpack(GrusStream& stream, PyJit::Object& obj, UnpackAction& act);

    bool DoNumpyPack(GrusStream& stream, PyJit::Object& obj);
    bool DoNumpyUnpack(GrusStream& stream, PyJit::Object& obj);
    bool DoCustomPack(std::string& objType, GrusStream& stream, PyJit::Object& obj);
    bool DoCustomUnpack(GrusStream& stream, PyJit::Object& obj);

    int QueryOrCreateShortName(std::string& fullname,bool& exist);
    std::unordered_map<std::string, int> m_shortNameMap;
    std::unordered_map<int, std::string> m_shortNameFromIdMap;
private:
    PackHandler* m_PackHandler = nullptr;
};

