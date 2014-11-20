#include "Op.h"

#include <sstream>
using namespace std;

Op::Op(Instruction ins)
{
	opCode = GET_OPCODE(ins);
	a = GETARG_A(ins);
	b = GETARG_B(ins);
	c = GETARG_C(ins);
	bx = GETARG_Bx(ins);
	sbx = GETARG_sBx(ins);
}

bool Op::isTest() 
{
	if (opCode == OP_EQ || opCode == OP_LT || opCode == OP_LE || opCode == OP_TEST || opCode == OP_TESTSET)
		return true;
	else
		return false;
}

string Op::getOpString()
{
	char *operators[22] =
	{ " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ",
			"+", "-", "*", "/", "%", "^", "-", "not ", "#", ".."};

	if (opCode == OP_EQ && a == 0)
		return " == ";
	else if (opCode == OP_EQ && a == 1)
		return " ~= ";

	if (opCode == OP_LT && a == 0)
		return " < ";
	else if (opCode == OP_LT && a == 1)
		return " > ";

	if (opCode == OP_LE && a == 0)
		return " <= ";
	else if (opCode == OP_LE && a == 1)
		return " >= ";

	if ((opCode == OP_TEST || opCode == OP_TESTSET) && c == 0)
		return "";
	else if ((opCode == OP_TEST || opCode == OP_TESTSET) && c == 1)
		return "not ";

	if (opCode <= 22)
		return operators[opCode];

	return "-- ERROR: invalid opcode -- Op::getOpString() --";
}

int Op::getOpPriority()
{
	int priorities[22] = 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 3, 3, 3, 1, 2, 2, 2, 5 };

	if (opCode <= 22)
		return priorities[opCode];
	else
		return 0;
}