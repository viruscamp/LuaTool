#include "ManilaFile.h"

#include <fstream>
using namespace std;

string ManilaFile::sigMode9bin = "\1\0\0\0\229\64\175";
const string ManilaFile::sigMode9xml = "<Mode9";
const string ManilaFile::sigLua1 = "luadec";
const string ManilaFile::sigLua2 = "LuaDec";
const string ManilaFile::sigLua3 = "LuaTool";
string ManilaFile::sigLuac = "L\0u\0a\0Q";
const string ManilaFile::sigLocXml1 = "<xliff";
const string ManilaFile::sigLocXml2 = "<?xml";
const string ManilaFile::sigQtc = "QTC1";

ManilaFile::ManilaFile(string inputFileName)
{
	read(inputFileName);
}

ManilaFile::MType ManilaFile::detectManilaType(string inputFileName)
{
	if (inputFileName == "")
		return ManilaFile::INVALID;

	string testStr;
	// get first 50 character
	testStr = getFirstN(50, inputFileName);

	sigLuac.clear();
	sigLuac.push_back('L');	sigLuac.push_back('\0'); sigLuac.push_back('u');
	sigLuac.push_back('\0'); sigLuac.push_back('a'); sigLuac.push_back('\0');
	sigLuac.push_back('Q');

	sigMode9bin.clear();
	sigMode9bin.push_back('\1'); sigMode9bin.push_back('\0'); sigMode9bin.push_back('\0');
	sigMode9bin.push_back('\0');

	MType mt;
	// set manila file type
	if (testStr.find(sigLuac) != string::npos)
		mt = ManilaFile::LUAC;
	else if (testStr.find(sigLocXml1) != string::npos || testStr.find(sigLocXml2) != string::npos)
		mt = ManilaFile::LOC_XML;
	else if (testStr.find(sigQtc) != string::npos)
		mt = ManilaFile::QTC;
	else if (testStr.find(sigMode9bin) != string::npos)
		mt = ManilaFile::MODE9_BIN;
	else if (testStr.find(sigLua1) != string::npos || testStr.find(sigLua2) != string::npos || testStr.find(sigLua3) != string::npos)
		mt = ManilaFile::LUA;
	else if (testStr.find(sigMode9xml) != string::npos)
		mt = ManilaFile::MODE9_XML;
	else
		mt = ManilaFile::INVALID;

	return mt;
}

void ManilaFile::read(string inputFileName)
{
	manilaType = detectManilaType(inputFileName);

	if (manilaType == ManilaFile::INVALID)
		return;

	TextFile::read(inputFileName);
}

string ManilaFile::getFirstN(unsigned n, string inputFileName)
{
	string str = ""; // string to be returned

	if (inputFileName == "")
		return str;

	ifstream inFile(inputFileName.c_str()); // new file stream
	
	if (!inFile) 
		return str; // no file
	inFile.peek();
	if (inFile.eof())
		return str; // empty file

	char test[3]; //check for unicode
	inFile.get(test, 3); // get first 2 characters of file
	inFile.seekg(0, ios::beg); // return to start of file
	if (test[0] == -1 && test[1] == -2)
	{ // 2 bytes per character encoding (this is for manila locales xmls)
		char c;
		unsigned i = 0;
		inFile.get(); // skip first byte
		inFile.get(); // skip second byte

		c = inFile.get(); // third byte is real start of file
		// get first 50 characters to test file type
		while (i < n && !inFile.eof())
		{
			str.push_back(c);
			inFile.get(); // skip upper half
			c = inFile.get(); // read lower half
			i++;
		}
	}
	else
	{ // standard one byte encoding
		char c;
		unsigned i = 0;
		c = inFile.get();

		// get first 50 characters to test file type
		while (i < n && !inFile.eof())
		{
			str.push_back(c);
			c = inFile.get();
			i++;
		}
	}

	return str;
}