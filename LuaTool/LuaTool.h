//==============================================//
//              LuaTool v1.2					// 
//					by Co0kieMonster			//
//==============================================//

#ifndef LuaTool_h
#define LuaTool_h

#include "TextFile.h"

#include <iostream>
#include <string>
#include <map>
using namespace std;

class LuaTool
{
public:
	LuaTool(map<int,string> args);

	void runLuaDec();
	void runMultiDec();
	void runLuaC();
	void runLuaCompare();
	void runManilaDetect();

	void printLuaToolUsage(string message = "");
	void printLuaDecUsage(string message = "");
	void printMultiDecUsage(string message = "");
	void printLuaCUsage(string message = "");
	void printLuaCompareUsage(string message = "");
	void printManilaDetectUsage(string message = "");

	static const string version;
private:
	void singleLuaDec(string inputName, string outputName, bool isDisassemble, int functionNum);

	int m9extract(string inputFileName, string outputDir);

	string removeExt(string str); // returns file name without file extension
	string removePath(string str); // returns file name without path (but with extension)
	
	inline void progress(unsigned i, unsigned total)
	{	
		double percent;
		percent = (100.0*i) / total;
		cout << "\r" << percent << "%";
	}

private:
	map<int, string> args; // command line arguments

	class Config
	{
	public:
		Config(string inputFileName);

		string luaWorkspace;
		string binDecompiled;
		string m9Workspace;
		string repository;
	private:
		void trim(string& str); // trim space from string
	};

};

#endif