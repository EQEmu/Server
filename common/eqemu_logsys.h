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

	enum DebugLevel {
		General = 0,	/* 0 - Low-Level general debugging, useful info on single line */
		Moderate,		/* 1 - Informational based, used in functions, when particular things load */
		Detail,			/* 2 - Use this for extreme detail in logging, usually in extreme debugging in the stack or interprocess communication */
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
		MaxCategoryID	/* Don't Remove this*/
	};

	void CloseFileLogs();
	void LoadLogSettingsDefaults();
	void DoLog(DebugLevel debug_level, uint16 log_category, std::string message, ...);
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
	std::function<void(uint16 log_category, std::string&)> on_log_gmsay_hook;

	std::string FormatDoLogMessageString(uint16 log_category, std::string in_message);

	void ProcessConsoleMessage(uint16 log_category, const std::string message);
	void ProcessGMSay(uint16 log_category, std::string message);
	void ProcessLogWrite(uint16 log_category, std::string message);
};

extern EQEmuLogSys Log;

/* If you add to this, make sure you update LogCategory */
static const char* LogCategoryName[EQEmuLogSys::LogCategory::MaxCategoryID] = {
	"",
	"AA",
	"AI",
	"Aggro",
	"Attack",
	"Client_Server_Packet",
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
	"QS_Server",
	"Quests",
	"Rules",
	"Skills",
	"Spawns",
	"Spells",
	"Status",
	"TCP_Connection",
	"Tasks",
	"Tradeskills",
	"Trading",
	"Tribute",
	"UCS_Server",
	"WebInterface_Server",
	"World_Server",
	"Zone_Server",
};

#endif