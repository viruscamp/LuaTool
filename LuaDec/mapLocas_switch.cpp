#include "Locals.h"
#include "Function.h"

extern "C" {
#include "lopcodes.h"
}

using namespace std;

void Locals::opSwitch(OpCode opCode)
{
	switch (opCode)
	{
	case OP_MOVE: opMove(); break;
	case OP_LOADK: opLoadK(); break;

	case OP_LOADBOOL: opLoadBool(); break;
	case OP_LOADNIL: opLoadNil(); break;
	
	case OP_VARARG:	opVarArg(); break;

	case OP_GETUPVAL: opGetUpval(); break;
	case OP_SETUPVAL: opSetUpval(); break;

	case OP_GETGLOBAL: opGetGlobal(); break;
	case OP_SETGLOBAL: opSetGlobal(); break;
	
	case OP_NEWTABLE: opNewTable(); break;
	case OP_GETTABLE: opGetTable(); break;
	case OP_SETTABLE: opSetTable(); break;
	case OP_SETLIST: opSetList(); break;

    case OP_SELF: opSelf(); break;
  
	case OP_ADD:
	case OP_SUB:
	case OP_MUL:
	case OP_DIV:
	case OP_POW:
	case OP_MOD: 
		opBinary(); 
		break;

	case OP_UNM:
	case OP_NOT:
	case OP_LEN:
		opUnary();
		break;

	case OP_CONCAT: opConcat(); break;

	case OP_JMP: opJmp(); break;
	
	case OP_EQ:
	case OP_LT:
	case OP_LE:
		opCompare();
		break;

	case OP_TESTSET: opTestSet(); break;
	case OP_TEST: opTest(); break;
		
	//case OP_FORLOOP: break;
	//case OP_TFORLOOP: break;

	case OP_FORPREP: opForPrep(); break;

	case OP_CALL: opCall();	break;
	case OP_TAILCALL: opTailCall();	break;

	case OP_RETURN: opReturn(); break;
	//case OP_CLOSE: break;
	case OP_CLOSURE:  opClosure(); break;

	default: break;
	}//end switch
}

void Locals::opMove()
{
	setReg(iOp.a);

	if (iOp.b <= iOp.a) 
		forceDeclare(iOp.b,iOp.b);
	else
	{ // a < b -- local declared at function start
		forceDeclare(iOp.a, iOp.a);
		if (potVar[iOp.a].empty())
			addPotVar(iOp.a, 0, func->codeSize-1);
	}

	loadReg(iOp.b);
}

void Locals::opLoadK()
{ 
	setReg(iOp.a);
}

void Locals::opLoadBool()
{
	setReg(iOp.a);
	
	// inline bool assignment
	if (iOp.c == 1)
		pcStep = 1;
}

void Locals::opLoadNil()
{ 
	setReg(iOp.a, iOp.b);

	if (pc == func->codeSize - 2)
		forceDeclare(iOp.a, iOp.b);
}

void Locals::opVarArg()
{
	if (iOp.b == 0) 
	{ // not sure about this one
		setReg(iOp.a, func->maxStackSize);
	}
	else
	{ // this one's good
		setReg(iOp.a, iOp.a + iOp.b - 1);
	}
}

void Locals::opGetUpval()
{
	setReg(iOp.a);
}

void Locals::opSetUpval()
{
	loadReg(iOp.a);
}

void Locals::opGetGlobal()
{
	setReg(iOp.a);
}

void Locals::opSetGlobal()
{
	loadReg(iOp.a);
}

void Locals::opNewTable()
{
	setReg(iOp.a);

	int tableAssignments = 0;
	if (iOp.b == 0)
	{ // keyed assignments
		tableAssignments += (iOp.c < 17 ? iOp.c : iOp.c + iOp.c - 17);
		func->opMap[pc].c = tableAssignments; // correction for later
		bool oneMore = tableAssignments >= 17;
		int firstA = iOp.a;

		while (tableAssignments != 0 && pc+pcStep < func->codeSize)
		{ // delay table declare until all assignments are done
			pcStep++;
			if (func->opMap[pc+pcStep].opCode == OP_NEWTABLE) // table inside iOp.a table
				tableAssignments += func->opMap[pc+pcStep].c;
			if (func->opMap[pc+pcStep].opCode == OP_SETTABLE)
				tableAssignments--;
		}

		if (oneMore && func->opMap[pc+pcStep+1].opCode == OP_SETTABLE && func->opMap[pc+pcStep+1].a == firstA)
		{
			func->opMap[pc].c++;
			pcStep++;
		}
	}
	else
	{
		tableAssignments += iOp.b;
		while (tableAssignments != 0 && pc+pcStep < func->codeSize)
		{ // delay table declare until all assignments are done
			pcStep++;
			if (func->opMap[pc+pcStep].opCode == OP_LOADK)
				tableAssignments--;
			if (func->opMap[pc+pcStep].opCode == OP_SETLIST)
				tableAssignments = 0;
		}
	}
}

void Locals::opGetTable()
{
	setReg(iOp.a);
	loadReg(iOp.b);

	if (iOp.b < iOp.a)
		forceDeclare(0, iOp.b);

	if (!isConst(iOp.c))
	{
		loadReg2(iOp.c);

		if (iOp.c < iOp.a && iOp.c < iOp.b)
			forceDeclare(0, iOp.c);
	}
}

