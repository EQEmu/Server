/**
 * DO NOT MODIFY THIS FILE
 *
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to the repository extending the base.
 * Any modifications to base repositories are to be made by the generator only
 *
 * @generator ./utils/scripts/generators/repository-generator.pl
 * @docs https://docs.eqemu.io/developer/repositories
 */

#ifndef EQEMU_BASE_SCRIPT_CONSTANTS_REPOSITORY_H
#define EQEMU_BASE_SCRIPT_CONSTANTS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseScriptConstantsRepository {
public:
	struct ScriptConstants {
		std::string zone;
		int16_t     version;
		std::string lua_namespace;
		std::string name;
		std::string value;
		int16_t     valuetype;
	};

	static std::string PrimaryKey()
	{
		return std::string("version");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"zone",
			"version",
			"lua_namespace",
			"name",
			"value",
			"valuetype",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"zone",
			"version",
			"lua_namespace",
			"name",
			"value",
			"valuetype",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("script_constants");
	}

	static std::string BaseSelect()
	{
		return fmt::format(
			"SELECT {} FROM {}",
			SelectColumnsRaw(),
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

	static ScriptConstants NewEntity()
	{
		ScriptConstants e{};

		e.zone          = "";
		e.version       = -1;
		e.lua_namespace = "";
		e.name          = "";
		e.value         = "";
		e.valuetype     = 0;

		return e;
	}

	static ScriptConstants GetScriptConstants(
		const std::vector<ScriptConstants> &script_constantss,
		int script_constants_id
	)
	{
		for (auto &script_constants : script_constantss) {
			if (script_constants.version == script_constants_id) {
				return script_constants;
			}
		}

		return NewEntity();
	}

	static ScriptConstants FindOne(
		Database& db,
		int script_constants_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				script_constants_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			ScriptConstants e{};

			e.zone          = row[0] ? row[0] : "";
			e.version       = row[1] ? static_cast<int16_t>(atoi(row[1])) : -1;
			e.lua_namespace = row[2] ? row[2] : "";
			e.name          = row[3] ? row[3] : "";
			e.value         = row[4] ? row[4] : "";
			e.valuetype     = row[5] ? static_cast<int16_t>(atoi(row[5])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int script_constants_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				script_constants_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const ScriptConstants &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = '" + Strings::Escape(e.zone) + "'");
		v.push_back(columns[1] + " = " + std::to_string(e.version));
		v.push_back(columns[2] + " = '" + Strings::Escape(e.lua_namespace) + "'");
		v.push_back(columns[3] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[4] + " = '" + Strings::Escape(e.value) + "'");
		v.push_back(columns[5] + " = " + std::to_string(e.valuetype));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.version
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static ScriptConstants InsertOne(
		Database& db,
		ScriptConstants e
	)
	{
		std::vector<std::string> v;

		v.push_back("'" + Strings::Escape(e.zone) + "'");
		v.push_back(std::to_string(e.version));
		v.push_back("'" + Strings::Escape(e.lua_namespace) + "'");
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back("'" + Strings::Escape(e.value) + "'");
		v.push_back(std::to_string(e.valuetype));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.version = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<ScriptConstants> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back("'" + Strings::Escape(e.zone) + "'");
			v.push_back(std::to_string(e.version));
			v.push_back("'" + Strings::Escape(e.lua_namespace) + "'");
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back("'" + Strings::Escape(e.value) + "'");
			v.push_back(std::to_string(e.valuetype));

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<ScriptConstants> All(Database& db)
	{
		std::vector<ScriptConstants> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ScriptConstants e{};

			e.zone          = row[0] ? row[0] : "";
			e.version       = row[1] ? static_cast<int16_t>(atoi(row[1])) : -1;
			e.lua_namespace = row[2] ? row[2] : "";
			e.name          = row[3] ? row[3] : "";
			e.value         = row[4] ? row[4] : "";
			e.valuetype     = row[5] ? static_cast<int16_t>(atoi(row[5])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<ScriptConstants> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<ScriptConstants> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ScriptConstants e{};

			e.zone          = row[0] ? row[0] : "";
			e.version       = row[1] ? static_cast<int16_t>(atoi(row[1])) : -1;
			e.lua_namespace = row[2] ? row[2] : "";
			e.name          = row[3] ? row[3] : "";
			e.value         = row[4] ? row[4] : "";
			e.valuetype     = row[5] ? static_cast<int16_t>(atoi(row[5])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static int DeleteWhere(Database& db, const std::string &where_filter)
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

	static int64 GetMaxId(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COALESCE(MAX({}), 0) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string &where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static std::string BaseReplace()
	{
		return fmt::format(
			"REPLACE INTO {} ({}) ",
			TableName(),
			ColumnsRaw()
		);
	}

	static int ReplaceOne(
		Database& db,
		const ScriptConstants &e
	)
	{
		std::vector<std::string> v;

		v.push_back("'" + Strings::Escape(e.zone) + "'");
		v.push_back(std::to_string(e.version));
		v.push_back("'" + Strings::Escape(e.lua_namespace) + "'");
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back("'" + Strings::Escape(e.value) + "'");
		v.push_back(std::to_string(e.valuetype));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseReplace(),
				Strings::Implode(",", v)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int ReplaceMany(
		Database& db,
		const std::vector<ScriptConstants> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back("'" + Strings::Escape(e.zone) + "'");
			v.push_back(std::to_string(e.version));
			v.push_back("'" + Strings::Escape(e.lua_namespace) + "'");
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back("'" + Strings::Escape(e.value) + "'");
			v.push_back(std::to_string(e.valuetype));

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseReplace(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}
};

#endif //EQEMU_BASE_SCRIPT_CONSTANTS_REPOSITORY_H
