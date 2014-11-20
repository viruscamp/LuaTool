// Class LogicExpressions 
#include "LogicExpressions.h"
#include "Function.h"

#include <string>
#include <sstream>
#include <deque>
#include <algorithm>
using namespace std;

void LogicExpressions::mapFunction(Function *functionToMap)
{
	func = functionToMap;

	while (!conditions.empty())
	{
		int chainSize = 1;
		// recognize a single logic chain
		for (unsigned i = 1; i < conditions.size(); i++)
		{
			if (func->assignmentsBetween(conditions[i-1].pc, conditions[i].pc) == false
				&& jumpsBetween(conditions[i-1].pc + 1, conditions[i].pc) == false)
				chainSize++;
			else
				break;
		}

		// exclude false chains
		PcAddr firstDestPC = conditions[0].destPC;
		PcAddr firstStartPC = conditions[0].pc + 2;
		if (firstDestPC == firstStartPC)
			chainSize = 1;
		else
		{
			//for (int i = chainSize-1; i >= 1; i--)
			//{
			//	PcAddr destPC = conditions[i].destPC;
			//	PcAddr startPC = conditions[i].pc + 2;
			//	if (firstDestPC > startPC && destPC != firstDestPC)
			//	{
			//		chainSize = i;
			//		//break;
			//	}
			//	else
			//		break;
			//}

			for (int i = chainSize-2; i >= 0; i--)
			{
				PcAddr lastDestPC = conditions[chainSize-1].destPC;
				PcAddr lastStartPC = conditions[chainSize-1].pc + 2;
				PcAddr destPC = conditions[i].destPC;
				PcAddr startPC = conditions[i].pc + 2;
				if (lastDestPC != destPC && destPC > lastStartPC)
				{
					chainSize = i+1;
					//break;
				}
			}

		}

		mapExpression(0, chainSize-1);

		stringstream ss;
		for (int i = 0; i < chainSize; i++)
		{
			ss << conditions[i].left;
			ss << conditions[i].op.getOpString();
			ss << conditions[i].right; 
			ss << conditions[i].chain;
		}

		logicExp[conditions[chainSize-1].pc] = ss.str();

		for (unsigned i = 0; i < chainSize; i++)
			conditions.erase(conditions.begin());
	}
}

int LogicExpressions::mapExpression(int start, int end, bool isMain)
{
	if (end - start < 1)
		return -1; // nothing to do with a single condition

	// map base ands
	PcAddr lastDestPC = conditions[end].destPC;
	PcAddr lastStartPC = conditions[end].pc + 2;
	bool baseFound = false;
	for (int i = start; i < end; i++)
	{
		if (conditions[i].chain != "")
			continue;

		PcAddr destPC = conditions[i].destPC;
		if (destPC == lastDestPC)
		{
			conditions[i].base = true;
			conditions[i].chain = " and ";
			baseFound = true;
		}
	}

	int prev = -1;
	int mo = -1;
	for (int i = start; i < end; i++)
	{
		if (prev == -1 && conditions[i].base == false)
			prev = i;
		
		if (prev != -1 && mo == -1 && baseFound && conditions[i].base == true)
		{
			mo = mapExpression(prev, i, false);
			
			if (mo == -1)
				prev = -1;
		}

		if (mo != -1 && i == mo)
		{
			mo = mapExpression(prev, i, false);
			
			if (mo == -1)
				prev = -1;
		}
	}
	if (prev != -1 && baseFound && prev != start)
		mapExpression(prev, end, false);

	// map everything else
	int magicOr = -1;
	for (int i = end - 1; i >= start; i--)
	{
		PcAddr destPC = conditions[i].destPC;
		
		if (conditions[i].chain != "")
			continue;

		for (unsigned j = i+1; j <= end; j++)
		{
			if (conditions[j].pc + 2 == destPC)
			{
				if (conditions[j].chain == " and " || conditions[j].pc + 2 == lastStartPC)
				{
					conditions[i].chain = " or ";
					invertOp(conditions[i].op);
				}
				else
					conditions[i].chain = " and ";
				break;
			}
			
			if (j == end)
			{
				conditions[i].chain = " or ";
				invertOp(conditions[i].op);
				magicOr = conditions[i].destPC;
			}
		}// end for j
	}// end for i
	
	// add paranthesis
	if (isMain == false && magicOr == -1)
	{
		conditions[start].left.insert(0, "(");
		conditions[end].right.append(")");
	}

	return magicOr;
}

void LogicExpressions::invertOp(Op &op)
{
	if (op.opCode == OP_TEST || op.opCode == OP_TESTSET)
		op.c = !op.c;
	else
		op.a = !op.a;
}