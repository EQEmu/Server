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

#include "eqemu_logsys.h"
#include "rulesys.h"
#include "platform.h"
#include "strings.h"
#include "misc.h"
#include "discord/discord.h"
#include "repositories/discord_webhooks_repository.h"
#include "repositories/logsys_categories_repository.h"

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <time.h>
#include <sys/stat.h>
#include <algorithm>

std::ofstream process_log;

#ifdef _WINDOWS
#include <direct.h>
#include <conio.h>
#include <iostream>
#include <dos.h>
#include <windows.h>
#include <process.h>
#else

#include <unistd.h>
#include <sys/stat.h>
#include <thread>

#endif

/**
 * Linux ANSI console color defines
 */
#define LC_RESET   "\033[0m"
#define LC_BLACK   "\033[30m" /* Black */
#define LC_RED     "\033[31m" /* Red */
#define LC_GREEN   "\033[32m" /* Green */
#define LC_YELLOW  "\033[33m" /* Yellow */
#define LC_BLUE    "\033[34m" /* Blue */
#define LC_MAGENTA "\033[35m" /* Magenta */
#define LC_CYAN    "\033[36m" /* Cyan */
#define LC_WHITE   "\033[37m" /* White */

namespace Console {
	enum Color {
		Black        = 0,
		Blue         = 1,
		Green        = 2,
		Cyan         = 3,
		Red          = 4,
		Magenta      = 5,
		Brown        = 6,
		LightGray    = 7,
		DarkGray     = 8,
		LightBlue    = 9,
		LightGreen   = 10,
		LightCyan    = 11,
		LightRed     = 12,
		LightMagenta = 13,
		Yellow       = 14,
		White        = 15
	};
}

/**
 * EQEmuLogSys Constructor
 */
EQEmuLogSys::EQEmuLogSys()
{
	m_on_log_gmsay_hook   = [](uint16 log_type, const std::string &) {};
	m_on_log_console_hook = [](uint16 log_type, const std::string &) {};
}

/**
 * EQEmuLogSys Deconstructor
 */
EQEmuLogSys::~EQEmuLogSys() = default;

