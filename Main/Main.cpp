#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
// Main.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//
#include "math_graphics.h"
#include <d3d11.h>
#include <D3DCompiler.h>
#include <Windows.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "create_window.h"
#include "time.h"
#include <emmintrin.h>
#include <immintrin.h>
#include <xmmintrin.h>
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

IDXGISwapChain* swap_chain;
ID3D11Device* device;
ID3D11DeviceContext* devicecontext;
ID3D11DepthStencilState* depth_stencil_state;

uint32 render_width = 1920;
uint32 render_height = 1080;

Sound_Effect se_player_pong_1 = load_sound_effect("../Assets/Player_Pong_1.wav");
Sound_Effect se_wall_pong_1 = load_sound_effect("../Assets/Wall_Pong_1.wav");
Sound_Effect se_wall_pong_2 = load_sound_effect("../Assets/Wall_Pong_2.wav");
Sound_Effect se_pong_game = load_sound_effect("../Assets/pong_game.wav");

//Spieler 1 Attribute
V2 p1_pos = { 0.1, 0.5 };
V2 p1_size = {0.01, 0.07};
V4 p1_color = { 0.9, 1, 0.5, 1 };
//V4 p1_color = { 1, 0, 0, 1 };
int p1_points = 0;

//Spieler 2 Attribute
V2 p2_pos = { 0.9, 0.5 };
V2 p2_size = { 0.01, 0.07 };
V4 p2_color = { 0.5, 1, 0.9, 1 };
int p2_points = 0;

//Ball Attribute
V2 ball_pos = { 0.5, 0.5 };
V2 ball_size = { 0.005, 0.005 };
V4 ball_color = {1, 0, 0, 1};
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

struct Pixel {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
};

#pragma pack 1
struct back_buffer_Pixel {
	uint8_t blue;
	uint8_t green;
	uint8_t red;
	uint8_t alpha;
};

//Rendering der einzelnen Quadrate mit Farbgebung

//TODO HARDWARE Rendering
//TODO 92 - 95: Scaling

auto blue_mask_4x  = _mm_set1_epi32(0xFF);
auto green_mask_4x = _mm_set1_epi32(0xFF00);
auto red_mask_4x   = _mm_set1_epi32(0xFF0000);

