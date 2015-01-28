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
#include "render-engine.h"
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
	luaopen_base(L);
	luaopen_coroutine(L);
	luaopen_package(L);
	luaopen_utf8(L);
	luaopen_string(L);
	luaopen_math(L);

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_LUANOVEL, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	lua_checkstack(L, 5);

	lua_newtable(L);
	lua_pushliteral(L, "system");
	lua_newtable(L);
	helper_lua_addNewFunction(L, "message", [](lua_State* L) {
		const char* message = lua_tostring(L, -1);
		if (message == NULL) return 0;
		MessageBoxA(ghWnd, message, NULL, MB_OK);
		return 1;
	});
	lua_rawset(L, -3);

	lua_pushliteral(L, "text");
	lua_newtable(L);
	helper_lua_addNewFunction(L, "print", [](lua_State* L) {
		const char* line = lua_tostring(L, -1);
		lua_pop(L, 1);
		int len = strlen(line);
		void *character = lua_touserdata(L, -1);
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
			else if (*i == '\x03') {// Lua inline expression
				char* exp = strdup(++i);
				char* texp = exp;
				for (; *texp != '\x03'; texp++, i++) {
					if (*texp == 0) {
						lua_pushliteral(L, "Unexpected end of Lua inline expression");
						free(exp);
						return lua_error(L);
					}
				}
				*texp = 0;
				lua_checkstack(L, 1);
				lua_getglobal(L, "tostring");
				char* expression = (char *)malloc(strlen(exp) + 10);
				strcpy(expression, "return (");
				strcat(expression, exp);
				strcat(expression, ")");
				luaL_dostring(L, expression);
				free(exp);
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
	});
	lua_rawset(L, -3);
	lua_setglobal(L, "luanovel");

	luaL_dofile(L, "init.txt");
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

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