EQEmuLogSys *EQEmuLogSys::LoadLogSettingsDefaults()
{
	/**
	 * Get Executable platform currently running this code (Zone/World/etc)
	 */
	m_log_platform = GetExecutablePlatformInt();

	for (int log_category_id = Logs::AA; log_category_id != Logs::MaxCategoryID; log_category_id++) {
		log_settings[log_category_id].log_to_console      = 0;
		log_settings[log_category_id].log_to_file         = 0;
		log_settings[log_category_id].log_to_gmsay        = 0;
		log_settings[log_category_id].log_to_discord      = 0;
		log_settings[log_category_id].is_category_enabled = 0;
	}

	m_file_logs_enabled = false;

	/**
	 * Set Defaults
	 */
	log_settings[Logs::WorldServer].log_to_console          = static_cast<uint8>(Logs::General);
	log_settings[Logs::ZoneServer].log_to_console           = static_cast<uint8>(Logs::General);
	log_settings[Logs::QSServer].log_to_console             = static_cast<uint8>(Logs::General);
	log_settings[Logs::UCSServer].log_to_console            = static_cast<uint8>(Logs::General);
	log_settings[Logs::Crash].log_to_console                = static_cast<uint8>(Logs::General);
	log_settings[Logs::MySQLError].log_to_console           = static_cast<uint8>(Logs::General);
	log_settings[Logs::Loginserver].log_to_console          = static_cast<uint8>(Logs::General);
	log_settings[Logs::HeadlessClient].log_to_console       = static_cast<uint8>(Logs::General);
	log_settings[Logs::NPCScaling].log_to_gmsay             = static_cast<uint8>(Logs::General);
	log_settings[Logs::HotReload].log_to_gmsay              = static_cast<uint8>(Logs::General);
	log_settings[Logs::HotReload].log_to_console            = static_cast<uint8>(Logs::General);
	log_settings[Logs::Loot].log_to_gmsay                   = static_cast<uint8>(Logs::General);
	log_settings[Logs::Scheduler].log_to_console            = static_cast<uint8>(Logs::General);
	log_settings[Logs::Cheat].log_to_console                = static_cast<uint8>(Logs::General);
	log_settings[Logs::HTTP].log_to_console                 = static_cast<uint8>(Logs::General);
	log_settings[Logs::HTTP].log_to_gmsay                   = static_cast<uint8>(Logs::General);
	log_settings[Logs::ChecksumVerification].log_to_console = static_cast<uint8>(Logs::General);
	log_settings[Logs::ChecksumVerification].log_to_gmsay   = static_cast<uint8>(Logs::General);
	log_settings[Logs::CombatRecord].log_to_gmsay           = static_cast<uint8>(Logs::General);
	log_settings[Logs::Discord].log_to_console              = static_cast<uint8>(Logs::General);
	log_settings[Logs::QuestErrors].log_to_gmsay            = static_cast<uint8>(Logs::General);
	log_settings[Logs::QuestErrors].log_to_console          = static_cast<uint8>(Logs::General);

	/**
	 * RFC 5424
	 */
	log_settings[Logs::Emergency].log_to_console = static_cast<uint8>(Logs::General);
	log_settings[Logs::Alert].log_to_console     = static_cast<uint8>(Logs::General);
	log_settings[Logs::Critical].log_to_console  = static_cast<uint8>(Logs::General);
	log_settings[Logs::Error].log_to_console     = static_cast<uint8>(Logs::General);
	log_settings[Logs::Warning].log_to_console   = static_cast<uint8>(Logs::General);
	log_settings[Logs::Notice].log_to_console    = static_cast<uint8>(Logs::General);
	log_settings[Logs::Info].log_to_console      = static_cast<uint8>(Logs::General);

	/**
	 * Set Category enabled status on defaults
	 */
	for (int log_category_id = Logs::AA; log_category_id != Logs::MaxCategoryID; log_category_id++) {
		const bool log_to_console      = log_settings[log_category_id].log_to_console > 0;
		const bool log_to_file         = log_settings[log_category_id].log_to_file > 0;
		const bool log_to_gmsay        = log_settings[log_category_id].log_to_gmsay > 0;
		const bool log_to_discord      = log_settings[log_category_id].log_to_discord > 0;
		const bool is_category_enabled = log_to_console || log_to_file || log_to_gmsay || log_to_discord;
		if (is_category_enabled) {
			log_settings[log_category_id].is_category_enabled = 1;
		}
	}

	/**
	 * Declare process file names for log writing=
	 */
	if (EQEmuLogSys::m_log_platform == EQEmuExePlatform::ExePlatformWorld) {
		m_platform_file_name = "world";
	}
	else if (EQEmuLogSys::m_log_platform == EQEmuExePlatform::ExePlatformQueryServ) {
		m_platform_file_name = "query_server";
	}
	else if (EQEmuLogSys::m_log_platform == EQEmuExePlatform::ExePlatformZone) {
		m_platform_file_name = "zone";
	}
	else if (EQEmuLogSys::m_log_platform == EQEmuExePlatform::ExePlatformUCS) {
		m_platform_file_name = "ucs";
	}
	else if (EQEmuLogSys::m_log_platform == EQEmuExePlatform::ExePlatformLogin) {
		m_platform_file_name = "login";
	}
	else if (EQEmuLogSys::m_log_platform == EQEmuExePlatform::ExePlatformLaunch) {
		m_platform_file_name = "launcher";
	}
	else if (EQEmuLogSys::m_log_platform == EQEmuExePlatform::ExePlatformHC) {
		m_platform_file_name = "hc";
	}

	return this;
}

/**
 * @param log_category
 * @return
 */
bool EQEmuLogSys::IsRfc5424LogCategory(uint16 log_category)
{
	return (
		log_category == Logs::Emergency ||
		log_category == Logs::Alert ||
		log_category == Logs::Critical ||
		log_category == Logs::Error ||
		log_category == Logs::Warning ||
		log_category == Logs::Notice ||
		log_category == Logs::Info ||
		log_category == Logs::Debug
	);
}

/**
 * @param debug_level
 * @param log_category
 * @param message
 */
