#define _CRT_SECURE_NO_WARNINGS
// Main.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//
#include "math_graphics.h"
#include <Windows.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "create_window.h"
#include "time.h"
#include <emmintrin.h>
#include <immintrin.h>
#include <dsound.h>
#include <mmreg.h>
#include "sb_string.h"
#include "load_entire_file.h"
#include "log.h"
#include "pong_sound.h"

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

Sound_Effect se_player_pong_1 = load_sound_effect("../Assets/Player_Pong_1.wav");
Sound_Effect se_wall_pong_1 = load_sound_effect("../Assets/Wall_Pong_1.wav");
Sound_Effect se_pong_game = load_sound_effect("../Assets/pong_game.wav");


//Spieler 1 Attribute
V2 p1_pos = { 0.1, 0.5 };
V2 p1_size = {0.01, 0.07};
V4 p1_color = { 0.9, 1, 0.5 };
int p1_points = 0;

//Spieler 2 Attribute
V2 p2_pos = { 0.9, 0.5 };
V2 p2_size = { 0.01, 0.07 };
V4 p2_color = { 0.5, 1, 0.9 };
int p2_points = 0;

//Ball Attribute
V2 ball_pos = { 0.5, 0.5 };
V2 ball_size = { 0.01, 0.01 };
V4 ball_color = {1, 0, 0};
float ball_speed = 0.5;
V2 ball_direction = normalize(V2{-1, 0.9});

//Gameplay Attribute
float player_speed = 0.5;
V2 border = { 0.01, 0.01 } ;
bool key_down[256];
double last_time = get_time();

#define renderQuad_times_num 1024
double renderQuad_times[renderQuad_times_num];
size_t renderQuad_times_current;

//Rendering der einzelnen Quadrate mit Farbgebung
void renderQuad(V2 pos, V2 halfsize, V4 color) {
	//auto start_time = get_time();

	V2 topleft = pos - halfsize;
	V2 bottomright = pos + halfsize;

	topleft = clamp01(topleft);
	bottomright = clamp01(bottomright);

	M2x2 projection = {
		(float)BitmapWidth, 0.0f,
		0.0f, (float)BitmapHeight
	};

	topleft = projection * topleft;
	bottomright = projection * bottomright;

	auto topleft_x = (int)topleft.x;
	auto topleft_y = (int)topleft.y;
	auto bottomright_x = (int)bottomright.x;
	auto bottomright_y = (int)bottomright.y;

	int color_int = (((int)(color.a * 255.0f)) << 24) | (((int)(color.r * 255.0f)) << 16) | (((int)(color.g * 255.0f)) << 8) | (((int)(color.b * 255.0f)) << 0);
	
	auto color_4x = _mm_set_epi32(color_int, color_int, color_int, color_int);

	for (int y = topleft_y; y < bottomright_y; y++) {
		auto x_width = bottomright_x - topleft_x;
		auto to_fill_4x = x_width / 4;
		for (int x = 0; x < to_fill_4x; x++) {
			_mm_storeu_si32((int*)BitmapMemory + topleft_x + x + y * BitmapWidth, color_4x);
		}

		for (int x = topleft_x + to_fill_4x; x < bottomright_x; x++) {
			auto p = (int *)BitmapMemory + x + y * BitmapWidth;
			*p = color_int;
		}
	}

	/*auto end_time = get_time();
	renderQuad_times[renderQuad_times_current] = end_time - start_time;
	renderQuad_times_current = (renderQuad_times_current + 1) % renderQuad_times_num;
	*/
}


struct Quad {
	V2 pos;
	V2 size;	
};

