//==============================================//
//           LuaCompare v1.2.1					// 
//					by Co0kieMonster			//
//==============================================//

#ifndef LuaCompare_h
#define LuaCompare_h

#include "TextFile.h"
#include "ErrorMsgs.h"

#include <map>
#include <string>
using namespace std;

class LuaCompare
{
public:
	LuaCompare(string originalFileName, string newFileName);

	void loadFiles(string originalFileName, string newFileName); // read file and compile/disassemble as needed
	bool compare(int functionNumber = -1, bool underline = true, bool sideBySide = false); // return true if files are identical
	void writeTo(string outFileName) const; // write output

	// stats
	map <int, bool> perFuncSame; // is functions num # the same?
	bool overallSame; // are the files completely identical?
	double percent; // success percentage

	ErrorMsgs errors;
	static const string version;

private:
	TextFile compareOut;

	map<int, list<string>> funcOrig;
	map<int, list<string>> funcNew;
	unsigned originalLineCount;
	unsigned maxLen;

	bool sideBySide;
	bool underline;
};

#endif