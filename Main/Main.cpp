// Main.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//
#include <Windows.h>
#include <stdio.h>

#define internal_function static
#define local_persist static
#define global_variable static

//global var (for now)
global_variable bool Running;

internal_function void ResizeDIBSection(int Width, int Height) {
	HBITMAP CreateDIBSection();
}

internal_function void UpdateWindow(HWND Window, int X, int Y, int Width, int Height) {
	int StrechDIBBits();
}

LRESULT WindowMsgs(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam) {
	LRESULT Result = 0;

	switch (Message) {
		case WM_SIZE: {
			RECT ClientRect; 
			GetClientRect(Window, &ClientRect);
			int Height = ClientRect.bottom - ClientRect.top;
			int Width = ClientRect.right - ClientRect.left;
			ResizeDIBSection(Width, Height);
			printf("size\n");
		} break;

		case WM_DESTROY: {
			Running = false;
			printf("destroy\n");
		} break;

		case WM_CLOSE: {
			Running = false;
			printf("close\n");
		} break;

		case WM_ACTIVATEAPP: {
			printf("app\n");
		} break;

		case WM_PAINT: {
			PAINTSTRUCT Paint;
			HDC Context = BeginPaint(Window, &Paint);
			int X = Paint.rcPaint.left;
			int Y = Paint.rcPaint.top;
			int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
			int Width = Paint.rcPaint.right - Paint.rcPaint.left;
			UpdateWindow(Window, X, Y, Width, Height);
			EndPaint(Window, &Paint);
			
		} break;

		default: {
			Result = DefWindowProc(Window, Message, wParam, lParam);
		} break;

	}
	return Result;
}


int main() {
	//MessageBoxA(0, "Hello World", "Hi", MB_OKCANCEL | MB_ICONERROR); //"0x11" (macro) as bitflags is possible too
	WNDCLASS WindowClass = {};
	
	WindowClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
	WindowClass.lpfnWndProc = WindowMsgs;
	WindowClass.hInstance = GetModuleHandle(0);
	WindowClass.lpszClassName = "PongWindowClass";
	
	if (!RegisterClass(&WindowClass)) {
		printf("RegisterClass failed!\n");
		return 1;
	}

	HWND WindowHandle = CreateWindowEx(0, WindowClass.lpszClassName, "PONG", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, WindowClass.hInstance, 0 );

	if (!WindowHandle) {
		printf("CreateWindow failed!\n");
		return 1;
	}
	
	MSG Message;
	Running = true;

	while(Running) {
		BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
		if (MessageResult <= 0) {
			break;
		}
		TranslateMessage(&Message);
		DispatchMessage(&Message);

	}


	return 0;
}