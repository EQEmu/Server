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
#include "../../string_util.h"

class BaseAaAbilityRepository {
public:
	struct AaAbility {
		int         id;
		std::string name;
		int         category;
		int         classes;
		int         races;
		int         drakkin_heritage;
		int         deities;
		int         status;
		int         type;
		int         charges;
		int         grant_only;
		int         first_rank_id;
		int         enabled;
		int         reset_on_death;
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

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("aa_ability");
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

	static AaAbility NewEntity()
	{
		AaAbility entry{};

		entry.id               = 0;
		entry.name             = "";
		entry.category         = -1;
		entry.classes          = 131070;
		entry.races            = 65535;
		entry.drakkin_heritage = 127;
		entry.deities          = 131071;
		entry.status           = 0;
		entry.type             = 0;
		entry.charges          = 0;
		entry.grant_only       = 0;
		entry.first_rank_id    = -1;
		entry.enabled          = 1;
		entry.reset_on_death   = 0;

		return entry;
	}

	static AaAbility GetAaAbilityEntry(
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
			AaAbility entry{};

			entry.id               = atoi(row[0]);
			entry.name             = row[1] ? row[1] : "";
			entry.category         = atoi(row[2]);
			entry.classes          = atoi(row[3]);
			entry.races            = atoi(row[4]);
			entry.drakkin_heritage = atoi(row[5]);
			entry.deities          = atoi(row[6]);
			entry.status           = atoi(row[7]);
			entry.type             = atoi(row[8]);
			entry.charges          = atoi(row[9]);
			entry.grant_only       = atoi(row[10]);
			entry.first_rank_id    = atoi(row[11]);
			entry.enabled          = atoi(row[12]);
			entry.reset_on_death   = atoi(row[13]);

			return entry;
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
		AaAbility aa_ability_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(aa_ability_entry.id));
		update_values.push_back(columns[1] + " = '" + EscapeString(aa_ability_entry.name) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(aa_ability_entry.category));
		update_values.push_back(columns[3] + " = " + std::to_string(aa_ability_entry.classes));
		update_values.push_back(columns[4] + " = " + std::to_string(aa_ability_entry.races));
		update_values.push_back(columns[5] + " = " + std::to_string(aa_ability_entry.drakkin_heritage));
		update_values.push_back(columns[6] + " = " + std::to_string(aa_ability_entry.deities));
		update_values.push_back(columns[7] + " = " + std::to_string(aa_ability_entry.status));
		update_values.push_back(columns[8] + " = " + std::to_string(aa_ability_entry.type));
		update_values.push_back(columns[9] + " = " + std::to_string(aa_ability_entry.charges));
		update_values.push_back(columns[10] + " = " + std::to_string(aa_ability_entry.grant_only));
		update_values.push_back(columns[11] + " = " + std::to_string(aa_ability_entry.first_rank_id));
		update_values.push_back(columns[12] + " = " + std::to_string(aa_ability_entry.enabled));
		update_values.push_back(columns[13] + " = " + std::to_string(aa_ability_entry.reset_on_death));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				aa_ability_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static AaAbility InsertOne(
		Database& db,
		AaAbility aa_ability_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(aa_ability_entry.id));
		insert_values.push_back("'" + EscapeString(aa_ability_entry.name) + "'");
		insert_values.push_back(std::to_string(aa_ability_entry.category));
		insert_values.push_back(std::to_string(aa_ability_entry.classes));
		insert_values.push_back(std::to_string(aa_ability_entry.races));
		insert_values.push_back(std::to_string(aa_ability_entry.drakkin_heritage));
		insert_values.push_back(std::to_string(aa_ability_entry.deities));
		insert_values.push_back(std::to_string(aa_ability_entry.status));
		insert_values.push_back(std::to_string(aa_ability_entry.type));
		insert_values.push_back(std::to_string(aa_ability_entry.charges));
		insert_values.push_back(std::to_string(aa_ability_entry.grant_only));
		insert_values.push_back(std::to_string(aa_ability_entry.first_rank_id));
		insert_values.push_back(std::to_string(aa_ability_entry.enabled));
		insert_values.push_back(std::to_string(aa_ability_entry.reset_on_death));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			aa_ability_entry.id = results.LastInsertedID();
			return aa_ability_entry;
		}

		aa_ability_entry = NewEntity();

		return aa_ability_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<AaAbility> aa_ability_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &aa_ability_entry: aa_ability_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(aa_ability_entry.id));
			insert_values.push_back("'" + EscapeString(aa_ability_entry.name) + "'");
			insert_values.push_back(std::to_string(aa_ability_entry.category));
			insert_values.push_back(std::to_string(aa_ability_entry.classes));
			insert_values.push_back(std::to_string(aa_ability_entry.races));
			insert_values.push_back(std::to_string(aa_ability_entry.drakkin_heritage));
			insert_values.push_back(std::to_string(aa_ability_entry.deities));
			insert_values.push_back(std::to_string(aa_ability_entry.status));
			insert_values.push_back(std::to_string(aa_ability_entry.type));
			insert_values.push_back(std::to_string(aa_ability_entry.charges));
			insert_values.push_back(std::to_string(aa_ability_entry.grant_only));
			insert_values.push_back(std::to_string(aa_ability_entry.first_rank_id));
			insert_values.push_back(std::to_string(aa_ability_entry.enabled));
			insert_values.push_back(std::to_string(aa_ability_entry.reset_on_death));

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
			AaAbility entry{};

			entry.id               = atoi(row[0]);
			entry.name             = row[1] ? row[1] : "";
			entry.category         = atoi(row[2]);
			entry.classes          = atoi(row[3]);
			entry.races            = atoi(row[4]);
			entry.drakkin_heritage = atoi(row[5]);
			entry.deities          = atoi(row[6]);
			entry.status           = atoi(row[7]);
			entry.type             = atoi(row[8]);
			entry.charges          = atoi(row[9]);
			entry.grant_only       = atoi(row[10]);
			entry.first_rank_id    = atoi(row[11]);
			entry.enabled          = atoi(row[12]);
			entry.reset_on_death   = atoi(row[13]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<AaAbility> GetWhere(Database& db, std::string where_filter)
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
			AaAbility entry{};

			entry.id               = atoi(row[0]);
			entry.name             = row[1] ? row[1] : "";
			entry.category         = atoi(row[2]);
			entry.classes          = atoi(row[3]);
			entry.races            = atoi(row[4]);
			entry.drakkin_heritage = atoi(row[5]);
			entry.deities          = atoi(row[6]);
			entry.status           = atoi(row[7]);
			entry.type             = atoi(row[8]);
			entry.charges          = atoi(row[9]);
			entry.grant_only       = atoi(row[10]);
			entry.first_rank_id    = atoi(row[11]);
			entry.enabled          = atoi(row[12]);
			entry.reset_on_death   = atoi(row[13]);

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

#endif //EQEMU_BASE_AA_ABILITY_REPOSITORY_H
