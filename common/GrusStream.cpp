#include "GrusStream.h"
#include "GrusJitHost.h"
#include <stdexcept>

GrusStream::GrusStream()
{
	m_streamKey = GrusJitHost::I().RegisterStream(this);
}

GrusStream::~GrusStream()
{
	GrusJitHost::I().UnregisterStream(m_streamKey);
	m_size = 0;
	curPos = { 0,0 };
}
bool GrusStream::FullCopyTo(char* buf, STREAM_SIZE bufSize)
{
	if (bufSize < Size())
	{
		return false;
	}
	int blkNum = BlockNum();
	if (blkNum == 0)
	{//empty
		return true;
	}
	for (int i = 0; i < blkNum; i++)
	{
		blockInfo& blk = GetBlockInfo(i);
		memcpy(buf, blk.buf, blk.data_size);
		buf += blk.data_size;
	}
	return true;
}
bool GrusStream::CopyTo(char* buf, STREAM_SIZE size)
{
	blockIndex bi = curPos;
	STREAM_SIZE leftSize = size;
	char* pOutputData = buf;
	int curBlockIndex = bi.blockIndex;
	STREAM_SIZE blockOffset = bi.offset;
	while (leftSize > 0)
	{
		if (curBlockIndex >= BlockNum())
		{
			return false;
		}
		blockInfo& curBlock=GetBlockInfo(curBlockIndex);

		STREAM_SIZE restSizeInBlock = curBlock.data_size - blockOffset;
		STREAM_SIZE copySize = leftSize < restSizeInBlock ? leftSize : restSizeInBlock;
		if (buf != nullptr)
		{
			memcpy(pOutputData, curBlock.buf + blockOffset, copySize);
		}
		pOutputData += copySize;
		blockOffset += copySize;
		leftSize -= copySize;
		if (leftSize > 0)//need next block
		{
			if (!MoveToNextBlock())
			{
				return false;
			}
			blockOffset = 0;
			curBlockIndex++;
		}
	}
	curPos.blockIndex = curBlockIndex;
	curPos.offset = blockOffset;

	return true;
}

bool GrusStream::appendchar(char c)
{
	int blkNum = BlockNum();
	if (curPos.blockIndex >= blkNum)
	{
		if (!NewBlock())
		{
			return false;
		}
	}
	blockInfo& curBlock=GetBlockInfo(curPos.blockIndex);
	if (curPos.offset == curBlock.block_size)
	{
		curPos.blockIndex++;
		if (curPos.blockIndex >= blkNum)
		{
			if (!NewBlock())
			{
				return false;
			}
		}
		curPos.offset = 0;
	}
	*(curBlock.buf + curPos.offset) = c;
	curPos.offset++;
	if (!m_InOverrideMode)
	{
		curBlock.data_size++;
		m_size++;
	}
	return true;
}
bool GrusStream::fetchchar(char& c)
{
	int blkNum = BlockNum();
	if (curPos.blockIndex >= blkNum)
	{
		return false;
	}
	blockInfo& curBlock=GetBlockInfo(curPos.blockIndex);
	if (curPos.offset == curBlock.block_size)
	{
		MoveToNextBlock();
		curPos.blockIndex++;
		if (curPos.blockIndex >= blkNum)
		{
			return false;
		}
		curPos.offset = 0;
	}
	c = *(curBlock.buf + curPos.offset);
	curPos.offset++;
	return true;
}
bool GrusStream::fetchstring(std::string& str)
{
	char ch = 0;
	bool bRet = true;
	while (bRet)
	{
		bRet = fetchchar(ch);
		if (ch == 0)
		{
			break;
		}
		if (bRet)
		{
			str += ch;
		}
	}
	return bRet;
}
bool GrusStream::append(char* data, STREAM_SIZE size)
{
	blockIndex bi = curPos;
	STREAM_SIZE leftSize = size;
	char* pInputData = data;
	int curBlockIndex = bi.blockIndex;
	STREAM_SIZE blockOffset = bi.offset;

	while (leftSize > 0)
	{
		if (curBlockIndex >= BlockNum())
		{
			if (!NewBlock())
			{
				return false;
			}
		}
		blockInfo& curBlock=GetBlockInfo(curBlockIndex);
		STREAM_SIZE restSizeInBlock = curBlock.block_size - blockOffset;
		STREAM_SIZE copySize = leftSize < restSizeInBlock ? leftSize : restSizeInBlock;
		memcpy(curBlock.buf + blockOffset, pInputData, copySize);
		if (!m_InOverrideMode)
		{
			curBlock.data_size += copySize;
		}
		pInputData += copySize;
		blockOffset += copySize;
		leftSize -= copySize;
		if (leftSize > 0)//need next block
		{
			blockOffset = 0;
			curBlockIndex++;
		}
	}
	curPos.blockIndex = curBlockIndex;
	curPos.offset = blockOffset;
	if (!m_InOverrideMode)
	{
		m_size += size;
	}
	return true;
}

STREAM_SIZE GrusStream::CalcSize(blockIndex pos)
{
	if (BlockNum() <= pos.blockIndex)
	{
		return -1;
	}
	STREAM_SIZE size = 0;
	for (int i = 0; i < pos.blockIndex - 1; i++)
	{
		blockInfo& curBlock=GetBlockInfo(i);
		size += curBlock.data_size;
	}
	//last block use offset,not datasize
	size += pos.offset;
	return size;
}

void GrusStream::Refresh()
{
	if (m_pProvider)
	{
		m_pProvider->Refresh();
	}
}

int GrusStream::BlockNum()
{
	return m_pProvider? m_pProvider->BlockNum():0;
}

blockInfo& GrusStream::GetBlockInfo(int index)
{
	if (m_pProvider)
	{
		return m_pProvider->GetBlockInfo(index);
	}
	else
	{
		static blockInfo blk = { 0 };
		return blk;
	}
}

bool GrusStream::NewBlock()
{
	return m_pProvider ? m_pProvider->NewBlock() : false;
}

bool GrusStream::MoveToNextBlock()
{
	return m_pProvider ? m_pProvider->MoveToNextBlock() : false;
}


