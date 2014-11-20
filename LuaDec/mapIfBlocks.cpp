#include "IfBlocks.h"
#include "Function.h"

using namespace std;

void IfBlocks::map(Function* func)
{
	for(int pc = 0; pc < func->codeSize; pc++)
	{
		Op iOp = func->opMap[pc];

		if (!iOp.isTest())
			continue; // this isn't a conditional
		if (func->logicExpressions[pc] == "")
			continue; // no logic expression for this test (it's part of a bigger chain)
		if (func->loopBlocks.getStart(pc) == "while " || func->loopBlocks.peekEnd(pc) == "until ")
			continue; // this isn't an if statement
		if (func->opMap[pc+2].opCode == OP_LOADBOOL && func->opMap[pc+2].c == 1)
			continue; // this isn't an if statement, it's a inline load bool

		Op jmp = func->opMap[pc+1];
		PcAddr destPC = jmp.sbx + pc + 2;
		PcAddr startPC, elsePC, endPC;
		
		// ========================================
		// find the base if, else and end addresses
		startPC = pc;
		if(func->opMap[destPC-1].opCode == OP_JMP && !func->opMap[destPC-2].isTest() 
			&& func->loopBlocks.getBreak(destPC-1) == "" && func->loopBlocks.peekEnd(destPC-1) == "")
		{ // matching else
			elsePC = destPC - 1;
			endPC = destPC + func->opMap[destPC-1].sbx;
		}
		else
		{// no else
			elsePC = -1;
			endPC = destPC;
		}

		// =======================================================================
		// find true end address - case when an outside else crosses an inside end
		for(int i=0; i < vec.size(); i++)
		{
			if (vec[i].elsePC != -1 &&
				vec[i].elsePC > startPC &&
				endPC > vec[i].elsePC && 
				elsePC != vec[i].elsePC)
			{
				endPC = vec[i].elsePC;
			}
		}

		// impossible
		if (endPC < startPC)
		{	// strange things may happen inside a while loop
			bool crossesWhileEnd = false;
			int pc2;
			for(pc2 = endPC; pc2 < func->codeSize; pc2++)
			{
				if (func->loopBlocks.getStart(pc2) == "while " && !func->assignmentsBetween(endPC, pc2))
				{
						crossesWhileEnd = true;
						break;
				}
			}

			if (crossesWhileEnd)
				endPC = func->loopBlocks.getEndPC(pc2);
		}

		// detect end-to-break optimization
		if (func->loopBlocks.peekEnd(endPC - 1) != "" && func->loopBlocks.getStartPC(endPC-1) < startPC)
		{
			for (PcAddr pc = startPC + 1; pc < endPC; pc++)
			{
				if (func->loopBlocks.getBreak(pc) != "")
				{
					endPC = pc;
					break;
				}
			}
		}

		SingleIfBlock ifB(startPC, elsePC, endPC);
		ifB.ifStatement = func->logicExpressions[pc];
		func->logicExpressions[pc].clear();

		vec.push_back(ifB);

		// detect elseif
		SingleIfBlock parentBlock = getParentBlock(vec.back().startPC);
		if (parentBlock.elsePC != -1 && vec.back().startPC >= parentBlock.elsePC && !func->assignmentsBetween(parentBlock.elsePC, vec.back().startPC))
		{
			if (parentBlock.elsePC == vec.back().endPC || parentBlock.endPC == vec.back().endPC-1 || parentBlock.endPC == vec.back().endPC) 
			{
				setElseIf(parentBlock.elsePC);
				vec.back().skipEnd = true;
			}
		}

	} // end for
}