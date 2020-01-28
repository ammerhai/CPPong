// Main.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//
#include "math_graphics.h"
#include <Windows.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "create_window.h"
#include "time.h"

#undef min
#undef max

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

struct PixelColor {
	uint8 b;
	uint8 g;
	uint8 r;
	uint8 a;
};


void renderQuad(V2 pos, V2 halfsize, V3 color) {
	V2 topleft = pos - halfsize;
	V2 bottomright = pos + halfsize;
	
	topleft = clamp01(topleft);
	bottomright = clamp01(bottomright);

	M2x2 projection = {
		BitmapWidth, 0.0f,
		0.0f, BitmapHeight
	};

	topleft = projection * topleft;
	bottomright = projection * bottomright;
	for (int y = topleft.y; y < bottomright.y; y++) {
		for (int x = topleft.x; x < bottomright.x; x++) {
			PixelColor* pixel = (PixelColor*)BitmapMemory + x + y * BitmapWidth;
			pixel->b = 255 * color.b;
			pixel->g = 255 * color.g;
			pixel->r = 255 * color.r;
			pixel->a = 0xFF;
		}
	}
}

//function to learn rendering
internal_function void RenderGradient(int XOffset, int YOffset) {

	int Width = BitmapWidth;
	int Height = BitmapHeight;
	int Pitch = Width * BytesPerPixel;
	uint8 *Row = (uint8 *)BitmapMemory;

	for (int Y = 0; Y < BitmapHeight; Y++) {
		PixelColor *pixel = (PixelColor *)Row;

		for (int X = 0; X < BitmapWidth; X++) {
			/*
									Pixel+0 Pixel+1 Pixel+2 Pixel+3
				Pixel in Memory:	00 00 00 00
									BB GG RR xx
								->  0x xxRRGGBB
			*/

			pixel->b = (uint8)(X + XOffset) ^ (uint8)(Y + YOffset);
			pixel->g = (uint8)(Y + YOffset);
			pixel->r = 0.5 * ((uint8)(Y + YOffset) + (uint8)(X + XOffset));

			pixel->a = 0xFF;
			pixel++;
		}

		Row += Pitch;
	}

}

//Spieler 1 Attribute
V2 pos_p1 = { 0.1, 0.5 };
V2 size_p1 = {0.01, 0.07};
V3 color_p1 = { 0.9, 1, 0.5 };

//Spieler 2 Attribute
V2 pos_p2 = { 0.9, 0.5 };
V2 size_p2 = { 0.01, 0.07 };
V3 color_p2 = { 0.5, 1, 0.9 };

//Ball Attribute
V2 pos_ball = { 0.5, 0.5 };
V2 size_ball = { 0.01, 0.01 };
V3 color_ball = {1, 0, 0};
float speed_ball = 0.4;
V2 direction_ball = normalize(V2{1, 0.5});

//Gameplay Attribute
float player_speed = 0.5;
V2 border = { 0.01, 0.01 } ;
bool key_down[256];
double last_time = get_time();

struct Minkowski_sum {
	V2 pos;
	V2 size;
	
	Minkowski_sum(V2 pos, V2 s1, V2 s2) : pos(pos) { size = s1 + s2; };

	V2 top_left() {
		return pos - size;
	}

	V2 bottom_right() {
		return pos + size;
	}

	V2 top_right() {
		return { pos.x + size.x, pos.y - size.y };
	}

	V2 bottom_left() {
		return { pos.x - size.x, pos.y + size.y };
	}
};

float Minkowski_intersects_ray(Minkowski_sum sum, V2 origin, V2 dir) {
	auto reciprocal_dir = 1.0f / dir;

	auto t1 = hadamard((sum.bottom_left() - origin), reciprocal_dir);
	auto t2 = hadamard((sum.top_right() - origin), reciprocal_dir);

	float tmin = max(min(t1, t2));
	float tmax = min(max(t1, t2));

	if (tmax < 0.0f)
		return 0.0f;

	if (tmin > tmax)
		return 0.0f;

	return tmin;
}

