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
		int32_t     id;
		std::string file_name;
		std::string long_name;
		std::string map_file_name;
		float       safe_x;
		float       safe_y;
		float       safe_z;
		float       safe_heading;
		float       graveyard_id;
		uint8_t     min_level;
		uint8_t     min_status;
		int32_t     zoneidnumber;
		uint8_t     version;
		int32_t     timezone;
		int32_t     maxclients;
		uint32_t    ruleset;
		std::string note;
		float       underworld;
		float       minclip;
		float       maxclip;
		float       fog_minclip;
		float       fog_maxclip;
		uint8_t     fog_blue;
		uint8_t     fog_red;
		uint8_t     fog_green;
		uint8_t     sky;
		uint8_t     ztype;
		float       zone_exp_multiplier;
		float       walkspeed;
		uint8_t     time_type;
		uint8_t     fog_red1;
		uint8_t     fog_green1;
		uint8_t     fog_blue1;
		float       fog_minclip1;
		float       fog_maxclip1;
		uint8_t     fog_red2;
		uint8_t     fog_green2;
		uint8_t     fog_blue2;
		float       fog_minclip2;
		float       fog_maxclip2;
		uint8_t     fog_red3;
		uint8_t     fog_green3;
		uint8_t     fog_blue3;
		float       fog_minclip3;
		float       fog_maxclip3;
		uint8_t     fog_red4;
		uint8_t     fog_green4;
		uint8_t     fog_blue4;
		float       fog_minclip4;
		float       fog_maxclip4;
		float       fog_density;
		std::string flag_needed;
		int8_t      canbind;
		int8_t      cancombat;
		int8_t      canlevitate;
		int8_t      castoutdoor;
		uint8_t     hotzone;
		uint8_t     insttype;
		uint64_t    shutdowndelay;
		int8_t      peqzone;
		uint8_t     suspendbuffs;
		int32_t     rain_chance1;
		int32_t     rain_chance2;
		int32_t     rain_chance3;
		int32_t     rain_chance4;
		int32_t     rain_duration1;
		int32_t     rain_duration2;
		int32_t     rain_duration3;
		int32_t     rain_duration4;
		int32_t     snow_chance1;
		int32_t     snow_chance2;
		int32_t     snow_chance3;
		int32_t     snow_chance4;
		int32_t     snow_duration1;
		int32_t     snow_duration2;
		int32_t     snow_duration3;
		int32_t     snow_duration4;
		float       gravity;
		int32_t     type;
		int8_t      skylock;
		int32_t     fast_regen_hp;
		int32_t     fast_regen_mana;
		int32_t     fast_regen_endurance;
		int32_t     npc_max_aggro_dist;
		uint32_t    max_movement_update_range;
		int8_t      min_expansion;
		int8_t      max_expansion;
		std::string content_flags;
		std::string content_flags_disabled;
		int32_t     underworld_teleport_index;
		int32_t     lava_damage;
		int32_t     min_lava_damage;
		int8_t      expansion;
		int8_t      bypass_expansion_check;
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
			e.id                        = static_cast<int32_t>(atoi(row[1]));
			e.file_name                 = row[2] ? row[2] : "";
			e.long_name                 = row[3] ? row[3] : "";
			e.map_file_name             = row[4] ? row[4] : "";
			e.safe_x                    = strtof(row[5], nullptr);
			e.safe_y                    = strtof(row[6], nullptr);
			e.safe_z                    = strtof(row[7], nullptr);
			e.safe_heading              = strtof(row[8], nullptr);
			e.graveyard_id              = strtof(row[9], nullptr);
			e.min_level                 = static_cast<uint8_t>(strtoul(row[10], nullptr, 10));
			e.min_status                = static_cast<uint8_t>(strtoul(row[11], nullptr, 10));
			e.zoneidnumber              = static_cast<int32_t>(atoi(row[12]));
			e.version                   = static_cast<uint8_t>(strtoul(row[13], nullptr, 10));
			e.timezone                  = static_cast<int32_t>(atoi(row[14]));
			e.maxclients                = static_cast<int32_t>(atoi(row[15]));
			e.ruleset                   = static_cast<uint32_t>(strtoul(row[16], nullptr, 10));
			e.note                      = row[17] ? row[17] : "";
			e.underworld                = strtof(row[18], nullptr);
			e.minclip                   = strtof(row[19], nullptr);
			e.maxclip                   = strtof(row[20], nullptr);
			e.fog_minclip               = strtof(row[21], nullptr);
			e.fog_maxclip               = strtof(row[22], nullptr);
			e.fog_blue                  = static_cast<uint8_t>(strtoul(row[23], nullptr, 10));
			e.fog_red                   = static_cast<uint8_t>(strtoul(row[24], nullptr, 10));
			e.fog_green                 = static_cast<uint8_t>(strtoul(row[25], nullptr, 10));
			e.sky                       = static_cast<uint8_t>(strtoul(row[26], nullptr, 10));
			e.ztype                     = static_cast<uint8_t>(strtoul(row[27], nullptr, 10));
			e.zone_exp_multiplier       = strtof(row[28], nullptr);
			e.walkspeed                 = strtof(row[29], nullptr);
			e.time_type                 = static_cast<uint8_t>(strtoul(row[30], nullptr, 10));
			e.fog_red1                  = static_cast<uint8_t>(strtoul(row[31], nullptr, 10));
			e.fog_green1                = static_cast<uint8_t>(strtoul(row[32], nullptr, 10));
			e.fog_blue1                 = static_cast<uint8_t>(strtoul(row[33], nullptr, 10));
			e.fog_minclip1              = strtof(row[34], nullptr);
			e.fog_maxclip1              = strtof(row[35], nullptr);
			e.fog_red2                  = static_cast<uint8_t>(strtoul(row[36], nullptr, 10));
			e.fog_green2                = static_cast<uint8_t>(strtoul(row[37], nullptr, 10));
			e.fog_blue2                 = static_cast<uint8_t>(strtoul(row[38], nullptr, 10));
			e.fog_minclip2              = strtof(row[39], nullptr);
			e.fog_maxclip2              = strtof(row[40], nullptr);
			e.fog_red3                  = static_cast<uint8_t>(strtoul(row[41], nullptr, 10));
			e.fog_green3                = static_cast<uint8_t>(strtoul(row[42], nullptr, 10));
			e.fog_blue3                 = static_cast<uint8_t>(strtoul(row[43], nullptr, 10));
			e.fog_minclip3              = strtof(row[44], nullptr);
			e.fog_maxclip3              = strtof(row[45], nullptr);
			e.fog_red4                  = static_cast<uint8_t>(strtoul(row[46], nullptr, 10));
			e.fog_green4                = static_cast<uint8_t>(strtoul(row[47], nullptr, 10));
			e.fog_blue4                 = static_cast<uint8_t>(strtoul(row[48], nullptr, 10));
			e.fog_minclip4              = strtof(row[49], nullptr);
			e.fog_maxclip4              = strtof(row[50], nullptr);
			e.fog_density               = strtof(row[51], nullptr);
			e.flag_needed               = row[52] ? row[52] : "";
			e.canbind                   = static_cast<int8_t>(atoi(row[53]));
			e.cancombat                 = static_cast<int8_t>(atoi(row[54]));
			e.canlevitate               = static_cast<int8_t>(atoi(row[55]));
			e.castoutdoor               = static_cast<int8_t>(atoi(row[56]));
			e.hotzone                   = static_cast<uint8_t>(strtoul(row[57], nullptr, 10));
			e.insttype                  = static_cast<uint8_t>(strtoul(row[58], nullptr, 10));
			e.shutdowndelay             = strtoull(row[59], nullptr, 10);
			e.peqzone                   = static_cast<int8_t>(atoi(row[60]));
			e.suspendbuffs              = static_cast<uint8_t>(strtoul(row[61], nullptr, 10));
			e.rain_chance1              = static_cast<int32_t>(atoi(row[62]));
			e.rain_chance2              = static_cast<int32_t>(atoi(row[63]));
			e.rain_chance3              = static_cast<int32_t>(atoi(row[64]));
			e.rain_chance4              = static_cast<int32_t>(atoi(row[65]));
			e.rain_duration1            = static_cast<int32_t>(atoi(row[66]));
			e.rain_duration2            = static_cast<int32_t>(atoi(row[67]));
			e.rain_duration3            = static_cast<int32_t>(atoi(row[68]));
			e.rain_duration4            = static_cast<int32_t>(atoi(row[69]));
			e.snow_chance1              = static_cast<int32_t>(atoi(row[70]));
			e.snow_chance2              = static_cast<int32_t>(atoi(row[71]));
			e.snow_chance3              = static_cast<int32_t>(atoi(row[72]));
			e.snow_chance4              = static_cast<int32_t>(atoi(row[73]));
			e.snow_duration1            = static_cast<int32_t>(atoi(row[74]));
			e.snow_duration2            = static_cast<int32_t>(atoi(row[75]));
			e.snow_duration3            = static_cast<int32_t>(atoi(row[76]));
			e.snow_duration4            = static_cast<int32_t>(atoi(row[77]));
			e.gravity                   = strtof(row[78], nullptr);
			e.type                      = static_cast<int32_t>(atoi(row[79]));
			e.skylock                   = static_cast<int8_t>(atoi(row[80]));
			e.fast_regen_hp             = static_cast<int32_t>(atoi(row[81]));
			e.fast_regen_mana           = static_cast<int32_t>(atoi(row[82]));
			e.fast_regen_endurance      = static_cast<int32_t>(atoi(row[83]));
			e.npc_max_aggro_dist        = static_cast<int32_t>(atoi(row[84]));
			e.max_movement_update_range = static_cast<uint32_t>(strtoul(row[85], nullptr, 10));
			e.min_expansion             = static_cast<int8_t>(atoi(row[86]));
			e.max_expansion             = static_cast<int8_t>(atoi(row[87]));
			e.content_flags             = row[88] ? row[88] : "";
			e.content_flags_disabled    = row[89] ? row[89] : "";
			e.underworld_teleport_index = static_cast<int32_t>(atoi(row[90]));
			e.lava_damage               = static_cast<int32_t>(atoi(row[91]));
			e.min_lava_damage           = static_cast<int32_t>(atoi(row[92]));
			e.expansion                 = static_cast<int8_t>(atoi(row[93]));
			e.bypass_expansion_check    = static_cast<int8_t>(atoi(row[94]));

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
			e.id                        = static_cast<int32_t>(atoi(row[1]));
			e.file_name                 = row[2] ? row[2] : "";
			e.long_name                 = row[3] ? row[3] : "";
			e.map_file_name             = row[4] ? row[4] : "";
			e.safe_x                    = strtof(row[5], nullptr);
			e.safe_y                    = strtof(row[6], nullptr);
			e.safe_z                    = strtof(row[7], nullptr);
			e.safe_heading              = strtof(row[8], nullptr);
			e.graveyard_id              = strtof(row[9], nullptr);
			e.min_level                 = static_cast<uint8_t>(strtoul(row[10], nullptr, 10));
			e.min_status                = static_cast<uint8_t>(strtoul(row[11], nullptr, 10));
			e.zoneidnumber              = static_cast<int32_t>(atoi(row[12]));
			e.version                   = static_cast<uint8_t>(strtoul(row[13], nullptr, 10));
			e.timezone                  = static_cast<int32_t>(atoi(row[14]));
			e.maxclients                = static_cast<int32_t>(atoi(row[15]));
			e.ruleset                   = static_cast<uint32_t>(strtoul(row[16], nullptr, 10));
			e.note                      = row[17] ? row[17] : "";
			e.underworld                = strtof(row[18], nullptr);
			e.minclip                   = strtof(row[19], nullptr);
			e.maxclip                   = strtof(row[20], nullptr);
			e.fog_minclip               = strtof(row[21], nullptr);
			e.fog_maxclip               = strtof(row[22], nullptr);
			e.fog_blue                  = static_cast<uint8_t>(strtoul(row[23], nullptr, 10));
			e.fog_red                   = static_cast<uint8_t>(strtoul(row[24], nullptr, 10));
			e.fog_green                 = static_cast<uint8_t>(strtoul(row[25], nullptr, 10));
			e.sky                       = static_cast<uint8_t>(strtoul(row[26], nullptr, 10));
			e.ztype                     = static_cast<uint8_t>(strtoul(row[27], nullptr, 10));
			e.zone_exp_multiplier       = strtof(row[28], nullptr);
			e.walkspeed                 = strtof(row[29], nullptr);
			e.time_type                 = static_cast<uint8_t>(strtoul(row[30], nullptr, 10));
			e.fog_red1                  = static_cast<uint8_t>(strtoul(row[31], nullptr, 10));
			e.fog_green1                = static_cast<uint8_t>(strtoul(row[32], nullptr, 10));
			e.fog_blue1                 = static_cast<uint8_t>(strtoul(row[33], nullptr, 10));
			e.fog_minclip1              = strtof(row[34], nullptr);
			e.fog_maxclip1              = strtof(row[35], nullptr);
			e.fog_red2                  = static_cast<uint8_t>(strtoul(row[36], nullptr, 10));
			e.fog_green2                = static_cast<uint8_t>(strtoul(row[37], nullptr, 10));
			e.fog_blue2                 = static_cast<uint8_t>(strtoul(row[38], nullptr, 10));
			e.fog_minclip2              = strtof(row[39], nullptr);
			e.fog_maxclip2              = strtof(row[40], nullptr);
			e.fog_red3                  = static_cast<uint8_t>(strtoul(row[41], nullptr, 10));
			e.fog_green3                = static_cast<uint8_t>(strtoul(row[42], nullptr, 10));
			e.fog_blue3                 = static_cast<uint8_t>(strtoul(row[43], nullptr, 10));
			e.fog_minclip3              = strtof(row[44], nullptr);
			e.fog_maxclip3              = strtof(row[45], nullptr);
			e.fog_red4                  = static_cast<uint8_t>(strtoul(row[46], nullptr, 10));
			e.fog_green4                = static_cast<uint8_t>(strtoul(row[47], nullptr, 10));
			e.fog_blue4                 = static_cast<uint8_t>(strtoul(row[48], nullptr, 10));
			e.fog_minclip4              = strtof(row[49], nullptr);
			e.fog_maxclip4              = strtof(row[50], nullptr);
			e.fog_density               = strtof(row[51], nullptr);
			e.flag_needed               = row[52] ? row[52] : "";
			e.canbind                   = static_cast<int8_t>(atoi(row[53]));
			e.cancombat                 = static_cast<int8_t>(atoi(row[54]));
			e.canlevitate               = static_cast<int8_t>(atoi(row[55]));
			e.castoutdoor               = static_cast<int8_t>(atoi(row[56]));
			e.hotzone                   = static_cast<uint8_t>(strtoul(row[57], nullptr, 10));
			e.insttype                  = static_cast<uint8_t>(strtoul(row[58], nullptr, 10));
			e.shutdowndelay             = strtoull(row[59], nullptr, 10);
			e.peqzone                   = static_cast<int8_t>(atoi(row[60]));
			e.suspendbuffs              = static_cast<uint8_t>(strtoul(row[61], nullptr, 10));
			e.rain_chance1              = static_cast<int32_t>(atoi(row[62]));
			e.rain_chance2              = static_cast<int32_t>(atoi(row[63]));
			e.rain_chance3              = static_cast<int32_t>(atoi(row[64]));
			e.rain_chance4              = static_cast<int32_t>(atoi(row[65]));
			e.rain_duration1            = static_cast<int32_t>(atoi(row[66]));
			e.rain_duration2            = static_cast<int32_t>(atoi(row[67]));
			e.rain_duration3            = static_cast<int32_t>(atoi(row[68]));
			e.rain_duration4            = static_cast<int32_t>(atoi(row[69]));
			e.snow_chance1              = static_cast<int32_t>(atoi(row[70]));
			e.snow_chance2              = static_cast<int32_t>(atoi(row[71]));
			e.snow_chance3              = static_cast<int32_t>(atoi(row[72]));
			e.snow_chance4              = static_cast<int32_t>(atoi(row[73]));
			e.snow_duration1            = static_cast<int32_t>(atoi(row[74]));
			e.snow_duration2            = static_cast<int32_t>(atoi(row[75]));
			e.snow_duration3            = static_cast<int32_t>(atoi(row[76]));
			e.snow_duration4            = static_cast<int32_t>(atoi(row[77]));
			e.gravity                   = strtof(row[78], nullptr);
			e.type                      = static_cast<int32_t>(atoi(row[79]));
			e.skylock                   = static_cast<int8_t>(atoi(row[80]));
			e.fast_regen_hp             = static_cast<int32_t>(atoi(row[81]));
			e.fast_regen_mana           = static_cast<int32_t>(atoi(row[82]));
			e.fast_regen_endurance      = static_cast<int32_t>(atoi(row[83]));
			e.npc_max_aggro_dist        = static_cast<int32_t>(atoi(row[84]));
			e.max_movement_update_range = static_cast<uint32_t>(strtoul(row[85], nullptr, 10));
			e.min_expansion             = static_cast<int8_t>(atoi(row[86]));
			e.max_expansion             = static_cast<int8_t>(atoi(row[87]));
			e.content_flags             = row[88] ? row[88] : "";
			e.content_flags_disabled    = row[89] ? row[89] : "";
			e.underworld_teleport_index = static_cast<int32_t>(atoi(row[90]));
			e.lava_damage               = static_cast<int32_t>(atoi(row[91]));
			e.min_lava_damage           = static_cast<int32_t>(atoi(row[92]));
			e.expansion                 = static_cast<int8_t>(atoi(row[93]));
			e.bypass_expansion_check    = static_cast<int8_t>(atoi(row[94]));

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
			e.id                        = static_cast<int32_t>(atoi(row[1]));
			e.file_name                 = row[2] ? row[2] : "";
			e.long_name                 = row[3] ? row[3] : "";
			e.map_file_name             = row[4] ? row[4] : "";
			e.safe_x                    = strtof(row[5], nullptr);
			e.safe_y                    = strtof(row[6], nullptr);
			e.safe_z                    = strtof(row[7], nullptr);
			e.safe_heading              = strtof(row[8], nullptr);
			e.graveyard_id              = strtof(row[9], nullptr);
			e.min_level                 = static_cast<uint8_t>(strtoul(row[10], nullptr, 10));
			e.min_status                = static_cast<uint8_t>(strtoul(row[11], nullptr, 10));
			e.zoneidnumber              = static_cast<int32_t>(atoi(row[12]));
			e.version                   = static_cast<uint8_t>(strtoul(row[13], nullptr, 10));
			e.timezone                  = static_cast<int32_t>(atoi(row[14]));
			e.maxclients                = static_cast<int32_t>(atoi(row[15]));
			e.ruleset                   = static_cast<uint32_t>(strtoul(row[16], nullptr, 10));
			e.note                      = row[17] ? row[17] : "";
			e.underworld                = strtof(row[18], nullptr);
			e.minclip                   = strtof(row[19], nullptr);
			e.maxclip                   = strtof(row[20], nullptr);
			e.fog_minclip               = strtof(row[21], nullptr);
			e.fog_maxclip               = strtof(row[22], nullptr);
			e.fog_blue                  = static_cast<uint8_t>(strtoul(row[23], nullptr, 10));
			e.fog_red                   = static_cast<uint8_t>(strtoul(row[24], nullptr, 10));
			e.fog_green                 = static_cast<uint8_t>(strtoul(row[25], nullptr, 10));
			e.sky                       = static_cast<uint8_t>(strtoul(row[26], nullptr, 10));
			e.ztype                     = static_cast<uint8_t>(strtoul(row[27], nullptr, 10));
			e.zone_exp_multiplier       = strtof(row[28], nullptr);
			e.walkspeed                 = strtof(row[29], nullptr);
			e.time_type                 = static_cast<uint8_t>(strtoul(row[30], nullptr, 10));
			e.fog_red1                  = static_cast<uint8_t>(strtoul(row[31], nullptr, 10));
			e.fog_green1                = static_cast<uint8_t>(strtoul(row[32], nullptr, 10));
			e.fog_blue1                 = static_cast<uint8_t>(strtoul(row[33], nullptr, 10));
			e.fog_minclip1              = strtof(row[34], nullptr);
			e.fog_maxclip1              = strtof(row[35], nullptr);
			e.fog_red2                  = static_cast<uint8_t>(strtoul(row[36], nullptr, 10));
			e.fog_green2                = static_cast<uint8_t>(strtoul(row[37], nullptr, 10));
			e.fog_blue2                 = static_cast<uint8_t>(strtoul(row[38], nullptr, 10));
			e.fog_minclip2              = strtof(row[39], nullptr);
			e.fog_maxclip2              = strtof(row[40], nullptr);
			e.fog_red3                  = static_cast<uint8_t>(strtoul(row[41], nullptr, 10));
			e.fog_green3                = static_cast<uint8_t>(strtoul(row[42], nullptr, 10));
			e.fog_blue3                 = static_cast<uint8_t>(strtoul(row[43], nullptr, 10));
			e.fog_minclip3              = strtof(row[44], nullptr);
			e.fog_maxclip3              = strtof(row[45], nullptr);
			e.fog_red4                  = static_cast<uint8_t>(strtoul(row[46], nullptr, 10));
			e.fog_green4                = static_cast<uint8_t>(strtoul(row[47], nullptr, 10));
			e.fog_blue4                 = static_cast<uint8_t>(strtoul(row[48], nullptr, 10));
			e.fog_minclip4              = strtof(row[49], nullptr);
			e.fog_maxclip4              = strtof(row[50], nullptr);
			e.fog_density               = strtof(row[51], nullptr);
			e.flag_needed               = row[52] ? row[52] : "";
			e.canbind                   = static_cast<int8_t>(atoi(row[53]));
			e.cancombat                 = static_cast<int8_t>(atoi(row[54]));
			e.canlevitate               = static_cast<int8_t>(atoi(row[55]));
			e.castoutdoor               = static_cast<int8_t>(atoi(row[56]));
			e.hotzone                   = static_cast<uint8_t>(strtoul(row[57], nullptr, 10));
			e.insttype                  = static_cast<uint8_t>(strtoul(row[58], nullptr, 10));
			e.shutdowndelay             = strtoull(row[59], nullptr, 10);
			e.peqzone                   = static_cast<int8_t>(atoi(row[60]));
			e.suspendbuffs              = static_cast<uint8_t>(strtoul(row[61], nullptr, 10));
			e.rain_chance1              = static_cast<int32_t>(atoi(row[62]));
			e.rain_chance2              = static_cast<int32_t>(atoi(row[63]));
			e.rain_chance3              = static_cast<int32_t>(atoi(row[64]));
			e.rain_chance4              = static_cast<int32_t>(atoi(row[65]));
			e.rain_duration1            = static_cast<int32_t>(atoi(row[66]));
			e.rain_duration2            = static_cast<int32_t>(atoi(row[67]));
			e.rain_duration3            = static_cast<int32_t>(atoi(row[68]));
			e.rain_duration4            = static_cast<int32_t>(atoi(row[69]));
			e.snow_chance1              = static_cast<int32_t>(atoi(row[70]));
			e.snow_chance2              = static_cast<int32_t>(atoi(row[71]));
			e.snow_chance3              = static_cast<int32_t>(atoi(row[72]));
			e.snow_chance4              = static_cast<int32_t>(atoi(row[73]));
			e.snow_duration1            = static_cast<int32_t>(atoi(row[74]));
			e.snow_duration2            = static_cast<int32_t>(atoi(row[75]));
			e.snow_duration3            = static_cast<int32_t>(atoi(row[76]));
			e.snow_duration4            = static_cast<int32_t>(atoi(row[77]));
			e.gravity                   = strtof(row[78], nullptr);
			e.type                      = static_cast<int32_t>(atoi(row[79]));
			e.skylock                   = static_cast<int8_t>(atoi(row[80]));
			e.fast_regen_hp             = static_cast<int32_t>(atoi(row[81]));
			e.fast_regen_mana           = static_cast<int32_t>(atoi(row[82]));
			e.fast_regen_endurance      = static_cast<int32_t>(atoi(row[83]));
			e.npc_max_aggro_dist        = static_cast<int32_t>(atoi(row[84]));
			e.max_movement_update_range = static_cast<uint32_t>(strtoul(row[85], nullptr, 10));
			e.min_expansion             = static_cast<int8_t>(atoi(row[86]));
			e.max_expansion             = static_cast<int8_t>(atoi(row[87]));
			e.content_flags             = row[88] ? row[88] : "";
			e.content_flags_disabled    = row[89] ? row[89] : "";
			e.underworld_teleport_index = static_cast<int32_t>(atoi(row[90]));
			e.lava_damage               = static_cast<int32_t>(atoi(row[91]));
			e.min_lava_damage           = static_cast<int32_t>(atoi(row[92]));
			e.expansion                 = static_cast<int8_t>(atoi(row[93]));
			e.bypass_expansion_check    = static_cast<int8_t>(atoi(row[94]));

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
