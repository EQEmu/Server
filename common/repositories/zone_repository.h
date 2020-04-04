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
 */

#ifndef EQEMU_ZONE_REPOSITORY_H
#define EQEMU_ZONE_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class ZoneRepository {
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
		float       graveyard_id;
		int8        min_level;
		int8        min_status;
		int         zoneidnumber;
		int8        version;
		int         timezone;
		int         maxclients;
		int         ruleset;
		std::string note;
		float       underworld;
		float       minclip;
		float       maxclip;
		float       fog_minclip;
		float       fog_maxclip;
		int8        fog_blue;
		int8        fog_red;
		int8        fog_green;
		int8        sky;
		int8        ztype;
		float       zone_exp_multiplier;
		float       walkspeed;
		int8        time_type;
		int8        fog_red1;
		int8        fog_green1;
		int8        fog_blue1;
		float       fog_minclip1;
		float       fog_maxclip1;
		int8        fog_red2;
		int8        fog_green2;
		int8        fog_blue2;
		float       fog_minclip2;
		float       fog_maxclip2;
		int8        fog_red3;
		int8        fog_green3;
		int8        fog_blue3;
		float       fog_minclip3;
		float       fog_maxclip3;
		int8        fog_red4;
		int8        fog_green4;
		int8        fog_blue4;
		float       fog_minclip4;
		float       fog_maxclip4;
		float       fog_density;
		std::string flag_needed;
		int8        canbind;
		int8        cancombat;
		int8        canlevitate;
		int8        castoutdoor;
		int8        hotzone;
		int8        insttype;
		int         shutdowndelay;
		int8        peqzone;
		int8        expansion;
		int8        suspendbuffs;
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
		int8        skylock;
		int         fast_regen_hp;
		int         fast_regen_mana;
		int         fast_regen_endurance;
		int         npc_max_aggro_dist;
		int         max_movement_update_range;
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
			"expansion",
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
		return std::string("zone");
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

	static Zone NewEntity()
	{
		Zone entry{};

		entry.short_name                = "";
		entry.id                        = 0;
		entry.file_name                 = "";
		entry.long_name                 = "";
		entry.map_file_name             = "";
		entry.safe_x                    = 0;
		entry.safe_y                    = 0;
		entry.safe_z                    = 0;
		entry.graveyard_id              = 0;
		entry.min_level                 = 0;
		entry.min_status                = 0;
		entry.zoneidnumber              = 0;
		entry.version                   = 0;
		entry.timezone                  = 0;
		entry.maxclients                = 0;
		entry.ruleset                   = 0;
		entry.note                      = "";
		entry.underworld                = 0;
		entry.minclip                   = 450;
		entry.maxclip                   = 450;
		entry.fog_minclip               = 450;
		entry.fog_maxclip               = 450;
		entry.fog_blue                  = 0;
		entry.fog_red                   = 0;
		entry.fog_green                 = 0;
		entry.sky                       = 1;
		entry.ztype                     = 1;
		entry.zone_exp_multiplier       = 0.00;
		entry.walkspeed                 = 0.4;
		entry.time_type                 = 2;
		entry.fog_red1                  = 0;
		entry.fog_green1                = 0;
		entry.fog_blue1                 = 0;
		entry.fog_minclip1              = 450;
		entry.fog_maxclip1              = 450;
		entry.fog_red2                  = 0;
		entry.fog_green2                = 0;
		entry.fog_blue2                 = 0;
		entry.fog_minclip2              = 450;
		entry.fog_maxclip2              = 450;
		entry.fog_red3                  = 0;
		entry.fog_green3                = 0;
		entry.fog_blue3                 = 0;
		entry.fog_minclip3              = 450;
		entry.fog_maxclip3              = 450;
		entry.fog_red4                  = 0;
		entry.fog_green4                = 0;
		entry.fog_blue4                 = 0;
		entry.fog_minclip4              = 450;
		entry.fog_maxclip4              = 450;
		entry.fog_density               = 0;
		entry.flag_needed               = "";
		entry.canbind                   = 1;
		entry.cancombat                 = 1;
		entry.canlevitate               = 1;
		entry.castoutdoor               = 1;
		entry.hotzone                   = 0;
		entry.insttype                  = 0;
		entry.shutdowndelay             = 5000;
		entry.peqzone                   = 1;
		entry.expansion                 = 0;
		entry.suspendbuffs              = 0;
		entry.rain_chance1              = 0;
		entry.rain_chance2              = 0;
		entry.rain_chance3              = 0;
		entry.rain_chance4              = 0;
		entry.rain_duration1            = 0;
		entry.rain_duration2            = 0;
		entry.rain_duration3            = 0;
		entry.rain_duration4            = 0;
		entry.snow_chance1              = 0;
		entry.snow_chance2              = 0;
		entry.snow_chance3              = 0;
		entry.snow_chance4              = 0;
		entry.snow_duration1            = 0;
		entry.snow_duration2            = 0;
		entry.snow_duration3            = 0;
		entry.snow_duration4            = 0;
		entry.gravity                   = 0.4;
		entry.type                      = 0;
		entry.skylock                   = 0;
		entry.fast_regen_hp             = 180;
		entry.fast_regen_mana           = 180;
		entry.fast_regen_endurance      = 180;
		entry.npc_max_aggro_dist        = 600;
		entry.max_movement_update_range = 600;

		return entry;
	}

	static Zone GetZoneEntry(
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
		int zone_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				zone_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Zone entry{};

			entry.short_name                = row[0];
			entry.id                        = atoi(row[1]);
			entry.file_name                 = row[2];
			entry.long_name                 = row[3];
			entry.map_file_name             = row[4];
			entry.safe_x                    = atof(row[5]);
			entry.safe_y                    = atof(row[6]);
			entry.safe_z                    = atof(row[7]);
			entry.graveyard_id              = atof(row[8]);
			entry.min_level                 = atoi(row[9]);
			entry.min_status                = atoi(row[10]);
			entry.zoneidnumber              = atoi(row[11]);
			entry.version                   = atoi(row[12]);
			entry.timezone                  = atoi(row[13]);
			entry.maxclients                = atoi(row[14]);
			entry.ruleset                   = atoi(row[15]);
			entry.note                      = row[16];
			entry.underworld                = atof(row[17]);
			entry.minclip                   = atof(row[18]);
			entry.maxclip                   = atof(row[19]);
			entry.fog_minclip               = atof(row[20]);
			entry.fog_maxclip               = atof(row[21]);
			entry.fog_blue                  = atoi(row[22]);
			entry.fog_red                   = atoi(row[23]);
			entry.fog_green                 = atoi(row[24]);
			entry.sky                       = atoi(row[25]);
			entry.ztype                     = atoi(row[26]);
			entry.zone_exp_multiplier       = atof(row[27]);
			entry.walkspeed                 = atof(row[28]);
			entry.time_type                 = atoi(row[29]);
			entry.fog_red1                  = atoi(row[30]);
			entry.fog_green1                = atoi(row[31]);
			entry.fog_blue1                 = atoi(row[32]);
			entry.fog_minclip1              = atof(row[33]);
			entry.fog_maxclip1              = atof(row[34]);
			entry.fog_red2                  = atoi(row[35]);
			entry.fog_green2                = atoi(row[36]);
			entry.fog_blue2                 = atoi(row[37]);
			entry.fog_minclip2              = atof(row[38]);
			entry.fog_maxclip2              = atof(row[39]);
			entry.fog_red3                  = atoi(row[40]);
			entry.fog_green3                = atoi(row[41]);
			entry.fog_blue3                 = atoi(row[42]);
			entry.fog_minclip3              = atof(row[43]);
			entry.fog_maxclip3              = atof(row[44]);
			entry.fog_red4                  = atoi(row[45]);
			entry.fog_green4                = atoi(row[46]);
			entry.fog_blue4                 = atoi(row[47]);
			entry.fog_minclip4              = atof(row[48]);
			entry.fog_maxclip4              = atof(row[49]);
			entry.fog_density               = atof(row[50]);
			entry.flag_needed               = row[51];
			entry.canbind                   = atoi(row[52]);
			entry.cancombat                 = atoi(row[53]);
			entry.canlevitate               = atoi(row[54]);
			entry.castoutdoor               = atoi(row[55]);
			entry.hotzone                   = atoi(row[56]);
			entry.insttype                  = atoi(row[57]);
			entry.shutdowndelay             = atoi(row[58]);
			entry.peqzone                   = atoi(row[59]);
			entry.expansion                 = atoi(row[60]);
			entry.suspendbuffs              = atoi(row[61]);
			entry.rain_chance1              = atoi(row[62]);
			entry.rain_chance2              = atoi(row[63]);
			entry.rain_chance3              = atoi(row[64]);
			entry.rain_chance4              = atoi(row[65]);
			entry.rain_duration1            = atoi(row[66]);
			entry.rain_duration2            = atoi(row[67]);
			entry.rain_duration3            = atoi(row[68]);
			entry.rain_duration4            = atoi(row[69]);
			entry.snow_chance1              = atoi(row[70]);
			entry.snow_chance2              = atoi(row[71]);
			entry.snow_chance3              = atoi(row[72]);
			entry.snow_chance4              = atoi(row[73]);
			entry.snow_duration1            = atoi(row[74]);
			entry.snow_duration2            = atoi(row[75]);
			entry.snow_duration3            = atoi(row[76]);
			entry.snow_duration4            = atoi(row[77]);
			entry.gravity                   = atof(row[78]);
			entry.type                      = atoi(row[79]);
			entry.skylock                   = atoi(row[80]);
			entry.fast_regen_hp             = atoi(row[81]);
			entry.fast_regen_mana           = atoi(row[82]);
			entry.fast_regen_endurance      = atoi(row[83]);
			entry.npc_max_aggro_dist        = atoi(row[84]);
			entry.max_movement_update_range = atoi(row[85]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int zone_id
	)
	{
		auto results = content_db.QueryDatabase(
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
		Zone zone_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = '" + EscapeString(zone_entry.short_name) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(zone_entry.file_name) + "'");
		update_values.push_back(columns[3] + " = '" + EscapeString(zone_entry.long_name) + "'");
		update_values.push_back(columns[4] + " = '" + EscapeString(zone_entry.map_file_name) + "'");
		update_values.push_back(columns[5] + " = " + std::to_string(zone_entry.safe_x));
		update_values.push_back(columns[6] + " = " + std::to_string(zone_entry.safe_y));
		update_values.push_back(columns[7] + " = " + std::to_string(zone_entry.safe_z));
		update_values.push_back(columns[8] + " = " + std::to_string(zone_entry.graveyard_id));
		update_values.push_back(columns[9] + " = " + std::to_string(zone_entry.min_level));
		update_values.push_back(columns[10] + " = " + std::to_string(zone_entry.min_status));
		update_values.push_back(columns[11] + " = " + std::to_string(zone_entry.zoneidnumber));
		update_values.push_back(columns[12] + " = " + std::to_string(zone_entry.version));
		update_values.push_back(columns[13] + " = " + std::to_string(zone_entry.timezone));
		update_values.push_back(columns[14] + " = " + std::to_string(zone_entry.maxclients));
		update_values.push_back(columns[15] + " = " + std::to_string(zone_entry.ruleset));
		update_values.push_back(columns[16] + " = '" + EscapeString(zone_entry.note) + "'");
		update_values.push_back(columns[17] + " = " + std::to_string(zone_entry.underworld));
		update_values.push_back(columns[18] + " = " + std::to_string(zone_entry.minclip));
		update_values.push_back(columns[19] + " = " + std::to_string(zone_entry.maxclip));
		update_values.push_back(columns[20] + " = " + std::to_string(zone_entry.fog_minclip));
		update_values.push_back(columns[21] + " = " + std::to_string(zone_entry.fog_maxclip));
		update_values.push_back(columns[22] + " = " + std::to_string(zone_entry.fog_blue));
		update_values.push_back(columns[23] + " = " + std::to_string(zone_entry.fog_red));
		update_values.push_back(columns[24] + " = " + std::to_string(zone_entry.fog_green));
		update_values.push_back(columns[25] + " = " + std::to_string(zone_entry.sky));
		update_values.push_back(columns[26] + " = " + std::to_string(zone_entry.ztype));
		update_values.push_back(columns[27] + " = " + std::to_string(zone_entry.zone_exp_multiplier));
		update_values.push_back(columns[28] + " = " + std::to_string(zone_entry.walkspeed));
		update_values.push_back(columns[29] + " = " + std::to_string(zone_entry.time_type));
		update_values.push_back(columns[30] + " = " + std::to_string(zone_entry.fog_red1));
		update_values.push_back(columns[31] + " = " + std::to_string(zone_entry.fog_green1));
		update_values.push_back(columns[32] + " = " + std::to_string(zone_entry.fog_blue1));
		update_values.push_back(columns[33] + " = " + std::to_string(zone_entry.fog_minclip1));
		update_values.push_back(columns[34] + " = " + std::to_string(zone_entry.fog_maxclip1));
		update_values.push_back(columns[35] + " = " + std::to_string(zone_entry.fog_red2));
		update_values.push_back(columns[36] + " = " + std::to_string(zone_entry.fog_green2));
		update_values.push_back(columns[37] + " = " + std::to_string(zone_entry.fog_blue2));
		update_values.push_back(columns[38] + " = " + std::to_string(zone_entry.fog_minclip2));
		update_values.push_back(columns[39] + " = " + std::to_string(zone_entry.fog_maxclip2));
		update_values.push_back(columns[40] + " = " + std::to_string(zone_entry.fog_red3));
		update_values.push_back(columns[41] + " = " + std::to_string(zone_entry.fog_green3));
		update_values.push_back(columns[42] + " = " + std::to_string(zone_entry.fog_blue3));
		update_values.push_back(columns[43] + " = " + std::to_string(zone_entry.fog_minclip3));
		update_values.push_back(columns[44] + " = " + std::to_string(zone_entry.fog_maxclip3));
		update_values.push_back(columns[45] + " = " + std::to_string(zone_entry.fog_red4));
		update_values.push_back(columns[46] + " = " + std::to_string(zone_entry.fog_green4));
		update_values.push_back(columns[47] + " = " + std::to_string(zone_entry.fog_blue4));
		update_values.push_back(columns[48] + " = " + std::to_string(zone_entry.fog_minclip4));
		update_values.push_back(columns[49] + " = " + std::to_string(zone_entry.fog_maxclip4));
		update_values.push_back(columns[50] + " = " + std::to_string(zone_entry.fog_density));
		update_values.push_back(columns[51] + " = '" + EscapeString(zone_entry.flag_needed) + "'");
		update_values.push_back(columns[52] + " = " + std::to_string(zone_entry.canbind));
		update_values.push_back(columns[53] + " = " + std::to_string(zone_entry.cancombat));
		update_values.push_back(columns[54] + " = " + std::to_string(zone_entry.canlevitate));
		update_values.push_back(columns[55] + " = " + std::to_string(zone_entry.castoutdoor));
		update_values.push_back(columns[56] + " = " + std::to_string(zone_entry.hotzone));
		update_values.push_back(columns[57] + " = " + std::to_string(zone_entry.insttype));
		update_values.push_back(columns[58] + " = " + std::to_string(zone_entry.shutdowndelay));
		update_values.push_back(columns[59] + " = " + std::to_string(zone_entry.peqzone));
		update_values.push_back(columns[60] + " = " + std::to_string(zone_entry.expansion));
		update_values.push_back(columns[61] + " = " + std::to_string(zone_entry.suspendbuffs));
		update_values.push_back(columns[62] + " = " + std::to_string(zone_entry.rain_chance1));
		update_values.push_back(columns[63] + " = " + std::to_string(zone_entry.rain_chance2));
		update_values.push_back(columns[64] + " = " + std::to_string(zone_entry.rain_chance3));
		update_values.push_back(columns[65] + " = " + std::to_string(zone_entry.rain_chance4));
		update_values.push_back(columns[66] + " = " + std::to_string(zone_entry.rain_duration1));
		update_values.push_back(columns[67] + " = " + std::to_string(zone_entry.rain_duration2));
		update_values.push_back(columns[68] + " = " + std::to_string(zone_entry.rain_duration3));
		update_values.push_back(columns[69] + " = " + std::to_string(zone_entry.rain_duration4));
		update_values.push_back(columns[70] + " = " + std::to_string(zone_entry.snow_chance1));
		update_values.push_back(columns[71] + " = " + std::to_string(zone_entry.snow_chance2));
		update_values.push_back(columns[72] + " = " + std::to_string(zone_entry.snow_chance3));
		update_values.push_back(columns[73] + " = " + std::to_string(zone_entry.snow_chance4));
		update_values.push_back(columns[74] + " = " + std::to_string(zone_entry.snow_duration1));
		update_values.push_back(columns[75] + " = " + std::to_string(zone_entry.snow_duration2));
		update_values.push_back(columns[76] + " = " + std::to_string(zone_entry.snow_duration3));
		update_values.push_back(columns[77] + " = " + std::to_string(zone_entry.snow_duration4));
		update_values.push_back(columns[78] + " = " + std::to_string(zone_entry.gravity));
		update_values.push_back(columns[79] + " = " + std::to_string(zone_entry.type));
		update_values.push_back(columns[80] + " = " + std::to_string(zone_entry.skylock));
		update_values.push_back(columns[81] + " = " + std::to_string(zone_entry.fast_regen_hp));
		update_values.push_back(columns[82] + " = " + std::to_string(zone_entry.fast_regen_mana));
		update_values.push_back(columns[83] + " = " + std::to_string(zone_entry.fast_regen_endurance));
		update_values.push_back(columns[84] + " = " + std::to_string(zone_entry.npc_max_aggro_dist));
		update_values.push_back(columns[85] + " = " + std::to_string(zone_entry.max_movement_update_range));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				zone_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Zone InsertOne(
		Zone zone_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(zone_entry.short_name) + "'");
		insert_values.push_back("'" + EscapeString(zone_entry.file_name) + "'");
		insert_values.push_back("'" + EscapeString(zone_entry.long_name) + "'");
		insert_values.push_back("'" + EscapeString(zone_entry.map_file_name) + "'");
		insert_values.push_back(std::to_string(zone_entry.safe_x));
		insert_values.push_back(std::to_string(zone_entry.safe_y));
		insert_values.push_back(std::to_string(zone_entry.safe_z));
		insert_values.push_back(std::to_string(zone_entry.graveyard_id));
		insert_values.push_back(std::to_string(zone_entry.min_level));
		insert_values.push_back(std::to_string(zone_entry.min_status));
		insert_values.push_back(std::to_string(zone_entry.zoneidnumber));
		insert_values.push_back(std::to_string(zone_entry.version));
		insert_values.push_back(std::to_string(zone_entry.timezone));
		insert_values.push_back(std::to_string(zone_entry.maxclients));
		insert_values.push_back(std::to_string(zone_entry.ruleset));
		insert_values.push_back("'" + EscapeString(zone_entry.note) + "'");
		insert_values.push_back(std::to_string(zone_entry.underworld));
		insert_values.push_back(std::to_string(zone_entry.minclip));
		insert_values.push_back(std::to_string(zone_entry.maxclip));
		insert_values.push_back(std::to_string(zone_entry.fog_minclip));
		insert_values.push_back(std::to_string(zone_entry.fog_maxclip));
		insert_values.push_back(std::to_string(zone_entry.fog_blue));
		insert_values.push_back(std::to_string(zone_entry.fog_red));
		insert_values.push_back(std::to_string(zone_entry.fog_green));
		insert_values.push_back(std::to_string(zone_entry.sky));
		insert_values.push_back(std::to_string(zone_entry.ztype));
		insert_values.push_back(std::to_string(zone_entry.zone_exp_multiplier));
		insert_values.push_back(std::to_string(zone_entry.walkspeed));
		insert_values.push_back(std::to_string(zone_entry.time_type));
		insert_values.push_back(std::to_string(zone_entry.fog_red1));
		insert_values.push_back(std::to_string(zone_entry.fog_green1));
		insert_values.push_back(std::to_string(zone_entry.fog_blue1));
		insert_values.push_back(std::to_string(zone_entry.fog_minclip1));
		insert_values.push_back(std::to_string(zone_entry.fog_maxclip1));
		insert_values.push_back(std::to_string(zone_entry.fog_red2));
		insert_values.push_back(std::to_string(zone_entry.fog_green2));
		insert_values.push_back(std::to_string(zone_entry.fog_blue2));
		insert_values.push_back(std::to_string(zone_entry.fog_minclip2));
		insert_values.push_back(std::to_string(zone_entry.fog_maxclip2));
		insert_values.push_back(std::to_string(zone_entry.fog_red3));
		insert_values.push_back(std::to_string(zone_entry.fog_green3));
		insert_values.push_back(std::to_string(zone_entry.fog_blue3));
		insert_values.push_back(std::to_string(zone_entry.fog_minclip3));
		insert_values.push_back(std::to_string(zone_entry.fog_maxclip3));
		insert_values.push_back(std::to_string(zone_entry.fog_red4));
		insert_values.push_back(std::to_string(zone_entry.fog_green4));
		insert_values.push_back(std::to_string(zone_entry.fog_blue4));
		insert_values.push_back(std::to_string(zone_entry.fog_minclip4));
		insert_values.push_back(std::to_string(zone_entry.fog_maxclip4));
		insert_values.push_back(std::to_string(zone_entry.fog_density));
		insert_values.push_back("'" + EscapeString(zone_entry.flag_needed) + "'");
		insert_values.push_back(std::to_string(zone_entry.canbind));
		insert_values.push_back(std::to_string(zone_entry.cancombat));
		insert_values.push_back(std::to_string(zone_entry.canlevitate));
		insert_values.push_back(std::to_string(zone_entry.castoutdoor));
		insert_values.push_back(std::to_string(zone_entry.hotzone));
		insert_values.push_back(std::to_string(zone_entry.insttype));
		insert_values.push_back(std::to_string(zone_entry.shutdowndelay));
		insert_values.push_back(std::to_string(zone_entry.peqzone));
		insert_values.push_back(std::to_string(zone_entry.expansion));
		insert_values.push_back(std::to_string(zone_entry.suspendbuffs));
		insert_values.push_back(std::to_string(zone_entry.rain_chance1));
		insert_values.push_back(std::to_string(zone_entry.rain_chance2));
		insert_values.push_back(std::to_string(zone_entry.rain_chance3));
		insert_values.push_back(std::to_string(zone_entry.rain_chance4));
		insert_values.push_back(std::to_string(zone_entry.rain_duration1));
		insert_values.push_back(std::to_string(zone_entry.rain_duration2));
		insert_values.push_back(std::to_string(zone_entry.rain_duration3));
		insert_values.push_back(std::to_string(zone_entry.rain_duration4));
		insert_values.push_back(std::to_string(zone_entry.snow_chance1));
		insert_values.push_back(std::to_string(zone_entry.snow_chance2));
		insert_values.push_back(std::to_string(zone_entry.snow_chance3));
		insert_values.push_back(std::to_string(zone_entry.snow_chance4));
		insert_values.push_back(std::to_string(zone_entry.snow_duration1));
		insert_values.push_back(std::to_string(zone_entry.snow_duration2));
		insert_values.push_back(std::to_string(zone_entry.snow_duration3));
		insert_values.push_back(std::to_string(zone_entry.snow_duration4));
		insert_values.push_back(std::to_string(zone_entry.gravity));
		insert_values.push_back(std::to_string(zone_entry.type));
		insert_values.push_back(std::to_string(zone_entry.skylock));
		insert_values.push_back(std::to_string(zone_entry.fast_regen_hp));
		insert_values.push_back(std::to_string(zone_entry.fast_regen_mana));
		insert_values.push_back(std::to_string(zone_entry.fast_regen_endurance));
		insert_values.push_back(std::to_string(zone_entry.npc_max_aggro_dist));
		insert_values.push_back(std::to_string(zone_entry.max_movement_update_range));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			zone_entry.id = results.LastInsertedID();
			return zone_entry;
		}

		zone_entry = ZoneRepository::NewEntity();

		return zone_entry;
	}

	static int InsertMany(
		std::vector<Zone> zone_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &zone_entry: zone_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(zone_entry.short_name) + "'");
			insert_values.push_back("'" + EscapeString(zone_entry.file_name) + "'");
			insert_values.push_back("'" + EscapeString(zone_entry.long_name) + "'");
			insert_values.push_back("'" + EscapeString(zone_entry.map_file_name) + "'");
			insert_values.push_back(std::to_string(zone_entry.safe_x));
			insert_values.push_back(std::to_string(zone_entry.safe_y));
			insert_values.push_back(std::to_string(zone_entry.safe_z));
			insert_values.push_back(std::to_string(zone_entry.graveyard_id));
			insert_values.push_back(std::to_string(zone_entry.min_level));
			insert_values.push_back(std::to_string(zone_entry.min_status));
			insert_values.push_back(std::to_string(zone_entry.zoneidnumber));
			insert_values.push_back(std::to_string(zone_entry.version));
			insert_values.push_back(std::to_string(zone_entry.timezone));
			insert_values.push_back(std::to_string(zone_entry.maxclients));
			insert_values.push_back(std::to_string(zone_entry.ruleset));
			insert_values.push_back("'" + EscapeString(zone_entry.note) + "'");
			insert_values.push_back(std::to_string(zone_entry.underworld));
			insert_values.push_back(std::to_string(zone_entry.minclip));
			insert_values.push_back(std::to_string(zone_entry.maxclip));
			insert_values.push_back(std::to_string(zone_entry.fog_minclip));
			insert_values.push_back(std::to_string(zone_entry.fog_maxclip));
			insert_values.push_back(std::to_string(zone_entry.fog_blue));
			insert_values.push_back(std::to_string(zone_entry.fog_red));
			insert_values.push_back(std::to_string(zone_entry.fog_green));
			insert_values.push_back(std::to_string(zone_entry.sky));
			insert_values.push_back(std::to_string(zone_entry.ztype));
			insert_values.push_back(std::to_string(zone_entry.zone_exp_multiplier));
			insert_values.push_back(std::to_string(zone_entry.walkspeed));
			insert_values.push_back(std::to_string(zone_entry.time_type));
			insert_values.push_back(std::to_string(zone_entry.fog_red1));
			insert_values.push_back(std::to_string(zone_entry.fog_green1));
			insert_values.push_back(std::to_string(zone_entry.fog_blue1));
			insert_values.push_back(std::to_string(zone_entry.fog_minclip1));
			insert_values.push_back(std::to_string(zone_entry.fog_maxclip1));
			insert_values.push_back(std::to_string(zone_entry.fog_red2));
			insert_values.push_back(std::to_string(zone_entry.fog_green2));
			insert_values.push_back(std::to_string(zone_entry.fog_blue2));
			insert_values.push_back(std::to_string(zone_entry.fog_minclip2));
			insert_values.push_back(std::to_string(zone_entry.fog_maxclip2));
			insert_values.push_back(std::to_string(zone_entry.fog_red3));
			insert_values.push_back(std::to_string(zone_entry.fog_green3));
			insert_values.push_back(std::to_string(zone_entry.fog_blue3));
			insert_values.push_back(std::to_string(zone_entry.fog_minclip3));
			insert_values.push_back(std::to_string(zone_entry.fog_maxclip3));
			insert_values.push_back(std::to_string(zone_entry.fog_red4));
			insert_values.push_back(std::to_string(zone_entry.fog_green4));
			insert_values.push_back(std::to_string(zone_entry.fog_blue4));
			insert_values.push_back(std::to_string(zone_entry.fog_minclip4));
			insert_values.push_back(std::to_string(zone_entry.fog_maxclip4));
			insert_values.push_back(std::to_string(zone_entry.fog_density));
			insert_values.push_back("'" + EscapeString(zone_entry.flag_needed) + "'");
			insert_values.push_back(std::to_string(zone_entry.canbind));
			insert_values.push_back(std::to_string(zone_entry.cancombat));
			insert_values.push_back(std::to_string(zone_entry.canlevitate));
			insert_values.push_back(std::to_string(zone_entry.castoutdoor));
			insert_values.push_back(std::to_string(zone_entry.hotzone));
			insert_values.push_back(std::to_string(zone_entry.insttype));
			insert_values.push_back(std::to_string(zone_entry.shutdowndelay));
			insert_values.push_back(std::to_string(zone_entry.peqzone));
			insert_values.push_back(std::to_string(zone_entry.expansion));
			insert_values.push_back(std::to_string(zone_entry.suspendbuffs));
			insert_values.push_back(std::to_string(zone_entry.rain_chance1));
			insert_values.push_back(std::to_string(zone_entry.rain_chance2));
			insert_values.push_back(std::to_string(zone_entry.rain_chance3));
			insert_values.push_back(std::to_string(zone_entry.rain_chance4));
			insert_values.push_back(std::to_string(zone_entry.rain_duration1));
			insert_values.push_back(std::to_string(zone_entry.rain_duration2));
			insert_values.push_back(std::to_string(zone_entry.rain_duration3));
			insert_values.push_back(std::to_string(zone_entry.rain_duration4));
			insert_values.push_back(std::to_string(zone_entry.snow_chance1));
			insert_values.push_back(std::to_string(zone_entry.snow_chance2));
			insert_values.push_back(std::to_string(zone_entry.snow_chance3));
			insert_values.push_back(std::to_string(zone_entry.snow_chance4));
			insert_values.push_back(std::to_string(zone_entry.snow_duration1));
			insert_values.push_back(std::to_string(zone_entry.snow_duration2));
			insert_values.push_back(std::to_string(zone_entry.snow_duration3));
			insert_values.push_back(std::to_string(zone_entry.snow_duration4));
			insert_values.push_back(std::to_string(zone_entry.gravity));
			insert_values.push_back(std::to_string(zone_entry.type));
			insert_values.push_back(std::to_string(zone_entry.skylock));
			insert_values.push_back(std::to_string(zone_entry.fast_regen_hp));
			insert_values.push_back(std::to_string(zone_entry.fast_regen_mana));
			insert_values.push_back(std::to_string(zone_entry.fast_regen_endurance));
			insert_values.push_back(std::to_string(zone_entry.npc_max_aggro_dist));
			insert_values.push_back(std::to_string(zone_entry.max_movement_update_range));

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<Zone> All()
	{
		std::vector<Zone> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Zone entry{};

			entry.short_name                = row[0];
			entry.id                        = atoi(row[1]);
			entry.file_name                 = row[2];
			entry.long_name                 = row[3];
			entry.map_file_name             = row[4];
			entry.safe_x                    = atof(row[5]);
			entry.safe_y                    = atof(row[6]);
			entry.safe_z                    = atof(row[7]);
			entry.graveyard_id              = atof(row[8]);
			entry.min_level                 = atoi(row[9]);
			entry.min_status                = atoi(row[10]);
			entry.zoneidnumber              = atoi(row[11]);
			entry.version                   = atoi(row[12]);
			entry.timezone                  = atoi(row[13]);
			entry.maxclients                = atoi(row[14]);
			entry.ruleset                   = atoi(row[15]);
			entry.note                      = row[16];
			entry.underworld                = atof(row[17]);
			entry.minclip                   = atof(row[18]);
			entry.maxclip                   = atof(row[19]);
			entry.fog_minclip               = atof(row[20]);
			entry.fog_maxclip               = atof(row[21]);
			entry.fog_blue                  = atoi(row[22]);
			entry.fog_red                   = atoi(row[23]);
			entry.fog_green                 = atoi(row[24]);
			entry.sky                       = atoi(row[25]);
			entry.ztype                     = atoi(row[26]);
			entry.zone_exp_multiplier       = atof(row[27]);
			entry.walkspeed                 = atof(row[28]);
			entry.time_type                 = atoi(row[29]);
			entry.fog_red1                  = atoi(row[30]);
			entry.fog_green1                = atoi(row[31]);
			entry.fog_blue1                 = atoi(row[32]);
			entry.fog_minclip1              = atof(row[33]);
			entry.fog_maxclip1              = atof(row[34]);
			entry.fog_red2                  = atoi(row[35]);
			entry.fog_green2                = atoi(row[36]);
			entry.fog_blue2                 = atoi(row[37]);
			entry.fog_minclip2              = atof(row[38]);
			entry.fog_maxclip2              = atof(row[39]);
			entry.fog_red3                  = atoi(row[40]);
			entry.fog_green3                = atoi(row[41]);
			entry.fog_blue3                 = atoi(row[42]);
			entry.fog_minclip3              = atof(row[43]);
			entry.fog_maxclip3              = atof(row[44]);
			entry.fog_red4                  = atoi(row[45]);
			entry.fog_green4                = atoi(row[46]);
			entry.fog_blue4                 = atoi(row[47]);
			entry.fog_minclip4              = atof(row[48]);
			entry.fog_maxclip4              = atof(row[49]);
			entry.fog_density               = atof(row[50]);
			entry.flag_needed               = row[51];
			entry.canbind                   = atoi(row[52]);
			entry.cancombat                 = atoi(row[53]);
			entry.canlevitate               = atoi(row[54]);
			entry.castoutdoor               = atoi(row[55]);
			entry.hotzone                   = atoi(row[56]);
			entry.insttype                  = atoi(row[57]);
			entry.shutdowndelay             = atoi(row[58]);
			entry.peqzone                   = atoi(row[59]);
			entry.expansion                 = atoi(row[60]);
			entry.suspendbuffs              = atoi(row[61]);
			entry.rain_chance1              = atoi(row[62]);
			entry.rain_chance2              = atoi(row[63]);
			entry.rain_chance3              = atoi(row[64]);
			entry.rain_chance4              = atoi(row[65]);
			entry.rain_duration1            = atoi(row[66]);
			entry.rain_duration2            = atoi(row[67]);
			entry.rain_duration3            = atoi(row[68]);
			entry.rain_duration4            = atoi(row[69]);
			entry.snow_chance1              = atoi(row[70]);
			entry.snow_chance2              = atoi(row[71]);
			entry.snow_chance3              = atoi(row[72]);
			entry.snow_chance4              = atoi(row[73]);
			entry.snow_duration1            = atoi(row[74]);
			entry.snow_duration2            = atoi(row[75]);
			entry.snow_duration3            = atoi(row[76]);
			entry.snow_duration4            = atoi(row[77]);
			entry.gravity                   = atof(row[78]);
			entry.type                      = atoi(row[79]);
			entry.skylock                   = atoi(row[80]);
			entry.fast_regen_hp             = atoi(row[81]);
			entry.fast_regen_mana           = atoi(row[82]);
			entry.fast_regen_endurance      = atoi(row[83]);
			entry.npc_max_aggro_dist        = atoi(row[84]);
			entry.max_movement_update_range = atoi(row[85]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Zone> GetWhere(std::string where_filter)
	{
		std::vector<Zone> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Zone entry{};

			entry.short_name                = row[0];
			entry.id                        = atoi(row[1]);
			entry.file_name                 = row[2];
			entry.long_name                 = row[3];
			entry.map_file_name             = row[4];
			entry.safe_x                    = atof(row[5]);
			entry.safe_y                    = atof(row[6]);
			entry.safe_z                    = atof(row[7]);
			entry.graveyard_id              = atof(row[8]);
			entry.min_level                 = atoi(row[9]);
			entry.min_status                = atoi(row[10]);
			entry.zoneidnumber              = atoi(row[11]);
			entry.version                   = atoi(row[12]);
			entry.timezone                  = atoi(row[13]);
			entry.maxclients                = atoi(row[14]);
			entry.ruleset                   = atoi(row[15]);
			entry.note                      = row[16];
			entry.underworld                = atof(row[17]);
			entry.minclip                   = atof(row[18]);
			entry.maxclip                   = atof(row[19]);
			entry.fog_minclip               = atof(row[20]);
			entry.fog_maxclip               = atof(row[21]);
			entry.fog_blue                  = atoi(row[22]);
			entry.fog_red                   = atoi(row[23]);
			entry.fog_green                 = atoi(row[24]);
			entry.sky                       = atoi(row[25]);
			entry.ztype                     = atoi(row[26]);
			entry.zone_exp_multiplier       = atof(row[27]);
			entry.walkspeed                 = atof(row[28]);
			entry.time_type                 = atoi(row[29]);
			entry.fog_red1                  = atoi(row[30]);
			entry.fog_green1                = atoi(row[31]);
			entry.fog_blue1                 = atoi(row[32]);
			entry.fog_minclip1              = atof(row[33]);
			entry.fog_maxclip1              = atof(row[34]);
			entry.fog_red2                  = atoi(row[35]);
			entry.fog_green2                = atoi(row[36]);
			entry.fog_blue2                 = atoi(row[37]);
			entry.fog_minclip2              = atof(row[38]);
			entry.fog_maxclip2              = atof(row[39]);
			entry.fog_red3                  = atoi(row[40]);
			entry.fog_green3                = atoi(row[41]);
			entry.fog_blue3                 = atoi(row[42]);
			entry.fog_minclip3              = atof(row[43]);
			entry.fog_maxclip3              = atof(row[44]);
			entry.fog_red4                  = atoi(row[45]);
			entry.fog_green4                = atoi(row[46]);
			entry.fog_blue4                 = atoi(row[47]);
			entry.fog_minclip4              = atof(row[48]);
			entry.fog_maxclip4              = atof(row[49]);
			entry.fog_density               = atof(row[50]);
			entry.flag_needed               = row[51];
			entry.canbind                   = atoi(row[52]);
			entry.cancombat                 = atoi(row[53]);
			entry.canlevitate               = atoi(row[54]);
			entry.castoutdoor               = atoi(row[55]);
			entry.hotzone                   = atoi(row[56]);
			entry.insttype                  = atoi(row[57]);
			entry.shutdowndelay             = atoi(row[58]);
			entry.peqzone                   = atoi(row[59]);
			entry.expansion                 = atoi(row[60]);
			entry.suspendbuffs              = atoi(row[61]);
			entry.rain_chance1              = atoi(row[62]);
			entry.rain_chance2              = atoi(row[63]);
			entry.rain_chance3              = atoi(row[64]);
			entry.rain_chance4              = atoi(row[65]);
			entry.rain_duration1            = atoi(row[66]);
			entry.rain_duration2            = atoi(row[67]);
			entry.rain_duration3            = atoi(row[68]);
			entry.rain_duration4            = atoi(row[69]);
			entry.snow_chance1              = atoi(row[70]);
			entry.snow_chance2              = atoi(row[71]);
			entry.snow_chance3              = atoi(row[72]);
			entry.snow_chance4              = atoi(row[73]);
			entry.snow_duration1            = atoi(row[74]);
			entry.snow_duration2            = atoi(row[75]);
			entry.snow_duration3            = atoi(row[76]);
			entry.snow_duration4            = atoi(row[77]);
			entry.gravity                   = atof(row[78]);
			entry.type                      = atoi(row[79]);
			entry.skylock                   = atoi(row[80]);
			entry.fast_regen_hp             = atoi(row[81]);
			entry.fast_regen_mana           = atoi(row[82]);
			entry.fast_regen_endurance      = atoi(row[83]);
			entry.npc_max_aggro_dist        = atoi(row[84]);
			entry.max_movement_update_range = atoi(row[85]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static int DeleteWhere(std::string where_filter)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				PrimaryKey(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_ZONE_REPOSITORY_H
