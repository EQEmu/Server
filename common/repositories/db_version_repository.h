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

#ifndef EQEMU_DB_VERSION_REPOSITORY_H
#define EQEMU_DB_VERSION_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class DbVersionRepository {
public:
	struct DbVersion {
		int version;
	};

	static std::string PrimaryKey()
	{
		return std::string("");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"version",
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
		return std::string("db_version");
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

	static DbVersion NewEntity()
	{
		DbVersion entry{};

		entry.version = 0;

		return entry;
	}

	static DbVersion GetDbVersionEntry(
		const std::vector<DbVersion> &db_versions,
		int db_version_id
	)
	{
		for (auto &db_version : db_versions) {
			if (db_version. == db_version_id) {
				return db_version;
			}
		}

		return NewEntity();
	}

	static DbVersion FindOne(
		int db_version_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				db_version_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			DbVersion entry{};

			entry.version = atoi(row[0]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int db_version_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				db_version_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		DbVersion db_version_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(db_version_entry.version));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				db_version_entry.
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static DbVersion InsertOne(
		DbVersion db_version_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(db_version_entry.version));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			db_version_entry.id = results.LastInsertedID();
			return db_version_entry;
		}

		db_version_entry = InstanceListRepository::NewEntity();

		return db_version_entry;
	}

	static int InsertMany(
		std::vector<DbVersion> db_version_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &db_version_entry: db_version_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(db_version_entry.version));

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

	static std::vector<DbVersion> All()
	{
		std::vector<DbVersion> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DbVersion entry{};

			entry.version = atoi(row[0]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

};

#endif //EQEMU_DB_VERSION_REPOSITORY_H
