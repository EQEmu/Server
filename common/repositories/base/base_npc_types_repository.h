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

#ifndef EQEMU_BASE_NPC_TYPES_REPOSITORY_H
#define EQEMU_BASE_NPC_TYPES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"
#include <ctime>

class BaseNpcTypesRepository {
public:
	struct NpcTypes {
		int         id;
		std::string name;
		std::string lastname;
		int         level;
		int         race;
		int         class_;
		int         bodytype;
		int64       hp;
		int64       mana;
		int         gender;
		int         texture;
		int         helmtexture;
		int         herosforgemodel;
		float       size;
		int64       hp_regen_rate;
		int64       hp_regen_per_second;
		int64       mana_regen_rate;
		int         loottable_id;
		int         merchant_id;
		int         alt_currency_id;
		int         npc_spells_id;
		int         npc_spells_effects_id;
		int         npc_faction_id;
		int         adventure_template_id;
		int         trap_template;
		int         mindmg;
		int         maxdmg;
		int         attack_count;
		std::string npcspecialattks;
		std::string special_abilities;
		int         aggroradius;
		int         assistradius;
		int         face;
		int         luclin_hairstyle;
		int         luclin_haircolor;
		int         luclin_eyecolor;
		int         luclin_eyecolor2;
		int         luclin_beardcolor;
		int         luclin_beard;
		int         drakkin_heritage;
		int         drakkin_tattoo;
		int         drakkin_details;
		int         armortint_id;
		int         armortint_red;
		int         armortint_green;
		int         armortint_blue;
		int         d_melee_texture1;
		int         d_melee_texture2;
		std::string ammo_idfile;
		int         prim_melee_type;
		int         sec_melee_type;
		int         ranged_type;
		float       runspeed;
		int         MR;
		int         CR;
		int         DR;
		int         FR;
		int         PR;
		int         Corrup;
		int         PhR;
		int         see_invis;
		int         see_invis_undead;
		int         qglobal;
		int         AC;
		int         npc_aggro;
		int         spawn_limit;
		float       attack_speed;
		int         attack_delay;
		int         findable;
		int         STR;
		int         STA;
		int         DEX;
		int         AGI;
		int         _INT;
		int         WIS;
		int         CHA;
		int         see_hide;
		int         see_improved_hide;
		int         trackable;
		int         isbot;
		int         exclude;
		int         ATK;
		int         Accuracy;
		int         Avoidance;
		int         slow_mitigation;
		int         version;
		int         maxlevel;
		int         scalerate;
		int         private_corpse;
		int         unique_spawn_by_name;
		int         underwater;
		int         isquest;
		int         emoteid;
		float       spellscale;
		float       healscale;
		int         no_target_hotkey;
		int         raid_target;
		int         armtexture;
		int         bracertexture;
		int         handtexture;
		int         legtexture;
		int         feettexture;
		int         light;
		int         walkspeed;
		int         peqid;
		int         unique_;
		int         fixed;
		int         ignore_despawn;
		int         show_name;
		int         untargetable;
		int         charm_ac;
		int         charm_min_dmg;
		int         charm_max_dmg;
		int         charm_attack_delay;
		int         charm_accuracy_rating;
		int         charm_avoidance_rating;
		int         charm_atk;
		int         skip_global_loot;
		int         rare_spawn;
		int         stuck_behavior;
		int         model;
		int         flymode;
		int         always_aggro;
		int         exp_mod;
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
			"lastname",
			"level",
			"race",
			"`class`",
			"bodytype",
			"hp",
			"mana",
			"gender",
			"texture",
			"helmtexture",
			"herosforgemodel",
			"size",
			"hp_regen_rate",
			"hp_regen_per_second",
			"mana_regen_rate",
			"loottable_id",
			"merchant_id",
			"alt_currency_id",
			"npc_spells_id",
			"npc_spells_effects_id",
			"npc_faction_id",
			"adventure_template_id",
			"trap_template",
			"mindmg",
			"maxdmg",
			"attack_count",
			"npcspecialattks",
			"special_abilities",
			"aggroradius",
			"assistradius",
			"face",
			"luclin_hairstyle",
			"luclin_haircolor",
			"luclin_eyecolor",
			"luclin_eyecolor2",
			"luclin_beardcolor",
			"luclin_beard",
			"drakkin_heritage",
			"drakkin_tattoo",
			"drakkin_details",
			"armortint_id",
			"armortint_red",
			"armortint_green",
			"armortint_blue",
			"d_melee_texture1",
			"d_melee_texture2",
			"ammo_idfile",
			"prim_melee_type",
			"sec_melee_type",
			"ranged_type",
			"runspeed",
			"MR",
			"CR",
			"DR",
			"FR",
			"PR",
			"Corrup",
			"PhR",
			"see_invis",
			"see_invis_undead",
			"qglobal",
			"AC",
			"npc_aggro",
			"spawn_limit",
			"attack_speed",
			"attack_delay",
			"findable",
			"STR",
			"STA",
			"DEX",
			"AGI",
			"_INT",
			"WIS",
			"CHA",
			"see_hide",
			"see_improved_hide",
			"trackable",
			"isbot",
			"exclude",
			"ATK",
			"Accuracy",
			"Avoidance",
			"slow_mitigation",
			"version",
			"maxlevel",
			"scalerate",
			"private_corpse",
			"unique_spawn_by_name",
			"underwater",
			"isquest",
			"emoteid",
			"spellscale",
			"healscale",
			"no_target_hotkey",
			"raid_target",
			"armtexture",
			"bracertexture",
			"handtexture",
			"legtexture",
			"feettexture",
			"light",
			"walkspeed",
			"peqid",
			"unique_",
			"fixed",
			"ignore_despawn",
			"show_name",
			"untargetable",
			"charm_ac",
			"charm_min_dmg",
			"charm_max_dmg",
			"charm_attack_delay",
			"charm_accuracy_rating",
			"charm_avoidance_rating",
			"charm_atk",
			"skip_global_loot",
			"rare_spawn",
			"stuck_behavior",
			"model",
			"flymode",
			"always_aggro",
			"exp_mod",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"name",
			"lastname",
			"level",
			"race",
			"`class`",
			"bodytype",
			"hp",
			"mana",
			"gender",
			"texture",
			"helmtexture",
			"herosforgemodel",
			"size",
			"hp_regen_rate",
			"hp_regen_per_second",
			"mana_regen_rate",
			"loottable_id",
			"merchant_id",
			"alt_currency_id",
			"npc_spells_id",
			"npc_spells_effects_id",
			"npc_faction_id",
			"adventure_template_id",
			"trap_template",
			"mindmg",
			"maxdmg",
			"attack_count",
			"npcspecialattks",
			"special_abilities",
			"aggroradius",
			"assistradius",
			"face",
			"luclin_hairstyle",
			"luclin_haircolor",
			"luclin_eyecolor",
			"luclin_eyecolor2",
			"luclin_beardcolor",
			"luclin_beard",
			"drakkin_heritage",
			"drakkin_tattoo",
			"drakkin_details",
			"armortint_id",
			"armortint_red",
			"armortint_green",
			"armortint_blue",
			"d_melee_texture1",
			"d_melee_texture2",
			"ammo_idfile",
			"prim_melee_type",
			"sec_melee_type",
			"ranged_type",
			"runspeed",
			"MR",
			"CR",
			"DR",
			"FR",
			"PR",
			"Corrup",
			"PhR",
			"see_invis",
			"see_invis_undead",
			"qglobal",
			"AC",
			"npc_aggro",
			"spawn_limit",
			"attack_speed",
			"attack_delay",
			"findable",
			"STR",
			"STA",
			"DEX",
			"AGI",
			"_INT",
			"WIS",
			"CHA",
			"see_hide",
			"see_improved_hide",
			"trackable",
			"isbot",
			"exclude",
			"ATK",
			"Accuracy",
			"Avoidance",
			"slow_mitigation",
			"version",
			"maxlevel",
			"scalerate",
			"private_corpse",
			"unique_spawn_by_name",
			"underwater",
			"isquest",
			"emoteid",
			"spellscale",
			"healscale",
			"no_target_hotkey",
			"raid_target",
			"armtexture",
			"bracertexture",
			"handtexture",
			"legtexture",
			"feettexture",
			"light",
			"walkspeed",
			"peqid",
			"unique_",
			"fixed",
			"ignore_despawn",
			"show_name",
			"untargetable",
			"charm_ac",
			"charm_min_dmg",
			"charm_max_dmg",
			"charm_attack_delay",
			"charm_accuracy_rating",
			"charm_avoidance_rating",
			"charm_atk",
			"skip_global_loot",
			"rare_spawn",
			"stuck_behavior",
			"model",
			"flymode",
			"always_aggro",
			"exp_mod",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("npc_types");
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

