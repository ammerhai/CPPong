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


#define log(format, ...) _log(__FUNCTION__, format, __VA_ARGS__)
void _log(const char* function_name, const char* format, ...) {
	printf("[%s] ", function_name);

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	printf("\n");
}

#define log_error(format, ...) _log_error(__FUNCTION__, format, __VA_ARGS__)
void _log_error(const char* function_name, const char* format, ...) {
	printf("[%s]ERROR: ", function_name);

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	printf("\n");
}

//Sound
struct Sound_Effect {
	int64 num_samples;
	float* samples;
};

Sound_Effect load_sound_effect(const char* path) {
	auto file = load_entire_file(path);
	auto file_start = file;

	if (!file.length) {
		log_error("'%s' Datei konnte nicht geladen werden", path);
		return { 0, 0 };
	}

	if (file.length < 36) {
		log_error("'%s' Datei ist zu klein", path);
		return { 0, 0 };
	}

	if (!starts_with(file, "RIFF")) {
		log_error("'%s' Keine Wave Datei - 'RIFF' erwartet, '%.*s' erhalten", path, min(file.length, 4), file.data);
		return { 0, 0 };
	}
	advance(file, 4);

	advance(file, 4);

	if (!starts_with(file, "WAVE")) {
		log_error("'%s' Keine Wave Datei - 'WAVE' erwartet, '%.*s' erhalten", path, min(file.length, 4), file.data);
		return { 0, 0 };
	}
	advance(file, 4);

	if (!starts_with(file, "fmt ")) {
		log_error("'%s' Keine Wave datei - 'fmt ' erwartet, '%.*s' erhalten", path, min(file.length, 4), file.data);
		return { 0, 0 };
	}
	advance(file, 4);

	if ((*(int32*)file.data) != 16) {
		log_error("'%s' fmt Chunklength korrumpiert '16' erwartet, '%.*s' erhalten", path, min(file.length, 4), file.data);
		return { 0, 0 };
	}
	advance(file, 4);

	auto format = *(WAVEFORMATEX*)file.data;
	advance(file, 16);

	if (!starts_with(file, "data")) {
		log_error("'%s' Keine Wave Datei - 'data' erwartet, '%.*s' erhalten", path, min(file.length, 4), file.data);
		return { 0, 0 };
	}
	advance(file, 4);

	auto data_length = *(int32*)file.data;
	advance(file, 4);

	if (!(format.nChannels == 1 || format.nChannels == 2)) {
		log_error("'%s' Nicht unterstützte Channelanzahl, '1' oder '2' erwartet, '%hd' erhalten", path, format.nChannels);
		return { 0, 0 };
	}

	if (format.wFormatTag != 1) {
		log_error("'%s' Falscher Formattag, '1' erwartet, '%hd' erhalten (siehe Google)", path, format.wFormatTag);
		return { 0, 0 };
	}

	if (format.nSamplesPerSec != 48000) {
		log_error("'%s' Falsche SamplesProSekunde, '48000' erwartet, '%d' erhalten", path, format.nSamplesPerSec);
		return { 0, 0 };
	}

	if (format.wBitsPerSample != 16) {
		log_error("'%s' Falsche BitsProSample '16' erwartet, '%hd' erhalten", path, format.wBitsPerSample);
		return { 0, 0 };
	}

	auto expected_block_align = format.wBitsPerSample / 8 * format.nChannels;

	if (format.nBlockAlign != expected_block_align) {
		log_error("'%s' Falscher BlockAlign '%d' erwartet, '%hd' erhalten", path, expected_block_align, format.nBlockAlign);
		return { 0, 0 };
	}

	int64 num_samples = data_length / (format.wBitsPerSample / 8);
	if (format.nChannels == 2)
		num_samples /= 2;

	float* samples = (float*)malloc(num_samples * sizeof(float) * 2);

	if (!samples) {
		log_error("'%s' malloc failed", path);
		return { 0, 0 };
	}

	if (format.nChannels == 1) {
		for (int i = 0; i < num_samples; i++) {
			samples[i * 2 + 0] = (*(int16*)file.data) / 32767.0f;
			samples[i * 2 + 1] = (*(int16*)file.data) / 32767.0f;
			advance(file, 2);
		}
	} else {
		for (int i = 0; i < num_samples; i++) {
			samples[i * 2 + 0] = (*(int16*)file.data) / 32767.0f;
			advance(file, 2);
			samples[i * 2 + 1] = (*(int16*)file.data) / 32767.0f;
			advance(file, 2);
		}
	}

	free(file_start.data);
	return { num_samples, samples };
}


