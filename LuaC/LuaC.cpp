#include "LuaC.h"
const string LuaC::version = "1.2";

extern "C" {
#include "lua.h"
#include "lauxlib.h"

#include "ldo.h"
#include "lfunc.h"
#include "lmem.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstring.h"
#include "lundump.h"
}

#include <cstdio>
#include <iostream>
using namespace std;

LuaC::LuaC(string iFN)
: inputName(iFN) {}

void LuaC::compile(string outputFileName, bool strip)
{
	outputName = outputFileName;

	// create new lua state
	lua_State* L = lua_open();
 
	if (L==NULL) {
		fatal("FATAL ERROR - Not enough memory for lua state."); return; }
	
	// load file from disk
	if (luaL_loadfile(L,inputName.c_str()) != 0) {
			fatal(lua_tostring(L,-1)); return; }

	// get function proto
	const Proto* f;
	const Closure* c=(const Closure*)lua_topointer(L,-1);
	f = c->l.p;

	// write compiled luac to disk
	FILE* D = (outputName.c_str() == NULL) ? stdout : fopen(outputName.c_str(), "wb");
	if (D==NULL) {
		cannot("open"); return; }
	
	lua_lock(L);
	luaU_dump(L,f,writer,D,strip);
	lua_unlock(L);
	
	if (ferror(D)) {
		cannot("write"); return; }
	if (fclose(D)) {
		cannot("close"); return; }

	// close lua state
	lua_close(L);
}

void LuaC::compileAndReplace(string destLuacName, int funcNumber, bool strip)
{
	outputName = destLuacName;

	// create two lua states for the input file and destination file
	lua_State* destL = lua_open();
	lua_State* inputL = lua_open();

	if (destL == NULL || inputL == NULL) {
		fatal("FATAL ERROR - Not enough memory for lua state."); return; }

	// load files from disk 
	// luaL_loadfile doesn't care if the source file is compiled or not
	if (luaL_loadfile(destL, destLuacName.c_str()) != 0) {
		fatal(lua_tostring(destL,-1)); return; }
	if (luaL_loadfile(inputL, inputName.c_str()) != 0) {
		fatal(lua_tostring(inputL,-1)); return; }

	// create and get functon protos
	Proto* destf, *inputf, *tempf;
	const Closure* destc=(const Closure*)lua_topointer(destL,-1);
	destf = destc->l.p;
	const Closure* inputc=(const Closure*)lua_topointer(inputL,-1);
	inputf = inputc->l.p;

	// task sanity checks
	if (funcNumber > destf->sizep)
		{ fatal("Invalid function number."); return; }
	if (inputf->sizep < 1)
		{ fatal("The input file doesn't contain any functions."); return; }

	// replace the desired function with input
	// tempf saves the pointer to the original function
	tempf = destf->p[funcNumber-1];
	destf->p[funcNumber-1] = inputf->p[0];

	// write the destionaton luac back to disk with the replaced function
	FILE* D = (destLuacName.c_str() == NULL) ? stdout : fopen(destLuacName.c_str(), "wb");
	if (D==NULL) {
		cannot("open");
		return;
	}
	
	lua_lock(destL);
	luaU_dump(destL,destf,writer,D,strip);
	lua_unlock(destL);
	
	if (ferror(D)) {
		cannot("write"); return; }
	if (fclose(D)) {
		cannot("close"); return; }

	// restore original state, so memory can be cleared properly
	destf->p[funcNumber-1] = tempf;

	lua_close(destL);
	lua_close(inputL);
}

int LuaC::writer(lua_State* L, const void* p, size_t size, void* u)
{
	UNUSED(L);
	return (fwrite(p,size,1,(FILE*)u)!=1) && (size!=0);
}

void LuaC::fatal(const char* msg)
{
	errors.set("LuaC: " + string(msg));
}

void LuaC::cannot(const char* what)
{
	errors.set("LuaC: cannot " + string(what) +" "+ outputName +": "+ string(strerror(errno)));
}