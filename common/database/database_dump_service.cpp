/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
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

#include <string>
#include <cstdio>
#include <iterator>
#include "database_dump_service.h"
#include "../eqemu_logsys.h"
#include "../strings.h"
#include "../eqemu_config.h"
#include "../database_schema.h"
#include "../file.h"
#include "../process/process.h"

#include <ctime>

#if _WIN32
#include <windows.h>
#else

#include <sys/time.h>

#endif

#define DATABASE_DUMP_PATH "backups/"

/**
 * @return bool
 */
bool DatabaseDumpService::IsMySQLInstalled()
{
	std::string version_output = GetMySQLVersion();

	return version_output.find("mysql") != std::string::npos && version_output.find("Ver") != std::string::npos;
}

/**
 * Linux
 * @return bool
 */
bool DatabaseDumpService::IsTarAvailable()
{
	std::string version_output = Process::execute("tar --version");

	return version_output.find("GNU tar") != std::string::npos;
}

/**
 * Windows
 * @return bool
 */
bool DatabaseDumpService::Is7ZipAvailable()
{
	std::string version_output = Process::execute("7z --help");

	return version_output.find("7-Zip") != std::string::npos;
}

/**
 * @return
 */
bool DatabaseDumpService::HasCompressionBinary()
{
	return IsTarAvailable() || Is7ZipAvailable();
}

/**
 * @return
 */
std::string DatabaseDumpService::GetMySQLVersion()
{
	std::string version_output = Process::execute("mysql --version");

	return Strings::Trim(version_output);
}

/**
 * @return
 */
std::string DatabaseDumpService::GetBaseMySQLDumpCommand()
{
	auto config = EQEmuConfig::get();
	if (IsDumpContentTables() && !config->ContentDbHost.empty()) {
		return fmt::format(
			"mysqldump -u {} -p{} -h {} --port={} {}",
			config->ContentDbUsername,
			config->ContentDbPassword,
			config->ContentDbHost,
			config->ContentDbPort,
			config->ContentDbName
		);
	};

	return fmt::format(
		"mysqldump -u {} -p{} -h {} --port={} {}",
		config->DatabaseUsername,
		config->DatabasePassword,
		config->DatabaseHost,
		config->DatabasePort,
		config->DatabaseDB
	);
}

std::string DatabaseDumpService::GetPlayerTablesList()
{
	return Strings::Join(DatabaseSchema::GetPlayerTables(), " ");
}

std::string DatabaseDumpService::GetBotTablesList()
{
	return Strings::Join(DatabaseSchema::GetBotTables(), " ");
}

std::string DatabaseDumpService::GetMercTablesList()
{
	return Strings::Join(DatabaseSchema::GetMercTables(), " ");
}

std::string DatabaseDumpService::GetLoginTableList()
{
	return Strings::Join(DatabaseSchema::GetLoginTables(), " ");
}

std::string DatabaseDumpService::GetQueryServTables()
{
	return Strings::Join(DatabaseSchema::GetQueryServerTables(), " ");
}

std::string DatabaseDumpService::GetSystemTablesList()
{
	auto system_tables = DatabaseSchema::GetServerTables();
	auto version_tables = DatabaseSchema::GetVersionTables();

	system_tables.insert(
		std::end(system_tables),
		std::begin(version_tables),
		std::end(version_tables)
	);

	return Strings::Join(system_tables, " ");
}

std::string DatabaseDumpService::GetStateTablesList()
{
	return Strings::Join(DatabaseSchema::GetStateTables(), " ");
}

std::string DatabaseDumpService::GetContentTablesList()
{
	return Strings::Join(DatabaseSchema::GetContentTables(), " ");
}

/**
 * @return
 */
std::string GetDumpDate()
{

	time_t    now = time(nullptr);
	struct tm time_struct{};
	char      buf[80];
	time_struct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d", &time_struct);

	std::string time = buf;

	return time;
}

/**
 * @return
 */
std::string DatabaseDumpService::GetSetDumpPath()
{
	return !GetDumpPath().empty() ? GetDumpPath() : DATABASE_DUMP_PATH;
}
/**
 * @return
 */
std::string DatabaseDumpService::GetDumpFileNameWithPath()
{
	return GetSetDumpPath() + GetDumpFileName();
}

