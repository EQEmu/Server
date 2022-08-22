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
#include "../../strings.h"
#include <ctime>

class BaseSkillCapsRepository {
public:
	struct SkillCaps {
		uint8_t  skillID;
		uint8_t  class_;
		uint8_t  level;
		uint32_t cap;
		uint8_t  class_;
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

	static std::vector<std::string> SelectColumns()
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
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("skill_caps");
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

	static SkillCaps NewEntity()
	{
		SkillCaps e{};

		e.skillID = 0;
		e.class_  = 0;
		e.level   = 0;
		e.cap     = 0;
		e.class_  = 0;

		return e;
	}

	static SkillCaps GetSkillCaps(
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
			SkillCaps e{};

			e.skillID = static_cast<uint8_t>(strtoul(row[0], nullptr, 10));
			e.class_  = static_cast<uint8_t>(strtoul(row[1], nullptr, 10));
			e.level   = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.cap     = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.class_  = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));

			return e;
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
		const SkillCaps &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.skillID));
		v.push_back(columns[1] + " = " + std::to_string(e.class_));
		v.push_back(columns[2] + " = " + std::to_string(e.level));
		v.push_back(columns[3] + " = " + std::to_string(e.cap));
		v.push_back(columns[4] + " = " + std::to_string(e.class_));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.skillID
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static SkillCaps InsertOne(
		Database& db,
		SkillCaps e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.skillID));
		v.push_back(std::to_string(e.class_));
		v.push_back(std::to_string(e.level));
		v.push_back(std::to_string(e.cap));
		v.push_back(std::to_string(e.class_));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.skillID = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<SkillCaps> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.skillID));
			v.push_back(std::to_string(e.class_));
			v.push_back(std::to_string(e.level));
			v.push_back(std::to_string(e.cap));
			v.push_back(std::to_string(e.class_));

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
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
			SkillCaps e{};

			e.skillID = static_cast<uint8_t>(strtoul(row[0], nullptr, 10));
			e.class_  = static_cast<uint8_t>(strtoul(row[1], nullptr, 10));
			e.level   = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.cap     = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.class_  = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<SkillCaps> GetWhere(Database& db, const std::string &where_filter)
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
			SkillCaps e{};

			e.skillID = static_cast<uint8_t>(strtoul(row[0], nullptr, 10));
			e.class_  = static_cast<uint8_t>(strtoul(row[1], nullptr, 10));
			e.level   = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.cap     = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.class_  = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static int DeleteWhere(Database& db, const std::string &where_filter)
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
				"SELECT COALESCE(MAX({}), 0) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string &where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

};

#endif //EQEMU_BASE_SKILL_CAPS_REPOSITORY_H
