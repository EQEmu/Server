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

#ifndef EQEMU_BASE_FACTION_BASE_DATA_REPOSITORY_H
#define EQEMU_BASE_FACTION_BASE_DATA_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseFactionBaseDataRepository {
public:
	struct FactionBaseData {
		int16_t client_faction_id;
		int16_t min;
		int16_t max;
		int16_t unk_hero1;
		int16_t unk_hero2;
		int16_t unk_hero3;
	};

	static std::string PrimaryKey()
	{
		return std::string("client_faction_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"client_faction_id",
			"min",
			"max",
			"unk_hero1",
			"unk_hero2",
			"unk_hero3",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"client_faction_id",
			"min",
			"max",
			"unk_hero1",
			"unk_hero2",
			"unk_hero3",
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
		return std::string("faction_base_data");
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

	static FactionBaseData NewEntity()
	{
		FactionBaseData e{};

		e.client_faction_id = 0;
		e.min               = -2000;
		e.max               = 2000;
		e.unk_hero1         = 0;
		e.unk_hero2         = 0;
		e.unk_hero3         = 0;

		return e;
	}

	static FactionBaseData GetFactionBaseData(
		const std::vector<FactionBaseData> &faction_base_datas,
		int faction_base_data_id
	)
	{
		for (auto &faction_base_data : faction_base_datas) {
			if (faction_base_data.client_faction_id == faction_base_data_id) {
				return faction_base_data;
			}
		}

		return NewEntity();
	}

	static FactionBaseData FindOne(
		Database& db,
		int faction_base_data_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				faction_base_data_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			FactionBaseData e{};

			e.client_faction_id = static_cast<int16_t>(atoi(row[0]));
			e.min               = static_cast<int16_t>(atoi(row[1]));
			e.max               = static_cast<int16_t>(atoi(row[2]));
			e.unk_hero1         = static_cast<int16_t>(atoi(row[3]));
			e.unk_hero2         = static_cast<int16_t>(atoi(row[4]));
			e.unk_hero3         = static_cast<int16_t>(atoi(row[5]));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int faction_base_data_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				faction_base_data_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const FactionBaseData &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.client_faction_id));
		v.push_back(columns[1] + " = " + std::to_string(e.min));
		v.push_back(columns[2] + " = " + std::to_string(e.max));
		v.push_back(columns[3] + " = " + std::to_string(e.unk_hero1));
		v.push_back(columns[4] + " = " + std::to_string(e.unk_hero2));
		v.push_back(columns[5] + " = " + std::to_string(e.unk_hero3));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.client_faction_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static FactionBaseData InsertOne(
		Database& db,
		FactionBaseData e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.client_faction_id));
		v.push_back(std::to_string(e.min));
		v.push_back(std::to_string(e.max));
		v.push_back(std::to_string(e.unk_hero1));
		v.push_back(std::to_string(e.unk_hero2));
		v.push_back(std::to_string(e.unk_hero3));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.client_faction_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<FactionBaseData> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.client_faction_id));
			v.push_back(std::to_string(e.min));
			v.push_back(std::to_string(e.max));
			v.push_back(std::to_string(e.unk_hero1));
			v.push_back(std::to_string(e.unk_hero2));
			v.push_back(std::to_string(e.unk_hero3));

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

	static std::vector<FactionBaseData> All(Database& db)
	{
		std::vector<FactionBaseData> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			FactionBaseData e{};

			e.client_faction_id = static_cast<int16_t>(atoi(row[0]));
			e.min               = static_cast<int16_t>(atoi(row[1]));
			e.max               = static_cast<int16_t>(atoi(row[2]));
			e.unk_hero1         = static_cast<int16_t>(atoi(row[3]));
			e.unk_hero2         = static_cast<int16_t>(atoi(row[4]));
			e.unk_hero3         = static_cast<int16_t>(atoi(row[5]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<FactionBaseData> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<FactionBaseData> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			FactionBaseData e{};

			e.client_faction_id = static_cast<int16_t>(atoi(row[0]));
			e.min               = static_cast<int16_t>(atoi(row[1]));
			e.max               = static_cast<int16_t>(atoi(row[2]));
			e.unk_hero1         = static_cast<int16_t>(atoi(row[3]));
			e.unk_hero2         = static_cast<int16_t>(atoi(row[4]));
			e.unk_hero3         = static_cast<int16_t>(atoi(row[5]));

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

#endif //EQEMU_BASE_FACTION_BASE_DATA_REPOSITORY_H
