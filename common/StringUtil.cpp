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
#include <stdexcept>

#ifdef _WINDOWS
	#include <windows.h>

	#define snprintf	_snprintf
	#define strncasecmp	_strnicmp
	#define strcasecmp  _stricmp
#else
	#include <stdlib.h>
#endif


// original source: 
// https://github.com/facebook/folly/blob/master/folly/String.cpp
//
void StringFormat(std::string& output, const char* format, ...) 
{
	va_list args;
	// Tru to the space at the end of output for our output buffer.
	// Find out write point then inflate its size temporarily to its
	// capacity; we will later shrink it to the size needed to represent
	// the formatted string.  If this buffer isn't large enough, we do a
	// resize and try again.

	const auto write_point = output.size();
	auto remaining = output.capacity() - write_point;
	output.resize(output.capacity());

	va_start(args, format);
	int bytes_used = vsnprintf(&output[write_point], remaining, format,args);
	va_end(args);
	if (bytes_used < 0) {
		
		std::string errorMessage("Invalid format string; snprintf returned negative with format string: ");
		errorMessage.append(format);
		
		throw std::runtime_error(errorMessage);
	} 
	else if ((unsigned int)bytes_used < remaining) {
		// There was enough room, just shrink and return.
		output.resize(write_point + bytes_used);
	} 
	else {
		output.resize(write_point + bytes_used + 1);
		remaining = bytes_used + 1;

		va_start(args, format);
		bytes_used = vsnprintf(&output[write_point], remaining, format, args);
		va_end(args);
		
		if ((unsigned int)(bytes_used + 1) != remaining) {
			
			std::string errorMessage("vsnprint retry did not manage to work with format string: ");
			errorMessage.append(format);
		
			throw std::runtime_error(errorMessage);
		}
		
		output.resize(write_point + bytes_used);
	}
}
