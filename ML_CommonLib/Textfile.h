// TextFile.h
// Class TextFile - Read and hold a text file

#ifndef TextFile_h
#define TextFile_h

#include <list>
#include <string>
using namespace std;

class TextFile
{
public:
	TextFile(string inputFileName = "", bool appendMode = false);

	void read(string inputFileName); // read from file
	void create(string inputFileName); // create new file
	
	int write() const; // write back to original file
	int writeTo(string outFileName) const; // write to new file

	void addText(string& text); // add new text
	void addLine(string& newLine);

	list<string> textLines; // list of text lines
	string filePath; // original input file path
	string fileName; // original input file name
};

#endif