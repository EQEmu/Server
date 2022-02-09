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
#include "../../string_util.h"

class BaseItemTickRepository {
public:
	struct ItemTick {
		int         it_itemid;
		int         it_chance;
		int         it_level;
		int         it_id;
		std::string it_qglobal;
		int         it_bagslot;
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

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("item_tick");
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

	static ItemTick NewEntity()
	{
		ItemTick entry{};

		entry.it_itemid  = 0;
		entry.it_chance  = 0;
		entry.it_level   = 0;
		entry.it_id      = 0;
		entry.it_qglobal = "";
		entry.it_bagslot = 0;

		return entry;
	}

	static ItemTick GetItemTickEntry(
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
			ItemTick entry{};

			entry.it_itemid  = atoi(row[0]);
			entry.it_chance  = atoi(row[1]);
			entry.it_level   = atoi(row[2]);
			entry.it_id      = atoi(row[3]);
			entry.it_qglobal = row[4] ? row[4] : "";
			entry.it_bagslot = atoi(row[5]);

			return entry;
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
		ItemTick item_tick_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(item_tick_entry.it_itemid));
		update_values.push_back(columns[1] + " = " + std::to_string(item_tick_entry.it_chance));
		update_values.push_back(columns[2] + " = " + std::to_string(item_tick_entry.it_level));
		update_values.push_back(columns[4] + " = '" + EscapeString(item_tick_entry.it_qglobal) + "'");
		update_values.push_back(columns[5] + " = " + std::to_string(item_tick_entry.it_bagslot));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				item_tick_entry.it_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static ItemTick InsertOne(
		Database& db,
		ItemTick item_tick_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(item_tick_entry.it_itemid));
		insert_values.push_back(std::to_string(item_tick_entry.it_chance));
		insert_values.push_back(std::to_string(item_tick_entry.it_level));
		insert_values.push_back(std::to_string(item_tick_entry.it_id));
		insert_values.push_back("'" + EscapeString(item_tick_entry.it_qglobal) + "'");
		insert_values.push_back(std::to_string(item_tick_entry.it_bagslot));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			item_tick_entry.it_id = results.LastInsertedID();
			return item_tick_entry;
		}

		item_tick_entry = NewEntity();

		return item_tick_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<ItemTick> item_tick_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &item_tick_entry: item_tick_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(item_tick_entry.it_itemid));
			insert_values.push_back(std::to_string(item_tick_entry.it_chance));
			insert_values.push_back(std::to_string(item_tick_entry.it_level));
			insert_values.push_back(std::to_string(item_tick_entry.it_id));
			insert_values.push_back("'" + EscapeString(item_tick_entry.it_qglobal) + "'");
			insert_values.push_back(std::to_string(item_tick_entry.it_bagslot));

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
			ItemTick entry{};

			entry.it_itemid  = atoi(row[0]);
			entry.it_chance  = atoi(row[1]);
			entry.it_level   = atoi(row[2]);
			entry.it_id      = atoi(row[3]);
			entry.it_qglobal = row[4] ? row[4] : "";
			entry.it_bagslot = atoi(row[5]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<ItemTick> GetWhere(Database& db, std::string where_filter)
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
			ItemTick entry{};

			entry.it_itemid  = atoi(row[0]);
			entry.it_chance  = atoi(row[1]);
			entry.it_level   = atoi(row[2]);
			entry.it_id      = atoi(row[3]);
			entry.it_qglobal = row[4] ? row[4] : "";
			entry.it_bagslot = atoi(row[5]);

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

#endif //EQEMU_BASE_ITEM_TICK_REPOSITORY_H