Sound_Effect se_player_pong_1 = load_sound_effect("../Assets/Player_Pong_1.wav");
Sound_Effect se_wall_pong_1 = load_sound_effect("../Assets/Wall_Pong_1.wav");
Sound_Effect se_pong_game = load_sound_effect("../Assets/stereo_tests.wav");

struct playing_sound {
	Sound_Effect* sound_effect;
	int64 current_sample;
};


#define concurrently_playable_sounds 16
playing_sound currently_playing_sounds[concurrently_playable_sounds];

void play_sound(Sound_Effect* sound_effect) {
	for (int i = 0; i < concurrently_playable_sounds; i++) {
		if (!currently_playing_sounds[i].sound_effect) {
			currently_playing_sounds[i].sound_effect = sound_effect;
			currently_playing_sounds[i].current_sample = 0;
			return;
		}
	}

	log_error("Keine weiteren Soundeffekte abspielbar");
	return;
}


struct PixelColor {
	uint8 b;
	uint8 g;
	uint8 r;
	uint8 a;
};



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



void init_samples() {
	play_sound(&se_pong_game);
#if 0

	float frequency = 220.0f;
	size_t samples_per_wavelength = samples_per_second / frequency;

	for (int i = 0; i < samples_to_play_num; i++) {
		auto current_sample_in_wave = i % samples_per_wavelength;

		if(current_sample_in_wave < samples_per_wavelength / 2)
			samples_to_play[i] = (((float)current_sample_in_wave / (float)samples_per_wavelength) * 4.0f - 1.0f);
		else
			samples_to_play[i] = (1.0f - ((float)(current_sample_in_wave - samples_per_wavelength / 2) / (float)samples_per_wavelength) * 4.0f);
	}
#endif
}

