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
#define WINDOW_STYLE WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU

HINSTANCE hInst;
HWND ghWnd;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];
lua_State* L;
cairo_surface_t* mainsurface;

clock_t drawtime;
clock_t steptime;

ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
void	CALLBACK	OnStep(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

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
	luaL_requiref(L, "base", &luaopen_base, 0);
	luaL_requiref(L, "string", &luaopen_string, 1);
	luaL_requiref(L, "utf8", &luaopen_utf8, 1);
	luaL_requiref(L, "math", &luaopen_math, 1);

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_LUANOVEL, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Initialize luanovel library functions
	lua_checkstack(L, 5);

	lua_newtable(L);
	lua_pushliteral(L, "system");
	lua_newtable(L);
	helper_lua_addNewFunction(L, "message", [](lua_State* L) {
		const char* message = lua_tostring(L, 1);
		if (message == NULL) return 0;
		lua_pop(L, 1);
		MessageBoxA(ghWnd, message, NULL, MB_OK);
		return 0;
	});
	lua_pushliteral(L, "outputmode");
	lua_pushliteral(L, "stdout");
	lua_rawset(L, -3);
	lua_pushliteral(L, "locale");
	lua_pushliteral(L, "ko-KR");
	lua_rawset(L, -3);
	lua_rawset(L, -3);

	lua_pushliteral(L, "text");
	lua_newtable(L);
	lua_rawset(L, -3);

	draw_initialize(L);
	character_initialize(L);

	lua_pushliteral(L, "status"); // this object is saved in savefile
	lua_newtable(L);
	lua_rawset(L, -3);

	lua_pushliteral(L, "internal"); // for internal functions etc
	lua_newtable(L);
	lua_rawset(L, -3);

	lua_setglobal(L, "luanovel");

	luaL_dofile(L, "init.luac");
	int errorn = luaL_dofile(L, "main.txt");
	if (errorn && errorn != LUA_ERRFILE) {
		const char* message = lua_tostring(L, -1);
		if (message == NULL) return 0;
		lua_pop(L, 1);
		MessageBoxA(ghWnd, message, NULL, MB_OK);
		lua_close(L);
		return FALSE;
	}

	if (!InitInstance(hInstance, nCmdShow))
	{
		lua_close(L);
		return FALSE;
	}


	lua_checkstack(L, 2);
	lua_getglobal(L, "luanovel");
	helper_lua_getTableContent(L, "system.on_init");
	int width = 800, height = 600;
	if (lua_isfunction(L, -1)) {
		lua_pcall(L, 0, 0, 0);

		lua_getglobal(L, "luanovel");
		lua_pushvalue(L, -1);
		helper_lua_getTableContent(L, "rendering.width");
		if (lua_isnumber(L, -1)) width = (int)lua_tonumber(L, -1);
		lua_pop(L, 1);
		helper_lua_getTableContent(L, "rendering.height");
		if (lua_isnumber(L, -1)) height = (int)lua_tonumber(L, -1);
		lua_pop(L, 1);
	}

	mainsurface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);

	RECT rc = { 0, 0, width, height };
	AdjustWindowRect(&rc, WINDOW_STYLE, FALSE);
	SetWindowPos(ghWnd, NULL,
		-1, -1, rc.right - rc.left, rc.bottom - rc.top,
		SWP_NOMOVE | SWP_NOZORDER);

	SetTimer(ghWnd, 1, 16, &OnStep);
	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) break;
		}
		else {
			clock_t st = clock();
			lua_checkstack(L, 3);
			lua_getglobal(L, "luanovel");
			if (!lua_istable(L, -1)) {
				lua_pop(L, 1);
				continue;
			}

			helper_lua_getTableContent(L, "rendering.on_draw");
			if (!lua_isfunction(L, -1)) {
				lua_pop(L, 1);
				continue;
			}

			cairo_t* cr = cairo_create(mainsurface);
			cairo_set_source_rgb(cr, 1, 1, 1);
			cairo_paint(cr);
			lua_pushlightuserdata(L, cr);
			lua_pushliteral(L, "test"); // phase
			lua_pcall(L, 2, 0, 0);
			clock_t en = clock();
			drawtime = en - st;

			lua_getglobal(L, "luanovel");
			helper_lua_getTableContent(L, "internal.debug");
			lua_pushlightuserdata(L, cr);
			lua_pushinteger(L, steptime);
			lua_pushinteger(L, drawtime);
			lua_pcall(L, 3, 0, 0);
			cairo_destroy(cr);

			InvalidateRect(ghWnd, NULL, FALSE);
		}
	}
	KillTimer(ghWnd, 1);

	cairo_surface_destroy(mainsurface);

	draw_cleanup();
	lua_close(L);
	return (int) msg.wParam;
}

void CALLBACK OnStep(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if (idEvent == 1) {
		clock_t st = clock();
		lua_checkstack(L, 3);
		lua_getglobal(L, "luanovel");
		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			return;
		}
		helper_lua_getTableContent(L, "system.on_step");
		if (!lua_isfunction(L, -1)) {
			lua_pop(L, 1);
			return;
		}
		lua_pushliteral(L, "test"); // phase
		lua_pcall(L, 1, 0, 0);

		// internal step function
		lua_getglobal(L, "luanovel");
		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			return;
		}
		helper_lua_getTableContent(L, "internal.on_step");
		if (!lua_isfunction(L, -1)) {
			lua_pop(L, 1);
			return;
		}
		lua_pushliteral(L, "test"); // phase
		lua_pcall(L, 1, 0, 0);
		clock_t en = clock();
		steptime = en - st;
	}
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

   hWnd = CreateWindow(szWindowClass, szTitle, WINDOW_STYLE,
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
	HDC hdc;
	PAINTSTRUCT ps;

	switch (message)
	{
	case WM_SIZE:
	{
		break;
	}
	case WM_KEYDOWN:
	{
		if (wParam == VK_RETURN) {
			lua_checkstack(L, 2);
			lua_getglobal(L, "luanovel");
			if (!lua_istable(L, -1)) {
				lua_pop(L, 1);
				break;
			}
			helper_lua_getTableContent(L, "text.advance");
			if (!lua_isfunction(L, -1)) {
				lua_pop(L, 1);
				break;
			}
			lua_pcall(L, 0, 0, 0);
		}
		break;
	}
	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);
		cairo_surface_t* surface = cairo_win32_surface_create(hdc);
		cairo_t* cr = cairo_create(surface);
		cairo_set_source_surface(cr, mainsurface, 0, 0);
		cairo_paint(cr);
		cairo_destroy(cr);
		cairo_surface_destroy(surface);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