Quad numbers[10][5] = {
	{
		{ { 0.2, 0.1 }, { 0.04, 0.01 } },
		{ { 0.17, 0.2 }, { 0.01, 0.1 } },
		{ { 0.23, 0.2 }, { 0.01, 0.1 } },
		{ { 0.2, 0.3 }, { 0.04, 0.01 } },
	},
	{
		{ { 0.23, 0.2 }, { 0.01, 0.1 } }
	},
	{
		{ { 0.2, 0.1 }, { 0.04, 0.01 } },
		{ {0.23, 0.15 }, { 0.01, 0.05 } },
		{ { 0.2, 0.2 }, { 0.04, 0.01 } },
		{ { 0.17, 0.25 }, { 0.01, 0.05 } },
		{ { 0.2, 0.3 }, { 0.04, 0.01 } }
	},
	{
		{ { 0.2, 0.1 }, { 0.04, 0.01 } },
		{ { 0.23, 0.15 }, { 0.01, 0.05 } },
		{ { 0.2, 0.2 }, { 0.04, 0.01 } },
		{ { 0.23, 0.25 }, { 0.01, 0.05 } },
		{ { 0.2, 0.3 }, { 0.04, 0.01 } }
	},
	{
		{ { 0.23, 0.2 }, { 0.01, 0.1 } },
		{ { 0.2, 0.2 }, { 0.04, 0.01 } },
		{ { 0.17, 0.15 }, { 0.01, 0.05 } },
	},
	{
		{ { 0.2, 0.1 }, { 0.04, 0.01 } },
		{ { 0.17, 0.15 }, { 0.01, 0.05 } },
		{ { 0.2, 0.2 }, { 0.04, 0.01 } },
		{ { 0.23, 0.25 }, { 0.01, 0.05 } },
		{ { 0.2, 0.3 }, { 0.04, 0.01 } }
	},
	{
		{ { 0.17, 0.2 }, { 0.01, 0.1 } },
		{ { 0.2, 0.2 }, { 0.04, 0.01 } },
		{ { 0.23, 0.25 }, { 0.01, 0.05 } },
		{ { 0.2, 0.3 }, { 0.04, 0.01 } }
	},
	{
		{ { 0.2, 0.1 }, { 0.04, 0.01 } },
		{ { 0.23, 0.2 }, { 0.01, 0.1 } }
	},
	{
		{ { 0.2, 0.1 }, { 0.04, 0.01 } },
		{ { 0.17, 0.2 }, { 0.01, 0.1 } },
		{ { 0.2, 0.2 }, { 0.04, 0.01 } },
		{ { 0.23, 0.2 }, { 0.01, 0.1 } },
		{ { 0.2, 0.3 }, { 0.04, 0.01 } }
	},
	{
		{ { 0.2, 0.1 }, { 0.04, 0.01 } },
		{ { 0.17, 0.15 }, { 0.01, 0.05 } },
		{ { 0.2, 0.2 }, { 0.04, 0.01 } },
		{ { 0.23, 0.2 }, { 0.01, 0.1 } }
	},
};


void render_number(V2 off, int number) {
	assert(number < 10);

	for (int i = 0; i < 5; i++) {
		auto quad = numbers[number][i];
		renderQuad(quad.pos + off, quad.size, {1,1,1});
	}
}

void render_point(int p1_points, int p2_points) {
	render_number({}, p1_points);
	render_number({.6}, p2_points);
}


void render_middle_line() {
	for (float a = 0.02f; a <= 1;) {
		renderQuad({ 0.5, a }, { 0.001, 0.01 }, { 1, 1, 1 });
		a = a + 0.08;
	}
}

void Reset_Ball() {
	ball_pos = { 0.5, 0.5 };
}

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

V2 move_ball(V2 ball_pos, V2& ball_direction, float distance_to_travel) {
	if (distance_to_travel == 0)
		return ball_pos;

	auto p1 = Minkowski_sum(p1_pos, p1_size, ball_size);
	auto p2 = Minkowski_sum(p2_pos, p2_size, ball_size);
	auto wall_top = Minkowski_sum({ 0.5, 0 }, { 1, 0 }, ball_size);
	auto wall_bottom = Minkowski_sum({ 0.5, 1 }, { 1, 0 }, ball_size);

	float distance_top = clamp(0, Minkowski_intersects_ray(wall_top, ball_pos, ball_direction), INFINITY);
	float distance_bottom = clamp(0, Minkowski_intersects_ray(wall_bottom, ball_pos, ball_direction), INFINITY);
	float distance_p1 = clamp(0, Minkowski_intersects_ray(p1, ball_pos, ball_direction), INFINITY);
	float distance_p2 = clamp(0, Minkowski_intersects_ray(p2, ball_pos, ball_direction), INFINITY);

	distance_top = distance_top == 0.0f ? INFINITY : distance_top;
	distance_bottom = distance_bottom == 0.0f ? INFINITY : distance_bottom;
	distance_p1 = distance_p1 == 0.0f ? INFINITY : distance_p1;
	distance_p2 = distance_p2 == 0.0f ? INFINITY : distance_p2;

	float distance_min = min(min(distance_p1, distance_p2), min(distance_top, distance_bottom));

	if (distance_min >= distance_to_travel)
		return ball_pos + ball_direction * distance_to_travel;

	ball_pos = ball_pos + ball_direction * distance_min;

	if (distance_min == distance_p1) {
		ball_direction.x = fabs(ball_direction.x);
		play_sound(&se_player_pong_1);
	}
	if (distance_min == distance_p2) {
		ball_direction.x = -fabs(ball_direction.x);
		play_sound(&se_player_pong_1);
	}
	if (distance_min == distance_top) {
		ball_direction.y = fabs(ball_direction.y);
		play_sound(&se_wall_pong_1);
	}
	if (distance_min == distance_bottom) {
		ball_direction.y = -fabs(ball_direction.y);
		play_sound(&se_wall_pong_1);
	}
	return move_ball(ball_pos, ball_direction, distance_to_travel - distance_min);
}

