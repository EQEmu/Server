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

#ifndef EQEMU_BASE_FISHING_REPOSITORY_H
#define EQEMU_BASE_FISHING_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseFishingRepository {
public:
	struct Fishing {
		int         id;
		int         zoneid;
		int         Itemid;
		int         skill_level;
		int         chance;
		int         npc_id;
		int         npc_chance;
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
			"Itemid",
			"skill_level",
			"chance",
			"npc_id",
			"npc_chance",
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
		return std::string("fishing");
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

	static Fishing NewEntity()
	{
		Fishing entry{};

		entry.id                     = 0;
		entry.zoneid                 = 0;
		entry.Itemid                 = 0;
		entry.skill_level            = 0;
		entry.chance                 = 0;
		entry.npc_id                 = 0;
		entry.npc_chance             = 0;
		entry.min_expansion          = 0;
		entry.max_expansion          = 0;
		entry.content_flags          = "";
		entry.content_flags_disabled = "";

		return entry;
	}

	static Fishing GetFishingEntry(
		const std::vector<Fishing> &fishings,
		int fishing_id
	)
	{
		for (auto &fishing : fishings) {
			if (fishing.id == fishing_id) {
				return fishing;
			}
		}

		return NewEntity();
	}

	static Fishing FindOne(
		Database& db,
		int fishing_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				fishing_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Fishing entry{};

			entry.id                     = atoi(row[0]);
			entry.zoneid                 = atoi(row[1]);
			entry.Itemid                 = atoi(row[2]);
			entry.skill_level            = atoi(row[3]);
			entry.chance                 = atoi(row[4]);
			entry.npc_id                 = atoi(row[5]);
			entry.npc_chance             = atoi(row[6]);
			entry.min_expansion          = atoi(row[7]);
			entry.max_expansion          = atoi(row[8]);
			entry.content_flags          = row[9] ? row[9] : "";
			entry.content_flags_disabled = row[10] ? row[10] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int fishing_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				fishing_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Fishing fishing_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(fishing_entry.zoneid));
		update_values.push_back(columns[2] + " = " + std::to_string(fishing_entry.Itemid));
		update_values.push_back(columns[3] + " = " + std::to_string(fishing_entry.skill_level));
		update_values.push_back(columns[4] + " = " + std::to_string(fishing_entry.chance));
		update_values.push_back(columns[5] + " = " + std::to_string(fishing_entry.npc_id));
		update_values.push_back(columns[6] + " = " + std::to_string(fishing_entry.npc_chance));
		update_values.push_back(columns[7] + " = " + std::to_string(fishing_entry.min_expansion));
		update_values.push_back(columns[8] + " = " + std::to_string(fishing_entry.max_expansion));
		update_values.push_back(columns[9] + " = '" + EscapeString(fishing_entry.content_flags) + "'");
		update_values.push_back(columns[10] + " = '" + EscapeString(fishing_entry.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				fishing_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Fishing InsertOne(
		Database& db,
		Fishing fishing_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(fishing_entry.id));
		insert_values.push_back(std::to_string(fishing_entry.zoneid));
		insert_values.push_back(std::to_string(fishing_entry.Itemid));
		insert_values.push_back(std::to_string(fishing_entry.skill_level));
		insert_values.push_back(std::to_string(fishing_entry.chance));
		insert_values.push_back(std::to_string(fishing_entry.npc_id));
		insert_values.push_back(std::to_string(fishing_entry.npc_chance));
		insert_values.push_back(std::to_string(fishing_entry.min_expansion));
		insert_values.push_back(std::to_string(fishing_entry.max_expansion));
		insert_values.push_back("'" + EscapeString(fishing_entry.content_flags) + "'");
		insert_values.push_back("'" + EscapeString(fishing_entry.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			fishing_entry.id = results.LastInsertedID();
			return fishing_entry;
		}

		fishing_entry = NewEntity();

		return fishing_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Fishing> fishing_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &fishing_entry: fishing_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(fishing_entry.id));
			insert_values.push_back(std::to_string(fishing_entry.zoneid));
			insert_values.push_back(std::to_string(fishing_entry.Itemid));
			insert_values.push_back(std::to_string(fishing_entry.skill_level));
			insert_values.push_back(std::to_string(fishing_entry.chance));
			insert_values.push_back(std::to_string(fishing_entry.npc_id));
			insert_values.push_back(std::to_string(fishing_entry.npc_chance));
			insert_values.push_back(std::to_string(fishing_entry.min_expansion));
			insert_values.push_back(std::to_string(fishing_entry.max_expansion));
			insert_values.push_back("'" + EscapeString(fishing_entry.content_flags) + "'");
			insert_values.push_back("'" + EscapeString(fishing_entry.content_flags_disabled) + "'");

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

	static std::vector<Fishing> All(Database& db)
	{
		std::vector<Fishing> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Fishing entry{};

			entry.id                     = atoi(row[0]);
			entry.zoneid                 = atoi(row[1]);
			entry.Itemid                 = atoi(row[2]);
			entry.skill_level            = atoi(row[3]);
			entry.chance                 = atoi(row[4]);
			entry.npc_id                 = atoi(row[5]);
			entry.npc_chance             = atoi(row[6]);
			entry.min_expansion          = atoi(row[7]);
			entry.max_expansion          = atoi(row[8]);
			entry.content_flags          = row[9] ? row[9] : "";
			entry.content_flags_disabled = row[10] ? row[10] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Fishing> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Fishing> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Fishing entry{};

			entry.id                     = atoi(row[0]);
			entry.zoneid                 = atoi(row[1]);
			entry.Itemid                 = atoi(row[2]);
			entry.skill_level            = atoi(row[3]);
			entry.chance                 = atoi(row[4]);
			entry.npc_id                 = atoi(row[5]);
			entry.npc_chance             = atoi(row[6]);
			entry.min_expansion          = atoi(row[7]);
			entry.max_expansion          = atoi(row[8]);
			entry.content_flags          = row[9] ? row[9] : "";
			entry.content_flags_disabled = row[10] ? row[10] : "";

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

#endif //EQEMU_BASE_FISHING_REPOSITORY_H
