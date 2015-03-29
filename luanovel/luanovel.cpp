/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2015 VBChunguk
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * */

#include "stdafx.h"
#include "luanovel.h"
#include "core.h"
#include "text.h"
#include "render-engine.h"
#include "character.h"
#include "helper.h"

#define MAX_LOADSTRING 100

HINSTANCE hInst;
HWND ghWnd;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];
lua_State* L;

ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;

	L = luaL_newstate();
	if (L == NULL) {
		// Not enough memory
		return 1;
	}

	// Load lua libraries
	luaopen_base(L);
	luaopen_coroutine(L);
	luaopen_package(L);
	luaopen_utf8(L);
	luaopen_string(L);
	luaopen_math(L);

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_LUANOVEL, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Initialize luanovel library functions
	lua_checkstack(L, 5);

	lua_newtable(L);
	lua_pushliteral(L, "system");
	lua_newtable(L);
	helper_lua_addNewFunction(L, "message", [](lua_State* L) {
		const char* message = lua_tostring(L, -1);
		if (message == NULL) return 0;
		lua_pop(L, 1);
		MessageBoxA(ghWnd, message, NULL, MB_OK);
		return 0;
	});
	lua_pushliteral(L, "outputmode");
	lua_pushliteral(L, "stdout");
	lua_rawset(L, -3);
	lua_rawset(L, -3);

	lua_pushliteral(L, "text");
	lua_newtable(L);
	helper_lua_addNewFunction(L, "_interpret", &_lua_text_interpret);
	lua_rawset(L, -3);

	draw_initialize(L);
	character_initialize(L);

	lua_setglobal(L, "luanovel");

	luaL_dofile(L, "init.luac");
	luaL_dofile(L, "main.txt");

	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	lua_close(L);
	return (int) msg.wParam;
}



ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LUANOVEL));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance;

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   ghWnd = hWnd;

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	static cairo_surface_t* mainsurface = NULL;

	switch (message)
	{
	case WM_CREATE:
	{
		mainsurface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, 800, 600);
		break;
	}

	case WM_PAINT:
	{
		if (!mainsurface) break;

		lua_checkstack(L, 3);
		lua_getglobal(L, "luanovel");
		if (lua_isnil(L, -1)) {
			lua_pop(L, 1);
			break;
		}

		lua_getfield(L, -1, "rendering");
		lua_remove(L, -2); // luanovel
		lua_getfield(L, -1, "on_draw");
		lua_remove(L, -2); // rendering
		if (lua_isnil(L, -1)) {
			lua_pop(L, 1);
			break;
		}

		hdc = BeginPaint(hWnd, &ps);
		cairo_t* cr = cairo_create(mainsurface);
		cairo_set_source_rgb(cr, 1, 1, 1);
		cairo_paint(cr);
		lua_pushlightuserdata(L, cr);
		lua_pushliteral(L, "test");
		lua_pcall(L, 2, 0, 0);

		cairo_destroy(cr);

		cairo_surface_t* surface = cairo_win32_surface_create(hdc);
		cr = cairo_create(surface);
		cairo_set_source_surface(cr, mainsurface, 0, 0);
		cairo_paint(cr);
		cairo_destroy(cr);
		cairo_surface_destroy(surface);

		static bool test = false;
		
		if (!test) {
			cairo_surface_write_to_png(mainsurface, "test.png");
			test = true;
		}
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_DESTROY:
		cairo_surface_destroy(mainsurface);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
