// Class RegisterFile - tracks current register states
// and keeps constant information

#ifndef RegisterFile_h
#define RegisterFile_h

#include "Register.h"

extern "C" {
#include "lauxlib.h"
#include "ldebug.h"
#include "lobject.h"
}

#include <string>
#include <vector>
#include <map>
using namespace std;

class RegisterFile
{
public:
	RegisterFile() {};

	void build(Proto* f); // build reg file from proto

	Register& operator[] (int index);
	string getBxConstant(int bx);
	string getGlobal(int bx);

	bool anyTempRegister(); // are any registers taged as temp

private:
	map<int, Register> registers; // register file
	map<int, Register> constants; // map of constants

	// helper functions
	string DecriptConstant(Proto* f, int n);
	string DecriptString(Proto* f, int n);
};

#endif