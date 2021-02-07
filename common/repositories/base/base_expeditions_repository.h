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

#ifndef EQEMU_BASE_EXPEDITIONS_REPOSITORY_H
#define EQEMU_BASE_EXPEDITIONS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseExpeditionsRepository {
public:
	struct Expeditions {
		int         id;
		std::string uuid;
		int         dynamic_zone_id;
		std::string expedition_name;
		int         leader_id;
		int         min_players;
		int         max_players;
		int         add_replay_on_join;
		int         is_locked;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"uuid",
			"dynamic_zone_id",
			"expedition_name",
			"leader_id",
			"min_players",
			"max_players",
			"add_replay_on_join",
			"is_locked",
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
		return std::string("expeditions");
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

	static Expeditions NewEntity()
	{
		Expeditions entry{};

		entry.id                 = 0;
		entry.uuid               = "";
		entry.dynamic_zone_id    = 0;
		entry.expedition_name    = "";
		entry.leader_id          = 0;
		entry.min_players        = 0;
		entry.max_players        = 0;
		entry.add_replay_on_join = 1;
		entry.is_locked          = 0;

		return entry;
	}

	static Expeditions GetExpeditionsEntry(
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
			Expeditions entry{};

			entry.id                 = atoi(row[0]);
			entry.uuid               = row[1] ? row[1] : "";
			entry.dynamic_zone_id    = atoi(row[2]);
			entry.expedition_name    = row[3] ? row[3] : "";
			entry.leader_id          = atoi(row[4]);
			entry.min_players        = atoi(row[5]);
			entry.max_players        = atoi(row[6]);
			entry.add_replay_on_join = atoi(row[7]);
			entry.is_locked          = atoi(row[8]);

			return entry;
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
		Expeditions expeditions_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(expeditions_entry.uuid) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(expeditions_entry.dynamic_zone_id));
		update_values.push_back(columns[3] + " = '" + EscapeString(expeditions_entry.expedition_name) + "'");
		update_values.push_back(columns[4] + " = " + std::to_string(expeditions_entry.leader_id));
		update_values.push_back(columns[5] + " = " + std::to_string(expeditions_entry.min_players));
		update_values.push_back(columns[6] + " = " + std::to_string(expeditions_entry.max_players));
		update_values.push_back(columns[7] + " = " + std::to_string(expeditions_entry.add_replay_on_join));
		update_values.push_back(columns[8] + " = " + std::to_string(expeditions_entry.is_locked));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				expeditions_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Expeditions InsertOne(
		Database& db,
		Expeditions expeditions_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(expeditions_entry.uuid) + "'");
		insert_values.push_back(std::to_string(expeditions_entry.dynamic_zone_id));
		insert_values.push_back("'" + EscapeString(expeditions_entry.expedition_name) + "'");
		insert_values.push_back(std::to_string(expeditions_entry.leader_id));
		insert_values.push_back(std::to_string(expeditions_entry.min_players));
		insert_values.push_back(std::to_string(expeditions_entry.max_players));
		insert_values.push_back(std::to_string(expeditions_entry.add_replay_on_join));
		insert_values.push_back(std::to_string(expeditions_entry.is_locked));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			expeditions_entry.id = results.LastInsertedID();
			return expeditions_entry;
		}

		expeditions_entry = NewEntity();

		return expeditions_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Expeditions> expeditions_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &expeditions_entry: expeditions_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(expeditions_entry.uuid) + "'");
			insert_values.push_back(std::to_string(expeditions_entry.dynamic_zone_id));
			insert_values.push_back("'" + EscapeString(expeditions_entry.expedition_name) + "'");
			insert_values.push_back(std::to_string(expeditions_entry.leader_id));
			insert_values.push_back(std::to_string(expeditions_entry.min_players));
			insert_values.push_back(std::to_string(expeditions_entry.max_players));
			insert_values.push_back(std::to_string(expeditions_entry.add_replay_on_join));
			insert_values.push_back(std::to_string(expeditions_entry.is_locked));

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
			Expeditions entry{};

			entry.id                 = atoi(row[0]);
			entry.uuid               = row[1] ? row[1] : "";
			entry.dynamic_zone_id    = atoi(row[2]);
			entry.expedition_name    = row[3] ? row[3] : "";
			entry.leader_id          = atoi(row[4]);
			entry.min_players        = atoi(row[5]);
			entry.max_players        = atoi(row[6]);
			entry.add_replay_on_join = atoi(row[7]);
			entry.is_locked          = atoi(row[8]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Expeditions> GetWhere(Database& db, std::string where_filter)
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
			Expeditions entry{};

			entry.id                 = atoi(row[0]);
			entry.uuid               = row[1] ? row[1] : "";
			entry.dynamic_zone_id    = atoi(row[2]);
			entry.expedition_name    = row[3] ? row[3] : "";
			entry.leader_id          = atoi(row[4]);
			entry.min_players        = atoi(row[5]);
			entry.max_players        = atoi(row[6]);
			entry.add_replay_on_join = atoi(row[7]);
			entry.is_locked          = atoi(row[8]);

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

#endif //EQEMU_BASE_EXPEDITIONS_REPOSITORY_H
