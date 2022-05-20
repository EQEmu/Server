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

#include <iostream>
#include <fstream>
#include <cstdio>
#include <functional>
#include <algorithm>

#ifdef _WIN32
#ifdef utf16_to_utf8
#undef utf16_to_utf8
#endif
#endif

#include <fmt/format.h>
#include "types.h"

namespace Logs {
	enum DebugLevel {
		General = 1,    // 1 - Low-Level general debugging, useful info on single line
		Moderate,       // 2 - Informational based, used in functions, when particular things load
		Detail          // 3 - Use this for extreme detail in logging, usually in extreme debugging in the stack or interprocess communication
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
		PacketClientServer,
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
		QSServer,
		Quests,
		Rules,
		Skills,
		Spawns,
		Spells,
		Status,
		TCPConnection,
		Tasks,
		Tradeskills,
		Trading,
		Tribute,
		UCSServer,
		WebInterfaceServer,
		WorldServer,
		ZoneServer,
		MySQLError,
		MySQLQuery,
		Mercenaries,
		QuestDebug,
		PacketServerClient,
		PacketClientServerUnhandled,
		PacketServerClientWithDump,
		PacketClientServerWithDump,
		Loginserver,
		ClientLogin,
		HeadlessClient,
		HPUpdate,
		FixZ,
		Food,
		Traps,
		NPCRoamBox,
		NPCScaling,
		MobAppearance,
		Info,
		Warning,
		Critical,
		Emergency,
		Alert,
		Notice,
		AIScanClose,
		AIYellForHelp,
		AICastBeneficialClose,
		AoeCast,
		EntityManagement,
		Flee,
		Aura,
		HotReload,
		Merchants,
		ZonePoints,
		Loot,
		Expeditions,
		DynamicZones,
		Scheduler,
		Cheat,
		ClientList,
		DiaWind,
		HTTP,
		Saylink,
		ChecksumVerification,
		CombatRecord,
		Hate,
		MaxCategoryID /* Don't Remove this */
	};

	/**
	 * If you add to this, make sure you update LogCategory
	 */
	static const char *LogCategoryName[LogCategory::MaxCategoryID] = {
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
		"Mob Appearance",
		"Info",
		"Warning",
		"Critical",
		"Emergency",
		"Alert",
		"Notice",
		"AI Scan Close",
		"AI Yell For Help",
		"AI Cast Beneficial Close",
		"AOE Cast",
		"Entity Management",
		"Flee",
		"Aura",
		"HotReload",
		"Merchants",
		"ZonePoints",
		"Loot",
		"Expeditions",
		"DynamicZones",
		"Scheduler",
		"Cheat",
		"ClientList",
		"DialogueWindow",
		"HTTP",
		"Saylink",
		"ChecksumVerification",
		"CombatRecord",
		"Hate",
	};
}

#include "eqemu_logsys_log_aliases.h"

class Database;

class EQEmuLogSys {
public:
	EQEmuLogSys();
	~EQEmuLogSys();

	/**
	 * Close File Logs wherever necessary, either at zone shutdown or entire process shutdown for everything else.
	 * This should be handled on deconstructor but to be safe we use it anyways.
	 */
	void CloseFileLogs();
	EQEmuLogSys *LoadLogSettingsDefaults();
	EQEmuLogSys *LoadLogDatabaseSettings();

	/**
	 * @param directory_name
	 */
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
	void Out(
		Logs::DebugLevel debug_level,
		uint16 log_category,
		const char *file,
		const char *func,
		int line,
		const char *message,
		...
	);

	/**
	 * Used in file logs to prepend a timestamp entry for logs
	 * @param time_stamp
	 */
	void SetCurrentTimeStamp(char *time_stamp);

	/**
	 * @param log_name
	 */
	void StartFileLogs(const std::string &log_name = "");

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
	 * Database loaded via LogSys.SetDatabase(&database)->LoadLogDatabaseSettings();
	*/
	LogSettings log_settings[Logs::LogCategory::MaxCategoryID]{};

	bool file_logs_enabled = false;

	int                                                                               log_platform = 0;
	std::string                                                                       platform_file_name;


	// gmsay
	uint16 GetGMSayColorFromCategory(uint16 log_category);

	EQEmuLogSys * SetGMSayHandler(std::function<void(uint16 log_type, const std::string &)> f) {
		on_log_gmsay_hook = f;
		return this;
	}

	// console
	void SetConsoleHandler(
		std::function<void(
			uint16 debug_level,
			uint16 log_type,
			const std::string &
		)> f
	) { on_log_console_hook = f; }
	void SilenceConsoleLogging();
	void EnableConsoleLogging();

	// database
	EQEmuLogSys *SetDatabase(Database *db);

private:

	// reference to database
	Database *m_database;

	std::function<void(uint16 log_category, const std::string &)>                     on_log_gmsay_hook;
	std::function<void(uint16 debug_level, uint16 log_category, const std::string &)> on_log_console_hook;

	std::string FormatOutMessageString(uint16 log_category, const std::string &in_message);
	std::string GetLinuxConsoleColorFromCategory(uint16 log_category);
	uint16 GetWindowsConsoleColorFromCategory(uint16 log_category);

	void ProcessConsoleMessage(uint16 debug_level, uint16 log_category, const std::string &message);
	void ProcessGMSay(uint16 debug_level, uint16 log_category, const std::string &message);
	void ProcessLogWrite(uint16 debug_level, uint16 log_category, const std::string &message);
	bool IsRfc5424LogCategory(uint16 log_category);
};

extern EQEmuLogSys LogSys;

/**
template<typename... Args>
void OutF(
	EQEmuLogSys &ls,
	Logs::DebugLevel debug_level,
	uint16 log_category,
	const char *file,
	const char *func,
	int line,
	const char *fmt,
	const Args &... args
)
{
	std::string log_str = fmt::format(fmt, args...);
	ls.Out(debug_level, log_category, file, func, line, log_str.c_str());
}
 **/

#define OutF(ls, debug_level, log_category, file, func, line, formatStr, ...) \
do { \
    ls.Out(debug_level, log_category, file, func, line, fmt::format(formatStr, ##__VA_ARGS__).c_str()); \
} while(0)

#endif
