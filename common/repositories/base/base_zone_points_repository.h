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

#ifndef EQEMU_BASE_ZONE_POINTS_REPOSITORY_H
#define EQEMU_BASE_ZONE_POINTS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseZonePointsRepository {
public:
	struct ZonePoints {
		int         id;
		std::string zone;
		int         version;
		int         number;
		float       y;
		float       x;
		float       z;
		float       heading;
		float       target_y;
		float       target_x;
		float       target_z;
		float       target_heading;
		int         zoneinst;
		int         target_zone_id;
		int         target_instance;
		float       buffer;
		int         client_version_mask;
		int         min_expansion;
		int         max_expansion;
		std::string content_flags;
		std::string content_flags_disabled;
		int         is_virtual;
		int         height;
		int         width;
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
			"number",
			"y",
			"x",
			"z",
			"heading",
			"target_y",
			"target_x",
			"target_z",
			"target_heading",
			"zoneinst",
			"target_zone_id",
			"target_instance",
			"buffer",
			"client_version_mask",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
			"is_virtual",
			"height",
			"width",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"zone",
			"version",
			"number",
			"y",
			"x",
			"z",
			"heading",
			"target_y",
			"target_x",
			"target_z",
			"target_heading",
			"zoneinst",
			"target_zone_id",
			"target_instance",
			"buffer",
			"client_version_mask",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
			"is_virtual",
			"height",
			"width",
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
		return std::string("zone_points");
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

	static ZonePoints NewEntity()
	{
		ZonePoints e{};

		e.id                     = 0;
		e.zone                   = "";
		e.version                = 0;
		e.number                 = 1;
		e.y                      = 0;
		e.x                      = 0;
		e.z                      = 0;
		e.heading                = 0;
		e.target_y               = 0;
		e.target_x               = 0;
		e.target_z               = 0;
		e.target_heading         = 0;
		e.zoneinst               = 0;
		e.target_zone_id         = 0;
		e.target_instance        = 0;
		e.buffer                 = 0;
		e.client_version_mask    = 4294967295;
		e.min_expansion          = -1;
		e.max_expansion          = -1;
		e.content_flags          = "";
		e.content_flags_disabled = "";
		e.is_virtual             = 0;
		e.height                 = 0;
		e.width                  = 0;

		return e;
	}

	static ZonePoints GetZonePointse(
		const std::vector<ZonePoints> &zone_pointss,
		int zone_points_id
	)
	{
		for (auto &zone_points : zone_pointss) {
			if (zone_points.id == zone_points_id) {
				return zone_points;
			}
		}

		return NewEntity();
	}

	static ZonePoints FindOne(
		Database& db,
		int zone_points_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				zone_points_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			ZonePoints e{};

			e.id                     = atoi(row[0]);
			e.zone                   = row[1] ? row[1] : "";
			e.version                = atoi(row[2]);
			e.number                 = atoi(row[3]);
			e.y                      = static_cast<float>(atof(row[4]));
			e.x                      = static_cast<float>(atof(row[5]));
			e.z                      = static_cast<float>(atof(row[6]));
			e.heading                = static_cast<float>(atof(row[7]));
			e.target_y               = static_cast<float>(atof(row[8]));
			e.target_x               = static_cast<float>(atof(row[9]));
			e.target_z               = static_cast<float>(atof(row[10]));
			e.target_heading         = static_cast<float>(atof(row[11]));
			e.zoneinst               = atoi(row[12]);
			e.target_zone_id         = atoi(row[13]);
			e.target_instance        = atoi(row[14]);
			e.buffer                 = static_cast<float>(atof(row[15]));
			e.client_version_mask    = atoi(row[16]);
			e.min_expansion          = atoi(row[17]);
			e.max_expansion          = atoi(row[18]);
			e.content_flags          = row[19] ? row[19] : "";
			e.content_flags_disabled = row[20] ? row[20] : "";
			e.is_virtual             = atoi(row[21]);
			e.height                 = atoi(row[22]);
			e.width                  = atoi(row[23]);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int zone_points_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				zone_points_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		ZonePoints zone_points_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + Strings::Escape(zone_points_e.zone) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(zone_points_e.version));
		update_values.push_back(columns[3] + " = " + std::to_string(zone_points_e.number));
		update_values.push_back(columns[4] + " = " + std::to_string(zone_points_e.y));
		update_values.push_back(columns[5] + " = " + std::to_string(zone_points_e.x));
		update_values.push_back(columns[6] + " = " + std::to_string(zone_points_e.z));
		update_values.push_back(columns[7] + " = " + std::to_string(zone_points_e.heading));
		update_values.push_back(columns[8] + " = " + std::to_string(zone_points_e.target_y));
		update_values.push_back(columns[9] + " = " + std::to_string(zone_points_e.target_x));
		update_values.push_back(columns[10] + " = " + std::to_string(zone_points_e.target_z));
		update_values.push_back(columns[11] + " = " + std::to_string(zone_points_e.target_heading));
		update_values.push_back(columns[12] + " = " + std::to_string(zone_points_e.zoneinst));
		update_values.push_back(columns[13] + " = " + std::to_string(zone_points_e.target_zone_id));
		update_values.push_back(columns[14] + " = " + std::to_string(zone_points_e.target_instance));
		update_values.push_back(columns[15] + " = " + std::to_string(zone_points_e.buffer));
		update_values.push_back(columns[16] + " = " + std::to_string(zone_points_e.client_version_mask));
		update_values.push_back(columns[17] + " = " + std::to_string(zone_points_e.min_expansion));
		update_values.push_back(columns[18] + " = " + std::to_string(zone_points_e.max_expansion));
		update_values.push_back(columns[19] + " = '" + Strings::Escape(zone_points_e.content_flags) + "'");
		update_values.push_back(columns[20] + " = '" + Strings::Escape(zone_points_e.content_flags_disabled) + "'");
		update_values.push_back(columns[21] + " = " + std::to_string(zone_points_e.is_virtual));
		update_values.push_back(columns[22] + " = " + std::to_string(zone_points_e.height));
		update_values.push_back(columns[23] + " = " + std::to_string(zone_points_e.width));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				zone_points_e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static ZonePoints InsertOne(
		Database& db,
		ZonePoints zone_points_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(zone_points_e.id));
		insert_values.push_back("'" + Strings::Escape(zone_points_e.zone) + "'");
		insert_values.push_back(std::to_string(zone_points_e.version));
		insert_values.push_back(std::to_string(zone_points_e.number));
		insert_values.push_back(std::to_string(zone_points_e.y));
		insert_values.push_back(std::to_string(zone_points_e.x));
		insert_values.push_back(std::to_string(zone_points_e.z));
		insert_values.push_back(std::to_string(zone_points_e.heading));
		insert_values.push_back(std::to_string(zone_points_e.target_y));
		insert_values.push_back(std::to_string(zone_points_e.target_x));
		insert_values.push_back(std::to_string(zone_points_e.target_z));
		insert_values.push_back(std::to_string(zone_points_e.target_heading));
		insert_values.push_back(std::to_string(zone_points_e.zoneinst));
		insert_values.push_back(std::to_string(zone_points_e.target_zone_id));
		insert_values.push_back(std::to_string(zone_points_e.target_instance));
		insert_values.push_back(std::to_string(zone_points_e.buffer));
		insert_values.push_back(std::to_string(zone_points_e.client_version_mask));
		insert_values.push_back(std::to_string(zone_points_e.min_expansion));
		insert_values.push_back(std::to_string(zone_points_e.max_expansion));
		insert_values.push_back("'" + Strings::Escape(zone_points_e.content_flags) + "'");
		insert_values.push_back("'" + Strings::Escape(zone_points_e.content_flags_disabled) + "'");
		insert_values.push_back(std::to_string(zone_points_e.is_virtual));
		insert_values.push_back(std::to_string(zone_points_e.height));
		insert_values.push_back(std::to_string(zone_points_e.width));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			zone_points_e.id = results.LastInsertedID();
			return zone_points_e;
		}

		zone_points_e = NewEntity();

		return zone_points_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<ZonePoints> zone_points_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &zone_points_e: zone_points_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(zone_points_e.id));
			insert_values.push_back("'" + Strings::Escape(zone_points_e.zone) + "'");
			insert_values.push_back(std::to_string(zone_points_e.version));
			insert_values.push_back(std::to_string(zone_points_e.number));
			insert_values.push_back(std::to_string(zone_points_e.y));
			insert_values.push_back(std::to_string(zone_points_e.x));
			insert_values.push_back(std::to_string(zone_points_e.z));
			insert_values.push_back(std::to_string(zone_points_e.heading));
			insert_values.push_back(std::to_string(zone_points_e.target_y));
			insert_values.push_back(std::to_string(zone_points_e.target_x));
			insert_values.push_back(std::to_string(zone_points_e.target_z));
			insert_values.push_back(std::to_string(zone_points_e.target_heading));
			insert_values.push_back(std::to_string(zone_points_e.zoneinst));
			insert_values.push_back(std::to_string(zone_points_e.target_zone_id));
			insert_values.push_back(std::to_string(zone_points_e.target_instance));
			insert_values.push_back(std::to_string(zone_points_e.buffer));
			insert_values.push_back(std::to_string(zone_points_e.client_version_mask));
			insert_values.push_back(std::to_string(zone_points_e.min_expansion));
			insert_values.push_back(std::to_string(zone_points_e.max_expansion));
			insert_values.push_back("'" + Strings::Escape(zone_points_e.content_flags) + "'");
			insert_values.push_back("'" + Strings::Escape(zone_points_e.content_flags_disabled) + "'");
			insert_values.push_back(std::to_string(zone_points_e.is_virtual));
			insert_values.push_back(std::to_string(zone_points_e.height));
			insert_values.push_back(std::to_string(zone_points_e.width));

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

	static std::vector<ZonePoints> All(Database& db)
	{
		std::vector<ZonePoints> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ZonePoints e{};

			e.id                     = atoi(row[0]);
			e.zone                   = row[1] ? row[1] : "";
			e.version                = atoi(row[2]);
			e.number                 = atoi(row[3]);
			e.y                      = static_cast<float>(atof(row[4]));
			e.x                      = static_cast<float>(atof(row[5]));
			e.z                      = static_cast<float>(atof(row[6]));
			e.heading                = static_cast<float>(atof(row[7]));
			e.target_y               = static_cast<float>(atof(row[8]));
			e.target_x               = static_cast<float>(atof(row[9]));
			e.target_z               = static_cast<float>(atof(row[10]));
			e.target_heading         = static_cast<float>(atof(row[11]));
			e.zoneinst               = atoi(row[12]);
			e.target_zone_id         = atoi(row[13]);
			e.target_instance        = atoi(row[14]);
			e.buffer                 = static_cast<float>(atof(row[15]));
			e.client_version_mask    = atoi(row[16]);
			e.min_expansion          = atoi(row[17]);
			e.max_expansion          = atoi(row[18]);
			e.content_flags          = row[19] ? row[19] : "";
			e.content_flags_disabled = row[20] ? row[20] : "";
			e.is_virtual             = atoi(row[21]);
			e.height                 = atoi(row[22]);
			e.width                  = atoi(row[23]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<ZonePoints> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<ZonePoints> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ZonePoints e{};

			e.id                     = atoi(row[0]);
			e.zone                   = row[1] ? row[1] : "";
			e.version                = atoi(row[2]);
			e.number                 = atoi(row[3]);
			e.y                      = static_cast<float>(atof(row[4]));
			e.x                      = static_cast<float>(atof(row[5]));
			e.z                      = static_cast<float>(atof(row[6]));
			e.heading                = static_cast<float>(atof(row[7]));
			e.target_y               = static_cast<float>(atof(row[8]));
			e.target_x               = static_cast<float>(atof(row[9]));
			e.target_z               = static_cast<float>(atof(row[10]));
			e.target_heading         = static_cast<float>(atof(row[11]));
			e.zoneinst               = atoi(row[12]);
			e.target_zone_id         = atoi(row[13]);
			e.target_instance        = atoi(row[14]);
			e.buffer                 = static_cast<float>(atof(row[15]));
			e.client_version_mask    = atoi(row[16]);
			e.min_expansion          = atoi(row[17]);
			e.max_expansion          = atoi(row[18]);
			e.content_flags          = row[19] ? row[19] : "";
			e.content_flags_disabled = row[20] ? row[20] : "";
			e.is_virtual             = atoi(row[21]);
			e.height                 = atoi(row[22]);
			e.width                  = atoi(row[23]);

			all_entries.push_back(e);
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

	static int64 GetMaxId(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT MAX({}) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string& where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

};

#endif //EQEMU_BASE_ZONE_POINTS_REPOSITORY_H
