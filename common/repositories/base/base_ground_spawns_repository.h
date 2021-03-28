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

#ifndef EQEMU_BASE_GROUND_SPAWNS_REPOSITORY_H
#define EQEMU_BASE_GROUND_SPAWNS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseGroundSpawnsRepository {
public:
	struct GroundSpawns {
		int         id;
		int         zoneid;
		int         version;
		float       max_x;
		float       max_y;
		float       max_z;
		float       min_x;
		float       min_y;
		float       heading;
		std::string name;
		int         item;
		int         max_allowed;
		std::string comment;
		int         respawn_timer;
		int         min_expansion;
		int         max_expansion;
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
			"zoneid",
			"version",
			"max_x",
			"max_y",
			"max_z",
			"min_x",
			"min_y",
			"heading",
			"name",
			"item",
			"max_allowed",
			"comment",
			"respawn_timer",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("ground_spawns");
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

	static GroundSpawns NewEntity()
	{
		GroundSpawns entry{};

		entry.id                     = 0;
		entry.zoneid                 = 0;
		entry.version                = 0;
		entry.max_x                  = 2000;
		entry.max_y                  = 2000;
		entry.max_z                  = 10000;
		entry.min_x                  = -2000;
		entry.min_y                  = -2000;
		entry.heading                = 0;
		entry.name                   = "";
		entry.item                   = 0;
		entry.max_allowed            = 1;
		entry.comment                = "";
		entry.respawn_timer          = 300;
		entry.min_expansion          = 0;
		entry.max_expansion          = 0;
		entry.content_flags          = "";
		entry.content_flags_disabled = "";

		return entry;
	}

	static GroundSpawns GetGroundSpawnsEntry(
		const std::vector<GroundSpawns> &ground_spawnss,
		int ground_spawns_id
	)
	{
		for (auto &ground_spawns : ground_spawnss) {
			if (ground_spawns.id == ground_spawns_id) {
				return ground_spawns;
			}
		}

		return NewEntity();
	}

	static GroundSpawns FindOne(
		Database& db,
		int ground_spawns_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				ground_spawns_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			GroundSpawns entry{};

			entry.id                     = atoi(row[0]);
			entry.zoneid                 = atoi(row[1]);
			entry.version                = atoi(row[2]);
			entry.max_x                  = static_cast<float>(atof(row[3]));
			entry.max_y                  = static_cast<float>(atof(row[4]));
			entry.max_z                  = static_cast<float>(atof(row[5]));
			entry.min_x                  = static_cast<float>(atof(row[6]));
			entry.min_y                  = static_cast<float>(atof(row[7]));
			entry.heading                = static_cast<float>(atof(row[8]));
			entry.name                   = row[9] ? row[9] : "";
			entry.item                   = atoi(row[10]);
			entry.max_allowed            = atoi(row[11]);
			entry.comment                = row[12] ? row[12] : "";
			entry.respawn_timer          = atoi(row[13]);
			entry.min_expansion          = atoi(row[14]);
			entry.max_expansion          = atoi(row[15]);
			entry.content_flags          = row[16] ? row[16] : "";
			entry.content_flags_disabled = row[17] ? row[17] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int ground_spawns_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				ground_spawns_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		GroundSpawns ground_spawns_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(ground_spawns_entry.zoneid));
		update_values.push_back(columns[2] + " = " + std::to_string(ground_spawns_entry.version));
		update_values.push_back(columns[3] + " = " + std::to_string(ground_spawns_entry.max_x));
		update_values.push_back(columns[4] + " = " + std::to_string(ground_spawns_entry.max_y));
		update_values.push_back(columns[5] + " = " + std::to_string(ground_spawns_entry.max_z));
		update_values.push_back(columns[6] + " = " + std::to_string(ground_spawns_entry.min_x));
		update_values.push_back(columns[7] + " = " + std::to_string(ground_spawns_entry.min_y));
		update_values.push_back(columns[8] + " = " + std::to_string(ground_spawns_entry.heading));
		update_values.push_back(columns[9] + " = '" + EscapeString(ground_spawns_entry.name) + "'");
		update_values.push_back(columns[10] + " = " + std::to_string(ground_spawns_entry.item));
		update_values.push_back(columns[11] + " = " + std::to_string(ground_spawns_entry.max_allowed));
		update_values.push_back(columns[12] + " = '" + EscapeString(ground_spawns_entry.comment) + "'");
		update_values.push_back(columns[13] + " = " + std::to_string(ground_spawns_entry.respawn_timer));
		update_values.push_back(columns[14] + " = " + std::to_string(ground_spawns_entry.min_expansion));
		update_values.push_back(columns[15] + " = " + std::to_string(ground_spawns_entry.max_expansion));
		update_values.push_back(columns[16] + " = '" + EscapeString(ground_spawns_entry.content_flags) + "'");
		update_values.push_back(columns[17] + " = '" + EscapeString(ground_spawns_entry.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				ground_spawns_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static GroundSpawns InsertOne(
		Database& db,
		GroundSpawns ground_spawns_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(ground_spawns_entry.id));
		insert_values.push_back(std::to_string(ground_spawns_entry.zoneid));
		insert_values.push_back(std::to_string(ground_spawns_entry.version));
		insert_values.push_back(std::to_string(ground_spawns_entry.max_x));
		insert_values.push_back(std::to_string(ground_spawns_entry.max_y));
		insert_values.push_back(std::to_string(ground_spawns_entry.max_z));
		insert_values.push_back(std::to_string(ground_spawns_entry.min_x));
		insert_values.push_back(std::to_string(ground_spawns_entry.min_y));
		insert_values.push_back(std::to_string(ground_spawns_entry.heading));
		insert_values.push_back("'" + EscapeString(ground_spawns_entry.name) + "'");
		insert_values.push_back(std::to_string(ground_spawns_entry.item));
		insert_values.push_back(std::to_string(ground_spawns_entry.max_allowed));
		insert_values.push_back("'" + EscapeString(ground_spawns_entry.comment) + "'");
		insert_values.push_back(std::to_string(ground_spawns_entry.respawn_timer));
		insert_values.push_back(std::to_string(ground_spawns_entry.min_expansion));
		insert_values.push_back(std::to_string(ground_spawns_entry.max_expansion));
		insert_values.push_back("'" + EscapeString(ground_spawns_entry.content_flags) + "'");
		insert_values.push_back("'" + EscapeString(ground_spawns_entry.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			ground_spawns_entry.id = results.LastInsertedID();
			return ground_spawns_entry;
		}

		ground_spawns_entry = NewEntity();

		return ground_spawns_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<GroundSpawns> ground_spawns_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &ground_spawns_entry: ground_spawns_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(ground_spawns_entry.id));
			insert_values.push_back(std::to_string(ground_spawns_entry.zoneid));
			insert_values.push_back(std::to_string(ground_spawns_entry.version));
			insert_values.push_back(std::to_string(ground_spawns_entry.max_x));
			insert_values.push_back(std::to_string(ground_spawns_entry.max_y));
			insert_values.push_back(std::to_string(ground_spawns_entry.max_z));
			insert_values.push_back(std::to_string(ground_spawns_entry.min_x));
			insert_values.push_back(std::to_string(ground_spawns_entry.min_y));
			insert_values.push_back(std::to_string(ground_spawns_entry.heading));
			insert_values.push_back("'" + EscapeString(ground_spawns_entry.name) + "'");
			insert_values.push_back(std::to_string(ground_spawns_entry.item));
			insert_values.push_back(std::to_string(ground_spawns_entry.max_allowed));
			insert_values.push_back("'" + EscapeString(ground_spawns_entry.comment) + "'");
			insert_values.push_back(std::to_string(ground_spawns_entry.respawn_timer));
			insert_values.push_back(std::to_string(ground_spawns_entry.min_expansion));
			insert_values.push_back(std::to_string(ground_spawns_entry.max_expansion));
			insert_values.push_back("'" + EscapeString(ground_spawns_entry.content_flags) + "'");
			insert_values.push_back("'" + EscapeString(ground_spawns_entry.content_flags_disabled) + "'");

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

	static std::vector<GroundSpawns> All(Database& db)
	{
		std::vector<GroundSpawns> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GroundSpawns entry{};

			entry.id                     = atoi(row[0]);
			entry.zoneid                 = atoi(row[1]);
			entry.version                = atoi(row[2]);
			entry.max_x                  = static_cast<float>(atof(row[3]));
			entry.max_y                  = static_cast<float>(atof(row[4]));
			entry.max_z                  = static_cast<float>(atof(row[5]));
			entry.min_x                  = static_cast<float>(atof(row[6]));
			entry.min_y                  = static_cast<float>(atof(row[7]));
			entry.heading                = static_cast<float>(atof(row[8]));
			entry.name                   = row[9] ? row[9] : "";
			entry.item                   = atoi(row[10]);
			entry.max_allowed            = atoi(row[11]);
			entry.comment                = row[12] ? row[12] : "";
			entry.respawn_timer          = atoi(row[13]);
			entry.min_expansion          = atoi(row[14]);
			entry.max_expansion          = atoi(row[15]);
			entry.content_flags          = row[16] ? row[16] : "";
			entry.content_flags_disabled = row[17] ? row[17] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<GroundSpawns> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<GroundSpawns> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GroundSpawns entry{};

			entry.id                     = atoi(row[0]);
			entry.zoneid                 = atoi(row[1]);
			entry.version                = atoi(row[2]);
			entry.max_x                  = static_cast<float>(atof(row[3]));
			entry.max_y                  = static_cast<float>(atof(row[4]));
			entry.max_z                  = static_cast<float>(atof(row[5]));
			entry.min_x                  = static_cast<float>(atof(row[6]));
			entry.min_y                  = static_cast<float>(atof(row[7]));
			entry.heading                = static_cast<float>(atof(row[8]));
			entry.name                   = row[9] ? row[9] : "";
			entry.item                   = atoi(row[10]);
			entry.max_allowed            = atoi(row[11]);
			entry.comment                = row[12] ? row[12] : "";
			entry.respawn_timer          = atoi(row[13]);
			entry.min_expansion          = atoi(row[14]);
			entry.max_expansion          = atoi(row[15]);
			entry.content_flags          = row[16] ? row[16] : "";
			entry.content_flags_disabled = row[17] ? row[17] : "";

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

#endif //EQEMU_BASE_GROUND_SPAWNS_REPOSITORY_H
