// Class Assignments implementation part II
// the base implementation methods
#include "Assignments.h"
#include "Function.h"
#include "Register.h"

#include <sstream>
using namespace std;

void Assignments::mapFunction(Function* functionToMap)
{
	func = functionToMap;

	for(pc=0; pc < func->codeSize; pc++)
	{
		iOp = func->opMap[pc]; // current operation

		declareLocals();
		releaseLocals();

		opSwitch(iOp.opCode);

		flushPending();
	}
}

void Assignments::assign(Register& a, Register& b, int priority)
{
	if (a.isPending)
	{
		func->decErrors[pc].append(" -- Overwrote pending register. (Assignments::assign)");
		return;
	}

	a.isPending = true;
	a.call = b.call;
	a.priority = priority;

	if (a.isLocal == false)
	{ // temp register
		a.value = b.getValue();
		a.isTemp = true;
	} 
	else
	{ // local variable
		Assignments::Pending p;
		p.r = &a;
		p.right = b.getValue();
		pending.push_back(p);
	}
}

void Assignments::assign(Register& a, string constVal, int priority)
{
	if (a.isPending)
	{
		func->decErrors[pc].append(" -- Overwrote pending register. (Assignments::assign)");
		return;
	}

	a.isPending = true;
	a.call = 0;
	a.priority = priority;

	if (a.isLocal == false)
	{
		a.value = constVal;
		a.isTemp = true;
	} 
	else
	{
		Assignments::Pending p;
		p.r = &a;
		p.right = constVal;
		pending.push_back(p);
	}
}

void Assignments::assign(string constA, string constB, int priority)
{
	Assignments::Pending p;
	p.left = constA;
	p.right = constB;
	pending.push_back(p);
}

void Assignments::flushPending()
{
	bool canAddAssignments = true;
	if (func->reg.anyTempRegister())
		canAddAssignments = false; // there is still something in flight
	if (pending.empty())
		canAddAssignments = false; // no pending registers

	if (!canAddAssignments)
		return; // nothing to do yet

	stringstream leftSide, rightSide;
	int leftCount = 0;
	int rightCount = 0;
	for (int i = pending.size()-1; i >= 0; i--)
	{
		if (pending[i].r != NULL && pending[i].r->isPending == false)
		{
			func->decErrors[pc].append(" -- Attempted to generate an assignment, but got confused about usage of registers. (Assignments::flushPending)");
			pending.clear();
			return;
		}

		if (leftCount > 0)
			leftSide << ", ";
		if (pending[i].r != NULL)
			leftSide << pending[i].r->getValue();
		else
			leftSide << pending[i].left;
		leftCount++;

		if (!pending[i].right.empty()) { //might be needed//&& (srcs > 0 || (srcs == 0 && strcmp(F->vpend->srcs[i], "nil") != 0) || i == size-1)) {
			if (rightCount > 0)
				rightSide << ", ";
			rightSide << pending[i].right;
			rightCount++;
		}
	}

	pending.clear();

	if (leftCount > 0)
		add(leftSide.str() + " = " + rightSide.str());
}

void Assignments::declareLocals()
{
	// left and right of local declaration
	stringstream leftSide, rightSide;
	leftSide << "local ";
	rightSide << " = ";
	 
	// get function paramater count
	int startParams = 0;
	if (pc == 0)
	{
		//startParams = func->f->numparams + (func->f->is_vararg & 2); // normal params + variable params
		if (func->isVarArg || func->isVarArg2)
			startParams = func->numParameters + 1;
		else
			startParams = func->numParameters;

		for (int i=0; i < startParams; i++)
			func->reg[i].setLocal(func->locals[i].name);
	}
	
	map <int,string> names;
	int locals = 0;
	int internalLocals = 0;

	for (int i = startParams; i < func->locals.size(); i++) 
	{
		if (func->locals[i].startPC == pc)
		{ 
			int r = func->freeLocal + locals + internalLocals;
			
			// handle FOR loops
			if (iOp.opCode == OP_FORPREP) 
			{
				func->locals[i].startPC = pc + 1;
				continue;
			}
			// handle TFOR loops
			if (iOp.opCode == OP_JMP) 
			{
				Op n2 = func->opMap[pc+1+iOp.sbx];
				if (n2.opCode == OP_TFORLOOP) 
				{
					func->locals[i].startPC = pc + 1;
					continue;
				}
			}
	     
			if (func->reg[r].isInternal)
			{
				names[r] = func->locals[i].name;
				func->reg[r].isPending = true;
				func->reg[r].isLocal = true;
				func->reg[r].isInternal = false;
				internalLocals++;
				continue;
			}

			if (func->reg[r].isPending || pc == 0) 
			{
				if (locals > 0) 
				{
					leftSide << ", ";
					if (func->reg[r].getValue() != "")
						rightSide << ", ";
				}

				leftSide << func->locals[i].name;
				if (locals == 0 && func->reg[r].getValue() == "")
					rightSide << "nil";
				else
					rightSide << func->reg[r].getValue();
         	} 
			else 
			{
				if (!(locals > 0)) {
					func->decErrors[pc].append(" -- Confused at declaration of local variable. (Assignments::declareLocals)");
					return;
				}
				leftSide << ", " << func->locals[i].name;
			}

			func->reg[r].call = 0;
			func->reg[r].isLocal = true;
			names[r] = func->locals[i].name;
			locals++;
		}// end big if
   }// end for

   if (locals > 0) 
   {
	   string declaration;
	   declaration = leftSide.str() + rightSide.str();
	   
	   
	   if (pc == 0)
		   add(declaration);
	   else
	   {
		   pc--;
		   add(declaration);
		   pc++;
	   }
   }
   
   for (int i = 0; i < locals + internalLocals; i++) 
   {
	  int r = func->freeLocal + i;
	  func->reg[r].setLocal(names[r]);
   }

   func->freeLocal += locals + internalLocals;
}

void Assignments::releaseLocals()
{
   for (int i = func->locals.size()-1; i >=0 ; i--)
   {
	   if (func->locals[i].endPC == pc)
	   {
		   int r;
		   func->freeLocal--;
		   r = func->freeLocal;
		   if (!func->reg[r].isLocal)
		   {
			   func->decErrors[pc].append(" -- Confused about usage of registers for local variables. (Assignments::releaseLocals)");
			   return;
		   }
		   func->reg[r].isLocal = false;
		   func->reg[r].priority = 0;
	   }
   }
}