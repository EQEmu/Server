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
#include "../../string_util.h"

class BaseContentFlagsRepository {
public:
	struct ContentFlags {
		int         id;
		std::string flag_name;
		int         enabled;
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

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("content_flags");
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

	static ContentFlags NewEntity()
	{
		ContentFlags entry{};

		entry.id        = 0;
		entry.flag_name = "";
		entry.enabled   = 0;
		entry.notes     = "";

		return entry;
	}

	static ContentFlags GetContentFlagsEntry(
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
			ContentFlags entry{};

			entry.id        = atoi(row[0]);
			entry.flag_name = row[1] ? row[1] : "";
			entry.enabled   = atoi(row[2]);
			entry.notes     = row[3] ? row[3] : "";

			return entry;
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
		ContentFlags content_flags_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(content_flags_entry.flag_name) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(content_flags_entry.enabled));
		update_values.push_back(columns[3] + " = '" + EscapeString(content_flags_entry.notes) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				content_flags_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static ContentFlags InsertOne(
		Database& db,
		ContentFlags content_flags_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(content_flags_entry.id));
		insert_values.push_back("'" + EscapeString(content_flags_entry.flag_name) + "'");
		insert_values.push_back(std::to_string(content_flags_entry.enabled));
		insert_values.push_back("'" + EscapeString(content_flags_entry.notes) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			content_flags_entry.id = results.LastInsertedID();
			return content_flags_entry;
		}

		content_flags_entry = NewEntity();

		return content_flags_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<ContentFlags> content_flags_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &content_flags_entry: content_flags_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(content_flags_entry.id));
			insert_values.push_back("'" + EscapeString(content_flags_entry.flag_name) + "'");
			insert_values.push_back(std::to_string(content_flags_entry.enabled));
			insert_values.push_back("'" + EscapeString(content_flags_entry.notes) + "'");

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
			ContentFlags entry{};

			entry.id        = atoi(row[0]);
			entry.flag_name = row[1] ? row[1] : "";
			entry.enabled   = atoi(row[2]);
			entry.notes     = row[3] ? row[3] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<ContentFlags> GetWhere(Database& db, std::string where_filter)
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
			ContentFlags entry{};

			entry.id        = atoi(row[0]);
			entry.flag_name = row[1] ? row[1] : "";
			entry.enabled   = atoi(row[2]);
			entry.notes     = row[3] ? row[3] : "";

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

#endif //EQEMU_BASE_CONTENT_FLAGS_REPOSITORY_H
