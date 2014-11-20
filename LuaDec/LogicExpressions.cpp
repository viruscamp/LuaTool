// Class LogicExpressions 
#include "LogicExpressions.h"
#include "Function.h"

#include <string>
using namespace std;

string& LogicExpressions::operator[] (PcAddr pc)
{
	return logicExp[pc];
}

void LogicExpressions::addCondition(PcAddr pc, PcAddr destPC, Op op, string left, string right)
{
	SingleCondition sc(pc, destPC, op, left, right);
	conditions.push_back(sc);
}

bool LogicExpressions::jumpsBetween(PcAddr start, PcAddr end)
{
	for (PcAddr pc = start+1; pc < end; pc++)
	{
		if (func->opMap[pc].opCode == OP_JMP)
			return true;
	}
	return false;
}