/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2018 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
*/

#ifndef EQEMU_LOGSYS_H
#define EQEMU_LOGSYS_H

#include <fmt/format.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <functional>
#include "types.h"

namespace Logs {
	enum DebugLevel {
		General = 1,	/* 1 - Low-Level general debugging, useful info on single line */
		Moderate,		/* 2 - Informational based, used in functions, when particular things load */
		Detail			/* 3 - Use this for extreme detail in logging, usually in extreme debugging in the stack or interprocess communication */
	};

	/**
	 * If you add to this, make sure you update LogCategoryName
	 *
	 * NOTE: Only add to the bottom of the enum because that is the type ID assignment
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
		Mercenaries,
		QuestDebug,
		Server_Client_Packet,
		Client_Server_Packet_Unhandled,
		Server_Client_Packet_With_Dump,
		Client_Server_Packet_With_Dump,
		Login_Server,
		Client_Login,
		Headless_Client,
		HP_Update,
		FixZ,
		Food,
		Traps,
		NPCRoamBox,
		NPCScaling,
		MobAppearance,
		MaxCategoryID /* Don't Remove this */
	};

	/**
	 * If you add to this, make sure you update LogCategory
	 */
	static const char* LogCategoryName[LogCategory::MaxCategoryID] = {
		"",
		"AA",
		"AI",
		"Aggro",
		"Attack",
		"Packet :: Client -> Server",
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
		"Mercenaries",
		"Quest Debug",
		"Packet :: Server -> Client",
		"Packet :: Client -> Server Unhandled",
		"Packet :: Server -> Client (Dump)",
		"Packet :: Client -> Server (Dump)",
		"Login Server",
		"Client Login",
		"Headless Client",
		"HP Update",
		"FixZ",
		"Food",
		"Traps",
		"NPC Roam Box",
		"NPC Scaling",
		"Mob Appearance"
	};
}

#define Log(debug_level, log_category, message, ...) do {\
    if (LogSys.log_settings[log_category].is_category_enabled == 1)\
        LogSys.Out(debug_level, log_category, message, ##__VA_ARGS__);\
} while (0)

#define LogF(debug_level, log_category, message, ...) do {\
    if (LogSys.log_settings[log_category].is_category_enabled == 1)\
        LogSys.OutF(debug_level, log_category, message, ##__VA_ARGS__);\
} while (0)

class EQEmuLogSys {
public:
	EQEmuLogSys();
	~EQEmuLogSys();

	/**
	 * Close File Logs wherever necessary, either at zone shutdown or entire process shutdown for everything else.
	 * This should be handled on deconstructor but to be safe we use it anyways.
	 */
	void CloseFileLogs();
	void LoadLogSettingsDefaults();
	void MakeDirectory(const std::string &directory_name);

	/**
	 *	The one and only Logging function that uses a debug level as a parameter, as well as a log_category
	 *		log_category - defined in Logs::LogCategory::[]
	 *		log_category name resolution works by passing the enum int ID to Logs::LogCategoryName[category_id]
	 *
	 *	Example: EQEmuLogSys::Out(Logs::General, Logs::Guilds, "This guild has no leader present");
	 *		- This would pipe the same category and debug level to all output formats, but the internal memory reference of log_settings would
	 *			be checked against to see if that piped output is set to actually process it for the category and debug level
	*/
	void Out(Logs::DebugLevel debug_level, uint16 log_category, std::string message, ...);

	/**
	 * Used in file logs to prepend a timestamp entry for logs
	 */
	void SetCurrentTimeStamp(char* time_stamp);
	void StartFileLogs(const std::string &log_name = "");

	template <typename... Args>
	void OutF(Logs::DebugLevel debug_level, uint16 log_category, const char *fmt, const Args&... args)
	{
		std::string log_str = fmt::format(fmt, args...);
		Out(debug_level, log_category, log_str);
	}

	/**
     * LogSettings Struct
     *
     * This struct is the master reference for all settings for each category, and for each output
     *
     * log_to_file[category_id]    = [1-3] - Sets debug level for category to output to file
     * log_to_console[category_id] = [1-3] - Sets debug level for category to output to console
     * log_to_gmsay[category_id]   = [1-3] - Sets debug level for category to output to gmsay
     *
	*/
	struct LogSettings {
		uint8 log_to_file;
		uint8 log_to_console;
		uint8 log_to_gmsay;
		uint8 is_category_enabled; /* When any log output in a category > 0, set this to 1 as (Enabled) */
	};

	/**
	 * Internally used memory reference for all log settings per category
	 * These are loaded via DB and have defaults loaded in LoadLogSettingsDefaults
	 * Database loaded via Database::LoadLogSettings(log_settings)
	*/
	LogSettings log_settings[Logs::LogCategory::MaxCategoryID];

	bool file_logs_enabled;

	/**
	 * Sets Executable platform (Zone/World/UCS) etc.
	 */
	int log_platform;

	/**
	 * File name used in writing logs
	 */
	std::string platform_file_name;

	/**
	 * GMSay Client Message colors mapped by category
	 *
	 * @param log_category
	 * @return
	 */
	uint16 GetGMSayColorFromCategory(uint16 log_category);

	void OnLogHookCallBackZone(std::function<void(uint16 log_type, const std::string&)> f) { on_log_gmsay_hook = f; }

private:

	/**
	 * Callback pointer to zone process for hooking logs to zone using GMSay
	 */
	std::function<void(uint16 log_category, const std::string&)> on_log_gmsay_hook;

	/**
	 * Formats log messages like '[Category] This is a log message'
	 */
	std::string FormatOutMessageString(uint16 log_category, const std::string &in_message);

	/**
	 * Linux console color messages mapped by category
	 * @param log_category
	 * @return
	 */
	std::string GetLinuxConsoleColorFromCategory(uint16 log_category);

	/**
	 * Windows console color messages mapped by category
	 */
	uint16 GetWindowsConsoleColorFromCategory(uint16 log_category);

	void ProcessConsoleMessage(uint16 debug_level, uint16 log_category, const std::string &message);
	void ProcessGMSay(uint16 debug_level, uint16 log_category, const std::string &message);
	void ProcessLogWrite(uint16 debug_level, uint16 log_category, const std::string &message);
};

extern EQEmuLogSys LogSys;

#endif
