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

//std::string based
const std::string str_tolower(std::string s);
const std::string str_toupper(std::string s);
const std::string ucfirst(std::string s);
std::vector<std::string> split(std::string str_to_split, char delimiter);
const std::string StringFormat(const char* format, ...);
const std::string vStringFormat(const char* format, va_list args);
std::vector<std::string> SplitString(const std::string &s, char delim);
std::string EscapeString(const char *src, size_t sz);
std::string EscapeString(const std::string &s);
bool StringIsNumber(const std::string &s);
void ToLowerString(std::string &s);
void ToUpperString(std::string &s);
std::string JoinString(const std::vector<std::string>& ar, const std::string &delim);
void find_replace(std::string& string_subject, const std::string& search_string, const std::string& replace_string);

//const char based

bool atobool(const char* iBool);
bool isAlphaNumeric(const char *text);
bool strn0cpyt(char* dest, const char* source, uint32 size);
char *CleanMobName(const char *in, char *out);
char *RemoveApostrophes(const char *s);
char* strn0cpy(char* dest, const char* source, uint32 size);
const char *ConvertArray(int input, char *returnchar);
const char *ConvertArrayF(float input, char *returnchar);
const char *MakeLowerString(const char *source);
int MakeAnyLenString(char** ret, const char* format, ...);
uint32 AppendAnyLenString(char** ret, uint32* bufsize, uint32* strlen, const char* format, ...);
uint32 hextoi(const char* num);
uint64 hextoi64(const char* num);
void MakeLowerString(const char *source, char *target);
void RemoveApostrophes(std::string &s);

#endif
