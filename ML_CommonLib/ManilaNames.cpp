#include "ManilaNames.h"

#include <sstream>
using namespace std;

ManilaNames::ManilaNames(string inputFile)
: manilaNames(inputFile.c_str(), true)
{}

string ManilaNames::getManilaName(string hashedName)
{
	toLowerString(hashedName); // just in case

	list<string>::iterator it;
	for(it = manilaNames.textLines.begin(); it != manilaNames.textLines.end(); it++)
	{
		string strLine = *it;
		if (hash(strLine) == hashedName) 
			return strLine;
	}

	return "";
}

string ManilaNames::hash(string fileName) const
{
	// ========== hash ===========
	long hexHash = 0x1505;
	long temp;

    for (unsigned i = 0; i < fileName.length(); i++)
	{
		temp = hexHash + (hexHash << 5);
		hexHash = temp + tolower(fileName[i]);
		hexHash %= 0x100000000;
	}

	hexHash %= 0x80000000;
	
	// === form hashed file name ===
	stringstream ss;
	ss.width(8);
	ss.fill('0');
	ss << right << hex << hexHash;
	
	return ss.str() + "_manila";
}