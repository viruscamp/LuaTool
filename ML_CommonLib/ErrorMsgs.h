#ifndef ErrorMsgs_h
#define ErrorMsgs_h

#include <string>
#include <vector>
using namespace std;

class ErrorMsgs
{
private:
	vector<string> errorMessages;

public:
	ErrorMsgs() {};

	void set(string newError)
	{ errorMessages.push_back(newError); };

	string getLast() // in case of no errors returns ""
	{ 
		if (errorMessages.empty() == false)
			return errorMessages.back();
		else
			return "";
	};

	void clearLast() // pop last error
	{ errorMessages.pop_back(); };

	vector<string> getAll() // get all errors
	{ return errorMessages; };
	
	void clearAll() // clear all error msgs
	{ errorMessages.clear(); };
};

#endif