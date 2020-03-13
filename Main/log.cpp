#include "log.h"
#include <stdio.h>
#include <stdarg.h>

void _log(const char* function_name, const char* format, ...) {
	printf("[%s] ", function_name);

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	printf("\n");
}

void _log_error(const char* function_name, const char* format, ...) {
	printf("[%s]ERROR: ", function_name);

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	printf("\n");
}
