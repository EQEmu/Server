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

#ifndef EQEMU_BASE_CONTENT_FLAGS_REPOSITORY_H
#define EQEMU_BASE_CONTENT_FLAGS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseContentFlagsRepository {
public:
	struct ContentFlags {
		int32_t     id;
		std::string flag_name;
		int8_t      enabled;
		std::string notes;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"flag_name",
			"enabled",
			"notes",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"flag_name",
			"enabled",
			"notes",
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
		return std::string("content_flags");
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

	static ContentFlags NewEntity()
	{
		ContentFlags e{};

		e.id        = 0;
		e.flag_name = "";
		e.enabled   = 0;
		e.notes     = "";

		return e;
	}

	static ContentFlags GetContentFlags(
		const std::vector<ContentFlags> &content_flagss,
		int content_flags_id
	)
	{
		for (auto &content_flags : content_flagss) {
			if (content_flags.id == content_flags_id) {
				return content_flags;
			}
		}

		return NewEntity();
	}

	static ContentFlags FindOne(
		Database& db,
		int content_flags_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				content_flags_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			ContentFlags e{};

			e.id        = static_cast<int32_t>(atoi(row[0]));
			e.flag_name = row[1] ? row[1] : "";
			e.enabled   = static_cast<int8_t>(atoi(row[2]));
			e.notes     = row[3] ? row[3] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int content_flags_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				content_flags_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const ContentFlags &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.flag_name) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.enabled));
		v.push_back(columns[3] + " = '" + Strings::Escape(e.notes) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static ContentFlags InsertOne(
		Database& db,
		ContentFlags e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.flag_name) + "'");
		v.push_back(std::to_string(e.enabled));
		v.push_back("'" + Strings::Escape(e.notes) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<ContentFlags> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.flag_name) + "'");
			v.push_back(std::to_string(e.enabled));
			v.push_back("'" + Strings::Escape(e.notes) + "'");

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

	static std::vector<ContentFlags> All(Database& db)
	{
		std::vector<ContentFlags> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ContentFlags e{};

			e.id        = static_cast<int32_t>(atoi(row[0]));
			e.flag_name = row[1] ? row[1] : "";
			e.enabled   = static_cast<int8_t>(atoi(row[2]));
			e.notes     = row[3] ? row[3] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<ContentFlags> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<ContentFlags> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ContentFlags e{};

			e.id        = static_cast<int32_t>(atoi(row[0]));
			e.flag_name = row[1] ? row[1] : "";
			e.enabled   = static_cast<int8_t>(atoi(row[2]));
			e.notes     = row[3] ? row[3] : "";

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

};

#endif //EQEMU_BASE_CONTENT_FLAGS_REPOSITORY_H
