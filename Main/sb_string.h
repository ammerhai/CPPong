#pragma once
#include <stdint.h>
#include "math_graphics.h"

struct string {
	size_t length = 0;
	char *data = 0;
	
	constexpr string() : length(0), data(0) {};
	constexpr string(size_t length, char *data) : length(length), data(data) {};
	template<size_t _len> constexpr string(const char (&data)[_len]) : length(_len - 1), data((char *)data) {};

	char operator[](size_t index) {
		if(index < 0)
			return 0;
		if (index < length)
			return data[index];
		return 0;
	}

	char *begin() {
		return data;
	}

	char *end() {
		return data + length;
	}
};

inline bool operator==(string a, string b) {
	if (a.length == b.length) {
		for (int i = 0; i < a.length; i++)
			if (a[i] != b[i])
				return false;
		return true;
	}
	return false;
}

constexpr inline string operator""_s(const char *str, size_t length) {
	return { length, (char *)str };
}

constexpr inline bool starts_with(string str, string start) {
	if (str.length < start.length)
		return false;
	for (int i = 0; i < start.length; i++)
		if (str[i] != start[i])
			return false;
	return true;
}

constexpr inline bool ends_with(string str, string end) {
	if (str.length < end.length)
		return false;
	for (int i = 0; i < end.length; i++)
		if (str[str.length - end.length + i] != end[i])
			return false;
	return true;
}

constexpr inline void advance(string& str, int64_t amount = 1) {
	int64_t to_advance = min(str.length, amount);
	str.data += to_advance;
	str.length -= to_advance;
}