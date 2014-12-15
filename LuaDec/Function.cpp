#include "Function.h"

#include <iostream>
#include <sstream>
using namespace std;

LuaState::LuaState()
{
	// open lua state
	L = lua_open();
	if (L == NULL)
	{
		// TODO: error handler
		return; // that's bad
	}
}

LuaState::~LuaState()
{
	if (L)
	{
		lua_close(L);
	}
}

lua_State* LuaState::getState()
{
	return L;
}

// default constructor
Function::Function(): shadow(NULL)
{
	//cerr << "default constructor\n";
}

// Global block constructors
Function::Function(string inputName, bool nosub, bool functionCompare)
: l(new LuaState()), codeSize(0), funcNumber("0"), isGlobal(true), nosub(nosub)
{
	//cerr << "Global block constructors\n";
	lua_State* L = l->getState();
	if (L == NULL)
		return; // that's bad

	// load file
	if (luaL_loadfile(L, inputName.c_str()) != 0)
	{
		errors.set("LuaDec: " + string(lua_tostring(L,-1)));
		return;
	}
	
	// get function proto
	const Closure* c = (const Closure*)lua_topointer(L,-1);
	Proto* f = c->l.p;

	// build function
	buildFromProto(f);

	shadow = NULL;
	if (functionCompare)
	{
		//cerr << "start makeShadow\n";
		makeShadow();
	}
}

Function::~Function()
{
	//cerr << "desctructor " << funcNumber << endl;
	if (isGlobal && shadow)
	{
		//cerr << "start shadow desctructor\n";
		delete shadow;
	}
}

void mapShadow(Function* org, Function* shadow)
{
	org->shadow = shadow;
	for (int i = 0; i < org->subFunctions.size(); i++)
	{
		mapShadow(org->subFunctions[i].get(), shadow->subFunctions[i].get());
	}
}

void Function::makeShadow()
{	
	shadow = new Function();
	shadow->l = l;
	shadow->isGlobal = isGlobal;
	shadow->proto = proto;
	shadow->funcNumber = funcNumber;
	shadow->nosub = true;
	shadow->shadow = NULL;

	shadow->buildFromProto(proto);

	mapShadow(this, shadow);
}

// Subfunction constructor
Function::Function(shared_ptr<LuaState> l, Proto *f, string number, vector<string> upvals, bool nosub)
: l(l), funcNumber(number), isGlobal(false), upvalues(upvals), nosub(nosub)
{
	//cerr << "Subfunction constructor " << number << endl;
	shadow = NULL;
	buildFromProto(f);
}

void Function::buildFromProto(Proto* f)
{
	opMap.resize(f->sizecode);
	subFunctions.resize(f->sizep);

	proto = f;
	indent = 0;
	
	// build register file
	reg.build(f);

	// get ops
	codeSize = f->sizecode;
	for (int pc=0; pc < codeSize; pc++)
		opMap[pc] = Op(f->code[pc]);

	// get locals info
	locals.mapProto(f);

	// build subfunctions
	if (f->sizep !=0)
	{
		for (int i=0; i < f->sizep; i++)
		{
			stringstream ss;
			ss << funcNumber << "_" << i;

			vector<string> upvals = getUpValues(f, i);
			subFunctions[i] = shared_ptr<Function>(new Function(l, f->p[i], ss.str(), upvals, nosub));
		}
	}

	// get function parameters
	numParameters = f->numparams;
	maxStackSize = f->maxstacksize;
	lineDefined = f->linedefined;
	
	freeLocal = f->numparams;

	// get variable parameters
	vararg = f->is_vararg;
	isVarArg = false;
	isVarArg2 = false;
	if (f->is_vararg==7) 
	{
		isVarArg = true;
		reg[freeLocal].setLocal("arg");
		freeLocal++;
	}
	if ((f->is_vararg & 2) && isGlobal == false)
	{	 
		isVarArg2 = true;
		freeLocal++;
	}
}

void Function::addDisLine(string line)
{
	if (line == "")
		return;

	disCode.append(line + "\n");
}

