/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2015 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/


#include "eqemu_logsys.h"
#include "platform.h"
#include "string_util.h"
#include "database.h"
#include "misc.h"

#include <iostream>
#include <fstream> 
#include <string>
#include <iomanip>
#include <time.h>

std::ofstream process_log;

#ifdef _WINDOWS
	#include <direct.h>
	#include <conio.h>
	#include <iostream>
	#include <dos.h>
	#include <windows.h>
#else
	#include <sys/stat.h>
#endif

/* Linux ANSI console color defines */
#define LC_RESET   "\033[0m"
#define LC_BLACK   "\033[30m"      /* Black */
#define LC_RED     "\033[31m"      /* Red */
#define LC_GREEN   "\033[32m"      /* Green */
#define LC_YELLOW  "\033[33m"      /* Yellow */
#define LC_BLUE    "\033[34m"      /* Blue */
#define LC_MAGENTA "\033[35m"      /* Magenta */
#define LC_CYAN    "\033[36m"      /* Cyan */
#define LC_WHITE   "\033[37m"      /* White */

namespace Console {
	enum Color {
		Black = 0,
		Blue = 1,
		Green = 2,
		Cyan = 3,
		Red = 4,
		Magenta = 5,
		Brown = 6,
		LightGray = 7,
		DarkGray = 8,
		LightBlue = 9,
		LightGreen = 10,
		LightCyan = 11,
		LightRed = 12,
		LightMagenta = 13,
		Yellow = 14,
		White = 15,
	};
}

EQEmuLogSys::EQEmuLogSys(){
	on_log_gmsay_hook = [](uint16 log_type, std::string&) {};
}

EQEmuLogSys::~EQEmuLogSys(){
}

void EQEmuLogSys::LoadLogSettingsDefaults()
{
	log_platform = GetExecutablePlatformInt();
	/* Write defaults */
	for (int i = 0; i < Logs::LogCategory::MaxCategoryID; i++){
		log_settings[i].log_to_console = 0;
		log_settings[i].log_to_file = 0;
		log_settings[i].log_to_gmsay = 0;
	}
	log_settings_loaded = true;
}

std::string EQEmuLogSys::FormatOutMessageString(uint16 log_category, std::string in_message){
	std::string category_string = "";
	if (log_category > 0 && Logs::LogCategoryName[log_category]){
		category_string = StringFormat("[%s] ", Logs::LogCategoryName[log_category]);
	}
	return StringFormat("%s%s", category_string.c_str(), in_message.c_str()); 
}

void EQEmuLogSys::ProcessGMSay(uint16 log_category, std::string message)
{
	/* Check if category enabled for process */
	if (log_settings[log_category].log_to_gmsay == 0)
		return;

	/* Enabling Netcode based GMSay output creates a feedback loop that ultimately ends in a crash */
	if (log_category == Logs::LogCategory::Netcode)
		return;

	/* Check to see if the process that actually ran this is zone */
	if (EQEmuLogSys::log_platform == EQEmuExePlatform::ExePlatformZone){
		on_log_gmsay_hook(log_category, message);
	}
}

void EQEmuLogSys::ProcessLogWrite(uint16 log_category, std::string message)
{
	/* Check if category enabled for process */
	if (log_settings[log_category].log_to_file == 0)
		return;

	char time_stamp[80];
	EQEmuLogSys::SetCurrentTimeStamp(time_stamp);

	if (process_log){
		process_log << time_stamp << " " << StringFormat("[%s] ", Logs::LogCategoryName[log_category]).c_str() << message << std::endl;
	}
}

uint16 EQEmuLogSys::GetWindowsConsoleColorFromCategory(uint16 log_category){
	switch (log_category) {
		case Logs::Status:
		case Logs::Normal:
			return Console::Color::Yellow;
		case Logs::MySQLError:
		case Logs::Error:
			return Console::Color::LightRed;
		case Logs::MySQLQuery:
		case Logs::Debug:
			return Console::Color::LightGreen;
		case Logs::Quests:
			return Console::Color::LightCyan;
		case Logs::Commands:
			return Console::Color::LightMagenta;
		case Logs::Crash:	
			return Console::Color::LightRed;
		default:
			return Console::Color::Yellow;
	}
}

