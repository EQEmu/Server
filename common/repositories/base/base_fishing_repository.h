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
#include "../../strings.h"
#include <ctime>

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

	static std::vector<std::string> SelectColumns()
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
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("fishing");
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

	static Fishing NewEntity()
	{
		Fishing e{};

		e.id                     = 0;
		e.zoneid                 = 0;
		e.Itemid                 = 0;
		e.skill_level            = 0;
		e.chance                 = 0;
		e.npc_id                 = 0;
		e.npc_chance             = 0;
		e.min_expansion          = -1;
		e.max_expansion          = -1;
		e.content_flags          = "";
		e.content_flags_disabled = "";

		return e;
	}

	static Fishing GetFishinge(
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
			Fishing e{};

			e.id                     = atoi(row[0]);
			e.zoneid                 = atoi(row[1]);
			e.Itemid                 = atoi(row[2]);
			e.skill_level            = atoi(row[3]);
			e.chance                 = atoi(row[4]);
			e.npc_id                 = atoi(row[5]);
			e.npc_chance             = atoi(row[6]);
			e.min_expansion          = atoi(row[7]);
			e.max_expansion          = atoi(row[8]);
			e.content_flags          = row[9] ? row[9] : "";
			e.content_flags_disabled = row[10] ? row[10] : "";

			return e;
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
		Fishing fishing_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(fishing_e.zoneid));
		update_values.push_back(columns[2] + " = " + std::to_string(fishing_e.Itemid));
		update_values.push_back(columns[3] + " = " + std::to_string(fishing_e.skill_level));
		update_values.push_back(columns[4] + " = " + std::to_string(fishing_e.chance));
		update_values.push_back(columns[5] + " = " + std::to_string(fishing_e.npc_id));
		update_values.push_back(columns[6] + " = " + std::to_string(fishing_e.npc_chance));
		update_values.push_back(columns[7] + " = " + std::to_string(fishing_e.min_expansion));
		update_values.push_back(columns[8] + " = " + std::to_string(fishing_e.max_expansion));
		update_values.push_back(columns[9] + " = '" + Strings::Escape(fishing_e.content_flags) + "'");
		update_values.push_back(columns[10] + " = '" + Strings::Escape(fishing_e.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				fishing_e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Fishing InsertOne(
		Database& db,
		Fishing fishing_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(fishing_e.id));
		insert_values.push_back(std::to_string(fishing_e.zoneid));
		insert_values.push_back(std::to_string(fishing_e.Itemid));
		insert_values.push_back(std::to_string(fishing_e.skill_level));
		insert_values.push_back(std::to_string(fishing_e.chance));
		insert_values.push_back(std::to_string(fishing_e.npc_id));
		insert_values.push_back(std::to_string(fishing_e.npc_chance));
		insert_values.push_back(std::to_string(fishing_e.min_expansion));
		insert_values.push_back(std::to_string(fishing_e.max_expansion));
		insert_values.push_back("'" + Strings::Escape(fishing_e.content_flags) + "'");
		insert_values.push_back("'" + Strings::Escape(fishing_e.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			fishing_e.id = results.LastInsertedID();
			return fishing_e;
		}

		fishing_e = NewEntity();

		return fishing_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<Fishing> fishing_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &fishing_e: fishing_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(fishing_e.id));
			insert_values.push_back(std::to_string(fishing_e.zoneid));
			insert_values.push_back(std::to_string(fishing_e.Itemid));
			insert_values.push_back(std::to_string(fishing_e.skill_level));
			insert_values.push_back(std::to_string(fishing_e.chance));
			insert_values.push_back(std::to_string(fishing_e.npc_id));
			insert_values.push_back(std::to_string(fishing_e.npc_chance));
			insert_values.push_back(std::to_string(fishing_e.min_expansion));
			insert_values.push_back(std::to_string(fishing_e.max_expansion));
			insert_values.push_back("'" + Strings::Escape(fishing_e.content_flags) + "'");
			insert_values.push_back("'" + Strings::Escape(fishing_e.content_flags_disabled) + "'");

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
			Fishing e{};

			e.id                     = atoi(row[0]);
			e.zoneid                 = atoi(row[1]);
			e.Itemid                 = atoi(row[2]);
			e.skill_level            = atoi(row[3]);
			e.chance                 = atoi(row[4]);
			e.npc_id                 = atoi(row[5]);
			e.npc_chance             = atoi(row[6]);
			e.min_expansion          = atoi(row[7]);
			e.max_expansion          = atoi(row[8]);
			e.content_flags          = row[9] ? row[9] : "";
			e.content_flags_disabled = row[10] ? row[10] : "";

			all_entries.push_back(e);
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
			Fishing e{};

			e.id                     = atoi(row[0]);
			e.zoneid                 = atoi(row[1]);
			e.Itemid                 = atoi(row[2]);
			e.skill_level            = atoi(row[3]);
			e.chance                 = atoi(row[4]);
			e.npc_id                 = atoi(row[5]);
			e.npc_chance             = atoi(row[6]);
			e.min_expansion          = atoi(row[7]);
			e.max_expansion          = atoi(row[8]);
			e.content_flags          = row[9] ? row[9] : "";
			e.content_flags_disabled = row[10] ? row[10] : "";

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

#endif //EQEMU_BASE_FISHING_REPOSITORY_H
