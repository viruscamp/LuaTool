#include "Function.h"
#include "LoopBlocks.h"

void LoopBlocks::map(Function* functionToMap)
{
	func = functionToMap;

	// mapping order is important!
	mapWhiles();
	mapRepeats();
	mapFors();
	mapTfors();
	mapDos();
}

void LoopBlocks::mapWhiles()
{
	// move through code and search for backward jumps
	for(PcAddr pc = 0; pc < func->codeSize; pc++)
	{
		Op iOp = func->opMap[pc]; // current op
		// initial assumtions for start and end pc
		PcAddr startPC = iOp.sbx + pc + 1; // current op's jump destination
		PcAddr endPC = pc; // current op's pc

		// ==========
		// rejections
		if (iOp.opCode != OP_JMP)
			continue; // we're only looking for jumps
		if (startPC >= pc)
			continue; // we're only looking for backward jumps
		if (pc > 0 && func->opMap[pc-1].isTest())
			continue; // this isn't a while loop (it's an if statement or repeat loop)
		if (pc > 0 && func->opMap[pc-1].opCode == OP_TFORLOOP)
			continue; // this isn't a while loop (it's a tfor loop)
		// ========================================================
		// we're now sure that this is a WHILE loop
		// and we have to map it's real start, end and any possible break
			
		// search for the while condition - real start pc
		PcAddr infiniteLoopStartPC = startPC;
		for (startPC; startPC < endPC; startPC++)
		{
			bool foundCondition = false;
			//foundCondition = func->opMap[startPC].isTest() && func->opMap[startPC+1].opCode == OP_JMP;
			foundCondition = func->logicExpressions[startPC] != "";
			
			if (!foundCondition)
				continue; // no condition found, continue
			else
			{ // condition found, but is it the right one?
				PcAddr conditionDest = func->opMap[startPC+1].sbx + startPC+1 + 1;
				
				if (conditionDest == func->opMap[endPC+1].sbx+endPC+2) // else crossing correction
					conditionDest = endPC+1;

				if (conditionDest == endPC + 1 || conditionDest < startPC) // is this the while end?
					break; // right startPC found
			}
		}

		if (startPC == pc)
		{// no condition found => infinite while
			add(infiniteLoopStartPC, "while 1 do", endPC, "end");
		}
		else
		{// regular conditional while
			add(startPC, "while ", endPC, "end");
		}

		// find breaks
		for(PcAddr breakPC = startPC+2; breakPC < endPC; breakPC++)
		{
			Op breakOp = func->opMap[breakPC];

			if (breakOp.opCode != OP_JMP || func->opMap[breakPC-1].isTest())
				continue; // a break is an unconditional jump, so these cases are rejected

			PcAddr breakDest = breakOp.sbx + breakPC + 1;
			if (breakDest == endPC + 1)
				setBreak(breakPC);
		}
	}//end for
}

void LoopBlocks::mapRepeats()
{
	// move through code and search for backward jumps
	for(PcAddr pc = 0; pc < func->codeSize; pc++)
	{
		Op iOp = func->opMap[pc]; // current op
		// initial assumtions for start and end pc
		PcAddr startPC = iOp.sbx + pc + 1; // current op's jump destination
		PcAddr endPC = pc-1; // current op's pc


		// ==========
		// rejections
		if (iOp.opCode != OP_JMP)
			continue; // we're only looking for jumps
		if (startPC >= pc)
			continue; // we're only looking for backward jumps
		if (pc > 0 && func->opMap[pc-1].isTest() == false)
			continue; // this isn't a repeat loop (it's a while loop)
		
		bool fakeRepeat = false; 
		for(PcAddr pc2 = startPC; pc2 < func->codeSize; pc2++)
		{
			if (getStart(pc2) == "while " && func->assignmentsBetween(startPC, pc2) == false)
			{
					fakeRepeat = true;
					break;
			}
		}
		if (fakeRepeat)
			continue; // this isn't a repeat loop (it's an if statement inside a while loop) 
		// =================================================================================
		// we're now sure that this is a REPEAT loop
		// and we have to map it's start, end and any possible break

		add(startPC, "repeat", endPC, "until ");

		// find breaks
		for(PcAddr breakPC = startPC; breakPC < endPC; breakPC++)
		{
			Op breakOp = func->opMap[breakPC];

			if (breakOp.opCode != OP_JMP || func->opMap[breakPC-1].isTest())
				continue; // a break is an unconditional jump, so these cases are rejected

			PcAddr breakDest = breakOp.sbx + breakPC + 1;
			if (breakDest == endPC + 1)
				setBreak(breakPC);
		}
	}//end for
}

void LoopBlocks::mapFors()
{
	// move through code and search for OP_FORPREP
	for(PcAddr pc = 0; pc < func->codeSize; pc++)
	{
		Op iOp = func->opMap[pc]; // current op

		// rejections
		if (iOp.opCode != OP_FORPREP)
			continue; // we're only looking for for loops
  
		PcAddr destPC = iOp.sbx + pc + 1;
		add(pc, "for ", destPC, "end");

		// find breaks
		for(PcAddr breakPC = pc + 1; breakPC < destPC; breakPC++)
		{
			Op breakOp = func->opMap[breakPC];

			if (breakOp.opCode != OP_JMP || func->opMap[breakPC-1].isTest())
				continue; // a break is an unconditional jump, so these cases are rejected

			PcAddr breakDest = breakOp.sbx + breakPC + 1;
			if (breakDest == destPC + 1)
				setBreak(breakPC);
		}
	}//end for
}

void LoopBlocks::mapTfors()
{
	// move through code and search for backward jumps
	for(PcAddr pc = 0; pc < func->codeSize; pc++)
	{
		Op iOp = func->opMap[pc]; // current op

		// rejections
		if (iOp.opCode != OP_JMP)
			continue; // we're only looking for jumps
		if (pc > 0 && func->opMap[pc-1].isTest())
			continue; // this isn't a tfor loop
  		PcAddr destPC = iOp.sbx + pc + 1;
		if (func->opMap[destPC].opCode != OP_TFORLOOP)
			continue; // this isn't a tfor loop
		PcAddr startPC = func->opMap[destPC+1].sbx + destPC+1;
		if (startPC != pc)
			continue; // false positive

		add(pc, "for ", destPC+1, "end");

		// find breaks
		for(PcAddr breakPC = pc + 1; breakPC < destPC; breakPC++)
		{
			Op breakOp = func->opMap[breakPC];

			if (breakOp.opCode != OP_JMP || func->opMap[breakPC-1].isTest())
				continue; // a break is an unconditional jump, so these cases are rejected

			PcAddr breakDest = breakOp.sbx + breakPC + 1;
			if (breakDest == destPC + 2)
				setBreak(breakPC);
		}
	}//end for
}

void LoopBlocks::mapDos()
{
	// move through code and search for OP_CLOSE
	for(PcAddr pc = 0; pc < func->codeSize; pc++)
	{
		Op iOp = func->opMap[pc];
		if (iOp.opCode != OP_CLOSE)
			continue; // we're only looking for CLOSE

		add(func->locals[iOp.a].startPC, "do", func->locals[iOp.a].endPC, "end");
	}//end for
}