void DatabaseDumpService::Dump()
{
	if (!IsMySQLInstalled()) {
		LogError("MySQL is not installed; Please check your PATH for a valid MySQL installation");
		return;
	}

	if (IsDumpDropTableSyntaxOnly()) {
		SetDumpOutputToConsole(true);
	}

	if (IsDumpOutputToConsole()) {
		LogSys.SilenceConsoleLogging();
	}

	LogInfo("MySQL installed [{}]", GetMySQLVersion());

	SetDumpFileName(EQEmuConfig::get()->DatabaseDB + '-' + GetDumpDate());

	auto config = EQEmuConfig::get();

	LogInfo(
		"Database [{}] Host [{}] Username [{}]",
		config->DatabaseDB,
		config->DatabaseHost,
		config->DatabaseUsername
	);

	std::string options = "--allow-keywords --extended-insert --max-allowed-packet=1G --net-buffer-length=32704";
	if (IsDumpWithNoData()) {
		options += " --no-data";
	}

	if (!IsDumpTableLock()) {
		options += " --skip-lock-tables";
	}

	std::string tables_to_dump;
	std::string dump_descriptor;

	if (!IsDumpAllTables()) {
		if (IsDumpPlayerTables()) {
			tables_to_dump += GetPlayerTablesList() + " ";
			dump_descriptor += "-player";
		}

		if (IsDumpBotTables()) {
			tables_to_dump += GetBotTablesList() + " ";
			dump_descriptor += "-bots";
		}

		if (IsDumpMercTables()) {
			tables_to_dump += GetMercTablesList() + " ";
			dump_descriptor += "-mercs";
		}

		if (IsDumpSystemTables()) {
			tables_to_dump += GetSystemTablesList() + " ";
			dump_descriptor += "-system";
		}

		if (IsDumpStateTables()) {
			tables_to_dump += GetStateTablesList() + " ";
			dump_descriptor += "-state";
		}

		if (IsDumpContentTables()) {
			tables_to_dump += GetContentTablesList() + " ";
			dump_descriptor += "-content";
		}

		if (IsDumpLoginServerTables()) {
			tables_to_dump += GetLoginTableList() + " ";
			dump_descriptor += "-login";
		}

		if (IsDumpQueryServerTables()) {
			tables_to_dump += GetQueryServTables();
			dump_descriptor += "-queryserv";
		}
	}

	if (!dump_descriptor.empty()) {
		SetDumpFileName(GetDumpFileName() + dump_descriptor);
	}

	/**
	 * If we are dumping to stdout then we don't generate a file
	 */
	std::string pipe_file;
	if (!IsDumpOutputToConsole()) {
		pipe_file = fmt::format(" > {}.sql", GetDumpFileNameWithPath());
	}

	std::string execute_command = fmt::format(
		"{} {} {} {}",
		GetBaseMySQLDumpCommand(),
		options,
		tables_to_dump,
		pipe_file
	);

	if (!File::Exists(GetSetDumpPath()) && !IsDumpOutputToConsole()) {
		File::Makedir(GetSetDumpPath());
	}

	if (IsDumpDropTableSyntaxOnly()) {
		std::vector<std::string> tables = Strings::Split(tables_to_dump, ' ');

		for (auto &table : tables) {
			std::cout << "DROP TABLE IF EXISTS `" << table << "`;" << std::endl;
		}

		if (tables_to_dump.empty()) {
			std::cerr << "No tables were specified" << std::endl;
		}
	}
	else {
		std::string execution_result = Process::execute(execute_command);
		if (!execution_result.empty() && IsDumpOutputToConsole()) {
			std::cout << execution_result;
		}
	}

	if (!tables_to_dump.empty()) {
		LogInfo("Dumping Tables [{}]", Strings::Trim(tables_to_dump));
	}

	LogInfo("Database dump created at [{}.sql]", GetDumpFileNameWithPath());

	if (IsDumpWithCompression() && !IsDumpOutputToConsole()) {
		if (HasCompressionBinary()) {
			LogInfo("Compression requested... Compressing dump [{}.sql]", GetDumpFileNameWithPath());

			if (IsTarAvailable()) {
				Process::execute(
					fmt::format(
						"tar -zcvf {}.tar.gz -C {} {}.sql",
						GetDumpFileNameWithPath(),
						GetSetDumpPath(),
						GetDumpFileName()
					)
				);
				LogInfo("Compressed dump created at [{}.tar.gz]", GetDumpFileNameWithPath());
			}
			else if (Is7ZipAvailable()) {
				Process::execute(
					fmt::format(
						"7z a -t7z {}.zip {}.sql",
						GetDumpFileNameWithPath(),
						GetDumpFileNameWithPath()
					)
				);
				LogInfo("Compressed dump created at [{}.zip]", GetDumpFileNameWithPath());
			}
			else {
				LogInfo("Compression requested, but no available compression binary was found");
			}
		}
		else {
			LogWarning("Compression requested but binary not found... Skipping...");
		}
	}

//	LogDebug("[{}] dump-to-console", IsDumpOutputToConsole());
//	LogDebug("[{}] dump-path", GetSetDumpPath());
//	LogDebug("[{}] compression", (IsDumpWithCompression() ? "true" : "false"));
//	LogDebug("[{}] query-serv", (IsDumpQueryServerTables() ? "true" : "false"));
//	LogDebug("[{}] has-compression-binary", (HasCompressionBinary() ? "true" : "false"));
//	LogDebug("[{}] content", (IsDumpContentTables() ? "true" : "false"));
//	LogDebug("[{}] no-data", (IsDumpWithNoData() ? "true" : "false"));
//	LogDebug("[{}] login", (IsDumpLoginServerTables() ? "true" : "false"));
//	LogDebug("[{}] player", (IsDumpPlayerTables() ? "true" : "false"));
//	LogDebug("[{}] system", (IsDumpSystemTables() ? "true" : "false"));
//	LogDebug("[{}] bot", (IsDumpBotTables() ? "true" : "false"));
}

