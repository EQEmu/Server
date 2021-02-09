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

#ifndef EQEMU_BASE_DYNAMIC_ZONES_REPOSITORY_H
#define EQEMU_BASE_DYNAMIC_ZONES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseDynamicZonesRepository {
public:
	struct DynamicZones {
		int   id;
		int   instance_id;
		int   type;
		int   compass_zone_id;
		float compass_x;
		float compass_y;
		float compass_z;
		int   safe_return_zone_id;
		float safe_return_x;
		float safe_return_y;
		float safe_return_z;
		float safe_return_heading;
		float zone_in_x;
		float zone_in_y;
		float zone_in_z;
		float zone_in_heading;
		int   has_zone_in;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"instance_id",
			"type",
			"compass_zone_id",
			"compass_x",
			"compass_y",
			"compass_z",
			"safe_return_zone_id",
			"safe_return_x",
			"safe_return_y",
			"safe_return_z",
			"safe_return_heading",
			"zone_in_x",
			"zone_in_y",
			"zone_in_z",
			"zone_in_heading",
			"has_zone_in",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("dynamic_zones");
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

	static DynamicZones NewEntity()
	{
		DynamicZones entry{};

		entry.id                  = 0;
		entry.instance_id         = 0;
		entry.type                = 0;
		entry.compass_zone_id     = 0;
		entry.compass_x           = 0;
		entry.compass_y           = 0;
		entry.compass_z           = 0;
		entry.safe_return_zone_id = 0;
		entry.safe_return_x       = 0;
		entry.safe_return_y       = 0;
		entry.safe_return_z       = 0;
		entry.safe_return_heading = 0;
		entry.zone_in_x           = 0;
		entry.zone_in_y           = 0;
		entry.zone_in_z           = 0;
		entry.zone_in_heading     = 0;
		entry.has_zone_in         = 0;

		return entry;
	}

	static DynamicZones GetDynamicZonesEntry(
		const std::vector<DynamicZones> &dynamic_zoness,
		int dynamic_zones_id
	)
	{
		for (auto &dynamic_zones : dynamic_zoness) {
			if (dynamic_zones.id == dynamic_zones_id) {
				return dynamic_zones;
			}
		}

		return NewEntity();
	}

	static DynamicZones FindOne(
		Database& db,
		int dynamic_zones_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				dynamic_zones_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			DynamicZones entry{};

			entry.id                  = atoi(row[0]);
			entry.instance_id         = atoi(row[1]);
			entry.type                = atoi(row[2]);
			entry.compass_zone_id     = atoi(row[3]);
			entry.compass_x           = static_cast<float>(atof(row[4]));
			entry.compass_y           = static_cast<float>(atof(row[5]));
			entry.compass_z           = static_cast<float>(atof(row[6]));
			entry.safe_return_zone_id = atoi(row[7]);
			entry.safe_return_x       = static_cast<float>(atof(row[8]));
			entry.safe_return_y       = static_cast<float>(atof(row[9]));
			entry.safe_return_z       = static_cast<float>(atof(row[10]));
			entry.safe_return_heading = static_cast<float>(atof(row[11]));
			entry.zone_in_x           = static_cast<float>(atof(row[12]));
			entry.zone_in_y           = static_cast<float>(atof(row[13]));
			entry.zone_in_z           = static_cast<float>(atof(row[14]));
			entry.zone_in_heading     = static_cast<float>(atof(row[15]));
			entry.has_zone_in         = atoi(row[16]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int dynamic_zones_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				dynamic_zones_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		DynamicZones dynamic_zones_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(dynamic_zones_entry.instance_id));
		update_values.push_back(columns[2] + " = " + std::to_string(dynamic_zones_entry.type));
		update_values.push_back(columns[3] + " = " + std::to_string(dynamic_zones_entry.compass_zone_id));
		update_values.push_back(columns[4] + " = " + std::to_string(dynamic_zones_entry.compass_x));
		update_values.push_back(columns[5] + " = " + std::to_string(dynamic_zones_entry.compass_y));
		update_values.push_back(columns[6] + " = " + std::to_string(dynamic_zones_entry.compass_z));
		update_values.push_back(columns[7] + " = " + std::to_string(dynamic_zones_entry.safe_return_zone_id));
		update_values.push_back(columns[8] + " = " + std::to_string(dynamic_zones_entry.safe_return_x));
		update_values.push_back(columns[9] + " = " + std::to_string(dynamic_zones_entry.safe_return_y));
		update_values.push_back(columns[10] + " = " + std::to_string(dynamic_zones_entry.safe_return_z));
		update_values.push_back(columns[11] + " = " + std::to_string(dynamic_zones_entry.safe_return_heading));
		update_values.push_back(columns[12] + " = " + std::to_string(dynamic_zones_entry.zone_in_x));
		update_values.push_back(columns[13] + " = " + std::to_string(dynamic_zones_entry.zone_in_y));
		update_values.push_back(columns[14] + " = " + std::to_string(dynamic_zones_entry.zone_in_z));
		update_values.push_back(columns[15] + " = " + std::to_string(dynamic_zones_entry.zone_in_heading));
		update_values.push_back(columns[16] + " = " + std::to_string(dynamic_zones_entry.has_zone_in));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				dynamic_zones_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static DynamicZones InsertOne(
		Database& db,
		DynamicZones dynamic_zones_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(dynamic_zones_entry.id));
		insert_values.push_back(std::to_string(dynamic_zones_entry.instance_id));
		insert_values.push_back(std::to_string(dynamic_zones_entry.type));
		insert_values.push_back(std::to_string(dynamic_zones_entry.compass_zone_id));
		insert_values.push_back(std::to_string(dynamic_zones_entry.compass_x));
		insert_values.push_back(std::to_string(dynamic_zones_entry.compass_y));
		insert_values.push_back(std::to_string(dynamic_zones_entry.compass_z));
		insert_values.push_back(std::to_string(dynamic_zones_entry.safe_return_zone_id));
		insert_values.push_back(std::to_string(dynamic_zones_entry.safe_return_x));
		insert_values.push_back(std::to_string(dynamic_zones_entry.safe_return_y));
		insert_values.push_back(std::to_string(dynamic_zones_entry.safe_return_z));
		insert_values.push_back(std::to_string(dynamic_zones_entry.safe_return_heading));
		insert_values.push_back(std::to_string(dynamic_zones_entry.zone_in_x));
		insert_values.push_back(std::to_string(dynamic_zones_entry.zone_in_y));
		insert_values.push_back(std::to_string(dynamic_zones_entry.zone_in_z));
		insert_values.push_back(std::to_string(dynamic_zones_entry.zone_in_heading));
		insert_values.push_back(std::to_string(dynamic_zones_entry.has_zone_in));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			dynamic_zones_entry.id = results.LastInsertedID();
			return dynamic_zones_entry;
		}

		dynamic_zones_entry = NewEntity();

		return dynamic_zones_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<DynamicZones> dynamic_zones_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &dynamic_zones_entry: dynamic_zones_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(dynamic_zones_entry.id));
			insert_values.push_back(std::to_string(dynamic_zones_entry.instance_id));
			insert_values.push_back(std::to_string(dynamic_zones_entry.type));
			insert_values.push_back(std::to_string(dynamic_zones_entry.compass_zone_id));
			insert_values.push_back(std::to_string(dynamic_zones_entry.compass_x));
			insert_values.push_back(std::to_string(dynamic_zones_entry.compass_y));
			insert_values.push_back(std::to_string(dynamic_zones_entry.compass_z));
			insert_values.push_back(std::to_string(dynamic_zones_entry.safe_return_zone_id));
			insert_values.push_back(std::to_string(dynamic_zones_entry.safe_return_x));
			insert_values.push_back(std::to_string(dynamic_zones_entry.safe_return_y));
			insert_values.push_back(std::to_string(dynamic_zones_entry.safe_return_z));
			insert_values.push_back(std::to_string(dynamic_zones_entry.safe_return_heading));
			insert_values.push_back(std::to_string(dynamic_zones_entry.zone_in_x));
			insert_values.push_back(std::to_string(dynamic_zones_entry.zone_in_y));
			insert_values.push_back(std::to_string(dynamic_zones_entry.zone_in_z));
			insert_values.push_back(std::to_string(dynamic_zones_entry.zone_in_heading));
			insert_values.push_back(std::to_string(dynamic_zones_entry.has_zone_in));

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

	static std::vector<DynamicZones> All(Database& db)
	{
		std::vector<DynamicZones> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DynamicZones entry{};

			entry.id                  = atoi(row[0]);
			entry.instance_id         = atoi(row[1]);
			entry.type                = atoi(row[2]);
			entry.compass_zone_id     = atoi(row[3]);
			entry.compass_x           = static_cast<float>(atof(row[4]));
			entry.compass_y           = static_cast<float>(atof(row[5]));
			entry.compass_z           = static_cast<float>(atof(row[6]));
			entry.safe_return_zone_id = atoi(row[7]);
			entry.safe_return_x       = static_cast<float>(atof(row[8]));
			entry.safe_return_y       = static_cast<float>(atof(row[9]));
			entry.safe_return_z       = static_cast<float>(atof(row[10]));
			entry.safe_return_heading = static_cast<float>(atof(row[11]));
			entry.zone_in_x           = static_cast<float>(atof(row[12]));
			entry.zone_in_y           = static_cast<float>(atof(row[13]));
			entry.zone_in_z           = static_cast<float>(atof(row[14]));
			entry.zone_in_heading     = static_cast<float>(atof(row[15]));
			entry.has_zone_in         = atoi(row[16]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<DynamicZones> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<DynamicZones> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DynamicZones entry{};

			entry.id                  = atoi(row[0]);
			entry.instance_id         = atoi(row[1]);
			entry.type                = atoi(row[2]);
			entry.compass_zone_id     = atoi(row[3]);
			entry.compass_x           = static_cast<float>(atof(row[4]));
			entry.compass_y           = static_cast<float>(atof(row[5]));
			entry.compass_z           = static_cast<float>(atof(row[6]));
			entry.safe_return_zone_id = atoi(row[7]);
			entry.safe_return_x       = static_cast<float>(atof(row[8]));
			entry.safe_return_y       = static_cast<float>(atof(row[9]));
			entry.safe_return_z       = static_cast<float>(atof(row[10]));
			entry.safe_return_heading = static_cast<float>(atof(row[11]));
			entry.zone_in_x           = static_cast<float>(atof(row[12]));
			entry.zone_in_y           = static_cast<float>(atof(row[13]));
			entry.zone_in_z           = static_cast<float>(atof(row[14]));
			entry.zone_in_heading     = static_cast<float>(atof(row[15]));
			entry.has_zone_in         = atoi(row[16]);

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

#endif //EQEMU_BASE_DYNAMIC_ZONES_REPOSITORY_H
