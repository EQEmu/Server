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

#ifndef EQEMU_BASE_ZONE_REPOSITORY_H
#define EQEMU_BASE_ZONE_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseZoneRepository {
public:
	struct Zone {
		std::string short_name;
		int         id;
		std::string file_name;
		std::string long_name;
		std::string map_file_name;
		float       safe_x;
		float       safe_y;
		float       safe_z;
		float       safe_heading;
		float       graveyard_id;
		int         min_level;
		int         min_status;
		int         zoneidnumber;
		int         version;
		int         timezone;
		int         maxclients;
		int         ruleset;
		std::string note;
		float       underworld;
		float       minclip;
		float       maxclip;
		float       fog_minclip;
		float       fog_maxclip;
		int         fog_blue;
		int         fog_red;
		int         fog_green;
		int         sky;
		int         ztype;
		float       zone_exp_multiplier;
		float       walkspeed;
		int         time_type;
		int         fog_red1;
		int         fog_green1;
		int         fog_blue1;
		float       fog_minclip1;
		float       fog_maxclip1;
		int         fog_red2;
		int         fog_green2;
		int         fog_blue2;
		float       fog_minclip2;
		float       fog_maxclip2;
		int         fog_red3;
		int         fog_green3;
		int         fog_blue3;
		float       fog_minclip3;
		float       fog_maxclip3;
		int         fog_red4;
		int         fog_green4;
		int         fog_blue4;
		float       fog_minclip4;
		float       fog_maxclip4;
		float       fog_density;
		std::string flag_needed;
		int         canbind;
		int         cancombat;
		int         canlevitate;
		int         castoutdoor;
		int         hotzone;
		int         insttype;
		int64       shutdowndelay;
		int         peqzone;
		int         suspendbuffs;
		int         rain_chance1;
		int         rain_chance2;
		int         rain_chance3;
		int         rain_chance4;
		int         rain_duration1;
		int         rain_duration2;
		int         rain_duration3;
		int         rain_duration4;
		int         snow_chance1;
		int         snow_chance2;
		int         snow_chance3;
		int         snow_chance4;
		int         snow_duration1;
		int         snow_duration2;
		int         snow_duration3;
		int         snow_duration4;
		float       gravity;
		int         type;
		int         skylock;
		int         fast_regen_hp;
		int         fast_regen_mana;
		int         fast_regen_endurance;
		int         npc_max_aggro_dist;
		int         max_movement_update_range;
		int         min_expansion;
		int         max_expansion;
		std::string content_flags;
		std::string content_flags_disabled;
		int         underworld_teleport_index;
		int         lava_damage;
		int         min_lava_damage;
		int         expansion;
		int         bypass_expansion_check;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"short_name",
			"id",
			"file_name",
			"long_name",
			"map_file_name",
			"safe_x",
			"safe_y",
			"safe_z",
			"safe_heading",
			"graveyard_id",
			"min_level",
			"min_status",
			"zoneidnumber",
			"version",
			"timezone",
			"maxclients",
			"ruleset",
			"note",
			"underworld",
			"minclip",
			"maxclip",
			"fog_minclip",
			"fog_maxclip",
			"fog_blue",
			"fog_red",
			"fog_green",
			"sky",
			"ztype",
			"zone_exp_multiplier",
			"walkspeed",
			"time_type",
			"fog_red1",
			"fog_green1",
			"fog_blue1",
			"fog_minclip1",
			"fog_maxclip1",
			"fog_red2",
			"fog_green2",
			"fog_blue2",
			"fog_minclip2",
			"fog_maxclip2",
			"fog_red3",
			"fog_green3",
			"fog_blue3",
			"fog_minclip3",
			"fog_maxclip3",
			"fog_red4",
			"fog_green4",
			"fog_blue4",
			"fog_minclip4",
			"fog_maxclip4",
			"fog_density",
			"flag_needed",
			"canbind",
			"cancombat",
			"canlevitate",
			"castoutdoor",
			"hotzone",
			"insttype",
			"shutdowndelay",
			"peqzone",
			"suspendbuffs",
			"rain_chance1",
			"rain_chance2",
			"rain_chance3",
			"rain_chance4",
			"rain_duration1",
			"rain_duration2",
			"rain_duration3",
			"rain_duration4",
			"snow_chance1",
			"snow_chance2",
			"snow_chance3",
			"snow_chance4",
			"snow_duration1",
			"snow_duration2",
			"snow_duration3",
			"snow_duration4",
			"gravity",
			"type",
			"skylock",
			"fast_regen_hp",
			"fast_regen_mana",
			"fast_regen_endurance",
			"npc_max_aggro_dist",
			"max_movement_update_range",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
			"underworld_teleport_index",
			"lava_damage",
			"min_lava_damage",
			"expansion",
			"bypass_expansion_check",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"short_name",
			"id",
			"file_name",
			"long_name",
			"map_file_name",
			"safe_x",
			"safe_y",
			"safe_z",
			"safe_heading",
			"graveyard_id",
			"min_level",
			"min_status",
			"zoneidnumber",
			"version",
			"timezone",
			"maxclients",
			"ruleset",
			"note",
			"underworld",
			"minclip",
			"maxclip",
			"fog_minclip",
			"fog_maxclip",
			"fog_blue",
			"fog_red",
			"fog_green",
			"sky",
			"ztype",
			"zone_exp_multiplier",
			"walkspeed",
			"time_type",
			"fog_red1",
			"fog_green1",
			"fog_blue1",
			"fog_minclip1",
			"fog_maxclip1",
			"fog_red2",
			"fog_green2",
			"fog_blue2",
			"fog_minclip2",
			"fog_maxclip2",
			"fog_red3",
			"fog_green3",
			"fog_blue3",
			"fog_minclip3",
			"fog_maxclip3",
			"fog_red4",
			"fog_green4",
			"fog_blue4",
			"fog_minclip4",
			"fog_maxclip4",
			"fog_density",
			"flag_needed",
			"canbind",
			"cancombat",
			"canlevitate",
			"castoutdoor",
			"hotzone",
			"insttype",
			"shutdowndelay",
			"peqzone",
			"suspendbuffs",
			"rain_chance1",
			"rain_chance2",
			"rain_chance3",
			"rain_chance4",
			"rain_duration1",
			"rain_duration2",
			"rain_duration3",
			"rain_duration4",
			"snow_chance1",
			"snow_chance2",
			"snow_chance3",
			"snow_chance4",
			"snow_duration1",
			"snow_duration2",
			"snow_duration3",
			"snow_duration4",
			"gravity",
			"type",
			"skylock",
			"fast_regen_hp",
			"fast_regen_mana",
			"fast_regen_endurance",
			"npc_max_aggro_dist",
			"max_movement_update_range",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
			"underworld_teleport_index",
			"lava_damage",
			"min_lava_damage",
			"expansion",
			"bypass_expansion_check",
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
		return std::string("zone");
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

	static Zone NewEntity()
	{
		Zone e{};

		e.short_name                = "";
		e.id                        = 0;
		e.file_name                 = "";
		e.long_name                 = "";
		e.map_file_name             = "";
		e.safe_x                    = 0;
		e.safe_y                    = 0;
		e.safe_z                    = 0;
		e.safe_heading              = 0;
		e.graveyard_id              = 0;
		e.min_level                 = 0;
		e.min_status                = 0;
		e.zoneidnumber              = 0;
		e.version                   = 0;
		e.timezone                  = 0;
		e.maxclients                = 0;
		e.ruleset                   = 0;
		e.note                      = "";
		e.underworld                = 0;
		e.minclip                   = 450;
		e.maxclip                   = 450;
		e.fog_minclip               = 450;
		e.fog_maxclip               = 450;
		e.fog_blue                  = 0;
		e.fog_red                   = 0;
		e.fog_green                 = 0;
		e.sky                       = 1;
		e.ztype                     = 1;
		e.zone_exp_multiplier       = 0.00;
		e.walkspeed                 = 0.4;
		e.time_type                 = 2;
		e.fog_red1                  = 0;
		e.fog_green1                = 0;
		e.fog_blue1                 = 0;
		e.fog_minclip1              = 450;
		e.fog_maxclip1              = 450;
		e.fog_red2                  = 0;
		e.fog_green2                = 0;
		e.fog_blue2                 = 0;
		e.fog_minclip2              = 450;
		e.fog_maxclip2              = 450;
		e.fog_red3                  = 0;
		e.fog_green3                = 0;
		e.fog_blue3                 = 0;
		e.fog_minclip3              = 450;
		e.fog_maxclip3              = 450;
		e.fog_red4                  = 0;
		e.fog_green4                = 0;
		e.fog_blue4                 = 0;
		e.fog_minclip4              = 450;
		e.fog_maxclip4              = 450;
		e.fog_density               = 0;
		e.flag_needed               = "";
		e.canbind                   = 1;
		e.cancombat                 = 1;
		e.canlevitate               = 1;
		e.castoutdoor               = 1;
		e.hotzone                   = 0;
		e.insttype                  = 0;
		e.shutdowndelay             = 5000;
		e.peqzone                   = 1;
		e.suspendbuffs              = 0;
		e.rain_chance1              = 0;
		e.rain_chance2              = 0;
		e.rain_chance3              = 0;
		e.rain_chance4              = 0;
		e.rain_duration1            = 0;
		e.rain_duration2            = 0;
		e.rain_duration3            = 0;
		e.rain_duration4            = 0;
		e.snow_chance1              = 0;
		e.snow_chance2              = 0;
		e.snow_chance3              = 0;
		e.snow_chance4              = 0;
		e.snow_duration1            = 0;
		e.snow_duration2            = 0;
		e.snow_duration3            = 0;
		e.snow_duration4            = 0;
		e.gravity                   = 0.4;
		e.type                      = 0;
		e.skylock                   = 0;
		e.fast_regen_hp             = 180;
		e.fast_regen_mana           = 180;
		e.fast_regen_endurance      = 180;
		e.npc_max_aggro_dist        = 600;
		e.max_movement_update_range = 600;
		e.min_expansion             = -1;
		e.max_expansion             = -1;
		e.content_flags             = "";
		e.content_flags_disabled    = "";
		e.underworld_teleport_index = 0;
		e.lava_damage               = 50;
		e.min_lava_damage           = 10;
		e.expansion                 = 0;
		e.bypass_expansion_check    = 0;

		return e;
	}

	static Zone GetZone(
		const std::vector<Zone> &zones,
		int zone_id
	)
	{
		for (auto &zone : zones) {
			if (zone.id == zone_id) {
				return zone;
			}
		}

		return NewEntity();
	}

	static Zone FindOne(
		Database& db,
		int zone_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				zone_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Zone e{};

			e.short_name                = row[0] ? row[0] : "";
			e.id                        = atoi(row[1]);
			e.file_name                 = row[2] ? row[2] : "";
			e.long_name                 = row[3] ? row[3] : "";
			e.map_file_name             = row[4] ? row[4] : "";
			e.safe_x                    = static_cast<float>(atof(row[5]));
			e.safe_y                    = static_cast<float>(atof(row[6]));
			e.safe_z                    = static_cast<float>(atof(row[7]));
			e.safe_heading              = static_cast<float>(atof(row[8]));
			e.graveyard_id              = static_cast<float>(atof(row[9]));
			e.min_level                 = atoi(row[10]);
			e.min_status                = atoi(row[11]);
			e.zoneidnumber              = atoi(row[12]);
			e.version                   = atoi(row[13]);
			e.timezone                  = atoi(row[14]);
			e.maxclients                = atoi(row[15]);
			e.ruleset                   = atoi(row[16]);
			e.note                      = row[17] ? row[17] : "";
			e.underworld                = static_cast<float>(atof(row[18]));
			e.minclip                   = static_cast<float>(atof(row[19]));
			e.maxclip                   = static_cast<float>(atof(row[20]));
			e.fog_minclip               = static_cast<float>(atof(row[21]));
			e.fog_maxclip               = static_cast<float>(atof(row[22]));
			e.fog_blue                  = atoi(row[23]);
			e.fog_red                   = atoi(row[24]);
			e.fog_green                 = atoi(row[25]);
			e.sky                       = atoi(row[26]);
			e.ztype                     = atoi(row[27]);
			e.zone_exp_multiplier       = static_cast<float>(atof(row[28]));
			e.walkspeed                 = static_cast<float>(atof(row[29]));
			e.time_type                 = atoi(row[30]);
			e.fog_red1                  = atoi(row[31]);
			e.fog_green1                = atoi(row[32]);
			e.fog_blue1                 = atoi(row[33]);
			e.fog_minclip1              = static_cast<float>(atof(row[34]));
			e.fog_maxclip1              = static_cast<float>(atof(row[35]));
			e.fog_red2                  = atoi(row[36]);
			e.fog_green2                = atoi(row[37]);
			e.fog_blue2                 = atoi(row[38]);
			e.fog_minclip2              = static_cast<float>(atof(row[39]));
			e.fog_maxclip2              = static_cast<float>(atof(row[40]));
			e.fog_red3                  = atoi(row[41]);
			e.fog_green3                = atoi(row[42]);
			e.fog_blue3                 = atoi(row[43]);
			e.fog_minclip3              = static_cast<float>(atof(row[44]));
			e.fog_maxclip3              = static_cast<float>(atof(row[45]));
			e.fog_red4                  = atoi(row[46]);
			e.fog_green4                = atoi(row[47]);
			e.fog_blue4                 = atoi(row[48]);
			e.fog_minclip4              = static_cast<float>(atof(row[49]));
			e.fog_maxclip4              = static_cast<float>(atof(row[50]));
			e.fog_density               = static_cast<float>(atof(row[51]));
			e.flag_needed               = row[52] ? row[52] : "";
			e.canbind                   = atoi(row[53]);
			e.cancombat                 = atoi(row[54]);
			e.canlevitate               = atoi(row[55]);
			e.castoutdoor               = atoi(row[56]);
			e.hotzone                   = atoi(row[57]);
			e.insttype                  = atoi(row[58]);
			e.shutdowndelay             = strtoll(row[59], nullptr, 10);
			e.peqzone                   = atoi(row[60]);
			e.suspendbuffs              = atoi(row[61]);
			e.rain_chance1              = atoi(row[62]);
			e.rain_chance2              = atoi(row[63]);
			e.rain_chance3              = atoi(row[64]);
			e.rain_chance4              = atoi(row[65]);
			e.rain_duration1            = atoi(row[66]);
			e.rain_duration2            = atoi(row[67]);
			e.rain_duration3            = atoi(row[68]);
			e.rain_duration4            = atoi(row[69]);
			e.snow_chance1              = atoi(row[70]);
			e.snow_chance2              = atoi(row[71]);
			e.snow_chance3              = atoi(row[72]);
			e.snow_chance4              = atoi(row[73]);
			e.snow_duration1            = atoi(row[74]);
			e.snow_duration2            = atoi(row[75]);
			e.snow_duration3            = atoi(row[76]);
			e.snow_duration4            = atoi(row[77]);
			e.gravity                   = static_cast<float>(atof(row[78]));
			e.type                      = atoi(row[79]);
			e.skylock                   = atoi(row[80]);
			e.fast_regen_hp             = atoi(row[81]);
			e.fast_regen_mana           = atoi(row[82]);
			e.fast_regen_endurance      = atoi(row[83]);
			e.npc_max_aggro_dist        = atoi(row[84]);
			e.max_movement_update_range = atoi(row[85]);
			e.min_expansion             = atoi(row[86]);
			e.max_expansion             = atoi(row[87]);
			e.content_flags             = row[88] ? row[88] : "";
			e.content_flags_disabled    = row[89] ? row[89] : "";
			e.underworld_teleport_index = atoi(row[90]);
			e.lava_damage               = atoi(row[91]);
			e.min_lava_damage           = atoi(row[92]);
			e.expansion                 = atoi(row[93]);
			e.bypass_expansion_check    = atoi(row[94]);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int zone_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				zone_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Zone &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = '" + Strings::Escape(e.short_name) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.file_name) + "'");
		v.push_back(columns[3] + " = '" + Strings::Escape(e.long_name) + "'");
		v.push_back(columns[4] + " = '" + Strings::Escape(e.map_file_name) + "'");
		v.push_back(columns[5] + " = " + std::to_string(e.safe_x));
		v.push_back(columns[6] + " = " + std::to_string(e.safe_y));
		v.push_back(columns[7] + " = " + std::to_string(e.safe_z));
		v.push_back(columns[8] + " = " + std::to_string(e.safe_heading));
		v.push_back(columns[9] + " = " + std::to_string(e.graveyard_id));
		v.push_back(columns[10] + " = " + std::to_string(e.min_level));
		v.push_back(columns[11] + " = " + std::to_string(e.min_status));
		v.push_back(columns[12] + " = " + std::to_string(e.zoneidnumber));
		v.push_back(columns[13] + " = " + std::to_string(e.version));
		v.push_back(columns[14] + " = " + std::to_string(e.timezone));
		v.push_back(columns[15] + " = " + std::to_string(e.maxclients));
		v.push_back(columns[16] + " = " + std::to_string(e.ruleset));
		v.push_back(columns[17] + " = '" + Strings::Escape(e.note) + "'");
		v.push_back(columns[18] + " = " + std::to_string(e.underworld));
		v.push_back(columns[19] + " = " + std::to_string(e.minclip));
		v.push_back(columns[20] + " = " + std::to_string(e.maxclip));
		v.push_back(columns[21] + " = " + std::to_string(e.fog_minclip));
		v.push_back(columns[22] + " = " + std::to_string(e.fog_maxclip));
		v.push_back(columns[23] + " = " + std::to_string(e.fog_blue));
		v.push_back(columns[24] + " = " + std::to_string(e.fog_red));
		v.push_back(columns[25] + " = " + std::to_string(e.fog_green));
		v.push_back(columns[26] + " = " + std::to_string(e.sky));
		v.push_back(columns[27] + " = " + std::to_string(e.ztype));
		v.push_back(columns[28] + " = " + std::to_string(e.zone_exp_multiplier));
		v.push_back(columns[29] + " = " + std::to_string(e.walkspeed));
		v.push_back(columns[30] + " = " + std::to_string(e.time_type));
		v.push_back(columns[31] + " = " + std::to_string(e.fog_red1));
		v.push_back(columns[32] + " = " + std::to_string(e.fog_green1));
		v.push_back(columns[33] + " = " + std::to_string(e.fog_blue1));
		v.push_back(columns[34] + " = " + std::to_string(e.fog_minclip1));
		v.push_back(columns[35] + " = " + std::to_string(e.fog_maxclip1));
		v.push_back(columns[36] + " = " + std::to_string(e.fog_red2));
		v.push_back(columns[37] + " = " + std::to_string(e.fog_green2));
		v.push_back(columns[38] + " = " + std::to_string(e.fog_blue2));
		v.push_back(columns[39] + " = " + std::to_string(e.fog_minclip2));
		v.push_back(columns[40] + " = " + std::to_string(e.fog_maxclip2));
		v.push_back(columns[41] + " = " + std::to_string(e.fog_red3));
		v.push_back(columns[42] + " = " + std::to_string(e.fog_green3));
		v.push_back(columns[43] + " = " + std::to_string(e.fog_blue3));
		v.push_back(columns[44] + " = " + std::to_string(e.fog_minclip3));
		v.push_back(columns[45] + " = " + std::to_string(e.fog_maxclip3));
		v.push_back(columns[46] + " = " + std::to_string(e.fog_red4));
		v.push_back(columns[47] + " = " + std::to_string(e.fog_green4));
		v.push_back(columns[48] + " = " + std::to_string(e.fog_blue4));
		v.push_back(columns[49] + " = " + std::to_string(e.fog_minclip4));
		v.push_back(columns[50] + " = " + std::to_string(e.fog_maxclip4));
		v.push_back(columns[51] + " = " + std::to_string(e.fog_density));
		v.push_back(columns[52] + " = '" + Strings::Escape(e.flag_needed) + "'");
		v.push_back(columns[53] + " = " + std::to_string(e.canbind));
		v.push_back(columns[54] + " = " + std::to_string(e.cancombat));
		v.push_back(columns[55] + " = " + std::to_string(e.canlevitate));
		v.push_back(columns[56] + " = " + std::to_string(e.castoutdoor));
		v.push_back(columns[57] + " = " + std::to_string(e.hotzone));
		v.push_back(columns[58] + " = " + std::to_string(e.insttype));
		v.push_back(columns[59] + " = " + std::to_string(e.shutdowndelay));
		v.push_back(columns[60] + " = " + std::to_string(e.peqzone));
		v.push_back(columns[61] + " = " + std::to_string(e.suspendbuffs));
		v.push_back(columns[62] + " = " + std::to_string(e.rain_chance1));
		v.push_back(columns[63] + " = " + std::to_string(e.rain_chance2));
		v.push_back(columns[64] + " = " + std::to_string(e.rain_chance3));
		v.push_back(columns[65] + " = " + std::to_string(e.rain_chance4));
		v.push_back(columns[66] + " = " + std::to_string(e.rain_duration1));
		v.push_back(columns[67] + " = " + std::to_string(e.rain_duration2));
		v.push_back(columns[68] + " = " + std::to_string(e.rain_duration3));
		v.push_back(columns[69] + " = " + std::to_string(e.rain_duration4));
		v.push_back(columns[70] + " = " + std::to_string(e.snow_chance1));
		v.push_back(columns[71] + " = " + std::to_string(e.snow_chance2));
		v.push_back(columns[72] + " = " + std::to_string(e.snow_chance3));
		v.push_back(columns[73] + " = " + std::to_string(e.snow_chance4));
		v.push_back(columns[74] + " = " + std::to_string(e.snow_duration1));
		v.push_back(columns[75] + " = " + std::to_string(e.snow_duration2));
		v.push_back(columns[76] + " = " + std::to_string(e.snow_duration3));
		v.push_back(columns[77] + " = " + std::to_string(e.snow_duration4));
		v.push_back(columns[78] + " = " + std::to_string(e.gravity));
		v.push_back(columns[79] + " = " + std::to_string(e.type));
		v.push_back(columns[80] + " = " + std::to_string(e.skylock));
		v.push_back(columns[81] + " = " + std::to_string(e.fast_regen_hp));
		v.push_back(columns[82] + " = " + std::to_string(e.fast_regen_mana));
		v.push_back(columns[83] + " = " + std::to_string(e.fast_regen_endurance));
		v.push_back(columns[84] + " = " + std::to_string(e.npc_max_aggro_dist));
		v.push_back(columns[85] + " = " + std::to_string(e.max_movement_update_range));
		v.push_back(columns[86] + " = " + std::to_string(e.min_expansion));
		v.push_back(columns[87] + " = " + std::to_string(e.max_expansion));
		v.push_back(columns[88] + " = '" + Strings::Escape(e.content_flags) + "'");
		v.push_back(columns[89] + " = '" + Strings::Escape(e.content_flags_disabled) + "'");
		v.push_back(columns[90] + " = " + std::to_string(e.underworld_teleport_index));
		v.push_back(columns[91] + " = " + std::to_string(e.lava_damage));
		v.push_back(columns[92] + " = " + std::to_string(e.min_lava_damage));
		v.push_back(columns[93] + " = " + std::to_string(e.expansion));
		v.push_back(columns[94] + " = " + std::to_string(e.bypass_expansion_check));

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

	static Zone InsertOne(
		Database& db,
		Zone e
	)
	{
		std::vector<std::string> v;

		v.push_back("'" + Strings::Escape(e.short_name) + "'");
		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.file_name) + "'");
		v.push_back("'" + Strings::Escape(e.long_name) + "'");
		v.push_back("'" + Strings::Escape(e.map_file_name) + "'");
		v.push_back(std::to_string(e.safe_x));
		v.push_back(std::to_string(e.safe_y));
		v.push_back(std::to_string(e.safe_z));
		v.push_back(std::to_string(e.safe_heading));
		v.push_back(std::to_string(e.graveyard_id));
		v.push_back(std::to_string(e.min_level));
		v.push_back(std::to_string(e.min_status));
		v.push_back(std::to_string(e.zoneidnumber));
		v.push_back(std::to_string(e.version));
		v.push_back(std::to_string(e.timezone));
		v.push_back(std::to_string(e.maxclients));
		v.push_back(std::to_string(e.ruleset));
		v.push_back("'" + Strings::Escape(e.note) + "'");
		v.push_back(std::to_string(e.underworld));
		v.push_back(std::to_string(e.minclip));
		v.push_back(std::to_string(e.maxclip));
		v.push_back(std::to_string(e.fog_minclip));
		v.push_back(std::to_string(e.fog_maxclip));
		v.push_back(std::to_string(e.fog_blue));
		v.push_back(std::to_string(e.fog_red));
		v.push_back(std::to_string(e.fog_green));
		v.push_back(std::to_string(e.sky));
		v.push_back(std::to_string(e.ztype));
		v.push_back(std::to_string(e.zone_exp_multiplier));
		v.push_back(std::to_string(e.walkspeed));
		v.push_back(std::to_string(e.time_type));
		v.push_back(std::to_string(e.fog_red1));
		v.push_back(std::to_string(e.fog_green1));
		v.push_back(std::to_string(e.fog_blue1));
		v.push_back(std::to_string(e.fog_minclip1));
		v.push_back(std::to_string(e.fog_maxclip1));
		v.push_back(std::to_string(e.fog_red2));
		v.push_back(std::to_string(e.fog_green2));
		v.push_back(std::to_string(e.fog_blue2));
		v.push_back(std::to_string(e.fog_minclip2));
		v.push_back(std::to_string(e.fog_maxclip2));
		v.push_back(std::to_string(e.fog_red3));
		v.push_back(std::to_string(e.fog_green3));
		v.push_back(std::to_string(e.fog_blue3));
		v.push_back(std::to_string(e.fog_minclip3));
		v.push_back(std::to_string(e.fog_maxclip3));
		v.push_back(std::to_string(e.fog_red4));
		v.push_back(std::to_string(e.fog_green4));
		v.push_back(std::to_string(e.fog_blue4));
		v.push_back(std::to_string(e.fog_minclip4));
		v.push_back(std::to_string(e.fog_maxclip4));
		v.push_back(std::to_string(e.fog_density));
		v.push_back("'" + Strings::Escape(e.flag_needed) + "'");
		v.push_back(std::to_string(e.canbind));
		v.push_back(std::to_string(e.cancombat));
		v.push_back(std::to_string(e.canlevitate));
		v.push_back(std::to_string(e.castoutdoor));
		v.push_back(std::to_string(e.hotzone));
		v.push_back(std::to_string(e.insttype));
		v.push_back(std::to_string(e.shutdowndelay));
		v.push_back(std::to_string(e.peqzone));
		v.push_back(std::to_string(e.suspendbuffs));
		v.push_back(std::to_string(e.rain_chance1));
		v.push_back(std::to_string(e.rain_chance2));
		v.push_back(std::to_string(e.rain_chance3));
		v.push_back(std::to_string(e.rain_chance4));
		v.push_back(std::to_string(e.rain_duration1));
		v.push_back(std::to_string(e.rain_duration2));
		v.push_back(std::to_string(e.rain_duration3));
		v.push_back(std::to_string(e.rain_duration4));
		v.push_back(std::to_string(e.snow_chance1));
		v.push_back(std::to_string(e.snow_chance2));
		v.push_back(std::to_string(e.snow_chance3));
		v.push_back(std::to_string(e.snow_chance4));
		v.push_back(std::to_string(e.snow_duration1));
		v.push_back(std::to_string(e.snow_duration2));
		v.push_back(std::to_string(e.snow_duration3));
		v.push_back(std::to_string(e.snow_duration4));
		v.push_back(std::to_string(e.gravity));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.skylock));
		v.push_back(std::to_string(e.fast_regen_hp));
		v.push_back(std::to_string(e.fast_regen_mana));
		v.push_back(std::to_string(e.fast_regen_endurance));
		v.push_back(std::to_string(e.npc_max_aggro_dist));
		v.push_back(std::to_string(e.max_movement_update_range));
		v.push_back(std::to_string(e.min_expansion));
		v.push_back(std::to_string(e.max_expansion));
		v.push_back("'" + Strings::Escape(e.content_flags) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");
		v.push_back(std::to_string(e.underworld_teleport_index));
		v.push_back(std::to_string(e.lava_damage));
		v.push_back(std::to_string(e.min_lava_damage));
		v.push_back(std::to_string(e.expansion));
		v.push_back(std::to_string(e.bypass_expansion_check));

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
		const std::vector<Zone> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back("'" + Strings::Escape(e.short_name) + "'");
			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.file_name) + "'");
			v.push_back("'" + Strings::Escape(e.long_name) + "'");
			v.push_back("'" + Strings::Escape(e.map_file_name) + "'");
			v.push_back(std::to_string(e.safe_x));
			v.push_back(std::to_string(e.safe_y));
			v.push_back(std::to_string(e.safe_z));
			v.push_back(std::to_string(e.safe_heading));
			v.push_back(std::to_string(e.graveyard_id));
			v.push_back(std::to_string(e.min_level));
			v.push_back(std::to_string(e.min_status));
			v.push_back(std::to_string(e.zoneidnumber));
			v.push_back(std::to_string(e.version));
			v.push_back(std::to_string(e.timezone));
			v.push_back(std::to_string(e.maxclients));
			v.push_back(std::to_string(e.ruleset));
			v.push_back("'" + Strings::Escape(e.note) + "'");
			v.push_back(std::to_string(e.underworld));
			v.push_back(std::to_string(e.minclip));
			v.push_back(std::to_string(e.maxclip));
			v.push_back(std::to_string(e.fog_minclip));
			v.push_back(std::to_string(e.fog_maxclip));
			v.push_back(std::to_string(e.fog_blue));
			v.push_back(std::to_string(e.fog_red));
			v.push_back(std::to_string(e.fog_green));
			v.push_back(std::to_string(e.sky));
			v.push_back(std::to_string(e.ztype));
			v.push_back(std::to_string(e.zone_exp_multiplier));
			v.push_back(std::to_string(e.walkspeed));
			v.push_back(std::to_string(e.time_type));
			v.push_back(std::to_string(e.fog_red1));
			v.push_back(std::to_string(e.fog_green1));
			v.push_back(std::to_string(e.fog_blue1));
			v.push_back(std::to_string(e.fog_minclip1));
			v.push_back(std::to_string(e.fog_maxclip1));
			v.push_back(std::to_string(e.fog_red2));
			v.push_back(std::to_string(e.fog_green2));
			v.push_back(std::to_string(e.fog_blue2));
			v.push_back(std::to_string(e.fog_minclip2));
			v.push_back(std::to_string(e.fog_maxclip2));
			v.push_back(std::to_string(e.fog_red3));
			v.push_back(std::to_string(e.fog_green3));
			v.push_back(std::to_string(e.fog_blue3));
			v.push_back(std::to_string(e.fog_minclip3));
			v.push_back(std::to_string(e.fog_maxclip3));
			v.push_back(std::to_string(e.fog_red4));
			v.push_back(std::to_string(e.fog_green4));
			v.push_back(std::to_string(e.fog_blue4));
			v.push_back(std::to_string(e.fog_minclip4));
			v.push_back(std::to_string(e.fog_maxclip4));
			v.push_back(std::to_string(e.fog_density));
			v.push_back("'" + Strings::Escape(e.flag_needed) + "'");
			v.push_back(std::to_string(e.canbind));
			v.push_back(std::to_string(e.cancombat));
			v.push_back(std::to_string(e.canlevitate));
			v.push_back(std::to_string(e.castoutdoor));
			v.push_back(std::to_string(e.hotzone));
			v.push_back(std::to_string(e.insttype));
			v.push_back(std::to_string(e.shutdowndelay));
			v.push_back(std::to_string(e.peqzone));
			v.push_back(std::to_string(e.suspendbuffs));
			v.push_back(std::to_string(e.rain_chance1));
			v.push_back(std::to_string(e.rain_chance2));
			v.push_back(std::to_string(e.rain_chance3));
			v.push_back(std::to_string(e.rain_chance4));
			v.push_back(std::to_string(e.rain_duration1));
			v.push_back(std::to_string(e.rain_duration2));
			v.push_back(std::to_string(e.rain_duration3));
			v.push_back(std::to_string(e.rain_duration4));
			v.push_back(std::to_string(e.snow_chance1));
			v.push_back(std::to_string(e.snow_chance2));
			v.push_back(std::to_string(e.snow_chance3));
			v.push_back(std::to_string(e.snow_chance4));
			v.push_back(std::to_string(e.snow_duration1));
			v.push_back(std::to_string(e.snow_duration2));
			v.push_back(std::to_string(e.snow_duration3));
			v.push_back(std::to_string(e.snow_duration4));
			v.push_back(std::to_string(e.gravity));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.skylock));
			v.push_back(std::to_string(e.fast_regen_hp));
			v.push_back(std::to_string(e.fast_regen_mana));
			v.push_back(std::to_string(e.fast_regen_endurance));
			v.push_back(std::to_string(e.npc_max_aggro_dist));
			v.push_back(std::to_string(e.max_movement_update_range));
			v.push_back(std::to_string(e.min_expansion));
			v.push_back(std::to_string(e.max_expansion));
			v.push_back("'" + Strings::Escape(e.content_flags) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");
			v.push_back(std::to_string(e.underworld_teleport_index));
			v.push_back(std::to_string(e.lava_damage));
			v.push_back(std::to_string(e.min_lava_damage));
			v.push_back(std::to_string(e.expansion));
			v.push_back(std::to_string(e.bypass_expansion_check));

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

	static std::vector<Zone> All(Database& db)
	{
		std::vector<Zone> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Zone e{};

			e.short_name                = row[0] ? row[0] : "";
			e.id                        = atoi(row[1]);
			e.file_name                 = row[2] ? row[2] : "";
			e.long_name                 = row[3] ? row[3] : "";
			e.map_file_name             = row[4] ? row[4] : "";
			e.safe_x                    = static_cast<float>(atof(row[5]));
			e.safe_y                    = static_cast<float>(atof(row[6]));
			e.safe_z                    = static_cast<float>(atof(row[7]));
			e.safe_heading              = static_cast<float>(atof(row[8]));
			e.graveyard_id              = static_cast<float>(atof(row[9]));
			e.min_level                 = atoi(row[10]);
			e.min_status                = atoi(row[11]);
			e.zoneidnumber              = atoi(row[12]);
			e.version                   = atoi(row[13]);
			e.timezone                  = atoi(row[14]);
			e.maxclients                = atoi(row[15]);
			e.ruleset                   = atoi(row[16]);
			e.note                      = row[17] ? row[17] : "";
			e.underworld                = static_cast<float>(atof(row[18]));
			e.minclip                   = static_cast<float>(atof(row[19]));
			e.maxclip                   = static_cast<float>(atof(row[20]));
			e.fog_minclip               = static_cast<float>(atof(row[21]));
			e.fog_maxclip               = static_cast<float>(atof(row[22]));
			e.fog_blue                  = atoi(row[23]);
			e.fog_red                   = atoi(row[24]);
			e.fog_green                 = atoi(row[25]);
			e.sky                       = atoi(row[26]);
			e.ztype                     = atoi(row[27]);
			e.zone_exp_multiplier       = static_cast<float>(atof(row[28]));
			e.walkspeed                 = static_cast<float>(atof(row[29]));
			e.time_type                 = atoi(row[30]);
			e.fog_red1                  = atoi(row[31]);
			e.fog_green1                = atoi(row[32]);
			e.fog_blue1                 = atoi(row[33]);
			e.fog_minclip1              = static_cast<float>(atof(row[34]));
			e.fog_maxclip1              = static_cast<float>(atof(row[35]));
			e.fog_red2                  = atoi(row[36]);
			e.fog_green2                = atoi(row[37]);
			e.fog_blue2                 = atoi(row[38]);
			e.fog_minclip2              = static_cast<float>(atof(row[39]));
			e.fog_maxclip2              = static_cast<float>(atof(row[40]));
			e.fog_red3                  = atoi(row[41]);
			e.fog_green3                = atoi(row[42]);
			e.fog_blue3                 = atoi(row[43]);
			e.fog_minclip3              = static_cast<float>(atof(row[44]));
			e.fog_maxclip3              = static_cast<float>(atof(row[45]));
			e.fog_red4                  = atoi(row[46]);
			e.fog_green4                = atoi(row[47]);
			e.fog_blue4                 = atoi(row[48]);
			e.fog_minclip4              = static_cast<float>(atof(row[49]));
			e.fog_maxclip4              = static_cast<float>(atof(row[50]));
			e.fog_density               = static_cast<float>(atof(row[51]));
			e.flag_needed               = row[52] ? row[52] : "";
			e.canbind                   = atoi(row[53]);
			e.cancombat                 = atoi(row[54]);
			e.canlevitate               = atoi(row[55]);
			e.castoutdoor               = atoi(row[56]);
			e.hotzone                   = atoi(row[57]);
			e.insttype                  = atoi(row[58]);
			e.shutdowndelay             = strtoll(row[59], nullptr, 10);
			e.peqzone                   = atoi(row[60]);
			e.suspendbuffs              = atoi(row[61]);
			e.rain_chance1              = atoi(row[62]);
			e.rain_chance2              = atoi(row[63]);
			e.rain_chance3              = atoi(row[64]);
			e.rain_chance4              = atoi(row[65]);
			e.rain_duration1            = atoi(row[66]);
			e.rain_duration2            = atoi(row[67]);
			e.rain_duration3            = atoi(row[68]);
			e.rain_duration4            = atoi(row[69]);
			e.snow_chance1              = atoi(row[70]);
			e.snow_chance2              = atoi(row[71]);
			e.snow_chance3              = atoi(row[72]);
			e.snow_chance4              = atoi(row[73]);
			e.snow_duration1            = atoi(row[74]);
			e.snow_duration2            = atoi(row[75]);
			e.snow_duration3            = atoi(row[76]);
			e.snow_duration4            = atoi(row[77]);
			e.gravity                   = static_cast<float>(atof(row[78]));
			e.type                      = atoi(row[79]);
			e.skylock                   = atoi(row[80]);
			e.fast_regen_hp             = atoi(row[81]);
			e.fast_regen_mana           = atoi(row[82]);
			e.fast_regen_endurance      = atoi(row[83]);
			e.npc_max_aggro_dist        = atoi(row[84]);
			e.max_movement_update_range = atoi(row[85]);
			e.min_expansion             = atoi(row[86]);
			e.max_expansion             = atoi(row[87]);
			e.content_flags             = row[88] ? row[88] : "";
			e.content_flags_disabled    = row[89] ? row[89] : "";
			e.underworld_teleport_index = atoi(row[90]);
			e.lava_damage               = atoi(row[91]);
			e.min_lava_damage           = atoi(row[92]);
			e.expansion                 = atoi(row[93]);
			e.bypass_expansion_check    = atoi(row[94]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Zone> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Zone> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Zone e{};

			e.short_name                = row[0] ? row[0] : "";
			e.id                        = atoi(row[1]);
			e.file_name                 = row[2] ? row[2] : "";
			e.long_name                 = row[3] ? row[3] : "";
			e.map_file_name             = row[4] ? row[4] : "";
			e.safe_x                    = static_cast<float>(atof(row[5]));
			e.safe_y                    = static_cast<float>(atof(row[6]));
			e.safe_z                    = static_cast<float>(atof(row[7]));
			e.safe_heading              = static_cast<float>(atof(row[8]));
			e.graveyard_id              = static_cast<float>(atof(row[9]));
			e.min_level                 = atoi(row[10]);
			e.min_status                = atoi(row[11]);
			e.zoneidnumber              = atoi(row[12]);
			e.version                   = atoi(row[13]);
			e.timezone                  = atoi(row[14]);
			e.maxclients                = atoi(row[15]);
			e.ruleset                   = atoi(row[16]);
			e.note                      = row[17] ? row[17] : "";
			e.underworld                = static_cast<float>(atof(row[18]));
			e.minclip                   = static_cast<float>(atof(row[19]));
			e.maxclip                   = static_cast<float>(atof(row[20]));
			e.fog_minclip               = static_cast<float>(atof(row[21]));
			e.fog_maxclip               = static_cast<float>(atof(row[22]));
			e.fog_blue                  = atoi(row[23]);
			e.fog_red                   = atoi(row[24]);
			e.fog_green                 = atoi(row[25]);
			e.sky                       = atoi(row[26]);
			e.ztype                     = atoi(row[27]);
			e.zone_exp_multiplier       = static_cast<float>(atof(row[28]));
			e.walkspeed                 = static_cast<float>(atof(row[29]));
			e.time_type                 = atoi(row[30]);
			e.fog_red1                  = atoi(row[31]);
			e.fog_green1                = atoi(row[32]);
			e.fog_blue1                 = atoi(row[33]);
			e.fog_minclip1              = static_cast<float>(atof(row[34]));
			e.fog_maxclip1              = static_cast<float>(atof(row[35]));
			e.fog_red2                  = atoi(row[36]);
			e.fog_green2                = atoi(row[37]);
			e.fog_blue2                 = atoi(row[38]);
			e.fog_minclip2              = static_cast<float>(atof(row[39]));
			e.fog_maxclip2              = static_cast<float>(atof(row[40]));
			e.fog_red3                  = atoi(row[41]);
			e.fog_green3                = atoi(row[42]);
			e.fog_blue3                 = atoi(row[43]);
			e.fog_minclip3              = static_cast<float>(atof(row[44]));
			e.fog_maxclip3              = static_cast<float>(atof(row[45]));
			e.fog_red4                  = atoi(row[46]);
			e.fog_green4                = atoi(row[47]);
			e.fog_blue4                 = atoi(row[48]);
			e.fog_minclip4              = static_cast<float>(atof(row[49]));
			e.fog_maxclip4              = static_cast<float>(atof(row[50]));
			e.fog_density               = static_cast<float>(atof(row[51]));
			e.flag_needed               = row[52] ? row[52] : "";
			e.canbind                   = atoi(row[53]);
			e.cancombat                 = atoi(row[54]);
			e.canlevitate               = atoi(row[55]);
			e.castoutdoor               = atoi(row[56]);
			e.hotzone                   = atoi(row[57]);
			e.insttype                  = atoi(row[58]);
			e.shutdowndelay             = strtoll(row[59], nullptr, 10);
			e.peqzone                   = atoi(row[60]);
			e.suspendbuffs              = atoi(row[61]);
			e.rain_chance1              = atoi(row[62]);
			e.rain_chance2              = atoi(row[63]);
			e.rain_chance3              = atoi(row[64]);
			e.rain_chance4              = atoi(row[65]);
			e.rain_duration1            = atoi(row[66]);
			e.rain_duration2            = atoi(row[67]);
			e.rain_duration3            = atoi(row[68]);
			e.rain_duration4            = atoi(row[69]);
			e.snow_chance1              = atoi(row[70]);
			e.snow_chance2              = atoi(row[71]);
			e.snow_chance3              = atoi(row[72]);
			e.snow_chance4              = atoi(row[73]);
			e.snow_duration1            = atoi(row[74]);
			e.snow_duration2            = atoi(row[75]);
			e.snow_duration3            = atoi(row[76]);
			e.snow_duration4            = atoi(row[77]);
			e.gravity                   = static_cast<float>(atof(row[78]));
			e.type                      = atoi(row[79]);
			e.skylock                   = atoi(row[80]);
			e.fast_regen_hp             = atoi(row[81]);
			e.fast_regen_mana           = atoi(row[82]);
			e.fast_regen_endurance      = atoi(row[83]);
			e.npc_max_aggro_dist        = atoi(row[84]);
			e.max_movement_update_range = atoi(row[85]);
			e.min_expansion             = atoi(row[86]);
			e.max_expansion             = atoi(row[87]);
			e.content_flags             = row[88] ? row[88] : "";
			e.content_flags_disabled    = row[89] ? row[89] : "";
			e.underworld_teleport_index = atoi(row[90]);
			e.lava_damage               = atoi(row[91]);
			e.min_lava_damage           = atoi(row[92]);
			e.expansion                 = atoi(row[93]);
			e.bypass_expansion_check    = atoi(row[94]);

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

#endif //EQEMU_BASE_ZONE_REPOSITORY_H
