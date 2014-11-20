// Class LoopBlock - maps loops in the function

#ifndef LoopBlocks_h
#define LoopBlocks_h

#include "Defs_LuaDec.h"

extern "C" {
#include "lopcodes.h"
}

#include <string>
#include <vector>
using namespace std;

class Function;

class LoopBlocks
{
public:
	LoopBlocks() {};

	void map(Function* functionToMap); // map all loop blocks in passed function

	// these methods return the loop type as string if the loop is at that pc
	// if not, they return an empty string
	string getStart(PcAddr pc);

	// TODO: look into these two
	string getEnd(PcAddr pc); 
	string peekEnd(PcAddr pc); // is this pc a block end?

	PcAddr getEndPC(PcAddr start); // get the end pc for this start pc
	PcAddr getStartPC(PcAddr end); // get the start pc for this end pc
	string getBreak(PcAddr pc); // is this pc a block break?
	
private:
	// main mapping methods
	void mapWhiles();
	void mapRepeats();
	void mapFors();
	void mapTfors();
	void mapDos();

	// helper methods
	void add(PcAddr startPC, string startStr, PcAddr endPC, string endStr = "end"); // add new block
	void setBreak(PcAddr pc, string breakStr = "break"); // set a break in the block
	void removeBreak(PcAddr pc); // remove a break from the block

private:
	Function* func; // pointer the function these loop block belong to

	class SingleLoopBlock
	{ 
	public:
		SingleLoopBlock(PcAddr start, string sStr, PcAddr end, string eStr) 
			: startPC(start), startStr(sStr), endPC(end), endStr(eStr), breakStr(""), endWritten(false) {};
	
		string startStr, breakStr, endStr;
		PcAddr startPC, endPC;
		bool endWritten;
		vector<PcAddr> breakPC;
	};
	vector<SingleLoopBlock> blocks;
};

#endif