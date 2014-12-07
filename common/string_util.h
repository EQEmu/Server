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
#include <vector>
#include <cstdarg>
#include <string.h>
#include "types.h"


const std::string vStringFormat(const char* format, va_list args);
const std::string StringFormat(const char* format, ...);
std::string EscapeString(const std::string &s);
std::string EscapeString(const char *src, size_t sz);

const char *MakeLowerString(const char *source);

void MakeLowerString(const char *source, char *target);

int MakeAnyLenString(char** ret, const char* format, ...);
uint32 AppendAnyLenString(char** ret, uint32* bufsize, uint32* strlen, const char* format, ...);

uint32 hextoi(const char* num);
uint64 hextoi64(const char* num);
bool atobool(const char* iBool);

char* strn0cpy(char* dest, const char* source, uint32 size);
		// return value =true if entire string(source) fit, false if it was truncated
bool strn0cpyt(char* dest, const char* source, uint32 size);

char *CleanMobName(const char *in, char *out);

const char *ConvertArray(int input, char *returnchar);
const char *ConvertArrayF(float input, char *returnchar);

void RemoveApostrophes(std::string &s);
char *RemoveApostrophes(const char *s);

std::vector<std::string> SplitString(const std::string &s, char delim);

bool isAlphaNumeric(const char *text);

#endif