auto new_mask_4x = _mm_set1_epi32(0xFF);

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

	auto back_buffer = (back_buffer_Pixel*)BitmapMemory;
	auto y_height = bottomright_y - topleft_y;

	for (int y = 0; y < y_height; y++) {
		auto x_width = bottomright_x - topleft_x;
		for (int x = 0; x < (x_width / 4) * 4; x += 4) {
			auto old_colors = _mm_loadu_si128((__m128i *)&back_buffer[(topleft_y + y) * BitmapWidth + topleft_x + x]);

			auto old_blue  = old_colors & blue_mask_4x;
			auto old_green = old_colors & green_mask_4x;
			auto old_red   = old_colors & red_mask_4x;

			auto shift_green = old_green >> 8;
			auto shift_red	 = old_red >> 16;

			auto old_blue_f32  = _mm_cvtepi32_ps(old_blue);
			auto old_green_f32 = _mm_cvtepi32_ps(shift_green);
			auto old_red_f32   = _mm_cvtepi32_ps(shift_red);

			old_blue_f32 = old_blue_f32 / 255.0f;
			old_green_f32 = old_green_f32 / 255.0f;
			old_red_f32   = old_red_f32 / 255.0f;

			old_blue_f32  = old_blue_f32 * old_blue_f32;
			old_green_f32 = old_green_f32 * old_green_f32;
			old_red_f32   = old_red_f32 * old_red_f32;

			auto colorb_4x = _mm_set1_ps(color.a * color.b);
			auto colorg_4x = _mm_set1_ps(color.a * color.g);
			auto colorr_4x = _mm_set1_ps(color.a * color.r);

			auto new_blue  = lerp(old_blue_f32, color.a, color.b);
			auto new_green = lerp(old_green_f32, color.a, color.g);
			auto new_red   = lerp(old_red_f32, color.a, color.r);

			new_blue  = square_root(new_blue);
			new_green = square_root(new_green);
			new_red   = square_root(new_red);

			new_blue  = new_blue * 255.0f;
			new_green = new_green * 255.0f;
			new_red   = new_red * 255.0f;

			auto new_blue_int  = _mm_cvtps_epi32(new_blue);
			auto new_green_int = _mm_cvtps_epi32(new_green);
			auto new_red_int   = _mm_cvtps_epi32(new_red);
			
			new_blue_int  = new_blue_int & new_mask_4x;
			new_green_int = new_green_int & new_mask_4x;
			new_red_int   = new_red_int & new_mask_4x;

			shift_green = new_green_int << 8;
			shift_red   = new_red_int << 16;

			auto new_colors = new_blue_int | shift_green | shift_red;
			_mm_storeu_si128((__m128i *)&back_buffer[(topleft_y + y) * BitmapWidth + topleft_x + x], new_colors);
		}

		for (int _x = 0; _x < x_width % 4; _x++) {
			auto x = (x_width / 4) * 4 + _x;

			auto old_red = back_buffer[(topleft_y + y) * BitmapWidth + topleft_x + x].red;
			auto old_green = back_buffer[(topleft_y + y) * BitmapWidth + topleft_x + x].green;
			auto old_blue = back_buffer[(topleft_y + y) * BitmapWidth + topleft_x + x].blue;

			auto old_red_f32 = old_red / 255.0f;
			auto old_green_f32 = old_green / 255.0f;
			auto old_blue_f32 = old_blue / 255.0f;

			//sRGB -> RGB
			old_red_f32 = old_red_f32 * old_red_f32;
			old_green_f32 = old_green_f32 * old_green_f32;
			old_blue_f32 = old_blue_f32 * old_blue_f32;

			auto new_red = lerp(old_red_f32, color.a, color.r);
			auto new_green = lerp(old_green_f32, color.a, color.g);
			auto new_blue = lerp(old_blue_f32, color.a, color.b);

			new_red = square_root(new_red);
			new_green = square_root(new_green);
			new_blue = square_root(new_blue);

			new_red = new_red * 255.0f;
			new_green = new_green * 255.0f;
			new_blue = new_blue * 255.0f;

			back_buffer[(topleft_y + y) * BitmapWidth + topleft_x + x].red = new_red;
			back_buffer[(topleft_y + y) * BitmapWidth + topleft_x + x].green = new_green;
			back_buffer[(topleft_y + y) * BitmapWidth + topleft_x + x].blue = new_blue;
		}
	}


	/*auto end_time = get_time();
	renderQuad_times[renderQuad_times_current] = end_time - start_time;
	renderQuad_times_current = (renderQuad_times_current + 1) % renderQuad_times_num;
	*/
}

void renderQuad_old(V2 pos, V2 halfsize, V4 color) {
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

	auto back_buffer = (back_buffer_Pixel *)BitmapMemory;
	auto y_height = bottomright_y - topleft_y ;	

	for (int y = 0; y < y_height; y++) {
		auto x_width = bottomright_x - topleft_x;
		for (int x = 0; x < x_width; x++) {
			auto old_red = back_buffer[(topleft_y + y) * BitmapWidth + topleft_x + x].red;
			auto old_green = back_buffer[(topleft_y + y) * BitmapWidth + topleft_x + x].green;
			auto old_blue = back_buffer[(topleft_y + y) * BitmapWidth + topleft_x + x].blue;

			auto old_red_f32 = old_red / 255.0f;
			auto old_green_f32 = old_green / 255.0f;
			auto old_blue_f32 = old_blue / 255.0f;

			//sRGB -> RGB
			old_red_f32 = old_red_f32 * old_red_f32;
			old_green_f32 = old_green_f32 * old_green_f32;
			old_blue_f32 = old_blue_f32 * old_blue_f32;

			auto new_red = lerp(old_red_f32, color.a, color.r);
			auto new_green = lerp(old_green_f32, color.a, color.g);
			auto new_blue = lerp(old_blue_f32, color.a, color.b);

			new_red = square_root(new_red);
			new_green = square_root(new_green);
			new_blue = square_root(new_blue);

			new_red = new_red * 255.0f;
			new_green = new_green * 255.0f;
			new_blue = new_blue * 255.0f;

			back_buffer[(topleft_y + y) * BitmapWidth + topleft_x + x].red = new_red;
			back_buffer[(topleft_y + y) * BitmapWidth + topleft_x + x].green = new_green;
			back_buffer[(topleft_y + y) * BitmapWidth + topleft_x + x].blue = new_blue;
		}
	}


	/*auto end_time = get_time();
	renderQuad_times[renderQuad_times_current] = end_time - start_time;
	renderQuad_times_current = (renderQuad_times_current + 1) % renderQuad_times_num;
	*/
}

