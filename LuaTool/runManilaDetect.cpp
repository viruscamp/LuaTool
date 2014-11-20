#include "LuaTool.h"
#include "ManilaFile.h"

#include <iostream>
using namespace std;

void LuaTool::printManilaDetectUsage(string message)
{
	cout << endl;
	if (message != "")
		cout <<"*"<< message <<"*"<< endl << endl;

	cout << "ManilaDetect " << "\n"
		"  Usage: LuaTool /detect <inputfile>\n";
}

void LuaTool::runManilaDetect()
{
	// check cmd line
	if (args.size() != 3) {
		printManilaDetectUsage("Invalid command line paramaters");
		return;
	}
	// safe to create input string
	string inputName = args[args.size()-1];

	ManilaFile::MType mt;

	mt = ManilaFile::detectManilaType(inputName);

	string str;
	if (mt == ManilaFile::MODE9_BIN)
		str = "Mode9";
	else if (mt == ManilaFile::MODE9_XML)
		str = "Mode9_XML";
	else if (mt == ManilaFile::LUAC)
		str = "Luac";
	else if (mt == ManilaFile::LUA)
		str = "Lua (decompiled)";
	else if (mt == ManilaFile::QTC)
		str = "QTC";
	else if (mt == ManilaFile::LOC_XML)
		str = "XML";
	else if (mt == ManilaFile::INVALID)
		str = "Unknown";

	cout << str << " file detected.";
}