#include "Function.h"

#include <sstream>
using namespace std;

string Function::decompile(int funcIndent)
{
	indent = funcIndent;

	// the decompile is done in multiple passes
	// this order of the passes is strict
	locals.mapFunction(this); // map local declarations
	assignments.mapFunction(this); // map assignments
	logicExpressions.mapFunction(this); // map logic expressions
	loopBlocks.map(this); // map all loops
	ifBlocks.map(this); // map if statements

	// print function header
	generateHeader();

	// locals declared at pc == 0 can interfere with some block starts
	// this is a workaround
	if (assignments[0] != "")
	{
		addStatement(assignments[0]);
		assignments[0].clear();
	}

	// now, take all the info of the previous pass
	// and put it together in this final pass
	for(PcAddr pc = 0; pc < codeSize; pc++)
	{
		startOrEndBlock(pc);

		if (assignments[pc] != "")
			addStatement(assignments[pc]);

		if (logicExpressions[pc] != "")
			addStatement(logicExpressions[pc]);

		if (decErrors[pc] != "")
			addStatement(decErrors[pc]);
	}

	if (!isGlobal)
	{
		indent--;
		addPartial("end");
	}

	return decCode;
}

void Function::generateHeader()
{
	if (isGlobal)
		return; // global function doesn't have a header

	stringstream ss;
	ss << "function(";
	if (numParameters > 0)
	{
		for (int i = 0; i < numParameters - 1; i++)
			ss << locals[i].name << ", ";
		ss << locals[numParameters - 1].name;

		if (isVarArg2)
			ss << " , ...";
	} 
	else 
	{
      if (isVarArg2)
		  ss << "...";
	}
	ss << ")";

	addStatement(ss.str());
	indent++;
}

void Function::startOrEndBlock(const PcAddr pc)
{
	// end if - there can be more then one end at a given pc
	for (int i=0; i < ifBlocks.getNumEnds(pc); i++)
	{
		indent--;
		addStatement("end");
	}

	// write break
	addStatement(loopBlocks.getBreak(pc));

	// block end - there can be more then one end at a given pc
	string loopEnd = loopBlocks.getEnd(pc);
	while (loopEnd != "")
	{
		indent--;
		if (loopEnd == "until ")
			addPartial(loopEnd);
		else
			addStatement(loopEnd);
	
		loopEnd = loopBlocks.getEnd(pc);
	}

	// write else
	if (ifBlocks.isElse(pc))
	{
		//elsePending = pc;
		indent--;
		addStatement("else");
		indent++;
	}

	// write else if
	if (ifBlocks.isElseIf(pc))
	{
		indent--;
		addPartial("else");
	}

	// loop block starts
	if (loopBlocks.getStart(pc) == "while " || loopBlocks.getStart(pc) == "for ")
	{
		addPartial(loopBlocks.getStart(pc), " do");
		indent++;
	}
	else if (loopBlocks.getStart(pc) != "")
	{
		addStatement(loopBlocks.getStart(pc));
		indent++;
	}

	// start if
 	if (ifBlocks.isStart(pc))
	{
		addStatement("if " + ifBlocks.getIfStatement(pc) + " then");
		indent++;
	}
}