#include "Function.h"

#include <sstream>
using namespace std;

#if LUA_VERSION_NUM == 501
	#define NUPS(f) (f->nups)
#endif
#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

Proto* toproto(lua_State* L, int i) {
	const Closure* c=(const Closure*)lua_topointer(L,i);
	return c->l.p;
}

int CompareProto(const Proto* fleft, const Proto* fright, string& str) {
	int sizesame, pc, minsizecode;
	int diff = 0;
	stringstream ss;
	if (fleft->numparams != fright->numparams) {
		diff++;
		ss << " different params size;";
	}
	if (NUPS(fleft) != NUPS(fright)) {
		diff++;
		ss << " different upvalues size;";
	}
	if (fleft->is_vararg != fright->is_vararg) {
		diff++;
		ss << " different is_vararg;";
	}
	if (fleft->sizecode != fright->sizecode) {
		diff++;
		ss << " different code size;";
	}
	sizesame = 0;
	minsizecode = MIN(fleft->sizecode, fright->sizecode);
	for (pc = 0; pc < minsizecode; pc++) {
		Instruction ileft = fleft->code[pc];
		Instruction iright = fright->code[pc];
		if (ileft == iright) {
			sizesame++;
		} else {
			OpCode opleft = GET_OPCODE(ileft);
			int aleft = GETARG_A(ileft);
			int bleft = GETARG_B(ileft);
			int cleft = GETARG_C(ileft);
			int bcleft = GETARG_Bx(ileft);
			int sbcleft = GETARG_sBx(ileft);
			OpCode opright = GET_OPCODE(iright);
			int aright = GETARG_A(iright);
			int bright = GETARG_B(iright);
			int cright = GETARG_C(iright);
			int bcright = GETARG_Bx(iright);
			int sbcright = GETARG_sBx(iright);

			if (opleft == opright) {
				if (opleft == OP_EQ && aleft == aright &&
					bleft == cright && cleft == bright) {
					sizesame++;
				}
			} else if ((opleft == OP_LT && opright == OP_LE) ||
				(opleft == OP_LE && opright == OP_LT)) {
				if (aleft == !aright &&	bleft == cright && cleft == bright) {
					sizesame++;
				}
			}
		}
	}
	if (sizesame != fleft->sizecode) {
		diff++;
		ss << " sizecode org: "<< fleft->sizecode
			<< ", decompiled: "<< fright->sizecode
			<< ", same: "<< sizesame
			<< ";";
	}
	str = ss.str();
	return diff;
}

int Function::doCompare(string& result_str) {
	stringstream ss;
	string decompiled = decompile(0);
	if (!isGlobal) {
		stringstream ss;
		string upvals = listUpvalues();
		if (!upvals.empty())
		{
			ss << "local " << upvals << ";\n";
		}
		ss << "DecompiledFunction_" << funcNumber << "=" << decompiled;
		decompiled = ss.str();
	}
	lua_State* newState = lua_open();
	int check_result;
	if (luaL_loadstring(newState, decompiled.c_str()) != 0) {
		check_result = -1;
		ss << "-- function check fail " << funcNumber << " : cannot compile";
	} else {
		Proto* newProto = toproto(newState, -1);;
		if (!isGlobal) {
			newProto = newProto->p[0];
		}
		string compare_result_str;
		check_result = CompareProto(proto, newProto, compare_result_str);
		if (check_result == 0) {
			ss << "-- function check pass " << funcNumber;
		} else {
			ss << "-- function check fail "<< funcNumber << " : " << compare_result_str;
		}
	}

	lua_close(newState);
	result_str = ss.str();
	return check_result;
}

string Function::decompile(int funcIndent)
{
	string compare_result_str;
	if (shadow)
	{
		shadow->doCompare(compare_result_str);
	}

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

	if (shadow)
	{
		addStatement(compare_result_str);
	}

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

string Function::decompileStub(int funcIndent)
{
	indent = funcIndent;

	generateHeader();

	if (this->upvalues.size() > 0) {
		stringstream ss;
		ss << "local _upvalues_ = {" << listUpvalues() << "};";
		addStatement(ss.str());
	}

	if (!isGlobal)
	{
		indent--;
		addPartial("end");
	}

	return decCode;
}
