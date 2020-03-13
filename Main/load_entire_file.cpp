#define _CRT_SECURE_NO_WARNINGS
#include "load_entire_file.h"

#include <stdio.h>
#include <stdlib.h> 

string load_entire_file(const char* filename) {
	auto file = fopen(filename, "rb");
	if (!file)
		return {};

	if (fseek(file, 0, SEEK_END))
		return {};

	auto file_length = ftell(file);
	if (file_length == -1)
		return {};

	if (fseek(file, 0, SEEK_SET))
		return {};

	auto file_mem = malloc(file_length);
	if (!file_mem)
		return {};

	if (fread(file_mem, file_length, 1, file) != 1) {
		free(file_mem);
		return {};
	}

	return { (size_t)file_length, (char*)file_mem };
}