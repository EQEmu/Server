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

#ifndef EQEMU_BASE_NPC_SCALE_GLOBAL_BASE_REPOSITORY_H
#define EQEMU_BASE_NPC_SCALE_GLOBAL_BASE_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseNpcScaleGlobalBaseRepository {
public:
	struct NpcScaleGlobalBase {
		int         type;
		int         level;
		int         ac;
		int         hp;
		int         accuracy;
		int         slow_mitigation;
		int         attack;
		int         strength;
		int         stamina;
		int         dexterity;
		int         agility;
		int         intelligence;
		int         wisdom;
		int         charisma;
		int         magic_resist;
		int         cold_resist;
		int         fire_resist;
		int         poison_resist;
		int         disease_resist;
		int         corruption_resist;
		int         physical_resist;
		int         min_dmg;
		int         max_dmg;
		int         hp_regen_rate;
		int         attack_delay;
		int         spell_scale;
		int         heal_scale;
		std::string special_abilities;
	};

	static std::string PrimaryKey()
	{
		return std::string("type");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"type",
			"level",
			"ac",
			"hp",
			"accuracy",
			"slow_mitigation",
			"attack",
			"strength",
			"stamina",
			"dexterity",
			"agility",
			"intelligence",
			"wisdom",
			"charisma",
			"magic_resist",
			"cold_resist",
			"fire_resist",
			"poison_resist",
			"disease_resist",
			"corruption_resist",
			"physical_resist",
			"min_dmg",
			"max_dmg",
			"hp_regen_rate",
			"attack_delay",
			"spell_scale",
			"heal_scale",
			"special_abilities",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"type",
			"level",
			"ac",
			"hp",
			"accuracy",
			"slow_mitigation",
			"attack",
			"strength",
			"stamina",
			"dexterity",
			"agility",
			"intelligence",
			"wisdom",
			"charisma",
			"magic_resist",
			"cold_resist",
			"fire_resist",
			"poison_resist",
			"disease_resist",
			"corruption_resist",
			"physical_resist",
			"min_dmg",
			"max_dmg",
			"hp_regen_rate",
			"attack_delay",
			"spell_scale",
			"heal_scale",
			"special_abilities",
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
		return std::string("npc_scale_global_base");
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

	static NpcScaleGlobalBase NewEntity()
	{
		NpcScaleGlobalBase e{};

		e.type              = 0;
		e.level             = 0;
		e.ac                = 0;
		e.hp                = 0;
		e.accuracy          = 0;
		e.slow_mitigation   = 0;
		e.attack            = 0;
		e.strength          = 0;
		e.stamina           = 0;
		e.dexterity         = 0;
		e.agility           = 0;
		e.intelligence      = 0;
		e.wisdom            = 0;
		e.charisma          = 0;
		e.magic_resist      = 0;
		e.cold_resist       = 0;
		e.fire_resist       = 0;
		e.poison_resist     = 0;
		e.disease_resist    = 0;
		e.corruption_resist = 0;
		e.physical_resist   = 0;
		e.min_dmg           = 0;
		e.max_dmg           = 0;
		e.hp_regen_rate     = 0;
		e.attack_delay      = 0;
		e.spell_scale       = 100;
		e.heal_scale        = 100;
		e.special_abilities = "";

		return e;
	}

	static NpcScaleGlobalBase GetNpcScaleGlobalBasee(
		const std::vector<NpcScaleGlobalBase> &npc_scale_global_bases,
		int npc_scale_global_base_id
	)
	{
		for (auto &npc_scale_global_base : npc_scale_global_bases) {
			if (npc_scale_global_base.type == npc_scale_global_base_id) {
				return npc_scale_global_base;
			}
		}

		return NewEntity();
	}

	static NpcScaleGlobalBase FindOne(
		Database& db,
		int npc_scale_global_base_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				npc_scale_global_base_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			NpcScaleGlobalBase e{};

			e.type              = atoi(row[0]);
			e.level             = atoi(row[1]);
			e.ac                = atoi(row[2]);
			e.hp                = atoi(row[3]);
			e.accuracy          = atoi(row[4]);
			e.slow_mitigation   = atoi(row[5]);
			e.attack            = atoi(row[6]);
			e.strength          = atoi(row[7]);
			e.stamina           = atoi(row[8]);
			e.dexterity         = atoi(row[9]);
			e.agility           = atoi(row[10]);
			e.intelligence      = atoi(row[11]);
			e.wisdom            = atoi(row[12]);
			e.charisma          = atoi(row[13]);
			e.magic_resist      = atoi(row[14]);
			e.cold_resist       = atoi(row[15]);
			e.fire_resist       = atoi(row[16]);
			e.poison_resist     = atoi(row[17]);
			e.disease_resist    = atoi(row[18]);
			e.corruption_resist = atoi(row[19]);
			e.physical_resist   = atoi(row[20]);
			e.min_dmg           = atoi(row[21]);
			e.max_dmg           = atoi(row[22]);
			e.hp_regen_rate     = atoi(row[23]);
			e.attack_delay      = atoi(row[24]);
			e.spell_scale       = atoi(row[25]);
			e.heal_scale        = atoi(row[26]);
			e.special_abilities = row[27] ? row[27] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int npc_scale_global_base_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				npc_scale_global_base_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		NpcScaleGlobalBase npc_scale_global_base_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(npc_scale_global_base_e.type));
		update_values.push_back(columns[1] + " = " + std::to_string(npc_scale_global_base_e.level));
		update_values.push_back(columns[2] + " = " + std::to_string(npc_scale_global_base_e.ac));
		update_values.push_back(columns[3] + " = " + std::to_string(npc_scale_global_base_e.hp));
		update_values.push_back(columns[4] + " = " + std::to_string(npc_scale_global_base_e.accuracy));
		update_values.push_back(columns[5] + " = " + std::to_string(npc_scale_global_base_e.slow_mitigation));
		update_values.push_back(columns[6] + " = " + std::to_string(npc_scale_global_base_e.attack));
		update_values.push_back(columns[7] + " = " + std::to_string(npc_scale_global_base_e.strength));
		update_values.push_back(columns[8] + " = " + std::to_string(npc_scale_global_base_e.stamina));
		update_values.push_back(columns[9] + " = " + std::to_string(npc_scale_global_base_e.dexterity));
		update_values.push_back(columns[10] + " = " + std::to_string(npc_scale_global_base_e.agility));
		update_values.push_back(columns[11] + " = " + std::to_string(npc_scale_global_base_e.intelligence));
		update_values.push_back(columns[12] + " = " + std::to_string(npc_scale_global_base_e.wisdom));
		update_values.push_back(columns[13] + " = " + std::to_string(npc_scale_global_base_e.charisma));
		update_values.push_back(columns[14] + " = " + std::to_string(npc_scale_global_base_e.magic_resist));
		update_values.push_back(columns[15] + " = " + std::to_string(npc_scale_global_base_e.cold_resist));
		update_values.push_back(columns[16] + " = " + std::to_string(npc_scale_global_base_e.fire_resist));
		update_values.push_back(columns[17] + " = " + std::to_string(npc_scale_global_base_e.poison_resist));
		update_values.push_back(columns[18] + " = " + std::to_string(npc_scale_global_base_e.disease_resist));
		update_values.push_back(columns[19] + " = " + std::to_string(npc_scale_global_base_e.corruption_resist));
		update_values.push_back(columns[20] + " = " + std::to_string(npc_scale_global_base_e.physical_resist));
		update_values.push_back(columns[21] + " = " + std::to_string(npc_scale_global_base_e.min_dmg));
		update_values.push_back(columns[22] + " = " + std::to_string(npc_scale_global_base_e.max_dmg));
		update_values.push_back(columns[23] + " = " + std::to_string(npc_scale_global_base_e.hp_regen_rate));
		update_values.push_back(columns[24] + " = " + std::to_string(npc_scale_global_base_e.attack_delay));
		update_values.push_back(columns[25] + " = " + std::to_string(npc_scale_global_base_e.spell_scale));
		update_values.push_back(columns[26] + " = " + std::to_string(npc_scale_global_base_e.heal_scale));
		update_values.push_back(columns[27] + " = '" + Strings::Escape(npc_scale_global_base_e.special_abilities) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				npc_scale_global_base_e.type
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static NpcScaleGlobalBase InsertOne(
		Database& db,
		NpcScaleGlobalBase npc_scale_global_base_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(npc_scale_global_base_e.type));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.level));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.ac));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.hp));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.accuracy));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.slow_mitigation));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.attack));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.strength));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.stamina));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.dexterity));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.agility));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.intelligence));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.wisdom));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.charisma));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.magic_resist));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.cold_resist));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.fire_resist));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.poison_resist));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.disease_resist));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.corruption_resist));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.physical_resist));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.min_dmg));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.max_dmg));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.hp_regen_rate));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.attack_delay));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.spell_scale));
		insert_values.push_back(std::to_string(npc_scale_global_base_e.heal_scale));
		insert_values.push_back("'" + Strings::Escape(npc_scale_global_base_e.special_abilities) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			npc_scale_global_base_e.type = results.LastInsertedID();
			return npc_scale_global_base_e;
		}

		npc_scale_global_base_e = NewEntity();

		return npc_scale_global_base_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<NpcScaleGlobalBase> npc_scale_global_base_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &npc_scale_global_base_e: npc_scale_global_base_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(npc_scale_global_base_e.type));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.level));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.ac));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.hp));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.accuracy));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.slow_mitigation));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.attack));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.strength));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.stamina));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.dexterity));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.agility));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.intelligence));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.wisdom));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.charisma));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.magic_resist));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.cold_resist));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.fire_resist));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.poison_resist));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.disease_resist));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.corruption_resist));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.physical_resist));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.min_dmg));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.max_dmg));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.hp_regen_rate));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.attack_delay));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.spell_scale));
			insert_values.push_back(std::to_string(npc_scale_global_base_e.heal_scale));
			insert_values.push_back("'" + Strings::Escape(npc_scale_global_base_e.special_abilities) + "'");

			insert_chunks.push_back("(" + Strings::Implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<NpcScaleGlobalBase> All(Database& db)
	{
		std::vector<NpcScaleGlobalBase> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcScaleGlobalBase e{};

			e.type              = atoi(row[0]);
			e.level             = atoi(row[1]);
			e.ac                = atoi(row[2]);
			e.hp                = atoi(row[3]);
			e.accuracy          = atoi(row[4]);
			e.slow_mitigation   = atoi(row[5]);
			e.attack            = atoi(row[6]);
			e.strength          = atoi(row[7]);
			e.stamina           = atoi(row[8]);
			e.dexterity         = atoi(row[9]);
			e.agility           = atoi(row[10]);
			e.intelligence      = atoi(row[11]);
			e.wisdom            = atoi(row[12]);
			e.charisma          = atoi(row[13]);
			e.magic_resist      = atoi(row[14]);
			e.cold_resist       = atoi(row[15]);
			e.fire_resist       = atoi(row[16]);
			e.poison_resist     = atoi(row[17]);
			e.disease_resist    = atoi(row[18]);
			e.corruption_resist = atoi(row[19]);
			e.physical_resist   = atoi(row[20]);
			e.min_dmg           = atoi(row[21]);
			e.max_dmg           = atoi(row[22]);
			e.hp_regen_rate     = atoi(row[23]);
			e.attack_delay      = atoi(row[24]);
			e.spell_scale       = atoi(row[25]);
			e.heal_scale        = atoi(row[26]);
			e.special_abilities = row[27] ? row[27] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<NpcScaleGlobalBase> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<NpcScaleGlobalBase> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcScaleGlobalBase e{};

			e.type              = atoi(row[0]);
			e.level             = atoi(row[1]);
			e.ac                = atoi(row[2]);
			e.hp                = atoi(row[3]);
			e.accuracy          = atoi(row[4]);
			e.slow_mitigation   = atoi(row[5]);
			e.attack            = atoi(row[6]);
			e.strength          = atoi(row[7]);
			e.stamina           = atoi(row[8]);
			e.dexterity         = atoi(row[9]);
			e.agility           = atoi(row[10]);
			e.intelligence      = atoi(row[11]);
			e.wisdom            = atoi(row[12]);
			e.charisma          = atoi(row[13]);
			e.magic_resist      = atoi(row[14]);
			e.cold_resist       = atoi(row[15]);
			e.fire_resist       = atoi(row[16]);
			e.poison_resist     = atoi(row[17]);
			e.disease_resist    = atoi(row[18]);
			e.corruption_resist = atoi(row[19]);
			e.physical_resist   = atoi(row[20]);
			e.min_dmg           = atoi(row[21]);
			e.max_dmg           = atoi(row[22]);
			e.hp_regen_rate     = atoi(row[23]);
			e.attack_delay      = atoi(row[24]);
			e.spell_scale       = atoi(row[25]);
			e.heal_scale        = atoi(row[26]);
			e.special_abilities = row[27] ? row[27] : "";

			all_entries.push_back(e);
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

	static int64 GetMaxId(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT MAX({}) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string& where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

};

#endif //EQEMU_BASE_NPC_SCALE_GLOBAL_BASE_REPOSITORY_H
