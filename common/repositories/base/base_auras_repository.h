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
#include "../../string_util.h"

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

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("auras");
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

	static Auras NewEntity()
	{
		Auras entry{};

		entry.type       = 0;
		entry.npc_type   = 0;
		entry.name       = "";
		entry.spell_id   = 0;
		entry.distance   = 60;
		entry.aura_type  = 1;
		entry.spawn_type = 0;
		entry.movement   = 0;
		entry.duration   = 5400;
		entry.icon       = -1;
		entry.cast_time  = 0;

		return entry;
	}

	static Auras GetAurasEntry(
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
			Auras entry{};

			entry.type       = atoi(row[0]);
			entry.npc_type   = atoi(row[1]);
			entry.name       = row[2] ? row[2] : "";
			entry.spell_id   = atoi(row[3]);
			entry.distance   = atoi(row[4]);
			entry.aura_type  = atoi(row[5]);
			entry.spawn_type = atoi(row[6]);
			entry.movement   = atoi(row[7]);
			entry.duration   = atoi(row[8]);
			entry.icon       = atoi(row[9]);
			entry.cast_time  = atoi(row[10]);

			return entry;
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
		Auras auras_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(auras_entry.type));
		update_values.push_back(columns[1] + " = " + std::to_string(auras_entry.npc_type));
		update_values.push_back(columns[2] + " = '" + EscapeString(auras_entry.name) + "'");
		update_values.push_back(columns[3] + " = " + std::to_string(auras_entry.spell_id));
		update_values.push_back(columns[4] + " = " + std::to_string(auras_entry.distance));
		update_values.push_back(columns[5] + " = " + std::to_string(auras_entry.aura_type));
		update_values.push_back(columns[6] + " = " + std::to_string(auras_entry.spawn_type));
		update_values.push_back(columns[7] + " = " + std::to_string(auras_entry.movement));
		update_values.push_back(columns[8] + " = " + std::to_string(auras_entry.duration));
		update_values.push_back(columns[9] + " = " + std::to_string(auras_entry.icon));
		update_values.push_back(columns[10] + " = " + std::to_string(auras_entry.cast_time));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				auras_entry.type
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Auras InsertOne(
		Database& db,
		Auras auras_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(auras_entry.type));
		insert_values.push_back(std::to_string(auras_entry.npc_type));
		insert_values.push_back("'" + EscapeString(auras_entry.name) + "'");
		insert_values.push_back(std::to_string(auras_entry.spell_id));
		insert_values.push_back(std::to_string(auras_entry.distance));
		insert_values.push_back(std::to_string(auras_entry.aura_type));
		insert_values.push_back(std::to_string(auras_entry.spawn_type));
		insert_values.push_back(std::to_string(auras_entry.movement));
		insert_values.push_back(std::to_string(auras_entry.duration));
		insert_values.push_back(std::to_string(auras_entry.icon));
		insert_values.push_back(std::to_string(auras_entry.cast_time));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			auras_entry.type = results.LastInsertedID();
			return auras_entry;
		}

		auras_entry = NewEntity();

		return auras_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Auras> auras_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &auras_entry: auras_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(auras_entry.type));
			insert_values.push_back(std::to_string(auras_entry.npc_type));
			insert_values.push_back("'" + EscapeString(auras_entry.name) + "'");
			insert_values.push_back(std::to_string(auras_entry.spell_id));
			insert_values.push_back(std::to_string(auras_entry.distance));
			insert_values.push_back(std::to_string(auras_entry.aura_type));
			insert_values.push_back(std::to_string(auras_entry.spawn_type));
			insert_values.push_back(std::to_string(auras_entry.movement));
			insert_values.push_back(std::to_string(auras_entry.duration));
			insert_values.push_back(std::to_string(auras_entry.icon));
			insert_values.push_back(std::to_string(auras_entry.cast_time));

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
			Auras entry{};

			entry.type       = atoi(row[0]);
			entry.npc_type   = atoi(row[1]);
			entry.name       = row[2] ? row[2] : "";
			entry.spell_id   = atoi(row[3]);
			entry.distance   = atoi(row[4]);
			entry.aura_type  = atoi(row[5]);
			entry.spawn_type = atoi(row[6]);
			entry.movement   = atoi(row[7]);
			entry.duration   = atoi(row[8]);
			entry.icon       = atoi(row[9]);
			entry.cast_time  = atoi(row[10]);

			all_entries.push_back(entry);
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
			Auras entry{};

			entry.type       = atoi(row[0]);
			entry.npc_type   = atoi(row[1]);
			entry.name       = row[2] ? row[2] : "";
			entry.spell_id   = atoi(row[3]);
			entry.distance   = atoi(row[4]);
			entry.aura_type  = atoi(row[5]);
			entry.spawn_type = atoi(row[6]);
			entry.movement   = atoi(row[7]);
			entry.duration   = atoi(row[8]);
			entry.icon       = atoi(row[9]);
			entry.cast_time  = atoi(row[10]);

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

#endif //EQEMU_BASE_AURAS_REPOSITORY_H