struct BMP_Texture {
	int64 bmp_width;
	int64 bmp_height;
	uint32* pixel;
};

#define print_value(v) _Generic((v), \
	int8_t: printf("%hhd", v), \
	int16_t: printf("%hd", v), \
	int32_t: printf("%d", v),  \
	int64_t: printf("%lld", v), \
	long: printf("%lld", v), \
	\
	uint8_t: printf("%hhu", v), \
	uint16_t: printf("%hu", v), \
	uint32_t: printf("%u", v),  \
	uint64_t: printf("%llu", v), \
	unsigned long: printf("%llu", v) \
)

template <typename T>
T read(string &file){
	if(file.length < sizeof(T))
		return {};
	T value = *(T*)file.data;
	advance(file, sizeof(T));
	return value;
}

bool _expect_read(string& file, string expected_value, const char* path, const char* error_msg) {
	if (file.length < expected_value.length)
		return false;
	if (!(string{ expected_value.length, file.data } == expected_value)) {
		log_error("'%s' %s expected '%.*s' got '%.*s'\n", path, error_msg, expected_value.length, expected_value.data, expected_value.length, file.data);
		return false;
	}
	advance(file, expected_value.length);
	return true;
}

template<typename T>
bool _expect_read(string& file, T expected_value, const char* path, const char* error_msg) {
	if (file.length < sizeof(T))
		return false;
	if (*(T*)file.data != expected_value) {
		printf("'%s' %s expected '", path, error_msg);
		print_value(expected_value);
		printf("' got '");
		print_value(*(T*)file.data);
		printf("'\n");
		return false;
	}

	advance(file, sizeof(T));
	return true;
}

template<typename T, typename S>
bool _expect(T value, S expected_value, const char* path, const char* error_msg) {
	if (value != expected_value) {
		printf("'%s' %s expected '", path, error_msg);
		print_value(expected_value);
		printf("' got '");
		print_value(value);
		printf("'\n");
		return false;
	}

	return true;
}

#define expect_read(expected_value, error_msg) if(!_expect_read(file, expected_value, path, error_msg)) return{0, 0}; 
#define expect(value, expected_value, error_msg) if(!_expect(value, expected_value, path, error_msg)) return{0, 0}; 

 BMP_Texture load_bmp_file(const char *path) {
	auto file = load_entire_file(path);
	auto start_file = file;

	if (!file.length) {
		log_error("'%s'", path);
		return{ 0, 0, 0 };
	}

	if (file.length < 54) {
		log_error("'%s'", path);
		return{ 0, 0, 0 };
	}

	expect_read("BM"_s, "Bitmap file corrupted");
	advance(file, 4);
	advance(file, 4);

	auto pixel_start_byte = *(int32*)file.data;
	advance(file, 4);

	expect_read((uint32)sizeof(BITMAPINFOHEADER), "Bitmapinfoheader wrong size");

	auto bitmap_header = *(BITMAPINFOHEADER*)(file.data - 4);
	advance(file, sizeof(BITMAPINFOHEADER));

	expect(bitmap_header.biPlanes, 1, "wrong number of Planes");

	expect(bitmap_header.biBitCount, 32, "wrong bits per pixel");

	expect(bitmap_header.biCompression, 0, "wrong compression");
	
	expect(bitmap_header.biClrUsed, 0, "no colortable expected");

	expect(bitmap_header.biClrImportant, 0, "colors from colortable used");

	advance(file, pixel_start_byte - (file.data - start_file.data));
	
	//expect(file.length, (bitmap_header.biWidth * bitmap_header.biHeight) * 4, "unexpected number of pixels");
	
	if (file.length < bitmap_header.biWidth * bitmap_header.biHeight * 4) {
		log_error("unexpected number of pixels");
		return{ 0, 0, 0 };
	}


	uint32 *pixel = (uint32*)malloc(bitmap_header.biWidth * bitmap_header.biHeight * 4);

	if (!pixel) {
		log_error("'%s' malloc failed", path);
		return { 0, 0 };
	}


	for (int y = 0; y < bitmap_header.biHeight; y++) {
		for (int x = 0; x < bitmap_header.biWidth; x++) {
				pixel[bitmap_header.biWidth * (bitmap_header.biHeight - y - 1) + x] = ((uint32*)file.data)[bitmap_header.biWidth * y + x];
		}
	}


	//memcpy(pixel, file.data, (bitmap_header.biWidth * bitmap_header.biHeight) * 4);

	free(start_file.data);
	return{ bitmap_header.biWidth, bitmap_header.biHeight, pixel };

}


