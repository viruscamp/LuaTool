// Class Function - defines a Lua function and everything needed to decompile it
#ifndef Function_h
#define Function_h

#include "Defs_LuaDec.h"
#include "Op.h"
#include "IfBlocks.h"
#include "LoopBlocks.h"
#include "RegisterFile.h"
#include "DecTable.h"
#include "Assignments.h"
#include "Locals.h"
#include "LogicExpressions.h"

#include "ErrorMsgs.h"

extern "C" {
#include "lauxlib.h"
#include "ldebug.h"
#include "lobject.h" 
}

#include <string>
#include <map>
#include <list>
#include <vector>
#include <memory>
using namespace std;
using namespace std::tr1;

class LuaState
{
public:
	LuaState();
	~LuaState();

	lua_State* getState();
private:
	lua_State* L;
};

class Function
{
// ===================== Constructors =====================
public:
	Function(const char* inputName, bool nosub = false, bool functionCompare = false); // global block constructor - from file
	Function() {}; // default constructor
private:
	Function(shared_ptr<LuaState> l, Proto* f, string number, map<int,string> upvals, bool nosub = false, bool functionCompare = false); // subfunction constructor

// ======================= Methods ========================
public:
	string disassemble();
	
	string decompile(int indent = 0);
	string decompileStub(int indent = 0);
	string getDecompiledCode() const;

	Function* getSubfunction(unsigned i);

	Function* findSubFunction(const string funcnumstr);

	string listUpvalues();
private:
	void buildFromProto(Proto* f); // build function from proto
	map<int, string> getUpValues(Proto* f, int numFunc); // gets upvalues for subfunctions

	void generateHeader(); // generates function header
	void startOrEndBlock(const PcAddr pc); // generate (if or loop) block start or end
	
	void addDisLine(string line); // add line to disCode
	void addStatement(string statement); // add statement to decCode
	void addPartial(string partial, string end = ""); // add partial statement to decCode

	bool assignmentsBetween(PcAddr start, PcAddr end); // are there any assignments here?

	int doCompare(string& result_str);

// ====================== Variables =======================
private:
	shared_ptr<LuaState> l;

	bool isGlobal; // indicates if this is the global function

	string decCode; // decompiled code
	string disCode; // disassembled code

	Locals locals; // maps local variable declarations, names and scopes
	Assignments assignments; // maps all assignments
	LogicExpressions logicExpressions; // maps complex logic expressions
	LoopBlocks loopBlocks; // maps loop blocks
	IfBlocks ifBlocks; // maps if statements
	RegisterFile reg; // keeps the current register state as well as constants
	map<int, string> upvalues; // map of upvalues

	map<PcAddr, string> decErrors; // holds errors encountered while decompiling

	int indent; // line indent
	string endOfLine; // pending end of statement
	int freeLocal; // first register not declared local

	string funcNumber; // function number
	int numParameters; // number of function parameters
	int maxStackSize; // max number of registers used by this function
	int lineDefined; // line at which this function is defined
	int vararg; // variable arguments // TODO: redo vararg handling
	bool isVarArg; // variable arguments 1
	bool isVarArg2; // variable arguments 2

	int codeSize; // total number of opcodes
	map<PcAddr, Op> opMap; // all function ops, mapped by program counter
	map<int, Function> subFunctions; // map of subfunctions

	bool nosub;
	bool functionCompare;
	Proto* proto;

// ======================= Friends ========================
public: 
	friend class LoopBlocks;
	friend class IfBlocks;
	friend class Assignments;
	friend class Locals;
	friend class LogicExpressions;

	ErrorMsgs errors; // non-decompile related error messages
};

#endif