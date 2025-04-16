/**
 * DO NOT MODIFY THIS FILE
 *
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to the repository extending the base.
 * Any modifications to base repositories are to be made by the generator only
 *
 * @generator ./utils/scripts/generators/repository-generator.pl
 * @docs https://docs.eqemu.io/developer/repositories
 */

#ifndef EQEMU_BASE_PERKS_REPOSITORY_H
#define EQEMU_BASE_PERKS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BasePerksRepository {
public:
	struct Perks {
		uint32_t    id;
		std::string name;
		int32_t     category;
		int32_t     type;
		int32_t     first_rank_id;
		uint8_t     enabled;
		int8_t      reset_on_death;
		int8_t      auto_grant_enabled;
		std::string description;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"name",
			"category",
			"type",
			"first_rank_id",
			"enabled",
			"reset_on_death",
			"auto_grant_enabled",
			"description",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"name",
			"category",
			"type",
			"first_rank_id",
			"enabled",
			"reset_on_death",
			"auto_grant_enabled",
			"description",
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
		return std::string("perks");
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

	static Perks NewEntity()
	{
		Perks e{};

		e.id                 = 0;
		e.name               = "";
		e.category           = -1;
		e.type               = 0;
		e.first_rank_id      = -1;
		e.enabled            = 1;
		e.reset_on_death     = 0;
		e.auto_grant_enabled = 0;
		e.description        = "";

		return e;
	}

	static Perks GetPerks(
		const std::vector<Perks> &perkss,
		int perks_id
	)
	{
		for (auto &perks : perkss) {
			if (perks.id == perks_id) {
				return perks;
			}
		}

		return NewEntity();
	}

	static Perks FindOne(
		Database& db,
		int perks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				perks_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Perks e{};

			e.id                 = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.name               = row[1] ? row[1] : "";
			e.category           = row[2] ? static_cast<int32_t>(atoi(row[2])) : -1;
			e.type               = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.first_rank_id      = row[4] ? static_cast<int32_t>(atoi(row[4])) : -1;
			e.enabled            = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 1;
			e.reset_on_death     = row[6] ? static_cast<int8_t>(atoi(row[6])) : 0;
			e.auto_grant_enabled = row[7] ? static_cast<int8_t>(atoi(row[7])) : 0;
			e.description        = row[8] ? row[8] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int perks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				perks_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Perks &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.id));
		v.push_back(columns[1] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.category));
		v.push_back(columns[3] + " = " + std::to_string(e.type));
		v.push_back(columns[4] + " = " + std::to_string(e.first_rank_id));
		v.push_back(columns[5] + " = " + std::to_string(e.enabled));
		v.push_back(columns[6] + " = " + std::to_string(e.reset_on_death));
		v.push_back(columns[7] + " = " + std::to_string(e.auto_grant_enabled));
		v.push_back(columns[8] + " = '" + Strings::Escape(e.description) + "'");

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

	static Perks InsertOne(
		Database& db,
		Perks e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.category));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.first_rank_id));
		v.push_back(std::to_string(e.enabled));
		v.push_back(std::to_string(e.reset_on_death));
		v.push_back(std::to_string(e.auto_grant_enabled));
		v.push_back("'" + Strings::Escape(e.description) + "'");

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
		const std::vector<Perks> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.category));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.first_rank_id));
			v.push_back(std::to_string(e.enabled));
			v.push_back(std::to_string(e.reset_on_death));
			v.push_back(std::to_string(e.auto_grant_enabled));
			v.push_back("'" + Strings::Escape(e.description) + "'");

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

	static std::vector<Perks> All(Database& db)
	{
		std::vector<Perks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Perks e{};

			e.id                 = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.name               = row[1] ? row[1] : "";
			e.category           = row[2] ? static_cast<int32_t>(atoi(row[2])) : -1;
			e.type               = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.first_rank_id      = row[4] ? static_cast<int32_t>(atoi(row[4])) : -1;
			e.enabled            = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 1;
			e.reset_on_death     = row[6] ? static_cast<int8_t>(atoi(row[6])) : 0;
			e.auto_grant_enabled = row[7] ? static_cast<int8_t>(atoi(row[7])) : 0;
			e.description        = row[8] ? row[8] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Perks> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Perks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Perks e{};

			e.id                 = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.name               = row[1] ? row[1] : "";
			e.category           = row[2] ? static_cast<int32_t>(atoi(row[2])) : -1;
			e.type               = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;
			e.first_rank_id      = row[4] ? static_cast<int32_t>(atoi(row[4])) : -1;
			e.enabled            = row[5] ? static_cast<uint8_t>(strtoul(row[5], nullptr, 10)) : 1;
			e.reset_on_death     = row[6] ? static_cast<int8_t>(atoi(row[6])) : 0;
			e.auto_grant_enabled = row[7] ? static_cast<int8_t>(atoi(row[7])) : 0;
			e.description        = row[8] ? row[8] : "";

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

	static std::string BaseReplace()
	{
		return fmt::format(
			"REPLACE INTO {} ({}) ",
			TableName(),
			ColumnsRaw()
		);
	}

	static int ReplaceOne(
		Database& db,
		const Perks &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.category));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.first_rank_id));
		v.push_back(std::to_string(e.enabled));
		v.push_back(std::to_string(e.reset_on_death));
		v.push_back(std::to_string(e.auto_grant_enabled));
		v.push_back("'" + Strings::Escape(e.description) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseReplace(),
				Strings::Implode(",", v)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int ReplaceMany(
		Database& db,
		const std::vector<Perks> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.category));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.first_rank_id));
			v.push_back(std::to_string(e.enabled));
			v.push_back(std::to_string(e.reset_on_death));
			v.push_back(std::to_string(e.auto_grant_enabled));
			v.push_back("'" + Strings::Escape(e.description) + "'");

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseReplace(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}
};

#endif //EQEMU_BASE_PERKS_REPOSITORY_H
