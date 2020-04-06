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
 *
 */

/**
 * This repository was automatically generated on Apr 5, 2020 and is NOT
 * to be modified directly. Any repository modifications are meant to be made to
 * the repository extending the base. Any modifications to base repositories are to
 * be made by the generator only
 */

#ifndef EQEMU_BASE_COMMAND_SETTINGS_REPOSITORY_H
#define EQEMU_BASE_COMMAND_SETTINGS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseCommandSettingsRepository {
public:
	struct CommandSettings {
		std::string command;
		int         access;
		std::string aliases;
	};

	static std::string PrimaryKey()
	{
		return std::string("command");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"command",
			"access",
			"aliases",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string InsertColumnsRaw()
	{
		std::vector<std::string> insert_columns;

		for (auto &column : Columns()) {
			if (column == PrimaryKey()) {
				continue;
			}

			insert_columns.push_back(column);
		}

		return std::string(implode(", ", insert_columns));
	}

	static std::string TableName()
	{
		return std::string("command_settings");
	}

	static std::string BaseSelect()
	{
		return fmt::format(
			"SELECT {} FROM {}",
			ColumnsRaw(),
			TableName()
		);
	}

	static std::string BaseInsert()
	{
		return fmt::format(
			"INSERT INTO {} ({}) ",
			TableName(),
			InsertColumnsRaw()
		);
	}

	static CommandSettings NewEntity()
	{
		CommandSettings entry{};

		entry.command = "";
		entry.access  = 0;
		entry.aliases = "";

		return entry;
	}

	static CommandSettings GetCommandSettingsEntry(
		const std::vector<CommandSettings> &command_settingss,
		int command_settings_id
	)
	{
		for (auto &command_settings : command_settingss) {
			if (command_settings.command == command_settings_id) {
				return command_settings;
			}
		}

		return NewEntity();
	}

	static CommandSettings FindOne(
		int command_settings_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				command_settings_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CommandSettings entry{};

			entry.command = row[0] ? row[0] : "";
			entry.access  = atoi(row[1]);
			entry.aliases = row[2] ? row[2] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int command_settings_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				command_settings_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		CommandSettings command_settings_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(command_settings_entry.access));
		update_values.push_back(columns[2] + " = '" + EscapeString(command_settings_entry.aliases) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				command_settings_entry.command
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CommandSettings InsertOne(
		CommandSettings command_settings_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(command_settings_entry.access));
		insert_values.push_back("'" + EscapeString(command_settings_entry.aliases) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			command_settings_entry.id = results.LastInsertedID();
			return command_settings_entry;
		}

		command_settings_entry = NewEntity();

		return command_settings_entry;
	}

	static int InsertMany(
		std::vector<CommandSettings> command_settings_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &command_settings_entry: command_settings_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(command_settings_entry.access));
			insert_values.push_back("'" + EscapeString(command_settings_entry.aliases) + "'");

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<CommandSettings> All()
	{
		std::vector<CommandSettings> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CommandSettings entry{};

			entry.command = row[0] ? row[0] : "";
			entry.access  = atoi(row[1]);
			entry.aliases = row[2] ? row[2] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CommandSettings> GetWhere(std::string where_filter)
	{
		std::vector<CommandSettings> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CommandSettings entry{};

			entry.command = row[0] ? row[0] : "";
			entry.access  = atoi(row[1]);
			entry.aliases = row[2] ? row[2] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static int DeleteWhere(std::string where_filter)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				PrimaryKey(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_COMMAND_SETTINGS_REPOSITORY_H
