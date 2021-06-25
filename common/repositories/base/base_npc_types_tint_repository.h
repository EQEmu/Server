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

#ifndef EQEMU_BASE_NPC_TYPES_TINT_REPOSITORY_H
#define EQEMU_BASE_NPC_TYPES_TINT_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseNpcTypesTintRepository {
public:
	struct NpcTypesTint {
		int         id;
		std::string tint_set_name;
		int         red1h;
		int         grn1h;
		int         blu1h;
		int         red2c;
		int         grn2c;
		int         blu2c;
		int         red3a;
		int         grn3a;
		int         blu3a;
		int         red4b;
		int         grn4b;
		int         blu4b;
		int         red5g;
		int         grn5g;
		int         blu5g;
		int         red6l;
		int         grn6l;
		int         blu6l;
		int         red7f;
		int         grn7f;
		int         blu7f;
		int         red8x;
		int         grn8x;
		int         blu8x;
		int         red9x;
		int         grn9x;
		int         blu9x;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"tint_set_name",
			"red1h",
			"grn1h",
			"blu1h",
			"red2c",
			"grn2c",
			"blu2c",
			"red3a",
			"grn3a",
			"blu3a",
			"red4b",
			"grn4b",
			"blu4b",
			"red5g",
			"grn5g",
			"blu5g",
			"red6l",
			"grn6l",
			"blu6l",
			"red7f",
			"grn7f",
			"blu7f",
			"red8x",
			"grn8x",
			"blu8x",
			"red9x",
			"grn9x",
			"blu9x",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("npc_types_tint");
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

	static NpcTypesTint NewEntity()
	{
		NpcTypesTint entry{};

		entry.id            = 0;
		entry.tint_set_name = "";
		entry.red1h         = 0;
		entry.grn1h         = 0;
		entry.blu1h         = 0;
		entry.red2c         = 0;
		entry.grn2c         = 0;
		entry.blu2c         = 0;
		entry.red3a         = 0;
		entry.grn3a         = 0;
		entry.blu3a         = 0;
		entry.red4b         = 0;
		entry.grn4b         = 0;
		entry.blu4b         = 0;
		entry.red5g         = 0;
		entry.grn5g         = 0;
		entry.blu5g         = 0;
		entry.red6l         = 0;
		entry.grn6l         = 0;
		entry.blu6l         = 0;
		entry.red7f         = 0;
		entry.grn7f         = 0;
		entry.blu7f         = 0;
		entry.red8x         = 0;
		entry.grn8x         = 0;
		entry.blu8x         = 0;
		entry.red9x         = 0;
		entry.grn9x         = 0;
		entry.blu9x         = 0;

		return entry;
	}

	static NpcTypesTint GetNpcTypesTintEntry(
		const std::vector<NpcTypesTint> &npc_types_tints,
		int npc_types_tint_id
	)
	{
		for (auto &npc_types_tint : npc_types_tints) {
			if (npc_types_tint.id == npc_types_tint_id) {
				return npc_types_tint;
			}
		}

		return NewEntity();
	}

	static NpcTypesTint FindOne(
		Database& db,
		int npc_types_tint_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				npc_types_tint_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			NpcTypesTint entry{};

			entry.id            = atoi(row[0]);
			entry.tint_set_name = row[1] ? row[1] : "";
			entry.red1h         = atoi(row[2]);
			entry.grn1h         = atoi(row[3]);
			entry.blu1h         = atoi(row[4]);
			entry.red2c         = atoi(row[5]);
			entry.grn2c         = atoi(row[6]);
			entry.blu2c         = atoi(row[7]);
			entry.red3a         = atoi(row[8]);
			entry.grn3a         = atoi(row[9]);
			entry.blu3a         = atoi(row[10]);
			entry.red4b         = atoi(row[11]);
			entry.grn4b         = atoi(row[12]);
			entry.blu4b         = atoi(row[13]);
			entry.red5g         = atoi(row[14]);
			entry.grn5g         = atoi(row[15]);
			entry.blu5g         = atoi(row[16]);
			entry.red6l         = atoi(row[17]);
			entry.grn6l         = atoi(row[18]);
			entry.blu6l         = atoi(row[19]);
			entry.red7f         = atoi(row[20]);
			entry.grn7f         = atoi(row[21]);
			entry.blu7f         = atoi(row[22]);
			entry.red8x         = atoi(row[23]);
			entry.grn8x         = atoi(row[24]);
			entry.blu8x         = atoi(row[25]);
			entry.red9x         = atoi(row[26]);
			entry.grn9x         = atoi(row[27]);
			entry.blu9x         = atoi(row[28]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int npc_types_tint_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				npc_types_tint_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		NpcTypesTint npc_types_tint_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(npc_types_tint_entry.id));
		update_values.push_back(columns[1] + " = '" + EscapeString(npc_types_tint_entry.tint_set_name) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(npc_types_tint_entry.red1h));
		update_values.push_back(columns[3] + " = " + std::to_string(npc_types_tint_entry.grn1h));
		update_values.push_back(columns[4] + " = " + std::to_string(npc_types_tint_entry.blu1h));
		update_values.push_back(columns[5] + " = " + std::to_string(npc_types_tint_entry.red2c));
		update_values.push_back(columns[6] + " = " + std::to_string(npc_types_tint_entry.grn2c));
		update_values.push_back(columns[7] + " = " + std::to_string(npc_types_tint_entry.blu2c));
		update_values.push_back(columns[8] + " = " + std::to_string(npc_types_tint_entry.red3a));
		update_values.push_back(columns[9] + " = " + std::to_string(npc_types_tint_entry.grn3a));
		update_values.push_back(columns[10] + " = " + std::to_string(npc_types_tint_entry.blu3a));
		update_values.push_back(columns[11] + " = " + std::to_string(npc_types_tint_entry.red4b));
		update_values.push_back(columns[12] + " = " + std::to_string(npc_types_tint_entry.grn4b));
		update_values.push_back(columns[13] + " = " + std::to_string(npc_types_tint_entry.blu4b));
		update_values.push_back(columns[14] + " = " + std::to_string(npc_types_tint_entry.red5g));
		update_values.push_back(columns[15] + " = " + std::to_string(npc_types_tint_entry.grn5g));
		update_values.push_back(columns[16] + " = " + std::to_string(npc_types_tint_entry.blu5g));
		update_values.push_back(columns[17] + " = " + std::to_string(npc_types_tint_entry.red6l));
		update_values.push_back(columns[18] + " = " + std::to_string(npc_types_tint_entry.grn6l));
		update_values.push_back(columns[19] + " = " + std::to_string(npc_types_tint_entry.blu6l));
		update_values.push_back(columns[20] + " = " + std::to_string(npc_types_tint_entry.red7f));
		update_values.push_back(columns[21] + " = " + std::to_string(npc_types_tint_entry.grn7f));
		update_values.push_back(columns[22] + " = " + std::to_string(npc_types_tint_entry.blu7f));
		update_values.push_back(columns[23] + " = " + std::to_string(npc_types_tint_entry.red8x));
		update_values.push_back(columns[24] + " = " + std::to_string(npc_types_tint_entry.grn8x));
		update_values.push_back(columns[25] + " = " + std::to_string(npc_types_tint_entry.blu8x));
		update_values.push_back(columns[26] + " = " + std::to_string(npc_types_tint_entry.red9x));
		update_values.push_back(columns[27] + " = " + std::to_string(npc_types_tint_entry.grn9x));
		update_values.push_back(columns[28] + " = " + std::to_string(npc_types_tint_entry.blu9x));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				npc_types_tint_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static NpcTypesTint InsertOne(
		Database& db,
		NpcTypesTint npc_types_tint_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(npc_types_tint_entry.id));
		insert_values.push_back("'" + EscapeString(npc_types_tint_entry.tint_set_name) + "'");
		insert_values.push_back(std::to_string(npc_types_tint_entry.red1h));
		insert_values.push_back(std::to_string(npc_types_tint_entry.grn1h));
		insert_values.push_back(std::to_string(npc_types_tint_entry.blu1h));
		insert_values.push_back(std::to_string(npc_types_tint_entry.red2c));
		insert_values.push_back(std::to_string(npc_types_tint_entry.grn2c));
		insert_values.push_back(std::to_string(npc_types_tint_entry.blu2c));
		insert_values.push_back(std::to_string(npc_types_tint_entry.red3a));
		insert_values.push_back(std::to_string(npc_types_tint_entry.grn3a));
		insert_values.push_back(std::to_string(npc_types_tint_entry.blu3a));
		insert_values.push_back(std::to_string(npc_types_tint_entry.red4b));
		insert_values.push_back(std::to_string(npc_types_tint_entry.grn4b));
		insert_values.push_back(std::to_string(npc_types_tint_entry.blu4b));
		insert_values.push_back(std::to_string(npc_types_tint_entry.red5g));
		insert_values.push_back(std::to_string(npc_types_tint_entry.grn5g));
		insert_values.push_back(std::to_string(npc_types_tint_entry.blu5g));
		insert_values.push_back(std::to_string(npc_types_tint_entry.red6l));
		insert_values.push_back(std::to_string(npc_types_tint_entry.grn6l));
		insert_values.push_back(std::to_string(npc_types_tint_entry.blu6l));
		insert_values.push_back(std::to_string(npc_types_tint_entry.red7f));
		insert_values.push_back(std::to_string(npc_types_tint_entry.grn7f));
		insert_values.push_back(std::to_string(npc_types_tint_entry.blu7f));
		insert_values.push_back(std::to_string(npc_types_tint_entry.red8x));
		insert_values.push_back(std::to_string(npc_types_tint_entry.grn8x));
		insert_values.push_back(std::to_string(npc_types_tint_entry.blu8x));
		insert_values.push_back(std::to_string(npc_types_tint_entry.red9x));
		insert_values.push_back(std::to_string(npc_types_tint_entry.grn9x));
		insert_values.push_back(std::to_string(npc_types_tint_entry.blu9x));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			npc_types_tint_entry.id = results.LastInsertedID();
			return npc_types_tint_entry;
		}

		npc_types_tint_entry = NewEntity();

		return npc_types_tint_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<NpcTypesTint> npc_types_tint_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &npc_types_tint_entry: npc_types_tint_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(npc_types_tint_entry.id));
			insert_values.push_back("'" + EscapeString(npc_types_tint_entry.tint_set_name) + "'");
			insert_values.push_back(std::to_string(npc_types_tint_entry.red1h));
			insert_values.push_back(std::to_string(npc_types_tint_entry.grn1h));
			insert_values.push_back(std::to_string(npc_types_tint_entry.blu1h));
			insert_values.push_back(std::to_string(npc_types_tint_entry.red2c));
			insert_values.push_back(std::to_string(npc_types_tint_entry.grn2c));
			insert_values.push_back(std::to_string(npc_types_tint_entry.blu2c));
			insert_values.push_back(std::to_string(npc_types_tint_entry.red3a));
			insert_values.push_back(std::to_string(npc_types_tint_entry.grn3a));
			insert_values.push_back(std::to_string(npc_types_tint_entry.blu3a));
			insert_values.push_back(std::to_string(npc_types_tint_entry.red4b));
			insert_values.push_back(std::to_string(npc_types_tint_entry.grn4b));
			insert_values.push_back(std::to_string(npc_types_tint_entry.blu4b));
			insert_values.push_back(std::to_string(npc_types_tint_entry.red5g));
			insert_values.push_back(std::to_string(npc_types_tint_entry.grn5g));
			insert_values.push_back(std::to_string(npc_types_tint_entry.blu5g));
			insert_values.push_back(std::to_string(npc_types_tint_entry.red6l));
			insert_values.push_back(std::to_string(npc_types_tint_entry.grn6l));
			insert_values.push_back(std::to_string(npc_types_tint_entry.blu6l));
			insert_values.push_back(std::to_string(npc_types_tint_entry.red7f));
			insert_values.push_back(std::to_string(npc_types_tint_entry.grn7f));
			insert_values.push_back(std::to_string(npc_types_tint_entry.blu7f));
			insert_values.push_back(std::to_string(npc_types_tint_entry.red8x));
			insert_values.push_back(std::to_string(npc_types_tint_entry.grn8x));
			insert_values.push_back(std::to_string(npc_types_tint_entry.blu8x));
			insert_values.push_back(std::to_string(npc_types_tint_entry.red9x));
			insert_values.push_back(std::to_string(npc_types_tint_entry.grn9x));
			insert_values.push_back(std::to_string(npc_types_tint_entry.blu9x));

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

	static std::vector<NpcTypesTint> All(Database& db)
	{
		std::vector<NpcTypesTint> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcTypesTint entry{};

			entry.id            = atoi(row[0]);
			entry.tint_set_name = row[1] ? row[1] : "";
			entry.red1h         = atoi(row[2]);
			entry.grn1h         = atoi(row[3]);
			entry.blu1h         = atoi(row[4]);
			entry.red2c         = atoi(row[5]);
			entry.grn2c         = atoi(row[6]);
			entry.blu2c         = atoi(row[7]);
			entry.red3a         = atoi(row[8]);
			entry.grn3a         = atoi(row[9]);
			entry.blu3a         = atoi(row[10]);
			entry.red4b         = atoi(row[11]);
			entry.grn4b         = atoi(row[12]);
			entry.blu4b         = atoi(row[13]);
			entry.red5g         = atoi(row[14]);
			entry.grn5g         = atoi(row[15]);
			entry.blu5g         = atoi(row[16]);
			entry.red6l         = atoi(row[17]);
			entry.grn6l         = atoi(row[18]);
			entry.blu6l         = atoi(row[19]);
			entry.red7f         = atoi(row[20]);
			entry.grn7f         = atoi(row[21]);
			entry.blu7f         = atoi(row[22]);
			entry.red8x         = atoi(row[23]);
			entry.grn8x         = atoi(row[24]);
			entry.blu8x         = atoi(row[25]);
			entry.red9x         = atoi(row[26]);
			entry.grn9x         = atoi(row[27]);
			entry.blu9x         = atoi(row[28]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<NpcTypesTint> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<NpcTypesTint> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcTypesTint entry{};

			entry.id            = atoi(row[0]);
			entry.tint_set_name = row[1] ? row[1] : "";
			entry.red1h         = atoi(row[2]);
			entry.grn1h         = atoi(row[3]);
			entry.blu1h         = atoi(row[4]);
			entry.red2c         = atoi(row[5]);
			entry.grn2c         = atoi(row[6]);
			entry.blu2c         = atoi(row[7]);
			entry.red3a         = atoi(row[8]);
			entry.grn3a         = atoi(row[9]);
			entry.blu3a         = atoi(row[10]);
			entry.red4b         = atoi(row[11]);
			entry.grn4b         = atoi(row[12]);
			entry.blu4b         = atoi(row[13]);
			entry.red5g         = atoi(row[14]);
			entry.grn5g         = atoi(row[15]);
			entry.blu5g         = atoi(row[16]);
			entry.red6l         = atoi(row[17]);
			entry.grn6l         = atoi(row[18]);
			entry.blu6l         = atoi(row[19]);
			entry.red7f         = atoi(row[20]);
			entry.grn7f         = atoi(row[21]);
			entry.blu7f         = atoi(row[22]);
			entry.red8x         = atoi(row[23]);
			entry.grn8x         = atoi(row[24]);
			entry.blu8x         = atoi(row[25]);
			entry.red9x         = atoi(row[26]);
			entry.grn9x         = atoi(row[27]);
			entry.blu9x         = atoi(row[28]);

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

#endif //EQEMU_BASE_NPC_TYPES_TINT_REPOSITORY_H
