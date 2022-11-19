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

#ifndef EQEMU_BASE_START_ZONES_REPOSITORY_H
#define EQEMU_BASE_START_ZONES_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseStartZonesRepository {
public:
	struct StartZones {
		float       x;
		float       y;
		float       z;
		float       heading;
		int32_t     zone_id;
		int32_t     bind_id;
		int32_t     player_choice;
		int32_t     player_class;
		int32_t     player_deity;
		int32_t     player_race;
		int32_t     start_zone;
		float       bind_x;
		float       bind_y;
		float       bind_z;
		uint8_t     select_rank;
		int8_t      min_expansion;
		int8_t      max_expansion;
		std::string content_flags;
		std::string content_flags_disabled;
	};

	static std::string PrimaryKey()
	{
		return std::string("player_choice");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"x",
			"y",
			"z",
			"heading",
			"zone_id",
			"bind_id",
			"player_choice",
			"player_class",
			"player_deity",
			"player_race",
			"start_zone",
			"bind_x",
			"bind_y",
			"bind_z",
			"select_rank",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"x",
			"y",
			"z",
			"heading",
			"zone_id",
			"bind_id",
			"player_choice",
			"player_class",
			"player_deity",
			"player_race",
			"start_zone",
			"bind_x",
			"bind_y",
			"bind_z",
			"select_rank",
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
		return std::string("start_zones");
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

	static StartZones NewEntity()
	{
		StartZones e{};

		e.x                      = 0;
		e.y                      = 0;
		e.z                      = 0;
		e.heading                = 0;
		e.zone_id                = 0;
		e.bind_id                = 0;
		e.player_choice          = 0;
		e.player_class           = 0;
		e.player_deity           = 0;
		e.player_race            = 0;
		e.start_zone             = 0;
		e.bind_x                 = 0;
		e.bind_y                 = 0;
		e.bind_z                 = 0;
		e.select_rank            = 50;
		e.min_expansion          = -1;
		e.max_expansion          = -1;
		e.content_flags          = "";
		e.content_flags_disabled = "";

		return e;
	}

	static StartZones GetStartZones(
		const std::vector<StartZones> &start_zoness,
		int start_zones_id
	)
	{
		for (auto &start_zones : start_zoness) {
			if (start_zones.player_choice == start_zones_id) {
				return start_zones;
			}
		}

		return NewEntity();
	}

	static StartZones FindOne(
		Database& db,
		int start_zones_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				start_zones_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			StartZones e{};

			e.x                      = strtof(row[0], nullptr);
			e.y                      = strtof(row[1], nullptr);
			e.z                      = strtof(row[2], nullptr);
			e.heading                = strtof(row[3], nullptr);
			e.zone_id                = static_cast<int32_t>(atoi(row[4]));
			e.bind_id                = static_cast<int32_t>(atoi(row[5]));
			e.player_choice          = static_cast<int32_t>(atoi(row[6]));
			e.player_class           = static_cast<int32_t>(atoi(row[7]));
			e.player_deity           = static_cast<int32_t>(atoi(row[8]));
			e.player_race            = static_cast<int32_t>(atoi(row[9]));
			e.start_zone             = static_cast<int32_t>(atoi(row[10]));
			e.bind_x                 = strtof(row[11], nullptr);
			e.bind_y                 = strtof(row[12], nullptr);
			e.bind_z                 = strtof(row[13], nullptr);
			e.select_rank            = static_cast<uint8_t>(strtoul(row[14], nullptr, 10));
			e.min_expansion          = static_cast<int8_t>(atoi(row[15]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[16]));
			e.content_flags          = row[17] ? row[17] : "";
			e.content_flags_disabled = row[18] ? row[18] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int start_zones_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				start_zones_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const StartZones &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.x));
		v.push_back(columns[1] + " = " + std::to_string(e.y));
		v.push_back(columns[2] + " = " + std::to_string(e.z));
		v.push_back(columns[3] + " = " + std::to_string(e.heading));
		v.push_back(columns[4] + " = " + std::to_string(e.zone_id));
		v.push_back(columns[5] + " = " + std::to_string(e.bind_id));
		v.push_back(columns[6] + " = " + std::to_string(e.player_choice));
		v.push_back(columns[7] + " = " + std::to_string(e.player_class));
		v.push_back(columns[8] + " = " + std::to_string(e.player_deity));
		v.push_back(columns[9] + " = " + std::to_string(e.player_race));
		v.push_back(columns[10] + " = " + std::to_string(e.start_zone));
		v.push_back(columns[11] + " = " + std::to_string(e.bind_x));
		v.push_back(columns[12] + " = " + std::to_string(e.bind_y));
		v.push_back(columns[13] + " = " + std::to_string(e.bind_z));
		v.push_back(columns[14] + " = " + std::to_string(e.select_rank));
		v.push_back(columns[15] + " = " + std::to_string(e.min_expansion));
		v.push_back(columns[16] + " = " + std::to_string(e.max_expansion));
		v.push_back(columns[17] + " = '" + Strings::Escape(e.content_flags) + "'");
		v.push_back(columns[18] + " = '" + Strings::Escape(e.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.player_choice
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static StartZones InsertOne(
		Database& db,
		StartZones e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.z));
		v.push_back(std::to_string(e.heading));
		v.push_back(std::to_string(e.zone_id));
		v.push_back(std::to_string(e.bind_id));
		v.push_back(std::to_string(e.player_choice));
		v.push_back(std::to_string(e.player_class));
		v.push_back(std::to_string(e.player_deity));
		v.push_back(std::to_string(e.player_race));
		v.push_back(std::to_string(e.start_zone));
		v.push_back(std::to_string(e.bind_x));
		v.push_back(std::to_string(e.bind_y));
		v.push_back(std::to_string(e.bind_z));
		v.push_back(std::to_string(e.select_rank));
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
			e.player_choice = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<StartZones> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.z));
			v.push_back(std::to_string(e.heading));
			v.push_back(std::to_string(e.zone_id));
			v.push_back(std::to_string(e.bind_id));
			v.push_back(std::to_string(e.player_choice));
			v.push_back(std::to_string(e.player_class));
			v.push_back(std::to_string(e.player_deity));
			v.push_back(std::to_string(e.player_race));
			v.push_back(std::to_string(e.start_zone));
			v.push_back(std::to_string(e.bind_x));
			v.push_back(std::to_string(e.bind_y));
			v.push_back(std::to_string(e.bind_z));
			v.push_back(std::to_string(e.select_rank));
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

	static std::vector<StartZones> All(Database& db)
	{
		std::vector<StartZones> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			StartZones e{};

			e.x                      = strtof(row[0], nullptr);
			e.y                      = strtof(row[1], nullptr);
			e.z                      = strtof(row[2], nullptr);
			e.heading                = strtof(row[3], nullptr);
			e.zone_id                = static_cast<int32_t>(atoi(row[4]));
			e.bind_id                = static_cast<int32_t>(atoi(row[5]));
			e.player_choice          = static_cast<int32_t>(atoi(row[6]));
			e.player_class           = static_cast<int32_t>(atoi(row[7]));
			e.player_deity           = static_cast<int32_t>(atoi(row[8]));
			e.player_race            = static_cast<int32_t>(atoi(row[9]));
			e.start_zone             = static_cast<int32_t>(atoi(row[10]));
			e.bind_x                 = strtof(row[11], nullptr);
			e.bind_y                 = strtof(row[12], nullptr);
			e.bind_z                 = strtof(row[13], nullptr);
			e.select_rank            = static_cast<uint8_t>(strtoul(row[14], nullptr, 10));
			e.min_expansion          = static_cast<int8_t>(atoi(row[15]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[16]));
			e.content_flags          = row[17] ? row[17] : "";
			e.content_flags_disabled = row[18] ? row[18] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<StartZones> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<StartZones> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			StartZones e{};

			e.x                      = strtof(row[0], nullptr);
			e.y                      = strtof(row[1], nullptr);
			e.z                      = strtof(row[2], nullptr);
			e.heading                = strtof(row[3], nullptr);
			e.zone_id                = static_cast<int32_t>(atoi(row[4]));
			e.bind_id                = static_cast<int32_t>(atoi(row[5]));
			e.player_choice          = static_cast<int32_t>(atoi(row[6]));
			e.player_class           = static_cast<int32_t>(atoi(row[7]));
			e.player_deity           = static_cast<int32_t>(atoi(row[8]));
			e.player_race            = static_cast<int32_t>(atoi(row[9]));
			e.start_zone             = static_cast<int32_t>(atoi(row[10]));
			e.bind_x                 = strtof(row[11], nullptr);
			e.bind_y                 = strtof(row[12], nullptr);
			e.bind_z                 = strtof(row[13], nullptr);
			e.select_rank            = static_cast<uint8_t>(strtoul(row[14], nullptr, 10));
			e.min_expansion          = static_cast<int8_t>(atoi(row[15]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[16]));
			e.content_flags          = row[17] ? row[17] : "";
			e.content_flags_disabled = row[18] ? row[18] : "";

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

#endif //EQEMU_BASE_START_ZONES_REPOSITORY_H
