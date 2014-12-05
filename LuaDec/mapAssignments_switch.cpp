// Class Assignments implementation part I 
// the op switch and individual op handling functions
#include "Assignments.h"
#include "Function.h"

#include <string>
#include <sstream>
using namespace std;

void Assignments::opSwitch(OpCode opCode)
{ // there is a case for every opcode in Lua 5.1
  // the commented out cases don't need any handling in Class Assignments
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

	case OP_JMP: opGeneralFor(); break;
	
	case OP_EQ:
	case OP_LT:
	case OP_LE:
		opCompare();
		break;

	case OP_TESTSET:
	case OP_TEST: 
		opTest();
		break;

	//case OP_FORLOOP: break;
	//case OP_TFORLOOP: break;

	case OP_FORPREP: opNumericFor(); break;

	case OP_CALL:
	case OP_TAILCALL: 
		opCall();
		break;

	case OP_RETURN: opReturn(); break;
	//case OP_CLOSE: break;
	case OP_CLOSURE:  opClosure(); break;

	default: break;
	}//end switch
}

void Assignments::opMove()
{
    if (iOp.a == iOp.b)
		return;
	
	if (func->reg[iOp.b].call < 2)
		assign(func->reg[iOp.a], func->reg[iOp.b], func->reg[iOp.b].priority);
	else
		func->reg[iOp.b].isPending = false;
}

void Assignments::opLoadK()
{ 
	assign(func->reg[iOp.a], func->reg.getBxConstant(iOp.bx));
}

void Assignments::opLoadBool()
{
	string strBool;
	if (iOp.b == 1)
		strBool = "true";
	else
		strBool = "false";

	assign(func->reg[iOp.a], strBool);
}

void Assignments::opLoadNil()
{ 
	if (func->reg[iOp.a].isLocal == false)
	{ // regular assignments
		for(int i = iOp.a; i <= iOp.b; i++)
			assign(func->reg[i], "nil");
	}
	else
	{ // in case the registers are locals, write assignments right away
		stringstream ss;
		for (int i = iOp.a; i < iOp.b; i++)
			ss << func->reg[i].getValue() << " = nil" << endl;
		ss << func->reg[iOp.b].getValue() << " = nil";

		add(ss.str());
	}
}

void Assignments::opVarArg()
{
	if (iOp.b == 0)
		assign(func->reg[iOp.a], "...");
	else
	{
		for(int i = 0; i < iOp.b; i++)
			assign(func->reg[iOp.a+1], "...");
	}
}

void Assignments::opGetUpval()
{
	assign(func->reg[iOp.a], func->upvalues[iOp.b]);
}

void Assignments::opSetUpval()
{
	add(func->upvalues[iOp.b] + " = " + func->reg[iOp.a].getValue());
}

void Assignments::opGetGlobal()
{
	assign(func->reg[iOp.a], func->reg.getGlobal(iOp.bx));
}

void Assignments::opSetGlobal()
{
	add(func->reg.getGlobal(iOp.bx) + " = " + func->reg[iOp.a].getValue());
}

void Assignments::opNewTable()
{
	DecTable newTable(iOp.b, iOp.c);

	func->reg[iOp.a].table = newTable;

	if (func->reg[iOp.a].table.isFull())
		assign(func->reg[iOp.a], func->reg[iOp.a].table.getString());
}

void Assignments::opGetTable()
{
	stringstream ss;
	string bStr = func->reg[iOp.b].getValue();
	if (bStr[0] == '{')
		ss <<"("<< bStr <<")";
	else
		ss << bStr;

	ss << makeIndex(func->reg[iOp.c].getValue());
	
	assign(func->reg[iOp.a], ss.str());
}

void Assignments::opSetTable()
{
	if (func->reg[iOp.a].table.isFull())
	{ // table is already generated, just assign it
		stringstream ss;
		ss << func->reg[iOp.a].getValue() << makeIndex(func->reg[iOp.b].getValue());
		assign(ss.str(), func->reg[iOp.c].getValue());
	}
	else
	{ // add to table being generated
		func->reg[iOp.a].table.setTable(func->reg[iOp.b].getValue(), func->reg[iOp.c].getValue());

		// if it's full now, assign it
		if (func->reg[iOp.a].table.isFull())
			assign(func->reg[iOp.a], func->reg[iOp.a].table.getString());
	}
}

