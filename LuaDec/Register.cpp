#include "Register.h"

using namespace std;

string Register::getValue()
{
	isPending = false;
	isTemp = false;

	return value;
}

void Register::setLocal(string val)
{
	isLocal = true;
	value = val;
	priority = 0;
	isPending = false;
}