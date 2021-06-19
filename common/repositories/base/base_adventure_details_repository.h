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

#ifndef EQEMU_BASE_ADVENTURE_DETAILS_REPOSITORY_H
#define EQEMU_BASE_ADVENTURE_DETAILS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseAdventureDetailsRepository {
public:
	struct AdventureDetails {
		int id;
		int adventure_id;
		int instance_id;
		int count;
		int assassinate_count;
		int status;
		int time_created;
		int time_zoned;
		int time_completed;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"adventure_id",
			"instance_id",
			"count",
			"assassinate_count",
			"status",
			"time_created",
			"time_zoned",
			"time_completed",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("adventure_details");
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

	static AdventureDetails NewEntity()
	{
		AdventureDetails entry{};

		entry.id                = 0;
		entry.adventure_id      = 0;
		entry.instance_id       = -1;
		entry.count             = 0;
		entry.assassinate_count = 0;
		entry.status            = 0;
		entry.time_created      = 0;
		entry.time_zoned        = 0;
		entry.time_completed    = 0;

		return entry;
	}

	static AdventureDetails GetAdventureDetailsEntry(
		const std::vector<AdventureDetails> &adventure_detailss,
		int adventure_details_id
	)
	{
		for (auto &adventure_details : adventure_detailss) {
			if (adventure_details.id == adventure_details_id) {
				return adventure_details;
			}
		}

		return NewEntity();
	}

	static AdventureDetails FindOne(
		Database& db,
		int adventure_details_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				adventure_details_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AdventureDetails entry{};

			entry.id                = atoi(row[0]);
			entry.adventure_id      = atoi(row[1]);
			entry.instance_id       = atoi(row[2]);
			entry.count             = atoi(row[3]);
			entry.assassinate_count = atoi(row[4]);
			entry.status            = atoi(row[5]);
			entry.time_created      = atoi(row[6]);
			entry.time_zoned        = atoi(row[7]);
			entry.time_completed    = atoi(row[8]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int adventure_details_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				adventure_details_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		AdventureDetails adventure_details_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(adventure_details_entry.adventure_id));
		update_values.push_back(columns[2] + " = " + std::to_string(adventure_details_entry.instance_id));
		update_values.push_back(columns[3] + " = " + std::to_string(adventure_details_entry.count));
		update_values.push_back(columns[4] + " = " + std::to_string(adventure_details_entry.assassinate_count));
		update_values.push_back(columns[5] + " = " + std::to_string(adventure_details_entry.status));
		update_values.push_back(columns[6] + " = " + std::to_string(adventure_details_entry.time_created));
		update_values.push_back(columns[7] + " = " + std::to_string(adventure_details_entry.time_zoned));
		update_values.push_back(columns[8] + " = " + std::to_string(adventure_details_entry.time_completed));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				adventure_details_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static AdventureDetails InsertOne(
		Database& db,
		AdventureDetails adventure_details_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(adventure_details_entry.id));
		insert_values.push_back(std::to_string(adventure_details_entry.adventure_id));
		insert_values.push_back(std::to_string(adventure_details_entry.instance_id));
		insert_values.push_back(std::to_string(adventure_details_entry.count));
		insert_values.push_back(std::to_string(adventure_details_entry.assassinate_count));
		insert_values.push_back(std::to_string(adventure_details_entry.status));
		insert_values.push_back(std::to_string(adventure_details_entry.time_created));
		insert_values.push_back(std::to_string(adventure_details_entry.time_zoned));
		insert_values.push_back(std::to_string(adventure_details_entry.time_completed));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			adventure_details_entry.id = results.LastInsertedID();
			return adventure_details_entry;
		}

		adventure_details_entry = NewEntity();

		return adventure_details_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<AdventureDetails> adventure_details_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &adventure_details_entry: adventure_details_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(adventure_details_entry.id));
			insert_values.push_back(std::to_string(adventure_details_entry.adventure_id));
			insert_values.push_back(std::to_string(adventure_details_entry.instance_id));
			insert_values.push_back(std::to_string(adventure_details_entry.count));
			insert_values.push_back(std::to_string(adventure_details_entry.assassinate_count));
			insert_values.push_back(std::to_string(adventure_details_entry.status));
			insert_values.push_back(std::to_string(adventure_details_entry.time_created));
			insert_values.push_back(std::to_string(adventure_details_entry.time_zoned));
			insert_values.push_back(std::to_string(adventure_details_entry.time_completed));

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

	static std::vector<AdventureDetails> All(Database& db)
	{
		std::vector<AdventureDetails> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AdventureDetails entry{};

			entry.id                = atoi(row[0]);
			entry.adventure_id      = atoi(row[1]);
			entry.instance_id       = atoi(row[2]);
			entry.count             = atoi(row[3]);
			entry.assassinate_count = atoi(row[4]);
			entry.status            = atoi(row[5]);
			entry.time_created      = atoi(row[6]);
			entry.time_zoned        = atoi(row[7]);
			entry.time_completed    = atoi(row[8]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<AdventureDetails> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<AdventureDetails> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AdventureDetails entry{};

			entry.id                = atoi(row[0]);
			entry.adventure_id      = atoi(row[1]);
			entry.instance_id       = atoi(row[2]);
			entry.count             = atoi(row[3]);
			entry.assassinate_count = atoi(row[4]);
			entry.status            = atoi(row[5]);
			entry.time_created      = atoi(row[6]);
			entry.time_zoned        = atoi(row[7]);
			entry.time_completed    = atoi(row[8]);

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

#endif //EQEMU_BASE_ADVENTURE_DETAILS_REPOSITORY_H