void Assignments::opSetList()
{
   for (int i = 1; i <= iOp.b; i++)
   {  // TODO: this does not acount for field C
	   func->reg[iOp.a].table.setNumeric(i, func->reg[iOp.a+i].getValue());
   }

   // if it's full now, assign it
   if (func->reg[iOp.a].table.isFull())
	assign(func->reg[iOp.a], func->reg[iOp.a].table.getString());
}

void Assignments::opSelf()
{
	assign(func->reg[iOp.a+1], func->reg[iOp.b], func->reg[iOp.b].priority);

	stringstream ss;
	ss << func->reg[iOp.b].getValue();
	ss << makeIndex(func->reg[iOp.c].getValue(), true);
	assign(func->reg[iOp.a], ss.str());
}

void Assignments::opBinary()
{
	int currentPriority = iOp.getOpPriority();
 	string bStr = func->reg[iOp.b].getValue();
	string cStr = func->reg[iOp.c].getValue();

	stringstream ss;
	// FIXME: might need to change from <= to < here
	if ( (currentPriority != 1 && func->reg[iOp.b].priority <= currentPriority) 
		|| (currentPriority == 1 && bStr[0] != '-'))
	{
		ss << bStr;
	}
	else 
		ss << "(" << bStr << ")";

	ss << " " << iOp.getOpString() << " ";

	// FIXME: being conservative in the use of parentheses
	if (func->reg[iOp.c].priority < currentPriority) 
		ss << cStr;
	else
		ss << "(" << cStr << ")";

	assign(func->reg[iOp.a], ss.str(), currentPriority);
}

void Assignments::opUnary()
{
	int currentPriority = iOp.getOpPriority();

	stringstream ss;
	ss << iOp.getOpString();

	if (func->reg[iOp.b].priority <= currentPriority)
		ss << func->reg[iOp.b].getValue();
	else
		ss << "(" << func->reg[iOp.b].getValue() << ")";
		
	assign(func->reg[iOp.a], ss.str());
}

void Assignments::opConcat()
{
	stringstream ss;
	for (int i = iOp.b; i <= iOp.c; i++)
	{
		if (func->reg[i].priority > iOp.getOpPriority())
			ss << "(" << func->reg[i].getValue() << ")";
		else
			ss << func->reg[i].getValue();

		if (i < iOp.c)
			ss << " .. ";
	}
	assign(func->reg[iOp.a], ss.str());
}

void Assignments::opCall()
{
	int limit;
	if (iOp.b == 0) 
	{
		limit = iOp.a + 1;
		while (func->reg[limit].isPending || func->reg[limit].isLocal)
			limit++;
	} 
	else
		limit = iOp.a + iOp.b;

	stringstream ss;
	if (iOp.opCode == OP_TAILCALL)
		ss << "return ";

	string aStr = func->reg[iOp.a].getValue();
	ss << aStr << "(";
    
	bool self = false;
	if (aStr.find(":") != string::npos)
		self = true;

	for (int i = iOp.a+1; i < limit; i++)
	{
		string iStr = func->reg[i].getValue();

		if (self && i == iOp.a+1)
			continue;

		if  ((!self && i > iOp.a+1) || (self && i > iOp.a+2) )
			ss << ", ";
		ss << iStr;
	}
	ss << ")";

	string str = ss.str();
  
	if (iOp.opCode == OP_TAILCALL || iOp.c == 1)
		add(str);
	else
	{
		assign(func->reg[iOp.a], str);

		for (int i = 1; i <= iOp.c-2; i++)
			assign(func->reg[iOp.a+i], "");
	
		// TODO: remove this
		for (int i = 0; i <= iOp.c-2; i++) 
			func->reg[iOp.a+i].call = 1;
	}
}

