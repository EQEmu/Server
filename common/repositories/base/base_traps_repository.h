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

#ifndef EQEMU_BASE_TRAPS_REPOSITORY_H
#define EQEMU_BASE_TRAPS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseTrapsRepository {
public:
	struct Traps {
		int         id;
		std::string zone;
		int         version;
		int         x;
		int         y;
		int         z;
		int         chance;
		float       maxzdiff;
		float       radius;
		int         effect;
		int         effectvalue;
		int         effectvalue2;
		std::string message;
		int         skill;
		int         level;
		int         respawn_time;
		int         respawn_var;
		int         triggered_number;
		int         group;
		int         despawn_when_triggered;
		int         undetectable;
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
			"zone",
			"version",
			"x",
			"y",
			"z",
			"chance",
			"maxzdiff",
			"radius",
			"effect",
			"effectvalue",
			"effectvalue2",
			"message",
			"skill",
			"level",
			"respawn_time",
			"respawn_var",
			"triggered_number",
			"group",
			"despawn_when_triggered",
			"undetectable",
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
		return std::string("traps");
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

	static Traps NewEntity()
	{
		Traps entry{};

		entry.id                     = 0;
		entry.zone                   = "";
		entry.version                = 0;
		entry.x                      = 0;
		entry.y                      = 0;
		entry.z                      = 0;
		entry.chance                 = 0;
		entry.maxzdiff               = 0;
		entry.radius                 = 0;
		entry.effect                 = 0;
		entry.effectvalue            = 0;
		entry.effectvalue2           = 0;
		entry.message                = "";
		entry.skill                  = 0;
		entry.level                  = 1;
		entry.respawn_time           = 60;
		entry.respawn_var            = 0;
		entry.triggered_number       = 0;
		entry.group                  = 0;
		entry.despawn_when_triggered = 0;
		entry.undetectable           = 0;
		entry.min_expansion          = 0;
		entry.max_expansion          = 0;
		entry.content_flags          = "";
		entry.content_flags_disabled = "";

		return entry;
	}

	static Traps GetTrapsEntry(
		const std::vector<Traps> &trapss,
		int traps_id
	)
	{
		for (auto &traps : trapss) {
			if (traps.id == traps_id) {
				return traps;
			}
		}

		return NewEntity();
	}

	static Traps FindOne(
		Database& db,
		int traps_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				traps_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Traps entry{};

			entry.id                     = atoi(row[0]);
			entry.zone                   = row[1] ? row[1] : "";
			entry.version                = atoi(row[2]);
			entry.x                      = atoi(row[3]);
			entry.y                      = atoi(row[4]);
			entry.z                      = atoi(row[5]);
			entry.chance                 = atoi(row[6]);
			entry.maxzdiff               = static_cast<float>(atof(row[7]));
			entry.radius                 = static_cast<float>(atof(row[8]));
			entry.effect                 = atoi(row[9]);
			entry.effectvalue            = atoi(row[10]);
			entry.effectvalue2           = atoi(row[11]);
			entry.message                = row[12] ? row[12] : "";
			entry.skill                  = atoi(row[13]);
			entry.level                  = atoi(row[14]);
			entry.respawn_time           = atoi(row[15]);
			entry.respawn_var            = atoi(row[16]);
			entry.triggered_number       = atoi(row[17]);
			entry.group                  = atoi(row[18]);
			entry.despawn_when_triggered = atoi(row[19]);
			entry.undetectable           = atoi(row[20]);
			entry.min_expansion          = atoi(row[21]);
			entry.max_expansion          = atoi(row[22]);
			entry.content_flags          = row[23] ? row[23] : "";
			entry.content_flags_disabled = row[24] ? row[24] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int traps_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				traps_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Traps traps_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(traps_entry.zone) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(traps_entry.version));
		update_values.push_back(columns[3] + " = " + std::to_string(traps_entry.x));
		update_values.push_back(columns[4] + " = " + std::to_string(traps_entry.y));
		update_values.push_back(columns[5] + " = " + std::to_string(traps_entry.z));
		update_values.push_back(columns[6] + " = " + std::to_string(traps_entry.chance));
		update_values.push_back(columns[7] + " = " + std::to_string(traps_entry.maxzdiff));
		update_values.push_back(columns[8] + " = " + std::to_string(traps_entry.radius));
		update_values.push_back(columns[9] + " = " + std::to_string(traps_entry.effect));
		update_values.push_back(columns[10] + " = " + std::to_string(traps_entry.effectvalue));
		update_values.push_back(columns[11] + " = " + std::to_string(traps_entry.effectvalue2));
		update_values.push_back(columns[12] + " = '" + EscapeString(traps_entry.message) + "'");
		update_values.push_back(columns[13] + " = " + std::to_string(traps_entry.skill));
		update_values.push_back(columns[14] + " = " + std::to_string(traps_entry.level));
		update_values.push_back(columns[15] + " = " + std::to_string(traps_entry.respawn_time));
		update_values.push_back(columns[16] + " = " + std::to_string(traps_entry.respawn_var));
		update_values.push_back(columns[17] + " = " + std::to_string(traps_entry.triggered_number));
		update_values.push_back(columns[18] + " = " + std::to_string(traps_entry.group));
		update_values.push_back(columns[19] + " = " + std::to_string(traps_entry.despawn_when_triggered));
		update_values.push_back(columns[20] + " = " + std::to_string(traps_entry.undetectable));
		update_values.push_back(columns[21] + " = " + std::to_string(traps_entry.min_expansion));
		update_values.push_back(columns[22] + " = " + std::to_string(traps_entry.max_expansion));
		update_values.push_back(columns[23] + " = '" + EscapeString(traps_entry.content_flags) + "'");
		update_values.push_back(columns[24] + " = '" + EscapeString(traps_entry.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				traps_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Traps InsertOne(
		Database& db,
		Traps traps_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(traps_entry.id));
		insert_values.push_back("'" + EscapeString(traps_entry.zone) + "'");
		insert_values.push_back(std::to_string(traps_entry.version));
		insert_values.push_back(std::to_string(traps_entry.x));
		insert_values.push_back(std::to_string(traps_entry.y));
		insert_values.push_back(std::to_string(traps_entry.z));
		insert_values.push_back(std::to_string(traps_entry.chance));
		insert_values.push_back(std::to_string(traps_entry.maxzdiff));
		insert_values.push_back(std::to_string(traps_entry.radius));
		insert_values.push_back(std::to_string(traps_entry.effect));
		insert_values.push_back(std::to_string(traps_entry.effectvalue));
		insert_values.push_back(std::to_string(traps_entry.effectvalue2));
		insert_values.push_back("'" + EscapeString(traps_entry.message) + "'");
		insert_values.push_back(std::to_string(traps_entry.skill));
		insert_values.push_back(std::to_string(traps_entry.level));
		insert_values.push_back(std::to_string(traps_entry.respawn_time));
		insert_values.push_back(std::to_string(traps_entry.respawn_var));
		insert_values.push_back(std::to_string(traps_entry.triggered_number));
		insert_values.push_back(std::to_string(traps_entry.group));
		insert_values.push_back(std::to_string(traps_entry.despawn_when_triggered));
		insert_values.push_back(std::to_string(traps_entry.undetectable));
		insert_values.push_back(std::to_string(traps_entry.min_expansion));
		insert_values.push_back(std::to_string(traps_entry.max_expansion));
		insert_values.push_back("'" + EscapeString(traps_entry.content_flags) + "'");
		insert_values.push_back("'" + EscapeString(traps_entry.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			traps_entry.id = results.LastInsertedID();
			return traps_entry;
		}

		traps_entry = NewEntity();

		return traps_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Traps> traps_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &traps_entry: traps_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(traps_entry.id));
			insert_values.push_back("'" + EscapeString(traps_entry.zone) + "'");
			insert_values.push_back(std::to_string(traps_entry.version));
			insert_values.push_back(std::to_string(traps_entry.x));
			insert_values.push_back(std::to_string(traps_entry.y));
			insert_values.push_back(std::to_string(traps_entry.z));
			insert_values.push_back(std::to_string(traps_entry.chance));
			insert_values.push_back(std::to_string(traps_entry.maxzdiff));
			insert_values.push_back(std::to_string(traps_entry.radius));
			insert_values.push_back(std::to_string(traps_entry.effect));
			insert_values.push_back(std::to_string(traps_entry.effectvalue));
			insert_values.push_back(std::to_string(traps_entry.effectvalue2));
			insert_values.push_back("'" + EscapeString(traps_entry.message) + "'");
			insert_values.push_back(std::to_string(traps_entry.skill));
			insert_values.push_back(std::to_string(traps_entry.level));
			insert_values.push_back(std::to_string(traps_entry.respawn_time));
			insert_values.push_back(std::to_string(traps_entry.respawn_var));
			insert_values.push_back(std::to_string(traps_entry.triggered_number));
			insert_values.push_back(std::to_string(traps_entry.group));
			insert_values.push_back(std::to_string(traps_entry.despawn_when_triggered));
			insert_values.push_back(std::to_string(traps_entry.undetectable));
			insert_values.push_back(std::to_string(traps_entry.min_expansion));
			insert_values.push_back(std::to_string(traps_entry.max_expansion));
			insert_values.push_back("'" + EscapeString(traps_entry.content_flags) + "'");
			insert_values.push_back("'" + EscapeString(traps_entry.content_flags_disabled) + "'");

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

	static std::vector<Traps> All(Database& db)
	{
		std::vector<Traps> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Traps entry{};

			entry.id                     = atoi(row[0]);
			entry.zone                   = row[1] ? row[1] : "";
			entry.version                = atoi(row[2]);
			entry.x                      = atoi(row[3]);
			entry.y                      = atoi(row[4]);
			entry.z                      = atoi(row[5]);
			entry.chance                 = atoi(row[6]);
			entry.maxzdiff               = static_cast<float>(atof(row[7]));
			entry.radius                 = static_cast<float>(atof(row[8]));
			entry.effect                 = atoi(row[9]);
			entry.effectvalue            = atoi(row[10]);
			entry.effectvalue2           = atoi(row[11]);
			entry.message                = row[12] ? row[12] : "";
			entry.skill                  = atoi(row[13]);
			entry.level                  = atoi(row[14]);
			entry.respawn_time           = atoi(row[15]);
			entry.respawn_var            = atoi(row[16]);
			entry.triggered_number       = atoi(row[17]);
			entry.group                  = atoi(row[18]);
			entry.despawn_when_triggered = atoi(row[19]);
			entry.undetectable           = atoi(row[20]);
			entry.min_expansion          = atoi(row[21]);
			entry.max_expansion          = atoi(row[22]);
			entry.content_flags          = row[23] ? row[23] : "";
			entry.content_flags_disabled = row[24] ? row[24] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Traps> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Traps> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Traps entry{};

			entry.id                     = atoi(row[0]);
			entry.zone                   = row[1] ? row[1] : "";
			entry.version                = atoi(row[2]);
			entry.x                      = atoi(row[3]);
			entry.y                      = atoi(row[4]);
			entry.z                      = atoi(row[5]);
			entry.chance                 = atoi(row[6]);
			entry.maxzdiff               = static_cast<float>(atof(row[7]));
			entry.radius                 = static_cast<float>(atof(row[8]));
			entry.effect                 = atoi(row[9]);
			entry.effectvalue            = atoi(row[10]);
			entry.effectvalue2           = atoi(row[11]);
			entry.message                = row[12] ? row[12] : "";
			entry.skill                  = atoi(row[13]);
			entry.level                  = atoi(row[14]);
			entry.respawn_time           = atoi(row[15]);
			entry.respawn_var            = atoi(row[16]);
			entry.triggered_number       = atoi(row[17]);
			entry.group                  = atoi(row[18]);
			entry.despawn_when_triggered = atoi(row[19]);
			entry.undetectable           = atoi(row[20]);
			entry.min_expansion          = atoi(row[21]);
			entry.max_expansion          = atoi(row[22]);
			entry.content_flags          = row[23] ? row[23] : "";
			entry.content_flags_disabled = row[24] ? row[24] : "";

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

#endif //EQEMU_BASE_TRAPS_REPOSITORY_H
