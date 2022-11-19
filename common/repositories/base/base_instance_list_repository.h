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

#ifndef EQEMU_BASE_INSTANCE_LIST_REPOSITORY_H
#define EQEMU_BASE_INSTANCE_LIST_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseInstanceListRepository {
public:
	struct InstanceList {
		int32_t  id;
		uint32_t zone;
		uint8_t  version;
		uint8_t  is_global;
		uint32_t start_time;
		uint32_t duration;
		uint8_t  never_expires;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"zone",
			"version",
			"is_global",
			"start_time",
			"duration",
			"never_expires",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"zone",
			"version",
			"is_global",
			"start_time",
			"duration",
			"never_expires",
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
		return std::string("instance_list");
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

	static InstanceList NewEntity()
	{
		InstanceList e{};

		e.id            = 0;
		e.zone          = 0;
		e.version       = 0;
		e.is_global     = 0;
		e.start_time    = 0;
		e.duration      = 0;
		e.never_expires = 0;

		return e;
	}

	static InstanceList GetInstanceList(
		const std::vector<InstanceList> &instance_lists,
		int instance_list_id
	)
	{
		for (auto &instance_list : instance_lists) {
			if (instance_list.id == instance_list_id) {
				return instance_list;
			}
		}

		return NewEntity();
	}

	static InstanceList FindOne(
		Database& db,
		int instance_list_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				instance_list_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			InstanceList e{};

			e.id            = static_cast<int32_t>(atoi(row[0]));
			e.zone          = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.version       = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.is_global     = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));
			e.start_time    = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));
			e.duration      = static_cast<uint32_t>(strtoul(row[5], nullptr, 10));
			e.never_expires = static_cast<uint8_t>(strtoul(row[6], nullptr, 10));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int instance_list_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				instance_list_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const InstanceList &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.zone));
		v.push_back(columns[2] + " = " + std::to_string(e.version));
		v.push_back(columns[3] + " = " + std::to_string(e.is_global));
		v.push_back(columns[4] + " = " + std::to_string(e.start_time));
		v.push_back(columns[5] + " = " + std::to_string(e.duration));
		v.push_back(columns[6] + " = " + std::to_string(e.never_expires));

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

	static InstanceList InsertOne(
		Database& db,
		InstanceList e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.zone));
		v.push_back(std::to_string(e.version));
		v.push_back(std::to_string(e.is_global));
		v.push_back(std::to_string(e.start_time));
		v.push_back(std::to_string(e.duration));
		v.push_back(std::to_string(e.never_expires));

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
		const std::vector<InstanceList> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.zone));
			v.push_back(std::to_string(e.version));
			v.push_back(std::to_string(e.is_global));
			v.push_back(std::to_string(e.start_time));
			v.push_back(std::to_string(e.duration));
			v.push_back(std::to_string(e.never_expires));

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

	static std::vector<InstanceList> All(Database& db)
	{
		std::vector<InstanceList> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			InstanceList e{};

			e.id            = static_cast<int32_t>(atoi(row[0]));
			e.zone          = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.version       = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.is_global     = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));
			e.start_time    = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));
			e.duration      = static_cast<uint32_t>(strtoul(row[5], nullptr, 10));
			e.never_expires = static_cast<uint8_t>(strtoul(row[6], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<InstanceList> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<InstanceList> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			InstanceList e{};

			e.id            = static_cast<int32_t>(atoi(row[0]));
			e.zone          = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.version       = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.is_global     = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));
			e.start_time    = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));
			e.duration      = static_cast<uint32_t>(strtoul(row[5], nullptr, 10));
			e.never_expires = static_cast<uint8_t>(strtoul(row[6], nullptr, 10));

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

#endif //EQEMU_BASE_INSTANCE_LIST_REPOSITORY_H
