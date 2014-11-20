// ManilaFile.h
// Class ManilaFile - read and hold a manila file

#ifndef ManilaFile_h
#define ManilaFile_h

#include "TextFile.h"

#include <list>
#include <string>
using namespace std;

class ManilaFile : public TextFile
{
public:
	enum MType {MODE9_BIN = 1, MODE9_XML, LUA, LUAC, LOC_XML, QTC, INVALID};

	ManilaFile(string inputFileName = "");
	void read(string inputFileName);

	static MType detectManilaType(string inputFileName);

	list<string> manilaNames; // names found in this file
	MType manilaType;

private:
	static string getFirstN(unsigned n, string inputFileName); // read first n characters from file

	static string sigMode9bin;
	static const string sigMode9xml;
	static const string sigLua1;
	static const string sigLua2;
	static const string sigLua3;
	static string sigLuac;
	static const string sigLocXml1;
	static const string sigLocXml2;
	static const string sigQtc;
};

#endif