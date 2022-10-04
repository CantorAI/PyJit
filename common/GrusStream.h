#pragma once

#include <string>
#include <exception>
#include "Jit_Host.h"
#include <string.h>

typedef long long STREAM_SIZE;
struct blockIndex
{
    int blockIndex;
    STREAM_SIZE offset;
};

class GrusStreamException 
    : public std::exception
{
public:
    GrusStreamException(int code)
    {
        m_code = code;
    }
    virtual const char* what() const throw()
    {
        return "GrusStream exception happened";
    }
    int Code()
    {
        return m_code;
    }
private:
    int m_code = 0;
};

class GrusStream:
    public JitStream
{
public:
    GrusStream();
    ~GrusStream();

    void SetProvider(JitStream* p)
    {
        m_pProvider = p;
    }
    void ResetPos()
    {
        curPos = { 0,0 };
    }
    STREAM_SIZE Size()
    {
        return m_size;
    }
    bool FullCopyTo(char* buf, STREAM_SIZE bufSize);
    bool CopyTo(char* buf, STREAM_SIZE size);
    bool appendchar(char c);
    bool fetchchar(char& c);
    bool fetchstring(std::string& str);
    bool append(char* data, STREAM_SIZE size);
    inline bool Skip(STREAM_SIZE size)
    {
        return CopyTo(nullptr, size);
    }
    template<typename T>
    GrusStream& operator<<(T v)
    {
        append((char*)&v, sizeof(v));
        return *this;
    }
    GrusStream& operator<<(const char c)
    {
        appendchar(c);
        return *this;
    }
    GrusStream& operator<<(std::string v)
    {
        int size = (int)v.size() + 1;
        append((char*)v.c_str(), size);
        return *this;
    }
    GrusStream& operator<<(const char* str)
    {
        int size = (int)strlen(str) + 1;
        append((char*)str, size);
        return *this;
    }
    template<typename T>
    GrusStream& operator>>(T& v)
    {
        CopyTo((char*)&v, sizeof(v));
        return *this;
    }
    GrusStream& operator>>(std::string& v)
    {
        fetchstring(v);
        return *this;
    }
    GrusStream& operator>>(char& c)
    {
        fetchchar(c);
        return *this;
    }
    unsigned long long GetKey()
    {
        return m_streamKey;
    }
    inline blockIndex GetPos()
    {
        return curPos;
    }
    inline void SetPos(blockIndex pos)
    {
        curPos = pos;
    }
    STREAM_SIZE CalcSize(blockIndex pos);
    void SetOverrideMode(bool b)
    {
        m_InOverrideMode = b;
    }
    bool IsEOS()
    {
        if ((BlockNum() - 1) == curPos.blockIndex)
        {
            blockInfo& blk=GetBlockInfo(curPos.blockIndex);
            return (blk.data_size == curPos.offset);
        }
        else
        {
            return false;
        }
    }
    virtual bool CanBeOverrideMode()
    {
        return true;
    }
protected:
    JitStream* m_pProvider = nullptr;//real impl.
    unsigned long long m_streamKey = 0;
    blockIndex curPos = { 0,0 };
    STREAM_SIZE m_size = 0;
    bool m_InOverrideMode = false;

    // Inherited via JitStream
    virtual void Refresh() override;
    virtual int BlockNum() override;
    virtual blockInfo& GetBlockInfo(int index) override;
    virtual bool NewBlock() override;
    virtual bool MoveToNextBlock() override;
};

