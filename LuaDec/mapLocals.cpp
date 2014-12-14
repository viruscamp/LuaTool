#include "Locals.h"
#include "Function.h"

extern "C" {
#include "lobject.h"
}

#include <sstream>
using namespace std;

void Locals::mapProto(Proto* f)
{
	// if available, get local variables from proto
	for (int i=0; i < f->sizelocvars; i++)
		declareLocal(i, f->locvars[i].startpc, f->locvars[i].endpc, (char*)getstr(f->locvars[i].varname));
}

void Locals::mapFunction(Function* functionToMap)
{
	func = functionToMap;

	// debug
	df = false;

	if (func->locals.size() > 0)
		return; // local info wasn't stripted, no need to map locals

	prepare();
	mapUsage();
	mapLocals();
	nameLocals();
}

void Locals::prepare()
{
	pc = 0;
	block = 0;
	lastfree = 0;
	addBlock(0, func->codeSize - 1, 0);

	// parameters, varargs, nil optimizations
	for (int i = 0; i < func->numParameters; i++) 
	{
		addPotVar(lastfree, 0, blockEnd(pc));
		declareLocal(lastfree, 0, blockEnd(pc) );
		lastfree++;
	}

	if ( func->isGlobal == false && (func->isVarArg || func->isVarArg2) )
	{
		addPotVar(lastfree, 0, blockEnd(pc));
		declareLocal(lastfree, 0, blockEnd(pc) );
		locals.back().name = "arg";
		lastfree++;
    }

	Op iOp = func->opMap[0];
	if ( iOp.opCode == OP_SETGLOBAL || iOp.opCode == OP_SETUPVAL) 
	{
		for (int i = lastfree; i <= iOp.a; i++)
		{
			addPotVar(lastfree, 0, blockEnd(pc));
			regUsage[lastfree].setLevel(1, pc);

			if (i != iOp.a) 
			{
				declareLocal(lastfree, 0, blockEnd(pc) );
				lastfree++;
			}

			//addPotVar(lastfree, 0, blockEnd(pc));
			//regUsage[lastfree].setLevel(1, pc);
		}
	} 
	else if (iOp.opCode != OP_JMP)
	{
		for (int i = lastfree; i < iOp.a; i++)
		{
			addPotVar(lastfree, 0, blockEnd(pc));
			declareLocal(lastfree, 0, blockEnd(pc) );
			lastfree++;
		}
	}
}

void Locals::mapUsage()
{
	// pre-pass to map repeats
	for (pc = 0; pc < func->codeSize; pc++)
	{
		iOp = func->opMap[pc];

		if (iOp.opCode == OP_JMP && func->opMap[pc-1].isTest() && iOp.sbx < 0)
			addRepeat(pc + iOp.sbx + 1, pc, -1);
	}

	// pre-pass to map other blocks
	for (pc = 0; pc < func->codeSize; pc++)
	{
		iOp = func->opMap[pc];

		if (iOp.opCode == OP_JMP && iOp.sbx > 0)
		{
			int dest = pc + iOp.sbx + 1;
			if (func->opMap[dest-1].opCode == OP_JMP && func->opMap[dest-2].isTest() == false)
				dest = dest-1;

			if (!(func->opMap[dest-1].opCode == OP_LOADBOOL && func->opMap[dest-1].c == 1/* && dest == pc + 2*/) )
			{
				block++;
				addBlock(pc, dest, block);
			}
		}
		else if (iOp.opCode == OP_FORPREP)
		{
			int dest = pc + iOp.sbx + 2;
			block++;
			if (func->opMap[dest-2].opCode == OP_JMP) 
				addBlock(pc, dest-2, block);
			else
				addBlock(pc, dest-1, block);
		}
	
		for (int i = 0; i < blockInfo.size(); i++)
		{
			if (blockInfo[i].end == pc+1)
				block--;
		}
	}

	// main pass to map usage
	for (pc = 0; pc < func->codeSize; pc++)
	{
		iOp = func->opMap[pc];
		pcStep = 0;

		opSwitch(iOp.opCode);

		// usage--
		if (useInfo[pc].loadReg != -1)
		{
			for (int i = useInfo[pc].loadReg; i <= useInfo[pc].loadRegTo; i++)
				regUsage[i].dec(pc);

			if (useInfo[pc].loadReg2!=-1) 
				regUsage[useInfo[pc].loadReg2].dec(pc);
			if (useInfo[pc].loadReg3!=-1) 
				regUsage[useInfo[pc].loadReg3].dec(pc);
		}

		// usage++
		if (useInfo[pc].setReg != -1) 
		{
			for (int i = useInfo[pc].setReg; i <= useInfo[pc].setRegTo; i++)
				regUsage[i].inc(pc);

			if (useInfo[pc].setReg2 != -1) 
				regUsage[useInfo[pc].setReg2].inc(pc);
		}

		for (int i = useInfo[pc].setReg; i <= useInfo[pc].setRegTo; i++) 
		{
			addPotVar(i, pc+1+pcStep, blockEnd(pc+1));
		}

		pc += pcStep;
	}// end for
}

