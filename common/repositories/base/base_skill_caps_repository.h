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

#ifndef EQEMU_BASE_SKILL_CAPS_REPOSITORY_H
#define EQEMU_BASE_SKILL_CAPS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseSkillCapsRepository {
public:
	struct SkillCaps {
		int skillID;
		int class;
		int level;
		int cap;
		int class_;
	};

	static std::string PrimaryKey()
	{
		return std::string("skillID");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"skillID",
			"class",
			"level",
			"cap",
			"class_",
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
		return std::string("skill_caps");
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

	static SkillCaps NewEntity()
	{
		SkillCaps entry{};

		entry.skillID = 0;
		entry.class   = 0;
		entry.level   = 0;
		entry.cap     = 0;
		entry.class_  = 0;

		return entry;
	}

	static SkillCaps GetSkillCapsEntry(
		const std::vector<SkillCaps> &skill_capss,
		int skill_caps_id
	)
	{
		for (auto &skill_caps : skill_capss) {
			if (skill_caps.skillID == skill_caps_id) {
				return skill_caps;
			}
		}

		return NewEntity();
	}

	static SkillCaps FindOne(
		int skill_caps_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				skill_caps_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			SkillCaps entry{};

			entry.skillID = atoi(row[0]);
			entry.class   = atoi(row[1]);
			entry.level   = atoi(row[2]);
			entry.cap     = atoi(row[3]);
			entry.class_  = atoi(row[4]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int skill_caps_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				skill_caps_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		SkillCaps skill_caps_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(skill_caps_entry.skillID));
		update_values.push_back(columns[1] + " = " + std::to_string(skill_caps_entry.class));
		update_values.push_back(columns[2] + " = " + std::to_string(skill_caps_entry.level));
		update_values.push_back(columns[3] + " = " + std::to_string(skill_caps_entry.cap));
		update_values.push_back(columns[4] + " = " + std::to_string(skill_caps_entry.class_));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				skill_caps_entry.skillID
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static SkillCaps InsertOne(
		SkillCaps skill_caps_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(skill_caps_entry.skillID));
		insert_values.push_back(std::to_string(skill_caps_entry.class));
		insert_values.push_back(std::to_string(skill_caps_entry.level));
		insert_values.push_back(std::to_string(skill_caps_entry.cap));
		insert_values.push_back(std::to_string(skill_caps_entry.class_));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			skill_caps_entry.skillID = results.LastInsertedID();
			return skill_caps_entry;
		}

		skill_caps_entry = NewEntity();

		return skill_caps_entry;
	}

	static int InsertMany(
		std::vector<SkillCaps> skill_caps_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &skill_caps_entry: skill_caps_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(skill_caps_entry.skillID));
			insert_values.push_back(std::to_string(skill_caps_entry.class));
			insert_values.push_back(std::to_string(skill_caps_entry.level));
			insert_values.push_back(std::to_string(skill_caps_entry.cap));
			insert_values.push_back(std::to_string(skill_caps_entry.class_));

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<SkillCaps> All()
	{
		std::vector<SkillCaps> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SkillCaps entry{};

			entry.skillID = atoi(row[0]);
			entry.class   = atoi(row[1]);
			entry.level   = atoi(row[2]);
			entry.cap     = atoi(row[3]);
			entry.class_  = atoi(row[4]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<SkillCaps> GetWhere(std::string where_filter)
	{
		std::vector<SkillCaps> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SkillCaps entry{};

			entry.skillID = atoi(row[0]);
			entry.class   = atoi(row[1]);
			entry.level   = atoi(row[2]);
			entry.cap     = atoi(row[3]);
			entry.class_  = atoi(row[4]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static int DeleteWhere(std::string where_filter)
	{
		auto results = content_db.QueryDatabase(
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
		auto results = content_db.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_SKILL_CAPS_REPOSITORY_H
