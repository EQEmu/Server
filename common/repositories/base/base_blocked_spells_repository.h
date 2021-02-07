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
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to
 * the repository extending the base. Any modifications to base repositories are to
 * be made by the generator only
 */

#ifndef EQEMU_BASE_BLOCKED_SPELLS_REPOSITORY_H
#define EQEMU_BASE_BLOCKED_SPELLS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseBlockedSpellsRepository {
public:
	struct BlockedSpells {
		int         id;
		int         spellid;
		int         type;
		int         zoneid;
		float       x;
		float       y;
		float       z;
		float       x_diff;
		float       y_diff;
		float       z_diff;
		std::string message;
		std::string description;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"spellid",
			"type",
			"zoneid",
			"x",
			"y",
			"z",
			"x_diff",
			"y_diff",
			"z_diff",
			"message",
			"description",
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
		return std::string("blocked_spells");
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

	static BlockedSpells NewEntity()
	{
		BlockedSpells entry{};

		entry.id          = 0;
		entry.spellid     = 0;
		entry.type        = 0;
		entry.zoneid      = 0;
		entry.x           = 0;
		entry.y           = 0;
		entry.z           = 0;
		entry.x_diff      = 0;
		entry.y_diff      = 0;
		entry.z_diff      = 0;
		entry.message     = "";
		entry.description = "";

		return entry;
	}

	static BlockedSpells GetBlockedSpellsEntry(
		const std::vector<BlockedSpells> &blocked_spellss,
		int blocked_spells_id
	)
	{
		for (auto &blocked_spells : blocked_spellss) {
			if (blocked_spells.id == blocked_spells_id) {
				return blocked_spells;
			}
		}

		return NewEntity();
	}

	static BlockedSpells FindOne(
		Database& db,
		int blocked_spells_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				blocked_spells_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			BlockedSpells entry{};

			entry.id          = atoi(row[0]);
			entry.spellid     = atoi(row[1]);
			entry.type        = atoi(row[2]);
			entry.zoneid      = atoi(row[3]);
			entry.x           = static_cast<float>(atof(row[4]));
			entry.y           = static_cast<float>(atof(row[5]));
			entry.z           = static_cast<float>(atof(row[6]));
			entry.x_diff      = static_cast<float>(atof(row[7]));
			entry.y_diff      = static_cast<float>(atof(row[8]));
			entry.z_diff      = static_cast<float>(atof(row[9]));
			entry.message     = row[10] ? row[10] : "";
			entry.description = row[11] ? row[11] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int blocked_spells_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				blocked_spells_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		BlockedSpells blocked_spells_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(blocked_spells_entry.spellid));
		update_values.push_back(columns[2] + " = " + std::to_string(blocked_spells_entry.type));
		update_values.push_back(columns[3] + " = " + std::to_string(blocked_spells_entry.zoneid));
		update_values.push_back(columns[4] + " = " + std::to_string(blocked_spells_entry.x));
		update_values.push_back(columns[5] + " = " + std::to_string(blocked_spells_entry.y));
		update_values.push_back(columns[6] + " = " + std::to_string(blocked_spells_entry.z));
		update_values.push_back(columns[7] + " = " + std::to_string(blocked_spells_entry.x_diff));
		update_values.push_back(columns[8] + " = " + std::to_string(blocked_spells_entry.y_diff));
		update_values.push_back(columns[9] + " = " + std::to_string(blocked_spells_entry.z_diff));
		update_values.push_back(columns[10] + " = '" + EscapeString(blocked_spells_entry.message) + "'");
		update_values.push_back(columns[11] + " = '" + EscapeString(blocked_spells_entry.description) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				blocked_spells_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static BlockedSpells InsertOne(
		Database& db,
		BlockedSpells blocked_spells_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(blocked_spells_entry.spellid));
		insert_values.push_back(std::to_string(blocked_spells_entry.type));
		insert_values.push_back(std::to_string(blocked_spells_entry.zoneid));
		insert_values.push_back(std::to_string(blocked_spells_entry.x));
		insert_values.push_back(std::to_string(blocked_spells_entry.y));
		insert_values.push_back(std::to_string(blocked_spells_entry.z));
		insert_values.push_back(std::to_string(blocked_spells_entry.x_diff));
		insert_values.push_back(std::to_string(blocked_spells_entry.y_diff));
		insert_values.push_back(std::to_string(blocked_spells_entry.z_diff));
		insert_values.push_back("'" + EscapeString(blocked_spells_entry.message) + "'");
		insert_values.push_back("'" + EscapeString(blocked_spells_entry.description) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			blocked_spells_entry.id = results.LastInsertedID();
			return blocked_spells_entry;
		}

		blocked_spells_entry = NewEntity();

		return blocked_spells_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<BlockedSpells> blocked_spells_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &blocked_spells_entry: blocked_spells_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(blocked_spells_entry.spellid));
			insert_values.push_back(std::to_string(blocked_spells_entry.type));
			insert_values.push_back(std::to_string(blocked_spells_entry.zoneid));
			insert_values.push_back(std::to_string(blocked_spells_entry.x));
			insert_values.push_back(std::to_string(blocked_spells_entry.y));
			insert_values.push_back(std::to_string(blocked_spells_entry.z));
			insert_values.push_back(std::to_string(blocked_spells_entry.x_diff));
			insert_values.push_back(std::to_string(blocked_spells_entry.y_diff));
			insert_values.push_back(std::to_string(blocked_spells_entry.z_diff));
			insert_values.push_back("'" + EscapeString(blocked_spells_entry.message) + "'");
			insert_values.push_back("'" + EscapeString(blocked_spells_entry.description) + "'");

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<BlockedSpells> All(Database& db)
	{
		std::vector<BlockedSpells> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BlockedSpells entry{};

			entry.id          = atoi(row[0]);
			entry.spellid     = atoi(row[1]);
			entry.type        = atoi(row[2]);
			entry.zoneid      = atoi(row[3]);
			entry.x           = static_cast<float>(atof(row[4]));
			entry.y           = static_cast<float>(atof(row[5]));
			entry.z           = static_cast<float>(atof(row[6]));
			entry.x_diff      = static_cast<float>(atof(row[7]));
			entry.y_diff      = static_cast<float>(atof(row[8]));
			entry.z_diff      = static_cast<float>(atof(row[9]));
			entry.message     = row[10] ? row[10] : "";
			entry.description = row[11] ? row[11] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<BlockedSpells> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<BlockedSpells> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BlockedSpells entry{};

			entry.id          = atoi(row[0]);
			entry.spellid     = atoi(row[1]);
			entry.type        = atoi(row[2]);
			entry.zoneid      = atoi(row[3]);
			entry.x           = static_cast<float>(atof(row[4]));
			entry.y           = static_cast<float>(atof(row[5]));
			entry.z           = static_cast<float>(atof(row[6]));
			entry.x_diff      = static_cast<float>(atof(row[7]));
			entry.y_diff      = static_cast<float>(atof(row[8]));
			entry.z_diff      = static_cast<float>(atof(row[9]));
			entry.message     = row[10] ? row[10] : "";
			entry.description = row[11] ? row[11] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static int DeleteWhere(Database& db, std::string where_filter)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_BLOCKED_SPELLS_REPOSITORY_H
