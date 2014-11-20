#include "DecTable.h"

#include <sstream>
using namespace std;

bool DecTable::isFull()
{
	if (keyed.size() >= keyedSize && numeric.size() >= numSize)
		return true;
	else
		return false;
}

void DecTable::setTable(string key, string value)
{
	keyed.push_back(Pair(key, value));
}

void DecTable::setNumeric(int index, string value)
{
	numeric[index] = value;
}

string DecTable::getString()
{
	stringstream ss;
	ss << "{";

	// numeric items
	if (!numeric.empty())
	{
		bool first = true;
		map<int, string>::const_iterator it;
		for(it = numeric.begin(); it != numeric.end(); it++)
		{
			if (first)
				first = false;
			else
				ss << ", ";

			ss << it->second;
		}
	}

	// keyed items
	if (!keyed.empty())
	{
		if (!numeric.empty())
			ss << "; ";

		bool first = true;
		vector<Pair>::const_iterator it;
		for(it = keyed.begin(); it != keyed.end(); it++)
		{
			if (first)
				first = false;
			else
				ss << ", ";

			string key = it->key;
			if (key[0] == '\"')
			{
				key.replace(0,1,"");
				key.replace(key.length()-1, 1, "");
				ss << key <<" = "<< it->value;
			}
			else
				ss <<"["<< key <<"] = "<< it->value; 
		}// end for
	}// end if

	ss << "}";

   return ss.str();
}