#pragma once

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("1942");
HWND hWndMain;

#define WINSIZEX 600
#define WINSIZEY 800