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

#ifndef EQEMU_BASE_ITEMS_REPOSITORY_H
#define EQEMU_BASE_ITEMS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseItemsRepository {
public:
	struct Items {
		int         id;
		int         minstatus;
		std::string Name;
		int         aagi;
		int         ac;
		int         accuracy;
		int         acha;
		int         adex;
		int         aint;
		int         artifactflag;
		int         asta;
		int         astr;
		int         attack;
		int         augrestrict;
		int         augslot1type;
		int         augslot1visible;
		int         augslot2type;
		int         augslot2visible;
		int         augslot3type;
		int         augslot3visible;
		int         augslot4type;
		int         augslot4visible;
		int         augslot5type;
		int         augslot5visible;
		int         augslot6type;
		int         augslot6visible;
		int         augtype;
		int         avoidance;
		int         awis;
		int         bagsize;
		int         bagslots;
		int         bagtype;
		int         bagwr;
		int         banedmgamt;
		int         banedmgraceamt;
		int         banedmgbody;
		int         banedmgrace;
		int         bardtype;
		int         bardvalue;
		int         book;
		int         casttime;
		int         casttime_;
		std::string charmfile;
		std::string charmfileid;
		int         classes;
		int         color;
		std::string combateffects;
		int         extradmgskill;
		int         extradmgamt;
		int         price;
		int         cr;
		int         damage;
		int         damageshield;
		int         deity;
		int         delay;
		int         augdistiller;
		int         dotshielding;
		int         dr;
		int         clicktype;
		int         clicklevel2;
		int         elemdmgtype;
		int         elemdmgamt;
		int         endur;
		int         factionamt1;
		int         factionamt2;
		int         factionamt3;
		int         factionamt4;
		int         factionmod1;
		int         factionmod2;
		int         factionmod3;
		int         factionmod4;
		std::string filename;
		int         focuseffect;
		int         fr;
		int         fvnodrop;
		int         haste;
		int         clicklevel;
		int         hp;
		int         regen;
		int         icon;
		std::string idfile;
		int         itemclass;
		int         itemtype;
		int         ldonprice;
		int         ldontheme;
		int         ldonsold;
		int         light;
		std::string lore;
		int         loregroup;
		int         magic;
		int         mana;
		int         manaregen;
		int         enduranceregen;
		int         material;
		int         herosforgemodel;
		int         maxcharges;
		int         mr;
		int         nodrop;
		int         norent;
		int         pendingloreflag;
		int         pr;
		int         procrate;
		int         races;
		int         range;
		int         reclevel;
		int         recskill;
		int         reqlevel;
		float       sellrate;
		int         shielding;
		int         size;
		int         skillmodtype;
		int         skillmodvalue;
		int         slots;
		int         clickeffect;
		int         spellshield;
		int         strikethrough;
		int         stunresist;
		int         summonedflag;
		int         tradeskills;
		int         favor;
		int         weight;
		int         UNK012;
		int         UNK013;
		int         benefitflag;
		int         UNK054;
		int         UNK059;
		int         booktype;
		int         recastdelay;
		int         recasttype;
		int         guildfavor;
		int         UNK123;
		int         UNK124;
		int         attuneable;
		int         nopet;
		time_t      updated;
		std::string comment;
		int         UNK127;
		int         pointtype;
		int         potionbelt;
		int         potionbeltslots;
		int         stacksize;
		int         notransfer;
		int         stackable;
		std::string UNK134;
		int         UNK137;
		int         proceffect;
		int         proctype;
		int         proclevel2;
		int         proclevel;
		int         UNK142;
		int         worneffect;
		int         worntype;
		int         wornlevel2;
		int         wornlevel;
		int         UNK147;
		int         focustype;
		int         focuslevel2;
		int         focuslevel;
		int         UNK152;
		int         scrolleffect;
		int         scrolltype;
		int         scrolllevel2;
		int         scrolllevel;
		int         UNK157;
		time_t      serialized;
		time_t      verified;
		std::string serialization;
		std::string source;
		int         UNK033;
		std::string lorefile;
		int         UNK014;
		int         svcorruption;
		int         skillmodmax;
		int         UNK060;
		int         augslot1unk2;
		int         augslot2unk2;
		int         augslot3unk2;
		int         augslot4unk2;
		int         augslot5unk2;
		int         augslot6unk2;
		int         UNK120;
		int         UNK121;
		int         questitemflag;
		std::string UNK132;
		int         clickunk5;
		std::string clickunk6;
		int         clickunk7;
		int         procunk1;
		int         procunk2;
		int         procunk3;
		int         procunk4;
		std::string procunk6;
		int         procunk7;
		int         wornunk1;
		int         wornunk2;
		int         wornunk3;
		int         wornunk4;
		int         wornunk5;
		std::string wornunk6;
		int         wornunk7;
		int         focusunk1;
		int         focusunk2;
		int         focusunk3;
		int         focusunk4;
		int         focusunk5;
		std::string focusunk6;
		int         focusunk7;
		int         scrollunk1;
		int         scrollunk2;
		int         scrollunk3;
		int         scrollunk4;
		int         scrollunk5;
		std::string scrollunk6;
		int         scrollunk7;
		int         UNK193;
		int         purity;
		int         evoitem;
		int         evoid;
		int         evolvinglevel;
		int         evomax;
		std::string clickname;
		std::string procname;
		std::string wornname;
		std::string focusname;
		std::string scrollname;
		int         dsmitigation;
		int         heroic_str;
		int         heroic_int;
		int         heroic_wis;
		int         heroic_agi;
		int         heroic_dex;
		int         heroic_sta;
		int         heroic_cha;
		int         heroic_pr;
		int         heroic_dr;
		int         heroic_fr;
		int         heroic_cr;
		int         heroic_mr;
		int         heroic_svcorrup;
		int         healamt;
		int         spelldmg;
		int         clairvoyance;
		int         backstabdmg;
		std::string created;
		int         elitematerial;
		int         ldonsellbackrate;
		int         scriptfileid;
		int         expendablearrow;
		int         powersourcecapacity;
		int         bardeffect;
		int         bardeffecttype;
		int         bardlevel2;
		int         bardlevel;
		int         bardunk1;
		int         bardunk2;
		int         bardunk3;
		int         bardunk4;
		int         bardunk5;
		std::string bardname;
		int         bardunk7;
		int         UNK214;
		int         subtype;
		int         UNK220;
		int         UNK221;
		int         heirloom;
		int         UNK223;
		int         UNK224;
		int         UNK225;
		int         UNK226;
		int         UNK227;
		int         UNK228;
		int         UNK229;
		int         UNK230;
		int         UNK231;
		int         UNK232;
		int         UNK233;
		int         UNK234;
		int         placeable;
		int         UNK236;
		int         UNK237;
		int         UNK238;
		int         UNK239;
		int         UNK240;
		int         UNK241;
		int         epicitem;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"minstatus",
			"Name",
			"aagi",
			"ac",
			"accuracy",
			"acha",
			"adex",
			"aint",
			"artifactflag",
			"asta",
			"astr",
			"attack",
			"augrestrict",
			"augslot1type",
			"augslot1visible",
			"augslot2type",
			"augslot2visible",
			"augslot3type",
			"augslot3visible",
			"augslot4type",
			"augslot4visible",
			"augslot5type",
			"augslot5visible",
			"augslot6type",
			"augslot6visible",
			"augtype",
			"avoidance",
			"awis",
			"bagsize",
			"bagslots",
			"bagtype",
			"bagwr",
			"banedmgamt",
			"banedmgraceamt",
			"banedmgbody",
			"banedmgrace",
			"bardtype",
			"bardvalue",
			"book",
			"casttime",
			"casttime_",
			"charmfile",
			"charmfileid",
			"classes",
			"color",
			"combateffects",
			"extradmgskill",
			"extradmgamt",
			"price",
			"cr",
			"damage",
			"damageshield",
			"deity",
			"delay",
			"augdistiller",
			"dotshielding",
			"dr",
			"clicktype",
			"clicklevel2",
			"elemdmgtype",
			"elemdmgamt",
			"endur",
			"factionamt1",
			"factionamt2",
			"factionamt3",
			"factionamt4",
			"factionmod1",
			"factionmod2",
			"factionmod3",
			"factionmod4",
			"filename",
			"focuseffect",
			"fr",
			"fvnodrop",
			"haste",
			"clicklevel",
			"hp",
			"regen",
			"icon",
			"idfile",
			"itemclass",
			"itemtype",
			"ldonprice",
			"ldontheme",
			"ldonsold",
			"light",
			"lore",
			"loregroup",
			"magic",
			"mana",
			"manaregen",
			"enduranceregen",
			"material",
			"herosforgemodel",
			"maxcharges",
			"mr",
			"nodrop",
			"norent",
			"pendingloreflag",
			"pr",
			"procrate",
			"races",
			"range",
			"reclevel",
			"recskill",
			"reqlevel",
			"sellrate",
			"shielding",
			"size",
			"skillmodtype",
			"skillmodvalue",
			"slots",
			"clickeffect",
			"spellshield",
			"strikethrough",
			"stunresist",
			"summonedflag",
			"tradeskills",
			"favor",
			"weight",
			"UNK012",
			"UNK013",
			"benefitflag",
			"UNK054",
			"UNK059",
			"booktype",
			"recastdelay",
			"recasttype",
			"guildfavor",
			"UNK123",
			"UNK124",
			"attuneable",
			"nopet",
			"updated",
			"comment",
			"UNK127",
			"pointtype",
			"potionbelt",
			"potionbeltslots",
			"stacksize",
			"notransfer",
			"stackable",
			"UNK134",
			"UNK137",
			"proceffect",
			"proctype",
			"proclevel2",
			"proclevel",
			"UNK142",
			"worneffect",
			"worntype",
			"wornlevel2",
			"wornlevel",
			"UNK147",
			"focustype",
			"focuslevel2",
			"focuslevel",
			"UNK152",
			"scrolleffect",
			"scrolltype",
			"scrolllevel2",
			"scrolllevel",
			"UNK157",
			"serialized",
			"verified",
			"serialization",
			"source",
			"UNK033",
			"lorefile",
			"UNK014",
			"svcorruption",
			"skillmodmax",
			"UNK060",
			"augslot1unk2",
			"augslot2unk2",
			"augslot3unk2",
			"augslot4unk2",
			"augslot5unk2",
			"augslot6unk2",
			"UNK120",
			"UNK121",
			"questitemflag",
			"UNK132",
			"clickunk5",
			"clickunk6",
			"clickunk7",
			"procunk1",
			"procunk2",
			"procunk3",
			"procunk4",
			"procunk6",
			"procunk7",
			"wornunk1",
			"wornunk2",
			"wornunk3",
			"wornunk4",
			"wornunk5",
			"wornunk6",
			"wornunk7",
			"focusunk1",
			"focusunk2",
			"focusunk3",
			"focusunk4",
			"focusunk5",
			"focusunk6",
			"focusunk7",
			"scrollunk1",
			"scrollunk2",
			"scrollunk3",
			"scrollunk4",
			"scrollunk5",
			"scrollunk6",
			"scrollunk7",
			"UNK193",
			"purity",
			"evoitem",
			"evoid",
			"evolvinglevel",
			"evomax",
			"clickname",
			"procname",
			"wornname",
			"focusname",
			"scrollname",
			"dsmitigation",
			"heroic_str",
			"heroic_int",
			"heroic_wis",
			"heroic_agi",
			"heroic_dex",
			"heroic_sta",
			"heroic_cha",
			"heroic_pr",
			"heroic_dr",
			"heroic_fr",
			"heroic_cr",
			"heroic_mr",
			"heroic_svcorrup",
			"healamt",
			"spelldmg",
			"clairvoyance",
			"backstabdmg",
			"created",
			"elitematerial",
			"ldonsellbackrate",
			"scriptfileid",
			"expendablearrow",
			"powersourcecapacity",
			"bardeffect",
			"bardeffecttype",
			"bardlevel2",
			"bardlevel",
			"bardunk1",
			"bardunk2",
			"bardunk3",
			"bardunk4",
			"bardunk5",
			"bardname",
			"bardunk7",
			"UNK214",
			"subtype",
			"UNK220",
			"UNK221",
			"heirloom",
			"UNK223",
			"UNK224",
			"UNK225",
			"UNK226",
			"UNK227",
			"UNK228",
			"UNK229",
			"UNK230",
			"UNK231",
			"UNK232",
			"UNK233",
			"UNK234",
			"placeable",
			"UNK236",
			"UNK237",
			"UNK238",
			"UNK239",
			"UNK240",
			"UNK241",
			"epicitem",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"minstatus",
			"Name",
			"aagi",
			"ac",
			"accuracy",
			"acha",
			"adex",
			"aint",
			"artifactflag",
			"asta",
			"astr",
			"attack",
			"augrestrict",
			"augslot1type",
			"augslot1visible",
			"augslot2type",
			"augslot2visible",
			"augslot3type",
			"augslot3visible",
			"augslot4type",
			"augslot4visible",
			"augslot5type",
			"augslot5visible",
			"augslot6type",
			"augslot6visible",
			"augtype",
			"avoidance",
			"awis",
			"bagsize",
			"bagslots",
			"bagtype",
			"bagwr",
			"banedmgamt",
			"banedmgraceamt",
			"banedmgbody",
			"banedmgrace",
			"bardtype",
			"bardvalue",
			"book",
			"casttime",
			"casttime_",
			"charmfile",
			"charmfileid",
			"classes",
			"color",
			"combateffects",
			"extradmgskill",
			"extradmgamt",
			"price",
			"cr",
			"damage",
			"damageshield",
			"deity",
			"delay",
			"augdistiller",
			"dotshielding",
			"dr",
			"clicktype",
			"clicklevel2",
			"elemdmgtype",
			"elemdmgamt",
			"endur",
			"factionamt1",
			"factionamt2",
			"factionamt3",
			"factionamt4",
			"factionmod1",
			"factionmod2",
			"factionmod3",
			"factionmod4",
			"filename",
			"focuseffect",
			"fr",
			"fvnodrop",
			"haste",
			"clicklevel",
			"hp",
			"regen",
			"icon",
			"idfile",
			"itemclass",
			"itemtype",
			"ldonprice",
			"ldontheme",
			"ldonsold",
			"light",
			"lore",
			"loregroup",
			"magic",
			"mana",
			"manaregen",
			"enduranceregen",
			"material",
			"herosforgemodel",
			"maxcharges",
			"mr",
			"nodrop",
			"norent",
			"pendingloreflag",
			"pr",
			"procrate",
			"races",
			"range",
			"reclevel",
			"recskill",
			"reqlevel",
			"sellrate",
			"shielding",
			"size",
			"skillmodtype",
			"skillmodvalue",
			"slots",
			"clickeffect",
			"spellshield",
			"strikethrough",
			"stunresist",
			"summonedflag",
			"tradeskills",
			"favor",
			"weight",
			"UNK012",
			"UNK013",
			"benefitflag",
			"UNK054",
			"UNK059",
			"booktype",
			"recastdelay",
			"recasttype",
			"guildfavor",
			"UNK123",
			"UNK124",
			"attuneable",
			"nopet",
			"UNIX_TIMESTAMP(updated)",
			"comment",
			"UNK127",
			"pointtype",
			"potionbelt",
			"potionbeltslots",
			"stacksize",
			"notransfer",
			"stackable",
			"UNK134",
			"UNK137",
			"proceffect",
			"proctype",
			"proclevel2",
			"proclevel",
			"UNK142",
			"worneffect",
			"worntype",
			"wornlevel2",
			"wornlevel",
			"UNK147",
			"focustype",
			"focuslevel2",
			"focuslevel",
			"UNK152",
			"scrolleffect",
			"scrolltype",
			"scrolllevel2",
			"scrolllevel",
			"UNK157",
			"UNIX_TIMESTAMP(serialized)",
			"UNIX_TIMESTAMP(verified)",
			"serialization",
			"source",
			"UNK033",
			"lorefile",
			"UNK014",
			"svcorruption",
			"skillmodmax",
			"UNK060",
			"augslot1unk2",
			"augslot2unk2",
			"augslot3unk2",
			"augslot4unk2",
			"augslot5unk2",
			"augslot6unk2",
			"UNK120",
			"UNK121",
			"questitemflag",
			"UNK132",
			"clickunk5",
			"clickunk6",
			"clickunk7",
			"procunk1",
			"procunk2",
			"procunk3",
			"procunk4",
			"procunk6",
			"procunk7",
			"wornunk1",
			"wornunk2",
			"wornunk3",
			"wornunk4",
			"wornunk5",
			"wornunk6",
			"wornunk7",
			"focusunk1",
			"focusunk2",
			"focusunk3",
			"focusunk4",
			"focusunk5",
			"focusunk6",
			"focusunk7",
			"scrollunk1",
			"scrollunk2",
			"scrollunk3",
			"scrollunk4",
			"scrollunk5",
			"scrollunk6",
			"scrollunk7",
			"UNK193",
			"purity",
			"evoitem",
			"evoid",
			"evolvinglevel",
			"evomax",
			"clickname",
			"procname",
			"wornname",
			"focusname",
			"scrollname",
			"dsmitigation",
			"heroic_str",
			"heroic_int",
			"heroic_wis",
			"heroic_agi",
			"heroic_dex",
			"heroic_sta",
			"heroic_cha",
			"heroic_pr",
			"heroic_dr",
			"heroic_fr",
			"heroic_cr",
			"heroic_mr",
			"heroic_svcorrup",
			"healamt",
			"spelldmg",
			"clairvoyance",
			"backstabdmg",
			"created",
			"elitematerial",
			"ldonsellbackrate",
			"scriptfileid",
			"expendablearrow",
			"powersourcecapacity",
			"bardeffect",
			"bardeffecttype",
			"bardlevel2",
			"bardlevel",
			"bardunk1",
			"bardunk2",
			"bardunk3",
			"bardunk4",
			"bardunk5",
			"bardname",
			"bardunk7",
			"UNK214",
			"subtype",
			"UNK220",
			"UNK221",
			"heirloom",
			"UNK223",
			"UNK224",
			"UNK225",
			"UNK226",
			"UNK227",
			"UNK228",
			"UNK229",
			"UNK230",
			"UNK231",
			"UNK232",
			"UNK233",
			"UNK234",
			"placeable",
			"UNK236",
			"UNK237",
			"UNK238",
			"UNK239",
			"UNK240",
			"UNK241",
			"epicitem",
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
		return std::string("items");
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

	static Items NewEntity()
	{
		Items e{};

		e.id                  = 0;
		e.minstatus           = 0;
		e.Name                = "";
		e.aagi                = 0;
		e.ac                  = 0;
		e.accuracy            = 0;
		e.acha                = 0;
		e.adex                = 0;
		e.aint                = 0;
		e.artifactflag        = 0;
		e.asta                = 0;
		e.astr                = 0;
		e.attack              = 0;
		e.augrestrict         = 0;
		e.augslot1type        = 0;
		e.augslot1visible     = 0;
		e.augslot2type        = 0;
		e.augslot2visible     = 0;
		e.augslot3type        = 0;
		e.augslot3visible     = 0;
		e.augslot4type        = 0;
		e.augslot4visible     = 0;
		e.augslot5type        = 0;
		e.augslot5visible     = 0;
		e.augslot6type        = 0;
		e.augslot6visible     = 0;
		e.augtype             = 0;
		e.avoidance           = 0;
		e.awis                = 0;
		e.bagsize             = 0;
		e.bagslots            = 0;
		e.bagtype             = 0;
		e.bagwr               = 0;
		e.banedmgamt          = 0;
		e.banedmgraceamt      = 0;
		e.banedmgbody         = 0;
		e.banedmgrace         = 0;
		e.bardtype            = 0;
		e.bardvalue           = 0;
		e.book                = 0;
		e.casttime            = 0;
		e.casttime_           = 0;
		e.charmfile           = "";
		e.charmfileid         = "";
		e.classes             = 0;
		e.color               = 0;
		e.combateffects       = "";
		e.extradmgskill       = 0;
		e.extradmgamt         = 0;
		e.price               = 0;
		e.cr                  = 0;
		e.damage              = 0;
		e.damageshield        = 0;
		e.deity               = 0;
		e.delay               = 0;
		e.augdistiller        = 0;
		e.dotshielding        = 0;
		e.dr                  = 0;
		e.clicktype           = 0;
		e.clicklevel2         = 0;
		e.elemdmgtype         = 0;
		e.elemdmgamt          = 0;
		e.endur               = 0;
		e.factionamt1         = 0;
		e.factionamt2         = 0;
		e.factionamt3         = 0;
		e.factionamt4         = 0;
		e.factionmod1         = 0;
		e.factionmod2         = 0;
		e.factionmod3         = 0;
		e.factionmod4         = 0;
		e.filename            = "";
		e.focuseffect         = 0;
		e.fr                  = 0;
		e.fvnodrop            = 0;
		e.haste               = 0;
		e.clicklevel          = 0;
		e.hp                  = 0;
		e.regen               = 0;
		e.icon                = 0;
		e.idfile              = "";
		e.itemclass           = 0;
		e.itemtype            = 0;
		e.ldonprice           = 0;
		e.ldontheme           = 0;
		e.ldonsold            = 0;
		e.light               = 0;
		e.lore                = "";
		e.loregroup           = 0;
		e.magic               = 0;
		e.mana                = 0;
		e.manaregen           = 0;
		e.enduranceregen      = 0;
		e.material            = 0;
		e.herosforgemodel     = 0;
		e.maxcharges          = 0;
		e.mr                  = 0;
		e.nodrop              = 0;
		e.norent              = 0;
		e.pendingloreflag     = 0;
		e.pr                  = 0;
		e.procrate            = 0;
		e.races               = 0;
		e.range               = 0;
		e.reclevel            = 0;
		e.recskill            = 0;
		e.reqlevel            = 0;
		e.sellrate            = 0;
		e.shielding           = 0;
		e.size                = 0;
		e.skillmodtype        = 0;
		e.skillmodvalue       = 0;
		e.slots               = 0;
		e.clickeffect         = 0;
		e.spellshield         = 0;
		e.strikethrough       = 0;
		e.stunresist          = 0;
		e.summonedflag        = 0;
		e.tradeskills         = 0;
		e.favor               = 0;
		e.weight              = 0;
		e.UNK012              = 0;
		e.UNK013              = 0;
		e.benefitflag         = 0;
		e.UNK054              = 0;
		e.UNK059              = 0;
		e.booktype            = 0;
		e.recastdelay         = 0;
		e.recasttype          = 0;
		e.guildfavor          = 0;
		e.UNK123              = 0;
		e.UNK124              = 0;
		e.attuneable          = 0;
		e.nopet               = 0;
		e.updated             = 0;
		e.comment             = "";
		e.UNK127              = 0;
		e.pointtype           = 0;
		e.potionbelt          = 0;
		e.potionbeltslots     = 0;
		e.stacksize           = 0;
		e.notransfer          = 0;
		e.stackable           = 0;
		e.UNK134              = "";
		e.UNK137              = 0;
		e.proceffect          = 0;
		e.proctype            = 0;
		e.proclevel2          = 0;
		e.proclevel           = 0;
		e.UNK142              = 0;
		e.worneffect          = 0;
		e.worntype            = 0;
		e.wornlevel2          = 0;
		e.wornlevel           = 0;
		e.UNK147              = 0;
		e.focustype           = 0;
		e.focuslevel2         = 0;
		e.focuslevel          = 0;
		e.UNK152              = 0;
		e.scrolleffect        = 0;
		e.scrolltype          = 0;
		e.scrolllevel2        = 0;
		e.scrolllevel         = 0;
		e.UNK157              = 0;
		e.serialized          = 0;
		e.verified            = 0;
		e.serialization       = "";
		e.source              = "";
		e.UNK033              = 0;
		e.lorefile            = "";
		e.UNK014              = 0;
		e.svcorruption        = 0;
		e.skillmodmax         = 0;
		e.UNK060              = 0;
		e.augslot1unk2        = 0;
		e.augslot2unk2        = 0;
		e.augslot3unk2        = 0;
		e.augslot4unk2        = 0;
		e.augslot5unk2        = 0;
		e.augslot6unk2        = 0;
		e.UNK120              = 0;
		e.UNK121              = 0;
		e.questitemflag       = 0;
		e.UNK132              = "";
		e.clickunk5           = 0;
		e.clickunk6           = "";
		e.clickunk7           = 0;
		e.procunk1            = 0;
		e.procunk2            = 0;
		e.procunk3            = 0;
		e.procunk4            = 0;
		e.procunk6            = "";
		e.procunk7            = 0;
		e.wornunk1            = 0;
		e.wornunk2            = 0;
		e.wornunk3            = 0;
		e.wornunk4            = 0;
		e.wornunk5            = 0;
		e.wornunk6            = "";
		e.wornunk7            = 0;
		e.focusunk1           = 0;
		e.focusunk2           = 0;
		e.focusunk3           = 0;
		e.focusunk4           = 0;
		e.focusunk5           = 0;
		e.focusunk6           = "";
		e.focusunk7           = 0;
		e.scrollunk1          = 0;
		e.scrollunk2          = 0;
		e.scrollunk3          = 0;
		e.scrollunk4          = 0;
		e.scrollunk5          = 0;
		e.scrollunk6          = "";
		e.scrollunk7          = 0;
		e.UNK193              = 0;
		e.purity              = 0;
		e.evoitem             = 0;
		e.evoid               = 0;
		e.evolvinglevel       = 0;
		e.evomax              = 0;
		e.clickname           = "";
		e.procname            = "";
		e.wornname            = "";
		e.focusname           = "";
		e.scrollname          = "";
		e.dsmitigation        = 0;
		e.heroic_str          = 0;
		e.heroic_int          = 0;
		e.heroic_wis          = 0;
		e.heroic_agi          = 0;
		e.heroic_dex          = 0;
		e.heroic_sta          = 0;
		e.heroic_cha          = 0;
		e.heroic_pr           = 0;
		e.heroic_dr           = 0;
		e.heroic_fr           = 0;
		e.heroic_cr           = 0;
		e.heroic_mr           = 0;
		e.heroic_svcorrup     = 0;
		e.healamt             = 0;
		e.spelldmg            = 0;
		e.clairvoyance        = 0;
		e.backstabdmg         = 0;
		e.created             = "";
		e.elitematerial       = 0;
		e.ldonsellbackrate    = 0;
		e.scriptfileid        = 0;
		e.expendablearrow     = 0;
		e.powersourcecapacity = 0;
		e.bardeffect          = 0;
		e.bardeffecttype      = 0;
		e.bardlevel2          = 0;
		e.bardlevel           = 0;
		e.bardunk1            = 0;
		e.bardunk2            = 0;
		e.bardunk3            = 0;
		e.bardunk4            = 0;
		e.bardunk5            = 0;
		e.bardname            = "";
		e.bardunk7            = 0;
		e.UNK214              = 0;
		e.subtype             = 0;
		e.UNK220              = 0;
		e.UNK221              = 0;
		e.heirloom            = 0;
		e.UNK223              = 0;
		e.UNK224              = 0;
		e.UNK225              = 0;
		e.UNK226              = 0;
		e.UNK227              = 0;
		e.UNK228              = 0;
		e.UNK229              = 0;
		e.UNK230              = 0;
		e.UNK231              = 0;
		e.UNK232              = 0;
		e.UNK233              = 0;
		e.UNK234              = 0;
		e.placeable           = 0;
		e.UNK236              = 0;
		e.UNK237              = 0;
		e.UNK238              = 0;
		e.UNK239              = 0;
		e.UNK240              = 0;
		e.UNK241              = 0;
		e.epicitem            = 0;

		return e;
	}

	static Items GetItems(
		const std::vector<Items> &itemss,
		int items_id
	)
	{
		for (auto &items : itemss) {
			if (items.id == items_id) {
				return items;
			}
		}

		return NewEntity();
	}

	static Items FindOne(
		Database& db,
		int items_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				items_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Items e{};

			e.id                  = atoi(row[0]);
			e.minstatus           = atoi(row[1]);
			e.Name                = row[2] ? row[2] : "";
			e.aagi                = atoi(row[3]);
			e.ac                  = atoi(row[4]);
			e.accuracy            = atoi(row[5]);
			e.acha                = atoi(row[6]);
			e.adex                = atoi(row[7]);
			e.aint                = atoi(row[8]);
			e.artifactflag        = atoi(row[9]);
			e.asta                = atoi(row[10]);
			e.astr                = atoi(row[11]);
			e.attack              = atoi(row[12]);
			e.augrestrict         = atoi(row[13]);
			e.augslot1type        = atoi(row[14]);
			e.augslot1visible     = atoi(row[15]);
			e.augslot2type        = atoi(row[16]);
			e.augslot2visible     = atoi(row[17]);
			e.augslot3type        = atoi(row[18]);
			e.augslot3visible     = atoi(row[19]);
			e.augslot4type        = atoi(row[20]);
			e.augslot4visible     = atoi(row[21]);
			e.augslot5type        = atoi(row[22]);
			e.augslot5visible     = atoi(row[23]);
			e.augslot6type        = atoi(row[24]);
			e.augslot6visible     = atoi(row[25]);
			e.augtype             = atoi(row[26]);
			e.avoidance           = atoi(row[27]);
			e.awis                = atoi(row[28]);
			e.bagsize             = atoi(row[29]);
			e.bagslots            = atoi(row[30]);
			e.bagtype             = atoi(row[31]);
			e.bagwr               = atoi(row[32]);
			e.banedmgamt          = atoi(row[33]);
			e.banedmgraceamt      = atoi(row[34]);
			e.banedmgbody         = atoi(row[35]);
			e.banedmgrace         = atoi(row[36]);
			e.bardtype            = atoi(row[37]);
			e.bardvalue           = atoi(row[38]);
			e.book                = atoi(row[39]);
			e.casttime            = atoi(row[40]);
			e.casttime_           = atoi(row[41]);
			e.charmfile           = row[42] ? row[42] : "";
			e.charmfileid         = row[43] ? row[43] : "";
			e.classes             = atoi(row[44]);
			e.color               = atoi(row[45]);
			e.combateffects       = row[46] ? row[46] : "";
			e.extradmgskill       = atoi(row[47]);
			e.extradmgamt         = atoi(row[48]);
			e.price               = atoi(row[49]);
			e.cr                  = atoi(row[50]);
			e.damage              = atoi(row[51]);
			e.damageshield        = atoi(row[52]);
			e.deity               = atoi(row[53]);
			e.delay               = atoi(row[54]);
			e.augdistiller        = atoi(row[55]);
			e.dotshielding        = atoi(row[56]);
			e.dr                  = atoi(row[57]);
			e.clicktype           = atoi(row[58]);
			e.clicklevel2         = atoi(row[59]);
			e.elemdmgtype         = atoi(row[60]);
			e.elemdmgamt          = atoi(row[61]);
			e.endur               = atoi(row[62]);
			e.factionamt1         = atoi(row[63]);
			e.factionamt2         = atoi(row[64]);
			e.factionamt3         = atoi(row[65]);
			e.factionamt4         = atoi(row[66]);
			e.factionmod1         = atoi(row[67]);
			e.factionmod2         = atoi(row[68]);
			e.factionmod3         = atoi(row[69]);
			e.factionmod4         = atoi(row[70]);
			e.filename            = row[71] ? row[71] : "";
			e.focuseffect         = atoi(row[72]);
			e.fr                  = atoi(row[73]);
			e.fvnodrop            = atoi(row[74]);
			e.haste               = atoi(row[75]);
			e.clicklevel          = atoi(row[76]);
			e.hp                  = atoi(row[77]);
			e.regen               = atoi(row[78]);
			e.icon                = atoi(row[79]);
			e.idfile              = row[80] ? row[80] : "";
			e.itemclass           = atoi(row[81]);
			e.itemtype            = atoi(row[82]);
			e.ldonprice           = atoi(row[83]);
			e.ldontheme           = atoi(row[84]);
			e.ldonsold            = atoi(row[85]);
			e.light               = atoi(row[86]);
			e.lore                = row[87] ? row[87] : "";
			e.loregroup           = atoi(row[88]);
			e.magic               = atoi(row[89]);
			e.mana                = atoi(row[90]);
			e.manaregen           = atoi(row[91]);
			e.enduranceregen      = atoi(row[92]);
			e.material            = atoi(row[93]);
			e.herosforgemodel     = atoi(row[94]);
			e.maxcharges          = atoi(row[95]);
			e.mr                  = atoi(row[96]);
			e.nodrop              = atoi(row[97]);
			e.norent              = atoi(row[98]);
			e.pendingloreflag     = atoi(row[99]);
			e.pr                  = atoi(row[100]);
			e.procrate            = atoi(row[101]);
			e.races               = atoi(row[102]);
			e.range               = atoi(row[103]);
			e.reclevel            = atoi(row[104]);
			e.recskill            = atoi(row[105]);
			e.reqlevel            = atoi(row[106]);
			e.sellrate            = static_cast<float>(atof(row[107]));
			e.shielding           = atoi(row[108]);
			e.size                = atoi(row[109]);
			e.skillmodtype        = atoi(row[110]);
			e.skillmodvalue       = atoi(row[111]);
			e.slots               = atoi(row[112]);
			e.clickeffect         = atoi(row[113]);
			e.spellshield         = atoi(row[114]);
			e.strikethrough       = atoi(row[115]);
			e.stunresist          = atoi(row[116]);
			e.summonedflag        = atoi(row[117]);
			e.tradeskills         = atoi(row[118]);
			e.favor               = atoi(row[119]);
			e.weight              = atoi(row[120]);
			e.UNK012              = atoi(row[121]);
			e.UNK013              = atoi(row[122]);
			e.benefitflag         = atoi(row[123]);
			e.UNK054              = atoi(row[124]);
			e.UNK059              = atoi(row[125]);
			e.booktype            = atoi(row[126]);
			e.recastdelay         = atoi(row[127]);
			e.recasttype          = atoi(row[128]);
			e.guildfavor          = atoi(row[129]);
			e.UNK123              = atoi(row[130]);
			e.UNK124              = atoi(row[131]);
			e.attuneable          = atoi(row[132]);
			e.nopet               = atoi(row[133]);
			e.updated             = strtoll(row[134] ? row[134] : "-1", nullptr, 10);
			e.comment             = row[135] ? row[135] : "";
			e.UNK127              = atoi(row[136]);
			e.pointtype           = atoi(row[137]);
			e.potionbelt          = atoi(row[138]);
			e.potionbeltslots     = atoi(row[139]);
			e.stacksize           = atoi(row[140]);
			e.notransfer          = atoi(row[141]);
			e.stackable           = atoi(row[142]);
			e.UNK134              = row[143] ? row[143] : "";
			e.UNK137              = atoi(row[144]);
			e.proceffect          = atoi(row[145]);
			e.proctype            = atoi(row[146]);
			e.proclevel2          = atoi(row[147]);
			e.proclevel           = atoi(row[148]);
			e.UNK142              = atoi(row[149]);
			e.worneffect          = atoi(row[150]);
			e.worntype            = atoi(row[151]);
			e.wornlevel2          = atoi(row[152]);
			e.wornlevel           = atoi(row[153]);
			e.UNK147              = atoi(row[154]);
			e.focustype           = atoi(row[155]);
			e.focuslevel2         = atoi(row[156]);
			e.focuslevel          = atoi(row[157]);
			e.UNK152              = atoi(row[158]);
			e.scrolleffect        = atoi(row[159]);
			e.scrolltype          = atoi(row[160]);
			e.scrolllevel2        = atoi(row[161]);
			e.scrolllevel         = atoi(row[162]);
			e.UNK157              = atoi(row[163]);
			e.serialized          = strtoll(row[164] ? row[164] : "-1", nullptr, 10);
			e.verified            = strtoll(row[165] ? row[165] : "-1", nullptr, 10);
			e.serialization       = row[166] ? row[166] : "";
			e.source              = row[167] ? row[167] : "";
			e.UNK033              = atoi(row[168]);
			e.lorefile            = row[169] ? row[169] : "";
			e.UNK014              = atoi(row[170]);
			e.svcorruption        = atoi(row[171]);
			e.skillmodmax         = atoi(row[172]);
			e.UNK060              = atoi(row[173]);
			e.augslot1unk2        = atoi(row[174]);
			e.augslot2unk2        = atoi(row[175]);
			e.augslot3unk2        = atoi(row[176]);
			e.augslot4unk2        = atoi(row[177]);
			e.augslot5unk2        = atoi(row[178]);
			e.augslot6unk2        = atoi(row[179]);
			e.UNK120              = atoi(row[180]);
			e.UNK121              = atoi(row[181]);
			e.questitemflag       = atoi(row[182]);
			e.UNK132              = row[183] ? row[183] : "";
			e.clickunk5           = atoi(row[184]);
			e.clickunk6           = row[185] ? row[185] : "";
			e.clickunk7           = atoi(row[186]);
			e.procunk1            = atoi(row[187]);
			e.procunk2            = atoi(row[188]);
			e.procunk3            = atoi(row[189]);
			e.procunk4            = atoi(row[190]);
			e.procunk6            = row[191] ? row[191] : "";
			e.procunk7            = atoi(row[192]);
			e.wornunk1            = atoi(row[193]);
			e.wornunk2            = atoi(row[194]);
			e.wornunk3            = atoi(row[195]);
			e.wornunk4            = atoi(row[196]);
			e.wornunk5            = atoi(row[197]);
			e.wornunk6            = row[198] ? row[198] : "";
			e.wornunk7            = atoi(row[199]);
			e.focusunk1           = atoi(row[200]);
			e.focusunk2           = atoi(row[201]);
			e.focusunk3           = atoi(row[202]);
			e.focusunk4           = atoi(row[203]);
			e.focusunk5           = atoi(row[204]);
			e.focusunk6           = row[205] ? row[205] : "";
			e.focusunk7           = atoi(row[206]);
			e.scrollunk1          = atoi(row[207]);
			e.scrollunk2          = atoi(row[208]);
			e.scrollunk3          = atoi(row[209]);
			e.scrollunk4          = atoi(row[210]);
			e.scrollunk5          = atoi(row[211]);
			e.scrollunk6          = row[212] ? row[212] : "";
			e.scrollunk7          = atoi(row[213]);
			e.UNK193              = atoi(row[214]);
			e.purity              = atoi(row[215]);
			e.evoitem             = atoi(row[216]);
			e.evoid               = atoi(row[217]);
			e.evolvinglevel       = atoi(row[218]);
			e.evomax              = atoi(row[219]);
			e.clickname           = row[220] ? row[220] : "";
			e.procname            = row[221] ? row[221] : "";
			e.wornname            = row[222] ? row[222] : "";
			e.focusname           = row[223] ? row[223] : "";
			e.scrollname          = row[224] ? row[224] : "";
			e.dsmitigation        = atoi(row[225]);
			e.heroic_str          = atoi(row[226]);
			e.heroic_int          = atoi(row[227]);
			e.heroic_wis          = atoi(row[228]);
			e.heroic_agi          = atoi(row[229]);
			e.heroic_dex          = atoi(row[230]);
			e.heroic_sta          = atoi(row[231]);
			e.heroic_cha          = atoi(row[232]);
			e.heroic_pr           = atoi(row[233]);
			e.heroic_dr           = atoi(row[234]);
			e.heroic_fr           = atoi(row[235]);
			e.heroic_cr           = atoi(row[236]);
			e.heroic_mr           = atoi(row[237]);
			e.heroic_svcorrup     = atoi(row[238]);
			e.healamt             = atoi(row[239]);
			e.spelldmg            = atoi(row[240]);
			e.clairvoyance        = atoi(row[241]);
			e.backstabdmg         = atoi(row[242]);
			e.created             = row[243] ? row[243] : "";
			e.elitematerial       = atoi(row[244]);
			e.ldonsellbackrate    = atoi(row[245]);
			e.scriptfileid        = atoi(row[246]);
			e.expendablearrow     = atoi(row[247]);
			e.powersourcecapacity = atoi(row[248]);
			e.bardeffect          = atoi(row[249]);
			e.bardeffecttype      = atoi(row[250]);
			e.bardlevel2          = atoi(row[251]);
			e.bardlevel           = atoi(row[252]);
			e.bardunk1            = atoi(row[253]);
			e.bardunk2            = atoi(row[254]);
			e.bardunk3            = atoi(row[255]);
			e.bardunk4            = atoi(row[256]);
			e.bardunk5            = atoi(row[257]);
			e.bardname            = row[258] ? row[258] : "";
			e.bardunk7            = atoi(row[259]);
			e.UNK214              = atoi(row[260]);
			e.subtype             = atoi(row[261]);
			e.UNK220              = atoi(row[262]);
			e.UNK221              = atoi(row[263]);
			e.heirloom            = atoi(row[264]);
			e.UNK223              = atoi(row[265]);
			e.UNK224              = atoi(row[266]);
			e.UNK225              = atoi(row[267]);
			e.UNK226              = atoi(row[268]);
			e.UNK227              = atoi(row[269]);
			e.UNK228              = atoi(row[270]);
			e.UNK229              = atoi(row[271]);
			e.UNK230              = atoi(row[272]);
			e.UNK231              = atoi(row[273]);
			e.UNK232              = atoi(row[274]);
			e.UNK233              = atoi(row[275]);
			e.UNK234              = atoi(row[276]);
			e.placeable           = atoi(row[277]);
			e.UNK236              = atoi(row[278]);
			e.UNK237              = atoi(row[279]);
			e.UNK238              = atoi(row[280]);
			e.UNK239              = atoi(row[281]);
			e.UNK240              = atoi(row[282]);
			e.UNK241              = atoi(row[283]);
			e.epicitem            = atoi(row[284]);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int items_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				items_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Items &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.id));
		v.push_back(columns[1] + " = " + std::to_string(e.minstatus));
		v.push_back(columns[2] + " = '" + Strings::Escape(e.Name) + "'");
		v.push_back(columns[3] + " = " + std::to_string(e.aagi));
		v.push_back(columns[4] + " = " + std::to_string(e.ac));
		v.push_back(columns[5] + " = " + std::to_string(e.accuracy));
		v.push_back(columns[6] + " = " + std::to_string(e.acha));
		v.push_back(columns[7] + " = " + std::to_string(e.adex));
		v.push_back(columns[8] + " = " + std::to_string(e.aint));
		v.push_back(columns[9] + " = " + std::to_string(e.artifactflag));
		v.push_back(columns[10] + " = " + std::to_string(e.asta));
		v.push_back(columns[11] + " = " + std::to_string(e.astr));
		v.push_back(columns[12] + " = " + std::to_string(e.attack));
		v.push_back(columns[13] + " = " + std::to_string(e.augrestrict));
		v.push_back(columns[14] + " = " + std::to_string(e.augslot1type));
		v.push_back(columns[15] + " = " + std::to_string(e.augslot1visible));
		v.push_back(columns[16] + " = " + std::to_string(e.augslot2type));
		v.push_back(columns[17] + " = " + std::to_string(e.augslot2visible));
		v.push_back(columns[18] + " = " + std::to_string(e.augslot3type));
		v.push_back(columns[19] + " = " + std::to_string(e.augslot3visible));
		v.push_back(columns[20] + " = " + std::to_string(e.augslot4type));
		v.push_back(columns[21] + " = " + std::to_string(e.augslot4visible));
		v.push_back(columns[22] + " = " + std::to_string(e.augslot5type));
		v.push_back(columns[23] + " = " + std::to_string(e.augslot5visible));
		v.push_back(columns[24] + " = " + std::to_string(e.augslot6type));
		v.push_back(columns[25] + " = " + std::to_string(e.augslot6visible));
		v.push_back(columns[26] + " = " + std::to_string(e.augtype));
		v.push_back(columns[27] + " = " + std::to_string(e.avoidance));
		v.push_back(columns[28] + " = " + std::to_string(e.awis));
		v.push_back(columns[29] + " = " + std::to_string(e.bagsize));
		v.push_back(columns[30] + " = " + std::to_string(e.bagslots));
		v.push_back(columns[31] + " = " + std::to_string(e.bagtype));
		v.push_back(columns[32] + " = " + std::to_string(e.bagwr));
		v.push_back(columns[33] + " = " + std::to_string(e.banedmgamt));
		v.push_back(columns[34] + " = " + std::to_string(e.banedmgraceamt));
		v.push_back(columns[35] + " = " + std::to_string(e.banedmgbody));
		v.push_back(columns[36] + " = " + std::to_string(e.banedmgrace));
		v.push_back(columns[37] + " = " + std::to_string(e.bardtype));
		v.push_back(columns[38] + " = " + std::to_string(e.bardvalue));
		v.push_back(columns[39] + " = " + std::to_string(e.book));
		v.push_back(columns[40] + " = " + std::to_string(e.casttime));
		v.push_back(columns[41] + " = " + std::to_string(e.casttime_));
		v.push_back(columns[42] + " = '" + Strings::Escape(e.charmfile) + "'");
		v.push_back(columns[43] + " = '" + Strings::Escape(e.charmfileid) + "'");
		v.push_back(columns[44] + " = " + std::to_string(e.classes));
		v.push_back(columns[45] + " = " + std::to_string(e.color));
		v.push_back(columns[46] + " = '" + Strings::Escape(e.combateffects) + "'");
		v.push_back(columns[47] + " = " + std::to_string(e.extradmgskill));
		v.push_back(columns[48] + " = " + std::to_string(e.extradmgamt));
		v.push_back(columns[49] + " = " + std::to_string(e.price));
		v.push_back(columns[50] + " = " + std::to_string(e.cr));
		v.push_back(columns[51] + " = " + std::to_string(e.damage));
		v.push_back(columns[52] + " = " + std::to_string(e.damageshield));
		v.push_back(columns[53] + " = " + std::to_string(e.deity));
		v.push_back(columns[54] + " = " + std::to_string(e.delay));
		v.push_back(columns[55] + " = " + std::to_string(e.augdistiller));
		v.push_back(columns[56] + " = " + std::to_string(e.dotshielding));
		v.push_back(columns[57] + " = " + std::to_string(e.dr));
		v.push_back(columns[58] + " = " + std::to_string(e.clicktype));
		v.push_back(columns[59] + " = " + std::to_string(e.clicklevel2));
		v.push_back(columns[60] + " = " + std::to_string(e.elemdmgtype));
		v.push_back(columns[61] + " = " + std::to_string(e.elemdmgamt));
		v.push_back(columns[62] + " = " + std::to_string(e.endur));
		v.push_back(columns[63] + " = " + std::to_string(e.factionamt1));
		v.push_back(columns[64] + " = " + std::to_string(e.factionamt2));
		v.push_back(columns[65] + " = " + std::to_string(e.factionamt3));
		v.push_back(columns[66] + " = " + std::to_string(e.factionamt4));
		v.push_back(columns[67] + " = " + std::to_string(e.factionmod1));
		v.push_back(columns[68] + " = " + std::to_string(e.factionmod2));
		v.push_back(columns[69] + " = " + std::to_string(e.factionmod3));
		v.push_back(columns[70] + " = " + std::to_string(e.factionmod4));
		v.push_back(columns[71] + " = '" + Strings::Escape(e.filename) + "'");
		v.push_back(columns[72] + " = " + std::to_string(e.focuseffect));
		v.push_back(columns[73] + " = " + std::to_string(e.fr));
		v.push_back(columns[74] + " = " + std::to_string(e.fvnodrop));
		v.push_back(columns[75] + " = " + std::to_string(e.haste));
		v.push_back(columns[76] + " = " + std::to_string(e.clicklevel));
		v.push_back(columns[77] + " = " + std::to_string(e.hp));
		v.push_back(columns[78] + " = " + std::to_string(e.regen));
		v.push_back(columns[79] + " = " + std::to_string(e.icon));
		v.push_back(columns[80] + " = '" + Strings::Escape(e.idfile) + "'");
		v.push_back(columns[81] + " = " + std::to_string(e.itemclass));
		v.push_back(columns[82] + " = " + std::to_string(e.itemtype));
		v.push_back(columns[83] + " = " + std::to_string(e.ldonprice));
		v.push_back(columns[84] + " = " + std::to_string(e.ldontheme));
		v.push_back(columns[85] + " = " + std::to_string(e.ldonsold));
		v.push_back(columns[86] + " = " + std::to_string(e.light));
		v.push_back(columns[87] + " = '" + Strings::Escape(e.lore) + "'");
		v.push_back(columns[88] + " = " + std::to_string(e.loregroup));
		v.push_back(columns[89] + " = " + std::to_string(e.magic));
		v.push_back(columns[90] + " = " + std::to_string(e.mana));
		v.push_back(columns[91] + " = " + std::to_string(e.manaregen));
		v.push_back(columns[92] + " = " + std::to_string(e.enduranceregen));
		v.push_back(columns[93] + " = " + std::to_string(e.material));
		v.push_back(columns[94] + " = " + std::to_string(e.herosforgemodel));
		v.push_back(columns[95] + " = " + std::to_string(e.maxcharges));
		v.push_back(columns[96] + " = " + std::to_string(e.mr));
		v.push_back(columns[97] + " = " + std::to_string(e.nodrop));
		v.push_back(columns[98] + " = " + std::to_string(e.norent));
		v.push_back(columns[99] + " = " + std::to_string(e.pendingloreflag));
		v.push_back(columns[100] + " = " + std::to_string(e.pr));
		v.push_back(columns[101] + " = " + std::to_string(e.procrate));
		v.push_back(columns[102] + " = " + std::to_string(e.races));
		v.push_back(columns[103] + " = " + std::to_string(e.range));
		v.push_back(columns[104] + " = " + std::to_string(e.reclevel));
		v.push_back(columns[105] + " = " + std::to_string(e.recskill));
		v.push_back(columns[106] + " = " + std::to_string(e.reqlevel));
		v.push_back(columns[107] + " = " + std::to_string(e.sellrate));
		v.push_back(columns[108] + " = " + std::to_string(e.shielding));
		v.push_back(columns[109] + " = " + std::to_string(e.size));
		v.push_back(columns[110] + " = " + std::to_string(e.skillmodtype));
		v.push_back(columns[111] + " = " + std::to_string(e.skillmodvalue));
		v.push_back(columns[112] + " = " + std::to_string(e.slots));
		v.push_back(columns[113] + " = " + std::to_string(e.clickeffect));
		v.push_back(columns[114] + " = " + std::to_string(e.spellshield));
		v.push_back(columns[115] + " = " + std::to_string(e.strikethrough));
		v.push_back(columns[116] + " = " + std::to_string(e.stunresist));
		v.push_back(columns[117] + " = " + std::to_string(e.summonedflag));
		v.push_back(columns[118] + " = " + std::to_string(e.tradeskills));
		v.push_back(columns[119] + " = " + std::to_string(e.favor));
		v.push_back(columns[120] + " = " + std::to_string(e.weight));
		v.push_back(columns[121] + " = " + std::to_string(e.UNK012));
		v.push_back(columns[122] + " = " + std::to_string(e.UNK013));
		v.push_back(columns[123] + " = " + std::to_string(e.benefitflag));
		v.push_back(columns[124] + " = " + std::to_string(e.UNK054));
		v.push_back(columns[125] + " = " + std::to_string(e.UNK059));
		v.push_back(columns[126] + " = " + std::to_string(e.booktype));
		v.push_back(columns[127] + " = " + std::to_string(e.recastdelay));
		v.push_back(columns[128] + " = " + std::to_string(e.recasttype));
		v.push_back(columns[129] + " = " + std::to_string(e.guildfavor));
		v.push_back(columns[130] + " = " + std::to_string(e.UNK123));
		v.push_back(columns[131] + " = " + std::to_string(e.UNK124));
		v.push_back(columns[132] + " = " + std::to_string(e.attuneable));
		v.push_back(columns[133] + " = " + std::to_string(e.nopet));
		v.push_back(columns[134] + " = FROM_UNIXTIME(" + (e.updated > 0 ? std::to_string(e.updated) : "null") + ")");
		v.push_back(columns[135] + " = '" + Strings::Escape(e.comment) + "'");
		v.push_back(columns[136] + " = " + std::to_string(e.UNK127));
		v.push_back(columns[137] + " = " + std::to_string(e.pointtype));
		v.push_back(columns[138] + " = " + std::to_string(e.potionbelt));
		v.push_back(columns[139] + " = " + std::to_string(e.potionbeltslots));
		v.push_back(columns[140] + " = " + std::to_string(e.stacksize));
		v.push_back(columns[141] + " = " + std::to_string(e.notransfer));
		v.push_back(columns[142] + " = " + std::to_string(e.stackable));
		v.push_back(columns[143] + " = '" + Strings::Escape(e.UNK134) + "'");
		v.push_back(columns[144] + " = " + std::to_string(e.UNK137));
		v.push_back(columns[145] + " = " + std::to_string(e.proceffect));
		v.push_back(columns[146] + " = " + std::to_string(e.proctype));
		v.push_back(columns[147] + " = " + std::to_string(e.proclevel2));
		v.push_back(columns[148] + " = " + std::to_string(e.proclevel));
		v.push_back(columns[149] + " = " + std::to_string(e.UNK142));
		v.push_back(columns[150] + " = " + std::to_string(e.worneffect));
		v.push_back(columns[151] + " = " + std::to_string(e.worntype));
		v.push_back(columns[152] + " = " + std::to_string(e.wornlevel2));
		v.push_back(columns[153] + " = " + std::to_string(e.wornlevel));
		v.push_back(columns[154] + " = " + std::to_string(e.UNK147));
		v.push_back(columns[155] + " = " + std::to_string(e.focustype));
		v.push_back(columns[156] + " = " + std::to_string(e.focuslevel2));
		v.push_back(columns[157] + " = " + std::to_string(e.focuslevel));
		v.push_back(columns[158] + " = " + std::to_string(e.UNK152));
		v.push_back(columns[159] + " = " + std::to_string(e.scrolleffect));
		v.push_back(columns[160] + " = " + std::to_string(e.scrolltype));
		v.push_back(columns[161] + " = " + std::to_string(e.scrolllevel2));
		v.push_back(columns[162] + " = " + std::to_string(e.scrolllevel));
		v.push_back(columns[163] + " = " + std::to_string(e.UNK157));
		v.push_back(columns[164] + " = FROM_UNIXTIME(" + (e.serialized > 0 ? std::to_string(e.serialized) : "null") + ")");
		v.push_back(columns[165] + " = FROM_UNIXTIME(" + (e.verified > 0 ? std::to_string(e.verified) : "null") + ")");
		v.push_back(columns[166] + " = '" + Strings::Escape(e.serialization) + "'");
		v.push_back(columns[167] + " = '" + Strings::Escape(e.source) + "'");
		v.push_back(columns[168] + " = " + std::to_string(e.UNK033));
		v.push_back(columns[169] + " = '" + Strings::Escape(e.lorefile) + "'");
		v.push_back(columns[170] + " = " + std::to_string(e.UNK014));
		v.push_back(columns[171] + " = " + std::to_string(e.svcorruption));
		v.push_back(columns[172] + " = " + std::to_string(e.skillmodmax));
		v.push_back(columns[173] + " = " + std::to_string(e.UNK060));
		v.push_back(columns[174] + " = " + std::to_string(e.augslot1unk2));
		v.push_back(columns[175] + " = " + std::to_string(e.augslot2unk2));
		v.push_back(columns[176] + " = " + std::to_string(e.augslot3unk2));
		v.push_back(columns[177] + " = " + std::to_string(e.augslot4unk2));
		v.push_back(columns[178] + " = " + std::to_string(e.augslot5unk2));
		v.push_back(columns[179] + " = " + std::to_string(e.augslot6unk2));
		v.push_back(columns[180] + " = " + std::to_string(e.UNK120));
		v.push_back(columns[181] + " = " + std::to_string(e.UNK121));
		v.push_back(columns[182] + " = " + std::to_string(e.questitemflag));
		v.push_back(columns[183] + " = '" + Strings::Escape(e.UNK132) + "'");
		v.push_back(columns[184] + " = " + std::to_string(e.clickunk5));
		v.push_back(columns[185] + " = '" + Strings::Escape(e.clickunk6) + "'");
		v.push_back(columns[186] + " = " + std::to_string(e.clickunk7));
		v.push_back(columns[187] + " = " + std::to_string(e.procunk1));
		v.push_back(columns[188] + " = " + std::to_string(e.procunk2));
		v.push_back(columns[189] + " = " + std::to_string(e.procunk3));
		v.push_back(columns[190] + " = " + std::to_string(e.procunk4));
		v.push_back(columns[191] + " = '" + Strings::Escape(e.procunk6) + "'");
		v.push_back(columns[192] + " = " + std::to_string(e.procunk7));
		v.push_back(columns[193] + " = " + std::to_string(e.wornunk1));
		v.push_back(columns[194] + " = " + std::to_string(e.wornunk2));
		v.push_back(columns[195] + " = " + std::to_string(e.wornunk3));
		v.push_back(columns[196] + " = " + std::to_string(e.wornunk4));
		v.push_back(columns[197] + " = " + std::to_string(e.wornunk5));
		v.push_back(columns[198] + " = '" + Strings::Escape(e.wornunk6) + "'");
		v.push_back(columns[199] + " = " + std::to_string(e.wornunk7));
		v.push_back(columns[200] + " = " + std::to_string(e.focusunk1));
		v.push_back(columns[201] + " = " + std::to_string(e.focusunk2));
		v.push_back(columns[202] + " = " + std::to_string(e.focusunk3));
		v.push_back(columns[203] + " = " + std::to_string(e.focusunk4));
		v.push_back(columns[204] + " = " + std::to_string(e.focusunk5));
		v.push_back(columns[205] + " = '" + Strings::Escape(e.focusunk6) + "'");
		v.push_back(columns[206] + " = " + std::to_string(e.focusunk7));
		v.push_back(columns[207] + " = " + std::to_string(e.scrollunk1));
		v.push_back(columns[208] + " = " + std::to_string(e.scrollunk2));
		v.push_back(columns[209] + " = " + std::to_string(e.scrollunk3));
		v.push_back(columns[210] + " = " + std::to_string(e.scrollunk4));
		v.push_back(columns[211] + " = " + std::to_string(e.scrollunk5));
		v.push_back(columns[212] + " = '" + Strings::Escape(e.scrollunk6) + "'");
		v.push_back(columns[213] + " = " + std::to_string(e.scrollunk7));
		v.push_back(columns[214] + " = " + std::to_string(e.UNK193));
		v.push_back(columns[215] + " = " + std::to_string(e.purity));
		v.push_back(columns[216] + " = " + std::to_string(e.evoitem));
		v.push_back(columns[217] + " = " + std::to_string(e.evoid));
		v.push_back(columns[218] + " = " + std::to_string(e.evolvinglevel));
		v.push_back(columns[219] + " = " + std::to_string(e.evomax));
		v.push_back(columns[220] + " = '" + Strings::Escape(e.clickname) + "'");
		v.push_back(columns[221] + " = '" + Strings::Escape(e.procname) + "'");
		v.push_back(columns[222] + " = '" + Strings::Escape(e.wornname) + "'");
		v.push_back(columns[223] + " = '" + Strings::Escape(e.focusname) + "'");
		v.push_back(columns[224] + " = '" + Strings::Escape(e.scrollname) + "'");
		v.push_back(columns[225] + " = " + std::to_string(e.dsmitigation));
		v.push_back(columns[226] + " = " + std::to_string(e.heroic_str));
		v.push_back(columns[227] + " = " + std::to_string(e.heroic_int));
		v.push_back(columns[228] + " = " + std::to_string(e.heroic_wis));
		v.push_back(columns[229] + " = " + std::to_string(e.heroic_agi));
		v.push_back(columns[230] + " = " + std::to_string(e.heroic_dex));
		v.push_back(columns[231] + " = " + std::to_string(e.heroic_sta));
		v.push_back(columns[232] + " = " + std::to_string(e.heroic_cha));
		v.push_back(columns[233] + " = " + std::to_string(e.heroic_pr));
		v.push_back(columns[234] + " = " + std::to_string(e.heroic_dr));
		v.push_back(columns[235] + " = " + std::to_string(e.heroic_fr));
		v.push_back(columns[236] + " = " + std::to_string(e.heroic_cr));
		v.push_back(columns[237] + " = " + std::to_string(e.heroic_mr));
		v.push_back(columns[238] + " = " + std::to_string(e.heroic_svcorrup));
		v.push_back(columns[239] + " = " + std::to_string(e.healamt));
		v.push_back(columns[240] + " = " + std::to_string(e.spelldmg));
		v.push_back(columns[241] + " = " + std::to_string(e.clairvoyance));
		v.push_back(columns[242] + " = " + std::to_string(e.backstabdmg));
		v.push_back(columns[243] + " = '" + Strings::Escape(e.created) + "'");
		v.push_back(columns[244] + " = " + std::to_string(e.elitematerial));
		v.push_back(columns[245] + " = " + std::to_string(e.ldonsellbackrate));
		v.push_back(columns[246] + " = " + std::to_string(e.scriptfileid));
		v.push_back(columns[247] + " = " + std::to_string(e.expendablearrow));
		v.push_back(columns[248] + " = " + std::to_string(e.powersourcecapacity));
		v.push_back(columns[249] + " = " + std::to_string(e.bardeffect));
		v.push_back(columns[250] + " = " + std::to_string(e.bardeffecttype));
		v.push_back(columns[251] + " = " + std::to_string(e.bardlevel2));
		v.push_back(columns[252] + " = " + std::to_string(e.bardlevel));
		v.push_back(columns[253] + " = " + std::to_string(e.bardunk1));
		v.push_back(columns[254] + " = " + std::to_string(e.bardunk2));
		v.push_back(columns[255] + " = " + std::to_string(e.bardunk3));
		v.push_back(columns[256] + " = " + std::to_string(e.bardunk4));
		v.push_back(columns[257] + " = " + std::to_string(e.bardunk5));
		v.push_back(columns[258] + " = '" + Strings::Escape(e.bardname) + "'");
		v.push_back(columns[259] + " = " + std::to_string(e.bardunk7));
		v.push_back(columns[260] + " = " + std::to_string(e.UNK214));
		v.push_back(columns[261] + " = " + std::to_string(e.subtype));
		v.push_back(columns[262] + " = " + std::to_string(e.UNK220));
		v.push_back(columns[263] + " = " + std::to_string(e.UNK221));
		v.push_back(columns[264] + " = " + std::to_string(e.heirloom));
		v.push_back(columns[265] + " = " + std::to_string(e.UNK223));
		v.push_back(columns[266] + " = " + std::to_string(e.UNK224));
		v.push_back(columns[267] + " = " + std::to_string(e.UNK225));
		v.push_back(columns[268] + " = " + std::to_string(e.UNK226));
		v.push_back(columns[269] + " = " + std::to_string(e.UNK227));
		v.push_back(columns[270] + " = " + std::to_string(e.UNK228));
		v.push_back(columns[271] + " = " + std::to_string(e.UNK229));
		v.push_back(columns[272] + " = " + std::to_string(e.UNK230));
		v.push_back(columns[273] + " = " + std::to_string(e.UNK231));
		v.push_back(columns[274] + " = " + std::to_string(e.UNK232));
		v.push_back(columns[275] + " = " + std::to_string(e.UNK233));
		v.push_back(columns[276] + " = " + std::to_string(e.UNK234));
		v.push_back(columns[277] + " = " + std::to_string(e.placeable));
		v.push_back(columns[278] + " = " + std::to_string(e.UNK236));
		v.push_back(columns[279] + " = " + std::to_string(e.UNK237));
		v.push_back(columns[280] + " = " + std::to_string(e.UNK238));
		v.push_back(columns[281] + " = " + std::to_string(e.UNK239));
		v.push_back(columns[282] + " = " + std::to_string(e.UNK240));
		v.push_back(columns[283] + " = " + std::to_string(e.UNK241));
		v.push_back(columns[284] + " = " + std::to_string(e.epicitem));

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

	static Items InsertOne(
		Database& db,
		Items e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.minstatus));
		v.push_back("'" + Strings::Escape(e.Name) + "'");
		v.push_back(std::to_string(e.aagi));
		v.push_back(std::to_string(e.ac));
		v.push_back(std::to_string(e.accuracy));
		v.push_back(std::to_string(e.acha));
		v.push_back(std::to_string(e.adex));
		v.push_back(std::to_string(e.aint));
		v.push_back(std::to_string(e.artifactflag));
		v.push_back(std::to_string(e.asta));
		v.push_back(std::to_string(e.astr));
		v.push_back(std::to_string(e.attack));
		v.push_back(std::to_string(e.augrestrict));
		v.push_back(std::to_string(e.augslot1type));
		v.push_back(std::to_string(e.augslot1visible));
		v.push_back(std::to_string(e.augslot2type));
		v.push_back(std::to_string(e.augslot2visible));
		v.push_back(std::to_string(e.augslot3type));
		v.push_back(std::to_string(e.augslot3visible));
		v.push_back(std::to_string(e.augslot4type));
		v.push_back(std::to_string(e.augslot4visible));
		v.push_back(std::to_string(e.augslot5type));
		v.push_back(std::to_string(e.augslot5visible));
		v.push_back(std::to_string(e.augslot6type));
		v.push_back(std::to_string(e.augslot6visible));
		v.push_back(std::to_string(e.augtype));
		v.push_back(std::to_string(e.avoidance));
		v.push_back(std::to_string(e.awis));
		v.push_back(std::to_string(e.bagsize));
		v.push_back(std::to_string(e.bagslots));
		v.push_back(std::to_string(e.bagtype));
		v.push_back(std::to_string(e.bagwr));
		v.push_back(std::to_string(e.banedmgamt));
		v.push_back(std::to_string(e.banedmgraceamt));
		v.push_back(std::to_string(e.banedmgbody));
		v.push_back(std::to_string(e.banedmgrace));
		v.push_back(std::to_string(e.bardtype));
		v.push_back(std::to_string(e.bardvalue));
		v.push_back(std::to_string(e.book));
		v.push_back(std::to_string(e.casttime));
		v.push_back(std::to_string(e.casttime_));
		v.push_back("'" + Strings::Escape(e.charmfile) + "'");
		v.push_back("'" + Strings::Escape(e.charmfileid) + "'");
		v.push_back(std::to_string(e.classes));
		v.push_back(std::to_string(e.color));
		v.push_back("'" + Strings::Escape(e.combateffects) + "'");
		v.push_back(std::to_string(e.extradmgskill));
		v.push_back(std::to_string(e.extradmgamt));
		v.push_back(std::to_string(e.price));
		v.push_back(std::to_string(e.cr));
		v.push_back(std::to_string(e.damage));
		v.push_back(std::to_string(e.damageshield));
		v.push_back(std::to_string(e.deity));
		v.push_back(std::to_string(e.delay));
		v.push_back(std::to_string(e.augdistiller));
		v.push_back(std::to_string(e.dotshielding));
		v.push_back(std::to_string(e.dr));
		v.push_back(std::to_string(e.clicktype));
		v.push_back(std::to_string(e.clicklevel2));
		v.push_back(std::to_string(e.elemdmgtype));
		v.push_back(std::to_string(e.elemdmgamt));
		v.push_back(std::to_string(e.endur));
		v.push_back(std::to_string(e.factionamt1));
		v.push_back(std::to_string(e.factionamt2));
		v.push_back(std::to_string(e.factionamt3));
		v.push_back(std::to_string(e.factionamt4));
		v.push_back(std::to_string(e.factionmod1));
		v.push_back(std::to_string(e.factionmod2));
		v.push_back(std::to_string(e.factionmod3));
		v.push_back(std::to_string(e.factionmod4));
		v.push_back("'" + Strings::Escape(e.filename) + "'");
		v.push_back(std::to_string(e.focuseffect));
		v.push_back(std::to_string(e.fr));
		v.push_back(std::to_string(e.fvnodrop));
		v.push_back(std::to_string(e.haste));
		v.push_back(std::to_string(e.clicklevel));
		v.push_back(std::to_string(e.hp));
		v.push_back(std::to_string(e.regen));
		v.push_back(std::to_string(e.icon));
		v.push_back("'" + Strings::Escape(e.idfile) + "'");
		v.push_back(std::to_string(e.itemclass));
		v.push_back(std::to_string(e.itemtype));
		v.push_back(std::to_string(e.ldonprice));
		v.push_back(std::to_string(e.ldontheme));
		v.push_back(std::to_string(e.ldonsold));
		v.push_back(std::to_string(e.light));
		v.push_back("'" + Strings::Escape(e.lore) + "'");
		v.push_back(std::to_string(e.loregroup));
		v.push_back(std::to_string(e.magic));
		v.push_back(std::to_string(e.mana));
		v.push_back(std::to_string(e.manaregen));
		v.push_back(std::to_string(e.enduranceregen));
		v.push_back(std::to_string(e.material));
		v.push_back(std::to_string(e.herosforgemodel));
		v.push_back(std::to_string(e.maxcharges));
		v.push_back(std::to_string(e.mr));
		v.push_back(std::to_string(e.nodrop));
		v.push_back(std::to_string(e.norent));
		v.push_back(std::to_string(e.pendingloreflag));
		v.push_back(std::to_string(e.pr));
		v.push_back(std::to_string(e.procrate));
		v.push_back(std::to_string(e.races));
		v.push_back(std::to_string(e.range));
		v.push_back(std::to_string(e.reclevel));
		v.push_back(std::to_string(e.recskill));
		v.push_back(std::to_string(e.reqlevel));
		v.push_back(std::to_string(e.sellrate));
		v.push_back(std::to_string(e.shielding));
		v.push_back(std::to_string(e.size));
		v.push_back(std::to_string(e.skillmodtype));
		v.push_back(std::to_string(e.skillmodvalue));
		v.push_back(std::to_string(e.slots));
		v.push_back(std::to_string(e.clickeffect));
		v.push_back(std::to_string(e.spellshield));
		v.push_back(std::to_string(e.strikethrough));
		v.push_back(std::to_string(e.stunresist));
		v.push_back(std::to_string(e.summonedflag));
		v.push_back(std::to_string(e.tradeskills));
		v.push_back(std::to_string(e.favor));
		v.push_back(std::to_string(e.weight));
		v.push_back(std::to_string(e.UNK012));
		v.push_back(std::to_string(e.UNK013));
		v.push_back(std::to_string(e.benefitflag));
		v.push_back(std::to_string(e.UNK054));
		v.push_back(std::to_string(e.UNK059));
		v.push_back(std::to_string(e.booktype));
		v.push_back(std::to_string(e.recastdelay));
		v.push_back(std::to_string(e.recasttype));
		v.push_back(std::to_string(e.guildfavor));
		v.push_back(std::to_string(e.UNK123));
		v.push_back(std::to_string(e.UNK124));
		v.push_back(std::to_string(e.attuneable));
		v.push_back(std::to_string(e.nopet));
		v.push_back("FROM_UNIXTIME(" + (e.updated > 0 ? std::to_string(e.updated) : "null") + ")");
		v.push_back("'" + Strings::Escape(e.comment) + "'");
		v.push_back(std::to_string(e.UNK127));
		v.push_back(std::to_string(e.pointtype));
		v.push_back(std::to_string(e.potionbelt));
		v.push_back(std::to_string(e.potionbeltslots));
		v.push_back(std::to_string(e.stacksize));
		v.push_back(std::to_string(e.notransfer));
		v.push_back(std::to_string(e.stackable));
		v.push_back("'" + Strings::Escape(e.UNK134) + "'");
		v.push_back(std::to_string(e.UNK137));
		v.push_back(std::to_string(e.proceffect));
		v.push_back(std::to_string(e.proctype));
		v.push_back(std::to_string(e.proclevel2));
		v.push_back(std::to_string(e.proclevel));
		v.push_back(std::to_string(e.UNK142));
		v.push_back(std::to_string(e.worneffect));
		v.push_back(std::to_string(e.worntype));
		v.push_back(std::to_string(e.wornlevel2));
		v.push_back(std::to_string(e.wornlevel));
		v.push_back(std::to_string(e.UNK147));
		v.push_back(std::to_string(e.focustype));
		v.push_back(std::to_string(e.focuslevel2));
		v.push_back(std::to_string(e.focuslevel));
		v.push_back(std::to_string(e.UNK152));
		v.push_back(std::to_string(e.scrolleffect));
		v.push_back(std::to_string(e.scrolltype));
		v.push_back(std::to_string(e.scrolllevel2));
		v.push_back(std::to_string(e.scrolllevel));
		v.push_back(std::to_string(e.UNK157));
		v.push_back("FROM_UNIXTIME(" + (e.serialized > 0 ? std::to_string(e.serialized) : "null") + ")");
		v.push_back("FROM_UNIXTIME(" + (e.verified > 0 ? std::to_string(e.verified) : "null") + ")");
		v.push_back("'" + Strings::Escape(e.serialization) + "'");
		v.push_back("'" + Strings::Escape(e.source) + "'");
		v.push_back(std::to_string(e.UNK033));
		v.push_back("'" + Strings::Escape(e.lorefile) + "'");
		v.push_back(std::to_string(e.UNK014));
		v.push_back(std::to_string(e.svcorruption));
		v.push_back(std::to_string(e.skillmodmax));
		v.push_back(std::to_string(e.UNK060));
		v.push_back(std::to_string(e.augslot1unk2));
		v.push_back(std::to_string(e.augslot2unk2));
		v.push_back(std::to_string(e.augslot3unk2));
		v.push_back(std::to_string(e.augslot4unk2));
		v.push_back(std::to_string(e.augslot5unk2));
		v.push_back(std::to_string(e.augslot6unk2));
		v.push_back(std::to_string(e.UNK120));
		v.push_back(std::to_string(e.UNK121));
		v.push_back(std::to_string(e.questitemflag));
		v.push_back("'" + Strings::Escape(e.UNK132) + "'");
		v.push_back(std::to_string(e.clickunk5));
		v.push_back("'" + Strings::Escape(e.clickunk6) + "'");
		v.push_back(std::to_string(e.clickunk7));
		v.push_back(std::to_string(e.procunk1));
		v.push_back(std::to_string(e.procunk2));
		v.push_back(std::to_string(e.procunk3));
		v.push_back(std::to_string(e.procunk4));
		v.push_back("'" + Strings::Escape(e.procunk6) + "'");
		v.push_back(std::to_string(e.procunk7));
		v.push_back(std::to_string(e.wornunk1));
		v.push_back(std::to_string(e.wornunk2));
		v.push_back(std::to_string(e.wornunk3));
		v.push_back(std::to_string(e.wornunk4));
		v.push_back(std::to_string(e.wornunk5));
		v.push_back("'" + Strings::Escape(e.wornunk6) + "'");
		v.push_back(std::to_string(e.wornunk7));
		v.push_back(std::to_string(e.focusunk1));
		v.push_back(std::to_string(e.focusunk2));
		v.push_back(std::to_string(e.focusunk3));
		v.push_back(std::to_string(e.focusunk4));
		v.push_back(std::to_string(e.focusunk5));
		v.push_back("'" + Strings::Escape(e.focusunk6) + "'");
		v.push_back(std::to_string(e.focusunk7));
		v.push_back(std::to_string(e.scrollunk1));
		v.push_back(std::to_string(e.scrollunk2));
		v.push_back(std::to_string(e.scrollunk3));
		v.push_back(std::to_string(e.scrollunk4));
		v.push_back(std::to_string(e.scrollunk5));
		v.push_back("'" + Strings::Escape(e.scrollunk6) + "'");
		v.push_back(std::to_string(e.scrollunk7));
		v.push_back(std::to_string(e.UNK193));
		v.push_back(std::to_string(e.purity));
		v.push_back(std::to_string(e.evoitem));
		v.push_back(std::to_string(e.evoid));
		v.push_back(std::to_string(e.evolvinglevel));
		v.push_back(std::to_string(e.evomax));
		v.push_back("'" + Strings::Escape(e.clickname) + "'");
		v.push_back("'" + Strings::Escape(e.procname) + "'");
		v.push_back("'" + Strings::Escape(e.wornname) + "'");
		v.push_back("'" + Strings::Escape(e.focusname) + "'");
		v.push_back("'" + Strings::Escape(e.scrollname) + "'");
		v.push_back(std::to_string(e.dsmitigation));
		v.push_back(std::to_string(e.heroic_str));
		v.push_back(std::to_string(e.heroic_int));
		v.push_back(std::to_string(e.heroic_wis));
		v.push_back(std::to_string(e.heroic_agi));
		v.push_back(std::to_string(e.heroic_dex));
		v.push_back(std::to_string(e.heroic_sta));
		v.push_back(std::to_string(e.heroic_cha));
		v.push_back(std::to_string(e.heroic_pr));
		v.push_back(std::to_string(e.heroic_dr));
		v.push_back(std::to_string(e.heroic_fr));
		v.push_back(std::to_string(e.heroic_cr));
		v.push_back(std::to_string(e.heroic_mr));
		v.push_back(std::to_string(e.heroic_svcorrup));
		v.push_back(std::to_string(e.healamt));
		v.push_back(std::to_string(e.spelldmg));
		v.push_back(std::to_string(e.clairvoyance));
		v.push_back(std::to_string(e.backstabdmg));
		v.push_back("'" + Strings::Escape(e.created) + "'");
		v.push_back(std::to_string(e.elitematerial));
		v.push_back(std::to_string(e.ldonsellbackrate));
		v.push_back(std::to_string(e.scriptfileid));
		v.push_back(std::to_string(e.expendablearrow));
		v.push_back(std::to_string(e.powersourcecapacity));
		v.push_back(std::to_string(e.bardeffect));
		v.push_back(std::to_string(e.bardeffecttype));
		v.push_back(std::to_string(e.bardlevel2));
		v.push_back(std::to_string(e.bardlevel));
		v.push_back(std::to_string(e.bardunk1));
		v.push_back(std::to_string(e.bardunk2));
		v.push_back(std::to_string(e.bardunk3));
		v.push_back(std::to_string(e.bardunk4));
		v.push_back(std::to_string(e.bardunk5));
		v.push_back("'" + Strings::Escape(e.bardname) + "'");
		v.push_back(std::to_string(e.bardunk7));
		v.push_back(std::to_string(e.UNK214));
		v.push_back(std::to_string(e.subtype));
		v.push_back(std::to_string(e.UNK220));
		v.push_back(std::to_string(e.UNK221));
		v.push_back(std::to_string(e.heirloom));
		v.push_back(std::to_string(e.UNK223));
		v.push_back(std::to_string(e.UNK224));
		v.push_back(std::to_string(e.UNK225));
		v.push_back(std::to_string(e.UNK226));
		v.push_back(std::to_string(e.UNK227));
		v.push_back(std::to_string(e.UNK228));
		v.push_back(std::to_string(e.UNK229));
		v.push_back(std::to_string(e.UNK230));
		v.push_back(std::to_string(e.UNK231));
		v.push_back(std::to_string(e.UNK232));
		v.push_back(std::to_string(e.UNK233));
		v.push_back(std::to_string(e.UNK234));
		v.push_back(std::to_string(e.placeable));
		v.push_back(std::to_string(e.UNK236));
		v.push_back(std::to_string(e.UNK237));
		v.push_back(std::to_string(e.UNK238));
		v.push_back(std::to_string(e.UNK239));
		v.push_back(std::to_string(e.UNK240));
		v.push_back(std::to_string(e.UNK241));
		v.push_back(std::to_string(e.epicitem));

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
		const std::vector<Items> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.minstatus));
			v.push_back("'" + Strings::Escape(e.Name) + "'");
			v.push_back(std::to_string(e.aagi));
			v.push_back(std::to_string(e.ac));
			v.push_back(std::to_string(e.accuracy));
			v.push_back(std::to_string(e.acha));
			v.push_back(std::to_string(e.adex));
			v.push_back(std::to_string(e.aint));
			v.push_back(std::to_string(e.artifactflag));
			v.push_back(std::to_string(e.asta));
			v.push_back(std::to_string(e.astr));
			v.push_back(std::to_string(e.attack));
			v.push_back(std::to_string(e.augrestrict));
			v.push_back(std::to_string(e.augslot1type));
			v.push_back(std::to_string(e.augslot1visible));
			v.push_back(std::to_string(e.augslot2type));
			v.push_back(std::to_string(e.augslot2visible));
			v.push_back(std::to_string(e.augslot3type));
			v.push_back(std::to_string(e.augslot3visible));
			v.push_back(std::to_string(e.augslot4type));
			v.push_back(std::to_string(e.augslot4visible));
			v.push_back(std::to_string(e.augslot5type));
			v.push_back(std::to_string(e.augslot5visible));
			v.push_back(std::to_string(e.augslot6type));
			v.push_back(std::to_string(e.augslot6visible));
			v.push_back(std::to_string(e.augtype));
			v.push_back(std::to_string(e.avoidance));
			v.push_back(std::to_string(e.awis));
			v.push_back(std::to_string(e.bagsize));
			v.push_back(std::to_string(e.bagslots));
			v.push_back(std::to_string(e.bagtype));
			v.push_back(std::to_string(e.bagwr));
			v.push_back(std::to_string(e.banedmgamt));
			v.push_back(std::to_string(e.banedmgraceamt));
			v.push_back(std::to_string(e.banedmgbody));
			v.push_back(std::to_string(e.banedmgrace));
			v.push_back(std::to_string(e.bardtype));
			v.push_back(std::to_string(e.bardvalue));
			v.push_back(std::to_string(e.book));
			v.push_back(std::to_string(e.casttime));
			v.push_back(std::to_string(e.casttime_));
			v.push_back("'" + Strings::Escape(e.charmfile) + "'");
			v.push_back("'" + Strings::Escape(e.charmfileid) + "'");
			v.push_back(std::to_string(e.classes));
			v.push_back(std::to_string(e.color));
			v.push_back("'" + Strings::Escape(e.combateffects) + "'");
			v.push_back(std::to_string(e.extradmgskill));
			v.push_back(std::to_string(e.extradmgamt));
			v.push_back(std::to_string(e.price));
			v.push_back(std::to_string(e.cr));
			v.push_back(std::to_string(e.damage));
			v.push_back(std::to_string(e.damageshield));
			v.push_back(std::to_string(e.deity));
			v.push_back(std::to_string(e.delay));
			v.push_back(std::to_string(e.augdistiller));
			v.push_back(std::to_string(e.dotshielding));
			v.push_back(std::to_string(e.dr));
			v.push_back(std::to_string(e.clicktype));
			v.push_back(std::to_string(e.clicklevel2));
			v.push_back(std::to_string(e.elemdmgtype));
			v.push_back(std::to_string(e.elemdmgamt));
			v.push_back(std::to_string(e.endur));
			v.push_back(std::to_string(e.factionamt1));
			v.push_back(std::to_string(e.factionamt2));
			v.push_back(std::to_string(e.factionamt3));
			v.push_back(std::to_string(e.factionamt4));
			v.push_back(std::to_string(e.factionmod1));
			v.push_back(std::to_string(e.factionmod2));
			v.push_back(std::to_string(e.factionmod3));
			v.push_back(std::to_string(e.factionmod4));
			v.push_back("'" + Strings::Escape(e.filename) + "'");
			v.push_back(std::to_string(e.focuseffect));
			v.push_back(std::to_string(e.fr));
			v.push_back(std::to_string(e.fvnodrop));
			v.push_back(std::to_string(e.haste));
			v.push_back(std::to_string(e.clicklevel));
			v.push_back(std::to_string(e.hp));
			v.push_back(std::to_string(e.regen));
			v.push_back(std::to_string(e.icon));
			v.push_back("'" + Strings::Escape(e.idfile) + "'");
			v.push_back(std::to_string(e.itemclass));
			v.push_back(std::to_string(e.itemtype));
			v.push_back(std::to_string(e.ldonprice));
			v.push_back(std::to_string(e.ldontheme));
			v.push_back(std::to_string(e.ldonsold));
			v.push_back(std::to_string(e.light));
			v.push_back("'" + Strings::Escape(e.lore) + "'");
			v.push_back(std::to_string(e.loregroup));
			v.push_back(std::to_string(e.magic));
			v.push_back(std::to_string(e.mana));
			v.push_back(std::to_string(e.manaregen));
			v.push_back(std::to_string(e.enduranceregen));
			v.push_back(std::to_string(e.material));
			v.push_back(std::to_string(e.herosforgemodel));
			v.push_back(std::to_string(e.maxcharges));
			v.push_back(std::to_string(e.mr));
			v.push_back(std::to_string(e.nodrop));
			v.push_back(std::to_string(e.norent));
			v.push_back(std::to_string(e.pendingloreflag));
			v.push_back(std::to_string(e.pr));
			v.push_back(std::to_string(e.procrate));
			v.push_back(std::to_string(e.races));
			v.push_back(std::to_string(e.range));
			v.push_back(std::to_string(e.reclevel));
			v.push_back(std::to_string(e.recskill));
			v.push_back(std::to_string(e.reqlevel));
			v.push_back(std::to_string(e.sellrate));
			v.push_back(std::to_string(e.shielding));
			v.push_back(std::to_string(e.size));
			v.push_back(std::to_string(e.skillmodtype));
			v.push_back(std::to_string(e.skillmodvalue));
			v.push_back(std::to_string(e.slots));
			v.push_back(std::to_string(e.clickeffect));
			v.push_back(std::to_string(e.spellshield));
			v.push_back(std::to_string(e.strikethrough));
			v.push_back(std::to_string(e.stunresist));
			v.push_back(std::to_string(e.summonedflag));
			v.push_back(std::to_string(e.tradeskills));
			v.push_back(std::to_string(e.favor));
			v.push_back(std::to_string(e.weight));
			v.push_back(std::to_string(e.UNK012));
			v.push_back(std::to_string(e.UNK013));
			v.push_back(std::to_string(e.benefitflag));
			v.push_back(std::to_string(e.UNK054));
			v.push_back(std::to_string(e.UNK059));
			v.push_back(std::to_string(e.booktype));
			v.push_back(std::to_string(e.recastdelay));
			v.push_back(std::to_string(e.recasttype));
			v.push_back(std::to_string(e.guildfavor));
			v.push_back(std::to_string(e.UNK123));
			v.push_back(std::to_string(e.UNK124));
			v.push_back(std::to_string(e.attuneable));
			v.push_back(std::to_string(e.nopet));
			v.push_back("FROM_UNIXTIME(" + (e.updated > 0 ? std::to_string(e.updated) : "null") + ")");
			v.push_back("'" + Strings::Escape(e.comment) + "'");
			v.push_back(std::to_string(e.UNK127));
			v.push_back(std::to_string(e.pointtype));
			v.push_back(std::to_string(e.potionbelt));
			v.push_back(std::to_string(e.potionbeltslots));
			v.push_back(std::to_string(e.stacksize));
			v.push_back(std::to_string(e.notransfer));
			v.push_back(std::to_string(e.stackable));
			v.push_back("'" + Strings::Escape(e.UNK134) + "'");
			v.push_back(std::to_string(e.UNK137));
			v.push_back(std::to_string(e.proceffect));
			v.push_back(std::to_string(e.proctype));
			v.push_back(std::to_string(e.proclevel2));
			v.push_back(std::to_string(e.proclevel));
			v.push_back(std::to_string(e.UNK142));
			v.push_back(std::to_string(e.worneffect));
			v.push_back(std::to_string(e.worntype));
			v.push_back(std::to_string(e.wornlevel2));
			v.push_back(std::to_string(e.wornlevel));
			v.push_back(std::to_string(e.UNK147));
			v.push_back(std::to_string(e.focustype));
			v.push_back(std::to_string(e.focuslevel2));
			v.push_back(std::to_string(e.focuslevel));
			v.push_back(std::to_string(e.UNK152));
			v.push_back(std::to_string(e.scrolleffect));
			v.push_back(std::to_string(e.scrolltype));
			v.push_back(std::to_string(e.scrolllevel2));
			v.push_back(std::to_string(e.scrolllevel));
			v.push_back(std::to_string(e.UNK157));
			v.push_back("FROM_UNIXTIME(" + (e.serialized > 0 ? std::to_string(e.serialized) : "null") + ")");
			v.push_back("FROM_UNIXTIME(" + (e.verified > 0 ? std::to_string(e.verified) : "null") + ")");
			v.push_back("'" + Strings::Escape(e.serialization) + "'");
			v.push_back("'" + Strings::Escape(e.source) + "'");
			v.push_back(std::to_string(e.UNK033));
			v.push_back("'" + Strings::Escape(e.lorefile) + "'");
			v.push_back(std::to_string(e.UNK014));
			v.push_back(std::to_string(e.svcorruption));
			v.push_back(std::to_string(e.skillmodmax));
			v.push_back(std::to_string(e.UNK060));
			v.push_back(std::to_string(e.augslot1unk2));
			v.push_back(std::to_string(e.augslot2unk2));
			v.push_back(std::to_string(e.augslot3unk2));
			v.push_back(std::to_string(e.augslot4unk2));
			v.push_back(std::to_string(e.augslot5unk2));
			v.push_back(std::to_string(e.augslot6unk2));
			v.push_back(std::to_string(e.UNK120));
			v.push_back(std::to_string(e.UNK121));
			v.push_back(std::to_string(e.questitemflag));
			v.push_back("'" + Strings::Escape(e.UNK132) + "'");
			v.push_back(std::to_string(e.clickunk5));
			v.push_back("'" + Strings::Escape(e.clickunk6) + "'");
			v.push_back(std::to_string(e.clickunk7));
			v.push_back(std::to_string(e.procunk1));
			v.push_back(std::to_string(e.procunk2));
			v.push_back(std::to_string(e.procunk3));
			v.push_back(std::to_string(e.procunk4));
			v.push_back("'" + Strings::Escape(e.procunk6) + "'");
			v.push_back(std::to_string(e.procunk7));
			v.push_back(std::to_string(e.wornunk1));
			v.push_back(std::to_string(e.wornunk2));
			v.push_back(std::to_string(e.wornunk3));
			v.push_back(std::to_string(e.wornunk4));
			v.push_back(std::to_string(e.wornunk5));
			v.push_back("'" + Strings::Escape(e.wornunk6) + "'");
			v.push_back(std::to_string(e.wornunk7));
			v.push_back(std::to_string(e.focusunk1));
			v.push_back(std::to_string(e.focusunk2));
			v.push_back(std::to_string(e.focusunk3));
			v.push_back(std::to_string(e.focusunk4));
			v.push_back(std::to_string(e.focusunk5));
			v.push_back("'" + Strings::Escape(e.focusunk6) + "'");
			v.push_back(std::to_string(e.focusunk7));
			v.push_back(std::to_string(e.scrollunk1));
			v.push_back(std::to_string(e.scrollunk2));
			v.push_back(std::to_string(e.scrollunk3));
			v.push_back(std::to_string(e.scrollunk4));
			v.push_back(std::to_string(e.scrollunk5));
			v.push_back("'" + Strings::Escape(e.scrollunk6) + "'");
			v.push_back(std::to_string(e.scrollunk7));
			v.push_back(std::to_string(e.UNK193));
			v.push_back(std::to_string(e.purity));
			v.push_back(std::to_string(e.evoitem));
			v.push_back(std::to_string(e.evoid));
			v.push_back(std::to_string(e.evolvinglevel));
			v.push_back(std::to_string(e.evomax));
			v.push_back("'" + Strings::Escape(e.clickname) + "'");
			v.push_back("'" + Strings::Escape(e.procname) + "'");
			v.push_back("'" + Strings::Escape(e.wornname) + "'");
			v.push_back("'" + Strings::Escape(e.focusname) + "'");
			v.push_back("'" + Strings::Escape(e.scrollname) + "'");
			v.push_back(std::to_string(e.dsmitigation));
			v.push_back(std::to_string(e.heroic_str));
			v.push_back(std::to_string(e.heroic_int));
			v.push_back(std::to_string(e.heroic_wis));
			v.push_back(std::to_string(e.heroic_agi));
			v.push_back(std::to_string(e.heroic_dex));
			v.push_back(std::to_string(e.heroic_sta));
			v.push_back(std::to_string(e.heroic_cha));
			v.push_back(std::to_string(e.heroic_pr));
			v.push_back(std::to_string(e.heroic_dr));
			v.push_back(std::to_string(e.heroic_fr));
			v.push_back(std::to_string(e.heroic_cr));
			v.push_back(std::to_string(e.heroic_mr));
			v.push_back(std::to_string(e.heroic_svcorrup));
			v.push_back(std::to_string(e.healamt));
			v.push_back(std::to_string(e.spelldmg));
			v.push_back(std::to_string(e.clairvoyance));
			v.push_back(std::to_string(e.backstabdmg));
			v.push_back("'" + Strings::Escape(e.created) + "'");
			v.push_back(std::to_string(e.elitematerial));
			v.push_back(std::to_string(e.ldonsellbackrate));
			v.push_back(std::to_string(e.scriptfileid));
			v.push_back(std::to_string(e.expendablearrow));
			v.push_back(std::to_string(e.powersourcecapacity));
			v.push_back(std::to_string(e.bardeffect));
			v.push_back(std::to_string(e.bardeffecttype));
			v.push_back(std::to_string(e.bardlevel2));
			v.push_back(std::to_string(e.bardlevel));
			v.push_back(std::to_string(e.bardunk1));
			v.push_back(std::to_string(e.bardunk2));
			v.push_back(std::to_string(e.bardunk3));
			v.push_back(std::to_string(e.bardunk4));
			v.push_back(std::to_string(e.bardunk5));
			v.push_back("'" + Strings::Escape(e.bardname) + "'");
			v.push_back(std::to_string(e.bardunk7));
			v.push_back(std::to_string(e.UNK214));
			v.push_back(std::to_string(e.subtype));
			v.push_back(std::to_string(e.UNK220));
			v.push_back(std::to_string(e.UNK221));
			v.push_back(std::to_string(e.heirloom));
			v.push_back(std::to_string(e.UNK223));
			v.push_back(std::to_string(e.UNK224));
			v.push_back(std::to_string(e.UNK225));
			v.push_back(std::to_string(e.UNK226));
			v.push_back(std::to_string(e.UNK227));
			v.push_back(std::to_string(e.UNK228));
			v.push_back(std::to_string(e.UNK229));
			v.push_back(std::to_string(e.UNK230));
			v.push_back(std::to_string(e.UNK231));
			v.push_back(std::to_string(e.UNK232));
			v.push_back(std::to_string(e.UNK233));
			v.push_back(std::to_string(e.UNK234));
			v.push_back(std::to_string(e.placeable));
			v.push_back(std::to_string(e.UNK236));
			v.push_back(std::to_string(e.UNK237));
			v.push_back(std::to_string(e.UNK238));
			v.push_back(std::to_string(e.UNK239));
			v.push_back(std::to_string(e.UNK240));
			v.push_back(std::to_string(e.UNK241));
			v.push_back(std::to_string(e.epicitem));

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

	static std::vector<Items> All(Database& db)
	{
		std::vector<Items> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Items e{};

			e.id                  = atoi(row[0]);
			e.minstatus           = atoi(row[1]);
			e.Name                = row[2] ? row[2] : "";
			e.aagi                = atoi(row[3]);
			e.ac                  = atoi(row[4]);
			e.accuracy            = atoi(row[5]);
			e.acha                = atoi(row[6]);
			e.adex                = atoi(row[7]);
			e.aint                = atoi(row[8]);
			e.artifactflag        = atoi(row[9]);
			e.asta                = atoi(row[10]);
			e.astr                = atoi(row[11]);
			e.attack              = atoi(row[12]);
			e.augrestrict         = atoi(row[13]);
			e.augslot1type        = atoi(row[14]);
			e.augslot1visible     = atoi(row[15]);
			e.augslot2type        = atoi(row[16]);
			e.augslot2visible     = atoi(row[17]);
			e.augslot3type        = atoi(row[18]);
			e.augslot3visible     = atoi(row[19]);
			e.augslot4type        = atoi(row[20]);
			e.augslot4visible     = atoi(row[21]);
			e.augslot5type        = atoi(row[22]);
			e.augslot5visible     = atoi(row[23]);
			e.augslot6type        = atoi(row[24]);
			e.augslot6visible     = atoi(row[25]);
			e.augtype             = atoi(row[26]);
			e.avoidance           = atoi(row[27]);
			e.awis                = atoi(row[28]);
			e.bagsize             = atoi(row[29]);
			e.bagslots            = atoi(row[30]);
			e.bagtype             = atoi(row[31]);
			e.bagwr               = atoi(row[32]);
			e.banedmgamt          = atoi(row[33]);
			e.banedmgraceamt      = atoi(row[34]);
			e.banedmgbody         = atoi(row[35]);
			e.banedmgrace         = atoi(row[36]);
			e.bardtype            = atoi(row[37]);
			e.bardvalue           = atoi(row[38]);
			e.book                = atoi(row[39]);
			e.casttime            = atoi(row[40]);
			e.casttime_           = atoi(row[41]);
			e.charmfile           = row[42] ? row[42] : "";
			e.charmfileid         = row[43] ? row[43] : "";
			e.classes             = atoi(row[44]);
			e.color               = atoi(row[45]);
			e.combateffects       = row[46] ? row[46] : "";
			e.extradmgskill       = atoi(row[47]);
			e.extradmgamt         = atoi(row[48]);
			e.price               = atoi(row[49]);
			e.cr                  = atoi(row[50]);
			e.damage              = atoi(row[51]);
			e.damageshield        = atoi(row[52]);
			e.deity               = atoi(row[53]);
			e.delay               = atoi(row[54]);
			e.augdistiller        = atoi(row[55]);
			e.dotshielding        = atoi(row[56]);
			e.dr                  = atoi(row[57]);
			e.clicktype           = atoi(row[58]);
			e.clicklevel2         = atoi(row[59]);
			e.elemdmgtype         = atoi(row[60]);
			e.elemdmgamt          = atoi(row[61]);
			e.endur               = atoi(row[62]);
			e.factionamt1         = atoi(row[63]);
			e.factionamt2         = atoi(row[64]);
			e.factionamt3         = atoi(row[65]);
			e.factionamt4         = atoi(row[66]);
			e.factionmod1         = atoi(row[67]);
			e.factionmod2         = atoi(row[68]);
			e.factionmod3         = atoi(row[69]);
			e.factionmod4         = atoi(row[70]);
			e.filename            = row[71] ? row[71] : "";
			e.focuseffect         = atoi(row[72]);
			e.fr                  = atoi(row[73]);
			e.fvnodrop            = atoi(row[74]);
			e.haste               = atoi(row[75]);
			e.clicklevel          = atoi(row[76]);
			e.hp                  = atoi(row[77]);
			e.regen               = atoi(row[78]);
			e.icon                = atoi(row[79]);
			e.idfile              = row[80] ? row[80] : "";
			e.itemclass           = atoi(row[81]);
			e.itemtype            = atoi(row[82]);
			e.ldonprice           = atoi(row[83]);
			e.ldontheme           = atoi(row[84]);
			e.ldonsold            = atoi(row[85]);
			e.light               = atoi(row[86]);
			e.lore                = row[87] ? row[87] : "";
			e.loregroup           = atoi(row[88]);
			e.magic               = atoi(row[89]);
			e.mana                = atoi(row[90]);
			e.manaregen           = atoi(row[91]);
			e.enduranceregen      = atoi(row[92]);
			e.material            = atoi(row[93]);
			e.herosforgemodel     = atoi(row[94]);
			e.maxcharges          = atoi(row[95]);
			e.mr                  = atoi(row[96]);
			e.nodrop              = atoi(row[97]);
			e.norent              = atoi(row[98]);
			e.pendingloreflag     = atoi(row[99]);
			e.pr                  = atoi(row[100]);
			e.procrate            = atoi(row[101]);
			e.races               = atoi(row[102]);
			e.range               = atoi(row[103]);
			e.reclevel            = atoi(row[104]);
			e.recskill            = atoi(row[105]);
			e.reqlevel            = atoi(row[106]);
			e.sellrate            = static_cast<float>(atof(row[107]));
			e.shielding           = atoi(row[108]);
			e.size                = atoi(row[109]);
			e.skillmodtype        = atoi(row[110]);
			e.skillmodvalue       = atoi(row[111]);
			e.slots               = atoi(row[112]);
			e.clickeffect         = atoi(row[113]);
			e.spellshield         = atoi(row[114]);
			e.strikethrough       = atoi(row[115]);
			e.stunresist          = atoi(row[116]);
			e.summonedflag        = atoi(row[117]);
			e.tradeskills         = atoi(row[118]);
			e.favor               = atoi(row[119]);
			e.weight              = atoi(row[120]);
			e.UNK012              = atoi(row[121]);
			e.UNK013              = atoi(row[122]);
			e.benefitflag         = atoi(row[123]);
			e.UNK054              = atoi(row[124]);
			e.UNK059              = atoi(row[125]);
			e.booktype            = atoi(row[126]);
			e.recastdelay         = atoi(row[127]);
			e.recasttype          = atoi(row[128]);
			e.guildfavor          = atoi(row[129]);
			e.UNK123              = atoi(row[130]);
			e.UNK124              = atoi(row[131]);
			e.attuneable          = atoi(row[132]);
			e.nopet               = atoi(row[133]);
			e.updated             = strtoll(row[134] ? row[134] : "-1", nullptr, 10);
			e.comment             = row[135] ? row[135] : "";
			e.UNK127              = atoi(row[136]);
			e.pointtype           = atoi(row[137]);
			e.potionbelt          = atoi(row[138]);
			e.potionbeltslots     = atoi(row[139]);
			e.stacksize           = atoi(row[140]);
			e.notransfer          = atoi(row[141]);
			e.stackable           = atoi(row[142]);
			e.UNK134              = row[143] ? row[143] : "";
			e.UNK137              = atoi(row[144]);
			e.proceffect          = atoi(row[145]);
			e.proctype            = atoi(row[146]);
			e.proclevel2          = atoi(row[147]);
			e.proclevel           = atoi(row[148]);
			e.UNK142              = atoi(row[149]);
			e.worneffect          = atoi(row[150]);
			e.worntype            = atoi(row[151]);
			e.wornlevel2          = atoi(row[152]);
			e.wornlevel           = atoi(row[153]);
			e.UNK147              = atoi(row[154]);
			e.focustype           = atoi(row[155]);
			e.focuslevel2         = atoi(row[156]);
			e.focuslevel          = atoi(row[157]);
			e.UNK152              = atoi(row[158]);
			e.scrolleffect        = atoi(row[159]);
			e.scrolltype          = atoi(row[160]);
			e.scrolllevel2        = atoi(row[161]);
			e.scrolllevel         = atoi(row[162]);
			e.UNK157              = atoi(row[163]);
			e.serialized          = strtoll(row[164] ? row[164] : "-1", nullptr, 10);
			e.verified            = strtoll(row[165] ? row[165] : "-1", nullptr, 10);
			e.serialization       = row[166] ? row[166] : "";
			e.source              = row[167] ? row[167] : "";
			e.UNK033              = atoi(row[168]);
			e.lorefile            = row[169] ? row[169] : "";
			e.UNK014              = atoi(row[170]);
			e.svcorruption        = atoi(row[171]);
			e.skillmodmax         = atoi(row[172]);
			e.UNK060              = atoi(row[173]);
			e.augslot1unk2        = atoi(row[174]);
			e.augslot2unk2        = atoi(row[175]);
			e.augslot3unk2        = atoi(row[176]);
			e.augslot4unk2        = atoi(row[177]);
			e.augslot5unk2        = atoi(row[178]);
			e.augslot6unk2        = atoi(row[179]);
			e.UNK120              = atoi(row[180]);
			e.UNK121              = atoi(row[181]);
			e.questitemflag       = atoi(row[182]);
			e.UNK132              = row[183] ? row[183] : "";
			e.clickunk5           = atoi(row[184]);
			e.clickunk6           = row[185] ? row[185] : "";
			e.clickunk7           = atoi(row[186]);
			e.procunk1            = atoi(row[187]);
			e.procunk2            = atoi(row[188]);
			e.procunk3            = atoi(row[189]);
			e.procunk4            = atoi(row[190]);
			e.procunk6            = row[191] ? row[191] : "";
			e.procunk7            = atoi(row[192]);
			e.wornunk1            = atoi(row[193]);
			e.wornunk2            = atoi(row[194]);
			e.wornunk3            = atoi(row[195]);
			e.wornunk4            = atoi(row[196]);
			e.wornunk5            = atoi(row[197]);
			e.wornunk6            = row[198] ? row[198] : "";
			e.wornunk7            = atoi(row[199]);
			e.focusunk1           = atoi(row[200]);
			e.focusunk2           = atoi(row[201]);
			e.focusunk3           = atoi(row[202]);
			e.focusunk4           = atoi(row[203]);
			e.focusunk5           = atoi(row[204]);
			e.focusunk6           = row[205] ? row[205] : "";
			e.focusunk7           = atoi(row[206]);
			e.scrollunk1          = atoi(row[207]);
			e.scrollunk2          = atoi(row[208]);
			e.scrollunk3          = atoi(row[209]);
			e.scrollunk4          = atoi(row[210]);
			e.scrollunk5          = atoi(row[211]);
			e.scrollunk6          = row[212] ? row[212] : "";
			e.scrollunk7          = atoi(row[213]);
			e.UNK193              = atoi(row[214]);
			e.purity              = atoi(row[215]);
			e.evoitem             = atoi(row[216]);
			e.evoid               = atoi(row[217]);
			e.evolvinglevel       = atoi(row[218]);
			e.evomax              = atoi(row[219]);
			e.clickname           = row[220] ? row[220] : "";
			e.procname            = row[221] ? row[221] : "";
			e.wornname            = row[222] ? row[222] : "";
			e.focusname           = row[223] ? row[223] : "";
			e.scrollname          = row[224] ? row[224] : "";
			e.dsmitigation        = atoi(row[225]);
			e.heroic_str          = atoi(row[226]);
			e.heroic_int          = atoi(row[227]);
			e.heroic_wis          = atoi(row[228]);
			e.heroic_agi          = atoi(row[229]);
			e.heroic_dex          = atoi(row[230]);
			e.heroic_sta          = atoi(row[231]);
			e.heroic_cha          = atoi(row[232]);
			e.heroic_pr           = atoi(row[233]);
			e.heroic_dr           = atoi(row[234]);
			e.heroic_fr           = atoi(row[235]);
			e.heroic_cr           = atoi(row[236]);
			e.heroic_mr           = atoi(row[237]);
			e.heroic_svcorrup     = atoi(row[238]);
			e.healamt             = atoi(row[239]);
			e.spelldmg            = atoi(row[240]);
			e.clairvoyance        = atoi(row[241]);
			e.backstabdmg         = atoi(row[242]);
			e.created             = row[243] ? row[243] : "";
			e.elitematerial       = atoi(row[244]);
			e.ldonsellbackrate    = atoi(row[245]);
			e.scriptfileid        = atoi(row[246]);
			e.expendablearrow     = atoi(row[247]);
			e.powersourcecapacity = atoi(row[248]);
			e.bardeffect          = atoi(row[249]);
			e.bardeffecttype      = atoi(row[250]);
			e.bardlevel2          = atoi(row[251]);
			e.bardlevel           = atoi(row[252]);
			e.bardunk1            = atoi(row[253]);
			e.bardunk2            = atoi(row[254]);
			e.bardunk3            = atoi(row[255]);
			e.bardunk4            = atoi(row[256]);
			e.bardunk5            = atoi(row[257]);
			e.bardname            = row[258] ? row[258] : "";
			e.bardunk7            = atoi(row[259]);
			e.UNK214              = atoi(row[260]);
			e.subtype             = atoi(row[261]);
			e.UNK220              = atoi(row[262]);
			e.UNK221              = atoi(row[263]);
			e.heirloom            = atoi(row[264]);
			e.UNK223              = atoi(row[265]);
			e.UNK224              = atoi(row[266]);
			e.UNK225              = atoi(row[267]);
			e.UNK226              = atoi(row[268]);
			e.UNK227              = atoi(row[269]);
			e.UNK228              = atoi(row[270]);
			e.UNK229              = atoi(row[271]);
			e.UNK230              = atoi(row[272]);
			e.UNK231              = atoi(row[273]);
			e.UNK232              = atoi(row[274]);
			e.UNK233              = atoi(row[275]);
			e.UNK234              = atoi(row[276]);
			e.placeable           = atoi(row[277]);
			e.UNK236              = atoi(row[278]);
			e.UNK237              = atoi(row[279]);
			e.UNK238              = atoi(row[280]);
			e.UNK239              = atoi(row[281]);
			e.UNK240              = atoi(row[282]);
			e.UNK241              = atoi(row[283]);
			e.epicitem            = atoi(row[284]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Items> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Items> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Items e{};

			e.id                  = atoi(row[0]);
			e.minstatus           = atoi(row[1]);
			e.Name                = row[2] ? row[2] : "";
			e.aagi                = atoi(row[3]);
			e.ac                  = atoi(row[4]);
			e.accuracy            = atoi(row[5]);
			e.acha                = atoi(row[6]);
			e.adex                = atoi(row[7]);
			e.aint                = atoi(row[8]);
			e.artifactflag        = atoi(row[9]);
			e.asta                = atoi(row[10]);
			e.astr                = atoi(row[11]);
			e.attack              = atoi(row[12]);
			e.augrestrict         = atoi(row[13]);
			e.augslot1type        = atoi(row[14]);
			e.augslot1visible     = atoi(row[15]);
			e.augslot2type        = atoi(row[16]);
			e.augslot2visible     = atoi(row[17]);
			e.augslot3type        = atoi(row[18]);
			e.augslot3visible     = atoi(row[19]);
			e.augslot4type        = atoi(row[20]);
			e.augslot4visible     = atoi(row[21]);
			e.augslot5type        = atoi(row[22]);
			e.augslot5visible     = atoi(row[23]);
			e.augslot6type        = atoi(row[24]);
			e.augslot6visible     = atoi(row[25]);
			e.augtype             = atoi(row[26]);
			e.avoidance           = atoi(row[27]);
			e.awis                = atoi(row[28]);
			e.bagsize             = atoi(row[29]);
			e.bagslots            = atoi(row[30]);
			e.bagtype             = atoi(row[31]);
			e.bagwr               = atoi(row[32]);
			e.banedmgamt          = atoi(row[33]);
			e.banedmgraceamt      = atoi(row[34]);
			e.banedmgbody         = atoi(row[35]);
			e.banedmgrace         = atoi(row[36]);
			e.bardtype            = atoi(row[37]);
			e.bardvalue           = atoi(row[38]);
			e.book                = atoi(row[39]);
			e.casttime            = atoi(row[40]);
			e.casttime_           = atoi(row[41]);
			e.charmfile           = row[42] ? row[42] : "";
			e.charmfileid         = row[43] ? row[43] : "";
			e.classes             = atoi(row[44]);
			e.color               = atoi(row[45]);
			e.combateffects       = row[46] ? row[46] : "";
			e.extradmgskill       = atoi(row[47]);
			e.extradmgamt         = atoi(row[48]);
			e.price               = atoi(row[49]);
			e.cr                  = atoi(row[50]);
			e.damage              = atoi(row[51]);
			e.damageshield        = atoi(row[52]);
			e.deity               = atoi(row[53]);
			e.delay               = atoi(row[54]);
			e.augdistiller        = atoi(row[55]);
			e.dotshielding        = atoi(row[56]);
			e.dr                  = atoi(row[57]);
			e.clicktype           = atoi(row[58]);
			e.clicklevel2         = atoi(row[59]);
			e.elemdmgtype         = atoi(row[60]);
			e.elemdmgamt          = atoi(row[61]);
			e.endur               = atoi(row[62]);
			e.factionamt1         = atoi(row[63]);
			e.factionamt2         = atoi(row[64]);
			e.factionamt3         = atoi(row[65]);
			e.factionamt4         = atoi(row[66]);
			e.factionmod1         = atoi(row[67]);
			e.factionmod2         = atoi(row[68]);
			e.factionmod3         = atoi(row[69]);
			e.factionmod4         = atoi(row[70]);
			e.filename            = row[71] ? row[71] : "";
			e.focuseffect         = atoi(row[72]);
			e.fr                  = atoi(row[73]);
			e.fvnodrop            = atoi(row[74]);
			e.haste               = atoi(row[75]);
			e.clicklevel          = atoi(row[76]);
			e.hp                  = atoi(row[77]);
			e.regen               = atoi(row[78]);
			e.icon                = atoi(row[79]);
			e.idfile              = row[80] ? row[80] : "";
			e.itemclass           = atoi(row[81]);
			e.itemtype            = atoi(row[82]);
			e.ldonprice           = atoi(row[83]);
			e.ldontheme           = atoi(row[84]);
			e.ldonsold            = atoi(row[85]);
			e.light               = atoi(row[86]);
			e.lore                = row[87] ? row[87] : "";
			e.loregroup           = atoi(row[88]);
			e.magic               = atoi(row[89]);
			e.mana                = atoi(row[90]);
			e.manaregen           = atoi(row[91]);
			e.enduranceregen      = atoi(row[92]);
			e.material            = atoi(row[93]);
			e.herosforgemodel     = atoi(row[94]);
			e.maxcharges          = atoi(row[95]);
			e.mr                  = atoi(row[96]);
			e.nodrop              = atoi(row[97]);
			e.norent              = atoi(row[98]);
			e.pendingloreflag     = atoi(row[99]);
			e.pr                  = atoi(row[100]);
			e.procrate            = atoi(row[101]);
			e.races               = atoi(row[102]);
			e.range               = atoi(row[103]);
			e.reclevel            = atoi(row[104]);
			e.recskill            = atoi(row[105]);
			e.reqlevel            = atoi(row[106]);
			e.sellrate            = static_cast<float>(atof(row[107]));
			e.shielding           = atoi(row[108]);
			e.size                = atoi(row[109]);
			e.skillmodtype        = atoi(row[110]);
			e.skillmodvalue       = atoi(row[111]);
			e.slots               = atoi(row[112]);
			e.clickeffect         = atoi(row[113]);
			e.spellshield         = atoi(row[114]);
			e.strikethrough       = atoi(row[115]);
			e.stunresist          = atoi(row[116]);
			e.summonedflag        = atoi(row[117]);
			e.tradeskills         = atoi(row[118]);
			e.favor               = atoi(row[119]);
			e.weight              = atoi(row[120]);
			e.UNK012              = atoi(row[121]);
			e.UNK013              = atoi(row[122]);
			e.benefitflag         = atoi(row[123]);
			e.UNK054              = atoi(row[124]);
			e.UNK059              = atoi(row[125]);
			e.booktype            = atoi(row[126]);
			e.recastdelay         = atoi(row[127]);
			e.recasttype          = atoi(row[128]);
			e.guildfavor          = atoi(row[129]);
			e.UNK123              = atoi(row[130]);
			e.UNK124              = atoi(row[131]);
			e.attuneable          = atoi(row[132]);
			e.nopet               = atoi(row[133]);
			e.updated             = strtoll(row[134] ? row[134] : "-1", nullptr, 10);
			e.comment             = row[135] ? row[135] : "";
			e.UNK127              = atoi(row[136]);
			e.pointtype           = atoi(row[137]);
			e.potionbelt          = atoi(row[138]);
			e.potionbeltslots     = atoi(row[139]);
			e.stacksize           = atoi(row[140]);
			e.notransfer          = atoi(row[141]);
			e.stackable           = atoi(row[142]);
			e.UNK134              = row[143] ? row[143] : "";
			e.UNK137              = atoi(row[144]);
			e.proceffect          = atoi(row[145]);
			e.proctype            = atoi(row[146]);
			e.proclevel2          = atoi(row[147]);
			e.proclevel           = atoi(row[148]);
			e.UNK142              = atoi(row[149]);
			e.worneffect          = atoi(row[150]);
			e.worntype            = atoi(row[151]);
			e.wornlevel2          = atoi(row[152]);
			e.wornlevel           = atoi(row[153]);
			e.UNK147              = atoi(row[154]);
			e.focustype           = atoi(row[155]);
			e.focuslevel2         = atoi(row[156]);
			e.focuslevel          = atoi(row[157]);
			e.UNK152              = atoi(row[158]);
			e.scrolleffect        = atoi(row[159]);
			e.scrolltype          = atoi(row[160]);
			e.scrolllevel2        = atoi(row[161]);
			e.scrolllevel         = atoi(row[162]);
			e.UNK157              = atoi(row[163]);
			e.serialized          = strtoll(row[164] ? row[164] : "-1", nullptr, 10);
			e.verified            = strtoll(row[165] ? row[165] : "-1", nullptr, 10);
			e.serialization       = row[166] ? row[166] : "";
			e.source              = row[167] ? row[167] : "";
			e.UNK033              = atoi(row[168]);
			e.lorefile            = row[169] ? row[169] : "";
			e.UNK014              = atoi(row[170]);
			e.svcorruption        = atoi(row[171]);
			e.skillmodmax         = atoi(row[172]);
			e.UNK060              = atoi(row[173]);
			e.augslot1unk2        = atoi(row[174]);
			e.augslot2unk2        = atoi(row[175]);
			e.augslot3unk2        = atoi(row[176]);
			e.augslot4unk2        = atoi(row[177]);
			e.augslot5unk2        = atoi(row[178]);
			e.augslot6unk2        = atoi(row[179]);
			e.UNK120              = atoi(row[180]);
			e.UNK121              = atoi(row[181]);
			e.questitemflag       = atoi(row[182]);
			e.UNK132              = row[183] ? row[183] : "";
			e.clickunk5           = atoi(row[184]);
			e.clickunk6           = row[185] ? row[185] : "";
			e.clickunk7           = atoi(row[186]);
			e.procunk1            = atoi(row[187]);
			e.procunk2            = atoi(row[188]);
			e.procunk3            = atoi(row[189]);
			e.procunk4            = atoi(row[190]);
			e.procunk6            = row[191] ? row[191] : "";
			e.procunk7            = atoi(row[192]);
			e.wornunk1            = atoi(row[193]);
			e.wornunk2            = atoi(row[194]);
			e.wornunk3            = atoi(row[195]);
			e.wornunk4            = atoi(row[196]);
			e.wornunk5            = atoi(row[197]);
			e.wornunk6            = row[198] ? row[198] : "";
			e.wornunk7            = atoi(row[199]);
			e.focusunk1           = atoi(row[200]);
			e.focusunk2           = atoi(row[201]);
			e.focusunk3           = atoi(row[202]);
			e.focusunk4           = atoi(row[203]);
			e.focusunk5           = atoi(row[204]);
			e.focusunk6           = row[205] ? row[205] : "";
			e.focusunk7           = atoi(row[206]);
			e.scrollunk1          = atoi(row[207]);
			e.scrollunk2          = atoi(row[208]);
			e.scrollunk3          = atoi(row[209]);
			e.scrollunk4          = atoi(row[210]);
			e.scrollunk5          = atoi(row[211]);
			e.scrollunk6          = row[212] ? row[212] : "";
			e.scrollunk7          = atoi(row[213]);
			e.UNK193              = atoi(row[214]);
			e.purity              = atoi(row[215]);
			e.evoitem             = atoi(row[216]);
			e.evoid               = atoi(row[217]);
			e.evolvinglevel       = atoi(row[218]);
			e.evomax              = atoi(row[219]);
			e.clickname           = row[220] ? row[220] : "";
			e.procname            = row[221] ? row[221] : "";
			e.wornname            = row[222] ? row[222] : "";
			e.focusname           = row[223] ? row[223] : "";
			e.scrollname          = row[224] ? row[224] : "";
			e.dsmitigation        = atoi(row[225]);
			e.heroic_str          = atoi(row[226]);
			e.heroic_int          = atoi(row[227]);
			e.heroic_wis          = atoi(row[228]);
			e.heroic_agi          = atoi(row[229]);
			e.heroic_dex          = atoi(row[230]);
			e.heroic_sta          = atoi(row[231]);
			e.heroic_cha          = atoi(row[232]);
			e.heroic_pr           = atoi(row[233]);
			e.heroic_dr           = atoi(row[234]);
			e.heroic_fr           = atoi(row[235]);
			e.heroic_cr           = atoi(row[236]);
			e.heroic_mr           = atoi(row[237]);
			e.heroic_svcorrup     = atoi(row[238]);
			e.healamt             = atoi(row[239]);
			e.spelldmg            = atoi(row[240]);
			e.clairvoyance        = atoi(row[241]);
			e.backstabdmg         = atoi(row[242]);
			e.created             = row[243] ? row[243] : "";
			e.elitematerial       = atoi(row[244]);
			e.ldonsellbackrate    = atoi(row[245]);
			e.scriptfileid        = atoi(row[246]);
			e.expendablearrow     = atoi(row[247]);
			e.powersourcecapacity = atoi(row[248]);
			e.bardeffect          = atoi(row[249]);
			e.bardeffecttype      = atoi(row[250]);
			e.bardlevel2          = atoi(row[251]);
			e.bardlevel           = atoi(row[252]);
			e.bardunk1            = atoi(row[253]);
			e.bardunk2            = atoi(row[254]);
			e.bardunk3            = atoi(row[255]);
			e.bardunk4            = atoi(row[256]);
			e.bardunk5            = atoi(row[257]);
			e.bardname            = row[258] ? row[258] : "";
			e.bardunk7            = atoi(row[259]);
			e.UNK214              = atoi(row[260]);
			e.subtype             = atoi(row[261]);
			e.UNK220              = atoi(row[262]);
			e.UNK221              = atoi(row[263]);
			e.heirloom            = atoi(row[264]);
			e.UNK223              = atoi(row[265]);
			e.UNK224              = atoi(row[266]);
			e.UNK225              = atoi(row[267]);
			e.UNK226              = atoi(row[268]);
			e.UNK227              = atoi(row[269]);
			e.UNK228              = atoi(row[270]);
			e.UNK229              = atoi(row[271]);
			e.UNK230              = atoi(row[272]);
			e.UNK231              = atoi(row[273]);
			e.UNK232              = atoi(row[274]);
			e.UNK233              = atoi(row[275]);
			e.UNK234              = atoi(row[276]);
			e.placeable           = atoi(row[277]);
			e.UNK236              = atoi(row[278]);
			e.UNK237              = atoi(row[279]);
			e.UNK238              = atoi(row[280]);
			e.UNK239              = atoi(row[281]);
			e.UNK240              = atoi(row[282]);
			e.UNK241              = atoi(row[283]);
			e.epicitem            = atoi(row[284]);

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

#endif //EQEMU_BASE_ITEMS_REPOSITORY_H
