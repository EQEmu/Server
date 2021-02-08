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

#ifndef EQEMU_BASE_ACCOUNT_FLAGS_REPOSITORY_H
#define EQEMU_BASE_ACCOUNT_FLAGS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseAccountFlagsRepository {
public:
	struct AccountFlags {
		int         p_accid;
		std::string p_flag;
		std::string p_value;
	};

	static std::string PrimaryKey()
	{
		return std::string("p_accid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"p_accid",
			"p_flag",
			"p_value",
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
		return std::string("account_flags");
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

	static AccountFlags NewEntity()
	{
		AccountFlags entry{};

		entry.p_accid = 0;
		entry.p_flag  = "";
		entry.p_value = "";

		return entry;
	}

	static AccountFlags GetAccountFlagsEntry(
		const std::vector<AccountFlags> &account_flagss,
		int account_flags_id
	)
	{
		for (auto &account_flags : account_flagss) {
			if (account_flags.p_accid == account_flags_id) {
				return account_flags;
			}
		}

		return NewEntity();
	}

	static AccountFlags FindOne(
		Database& db,
		int account_flags_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				account_flags_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AccountFlags entry{};

			entry.p_accid = atoi(row[0]);
			entry.p_flag  = row[1] ? row[1] : "";
			entry.p_value = row[2] ? row[2] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int account_flags_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				account_flags_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		AccountFlags account_flags_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(account_flags_entry.p_accid));
		update_values.push_back(columns[1] + " = '" + EscapeString(account_flags_entry.p_flag) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(account_flags_entry.p_value) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				account_flags_entry.p_accid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static AccountFlags InsertOne(
		Database& db,
		AccountFlags account_flags_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(account_flags_entry.p_accid));
		insert_values.push_back("'" + EscapeString(account_flags_entry.p_flag) + "'");
		insert_values.push_back("'" + EscapeString(account_flags_entry.p_value) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			account_flags_entry.p_accid = results.LastInsertedID();
			return account_flags_entry;
		}

		account_flags_entry = NewEntity();

		return account_flags_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<AccountFlags> account_flags_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &account_flags_entry: account_flags_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(account_flags_entry.p_accid));
			insert_values.push_back("'" + EscapeString(account_flags_entry.p_flag) + "'");
			insert_values.push_back("'" + EscapeString(account_flags_entry.p_value) + "'");

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

	static std::vector<AccountFlags> All(Database& db)
	{
		std::vector<AccountFlags> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AccountFlags entry{};

			entry.p_accid = atoi(row[0]);
			entry.p_flag  = row[1] ? row[1] : "";
			entry.p_value = row[2] ? row[2] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<AccountFlags> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<AccountFlags> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AccountFlags entry{};

			entry.p_accid = atoi(row[0]);
			entry.p_flag  = row[1] ? row[1] : "";
			entry.p_value = row[2] ? row[2] : "";

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

#endif //EQEMU_BASE_ACCOUNT_FLAGS_REPOSITORY_H
