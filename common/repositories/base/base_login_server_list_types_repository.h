/**
 * DO NOT MODIFY THIS FILE
 *
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to the repository extending the base.
 * Any modifications to base repositories are to be made by the generator only
 *
 * @generator ./utils/scripts/generators/repository-generator.pl
 * @docs https://eqemu.gitbook.io/server/in-development/developer-area/repositories
 */

#ifndef EQEMU_BASE_LOGIN_SERVER_LIST_TYPES_REPOSITORY_H
#define EQEMU_BASE_LOGIN_SERVER_LIST_TYPES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseLoginServerListTypesRepository {
public:
	struct LoginServerListTypes {
		int         id;
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
			"description",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("login_server_list_types");
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
			ColumnsRaw()
		);
	}

	static LoginServerListTypes NewEntity()
	{
		LoginServerListTypes entry{};

		entry.id          = 0;
		entry.description = "";

		return entry;
	}

	static LoginServerListTypes GetLoginServerListTypesEntry(
		const std::vector<LoginServerListTypes> &login_server_list_typess,
		int login_server_list_types_id
	)
	{
		for (auto &login_server_list_types : login_server_list_typess) {
			if (login_server_list_types.id == login_server_list_types_id) {
				return login_server_list_types;
			}
		}

		return NewEntity();
	}

	static LoginServerListTypes FindOne(
		Database& db,
		int login_server_list_types_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				login_server_list_types_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			LoginServerListTypes entry{};

			entry.id          = atoi(row[0]);
			entry.description = row[1] ? row[1] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int login_server_list_types_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				login_server_list_types_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		LoginServerListTypes login_server_list_types_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(login_server_list_types_entry.id));
		update_values.push_back(columns[1] + " = '" + EscapeString(login_server_list_types_entry.description) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				login_server_list_types_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static LoginServerListTypes InsertOne(
		Database& db,
		LoginServerListTypes login_server_list_types_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(login_server_list_types_entry.id));
		insert_values.push_back("'" + EscapeString(login_server_list_types_entry.description) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			login_server_list_types_entry.id = results.LastInsertedID();
			return login_server_list_types_entry;
		}

		login_server_list_types_entry = NewEntity();

		return login_server_list_types_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<LoginServerListTypes> login_server_list_types_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &login_server_list_types_entry: login_server_list_types_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(login_server_list_types_entry.id));
			insert_values.push_back("'" + EscapeString(login_server_list_types_entry.description) + "'");

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

	static std::vector<LoginServerListTypes> All(Database& db)
	{
		std::vector<LoginServerListTypes> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoginServerListTypes entry{};

			entry.id          = atoi(row[0]);
			entry.description = row[1] ? row[1] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<LoginServerListTypes> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<LoginServerListTypes> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoginServerListTypes entry{};

			entry.id          = atoi(row[0]);
			entry.description = row[1] ? row[1] : "";

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

#endif //EQEMU_BASE_LOGIN_SERVER_LIST_TYPES_REPOSITORY_H
