#include "stdafx.h"
#include "render-engine.h"
#include "helper.h"
#include "resource-manager.h"

#include <list>

typedef struct {
	cairo_surface_t* b;
} bitmap_wrapper;

typedef bitmap_wrapper* imageobj_t;

PangoFontMap* pango_fontmap;
PangoContext* pango_ctx;

// ------------------- LUA FUNCTIONS -------------------

// imageobj:dispose()
// also run by __gc
static int lua_image_obj_dispose(lua_State* L)
{
	imageobj_t b = (imageobj_t)lua_touserdata(L, -1);
	lua_pop(L, 1);
	if (!(b->b)) return 0;
	cairo_surface_destroy(b->b);
	b->b = NULL;
	return 0;
}

// image.load(filename)
static int lua_image_load(lua_State* L)
{
	const char* filename = lua_tostring(L, -1);
	lua_pop(L, 1);

	cairo_surface_t* ret = cairo_image_surface_create_from_png(filename);
	if (!ret) {
		lua_pushnil(L);
		return 1;
	}

	lua_checkstack(L, 4);
	imageobj_t b = (imageobj_t)lua_newuserdata(L, sizeof(bitmap_wrapper));
	lua_newtable(L);
	lua_pushliteral(L, "__gc");
	lua_pushcfunction(L, &lua_image_obj_dispose);
	lua_rawset(L, -3);
	lua_pushliteral(L, "index");
	lua_newtable(L);
	helper_lua_addNewFunction(L, "dispose", &lua_image_obj_dispose);
	lua_rawset(L, -3);
	lua_setmetatable(L, -2);

	b->b = ret;
	// returns imageobj_t
	return 1;
}

// image.loadptr(filename)
static int lua_image_loadptr(lua_State* L)
{
	lua_getfield(L, -1, "locale");
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		lua_getglobal(L, "luanovel");
		helper_lua_getTableContent(L, "system.locale");
	}
	const char* locale = lua_tostring(L, -1);
	lua_getfield(L, -2, "pointer");
	luanovel_pointer_t pointer = (luanovel_pointer_t)lua_tointeger(L, -1);

	resource_open(locale);
	resource_request_t req = { locale, pointer };
	void* closure = resource_make_closure(&req);
	cairo_surface_t* ret = cairo_image_surface_create_from_png_stream(resource_cairo_read, closure);
	resource_closure_destroy(closure);
	lua_pop(L, 3);

	if (!ret) {
		lua_pushnil(L);
		return 1;
	}

	lua_checkstack(L, 4);
	imageobj_t b = (imageobj_t)lua_newuserdata(L, sizeof(bitmap_wrapper));
	lua_newtable(L);
	lua_pushliteral(L, "__gc");
	lua_pushcfunction(L, &lua_image_obj_dispose);
	lua_rawset(L, -3);
	lua_pushliteral(L, "index");
	lua_newtable(L);
	helper_lua_addNewFunction(L, "dispose", &lua_image_obj_dispose);
	lua_rawset(L, -3);
	lua_setmetatable(L, -2);

	b->b = ret;
	// returns imageobj_t
	return 1;
}

// rendering.drawtext(cairo_t*, string, number, number, string)
static int lua_rendering_drawtext(lua_State* L)
{
	cairo_t* g = (cairo_t *)lua_touserdata(L, -5);
	const char* text = lua_tostring(L, -4);
	double x = lua_tonumber(L, -3);
	double y = lua_tonumber(L, -2);
	const char* fontstyle = lua_tostring(L, -1);
	lua_pop(L, 5);

	PangoLayout* layout = pango_layout_new(pango_ctx);
	PangoFontDescription* desc = pango_font_description_from_string(fontstyle);
	pango_layout_set_font_description(layout, desc);
	pango_font_map_load_font(pango_fontmap, pango_ctx, desc);
	pango_font_description_free(desc);
	
	pango_layout_set_markup(layout, text, -1);
	cairo_set_source_rgb(g, 0, 0, 0);
	pango_cairo_update_layout(g, layout);
	cairo_move_to(g, x, y);
	pango_cairo_show_layout(g, layout);
	
	g_object_unref(layout);
	return 0;
}

// rendering.drawimage(cairo_t*, imageobj, x, y, scalex, scaley)
static int lua_rendering_drawimage(lua_State* L)
{
	cairo_t* cr = (cairo_t *)lua_touserdata(L, -6);
	imageobj_t b = (imageobj_t)lua_touserdata(L, -5);
	double x = lua_tonumber(L, -4);
	double y = lua_tonumber(L, -3);
	double scalex = lua_tonumber(L, -2);
	double scaley = lua_tonumber(L, -1);
	lua_pop(L, 6);

	if (!(b->b)) {
		return 0;
	}

	cairo_save(cr);
	cairo_translate(cr, x, y);
	cairo_scale(cr, scalex, scaley);
	cairo_set_source_surface(cr, b->b, 0, 0);
	cairo_paint(cr);
	cairo_restore(cr);

	return 0;
}

int draw_initialize(lua_State* L)
{
	lua_checkstack(L, 5);

	lua_pushliteral(L, "image");
	lua_newtable(L);
	helper_lua_addNewFunction(L, "load", &lua_image_load);
	helper_lua_addNewFunction(L, "loadptr", &lua_image_loadptr);
	lua_rawset(L, -3);

	lua_pushliteral(L, "rendering");
	lua_newtable(L);
	helper_lua_addNewFunction(L, "drawtext", &lua_rendering_drawtext);
	helper_lua_addNewFunction(L, "drawimage", &lua_rendering_drawimage);
	lua_rawset(L, -3);

	pango_fontmap = pango_cairo_font_map_new_for_font_type(CAIRO_FONT_TYPE_FT);
	pango_ctx = pango_font_map_create_context(pango_fontmap);

	cairo_font_options_t* fo = cairo_font_options_create();
	cairo_font_options_set_antialias(fo, CAIRO_ANTIALIAS_SUBPIXEL);
	cairo_font_options_set_subpixel_order(fo, CAIRO_SUBPIXEL_ORDER_RGB);
	cairo_font_options_set_hint_style(fo, CAIRO_HINT_STYLE_SLIGHT);
	pango_cairo_context_set_font_options(pango_ctx, fo);
	cairo_font_options_destroy(fo);

	return 0;
}

int draw_cleanup()
{
	g_object_unref(pango_ctx);
	g_object_unref(pango_fontmap);
	return 0;
}
