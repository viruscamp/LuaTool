#include "LuaDec.h"
const string LuaDec::version = "3.2";
ManilaNames LuaDec::manilaNames("m9editor.names.txt");

#include "Function.h"
#include "ManilaFile.h"

#include <iostream>
#include <sstream>
#include <ctime>
using namespace std;

LuaDec::LuaDec(string inputFileName)
:inputName(inputFileName)
{}

void LuaDec::decompile(string outputFileName, int functionNum)
{
	output.create(outputFileName);
	output.addText(makeHeader());

	/*
	if (ManilaFile::detectManilaType(inputName) != ManilaFile::LUAC)
	{
		errors.set("Input is not a valid LUAC file.");
		return;
	}
	*/

	Function luaGlobal(inputName.c_str());
	if (luaGlobal.errors.getLast() != "")
	{
		errors.set(luaGlobal.errors.getLast());
		return;
	}

	string outCode;
	outCode = luaGlobal.decompile();
	if (functionNum > 0)
		outCode = luaGlobal.getSubfunction(functionNum - 1)->getDecompiledCode();

	output.addText(outCode);
	output.write();
}

void LuaDec::disassemble(string outputFileName, int functionNum)
{
	output.create(outputFileName);
	output.addText(makeHeader(true));

	/*
	if (ManilaFile::detectManilaType(inputName) != ManilaFile::LUAC
		&& ManilaFile::detectManilaType(inputName) != ManilaFile::LUA)
	{
		errors.set("Input is not a valid LUAC file.");
		return;
	}
	*/

	Function luaGlobal(inputName.c_str());
	if (luaGlobal.errors.getLast() != "")
	{
		errors.set(luaGlobal.errors.getLast());
		return;
	}

	string outCode;
	if (functionNum > 0)
		outCode = luaGlobal.getSubfunction(functionNum - 1)->disassemble();
	else
		outCode = luaGlobal.disassemble();

	output.addText(outCode);
	output.write();
}

TextFile LuaDec::disassembleForCompare()
{
	output.create("dummy"); // no file s actually created
	output.addText(makeHeader(true));

	/*
	if (ManilaFile::detectManilaType(inputName) != ManilaFile::LUAC
		&& ManilaFile::detectManilaType(inputName) != ManilaFile::LUA)
	{
		errors.set("Input is not a valid LUAC file.");
		return output;
	}
	*/

	Function luaGlobal(inputName.c_str());
	if (luaGlobal.errors.getLast() != "")
	{
		errors.set(luaGlobal.errors.getLast());
		return output;
	}

	output.addText(luaGlobal.disassemble());
	// no write to disk
	return output; // directly pass TextFile
}

string LuaDec::makeHeader(bool isDisassemble)
{
	stringstream ssHeader;

	if (isDisassemble)
		ssHeader << "; Disassembled using luadec "<< LuaDec::version <<" -- ";
	else
		ssHeader << "-- Decompiled using luadec "<< LuaDec::version <<" -- ";
	
	// time
	time_t currentTime = time(0);
	char sTime[50];
	ctime_s(sTime, 50, &currentTime);
	ssHeader << sTime;

	// file name
	if (isDisassemble)
		ssHeader << "; File name: ";
	else
		ssHeader << "-- File name: "; 

	string fName;
	int pos = inputName.rfind("\\");
	if (pos != string::npos)
		fName = inputName.substr(pos+1);
	else
		fName = inputName;
	ssHeader << fName;

	// manila name
	manilaName = manilaNames.getManilaName(fName);
	if (manilaName != "")
		ssHeader << " -- " << manilaName;
	ssHeader << endl << endl;

	return ssHeader.str();
}