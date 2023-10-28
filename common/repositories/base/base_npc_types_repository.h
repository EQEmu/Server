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
#include "../../strings.h"
#include <ctime>


class BaseNpcTypesRepository {
public:
	struct NpcTypes {
		int32_t     id;
		std::string name;
		std::string lastname;
		uint8_t     level;
		uint16_t    race;
		uint8_t     class_;
		int32_t     bodytype;
		int64_t     hp;
		int64_t     mana;
		uint8_t     gender;
		uint8_t     texture;
		uint8_t     helmtexture;
		int32_t     herosforgemodel;
		float       size;
		int64_t     hp_regen_rate;
		int64_t     hp_regen_per_second;
		int64_t     mana_regen_rate;
		uint32_t    loottable_id;
		uint32_t    merchant_id;
		uint32_t    alt_currency_id;
		uint32_t    npc_spells_id;
		uint32_t    npc_spells_effects_id;
		int32_t     npc_faction_id;
		uint32_t    adventure_template_id;
		uint32_t    trap_template;
		uint32_t    mindmg;
		uint32_t    maxdmg;
		int16_t     attack_count;
		std::string npcspecialattks;
		std::string special_abilities;
		uint32_t    aggroradius;
		uint32_t    assistradius;
		uint32_t    face;
		uint32_t    luclin_hairstyle;
		uint32_t    luclin_haircolor;
		uint32_t    luclin_eyecolor;
		uint32_t    luclin_eyecolor2;
		uint32_t    luclin_beardcolor;
		uint32_t    luclin_beard;
		int32_t     drakkin_heritage;
		int32_t     drakkin_tattoo;
		int32_t     drakkin_details;
		uint32_t    armortint_id;
		uint8_t     armortint_red;
		uint8_t     armortint_green;
		uint8_t     armortint_blue;
		uint32_t    d_melee_texture1;
		uint32_t    d_melee_texture2;
		std::string ammo_idfile;
		uint8_t     prim_melee_type;
		uint8_t     sec_melee_type;
		uint8_t     ranged_type;
		float       runspeed;
		int16_t     MR;
		int16_t     CR;
		int16_t     DR;
		int16_t     FR;
		int16_t     PR;
		int16_t     Corrup;
		uint16_t    PhR;
		int16_t     see_invis;
		int16_t     see_invis_undead;
		uint32_t    qglobal;
		int16_t     AC;
		int8_t      npc_aggro;
		int8_t      spawn_limit;
		float       attack_speed;
		uint8_t     attack_delay;
		int8_t      findable;
		uint32_t    STR;
		uint32_t    STA;
		uint32_t    DEX;
		uint32_t    AGI;
		uint32_t    _INT;
		uint32_t    WIS;
		uint32_t    CHA;
		int8_t      see_hide;
		int8_t      see_improved_hide;
		int8_t      trackable;
		int8_t      isbot;
		int8_t      exclude;
		int32_t     ATK;
		int32_t     Accuracy;
		uint32_t    Avoidance;
		int16_t     slow_mitigation;
		uint16_t    version;
		int8_t      maxlevel;
		int32_t     scalerate;
		uint8_t     private_corpse;
		uint8_t     unique_spawn_by_name;
		uint8_t     underwater;
		int8_t      isquest;
		uint32_t    emoteid;
		float       spellscale;
		float       healscale;
		uint8_t     no_target_hotkey;
		uint8_t     raid_target;
		int8_t      armtexture;
		int8_t      bracertexture;
		int8_t      handtexture;
		int8_t      legtexture;
		int8_t      feettexture;
		int8_t      light;
		int8_t      walkspeed;
		int32_t     peqid;
		int8_t      unique_;
		int8_t      fixed;
		int8_t      ignore_despawn;
		int8_t      show_name;
		int8_t      untargetable;
		int16_t     charm_ac;
		int32_t     charm_min_dmg;
		int32_t     charm_max_dmg;
		int8_t      charm_attack_delay;
		int32_t     charm_accuracy_rating;
		int32_t     charm_avoidance_rating;
		int32_t     charm_atk;
		int8_t      skip_global_loot;
		int8_t      rare_spawn;
		int8_t      stuck_behavior;
		int16_t     model;
		int8_t      flymode;
		int8_t      always_aggro;
		int32_t     exp_mod;
		int32_t     heroic_strikethrough;
		int32_t     faction_amount;
		uint8_t     keeps_sold_items;
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
			"heroic_strikethrough",
			"faction_amount",
			"keeps_sold_items",
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
			"heroic_strikethrough",
			"faction_amount",
			"keeps_sold_items",
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
		NpcTypes e{};

		e.id                     = 0;
		e.name                   = "";
		e.lastname               = "";
		e.level                  = 0;
		e.race                   = 0;
		e.class_                 = 0;
		e.bodytype               = 1;
		e.hp                     = 0;
		e.mana                   = 0;
		e.gender                 = 0;
		e.texture                = 0;
		e.helmtexture            = 0;
		e.herosforgemodel        = 0;
		e.size                   = 0;
		e.hp_regen_rate          = 0;
		e.hp_regen_per_second    = 0;
		e.mana_regen_rate        = 0;
		e.loottable_id           = 0;
		e.merchant_id            = 0;
		e.alt_currency_id        = 0;
		e.npc_spells_id          = 0;
		e.npc_spells_effects_id  = 0;
		e.npc_faction_id         = 0;
		e.adventure_template_id  = 0;
		e.trap_template          = 0;
		e.mindmg                 = 0;
		e.maxdmg                 = 0;
		e.attack_count           = -1;
		e.npcspecialattks        = "";
		e.special_abilities      = "";
		e.aggroradius            = 0;
		e.assistradius           = 0;
		e.face                   = 1;
		e.luclin_hairstyle       = 1;
		e.luclin_haircolor       = 1;
		e.luclin_eyecolor        = 1;
		e.luclin_eyecolor2       = 1;
		e.luclin_beardcolor      = 1;
		e.luclin_beard           = 0;
		e.drakkin_heritage       = 0;
		e.drakkin_tattoo         = 0;
		e.drakkin_details        = 0;
		e.armortint_id           = 0;
		e.armortint_red          = 0;
		e.armortint_green        = 0;
		e.armortint_blue         = 0;
		e.d_melee_texture1       = 0;
		e.d_melee_texture2       = 0;
		e.ammo_idfile            = "IT10";
		e.prim_melee_type        = 28;
		e.sec_melee_type         = 28;
		e.ranged_type            = 7;
		e.runspeed               = 0;
		e.MR                     = 0;
		e.CR                     = 0;
		e.DR                     = 0;
		e.FR                     = 0;
		e.PR                     = 0;
		e.Corrup                 = 0;
		e.PhR                    = 0;
		e.see_invis              = 0;
		e.see_invis_undead       = 0;
		e.qglobal                = 0;
		e.AC                     = 0;
		e.npc_aggro              = 0;
		e.spawn_limit            = 0;
		e.attack_speed           = 0;
		e.attack_delay           = 30;
		e.findable               = 0;
		e.STR                    = 75;
		e.STA                    = 75;
		e.DEX                    = 75;
		e.AGI                    = 75;
		e._INT                   = 80;
		e.WIS                    = 75;
		e.CHA                    = 75;
		e.see_hide               = 0;
		e.see_improved_hide      = 0;
		e.trackable              = 1;
		e.isbot                  = 0;
		e.exclude                = 1;
		e.ATK                    = 0;
		e.Accuracy               = 0;
		e.Avoidance              = 0;
		e.slow_mitigation        = 0;
		e.version                = 0;
		e.maxlevel               = 0;
		e.scalerate              = 100;
		e.private_corpse         = 0;
		e.unique_spawn_by_name   = 0;
		e.underwater             = 0;
		e.isquest                = 0;
		e.emoteid                = 0;
		e.spellscale             = 100;
		e.healscale              = 100;
		e.no_target_hotkey       = 0;
		e.raid_target            = 0;
		e.armtexture             = 0;
		e.bracertexture          = 0;
		e.handtexture            = 0;
		e.legtexture             = 0;
		e.feettexture            = 0;
		e.light                  = 0;
		e.walkspeed              = 0;
		e.peqid                  = 0;
		e.unique_                = 0;
		e.fixed                  = 0;
		e.ignore_despawn         = 0;
		e.show_name              = 1;
		e.untargetable           = 0;
		e.charm_ac               = 0;
		e.charm_min_dmg          = 0;
		e.charm_max_dmg          = 0;
		e.charm_attack_delay     = 0;
		e.charm_accuracy_rating  = 0;
		e.charm_avoidance_rating = 0;
		e.charm_atk              = 0;
		e.skip_global_loot       = 0;
		e.rare_spawn             = 0;
		e.stuck_behavior         = 0;
		e.model                  = 0;
		e.flymode                = -1;
		e.always_aggro           = 0;
		e.exp_mod                = 100;
		e.heroic_strikethrough   = 0;
		e.faction_amount         = 0;
		e.keeps_sold_items       = 1;

