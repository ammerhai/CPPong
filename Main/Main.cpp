// Main.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//
#include <Windows.h>
#include <stdio.h>

#define internal_function static
#define local_persist static
#define global_variable static

//global var (for now) 
//TODO:
global_variable bool Running;

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapContext;

internal_function void ResizeDIBSection(int Width, int Height) {
	
	//TODO: free DIBSection

	if (BitmapHandle) {
		DeleteObject(BitmapHandle);
	} 

	if(!BitmapContext){
		//TODO: change? -> new Monitor with another resolution
		BitmapContext = CreateCompatibleDC(0);
	}
	
	BITMAPINFO BitmapInfo;
	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = Width;
	BitmapInfo.bmiHeader.biHeight = Height;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 64;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;

	HBITMAP BitmapHandle = CreateDIBSection(BitmapContext, &BitmapInfo, DIB_RGB_COLORS, &BitmapMemory, 0, 0);
}

internal_function void UpdateWindow(HDC Context, int X, int Y, int Width, int Height) {
	StretchDIBits(Context, X, Y, Width, Height, X, Y, Width, Height, BitmapMemory, &BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
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
			UpdateWindow(Context, X, Y, Width, Height);
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