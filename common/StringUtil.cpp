/*
 * Copyright 2013 Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "StringUtil.h"

#include <string>
#include <cstdarg>
#include <cstring> // for strncpy
#include <stdexcept>

#ifdef _WINDOWS
	#include <windows.h>

	#define snprintf	_snprintf
	#define strncasecmp	_strnicmp
	#define strcasecmp  _stricmp

#else
	#include <stdlib.h>
	#include <stdio.h>
#endif

#ifndef va_copy
	#define va_copy(d,s) ((d) = (s))
#endif

// original source: 
// https://github.com/facebook/folly/blob/master/folly/String.cpp
//
void vStringFormat(std::string& output, const char* format, va_list args)
{
	va_list tmpargs;

	va_copy(tmpargs,args);
	int characters_used = vsnprintf(nullptr, 0, format, tmpargs);
	va_end(tmpargs);

	if (characters_used < 0) {
		// Looks like we have an invalid format string.
		// error out.
		std::string errorMessage("Invalid format string; snprintf returned negative with format string: ");
		errorMessage.append(format);
		
		throw std::runtime_error(errorMessage);
	}
	else if ((unsigned int)characters_used > output.capacity()) {
		output.resize(characters_used+1);
		va_copy(tmpargs,args);
		characters_used = vsnprintf(&output[0], output.capacity(), format, tmpargs);
		va_end(tmpargs);

		if (characters_used < 0) {
			// We shouldn't have a format error by this point, but I can't imagine what error we
			// could have by this point. Still, error out and report it.
			std::string errorMessage("Invalid format string or unknown vsnprintf error; vsnprintf returned negative with format string: ");
			errorMessage.append(format);
		
			throw std::runtime_error(errorMessage);
		}
	} 
	else {
		output.resize(characters_used + 1);

		va_copy(tmpargs,args);
		characters_used = vsnprintf(&output[0], output.capacity(), format, tmpargs);
		va_end(tmpargs);

		if (characters_used < 0) {
			// We shouldn't have a format error by this point, but I can't imagine what error we
			// could have by this point. still error out and report it.
			std::string errorMessage("Invalid format string or unknown vsnprintf error; vsnprintf returned negative with format string: ");
			errorMessage.append(format);
		
			throw std::runtime_error(errorMessage);
		}
		
	}
}

void StringFormat(std::string& output, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vStringFormat(output,format,args);
	va_end(args);
}

// normal strncpy doesnt put a null term on copied strings, this one does
// ref: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wcecrt/htm/_wcecrt_strncpy_wcsncpy.asp
char* strn0cpy(char* dest, const char* source, uint32 size) {
	if (!dest)
		return 0;
	if (size == 0 || source == 0) {
		dest[0] = 0;
		return dest;
	}
	strncpy(dest, source, size);
	dest[size - 1] = 0;
	return dest;
}

// String N w/null Copy Truncated?
// return value =true if entire string(source) fit, false if it was truncated
bool strn0cpyt(char* dest, const char* source, uint32 size) {
	if (!dest)
		return 0;
	if (size == 0 || source == 0) {
		dest[0] = 0;
		return false;
	}
	strncpy(dest, source, size);
	dest[size - 1] = 0;
	return (bool) (source[strlen(dest)] == 0);
}

const char *MakeLowerString(const char *source) {
	static char str[128];
	if (!source)
		return nullptr;
	MakeLowerString(source, str);
	return str;
}

void MakeLowerString(const char *source, char *target) {
	if (!source || !target) {
	*target=0;
		return;
	}
	while (*source)
	{
		*target = tolower(*source);
		target++;source++;
	}
	*target = 0;
}

int MakeAnyLenString(char** ret, const char* format, ...) {
	int buf_len = 128;
	int chars = -1;
	va_list argptr, tmpargptr;
	va_start(argptr, format);
	while (chars == -1 || chars >= buf_len) {
		safe_delete_array(*ret);
		if (chars == -1)
			buf_len *= 2;
		else
			buf_len = chars + 1;
		*ret = new char[buf_len];
		va_copy(tmpargptr, argptr);
		chars = vsnprintf(*ret, buf_len, format, tmpargptr);
	}
	va_end(argptr);
	return chars;
}

uint32 AppendAnyLenString(char** ret, uint32* bufsize, uint32* strlen, const char* format, ...) {
	if (*bufsize == 0)
		*bufsize = 256;
	if (*ret == 0)
		*strlen = 0;
	int chars = -1;
	char* oldret = 0;
	va_list argptr, tmpargptr;
	va_start(argptr, format);
	while (chars == -1 || chars >= (int32)(*bufsize-*strlen)) {
		if (chars == -1)
			*bufsize += 256;
		else
			*bufsize += chars + 25;
		oldret = *ret;
		*ret = new char[*bufsize];
		if (oldret) {
			if (*strlen)
				memcpy(*ret, oldret, *strlen);
			safe_delete_array(oldret);
		}
		va_copy(tmpargptr, argptr);
		chars = vsnprintf(&(*ret)[*strlen], (*bufsize-*strlen), format, tmpargptr);
	}
	va_end(argptr);
	*strlen += chars;
	return *strlen;
}

uint32 hextoi(char* num) {
	int len = strlen(num);
	if (len < 3)
		return 0;

	if (num[0] != '0' || (num[1] != 'x' && num[1] != 'X'))
		return 0;

	uint32 ret = 0;
	int mul = 1;
	for (int i=len-1; i>=2; i--) {
		if (num[i] >= 'A' && num[i] <= 'F')
			ret += ((num[i] - 'A') + 10) * mul;
		else if (num[i] >= 'a' && num[i] <= 'f')
			ret += ((num[i] - 'a') + 10) * mul;
		else if (num[i] >= '0' && num[i] <= '9')
			ret += (num[i] - '0') * mul;
		else
			return 0;
		mul *= 16;
	}
	return ret;
}

uint64 hextoi64(char* num) {
	int len = strlen(num);
	if (len < 3)
		return 0;

	if (num[0] != '0' || (num[1] != 'x' && num[1] != 'X'))
		return 0;

	uint64 ret = 0;
	int mul = 1;
	for (int i=len-1; i>=2; i--) {
		if (num[i] >= 'A' && num[i] <= 'F')
			ret += ((num[i] - 'A') + 10) * mul;
		else if (num[i] >= 'a' && num[i] <= 'f')
			ret += ((num[i] - 'a') + 10) * mul;
		else if (num[i] >= '0' && num[i] <= '9')
			ret += (num[i] - '0') * mul;
		else
			return 0;
		mul *= 16;
	}
	return ret;
}

bool atobool(char* iBool) {
	if (!strcasecmp(iBool, "true"))
		return true;
	if (!strcasecmp(iBool, "false"))
		return false;
	if (!strcasecmp(iBool, "yes"))
		return true;
	if (!strcasecmp(iBool, "no"))
		return false;
	if (!strcasecmp(iBool, "on"))
		return true;
	if (!strcasecmp(iBool, "off"))
		return false;
	if (!strcasecmp(iBool, "enable"))
		return true;
	if (!strcasecmp(iBool, "disable"))
		return false;
	if (!strcasecmp(iBool, "enabled"))
		return true;
	if (!strcasecmp(iBool, "disabled"))
		return false;
	if (!strcasecmp(iBool, "y"))
		return true;
	if (!strcasecmp(iBool, "n"))
		return false;
	if (atoi(iBool))
		return true;
	return false;
}

// solar: removes the crap and turns the underscores into spaces.
char *CleanMobName(const char *in, char *out)
{
	unsigned i, j;
	
	for(i = j = 0; i < strlen(in); i++)
	{
		// convert _ to space.. any other conversions like this?  I *think* this
		// is the only non alpha char that's not stripped but converted.
		if(in[i] == '_')
		{
			out[j++] = ' ';
		}
		else
		{
			if(isalpha(in[i]) || (in[i] == '`'))	// numbers, #, or any other crap just gets skipped
				out[j++] = in[i];
		}
	}
	out[j] = 0;	// terimnate the string before returning it
	return out;
}


void RemoveApostrophes(std::string &s)
{
	for(unsigned int i = 0; i < s.length(); ++i)
		if(s[i] == '\'')
			 s[i] = '_';
}

char *RemoveApostrophes(const char *s)
{
	char *NewString = new char[strlen(s) + 1];

	strcpy(NewString, s);

	for(unsigned int i = 0 ; i < strlen(NewString); ++i)
		if(NewString[i] == '\'')
			 NewString[i] = '_';

	return NewString;
}

const char *ConvertArray(int input, char *returnchar)
{
	sprintf(returnchar, "%i" ,input);
	return returnchar;
}

const char *ConvertArrayF(float input, char *returnchar)
{
	sprintf(returnchar, "%0.2f", input);
	return returnchar;
}
