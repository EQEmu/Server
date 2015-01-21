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

namespace Logs{
	enum DebugLevel {
		General = 1,	/* 1 - Low-Level general debugging, useful info on single line */
		Moderate,		/* 2 - Informational based, used in functions, when particular things load */
		Detail,			/* 3 - Use this for extreme detail in logging, usually in extreme debugging in the stack or interprocess communication */
	};

	/*
		If you add to this, make sure you update LogCategoryName
		NOTE: Only add to the bottom of the enum because that is the type ID assignment
	*/

	enum LogCategory {
		None = 0,
		AA,
		AI,
		Aggro,
		Attack,
		Client_Server_Packet,
		Combat,
		Commands,
		Crash,
		Debug,
		Doors,
		Error,
		Guilds,
		Inventory,
		Launcher,
		Netcode,
		Normal,
		Object,
		Pathing,
		QS_Server,
		Quests,
		Rules,
		Skills,
		Spawns,
		Spells,
		Status,
		TCP_Connection,
		Tasks,
		Tradeskills,
		Trading,
		Tribute,
		UCS_Server,
		WebInterface_Server,
		World_Server,
		Zone_Server,
		MySQLError,
		MySQLQuery,
		MaxCategoryID	/* Don't Remove this*/
	};

	/* If you add to this, make sure you update LogCategory */
	static const char* LogCategoryName[LogCategory::MaxCategoryID] = {
		"",
		"AA",
		"AI",
		"Aggro",
		"Attack",
		"Client Server Packet",
		"Combat",
		"Commands",
		"Crash",
		"Debug",
		"Doors",
		"Error",
		"Guilds",
		"Inventory",
		"Launcher",
		"Netcode",
		"Normal",
		"Object",
		"Pathing",
		"QS Server",
		"Quests",
		"Rules",
		"Skills",
		"Spawns",
		"Spells",
		"Status",
		"TCP Connection",
		"Tasks",
		"Tradeskills",
		"Trading",
		"Tribute",
		"UCS Server",
		"WebInterface Server",
		"World Server",
		"Zone Server",
		"MySQL Error",
		"MySQL Query",
	}; 
}

class EQEmuLogSys {
public:
	EQEmuLogSys();
	~EQEmuLogSys();

	void CloseFileLogs();
	void LoadLogSettingsDefaults();
	void MakeDirectory(std::string directory_name);
	void Out(Logs::DebugLevel debug_level, uint16 log_category, std::string message, ...);
	void SetCurrentTimeStamp(char* time_stamp);
	void StartFileLogs(const std::string log_name = "");

	struct LogSettings{
		uint8 log_to_file;
		uint8 log_to_console;
		uint8 log_to_gmsay;
	};

	LogSettings log_settings[Logs::LogCategory::MaxCategoryID];

	bool file_logs_enabled = false; /* Set when log settings are loaded to determine if keeping a file open is necessary */

	int log_platform = 0; /* Sets Executable platform (Zone/World/UCS) etc. */

	std::string platform_file_name; /* File name used in writing logs */ 

	uint16 GetGMSayColorFromCategory(uint16 log_category);

	void OnLogHookCallBackZone(std::function<void(uint16 log_type, std::string&)> f) { on_log_gmsay_hook = f; }

private:
	bool zone_general_init = false;
	
	std::function<void(uint16 log_category, std::string&)> on_log_gmsay_hook;
	std::string FormatOutMessageString(uint16 log_category, std::string in_message);
	std::string GetLinuxConsoleColorFromCategory(uint16 log_category);
	
	uint16 GetWindowsConsoleColorFromCategory(uint16 log_category);

	void ProcessConsoleMessage(uint16 debug_level, uint16 log_category, const std::string message);
	void ProcessGMSay(uint16 debug_level, uint16 log_category, std::string message);
	void ProcessLogWrite(uint16 debug_level, uint16 log_category, std::string message);
};

extern EQEmuLogSys Log;

#endif