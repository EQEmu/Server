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

#ifndef EQEMU_BASE_SPELL_GLOBALS_REPOSITORY_H
#define EQEMU_BASE_SPELL_GLOBALS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseSpellGlobalsRepository {
public:
	struct SpellGlobals {
		int         spellid;
		std::string spell_name;
		std::string qglobal;
		std::string value;
	};

	static std::string PrimaryKey()
	{
		return std::string("spellid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"spellid",
			"spell_name",
			"qglobal",
			"value",
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
		return std::string("spell_globals");
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

	static SpellGlobals NewEntity()
	{
		SpellGlobals entry{};

		entry.spellid    = 0;
		entry.spell_name = "";
		entry.qglobal    = "";
		entry.value      = "";

		return entry;
	}

	static SpellGlobals GetSpellGlobalsEntry(
		const std::vector<SpellGlobals> &spell_globalss,
		int spell_globals_id
	)
	{
		for (auto &spell_globals : spell_globalss) {
			if (spell_globals.spellid == spell_globals_id) {
				return spell_globals;
			}
		}

		return NewEntity();
	}

	static SpellGlobals FindOne(
		int spell_globals_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				spell_globals_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			SpellGlobals entry{};

			entry.spellid    = atoi(row[0]);
			entry.spell_name = row[1] ? row[1] : "";
			entry.qglobal    = row[2] ? row[2] : "";
			entry.value      = row[3] ? row[3] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int spell_globals_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				spell_globals_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		SpellGlobals spell_globals_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(spell_globals_entry.spellid));
		update_values.push_back(columns[1] + " = '" + EscapeString(spell_globals_entry.spell_name) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(spell_globals_entry.qglobal) + "'");
		update_values.push_back(columns[3] + " = '" + EscapeString(spell_globals_entry.value) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				spell_globals_entry.spellid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static SpellGlobals InsertOne(
		SpellGlobals spell_globals_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(spell_globals_entry.spellid));
		insert_values.push_back("'" + EscapeString(spell_globals_entry.spell_name) + "'");
		insert_values.push_back("'" + EscapeString(spell_globals_entry.qglobal) + "'");
		insert_values.push_back("'" + EscapeString(spell_globals_entry.value) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			spell_globals_entry.spellid = results.LastInsertedID();
			return spell_globals_entry;
		}

		spell_globals_entry = NewEntity();

		return spell_globals_entry;
	}

	static int InsertMany(
		std::vector<SpellGlobals> spell_globals_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &spell_globals_entry: spell_globals_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(spell_globals_entry.spellid));
			insert_values.push_back("'" + EscapeString(spell_globals_entry.spell_name) + "'");
			insert_values.push_back("'" + EscapeString(spell_globals_entry.qglobal) + "'");
			insert_values.push_back("'" + EscapeString(spell_globals_entry.value) + "'");

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

	static std::vector<SpellGlobals> All()
	{
		std::vector<SpellGlobals> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpellGlobals entry{};

			entry.spellid    = atoi(row[0]);
			entry.spell_name = row[1] ? row[1] : "";
			entry.qglobal    = row[2] ? row[2] : "";
			entry.value      = row[3] ? row[3] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<SpellGlobals> GetWhere(std::string where_filter)
	{
		std::vector<SpellGlobals> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpellGlobals entry{};

			entry.spellid    = atoi(row[0]);
			entry.spell_name = row[1] ? row[1] : "";
			entry.qglobal    = row[2] ? row[2] : "";
			entry.value      = row[3] ? row[3] : "";

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
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate()
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_SPELL_GLOBALS_REPOSITORY_H
