#include <windows.h>
#include <stdint.h>

#include "time.h"

double get_frequency() {
	LARGE_INTEGER hz;

	QueryPerformanceFrequency(&hz);
	return hz.QuadPart;
}

uint64_t get_start_time() {
	LARGE_INTEGER time;

	QueryPerformanceCounter(&time);
	return time.QuadPart;
}

double get_time() {
	LARGE_INTEGER time;
	static const auto frequency = get_frequency();
	static const auto start_time = get_start_time();

	QueryPerformanceCounter(&time);
	return (time.QuadPart - start_time) / frequency;
}