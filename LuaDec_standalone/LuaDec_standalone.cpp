#include "LuaDec.h"

#include <iostream>
#include <string>
#include <map>
using namespace std;

void printLuaDecUsage(string message)
{
	cout << endl;
	if (message != "")
		cout <<"*"<< message <<"*"<< endl;

	cout << "LuaDec " << LuaDec::version << "\n"
		"  Usage: LuaTool /decompile [options] <inputfile>\n"
 		"  Available Options:\n"
 		"  -o <filename>   specify output file name\n"
 		"  -dis            don't decompile, just disassemble\n"
 		"  -f <number>     decompile/disassemble only function number (0 = global block)\n";
}

int main(int argn, char* argv[])
{
	map<int,string> args;
	for(int i=0; i < argn; i++)
		args[i].assign(argv[i]);

	bool isDisassemble = false;
	int functionNum = 0;
	string inputName = "";
	string outputName = "";
	
	int numOptions = 0;
	// go through options
	for (unsigned i = 1; i < args.size(); i++)
	{
		if (args[i] == "-dis")
		{	// disassemble
			isDisassemble = true;
			numOptions++;
		}
		else if (args[i] == "-f")
		{	// only decompile this function
			functionNum = atoi(args[i+1].c_str());
			numOptions +=2;
			i++;
		}
		else if (args[i] == "-o")
		{	// output name
			outputName = args[i+1];
			numOptions +=2;
			i++;
		}
	}
	// check if options were valid
	if (numOptions != args.size() - 2) {
		printLuaDecUsage("Invalid command line paramaters");
		return 1;
	}
	// safe to create input string
	inputName = args[args.size()-1];

	if (outputName == "")
	{
		/*
		// default output name
		if (isDisassemble)
			outputName = inputName + ".dis.lua";
		else
			outputName = inputName + ".lua";
		*/
	}

	// disassemble/decompile task
	LuaDec ld(inputName);
	if (isDisassemble)
	{
		ld.disassemble(outputName, functionNum);
		if (ld.errors.getLast() != "")
		{
			cout << ld.errors.getLast() << endl;
			ld.errors.clearAll();
			return 1;
		}
	}
	else
	{
		ld.decompile(outputName, functionNum);
		if (ld.errors.getLast() != "")
		{
			cout << ld.errors.getLast() << endl;
			ld.errors.clearAll();
			return 1;
		}
	}

#ifdef _DEBUG
	system("PAUSE");
#endif

	return 0;
}