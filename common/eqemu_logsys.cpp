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

std::ofstream process_log;

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

static const char* TypeNames[EQEmuLogSys::MaxLogID] = { 
		"Status", 
		"Normal", 
		"Error", 
		"Debug", 
		"Quest", 
		"Command", 
		"Crash" 
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
}

EQEmuLogSys::~EQEmuLogSys(){
}

void EQEmuLogSys::StartZoneLogs(const std::string log_name)
{
	_mkdir("logs/zone");
	std::cout << "Starting Zone Logs..." << std::endl;
	process_log.open(StringFormat("logs/zone/%s.txt", log_name.c_str()), std::ios_base::app | std::ios_base::out);
}

void EQEmuLogSys::LogDebug(DebugLevel debug_level, std::string message, ...){
	if (RuleI(Logging, DebugLogLevel) < debug_level){ return;  }

	va_list args;
	va_start(args, message);
	std::string output_message = vStringFormat(message.c_str(), args);
	va_end(args);

	EQEmuLogSys::Log(EQEmuLogSys::LogType::Debug, output_message);
}

void EQEmuLogSys::Log(uint16 log_type, const std::string message, ...)
{
	if (log_type > EQEmuLogSys::MaxLogID){
		return;
	}
	if (!RuleB(Logging, LogFileCommands) && log_type == EQEmuLogSys::LogType::Commands){ return; }

	va_list args;
	va_start(args, message);
	std::string output_message = vStringFormat(message.c_str(), args);
	va_end(args);

	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);
	EQEmuLogSys::ConsoleMessage(log_type, message);

	if (process_log){
		process_log << std::put_time(&tm, "[%d-%m-%Y :: %H:%M:%S] ") << StringFormat("[%s] ", TypeNames[log_type]).c_str() << output_message << std::endl;
	}
	else{
		std::cout << "[DEBUG] " << ":: There currently is no log file open for this process " << std::endl;
	}
}

void EQEmuLogSys::ConsoleMessage(uint16 log_type, const std::string message)
{
	if (log_type > EQEmuLogSys::MaxLogID){
		return;
	}
	if (!RuleB(Logging, ConsoleLogCommands) && log_type == EQEmuLogSys::LogType::Commands){ return; }

#ifdef _WINDOWS
	HANDLE  console_handle;
	console_handle = GetStdHandle(STD_OUTPUT_HANDLE); 
	CONSOLE_FONT_INFOEX info = { 0 };
	info.cbSize = sizeof(info);
	info.dwFontSize.Y = 12; // leave X as zero
	info.FontWeight = FW_NORMAL;
	wcscpy(info.FaceName, L"Lucida Console");
	SetCurrentConsoleFontEx(console_handle, NULL, &info); 
	SetConsoleTextAttribute(console_handle, LogColors[log_type]); 
#endif

	std::cout << "[N::" << TypeNames[log_type] << "] " << message << std::endl;

#ifdef _WINDOWS
	/* Always set back to white*/
	SetConsoleTextAttribute(console_handle, Console::Color::White);
#endif
}

void EQEmuLogSys::CloseZoneLogs(){
	std::cout << "Closing down zone logs..." << std::endl;
	process_log.close();
}