void Assignments::opReturn()
{
	if (func->opMap[pc-1].opCode == OP_TAILCALL)
		return; // tailcall already handles the return
	if (pc == func->codeSize - 1)
		return; // last return -> end function

	int limit;
	if (iOp.b == 0)
	{
		limit = iOp.a;
		while (func->reg[limit].isPending || func->reg[limit].isLocal) 
			limit++;
	}
	else
	{
		limit = iOp.a + iOp.b - 1;
	}

	stringstream ss;
	ss << "return ";

	for (int i = iOp.a; i < limit; i++)
	{
		if (i > iOp.a)
			ss << ", ";
		ss << func->reg[i].getValue();
	}

	add(ss.str());
}

void Assignments::opClosure()
{
	// cosmetic: write an empty line before first function
	if (func->subFunctions[iOp.bx].funcNumber == "0_0")
		add(" ");

	// decompile subfunction
	stringstream decompiledFunction;
	decompiledFunction << func->subFunctions[iOp.bx].decompile(func->indent);

	// cosmetic: write an empty line after ground level functions
	if (func->indent == 0)
		decompiledFunction << endl;

	assign(func->reg[iOp.a], decompiledFunction.str());
	
	pc += func->subFunctions[iOp.bx].upvalues.size(); // ignore the following upvalues
}

void Assignments::opCompare()
{
	PcAddr dest = func->opMap[pc+1].sbx + pc+1;

	//if (func->opMap[pc+2].opCode == OP_LOADBOOL && func->opMap[pc+2].c == 1)
	if (func->opMap[dest].opCode == OP_LOADBOOL && func->opMap[dest].c == 1)
	{ // inline load bool
		iOp.a = !iOp.a;
		
		stringstream ss;
		ss << func->reg[iOp.b].getValue() << iOp.getOpString() << func->reg[iOp.c].getValue();

		pc++;
		for ( ; pc < dest; pc++) // this is a complex inline bool
		{
			iOp = func->opMap[pc];

			if (iOp.isTest() == false)
			{
				if (iOp.opCode != OP_JMP && iOp.opCode != OP_LOADBOOL)
					opSwitch(iOp.opCode);
				continue;
			}

			iOp.a = !iOp.a;
			ss << " or ";
			ss << func->reg[iOp.b].getValue() << iOp.getOpString() << func->reg[iOp.c].getValue();
		}
		
		//assign(func->reg[func->opMap[pc+2].a], ss.str());
		assign(func->reg[func->opMap[dest].a], ss.str());
		//pc += 3; // skip the opLoadBools
		pc = dest+1; // skip the opLoadBools
	}
	else
	{ // normal compare

		// TODO: this is to complicated, change it
		bool invertCompare = false;
		if (iOp.opCode == OP_LT || iOp.opCode == OP_LE)
		{	
			if (!func->reg[iOp.b].isConstant && !func->reg[iOp.c].isConstant)
			{
				for (int pc2 = pc-1; pc2 >= 0; pc2--)
				{
					if (func->opMap[pc2].a == iOp.b)
					{
						invertCompare = true;
						break;
					}
					else if (func->opMap[pc2].a == iOp.c)
					{
						invertCompare = false;
						break;
					}
				}
			}

			if (func->reg[iOp.b].isConstant || invertCompare)
			{ //invert compare order
				int swap = iOp.b;
				iOp.b = iOp.c;
				iOp.c = swap;
				iOp.a = !iOp.a;
			}
		}

		Op jmp = func->opMap[pc+1];
		PcAddr destPC = jmp.sbx + pc + 2;

		func->logicExpressions.addCondition(pc, destPC, iOp, func->reg[iOp.b].getValue(), func->reg[iOp.c].getValue());

		//stringstream ss;
		//ss << func->reg[iOp.b].getValue() <<" "<< iOp.getOpString() <<" "<< func->reg[iOp.c].getValue();
		//
		//add(ss.str());
	}
}

void Assignments::opTest()
{
	if (iOp.opCode == OP_TEST)
		iOp.b = iOp.a;

	Op jmp = func->opMap[pc+1];
	PcAddr destPC = jmp.sbx + pc + 2;

	func->logicExpressions.addCondition(pc, destPC, iOp, "", func->reg[iOp.b].getValue());

	//stringstream ss;
	//if (iOp.getOpString() != "")
	//	ss << iOp.getOpString() <<" "<< func->reg[iOp.b].getValue();
	//else
	//	ss << func->reg[iOp.b].getValue();

	//add(ss.str());
}

