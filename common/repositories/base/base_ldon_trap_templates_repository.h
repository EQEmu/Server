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

#ifndef EQEMU_BASE_LDON_TRAP_TEMPLATES_REPOSITORY_H
#define EQEMU_BASE_LDON_TRAP_TEMPLATES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseLdonTrapTemplatesRepository {
public:
	struct LdonTrapTemplates {
		int id;
		int type;
		int spell_id;
		int skill;
		int locked;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"type",
			"spell_id",
			"skill",
			"locked",
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
		return std::string("ldon_trap_templates");
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

	static LdonTrapTemplates NewEntity()
	{
		LdonTrapTemplates entry{};

		entry.id       = 0;
		entry.type     = 1;
		entry.spell_id = 0;
		entry.skill    = 0;
		entry.locked   = 0;

		return entry;
	}

	static LdonTrapTemplates GetLdonTrapTemplatesEntry(
		const std::vector<LdonTrapTemplates> &ldon_trap_templatess,
		int ldon_trap_templates_id
	)
	{
		for (auto &ldon_trap_templates : ldon_trap_templatess) {
			if (ldon_trap_templates.id == ldon_trap_templates_id) {
				return ldon_trap_templates;
			}
		}

		return NewEntity();
	}

	static LdonTrapTemplates FindOne(
		Database& db,
		int ldon_trap_templates_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				ldon_trap_templates_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			LdonTrapTemplates entry{};

			entry.id       = atoi(row[0]);
			entry.type     = atoi(row[1]);
			entry.spell_id = atoi(row[2]);
			entry.skill    = atoi(row[3]);
			entry.locked   = atoi(row[4]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int ldon_trap_templates_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				ldon_trap_templates_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		LdonTrapTemplates ldon_trap_templates_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(ldon_trap_templates_entry.id));
		update_values.push_back(columns[1] + " = " + std::to_string(ldon_trap_templates_entry.type));
		update_values.push_back(columns[2] + " = " + std::to_string(ldon_trap_templates_entry.spell_id));
		update_values.push_back(columns[3] + " = " + std::to_string(ldon_trap_templates_entry.skill));
		update_values.push_back(columns[4] + " = " + std::to_string(ldon_trap_templates_entry.locked));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				ldon_trap_templates_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static LdonTrapTemplates InsertOne(
		Database& db,
		LdonTrapTemplates ldon_trap_templates_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(ldon_trap_templates_entry.id));
		insert_values.push_back(std::to_string(ldon_trap_templates_entry.type));
		insert_values.push_back(std::to_string(ldon_trap_templates_entry.spell_id));
		insert_values.push_back(std::to_string(ldon_trap_templates_entry.skill));
		insert_values.push_back(std::to_string(ldon_trap_templates_entry.locked));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			ldon_trap_templates_entry.id = results.LastInsertedID();
			return ldon_trap_templates_entry;
		}

		ldon_trap_templates_entry = NewEntity();

		return ldon_trap_templates_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<LdonTrapTemplates> ldon_trap_templates_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &ldon_trap_templates_entry: ldon_trap_templates_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(ldon_trap_templates_entry.id));
			insert_values.push_back(std::to_string(ldon_trap_templates_entry.type));
			insert_values.push_back(std::to_string(ldon_trap_templates_entry.spell_id));
			insert_values.push_back(std::to_string(ldon_trap_templates_entry.skill));
			insert_values.push_back(std::to_string(ldon_trap_templates_entry.locked));

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

	static std::vector<LdonTrapTemplates> All(Database& db)
	{
		std::vector<LdonTrapTemplates> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LdonTrapTemplates entry{};

			entry.id       = atoi(row[0]);
			entry.type     = atoi(row[1]);
			entry.spell_id = atoi(row[2]);
			entry.skill    = atoi(row[3]);
			entry.locked   = atoi(row[4]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<LdonTrapTemplates> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<LdonTrapTemplates> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LdonTrapTemplates entry{};

			entry.id       = atoi(row[0]);
			entry.type     = atoi(row[1]);
			entry.spell_id = atoi(row[2]);
			entry.skill    = atoi(row[3]);
			entry.locked   = atoi(row[4]);

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

#endif //EQEMU_BASE_LDON_TRAP_TEMPLATES_REPOSITORY_H
