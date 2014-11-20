// Class DecTable - decompiled table information

#ifndef DecTable_h
#define DecTable_h

#include <string>
#include <map>
#include <vector>
using namespace std;

class DecTable
{
public:
	DecTable(int b = 0, int c = 0) 
		: numSize(b), keyedSize(c) {};

	bool isFull();
	string getString();
	void setTable(string key, string value);
	void setNumeric(int index, string value);

private:
	class Pair 
	{
	public:
		Pair(string k, string v)
			:key(k), value(v) {};

		string key;
		string value;
	};

	int numSize;
	map<int, string> numeric;
	int keyedSize;
	vector<Pair> keyed;
};

#endif