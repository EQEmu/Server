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

#include "string_util.h"
#include <fmt/format.h>
#include <algorithm>
#include <cctype>

#ifdef _WINDOWS
#include <windows.h>

#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp  _stricmp

#else

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#endif

#ifndef va_copy
#define va_copy(d,s) ((d) = (s))
#endif

// original source:
// https://github.com/facebook/folly/blob/master/folly/String.cpp
//
const std::string vStringFormat(const char *format, va_list args)
{
	std::string output;
	va_list     tmpargs;

	va_copy(tmpargs, args);
	int characters_used = vsnprintf(nullptr, 0, format, tmpargs);
	va_end(tmpargs);

	// Looks like we have a valid format string.
	if (characters_used > 0) {
		output.resize(characters_used + 1);

		va_copy(tmpargs, args);
		characters_used = vsnprintf(&output[0], output.capacity(), format, tmpargs);
		va_end(tmpargs);

		output.resize(characters_used);

		// We shouldn't have a format error by this point, but I can't imagine what error we
		// could have by this point. Still, return empty string;
		if (characters_used < 0) {
			output.clear();
		}
	}
	return output;
}

const std::string str_tolower(std::string s)
{
	std::transform(
		s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return ::tolower(c); }
	);
	return s;
}

const std::string str_toupper(std::string s)
{
	std::transform(
		s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return ::toupper(c); }
	);
	return s;
}

const std::string ucfirst(std::string s)
{
	std::string output = s;
	if (!s.empty()) {
		output[0] = static_cast<char>(::toupper(s[0]));
	}

	return output;
}

const std::string StringFormat(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	std::string output = vStringFormat(format, args);
	va_end(args);
	return output;
}

std::vector<std::string> SplitString(const std::string &str, const char delim)
{
	std::vector<std::string> ret;
	std::string::size_type   start = 0;
	auto                     end   = str.find(delim);
	while (end != std::string::npos) {
		ret.emplace_back(str, start, end - start);
		start = end + 1;
		end   = str.find(delim, start);
	}
	// this will catch the last word since the string is unlikely to end with a delimiter
	if (str.length() > start) {
		ret.emplace_back(str, start, str.length() - start);
	}
	return ret;
}

// this one takes delimiter length into consideration
std::vector<std::string> split_string(std::string s, std::string delimiter)
{
	size_t                   pos_start = 0, pos_end, delim_len = delimiter.length();
	std::string              token;
	std::vector<std::string> res;

	while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
		token     = s.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back(token);
	}

	res.push_back(s.substr(pos_start));
	return res;
}

std::string get_between(const std::string &s, std::string start_delim, std::string stop_delim)
{
	if (s.find(start_delim) == std::string::npos && s.find(stop_delim) == std::string::npos) {
		return "";
	}

	auto first_split = split_string(s, start_delim);
	if (!first_split.empty()) {
		std::string remaining_block = first_split[1];
		auto        second_split    = split_string(remaining_block, stop_delim);
		if (!second_split.empty()) {
			std::string between = second_split[0];
			return between;
		}
	}

	return "";
}

std::string::size_type
search_deliminated_string(const std::string &haystack, const std::string &needle, const char deliminator)
{
	// this shouldn't go out of bounds, even without obvious bounds checks
	auto pos = haystack.find(needle);
	while (pos != std::string::npos) {
		auto c = haystack[pos + needle.length()];
		if ((c == '\0' || c == deliminator) && (pos == 0 || haystack[pos - 1] == deliminator)) {
			return pos;
		}
		pos = haystack.find(needle, pos + needle.length());
	}
	return std::string::npos;
}

std::string implode(std::string glue, std::vector<std::string> src)
{
	if (src.empty()) {
		return {};
	}

	std::ostringstream                 output;
	std::vector<std::string>::iterator src_iter;

	for (src_iter = src.begin(); src_iter != src.end(); src_iter++) {
		output << *src_iter << glue;
	}

	std::string final_output = output.str();
	final_output.resize(output.str().size() - glue.size());

	return final_output;
}

