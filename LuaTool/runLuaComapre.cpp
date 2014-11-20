#include "LuaTool.h"
#include "LuaCompare.h"
#include "LuaC.h"
#include "LuaDec.h"
#include "ManilaFile.h"

#include <iostream>
#include <iomanip>
#include <map>
#include <string>
using namespace std;

void LuaTool::printLuaCompareUsage(string message)
{
	cout << endl;
	if (message != "")
		cout <<"*"<< message <<"*"<< endl << endl;

	cout << "LuaCompare " << LuaCompare::version << "\n"
		"  Usage: LuaTool /compare [options] <original.luac> <newfile.lua(c)>\n"
		"  Available Options:\n"
		"  -o <filename>   specify output file name\n"
		"  -s              side by side file comparison\n"
		"  -du             disable underline\n";
}

void LuaTool::runLuaCompare()
{
	bool underline = true;
	bool sideBySide = false;
	string outputName = "";

	int numOptions = 0;
	// go through options
	for (unsigned i = 2; i < args.size(); i++)
	{
		if (args[i] == "-s")
		{	// side by side
			sideBySide = true;
			numOptions++;
		}
		else if (args[i] == "-du")
		{	// disable underline
			underline = false;
			numOptions++;
		}
		else if (args[i] == "-o")
		{	// output name
			outputName = args[i+1];
			numOptions +=2;
			i++;
		}
	}
	// check if options were valid
	if (numOptions != args.size() - 4) {
		printLuaCompareUsage("Invalid command line paramaters.");
		return;
	}
	// safe to create string
	string originalFile = args[args.size()-2];
	string newFile = args[args.size()-1];

	if (outputName == "") // default output
		outputName = removeExt(newFile) + ".cmp.lua";

	// compare task
	LuaCompare lc(originalFile, newFile);
	lc.compare(0, underline, sideBySide);
	lc.writeTo(outputName);

	// errors?
	if (lc.errors.getLast() != "")
	{
		cout << lc.errors.getLast() << endl;
		lc.errors.clearAll();
	}

	// output success rate
	cout << fixed << setprecision(0);
	cout << "Files match: " << lc.percent << "%" << endl;
}