std::string EQEmuLogSys::GetLinuxConsoleColorFromCategory(uint16 log_category){
	switch (log_category) {
		case Logs::Status:
		case Logs::Normal:
			return LC_YELLOW;
		case Logs::MySQLError:
		case Logs::Error:
			return LC_RED;
		case Logs::MySQLQuery:
		case Logs::Debug:
			return LC_GREEN;
		case Logs::Quests:
			return LC_CYAN;
		case Logs::Commands:
			return LC_MAGENTA;
		case Logs::Crash:
			return LC_RED;
		default:
			return LC_YELLOW;
	}
}

uint16 EQEmuLogSys::GetGMSayColorFromCategory(uint16 log_category){
	switch (log_category) {
		case Logs::Status:
		case Logs::Normal:
			return 15; /* Yellow */
		case Logs::MySQLError:
		case Logs::Error:
			return 13; /* Red */
		case Logs::MySQLQuery:
		case Logs::Debug:
			return 14; /* Light Green */
		case Logs::Quests:
			return 258; /* Light Cyan */ 
		case Logs::Commands:
			return 5; /* Light Purple */
		case Logs::Crash:
			return 13; /* Red */
		default:
			return 15; /* Yellow */
	}
}

void EQEmuLogSys::ProcessConsoleMessage(uint16 log_category, const std::string message)
{
	/* Check if category enabled for process */
	if (log_settings[log_category].log_to_console == 0)
		return;

	#ifdef _WINDOWS
		HANDLE  console_handle;
		console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_FONT_INFOEX info = { 0 };
		info.cbSize = sizeof(info);
		info.dwFontSize.Y = 12; // leave X as zero
		info.FontWeight = FW_NORMAL;
		wcscpy(info.FaceName, L"Lucida Console");
		SetCurrentConsoleFontEx(console_handle, NULL, &info);
		SetConsoleTextAttribute(console_handle, EQEmuLogSys::GetWindowsConsoleColorFromCategory(log_category));
		std::cout << message << "\n";
		SetConsoleTextAttribute(console_handle, Console::Color::White);
	#else
		std::cout << EQEmuLogSys::GetLinuxConsoleColorFromCategory(log_category) << message << LC_RESET << std::endl;
	#endif
}

void EQEmuLogSys::Out(Logs::DebugLevel debug_level, uint16 log_category, std::string message, ...)
{
	va_list args;
	va_start(args, message);
	std::string output_message = vStringFormat(message.c_str(), args);
	va_end(args);

	std::string output_debug_message = EQEmuLogSys::FormatOutMessageString(log_category, output_message);

	EQEmuLogSys::ProcessConsoleMessage(log_category, output_debug_message);
	EQEmuLogSys::ProcessGMSay(log_category, output_debug_message);
	EQEmuLogSys::ProcessLogWrite(log_category, output_debug_message);
}

void EQEmuLogSys::SetCurrentTimeStamp(char* time_stamp){
	time_t raw_time;
	struct tm * time_info;
	time(&raw_time);
	time_info = localtime(&raw_time);
	strftime(time_stamp, 80, "[%d-%m-%Y :: %H:%M:%S]", time_info);
}

void EQEmuLogSys::MakeDirectory(std::string directory_name){
#ifdef _WINDOWS
	_mkdir(directory_name.c_str());
#else
	mkdir(directory_name.c_str(), 0755);
#endif
}

void EQEmuLogSys::CloseFileLogs()
{
	if (EQEmuLogSys::log_platform == EQEmuExePlatform::ExePlatformZone){
		std::cout << "Closing down zone logs..." << std::endl;
		process_log.close();
	}
}

void EQEmuLogSys::StartFileLogs(const std::string log_name)
{
	if (EQEmuLogSys::log_platform == EQEmuExePlatform::ExePlatformZone){
		std::cout << "Starting Zone Logs..." << std::endl;
		EQEmuLogSys::MakeDirectory("logs/zone");
		process_log.open(StringFormat("logs/zone/%s.txt", log_name.c_str()), std::ios_base::app | std::ios_base::out);
	}
}