void EQEmuLogSys::ProcessLogWrite(
	uint16 log_category,
	const std::string &message
)
{
	if (log_category == Logs::Crash) {
		char time_stamp[80];
		EQEmuLogSys::SetCurrentTimeStamp(time_stamp);
		std::ofstream crash_log;
		EQEmuLogSys::MakeDirectory("logs/crashes");
		crash_log.open(
			StringFormat("logs/crashes/crash_%s_%i.log", m_platform_file_name.c_str(), getpid()),
			std::ios_base::app | std::ios_base::out
		);
		crash_log << time_stamp << " " << message << "\n";
		crash_log.close();
	}

	if (process_log) {
		char time_stamp[80];
		EQEmuLogSys::SetCurrentTimeStamp(time_stamp);
		process_log << time_stamp << " " << message << std::endl;
	}
}

/**
 * @param log_category
 * @return
 */
uint16 EQEmuLogSys::GetWindowsConsoleColorFromCategory(uint16 log_category)
{
	switch (log_category) {
		case Logs::Status:
		case Logs::Normal:
			return Console::Color::Yellow;
		case Logs::MySQLError:
		case Logs::Error:
		case Logs::QuestErrors:
			return Console::Color::LightRed;
		case Logs::MySQLQuery:
		case Logs::Debug:
			return Console::Color::LightGreen;
		case Logs::Quests:
			return Console::Color::LightCyan;
		case Logs::Commands:
		case Logs::Mercenaries:
			return Console::Color::LightMagenta;
		case Logs::Crash:
			return Console::Color::LightRed;
		default:
			return Console::Color::Yellow;
	}
}

/**
 * @param log_category
 * @return
 */
std::string EQEmuLogSys::GetLinuxConsoleColorFromCategory(uint16 log_category)
{
	switch (log_category) {
		case Logs::Status:
		case Logs::Normal:
			return LC_YELLOW;
		case Logs::MySQLError:
		case Logs::QuestErrors:
		case Logs::Warning:
		case Logs::Critical:
		case Logs::Error:
			return LC_RED;
		case Logs::MySQLQuery:
		case Logs::Debug:
			return LC_GREEN;
		case Logs::Quests:
			return LC_CYAN;
		case Logs::Commands:
		case Logs::Mercenaries:
			return LC_MAGENTA;
		case Logs::Crash:
			return LC_RED;
		default:
			return LC_YELLOW;
	}
}

/**
 * @param log_category
 * @return
 */
uint16 EQEmuLogSys::GetGMSayColorFromCategory(uint16 log_category)
{
	switch (log_category) {
		case Logs::Status:
		case Logs::Normal:
			return Chat::Yellow;
		case Logs::MySQLError:
		case Logs::QuestErrors:
		case Logs::Error:
			return Chat::Red;
		case Logs::MySQLQuery:
		case Logs::Debug:
			return Chat::Lime;
		case Logs::Quests:
			return Chat::Group;
		case Logs::Commands:
		case Logs::Mercenaries:
			return Chat::Magenta;
		case Logs::Crash:
			return Chat::Red;
		default:
			return Chat::Yellow;
	}
}

/**
 * @param debug_level
 * @param log_category
 * @param message
 */
void EQEmuLogSys::ProcessConsoleMessage(uint16 log_category, const std::string &message)
{
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

	m_on_log_console_hook(log_category, message);
}

/**
 * @param str
 * @return
 */
constexpr const char *str_end(const char *str)
{
	return *str ? str_end(str + 1) : str;
}

/**
 * @param str
 * @return
 */
constexpr bool str_slant(const char *str)
{
	return *str == '/' ? true : (*str ? str_slant(str + 1) : false);
}

/**
 * @param str
 * @return
 */
constexpr const char *r_slant(const char *str)
{
	return *str == '/' ? (str + 1) : r_slant(str - 1);
}

/**
 * @param str
 * @return
 */
constexpr const char *base_file_name(const char *str)
{
	return str_slant(str) ? r_slant(str_end(str)) : str;
}

/**
 * Core logging function
 *
 * @param debug_level
 * @param log_category
 * @param message
 * @param ...
 */
