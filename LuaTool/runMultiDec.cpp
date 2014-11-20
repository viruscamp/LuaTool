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
#include <fstream>
using namespace std;

void LuaTool::printMultiDecUsage(string message)
{
	//cout << endl;
	//if (message != "")
	//	cout <<"*"<< message <<"*"<< endl;

	//cout << "LuaDec " << LuaDec::version << "\n"
	//	"  Usage: LuaTool /decompile [options] <inputfile>\n"
 //		"  Available Options:\n"
 //		"  -o <filename>   specify output file name\n"
 //		"  -dis            don't decompile, just disassemble\n"
 //		"  -f <number>     decompile/disassemble only function number (0 = global block)\n";
	//	//"  -m              decompile multiple files - input and output should be folders\n";
}

void LuaTool::runMultiDec()
{
	string inputDir = "";
	string outputName = "";
	
	// check if cmd line is valid
	if (0 != args.size() - 3) {
		printLuaDecUsage("Invalid command line paramaters");
		return;
	}
	// safe to create input string
	inputDir = args[args.size()-1];
	if (inputDir[inputDir.length()-1] != '\\')
		inputDir.append("\\");

	Config config("LuaTool.cfg");
	string clear = "";
	stringstream ss;
	ss << "MKDIR " << config.binDecompiled << "Completed";
	system(ss.str().c_str());
	ss.str(clear);
	ss << "MKDIR " << config.binDecompiled << "Failed";
	system(ss.str().c_str());
	ss.str(clear);
	ss << "MKDIR " << config.m9Workspace << "Mode9";
	system(ss.str().c_str());
	ss.str(clear);
	ss << "MKDIR " << config.m9Workspace << "QTC";
	system(ss.str().c_str());
	ss.str(clear);
	ss << "MKDIR " << config.m9Workspace << "XML";
	system(ss.str().c_str());
	ss.str(clear);
	ss << "MKDIR " << config.m9Workspace << "Other";
	system(ss.str().c_str());
	ss.str(clear);
	ss << "MKDIR " << inputDir << "Embedded_Scripts";
	system(ss.str().c_str());
	ss.str(clear);

	// preparation
	cout.precision(0);
	cout << setw(6) << fixed;

	bool didM9extract = false;
	Directory dir(inputDir.c_str());
	unsigned total = dir.files.size() - 1;
	for(unsigned i = 0; i < dir.files.size(); i++)
	{
		string iFileName = dir.files[i];
		string inputName = inputDir + iFileName;
		ManilaFile::MType manilaType;

		stringstream ssAttrib;
		ssAttrib << "ATTRIB -s -r " << inputName;
		system(ssAttrib.str().c_str());

		manilaType = ManilaFile::detectManilaType(inputName);

		if (manilaType == ManilaFile::LUAC)
		{
			string outDir, outName;
			stringstream ss;
			outDir = config.luaWorkspace + removeExt(iFileName);
			
			ss << "MKDIR " << outDir;
			system(ss.str().c_str());

			outName = outDir + '\\' + removeExt(iFileName) + ".lua";

			// decompile task
			LuaDec ld(inputName);
			ld.decompile(outName);
			if (ld.errors.getLast() != "")
			{
				cout << ld.errors.getLast() << endl;
				ld.errors.clearAll();
				continue;
			}

			// compare output with original
			string cmpOutput;
			cmpOutput = removeExt(outName) + ".cmp.lua";
			LuaCompare lc(inputName, outName);

			// in case of bad decompile produce compare file and disassembly
			if (lc.compare() == false)
			{
				lc.writeTo(cmpOutput);

				if (lc.errors.getLast() != "")
				{
					string errFileName = removeExt(outName) + ".error.txt";
					TextFile errFile;
					errFile.addText(lc.errors.getLast());
					errFile.writeTo(errFileName);
					lc.errors.clearAll();
				}
				
				string disName = removeExt(outName) + ".dis.lua";
				ld.disassemble(disName);
			}

			// output success rate
			stringstream ss2;
			ss2 << "RENAME " << outDir << " ";
			ss2 << fixed << setprecision(0) << setw(3) << setfill('0');
			ss2 << lc.percent << "%_" << iFileName;
			system(ss2.str().c_str());

			// sort files
			stringstream ss3;
			ss3 << "COPY " << inputName << " " << config.binDecompiled;
			if (lc.percent == 100)
				ss3 << "Completed";
			else
				ss3 << "Failed";
			system(ss3.str().c_str());

			//progress(i, total);
		}
		else if (manilaType == ManilaFile::MODE9_BIN)
		{
			if ( m9extract(inputName, inputDir + "Embedded_Scripts") > 0)
				didM9extract = true;

			stringstream ssCopy;
			ssCopy << "COPY " << inputName << " ";
			ssCopy << config.m9Workspace << "Mode9";
			system(ssCopy.str().c_str());
		}
		else if (manilaType == ManilaFile::QTC)
		{
			stringstream ssCopy;
			ssCopy << "COPY " << inputName << " ";
			ssCopy << config.m9Workspace << "QTC";
			system(ssCopy.str().c_str());
		}
		else if (manilaType == ManilaFile::LOC_XML)
		{
			stringstream ssCopy;
			ssCopy << "COPY " << inputName << " ";
			ssCopy << config.m9Workspace << "XML";
			system(ssCopy.str().c_str());
		}
		else
		{
			stringstream ssCopy;
			ssCopy << "COPY " << inputName << " ";
			ssCopy << config.m9Workspace << "Other";
			system(ssCopy.str().c_str());
		}
	} // end dir for

	if (didM9extract == true)
	{
		string embDir = inputDir + "Embedded_Scripts";
		Directory dir(embDir.c_str());
		unsigned total = dir.files.size() - 1;
		for(unsigned i = 0; i < dir.files.size(); i++)
		{
			string iFileName = dir.files[i];
			string inputName = embDir + iFileName;
			ManilaFile::MType manilaType;
	
			manilaType = ManilaFile::detectManilaType(inputName);

			if (manilaType != ManilaFile::LUAC)
				continue;

			string outDir, outName;
			stringstream ss;
			outDir = config.luaWorkspace + removeExt(iFileName);
			
			ss << "MKDIR " << outDir;
			system(ss.str().c_str());

			outName = outDir + '\\' + removeExt(iFileName) + ".lua";

			// decompile task
			LuaDec ld(inputName);
			ld.decompile(outName);
			if (ld.errors.getLast() != "")
			{
				cout << ld.errors.getLast() << endl;
				ld.errors.clearAll();
				continue;
			}

			// compare output with original
			string cmpOutput;
			cmpOutput = removeExt(outName) + ".cmp.lua";
			LuaCompare lc(inputName, outName);

			// in case of bad decompile produce compare file and disassembly
			if (lc.compare() == false)
			{
				lc.writeTo(cmpOutput);

				if (lc.errors.getLast() != "")
				{
					string errFileName = removeExt(outName) + ".error.txt";
					TextFile errFile;
					errFile.addText(lc.errors.getLast());
					errFile.writeTo(errFileName);
					lc.errors.clearAll();
				}
				
				string disName = removeExt(outName) + ".dis.lua";
				ld.disassemble(disName);
			}

			// output success rate
			stringstream ss2;
			ss2 << "RENAME " << outDir << " ";
			ss2 << fixed << setprecision(0) << setw(3) << setfill('0');
			ss2 << lc.percent << "%_" << iFileName;
			system(ss2.str().c_str());

			// sort files
			stringstream ss3;
			ss3 << "COPY " << inputName << " " << config.binDecompiled;
			if (lc.percent == 100)
				ss3 << "Complete";
			else
				ss3 << "Failed";
			system(ss3.str().c_str());

			progress(i, total);
		}// end dir for
	} // end if
}

