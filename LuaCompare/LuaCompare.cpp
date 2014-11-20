#include "LuaCompare.h"
const string LuaCompare::version = "1.2.1";

#include "ManilaFile.h"
#include "LuaDec.h"
#include "LuaC.h"

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <list>
#include <cmath>
using namespace std;

LuaCompare::LuaCompare(string originalFileName, string newFileName)
: overallSame(false), percent(0)
{
	loadFiles(originalFileName, newFileName);
}

void LuaCompare::loadFiles(string originalFileName, string newFileName)
{
	// =========================
	// disassemble original file
	LuaDec ldOriginal(originalFileName);
	TextFile originalDis = ldOriginal.disassembleForCompare();

	if (ldOriginal.errors.getLast() != "")
	{
		errors.set(ldOriginal.errors.getLast());
		return;
	}
	
	// ====================
	// disassemble new file
	LuaDec ldNew(newFileName);
	TextFile newDis = ldNew.disassembleForCompare();

	if (ldNew.errors.getLast() != "")
	{
		errors.set(ldNew.errors.getLast());
		return;
	}

	// ========================================
	// ========================================
	maxLen = 0;
	originalLineCount = 0;

	list<string>::const_iterator itOrig, itNew;
	itOrig = originalDis.textLines.begin();
	itNew = newDis.textLines.begin();
	// ===============================
	// map all functions from original
	for(unsigned i = 0; itOrig != originalDis.textLines.end(); i++)
	{
		if (i == 0) // clear global header
			do itOrig++; while (itOrig != originalDis.textLines.end() && *itOrig != "" && *itOrig != " ");
		// clear function header
		do itOrig++; while (itOrig != originalDis.textLines.end() && *itOrig != "" && *itOrig != " ");
		itOrig++;

		// get ops
		while (itOrig != originalDis.textLines.end() && *itOrig != "" && *itOrig != " ") {
			funcOrig[i].push_back(*itOrig);
			originalLineCount++;
			itOrig++; 
		}
		
		// clear empty lines
		while (itOrig != originalDis.textLines.end() && (*itOrig == "" || *itOrig == " "))
			itOrig++;
	}// end orig for

	// ======================
	// same thing for new dis
	for(unsigned i = 0; itNew != newDis.textLines.end(); i++)
	{
		if (i == 0)
			do itNew++; while (itNew != newDis.textLines.end() && *itNew != "" && *itNew != " ");
		do itNew++; while (itNew != newDis.textLines.end() && *itNew != "" && *itNew != " ");
		itNew++;

		while (itNew != newDis.textLines.end() && *itNew != "" && *itNew != " ") {
			funcNew[i].push_back(*itNew);
			itNew++;
		}
		
		while (itNew != newDis.textLines.end() && (*itNew == "" || *itNew == " "))
			itNew++;
	} // end new for

	// ========================
	// find maximum line lenght
	string strOrig, strNew;
	for (unsigned i = 0; i < funcOrig.size(); i++) {
		itOrig = funcOrig[i].begin();
		itNew = funcNew[i].begin();
		while(itOrig != funcOrig[i].end() || itNew != funcNew[i].end())
		{
			if (itOrig != funcOrig[i].end()) strOrig = *itOrig;
			else strOrig = "nil";

			if (itNew != funcNew[i].end()) strNew = *itNew;
			else strNew = "nil";

			if (strOrig != strNew && maxLen < strOrig.length())
				maxLen = strOrig.length();

			if (itOrig != funcOrig[i].end()) itOrig++;
			if (itNew != funcNew[i].end()) itNew++;
		}
	} // end max len for
}

bool LuaCompare::compare(int functionNumber, bool underline, bool sideBySide)
{
	if (funcOrig.empty() || funcNew.empty())
		return false;

	int globalSameCount = 0;
	list<string>::const_iterator itOrig, itNew;
	
	// ===============
	// what to compare
	int start, end, diff;
	if (functionNumber <= 0 || functionNumber >= funcOrig.size())
	{
		start = 0;
		end = funcOrig.size();
		diff = 0;
	}
	else
	{
		start = functionNumber;
		end = functionNumber + 1;
		diff = functionNumber - 1;
	}

	// ============================
	// compare function by function
	string strOrig, strNew;
	for (unsigned i=start; i < end; i++)
	{
		bool same = true; // assume functions are same
		int numSame = 0;

		// function name
		if (i == 0)
			compareOut.textLines.push_back("Main block:");
		else {
			stringstream ss;
			ss << "Function " << i << ":";
			compareOut.textLines.push_back(ss.str());
		}

		// go through function lines
		itOrig = funcOrig[i].begin();
		itNew = funcNew[i-diff].begin();
		bool threeDots = false;
		while(itOrig != funcOrig[i].end() || itNew != funcNew[i-diff].end())
		{
			// set string where possible
			if (itOrig != funcOrig[i].end())
				strOrig = *itOrig;
			else
				strOrig = "nil";

			if (itNew != funcNew[i-diff].end())
				strNew = *itNew;
			else
				strNew = "nil";

			stringstream ss;
			if (strOrig != strNew)
			{ // line are different
				same = false;
				threeDots = true;
				if (sideBySide) {
					ss << setw(maxLen) << left << strOrig << " | " << strNew;
				} else {
					ss << "  1>" << strOrig << endl;
					ss << "  2>" << strNew;
					if (underline)
						ss << endl << setw(maxLen + 4) << setfill('-') << "-";
				}
				compareOut.textLines.push_back(ss.str());
			}
			else 
			{ // lines are same
				numSame++;
				globalSameCount++;

				if (underline && threeDots) {
					ss << "..." << endl << setw(maxLen + 4) << setfill('-') << "-";
					threeDots = false;
					compareOut.textLines.push_back(ss.str());
				}
			}

			// move on where possible
			if (itOrig != funcOrig[i].end()) itOrig++;
			if (itNew != funcNew[i-diff].end()) itNew++;
		} // end while

		// are all lines the same?
		stringstream ss;
		if (same)
			ss << "Same";
		else
		{
			ss << "Opcodes in original: " << funcOrig[i].size() << endl;
			ss << "Same lines in both files: " << numSame << " ";
			double per = floor(100.00*numSame / funcOrig[i].size());
			ss << fixed << setprecision(0) << per << "%";
		}
		if (underline)
			ss << endl << setw(maxLen + 4) << setfill('=') << "=";
		compareOut.textLines.push_back(ss.str());

		// save same stat for this function and move to next
		perFuncSame[i] = same;
	}// end for

	// global
	stringstream ss;
	ss << endl;
	if (underline)
		ss << setw(maxLen + 4) << setfill('=') << "=" << endl;
	ss << "Global:" << endl;
	ss << "Opcodes in original: " << originalLineCount << endl;
	ss << "Same lines in both files: " << globalSameCount << " ";
	
	if (start == 0)
		percent = floor(100.00*globalSameCount / originalLineCount);
	else
		percent = perFuncSame[functionNumber] ? 100.00 : 0.00;

	ss << fixed << setprecision(0) << percent << "%";
	ss << endl;
	compareOut.textLines.push_back(ss.str());

	if (percent == 100)
		overallSame = true;
	else
		overallSame = false;

	return overallSame;
}

void LuaCompare::writeTo(string outFileName) const
{
	compareOut.writeTo(outFileName);
}