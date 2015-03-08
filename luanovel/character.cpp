#include "stdafx.h"
#include "character.h"
#include "helper.h"

int character_initialize(lua_State* L)
{
	lua_pushliteral(L, "character");
	lua_newtable(L);
	lua_rawset(L, -3);
	return 0;
}
