// Class Assignments - general methods
#include "Assignments.h"

#include <sstream>
using namespace std;

string& Assignments::operator [](PcAddr index)
{
	return assignments[index];
}

void Assignments::add(string assignment)
{
	if (assignments[pc].empty())
		assignments[pc] = assignment;
	else
		assignments[pc] += "\n" + assignment;
}

string Assignments::makeIndex(string str, bool self)
{
	// TODO: review whole thing

	bool dot = false;
	// see if index can be expressed without quotes 
	if (str[0] == '\"') 
	{
		if (isalpha(str[1]) || str[1] == '_') 
		{
			int i = 1;
			dot = true;
			while (str[i] != '"') 
			{
				if (!isalnum(str[i]) && str[i] != '_')
				{
					dot = false;
					break;
				}
				i++;
			}
		}
	}
   
	if (dot) 
	{
		str.replace(0,1,"");
		str.replace(str.length()-1, 1, "");

		if (self)
			return ":" + str;
		else
			return "." + str;
	}
	else
		return "[" + str + "]";
}