void EQEmuLogSys::Out(
	Logs::DebugLevel debug_level,
	uint16 log_category,
	const char *file,
	const char *func,
	int line,
	const char *message,
	...
)
{
	auto l = GetLogsEnabled(debug_level, log_category);

	// bail out if nothing to log
	if (!l.log_enabled) {
		return;
	}

	std::string prefix;
	if (RuleB(Logging, PrintFileFunctionAndLine)) {
		prefix = fmt::format("[{0}::{1}:{2}] ", base_file_name(file), func, line);
	}

	// remove this when we remove all legacy logs
	bool ignore_log_legacy_format = (
		log_category == Logs::Netcode ||
		log_category == Logs::PacketServerClient ||
		log_category == Logs::PacketClientServer ||
		log_category == Logs::PacketServerToServer
	);

	// remove this when we remove all legacy logs
	std::string output_message = message;
	if (!ignore_log_legacy_format) {
		va_list args;
		va_start(args, message);
		output_message = vStringFormat(message, args);
		va_end(args);
	}

	if (l.log_to_console_enabled) {
		EQEmuLogSys::ProcessConsoleMessage(
			log_category,
			fmt::format("[{}] [{}] {}", GetPlatformName(), Logs::LogCategoryName[log_category], prefix + output_message)
		);
	}
	if (l.log_to_gmsay_enabled) {
		m_on_log_gmsay_hook(log_category, output_message);
	}
	if (l.log_to_file_enabled) {
		EQEmuLogSys::ProcessLogWrite(
			log_category,
			fmt::format("[{}] [{}] {}", GetPlatformName(), Logs::LogCategoryName[log_category], prefix + output_message)
		);
	}
	if (l.log_to_discord_enabled && m_on_log_discord_hook) {
		m_on_log_discord_hook(log_category, log_settings[log_category].discord_webhook_id, output_message);
	}
}

/**
 * @param time_stamp
 */
void EQEmuLogSys::SetCurrentTimeStamp(char *time_stamp)
{
	time_t    raw_time;
	struct tm *time_info;
	time(&raw_time);
	time_info = localtime(&raw_time);
	strftime(time_stamp, 80, "[%m-%d-%Y %H:%M:%S]", time_info);
}

/**
 * @param directory_name
 */
void EQEmuLogSys::MakeDirectory(const std::string &directory_name)
{
#ifdef _WINDOWS
	struct _stat st;
	if (_stat(directory_name.c_str(), &st) == 0) // exists
		return;
	_mkdir(directory_name.c_str());
#else
	struct stat st{};
	if (stat(directory_name.c_str(), &st) == 0) { // exists
		return;
	}
	mkdir(directory_name.c_str(), 0755);
#endif
}

void EQEmuLogSys::CloseFileLogs()
{
	if (process_log.is_open()) {
		process_log.close();
	}
}

/**
 * @param log_name
 */
void EQEmuLogSys::StartFileLogs(const std::string &log_name)
{
	EQEmuLogSys::CloseFileLogs();

	/**
	 * When loading settings, we must have been given a reason in category based logging to output to a file in order to even create or open one...
	 */
	if (!m_file_logs_enabled) {
		return;
	}

	/**
	 * Zone
	 */
	if (EQEmuLogSys::m_log_platform == EQEmuExePlatform::ExePlatformZone) {
		if (!log_name.empty()) {
			m_platform_file_name = log_name;
		}

		if (m_platform_file_name.empty()) {
			return;
		}

		LogInfo("Starting File Log [{}/zone/{}_{}.log]", GetLogPath(), m_platform_file_name.c_str(), getpid());

		// Make directory if not exists
		EQEmuLogSys::MakeDirectory(fmt::format("{}/zone", GetLogPath()));

		// Open file pointer
		process_log.open(
			fmt::format("{}/zone/{}_{}.log", GetLogPath(), m_platform_file_name, getpid()),
			std::ios_base::app | std::ios_base::out
		);
	}
	else {

		// All other processes
		if (m_platform_file_name.empty()) {
			return;
		}

		LogInfo("Starting File Log [{}/{}_{}.log]", GetLogPath(), m_platform_file_name.c_str(), getpid());

		// Open file pointer
		process_log.open(
			fmt::format("{}/{}_{}.log", GetLogPath(), m_platform_file_name.c_str(), getpid()),
			std::ios_base::app | std::ios_base::out
		);
	}
}

/**
 * Silence console logging
 */
void EQEmuLogSys::SilenceConsoleLogging()
{
	for (int log_index = Logs::AA; log_index != Logs::MaxCategoryID; log_index++) {
		log_settings[log_index].log_to_console      = 0;
		log_settings[log_index].is_category_enabled = 0;
	}
}

