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
#include "../../string_util.h"

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

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("npc_scale_global_base");
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

	static NpcScaleGlobalBase NewEntity()
	{
		NpcScaleGlobalBase entry{};

		entry.type              = 0;
		entry.level             = 0;
		entry.ac                = 0;
		entry.hp                = 0;
		entry.accuracy          = 0;
		entry.slow_mitigation   = 0;
		entry.attack            = 0;
		entry.strength          = 0;
		entry.stamina           = 0;
		entry.dexterity         = 0;
		entry.agility           = 0;
		entry.intelligence      = 0;
		entry.wisdom            = 0;
		entry.charisma          = 0;
		entry.magic_resist      = 0;
		entry.cold_resist       = 0;
		entry.fire_resist       = 0;
		entry.poison_resist     = 0;
		entry.disease_resist    = 0;
		entry.corruption_resist = 0;
		entry.physical_resist   = 0;
		entry.min_dmg           = 0;
		entry.max_dmg           = 0;
		entry.hp_regen_rate     = 0;
		entry.attack_delay      = 0;
		entry.spell_scale       = 100;
		entry.heal_scale        = 100;
		entry.special_abilities = "";

		return entry;
	}

	static NpcScaleGlobalBase GetNpcScaleGlobalBaseEntry(
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
			NpcScaleGlobalBase entry{};

			entry.type              = atoi(row[0]);
			entry.level             = atoi(row[1]);
			entry.ac                = atoi(row[2]);
			entry.hp                = atoi(row[3]);
			entry.accuracy          = atoi(row[4]);
			entry.slow_mitigation   = atoi(row[5]);
			entry.attack            = atoi(row[6]);
			entry.strength          = atoi(row[7]);
			entry.stamina           = atoi(row[8]);
			entry.dexterity         = atoi(row[9]);
			entry.agility           = atoi(row[10]);
			entry.intelligence      = atoi(row[11]);
			entry.wisdom            = atoi(row[12]);
			entry.charisma          = atoi(row[13]);
			entry.magic_resist      = atoi(row[14]);
			entry.cold_resist       = atoi(row[15]);
			entry.fire_resist       = atoi(row[16]);
			entry.poison_resist     = atoi(row[17]);
			entry.disease_resist    = atoi(row[18]);
			entry.corruption_resist = atoi(row[19]);
			entry.physical_resist   = atoi(row[20]);
			entry.min_dmg           = atoi(row[21]);
			entry.max_dmg           = atoi(row[22]);
			entry.hp_regen_rate     = atoi(row[23]);
			entry.attack_delay      = atoi(row[24]);
			entry.spell_scale       = atoi(row[25]);
			entry.heal_scale        = atoi(row[26]);
			entry.special_abilities = row[27] ? row[27] : "";

			return entry;
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
		NpcScaleGlobalBase npc_scale_global_base_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(npc_scale_global_base_entry.type));
		update_values.push_back(columns[1] + " = " + std::to_string(npc_scale_global_base_entry.level));
		update_values.push_back(columns[2] + " = " + std::to_string(npc_scale_global_base_entry.ac));
		update_values.push_back(columns[3] + " = " + std::to_string(npc_scale_global_base_entry.hp));
		update_values.push_back(columns[4] + " = " + std::to_string(npc_scale_global_base_entry.accuracy));
		update_values.push_back(columns[5] + " = " + std::to_string(npc_scale_global_base_entry.slow_mitigation));
		update_values.push_back(columns[6] + " = " + std::to_string(npc_scale_global_base_entry.attack));
		update_values.push_back(columns[7] + " = " + std::to_string(npc_scale_global_base_entry.strength));
		update_values.push_back(columns[8] + " = " + std::to_string(npc_scale_global_base_entry.stamina));
		update_values.push_back(columns[9] + " = " + std::to_string(npc_scale_global_base_entry.dexterity));
		update_values.push_back(columns[10] + " = " + std::to_string(npc_scale_global_base_entry.agility));
		update_values.push_back(columns[11] + " = " + std::to_string(npc_scale_global_base_entry.intelligence));
		update_values.push_back(columns[12] + " = " + std::to_string(npc_scale_global_base_entry.wisdom));
		update_values.push_back(columns[13] + " = " + std::to_string(npc_scale_global_base_entry.charisma));
		update_values.push_back(columns[14] + " = " + std::to_string(npc_scale_global_base_entry.magic_resist));
		update_values.push_back(columns[15] + " = " + std::to_string(npc_scale_global_base_entry.cold_resist));
		update_values.push_back(columns[16] + " = " + std::to_string(npc_scale_global_base_entry.fire_resist));
		update_values.push_back(columns[17] + " = " + std::to_string(npc_scale_global_base_entry.poison_resist));
		update_values.push_back(columns[18] + " = " + std::to_string(npc_scale_global_base_entry.disease_resist));
		update_values.push_back(columns[19] + " = " + std::to_string(npc_scale_global_base_entry.corruption_resist));
		update_values.push_back(columns[20] + " = " + std::to_string(npc_scale_global_base_entry.physical_resist));
		update_values.push_back(columns[21] + " = " + std::to_string(npc_scale_global_base_entry.min_dmg));
		update_values.push_back(columns[22] + " = " + std::to_string(npc_scale_global_base_entry.max_dmg));
		update_values.push_back(columns[23] + " = " + std::to_string(npc_scale_global_base_entry.hp_regen_rate));
		update_values.push_back(columns[24] + " = " + std::to_string(npc_scale_global_base_entry.attack_delay));
		update_values.push_back(columns[25] + " = " + std::to_string(npc_scale_global_base_entry.spell_scale));
		update_values.push_back(columns[26] + " = " + std::to_string(npc_scale_global_base_entry.heal_scale));
		update_values.push_back(columns[27] + " = '" + EscapeString(npc_scale_global_base_entry.special_abilities) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				npc_scale_global_base_entry.type
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static NpcScaleGlobalBase InsertOne(
		Database& db,
		NpcScaleGlobalBase npc_scale_global_base_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(npc_scale_global_base_entry.type));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.level));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.ac));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.hp));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.accuracy));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.slow_mitigation));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.attack));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.strength));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.stamina));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.dexterity));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.agility));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.intelligence));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.wisdom));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.charisma));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.magic_resist));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.cold_resist));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.fire_resist));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.poison_resist));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.disease_resist));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.corruption_resist));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.physical_resist));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.min_dmg));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.max_dmg));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.hp_regen_rate));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.attack_delay));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.spell_scale));
		insert_values.push_back(std::to_string(npc_scale_global_base_entry.heal_scale));
		insert_values.push_back("'" + EscapeString(npc_scale_global_base_entry.special_abilities) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			npc_scale_global_base_entry.type = results.LastInsertedID();
			return npc_scale_global_base_entry;
		}

		npc_scale_global_base_entry = NewEntity();

		return npc_scale_global_base_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<NpcScaleGlobalBase> npc_scale_global_base_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &npc_scale_global_base_entry: npc_scale_global_base_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(npc_scale_global_base_entry.type));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.level));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.ac));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.hp));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.accuracy));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.slow_mitigation));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.attack));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.strength));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.stamina));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.dexterity));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.agility));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.intelligence));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.wisdom));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.charisma));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.magic_resist));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.cold_resist));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.fire_resist));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.poison_resist));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.disease_resist));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.corruption_resist));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.physical_resist));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.min_dmg));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.max_dmg));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.hp_regen_rate));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.attack_delay));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.spell_scale));
			insert_values.push_back(std::to_string(npc_scale_global_base_entry.heal_scale));
			insert_values.push_back("'" + EscapeString(npc_scale_global_base_entry.special_abilities) + "'");

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
			NpcScaleGlobalBase entry{};

			entry.type              = atoi(row[0]);
			entry.level             = atoi(row[1]);
			entry.ac                = atoi(row[2]);
			entry.hp                = atoi(row[3]);
			entry.accuracy          = atoi(row[4]);
			entry.slow_mitigation   = atoi(row[5]);
			entry.attack            = atoi(row[6]);
			entry.strength          = atoi(row[7]);
			entry.stamina           = atoi(row[8]);
			entry.dexterity         = atoi(row[9]);
			entry.agility           = atoi(row[10]);
			entry.intelligence      = atoi(row[11]);
			entry.wisdom            = atoi(row[12]);
			entry.charisma          = atoi(row[13]);
			entry.magic_resist      = atoi(row[14]);
			entry.cold_resist       = atoi(row[15]);
			entry.fire_resist       = atoi(row[16]);
			entry.poison_resist     = atoi(row[17]);
			entry.disease_resist    = atoi(row[18]);
			entry.corruption_resist = atoi(row[19]);
			entry.physical_resist   = atoi(row[20]);
			entry.min_dmg           = atoi(row[21]);
			entry.max_dmg           = atoi(row[22]);
			entry.hp_regen_rate     = atoi(row[23]);
			entry.attack_delay      = atoi(row[24]);
			entry.spell_scale       = atoi(row[25]);
			entry.heal_scale        = atoi(row[26]);
			entry.special_abilities = row[27] ? row[27] : "";

			all_entries.push_back(entry);
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
			NpcScaleGlobalBase entry{};

			entry.type              = atoi(row[0]);
			entry.level             = atoi(row[1]);
			entry.ac                = atoi(row[2]);
			entry.hp                = atoi(row[3]);
			entry.accuracy          = atoi(row[4]);
			entry.slow_mitigation   = atoi(row[5]);
			entry.attack            = atoi(row[6]);
			entry.strength          = atoi(row[7]);
			entry.stamina           = atoi(row[8]);
			entry.dexterity         = atoi(row[9]);
			entry.agility           = atoi(row[10]);
			entry.intelligence      = atoi(row[11]);
			entry.wisdom            = atoi(row[12]);
			entry.charisma          = atoi(row[13]);
			entry.magic_resist      = atoi(row[14]);
			entry.cold_resist       = atoi(row[15]);
			entry.fire_resist       = atoi(row[16]);
			entry.poison_resist     = atoi(row[17]);
			entry.disease_resist    = atoi(row[18]);
			entry.corruption_resist = atoi(row[19]);
			entry.physical_resist   = atoi(row[20]);
			entry.min_dmg           = atoi(row[21]);
			entry.max_dmg           = atoi(row[22]);
			entry.hp_regen_rate     = atoi(row[23]);
			entry.attack_delay      = atoi(row[24]);
			entry.spell_scale       = atoi(row[25]);
			entry.heal_scale        = atoi(row[26]);
			entry.special_abilities = row[27] ? row[27] : "";

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

#endif //EQEMU_BASE_NPC_SCALE_GLOBAL_BASE_REPOSITORY_H
