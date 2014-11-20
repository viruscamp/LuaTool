// directory.h
// Class Directory - get a list of files in dir

#ifndef directory_h
#define directory_h

#include <vector>
#include <string>
using namespace std;

class Directory
{
public:
	Directory() {};
	Directory(const char* inputDirName);
	
	void read(const char* inputDirName);

	vector<string> files;
	string dirName;
};

#endif