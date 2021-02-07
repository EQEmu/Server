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

#ifndef EQEMU_BASE_OBJECT_CONTENTS_REPOSITORY_H
#define EQEMU_BASE_OBJECT_CONTENTS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseObjectContentsRepository {
public:
	struct ObjectContents {
		int         zoneid;
		int         parentid;
		int         bagidx;
		int         itemid;
		int         charges;
		std::string droptime;
		int         augslot1;
		int         augslot2;
		int         augslot3;
		int         augslot4;
		int         augslot5;
		int         augslot6;
	};

	static std::string PrimaryKey()
	{
		return std::string("parentid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"zoneid",
			"parentid",
			"bagidx",
			"itemid",
			"charges",
			"droptime",
			"augslot1",
			"augslot2",
			"augslot3",
			"augslot4",
			"augslot5",
			"augslot6",
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
		return std::string("object_contents");
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

	static ObjectContents NewEntity()
	{
		ObjectContents entry{};

		entry.zoneid   = 0;
		entry.parentid = 0;
		entry.bagidx   = 0;
		entry.itemid   = 0;
		entry.charges  = 0;
		entry.droptime = "0000-00-00 00:00:00";
		entry.augslot1 = 0;
		entry.augslot2 = 0;
		entry.augslot3 = 0;
		entry.augslot4 = 0;
		entry.augslot5 = 0;
		entry.augslot6 = 0;

		return entry;
	}

	static ObjectContents GetObjectContentsEntry(
		const std::vector<ObjectContents> &object_contentss,
		int object_contents_id
	)
	{
		for (auto &object_contents : object_contentss) {
			if (object_contents.parentid == object_contents_id) {
				return object_contents;
			}
		}

		return NewEntity();
	}

	static ObjectContents FindOne(
		Database& db,
		int object_contents_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				object_contents_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			ObjectContents entry{};

			entry.zoneid   = atoi(row[0]);
			entry.parentid = atoi(row[1]);
			entry.bagidx   = atoi(row[2]);
			entry.itemid   = atoi(row[3]);
			entry.charges  = atoi(row[4]);
			entry.droptime = row[5] ? row[5] : "";
			entry.augslot1 = atoi(row[6]);
			entry.augslot2 = atoi(row[7]);
			entry.augslot3 = atoi(row[8]);
			entry.augslot4 = atoi(row[9]);
			entry.augslot5 = atoi(row[10]);
			entry.augslot6 = atoi(row[11]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int object_contents_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				object_contents_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		ObjectContents object_contents_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(object_contents_entry.zoneid));
		update_values.push_back(columns[1] + " = " + std::to_string(object_contents_entry.parentid));
		update_values.push_back(columns[2] + " = " + std::to_string(object_contents_entry.bagidx));
		update_values.push_back(columns[3] + " = " + std::to_string(object_contents_entry.itemid));
		update_values.push_back(columns[4] + " = " + std::to_string(object_contents_entry.charges));
		update_values.push_back(columns[5] + " = '" + EscapeString(object_contents_entry.droptime) + "'");
		update_values.push_back(columns[6] + " = " + std::to_string(object_contents_entry.augslot1));
		update_values.push_back(columns[7] + " = " + std::to_string(object_contents_entry.augslot2));
		update_values.push_back(columns[8] + " = " + std::to_string(object_contents_entry.augslot3));
		update_values.push_back(columns[9] + " = " + std::to_string(object_contents_entry.augslot4));
		update_values.push_back(columns[10] + " = " + std::to_string(object_contents_entry.augslot5));
		update_values.push_back(columns[11] + " = " + std::to_string(object_contents_entry.augslot6));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				object_contents_entry.parentid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static ObjectContents InsertOne(
		Database& db,
		ObjectContents object_contents_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(object_contents_entry.zoneid));
		insert_values.push_back(std::to_string(object_contents_entry.parentid));
		insert_values.push_back(std::to_string(object_contents_entry.bagidx));
		insert_values.push_back(std::to_string(object_contents_entry.itemid));
		insert_values.push_back(std::to_string(object_contents_entry.charges));
		insert_values.push_back("'" + EscapeString(object_contents_entry.droptime) + "'");
		insert_values.push_back(std::to_string(object_contents_entry.augslot1));
		insert_values.push_back(std::to_string(object_contents_entry.augslot2));
		insert_values.push_back(std::to_string(object_contents_entry.augslot3));
		insert_values.push_back(std::to_string(object_contents_entry.augslot4));
		insert_values.push_back(std::to_string(object_contents_entry.augslot5));
		insert_values.push_back(std::to_string(object_contents_entry.augslot6));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			object_contents_entry.parentid = results.LastInsertedID();
			return object_contents_entry;
		}

		object_contents_entry = NewEntity();

		return object_contents_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<ObjectContents> object_contents_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &object_contents_entry: object_contents_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(object_contents_entry.zoneid));
			insert_values.push_back(std::to_string(object_contents_entry.parentid));
			insert_values.push_back(std::to_string(object_contents_entry.bagidx));
			insert_values.push_back(std::to_string(object_contents_entry.itemid));
			insert_values.push_back(std::to_string(object_contents_entry.charges));
			insert_values.push_back("'" + EscapeString(object_contents_entry.droptime) + "'");
			insert_values.push_back(std::to_string(object_contents_entry.augslot1));
			insert_values.push_back(std::to_string(object_contents_entry.augslot2));
			insert_values.push_back(std::to_string(object_contents_entry.augslot3));
			insert_values.push_back(std::to_string(object_contents_entry.augslot4));
			insert_values.push_back(std::to_string(object_contents_entry.augslot5));
			insert_values.push_back(std::to_string(object_contents_entry.augslot6));

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

	static std::vector<ObjectContents> All(Database& db)
	{
		std::vector<ObjectContents> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ObjectContents entry{};

			entry.zoneid   = atoi(row[0]);
			entry.parentid = atoi(row[1]);
			entry.bagidx   = atoi(row[2]);
			entry.itemid   = atoi(row[3]);
			entry.charges  = atoi(row[4]);
			entry.droptime = row[5] ? row[5] : "";
			entry.augslot1 = atoi(row[6]);
			entry.augslot2 = atoi(row[7]);
			entry.augslot3 = atoi(row[8]);
			entry.augslot4 = atoi(row[9]);
			entry.augslot5 = atoi(row[10]);
			entry.augslot6 = atoi(row[11]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<ObjectContents> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<ObjectContents> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ObjectContents entry{};

			entry.zoneid   = atoi(row[0]);
			entry.parentid = atoi(row[1]);
			entry.bagidx   = atoi(row[2]);
			entry.itemid   = atoi(row[3]);
			entry.charges  = atoi(row[4]);
			entry.droptime = row[5] ? row[5] : "";
			entry.augslot1 = atoi(row[6]);
			entry.augslot2 = atoi(row[7]);
			entry.augslot3 = atoi(row[8]);
			entry.augslot4 = atoi(row[9]);
			entry.augslot5 = atoi(row[10]);
			entry.augslot6 = atoi(row[11]);

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

#endif //EQEMU_BASE_OBJECT_CONTENTS_REPOSITORY_H
