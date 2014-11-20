// Class Locals - maps local variable declarations

#ifndef Locals_h
#define Locals_h

#include "Defs_LuaDec.h"
#include "Op.h"

#include <string>
#include <vector>
#include <map>
using namespace std;

class Function;
struct Proto;

class Locals
{
public:
	// local variable subclass
	class Local
	{
	public:
		Local() {};
		Local(int r, PcAddr start, PcAddr end) 
			: reg(r), startPC(start), endPC(end) {};
		
		int reg; // the register this local is assigned to
		PcAddr startPC; // local declaration pc
		PcAddr endPC; // local release pc
		string name; // local name
	};

public:
	Locals() {}; // empty constructor

	void mapProto(Proto* f); // non-stripted scripts keep the local data in their proto a.k.a. the easy way
	void mapFunction(Function* functionToMap); // striped scripts need to be searched for locals a.k.a. the hard way
	
	Local& operator[] (size_t index); // [] operator for easy external info access
	size_t size(); // returns the local variable count

private:
	void prepare(); // pre-start preparation
	void mapUsage(); // this stage maps register usage by pc
	void mapLocals(); // this stage maps locals based on usage info from prevous stage
	void nameLocals(); // final stage, just assigns names to locals

private:
	// functions for per pc usage info gathering
	void setReg(int setReg, int setRegTo = -1);
	void setReg2(int setReg2);
	void loadReg(int loadReg, int loadRegTo = -1);
	void loadReg2(int loadReg2);
	void loadReg3(int loadReg3);
	void forceDeclare(int fromReg, int toReg);

	// functions for block info gathering and access
	void addBlock(PcAddr start, PcAddr end, int level);
	void addRepeat(PcAddr start, PcAddr end, int level);
	int getBlockLevel(PcAddr pc);
	PcAddr blockEnd(PcAddr pc);

	// store potential variable information
	void addPotVar(int reg, PcAddr start, PcAddr end);
	struct PotVar;
	PotVar getPotVar(int reg, PcAddr pc);

	// definitely found locals
	void declareLocal(int reg, PcAddr start, PcAddr end, string name = "");
	void declareLocal(int reg, PcAddr pc);

	// helper function
	inline bool isConst(int r) 
	{ return r >= 256; };

private:
	void opSwitch(OpCode opCode); // calls the right method for the passed opcode
	void opMove(); // copy from one register to another
	void opLoadK(); // load constant into a register
	void opLoadBool(); // load bool into a register
	void opLoadNil(); // load nil into register
	void opVarArg(); // load "..." into register
	void opGetUpval(); // load upvalue into register
	void opSetUpval(); // set upvalue from register
	void opGetGlobal(); // load global into register
	void opSetGlobal(); // set global from register
	void opNewTable(); // load new table into register
	void opGetTable(); // load table entry into register
	void opSetTable(); // set table entry from register
	void opSetList(); // set numeric table entries
	void opSelf(); // self call
	void opBinary(); // binary operation (+, -, *, /, %, ^)
	void opUnary(); // unary operation (-, not, length of)
	void opConcat(); // combine data in registers
	void opCall(); // function call
	void opTailCall(); // return call
	void opReturn(); // function return
	void opClosure(); // function
	void opCompare(); // compare operation (<, <=, ==)
	void opTestSet(); // test a bool value then set it
	void opTest(); // test a bool value
	void opForPrep(); // setup numeric for loop header
	void opJmp(); // setup general for loop header

private:
	Function* func; // owner function
	PcAddr pc; // current pc
	PcAddr pcStep; // in case a few ops need to be skiped
	Op iOp; // current op
	bool df; // used for debug

	int lastfree; // last free register
	vector<Local> locals; // local declaration info

	// block map
	struct BlockInfo
	{
		PcAddr start;
		PcAddr end;
		int level;
	};
	int block;
	vector<BlockInfo> repeats;
	vector<BlockInfo> blockInfo;

	// potential variables
	struct PotVar
	{
		int start;
		int end;
	};
	map<int, vector<PotVar>> potVar;

	// per pc usage information
	class UseInfo
	{
	public:
		UseInfo() 
			: setReg(-1), setRegTo(-1), setReg2(-1),
			loadReg(-1), loadRegTo(-1), loadReg2(-1), loadReg3(-1),
			forceDeclareFrom(-1), forceDeclareTo(-1), selfLoad(-1), multiCall(-1) {};

		int setReg;
		int setRegTo;
		int setReg2;
		int loadReg;
		int loadReg2;
		int loadReg3;
		int loadRegTo;
		int forceDeclareFrom;
		int forceDeclareTo;
		int selfLoad;
		int multiCall;
	};
	map<PcAddr, UseInfo> useInfo;

	// per register usage level information
	class RegUsage
	{
	public: 
		RegUsage() {};

		void inc(PcAddr pc);
		void dec(PcAddr pc);
		const int operator[] (PcAddr pc);
		void setLevel (int usageLevel, PcAddr pc);
		void normalize(PcAddr pc);

	private:
		struct Usage
		{
			PcAddr pc;
			int usageLevel;
		};

		vector<Usage> usage;
	};
	map<int, RegUsage> regUsage;
};

#endif