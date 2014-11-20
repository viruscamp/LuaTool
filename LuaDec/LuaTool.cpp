//////////////////////////////////////////////////
//           LuaTool v1.0						// 
//					by Co0kieMonster			//
//////////////////////////////////////////////////

#include "LuaDec.h"
#include "LuaC.h"

#include <iostream>
#include <string>
#include <map>
using namespace std;

static const string progVersion = "1.0";

void printUsage(string message = "");

int main(int argn, char* argv[])
{
	map<int, string> args;

	for(int i=0; i < argn; i++)
		args[i].assign(argv[i]);

	if(args[1] == "/decompile" || args[1] == "/d")
		LuaDec ld(argn, argv);
	else if (args[1] == "/compile" || args[1] == "/c")
		LuaC lc(args);
	/*else if (args[1] == "/compare" || args[1] == "/cr")
		luaCompare();
	else if (args[1] == "/detect" || args[1] == "/dt")
		manilaDetect();*/
	else if (args[1] == "/help" || args[1] == "/h" || args[1] == "")
		printUsage();
	else
		printUsage("Invalid task selected.");


#ifdef _DEBUG
	system("PAUSE");
#endif

	return 0;
}

void printUsage(string message)
{
	if (message != "")
		cout <<"*"<< message <<"*"<< endl;

	cout << "LuaTool " << progVersion << " by Co0kieMonster\n"
		"  Usage: LuaTool <task_select> [options] <input_file>\n"
		" Tasks:"
		"  /decompile (or /d) \n"
		"  /compile   (or /c) \n"
		"  /compare   (or /cr)\n"
		"  /detect    (or /dt)\n";
	exit(0);
}