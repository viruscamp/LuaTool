#include "IfBlocks.h"
#include "Function.h"

using namespace std;

bool IfBlocks::isStart(PcAddr pc)
{
	for(int i=0; i < vec.size(); i++)
	{
		if (vec[i].startPC == pc)
			return true;
	}
	return false;
}

string IfBlocks::getIfStatement(PcAddr pc)
{
	for(int i=0; i < vec.size(); i++)
	{
		if (vec[i].startPC == pc)
		{
			return vec[i].ifStatement;
		}
	}

	return "ERROR (IfBlocks::getIfStatement)";
}

bool IfBlocks::isElse(PcAddr pc)
{
	if (pc == -1)
		return false;

	for(int i=0; i < vec.size(); i++)
	{
		if (vec[i].elsePC == pc && vec[i].elseIf == false)
			return true;
	}
	return false;
}

bool IfBlocks::isElseIf(PcAddr pc)
{
	for(int i=0; i < vec.size(); i++)
	{
		if (vec[i].elsePC == pc && vec[i].elseIf == true)
			return true;
	}
	return false;
}

void IfBlocks::setElseIf(PcAddr pc)
{
	for(int i=0; i < vec.size(); i++)
	{
		if (vec[i].elsePC == pc)
			 vec[i].elseIf = true;
	}
}

bool IfBlocks::isEnd(PcAddr pc)
{
	for(int i=0; i < vec.size(); i++)
	{
		if (vec[i].endPC == pc && vec[i].skipEnd == false)
			return true;
	}
	return false;
}

int IfBlocks::getNumEnds(PcAddr pc)
{
	int n = 0;
	for(int i=0; i < vec.size(); i++)
	{
		if (vec[i].endPC == pc && vec[i].skipEnd == false)
			n++;
	}

	return n;
}

IfBlocks::SingleIfBlock IfBlocks::getParentBlock(PcAddr pc)
{
	for(int i = vec.size()-1; i >= 0; i--)
	{
		if (vec[i].endPC > pc && vec[i].startPC < pc)
			return vec[i];
	}

	return SingleIfBlock();
}

bool IfBlocks::ifsBetween(PcAddr pc1, PcAddr pc2)
{
	for(int pc = pc1+1; pc < pc2; pc++)
	{
		if (isStart(pc))
			return true;
	}
	return false;
}

//void IfBlocks::invertCondition(string& str)
//{
//	size_t pos;
//	size_t startPos = 0, endPos;
//
//	do {
//		if ((endPos = str.find(" and ", startPos+3)) != string::npos)
//			str.replace(endPos,5," or ");
//		else if ((endPos = str.find(" or ", startPos+3)) != string::npos)
//		{}
//		else
//			endPos = str.length() - 1;
//
//		if ((pos = str.find("==", startPos)) != string::npos && pos < endPos)
//			str.replace(pos,2,"~=");
//		else if ((pos = str.find("~=", startPos)) != string::npos && pos < endPos)
//			str.replace(pos,2,"==");
//		else if ((pos = str.find("<", startPos)) != string::npos && pos < endPos)
//			str.replace(pos,1,">");
//		else if ((pos = str.find(">", startPos)) != string::npos && pos < endPos)
//			str.replace(pos,1,"<");
//		else if ((pos = str.find("<=", startPos)) != string::npos && pos < endPos)
//			str.replace(pos,2,">=");
//		else if ((pos = str.find(">=", startPos)) != string::npos && pos < endPos)
//			str.replace(pos,2,"<=");
//		else if ((pos = str.find("not ", startPos)) != string::npos && pos < endPos)
//			str.replace(pos,4,"");
//		else
//			str.replace(startPos,0,"not ");
//
//		if ((endPos = str.find(" and ", startPos+3)) != string::npos)
//		{
//			str.replace(endPos,5," or ");
//			startPos = endPos + 4;
//		}
//		else if ((endPos = str.find(" or ", startPos+3)) != string::npos)
//		{
//			startPos = endPos + 4;
//		}
//		else
//		{
//			endPos = str.length() - 1;
//			startPos = endPos;
//		}
//	
//	} while (startPos < str.length() - 1);
//}