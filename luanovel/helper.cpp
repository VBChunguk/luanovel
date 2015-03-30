#include "stdafx.h"
#include "helper.h"

void helper_lua_addNewFunction(lua_State* L, const char* fname, lua_CFunction func)
{
	lua_checkstack(L, 2);
	lua_pushlstring(L, fname, strlen(fname));
	lua_pushcfunction(L, func);
	lua_rawset(L, -3);
}

wchar_t* helper_utf8_to_utf16(const char* utf8)
{
	wchar_t* ret = NULL;
#ifdef _WIN32
	int size;
	size = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	if (size == 0) {
		int a = GetLastError();
		a = a;
	}
	ret = (wchar_t*)malloc(sizeof(wchar_t) * (size + 1));
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, ret, size);
#else
	// not implemented
#endif
	return ret;
}