std::vector<std::string> wrap(std::vector<std::string> &src, std::string character)
{
	std::vector<std::string> new_vector;
	new_vector.reserve(src.size());

	for (auto &e: src) {
		if (e == "null") {
			new_vector.emplace_back(e);
			continue;
		}

		new_vector.emplace_back(character + e + character);
	}

	return new_vector;
}

std::string EscapeString(const std::string &s)
{
	std::string ret;

	size_t      sz = s.length();
	for (size_t i  = 0; i < sz; ++i) {
		char c = s[i];
		switch (c) {
			case '\x00':
				ret += "\\x00";
				break;
			case '\n':
				ret += "\\n";
				break;
			case '\r':
				ret += "\\r";
				break;
			case '\\':
				ret += "\\\\";
				break;
			case '\'':
				ret += "\\'";
				break;
			case '\"':
				ret += "\\\"";
				break;
			case '\x1a':
				ret += "\\x1a";
				break;
			default:
				ret.push_back(c);
				break;
		}
	}

	return ret;
}

std::string EscapeString(const char *src, size_t sz)
{
	std::string ret;

	for (size_t i = 0; i < sz; ++i) {
		char c = src[i];
		switch (c) {
			case '\x00':
				ret += "\\x00";
				break;
			case '\n':
				ret += "\\n";
				break;
			case '\r':
				ret += "\\r";
				break;
			case '\\':
				ret += "\\\\";
				break;
			case '\'':
				ret += "\\'";
				break;
			case '\"':
				ret += "\\\"";
				break;
			case '\x1a':
				ret += "\\x1a";
				break;
			default:
				ret.push_back(c);
				break;
		}
	}

	return ret;
}

bool StringIsNumber(const std::string &s)
{
	try {
		auto r = stod(s);
		return true;
	}
	catch (std::exception &) {
		return false;
	}
}

void ToLowerString(std::string &s)
{
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
}

void ToUpperString(std::string &s)
{
	std::transform(s.begin(), s.end(), s.begin(), ::toupper);
}

std::string JoinString(const std::vector<std::string> &ar, const std::string &delim)
{
	std::string ret;
	for (size_t i = 0; i < ar.size(); ++i) {
		if (i != 0) {
			ret += delim;
		}

		ret += ar[i];
	}

	return ret;
}

void find_replace(std::string &string_subject, const std::string &search_string, const std::string &replace_string)
{
	if (string_subject.find(search_string) == std::string::npos) {
		return;
	}

	size_t start_pos = 0;
	while ((start_pos = string_subject.find(search_string, start_pos)) != std::string::npos) {
		string_subject.replace(start_pos, search_string.length(), replace_string);
		start_pos += replace_string.length();
	}
}

std::string replace_string(std::string subject, const std::string &search, const std::string &replace)
{
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return subject;
}

void ParseAccountString(const std::string &s, std::string &account, std::string &loginserver)
{
	auto split = SplitString(s, ':');
	if (split.size() == 2) {
		loginserver = split[0];
		account     = split[1];
	}
	else if (split.size() == 1) {
		account = split[0];
	}
}

//Const char based