	static NpcTypes NewEntity()
	{
		NpcTypes entry{};

		entry.id                     = 0;
		entry.name                   = "";
		entry.lastname               = "";
		entry.level                  = 0;
		entry.race                   = 0;
		entry.class_                 = 0;
		entry.bodytype               = 1;
		entry.hp                     = 0;
		entry.mana                   = 0;
		entry.gender                 = 0;
		entry.texture                = 0;
		entry.helmtexture            = 0;
		entry.herosforgemodel        = 0;
		entry.size                   = 0;
		entry.hp_regen_rate          = 0;
		entry.hp_regen_per_second    = 0;
		entry.mana_regen_rate        = 0;
		entry.loottable_id           = 0;
		entry.merchant_id            = 0;
		entry.alt_currency_id        = 0;
		entry.npc_spells_id          = 0;
		entry.npc_spells_effects_id  = 0;
		entry.npc_faction_id         = 0;
		entry.adventure_template_id  = 0;
		entry.trap_template          = 0;
		entry.mindmg                 = 0;
		entry.maxdmg                 = 0;
		entry.attack_count           = -1;
		entry.npcspecialattks        = "";
		entry.special_abilities      = "";
		entry.aggroradius            = 0;
		entry.assistradius           = 0;
		entry.face                   = 1;
		entry.luclin_hairstyle       = 1;
		entry.luclin_haircolor       = 1;
		entry.luclin_eyecolor        = 1;
		entry.luclin_eyecolor2       = 1;
		entry.luclin_beardcolor      = 1;
		entry.luclin_beard           = 0;
		entry.drakkin_heritage       = 0;
		entry.drakkin_tattoo         = 0;
		entry.drakkin_details        = 0;
		entry.armortint_id           = 0;
		entry.armortint_red          = 0;
		entry.armortint_green        = 0;
		entry.armortint_blue         = 0;
		entry.d_melee_texture1       = 0;
		entry.d_melee_texture2       = 0;
		entry.ammo_idfile            = "IT10";
		entry.prim_melee_type        = 28;
		entry.sec_melee_type         = 28;
		entry.ranged_type            = 7;
		entry.runspeed               = 0;
		entry.MR                     = 0;
		entry.CR                     = 0;
		entry.DR                     = 0;
		entry.FR                     = 0;
		entry.PR                     = 0;
		entry.Corrup                 = 0;
		entry.PhR                    = 0;
		entry.see_invis              = 0;
		entry.see_invis_undead       = 0;
		entry.qglobal                = 0;
		entry.AC                     = 0;
		entry.npc_aggro              = 0;
		entry.spawn_limit            = 0;
		entry.attack_speed           = 0;
		entry.attack_delay           = 30;
		entry.findable               = 0;
		entry.STR                    = 75;
		entry.STA                    = 75;
		entry.DEX                    = 75;
		entry.AGI                    = 75;
		entry._INT                   = 80;
		entry.WIS                    = 75;
		entry.CHA                    = 75;
		entry.see_hide               = 0;
		entry.see_improved_hide      = 0;
		entry.trackable              = 1;
		entry.isbot                  = 0;
		entry.exclude                = 1;
		entry.ATK                    = 0;
		entry.Accuracy               = 0;
		entry.Avoidance              = 0;
		entry.slow_mitigation        = 0;
		entry.version                = 0;
		entry.maxlevel               = 0;
		entry.scalerate              = 100;
		entry.private_corpse         = 0;
		entry.unique_spawn_by_name   = 0;
		entry.underwater             = 0;
		entry.isquest                = 0;
		entry.emoteid                = 0;
		entry.spellscale             = 100;
		entry.healscale              = 100;
		entry.no_target_hotkey       = 0;
		entry.raid_target            = 0;
		entry.armtexture             = 0;
		entry.bracertexture          = 0;
		entry.handtexture            = 0;
		entry.legtexture             = 0;
		entry.feettexture            = 0;
		entry.light                  = 0;
		entry.walkspeed              = 0;
		entry.peqid                  = 0;
		entry.unique_                = 0;
		entry.fixed                  = 0;
		entry.ignore_despawn         = 0;
		entry.show_name              = 1;
		entry.untargetable           = 0;
		entry.charm_ac               = 0;
		entry.charm_min_dmg          = 0;
		entry.charm_max_dmg          = 0;
		entry.charm_attack_delay     = 0;
		entry.charm_accuracy_rating  = 0;
		entry.charm_avoidance_rating = 0;
		entry.charm_atk              = 0;
		entry.skip_global_loot       = 0;
		entry.rare_spawn             = 0;
		entry.stuck_behavior         = 0;
		entry.model                  = 0;
		entry.flymode                = -1;
		entry.always_aggro           = 0;
		entry.exp_mod                = 100;

		return entry;
	}

	static NpcTypes GetNpcTypesEntry(
		const std::vector<NpcTypes> &npc_typess,
		int npc_types_id
	)
	{
		for (auto &npc_types : npc_typess) {
			if (npc_types.id == npc_types_id) {
				return npc_types;
			}
		}

		return NewEntity();
	}