/**
 * Enables console logging
 */
void EQEmuLogSys::EnableConsoleLogging()
{
	for (int log_index = Logs::AA; log_index != Logs::MaxCategoryID; log_index++) {
		log_settings[log_index].log_to_console      = Logs::General;
		log_settings[log_index].is_category_enabled = 1;
	}
}

EQEmuLogSys *EQEmuLogSys::LoadLogDatabaseSettings()
{
	InjectTablesIfNotExist();

	auto categories = LogsysCategoriesRepository::GetWhere(
		*m_database,
		"TRUE ORDER BY log_category_id"
	);

	// keep track of categories
	std::vector<int> db_categories{};
	db_categories.reserve(categories.size());

	// loop through database categories
	for (auto &c: categories) {
		if (c.log_category_id <= Logs::None || c.log_category_id >= Logs::MaxCategoryID) {
			continue;
		}

		log_settings[c.log_category_id].log_to_console     = static_cast<uint8>(c.log_to_console);
		log_settings[c.log_category_id].log_to_file        = static_cast<uint8>(c.log_to_file);
		log_settings[c.log_category_id].log_to_gmsay       = static_cast<uint8>(c.log_to_gmsay);
		log_settings[c.log_category_id].log_to_discord     = static_cast<uint8>(c.log_to_discord);
		log_settings[c.log_category_id].discord_webhook_id = c.discord_webhook_id;

		// Determine if any output method is enabled for the category
		// and set it to 1 so it can used to check if category is enabled
		const bool log_to_console      = log_settings[c.log_category_id].log_to_console > 0;
		const bool log_to_file         = log_settings[c.log_category_id].log_to_file > 0;
		const bool log_to_gmsay        = log_settings[c.log_category_id].log_to_gmsay > 0;
		const bool log_to_discord      = log_settings[c.log_category_id].log_to_discord > 0 &&
										 log_settings[c.log_category_id].discord_webhook_id > 0;
		const bool is_category_enabled = log_to_console || log_to_file || log_to_gmsay || log_to_discord;

		if (is_category_enabled) {
			log_settings[c.log_category_id].is_category_enabled = 1;
		}

		// This determines whether or not the process needs to actually file log anything.
		// If we go through this whole loop and nothing is set to any debug level, there
		// is no point to create a file or keep anything open
		if (log_settings[c.log_category_id].log_to_file > 0) {
			LogSys.m_file_logs_enabled = true;
		}

		db_categories.emplace_back(c.log_category_id);
	}

	// Auto inject categories that don't exist in the database...
	for (int i = Logs::AA; i != Logs::MaxCategoryID; i++) {

		bool is_missing_in_database = std::find(db_categories.begin(), db_categories.end(), i) == db_categories.end();
		bool is_deprecated_category = Strings::Contains(fmt::format("{}", Logs::LogCategoryName[i]), "Deprecated");
		if (!is_missing_in_database && is_deprecated_category) {
			LogInfo(
				"Logging category [{}] ({}) is now deprecated, deleting from database",
				Logs::LogCategoryName[i],
				i
			);
			LogsysCategoriesRepository::DeleteOne(*m_database, i);
		}

		if (is_missing_in_database && !is_deprecated_category) {
			LogInfo(
				"Automatically adding new log category [{}] ({})",
				Logs::LogCategoryName[i],
				i
			);

			auto new_category = LogsysCategoriesRepository::NewEntity();
			new_category.log_category_id          = i;
			new_category.log_category_description = Strings::Escape(Logs::LogCategoryName[i]);
			new_category.log_to_console           = log_settings[i].log_to_console;
			new_category.log_to_gmsay             = log_settings[i].log_to_gmsay;
			new_category.log_to_file              = log_settings[i].log_to_file;
			new_category.log_to_discord           = log_settings[i].log_to_discord;

			LogsysCategoriesRepository::InsertOne(*m_database, new_category);
		}
	}

	LogInfo("Loaded [{}] log categories", categories.size());

	auto webhooks = DiscordWebhooksRepository::GetWhere(*m_database, fmt::format("id < {}", MAX_DISCORD_WEBHOOK_ID));
	if (!webhooks.empty()) {
		for (auto &w: webhooks) {
			m_discord_webhooks[w.id] = {w.id, w.webhook_name, w.webhook_url};
		}
		LogInfo("Loaded [{}] Discord webhooks", webhooks.size());
	}

	return this;
}

