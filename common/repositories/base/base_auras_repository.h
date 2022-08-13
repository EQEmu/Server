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

#ifndef EQEMU_BASE_AURAS_REPOSITORY_H
#define EQEMU_BASE_AURAS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseAurasRepository {
public:
	struct Auras {
		int         type;
		int         npc_type;
		std::string name;
		int         spell_id;
		int         distance;
		int         aura_type;
		int         spawn_type;
		int         movement;
		int         duration;
		int         icon;
		int         cast_time;
	};

	static std::string PrimaryKey()
	{
		return std::string("type");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"type",
			"npc_type",
			"name",
			"spell_id",
			"distance",
			"aura_type",
			"spawn_type",
			"movement",
			"duration",
			"icon",
			"cast_time",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"type",
			"npc_type",
			"name",
			"spell_id",
			"distance",
			"aura_type",
			"spawn_type",
			"movement",
			"duration",
			"icon",
			"cast_time",
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
		return std::string("auras");
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

	static Auras NewEntity()
	{
		Auras e{};

		e.type       = 0;
		e.npc_type   = 0;
		e.name       = "";
		e.spell_id   = 0;
		e.distance   = 60;
		e.aura_type  = 1;
		e.spawn_type = 0;
		e.movement   = 0;
		e.duration   = 5400;
		e.icon       = -1;
		e.cast_time  = 0;

		return e;
	}

	static Auras GetAurase(
		const std::vector<Auras> &aurass,
		int auras_id
	)
	{
		for (auto &auras : aurass) {
			if (auras.type == auras_id) {
				return auras;
			}
		}

		return NewEntity();
	}

	static Auras FindOne(
		Database& db,
		int auras_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				auras_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Auras e{};

			e.type       = atoi(row[0]);
			e.npc_type   = atoi(row[1]);
			e.name       = row[2] ? row[2] : "";
			e.spell_id   = atoi(row[3]);
			e.distance   = atoi(row[4]);
			e.aura_type  = atoi(row[5]);
			e.spawn_type = atoi(row[6]);
			e.movement   = atoi(row[7]);
			e.duration   = atoi(row[8]);
			e.icon       = atoi(row[9]);
			e.cast_time  = atoi(row[10]);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int auras_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				auras_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Auras e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(e.type));
		update_values.push_back(columns[1] + " = " + std::to_string(e.npc_type));
		update_values.push_back(columns[2] + " = '" + Strings::Escape(e.name) + "'");
		update_values.push_back(columns[3] + " = " + std::to_string(e.spell_id));
		update_values.push_back(columns[4] + " = " + std::to_string(e.distance));
		update_values.push_back(columns[5] + " = " + std::to_string(e.aura_type));
		update_values.push_back(columns[6] + " = " + std::to_string(e.spawn_type));
		update_values.push_back(columns[7] + " = " + std::to_string(e.movement));
		update_values.push_back(columns[8] + " = " + std::to_string(e.duration));
		update_values.push_back(columns[9] + " = " + std::to_string(e.icon));
		update_values.push_back(columns[10] + " = " + std::to_string(e.cast_time));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				e.type
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Auras InsertOne(
		Database& db,
		Auras e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(e.type));
		insert_values.push_back(std::to_string(e.npc_type));
		insert_values.push_back("'" + Strings::Escape(e.name) + "'");
		insert_values.push_back(std::to_string(e.spell_id));
		insert_values.push_back(std::to_string(e.distance));
		insert_values.push_back(std::to_string(e.aura_type));
		insert_values.push_back(std::to_string(e.spawn_type));
		insert_values.push_back(std::to_string(e.movement));
		insert_values.push_back(std::to_string(e.duration));
		insert_values.push_back(std::to_string(e.icon));
		insert_values.push_back(std::to_string(e.cast_time));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			e.type = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		std::vector<Auras> entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(e.type));
			insert_values.push_back(std::to_string(e.npc_type));
			insert_values.push_back("'" + Strings::Escape(e.name) + "'");
			insert_values.push_back(std::to_string(e.spell_id));
			insert_values.push_back(std::to_string(e.distance));
			insert_values.push_back(std::to_string(e.aura_type));
			insert_values.push_back(std::to_string(e.spawn_type));
			insert_values.push_back(std::to_string(e.movement));
			insert_values.push_back(std::to_string(e.duration));
			insert_values.push_back(std::to_string(e.icon));
			insert_values.push_back(std::to_string(e.cast_time));

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

	static std::vector<Auras> All(Database& db)
	{
		std::vector<Auras> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Auras e{};

			e.type       = atoi(row[0]);
			e.npc_type   = atoi(row[1]);
			e.name       = row[2] ? row[2] : "";
			e.spell_id   = atoi(row[3]);
			e.distance   = atoi(row[4]);
			e.aura_type  = atoi(row[5]);
			e.spawn_type = atoi(row[6]);
			e.movement   = atoi(row[7]);
			e.duration   = atoi(row[8]);
			e.icon       = atoi(row[9]);
			e.cast_time  = atoi(row[10]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Auras> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Auras> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Auras e{};

			e.type       = atoi(row[0]);
			e.npc_type   = atoi(row[1]);
			e.name       = row[2] ? row[2] : "";
			e.spell_id   = atoi(row[3]);
			e.distance   = atoi(row[4]);
			e.aura_type  = atoi(row[5]);
			e.spawn_type = atoi(row[6]);
			e.movement   = atoi(row[7]);
			e.duration   = atoi(row[8]);
			e.icon       = atoi(row[9]);
			e.cast_time  = atoi(row[10]);

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

#endif //EQEMU_BASE_AURAS_REPOSITORY_H