#pragma pack(1)
struct TGA_Color_Map_Info {
	uint16 index;
	uint16 length;
	uint8 size;
};

#pragma pack(1)
struct TGA_Image_Specification {
	uint16 x_origin;
	uint16 y_origin;
	uint16 width;
	uint16 height;
	uint8 bits_per_pixel;
	uint8 image_descriptor;
};

BMP_Texture load_tga_file(const char* path) {
	auto file = load_entire_file(path);
	auto start_file = file;

	if (!file.length) {
		log_error("'%s' failed to load", path);
		return{ 0, 0, 0 };
	}

	auto id_length = read<uint8>(file);

	auto color_map_type = read<uint8>(file);
	expect(color_map_type, 0, "wrong color map type");

	auto image_type = read<uint8>(file);
	expect(image_type, 10, "wrong image type");

	auto color_map_info = read<TGA_Color_Map_Info >(file);
	expect(color_map_info.size, 0, "no existing color map");
	
	auto image_specification = read<TGA_Image_Specification>(file);
	expect(image_specification.bits_per_pixel, 32, "not 32 bitsperpixel");
	
	auto descriptor_mask = 0x30;
	auto image_origin = image_specification.image_descriptor & descriptor_mask; 
	expect(image_origin, 32, "wrong image origin");

	advance(file, id_length);

	uint32* start_pixel = (uint32*)malloc(image_specification.width * image_specification.height * 4);
	
	uint32* pixel = start_pixel;


	if (!start_pixel) {
		log_error("'%s' malloc failed", path);
		return { 0, 0 };
	}

	auto needed_pixel = image_specification.height * image_specification.width;

	for (; needed_pixel;) {
		auto type_and_repetition_count = read<uint8>(file);
		auto repetition_count = type_and_repetition_count & 0x7F;
		
		if ((type_and_repetition_count & 0x80)) {
			auto pixel_value = read<uint32>(file);
			for (; repetition_count + 1; repetition_count--) {
				*pixel = pixel_value;
				pixel++;
				needed_pixel--;
			}
		} else {
			for (; repetition_count + 1; repetition_count--) {
				auto pixel_value = read<uint32>(file);
				*pixel = pixel_value;
				pixel++;
				needed_pixel--;
			}
		}
	}

	return { image_specification.width, image_specification.height, start_pixel };
}


