#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "utility.h"

#if (_MSC_VER < 1500)
	#define vsnprintf	_vsnprintf
#endif

#define safe_delete(d) if(d) { delete d; d=0; }
#define safe_delete_array(d) if(d) { delete[] d; d=0; }

int MakeAnyLenString(char** ret, const char* format, ...) {
	int buf_len = 128;
    int chars = -1;
	va_list argptr;
	va_start(argptr, format);
	while (chars == -1 || chars >= buf_len) {
		safe_delete_array(*ret);
		if (chars == -1)
			buf_len *= 2;
		else
			buf_len = chars + 1;
		*ret = new char[buf_len];
		chars = vsnprintf(*ret, buf_len, format, argptr);
	}
	va_end(argptr);
	return chars;
}