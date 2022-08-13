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

#ifndef EQEMU_BASE_DYNAMIC_ZONE_TEMPLATES_REPOSITORY_H
#define EQEMU_BASE_DYNAMIC_ZONE_TEMPLATES_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseDynamicZoneTemplatesRepository {
public:
	struct DynamicZoneTemplates {
		int         id;
		int         zone_id;
		int         zone_version;
		std::string name;
		int         min_players;
		int         max_players;
		int         duration_seconds;
		int         dz_switch_id;
		int         compass_zone_id;
		float       compass_x;
		float       compass_y;
		float       compass_z;
		int         return_zone_id;
		float       return_x;
		float       return_y;
		float       return_z;
		float       return_h;
		int         override_zone_in;
		float       zone_in_x;
		float       zone_in_y;
		float       zone_in_z;
		float       zone_in_h;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"zone_id",
			"zone_version",
			"name",
			"min_players",
			"max_players",
			"duration_seconds",
			"dz_switch_id",
			"compass_zone_id",
			"compass_x",
			"compass_y",
			"compass_z",
			"return_zone_id",
			"return_x",
			"return_y",
			"return_z",
			"return_h",
			"override_zone_in",
			"zone_in_x",
			"zone_in_y",
			"zone_in_z",
			"zone_in_h",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"zone_id",
			"zone_version",
			"name",
			"min_players",
			"max_players",
			"duration_seconds",
			"dz_switch_id",
			"compass_zone_id",
			"compass_x",
			"compass_y",
			"compass_z",
			"return_zone_id",
			"return_x",
			"return_y",
			"return_z",
			"return_h",
			"override_zone_in",
			"zone_in_x",
			"zone_in_y",
			"zone_in_z",
			"zone_in_h",
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
		return std::string("dynamic_zone_templates");
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

	static DynamicZoneTemplates NewEntity()
	{
		DynamicZoneTemplates e{};

		e.id               = 0;
		e.zone_id          = 0;
		e.zone_version     = 0;
		e.name             = "";
		e.min_players      = 0;
		e.max_players      = 0;
		e.duration_seconds = 0;
		e.dz_switch_id     = 0;
		e.compass_zone_id  = 0;
		e.compass_x        = 0;
		e.compass_y        = 0;
		e.compass_z        = 0;
		e.return_zone_id   = 0;
		e.return_x         = 0;
		e.return_y         = 0;
		e.return_z         = 0;
		e.return_h         = 0;
		e.override_zone_in = 0;
		e.zone_in_x        = 0;
		e.zone_in_y        = 0;
		e.zone_in_z        = 0;
		e.zone_in_h        = 0;

		return e;
	}

	static DynamicZoneTemplates GetDynamicZoneTemplates(
		const std::vector<DynamicZoneTemplates> &dynamic_zone_templatess,
		int dynamic_zone_templates_id
	)
	{
		for (auto &dynamic_zone_templates : dynamic_zone_templatess) {
			if (dynamic_zone_templates.id == dynamic_zone_templates_id) {
				return dynamic_zone_templates;
			}
		}

		return NewEntity();
	}

	static DynamicZoneTemplates FindOne(
		Database& db,
		int dynamic_zone_templates_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				dynamic_zone_templates_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			DynamicZoneTemplates e{};

			e.id               = atoi(row[0]);
			e.zone_id          = atoi(row[1]);
			e.zone_version     = atoi(row[2]);
			e.name             = row[3] ? row[3] : "";
			e.min_players      = atoi(row[4]);
			e.max_players      = atoi(row[5]);
			e.duration_seconds = atoi(row[6]);
			e.dz_switch_id     = atoi(row[7]);
			e.compass_zone_id  = atoi(row[8]);
			e.compass_x        = static_cast<float>(atof(row[9]));
			e.compass_y        = static_cast<float>(atof(row[10]));
			e.compass_z        = static_cast<float>(atof(row[11]));
			e.return_zone_id   = atoi(row[12]);
			e.return_x         = static_cast<float>(atof(row[13]));
			e.return_y         = static_cast<float>(atof(row[14]));
			e.return_z         = static_cast<float>(atof(row[15]));
			e.return_h         = static_cast<float>(atof(row[16]));
			e.override_zone_in = atoi(row[17]);
			e.zone_in_x        = static_cast<float>(atof(row[18]));
			e.zone_in_y        = static_cast<float>(atof(row[19]));
			e.zone_in_z        = static_cast<float>(atof(row[20]));
			e.zone_in_h        = static_cast<float>(atof(row[21]));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int dynamic_zone_templates_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				dynamic_zone_templates_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const DynamicZoneTemplates &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.zone_id));
		v.push_back(columns[2] + " = " + std::to_string(e.zone_version));
		v.push_back(columns[3] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[4] + " = " + std::to_string(e.min_players));
		v.push_back(columns[5] + " = " + std::to_string(e.max_players));
		v.push_back(columns[6] + " = " + std::to_string(e.duration_seconds));
		v.push_back(columns[7] + " = " + std::to_string(e.dz_switch_id));
		v.push_back(columns[8] + " = " + std::to_string(e.compass_zone_id));
		v.push_back(columns[9] + " = " + std::to_string(e.compass_x));
		v.push_back(columns[10] + " = " + std::to_string(e.compass_y));
		v.push_back(columns[11] + " = " + std::to_string(e.compass_z));
		v.push_back(columns[12] + " = " + std::to_string(e.return_zone_id));
		v.push_back(columns[13] + " = " + std::to_string(e.return_x));
		v.push_back(columns[14] + " = " + std::to_string(e.return_y));
		v.push_back(columns[15] + " = " + std::to_string(e.return_z));
		v.push_back(columns[16] + " = " + std::to_string(e.return_h));
		v.push_back(columns[17] + " = " + std::to_string(e.override_zone_in));
		v.push_back(columns[18] + " = " + std::to_string(e.zone_in_x));
		v.push_back(columns[19] + " = " + std::to_string(e.zone_in_y));
		v.push_back(columns[20] + " = " + std::to_string(e.zone_in_z));
		v.push_back(columns[21] + " = " + std::to_string(e.zone_in_h));

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

	static DynamicZoneTemplates InsertOne(
		Database& db,
		DynamicZoneTemplates e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.zone_id));
		v.push_back(std::to_string(e.zone_version));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.min_players));
		v.push_back(std::to_string(e.max_players));
		v.push_back(std::to_string(e.duration_seconds));
		v.push_back(std::to_string(e.dz_switch_id));
		v.push_back(std::to_string(e.compass_zone_id));
		v.push_back(std::to_string(e.compass_x));
		v.push_back(std::to_string(e.compass_y));
		v.push_back(std::to_string(e.compass_z));
		v.push_back(std::to_string(e.return_zone_id));
		v.push_back(std::to_string(e.return_x));
		v.push_back(std::to_string(e.return_y));
		v.push_back(std::to_string(e.return_z));
		v.push_back(std::to_string(e.return_h));
		v.push_back(std::to_string(e.override_zone_in));
		v.push_back(std::to_string(e.zone_in_x));
		v.push_back(std::to_string(e.zone_in_y));
		v.push_back(std::to_string(e.zone_in_z));
		v.push_back(std::to_string(e.zone_in_h));

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
		const std::vector<DynamicZoneTemplates> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.zone_id));
			v.push_back(std::to_string(e.zone_version));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.min_players));
			v.push_back(std::to_string(e.max_players));
			v.push_back(std::to_string(e.duration_seconds));
			v.push_back(std::to_string(e.dz_switch_id));
			v.push_back(std::to_string(e.compass_zone_id));
			v.push_back(std::to_string(e.compass_x));
			v.push_back(std::to_string(e.compass_y));
			v.push_back(std::to_string(e.compass_z));
			v.push_back(std::to_string(e.return_zone_id));
			v.push_back(std::to_string(e.return_x));
			v.push_back(std::to_string(e.return_y));
			v.push_back(std::to_string(e.return_z));
			v.push_back(std::to_string(e.return_h));
			v.push_back(std::to_string(e.override_zone_in));
			v.push_back(std::to_string(e.zone_in_x));
			v.push_back(std::to_string(e.zone_in_y));
			v.push_back(std::to_string(e.zone_in_z));
			v.push_back(std::to_string(e.zone_in_h));

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

	static std::vector<DynamicZoneTemplates> All(Database& db)
	{
		std::vector<DynamicZoneTemplates> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DynamicZoneTemplates e{};

			e.id               = atoi(row[0]);
			e.zone_id          = atoi(row[1]);
			e.zone_version     = atoi(row[2]);
			e.name             = row[3] ? row[3] : "";
			e.min_players      = atoi(row[4]);
			e.max_players      = atoi(row[5]);
			e.duration_seconds = atoi(row[6]);
			e.dz_switch_id     = atoi(row[7]);
			e.compass_zone_id  = atoi(row[8]);
			e.compass_x        = static_cast<float>(atof(row[9]));
			e.compass_y        = static_cast<float>(atof(row[10]));
			e.compass_z        = static_cast<float>(atof(row[11]));
			e.return_zone_id   = atoi(row[12]);
			e.return_x         = static_cast<float>(atof(row[13]));
			e.return_y         = static_cast<float>(atof(row[14]));
			e.return_z         = static_cast<float>(atof(row[15]));
			e.return_h         = static_cast<float>(atof(row[16]));
			e.override_zone_in = atoi(row[17]);
			e.zone_in_x        = static_cast<float>(atof(row[18]));
			e.zone_in_y        = static_cast<float>(atof(row[19]));
			e.zone_in_z        = static_cast<float>(atof(row[20]));
			e.zone_in_h        = static_cast<float>(atof(row[21]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<DynamicZoneTemplates> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<DynamicZoneTemplates> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DynamicZoneTemplates e{};

			e.id               = atoi(row[0]);
			e.zone_id          = atoi(row[1]);
			e.zone_version     = atoi(row[2]);
			e.name             = row[3] ? row[3] : "";
			e.min_players      = atoi(row[4]);
			e.max_players      = atoi(row[5]);
			e.duration_seconds = atoi(row[6]);
			e.dz_switch_id     = atoi(row[7]);
			e.compass_zone_id  = atoi(row[8]);
			e.compass_x        = static_cast<float>(atof(row[9]));
			e.compass_y        = static_cast<float>(atof(row[10]));
			e.compass_z        = static_cast<float>(atof(row[11]));
			e.return_zone_id   = atoi(row[12]);
			e.return_x         = static_cast<float>(atof(row[13]));
			e.return_y         = static_cast<float>(atof(row[14]));
			e.return_z         = static_cast<float>(atof(row[15]));
			e.return_h         = static_cast<float>(atof(row[16]));
			e.override_zone_in = atoi(row[17]);
			e.zone_in_x        = static_cast<float>(atof(row[18]));
			e.zone_in_y        = static_cast<float>(atof(row[19]));
			e.zone_in_z        = static_cast<float>(atof(row[20]));
			e.zone_in_h        = static_cast<float>(atof(row[21]));

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

#endif //EQEMU_BASE_DYNAMIC_ZONE_TEMPLATES_REPOSITORY_H
