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

#ifndef EQEMU_LOGSYS_H
#define EQEMU_LOGSYS_H

#include <iostream>
#include <fstream> 
#include <stdio.h>
#include <functional>

#include "types.h"

class EQEmuLogSys {
public:
	EQEmuLogSys();
	~EQEmuLogSys();

	enum LogType {
		Status = 0,	 /* This must stay the first entry in this list */
		Normal,		 /* Normal Logs */
		Error,		 /* Error Logs */
		Debug,		 /* Debug Logs */
		Quest,		 /* Quest Logs */
		Commands,	 /* Issued Commands */
		Crash,		 /* Crash Logs */
		MaxLogID	 /* Max, used in functions to get the max log ID */
	};

	enum DebugLevel {
		General = 0,	/* 0 - Low-Level general debugging, useful info on single line */
		Moderate,		/* 1 - Informational based, used in functions, when particular things load */
		Detail,			/* 2 - Use this for extreme detail in logging, usually in extreme debugging in the stack or interprocess communication */
	};

	/* If you add to this, make sure you update LogCategoryName in eqemu_logsys.cpp */
	enum LogCategory {
		Zone_Server = 0,
		World_Server,
		UCS_Server,
		QS_Server,
		WebInterface_Server,
		AA,
		Doors,
		Guilds,
		Inventory,
		Launcher,
		Netcode,
		Object,
		Rules,
		Skills,
		Spawns,
		Spells,
		Tasks,
		Trading,
		Tradeskills,
		Tribute,
		MaxCategoryID	/* Don't Remove this*/
	};

	void CloseFileLogs();
	
	void LoadLogSettings();
	void Log(uint16 log_type, const std::string message, ...);
	void LogDebug(DebugLevel debug_level, std::string message, ...);
	void LogDebugType(DebugLevel debug_level, uint16 log_category, std::string message, ...);
	void MakeDirectory(std::string directory_name);
	void SetCurrentTimeStamp(char* time_stamp);
	void StartFileLogs(const std::string log_name);

	struct LogSettings{
		uint8 log_to_file;
		uint8 log_to_console;
		uint8 log_to_gmsay;
	};

	LogSettings log_settings[EQEmuLogSys::LogCategory::MaxCategoryID];
	bool log_settings_loaded = false;
	int log_platform = 0;

	void OnLogHookCallBackZone(std::function<void(uint16 log_type, std::string&)> f) { on_log_gmsay_hook = f; }

private:
	bool zone_general_init = false;
	std::function<void(uint16 log_type, std::string&)> on_log_gmsay_hook;

	std::string FormatDebugCategoryMessageString(uint16 log_category, std::string in_message);

	void ProcessConsoleMessage(uint16 log_type, const std::string message);
	void ProcessGMSay(uint16 log_type, std::string message);
	void ProcessLogWrite(uint16 log_type, std::string message);
};

extern EQEmuLogSys logger;


#endif