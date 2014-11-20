// Class LogicExpressions - makes complex logic expressions which
// are later used by LoopBlocks and IfBlocks classes
#ifndef LogicExpressions_h
#define LogicExpressions_h

#include "Defs_LuaDec.h"
#include "Op.h"

#include <string>
#include <map>
#include <vector>
using namespace std;

class Function;

class LogicExpressions
{
public:
	LogicExpressions() {};

	void mapFunction(Function* functionToMap);
	string& operator[] (PcAddr pc);

	void addCondition(PcAddr pc, PcAddr destPC, Op op, string left, string right);

private:
	int mapExpression(int start, int end, bool isMain = true);
	void invertOp(Op &op);
	bool jumpsBetween(PcAddr start, PcAddr end);

private:
	Function* func; // owner function
	PcAddr pc; // current pc
	Op iOp; // current op

	map<PcAddr, string> logicExp; // expressions maped by pc

	class SingleCondition
	{
	public:
		SingleCondition(PcAddr p, PcAddr dp, Op o, string l, string r)
		: pc(p), destPC(dp), op(o), left(l), right(r), chain(""), base(false) {};

		PcAddr pc;
		PcAddr destPC;
		Op op;
		string left;
		string right;
		string chain;
		bool base;
	};
	vector<SingleCondition> conditions; // conditions passed from Assignments class
};

#endif