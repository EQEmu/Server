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

#ifndef EQEMU_BASE_AA_ABILITY_REPOSITORY_H
#define EQEMU_BASE_AA_ABILITY_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseAaAbilityRepository {
public:
	struct AaAbility {
		uint32_t    id;
		std::string name;
		int32_t     category;
		int32_t     classes;
		int32_t     races;
		int32_t     drakkin_heritage;
		int32_t     deities;
		int32_t     status;
		int32_t     type;
		int32_t     charges;
		int8_t      grant_only;
		int32_t     first_rank_id;
		uint8_t     enabled;
		int8_t      reset_on_death;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"name",
			"category",
			"classes",
			"races",
			"drakkin_heritage",
			"deities",
			"status",
			"type",
			"charges",
			"grant_only",
			"first_rank_id",
			"enabled",
			"reset_on_death",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"name",
			"category",
			"classes",
			"races",
			"drakkin_heritage",
			"deities",
			"status",
			"type",
			"charges",
			"grant_only",
			"first_rank_id",
			"enabled",
			"reset_on_death",
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
		return std::string("aa_ability");
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

	static AaAbility NewEntity()
	{
		AaAbility e{};

		e.id               = 0;
		e.name             = "";
		e.category         = -1;
		e.classes          = 131070;
		e.races            = 65535;
		e.drakkin_heritage = 127;
		e.deities          = 131071;
		e.status           = 0;
		e.type             = 0;
		e.charges          = 0;
		e.grant_only       = 0;
		e.first_rank_id    = -1;
		e.enabled          = 1;
		e.reset_on_death   = 0;

		return e;
	}

	static AaAbility GetAaAbility(
		const std::vector<AaAbility> &aa_abilitys,
		int aa_ability_id
	)
	{
		for (auto &aa_ability : aa_abilitys) {
			if (aa_ability.id == aa_ability_id) {
				return aa_ability;
			}
		}

		return NewEntity();
	}

	static AaAbility FindOne(
		Database& db,
		int aa_ability_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				aa_ability_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AaAbility e{};

			e.id               = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.name             = row[1] ? row[1] : "";
			e.category         = static_cast<int32_t>(atoi(row[2]));
			e.classes          = static_cast<int32_t>(atoi(row[3]));
			e.races            = static_cast<int32_t>(atoi(row[4]));
			e.drakkin_heritage = static_cast<int32_t>(atoi(row[5]));
			e.deities          = static_cast<int32_t>(atoi(row[6]));
			e.status           = static_cast<int32_t>(atoi(row[7]));
			e.type             = static_cast<int32_t>(atoi(row[8]));
			e.charges          = static_cast<int32_t>(atoi(row[9]));
			e.grant_only       = static_cast<int8_t>(atoi(row[10]));
			e.first_rank_id    = static_cast<int32_t>(atoi(row[11]));
			e.enabled          = static_cast<uint8_t>(strtoul(row[12], nullptr, 10));
			e.reset_on_death   = static_cast<int8_t>(atoi(row[13]));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int aa_ability_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				aa_ability_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const AaAbility &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.id));
		v.push_back(columns[1] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.category));
		v.push_back(columns[3] + " = " + std::to_string(e.classes));
		v.push_back(columns[4] + " = " + std::to_string(e.races));
		v.push_back(columns[5] + " = " + std::to_string(e.drakkin_heritage));
		v.push_back(columns[6] + " = " + std::to_string(e.deities));
		v.push_back(columns[7] + " = " + std::to_string(e.status));
		v.push_back(columns[8] + " = " + std::to_string(e.type));
		v.push_back(columns[9] + " = " + std::to_string(e.charges));
		v.push_back(columns[10] + " = " + std::to_string(e.grant_only));
		v.push_back(columns[11] + " = " + std::to_string(e.first_rank_id));
		v.push_back(columns[12] + " = " + std::to_string(e.enabled));
		v.push_back(columns[13] + " = " + std::to_string(e.reset_on_death));

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

	static AaAbility InsertOne(
		Database& db,
		AaAbility e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.category));
		v.push_back(std::to_string(e.classes));
		v.push_back(std::to_string(e.races));
		v.push_back(std::to_string(e.drakkin_heritage));
		v.push_back(std::to_string(e.deities));
		v.push_back(std::to_string(e.status));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.charges));
		v.push_back(std::to_string(e.grant_only));
		v.push_back(std::to_string(e.first_rank_id));
		v.push_back(std::to_string(e.enabled));
		v.push_back(std::to_string(e.reset_on_death));

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
		const std::vector<AaAbility> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.category));
			v.push_back(std::to_string(e.classes));
			v.push_back(std::to_string(e.races));
			v.push_back(std::to_string(e.drakkin_heritage));
			v.push_back(std::to_string(e.deities));
			v.push_back(std::to_string(e.status));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.charges));
			v.push_back(std::to_string(e.grant_only));
			v.push_back(std::to_string(e.first_rank_id));
			v.push_back(std::to_string(e.enabled));
			v.push_back(std::to_string(e.reset_on_death));

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

	static std::vector<AaAbility> All(Database& db)
	{
		std::vector<AaAbility> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AaAbility e{};

			e.id               = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.name             = row[1] ? row[1] : "";
			e.category         = static_cast<int32_t>(atoi(row[2]));
			e.classes          = static_cast<int32_t>(atoi(row[3]));
			e.races            = static_cast<int32_t>(atoi(row[4]));
			e.drakkin_heritage = static_cast<int32_t>(atoi(row[5]));
			e.deities          = static_cast<int32_t>(atoi(row[6]));
			e.status           = static_cast<int32_t>(atoi(row[7]));
			e.type             = static_cast<int32_t>(atoi(row[8]));
			e.charges          = static_cast<int32_t>(atoi(row[9]));
			e.grant_only       = static_cast<int8_t>(atoi(row[10]));
			e.first_rank_id    = static_cast<int32_t>(atoi(row[11]));
			e.enabled          = static_cast<uint8_t>(strtoul(row[12], nullptr, 10));
			e.reset_on_death   = static_cast<int8_t>(atoi(row[13]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<AaAbility> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<AaAbility> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AaAbility e{};

			e.id               = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.name             = row[1] ? row[1] : "";
			e.category         = static_cast<int32_t>(atoi(row[2]));
			e.classes          = static_cast<int32_t>(atoi(row[3]));
			e.races            = static_cast<int32_t>(atoi(row[4]));
			e.drakkin_heritage = static_cast<int32_t>(atoi(row[5]));
			e.deities          = static_cast<int32_t>(atoi(row[6]));
			e.status           = static_cast<int32_t>(atoi(row[7]));
			e.type             = static_cast<int32_t>(atoi(row[8]));
			e.charges          = static_cast<int32_t>(atoi(row[9]));
			e.grant_only       = static_cast<int8_t>(atoi(row[10]));
			e.first_rank_id    = static_cast<int32_t>(atoi(row[11]));
			e.enabled          = static_cast<uint8_t>(strtoul(row[12], nullptr, 10));
			e.reset_on_death   = static_cast<int8_t>(atoi(row[13]));

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

#endif //EQEMU_BASE_AA_ABILITY_REPOSITORY_H