		return e;
	}

	static NpcTypes GetNpcTypes(
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
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				npc_types_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			NpcTypes e{};

			e.id                     = static_cast<int32_t>(atoi(row[0]));
			e.name                   = row[1] ? row[1] : "";
			e.lastname               = row[2] ? row[2] : "";
			e.level                  = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));
			e.race                   = static_cast<uint16_t>(strtoul(row[4], nullptr, 10));
			e.class_                 = static_cast<uint8_t>(strtoul(row[5], nullptr, 10));
			e.bodytype               = static_cast<int32_t>(atoi(row[6]));
			e.hp                     = strtoll(row[7], nullptr, 10);
			e.mana                   = strtoll(row[8], nullptr, 10);
			e.gender                 = static_cast<uint8_t>(strtoul(row[9], nullptr, 10));
			e.texture                = static_cast<uint8_t>(strtoul(row[10], nullptr, 10));
			e.helmtexture            = static_cast<uint8_t>(strtoul(row[11], nullptr, 10));
			e.herosforgemodel        = static_cast<int32_t>(atoi(row[12]));
			e.size                   = strtof(row[13], nullptr);
			e.hp_regen_rate          = strtoll(row[14], nullptr, 10);
			e.hp_regen_per_second    = strtoll(row[15], nullptr, 10);
			e.mana_regen_rate        = strtoll(row[16], nullptr, 10);
			e.loottable_id           = static_cast<uint32_t>(strtoul(row[17], nullptr, 10));
			e.merchant_id            = static_cast<uint32_t>(strtoul(row[18], nullptr, 10));
			e.alt_currency_id        = static_cast<uint32_t>(strtoul(row[19], nullptr, 10));
			e.npc_spells_id          = static_cast<uint32_t>(strtoul(row[20], nullptr, 10));
			e.npc_spells_effects_id  = static_cast<uint32_t>(strtoul(row[21], nullptr, 10));
			e.npc_faction_id         = static_cast<int32_t>(atoi(row[22]));
			e.adventure_template_id  = static_cast<uint32_t>(strtoul(row[23], nullptr, 10));
			e.trap_template          = static_cast<uint32_t>(strtoul(row[24], nullptr, 10));
			e.mindmg                 = static_cast<uint32_t>(strtoul(row[25], nullptr, 10));
			e.maxdmg                 = static_cast<uint32_t>(strtoul(row[26], nullptr, 10));
			e.attack_count           = static_cast<int16_t>(atoi(row[27]));
			e.npcspecialattks        = row[28] ? row[28] : "";
			e.special_abilities      = row[29] ? row[29] : "";
			e.aggroradius            = static_cast<uint32_t>(strtoul(row[30], nullptr, 10));
			e.assistradius           = static_cast<uint32_t>(strtoul(row[31], nullptr, 10));
			e.face                   = static_cast<uint32_t>(strtoul(row[32], nullptr, 10));
			e.luclin_hairstyle       = static_cast<uint32_t>(strtoul(row[33], nullptr, 10));
			e.luclin_haircolor       = static_cast<uint32_t>(strtoul(row[34], nullptr, 10));
			e.luclin_eyecolor        = static_cast<uint32_t>(strtoul(row[35], nullptr, 10));
			e.luclin_eyecolor2       = static_cast<uint32_t>(strtoul(row[36], nullptr, 10));
			e.luclin_beardcolor      = static_cast<uint32_t>(strtoul(row[37], nullptr, 10));
			e.luclin_beard           = static_cast<uint32_t>(strtoul(row[38], nullptr, 10));
			e.drakkin_heritage       = static_cast<int32_t>(atoi(row[39]));
			e.drakkin_tattoo         = static_cast<int32_t>(atoi(row[40]));
			e.drakkin_details        = static_cast<int32_t>(atoi(row[41]));
			e.armortint_id           = static_cast<uint32_t>(strtoul(row[42], nullptr, 10));
			e.armortint_red          = static_cast<uint8_t>(strtoul(row[43], nullptr, 10));
			e.armortint_green        = static_cast<uint8_t>(strtoul(row[44], nullptr, 10));
			e.armortint_blue         = static_cast<uint8_t>(strtoul(row[45], nullptr, 10));
			e.d_melee_texture1       = static_cast<uint32_t>(strtoul(row[46], nullptr, 10));
			e.d_melee_texture2       = static_cast<uint32_t>(strtoul(row[47], nullptr, 10));
			e.ammo_idfile            = row[48] ? row[48] : "";
			e.prim_melee_type        = static_cast<uint8_t>(strtoul(row[49], nullptr, 10));
			e.sec_melee_type         = static_cast<uint8_t>(strtoul(row[50], nullptr, 10));
			e.ranged_type            = static_cast<uint8_t>(strtoul(row[51], nullptr, 10));
			e.runspeed               = strtof(row[52], nullptr);
			e.MR                     = static_cast<int16_t>(atoi(row[53]));
			e.CR                     = static_cast<int16_t>(atoi(row[54]));
			e.DR                     = static_cast<int16_t>(atoi(row[55]));
			e.FR                     = static_cast<int16_t>(atoi(row[56]));
			e.PR                     = static_cast<int16_t>(atoi(row[57]));
			e.Corrup                 = static_cast<int16_t>(atoi(row[58]));
			e.PhR                    = static_cast<uint16_t>(strtoul(row[59], nullptr, 10));
			e.see_invis              = static_cast<int16_t>(atoi(row[60]));
			e.see_invis_undead       = static_cast<int16_t>(atoi(row[61]));
			e.qglobal                = static_cast<uint32_t>(strtoul(row[62], nullptr, 10));
			e.AC                     = static_cast<int16_t>(atoi(row[63]));
			e.npc_aggro              = static_cast<int8_t>(atoi(row[64]));
			e.spawn_limit            = static_cast<int8_t>(atoi(row[65]));
			e.attack_speed           = strtof(row[66], nullptr);
			e.attack_delay           = static_cast<uint8_t>(strtoul(row[67], nullptr, 10));
			e.findable               = static_cast<int8_t>(atoi(row[68]));
			e.STR                    = static_cast<uint32_t>(strtoul(row[69], nullptr, 10));
			e.STA                    = static_cast<uint32_t>(strtoul(row[70], nullptr, 10));
			e.DEX                    = static_cast<uint32_t>(strtoul(row[71], nullptr, 10));
			e.AGI                    = static_cast<uint32_t>(strtoul(row[72], nullptr, 10));
			e._INT                   = static_cast<uint32_t>(strtoul(row[73], nullptr, 10));
			e.WIS                    = static_cast<uint32_t>(strtoul(row[74], nullptr, 10));
			e.CHA                    = static_cast<uint32_t>(strtoul(row[75], nullptr, 10));
			e.see_hide               = static_cast<int8_t>(atoi(row[76]));
			e.see_improved_hide      = static_cast<int8_t>(atoi(row[77]));
			e.trackable              = static_cast<int8_t>(atoi(row[78]));
			e.isbot                  = static_cast<int8_t>(atoi(row[79]));
			e.exclude                = static_cast<int8_t>(atoi(row[80]));
			e.ATK                    = static_cast<int32_t>(atoi(row[81]));
			e.Accuracy               = static_cast<int32_t>(atoi(row[82]));
			e.Avoidance              = static_cast<uint32_t>(strtoul(row[83], nullptr, 10));
			e.slow_mitigation        = static_cast<int16_t>(atoi(row[84]));
			e.version                = static_cast<uint16_t>(strtoul(row[85], nullptr, 10));
			e.maxlevel               = static_cast<int8_t>(atoi(row[86]));
			e.scalerate              = static_cast<int32_t>(atoi(row[87]));
			e.private_corpse         = static_cast<uint8_t>(strtoul(row[88], nullptr, 10));
			e.unique_spawn_by_name   = static_cast<uint8_t>(strtoul(row[89], nullptr, 10));
			e.underwater             = static_cast<uint8_t>(strtoul(row[90], nullptr, 10));
			e.isquest                = static_cast<int8_t>(atoi(row[91]));
			e.emoteid                = static_cast<uint32_t>(strtoul(row[92], nullptr, 10));
			e.spellscale             = strtof(row[93], nullptr);
			e.healscale              = strtof(row[94], nullptr);
			e.no_target_hotkey       = static_cast<uint8_t>(strtoul(row[95], nullptr, 10));
			e.raid_target            = static_cast<uint8_t>(strtoul(row[96], nullptr, 10));
			e.armtexture             = static_cast<int8_t>(atoi(row[97]));
			e.bracertexture          = static_cast<int8_t>(atoi(row[98]));
			e.handtexture            = static_cast<int8_t>(atoi(row[99]));
			e.legtexture             = static_cast<int8_t>(atoi(row[100]));
			e.feettexture            = static_cast<int8_t>(atoi(row[101]));
			e.light                  = static_cast<int8_t>(atoi(row[102]));
			e.walkspeed              = static_cast<int8_t>(atoi(row[103]));
			e.peqid                  = static_cast<int32_t>(atoi(row[104]));
			e.unique_                = static_cast<int8_t>(atoi(row[105]));
			e.fixed                  = static_cast<int8_t>(atoi(row[106]));
			e.ignore_despawn         = static_cast<int8_t>(atoi(row[107]));
			e.show_name              = static_cast<int8_t>(atoi(row[108]));
			e.untargetable           = static_cast<int8_t>(atoi(row[109]));
			e.charm_ac               = static_cast<int16_t>(atoi(row[110]));
			e.charm_min_dmg          = static_cast<int32_t>(atoi(row[111]));
			e.charm_max_dmg          = static_cast<int32_t>(atoi(row[112]));
			e.charm_attack_delay     = static_cast<int8_t>(atoi(row[113]));
			e.charm_accuracy_rating  = static_cast<int32_t>(atoi(row[114]));
			e.charm_avoidance_rating = static_cast<int32_t>(atoi(row[115]));
			e.charm_atk              = static_cast<int32_t>(atoi(row[116]));
			e.skip_global_loot       = static_cast<int8_t>(atoi(row[117]));
			e.rare_spawn             = static_cast<int8_t>(atoi(row[118]));
			e.stuck_behavior         = static_cast<int8_t>(atoi(row[119]));
			e.model                  = static_cast<int16_t>(atoi(row[120]));
			e.flymode                = static_cast<int8_t>(atoi(row[121]));
			e.always_aggro           = static_cast<int8_t>(atoi(row[122]));
			e.exp_mod                = static_cast<int32_t>(atoi(row[123]));
			e.heroic_strikethrough   = static_cast<int32_t>(atoi(row[124]));
			e.faction_amount         = static_cast<int32_t>(atoi(row[125]));
			e.keeps_sold_items       = static_cast<uint8_t>(strtoul(row[126], nullptr, 10));

			return e;
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
		const NpcTypes &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.lastname) + "'");
		v.push_back(columns[3] + " = " + std::to_string(e.level));
		v.push_back(columns[4] + " = " + std::to_string(e.race));
		v.push_back(columns[5] + " = " + std::to_string(e.class_));
		v.push_back(columns[6] + " = " + std::to_string(e.bodytype));
		v.push_back(columns[7] + " = " + std::to_string(e.hp));
		v.push_back(columns[8] + " = " + std::to_string(e.mana));
		v.push_back(columns[9] + " = " + std::to_string(e.gender));
		v.push_back(columns[10] + " = " + std::to_string(e.texture));
		v.push_back(columns[11] + " = " + std::to_string(e.helmtexture));
		v.push_back(columns[12] + " = " + std::to_string(e.herosforgemodel));
		v.push_back(columns[13] + " = " + std::to_string(e.size));
		v.push_back(columns[14] + " = " + std::to_string(e.hp_regen_rate));
		v.push_back(columns[15] + " = " + std::to_string(e.hp_regen_per_second));
		v.push_back(columns[16] + " = " + std::to_string(e.mana_regen_rate));
		v.push_back(columns[17] + " = " + std::to_string(e.loottable_id));
		v.push_back(columns[18] + " = " + std::to_string(e.merchant_id));
		v.push_back(columns[19] + " = " + std::to_string(e.alt_currency_id));
		v.push_back(columns[20] + " = " + std::to_string(e.npc_spells_id));
		v.push_back(columns[21] + " = " + std::to_string(e.npc_spells_effects_id));
		v.push_back(columns[22] + " = " + std::to_string(e.npc_faction_id));
		v.push_back(columns[23] + " = " + std::to_string(e.adventure_template_id));
		v.push_back(columns[24] + " = " + std::to_string(e.trap_template));
		v.push_back(columns[25] + " = " + std::to_string(e.mindmg));
		v.push_back(columns[26] + " = " + std::to_string(e.maxdmg));
		v.push_back(columns[27] + " = " + std::to_string(e.attack_count));
		v.push_back(columns[28] + " = '" + Strings::Escape(e.npcspecialattks) + "'");
		v.push_back(columns[29] + " = '" + Strings::Escape(e.special_abilities) + "'");
		v.push_back(columns[30] + " = " + std::to_string(e.aggroradius));
		v.push_back(columns[31] + " = " + std::to_string(e.assistradius));
		v.push_back(columns[32] + " = " + std::to_string(e.face));
		v.push_back(columns[33] + " = " + std::to_string(e.luclin_hairstyle));
		v.push_back(columns[34] + " = " + std::to_string(e.luclin_haircolor));
		v.push_back(columns[35] + " = " + std::to_string(e.luclin_eyecolor));
		v.push_back(columns[36] + " = " + std::to_string(e.luclin_eyecolor2));
		v.push_back(columns[37] + " = " + std::to_string(e.luclin_beardcolor));
		v.push_back(columns[38] + " = " + std::to_string(e.luclin_beard));
		v.push_back(columns[39] + " = " + std::to_string(e.drakkin_heritage));
		v.push_back(columns[40] + " = " + std::to_string(e.drakkin_tattoo));
		v.push_back(columns[41] + " = " + std::to_string(e.drakkin_details));
		v.push_back(columns[42] + " = " + std::to_string(e.armortint_id));
		v.push_back(columns[43] + " = " + std::to_string(e.armortint_red));
		v.push_back(columns[44] + " = " + std::to_string(e.armortint_green));
		v.push_back(columns[45] + " = " + std::to_string(e.armortint_blue));
		v.push_back(columns[46] + " = " + std::to_string(e.d_melee_texture1));
		v.push_back(columns[47] + " = " + std::to_string(e.d_melee_texture2));
		v.push_back(columns[48] + " = '" + Strings::Escape(e.ammo_idfile) + "'");
		v.push_back(columns[49] + " = " + std::to_string(e.prim_melee_type));
		v.push_back(columns[50] + " = " + std::to_string(e.sec_melee_type));
		v.push_back(columns[51] + " = " + std::to_string(e.ranged_type));
		v.push_back(columns[52] + " = " + std::to_string(e.runspeed));
		v.push_back(columns[53] + " = " + std::to_string(e.MR));
		v.push_back(columns[54] + " = " + std::to_string(e.CR));
		v.push_back(columns[55] + " = " + std::to_string(e.DR));
		v.push_back(columns[56] + " = " + std::to_string(e.FR));
		v.push_back(columns[57] + " = " + std::to_string(e.PR));
		v.push_back(columns[58] + " = " + std::to_string(e.Corrup));
		v.push_back(columns[59] + " = " + std::to_string(e.PhR));
		v.push_back(columns[60] + " = " + std::to_string(e.see_invis));
		v.push_back(columns[61] + " = " + std::to_string(e.see_invis_undead));
		v.push_back(columns[62] + " = " + std::to_string(e.qglobal));
		v.push_back(columns[63] + " = " + std::to_string(e.AC));
		v.push_back(columns[64] + " = " + std::to_string(e.npc_aggro));
		v.push_back(columns[65] + " = " + std::to_string(e.spawn_limit));
		v.push_back(columns[66] + " = " + std::to_string(e.attack_speed));
		v.push_back(columns[67] + " = " + std::to_string(e.attack_delay));
		v.push_back(columns[68] + " = " + std::to_string(e.findable));
		v.push_back(columns[69] + " = " + std::to_string(e.STR));
		v.push_back(columns[70] + " = " + std::to_string(e.STA));
		v.push_back(columns[71] + " = " + std::to_string(e.DEX));
		v.push_back(columns[72] + " = " + std::to_string(e.AGI));
		v.push_back(columns[73] + " = " + std::to_string(e._INT));
		v.push_back(columns[74] + " = " + std::to_string(e.WIS));
		v.push_back(columns[75] + " = " + std::to_string(e.CHA));
		v.push_back(columns[76] + " = " + std::to_string(e.see_hide));
		v.push_back(columns[77] + " = " + std::to_string(e.see_improved_hide));
		v.push_back(columns[78] + " = " + std::to_string(e.trackable));
		v.push_back(columns[79] + " = " + std::to_string(e.isbot));
		v.push_back(columns[80] + " = " + std::to_string(e.exclude));
		v.push_back(columns[81] + " = " + std::to_string(e.ATK));
		v.push_back(columns[82] + " = " + std::to_string(e.Accuracy));
		v.push_back(columns[83] + " = " + std::to_string(e.Avoidance));
		v.push_back(columns[84] + " = " + std::to_string(e.slow_mitigation));
		v.push_back(columns[85] + " = " + std::to_string(e.version));
		v.push_back(columns[86] + " = " + std::to_string(e.maxlevel));
		v.push_back(columns[87] + " = " + std::to_string(e.scalerate));
		v.push_back(columns[88] + " = " + std::to_string(e.private_corpse));
		v.push_back(columns[89] + " = " + std::to_string(e.unique_spawn_by_name));
		v.push_back(columns[90] + " = " + std::to_string(e.underwater));
		v.push_back(columns[91] + " = " + std::to_string(e.isquest));
		v.push_back(columns[92] + " = " + std::to_string(e.emoteid));
		v.push_back(columns[93] + " = " + std::to_string(e.spellscale));
		v.push_back(columns[94] + " = " + std::to_string(e.healscale));
		v.push_back(columns[95] + " = " + std::to_string(e.no_target_hotkey));
		v.push_back(columns[96] + " = " + std::to_string(e.raid_target));
		v.push_back(columns[97] + " = " + std::to_string(e.armtexture));
		v.push_back(columns[98] + " = " + std::to_string(e.bracertexture));
		v.push_back(columns[99] + " = " + std::to_string(e.handtexture));
		v.push_back(columns[100] + " = " + std::to_string(e.legtexture));
		v.push_back(columns[101] + " = " + std::to_string(e.feettexture));
		v.push_back(columns[102] + " = " + std::to_string(e.light));
		v.push_back(columns[103] + " = " + std::to_string(e.walkspeed));
		v.push_back(columns[104] + " = " + std::to_string(e.peqid));
		v.push_back(columns[105] + " = " + std::to_string(e.unique_));
		v.push_back(columns[106] + " = " + std::to_string(e.fixed));
		v.push_back(columns[107] + " = " + std::to_string(e.ignore_despawn));
		v.push_back(columns[108] + " = " + std::to_string(e.show_name));
		v.push_back(columns[109] + " = " + std::to_string(e.untargetable));
		v.push_back(columns[110] + " = " + std::to_string(e.charm_ac));
		v.push_back(columns[111] + " = " + std::to_string(e.charm_min_dmg));
		v.push_back(columns[112] + " = " + std::to_string(e.charm_max_dmg));
		v.push_back(columns[113] + " = " + std::to_string(e.charm_attack_delay));
		v.push_back(columns[114] + " = " + std::to_string(e.charm_accuracy_rating));
		v.push_back(columns[115] + " = " + std::to_string(e.charm_avoidance_rating));
		v.push_back(columns[116] + " = " + std::to_string(e.charm_atk));
		v.push_back(columns[117] + " = " + std::to_string(e.skip_global_loot));
		v.push_back(columns[118] + " = " + std::to_string(e.rare_spawn));
		v.push_back(columns[119] + " = " + std::to_string(e.stuck_behavior));
		v.push_back(columns[120] + " = " + std::to_string(e.model));
		v.push_back(columns[121] + " = " + std::to_string(e.flymode));
		v.push_back(columns[122] + " = " + std::to_string(e.always_aggro));
		v.push_back(columns[123] + " = " + std::to_string(e.exp_mod));
		v.push_back(columns[124] + " = " + std::to_string(e.heroic_strikethrough));
		v.push_back(columns[125] + " = " + std::to_string(e.faction_amount));
		v.push_back(columns[126] + " = " + std::to_string(e.keeps_sold_items));

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

	static NpcTypes InsertOne(
		Database& db,
		NpcTypes e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back("'" + Strings::Escape(e.lastname) + "'");
		v.push_back(std::to_string(e.level));
		v.push_back(std::to_string(e.race));
		v.push_back(std::to_string(e.class_));
		v.push_back(std::to_string(e.bodytype));
		v.push_back(std::to_string(e.hp));
		v.push_back(std::to_string(e.mana));
		v.push_back(std::to_string(e.gender));
		v.push_back(std::to_string(e.texture));
		v.push_back(std::to_string(e.helmtexture));
		v.push_back(std::to_string(e.herosforgemodel));
		v.push_back(std::to_string(e.size));
		v.push_back(std::to_string(e.hp_regen_rate));
		v.push_back(std::to_string(e.hp_regen_per_second));
		v.push_back(std::to_string(e.mana_regen_rate));
		v.push_back(std::to_string(e.loottable_id));
		v.push_back(std::to_string(e.merchant_id));
		v.push_back(std::to_string(e.alt_currency_id));
		v.push_back(std::to_string(e.npc_spells_id));
		v.push_back(std::to_string(e.npc_spells_effects_id));
		v.push_back(std::to_string(e.npc_faction_id));
		v.push_back(std::to_string(e.adventure_template_id));
		v.push_back(std::to_string(e.trap_template));
		v.push_back(std::to_string(e.mindmg));
		v.push_back(std::to_string(e.maxdmg));
		v.push_back(std::to_string(e.attack_count));
		v.push_back("'" + Strings::Escape(e.npcspecialattks) + "'");
		v.push_back("'" + Strings::Escape(e.special_abilities) + "'");
		v.push_back(std::to_string(e.aggroradius));
		v.push_back(std::to_string(e.assistradius));
		v.push_back(std::to_string(e.face));
		v.push_back(std::to_string(e.luclin_hairstyle));
		v.push_back(std::to_string(e.luclin_haircolor));
		v.push_back(std::to_string(e.luclin_eyecolor));
		v.push_back(std::to_string(e.luclin_eyecolor2));
		v.push_back(std::to_string(e.luclin_beardcolor));
		v.push_back(std::to_string(e.luclin_beard));
		v.push_back(std::to_string(e.drakkin_heritage));
		v.push_back(std::to_string(e.drakkin_tattoo));
		v.push_back(std::to_string(e.drakkin_details));
		v.push_back(std::to_string(e.armortint_id));
		v.push_back(std::to_string(e.armortint_red));
		v.push_back(std::to_string(e.armortint_green));
		v.push_back(std::to_string(e.armortint_blue));
		v.push_back(std::to_string(e.d_melee_texture1));
		v.push_back(std::to_string(e.d_melee_texture2));
		v.push_back("'" + Strings::Escape(e.ammo_idfile) + "'");
		v.push_back(std::to_string(e.prim_melee_type));
		v.push_back(std::to_string(e.sec_melee_type));
		v.push_back(std::to_string(e.ranged_type));
		v.push_back(std::to_string(e.runspeed));
		v.push_back(std::to_string(e.MR));
		v.push_back(std::to_string(e.CR));
		v.push_back(std::to_string(e.DR));
		v.push_back(std::to_string(e.FR));
		v.push_back(std::to_string(e.PR));
		v.push_back(std::to_string(e.Corrup));
		v.push_back(std::to_string(e.PhR));
		v.push_back(std::to_string(e.see_invis));
		v.push_back(std::to_string(e.see_invis_undead));
		v.push_back(std::to_string(e.qglobal));
		v.push_back(std::to_string(e.AC));
		v.push_back(std::to_string(e.npc_aggro));
		v.push_back(std::to_string(e.spawn_limit));
		v.push_back(std::to_string(e.attack_speed));
		v.push_back(std::to_string(e.attack_delay));
		v.push_back(std::to_string(e.findable));
		v.push_back(std::to_string(e.STR));
		v.push_back(std::to_string(e.STA));
		v.push_back(std::to_string(e.DEX));
		v.push_back(std::to_string(e.AGI));
		v.push_back(std::to_string(e._INT));
		v.push_back(std::to_string(e.WIS));
		v.push_back(std::to_string(e.CHA));
		v.push_back(std::to_string(e.see_hide));
		v.push_back(std::to_string(e.see_improved_hide));
		v.push_back(std::to_string(e.trackable));
		v.push_back(std::to_string(e.isbot));
		v.push_back(std::to_string(e.exclude));
		v.push_back(std::to_string(e.ATK));
		v.push_back(std::to_string(e.Accuracy));
		v.push_back(std::to_string(e.Avoidance));
		v.push_back(std::to_string(e.slow_mitigation));
		v.push_back(std::to_string(e.version));
		v.push_back(std::to_string(e.maxlevel));
		v.push_back(std::to_string(e.scalerate));
		v.push_back(std::to_string(e.private_corpse));
		v.push_back(std::to_string(e.unique_spawn_by_name));
		v.push_back(std::to_string(e.underwater));
		v.push_back(std::to_string(e.isquest));
		v.push_back(std::to_string(e.emoteid));
		v.push_back(std::to_string(e.spellscale));
		v.push_back(std::to_string(e.healscale));
		v.push_back(std::to_string(e.no_target_hotkey));
		v.push_back(std::to_string(e.raid_target));
		v.push_back(std::to_string(e.armtexture));
		v.push_back(std::to_string(e.bracertexture));
		v.push_back(std::to_string(e.handtexture));
		v.push_back(std::to_string(e.legtexture));
		v.push_back(std::to_string(e.feettexture));
		v.push_back(std::to_string(e.light));
		v.push_back(std::to_string(e.walkspeed));
		v.push_back(std::to_string(e.peqid));
		v.push_back(std::to_string(e.unique_));
		v.push_back(std::to_string(e.fixed));
		v.push_back(std::to_string(e.ignore_despawn));
		v.push_back(std::to_string(e.show_name));
		v.push_back(std::to_string(e.untargetable));
		v.push_back(std::to_string(e.charm_ac));
		v.push_back(std::to_string(e.charm_min_dmg));
		v.push_back(std::to_string(e.charm_max_dmg));
		v.push_back(std::to_string(e.charm_attack_delay));
		v.push_back(std::to_string(e.charm_accuracy_rating));
		v.push_back(std::to_string(e.charm_avoidance_rating));
		v.push_back(std::to_string(e.charm_atk));
		v.push_back(std::to_string(e.skip_global_loot));
		v.push_back(std::to_string(e.rare_spawn));
		v.push_back(std::to_string(e.stuck_behavior));
		v.push_back(std::to_string(e.model));
		v.push_back(std::to_string(e.flymode));
		v.push_back(std::to_string(e.always_aggro));
		v.push_back(std::to_string(e.exp_mod));
		v.push_back(std::to_string(e.heroic_strikethrough));
		v.push_back(std::to_string(e.faction_amount));
		v.push_back(std::to_string(e.keeps_sold_items));

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
		const std::vector<NpcTypes> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back("'" + Strings::Escape(e.lastname) + "'");
			v.push_back(std::to_string(e.level));
			v.push_back(std::to_string(e.race));
			v.push_back(std::to_string(e.class_));
			v.push_back(std::to_string(e.bodytype));
			v.push_back(std::to_string(e.hp));
			v.push_back(std::to_string(e.mana));
			v.push_back(std::to_string(e.gender));
			v.push_back(std::to_string(e.texture));
			v.push_back(std::to_string(e.helmtexture));
			v.push_back(std::to_string(e.herosforgemodel));
			v.push_back(std::to_string(e.size));
			v.push_back(std::to_string(e.hp_regen_rate));
			v.push_back(std::to_string(e.hp_regen_per_second));
			v.push_back(std::to_string(e.mana_regen_rate));
			v.push_back(std::to_string(e.loottable_id));
			v.push_back(std::to_string(e.merchant_id));
			v.push_back(std::to_string(e.alt_currency_id));
			v.push_back(std::to_string(e.npc_spells_id));
			v.push_back(std::to_string(e.npc_spells_effects_id));
			v.push_back(std::to_string(e.npc_faction_id));
			v.push_back(std::to_string(e.adventure_template_id));
			v.push_back(std::to_string(e.trap_template));
			v.push_back(std::to_string(e.mindmg));
			v.push_back(std::to_string(e.maxdmg));
			v.push_back(std::to_string(e.attack_count));
			v.push_back("'" + Strings::Escape(e.npcspecialattks) + "'");
			v.push_back("'" + Strings::Escape(e.special_abilities) + "'");
			v.push_back(std::to_string(e.aggroradius));
			v.push_back(std::to_string(e.assistradius));
			v.push_back(std::to_string(e.face));
			v.push_back(std::to_string(e.luclin_hairstyle));
			v.push_back(std::to_string(e.luclin_haircolor));
			v.push_back(std::to_string(e.luclin_eyecolor));
			v.push_back(std::to_string(e.luclin_eyecolor2));
			v.push_back(std::to_string(e.luclin_beardcolor));
			v.push_back(std::to_string(e.luclin_beard));
			v.push_back(std::to_string(e.drakkin_heritage));
			v.push_back(std::to_string(e.drakkin_tattoo));
			v.push_back(std::to_string(e.drakkin_details));
			v.push_back(std::to_string(e.armortint_id));
			v.push_back(std::to_string(e.armortint_red));
			v.push_back(std::to_string(e.armortint_green));
			v.push_back(std::to_string(e.armortint_blue));
			v.push_back(std::to_string(e.d_melee_texture1));
			v.push_back(std::to_string(e.d_melee_texture2));
			v.push_back("'" + Strings::Escape(e.ammo_idfile) + "'");
			v.push_back(std::to_string(e.prim_melee_type));
			v.push_back(std::to_string(e.sec_melee_type));
			v.push_back(std::to_string(e.ranged_type));
			v.push_back(std::to_string(e.runspeed));
			v.push_back(std::to_string(e.MR));
			v.push_back(std::to_string(e.CR));
			v.push_back(std::to_string(e.DR));
			v.push_back(std::to_string(e.FR));
			v.push_back(std::to_string(e.PR));
			v.push_back(std::to_string(e.Corrup));
			v.push_back(std::to_string(e.PhR));
			v.push_back(std::to_string(e.see_invis));
			v.push_back(std::to_string(e.see_invis_undead));
			v.push_back(std::to_string(e.qglobal));
			v.push_back(std::to_string(e.AC));
			v.push_back(std::to_string(e.npc_aggro));
			v.push_back(std::to_string(e.spawn_limit));
			v.push_back(std::to_string(e.attack_speed));
			v.push_back(std::to_string(e.attack_delay));
			v.push_back(std::to_string(e.findable));
			v.push_back(std::to_string(e.STR));
			v.push_back(std::to_string(e.STA));
			v.push_back(std::to_string(e.DEX));
			v.push_back(std::to_string(e.AGI));
			v.push_back(std::to_string(e._INT));
			v.push_back(std::to_string(e.WIS));
			v.push_back(std::to_string(e.CHA));
			v.push_back(std::to_string(e.see_hide));
			v.push_back(std::to_string(e.see_improved_hide));
			v.push_back(std::to_string(e.trackable));
			v.push_back(std::to_string(e.isbot));
			v.push_back(std::to_string(e.exclude));
			v.push_back(std::to_string(e.ATK));
			v.push_back(std::to_string(e.Accuracy));
			v.push_back(std::to_string(e.Avoidance));
			v.push_back(std::to_string(e.slow_mitigation));
			v.push_back(std::to_string(e.version));
			v.push_back(std::to_string(e.maxlevel));
			v.push_back(std::to_string(e.scalerate));
			v.push_back(std::to_string(e.private_corpse));
			v.push_back(std::to_string(e.unique_spawn_by_name));
			v.push_back(std::to_string(e.underwater));
			v.push_back(std::to_string(e.isquest));
			v.push_back(std::to_string(e.emoteid));
			v.push_back(std::to_string(e.spellscale));
			v.push_back(std::to_string(e.healscale));
			v.push_back(std::to_string(e.no_target_hotkey));
			v.push_back(std::to_string(e.raid_target));
			v.push_back(std::to_string(e.armtexture));
			v.push_back(std::to_string(e.bracertexture));
			v.push_back(std::to_string(e.handtexture));
			v.push_back(std::to_string(e.legtexture));
			v.push_back(std::to_string(e.feettexture));
			v.push_back(std::to_string(e.light));
			v.push_back(std::to_string(e.walkspeed));
			v.push_back(std::to_string(e.peqid));
			v.push_back(std::to_string(e.unique_));
			v.push_back(std::to_string(e.fixed));
			v.push_back(std::to_string(e.ignore_despawn));
			v.push_back(std::to_string(e.show_name));
			v.push_back(std::to_string(e.untargetable));
			v.push_back(std::to_string(e.charm_ac));
			v.push_back(std::to_string(e.charm_min_dmg));
			v.push_back(std::to_string(e.charm_max_dmg));
			v.push_back(std::to_string(e.charm_attack_delay));
			v.push_back(std::to_string(e.charm_accuracy_rating));
			v.push_back(std::to_string(e.charm_avoidance_rating));
			v.push_back(std::to_string(e.charm_atk));
			v.push_back(std::to_string(e.skip_global_loot));
			v.push_back(std::to_string(e.rare_spawn));
			v.push_back(std::to_string(e.stuck_behavior));
			v.push_back(std::to_string(e.model));
			v.push_back(std::to_string(e.flymode));
			v.push_back(std::to_string(e.always_aggro));
			v.push_back(std::to_string(e.exp_mod));
			v.push_back(std::to_string(e.heroic_strikethrough));
			v.push_back(std::to_string(e.faction_amount));
			v.push_back(std::to_string(e.keeps_sold_items));

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
			NpcTypes e{};

			e.id                     = static_cast<int32_t>(atoi(row[0]));
			e.name                   = row[1] ? row[1] : "";
			e.lastname               = row[2] ? row[2] : "";
			e.level                  = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));
			e.race                   = static_cast<uint16_t>(strtoul(row[4], nullptr, 10));
			e.class_                 = static_cast<uint8_t>(strtoul(row[5], nullptr, 10));
			e.bodytype               = static_cast<int32_t>(atoi(row[6]));
			e.hp                     = strtoll(row[7], nullptr, 10);
			e.mana                   = strtoll(row[8], nullptr, 10);
			e.gender                 = static_cast<uint8_t>(strtoul(row[9], nullptr, 10));
			e.texture                = static_cast<uint8_t>(strtoul(row[10], nullptr, 10));
			e.helmtexture            = static_cast<uint8_t>(strtoul(row[11], nullptr, 10));
			e.herosforgemodel        = static_cast<int32_t>(atoi(row[12]));
			e.size                   = strtof(row[13], nullptr);
			e.hp_regen_rate          = strtoll(row[14], nullptr, 10);
			e.hp_regen_per_second    = strtoll(row[15], nullptr, 10);
			e.mana_regen_rate        = strtoll(row[16], nullptr, 10);
			e.loottable_id           = static_cast<uint32_t>(strtoul(row[17], nullptr, 10));
			e.merchant_id            = static_cast<uint32_t>(strtoul(row[18], nullptr, 10));
			e.alt_currency_id        = static_cast<uint32_t>(strtoul(row[19], nullptr, 10));
			e.npc_spells_id          = static_cast<uint32_t>(strtoul(row[20], nullptr, 10));
			e.npc_spells_effects_id  = static_cast<uint32_t>(strtoul(row[21], nullptr, 10));
			e.npc_faction_id         = static_cast<int32_t>(atoi(row[22]));
			e.adventure_template_id  = static_cast<uint32_t>(strtoul(row[23], nullptr, 10));
			e.trap_template          = static_cast<uint32_t>(strtoul(row[24], nullptr, 10));
			e.mindmg                 = static_cast<uint32_t>(strtoul(row[25], nullptr, 10));
			e.maxdmg                 = static_cast<uint32_t>(strtoul(row[26], nullptr, 10));
			e.attack_count           = static_cast<int16_t>(atoi(row[27]));
			e.npcspecialattks        = row[28] ? row[28] : "";
			e.special_abilities      = row[29] ? row[29] : "";
			e.aggroradius            = static_cast<uint32_t>(strtoul(row[30], nullptr, 10));
			e.assistradius           = static_cast<uint32_t>(strtoul(row[31], nullptr, 10));
			e.face                   = static_cast<uint32_t>(strtoul(row[32], nullptr, 10));
			e.luclin_hairstyle       = static_cast<uint32_t>(strtoul(row[33], nullptr, 10));
			e.luclin_haircolor       = static_cast<uint32_t>(strtoul(row[34], nullptr, 10));
			e.luclin_eyecolor        = static_cast<uint32_t>(strtoul(row[35], nullptr, 10));
			e.luclin_eyecolor2       = static_cast<uint32_t>(strtoul(row[36], nullptr, 10));
			e.luclin_beardcolor      = static_cast<uint32_t>(strtoul(row[37], nullptr, 10));
			e.luclin_beard           = static_cast<uint32_t>(strtoul(row[38], nullptr, 10));
			e.drakkin_heritage       = static_cast<int32_t>(atoi(row[39]));
			e.drakkin_tattoo         = static_cast<int32_t>(atoi(row[40]));
			e.drakkin_details        = static_cast<int32_t>(atoi(row[41]));
			e.armortint_id           = static_cast<uint32_t>(strtoul(row[42], nullptr, 10));
			e.armortint_red          = static_cast<uint8_t>(strtoul(row[43], nullptr, 10));
			e.armortint_green        = static_cast<uint8_t>(strtoul(row[44], nullptr, 10));
			e.armortint_blue         = static_cast<uint8_t>(strtoul(row[45], nullptr, 10));
			e.d_melee_texture1       = static_cast<uint32_t>(strtoul(row[46], nullptr, 10));
			e.d_melee_texture2       = static_cast<uint32_t>(strtoul(row[47], nullptr, 10));
			e.ammo_idfile            = row[48] ? row[48] : "";
			e.prim_melee_type        = static_cast<uint8_t>(strtoul(row[49], nullptr, 10));
			e.sec_melee_type         = static_cast<uint8_t>(strtoul(row[50], nullptr, 10));
			e.ranged_type            = static_cast<uint8_t>(strtoul(row[51], nullptr, 10));
			e.runspeed               = strtof(row[52], nullptr);
			e.MR                     = static_cast<int16_t>(atoi(row[53]));
			e.CR                     = static_cast<int16_t>(atoi(row[54]));
			e.DR                     = static_cast<int16_t>(atoi(row[55]));
			e.FR                     = static_cast<int16_t>(atoi(row[56]));
			e.PR                     = static_cast<int16_t>(atoi(row[57]));
			e.Corrup                 = static_cast<int16_t>(atoi(row[58]));
			e.PhR                    = static_cast<uint16_t>(strtoul(row[59], nullptr, 10));
			e.see_invis              = static_cast<int16_t>(atoi(row[60]));
			e.see_invis_undead       = static_cast<int16_t>(atoi(row[61]));
			e.qglobal                = static_cast<uint32_t>(strtoul(row[62], nullptr, 10));
			e.AC                     = static_cast<int16_t>(atoi(row[63]));
			e.npc_aggro              = static_cast<int8_t>(atoi(row[64]));
			e.spawn_limit            = static_cast<int8_t>(atoi(row[65]));
			e.attack_speed           = strtof(row[66], nullptr);
			e.attack_delay           = static_cast<uint8_t>(strtoul(row[67], nullptr, 10));
			e.findable               = static_cast<int8_t>(atoi(row[68]));
			e.STR                    = static_cast<uint32_t>(strtoul(row[69], nullptr, 10));
			e.STA                    = static_cast<uint32_t>(strtoul(row[70], nullptr, 10));
			e.DEX                    = static_cast<uint32_t>(strtoul(row[71], nullptr, 10));
			e.AGI                    = static_cast<uint32_t>(strtoul(row[72], nullptr, 10));
			e._INT                   = static_cast<uint32_t>(strtoul(row[73], nullptr, 10));
			e.WIS                    = static_cast<uint32_t>(strtoul(row[74], nullptr, 10));
			e.CHA                    = static_cast<uint32_t>(strtoul(row[75], nullptr, 10));
			e.see_hide               = static_cast<int8_t>(atoi(row[76]));
			e.see_improved_hide      = static_cast<int8_t>(atoi(row[77]));
			e.trackable              = static_cast<int8_t>(atoi(row[78]));
			e.isbot                  = static_cast<int8_t>(atoi(row[79]));
			e.exclude                = static_cast<int8_t>(atoi(row[80]));
			e.ATK                    = static_cast<int32_t>(atoi(row[81]));
			e.Accuracy               = static_cast<int32_t>(atoi(row[82]));
			e.Avoidance              = static_cast<uint32_t>(strtoul(row[83], nullptr, 10));
			e.slow_mitigation        = static_cast<int16_t>(atoi(row[84]));
			e.version                = static_cast<uint16_t>(strtoul(row[85], nullptr, 10));
			e.maxlevel               = static_cast<int8_t>(atoi(row[86]));
			e.scalerate              = static_cast<int32_t>(atoi(row[87]));
			e.private_corpse         = static_cast<uint8_t>(strtoul(row[88], nullptr, 10));
			e.unique_spawn_by_name   = static_cast<uint8_t>(strtoul(row[89], nullptr, 10));
			e.underwater             = static_cast<uint8_t>(strtoul(row[90], nullptr, 10));
			e.isquest                = static_cast<int8_t>(atoi(row[91]));
			e.emoteid                = static_cast<uint32_t>(strtoul(row[92], nullptr, 10));
			e.spellscale             = strtof(row[93], nullptr);
			e.healscale              = strtof(row[94], nullptr);
			e.no_target_hotkey       = static_cast<uint8_t>(strtoul(row[95], nullptr, 10));
			e.raid_target            = static_cast<uint8_t>(strtoul(row[96], nullptr, 10));
			e.armtexture             = static_cast<int8_t>(atoi(row[97]));
			e.bracertexture          = static_cast<int8_t>(atoi(row[98]));
			e.handtexture            = static_cast<int8_t>(atoi(row[99]));
			e.legtexture             = static_cast<int8_t>(atoi(row[100]));
			e.feettexture            = static_cast<int8_t>(atoi(row[101]));
			e.light                  = static_cast<int8_t>(atoi(row[102]));
			e.walkspeed              = static_cast<int8_t>(atoi(row[103]));
			e.peqid                  = static_cast<int32_t>(atoi(row[104]));
			e.unique_                = static_cast<int8_t>(atoi(row[105]));
			e.fixed                  = static_cast<int8_t>(atoi(row[106]));
			e.ignore_despawn         = static_cast<int8_t>(atoi(row[107]));
			e.show_name              = static_cast<int8_t>(atoi(row[108]));
			e.untargetable           = static_cast<int8_t>(atoi(row[109]));
			e.charm_ac               = static_cast<int16_t>(atoi(row[110]));
			e.charm_min_dmg          = static_cast<int32_t>(atoi(row[111]));
			e.charm_max_dmg          = static_cast<int32_t>(atoi(row[112]));
			e.charm_attack_delay     = static_cast<int8_t>(atoi(row[113]));
			e.charm_accuracy_rating  = static_cast<int32_t>(atoi(row[114]));
			e.charm_avoidance_rating = static_cast<int32_t>(atoi(row[115]));
			e.charm_atk              = static_cast<int32_t>(atoi(row[116]));
			e.skip_global_loot       = static_cast<int8_t>(atoi(row[117]));
			e.rare_spawn             = static_cast<int8_t>(atoi(row[118]));
			e.stuck_behavior         = static_cast<int8_t>(atoi(row[119]));
			e.model                  = static_cast<int16_t>(atoi(row[120]));
			e.flymode                = static_cast<int8_t>(atoi(row[121]));
			e.always_aggro           = static_cast<int8_t>(atoi(row[122]));
			e.exp_mod                = static_cast<int32_t>(atoi(row[123]));
			e.heroic_strikethrough   = static_cast<int32_t>(atoi(row[124]));
			e.faction_amount         = static_cast<int32_t>(atoi(row[125]));
			e.keeps_sold_items       = static_cast<uint8_t>(strtoul(row[126], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<NpcTypes> GetWhere(Database& db, const std::string &where_filter)
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
			NpcTypes e{};

			e.id                     = static_cast<int32_t>(atoi(row[0]));
			e.name                   = row[1] ? row[1] : "";
			e.lastname               = row[2] ? row[2] : "";
			e.level                  = static_cast<uint8_t>(strtoul(row[3], nullptr, 10));
			e.race                   = static_cast<uint16_t>(strtoul(row[4], nullptr, 10));
			e.class_                 = static_cast<uint8_t>(strtoul(row[5], nullptr, 10));
			e.bodytype               = static_cast<int32_t>(atoi(row[6]));
			e.hp                     = strtoll(row[7], nullptr, 10);
			e.mana                   = strtoll(row[8], nullptr, 10);
			e.gender                 = static_cast<uint8_t>(strtoul(row[9], nullptr, 10));
			e.texture                = static_cast<uint8_t>(strtoul(row[10], nullptr, 10));
			e.helmtexture            = static_cast<uint8_t>(strtoul(row[11], nullptr, 10));
			e.herosforgemodel        = static_cast<int32_t>(atoi(row[12]));
			e.size                   = strtof(row[13], nullptr);
			e.hp_regen_rate          = strtoll(row[14], nullptr, 10);
			e.hp_regen_per_second    = strtoll(row[15], nullptr, 10);
			e.mana_regen_rate        = strtoll(row[16], nullptr, 10);
			e.loottable_id           = static_cast<uint32_t>(strtoul(row[17], nullptr, 10));
			e.merchant_id            = static_cast<uint32_t>(strtoul(row[18], nullptr, 10));
			e.alt_currency_id        = static_cast<uint32_t>(strtoul(row[19], nullptr, 10));
			e.npc_spells_id          = static_cast<uint32_t>(strtoul(row[20], nullptr, 10));
			e.npc_spells_effects_id  = static_cast<uint32_t>(strtoul(row[21], nullptr, 10));
			e.npc_faction_id         = static_cast<int32_t>(atoi(row[22]));
			e.adventure_template_id  = static_cast<uint32_t>(strtoul(row[23], nullptr, 10));
			e.trap_template          = static_cast<uint32_t>(strtoul(row[24], nullptr, 10));
			e.mindmg                 = static_cast<uint32_t>(strtoul(row[25], nullptr, 10));
			e.maxdmg                 = static_cast<uint32_t>(strtoul(row[26], nullptr, 10));
			e.attack_count           = static_cast<int16_t>(atoi(row[27]));
			e.npcspecialattks        = row[28] ? row[28] : "";
			e.special_abilities      = row[29] ? row[29] : "";
			e.aggroradius            = static_cast<uint32_t>(strtoul(row[30], nullptr, 10));
			e.assistradius           = static_cast<uint32_t>(strtoul(row[31], nullptr, 10));
			e.face                   = static_cast<uint32_t>(strtoul(row[32], nullptr, 10));
			e.luclin_hairstyle       = static_cast<uint32_t>(strtoul(row[33], nullptr, 10));
			e.luclin_haircolor       = static_cast<uint32_t>(strtoul(row[34], nullptr, 10));
			e.luclin_eyecolor        = static_cast<uint32_t>(strtoul(row[35], nullptr, 10));
			e.luclin_eyecolor2       = static_cast<uint32_t>(strtoul(row[36], nullptr, 10));
			e.luclin_beardcolor      = static_cast<uint32_t>(strtoul(row[37], nullptr, 10));
			e.luclin_beard           = static_cast<uint32_t>(strtoul(row[38], nullptr, 10));
			e.drakkin_heritage       = static_cast<int32_t>(atoi(row[39]));
			e.drakkin_tattoo         = static_cast<int32_t>(atoi(row[40]));
			e.drakkin_details        = static_cast<int32_t>(atoi(row[41]));
			e.armortint_id           = static_cast<uint32_t>(strtoul(row[42], nullptr, 10));
			e.armortint_red          = static_cast<uint8_t>(strtoul(row[43], nullptr, 10));
			e.armortint_green        = static_cast<uint8_t>(strtoul(row[44], nullptr, 10));
			e.armortint_blue         = static_cast<uint8_t>(strtoul(row[45], nullptr, 10));
			e.d_melee_texture1       = static_cast<uint32_t>(strtoul(row[46], nullptr, 10));
			e.d_melee_texture2       = static_cast<uint32_t>(strtoul(row[47], nullptr, 10));
			e.ammo_idfile            = row[48] ? row[48] : "";
			e.prim_melee_type        = static_cast<uint8_t>(strtoul(row[49], nullptr, 10));
			e.sec_melee_type         = static_cast<uint8_t>(strtoul(row[50], nullptr, 10));
			e.ranged_type            = static_cast<uint8_t>(strtoul(row[51], nullptr, 10));
			e.runspeed               = strtof(row[52], nullptr);
			e.MR                     = static_cast<int16_t>(atoi(row[53]));
			e.CR                     = static_cast<int16_t>(atoi(row[54]));
			e.DR                     = static_cast<int16_t>(atoi(row[55]));
			e.FR                     = static_cast<int16_t>(atoi(row[56]));
			e.PR                     = static_cast<int16_t>(atoi(row[57]));
			e.Corrup                 = static_cast<int16_t>(atoi(row[58]));
			e.PhR                    = static_cast<uint16_t>(strtoul(row[59], nullptr, 10));
			e.see_invis              = static_cast<int16_t>(atoi(row[60]));
			e.see_invis_undead       = static_cast<int16_t>(atoi(row[61]));
			e.qglobal                = static_cast<uint32_t>(strtoul(row[62], nullptr, 10));
			e.AC                     = static_cast<int16_t>(atoi(row[63]));
			e.npc_aggro              = static_cast<int8_t>(atoi(row[64]));
			e.spawn_limit            = static_cast<int8_t>(atoi(row[65]));
			e.attack_speed           = strtof(row[66], nullptr);
			e.attack_delay           = static_cast<uint8_t>(strtoul(row[67], nullptr, 10));
			e.findable               = static_cast<int8_t>(atoi(row[68]));
			e.STR                    = static_cast<uint32_t>(strtoul(row[69], nullptr, 10));
			e.STA                    = static_cast<uint32_t>(strtoul(row[70], nullptr, 10));
			e.DEX                    = static_cast<uint32_t>(strtoul(row[71], nullptr, 10));
			e.AGI                    = static_cast<uint32_t>(strtoul(row[72], nullptr, 10));
			e._INT                   = static_cast<uint32_t>(strtoul(row[73], nullptr, 10));
			e.WIS                    = static_cast<uint32_t>(strtoul(row[74], nullptr, 10));
			e.CHA                    = static_cast<uint32_t>(strtoul(row[75], nullptr, 10));
			e.see_hide               = static_cast<int8_t>(atoi(row[76]));
			e.see_improved_hide      = static_cast<int8_t>(atoi(row[77]));
			e.trackable              = static_cast<int8_t>(atoi(row[78]));
			e.isbot                  = static_cast<int8_t>(atoi(row[79]));
			e.exclude                = static_cast<int8_t>(atoi(row[80]));
			e.ATK                    = static_cast<int32_t>(atoi(row[81]));
			e.Accuracy               = static_cast<int32_t>(atoi(row[82]));
			e.Avoidance              = static_cast<uint32_t>(strtoul(row[83], nullptr, 10));
			e.slow_mitigation        = static_cast<int16_t>(atoi(row[84]));
			e.version                = static_cast<uint16_t>(strtoul(row[85], nullptr, 10));
			e.maxlevel               = static_cast<int8_t>(atoi(row[86]));
			e.scalerate              = static_cast<int32_t>(atoi(row[87]));
			e.private_corpse         = static_cast<uint8_t>(strtoul(row[88], nullptr, 10));
			e.unique_spawn_by_name   = static_cast<uint8_t>(strtoul(row[89], nullptr, 10));
			e.underwater             = static_cast<uint8_t>(strtoul(row[90], nullptr, 10));
			e.isquest                = static_cast<int8_t>(atoi(row[91]));
			e.emoteid                = static_cast<uint32_t>(strtoul(row[92], nullptr, 10));
			e.spellscale             = strtof(row[93], nullptr);
			e.healscale              = strtof(row[94], nullptr);
			e.no_target_hotkey       = static_cast<uint8_t>(strtoul(row[95], nullptr, 10));
			e.raid_target            = static_cast<uint8_t>(strtoul(row[96], nullptr, 10));
			e.armtexture             = static_cast<int8_t>(atoi(row[97]));
			e.bracertexture          = static_cast<int8_t>(atoi(row[98]));
			e.handtexture            = static_cast<int8_t>(atoi(row[99]));
			e.legtexture             = static_cast<int8_t>(atoi(row[100]));
			e.feettexture            = static_cast<int8_t>(atoi(row[101]));
			e.light                  = static_cast<int8_t>(atoi(row[102]));
			e.walkspeed              = static_cast<int8_t>(atoi(row[103]));
			e.peqid                  = static_cast<int32_t>(atoi(row[104]));
			e.unique_                = static_cast<int8_t>(atoi(row[105]));
			e.fixed                  = static_cast<int8_t>(atoi(row[106]));
			e.ignore_despawn         = static_cast<int8_t>(atoi(row[107]));
			e.show_name              = static_cast<int8_t>(atoi(row[108]));
			e.untargetable           = static_cast<int8_t>(atoi(row[109]));
			e.charm_ac               = static_cast<int16_t>(atoi(row[110]));
			e.charm_min_dmg          = static_cast<int32_t>(atoi(row[111]));
			e.charm_max_dmg          = static_cast<int32_t>(atoi(row[112]));
			e.charm_attack_delay     = static_cast<int8_t>(atoi(row[113]));
			e.charm_accuracy_rating  = static_cast<int32_t>(atoi(row[114]));
			e.charm_avoidance_rating = static_cast<int32_t>(atoi(row[115]));
			e.charm_atk              = static_cast<int32_t>(atoi(row[116]));
			e.skip_global_loot       = static_cast<int8_t>(atoi(row[117]));
			e.rare_spawn             = static_cast<int8_t>(atoi(row[118]));
			e.stuck_behavior         = static_cast<int8_t>(atoi(row[119]));
			e.model                  = static_cast<int16_t>(atoi(row[120]));
			e.flymode                = static_cast<int8_t>(atoi(row[121]));
			e.always_aggro           = static_cast<int8_t>(atoi(row[122]));
			e.exp_mod                = static_cast<int32_t>(atoi(row[123]));
			e.heroic_strikethrough   = static_cast<int32_t>(atoi(row[124]));
			e.faction_amount         = static_cast<int32_t>(atoi(row[125]));
			e.keeps_sold_items       = static_cast<uint8_t>(strtoul(row[126], nullptr, 10));

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

#endif //EQEMU_BASE_NPC_TYPES_REPOSITORY_H