void Function::addStatement(string statement)
{
	if (statement == "")
		return;

	if (decCode.empty() || decCode[decCode.length()-1] == '\n')
	{ // print indent only at the beginning of a new line
		for (int i=0; i < indent; i++)
			decCode.append("  ");
	}

	if (statement.find('\n') == string::npos)
	{ // single line statement
		if (endOfLine.empty())
			decCode.append(statement + "\n");
		else 
		{
			decCode.append(statement + endOfLine + "\n");
			endOfLine.clear();
		}
	}
	else
	{ // multi line statement
		size_t startPos = 0, endPos = 0;
		string newLine;

		while((endPos = statement.find('\n', startPos)) != string::npos)
		{
			newLine = statement.substr(startPos, endPos - startPos);
			addStatement(newLine);
			startPos = endPos+1;
		}

		if (startPos < statement.length())
		{
			newLine = statement.substr(startPos);
			addStatement(newLine);
		}

		if (statement[statement.size()-1] == '\n') // empty line at the end
			addStatement(" ");
	}
}

void Function::addPartial(string start, string end)
{
	if (start == "")
		return;

	if (decCode.empty() || decCode[decCode.length()-1] == '\n')
	{ // print indent only at the beginning of a new line
		for (int i=0; i < indent; i++)
			decCode.append("  ");
	}

	decCode.append(start);
	endOfLine = end;
}

vector<string> Function::getUpValues(Proto* f, int numFunc)
{
	for(PcAddr pc = 0; pc < codeSize; pc++)
	{
		Op iOp = opMap[pc];

		if (iOp.opCode != OP_CLOSURE || iOp.bx != numFunc)
			continue;
		// upvalues are defined after the function closure

		int uvn = f->p[iOp.bx]->nups;
		vector<string> upvals(uvn);

		for (int i=0; i<uvn; i++)
		{
			Op curOp = opMap[pc+i+1];

			stringstream ss;
			if (curOp.opCode == OP_MOVE) 
			{
				ss << "l_" << funcNumber << "_" << curOp.b;
				upvals[i] = ss.str();
			} 
			else if (curOp.opCode == OP_GETUPVAL)
			{
				upvals[i] = upvalues[curOp.b];
			} 
			else
			{
				ss << "upval_" << funcNumber << "_" << i;
				upvals[i] = ss.str();
			}
		} // end inner for

		return upvals;
	} // end outer for
	return vector<string>();
}

bool Function::assignmentsBetween(PcAddr start, PcAddr end)
{
	for (PcAddr pc = start; pc < end; pc++)
	{
		if (assignments[pc] != "")
			return true;
	}
	return false;
}

Function* Function::getSubfunction(unsigned i)
{ 
	if (i < subFunctions.size())
		return subFunctions[i].get();
	else
		return this;
}

string Function::getDecompiledCode() const
{ 
	if (!isGlobal)
		return "SingleFunction = " + decCode;
	else
		return decCode; 
}


Function* Function::findSubFunction(const string funcnumstr) {
	Function* cf = this;
	const char* startstr = funcnumstr.c_str();
	const char* endstr;

	int c = atoi(startstr);
	if (c != 0) {
		return NULL;
	}
	endstr = strchr(startstr, '_');
	startstr = endstr + 1;
	stringstream realfuncnumstr;
	realfuncnumstr << "0";

	while (!(endstr == NULL)) {
		c = atoi(startstr);
		if (c < 0 || c >= cf->subFunctions.size()) {
			return NULL;
		}
		cf = cf->subFunctions[c].get();
		endstr = strchr(startstr, '_');
		startstr = endstr + 1;
		realfuncnumstr << "_" << c;
	}
	cf->funcNumber = realfuncnumstr.str();
	return cf;
}

string Function::listUpvalues()
{
	if (upvalues.size() == 0)
		return string("");

	stringstream ss;

	for (int i=0; i<upvalues.size(); i++)
	{
		if (i != 0)
			ss <<", ";
		ss << upvalues[i];
	}

	return ss.str();
}
