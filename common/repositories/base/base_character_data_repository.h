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
		int         id;
		int         account_id;
		std::string name;
		std::string last_name;
		std::string title;
		std::string suffix;
		int         zone_id;
		int         zone_instance;
		float       y;
		float       x;
		float       z;
		float       heading;
		int         gender;
		int         race;
		int         class_;
		int         level;
		int         deity;
		int         birthday;
		int         last_login;
		int         time_played;
		int         level2;
		int         anon;
		int         gm;
		int         face;
		int         hair_color;
		int         hair_style;
		int         beard;
		int         beard_color;
		int         eye_color_1;
		int         eye_color_2;
		int         drakkin_heritage;
		int         drakkin_tattoo;
		int         drakkin_details;
		int         ability_time_seconds;
		int         ability_number;
		int         ability_time_minutes;
		int         ability_time_hours;
		int         exp;
		int         aa_points_spent;
		int         aa_exp;
		int         aa_points;
		int         group_leadership_exp;
		int         raid_leadership_exp;
		int         group_leadership_points;
		int         raid_leadership_points;
		int         points;
		int         cur_hp;
		int         mana;
		int         endurance;
		int         intoxication;
		int         str;
		int         sta;
		int         cha;
		int         dex;
		int         int_;
		int         agi;
		int         wis;
		int         zone_change_count;
		int         toxicity;
		int         hunger_level;
		int         thirst_level;
		int         ability_up;
		int         ldon_points_guk;
		int         ldon_points_mir;
		int         ldon_points_mmc;
		int         ldon_points_ruj;
		int         ldon_points_tak;
		int         ldon_points_available;
		int         tribute_time_remaining;
		int         career_tribute_points;
		int         tribute_points;
		int         tribute_active;
		int         pvp_status;
		int         pvp_kills;
		int         pvp_deaths;
		int         pvp_current_points;
		int         pvp_career_points;
		int         pvp_best_kill_streak;
		int         pvp_worst_death_streak;
		int         pvp_current_kill_streak;
		int         pvp2;
		int         pvp_type;
		int         show_helm;
		int         group_auto_consent;
		int         raid_auto_consent;
		int         guild_auto_consent;
		int         leadership_exp_on;
		int         RestTimer;
		int         air_remaining;
		int         autosplit_enabled;
		int         lfp;
		int         lfg;
		std::string mailkey;
		int         xtargets;
		int         firstlogon;
		int         e_aa_effects;
		int         e_percent_to_aa;
		int         e_expended_aa_spent;
		int         aa_points_spent_old;
		int         aa_points_old;
		int         e_last_invsnapshot;
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

			e.id                      = atoi(row[0]);
			e.account_id              = atoi(row[1]);
			e.name                    = row[2] ? row[2] : "";
			e.last_name               = row[3] ? row[3] : "";
			e.title                   = row[4] ? row[4] : "";
			e.suffix                  = row[5] ? row[5] : "";
			e.zone_id                 = atoi(row[6]);
			e.zone_instance           = atoi(row[7]);
			e.y                       = static_cast<float>(atof(row[8]));
			e.x                       = static_cast<float>(atof(row[9]));
			e.z                       = static_cast<float>(atof(row[10]));
			e.heading                 = static_cast<float>(atof(row[11]));
			e.gender                  = atoi(row[12]);
			e.race                    = atoi(row[13]);
			e.class_                  = atoi(row[14]);
			e.level                   = atoi(row[15]);
			e.deity                   = atoi(row[16]);
			e.birthday                = atoi(row[17]);
			e.last_login              = atoi(row[18]);
			e.time_played             = atoi(row[19]);
			e.level2                  = atoi(row[20]);
			e.anon                    = atoi(row[21]);
			e.gm                      = atoi(row[22]);
			e.face                    = atoi(row[23]);
			e.hair_color              = atoi(row[24]);
			e.hair_style              = atoi(row[25]);
			e.beard                   = atoi(row[26]);
			e.beard_color             = atoi(row[27]);
			e.eye_color_1             = atoi(row[28]);
			e.eye_color_2             = atoi(row[29]);
			e.drakkin_heritage        = atoi(row[30]);
			e.drakkin_tattoo          = atoi(row[31]);
			e.drakkin_details         = atoi(row[32]);
			e.ability_time_seconds    = atoi(row[33]);
			e.ability_number          = atoi(row[34]);
			e.ability_time_minutes    = atoi(row[35]);
			e.ability_time_hours      = atoi(row[36]);
			e.exp                     = atoi(row[37]);
			e.aa_points_spent         = atoi(row[38]);
			e.aa_exp                  = atoi(row[39]);
			e.aa_points               = atoi(row[40]);
			e.group_leadership_exp    = atoi(row[41]);
			e.raid_leadership_exp     = atoi(row[42]);
			e.group_leadership_points = atoi(row[43]);
			e.raid_leadership_points  = atoi(row[44]);
			e.points                  = atoi(row[45]);
			e.cur_hp                  = atoi(row[46]);
			e.mana                    = atoi(row[47]);
			e.endurance               = atoi(row[48]);
			e.intoxication            = atoi(row[49]);
			e.str                     = atoi(row[50]);
			e.sta                     = atoi(row[51]);
			e.cha                     = atoi(row[52]);
			e.dex                     = atoi(row[53]);
			e.int_                    = atoi(row[54]);
			e.agi                     = atoi(row[55]);
			e.wis                     = atoi(row[56]);
			e.zone_change_count       = atoi(row[57]);
			e.toxicity                = atoi(row[58]);
			e.hunger_level            = atoi(row[59]);
			e.thirst_level            = atoi(row[60]);
			e.ability_up              = atoi(row[61]);
			e.ldon_points_guk         = atoi(row[62]);
			e.ldon_points_mir         = atoi(row[63]);
			e.ldon_points_mmc         = atoi(row[64]);
			e.ldon_points_ruj         = atoi(row[65]);
			e.ldon_points_tak         = atoi(row[66]);
			e.ldon_points_available   = atoi(row[67]);
			e.tribute_time_remaining  = atoi(row[68]);
			e.career_tribute_points   = atoi(row[69]);
			e.tribute_points          = atoi(row[70]);
			e.tribute_active          = atoi(row[71]);
			e.pvp_status              = atoi(row[72]);
			e.pvp_kills               = atoi(row[73]);
			e.pvp_deaths              = atoi(row[74]);
			e.pvp_current_points      = atoi(row[75]);
			e.pvp_career_points       = atoi(row[76]);
			e.pvp_best_kill_streak    = atoi(row[77]);
			e.pvp_worst_death_streak  = atoi(row[78]);
			e.pvp_current_kill_streak = atoi(row[79]);
			e.pvp2                    = atoi(row[80]);
			e.pvp_type                = atoi(row[81]);
			e.show_helm               = atoi(row[82]);
			e.group_auto_consent      = atoi(row[83]);
			e.raid_auto_consent       = atoi(row[84]);
			e.guild_auto_consent      = atoi(row[85]);
			e.leadership_exp_on       = atoi(row[86]);
			e.RestTimer               = atoi(row[87]);
			e.air_remaining           = atoi(row[88]);
			e.autosplit_enabled       = atoi(row[89]);
			e.lfp                     = atoi(row[90]);
			e.lfg                     = atoi(row[91]);
			e.mailkey                 = row[92] ? row[92] : "";
			e.xtargets                = atoi(row[93]);
			e.firstlogon              = atoi(row[94]);
			e.e_aa_effects            = atoi(row[95]);
			e.e_percent_to_aa         = atoi(row[96]);
			e.e_expended_aa_spent     = atoi(row[97]);
			e.aa_points_spent_old     = atoi(row[98]);
			e.aa_points_old           = atoi(row[99]);
			e.e_last_invsnapshot      = atoi(row[100]);
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

			e.id                      = atoi(row[0]);
			e.account_id              = atoi(row[1]);
			e.name                    = row[2] ? row[2] : "";
			e.last_name               = row[3] ? row[3] : "";
			e.title                   = row[4] ? row[4] : "";
			e.suffix                  = row[5] ? row[5] : "";
			e.zone_id                 = atoi(row[6]);
			e.zone_instance           = atoi(row[7]);
			e.y                       = static_cast<float>(atof(row[8]));
			e.x                       = static_cast<float>(atof(row[9]));
			e.z                       = static_cast<float>(atof(row[10]));
			e.heading                 = static_cast<float>(atof(row[11]));
			e.gender                  = atoi(row[12]);
			e.race                    = atoi(row[13]);
			e.class_                  = atoi(row[14]);
			e.level                   = atoi(row[15]);
			e.deity                   = atoi(row[16]);
			e.birthday                = atoi(row[17]);
			e.last_login              = atoi(row[18]);
			e.time_played             = atoi(row[19]);
			e.level2                  = atoi(row[20]);
			e.anon                    = atoi(row[21]);
			e.gm                      = atoi(row[22]);
			e.face                    = atoi(row[23]);
			e.hair_color              = atoi(row[24]);
			e.hair_style              = atoi(row[25]);
			e.beard                   = atoi(row[26]);
			e.beard_color             = atoi(row[27]);
			e.eye_color_1             = atoi(row[28]);
			e.eye_color_2             = atoi(row[29]);
			e.drakkin_heritage        = atoi(row[30]);
			e.drakkin_tattoo          = atoi(row[31]);
			e.drakkin_details         = atoi(row[32]);
			e.ability_time_seconds    = atoi(row[33]);
			e.ability_number          = atoi(row[34]);
			e.ability_time_minutes    = atoi(row[35]);
			e.ability_time_hours      = atoi(row[36]);
			e.exp                     = atoi(row[37]);
			e.aa_points_spent         = atoi(row[38]);
			e.aa_exp                  = atoi(row[39]);
			e.aa_points               = atoi(row[40]);
			e.group_leadership_exp    = atoi(row[41]);
			e.raid_leadership_exp     = atoi(row[42]);
			e.group_leadership_points = atoi(row[43]);
			e.raid_leadership_points  = atoi(row[44]);
			e.points                  = atoi(row[45]);
			e.cur_hp                  = atoi(row[46]);
			e.mana                    = atoi(row[47]);
			e.endurance               = atoi(row[48]);
			e.intoxication            = atoi(row[49]);
			e.str                     = atoi(row[50]);
			e.sta                     = atoi(row[51]);
			e.cha                     = atoi(row[52]);
			e.dex                     = atoi(row[53]);
			e.int_                    = atoi(row[54]);
			e.agi                     = atoi(row[55]);
			e.wis                     = atoi(row[56]);
			e.zone_change_count       = atoi(row[57]);
			e.toxicity                = atoi(row[58]);
			e.hunger_level            = atoi(row[59]);
			e.thirst_level            = atoi(row[60]);
			e.ability_up              = atoi(row[61]);
			e.ldon_points_guk         = atoi(row[62]);
			e.ldon_points_mir         = atoi(row[63]);
			e.ldon_points_mmc         = atoi(row[64]);
			e.ldon_points_ruj         = atoi(row[65]);
			e.ldon_points_tak         = atoi(row[66]);
			e.ldon_points_available   = atoi(row[67]);
			e.tribute_time_remaining  = atoi(row[68]);
			e.career_tribute_points   = atoi(row[69]);
			e.tribute_points          = atoi(row[70]);
			e.tribute_active          = atoi(row[71]);
			e.pvp_status              = atoi(row[72]);
			e.pvp_kills               = atoi(row[73]);
			e.pvp_deaths              = atoi(row[74]);
			e.pvp_current_points      = atoi(row[75]);
			e.pvp_career_points       = atoi(row[76]);
			e.pvp_best_kill_streak    = atoi(row[77]);
			e.pvp_worst_death_streak  = atoi(row[78]);
			e.pvp_current_kill_streak = atoi(row[79]);
			e.pvp2                    = atoi(row[80]);
			e.pvp_type                = atoi(row[81]);
			e.show_helm               = atoi(row[82]);
			e.group_auto_consent      = atoi(row[83]);
			e.raid_auto_consent       = atoi(row[84]);
			e.guild_auto_consent      = atoi(row[85]);
			e.leadership_exp_on       = atoi(row[86]);
			e.RestTimer               = atoi(row[87]);
			e.air_remaining           = atoi(row[88]);
			e.autosplit_enabled       = atoi(row[89]);
			e.lfp                     = atoi(row[90]);
			e.lfg                     = atoi(row[91]);
			e.mailkey                 = row[92] ? row[92] : "";
			e.xtargets                = atoi(row[93]);
			e.firstlogon              = atoi(row[94]);
			e.e_aa_effects            = atoi(row[95]);
			e.e_percent_to_aa         = atoi(row[96]);
			e.e_expended_aa_spent     = atoi(row[97]);
			e.aa_points_spent_old     = atoi(row[98]);
			e.aa_points_old           = atoi(row[99]);
			e.e_last_invsnapshot      = atoi(row[100]);
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

			e.id                      = atoi(row[0]);
			e.account_id              = atoi(row[1]);
			e.name                    = row[2] ? row[2] : "";
			e.last_name               = row[3] ? row[3] : "";
			e.title                   = row[4] ? row[4] : "";
			e.suffix                  = row[5] ? row[5] : "";
			e.zone_id                 = atoi(row[6]);
			e.zone_instance           = atoi(row[7]);
			e.y                       = static_cast<float>(atof(row[8]));
			e.x                       = static_cast<float>(atof(row[9]));
			e.z                       = static_cast<float>(atof(row[10]));
			e.heading                 = static_cast<float>(atof(row[11]));
			e.gender                  = atoi(row[12]);
			e.race                    = atoi(row[13]);
			e.class_                  = atoi(row[14]);
			e.level                   = atoi(row[15]);
			e.deity                   = atoi(row[16]);
			e.birthday                = atoi(row[17]);
			e.last_login              = atoi(row[18]);
			e.time_played             = atoi(row[19]);
			e.level2                  = atoi(row[20]);
			e.anon                    = atoi(row[21]);
			e.gm                      = atoi(row[22]);
			e.face                    = atoi(row[23]);
			e.hair_color              = atoi(row[24]);
			e.hair_style              = atoi(row[25]);
			e.beard                   = atoi(row[26]);
			e.beard_color             = atoi(row[27]);
			e.eye_color_1             = atoi(row[28]);
			e.eye_color_2             = atoi(row[29]);
			e.drakkin_heritage        = atoi(row[30]);
			e.drakkin_tattoo          = atoi(row[31]);
			e.drakkin_details         = atoi(row[32]);
			e.ability_time_seconds    = atoi(row[33]);
			e.ability_number          = atoi(row[34]);
			e.ability_time_minutes    = atoi(row[35]);
			e.ability_time_hours      = atoi(row[36]);
			e.exp                     = atoi(row[37]);
			e.aa_points_spent         = atoi(row[38]);
			e.aa_exp                  = atoi(row[39]);
			e.aa_points               = atoi(row[40]);
			e.group_leadership_exp    = atoi(row[41]);
			e.raid_leadership_exp     = atoi(row[42]);
			e.group_leadership_points = atoi(row[43]);
			e.raid_leadership_points  = atoi(row[44]);
			e.points                  = atoi(row[45]);
			e.cur_hp                  = atoi(row[46]);
			e.mana                    = atoi(row[47]);
			e.endurance               = atoi(row[48]);
			e.intoxication            = atoi(row[49]);
			e.str                     = atoi(row[50]);
			e.sta                     = atoi(row[51]);
			e.cha                     = atoi(row[52]);
			e.dex                     = atoi(row[53]);
			e.int_                    = atoi(row[54]);
			e.agi                     = atoi(row[55]);
			e.wis                     = atoi(row[56]);
			e.zone_change_count       = atoi(row[57]);
			e.toxicity                = atoi(row[58]);
			e.hunger_level            = atoi(row[59]);
			e.thirst_level            = atoi(row[60]);
			e.ability_up              = atoi(row[61]);
			e.ldon_points_guk         = atoi(row[62]);
			e.ldon_points_mir         = atoi(row[63]);
			e.ldon_points_mmc         = atoi(row[64]);
			e.ldon_points_ruj         = atoi(row[65]);
			e.ldon_points_tak         = atoi(row[66]);
			e.ldon_points_available   = atoi(row[67]);
			e.tribute_time_remaining  = atoi(row[68]);
			e.career_tribute_points   = atoi(row[69]);
			e.tribute_points          = atoi(row[70]);
			e.tribute_active          = atoi(row[71]);
			e.pvp_status              = atoi(row[72]);
			e.pvp_kills               = atoi(row[73]);
			e.pvp_deaths              = atoi(row[74]);
			e.pvp_current_points      = atoi(row[75]);
			e.pvp_career_points       = atoi(row[76]);
			e.pvp_best_kill_streak    = atoi(row[77]);
			e.pvp_worst_death_streak  = atoi(row[78]);
			e.pvp_current_kill_streak = atoi(row[79]);
			e.pvp2                    = atoi(row[80]);
			e.pvp_type                = atoi(row[81]);
			e.show_helm               = atoi(row[82]);
			e.group_auto_consent      = atoi(row[83]);
			e.raid_auto_consent       = atoi(row[84]);
			e.guild_auto_consent      = atoi(row[85]);
			e.leadership_exp_on       = atoi(row[86]);
			e.RestTimer               = atoi(row[87]);
			e.air_remaining           = atoi(row[88]);
			e.autosplit_enabled       = atoi(row[89]);
			e.lfp                     = atoi(row[90]);
			e.lfg                     = atoi(row[91]);
			e.mailkey                 = row[92] ? row[92] : "";
			e.xtargets                = atoi(row[93]);
			e.firstlogon              = atoi(row[94]);
			e.e_aa_effects            = atoi(row[95]);
			e.e_percent_to_aa         = atoi(row[96]);
			e.e_expended_aa_spent     = atoi(row[97]);
			e.aa_points_spent_old     = atoi(row[98]);
			e.aa_points_old           = atoi(row[99]);
			e.e_last_invsnapshot      = atoi(row[100]);
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
