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

#ifndef EQEMU_BASE_SPLASH_FACTION_REPOSITORY_H
#define EQEMU_BASE_SPLASH_FACTION_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"
#include <ctime>

class BaseSplashFactionRepository {
public:
	struct SplashFaction {
		int   id;
		int   splash_id1;
		float splash_mod1;
		int   splash_id2;
		float splash_mod2;
		int   splash_id3;
		float splash_mod3;
		int   splash_id4;
		float splash_mod4;
		int   splash_id5;
		float splash_mod5;
		int   splash_id6;
		float splash_mod6;
		int   splash_id7;
		float splash_mod7;
		int   splash_id8;
		float splash_mod8;
		int   splash_id9;
		float splash_mod9;
		int   splash_id10;
		float splash_mod10;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"splash_id1",
			"splash_mod1",
			"splash_id2",
			"splash_mod2",
			"splash_id3",
			"splash_mod3",
			"splash_id4",
			"splash_mod4",
			"splash_id5",
			"splash_mod5",
			"splash_id6",
			"splash_mod6",
			"splash_id7",
			"splash_mod7",
			"splash_id8",
			"splash_mod8",
			"splash_id9",
			"splash_mod9",
			"splash_id10",
			"splash_mod10",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"splash_id1",
			"splash_mod1",
			"splash_id2",
			"splash_mod2",
			"splash_id3",
			"splash_mod3",
			"splash_id4",
			"splash_mod4",
			"splash_id5",
			"splash_mod5",
			"splash_id6",
			"splash_mod6",
			"splash_id7",
			"splash_mod7",
			"splash_id8",
			"splash_mod8",
			"splash_id9",
			"splash_mod9",
			"splash_id10",
			"splash_mod10",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("splash_faction");
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

	static SplashFaction NewEntity()
	{
		SplashFaction entry{};

		entry.id           = 0;
		entry.splash_id1   = 0;
		entry.splash_mod1  = 0;
		entry.splash_id2   = 0;
		entry.splash_mod2  = 0;
		entry.splash_id3   = 0;
		entry.splash_mod3  = 0;
		entry.splash_id4   = 0;
		entry.splash_mod4  = 0;
		entry.splash_id5   = 0;
		entry.splash_mod5  = 0;
		entry.splash_id6   = 0;
		entry.splash_mod6  = 0;
		entry.splash_id7   = 0;
		entry.splash_mod7  = 0;
		entry.splash_id8   = 0;
		entry.splash_mod8  = 0;
		entry.splash_id9   = 0;
		entry.splash_mod9  = 0;
		entry.splash_id10  = 0;
		entry.splash_mod10 = 0;

		return entry;
	}

	static SplashFaction GetSplashFactionEntry(
		const std::vector<SplashFaction> &splash_factions,
		int splash_faction_id
	)
	{
		for (auto &splash_faction : splash_factions) {
			if (splash_faction.id == splash_faction_id) {
				return splash_faction;
			}
		}

		return NewEntity();
	}

