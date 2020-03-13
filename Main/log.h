#pragma once
#define log(format, ...) _log(__FUNCTION__, format, __VA_ARGS__)
#define log_error(format, ...) _log_error(__FUNCTION__, format, __VA_ARGS__)

void _log(const char* function_name, const char* format, ...);
void _log_error(const char* function_name, const char* format, ...);