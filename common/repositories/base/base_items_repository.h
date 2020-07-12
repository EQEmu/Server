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

#ifndef EQEMU_BASE_ITEMS_REPOSITORY_H
#define EQEMU_BASE_ITEMS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

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
		std::string updated;
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
		std::string serialized;
		std::string verified;
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
		return std::string("items");
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

	static Items NewEntity()
	{
		Items entry{};

		entry.id                  = 0;
		entry.minstatus           = 0;
		entry.Name                = "";
		entry.aagi                = 0;
		entry.ac                  = 0;
		entry.accuracy            = 0;
		entry.acha                = 0;
		entry.adex                = 0;
		entry.aint                = 0;
		entry.artifactflag        = 0;
		entry.asta                = 0;
		entry.astr                = 0;
		entry.attack              = 0;
		entry.augrestrict         = 0;
		entry.augslot1type        = 0;
		entry.augslot1visible     = 0;
		entry.augslot2type        = 0;
		entry.augslot2visible     = 0;
		entry.augslot3type        = 0;
		entry.augslot3visible     = 0;
		entry.augslot4type        = 0;
		entry.augslot4visible     = 0;
		entry.augslot5type        = 0;
		entry.augslot5visible     = 0;
		entry.augslot6type        = 0;
		entry.augslot6visible     = 0;
		entry.augtype             = 0;
		entry.avoidance           = 0;
		entry.awis                = 0;
		entry.bagsize             = 0;
		entry.bagslots            = 0;
		entry.bagtype             = 0;
		entry.bagwr               = 0;
		entry.banedmgamt          = 0;
		entry.banedmgraceamt      = 0;
		entry.banedmgbody         = 0;
		entry.banedmgrace         = 0;
		entry.bardtype            = 0;
		entry.bardvalue           = 0;
		entry.book                = 0;
		entry.casttime            = 0;
		entry.casttime_           = 0;
		entry.charmfile           = "";
		entry.charmfileid         = "";
		entry.classes             = 0;
		entry.color               = 0;
		entry.combateffects       = "";
		entry.extradmgskill       = 0;
		entry.extradmgamt         = 0;
		entry.price               = 0;
		entry.cr                  = 0;
		entry.damage              = 0;
		entry.damageshield        = 0;
		entry.deity               = 0;
		entry.delay               = 0;
		entry.augdistiller        = 0;
		entry.dotshielding        = 0;
		entry.dr                  = 0;
		entry.clicktype           = 0;
		entry.clicklevel2         = 0;
		entry.elemdmgtype         = 0;
		entry.elemdmgamt          = 0;
		entry.endur               = 0;
		entry.factionamt1         = 0;
		entry.factionamt2         = 0;
		entry.factionamt3         = 0;
		entry.factionamt4         = 0;
		entry.factionmod1         = 0;
		entry.factionmod2         = 0;
		entry.factionmod3         = 0;
		entry.factionmod4         = 0;
		entry.filename            = "";
		entry.focuseffect         = 0;
		entry.fr                  = 0;
		entry.fvnodrop            = 0;
		entry.haste               = 0;
		entry.clicklevel          = 0;
		entry.hp                  = 0;
		entry.regen               = 0;
		entry.icon                = 0;
		entry.idfile              = "";
		entry.itemclass           = 0;
		entry.itemtype            = 0;
		entry.ldonprice           = 0;
		entry.ldontheme           = 0;
		entry.ldonsold            = 0;
		entry.light               = 0;
		entry.lore                = "";
		entry.loregroup           = 0;
		entry.magic               = 0;
		entry.mana                = 0;
		entry.manaregen           = 0;
		entry.enduranceregen      = 0;
		entry.material            = 0;
		entry.herosforgemodel     = 0;
		entry.maxcharges          = 0;
		entry.mr                  = 0;
		entry.nodrop              = 0;
		entry.norent              = 0;
		entry.pendingloreflag     = 0;
		entry.pr                  = 0;
		entry.procrate            = 0;
		entry.races               = 0;
		entry.range               = 0;
		entry.reclevel            = 0;
		entry.recskill            = 0;
		entry.reqlevel            = 0;
		entry.sellrate            = 0;
		entry.shielding           = 0;
		entry.size                = 0;
		entry.skillmodtype        = 0;
		entry.skillmodvalue       = 0;
		entry.slots               = 0;
		entry.clickeffect         = 0;
		entry.spellshield         = 0;
		entry.strikethrough       = 0;
		entry.stunresist          = 0;
		entry.summonedflag        = 0;
		entry.tradeskills         = 0;
		entry.favor               = 0;
		entry.weight              = 0;
		entry.UNK012              = 0;
		entry.UNK013              = 0;
		entry.benefitflag         = 0;
		entry.UNK054              = 0;
		entry.UNK059              = 0;
		entry.booktype            = 0;
		entry.recastdelay         = 0;
		entry.recasttype          = 0;
		entry.guildfavor          = 0;
		entry.UNK123              = 0;
		entry.UNK124              = 0;
		entry.attuneable          = 0;
		entry.nopet               = 0;
		entry.updated             = "0000-00-00 00:00:00";
		entry.comment             = "";
		entry.UNK127              = 0;
		entry.pointtype           = 0;
		entry.potionbelt          = 0;
		entry.potionbeltslots     = 0;
		entry.stacksize           = 0;
		entry.notransfer          = 0;
		entry.stackable           = 0;
		entry.UNK134              = "";
		entry.UNK137              = 0;
		entry.proceffect          = 0;
		entry.proctype            = 0;
		entry.proclevel2          = 0;
		entry.proclevel           = 0;
		entry.UNK142              = 0;
		entry.worneffect          = 0;
		entry.worntype            = 0;
		entry.wornlevel2          = 0;
		entry.wornlevel           = 0;
		entry.UNK147              = 0;
		entry.focustype           = 0;
		entry.focuslevel2         = 0;
		entry.focuslevel          = 0;
		entry.UNK152              = 0;
		entry.scrolleffect        = 0;
		entry.scrolltype          = 0;
		entry.scrolllevel2        = 0;
		entry.scrolllevel         = 0;
		entry.UNK157              = 0;
		entry.serialized          = 0;
		entry.verified            = 0;
		entry.serialization       = "";
		entry.source              = "";
		entry.UNK033              = 0;
		entry.lorefile            = "";
		entry.UNK014              = 0;
		entry.svcorruption        = 0;
		entry.skillmodmax         = 0;
		entry.UNK060              = 0;
		entry.augslot1unk2        = 0;
		entry.augslot2unk2        = 0;
		entry.augslot3unk2        = 0;
		entry.augslot4unk2        = 0;
		entry.augslot5unk2        = 0;
		entry.augslot6unk2        = 0;
		entry.UNK120              = 0;
		entry.UNK121              = 0;
		entry.questitemflag       = 0;
		entry.UNK132              = "";
		entry.clickunk5           = 0;
		entry.clickunk6           = "";
		entry.clickunk7           = 0;
		entry.procunk1            = 0;
		entry.procunk2            = 0;
		entry.procunk3            = 0;
		entry.procunk4            = 0;
		entry.procunk6            = "";
		entry.procunk7            = 0;
		entry.wornunk1            = 0;
		entry.wornunk2            = 0;
		entry.wornunk3            = 0;
		entry.wornunk4            = 0;
		entry.wornunk5            = 0;
		entry.wornunk6            = "";
		entry.wornunk7            = 0;
		entry.focusunk1           = 0;
		entry.focusunk2           = 0;
		entry.focusunk3           = 0;
		entry.focusunk4           = 0;
		entry.focusunk5           = 0;
		entry.focusunk6           = "";
		entry.focusunk7           = 0;
		entry.scrollunk1          = 0;
		entry.scrollunk2          = 0;
		entry.scrollunk3          = 0;
		entry.scrollunk4          = 0;
		entry.scrollunk5          = 0;
		entry.scrollunk6          = "";
		entry.scrollunk7          = 0;
		entry.UNK193              = 0;
		entry.purity              = 0;
		entry.evoitem             = 0;
		entry.evoid               = 0;
		entry.evolvinglevel       = 0;
		entry.evomax              = 0;
		entry.clickname           = "";
		entry.procname            = "";
		entry.wornname            = "";
		entry.focusname           = "";
		entry.scrollname          = "";
		entry.dsmitigation        = 0;
		entry.heroic_str          = 0;
		entry.heroic_int          = 0;
		entry.heroic_wis          = 0;
		entry.heroic_agi          = 0;
		entry.heroic_dex          = 0;
		entry.heroic_sta          = 0;
		entry.heroic_cha          = 0;
		entry.heroic_pr           = 0;
		entry.heroic_dr           = 0;
		entry.heroic_fr           = 0;
		entry.heroic_cr           = 0;
		entry.heroic_mr           = 0;
		entry.heroic_svcorrup     = 0;
		entry.healamt             = 0;
		entry.spelldmg            = 0;
		entry.clairvoyance        = 0;
		entry.backstabdmg         = 0;
		entry.created             = "";
		entry.elitematerial       = 0;
		entry.ldonsellbackrate    = 0;
		entry.scriptfileid        = 0;
		entry.expendablearrow     = 0;
		entry.powersourcecapacity = 0;
		entry.bardeffect          = 0;
		entry.bardeffecttype      = 0;
		entry.bardlevel2          = 0;
		entry.bardlevel           = 0;
		entry.bardunk1            = 0;
		entry.bardunk2            = 0;
		entry.bardunk3            = 0;
		entry.bardunk4            = 0;
		entry.bardunk5            = 0;
		entry.bardname            = "";
		entry.bardunk7            = 0;
		entry.UNK214              = 0;
		entry.subtype             = 0;
		entry.UNK220              = 0;
		entry.UNK221              = 0;
		entry.heirloom            = 0;
		entry.UNK223              = 0;
		entry.UNK224              = 0;
		entry.UNK225              = 0;
		entry.UNK226              = 0;
		entry.UNK227              = 0;
		entry.UNK228              = 0;
		entry.UNK229              = 0;
		entry.UNK230              = 0;
		entry.UNK231              = 0;
		entry.UNK232              = 0;
		entry.UNK233              = 0;
		entry.UNK234              = 0;
		entry.placeable           = 0;
		entry.UNK236              = 0;
		entry.UNK237              = 0;
		entry.UNK238              = 0;
		entry.UNK239              = 0;
		entry.UNK240              = 0;
		entry.UNK241              = 0;
		entry.epicitem            = 0;

		return entry;
	}

	static Items GetItemsEntry(
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
		int items_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				items_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Items entry{};

			entry.id                  = atoi(row[0]);
			entry.minstatus           = atoi(row[1]);
			entry.Name                = row[2] ? row[2] : "";
			entry.aagi                = atoi(row[3]);
			entry.ac                  = atoi(row[4]);
			entry.accuracy            = atoi(row[5]);
			entry.acha                = atoi(row[6]);
			entry.adex                = atoi(row[7]);
			entry.aint                = atoi(row[8]);
			entry.artifactflag        = atoi(row[9]);
			entry.asta                = atoi(row[10]);
			entry.astr                = atoi(row[11]);
			entry.attack              = atoi(row[12]);
			entry.augrestrict         = atoi(row[13]);
			entry.augslot1type        = atoi(row[14]);
			entry.augslot1visible     = atoi(row[15]);
			entry.augslot2type        = atoi(row[16]);
			entry.augslot2visible     = atoi(row[17]);
			entry.augslot3type        = atoi(row[18]);
			entry.augslot3visible     = atoi(row[19]);
			entry.augslot4type        = atoi(row[20]);
			entry.augslot4visible     = atoi(row[21]);
			entry.augslot5type        = atoi(row[22]);
			entry.augslot5visible     = atoi(row[23]);
			entry.augslot6type        = atoi(row[24]);
			entry.augslot6visible     = atoi(row[25]);
			entry.augtype             = atoi(row[26]);
			entry.avoidance           = atoi(row[27]);
			entry.awis                = atoi(row[28]);
			entry.bagsize             = atoi(row[29]);
			entry.bagslots            = atoi(row[30]);
			entry.bagtype             = atoi(row[31]);
			entry.bagwr               = atoi(row[32]);
			entry.banedmgamt          = atoi(row[33]);
			entry.banedmgraceamt      = atoi(row[34]);
			entry.banedmgbody         = atoi(row[35]);
			entry.banedmgrace         = atoi(row[36]);
			entry.bardtype            = atoi(row[37]);
			entry.bardvalue           = atoi(row[38]);
			entry.book                = atoi(row[39]);
			entry.casttime            = atoi(row[40]);
			entry.casttime_           = atoi(row[41]);
			entry.charmfile           = row[42] ? row[42] : "";
			entry.charmfileid         = row[43] ? row[43] : "";
			entry.classes             = atoi(row[44]);
			entry.color               = atoi(row[45]);
			entry.combateffects       = row[46] ? row[46] : "";
			entry.extradmgskill       = atoi(row[47]);
			entry.extradmgamt         = atoi(row[48]);
			entry.price               = atoi(row[49]);
			entry.cr                  = atoi(row[50]);
			entry.damage              = atoi(row[51]);
			entry.damageshield        = atoi(row[52]);
			entry.deity               = atoi(row[53]);
			entry.delay               = atoi(row[54]);
			entry.augdistiller        = atoi(row[55]);
			entry.dotshielding        = atoi(row[56]);
			entry.dr                  = atoi(row[57]);
			entry.clicktype           = atoi(row[58]);
			entry.clicklevel2         = atoi(row[59]);
			entry.elemdmgtype         = atoi(row[60]);
			entry.elemdmgamt          = atoi(row[61]);
			entry.endur               = atoi(row[62]);
			entry.factionamt1         = atoi(row[63]);
			entry.factionamt2         = atoi(row[64]);
			entry.factionamt3         = atoi(row[65]);
			entry.factionamt4         = atoi(row[66]);
			entry.factionmod1         = atoi(row[67]);
			entry.factionmod2         = atoi(row[68]);
			entry.factionmod3         = atoi(row[69]);
			entry.factionmod4         = atoi(row[70]);
			entry.filename            = row[71] ? row[71] : "";
			entry.focuseffect         = atoi(row[72]);
			entry.fr                  = atoi(row[73]);
			entry.fvnodrop            = atoi(row[74]);
			entry.haste               = atoi(row[75]);
			entry.clicklevel          = atoi(row[76]);
			entry.hp                  = atoi(row[77]);
			entry.regen               = atoi(row[78]);
			entry.icon                = atoi(row[79]);
			entry.idfile              = row[80] ? row[80] : "";
			entry.itemclass           = atoi(row[81]);
			entry.itemtype            = atoi(row[82]);
			entry.ldonprice           = atoi(row[83]);
			entry.ldontheme           = atoi(row[84]);
			entry.ldonsold            = atoi(row[85]);
			entry.light               = atoi(row[86]);
			entry.lore                = row[87] ? row[87] : "";
			entry.loregroup           = atoi(row[88]);
			entry.magic               = atoi(row[89]);
			entry.mana                = atoi(row[90]);
			entry.manaregen           = atoi(row[91]);
			entry.enduranceregen      = atoi(row[92]);
			entry.material            = atoi(row[93]);
			entry.herosforgemodel     = atoi(row[94]);
			entry.maxcharges          = atoi(row[95]);
			entry.mr                  = atoi(row[96]);
			entry.nodrop              = atoi(row[97]);
			entry.norent              = atoi(row[98]);
			entry.pendingloreflag     = atoi(row[99]);
			entry.pr                  = atoi(row[100]);
			entry.procrate            = atoi(row[101]);
			entry.races               = atoi(row[102]);
			entry.range               = atoi(row[103]);
			entry.reclevel            = atoi(row[104]);
			entry.recskill            = atoi(row[105]);
			entry.reqlevel            = atoi(row[106]);
			entry.sellrate            = static_cast<float>(atof(row[107]));
			entry.shielding           = atoi(row[108]);
			entry.size                = atoi(row[109]);
			entry.skillmodtype        = atoi(row[110]);
			entry.skillmodvalue       = atoi(row[111]);
			entry.slots               = atoi(row[112]);
			entry.clickeffect         = atoi(row[113]);
			entry.spellshield         = atoi(row[114]);
			entry.strikethrough       = atoi(row[115]);
			entry.stunresist          = atoi(row[116]);
			entry.summonedflag        = atoi(row[117]);
			entry.tradeskills         = atoi(row[118]);
			entry.favor               = atoi(row[119]);
			entry.weight              = atoi(row[120]);
			entry.UNK012              = atoi(row[121]);
			entry.UNK013              = atoi(row[122]);
			entry.benefitflag         = atoi(row[123]);
			entry.UNK054              = atoi(row[124]);
			entry.UNK059              = atoi(row[125]);
			entry.booktype            = atoi(row[126]);
			entry.recastdelay         = atoi(row[127]);
			entry.recasttype          = atoi(row[128]);
			entry.guildfavor          = atoi(row[129]);
			entry.UNK123              = atoi(row[130]);
			entry.UNK124              = atoi(row[131]);
			entry.attuneable          = atoi(row[132]);
			entry.nopet               = atoi(row[133]);
			entry.updated             = row[134] ? row[134] : "";
			entry.comment             = row[135] ? row[135] : "";
			entry.UNK127              = atoi(row[136]);
			entry.pointtype           = atoi(row[137]);
			entry.potionbelt          = atoi(row[138]);
			entry.potionbeltslots     = atoi(row[139]);
			entry.stacksize           = atoi(row[140]);
			entry.notransfer          = atoi(row[141]);
			entry.stackable           = atoi(row[142]);
			entry.UNK134              = row[143] ? row[143] : "";
			entry.UNK137              = atoi(row[144]);
			entry.proceffect          = atoi(row[145]);
			entry.proctype            = atoi(row[146]);
			entry.proclevel2          = atoi(row[147]);
			entry.proclevel           = atoi(row[148]);
			entry.UNK142              = atoi(row[149]);
			entry.worneffect          = atoi(row[150]);
			entry.worntype            = atoi(row[151]);
			entry.wornlevel2          = atoi(row[152]);
			entry.wornlevel           = atoi(row[153]);
			entry.UNK147              = atoi(row[154]);
			entry.focustype           = atoi(row[155]);
			entry.focuslevel2         = atoi(row[156]);
			entry.focuslevel          = atoi(row[157]);
			entry.UNK152              = atoi(row[158]);
			entry.scrolleffect        = atoi(row[159]);
			entry.scrolltype          = atoi(row[160]);
			entry.scrolllevel2        = atoi(row[161]);
			entry.scrolllevel         = atoi(row[162]);
			entry.UNK157              = atoi(row[163]);
			entry.serialized          = row[164] ? row[164] : "";
			entry.verified            = row[165] ? row[165] : "";
			entry.serialization       = row[166] ? row[166] : "";
			entry.source              = row[167] ? row[167] : "";
			entry.UNK033              = atoi(row[168]);
			entry.lorefile            = row[169] ? row[169] : "";
			entry.UNK014              = atoi(row[170]);
			entry.svcorruption        = atoi(row[171]);
			entry.skillmodmax         = atoi(row[172]);
			entry.UNK060              = atoi(row[173]);
			entry.augslot1unk2        = atoi(row[174]);
			entry.augslot2unk2        = atoi(row[175]);
			entry.augslot3unk2        = atoi(row[176]);
			entry.augslot4unk2        = atoi(row[177]);
			entry.augslot5unk2        = atoi(row[178]);
			entry.augslot6unk2        = atoi(row[179]);
			entry.UNK120              = atoi(row[180]);
			entry.UNK121              = atoi(row[181]);
			entry.questitemflag       = atoi(row[182]);
			entry.UNK132              = row[183] ? row[183] : "";
			entry.clickunk5           = atoi(row[184]);
			entry.clickunk6           = row[185] ? row[185] : "";
			entry.clickunk7           = atoi(row[186]);
			entry.procunk1            = atoi(row[187]);
			entry.procunk2            = atoi(row[188]);
			entry.procunk3            = atoi(row[189]);
			entry.procunk4            = atoi(row[190]);
			entry.procunk6            = row[191] ? row[191] : "";
			entry.procunk7            = atoi(row[192]);
			entry.wornunk1            = atoi(row[193]);
			entry.wornunk2            = atoi(row[194]);
			entry.wornunk3            = atoi(row[195]);
			entry.wornunk4            = atoi(row[196]);
			entry.wornunk5            = atoi(row[197]);
			entry.wornunk6            = row[198] ? row[198] : "";
			entry.wornunk7            = atoi(row[199]);
			entry.focusunk1           = atoi(row[200]);
			entry.focusunk2           = atoi(row[201]);
			entry.focusunk3           = atoi(row[202]);
			entry.focusunk4           = atoi(row[203]);
			entry.focusunk5           = atoi(row[204]);
			entry.focusunk6           = row[205] ? row[205] : "";
			entry.focusunk7           = atoi(row[206]);
			entry.scrollunk1          = atoi(row[207]);
			entry.scrollunk2          = atoi(row[208]);
			entry.scrollunk3          = atoi(row[209]);
			entry.scrollunk4          = atoi(row[210]);
			entry.scrollunk5          = atoi(row[211]);
			entry.scrollunk6          = row[212] ? row[212] : "";
			entry.scrollunk7          = atoi(row[213]);
			entry.UNK193              = atoi(row[214]);
			entry.purity              = atoi(row[215]);
			entry.evoitem             = atoi(row[216]);
			entry.evoid               = atoi(row[217]);
			entry.evolvinglevel       = atoi(row[218]);
			entry.evomax              = atoi(row[219]);
			entry.clickname           = row[220] ? row[220] : "";
			entry.procname            = row[221] ? row[221] : "";
			entry.wornname            = row[222] ? row[222] : "";
			entry.focusname           = row[223] ? row[223] : "";
			entry.scrollname          = row[224] ? row[224] : "";
			entry.dsmitigation        = atoi(row[225]);
			entry.heroic_str          = atoi(row[226]);
			entry.heroic_int          = atoi(row[227]);
			entry.heroic_wis          = atoi(row[228]);
			entry.heroic_agi          = atoi(row[229]);
			entry.heroic_dex          = atoi(row[230]);
			entry.heroic_sta          = atoi(row[231]);
			entry.heroic_cha          = atoi(row[232]);
			entry.heroic_pr           = atoi(row[233]);
			entry.heroic_dr           = atoi(row[234]);
			entry.heroic_fr           = atoi(row[235]);
			entry.heroic_cr           = atoi(row[236]);
			entry.heroic_mr           = atoi(row[237]);
			entry.heroic_svcorrup     = atoi(row[238]);
			entry.healamt             = atoi(row[239]);
			entry.spelldmg            = atoi(row[240]);
			entry.clairvoyance        = atoi(row[241]);
			entry.backstabdmg         = atoi(row[242]);
			entry.created             = row[243] ? row[243] : "";
			entry.elitematerial       = atoi(row[244]);
			entry.ldonsellbackrate    = atoi(row[245]);
			entry.scriptfileid        = atoi(row[246]);
			entry.expendablearrow     = atoi(row[247]);
			entry.powersourcecapacity = atoi(row[248]);
			entry.bardeffect          = atoi(row[249]);
			entry.bardeffecttype      = atoi(row[250]);
			entry.bardlevel2          = atoi(row[251]);
			entry.bardlevel           = atoi(row[252]);
			entry.bardunk1            = atoi(row[253]);
			entry.bardunk2            = atoi(row[254]);
			entry.bardunk3            = atoi(row[255]);
			entry.bardunk4            = atoi(row[256]);
			entry.bardunk5            = atoi(row[257]);
			entry.bardname            = row[258] ? row[258] : "";
			entry.bardunk7            = atoi(row[259]);
			entry.UNK214              = atoi(row[260]);
			entry.subtype             = atoi(row[261]);
			entry.UNK220              = atoi(row[262]);
			entry.UNK221              = atoi(row[263]);
			entry.heirloom            = atoi(row[264]);
			entry.UNK223              = atoi(row[265]);
			entry.UNK224              = atoi(row[266]);
			entry.UNK225              = atoi(row[267]);
			entry.UNK226              = atoi(row[268]);
			entry.UNK227              = atoi(row[269]);
			entry.UNK228              = atoi(row[270]);
			entry.UNK229              = atoi(row[271]);
			entry.UNK230              = atoi(row[272]);
			entry.UNK231              = atoi(row[273]);
			entry.UNK232              = atoi(row[274]);
			entry.UNK233              = atoi(row[275]);
			entry.UNK234              = atoi(row[276]);
			entry.placeable           = atoi(row[277]);
			entry.UNK236              = atoi(row[278]);
			entry.UNK237              = atoi(row[279]);
			entry.UNK238              = atoi(row[280]);
			entry.UNK239              = atoi(row[281]);
			entry.UNK240              = atoi(row[282]);
			entry.UNK241              = atoi(row[283]);
			entry.epicitem            = atoi(row[284]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int items_id
	)
	{
		auto results = content_db.QueryDatabase(
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
		Items items_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(items_entry.id));
		update_values.push_back(columns[1] + " = " + std::to_string(items_entry.minstatus));
		update_values.push_back(columns[2] + " = '" + EscapeString(items_entry.Name) + "'");
		update_values.push_back(columns[3] + " = " + std::to_string(items_entry.aagi));
		update_values.push_back(columns[4] + " = " + std::to_string(items_entry.ac));
		update_values.push_back(columns[5] + " = " + std::to_string(items_entry.accuracy));
		update_values.push_back(columns[6] + " = " + std::to_string(items_entry.acha));
		update_values.push_back(columns[7] + " = " + std::to_string(items_entry.adex));
		update_values.push_back(columns[8] + " = " + std::to_string(items_entry.aint));
		update_values.push_back(columns[9] + " = " + std::to_string(items_entry.artifactflag));
		update_values.push_back(columns[10] + " = " + std::to_string(items_entry.asta));
		update_values.push_back(columns[11] + " = " + std::to_string(items_entry.astr));
		update_values.push_back(columns[12] + " = " + std::to_string(items_entry.attack));
		update_values.push_back(columns[13] + " = " + std::to_string(items_entry.augrestrict));
		update_values.push_back(columns[14] + " = " + std::to_string(items_entry.augslot1type));
		update_values.push_back(columns[15] + " = " + std::to_string(items_entry.augslot1visible));
		update_values.push_back(columns[16] + " = " + std::to_string(items_entry.augslot2type));
		update_values.push_back(columns[17] + " = " + std::to_string(items_entry.augslot2visible));
		update_values.push_back(columns[18] + " = " + std::to_string(items_entry.augslot3type));
		update_values.push_back(columns[19] + " = " + std::to_string(items_entry.augslot3visible));
		update_values.push_back(columns[20] + " = " + std::to_string(items_entry.augslot4type));
		update_values.push_back(columns[21] + " = " + std::to_string(items_entry.augslot4visible));
		update_values.push_back(columns[22] + " = " + std::to_string(items_entry.augslot5type));
		update_values.push_back(columns[23] + " = " + std::to_string(items_entry.augslot5visible));
		update_values.push_back(columns[24] + " = " + std::to_string(items_entry.augslot6type));
		update_values.push_back(columns[25] + " = " + std::to_string(items_entry.augslot6visible));
		update_values.push_back(columns[26] + " = " + std::to_string(items_entry.augtype));
		update_values.push_back(columns[27] + " = " + std::to_string(items_entry.avoidance));
		update_values.push_back(columns[28] + " = " + std::to_string(items_entry.awis));
		update_values.push_back(columns[29] + " = " + std::to_string(items_entry.bagsize));
		update_values.push_back(columns[30] + " = " + std::to_string(items_entry.bagslots));
		update_values.push_back(columns[31] + " = " + std::to_string(items_entry.bagtype));
		update_values.push_back(columns[32] + " = " + std::to_string(items_entry.bagwr));
		update_values.push_back(columns[33] + " = " + std::to_string(items_entry.banedmgamt));
		update_values.push_back(columns[34] + " = " + std::to_string(items_entry.banedmgraceamt));
		update_values.push_back(columns[35] + " = " + std::to_string(items_entry.banedmgbody));
		update_values.push_back(columns[36] + " = " + std::to_string(items_entry.banedmgrace));
		update_values.push_back(columns[37] + " = " + std::to_string(items_entry.bardtype));
		update_values.push_back(columns[38] + " = " + std::to_string(items_entry.bardvalue));
		update_values.push_back(columns[39] + " = " + std::to_string(items_entry.book));
		update_values.push_back(columns[40] + " = " + std::to_string(items_entry.casttime));
		update_values.push_back(columns[41] + " = " + std::to_string(items_entry.casttime_));
		update_values.push_back(columns[42] + " = '" + EscapeString(items_entry.charmfile) + "'");
		update_values.push_back(columns[43] + " = '" + EscapeString(items_entry.charmfileid) + "'");
		update_values.push_back(columns[44] + " = " + std::to_string(items_entry.classes));
		update_values.push_back(columns[45] + " = " + std::to_string(items_entry.color));
		update_values.push_back(columns[46] + " = '" + EscapeString(items_entry.combateffects) + "'");
		update_values.push_back(columns[47] + " = " + std::to_string(items_entry.extradmgskill));
		update_values.push_back(columns[48] + " = " + std::to_string(items_entry.extradmgamt));
		update_values.push_back(columns[49] + " = " + std::to_string(items_entry.price));
		update_values.push_back(columns[50] + " = " + std::to_string(items_entry.cr));
		update_values.push_back(columns[51] + " = " + std::to_string(items_entry.damage));
		update_values.push_back(columns[52] + " = " + std::to_string(items_entry.damageshield));
		update_values.push_back(columns[53] + " = " + std::to_string(items_entry.deity));
		update_values.push_back(columns[54] + " = " + std::to_string(items_entry.delay));
		update_values.push_back(columns[55] + " = " + std::to_string(items_entry.augdistiller));
		update_values.push_back(columns[56] + " = " + std::to_string(items_entry.dotshielding));
		update_values.push_back(columns[57] + " = " + std::to_string(items_entry.dr));
		update_values.push_back(columns[58] + " = " + std::to_string(items_entry.clicktype));
		update_values.push_back(columns[59] + " = " + std::to_string(items_entry.clicklevel2));
		update_values.push_back(columns[60] + " = " + std::to_string(items_entry.elemdmgtype));
		update_values.push_back(columns[61] + " = " + std::to_string(items_entry.elemdmgamt));
		update_values.push_back(columns[62] + " = " + std::to_string(items_entry.endur));
		update_values.push_back(columns[63] + " = " + std::to_string(items_entry.factionamt1));
		update_values.push_back(columns[64] + " = " + std::to_string(items_entry.factionamt2));
		update_values.push_back(columns[65] + " = " + std::to_string(items_entry.factionamt3));
		update_values.push_back(columns[66] + " = " + std::to_string(items_entry.factionamt4));
		update_values.push_back(columns[67] + " = " + std::to_string(items_entry.factionmod1));
		update_values.push_back(columns[68] + " = " + std::to_string(items_entry.factionmod2));
		update_values.push_back(columns[69] + " = " + std::to_string(items_entry.factionmod3));
		update_values.push_back(columns[70] + " = " + std::to_string(items_entry.factionmod4));
		update_values.push_back(columns[71] + " = '" + EscapeString(items_entry.filename) + "'");
		update_values.push_back(columns[72] + " = " + std::to_string(items_entry.focuseffect));
		update_values.push_back(columns[73] + " = " + std::to_string(items_entry.fr));
		update_values.push_back(columns[74] + " = " + std::to_string(items_entry.fvnodrop));
		update_values.push_back(columns[75] + " = " + std::to_string(items_entry.haste));
		update_values.push_back(columns[76] + " = " + std::to_string(items_entry.clicklevel));
		update_values.push_back(columns[77] + " = " + std::to_string(items_entry.hp));
		update_values.push_back(columns[78] + " = " + std::to_string(items_entry.regen));
		update_values.push_back(columns[79] + " = " + std::to_string(items_entry.icon));
		update_values.push_back(columns[80] + " = '" + EscapeString(items_entry.idfile) + "'");
		update_values.push_back(columns[81] + " = " + std::to_string(items_entry.itemclass));
		update_values.push_back(columns[82] + " = " + std::to_string(items_entry.itemtype));
		update_values.push_back(columns[83] + " = " + std::to_string(items_entry.ldonprice));
		update_values.push_back(columns[84] + " = " + std::to_string(items_entry.ldontheme));
		update_values.push_back(columns[85] + " = " + std::to_string(items_entry.ldonsold));
		update_values.push_back(columns[86] + " = " + std::to_string(items_entry.light));
		update_values.push_back(columns[87] + " = '" + EscapeString(items_entry.lore) + "'");
		update_values.push_back(columns[88] + " = " + std::to_string(items_entry.loregroup));
		update_values.push_back(columns[89] + " = " + std::to_string(items_entry.magic));
		update_values.push_back(columns[90] + " = " + std::to_string(items_entry.mana));
		update_values.push_back(columns[91] + " = " + std::to_string(items_entry.manaregen));
		update_values.push_back(columns[92] + " = " + std::to_string(items_entry.enduranceregen));
		update_values.push_back(columns[93] + " = " + std::to_string(items_entry.material));
		update_values.push_back(columns[94] + " = " + std::to_string(items_entry.herosforgemodel));
		update_values.push_back(columns[95] + " = " + std::to_string(items_entry.maxcharges));
		update_values.push_back(columns[96] + " = " + std::to_string(items_entry.mr));
		update_values.push_back(columns[97] + " = " + std::to_string(items_entry.nodrop));
		update_values.push_back(columns[98] + " = " + std::to_string(items_entry.norent));
		update_values.push_back(columns[99] + " = " + std::to_string(items_entry.pendingloreflag));
		update_values.push_back(columns[100] + " = " + std::to_string(items_entry.pr));
		update_values.push_back(columns[101] + " = " + std::to_string(items_entry.procrate));
		update_values.push_back(columns[102] + " = " + std::to_string(items_entry.races));
		update_values.push_back(columns[103] + " = " + std::to_string(items_entry.range));
		update_values.push_back(columns[104] + " = " + std::to_string(items_entry.reclevel));
		update_values.push_back(columns[105] + " = " + std::to_string(items_entry.recskill));
		update_values.push_back(columns[106] + " = " + std::to_string(items_entry.reqlevel));
		update_values.push_back(columns[107] + " = " + std::to_string(items_entry.sellrate));
		update_values.push_back(columns[108] + " = " + std::to_string(items_entry.shielding));
		update_values.push_back(columns[109] + " = " + std::to_string(items_entry.size));
		update_values.push_back(columns[110] + " = " + std::to_string(items_entry.skillmodtype));
		update_values.push_back(columns[111] + " = " + std::to_string(items_entry.skillmodvalue));
		update_values.push_back(columns[112] + " = " + std::to_string(items_entry.slots));
		update_values.push_back(columns[113] + " = " + std::to_string(items_entry.clickeffect));
		update_values.push_back(columns[114] + " = " + std::to_string(items_entry.spellshield));
		update_values.push_back(columns[115] + " = " + std::to_string(items_entry.strikethrough));
		update_values.push_back(columns[116] + " = " + std::to_string(items_entry.stunresist));
		update_values.push_back(columns[117] + " = " + std::to_string(items_entry.summonedflag));
		update_values.push_back(columns[118] + " = " + std::to_string(items_entry.tradeskills));
		update_values.push_back(columns[119] + " = " + std::to_string(items_entry.favor));
		update_values.push_back(columns[120] + " = " + std::to_string(items_entry.weight));
		update_values.push_back(columns[121] + " = " + std::to_string(items_entry.UNK012));
		update_values.push_back(columns[122] + " = " + std::to_string(items_entry.UNK013));
		update_values.push_back(columns[123] + " = " + std::to_string(items_entry.benefitflag));
		update_values.push_back(columns[124] + " = " + std::to_string(items_entry.UNK054));
		update_values.push_back(columns[125] + " = " + std::to_string(items_entry.UNK059));
		update_values.push_back(columns[126] + " = " + std::to_string(items_entry.booktype));
		update_values.push_back(columns[127] + " = " + std::to_string(items_entry.recastdelay));
		update_values.push_back(columns[128] + " = " + std::to_string(items_entry.recasttype));
		update_values.push_back(columns[129] + " = " + std::to_string(items_entry.guildfavor));
		update_values.push_back(columns[130] + " = " + std::to_string(items_entry.UNK123));
		update_values.push_back(columns[131] + " = " + std::to_string(items_entry.UNK124));
		update_values.push_back(columns[132] + " = " + std::to_string(items_entry.attuneable));
		update_values.push_back(columns[133] + " = " + std::to_string(items_entry.nopet));
		update_values.push_back(columns[134] + " = '" + EscapeString(items_entry.updated) + "'");
		update_values.push_back(columns[135] + " = '" + EscapeString(items_entry.comment) + "'");
		update_values.push_back(columns[136] + " = " + std::to_string(items_entry.UNK127));
		update_values.push_back(columns[137] + " = " + std::to_string(items_entry.pointtype));
		update_values.push_back(columns[138] + " = " + std::to_string(items_entry.potionbelt));
		update_values.push_back(columns[139] + " = " + std::to_string(items_entry.potionbeltslots));
		update_values.push_back(columns[140] + " = " + std::to_string(items_entry.stacksize));
		update_values.push_back(columns[141] + " = " + std::to_string(items_entry.notransfer));
		update_values.push_back(columns[142] + " = " + std::to_string(items_entry.stackable));
		update_values.push_back(columns[143] + " = '" + EscapeString(items_entry.UNK134) + "'");
		update_values.push_back(columns[144] + " = " + std::to_string(items_entry.UNK137));
		update_values.push_back(columns[145] + " = " + std::to_string(items_entry.proceffect));
		update_values.push_back(columns[146] + " = " + std::to_string(items_entry.proctype));
		update_values.push_back(columns[147] + " = " + std::to_string(items_entry.proclevel2));
		update_values.push_back(columns[148] + " = " + std::to_string(items_entry.proclevel));
		update_values.push_back(columns[149] + " = " + std::to_string(items_entry.UNK142));
		update_values.push_back(columns[150] + " = " + std::to_string(items_entry.worneffect));
		update_values.push_back(columns[151] + " = " + std::to_string(items_entry.worntype));
		update_values.push_back(columns[152] + " = " + std::to_string(items_entry.wornlevel2));
		update_values.push_back(columns[153] + " = " + std::to_string(items_entry.wornlevel));
		update_values.push_back(columns[154] + " = " + std::to_string(items_entry.UNK147));
		update_values.push_back(columns[155] + " = " + std::to_string(items_entry.focustype));
		update_values.push_back(columns[156] + " = " + std::to_string(items_entry.focuslevel2));
		update_values.push_back(columns[157] + " = " + std::to_string(items_entry.focuslevel));
		update_values.push_back(columns[158] + " = " + std::to_string(items_entry.UNK152));
		update_values.push_back(columns[159] + " = " + std::to_string(items_entry.scrolleffect));
		update_values.push_back(columns[160] + " = " + std::to_string(items_entry.scrolltype));
		update_values.push_back(columns[161] + " = " + std::to_string(items_entry.scrolllevel2));
		update_values.push_back(columns[162] + " = " + std::to_string(items_entry.scrolllevel));
		update_values.push_back(columns[163] + " = " + std::to_string(items_entry.UNK157));
		update_values.push_back(columns[164] + " = '" + EscapeString(items_entry.serialized) + "'");
		update_values.push_back(columns[165] + " = '" + EscapeString(items_entry.verified) + "'");
		update_values.push_back(columns[166] + " = '" + EscapeString(items_entry.serialization) + "'");
		update_values.push_back(columns[167] + " = '" + EscapeString(items_entry.source) + "'");
		update_values.push_back(columns[168] + " = " + std::to_string(items_entry.UNK033));
		update_values.push_back(columns[169] + " = '" + EscapeString(items_entry.lorefile) + "'");
		update_values.push_back(columns[170] + " = " + std::to_string(items_entry.UNK014));
		update_values.push_back(columns[171] + " = " + std::to_string(items_entry.svcorruption));
		update_values.push_back(columns[172] + " = " + std::to_string(items_entry.skillmodmax));
		update_values.push_back(columns[173] + " = " + std::to_string(items_entry.UNK060));
		update_values.push_back(columns[174] + " = " + std::to_string(items_entry.augslot1unk2));
		update_values.push_back(columns[175] + " = " + std::to_string(items_entry.augslot2unk2));
		update_values.push_back(columns[176] + " = " + std::to_string(items_entry.augslot3unk2));
		update_values.push_back(columns[177] + " = " + std::to_string(items_entry.augslot4unk2));
		update_values.push_back(columns[178] + " = " + std::to_string(items_entry.augslot5unk2));
		update_values.push_back(columns[179] + " = " + std::to_string(items_entry.augslot6unk2));
		update_values.push_back(columns[180] + " = " + std::to_string(items_entry.UNK120));
		update_values.push_back(columns[181] + " = " + std::to_string(items_entry.UNK121));
		update_values.push_back(columns[182] + " = " + std::to_string(items_entry.questitemflag));
		update_values.push_back(columns[183] + " = '" + EscapeString(items_entry.UNK132) + "'");
		update_values.push_back(columns[184] + " = " + std::to_string(items_entry.clickunk5));
		update_values.push_back(columns[185] + " = '" + EscapeString(items_entry.clickunk6) + "'");
		update_values.push_back(columns[186] + " = " + std::to_string(items_entry.clickunk7));
		update_values.push_back(columns[187] + " = " + std::to_string(items_entry.procunk1));
		update_values.push_back(columns[188] + " = " + std::to_string(items_entry.procunk2));
		update_values.push_back(columns[189] + " = " + std::to_string(items_entry.procunk3));
		update_values.push_back(columns[190] + " = " + std::to_string(items_entry.procunk4));
		update_values.push_back(columns[191] + " = '" + EscapeString(items_entry.procunk6) + "'");
		update_values.push_back(columns[192] + " = " + std::to_string(items_entry.procunk7));
		update_values.push_back(columns[193] + " = " + std::to_string(items_entry.wornunk1));
		update_values.push_back(columns[194] + " = " + std::to_string(items_entry.wornunk2));
		update_values.push_back(columns[195] + " = " + std::to_string(items_entry.wornunk3));
		update_values.push_back(columns[196] + " = " + std::to_string(items_entry.wornunk4));
		update_values.push_back(columns[197] + " = " + std::to_string(items_entry.wornunk5));
		update_values.push_back(columns[198] + " = '" + EscapeString(items_entry.wornunk6) + "'");
		update_values.push_back(columns[199] + " = " + std::to_string(items_entry.wornunk7));
		update_values.push_back(columns[200] + " = " + std::to_string(items_entry.focusunk1));
		update_values.push_back(columns[201] + " = " + std::to_string(items_entry.focusunk2));
		update_values.push_back(columns[202] + " = " + std::to_string(items_entry.focusunk3));
		update_values.push_back(columns[203] + " = " + std::to_string(items_entry.focusunk4));
		update_values.push_back(columns[204] + " = " + std::to_string(items_entry.focusunk5));
		update_values.push_back(columns[205] + " = '" + EscapeString(items_entry.focusunk6) + "'");
		update_values.push_back(columns[206] + " = " + std::to_string(items_entry.focusunk7));
		update_values.push_back(columns[207] + " = " + std::to_string(items_entry.scrollunk1));
		update_values.push_back(columns[208] + " = " + std::to_string(items_entry.scrollunk2));
		update_values.push_back(columns[209] + " = " + std::to_string(items_entry.scrollunk3));
		update_values.push_back(columns[210] + " = " + std::to_string(items_entry.scrollunk4));
		update_values.push_back(columns[211] + " = " + std::to_string(items_entry.scrollunk5));
		update_values.push_back(columns[212] + " = '" + EscapeString(items_entry.scrollunk6) + "'");
		update_values.push_back(columns[213] + " = " + std::to_string(items_entry.scrollunk7));
		update_values.push_back(columns[214] + " = " + std::to_string(items_entry.UNK193));
		update_values.push_back(columns[215] + " = " + std::to_string(items_entry.purity));
		update_values.push_back(columns[216] + " = " + std::to_string(items_entry.evoitem));
		update_values.push_back(columns[217] + " = " + std::to_string(items_entry.evoid));
		update_values.push_back(columns[218] + " = " + std::to_string(items_entry.evolvinglevel));
		update_values.push_back(columns[219] + " = " + std::to_string(items_entry.evomax));
		update_values.push_back(columns[220] + " = '" + EscapeString(items_entry.clickname) + "'");
		update_values.push_back(columns[221] + " = '" + EscapeString(items_entry.procname) + "'");
		update_values.push_back(columns[222] + " = '" + EscapeString(items_entry.wornname) + "'");
		update_values.push_back(columns[223] + " = '" + EscapeString(items_entry.focusname) + "'");
		update_values.push_back(columns[224] + " = '" + EscapeString(items_entry.scrollname) + "'");
		update_values.push_back(columns[225] + " = " + std::to_string(items_entry.dsmitigation));
		update_values.push_back(columns[226] + " = " + std::to_string(items_entry.heroic_str));
		update_values.push_back(columns[227] + " = " + std::to_string(items_entry.heroic_int));
		update_values.push_back(columns[228] + " = " + std::to_string(items_entry.heroic_wis));
		update_values.push_back(columns[229] + " = " + std::to_string(items_entry.heroic_agi));
		update_values.push_back(columns[230] + " = " + std::to_string(items_entry.heroic_dex));
		update_values.push_back(columns[231] + " = " + std::to_string(items_entry.heroic_sta));
		update_values.push_back(columns[232] + " = " + std::to_string(items_entry.heroic_cha));
		update_values.push_back(columns[233] + " = " + std::to_string(items_entry.heroic_pr));
		update_values.push_back(columns[234] + " = " + std::to_string(items_entry.heroic_dr));
		update_values.push_back(columns[235] + " = " + std::to_string(items_entry.heroic_fr));
		update_values.push_back(columns[236] + " = " + std::to_string(items_entry.heroic_cr));
		update_values.push_back(columns[237] + " = " + std::to_string(items_entry.heroic_mr));
		update_values.push_back(columns[238] + " = " + std::to_string(items_entry.heroic_svcorrup));
		update_values.push_back(columns[239] + " = " + std::to_string(items_entry.healamt));
		update_values.push_back(columns[240] + " = " + std::to_string(items_entry.spelldmg));
		update_values.push_back(columns[241] + " = " + std::to_string(items_entry.clairvoyance));
		update_values.push_back(columns[242] + " = " + std::to_string(items_entry.backstabdmg));
		update_values.push_back(columns[243] + " = '" + EscapeString(items_entry.created) + "'");
		update_values.push_back(columns[244] + " = " + std::to_string(items_entry.elitematerial));
		update_values.push_back(columns[245] + " = " + std::to_string(items_entry.ldonsellbackrate));
		update_values.push_back(columns[246] + " = " + std::to_string(items_entry.scriptfileid));
		update_values.push_back(columns[247] + " = " + std::to_string(items_entry.expendablearrow));
		update_values.push_back(columns[248] + " = " + std::to_string(items_entry.powersourcecapacity));
		update_values.push_back(columns[249] + " = " + std::to_string(items_entry.bardeffect));
		update_values.push_back(columns[250] + " = " + std::to_string(items_entry.bardeffecttype));
		update_values.push_back(columns[251] + " = " + std::to_string(items_entry.bardlevel2));
		update_values.push_back(columns[252] + " = " + std::to_string(items_entry.bardlevel));
		update_values.push_back(columns[253] + " = " + std::to_string(items_entry.bardunk1));
		update_values.push_back(columns[254] + " = " + std::to_string(items_entry.bardunk2));
		update_values.push_back(columns[255] + " = " + std::to_string(items_entry.bardunk3));
		update_values.push_back(columns[256] + " = " + std::to_string(items_entry.bardunk4));
		update_values.push_back(columns[257] + " = " + std::to_string(items_entry.bardunk5));
		update_values.push_back(columns[258] + " = '" + EscapeString(items_entry.bardname) + "'");
		update_values.push_back(columns[259] + " = " + std::to_string(items_entry.bardunk7));
		update_values.push_back(columns[260] + " = " + std::to_string(items_entry.UNK214));
		update_values.push_back(columns[261] + " = " + std::to_string(items_entry.subtype));
		update_values.push_back(columns[262] + " = " + std::to_string(items_entry.UNK220));
		update_values.push_back(columns[263] + " = " + std::to_string(items_entry.UNK221));
		update_values.push_back(columns[264] + " = " + std::to_string(items_entry.heirloom));
		update_values.push_back(columns[265] + " = " + std::to_string(items_entry.UNK223));
		update_values.push_back(columns[266] + " = " + std::to_string(items_entry.UNK224));
		update_values.push_back(columns[267] + " = " + std::to_string(items_entry.UNK225));
		update_values.push_back(columns[268] + " = " + std::to_string(items_entry.UNK226));
		update_values.push_back(columns[269] + " = " + std::to_string(items_entry.UNK227));
		update_values.push_back(columns[270] + " = " + std::to_string(items_entry.UNK228));
		update_values.push_back(columns[271] + " = " + std::to_string(items_entry.UNK229));
		update_values.push_back(columns[272] + " = " + std::to_string(items_entry.UNK230));
		update_values.push_back(columns[273] + " = " + std::to_string(items_entry.UNK231));
		update_values.push_back(columns[274] + " = " + std::to_string(items_entry.UNK232));
		update_values.push_back(columns[275] + " = " + std::to_string(items_entry.UNK233));
		update_values.push_back(columns[276] + " = " + std::to_string(items_entry.UNK234));
		update_values.push_back(columns[277] + " = " + std::to_string(items_entry.placeable));
		update_values.push_back(columns[278] + " = " + std::to_string(items_entry.UNK236));
		update_values.push_back(columns[279] + " = " + std::to_string(items_entry.UNK237));
		update_values.push_back(columns[280] + " = " + std::to_string(items_entry.UNK238));
		update_values.push_back(columns[281] + " = " + std::to_string(items_entry.UNK239));
		update_values.push_back(columns[282] + " = " + std::to_string(items_entry.UNK240));
		update_values.push_back(columns[283] + " = " + std::to_string(items_entry.UNK241));
		update_values.push_back(columns[284] + " = " + std::to_string(items_entry.epicitem));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				items_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Items InsertOne(
		Items items_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(items_entry.id));
		insert_values.push_back(std::to_string(items_entry.minstatus));
		insert_values.push_back("'" + EscapeString(items_entry.Name) + "'");
		insert_values.push_back(std::to_string(items_entry.aagi));
		insert_values.push_back(std::to_string(items_entry.ac));
		insert_values.push_back(std::to_string(items_entry.accuracy));
		insert_values.push_back(std::to_string(items_entry.acha));
		insert_values.push_back(std::to_string(items_entry.adex));
		insert_values.push_back(std::to_string(items_entry.aint));
		insert_values.push_back(std::to_string(items_entry.artifactflag));
		insert_values.push_back(std::to_string(items_entry.asta));
		insert_values.push_back(std::to_string(items_entry.astr));
		insert_values.push_back(std::to_string(items_entry.attack));
		insert_values.push_back(std::to_string(items_entry.augrestrict));
		insert_values.push_back(std::to_string(items_entry.augslot1type));
		insert_values.push_back(std::to_string(items_entry.augslot1visible));
		insert_values.push_back(std::to_string(items_entry.augslot2type));
		insert_values.push_back(std::to_string(items_entry.augslot2visible));
		insert_values.push_back(std::to_string(items_entry.augslot3type));
		insert_values.push_back(std::to_string(items_entry.augslot3visible));
		insert_values.push_back(std::to_string(items_entry.augslot4type));
		insert_values.push_back(std::to_string(items_entry.augslot4visible));
		insert_values.push_back(std::to_string(items_entry.augslot5type));
		insert_values.push_back(std::to_string(items_entry.augslot5visible));
		insert_values.push_back(std::to_string(items_entry.augslot6type));
		insert_values.push_back(std::to_string(items_entry.augslot6visible));
		insert_values.push_back(std::to_string(items_entry.augtype));
		insert_values.push_back(std::to_string(items_entry.avoidance));
		insert_values.push_back(std::to_string(items_entry.awis));
		insert_values.push_back(std::to_string(items_entry.bagsize));
		insert_values.push_back(std::to_string(items_entry.bagslots));
		insert_values.push_back(std::to_string(items_entry.bagtype));
		insert_values.push_back(std::to_string(items_entry.bagwr));
		insert_values.push_back(std::to_string(items_entry.banedmgamt));
		insert_values.push_back(std::to_string(items_entry.banedmgraceamt));
		insert_values.push_back(std::to_string(items_entry.banedmgbody));
		insert_values.push_back(std::to_string(items_entry.banedmgrace));
		insert_values.push_back(std::to_string(items_entry.bardtype));
		insert_values.push_back(std::to_string(items_entry.bardvalue));
		insert_values.push_back(std::to_string(items_entry.book));
		insert_values.push_back(std::to_string(items_entry.casttime));
		insert_values.push_back(std::to_string(items_entry.casttime_));
		insert_values.push_back("'" + EscapeString(items_entry.charmfile) + "'");
		insert_values.push_back("'" + EscapeString(items_entry.charmfileid) + "'");
		insert_values.push_back(std::to_string(items_entry.classes));
		insert_values.push_back(std::to_string(items_entry.color));
		insert_values.push_back("'" + EscapeString(items_entry.combateffects) + "'");
		insert_values.push_back(std::to_string(items_entry.extradmgskill));
		insert_values.push_back(std::to_string(items_entry.extradmgamt));
		insert_values.push_back(std::to_string(items_entry.price));
		insert_values.push_back(std::to_string(items_entry.cr));
		insert_values.push_back(std::to_string(items_entry.damage));
		insert_values.push_back(std::to_string(items_entry.damageshield));
		insert_values.push_back(std::to_string(items_entry.deity));
		insert_values.push_back(std::to_string(items_entry.delay));
		insert_values.push_back(std::to_string(items_entry.augdistiller));
		insert_values.push_back(std::to_string(items_entry.dotshielding));
		insert_values.push_back(std::to_string(items_entry.dr));
		insert_values.push_back(std::to_string(items_entry.clicktype));
		insert_values.push_back(std::to_string(items_entry.clicklevel2));
		insert_values.push_back(std::to_string(items_entry.elemdmgtype));
		insert_values.push_back(std::to_string(items_entry.elemdmgamt));
		insert_values.push_back(std::to_string(items_entry.endur));
		insert_values.push_back(std::to_string(items_entry.factionamt1));
		insert_values.push_back(std::to_string(items_entry.factionamt2));
		insert_values.push_back(std::to_string(items_entry.factionamt3));
		insert_values.push_back(std::to_string(items_entry.factionamt4));
		insert_values.push_back(std::to_string(items_entry.factionmod1));
		insert_values.push_back(std::to_string(items_entry.factionmod2));
		insert_values.push_back(std::to_string(items_entry.factionmod3));
		insert_values.push_back(std::to_string(items_entry.factionmod4));
		insert_values.push_back("'" + EscapeString(items_entry.filename) + "'");
		insert_values.push_back(std::to_string(items_entry.focuseffect));
		insert_values.push_back(std::to_string(items_entry.fr));
		insert_values.push_back(std::to_string(items_entry.fvnodrop));
		insert_values.push_back(std::to_string(items_entry.haste));
		insert_values.push_back(std::to_string(items_entry.clicklevel));
		insert_values.push_back(std::to_string(items_entry.hp));
		insert_values.push_back(std::to_string(items_entry.regen));
		insert_values.push_back(std::to_string(items_entry.icon));
		insert_values.push_back("'" + EscapeString(items_entry.idfile) + "'");
		insert_values.push_back(std::to_string(items_entry.itemclass));
		insert_values.push_back(std::to_string(items_entry.itemtype));
		insert_values.push_back(std::to_string(items_entry.ldonprice));
		insert_values.push_back(std::to_string(items_entry.ldontheme));
		insert_values.push_back(std::to_string(items_entry.ldonsold));
		insert_values.push_back(std::to_string(items_entry.light));
		insert_values.push_back("'" + EscapeString(items_entry.lore) + "'");
		insert_values.push_back(std::to_string(items_entry.loregroup));
		insert_values.push_back(std::to_string(items_entry.magic));
		insert_values.push_back(std::to_string(items_entry.mana));
		insert_values.push_back(std::to_string(items_entry.manaregen));
		insert_values.push_back(std::to_string(items_entry.enduranceregen));
		insert_values.push_back(std::to_string(items_entry.material));
		insert_values.push_back(std::to_string(items_entry.herosforgemodel));
		insert_values.push_back(std::to_string(items_entry.maxcharges));
		insert_values.push_back(std::to_string(items_entry.mr));
		insert_values.push_back(std::to_string(items_entry.nodrop));
		insert_values.push_back(std::to_string(items_entry.norent));
		insert_values.push_back(std::to_string(items_entry.pendingloreflag));
		insert_values.push_back(std::to_string(items_entry.pr));
		insert_values.push_back(std::to_string(items_entry.procrate));
		insert_values.push_back(std::to_string(items_entry.races));
		insert_values.push_back(std::to_string(items_entry.range));
		insert_values.push_back(std::to_string(items_entry.reclevel));
		insert_values.push_back(std::to_string(items_entry.recskill));
		insert_values.push_back(std::to_string(items_entry.reqlevel));
		insert_values.push_back(std::to_string(items_entry.sellrate));
		insert_values.push_back(std::to_string(items_entry.shielding));
		insert_values.push_back(std::to_string(items_entry.size));
		insert_values.push_back(std::to_string(items_entry.skillmodtype));
		insert_values.push_back(std::to_string(items_entry.skillmodvalue));
		insert_values.push_back(std::to_string(items_entry.slots));
		insert_values.push_back(std::to_string(items_entry.clickeffect));
		insert_values.push_back(std::to_string(items_entry.spellshield));
		insert_values.push_back(std::to_string(items_entry.strikethrough));
		insert_values.push_back(std::to_string(items_entry.stunresist));
		insert_values.push_back(std::to_string(items_entry.summonedflag));
		insert_values.push_back(std::to_string(items_entry.tradeskills));
		insert_values.push_back(std::to_string(items_entry.favor));
		insert_values.push_back(std::to_string(items_entry.weight));
		insert_values.push_back(std::to_string(items_entry.UNK012));
		insert_values.push_back(std::to_string(items_entry.UNK013));
		insert_values.push_back(std::to_string(items_entry.benefitflag));
		insert_values.push_back(std::to_string(items_entry.UNK054));
		insert_values.push_back(std::to_string(items_entry.UNK059));
		insert_values.push_back(std::to_string(items_entry.booktype));
		insert_values.push_back(std::to_string(items_entry.recastdelay));
		insert_values.push_back(std::to_string(items_entry.recasttype));
		insert_values.push_back(std::to_string(items_entry.guildfavor));
		insert_values.push_back(std::to_string(items_entry.UNK123));
		insert_values.push_back(std::to_string(items_entry.UNK124));
		insert_values.push_back(std::to_string(items_entry.attuneable));
		insert_values.push_back(std::to_string(items_entry.nopet));
		insert_values.push_back("'" + EscapeString(items_entry.updated) + "'");
		insert_values.push_back("'" + EscapeString(items_entry.comment) + "'");
		insert_values.push_back(std::to_string(items_entry.UNK127));
		insert_values.push_back(std::to_string(items_entry.pointtype));
		insert_values.push_back(std::to_string(items_entry.potionbelt));
		insert_values.push_back(std::to_string(items_entry.potionbeltslots));
		insert_values.push_back(std::to_string(items_entry.stacksize));
		insert_values.push_back(std::to_string(items_entry.notransfer));
		insert_values.push_back(std::to_string(items_entry.stackable));
		insert_values.push_back("'" + EscapeString(items_entry.UNK134) + "'");
		insert_values.push_back(std::to_string(items_entry.UNK137));
		insert_values.push_back(std::to_string(items_entry.proceffect));
		insert_values.push_back(std::to_string(items_entry.proctype));
		insert_values.push_back(std::to_string(items_entry.proclevel2));
		insert_values.push_back(std::to_string(items_entry.proclevel));
		insert_values.push_back(std::to_string(items_entry.UNK142));
		insert_values.push_back(std::to_string(items_entry.worneffect));
		insert_values.push_back(std::to_string(items_entry.worntype));
		insert_values.push_back(std::to_string(items_entry.wornlevel2));
		insert_values.push_back(std::to_string(items_entry.wornlevel));
		insert_values.push_back(std::to_string(items_entry.UNK147));
		insert_values.push_back(std::to_string(items_entry.focustype));
		insert_values.push_back(std::to_string(items_entry.focuslevel2));
		insert_values.push_back(std::to_string(items_entry.focuslevel));
		insert_values.push_back(std::to_string(items_entry.UNK152));
		insert_values.push_back(std::to_string(items_entry.scrolleffect));
		insert_values.push_back(std::to_string(items_entry.scrolltype));
		insert_values.push_back(std::to_string(items_entry.scrolllevel2));
		insert_values.push_back(std::to_string(items_entry.scrolllevel));
		insert_values.push_back(std::to_string(items_entry.UNK157));
		insert_values.push_back("'" + EscapeString(items_entry.serialized) + "'");
		insert_values.push_back("'" + EscapeString(items_entry.verified) + "'");
		insert_values.push_back("'" + EscapeString(items_entry.serialization) + "'");
		insert_values.push_back("'" + EscapeString(items_entry.source) + "'");
		insert_values.push_back(std::to_string(items_entry.UNK033));
		insert_values.push_back("'" + EscapeString(items_entry.lorefile) + "'");
		insert_values.push_back(std::to_string(items_entry.UNK014));
		insert_values.push_back(std::to_string(items_entry.svcorruption));
		insert_values.push_back(std::to_string(items_entry.skillmodmax));
		insert_values.push_back(std::to_string(items_entry.UNK060));
		insert_values.push_back(std::to_string(items_entry.augslot1unk2));
		insert_values.push_back(std::to_string(items_entry.augslot2unk2));
		insert_values.push_back(std::to_string(items_entry.augslot3unk2));
		insert_values.push_back(std::to_string(items_entry.augslot4unk2));
		insert_values.push_back(std::to_string(items_entry.augslot5unk2));
		insert_values.push_back(std::to_string(items_entry.augslot6unk2));
		insert_values.push_back(std::to_string(items_entry.UNK120));
		insert_values.push_back(std::to_string(items_entry.UNK121));
		insert_values.push_back(std::to_string(items_entry.questitemflag));
		insert_values.push_back("'" + EscapeString(items_entry.UNK132) + "'");
		insert_values.push_back(std::to_string(items_entry.clickunk5));
		insert_values.push_back("'" + EscapeString(items_entry.clickunk6) + "'");
		insert_values.push_back(std::to_string(items_entry.clickunk7));
		insert_values.push_back(std::to_string(items_entry.procunk1));
		insert_values.push_back(std::to_string(items_entry.procunk2));
		insert_values.push_back(std::to_string(items_entry.procunk3));
		insert_values.push_back(std::to_string(items_entry.procunk4));
		insert_values.push_back("'" + EscapeString(items_entry.procunk6) + "'");
		insert_values.push_back(std::to_string(items_entry.procunk7));
		insert_values.push_back(std::to_string(items_entry.wornunk1));
		insert_values.push_back(std::to_string(items_entry.wornunk2));
		insert_values.push_back(std::to_string(items_entry.wornunk3));
		insert_values.push_back(std::to_string(items_entry.wornunk4));
		insert_values.push_back(std::to_string(items_entry.wornunk5));
		insert_values.push_back("'" + EscapeString(items_entry.wornunk6) + "'");
		insert_values.push_back(std::to_string(items_entry.wornunk7));
		insert_values.push_back(std::to_string(items_entry.focusunk1));
		insert_values.push_back(std::to_string(items_entry.focusunk2));
		insert_values.push_back(std::to_string(items_entry.focusunk3));
		insert_values.push_back(std::to_string(items_entry.focusunk4));
		insert_values.push_back(std::to_string(items_entry.focusunk5));
		insert_values.push_back("'" + EscapeString(items_entry.focusunk6) + "'");
		insert_values.push_back(std::to_string(items_entry.focusunk7));
		insert_values.push_back(std::to_string(items_entry.scrollunk1));
		insert_values.push_back(std::to_string(items_entry.scrollunk2));
		insert_values.push_back(std::to_string(items_entry.scrollunk3));
		insert_values.push_back(std::to_string(items_entry.scrollunk4));
		insert_values.push_back(std::to_string(items_entry.scrollunk5));
		insert_values.push_back("'" + EscapeString(items_entry.scrollunk6) + "'");
		insert_values.push_back(std::to_string(items_entry.scrollunk7));
		insert_values.push_back(std::to_string(items_entry.UNK193));
		insert_values.push_back(std::to_string(items_entry.purity));
		insert_values.push_back(std::to_string(items_entry.evoitem));
		insert_values.push_back(std::to_string(items_entry.evoid));
		insert_values.push_back(std::to_string(items_entry.evolvinglevel));
		insert_values.push_back(std::to_string(items_entry.evomax));
		insert_values.push_back("'" + EscapeString(items_entry.clickname) + "'");
		insert_values.push_back("'" + EscapeString(items_entry.procname) + "'");
		insert_values.push_back("'" + EscapeString(items_entry.wornname) + "'");
		insert_values.push_back("'" + EscapeString(items_entry.focusname) + "'");
		insert_values.push_back("'" + EscapeString(items_entry.scrollname) + "'");
		insert_values.push_back(std::to_string(items_entry.dsmitigation));
		insert_values.push_back(std::to_string(items_entry.heroic_str));
		insert_values.push_back(std::to_string(items_entry.heroic_int));
		insert_values.push_back(std::to_string(items_entry.heroic_wis));
		insert_values.push_back(std::to_string(items_entry.heroic_agi));
		insert_values.push_back(std::to_string(items_entry.heroic_dex));
		insert_values.push_back(std::to_string(items_entry.heroic_sta));
		insert_values.push_back(std::to_string(items_entry.heroic_cha));
		insert_values.push_back(std::to_string(items_entry.heroic_pr));
		insert_values.push_back(std::to_string(items_entry.heroic_dr));
		insert_values.push_back(std::to_string(items_entry.heroic_fr));
		insert_values.push_back(std::to_string(items_entry.heroic_cr));
		insert_values.push_back(std::to_string(items_entry.heroic_mr));
		insert_values.push_back(std::to_string(items_entry.heroic_svcorrup));
		insert_values.push_back(std::to_string(items_entry.healamt));
		insert_values.push_back(std::to_string(items_entry.spelldmg));
		insert_values.push_back(std::to_string(items_entry.clairvoyance));
		insert_values.push_back(std::to_string(items_entry.backstabdmg));
		insert_values.push_back("'" + EscapeString(items_entry.created) + "'");
		insert_values.push_back(std::to_string(items_entry.elitematerial));
		insert_values.push_back(std::to_string(items_entry.ldonsellbackrate));
		insert_values.push_back(std::to_string(items_entry.scriptfileid));
		insert_values.push_back(std::to_string(items_entry.expendablearrow));
		insert_values.push_back(std::to_string(items_entry.powersourcecapacity));
		insert_values.push_back(std::to_string(items_entry.bardeffect));
		insert_values.push_back(std::to_string(items_entry.bardeffecttype));
		insert_values.push_back(std::to_string(items_entry.bardlevel2));
		insert_values.push_back(std::to_string(items_entry.bardlevel));
		insert_values.push_back(std::to_string(items_entry.bardunk1));
		insert_values.push_back(std::to_string(items_entry.bardunk2));
		insert_values.push_back(std::to_string(items_entry.bardunk3));
		insert_values.push_back(std::to_string(items_entry.bardunk4));
		insert_values.push_back(std::to_string(items_entry.bardunk5));
		insert_values.push_back("'" + EscapeString(items_entry.bardname) + "'");
		insert_values.push_back(std::to_string(items_entry.bardunk7));
		insert_values.push_back(std::to_string(items_entry.UNK214));
		insert_values.push_back(std::to_string(items_entry.subtype));
		insert_values.push_back(std::to_string(items_entry.UNK220));
		insert_values.push_back(std::to_string(items_entry.UNK221));
		insert_values.push_back(std::to_string(items_entry.heirloom));
		insert_values.push_back(std::to_string(items_entry.UNK223));
		insert_values.push_back(std::to_string(items_entry.UNK224));
		insert_values.push_back(std::to_string(items_entry.UNK225));
		insert_values.push_back(std::to_string(items_entry.UNK226));
		insert_values.push_back(std::to_string(items_entry.UNK227));
		insert_values.push_back(std::to_string(items_entry.UNK228));
		insert_values.push_back(std::to_string(items_entry.UNK229));
		insert_values.push_back(std::to_string(items_entry.UNK230));
		insert_values.push_back(std::to_string(items_entry.UNK231));
		insert_values.push_back(std::to_string(items_entry.UNK232));
		insert_values.push_back(std::to_string(items_entry.UNK233));
		insert_values.push_back(std::to_string(items_entry.UNK234));
		insert_values.push_back(std::to_string(items_entry.placeable));
		insert_values.push_back(std::to_string(items_entry.UNK236));
		insert_values.push_back(std::to_string(items_entry.UNK237));
		insert_values.push_back(std::to_string(items_entry.UNK238));
		insert_values.push_back(std::to_string(items_entry.UNK239));
		insert_values.push_back(std::to_string(items_entry.UNK240));
		insert_values.push_back(std::to_string(items_entry.UNK241));
		insert_values.push_back(std::to_string(items_entry.epicitem));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			items_entry.id = results.LastInsertedID();
			return items_entry;
		}

		items_entry = NewEntity();

		return items_entry;
	}

	static int InsertMany(
		std::vector<Items> items_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &items_entry: items_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(items_entry.id));
			insert_values.push_back(std::to_string(items_entry.minstatus));
			insert_values.push_back("'" + EscapeString(items_entry.Name) + "'");
			insert_values.push_back(std::to_string(items_entry.aagi));
			insert_values.push_back(std::to_string(items_entry.ac));
			insert_values.push_back(std::to_string(items_entry.accuracy));
			insert_values.push_back(std::to_string(items_entry.acha));
			insert_values.push_back(std::to_string(items_entry.adex));
			insert_values.push_back(std::to_string(items_entry.aint));
			insert_values.push_back(std::to_string(items_entry.artifactflag));
			insert_values.push_back(std::to_string(items_entry.asta));
			insert_values.push_back(std::to_string(items_entry.astr));
			insert_values.push_back(std::to_string(items_entry.attack));
			insert_values.push_back(std::to_string(items_entry.augrestrict));
			insert_values.push_back(std::to_string(items_entry.augslot1type));
			insert_values.push_back(std::to_string(items_entry.augslot1visible));
			insert_values.push_back(std::to_string(items_entry.augslot2type));
			insert_values.push_back(std::to_string(items_entry.augslot2visible));
			insert_values.push_back(std::to_string(items_entry.augslot3type));
			insert_values.push_back(std::to_string(items_entry.augslot3visible));
			insert_values.push_back(std::to_string(items_entry.augslot4type));
			insert_values.push_back(std::to_string(items_entry.augslot4visible));
			insert_values.push_back(std::to_string(items_entry.augslot5type));
			insert_values.push_back(std::to_string(items_entry.augslot5visible));
			insert_values.push_back(std::to_string(items_entry.augslot6type));
			insert_values.push_back(std::to_string(items_entry.augslot6visible));
			insert_values.push_back(std::to_string(items_entry.augtype));
			insert_values.push_back(std::to_string(items_entry.avoidance));
			insert_values.push_back(std::to_string(items_entry.awis));
			insert_values.push_back(std::to_string(items_entry.bagsize));
			insert_values.push_back(std::to_string(items_entry.bagslots));
			insert_values.push_back(std::to_string(items_entry.bagtype));
			insert_values.push_back(std::to_string(items_entry.bagwr));
			insert_values.push_back(std::to_string(items_entry.banedmgamt));
			insert_values.push_back(std::to_string(items_entry.banedmgraceamt));
			insert_values.push_back(std::to_string(items_entry.banedmgbody));
			insert_values.push_back(std::to_string(items_entry.banedmgrace));
			insert_values.push_back(std::to_string(items_entry.bardtype));
			insert_values.push_back(std::to_string(items_entry.bardvalue));
			insert_values.push_back(std::to_string(items_entry.book));
			insert_values.push_back(std::to_string(items_entry.casttime));
			insert_values.push_back(std::to_string(items_entry.casttime_));
			insert_values.push_back("'" + EscapeString(items_entry.charmfile) + "'");
			insert_values.push_back("'" + EscapeString(items_entry.charmfileid) + "'");
			insert_values.push_back(std::to_string(items_entry.classes));
			insert_values.push_back(std::to_string(items_entry.color));
			insert_values.push_back("'" + EscapeString(items_entry.combateffects) + "'");
			insert_values.push_back(std::to_string(items_entry.extradmgskill));
			insert_values.push_back(std::to_string(items_entry.extradmgamt));
			insert_values.push_back(std::to_string(items_entry.price));
			insert_values.push_back(std::to_string(items_entry.cr));
			insert_values.push_back(std::to_string(items_entry.damage));
			insert_values.push_back(std::to_string(items_entry.damageshield));
			insert_values.push_back(std::to_string(items_entry.deity));
			insert_values.push_back(std::to_string(items_entry.delay));
			insert_values.push_back(std::to_string(items_entry.augdistiller));
			insert_values.push_back(std::to_string(items_entry.dotshielding));
			insert_values.push_back(std::to_string(items_entry.dr));
			insert_values.push_back(std::to_string(items_entry.clicktype));
			insert_values.push_back(std::to_string(items_entry.clicklevel2));
			insert_values.push_back(std::to_string(items_entry.elemdmgtype));
			insert_values.push_back(std::to_string(items_entry.elemdmgamt));
			insert_values.push_back(std::to_string(items_entry.endur));
			insert_values.push_back(std::to_string(items_entry.factionamt1));
			insert_values.push_back(std::to_string(items_entry.factionamt2));
			insert_values.push_back(std::to_string(items_entry.factionamt3));
			insert_values.push_back(std::to_string(items_entry.factionamt4));
			insert_values.push_back(std::to_string(items_entry.factionmod1));
			insert_values.push_back(std::to_string(items_entry.factionmod2));
			insert_values.push_back(std::to_string(items_entry.factionmod3));
			insert_values.push_back(std::to_string(items_entry.factionmod4));
			insert_values.push_back("'" + EscapeString(items_entry.filename) + "'");
			insert_values.push_back(std::to_string(items_entry.focuseffect));
			insert_values.push_back(std::to_string(items_entry.fr));
			insert_values.push_back(std::to_string(items_entry.fvnodrop));
			insert_values.push_back(std::to_string(items_entry.haste));
			insert_values.push_back(std::to_string(items_entry.clicklevel));
			insert_values.push_back(std::to_string(items_entry.hp));
			insert_values.push_back(std::to_string(items_entry.regen));
			insert_values.push_back(std::to_string(items_entry.icon));
			insert_values.push_back("'" + EscapeString(items_entry.idfile) + "'");
			insert_values.push_back(std::to_string(items_entry.itemclass));
			insert_values.push_back(std::to_string(items_entry.itemtype));
			insert_values.push_back(std::to_string(items_entry.ldonprice));
			insert_values.push_back(std::to_string(items_entry.ldontheme));
			insert_values.push_back(std::to_string(items_entry.ldonsold));
			insert_values.push_back(std::to_string(items_entry.light));
			insert_values.push_back("'" + EscapeString(items_entry.lore) + "'");
			insert_values.push_back(std::to_string(items_entry.loregroup));
			insert_values.push_back(std::to_string(items_entry.magic));
			insert_values.push_back(std::to_string(items_entry.mana));
			insert_values.push_back(std::to_string(items_entry.manaregen));
			insert_values.push_back(std::to_string(items_entry.enduranceregen));
			insert_values.push_back(std::to_string(items_entry.material));
			insert_values.push_back(std::to_string(items_entry.herosforgemodel));
			insert_values.push_back(std::to_string(items_entry.maxcharges));
			insert_values.push_back(std::to_string(items_entry.mr));
			insert_values.push_back(std::to_string(items_entry.nodrop));
			insert_values.push_back(std::to_string(items_entry.norent));
			insert_values.push_back(std::to_string(items_entry.pendingloreflag));
			insert_values.push_back(std::to_string(items_entry.pr));
			insert_values.push_back(std::to_string(items_entry.procrate));
			insert_values.push_back(std::to_string(items_entry.races));
			insert_values.push_back(std::to_string(items_entry.range));
			insert_values.push_back(std::to_string(items_entry.reclevel));
			insert_values.push_back(std::to_string(items_entry.recskill));
			insert_values.push_back(std::to_string(items_entry.reqlevel));
			insert_values.push_back(std::to_string(items_entry.sellrate));
			insert_values.push_back(std::to_string(items_entry.shielding));
			insert_values.push_back(std::to_string(items_entry.size));
			insert_values.push_back(std::to_string(items_entry.skillmodtype));
			insert_values.push_back(std::to_string(items_entry.skillmodvalue));
			insert_values.push_back(std::to_string(items_entry.slots));
			insert_values.push_back(std::to_string(items_entry.clickeffect));
			insert_values.push_back(std::to_string(items_entry.spellshield));
			insert_values.push_back(std::to_string(items_entry.strikethrough));
			insert_values.push_back(std::to_string(items_entry.stunresist));
			insert_values.push_back(std::to_string(items_entry.summonedflag));
			insert_values.push_back(std::to_string(items_entry.tradeskills));
			insert_values.push_back(std::to_string(items_entry.favor));
			insert_values.push_back(std::to_string(items_entry.weight));
			insert_values.push_back(std::to_string(items_entry.UNK012));
			insert_values.push_back(std::to_string(items_entry.UNK013));
			insert_values.push_back(std::to_string(items_entry.benefitflag));
			insert_values.push_back(std::to_string(items_entry.UNK054));
			insert_values.push_back(std::to_string(items_entry.UNK059));
			insert_values.push_back(std::to_string(items_entry.booktype));
			insert_values.push_back(std::to_string(items_entry.recastdelay));
			insert_values.push_back(std::to_string(items_entry.recasttype));
			insert_values.push_back(std::to_string(items_entry.guildfavor));
			insert_values.push_back(std::to_string(items_entry.UNK123));
			insert_values.push_back(std::to_string(items_entry.UNK124));
			insert_values.push_back(std::to_string(items_entry.attuneable));
			insert_values.push_back(std::to_string(items_entry.nopet));
			insert_values.push_back("'" + EscapeString(items_entry.updated) + "'");
			insert_values.push_back("'" + EscapeString(items_entry.comment) + "'");
			insert_values.push_back(std::to_string(items_entry.UNK127));
			insert_values.push_back(std::to_string(items_entry.pointtype));
			insert_values.push_back(std::to_string(items_entry.potionbelt));
			insert_values.push_back(std::to_string(items_entry.potionbeltslots));
			insert_values.push_back(std::to_string(items_entry.stacksize));
			insert_values.push_back(std::to_string(items_entry.notransfer));
			insert_values.push_back(std::to_string(items_entry.stackable));
			insert_values.push_back("'" + EscapeString(items_entry.UNK134) + "'");
			insert_values.push_back(std::to_string(items_entry.UNK137));
			insert_values.push_back(std::to_string(items_entry.proceffect));
			insert_values.push_back(std::to_string(items_entry.proctype));
			insert_values.push_back(std::to_string(items_entry.proclevel2));
			insert_values.push_back(std::to_string(items_entry.proclevel));
			insert_values.push_back(std::to_string(items_entry.UNK142));
			insert_values.push_back(std::to_string(items_entry.worneffect));
			insert_values.push_back(std::to_string(items_entry.worntype));
			insert_values.push_back(std::to_string(items_entry.wornlevel2));
			insert_values.push_back(std::to_string(items_entry.wornlevel));
			insert_values.push_back(std::to_string(items_entry.UNK147));
			insert_values.push_back(std::to_string(items_entry.focustype));
			insert_values.push_back(std::to_string(items_entry.focuslevel2));
			insert_values.push_back(std::to_string(items_entry.focuslevel));
			insert_values.push_back(std::to_string(items_entry.UNK152));
			insert_values.push_back(std::to_string(items_entry.scrolleffect));
			insert_values.push_back(std::to_string(items_entry.scrolltype));
			insert_values.push_back(std::to_string(items_entry.scrolllevel2));
			insert_values.push_back(std::to_string(items_entry.scrolllevel));
			insert_values.push_back(std::to_string(items_entry.UNK157));
			insert_values.push_back("'" + EscapeString(items_entry.serialized) + "'");
			insert_values.push_back("'" + EscapeString(items_entry.verified) + "'");
			insert_values.push_back("'" + EscapeString(items_entry.serialization) + "'");
			insert_values.push_back("'" + EscapeString(items_entry.source) + "'");
			insert_values.push_back(std::to_string(items_entry.UNK033));
			insert_values.push_back("'" + EscapeString(items_entry.lorefile) + "'");
			insert_values.push_back(std::to_string(items_entry.UNK014));
			insert_values.push_back(std::to_string(items_entry.svcorruption));
			insert_values.push_back(std::to_string(items_entry.skillmodmax));
			insert_values.push_back(std::to_string(items_entry.UNK060));
			insert_values.push_back(std::to_string(items_entry.augslot1unk2));
			insert_values.push_back(std::to_string(items_entry.augslot2unk2));
			insert_values.push_back(std::to_string(items_entry.augslot3unk2));
			insert_values.push_back(std::to_string(items_entry.augslot4unk2));
			insert_values.push_back(std::to_string(items_entry.augslot5unk2));
			insert_values.push_back(std::to_string(items_entry.augslot6unk2));
			insert_values.push_back(std::to_string(items_entry.UNK120));
			insert_values.push_back(std::to_string(items_entry.UNK121));
			insert_values.push_back(std::to_string(items_entry.questitemflag));
			insert_values.push_back("'" + EscapeString(items_entry.UNK132) + "'");
			insert_values.push_back(std::to_string(items_entry.clickunk5));
			insert_values.push_back("'" + EscapeString(items_entry.clickunk6) + "'");
			insert_values.push_back(std::to_string(items_entry.clickunk7));
			insert_values.push_back(std::to_string(items_entry.procunk1));
			insert_values.push_back(std::to_string(items_entry.procunk2));
			insert_values.push_back(std::to_string(items_entry.procunk3));
			insert_values.push_back(std::to_string(items_entry.procunk4));
			insert_values.push_back("'" + EscapeString(items_entry.procunk6) + "'");
			insert_values.push_back(std::to_string(items_entry.procunk7));
			insert_values.push_back(std::to_string(items_entry.wornunk1));
			insert_values.push_back(std::to_string(items_entry.wornunk2));
			insert_values.push_back(std::to_string(items_entry.wornunk3));
			insert_values.push_back(std::to_string(items_entry.wornunk4));
			insert_values.push_back(std::to_string(items_entry.wornunk5));
			insert_values.push_back("'" + EscapeString(items_entry.wornunk6) + "'");
			insert_values.push_back(std::to_string(items_entry.wornunk7));
			insert_values.push_back(std::to_string(items_entry.focusunk1));
			insert_values.push_back(std::to_string(items_entry.focusunk2));
			insert_values.push_back(std::to_string(items_entry.focusunk3));
			insert_values.push_back(std::to_string(items_entry.focusunk4));
			insert_values.push_back(std::to_string(items_entry.focusunk5));
			insert_values.push_back("'" + EscapeString(items_entry.focusunk6) + "'");
			insert_values.push_back(std::to_string(items_entry.focusunk7));
			insert_values.push_back(std::to_string(items_entry.scrollunk1));
			insert_values.push_back(std::to_string(items_entry.scrollunk2));
			insert_values.push_back(std::to_string(items_entry.scrollunk3));
			insert_values.push_back(std::to_string(items_entry.scrollunk4));
			insert_values.push_back(std::to_string(items_entry.scrollunk5));
			insert_values.push_back("'" + EscapeString(items_entry.scrollunk6) + "'");
			insert_values.push_back(std::to_string(items_entry.scrollunk7));
			insert_values.push_back(std::to_string(items_entry.UNK193));
			insert_values.push_back(std::to_string(items_entry.purity));
			insert_values.push_back(std::to_string(items_entry.evoitem));
			insert_values.push_back(std::to_string(items_entry.evoid));
			insert_values.push_back(std::to_string(items_entry.evolvinglevel));
			insert_values.push_back(std::to_string(items_entry.evomax));
			insert_values.push_back("'" + EscapeString(items_entry.clickname) + "'");
			insert_values.push_back("'" + EscapeString(items_entry.procname) + "'");
			insert_values.push_back("'" + EscapeString(items_entry.wornname) + "'");
			insert_values.push_back("'" + EscapeString(items_entry.focusname) + "'");
			insert_values.push_back("'" + EscapeString(items_entry.scrollname) + "'");
			insert_values.push_back(std::to_string(items_entry.dsmitigation));
			insert_values.push_back(std::to_string(items_entry.heroic_str));
			insert_values.push_back(std::to_string(items_entry.heroic_int));
			insert_values.push_back(std::to_string(items_entry.heroic_wis));
			insert_values.push_back(std::to_string(items_entry.heroic_agi));
			insert_values.push_back(std::to_string(items_entry.heroic_dex));
			insert_values.push_back(std::to_string(items_entry.heroic_sta));
			insert_values.push_back(std::to_string(items_entry.heroic_cha));
			insert_values.push_back(std::to_string(items_entry.heroic_pr));
			insert_values.push_back(std::to_string(items_entry.heroic_dr));
			insert_values.push_back(std::to_string(items_entry.heroic_fr));
			insert_values.push_back(std::to_string(items_entry.heroic_cr));
			insert_values.push_back(std::to_string(items_entry.heroic_mr));
			insert_values.push_back(std::to_string(items_entry.heroic_svcorrup));
			insert_values.push_back(std::to_string(items_entry.healamt));
			insert_values.push_back(std::to_string(items_entry.spelldmg));
			insert_values.push_back(std::to_string(items_entry.clairvoyance));
			insert_values.push_back(std::to_string(items_entry.backstabdmg));
			insert_values.push_back("'" + EscapeString(items_entry.created) + "'");
			insert_values.push_back(std::to_string(items_entry.elitematerial));
			insert_values.push_back(std::to_string(items_entry.ldonsellbackrate));
			insert_values.push_back(std::to_string(items_entry.scriptfileid));
			insert_values.push_back(std::to_string(items_entry.expendablearrow));
			insert_values.push_back(std::to_string(items_entry.powersourcecapacity));
			insert_values.push_back(std::to_string(items_entry.bardeffect));
			insert_values.push_back(std::to_string(items_entry.bardeffecttype));
			insert_values.push_back(std::to_string(items_entry.bardlevel2));
			insert_values.push_back(std::to_string(items_entry.bardlevel));
			insert_values.push_back(std::to_string(items_entry.bardunk1));
			insert_values.push_back(std::to_string(items_entry.bardunk2));
			insert_values.push_back(std::to_string(items_entry.bardunk3));
			insert_values.push_back(std::to_string(items_entry.bardunk4));
			insert_values.push_back(std::to_string(items_entry.bardunk5));
			insert_values.push_back("'" + EscapeString(items_entry.bardname) + "'");
			insert_values.push_back(std::to_string(items_entry.bardunk7));
			insert_values.push_back(std::to_string(items_entry.UNK214));
			insert_values.push_back(std::to_string(items_entry.subtype));
			insert_values.push_back(std::to_string(items_entry.UNK220));
			insert_values.push_back(std::to_string(items_entry.UNK221));
			insert_values.push_back(std::to_string(items_entry.heirloom));
			insert_values.push_back(std::to_string(items_entry.UNK223));
			insert_values.push_back(std::to_string(items_entry.UNK224));
			insert_values.push_back(std::to_string(items_entry.UNK225));
			insert_values.push_back(std::to_string(items_entry.UNK226));
			insert_values.push_back(std::to_string(items_entry.UNK227));
			insert_values.push_back(std::to_string(items_entry.UNK228));
			insert_values.push_back(std::to_string(items_entry.UNK229));
			insert_values.push_back(std::to_string(items_entry.UNK230));
			insert_values.push_back(std::to_string(items_entry.UNK231));
			insert_values.push_back(std::to_string(items_entry.UNK232));
			insert_values.push_back(std::to_string(items_entry.UNK233));
			insert_values.push_back(std::to_string(items_entry.UNK234));
			insert_values.push_back(std::to_string(items_entry.placeable));
			insert_values.push_back(std::to_string(items_entry.UNK236));
			insert_values.push_back(std::to_string(items_entry.UNK237));
			insert_values.push_back(std::to_string(items_entry.UNK238));
			insert_values.push_back(std::to_string(items_entry.UNK239));
			insert_values.push_back(std::to_string(items_entry.UNK240));
			insert_values.push_back(std::to_string(items_entry.UNK241));
			insert_values.push_back(std::to_string(items_entry.epicitem));

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

	static std::vector<Items> All()
	{
		std::vector<Items> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Items entry{};

			entry.id                  = atoi(row[0]);
			entry.minstatus           = atoi(row[1]);
			entry.Name                = row[2] ? row[2] : "";
			entry.aagi                = atoi(row[3]);
			entry.ac                  = atoi(row[4]);
			entry.accuracy            = atoi(row[5]);
			entry.acha                = atoi(row[6]);
			entry.adex                = atoi(row[7]);
			entry.aint                = atoi(row[8]);
			entry.artifactflag        = atoi(row[9]);
			entry.asta                = atoi(row[10]);
			entry.astr                = atoi(row[11]);
			entry.attack              = atoi(row[12]);
			entry.augrestrict         = atoi(row[13]);
			entry.augslot1type        = atoi(row[14]);
			entry.augslot1visible     = atoi(row[15]);
			entry.augslot2type        = atoi(row[16]);
			entry.augslot2visible     = atoi(row[17]);
			entry.augslot3type        = atoi(row[18]);
			entry.augslot3visible     = atoi(row[19]);
			entry.augslot4type        = atoi(row[20]);
			entry.augslot4visible     = atoi(row[21]);
			entry.augslot5type        = atoi(row[22]);
			entry.augslot5visible     = atoi(row[23]);
			entry.augslot6type        = atoi(row[24]);
			entry.augslot6visible     = atoi(row[25]);
			entry.augtype             = atoi(row[26]);
			entry.avoidance           = atoi(row[27]);
			entry.awis                = atoi(row[28]);
			entry.bagsize             = atoi(row[29]);
			entry.bagslots            = atoi(row[30]);
			entry.bagtype             = atoi(row[31]);
			entry.bagwr               = atoi(row[32]);
			entry.banedmgamt          = atoi(row[33]);
			entry.banedmgraceamt      = atoi(row[34]);
			entry.banedmgbody         = atoi(row[35]);
			entry.banedmgrace         = atoi(row[36]);
			entry.bardtype            = atoi(row[37]);
			entry.bardvalue           = atoi(row[38]);
			entry.book                = atoi(row[39]);
			entry.casttime            = atoi(row[40]);
			entry.casttime_           = atoi(row[41]);
			entry.charmfile           = row[42] ? row[42] : "";
			entry.charmfileid         = row[43] ? row[43] : "";
			entry.classes             = atoi(row[44]);
			entry.color               = atoi(row[45]);
			entry.combateffects       = row[46] ? row[46] : "";
			entry.extradmgskill       = atoi(row[47]);
			entry.extradmgamt         = atoi(row[48]);
			entry.price               = atoi(row[49]);
			entry.cr                  = atoi(row[50]);
			entry.damage              = atoi(row[51]);
			entry.damageshield        = atoi(row[52]);
			entry.deity               = atoi(row[53]);
			entry.delay               = atoi(row[54]);
			entry.augdistiller        = atoi(row[55]);
			entry.dotshielding        = atoi(row[56]);
			entry.dr                  = atoi(row[57]);
			entry.clicktype           = atoi(row[58]);
			entry.clicklevel2         = atoi(row[59]);
			entry.elemdmgtype         = atoi(row[60]);
			entry.elemdmgamt          = atoi(row[61]);
			entry.endur               = atoi(row[62]);
			entry.factionamt1         = atoi(row[63]);
			entry.factionamt2         = atoi(row[64]);
			entry.factionamt3         = atoi(row[65]);
			entry.factionamt4         = atoi(row[66]);
			entry.factionmod1         = atoi(row[67]);
			entry.factionmod2         = atoi(row[68]);
			entry.factionmod3         = atoi(row[69]);
			entry.factionmod4         = atoi(row[70]);
			entry.filename            = row[71] ? row[71] : "";
			entry.focuseffect         = atoi(row[72]);
			entry.fr                  = atoi(row[73]);
			entry.fvnodrop            = atoi(row[74]);
			entry.haste               = atoi(row[75]);
			entry.clicklevel          = atoi(row[76]);
			entry.hp                  = atoi(row[77]);
			entry.regen               = atoi(row[78]);
			entry.icon                = atoi(row[79]);
			entry.idfile              = row[80] ? row[80] : "";
			entry.itemclass           = atoi(row[81]);
			entry.itemtype            = atoi(row[82]);
			entry.ldonprice           = atoi(row[83]);
			entry.ldontheme           = atoi(row[84]);
			entry.ldonsold            = atoi(row[85]);
			entry.light               = atoi(row[86]);
			entry.lore                = row[87] ? row[87] : "";
			entry.loregroup           = atoi(row[88]);
			entry.magic               = atoi(row[89]);
			entry.mana                = atoi(row[90]);
			entry.manaregen           = atoi(row[91]);
			entry.enduranceregen      = atoi(row[92]);
			entry.material            = atoi(row[93]);
			entry.herosforgemodel     = atoi(row[94]);
			entry.maxcharges          = atoi(row[95]);
			entry.mr                  = atoi(row[96]);
			entry.nodrop              = atoi(row[97]);
			entry.norent              = atoi(row[98]);
			entry.pendingloreflag     = atoi(row[99]);
			entry.pr                  = atoi(row[100]);
			entry.procrate            = atoi(row[101]);
			entry.races               = atoi(row[102]);
			entry.range               = atoi(row[103]);
			entry.reclevel            = atoi(row[104]);
			entry.recskill            = atoi(row[105]);
			entry.reqlevel            = atoi(row[106]);
			entry.sellrate            = static_cast<float>(atof(row[107]));
			entry.shielding           = atoi(row[108]);
			entry.size                = atoi(row[109]);
			entry.skillmodtype        = atoi(row[110]);
			entry.skillmodvalue       = atoi(row[111]);
			entry.slots               = atoi(row[112]);
			entry.clickeffect         = atoi(row[113]);
			entry.spellshield         = atoi(row[114]);
			entry.strikethrough       = atoi(row[115]);
			entry.stunresist          = atoi(row[116]);
			entry.summonedflag        = atoi(row[117]);
			entry.tradeskills         = atoi(row[118]);
			entry.favor               = atoi(row[119]);
			entry.weight              = atoi(row[120]);
			entry.UNK012              = atoi(row[121]);
			entry.UNK013              = atoi(row[122]);
			entry.benefitflag         = atoi(row[123]);
			entry.UNK054              = atoi(row[124]);
			entry.UNK059              = atoi(row[125]);
			entry.booktype            = atoi(row[126]);
			entry.recastdelay         = atoi(row[127]);
			entry.recasttype          = atoi(row[128]);
			entry.guildfavor          = atoi(row[129]);
			entry.UNK123              = atoi(row[130]);
			entry.UNK124              = atoi(row[131]);
			entry.attuneable          = atoi(row[132]);
			entry.nopet               = atoi(row[133]);
			entry.updated             = row[134] ? row[134] : "";
			entry.comment             = row[135] ? row[135] : "";
			entry.UNK127              = atoi(row[136]);
			entry.pointtype           = atoi(row[137]);
			entry.potionbelt          = atoi(row[138]);
			entry.potionbeltslots     = atoi(row[139]);
			entry.stacksize           = atoi(row[140]);
			entry.notransfer          = atoi(row[141]);
			entry.stackable           = atoi(row[142]);
			entry.UNK134              = row[143] ? row[143] : "";
			entry.UNK137              = atoi(row[144]);
			entry.proceffect          = atoi(row[145]);
			entry.proctype            = atoi(row[146]);
			entry.proclevel2          = atoi(row[147]);
			entry.proclevel           = atoi(row[148]);
			entry.UNK142              = atoi(row[149]);
			entry.worneffect          = atoi(row[150]);
			entry.worntype            = atoi(row[151]);
			entry.wornlevel2          = atoi(row[152]);
			entry.wornlevel           = atoi(row[153]);
			entry.UNK147              = atoi(row[154]);
			entry.focustype           = atoi(row[155]);
			entry.focuslevel2         = atoi(row[156]);
			entry.focuslevel          = atoi(row[157]);
			entry.UNK152              = atoi(row[158]);
			entry.scrolleffect        = atoi(row[159]);
			entry.scrolltype          = atoi(row[160]);
			entry.scrolllevel2        = atoi(row[161]);
			entry.scrolllevel         = atoi(row[162]);
			entry.UNK157              = atoi(row[163]);
			entry.serialized          = row[164] ? row[164] : "";
			entry.verified            = row[165] ? row[165] : "";
			entry.serialization       = row[166] ? row[166] : "";
			entry.source              = row[167] ? row[167] : "";
			entry.UNK033              = atoi(row[168]);
			entry.lorefile            = row[169] ? row[169] : "";
			entry.UNK014              = atoi(row[170]);
			entry.svcorruption        = atoi(row[171]);
			entry.skillmodmax         = atoi(row[172]);
			entry.UNK060              = atoi(row[173]);
			entry.augslot1unk2        = atoi(row[174]);
			entry.augslot2unk2        = atoi(row[175]);
			entry.augslot3unk2        = atoi(row[176]);
			entry.augslot4unk2        = atoi(row[177]);
			entry.augslot5unk2        = atoi(row[178]);
			entry.augslot6unk2        = atoi(row[179]);
			entry.UNK120              = atoi(row[180]);
			entry.UNK121              = atoi(row[181]);
			entry.questitemflag       = atoi(row[182]);
			entry.UNK132              = row[183] ? row[183] : "";
			entry.clickunk5           = atoi(row[184]);
			entry.clickunk6           = row[185] ? row[185] : "";
			entry.clickunk7           = atoi(row[186]);
			entry.procunk1            = atoi(row[187]);
			entry.procunk2            = atoi(row[188]);
			entry.procunk3            = atoi(row[189]);
			entry.procunk4            = atoi(row[190]);
			entry.procunk6            = row[191] ? row[191] : "";
			entry.procunk7            = atoi(row[192]);
			entry.wornunk1            = atoi(row[193]);
			entry.wornunk2            = atoi(row[194]);
			entry.wornunk3            = atoi(row[195]);
			entry.wornunk4            = atoi(row[196]);
			entry.wornunk5            = atoi(row[197]);
			entry.wornunk6            = row[198] ? row[198] : "";
			entry.wornunk7            = atoi(row[199]);
			entry.focusunk1           = atoi(row[200]);
			entry.focusunk2           = atoi(row[201]);
			entry.focusunk3           = atoi(row[202]);
			entry.focusunk4           = atoi(row[203]);
			entry.focusunk5           = atoi(row[204]);
			entry.focusunk6           = row[205] ? row[205] : "";
			entry.focusunk7           = atoi(row[206]);
			entry.scrollunk1          = atoi(row[207]);
			entry.scrollunk2          = atoi(row[208]);
			entry.scrollunk3          = atoi(row[209]);
			entry.scrollunk4          = atoi(row[210]);
			entry.scrollunk5          = atoi(row[211]);
			entry.scrollunk6          = row[212] ? row[212] : "";
			entry.scrollunk7          = atoi(row[213]);
			entry.UNK193              = atoi(row[214]);
			entry.purity              = atoi(row[215]);
			entry.evoitem             = atoi(row[216]);
			entry.evoid               = atoi(row[217]);
			entry.evolvinglevel       = atoi(row[218]);
			entry.evomax              = atoi(row[219]);
			entry.clickname           = row[220] ? row[220] : "";
			entry.procname            = row[221] ? row[221] : "";
			entry.wornname            = row[222] ? row[222] : "";
			entry.focusname           = row[223] ? row[223] : "";
			entry.scrollname          = row[224] ? row[224] : "";
			entry.dsmitigation        = atoi(row[225]);
			entry.heroic_str          = atoi(row[226]);
			entry.heroic_int          = atoi(row[227]);
			entry.heroic_wis          = atoi(row[228]);
			entry.heroic_agi          = atoi(row[229]);
			entry.heroic_dex          = atoi(row[230]);
			entry.heroic_sta          = atoi(row[231]);
			entry.heroic_cha          = atoi(row[232]);
			entry.heroic_pr           = atoi(row[233]);
			entry.heroic_dr           = atoi(row[234]);
			entry.heroic_fr           = atoi(row[235]);
			entry.heroic_cr           = atoi(row[236]);
			entry.heroic_mr           = atoi(row[237]);
			entry.heroic_svcorrup     = atoi(row[238]);
			entry.healamt             = atoi(row[239]);
			entry.spelldmg            = atoi(row[240]);
			entry.clairvoyance        = atoi(row[241]);
			entry.backstabdmg         = atoi(row[242]);
			entry.created             = row[243] ? row[243] : "";
			entry.elitematerial       = atoi(row[244]);
			entry.ldonsellbackrate    = atoi(row[245]);
			entry.scriptfileid        = atoi(row[246]);
			entry.expendablearrow     = atoi(row[247]);
			entry.powersourcecapacity = atoi(row[248]);
			entry.bardeffect          = atoi(row[249]);
			entry.bardeffecttype      = atoi(row[250]);
			entry.bardlevel2          = atoi(row[251]);
			entry.bardlevel           = atoi(row[252]);
			entry.bardunk1            = atoi(row[253]);
			entry.bardunk2            = atoi(row[254]);
			entry.bardunk3            = atoi(row[255]);
			entry.bardunk4            = atoi(row[256]);
			entry.bardunk5            = atoi(row[257]);
			entry.bardname            = row[258] ? row[258] : "";
			entry.bardunk7            = atoi(row[259]);
			entry.UNK214              = atoi(row[260]);
			entry.subtype             = atoi(row[261]);
			entry.UNK220              = atoi(row[262]);
			entry.UNK221              = atoi(row[263]);
			entry.heirloom            = atoi(row[264]);
			entry.UNK223              = atoi(row[265]);
			entry.UNK224              = atoi(row[266]);
			entry.UNK225              = atoi(row[267]);
			entry.UNK226              = atoi(row[268]);
			entry.UNK227              = atoi(row[269]);
			entry.UNK228              = atoi(row[270]);
			entry.UNK229              = atoi(row[271]);
			entry.UNK230              = atoi(row[272]);
			entry.UNK231              = atoi(row[273]);
			entry.UNK232              = atoi(row[274]);
			entry.UNK233              = atoi(row[275]);
			entry.UNK234              = atoi(row[276]);
			entry.placeable           = atoi(row[277]);
			entry.UNK236              = atoi(row[278]);
			entry.UNK237              = atoi(row[279]);
			entry.UNK238              = atoi(row[280]);
			entry.UNK239              = atoi(row[281]);
			entry.UNK240              = atoi(row[282]);
			entry.UNK241              = atoi(row[283]);
			entry.epicitem            = atoi(row[284]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Items> GetWhere(std::string where_filter)
	{
		std::vector<Items> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Items entry{};

			entry.id                  = atoi(row[0]);
			entry.minstatus           = atoi(row[1]);
			entry.Name                = row[2] ? row[2] : "";
			entry.aagi                = atoi(row[3]);
			entry.ac                  = atoi(row[4]);
			entry.accuracy            = atoi(row[5]);
			entry.acha                = atoi(row[6]);
			entry.adex                = atoi(row[7]);
			entry.aint                = atoi(row[8]);
			entry.artifactflag        = atoi(row[9]);
			entry.asta                = atoi(row[10]);
			entry.astr                = atoi(row[11]);
			entry.attack              = atoi(row[12]);
			entry.augrestrict         = atoi(row[13]);
			entry.augslot1type        = atoi(row[14]);
			entry.augslot1visible     = atoi(row[15]);
			entry.augslot2type        = atoi(row[16]);
			entry.augslot2visible     = atoi(row[17]);
			entry.augslot3type        = atoi(row[18]);
			entry.augslot3visible     = atoi(row[19]);
			entry.augslot4type        = atoi(row[20]);
			entry.augslot4visible     = atoi(row[21]);
			entry.augslot5type        = atoi(row[22]);
			entry.augslot5visible     = atoi(row[23]);
			entry.augslot6type        = atoi(row[24]);
			entry.augslot6visible     = atoi(row[25]);
			entry.augtype             = atoi(row[26]);
			entry.avoidance           = atoi(row[27]);
			entry.awis                = atoi(row[28]);
			entry.bagsize             = atoi(row[29]);
			entry.bagslots            = atoi(row[30]);
			entry.bagtype             = atoi(row[31]);
			entry.bagwr               = atoi(row[32]);
			entry.banedmgamt          = atoi(row[33]);
			entry.banedmgraceamt      = atoi(row[34]);
			entry.banedmgbody         = atoi(row[35]);
			entry.banedmgrace         = atoi(row[36]);
			entry.bardtype            = atoi(row[37]);
			entry.bardvalue           = atoi(row[38]);
			entry.book                = atoi(row[39]);
			entry.casttime            = atoi(row[40]);
			entry.casttime_           = atoi(row[41]);
			entry.charmfile           = row[42] ? row[42] : "";
			entry.charmfileid         = row[43] ? row[43] : "";
			entry.classes             = atoi(row[44]);
			entry.color               = atoi(row[45]);
			entry.combateffects       = row[46] ? row[46] : "";
			entry.extradmgskill       = atoi(row[47]);
			entry.extradmgamt         = atoi(row[48]);
			entry.price               = atoi(row[49]);
			entry.cr                  = atoi(row[50]);
			entry.damage              = atoi(row[51]);
			entry.damageshield        = atoi(row[52]);
			entry.deity               = atoi(row[53]);
			entry.delay               = atoi(row[54]);
			entry.augdistiller        = atoi(row[55]);
			entry.dotshielding        = atoi(row[56]);
			entry.dr                  = atoi(row[57]);
			entry.clicktype           = atoi(row[58]);
			entry.clicklevel2         = atoi(row[59]);
			entry.elemdmgtype         = atoi(row[60]);
			entry.elemdmgamt          = atoi(row[61]);
			entry.endur               = atoi(row[62]);
			entry.factionamt1         = atoi(row[63]);
			entry.factionamt2         = atoi(row[64]);
			entry.factionamt3         = atoi(row[65]);
			entry.factionamt4         = atoi(row[66]);
			entry.factionmod1         = atoi(row[67]);
			entry.factionmod2         = atoi(row[68]);
			entry.factionmod3         = atoi(row[69]);
			entry.factionmod4         = atoi(row[70]);
			entry.filename            = row[71] ? row[71] : "";
			entry.focuseffect         = atoi(row[72]);
			entry.fr                  = atoi(row[73]);
			entry.fvnodrop            = atoi(row[74]);
			entry.haste               = atoi(row[75]);
			entry.clicklevel          = atoi(row[76]);
			entry.hp                  = atoi(row[77]);
			entry.regen               = atoi(row[78]);
			entry.icon                = atoi(row[79]);
			entry.idfile              = row[80] ? row[80] : "";
			entry.itemclass           = atoi(row[81]);
			entry.itemtype            = atoi(row[82]);
			entry.ldonprice           = atoi(row[83]);
			entry.ldontheme           = atoi(row[84]);
			entry.ldonsold            = atoi(row[85]);
			entry.light               = atoi(row[86]);
			entry.lore                = row[87] ? row[87] : "";
			entry.loregroup           = atoi(row[88]);
			entry.magic               = atoi(row[89]);
			entry.mana                = atoi(row[90]);
			entry.manaregen           = atoi(row[91]);
			entry.enduranceregen      = atoi(row[92]);
			entry.material            = atoi(row[93]);
			entry.herosforgemodel     = atoi(row[94]);
			entry.maxcharges          = atoi(row[95]);
			entry.mr                  = atoi(row[96]);
			entry.nodrop              = atoi(row[97]);
			entry.norent              = atoi(row[98]);
			entry.pendingloreflag     = atoi(row[99]);
			entry.pr                  = atoi(row[100]);
			entry.procrate            = atoi(row[101]);
			entry.races               = atoi(row[102]);
			entry.range               = atoi(row[103]);
			entry.reclevel            = atoi(row[104]);
			entry.recskill            = atoi(row[105]);
			entry.reqlevel            = atoi(row[106]);
			entry.sellrate            = static_cast<float>(atof(row[107]));
			entry.shielding           = atoi(row[108]);
			entry.size                = atoi(row[109]);
			entry.skillmodtype        = atoi(row[110]);
			entry.skillmodvalue       = atoi(row[111]);
			entry.slots               = atoi(row[112]);
			entry.clickeffect         = atoi(row[113]);
			entry.spellshield         = atoi(row[114]);
			entry.strikethrough       = atoi(row[115]);
			entry.stunresist          = atoi(row[116]);
			entry.summonedflag        = atoi(row[117]);
			entry.tradeskills         = atoi(row[118]);
			entry.favor               = atoi(row[119]);
			entry.weight              = atoi(row[120]);
			entry.UNK012              = atoi(row[121]);
			entry.UNK013              = atoi(row[122]);
			entry.benefitflag         = atoi(row[123]);
			entry.UNK054              = atoi(row[124]);
			entry.UNK059              = atoi(row[125]);
			entry.booktype            = atoi(row[126]);
			entry.recastdelay         = atoi(row[127]);
			entry.recasttype          = atoi(row[128]);
			entry.guildfavor          = atoi(row[129]);
			entry.UNK123              = atoi(row[130]);
			entry.UNK124              = atoi(row[131]);
			entry.attuneable          = atoi(row[132]);
			entry.nopet               = atoi(row[133]);
			entry.updated             = row[134] ? row[134] : "";
			entry.comment             = row[135] ? row[135] : "";
			entry.UNK127              = atoi(row[136]);
			entry.pointtype           = atoi(row[137]);
			entry.potionbelt          = atoi(row[138]);
			entry.potionbeltslots     = atoi(row[139]);
			entry.stacksize           = atoi(row[140]);
			entry.notransfer          = atoi(row[141]);
			entry.stackable           = atoi(row[142]);
			entry.UNK134              = row[143] ? row[143] : "";
			entry.UNK137              = atoi(row[144]);
			entry.proceffect          = atoi(row[145]);
			entry.proctype            = atoi(row[146]);
			entry.proclevel2          = atoi(row[147]);
			entry.proclevel           = atoi(row[148]);
			entry.UNK142              = atoi(row[149]);
			entry.worneffect          = atoi(row[150]);
			entry.worntype            = atoi(row[151]);
			entry.wornlevel2          = atoi(row[152]);
			entry.wornlevel           = atoi(row[153]);
			entry.UNK147              = atoi(row[154]);
			entry.focustype           = atoi(row[155]);
			entry.focuslevel2         = atoi(row[156]);
			entry.focuslevel          = atoi(row[157]);
			entry.UNK152              = atoi(row[158]);
			entry.scrolleffect        = atoi(row[159]);
			entry.scrolltype          = atoi(row[160]);
			entry.scrolllevel2        = atoi(row[161]);
			entry.scrolllevel         = atoi(row[162]);
			entry.UNK157              = atoi(row[163]);
			entry.serialized          = row[164] ? row[164] : "";
			entry.verified            = row[165] ? row[165] : "";
			entry.serialization       = row[166] ? row[166] : "";
			entry.source              = row[167] ? row[167] : "";
			entry.UNK033              = atoi(row[168]);
			entry.lorefile            = row[169] ? row[169] : "";
			entry.UNK014              = atoi(row[170]);
			entry.svcorruption        = atoi(row[171]);
			entry.skillmodmax         = atoi(row[172]);
			entry.UNK060              = atoi(row[173]);
			entry.augslot1unk2        = atoi(row[174]);
			entry.augslot2unk2        = atoi(row[175]);
			entry.augslot3unk2        = atoi(row[176]);
			entry.augslot4unk2        = atoi(row[177]);
			entry.augslot5unk2        = atoi(row[178]);
			entry.augslot6unk2        = atoi(row[179]);
			entry.UNK120              = atoi(row[180]);
			entry.UNK121              = atoi(row[181]);
			entry.questitemflag       = atoi(row[182]);
			entry.UNK132              = row[183] ? row[183] : "";
			entry.clickunk5           = atoi(row[184]);
			entry.clickunk6           = row[185] ? row[185] : "";
			entry.clickunk7           = atoi(row[186]);
			entry.procunk1            = atoi(row[187]);
			entry.procunk2            = atoi(row[188]);
			entry.procunk3            = atoi(row[189]);
			entry.procunk4            = atoi(row[190]);
			entry.procunk6            = row[191] ? row[191] : "";
			entry.procunk7            = atoi(row[192]);
			entry.wornunk1            = atoi(row[193]);
			entry.wornunk2            = atoi(row[194]);
			entry.wornunk3            = atoi(row[195]);
			entry.wornunk4            = atoi(row[196]);
			entry.wornunk5            = atoi(row[197]);
			entry.wornunk6            = row[198] ? row[198] : "";
			entry.wornunk7            = atoi(row[199]);
			entry.focusunk1           = atoi(row[200]);
			entry.focusunk2           = atoi(row[201]);
			entry.focusunk3           = atoi(row[202]);
			entry.focusunk4           = atoi(row[203]);
			entry.focusunk5           = atoi(row[204]);
			entry.focusunk6           = row[205] ? row[205] : "";
			entry.focusunk7           = atoi(row[206]);
			entry.scrollunk1          = atoi(row[207]);
			entry.scrollunk2          = atoi(row[208]);
			entry.scrollunk3          = atoi(row[209]);
			entry.scrollunk4          = atoi(row[210]);
			entry.scrollunk5          = atoi(row[211]);
			entry.scrollunk6          = row[212] ? row[212] : "";
			entry.scrollunk7          = atoi(row[213]);
			entry.UNK193              = atoi(row[214]);
			entry.purity              = atoi(row[215]);
			entry.evoitem             = atoi(row[216]);
			entry.evoid               = atoi(row[217]);
			entry.evolvinglevel       = atoi(row[218]);
			entry.evomax              = atoi(row[219]);
			entry.clickname           = row[220] ? row[220] : "";
			entry.procname            = row[221] ? row[221] : "";
			entry.wornname            = row[222] ? row[222] : "";
			entry.focusname           = row[223] ? row[223] : "";
			entry.scrollname          = row[224] ? row[224] : "";
			entry.dsmitigation        = atoi(row[225]);
			entry.heroic_str          = atoi(row[226]);
			entry.heroic_int          = atoi(row[227]);
			entry.heroic_wis          = atoi(row[228]);
			entry.heroic_agi          = atoi(row[229]);
			entry.heroic_dex          = atoi(row[230]);
			entry.heroic_sta          = atoi(row[231]);
			entry.heroic_cha          = atoi(row[232]);
			entry.heroic_pr           = atoi(row[233]);
			entry.heroic_dr           = atoi(row[234]);
			entry.heroic_fr           = atoi(row[235]);
			entry.heroic_cr           = atoi(row[236]);
			entry.heroic_mr           = atoi(row[237]);
			entry.heroic_svcorrup     = atoi(row[238]);
			entry.healamt             = atoi(row[239]);
			entry.spelldmg            = atoi(row[240]);
			entry.clairvoyance        = atoi(row[241]);
			entry.backstabdmg         = atoi(row[242]);
			entry.created             = row[243] ? row[243] : "";
			entry.elitematerial       = atoi(row[244]);
			entry.ldonsellbackrate    = atoi(row[245]);
			entry.scriptfileid        = atoi(row[246]);
			entry.expendablearrow     = atoi(row[247]);
			entry.powersourcecapacity = atoi(row[248]);
			entry.bardeffect          = atoi(row[249]);
			entry.bardeffecttype      = atoi(row[250]);
			entry.bardlevel2          = atoi(row[251]);
			entry.bardlevel           = atoi(row[252]);
			entry.bardunk1            = atoi(row[253]);
			entry.bardunk2            = atoi(row[254]);
			entry.bardunk3            = atoi(row[255]);
			entry.bardunk4            = atoi(row[256]);
			entry.bardunk5            = atoi(row[257]);
			entry.bardname            = row[258] ? row[258] : "";
			entry.bardunk7            = atoi(row[259]);
			entry.UNK214              = atoi(row[260]);
			entry.subtype             = atoi(row[261]);
			entry.UNK220              = atoi(row[262]);
			entry.UNK221              = atoi(row[263]);
			entry.heirloom            = atoi(row[264]);
			entry.UNK223              = atoi(row[265]);
			entry.UNK224              = atoi(row[266]);
			entry.UNK225              = atoi(row[267]);
			entry.UNK226              = atoi(row[268]);
			entry.UNK227              = atoi(row[269]);
			entry.UNK228              = atoi(row[270]);
			entry.UNK229              = atoi(row[271]);
			entry.UNK230              = atoi(row[272]);
			entry.UNK231              = atoi(row[273]);
			entry.UNK232              = atoi(row[274]);
			entry.UNK233              = atoi(row[275]);
			entry.UNK234              = atoi(row[276]);
			entry.placeable           = atoi(row[277]);
			entry.UNK236              = atoi(row[278]);
			entry.UNK237              = atoi(row[279]);
			entry.UNK238              = atoi(row[280]);
			entry.UNK239              = atoi(row[281]);
			entry.UNK240              = atoi(row[282]);
			entry.UNK241              = atoi(row[283]);
			entry.epicitem            = atoi(row[284]);

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
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate()
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_ITEMS_REPOSITORY_H
