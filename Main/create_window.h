#pragma once
#include <Windows.h>
void* create_window(LRESULT(*WindowProc)(HWND, UINT, WPARAM, LPARAM));