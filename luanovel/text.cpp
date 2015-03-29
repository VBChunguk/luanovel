#include "stdafx.h"
#include "text.h"

// text._interpret(str)
int _lua_text_interpret(lua_State* L)
{
	const char* line = lua_tostring(L, -1);
	lua_pop(L, 1);
	int len = strlen(line);
	bool escaping = false;

	std::string output;
	for (const char* i = line; *i != 0; i++) {
		if (escaping) {
			output += *i;
			escaping = false;
			continue;
		}
		if (*i == '\\') {
			escaping = true;
		}
		else if (*i == '\x03') { // Lua inline expression
			const char* exp = ++i;
			const char* texp = exp;
			for (; *texp != '\x03'; texp++, i++) {
				if (*texp == 0) {
					lua_pushliteral(L, "Unexpected end of Lua inline expression");
					return lua_error(L);
				}
			}
			std::string ex(exp, texp);
			lua_checkstack(L, 1);
			lua_getglobal(L, "tostring");
			char* expression = (char *)malloc(ex.size() + 10);
			strcpy(expression, "return (");
			strcat(expression, ex.c_str());
			strcat(expression, ")");
			luaL_dostring(L, expression);
			free(expression);
			lua_pcall(L, 1, 1, 0);

			output += lua_tostring(L, -1);
			lua_pop(L, 1);
		}
		else {
			output += *i;
		}
	}
	lua_pushstring(L, output.c_str());
	return 1;
}
