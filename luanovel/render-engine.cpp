#include "stdafx.h"
#include "render-engine.h"
#include "helper.h"

#include <list>

typedef struct {
	Bitmap* b;
} bitmap_wrapper;

typedef struct {
	FT_Face f;
} font_wrapper;


// ------------------- LUA FUNCTIONS -------------------

// image.load(filename)
static int lua_image_load(lua_State* L) {
	const char* filename = lua_tostring(L, -1);
	lua_pop(L, 1);

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

// rendering.drawtext(lightuserdata, string, number, number, string)
static int lua_rendering_drawtext(lua_State* L)
{
	cairo_t* g = (cairo_t *)lua_touserdata(L, -5);
	const char* text = lua_tostring(L, -4);
	double x = lua_tonumber(L, -3);
	double y = lua_tonumber(L, -2);
	const char* fontstyle = lua_tostring(L, -1);
	lua_pop(L, 5);

	PangoFontMap* fontmap = pango_cairo_font_map_new_for_font_type(CAIRO_FONT_TYPE_FT);
	PangoContext* ctx = pango_font_map_create_context(fontmap);

	cairo_font_options_t* fo = cairo_font_options_create();
	cairo_font_options_set_antialias(fo, CAIRO_ANTIALIAS_SUBPIXEL);
	cairo_font_options_set_subpixel_order(fo, CAIRO_SUBPIXEL_ORDER_RGB);
	cairo_font_options_set_hint_style(fo, CAIRO_HINT_STYLE_SLIGHT);
	pango_cairo_context_set_font_options(ctx, fo);

	PangoLayout* layout = pango_layout_new(ctx);
	PangoFontDescription* desc = pango_font_description_from_string(fontstyle);
	pango_layout_set_font_description(layout, desc);
	pango_font_map_load_font(fontmap, ctx, desc);
	pango_font_description_free(desc);
	
	pango_layout_set_markup(layout, text, -1);
	cairo_set_source_rgb(g, 0, 0, 0);
	pango_cairo_update_layout(g, layout);
	cairo_move_to(g, x, y);
	pango_cairo_show_layout(g, layout);
	
	cairo_font_options_destroy(fo);
	g_object_unref(layout);
	g_object_unref(ctx);
	g_object_unref(fontmap);

	return 0;
}

int draw_initialize(lua_State* L)
{
	lua_checkstack(L, 5);

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