void Locals::opSetTable()
{
	loadReg(iOp.a);
	if (!isConst(iOp.b))
		loadReg2(iOp.b);

	int forceDeclareTo = -1;

	if (!isConst(iOp.c))
	{
		loadReg3(iOp.c);

		if ( iOp.a+1 != iOp.c && iOp.c > iOp.a && isConst(iOp.b) && useInfo[pc].multiCall == -1)
			forceDeclareTo = iOp.c-1;
	}

	if (iOp.a-1 >= forceDeclareTo)
		forceDeclareTo = iOp.a-1;

	if (isConst(iOp.b) && isConst(iOp.c) && pc > 0 && useInfo[pc-1].setReg != iOp.a)
		forceDeclareTo = iOp.a;

	forceDeclare(0, forceDeclareTo);
}

void Locals::opSetList()
{
	setReg(iOp.a);
	loadReg(iOp.a);
	/*//2.0//
	if (iOp.b==0) {
		loadRegTo = f->maxstacksize;
	} else {
		loadRegTo = iOp.a+iOp.b;
	}*/
}

void Locals::opSelf()
{
	setReg(iOp.a, iOp.a+1);
	loadReg(iOp.b);

	if (iOp.a > iOp.b) 
		forceDeclare(0, iOp.b);

	if (!isConst(iOp.c))
		loadReg2(iOp.c);
}

void Locals::opBinary()
{
	setReg(iOp.a);
	if (!isConst(iOp.b))
		loadReg(iOp.b);

	if (!isConst(iOp.c))
		loadReg2(iOp.c);

	if (iOp.a == iOp.b || iOp.a == iOp.c)
		useInfo[pc].selfLoad = iOp.a;
}

void Locals::opUnary()
{
	setReg(iOp.a);
	loadReg(iOp.b);
}

void Locals::opConcat()
{
	setReg(iOp.a);
	loadReg(iOp.b, iOp.c);
}

void Locals::opCall()
{
	if (iOp.c == 0) 
		setReg(iOp.a, func->maxStackSize);
	else if (iOp.c >= 2)
		setReg(iOp.a, iOp.a+iOp.c-2);
	
	if (iOp.c == 1)
		forceDeclare(0, iOp.a-1);
	
	// multiCall
	if (iOp.c > 2 && func->opMap[pc+1].opCode == OP_SETTABLE)
		for (int i = 1; i < iOp.a+iOp.c-2; i++)
			useInfo[pc+i].multiCall = iOp.a+iOp.c-2;

	if (iOp.b == 0) 
		loadReg(iOp.a, func->maxStackSize);
	else
		loadReg(iOp.a, iOp.a+iOp.b-1);
}

void Locals::opTailCall()
{
	if (iOp.c == 0) 
		setReg(iOp.a, func->maxStackSize);
	else if (iOp.c >= 2)
		setReg(iOp.a, iOp.a+iOp.c-2);

	if ( iOp.b==0 )
		loadReg(iOp.a, func->maxStackSize);
	else
		loadReg(iOp.a, iOp.a+iOp.b-1);
}

void Locals::opReturn()
{
	if (iOp.b == 0) 
		loadReg(iOp.a, func->maxStackSize);
	else if (iOp.b >= 2)
		loadReg(iOp.a, iOp.a+iOp.b-2);

	if (pc > 0) {
		// local declare before return
		if (func->opMap[pc-1].opCode == OP_CALL && iOp.b == 2 && iOp.a == func->opMap[pc-1].a)
			forceDeclare(iOp.a, iOp.a);

		if (useInfo[pc-1].setReg != iOp.a && iOp.b == 2)
			forceDeclare(iOp.a, iOp.a);
	}
}

void Locals::opClosure()
{
	setReg(iOp.a);
}

void Locals::opCompare()
{
	if (!isConst(iOp.b))
		loadReg(iOp.b);
		
	if (!isConst(iOp.c))
		loadReg2(iOp.c);
}

void Locals::opTestSet()
{
	setReg(iOp.a);
	loadReg(iOp.b);
}

void Locals::opTest()
{
	loadReg(iOp.a);
	forceDeclare(iOp.a-1, iOp.a-1);
}

void Locals::opForPrep()
{
	int dest = pc + iOp.sbx + 2;

	loadReg(iOp.a, iOp.a+2);
	setReg(iOp.a, iOp.a+3);
	forceDeclare(iOp.a, iOp.a+3);
	
	addPotVar(iOp.a, pc, dest);
	addPotVar(iOp.a+1, pc, dest);
	addPotVar(iOp.a+2, pc, dest);
	addPotVar(iOp.a+3, pc, dest);
}

void Locals::opJmp()
{
	int dest = pc + iOp.sbx + 2;

	if (func->opMap[dest-1].opCode == OP_TFORLOOP) 
	{
		int a = func->opMap[dest-1].a;
		int c = func->opMap[dest-1].c;
		
		setReg(a, a+c+2);
		loadReg(a, a+2);
		//forceDeclare(a, a+c+2);
		forceDeclare(0, a+c+2);

		addPotVar(a, pc, dest+1);
		addPotVar(a+1, pc, dest+1);
		addPotVar(a+2, pc, dest+1);

		for (int i = a+3; i <= a+c+2; i++) 
			addPotVar(i, pc, dest-1);
	}
}