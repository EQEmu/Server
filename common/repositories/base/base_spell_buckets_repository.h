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

#ifndef EQEMU_BASE_SPELL_BUCKETS_REPOSITORY_H
#define EQEMU_BASE_SPELL_BUCKETS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseSpellBucketsRepository {
public:
	struct SpellBuckets {
		int         spellid;
		std::string key;
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
			"key",
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
		return std::string("spell_buckets");
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

	static SpellBuckets NewEntity()
	{
		SpellBuckets entry{};

		entry.spellid = 0;
		entry.key     = "";
		entry.value   = "";

		return entry;
	}

	static SpellBuckets GetSpellBucketsEntry(
		const std::vector<SpellBuckets> &spell_bucketss,
		int spell_buckets_id
	)
	{
		for (auto &spell_buckets : spell_bucketss) {
			if (spell_buckets.spellid == spell_buckets_id) {
				return spell_buckets;
			}
		}

		return NewEntity();
	}

	static SpellBuckets FindOne(
		Database& db,
		int spell_buckets_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				spell_buckets_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			SpellBuckets entry{};

			entry.spellid = atoi(row[0]);
			entry.key     = row[1] ? row[1] : "";
			entry.value   = row[2] ? row[2] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int spell_buckets_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				spell_buckets_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		SpellBuckets spell_buckets_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(spell_buckets_entry.spellid));
		update_values.push_back(columns[1] + " = '" + EscapeString(spell_buckets_entry.key) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(spell_buckets_entry.value) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				spell_buckets_entry.spellid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static SpellBuckets InsertOne(
		Database& db,
		SpellBuckets spell_buckets_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(spell_buckets_entry.spellid));
		insert_values.push_back("'" + EscapeString(spell_buckets_entry.key) + "'");
		insert_values.push_back("'" + EscapeString(spell_buckets_entry.value) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			spell_buckets_entry.spellid = results.LastInsertedID();
			return spell_buckets_entry;
		}

		spell_buckets_entry = NewEntity();

		return spell_buckets_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<SpellBuckets> spell_buckets_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &spell_buckets_entry: spell_buckets_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(spell_buckets_entry.spellid));
			insert_values.push_back("'" + EscapeString(spell_buckets_entry.key) + "'");
			insert_values.push_back("'" + EscapeString(spell_buckets_entry.value) + "'");

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

	static std::vector<SpellBuckets> All(Database& db)
	{
		std::vector<SpellBuckets> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpellBuckets entry{};

			entry.spellid = atoi(row[0]);
			entry.key     = row[1] ? row[1] : "";
			entry.value   = row[2] ? row[2] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<SpellBuckets> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<SpellBuckets> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpellBuckets entry{};

			entry.spellid = atoi(row[0]);
			entry.key     = row[1] ? row[1] : "";
			entry.value   = row[2] ? row[2] : "";

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

#endif //EQEMU_BASE_SPELL_BUCKETS_REPOSITORY_H
