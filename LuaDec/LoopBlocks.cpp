#include "LoopBlocks.h"

using namespace std;

void LoopBlocks::add(PcAddr startPC, string startStr, PcAddr endPC, string endStr)
{
	blocks.push_back(SingleLoopBlock(startPC, startStr, endPC, endStr));
}

void LoopBlocks::setBreak(PcAddr pc, string breakStr)
{
	removeBreak(pc); // break already mapped to wrong block, remove it

	// add it to right block
	for(int i = blocks.size()-1; i >= 0; i--)
	{
		if (blocks[i].startPC < pc && blocks[i].endPC > pc)
		{
			blocks[i].breakPC.push_back(pc);
			blocks[i].breakStr = breakStr;
			break;
		}
	}
}

string LoopBlocks::getStart(PcAddr pc)
{
	for(int i=0; i < blocks.size(); i++)
	{
		if (blocks[i].startPC == pc)
			return blocks[i].startStr;
	}
	return "";
}

string LoopBlocks::getEnd(PcAddr pc)
{
	for(int i=0; i < blocks.size(); i++)
	{
		if (blocks[i].endPC == pc && blocks[i].endWritten == false)
		{
			blocks[i].endWritten = true;
			return blocks[i].endStr;
		}
	}
	return "";
}

string LoopBlocks::peekEnd(PcAddr pc)
{
	for(int i=0; i < blocks.size(); i++)
	{
		if (blocks[i].endPC == pc && blocks[i].endWritten == false)
			return blocks[i].endStr;
	}
	return "";
}

PcAddr LoopBlocks::getEndPC(PcAddr start)
{
	for(int i=0; i < blocks.size(); i++)
	{
		if (blocks[i].startPC == start)
			return blocks[i].endPC;
	}
	return -1;
}

PcAddr LoopBlocks::getStartPC(PcAddr end)
{
	for(int i=0; i < blocks.size(); i++)
	{
		if (blocks[i].endPC == end)
			return blocks[i].startPC;
	}
	return -1;
}

string LoopBlocks::getBreak(PcAddr pc)
{
	for(int i=0; i < blocks.size(); i++)
	{
		for(int j=0; j < blocks[i].breakPC.size(); j++)
			if (blocks[i].breakPC[j] == pc)
				return blocks[i].breakStr;
	}
	return "";
}

void LoopBlocks::removeBreak(PcAddr pc)
{
	for(int i=0; i < blocks.size(); i++)
	{
		for(int j=0; j < blocks[i].breakPC.size(); j++)
			if (blocks[i].breakPC[j] == pc)
			{
				blocks[i].breakPC.erase(blocks[i].breakPC.begin() + j);
				if (blocks[i].breakPC.empty())
					blocks[i].breakStr.clear();
			}
	}
}