BMP_Texture tex1 = load_tga_file("../Assets/strawberry.tga");
//BMP_Texture tex1 = load_bmp_file("../Assets/strawberry.bmp");


 void render_sprite(BMP_Texture sprite, int x_offset, int y_offset) {
	 auto back_buffer = (Pixel *)BitmapMemory;
	 for (int y = 0; y < sprite.bmp_height; y++) {
		 for (int x = 0; x < sprite.bmp_width; x++) {
			 if (x + x_offset < 0 || x + x_offset >= BitmapWidth) {
				 continue;
			 }
			 if (y + y_offset < 0 || y + y_offset >= BitmapHeight) {
				 continue;
			 }

			 auto color = (Pixel *)&sprite.pixel[sprite.bmp_width * y + x];
			 auto red = color->red / 255.0f;
			 auto green = color->green / 255.0f;
			 auto blue = color->blue / 255.0f;

			 //sqrt für gamma_korrektur
			 red = square_root(red) * 255;
			 green = square_root(green) * 255;
			 blue = square_root(blue) * 255;

			 back_buffer[(y_offset + y) * BitmapWidth + x_offset + x].red = lerp(back_buffer[(y_offset + y) * BitmapWidth + x_offset + x].red, color->alpha / 255.0f, red);
			 back_buffer[(y_offset + y) * BitmapWidth + x_offset + x].green = lerp(back_buffer[(y_offset + y) * BitmapWidth + x_offset + x].green, color->alpha / 255.0f, green);
			 back_buffer[(y_offset + y) * BitmapWidth + x_offset + x].blue = lerp(back_buffer[(y_offset + y) * BitmapWidth + x_offset + x].blue, color->alpha / 255.0f, blue);
			

		 }
	 }
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
		renderQuad({ 0.5, a }, { 0.001, 0.01 }, { 1, 1, 1, 1 });
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
		play_sound(&se_wall_pong_2);
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

	float ratio = (float)BitmapWidth / (float)BitmapHeight;
	ball_size.y = ball_size.x * ratio;

	memset(BitmapMemory, 0, BitmapWidth * BitmapHeight * BytesPerPixel);

	render_middle_line();
	render_point(p1_points, p2_points);
	renderQuad(p1_pos, p1_size, p1_color);
	renderQuad(p2_pos, p2_size, p2_color);
	renderQuad(ball_pos, ball_size, ball_color);
	
	render_sprite(tex1, ball_pos.x * BitmapWidth - 0.5 * tex1.bmp_width * 10, ball_pos.y * BitmapHeight - 0.5 * tex1.bmp_height * 10);

	renderQuad({ 0.4, 0.4 }, { 0.2, 0.2 }, { 1, 1 , 1, 1 });
	renderQuad({ 0.3, 0.3 }, { 0.1, 0.1 }, { 1, 0 , 1, 1 });
	renderQuad({ 0.5, 0.4 }, { 0.1, 0.1 }, { 1, 0 , 0, 1 });
	renderQuad({ 0.5, 0.5 }, { 0.1, 0.1 }, { 0, 0 , 1, 0.5 });

	//correct_gamma();
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

bool LoadShaders();

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

			if (wParam == VK_F5) {
				LoadShaders();
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

ID3D11Texture2D* create_texture2d(uint32 width, uint32 height, DXGI_FORMAT format, uint32 bindflags = D3D11_BIND_SHADER_RESOURCE, DXGI_SAMPLE_DESC sampledesc = { 1, 0 }, uint32 miplevels = 1, uint32 arraysize = 1, uint32 miscflags = 0, D3D11_USAGE usage = D3D11_USAGE_DEFAULT, uint32 cpuaccesflags = 0) {
	D3D11_TEXTURE2D_DESC buffer_desc = {
		.Width			= width,
		.Height			= height,
		.MipLevels		= miplevels,
		.ArraySize		= arraysize,
		.Format			= format,
		.SampleDesc		= sampledesc,
		.Usage			= usage,
		.BindFlags		= bindflags,
		.CPUAccessFlags = cpuaccesflags,
		.MiscFlags		= miscflags,
	};

	ID3D11Texture2D* buffer;

	if (device->CreateTexture2D(&buffer_desc, 0, &buffer)) {
		log_error("CreateTexture2D() has failed");
		return 0;
	}

	return buffer;
}

ID3D11VertexShader *vertex_shader;
ID3D11InputLayout *input_layout;
ID3DBlob *vertex_shader_code;
ID3D11Buffer *vertex_buffer;

ID3DBlob *pixel_shader_code;
ID3D11PixelShader *pixel_shader;

struct Vertex {
	V4 pos;
	V4 color;
};

Vertex vertices[] = {
	{{ 0, .50, 0, 1 }, { 1, 0, 1, 1 } },
	{{ .50, -.50, 0, 1 }, { 0, 1, 1, 1 } },
	{{ -.50, -.50, 0, 1 }, { 1, 1, 0, 1 } },
};

uint32 vertex_size = sizeof(Vertex);
uint32 offset;

void RenderPongGPU() {

	devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devicecontext->IASetInputLayout(input_layout);
	devicecontext->IASetVertexBuffers(0, 1, &vertex_buffer, &vertex_size, &offset);
	devicecontext->VSSetShader(vertex_shader, 0, 0);
	devicecontext->PSSetShader(pixel_shader, 0, 0);
	devicecontext->Draw(3, 0);
}

bool LoadShaders() {

	ID3DBlob* error_msgs = 0;
	HRESULT error_code = 0;

	if (error_code = D3DCompileFromFile(L"../Assets/Shader/basic_vertex_shader.hlsl", 0, 0, "main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_OPTIMIZATION_LEVEL0, 0, &vertex_shader_code, &error_msgs)) {
		log("CompileFromFile has failed");
		if(error_msgs)
			log_error("%.*s", error_msgs->GetBufferSize(), error_msgs->GetBufferPointer());
		return false;
	}

	if (device->CreateVertexShader(vertex_shader_code->GetBufferPointer(), vertex_shader_code->GetBufferSize(), 0, &vertex_shader)) {
		log_error("CreateVertexShader has failed");
		return false;
	}

	if (error_code = D3DCompileFromFile(L"../Assets/Shader/basic_pixel_shader.hlsl", 0, 0, "main", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_OPTIMIZATION_LEVEL0, 0, &pixel_shader_code, &error_msgs)) {
		log("CompileFromFile has failed");
		if (error_msgs)
			log_error("%.*s", error_msgs->GetBufferSize(), error_msgs->GetBufferPointer());
		return false;
	}

	if (device->CreatePixelShader(pixel_shader_code->GetBufferPointer(), pixel_shader_code->GetBufferSize(), 0, &pixel_shader)) {
		log_error("CreateVertexShader has failed");
		return false;
	}
	return true;
}

int main() {
	CoInitializeEx(0, COINIT_APARTMENTTHREADED);
	//MessageBoxA(0, "Hello World", "Hi", MB_OKCANCEL | MB_ICONERROR); //"0x11" (macro) as bitflags is possible too
	init_sound();
	//play_sound(&se_pong_game);

	auto Window = (HWND)create_window(WindowMsgs);
	if (!Window) {

		printf("CreateWindow failed!\n");
		return 1;
	}

	D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 };

	DXGI_RATIONAL refresh_rate = {
		.Numerator = 60,
		.Denominator = 1,
	};

	DXGI_MODE_DESC buffer_desc = {
		.Width = render_width,
		.Height = render_height,
		.RefreshRate = refresh_rate,
		.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
		.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
		.Scaling = DXGI_MODE_SCALING_UNSPECIFIED,
	};

	DXGI_SAMPLE_DESC sample_desc = {
		.Count = 1,
		.Quality = 0,
	};

	DXGI_SWAP_CHAIN_DESC swap_chain_desc = {
		.BufferDesc = buffer_desc,
		.SampleDesc = sample_desc,
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = 2,
		.OutputWindow = Window,
		.Windowed = TRUE,
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
		.Flags = 0,
	};
	
	if (D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, D3D11_CREATE_DEVICE_DEBUG, feature_levels, 1, D3D11_SDK_VERSION, &swap_chain_desc, &swap_chain, &device, 0, &devicecontext)) {
		log_error("D3D11CreateDeviceAndSwapChain has failed.");
		return 1;
	}

	ID3D11Texture2D* present_buffer;
	if (swap_chain->GetBuffer(0, IID_PPV_ARGS(&present_buffer))) {
		log_error("PresentBuffer getting failed");
		return 1;
	}

	ID3D11RenderTargetView* render_target_view;
	if (device->CreateRenderTargetView(present_buffer, 0, &render_target_view)) {
		log_error("CreateRenderTargetView has failed");
		return 1;
	}

	present_buffer->Release();

	ID3D11Texture2D* present_depth_stencil_buffer = create_texture2d(render_width, render_height, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL);
	if (!present_depth_stencil_buffer)
		return 1;

	ID3D11DepthStencilView *present_depth_stencil_view;
	if (device->CreateDepthStencilView(present_depth_stencil_buffer, 0, &present_depth_stencil_view)) {
		log_error("CreateDepthStencilView has failed.");
		return 1;
	}

	present_depth_stencil_buffer->Release();

	ID3D11Texture2D* back_buffer = create_texture2d(render_width, render_height, DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_RENDER_TARGET);
	if (!back_buffer)
		return 1;

	ID3D11RenderTargetView* back_buffer_render_target_view;
	if (device->CreateRenderTargetView(back_buffer, 0, &back_buffer_render_target_view)) {
		log_error("CreateRenderTargetView has failed");
		return 1;
	}

	back_buffer->Release();

	ID3D11Texture2D* back_depth_stencil_buffer = create_texture2d(render_width, render_height, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL);
	if (!back_depth_stencil_buffer)
		return 1;

	ID3D11DepthStencilView* back_buffer_depth_stencil_view;
	if (device->CreateDepthStencilView(back_depth_stencil_buffer, 0, &back_buffer_depth_stencil_view)) {
		log_error("CreateDepthStencilView has failed.");
		return 1;
	}

	back_depth_stencil_buffer->Release();

	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc = {
		.DepthEnable = TRUE,
		.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
		.DepthFunc = D3D11_COMPARISON_LESS,
		.StencilEnable = FALSE,
		.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
		.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
		.FrontFace = { 
			.StencilFunc = D3D11_COMPARISON_ALWAYS,
			.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
			.StencilFailOp = D3D11_STENCIL_OP_KEEP,
			.StencilPassOp = D3D11_STENCIL_OP_KEEP,
		},
		.BackFace = {
			.StencilFunc = D3D11_COMPARISON_ALWAYS,
			.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
			.StencilFailOp = D3D11_STENCIL_OP_KEEP,
			.StencilPassOp = D3D11_STENCIL_OP_KEEP,
		},
	};


	if (device->CreateDepthStencilState(&depth_stencil_desc, &depth_stencil_state)) {
		log_error("CreateDepthStencilState has failed.");
		return 1;
	}

	if (!LoadShaders()) {
		log_error("LoadShaders has failed");
		return 1;
	}
	//device->CreateVertexShader()

	D3D11_INPUT_ELEMENT_DESC input_element_desc[] = { 
		{ "VERTEX_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "VERTEX_COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	if (device->CreateInputLayout(input_element_desc, 2, vertex_shader_code->GetBufferPointer(), vertex_shader_code->GetBufferSize(), &input_layout)) {
		log_error("CreateInputLayout has failed");
		return 1;
	}

	D3D11_VIEWPORT viewport = {
		.TopLeftX = 0,
		.TopLeftY = 0,
		.Width = (float)render_width,
		.Height = (float)render_height,
		.MinDepth = 0,
		.MaxDepth = 1,
	};
	devicecontext->RSSetViewports(1, &viewport);

	D3D11_BUFFER_DESC vertex_buffer_desc = {
		.ByteWidth = sizeof(vertices),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		.MiscFlags = 0,
		.StructureByteStride = 0,
	};

	D3D11_SUBRESOURCE_DATA subresource_data = {
		.pSysMem = vertices,
		.SysMemPitch = 0,
		.SysMemSlicePitch = 0,
	};

	if (device->CreateBuffer(&vertex_buffer_desc, &subresource_data, &vertex_buffer)) {
		log_error("Create Buffer has failed");
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

		UpdatePong(); 

		devicecontext->OMSetRenderTargets(1, &render_target_view, present_depth_stencil_view);
		V4 clear_color = { 0, 0, 0, 1 }; 
		devicecontext->ClearRenderTargetView(render_target_view, clear_color.E);
		devicecontext->ClearDepthStencilView(present_depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		//RenderPong();
		RenderPongGPU();

		//Win32UpdateWindow(Context, Window, 0, 0);
		swap_chain->Present(0, 0);
		
		XOffset++;
	}

	return 0;
}