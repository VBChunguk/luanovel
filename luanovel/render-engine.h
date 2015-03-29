#pragma once

FT_Library* InitializeFreeType();
void FinalizeFreeType();

int draw_initialize(lua_State* L);
static int draw_loadResource(lua_State* L);
static int draw_draw(lua_State* L);