	static NpcTypes FindOne(
		Database& db,
		int npc_types_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				npc_types_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			NpcTypes entry{};

			entry.id                     = atoi(row[0]);
			entry.name                   = row[1] ? row[1] : "";
			entry.lastname               = row[2] ? row[2] : "";
			entry.level                  = atoi(row[3]);
			entry.race                   = atoi(row[4]);
			entry.class_                 = atoi(row[5]);
			entry.bodytype               = atoi(row[6]);
			entry.hp                     = strtoll(row[7], nullptr, 10);
			entry.mana                   = strtoll(row[8], nullptr, 10);
			entry.gender                 = atoi(row[9]);
			entry.texture                = atoi(row[10]);
			entry.helmtexture            = atoi(row[11]);
			entry.herosforgemodel        = atoi(row[12]);
			entry.size                   = static_cast<float>(atof(row[13]));
			entry.hp_regen_rate          = strtoll(row[14], nullptr, 10);
			entry.hp_regen_per_second    = strtoll(row[15], nullptr, 10);
			entry.mana_regen_rate        = strtoll(row[16], nullptr, 10);
			entry.loottable_id           = atoi(row[17]);
			entry.merchant_id            = atoi(row[18]);
			entry.alt_currency_id        = atoi(row[19]);
			entry.npc_spells_id          = atoi(row[20]);
			entry.npc_spells_effects_id  = atoi(row[21]);
			entry.npc_faction_id         = atoi(row[22]);
			entry.adventure_template_id  = atoi(row[23]);
			entry.trap_template          = atoi(row[24]);
			entry.mindmg                 = atoi(row[25]);
			entry.maxdmg                 = atoi(row[26]);
			entry.attack_count           = atoi(row[27]);
			entry.npcspecialattks        = row[28] ? row[28] : "";
			entry.special_abilities      = row[29] ? row[29] : "";
			entry.aggroradius            = atoi(row[30]);
			entry.assistradius           = atoi(row[31]);
			entry.face                   = atoi(row[32]);
			entry.luclin_hairstyle       = atoi(row[33]);
			entry.luclin_haircolor       = atoi(row[34]);
			entry.luclin_eyecolor        = atoi(row[35]);
			entry.luclin_eyecolor2       = atoi(row[36]);
			entry.luclin_beardcolor      = atoi(row[37]);
			entry.luclin_beard           = atoi(row[38]);
			entry.drakkin_heritage       = atoi(row[39]);
			entry.drakkin_tattoo         = atoi(row[40]);
			entry.drakkin_details        = atoi(row[41]);
			entry.armortint_id           = atoi(row[42]);
			entry.armortint_red          = atoi(row[43]);
			entry.armortint_green        = atoi(row[44]);
			entry.armortint_blue         = atoi(row[45]);
			entry.d_melee_texture1       = atoi(row[46]);
			entry.d_melee_texture2       = atoi(row[47]);
			entry.ammo_idfile            = row[48] ? row[48] : "";
			entry.prim_melee_type        = atoi(row[49]);
			entry.sec_melee_type         = atoi(row[50]);
			entry.ranged_type            = atoi(row[51]);
			entry.runspeed               = static_cast<float>(atof(row[52]));
			entry.MR                     = atoi(row[53]);
			entry.CR                     = atoi(row[54]);
			entry.DR                     = atoi(row[55]);
			entry.FR                     = atoi(row[56]);
			entry.PR                     = atoi(row[57]);
			entry.Corrup                 = atoi(row[58]);
			entry.PhR                    = atoi(row[59]);
			entry.see_invis              = atoi(row[60]);
			entry.see_invis_undead       = atoi(row[61]);
			entry.qglobal                = atoi(row[62]);
			entry.AC                     = atoi(row[63]);
			entry.npc_aggro              = atoi(row[64]);
			entry.spawn_limit            = atoi(row[65]);
			entry.attack_speed           = static_cast<float>(atof(row[66]));
			entry.attack_delay           = atoi(row[67]);
			entry.findable               = atoi(row[68]);
			entry.STR                    = atoi(row[69]);
			entry.STA                    = atoi(row[70]);
			entry.DEX                    = atoi(row[71]);
			entry.AGI                    = atoi(row[72]);
			entry._INT                   = atoi(row[73]);
			entry.WIS                    = atoi(row[74]);
			entry.CHA                    = atoi(row[75]);
			entry.see_hide               = atoi(row[76]);
			entry.see_improved_hide      = atoi(row[77]);
			entry.trackable              = atoi(row[78]);
			entry.isbot                  = atoi(row[79]);
			entry.exclude                = atoi(row[80]);
			entry.ATK                    = atoi(row[81]);
			entry.Accuracy               = atoi(row[82]);
			entry.Avoidance              = atoi(row[83]);
			entry.slow_mitigation        = atoi(row[84]);
			entry.version                = atoi(row[85]);
			entry.maxlevel               = atoi(row[86]);
			entry.scalerate              = atoi(row[87]);
			entry.private_corpse         = atoi(row[88]);
			entry.unique_spawn_by_name   = atoi(row[89]);
			entry.underwater             = atoi(row[90]);
			entry.isquest                = atoi(row[91]);
			entry.emoteid                = atoi(row[92]);
			entry.spellscale             = static_cast<float>(atof(row[93]));
			entry.healscale              = static_cast<float>(atof(row[94]));
			entry.no_target_hotkey       = atoi(row[95]);
			entry.raid_target            = atoi(row[96]);
			entry.armtexture             = atoi(row[97]);
			entry.bracertexture          = atoi(row[98]);
			entry.handtexture            = atoi(row[99]);
			entry.legtexture             = atoi(row[100]);
			entry.feettexture            = atoi(row[101]);
			entry.light                  = atoi(row[102]);
			entry.walkspeed              = atoi(row[103]);
			entry.peqid                  = atoi(row[104]);
			entry.unique_                = atoi(row[105]);
			entry.fixed                  = atoi(row[106]);
			entry.ignore_despawn         = atoi(row[107]);
			entry.show_name              = atoi(row[108]);
			entry.untargetable           = atoi(row[109]);
			entry.charm_ac               = atoi(row[110]);
			entry.charm_min_dmg          = atoi(row[111]);
			entry.charm_max_dmg          = atoi(row[112]);
			entry.charm_attack_delay     = atoi(row[113]);
			entry.charm_accuracy_rating  = atoi(row[114]);
			entry.charm_avoidance_rating = atoi(row[115]);
			entry.charm_atk              = atoi(row[116]);
			entry.skip_global_loot       = atoi(row[117]);
			entry.rare_spawn             = atoi(row[118]);
			entry.stuck_behavior         = atoi(row[119]);
			entry.model                  = atoi(row[120]);
			entry.flymode                = atoi(row[121]);
			entry.always_aggro           = atoi(row[122]);
			entry.exp_mod                = atoi(row[123]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int npc_types_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				npc_types_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		NpcTypes npc_types_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(npc_types_entry.name) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(npc_types_entry.lastname) + "'");
		update_values.push_back(columns[3] + " = " + std::to_string(npc_types_entry.level));
		update_values.push_back(columns[4] + " = " + std::to_string(npc_types_entry.race));
		update_values.push_back(columns[5] + " = " + std::to_string(npc_types_entry.class_));
		update_values.push_back(columns[6] + " = " + std::to_string(npc_types_entry.bodytype));
		update_values.push_back(columns[7] + " = " + std::to_string(npc_types_entry.hp));
		update_values.push_back(columns[8] + " = " + std::to_string(npc_types_entry.mana));
		update_values.push_back(columns[9] + " = " + std::to_string(npc_types_entry.gender));
		update_values.push_back(columns[10] + " = " + std::to_string(npc_types_entry.texture));
		update_values.push_back(columns[11] + " = " + std::to_string(npc_types_entry.helmtexture));
		update_values.push_back(columns[12] + " = " + std::to_string(npc_types_entry.herosforgemodel));
		update_values.push_back(columns[13] + " = " + std::to_string(npc_types_entry.size));
		update_values.push_back(columns[14] + " = " + std::to_string(npc_types_entry.hp_regen_rate));
		update_values.push_back(columns[15] + " = " + std::to_string(npc_types_entry.hp_regen_per_second));
		update_values.push_back(columns[16] + " = " + std::to_string(npc_types_entry.mana_regen_rate));
		update_values.push_back(columns[17] + " = " + std::to_string(npc_types_entry.loottable_id));
		update_values.push_back(columns[18] + " = " + std::to_string(npc_types_entry.merchant_id));
		update_values.push_back(columns[19] + " = " + std::to_string(npc_types_entry.alt_currency_id));
		update_values.push_back(columns[20] + " = " + std::to_string(npc_types_entry.npc_spells_id));
		update_values.push_back(columns[21] + " = " + std::to_string(npc_types_entry.npc_spells_effects_id));
		update_values.push_back(columns[22] + " = " + std::to_string(npc_types_entry.npc_faction_id));
		update_values.push_back(columns[23] + " = " + std::to_string(npc_types_entry.adventure_template_id));
		update_values.push_back(columns[24] + " = " + std::to_string(npc_types_entry.trap_template));
		update_values.push_back(columns[25] + " = " + std::to_string(npc_types_entry.mindmg));
		update_values.push_back(columns[26] + " = " + std::to_string(npc_types_entry.maxdmg));
		update_values.push_back(columns[27] + " = " + std::to_string(npc_types_entry.attack_count));
		update_values.push_back(columns[28] + " = '" + EscapeString(npc_types_entry.npcspecialattks) + "'");
		update_values.push_back(columns[29] + " = '" + EscapeString(npc_types_entry.special_abilities) + "'");
		update_values.push_back(columns[30] + " = " + std::to_string(npc_types_entry.aggroradius));
		update_values.push_back(columns[31] + " = " + std::to_string(npc_types_entry.assistradius));
		update_values.push_back(columns[32] + " = " + std::to_string(npc_types_entry.face));
		update_values.push_back(columns[33] + " = " + std::to_string(npc_types_entry.luclin_hairstyle));
		update_values.push_back(columns[34] + " = " + std::to_string(npc_types_entry.luclin_haircolor));
		update_values.push_back(columns[35] + " = " + std::to_string(npc_types_entry.luclin_eyecolor));
		update_values.push_back(columns[36] + " = " + std::to_string(npc_types_entry.luclin_eyecolor2));
		update_values.push_back(columns[37] + " = " + std::to_string(npc_types_entry.luclin_beardcolor));
		update_values.push_back(columns[38] + " = " + std::to_string(npc_types_entry.luclin_beard));
		update_values.push_back(columns[39] + " = " + std::to_string(npc_types_entry.drakkin_heritage));
		update_values.push_back(columns[40] + " = " + std::to_string(npc_types_entry.drakkin_tattoo));
		update_values.push_back(columns[41] + " = " + std::to_string(npc_types_entry.drakkin_details));
		update_values.push_back(columns[42] + " = " + std::to_string(npc_types_entry.armortint_id));
		update_values.push_back(columns[43] + " = " + std::to_string(npc_types_entry.armortint_red));
		update_values.push_back(columns[44] + " = " + std::to_string(npc_types_entry.armortint_green));
		update_values.push_back(columns[45] + " = " + std::to_string(npc_types_entry.armortint_blue));
		update_values.push_back(columns[46] + " = " + std::to_string(npc_types_entry.d_melee_texture1));
		update_values.push_back(columns[47] + " = " + std::to_string(npc_types_entry.d_melee_texture2));
		update_values.push_back(columns[48] + " = '" + EscapeString(npc_types_entry.ammo_idfile) + "'");
		update_values.push_back(columns[49] + " = " + std::to_string(npc_types_entry.prim_melee_type));
		update_values.push_back(columns[50] + " = " + std::to_string(npc_types_entry.sec_melee_type));
		update_values.push_back(columns[51] + " = " + std::to_string(npc_types_entry.ranged_type));
		update_values.push_back(columns[52] + " = " + std::to_string(npc_types_entry.runspeed));
		update_values.push_back(columns[53] + " = " + std::to_string(npc_types_entry.MR));
		update_values.push_back(columns[54] + " = " + std::to_string(npc_types_entry.CR));
		update_values.push_back(columns[55] + " = " + std::to_string(npc_types_entry.DR));
		update_values.push_back(columns[56] + " = " + std::to_string(npc_types_entry.FR));
		update_values.push_back(columns[57] + " = " + std::to_string(npc_types_entry.PR));
		update_values.push_back(columns[58] + " = " + std::to_string(npc_types_entry.Corrup));
		update_values.push_back(columns[59] + " = " + std::to_string(npc_types_entry.PhR));
		update_values.push_back(columns[60] + " = " + std::to_string(npc_types_entry.see_invis));
		update_values.push_back(columns[61] + " = " + std::to_string(npc_types_entry.see_invis_undead));
		update_values.push_back(columns[62] + " = " + std::to_string(npc_types_entry.qglobal));
		update_values.push_back(columns[63] + " = " + std::to_string(npc_types_entry.AC));
		update_values.push_back(columns[64] + " = " + std::to_string(npc_types_entry.npc_aggro));
		update_values.push_back(columns[65] + " = " + std::to_string(npc_types_entry.spawn_limit));
		update_values.push_back(columns[66] + " = " + std::to_string(npc_types_entry.attack_speed));
		update_values.push_back(columns[67] + " = " + std::to_string(npc_types_entry.attack_delay));
		update_values.push_back(columns[68] + " = " + std::to_string(npc_types_entry.findable));
		update_values.push_back(columns[69] + " = " + std::to_string(npc_types_entry.STR));
		update_values.push_back(columns[70] + " = " + std::to_string(npc_types_entry.STA));
		update_values.push_back(columns[71] + " = " + std::to_string(npc_types_entry.DEX));
		update_values.push_back(columns[72] + " = " + std::to_string(npc_types_entry.AGI));
		update_values.push_back(columns[73] + " = " + std::to_string(npc_types_entry._INT));
		update_values.push_back(columns[74] + " = " + std::to_string(npc_types_entry.WIS));
		update_values.push_back(columns[75] + " = " + std::to_string(npc_types_entry.CHA));
		update_values.push_back(columns[76] + " = " + std::to_string(npc_types_entry.see_hide));
		update_values.push_back(columns[77] + " = " + std::to_string(npc_types_entry.see_improved_hide));
		update_values.push_back(columns[78] + " = " + std::to_string(npc_types_entry.trackable));
		update_values.push_back(columns[79] + " = " + std::to_string(npc_types_entry.isbot));
		update_values.push_back(columns[80] + " = " + std::to_string(npc_types_entry.exclude));
		update_values.push_back(columns[81] + " = " + std::to_string(npc_types_entry.ATK));
		update_values.push_back(columns[82] + " = " + std::to_string(npc_types_entry.Accuracy));
		update_values.push_back(columns[83] + " = " + std::to_string(npc_types_entry.Avoidance));
		update_values.push_back(columns[84] + " = " + std::to_string(npc_types_entry.slow_mitigation));
		update_values.push_back(columns[85] + " = " + std::to_string(npc_types_entry.version));
		update_values.push_back(columns[86] + " = " + std::to_string(npc_types_entry.maxlevel));
		update_values.push_back(columns[87] + " = " + std::to_string(npc_types_entry.scalerate));
		update_values.push_back(columns[88] + " = " + std::to_string(npc_types_entry.private_corpse));
		update_values.push_back(columns[89] + " = " + std::to_string(npc_types_entry.unique_spawn_by_name));
		update_values.push_back(columns[90] + " = " + std::to_string(npc_types_entry.underwater));
		update_values.push_back(columns[91] + " = " + std::to_string(npc_types_entry.isquest));
		update_values.push_back(columns[92] + " = " + std::to_string(npc_types_entry.emoteid));
		update_values.push_back(columns[93] + " = " + std::to_string(npc_types_entry.spellscale));
		update_values.push_back(columns[94] + " = " + std::to_string(npc_types_entry.healscale));
		update_values.push_back(columns[95] + " = " + std::to_string(npc_types_entry.no_target_hotkey));
		update_values.push_back(columns[96] + " = " + std::to_string(npc_types_entry.raid_target));
		update_values.push_back(columns[97] + " = " + std::to_string(npc_types_entry.armtexture));
		update_values.push_back(columns[98] + " = " + std::to_string(npc_types_entry.bracertexture));
		update_values.push_back(columns[99] + " = " + std::to_string(npc_types_entry.handtexture));
		update_values.push_back(columns[100] + " = " + std::to_string(npc_types_entry.legtexture));
		update_values.push_back(columns[101] + " = " + std::to_string(npc_types_entry.feettexture));
		update_values.push_back(columns[102] + " = " + std::to_string(npc_types_entry.light));
		update_values.push_back(columns[103] + " = " + std::to_string(npc_types_entry.walkspeed));
		update_values.push_back(columns[104] + " = " + std::to_string(npc_types_entry.peqid));
		update_values.push_back(columns[105] + " = " + std::to_string(npc_types_entry.unique_));
		update_values.push_back(columns[106] + " = " + std::to_string(npc_types_entry.fixed));
		update_values.push_back(columns[107] + " = " + std::to_string(npc_types_entry.ignore_despawn));
		update_values.push_back(columns[108] + " = " + std::to_string(npc_types_entry.show_name));
		update_values.push_back(columns[109] + " = " + std::to_string(npc_types_entry.untargetable));
		update_values.push_back(columns[110] + " = " + std::to_string(npc_types_entry.charm_ac));
		update_values.push_back(columns[111] + " = " + std::to_string(npc_types_entry.charm_min_dmg));
		update_values.push_back(columns[112] + " = " + std::to_string(npc_types_entry.charm_max_dmg));
		update_values.push_back(columns[113] + " = " + std::to_string(npc_types_entry.charm_attack_delay));
		update_values.push_back(columns[114] + " = " + std::to_string(npc_types_entry.charm_accuracy_rating));
		update_values.push_back(columns[115] + " = " + std::to_string(npc_types_entry.charm_avoidance_rating));
		update_values.push_back(columns[116] + " = " + std::to_string(npc_types_entry.charm_atk));
		update_values.push_back(columns[117] + " = " + std::to_string(npc_types_entry.skip_global_loot));
		update_values.push_back(columns[118] + " = " + std::to_string(npc_types_entry.rare_spawn));
		update_values.push_back(columns[119] + " = " + std::to_string(npc_types_entry.stuck_behavior));
		update_values.push_back(columns[120] + " = " + std::to_string(npc_types_entry.model));
		update_values.push_back(columns[121] + " = " + std::to_string(npc_types_entry.flymode));
		update_values.push_back(columns[122] + " = " + std::to_string(npc_types_entry.always_aggro));
		update_values.push_back(columns[123] + " = " + std::to_string(npc_types_entry.exp_mod));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				npc_types_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static NpcTypes InsertOne(
		Database& db,
		NpcTypes npc_types_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(npc_types_entry.id));
		insert_values.push_back("'" + EscapeString(npc_types_entry.name) + "'");
		insert_values.push_back("'" + EscapeString(npc_types_entry.lastname) + "'");
		insert_values.push_back(std::to_string(npc_types_entry.level));
		insert_values.push_back(std::to_string(npc_types_entry.race));
		insert_values.push_back(std::to_string(npc_types_entry.class_));
		insert_values.push_back(std::to_string(npc_types_entry.bodytype));
		insert_values.push_back(std::to_string(npc_types_entry.hp));
		insert_values.push_back(std::to_string(npc_types_entry.mana));
		insert_values.push_back(std::to_string(npc_types_entry.gender));
		insert_values.push_back(std::to_string(npc_types_entry.texture));
		insert_values.push_back(std::to_string(npc_types_entry.helmtexture));
		insert_values.push_back(std::to_string(npc_types_entry.herosforgemodel));
		insert_values.push_back(std::to_string(npc_types_entry.size));
		insert_values.push_back(std::to_string(npc_types_entry.hp_regen_rate));
		insert_values.push_back(std::to_string(npc_types_entry.hp_regen_per_second));
		insert_values.push_back(std::to_string(npc_types_entry.mana_regen_rate));
		insert_values.push_back(std::to_string(npc_types_entry.loottable_id));
		insert_values.push_back(std::to_string(npc_types_entry.merchant_id));
		insert_values.push_back(std::to_string(npc_types_entry.alt_currency_id));
		insert_values.push_back(std::to_string(npc_types_entry.npc_spells_id));
		insert_values.push_back(std::to_string(npc_types_entry.npc_spells_effects_id));
		insert_values.push_back(std::to_string(npc_types_entry.npc_faction_id));
		insert_values.push_back(std::to_string(npc_types_entry.adventure_template_id));
		insert_values.push_back(std::to_string(npc_types_entry.trap_template));
		insert_values.push_back(std::to_string(npc_types_entry.mindmg));
		insert_values.push_back(std::to_string(npc_types_entry.maxdmg));
		insert_values.push_back(std::to_string(npc_types_entry.attack_count));
		insert_values.push_back("'" + EscapeString(npc_types_entry.npcspecialattks) + "'");
		insert_values.push_back("'" + EscapeString(npc_types_entry.special_abilities) + "'");
		insert_values.push_back(std::to_string(npc_types_entry.aggroradius));
		insert_values.push_back(std::to_string(npc_types_entry.assistradius));
		insert_values.push_back(std::to_string(npc_types_entry.face));
		insert_values.push_back(std::to_string(npc_types_entry.luclin_hairstyle));
		insert_values.push_back(std::to_string(npc_types_entry.luclin_haircolor));
		insert_values.push_back(std::to_string(npc_types_entry.luclin_eyecolor));
		insert_values.push_back(std::to_string(npc_types_entry.luclin_eyecolor2));
		insert_values.push_back(std::to_string(npc_types_entry.luclin_beardcolor));
		insert_values.push_back(std::to_string(npc_types_entry.luclin_beard));
		insert_values.push_back(std::to_string(npc_types_entry.drakkin_heritage));
		insert_values.push_back(std::to_string(npc_types_entry.drakkin_tattoo));
		insert_values.push_back(std::to_string(npc_types_entry.drakkin_details));
		insert_values.push_back(std::to_string(npc_types_entry.armortint_id));
		insert_values.push_back(std::to_string(npc_types_entry.armortint_red));
		insert_values.push_back(std::to_string(npc_types_entry.armortint_green));
		insert_values.push_back(std::to_string(npc_types_entry.armortint_blue));
		insert_values.push_back(std::to_string(npc_types_entry.d_melee_texture1));
		insert_values.push_back(std::to_string(npc_types_entry.d_melee_texture2));
		insert_values.push_back("'" + EscapeString(npc_types_entry.ammo_idfile) + "'");
		insert_values.push_back(std::to_string(npc_types_entry.prim_melee_type));
		insert_values.push_back(std::to_string(npc_types_entry.sec_melee_type));
		insert_values.push_back(std::to_string(npc_types_entry.ranged_type));
		insert_values.push_back(std::to_string(npc_types_entry.runspeed));
		insert_values.push_back(std::to_string(npc_types_entry.MR));
		insert_values.push_back(std::to_string(npc_types_entry.CR));
		insert_values.push_back(std::to_string(npc_types_entry.DR));
		insert_values.push_back(std::to_string(npc_types_entry.FR));
		insert_values.push_back(std::to_string(npc_types_entry.PR));
		insert_values.push_back(std::to_string(npc_types_entry.Corrup));
		insert_values.push_back(std::to_string(npc_types_entry.PhR));
		insert_values.push_back(std::to_string(npc_types_entry.see_invis));
		insert_values.push_back(std::to_string(npc_types_entry.see_invis_undead));
		insert_values.push_back(std::to_string(npc_types_entry.qglobal));
		insert_values.push_back(std::to_string(npc_types_entry.AC));
		insert_values.push_back(std::to_string(npc_types_entry.npc_aggro));
		insert_values.push_back(std::to_string(npc_types_entry.spawn_limit));
		insert_values.push_back(std::to_string(npc_types_entry.attack_speed));
		insert_values.push_back(std::to_string(npc_types_entry.attack_delay));
		insert_values.push_back(std::to_string(npc_types_entry.findable));
		insert_values.push_back(std::to_string(npc_types_entry.STR));
		insert_values.push_back(std::to_string(npc_types_entry.STA));
		insert_values.push_back(std::to_string(npc_types_entry.DEX));
		insert_values.push_back(std::to_string(npc_types_entry.AGI));
		insert_values.push_back(std::to_string(npc_types_entry._INT));
		insert_values.push_back(std::to_string(npc_types_entry.WIS));
		insert_values.push_back(std::to_string(npc_types_entry.CHA));
		insert_values.push_back(std::to_string(npc_types_entry.see_hide));
		insert_values.push_back(std::to_string(npc_types_entry.see_improved_hide));
		insert_values.push_back(std::to_string(npc_types_entry.trackable));
		insert_values.push_back(std::to_string(npc_types_entry.isbot));
		insert_values.push_back(std::to_string(npc_types_entry.exclude));
		insert_values.push_back(std::to_string(npc_types_entry.ATK));
		insert_values.push_back(std::to_string(npc_types_entry.Accuracy));
		insert_values.push_back(std::to_string(npc_types_entry.Avoidance));
		insert_values.push_back(std::to_string(npc_types_entry.slow_mitigation));
		insert_values.push_back(std::to_string(npc_types_entry.version));
		insert_values.push_back(std::to_string(npc_types_entry.maxlevel));
		insert_values.push_back(std::to_string(npc_types_entry.scalerate));
		insert_values.push_back(std::to_string(npc_types_entry.private_corpse));
		insert_values.push_back(std::to_string(npc_types_entry.unique_spawn_by_name));
		insert_values.push_back(std::to_string(npc_types_entry.underwater));
		insert_values.push_back(std::to_string(npc_types_entry.isquest));
		insert_values.push_back(std::to_string(npc_types_entry.emoteid));
		insert_values.push_back(std::to_string(npc_types_entry.spellscale));
		insert_values.push_back(std::to_string(npc_types_entry.healscale));
		insert_values.push_back(std::to_string(npc_types_entry.no_target_hotkey));
		insert_values.push_back(std::to_string(npc_types_entry.raid_target));
		insert_values.push_back(std::to_string(npc_types_entry.armtexture));
		insert_values.push_back(std::to_string(npc_types_entry.bracertexture));
		insert_values.push_back(std::to_string(npc_types_entry.handtexture));
		insert_values.push_back(std::to_string(npc_types_entry.legtexture));
		insert_values.push_back(std::to_string(npc_types_entry.feettexture));
		insert_values.push_back(std::to_string(npc_types_entry.light));
		insert_values.push_back(std::to_string(npc_types_entry.walkspeed));
		insert_values.push_back(std::to_string(npc_types_entry.peqid));
		insert_values.push_back(std::to_string(npc_types_entry.unique_));
		insert_values.push_back(std::to_string(npc_types_entry.fixed));
		insert_values.push_back(std::to_string(npc_types_entry.ignore_despawn));
		insert_values.push_back(std::to_string(npc_types_entry.show_name));
		insert_values.push_back(std::to_string(npc_types_entry.untargetable));
		insert_values.push_back(std::to_string(npc_types_entry.charm_ac));
		insert_values.push_back(std::to_string(npc_types_entry.charm_min_dmg));
		insert_values.push_back(std::to_string(npc_types_entry.charm_max_dmg));
		insert_values.push_back(std::to_string(npc_types_entry.charm_attack_delay));
		insert_values.push_back(std::to_string(npc_types_entry.charm_accuracy_rating));
		insert_values.push_back(std::to_string(npc_types_entry.charm_avoidance_rating));
		insert_values.push_back(std::to_string(npc_types_entry.charm_atk));
		insert_values.push_back(std::to_string(npc_types_entry.skip_global_loot));
		insert_values.push_back(std::to_string(npc_types_entry.rare_spawn));
		insert_values.push_back(std::to_string(npc_types_entry.stuck_behavior));
		insert_values.push_back(std::to_string(npc_types_entry.model));
		insert_values.push_back(std::to_string(npc_types_entry.flymode));
		insert_values.push_back(std::to_string(npc_types_entry.always_aggro));
		insert_values.push_back(std::to_string(npc_types_entry.exp_mod));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			npc_types_entry.id = results.LastInsertedID();
			return npc_types_entry;
		}

