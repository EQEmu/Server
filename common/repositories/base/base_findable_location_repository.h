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

#ifndef EQEMU_BASE_FINDABLE_LOCATION_REPOSITORY_H
#define EQEMU_BASE_FINDABLE_LOCATION_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseFindableLocationRepository {
public:
	struct FindableLocation {
		uint32_t    id;
		std::string zone;
		int32_t     version;
		int32_t     findable_id;
		int32_t     findable_sub_id;
		int32_t     type;
		int32_t     zone_id;
		int32_t     zone_id_index;
		float       x;
		float       y;
		float       z;
		int8_t      min_expansion;
		int8_t      max_expansion;
		std::string content_flags;
		std::string content_flags_disabled;
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
			"findable_id",
			"findable_sub_id",
			"type",
			"zone_id",
			"zone_id_index",
			"x",
			"y",
			"z",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"zone",
			"version",
			"findable_id",
			"findable_sub_id",
			"type",
			"zone_id",
			"zone_id_index",
			"x",
			"y",
			"z",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
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
		return std::string("findable_location");
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

	static FindableLocation NewEntity()
	{
		FindableLocation e{};

		e.id                     = 0;
		e.zone                   = "";
		e.version                = 0;
		e.findable_id            = 0;
		e.findable_sub_id        = 0;
		e.type                   = 0;
		e.zone_id                = 0;
		e.zone_id_index          = 0;
		e.x                      = 0;
		e.y                      = 0;
		e.z                      = 0;
		e.min_expansion          = -1;
		e.max_expansion          = -1;
		e.content_flags          = "";
		e.content_flags_disabled = "";

		return e;
	}

	static FindableLocation GetFindableLocation(
		const std::vector<FindableLocation> &findable_locations,
		int findable_location_id
	)
	{
		for (auto &findable_location : findable_locations) {
			if (findable_location.id == findable_location_id) {
				return findable_location;
			}
		}

		return NewEntity();
	}

	static FindableLocation FindOne(
		Database& db,
		int findable_location_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				findable_location_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			FindableLocation e{};

			e.id                     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.zone                   = row[1] ? row[1] : "";
			e.version                = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.findable_id            = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.findable_sub_id        = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.type                   = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.zone_id                = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.zone_id_index          = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.x                      = row[8] ? strtof(row[8], nullptr) : 0;
			e.y                      = row[9] ? strtof(row[9], nullptr) : 0;
			e.z                      = row[10] ? strtof(row[10], nullptr) : 0;
			e.min_expansion          = row[11] ? static_cast<int8_t>(atoi(row[11])) : -1;
			e.max_expansion          = row[12] ? static_cast<int8_t>(atoi(row[12])) : -1;
			e.content_flags          = row[13] ? row[13] : "";
			e.content_flags_disabled = row[14] ? row[14] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int findable_location_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				findable_location_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const FindableLocation &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.zone) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.version));
		v.push_back(columns[3] + " = " + std::to_string(e.findable_id));
		v.push_back(columns[4] + " = " + std::to_string(e.findable_sub_id));
		v.push_back(columns[5] + " = " + std::to_string(e.type));
		v.push_back(columns[6] + " = " + std::to_string(e.zone_id));
		v.push_back(columns[7] + " = " + std::to_string(e.zone_id_index));
		v.push_back(columns[8] + " = " + std::to_string(e.x));
		v.push_back(columns[9] + " = " + std::to_string(e.y));
		v.push_back(columns[10] + " = " + std::to_string(e.z));
		v.push_back(columns[11] + " = " + std::to_string(e.min_expansion));
		v.push_back(columns[12] + " = " + std::to_string(e.max_expansion));
		v.push_back(columns[13] + " = '" + Strings::Escape(e.content_flags) + "'");
		v.push_back(columns[14] + " = '" + Strings::Escape(e.content_flags_disabled) + "'");

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

	static FindableLocation InsertOne(
		Database& db,
		FindableLocation e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.zone) + "'");
		v.push_back(std::to_string(e.version));
		v.push_back(std::to_string(e.findable_id));
		v.push_back(std::to_string(e.findable_sub_id));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.zone_id));
		v.push_back(std::to_string(e.zone_id_index));
		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.z));
		v.push_back(std::to_string(e.min_expansion));
		v.push_back(std::to_string(e.max_expansion));
		v.push_back("'" + Strings::Escape(e.content_flags) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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
		const std::vector<FindableLocation> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.zone) + "'");
			v.push_back(std::to_string(e.version));
			v.push_back(std::to_string(e.findable_id));
			v.push_back(std::to_string(e.findable_sub_id));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.zone_id));
			v.push_back(std::to_string(e.zone_id_index));
			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.z));
			v.push_back(std::to_string(e.min_expansion));
			v.push_back(std::to_string(e.max_expansion));
			v.push_back("'" + Strings::Escape(e.content_flags) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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

	static std::vector<FindableLocation> All(Database& db)
	{
		std::vector<FindableLocation> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			FindableLocation e{};

			e.id                     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.zone                   = row[1] ? row[1] : "";
			e.version                = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.findable_id            = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.findable_sub_id        = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.type                   = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.zone_id                = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.zone_id_index          = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.x                      = row[8] ? strtof(row[8], nullptr) : 0;
			e.y                      = row[9] ? strtof(row[9], nullptr) : 0;
			e.z                      = row[10] ? strtof(row[10], nullptr) : 0;
			e.min_expansion          = row[11] ? static_cast<int8_t>(atoi(row[11])) : -1;
			e.max_expansion          = row[12] ? static_cast<int8_t>(atoi(row[12])) : -1;
			e.content_flags          = row[13] ? row[13] : "";
			e.content_flags_disabled = row[14] ? row[14] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<FindableLocation> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<FindableLocation> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			FindableLocation e{};

			e.id                     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.zone                   = row[1] ? row[1] : "";
			e.version                = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.findable_id            = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.findable_sub_id        = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.type                   = row[5] ? static_cast<int32_t>(atoi(row[5])) : 0;
			e.zone_id                = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.zone_id_index          = row[7] ? static_cast<int32_t>(atoi(row[7])) : 0;
			e.x                      = row[8] ? strtof(row[8], nullptr) : 0;
			e.y                      = row[9] ? strtof(row[9], nullptr) : 0;
			e.z                      = row[10] ? strtof(row[10], nullptr) : 0;
			e.min_expansion          = row[11] ? static_cast<int8_t>(atoi(row[11])) : -1;
			e.max_expansion          = row[12] ? static_cast<int8_t>(atoi(row[12])) : -1;
			e.content_flags          = row[13] ? row[13] : "";
			e.content_flags_disabled = row[14] ? row[14] : "";

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
		const FindableLocation &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.zone) + "'");
		v.push_back(std::to_string(e.version));
		v.push_back(std::to_string(e.findable_id));
		v.push_back(std::to_string(e.findable_sub_id));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.zone_id));
		v.push_back(std::to_string(e.zone_id_index));
		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.z));
		v.push_back(std::to_string(e.min_expansion));
		v.push_back(std::to_string(e.max_expansion));
		v.push_back("'" + Strings::Escape(e.content_flags) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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
		const std::vector<FindableLocation> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.zone) + "'");
			v.push_back(std::to_string(e.version));
			v.push_back(std::to_string(e.findable_id));
			v.push_back(std::to_string(e.findable_sub_id));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.zone_id));
			v.push_back(std::to_string(e.zone_id_index));
			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.z));
			v.push_back(std::to_string(e.min_expansion));
			v.push_back(std::to_string(e.max_expansion));
			v.push_back("'" + Strings::Escape(e.content_flags) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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

#endif //EQEMU_BASE_FINDABLE_LOCATION_REPOSITORY_H