int SoundThread(void* arg) {
	init_samples();

	typedef HRESULT direct_sound_create(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN  pUnkOuter);

	HMODULE DSound = LoadLibraryA("dsound.dll");

	if (!DSound) {
		printf("LoadLibrary dsound.dll failed");
		return 0;
	}

	direct_sound_create* DSoundCreate = (direct_sound_create* )GetProcAddress(DSound, "DirectSoundCreate");
	if (!DSoundCreate) {
		printf("GetProcAddress of DirectSoundCreate failed");
		return 0;
	}

	//WICHTIG
	LPDIRECTSOUND direct_sound;

	if (DSoundCreate(0, &direct_sound, 0) != DS_OK) {
		printf("DSoundCreate failed");
		return 0;
	}

	if (direct_sound->SetCooperativeLevel(GetDesktopWindow(), DSSCL_PRIORITY) != DS_OK) {
		printf("SetCooperationLevel failed");
		return 0;
	}

	DSBUFFERDESC primary_buffer_description = {};
	primary_buffer_description.dwSize = sizeof(DSBUFFERDESC);
	primary_buffer_description.dwFlags = DSBCAPS_PRIMARYBUFFER;

	LPDIRECTSOUNDBUFFER primary_buffer;

	if (direct_sound->CreateSoundBuffer(&primary_buffer_description, &primary_buffer, 0) != DS_OK) {
		printf("CreatePrimaryBuffer failed");
		return 0;
	}

	WAVEFORMATEX format = {};
	format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
	format.nChannels = 2;
	format.nSamplesPerSec = 48000;
	format.wBitsPerSample = 32;
	format.nBlockAlign = (format.nChannels * format.wBitsPerSample) / 8;
	format.nAvgBytesPerSec = format.nBlockAlign * format.nSamplesPerSec;

	auto bytes_per_sample = (format.wBitsPerSample / 8) * format.nChannels;

	if (primary_buffer->SetFormat(&format) != DS_OK) {
		printf("SetFormat failed");
		return 0;
	}

	DSBUFFERDESC sound_buffer_description = {};
	sound_buffer_description.dwSize = sizeof(DSBUFFERDESC);
	sound_buffer_description.dwBufferBytes = (format.nSamplesPerSec / 20) * bytes_per_sample;
	sound_buffer_description.dwFlags = DSBCAPS_GLOBALFOCUS;
	sound_buffer_description.lpwfxFormat = &format;

	LPDIRECTSOUNDBUFFER sound_buffer;

	if (direct_sound->CreateSoundBuffer(&sound_buffer_description, &sound_buffer, 0) != DS_OK) {
		printf("CreateSoundBuffer failed");
		return 0;
	}

	if (sound_buffer->Play(0, 0, DSBPLAY_LOOPING) != DS_OK) {
		printf("play sound_buffer failed");
		return 0;
	}

	float* region_1 = 0;
	DWORD region_1_size = 0;
	float* region_2 = 0;
	DWORD region_2_size = 0;

	DWORD current_play_position;
	DWORD current_write_position;

	int64 last_written_sample = 0;
	int64 samples_to_write = 0;

	int64 buffer_samples = sound_buffer_description.dwBufferBytes / bytes_per_sample;

	while (true) {
		sound_buffer->GetCurrentPosition(&current_play_position, &current_write_position);

		/*if (last_written_sample == current_play_position) {
			Sleep(1);
			continue;
		}

		if (last_written_sample < current_play_position)
			samples_to_write = min((current_play_position - last_written_sample) / (bytes_per_sample * 1), (long long)(samples_to_play_num - current_sample));
		else 
			samples_to_write = min((current_play_position + sound_buffer_description.dwBufferBytes - last_written_sample) / (bytes_per_sample * 1), (long long)(samples_to_play_num - current_sample));

		if(!SUCCEEDED(sound_buffer->Lock()*/


		auto current_play_samples = current_play_position / bytes_per_sample;

		if (current_play_samples >= last_written_sample)
			samples_to_write = current_play_samples - last_written_sample;
		else
			samples_to_write = buffer_samples - last_written_sample + current_play_samples;

		if (samples_to_write == 0)
			continue;

		last_written_sample = (last_written_sample + samples_to_write) % buffer_samples;

		if (sound_buffer->Lock(last_written_sample * bytes_per_sample, samples_to_write * bytes_per_sample, (void**)&region_1, &region_1_size, (void **)&region_2, &region_2_size, 0) != DS_OK) {
			printf("lock sound_buffer failed");
			return 0;
		}

		auto region_1_samples = region_1_size / sizeof(float) / 2;
		auto region_2_samples = region_2_size / sizeof(float) / 2;

		for (int i = 0; i < region_1_samples; i++) {
			float playing_sound_effects_left = 0;
			float playing_sound_effects_right = 0;

			for (int j = 0; j < concurrently_playable_sounds; j++) {
				if (!currently_playing_sounds[j].sound_effect) 
					continue;

				if (!currently_playing_sounds[j].sound_effect->num_samples)
					continue;

				playing_sound_effects_left += currently_playing_sounds[j].sound_effect->samples[currently_playing_sounds[j].current_sample * 2 + 0];
				playing_sound_effects_right += currently_playing_sounds[j].sound_effect->samples[currently_playing_sounds[j].current_sample * 2 + 1];
				currently_playing_sounds[j].current_sample++;
				if (currently_playing_sounds[j].current_sample == currently_playing_sounds[j].sound_effect->num_samples) {
					currently_playing_sounds[j].sound_effect = 0;
				}
			}
			region_1[i * 2 + 0] = playing_sound_effects_left;			
			region_1[i * 2 + 1] = playing_sound_effects_right;
		}

		for (int i = 0; i < region_2_samples; i++) {
			float playing_sound_effects_left = 0;
			float playing_sound_effects_right = 0;

			for (int j = 0; j < concurrently_playable_sounds; j++) {
				if (!currently_playing_sounds[j].sound_effect)
					continue;

				if (!currently_playing_sounds[j].sound_effect->num_samples)
					continue;

				playing_sound_effects_left += currently_playing_sounds[j].sound_effect->samples[currently_playing_sounds[j].current_sample * 2 + 0];
				playing_sound_effects_right += currently_playing_sounds[j].sound_effect->samples[currently_playing_sounds[j].current_sample * 2 + 1];
				currently_playing_sounds[j].current_sample++;
				if (currently_playing_sounds[j].current_sample == currently_playing_sounds[j].sound_effect->num_samples) {
					currently_playing_sounds[j].sound_effect = 0;
				}
			}
			region_2[i * 2 + 0] = playing_sound_effects_left;
			region_2[i * 2 + 1] = playing_sound_effects_right;
		}



		if (sound_buffer->Unlock(region_1, region_1_size, region_2, region_2_size) != DS_OK) {
			printf("unlock sound_buffer failed");
			return 0;
		}
	}

	return 0;
}

int main() {
	//MessageBoxA(0, "Hello World", "Hi", MB_OKCANCEL | MB_ICONERROR); //"0x11" (macro) as bitflags is possible too
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&SoundThread, 0, 0, 0);

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