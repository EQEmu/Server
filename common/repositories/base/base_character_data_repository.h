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

#ifndef EQEMU_BASE_CHARACTER_DATA_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_DATA_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseCharacterDataRepository {
public:
	struct CharacterData {
		uint32_t    id;
		int32_t     account_id;
		std::string name;
		std::string last_name;
		std::string title;
		std::string suffix;
		uint32_t    zone_id;
		uint32_t    zone_instance;
		float       y;
		float       x;
		float       z;
		float       heading;
		uint8_t     gender;
		uint16_t    race;
		uint8_t     class_;
		uint32_t    level;
		uint32_t    deity;
		uint32_t    birthday;
		uint32_t    last_login;
		uint32_t    time_played;
		uint8_t     level2;
		uint8_t     anon;
		uint8_t     gm;
		uint32_t    face;
		uint8_t     hair_color;
		uint8_t     hair_style;
		uint8_t     beard;
		uint8_t     beard_color;
		uint8_t     eye_color_1;
		uint8_t     eye_color_2;
		uint32_t    drakkin_heritage;
		uint32_t    drakkin_tattoo;
		uint32_t    drakkin_details;
		uint8_t     ability_time_seconds;
		uint8_t     ability_number;
		uint8_t     ability_time_minutes;
		uint8_t     ability_time_hours;
		uint32_t    exp;
		uint32_t    aa_points_spent;
		uint32_t    aa_exp;
		uint32_t    aa_points;
		uint32_t    group_leadership_exp;
		uint32_t    raid_leadership_exp;
		uint32_t    group_leadership_points;
		uint32_t    raid_leadership_points;
		uint32_t    points;
		uint32_t    cur_hp;
		uint32_t    mana;
		uint32_t    endurance;
		uint32_t    intoxication;
		uint32_t    str;
		uint32_t    sta;
		uint32_t    cha;
		uint32_t    dex;
		uint32_t    int_;
		uint32_t    agi;
		uint32_t    wis;
		uint32_t    zone_change_count;
		uint32_t    toxicity;
		uint32_t    hunger_level;
		uint32_t    thirst_level;
		uint32_t    ability_up;
		uint32_t    ldon_points_guk;
		uint32_t    ldon_points_mir;
		uint32_t    ldon_points_mmc;
		uint32_t    ldon_points_ruj;
		uint32_t    ldon_points_tak;
		uint32_t    ldon_points_available;
		uint32_t    tribute_time_remaining;
		uint32_t    career_tribute_points;
		uint32_t    tribute_points;
		uint32_t    tribute_active;
		uint8_t     pvp_status;
		uint32_t    pvp_kills;
		uint32_t    pvp_deaths;
		uint32_t    pvp_current_points;
		uint32_t    pvp_career_points;
		uint32_t    pvp_best_kill_streak;
		uint32_t    pvp_worst_death_streak;
		uint32_t    pvp_current_kill_streak;
		uint32_t    pvp2;
		uint32_t    pvp_type;
		uint32_t    show_helm;
		uint8_t     group_auto_consent;
		uint8_t     raid_auto_consent;
		uint8_t     guild_auto_consent;
		uint8_t     leadership_exp_on;
		uint32_t    RestTimer;
		uint32_t    air_remaining;
		uint32_t    autosplit_enabled;
		uint8_t     lfp;
		uint8_t     lfg;
		std::string mailkey;
		uint8_t     xtargets;
		int8_t      firstlogon;
		uint32_t    e_aa_effects;
		uint32_t    e_percent_to_aa;
		uint32_t    e_expended_aa_spent;
		uint32_t    aa_points_spent_old;
		uint32_t    aa_points_old;
		uint32_t    e_last_invsnapshot;
		time_t      deleted_at;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"account_id",
			"name",
			"last_name",
			"title",
			"suffix",
			"zone_id",
			"zone_instance",
			"y",
			"x",
			"z",
			"heading",
			"gender",
			"race",
			"`class`",
			"level",
			"deity",
			"birthday",
			"last_login",
			"time_played",
			"level2",
			"anon",
			"gm",
			"face",
			"hair_color",
			"hair_style",
			"beard",
			"beard_color",
			"eye_color_1",
			"eye_color_2",
			"drakkin_heritage",
			"drakkin_tattoo",
			"drakkin_details",
			"ability_time_seconds",
			"ability_number",
			"ability_time_minutes",
			"ability_time_hours",
			"exp",
			"aa_points_spent",
			"aa_exp",
			"aa_points",
			"group_leadership_exp",
			"raid_leadership_exp",
			"group_leadership_points",
			"raid_leadership_points",
			"points",
			"cur_hp",
			"mana",
			"endurance",
			"intoxication",
			"str",
			"sta",
			"cha",
			"dex",
			"`int`",
			"agi",
			"wis",
			"zone_change_count",
			"toxicity",
			"hunger_level",
			"thirst_level",
			"ability_up",
			"ldon_points_guk",
			"ldon_points_mir",
			"ldon_points_mmc",
			"ldon_points_ruj",
			"ldon_points_tak",
			"ldon_points_available",
			"tribute_time_remaining",
			"career_tribute_points",
			"tribute_points",
			"tribute_active",
			"pvp_status",
			"pvp_kills",
			"pvp_deaths",
			"pvp_current_points",
			"pvp_career_points",
			"pvp_best_kill_streak",
			"pvp_worst_death_streak",
			"pvp_current_kill_streak",
			"pvp2",
			"pvp_type",
			"show_helm",
			"group_auto_consent",
			"raid_auto_consent",
			"guild_auto_consent",
			"leadership_exp_on",
			"RestTimer",
			"air_remaining",
			"autosplit_enabled",
			"lfp",
			"lfg",
			"mailkey",
			"xtargets",
			"firstlogon",
			"e_aa_effects",
			"e_percent_to_aa",
			"e_expended_aa_spent",
			"aa_points_spent_old",
			"aa_points_old",
			"e_last_invsnapshot",
			"deleted_at",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"account_id",
			"name",
			"last_name",
			"title",
			"suffix",
			"zone_id",
			"zone_instance",
			"y",
			"x",
			"z",
			"heading",
			"gender",
			"race",
			"`class`",
			"level",
			"deity",
			"birthday",
			"last_login",
			"time_played",
			"level2",
			"anon",
			"gm",
			"face",
			"hair_color",
			"hair_style",
			"beard",
			"beard_color",
			"eye_color_1",
			"eye_color_2",
			"drakkin_heritage",
			"drakkin_tattoo",
			"drakkin_details",
			"ability_time_seconds",
			"ability_number",
			"ability_time_minutes",
			"ability_time_hours",
			"exp",
			"aa_points_spent",
			"aa_exp",
			"aa_points",
			"group_leadership_exp",
			"raid_leadership_exp",
			"group_leadership_points",
			"raid_leadership_points",
			"points",
			"cur_hp",
			"mana",
			"endurance",
			"intoxication",
			"str",
			"sta",
			"cha",
			"dex",
			"`int`",
			"agi",
			"wis",
			"zone_change_count",
			"toxicity",
			"hunger_level",
			"thirst_level",
			"ability_up",
			"ldon_points_guk",
			"ldon_points_mir",
			"ldon_points_mmc",
			"ldon_points_ruj",
			"ldon_points_tak",
			"ldon_points_available",
			"tribute_time_remaining",
			"career_tribute_points",
			"tribute_points",
			"tribute_active",
			"pvp_status",
			"pvp_kills",
			"pvp_deaths",
			"pvp_current_points",
			"pvp_career_points",
			"pvp_best_kill_streak",
			"pvp_worst_death_streak",
			"pvp_current_kill_streak",
			"pvp2",
			"pvp_type",
			"show_helm",
			"group_auto_consent",
			"raid_auto_consent",
			"guild_auto_consent",
			"leadership_exp_on",
			"RestTimer",
			"air_remaining",
			"autosplit_enabled",
			"lfp",
			"lfg",
			"mailkey",
			"xtargets",
			"firstlogon",
			"e_aa_effects",
			"e_percent_to_aa",
			"e_expended_aa_spent",
			"aa_points_spent_old",
			"aa_points_old",
			"e_last_invsnapshot",
			"UNIX_TIMESTAMP(deleted_at)",
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
		return std::string("character_data");
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

	static CharacterData NewEntity()
	{
		CharacterData e{};

		e.id                      = 0;
		e.account_id              = 0;
		e.name                    = "";
		e.last_name               = "";
		e.title                   = "";
		e.suffix                  = "";
		e.zone_id                 = 0;
		e.zone_instance           = 0;
		e.y                       = 0;
		e.x                       = 0;
		e.z                       = 0;
		e.heading                 = 0;
		e.gender                  = 0;
		e.race                    = 0;
		e.class_                  = 0;
		e.level                   = 0;
		e.deity                   = 0;
		e.birthday                = 0;
		e.last_login              = 0;
		e.time_played             = 0;
		e.level2                  = 0;
		e.anon                    = 0;
		e.gm                      = 0;
		e.face                    = 0;
		e.hair_color              = 0;
		e.hair_style              = 0;
		e.beard                   = 0;
		e.beard_color             = 0;
		e.eye_color_1             = 0;
		e.eye_color_2             = 0;
		e.drakkin_heritage        = 0;
		e.drakkin_tattoo          = 0;
		e.drakkin_details         = 0;
		e.ability_time_seconds    = 0;
		e.ability_number          = 0;
		e.ability_time_minutes    = 0;
		e.ability_time_hours      = 0;
		e.exp                     = 0;
		e.aa_points_spent         = 0;
		e.aa_exp                  = 0;
		e.aa_points               = 0;
		e.group_leadership_exp    = 0;
		e.raid_leadership_exp     = 0;
		e.group_leadership_points = 0;
		e.raid_leadership_points  = 0;
		e.points                  = 0;
		e.cur_hp                  = 0;
		e.mana                    = 0;
		e.endurance               = 0;
		e.intoxication            = 0;
		e.str                     = 0;
		e.sta                     = 0;
		e.cha                     = 0;
		e.dex                     = 0;
		e.int_                    = 0;
		e.agi                     = 0;
		e.wis                     = 0;
		e.zone_change_count       = 0;
		e.toxicity                = 0;
		e.hunger_level            = 0;
		e.thirst_level            = 0;
		e.ability_up              = 0;
		e.ldon_points_guk         = 0;
		e.ldon_points_mir         = 0;
		e.ldon_points_mmc         = 0;
		e.ldon_points_ruj         = 0;
		e.ldon_points_tak         = 0;
		e.ldon_points_available   = 0;
		e.tribute_time_remaining  = 0;
		e.career_tribute_points   = 0;
		e.tribute_points          = 0;
		e.tribute_active          = 0;
		e.pvp_status              = 0;
		e.pvp_kills               = 0;
		e.pvp_deaths              = 0;
		e.pvp_current_points      = 0;
		e.pvp_career_points       = 0;
		e.pvp_best_kill_streak    = 0;
		e.pvp_worst_death_streak  = 0;
		e.pvp_current_kill_streak = 0;
		e.pvp2                    = 0;
		e.pvp_type                = 0;
		e.show_helm               = 0;
		e.group_auto_consent      = 0;
		e.raid_auto_consent       = 0;
		e.guild_auto_consent      = 0;
		e.leadership_exp_on       = 0;
		e.RestTimer               = 0;
		e.air_remaining           = 0;
		e.autosplit_enabled       = 0;
		e.lfp                     = 0;
		e.lfg                     = 0;
		e.mailkey                 = "";
		e.xtargets                = 5;
		e.firstlogon              = 0;
		e.e_aa_effects            = 0;
		e.e_percent_to_aa         = 0;
		e.e_expended_aa_spent     = 0;
		e.aa_points_spent_old     = 0;
		e.aa_points_old           = 0;
		e.e_last_invsnapshot      = 0;
		e.deleted_at              = 0;

		return e;
	}

	static CharacterData GetCharacterData(
		const std::vector<CharacterData> &character_datas,
		int character_data_id
	)
	{
		for (auto &character_data : character_datas) {
			if (character_data.id == character_data_id) {
				return character_data;
			}
		}

		return NewEntity();
	}

	static CharacterData FindOne(
		Database& db,
		int character_data_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_data_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterData e{};

			e.id                      = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.account_id              = static_cast<int32_t>(atoi(row[1]));
			e.name                    = row[2] ? row[2] : "";
			e.last_name               = row[3] ? row[3] : "";
			e.title                   = row[4] ? row[4] : "";
			e.suffix                  = row[5] ? row[5] : "";
			e.zone_id                 = static_cast<uint32_t>(strtoul(row[6], nullptr, 10));
			e.zone_instance           = static_cast<uint32_t>(strtoul(row[7], nullptr, 10));
			e.y                       = strtof(row[8], nullptr);
			e.x                       = strtof(row[9], nullptr);
			e.z                       = strtof(row[10], nullptr);
			e.heading                 = strtof(row[11], nullptr);
			e.gender                  = static_cast<uint8_t>(strtoul(row[12], nullptr, 10));
			e.race                    = static_cast<uint16_t>(strtoul(row[13], nullptr, 10));
			e.class_                  = static_cast<uint8_t>(strtoul(row[14], nullptr, 10));
			e.level                   = static_cast<uint32_t>(strtoul(row[15], nullptr, 10));
			e.deity                   = static_cast<uint32_t>(strtoul(row[16], nullptr, 10));
			e.birthday                = static_cast<uint32_t>(strtoul(row[17], nullptr, 10));
			e.last_login              = static_cast<uint32_t>(strtoul(row[18], nullptr, 10));
			e.time_played             = static_cast<uint32_t>(strtoul(row[19], nullptr, 10));
			e.level2                  = static_cast<uint8_t>(strtoul(row[20], nullptr, 10));
			e.anon                    = static_cast<uint8_t>(strtoul(row[21], nullptr, 10));
			e.gm                      = static_cast<uint8_t>(strtoul(row[22], nullptr, 10));
			e.face                    = static_cast<uint32_t>(strtoul(row[23], nullptr, 10));
			e.hair_color              = static_cast<uint8_t>(strtoul(row[24], nullptr, 10));
			e.hair_style              = static_cast<uint8_t>(strtoul(row[25], nullptr, 10));
			e.beard                   = static_cast<uint8_t>(strtoul(row[26], nullptr, 10));
			e.beard_color             = static_cast<uint8_t>(strtoul(row[27], nullptr, 10));
			e.eye_color_1             = static_cast<uint8_t>(strtoul(row[28], nullptr, 10));
			e.eye_color_2             = static_cast<uint8_t>(strtoul(row[29], nullptr, 10));
			e.drakkin_heritage        = static_cast<uint32_t>(strtoul(row[30], nullptr, 10));
			e.drakkin_tattoo          = static_cast<uint32_t>(strtoul(row[31], nullptr, 10));
			e.drakkin_details         = static_cast<uint32_t>(strtoul(row[32], nullptr, 10));
			e.ability_time_seconds    = static_cast<uint8_t>(strtoul(row[33], nullptr, 10));
			e.ability_number          = static_cast<uint8_t>(strtoul(row[34], nullptr, 10));
			e.ability_time_minutes    = static_cast<uint8_t>(strtoul(row[35], nullptr, 10));
			e.ability_time_hours      = static_cast<uint8_t>(strtoul(row[36], nullptr, 10));
			e.exp                     = static_cast<uint32_t>(strtoul(row[37], nullptr, 10));
			e.aa_points_spent         = static_cast<uint32_t>(strtoul(row[38], nullptr, 10));
			e.aa_exp                  = static_cast<uint32_t>(strtoul(row[39], nullptr, 10));
			e.aa_points               = static_cast<uint32_t>(strtoul(row[40], nullptr, 10));
			e.group_leadership_exp    = static_cast<uint32_t>(strtoul(row[41], nullptr, 10));
			e.raid_leadership_exp     = static_cast<uint32_t>(strtoul(row[42], nullptr, 10));
			e.group_leadership_points = static_cast<uint32_t>(strtoul(row[43], nullptr, 10));
			e.raid_leadership_points  = static_cast<uint32_t>(strtoul(row[44], nullptr, 10));
			e.points                  = static_cast<uint32_t>(strtoul(row[45], nullptr, 10));
			e.cur_hp                  = static_cast<uint32_t>(strtoul(row[46], nullptr, 10));
			e.mana                    = static_cast<uint32_t>(strtoul(row[47], nullptr, 10));
			e.endurance               = static_cast<uint32_t>(strtoul(row[48], nullptr, 10));
			e.intoxication            = static_cast<uint32_t>(strtoul(row[49], nullptr, 10));
			e.str                     = static_cast<uint32_t>(strtoul(row[50], nullptr, 10));
			e.sta                     = static_cast<uint32_t>(strtoul(row[51], nullptr, 10));
			e.cha                     = static_cast<uint32_t>(strtoul(row[52], nullptr, 10));
			e.dex                     = static_cast<uint32_t>(strtoul(row[53], nullptr, 10));
			e.int_                    = static_cast<uint32_t>(strtoul(row[54], nullptr, 10));
			e.agi                     = static_cast<uint32_t>(strtoul(row[55], nullptr, 10));
			e.wis                     = static_cast<uint32_t>(strtoul(row[56], nullptr, 10));
			e.zone_change_count       = static_cast<uint32_t>(strtoul(row[57], nullptr, 10));
			e.toxicity                = static_cast<uint32_t>(strtoul(row[58], nullptr, 10));
			e.hunger_level            = static_cast<uint32_t>(strtoul(row[59], nullptr, 10));
			e.thirst_level            = static_cast<uint32_t>(strtoul(row[60], nullptr, 10));
			e.ability_up              = static_cast<uint32_t>(strtoul(row[61], nullptr, 10));
			e.ldon_points_guk         = static_cast<uint32_t>(strtoul(row[62], nullptr, 10));
			e.ldon_points_mir         = static_cast<uint32_t>(strtoul(row[63], nullptr, 10));
			e.ldon_points_mmc         = static_cast<uint32_t>(strtoul(row[64], nullptr, 10));
			e.ldon_points_ruj         = static_cast<uint32_t>(strtoul(row[65], nullptr, 10));
			e.ldon_points_tak         = static_cast<uint32_t>(strtoul(row[66], nullptr, 10));
			e.ldon_points_available   = static_cast<uint32_t>(strtoul(row[67], nullptr, 10));
			e.tribute_time_remaining  = static_cast<uint32_t>(strtoul(row[68], nullptr, 10));
			e.career_tribute_points   = static_cast<uint32_t>(strtoul(row[69], nullptr, 10));
			e.tribute_points          = static_cast<uint32_t>(strtoul(row[70], nullptr, 10));
			e.tribute_active          = static_cast<uint32_t>(strtoul(row[71], nullptr, 10));
			e.pvp_status              = static_cast<uint8_t>(strtoul(row[72], nullptr, 10));
			e.pvp_kills               = static_cast<uint32_t>(strtoul(row[73], nullptr, 10));
			e.pvp_deaths              = static_cast<uint32_t>(strtoul(row[74], nullptr, 10));
			e.pvp_current_points      = static_cast<uint32_t>(strtoul(row[75], nullptr, 10));
			e.pvp_career_points       = static_cast<uint32_t>(strtoul(row[76], nullptr, 10));
			e.pvp_best_kill_streak    = static_cast<uint32_t>(strtoul(row[77], nullptr, 10));
			e.pvp_worst_death_streak  = static_cast<uint32_t>(strtoul(row[78], nullptr, 10));
			e.pvp_current_kill_streak = static_cast<uint32_t>(strtoul(row[79], nullptr, 10));
			e.pvp2                    = static_cast<uint32_t>(strtoul(row[80], nullptr, 10));
			e.pvp_type                = static_cast<uint32_t>(strtoul(row[81], nullptr, 10));
			e.show_helm               = static_cast<uint32_t>(strtoul(row[82], nullptr, 10));
			e.group_auto_consent      = static_cast<uint8_t>(strtoul(row[83], nullptr, 10));
			e.raid_auto_consent       = static_cast<uint8_t>(strtoul(row[84], nullptr, 10));
			e.guild_auto_consent      = static_cast<uint8_t>(strtoul(row[85], nullptr, 10));
			e.leadership_exp_on       = static_cast<uint8_t>(strtoul(row[86], nullptr, 10));
			e.RestTimer               = static_cast<uint32_t>(strtoul(row[87], nullptr, 10));
			e.air_remaining           = static_cast<uint32_t>(strtoul(row[88], nullptr, 10));
			e.autosplit_enabled       = static_cast<uint32_t>(strtoul(row[89], nullptr, 10));
			e.lfp                     = static_cast<uint8_t>(strtoul(row[90], nullptr, 10));
			e.lfg                     = static_cast<uint8_t>(strtoul(row[91], nullptr, 10));
			e.mailkey                 = row[92] ? row[92] : "";
			e.xtargets                = static_cast<uint8_t>(strtoul(row[93], nullptr, 10));
			e.firstlogon              = static_cast<int8_t>(atoi(row[94]));
			e.e_aa_effects            = static_cast<uint32_t>(strtoul(row[95], nullptr, 10));
			e.e_percent_to_aa         = static_cast<uint32_t>(strtoul(row[96], nullptr, 10));
			e.e_expended_aa_spent     = static_cast<uint32_t>(strtoul(row[97], nullptr, 10));
			e.aa_points_spent_old     = static_cast<uint32_t>(strtoul(row[98], nullptr, 10));
			e.aa_points_old           = static_cast<uint32_t>(strtoul(row[99], nullptr, 10));
			e.e_last_invsnapshot      = static_cast<uint32_t>(strtoul(row[100], nullptr, 10));
			e.deleted_at              = strtoll(row[101] ? row[101] : "-1", nullptr, 10);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_data_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_data_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterData &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.account_id));
		v.push_back(columns[2] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[3] + " = '" + Strings::Escape(e.last_name) + "'");
		v.push_back(columns[4] + " = '" + Strings::Escape(e.title) + "'");
		v.push_back(columns[5] + " = '" + Strings::Escape(e.suffix) + "'");
		v.push_back(columns[6] + " = " + std::to_string(e.zone_id));
		v.push_back(columns[7] + " = " + std::to_string(e.zone_instance));
		v.push_back(columns[8] + " = " + std::to_string(e.y));
		v.push_back(columns[9] + " = " + std::to_string(e.x));
		v.push_back(columns[10] + " = " + std::to_string(e.z));
		v.push_back(columns[11] + " = " + std::to_string(e.heading));
		v.push_back(columns[12] + " = " + std::to_string(e.gender));
		v.push_back(columns[13] + " = " + std::to_string(e.race));
		v.push_back(columns[14] + " = " + std::to_string(e.class_));
		v.push_back(columns[15] + " = " + std::to_string(e.level));
		v.push_back(columns[16] + " = " + std::to_string(e.deity));
		v.push_back(columns[17] + " = " + std::to_string(e.birthday));
		v.push_back(columns[18] + " = " + std::to_string(e.last_login));
		v.push_back(columns[19] + " = " + std::to_string(e.time_played));
		v.push_back(columns[20] + " = " + std::to_string(e.level2));
		v.push_back(columns[21] + " = " + std::to_string(e.anon));
		v.push_back(columns[22] + " = " + std::to_string(e.gm));
		v.push_back(columns[23] + " = " + std::to_string(e.face));
		v.push_back(columns[24] + " = " + std::to_string(e.hair_color));
		v.push_back(columns[25] + " = " + std::to_string(e.hair_style));
		v.push_back(columns[26] + " = " + std::to_string(e.beard));
		v.push_back(columns[27] + " = " + std::to_string(e.beard_color));
		v.push_back(columns[28] + " = " + std::to_string(e.eye_color_1));
		v.push_back(columns[29] + " = " + std::to_string(e.eye_color_2));
		v.push_back(columns[30] + " = " + std::to_string(e.drakkin_heritage));
		v.push_back(columns[31] + " = " + std::to_string(e.drakkin_tattoo));
		v.push_back(columns[32] + " = " + std::to_string(e.drakkin_details));
		v.push_back(columns[33] + " = " + std::to_string(e.ability_time_seconds));
		v.push_back(columns[34] + " = " + std::to_string(e.ability_number));
		v.push_back(columns[35] + " = " + std::to_string(e.ability_time_minutes));
		v.push_back(columns[36] + " = " + std::to_string(e.ability_time_hours));
		v.push_back(columns[37] + " = " + std::to_string(e.exp));
		v.push_back(columns[38] + " = " + std::to_string(e.aa_points_spent));
		v.push_back(columns[39] + " = " + std::to_string(e.aa_exp));
		v.push_back(columns[40] + " = " + std::to_string(e.aa_points));
		v.push_back(columns[41] + " = " + std::to_string(e.group_leadership_exp));
		v.push_back(columns[42] + " = " + std::to_string(e.raid_leadership_exp));
		v.push_back(columns[43] + " = " + std::to_string(e.group_leadership_points));
		v.push_back(columns[44] + " = " + std::to_string(e.raid_leadership_points));
		v.push_back(columns[45] + " = " + std::to_string(e.points));
		v.push_back(columns[46] + " = " + std::to_string(e.cur_hp));
		v.push_back(columns[47] + " = " + std::to_string(e.mana));
		v.push_back(columns[48] + " = " + std::to_string(e.endurance));
		v.push_back(columns[49] + " = " + std::to_string(e.intoxication));
		v.push_back(columns[50] + " = " + std::to_string(e.str));
		v.push_back(columns[51] + " = " + std::to_string(e.sta));
		v.push_back(columns[52] + " = " + std::to_string(e.cha));
		v.push_back(columns[53] + " = " + std::to_string(e.dex));
		v.push_back(columns[54] + " = " + std::to_string(e.int_));
		v.push_back(columns[55] + " = " + std::to_string(e.agi));
		v.push_back(columns[56] + " = " + std::to_string(e.wis));
		v.push_back(columns[57] + " = " + std::to_string(e.zone_change_count));
		v.push_back(columns[58] + " = " + std::to_string(e.toxicity));
		v.push_back(columns[59] + " = " + std::to_string(e.hunger_level));
		v.push_back(columns[60] + " = " + std::to_string(e.thirst_level));
		v.push_back(columns[61] + " = " + std::to_string(e.ability_up));
		v.push_back(columns[62] + " = " + std::to_string(e.ldon_points_guk));
		v.push_back(columns[63] + " = " + std::to_string(e.ldon_points_mir));
		v.push_back(columns[64] + " = " + std::to_string(e.ldon_points_mmc));
		v.push_back(columns[65] + " = " + std::to_string(e.ldon_points_ruj));
		v.push_back(columns[66] + " = " + std::to_string(e.ldon_points_tak));
		v.push_back(columns[67] + " = " + std::to_string(e.ldon_points_available));
		v.push_back(columns[68] + " = " + std::to_string(e.tribute_time_remaining));
		v.push_back(columns[69] + " = " + std::to_string(e.career_tribute_points));
		v.push_back(columns[70] + " = " + std::to_string(e.tribute_points));
		v.push_back(columns[71] + " = " + std::to_string(e.tribute_active));
		v.push_back(columns[72] + " = " + std::to_string(e.pvp_status));
		v.push_back(columns[73] + " = " + std::to_string(e.pvp_kills));
		v.push_back(columns[74] + " = " + std::to_string(e.pvp_deaths));
		v.push_back(columns[75] + " = " + std::to_string(e.pvp_current_points));
		v.push_back(columns[76] + " = " + std::to_string(e.pvp_career_points));
		v.push_back(columns[77] + " = " + std::to_string(e.pvp_best_kill_streak));
		v.push_back(columns[78] + " = " + std::to_string(e.pvp_worst_death_streak));
		v.push_back(columns[79] + " = " + std::to_string(e.pvp_current_kill_streak));
		v.push_back(columns[80] + " = " + std::to_string(e.pvp2));
		v.push_back(columns[81] + " = " + std::to_string(e.pvp_type));
		v.push_back(columns[82] + " = " + std::to_string(e.show_helm));
		v.push_back(columns[83] + " = " + std::to_string(e.group_auto_consent));
		v.push_back(columns[84] + " = " + std::to_string(e.raid_auto_consent));
		v.push_back(columns[85] + " = " + std::to_string(e.guild_auto_consent));
		v.push_back(columns[86] + " = " + std::to_string(e.leadership_exp_on));
		v.push_back(columns[87] + " = " + std::to_string(e.RestTimer));
		v.push_back(columns[88] + " = " + std::to_string(e.air_remaining));
		v.push_back(columns[89] + " = " + std::to_string(e.autosplit_enabled));
		v.push_back(columns[90] + " = " + std::to_string(e.lfp));
		v.push_back(columns[91] + " = " + std::to_string(e.lfg));
		v.push_back(columns[92] + " = '" + Strings::Escape(e.mailkey) + "'");
		v.push_back(columns[93] + " = " + std::to_string(e.xtargets));
		v.push_back(columns[94] + " = " + std::to_string(e.firstlogon));
		v.push_back(columns[95] + " = " + std::to_string(e.e_aa_effects));
		v.push_back(columns[96] + " = " + std::to_string(e.e_percent_to_aa));
		v.push_back(columns[97] + " = " + std::to_string(e.e_expended_aa_spent));
		v.push_back(columns[98] + " = " + std::to_string(e.aa_points_spent_old));
		v.push_back(columns[99] + " = " + std::to_string(e.aa_points_old));
		v.push_back(columns[100] + " = " + std::to_string(e.e_last_invsnapshot));
		v.push_back(columns[101] + " = FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");

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

	static CharacterData InsertOne(
		Database& db,
		CharacterData e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.account_id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back("'" + Strings::Escape(e.last_name) + "'");
		v.push_back("'" + Strings::Escape(e.title) + "'");
		v.push_back("'" + Strings::Escape(e.suffix) + "'");
		v.push_back(std::to_string(e.zone_id));
		v.push_back(std::to_string(e.zone_instance));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.z));
		v.push_back(std::to_string(e.heading));
		v.push_back(std::to_string(e.gender));
		v.push_back(std::to_string(e.race));
		v.push_back(std::to_string(e.class_));
		v.push_back(std::to_string(e.level));
		v.push_back(std::to_string(e.deity));
		v.push_back(std::to_string(e.birthday));
		v.push_back(std::to_string(e.last_login));
		v.push_back(std::to_string(e.time_played));
		v.push_back(std::to_string(e.level2));
		v.push_back(std::to_string(e.anon));
		v.push_back(std::to_string(e.gm));
		v.push_back(std::to_string(e.face));
		v.push_back(std::to_string(e.hair_color));
		v.push_back(std::to_string(e.hair_style));
		v.push_back(std::to_string(e.beard));
		v.push_back(std::to_string(e.beard_color));
		v.push_back(std::to_string(e.eye_color_1));
		v.push_back(std::to_string(e.eye_color_2));
		v.push_back(std::to_string(e.drakkin_heritage));
		v.push_back(std::to_string(e.drakkin_tattoo));
		v.push_back(std::to_string(e.drakkin_details));
		v.push_back(std::to_string(e.ability_time_seconds));
		v.push_back(std::to_string(e.ability_number));
		v.push_back(std::to_string(e.ability_time_minutes));
		v.push_back(std::to_string(e.ability_time_hours));
		v.push_back(std::to_string(e.exp));
		v.push_back(std::to_string(e.aa_points_spent));
		v.push_back(std::to_string(e.aa_exp));
		v.push_back(std::to_string(e.aa_points));
		v.push_back(std::to_string(e.group_leadership_exp));
		v.push_back(std::to_string(e.raid_leadership_exp));
		v.push_back(std::to_string(e.group_leadership_points));
		v.push_back(std::to_string(e.raid_leadership_points));
		v.push_back(std::to_string(e.points));
		v.push_back(std::to_string(e.cur_hp));
		v.push_back(std::to_string(e.mana));
		v.push_back(std::to_string(e.endurance));
		v.push_back(std::to_string(e.intoxication));
		v.push_back(std::to_string(e.str));
		v.push_back(std::to_string(e.sta));
		v.push_back(std::to_string(e.cha));
		v.push_back(std::to_string(e.dex));
		v.push_back(std::to_string(e.int_));
		v.push_back(std::to_string(e.agi));
		v.push_back(std::to_string(e.wis));
		v.push_back(std::to_string(e.zone_change_count));
		v.push_back(std::to_string(e.toxicity));
		v.push_back(std::to_string(e.hunger_level));
		v.push_back(std::to_string(e.thirst_level));
		v.push_back(std::to_string(e.ability_up));
		v.push_back(std::to_string(e.ldon_points_guk));
		v.push_back(std::to_string(e.ldon_points_mir));
		v.push_back(std::to_string(e.ldon_points_mmc));
		v.push_back(std::to_string(e.ldon_points_ruj));
		v.push_back(std::to_string(e.ldon_points_tak));
		v.push_back(std::to_string(e.ldon_points_available));
		v.push_back(std::to_string(e.tribute_time_remaining));
		v.push_back(std::to_string(e.career_tribute_points));
		v.push_back(std::to_string(e.tribute_points));
		v.push_back(std::to_string(e.tribute_active));
		v.push_back(std::to_string(e.pvp_status));
		v.push_back(std::to_string(e.pvp_kills));
		v.push_back(std::to_string(e.pvp_deaths));
		v.push_back(std::to_string(e.pvp_current_points));
		v.push_back(std::to_string(e.pvp_career_points));
		v.push_back(std::to_string(e.pvp_best_kill_streak));
		v.push_back(std::to_string(e.pvp_worst_death_streak));
		v.push_back(std::to_string(e.pvp_current_kill_streak));
		v.push_back(std::to_string(e.pvp2));
		v.push_back(std::to_string(e.pvp_type));
		v.push_back(std::to_string(e.show_helm));
		v.push_back(std::to_string(e.group_auto_consent));
		v.push_back(std::to_string(e.raid_auto_consent));
		v.push_back(std::to_string(e.guild_auto_consent));
		v.push_back(std::to_string(e.leadership_exp_on));
		v.push_back(std::to_string(e.RestTimer));
		v.push_back(std::to_string(e.air_remaining));
		v.push_back(std::to_string(e.autosplit_enabled));
		v.push_back(std::to_string(e.lfp));
		v.push_back(std::to_string(e.lfg));
		v.push_back("'" + Strings::Escape(e.mailkey) + "'");
		v.push_back(std::to_string(e.xtargets));
		v.push_back(std::to_string(e.firstlogon));
		v.push_back(std::to_string(e.e_aa_effects));
		v.push_back(std::to_string(e.e_percent_to_aa));
		v.push_back(std::to_string(e.e_expended_aa_spent));
		v.push_back(std::to_string(e.aa_points_spent_old));
		v.push_back(std::to_string(e.aa_points_old));
		v.push_back(std::to_string(e.e_last_invsnapshot));
		v.push_back("FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");

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
		const std::vector<CharacterData> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.account_id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back("'" + Strings::Escape(e.last_name) + "'");
			v.push_back("'" + Strings::Escape(e.title) + "'");
			v.push_back("'" + Strings::Escape(e.suffix) + "'");
			v.push_back(std::to_string(e.zone_id));
			v.push_back(std::to_string(e.zone_instance));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.z));
			v.push_back(std::to_string(e.heading));
			v.push_back(std::to_string(e.gender));
			v.push_back(std::to_string(e.race));
			v.push_back(std::to_string(e.class_));
			v.push_back(std::to_string(e.level));
			v.push_back(std::to_string(e.deity));
			v.push_back(std::to_string(e.birthday));
			v.push_back(std::to_string(e.last_login));
			v.push_back(std::to_string(e.time_played));
			v.push_back(std::to_string(e.level2));
			v.push_back(std::to_string(e.anon));
			v.push_back(std::to_string(e.gm));
			v.push_back(std::to_string(e.face));
			v.push_back(std::to_string(e.hair_color));
			v.push_back(std::to_string(e.hair_style));
			v.push_back(std::to_string(e.beard));
			v.push_back(std::to_string(e.beard_color));
			v.push_back(std::to_string(e.eye_color_1));
			v.push_back(std::to_string(e.eye_color_2));
			v.push_back(std::to_string(e.drakkin_heritage));
			v.push_back(std::to_string(e.drakkin_tattoo));
			v.push_back(std::to_string(e.drakkin_details));
			v.push_back(std::to_string(e.ability_time_seconds));
			v.push_back(std::to_string(e.ability_number));
			v.push_back(std::to_string(e.ability_time_minutes));
			v.push_back(std::to_string(e.ability_time_hours));
			v.push_back(std::to_string(e.exp));
			v.push_back(std::to_string(e.aa_points_spent));
			v.push_back(std::to_string(e.aa_exp));
			v.push_back(std::to_string(e.aa_points));
			v.push_back(std::to_string(e.group_leadership_exp));
			v.push_back(std::to_string(e.raid_leadership_exp));
			v.push_back(std::to_string(e.group_leadership_points));
			v.push_back(std::to_string(e.raid_leadership_points));
			v.push_back(std::to_string(e.points));
			v.push_back(std::to_string(e.cur_hp));
			v.push_back(std::to_string(e.mana));
			v.push_back(std::to_string(e.endurance));
			v.push_back(std::to_string(e.intoxication));
			v.push_back(std::to_string(e.str));
			v.push_back(std::to_string(e.sta));
			v.push_back(std::to_string(e.cha));
			v.push_back(std::to_string(e.dex));
			v.push_back(std::to_string(e.int_));
			v.push_back(std::to_string(e.agi));
			v.push_back(std::to_string(e.wis));
			v.push_back(std::to_string(e.zone_change_count));
			v.push_back(std::to_string(e.toxicity));
			v.push_back(std::to_string(e.hunger_level));
			v.push_back(std::to_string(e.thirst_level));
			v.push_back(std::to_string(e.ability_up));
			v.push_back(std::to_string(e.ldon_points_guk));
			v.push_back(std::to_string(e.ldon_points_mir));
			v.push_back(std::to_string(e.ldon_points_mmc));
			v.push_back(std::to_string(e.ldon_points_ruj));
			v.push_back(std::to_string(e.ldon_points_tak));
			v.push_back(std::to_string(e.ldon_points_available));
			v.push_back(std::to_string(e.tribute_time_remaining));
			v.push_back(std::to_string(e.career_tribute_points));
			v.push_back(std::to_string(e.tribute_points));
			v.push_back(std::to_string(e.tribute_active));
			v.push_back(std::to_string(e.pvp_status));
			v.push_back(std::to_string(e.pvp_kills));
			v.push_back(std::to_string(e.pvp_deaths));
			v.push_back(std::to_string(e.pvp_current_points));
			v.push_back(std::to_string(e.pvp_career_points));
			v.push_back(std::to_string(e.pvp_best_kill_streak));
			v.push_back(std::to_string(e.pvp_worst_death_streak));
			v.push_back(std::to_string(e.pvp_current_kill_streak));
			v.push_back(std::to_string(e.pvp2));
			v.push_back(std::to_string(e.pvp_type));
			v.push_back(std::to_string(e.show_helm));
			v.push_back(std::to_string(e.group_auto_consent));
			v.push_back(std::to_string(e.raid_auto_consent));
			v.push_back(std::to_string(e.guild_auto_consent));
			v.push_back(std::to_string(e.leadership_exp_on));
			v.push_back(std::to_string(e.RestTimer));
			v.push_back(std::to_string(e.air_remaining));
			v.push_back(std::to_string(e.autosplit_enabled));
			v.push_back(std::to_string(e.lfp));
			v.push_back(std::to_string(e.lfg));
			v.push_back("'" + Strings::Escape(e.mailkey) + "'");
			v.push_back(std::to_string(e.xtargets));
			v.push_back(std::to_string(e.firstlogon));
			v.push_back(std::to_string(e.e_aa_effects));
			v.push_back(std::to_string(e.e_percent_to_aa));
			v.push_back(std::to_string(e.e_expended_aa_spent));
			v.push_back(std::to_string(e.aa_points_spent_old));
			v.push_back(std::to_string(e.aa_points_old));
			v.push_back(std::to_string(e.e_last_invsnapshot));
			v.push_back("FROM_UNIXTIME(" + (e.deleted_at > 0 ? std::to_string(e.deleted_at) : "null") + ")");

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

	static std::vector<CharacterData> All(Database& db)
	{
		std::vector<CharacterData> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterData e{};

			e.id                      = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.account_id              = static_cast<int32_t>(atoi(row[1]));
			e.name                    = row[2] ? row[2] : "";
			e.last_name               = row[3] ? row[3] : "";
			e.title                   = row[4] ? row[4] : "";
			e.suffix                  = row[5] ? row[5] : "";
			e.zone_id                 = static_cast<uint32_t>(strtoul(row[6], nullptr, 10));
			e.zone_instance           = static_cast<uint32_t>(strtoul(row[7], nullptr, 10));
			e.y                       = strtof(row[8], nullptr);
			e.x                       = strtof(row[9], nullptr);
			e.z                       = strtof(row[10], nullptr);
			e.heading                 = strtof(row[11], nullptr);
			e.gender                  = static_cast<uint8_t>(strtoul(row[12], nullptr, 10));
			e.race                    = static_cast<uint16_t>(strtoul(row[13], nullptr, 10));
			e.class_                  = static_cast<uint8_t>(strtoul(row[14], nullptr, 10));
			e.level                   = static_cast<uint32_t>(strtoul(row[15], nullptr, 10));
			e.deity                   = static_cast<uint32_t>(strtoul(row[16], nullptr, 10));
			e.birthday                = static_cast<uint32_t>(strtoul(row[17], nullptr, 10));
			e.last_login              = static_cast<uint32_t>(strtoul(row[18], nullptr, 10));
			e.time_played             = static_cast<uint32_t>(strtoul(row[19], nullptr, 10));
			e.level2                  = static_cast<uint8_t>(strtoul(row[20], nullptr, 10));
			e.anon                    = static_cast<uint8_t>(strtoul(row[21], nullptr, 10));
			e.gm                      = static_cast<uint8_t>(strtoul(row[22], nullptr, 10));
			e.face                    = static_cast<uint32_t>(strtoul(row[23], nullptr, 10));
			e.hair_color              = static_cast<uint8_t>(strtoul(row[24], nullptr, 10));
			e.hair_style              = static_cast<uint8_t>(strtoul(row[25], nullptr, 10));
			e.beard                   = static_cast<uint8_t>(strtoul(row[26], nullptr, 10));
			e.beard_color             = static_cast<uint8_t>(strtoul(row[27], nullptr, 10));
			e.eye_color_1             = static_cast<uint8_t>(strtoul(row[28], nullptr, 10));
			e.eye_color_2             = static_cast<uint8_t>(strtoul(row[29], nullptr, 10));
			e.drakkin_heritage        = static_cast<uint32_t>(strtoul(row[30], nullptr, 10));
			e.drakkin_tattoo          = static_cast<uint32_t>(strtoul(row[31], nullptr, 10));
			e.drakkin_details         = static_cast<uint32_t>(strtoul(row[32], nullptr, 10));
			e.ability_time_seconds    = static_cast<uint8_t>(strtoul(row[33], nullptr, 10));
			e.ability_number          = static_cast<uint8_t>(strtoul(row[34], nullptr, 10));
			e.ability_time_minutes    = static_cast<uint8_t>(strtoul(row[35], nullptr, 10));
			e.ability_time_hours      = static_cast<uint8_t>(strtoul(row[36], nullptr, 10));
			e.exp                     = static_cast<uint32_t>(strtoul(row[37], nullptr, 10));
			e.aa_points_spent         = static_cast<uint32_t>(strtoul(row[38], nullptr, 10));
			e.aa_exp                  = static_cast<uint32_t>(strtoul(row[39], nullptr, 10));
			e.aa_points               = static_cast<uint32_t>(strtoul(row[40], nullptr, 10));
			e.group_leadership_exp    = static_cast<uint32_t>(strtoul(row[41], nullptr, 10));
			e.raid_leadership_exp     = static_cast<uint32_t>(strtoul(row[42], nullptr, 10));
			e.group_leadership_points = static_cast<uint32_t>(strtoul(row[43], nullptr, 10));
			e.raid_leadership_points  = static_cast<uint32_t>(strtoul(row[44], nullptr, 10));
			e.points                  = static_cast<uint32_t>(strtoul(row[45], nullptr, 10));
			e.cur_hp                  = static_cast<uint32_t>(strtoul(row[46], nullptr, 10));
			e.mana                    = static_cast<uint32_t>(strtoul(row[47], nullptr, 10));
			e.endurance               = static_cast<uint32_t>(strtoul(row[48], nullptr, 10));
			e.intoxication            = static_cast<uint32_t>(strtoul(row[49], nullptr, 10));
			e.str                     = static_cast<uint32_t>(strtoul(row[50], nullptr, 10));
			e.sta                     = static_cast<uint32_t>(strtoul(row[51], nullptr, 10));
			e.cha                     = static_cast<uint32_t>(strtoul(row[52], nullptr, 10));
			e.dex                     = static_cast<uint32_t>(strtoul(row[53], nullptr, 10));
			e.int_                    = static_cast<uint32_t>(strtoul(row[54], nullptr, 10));
			e.agi                     = static_cast<uint32_t>(strtoul(row[55], nullptr, 10));
			e.wis                     = static_cast<uint32_t>(strtoul(row[56], nullptr, 10));
			e.zone_change_count       = static_cast<uint32_t>(strtoul(row[57], nullptr, 10));
			e.toxicity                = static_cast<uint32_t>(strtoul(row[58], nullptr, 10));
			e.hunger_level            = static_cast<uint32_t>(strtoul(row[59], nullptr, 10));
			e.thirst_level            = static_cast<uint32_t>(strtoul(row[60], nullptr, 10));
			e.ability_up              = static_cast<uint32_t>(strtoul(row[61], nullptr, 10));
			e.ldon_points_guk         = static_cast<uint32_t>(strtoul(row[62], nullptr, 10));
			e.ldon_points_mir         = static_cast<uint32_t>(strtoul(row[63], nullptr, 10));
			e.ldon_points_mmc         = static_cast<uint32_t>(strtoul(row[64], nullptr, 10));
			e.ldon_points_ruj         = static_cast<uint32_t>(strtoul(row[65], nullptr, 10));
			e.ldon_points_tak         = static_cast<uint32_t>(strtoul(row[66], nullptr, 10));
			e.ldon_points_available   = static_cast<uint32_t>(strtoul(row[67], nullptr, 10));
			e.tribute_time_remaining  = static_cast<uint32_t>(strtoul(row[68], nullptr, 10));
			e.career_tribute_points   = static_cast<uint32_t>(strtoul(row[69], nullptr, 10));
			e.tribute_points          = static_cast<uint32_t>(strtoul(row[70], nullptr, 10));
			e.tribute_active          = static_cast<uint32_t>(strtoul(row[71], nullptr, 10));
			e.pvp_status              = static_cast<uint8_t>(strtoul(row[72], nullptr, 10));
			e.pvp_kills               = static_cast<uint32_t>(strtoul(row[73], nullptr, 10));
			e.pvp_deaths              = static_cast<uint32_t>(strtoul(row[74], nullptr, 10));
			e.pvp_current_points      = static_cast<uint32_t>(strtoul(row[75], nullptr, 10));
			e.pvp_career_points       = static_cast<uint32_t>(strtoul(row[76], nullptr, 10));
			e.pvp_best_kill_streak    = static_cast<uint32_t>(strtoul(row[77], nullptr, 10));
			e.pvp_worst_death_streak  = static_cast<uint32_t>(strtoul(row[78], nullptr, 10));
			e.pvp_current_kill_streak = static_cast<uint32_t>(strtoul(row[79], nullptr, 10));
			e.pvp2                    = static_cast<uint32_t>(strtoul(row[80], nullptr, 10));
			e.pvp_type                = static_cast<uint32_t>(strtoul(row[81], nullptr, 10));
			e.show_helm               = static_cast<uint32_t>(strtoul(row[82], nullptr, 10));
			e.group_auto_consent      = static_cast<uint8_t>(strtoul(row[83], nullptr, 10));
			e.raid_auto_consent       = static_cast<uint8_t>(strtoul(row[84], nullptr, 10));
			e.guild_auto_consent      = static_cast<uint8_t>(strtoul(row[85], nullptr, 10));
			e.leadership_exp_on       = static_cast<uint8_t>(strtoul(row[86], nullptr, 10));
			e.RestTimer               = static_cast<uint32_t>(strtoul(row[87], nullptr, 10));
			e.air_remaining           = static_cast<uint32_t>(strtoul(row[88], nullptr, 10));
			e.autosplit_enabled       = static_cast<uint32_t>(strtoul(row[89], nullptr, 10));
			e.lfp                     = static_cast<uint8_t>(strtoul(row[90], nullptr, 10));
			e.lfg                     = static_cast<uint8_t>(strtoul(row[91], nullptr, 10));
			e.mailkey                 = row[92] ? row[92] : "";
			e.xtargets                = static_cast<uint8_t>(strtoul(row[93], nullptr, 10));
			e.firstlogon              = static_cast<int8_t>(atoi(row[94]));
			e.e_aa_effects            = static_cast<uint32_t>(strtoul(row[95], nullptr, 10));
			e.e_percent_to_aa         = static_cast<uint32_t>(strtoul(row[96], nullptr, 10));
			e.e_expended_aa_spent     = static_cast<uint32_t>(strtoul(row[97], nullptr, 10));
			e.aa_points_spent_old     = static_cast<uint32_t>(strtoul(row[98], nullptr, 10));
			e.aa_points_old           = static_cast<uint32_t>(strtoul(row[99], nullptr, 10));
			e.e_last_invsnapshot      = static_cast<uint32_t>(strtoul(row[100], nullptr, 10));
			e.deleted_at              = strtoll(row[101] ? row[101] : "-1", nullptr, 10);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterData> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterData> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterData e{};

			e.id                      = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.account_id              = static_cast<int32_t>(atoi(row[1]));
			e.name                    = row[2] ? row[2] : "";
			e.last_name               = row[3] ? row[3] : "";
			e.title                   = row[4] ? row[4] : "";
			e.suffix                  = row[5] ? row[5] : "";
			e.zone_id                 = static_cast<uint32_t>(strtoul(row[6], nullptr, 10));
			e.zone_instance           = static_cast<uint32_t>(strtoul(row[7], nullptr, 10));
			e.y                       = strtof(row[8], nullptr);
			e.x                       = strtof(row[9], nullptr);
			e.z                       = strtof(row[10], nullptr);
			e.heading                 = strtof(row[11], nullptr);
			e.gender                  = static_cast<uint8_t>(strtoul(row[12], nullptr, 10));
			e.race                    = static_cast<uint16_t>(strtoul(row[13], nullptr, 10));
			e.class_                  = static_cast<uint8_t>(strtoul(row[14], nullptr, 10));
			e.level                   = static_cast<uint32_t>(strtoul(row[15], nullptr, 10));
			e.deity                   = static_cast<uint32_t>(strtoul(row[16], nullptr, 10));
			e.birthday                = static_cast<uint32_t>(strtoul(row[17], nullptr, 10));
			e.last_login              = static_cast<uint32_t>(strtoul(row[18], nullptr, 10));
			e.time_played             = static_cast<uint32_t>(strtoul(row[19], nullptr, 10));
			e.level2                  = static_cast<uint8_t>(strtoul(row[20], nullptr, 10));
			e.anon                    = static_cast<uint8_t>(strtoul(row[21], nullptr, 10));
			e.gm                      = static_cast<uint8_t>(strtoul(row[22], nullptr, 10));
			e.face                    = static_cast<uint32_t>(strtoul(row[23], nullptr, 10));
			e.hair_color              = static_cast<uint8_t>(strtoul(row[24], nullptr, 10));
			e.hair_style              = static_cast<uint8_t>(strtoul(row[25], nullptr, 10));
			e.beard                   = static_cast<uint8_t>(strtoul(row[26], nullptr, 10));
			e.beard_color             = static_cast<uint8_t>(strtoul(row[27], nullptr, 10));
			e.eye_color_1             = static_cast<uint8_t>(strtoul(row[28], nullptr, 10));
			e.eye_color_2             = static_cast<uint8_t>(strtoul(row[29], nullptr, 10));
			e.drakkin_heritage        = static_cast<uint32_t>(strtoul(row[30], nullptr, 10));
			e.drakkin_tattoo          = static_cast<uint32_t>(strtoul(row[31], nullptr, 10));
			e.drakkin_details         = static_cast<uint32_t>(strtoul(row[32], nullptr, 10));
			e.ability_time_seconds    = static_cast<uint8_t>(strtoul(row[33], nullptr, 10));
			e.ability_number          = static_cast<uint8_t>(strtoul(row[34], nullptr, 10));
			e.ability_time_minutes    = static_cast<uint8_t>(strtoul(row[35], nullptr, 10));
			e.ability_time_hours      = static_cast<uint8_t>(strtoul(row[36], nullptr, 10));
			e.exp                     = static_cast<uint32_t>(strtoul(row[37], nullptr, 10));
			e.aa_points_spent         = static_cast<uint32_t>(strtoul(row[38], nullptr, 10));
			e.aa_exp                  = static_cast<uint32_t>(strtoul(row[39], nullptr, 10));
			e.aa_points               = static_cast<uint32_t>(strtoul(row[40], nullptr, 10));
			e.group_leadership_exp    = static_cast<uint32_t>(strtoul(row[41], nullptr, 10));
			e.raid_leadership_exp     = static_cast<uint32_t>(strtoul(row[42], nullptr, 10));
			e.group_leadership_points = static_cast<uint32_t>(strtoul(row[43], nullptr, 10));
			e.raid_leadership_points  = static_cast<uint32_t>(strtoul(row[44], nullptr, 10));
			e.points                  = static_cast<uint32_t>(strtoul(row[45], nullptr, 10));
			e.cur_hp                  = static_cast<uint32_t>(strtoul(row[46], nullptr, 10));
			e.mana                    = static_cast<uint32_t>(strtoul(row[47], nullptr, 10));
			e.endurance               = static_cast<uint32_t>(strtoul(row[48], nullptr, 10));
			e.intoxication            = static_cast<uint32_t>(strtoul(row[49], nullptr, 10));
			e.str                     = static_cast<uint32_t>(strtoul(row[50], nullptr, 10));
			e.sta                     = static_cast<uint32_t>(strtoul(row[51], nullptr, 10));
			e.cha                     = static_cast<uint32_t>(strtoul(row[52], nullptr, 10));
			e.dex                     = static_cast<uint32_t>(strtoul(row[53], nullptr, 10));
			e.int_                    = static_cast<uint32_t>(strtoul(row[54], nullptr, 10));
			e.agi                     = static_cast<uint32_t>(strtoul(row[55], nullptr, 10));
			e.wis                     = static_cast<uint32_t>(strtoul(row[56], nullptr, 10));
			e.zone_change_count       = static_cast<uint32_t>(strtoul(row[57], nullptr, 10));
			e.toxicity                = static_cast<uint32_t>(strtoul(row[58], nullptr, 10));
			e.hunger_level            = static_cast<uint32_t>(strtoul(row[59], nullptr, 10));
			e.thirst_level            = static_cast<uint32_t>(strtoul(row[60], nullptr, 10));
			e.ability_up              = static_cast<uint32_t>(strtoul(row[61], nullptr, 10));
			e.ldon_points_guk         = static_cast<uint32_t>(strtoul(row[62], nullptr, 10));
			e.ldon_points_mir         = static_cast<uint32_t>(strtoul(row[63], nullptr, 10));
			e.ldon_points_mmc         = static_cast<uint32_t>(strtoul(row[64], nullptr, 10));
			e.ldon_points_ruj         = static_cast<uint32_t>(strtoul(row[65], nullptr, 10));
			e.ldon_points_tak         = static_cast<uint32_t>(strtoul(row[66], nullptr, 10));
			e.ldon_points_available   = static_cast<uint32_t>(strtoul(row[67], nullptr, 10));
			e.tribute_time_remaining  = static_cast<uint32_t>(strtoul(row[68], nullptr, 10));
			e.career_tribute_points   = static_cast<uint32_t>(strtoul(row[69], nullptr, 10));
			e.tribute_points          = static_cast<uint32_t>(strtoul(row[70], nullptr, 10));
			e.tribute_active          = static_cast<uint32_t>(strtoul(row[71], nullptr, 10));
			e.pvp_status              = static_cast<uint8_t>(strtoul(row[72], nullptr, 10));
			e.pvp_kills               = static_cast<uint32_t>(strtoul(row[73], nullptr, 10));
			e.pvp_deaths              = static_cast<uint32_t>(strtoul(row[74], nullptr, 10));
			e.pvp_current_points      = static_cast<uint32_t>(strtoul(row[75], nullptr, 10));
			e.pvp_career_points       = static_cast<uint32_t>(strtoul(row[76], nullptr, 10));
			e.pvp_best_kill_streak    = static_cast<uint32_t>(strtoul(row[77], nullptr, 10));
			e.pvp_worst_death_streak  = static_cast<uint32_t>(strtoul(row[78], nullptr, 10));
			e.pvp_current_kill_streak = static_cast<uint32_t>(strtoul(row[79], nullptr, 10));
			e.pvp2                    = static_cast<uint32_t>(strtoul(row[80], nullptr, 10));
			e.pvp_type                = static_cast<uint32_t>(strtoul(row[81], nullptr, 10));
			e.show_helm               = static_cast<uint32_t>(strtoul(row[82], nullptr, 10));
			e.group_auto_consent      = static_cast<uint8_t>(strtoul(row[83], nullptr, 10));
			e.raid_auto_consent       = static_cast<uint8_t>(strtoul(row[84], nullptr, 10));
			e.guild_auto_consent      = static_cast<uint8_t>(strtoul(row[85], nullptr, 10));
			e.leadership_exp_on       = static_cast<uint8_t>(strtoul(row[86], nullptr, 10));
			e.RestTimer               = static_cast<uint32_t>(strtoul(row[87], nullptr, 10));
			e.air_remaining           = static_cast<uint32_t>(strtoul(row[88], nullptr, 10));
			e.autosplit_enabled       = static_cast<uint32_t>(strtoul(row[89], nullptr, 10));
			e.lfp                     = static_cast<uint8_t>(strtoul(row[90], nullptr, 10));
			e.lfg                     = static_cast<uint8_t>(strtoul(row[91], nullptr, 10));
			e.mailkey                 = row[92] ? row[92] : "";
			e.xtargets                = static_cast<uint8_t>(strtoul(row[93], nullptr, 10));
			e.firstlogon              = static_cast<int8_t>(atoi(row[94]));
			e.e_aa_effects            = static_cast<uint32_t>(strtoul(row[95], nullptr, 10));
			e.e_percent_to_aa         = static_cast<uint32_t>(strtoul(row[96], nullptr, 10));
			e.e_expended_aa_spent     = static_cast<uint32_t>(strtoul(row[97], nullptr, 10));
			e.aa_points_spent_old     = static_cast<uint32_t>(strtoul(row[98], nullptr, 10));
			e.aa_points_old           = static_cast<uint32_t>(strtoul(row[99], nullptr, 10));
			e.e_last_invsnapshot      = static_cast<uint32_t>(strtoul(row[100], nullptr, 10));
			e.deleted_at              = strtoll(row[101] ? row[101] : "-1", nullptr, 10);

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

#endif //EQEMU_BASE_CHARACTER_DATA_REPOSITORY_H
