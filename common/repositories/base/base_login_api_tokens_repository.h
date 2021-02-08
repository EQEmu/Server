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

#ifndef EQEMU_BASE_LOGIN_API_TOKENS_REPOSITORY_H
#define EQEMU_BASE_LOGIN_API_TOKENS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseLoginApiTokensRepository {
public:
	struct LoginApiTokens {
		int         id;
		std::string token;
		int         can_write;
		int         can_read;
		std::string created_at;
		std::string updated_at;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"token",
			"can_write",
			"can_read",
			"created_at",
			"updated_at",
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
		return std::string("login_api_tokens");
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

	static LoginApiTokens NewEntity()
	{
		LoginApiTokens entry{};

		entry.id         = 0;
		entry.token      = "";
		entry.can_write  = 0;
		entry.can_read   = 0;
		entry.created_at = 0;
		entry.updated_at = current_timestamp();

		return entry;
	}

	static LoginApiTokens GetLoginApiTokensEntry(
		const std::vector<LoginApiTokens> &login_api_tokenss,
		int login_api_tokens_id
	)
	{
		for (auto &login_api_tokens : login_api_tokenss) {
			if (login_api_tokens.id == login_api_tokens_id) {
				return login_api_tokens;
			}
		}

		return NewEntity();
	}

	static LoginApiTokens FindOne(
		Database& db,
		int login_api_tokens_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				login_api_tokens_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			LoginApiTokens entry{};

			entry.id         = atoi(row[0]);
			entry.token      = row[1] ? row[1] : "";
			entry.can_write  = atoi(row[2]);
			entry.can_read   = atoi(row[3]);
			entry.created_at = row[4] ? row[4] : "";
			entry.updated_at = row[5] ? row[5] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int login_api_tokens_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				login_api_tokens_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		LoginApiTokens login_api_tokens_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(login_api_tokens_entry.token) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(login_api_tokens_entry.can_write));
		update_values.push_back(columns[3] + " = " + std::to_string(login_api_tokens_entry.can_read));
		update_values.push_back(columns[4] + " = '" + EscapeString(login_api_tokens_entry.created_at) + "'");
		update_values.push_back(columns[5] + " = '" + EscapeString(login_api_tokens_entry.updated_at) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				login_api_tokens_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static LoginApiTokens InsertOne(
		Database& db,
		LoginApiTokens login_api_tokens_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(login_api_tokens_entry.token) + "'");
		insert_values.push_back(std::to_string(login_api_tokens_entry.can_write));
		insert_values.push_back(std::to_string(login_api_tokens_entry.can_read));
		insert_values.push_back("'" + EscapeString(login_api_tokens_entry.created_at) + "'");
		insert_values.push_back("'" + EscapeString(login_api_tokens_entry.updated_at) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			login_api_tokens_entry.id = results.LastInsertedID();
			return login_api_tokens_entry;
		}

		login_api_tokens_entry = NewEntity();

		return login_api_tokens_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<LoginApiTokens> login_api_tokens_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &login_api_tokens_entry: login_api_tokens_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(login_api_tokens_entry.token) + "'");
			insert_values.push_back(std::to_string(login_api_tokens_entry.can_write));
			insert_values.push_back(std::to_string(login_api_tokens_entry.can_read));
			insert_values.push_back("'" + EscapeString(login_api_tokens_entry.created_at) + "'");
			insert_values.push_back("'" + EscapeString(login_api_tokens_entry.updated_at) + "'");

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

	static std::vector<LoginApiTokens> All(Database& db)
	{
		std::vector<LoginApiTokens> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoginApiTokens entry{};

			entry.id         = atoi(row[0]);
			entry.token      = row[1] ? row[1] : "";
			entry.can_write  = atoi(row[2]);
			entry.can_read   = atoi(row[3]);
			entry.created_at = row[4] ? row[4] : "";
			entry.updated_at = row[5] ? row[5] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<LoginApiTokens> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<LoginApiTokens> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoginApiTokens entry{};

			entry.id         = atoi(row[0]);
			entry.token      = row[1] ? row[1] : "";
			entry.can_write  = atoi(row[2]);
			entry.can_read   = atoi(row[3]);
			entry.created_at = row[4] ? row[4] : "";
			entry.updated_at = row[5] ? row[5] : "";

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

#endif //EQEMU_BASE_LOGIN_API_TOKENS_REPOSITORY_H
