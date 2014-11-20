#include "TextFile.h"

#include <iostream>
#include <fstream>
using namespace std;

TextFile::TextFile(string inputFileName, bool appendMode)
{
	if (appendMode)
		read(inputFileName);
	else
		create(inputFileName);
}

void TextFile::create(string inputFileName)
{
	if (inputFileName == "")
		return;

	// save file name and path separately
	size_t slashPos, endPos;
	endPos = inputFileName.length() - 1;
	slashPos = inputFileName.rfind("\\", endPos);
	if (slashPos != string::npos)
	{	
		filePath = inputFileName.substr(0, slashPos+1);
		fileName = inputFileName.substr(slashPos+1);
	}
	else
		fileName = inputFileName;

	textLines.clear();
}

void TextFile::read(string inputFileName)
{
	if (inputFileName == "")
		return;

	// save file name and path separately
	size_t slashPos, endPos;
	endPos = inputFileName.length() - 1;
	slashPos = inputFileName.rfind("\\", endPos);
	if (slashPos != string::npos)
	{	
		filePath = inputFileName.substr(0, slashPos+1);
		fileName = inputFileName.substr(slashPos+1);
	}
	else
		fileName = inputFileName;

	textLines.clear();

	// new file stream
	ifstream inFile(inputFileName.c_str());
	
	// no file?
	if (!inFile) 
		return;

	// empty file?
	inFile.peek();
	if (inFile.eof())
		return;
	
	//check for unicode xml
	char test[3];
	inFile.get(test, 3); // get first 2 characters of file
	inFile.seekg(0, ios::beg); // return to start of file
	if (test[0] == -1 && test[1] == -2)
	{ // 2 bytes per character encoding (this is for manila locales xmls)
		// loop through file to get all lines
		char c;
		inFile.get(); // skip first byte
		while( !inFile.eof() )
		{
			string newLine;

			inFile.get(); // skip second byte
			c = inFile.get(); // third byte is real start of file
			// get line
			while (c != '\n' && !inFile.eof())
			{
				newLine.push_back(c);

				inFile.get(); // skip upper half
				c = inFile.get(); // read lower half
			}

			// add to list
			textLines.push_back(newLine);
		}// end while
	}
	else
	{ // standard one byte encoding
		// loop through file to get all lines
		char c;
		while( !inFile.eof() )
		{
			string newLine;

			c = inFile.get();
			// get line
			while (c != '\n' && !inFile.eof())
			{
				newLine.push_back(c);
				c = inFile.get();
			}

			// add to list
			textLines.push_back(newLine);
		}// end while
	}
}

int TextFile::writeTo(string filename) const
{
	if (textLines.empty())
		return 0;

	ofstream outFile(filename.c_str());

	// error check
	if (!outFile)
		return 1; // can't write to file

	// write every line
	list<string>::const_iterator it;
	outFile << *textLines.begin();
	for (it = textLines.begin(), it++; it != textLines.end(); it++)
	{
		outFile << endl << *it;
	}
	return 1;
}

int TextFile::write() const
{
	return writeTo(filePath + fileName);
}

void TextFile::addLine(string& newLine)
{
	textLines.push_back(newLine);
}

void TextFile::addText(string& text)
{
	size_t startPos = 0, endPos = 0;
	string newLine;

	while((endPos = text.find('\n', startPos)) != string::npos)
	{
		newLine = text.substr(startPos, endPos - startPos);
		textLines.push_back(newLine);
		startPos = endPos+1;
	}

	if (startPos < text.length())
	{
		newLine = text.substr(startPos);
		textLines.push_back(newLine);
	}
}