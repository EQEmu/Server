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
		int32_t     type;
		int32_t     level;
		int32_t     ac;
		int32_t     hp;
		int32_t     accuracy;
		int32_t     slow_mitigation;
		int32_t     attack;
		int32_t     strength;
		int32_t     stamina;
		int32_t     dexterity;
		int32_t     agility;
		int32_t     intelligence;
		int32_t     wisdom;
		int32_t     charisma;
		int32_t     magic_resist;
		int32_t     cold_resist;
		int32_t     fire_resist;
		int32_t     poison_resist;
		int32_t     disease_resist;
		int32_t     corruption_resist;
		int32_t     physical_resist;
		int32_t     min_dmg;
		int32_t     max_dmg;
		int32_t     hp_regen_rate;
		int32_t     attack_delay;
		int32_t     spell_scale;
		int32_t     heal_scale;
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

	static NpcScaleGlobalBase GetNpcScaleGlobalBase(
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

			e.type              = static_cast<int32_t>(atoi(row[0]));
			e.level             = static_cast<int32_t>(atoi(row[1]));
			e.ac                = static_cast<int32_t>(atoi(row[2]));
			e.hp                = static_cast<int32_t>(atoi(row[3]));
			e.accuracy          = static_cast<int32_t>(atoi(row[4]));
			e.slow_mitigation   = static_cast<int32_t>(atoi(row[5]));
			e.attack            = static_cast<int32_t>(atoi(row[6]));
			e.strength          = static_cast<int32_t>(atoi(row[7]));
			e.stamina           = static_cast<int32_t>(atoi(row[8]));
			e.dexterity         = static_cast<int32_t>(atoi(row[9]));
			e.agility           = static_cast<int32_t>(atoi(row[10]));
			e.intelligence      = static_cast<int32_t>(atoi(row[11]));
			e.wisdom            = static_cast<int32_t>(atoi(row[12]));
			e.charisma          = static_cast<int32_t>(atoi(row[13]));
			e.magic_resist      = static_cast<int32_t>(atoi(row[14]));
			e.cold_resist       = static_cast<int32_t>(atoi(row[15]));
			e.fire_resist       = static_cast<int32_t>(atoi(row[16]));
			e.poison_resist     = static_cast<int32_t>(atoi(row[17]));
			e.disease_resist    = static_cast<int32_t>(atoi(row[18]));
			e.corruption_resist = static_cast<int32_t>(atoi(row[19]));
			e.physical_resist   = static_cast<int32_t>(atoi(row[20]));
			e.min_dmg           = static_cast<int32_t>(atoi(row[21]));
			e.max_dmg           = static_cast<int32_t>(atoi(row[22]));
			e.hp_regen_rate     = static_cast<int32_t>(atoi(row[23]));
			e.attack_delay      = static_cast<int32_t>(atoi(row[24]));
			e.spell_scale       = static_cast<int32_t>(atoi(row[25]));
			e.heal_scale        = static_cast<int32_t>(atoi(row[26]));
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
		const NpcScaleGlobalBase &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.type));
		v.push_back(columns[1] + " = " + std::to_string(e.level));
		v.push_back(columns[2] + " = " + std::to_string(e.ac));
		v.push_back(columns[3] + " = " + std::to_string(e.hp));
		v.push_back(columns[4] + " = " + std::to_string(e.accuracy));
		v.push_back(columns[5] + " = " + std::to_string(e.slow_mitigation));
		v.push_back(columns[6] + " = " + std::to_string(e.attack));
		v.push_back(columns[7] + " = " + std::to_string(e.strength));
		v.push_back(columns[8] + " = " + std::to_string(e.stamina));
		v.push_back(columns[9] + " = " + std::to_string(e.dexterity));
		v.push_back(columns[10] + " = " + std::to_string(e.agility));
		v.push_back(columns[11] + " = " + std::to_string(e.intelligence));
		v.push_back(columns[12] + " = " + std::to_string(e.wisdom));
		v.push_back(columns[13] + " = " + std::to_string(e.charisma));
		v.push_back(columns[14] + " = " + std::to_string(e.magic_resist));
		v.push_back(columns[15] + " = " + std::to_string(e.cold_resist));
		v.push_back(columns[16] + " = " + std::to_string(e.fire_resist));
		v.push_back(columns[17] + " = " + std::to_string(e.poison_resist));
		v.push_back(columns[18] + " = " + std::to_string(e.disease_resist));
		v.push_back(columns[19] + " = " + std::to_string(e.corruption_resist));
		v.push_back(columns[20] + " = " + std::to_string(e.physical_resist));
		v.push_back(columns[21] + " = " + std::to_string(e.min_dmg));
		v.push_back(columns[22] + " = " + std::to_string(e.max_dmg));
		v.push_back(columns[23] + " = " + std::to_string(e.hp_regen_rate));
		v.push_back(columns[24] + " = " + std::to_string(e.attack_delay));
		v.push_back(columns[25] + " = " + std::to_string(e.spell_scale));
		v.push_back(columns[26] + " = " + std::to_string(e.heal_scale));
		v.push_back(columns[27] + " = '" + Strings::Escape(e.special_abilities) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.type
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static NpcScaleGlobalBase InsertOne(
		Database& db,
		NpcScaleGlobalBase e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.level));
		v.push_back(std::to_string(e.ac));
		v.push_back(std::to_string(e.hp));
		v.push_back(std::to_string(e.accuracy));
		v.push_back(std::to_string(e.slow_mitigation));
		v.push_back(std::to_string(e.attack));
		v.push_back(std::to_string(e.strength));
		v.push_back(std::to_string(e.stamina));
		v.push_back(std::to_string(e.dexterity));
		v.push_back(std::to_string(e.agility));
		v.push_back(std::to_string(e.intelligence));
		v.push_back(std::to_string(e.wisdom));
		v.push_back(std::to_string(e.charisma));
		v.push_back(std::to_string(e.magic_resist));
		v.push_back(std::to_string(e.cold_resist));
		v.push_back(std::to_string(e.fire_resist));
		v.push_back(std::to_string(e.poison_resist));
		v.push_back(std::to_string(e.disease_resist));
		v.push_back(std::to_string(e.corruption_resist));
		v.push_back(std::to_string(e.physical_resist));
		v.push_back(std::to_string(e.min_dmg));
		v.push_back(std::to_string(e.max_dmg));
		v.push_back(std::to_string(e.hp_regen_rate));
		v.push_back(std::to_string(e.attack_delay));
		v.push_back(std::to_string(e.spell_scale));
		v.push_back(std::to_string(e.heal_scale));
		v.push_back("'" + Strings::Escape(e.special_abilities) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.type = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<NpcScaleGlobalBase> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.level));
			v.push_back(std::to_string(e.ac));
			v.push_back(std::to_string(e.hp));
			v.push_back(std::to_string(e.accuracy));
			v.push_back(std::to_string(e.slow_mitigation));
			v.push_back(std::to_string(e.attack));
			v.push_back(std::to_string(e.strength));
			v.push_back(std::to_string(e.stamina));
			v.push_back(std::to_string(e.dexterity));
			v.push_back(std::to_string(e.agility));
			v.push_back(std::to_string(e.intelligence));
			v.push_back(std::to_string(e.wisdom));
			v.push_back(std::to_string(e.charisma));
			v.push_back(std::to_string(e.magic_resist));
			v.push_back(std::to_string(e.cold_resist));
			v.push_back(std::to_string(e.fire_resist));
			v.push_back(std::to_string(e.poison_resist));
			v.push_back(std::to_string(e.disease_resist));
			v.push_back(std::to_string(e.corruption_resist));
			v.push_back(std::to_string(e.physical_resist));
			v.push_back(std::to_string(e.min_dmg));
			v.push_back(std::to_string(e.max_dmg));
			v.push_back(std::to_string(e.hp_regen_rate));
			v.push_back(std::to_string(e.attack_delay));
			v.push_back(std::to_string(e.spell_scale));
			v.push_back(std::to_string(e.heal_scale));
			v.push_back("'" + Strings::Escape(e.special_abilities) + "'");

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

			e.type              = static_cast<int32_t>(atoi(row[0]));
			e.level             = static_cast<int32_t>(atoi(row[1]));
			e.ac                = static_cast<int32_t>(atoi(row[2]));
			e.hp                = static_cast<int32_t>(atoi(row[3]));
			e.accuracy          = static_cast<int32_t>(atoi(row[4]));
			e.slow_mitigation   = static_cast<int32_t>(atoi(row[5]));
			e.attack            = static_cast<int32_t>(atoi(row[6]));
			e.strength          = static_cast<int32_t>(atoi(row[7]));
			e.stamina           = static_cast<int32_t>(atoi(row[8]));
			e.dexterity         = static_cast<int32_t>(atoi(row[9]));
			e.agility           = static_cast<int32_t>(atoi(row[10]));
			e.intelligence      = static_cast<int32_t>(atoi(row[11]));
			e.wisdom            = static_cast<int32_t>(atoi(row[12]));
			e.charisma          = static_cast<int32_t>(atoi(row[13]));
			e.magic_resist      = static_cast<int32_t>(atoi(row[14]));
			e.cold_resist       = static_cast<int32_t>(atoi(row[15]));
			e.fire_resist       = static_cast<int32_t>(atoi(row[16]));
			e.poison_resist     = static_cast<int32_t>(atoi(row[17]));
			e.disease_resist    = static_cast<int32_t>(atoi(row[18]));
			e.corruption_resist = static_cast<int32_t>(atoi(row[19]));
			e.physical_resist   = static_cast<int32_t>(atoi(row[20]));
			e.min_dmg           = static_cast<int32_t>(atoi(row[21]));
			e.max_dmg           = static_cast<int32_t>(atoi(row[22]));
			e.hp_regen_rate     = static_cast<int32_t>(atoi(row[23]));
			e.attack_delay      = static_cast<int32_t>(atoi(row[24]));
			e.spell_scale       = static_cast<int32_t>(atoi(row[25]));
			e.heal_scale        = static_cast<int32_t>(atoi(row[26]));
			e.special_abilities = row[27] ? row[27] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<NpcScaleGlobalBase> GetWhere(Database& db, const std::string &where_filter)
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

			e.type              = static_cast<int32_t>(atoi(row[0]));
			e.level             = static_cast<int32_t>(atoi(row[1]));
			e.ac                = static_cast<int32_t>(atoi(row[2]));
			e.hp                = static_cast<int32_t>(atoi(row[3]));
			e.accuracy          = static_cast<int32_t>(atoi(row[4]));
			e.slow_mitigation   = static_cast<int32_t>(atoi(row[5]));
			e.attack            = static_cast<int32_t>(atoi(row[6]));
			e.strength          = static_cast<int32_t>(atoi(row[7]));
			e.stamina           = static_cast<int32_t>(atoi(row[8]));
			e.dexterity         = static_cast<int32_t>(atoi(row[9]));
			e.agility           = static_cast<int32_t>(atoi(row[10]));
			e.intelligence      = static_cast<int32_t>(atoi(row[11]));
			e.wisdom            = static_cast<int32_t>(atoi(row[12]));
			e.charisma          = static_cast<int32_t>(atoi(row[13]));
			e.magic_resist      = static_cast<int32_t>(atoi(row[14]));
			e.cold_resist       = static_cast<int32_t>(atoi(row[15]));
			e.fire_resist       = static_cast<int32_t>(atoi(row[16]));
			e.poison_resist     = static_cast<int32_t>(atoi(row[17]));
			e.disease_resist    = static_cast<int32_t>(atoi(row[18]));
			e.corruption_resist = static_cast<int32_t>(atoi(row[19]));
			e.physical_resist   = static_cast<int32_t>(atoi(row[20]));
			e.min_dmg           = static_cast<int32_t>(atoi(row[21]));
			e.max_dmg           = static_cast<int32_t>(atoi(row[22]));
			e.hp_regen_rate     = static_cast<int32_t>(atoi(row[23]));
			e.attack_delay      = static_cast<int32_t>(atoi(row[24]));
			e.spell_scale       = static_cast<int32_t>(atoi(row[25]));
			e.heal_scale        = static_cast<int32_t>(atoi(row[26]));
			e.special_abilities = row[27] ? row[27] : "";

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

#endif //EQEMU_BASE_NPC_SCALE_GLOBAL_BASE_REPOSITORY_H
