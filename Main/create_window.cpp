#include "create_window.h"


//create Window
void* create_window(LRESULT (*WindowProc)(HWND, UINT, WPARAM, LPARAM)) {
	WNDCLASSEXW window_class_struct = {};
	window_class_struct.cbSize = sizeof(WNDCLASSEXW);
	window_class_struct.style = CS_CLASSDC | CS_HREDRAW | CS_VREDRAW;
	window_class_struct.lpfnWndProc = WindowProc;
	window_class_struct.cbClsExtra = 0;
	window_class_struct.cbWndExtra = 0;
	window_class_struct.hInstance = GetModuleHandle(NULL);
	window_class_struct.hIcon = 0;
	window_class_struct.hCursor = LoadCursor(NULL, IDC_ARROW);
	window_class_struct.hbrBackground = 0;
	window_class_struct.lpszMenuName = 0;
	window_class_struct.lpszClassName = L"DX_WINDOW_CLASS";
	window_class_struct.hIconSm = 0;

	ATOM window_class = RegisterClassExW(&window_class_struct);
	if (!window_class)
		return 0;

	return CreateWindowExW(0, L"DX_WINDOW_CLASS", L"DX", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, GetModuleHandle(NULL), 0);
}

/*WNDCLASS WindowClass = {};

	WindowClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
	WindowClass.lpfnWndProc = WindowMsgs;
	WindowClass.hInstance = GetModuleHandle(0);
	WindowClass.lpszClassName = "PongWindowClass";

	if (!RegisterClass(&WindowClass)) {
		printf("RegisterClass failed!\n");
		return 1;
	}

	HWND Window = CreateWindowEx(0, WindowClass.lpszClassName, "PONG", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, WindowClass.hInstance, 0 );
*/