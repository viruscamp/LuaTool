#include "LuaTool.h"

#include <iostream>
#include <string>
#include <map>
#include <ctime>
using namespace std;

int main(int argn, char* argv[])
{
	map<int,string> args;
	for(int i=0; i < argn; i++)
		args[i].assign(argv[i]);

	LuaTool lt(args);

	string taskSelect = args[1];
	if(taskSelect == "/decompile" || taskSelect == "/d")
		lt.runLuaDec();
	else if (taskSelect == "/multidec" || taskSelect == "/md")
		lt.runMultiDec();
	else if (taskSelect == "/compile" || taskSelect == "/c")
		lt.runLuaC();
	else if (taskSelect == "/compare" || taskSelect == "/cr")
		lt.runLuaCompare();
	else if (taskSelect == "/detect" || taskSelect == "/dt")
		lt.runManilaDetect();
	else if (taskSelect == "/help" || taskSelect == "/h" || taskSelect == "")
		lt.printLuaToolUsage();
	else
		lt.printLuaToolUsage("Invalid task selected.");

	//cout.precision(2);
	//cout << endl << (float)clock() / CLOCKS_PER_SEC << "s" << endl;

#ifdef _DEBUG
	system("PAUSE");
#endif

	return 0;
}