// normal strncpy doesnt put a null term on copied strings, this one does
// ref: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wcecrt/htm/_wcecrt_strncpy_wcsncpy.asp
char *strn0cpy(char *dest, const char *source, uint32 size)
{
	if (!dest) {
		return 0;
	}
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
bool strn0cpyt(char *dest, const char *source, uint32 size)
{
	if (!dest) {
		return 0;
	}
	if (size == 0 || source == 0) {
		dest[0] = 0;
		return false;
	}
	strncpy(dest, source, size);
	dest[size - 1] = 0;
	return (bool) (source[strlen(dest)] == 0);
}

const char *MakeLowerString(const char *source)
{
	static char str[128];
	if (!source) {
		return nullptr;
	}
	MakeLowerString(source, str);
	return str;
}

void MakeLowerString(const char *source, char *target)
{
	if (!source || !target) {
		*target = 0;
		return;
	}
	while (*source) {
		*target = tolower(*source);
		target++;
		source++;
	}
	*target = 0;
}

uint32 hextoi(const char *num)
{
	if (num == nullptr) {
		return 0;
	}

	int len = strlen(num);
	if (len < 3) {
		return 0;
	}

	if (num[0] != '0' || (num[1] != 'x' && num[1] != 'X')) {
		return 0;
	}

	uint32   ret = 0;
	int      mul = 1;
	for (int i   = len - 1; i >= 2; i--) {
		if (num[i] >= 'A' && num[i] <= 'F') {
			ret += ((num[i] - 'A') + 10) * mul;
		}
		else if (num[i] >= 'a' && num[i] <= 'f') {
			ret += ((num[i] - 'a') + 10) * mul;
		}
		else if (num[i] >= '0' && num[i] <= '9') {
			ret += (num[i] - '0') * mul;
		}
		else {
			return 0;
		}
		mul *= 16;
	}
	return ret;
}

uint64 hextoi64(const char *num)
{
	if (num == nullptr) {
		return 0;
	}

	int len = strlen(num);
	if (len < 3) {
		return 0;
	}

	if (num[0] != '0' || (num[1] != 'x' && num[1] != 'X')) {
		return 0;
	}

	uint64   ret = 0;
	int      mul = 1;
	for (int i   = len - 1; i >= 2; i--) {
		if (num[i] >= 'A' && num[i] <= 'F') {
			ret += ((num[i] - 'A') + 10) * mul;
		}
		else if (num[i] >= 'a' && num[i] <= 'f') {
			ret += ((num[i] - 'a') + 10) * mul;
		}
		else if (num[i] >= '0' && num[i] <= '9') {
			ret += (num[i] - '0') * mul;
		}
		else {
			return 0;
		}
		mul *= 16;
	}
	return ret;
}

bool atobool(const char *iBool)
{

	if (iBool == nullptr) {
		return false;
	}
	if (!strcasecmp(iBool, "true")) {
		return true;
	}
	if (!strcasecmp(iBool, "false")) {
		return false;
	}
	if (!strcasecmp(iBool, "yes")) {
		return true;
	}
	if (!strcasecmp(iBool, "no")) {
		return false;
	}
	if (!strcasecmp(iBool, "on")) {
		return true;
	}
	if (!strcasecmp(iBool, "off")) {
		return false;
	}
	if (!strcasecmp(iBool, "enable")) {
		return true;
	}
	if (!strcasecmp(iBool, "disable")) {
		return false;
	}
	if (!strcasecmp(iBool, "enabled")) {
		return true;
	}
	if (!strcasecmp(iBool, "disabled")) {
		return false;
	}
	if (!strcasecmp(iBool, "y")) {
		return true;
	}
	if (!strcasecmp(iBool, "n")) {
		return false;
	}
	if (atoi(iBool)) {
		return true;
	}
	return false;
}

// removes the crap and turns the underscores into spaces.
char *CleanMobName(const char *in, char *out)
{
	unsigned i, j;

	for (i = j = 0; i < strlen(in); i++) {
		// convert _ to space.. any other conversions like this?  I *think* this
		// is the only non alpha char that's not stripped but converted.
		if (in[i] == '_') {
			out[j++] = ' ';
		}
		else {
			if (isalpha(in[i]) || (in[i] == '`')) {    // numbers, #, or any other crap just gets skipped
				out[j++] = in[i];
			}
		}
	}
	out[j] = 0;    // terimnate the string before returning it
	return out;
}


void RemoveApostrophes(std::string &s)
{
	for (unsigned int i = 0; i < s.length(); ++i)
		if (s[i] == '\'') {
			s[i] = '_';
		}
}

char *RemoveApostrophes(const char *s)
{
	auto NewString = new char[strlen(s) + 1];

	strcpy(NewString, s);

	for (unsigned int i = 0; i < strlen(NewString); ++i)
		if (NewString[i] == '\'') {
			NewString[i] = '_';
		}

	return NewString;
}

const char *ConvertArray(int input, char *returnchar)
{
	sprintf(returnchar, "%i", input);
	return returnchar;
}

const char *ConvertArrayF(float input, char *returnchar)
{
	sprintf(returnchar, "%0.2f", input);
	return returnchar;
}

bool isAlphaNumeric(const char *text)
{
	for (unsigned int charIndex = 0; charIndex < strlen(text); charIndex++) {
		if ((text[charIndex] < 'a' || text[charIndex] > 'z') &&
			(text[charIndex] < 'A' || text[charIndex] > 'Z') &&
			(text[charIndex] < '0' || text[charIndex] > '9')) {
			return false;
		}
	}

	return true;
}

// Function to convert single digit or two digit number into words
std::string convert2digit(int n, std::string suffix)
{
	// if n is zero
	if (n == 0) {
		return "";
	}

	// split n if it is more than 19
	if (n > 19) {
		return NUM_TO_ENGLISH_Y[n / 10] + NUM_TO_ENGLISH_X[n % 10] + suffix;
	}
	else {
		return NUM_TO_ENGLISH_X[n] + suffix;
	}
}

// Function to convert a given number (max 9-digits) into words
std::string numberToWords(unsigned long long int n)
{
	// string to store word representation of given number
	std::string res;

	// this handles digits at ones & tens place
	res = convert2digit((n % 100), "");

	if (n > 100 && n % 100) {
		res = "and " + res;
	}

	// this handles digit at hundreds place
	res = convert2digit(((n / 100) % 10), "Hundred ") + res;

	// this handles digits at thousands & tens thousands place
	res = convert2digit(((n / 1000) % 100), "Thousand ") + res;

	// this handles digits at hundred thousands & one millions place
	res = convert2digit(((n / 100000) % 100), "Lakh, ") + res;

	// this handles digits at ten millions & hundred millions place
	res = convert2digit((n / 10000000) % 100, "Crore, ") + res;

	// this handles digits at ten millions & hundred millions place
	res = convert2digit((n / 1000000000) % 100, "Billion, ") + res;

	return res;
}

// first letter capitalized and rest made lower case
std::string FormatName(const std::string &char_name)
{
	std::string formatted(char_name);
	if (!formatted.empty()) {
		std::transform(formatted.begin(), formatted.end(), formatted.begin(), ::tolower);
		formatted[0] = ::toupper(formatted[0]);
	}
	return formatted;
}

bool IsAllowedWorldServerCharacterList(char c)
{
	const char *valid_characters = ":[](){}.!@#$%^&*-=+<>/\\|'\"";
	if (strchr(valid_characters, c)) {
		return true;
	}

	return false;
}

void SanitizeWorldServerName(char *name)
{
	std::string server_long_name = name;

	strcpy(name, SanitizeWorldServerName(server_long_name).c_str());
}

std::string SanitizeWorldServerName(std::string server_long_name)
{
	server_long_name.erase(
		std::remove_if(
			server_long_name.begin(),
			server_long_name.end(),
			[](char c) {
				return !(std::isalpha(c) || std::isalnum(c) || std::isspace(c) || IsAllowedWorldServerCharacterList(c));
			}
		), server_long_name.end()
	);

	server_long_name = trim(server_long_name);

	// bad word filter
	for (auto &piece: split_string(server_long_name, " ")) {
		for (auto &word: GetBadWords()) {
			// for shorter words that can actually be part of legitimate words
			// make sure that it isn't part of another word by matching on a space
			if (str_tolower(piece) == word) {
				find_replace(
					server_long_name,
					piece,
					repeat("*", (int) word.length())
				);
				continue;
			}

			auto pos = str_tolower(piece).find(word);
			if (str_tolower(piece).find(word) != std::string::npos && piece.length() > 4 && word.length() > 4) {
				auto found_word = piece.substr(pos, word.length());
				std::string replaced_piece = piece.substr(pos, word.length());

				find_replace(
					server_long_name,
					replaced_piece,
					repeat("*", (int) word.length())
				);
			}
		}
	}

	return server_long_name;
}

std::string repeat(std::string s, int n)
{
	std::string s1 = s;
	for (int    i  = 1; i < n; i++) {
		s += s1;
	}

	return s;
}

std::vector<std::string> GetBadWords()
{
	return std::vector<std::string>{
		"2g1c",
		"acrotomophilia",
		"anal",
		"anilingus",
		"anus",
		"apeshit",
		"arsehole",
		"ass",
		"asshole",
		"assmunch",
		"autoerotic",
		"babeland",
		"bangbros",
		"bangbus",
		"bareback",
		"barenaked",
		"bastard",
		"bastardo",
		"bastinado",
		"bbw",
		"bdsm",
		"beaner",
		"beaners",
		"beaver",
		"beastiality",
		"bestiality",
		"bimbos",
		"birdlock",
		"bitch",
		"bitches",
		"blowjob",
		"blumpkin",
		"bollocks",
		"bondage",
		"boner",
		"boob",
		"boobs",
		"bukkake",
		"bulldyke",
		"bullshit",
		"bung",
		"bunghole",
		"busty",
		"butt",
		"buttcheeks",
		"butthole",
		"camel toe",
		"camgirl",
		"camslut",
		"camwhore",
		"carpetmuncher",
		"cialis",
		"circlejerk",
		"clit",
		"clitoris",
		"clusterfuck",
		"cock",
		"cocks",
		"coprolagnia",
		"coprophilia",
		"cornhole",
		"coon",
		"coons",
		"creampie",
		"cum",
		"cumming",
		"cumshot",
		"cumshots",
		"cunnilingus",
		"cunt",
		"darkie",
		"daterape",
		"deepthroat",
		"dendrophilia",
		"dick",
		"dildo",
		"dingleberry",
		"dingleberries",
		"doggiestyle",
		"doggystyle",
		"dolcett",
		"domination",
		"dominatrix",
		"dommes",
		"hump",
		"dvda",
		"ecchi",
		"ejaculation",
		"erotic",
		"erotism",
		"escort",
		"eunuch",
		"fag",
		"faggot",
		"fecal",
		"felch",
		"fellatio",
		"feltch",
		"femdom",
		"figging",
		"fingerbang",
		"fingering",
		"fisting",
		"footjob",
		"frotting",
		"fuck",
		"fuckin",
		"fucking",
		"fucktards",
		"fudgepacker",
		"futanari",
		"gangbang",
		"gangbang",
		"gaysex",
		"genitals",
		"goatcx",
		"goatse",
		"gokkun",
		"goodpoop",
		"goregasm",
		"grope",
		"g-spot",
		"guro",
		"handjob",
		"hentai",
		"homoerotic",
		"honkey",
		"hooker",
		"horny",
		"humping",
		"incest",
		"intercourse",
		"jailbait",
		"jigaboo",
		"jiggaboo",
		"jiggerboo",
		"jizz",
		"juggs",
		"kike",
		"kinbaku",
		"kinkster",
		"kinky",
		"knobbing",
		"livesex",
		"lolita",
		"lovemaking",
		"masturbate",
		"masturbating",
		"masturbation",
		"milf",
		"mong",
		"motherfucker",
		"muffdiving",
		"nambla",
		"nawashi",
		"negro",
		"neonazi",
		"nigga",
		"nigger",
		"nimphomania",
		"nipple",
		"nipples",
		"nsfw",
		"nude",
		"nudity",
		"nutten",
		"nympho",
		"nymphomania",
		"octopussy",
		"omorashi",
		"orgasm",
		"orgy",
		"paedophile",
		"paki",
		"panties",
		"panty",
		"pedobear",
		"pedophile",
		"pegging",
		"penis",
		"pikey",
		"pissing",
		"pisspig",
		"playboy",
		"ponyplay",
		"poof",
		"poon",
		"poontang",
		"punany",
		"poopchute",
		"porn",
		"porno",
		"pornography",
		"pthc",
		"pubes",
		"pussy",
		"queaf",
		"queef",
		"quim",
		"raghead",
		"rape",
		"raping",
		"rapist",
		"rectum",
		"rimjob",
		"rimming",
		"sadism",
		"santorum",
		"scat",
		"schlong",
		"scissoring",
		"semen",
		"sex",
		"sexcam",
		"sexo",
		"sexy",
		"sexual",
		"sexually",
		"sexuality",
		"shemale",
		"shibari",
		"shit",
		"shitblimp",
		"shitty",
		"shota",
		"shrimping",
		"skeet",
		"slanteye",
		"slut",
		"s&m",
		"smut",
		"snatch",
		"snowballing",
		"sodomize",
		"sodomy",
		"spastic",
		"spic",
		"splooge",
		"spooge",
		"spunk",
		"strapon",
		"strappado",
		"suck",
		"sucks",
		"swastika",
		"swinger",
		"threesome",
		"throating",
		"thumbzilla",
		"tight white",
		"tit",
		"tits",
		"titties",
		"titty",
		"topless",
		"tosser",
		"towelhead",
		"tranny",
		"tribadism",
		"tubgirl",
		"tushy",
		"twat",
		"twink",
		"twinkie",
		"undressing",
		"upskirt",
		"urophilia",
		"vagina",
		"viagra",
		"vibrator",
		"vorarephilia",
		"voyeur",
		"voyeurweb",
		"voyuer",
		"vulva",
		"wank",
		"wetback",
		"whore",
		"worldsex",
		"xx",
		"xxx",
		"yaoi",
		"yiffy",
		"zoophilia"
	};
}

std::string ConvertSecondsToTime(int duration, bool is_milliseconds)
{
	if (duration <= 0) {
		return "Unknown";
	}

	if (is_milliseconds && duration < 1000) {
		return fmt::format(
			"{} Millisecond{}",
			duration,
			duration != 1 ? "s" : ""
		);
	}

	int timer_length = (
		is_milliseconds ?
		static_cast<int>(std::ceil(static_cast<float>(duration) / 1000.0f)) :
		duration
	);

	int days = int(timer_length / 86400);
	timer_length %= 86400;
	int hours = int(timer_length / 3600);
	timer_length %= 3600;
	int minutes = int(timer_length / 60);
	timer_length %= 60;
	int seconds = timer_length;
	std::string time_string = "Unknown";
	std::string day_string = (days == 1 ? "Day" : "Days");
	std::string hour_string = (hours == 1 ? "Hour" : "Hours");
	std::string minute_string = (minutes == 1 ? "Minute" : "Minutes");
	std::string second_string = (seconds == 1 ? "Second" : "Seconds");
	if (days && hours && minutes && seconds) { // DHMS
		time_string = fmt::format(
			"{} {}, {} {}, {} {}, and {} {}",
			days,
			day_string,
			hours,
			hour_string,
			minutes,
			minute_string,
			seconds,
			second_string
		);
	} else if (days && hours && minutes && !seconds) { // DHM
		time_string = fmt::format(
			"{} {}, {} {}, and {} {}",
			days,
			day_string,
			hours,
			hour_string,
			minutes,
			minute_string
		);
	} else if (days && hours && !minutes && seconds) { // DHS
		time_string = fmt::format(
			"{} {}, {} {}, and {} {}",
			days,
			day_string,
			hours,
			hour_string,
			seconds,
			second_string
		);
	} else if (days && hours && !minutes && !seconds) { // DH
		time_string = fmt::format(
			"{} {} and {} {}",
			days,
			day_string,
			hours,
			hour_string
		);
	} else if (days && !hours && minutes && seconds) { // DMS
		time_string = fmt::format(
			"{} {}, {} {}, and {} {}",
			days,
			day_string,
			minutes,
			minute_string,
			seconds,
			second_string
		);
	} else if (days && !hours && minutes && !seconds) { // DM
		time_string = fmt::format(
			"{} {} and {} {}",
			days,
			day_string,
			minutes,
			minute_string
		);
	} else if (days && !hours && !minutes && seconds) { // DS
		time_string = fmt::format(
			"{} {} and {} {}",
			days,
			day_string,
			seconds,
			second_string
		);
	} else if (days && !hours && !minutes && !seconds) { // D
		time_string = fmt::format(
			"{} {}",
			days,
			day_string
		);
	} else if (!days && hours && minutes && seconds) { // HMS
		time_string = fmt::format(
			"{} {}, {} {}, and {} {}",
			hours,
			hour_string,
			minutes,
			minute_string,
			seconds,
			second_string
		);
	} else if (!days && hours && minutes && !seconds) { // HM
		time_string = fmt::format(
			"{} {} and {} {}",
			hours,
			hour_string,
			minutes,
			minute_string
		);
	} else if (!days && hours && !minutes && seconds) { // HS
		time_string = fmt::format(
			"{} {} and {} {}",
			hours,
			hour_string,
			seconds,
			second_string
		);
	} else if (!days && hours && !minutes && !seconds) { // H
		time_string = fmt::format(
			"{} {}",
			hours,
			hour_string
		);
	} else if (!days && !hours && minutes && seconds) { // MS
		time_string = fmt::format(
			"{} {} and {} {}",
			minutes,
			minute_string,
			seconds,
			second_string
		);
	} else if (!days && !hours && minutes && !seconds) { // M
		time_string = fmt::format(
			"{} {}",
			minutes,
			minute_string
		);
	} else if (!days && !hours && !minutes && seconds) { // S
		time_string = fmt::format(
			"{} {}",
			seconds,
			second_string
		);
	}
	return time_string;
}

std::string ConvertMoneyToString(uint32 platinum, uint32 gold, uint32 silver, uint32 copper)
{
	std::string money_string = "Unknown";
	if (copper && silver && gold && platinum) { // CSGP
		money_string = fmt::format(
			"{} Platinum, {} Gold, {} Silver, and {} Copper",
			platinum,
			gold,
			silver,
			copper
		);
	} else if (copper && silver && gold && !platinum) { // CSG
		money_string = fmt::format(
			"{} Gold, {} Silver, and {} Copper",
			gold,
			silver,
			copper
		);
	} else if (copper && silver && !gold && !platinum) { // CS
		money_string = fmt::format(
			"{} Silver and {} Copper",
			silver,
			copper
		);
	} else if (copper && !silver && !gold && !platinum) { // C
		money_string = fmt::format(
			"{} Copper",
			copper
		);
	} else if (!copper && silver && gold && platinum) { // SGP
		money_string = fmt::format(
			"{} Platinum, {} Gold, and {} Silver",
			platinum,
			gold,
			silver
		);
	} else if (!copper && silver && gold && !platinum) { // SG
		money_string = fmt::format(
			"{} Gold and {} Silver",
			gold,
			silver
		);
	} else if (!copper && silver && !gold && !platinum) { // S
		money_string = fmt::format(
			"{} Silver",
			silver
		);
	} else if (copper && !silver && gold && platinum) { // CGP
		money_string = fmt::format(
			"{} Platinum, {} Gold, and {} Copper",
			platinum,
			gold,
			copper
		);
	} else if (copper && !silver && gold && !platinum) { // CG
		money_string = fmt::format(
			"{} Gold and {} Copper",
			gold,
			copper
		);
	} else if (!copper && !silver && gold && platinum) { // GP
		money_string = fmt::format(
			"{} Platinum and {} Gold",
			platinum,
			gold
		);
	} else if (!copper && !silver && gold && !platinum) { // G
		money_string = fmt::format(
			"{} Gold",
			gold
		);
	}
	return money_string;
}