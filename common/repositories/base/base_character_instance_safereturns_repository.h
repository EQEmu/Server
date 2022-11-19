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

#ifndef EQEMU_BASE_CHARACTER_INSTANCE_SAFERETURNS_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_INSTANCE_SAFERETURNS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseCharacterInstanceSafereturnsRepository {
public:
	struct CharacterInstanceSafereturns {
		uint32_t id;
		uint32_t character_id;
		int32_t  instance_zone_id;
		int32_t  instance_id;
		int32_t  safe_zone_id;
		float    safe_x;
		float    safe_y;
		float    safe_z;
		float    safe_heading;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"character_id",
			"instance_zone_id",
			"instance_id",
			"safe_zone_id",
			"safe_x",
			"safe_y",
			"safe_z",
			"safe_heading",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"character_id",
			"instance_zone_id",
			"instance_id",
			"safe_zone_id",
			"safe_x",
			"safe_y",
			"safe_z",
			"safe_heading",
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
		return std::string("character_instance_safereturns");
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

	static CharacterInstanceSafereturns NewEntity()
	{
		CharacterInstanceSafereturns e{};

		e.id               = 0;
		e.character_id     = 0;
		e.instance_zone_id = 0;
		e.instance_id      = 0;
		e.safe_zone_id     = 0;
		e.safe_x           = 0;
		e.safe_y           = 0;
		e.safe_z           = 0;
		e.safe_heading     = 0;

		return e;
	}

	static CharacterInstanceSafereturns GetCharacterInstanceSafereturns(
		const std::vector<CharacterInstanceSafereturns> &character_instance_safereturnss,
		int character_instance_safereturns_id
	)
	{
		for (auto &character_instance_safereturns : character_instance_safereturnss) {
			if (character_instance_safereturns.id == character_instance_safereturns_id) {
				return character_instance_safereturns;
			}
		}

		return NewEntity();
	}

	static CharacterInstanceSafereturns FindOne(
		Database& db,
		int character_instance_safereturns_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_instance_safereturns_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterInstanceSafereturns e{};

			e.id               = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.character_id     = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.instance_zone_id = static_cast<int32_t>(atoi(row[2]));
			e.instance_id      = static_cast<int32_t>(atoi(row[3]));
			e.safe_zone_id     = static_cast<int32_t>(atoi(row[4]));
			e.safe_x           = strtof(row[5], nullptr);
			e.safe_y           = strtof(row[6], nullptr);
			e.safe_z           = strtof(row[7], nullptr);
			e.safe_heading     = strtof(row[8], nullptr);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_instance_safereturns_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_instance_safereturns_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterInstanceSafereturns &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.character_id));
		v.push_back(columns[2] + " = " + std::to_string(e.instance_zone_id));
		v.push_back(columns[3] + " = " + std::to_string(e.instance_id));
		v.push_back(columns[4] + " = " + std::to_string(e.safe_zone_id));
		v.push_back(columns[5] + " = " + std::to_string(e.safe_x));
		v.push_back(columns[6] + " = " + std::to_string(e.safe_y));
		v.push_back(columns[7] + " = " + std::to_string(e.safe_z));
		v.push_back(columns[8] + " = " + std::to_string(e.safe_heading));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterInstanceSafereturns InsertOne(
		Database& db,
		CharacterInstanceSafereturns e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.instance_zone_id));
		v.push_back(std::to_string(e.instance_id));
		v.push_back(std::to_string(e.safe_zone_id));
		v.push_back(std::to_string(e.safe_x));
		v.push_back(std::to_string(e.safe_y));
		v.push_back(std::to_string(e.safe_z));
		v.push_back(std::to_string(e.safe_heading));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<CharacterInstanceSafereturns> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.instance_zone_id));
			v.push_back(std::to_string(e.instance_id));
			v.push_back(std::to_string(e.safe_zone_id));
			v.push_back(std::to_string(e.safe_x));
			v.push_back(std::to_string(e.safe_y));
			v.push_back(std::to_string(e.safe_z));
			v.push_back(std::to_string(e.safe_heading));

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

	static std::vector<CharacterInstanceSafereturns> All(Database& db)
	{
		std::vector<CharacterInstanceSafereturns> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterInstanceSafereturns e{};

			e.id               = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.character_id     = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.instance_zone_id = static_cast<int32_t>(atoi(row[2]));
			e.instance_id      = static_cast<int32_t>(atoi(row[3]));
			e.safe_zone_id     = static_cast<int32_t>(atoi(row[4]));
			e.safe_x           = strtof(row[5], nullptr);
			e.safe_y           = strtof(row[6], nullptr);
			e.safe_z           = strtof(row[7], nullptr);
			e.safe_heading     = strtof(row[8], nullptr);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterInstanceSafereturns> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterInstanceSafereturns> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterInstanceSafereturns e{};

			e.id               = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.character_id     = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.instance_zone_id = static_cast<int32_t>(atoi(row[2]));
			e.instance_id      = static_cast<int32_t>(atoi(row[3]));
			e.safe_zone_id     = static_cast<int32_t>(atoi(row[4]));
			e.safe_x           = strtof(row[5], nullptr);
			e.safe_y           = strtof(row[6], nullptr);
			e.safe_z           = strtof(row[7], nullptr);
			e.safe_heading     = strtof(row[8], nullptr);

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

#endif //EQEMU_BASE_CHARACTER_INSTANCE_SAFERETURNS_REPOSITORY_H
