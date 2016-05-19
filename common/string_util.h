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
#ifndef _STRINGUTIL_H_
#define _STRINGUTIL_H_

#include <sstream>
#include <string.h>
#include <vector>
#include <cstdarg>

#include "types.h"

bool atobool(const char* iBool);
bool isAlphaNumeric(const char *text);
bool strn0cpyt(char* dest, const char* source, uint32 size);

char *CleanMobName(const char *in, char *out);
char *RemoveApostrophes(const char *s);
char* strn0cpy(char* dest, const char* source, uint32 size);

const char *ConvertArray(int input, char *returnchar);
const char *ConvertArrayF(float input, char *returnchar);
const char *MakeLowerString(const char *source);
const std::string StringFormat(const char* format, ...);
const std::string vStringFormat(const char* format, va_list args);

int MakeAnyLenString(char** ret, const char* format, ...);

std::string EscapeString(const char *src, size_t sz);
std::string EscapeString(const std::string &s);

std::vector<std::string> SplitString(const std::string &s, char delim);

uint32 AppendAnyLenString(char** ret, uint32* bufsize, uint32* strlen, const char* format, ...);
uint32 hextoi(const char* num);

uint64 hextoi64(const char* num);

void MakeLowerString(const char *source, char *target);
void RemoveApostrophes(std::string &s);
void find_replace(std::string& string_subject, const std::string& search_string, const std::string& replace_string);



/*	EQEMu:  Everquest Server Emulator
	
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


namespace EQEmu
{
	class OutBuffer : public std::stringstream {
	public:
		inline size_t size() { return tellp(); }
		void overwrite(OutBuffer::pos_type position, const char *_Str, std::streamsize _Count);
		uchar* detach();
	};
}

#endif
