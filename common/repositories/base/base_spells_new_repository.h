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

#ifndef EQEMU_BASE_SPELLS_NEW_REPOSITORY_H
#define EQEMU_BASE_SPELLS_NEW_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

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
		int         pvp_duration;
		int         pvp_duration_cap;
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
			"pvp_duration",
			"pvp_duration_cap",
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

	static std::vector<std::string> SelectColumns()
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
			"pvp_duration",
			"pvp_duration_cap",
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
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("spells_new");
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

	static SpellsNew NewEntity()
	{
		SpellsNew e{};

		e.id                   = 0;
		e.name                 = "";
		e.player_1             = "BLUE_TRAIL";
		e.teleport_zone        = "";
		e.you_cast             = "";
		e.other_casts          = "";
		e.cast_on_you          = "";
		e.cast_on_other        = "";
		e.spell_fades          = "";
		e.range                = 100;
		e.aoerange             = 0;
		e.pushback             = 0;
		e.pushup               = 0;
		e.cast_time            = 0;
		e.recovery_time        = 0;
		e.recast_time          = 0;
		e.buffdurationformula  = 7;
		e.buffduration         = 65;
		e.AEDuration           = 0;
		e.mana                 = 0;
		e.effect_base_value1   = 100;
		e.effect_base_value2   = 0;
		e.effect_base_value3   = 0;
		e.effect_base_value4   = 0;
		e.effect_base_value5   = 0;
		e.effect_base_value6   = 0;
		e.effect_base_value7   = 0;
		e.effect_base_value8   = 0;
		e.effect_base_value9   = 0;
		e.effect_base_value10  = 0;
		e.effect_base_value11  = 0;
		e.effect_base_value12  = 0;
		e.effect_limit_value1  = 0;
		e.effect_limit_value2  = 0;
		e.effect_limit_value3  = 0;
		e.effect_limit_value4  = 0;
		e.effect_limit_value5  = 0;
		e.effect_limit_value6  = 0;
		e.effect_limit_value7  = 0;
		e.effect_limit_value8  = 0;
		e.effect_limit_value9  = 0;
		e.effect_limit_value10 = 0;
		e.effect_limit_value11 = 0;
		e.effect_limit_value12 = 0;
		e.max1                 = 0;
		e.max2                 = 0;
		e.max3                 = 0;
		e.max4                 = 0;
		e.max5                 = 0;
		e.max6                 = 0;
		e.max7                 = 0;
		e.max8                 = 0;
		e.max9                 = 0;
		e.max10                = 0;
		e.max11                = 0;
		e.max12                = 0;
		e.icon                 = 0;
		e.memicon              = 0;
		e.components1          = -1;
		e.components2          = -1;
		e.components3          = -1;
		e.components4          = -1;
		e.component_counts1    = 1;
		e.component_counts2    = 1;
		e.component_counts3    = 1;
		e.component_counts4    = 1;
		e.NoexpendReagent1     = -1;
		e.NoexpendReagent2     = -1;
		e.NoexpendReagent3     = -1;
		e.NoexpendReagent4     = -1;
		e.formula1             = 100;
		e.formula2             = 100;
		e.formula3             = 100;
		e.formula4             = 100;
		e.formula5             = 100;
		e.formula6             = 100;
		e.formula7             = 100;
		e.formula8             = 100;
		e.formula9             = 100;
		e.formula10            = 100;
		e.formula11            = 100;
		e.formula12            = 100;
		e.LightType            = 0;
		e.goodEffect           = 0;
		e.Activated            = 0;
		e.resisttype           = 0;
		e.effectid1            = 254;
		e.effectid2            = 254;
		e.effectid3            = 254;
		e.effectid4            = 254;
		e.effectid5            = 254;
		e.effectid6            = 254;
		e.effectid7            = 254;
		e.effectid8            = 254;
		e.effectid9            = 254;
		e.effectid10           = 254;
		e.effectid11           = 254;
		e.effectid12           = 254;
		e.targettype           = 2;
		e.basediff             = 0;
		e.skill                = 98;
		e.zonetype             = -1;
		e.EnvironmentType      = 0;
		e.TimeOfDay            = 0;
		e.classes1             = 255;
		e.classes2             = 255;
		e.classes3             = 255;
		e.classes4             = 255;
		e.classes5             = 255;
		e.classes6             = 255;
		e.classes7             = 255;
		e.classes8             = 255;
		e.classes9             = 255;
		e.classes10            = 255;
		e.classes11            = 255;
		e.classes12            = 255;
		e.classes13            = 255;
		e.classes14            = 255;
		e.classes15            = 255;
		e.classes16            = 255;
		e.CastingAnim          = 44;
		e.TargetAnim           = 13;
		e.TravelType           = 0;
		e.SpellAffectIndex     = -1;
		e.disallow_sit         = 0;
		e.deities0             = 0;
		e.deities1             = 0;
		e.deities2             = 0;
		e.deities3             = 0;
		e.deities4             = 0;
		e.deities5             = 0;
		e.deities6             = 0;
		e.deities7             = 0;
		e.deities8             = 0;
		e.deities9             = 0;
		e.deities10            = 0;
		e.deities11            = 0;
		e.deities12            = 0;
		e.deities13            = 0;
		e.deities14            = 0;
		e.deities15            = 0;
		e.deities16            = 0;
		e.field142             = 100;
		e.field143             = 0;
		e.new_icon             = 161;
		e.spellanim            = 0;
		e.uninterruptable      = 0;
		e.ResistDiff           = -150;
		e.dot_stacking_exempt  = 0;
		e.deleteable           = 0;
		e.RecourseLink         = 0;
		e.no_partial_resist    = 0;
		e.field152             = 0;
		e.field153             = 0;
		e.short_buff_box       = -1;
		e.descnum              = 0;
		e.typedescnum          = 0;
		e.effectdescnum        = 0;
		e.effectdescnum2       = 0;
		e.npc_no_los           = 0;
		e.field160             = 0;
		e.reflectable          = 0;
		e.bonushate            = 0;
		e.field163             = 100;
		e.field164             = -150;
		e.ldon_trap            = 0;
		e.EndurCost            = 0;
		e.EndurTimerIndex      = 0;
		e.IsDiscipline         = 0;
		e.field169             = 0;
		e.field170             = 0;
		e.field171             = 0;
		e.field172             = 0;
		e.HateAdded            = 0;
		e.EndurUpkeep          = 0;
		e.numhitstype          = 0;
		e.numhits              = 0;
		e.pvpresistbase        = -150;
		e.pvpresistcalc        = 100;
		e.pvpresistcap         = -150;
		e.spell_category       = -99;
		e.pvp_duration         = 0;
		e.pvp_duration_cap     = 0;
		e.pcnpc_only_flag      = 0;
		e.cast_not_standing    = 0;
		e.can_mgb              = 0;
		e.nodispell            = -1;
		e.npc_category         = 0;
		e.npc_usefulness       = 0;
		e.MinResist            = 0;
		e.MaxResist            = 0;
		e.viral_targets        = 0;
		e.viral_timer          = 0;
		e.nimbuseffect         = 0;
		e.ConeStartAngle       = 0;
		e.ConeStopAngle        = 0;
		e.sneaking             = 0;
		e.not_extendable       = 0;
		e.field198             = 0;
		e.field199             = 1;
		e.suspendable          = 0;
		e.viral_range          = 0;
		e.songcap              = 0;
		e.field203             = 0;
		e.field204             = 0;
		e.no_block             = 0;
		e.field206             = -1;
		e.spellgroup           = 0;
		e.rank                 = 0;
		e.field209             = 0;
		e.field210             = 1;
		e.CastRestriction      = 0;
		e.allowrest            = 0;
		e.InCombat             = 0;
		e.OutofCombat          = 0;
		e.field215             = 0;
		e.field216             = 0;
		e.field217             = 0;
		e.aemaxtargets         = 0;
		e.maxtargets           = 0;
		e.field220             = 0;
		e.field221             = 0;
		e.field222             = 0;
		e.field223             = 0;
		e.persistdeath         = 0;
		e.field225             = 0;
		e.field226             = 0;
		e.min_dist             = 0;
		e.min_dist_mod         = 0;
		e.max_dist             = 0;
		e.max_dist_mod         = 0;
		e.min_range            = 0;
		e.field232             = 0;
		e.field233             = 0;
		e.field234             = 0;
		e.field235             = 0;
		e.field236             = 0;

		return e;
	}

	static SpellsNew GetSpellsNew(
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
			SpellsNew e{};

			e.id                   = atoi(row[0]);
			e.name                 = row[1] ? row[1] : "";
			e.player_1             = row[2] ? row[2] : "";
			e.teleport_zone        = row[3] ? row[3] : "";
			e.you_cast             = row[4] ? row[4] : "";
			e.other_casts          = row[5] ? row[5] : "";
			e.cast_on_you          = row[6] ? row[6] : "";
			e.cast_on_other        = row[7] ? row[7] : "";
			e.spell_fades          = row[8] ? row[8] : "";
			e.range                = atoi(row[9]);
			e.aoerange             = atoi(row[10]);
			e.pushback             = atoi(row[11]);
			e.pushup               = atoi(row[12]);
			e.cast_time            = atoi(row[13]);
			e.recovery_time        = atoi(row[14]);
			e.recast_time          = atoi(row[15]);
			e.buffdurationformula  = atoi(row[16]);
			e.buffduration         = atoi(row[17]);
			e.AEDuration           = atoi(row[18]);
			e.mana                 = atoi(row[19]);
			e.effect_base_value1   = atoi(row[20]);
			e.effect_base_value2   = atoi(row[21]);
			e.effect_base_value3   = atoi(row[22]);
			e.effect_base_value4   = atoi(row[23]);
			e.effect_base_value5   = atoi(row[24]);
			e.effect_base_value6   = atoi(row[25]);
			e.effect_base_value7   = atoi(row[26]);
			e.effect_base_value8   = atoi(row[27]);
			e.effect_base_value9   = atoi(row[28]);
			e.effect_base_value10  = atoi(row[29]);
			e.effect_base_value11  = atoi(row[30]);
			e.effect_base_value12  = atoi(row[31]);
			e.effect_limit_value1  = atoi(row[32]);
			e.effect_limit_value2  = atoi(row[33]);
			e.effect_limit_value3  = atoi(row[34]);
			e.effect_limit_value4  = atoi(row[35]);
			e.effect_limit_value5  = atoi(row[36]);
			e.effect_limit_value6  = atoi(row[37]);
			e.effect_limit_value7  = atoi(row[38]);
			e.effect_limit_value8  = atoi(row[39]);
			e.effect_limit_value9  = atoi(row[40]);
			e.effect_limit_value10 = atoi(row[41]);
			e.effect_limit_value11 = atoi(row[42]);
			e.effect_limit_value12 = atoi(row[43]);
			e.max1                 = atoi(row[44]);
			e.max2                 = atoi(row[45]);
			e.max3                 = atoi(row[46]);
			e.max4                 = atoi(row[47]);
			e.max5                 = atoi(row[48]);
			e.max6                 = atoi(row[49]);
			e.max7                 = atoi(row[50]);
			e.max8                 = atoi(row[51]);
			e.max9                 = atoi(row[52]);
			e.max10                = atoi(row[53]);
			e.max11                = atoi(row[54]);
			e.max12                = atoi(row[55]);
			e.icon                 = atoi(row[56]);
			e.memicon              = atoi(row[57]);
			e.components1          = atoi(row[58]);
			e.components2          = atoi(row[59]);
			e.components3          = atoi(row[60]);
			e.components4          = atoi(row[61]);
			e.component_counts1    = atoi(row[62]);
			e.component_counts2    = atoi(row[63]);
			e.component_counts3    = atoi(row[64]);
			e.component_counts4    = atoi(row[65]);
			e.NoexpendReagent1     = atoi(row[66]);
			e.NoexpendReagent2     = atoi(row[67]);
			e.NoexpendReagent3     = atoi(row[68]);
			e.NoexpendReagent4     = atoi(row[69]);
			e.formula1             = atoi(row[70]);
			e.formula2             = atoi(row[71]);
			e.formula3             = atoi(row[72]);
			e.formula4             = atoi(row[73]);
			e.formula5             = atoi(row[74]);
			e.formula6             = atoi(row[75]);
			e.formula7             = atoi(row[76]);
			e.formula8             = atoi(row[77]);
			e.formula9             = atoi(row[78]);
			e.formula10            = atoi(row[79]);
			e.formula11            = atoi(row[80]);
			e.formula12            = atoi(row[81]);
			e.LightType            = atoi(row[82]);
			e.goodEffect           = atoi(row[83]);
			e.Activated            = atoi(row[84]);
			e.resisttype           = atoi(row[85]);
			e.effectid1            = atoi(row[86]);
			e.effectid2            = atoi(row[87]);
			e.effectid3            = atoi(row[88]);
			e.effectid4            = atoi(row[89]);
			e.effectid5            = atoi(row[90]);
			e.effectid6            = atoi(row[91]);
			e.effectid7            = atoi(row[92]);
			e.effectid8            = atoi(row[93]);
			e.effectid9            = atoi(row[94]);
			e.effectid10           = atoi(row[95]);
			e.effectid11           = atoi(row[96]);
			e.effectid12           = atoi(row[97]);
			e.targettype           = atoi(row[98]);
			e.basediff             = atoi(row[99]);
			e.skill                = atoi(row[100]);
			e.zonetype             = atoi(row[101]);
			e.EnvironmentType      = atoi(row[102]);
			e.TimeOfDay            = atoi(row[103]);
			e.classes1             = atoi(row[104]);
			e.classes2             = atoi(row[105]);
			e.classes3             = atoi(row[106]);
			e.classes4             = atoi(row[107]);
			e.classes5             = atoi(row[108]);
			e.classes6             = atoi(row[109]);
			e.classes7             = atoi(row[110]);
			e.classes8             = atoi(row[111]);
			e.classes9             = atoi(row[112]);
			e.classes10            = atoi(row[113]);
			e.classes11            = atoi(row[114]);
			e.classes12            = atoi(row[115]);
			e.classes13            = atoi(row[116]);
			e.classes14            = atoi(row[117]);
			e.classes15            = atoi(row[118]);
			e.classes16            = atoi(row[119]);
			e.CastingAnim          = atoi(row[120]);
			e.TargetAnim           = atoi(row[121]);
			e.TravelType           = atoi(row[122]);
			e.SpellAffectIndex     = atoi(row[123]);
			e.disallow_sit         = atoi(row[124]);
			e.deities0             = atoi(row[125]);
			e.deities1             = atoi(row[126]);
			e.deities2             = atoi(row[127]);
			e.deities3             = atoi(row[128]);
			e.deities4             = atoi(row[129]);
			e.deities5             = atoi(row[130]);
			e.deities6             = atoi(row[131]);
			e.deities7             = atoi(row[132]);
			e.deities8             = atoi(row[133]);
			e.deities9             = atoi(row[134]);
			e.deities10            = atoi(row[135]);
			e.deities11            = atoi(row[136]);
			e.deities12            = atoi(row[137]);
			e.deities13            = atoi(row[138]);
			e.deities14            = atoi(row[139]);
			e.deities15            = atoi(row[140]);
			e.deities16            = atoi(row[141]);
			e.field142             = atoi(row[142]);
			e.field143             = atoi(row[143]);
			e.new_icon             = atoi(row[144]);
			e.spellanim            = atoi(row[145]);
			e.uninterruptable      = atoi(row[146]);
			e.ResistDiff           = atoi(row[147]);
			e.dot_stacking_exempt  = atoi(row[148]);
			e.deleteable           = atoi(row[149]);
			e.RecourseLink         = atoi(row[150]);
			e.no_partial_resist    = atoi(row[151]);
			e.field152             = atoi(row[152]);
			e.field153             = atoi(row[153]);
			e.short_buff_box       = atoi(row[154]);
			e.descnum              = atoi(row[155]);
			e.typedescnum          = atoi(row[156]);
			e.effectdescnum        = atoi(row[157]);
			e.effectdescnum2       = atoi(row[158]);
			e.npc_no_los           = atoi(row[159]);
			e.field160             = atoi(row[160]);
			e.reflectable          = atoi(row[161]);
			e.bonushate            = atoi(row[162]);
			e.field163             = atoi(row[163]);
			e.field164             = atoi(row[164]);
			e.ldon_trap            = atoi(row[165]);
			e.EndurCost            = atoi(row[166]);
			e.EndurTimerIndex      = atoi(row[167]);
			e.IsDiscipline         = atoi(row[168]);
			e.field169             = atoi(row[169]);
			e.field170             = atoi(row[170]);
			e.field171             = atoi(row[171]);
			e.field172             = atoi(row[172]);
			e.HateAdded            = atoi(row[173]);
			e.EndurUpkeep          = atoi(row[174]);
			e.numhitstype          = atoi(row[175]);
			e.numhits              = atoi(row[176]);
			e.pvpresistbase        = atoi(row[177]);
			e.pvpresistcalc        = atoi(row[178]);
			e.pvpresistcap         = atoi(row[179]);
			e.spell_category       = atoi(row[180]);
			e.pvp_duration         = atoi(row[181]);
			e.pvp_duration_cap     = atoi(row[182]);
			e.pcnpc_only_flag      = atoi(row[183]);
			e.cast_not_standing    = atoi(row[184]);
			e.can_mgb              = atoi(row[185]);
			e.nodispell            = atoi(row[186]);
			e.npc_category         = atoi(row[187]);
			e.npc_usefulness       = atoi(row[188]);
			e.MinResist            = atoi(row[189]);
			e.MaxResist            = atoi(row[190]);
			e.viral_targets        = atoi(row[191]);
			e.viral_timer          = atoi(row[192]);
			e.nimbuseffect         = atoi(row[193]);
			e.ConeStartAngle       = atoi(row[194]);
			e.ConeStopAngle        = atoi(row[195]);
			e.sneaking             = atoi(row[196]);
			e.not_extendable       = atoi(row[197]);
			e.field198             = atoi(row[198]);
			e.field199             = atoi(row[199]);
			e.suspendable          = atoi(row[200]);
			e.viral_range          = atoi(row[201]);
			e.songcap              = atoi(row[202]);
			e.field203             = atoi(row[203]);
			e.field204             = atoi(row[204]);
			e.no_block             = atoi(row[205]);
			e.field206             = atoi(row[206]);
			e.spellgroup           = atoi(row[207]);
			e.rank                 = atoi(row[208]);
			e.field209             = atoi(row[209]);
			e.field210             = atoi(row[210]);
			e.CastRestriction      = atoi(row[211]);
			e.allowrest            = atoi(row[212]);
			e.InCombat             = atoi(row[213]);
			e.OutofCombat          = atoi(row[214]);
			e.field215             = atoi(row[215]);
			e.field216             = atoi(row[216]);
			e.field217             = atoi(row[217]);
			e.aemaxtargets         = atoi(row[218]);
			e.maxtargets           = atoi(row[219]);
			e.field220             = atoi(row[220]);
			e.field221             = atoi(row[221]);
			e.field222             = atoi(row[222]);
			e.field223             = atoi(row[223]);
			e.persistdeath         = atoi(row[224]);
			e.field225             = atoi(row[225]);
			e.field226             = atoi(row[226]);
			e.min_dist             = static_cast<float>(atof(row[227]));
			e.min_dist_mod         = static_cast<float>(atof(row[228]));
			e.max_dist             = static_cast<float>(atof(row[229]));
			e.max_dist_mod         = static_cast<float>(atof(row[230]));
			e.min_range            = atoi(row[231]);
			e.field232             = atoi(row[232]);
			e.field233             = atoi(row[233]);
			e.field234             = atoi(row[234]);
			e.field235             = atoi(row[235]);
			e.field236             = atoi(row[236]);

			return e;
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
		const SpellsNew &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.id));
		v.push_back(columns[1] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.player_1) + "'");
		v.push_back(columns[3] + " = '" + Strings::Escape(e.teleport_zone) + "'");
		v.push_back(columns[4] + " = '" + Strings::Escape(e.you_cast) + "'");
		v.push_back(columns[5] + " = '" + Strings::Escape(e.other_casts) + "'");
		v.push_back(columns[6] + " = '" + Strings::Escape(e.cast_on_you) + "'");
		v.push_back(columns[7] + " = '" + Strings::Escape(e.cast_on_other) + "'");
		v.push_back(columns[8] + " = '" + Strings::Escape(e.spell_fades) + "'");
		v.push_back(columns[9] + " = " + std::to_string(e.range));
		v.push_back(columns[10] + " = " + std::to_string(e.aoerange));
		v.push_back(columns[11] + " = " + std::to_string(e.pushback));
		v.push_back(columns[12] + " = " + std::to_string(e.pushup));
		v.push_back(columns[13] + " = " + std::to_string(e.cast_time));
		v.push_back(columns[14] + " = " + std::to_string(e.recovery_time));
		v.push_back(columns[15] + " = " + std::to_string(e.recast_time));
		v.push_back(columns[16] + " = " + std::to_string(e.buffdurationformula));
		v.push_back(columns[17] + " = " + std::to_string(e.buffduration));
		v.push_back(columns[18] + " = " + std::to_string(e.AEDuration));
		v.push_back(columns[19] + " = " + std::to_string(e.mana));
		v.push_back(columns[20] + " = " + std::to_string(e.effect_base_value1));
		v.push_back(columns[21] + " = " + std::to_string(e.effect_base_value2));
		v.push_back(columns[22] + " = " + std::to_string(e.effect_base_value3));
		v.push_back(columns[23] + " = " + std::to_string(e.effect_base_value4));
		v.push_back(columns[24] + " = " + std::to_string(e.effect_base_value5));
		v.push_back(columns[25] + " = " + std::to_string(e.effect_base_value6));
		v.push_back(columns[26] + " = " + std::to_string(e.effect_base_value7));
		v.push_back(columns[27] + " = " + std::to_string(e.effect_base_value8));
		v.push_back(columns[28] + " = " + std::to_string(e.effect_base_value9));
		v.push_back(columns[29] + " = " + std::to_string(e.effect_base_value10));
		v.push_back(columns[30] + " = " + std::to_string(e.effect_base_value11));
		v.push_back(columns[31] + " = " + std::to_string(e.effect_base_value12));
		v.push_back(columns[32] + " = " + std::to_string(e.effect_limit_value1));
		v.push_back(columns[33] + " = " + std::to_string(e.effect_limit_value2));
		v.push_back(columns[34] + " = " + std::to_string(e.effect_limit_value3));
		v.push_back(columns[35] + " = " + std::to_string(e.effect_limit_value4));
		v.push_back(columns[36] + " = " + std::to_string(e.effect_limit_value5));
		v.push_back(columns[37] + " = " + std::to_string(e.effect_limit_value6));
		v.push_back(columns[38] + " = " + std::to_string(e.effect_limit_value7));
		v.push_back(columns[39] + " = " + std::to_string(e.effect_limit_value8));
		v.push_back(columns[40] + " = " + std::to_string(e.effect_limit_value9));
		v.push_back(columns[41] + " = " + std::to_string(e.effect_limit_value10));
		v.push_back(columns[42] + " = " + std::to_string(e.effect_limit_value11));
		v.push_back(columns[43] + " = " + std::to_string(e.effect_limit_value12));
		v.push_back(columns[44] + " = " + std::to_string(e.max1));
		v.push_back(columns[45] + " = " + std::to_string(e.max2));
		v.push_back(columns[46] + " = " + std::to_string(e.max3));
		v.push_back(columns[47] + " = " + std::to_string(e.max4));
		v.push_back(columns[48] + " = " + std::to_string(e.max5));
		v.push_back(columns[49] + " = " + std::to_string(e.max6));
		v.push_back(columns[50] + " = " + std::to_string(e.max7));
		v.push_back(columns[51] + " = " + std::to_string(e.max8));
		v.push_back(columns[52] + " = " + std::to_string(e.max9));
		v.push_back(columns[53] + " = " + std::to_string(e.max10));
		v.push_back(columns[54] + " = " + std::to_string(e.max11));
		v.push_back(columns[55] + " = " + std::to_string(e.max12));
		v.push_back(columns[56] + " = " + std::to_string(e.icon));
		v.push_back(columns[57] + " = " + std::to_string(e.memicon));
		v.push_back(columns[58] + " = " + std::to_string(e.components1));
		v.push_back(columns[59] + " = " + std::to_string(e.components2));
		v.push_back(columns[60] + " = " + std::to_string(e.components3));
		v.push_back(columns[61] + " = " + std::to_string(e.components4));
		v.push_back(columns[62] + " = " + std::to_string(e.component_counts1));
		v.push_back(columns[63] + " = " + std::to_string(e.component_counts2));
		v.push_back(columns[64] + " = " + std::to_string(e.component_counts3));
		v.push_back(columns[65] + " = " + std::to_string(e.component_counts4));
		v.push_back(columns[66] + " = " + std::to_string(e.NoexpendReagent1));
		v.push_back(columns[67] + " = " + std::to_string(e.NoexpendReagent2));
		v.push_back(columns[68] + " = " + std::to_string(e.NoexpendReagent3));
		v.push_back(columns[69] + " = " + std::to_string(e.NoexpendReagent4));
		v.push_back(columns[70] + " = " + std::to_string(e.formula1));
		v.push_back(columns[71] + " = " + std::to_string(e.formula2));
		v.push_back(columns[72] + " = " + std::to_string(e.formula3));
		v.push_back(columns[73] + " = " + std::to_string(e.formula4));
		v.push_back(columns[74] + " = " + std::to_string(e.formula5));
		v.push_back(columns[75] + " = " + std::to_string(e.formula6));
		v.push_back(columns[76] + " = " + std::to_string(e.formula7));
		v.push_back(columns[77] + " = " + std::to_string(e.formula8));
		v.push_back(columns[78] + " = " + std::to_string(e.formula9));
		v.push_back(columns[79] + " = " + std::to_string(e.formula10));
		v.push_back(columns[80] + " = " + std::to_string(e.formula11));
		v.push_back(columns[81] + " = " + std::to_string(e.formula12));
		v.push_back(columns[82] + " = " + std::to_string(e.LightType));
		v.push_back(columns[83] + " = " + std::to_string(e.goodEffect));
		v.push_back(columns[84] + " = " + std::to_string(e.Activated));
		v.push_back(columns[85] + " = " + std::to_string(e.resisttype));
		v.push_back(columns[86] + " = " + std::to_string(e.effectid1));
		v.push_back(columns[87] + " = " + std::to_string(e.effectid2));
		v.push_back(columns[88] + " = " + std::to_string(e.effectid3));
		v.push_back(columns[89] + " = " + std::to_string(e.effectid4));
		v.push_back(columns[90] + " = " + std::to_string(e.effectid5));
		v.push_back(columns[91] + " = " + std::to_string(e.effectid6));
		v.push_back(columns[92] + " = " + std::to_string(e.effectid7));
		v.push_back(columns[93] + " = " + std::to_string(e.effectid8));
		v.push_back(columns[94] + " = " + std::to_string(e.effectid9));
		v.push_back(columns[95] + " = " + std::to_string(e.effectid10));
		v.push_back(columns[96] + " = " + std::to_string(e.effectid11));
		v.push_back(columns[97] + " = " + std::to_string(e.effectid12));
		v.push_back(columns[98] + " = " + std::to_string(e.targettype));
		v.push_back(columns[99] + " = " + std::to_string(e.basediff));
		v.push_back(columns[100] + " = " + std::to_string(e.skill));
		v.push_back(columns[101] + " = " + std::to_string(e.zonetype));
		v.push_back(columns[102] + " = " + std::to_string(e.EnvironmentType));
		v.push_back(columns[103] + " = " + std::to_string(e.TimeOfDay));
		v.push_back(columns[104] + " = " + std::to_string(e.classes1));
		v.push_back(columns[105] + " = " + std::to_string(e.classes2));
		v.push_back(columns[106] + " = " + std::to_string(e.classes3));
		v.push_back(columns[107] + " = " + std::to_string(e.classes4));
		v.push_back(columns[108] + " = " + std::to_string(e.classes5));
		v.push_back(columns[109] + " = " + std::to_string(e.classes6));
		v.push_back(columns[110] + " = " + std::to_string(e.classes7));
		v.push_back(columns[111] + " = " + std::to_string(e.classes8));
		v.push_back(columns[112] + " = " + std::to_string(e.classes9));
		v.push_back(columns[113] + " = " + std::to_string(e.classes10));
		v.push_back(columns[114] + " = " + std::to_string(e.classes11));
		v.push_back(columns[115] + " = " + std::to_string(e.classes12));
		v.push_back(columns[116] + " = " + std::to_string(e.classes13));
		v.push_back(columns[117] + " = " + std::to_string(e.classes14));
		v.push_back(columns[118] + " = " + std::to_string(e.classes15));
		v.push_back(columns[119] + " = " + std::to_string(e.classes16));
		v.push_back(columns[120] + " = " + std::to_string(e.CastingAnim));
		v.push_back(columns[121] + " = " + std::to_string(e.TargetAnim));
		v.push_back(columns[122] + " = " + std::to_string(e.TravelType));
		v.push_back(columns[123] + " = " + std::to_string(e.SpellAffectIndex));
		v.push_back(columns[124] + " = " + std::to_string(e.disallow_sit));
		v.push_back(columns[125] + " = " + std::to_string(e.deities0));
		v.push_back(columns[126] + " = " + std::to_string(e.deities1));
		v.push_back(columns[127] + " = " + std::to_string(e.deities2));
		v.push_back(columns[128] + " = " + std::to_string(e.deities3));
		v.push_back(columns[129] + " = " + std::to_string(e.deities4));
		v.push_back(columns[130] + " = " + std::to_string(e.deities5));
		v.push_back(columns[131] + " = " + std::to_string(e.deities6));
		v.push_back(columns[132] + " = " + std::to_string(e.deities7));
		v.push_back(columns[133] + " = " + std::to_string(e.deities8));
		v.push_back(columns[134] + " = " + std::to_string(e.deities9));
		v.push_back(columns[135] + " = " + std::to_string(e.deities10));
		v.push_back(columns[136] + " = " + std::to_string(e.deities11));
		v.push_back(columns[137] + " = " + std::to_string(e.deities12));
		v.push_back(columns[138] + " = " + std::to_string(e.deities13));
		v.push_back(columns[139] + " = " + std::to_string(e.deities14));
		v.push_back(columns[140] + " = " + std::to_string(e.deities15));
		v.push_back(columns[141] + " = " + std::to_string(e.deities16));
		v.push_back(columns[142] + " = " + std::to_string(e.field142));
		v.push_back(columns[143] + " = " + std::to_string(e.field143));
		v.push_back(columns[144] + " = " + std::to_string(e.new_icon));
		v.push_back(columns[145] + " = " + std::to_string(e.spellanim));
		v.push_back(columns[146] + " = " + std::to_string(e.uninterruptable));
		v.push_back(columns[147] + " = " + std::to_string(e.ResistDiff));
		v.push_back(columns[148] + " = " + std::to_string(e.dot_stacking_exempt));
		v.push_back(columns[149] + " = " + std::to_string(e.deleteable));
		v.push_back(columns[150] + " = " + std::to_string(e.RecourseLink));
		v.push_back(columns[151] + " = " + std::to_string(e.no_partial_resist));
		v.push_back(columns[152] + " = " + std::to_string(e.field152));
		v.push_back(columns[153] + " = " + std::to_string(e.field153));
		v.push_back(columns[154] + " = " + std::to_string(e.short_buff_box));
		v.push_back(columns[155] + " = " + std::to_string(e.descnum));
		v.push_back(columns[156] + " = " + std::to_string(e.typedescnum));
		v.push_back(columns[157] + " = " + std::to_string(e.effectdescnum));
		v.push_back(columns[158] + " = " + std::to_string(e.effectdescnum2));
		v.push_back(columns[159] + " = " + std::to_string(e.npc_no_los));
		v.push_back(columns[160] + " = " + std::to_string(e.field160));
		v.push_back(columns[161] + " = " + std::to_string(e.reflectable));
		v.push_back(columns[162] + " = " + std::to_string(e.bonushate));
		v.push_back(columns[163] + " = " + std::to_string(e.field163));
		v.push_back(columns[164] + " = " + std::to_string(e.field164));
		v.push_back(columns[165] + " = " + std::to_string(e.ldon_trap));
		v.push_back(columns[166] + " = " + std::to_string(e.EndurCost));
		v.push_back(columns[167] + " = " + std::to_string(e.EndurTimerIndex));
		v.push_back(columns[168] + " = " + std::to_string(e.IsDiscipline));
		v.push_back(columns[169] + " = " + std::to_string(e.field169));
		v.push_back(columns[170] + " = " + std::to_string(e.field170));
		v.push_back(columns[171] + " = " + std::to_string(e.field171));
		v.push_back(columns[172] + " = " + std::to_string(e.field172));
		v.push_back(columns[173] + " = " + std::to_string(e.HateAdded));
		v.push_back(columns[174] + " = " + std::to_string(e.EndurUpkeep));
		v.push_back(columns[175] + " = " + std::to_string(e.numhitstype));
		v.push_back(columns[176] + " = " + std::to_string(e.numhits));
		v.push_back(columns[177] + " = " + std::to_string(e.pvpresistbase));
		v.push_back(columns[178] + " = " + std::to_string(e.pvpresistcalc));
		v.push_back(columns[179] + " = " + std::to_string(e.pvpresistcap));
		v.push_back(columns[180] + " = " + std::to_string(e.spell_category));
		v.push_back(columns[181] + " = " + std::to_string(e.pvp_duration));
		v.push_back(columns[182] + " = " + std::to_string(e.pvp_duration_cap));
		v.push_back(columns[183] + " = " + std::to_string(e.pcnpc_only_flag));
		v.push_back(columns[184] + " = " + std::to_string(e.cast_not_standing));
		v.push_back(columns[185] + " = " + std::to_string(e.can_mgb));
		v.push_back(columns[186] + " = " + std::to_string(e.nodispell));
		v.push_back(columns[187] + " = " + std::to_string(e.npc_category));
		v.push_back(columns[188] + " = " + std::to_string(e.npc_usefulness));
		v.push_back(columns[189] + " = " + std::to_string(e.MinResist));
		v.push_back(columns[190] + " = " + std::to_string(e.MaxResist));
		v.push_back(columns[191] + " = " + std::to_string(e.viral_targets));
		v.push_back(columns[192] + " = " + std::to_string(e.viral_timer));
		v.push_back(columns[193] + " = " + std::to_string(e.nimbuseffect));
		v.push_back(columns[194] + " = " + std::to_string(e.ConeStartAngle));
		v.push_back(columns[195] + " = " + std::to_string(e.ConeStopAngle));
		v.push_back(columns[196] + " = " + std::to_string(e.sneaking));
		v.push_back(columns[197] + " = " + std::to_string(e.not_extendable));
		v.push_back(columns[198] + " = " + std::to_string(e.field198));
		v.push_back(columns[199] + " = " + std::to_string(e.field199));
		v.push_back(columns[200] + " = " + std::to_string(e.suspendable));
		v.push_back(columns[201] + " = " + std::to_string(e.viral_range));
		v.push_back(columns[202] + " = " + std::to_string(e.songcap));
		v.push_back(columns[203] + " = " + std::to_string(e.field203));
		v.push_back(columns[204] + " = " + std::to_string(e.field204));
		v.push_back(columns[205] + " = " + std::to_string(e.no_block));
		v.push_back(columns[206] + " = " + std::to_string(e.field206));
		v.push_back(columns[207] + " = " + std::to_string(e.spellgroup));
		v.push_back(columns[208] + " = " + std::to_string(e.rank));
		v.push_back(columns[209] + " = " + std::to_string(e.field209));
		v.push_back(columns[210] + " = " + std::to_string(e.field210));
		v.push_back(columns[211] + " = " + std::to_string(e.CastRestriction));
		v.push_back(columns[212] + " = " + std::to_string(e.allowrest));
		v.push_back(columns[213] + " = " + std::to_string(e.InCombat));
		v.push_back(columns[214] + " = " + std::to_string(e.OutofCombat));
		v.push_back(columns[215] + " = " + std::to_string(e.field215));
		v.push_back(columns[216] + " = " + std::to_string(e.field216));
		v.push_back(columns[217] + " = " + std::to_string(e.field217));
		v.push_back(columns[218] + " = " + std::to_string(e.aemaxtargets));
		v.push_back(columns[219] + " = " + std::to_string(e.maxtargets));
		v.push_back(columns[220] + " = " + std::to_string(e.field220));
		v.push_back(columns[221] + " = " + std::to_string(e.field221));
		v.push_back(columns[222] + " = " + std::to_string(e.field222));
		v.push_back(columns[223] + " = " + std::to_string(e.field223));
		v.push_back(columns[224] + " = " + std::to_string(e.persistdeath));
		v.push_back(columns[225] + " = " + std::to_string(e.field225));
		v.push_back(columns[226] + " = " + std::to_string(e.field226));
		v.push_back(columns[227] + " = " + std::to_string(e.min_dist));
		v.push_back(columns[228] + " = " + std::to_string(e.min_dist_mod));
		v.push_back(columns[229] + " = " + std::to_string(e.max_dist));
		v.push_back(columns[230] + " = " + std::to_string(e.max_dist_mod));
		v.push_back(columns[231] + " = " + std::to_string(e.min_range));
		v.push_back(columns[232] + " = " + std::to_string(e.field232));
		v.push_back(columns[233] + " = " + std::to_string(e.field233));
		v.push_back(columns[234] + " = " + std::to_string(e.field234));
		v.push_back(columns[235] + " = " + std::to_string(e.field235));
		v.push_back(columns[236] + " = " + std::to_string(e.field236));

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

	static SpellsNew InsertOne(
		Database& db,
		SpellsNew e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back("'" + Strings::Escape(e.player_1) + "'");
		v.push_back("'" + Strings::Escape(e.teleport_zone) + "'");
		v.push_back("'" + Strings::Escape(e.you_cast) + "'");
		v.push_back("'" + Strings::Escape(e.other_casts) + "'");
		v.push_back("'" + Strings::Escape(e.cast_on_you) + "'");
		v.push_back("'" + Strings::Escape(e.cast_on_other) + "'");
		v.push_back("'" + Strings::Escape(e.spell_fades) + "'");
		v.push_back(std::to_string(e.range));
		v.push_back(std::to_string(e.aoerange));
		v.push_back(std::to_string(e.pushback));
		v.push_back(std::to_string(e.pushup));
		v.push_back(std::to_string(e.cast_time));
		v.push_back(std::to_string(e.recovery_time));
		v.push_back(std::to_string(e.recast_time));
		v.push_back(std::to_string(e.buffdurationformula));
		v.push_back(std::to_string(e.buffduration));
		v.push_back(std::to_string(e.AEDuration));
		v.push_back(std::to_string(e.mana));
		v.push_back(std::to_string(e.effect_base_value1));
		v.push_back(std::to_string(e.effect_base_value2));
		v.push_back(std::to_string(e.effect_base_value3));
		v.push_back(std::to_string(e.effect_base_value4));
		v.push_back(std::to_string(e.effect_base_value5));
		v.push_back(std::to_string(e.effect_base_value6));
		v.push_back(std::to_string(e.effect_base_value7));
		v.push_back(std::to_string(e.effect_base_value8));
		v.push_back(std::to_string(e.effect_base_value9));
		v.push_back(std::to_string(e.effect_base_value10));
		v.push_back(std::to_string(e.effect_base_value11));
		v.push_back(std::to_string(e.effect_base_value12));
		v.push_back(std::to_string(e.effect_limit_value1));
		v.push_back(std::to_string(e.effect_limit_value2));
		v.push_back(std::to_string(e.effect_limit_value3));
		v.push_back(std::to_string(e.effect_limit_value4));
		v.push_back(std::to_string(e.effect_limit_value5));
		v.push_back(std::to_string(e.effect_limit_value6));
		v.push_back(std::to_string(e.effect_limit_value7));
		v.push_back(std::to_string(e.effect_limit_value8));
		v.push_back(std::to_string(e.effect_limit_value9));
		v.push_back(std::to_string(e.effect_limit_value10));
		v.push_back(std::to_string(e.effect_limit_value11));
		v.push_back(std::to_string(e.effect_limit_value12));
		v.push_back(std::to_string(e.max1));
		v.push_back(std::to_string(e.max2));
		v.push_back(std::to_string(e.max3));
		v.push_back(std::to_string(e.max4));
		v.push_back(std::to_string(e.max5));
		v.push_back(std::to_string(e.max6));
		v.push_back(std::to_string(e.max7));
		v.push_back(std::to_string(e.max8));
		v.push_back(std::to_string(e.max9));
		v.push_back(std::to_string(e.max10));
		v.push_back(std::to_string(e.max11));
		v.push_back(std::to_string(e.max12));
		v.push_back(std::to_string(e.icon));
		v.push_back(std::to_string(e.memicon));
		v.push_back(std::to_string(e.components1));
		v.push_back(std::to_string(e.components2));
		v.push_back(std::to_string(e.components3));
		v.push_back(std::to_string(e.components4));
		v.push_back(std::to_string(e.component_counts1));
		v.push_back(std::to_string(e.component_counts2));
		v.push_back(std::to_string(e.component_counts3));
		v.push_back(std::to_string(e.component_counts4));
		v.push_back(std::to_string(e.NoexpendReagent1));
		v.push_back(std::to_string(e.NoexpendReagent2));
		v.push_back(std::to_string(e.NoexpendReagent3));
		v.push_back(std::to_string(e.NoexpendReagent4));
		v.push_back(std::to_string(e.formula1));
		v.push_back(std::to_string(e.formula2));
		v.push_back(std::to_string(e.formula3));
		v.push_back(std::to_string(e.formula4));
		v.push_back(std::to_string(e.formula5));
		v.push_back(std::to_string(e.formula6));
		v.push_back(std::to_string(e.formula7));
		v.push_back(std::to_string(e.formula8));
		v.push_back(std::to_string(e.formula9));
		v.push_back(std::to_string(e.formula10));
		v.push_back(std::to_string(e.formula11));
		v.push_back(std::to_string(e.formula12));
		v.push_back(std::to_string(e.LightType));
		v.push_back(std::to_string(e.goodEffect));
		v.push_back(std::to_string(e.Activated));
		v.push_back(std::to_string(e.resisttype));
		v.push_back(std::to_string(e.effectid1));
		v.push_back(std::to_string(e.effectid2));
		v.push_back(std::to_string(e.effectid3));
		v.push_back(std::to_string(e.effectid4));
		v.push_back(std::to_string(e.effectid5));
		v.push_back(std::to_string(e.effectid6));
		v.push_back(std::to_string(e.effectid7));
		v.push_back(std::to_string(e.effectid8));
		v.push_back(std::to_string(e.effectid9));
		v.push_back(std::to_string(e.effectid10));
		v.push_back(std::to_string(e.effectid11));
		v.push_back(std::to_string(e.effectid12));
		v.push_back(std::to_string(e.targettype));
		v.push_back(std::to_string(e.basediff));
		v.push_back(std::to_string(e.skill));
		v.push_back(std::to_string(e.zonetype));
		v.push_back(std::to_string(e.EnvironmentType));
		v.push_back(std::to_string(e.TimeOfDay));
		v.push_back(std::to_string(e.classes1));
		v.push_back(std::to_string(e.classes2));
		v.push_back(std::to_string(e.classes3));
		v.push_back(std::to_string(e.classes4));
		v.push_back(std::to_string(e.classes5));
		v.push_back(std::to_string(e.classes6));
		v.push_back(std::to_string(e.classes7));
		v.push_back(std::to_string(e.classes8));
		v.push_back(std::to_string(e.classes9));
		v.push_back(std::to_string(e.classes10));
		v.push_back(std::to_string(e.classes11));
		v.push_back(std::to_string(e.classes12));
		v.push_back(std::to_string(e.classes13));
		v.push_back(std::to_string(e.classes14));
		v.push_back(std::to_string(e.classes15));
		v.push_back(std::to_string(e.classes16));
		v.push_back(std::to_string(e.CastingAnim));
		v.push_back(std::to_string(e.TargetAnim));
		v.push_back(std::to_string(e.TravelType));
		v.push_back(std::to_string(e.SpellAffectIndex));
		v.push_back(std::to_string(e.disallow_sit));
		v.push_back(std::to_string(e.deities0));
		v.push_back(std::to_string(e.deities1));
		v.push_back(std::to_string(e.deities2));
		v.push_back(std::to_string(e.deities3));
		v.push_back(std::to_string(e.deities4));
		v.push_back(std::to_string(e.deities5));
		v.push_back(std::to_string(e.deities6));
		v.push_back(std::to_string(e.deities7));
		v.push_back(std::to_string(e.deities8));
		v.push_back(std::to_string(e.deities9));
		v.push_back(std::to_string(e.deities10));
		v.push_back(std::to_string(e.deities11));
		v.push_back(std::to_string(e.deities12));
		v.push_back(std::to_string(e.deities13));
		v.push_back(std::to_string(e.deities14));
		v.push_back(std::to_string(e.deities15));
		v.push_back(std::to_string(e.deities16));
		v.push_back(std::to_string(e.field142));
		v.push_back(std::to_string(e.field143));
		v.push_back(std::to_string(e.new_icon));
		v.push_back(std::to_string(e.spellanim));
		v.push_back(std::to_string(e.uninterruptable));
		v.push_back(std::to_string(e.ResistDiff));
		v.push_back(std::to_string(e.dot_stacking_exempt));
		v.push_back(std::to_string(e.deleteable));
		v.push_back(std::to_string(e.RecourseLink));
		v.push_back(std::to_string(e.no_partial_resist));
		v.push_back(std::to_string(e.field152));
		v.push_back(std::to_string(e.field153));
		v.push_back(std::to_string(e.short_buff_box));
		v.push_back(std::to_string(e.descnum));
		v.push_back(std::to_string(e.typedescnum));
		v.push_back(std::to_string(e.effectdescnum));
		v.push_back(std::to_string(e.effectdescnum2));
		v.push_back(std::to_string(e.npc_no_los));
		v.push_back(std::to_string(e.field160));
		v.push_back(std::to_string(e.reflectable));
		v.push_back(std::to_string(e.bonushate));
		v.push_back(std::to_string(e.field163));
		v.push_back(std::to_string(e.field164));
		v.push_back(std::to_string(e.ldon_trap));
		v.push_back(std::to_string(e.EndurCost));
		v.push_back(std::to_string(e.EndurTimerIndex));
		v.push_back(std::to_string(e.IsDiscipline));
		v.push_back(std::to_string(e.field169));
		v.push_back(std::to_string(e.field170));
		v.push_back(std::to_string(e.field171));
		v.push_back(std::to_string(e.field172));
		v.push_back(std::to_string(e.HateAdded));
		v.push_back(std::to_string(e.EndurUpkeep));
		v.push_back(std::to_string(e.numhitstype));
		v.push_back(std::to_string(e.numhits));
		v.push_back(std::to_string(e.pvpresistbase));
		v.push_back(std::to_string(e.pvpresistcalc));
		v.push_back(std::to_string(e.pvpresistcap));
		v.push_back(std::to_string(e.spell_category));
		v.push_back(std::to_string(e.pvp_duration));
		v.push_back(std::to_string(e.pvp_duration_cap));
		v.push_back(std::to_string(e.pcnpc_only_flag));
		v.push_back(std::to_string(e.cast_not_standing));
		v.push_back(std::to_string(e.can_mgb));
		v.push_back(std::to_string(e.nodispell));
		v.push_back(std::to_string(e.npc_category));
		v.push_back(std::to_string(e.npc_usefulness));
		v.push_back(std::to_string(e.MinResist));
		v.push_back(std::to_string(e.MaxResist));
		v.push_back(std::to_string(e.viral_targets));
		v.push_back(std::to_string(e.viral_timer));
		v.push_back(std::to_string(e.nimbuseffect));
		v.push_back(std::to_string(e.ConeStartAngle));
		v.push_back(std::to_string(e.ConeStopAngle));
		v.push_back(std::to_string(e.sneaking));
		v.push_back(std::to_string(e.not_extendable));
		v.push_back(std::to_string(e.field198));
		v.push_back(std::to_string(e.field199));
		v.push_back(std::to_string(e.suspendable));
		v.push_back(std::to_string(e.viral_range));
		v.push_back(std::to_string(e.songcap));
		v.push_back(std::to_string(e.field203));
		v.push_back(std::to_string(e.field204));
		v.push_back(std::to_string(e.no_block));
		v.push_back(std::to_string(e.field206));
		v.push_back(std::to_string(e.spellgroup));
		v.push_back(std::to_string(e.rank));
		v.push_back(std::to_string(e.field209));
		v.push_back(std::to_string(e.field210));
		v.push_back(std::to_string(e.CastRestriction));
		v.push_back(std::to_string(e.allowrest));
		v.push_back(std::to_string(e.InCombat));
		v.push_back(std::to_string(e.OutofCombat));
		v.push_back(std::to_string(e.field215));
		v.push_back(std::to_string(e.field216));
		v.push_back(std::to_string(e.field217));
		v.push_back(std::to_string(e.aemaxtargets));
		v.push_back(std::to_string(e.maxtargets));
		v.push_back(std::to_string(e.field220));
		v.push_back(std::to_string(e.field221));
		v.push_back(std::to_string(e.field222));
		v.push_back(std::to_string(e.field223));
		v.push_back(std::to_string(e.persistdeath));
		v.push_back(std::to_string(e.field225));
		v.push_back(std::to_string(e.field226));
		v.push_back(std::to_string(e.min_dist));
		v.push_back(std::to_string(e.min_dist_mod));
		v.push_back(std::to_string(e.max_dist));
		v.push_back(std::to_string(e.max_dist_mod));
		v.push_back(std::to_string(e.min_range));
		v.push_back(std::to_string(e.field232));
		v.push_back(std::to_string(e.field233));
		v.push_back(std::to_string(e.field234));
		v.push_back(std::to_string(e.field235));
		v.push_back(std::to_string(e.field236));

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
		const std::vector<SpellsNew> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back("'" + Strings::Escape(e.player_1) + "'");
			v.push_back("'" + Strings::Escape(e.teleport_zone) + "'");
			v.push_back("'" + Strings::Escape(e.you_cast) + "'");
			v.push_back("'" + Strings::Escape(e.other_casts) + "'");
			v.push_back("'" + Strings::Escape(e.cast_on_you) + "'");
			v.push_back("'" + Strings::Escape(e.cast_on_other) + "'");
			v.push_back("'" + Strings::Escape(e.spell_fades) + "'");
			v.push_back(std::to_string(e.range));
			v.push_back(std::to_string(e.aoerange));
			v.push_back(std::to_string(e.pushback));
			v.push_back(std::to_string(e.pushup));
			v.push_back(std::to_string(e.cast_time));
			v.push_back(std::to_string(e.recovery_time));
			v.push_back(std::to_string(e.recast_time));
			v.push_back(std::to_string(e.buffdurationformula));
			v.push_back(std::to_string(e.buffduration));
			v.push_back(std::to_string(e.AEDuration));
			v.push_back(std::to_string(e.mana));
			v.push_back(std::to_string(e.effect_base_value1));
			v.push_back(std::to_string(e.effect_base_value2));
			v.push_back(std::to_string(e.effect_base_value3));
			v.push_back(std::to_string(e.effect_base_value4));
			v.push_back(std::to_string(e.effect_base_value5));
			v.push_back(std::to_string(e.effect_base_value6));
			v.push_back(std::to_string(e.effect_base_value7));
			v.push_back(std::to_string(e.effect_base_value8));
			v.push_back(std::to_string(e.effect_base_value9));
			v.push_back(std::to_string(e.effect_base_value10));
			v.push_back(std::to_string(e.effect_base_value11));
			v.push_back(std::to_string(e.effect_base_value12));
			v.push_back(std::to_string(e.effect_limit_value1));
			v.push_back(std::to_string(e.effect_limit_value2));
			v.push_back(std::to_string(e.effect_limit_value3));
			v.push_back(std::to_string(e.effect_limit_value4));
			v.push_back(std::to_string(e.effect_limit_value5));
			v.push_back(std::to_string(e.effect_limit_value6));
			v.push_back(std::to_string(e.effect_limit_value7));
			v.push_back(std::to_string(e.effect_limit_value8));
			v.push_back(std::to_string(e.effect_limit_value9));
			v.push_back(std::to_string(e.effect_limit_value10));
			v.push_back(std::to_string(e.effect_limit_value11));
			v.push_back(std::to_string(e.effect_limit_value12));
			v.push_back(std::to_string(e.max1));
			v.push_back(std::to_string(e.max2));
			v.push_back(std::to_string(e.max3));
			v.push_back(std::to_string(e.max4));
			v.push_back(std::to_string(e.max5));
			v.push_back(std::to_string(e.max6));
			v.push_back(std::to_string(e.max7));
			v.push_back(std::to_string(e.max8));
			v.push_back(std::to_string(e.max9));
			v.push_back(std::to_string(e.max10));
			v.push_back(std::to_string(e.max11));
			v.push_back(std::to_string(e.max12));
			v.push_back(std::to_string(e.icon));
			v.push_back(std::to_string(e.memicon));
			v.push_back(std::to_string(e.components1));
			v.push_back(std::to_string(e.components2));
			v.push_back(std::to_string(e.components3));
			v.push_back(std::to_string(e.components4));
			v.push_back(std::to_string(e.component_counts1));
			v.push_back(std::to_string(e.component_counts2));
			v.push_back(std::to_string(e.component_counts3));
			v.push_back(std::to_string(e.component_counts4));
			v.push_back(std::to_string(e.NoexpendReagent1));
			v.push_back(std::to_string(e.NoexpendReagent2));
			v.push_back(std::to_string(e.NoexpendReagent3));
			v.push_back(std::to_string(e.NoexpendReagent4));
			v.push_back(std::to_string(e.formula1));
			v.push_back(std::to_string(e.formula2));
			v.push_back(std::to_string(e.formula3));
			v.push_back(std::to_string(e.formula4));
			v.push_back(std::to_string(e.formula5));
			v.push_back(std::to_string(e.formula6));
			v.push_back(std::to_string(e.formula7));
			v.push_back(std::to_string(e.formula8));
			v.push_back(std::to_string(e.formula9));
			v.push_back(std::to_string(e.formula10));
			v.push_back(std::to_string(e.formula11));
			v.push_back(std::to_string(e.formula12));
			v.push_back(std::to_string(e.LightType));
			v.push_back(std::to_string(e.goodEffect));
			v.push_back(std::to_string(e.Activated));
			v.push_back(std::to_string(e.resisttype));
			v.push_back(std::to_string(e.effectid1));
			v.push_back(std::to_string(e.effectid2));
			v.push_back(std::to_string(e.effectid3));
			v.push_back(std::to_string(e.effectid4));
			v.push_back(std::to_string(e.effectid5));
			v.push_back(std::to_string(e.effectid6));
			v.push_back(std::to_string(e.effectid7));
			v.push_back(std::to_string(e.effectid8));
			v.push_back(std::to_string(e.effectid9));
			v.push_back(std::to_string(e.effectid10));
			v.push_back(std::to_string(e.effectid11));
			v.push_back(std::to_string(e.effectid12));
			v.push_back(std::to_string(e.targettype));
			v.push_back(std::to_string(e.basediff));
			v.push_back(std::to_string(e.skill));
			v.push_back(std::to_string(e.zonetype));
			v.push_back(std::to_string(e.EnvironmentType));
			v.push_back(std::to_string(e.TimeOfDay));
			v.push_back(std::to_string(e.classes1));
			v.push_back(std::to_string(e.classes2));
			v.push_back(std::to_string(e.classes3));
			v.push_back(std::to_string(e.classes4));
			v.push_back(std::to_string(e.classes5));
			v.push_back(std::to_string(e.classes6));
			v.push_back(std::to_string(e.classes7));
			v.push_back(std::to_string(e.classes8));
			v.push_back(std::to_string(e.classes9));
			v.push_back(std::to_string(e.classes10));
			v.push_back(std::to_string(e.classes11));
			v.push_back(std::to_string(e.classes12));
			v.push_back(std::to_string(e.classes13));
			v.push_back(std::to_string(e.classes14));
			v.push_back(std::to_string(e.classes15));
			v.push_back(std::to_string(e.classes16));
			v.push_back(std::to_string(e.CastingAnim));
			v.push_back(std::to_string(e.TargetAnim));
			v.push_back(std::to_string(e.TravelType));
			v.push_back(std::to_string(e.SpellAffectIndex));
			v.push_back(std::to_string(e.disallow_sit));
			v.push_back(std::to_string(e.deities0));
			v.push_back(std::to_string(e.deities1));
			v.push_back(std::to_string(e.deities2));
			v.push_back(std::to_string(e.deities3));
			v.push_back(std::to_string(e.deities4));
			v.push_back(std::to_string(e.deities5));
			v.push_back(std::to_string(e.deities6));
			v.push_back(std::to_string(e.deities7));
			v.push_back(std::to_string(e.deities8));
			v.push_back(std::to_string(e.deities9));
			v.push_back(std::to_string(e.deities10));
			v.push_back(std::to_string(e.deities11));
			v.push_back(std::to_string(e.deities12));
			v.push_back(std::to_string(e.deities13));
			v.push_back(std::to_string(e.deities14));
			v.push_back(std::to_string(e.deities15));
			v.push_back(std::to_string(e.deities16));
			v.push_back(std::to_string(e.field142));
			v.push_back(std::to_string(e.field143));
			v.push_back(std::to_string(e.new_icon));
			v.push_back(std::to_string(e.spellanim));
			v.push_back(std::to_string(e.uninterruptable));
			v.push_back(std::to_string(e.ResistDiff));
			v.push_back(std::to_string(e.dot_stacking_exempt));
			v.push_back(std::to_string(e.deleteable));
			v.push_back(std::to_string(e.RecourseLink));
			v.push_back(std::to_string(e.no_partial_resist));
			v.push_back(std::to_string(e.field152));
			v.push_back(std::to_string(e.field153));
			v.push_back(std::to_string(e.short_buff_box));
			v.push_back(std::to_string(e.descnum));
			v.push_back(std::to_string(e.typedescnum));
			v.push_back(std::to_string(e.effectdescnum));
			v.push_back(std::to_string(e.effectdescnum2));
			v.push_back(std::to_string(e.npc_no_los));
			v.push_back(std::to_string(e.field160));
			v.push_back(std::to_string(e.reflectable));
			v.push_back(std::to_string(e.bonushate));
			v.push_back(std::to_string(e.field163));
			v.push_back(std::to_string(e.field164));
			v.push_back(std::to_string(e.ldon_trap));
			v.push_back(std::to_string(e.EndurCost));
			v.push_back(std::to_string(e.EndurTimerIndex));
			v.push_back(std::to_string(e.IsDiscipline));
			v.push_back(std::to_string(e.field169));
			v.push_back(std::to_string(e.field170));
			v.push_back(std::to_string(e.field171));
			v.push_back(std::to_string(e.field172));
			v.push_back(std::to_string(e.HateAdded));
			v.push_back(std::to_string(e.EndurUpkeep));
			v.push_back(std::to_string(e.numhitstype));
			v.push_back(std::to_string(e.numhits));
			v.push_back(std::to_string(e.pvpresistbase));
			v.push_back(std::to_string(e.pvpresistcalc));
			v.push_back(std::to_string(e.pvpresistcap));
			v.push_back(std::to_string(e.spell_category));
			v.push_back(std::to_string(e.pvp_duration));
			v.push_back(std::to_string(e.pvp_duration_cap));
			v.push_back(std::to_string(e.pcnpc_only_flag));
			v.push_back(std::to_string(e.cast_not_standing));
			v.push_back(std::to_string(e.can_mgb));
			v.push_back(std::to_string(e.nodispell));
			v.push_back(std::to_string(e.npc_category));
			v.push_back(std::to_string(e.npc_usefulness));
			v.push_back(std::to_string(e.MinResist));
			v.push_back(std::to_string(e.MaxResist));
			v.push_back(std::to_string(e.viral_targets));
			v.push_back(std::to_string(e.viral_timer));
			v.push_back(std::to_string(e.nimbuseffect));
			v.push_back(std::to_string(e.ConeStartAngle));
			v.push_back(std::to_string(e.ConeStopAngle));
			v.push_back(std::to_string(e.sneaking));
			v.push_back(std::to_string(e.not_extendable));
			v.push_back(std::to_string(e.field198));
			v.push_back(std::to_string(e.field199));
			v.push_back(std::to_string(e.suspendable));
			v.push_back(std::to_string(e.viral_range));
			v.push_back(std::to_string(e.songcap));
			v.push_back(std::to_string(e.field203));
			v.push_back(std::to_string(e.field204));
			v.push_back(std::to_string(e.no_block));
			v.push_back(std::to_string(e.field206));
			v.push_back(std::to_string(e.spellgroup));
			v.push_back(std::to_string(e.rank));
			v.push_back(std::to_string(e.field209));
			v.push_back(std::to_string(e.field210));
			v.push_back(std::to_string(e.CastRestriction));
			v.push_back(std::to_string(e.allowrest));
			v.push_back(std::to_string(e.InCombat));
			v.push_back(std::to_string(e.OutofCombat));
			v.push_back(std::to_string(e.field215));
			v.push_back(std::to_string(e.field216));
			v.push_back(std::to_string(e.field217));
			v.push_back(std::to_string(e.aemaxtargets));
			v.push_back(std::to_string(e.maxtargets));
			v.push_back(std::to_string(e.field220));
			v.push_back(std::to_string(e.field221));
			v.push_back(std::to_string(e.field222));
			v.push_back(std::to_string(e.field223));
			v.push_back(std::to_string(e.persistdeath));
			v.push_back(std::to_string(e.field225));
			v.push_back(std::to_string(e.field226));
			v.push_back(std::to_string(e.min_dist));
			v.push_back(std::to_string(e.min_dist_mod));
			v.push_back(std::to_string(e.max_dist));
			v.push_back(std::to_string(e.max_dist_mod));
			v.push_back(std::to_string(e.min_range));
			v.push_back(std::to_string(e.field232));
			v.push_back(std::to_string(e.field233));
			v.push_back(std::to_string(e.field234));
			v.push_back(std::to_string(e.field235));
			v.push_back(std::to_string(e.field236));

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
			SpellsNew e{};

			e.id                   = atoi(row[0]);
			e.name                 = row[1] ? row[1] : "";
			e.player_1             = row[2] ? row[2] : "";
			e.teleport_zone        = row[3] ? row[3] : "";
			e.you_cast             = row[4] ? row[4] : "";
			e.other_casts          = row[5] ? row[5] : "";
			e.cast_on_you          = row[6] ? row[6] : "";
			e.cast_on_other        = row[7] ? row[7] : "";
			e.spell_fades          = row[8] ? row[8] : "";
			e.range                = atoi(row[9]);
			e.aoerange             = atoi(row[10]);
			e.pushback             = atoi(row[11]);
			e.pushup               = atoi(row[12]);
			e.cast_time            = atoi(row[13]);
			e.recovery_time        = atoi(row[14]);
			e.recast_time          = atoi(row[15]);
			e.buffdurationformula  = atoi(row[16]);
			e.buffduration         = atoi(row[17]);
			e.AEDuration           = atoi(row[18]);
			e.mana                 = atoi(row[19]);
			e.effect_base_value1   = atoi(row[20]);
			e.effect_base_value2   = atoi(row[21]);
			e.effect_base_value3   = atoi(row[22]);
			e.effect_base_value4   = atoi(row[23]);
			e.effect_base_value5   = atoi(row[24]);
			e.effect_base_value6   = atoi(row[25]);
			e.effect_base_value7   = atoi(row[26]);
			e.effect_base_value8   = atoi(row[27]);
			e.effect_base_value9   = atoi(row[28]);
			e.effect_base_value10  = atoi(row[29]);
			e.effect_base_value11  = atoi(row[30]);
			e.effect_base_value12  = atoi(row[31]);
			e.effect_limit_value1  = atoi(row[32]);
			e.effect_limit_value2  = atoi(row[33]);
			e.effect_limit_value3  = atoi(row[34]);
			e.effect_limit_value4  = atoi(row[35]);
			e.effect_limit_value5  = atoi(row[36]);
			e.effect_limit_value6  = atoi(row[37]);
			e.effect_limit_value7  = atoi(row[38]);
			e.effect_limit_value8  = atoi(row[39]);
			e.effect_limit_value9  = atoi(row[40]);
			e.effect_limit_value10 = atoi(row[41]);
			e.effect_limit_value11 = atoi(row[42]);
			e.effect_limit_value12 = atoi(row[43]);
			e.max1                 = atoi(row[44]);
			e.max2                 = atoi(row[45]);
			e.max3                 = atoi(row[46]);
			e.max4                 = atoi(row[47]);
			e.max5                 = atoi(row[48]);
			e.max6                 = atoi(row[49]);
			e.max7                 = atoi(row[50]);
			e.max8                 = atoi(row[51]);
			e.max9                 = atoi(row[52]);
			e.max10                = atoi(row[53]);
			e.max11                = atoi(row[54]);
			e.max12                = atoi(row[55]);
			e.icon                 = atoi(row[56]);
			e.memicon              = atoi(row[57]);
			e.components1          = atoi(row[58]);
			e.components2          = atoi(row[59]);
			e.components3          = atoi(row[60]);
			e.components4          = atoi(row[61]);
			e.component_counts1    = atoi(row[62]);
			e.component_counts2    = atoi(row[63]);
			e.component_counts3    = atoi(row[64]);
			e.component_counts4    = atoi(row[65]);
			e.NoexpendReagent1     = atoi(row[66]);
			e.NoexpendReagent2     = atoi(row[67]);
			e.NoexpendReagent3     = atoi(row[68]);
			e.NoexpendReagent4     = atoi(row[69]);
			e.formula1             = atoi(row[70]);
			e.formula2             = atoi(row[71]);
			e.formula3             = atoi(row[72]);
			e.formula4             = atoi(row[73]);
			e.formula5             = atoi(row[74]);
			e.formula6             = atoi(row[75]);
			e.formula7             = atoi(row[76]);
			e.formula8             = atoi(row[77]);
			e.formula9             = atoi(row[78]);
			e.formula10            = atoi(row[79]);
			e.formula11            = atoi(row[80]);
			e.formula12            = atoi(row[81]);
			e.LightType            = atoi(row[82]);
			e.goodEffect           = atoi(row[83]);
			e.Activated            = atoi(row[84]);
			e.resisttype           = atoi(row[85]);
			e.effectid1            = atoi(row[86]);
			e.effectid2            = atoi(row[87]);
			e.effectid3            = atoi(row[88]);
			e.effectid4            = atoi(row[89]);
			e.effectid5            = atoi(row[90]);
			e.effectid6            = atoi(row[91]);
			e.effectid7            = atoi(row[92]);
			e.effectid8            = atoi(row[93]);
			e.effectid9            = atoi(row[94]);
			e.effectid10           = atoi(row[95]);
			e.effectid11           = atoi(row[96]);
			e.effectid12           = atoi(row[97]);
			e.targettype           = atoi(row[98]);
			e.basediff             = atoi(row[99]);
			e.skill                = atoi(row[100]);
			e.zonetype             = atoi(row[101]);
			e.EnvironmentType      = atoi(row[102]);
			e.TimeOfDay            = atoi(row[103]);
			e.classes1             = atoi(row[104]);
			e.classes2             = atoi(row[105]);
			e.classes3             = atoi(row[106]);
			e.classes4             = atoi(row[107]);
			e.classes5             = atoi(row[108]);
			e.classes6             = atoi(row[109]);
			e.classes7             = atoi(row[110]);
			e.classes8             = atoi(row[111]);
			e.classes9             = atoi(row[112]);
			e.classes10            = atoi(row[113]);
			e.classes11            = atoi(row[114]);
			e.classes12            = atoi(row[115]);
			e.classes13            = atoi(row[116]);
			e.classes14            = atoi(row[117]);
			e.classes15            = atoi(row[118]);
			e.classes16            = atoi(row[119]);
			e.CastingAnim          = atoi(row[120]);
			e.TargetAnim           = atoi(row[121]);
			e.TravelType           = atoi(row[122]);
			e.SpellAffectIndex     = atoi(row[123]);
			e.disallow_sit         = atoi(row[124]);
			e.deities0             = atoi(row[125]);
			e.deities1             = atoi(row[126]);
			e.deities2             = atoi(row[127]);
			e.deities3             = atoi(row[128]);
			e.deities4             = atoi(row[129]);
			e.deities5             = atoi(row[130]);
			e.deities6             = atoi(row[131]);
			e.deities7             = atoi(row[132]);
			e.deities8             = atoi(row[133]);
			e.deities9             = atoi(row[134]);
			e.deities10            = atoi(row[135]);
			e.deities11            = atoi(row[136]);
			e.deities12            = atoi(row[137]);
			e.deities13            = atoi(row[138]);
			e.deities14            = atoi(row[139]);
			e.deities15            = atoi(row[140]);
			e.deities16            = atoi(row[141]);
			e.field142             = atoi(row[142]);
			e.field143             = atoi(row[143]);
			e.new_icon             = atoi(row[144]);
			e.spellanim            = atoi(row[145]);
			e.uninterruptable      = atoi(row[146]);
			e.ResistDiff           = atoi(row[147]);
			e.dot_stacking_exempt  = atoi(row[148]);
			e.deleteable           = atoi(row[149]);
			e.RecourseLink         = atoi(row[150]);
			e.no_partial_resist    = atoi(row[151]);
			e.field152             = atoi(row[152]);
			e.field153             = atoi(row[153]);
			e.short_buff_box       = atoi(row[154]);
			e.descnum              = atoi(row[155]);
			e.typedescnum          = atoi(row[156]);
			e.effectdescnum        = atoi(row[157]);
			e.effectdescnum2       = atoi(row[158]);
			e.npc_no_los           = atoi(row[159]);
			e.field160             = atoi(row[160]);
			e.reflectable          = atoi(row[161]);
			e.bonushate            = atoi(row[162]);
			e.field163             = atoi(row[163]);
			e.field164             = atoi(row[164]);
			e.ldon_trap            = atoi(row[165]);
			e.EndurCost            = atoi(row[166]);
			e.EndurTimerIndex      = atoi(row[167]);
			e.IsDiscipline         = atoi(row[168]);
			e.field169             = atoi(row[169]);
			e.field170             = atoi(row[170]);
			e.field171             = atoi(row[171]);
			e.field172             = atoi(row[172]);
			e.HateAdded            = atoi(row[173]);
			e.EndurUpkeep          = atoi(row[174]);
			e.numhitstype          = atoi(row[175]);
			e.numhits              = atoi(row[176]);
			e.pvpresistbase        = atoi(row[177]);
			e.pvpresistcalc        = atoi(row[178]);
			e.pvpresistcap         = atoi(row[179]);
			e.spell_category       = atoi(row[180]);
			e.pvp_duration         = atoi(row[181]);
			e.pvp_duration_cap     = atoi(row[182]);
			e.pcnpc_only_flag      = atoi(row[183]);
			e.cast_not_standing    = atoi(row[184]);
			e.can_mgb              = atoi(row[185]);
			e.nodispell            = atoi(row[186]);
			e.npc_category         = atoi(row[187]);
			e.npc_usefulness       = atoi(row[188]);
			e.MinResist            = atoi(row[189]);
			e.MaxResist            = atoi(row[190]);
			e.viral_targets        = atoi(row[191]);
			e.viral_timer          = atoi(row[192]);
			e.nimbuseffect         = atoi(row[193]);
			e.ConeStartAngle       = atoi(row[194]);
			e.ConeStopAngle        = atoi(row[195]);
			e.sneaking             = atoi(row[196]);
			e.not_extendable       = atoi(row[197]);
			e.field198             = atoi(row[198]);
			e.field199             = atoi(row[199]);
			e.suspendable          = atoi(row[200]);
			e.viral_range          = atoi(row[201]);
			e.songcap              = atoi(row[202]);
			e.field203             = atoi(row[203]);
			e.field204             = atoi(row[204]);
			e.no_block             = atoi(row[205]);
			e.field206             = atoi(row[206]);
			e.spellgroup           = atoi(row[207]);
			e.rank                 = atoi(row[208]);
			e.field209             = atoi(row[209]);
			e.field210             = atoi(row[210]);
			e.CastRestriction      = atoi(row[211]);
			e.allowrest            = atoi(row[212]);
			e.InCombat             = atoi(row[213]);
			e.OutofCombat          = atoi(row[214]);
			e.field215             = atoi(row[215]);
			e.field216             = atoi(row[216]);
			e.field217             = atoi(row[217]);
			e.aemaxtargets         = atoi(row[218]);
			e.maxtargets           = atoi(row[219]);
			e.field220             = atoi(row[220]);
			e.field221             = atoi(row[221]);
			e.field222             = atoi(row[222]);
			e.field223             = atoi(row[223]);
			e.persistdeath         = atoi(row[224]);
			e.field225             = atoi(row[225]);
			e.field226             = atoi(row[226]);
			e.min_dist             = static_cast<float>(atof(row[227]));
			e.min_dist_mod         = static_cast<float>(atof(row[228]));
			e.max_dist             = static_cast<float>(atof(row[229]));
			e.max_dist_mod         = static_cast<float>(atof(row[230]));
			e.min_range            = atoi(row[231]);
			e.field232             = atoi(row[232]);
			e.field233             = atoi(row[233]);
			e.field234             = atoi(row[234]);
			e.field235             = atoi(row[235]);
			e.field236             = atoi(row[236]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<SpellsNew> GetWhere(Database& db, const std::string &where_filter)
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
			SpellsNew e{};

			e.id                   = atoi(row[0]);
			e.name                 = row[1] ? row[1] : "";
			e.player_1             = row[2] ? row[2] : "";
			e.teleport_zone        = row[3] ? row[3] : "";
			e.you_cast             = row[4] ? row[4] : "";
			e.other_casts          = row[5] ? row[5] : "";
			e.cast_on_you          = row[6] ? row[6] : "";
			e.cast_on_other        = row[7] ? row[7] : "";
			e.spell_fades          = row[8] ? row[8] : "";
			e.range                = atoi(row[9]);
			e.aoerange             = atoi(row[10]);
			e.pushback             = atoi(row[11]);
			e.pushup               = atoi(row[12]);
			e.cast_time            = atoi(row[13]);
			e.recovery_time        = atoi(row[14]);
			e.recast_time          = atoi(row[15]);
			e.buffdurationformula  = atoi(row[16]);
			e.buffduration         = atoi(row[17]);
			e.AEDuration           = atoi(row[18]);
			e.mana                 = atoi(row[19]);
			e.effect_base_value1   = atoi(row[20]);
			e.effect_base_value2   = atoi(row[21]);
			e.effect_base_value3   = atoi(row[22]);
			e.effect_base_value4   = atoi(row[23]);
			e.effect_base_value5   = atoi(row[24]);
			e.effect_base_value6   = atoi(row[25]);
			e.effect_base_value7   = atoi(row[26]);
			e.effect_base_value8   = atoi(row[27]);
			e.effect_base_value9   = atoi(row[28]);
			e.effect_base_value10  = atoi(row[29]);
			e.effect_base_value11  = atoi(row[30]);
			e.effect_base_value12  = atoi(row[31]);
			e.effect_limit_value1  = atoi(row[32]);
			e.effect_limit_value2  = atoi(row[33]);
			e.effect_limit_value3  = atoi(row[34]);
			e.effect_limit_value4  = atoi(row[35]);
			e.effect_limit_value5  = atoi(row[36]);
			e.effect_limit_value6  = atoi(row[37]);
			e.effect_limit_value7  = atoi(row[38]);
			e.effect_limit_value8  = atoi(row[39]);
			e.effect_limit_value9  = atoi(row[40]);
			e.effect_limit_value10 = atoi(row[41]);
			e.effect_limit_value11 = atoi(row[42]);
			e.effect_limit_value12 = atoi(row[43]);
			e.max1                 = atoi(row[44]);
			e.max2                 = atoi(row[45]);
			e.max3                 = atoi(row[46]);
			e.max4                 = atoi(row[47]);
			e.max5                 = atoi(row[48]);
			e.max6                 = atoi(row[49]);
			e.max7                 = atoi(row[50]);
			e.max8                 = atoi(row[51]);
			e.max9                 = atoi(row[52]);
			e.max10                = atoi(row[53]);
			e.max11                = atoi(row[54]);
			e.max12                = atoi(row[55]);
			e.icon                 = atoi(row[56]);
			e.memicon              = atoi(row[57]);
			e.components1          = atoi(row[58]);
			e.components2          = atoi(row[59]);
			e.components3          = atoi(row[60]);
			e.components4          = atoi(row[61]);
			e.component_counts1    = atoi(row[62]);
			e.component_counts2    = atoi(row[63]);
			e.component_counts3    = atoi(row[64]);
			e.component_counts4    = atoi(row[65]);
			e.NoexpendReagent1     = atoi(row[66]);
			e.NoexpendReagent2     = atoi(row[67]);
			e.NoexpendReagent3     = atoi(row[68]);
			e.NoexpendReagent4     = atoi(row[69]);
			e.formula1             = atoi(row[70]);
			e.formula2             = atoi(row[71]);
			e.formula3             = atoi(row[72]);
			e.formula4             = atoi(row[73]);
			e.formula5             = atoi(row[74]);
			e.formula6             = atoi(row[75]);
			e.formula7             = atoi(row[76]);
			e.formula8             = atoi(row[77]);
			e.formula9             = atoi(row[78]);
			e.formula10            = atoi(row[79]);
			e.formula11            = atoi(row[80]);
			e.formula12            = atoi(row[81]);
			e.LightType            = atoi(row[82]);
			e.goodEffect           = atoi(row[83]);
			e.Activated            = atoi(row[84]);
			e.resisttype           = atoi(row[85]);
			e.effectid1            = atoi(row[86]);
			e.effectid2            = atoi(row[87]);
			e.effectid3            = atoi(row[88]);
			e.effectid4            = atoi(row[89]);
			e.effectid5            = atoi(row[90]);
			e.effectid6            = atoi(row[91]);
			e.effectid7            = atoi(row[92]);
			e.effectid8            = atoi(row[93]);
			e.effectid9            = atoi(row[94]);
			e.effectid10           = atoi(row[95]);
			e.effectid11           = atoi(row[96]);
			e.effectid12           = atoi(row[97]);
			e.targettype           = atoi(row[98]);
			e.basediff             = atoi(row[99]);
			e.skill                = atoi(row[100]);
			e.zonetype             = atoi(row[101]);
			e.EnvironmentType      = atoi(row[102]);
			e.TimeOfDay            = atoi(row[103]);
			e.classes1             = atoi(row[104]);
			e.classes2             = atoi(row[105]);
			e.classes3             = atoi(row[106]);
			e.classes4             = atoi(row[107]);
			e.classes5             = atoi(row[108]);
			e.classes6             = atoi(row[109]);
			e.classes7             = atoi(row[110]);
			e.classes8             = atoi(row[111]);
			e.classes9             = atoi(row[112]);
			e.classes10            = atoi(row[113]);
			e.classes11            = atoi(row[114]);
			e.classes12            = atoi(row[115]);
			e.classes13            = atoi(row[116]);
			e.classes14            = atoi(row[117]);
			e.classes15            = atoi(row[118]);
			e.classes16            = atoi(row[119]);
			e.CastingAnim          = atoi(row[120]);
			e.TargetAnim           = atoi(row[121]);
			e.TravelType           = atoi(row[122]);
			e.SpellAffectIndex     = atoi(row[123]);
			e.disallow_sit         = atoi(row[124]);
			e.deities0             = atoi(row[125]);
			e.deities1             = atoi(row[126]);
			e.deities2             = atoi(row[127]);
			e.deities3             = atoi(row[128]);
			e.deities4             = atoi(row[129]);
			e.deities5             = atoi(row[130]);
			e.deities6             = atoi(row[131]);
			e.deities7             = atoi(row[132]);
			e.deities8             = atoi(row[133]);
			e.deities9             = atoi(row[134]);
			e.deities10            = atoi(row[135]);
			e.deities11            = atoi(row[136]);
			e.deities12            = atoi(row[137]);
			e.deities13            = atoi(row[138]);
			e.deities14            = atoi(row[139]);
			e.deities15            = atoi(row[140]);
			e.deities16            = atoi(row[141]);
			e.field142             = atoi(row[142]);
			e.field143             = atoi(row[143]);
			e.new_icon             = atoi(row[144]);
			e.spellanim            = atoi(row[145]);
			e.uninterruptable      = atoi(row[146]);
			e.ResistDiff           = atoi(row[147]);
			e.dot_stacking_exempt  = atoi(row[148]);
			e.deleteable           = atoi(row[149]);
			e.RecourseLink         = atoi(row[150]);
			e.no_partial_resist    = atoi(row[151]);
			e.field152             = atoi(row[152]);
			e.field153             = atoi(row[153]);
			e.short_buff_box       = atoi(row[154]);
			e.descnum              = atoi(row[155]);
			e.typedescnum          = atoi(row[156]);
			e.effectdescnum        = atoi(row[157]);
			e.effectdescnum2       = atoi(row[158]);
			e.npc_no_los           = atoi(row[159]);
			e.field160             = atoi(row[160]);
			e.reflectable          = atoi(row[161]);
			e.bonushate            = atoi(row[162]);
			e.field163             = atoi(row[163]);
			e.field164             = atoi(row[164]);
			e.ldon_trap            = atoi(row[165]);
			e.EndurCost            = atoi(row[166]);
			e.EndurTimerIndex      = atoi(row[167]);
			e.IsDiscipline         = atoi(row[168]);
			e.field169             = atoi(row[169]);
			e.field170             = atoi(row[170]);
			e.field171             = atoi(row[171]);
			e.field172             = atoi(row[172]);
			e.HateAdded            = atoi(row[173]);
			e.EndurUpkeep          = atoi(row[174]);
			e.numhitstype          = atoi(row[175]);
			e.numhits              = atoi(row[176]);
			e.pvpresistbase        = atoi(row[177]);
			e.pvpresistcalc        = atoi(row[178]);
			e.pvpresistcap         = atoi(row[179]);
			e.spell_category       = atoi(row[180]);
			e.pvp_duration         = atoi(row[181]);
			e.pvp_duration_cap     = atoi(row[182]);
			e.pcnpc_only_flag      = atoi(row[183]);
			e.cast_not_standing    = atoi(row[184]);
			e.can_mgb              = atoi(row[185]);
			e.nodispell            = atoi(row[186]);
			e.npc_category         = atoi(row[187]);
			e.npc_usefulness       = atoi(row[188]);
			e.MinResist            = atoi(row[189]);
			e.MaxResist            = atoi(row[190]);
			e.viral_targets        = atoi(row[191]);
			e.viral_timer          = atoi(row[192]);
			e.nimbuseffect         = atoi(row[193]);
			e.ConeStartAngle       = atoi(row[194]);
			e.ConeStopAngle        = atoi(row[195]);
			e.sneaking             = atoi(row[196]);
			e.not_extendable       = atoi(row[197]);
			e.field198             = atoi(row[198]);
			e.field199             = atoi(row[199]);
			e.suspendable          = atoi(row[200]);
			e.viral_range          = atoi(row[201]);
			e.songcap              = atoi(row[202]);
			e.field203             = atoi(row[203]);
			e.field204             = atoi(row[204]);
			e.no_block             = atoi(row[205]);
			e.field206             = atoi(row[206]);
			e.spellgroup           = atoi(row[207]);
			e.rank                 = atoi(row[208]);
			e.field209             = atoi(row[209]);
			e.field210             = atoi(row[210]);
			e.CastRestriction      = atoi(row[211]);
			e.allowrest            = atoi(row[212]);
			e.InCombat             = atoi(row[213]);
			e.OutofCombat          = atoi(row[214]);
			e.field215             = atoi(row[215]);
			e.field216             = atoi(row[216]);
			e.field217             = atoi(row[217]);
			e.aemaxtargets         = atoi(row[218]);
			e.maxtargets           = atoi(row[219]);
			e.field220             = atoi(row[220]);
			e.field221             = atoi(row[221]);
			e.field222             = atoi(row[222]);
			e.field223             = atoi(row[223]);
			e.persistdeath         = atoi(row[224]);
			e.field225             = atoi(row[225]);
			e.field226             = atoi(row[226]);
			e.min_dist             = static_cast<float>(atof(row[227]));
			e.min_dist_mod         = static_cast<float>(atof(row[228]));
			e.max_dist             = static_cast<float>(atof(row[229]));
			e.max_dist_mod         = static_cast<float>(atof(row[230]));
			e.min_range            = atoi(row[231]);
			e.field232             = atoi(row[232]);
			e.field233             = atoi(row[233]);
			e.field234             = atoi(row[234]);
			e.field235             = atoi(row[235]);
			e.field236             = atoi(row[236]);

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

#endif //EQEMU_BASE_SPELLS_NEW_REPOSITORY_H
