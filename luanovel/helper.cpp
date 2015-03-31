#include "stdafx.h"
#include "helper.h"

void helper_lua_addNewFunction(lua_State* L, const char* fname, lua_CFunction func)
{
	lua_checkstack(L, 2);
	lua_pushlstring(L, fname, strlen(fname));
	lua_pushcfunction(L, func);
	lua_rawset(L, -3);
}

void helper_lua_getTableContent(lua_State* L, const char* name)
{
	char* tname = (char*)malloc(strlen(name) + 1);
	char* oname = tname;
	const char* p = tname;
	strcpy(tname, name);

	lua_checkstack(L, 2);
	for (; *tname != 0; tname++)
	{
		if (*tname == '.') {
			*tname = 0;
			lua_getfield(L, -1, p);
			lua_remove(L, -2);
			if (lua_isnil(L, -1)) {
				free(tname);
				return;
			}
			p = tname + 1;
		}
	}
	lua_getfield(L, -1, p);
	lua_remove(L, -2);
	free(oname);
}

wchar_t* helper_utf8_to_utf16(const char* utf8)
{
	wchar_t* ret = NULL;
#ifdef _WIN32
	int size;
	size = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	if (size == 0) return NULL;
	ret = (wchar_t*)malloc(sizeof(wchar_t) * (size + 1));
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, ret, size);
#else
	// not implemented
#endif
	return ret;
}
