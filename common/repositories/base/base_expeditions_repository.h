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

#ifndef EQEMU_BASE_EXPEDITIONS_REPOSITORY_H
#define EQEMU_BASE_EXPEDITIONS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseExpeditionsRepository {
public:
	struct Expeditions {
		uint32_t id;
		uint32_t dynamic_zone_id;
		uint8_t  add_replay_on_join;
		uint8_t  is_locked;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"dynamic_zone_id",
			"add_replay_on_join",
			"is_locked",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"dynamic_zone_id",
			"add_replay_on_join",
			"is_locked",
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
		return std::string("expeditions");
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

	static Expeditions NewEntity()
	{
		Expeditions e{};

		e.id                 = 0;
		e.dynamic_zone_id    = 0;
		e.add_replay_on_join = 1;
		e.is_locked          = 0;

		return e;
	}

	static Expeditions GetExpeditions(
		const std::vector<Expeditions> &expeditionss,
		int expeditions_id
	)
	{
		for (auto &expeditions : expeditionss) {
			if (expeditions.id == expeditions_id) {
				return expeditions;
			}
		}

		return NewEntity();
	}

	static Expeditions FindOne(
		Database& db,
		int expeditions_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				expeditions_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Expeditions e{};

			e.id                 = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.dynamic_zone_id    = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.add_replay_on_join = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.is_locked          = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int expeditions_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				expeditions_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Expeditions &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.dynamic_zone_id));
		v.push_back(columns[2] + " = " + std::to_string(e.add_replay_on_join));
		v.push_back(columns[3] + " = " + std::to_string(e.is_locked));

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

	static Expeditions InsertOne(
		Database& db,
		Expeditions e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.dynamic_zone_id));
		v.push_back(std::to_string(e.add_replay_on_join));
		v.push_back(std::to_string(e.is_locked));

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
		const std::vector<Expeditions> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.dynamic_zone_id));
			v.push_back(std::to_string(e.add_replay_on_join));
			v.push_back(std::to_string(e.is_locked));

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

	static std::vector<Expeditions> All(Database& db)
	{
		std::vector<Expeditions> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Expeditions e{};

			e.id                 = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.dynamic_zone_id    = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.add_replay_on_join = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.is_locked          = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Expeditions> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Expeditions> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Expeditions e{};

			e.id                 = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.dynamic_zone_id    = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.add_replay_on_join = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.is_locked          = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));

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

#endif //EQEMU_BASE_EXPEDITIONS_REPOSITORY_H