int LuaTool::m9extract(string inputFileName, string outputDir)
{
	// new file stream
	ifstream inFile(inputFileName.c_str(), ios::binary);
	
	// save file name and path separately
	string fileName, filePath;
	size_t slashPos;
	slashPos = inputFileName.rfind("\\", inputFileName.length() - 1);
	if (slashPos != string::npos)
	{	
		filePath = inputFileName.substr(0, slashPos+1);
		fileName = inputFileName.substr(slashPos+1);
	}
	else
		fileName = inputFileName;

	if (outputDir != "")
	{
		filePath = outputDir;
		if (outputDir[outputDir.length()-1] != '\\')
			filePath.append("\\");
	}

	// no file?
	if (!inFile) 
		return 0;
	// empty file?
	inFile.peek();
	if (inFile.eof())
		return 0;
	
	inFile.seekg (0, ios::end);
	size_t length = inFile.tellg();
	inFile.seekg (0, ios::beg);

	string str;
	for (size_t i = 0; i < length; i++)
		str.push_back(inFile.get());

	string sigLuac;
	sigLuac.push_back(0x1B);
	sigLuac.push_back('\0'); sigLuac.push_back('L');
	sigLuac.push_back('\0'); sigLuac.push_back('u');
	sigLuac.push_back('\0'); sigLuac.push_back('a'); 
	sigLuac.push_back('\0'); sigLuac.push_back('Q');
	string sigEnd;
	sigEnd.push_back('\1');	
	sigEnd.push_back('\0'); sigEnd.push_back('\0');
	sigEnd.push_back('\0');

	int count = 0;
	size_t startPos = 0, endPos = 0;
	while ( (startPos = str.find(sigLuac, endPos)) != string::npos)
	{
		if ( (endPos = str.find(sigEnd, startPos)) == string::npos )
			break;

		while( endPos != string::npos && endPos+19 < str.length() && str[endPos+19] != 0x7c )
			endPos = str.find(sigEnd, endPos+19);
				
		if (endPos == string::npos)
			endPos = str.length()-1;

		string embLuac;
		embLuac = str.substr(startPos, endPos - startPos);

		size_t pos = 0;
		string st;
		st.push_back(0x00); st.push_back(0x0D); st.push_back(0x0A);
		while( (pos = embLuac.find(st, pos)) != string::npos )
			embLuac.replace(pos+1, 1, "");

		stringstream outName;
		outName << filePath << fileName << "_" << count;
		outName << "_" << hex << setw(6) << setfill('0') << startPos;
		
		ofstream outFile( outName.str().c_str(), ios::binary );
		outFile.write(embLuac.c_str(), embLuac.length());
		outFile.close();

		count++;
	}

	return count;
}