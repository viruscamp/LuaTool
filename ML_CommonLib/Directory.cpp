// I think this code is a mess,
// but it does it's job so guess that's OK.
// In any case it'll have to do until I get some time
// to remake this whole thing using .Net...

#include "Directory.h"
#include <windows.h>
#include <tchar.h> 
#include <strsafe.h>
using namespace std;

Directory::Directory(const char* inputDirName)
{
	read(inputDirName);
}

void Directory::read(const char *inputDirName)
{
	if(inputDirName == NULL)
		return;

	dirName = inputDirName;
	files.clear();

	WIN32_FIND_DATA ffd;
	TCHAR strDir[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;

	for(unsigned i = 0; i <= strlen(inputDirName); i++)
		strDir[i] = inputDirName[i];
	StringCchCat(strDir, MAX_PATH, TEXT("\\*"));
		
	// Find the first file in the directory.
	hFind = FindFirstFile(strDir, &ffd);

    if (INVALID_HANDLE_VALUE == hFind) 
    {
       return;
    } 
	
	// List all the files in the directory with some info about them.
	string strFileName = "";
	do
    {
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{ // it's a subdirectory
		}
		else
		{ // it's a file
			for(int i = 0; ffd.cFileName[i] != NULL; i++)
				strFileName.push_back((char)ffd.cFileName[i]);
			files.push_back(strFileName);
			strFileName.clear();
		}
	}
	while (FindNextFile(hFind, &ffd) != 0);
 
	FindClose(hFind);
}