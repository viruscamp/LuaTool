#include "LuaTool.h"
const string LuaTool::version = "1.2";

#include <iostream>
using namespace std;

LuaTool::LuaTool(map<int,string> a)
: args(a)
{}

string LuaTool::removeExt(string str)
{
	size_t pos;
	pos = str.rfind('.');
	if (pos != string::npos && str[pos+1] != '\\')
		str = str.substr(0, pos);
	
	return str;
}

string LuaTool::removePath(string str)
{
	size_t pos;
	pos = str.rfind('\\');
	if (pos != string::npos)
		str = str.substr(pos+1);
	
	return str;
}

void LuaTool::printLuaToolUsage(string message)
{
	cout << endl;
	if (message != "")
		cout <<"*"<< message <<"*"<< endl << endl;

	cout <<"LuaTool " << LuaTool::version << " by Co0kieMonster\n"
		"  Usage: LuaTool <task_select> [task_options] <task_input>\n"
		"  Tasks:\n"
		"  /decompile (or /d)  -- Lua Decompiler\n"
		"  /compile   (or /c)  -- Lua Compiler\n"
		"  /compare   (or /cr) -- Lua Compare utility\n"
		"  /detect    (or /dt) -- Manila file type detect utility\n";
	
	printLuaDecUsage();
	printLuaCUsage();
	printLuaCompareUsage();
	printManilaDetectUsage();
}