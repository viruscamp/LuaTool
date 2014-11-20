//==============================================//
//             LuaC v1.2						// 
//					by Co0kieMonster			//
//==============================================//
//	Lua 5.1.4 Compiler							//
//  - modified to handle break [n] and			//
//    continue [n] statements					//
//	- can replace a single function of a binary	//
//	  luac file									//
//==============================================//

#ifndef LuaC_h
#define LuaC_h

struct lua_State;

#include "ErrorMsgs.h"
#include <string>
using namespace std;

class LuaC
{
public:
	LuaC(string inputFileName); // constructor sets the input file

	void compile(string outputFileName, bool strip = false); // write compiled file to
	void compileAndReplace(string destLuacName, // compile the inputFile and inject it as
						   int funcNumber,      // a function into the destination .luac 
						   bool strip = false); // at funcNumber
																		
	ErrorMsgs errors;
	static const string version;

private:
	static int writer(lua_State* L, const void* p, size_t size, void* u);
	void fatal(const char* msg);
	void cannot(const char* what);

private:
	string inputName;
	string outputName;
};

#endif