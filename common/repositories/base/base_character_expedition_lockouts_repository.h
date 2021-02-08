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

#ifndef EQEMU_BASE_CHARACTER_EXPEDITION_LOCKOUTS_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_EXPEDITION_LOCKOUTS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseCharacterExpeditionLockoutsRepository {
public:
	struct CharacterExpeditionLockouts {
		int         id;
		int         character_id;
		std::string expedition_name;
		std::string event_name;
		std::string expire_time;
		int         duration;
		std::string from_expedition_uuid;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"character_id",
			"expedition_name",
			"event_name",
			"expire_time",
			"duration",
			"from_expedition_uuid",
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
		return std::string("character_expedition_lockouts");
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

	static CharacterExpeditionLockouts NewEntity()
	{
		CharacterExpeditionLockouts entry{};

		entry.id                   = 0;
		entry.character_id         = 0;
		entry.expedition_name      = "";
		entry.event_name           = "";
		entry.expire_time          = current_timestamp();
		entry.duration             = 0;
		entry.from_expedition_uuid = "";

		return entry;
	}

	static CharacterExpeditionLockouts GetCharacterExpeditionLockoutsEntry(
		const std::vector<CharacterExpeditionLockouts> &character_expedition_lockoutss,
		int character_expedition_lockouts_id
	)
	{
		for (auto &character_expedition_lockouts : character_expedition_lockoutss) {
			if (character_expedition_lockouts.id == character_expedition_lockouts_id) {
				return character_expedition_lockouts;
			}
		}

		return NewEntity();
	}

	static CharacterExpeditionLockouts FindOne(
		Database& db,
		int character_expedition_lockouts_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_expedition_lockouts_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterExpeditionLockouts entry{};

			entry.id                   = atoi(row[0]);
			entry.character_id         = atoi(row[1]);
			entry.expedition_name      = row[2] ? row[2] : "";
			entry.event_name           = row[3] ? row[3] : "";
			entry.expire_time          = row[4] ? row[4] : "";
			entry.duration             = atoi(row[5]);
			entry.from_expedition_uuid = row[6] ? row[6] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_expedition_lockouts_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_expedition_lockouts_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		CharacterExpeditionLockouts character_expedition_lockouts_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(character_expedition_lockouts_entry.character_id));
		update_values.push_back(columns[2] + " = '" + EscapeString(character_expedition_lockouts_entry.expedition_name) + "'");
		update_values.push_back(columns[3] + " = '" + EscapeString(character_expedition_lockouts_entry.event_name) + "'");
		update_values.push_back(columns[4] + " = '" + EscapeString(character_expedition_lockouts_entry.expire_time) + "'");
		update_values.push_back(columns[5] + " = " + std::to_string(character_expedition_lockouts_entry.duration));
		update_values.push_back(columns[6] + " = '" + EscapeString(character_expedition_lockouts_entry.from_expedition_uuid) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				character_expedition_lockouts_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterExpeditionLockouts InsertOne(
		Database& db,
		CharacterExpeditionLockouts character_expedition_lockouts_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_expedition_lockouts_entry.character_id));
		insert_values.push_back("'" + EscapeString(character_expedition_lockouts_entry.expedition_name) + "'");
		insert_values.push_back("'" + EscapeString(character_expedition_lockouts_entry.event_name) + "'");
		insert_values.push_back("'" + EscapeString(character_expedition_lockouts_entry.expire_time) + "'");
		insert_values.push_back(std::to_string(character_expedition_lockouts_entry.duration));
		insert_values.push_back("'" + EscapeString(character_expedition_lockouts_entry.from_expedition_uuid) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			character_expedition_lockouts_entry.id = results.LastInsertedID();
			return character_expedition_lockouts_entry;
		}

		character_expedition_lockouts_entry = NewEntity();

		return character_expedition_lockouts_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<CharacterExpeditionLockouts> character_expedition_lockouts_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &character_expedition_lockouts_entry: character_expedition_lockouts_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(character_expedition_lockouts_entry.character_id));
			insert_values.push_back("'" + EscapeString(character_expedition_lockouts_entry.expedition_name) + "'");
			insert_values.push_back("'" + EscapeString(character_expedition_lockouts_entry.event_name) + "'");
			insert_values.push_back("'" + EscapeString(character_expedition_lockouts_entry.expire_time) + "'");
			insert_values.push_back(std::to_string(character_expedition_lockouts_entry.duration));
			insert_values.push_back("'" + EscapeString(character_expedition_lockouts_entry.from_expedition_uuid) + "'");

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

	static std::vector<CharacterExpeditionLockouts> All(Database& db)
	{
		std::vector<CharacterExpeditionLockouts> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterExpeditionLockouts entry{};

			entry.id                   = atoi(row[0]);
			entry.character_id         = atoi(row[1]);
			entry.expedition_name      = row[2] ? row[2] : "";
			entry.event_name           = row[3] ? row[3] : "";
			entry.expire_time          = row[4] ? row[4] : "";
			entry.duration             = atoi(row[5]);
			entry.from_expedition_uuid = row[6] ? row[6] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CharacterExpeditionLockouts> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<CharacterExpeditionLockouts> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterExpeditionLockouts entry{};

			entry.id                   = atoi(row[0]);
			entry.character_id         = atoi(row[1]);
			entry.expedition_name      = row[2] ? row[2] : "";
			entry.event_name           = row[3] ? row[3] : "";
			entry.expire_time          = row[4] ? row[4] : "";
			entry.duration             = atoi(row[5]);
			entry.from_expedition_uuid = row[6] ? row[6] : "";

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

#endif //EQEMU_BASE_CHARACTER_EXPEDITION_LOCKOUTS_REPOSITORY_H
