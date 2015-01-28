#include "stdafx.h"
#include "helper.h"

void helper_lua_addNewFunction(lua_State* L, const char* fname, lua_CFunction func)
{
	lua_pushlstring(L, fname, strlen(fname));
	lua_pushcfunction(L, func);
	lua_rawset(L, -3);
}
