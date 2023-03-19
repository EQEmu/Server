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

#ifndef EQEMU_BASE_ITEM_TICK_REPOSITORY_H
#define EQEMU_BASE_ITEM_TICK_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseItemTickRepository {
public:
	struct ItemTick {
		int32_t     it_itemid;
		int32_t     it_chance;
		int32_t     it_level;
		int32_t     it_id;
		std::string it_qglobal;
		int8_t      it_bagslot;
	};

	static std::string PrimaryKey()
	{
		return std::string("it_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"it_itemid",
			"it_chance",
			"it_level",
			"it_id",
			"it_qglobal",
			"it_bagslot",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"it_itemid",
			"it_chance",
			"it_level",
			"it_id",
			"it_qglobal",
			"it_bagslot",
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
		return std::string("item_tick");
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

	static ItemTick NewEntity()
	{
		ItemTick e{};

		e.it_itemid  = 0;
		e.it_chance  = 0;
		e.it_level   = 0;
		e.it_id      = 0;
		e.it_qglobal = "";
		e.it_bagslot = 0;

		return e;
	}

	static ItemTick GetItemTick(
		const std::vector<ItemTick> &item_ticks,
		int item_tick_id
	)
	{
		for (auto &item_tick : item_ticks) {
			if (item_tick.it_id == item_tick_id) {
				return item_tick;
			}
		}

		return NewEntity();
	}

	static ItemTick FindOne(
		Database& db,
		int item_tick_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				item_tick_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			ItemTick e{};

			e.it_itemid  = static_cast<int32_t>(atoi(row[0]));
			e.it_chance  = static_cast<int32_t>(atoi(row[1]));
			e.it_level   = static_cast<int32_t>(atoi(row[2]));
			e.it_id      = static_cast<int32_t>(atoi(row[3]));
			e.it_qglobal = row[4] ? row[4] : "";
			e.it_bagslot = static_cast<int8_t>(atoi(row[5]));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int item_tick_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				item_tick_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const ItemTick &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.it_itemid));
		v.push_back(columns[1] + " = " + std::to_string(e.it_chance));
		v.push_back(columns[2] + " = " + std::to_string(e.it_level));
		v.push_back(columns[4] + " = '" + Strings::Escape(e.it_qglobal) + "'");
		v.push_back(columns[5] + " = " + std::to_string(e.it_bagslot));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.it_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static ItemTick InsertOne(
		Database& db,
		ItemTick e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.it_itemid));
		v.push_back(std::to_string(e.it_chance));
		v.push_back(std::to_string(e.it_level));
		v.push_back(std::to_string(e.it_id));
		v.push_back("'" + Strings::Escape(e.it_qglobal) + "'");
		v.push_back(std::to_string(e.it_bagslot));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.it_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<ItemTick> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.it_itemid));
			v.push_back(std::to_string(e.it_chance));
			v.push_back(std::to_string(e.it_level));
			v.push_back(std::to_string(e.it_id));
			v.push_back("'" + Strings::Escape(e.it_qglobal) + "'");
			v.push_back(std::to_string(e.it_bagslot));

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

	static std::vector<ItemTick> All(Database& db)
	{
		std::vector<ItemTick> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ItemTick e{};

			e.it_itemid  = static_cast<int32_t>(atoi(row[0]));
			e.it_chance  = static_cast<int32_t>(atoi(row[1]));
			e.it_level   = static_cast<int32_t>(atoi(row[2]));
			e.it_id      = static_cast<int32_t>(atoi(row[3]));
			e.it_qglobal = row[4] ? row[4] : "";
			e.it_bagslot = static_cast<int8_t>(atoi(row[5]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<ItemTick> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<ItemTick> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ItemTick e{};

			e.it_itemid  = static_cast<int32_t>(atoi(row[0]));
			e.it_chance  = static_cast<int32_t>(atoi(row[1]));
			e.it_level   = static_cast<int32_t>(atoi(row[2]));
			e.it_id      = static_cast<int32_t>(atoi(row[3]));
			e.it_qglobal = row[4] ? row[4] : "";
			e.it_bagslot = static_cast<int8_t>(atoi(row[5]));

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

#endif //EQEMU_BASE_ITEM_TICK_REPOSITORY_H
