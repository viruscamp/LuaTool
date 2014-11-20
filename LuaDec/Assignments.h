// Class Assignments - keeps track of all assignments including local decplarations
// also prints the basic form of conditionals (LogicExpressions class later 
// makes complex conditional expressions from the basic ones found here)
#ifndef Assignments_h
#define Assignments_h

#include "Defs_LuaDec.h"
#include "Op.h"

#include <string>
#include <map>
#include <vector>
using namespace std;

class Function;
class Register;

class Assignments
{
public:
	Assignments() {};

	void mapFunction(Function* functionToMap);
	string& operator[] (PcAddr pc);

private:
	void assign(Register& a, Register& b, int priority = 0); // R(a) = R(b)
	void assign(Register& a, string c, int priority = 0); // R(a) = K(c)
	void assign(string constA, string constB, int priority = 0); // K(b) = K(c)
	
	void declareLocals(); // declare local variables
	void releaseLocals(); // release local variables
	void flushPending(); // add all pending assignments

	void add(string assignment); // add assignment to map
	string makeIndex(string constant, bool self = false); // index as "[]", "." or ":"

	void opSwitch(OpCode opCode); // calls the right method for the passed opcode
	void opMove(); // copy from one register to another
	void opLoadK(); // load constant into a register
	void opLoadBool(); // load bool into a register
	void opLoadNil(); // load nil into register
	void opVarArg(); // load "..." into register
	void opGetUpval(); // load upvalue into register
	void opSetUpval(); // set upvalue from register
	void opGetGlobal(); // load global into register
	void opSetGlobal(); // set global from register
	void opNewTable(); // load new table into register
	void opGetTable(); // load table entry into register
	void opSetTable(); // set table entry from register
	void opSetList(); // set numeric table entries
	void opSelf(); // self call
	void opBinary(); // binary operation (+, -, *, /, %, ^)
	void opUnary(); // unary operation (-, not, length of)
	void opConcat(); // combine data in registers
	void opCall(); // function call
	void opReturn(); // function return
	void opClosure(); // function
	void opCompare(); // compare operation (<, <=, ==)
	void opTest(); // test a bool value
	void opNumericFor(); // setup numeric for loop header
	void opGeneralFor(); // setup general for loop header

private:
	Function* func; // owner function
	PcAddr pc; // current pc
	Op iOp; // current op

	map<PcAddr, string> assignments; // assignments maped by pc

	class Pending
	{
	public:
		Pending() 
			: r(NULL), left(""), right("") {};

		Register* r;
		string left;
		string right;
	};
	vector<Pending> pending; // pending assignments
};

#endif