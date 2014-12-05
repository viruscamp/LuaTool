#include "LuaTool.h"
#include "LuaDec.h"
#include "LuaCompare.h"
#include "Directory.h"
#include "ManilaFile.h"

#include <iostream>
#include <iomanip>
#include <map>
#include <string>
#include <sstream>
using namespace std;

void LuaTool::printLuaDecUsage(string message)
{
	cout << endl;
	if (message != "")
		cout <<"*"<< message <<"*"<< endl << endl;

	cout << "LuaDec " << LuaDec::version << "\n"
		"  Usage: LuaTool /decompile [options] <inputfile>\n"
 		"  Available Options:\n"
 		"  -o <filename>   specify output file name\n"
 		"  -dis            don't decompile, just disassemble\n"
 		"  -f <number>     decompile/disassemble only function number (0 = global block)\n";
}

void LuaTool::runLuaDec()
{
	bool isDisassemble = false;
	bool isMulti = false;
	string functionNum = "";
	string inputName = "";
	string outputName = "";
	
	int numOptions = 0;
	// go through options
	for (unsigned i = 2; i < args.size(); i++)
	{
		if (args[i] == "-dis")
		{	// disassemble
			isDisassemble = true;
			numOptions++;
		}
		else if (args[i] == "-f")
		{	// only decompile this function
			functionNum = args[i+1];
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
	if (numOptions != args.size() - 3) {
		printLuaDecUsage("Invalid command line paramaters");
		return;
	}
	// safe to create input string
	inputName = args[args.size()-1];

	singleLuaDec(inputName, outputName, isDisassemble, functionNum);
}

void LuaTool::singleLuaDec(string inputName, string outputName, bool isDisassemble, const string functionNum)
{
	if (outputName == "")
	{ // default output name
		if (isDisassemble)
			outputName = removeExt(inputName) + ".dis.lua";
		else
			outputName = removeExt(inputName) + ".lua";
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
			return;
		}
	}
	else
	{
		ld.decompile(outputName, functionNum);
		if (ld.errors.getLast() != "")
		{
			cout << ld.errors.getLast() << endl;
			ld.errors.clearAll();
			return;
		}

		// compare output with original
		string cmpOutput;
		cmpOutput = removeExt(outputName) + ".cmp.lua";
		LuaCompare lc(inputName, outputName);

		// in case of bad decompile produce compare file and disassembly
		if (lc.compare(atoi(functionNum.c_str())) == false)
		{
			lc.writeTo(cmpOutput);

			if (lc.errors.getLast() != "")
			{
				string errFileName = removeExt(outputName) + ".error.txt";
				TextFile errFile;
				errFile.addText(lc.errors.getLast());
				errFile.writeTo(errFileName);
				lc.errors.clearAll();
			}
			
			outputName = removeExt(outputName) + ".dis.lua";
			ld.disassemble(outputName, functionNum);
		}

		// output success rate
		cout << fixed << setprecision(0);
		cout << "Decompile success: " << lc.percent << "%" << endl;
	}
}