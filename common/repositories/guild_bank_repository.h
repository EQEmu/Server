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

#ifndef EQEMU_GUILD_BANK_REPOSITORY_H
#define EQEMU_GUILD_BANK_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class GuildBankRepository {
public:
	struct GuildBank {
		int         guildid;
		int8        area;
		int         slot;
		int         itemid;
		int         qty;
		std::string donator;
		int8        permissions;
		std::string whofor;
	};

	static std::string PrimaryKey()
	{
		return std::string("");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"guildid",
			"area",
			"slot",
			"itemid",
			"qty",
			"donator",
			"permissions",
			"whofor",
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
		return std::string("guild_bank");
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

	static GuildBank NewEntity()
	{
		GuildBank entry{};

		entry.guildid     = 0;
		entry.area        = 0;
		entry.slot        = 0;
		entry.itemid      = 0;
		entry.qty         = 0;
		entry.donator     = 0;
		entry.permissions = 0;
		entry.whofor      = 0;

		return entry;
	}

	static GuildBank GetGuildBankEntry(
		const std::vector<GuildBank> &guild_banks,
		int guild_bank_id
	)
	{
		for (auto &guild_bank : guild_banks) {
			if (guild_bank. == guild_bank_id) {
				return guild_bank;
			}
		}

		return NewEntity();
	}

	static GuildBank FindOne(
		int guild_bank_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				guild_bank_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			GuildBank entry{};

			entry.guildid     = atoi(row[0]);
			entry.area        = atoi(row[1]);
			entry.slot        = atoi(row[2]);
			entry.itemid      = atoi(row[3]);
			entry.qty         = atoi(row[4]);
			entry.donator     = row[5];
			entry.permissions = atoi(row[6]);
			entry.whofor      = row[7];

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int guild_bank_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				guild_bank_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		GuildBank guild_bank_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(guild_bank_entry.guildid));
		update_values.push_back(columns[1] + " = " + std::to_string(guild_bank_entry.area));
		update_values.push_back(columns[2] + " = " + std::to_string(guild_bank_entry.slot));
		update_values.push_back(columns[3] + " = " + std::to_string(guild_bank_entry.itemid));
		update_values.push_back(columns[4] + " = " + std::to_string(guild_bank_entry.qty));
		update_values.push_back(columns[5] + " = '" + EscapeString(guild_bank_entry.donator) + "'");
		update_values.push_back(columns[6] + " = " + std::to_string(guild_bank_entry.permissions));
		update_values.push_back(columns[7] + " = '" + EscapeString(guild_bank_entry.whofor) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				guild_bank_entry.
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static GuildBank InsertOne(
		GuildBank guild_bank_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(guild_bank_entry.guildid));
		insert_values.push_back(std::to_string(guild_bank_entry.area));
		insert_values.push_back(std::to_string(guild_bank_entry.slot));
		insert_values.push_back(std::to_string(guild_bank_entry.itemid));
		insert_values.push_back(std::to_string(guild_bank_entry.qty));
		insert_values.push_back("'" + EscapeString(guild_bank_entry.donator) + "'");
		insert_values.push_back(std::to_string(guild_bank_entry.permissions));
		insert_values.push_back("'" + EscapeString(guild_bank_entry.whofor) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			guild_bank_entry.id = results.LastInsertedID();
			return guild_bank_entry;
		}

		guild_bank_entry = InstanceListRepository::NewEntity();

		return guild_bank_entry;
	}

	static int InsertMany(
		std::vector<GuildBank> guild_bank_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &guild_bank_entry: guild_bank_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(guild_bank_entry.guildid));
			insert_values.push_back(std::to_string(guild_bank_entry.area));
			insert_values.push_back(std::to_string(guild_bank_entry.slot));
			insert_values.push_back(std::to_string(guild_bank_entry.itemid));
			insert_values.push_back(std::to_string(guild_bank_entry.qty));
			insert_values.push_back("'" + EscapeString(guild_bank_entry.donator) + "'");
			insert_values.push_back(std::to_string(guild_bank_entry.permissions));
			insert_values.push_back("'" + EscapeString(guild_bank_entry.whofor) + "'");

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

	static std::vector<GuildBank> All()
	{
		std::vector<GuildBank> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GuildBank entry{};

			entry.guildid     = atoi(row[0]);
			entry.area        = atoi(row[1]);
			entry.slot        = atoi(row[2]);
			entry.itemid      = atoi(row[3]);
			entry.qty         = atoi(row[4]);
			entry.donator     = row[5];
			entry.permissions = atoi(row[6]);
			entry.whofor      = row[7];

			all_entries.push_back(entry);
		}

		return all_entries;
	}

};

#endif //EQEMU_GUILD_BANK_REPOSITORY_H
