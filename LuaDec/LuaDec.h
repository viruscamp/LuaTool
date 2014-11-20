//==============================================//
//           LuaDec v3.2						// 
//					by Co0kieMonster			//
//==============================================//

#ifndef LuaDec_h
#define LuaDec_h

#include "TextFile.h"
#include "ManilaNames.h"
#include "ErrorMsgs.h"

struct Proto;

#include <string>
using namespace std;

class LuaDec
{
public:
	LuaDec(string inputFileName); // decompile from file

	void decompile(string outputFileName, int functionNum = 0);
	void disassemble(string outputFileName, int functionNum = 0);
	TextFile disassembleForCompare(); // used to directly pass the dissasembly to LuaCompare instead of writing to disk 

	string manilaName; // full manila name of this file

	ErrorMsgs errors;
	static const string version;

private:
	string makeHeader(bool isDisassemble = false);

private:
	string inputName; // input file name
	TextFile output; // output file

	static ManilaNames manilaNames;
};

#endif