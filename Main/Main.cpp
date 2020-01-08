// Main.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//
#include <Windows.h>
#include <stdio.h>
#include <stdint.h>

#define internal_function static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

//global var (for now) 
//TODO:
global_variable bool Running;

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;

internal_function void RenderGradient(int XOffset, int YOffset) {

	int Width = BitmapWidth;
	int Height = BitmapHeight;
	int Pitch = Width * BytesPerPixel;
	uint8 *Row = (uint8 *)BitmapMemory;

	for (int Y = 0; Y < BitmapHeight; Y++) {
		uint8 *Pixel = (uint8 *)Row;

		for (int X = 0; X < BitmapWidth; X++) {
			/*
									Pixel+0 Pixel+1 Pixel+2 Pixel+3
				Pixel in Memory:	00 00 00 00
									BB GG RR xx
								->  0x xxRRGGBB
			*/

			*Pixel = (uint8)(X + XOffset) ^ (uint8)(Y + YOffset);
			Pixel++;

			*Pixel = (uint8)(Y + YOffset);
			Pixel++;

			*Pixel = (uint8)(Y + YOffset) + ((uint8)(X + XOffset));
			Pixel++;

			*Pixel = 0xFF;
			Pixel++;

		}

		Row += Pitch;
	}
}


internal_function void ResizeDIBSection(int Width, int Height) {
	
	//TODO: free DIBSection
	if (BitmapMemory) {
		VirtualFree(BitmapMemory, 0, MEM_RELEASE);
	}

	BitmapWidth = Width;
	BitmapHeight = Height;

	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = BitmapWidth;
	BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;

	//int BytesPerPixel = 4;
	int BitmapMemorySize = (BitmapWidth * BitmapHeight) * BytesPerPixel;
	BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

	//RenderGradient(128, 128);
}

internal_function void Win32UpdateWindow(HDC Context, RECT *ClientRect, int X, int Y, int Width, int Height) {

	int WindowWidth = ClientRect->right - ClientRect->left ;
	int WindowHeight = ClientRect ->bottom - ClientRect->top;
	StretchDIBits(Context, /*X, Y, Width, Height, X, Y, Width, Height, */ 0, 0, WindowWidth, WindowHeight, 0, 0, BitmapWidth, BitmapHeight, BitmapMemory, &BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
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

			RECT ClientRect;
			GetClientRect(Window, &ClientRect);
			Win32UpdateWindow(Context, &ClientRect, X, Y, Width, Height);
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

	HWND Window = CreateWindowEx(0, WindowClass.lpszClassName, "PONG", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, WindowClass.hInstance, 0 );

	if (!Window) {
		printf("CreateWindow failed!\n");
		return 1;
	}
	
	MSG Message;
	Running = true;

	int XOffset = 0;
	int YOffset = 0;
	
	while(Running) {

		MSG Message;
		
		while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {

			if (Message.message == WM_QUIT) {
				Running = false;
			}
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		RenderGradient(XOffset, YOffset);

		
		HDC Context = GetDC(Window) ;
		RECT ClientRect;
		GetClientRect(Window, &ClientRect);
		int WindowWidth = ClientRect.right - ClientRect.left;
		int WindowHeight = ClientRect.bottom - ClientRect.top;
		Win32UpdateWindow(Context, &ClientRect, 0, 0, WindowWidth, WindowHeight);
		ReleaseDC(Window, Context);
		
		XOffset++;
	}



	return 0;
}