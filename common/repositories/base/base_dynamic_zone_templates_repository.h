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
		DynamicZoneTemplates entry{};

		entry.id               = 0;
		entry.zone_id          = 0;
		entry.zone_version     = 0;
		entry.name             = "";
		entry.min_players      = 0;
		entry.max_players      = 0;
		entry.duration_seconds = 0;
		entry.dz_switch_id     = 0;
		entry.compass_zone_id  = 0;
		entry.compass_x        = 0;
		entry.compass_y        = 0;
		entry.compass_z        = 0;
		entry.return_zone_id   = 0;
		entry.return_x         = 0;
		entry.return_y         = 0;
		entry.return_z         = 0;
		entry.return_h         = 0;
		entry.override_zone_in = 0;
		entry.zone_in_x        = 0;
		entry.zone_in_y        = 0;
		entry.zone_in_z        = 0;
		entry.zone_in_h        = 0;

		return entry;
	}

	static DynamicZoneTemplates GetDynamicZoneTemplatesEntry(
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
			DynamicZoneTemplates entry{};

			entry.id               = atoi(row[0]);
			entry.zone_id          = atoi(row[1]);
			entry.zone_version     = atoi(row[2]);
			entry.name             = row[3] ? row[3] : "";
			entry.min_players      = atoi(row[4]);
			entry.max_players      = atoi(row[5]);
			entry.duration_seconds = atoi(row[6]);
			entry.dz_switch_id     = atoi(row[7]);
			entry.compass_zone_id  = atoi(row[8]);
			entry.compass_x        = static_cast<float>(atof(row[9]));
			entry.compass_y        = static_cast<float>(atof(row[10]));
			entry.compass_z        = static_cast<float>(atof(row[11]));
			entry.return_zone_id   = atoi(row[12]);
			entry.return_x         = static_cast<float>(atof(row[13]));
			entry.return_y         = static_cast<float>(atof(row[14]));
			entry.return_z         = static_cast<float>(atof(row[15]));
			entry.return_h         = static_cast<float>(atof(row[16]));
			entry.override_zone_in = atoi(row[17]);
			entry.zone_in_x        = static_cast<float>(atof(row[18]));
			entry.zone_in_y        = static_cast<float>(atof(row[19]));
			entry.zone_in_z        = static_cast<float>(atof(row[20]));
			entry.zone_in_h        = static_cast<float>(atof(row[21]));

			return entry;
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
		DynamicZoneTemplates dynamic_zone_templates_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(dynamic_zone_templates_entry.zone_id));
		update_values.push_back(columns[2] + " = " + std::to_string(dynamic_zone_templates_entry.zone_version));
		update_values.push_back(columns[3] + " = '" + Strings::Escape(dynamic_zone_templates_entry.name) + "'");
		update_values.push_back(columns[4] + " = " + std::to_string(dynamic_zone_templates_entry.min_players));
		update_values.push_back(columns[5] + " = " + std::to_string(dynamic_zone_templates_entry.max_players));
		update_values.push_back(columns[6] + " = " + std::to_string(dynamic_zone_templates_entry.duration_seconds));
		update_values.push_back(columns[7] + " = " + std::to_string(dynamic_zone_templates_entry.dz_switch_id));
		update_values.push_back(columns[8] + " = " + std::to_string(dynamic_zone_templates_entry.compass_zone_id));
		update_values.push_back(columns[9] + " = " + std::to_string(dynamic_zone_templates_entry.compass_x));
		update_values.push_back(columns[10] + " = " + std::to_string(dynamic_zone_templates_entry.compass_y));
		update_values.push_back(columns[11] + " = " + std::to_string(dynamic_zone_templates_entry.compass_z));
		update_values.push_back(columns[12] + " = " + std::to_string(dynamic_zone_templates_entry.return_zone_id));
		update_values.push_back(columns[13] + " = " + std::to_string(dynamic_zone_templates_entry.return_x));
		update_values.push_back(columns[14] + " = " + std::to_string(dynamic_zone_templates_entry.return_y));
		update_values.push_back(columns[15] + " = " + std::to_string(dynamic_zone_templates_entry.return_z));
		update_values.push_back(columns[16] + " = " + std::to_string(dynamic_zone_templates_entry.return_h));
		update_values.push_back(columns[17] + " = " + std::to_string(dynamic_zone_templates_entry.override_zone_in));
		update_values.push_back(columns[18] + " = " + std::to_string(dynamic_zone_templates_entry.zone_in_x));
		update_values.push_back(columns[19] + " = " + std::to_string(dynamic_zone_templates_entry.zone_in_y));
		update_values.push_back(columns[20] + " = " + std::to_string(dynamic_zone_templates_entry.zone_in_z));
		update_values.push_back(columns[21] + " = " + std::to_string(dynamic_zone_templates_entry.zone_in_h));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				dynamic_zone_templates_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static DynamicZoneTemplates InsertOne(
		Database& db,
		DynamicZoneTemplates dynamic_zone_templates_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(dynamic_zone_templates_entry.id));
		insert_values.push_back(std::to_string(dynamic_zone_templates_entry.zone_id));
		insert_values.push_back(std::to_string(dynamic_zone_templates_entry.zone_version));
		insert_values.push_back("'" + Strings::Escape(dynamic_zone_templates_entry.name) + "'");
		insert_values.push_back(std::to_string(dynamic_zone_templates_entry.min_players));
		insert_values.push_back(std::to_string(dynamic_zone_templates_entry.max_players));
		insert_values.push_back(std::to_string(dynamic_zone_templates_entry.duration_seconds));
		insert_values.push_back(std::to_string(dynamic_zone_templates_entry.dz_switch_id));
		insert_values.push_back(std::to_string(dynamic_zone_templates_entry.compass_zone_id));
		insert_values.push_back(std::to_string(dynamic_zone_templates_entry.compass_x));
		insert_values.push_back(std::to_string(dynamic_zone_templates_entry.compass_y));
		insert_values.push_back(std::to_string(dynamic_zone_templates_entry.compass_z));
		insert_values.push_back(std::to_string(dynamic_zone_templates_entry.return_zone_id));
		insert_values.push_back(std::to_string(dynamic_zone_templates_entry.return_x));
		insert_values.push_back(std::to_string(dynamic_zone_templates_entry.return_y));
		insert_values.push_back(std::to_string(dynamic_zone_templates_entry.return_z));
		insert_values.push_back(std::to_string(dynamic_zone_templates_entry.return_h));
		insert_values.push_back(std::to_string(dynamic_zone_templates_entry.override_zone_in));
		insert_values.push_back(std::to_string(dynamic_zone_templates_entry.zone_in_x));
		insert_values.push_back(std::to_string(dynamic_zone_templates_entry.zone_in_y));
		insert_values.push_back(std::to_string(dynamic_zone_templates_entry.zone_in_z));
		insert_values.push_back(std::to_string(dynamic_zone_templates_entry.zone_in_h));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			dynamic_zone_templates_entry.id = results.LastInsertedID();
			return dynamic_zone_templates_entry;
		}

		dynamic_zone_templates_entry = NewEntity();

		return dynamic_zone_templates_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<DynamicZoneTemplates> dynamic_zone_templates_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &dynamic_zone_templates_entry: dynamic_zone_templates_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(dynamic_zone_templates_entry.id));
			insert_values.push_back(std::to_string(dynamic_zone_templates_entry.zone_id));
			insert_values.push_back(std::to_string(dynamic_zone_templates_entry.zone_version));
			insert_values.push_back("'" + Strings::Escape(dynamic_zone_templates_entry.name) + "'");
			insert_values.push_back(std::to_string(dynamic_zone_templates_entry.min_players));
			insert_values.push_back(std::to_string(dynamic_zone_templates_entry.max_players));
			insert_values.push_back(std::to_string(dynamic_zone_templates_entry.duration_seconds));
			insert_values.push_back(std::to_string(dynamic_zone_templates_entry.dz_switch_id));
			insert_values.push_back(std::to_string(dynamic_zone_templates_entry.compass_zone_id));
			insert_values.push_back(std::to_string(dynamic_zone_templates_entry.compass_x));
			insert_values.push_back(std::to_string(dynamic_zone_templates_entry.compass_y));
			insert_values.push_back(std::to_string(dynamic_zone_templates_entry.compass_z));
			insert_values.push_back(std::to_string(dynamic_zone_templates_entry.return_zone_id));
			insert_values.push_back(std::to_string(dynamic_zone_templates_entry.return_x));
			insert_values.push_back(std::to_string(dynamic_zone_templates_entry.return_y));
			insert_values.push_back(std::to_string(dynamic_zone_templates_entry.return_z));
			insert_values.push_back(std::to_string(dynamic_zone_templates_entry.return_h));
			insert_values.push_back(std::to_string(dynamic_zone_templates_entry.override_zone_in));
			insert_values.push_back(std::to_string(dynamic_zone_templates_entry.zone_in_x));
			insert_values.push_back(std::to_string(dynamic_zone_templates_entry.zone_in_y));
			insert_values.push_back(std::to_string(dynamic_zone_templates_entry.zone_in_z));
			insert_values.push_back(std::to_string(dynamic_zone_templates_entry.zone_in_h));

			insert_chunks.push_back("(" + Strings::Implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

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
			DynamicZoneTemplates entry{};

			entry.id               = atoi(row[0]);
			entry.zone_id          = atoi(row[1]);
			entry.zone_version     = atoi(row[2]);
			entry.name             = row[3] ? row[3] : "";
			entry.min_players      = atoi(row[4]);
			entry.max_players      = atoi(row[5]);
			entry.duration_seconds = atoi(row[6]);
			entry.dz_switch_id     = atoi(row[7]);
			entry.compass_zone_id  = atoi(row[8]);
			entry.compass_x        = static_cast<float>(atof(row[9]));
			entry.compass_y        = static_cast<float>(atof(row[10]));
			entry.compass_z        = static_cast<float>(atof(row[11]));
			entry.return_zone_id   = atoi(row[12]);
			entry.return_x         = static_cast<float>(atof(row[13]));
			entry.return_y         = static_cast<float>(atof(row[14]));
			entry.return_z         = static_cast<float>(atof(row[15]));
			entry.return_h         = static_cast<float>(atof(row[16]));
			entry.override_zone_in = atoi(row[17]);
			entry.zone_in_x        = static_cast<float>(atof(row[18]));
			entry.zone_in_y        = static_cast<float>(atof(row[19]));
			entry.zone_in_z        = static_cast<float>(atof(row[20]));
			entry.zone_in_h        = static_cast<float>(atof(row[21]));

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<DynamicZoneTemplates> GetWhere(Database& db, std::string where_filter)
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
			DynamicZoneTemplates entry{};

			entry.id               = atoi(row[0]);
			entry.zone_id          = atoi(row[1]);
			entry.zone_version     = atoi(row[2]);
			entry.name             = row[3] ? row[3] : "";
			entry.min_players      = atoi(row[4]);
			entry.max_players      = atoi(row[5]);
			entry.duration_seconds = atoi(row[6]);
			entry.dz_switch_id     = atoi(row[7]);
			entry.compass_zone_id  = atoi(row[8]);
			entry.compass_x        = static_cast<float>(atof(row[9]));
			entry.compass_y        = static_cast<float>(atof(row[10]));
			entry.compass_z        = static_cast<float>(atof(row[11]));
			entry.return_zone_id   = atoi(row[12]);
			entry.return_x         = static_cast<float>(atof(row[13]));
			entry.return_y         = static_cast<float>(atof(row[14]));
			entry.return_z         = static_cast<float>(atof(row[15]));
			entry.return_h         = static_cast<float>(atof(row[16]));
			entry.override_zone_in = atoi(row[17]);
			entry.zone_in_x        = static_cast<float>(atof(row[18]));
			entry.zone_in_y        = static_cast<float>(atof(row[19]));
			entry.zone_in_z        = static_cast<float>(atof(row[20]));
			entry.zone_in_h        = static_cast<float>(atof(row[21]));

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

#endif //EQEMU_BASE_DYNAMIC_ZONE_TEMPLATES_REPOSITORY_H
