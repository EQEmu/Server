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
#include "../../strings.h"
#include <ctime>

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

	static std::vector<std::string> SelectColumns()
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
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("adventure_details");
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

	static AdventureDetails NewEntity()
	{
		AdventureDetails e{};

		e.id                = 0;
		e.adventure_id      = 0;
		e.instance_id       = -1;
		e.count             = 0;
		e.assassinate_count = 0;
		e.status            = 0;
		e.time_created      = 0;
		e.time_zoned        = 0;
		e.time_completed    = 0;

		return e;
	}

	static AdventureDetails GetAdventureDetailse(
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
			AdventureDetails e{};

			e.id                = atoi(row[0]);
			e.adventure_id      = atoi(row[1]);
			e.instance_id       = atoi(row[2]);
			e.count             = atoi(row[3]);
			e.assassinate_count = atoi(row[4]);
			e.status            = atoi(row[5]);
			e.time_created      = atoi(row[6]);
			e.time_zoned        = atoi(row[7]);
			e.time_completed    = atoi(row[8]);

			return e;
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
		AdventureDetails adventure_details_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(adventure_details_e.adventure_id));
		update_values.push_back(columns[2] + " = " + std::to_string(adventure_details_e.instance_id));
		update_values.push_back(columns[3] + " = " + std::to_string(adventure_details_e.count));
		update_values.push_back(columns[4] + " = " + std::to_string(adventure_details_e.assassinate_count));
		update_values.push_back(columns[5] + " = " + std::to_string(adventure_details_e.status));
		update_values.push_back(columns[6] + " = " + std::to_string(adventure_details_e.time_created));
		update_values.push_back(columns[7] + " = " + std::to_string(adventure_details_e.time_zoned));
		update_values.push_back(columns[8] + " = " + std::to_string(adventure_details_e.time_completed));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				adventure_details_e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static AdventureDetails InsertOne(
		Database& db,
		AdventureDetails adventure_details_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(adventure_details_e.id));
		insert_values.push_back(std::to_string(adventure_details_e.adventure_id));
		insert_values.push_back(std::to_string(adventure_details_e.instance_id));
		insert_values.push_back(std::to_string(adventure_details_e.count));
		insert_values.push_back(std::to_string(adventure_details_e.assassinate_count));
		insert_values.push_back(std::to_string(adventure_details_e.status));
		insert_values.push_back(std::to_string(adventure_details_e.time_created));
		insert_values.push_back(std::to_string(adventure_details_e.time_zoned));
		insert_values.push_back(std::to_string(adventure_details_e.time_completed));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			adventure_details_e.id = results.LastInsertedID();
			return adventure_details_e;
		}

		adventure_details_e = NewEntity();

		return adventure_details_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<AdventureDetails> adventure_details_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &adventure_details_e: adventure_details_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(adventure_details_e.id));
			insert_values.push_back(std::to_string(adventure_details_e.adventure_id));
			insert_values.push_back(std::to_string(adventure_details_e.instance_id));
			insert_values.push_back(std::to_string(adventure_details_e.count));
			insert_values.push_back(std::to_string(adventure_details_e.assassinate_count));
			insert_values.push_back(std::to_string(adventure_details_e.status));
			insert_values.push_back(std::to_string(adventure_details_e.time_created));
			insert_values.push_back(std::to_string(adventure_details_e.time_zoned));
			insert_values.push_back(std::to_string(adventure_details_e.time_completed));

			insert_chunks.push_back("(" + Strings::Implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
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
			AdventureDetails e{};

			e.id                = atoi(row[0]);
			e.adventure_id      = atoi(row[1]);
			e.instance_id       = atoi(row[2]);
			e.count             = atoi(row[3]);
			e.assassinate_count = atoi(row[4]);
			e.status            = atoi(row[5]);
			e.time_created      = atoi(row[6]);
			e.time_zoned        = atoi(row[7]);
			e.time_completed    = atoi(row[8]);

			all_entries.push_back(e);
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
			AdventureDetails e{};

			e.id                = atoi(row[0]);
			e.adventure_id      = atoi(row[1]);
			e.instance_id       = atoi(row[2]);
			e.count             = atoi(row[3]);
			e.assassinate_count = atoi(row[4]);
			e.status            = atoi(row[5]);
			e.time_created      = atoi(row[6]);
			e.time_zoned        = atoi(row[7]);
			e.time_completed    = atoi(row[8]);

			all_entries.push_back(e);
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

	static int64 GetMaxId(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT MAX({}) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string& where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

};

#endif //EQEMU_BASE_ADVENTURE_DETAILS_REPOSITORY_H
