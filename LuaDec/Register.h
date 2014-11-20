// Class Register - single register info

#ifndef Register_h
#define Register_h

#include "DecTable.h"

#include <string>
using namespace std;

class Register
{
public:
	Register() 
		: value("nil"), priority(0), call(0), isConstant(false), isTemp(false),
		isLocal(false), isPending(false), isInternal(false), isTable(false) {};
	
	string getValue(); // get this register's value
	void setLocal(string localVal); // set this register to represent a local

	int priority; // if the register holds an operation, this is the priority of that operation
	int call;

	bool isTemp; // register holds a temporary value
	bool isConstant; // represents a constant value
	bool isLocal; // represents a local variable
	bool isInternal; // is an internal variable of a for loop
	bool isPending; // has been writen to, but not read from yet

	bool isTable;
	DecTable table;

	string value;
};

#endif