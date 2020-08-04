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

#ifndef EQEMU_BASE_EVENTLOG_REPOSITORY_H
#define EQEMU_BASE_EVENTLOG_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseEventlogRepository {
public:
	struct Eventlog {
		int         id;
		std::string accountname;
		int         accountid;
		int         status;
		std::string charname;
		std::string target;
		std::string time;
		std::string descriptiontype;
		std::string description;
		int         event_nid;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"accountname",
			"accountid",
			"status",
			"charname",
			"target",
			"time",
			"descriptiontype",
			"description",
			"event_nid",
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
		return std::string("eventlog");
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

	static Eventlog NewEntity()
	{
		Eventlog entry{};

		entry.id              = 0;
		entry.accountname     = "";
		entry.accountid       = 0;
		entry.status          = 0;
		entry.charname        = "";
		entry.target          = "None";
		entry.time            = current_timestamp();
		entry.descriptiontype = "";
		entry.description     = "";
		entry.event_nid       = 0;

		return entry;
	}

	static Eventlog GetEventlogEntry(
		const std::vector<Eventlog> &eventlogs,
		int eventlog_id
	)
	{
		for (auto &eventlog : eventlogs) {
			if (eventlog.id == eventlog_id) {
				return eventlog;
			}
		}

		return NewEntity();
	}

	static Eventlog FindOne(
		int eventlog_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				eventlog_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Eventlog entry{};

			entry.id              = atoi(row[0]);
			entry.accountname     = row[1] ? row[1] : "";
			entry.accountid       = atoi(row[2]);
			entry.status          = atoi(row[3]);
			entry.charname        = row[4] ? row[4] : "";
			entry.target          = row[5] ? row[5] : "";
			entry.time            = row[6] ? row[6] : "";
			entry.descriptiontype = row[7] ? row[7] : "";
			entry.description     = row[8] ? row[8] : "";
			entry.event_nid       = atoi(row[9]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int eventlog_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				eventlog_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Eventlog eventlog_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(eventlog_entry.accountname) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(eventlog_entry.accountid));
		update_values.push_back(columns[3] + " = " + std::to_string(eventlog_entry.status));
		update_values.push_back(columns[4] + " = '" + EscapeString(eventlog_entry.charname) + "'");
		update_values.push_back(columns[5] + " = '" + EscapeString(eventlog_entry.target) + "'");
		update_values.push_back(columns[6] + " = '" + EscapeString(eventlog_entry.time) + "'");
		update_values.push_back(columns[7] + " = '" + EscapeString(eventlog_entry.descriptiontype) + "'");
		update_values.push_back(columns[8] + " = '" + EscapeString(eventlog_entry.description) + "'");
		update_values.push_back(columns[9] + " = " + std::to_string(eventlog_entry.event_nid));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				eventlog_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Eventlog InsertOne(
		Eventlog eventlog_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(eventlog_entry.accountname) + "'");
		insert_values.push_back(std::to_string(eventlog_entry.accountid));
		insert_values.push_back(std::to_string(eventlog_entry.status));
		insert_values.push_back("'" + EscapeString(eventlog_entry.charname) + "'");
		insert_values.push_back("'" + EscapeString(eventlog_entry.target) + "'");
		insert_values.push_back("'" + EscapeString(eventlog_entry.time) + "'");
		insert_values.push_back("'" + EscapeString(eventlog_entry.descriptiontype) + "'");
		insert_values.push_back("'" + EscapeString(eventlog_entry.description) + "'");
		insert_values.push_back(std::to_string(eventlog_entry.event_nid));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			eventlog_entry.id = results.LastInsertedID();
			return eventlog_entry;
		}

		eventlog_entry = NewEntity();

		return eventlog_entry;
	}

	static int InsertMany(
		std::vector<Eventlog> eventlog_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &eventlog_entry: eventlog_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(eventlog_entry.accountname) + "'");
			insert_values.push_back(std::to_string(eventlog_entry.accountid));
			insert_values.push_back(std::to_string(eventlog_entry.status));
			insert_values.push_back("'" + EscapeString(eventlog_entry.charname) + "'");
			insert_values.push_back("'" + EscapeString(eventlog_entry.target) + "'");
			insert_values.push_back("'" + EscapeString(eventlog_entry.time) + "'");
			insert_values.push_back("'" + EscapeString(eventlog_entry.descriptiontype) + "'");
			insert_values.push_back("'" + EscapeString(eventlog_entry.description) + "'");
			insert_values.push_back(std::to_string(eventlog_entry.event_nid));

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

	static std::vector<Eventlog> All()
	{
		std::vector<Eventlog> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Eventlog entry{};

			entry.id              = atoi(row[0]);
			entry.accountname     = row[1] ? row[1] : "";
			entry.accountid       = atoi(row[2]);
			entry.status          = atoi(row[3]);
			entry.charname        = row[4] ? row[4] : "";
			entry.target          = row[5] ? row[5] : "";
			entry.time            = row[6] ? row[6] : "";
			entry.descriptiontype = row[7] ? row[7] : "";
			entry.description     = row[8] ? row[8] : "";
			entry.event_nid       = atoi(row[9]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Eventlog> GetWhere(std::string where_filter)
	{
		std::vector<Eventlog> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Eventlog entry{};

			entry.id              = atoi(row[0]);
			entry.accountname     = row[1] ? row[1] : "";
			entry.accountid       = atoi(row[2]);
			entry.status          = atoi(row[3]);
			entry.charname        = row[4] ? row[4] : "";
			entry.target          = row[5] ? row[5] : "";
			entry.time            = row[6] ? row[6] : "";
			entry.descriptiontype = row[7] ? row[7] : "";
			entry.description     = row[8] ? row[8] : "";
			entry.event_nid       = atoi(row[9]);

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

#endif //EQEMU_BASE_EVENTLOG_REPOSITORY_H
