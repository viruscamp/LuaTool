#include "LuaTool.h"
#include "LuaC.h"

#include "TextFile.h"

#include <iostream>
#include <map>
#include <string>
using namespace std;

void LuaTool::printLuaCUsage(string message)
{
	cout << endl;
	if (message != "")
		cout <<"*"<< message <<"*"<< endl << endl;

	cout << "LuaC " << LuaC::version << "\n"
		"  Usage: LuaTool /compile [options] <inputfile>\n"
 		"  Available Options:\n"
 		"  -o <filename>        specify output file name\n"
 		"  -s                   strip debug information\n"
		"  -r <n> <luac_file>   replace function <n> in <luac_file> with <inputfile>\n";
}

void LuaTool::runLuaC()
{
	bool isStripping = false;
	string inputName = "";
	string outputName = "";
	int replaceNumber = 0;
	bool isReplace = false;

	int numOptions = 0;
	// go through options
	for (unsigned i = 2; i < args.size(); i++)
	{
		if (args[i] == "-s")
		{	// disassemble
			isStripping = true;
			numOptions++;
		}
		else if (args[i] == "-o")
		{	// output name
			outputName = args[i+1];
			numOptions +=2;
			i++;
		}
		else if (args[i] == "-r")
		{	// output name
			isReplace = true;
			replaceNumber = atoi(args[i+1].c_str());
			outputName = args[i+2];
			numOptions +=3;
			i+=2;
		}
	}
	// check if options were valid
	if (numOptions != args.size() - 3) {
		printLuaCUsage("Invalid command line paramaters");
		return;
	}
	if (isReplace && replaceNumber <= 0){
		printLuaCUsage("Function number must be greater than zero");
		return;
	}
	// safe to create input string
	inputName = args[args.size()-1];

	if (outputName == "") // default output name
		outputName = removeExt(inputName) + ".luac";

	// compile task
	LuaC lc(inputName);
	if (!isReplace)
		lc.compile(outputName, isStripping);
	else
		lc.compileAndReplace(outputName, replaceNumber, isStripping);

	// errors?
	if (lc.errors.getLast() != "")
	{
		cout << lc.errors.getLast();
		lc.errors.clearAll();
	}
}