void Assignments::opNumericFor()
{
	string idxname;
	if (func->reg[iOp.a+3].isLocal == false)
	{
		int loopvars = 0;
		for (unsigned i = 0; i < func->locals.size(); i++)
		{
			if (func->locals[i].startPC == pc + 1)
			{
				loopvars++;
				//search for the loop variable. Set it's endpc one step further so it will be the same for all loop variables
				if (func->opMap[func->locals[i].endPC-1].opCode == OP_FORLOOP)
					func->locals[i].endPC -=1;

				if (loopvars == 4)
				{ // loop counter has been declared before the loop											
					idxname = func->locals[i].name;
					break;
				}
			}//end if
		}// end for
		if (idxname == "")
		{	// if nothing is found (shouldn't happen) use this
			idxname = "i";
			func->reg[iOp.a+3].setLocal(idxname);
		}
	} 
	else
	{ // counter is not a local
		idxname = func->reg[iOp.a+3].getValue();
	}

	string startVal = func->reg[iOp.a].getValue();
	string endVal = func->reg[iOp.a+1].getValue();
	string stepVal = func->reg[iOp.a+2].getValue();
	
	stringstream ss;
	if (stepVal == "1")
		ss << idxname <<" = "<< startVal <<", "<< endVal;
	else
		ss << idxname <<" = "<< startVal <<", "<< endVal <<", "<< stepVal;

	func->reg[iOp.a].isInternal = true;
	func->reg[iOp.a+1].isInternal = true;
	func->reg[iOp.a+2].isInternal = true;
	func->reg[iOp.a+3].isInternal = true;
	
	add(ss.str());
}

void Assignments::opGeneralFor()
{
	PcAddr dest = iOp.sbx + pc + 1;

	if(func->opMap[pc-1].isTest() || func->opMap[dest].opCode != OP_TFORLOOP)
		return; // this isn't a general for loop
	PcAddr startPC = func->opMap[dest+1].sbx + dest+1;
	if (startPC != pc)
		return; // false positive


	iOp = func->opMap[dest];

	int step;
	string generator;
	string control;
	string state;
	char *variables[20];
	map<int, string> vname;
	int stepLen;

	generator = func->reg[iOp.a].getValue();
	state = func->reg[iOp.a+1].getValue();
	control = func->reg[iOp.a+2].getValue();

	for (int i = 1; i <= iOp.c; i++)
	{
		if (func->reg[iOp.a+2+i].isLocal == false)
		{
			int loopvars = 0;
			for (int i2 = 0; i2 < func->locals.size(); i2++)
			{
				if (func->locals[i2].startPC == pc + 1)
				{
					loopvars++;
					//search for the loop variable. Set it's endpc one step further so it will be the same for all loop variables
					if (func->opMap[func->locals[i2].endPC-2].opCode == OP_TFORLOOP)
						func->locals[i2].endPC -= 2;
					
					if (func->opMap[func->locals[i2].endPC-1].opCode == OP_TFORLOOP) 
						func->locals[i2].endPC -= 1;
					
					if (loopvars==3+i)
					{											
						vname[i-1] = func->locals[i2].name;
						break;
					}
				}// end if
			}//end for
		
			if (vname[i-1] == "") 
			{
				stringstream ss;
				ss << "i_" << i;
				vname[i-1] = ss.str();
				func->reg[iOp.a+i+2].setLocal(vname[i-1]);
			}
		} 
		else 
		   vname[i-1] = func->reg[iOp.a+2+i].getValue();

		func->reg[iOp.a+2+i].isInternal = true;
	}// end for

	
	stringstream ss;
	ss << vname[0];
	
	for (int i=2; i<=iOp.c; i++)
		ss <<","<< vname[i-1];

	ss << " in " << generator;

	func->reg[iOp.a].isInternal = true;
	func->reg[iOp.a+1].isInternal = true;
	func->reg[iOp.a+2].isInternal = true;
	
	add(ss.str());
}