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

static const char* TypeNames[EQEmuLogSys::MaxLogID] = { 
	"Status",
	"Normal",
	"Error",
	"Debug",
	"Quest",
	"Command",
	"Crash",
};

static Console::Color LogColors[EQEmuLogSys::MaxLogID] = {
	Console::Color::Yellow, 		   // "Status", 
	Console::Color::Yellow,			   // "Normal", 
	Console::Color::LightRed,		   // "Error", 
	Console::Color::LightGreen,		   // "Debug", 
	Console::Color::LightCyan,		   // "Quest", 
	Console::Color::LightMagenta,	   // "Command", 
	Console::Color::LightRed		   // "Crash" 
};


EQEmuLogSys::EQEmuLogSys(){
	on_log_gmsay_hook = [](uint16 log_type, std::string&) {};
}

EQEmuLogSys::~EQEmuLogSys(){
}

void EQEmuLogSys::LoadLogSettingsDefaults()
{
	log_platform = GetExecutablePlatformInt();
	/* Write defaults */
	for (int i = 0; i < EQEmuLogSys::LogCategory::MaxCategoryID; i++){
		log_settings[i].log_to_console = 0;
		log_settings[i].log_to_file = 0;
		log_settings[i].log_to_gmsay = 0;
	}
	log_settings_loaded = true;
}

std::string EQEmuLogSys::FormatDebugCategoryMessageString(uint16 log_category, std::string in_message){
	std::string category_string = "";
	if (log_category > 0 && LogCategoryName[log_category]){ 
		category_string = StringFormat("[%s] ", LogCategoryName[log_category]);
	}
	return StringFormat("%s%s", category_string.c_str(), in_message.c_str()); 
}

void EQEmuLogSys::ProcessGMSay(uint16 log_type, uint16 log_category, std::string message)
{
	/* Check if category enabled for process */
	if (log_settings[log_category].log_to_gmsay == 0)
		return;

	/* Enabling Netcode based GMSay output creates a feedback loop that ultimately ends in a crash */
	if (log_category == EQEmuLogSys::LogCategory::Netcode)
		return;

	if (EQEmuLogSys::log_platform == EQEmuExePlatform::ExePlatformZone){
		on_log_gmsay_hook(log_type, message);
	}
}

void EQEmuLogSys::ProcessLogWrite(uint16 log_type, uint16 log_category, std::string message)
{
	/* Check if category enabled for process */
	if (log_settings[log_category].log_to_file == 0)
		return;

	char time_stamp[80];
	EQEmuLogSys::SetCurrentTimeStamp(time_stamp);

	if (process_log){
		process_log << time_stamp << " " << StringFormat("[%s] ", TypeNames[log_type]).c_str() << message << std::endl;
	}
	else{
		// std::cout << "[DEBUG] " << ":: There currently is no log file open for this process " << "\n";
	}
}

void EQEmuLogSys::ProcessConsoleMessage(uint16 log_type, uint16 log_category, const std::string message)
{
	/* Check if category enabled for process */
	if (log_settings[log_category].log_to_console == 0)
		return;

	if (log_type > EQEmuLogSys::MaxLogID){
		return;
	}

	#ifdef _WINDOWS
		HANDLE  console_handle;
		console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_FONT_INFOEX info = { 0 };
		info.cbSize = sizeof(info);
		info.dwFontSize.Y = 12; // leave X as zero
		info.FontWeight = FW_NORMAL;
		wcscpy(info.FaceName, L"Lucida Console");
		SetCurrentConsoleFontEx(console_handle, NULL, &info);
		if (LogColors[log_type]){
			SetConsoleTextAttribute(console_handle, LogColors[log_type]);
		}
		else{
			SetConsoleTextAttribute(console_handle, Console::Color::White);
		}
	#endif

	std::cout << "[N::" << TypeNames[log_type] << "] " << message << "\n";

	#ifdef _WINDOWS
		/* Always set back to white*/
		SetConsoleTextAttribute(console_handle, Console::Color::White);
	#endif
}

void EQEmuLogSys::LogDebugType(DebugLevel debug_level, uint16 log_category, std::string message, ...)
{
	va_list args;
	va_start(args, message);
	std::string output_message = vStringFormat(message.c_str(), args);
	va_end(args);

	std::string output_debug_message = EQEmuLogSys::FormatDebugCategoryMessageString(log_category, output_message);

	EQEmuLogSys::ProcessConsoleMessage(EQEmuLogSys::Debug, log_category, output_debug_message);
	EQEmuLogSys::ProcessGMSay(EQEmuLogSys::Debug, log_category, output_debug_message);
	EQEmuLogSys::ProcessLogWrite(EQEmuLogSys::Debug, log_category, output_debug_message);
}

void EQEmuLogSys::LogDebug(DebugLevel debug_level, std::string message, ...)
{
	va_list args;
	va_start(args, message);
	std::string output_message = vStringFormat(message.c_str(), args);
	va_end(args);

	EQEmuLogSys::ProcessConsoleMessage(EQEmuLogSys::Debug, 0, output_message);
	EQEmuLogSys::ProcessGMSay(EQEmuLogSys::Debug, 0, output_message);
	EQEmuLogSys::ProcessLogWrite(EQEmuLogSys::Debug, 0, output_message);
}

void EQEmuLogSys::Log(uint16 log_type, const std::string message, ...)
{
	if (log_type > EQEmuLogSys::MaxLogID){
		return;
	}

	va_list args;
	va_start(args, message);
	std::string output_message = vStringFormat(message.c_str(), args);
	va_end(args);

	EQEmuLogSys::ProcessConsoleMessage(log_type, 0, output_message);
	EQEmuLogSys::ProcessGMSay(log_type, 0, output_message);
	EQEmuLogSys::ProcessLogWrite(log_type, 0, output_message);
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
	mkdir(directory_name.c_str());
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