void Locals::mapLocals()
{
	for (pc = 0; pc < func->codeSize; pc++)
	{
		// which registers should be declared?
		int declareTo = lastfree-1;
		bool lastZero = regUsage[lastfree][pc] == 0; // last free registers usage is zero
		bool blockEndDeclare = pc == blockEnd(pc) && pc != func->codeSize-1; // block end, but not function end
		for (int i=lastfree; i < func->maxStackSize; i++) 
		{
			if ( lastZero && regUsage[i][pc] != 0 )
			{	// in a last free zero case, the first non-zero usage triggers declaration
				declareTo = i-1; 
				lastZero = false;   
			}

			if ( blockEndDeclare && regUsage[i][pc-1] == 1)
				declareTo = i; // at the end of a block ones also triggers declaration

			if ( regUsage[i][pc] < 0 || regUsage[i][pc] > 1 ) 
				declareTo = i; // register usage outside margins triggers declaration

			if ( useInfo[pc].forceDeclareFrom <= i && i <= useInfo[pc].forceDeclareTo )
				declareTo = i; // special case overrides can force a declaration

			if ( useInfo[pc].selfLoad == i && regUsage[i][pc] == 0)
				declareTo = i; // TODO: replace this with a forceDeclare
		}

		/*if (declareTo == lastfree-1 && pc == blockEnd(pc) && pc != func->codeSize-1)
		{
			for (int i=lastfree; i < func->maxStackSize; i++) 
			{
				if (regUsage[i][pc-1] == 1)
					declareTo = i; 
			}
		}*/

		// save found locals
		for (int i=lastfree; i <= declareTo; i++) 
		{ 
			declareLocal(lastfree, pc);
			lastfree++;
		}

		// block end
		for (int i = 0; i < locals.size(); i++)
		{
			if (locals[i].endPC == pc && pc != func->codeSize-1)
			{
				lastfree--;
				regUsage[lastfree].normalize(pc);
			}
		}
	}

	// declare anything left in usage state 1
	int declareTo = lastfree-1;
	for (int i=lastfree; i < func->maxStackSize; i++)
	{
		if (regUsage[i][pc] == 1)
			declareTo = i;
	}

	for (int i=lastfree; i <= declareTo; i++) 
	{ 
		if (func->opMap[potVar[i].back().start].opCode == OP_LOADNIL)
			potVar[i].pop_back();

		declareLocal(lastfree, pc-1);
		lastfree++;
	}

	// subfunction upvalues should also be declared
	for(PcAddr tpc = 0; tpc < func->codeSize; tpc++)
	{
		Op tOp = func->opMap[tpc];
		if (tOp.opCode != OP_CLOSURE)
			continue;
		// upvalues are defined after the function closure

		int uvn = func->subFunctions[tOp.bx]->upvalues.size();

		for (int i=0; i<uvn; i++)
		{
			Op curOp = func->opMap[tpc+i+1];
			if (curOp.opCode == OP_MOVE && lastfree <= curOp.b) 
			{
				declareLocal(lastfree, tpc);
				lastfree++;
			} 
		} // end inner for
	} // end outer for

	// clean up
	for(int i=0; i < locals.size(); i++)
	{
		for(int j=i+1; j < locals.size(); j++)
		{
			if (locals[i].reg == locals[j].reg 
				&& locals[j].startPC < locals[i].startPC)
			{
				locals[i] = locals[j];
				locals.erase(locals.begin() + j);
			}
		}
	}
}

void Locals::nameLocals()
{
	for(int i=0; i < locals.size(); i++)
	{
		if (locals[i].name == "")
		{
			stringstream ss;
			ss << "l_" << func->funcNumber << "_" << i;
			locals[i].name = ss.str();
		}
	}
}