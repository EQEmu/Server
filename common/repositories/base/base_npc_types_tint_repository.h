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
#include "../../strings.h"
#include <ctime>

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

	static std::vector<std::string> SelectColumns()
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
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("npc_types_tint");
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

	static NpcTypesTint NewEntity()
	{
		NpcTypesTint e{};

		e.id            = 0;
		e.tint_set_name = "";
		e.red1h         = 0;
		e.grn1h         = 0;
		e.blu1h         = 0;
		e.red2c         = 0;
		e.grn2c         = 0;
		e.blu2c         = 0;
		e.red3a         = 0;
		e.grn3a         = 0;
		e.blu3a         = 0;
		e.red4b         = 0;
		e.grn4b         = 0;
		e.blu4b         = 0;
		e.red5g         = 0;
		e.grn5g         = 0;
		e.blu5g         = 0;
		e.red6l         = 0;
		e.grn6l         = 0;
		e.blu6l         = 0;
		e.red7f         = 0;
		e.grn7f         = 0;
		e.blu7f         = 0;
		e.red8x         = 0;
		e.grn8x         = 0;
		e.blu8x         = 0;
		e.red9x         = 0;
		e.grn9x         = 0;
		e.blu9x         = 0;

		return e;
	}

	static NpcTypesTint GetNpcTypesTint(
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
			NpcTypesTint e{};

			e.id            = atoi(row[0]);
			e.tint_set_name = row[1] ? row[1] : "";
			e.red1h         = atoi(row[2]);
			e.grn1h         = atoi(row[3]);
			e.blu1h         = atoi(row[4]);
			e.red2c         = atoi(row[5]);
			e.grn2c         = atoi(row[6]);
			e.blu2c         = atoi(row[7]);
			e.red3a         = atoi(row[8]);
			e.grn3a         = atoi(row[9]);
			e.blu3a         = atoi(row[10]);
			e.red4b         = atoi(row[11]);
			e.grn4b         = atoi(row[12]);
			e.blu4b         = atoi(row[13]);
			e.red5g         = atoi(row[14]);
			e.grn5g         = atoi(row[15]);
			e.blu5g         = atoi(row[16]);
			e.red6l         = atoi(row[17]);
			e.grn6l         = atoi(row[18]);
			e.blu6l         = atoi(row[19]);
			e.red7f         = atoi(row[20]);
			e.grn7f         = atoi(row[21]);
			e.blu7f         = atoi(row[22]);
			e.red8x         = atoi(row[23]);
			e.grn8x         = atoi(row[24]);
			e.blu8x         = atoi(row[25]);
			e.red9x         = atoi(row[26]);
			e.grn9x         = atoi(row[27]);
			e.blu9x         = atoi(row[28]);

			return e;
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
		const NpcTypesTint &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.id));
		v.push_back(columns[1] + " = '" + Strings::Escape(e.tint_set_name) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.red1h));
		v.push_back(columns[3] + " = " + std::to_string(e.grn1h));
		v.push_back(columns[4] + " = " + std::to_string(e.blu1h));
		v.push_back(columns[5] + " = " + std::to_string(e.red2c));
		v.push_back(columns[6] + " = " + std::to_string(e.grn2c));
		v.push_back(columns[7] + " = " + std::to_string(e.blu2c));
		v.push_back(columns[8] + " = " + std::to_string(e.red3a));
		v.push_back(columns[9] + " = " + std::to_string(e.grn3a));
		v.push_back(columns[10] + " = " + std::to_string(e.blu3a));
		v.push_back(columns[11] + " = " + std::to_string(e.red4b));
		v.push_back(columns[12] + " = " + std::to_string(e.grn4b));
		v.push_back(columns[13] + " = " + std::to_string(e.blu4b));
		v.push_back(columns[14] + " = " + std::to_string(e.red5g));
		v.push_back(columns[15] + " = " + std::to_string(e.grn5g));
		v.push_back(columns[16] + " = " + std::to_string(e.blu5g));
		v.push_back(columns[17] + " = " + std::to_string(e.red6l));
		v.push_back(columns[18] + " = " + std::to_string(e.grn6l));
		v.push_back(columns[19] + " = " + std::to_string(e.blu6l));
		v.push_back(columns[20] + " = " + std::to_string(e.red7f));
		v.push_back(columns[21] + " = " + std::to_string(e.grn7f));
		v.push_back(columns[22] + " = " + std::to_string(e.blu7f));
		v.push_back(columns[23] + " = " + std::to_string(e.red8x));
		v.push_back(columns[24] + " = " + std::to_string(e.grn8x));
		v.push_back(columns[25] + " = " + std::to_string(e.blu8x));
		v.push_back(columns[26] + " = " + std::to_string(e.red9x));
		v.push_back(columns[27] + " = " + std::to_string(e.grn9x));
		v.push_back(columns[28] + " = " + std::to_string(e.blu9x));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static NpcTypesTint InsertOne(
		Database& db,
		NpcTypesTint e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.tint_set_name) + "'");
		v.push_back(std::to_string(e.red1h));
		v.push_back(std::to_string(e.grn1h));
		v.push_back(std::to_string(e.blu1h));
		v.push_back(std::to_string(e.red2c));
		v.push_back(std::to_string(e.grn2c));
		v.push_back(std::to_string(e.blu2c));
		v.push_back(std::to_string(e.red3a));
		v.push_back(std::to_string(e.grn3a));
		v.push_back(std::to_string(e.blu3a));
		v.push_back(std::to_string(e.red4b));
		v.push_back(std::to_string(e.grn4b));
		v.push_back(std::to_string(e.blu4b));
		v.push_back(std::to_string(e.red5g));
		v.push_back(std::to_string(e.grn5g));
		v.push_back(std::to_string(e.blu5g));
		v.push_back(std::to_string(e.red6l));
		v.push_back(std::to_string(e.grn6l));
		v.push_back(std::to_string(e.blu6l));
		v.push_back(std::to_string(e.red7f));
		v.push_back(std::to_string(e.grn7f));
		v.push_back(std::to_string(e.blu7f));
		v.push_back(std::to_string(e.red8x));
		v.push_back(std::to_string(e.grn8x));
		v.push_back(std::to_string(e.blu8x));
		v.push_back(std::to_string(e.red9x));
		v.push_back(std::to_string(e.grn9x));
		v.push_back(std::to_string(e.blu9x));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<NpcTypesTint> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.tint_set_name) + "'");
			v.push_back(std::to_string(e.red1h));
			v.push_back(std::to_string(e.grn1h));
			v.push_back(std::to_string(e.blu1h));
			v.push_back(std::to_string(e.red2c));
			v.push_back(std::to_string(e.grn2c));
			v.push_back(std::to_string(e.blu2c));
			v.push_back(std::to_string(e.red3a));
			v.push_back(std::to_string(e.grn3a));
			v.push_back(std::to_string(e.blu3a));
			v.push_back(std::to_string(e.red4b));
			v.push_back(std::to_string(e.grn4b));
			v.push_back(std::to_string(e.blu4b));
			v.push_back(std::to_string(e.red5g));
			v.push_back(std::to_string(e.grn5g));
			v.push_back(std::to_string(e.blu5g));
			v.push_back(std::to_string(e.red6l));
			v.push_back(std::to_string(e.grn6l));
			v.push_back(std::to_string(e.blu6l));
			v.push_back(std::to_string(e.red7f));
			v.push_back(std::to_string(e.grn7f));
			v.push_back(std::to_string(e.blu7f));
			v.push_back(std::to_string(e.red8x));
			v.push_back(std::to_string(e.grn8x));
			v.push_back(std::to_string(e.blu8x));
			v.push_back(std::to_string(e.red9x));
			v.push_back(std::to_string(e.grn9x));
			v.push_back(std::to_string(e.blu9x));

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
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
			NpcTypesTint e{};

			e.id            = atoi(row[0]);
			e.tint_set_name = row[1] ? row[1] : "";
			e.red1h         = atoi(row[2]);
			e.grn1h         = atoi(row[3]);
			e.blu1h         = atoi(row[4]);
			e.red2c         = atoi(row[5]);
			e.grn2c         = atoi(row[6]);
			e.blu2c         = atoi(row[7]);
			e.red3a         = atoi(row[8]);
			e.grn3a         = atoi(row[9]);
			e.blu3a         = atoi(row[10]);
			e.red4b         = atoi(row[11]);
			e.grn4b         = atoi(row[12]);
			e.blu4b         = atoi(row[13]);
			e.red5g         = atoi(row[14]);
			e.grn5g         = atoi(row[15]);
			e.blu5g         = atoi(row[16]);
			e.red6l         = atoi(row[17]);
			e.grn6l         = atoi(row[18]);
			e.blu6l         = atoi(row[19]);
			e.red7f         = atoi(row[20]);
			e.grn7f         = atoi(row[21]);
			e.blu7f         = atoi(row[22]);
			e.red8x         = atoi(row[23]);
			e.grn8x         = atoi(row[24]);
			e.blu8x         = atoi(row[25]);
			e.red9x         = atoi(row[26]);
			e.grn9x         = atoi(row[27]);
			e.blu9x         = atoi(row[28]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<NpcTypesTint> GetWhere(Database& db, const std::string &where_filter)
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
			NpcTypesTint e{};

			e.id            = atoi(row[0]);
			e.tint_set_name = row[1] ? row[1] : "";
			e.red1h         = atoi(row[2]);
			e.grn1h         = atoi(row[3]);
			e.blu1h         = atoi(row[4]);
			e.red2c         = atoi(row[5]);
			e.grn2c         = atoi(row[6]);
			e.blu2c         = atoi(row[7]);
			e.red3a         = atoi(row[8]);
			e.grn3a         = atoi(row[9]);
			e.blu3a         = atoi(row[10]);
			e.red4b         = atoi(row[11]);
			e.grn4b         = atoi(row[12]);
			e.blu4b         = atoi(row[13]);
			e.red5g         = atoi(row[14]);
			e.grn5g         = atoi(row[15]);
			e.blu5g         = atoi(row[16]);
			e.red6l         = atoi(row[17]);
			e.grn6l         = atoi(row[18]);
			e.blu6l         = atoi(row[19]);
			e.red7f         = atoi(row[20]);
			e.grn7f         = atoi(row[21]);
			e.blu7f         = atoi(row[22]);
			e.red8x         = atoi(row[23]);
			e.grn8x         = atoi(row[24]);
			e.blu8x         = atoi(row[25]);
			e.red9x         = atoi(row[26]);
			e.grn9x         = atoi(row[27]);
			e.blu9x         = atoi(row[28]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static int DeleteWhere(Database& db, const std::string &where_filter)
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
				"SELECT COALESCE(MAX({}), 0) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string &where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

};

#endif //EQEMU_BASE_NPC_TYPES_TINT_REPOSITORY_H
