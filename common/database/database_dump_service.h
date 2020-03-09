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

#ifndef EQEMU_DATABASE_DUMP_SERVICE_H
#define EQEMU_DATABASE_DUMP_SERVICE_H


class DatabaseDumpService {
public:
	void Dump();
	bool IsDumpAllTables() const;
	void SetDumpAllTables(bool dump_all_tables);
	bool IsDumpWithNoData() const;
	void SetDumpWithNoData(bool dump_with_no_data);
	bool IsDumpSystemTables() const;
	void SetDumpSystemTables(bool dump_system_tables);
	bool IsDumpContentTables() const;
	void SetDumpContentTables(bool dump_content_tables);
	bool IsDumpPlayerTables() const;
	void SetDumpPlayerTables(bool dump_player_tables);
	bool IsDumpLoginServerTables() const;
	void SetDumpLoginServerTables(bool dump_login_server_tables);
	bool IsDumpNoTableLock() const;
	void SetDumpNoTableLock(bool dump_no_table_lock);
	bool IsDumpWithCompression() const;
	void SetDumpWithCompression(bool dump_with_compression);
	const std::string &GetDumpPath() const;
	void SetDumpPath(const std::string &dump_path);
	const std::string &GetDumpFileName() const;
	void SetDumpFileName(const std::string &dump_file_name);

private:
	bool dump_all_tables;
	bool dump_system_tables;
	bool dump_content_tables;
	bool dump_player_tables;
	bool dump_login_server_tables;
	bool dump_with_no_data;
	bool dump_no_table_lock;
	bool dump_with_compression;
	std::string dump_path;
	std::string dump_file_name;

	std::string execute(const std::string &cmd, bool return_result);
	bool IsMySQLInstalled();
	std::string GetMySQLVersion();
	std::string GetBaseMySQLDumpCommand();
	std::string GetPlayerTablesList();
	std::string GetSystemTablesList();
	std::string GetContentTablesList();
	std::string GetLoginTableList();
	bool IsTarAvailable();
	bool IsRarAvailable();
	bool HasCompressionBinary();
	std::string GetDumpFileNameWithPath();
	std::string GetSetDumpPath();
};


#endif //EQEMU_DATABASE_DUMP_SERVICE_H
