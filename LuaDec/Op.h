// Class Op - single op info - opcode and fields

#ifndef Op_h
#define Op_h

extern "C" {
#include "lopcodes.h"
}

#include <string>
using namespace std;

class Op
{
public:
	Op() {};
	Op(Instruction ins);

	bool isTest(); // is this test op?
	string getOpString(); // e.g. OP_ADD returns "+"
	int getOpPriority(); // gets this ops priority

	OpCode opCode;

	// op fields
	int a, b, c;
	int bx;
	int sbx;
};

#endif