		npc_types_entry = NewEntity();

		return npc_types_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<NpcTypes> npc_types_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &npc_types_entry: npc_types_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(npc_types_entry.id));
			insert_values.push_back("'" + EscapeString(npc_types_entry.name) + "'");
			insert_values.push_back("'" + EscapeString(npc_types_entry.lastname) + "'");
			insert_values.push_back(std::to_string(npc_types_entry.level));
			insert_values.push_back(std::to_string(npc_types_entry.race));
			insert_values.push_back(std::to_string(npc_types_entry.class_));
			insert_values.push_back(std::to_string(npc_types_entry.bodytype));
			insert_values.push_back(std::to_string(npc_types_entry.hp));
			insert_values.push_back(std::to_string(npc_types_entry.mana));
			insert_values.push_back(std::to_string(npc_types_entry.gender));
			insert_values.push_back(std::to_string(npc_types_entry.texture));
			insert_values.push_back(std::to_string(npc_types_entry.helmtexture));
			insert_values.push_back(std::to_string(npc_types_entry.herosforgemodel));
			insert_values.push_back(std::to_string(npc_types_entry.size));
			insert_values.push_back(std::to_string(npc_types_entry.hp_regen_rate));
			insert_values.push_back(std::to_string(npc_types_entry.hp_regen_per_second));
			insert_values.push_back(std::to_string(npc_types_entry.mana_regen_rate));
			insert_values.push_back(std::to_string(npc_types_entry.loottable_id));
			insert_values.push_back(std::to_string(npc_types_entry.merchant_id));
			insert_values.push_back(std::to_string(npc_types_entry.alt_currency_id));
			insert_values.push_back(std::to_string(npc_types_entry.npc_spells_id));
			insert_values.push_back(std::to_string(npc_types_entry.npc_spells_effects_id));
			insert_values.push_back(std::to_string(npc_types_entry.npc_faction_id));
			insert_values.push_back(std::to_string(npc_types_entry.adventure_template_id));
			insert_values.push_back(std::to_string(npc_types_entry.trap_template));
			insert_values.push_back(std::to_string(npc_types_entry.mindmg));
			insert_values.push_back(std::to_string(npc_types_entry.maxdmg));
			insert_values.push_back(std::to_string(npc_types_entry.attack_count));
			insert_values.push_back("'" + EscapeString(npc_types_entry.npcspecialattks) + "'");
			insert_values.push_back("'" + EscapeString(npc_types_entry.special_abilities) + "'");
			insert_values.push_back(std::to_string(npc_types_entry.aggroradius));
			insert_values.push_back(std::to_string(npc_types_entry.assistradius));
			insert_values.push_back(std::to_string(npc_types_entry.face));
			insert_values.push_back(std::to_string(npc_types_entry.luclin_hairstyle));
			insert_values.push_back(std::to_string(npc_types_entry.luclin_haircolor));
			insert_values.push_back(std::to_string(npc_types_entry.luclin_eyecolor));
			insert_values.push_back(std::to_string(npc_types_entry.luclin_eyecolor2));
			insert_values.push_back(std::to_string(npc_types_entry.luclin_beardcolor));
			insert_values.push_back(std::to_string(npc_types_entry.luclin_beard));
			insert_values.push_back(std::to_string(npc_types_entry.drakkin_heritage));
			insert_values.push_back(std::to_string(npc_types_entry.drakkin_tattoo));
			insert_values.push_back(std::to_string(npc_types_entry.drakkin_details));
			insert_values.push_back(std::to_string(npc_types_entry.armortint_id));
			insert_values.push_back(std::to_string(npc_types_entry.armortint_red));
			insert_values.push_back(std::to_string(npc_types_entry.armortint_green));
			insert_values.push_back(std::to_string(npc_types_entry.armortint_blue));
			insert_values.push_back(std::to_string(npc_types_entry.d_melee_texture1));
			insert_values.push_back(std::to_string(npc_types_entry.d_melee_texture2));
			insert_values.push_back("'" + EscapeString(npc_types_entry.ammo_idfile) + "'");
			insert_values.push_back(std::to_string(npc_types_entry.prim_melee_type));
			insert_values.push_back(std::to_string(npc_types_entry.sec_melee_type));
			insert_values.push_back(std::to_string(npc_types_entry.ranged_type));
			insert_values.push_back(std::to_string(npc_types_entry.runspeed));
			insert_values.push_back(std::to_string(npc_types_entry.MR));
			insert_values.push_back(std::to_string(npc_types_entry.CR));
			insert_values.push_back(std::to_string(npc_types_entry.DR));
			insert_values.push_back(std::to_string(npc_types_entry.FR));
			insert_values.push_back(std::to_string(npc_types_entry.PR));
			insert_values.push_back(std::to_string(npc_types_entry.Corrup));
			insert_values.push_back(std::to_string(npc_types_entry.PhR));
			insert_values.push_back(std::to_string(npc_types_entry.see_invis));
			insert_values.push_back(std::to_string(npc_types_entry.see_invis_undead));
			insert_values.push_back(std::to_string(npc_types_entry.qglobal));
			insert_values.push_back(std::to_string(npc_types_entry.AC));
			insert_values.push_back(std::to_string(npc_types_entry.npc_aggro));
			insert_values.push_back(std::to_string(npc_types_entry.spawn_limit));
			insert_values.push_back(std::to_string(npc_types_entry.attack_speed));
			insert_values.push_back(std::to_string(npc_types_entry.attack_delay));
			insert_values.push_back(std::to_string(npc_types_entry.findable));
			insert_values.push_back(std::to_string(npc_types_entry.STR));
			insert_values.push_back(std::to_string(npc_types_entry.STA));
			insert_values.push_back(std::to_string(npc_types_entry.DEX));
			insert_values.push_back(std::to_string(npc_types_entry.AGI));
			insert_values.push_back(std::to_string(npc_types_entry._INT));
			insert_values.push_back(std::to_string(npc_types_entry.WIS));
			insert_values.push_back(std::to_string(npc_types_entry.CHA));
			insert_values.push_back(std::to_string(npc_types_entry.see_hide));
			insert_values.push_back(std::to_string(npc_types_entry.see_improved_hide));
			insert_values.push_back(std::to_string(npc_types_entry.trackable));
			insert_values.push_back(std::to_string(npc_types_entry.isbot));
			insert_values.push_back(std::to_string(npc_types_entry.exclude));
			insert_values.push_back(std::to_string(npc_types_entry.ATK));
			insert_values.push_back(std::to_string(npc_types_entry.Accuracy));
			insert_values.push_back(std::to_string(npc_types_entry.Avoidance));
			insert_values.push_back(std::to_string(npc_types_entry.slow_mitigation));
			insert_values.push_back(std::to_string(npc_types_entry.version));
			insert_values.push_back(std::to_string(npc_types_entry.maxlevel));
			insert_values.push_back(std::to_string(npc_types_entry.scalerate));
			insert_values.push_back(std::to_string(npc_types_entry.private_corpse));
			insert_values.push_back(std::to_string(npc_types_entry.unique_spawn_by_name));
			insert_values.push_back(std::to_string(npc_types_entry.underwater));
			insert_values.push_back(std::to_string(npc_types_entry.isquest));
			insert_values.push_back(std::to_string(npc_types_entry.emoteid));
			insert_values.push_back(std::to_string(npc_types_entry.spellscale));
			insert_values.push_back(std::to_string(npc_types_entry.healscale));
			insert_values.push_back(std::to_string(npc_types_entry.no_target_hotkey));
			insert_values.push_back(std::to_string(npc_types_entry.raid_target));
			insert_values.push_back(std::to_string(npc_types_entry.armtexture));
			insert_values.push_back(std::to_string(npc_types_entry.bracertexture));
			insert_values.push_back(std::to_string(npc_types_entry.handtexture));
			insert_values.push_back(std::to_string(npc_types_entry.legtexture));
			insert_values.push_back(std::to_string(npc_types_entry.feettexture));
			insert_values.push_back(std::to_string(npc_types_entry.light));
			insert_values.push_back(std::to_string(npc_types_entry.walkspeed));
			insert_values.push_back(std::to_string(npc_types_entry.peqid));
			insert_values.push_back(std::to_string(npc_types_entry.unique_));
			insert_values.push_back(std::to_string(npc_types_entry.fixed));
			insert_values.push_back(std::to_string(npc_types_entry.ignore_despawn));
			insert_values.push_back(std::to_string(npc_types_entry.show_name));
			insert_values.push_back(std::to_string(npc_types_entry.untargetable));
			insert_values.push_back(std::to_string(npc_types_entry.charm_ac));
			insert_values.push_back(std::to_string(npc_types_entry.charm_min_dmg));
			insert_values.push_back(std::to_string(npc_types_entry.charm_max_dmg));
			insert_values.push_back(std::to_string(npc_types_entry.charm_attack_delay));
			insert_values.push_back(std::to_string(npc_types_entry.charm_accuracy_rating));
			insert_values.push_back(std::to_string(npc_types_entry.charm_avoidance_rating));
			insert_values.push_back(std::to_string(npc_types_entry.charm_atk));
			insert_values.push_back(std::to_string(npc_types_entry.skip_global_loot));
			insert_values.push_back(std::to_string(npc_types_entry.rare_spawn));
			insert_values.push_back(std::to_string(npc_types_entry.stuck_behavior));
			insert_values.push_back(std::to_string(npc_types_entry.model));
			insert_values.push_back(std::to_string(npc_types_entry.flymode));
			insert_values.push_back(std::to_string(npc_types_entry.always_aggro));
			insert_values.push_back(std::to_string(npc_types_entry.exp_mod));

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

