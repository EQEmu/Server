/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *
 */

/**
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to
 * the repository extending the base. Any modifications to base repositories are to
 * be made by the generator only
 */

#ifndef EQEMU_BASE_DATA_BUCKETS_REPOSITORY_H
#define EQEMU_BASE_DATA_BUCKETS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseDataBucketsRepository {
public:
	struct DataBuckets {
		int         id;
		std::string key;
		std::string value;
		int         expires;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"key",
			"value",
			"expires",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string InsertColumnsRaw()
	{
		std::vector<std::string> insert_columns;

		for (auto &column : Columns()) {
			if (column == PrimaryKey()) {
				continue;
			}

			insert_columns.push_back(column);
		}

		return std::string(implode(", ", insert_columns));
	}

	static std::string TableName()
	{
		return std::string("data_buckets");
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
			InsertColumnsRaw()
		);
	}

	static DataBuckets NewEntity()
	{
		DataBuckets entry{};

		entry.id      = 0;
		entry.key     = "";
		entry.value   = "";
		entry.expires = 0;

		return entry;
	}

	static DataBuckets GetDataBucketsEntry(
		const std::vector<DataBuckets> &data_bucketss,
		int data_buckets_id
	)
	{
		for (auto &data_buckets : data_bucketss) {
			if (data_buckets.id == data_buckets_id) {
				return data_buckets;
			}
		}

		return NewEntity();
	}

	static DataBuckets FindOne(
		int data_buckets_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				data_buckets_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			DataBuckets entry{};

			entry.id      = atoi(row[0]);
			entry.key     = row[1] ? row[1] : "";
			entry.value   = row[2] ? row[2] : "";
			entry.expires = atoi(row[3]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int data_buckets_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				data_buckets_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		DataBuckets data_buckets_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(data_buckets_entry.key) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(data_buckets_entry.value) + "'");
		update_values.push_back(columns[3] + " = " + std::to_string(data_buckets_entry.expires));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				data_buckets_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static DataBuckets InsertOne(
		DataBuckets data_buckets_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(data_buckets_entry.key) + "'");
		insert_values.push_back("'" + EscapeString(data_buckets_entry.value) + "'");
		insert_values.push_back(std::to_string(data_buckets_entry.expires));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			data_buckets_entry.id = results.LastInsertedID();
			return data_buckets_entry;
		}

		data_buckets_entry = NewEntity();

		return data_buckets_entry;
	}

	static int InsertMany(
		std::vector<DataBuckets> data_buckets_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &data_buckets_entry: data_buckets_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(data_buckets_entry.key) + "'");
			insert_values.push_back("'" + EscapeString(data_buckets_entry.value) + "'");
			insert_values.push_back(std::to_string(data_buckets_entry.expires));

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<DataBuckets> All()
	{
		std::vector<DataBuckets> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DataBuckets entry{};

			entry.id      = atoi(row[0]);
			entry.key     = row[1] ? row[1] : "";
			entry.value   = row[2] ? row[2] : "";
			entry.expires = atoi(row[3]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<DataBuckets> GetWhere(std::string where_filter)
	{
		std::vector<DataBuckets> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DataBuckets entry{};

			entry.id      = atoi(row[0]);
			entry.key     = row[1] ? row[1] : "";
			entry.value   = row[2] ? row[2] : "";
			entry.expires = atoi(row[3]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static int DeleteWhere(std::string where_filter)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate()
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_DATA_BUCKETS_REPOSITORY_H