	static SplashFaction FindOne(
		Database& db,
		int splash_faction_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				splash_faction_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			SplashFaction entry{};

			entry.id           = atoi(row[0]);
			entry.splash_id1   = atoi(row[1]);
			entry.splash_mod1  = static_cast<float>(atof(row[2]));
			entry.splash_id2   = atoi(row[3]);
			entry.splash_mod2  = static_cast<float>(atof(row[4]));
			entry.splash_id3   = atoi(row[5]);
			entry.splash_mod3  = static_cast<float>(atof(row[6]));
			entry.splash_id4   = atoi(row[7]);
			entry.splash_mod4  = static_cast<float>(atof(row[8]));
			entry.splash_id5   = atoi(row[9]);
			entry.splash_mod5  = static_cast<float>(atof(row[10]));
			entry.splash_id6   = atoi(row[11]);
			entry.splash_mod6  = static_cast<float>(atof(row[12]));
			entry.splash_id7   = atoi(row[13]);
			entry.splash_mod7  = static_cast<float>(atof(row[14]));
			entry.splash_id8   = atoi(row[15]);
			entry.splash_mod8  = static_cast<float>(atof(row[16]));
			entry.splash_id9   = atoi(row[17]);
			entry.splash_mod9  = static_cast<float>(atof(row[18]));
			entry.splash_id10  = atoi(row[19]);
			entry.splash_mod10 = static_cast<float>(atof(row[20]));

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int splash_faction_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				splash_faction_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		SplashFaction splash_faction_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(splash_faction_entry.id));
		update_values.push_back(columns[1] + " = " + std::to_string(splash_faction_entry.splash_id1));
		update_values.push_back(columns[2] + " = " + std::to_string(splash_faction_entry.splash_mod1));
		update_values.push_back(columns[3] + " = " + std::to_string(splash_faction_entry.splash_id2));
		update_values.push_back(columns[4] + " = " + std::to_string(splash_faction_entry.splash_mod2));
		update_values.push_back(columns[5] + " = " + std::to_string(splash_faction_entry.splash_id3));
		update_values.push_back(columns[6] + " = " + std::to_string(splash_faction_entry.splash_mod3));
		update_values.push_back(columns[7] + " = " + std::to_string(splash_faction_entry.splash_id4));
		update_values.push_back(columns[8] + " = " + std::to_string(splash_faction_entry.splash_mod4));
		update_values.push_back(columns[9] + " = " + std::to_string(splash_faction_entry.splash_id5));
		update_values.push_back(columns[10] + " = " + std::to_string(splash_faction_entry.splash_mod5));
		update_values.push_back(columns[11] + " = " + std::to_string(splash_faction_entry.splash_id6));
		update_values.push_back(columns[12] + " = " + std::to_string(splash_faction_entry.splash_mod6));
		update_values.push_back(columns[13] + " = " + std::to_string(splash_faction_entry.splash_id7));
		update_values.push_back(columns[14] + " = " + std::to_string(splash_faction_entry.splash_mod7));
		update_values.push_back(columns[15] + " = " + std::to_string(splash_faction_entry.splash_id8));
		update_values.push_back(columns[16] + " = " + std::to_string(splash_faction_entry.splash_mod8));
		update_values.push_back(columns[17] + " = " + std::to_string(splash_faction_entry.splash_id9));
		update_values.push_back(columns[18] + " = " + std::to_string(splash_faction_entry.splash_mod9));
		update_values.push_back(columns[19] + " = " + std::to_string(splash_faction_entry.splash_id10));
		update_values.push_back(columns[20] + " = " + std::to_string(splash_faction_entry.splash_mod10));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				splash_faction_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static SplashFaction InsertOne(
		Database& db,
		SplashFaction splash_faction_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(splash_faction_entry.id));
		insert_values.push_back(std::to_string(splash_faction_entry.splash_id1));
		insert_values.push_back(std::to_string(splash_faction_entry.splash_mod1));
		insert_values.push_back(std::to_string(splash_faction_entry.splash_id2));
		insert_values.push_back(std::to_string(splash_faction_entry.splash_mod2));
		insert_values.push_back(std::to_string(splash_faction_entry.splash_id3));
		insert_values.push_back(std::to_string(splash_faction_entry.splash_mod3));
		insert_values.push_back(std::to_string(splash_faction_entry.splash_id4));
		insert_values.push_back(std::to_string(splash_faction_entry.splash_mod4));
		insert_values.push_back(std::to_string(splash_faction_entry.splash_id5));
		insert_values.push_back(std::to_string(splash_faction_entry.splash_mod5));
		insert_values.push_back(std::to_string(splash_faction_entry.splash_id6));
		insert_values.push_back(std::to_string(splash_faction_entry.splash_mod6));
		insert_values.push_back(std::to_string(splash_faction_entry.splash_id7));
		insert_values.push_back(std::to_string(splash_faction_entry.splash_mod7));
		insert_values.push_back(std::to_string(splash_faction_entry.splash_id8));
		insert_values.push_back(std::to_string(splash_faction_entry.splash_mod8));
		insert_values.push_back(std::to_string(splash_faction_entry.splash_id9));
		insert_values.push_back(std::to_string(splash_faction_entry.splash_mod9));
		insert_values.push_back(std::to_string(splash_faction_entry.splash_id10));
		insert_values.push_back(std::to_string(splash_faction_entry.splash_mod10));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			splash_faction_entry.id = results.LastInsertedID();
			return splash_faction_entry;
		}

		splash_faction_entry = NewEntity();

