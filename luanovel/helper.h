#pragma once

void helper_lua_addNewFunction(lua_State* L, const char* name, lua_CFunction func);
void helper_lua_getTableContent(lua_State* L, const char* name);

// Please free() the return value after use.
wchar_t* helper_utf8_to_utf16(const char* utf8);