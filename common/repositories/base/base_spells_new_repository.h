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

#ifndef EQEMU_BASE_SPELLS_NEW_REPOSITORY_H
#define EQEMU_BASE_SPELLS_NEW_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseSpellsNewRepository {
public:
	struct SpellsNew {
		int         id;
		std::string name;
		std::string player_1;
		std::string teleport_zone;
		std::string you_cast;
		std::string other_casts;
		std::string cast_on_you;
		std::string cast_on_other;
		std::string spell_fades;
		int         range;
		int         aoerange;
		int         pushback;
		int         pushup;
		int         cast_time;
		int         recovery_time;
		int         recast_time;
		int         buffdurationformula;
		int         buffduration;
		int         AEDuration;
		int         mana;
		int         effect_base_value1;
		int         effect_base_value2;
		int         effect_base_value3;
		int         effect_base_value4;
		int         effect_base_value5;
		int         effect_base_value6;
		int         effect_base_value7;
		int         effect_base_value8;
		int         effect_base_value9;
		int         effect_base_value10;
		int         effect_base_value11;
		int         effect_base_value12;
		int         effect_limit_value1;
		int         effect_limit_value2;
		int         effect_limit_value3;
		int         effect_limit_value4;
		int         effect_limit_value5;
		int         effect_limit_value6;
		int         effect_limit_value7;
		int         effect_limit_value8;
		int         effect_limit_value9;
		int         effect_limit_value10;
		int         effect_limit_value11;
		int         effect_limit_value12;
		int         max1;
		int         max2;
		int         max3;
		int         max4;
		int         max5;
		int         max6;
		int         max7;
		int         max8;
		int         max9;
		int         max10;
		int         max11;
		int         max12;
		int         icon;
		int         memicon;
		int         components1;
		int         components2;
		int         components3;
		int         components4;
		int         component_counts1;
		int         component_counts2;
		int         component_counts3;
		int         component_counts4;
		int         NoexpendReagent1;
		int         NoexpendReagent2;
		int         NoexpendReagent3;
		int         NoexpendReagent4;
		int         formula1;
		int         formula2;
		int         formula3;
		int         formula4;
		int         formula5;
		int         formula6;
		int         formula7;
		int         formula8;
		int         formula9;
		int         formula10;
		int         formula11;
		int         formula12;
		int         LightType;
		int         goodEffect;
		int         Activated;
		int         resisttype;
		int         effectid1;
		int         effectid2;
		int         effectid3;
		int         effectid4;
		int         effectid5;
		int         effectid6;
		int         effectid7;
		int         effectid8;
		int         effectid9;
		int         effectid10;
		int         effectid11;
		int         effectid12;
		int         targettype;
		int         basediff;
		int         skill;
		int         zonetype;
		int         EnvironmentType;
		int         TimeOfDay;
		int         classes1;
		int         classes2;
		int         classes3;
		int         classes4;
		int         classes5;
		int         classes6;
		int         classes7;
		int         classes8;
		int         classes9;
		int         classes10;
		int         classes11;
		int         classes12;
		int         classes13;
		int         classes14;
		int         classes15;
		int         classes16;
		int         CastingAnim;
		int         TargetAnim;
		int         TravelType;
		int         SpellAffectIndex;
		int         disallow_sit;
		int         deities0;
		int         deities1;
		int         deities2;
		int         deities3;
		int         deities4;
		int         deities5;
		int         deities6;
		int         deities7;
		int         deities8;
		int         deities9;
		int         deities10;
		int         deities11;
		int         deities12;
		int         deities13;
		int         deities14;
		int         deities15;
		int         deities16;
		int         field142;
		int         field143;
		int         new_icon;
		int         spellanim;
		int         uninterruptable;
		int         ResistDiff;
		int         dot_stacking_exempt;
		int         deleteable;
		int         RecourseLink;
		int         no_partial_resist;
		int         field152;
		int         field153;
		int         short_buff_box;
		int         descnum;
		int         typedescnum;
		int         effectdescnum;
		int         effectdescnum2;
		int         npc_no_los;
		int         field160;
		int         reflectable;
		int         bonushate;
		int         field163;
		int         field164;
		int         ldon_trap;
		int         EndurCost;
		int         EndurTimerIndex;
		int         IsDiscipline;
		int         field169;
		int         field170;
		int         field171;
		int         field172;
		int         HateAdded;
		int         EndurUpkeep;
		int         numhitstype;
		int         numhits;
		int         pvpresistbase;
		int         pvpresistcalc;
		int         pvpresistcap;
		int         spell_category;
		int         field181;
		int         field182;
		int         pcnpc_only_flag;
		int         cast_not_standing;
		int         can_mgb;
		int         nodispell;
		int         npc_category;
		int         npc_usefulness;
		int         MinResist;
		int         MaxResist;
		int         viral_targets;
		int         viral_timer;
		int         nimbuseffect;
		int         ConeStartAngle;
		int         ConeStopAngle;
		int         sneaking;
		int         not_extendable;
		int         field198;
		int         field199;
		int         suspendable;
		int         viral_range;
		int         songcap;
		int         field203;
		int         field204;
		int         no_block;
		int         field206;
		int         spellgroup;
		int         rank;
		int         field209;
		int         field210;
		int         CastRestriction;
		int         allowrest;
		int         InCombat;
		int         OutofCombat;
		int         field215;
		int         field216;
		int         field217;
		int         aemaxtargets;
		int         maxtargets;
		int         field220;
		int         field221;
		int         field222;
		int         field223;
		int         persistdeath;
		int         field225;
		int         field226;
		float       min_dist;
		float       min_dist_mod;
		float       max_dist;
		float       max_dist_mod;
		int         min_range;
		int         field232;
		int         field233;
		int         field234;
		int         field235;
		int         field236;
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
			"player_1",
			"teleport_zone",
			"you_cast",
			"other_casts",
			"cast_on_you",
			"cast_on_other",
			"spell_fades",
			"range",
			"aoerange",
			"pushback",
			"pushup",
			"cast_time",
			"recovery_time",
			"recast_time",
			"buffdurationformula",
			"buffduration",
			"AEDuration",
			"mana",
			"effect_base_value1",
			"effect_base_value2",
			"effect_base_value3",
			"effect_base_value4",
			"effect_base_value5",
			"effect_base_value6",
			"effect_base_value7",
			"effect_base_value8",
			"effect_base_value9",
			"effect_base_value10",
			"effect_base_value11",
			"effect_base_value12",
			"effect_limit_value1",
			"effect_limit_value2",
			"effect_limit_value3",
			"effect_limit_value4",
			"effect_limit_value5",
			"effect_limit_value6",
			"effect_limit_value7",
			"effect_limit_value8",
			"effect_limit_value9",
			"effect_limit_value10",
			"effect_limit_value11",
			"effect_limit_value12",
			"max1",
			"max2",
			"max3",
			"max4",
			"max5",
			"max6",
			"max7",
			"max8",
			"max9",
			"max10",
			"max11",
			"max12",
			"icon",
			"memicon",
			"components1",
			"components2",
			"components3",
			"components4",
			"component_counts1",
			"component_counts2",
			"component_counts3",
			"component_counts4",
			"NoexpendReagent1",
			"NoexpendReagent2",
			"NoexpendReagent3",
			"NoexpendReagent4",
			"formula1",
			"formula2",
			"formula3",
			"formula4",
			"formula5",
			"formula6",
			"formula7",
			"formula8",
			"formula9",
			"formula10",
			"formula11",
			"formula12",
			"LightType",
			"goodEffect",
			"Activated",
			"resisttype",
			"effectid1",
			"effectid2",
			"effectid3",
			"effectid4",
			"effectid5",
			"effectid6",
			"effectid7",
			"effectid8",
			"effectid9",
			"effectid10",
			"effectid11",
			"effectid12",
			"targettype",
			"basediff",
			"skill",
			"zonetype",
			"EnvironmentType",
			"TimeOfDay",
			"classes1",
			"classes2",
			"classes3",
			"classes4",
			"classes5",
			"classes6",
			"classes7",
			"classes8",
			"classes9",
			"classes10",
			"classes11",
			"classes12",
			"classes13",
			"classes14",
			"classes15",
			"classes16",
			"CastingAnim",
			"TargetAnim",
			"TravelType",
			"SpellAffectIndex",
			"disallow_sit",
			"deities0",
			"deities1",
			"deities2",
			"deities3",
			"deities4",
			"deities5",
			"deities6",
			"deities7",
			"deities8",
			"deities9",
			"deities10",
			"deities11",
			"deities12",
			"deities13",
			"deities14",
			"deities15",
			"deities16",
			"field142",
			"field143",
			"new_icon",
			"spellanim",
			"uninterruptable",
			"ResistDiff",
			"dot_stacking_exempt",
			"deleteable",
			"RecourseLink",
			"no_partial_resist",
			"field152",
			"field153",
			"short_buff_box",
			"descnum",
			"typedescnum",
			"effectdescnum",
			"effectdescnum2",
			"npc_no_los",
			"field160",
			"reflectable",
			"bonushate",
			"field163",
			"field164",
			"ldon_trap",
			"EndurCost",
			"EndurTimerIndex",
			"IsDiscipline",
			"field169",
			"field170",
			"field171",
			"field172",
			"HateAdded",
			"EndurUpkeep",
			"numhitstype",
			"numhits",
			"pvpresistbase",
			"pvpresistcalc",
			"pvpresistcap",
			"spell_category",
			"field181",
			"field182",
			"pcnpc_only_flag",
			"cast_not_standing",
			"can_mgb",
			"nodispell",
			"npc_category",
			"npc_usefulness",
			"MinResist",
			"MaxResist",
			"viral_targets",
			"viral_timer",
			"nimbuseffect",
			"ConeStartAngle",
			"ConeStopAngle",
			"sneaking",
			"not_extendable",
			"field198",
			"field199",
			"suspendable",
			"viral_range",
			"songcap",
			"field203",
			"field204",
			"no_block",
			"field206",
			"spellgroup",
			"rank",
			"field209",
			"field210",
			"CastRestriction",
			"allowrest",
			"InCombat",
			"OutofCombat",
			"field215",
			"field216",
			"field217",
			"aemaxtargets",
			"maxtargets",
			"field220",
			"field221",
			"field222",
			"field223",
			"persistdeath",
			"field225",
			"field226",
			"min_dist",
			"min_dist_mod",
			"max_dist",
			"max_dist_mod",
			"min_range",
			"field232",
			"field233",
			"field234",
			"field235",
			"field236",
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
		return std::string("spells_new");
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

	static SpellsNew NewEntity()
	{
		SpellsNew entry{};

		entry.id                   = 0;
		entry.name                 = "";
		entry.player_1             = "BLUE_TRAIL";
		entry.teleport_zone        = "";
		entry.you_cast             = "";
		entry.other_casts          = "";
		entry.cast_on_you          = "";
		entry.cast_on_other        = "";
		entry.spell_fades          = "";
		entry.range                = 100;
		entry.aoerange             = 0;
		entry.pushback             = 0;
		entry.pushup               = 0;
		entry.cast_time            = 0;
		entry.recovery_time        = 0;
		entry.recast_time          = 0;
		entry.buffdurationformula  = 7;
		entry.buffduration         = 65;
		entry.AEDuration           = 0;
		entry.mana                 = 0;
		entry.effect_base_value1   = 100;
		entry.effect_base_value2   = 0;
		entry.effect_base_value3   = 0;
		entry.effect_base_value4   = 0;
		entry.effect_base_value5   = 0;
		entry.effect_base_value6   = 0;
		entry.effect_base_value7   = 0;
		entry.effect_base_value8   = 0;
		entry.effect_base_value9   = 0;
		entry.effect_base_value10  = 0;
		entry.effect_base_value11  = 0;
		entry.effect_base_value12  = 0;
		entry.effect_limit_value1  = 0;
		entry.effect_limit_value2  = 0;
		entry.effect_limit_value3  = 0;
		entry.effect_limit_value4  = 0;
		entry.effect_limit_value5  = 0;
		entry.effect_limit_value6  = 0;
		entry.effect_limit_value7  = 0;
		entry.effect_limit_value8  = 0;
		entry.effect_limit_value9  = 0;
		entry.effect_limit_value10 = 0;
		entry.effect_limit_value11 = 0;
		entry.effect_limit_value12 = 0;
		entry.max1                 = 0;
		entry.max2                 = 0;
		entry.max3                 = 0;
		entry.max4                 = 0;
		entry.max5                 = 0;
		entry.max6                 = 0;
		entry.max7                 = 0;
		entry.max8                 = 0;
		entry.max9                 = 0;
		entry.max10                = 0;
		entry.max11                = 0;
		entry.max12                = 0;
		entry.icon                 = 0;
		entry.memicon              = 0;
		entry.components1          = -1;
		entry.components2          = -1;
		entry.components3          = -1;
		entry.components4          = -1;
		entry.component_counts1    = 1;
		entry.component_counts2    = 1;
		entry.component_counts3    = 1;
		entry.component_counts4    = 1;
		entry.NoexpendReagent1     = -1;
		entry.NoexpendReagent2     = -1;
		entry.NoexpendReagent3     = -1;
		entry.NoexpendReagent4     = -1;
		entry.formula1             = 100;
		entry.formula2             = 100;
		entry.formula3             = 100;
		entry.formula4             = 100;
		entry.formula5             = 100;
		entry.formula6             = 100;
		entry.formula7             = 100;
		entry.formula8             = 100;
		entry.formula9             = 100;
		entry.formula10            = 100;
		entry.formula11            = 100;
		entry.formula12            = 100;
		entry.LightType            = 0;
		entry.goodEffect           = 0;
		entry.Activated            = 0;
		entry.resisttype           = 0;
		entry.effectid1            = 254;
		entry.effectid2            = 254;
		entry.effectid3            = 254;
		entry.effectid4            = 254;
		entry.effectid5            = 254;
		entry.effectid6            = 254;
		entry.effectid7            = 254;
		entry.effectid8            = 254;
		entry.effectid9            = 254;
		entry.effectid10           = 254;
		entry.effectid11           = 254;
		entry.effectid12           = 254;
		entry.targettype           = 2;
		entry.basediff             = 0;
		entry.skill                = 98;
		entry.zonetype             = -1;
		entry.EnvironmentType      = 0;
		entry.TimeOfDay            = 0;
		entry.classes1             = 255;
		entry.classes2             = 255;
		entry.classes3             = 255;
		entry.classes4             = 255;
		entry.classes5             = 255;
		entry.classes6             = 255;
		entry.classes7             = 255;
		entry.classes8             = 255;
		entry.classes9             = 255;
		entry.classes10            = 255;
		entry.classes11            = 255;
		entry.classes12            = 255;
		entry.classes13            = 255;
		entry.classes14            = 255;
		entry.classes15            = 255;
		entry.classes16            = 255;
		entry.CastingAnim          = 44;
		entry.TargetAnim           = 13;
		entry.TravelType           = 0;
		entry.SpellAffectIndex     = -1;
		entry.disallow_sit         = 0;
		entry.deities0             = 0;
		entry.deities1             = 0;
		entry.deities2             = 0;
		entry.deities3             = 0;
		entry.deities4             = 0;
		entry.deities5             = 0;
		entry.deities6             = 0;
		entry.deities7             = 0;
		entry.deities8             = 0;
		entry.deities9             = 0;
		entry.deities10            = 0;
		entry.deities11            = 0;
		entry.deities12            = 0;
		entry.deities13            = 0;
		entry.deities14            = 0;
		entry.deities15            = 0;
		entry.deities16            = 0;
		entry.field142             = 100;
		entry.field143             = 0;
		entry.new_icon             = 161;
		entry.spellanim            = 0;
		entry.uninterruptable      = 0;
		entry.ResistDiff           = -150;
		entry.dot_stacking_exempt  = 0;
		entry.deleteable           = 0;
		entry.RecourseLink         = 0;
		entry.no_partial_resist    = 0;
		entry.field152             = 0;
		entry.field153             = 0;
		entry.short_buff_box       = -1;
		entry.descnum              = 0;
		entry.typedescnum          = 0;
		entry.effectdescnum        = 0;
		entry.effectdescnum2       = 0;
		entry.npc_no_los           = 0;
		entry.field160             = 0;
		entry.reflectable          = 0;
		entry.bonushate            = 0;
		entry.field163             = 100;
		entry.field164             = -150;
		entry.ldon_trap            = 0;
		entry.EndurCost            = 0;
		entry.EndurTimerIndex      = 0;
		entry.IsDiscipline         = 0;
		entry.field169             = 0;
		entry.field170             = 0;
		entry.field171             = 0;
		entry.field172             = 0;
		entry.HateAdded            = 0;
		entry.EndurUpkeep          = 0;
		entry.numhitstype          = 0;
		entry.numhits              = 0;
		entry.pvpresistbase        = -150;
		entry.pvpresistcalc        = 100;
		entry.pvpresistcap         = -150;
		entry.spell_category       = -99;
		entry.field181             = 7;
		entry.field182             = 65;
		entry.pcnpc_only_flag      = 0;
		entry.cast_not_standing    = 0;
		entry.can_mgb              = 0;
		entry.nodispell            = -1;
		entry.npc_category         = 0;
		entry.npc_usefulness       = 0;
		entry.MinResist            = 0;
		entry.MaxResist            = 0;
		entry.viral_targets        = 0;
		entry.viral_timer          = 0;
		entry.nimbuseffect         = 0;
		entry.ConeStartAngle       = 0;
		entry.ConeStopAngle        = 0;
		entry.sneaking             = 0;
		entry.not_extendable       = 0;
		entry.field198             = 0;
		entry.field199             = 1;
		entry.suspendable          = 0;
		entry.viral_range          = 0;
		entry.songcap              = 0;
		entry.field203             = 0;
		entry.field204             = 0;
		entry.no_block             = 0;
		entry.field206             = -1;
		entry.spellgroup           = 0;
		entry.rank                 = 0;
		entry.field209             = 0;
		entry.field210             = 1;
		entry.CastRestriction      = 0;
		entry.allowrest            = 0;
		entry.InCombat             = 0;
		entry.OutofCombat          = 0;
		entry.field215             = 0;
		entry.field216             = 0;
		entry.field217             = 0;
		entry.aemaxtargets         = 0;
		entry.maxtargets           = 0;
		entry.field220             = 0;
		entry.field221             = 0;
		entry.field222             = 0;
		entry.field223             = 0;
		entry.persistdeath         = 0;
		entry.field225             = 0;
		entry.field226             = 0;
		entry.min_dist             = 0;
		entry.min_dist_mod         = 0;
		entry.max_dist             = 0;
		entry.max_dist_mod         = 0;
		entry.min_range            = 0;
		entry.field232             = 0;
		entry.field233             = 0;
		entry.field234             = 0;
		entry.field235             = 0;
		entry.field236             = 0;

		return entry;
	}

	static SpellsNew GetSpellsNewEntry(
		const std::vector<SpellsNew> &spells_news,
		int spells_new_id
	)
	{
		for (auto &spells_new : spells_news) {
			if (spells_new.id == spells_new_id) {
				return spells_new;
			}
		}

		return NewEntity();
	}

	static SpellsNew FindOne(
		Database& db,
		int spells_new_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				spells_new_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			SpellsNew entry{};

			entry.id                   = atoi(row[0]);
			entry.name                 = row[1] ? row[1] : "";
			entry.player_1             = row[2] ? row[2] : "";
			entry.teleport_zone        = row[3] ? row[3] : "";
			entry.you_cast             = row[4] ? row[4] : "";
			entry.other_casts          = row[5] ? row[5] : "";
			entry.cast_on_you          = row[6] ? row[6] : "";
			entry.cast_on_other        = row[7] ? row[7] : "";
			entry.spell_fades          = row[8] ? row[8] : "";
			entry.range                = atoi(row[9]);
			entry.aoerange             = atoi(row[10]);
			entry.pushback             = atoi(row[11]);
			entry.pushup               = atoi(row[12]);
			entry.cast_time            = atoi(row[13]);
			entry.recovery_time        = atoi(row[14]);
			entry.recast_time          = atoi(row[15]);
			entry.buffdurationformula  = atoi(row[16]);
			entry.buffduration         = atoi(row[17]);
			entry.AEDuration           = atoi(row[18]);
			entry.mana                 = atoi(row[19]);
			entry.effect_base_value1   = atoi(row[20]);
			entry.effect_base_value2   = atoi(row[21]);
			entry.effect_base_value3   = atoi(row[22]);
			entry.effect_base_value4   = atoi(row[23]);
			entry.effect_base_value5   = atoi(row[24]);
			entry.effect_base_value6   = atoi(row[25]);
			entry.effect_base_value7   = atoi(row[26]);
			entry.effect_base_value8   = atoi(row[27]);
			entry.effect_base_value9   = atoi(row[28]);
			entry.effect_base_value10  = atoi(row[29]);
			entry.effect_base_value11  = atoi(row[30]);
			entry.effect_base_value12  = atoi(row[31]);
			entry.effect_limit_value1  = atoi(row[32]);
			entry.effect_limit_value2  = atoi(row[33]);
			entry.effect_limit_value3  = atoi(row[34]);
			entry.effect_limit_value4  = atoi(row[35]);
			entry.effect_limit_value5  = atoi(row[36]);
			entry.effect_limit_value6  = atoi(row[37]);
			entry.effect_limit_value7  = atoi(row[38]);
			entry.effect_limit_value8  = atoi(row[39]);
			entry.effect_limit_value9  = atoi(row[40]);
			entry.effect_limit_value10 = atoi(row[41]);
			entry.effect_limit_value11 = atoi(row[42]);
			entry.effect_limit_value12 = atoi(row[43]);
			entry.max1                 = atoi(row[44]);
			entry.max2                 = atoi(row[45]);
			entry.max3                 = atoi(row[46]);
			entry.max4                 = atoi(row[47]);
			entry.max5                 = atoi(row[48]);
			entry.max6                 = atoi(row[49]);
			entry.max7                 = atoi(row[50]);
			entry.max8                 = atoi(row[51]);
			entry.max9                 = atoi(row[52]);
			entry.max10                = atoi(row[53]);
			entry.max11                = atoi(row[54]);
			entry.max12                = atoi(row[55]);
			entry.icon                 = atoi(row[56]);
			entry.memicon              = atoi(row[57]);
			entry.components1          = atoi(row[58]);
			entry.components2          = atoi(row[59]);
			entry.components3          = atoi(row[60]);
			entry.components4          = atoi(row[61]);
			entry.component_counts1    = atoi(row[62]);
			entry.component_counts2    = atoi(row[63]);
			entry.component_counts3    = atoi(row[64]);
			entry.component_counts4    = atoi(row[65]);
			entry.NoexpendReagent1     = atoi(row[66]);
			entry.NoexpendReagent2     = atoi(row[67]);
			entry.NoexpendReagent3     = atoi(row[68]);
			entry.NoexpendReagent4     = atoi(row[69]);
			entry.formula1             = atoi(row[70]);
			entry.formula2             = atoi(row[71]);
			entry.formula3             = atoi(row[72]);
			entry.formula4             = atoi(row[73]);
			entry.formula5             = atoi(row[74]);
			entry.formula6             = atoi(row[75]);
			entry.formula7             = atoi(row[76]);
			entry.formula8             = atoi(row[77]);
			entry.formula9             = atoi(row[78]);
			entry.formula10            = atoi(row[79]);
			entry.formula11            = atoi(row[80]);
			entry.formula12            = atoi(row[81]);
			entry.LightType            = atoi(row[82]);
			entry.goodEffect           = atoi(row[83]);
			entry.Activated            = atoi(row[84]);
			entry.resisttype           = atoi(row[85]);
			entry.effectid1            = atoi(row[86]);
			entry.effectid2            = atoi(row[87]);
			entry.effectid3            = atoi(row[88]);
			entry.effectid4            = atoi(row[89]);
			entry.effectid5            = atoi(row[90]);
			entry.effectid6            = atoi(row[91]);
			entry.effectid7            = atoi(row[92]);
			entry.effectid8            = atoi(row[93]);
			entry.effectid9            = atoi(row[94]);
			entry.effectid10           = atoi(row[95]);
			entry.effectid11           = atoi(row[96]);
			entry.effectid12           = atoi(row[97]);
			entry.targettype           = atoi(row[98]);
			entry.basediff             = atoi(row[99]);
			entry.skill                = atoi(row[100]);
			entry.zonetype             = atoi(row[101]);
			entry.EnvironmentType      = atoi(row[102]);
			entry.TimeOfDay            = atoi(row[103]);
			entry.classes1             = atoi(row[104]);
			entry.classes2             = atoi(row[105]);
			entry.classes3             = atoi(row[106]);
			entry.classes4             = atoi(row[107]);
			entry.classes5             = atoi(row[108]);
			entry.classes6             = atoi(row[109]);
			entry.classes7             = atoi(row[110]);
			entry.classes8             = atoi(row[111]);
			entry.classes9             = atoi(row[112]);
			entry.classes10            = atoi(row[113]);
			entry.classes11            = atoi(row[114]);
			entry.classes12            = atoi(row[115]);
			entry.classes13            = atoi(row[116]);
			entry.classes14            = atoi(row[117]);
			entry.classes15            = atoi(row[118]);
			entry.classes16            = atoi(row[119]);
			entry.CastingAnim          = atoi(row[120]);
			entry.TargetAnim           = atoi(row[121]);
			entry.TravelType           = atoi(row[122]);
			entry.SpellAffectIndex     = atoi(row[123]);
			entry.disallow_sit         = atoi(row[124]);
			entry.deities0             = atoi(row[125]);
			entry.deities1             = atoi(row[126]);
			entry.deities2             = atoi(row[127]);
			entry.deities3             = atoi(row[128]);
			entry.deities4             = atoi(row[129]);
			entry.deities5             = atoi(row[130]);
			entry.deities6             = atoi(row[131]);
			entry.deities7             = atoi(row[132]);
			entry.deities8             = atoi(row[133]);
			entry.deities9             = atoi(row[134]);
			entry.deities10            = atoi(row[135]);
			entry.deities11            = atoi(row[136]);
			entry.deities12            = atoi(row[137]);
			entry.deities13            = atoi(row[138]);
			entry.deities14            = atoi(row[139]);
			entry.deities15            = atoi(row[140]);
			entry.deities16            = atoi(row[141]);
			entry.field142             = atoi(row[142]);
			entry.field143             = atoi(row[143]);
			entry.new_icon             = atoi(row[144]);
			entry.spellanim            = atoi(row[145]);
			entry.uninterruptable      = atoi(row[146]);
			entry.ResistDiff           = atoi(row[147]);
			entry.dot_stacking_exempt  = atoi(row[148]);
			entry.deleteable           = atoi(row[149]);
			entry.RecourseLink         = atoi(row[150]);
			entry.no_partial_resist    = atoi(row[151]);
			entry.field152             = atoi(row[152]);
			entry.field153             = atoi(row[153]);
			entry.short_buff_box       = atoi(row[154]);
			entry.descnum              = atoi(row[155]);
			entry.typedescnum          = atoi(row[156]);
			entry.effectdescnum        = atoi(row[157]);
			entry.effectdescnum2       = atoi(row[158]);
			entry.npc_no_los           = atoi(row[159]);
			entry.field160             = atoi(row[160]);
			entry.reflectable          = atoi(row[161]);
			entry.bonushate            = atoi(row[162]);
			entry.field163             = atoi(row[163]);
			entry.field164             = atoi(row[164]);
			entry.ldon_trap            = atoi(row[165]);
			entry.EndurCost            = atoi(row[166]);
			entry.EndurTimerIndex      = atoi(row[167]);
			entry.IsDiscipline         = atoi(row[168]);
			entry.field169             = atoi(row[169]);
			entry.field170             = atoi(row[170]);
			entry.field171             = atoi(row[171]);
			entry.field172             = atoi(row[172]);
			entry.HateAdded            = atoi(row[173]);
			entry.EndurUpkeep          = atoi(row[174]);
			entry.numhitstype          = atoi(row[175]);
			entry.numhits              = atoi(row[176]);
			entry.pvpresistbase        = atoi(row[177]);
			entry.pvpresistcalc        = atoi(row[178]);
			entry.pvpresistcap         = atoi(row[179]);
			entry.spell_category       = atoi(row[180]);
			entry.field181             = atoi(row[181]);
			entry.field182             = atoi(row[182]);
			entry.pcnpc_only_flag      = atoi(row[183]);
			entry.cast_not_standing    = atoi(row[184]);
			entry.can_mgb              = atoi(row[185]);
			entry.nodispell            = atoi(row[186]);
			entry.npc_category         = atoi(row[187]);
			entry.npc_usefulness       = atoi(row[188]);
			entry.MinResist            = atoi(row[189]);
			entry.MaxResist            = atoi(row[190]);
			entry.viral_targets        = atoi(row[191]);
			entry.viral_timer          = atoi(row[192]);
			entry.nimbuseffect         = atoi(row[193]);
			entry.ConeStartAngle       = atoi(row[194]);
			entry.ConeStopAngle        = atoi(row[195]);
			entry.sneaking             = atoi(row[196]);
			entry.not_extendable       = atoi(row[197]);
			entry.field198             = atoi(row[198]);
			entry.field199             = atoi(row[199]);
			entry.suspendable          = atoi(row[200]);
			entry.viral_range          = atoi(row[201]);
			entry.songcap              = atoi(row[202]);
			entry.field203             = atoi(row[203]);
			entry.field204             = atoi(row[204]);
			entry.no_block             = atoi(row[205]);
			entry.field206             = atoi(row[206]);
			entry.spellgroup           = atoi(row[207]);
			entry.rank                 = atoi(row[208]);
			entry.field209             = atoi(row[209]);
			entry.field210             = atoi(row[210]);
			entry.CastRestriction      = atoi(row[211]);
			entry.allowrest            = atoi(row[212]);
			entry.InCombat             = atoi(row[213]);
			entry.OutofCombat          = atoi(row[214]);
			entry.field215             = atoi(row[215]);
			entry.field216             = atoi(row[216]);
			entry.field217             = atoi(row[217]);
			entry.aemaxtargets         = atoi(row[218]);
			entry.maxtargets           = atoi(row[219]);
			entry.field220             = atoi(row[220]);
			entry.field221             = atoi(row[221]);
			entry.field222             = atoi(row[222]);
			entry.field223             = atoi(row[223]);
			entry.persistdeath         = atoi(row[224]);
			entry.field225             = atoi(row[225]);
			entry.field226             = atoi(row[226]);
			entry.min_dist             = static_cast<float>(atof(row[227]));
			entry.min_dist_mod         = static_cast<float>(atof(row[228]));
			entry.max_dist             = static_cast<float>(atof(row[229]));
			entry.max_dist_mod         = static_cast<float>(atof(row[230]));
			entry.min_range            = atoi(row[231]);
			entry.field232             = atoi(row[232]);
			entry.field233             = atoi(row[233]);
			entry.field234             = atoi(row[234]);
			entry.field235             = atoi(row[235]);
			entry.field236             = atoi(row[236]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int spells_new_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				spells_new_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		SpellsNew spells_new_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(spells_new_entry.id));
		update_values.push_back(columns[1] + " = '" + EscapeString(spells_new_entry.name) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(spells_new_entry.player_1) + "'");
		update_values.push_back(columns[3] + " = '" + EscapeString(spells_new_entry.teleport_zone) + "'");
		update_values.push_back(columns[4] + " = '" + EscapeString(spells_new_entry.you_cast) + "'");
		update_values.push_back(columns[5] + " = '" + EscapeString(spells_new_entry.other_casts) + "'");
		update_values.push_back(columns[6] + " = '" + EscapeString(spells_new_entry.cast_on_you) + "'");
		update_values.push_back(columns[7] + " = '" + EscapeString(spells_new_entry.cast_on_other) + "'");
		update_values.push_back(columns[8] + " = '" + EscapeString(spells_new_entry.spell_fades) + "'");
		update_values.push_back(columns[9] + " = " + std::to_string(spells_new_entry.range));
		update_values.push_back(columns[10] + " = " + std::to_string(spells_new_entry.aoerange));
		update_values.push_back(columns[11] + " = " + std::to_string(spells_new_entry.pushback));
		update_values.push_back(columns[12] + " = " + std::to_string(spells_new_entry.pushup));
		update_values.push_back(columns[13] + " = " + std::to_string(spells_new_entry.cast_time));
		update_values.push_back(columns[14] + " = " + std::to_string(spells_new_entry.recovery_time));
		update_values.push_back(columns[15] + " = " + std::to_string(spells_new_entry.recast_time));
		update_values.push_back(columns[16] + " = " + std::to_string(spells_new_entry.buffdurationformula));
		update_values.push_back(columns[17] + " = " + std::to_string(spells_new_entry.buffduration));
		update_values.push_back(columns[18] + " = " + std::to_string(spells_new_entry.AEDuration));
		update_values.push_back(columns[19] + " = " + std::to_string(spells_new_entry.mana));
		update_values.push_back(columns[20] + " = " + std::to_string(spells_new_entry.effect_base_value1));
		update_values.push_back(columns[21] + " = " + std::to_string(spells_new_entry.effect_base_value2));
		update_values.push_back(columns[22] + " = " + std::to_string(spells_new_entry.effect_base_value3));
		update_values.push_back(columns[23] + " = " + std::to_string(spells_new_entry.effect_base_value4));
		update_values.push_back(columns[24] + " = " + std::to_string(spells_new_entry.effect_base_value5));
		update_values.push_back(columns[25] + " = " + std::to_string(spells_new_entry.effect_base_value6));
		update_values.push_back(columns[26] + " = " + std::to_string(spells_new_entry.effect_base_value7));
		update_values.push_back(columns[27] + " = " + std::to_string(spells_new_entry.effect_base_value8));
		update_values.push_back(columns[28] + " = " + std::to_string(spells_new_entry.effect_base_value9));
		update_values.push_back(columns[29] + " = " + std::to_string(spells_new_entry.effect_base_value10));
		update_values.push_back(columns[30] + " = " + std::to_string(spells_new_entry.effect_base_value11));
		update_values.push_back(columns[31] + " = " + std::to_string(spells_new_entry.effect_base_value12));
		update_values.push_back(columns[32] + " = " + std::to_string(spells_new_entry.effect_limit_value1));
		update_values.push_back(columns[33] + " = " + std::to_string(spells_new_entry.effect_limit_value2));
		update_values.push_back(columns[34] + " = " + std::to_string(spells_new_entry.effect_limit_value3));
		update_values.push_back(columns[35] + " = " + std::to_string(spells_new_entry.effect_limit_value4));
		update_values.push_back(columns[36] + " = " + std::to_string(spells_new_entry.effect_limit_value5));
		update_values.push_back(columns[37] + " = " + std::to_string(spells_new_entry.effect_limit_value6));
		update_values.push_back(columns[38] + " = " + std::to_string(spells_new_entry.effect_limit_value7));
		update_values.push_back(columns[39] + " = " + std::to_string(spells_new_entry.effect_limit_value8));
		update_values.push_back(columns[40] + " = " + std::to_string(spells_new_entry.effect_limit_value9));
		update_values.push_back(columns[41] + " = " + std::to_string(spells_new_entry.effect_limit_value10));
		update_values.push_back(columns[42] + " = " + std::to_string(spells_new_entry.effect_limit_value11));
		update_values.push_back(columns[43] + " = " + std::to_string(spells_new_entry.effect_limit_value12));
		update_values.push_back(columns[44] + " = " + std::to_string(spells_new_entry.max1));
		update_values.push_back(columns[45] + " = " + std::to_string(spells_new_entry.max2));
		update_values.push_back(columns[46] + " = " + std::to_string(spells_new_entry.max3));
		update_values.push_back(columns[47] + " = " + std::to_string(spells_new_entry.max4));
		update_values.push_back(columns[48] + " = " + std::to_string(spells_new_entry.max5));
		update_values.push_back(columns[49] + " = " + std::to_string(spells_new_entry.max6));
		update_values.push_back(columns[50] + " = " + std::to_string(spells_new_entry.max7));
		update_values.push_back(columns[51] + " = " + std::to_string(spells_new_entry.max8));
		update_values.push_back(columns[52] + " = " + std::to_string(spells_new_entry.max9));
		update_values.push_back(columns[53] + " = " + std::to_string(spells_new_entry.max10));
		update_values.push_back(columns[54] + " = " + std::to_string(spells_new_entry.max11));
		update_values.push_back(columns[55] + " = " + std::to_string(spells_new_entry.max12));
		update_values.push_back(columns[56] + " = " + std::to_string(spells_new_entry.icon));
		update_values.push_back(columns[57] + " = " + std::to_string(spells_new_entry.memicon));
		update_values.push_back(columns[58] + " = " + std::to_string(spells_new_entry.components1));
		update_values.push_back(columns[59] + " = " + std::to_string(spells_new_entry.components2));
		update_values.push_back(columns[60] + " = " + std::to_string(spells_new_entry.components3));
		update_values.push_back(columns[61] + " = " + std::to_string(spells_new_entry.components4));
		update_values.push_back(columns[62] + " = " + std::to_string(spells_new_entry.component_counts1));
		update_values.push_back(columns[63] + " = " + std::to_string(spells_new_entry.component_counts2));
		update_values.push_back(columns[64] + " = " + std::to_string(spells_new_entry.component_counts3));
		update_values.push_back(columns[65] + " = " + std::to_string(spells_new_entry.component_counts4));
		update_values.push_back(columns[66] + " = " + std::to_string(spells_new_entry.NoexpendReagent1));
		update_values.push_back(columns[67] + " = " + std::to_string(spells_new_entry.NoexpendReagent2));
		update_values.push_back(columns[68] + " = " + std::to_string(spells_new_entry.NoexpendReagent3));
		update_values.push_back(columns[69] + " = " + std::to_string(spells_new_entry.NoexpendReagent4));
		update_values.push_back(columns[70] + " = " + std::to_string(spells_new_entry.formula1));
		update_values.push_back(columns[71] + " = " + std::to_string(spells_new_entry.formula2));
		update_values.push_back(columns[72] + " = " + std::to_string(spells_new_entry.formula3));
		update_values.push_back(columns[73] + " = " + std::to_string(spells_new_entry.formula4));
		update_values.push_back(columns[74] + " = " + std::to_string(spells_new_entry.formula5));
		update_values.push_back(columns[75] + " = " + std::to_string(spells_new_entry.formula6));
		update_values.push_back(columns[76] + " = " + std::to_string(spells_new_entry.formula7));
		update_values.push_back(columns[77] + " = " + std::to_string(spells_new_entry.formula8));
		update_values.push_back(columns[78] + " = " + std::to_string(spells_new_entry.formula9));
		update_values.push_back(columns[79] + " = " + std::to_string(spells_new_entry.formula10));
		update_values.push_back(columns[80] + " = " + std::to_string(spells_new_entry.formula11));
		update_values.push_back(columns[81] + " = " + std::to_string(spells_new_entry.formula12));
		update_values.push_back(columns[82] + " = " + std::to_string(spells_new_entry.LightType));
		update_values.push_back(columns[83] + " = " + std::to_string(spells_new_entry.goodEffect));
		update_values.push_back(columns[84] + " = " + std::to_string(spells_new_entry.Activated));
		update_values.push_back(columns[85] + " = " + std::to_string(spells_new_entry.resisttype));
		update_values.push_back(columns[86] + " = " + std::to_string(spells_new_entry.effectid1));
		update_values.push_back(columns[87] + " = " + std::to_string(spells_new_entry.effectid2));
		update_values.push_back(columns[88] + " = " + std::to_string(spells_new_entry.effectid3));
		update_values.push_back(columns[89] + " = " + std::to_string(spells_new_entry.effectid4));
		update_values.push_back(columns[90] + " = " + std::to_string(spells_new_entry.effectid5));
		update_values.push_back(columns[91] + " = " + std::to_string(spells_new_entry.effectid6));
		update_values.push_back(columns[92] + " = " + std::to_string(spells_new_entry.effectid7));
		update_values.push_back(columns[93] + " = " + std::to_string(spells_new_entry.effectid8));
		update_values.push_back(columns[94] + " = " + std::to_string(spells_new_entry.effectid9));
		update_values.push_back(columns[95] + " = " + std::to_string(spells_new_entry.effectid10));
		update_values.push_back(columns[96] + " = " + std::to_string(spells_new_entry.effectid11));
		update_values.push_back(columns[97] + " = " + std::to_string(spells_new_entry.effectid12));
		update_values.push_back(columns[98] + " = " + std::to_string(spells_new_entry.targettype));
		update_values.push_back(columns[99] + " = " + std::to_string(spells_new_entry.basediff));
		update_values.push_back(columns[100] + " = " + std::to_string(spells_new_entry.skill));
		update_values.push_back(columns[101] + " = " + std::to_string(spells_new_entry.zonetype));
		update_values.push_back(columns[102] + " = " + std::to_string(spells_new_entry.EnvironmentType));
		update_values.push_back(columns[103] + " = " + std::to_string(spells_new_entry.TimeOfDay));
		update_values.push_back(columns[104] + " = " + std::to_string(spells_new_entry.classes1));
		update_values.push_back(columns[105] + " = " + std::to_string(spells_new_entry.classes2));
		update_values.push_back(columns[106] + " = " + std::to_string(spells_new_entry.classes3));
		update_values.push_back(columns[107] + " = " + std::to_string(spells_new_entry.classes4));
		update_values.push_back(columns[108] + " = " + std::to_string(spells_new_entry.classes5));
		update_values.push_back(columns[109] + " = " + std::to_string(spells_new_entry.classes6));
		update_values.push_back(columns[110] + " = " + std::to_string(spells_new_entry.classes7));
		update_values.push_back(columns[111] + " = " + std::to_string(spells_new_entry.classes8));
		update_values.push_back(columns[112] + " = " + std::to_string(spells_new_entry.classes9));
		update_values.push_back(columns[113] + " = " + std::to_string(spells_new_entry.classes10));
		update_values.push_back(columns[114] + " = " + std::to_string(spells_new_entry.classes11));
		update_values.push_back(columns[115] + " = " + std::to_string(spells_new_entry.classes12));
		update_values.push_back(columns[116] + " = " + std::to_string(spells_new_entry.classes13));
		update_values.push_back(columns[117] + " = " + std::to_string(spells_new_entry.classes14));
		update_values.push_back(columns[118] + " = " + std::to_string(spells_new_entry.classes15));
		update_values.push_back(columns[119] + " = " + std::to_string(spells_new_entry.classes16));
		update_values.push_back(columns[120] + " = " + std::to_string(spells_new_entry.CastingAnim));
		update_values.push_back(columns[121] + " = " + std::to_string(spells_new_entry.TargetAnim));
		update_values.push_back(columns[122] + " = " + std::to_string(spells_new_entry.TravelType));
		update_values.push_back(columns[123] + " = " + std::to_string(spells_new_entry.SpellAffectIndex));
		update_values.push_back(columns[124] + " = " + std::to_string(spells_new_entry.disallow_sit));
		update_values.push_back(columns[125] + " = " + std::to_string(spells_new_entry.deities0));
		update_values.push_back(columns[126] + " = " + std::to_string(spells_new_entry.deities1));
		update_values.push_back(columns[127] + " = " + std::to_string(spells_new_entry.deities2));
		update_values.push_back(columns[128] + " = " + std::to_string(spells_new_entry.deities3));
		update_values.push_back(columns[129] + " = " + std::to_string(spells_new_entry.deities4));
		update_values.push_back(columns[130] + " = " + std::to_string(spells_new_entry.deities5));
		update_values.push_back(columns[131] + " = " + std::to_string(spells_new_entry.deities6));
		update_values.push_back(columns[132] + " = " + std::to_string(spells_new_entry.deities7));
		update_values.push_back(columns[133] + " = " + std::to_string(spells_new_entry.deities8));
		update_values.push_back(columns[134] + " = " + std::to_string(spells_new_entry.deities9));
		update_values.push_back(columns[135] + " = " + std::to_string(spells_new_entry.deities10));
		update_values.push_back(columns[136] + " = " + std::to_string(spells_new_entry.deities11));
		update_values.push_back(columns[137] + " = " + std::to_string(spells_new_entry.deities12));
		update_values.push_back(columns[138] + " = " + std::to_string(spells_new_entry.deities13));
		update_values.push_back(columns[139] + " = " + std::to_string(spells_new_entry.deities14));
		update_values.push_back(columns[140] + " = " + std::to_string(spells_new_entry.deities15));
		update_values.push_back(columns[141] + " = " + std::to_string(spells_new_entry.deities16));
		update_values.push_back(columns[142] + " = " + std::to_string(spells_new_entry.field142));
		update_values.push_back(columns[143] + " = " + std::to_string(spells_new_entry.field143));
		update_values.push_back(columns[144] + " = " + std::to_string(spells_new_entry.new_icon));
		update_values.push_back(columns[145] + " = " + std::to_string(spells_new_entry.spellanim));
		update_values.push_back(columns[146] + " = " + std::to_string(spells_new_entry.uninterruptable));
		update_values.push_back(columns[147] + " = " + std::to_string(spells_new_entry.ResistDiff));
		update_values.push_back(columns[148] + " = " + std::to_string(spells_new_entry.dot_stacking_exempt));
		update_values.push_back(columns[149] + " = " + std::to_string(spells_new_entry.deleteable));
		update_values.push_back(columns[150] + " = " + std::to_string(spells_new_entry.RecourseLink));
		update_values.push_back(columns[151] + " = " + std::to_string(spells_new_entry.no_partial_resist));
		update_values.push_back(columns[152] + " = " + std::to_string(spells_new_entry.field152));
		update_values.push_back(columns[153] + " = " + std::to_string(spells_new_entry.field153));
		update_values.push_back(columns[154] + " = " + std::to_string(spells_new_entry.short_buff_box));
		update_values.push_back(columns[155] + " = " + std::to_string(spells_new_entry.descnum));
		update_values.push_back(columns[156] + " = " + std::to_string(spells_new_entry.typedescnum));
		update_values.push_back(columns[157] + " = " + std::to_string(spells_new_entry.effectdescnum));
		update_values.push_back(columns[158] + " = " + std::to_string(spells_new_entry.effectdescnum2));
		update_values.push_back(columns[159] + " = " + std::to_string(spells_new_entry.npc_no_los));
		update_values.push_back(columns[160] + " = " + std::to_string(spells_new_entry.field160));
		update_values.push_back(columns[161] + " = " + std::to_string(spells_new_entry.reflectable));
		update_values.push_back(columns[162] + " = " + std::to_string(spells_new_entry.bonushate));
		update_values.push_back(columns[163] + " = " + std::to_string(spells_new_entry.field163));
		update_values.push_back(columns[164] + " = " + std::to_string(spells_new_entry.field164));
		update_values.push_back(columns[165] + " = " + std::to_string(spells_new_entry.ldon_trap));
		update_values.push_back(columns[166] + " = " + std::to_string(spells_new_entry.EndurCost));
		update_values.push_back(columns[167] + " = " + std::to_string(spells_new_entry.EndurTimerIndex));
		update_values.push_back(columns[168] + " = " + std::to_string(spells_new_entry.IsDiscipline));
		update_values.push_back(columns[169] + " = " + std::to_string(spells_new_entry.field169));
		update_values.push_back(columns[170] + " = " + std::to_string(spells_new_entry.field170));
		update_values.push_back(columns[171] + " = " + std::to_string(spells_new_entry.field171));
		update_values.push_back(columns[172] + " = " + std::to_string(spells_new_entry.field172));
		update_values.push_back(columns[173] + " = " + std::to_string(spells_new_entry.HateAdded));
		update_values.push_back(columns[174] + " = " + std::to_string(spells_new_entry.EndurUpkeep));
		update_values.push_back(columns[175] + " = " + std::to_string(spells_new_entry.numhitstype));
		update_values.push_back(columns[176] + " = " + std::to_string(spells_new_entry.numhits));
		update_values.push_back(columns[177] + " = " + std::to_string(spells_new_entry.pvpresistbase));
		update_values.push_back(columns[178] + " = " + std::to_string(spells_new_entry.pvpresistcalc));
		update_values.push_back(columns[179] + " = " + std::to_string(spells_new_entry.pvpresistcap));
		update_values.push_back(columns[180] + " = " + std::to_string(spells_new_entry.spell_category));
		update_values.push_back(columns[181] + " = " + std::to_string(spells_new_entry.field181));
		update_values.push_back(columns[182] + " = " + std::to_string(spells_new_entry.field182));
		update_values.push_back(columns[183] + " = " + std::to_string(spells_new_entry.pcnpc_only_flag));
		update_values.push_back(columns[184] + " = " + std::to_string(spells_new_entry.cast_not_standing));
		update_values.push_back(columns[185] + " = " + std::to_string(spells_new_entry.can_mgb));
		update_values.push_back(columns[186] + " = " + std::to_string(spells_new_entry.nodispell));
		update_values.push_back(columns[187] + " = " + std::to_string(spells_new_entry.npc_category));
		update_values.push_back(columns[188] + " = " + std::to_string(spells_new_entry.npc_usefulness));
		update_values.push_back(columns[189] + " = " + std::to_string(spells_new_entry.MinResist));
		update_values.push_back(columns[190] + " = " + std::to_string(spells_new_entry.MaxResist));
		update_values.push_back(columns[191] + " = " + std::to_string(spells_new_entry.viral_targets));
		update_values.push_back(columns[192] + " = " + std::to_string(spells_new_entry.viral_timer));
		update_values.push_back(columns[193] + " = " + std::to_string(spells_new_entry.nimbuseffect));
		update_values.push_back(columns[194] + " = " + std::to_string(spells_new_entry.ConeStartAngle));
		update_values.push_back(columns[195] + " = " + std::to_string(spells_new_entry.ConeStopAngle));
		update_values.push_back(columns[196] + " = " + std::to_string(spells_new_entry.sneaking));
		update_values.push_back(columns[197] + " = " + std::to_string(spells_new_entry.not_extendable));
		update_values.push_back(columns[198] + " = " + std::to_string(spells_new_entry.field198));
		update_values.push_back(columns[199] + " = " + std::to_string(spells_new_entry.field199));
		update_values.push_back(columns[200] + " = " + std::to_string(spells_new_entry.suspendable));
		update_values.push_back(columns[201] + " = " + std::to_string(spells_new_entry.viral_range));
		update_values.push_back(columns[202] + " = " + std::to_string(spells_new_entry.songcap));
		update_values.push_back(columns[203] + " = " + std::to_string(spells_new_entry.field203));
		update_values.push_back(columns[204] + " = " + std::to_string(spells_new_entry.field204));
		update_values.push_back(columns[205] + " = " + std::to_string(spells_new_entry.no_block));
		update_values.push_back(columns[206] + " = " + std::to_string(spells_new_entry.field206));
		update_values.push_back(columns[207] + " = " + std::to_string(spells_new_entry.spellgroup));
		update_values.push_back(columns[208] + " = " + std::to_string(spells_new_entry.rank));
		update_values.push_back(columns[209] + " = " + std::to_string(spells_new_entry.field209));
		update_values.push_back(columns[210] + " = " + std::to_string(spells_new_entry.field210));
		update_values.push_back(columns[211] + " = " + std::to_string(spells_new_entry.CastRestriction));
		update_values.push_back(columns[212] + " = " + std::to_string(spells_new_entry.allowrest));
		update_values.push_back(columns[213] + " = " + std::to_string(spells_new_entry.InCombat));
		update_values.push_back(columns[214] + " = " + std::to_string(spells_new_entry.OutofCombat));
		update_values.push_back(columns[215] + " = " + std::to_string(spells_new_entry.field215));
		update_values.push_back(columns[216] + " = " + std::to_string(spells_new_entry.field216));
		update_values.push_back(columns[217] + " = " + std::to_string(spells_new_entry.field217));
		update_values.push_back(columns[218] + " = " + std::to_string(spells_new_entry.aemaxtargets));
		update_values.push_back(columns[219] + " = " + std::to_string(spells_new_entry.maxtargets));
		update_values.push_back(columns[220] + " = " + std::to_string(spells_new_entry.field220));
		update_values.push_back(columns[221] + " = " + std::to_string(spells_new_entry.field221));
		update_values.push_back(columns[222] + " = " + std::to_string(spells_new_entry.field222));
		update_values.push_back(columns[223] + " = " + std::to_string(spells_new_entry.field223));
		update_values.push_back(columns[224] + " = " + std::to_string(spells_new_entry.persistdeath));
		update_values.push_back(columns[225] + " = " + std::to_string(spells_new_entry.field225));
		update_values.push_back(columns[226] + " = " + std::to_string(spells_new_entry.field226));
		update_values.push_back(columns[227] + " = " + std::to_string(spells_new_entry.min_dist));
		update_values.push_back(columns[228] + " = " + std::to_string(spells_new_entry.min_dist_mod));
		update_values.push_back(columns[229] + " = " + std::to_string(spells_new_entry.max_dist));
		update_values.push_back(columns[230] + " = " + std::to_string(spells_new_entry.max_dist_mod));
		update_values.push_back(columns[231] + " = " + std::to_string(spells_new_entry.min_range));
		update_values.push_back(columns[232] + " = " + std::to_string(spells_new_entry.field232));
		update_values.push_back(columns[233] + " = " + std::to_string(spells_new_entry.field233));
		update_values.push_back(columns[234] + " = " + std::to_string(spells_new_entry.field234));
		update_values.push_back(columns[235] + " = " + std::to_string(spells_new_entry.field235));
		update_values.push_back(columns[236] + " = " + std::to_string(spells_new_entry.field236));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				spells_new_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static SpellsNew InsertOne(
		Database& db,
		SpellsNew spells_new_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(spells_new_entry.id));
		insert_values.push_back("'" + EscapeString(spells_new_entry.name) + "'");
		insert_values.push_back("'" + EscapeString(spells_new_entry.player_1) + "'");
		insert_values.push_back("'" + EscapeString(spells_new_entry.teleport_zone) + "'");
		insert_values.push_back("'" + EscapeString(spells_new_entry.you_cast) + "'");
		insert_values.push_back("'" + EscapeString(spells_new_entry.other_casts) + "'");
		insert_values.push_back("'" + EscapeString(spells_new_entry.cast_on_you) + "'");
		insert_values.push_back("'" + EscapeString(spells_new_entry.cast_on_other) + "'");
		insert_values.push_back("'" + EscapeString(spells_new_entry.spell_fades) + "'");
		insert_values.push_back(std::to_string(spells_new_entry.range));
		insert_values.push_back(std::to_string(spells_new_entry.aoerange));
		insert_values.push_back(std::to_string(spells_new_entry.pushback));
		insert_values.push_back(std::to_string(spells_new_entry.pushup));
		insert_values.push_back(std::to_string(spells_new_entry.cast_time));
		insert_values.push_back(std::to_string(spells_new_entry.recovery_time));
		insert_values.push_back(std::to_string(spells_new_entry.recast_time));
		insert_values.push_back(std::to_string(spells_new_entry.buffdurationformula));
		insert_values.push_back(std::to_string(spells_new_entry.buffduration));
		insert_values.push_back(std::to_string(spells_new_entry.AEDuration));
		insert_values.push_back(std::to_string(spells_new_entry.mana));
		insert_values.push_back(std::to_string(spells_new_entry.effect_base_value1));
		insert_values.push_back(std::to_string(spells_new_entry.effect_base_value2));
		insert_values.push_back(std::to_string(spells_new_entry.effect_base_value3));
		insert_values.push_back(std::to_string(spells_new_entry.effect_base_value4));
		insert_values.push_back(std::to_string(spells_new_entry.effect_base_value5));
		insert_values.push_back(std::to_string(spells_new_entry.effect_base_value6));
		insert_values.push_back(std::to_string(spells_new_entry.effect_base_value7));
		insert_values.push_back(std::to_string(spells_new_entry.effect_base_value8));
		insert_values.push_back(std::to_string(spells_new_entry.effect_base_value9));
		insert_values.push_back(std::to_string(spells_new_entry.effect_base_value10));
		insert_values.push_back(std::to_string(spells_new_entry.effect_base_value11));
		insert_values.push_back(std::to_string(spells_new_entry.effect_base_value12));
		insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value1));
		insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value2));
		insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value3));
		insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value4));
		insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value5));
		insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value6));
		insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value7));
		insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value8));
		insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value9));
		insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value10));
		insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value11));
		insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value12));
		insert_values.push_back(std::to_string(spells_new_entry.max1));
		insert_values.push_back(std::to_string(spells_new_entry.max2));
		insert_values.push_back(std::to_string(spells_new_entry.max3));
		insert_values.push_back(std::to_string(spells_new_entry.max4));
		insert_values.push_back(std::to_string(spells_new_entry.max5));
		insert_values.push_back(std::to_string(spells_new_entry.max6));
		insert_values.push_back(std::to_string(spells_new_entry.max7));
		insert_values.push_back(std::to_string(spells_new_entry.max8));
		insert_values.push_back(std::to_string(spells_new_entry.max9));
		insert_values.push_back(std::to_string(spells_new_entry.max10));
		insert_values.push_back(std::to_string(spells_new_entry.max11));
		insert_values.push_back(std::to_string(spells_new_entry.max12));
		insert_values.push_back(std::to_string(spells_new_entry.icon));
		insert_values.push_back(std::to_string(spells_new_entry.memicon));
		insert_values.push_back(std::to_string(spells_new_entry.components1));
		insert_values.push_back(std::to_string(spells_new_entry.components2));
		insert_values.push_back(std::to_string(spells_new_entry.components3));
		insert_values.push_back(std::to_string(spells_new_entry.components4));
		insert_values.push_back(std::to_string(spells_new_entry.component_counts1));
		insert_values.push_back(std::to_string(spells_new_entry.component_counts2));
		insert_values.push_back(std::to_string(spells_new_entry.component_counts3));
		insert_values.push_back(std::to_string(spells_new_entry.component_counts4));
		insert_values.push_back(std::to_string(spells_new_entry.NoexpendReagent1));
		insert_values.push_back(std::to_string(spells_new_entry.NoexpendReagent2));
		insert_values.push_back(std::to_string(spells_new_entry.NoexpendReagent3));
		insert_values.push_back(std::to_string(spells_new_entry.NoexpendReagent4));
		insert_values.push_back(std::to_string(spells_new_entry.formula1));
		insert_values.push_back(std::to_string(spells_new_entry.formula2));
		insert_values.push_back(std::to_string(spells_new_entry.formula3));
		insert_values.push_back(std::to_string(spells_new_entry.formula4));
		insert_values.push_back(std::to_string(spells_new_entry.formula5));
		insert_values.push_back(std::to_string(spells_new_entry.formula6));
		insert_values.push_back(std::to_string(spells_new_entry.formula7));
		insert_values.push_back(std::to_string(spells_new_entry.formula8));
		insert_values.push_back(std::to_string(spells_new_entry.formula9));
		insert_values.push_back(std::to_string(spells_new_entry.formula10));
		insert_values.push_back(std::to_string(spells_new_entry.formula11));
		insert_values.push_back(std::to_string(spells_new_entry.formula12));
		insert_values.push_back(std::to_string(spells_new_entry.LightType));
		insert_values.push_back(std::to_string(spells_new_entry.goodEffect));
		insert_values.push_back(std::to_string(spells_new_entry.Activated));
		insert_values.push_back(std::to_string(spells_new_entry.resisttype));
		insert_values.push_back(std::to_string(spells_new_entry.effectid1));
		insert_values.push_back(std::to_string(spells_new_entry.effectid2));
		insert_values.push_back(std::to_string(spells_new_entry.effectid3));
		insert_values.push_back(std::to_string(spells_new_entry.effectid4));
		insert_values.push_back(std::to_string(spells_new_entry.effectid5));
		insert_values.push_back(std::to_string(spells_new_entry.effectid6));
		insert_values.push_back(std::to_string(spells_new_entry.effectid7));
		insert_values.push_back(std::to_string(spells_new_entry.effectid8));
		insert_values.push_back(std::to_string(spells_new_entry.effectid9));
		insert_values.push_back(std::to_string(spells_new_entry.effectid10));
		insert_values.push_back(std::to_string(spells_new_entry.effectid11));
		insert_values.push_back(std::to_string(spells_new_entry.effectid12));
		insert_values.push_back(std::to_string(spells_new_entry.targettype));
		insert_values.push_back(std::to_string(spells_new_entry.basediff));
		insert_values.push_back(std::to_string(spells_new_entry.skill));
		insert_values.push_back(std::to_string(spells_new_entry.zonetype));
		insert_values.push_back(std::to_string(spells_new_entry.EnvironmentType));
		insert_values.push_back(std::to_string(spells_new_entry.TimeOfDay));
		insert_values.push_back(std::to_string(spells_new_entry.classes1));
		insert_values.push_back(std::to_string(spells_new_entry.classes2));
		insert_values.push_back(std::to_string(spells_new_entry.classes3));
		insert_values.push_back(std::to_string(spells_new_entry.classes4));
		insert_values.push_back(std::to_string(spells_new_entry.classes5));
		insert_values.push_back(std::to_string(spells_new_entry.classes6));
		insert_values.push_back(std::to_string(spells_new_entry.classes7));
		insert_values.push_back(std::to_string(spells_new_entry.classes8));
		insert_values.push_back(std::to_string(spells_new_entry.classes9));
		insert_values.push_back(std::to_string(spells_new_entry.classes10));
		insert_values.push_back(std::to_string(spells_new_entry.classes11));
		insert_values.push_back(std::to_string(spells_new_entry.classes12));
		insert_values.push_back(std::to_string(spells_new_entry.classes13));
		insert_values.push_back(std::to_string(spells_new_entry.classes14));
		insert_values.push_back(std::to_string(spells_new_entry.classes15));
		insert_values.push_back(std::to_string(spells_new_entry.classes16));
		insert_values.push_back(std::to_string(spells_new_entry.CastingAnim));
		insert_values.push_back(std::to_string(spells_new_entry.TargetAnim));
		insert_values.push_back(std::to_string(spells_new_entry.TravelType));
		insert_values.push_back(std::to_string(spells_new_entry.SpellAffectIndex));
		insert_values.push_back(std::to_string(spells_new_entry.disallow_sit));
		insert_values.push_back(std::to_string(spells_new_entry.deities0));
		insert_values.push_back(std::to_string(spells_new_entry.deities1));
		insert_values.push_back(std::to_string(spells_new_entry.deities2));
		insert_values.push_back(std::to_string(spells_new_entry.deities3));
		insert_values.push_back(std::to_string(spells_new_entry.deities4));
		insert_values.push_back(std::to_string(spells_new_entry.deities5));
		insert_values.push_back(std::to_string(spells_new_entry.deities6));
		insert_values.push_back(std::to_string(spells_new_entry.deities7));
		insert_values.push_back(std::to_string(spells_new_entry.deities8));
		insert_values.push_back(std::to_string(spells_new_entry.deities9));
		insert_values.push_back(std::to_string(spells_new_entry.deities10));
		insert_values.push_back(std::to_string(spells_new_entry.deities11));
		insert_values.push_back(std::to_string(spells_new_entry.deities12));
		insert_values.push_back(std::to_string(spells_new_entry.deities13));
		insert_values.push_back(std::to_string(spells_new_entry.deities14));
		insert_values.push_back(std::to_string(spells_new_entry.deities15));
		insert_values.push_back(std::to_string(spells_new_entry.deities16));
		insert_values.push_back(std::to_string(spells_new_entry.field142));
		insert_values.push_back(std::to_string(spells_new_entry.field143));
		insert_values.push_back(std::to_string(spells_new_entry.new_icon));
		insert_values.push_back(std::to_string(spells_new_entry.spellanim));
		insert_values.push_back(std::to_string(spells_new_entry.uninterruptable));
		insert_values.push_back(std::to_string(spells_new_entry.ResistDiff));
		insert_values.push_back(std::to_string(spells_new_entry.dot_stacking_exempt));
		insert_values.push_back(std::to_string(spells_new_entry.deleteable));
		insert_values.push_back(std::to_string(spells_new_entry.RecourseLink));
		insert_values.push_back(std::to_string(spells_new_entry.no_partial_resist));
		insert_values.push_back(std::to_string(spells_new_entry.field152));
		insert_values.push_back(std::to_string(spells_new_entry.field153));
		insert_values.push_back(std::to_string(spells_new_entry.short_buff_box));
		insert_values.push_back(std::to_string(spells_new_entry.descnum));
		insert_values.push_back(std::to_string(spells_new_entry.typedescnum));
		insert_values.push_back(std::to_string(spells_new_entry.effectdescnum));
		insert_values.push_back(std::to_string(spells_new_entry.effectdescnum2));
		insert_values.push_back(std::to_string(spells_new_entry.npc_no_los));
		insert_values.push_back(std::to_string(spells_new_entry.field160));
		insert_values.push_back(std::to_string(spells_new_entry.reflectable));
		insert_values.push_back(std::to_string(spells_new_entry.bonushate));
		insert_values.push_back(std::to_string(spells_new_entry.field163));
		insert_values.push_back(std::to_string(spells_new_entry.field164));
		insert_values.push_back(std::to_string(spells_new_entry.ldon_trap));
		insert_values.push_back(std::to_string(spells_new_entry.EndurCost));
		insert_values.push_back(std::to_string(spells_new_entry.EndurTimerIndex));
		insert_values.push_back(std::to_string(spells_new_entry.IsDiscipline));
		insert_values.push_back(std::to_string(spells_new_entry.field169));
		insert_values.push_back(std::to_string(spells_new_entry.field170));
		insert_values.push_back(std::to_string(spells_new_entry.field171));
		insert_values.push_back(std::to_string(spells_new_entry.field172));
		insert_values.push_back(std::to_string(spells_new_entry.HateAdded));
		insert_values.push_back(std::to_string(spells_new_entry.EndurUpkeep));
		insert_values.push_back(std::to_string(spells_new_entry.numhitstype));
		insert_values.push_back(std::to_string(spells_new_entry.numhits));
		insert_values.push_back(std::to_string(spells_new_entry.pvpresistbase));
		insert_values.push_back(std::to_string(spells_new_entry.pvpresistcalc));
		insert_values.push_back(std::to_string(spells_new_entry.pvpresistcap));
		insert_values.push_back(std::to_string(spells_new_entry.spell_category));
		insert_values.push_back(std::to_string(spells_new_entry.field181));
		insert_values.push_back(std::to_string(spells_new_entry.field182));
		insert_values.push_back(std::to_string(spells_new_entry.pcnpc_only_flag));
		insert_values.push_back(std::to_string(spells_new_entry.cast_not_standing));
		insert_values.push_back(std::to_string(spells_new_entry.can_mgb));
		insert_values.push_back(std::to_string(spells_new_entry.nodispell));
		insert_values.push_back(std::to_string(spells_new_entry.npc_category));
		insert_values.push_back(std::to_string(spells_new_entry.npc_usefulness));
		insert_values.push_back(std::to_string(spells_new_entry.MinResist));
		insert_values.push_back(std::to_string(spells_new_entry.MaxResist));
		insert_values.push_back(std::to_string(spells_new_entry.viral_targets));
		insert_values.push_back(std::to_string(spells_new_entry.viral_timer));
		insert_values.push_back(std::to_string(spells_new_entry.nimbuseffect));
		insert_values.push_back(std::to_string(spells_new_entry.ConeStartAngle));
		insert_values.push_back(std::to_string(spells_new_entry.ConeStopAngle));
		insert_values.push_back(std::to_string(spells_new_entry.sneaking));
		insert_values.push_back(std::to_string(spells_new_entry.not_extendable));
		insert_values.push_back(std::to_string(spells_new_entry.field198));
		insert_values.push_back(std::to_string(spells_new_entry.field199));
		insert_values.push_back(std::to_string(spells_new_entry.suspendable));
		insert_values.push_back(std::to_string(spells_new_entry.viral_range));
		insert_values.push_back(std::to_string(spells_new_entry.songcap));
		insert_values.push_back(std::to_string(spells_new_entry.field203));
		insert_values.push_back(std::to_string(spells_new_entry.field204));
		insert_values.push_back(std::to_string(spells_new_entry.no_block));
		insert_values.push_back(std::to_string(spells_new_entry.field206));
		insert_values.push_back(std::to_string(spells_new_entry.spellgroup));
		insert_values.push_back(std::to_string(spells_new_entry.rank));
		insert_values.push_back(std::to_string(spells_new_entry.field209));
		insert_values.push_back(std::to_string(spells_new_entry.field210));
		insert_values.push_back(std::to_string(spells_new_entry.CastRestriction));
		insert_values.push_back(std::to_string(spells_new_entry.allowrest));
		insert_values.push_back(std::to_string(spells_new_entry.InCombat));
		insert_values.push_back(std::to_string(spells_new_entry.OutofCombat));
		insert_values.push_back(std::to_string(spells_new_entry.field215));
		insert_values.push_back(std::to_string(spells_new_entry.field216));
		insert_values.push_back(std::to_string(spells_new_entry.field217));
		insert_values.push_back(std::to_string(spells_new_entry.aemaxtargets));
		insert_values.push_back(std::to_string(spells_new_entry.maxtargets));
		insert_values.push_back(std::to_string(spells_new_entry.field220));
		insert_values.push_back(std::to_string(spells_new_entry.field221));
		insert_values.push_back(std::to_string(spells_new_entry.field222));
		insert_values.push_back(std::to_string(spells_new_entry.field223));
		insert_values.push_back(std::to_string(spells_new_entry.persistdeath));
		insert_values.push_back(std::to_string(spells_new_entry.field225));
		insert_values.push_back(std::to_string(spells_new_entry.field226));
		insert_values.push_back(std::to_string(spells_new_entry.min_dist));
		insert_values.push_back(std::to_string(spells_new_entry.min_dist_mod));
		insert_values.push_back(std::to_string(spells_new_entry.max_dist));
		insert_values.push_back(std::to_string(spells_new_entry.max_dist_mod));
		insert_values.push_back(std::to_string(spells_new_entry.min_range));
		insert_values.push_back(std::to_string(spells_new_entry.field232));
		insert_values.push_back(std::to_string(spells_new_entry.field233));
		insert_values.push_back(std::to_string(spells_new_entry.field234));
		insert_values.push_back(std::to_string(spells_new_entry.field235));
		insert_values.push_back(std::to_string(spells_new_entry.field236));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			spells_new_entry.id = results.LastInsertedID();
			return spells_new_entry;
		}

		spells_new_entry = NewEntity();

		return spells_new_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<SpellsNew> spells_new_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &spells_new_entry: spells_new_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(spells_new_entry.id));
			insert_values.push_back("'" + EscapeString(spells_new_entry.name) + "'");
			insert_values.push_back("'" + EscapeString(spells_new_entry.player_1) + "'");
			insert_values.push_back("'" + EscapeString(spells_new_entry.teleport_zone) + "'");
			insert_values.push_back("'" + EscapeString(spells_new_entry.you_cast) + "'");
			insert_values.push_back("'" + EscapeString(spells_new_entry.other_casts) + "'");
			insert_values.push_back("'" + EscapeString(spells_new_entry.cast_on_you) + "'");
			insert_values.push_back("'" + EscapeString(spells_new_entry.cast_on_other) + "'");
			insert_values.push_back("'" + EscapeString(spells_new_entry.spell_fades) + "'");
			insert_values.push_back(std::to_string(spells_new_entry.range));
			insert_values.push_back(std::to_string(spells_new_entry.aoerange));
			insert_values.push_back(std::to_string(spells_new_entry.pushback));
			insert_values.push_back(std::to_string(spells_new_entry.pushup));
			insert_values.push_back(std::to_string(spells_new_entry.cast_time));
			insert_values.push_back(std::to_string(spells_new_entry.recovery_time));
			insert_values.push_back(std::to_string(spells_new_entry.recast_time));
			insert_values.push_back(std::to_string(spells_new_entry.buffdurationformula));
			insert_values.push_back(std::to_string(spells_new_entry.buffduration));
			insert_values.push_back(std::to_string(spells_new_entry.AEDuration));
			insert_values.push_back(std::to_string(spells_new_entry.mana));
			insert_values.push_back(std::to_string(spells_new_entry.effect_base_value1));
			insert_values.push_back(std::to_string(spells_new_entry.effect_base_value2));
			insert_values.push_back(std::to_string(spells_new_entry.effect_base_value3));
			insert_values.push_back(std::to_string(spells_new_entry.effect_base_value4));
			insert_values.push_back(std::to_string(spells_new_entry.effect_base_value5));
			insert_values.push_back(std::to_string(spells_new_entry.effect_base_value6));
			insert_values.push_back(std::to_string(spells_new_entry.effect_base_value7));
			insert_values.push_back(std::to_string(spells_new_entry.effect_base_value8));
			insert_values.push_back(std::to_string(spells_new_entry.effect_base_value9));
			insert_values.push_back(std::to_string(spells_new_entry.effect_base_value10));
			insert_values.push_back(std::to_string(spells_new_entry.effect_base_value11));
			insert_values.push_back(std::to_string(spells_new_entry.effect_base_value12));
			insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value1));
			insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value2));
			insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value3));
			insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value4));
			insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value5));
			insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value6));
			insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value7));
			insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value8));
			insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value9));
			insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value10));
			insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value11));
			insert_values.push_back(std::to_string(spells_new_entry.effect_limit_value12));
			insert_values.push_back(std::to_string(spells_new_entry.max1));
			insert_values.push_back(std::to_string(spells_new_entry.max2));
			insert_values.push_back(std::to_string(spells_new_entry.max3));
			insert_values.push_back(std::to_string(spells_new_entry.max4));
			insert_values.push_back(std::to_string(spells_new_entry.max5));
			insert_values.push_back(std::to_string(spells_new_entry.max6));
			insert_values.push_back(std::to_string(spells_new_entry.max7));
			insert_values.push_back(std::to_string(spells_new_entry.max8));
			insert_values.push_back(std::to_string(spells_new_entry.max9));
			insert_values.push_back(std::to_string(spells_new_entry.max10));
			insert_values.push_back(std::to_string(spells_new_entry.max11));
			insert_values.push_back(std::to_string(spells_new_entry.max12));
			insert_values.push_back(std::to_string(spells_new_entry.icon));
			insert_values.push_back(std::to_string(spells_new_entry.memicon));
			insert_values.push_back(std::to_string(spells_new_entry.components1));
			insert_values.push_back(std::to_string(spells_new_entry.components2));
			insert_values.push_back(std::to_string(spells_new_entry.components3));
			insert_values.push_back(std::to_string(spells_new_entry.components4));
			insert_values.push_back(std::to_string(spells_new_entry.component_counts1));
			insert_values.push_back(std::to_string(spells_new_entry.component_counts2));
			insert_values.push_back(std::to_string(spells_new_entry.component_counts3));
			insert_values.push_back(std::to_string(spells_new_entry.component_counts4));
			insert_values.push_back(std::to_string(spells_new_entry.NoexpendReagent1));
			insert_values.push_back(std::to_string(spells_new_entry.NoexpendReagent2));
			insert_values.push_back(std::to_string(spells_new_entry.NoexpendReagent3));
			insert_values.push_back(std::to_string(spells_new_entry.NoexpendReagent4));
			insert_values.push_back(std::to_string(spells_new_entry.formula1));
			insert_values.push_back(std::to_string(spells_new_entry.formula2));
			insert_values.push_back(std::to_string(spells_new_entry.formula3));
			insert_values.push_back(std::to_string(spells_new_entry.formula4));
			insert_values.push_back(std::to_string(spells_new_entry.formula5));
			insert_values.push_back(std::to_string(spells_new_entry.formula6));
			insert_values.push_back(std::to_string(spells_new_entry.formula7));
			insert_values.push_back(std::to_string(spells_new_entry.formula8));
			insert_values.push_back(std::to_string(spells_new_entry.formula9));
			insert_values.push_back(std::to_string(spells_new_entry.formula10));
			insert_values.push_back(std::to_string(spells_new_entry.formula11));
			insert_values.push_back(std::to_string(spells_new_entry.formula12));
			insert_values.push_back(std::to_string(spells_new_entry.LightType));
			insert_values.push_back(std::to_string(spells_new_entry.goodEffect));
			insert_values.push_back(std::to_string(spells_new_entry.Activated));
			insert_values.push_back(std::to_string(spells_new_entry.resisttype));
			insert_values.push_back(std::to_string(spells_new_entry.effectid1));
			insert_values.push_back(std::to_string(spells_new_entry.effectid2));
			insert_values.push_back(std::to_string(spells_new_entry.effectid3));
			insert_values.push_back(std::to_string(spells_new_entry.effectid4));
			insert_values.push_back(std::to_string(spells_new_entry.effectid5));
			insert_values.push_back(std::to_string(spells_new_entry.effectid6));
			insert_values.push_back(std::to_string(spells_new_entry.effectid7));
			insert_values.push_back(std::to_string(spells_new_entry.effectid8));
			insert_values.push_back(std::to_string(spells_new_entry.effectid9));
			insert_values.push_back(std::to_string(spells_new_entry.effectid10));
			insert_values.push_back(std::to_string(spells_new_entry.effectid11));
			insert_values.push_back(std::to_string(spells_new_entry.effectid12));
			insert_values.push_back(std::to_string(spells_new_entry.targettype));
			insert_values.push_back(std::to_string(spells_new_entry.basediff));
			insert_values.push_back(std::to_string(spells_new_entry.skill));
			insert_values.push_back(std::to_string(spells_new_entry.zonetype));
			insert_values.push_back(std::to_string(spells_new_entry.EnvironmentType));
			insert_values.push_back(std::to_string(spells_new_entry.TimeOfDay));
			insert_values.push_back(std::to_string(spells_new_entry.classes1));
			insert_values.push_back(std::to_string(spells_new_entry.classes2));
			insert_values.push_back(std::to_string(spells_new_entry.classes3));
			insert_values.push_back(std::to_string(spells_new_entry.classes4));
			insert_values.push_back(std::to_string(spells_new_entry.classes5));
			insert_values.push_back(std::to_string(spells_new_entry.classes6));
			insert_values.push_back(std::to_string(spells_new_entry.classes7));
			insert_values.push_back(std::to_string(spells_new_entry.classes8));
			insert_values.push_back(std::to_string(spells_new_entry.classes9));
			insert_values.push_back(std::to_string(spells_new_entry.classes10));
			insert_values.push_back(std::to_string(spells_new_entry.classes11));
			insert_values.push_back(std::to_string(spells_new_entry.classes12));
			insert_values.push_back(std::to_string(spells_new_entry.classes13));
			insert_values.push_back(std::to_string(spells_new_entry.classes14));
			insert_values.push_back(std::to_string(spells_new_entry.classes15));
			insert_values.push_back(std::to_string(spells_new_entry.classes16));
			insert_values.push_back(std::to_string(spells_new_entry.CastingAnim));
			insert_values.push_back(std::to_string(spells_new_entry.TargetAnim));
			insert_values.push_back(std::to_string(spells_new_entry.TravelType));
			insert_values.push_back(std::to_string(spells_new_entry.SpellAffectIndex));
			insert_values.push_back(std::to_string(spells_new_entry.disallow_sit));
			insert_values.push_back(std::to_string(spells_new_entry.deities0));
			insert_values.push_back(std::to_string(spells_new_entry.deities1));
			insert_values.push_back(std::to_string(spells_new_entry.deities2));
			insert_values.push_back(std::to_string(spells_new_entry.deities3));
			insert_values.push_back(std::to_string(spells_new_entry.deities4));
			insert_values.push_back(std::to_string(spells_new_entry.deities5));
			insert_values.push_back(std::to_string(spells_new_entry.deities6));
			insert_values.push_back(std::to_string(spells_new_entry.deities7));
			insert_values.push_back(std::to_string(spells_new_entry.deities8));
			insert_values.push_back(std::to_string(spells_new_entry.deities9));
			insert_values.push_back(std::to_string(spells_new_entry.deities10));
			insert_values.push_back(std::to_string(spells_new_entry.deities11));
			insert_values.push_back(std::to_string(spells_new_entry.deities12));
			insert_values.push_back(std::to_string(spells_new_entry.deities13));
			insert_values.push_back(std::to_string(spells_new_entry.deities14));
			insert_values.push_back(std::to_string(spells_new_entry.deities15));
			insert_values.push_back(std::to_string(spells_new_entry.deities16));
			insert_values.push_back(std::to_string(spells_new_entry.field142));
			insert_values.push_back(std::to_string(spells_new_entry.field143));
			insert_values.push_back(std::to_string(spells_new_entry.new_icon));
			insert_values.push_back(std::to_string(spells_new_entry.spellanim));
			insert_values.push_back(std::to_string(spells_new_entry.uninterruptable));
			insert_values.push_back(std::to_string(spells_new_entry.ResistDiff));
			insert_values.push_back(std::to_string(spells_new_entry.dot_stacking_exempt));
			insert_values.push_back(std::to_string(spells_new_entry.deleteable));
			insert_values.push_back(std::to_string(spells_new_entry.RecourseLink));
			insert_values.push_back(std::to_string(spells_new_entry.no_partial_resist));
			insert_values.push_back(std::to_string(spells_new_entry.field152));
			insert_values.push_back(std::to_string(spells_new_entry.field153));
			insert_values.push_back(std::to_string(spells_new_entry.short_buff_box));
			insert_values.push_back(std::to_string(spells_new_entry.descnum));
			insert_values.push_back(std::to_string(spells_new_entry.typedescnum));
			insert_values.push_back(std::to_string(spells_new_entry.effectdescnum));
			insert_values.push_back(std::to_string(spells_new_entry.effectdescnum2));
			insert_values.push_back(std::to_string(spells_new_entry.npc_no_los));
			insert_values.push_back(std::to_string(spells_new_entry.field160));
			insert_values.push_back(std::to_string(spells_new_entry.reflectable));
			insert_values.push_back(std::to_string(spells_new_entry.bonushate));
			insert_values.push_back(std::to_string(spells_new_entry.field163));
			insert_values.push_back(std::to_string(spells_new_entry.field164));
			insert_values.push_back(std::to_string(spells_new_entry.ldon_trap));
			insert_values.push_back(std::to_string(spells_new_entry.EndurCost));
			insert_values.push_back(std::to_string(spells_new_entry.EndurTimerIndex));
			insert_values.push_back(std::to_string(spells_new_entry.IsDiscipline));
			insert_values.push_back(std::to_string(spells_new_entry.field169));
			insert_values.push_back(std::to_string(spells_new_entry.field170));
			insert_values.push_back(std::to_string(spells_new_entry.field171));
			insert_values.push_back(std::to_string(spells_new_entry.field172));
			insert_values.push_back(std::to_string(spells_new_entry.HateAdded));
			insert_values.push_back(std::to_string(spells_new_entry.EndurUpkeep));
			insert_values.push_back(std::to_string(spells_new_entry.numhitstype));
			insert_values.push_back(std::to_string(spells_new_entry.numhits));
			insert_values.push_back(std::to_string(spells_new_entry.pvpresistbase));
			insert_values.push_back(std::to_string(spells_new_entry.pvpresistcalc));
			insert_values.push_back(std::to_string(spells_new_entry.pvpresistcap));
			insert_values.push_back(std::to_string(spells_new_entry.spell_category));
			insert_values.push_back(std::to_string(spells_new_entry.field181));
			insert_values.push_back(std::to_string(spells_new_entry.field182));
			insert_values.push_back(std::to_string(spells_new_entry.pcnpc_only_flag));
			insert_values.push_back(std::to_string(spells_new_entry.cast_not_standing));
			insert_values.push_back(std::to_string(spells_new_entry.can_mgb));
			insert_values.push_back(std::to_string(spells_new_entry.nodispell));
			insert_values.push_back(std::to_string(spells_new_entry.npc_category));
			insert_values.push_back(std::to_string(spells_new_entry.npc_usefulness));
			insert_values.push_back(std::to_string(spells_new_entry.MinResist));
			insert_values.push_back(std::to_string(spells_new_entry.MaxResist));
			insert_values.push_back(std::to_string(spells_new_entry.viral_targets));
			insert_values.push_back(std::to_string(spells_new_entry.viral_timer));
			insert_values.push_back(std::to_string(spells_new_entry.nimbuseffect));
			insert_values.push_back(std::to_string(spells_new_entry.ConeStartAngle));
			insert_values.push_back(std::to_string(spells_new_entry.ConeStopAngle));
			insert_values.push_back(std::to_string(spells_new_entry.sneaking));
			insert_values.push_back(std::to_string(spells_new_entry.not_extendable));
			insert_values.push_back(std::to_string(spells_new_entry.field198));
			insert_values.push_back(std::to_string(spells_new_entry.field199));
			insert_values.push_back(std::to_string(spells_new_entry.suspendable));
			insert_values.push_back(std::to_string(spells_new_entry.viral_range));
			insert_values.push_back(std::to_string(spells_new_entry.songcap));
			insert_values.push_back(std::to_string(spells_new_entry.field203));
			insert_values.push_back(std::to_string(spells_new_entry.field204));
			insert_values.push_back(std::to_string(spells_new_entry.no_block));
			insert_values.push_back(std::to_string(spells_new_entry.field206));
			insert_values.push_back(std::to_string(spells_new_entry.spellgroup));
			insert_values.push_back(std::to_string(spells_new_entry.rank));
			insert_values.push_back(std::to_string(spells_new_entry.field209));
			insert_values.push_back(std::to_string(spells_new_entry.field210));
			insert_values.push_back(std::to_string(spells_new_entry.CastRestriction));
			insert_values.push_back(std::to_string(spells_new_entry.allowrest));
			insert_values.push_back(std::to_string(spells_new_entry.InCombat));
			insert_values.push_back(std::to_string(spells_new_entry.OutofCombat));
			insert_values.push_back(std::to_string(spells_new_entry.field215));
			insert_values.push_back(std::to_string(spells_new_entry.field216));
			insert_values.push_back(std::to_string(spells_new_entry.field217));
			insert_values.push_back(std::to_string(spells_new_entry.aemaxtargets));
			insert_values.push_back(std::to_string(spells_new_entry.maxtargets));
			insert_values.push_back(std::to_string(spells_new_entry.field220));
			insert_values.push_back(std::to_string(spells_new_entry.field221));
			insert_values.push_back(std::to_string(spells_new_entry.field222));
			insert_values.push_back(std::to_string(spells_new_entry.field223));
			insert_values.push_back(std::to_string(spells_new_entry.persistdeath));
			insert_values.push_back(std::to_string(spells_new_entry.field225));
			insert_values.push_back(std::to_string(spells_new_entry.field226));
			insert_values.push_back(std::to_string(spells_new_entry.min_dist));
			insert_values.push_back(std::to_string(spells_new_entry.min_dist_mod));
			insert_values.push_back(std::to_string(spells_new_entry.max_dist));
			insert_values.push_back(std::to_string(spells_new_entry.max_dist_mod));
			insert_values.push_back(std::to_string(spells_new_entry.min_range));
			insert_values.push_back(std::to_string(spells_new_entry.field232));
			insert_values.push_back(std::to_string(spells_new_entry.field233));
			insert_values.push_back(std::to_string(spells_new_entry.field234));
			insert_values.push_back(std::to_string(spells_new_entry.field235));
			insert_values.push_back(std::to_string(spells_new_entry.field236));

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

	static std::vector<SpellsNew> All(Database& db)
	{
		std::vector<SpellsNew> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpellsNew entry{};

			entry.id                   = atoi(row[0]);
			entry.name                 = row[1] ? row[1] : "";
			entry.player_1             = row[2] ? row[2] : "";
			entry.teleport_zone        = row[3] ? row[3] : "";
			entry.you_cast             = row[4] ? row[4] : "";
			entry.other_casts          = row[5] ? row[5] : "";
			entry.cast_on_you          = row[6] ? row[6] : "";
			entry.cast_on_other        = row[7] ? row[7] : "";
			entry.spell_fades          = row[8] ? row[8] : "";
			entry.range                = atoi(row[9]);
			entry.aoerange             = atoi(row[10]);
			entry.pushback             = atoi(row[11]);
			entry.pushup               = atoi(row[12]);
			entry.cast_time            = atoi(row[13]);
			entry.recovery_time        = atoi(row[14]);
			entry.recast_time          = atoi(row[15]);
			entry.buffdurationformula  = atoi(row[16]);
			entry.buffduration         = atoi(row[17]);
			entry.AEDuration           = atoi(row[18]);
			entry.mana                 = atoi(row[19]);
			entry.effect_base_value1   = atoi(row[20]);
			entry.effect_base_value2   = atoi(row[21]);
			entry.effect_base_value3   = atoi(row[22]);
			entry.effect_base_value4   = atoi(row[23]);
			entry.effect_base_value5   = atoi(row[24]);
			entry.effect_base_value6   = atoi(row[25]);
			entry.effect_base_value7   = atoi(row[26]);
			entry.effect_base_value8   = atoi(row[27]);
			entry.effect_base_value9   = atoi(row[28]);
			entry.effect_base_value10  = atoi(row[29]);
			entry.effect_base_value11  = atoi(row[30]);
			entry.effect_base_value12  = atoi(row[31]);
			entry.effect_limit_value1  = atoi(row[32]);
			entry.effect_limit_value2  = atoi(row[33]);
			entry.effect_limit_value3  = atoi(row[34]);
			entry.effect_limit_value4  = atoi(row[35]);
			entry.effect_limit_value5  = atoi(row[36]);
			entry.effect_limit_value6  = atoi(row[37]);
			entry.effect_limit_value7  = atoi(row[38]);
			entry.effect_limit_value8  = atoi(row[39]);
			entry.effect_limit_value9  = atoi(row[40]);
			entry.effect_limit_value10 = atoi(row[41]);
			entry.effect_limit_value11 = atoi(row[42]);
			entry.effect_limit_value12 = atoi(row[43]);
			entry.max1                 = atoi(row[44]);
			entry.max2                 = atoi(row[45]);
			entry.max3                 = atoi(row[46]);
			entry.max4                 = atoi(row[47]);
			entry.max5                 = atoi(row[48]);
			entry.max6                 = atoi(row[49]);
			entry.max7                 = atoi(row[50]);
			entry.max8                 = atoi(row[51]);
			entry.max9                 = atoi(row[52]);
			entry.max10                = atoi(row[53]);
			entry.max11                = atoi(row[54]);
			entry.max12                = atoi(row[55]);
			entry.icon                 = atoi(row[56]);
			entry.memicon              = atoi(row[57]);
			entry.components1          = atoi(row[58]);
			entry.components2          = atoi(row[59]);
			entry.components3          = atoi(row[60]);
			entry.components4          = atoi(row[61]);
			entry.component_counts1    = atoi(row[62]);
			entry.component_counts2    = atoi(row[63]);
			entry.component_counts3    = atoi(row[64]);
			entry.component_counts4    = atoi(row[65]);
			entry.NoexpendReagent1     = atoi(row[66]);
			entry.NoexpendReagent2     = atoi(row[67]);
			entry.NoexpendReagent3     = atoi(row[68]);
			entry.NoexpendReagent4     = atoi(row[69]);
			entry.formula1             = atoi(row[70]);
			entry.formula2             = atoi(row[71]);
			entry.formula3             = atoi(row[72]);
			entry.formula4             = atoi(row[73]);
			entry.formula5             = atoi(row[74]);
			entry.formula6             = atoi(row[75]);
			entry.formula7             = atoi(row[76]);
			entry.formula8             = atoi(row[77]);
			entry.formula9             = atoi(row[78]);
			entry.formula10            = atoi(row[79]);
			entry.formula11            = atoi(row[80]);
			entry.formula12            = atoi(row[81]);
			entry.LightType            = atoi(row[82]);
			entry.goodEffect           = atoi(row[83]);
			entry.Activated            = atoi(row[84]);
			entry.resisttype           = atoi(row[85]);
			entry.effectid1            = atoi(row[86]);
			entry.effectid2            = atoi(row[87]);
			entry.effectid3            = atoi(row[88]);
			entry.effectid4            = atoi(row[89]);
			entry.effectid5            = atoi(row[90]);
			entry.effectid6            = atoi(row[91]);
			entry.effectid7            = atoi(row[92]);
			entry.effectid8            = atoi(row[93]);
			entry.effectid9            = atoi(row[94]);
			entry.effectid10           = atoi(row[95]);
			entry.effectid11           = atoi(row[96]);
			entry.effectid12           = atoi(row[97]);
			entry.targettype           = atoi(row[98]);
			entry.basediff             = atoi(row[99]);
			entry.skill                = atoi(row[100]);
			entry.zonetype             = atoi(row[101]);
			entry.EnvironmentType      = atoi(row[102]);
			entry.TimeOfDay            = atoi(row[103]);
			entry.classes1             = atoi(row[104]);
			entry.classes2             = atoi(row[105]);
			entry.classes3             = atoi(row[106]);
			entry.classes4             = atoi(row[107]);
			entry.classes5             = atoi(row[108]);
			entry.classes6             = atoi(row[109]);
			entry.classes7             = atoi(row[110]);
			entry.classes8             = atoi(row[111]);
			entry.classes9             = atoi(row[112]);
			entry.classes10            = atoi(row[113]);
			entry.classes11            = atoi(row[114]);
			entry.classes12            = atoi(row[115]);
			entry.classes13            = atoi(row[116]);
			entry.classes14            = atoi(row[117]);
			entry.classes15            = atoi(row[118]);
			entry.classes16            = atoi(row[119]);
			entry.CastingAnim          = atoi(row[120]);
			entry.TargetAnim           = atoi(row[121]);
			entry.TravelType           = atoi(row[122]);
			entry.SpellAffectIndex     = atoi(row[123]);
			entry.disallow_sit         = atoi(row[124]);
			entry.deities0             = atoi(row[125]);
			entry.deities1             = atoi(row[126]);
			entry.deities2             = atoi(row[127]);
			entry.deities3             = atoi(row[128]);
			entry.deities4             = atoi(row[129]);
			entry.deities5             = atoi(row[130]);
			entry.deities6             = atoi(row[131]);
			entry.deities7             = atoi(row[132]);
			entry.deities8             = atoi(row[133]);
			entry.deities9             = atoi(row[134]);
			entry.deities10            = atoi(row[135]);
			entry.deities11            = atoi(row[136]);
			entry.deities12            = atoi(row[137]);
			entry.deities13            = atoi(row[138]);
			entry.deities14            = atoi(row[139]);
			entry.deities15            = atoi(row[140]);
			entry.deities16            = atoi(row[141]);
			entry.field142             = atoi(row[142]);
			entry.field143             = atoi(row[143]);
			entry.new_icon             = atoi(row[144]);
			entry.spellanim            = atoi(row[145]);
			entry.uninterruptable      = atoi(row[146]);
			entry.ResistDiff           = atoi(row[147]);
			entry.dot_stacking_exempt  = atoi(row[148]);
			entry.deleteable           = atoi(row[149]);
			entry.RecourseLink         = atoi(row[150]);
			entry.no_partial_resist    = atoi(row[151]);
			entry.field152             = atoi(row[152]);
			entry.field153             = atoi(row[153]);
			entry.short_buff_box       = atoi(row[154]);
			entry.descnum              = atoi(row[155]);
			entry.typedescnum          = atoi(row[156]);
			entry.effectdescnum        = atoi(row[157]);
			entry.effectdescnum2       = atoi(row[158]);
			entry.npc_no_los           = atoi(row[159]);
			entry.field160             = atoi(row[160]);
			entry.reflectable          = atoi(row[161]);
			entry.bonushate            = atoi(row[162]);
			entry.field163             = atoi(row[163]);
			entry.field164             = atoi(row[164]);
			entry.ldon_trap            = atoi(row[165]);
			entry.EndurCost            = atoi(row[166]);
			entry.EndurTimerIndex      = atoi(row[167]);
			entry.IsDiscipline         = atoi(row[168]);
			entry.field169             = atoi(row[169]);
			entry.field170             = atoi(row[170]);
			entry.field171             = atoi(row[171]);
			entry.field172             = atoi(row[172]);
			entry.HateAdded            = atoi(row[173]);
			entry.EndurUpkeep          = atoi(row[174]);
			entry.numhitstype          = atoi(row[175]);
			entry.numhits              = atoi(row[176]);
			entry.pvpresistbase        = atoi(row[177]);
			entry.pvpresistcalc        = atoi(row[178]);
			entry.pvpresistcap         = atoi(row[179]);
			entry.spell_category       = atoi(row[180]);
			entry.field181             = atoi(row[181]);
			entry.field182             = atoi(row[182]);
			entry.pcnpc_only_flag      = atoi(row[183]);
			entry.cast_not_standing    = atoi(row[184]);
			entry.can_mgb              = atoi(row[185]);
			entry.nodispell            = atoi(row[186]);
			entry.npc_category         = atoi(row[187]);
			entry.npc_usefulness       = atoi(row[188]);
			entry.MinResist            = atoi(row[189]);
			entry.MaxResist            = atoi(row[190]);
			entry.viral_targets        = atoi(row[191]);
			entry.viral_timer          = atoi(row[192]);
			entry.nimbuseffect         = atoi(row[193]);
			entry.ConeStartAngle       = atoi(row[194]);
			entry.ConeStopAngle        = atoi(row[195]);
			entry.sneaking             = atoi(row[196]);
			entry.not_extendable       = atoi(row[197]);
			entry.field198             = atoi(row[198]);
			entry.field199             = atoi(row[199]);
			entry.suspendable          = atoi(row[200]);
			entry.viral_range          = atoi(row[201]);
			entry.songcap              = atoi(row[202]);
			entry.field203             = atoi(row[203]);
			entry.field204             = atoi(row[204]);
			entry.no_block             = atoi(row[205]);
			entry.field206             = atoi(row[206]);
			entry.spellgroup           = atoi(row[207]);
			entry.rank                 = atoi(row[208]);
			entry.field209             = atoi(row[209]);
			entry.field210             = atoi(row[210]);
			entry.CastRestriction      = atoi(row[211]);
			entry.allowrest            = atoi(row[212]);
			entry.InCombat             = atoi(row[213]);
			entry.OutofCombat          = atoi(row[214]);
			entry.field215             = atoi(row[215]);
			entry.field216             = atoi(row[216]);
			entry.field217             = atoi(row[217]);
			entry.aemaxtargets         = atoi(row[218]);
			entry.maxtargets           = atoi(row[219]);
			entry.field220             = atoi(row[220]);
			entry.field221             = atoi(row[221]);
			entry.field222             = atoi(row[222]);
			entry.field223             = atoi(row[223]);
			entry.persistdeath         = atoi(row[224]);
			entry.field225             = atoi(row[225]);
			entry.field226             = atoi(row[226]);
			entry.min_dist             = static_cast<float>(atof(row[227]));
			entry.min_dist_mod         = static_cast<float>(atof(row[228]));
			entry.max_dist             = static_cast<float>(atof(row[229]));
			entry.max_dist_mod         = static_cast<float>(atof(row[230]));
			entry.min_range            = atoi(row[231]);
			entry.field232             = atoi(row[232]);
			entry.field233             = atoi(row[233]);
			entry.field234             = atoi(row[234]);
			entry.field235             = atoi(row[235]);
			entry.field236             = atoi(row[236]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<SpellsNew> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<SpellsNew> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpellsNew entry{};

			entry.id                   = atoi(row[0]);
			entry.name                 = row[1] ? row[1] : "";
			entry.player_1             = row[2] ? row[2] : "";
			entry.teleport_zone        = row[3] ? row[3] : "";
			entry.you_cast             = row[4] ? row[4] : "";
			entry.other_casts          = row[5] ? row[5] : "";
			entry.cast_on_you          = row[6] ? row[6] : "";
			entry.cast_on_other        = row[7] ? row[7] : "";
			entry.spell_fades          = row[8] ? row[8] : "";
			entry.range                = atoi(row[9]);
			entry.aoerange             = atoi(row[10]);
			entry.pushback             = atoi(row[11]);
			entry.pushup               = atoi(row[12]);
			entry.cast_time            = atoi(row[13]);
			entry.recovery_time        = atoi(row[14]);
			entry.recast_time          = atoi(row[15]);
			entry.buffdurationformula  = atoi(row[16]);
			entry.buffduration         = atoi(row[17]);
			entry.AEDuration           = atoi(row[18]);
			entry.mana                 = atoi(row[19]);
			entry.effect_base_value1   = atoi(row[20]);
			entry.effect_base_value2   = atoi(row[21]);
			entry.effect_base_value3   = atoi(row[22]);
			entry.effect_base_value4   = atoi(row[23]);
			entry.effect_base_value5   = atoi(row[24]);
			entry.effect_base_value6   = atoi(row[25]);
			entry.effect_base_value7   = atoi(row[26]);
			entry.effect_base_value8   = atoi(row[27]);
			entry.effect_base_value9   = atoi(row[28]);
			entry.effect_base_value10  = atoi(row[29]);
			entry.effect_base_value11  = atoi(row[30]);
			entry.effect_base_value12  = atoi(row[31]);
			entry.effect_limit_value1  = atoi(row[32]);
			entry.effect_limit_value2  = atoi(row[33]);
			entry.effect_limit_value3  = atoi(row[34]);
			entry.effect_limit_value4  = atoi(row[35]);
			entry.effect_limit_value5  = atoi(row[36]);
			entry.effect_limit_value6  = atoi(row[37]);
			entry.effect_limit_value7  = atoi(row[38]);
			entry.effect_limit_value8  = atoi(row[39]);
			entry.effect_limit_value9  = atoi(row[40]);
			entry.effect_limit_value10 = atoi(row[41]);
			entry.effect_limit_value11 = atoi(row[42]);
			entry.effect_limit_value12 = atoi(row[43]);
			entry.max1                 = atoi(row[44]);
			entry.max2                 = atoi(row[45]);
			entry.max3                 = atoi(row[46]);
			entry.max4                 = atoi(row[47]);
			entry.max5                 = atoi(row[48]);
			entry.max6                 = atoi(row[49]);
			entry.max7                 = atoi(row[50]);
			entry.max8                 = atoi(row[51]);
			entry.max9                 = atoi(row[52]);
			entry.max10                = atoi(row[53]);
			entry.max11                = atoi(row[54]);
			entry.max12                = atoi(row[55]);
			entry.icon                 = atoi(row[56]);
			entry.memicon              = atoi(row[57]);
			entry.components1          = atoi(row[58]);
			entry.components2          = atoi(row[59]);
			entry.components3          = atoi(row[60]);
			entry.components4          = atoi(row[61]);
			entry.component_counts1    = atoi(row[62]);
			entry.component_counts2    = atoi(row[63]);
			entry.component_counts3    = atoi(row[64]);
			entry.component_counts4    = atoi(row[65]);
			entry.NoexpendReagent1     = atoi(row[66]);
			entry.NoexpendReagent2     = atoi(row[67]);
			entry.NoexpendReagent3     = atoi(row[68]);
			entry.NoexpendReagent4     = atoi(row[69]);
			entry.formula1             = atoi(row[70]);
			entry.formula2             = atoi(row[71]);
			entry.formula3             = atoi(row[72]);
			entry.formula4             = atoi(row[73]);
			entry.formula5             = atoi(row[74]);
			entry.formula6             = atoi(row[75]);
			entry.formula7             = atoi(row[76]);
			entry.formula8             = atoi(row[77]);
			entry.formula9             = atoi(row[78]);
			entry.formula10            = atoi(row[79]);
			entry.formula11            = atoi(row[80]);
			entry.formula12            = atoi(row[81]);
			entry.LightType            = atoi(row[82]);
			entry.goodEffect           = atoi(row[83]);
			entry.Activated            = atoi(row[84]);
			entry.resisttype           = atoi(row[85]);
			entry.effectid1            = atoi(row[86]);
			entry.effectid2            = atoi(row[87]);
			entry.effectid3            = atoi(row[88]);
			entry.effectid4            = atoi(row[89]);
			entry.effectid5            = atoi(row[90]);
			entry.effectid6            = atoi(row[91]);
			entry.effectid7            = atoi(row[92]);
			entry.effectid8            = atoi(row[93]);
			entry.effectid9            = atoi(row[94]);
			entry.effectid10           = atoi(row[95]);
			entry.effectid11           = atoi(row[96]);
			entry.effectid12           = atoi(row[97]);
			entry.targettype           = atoi(row[98]);
			entry.basediff             = atoi(row[99]);
			entry.skill                = atoi(row[100]);
			entry.zonetype             = atoi(row[101]);
			entry.EnvironmentType      = atoi(row[102]);
			entry.TimeOfDay            = atoi(row[103]);
			entry.classes1             = atoi(row[104]);
			entry.classes2             = atoi(row[105]);
			entry.classes3             = atoi(row[106]);
			entry.classes4             = atoi(row[107]);
			entry.classes5             = atoi(row[108]);
			entry.classes6             = atoi(row[109]);
			entry.classes7             = atoi(row[110]);
			entry.classes8             = atoi(row[111]);
			entry.classes9             = atoi(row[112]);
			entry.classes10            = atoi(row[113]);
			entry.classes11            = atoi(row[114]);
			entry.classes12            = atoi(row[115]);
			entry.classes13            = atoi(row[116]);
			entry.classes14            = atoi(row[117]);
			entry.classes15            = atoi(row[118]);
			entry.classes16            = atoi(row[119]);
			entry.CastingAnim          = atoi(row[120]);
			entry.TargetAnim           = atoi(row[121]);
			entry.TravelType           = atoi(row[122]);
			entry.SpellAffectIndex     = atoi(row[123]);
			entry.disallow_sit         = atoi(row[124]);
			entry.deities0             = atoi(row[125]);
			entry.deities1             = atoi(row[126]);
			entry.deities2             = atoi(row[127]);
			entry.deities3             = atoi(row[128]);
			entry.deities4             = atoi(row[129]);
			entry.deities5             = atoi(row[130]);
			entry.deities6             = atoi(row[131]);
			entry.deities7             = atoi(row[132]);
			entry.deities8             = atoi(row[133]);
			entry.deities9             = atoi(row[134]);
			entry.deities10            = atoi(row[135]);
			entry.deities11            = atoi(row[136]);
			entry.deities12            = atoi(row[137]);
			entry.deities13            = atoi(row[138]);
			entry.deities14            = atoi(row[139]);
			entry.deities15            = atoi(row[140]);
			entry.deities16            = atoi(row[141]);
			entry.field142             = atoi(row[142]);
			entry.field143             = atoi(row[143]);
			entry.new_icon             = atoi(row[144]);
			entry.spellanim            = atoi(row[145]);
			entry.uninterruptable      = atoi(row[146]);
			entry.ResistDiff           = atoi(row[147]);
			entry.dot_stacking_exempt  = atoi(row[148]);
			entry.deleteable           = atoi(row[149]);
			entry.RecourseLink         = atoi(row[150]);
			entry.no_partial_resist    = atoi(row[151]);
			entry.field152             = atoi(row[152]);
			entry.field153             = atoi(row[153]);
			entry.short_buff_box       = atoi(row[154]);
			entry.descnum              = atoi(row[155]);
			entry.typedescnum          = atoi(row[156]);
			entry.effectdescnum        = atoi(row[157]);
			entry.effectdescnum2       = atoi(row[158]);
			entry.npc_no_los           = atoi(row[159]);
			entry.field160             = atoi(row[160]);
			entry.reflectable          = atoi(row[161]);
			entry.bonushate            = atoi(row[162]);
			entry.field163             = atoi(row[163]);
			entry.field164             = atoi(row[164]);
			entry.ldon_trap            = atoi(row[165]);
			entry.EndurCost            = atoi(row[166]);
			entry.EndurTimerIndex      = atoi(row[167]);
			entry.IsDiscipline         = atoi(row[168]);
			entry.field169             = atoi(row[169]);
			entry.field170             = atoi(row[170]);
			entry.field171             = atoi(row[171]);
			entry.field172             = atoi(row[172]);
			entry.HateAdded            = atoi(row[173]);
			entry.EndurUpkeep          = atoi(row[174]);
			entry.numhitstype          = atoi(row[175]);
			entry.numhits              = atoi(row[176]);
			entry.pvpresistbase        = atoi(row[177]);
			entry.pvpresistcalc        = atoi(row[178]);
			entry.pvpresistcap         = atoi(row[179]);
			entry.spell_category       = atoi(row[180]);
			entry.field181             = atoi(row[181]);
			entry.field182             = atoi(row[182]);
			entry.pcnpc_only_flag      = atoi(row[183]);
			entry.cast_not_standing    = atoi(row[184]);
			entry.can_mgb              = atoi(row[185]);
			entry.nodispell            = atoi(row[186]);
			entry.npc_category         = atoi(row[187]);
			entry.npc_usefulness       = atoi(row[188]);
			entry.MinResist            = atoi(row[189]);
			entry.MaxResist            = atoi(row[190]);
			entry.viral_targets        = atoi(row[191]);
			entry.viral_timer          = atoi(row[192]);
			entry.nimbuseffect         = atoi(row[193]);
			entry.ConeStartAngle       = atoi(row[194]);
			entry.ConeStopAngle        = atoi(row[195]);
			entry.sneaking             = atoi(row[196]);
			entry.not_extendable       = atoi(row[197]);
			entry.field198             = atoi(row[198]);
			entry.field199             = atoi(row[199]);
			entry.suspendable          = atoi(row[200]);
			entry.viral_range          = atoi(row[201]);
			entry.songcap              = atoi(row[202]);
			entry.field203             = atoi(row[203]);
			entry.field204             = atoi(row[204]);
			entry.no_block             = atoi(row[205]);
			entry.field206             = atoi(row[206]);
			entry.spellgroup           = atoi(row[207]);
			entry.rank                 = atoi(row[208]);
			entry.field209             = atoi(row[209]);
			entry.field210             = atoi(row[210]);
			entry.CastRestriction      = atoi(row[211]);
			entry.allowrest            = atoi(row[212]);
			entry.InCombat             = atoi(row[213]);
			entry.OutofCombat          = atoi(row[214]);
			entry.field215             = atoi(row[215]);
			entry.field216             = atoi(row[216]);
			entry.field217             = atoi(row[217]);
			entry.aemaxtargets         = atoi(row[218]);
			entry.maxtargets           = atoi(row[219]);
			entry.field220             = atoi(row[220]);
			entry.field221             = atoi(row[221]);
			entry.field222             = atoi(row[222]);
			entry.field223             = atoi(row[223]);
			entry.persistdeath         = atoi(row[224]);
			entry.field225             = atoi(row[225]);
			entry.field226             = atoi(row[226]);
			entry.min_dist             = static_cast<float>(atof(row[227]));
			entry.min_dist_mod         = static_cast<float>(atof(row[228]));
			entry.max_dist             = static_cast<float>(atof(row[229]));
			entry.max_dist_mod         = static_cast<float>(atof(row[230]));
			entry.min_range            = atoi(row[231]);
			entry.field232             = atoi(row[232]);
			entry.field233             = atoi(row[233]);
			entry.field234             = atoi(row[234]);
			entry.field235             = atoi(row[235]);
			entry.field236             = atoi(row[236]);

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

#endif //EQEMU_BASE_SPELLS_NEW_REPOSITORY_H
