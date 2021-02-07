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

#ifndef EQEMU_BASE_VETERAN_REWARD_TEMPLATES_REPOSITORY_H
#define EQEMU_BASE_VETERAN_REWARD_TEMPLATES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseVeteranRewardTemplatesRepository {
public:
	struct VeteranRewardTemplates {
		int         claim_id;
		std::string name;
		int         item_id;
		int         charges;
		int         reward_slot;
	};

	static std::string PrimaryKey()
	{
		return std::string("claim_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"claim_id",
			"name",
			"item_id",
			"charges",
			"reward_slot",
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
		return std::string("veteran_reward_templates");
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

	static VeteranRewardTemplates NewEntity()
	{
		VeteranRewardTemplates entry{};

		entry.claim_id    = 0;
		entry.name        = "";
		entry.item_id     = 0;
		entry.charges     = 0;
		entry.reward_slot = 0;

		return entry;
	}

	static VeteranRewardTemplates GetVeteranRewardTemplatesEntry(
		const std::vector<VeteranRewardTemplates> &veteran_reward_templatess,
		int veteran_reward_templates_id
	)
	{
		for (auto &veteran_reward_templates : veteran_reward_templatess) {
			if (veteran_reward_templates.claim_id == veteran_reward_templates_id) {
				return veteran_reward_templates;
			}
		}

		return NewEntity();
	}

	static VeteranRewardTemplates FindOne(
		Database& db,
		int veteran_reward_templates_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				veteran_reward_templates_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			VeteranRewardTemplates entry{};

			entry.claim_id    = atoi(row[0]);
			entry.name        = row[1] ? row[1] : "";
			entry.item_id     = atoi(row[2]);
			entry.charges     = atoi(row[3]);
			entry.reward_slot = atoi(row[4]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int veteran_reward_templates_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				veteran_reward_templates_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		VeteranRewardTemplates veteran_reward_templates_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(veteran_reward_templates_entry.claim_id));
		update_values.push_back(columns[1] + " = '" + EscapeString(veteran_reward_templates_entry.name) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(veteran_reward_templates_entry.item_id));
		update_values.push_back(columns[3] + " = " + std::to_string(veteran_reward_templates_entry.charges));
		update_values.push_back(columns[4] + " = " + std::to_string(veteran_reward_templates_entry.reward_slot));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				veteran_reward_templates_entry.claim_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static VeteranRewardTemplates InsertOne(
		Database& db,
		VeteranRewardTemplates veteran_reward_templates_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(veteran_reward_templates_entry.claim_id));
		insert_values.push_back("'" + EscapeString(veteran_reward_templates_entry.name) + "'");
		insert_values.push_back(std::to_string(veteran_reward_templates_entry.item_id));
		insert_values.push_back(std::to_string(veteran_reward_templates_entry.charges));
		insert_values.push_back(std::to_string(veteran_reward_templates_entry.reward_slot));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			veteran_reward_templates_entry.claim_id = results.LastInsertedID();
			return veteran_reward_templates_entry;
		}

		veteran_reward_templates_entry = NewEntity();

		return veteran_reward_templates_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<VeteranRewardTemplates> veteran_reward_templates_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &veteran_reward_templates_entry: veteran_reward_templates_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(veteran_reward_templates_entry.claim_id));
			insert_values.push_back("'" + EscapeString(veteran_reward_templates_entry.name) + "'");
			insert_values.push_back(std::to_string(veteran_reward_templates_entry.item_id));
			insert_values.push_back(std::to_string(veteran_reward_templates_entry.charges));
			insert_values.push_back(std::to_string(veteran_reward_templates_entry.reward_slot));

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

	static std::vector<VeteranRewardTemplates> All(Database& db)
	{
		std::vector<VeteranRewardTemplates> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			VeteranRewardTemplates entry{};

			entry.claim_id    = atoi(row[0]);
			entry.name        = row[1] ? row[1] : "";
			entry.item_id     = atoi(row[2]);
			entry.charges     = atoi(row[3]);
			entry.reward_slot = atoi(row[4]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<VeteranRewardTemplates> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<VeteranRewardTemplates> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			VeteranRewardTemplates entry{};

			entry.claim_id    = atoi(row[0]);
			entry.name        = row[1] ? row[1] : "";
			entry.item_id     = atoi(row[2]);
			entry.charges     = atoi(row[3]);
			entry.reward_slot = atoi(row[4]);

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

#endif //EQEMU_BASE_VETERAN_REWARD_TEMPLATES_REPOSITORY_H
