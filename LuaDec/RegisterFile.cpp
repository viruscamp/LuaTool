#include "RegisterFile.h"

extern "C"{
#include "lopcodes.h"
}

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sstream>
using namespace std;

void RegisterFile::build(Proto* f)
{
	// all register are automatically initialized to "nil" value

	// get constants
	for (int i=0; i < f->sizek; i++)
	{
		constants[i].value = DecriptConstant(f, i);
		constants[i].isConstant = true;
	}
}

Register& RegisterFile::operator [](int index)
{
	if (ISK(index)) // is this a constant?
		return constants[INDEXK(index)]; // switch to constant indexing
	else
		return registers[index]; // normal indexing
}

string RegisterFile::getBxConstant(int bx)
{
	return constants[bx].value;
}

string RegisterFile::getGlobal(int bx)
{
	string str = constants[bx].value;
	str.replace(0,1,"");
	str.replace(str.length()-1, 1, "");

	return str;
}

bool RegisterFile::anyTempRegister()
{
	map<int, Register>::const_iterator find;
	for (find = registers.begin(); find != registers.end(); find++)
	{
		if (find->second.isTemp)
			return true;
	}

	return false;
}

string RegisterFile::DecriptConstant(Proto* f, int n)
{
	string str;
    const TValue* tv = &f->k[n];
    switch (ttype(tv)) {
  		case LUA_TBOOLEAN: // Lua5.1 specific
			if (tv->value.b)
				str = "true";
			else
				str = "false";
			break;
		case LUA_TNUMBER:
			char ret[255];
			sprintf(ret, LUA_NUMBER_FMT, nvalue(tv));
			str.assign(ret);
			break;
		case LUA_TSTRING:
			str = DecriptString(f, n);
			break;
		case LUA_TNIL:
			str = "nil";
			break;
		default:                   /* cannot happen */
			str = "Uknown_Type_Error";
			break;
	}//end switch

	return str;
}

// PrintString from luac is not 8-bit clean
string RegisterFile::DecriptString(Proto* f, int n)
{
	const char *s = svalue(&f->k[n]);
    int len = (&(&f->k[n])->value.gc->ts)->tsv.len;
    char *ret = (char*)malloc(strlen(s) * 4 + 3);
	int p = 0;
    ret[p++] = '"';
    for (int i = 0; i < len; i++, s++) {
        switch (*s) {
        case '"':
            ret[p++] = '\\';
            ret[p++] = '"';
            break;
        case '\a':
            ret[p++] = '\\';
            ret[p++] = 'a';
            break;
        case '\b':
            ret[p++] = '\\';
            ret[p++] = 'b';
            break;
        case '\f':
            ret[p++] = '\\';
            ret[p++] = 'f';
            break;
        case '\n':
            ret[p++] = '\\';
            ret[p++] = 'n';
            break;
        case '\r':
            ret[p++] = '\\';
            ret[p++] = 'r';
            break;
        case '\t':
            ret[p++] = '\\';
            ret[p++] = 't';
            break;
        case '\v':
            ret[p++] = '\\';
            ret[p++] = 'v';
            break;
        case '\\':
            ret[p++] = '\\';
            ret[p++] = '\\';
            break;
        default:
            if (*s < 32 || *s > 127) {
               char* pos = &(ret[p]);
			   unsigned char c = *s;
               sprintf(pos, "\\%d", c);
               p += strlen(pos);
            } else {
               ret[p++] = *s;
            }
            break;
        }//end weitch
    }//end for

    ret[p++] = '"';
    ret[p] = '\0';

	string str(ret);
	free(ret);
	
    return str;
}