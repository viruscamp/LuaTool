#include "LuaTool.h"
#include "TextFile.h"

#include <list>
using namespace std;

LuaTool::Config::Config(string inputFileName)
{
	TextFile configFile(inputFileName, true);

	list<string>::const_iterator it;
	for (it = configFile.textLines.begin(); it != configFile.textLines.end(); it++)
	{
		string strLine = *it;
		string option, value;

		size_t pos;
		pos = strLine.find("=");
		if (pos != string::npos)
		{
			option = strLine.substr(0, pos);
			value = strLine.substr(pos+1);

			trim(option);
			trim(value);

			if (value[value.length()-1] != '\\')
				value.append("\\");
		}

		if (option == "Lua_Workspace_Path")
			luaWorkspace = value;
		else if(option == "Decompiled_Path")
			binDecompiled = value;
		else if(option == "Mode9_Workspace_Path")
			m9Workspace = value;
		else if(option == "Repository_Path")
			repository = value;
	}
}

void LuaTool::Config::trim(string& str)
{
	while (!str.empty() && str[0] == ' ')
		str.erase(0,1);
	while (!str.empty() && str[str.length()-1] == ' ')
		str.erase(str.length()-1,1);
}