void UpdatePong() {

#define left(a) (a ##_pos.x - a ##_size.x)
#define right(a) (a ##_pos.x + a ##_size.x)
#define top(a) (a ##_pos.y - a ##_size.y)
#define bottom(a) (a ##_pos.y + a ##_size.y)

	double current_time = get_time();
	double delta_time = current_time - last_time;

	//Steuerung Spieler 1
	if (key_down['W']) {
		p1_pos.y = p1_pos.y - player_speed * delta_time;
		p1_pos = clamp01(p1_pos - p1_size - border) + p1_size + border;
	}
	if (key_down['S']) {
		p1_pos.y = p1_pos.y + player_speed * delta_time;
		p1_pos = clamp01(p1_pos + p1_size + border) - p1_size - border;
	}

	//Steuerung Spieler 2
	if (key_down[VK_UP]) {
		p2_pos.y = p2_pos.y - player_speed * delta_time;
		p2_pos = clamp01(p2_pos - p2_size - border) + p2_size + border;
	}
	if (key_down[VK_DOWN]) {
		p2_pos.y = p2_pos.y + player_speed * delta_time;
		p2_pos = clamp01(p2_pos + p2_size + border) - p2_size - border;
	}

	float distance_traveled = ball_speed * delta_time;

	ball_pos = move_ball(ball_pos, ball_direction, distance_traveled);

	//Punkte
	if (right(ball) > 1 && p1_points < 9) {
		p1_points = p1_points + 1;
		Reset_Ball();
	}
	if (left(ball) < 0 && p2_points < 9) {
		p2_points = p2_points + 1;
		Reset_Ball();
	}


	//Farbänderung, TODO: nach v1.0 -> Items, wird aktiviert
	ball_color.r = sin(fmod(current_time, 6.28318530718));

	//printf("frame time: %.5g fps: %.3g\n", (current_time - last_time), 1.0 / (current_time - last_time));

	last_time = current_time;
}

void RenderPong() {
	UpdatePong();

	float ratio = (float)BitmapWidth / (float)BitmapHeight;
	ball_size.y = ball_size.x * ratio;

	memset(BitmapMemory, 0, BitmapWidth * BitmapHeight * BytesPerPixel);
	render_middle_line();
	render_point(p1_points, p2_points);
	renderQuad(p1_pos, p1_size, p1_color);
	renderQuad(p2_pos, p2_size, p2_color);
	renderQuad(ball_pos, ball_size, ball_color);
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
	init_sound();
	play_sound(&se_pong_game);

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

		/*double min_time = INFINITY;
		double max_time = 0.0;
		double avg_time = 0.0;

		if (renderQuad_times_current == 0) {
			for (auto t : renderQuad_times) {
				min_time = min(min_time, t * 1000.0);
				max_time = max(max_time, t * 1000.0);
				avg_time += (t * 1000.0) / (double)renderQuad_times_num;
			}
			printf("renderQuad_times: %f %f %f\n", min_time, avg_time, max_time);
		}
		*/
		Win32UpdateWindow(Context, Window, 0, 0);
		
		XOffset++;
	}

	return 0;
}