	static std::vector<NpcTypes> All(Database& db)
	{
		std::vector<NpcTypes> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcTypes entry{};

			entry.id                     = atoi(row[0]);
			entry.name                   = row[1] ? row[1] : "";
			entry.lastname               = row[2] ? row[2] : "";
			entry.level                  = atoi(row[3]);
			entry.race                   = atoi(row[4]);
			entry.class_                 = atoi(row[5]);
			entry.bodytype               = atoi(row[6]);
			entry.hp                     = strtoll(row[7], nullptr, 10);
			entry.mana                   = strtoll(row[8], nullptr, 10);
			entry.gender                 = atoi(row[9]);
			entry.texture                = atoi(row[10]);
			entry.helmtexture            = atoi(row[11]);
			entry.herosforgemodel        = atoi(row[12]);
			entry.size                   = static_cast<float>(atof(row[13]));
			entry.hp_regen_rate          = strtoll(row[14], nullptr, 10);
			entry.hp_regen_per_second    = strtoll(row[15], nullptr, 10);
			entry.mana_regen_rate        = strtoll(row[16], nullptr, 10);
			entry.loottable_id           = atoi(row[17]);
			entry.merchant_id            = atoi(row[18]);
			entry.alt_currency_id        = atoi(row[19]);
			entry.npc_spells_id          = atoi(row[20]);
			entry.npc_spells_effects_id  = atoi(row[21]);
			entry.npc_faction_id         = atoi(row[22]);
			entry.adventure_template_id  = atoi(row[23]);
			entry.trap_template          = atoi(row[24]);
			entry.mindmg                 = atoi(row[25]);
			entry.maxdmg                 = atoi(row[26]);
			entry.attack_count           = atoi(row[27]);
			entry.npcspecialattks        = row[28] ? row[28] : "";
			entry.special_abilities      = row[29] ? row[29] : "";
			entry.aggroradius            = atoi(row[30]);
			entry.assistradius           = atoi(row[31]);
			entry.face                   = atoi(row[32]);
			entry.luclin_hairstyle       = atoi(row[33]);
			entry.luclin_haircolor       = atoi(row[34]);
			entry.luclin_eyecolor        = atoi(row[35]);
			entry.luclin_eyecolor2       = atoi(row[36]);
			entry.luclin_beardcolor      = atoi(row[37]);
			entry.luclin_beard           = atoi(row[38]);
			entry.drakkin_heritage       = atoi(row[39]);
			entry.drakkin_tattoo         = atoi(row[40]);
			entry.drakkin_details        = atoi(row[41]);
			entry.armortint_id           = atoi(row[42]);
			entry.armortint_red          = atoi(row[43]);
			entry.armortint_green        = atoi(row[44]);
			entry.armortint_blue         = atoi(row[45]);
			entry.d_melee_texture1       = atoi(row[46]);
			entry.d_melee_texture2       = atoi(row[47]);
			entry.ammo_idfile            = row[48] ? row[48] : "";
			entry.prim_melee_type        = atoi(row[49]);
			entry.sec_melee_type         = atoi(row[50]);
			entry.ranged_type            = atoi(row[51]);
			entry.runspeed               = static_cast<float>(atof(row[52]));
			entry.MR                     = atoi(row[53]);
			entry.CR                     = atoi(row[54]);
			entry.DR                     = atoi(row[55]);
			entry.FR                     = atoi(row[56]);
			entry.PR                     = atoi(row[57]);
			entry.Corrup                 = atoi(row[58]);
			entry.PhR                    = atoi(row[59]);
			entry.see_invis              = atoi(row[60]);
			entry.see_invis_undead       = atoi(row[61]);
			entry.qglobal                = atoi(row[62]);
			entry.AC                     = atoi(row[63]);
			entry.npc_aggro              = atoi(row[64]);
			entry.spawn_limit            = atoi(row[65]);
			entry.attack_speed           = static_cast<float>(atof(row[66]));
			entry.attack_delay           = atoi(row[67]);
			entry.findable               = atoi(row[68]);
			entry.STR                    = atoi(row[69]);
			entry.STA                    = atoi(row[70]);
			entry.DEX                    = atoi(row[71]);
			entry.AGI                    = atoi(row[72]);
			entry._INT                   = atoi(row[73]);
			entry.WIS                    = atoi(row[74]);
			entry.CHA                    = atoi(row[75]);
			entry.see_hide               = atoi(row[76]);
			entry.see_improved_hide      = atoi(row[77]);
			entry.trackable              = atoi(row[78]);
			entry.isbot                  = atoi(row[79]);
			entry.exclude                = atoi(row[80]);
			entry.ATK                    = atoi(row[81]);
			entry.Accuracy               = atoi(row[82]);
			entry.Avoidance              = atoi(row[83]);
			entry.slow_mitigation        = atoi(row[84]);
			entry.version                = atoi(row[85]);
			entry.maxlevel               = atoi(row[86]);
			entry.scalerate              = atoi(row[87]);
			entry.private_corpse         = atoi(row[88]);
			entry.unique_spawn_by_name   = atoi(row[89]);
			entry.underwater             = atoi(row[90]);
			entry.isquest                = atoi(row[91]);
			entry.emoteid                = atoi(row[92]);
			entry.spellscale             = static_cast<float>(atof(row[93]));
			entry.healscale              = static_cast<float>(atof(row[94]));
			entry.no_target_hotkey       = atoi(row[95]);
			entry.raid_target            = atoi(row[96]);
			entry.armtexture             = atoi(row[97]);
			entry.bracertexture          = atoi(row[98]);
			entry.handtexture            = atoi(row[99]);
			entry.legtexture             = atoi(row[100]);
			entry.feettexture            = atoi(row[101]);
			entry.light                  = atoi(row[102]);
			entry.walkspeed              = atoi(row[103]);
			entry.peqid                  = atoi(row[104]);
			entry.unique_                = atoi(row[105]);
			entry.fixed                  = atoi(row[106]);
			entry.ignore_despawn         = atoi(row[107]);
			entry.show_name              = atoi(row[108]);
			entry.untargetable           = atoi(row[109]);
			entry.charm_ac               = atoi(row[110]);
			entry.charm_min_dmg          = atoi(row[111]);
			entry.charm_max_dmg          = atoi(row[112]);
			entry.charm_attack_delay     = atoi(row[113]);
			entry.charm_accuracy_rating  = atoi(row[114]);
			entry.charm_avoidance_rating = atoi(row[115]);
			entry.charm_atk              = atoi(row[116]);
			entry.skip_global_loot       = atoi(row[117]);
			entry.rare_spawn             = atoi(row[118]);
			entry.stuck_behavior         = atoi(row[119]);
			entry.model                  = atoi(row[120]);
			entry.flymode                = atoi(row[121]);
			entry.always_aggro           = atoi(row[122]);
			entry.exp_mod                = atoi(row[123]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<NpcTypes> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<NpcTypes> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcTypes entry{};

			entry.id                     = atoi(row[0]);
			entry.name                   = row[1] ? row[1] : "";
			entry.lastname               = row[2] ? row[2] : "";
			entry.level                  = atoi(row[3]);
			entry.race                   = atoi(row[4]);
			entry.class_                 = atoi(row[5]);
			entry.bodytype               = atoi(row[6]);
			entry.hp                     = strtoll(row[7], nullptr, 10);
			entry.mana                   = strtoll(row[8], nullptr, 10);
			entry.gender                 = atoi(row[9]);
			entry.texture                = atoi(row[10]);
			entry.helmtexture            = atoi(row[11]);
			entry.herosforgemodel        = atoi(row[12]);
			entry.size                   = static_cast<float>(atof(row[13]));
			entry.hp_regen_rate          = strtoll(row[14], nullptr, 10);
			entry.hp_regen_per_second    = strtoll(row[15], nullptr, 10);
			entry.mana_regen_rate        = strtoll(row[16], nullptr, 10);
			entry.loottable_id           = atoi(row[17]);
			entry.merchant_id            = atoi(row[18]);
			entry.alt_currency_id        = atoi(row[19]);
			entry.npc_spells_id          = atoi(row[20]);
			entry.npc_spells_effects_id  = atoi(row[21]);
			entry.npc_faction_id         = atoi(row[22]);
			entry.adventure_template_id  = atoi(row[23]);
			entry.trap_template          = atoi(row[24]);
			entry.mindmg                 = atoi(row[25]);
			entry.maxdmg                 = atoi(row[26]);
			entry.attack_count           = atoi(row[27]);
			entry.npcspecialattks        = row[28] ? row[28] : "";
			entry.special_abilities      = row[29] ? row[29] : "";
			entry.aggroradius            = atoi(row[30]);
			entry.assistradius           = atoi(row[31]);
			entry.face                   = atoi(row[32]);
			entry.luclin_hairstyle       = atoi(row[33]);
			entry.luclin_haircolor       = atoi(row[34]);
			entry.luclin_eyecolor        = atoi(row[35]);
			entry.luclin_eyecolor2       = atoi(row[36]);
			entry.luclin_beardcolor      = atoi(row[37]);
			entry.luclin_beard           = atoi(row[38]);
			entry.drakkin_heritage       = atoi(row[39]);
			entry.drakkin_tattoo         = atoi(row[40]);
			entry.drakkin_details        = atoi(row[41]);
			entry.armortint_id           = atoi(row[42]);
			entry.armortint_red          = atoi(row[43]);
			entry.armortint_green        = atoi(row[44]);
			entry.armortint_blue         = atoi(row[45]);
			entry.d_melee_texture1       = atoi(row[46]);
			entry.d_melee_texture2       = atoi(row[47]);
			entry.ammo_idfile            = row[48] ? row[48] : "";
			entry.prim_melee_type        = atoi(row[49]);
			entry.sec_melee_type         = atoi(row[50]);
			entry.ranged_type            = atoi(row[51]);
			entry.runspeed               = static_cast<float>(atof(row[52]));
			entry.MR                     = atoi(row[53]);
			entry.CR                     = atoi(row[54]);
			entry.DR                     = atoi(row[55]);
			entry.FR                     = atoi(row[56]);
			entry.PR                     = atoi(row[57]);
			entry.Corrup                 = atoi(row[58]);
			entry.PhR                    = atoi(row[59]);
			entry.see_invis              = atoi(row[60]);
			entry.see_invis_undead       = atoi(row[61]);
			entry.qglobal                = atoi(row[62]);
			entry.AC                     = atoi(row[63]);
			entry.npc_aggro              = atoi(row[64]);
			entry.spawn_limit            = atoi(row[65]);
			entry.attack_speed           = static_cast<float>(atof(row[66]));
			entry.attack_delay           = atoi(row[67]);
			entry.findable               = atoi(row[68]);
			entry.STR                    = atoi(row[69]);
			entry.STA                    = atoi(row[70]);
			entry.DEX                    = atoi(row[71]);
			entry.AGI                    = atoi(row[72]);
			entry._INT                   = atoi(row[73]);
			entry.WIS                    = atoi(row[74]);
			entry.CHA                    = atoi(row[75]);
			entry.see_hide               = atoi(row[76]);
			entry.see_improved_hide      = atoi(row[77]);
			entry.trackable              = atoi(row[78]);
			entry.isbot                  = atoi(row[79]);
			entry.exclude                = atoi(row[80]);
			entry.ATK                    = atoi(row[81]);
			entry.Accuracy               = atoi(row[82]);
			entry.Avoidance              = atoi(row[83]);
			entry.slow_mitigation        = atoi(row[84]);
			entry.version                = atoi(row[85]);
			entry.maxlevel               = atoi(row[86]);
			entry.scalerate              = atoi(row[87]);
			entry.private_corpse         = atoi(row[88]);
			entry.unique_spawn_by_name   = atoi(row[89]);
			entry.underwater             = atoi(row[90]);
			entry.isquest                = atoi(row[91]);
			entry.emoteid                = atoi(row[92]);
			entry.spellscale             = static_cast<float>(atof(row[93]));
			entry.healscale              = static_cast<float>(atof(row[94]));
			entry.no_target_hotkey       = atoi(row[95]);
			entry.raid_target            = atoi(row[96]);
			entry.armtexture             = atoi(row[97]);
			entry.bracertexture          = atoi(row[98]);
			entry.handtexture            = atoi(row[99]);
			entry.legtexture             = atoi(row[100]);
			entry.feettexture            = atoi(row[101]);
			entry.light                  = atoi(row[102]);
			entry.walkspeed              = atoi(row[103]);
			entry.peqid                  = atoi(row[104]);
			entry.unique_                = atoi(row[105]);
			entry.fixed                  = atoi(row[106]);
			entry.ignore_despawn         = atoi(row[107]);
			entry.show_name              = atoi(row[108]);
			entry.untargetable           = atoi(row[109]);
			entry.charm_ac               = atoi(row[110]);
			entry.charm_min_dmg          = atoi(row[111]);
			entry.charm_max_dmg          = atoi(row[112]);
			entry.charm_attack_delay     = atoi(row[113]);
			entry.charm_accuracy_rating  = atoi(row[114]);
			entry.charm_avoidance_rating = atoi(row[115]);
			entry.charm_atk              = atoi(row[116]);
			entry.skip_global_loot       = atoi(row[117]);
			entry.rare_spawn             = atoi(row[118]);
			entry.stuck_behavior         = atoi(row[119]);
			entry.model                  = atoi(row[120]);
			entry.flymode                = atoi(row[121]);
			entry.always_aggro           = atoi(row[122]);
			entry.exp_mod                = atoi(row[123]);

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

#endif //EQEMU_BASE_NPC_TYPES_REPOSITORY_H
