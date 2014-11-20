#ifndef IfBlocks_h
#define IfBlocks_h

#include "Defs_LuaDec.h"
class Function;

#include "lopcodes.h"

#include <string>
#include <vector>
using namespace std;

class IfBlocks
{
public:
	IfBlocks() {};

	void map(Function* func);

	bool isStart(PcAddr pc); // is this pc an if start?
	bool isElse(PcAddr pc); // is this pc an else?
	bool isElseIf(PcAddr pc); // is this pc an elseif?
	bool isEnd(PcAddr pc); // is this pc an end?

	int getNumEnds(PcAddr pc); // number of ends at this pc
	string getIfStatement(PcAddr pc);
	
private:
	class SingleIfBlock
	{ 
	public:
		SingleIfBlock(PcAddr start = -1, PcAddr elseAddr = -1, PcAddr end = -1) 
			: startPC(start), elsePC(elseAddr), endPC(end), 
			elseIf(false), skipEnd(false) {};
	
		PcAddr startPC, elsePC, endPC;
		bool elseIf;
		bool skipEnd;
		string ifStatement;
	};

	void setElseIf(PcAddr elseIfPC);
	SingleIfBlock getParentBlock(PcAddr pc);
	bool ifsBetween(PcAddr pc1, PcAddr pc2);

	vector<SingleIfBlock> vec;
};

#endif