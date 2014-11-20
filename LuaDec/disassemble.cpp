// Dissassemble
// Note: still needs some cleaning up, but not a priority
#include "Function.h"

#include <sstream>
#include <iomanip>
using namespace std;

// helpers
#define opstr(o) ((o)==OP_EQ?"==":(o)==OP_LE?"<=":(o)==OP_LT?"<":(((o)==OP_TEST)||((o)==OP_TESTSET))?NULL:"?")
#define invopstr(o) ((o)==OP_EQ?"~=":(o)==OP_LE?">=":(o)==OP_LT?">":(((o)==OP_TEST)||((o)==OP_TESTSET))?"not":"?")

char* operators[22] =
    { " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ",
	"+", "-", "*", "/", "%", "^", "-", "not ", "#", ".." };

char getPrefix(int x)
{
	if (x >= 256)
		return 'K';
	else
		return 'R';
}

string Function::disassemble()
{
	// TODO: print name
	stringstream ssHeader;
	ssHeader << "; Name:            \n";
	ssHeader << "; Defined at line: " << lineDefined << endl;
	ssHeader << "; #Upvalues:       " << upvalues.size() << endl;
	ssHeader << "; #Parameters:     " << numParameters << endl;
	ssHeader << "; Is_vararg:       " << vararg << endl;
	ssHeader << "; Max Stack Size:  " << maxStackSize << endl;

	addDisLine(ssHeader.str());

	for (int pc = 0; pc < codeSize; pc++)
	{
		// current op
		Op op = opMap[pc];
		char aPrefix = getPrefix(op.a);
		char bPrefix = getPrefix(op.b);
		char cPrefix = getPrefix(op.c);
		char bxPrefix = getPrefix(op.bx);
		char sbxPrefix = getPrefix(op.sbx);

		if (aPrefix == 'K')
			op.a -= 256;
		if (bPrefix == 'K')
			op.b -= 256;
		if (cPrefix == 'K')
			op.c -= 256;

		stringstream line, lend;

		switch (op.opCode) {
	      case OP_MOVE:
			  line << aPrefix << op.a << " " << bPrefix << op.b;
			  lend << aPrefix << op.a << " := " << bPrefix << op.b;
			 break;
		  case OP_LOADK:
			  line << aPrefix << op.a << " K" << op.bx;
			  lend << aPrefix << op.a << " := " + reg.getBxConstant(op.bx);
			  break;
		  case OP_LOADBOOL:
			  line << aPrefix << op.a << " " << op.b << " " << op.c;
			  if (op.b) {
				  if (op.c)
					  lend << aPrefix << op.a << " := true; PC := " << pc+2;
				  else
					  lend << aPrefix << op.a << " := true";
			  } else {
				  if (op.c)
					  lend << aPrefix << op.a << " := false; PC := " << pc+2;
				  else
					  lend << aPrefix << op.a << " := false";
			  }
			  break;
		  case OP_LOADNIL:
			  line << aPrefix << op.a << " " << bPrefix << op.b;
			  for (int i = op.a; i <= op.b; i++)
				  lend << "R" << i << " := ";
			  lend << "nil";
			  break;
		  case OP_VARARG:
			  line << aPrefix << op.a << " " << bPrefix << op.b;
			  for (int i = op.a; i <= op.b; i++)
				  lend << "R" << i << " := ";
				 
			  lend << "...";
			  break;
		  case OP_GETUPVAL:
			  line << aPrefix << op.a << " " << "U" << op.b;
			  lend << aPrefix << op.a << " := " << "U" << op.b;
			  break;
		  case OP_GETGLOBAL:
			  line << aPrefix << op.a << " " << "K" << op.bx;
			  lend << aPrefix << op.a << " := " << reg.getGlobal(op.bx);
			  break;
		  case OP_GETTABLE:
			  line << aPrefix << op.a << " " << bPrefix << op.b << " " << cPrefix << op.c;
			  if (cPrefix == 'K')
				  lend << "R" << op.a << " := R" << op.b << "[" << reg.getBxConstant(op.c) << "]";
			  else
				  lend << "R" << op.a << " := R" << op.b << "[R" << op.c << "]";
			  break;
		  case OP_SETGLOBAL:
			  line << aPrefix << op.a << " " << "K" << op.bx;
			  lend << reg.getGlobal(op.bx) << " := " << aPrefix << op.a;
			  break;
		  case OP_SETUPVAL:
			  line << aPrefix << op.a << " " << "U" << op.b;
			  lend << "U" << op.b << " := " << aPrefix << op.a;
			  break;
		  case OP_SETTABLE:
			  line << aPrefix << op.a << " " << bPrefix << op.b << " " << cPrefix << op.c;
			  if (bPrefix == 'K') {
				  if (cPrefix == 'K')
					  lend << "R" << op.a << "[" << reg.getBxConstant(op.b) << "] := " << reg.getBxConstant(op.c);
				  else
					  lend << "R" << op.a << "[" << reg.getBxConstant(op.b) << "] := R" << op.c;
			  } else {
				  if (cPrefix == 'K')
					  lend << "R" << op.a << "[R" << op.b << "] := " << reg.getBxConstant(op.c);
				  else
					  lend << "R" << op.a << "[R" << op.b << "] := R" << op.c;
			  }
			  break;
		  case OP_NEWTABLE:
			  line << aPrefix << op.a << " " << op.b << " " << op.c;
			  lend << aPrefix << op.a << " := {}";
			  break;
		  case OP_SELF:
			  line << "R" << op.a << " R" << op.b << " " << cPrefix << op.c;
			  if (cPrefix == 'K')
				  lend << "R" << op.a+1 <<" := R" << op.b << "; R" << op.a << " := R" << op.b << "[" << reg.getBxConstant(op.c) << "]";
			  else
				  lend << "R" << op.a+1 <<" := R" << op.b << "; R" << op.a << " := R" << op.b << "[R" << op.c << "]";
			  break;
		  case OP_ADD:
		  case OP_SUB:
		  case OP_MUL:
		  case OP_DIV:
		  case OP_POW:
		  case OP_MOD:
			  line << aPrefix << op.a << " " << bPrefix << op.b << " " << cPrefix << op.c;
			  if (bPrefix == 'K') {
				  if (cPrefix == 'K')
					  lend << "R" << op.a <<" := " << reg.getBxConstant(op.b) <<" "<< operators[op.opCode] <<" "<< reg.getBxConstant(op.c);
				  else 
					  lend << "R" << op.a <<" := " << reg.getBxConstant(op.b) <<" "<< operators[op.opCode] <<" R" << op.c;
			  } else {
				  if (cPrefix == 'K')
					  lend << "R" << op.a <<" := R" << op.b <<" "<< operators[op.opCode] <<" "<< reg.getBxConstant(op.c);
				  else
					  lend << "R" << op.a <<" := R" << op.b <<" "<< operators[op.opCode] <<" R" << op.c;
			  }
			  break;
		  case OP_UNM:
		  case OP_NOT:
		  case OP_LEN:
			  line << aPrefix << op.a << " " << bPrefix << op.b;
			  if (bPrefix == 'K')
				  lend << "R" << op.a <<" := " << operators[op.opCode] <<" "<< reg.getBxConstant(op.b);
			  else
				  lend << "R" << op.a <<" := " << operators[op.opCode] <<" R"<< op.b;
			  break;
		  case OP_CONCAT:
			  line << aPrefix << op.a << " " << bPrefix << op.b << " " << cPrefix << op.c;
			  lend << "R" << op.a << " := ";
			  for (int i = op.b; i < op.c; i++)
				  lend << "R" << i << " .. ";
			  lend << "R" << op.c;
			  break;
		  case OP_JMP:
			  {
			  int dest = op.sbx+pc+1;

			  line << dest;
			  lend << "PC := " << dest;
			  break;
			  }
		  case OP_EQ:
		  case OP_LT:
		  case OP_LE:
			  {
			  int dest = opMap[pc+1].sbx + pc+1 + 1;
			  stringstream ssB, ssC;

			  line << op.a << " " << bPrefix << op.b << " " << cPrefix << op.c;
			  
			  if (bPrefix == 'K')
				  ssB << reg.getBxConstant(op.b);
			  else
				  ssB << "R" << op.b;

			  if (cPrefix == 'K') 
				  ssC << reg.getBxConstant(op.c);
			  else
				  ssC << "R" << op.c;

			  if (op.a)
				  lend << "if " << ssB.str() <<" "<< opstr(op.opCode) <<" "<< ssC.str() << " then PC := " << dest;
			  else
				  lend << "if " << ssB.str() <<" "<< invopstr(op.opCode) <<" "<< ssC.str() << " then PC := " << dest;

			  break;
			  }
		  case OP_TEST:
			  {
			  int dest = opMap[pc+1].sbx + pc+1 + 1;
			  stringstream ss;

			  line << aPrefix << op.a << " " << op.c;
			  

			  if (aPrefix == 'K')
				  ss << reg.getBxConstant(op.a);
			  else
				  ss << "R" << op.a;
			
			  if (op.c) 
				  lend << "if " << ss.str() << " then PC := " << dest;
			  else
				  lend << "if not " << ss.str() << " then PC := " << dest;
			
			  break;
			  }
		  case OP_TESTSET:
			  {
				int dest = opMap[pc+1].sbx + pc+1 + 1;
				stringstream ssA, ssB;

				line << aPrefix << op.a <<" "<< bPrefix << op.b <<" "<< op.c;
			 
				if (aPrefix == 'K')
					ssA << reg.getBxConstant(op.a);
				else
					ssA << "R" << op.a;

				if (bPrefix == 'K') 
					ssB << reg.getBxConstant(op.b);
				else
					ssB << "R" << op.c;
				
				if (op.c) 
					lend << "if " << ssB.str() << " then PC := " << dest << " else " << ssA.str() <<" := "<< ssB.str();
				else 
					lend << "if not " << ssB.str() << " then PC := " << dest << " else " << ssA.str() <<" := "<< ssB.str();
				 
				break;
			  }
		  case OP_CALL:
		  case OP_TAILCALL:
			  {
			  line << "R" << op.a <<" "<< op.b <<" "<< op.c;
			 
			  stringstream ss1, ss2;
			  if (op.b >= 2)
			  {
				  for (int i = op.a+1; i < op.a+op.b-1; i++)
					  ss1 << "R" << i << ",";
				  ss1 << "R" << op.a+op.b-1;
			  } 
			  else if (op.b == 0)
				  ss1 << "R" << op.a+1 << ",...";
			  else
				  ss1 << "";

			  if (op.c>=2) 
			  {
				  for (int i = op.a; i < op.a+op.c-2; i++)
					  ss2 << "R" << i << ",";
						
				  ss2 << "R" << op.a+op.c-2 << " := ";
			  } 
			  else if (op.c == 0)
				  ss2 << "R" << op.a << ",... := ";
			  else 
				  ss2 << "";

			  lend << ss2.str() << "R" << op.a << "(" << ss1.str() << ")";
			  break;
			  }
		  case OP_RETURN:
			  {
			  line << "R" << op.a <<" "<< op.b;
					
			  stringstream ss;
			  if (op.b >= 2)
			  {
				  for (int i = op.a; i < op.a+op.b-2; i++)
					  ss << "R" << i << ",";

				  ss << "R" << op.a+op.b-2;
			  }
			  else if (op.b == 0)
				  ss << "R" << op.a << ",...";
			  else
				  ss << "";

			  lend << "return " << ss.str();
			  break;
			  }
		  case OP_FORLOOP:
			  line << "R" << op.a <<" "<< pc+op.sbx+1;
			  lend << "R" << op.a <<" += R"<< op.a+2 <<"; if R"<< op.a <<" <= R" << op.a+1 <<" then begin PC := "<< pc+op.sbx+1 <<"; R"<< op.a+3 <<" := R"<< op.a << " end"; 
			  break;
		  case OP_TFORLOOP: 
			  {
			  int dest = opMap[pc+1].sbx + pc+1 + 1;
			  line << "R" << op.a <<" "<< op.c;

			  stringstream ss;
			  if (op.c >= 1)
			  {
				  for (int i = op.a+3; i< op.a+op.c+2; i++)
					  ss << "R" << i << ",";
				  ss<< "R" << op.a+op.c+2 << " := ";
			  } 
			  else
				  ss << "R" << op.a << ",... := ";

			  lend << ss.str() <<" R"<< op.a <<"(R"<< op.a+1 <<",R"<< op.a+2 <<"); if R"<< op.a+3 <<" ~= nil then begin PC := "<< dest <<"; R"<< op.a+2 <<" := R"<< op.a+3 <<" end";
			  break;
			  }
		  case OP_FORPREP:
			  line << "R" << op.a <<" "<< pc+op.sbx+1;
			  lend << "R" << op.a <<" -= R" << op.a+2 <<"; PC := "<< pc+op.sbx+1;
			  break;
		  case OP_SETLIST:
			  line << "R" << op.a <<" "<< op.b <<" "<< op.c;
			  lend << "R" << op.a <<"[("<< op.c <<"-1)*FPF+i] := R("<< op.a <<"+i), 1 <= i <= " << op.b;
			  break;         
		  case OP_CLOSE:
			  line << "R" << op.a;
			  lend << "SAVE R" << op.a << ",...";
			  break;
		  case OP_CLOSURE:
			  line << "R" << op.a <<" "<< op.bx;
				
			  if (funcNumber == "0")
				  lend << "R" << op.a <<" := closure(Function #" << op.bx+1 << ")";
			  else
				  lend << "R" << op.a <<" := closure(Function #" << funcNumber <<"."<< op.bx+1 << ")";
			  break;
		  default:
			  break;
      }// end switch
	  stringstream ssLine;
	  ssLine << setw(3) << pc << " [-]: " << left << setw(10) << luaP_opnames[op.opCode]; 
	  ssLine << left << setw(13) << line.str() << "; " << lend.str();
	  addDisLine(ssLine.str());
	}// end for

	// cosmetic: empty lines between functions
	if(isGlobal)
		addDisLine(" ");
	addDisLine(" ");

	if (subFunctions.empty() != true)
	{	
		for (unsigned int i=0; i < subFunctions.size(); i++) {
			stringstream ss;
			if (funcNumber == "0")
				ss << i+1;
			else
				ss << funcNumber << "." << i+1;

			addDisLine("; Function #" + ss.str() + ":");
			addDisLine(";");
			addDisLine(subFunctions[i].disassemble());
		}
	}

	return disCode;
}