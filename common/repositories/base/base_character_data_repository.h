/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *
 */

/**
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to
 * the repository extending the base. Any modifications to base repositories are to
 * be made by the generator only
 */

#ifndef EQEMU_BASE_CHARACTER_DATA_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_DATA_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

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
		int         class;
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
		int         int;
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
		std::string deleted_at;
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
			"class",
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
			"int",
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

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string InsertColumnsRaw()
	{
		std::vector<std::string> insert_columns;

		for (auto &column : Columns()) {
			if (column == PrimaryKey()) {
				continue;
			}

			insert_columns.push_back(column);
		}

		return std::string(implode(", ", insert_columns));
	}

	static std::string TableName()
	{
		return std::string("character_data");
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
			InsertColumnsRaw()
		);
	}

	static CharacterData NewEntity()
	{
		CharacterData entry{};

		entry.id                      = 0;
		entry.account_id              = 0;
		entry.name                    = "";
		entry.last_name               = "";
		entry.title                   = "";
		entry.suffix                  = "";
		entry.zone_id                 = 0;
		entry.zone_instance           = 0;
		entry.y                       = 0;
		entry.x                       = 0;
		entry.z                       = 0;
		entry.heading                 = 0;
		entry.gender                  = 0;
		entry.race                    = 0;
		entry.class                   = 0;
		entry.level                   = 0;
		entry.deity                   = 0;
		entry.birthday                = 0;
		entry.last_login              = 0;
		entry.time_played             = 0;
		entry.level2                  = 0;
		entry.anon                    = 0;
		entry.gm                      = 0;
		entry.face                    = 0;
		entry.hair_color              = 0;
		entry.hair_style              = 0;
		entry.beard                   = 0;
		entry.beard_color             = 0;
		entry.eye_color_1             = 0;
		entry.eye_color_2             = 0;
		entry.drakkin_heritage        = 0;
		entry.drakkin_tattoo          = 0;
		entry.drakkin_details         = 0;
		entry.ability_time_seconds    = 0;
		entry.ability_number          = 0;
		entry.ability_time_minutes    = 0;
		entry.ability_time_hours      = 0;
		entry.exp                     = 0;
		entry.aa_points_spent         = 0;
		entry.aa_exp                  = 0;
		entry.aa_points               = 0;
		entry.group_leadership_exp    = 0;
		entry.raid_leadership_exp     = 0;
		entry.group_leadership_points = 0;
		entry.raid_leadership_points  = 0;
		entry.points                  = 0;
		entry.cur_hp                  = 0;
		entry.mana                    = 0;
		entry.endurance               = 0;
		entry.intoxication            = 0;
		entry.str                     = 0;
		entry.sta                     = 0;
		entry.cha                     = 0;
		entry.dex                     = 0;
		entry.int                     = 0;
		entry.agi                     = 0;
		entry.wis                     = 0;
		entry.zone_change_count       = 0;
		entry.toxicity                = 0;
		entry.hunger_level            = 0;
		entry.thirst_level            = 0;
		entry.ability_up              = 0;
		entry.ldon_points_guk         = 0;
		entry.ldon_points_mir         = 0;
		entry.ldon_points_mmc         = 0;
		entry.ldon_points_ruj         = 0;
		entry.ldon_points_tak         = 0;
		entry.ldon_points_available   = 0;
		entry.tribute_time_remaining  = 0;
		entry.career_tribute_points   = 0;
		entry.tribute_points          = 0;
		entry.tribute_active          = 0;
		entry.pvp_status              = 0;
		entry.pvp_kills               = 0;
		entry.pvp_deaths              = 0;
		entry.pvp_current_points      = 0;
		entry.pvp_career_points       = 0;
		entry.pvp_best_kill_streak    = 0;
		entry.pvp_worst_death_streak  = 0;
		entry.pvp_current_kill_streak = 0;
		entry.pvp2                    = 0;
		entry.pvp_type                = 0;
		entry.show_helm               = 0;
		entry.group_auto_consent      = 0;
		entry.raid_auto_consent       = 0;
		entry.guild_auto_consent      = 0;
		entry.leadership_exp_on       = 0;
		entry.RestTimer               = 0;
		entry.air_remaining           = 0;
		entry.autosplit_enabled       = 0;
		entry.lfp                     = 0;
		entry.lfg                     = 0;
		entry.mailkey                 = "";
		entry.xtargets                = 5;
		entry.firstlogon              = 0;
		entry.e_aa_effects            = 0;
		entry.e_percent_to_aa         = 0;
		entry.e_expended_aa_spent     = 0;
		entry.aa_points_spent_old     = 0;
		entry.aa_points_old           = 0;
		entry.e_last_invsnapshot      = 0;
		entry.deleted_at              = 0;

		return entry;
	}

	static CharacterData GetCharacterDataEntry(
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
		int character_data_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_data_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterData entry{};

			entry.id                      = atoi(row[0]);
			entry.account_id              = atoi(row[1]);
			entry.name                    = row[2] ? row[2] : "";
			entry.last_name               = row[3] ? row[3] : "";
			entry.title                   = row[4] ? row[4] : "";
			entry.suffix                  = row[5] ? row[5] : "";
			entry.zone_id                 = atoi(row[6]);
			entry.zone_instance           = atoi(row[7]);
			entry.y                       = static_cast<float>(atof(row[8]));
			entry.x                       = static_cast<float>(atof(row[9]));
			entry.z                       = static_cast<float>(atof(row[10]));
			entry.heading                 = static_cast<float>(atof(row[11]));
			entry.gender                  = atoi(row[12]);
			entry.race                    = atoi(row[13]);
			entry.class                   = atoi(row[14]);
			entry.level                   = atoi(row[15]);
			entry.deity                   = atoi(row[16]);
			entry.birthday                = atoi(row[17]);
			entry.last_login              = atoi(row[18]);
			entry.time_played             = atoi(row[19]);
			entry.level2                  = atoi(row[20]);
			entry.anon                    = atoi(row[21]);
			entry.gm                      = atoi(row[22]);
			entry.face                    = atoi(row[23]);
			entry.hair_color              = atoi(row[24]);
			entry.hair_style              = atoi(row[25]);
			entry.beard                   = atoi(row[26]);
			entry.beard_color             = atoi(row[27]);
			entry.eye_color_1             = atoi(row[28]);
			entry.eye_color_2             = atoi(row[29]);
			entry.drakkin_heritage        = atoi(row[30]);
			entry.drakkin_tattoo          = atoi(row[31]);
			entry.drakkin_details         = atoi(row[32]);
			entry.ability_time_seconds    = atoi(row[33]);
			entry.ability_number          = atoi(row[34]);
			entry.ability_time_minutes    = atoi(row[35]);
			entry.ability_time_hours      = atoi(row[36]);
			entry.exp                     = atoi(row[37]);
			entry.aa_points_spent         = atoi(row[38]);
			entry.aa_exp                  = atoi(row[39]);
			entry.aa_points               = atoi(row[40]);
			entry.group_leadership_exp    = atoi(row[41]);
			entry.raid_leadership_exp     = atoi(row[42]);
			entry.group_leadership_points = atoi(row[43]);
			entry.raid_leadership_points  = atoi(row[44]);
			entry.points                  = atoi(row[45]);
			entry.cur_hp                  = atoi(row[46]);
			entry.mana                    = atoi(row[47]);
			entry.endurance               = atoi(row[48]);
			entry.intoxication            = atoi(row[49]);
			entry.str                     = atoi(row[50]);
			entry.sta                     = atoi(row[51]);
			entry.cha                     = atoi(row[52]);
			entry.dex                     = atoi(row[53]);
			entry.int                     = atoi(row[54]);
			entry.agi                     = atoi(row[55]);
			entry.wis                     = atoi(row[56]);
			entry.zone_change_count       = atoi(row[57]);
			entry.toxicity                = atoi(row[58]);
			entry.hunger_level            = atoi(row[59]);
			entry.thirst_level            = atoi(row[60]);
			entry.ability_up              = atoi(row[61]);
			entry.ldon_points_guk         = atoi(row[62]);
			entry.ldon_points_mir         = atoi(row[63]);
			entry.ldon_points_mmc         = atoi(row[64]);
			entry.ldon_points_ruj         = atoi(row[65]);
			entry.ldon_points_tak         = atoi(row[66]);
			entry.ldon_points_available   = atoi(row[67]);
			entry.tribute_time_remaining  = atoi(row[68]);
			entry.career_tribute_points   = atoi(row[69]);
			entry.tribute_points          = atoi(row[70]);
			entry.tribute_active          = atoi(row[71]);
			entry.pvp_status              = atoi(row[72]);
			entry.pvp_kills               = atoi(row[73]);
			entry.pvp_deaths              = atoi(row[74]);
			entry.pvp_current_points      = atoi(row[75]);
			entry.pvp_career_points       = atoi(row[76]);
			entry.pvp_best_kill_streak    = atoi(row[77]);
			entry.pvp_worst_death_streak  = atoi(row[78]);
			entry.pvp_current_kill_streak = atoi(row[79]);
			entry.pvp2                    = atoi(row[80]);
			entry.pvp_type                = atoi(row[81]);
			entry.show_helm               = atoi(row[82]);
			entry.group_auto_consent      = atoi(row[83]);
			entry.raid_auto_consent       = atoi(row[84]);
			entry.guild_auto_consent      = atoi(row[85]);
			entry.leadership_exp_on       = atoi(row[86]);
			entry.RestTimer               = atoi(row[87]);
			entry.air_remaining           = atoi(row[88]);
			entry.autosplit_enabled       = atoi(row[89]);
			entry.lfp                     = atoi(row[90]);
			entry.lfg                     = atoi(row[91]);
			entry.mailkey                 = row[92] ? row[92] : "";
			entry.xtargets                = atoi(row[93]);
			entry.firstlogon              = atoi(row[94]);
			entry.e_aa_effects            = atoi(row[95]);
			entry.e_percent_to_aa         = atoi(row[96]);
			entry.e_expended_aa_spent     = atoi(row[97]);
			entry.aa_points_spent_old     = atoi(row[98]);
			entry.aa_points_old           = atoi(row[99]);
			entry.e_last_invsnapshot      = atoi(row[100]);
			entry.deleted_at              = row[101] ? row[101] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int character_data_id
	)
	{
		auto results = database.QueryDatabase(
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
		CharacterData character_data_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(character_data_entry.account_id));
		update_values.push_back(columns[2] + " = '" + EscapeString(character_data_entry.name) + "'");
		update_values.push_back(columns[3] + " = '" + EscapeString(character_data_entry.last_name) + "'");
		update_values.push_back(columns[4] + " = '" + EscapeString(character_data_entry.title) + "'");
		update_values.push_back(columns[5] + " = '" + EscapeString(character_data_entry.suffix) + "'");
		update_values.push_back(columns[6] + " = " + std::to_string(character_data_entry.zone_id));
		update_values.push_back(columns[7] + " = " + std::to_string(character_data_entry.zone_instance));
		update_values.push_back(columns[8] + " = " + std::to_string(character_data_entry.y));
		update_values.push_back(columns[9] + " = " + std::to_string(character_data_entry.x));
		update_values.push_back(columns[10] + " = " + std::to_string(character_data_entry.z));
		update_values.push_back(columns[11] + " = " + std::to_string(character_data_entry.heading));
		update_values.push_back(columns[12] + " = " + std::to_string(character_data_entry.gender));
		update_values.push_back(columns[13] + " = " + std::to_string(character_data_entry.race));
		update_values.push_back(columns[14] + " = " + std::to_string(character_data_entry.class));
		update_values.push_back(columns[15] + " = " + std::to_string(character_data_entry.level));
		update_values.push_back(columns[16] + " = " + std::to_string(character_data_entry.deity));
		update_values.push_back(columns[17] + " = " + std::to_string(character_data_entry.birthday));
		update_values.push_back(columns[18] + " = " + std::to_string(character_data_entry.last_login));
		update_values.push_back(columns[19] + " = " + std::to_string(character_data_entry.time_played));
		update_values.push_back(columns[20] + " = " + std::to_string(character_data_entry.level2));
		update_values.push_back(columns[21] + " = " + std::to_string(character_data_entry.anon));
		update_values.push_back(columns[22] + " = " + std::to_string(character_data_entry.gm));
		update_values.push_back(columns[23] + " = " + std::to_string(character_data_entry.face));
		update_values.push_back(columns[24] + " = " + std::to_string(character_data_entry.hair_color));
		update_values.push_back(columns[25] + " = " + std::to_string(character_data_entry.hair_style));
		update_values.push_back(columns[26] + " = " + std::to_string(character_data_entry.beard));
		update_values.push_back(columns[27] + " = " + std::to_string(character_data_entry.beard_color));
		update_values.push_back(columns[28] + " = " + std::to_string(character_data_entry.eye_color_1));
		update_values.push_back(columns[29] + " = " + std::to_string(character_data_entry.eye_color_2));
		update_values.push_back(columns[30] + " = " + std::to_string(character_data_entry.drakkin_heritage));
		update_values.push_back(columns[31] + " = " + std::to_string(character_data_entry.drakkin_tattoo));
		update_values.push_back(columns[32] + " = " + std::to_string(character_data_entry.drakkin_details));
		update_values.push_back(columns[33] + " = " + std::to_string(character_data_entry.ability_time_seconds));
		update_values.push_back(columns[34] + " = " + std::to_string(character_data_entry.ability_number));
		update_values.push_back(columns[35] + " = " + std::to_string(character_data_entry.ability_time_minutes));
		update_values.push_back(columns[36] + " = " + std::to_string(character_data_entry.ability_time_hours));
		update_values.push_back(columns[37] + " = " + std::to_string(character_data_entry.exp));
		update_values.push_back(columns[38] + " = " + std::to_string(character_data_entry.aa_points_spent));
		update_values.push_back(columns[39] + " = " + std::to_string(character_data_entry.aa_exp));
		update_values.push_back(columns[40] + " = " + std::to_string(character_data_entry.aa_points));
		update_values.push_back(columns[41] + " = " + std::to_string(character_data_entry.group_leadership_exp));
		update_values.push_back(columns[42] + " = " + std::to_string(character_data_entry.raid_leadership_exp));
		update_values.push_back(columns[43] + " = " + std::to_string(character_data_entry.group_leadership_points));
		update_values.push_back(columns[44] + " = " + std::to_string(character_data_entry.raid_leadership_points));
		update_values.push_back(columns[45] + " = " + std::to_string(character_data_entry.points));
		update_values.push_back(columns[46] + " = " + std::to_string(character_data_entry.cur_hp));
		update_values.push_back(columns[47] + " = " + std::to_string(character_data_entry.mana));
		update_values.push_back(columns[48] + " = " + std::to_string(character_data_entry.endurance));
		update_values.push_back(columns[49] + " = " + std::to_string(character_data_entry.intoxication));
		update_values.push_back(columns[50] + " = " + std::to_string(character_data_entry.str));
		update_values.push_back(columns[51] + " = " + std::to_string(character_data_entry.sta));
		update_values.push_back(columns[52] + " = " + std::to_string(character_data_entry.cha));
		update_values.push_back(columns[53] + " = " + std::to_string(character_data_entry.dex));
		update_values.push_back(columns[54] + " = " + std::to_string(character_data_entry.int));
		update_values.push_back(columns[55] + " = " + std::to_string(character_data_entry.agi));
		update_values.push_back(columns[56] + " = " + std::to_string(character_data_entry.wis));
		update_values.push_back(columns[57] + " = " + std::to_string(character_data_entry.zone_change_count));
		update_values.push_back(columns[58] + " = " + std::to_string(character_data_entry.toxicity));
		update_values.push_back(columns[59] + " = " + std::to_string(character_data_entry.hunger_level));
		update_values.push_back(columns[60] + " = " + std::to_string(character_data_entry.thirst_level));
		update_values.push_back(columns[61] + " = " + std::to_string(character_data_entry.ability_up));
		update_values.push_back(columns[62] + " = " + std::to_string(character_data_entry.ldon_points_guk));
		update_values.push_back(columns[63] + " = " + std::to_string(character_data_entry.ldon_points_mir));
		update_values.push_back(columns[64] + " = " + std::to_string(character_data_entry.ldon_points_mmc));
		update_values.push_back(columns[65] + " = " + std::to_string(character_data_entry.ldon_points_ruj));
		update_values.push_back(columns[66] + " = " + std::to_string(character_data_entry.ldon_points_tak));
		update_values.push_back(columns[67] + " = " + std::to_string(character_data_entry.ldon_points_available));
		update_values.push_back(columns[68] + " = " + std::to_string(character_data_entry.tribute_time_remaining));
		update_values.push_back(columns[69] + " = " + std::to_string(character_data_entry.career_tribute_points));
		update_values.push_back(columns[70] + " = " + std::to_string(character_data_entry.tribute_points));
		update_values.push_back(columns[71] + " = " + std::to_string(character_data_entry.tribute_active));
		update_values.push_back(columns[72] + " = " + std::to_string(character_data_entry.pvp_status));
		update_values.push_back(columns[73] + " = " + std::to_string(character_data_entry.pvp_kills));
		update_values.push_back(columns[74] + " = " + std::to_string(character_data_entry.pvp_deaths));
		update_values.push_back(columns[75] + " = " + std::to_string(character_data_entry.pvp_current_points));
		update_values.push_back(columns[76] + " = " + std::to_string(character_data_entry.pvp_career_points));
		update_values.push_back(columns[77] + " = " + std::to_string(character_data_entry.pvp_best_kill_streak));
		update_values.push_back(columns[78] + " = " + std::to_string(character_data_entry.pvp_worst_death_streak));
		update_values.push_back(columns[79] + " = " + std::to_string(character_data_entry.pvp_current_kill_streak));
		update_values.push_back(columns[80] + " = " + std::to_string(character_data_entry.pvp2));
		update_values.push_back(columns[81] + " = " + std::to_string(character_data_entry.pvp_type));
		update_values.push_back(columns[82] + " = " + std::to_string(character_data_entry.show_helm));
		update_values.push_back(columns[83] + " = " + std::to_string(character_data_entry.group_auto_consent));
		update_values.push_back(columns[84] + " = " + std::to_string(character_data_entry.raid_auto_consent));
		update_values.push_back(columns[85] + " = " + std::to_string(character_data_entry.guild_auto_consent));
		update_values.push_back(columns[86] + " = " + std::to_string(character_data_entry.leadership_exp_on));
		update_values.push_back(columns[87] + " = " + std::to_string(character_data_entry.RestTimer));
		update_values.push_back(columns[88] + " = " + std::to_string(character_data_entry.air_remaining));
		update_values.push_back(columns[89] + " = " + std::to_string(character_data_entry.autosplit_enabled));
		update_values.push_back(columns[90] + " = " + std::to_string(character_data_entry.lfp));
		update_values.push_back(columns[91] + " = " + std::to_string(character_data_entry.lfg));
		update_values.push_back(columns[92] + " = '" + EscapeString(character_data_entry.mailkey) + "'");
		update_values.push_back(columns[93] + " = " + std::to_string(character_data_entry.xtargets));
		update_values.push_back(columns[94] + " = " + std::to_string(character_data_entry.firstlogon));
		update_values.push_back(columns[95] + " = " + std::to_string(character_data_entry.e_aa_effects));
		update_values.push_back(columns[96] + " = " + std::to_string(character_data_entry.e_percent_to_aa));
		update_values.push_back(columns[97] + " = " + std::to_string(character_data_entry.e_expended_aa_spent));
		update_values.push_back(columns[98] + " = " + std::to_string(character_data_entry.aa_points_spent_old));
		update_values.push_back(columns[99] + " = " + std::to_string(character_data_entry.aa_points_old));
		update_values.push_back(columns[100] + " = " + std::to_string(character_data_entry.e_last_invsnapshot));
		update_values.push_back(columns[101] + " = '" + EscapeString(character_data_entry.deleted_at) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				character_data_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterData InsertOne(
		CharacterData character_data_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_data_entry.account_id));
		insert_values.push_back("'" + EscapeString(character_data_entry.name) + "'");
		insert_values.push_back("'" + EscapeString(character_data_entry.last_name) + "'");
		insert_values.push_back("'" + EscapeString(character_data_entry.title) + "'");
		insert_values.push_back("'" + EscapeString(character_data_entry.suffix) + "'");
		insert_values.push_back(std::to_string(character_data_entry.zone_id));
		insert_values.push_back(std::to_string(character_data_entry.zone_instance));
		insert_values.push_back(std::to_string(character_data_entry.y));
		insert_values.push_back(std::to_string(character_data_entry.x));
		insert_values.push_back(std::to_string(character_data_entry.z));
		insert_values.push_back(std::to_string(character_data_entry.heading));
		insert_values.push_back(std::to_string(character_data_entry.gender));
		insert_values.push_back(std::to_string(character_data_entry.race));
		insert_values.push_back(std::to_string(character_data_entry.class));
		insert_values.push_back(std::to_string(character_data_entry.level));
		insert_values.push_back(std::to_string(character_data_entry.deity));
		insert_values.push_back(std::to_string(character_data_entry.birthday));
		insert_values.push_back(std::to_string(character_data_entry.last_login));
		insert_values.push_back(std::to_string(character_data_entry.time_played));
		insert_values.push_back(std::to_string(character_data_entry.level2));
		insert_values.push_back(std::to_string(character_data_entry.anon));
		insert_values.push_back(std::to_string(character_data_entry.gm));
		insert_values.push_back(std::to_string(character_data_entry.face));
		insert_values.push_back(std::to_string(character_data_entry.hair_color));
		insert_values.push_back(std::to_string(character_data_entry.hair_style));
		insert_values.push_back(std::to_string(character_data_entry.beard));
		insert_values.push_back(std::to_string(character_data_entry.beard_color));
		insert_values.push_back(std::to_string(character_data_entry.eye_color_1));
		insert_values.push_back(std::to_string(character_data_entry.eye_color_2));
		insert_values.push_back(std::to_string(character_data_entry.drakkin_heritage));
		insert_values.push_back(std::to_string(character_data_entry.drakkin_tattoo));
		insert_values.push_back(std::to_string(character_data_entry.drakkin_details));
		insert_values.push_back(std::to_string(character_data_entry.ability_time_seconds));
		insert_values.push_back(std::to_string(character_data_entry.ability_number));
		insert_values.push_back(std::to_string(character_data_entry.ability_time_minutes));
		insert_values.push_back(std::to_string(character_data_entry.ability_time_hours));
		insert_values.push_back(std::to_string(character_data_entry.exp));
		insert_values.push_back(std::to_string(character_data_entry.aa_points_spent));
		insert_values.push_back(std::to_string(character_data_entry.aa_exp));
		insert_values.push_back(std::to_string(character_data_entry.aa_points));
		insert_values.push_back(std::to_string(character_data_entry.group_leadership_exp));
		insert_values.push_back(std::to_string(character_data_entry.raid_leadership_exp));
		insert_values.push_back(std::to_string(character_data_entry.group_leadership_points));
		insert_values.push_back(std::to_string(character_data_entry.raid_leadership_points));
		insert_values.push_back(std::to_string(character_data_entry.points));
		insert_values.push_back(std::to_string(character_data_entry.cur_hp));
		insert_values.push_back(std::to_string(character_data_entry.mana));
		insert_values.push_back(std::to_string(character_data_entry.endurance));
		insert_values.push_back(std::to_string(character_data_entry.intoxication));
		insert_values.push_back(std::to_string(character_data_entry.str));
		insert_values.push_back(std::to_string(character_data_entry.sta));
		insert_values.push_back(std::to_string(character_data_entry.cha));
		insert_values.push_back(std::to_string(character_data_entry.dex));
		insert_values.push_back(std::to_string(character_data_entry.int));
		insert_values.push_back(std::to_string(character_data_entry.agi));
		insert_values.push_back(std::to_string(character_data_entry.wis));
		insert_values.push_back(std::to_string(character_data_entry.zone_change_count));
		insert_values.push_back(std::to_string(character_data_entry.toxicity));
		insert_values.push_back(std::to_string(character_data_entry.hunger_level));
		insert_values.push_back(std::to_string(character_data_entry.thirst_level));
		insert_values.push_back(std::to_string(character_data_entry.ability_up));
		insert_values.push_back(std::to_string(character_data_entry.ldon_points_guk));
		insert_values.push_back(std::to_string(character_data_entry.ldon_points_mir));
		insert_values.push_back(std::to_string(character_data_entry.ldon_points_mmc));
		insert_values.push_back(std::to_string(character_data_entry.ldon_points_ruj));
		insert_values.push_back(std::to_string(character_data_entry.ldon_points_tak));
		insert_values.push_back(std::to_string(character_data_entry.ldon_points_available));
		insert_values.push_back(std::to_string(character_data_entry.tribute_time_remaining));
		insert_values.push_back(std::to_string(character_data_entry.career_tribute_points));
		insert_values.push_back(std::to_string(character_data_entry.tribute_points));
		insert_values.push_back(std::to_string(character_data_entry.tribute_active));
		insert_values.push_back(std::to_string(character_data_entry.pvp_status));
		insert_values.push_back(std::to_string(character_data_entry.pvp_kills));
		insert_values.push_back(std::to_string(character_data_entry.pvp_deaths));
		insert_values.push_back(std::to_string(character_data_entry.pvp_current_points));
		insert_values.push_back(std::to_string(character_data_entry.pvp_career_points));
		insert_values.push_back(std::to_string(character_data_entry.pvp_best_kill_streak));
		insert_values.push_back(std::to_string(character_data_entry.pvp_worst_death_streak));
		insert_values.push_back(std::to_string(character_data_entry.pvp_current_kill_streak));
		insert_values.push_back(std::to_string(character_data_entry.pvp2));
		insert_values.push_back(std::to_string(character_data_entry.pvp_type));
		insert_values.push_back(std::to_string(character_data_entry.show_helm));
		insert_values.push_back(std::to_string(character_data_entry.group_auto_consent));
		insert_values.push_back(std::to_string(character_data_entry.raid_auto_consent));
		insert_values.push_back(std::to_string(character_data_entry.guild_auto_consent));
		insert_values.push_back(std::to_string(character_data_entry.leadership_exp_on));
		insert_values.push_back(std::to_string(character_data_entry.RestTimer));
		insert_values.push_back(std::to_string(character_data_entry.air_remaining));
		insert_values.push_back(std::to_string(character_data_entry.autosplit_enabled));
		insert_values.push_back(std::to_string(character_data_entry.lfp));
		insert_values.push_back(std::to_string(character_data_entry.lfg));
		insert_values.push_back("'" + EscapeString(character_data_entry.mailkey) + "'");
		insert_values.push_back(std::to_string(character_data_entry.xtargets));
		insert_values.push_back(std::to_string(character_data_entry.firstlogon));
		insert_values.push_back(std::to_string(character_data_entry.e_aa_effects));
		insert_values.push_back(std::to_string(character_data_entry.e_percent_to_aa));
		insert_values.push_back(std::to_string(character_data_entry.e_expended_aa_spent));
		insert_values.push_back(std::to_string(character_data_entry.aa_points_spent_old));
		insert_values.push_back(std::to_string(character_data_entry.aa_points_old));
		insert_values.push_back(std::to_string(character_data_entry.e_last_invsnapshot));
		insert_values.push_back("'" + EscapeString(character_data_entry.deleted_at) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			character_data_entry.id = results.LastInsertedID();
			return character_data_entry;
		}

		character_data_entry = NewEntity();

		return character_data_entry;
	}

	static int InsertMany(
		std::vector<CharacterData> character_data_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &character_data_entry: character_data_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(character_data_entry.account_id));
			insert_values.push_back("'" + EscapeString(character_data_entry.name) + "'");
			insert_values.push_back("'" + EscapeString(character_data_entry.last_name) + "'");
			insert_values.push_back("'" + EscapeString(character_data_entry.title) + "'");
			insert_values.push_back("'" + EscapeString(character_data_entry.suffix) + "'");
			insert_values.push_back(std::to_string(character_data_entry.zone_id));
			insert_values.push_back(std::to_string(character_data_entry.zone_instance));
			insert_values.push_back(std::to_string(character_data_entry.y));
			insert_values.push_back(std::to_string(character_data_entry.x));
			insert_values.push_back(std::to_string(character_data_entry.z));
			insert_values.push_back(std::to_string(character_data_entry.heading));
			insert_values.push_back(std::to_string(character_data_entry.gender));
			insert_values.push_back(std::to_string(character_data_entry.race));
			insert_values.push_back(std::to_string(character_data_entry.class));
			insert_values.push_back(std::to_string(character_data_entry.level));
			insert_values.push_back(std::to_string(character_data_entry.deity));
			insert_values.push_back(std::to_string(character_data_entry.birthday));
			insert_values.push_back(std::to_string(character_data_entry.last_login));
			insert_values.push_back(std::to_string(character_data_entry.time_played));
			insert_values.push_back(std::to_string(character_data_entry.level2));
			insert_values.push_back(std::to_string(character_data_entry.anon));
			insert_values.push_back(std::to_string(character_data_entry.gm));
			insert_values.push_back(std::to_string(character_data_entry.face));
			insert_values.push_back(std::to_string(character_data_entry.hair_color));
			insert_values.push_back(std::to_string(character_data_entry.hair_style));
			insert_values.push_back(std::to_string(character_data_entry.beard));
			insert_values.push_back(std::to_string(character_data_entry.beard_color));
			insert_values.push_back(std::to_string(character_data_entry.eye_color_1));
			insert_values.push_back(std::to_string(character_data_entry.eye_color_2));
			insert_values.push_back(std::to_string(character_data_entry.drakkin_heritage));
			insert_values.push_back(std::to_string(character_data_entry.drakkin_tattoo));
			insert_values.push_back(std::to_string(character_data_entry.drakkin_details));
			insert_values.push_back(std::to_string(character_data_entry.ability_time_seconds));
			insert_values.push_back(std::to_string(character_data_entry.ability_number));
			insert_values.push_back(std::to_string(character_data_entry.ability_time_minutes));
			insert_values.push_back(std::to_string(character_data_entry.ability_time_hours));
			insert_values.push_back(std::to_string(character_data_entry.exp));
			insert_values.push_back(std::to_string(character_data_entry.aa_points_spent));
			insert_values.push_back(std::to_string(character_data_entry.aa_exp));
			insert_values.push_back(std::to_string(character_data_entry.aa_points));
			insert_values.push_back(std::to_string(character_data_entry.group_leadership_exp));
			insert_values.push_back(std::to_string(character_data_entry.raid_leadership_exp));
			insert_values.push_back(std::to_string(character_data_entry.group_leadership_points));
			insert_values.push_back(std::to_string(character_data_entry.raid_leadership_points));
			insert_values.push_back(std::to_string(character_data_entry.points));
			insert_values.push_back(std::to_string(character_data_entry.cur_hp));
			insert_values.push_back(std::to_string(character_data_entry.mana));
			insert_values.push_back(std::to_string(character_data_entry.endurance));
			insert_values.push_back(std::to_string(character_data_entry.intoxication));
			insert_values.push_back(std::to_string(character_data_entry.str));
			insert_values.push_back(std::to_string(character_data_entry.sta));
			insert_values.push_back(std::to_string(character_data_entry.cha));
			insert_values.push_back(std::to_string(character_data_entry.dex));
			insert_values.push_back(std::to_string(character_data_entry.int));
			insert_values.push_back(std::to_string(character_data_entry.agi));
			insert_values.push_back(std::to_string(character_data_entry.wis));
			insert_values.push_back(std::to_string(character_data_entry.zone_change_count));
			insert_values.push_back(std::to_string(character_data_entry.toxicity));
			insert_values.push_back(std::to_string(character_data_entry.hunger_level));
			insert_values.push_back(std::to_string(character_data_entry.thirst_level));
			insert_values.push_back(std::to_string(character_data_entry.ability_up));
			insert_values.push_back(std::to_string(character_data_entry.ldon_points_guk));
			insert_values.push_back(std::to_string(character_data_entry.ldon_points_mir));
			insert_values.push_back(std::to_string(character_data_entry.ldon_points_mmc));
			insert_values.push_back(std::to_string(character_data_entry.ldon_points_ruj));
			insert_values.push_back(std::to_string(character_data_entry.ldon_points_tak));
			insert_values.push_back(std::to_string(character_data_entry.ldon_points_available));
			insert_values.push_back(std::to_string(character_data_entry.tribute_time_remaining));
			insert_values.push_back(std::to_string(character_data_entry.career_tribute_points));
			insert_values.push_back(std::to_string(character_data_entry.tribute_points));
			insert_values.push_back(std::to_string(character_data_entry.tribute_active));
			insert_values.push_back(std::to_string(character_data_entry.pvp_status));
			insert_values.push_back(std::to_string(character_data_entry.pvp_kills));
			insert_values.push_back(std::to_string(character_data_entry.pvp_deaths));
			insert_values.push_back(std::to_string(character_data_entry.pvp_current_points));
			insert_values.push_back(std::to_string(character_data_entry.pvp_career_points));
			insert_values.push_back(std::to_string(character_data_entry.pvp_best_kill_streak));
			insert_values.push_back(std::to_string(character_data_entry.pvp_worst_death_streak));
			insert_values.push_back(std::to_string(character_data_entry.pvp_current_kill_streak));
			insert_values.push_back(std::to_string(character_data_entry.pvp2));
			insert_values.push_back(std::to_string(character_data_entry.pvp_type));
			insert_values.push_back(std::to_string(character_data_entry.show_helm));
			insert_values.push_back(std::to_string(character_data_entry.group_auto_consent));
			insert_values.push_back(std::to_string(character_data_entry.raid_auto_consent));
			insert_values.push_back(std::to_string(character_data_entry.guild_auto_consent));
			insert_values.push_back(std::to_string(character_data_entry.leadership_exp_on));
			insert_values.push_back(std::to_string(character_data_entry.RestTimer));
			insert_values.push_back(std::to_string(character_data_entry.air_remaining));
			insert_values.push_back(std::to_string(character_data_entry.autosplit_enabled));
			insert_values.push_back(std::to_string(character_data_entry.lfp));
			insert_values.push_back(std::to_string(character_data_entry.lfg));
			insert_values.push_back("'" + EscapeString(character_data_entry.mailkey) + "'");
			insert_values.push_back(std::to_string(character_data_entry.xtargets));
			insert_values.push_back(std::to_string(character_data_entry.firstlogon));
			insert_values.push_back(std::to_string(character_data_entry.e_aa_effects));
			insert_values.push_back(std::to_string(character_data_entry.e_percent_to_aa));
			insert_values.push_back(std::to_string(character_data_entry.e_expended_aa_spent));
			insert_values.push_back(std::to_string(character_data_entry.aa_points_spent_old));
			insert_values.push_back(std::to_string(character_data_entry.aa_points_old));
			insert_values.push_back(std::to_string(character_data_entry.e_last_invsnapshot));
			insert_values.push_back("'" + EscapeString(character_data_entry.deleted_at) + "'");

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<CharacterData> All()
	{
		std::vector<CharacterData> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterData entry{};

			entry.id                      = atoi(row[0]);
			entry.account_id              = atoi(row[1]);
			entry.name                    = row[2] ? row[2] : "";
			entry.last_name               = row[3] ? row[3] : "";
			entry.title                   = row[4] ? row[4] : "";
			entry.suffix                  = row[5] ? row[5] : "";
			entry.zone_id                 = atoi(row[6]);
			entry.zone_instance           = atoi(row[7]);
			entry.y                       = static_cast<float>(atof(row[8]));
			entry.x                       = static_cast<float>(atof(row[9]));
			entry.z                       = static_cast<float>(atof(row[10]));
			entry.heading                 = static_cast<float>(atof(row[11]));
			entry.gender                  = atoi(row[12]);
			entry.race                    = atoi(row[13]);
			entry.class                   = atoi(row[14]);
			entry.level                   = atoi(row[15]);
			entry.deity                   = atoi(row[16]);
			entry.birthday                = atoi(row[17]);
			entry.last_login              = atoi(row[18]);
			entry.time_played             = atoi(row[19]);
			entry.level2                  = atoi(row[20]);
			entry.anon                    = atoi(row[21]);
			entry.gm                      = atoi(row[22]);
			entry.face                    = atoi(row[23]);
			entry.hair_color              = atoi(row[24]);
			entry.hair_style              = atoi(row[25]);
			entry.beard                   = atoi(row[26]);
			entry.beard_color             = atoi(row[27]);
			entry.eye_color_1             = atoi(row[28]);
			entry.eye_color_2             = atoi(row[29]);
			entry.drakkin_heritage        = atoi(row[30]);
			entry.drakkin_tattoo          = atoi(row[31]);
			entry.drakkin_details         = atoi(row[32]);
			entry.ability_time_seconds    = atoi(row[33]);
			entry.ability_number          = atoi(row[34]);
			entry.ability_time_minutes    = atoi(row[35]);
			entry.ability_time_hours      = atoi(row[36]);
			entry.exp                     = atoi(row[37]);
			entry.aa_points_spent         = atoi(row[38]);
			entry.aa_exp                  = atoi(row[39]);
			entry.aa_points               = atoi(row[40]);
			entry.group_leadership_exp    = atoi(row[41]);
			entry.raid_leadership_exp     = atoi(row[42]);
			entry.group_leadership_points = atoi(row[43]);
			entry.raid_leadership_points  = atoi(row[44]);
			entry.points                  = atoi(row[45]);
			entry.cur_hp                  = atoi(row[46]);
			entry.mana                    = atoi(row[47]);
			entry.endurance               = atoi(row[48]);
			entry.intoxication            = atoi(row[49]);
			entry.str                     = atoi(row[50]);
			entry.sta                     = atoi(row[51]);
			entry.cha                     = atoi(row[52]);
			entry.dex                     = atoi(row[53]);
			entry.int                     = atoi(row[54]);
			entry.agi                     = atoi(row[55]);
			entry.wis                     = atoi(row[56]);
			entry.zone_change_count       = atoi(row[57]);
			entry.toxicity                = atoi(row[58]);
			entry.hunger_level            = atoi(row[59]);
			entry.thirst_level            = atoi(row[60]);
			entry.ability_up              = atoi(row[61]);
			entry.ldon_points_guk         = atoi(row[62]);
			entry.ldon_points_mir         = atoi(row[63]);
			entry.ldon_points_mmc         = atoi(row[64]);
			entry.ldon_points_ruj         = atoi(row[65]);
			entry.ldon_points_tak         = atoi(row[66]);
			entry.ldon_points_available   = atoi(row[67]);
			entry.tribute_time_remaining  = atoi(row[68]);
			entry.career_tribute_points   = atoi(row[69]);
			entry.tribute_points          = atoi(row[70]);
			entry.tribute_active          = atoi(row[71]);
			entry.pvp_status              = atoi(row[72]);
			entry.pvp_kills               = atoi(row[73]);
			entry.pvp_deaths              = atoi(row[74]);
			entry.pvp_current_points      = atoi(row[75]);
			entry.pvp_career_points       = atoi(row[76]);
			entry.pvp_best_kill_streak    = atoi(row[77]);
			entry.pvp_worst_death_streak  = atoi(row[78]);
			entry.pvp_current_kill_streak = atoi(row[79]);
			entry.pvp2                    = atoi(row[80]);
			entry.pvp_type                = atoi(row[81]);
			entry.show_helm               = atoi(row[82]);
			entry.group_auto_consent      = atoi(row[83]);
			entry.raid_auto_consent       = atoi(row[84]);
			entry.guild_auto_consent      = atoi(row[85]);
			entry.leadership_exp_on       = atoi(row[86]);
			entry.RestTimer               = atoi(row[87]);
			entry.air_remaining           = atoi(row[88]);
			entry.autosplit_enabled       = atoi(row[89]);
			entry.lfp                     = atoi(row[90]);
			entry.lfg                     = atoi(row[91]);
			entry.mailkey                 = row[92] ? row[92] : "";
			entry.xtargets                = atoi(row[93]);
			entry.firstlogon              = atoi(row[94]);
			entry.e_aa_effects            = atoi(row[95]);
			entry.e_percent_to_aa         = atoi(row[96]);
			entry.e_expended_aa_spent     = atoi(row[97]);
			entry.aa_points_spent_old     = atoi(row[98]);
			entry.aa_points_old           = atoi(row[99]);
			entry.e_last_invsnapshot      = atoi(row[100]);
			entry.deleted_at              = row[101] ? row[101] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CharacterData> GetWhere(std::string where_filter)
	{
		std::vector<CharacterData> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterData entry{};

			entry.id                      = atoi(row[0]);
			entry.account_id              = atoi(row[1]);
			entry.name                    = row[2] ? row[2] : "";
			entry.last_name               = row[3] ? row[3] : "";
			entry.title                   = row[4] ? row[4] : "";
			entry.suffix                  = row[5] ? row[5] : "";
			entry.zone_id                 = atoi(row[6]);
			entry.zone_instance           = atoi(row[7]);
			entry.y                       = static_cast<float>(atof(row[8]));
			entry.x                       = static_cast<float>(atof(row[9]));
			entry.z                       = static_cast<float>(atof(row[10]));
			entry.heading                 = static_cast<float>(atof(row[11]));
			entry.gender                  = atoi(row[12]);
			entry.race                    = atoi(row[13]);
			entry.class                   = atoi(row[14]);
			entry.level                   = atoi(row[15]);
			entry.deity                   = atoi(row[16]);
			entry.birthday                = atoi(row[17]);
			entry.last_login              = atoi(row[18]);
			entry.time_played             = atoi(row[19]);
			entry.level2                  = atoi(row[20]);
			entry.anon                    = atoi(row[21]);
			entry.gm                      = atoi(row[22]);
			entry.face                    = atoi(row[23]);
			entry.hair_color              = atoi(row[24]);
			entry.hair_style              = atoi(row[25]);
			entry.beard                   = atoi(row[26]);
			entry.beard_color             = atoi(row[27]);
			entry.eye_color_1             = atoi(row[28]);
			entry.eye_color_2             = atoi(row[29]);
			entry.drakkin_heritage        = atoi(row[30]);
			entry.drakkin_tattoo          = atoi(row[31]);
			entry.drakkin_details         = atoi(row[32]);
			entry.ability_time_seconds    = atoi(row[33]);
			entry.ability_number          = atoi(row[34]);
			entry.ability_time_minutes    = atoi(row[35]);
			entry.ability_time_hours      = atoi(row[36]);
			entry.exp                     = atoi(row[37]);
			entry.aa_points_spent         = atoi(row[38]);
			entry.aa_exp                  = atoi(row[39]);
			entry.aa_points               = atoi(row[40]);
			entry.group_leadership_exp    = atoi(row[41]);
			entry.raid_leadership_exp     = atoi(row[42]);
			entry.group_leadership_points = atoi(row[43]);
			entry.raid_leadership_points  = atoi(row[44]);
			entry.points                  = atoi(row[45]);
			entry.cur_hp                  = atoi(row[46]);
			entry.mana                    = atoi(row[47]);
			entry.endurance               = atoi(row[48]);
			entry.intoxication            = atoi(row[49]);
			entry.str                     = atoi(row[50]);
			entry.sta                     = atoi(row[51]);
			entry.cha                     = atoi(row[52]);
			entry.dex                     = atoi(row[53]);
			entry.int                     = atoi(row[54]);
			entry.agi                     = atoi(row[55]);
			entry.wis                     = atoi(row[56]);
			entry.zone_change_count       = atoi(row[57]);
			entry.toxicity                = atoi(row[58]);
			entry.hunger_level            = atoi(row[59]);
			entry.thirst_level            = atoi(row[60]);
			entry.ability_up              = atoi(row[61]);
			entry.ldon_points_guk         = atoi(row[62]);
			entry.ldon_points_mir         = atoi(row[63]);
			entry.ldon_points_mmc         = atoi(row[64]);
			entry.ldon_points_ruj         = atoi(row[65]);
			entry.ldon_points_tak         = atoi(row[66]);
			entry.ldon_points_available   = atoi(row[67]);
			entry.tribute_time_remaining  = atoi(row[68]);
			entry.career_tribute_points   = atoi(row[69]);
			entry.tribute_points          = atoi(row[70]);
			entry.tribute_active          = atoi(row[71]);
			entry.pvp_status              = atoi(row[72]);
			entry.pvp_kills               = atoi(row[73]);
			entry.pvp_deaths              = atoi(row[74]);
			entry.pvp_current_points      = atoi(row[75]);
			entry.pvp_career_points       = atoi(row[76]);
			entry.pvp_best_kill_streak    = atoi(row[77]);
			entry.pvp_worst_death_streak  = atoi(row[78]);
			entry.pvp_current_kill_streak = atoi(row[79]);
			entry.pvp2                    = atoi(row[80]);
			entry.pvp_type                = atoi(row[81]);
			entry.show_helm               = atoi(row[82]);
			entry.group_auto_consent      = atoi(row[83]);
			entry.raid_auto_consent       = atoi(row[84]);
			entry.guild_auto_consent      = atoi(row[85]);
			entry.leadership_exp_on       = atoi(row[86]);
			entry.RestTimer               = atoi(row[87]);
			entry.air_remaining           = atoi(row[88]);
			entry.autosplit_enabled       = atoi(row[89]);
			entry.lfp                     = atoi(row[90]);
			entry.lfg                     = atoi(row[91]);
			entry.mailkey                 = row[92] ? row[92] : "";
			entry.xtargets                = atoi(row[93]);
			entry.firstlogon              = atoi(row[94]);
			entry.e_aa_effects            = atoi(row[95]);
			entry.e_percent_to_aa         = atoi(row[96]);
			entry.e_expended_aa_spent     = atoi(row[97]);
			entry.aa_points_spent_old     = atoi(row[98]);
			entry.aa_points_old           = atoi(row[99]);
			entry.e_last_invsnapshot      = atoi(row[100]);
			entry.deleted_at              = row[101] ? row[101] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static int DeleteWhere(std::string where_filter)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate()
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_CHARACTER_DATA_REPOSITORY_H