EQEmuLogSys *EQEmuLogSys::SetDatabase(Database *db)
{
	m_database = db;

	return this;
}

void EQEmuLogSys::InjectTablesIfNotExist()
{
	// do not run injections for zone as its unnecessary hits every time a zone boots
	// other processes less frequently ran can pick up injection
	if (m_log_platform == EQEmuExePlatform::ExePlatformZone) {
		return;
	}

	// inject discord_webhooks
	if (!m_database->DoesTableExist("discord_webhooks")) {
		LogInfo("Creating table [discord_webhooks]");
		m_database->QueryDatabase(
			SQL(
				CREATE TABLE discord_webhooks
				(
					id INT auto_increment primary key NULL,
				webhook_name varchar(100) NULL,
				webhook_url varchar(255) NULL,
				created_at DATETIME NULL,
				deleted_at DATETIME NULL
			) ENGINE=InnoDB
				DEFAULT CHARSET=utf8mb4
				COLLATE=utf8mb4_general_ci;
			)
		);
	}

	// inject logsys_categories
	if (!m_database->DoesTableExist("logsys_categories")) {
		LogInfo("Creating table [logsys_categories]");
		m_database->QueryDatabase(
			SQL(
				CREATE TABLE `logsys_categories` (
				`log_category_id` int(11) NOT NULL,
				`log_category_description` varchar(150) DEFAULT NULL,
				`log_to_console` smallint(11) DEFAULT 0,
				`log_to_file` smallint(11) DEFAULT 0,
				`log_to_gmsay` smallint(11) DEFAULT 0,
				`log_to_discord` smallint(11) DEFAULT 0,
				`discord_webhook_id` int(11) DEFAULT 0,
				PRIMARY KEY (`log_category_id`)
			) ENGINE=InnoDB DEFAULT CHARSET=latin1
			)
		);
	}
}

const EQEmuLogSys::DiscordWebhooks *EQEmuLogSys::GetDiscordWebhooks() const
{
	return m_discord_webhooks;
}

EQEmuLogSys::LogEnabled EQEmuLogSys::GetLogsEnabled(const Logs::DebugLevel &debug_level, const uint16 &log_category)
{
	auto e = LogEnabled{};

	e.log_to_console_enabled = log_settings[log_category].log_to_console > 0 &&
							   log_settings[log_category].log_to_console >= debug_level;
	e.log_to_file_enabled    = log_settings[log_category].log_to_file > 0 &&
							   log_settings[log_category].log_to_file >= debug_level;
	e.log_to_gmsay_enabled   = log_settings[log_category].log_to_gmsay > 0 &&
							   log_settings[log_category].log_to_gmsay >= debug_level &&
							   log_category != Logs::LogCategory::Netcode &&
							   (EQEmuLogSys::m_log_platform == EQEmuExePlatform::ExePlatformZone ||
								EQEmuLogSys::m_log_platform == EQEmuExePlatform::ExePlatformWorld);
	e.log_to_discord_enabled = EQEmuLogSys::m_log_platform == EQEmuExePlatform::ExePlatformZone &&
							   log_settings[log_category].log_to_discord > 0 &&
							   log_settings[log_category].log_to_discord >= debug_level &&
							   log_settings[log_category].discord_webhook_id > 0 &&
							   log_settings[log_category].discord_webhook_id < MAX_DISCORD_WEBHOOK_ID;
	e.log_enabled =
		e.log_to_console_enabled || e.log_to_file_enabled || e.log_to_gmsay_enabled || e.log_to_discord_enabled;

	return e;
}

bool EQEmuLogSys::IsLogEnabled(const Logs::DebugLevel &debug_level, const uint16 &log_category)
{
	return GetLogsEnabled(debug_level, log_category).log_enabled;
}

const std::string &EQEmuLogSys::GetLogPath() const
{
	return m_log_path;
}

EQEmuLogSys *EQEmuLogSys::SetLogPath(const std::string &log_path)
{
	EQEmuLogSys::m_log_path = log_path;

	return this;
}

