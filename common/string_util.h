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
#include <tuple>

#ifndef _WIN32
// this doesn't appear to affect linux-based systems..need feedback for _WIN64
#include <fmt/format.h>
#endif

#ifdef _WINDOWS
#include <ctype.h>
#include <functional>
#include <algorithm>
#endif

#include "types.h"

//std::string based
const std::string str_tolower(std::string s);
const std::string str_toupper(std::string s);
const std::string ucfirst(std::string s);
std::vector<std::string> split(std::string str_to_split, char delimiter);
const std::string StringFormat(const char* format, ...);
const std::string vStringFormat(const char* format, va_list args);
std::string implode(std::string glue, std::vector<std::string> src);

/**
 * @param str
 * @param chars
 * @return
 */
inline std::string &ltrim(std::string &str, const std::string &chars = "\t\n\v\f\r ")
{
	str.erase(0, str.find_first_not_of(chars));
	return str;
}

/**
 * @param str
 * @param chars
 * @return
 */
inline std::string &rtrim(std::string &str, const std::string &chars = "\t\n\v\f\r ")
{
	str.erase(str.find_last_not_of(chars) + 1);
	return str;
}

/**
 * @param str
 * @param chars
 * @return
 */
inline std::string &trim(std::string &str, const std::string &chars = "\t\n\v\f\r ")
{
	return ltrim(rtrim(str, chars), chars);
}

template <typename T>
std::string implode(const std::string &glue, const std::pair<char, char> &encapsulation, const std::vector<T> &src)
{
	if (src.empty()) {
		return {};
	}

	std::ostringstream oss;
	
	for (const T &src_iter : src) {
		oss << encapsulation.first << src_iter << encapsulation.second << glue;
	}

	std::string output(oss.str());
	output.resize(output.size() - glue.size());
	
	return output;
}

// _WIN32 builds require that #include<fmt/format.h> be included in whatever code file the invocation is made from (no header files)
template <typename T1, typename T2>
std::vector<std::string> join_pair(const std::string &glue, const std::pair<char, char> &encapsulation, const std::vector<std::pair<T1, T2>> &src)
{
	if (src.empty()) {
		return {};
	}

	std::vector<std::string> output;

	for (const std::pair<T1, T2> &src_iter : src) {
		output.push_back(
			
			fmt::format(
				"{}{}{}{}{}{}{}",
				encapsulation.first,
				src_iter.first,
				encapsulation.second,
				glue,
				encapsulation.first,
				src_iter.second,
				encapsulation.second
			)
		);
	}

	return output;
}

// _WIN32 builds require that #include<fmt/format.h> be included in whatever code file the invocation is made from (no header files)
template <typename T1, typename T2, typename T3, typename T4>
std::vector<std::string> join_tuple(const std::string &glue, const std::pair<char, char> &encapsulation, const std::vector<std::tuple<T1, T2, T3, T4>> &src)
{
	if (src.empty()) {
		return {};
	}

	std::vector<std::string> output;

	for (const std::tuple<T1, T2, T3, T4> &src_iter : src) {

		output.push_back(
			
			fmt::format(
				"{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}",
				encapsulation.first,
				std::get<0>(src_iter),
				encapsulation.second,
				glue,
				encapsulation.first,
				std::get<1>(src_iter),
				encapsulation.second,
				glue,
				encapsulation.first,
				std::get<2>(src_iter),
				encapsulation.second,
				glue,
				encapsulation.first,
				std::get<3>(src_iter),
				encapsulation.second
			)
		);
	}

	return output;
}

std::vector<std::string> SplitString(const std::string &s, char delim);
std::string EscapeString(const char *src, size_t sz);
std::string EscapeString(const std::string &s);
bool StringIsNumber(const std::string &s);
void ToLowerString(std::string &s);
void ToUpperString(std::string &s);
std::string JoinString(const std::vector<std::string>& ar, const std::string &delim);
void find_replace(std::string& string_subject, const std::string& search_string, const std::string& replace_string);
void ParseAccountString(const std::string &s, std::string &account, std::string &loginserver);

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
