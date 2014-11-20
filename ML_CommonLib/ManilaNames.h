#ifndef ManilaNames_h
#define ManilaNames_h

#include "TextFile.h"
#include <string>
using namespace std;

class ManilaNames
{
public:
	ManilaNames(string inputFile);

	string getManilaName(string hashedName); // find full manila name based on hashed name
	string hash(string fileName) const; // hash the passed string

private:
	TextFile manilaNames;

	void toLowerString(string &s)
	{
		for (unsigned i=0; i < s.length(); i++)
			s[i] = tolower(s[i]);
	}
};

#endif