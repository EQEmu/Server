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

#ifndef EQEMU_BASE_ADVENTURE_TEMPLATE_REPOSITORY_H
#define EQEMU_BASE_ADVENTURE_TEMPLATE_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseAdventureTemplateRepository {
public:
	struct AdventureTemplate {
		int         id;
		std::string zone;
		int         zone_version;
		int         is_hard;
		int         is_raid;
		int         min_level;
		int         max_level;
		int         type;
		int         type_data;
		int         type_count;
		float       assa_x;
		float       assa_y;
		float       assa_z;
		float       assa_h;
		std::string text;
		int         duration;
		int         zone_in_time;
		int         win_points;
		int         lose_points;
		int         theme;
		int         zone_in_zone_id;
		float       zone_in_x;
		float       zone_in_y;
		int         zone_in_object_id;
		float       dest_x;
		float       dest_y;
		float       dest_z;
		float       dest_h;
		int         graveyard_zone_id;
		float       graveyard_x;
		float       graveyard_y;
		float       graveyard_z;
		float       graveyard_radius;
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
			"zone_version",
			"is_hard",
			"is_raid",
			"min_level",
			"max_level",
			"type",
			"type_data",
			"type_count",
			"assa_x",
			"assa_y",
			"assa_z",
			"assa_h",
			"text",
			"duration",
			"zone_in_time",
			"win_points",
			"lose_points",
			"theme",
			"zone_in_zone_id",
			"zone_in_x",
			"zone_in_y",
			"zone_in_object_id",
			"dest_x",
			"dest_y",
			"dest_z",
			"dest_h",
			"graveyard_zone_id",
			"graveyard_x",
			"graveyard_y",
			"graveyard_z",
			"graveyard_radius",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"zone",
			"zone_version",
			"is_hard",
			"is_raid",
			"min_level",
			"max_level",
			"type",
			"type_data",
			"type_count",
			"assa_x",
			"assa_y",
			"assa_z",
			"assa_h",
			"text",
			"duration",
			"zone_in_time",
			"win_points",
			"lose_points",
			"theme",
			"zone_in_zone_id",
			"zone_in_x",
			"zone_in_y",
			"zone_in_object_id",
			"dest_x",
			"dest_y",
			"dest_z",
			"dest_h",
			"graveyard_zone_id",
			"graveyard_x",
			"graveyard_y",
			"graveyard_z",
			"graveyard_radius",
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
		return std::string("adventure_template");
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

	static AdventureTemplate NewEntity()
	{
		AdventureTemplate e{};

		e.id                = 0;
		e.zone              = "";
		e.zone_version      = 0;
		e.is_hard           = 0;
		e.is_raid           = 0;
		e.min_level         = 1;
		e.max_level         = 65;
		e.type              = 0;
		e.type_data         = 0;
		e.type_count        = 0;
		e.assa_x            = 0;
		e.assa_y            = 0;
		e.assa_z            = 0;
		e.assa_h            = 0;
		e.text              = "";
		e.duration          = 7200;
		e.zone_in_time      = 1800;
		e.win_points        = 0;
		e.lose_points       = 0;
		e.theme             = 1;
		e.zone_in_zone_id   = 0;
		e.zone_in_x         = 0;
		e.zone_in_y         = 0;
		e.zone_in_object_id = 0;
		e.dest_x            = 0;
		e.dest_y            = 0;
		e.dest_z            = 0;
		e.dest_h            = 0;
		e.graveyard_zone_id = 0;
		e.graveyard_x       = 0;
		e.graveyard_y       = 0;
		e.graveyard_z       = 0;
		e.graveyard_radius  = 0;

		return e;
	}

	static AdventureTemplate GetAdventureTemplate(
		const std::vector<AdventureTemplate> &adventure_templates,
		int adventure_template_id
	)
	{
		for (auto &adventure_template : adventure_templates) {
			if (adventure_template.id == adventure_template_id) {
				return adventure_template;
			}
		}

		return NewEntity();
	}

	static AdventureTemplate FindOne(
		Database& db,
		int adventure_template_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				adventure_template_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AdventureTemplate e{};

			e.id                = atoi(row[0]);
			e.zone              = row[1] ? row[1] : "";
			e.zone_version      = atoi(row[2]);
			e.is_hard           = atoi(row[3]);
			e.is_raid           = atoi(row[4]);
			e.min_level         = atoi(row[5]);
			e.max_level         = atoi(row[6]);
			e.type              = atoi(row[7]);
			e.type_data         = atoi(row[8]);
			e.type_count        = atoi(row[9]);
			e.assa_x            = static_cast<float>(atof(row[10]));
			e.assa_y            = static_cast<float>(atof(row[11]));
			e.assa_z            = static_cast<float>(atof(row[12]));
			e.assa_h            = static_cast<float>(atof(row[13]));
			e.text              = row[14] ? row[14] : "";
			e.duration          = atoi(row[15]);
			e.zone_in_time      = atoi(row[16]);
			e.win_points        = atoi(row[17]);
			e.lose_points       = atoi(row[18]);
			e.theme             = atoi(row[19]);
			e.zone_in_zone_id   = atoi(row[20]);
			e.zone_in_x         = static_cast<float>(atof(row[21]));
			e.zone_in_y         = static_cast<float>(atof(row[22]));
			e.zone_in_object_id = atoi(row[23]);
			e.dest_x            = static_cast<float>(atof(row[24]));
			e.dest_y            = static_cast<float>(atof(row[25]));
			e.dest_z            = static_cast<float>(atof(row[26]));
			e.dest_h            = static_cast<float>(atof(row[27]));
			e.graveyard_zone_id = atoi(row[28]);
			e.graveyard_x       = static_cast<float>(atof(row[29]));
			e.graveyard_y       = static_cast<float>(atof(row[30]));
			e.graveyard_z       = static_cast<float>(atof(row[31]));
			e.graveyard_radius  = static_cast<float>(atof(row[32]));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int adventure_template_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				adventure_template_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const AdventureTemplate &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.id));
		v.push_back(columns[1] + " = '" + Strings::Escape(e.zone) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.zone_version));
		v.push_back(columns[3] + " = " + std::to_string(e.is_hard));
		v.push_back(columns[4] + " = " + std::to_string(e.is_raid));
		v.push_back(columns[5] + " = " + std::to_string(e.min_level));
		v.push_back(columns[6] + " = " + std::to_string(e.max_level));
		v.push_back(columns[7] + " = " + std::to_string(e.type));
		v.push_back(columns[8] + " = " + std::to_string(e.type_data));
		v.push_back(columns[9] + " = " + std::to_string(e.type_count));
		v.push_back(columns[10] + " = " + std::to_string(e.assa_x));
		v.push_back(columns[11] + " = " + std::to_string(e.assa_y));
		v.push_back(columns[12] + " = " + std::to_string(e.assa_z));
		v.push_back(columns[13] + " = " + std::to_string(e.assa_h));
		v.push_back(columns[14] + " = '" + Strings::Escape(e.text) + "'");
		v.push_back(columns[15] + " = " + std::to_string(e.duration));
		v.push_back(columns[16] + " = " + std::to_string(e.zone_in_time));
		v.push_back(columns[17] + " = " + std::to_string(e.win_points));
		v.push_back(columns[18] + " = " + std::to_string(e.lose_points));
		v.push_back(columns[19] + " = " + std::to_string(e.theme));
		v.push_back(columns[20] + " = " + std::to_string(e.zone_in_zone_id));
		v.push_back(columns[21] + " = " + std::to_string(e.zone_in_x));
		v.push_back(columns[22] + " = " + std::to_string(e.zone_in_y));
		v.push_back(columns[23] + " = " + std::to_string(e.zone_in_object_id));
		v.push_back(columns[24] + " = " + std::to_string(e.dest_x));
		v.push_back(columns[25] + " = " + std::to_string(e.dest_y));
		v.push_back(columns[26] + " = " + std::to_string(e.dest_z));
		v.push_back(columns[27] + " = " + std::to_string(e.dest_h));
		v.push_back(columns[28] + " = " + std::to_string(e.graveyard_zone_id));
		v.push_back(columns[29] + " = " + std::to_string(e.graveyard_x));
		v.push_back(columns[30] + " = " + std::to_string(e.graveyard_y));
		v.push_back(columns[31] + " = " + std::to_string(e.graveyard_z));
		v.push_back(columns[32] + " = " + std::to_string(e.graveyard_radius));

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

	static AdventureTemplate InsertOne(
		Database& db,
		AdventureTemplate e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.zone) + "'");
		v.push_back(std::to_string(e.zone_version));
		v.push_back(std::to_string(e.is_hard));
		v.push_back(std::to_string(e.is_raid));
		v.push_back(std::to_string(e.min_level));
		v.push_back(std::to_string(e.max_level));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.type_data));
		v.push_back(std::to_string(e.type_count));
		v.push_back(std::to_string(e.assa_x));
		v.push_back(std::to_string(e.assa_y));
		v.push_back(std::to_string(e.assa_z));
		v.push_back(std::to_string(e.assa_h));
		v.push_back("'" + Strings::Escape(e.text) + "'");
		v.push_back(std::to_string(e.duration));
		v.push_back(std::to_string(e.zone_in_time));
		v.push_back(std::to_string(e.win_points));
		v.push_back(std::to_string(e.lose_points));
		v.push_back(std::to_string(e.theme));
		v.push_back(std::to_string(e.zone_in_zone_id));
		v.push_back(std::to_string(e.zone_in_x));
		v.push_back(std::to_string(e.zone_in_y));
		v.push_back(std::to_string(e.zone_in_object_id));
		v.push_back(std::to_string(e.dest_x));
		v.push_back(std::to_string(e.dest_y));
		v.push_back(std::to_string(e.dest_z));
		v.push_back(std::to_string(e.dest_h));
		v.push_back(std::to_string(e.graveyard_zone_id));
		v.push_back(std::to_string(e.graveyard_x));
		v.push_back(std::to_string(e.graveyard_y));
		v.push_back(std::to_string(e.graveyard_z));
		v.push_back(std::to_string(e.graveyard_radius));

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
		const std::vector<AdventureTemplate> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.zone) + "'");
			v.push_back(std::to_string(e.zone_version));
			v.push_back(std::to_string(e.is_hard));
			v.push_back(std::to_string(e.is_raid));
			v.push_back(std::to_string(e.min_level));
			v.push_back(std::to_string(e.max_level));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.type_data));
			v.push_back(std::to_string(e.type_count));
			v.push_back(std::to_string(e.assa_x));
			v.push_back(std::to_string(e.assa_y));
			v.push_back(std::to_string(e.assa_z));
			v.push_back(std::to_string(e.assa_h));
			v.push_back("'" + Strings::Escape(e.text) + "'");
			v.push_back(std::to_string(e.duration));
			v.push_back(std::to_string(e.zone_in_time));
			v.push_back(std::to_string(e.win_points));
			v.push_back(std::to_string(e.lose_points));
			v.push_back(std::to_string(e.theme));
			v.push_back(std::to_string(e.zone_in_zone_id));
			v.push_back(std::to_string(e.zone_in_x));
			v.push_back(std::to_string(e.zone_in_y));
			v.push_back(std::to_string(e.zone_in_object_id));
			v.push_back(std::to_string(e.dest_x));
			v.push_back(std::to_string(e.dest_y));
			v.push_back(std::to_string(e.dest_z));
			v.push_back(std::to_string(e.dest_h));
			v.push_back(std::to_string(e.graveyard_zone_id));
			v.push_back(std::to_string(e.graveyard_x));
			v.push_back(std::to_string(e.graveyard_y));
			v.push_back(std::to_string(e.graveyard_z));
			v.push_back(std::to_string(e.graveyard_radius));

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

	static std::vector<AdventureTemplate> All(Database& db)
	{
		std::vector<AdventureTemplate> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AdventureTemplate e{};

			e.id                = atoi(row[0]);
			e.zone              = row[1] ? row[1] : "";
			e.zone_version      = atoi(row[2]);
			e.is_hard           = atoi(row[3]);
			e.is_raid           = atoi(row[4]);
			e.min_level         = atoi(row[5]);
			e.max_level         = atoi(row[6]);
			e.type              = atoi(row[7]);
			e.type_data         = atoi(row[8]);
			e.type_count        = atoi(row[9]);
			e.assa_x            = static_cast<float>(atof(row[10]));
			e.assa_y            = static_cast<float>(atof(row[11]));
			e.assa_z            = static_cast<float>(atof(row[12]));
			e.assa_h            = static_cast<float>(atof(row[13]));
			e.text              = row[14] ? row[14] : "";
			e.duration          = atoi(row[15]);
			e.zone_in_time      = atoi(row[16]);
			e.win_points        = atoi(row[17]);
			e.lose_points       = atoi(row[18]);
			e.theme             = atoi(row[19]);
			e.zone_in_zone_id   = atoi(row[20]);
			e.zone_in_x         = static_cast<float>(atof(row[21]));
			e.zone_in_y         = static_cast<float>(atof(row[22]));
			e.zone_in_object_id = atoi(row[23]);
			e.dest_x            = static_cast<float>(atof(row[24]));
			e.dest_y            = static_cast<float>(atof(row[25]));
			e.dest_z            = static_cast<float>(atof(row[26]));
			e.dest_h            = static_cast<float>(atof(row[27]));
			e.graveyard_zone_id = atoi(row[28]);
			e.graveyard_x       = static_cast<float>(atof(row[29]));
			e.graveyard_y       = static_cast<float>(atof(row[30]));
			e.graveyard_z       = static_cast<float>(atof(row[31]));
			e.graveyard_radius  = static_cast<float>(atof(row[32]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<AdventureTemplate> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<AdventureTemplate> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AdventureTemplate e{};

			e.id                = atoi(row[0]);
			e.zone              = row[1] ? row[1] : "";
			e.zone_version      = atoi(row[2]);
			e.is_hard           = atoi(row[3]);
			e.is_raid           = atoi(row[4]);
			e.min_level         = atoi(row[5]);
			e.max_level         = atoi(row[6]);
			e.type              = atoi(row[7]);
			e.type_data         = atoi(row[8]);
			e.type_count        = atoi(row[9]);
			e.assa_x            = static_cast<float>(atof(row[10]));
			e.assa_y            = static_cast<float>(atof(row[11]));
			e.assa_z            = static_cast<float>(atof(row[12]));
			e.assa_h            = static_cast<float>(atof(row[13]));
			e.text              = row[14] ? row[14] : "";
			e.duration          = atoi(row[15]);
			e.zone_in_time      = atoi(row[16]);
			e.win_points        = atoi(row[17]);
			e.lose_points       = atoi(row[18]);
			e.theme             = atoi(row[19]);
			e.zone_in_zone_id   = atoi(row[20]);
			e.zone_in_x         = static_cast<float>(atof(row[21]));
			e.zone_in_y         = static_cast<float>(atof(row[22]));
			e.zone_in_object_id = atoi(row[23]);
			e.dest_x            = static_cast<float>(atof(row[24]));
			e.dest_y            = static_cast<float>(atof(row[25]));
			e.dest_z            = static_cast<float>(atof(row[26]));
			e.dest_h            = static_cast<float>(atof(row[27]));
			e.graveyard_zone_id = atoi(row[28]);
			e.graveyard_x       = static_cast<float>(atof(row[29]));
			e.graveyard_y       = static_cast<float>(atof(row[30]));
			e.graveyard_z       = static_cast<float>(atof(row[31]));
			e.graveyard_radius  = static_cast<float>(atof(row[32]));

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

#endif //EQEMU_BASE_ADVENTURE_TEMPLATE_REPOSITORY_H
