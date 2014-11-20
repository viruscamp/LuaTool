#include "Locals.h"
#include "Function.h"

using namespace std;

void Locals::setReg(int setReg, int setRegTo)
{
	if (setRegTo == -1)
		setRegTo = setReg;

	useInfo[pc].setReg = setReg;
	useInfo[pc].setRegTo = setRegTo;
}

void Locals::setReg2(int setReg2)
{
	useInfo[pc].setReg2 = setReg2;
}

void Locals::loadReg(int loadReg, int loadRegTo)
{
	if (loadRegTo == -1)
		loadRegTo = loadReg;

	useInfo[pc].loadReg = loadReg;
	useInfo[pc].loadRegTo = loadRegTo;
}

void Locals::loadReg2(int loadReg2)
{
	if (useInfo[pc].loadReg == -1)
		loadReg(loadReg2);
	else
		useInfo[pc].loadReg2 = loadReg2;
}

void Locals::loadReg3(int loadReg3)
{
	if (useInfo[pc].loadReg2 == -1)
		loadReg2(loadReg3);
	else
		useInfo[pc].loadReg3 = loadReg3;
}

void Locals::forceDeclare(int forceDeclareFrom, int forceDeclareTo)
{
	useInfo[pc].forceDeclareFrom = forceDeclareFrom;
	useInfo[pc].forceDeclareTo = forceDeclareTo;
}

void Locals::addBlock(PcAddr start, PcAddr end, int level)
{
	BlockInfo bi;
	BlockInfo repeat;

	for (int i = 0; i < repeats.size(); i++)
	{
		if (pc > repeats[i].start)
		{
			if (repeats[i].start < start && repeats[i].end < end)
			{
				repeats.erase(repeats.begin() + i);
				continue;
			}

			repeat = repeats[i];
			repeats.erase(repeats.begin() + i);

			repeat.level = level;
			level++;
			blockInfo.push_back(repeat);

			break;
		}
	}

	for (int i=0; i < blockInfo.size(); i++)
	{
		if (start < blockInfo[i].end-1 && start > blockInfo[i].start && end > blockInfo[i].end )
			end = blockInfo[i].end;
	}

	bi.start = start;
	bi.end = end;
	bi.level = level;

	blockInfo.push_back(bi);
	block = level;
}

void Locals::addRepeat(PcAddr start, PcAddr end, int level)
{
	BlockInfo bi;

	bi.start = start;
	bi.end = end;
	bi.level = level;

	repeats.push_back(bi);
}

PcAddr Locals::blockEnd(PcAddr pc)
{
	for (int i = blockInfo.size()-1; i >= 0 ; i--)
	{
		if (pc > blockInfo[i].start && pc <= blockInfo[i].end)
			return blockInfo[i].end;
	}
	return func->codeSize-1;
}

int Locals::getBlockLevel(PcAddr pc)
{
	for (int i = blockInfo.size()-1; i >= 0 ; i--)
	{
		if (pc >= blockInfo[i].start && pc <= blockInfo[i].end)
			return blockInfo[i].level;
	}
}

void Locals::declareLocal(int reg, PcAddr start, PcAddr end, string name)
{
	Local l(reg, start, end);
	l.name = name;
	locals.push_back(l);
}

void Locals::declareLocal(int reg, PcAddr pc)
{
	Local l(reg, getPotVar(reg, pc).start, getPotVar(reg, pc).end);
	locals.push_back(l);
}

void Locals::addPotVar(int reg, PcAddr start, PcAddr end)
{
	PotVar pt;

	pt.start = start;
	pt.end = end;

	potVar[reg].push_back(pt);
}

Locals::PotVar Locals::getPotVar(int reg, PcAddr pc)
{	
	PotVar pv;
	pv.start = -2; pv.end = -2;
	
	if (potVar[reg].empty())
		return pv;

	for (int i = potVar[reg].size()-1; i >= 0 ; i--)
	{
		if (pc >= potVar[reg][i].start && pc <= potVar[reg][i].end)
			return potVar[reg][i];
	}

	for (int i = potVar[reg].size()-1; i >= 0 ; i--)
	{
		if (pc >= potVar[reg][i].start)
			return potVar[reg][i];
	}

	return pv;
}

void Locals::RegUsage::inc(PcAddr pc)
{
	Usage u;
	u.pc = pc;

	if (usage.empty())
		u.usageLevel = 1;
	else
		u.usageLevel = usage.back().usageLevel + 1;

	usage.push_back(u);
}

void Locals::RegUsage::dec(PcAddr pc)
{
	Usage u;
	u.pc = pc;

	if (usage.empty())
		u.usageLevel = -1;
	else
		u.usageLevel = usage.back().usageLevel - 1;

	usage.push_back(u);
}

const int Locals::RegUsage::operator [](PcAddr pc)
{
	for (int i = usage.size() - 1; i >= 0 ; i--)
	{
		if (usage[i].pc <= pc)
			return usage[i].usageLevel;
	}
	return 0;
}

void Locals::RegUsage::setLevel(int usageLevel, PcAddr pc)
{
	Usage u;
	u.pc = pc;
	u.usageLevel = usageLevel;
	usage.push_back(u);
}

void Locals::RegUsage::normalize(PcAddr pc)
{
	int prevUsage = 0;
	int insertHere = 0;

	for (int i = usage.size() - 1; i >= 0 ; i--)
	{
		if (usage[i].pc < pc)
		{
			prevUsage = usage[i].usageLevel;
			insertHere = i;
			break;
		}
	}

	for (int i = usage.size() - 1; i >= 0 ; i--)
	{
		if (usage[i].pc >= pc)
			usage[i].usageLevel -= prevUsage;
		else
			break;
	}

	Usage u;
	u.pc = pc;
	u.usageLevel = 0;

	if (insertHere + 1 < usage.size())
	{
		if (insertHere >= 0 && usage[insertHere].pc == pc)
			usage.insert(usage.begin()+insertHere, u);
		else
			usage.insert(usage.begin()+insertHere+1, u);
	}
	else
		usage.push_back(u);
}

Locals::Local& Locals::operator[] (size_t index)
{
	if ( index < locals.size() )
		return locals[index];
	else
		return locals.back();
}

size_t Locals::size()
{
	return locals.size();
}