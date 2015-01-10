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
#include "string_util.h"
#include "rulesys.h"

#include <iostream>
#include <fstream> 
#include <string>
#include <iomanip>
#include <direct.h>

std::ofstream zone_general_log;

static const char* TypeNames[EQEmuLogSys::MaxLogID] = { "Status", "Normal", "Error", "Debug", "Quest", "Command", "Crash"};

#ifdef _WINDOWS
#include <conio.h>
#include <iostream>
#include <dos.h>
#include <windows.h>
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

EQEmuLogSys::EQEmuLogSys(){
}

EQEmuLogSys::~EQEmuLogSys(){
}

void EQEmuLogSys::StartZoneLogs(const std::string log_name)
{
	_mkdir("logs/zone");
	std::cout << "Starting Zone Logs..." << std::endl;
	zone_general_log.open(StringFormat("logs/zone/%s.txt", log_name.c_str()), std::ios_base::app | std::ios_base::out);
}

void EQEmuLogSys::WriteZoneLog(uint16 log_type, const std::string message)
{
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);
	EQEmuLogSys::ConsoleMessage(log_type, message);
	zone_general_log << std::put_time(&tm, "[%d-%m-%Y :: %H:%M:%S] ") << message << std::endl;
}

void EQEmuLogSys::ConsoleMessage(uint16 log_type, const std::string message){
#ifdef _WINDOWS
	HANDLE  console_handle;
	console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_FONT_INFOEX info = { 0 };
	info.cbSize = sizeof(info);
	info.dwFontSize.Y = 12; // leave X as zero
	info.FontWeight = FW_NORMAL;
	wcscpy(info.FaceName, L"Lucida Console");
	SetCurrentConsoleFontEx(console_handle, NULL, &info);

	if (log_type == EQEmuLogSys::LogType::Status){ SetConsoleTextAttribute(console_handle, Console::Color::Yellow); }
	if (log_type == EQEmuLogSys::LogType::Error){ SetConsoleTextAttribute(console_handle, Console::Color::LightRed); }
	if (log_type == EQEmuLogSys::LogType::Normal){ SetConsoleTextAttribute(console_handle, Console::Color::LightGreen); }
	if (log_type == EQEmuLogSys::LogType::Debug){ SetConsoleTextAttribute(console_handle, Console::Color::Yellow); }
	if (log_type == EQEmuLogSys::LogType::Quest){ SetConsoleTextAttribute(console_handle, Console::Color::LightCyan); }
	if (log_type == EQEmuLogSys::LogType::Commands){ SetConsoleTextAttribute(console_handle, Console::Color::LightMagenta); }
	if (log_type == EQEmuLogSys::LogType::Crash){ SetConsoleTextAttribute(console_handle, Console::Color::LightRed); }
#endif
	std::cout << "[" << TypeNames[log_type] << "] " << message << std::endl;
#ifdef _WINDOWS
	/* Always set back to white*/
	SetConsoleTextAttribute(console_handle, Console::Color::White);
#endif
}

void EQEmuLogSys::CloseZoneLogs(){
	std::cout << "Closing down zone logs..." << std::endl;
	zone_general_log.close();
}