		return splash_faction_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<SplashFaction> splash_faction_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &splash_faction_entry: splash_faction_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(splash_faction_entry.id));
			insert_values.push_back(std::to_string(splash_faction_entry.splash_id1));
			insert_values.push_back(std::to_string(splash_faction_entry.splash_mod1));
			insert_values.push_back(std::to_string(splash_faction_entry.splash_id2));
			insert_values.push_back(std::to_string(splash_faction_entry.splash_mod2));
			insert_values.push_back(std::to_string(splash_faction_entry.splash_id3));
			insert_values.push_back(std::to_string(splash_faction_entry.splash_mod3));
			insert_values.push_back(std::to_string(splash_faction_entry.splash_id4));
			insert_values.push_back(std::to_string(splash_faction_entry.splash_mod4));
			insert_values.push_back(std::to_string(splash_faction_entry.splash_id5));
			insert_values.push_back(std::to_string(splash_faction_entry.splash_mod5));
			insert_values.push_back(std::to_string(splash_faction_entry.splash_id6));
			insert_values.push_back(std::to_string(splash_faction_entry.splash_mod6));
			insert_values.push_back(std::to_string(splash_faction_entry.splash_id7));
			insert_values.push_back(std::to_string(splash_faction_entry.splash_mod7));
			insert_values.push_back(std::to_string(splash_faction_entry.splash_id8));
			insert_values.push_back(std::to_string(splash_faction_entry.splash_mod8));
			insert_values.push_back(std::to_string(splash_faction_entry.splash_id9));
			insert_values.push_back(std::to_string(splash_faction_entry.splash_mod9));
			insert_values.push_back(std::to_string(splash_faction_entry.splash_id10));
			insert_values.push_back(std::to_string(splash_faction_entry.splash_mod10));

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

	static std::vector<SplashFaction> All(Database& db)
	{
		std::vector<SplashFaction> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SplashFaction entry{};

			entry.id           = atoi(row[0]);
			entry.splash_id1   = atoi(row[1]);
			entry.splash_mod1  = static_cast<float>(atof(row[2]));
			entry.splash_id2   = atoi(row[3]);
			entry.splash_mod2  = static_cast<float>(atof(row[4]));
			entry.splash_id3   = atoi(row[5]);
			entry.splash_mod3  = static_cast<float>(atof(row[6]));
			entry.splash_id4   = atoi(row[7]);
			entry.splash_mod4  = static_cast<float>(atof(row[8]));
			entry.splash_id5   = atoi(row[9]);
			entry.splash_mod5  = static_cast<float>(atof(row[10]));
			entry.splash_id6   = atoi(row[11]);
			entry.splash_mod6  = static_cast<float>(atof(row[12]));
			entry.splash_id7   = atoi(row[13]);
			entry.splash_mod7  = static_cast<float>(atof(row[14]));
			entry.splash_id8   = atoi(row[15]);
			entry.splash_mod8  = static_cast<float>(atof(row[16]));
			entry.splash_id9   = atoi(row[17]);
			entry.splash_mod9  = static_cast<float>(atof(row[18]));
			entry.splash_id10  = atoi(row[19]);
			entry.splash_mod10 = static_cast<float>(atof(row[20]));

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<SplashFaction> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<SplashFaction> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SplashFaction entry{};

			entry.id           = atoi(row[0]);
			entry.splash_id1   = atoi(row[1]);
			entry.splash_mod1  = static_cast<float>(atof(row[2]));
			entry.splash_id2   = atoi(row[3]);
			entry.splash_mod2  = static_cast<float>(atof(row[4]));
			entry.splash_id3   = atoi(row[5]);
			entry.splash_mod3  = static_cast<float>(atof(row[6]));
			entry.splash_id4   = atoi(row[7]);
			entry.splash_mod4  = static_cast<float>(atof(row[8]));
			entry.splash_id5   = atoi(row[9]);
			entry.splash_mod5  = static_cast<float>(atof(row[10]));
			entry.splash_id6   = atoi(row[11]);
			entry.splash_mod6  = static_cast<float>(atof(row[12]));
			entry.splash_id7   = atoi(row[13]);
			entry.splash_mod7  = static_cast<float>(atof(row[14]));
			entry.splash_id8   = atoi(row[15]);
			entry.splash_mod8  = static_cast<float>(atof(row[16]));
			entry.splash_id9   = atoi(row[17]);
			entry.splash_mod9  = static_cast<float>(atof(row[18]));
			entry.splash_id10  = atoi(row[19]);
			entry.splash_mod10 = static_cast<float>(atof(row[20]));

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

#endif //EQEMU_BASE_SPLASH_FACTION_REPOSITORY_H