bool DatabaseDumpService::IsDumpSystemTables() const
{
	return dump_system_tables;
}

void DatabaseDumpService::SetDumpSystemTables(bool dump_system_tables)
{
	DatabaseDumpService::dump_system_tables = dump_system_tables;
}

bool DatabaseDumpService::IsDumpContentTables() const
{
	return dump_content_tables;
}

void DatabaseDumpService::SetDumpContentTables(bool dump_content_tables)
{
	DatabaseDumpService::dump_content_tables = dump_content_tables;
}

bool DatabaseDumpService::IsDumpPlayerTables() const
{
	return dump_player_tables;
}

void DatabaseDumpService::SetDumpPlayerTables(bool dump_player_tables)
{
	DatabaseDumpService::dump_player_tables = dump_player_tables;
}

bool DatabaseDumpService::IsDumpLoginServerTables() const
{
	return dump_login_server_tables;
}

void DatabaseDumpService::SetDumpLoginServerTables(bool dump_login_server_tables)
{
	DatabaseDumpService::dump_login_server_tables = dump_login_server_tables;
}

bool DatabaseDumpService::IsDumpWithNoData() const
{
	return dump_with_no_data;
}

void DatabaseDumpService::SetDumpWithNoData(bool dump_with_no_data)
{
	DatabaseDumpService::dump_with_no_data = dump_with_no_data;
}

bool DatabaseDumpService::IsDumpAllTables() const
{
	return dump_all_tables;
}

void DatabaseDumpService::SetDumpAllTables(bool dump_all_tables)
{
	DatabaseDumpService::dump_all_tables = dump_all_tables;
}

bool DatabaseDumpService::IsDumpTableLock() const
{
	return dump_table_lock;
}

void DatabaseDumpService::SetDumpTableLock(bool dump_table_lock)
{
	DatabaseDumpService::dump_table_lock = dump_table_lock;
}

bool DatabaseDumpService::IsDumpWithCompression() const
{
	return dump_with_compression;
}

void DatabaseDumpService::SetDumpWithCompression(bool dump_with_compression)
{
	DatabaseDumpService::dump_with_compression = dump_with_compression;
}

const std::string &DatabaseDumpService::GetDumpPath() const
{
	return dump_path;
}

void DatabaseDumpService::SetDumpPath(const std::string &dump_path)
{
	DatabaseDumpService::dump_path = dump_path;
}

void DatabaseDumpService::SetDumpFileName(const std::string &dump_file_name)
{
	DatabaseDumpService::dump_file_name = dump_file_name;
}

const std::string &DatabaseDumpService::GetDumpFileName() const
{
	return dump_file_name;
}

bool DatabaseDumpService::IsDumpQueryServerTables() const
{
	return dump_query_server_tables;
}

void DatabaseDumpService::SetDumpQueryServerTables(bool dump_query_server_tables)
{
	DatabaseDumpService::dump_query_server_tables = dump_query_server_tables;
}

bool DatabaseDumpService::IsDumpOutputToConsole() const
{
	return dump_output_to_console;
}

void DatabaseDumpService::SetDumpOutputToConsole(bool dump_output_to_console)
{
	DatabaseDumpService::dump_output_to_console = dump_output_to_console;
}

bool DatabaseDumpService::IsDumpDropTableSyntaxOnly() const
{
	return dump_drop_table_syntax_only;
}

void DatabaseDumpService::SetDumpDropTableSyntaxOnly(bool dump_drop_table_syntax_only)
{
	DatabaseDumpService::dump_drop_table_syntax_only = dump_drop_table_syntax_only;
}

bool DatabaseDumpService::IsDumpStateTables() const
{
	return dump_state_tables;
}

void DatabaseDumpService::SetDumpStateTables(bool dump_state_tables)
{
	DatabaseDumpService::dump_state_tables = dump_state_tables;
}

bool DatabaseDumpService::IsDumpBotTables() const
{
	return dump_bot_tables;
}

void DatabaseDumpService::SetDumpBotTables(bool dump_bot_tables)
{
	DatabaseDumpService::dump_bot_tables = dump_bot_tables;
}

bool DatabaseDumpService::IsDumpMercTables() const
{
	return dump_merc_tables;
}

void DatabaseDumpService::SetDumpMercTables(bool dump_merc_tables)
{
	DatabaseDumpService::dump_merc_tables = dump_merc_tables;
}
