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

#ifndef EQEMU_BASE_SKILL_CAPS_REPOSITORY_H
#define EQEMU_BASE_SKILL_CAPS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseSkillCapsRepository {
public:
	struct SkillCaps {
		int skillID;
		int class_;
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
			"`class`",
			"level",
			"cap",
			"class_",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
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
			ColumnsRaw()
		);
	}

	static SkillCaps NewEntity()
	{
		SkillCaps entry{};

		entry.skillID = 0;
		entry.class_  = 0;
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
		Database& db,
		int skill_caps_id
	)
	{
		auto results = db.QueryDatabase(
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
			entry.class_  = atoi(row[1]);
			entry.level   = atoi(row[2]);
			entry.cap     = atoi(row[3]);
			entry.class_  = atoi(row[4]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int skill_caps_id
	)
	{
		auto results = db.QueryDatabase(
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
		Database& db,
		SkillCaps skill_caps_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(skill_caps_entry.skillID));
		update_values.push_back(columns[1] + " = " + std::to_string(skill_caps_entry.class_));
		update_values.push_back(columns[2] + " = " + std::to_string(skill_caps_entry.level));
		update_values.push_back(columns[3] + " = " + std::to_string(skill_caps_entry.cap));
		update_values.push_back(columns[4] + " = " + std::to_string(skill_caps_entry.class_));

		auto results = db.QueryDatabase(
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
		Database& db,
		SkillCaps skill_caps_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(skill_caps_entry.skillID));
		insert_values.push_back(std::to_string(skill_caps_entry.class_));
		insert_values.push_back(std::to_string(skill_caps_entry.level));
		insert_values.push_back(std::to_string(skill_caps_entry.cap));
		insert_values.push_back(std::to_string(skill_caps_entry.class_));

		auto results = db.QueryDatabase(
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
		Database& db,
		std::vector<SkillCaps> skill_caps_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &skill_caps_entry: skill_caps_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(skill_caps_entry.skillID));
			insert_values.push_back(std::to_string(skill_caps_entry.class_));
			insert_values.push_back(std::to_string(skill_caps_entry.level));
			insert_values.push_back(std::to_string(skill_caps_entry.cap));
			insert_values.push_back(std::to_string(skill_caps_entry.class_));

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

	static std::vector<SkillCaps> All(Database& db)
	{
		std::vector<SkillCaps> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SkillCaps entry{};

			entry.skillID = atoi(row[0]);
			entry.class_  = atoi(row[1]);
			entry.level   = atoi(row[2]);
			entry.cap     = atoi(row[3]);
			entry.class_  = atoi(row[4]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<SkillCaps> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<SkillCaps> all_entries;

		auto results = db.QueryDatabase(
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
			entry.class_  = atoi(row[1]);
			entry.level   = atoi(row[2]);
			entry.cap     = atoi(row[3]);
			entry.class_  = atoi(row[4]);

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

#endif //EQEMU_BASE_SKILL_CAPS_REPOSITORY_H
