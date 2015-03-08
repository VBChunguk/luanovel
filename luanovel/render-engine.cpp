#include "stdafx.h"
#include "render-engine.h"
#include "helper.h"

typedef struct {
	Bitmap* b;
} bitmap_wrapper;

// image.load(filename)
static int lua_image_load(lua_State* L) {
	const char* filename = lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_checkstack(L, 3);
	lua_getglobal(L, "luanovel");
	lua_getfield(L, -1, "_imagetable");
	lua_remove(L, -2); // luanovel
	lua_pushstring(L, filename);
	lua_pushvalue(L, -1);
	lua_gettable(L, -1);
	if (!lua_isnil(L, -1)) { // if exists, then return it
		lua_remove(L, -2);
		lua_remove(L, -2);
		return 1;
	}
	lua_remove(L, -2);
	wchar_t* tmp = helper_utf8_to_utf16(filename);
	Bitmap* ret = Bitmap::FromFile(tmp);
	free(tmp);

	lua_checkstack(L, 5);
	bitmap_wrapper* b = (bitmap_wrapper*)lua_newuserdata(L, sizeof(bitmap_wrapper));
	lua_pushvalue(L, -1);
	lua_insert(L, 0); // idx 0 has bitmap_wrapper*
	lua_newtable(L);
	lua_pushliteral(L, "__gc");
	lua_pushcfunction(L, [](lua_State* L) {
		bitmap_wrapper* b = (bitmap_wrapper*)lua_touserdata(L, -1);
		lua_pop(L, 1);
		delete b->b;
		b->b = NULL;
		return 0;
	});
	lua_rawset(L, -3);
	lua_setmetatable(L, -2);
	b->b = ret;
	lua_rawset(L, -3);
	lua_pop(L, 1);
	// returns bitmap_wrapper*
	return 1;
}

// rendering.drawtext(string, number, number, userdata(font))
static int lua_rendering_drawtext(lua_State* L)
{
	return 0;
}

int draw_initialize(lua_State* L)
{
	lua_checkstack(L, 5);

	lua_pushliteral(L, "_imagetable");
	lua_newtable(L); // _imagetable
	lua_newtable(L); // (metatable)
	lua_pushliteral(L, "__mode");
	lua_pushliteral(L, "v");
	lua_rawset(L, -3); // (metatable)
	lua_setmetatable(L, -2);
	lua_rawset(L, -3); // _imagetable

	lua_pushliteral(L, "image");
	lua_newtable(L);
	helper_lua_addNewFunction(L, "load", &lua_image_load);
	lua_rawset(L, -3);

	lua_pushliteral(L, "rendering");
	lua_newtable(L);
	helper_lua_addNewFunction(L, "drawtext", &lua_rendering_drawtext);
	lua_rawset(L, -3);
	return 0;
}

static int draw_loadResource(lua_State* L)
{
	return 1;
}

static int draw_draw(lua_State* L)
{
	return 1;
}