void UpdatePong() {

#define left(a) (pos_##a .x - size_##a .x)
#define right(a) (pos_##a .x + size_##a .x)
#define top(a) (pos_##a .y - size_##a .y)
#define bottom(a) (pos_##a .y + size_##a .y)

	double current_time = get_time();
	double delta_time = current_time - last_time;

	//Steuerung Spieler 1
	if (key_down['W']) {
		pos_p1.y = pos_p1.y - player_speed * delta_time;
		pos_p1 = clamp01(pos_p1 - size_p1 - border) + size_p1 + border;
	}
	if (key_down['S']) {
		pos_p1.y = pos_p1.y + player_speed * delta_time;
		pos_p1 = clamp01(pos_p1 + size_p1 + border) - size_p1 - border;
	}

	//Steuerung Spieler 2
	if (key_down[VK_UP]) {
		pos_p2.y = pos_p2.y - player_speed * delta_time;
		pos_p2 = clamp01(pos_p2 - size_p2 - border) + size_p2 + border;
	}
	if (key_down[VK_DOWN]) {
		pos_p2.y = pos_p2.y + player_speed * delta_time;
		pos_p2 = clamp01(pos_p2 + size_p2 + border) - size_p2 - border;
	}

	auto p1 = Minkowski_sum(pos_p1, size_p1, size_ball);
	auto p2 = Minkowski_sum(pos_p2, size_p2, size_ball);

	//Berührung Rand oben
	if (top(ball) < 0) {
		direction_ball.y = fabs(direction_ball.y);
	}

	//Berührung Rand unten
	if (bottom(ball) > 1) {
		direction_ball.y = -fabs(direction_ball.y);
	}

	float distance = 0.0f;
	//Berührung Spieler 1
	if((distance = Minkowski_intersects_ray(p1, pos_ball, direction_ball)) != 0.0f)
		if(distance < speed_ball * delta_time)
			direction_ball.x = fabs(direction_ball.x);
	
	//Berührung Spieler 2
	if ((distance = Minkowski_intersects_ray(p2, pos_ball, direction_ball)) != 0.0f)
		if (distance < speed_ball * delta_time)
			direction_ball.x = -fabs(direction_ball.x);

	//Bewegung des Balles
	pos_ball = pos_ball + direction_ball * speed_ball * delta_time;


	color_ball.r = sin(fmod(current_time, 6.28318530718));

	last_time = current_time;
}

void RenderPong() {
	UpdatePong();

	float ratio = (float)BitmapWidth / (float)BitmapHeight;
	size_ball.y = size_ball.x * ratio;


	memset(BitmapMemory, 0, BitmapWidth * BitmapHeight * BytesPerPixel);
	renderQuad(pos_p1, size_p1, color_p1);
	renderQuad(pos_p2, size_p2, color_p2);
	renderQuad(pos_ball, size_ball, color_ball);

}


internal_function void ResizeDIBSection(int Width, int Height) {
	
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

internal_function void Win32UpdateWindow(HDC Context, HWND Window, int X, int Y) {

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	int WindowWidth = ClientRect.right - ClientRect.left ;
	int WindowHeight = ClientRect.bottom - ClientRect.top;
	StretchDIBits(Context, 0, 0, WindowWidth, WindowHeight, 0, 0, BitmapWidth, BitmapHeight, BitmapMemory, &BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
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

		
		case WM_KEYDOWN: {
			if (!(lParam & (1 << 30))) {
				key_down[wParam] = true;
			}
		} break;

		case WM_KEYUP: {
			key_down[wParam] = false;
		} break;

		case WM_PAINT: {
			PAINTSTRUCT Paint;
			BeginPaint(Window, &Paint);
			EndPaint(Window, &Paint);

			/*PAINTSTRUCT Paint;
			HDC Context = BeginPaint(Window, &Paint);
			int X = Paint.rcPaint.left;
			int Y = Paint.rcPaint.top;
			int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
			int Width = Paint.rcPaint.right - Paint.rcPaint.left;

			RECT ClientRect;
			GetClientRect(Window, &ClientRect);
			Win32UpdateWindow(Context, &ClientRect, X, Y, Width, Height);
			EndPaint(Window, &Paint);*/
			
		} break;

		default: {
			Result = DefWindowProc(Window, Message, wParam, lParam);
		} break;

	}
	return Result;
}


int main() {
	//MessageBoxA(0, "Hello World", "Hi", MB_OKCANCEL | MB_ICONERROR); //"0x11" (macro) as bitflags is possible too

	auto Window = (HWND)create_window(WindowMsgs);
	if (!Window) {
		printf("CreateWindow failed!\n");
		return 1;
	}
	
	MSG Message;
	Running = true;

	int XOffset = 0;
	int YOffset = 0;
	
	HDC Context = GetDC(Window);

	while(Running) {

		MSG Message;
		
		while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {

			if (Message.message == WM_QUIT) {
				Running = false;
			}
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		//RenderGradient(XOffset, YOffset);
		RenderPong();

		Win32UpdateWindow(Context, Window, 0, 0);
		
		XOffset++;
	}

	return 0;
}