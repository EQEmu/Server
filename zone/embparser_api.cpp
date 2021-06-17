/*  EQEMu:  Everquest Server Emulator
	Copyright (C) 2001-2006  EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "../common/features.h"
#include "../common/content/world_content_service.h"

#ifdef EMBPERL
#ifdef EMBPERL_XS

#include "../common/global_define.h"
#include "../common/misc_functions.h"
#include "../common/eqemu_logsys.h"

#include "embparser.h"
#include "embxs.h"
#include "entity.h"
#include "expedition.h"
#include "queryserv.h"
#include "questmgr.h"
#include "zone.h"
#include "data_bucket.h"

#include <cctype>

extern Zone      *zone;
extern QueryServ *QServ;

/*

Some useful perl API info:

SvUV == string to unsigned value (char->ulong)
SvIV == string to signed value (char->long)
SvNV == string to real value (float,double)
SvPV_nolen == string with no length restriction

*/

#ifdef EMBPERL_XS_CLASSES

//Any creation of new Client objects gets the current quest Client
XS(XS_Client_new);
XS(XS_Client_new) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::Client::new()");
	{
		Client *RETVAL;

		RETVAL = quest_manager.GetInitiator();
		ST(0) = sv_newmortal();
		if (RETVAL)
			sv_setref_pv(ST(0), "Client", (void *) RETVAL);
	}
	XSRETURN(1);
}

//Any creation of new NPC objects gets the current quest NPC
XS(XS_NPC_new);
XS(XS_NPC_new) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::NPC::new()");
	{
		NPC *RETVAL;

		RETVAL = quest_manager.GetNPC();
		ST(0) = sv_newmortal();
		if (RETVAL)
			sv_setref_pv(ST(0), "NPC", (void *) RETVAL);
	}
	XSRETURN(1);
}

//Any creation of new NPC objects gets the current quest NPC
XS(XS_EntityList_new);
XS(XS_EntityList_new) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::EntityList::new()");
	{
		EntityList *RETVAL;

		RETVAL = &entity_list;
		ST(0) = sv_newmortal();
		if (RETVAL)
			sv_setref_pv(ST(0), "EntityList", (void *) RETVAL);
	}
	XSRETURN(1);
}

//Any creation of new inventory gets the curreny inventory
XS(XS_Inventory_new);
XS(XS_Inventory_new) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::Inventory::new()");

	EQ::InventoryProfile* RETVAL;

	RETVAL = quest_manager.GetInventory();
	ST(0) = sv_newmortal();
	if (RETVAL)
		sv_setref_pv(ST(0), "Inventory", (void *) RETVAL);

	XSRETURN(1);
}

//Any creation of new quest items gets the current quest item
XS(XS_QuestItem_new);
XS(XS_QuestItem_new) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::QuestItem::new()");

	EQ::ItemInstance *RETVAL;

	RETVAL = quest_manager.GetQuestItem();
	ST(0) = sv_newmortal();
	if (RETVAL)
		sv_setref_pv(ST(0), "QuestItem", (void *) RETVAL);

	XSRETURN(1);
}

//Any creation of new quest items gets the current quest item
XS(XS_MobList_new);
XS(XS_MobList_new) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::MobList::new()");

	ListElement<Mob *> *RETVAL;

	RETVAL = nullptr;
	ST(0) = sv_newmortal();
	if (RETVAL)
		sv_setref_pv(ST(0), "MobList", (void *) RETVAL);

	XSRETURN(1);
}

#endif //EMBPERL_XS_CLASSES


XS(XS__echo); // prototype to pass -Wmissing-prototypes
XS(XS__echo) {
	dXSARGS;

	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::echo(int emote_color_id, string message)");

	quest_manager.echo(SvUV(ST(0)), SvPV_nolen(ST(1)));

	XSRETURN_EMPTY;
}

XS(XS__say); // prototype to pass -Wmissing-prototypes
XS(XS__say) {
	dXSARGS;

	Journal::Options opts;
	// we currently default to these
	opts.speak_mode = Journal::SpeakMode::Say;
	opts.journal_mode = Journal::Mode::Log2;
	opts.language = 0;
	opts.message_type = Chat::NPCQuestSay;
	if (items == 0 || items > 5) {
		Perl_croak(aTHX_ "Usage: quest::say(string message, [int language_id], [int message_type], [int speak_mode], [int journal_mode])");
	} else if (items == 2) {
		opts.language = (int)SvIV(ST(1));
	} else if (items == 3) {
		opts.language = (int)SvIV(ST(1));
		opts.message_type = (int)SvIV(ST(2));
	} else if (items == 4) {
		opts.language = (int)SvIV(ST(1));
		opts.message_type = (int)SvIV(ST(2));
		opts.speak_mode = (Journal::SpeakMode)SvIV(ST(3));
	} else if (items == 5) {
		opts.language = (int)SvIV(ST(1));
		opts.message_type = (int)SvIV(ST(2));
		opts.speak_mode = (Journal::SpeakMode)SvIV(ST(3));
		opts.journal_mode = (Journal::Mode)SvIV(ST(4));
	}

	quest_manager.say(SvPV_nolen(ST(0)), opts);

	XSRETURN_EMPTY;
}

XS(XS__me); // prototype to pass -Wmissing-prototypes
XS(XS__me) {
	dXSARGS;

	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::me(string message)");

	quest_manager.me(SvPV_nolen(ST(0)));

	XSRETURN_EMPTY;
}

XS(XS__summonitem); // prototype to pass -Wmissing-prototypes
XS(XS__summonitem) {
	dXSARGS;
	if (items == 1)
		quest_manager.summonitem(SvUV(ST(0)));
	else if (items == 2)
		quest_manager.summonitem(SvUV(ST(0)), SvUV(ST(1)));
	else
		Perl_croak(aTHX_ "Usage: quest::summonitem(int item_id, [int charges])");
	XSRETURN_EMPTY;
}

XS(XS__write);
XS(XS__write) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::write(string file_name, string message)");

	char *file    = (char *) SvPV_nolen(ST(0));
	char *message = (char *) SvPV_nolen(ST(1));

	quest_manager.write(file, message);

	XSRETURN_EMPTY;
}

XS(XS__spawn);
XS(XS__spawn) {
	dXSARGS;
	if (items != 6)
		Perl_croak(aTHX_ "Usage: quest::spawn(int npc_type_id, int grid_id, int int_unused, float x, float y, float z)");

	uint16 RETVAL;
	dXSTARG;

	int  npc_type_id = (int) SvIV(ST(0));
	int  grid_id     = (int) SvIV(ST(1));
	int  int_unused  = (int) SvIV(ST(2));
	auto position    = glm::vec4((float) SvNV(ST(3)), (float) SvNV(ST(4)), (float) SvNV(ST(5)), 0.0f);

	Mob *r = quest_manager.spawn2(npc_type_id, grid_id, int_unused, position);
	RETVAL = (r != nullptr) ? r->GetID() : 0;
	XSprePUSH;
	PUSHu((UV) RETVAL);

	XSRETURN(1);
}

XS(XS__spawn2);
XS(XS__spawn2) {
	dXSARGS;
	if (items != 7)
		Perl_croak(aTHX_ "Usage: quest::spawn2(int npc_type_id, int grid_id, int int_unused, float x, float y, float z, float heading)");

	uint16 RETVAL;
	dXSTARG;

	int  npc_type_id = (int) SvIV(ST(0));
	int  grid_id     = (int) SvIV(ST(1));
	int  int_unused  = (int) SvIV(ST(2));
	auto position    = glm::vec4((float) SvNV(ST(3)), (float) SvNV(ST(4)), (float) SvNV(ST(5)), (float) SvNV(ST(6)));

	Mob *r = quest_manager.spawn2(npc_type_id, grid_id, int_unused, position);
	RETVAL = (r != nullptr) ? r->GetID() : 0;
	XSprePUSH;
	PUSHu((UV) RETVAL);

	XSRETURN(1);
}

XS(XS__unique_spawn);
XS(XS__unique_spawn) {
	dXSARGS;
	if (items != 6 && items != 7)
		Perl_croak(aTHX_ "Usage: quest::unique_spawn(int npc_type_id, int grid_id, int int_unused, float x, float y, float z, [float heading])");

	uint16 RETVAL;
	dXSTARG;

	int   npc_type_id = (int) SvIV(ST(0));
	int   grid_id     = (int) SvIV(ST(1));
	int   int_unused  = (int) SvIV(ST(2));
	float x           = (float) SvNV(ST(3));
	float y           = (float) SvNV(ST(4));
	float z           = (float) SvNV(ST(5));
	float heading     = 0;
	if (items == 7)
		heading = (float) SvNV(ST(6));

	Mob *r = quest_manager.unique_spawn(npc_type_id, grid_id, int_unused, glm::vec4(x, y, z, heading));
	RETVAL = (r != nullptr) ? r->GetID() : 0;

	XSprePUSH;
	PUSHu((UV) RETVAL);

	XSRETURN(1);
}

XS(XS__spawn_from_spawn2);
XS(XS__spawn_from_spawn2) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::spawn_from_spawn2(int spawn2_id)");

	uint16 RETVAL;
	dXSTARG;

	int spawn2_id = (int) SvIV(ST(0));

	Mob *r = quest_manager.spawn_from_spawn2(spawn2_id);
	RETVAL = (r != nullptr) ? r->GetID() : 0;

	XSprePUSH;
	PUSHu((UV) RETVAL);

	XSRETURN(1);
}

XS(XS__enable_spawn2);
XS(XS__enable_spawn2) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::enable_spawn2(int spawn2_id)");

	int spawn2_id = (int) SvIV(ST(0));

	quest_manager.enable_spawn2(spawn2_id);
	XSRETURN_EMPTY;
}

XS(XS__disable_spawn2);
XS(XS__disable_spawn2) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::disable_spawn2(int spawn2_id)");

	int spawn2_id = (int) SvIV(ST(0));

	quest_manager.disable_spawn2(spawn2_id);
	XSRETURN_EMPTY;
}

XS(XS__setstat);
XS(XS__setstat) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::setstat(stat_id, int_value)");

	int stat_id   = (int) SvIV(ST(0));
	int int_value = (int) SvIV(ST(1));

	quest_manager.setstat(stat_id, int_value);

	XSRETURN_EMPTY;
}

XS(XS__incstat);  //old setstat command aza
XS(XS__incstat) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::incstat(int stat_id, int value)");

	int stat_id   = (int) SvIV(ST(0));
	int int_value = (int) SvIV(ST(1));

	quest_manager.incstat(stat_id, int_value);

	XSRETURN_EMPTY;
}

XS(XS__getinventoryslotid);
XS(XS__getinventoryslotid) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::getinventoryslotid(string identifier)");

	int16 RETVAL = EQ::invslot::SLOT_INVALID;
	dXSTARG;

	std::string identifier = (Const_char *)SvPV_nolen(ST(0));
	for (std::string::size_type i = 0; i < identifier.length(); ++i)
		identifier[i] = std::tolower(identifier[i]);

	if (identifier.find('.') == std::string::npos) {
		if (identifier == "invalid")                    RETVAL = EQ::invslot::SLOT_INVALID;
		else if (identifier == "charm")                 RETVAL = EQ::invslot::slotCharm;
		else if (identifier == "ear1")                  RETVAL = EQ::invslot::slotEar1;
		else if (identifier == "head")                  RETVAL = EQ::invslot::slotHead;
		else if (identifier == "face")                  RETVAL = EQ::invslot::slotFace;
		else if (identifier == "ear2")                  RETVAL = EQ::invslot::slotEar2;
		else if (identifier == "neck")                  RETVAL = EQ::invslot::slotNeck;
		else if (identifier == "shoulders")             RETVAL = EQ::invslot::slotShoulders;
		else if (identifier == "arms")                  RETVAL = EQ::invslot::slotArms;
		else if (identifier == "back")                  RETVAL = EQ::invslot::slotBack;
		else if (identifier == "wrist1")                RETVAL = EQ::invslot::slotWrist1;
		else if (identifier == "wrist2")                RETVAL = EQ::invslot::slotWrist2;
		else if (identifier == "range")                 RETVAL = EQ::invslot::slotRange;
		else if (identifier == "hands")                 RETVAL = EQ::invslot::slotHands;
		else if (identifier == "primary")               RETVAL = EQ::invslot::slotPrimary;
		else if (identifier == "secondary")             RETVAL = EQ::invslot::slotSecondary;
		else if (identifier == "finger1")               RETVAL = EQ::invslot::slotFinger1;
		else if (identifier == "finger2")               RETVAL = EQ::invslot::slotFinger2;
		else if (identifier == "chest")                 RETVAL = EQ::invslot::slotChest;
		else if (identifier == "legs")                  RETVAL = EQ::invslot::slotLegs;
		else if (identifier == "feet")                  RETVAL = EQ::invslot::slotFeet;
		else if (identifier == "waist")                 RETVAL = EQ::invslot::slotWaist;
		else if (identifier == "powersource")           RETVAL = EQ::invslot::slotPowerSource;
		else if (identifier == "ammo")                  RETVAL = EQ::invslot::slotAmmo;
		else if (identifier == "general1")              RETVAL = EQ::invslot::slotGeneral1;
		else if (identifier == "general2")              RETVAL = EQ::invslot::slotGeneral2;
		else if (identifier == "general3")              RETVAL = EQ::invslot::slotGeneral3;
		else if (identifier == "general4")              RETVAL = EQ::invslot::slotGeneral4;
		else if (identifier == "general5")              RETVAL = EQ::invslot::slotGeneral5;
		else if (identifier == "general6")              RETVAL = EQ::invslot::slotGeneral6;
		else if (identifier == "general7")              RETVAL = EQ::invslot::slotGeneral7;
		else if (identifier == "general8")              RETVAL = EQ::invslot::slotGeneral8;
		else if (identifier == "general9")              RETVAL = EQ::invslot::slotGeneral9;
		else if (identifier == "general10")             RETVAL = EQ::invslot::slotGeneral10;
		else if (identifier == "cursor")                RETVAL = EQ::invslot::slotCursor;
		else if (identifier == "tradeskill")            RETVAL = EQ::invslot::SLOT_TRADESKILL_EXPERIMENT_COMBINE;
		else if (identifier == "augment")               RETVAL = EQ::invslot::SLOT_AUGMENT_GENERIC_RETURN;
	}
	else {
		if (identifier == "possessions.begin")          RETVAL = EQ::invslot::POSSESSIONS_BEGIN;
		else if (identifier == "possessions.end")       RETVAL = EQ::invslot::POSSESSIONS_END;
		else if (identifier == "equipment.begin")       RETVAL = EQ::invslot::EQUIPMENT_BEGIN;
		else if (identifier == "equipment.end")         RETVAL = EQ::invslot::EQUIPMENT_END;
		else if (identifier == "general.begin")         RETVAL = EQ::invslot::GENERAL_BEGIN;
		else if (identifier == "general.end")           RETVAL = EQ::invslot::GENERAL_END;
		else if (identifier == "possessionsbags.begin") RETVAL = EQ::invbag::GENERAL_BAGS_BEGIN;
		else if (identifier == "possessionsbags.end")   RETVAL = EQ::invbag::CURSOR_BAG_END;
		else if (identifier == "generalbags.begin")     RETVAL = EQ::invbag::GENERAL_BAGS_BEGIN;
		else if (identifier == "generalbags.end")       RETVAL = EQ::invbag::GENERAL_BAGS_END;
		else if (identifier == "general1bag.begin")     RETVAL = EQ::invbag::GENERAL_BAGS_BEGIN;
		else if (identifier == "general1bag.end")       RETVAL = EQ::invbag::GENERAL_BAGS_BEGIN + 9;
		else if (identifier == "general2bag.begin")     RETVAL = EQ::invbag::GENERAL_BAGS_BEGIN + 10;
		else if (identifier == "general2bag.end")       RETVAL = EQ::invbag::GENERAL_BAGS_BEGIN + 19;
		else if (identifier == "general3bag.begin")     RETVAL = EQ::invbag::GENERAL_BAGS_BEGIN + 20;
		else if (identifier == "general3bag.end")       RETVAL = EQ::invbag::GENERAL_BAGS_BEGIN + 29;
		else if (identifier == "general4bag.begin")     RETVAL = EQ::invbag::GENERAL_BAGS_BEGIN + 30;
		else if (identifier == "general4bag.end")       RETVAL = EQ::invbag::GENERAL_BAGS_BEGIN + 39;
		else if (identifier == "general5bag.begin")     RETVAL = EQ::invbag::GENERAL_BAGS_BEGIN + 40;
		else if (identifier == "general5bag.end")       RETVAL = EQ::invbag::GENERAL_BAGS_BEGIN + 49;
		else if (identifier == "general6bag.begin")     RETVAL = EQ::invbag::GENERAL_BAGS_BEGIN + 50;
		else if (identifier == "general6bag.end")       RETVAL = EQ::invbag::GENERAL_BAGS_BEGIN + 59;
		else if (identifier == "general7bag.begin")     RETVAL = EQ::invbag::GENERAL_BAGS_BEGIN + 60;
		else if (identifier == "general7bag.end")       RETVAL = EQ::invbag::GENERAL_BAGS_BEGIN + 69;
		else if (identifier == "general8bag.begin")     RETVAL = EQ::invbag::GENERAL_BAGS_BEGIN + 70;
		else if (identifier == "general8bag.end")       RETVAL = EQ::invbag::GENERAL_BAGS_BEGIN + 79;
		else if (identifier == "general9bag.begin")     RETVAL = EQ::invbag::GENERAL_BAGS_BEGIN + 80;
		else if (identifier == "general9bag.end")       RETVAL = EQ::invbag::GENERAL_BAGS_BEGIN + 89;
		else if (identifier == "general10bag.begin")    RETVAL = EQ::invbag::GENERAL_BAGS_BEGIN + 90;
		else if (identifier == "general10bag.end")      RETVAL = EQ::invbag::GENERAL_BAGS_BEGIN + 99;
		else if (identifier == "cursorbag.begin")       RETVAL = EQ::invbag::CURSOR_BAG_BEGIN;
		else if (identifier == "cursorbag.end")         RETVAL = EQ::invbag::CURSOR_BAG_END;
		else if (identifier == "bank.begin")            RETVAL = EQ::invslot::BANK_BEGIN;
		else if (identifier == "bank.end")              RETVAL = EQ::invslot::BANK_END;
		else if (identifier == "bankbags.begin")        RETVAL = EQ::invbag::BANK_BAGS_BEGIN;
		else if (identifier == "bankbags.end")          RETVAL = EQ::invbag::BANK_BAGS_END;
		else if (identifier == "sharedbank.begin")      RETVAL = EQ::invslot::SHARED_BANK_BEGIN;
		else if (identifier == "sharedbank.end")        RETVAL = EQ::invslot::SHARED_BANK_END;
		else if (identifier == "sharedbankbags.begin")  RETVAL = EQ::invbag::SHARED_BANK_BAGS_BEGIN;
		else if (identifier == "sharedbankbags.end")    RETVAL = EQ::invbag::SHARED_BANK_BAGS_END;
		else if (identifier == "bagslot.begin")         RETVAL = EQ::invbag::SLOT_BEGIN;
		else if (identifier == "bagslot.end")           RETVAL = EQ::invbag::SLOT_END;
		else if (identifier == "augsocket.begin")       RETVAL = EQ::invaug::SOCKET_BEGIN;
		else if (identifier == "augsocket.end")         RETVAL = EQ::invaug::SOCKET_END;
	}

	XSprePUSH; PUSHu((IV)RETVAL);

	XSRETURN(1);
}

XS(XS__castspell);
XS(XS__castspell) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::castspell(int spell_id, int target_id)");

	int spell_id  = (int) SvIV(ST(0));
	int target_id = (int) SvIV(ST(1));

	quest_manager.castspell(spell_id, target_id);

	XSRETURN_EMPTY;
}

XS(XS__selfcast);
XS(XS__selfcast) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::selfcast(int spell_id)");

	int spell_id = (int) SvIV(ST(0));

	quest_manager.selfcast(spell_id);

	XSRETURN_EMPTY;
}

XS(XS__addloot);
XS(XS__addloot) {
	dXSARGS;
	if (items < 1 || items > 3)
		Perl_croak(aTHX_ "Usage: quest::addloot(uint32 item_id, uint16 charges = 0, [bool equip_item = true])");

	uint32 item_id   = (uint32) SvUV(ST(0));
	uint16 charges   = 0;
	bool   equipitem = true;

	if (items > 1)
		charges   = (uint16) SvUV(ST(1));
	if (items > 2)
		equipitem = (bool) SvTRUE(ST(2));

	quest_manager.addloot(item_id, charges, equipitem);

	XSRETURN_EMPTY;
}

XS(XS__zone);
XS(XS__zone) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::zone(string zone_name)");

	char *zone_name = (char *) SvPV_nolen(ST(0));

	quest_manager.Zone(zone_name);

	XSRETURN_EMPTY;
}

XS(XS__zonegroup);
XS(XS__zonegroup) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::zonegroup(string zone_name)");

	char *zone_name = (char *) SvPV_nolen(ST(0));

	quest_manager.ZoneGroup(zone_name);

	XSRETURN_EMPTY;
}

XS(XS__zoneraid);
XS(XS__zoneraid) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::zoneraid(string zone_name)");

	char *zone_name = (char *) SvPV_nolen(ST(0));

	quest_manager.ZoneRaid(zone_name);

	XSRETURN_EMPTY;
}

XS(XS__settimer);
XS(XS__settimer) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::settimer(string timer_name, int seconds)");

	char *timer_name = (char *) SvPV_nolen(ST(0));
	int  seconds     = (int) SvIV(ST(1));

	quest_manager.settimer(timer_name, seconds);

	XSRETURN_EMPTY;
}

XS(XS__settimerMS);
XS(XS__settimerMS) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::settimerMS(string timer_name, int milliseconds)");

	char *timer_name  = (char *) SvPV_nolen(ST(0));
	int  milliseconds = (int) SvIV(ST(1));

	quest_manager.settimerMS(timer_name, milliseconds);

	XSRETURN_EMPTY;
}

XS(XS__stoptimer);
XS(XS__stoptimer) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::stoptimer(string timer_name)");

	char *timer_name = (char *) SvPV_nolen(ST(0));

	quest_manager.stoptimer(timer_name);

	XSRETURN_EMPTY;
}

XS(XS__stopalltimers);
XS(XS__stopalltimers) {
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: quest::stopalltimers()");

	quest_manager.stopalltimers();

	XSRETURN_EMPTY;
}

XS(XS__emote);
XS(XS__emote) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::emote(string message)");

	char *message = (char *) SvPV_nolen(ST(0));

	quest_manager.emote(message);

	XSRETURN_EMPTY;
}

XS(XS__shout);
XS(XS__shout) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::shout(string message)");

	char *message = (char *) SvPV_nolen(ST(0));

	quest_manager.shout(message);

	XSRETURN_EMPTY;
}

XS(XS__shout2);
XS(XS__shout2) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::shout2(string message)");

	char *message = (char *) SvPV_nolen(ST(0));

	quest_manager.shout2(message);

	XSRETURN_EMPTY;
}

XS(XS__gmsay);
XS(XS__gmsay) {
	dXSARGS;
	if ((items < 1) || (items > 5))
		Perl_croak(aTHX_ "Usage: quest::gmsay(string message, [int color_id], [bool send_to_world = 0])");

	char   *message      = (char *) SvPV_nolen(ST(0));
	int    color_id      = 7;
	bool   send_to_world = 0;
	uint32 to_guilddbid  = 0;
	uint16 to_minstatus  = 80;

	if (items > 1) {
		color_id = (int) SvIV(ST(1));
	}

	if (items > 2) {
		send_to_world = ((int) SvIV(ST(2))) == 0 ? false : true;
	}

	if (items > 3)
		to_guilddbid = (int) SvUV(ST(3));

	if (items > 4)
		to_minstatus = (int) SvUV(ST(4));

	quest_manager.gmsay(message, color_id, send_to_world, to_guilddbid, to_minstatus);

	XSRETURN_EMPTY;
}

XS(XS__depop);
XS(XS__depop) {
	dXSARGS;
	if (items < 0 || items > 1)
		Perl_croak(aTHX_ "Usage: quest::depop(int npc_type_id = 0)");

	int npc_type_id;

	if (items < 1)
		npc_type_id = 0;
	else
		npc_type_id = (int) SvIV(ST(0));


	quest_manager.depop(npc_type_id);

	XSRETURN_EMPTY;
}

XS(XS__depop_withtimer);
XS(XS__depop_withtimer) {
	dXSARGS;
	if (items < 0 || items > 1)
		Perl_croak(aTHX_ "Usage: quest::depop_withtimer(int npc_type_id = 0)");

	int npc_type_id;

	if (items < 1)
		npc_type_id = 0;
	else
		npc_type_id = (int) SvIV(ST(0));


	quest_manager.depop_withtimer(npc_type_id);

	XSRETURN_EMPTY;
}

XS(XS__depopall);
XS(XS__depopall) {
	dXSARGS;
	if (items < 0 || items > 1)
		Perl_croak(aTHX_ "Usage: quest::depopall(int npc_type_id = 0)");

	int npc_type_id;

	if (items < 1)
		npc_type_id = 0;
	else
		npc_type_id = (int) SvIV(ST(0));


	quest_manager.depopall(npc_type_id);

	XSRETURN_EMPTY;
}

XS(XS__settarget);
XS(XS__settarget) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::settarget(string target_enum ['npc_type', 'entity'], int target_id)");

	char *target_enum = (char *) SvPV_nolen(ST(0));
	int  target_id    = (int) SvIV(ST(1));

	quest_manager.settarget(target_enum, target_id);

	XSRETURN_EMPTY;
}

XS(XS__follow);
XS(XS__follow) {
	dXSARGS;
	if (items != 1 && items != 2)
		Perl_croak(aTHX_ "Usage: quest::follow(int entity_id, [int distance = 10])");

	int entity_id = (int) SvIV(ST(0));
	int distance;

	if (items == 2)
		distance = (int) SvIV(ST(1));
	else
		distance = 10;

	quest_manager.follow(entity_id, distance);

	XSRETURN_EMPTY;
}

XS(XS__sfollow);
XS(XS__sfollow) {
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: quest::sfollow()");


	quest_manager.sfollow();

	XSRETURN_EMPTY;
}

XS(XS__changedeity);
XS(XS__changedeity) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::changedeity(int deity_id)");

	int deity_id = (int) SvIV(ST(0));

	quest_manager.changedeity(deity_id);

	XSRETURN_EMPTY;
}

XS(XS__exp);
XS(XS__exp) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::exp(int amount)");

	int amt = (int) SvIV(ST(0));

	quest_manager.exp(amt);

	XSRETURN_EMPTY;
}

XS(XS__level);
XS(XS__level) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::level(int new_level)");

	int newlevel = (int) SvIV(ST(0));

	quest_manager.level(newlevel);

	XSRETURN_EMPTY;
}

XS(XS__traindisc);
XS(XS__traindisc) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::traindisc(int tome_item_id)");

	int discipline_tome_item_id = (int) SvIV(ST(0));

	quest_manager.traindisc(discipline_tome_item_id);

	XSRETURN_EMPTY;
}

XS(XS__isdisctome);
XS(XS__isdisctome) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::isdisctome(int item_id)");

	bool RETVAL;
	int  item_id = (int) SvIV(ST(0));

	RETVAL = quest_manager.isdisctome(item_id);

	ST(0)        = boolSV(RETVAL);
	sv_2mortal(ST(0));
	XSRETURN(1);
}

XS(XS__getracename);
XS(XS__getracename) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::getracename(uint16 race_id)");

	dXSTARG;
	uint16 race_id = (int) SvIV(ST(0));
	std::string race_name = quest_manager.getracename(race_id);

	sv_setpv(TARG, race_name.c_str());
	XSprePUSH;
	PUSHTARG;
	XSRETURN(1);
}

XS(XS__getspellname);
XS(XS__getspellname) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::getspellname(uint32 spell_id)");

	dXSTARG;
	uint32 spell_id = (int) SvIV(ST(0));
	std::string spell_name = quest_manager.getspellname(spell_id);

	sv_setpv(TARG, spell_name.c_str());
	XSprePUSH;
	PUSHTARG;
	XSRETURN(1);
}

XS(XS__get_spell_level);
XS(XS__get_spell_level) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::get_spell_level(uint16 spell_id, uint8 class_id)");

	dXSTARG;
	uint16 spell_id = (int)SvIV(ST(0));
	uint8 class_id = (int)SvIV(ST(1));
	uint8 spell_level = IsValidSpell(spell_id) ? GetSpellLevel(spell_id, class_id) : 0;
	uint8 server_max_level = RuleI(Character, MaxLevel);

	if (spell_level && spell_level > server_max_level)
		spell_level = 0;

	XSprePUSH;
	PUSHu((UV)spell_level);

	XSRETURN(1);
}

XS(XS__getskillname);
XS(XS__getskillname) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::getskillname(int skill_id)");

	dXSTARG;
	int skill_id = (int) SvIV(ST(0));
	std::string skill_name = quest_manager.getskillname(skill_id);

	sv_setpv(TARG, skill_name.c_str());
	XSprePUSH;
	PUSHTARG;
	XSRETURN(1);
}

XS(XS__safemove);
XS(XS__safemove) {
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: quest::safemove()");


	quest_manager.safemove();

	XSRETURN_EMPTY;
}

XS(XS__rain);
XS(XS__rain) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::rain(int weather)");

	int weather = (int) SvIV(ST(0));

	quest_manager.rain(weather);

	XSRETURN_EMPTY;
}

XS(XS__snow);
XS(XS__snow) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::snow(int weather)");

	int weather = (int) SvIV(ST(0));

	quest_manager.snow(weather);

	XSRETURN_EMPTY;
}

XS(XS__surname);
XS(XS__surname) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::surname(string name)");

	char *name = (char *) SvPV_nolen(ST(0));

	quest_manager.surname(name);

	XSRETURN_EMPTY;
}

XS(XS__permaclass);
XS(XS__permaclass) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::permaclass(int class_id)");

	int class_id = (int) SvIV(ST(0));

	quest_manager.permaclass(class_id);

	XSRETURN_EMPTY;
}

XS(XS__permarace);
XS(XS__permarace) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::permarace(int race_id)");

	int race_id = (int) SvIV(ST(0));

	quest_manager.permarace(race_id);

	XSRETURN_EMPTY;
}

XS(XS__permagender);
XS(XS__permagender) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::permagender(int gender_id)");

	int gender_id = (int) SvIV(ST(0));

	quest_manager.permagender(gender_id);

	XSRETURN_EMPTY;
}

XS(XS__scribespells);
XS(XS__scribespells) {
	dXSARGS;
	if (items < 1)
		Perl_croak(aTHX_ "Usage: quest::scribespells(int max_level, [int min_level = 1])");

	uint16 RETVAL;
	dXSTARG;

	uint8 max_level = (uint8) SvIV(ST(0));
	uint8 min_level = (uint8) SvIV(ST(1));

	if (min_level)
		RETVAL = quest_manager.scribespells(max_level, min_level);
	else
		RETVAL = quest_manager.scribespells(max_level);

	XSprePUSH;
	PUSHu((IV) RETVAL);
	XSRETURN(1);
}

XS(XS__traindiscs);
XS(XS__traindiscs) {
	dXSARGS;
	if (items < 1)
		Perl_croak(aTHX_ "Usage: quest::traindiscs(int max_level, [int min_level = 1])");

	uint16 RETVAL;
	dXSTARG;

	uint8 max_level = (uint8) SvIV(ST(0));
	uint8 min_level = (uint8) SvIV(ST(1));

	if (min_level)
		RETVAL = quest_manager.traindiscs(max_level, min_level);
	else
		RETVAL = quest_manager.traindiscs(max_level);

	XSprePUSH;
	PUSHu((IV) RETVAL);
	XSRETURN(1);
}

XS(XS__unscribespells);
XS(XS__unscribespells) {
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: quest::unscribespells()");


	quest_manager.unscribespells();

	XSRETURN_EMPTY;
}

XS(XS__untraindiscs);
XS(XS__untraindiscs) {
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: quest::untraindiscs()");


	quest_manager.untraindiscs();

	XSRETURN_EMPTY;
}

XS(XS__givecash);
XS(XS__givecash) {
	dXSARGS;
	if (items != 4)
		Perl_croak(aTHX_ "Usage: quest::givecash(int copper, int silver, int gold, int platinum)");

	int copper   = (int) SvIV(ST(0));
	int silver   = (int) SvIV(ST(1));
	int gold     = (int) SvIV(ST(2));
	int platinum = (int) SvIV(ST(3));

	quest_manager.givecash(copper, silver, gold, platinum);

	XSRETURN_EMPTY;
}

XS(XS__pvp);
XS(XS__pvp) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::pvp(string mode [on|off])");

	char *mode = (char *) SvPV_nolen(ST(0));

	quest_manager.pvp(mode);

	XSRETURN_EMPTY;
}

XS(XS__movepc);
XS(XS__movepc) {
	dXSARGS;
	if (items != 4 && items != 5)
		Perl_croak(aTHX_ "Usage: quest::movepc(int zone_id, float x, float y, float z [float heading])");

	int   zone_id = (int) SvIV(ST(0));
	float x       = (float) SvNV(ST(1));
	float y       = (float) SvNV(ST(2));
	float z       = (float) SvNV(ST(3));

	if (items == 4)

		quest_manager.movepc(zone_id, x, y, z, 0.0f);

	else {
		float heading = (float) SvNV(ST(4));
		quest_manager.movepc(zone_id, x, y, z, heading);
	}

	XSRETURN_EMPTY;
}

XS(XS__gmmove);
XS(XS__gmmove) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: quest::gmmove(float x, float y, float z)");

	float x = (float) SvNV(ST(0));
	float y = (float) SvNV(ST(1));
	float z = (float) SvNV(ST(2));

	quest_manager.gmmove(x, y, z);

	XSRETURN_EMPTY;
}

XS(XS__movegrp);
XS(XS__movegrp) {
	dXSARGS;
	if (items != 4)
		Perl_croak(aTHX_ "Usage: quest::movegrp(int zone_id, float x, float y, float z)");

	int   zone_id = (int) SvIV(ST(0));
	float x       = (float) SvNV(ST(1));
	float y       = (float) SvNV(ST(2));
	float z       = (float) SvNV(ST(3));

	quest_manager.movegrp(zone_id, x, y, z);

	XSRETURN_EMPTY;
}

XS(XS__doanim);
XS(XS__doanim) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::doanim(int animation_id)");

	int anim_id = (int) SvIV(ST(0));

	quest_manager.doanim(anim_id);

	XSRETURN_EMPTY;
}

XS(XS__addskill);
XS(XS__addskill) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::addskill(int skill_id, int value)");

	int skill_id  = (int) SvIV(ST(0));
	int int_value = (int) SvIV(ST(1));

	quest_manager.addskill(skill_id, int_value);

	XSRETURN_EMPTY;
}

XS(XS__setlanguage);
XS(XS__setlanguage) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::setlanguage(int skill_id, int value)");

	int skill_id  = (int) SvIV(ST(0));
	int int_value = (int) SvIV(ST(1));

	quest_manager.setlanguage(skill_id, int_value);

	XSRETURN_EMPTY;
}

XS(XS__setskill);
XS(XS__setskill) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::setskill(int skill_id, int value)");

	int skill_id  = (int) SvIV(ST(0));
	int int_value = (int) SvIV(ST(1));

	quest_manager.setskill(skill_id, int_value);

	XSRETURN_EMPTY;
}

XS(XS__setallskill);
XS(XS__setallskill) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::setallskill(int value)");

	int int_value = (int) SvIV(ST(0));

	quest_manager.setallskill(int_value);

	XSRETURN_EMPTY;
}

XS(XS__attack);
XS(XS__attack) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::attack(string client_name)");

	char *client_name = (char *) SvPV_nolen(ST(0));

	quest_manager.attack(client_name);

	XSRETURN_EMPTY;
}

XS(XS__attacknpc);
XS(XS__attacknpc) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::attacknpc(int npc_entity_id)");

	int npc_entity_id = (int) SvIV(ST(0));

	quest_manager.attacknpc(npc_entity_id);

	XSRETURN_EMPTY;
}

XS(XS__attacknpctype);
XS(XS__attacknpctype) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::attacknpctype(int npc_type_id)");

	int npc_type_id = (int) SvIV(ST(0));

	quest_manager.attacknpctype(npc_type_id);

	XSRETURN_EMPTY;
}

XS(XS__save);
XS(XS__save) {
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: quest::save()");


	quest_manager.save();

	XSRETURN_EMPTY;
}

XS(XS__faction);
XS(XS__faction) {
	dXSARGS;
	if (items < 2 || items > 3)
		Perl_croak(aTHX_ "Usage: quest::faction(int faction_id, int value, [int temp = 0])");

	int faction_id = (int) SvIV(ST(0));
	int int_value  = (int) SvIV(ST(1));
	int temp;

	if (items == 2)
		temp = 0;
	else
		temp = (int) SvIV(ST(2));

	quest_manager.faction(faction_id, int_value, temp);

	XSRETURN_EMPTY;
}

XS(XS__setsky);
XS(XS__setsky) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::setsky(uint8 sky)");

	unsigned char new_sky = (unsigned char) SvUV(ST(0));

	quest_manager.setsky(new_sky);

	XSRETURN_EMPTY;
}

XS(XS__setguild);
XS(XS__setguild) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::setguild(int guild_id, int guild_rank_id)");

	unsigned long new_guild_id  = (unsigned long) SvUV(ST(0));
	int           guild_rank_id = (int) SvIV(ST(1));

	quest_manager.setguild(new_guild_id, guild_rank_id);

	XSRETURN_EMPTY;
}

XS(XS__createguild);
XS(XS__createguild) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::createguild(string guild_name, string leader_name)");

	char *guild_name  = (char *) SvPV_nolen(ST(0));
	char *leader_name = (char *) SvPV_nolen(ST(1));

	quest_manager.CreateGuild(guild_name, leader_name);

	XSRETURN_EMPTY;
}

XS(XS__settime);
XS(XS__settime) {
	dXSARGS;
	if (items < 2)
		Perl_croak(aTHX_ "Usage: quest::settime(int new_hour, int new_min, [bool update_world = true])");

	if (items == 2) {
		int new_hour = (int) SvIV(ST(0));
		int new_min  = (int) SvIV(ST(1));
		quest_manager.settime(new_hour, new_min, true);
	} else if (items == 3) {
		int new_hour = (int) SvIV(ST(0));
		int new_min  = (int) SvIV(ST(1));

		int update_world = (int) SvIV(ST(2));
		if (update_world == 1) {
			quest_manager.settime(new_hour, new_min, true);
		} else {
			quest_manager.settime(new_hour, new_min, false);
		}
	}

	XSRETURN_EMPTY;
}

XS(XS__itemlink);
XS(XS__itemlink) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::itemlink(int item_id)");

	int item_id = (int) SvIV(ST(0));

	quest_manager.itemlink(item_id);

	XSRETURN_EMPTY;
}

XS(XS__signalwith);
XS(XS__signalwith) {
	dXSARGS;

	if (items == 2) {
		int npc_id    = (int) SvIV(ST(0));
		int signal_id = (int) SvIV(ST(1));
		quest_manager.signalwith(npc_id, signal_id);
	} else if (items == 3) {
		int npc_id    = (int) SvIV(ST(0));
		int signal_id = (int) SvIV(ST(1));
		int wait      = (int) SvIV(ST(2));
		quest_manager.signalwith(npc_id, signal_id, wait);
	} else {
		Perl_croak(aTHX_ "Usage: quest::signalwith(int npc_id, int signal_id, [int wait_ms])");
	}

	XSRETURN_EMPTY;
}

XS(XS__signal);
XS(XS__signal) {
	dXSARGS;

	if (items == 1) {
		int npc_id = (int) SvIV(ST(0));
		quest_manager.signal(npc_id);
	} else if (items == 2) {
		int npc_id = (int) SvIV(ST(0));
		int wait   = (int) SvIV(ST(1));
		quest_manager.signal(npc_id, wait);
	} else {
		Perl_croak(aTHX_ "Usage: quest::signal(int npc_id, [int wait_ms])");
	}

	XSRETURN_EMPTY;
}

XS(XS__setglobal);
XS(XS__setglobal) {
	dXSARGS;
	if (items != 4)
		Perl_croak(aTHX_ "Usage: quest::setglobal(stirng key, string value, int options, string duration)");

	char *key       = (char *) SvPV_nolen(ST(0));
	char *str_value = (char *) SvPV_nolen(ST(1));
	int  options    = (int) SvIV(ST(2));
	char *duration  = (char *) SvPV_nolen(ST(3));

	quest_manager.setglobal(key, str_value, options, duration);

	XSRETURN_EMPTY;
}

XS(XS__targlobal);
XS(XS__targlobal) {
	dXSARGS;
	if (items != 6)
		Perl_croak(aTHX_ "Usage: quest::targlobal(stirng key, string value, string duration, int npc_id, int chararacter_id, int zone_id)");

	char *key       = (char *) SvPV_nolen(ST(0));
	char *str_value = (char *) SvPV_nolen(ST(1));
	char *duration  = (char *) SvPV_nolen(ST(2));
	int  npc_id     = (int) SvIV(ST(3));
	int  char_id    = (int) SvIV(ST(4));
	int  zone_id    = (int) SvIV(ST(5));

	quest_manager.targlobal(key, str_value, duration, npc_id, char_id, zone_id);

	XSRETURN_EMPTY;
}

XS(XS__delglobal);
XS(XS__delglobal) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::delglobal(string key)");

	char *key = (char *) SvPV_nolen(ST(0));

	quest_manager.delglobal(key);

	XSRETURN_EMPTY;
}

XS(XS__ding);
XS(XS__ding) {
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: quest::ding()");


	quest_manager.ding();

	XSRETURN_EMPTY;
}

XS(XS__rebind);
XS(XS__rebind) {
	dXSARGS;
	if (items < 4 || items > 5)
		Perl_croak(aTHX_ "Usage: quest::rebind(int zone_id, float x, float y, float z, [float heading])");

	int zone_id = (int) SvIV(ST(0));
	float target_x = (float) SvNV(ST(1));
	float target_y = (float) SvNV(ST(2));
	float target_z = (float) SvNV(ST(3));
	if (items > 4) {
		float target_heading = (float) SvNV(ST(4));
		quest_manager.rebind(zone_id, glm::vec4(target_x, target_y, target_z, target_heading));
	} else {
		quest_manager.rebind(zone_id, glm::vec3(target_x, target_y, target_z));
	}
	XSRETURN_EMPTY;
}

XS(XS__start);
XS(XS__start) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::start(int waypoint)");

	int wp = (int) SvIV(ST(0));

	quest_manager.start(wp);

	XSRETURN_EMPTY;
}

XS(XS__stop);
XS(XS__stop) {
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: quest::stop()");


	quest_manager.stop();

	XSRETURN_EMPTY;
}

XS(XS__pause);
XS(XS__pause) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::pause(int duration-ms)");

	int duration = (int) SvIV(ST(0));

	quest_manager.pause(duration);

	XSRETURN_EMPTY;
}

XS(XS__moveto);
XS(XS__moveto) {
	dXSARGS;
	if (items != 3 && items != 4 && items != 5)
		Perl_croak(aTHX_ "Usage: quest::moveto(float x, float y, float z, [float heading], [bool save_guard_location])");

	float x = (float) SvNV(ST(0));
	float y = (float) SvNV(ST(1));
	float z = (float) SvNV(ST(2));
	float h;
	bool  saveguard;

	if (items > 3)
		h = (float) SvNV(ST(3));
	else
		h = 0;

	if (items > 4)
		saveguard = (bool) SvTRUE(ST(4));
	else
		saveguard = false;

	quest_manager.moveto(glm::vec4(x, y, z, h), saveguard);

	XSRETURN_EMPTY;
}

XS(XS__resume);
XS(XS__resume) {
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: quest::resume()");


	quest_manager.resume();

	XSRETURN_EMPTY;
}

XS(XS__addldonpoints);
XS(XS__addldonpoints) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::addldonpoints(uint32 theme_id, int points)");
	
	uint32 theme_id = (uint32) SvUV(ST(0));
	int points = (int) SvIV(ST(1));
	quest_manager.addldonpoints(theme_id, points);
	XSRETURN_EMPTY;
}

XS(XS__addldonwin);
XS(XS__addldonwin) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::addldonwin(uint32 theme_id)");

	uint32 theme_id = (uint32) SvUV(ST(0));
	quest_manager.addldonwin(theme_id);
	XSRETURN_EMPTY;
}

XS(XS__addldonloss);
XS(XS__addldonloss) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::addldonloss(uint32 theme_id)");

	uint32 theme_id = (uint32) SvUV(ST(0));
	quest_manager.addldonloss(theme_id);
	XSRETURN_EMPTY;
}

XS(XS__setnexthpevent);
XS(XS__setnexthpevent) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::setnexthpevent(int at_mob_percentage)");

	int at = (int)SvIV(ST(0));

	quest_manager.setnexthpevent(at);

	XSRETURN_EMPTY;
}

XS(XS__setnextinchpevent);
XS(XS__setnextinchpevent) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::setnextinchpevent(int at_mob_percentage)");

	int at = (int)SvIV(ST(0));

	quest_manager.setnextinchpevent(at);

	XSRETURN_EMPTY;
}

XS(XS__sethp);
XS(XS__sethp) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::sethp(int mob_health_percentage [0-100])");

	int hpperc = (int)SvIV(ST(0));

	quest_manager.sethp(hpperc);

	XSRETURN_EMPTY;
}

XS(XS__respawn);
XS(XS__respawn) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::respawn(int npc_type_id, int grid_id)");

	int npc_type_id = (int)SvIV(ST(0));
	int grid_id = (int)SvIV(ST(1));

	quest_manager.respawn(npc_type_id, grid_id);

	XSRETURN_EMPTY;
}

//64 bit windows seems to optimize something poorly here causing access violations.
//If you don't do anything with index before passing it to perl it gets optimized out
//Disabling optimization right now for msvc on this function is the best solution.
#ifdef _MSC_VER
#pragma optimize( "", off )
#endif

XS(XS__ChooseRandom);
XS(XS__ChooseRandom) {
	dXSARGS;
	if (items < 1)
		Perl_croak(aTHX_ "Usage: quest::ChooseRandom(option1, option2, option3, option4, option5...[no limit])");

	dXSTARG;
	int index = zone->random.Int(0, items - 1);
	SV* RETVAL = ST(index);

	XSprePUSH;
	PUSHs(RETVAL);

	XSRETURN(1);    //return 1 element from the stack (ST(0))
}

#ifdef _MSC_VER
#pragma optimize( "", on )
#endif

XS(XS__set_proximity);
XS(XS__set_proximity) {
	dXSARGS;
	if (items != 4 && items != 6 && items != 7)
		Perl_croak(aTHX_ "Usage: quest::set_proximity(float min_x, float max_x, float min_y, float max_y, [float min_z], [float max_z], [say])");

	float min_x = (float) SvNV(ST(0));
	float max_x = (float) SvNV(ST(1));
	float min_y = (float) SvNV(ST(2));
	float max_y = (float) SvNV(ST(3));

	if (items == 4)
		quest_manager.set_proximity(min_x, max_x, min_y, max_y);
	else {
		float min_z = (float) SvNV(ST(4));
		float max_z = (float) SvNV(ST(5));
		bool  bSay  = false;
		if (items == 7)
			bSay = (bool) SvTRUE(ST(6));
		quest_manager.set_proximity(min_x, max_x, min_y, max_y, min_z, max_z, bSay);
	}

	XSRETURN_EMPTY;
}

XS(XS__clear_proximity);
XS(XS__clear_proximity) {
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: quest::clear_proximity()");

	quest_manager.clear_proximity();

	XSRETURN_EMPTY;
}

XS(XS__enable_proximity_say);
XS(XS__enable_proximity_say) {
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: quest::enable_proximity_say()");

	quest_manager.enable_proximity_say();

	XSRETURN_EMPTY;
}

XS(XS__disable_proximity_say);
XS(XS__disable_proximity_say) {
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: quest::disable_proximity_say()");

	quest_manager.disable_proximity_say();

	XSRETURN_EMPTY;
}

XS(XS__setanim);
XS(XS__setanim) //Cisyouc: mob->setappearance() addition
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::setanim(int npc_type_id, int appearance_number[0-4]);");

	quest_manager.setanim(SvUV(ST(0)), SvUV(ST(1)));

	XSRETURN_EMPTY;
}

XS(XS__showgrid);
XS(XS__showgrid) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::showgrid(int grid_id);");

	quest_manager.showgrid(SvUV(ST(0)));

	XSRETURN_EMPTY;
}

XS(XS__spawn_condition);
XS(XS__spawn_condition) {
	dXSARGS;
	if (items < 3 || items > 4)
		Perl_croak(aTHX_ "Usage: quest::spawn_condition(string zone_short, [int instance_id], uint16 condition_id, int16 value)");

	if (items == 3) {
		char   *zone_short  = (char *) SvPV_nolen(ST(0));
		uint16 condition_id = (int) SvUV(ST(1));
		int16  int_value    = (int) SvIV(ST(2));

		quest_manager.spawn_condition(zone_short, zone->GetInstanceID(), condition_id, int_value);
	} else {
		char   *zone_short  = (char *) SvPV_nolen(ST(0));
		uint32 instance_id  = (int) SvUV(ST(1));
		uint16 condition_id = (int) SvUV(ST(2));
		int16  int_value    = (int) SvIV(ST(3));

		quest_manager.spawn_condition(zone_short, instance_id, condition_id, int_value);
	}
	XSRETURN_EMPTY;
}

XS(XS__get_spawn_condition);
XS(XS__get_spawn_condition) {
	dXSARGS;
	if (items < 2 || items > 3)
		Perl_croak(aTHX_ "Usage: quest::get_spawn_condition(string zone_short, [int instance_id], int condition_id)");

	if (items == 2) {
		int16 RETVAL;
		dXSTARG;

		char   *zone_short = (char *) SvPV_nolen(ST(0));
		uint16 cond_id     = (int) SvIV(ST(1));

		RETVAL = quest_manager.get_spawn_condition(zone_short, zone->GetInstanceID(), cond_id);
		XSprePUSH;
		PUSHu((IV) RETVAL);

		XSRETURN(1);
	} else {
		int16 RETVAL;
		dXSTARG;

		char   *zone_short = (char *) SvPV_nolen(ST(0));
		uint16 instance_id = (int) SvIV(ST(1));
		uint16 cond_id     = (int) SvIV(ST(2));

		RETVAL = quest_manager.get_spawn_condition(zone_short, instance_id, cond_id);
		XSprePUSH;
		PUSHu((IV) RETVAL);

		XSRETURN(1);
	}
}

XS(XS__toggle_spawn_event);
XS(XS__toggle_spawn_event) {
	dXSARGS;
	if (items != 4)
		Perl_croak(aTHX_ "Usage: quest::toggle_spawn_event(uint32 event_id, [bool is_enabled = false], [bool is_strict = false], [bool reset_base = false])");

	uint32 event_id   = (int) SvIV(ST(0));
	bool   is_enabled = ((int) SvIV(ST(1))) == 0 ? false : true;
	bool   is_strict  = ((int) SvIV(ST(2))) == 0 ? false : true;
	bool   reset_base = ((int) SvIV(ST(3))) == 0 ? false : true;

	quest_manager.toggle_spawn_event(event_id, is_enabled, is_strict, reset_base);

	XSRETURN_EMPTY;
}

XS(XS__has_zone_flag);
XS(XS__has_zone_flag) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::has_zone_flag(uint32 zone_id)");

	int16 RETVAL;
	dXSTARG;

	uint32 zone_id = (int) SvIV(ST(0));

	RETVAL = quest_manager.has_zone_flag(zone_id);
	XSprePUSH;
	PUSHu((IV) RETVAL);

	XSRETURN(1);

}

XS(XS__set_zone_flag);
XS(XS__set_zone_flag) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::set_zone_flag(uint32 zone_id)");

	uint32 zone_id = (int) SvIV(ST(0));

	quest_manager.set_zone_flag(zone_id);

	XSRETURN_EMPTY;
}

XS(XS__clear_zone_flag);
XS(XS__clear_zone_flag) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::clear_zone_flag(uint32 zone_id)");

	uint32 zone_id = (int) SvIV(ST(0));

	quest_manager.clear_zone_flag(zone_id);

	XSRETURN_EMPTY;
}

XS(XS__summonburiedplayercorpse);
XS(XS__summonburiedplayercorpse) {
	dXSARGS;
	if (items != 5)
		Perl_croak(aTHX_ "Usage: quest::summonburiedplayercorpse(uint32 char_id, float dest_x, float dest_y, float dest_z, float dest_heading)");

	bool   RETVAL;
	uint32 char_id  = (int) SvIV(ST(0));
	auto   position = glm::vec4((float) SvIV(ST(1)), (float) SvIV(ST(2)), (float) SvIV(ST(3)), (float) SvIV(ST(4)));

	RETVAL = quest_manager.summonburiedplayercorpse(char_id, position);

	ST(0)           = boolSV(RETVAL);
	sv_2mortal(ST(0));
	XSRETURN(1);
}

XS(XS__summonallplayercorpses);
XS(XS__summonallplayercorpses) {
	dXSARGS;
	if (items != 5)
		Perl_croak(aTHX_ "Usage: quest::summonallplayercorpses(int char_id, float dest_x, float dest_y, float dest_z, float dest_heading)");

	bool   RETVAL;
	uint32 char_id  = (int) SvIV(ST(0));
	auto   position = glm::vec4((float) SvIV(ST(1)), (float) SvIV(ST(2)), (float) SvIV(ST(3)), (float) SvIV(ST(4)));

	RETVAL = quest_manager.summonallplayercorpses(char_id, position);

	ST(0)           = boolSV(RETVAL);
	sv_2mortal(ST(0));
	XSRETURN(1);
}

XS(XS__getplayercorpsecount);
XS(XS__getplayercorpsecount) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::getplayercorpsecount(uint32 char_id)");

	uint32 RETVAL;
	dXSTARG;

	uint32 char_id = (int) SvIV(ST(0));

	RETVAL = quest_manager.getplayercorpsecount(char_id);
	XSprePUSH;
	PUSHu((IV) RETVAL);

	XSRETURN(1);
}

XS(XS__getplayercorpsecountbyzoneid);
XS(XS__getplayercorpsecountbyzoneid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::getplayercorpsecountbyzoneid(uint32 char_id, uint32 zone_id)");

	uint32 RETVAL;
	dXSTARG;

	uint32 char_id = (int) SvIV(ST(0));
	uint32 zone_id = (int)SvIV(ST(1));

	RETVAL = quest_manager.getplayercorpsecountbyzoneid(char_id, zone_id);
	XSprePUSH;
	PUSHu((IV) RETVAL);

	XSRETURN(1);
}

XS(XS__getplayerburiedcorpsecount);
XS(XS__getplayerburiedcorpsecount) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::getplayerburiedcorpsecount(int character_id)");

	uint32 RETVAL;
	dXSTARG;

	uint32 char_id = (int) SvIV(ST(0));

	RETVAL = quest_manager.getplayerburiedcorpsecount(char_id);
	XSprePUSH;
	PUSHu((IV) RETVAL);

	XSRETURN(1);
}

XS(XS__buryplayercorpse);
XS(XS__buryplayercorpse) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::buryplayercorpse(int character_id)");

	uint32 RETVAL;
	dXSTARG;

	uint32 char_id = (int) SvIV(ST(0));

	RETVAL = quest_manager.buryplayercorpse(char_id);
	XSprePUSH;
	PUSHu((IV) RETVAL);

	XSRETURN(1);
}

XS(XS__forcedooropen);
XS(XS__forcedooropen) {
	dXSARGS;
	if (items < 1 || items > 2)
		Perl_croak(aTHX_ "Usage: quest::forcedooropen(int door_id, [int alt_mode=0])");

	if (items == 1) {
		uint32 door_id = (int) SvIV(ST(0));

		quest_manager.forcedooropen(door_id, false);

		XSRETURN_EMPTY;
	} else {
		uint32 door_id  = (int) SvIV(ST(0));
		bool   alt_mode = (int) SvIV(ST(1)) == 0 ? false : true;

		quest_manager.forcedooropen(door_id, alt_mode);

		XSRETURN_EMPTY;
	}
}

XS(XS__forcedoorclose);
XS(XS__forcedoorclose) {
	dXSARGS;
	if (items < 1 || items > 2)
		Perl_croak(aTHX_ "Usage: quest::forcedoorclose(int door_id, [bool alt_mode = 0])");

	if (items == 1) {
		uint32 door_id = (int) SvIV(ST(0));

		quest_manager.forcedoorclose(door_id, false);

		XSRETURN_EMPTY;
	} else {
		uint32 door_id  = (int) SvIV(ST(0));
		bool   alt_mode = (int) SvIV(ST(1)) == 0 ? false : true;

		quest_manager.forcedoorclose(door_id, alt_mode);

		XSRETURN_EMPTY;
	}
}

XS(XS__toggledoorstate);
XS(XS__toggledoorstate) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::toggledoorstate(int door_id)");

	uint32 door_id = (int) SvIV(ST(0));

	quest_manager.toggledoorstate(door_id);

	XSRETURN_EMPTY;
}

XS(XS__isdooropen);
XS(XS__isdooropen) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::isdooropen(int door_id)");

	bool RETVAL;
	dXSTARG;

	uint32 door_id = (int) SvIV(ST(0));

	RETVAL = quest_manager.isdooropen(door_id);
	XSprePUSH;
	PUSHu((IV) RETVAL);

	XSRETURN(1);
}

XS(XS__depopzone);
XS(XS__depopzone) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::depopzone([bool start_spawn_status = false])");

	bool StartSpawnStatus = ((int) SvIV(ST(0))) == 0 ? false : true;

	quest_manager.depopzone(StartSpawnStatus);

	XSRETURN_EMPTY;
}

XS(XS__repopzone);
XS(XS__repopzone) {
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: quest::repopzone()");

	quest_manager.repopzone();

	XSRETURN_EMPTY;
}

XS(XS__processmobswhilezoneempty);
XS(XS__processmobswhilezoneempty) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::processmobswhilezoneempty(bool on)");

	bool ProcessingOn = ((int) SvIV(ST(0))) == 0 ? false : true;

	quest_manager.processmobswhilezoneempty(ProcessingOn);

	XSRETURN_EMPTY;
}

XS(XS__npcrace);
XS(XS__npcrace) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::npcrace(int race_id)");

	int race_id = (int) SvIV(ST(0));

	quest_manager.npcrace(race_id);

	XSRETURN_EMPTY;
}

XS(XS__npcgender);
XS(XS__npcgender) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::npcgender(int gender_id)");

	int gender_id = (int) SvIV(ST(0));

	quest_manager.npcgender(gender_id);

	XSRETURN_EMPTY;
}

XS(XS__npcsize);
XS(XS__npcsize) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::npcsize(int size)");

	int size = (int) SvIV(ST(0));

	quest_manager.npcsize(size);

	XSRETURN_EMPTY;
}

XS(XS__npctexture);
XS(XS__npctexture) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::npctexture(int texture_id)");

	int texture_id = (int) SvIV(ST(0));

	quest_manager.npctexture(texture_id);

	XSRETURN_EMPTY;
}

XS(XS__playerrace);
XS(XS__playerrace) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::playerrace(int race_id)");

	int race_id = (int) SvIV(ST(0));

	quest_manager.playerrace(race_id);

	XSRETURN_EMPTY;
}

XS(XS__playergender);
XS(XS__playergender) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::playergender(int gender_id)");

	int gender_id = (int) SvIV(ST(0));

	quest_manager.playergender(gender_id);

	XSRETURN_EMPTY;
}

XS(XS__playersize);
XS(XS__playersize) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::playersize(int newsize)");

	int newsize = (int) SvIV(ST(0));

	quest_manager.playersize(newsize);

	XSRETURN_EMPTY;
}

XS(XS__playertexture);
XS(XS__playertexture) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::playertexture(int texture_id)");

	int texture_id = (int) SvIV(ST(0));

	quest_manager.playertexture(texture_id);

	XSRETURN_EMPTY;
}

XS(XS__playerfeature);
XS(XS__playerfeature) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::playerfeature(string feature [race|gender|texture|helm|haircolor|beardcolor|eyecolor1|eyecolor2|hair|face|beard|heritage|tatoo|details|size], int setting)");

	char *str_value = (char *) SvPV_nolen(ST(0));
	int  int_value  = (int) SvIV(ST(1));

	quest_manager.playerfeature(str_value, int_value);

	XSRETURN_EMPTY;
}

XS(XS__npcfeature);
XS(XS__npcfeature) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::npcfeature(string feature [race|gender|texture|helm|haircolor|beardcolor|eyecolor1|eyecolor2|hair|face|beard|heritage|tatoo|details|size], int value)");

	char *str_value = (char *) SvPV_nolen(ST(0));
	int  int_value  = (int) SvIV(ST(1));

	quest_manager.npcfeature(str_value, int_value);

	XSRETURN_EMPTY;
}

#ifdef BOTS

XS(XS__createbotcount);
XS(XS__createbotcount)
{
	dXSARGS;
	int		RETVAL;
	dXSTARG;

	RETVAL = quest_manager.createbotcount();
	XSprePUSH; PUSHi((IV)RETVAL);

	XSRETURN(1);
}

XS(XS__spawnbotcount);
XS(XS__spawnbotcount)
{
	dXSARGS;
	int		RETVAL;
	dXSTARG;

	RETVAL = quest_manager.spawnbotcount();
	XSprePUSH; PUSHi((IV)RETVAL);

	XSRETURN(1);
}

XS(XS__botquest);
XS(XS__botquest)
{
	dXSARGS;
	bool		RETVAL;
	dXSTARG;

	RETVAL = quest_manager.botquest();
	XSprePUSH; PUSHu((IV)RETVAL);

	XSRETURN(1);
}

XS(XS__createBot);
XS(XS__createBot)
{
	dXSARGS;
	bool		RETVAL;
	dXSTARG;

	if(items != 6)
	{
		Perl_croak(aTHX_ "Usage: quest::createBot(string first_name, string last_name, int level, int race_id, int class_id, int gender_id)");
	}

	char *firstname = (char *)SvPV_nolen(ST(0));
	char *lastname = (char *)SvPV_nolen(ST(1));
	int level = (int) SvIV(ST(2));
	int race_id = (int) SvIV(ST(3));
	int class_id = (int) SvIV(ST(4));
	int gender_id = (int) SvIV(ST(5));

	RETVAL = quest_manager.createBot(firstname, lastname, level, race_id, class_id, gender_id);
	XSprePUSH; PUSHu((IV)RETVAL);
	XSRETURN(1);
}

#endif //BOTS

XS(XS__taskselector);
XS(XS__taskselector) {
	dXSARGS;
	if ((items >= 1) && (items <= MAXCHOOSERENTRIES)) {
		int      tasks[MAXCHOOSERENTRIES];
		for (int i = 0; i < items; i++) {
			tasks[i] = (int) SvIV(ST(i));
		}
		quest_manager.taskselector(items, tasks);
	} else {
		Perl_croak(aTHX_ "Usage: quest::taskselector(int task_id, 2, 3, 4, 5 [up to 40])");
	}

	XSRETURN_EMPTY;
}
XS(XS__task_setselector);
XS(XS__task_setselector) {
	dXSARGS;
	if (items == 1) {
		int task_setid = (int) SvIV(ST(0));
		quest_manager.tasksetselector(task_setid);
	} else {
		Perl_croak(aTHX_ "Usage: quest::task_setselector(int task_set_id)");
	}

	XSRETURN_EMPTY;
}
XS(XS__enabletask);
XS(XS__enabletask) {
	dXSARGS;
	if ((items >= 1) && (items <= 10)) {
		int      tasks[10];
		for (int i = 0; i < items; i++) {
			tasks[i] = (int) SvIV(ST(i));
		}
		quest_manager.enabletask(items, tasks);
	} else {
		Perl_croak(aTHX_ "Usage: quest::enabletask(int task_id, 2, 3, [up to 10])");
	}

	XSRETURN_EMPTY;
}
XS(XS__disabletask);
XS(XS__disabletask) {
	dXSARGS;
	if ((items >= 1) && (items <= 10)) {
		int      tasks[10];
		for (int i = 0; i < items; i++) {
			tasks[i] = (int) SvIV(ST(i));
		}
		quest_manager.disabletask(items, tasks);
	} else {
		Perl_croak(aTHX_ "Usage: quest::disabletask(int task_id, 2, 3, [up to 10])");
	}

	XSRETURN_EMPTY;
}

XS(XS__istaskenabled);
XS(XS__istaskenabled) {
	dXSARGS;
	bool RETVAL;
	dXSTARG;

	if (items == 1) {
		unsigned int task_id = (int) SvIV(ST(0));
		RETVAL = quest_manager.istaskenabled(task_id);
	} else {
		Perl_croak(aTHX_ "Usage: quest::istaskenabled(int task_id)");
	}

	XSprePUSH;
	PUSHu((IV) RETVAL);

	XSRETURN(1);
}
XS(XS__istaskactive);
XS(XS__istaskactive) {
	dXSARGS;
	bool RETVAL;
	dXSTARG;

	if (items == 1) {
		unsigned int task_id = (int) SvIV(ST(0));
		RETVAL = quest_manager.istaskactive(task_id);
	} else {
		Perl_croak(aTHX_ "Usage: quest::istaskactive(int task_id)");
	}

	XSprePUSH;
	PUSHu((IV) RETVAL);

	XSRETURN(1);
}
XS(XS__istaskactivityactive);
XS(XS__istaskactivityactive) {
	dXSARGS;
	bool RETVAL;
	dXSTARG;

	if (items == 2) {
		unsigned int task_id     = (int) SvIV(ST(0));
		unsigned int activity_id = (int) SvIV(ST(1));
		RETVAL = quest_manager.istaskactivityactive(task_id, activity_id);
	} else {
		Perl_croak(aTHX_ "Usage: quest::istaskactivityactive(int task_id, int activity_id)");
	}

	XSprePUSH;
	PUSHu((IV) RETVAL);

	XSRETURN(1);
}
XS(XS__gettaskactivitydonecount);
XS(XS__gettaskactivitydonecount) {
	dXSARGS;
	uint32 RETVAL;
	dXSTARG;

	if (items == 2) {
		unsigned int task_id     = (int) SvIV(ST(0));
		unsigned int activity_id = (int) SvIV(ST(1));
		RETVAL = quest_manager.gettaskactivitydonecount(task_id, activity_id);
		XSprePUSH;
		PUSHu((UV) RETVAL);
	} else {
		Perl_croak(aTHX_ "Usage: quest::gettaskactivitydonecount(int task_id, int activity_id)");
	}

	XSRETURN(1);
}
XS(XS__updatetaskactivity);
XS(XS__updatetaskactivity) {
	dXSARGS;
	unsigned int task_id, activity_id;
	int          count               = 1;
	bool         ignore_quest_update = false;
	if (items == 2) {
		task_id     = (int) SvIV(ST(0));
		activity_id = (int) SvIV(ST(1));
		quest_manager.updatetaskactivity(task_id, activity_id, count, false);
	} else if (items == 3 || items == 4) {
		task_id     = (int) SvIV(ST(0));
		activity_id = (int) SvIV(ST(1));
		count       = (int) SvIV(ST(2));
		if (items == 4) {
			bool ignore_quest_update = (bool) SvTRUE(ST(3));
		}
		quest_manager.updatetaskactivity(task_id, activity_id, count, ignore_quest_update);
	} else {
		Perl_croak(aTHX_ "Usage: quest::updatetaskactivity(int task_id, int activity_id, [int count], [bool ignore_quest_update = false])");
	}

	XSRETURN_EMPTY;
}

XS(XS__resettaskactivity);
XS(XS__resettaskactivity) {
	dXSARGS;
	if (items == 2) {
		int task_id     = (int) SvIV(ST(0));
		int activity_id = (int) SvIV(ST(1));

		quest_manager.resettaskactivity(task_id, activity_id);

	} else {
		Perl_croak(aTHX_ "Usage: quest::resettaskactivity(int task_id, int activity_id)");
	}

	XSRETURN_EMPTY;
}

XS(XS__taskexploredarea);
XS(XS__taskexploredarea) {
	dXSARGS;
	unsigned int explore_id;
	if (items == 1) {
		explore_id = (int) SvIV(ST(0));
		quest_manager.taskexploredarea(explore_id);
	} else {
		Perl_croak(aTHX_ "Usage: quest::taskexplorearea(int explore_id)");
	}

	XSRETURN_EMPTY;
}

XS(XS__assigntask);
XS(XS__assigntask) {
	dXSARGS;
	unsigned int task_id;
	bool         enforce_level_requirement = false;
	if (items == 1 || items == 2) {
		task_id = (int) SvIV(ST(0));
		if (items == 2) {
			if ((int) SvIV(ST(1)) == 1) {
				enforce_level_requirement = true;
			}
		}
		quest_manager.assigntask(task_id, enforce_level_requirement);
	} else {
		Perl_croak(aTHX_ "Usage: quest::assigntask(int task_id, [bool enforce_level_requirement = false])");
	}

	XSRETURN_EMPTY;
}

XS(XS__failtask);
XS(XS__failtask) {
	dXSARGS;
	unsigned int task_id;
	if (items == 1) {
		task_id = (int) SvIV(ST(0));
		quest_manager.failtask(task_id);
	} else {
		Perl_croak(aTHX_ "Usage: quest::failtask(int task_id)");
	}

	XSRETURN_EMPTY;
}

XS(XS__tasktimeleft);
XS(XS__tasktimeleft) {
	dXSARGS;
	int RETVAL;
	dXSTARG;

	if (items == 1) {
		unsigned int task_id = (int) SvIV(ST(0));
		RETVAL = quest_manager.tasktimeleft(task_id);
	} else {
		Perl_croak(aTHX_ "Usage: quest::tasktimeleft(int task_id)");
	}

	XSprePUSH;
	PUSHi((IV) RETVAL);

	XSRETURN(1);
}

XS(XS__istaskcompleted);
XS(XS__istaskcompleted) {
	dXSARGS;
	int RETVAL;
	dXSTARG;

	if (items == 1) {
		unsigned int task_id = (int) SvIV(ST(0));
		RETVAL = quest_manager.istaskcompleted(task_id);
	} else {
		Perl_croak(aTHX_ "Usage: quest::istaskcompleted(int task_id)");
	}

	XSprePUSH;
	PUSHi((IV) RETVAL);

	XSRETURN(1);
}

XS(XS__enabledtaskcount);
XS(XS__enabledtaskcount) {
	dXSARGS;
	int RETVAL;
	dXSTARG;

	if (items == 1) {
		unsigned int task_set = (int) SvIV(ST(0));
		RETVAL = quest_manager.enabledtaskcount(task_set);
	} else {
		Perl_croak(aTHX_ "Usage: quest::enabledtaskcount(int task_set)");
	}

	XSprePUSH;
	PUSHi((IV) RETVAL);

	XSRETURN(1);
}

XS(XS__firsttaskinset);
XS(XS__firsttaskinset) {
	dXSARGS;
	int RETVAL;
	dXSTARG;

	if (items == 1) {
		unsigned int task_set = (int) SvIV(ST(0));
		RETVAL = quest_manager.firsttaskinset(task_set);
	} else {
		Perl_croak(aTHX_ "Usage: quest::firsttaskinset(int task_set)");
	}

	XSprePUSH;
	PUSHi((IV) RETVAL);

	XSRETURN(1);
}

XS(XS__lasttaskinset);
XS(XS__lasttaskinset) {
	dXSARGS;
	int RETVAL;
	dXSTARG;

	if (items == 1) {
		unsigned int task_set = (int) SvIV(ST(0));
		RETVAL = quest_manager.lasttaskinset(task_set);
	} else {
		Perl_croak(aTHX_ "Usage: quest::lasttaskinset(int task_set)");
	}

	XSprePUSH;
	PUSHi((IV) RETVAL);

	XSRETURN(1);
}

XS(XS__nexttaskinset);
XS(XS__nexttaskinset) {
	dXSARGS;
	int RETVAL;
	dXSTARG;

	if (items == 2) {
		unsigned int task_set = (int) SvIV(ST(0));
		unsigned int task_id  = (int) SvIV(ST(1));
		RETVAL = quest_manager.nexttaskinset(task_set, task_id);
	} else {
		Perl_croak(aTHX_ "Usage: quest::nexttaskinset(int task_set, int task_id)");
	}

	XSprePUSH;
	PUSHi((IV) RETVAL);

	XSRETURN(1);
}
XS(XS__activespeaktask);
XS(XS__activespeaktask) {
	dXSARGS;
	int RETVAL;
	dXSTARG;

	if (items == 0) {
		RETVAL = quest_manager.activespeaktask();
	} else {
		Perl_croak(aTHX_ "Usage: quest::activespeaktask()");
	}

	XSprePUSH;
	PUSHi((IV) RETVAL);

	XSRETURN(1);
}

XS(XS__activespeakactivity);
XS(XS__activespeakactivity) {
	dXSARGS;
	int RETVAL;
	dXSTARG;

	if (items == 1) {
		unsigned int task_id = (int) SvIV(ST(0));
		RETVAL = quest_manager.activespeakactivity(task_id);
	} else {
		Perl_croak(aTHX_ "Usage: quest::activespeakactivity(int task_id)");
	}

	XSprePUSH;
	PUSHi((IV) RETVAL);

	XSRETURN(1);
}

XS(XS__activetasksinset);
XS(XS__activetasksinset) {
	dXSARGS;
	int RETVAL;
	dXSTARG;

	if (items == 1) {
		unsigned int task_set = (int) SvIV(ST(0));
		RETVAL = quest_manager.activetasksinset(task_set);
	} else {
		Perl_croak(aTHX_ "Usage: quest::activetasksinset(int task_set)");
	}

	XSprePUSH;
	PUSHi((IV) RETVAL);

	XSRETURN(1);
}

XS(XS__completedtasksinset);
XS(XS__completedtasksinset) {
	dXSARGS;
	int RETVAL;
	dXSTARG;

	if (items == 1) {
		unsigned int task_set = (int) SvIV(ST(0));
		RETVAL = quest_manager.completedtasksinset(task_set);
	} else {
		Perl_croak(aTHX_ "Usage: quest::completedtasksinset(int task_set)");
	}

	XSprePUSH;
	PUSHi((IV) RETVAL);

	XSRETURN(1);
}


XS(XS__istaskappropriate);
XS(XS__istaskappropriate) {
	dXSARGS;
	bool RETVAL;
	dXSTARG;

	if (items == 1) {
		unsigned int task = (int) SvIV(ST(0));
		RETVAL = quest_manager.istaskappropriate(task);
	} else {
		Perl_croak(aTHX_ "Usage: quest::istaskaappropriate(int task_id)");
	}

	XSprePUSH;
	PUSHu((IV) RETVAL);

	XSRETURN(1);
}

XS(XS__gettaskname);
XS(XS__gettaskname) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: quest::gettaskname(uint32 task_id)");
	}

	dXSTARG;
	uint32 task_id = (int) SvIV(ST(0));
	std::string task_name = quest_manager.gettaskname(task_id);

	sv_setpv(TARG, task_name.c_str());
	XSprePUSH;
	PUSHTARG;
	XSRETURN(1);
}

XS(XS__popup); // prototype to pass -Wmissing-prototypes
XS(XS__popup) {
	dXSARGS;
	int popup_id = 0;
	int buttons  = 0;
	int duration = 0;

	if ((items < 2) || (items > 5))
		Perl_croak(aTHX_ "Usage: quest::popup(string window_title, string message, int popup_id, int buttons, int duration)");

	if (items >= 3)
		popup_id = (int) SvIV(ST(2));

	if (items >= 4)
		buttons = (int) SvIV(ST(3));

	if (items == 5)
		duration = (int) SvIV(ST(4));

	quest_manager.popup(SvPV_nolen(ST(0)), SvPV_nolen(ST(1)), popup_id, buttons, duration);

	XSRETURN_EMPTY;
}

XS(XS__clearspawntimers);
XS(XS__clearspawntimers) {
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: quest::clearspawntimers()");

	quest_manager.clearspawntimers();

	XSRETURN_EMPTY;
}

XS(XS__ze);
XS(XS__ze) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::ze(int emote_color_id, string message)");

	int  channel_id = (int) SvIV(ST(0));
	char *message   = (char *) SvPV_nolen(ST(1));

	quest_manager.ze(channel_id, message);

	XSRETURN_EMPTY;
}

XS(XS__we);
XS(XS__we) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::we(int emote_color_id, string message)");

	int  channel_id = (int) SvIV(ST(0));
	char *message   = (char *) SvPV_nolen(ST(1));

	quest_manager.we(channel_id, message);

	XSRETURN_EMPTY;
}

XS(XS__message);
XS(XS__message) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::message(int color, string message)");

	int color = (int) SvIV(ST(0));
	char *message = (char *) SvPV_nolen(ST(1));
	quest_manager.message(color, message);
	XSRETURN_EMPTY;
}

XS(XS__whisper);
XS(XS__whisper) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::whisper(string message)");

	char *message = (char *) SvPV_nolen(ST(0));
	quest_manager.whisper(message);
	XSRETURN_EMPTY;
}

XS(XS__getlevel);
XS(XS__getlevel) {
	dXSARGS;
	if (items > 1)
		Perl_croak(aTHX_ "Usage: quest::getlevel(int type)");

	int RETVAL;
	dXSTARG;

	int type;
	if (items == 1)
		type = (int) SvIV(ST(0));
	else
		type = 0;

	RETVAL = quest_manager.getlevel(type);
	XSprePUSH;
	PUSHu((IV) RETVAL);

	XSRETURN(1);
}

XS(XS__CreateGroundObject);
XS(XS__CreateGroundObject) {
	dXSARGS;
	if (items != 5 && items != 6)
		Perl_croak(aTHX_ "Usage: quest::creategroundobject(int item_id, float x, float y, float z, float heading, [uint32 decay_time-ms = 300000])");

	int    item_id = (int) SvIV(ST(0));
	float  x       = (float) SvNV(ST(1));
	float  y       = (float) SvNV(ST(2));
	float  z       = (float) SvNV(ST(3));
	float  heading = (float) SvNV(ST(4));
	uint16 id      = 0;

	if (items == 5)
		id = quest_manager.CreateGroundObject(item_id, glm::vec4(x, y, z, heading));
	else {
		uint32 decay_time = (uint32) SvIV(ST(5));
		id = quest_manager.CreateGroundObject(item_id, glm::vec4(x, y, z, heading), decay_time);
	}

	XSRETURN_IV(id);
}

XS(XS__CreateGroundObjectFromModel);
XS(XS__CreateGroundObjectFromModel) {
	dXSARGS;
	if (items < 5 || items > 7)
		Perl_croak(aTHX_ "Usage: quest::creategroundobjectfrommodel(string model_name, float x, float y, float z, float heading, [int object_type], [uint32 decay_time-ms = 300000])");

	char   *modelname  = (char *) SvPV_nolen(ST(0));
	float  x           = (float) SvNV(ST(1));
	float  y           = (float) SvNV(ST(2));
	float  z           = (float) SvNV(ST(3));
	float  heading     = (float) SvNV(ST(4));
	uint32 object_type = 0;
	uint32 decay_time  = 0;
	uint16 id          = 0;

	if (items > 5)
		object_type = (uint32) SvIV(ST(5));

	if (items > 6)
		decay_time = (uint32) SvIV(ST(6));

	id = quest_manager.CreateGroundObjectFromModel(modelname, glm::vec4(x, y, z, heading), object_type, decay_time);
	XSRETURN_IV(id);
}

XS(XS__CreateDoor);
XS(XS__CreateDoor) {
	dXSARGS;
	if (items < 5 || items > 7)
		Perl_croak(aTHX_ "Usage: quest::createdoor(string model_name, float x, float y, float z, float heading, [int object_type = 58], [int size = 100])");

	char   *modelname  = (char *) SvPV_nolen(ST(0));
	float  x           = (float) SvNV(ST(1));
	float  y           = (float) SvNV(ST(2));
	float  z           = (float) SvNV(ST(3));
	float  heading     = (float) SvNV(ST(4));
	uint32 object_type = 58;
	uint32 size        = 100;
	uint16 id          = 0;

	if (items > 5)
		object_type = (uint32) SvIV(ST(5));

	if (items > 6)
		size = (uint32) SvIV(ST(6));

	id = quest_manager.CreateDoor(modelname, x, y, z, heading, object_type, size);
	XSRETURN_IV(id);
}

XS(XS__ModifyNPCStat);
XS(XS__ModifyNPCStat) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::modifynpcstat(string key, string value)");

	quest_manager.ModifyNPCStat(SvPV_nolen(ST(0)), SvPV_nolen(ST(1)));

	XSRETURN_EMPTY;
}

XS(XS__collectitems);
XS(XS__collectitems) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::collectitems(int item_id, [bool remove_item = true])");

	uint32 item_id     = (int) SvIV(ST(0));
	bool   remove_item = ((int) SvIV(ST(1))) == 0 ? false : true;

	int quantity =
			    quest_manager.collectitems(item_id, remove_item);

	XSRETURN_IV(quantity);
}

XS(XS__countitem);
XS(XS__countitem) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::countitem(int item_id)");

	uint32 item_id = (int) SvIV(ST(0));

	int quantity = quest_manager.countitem(item_id);

	XSRETURN_IV(quantity);
}

XS(XS__removeitem);
XS(XS__removeitem) {
	dXSARGS;
	if (items < 1 || items > 2)
		Perl_croak(aTHX_ "Usage: quest::removeitem(int item_id, [int quantity = 1])");

	uint32 item_id = (int) SvIV(ST(0));
	uint32 quantity = 1;
	if (items > 1)
		quantity = (int) SvIV(ST(1));

	quest_manager.removeitem(item_id, quantity);

	XSRETURN_EMPTY;
}

XS(XS__getitemname);
XS(XS__getitemname) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::getitemname(uint32 item_id)");

	dXSTARG;
	uint32 item_id = (int) SvIV(ST(0));
	std::string item_name = quest_manager.getitemname(item_id);

	sv_setpv(TARG, item_name.c_str());
	XSprePUSH;
	PUSHTARG;
	XSRETURN(1);
}

XS(XS__getnpcnamebyid);
XS(XS__getnpcnamebyid) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::getnpcnamebyid(uint32 npc_id)");

	dXSTARG;
	uint32 npc_id = (int) SvIV(ST(0));
	auto npc_name = quest_manager.getnpcnamebyid(npc_id);
	sv_setpv(TARG, npc_name.c_str());
	XSprePUSH;
	PUSHTARG;
	XSRETURN(1);
}

XS(XS__UpdateSpawnTimer);
XS(XS__UpdateSpawnTimer) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::UpdateSpawnTimer(uint32 spawn2_id, uint32 updated_time_till_repop)");

	uint32 spawn2_id               = (int) SvIV(ST(0));
	uint32 updated_time_till_repop = (int) SvIV(ST(1));

	quest_manager.UpdateSpawnTimer(spawn2_id, updated_time_till_repop);

	XSRETURN_EMPTY;
}

XS(XS__MerchantSetItem);
XS(XS__MerchantSetItem) {
	dXSARGS;
	if (items != 2 && items != 3)
		Perl_croak(aTHX_ "Usage: quest::MerchantSetItem(uint32 npc_id, uint32 item_id, [uint32 quantity])");

	uint32 npc_id   = (int) SvUV(ST(0));
	uint32 item_id  = (int) SvUV(ST(1));
	uint32 quantity = 0;
	if (items == 3)
		quantity = (int) SvUV(ST(2));

	quest_manager.MerchantSetItem(npc_id, item_id, quantity);

	XSRETURN_EMPTY;
}

XS(XS__MerchantCountItem);
XS(XS__MerchantCountItem) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::MerchantCountItem(uint32 npc_id, uint32 item_id)");

	uint32 npc_id   = (int) SvUV(ST(0));
	uint32 item_id  = (int) SvUV(ST(1));
	uint32 quantity = quest_manager.MerchantCountItem(npc_id, item_id);

	XSRETURN_UV(quantity);
}

XS(XS__varlink);
XS(XS__varlink) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::varlink(uint32 item_id)");
	dXSTARG;

	Const_char *RETVAL;
	char       text[250];
	uint32     item_id;
	item_id = (int) SvUV(ST(0));

	RETVAL = quest_manager.varlink(text, item_id);

	sv_setpv(TARG, RETVAL);
	XSprePUSH;
	PUSHTARG;
	XSRETURN(1);
}

XS(XS__CreateInstance);
XS(XS__CreateInstance) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: quest::CreateInstance(string zone_name, uint16 version, uint32 duration)");

	char   *zone    = (char *) SvPV_nolen(ST(0));
	uint16 version  = (int) SvUV(ST(1));
	uint32 duration = (int) SvUV(ST(2));
	uint32 id       = quest_manager.CreateInstance(zone, version, duration);

	XSRETURN_UV(id);
}

XS(XS__DestroyInstance);
XS(XS__DestroyInstance) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::DestroyInstance(int id)");

	uint16 id = (int) SvUV(ST(0));
	quest_manager.DestroyInstance(id);

	XSRETURN_EMPTY;
}

XS(XS__UpdateInstanceTimer);
XS(XS__UpdateInstanceTimer) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::UpdateInstanceTimer(int16 instance_id, uint32 duration)");

	uint16 instance_id = (uint16) SvUV(ST(0));
	uint32 duration    = (uint32) SvUV(ST(1));
	quest_manager.UpdateInstanceTimer(instance_id, duration);

	XSRETURN_EMPTY;
}

XS(XS__GetInstanceTimer);
XS(XS__GetInstanceTimer) {
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: quest::GetInstanceTimer()");

	uint32 timer = quest_manager.GetInstanceTimer();

	XSRETURN_UV(timer);
}

XS(XS__GetInstanceTimerByID);
XS(XS__GetInstanceTimerByID) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::GetInstanceTimerByID(uint16 instance_id)");

	uint16 instance_id = (uint16) SvUV(ST(0));
	uint32 timer       = quest_manager.GetInstanceTimerByID(instance_id);

	XSRETURN_UV(timer);
}

XS(XS__GetInstanceID);
XS(XS__GetInstanceID) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::GetInstanceID(string zone_name, uint16 version)");

	char   *zone   = (char *) SvPV_nolen(ST(0));
	uint16 version = (int) SvUV(ST(1));
	uint16 id      = quest_manager.GetInstanceID(zone, version);

	XSRETURN_UV(id);
}

XS(XS__GetInstanceIDByCharID);
XS(XS__GetInstanceIDByCharID) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: quest::GetInstanceIDByCharID(string zone_name, uint16 version, uint32 char_id)");

	char   *zone   = (char *) SvPV_nolen(ST(0));
	uint16 version = (int) SvUV(ST(1));
	uint32 char_id = (int) SvUV(ST(2));
	uint16 id      = quest_manager.GetInstanceIDByCharID(zone, version, char_id);

	XSRETURN_UV(id);
}

XS(XS__GetCharactersInInstance);
XS(XS__GetCharactersInInstance) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::GetCharactersInInstance(uint16 instance_id)");
	dXSTARG;

	Const_char        *RETVAL;
	uint16            instance_id = (int) SvUV(ST(0));
	std::list<uint32> char_id_list;
	std::string       char_id_string;

	database.GetCharactersInInstance(instance_id, char_id_list);

	if (char_id_list.size() > 0) {
		char_id_string = itoa(char_id_list.size());
		char_id_string += " player(s) in instance: ";
		auto iter = char_id_list.begin();
		while (iter != char_id_list.end()) {
			char char_name[64];
			database.GetCharName(*iter, char_name);
			char_id_string += char_name;
			char_id_string += "(";
			char_id_string += itoa(*iter);
			char_id_string += ")";
			++iter;
			if (iter != char_id_list.end())
				char_id_string += ", ";
		}
		RETVAL    = char_id_string.c_str();
	} else
		RETVAL = "No players in that instance.";

	sv_setpv(TARG, RETVAL);
	XSprePUSH;
	PUSHTARG;
	XSRETURN(1);
}

XS(XS__AssignToInstance);
XS(XS__AssignToInstance) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::AssignToInstance(uint16 instance_id)");

	uint16 instance_id = (int) SvUV(ST(0));
	quest_manager.AssignToInstance(instance_id);

	XSRETURN_EMPTY;
}

XS(XS__AssignToInstanceByCharID);
XS(XS__AssignToInstanceByCharID) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::AssignToInstanceByCharID(uint16 instance_id, uint32 char_id)");

	uint16 instance_id = (int) SvUV(ST(0));
	uint32 char_id = (int) SvUV(ST(1));
	quest_manager.AssignToInstanceByCharID(instance_id, char_id);

	XSRETURN_EMPTY;
}

XS(XS__AssignGroupToInstance);
XS(XS__AssignGroupToInstance) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::AssignGroupToInstance(uint16 instance_id)");

	uint16 instance_id = (int) SvUV(ST(0));
	quest_manager.AssignGroupToInstance(instance_id);

	XSRETURN_EMPTY;
}

XS(XS__AssignRaidToInstance);
XS(XS__AssignRaidToInstance) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::AssignRaidToInstance(uint16 instance_id)");

	uint16 instance_id = (int) SvUV(ST(0));
	quest_manager.AssignRaidToInstance(instance_id);

	XSRETURN_EMPTY;
}

XS(XS__RemoveFromInstance);
XS(XS__RemoveFromInstance) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::RemoveFromInstance(uint16 instance_id)");

	uint16 instance_id = (int) SvUV(ST(0));
	quest_manager.RemoveFromInstance(instance_id);

	XSRETURN_EMPTY;
}

XS(XS__RemoveFromInstanceByCharID);
XS(XS__RemoveFromInstanceByCharID) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::RemoveFromInstanceByCharID(uint16 instance_id, uint32 char_id)");

	uint16 instance_id = (int) SvUV(ST(0));
	uint32 char_id = (int) SvUV(ST(1));
	quest_manager.RemoveFromInstanceByCharID(instance_id, char_id);

	XSRETURN_EMPTY;
}

XS(XS__CheckInstanceByCharID);
XS(XS__CheckInstanceByCharID) {
	dXSARGS;
	if (items != 2) {
		Perl_croak(aTHX_ "Usage: quest::CheckInstanceByCharID(uint16 instance_id, uint32 char_id)");
	}

	bool RETVAL;
	dXSTARG;

	uint16 instance_id = (int) SvUV(ST(0));
	uint32 char_id = (int) SvUV(ST(1));
	RETVAL = quest_manager.CheckInstanceByCharID(instance_id, char_id);
	XSprePUSH;
	PUSHu((IV) RETVAL);

	XSRETURN(1);
}

XS(XS__RemoveAllFromInstance);
XS(XS__RemoveAllFromInstance) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::RemoveAllFromInstance(uint16 instance_id)");

	uint16 instance_id = (int) SvUV(ST(0));
	quest_manager.RemoveAllFromInstance(instance_id);

	XSRETURN_EMPTY;
}

XS(XS__MovePCInstance);
XS(XS__MovePCInstance) {
	dXSARGS;
	if (items != 5 && items != 6)
		Perl_croak(aTHX_ "Usage: quest::MovePCInstance(int zone_id, int instance_id, float x, float y, float z, [float heading])");

	int   zone_id    = (int) SvIV(ST(0));
	int   instanceid = (int) SvIV(ST(1));
	float x          = (float) SvNV(ST(2));
	float y          = (float) SvNV(ST(3));
	float z          = (float) SvNV(ST(4));

	if (items == 4) {
		quest_manager.MovePCInstance(zone_id, instanceid, glm::vec4(x, y, z, 0.0f));
	} else {
		float heading = (float) SvNV(ST(5));
		quest_manager.MovePCInstance(zone_id, instanceid, glm::vec4(x, y, z, heading));
	}

	XSRETURN_EMPTY;
}

XS(XS__FlagInstanceByGroupLeader);
XS(XS__FlagInstanceByGroupLeader) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::FlagInstanceByGroupLeader(uint32 zone, uint16 version)");

	uint32 zone    = (int) SvUV(ST(0));
	uint16 version = (int) SvUV(ST(1));
	quest_manager.FlagInstanceByGroupLeader(zone, version);

	XSRETURN_EMPTY;
}

XS(XS__FlagInstanceByRaidLeader);
XS(XS__FlagInstanceByRaidLeader) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::FlagInstanceByRaidLeader(uint32 zone, uint16 version)");

	uint32 zone    = (int) SvUV(ST(0));
	uint16 version = (int) SvUV(ST(1));
	quest_manager.FlagInstanceByRaidLeader(zone, version);

	XSRETURN_EMPTY;
}

XS(XS__saylink);
XS(XS__saylink) {
	dXSARGS;
	if (items < 1 || items > 3)
		Perl_croak(aTHX_ "Usage: quest::saylink(string message, [bool silent = false], [link_name = message])");
	dXSTARG;

	std::string RETVAL;
	char message[250];
	char link_name[250];
	bool silent = false;
	strcpy(message, (char *) SvPV_nolen(ST(0)));
	if (items >= 2) {
		silent = ((int) SvIV(ST(1))) == 0 ? false : true;
	}
	if (items == 3)
		strcpy(link_name, (char *) SvPV_nolen(ST(2)));
	else
		strcpy(link_name, message);

	RETVAL = quest_manager.saylink(message, silent, link_name);

	sv_setpv(TARG, RETVAL.c_str());
	XSprePUSH;
	PUSHTARG;
	XSRETURN(1);
}

XS(XS__getcharnamebyid);
XS(XS__getcharnamebyid) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::getcharnamebyid(uint32 char_id)");
	dXSTARG;

	Const_char *RETVAL;
	uint32     char_id = (int) SvUV(ST(0));
	auto       name    = quest_manager.getcharnamebyid(char_id);

	RETVAL = name.c_str();

	sv_setpv(TARG, RETVAL);
	XSprePUSH;
	PUSHTARG;
}

XS(XS__getcharidbyname);
XS(XS__getcharidbyname) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::getcharidbyname(string name)");
	dXSTARG;

	uint32 		RETVAL;
	const char *name = (const char *) SvPV_nolen(ST(0));

	RETVAL = quest_manager.getcharidbyname(name);
	XSprePUSH;
	PUSHu((UV)RETVAL);

	XSRETURN(1);
}

XS(XS__getclassname);
XS(XS__getclassname) {
	dXSARGS;
	if (items < 1 || items > 2)
		Perl_croak(aTHX_ "Usage: quest::getclassname(uint8 class_id, [uint8 level = 0])");
	dXSTARG;

	std::string RETVAL;
	uint8  class_id = (int) SvUV(ST(0));
	uint8  level = 0;
	if (items > 1)
		level = (int) SvUV(ST(1));

	RETVAL = quest_manager.getclassname(class_id, level);
	sv_setpv(TARG, RETVAL.c_str());
	XSprePUSH;
	PUSHTARG;
  XSRETURN(1);
}

XS(XS__getcurrencyitemid);
XS(XS__getcurrencyitemid) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::getcurrencyitemid(int currency_id)");
	dXSTARG;

	int RETVAL;
	int currency_id = (int) SvUV(ST(0));

	RETVAL = quest_manager.getcurrencyitemid(currency_id);

	XSprePUSH;
	PUSHi((IV)RETVAL);
  	XSRETURN(1);
}

XS(XS__getcurrencyid);
XS(XS__getcurrencyid) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::getcurrencyid(uint32 item_id)");
	dXSTARG;

	int 		RETVAL;
	uint32      item_id = (int) SvUV(ST(0));

	RETVAL = quest_manager.getcurrencyid(item_id);
	XSprePUSH;
	PUSHi((IV)RETVAL);
	XSRETURN(1);
}

XS(XS__getguildnamebyid);
XS(XS__getguildnamebyid) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::getguildnamebyid(uint32 guild_id)");
	dXSTARG;

	Const_char *RETVAL;
	uint32     guild_id = (int) SvUV(ST(0));

	RETVAL = quest_manager.getguildnamebyid(guild_id);

	sv_setpv(TARG, RETVAL);
	XSprePUSH;
	PUSHTARG;
	XSRETURN(1);
}

XS(XS__getguildidbycharid);
XS(XS__getguildidbycharid) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::getguildidbycharid(uint32 char_id)");
	dXSTARG;

	int     RETVAL;
	uint32  char_id = (int) SvUV(ST(0));

	RETVAL = quest_manager.getguildidbycharid(char_id);

	XSprePUSH;
	PUSHi((IV)RETVAL);

	XSRETURN(1);
}

XS(XS__getgroupidbycharid);
XS(XS__getgroupidbycharid) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::getgroupidbycharid(uint32 char_id)");
	dXSTARG;

	int     RETVAL;
	uint32  char_id = (int) SvUV(ST(0));

	RETVAL = quest_manager.getgroupidbycharid(char_id);
	XSprePUSH;
	PUSHi((IV)RETVAL);

	XSRETURN(1);
}

XS(XS__getraididbycharid);
XS(XS__getraididbycharid) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::getraididbycharid(uint32 char_id)");
	dXSTARG;

	int     RETVAL;
	uint32  char_id = (int) SvUV(ST(0));

	RETVAL = quest_manager.getraididbycharid(char_id);
	XSprePUSH;
	PUSHi((IV)RETVAL);

	XSRETURN(1);
}

XS(XS__SetRunning);
XS(XS__SetRunning) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::SetRunning(bool is_running)");

	bool val = ((int) SvIV(ST(0))) == 0 ? false : true;

	quest_manager.SetRunning(val);

	XSRETURN_EMPTY;
}

XS(XS__IsRunning);
XS(XS__IsRunning) {
	dXSARGS;
	if (items >= 1)
		Perl_croak(aTHX_ "Usage: quest::IsRunning()");

	bool RETVAL;
	dXSTARG;


	RETVAL = quest_manager.IsRunning();
	XSprePUSH;
	PUSHu((IV) RETVAL);

	XSRETURN(1);
}

XS(XS__IsEffectInSpell);
XS(XS__IsEffectInSpell) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::IsEffectInSpell(uint32 spell_id, uint32 effect_id)");

	uint32 spell_id  = (uint32) SvUV(ST(0));
	uint32 effect_id = (uint32) SvUV(ST(1));
	bool   RETVAL;
	dXSTARG;


	RETVAL = IsEffectInSpell(spell_id, effect_id);
	XSprePUSH;
	PUSHu((IV) RETVAL);

	XSRETURN(1);
}

XS(XS__IsBeneficialSpell);
XS(XS__IsBeneficialSpell) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::IsBeneficialSpell(uint32 spell_id)");

	uint32 spell_id = (uint32) SvUV(ST(0));
	bool   RETVAL;
	dXSTARG;


	RETVAL = BeneficialSpell(spell_id);
	XSprePUSH;
	PUSHu((IV) RETVAL);

	XSRETURN(1);
}

XS(XS__GetSpellResistType);
XS(XS__GetSpellResistType) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::GetSpellResistType(uint32 spell_id)");

	uint32 spell_id  = (uint32) SvUV(ST(0));
	int32  spell_val = 0;
	dXSTARG;

	spell_val = GetSpellResistType(spell_id);
	XSRETURN_UV(spell_val);
}

XS(XS__GetSpellTargetType);
XS(XS__GetSpellTargetType) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::GetSpellTargetType(uint32 spell_id)");

	uint32 spell_id  = (uint32) SvUV(ST(0));
	int32  spell_val = 0;
	dXSTARG;

	spell_val = GetSpellTargetType(spell_id);
	XSRETURN_UV(spell_val);
}

XS(XS__FlyMode);
XS(XS__FlyMode) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::FlyMode(uint8 mode [0-5])");

	GravityBehavior flymode = (GravityBehavior) SvUV(ST(0));
	quest_manager.FlyMode(flymode);

	XSRETURN_EMPTY;
}

XS(XS_FactionValue);
XS(XS_FactionValue) {
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: quest::factionvalue()");

	uint8 fac = quest_manager.FactionValue();
	XSRETURN_UV(fac);
}

XS(XS__enabletitle);
XS(XS__enabletitle) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::enabletitle(int title_set_id)");

	int titleset = (int) SvIV(ST(0));

	quest_manager.enabletitle(titleset);

	XSRETURN_EMPTY;
}

XS(XS__checktitle);
XS(XS__checktitle) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::checktitle(int title_set_id)");

	bool RETVAL;
	int  titleset = (int) SvIV(ST(0));

	RETVAL = quest_manager.checktitle(titleset);

	ST(0)         = boolSV(RETVAL);
	sv_2mortal(ST(0));
	XSRETURN(1);
}

XS(XS__removetitle);
XS(XS__removetitle) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::removetitle(int title_set_id)");

	int titleset = (int) SvIV(ST(0));

	quest_manager.removetitle(titleset);

	XSRETURN_EMPTY;
}

XS(XS__wearchange);
XS(XS__wearchange) {
	dXSARGS;
	if (items < 2)
		Perl_croak(aTHX_ "Usage: quest::wearchange(uint8 slot, uint16 texture_id, [uint32 hero_forge_model_id = 0], [uint32 elite_material_id = 0])");

	uint8  slot       = (int) SvUV(ST(0));
	uint16 texture_id = (int) SvUV(ST(1));

	uint32 hero_forge_model_id = 0;
	uint32 elite_material_id   = 0;

	if (items > 2)
		hero_forge_model_id = (int) SvUV(ST(2));

	if (items > 3)
		elite_material_id = (int) SvUV(ST(3));

	quest_manager.wearchange(slot, texture_id, hero_forge_model_id, elite_material_id);

	XSRETURN_EMPTY;
}

XS(XS__voicetell);
XS(XS__voicetell) {
	dXSARGS;
	if (items != 4)
		Perl_croak(aTHX_ "Usage: quest::voicetell(string client_name, int macro_id, int ace_id, int gender_id)");

	char *client_name = (char *) SvPV_nolen(ST(0));
	int  macro_id     = (int) SvIV(ST(1));
	int  race_id      = (int) SvIV(ST(2));
	int  gender_id    = (int) SvIV(ST(3));

	quest_manager.voicetell(client_name, macro_id, race_id, gender_id);

	XSRETURN_EMPTY;
}

XS(XS__LearnRecipe);
XS(XS__LearnRecipe) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::LearnRecipe(int recipe_id)");

	uint32 recipe_id = (uint32) SvIV(ST(0));

	quest_manager.LearnRecipe(recipe_id);

	XSRETURN_EMPTY;
}

XS(XS__SendMail);
XS(XS__SendMail) {
	dXSARGS;
	if (items != 4)
		Perl_croak(aTHX_ "Usage: quest::SendMail(stirng to, string from, string subject, string message)");

	char *to      = (char *) SvPV_nolen(ST(0));
	char *from    = (char *) SvPV_nolen(ST(1));
	char *subject = (char *) SvPV_nolen(ST(2));
	char *message = (char *) SvPV_nolen(ST(3));

	quest_manager.SendMail(to, from, subject, message);

	XSRETURN_EMPTY;
}

XS(XS__GetZoneID);
XS(XS__GetZoneID) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::GetZoneID(string zone)");

	char  *zone = (char *) SvPV_nolen(ST(0));
	int32 id    = quest_manager.GetZoneID(zone);

	XSRETURN_IV(id);
}

XS(XS__GetZoneLongName);
XS(XS__GetZoneLongName) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::GetZoneLongName(string zone)");
	dXSTARG;

	std::string zone = (std::string) SvPV_nolen(ST(0));
	std::string RETVAL = quest_manager.GetZoneLongName(zone);

	sv_setpv(TARG, RETVAL.c_str());
	XSprePUSH;
	PUSHTARG;
	XSRETURN(1);
}

XS(XS__GetZoneLongNameByID);
XS(XS__GetZoneLongNameByID) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::GetZoneLongNameByID(uint32 zone_id)");

	dXSTARG;
	uint32 zone_id = (uint32) SvUV(ST(0));
	std::string zone_long_name = quest_manager.GetZoneLongNameByID(zone_id);
	sv_setpv(TARG, zone_long_name.c_str());
	XSprePUSH;
	PUSHTARG;
	XSRETURN(1);
}

XS(XS__GetZoneShortName);
XS(XS__GetZoneShortName) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::GetZoneShortName(uint32 zone_id)");

	dXSTARG;
	uint32 zone_id = (uint32) SvUV(ST(0));
	std::string zone_short_name = quest_manager.GetZoneShortName(zone_id);
	sv_setpv(TARG, zone_short_name.c_str());
	XSprePUSH;
	PUSHTARG;
	XSRETURN(1);
}

XS(XS__GetTimeSeconds);
XS(XS__GetTimeSeconds) {
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: quest::GetTimeSeconds()");

	uint32 seconds = 0;
	dXSTARG;

	seconds = Timer::GetTimeSeconds();
	XSRETURN_UV(seconds);
}

XS(XS__crosszoneassigntaskbycharid);
XS(XS__crosszoneassigntaskbycharid) {
	dXSARGS;
	if (items < 2 || items > 3)
		Perl_croak(aTHX_ "Usage: quest::crosszoneassigntaskbycharid(int character_id, uint32 task_id, [bool enforce_level_requirement = false])");
	{
		int character_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvIV(ST(1));
		bool enforce_level_requirement = false;

		if (items == 3) {
			enforce_level_requirement = (bool) SvTRUE(ST(2));
		}
		quest_manager.CrossZoneAssignTaskByCharID(character_id, task_id, enforce_level_requirement);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszoneassigntaskbygroupid);
XS(XS__crosszoneassigntaskbygroupid) {
	dXSARGS;
	if (items < 2 || items > 3)
		Perl_croak(aTHX_ "Usage: quest::crosszoneassigntaskbygroupid(int group_id, uint32 task_id, [bool enforce_level_requirement = false])");
	{
		int group_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvIV(ST(1));
		bool enforce_level_requirement = false;

		if (items == 3) {
			enforce_level_requirement = (bool) SvTRUE(ST(2));
		}
		quest_manager.CrossZoneAssignTaskByGroupID(group_id, task_id, enforce_level_requirement);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszoneassigntaskbyraidid);
XS(XS__crosszoneassigntaskbyraidid) {
	dXSARGS;
	if (items < 2 || items > 3)
		Perl_croak(aTHX_ "Usage: quest::crosszoneassigntaskbyraidid(int raid_id, uint32 task_id, [bool enforce_level_requirement = false])");\
	{
		int raid_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvIV(ST(1));
		bool enforce_level_requirement = false;

		if (items == 3) {
			enforce_level_requirement = (bool) SvTRUE(ST(2));
		}
		quest_manager.CrossZoneAssignTaskByRaidID(raid_id, task_id, enforce_level_requirement);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszoneassigntaskbyguildid);
XS(XS__crosszoneassigntaskbyguildid) {
	dXSARGS;
	if (items < 2 || items > 3)
		Perl_croak(aTHX_ "Usage: quest::crosszoneassigntaskbyguildid(int guild_id, uint32 task_id, [bool enforce_level_requirement = false])");
	{
		int guild_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvIV(ST(1));
		bool enforce_level_requirement = false;

		if (items == 3) {
			enforce_level_requirement = (bool) SvTRUE(ST(2));
		}
		quest_manager.CrossZoneAssignTaskByGuildID(guild_id, task_id, enforce_level_requirement);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszonecastspellbycharid);
XS(XS__crosszonecastspellbycharid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonecastspellbycharid(int character_id, uint32 spell_id)");
	{
		int character_id = (int) SvIV(ST(0));
		uint32 spell_id = (uint32) SvIV(ST(1));
		quest_manager.CrossZoneCastSpellByCharID(character_id, spell_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszonecastspellbygroupid);
XS(XS__crosszonecastspellbygroupid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonecastspellbygroupid(int group_id, uint32 spell_id)");
	{
		int group_id = (int) SvIV(ST(0));
		uint32 spell_id = (uint32) SvIV(ST(1));
		quest_manager.CrossZoneCastSpellByGroupID(group_id, spell_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszonecastspellbyraidid);
XS(XS__crosszonecastspellbyraidid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonecastspellbyraidid(int raid_id, uint32 spell_id)");
	{
		int raid_id = (int) SvIV(ST(0));
		uint32 spell_id = (uint32) SvIV(ST(1));
		quest_manager.CrossZoneCastSpellByRaidID(raid_id, spell_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszonecastspellbyguildid);
XS(XS__crosszonecastspellbyguildid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonecastspellbyguildid(int guild_id, uint32 spell_id)");
	{
		int guild_id = (int) SvIV(ST(0));
		uint32 spell_id = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneCastSpellByGuildID(guild_id, spell_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszonedisabletaskbycharid);
XS(XS__crosszonedisabletaskbycharid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonedisabletaskbycharid(int character_id, uint32 task_id)");
	{
		int char_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneDisableTaskByCharID(char_id, task_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszonedisabletaskbygroupid);
XS(XS__crosszonedisabletaskbygroupid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonedisabletaskbygroupid(int group_id, uint32 task_id)");
	{
		int group_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneDisableTaskByGroupID(group_id, task_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszonedisabletaskbyraidid);
XS(XS__crosszonedisabletaskbyraidid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonedisabletaskbyraidid(int raid_id, uint32 task_id)");
	{
		int raid_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneDisableTaskByRaidID(raid_id, task_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszonedisabletaskbyguildid);
XS(XS__crosszonedisabletaskbyguildid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonedisabletaskbyguildid(int guild_id, uint32 task_id)");
	{
		int guild_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneDisableTaskByGuildID(guild_id, task_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszoneenabletaskbycharid);
XS(XS__crosszoneenabletaskbycharid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszoneenabletaskbycharid(int character_id, uint32 task_id)");
	{
		int char_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneEnableTaskByCharID(char_id, task_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszoneenabletaskbygroupid);
XS(XS__crosszoneenabletaskbygroupid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszoneenabletaskbygroupid(int group_id, uint32 task_id)");
	{
		int group_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneEnableTaskByGroupID(group_id, task_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszoneenabletaskbyraidid);
XS(XS__crosszoneenabletaskbyraidid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszoneenabletaskbyraidid(int raid_id, uint32 task_id)");
	{
		int raid_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneEnableTaskByRaidID(raid_id, task_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszoneenabletaskbyguildid);
XS(XS__crosszoneenabletaskbyguildid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszoneenabletaskbyguildid(int guild_id, uint32 task_id)");
	{
		int guild_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneEnableTaskByGuildID(guild_id, task_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszonefailtaskbycharid);
XS(XS__crosszonefailtaskbycharid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonefailtaskbycharid(int character_id, uint32 task_id)");
	{
		int char_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneFailTaskByCharID(char_id, task_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszonefailtaskbygroupid);
XS(XS__crosszonefailtaskbygroupid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonefailtaskbygroupid(int group_id, uint32 task_id)");
	{
		int group_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneFailTaskByGroupID(group_id, task_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszonefailtaskbyraidid);
XS(XS__crosszonefailtaskbyraidid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonefailtaskbyraidid(int raid_id, uint32 task_id)");
	{
		int raid_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneFailTaskByRaidID(raid_id, task_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszonefailtaskbyguildid);
XS(XS__crosszonefailtaskbyguildid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonefailtaskbyguildid(int guild_id, uint32 task_id)");
	{
		int guild_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneFailTaskByGuildID(guild_id, task_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszonemarqueebycharid);
XS(XS__crosszonemarqueebycharid) {
	dXSARGS;

	if (items != 7)
		Perl_croak(aTHX_ "Usage: quest::crosszonemarqueebycharid(int character_id, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char *message)");

	if (items == 7) {
		int character_id = (int) SvIV(ST(0));
		int type = (int) SvIV(ST(1));
		int priority = (int) SvIV(ST(2));
		int fade_in = (int) SvIV(ST(3));
		int fade_out = (int) SvIV(ST(4));
		int duration = (int) SvIV(ST(5));
		char *message = (char *) SvPV_nolen(ST(6));
		quest_manager.CrossZoneMarqueeByCharID(character_id, type, priority, fade_in, fade_out, duration, message);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonemarqueebygroupid);
XS(XS__crosszonemarqueebygroupid) {
	dXSARGS;

	if (items != 7)
		Perl_croak(aTHX_ "Usage: quest::crosszonemarqueebygroupid(int group_id, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char *message)");

	if (items == 7) {
		int group_id = (int) SvIV(ST(0));
		int type = (int) SvIV(ST(1));
		int priority = (int) SvIV(ST(2));
		int fade_in = (int) SvIV(ST(3));
		int fade_out = (int) SvIV(ST(4));
		int duration = (int) SvIV(ST(5));
		char *message = (char *) SvPV_nolen(ST(6));
		quest_manager.CrossZoneMarqueeByGroupID(group_id, type, priority, fade_in, fade_out, duration, message);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonemarqueebyraidid);
XS(XS__crosszonemarqueebyraidid) {
	dXSARGS;

	if (items != 7)
		Perl_croak(aTHX_ "Usage: quest::crosszonemarqueebyraidid(int raid_id, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char *message)");

	if (items == 7) {
		int raid_id = (int) SvIV(ST(0));
		int type = (int) SvIV(ST(1));
		int priority = (int) SvIV(ST(2));
		int fade_in = (int) SvIV(ST(3));
		int fade_out = (int) SvIV(ST(4));
		int duration = (int) SvIV(ST(5));
		char *message = (char *) SvPV_nolen(ST(6));
		quest_manager.CrossZoneMarqueeByRaidID(raid_id, type, priority, fade_in, fade_out, duration, message);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonemarqueebyguildid);
XS(XS__crosszonemarqueebyguildid) {
	dXSARGS;

	if (items != 7)
		Perl_croak(aTHX_ "Usage: quest::crosszonemarqueebyguildid(int guild_id, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char *message)");

	if (items == 7) {
		int guild_id = (int) SvIV(ST(0));
		int type = (int) SvIV(ST(1));
		int priority = (int) SvIV(ST(2));
		int fade_in = (int) SvIV(ST(3));
		int fade_out = (int) SvIV(ST(4));
		int duration = (int) SvIV(ST(5));
		char *message = (char *) SvPV_nolen(ST(6));
		quest_manager.CrossZoneMarqueeByGuildID(guild_id, type, priority, fade_in, fade_out, duration, message);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonemessageplayerbyname);
XS(XS__crosszonemessageplayerbyname) {
	dXSARGS;

	if (items != 3)
		Perl_croak(aTHX_ "Usage: quest::crosszonemessageplayerbyname(uint32 type, string name, string message)");

	if (items == 3) {
		uint32 type = (uint32) SvUV(ST(0));
		char *name = (char *) SvPV_nolen(ST(1));
		char *message = (char *) SvPV_nolen(ST(2));
		quest_manager.CrossZoneMessagePlayerByName(type, name, message);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonemessageplayerbygroupid);
XS(XS__crosszonemessageplayerbygroupid) {
	dXSARGS;

	if (items != 3)
		Perl_croak(aTHX_ "Usage: quest::crosszonemessageplayerbygroupid(uint32 type, int group_id, string message)");

	if (items == 3) {
		uint32 type = (uint32) SvUV(ST(0));
		int group_id = (int) SvIV(ST(1));
		char *message = (char *) SvPV_nolen(ST(2));
		quest_manager.CrossZoneMessagePlayerByGroupID(type, group_id, message);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonemessageplayerbyraidid);
XS(XS__crosszonemessageplayerbyraidid) {
	dXSARGS;

	if (items != 3)
		Perl_croak(aTHX_ "Usage: quest::crosszonemessageplayerbyraidid(uint32 type, int raid_id, string message)");

	if (items == 3) {
		uint32 type = (uint32) SvUV(ST(0));
		int raid_id = (int) SvIV(ST(1));
		char *message = (char *) SvPV_nolen(ST(2));
		quest_manager.CrossZoneMessagePlayerByRaidID(type, raid_id, message);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonemessageplayerbyguildid);
XS(XS__crosszonemessageplayerbyguildid) {
	dXSARGS;

	if (items != 3)
		Perl_croak(aTHX_ "Usage: quest::crosszonemessageplayerbyguildid(uint32 type, int guild_id, string message)");

	if (items == 3) {
		uint32 type = (uint32) SvUV(ST(0));
		int guild_id = (int) SvIV(ST(1));
		char *message = (char *) SvPV_nolen(ST(2));
		quest_manager.CrossZoneMessagePlayerByGuildID(type, guild_id, message);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonemoveplayerbycharid);
XS(XS__crosszonemoveplayerbycharid) {
	dXSARGS;

	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonemoveplayerbycharid(int character_id, string zone_short_name)");

	if (items == 2) {
		int character_id = (int) SvIV(ST(0));
		char *zone_short_name = (char *) SvPV_nolen(ST(1));
		quest_manager.CrossZoneMovePlayerByCharID(character_id, zone_short_name);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonemoveplayerbygroupid);
XS(XS__crosszonemoveplayerbygroupid) {
	dXSARGS;

	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonemoveplayerbygroupid(int group_id, string zone_short_name)");

	if (items == 2) {
		int group_id = (int) SvIV(ST(0));
		char *zone_short_name = (char *) SvPV_nolen(ST(1));
		quest_manager.CrossZoneMovePlayerByGroupID(group_id, zone_short_name);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonemoveplayerbyraidid);
XS(XS__crosszonemoveplayerbyraidid) {
	dXSARGS;

	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonemoveplayerbyraidid(int raid_id, string zone_short_name)");

	if (items == 2) {
		int raid_id = (int) SvIV(ST(0));
		char *zone_short_name = (char *) SvPV_nolen(ST(1));
		quest_manager.CrossZoneMovePlayerByRaidID(raid_id, zone_short_name);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonemoveplayerbyguildid);
XS(XS__crosszonemoveplayerbyguildid) {
	dXSARGS;

	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonemoveplayerbyguildid(int guild_id, string zone_short_name)");

	if (items == 2) {
		int guild_id = (int) SvIV(ST(0));
		char *zone_short_name = (char *) SvPV_nolen(ST(1));
		quest_manager.CrossZoneMovePlayerByGuildID(guild_id, zone_short_name);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonemoveinstancebycharid);
XS(XS__crosszonemoveinstancebycharid) {
	dXSARGS;

	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonemoveinstancebycharid(int character_id, uint16 instance_id)");

	if (items == 2) {
		int character_id = (int) SvIV(ST(0));
		uint16 instance_id = (uint16) SvUV(ST(1));
		quest_manager.CrossZoneMoveInstanceByCharID(character_id, instance_id);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonemoveinstancebygroupid);
XS(XS__crosszonemoveinstancebygroupid) {
	dXSARGS;

	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonemoveinstancebygroupid(int group_id, uint16 instance_id)");

	if (items == 2) {
		int group_id = (int) SvIV(ST(0));
		uint16 instance_id = (uint16) SvUV(ST(1));
		quest_manager.CrossZoneMoveInstanceByGroupID(group_id, instance_id);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonemoveinstancebyraidid);
XS(XS__crosszonemoveinstancebyraidid) {
	dXSARGS;

	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonemoveinstancebyraidid(int raid_id, uint16 instance_id)");

	if (items == 2) {
		int raid_id = (int) SvIV(ST(0));
		uint16 instance_id = (uint16) SvUV(ST(1));
		quest_manager.CrossZoneMoveInstanceByRaidID(raid_id, instance_id);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonemoveinstancebyguildid);
XS(XS__crosszonemoveinstancebyguildid) {
	dXSARGS;

	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonemoveinstancebyguildid(int guild_id, uint16 instance_id)");

	if (items == 2) {
		int guild_id = (int) SvIV(ST(0));
		uint16 instance_id = (uint16) SvUV(ST(1));
		quest_manager.CrossZoneMoveInstanceByGuildID(guild_id, instance_id);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszoneremovespellbycharid);
XS(XS__crosszoneremovespellbycharid) {
	dXSARGS;

	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszoneremovespellbycharid(int character_id, uint32 spell_id)");

	if (items == 2) {
		int character_id = (int) SvIV(ST(0));
		uint32 spell_id = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneRemoveSpellByCharID(character_id, spell_id);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszoneremovespellbygroupid);
XS(XS__crosszoneremovespellbygroupid) {
	dXSARGS;

	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszoneremovespellbygroupid(int group_id, uint32 spell_id)");

	if (items == 2) {
		int group_id = (int) SvIV(ST(0));
		uint32 spell_id = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneRemoveSpellByGroupID(group_id, spell_id);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszoneremovespellbyraidid);
XS(XS__crosszoneremovespellbyraidid) {
	dXSARGS;

	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszoneremovespellbyraidid(int raid_id, uint32 spell_id)");

	if (items == 2) {
		int raid_id = (int) SvIV(ST(0));
		uint32 spell_id = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneRemoveSpellByRaidID(raid_id, spell_id);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszoneremovespellbyguildid);
XS(XS__crosszoneremovespellbyguildid) {
	dXSARGS;

	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszoneremovespellbyguildid(int guild_id, uint32 spell_id)");

	if (items == 2) {
		int guild_id = (int) SvIV(ST(0));
		uint32 spell_id = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneRemoveSpellByGuildID(guild_id, spell_id);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszoneremovetaskbycharid);
XS(XS__crosszoneremovetaskbycharid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszoneremovetaskbycharid(int character_id, uint32 task_id)");
	{
		int char_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneRemoveTaskByCharID(char_id, task_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszoneremovetaskbygroupid);
XS(XS__crosszoneremovetaskbygroupid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszoneremovetaskbygroupid(int group_id, uint32 task_id)");
	{
		int group_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneRemoveTaskByGroupID(group_id, task_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszoneremovetaskbyraidid);
XS(XS__crosszoneremovetaskbyraidid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszoneremovetaskbyraidid(int raid_id, uint32 task_id)");
	{
		int raid_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneRemoveTaskByRaidID(raid_id, task_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszoneremovetaskbyguildid);
XS(XS__crosszoneremovetaskbyguildid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszoneremovetaskbyguildid(int guild_id, uint32 task_id)");
	{
		int guild_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneRemoveTaskByGuildID(guild_id, task_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszoneresetactivitybycharid);
XS(XS__crosszoneresetactivitybycharid) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: quest::crosszoneresetactivitybycharid(int char_id, uint32 task_id, int activity_id)");
	{
		int char_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		int activity_id = (int) SvIV(ST(2));
		quest_manager.CrossZoneResetActivityByCharID(char_id, task_id, activity_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszoneresetactivitybygroupid);
XS(XS__crosszoneresetactivitybygroupid) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: quest::crosszoneresetactivitybygroupid(int group_id, uint32 task_id, int activity_id)");
	{
		int group_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		int activity_id = (int) SvIV(ST(2));
		quest_manager.CrossZoneResetActivityByGroupID(group_id, task_id, activity_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszoneresetactivitybyraidid);
XS(XS__crosszoneresetactivitybyraidid) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: quest::crosszoneresetactivitybyraidid(int raid_id, uint32 task_id, int activity_id)");
	{
		int raid_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		int activity_id = (int) SvIV(ST(2));
		quest_manager.CrossZoneResetActivityByRaidID(raid_id, task_id, activity_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszoneresetactivitybyguildid);
XS(XS__crosszoneresetactivitybyguildid) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: quest::crosszoneresetactivitybyguildid(int guild_id, uint32 task_id, int activity_id)");
	{
		int guild_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		int activity_id = (int) SvIV(ST(2));
		quest_manager.CrossZoneResetActivityByGuildID(guild_id, task_id, activity_id);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszonesetentityvariablebynpctypeid);
XS(XS__crosszonesetentityvariablebynpctypeid) {
	dXSARGS;

	if (items != 3)
		Perl_croak(aTHX_ "Usage: quest::crosszonesetentityvariablebynpctypeid(int npc_type_id, string key, string value)");

	if (items == 3) {
		uint32     npc_type_id = (uint32) SvUV(ST(0));
		const char *key        = (const char *) SvPV_nolen(ST(1));
		const char *str_value  = (const char *) SvPV_nolen(ST(2));
		quest_manager.CrossZoneSetEntityVariableByNPCTypeID(npc_type_id, key, str_value);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonesetentityvariablebyclientname);
XS(XS__crosszonesetentityvariablebyclientname) {
	dXSARGS;

	if (items != 3)
		Perl_croak(aTHX_ "Usage: quest::crosszonesetentityvariablebyclientname(string client_name, string key, string value)");

	if (items == 3) {
		const char *client_name = (const char *) SvPV_nolen(ST(0));
		const char *key         = (const char *) SvPV_nolen(ST(1));
		const char *str_value   = (const char *) SvPV_nolen(ST(2));
		quest_manager.CrossZoneSetEntityVariableByClientName(client_name, key, str_value);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonesetentityvariablebygroupid);
XS(XS__crosszonesetentityvariablebygroupid) {
	dXSARGS;

	if (items != 3)
		Perl_croak(aTHX_ "Usage: quest::crosszonesetentityvariablebygroupid(int group_id, string key, string value)");

	if (items == 3) {
		int group_id = SvIV(ST(0));
		const char *key = (const char *) SvPV_nolen(ST(1));
		const char *str_value = (const char *) SvPV_nolen(ST(2));
		quest_manager.CrossZoneSetEntityVariableByGroupID(group_id, key, str_value);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonesetentityvariablebyraidid);
XS(XS__crosszonesetentityvariablebyraidid) {
	dXSARGS;

	if (items != 3)
		Perl_croak(aTHX_ "Usage: quest::crosszonesetentityvariablebyraidid(int raid_id, string key, string value)");

	if (items == 3) {
		int raid_id = SvIV(ST(0));
		const char *key = (const char *) SvPV_nolen(ST(1));
		const char *str_value = (const char *) SvPV_nolen(ST(2));
		quest_manager.CrossZoneSetEntityVariableByRaidID(raid_id, key, str_value);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonesetentityvariablebyguildid);
XS(XS__crosszonesetentityvariablebyguildid) {
	dXSARGS;

	if (items != 3)
		Perl_croak(aTHX_ "Usage: quest::crosszonesetentityvariablebyguildid(int guild_id, string key, string value)");

	if (items == 3) {
		int guild_id = SvIV(ST(0));
		const char *key = (const char *) SvPV_nolen(ST(1));
		const char *str_value = (const char *) SvPV_nolen(ST(2));
		quest_manager.CrossZoneSetEntityVariableByGuildID(guild_id, key, str_value);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonesignalclientbycharid);
XS(XS__crosszonesignalclientbycharid) {
	dXSARGS;

	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonesignalclientbycharid(int character_id, uint32 signal)");

	if (items == 2) {
		int char_id = (int) SvIV(ST(0));
		uint32 signal = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneSignalPlayerByCharID(char_id, signal);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonesignalclientbygroupid);
XS(XS__crosszonesignalclientbygroupid) {
	dXSARGS;

	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonesignalclientbygroupid(int group_id, uint32 signal)");

	if (items == 2) {
		int group_id = (int) SvIV(ST(0));
		uint32 signal = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneSignalPlayerByGroupID(group_id, signal);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonesignalclientbyraidid);
XS(XS__crosszonesignalclientbyraidid) {
	dXSARGS;

	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonesignalclientbyraidid(int raid_id, uint32 signal)");

	if (items == 2) {
		int raid_id = (int) SvIV(ST(0));
		uint32 signal = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneSignalPlayerByRaidID(raid_id, signal);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonesignalclientbyguildid);
XS(XS__crosszonesignalclientbyguildid) {
	dXSARGS;

	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonesignalclientbyguildid(int guild_id, uint32 signal)");

	if (items == 2) {
		int guild_id = (int) SvIV(ST(0));
		uint32 signal = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneSignalPlayerByGuildID(guild_id, signal);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonesignalclientbyname);
XS(XS__crosszonesignalclientbyname) {
	dXSARGS;

	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonesignalclientbyname(string name, uint32 signal)");

	if (items == 2) {
		char *name = (char *) SvPV_nolen(ST(0));
		uint32 signal = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneSignalPlayerByName(name, signal);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonesignalnpcbynpctypeid);
XS(XS__crosszonesignalnpcbynpctypeid) {
	dXSARGS;

	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszonesignalnpcbynpctypeid(uint32 npc_type_id, uint32 value)");

	if (items == 2) {
		uint32 npc_type_id = (uint32) SvUV(ST(0));
		uint32 int_value   = (uint32) SvUV(ST(1));
		quest_manager.CrossZoneSignalNPCByNPCTypeID(npc_type_id, int_value);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszoneupdateactivitybycharid);
XS(XS__crosszoneupdateactivitybycharid) {
	dXSARGS;
	if (items < 3 || items > 4)
		Perl_croak(aTHX_ "Usage: quest::crosszoneupdateactivitybycharid(int char_id, uint32 task_id, int activity_id, [int activity_count = 1])");
	{
		int char_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		int activity_id = (int) SvIV(ST(2));
		int activity_count = 1;
		if (items == 4) {
			activity_count = (int) SvIV(ST(3));
		}
		quest_manager.CrossZoneUpdateActivityByCharID(char_id, task_id, activity_id, activity_count);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszoneupdateactivitybygroupid);
XS(XS__crosszoneupdateactivitybygroupid) {
	dXSARGS;
	if (items < 3 || items > 4)
		Perl_croak(aTHX_ "Usage: quest::crosszoneupdateactivitybygroupid(int group_id, uint32 task_id, int activity_id, [int activity_count = 1])");
	{
		int group_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		int activity_id = (int) SvIV(ST(2));
		int activity_count = 1;
		if (items == 4) {
			activity_count = (int) SvIV(ST(3));
		}
		quest_manager.CrossZoneUpdateActivityByGroupID(group_id, task_id, activity_id, activity_count);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszoneupdateactivitybyraidid);
XS(XS__crosszoneupdateactivitybyraidid) {
	dXSARGS;
	if (items < 3 || items > 4)
		Perl_croak(aTHX_ "Usage: quest::crosszoneupdateactivitybyraidid(int raid_id, uint32 task_id, int activity_id, [int activity_count = 1])");
	{
		int raid_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		int activity_id = (int) SvIV(ST(2));
		int activity_count = 1;
		if (items == 4) {
			activity_count = (int) SvIV(ST(3));
		}
		quest_manager.CrossZoneUpdateActivityByRaidID(raid_id, task_id, activity_id, activity_count);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszoneupdateactivitybyguildid);
XS(XS__crosszoneupdateactivitybyguildid) {
	dXSARGS;
	if (items < 3 || items > 4)
		Perl_croak(aTHX_ "Usage: quest::crosszoneupdateactivitybyguildid(int guild_id, uint32 task_id, int activity_id, [int activity_count = 1])");
	{
		int guild_id = (int) SvIV(ST(0));
		uint32 task_id = (uint32) SvUV(ST(1));
		int activity_id = (int) SvIV(ST(2));
		int activity_count = 1;
		if (items == 4) {
			activity_count = (int) SvIV(ST(3));
		}
		quest_manager.CrossZoneUpdateActivityByGuildID(guild_id, task_id, activity_id, activity_count);
	}
	XSRETURN_EMPTY;
}

XS(XS__worldwideassigntask);
XS(XS__worldwideassigntask) {
	dXSARGS;
	if (items < 1 || items > 3)
		Perl_croak(aTHX_ "Usage: quest::worldwideassigntask(uint32 task_id, [uint8 min_status = 0, uint8 max_status = 0])");
	{
		uint32 task_id = (uint32) SvUV(ST(0));
		uint8 min_status = 0;
		uint8 max_status = 0;
		if (items == 2) {
			min_status = (uint8) SvUV(ST(1));
		}

		if (items == 3) {
			max_status = (uint8) SvUV(ST(2));
		}
		quest_manager.WorldWideAssignTask(task_id, min_status, max_status);
	}

	XSRETURN_EMPTY;
}

XS(XS__worldwidecastspell);
XS(XS__worldwidecastspell) {
	dXSARGS;
	if (items < 1 || items > 3)
		Perl_croak(aTHX_ "Usage: quest::worldwidecastspell(uint32 spell_id, [uint8 min_status = 0, uint8 max_status = 0])");
	{
		uint32 spell_id = (uint32) SvUV(ST(0));
		uint8 min_status = 0;
		uint8 max_status = 0;
		if (items == 2) {
			min_status = (uint8) SvUV(ST(1));
		}

		if (items == 3) {
			max_status = (uint8) SvUV(ST(2));
		}
		quest_manager.WorldWideCastSpell(spell_id, min_status, max_status);
	}

	XSRETURN_EMPTY;
}

XS(XS__worldwidedisabletask);
XS(XS__worldwidedisabletask) {
	dXSARGS;
	if (items < 1 || items > 3)
		Perl_croak(aTHX_ "Usage: quest::worldwidedisabletask(uint32 task_id, [uint8 min_status = 0, uint8 max_status = 0])");
	{
		uint32 task_id = (uint32) SvUV(ST(0));
		uint8 min_status = 0;
		uint8 max_status = 0;
		if (items == 2) {
			min_status = (uint8) SvUV(ST(1));
		}

		if (items == 3) {
			max_status = (uint8) SvUV(ST(2));
		}
		quest_manager.WorldWideDisableTask(task_id, min_status, max_status);
	}

	XSRETURN_EMPTY;
}

XS(XS__worldwideenabletask);
XS(XS__worldwideenabletask) {
	dXSARGS;
	if (items < 1 || items > 3)
		Perl_croak(aTHX_ "Usage: quest::worldwideenabletask(uint32 task_id, [uint8 min_status = 0, uint8 max_status = 0])");
	{
		uint32 task_id = (uint32) SvUV(ST(0));
		uint8 min_status = 0;
		uint8 max_status = 0;
		if (items == 2) {
			min_status = (uint8) SvUV(ST(1));
		}

		if (items == 3) {
			max_status = (uint8) SvUV(ST(2));
		}
		quest_manager.WorldWideEnableTask(task_id, min_status, max_status);
	}

	XSRETURN_EMPTY;
}

XS(XS__worldwidefailtask);
XS(XS__worldwidefailtask) {
	dXSARGS;
	if (items < 1 || items > 3)
		Perl_croak(aTHX_ "Usage: quest::worldwidefailtask(uint32 task_id, [uint8 min_status = 0, uint8 max_status = 0])");
	{
		uint32 task_id = (uint32) SvUV(ST(0));
		uint8 min_status = 0;
		uint8 max_status = 0;
		if (items == 2) {
			min_status = (uint8) SvUV(ST(1));
		}

		if (items == 3) {
			max_status = (uint8) SvUV(ST(2));
		}
		quest_manager.WorldWideFailTask(task_id, min_status, max_status);
	}

	XSRETURN_EMPTY;
}

XS(XS__worldwidemarquee);
XS(XS__worldwidemarquee) {
	dXSARGS;
	if (items < 6 || items > 8)
		Perl_croak(aTHX_ "Usage: quest::worldwidemarquee(uint32 color_id, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, string message, [uint8 min_status = 0, uint8 max_status = 0])");
	{
		uint32 color_id = (uint32) SvUV(ST(0));
		uint32 priority = (uint32) SvUV(ST(1));
		uint32 fade_in  = (uint32) SvUV(ST(2));
		uint32 fade_out = (uint32) SvUV(ST(3));
		uint32 duration = (uint32) SvUV(ST(4));
		char   *message = (char *) SvPV_nolen(ST(5));
		uint8 min_status = 0;
		uint8 max_status = 0;
		if (items == 7) {
			min_status = (uint8) SvUV(ST(6));
		}

		if (items == 8) {
			max_status = (uint8) SvUV(ST(7));
		}
		quest_manager.WorldWideMarquee(color_id, priority, fade_in, fade_out, duration, message, min_status, max_status);
	}

	XSRETURN_EMPTY;
}

XS(XS__worldwidemessage);
XS(XS__worldwidemessage) {
	dXSARGS;
	if (items < 2 || items > 4)
		Perl_croak(aTHX_ "Usage: quest::worldwidemessage(uint32 type, string message, [uint8 min_status = 0, uint8 max_status = 0])");
	{
		uint32 type = (uint32)SvUV(ST(0));
		const char *message = (const char*) SvPV_nolen(ST(1));
		uint8 min_status = 0;
		uint8 max_status = 0;
		if (items == 3) {
			min_status = (uint8) SvUV(ST(2));
		}

		if (items == 4) {
			max_status = (uint8) SvUV(ST(3));
		}
		quest_manager.WorldWideMessage(type, message, min_status, max_status);
	}

	XSRETURN_EMPTY;
}

XS(XS__worldwidemove);
XS(XS__worldwidemove) {
	dXSARGS;
	if (items < 1 || items > 3)
		Perl_croak(aTHX_ "Usage: quest::worldwidemove(string zone_short_name, [uint8 min_status = 0, uint8 max_status = 0])");

	if (items == 1) {
		const char *zone_short_name = (const char*) SvPV_nolen(ST(0));
		uint8 min_status = 0;
		uint8 max_status = 0;
		if (items == 2) {
			min_status = (uint8) SvUV(ST(1));
		}

		if (items == 3) {
			max_status = (uint8) SvUV(ST(2));
		}
		quest_manager.WorldWideMove(zone_short_name, min_status, max_status);
	}

	XSRETURN_EMPTY;
}

XS(XS__worldwidemoveinstance);
XS(XS__worldwidemoveinstance) {
	dXSARGS;
	if (items < 1 || items > 3)
		Perl_croak(aTHX_ "Usage: quest::worldwidemoveinstance(uint16 instance_id, [uint8 min_status = 0, uint max_status = 0])");
	{
		uint16 instance_id = (uint16) SvUV(ST(0));
		uint8 min_status = 0;
		uint8 max_status = 0;
		if (items == 2) {
			min_status = (uint8) SvUV(ST(1));
		}

		if (items == 3) {
			max_status = (uint8) SvUV(ST(2));
		}
		quest_manager.WorldWideMoveInstance(instance_id, min_status, max_status);
	}

	XSRETURN_EMPTY;
}

XS(XS__worldwideremovespell);
XS(XS__worldwideremovespell) {
	dXSARGS;
	if (items < 1 || items > 3)
		Perl_croak(aTHX_ "Usage: quest::worldwideremovespell(uint32 spell_id, [uint8 min_status = 0, uint max_status = 0])");
	{
		uint32 spell_id = (uint32) SvUV(ST(0));
		uint8 min_status = 0;
		uint8 max_status = 0;
		if (items == 2) {
			min_status = (uint8) SvUV(ST(1));
		}

		if (items == 3) {
			max_status = (uint8) SvUV(ST(2));
		}
		quest_manager.WorldWideRemoveSpell(spell_id, min_status, max_status);
	}

	XSRETURN_EMPTY;
}

XS(XS__worldwideremovetask);
XS(XS__worldwideremovetask) {
	dXSARGS;
	if (items < 1 || items > 3)
		Perl_croak(aTHX_ "Usage: quest::worldwideremovetask(uint32 task_id, [uint8 min_status = 0, uint max_status = 0])");
	{
		uint32 task_id = (uint32) SvUV(ST(0));
		uint8 min_status = 0;
		uint8 max_status = 0;
		if (items == 2) {
			min_status = (uint8) SvUV(ST(1));
		}

		if (items == 3) {
			max_status = (uint8) SvUV(ST(2));
		}
		quest_manager.WorldWideRemoveTask(task_id, min_status, max_status);
	}

	XSRETURN_EMPTY;
}

XS(XS__worldwideresetactivity);
XS(XS__worldwideresetactivity) {
	dXSARGS;
	if (items < 2 || items > 4)
		Perl_croak(aTHX_ "Usage: quest::worldwideresetactivity(uint32 task_id, int activity_id, [uint8 min_status = 0, uint max_status = 0])");
	{
		uint32 task_id = (uint32) SvUV(ST(0));
		int activity_id = (int) SvIV(ST(1));
		uint8 min_status = 0;
		uint8 max_status = 0;
		if (items == 3) {
			min_status = (uint8) SvUV(ST(2));
		}

		if (items == 4) {
			max_status = (uint8) SvUV(ST(3));
		}
		quest_manager.WorldWideResetActivity(task_id, activity_id, min_status, max_status);
	}

	XSRETURN_EMPTY;
}

XS(XS__worldwidesetentityvariableclient);
XS(XS__worldwidesetentityvariableclient) {
	dXSARGS;
	if (items < 2 || items > 4)
		Perl_croak(aTHX_ "Usage: quest::worldwidesetentityvariableclient(string variable_name, string variable_value, [uint8 min_status = 0, uint max_status = 0])");
	{
		const char *variable_name = (const char*) SvPV_nolen(ST(0));
		const char *variable_value = (const char*) SvPV_nolen(ST(1));
		uint8 min_status = 0;
		uint8 max_status = 0;
		if (items == 3) {
			min_status = (uint8) SvUV(ST(2));
		}

		if (items == 4) {
			max_status = (uint8) SvUV(ST(3));
		}
		quest_manager.WorldWideSetEntityVariableClient(variable_name, variable_value, min_status, max_status);
	}

	XSRETURN_EMPTY;
}

XS(XS__worldwidesetentityvariablenpc);
XS(XS__worldwidesetentityvariablenpc) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::worldwidesetentityvariablenpc(string variable_name, string variable_value)");
	{
		const char *variable_name = (const char*) SvPV_nolen(ST(0));
		const char *variable_value = (const char*) SvPV_nolen(ST(1));
		quest_manager.WorldWideSetEntityVariableNPC(variable_name, variable_value);
	}

	XSRETURN_EMPTY;
}

XS(XS__worldwidesignalnpc);
XS(XS__worldwidesignalnpc) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::worldwidesignalnpc(uint32 signal)");
	{
		uint32 signal = (uint32) SvUV(ST(0));
		quest_manager.WorldWideSignalNPC(signal);
	}

	XSRETURN_EMPTY;
}

XS(XS__worldwidesignalclient);
XS(XS__worldwidesignalclient) {
	dXSARGS;
	if (items < 1 || items > 3)
		Perl_croak(aTHX_ "Usage: quest::worldwidesignalclient(uint32 signal, [uint8 min_status = 0, uint max_status = 0])");
	{
		uint32 signal = (uint32) SvUV(ST(0));
		uint8 min_status = 0;
		uint8 max_status = 0;
		if (items == 2) {
			min_status = (uint8) SvUV(ST(1));
		}

		if (items == 3) {
			max_status = (uint8) SvUV(ST(1));
		}
		quest_manager.WorldWideSignalClient(signal, min_status, max_status);
	}

	XSRETURN_EMPTY;
}
XS(XS__worldwideupdateactivity);
XS(XS__worldwideupdateactivity) {
	dXSARGS;
	if (items < 2 || items > 5)
		Perl_croak(aTHX_ "Usage: quest::worldwideupdateactivity(uint32 task_id, int activity_id, [int activity_count = 1, uint8 min_status = 0, uint max_status = 0])");
	{
		uint32 task_id = (uint32) SvUV(ST(0));
		int activity_id = (int) SvIV(ST(1));
		int activity_count = 1;
		uint8 min_status = 0;
		uint8 max_status = 0;
		if (items == 3) {
			activity_count = (int) SvIV(ST(2));
		}

		if (items == 4) {
			min_status = (uint8) SvUV(ST(3));
		}

		if (items == 5) {
			max_status = (uint8) SvUV(ST(4));
		}
		quest_manager.WorldWideUpdateActivity(task_id, activity_id, activity_count, min_status, max_status);
	}

	XSRETURN_EMPTY;
}

XS(XS__enablerecipe);
XS(XS__enablerecipe) {
	dXSARGS;
	bool success = false;

	if (items != 1) {
		Perl_croak(aTHX_ "Usage: quest::enablerecipe(int recipe_id)");
	} else {
		uint32 recipe_id = (uint32) SvIV(ST(0));
		success = quest_manager.EnableRecipe(recipe_id);
	}
	if (!success) {
		XSRETURN_NO;
	}

	XSRETURN_YES;
}

XS(XS__disablerecipe);
XS(XS__disablerecipe) {
	dXSARGS;
	bool success = false;

	if (items != 1) {
		Perl_croak(aTHX_ "Usage: quest::disablerecipe(int recipe_id)");
	} else {
		uint32 recipe_id = (uint32) SvIV(ST(0));
		success = quest_manager.DisableRecipe(recipe_id);
	}
	if (!success) {
		XSRETURN_NO;
	}

	XSRETURN_YES;
}

XS(XS__clear_npctype_cache);
XS(XS__clear_npctype_cache) {
	dXSARGS;

	if (items != 1) {
		Perl_croak(aTHX_ "Usage: quest::clear_npctype_cache(int npc_type_id)");
	} else {
		int32 npc_type_id = (int32) SvIV(ST(0));
		quest_manager.ClearNPCTypeCache(npc_type_id);
	}

	XSRETURN_EMPTY;
}

XS(XS__reloadzonestaticdata);
XS(XS__reloadzonestaticdata) {
	dXSARGS;

	quest_manager.ReloadZoneStaticData();

	XSRETURN_EMPTY;
}

XS(XS__qs_send_query);
XS(XS__qs_send_query) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: quest::qs_send_query(string query)");
	} else {
		// char *Query = (char *)SvPV_nolen(ST(0));
		std::string Query = (std::string) SvPV_nolen(ST(0));
		QServ->SendQuery(Query);
	}
	XSRETURN_EMPTY;
}

XS(XS__qs_player_event);
XS(XS__qs_player_event) {
	dXSARGS;
	if (items != 2) {
		Perl_croak(aTHX_ "Usage: quest::qs_player_event(int character_id, string message)");
	} else {
		int         char_id = (int) SvIV(ST(0));
		std::string message = (std::string) SvPV_nolen(ST(1));
		QServ->PlayerLogEvent(Player_Log_Quest, char_id, message);
	}
	XSRETURN_EMPTY;
}



XS(XS__log);
XS(XS__log) {
	dXSARGS;
	if (items != 1 && items != 2) {
		Perl_croak(aTHX_ "Usage: quest::log(uint8 log_category, string message)");
	}
	else {
		uint8       log_category = (uint8)SvIV(ST(0));
		std::string log_message = (std::string) SvPV_nolen(ST(1));

		if (log_category >= Logs::MaxCategoryID) {
			return;
		}

		Log(Logs::General, log_category, log_message.c_str());
	}
	XSRETURN_EMPTY;
}

XS(XS__debug);
XS(XS__debug) {
	dXSARGS;
	if (items != 1 && items != 2) {
		Perl_croak(aTHX_ "Usage: quest::debug(string message, [uint8 debug_level = 1 [1-3]])");
	} else {
		std::string log_message = (std::string) SvPV_nolen(ST(0));
		uint8       debug_level = 1;

		if (items == 2)
			debug_level = (uint8) SvIV(ST(1));

		if (debug_level > Logs::Detail)
			return;

		if (debug_level == Logs::General) {
			Log(Logs::General, Logs::QuestDebug, log_message.c_str());
		} else if (debug_level == Logs::Moderate) {
			Log(Logs::Moderate, Logs::QuestDebug, log_message.c_str());
		} else if (debug_level == Logs::Detail) {
			Log(Logs::Detail, Logs::QuestDebug, log_message.c_str());
		}
	}
	XSRETURN_EMPTY;
}

XS(XS__log_combat);
XS(XS__log_combat) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: quest::log_combat(string message)");
	}
	else {

		std::string log_message = (std::string) SvPV_nolen(ST(0));
		Log(Logs::General, Logs::Combat, log_message.c_str());
	}
	XSRETURN_EMPTY;
}


XS(XS__UpdateZoneHeader);
XS(XS__UpdateZoneHeader) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::UpdateZoneHeader(string key, string value)");

	std::string key       = (std::string) SvPV_nolen(ST(0));
	std::string str_value = (std::string) SvPV_nolen(ST(1));
	quest_manager.UpdateZoneHeader(key, str_value);

	XSRETURN_EMPTY;
}

XS(XS__set_rule);
XS(XS__set_rule) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::set_rule(string rule_name, string rule_value)");

	std::string rule_name = (std::string) SvPV_nolen(ST(0));
	std::string rule_value = (std::string) SvPV_nolen(ST(1));
	RuleManager::Instance()->SetRule(rule_name.c_str(), rule_value.c_str());

	XSRETURN_EMPTY;
}

XS(XS__get_rule);
XS(XS__get_rule) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::get_rule(string rule_name)");

	dXSTARG;
	std::string rule_name = (std::string) SvPV_nolen(ST(0));
	std::string rule_value;
	RuleManager::Instance()->GetRule(rule_name.c_str(), rule_value);

	sv_setpv(TARG, rule_value.c_str());
	XSprePUSH;
	PUSHTARG;
	XSRETURN(1);
}

XS(XS__get_data);
XS(XS__get_data) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::get_data(string bucket_key)");

	dXSTARG;
	std::string key = (std::string) SvPV_nolen(ST(0));

	sv_setpv(TARG, DataBucket::GetData(key).c_str());
	XSprePUSH;
	PUSHTARG;
	XSRETURN(1);
}

XS(XS__get_data_expires);
XS(XS__get_data_expires) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::get_data_expires(string bucket_key)");

	dXSTARG;
	std::string key = (std::string) SvPV_nolen(ST(0));

	sv_setpv(TARG, DataBucket::GetDataExpires(key).c_str());
	XSprePUSH;
	PUSHTARG;
	XSRETURN(1);
}

XS(XS__set_data);
XS(XS__set_data) {
	dXSARGS;
	if (items != 2 && items != 3) {
		Perl_croak(aTHX_ "Usage: quest::set_data(string key, string value, [string expires_at = 0])");
	} else {
		std::string key   = (std::string) SvPV_nolen(ST(0));
		std::string value = (std::string) SvPV_nolen(ST(1));

		std::string expires_at;
		if (items == 3)
			expires_at = (std::string) SvPV_nolen(ST(2));

		DataBucket::SetData(key, value, expires_at);
	}
	XSRETURN_EMPTY;
}

XS(XS__delete_data);
XS(XS__delete_data) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::delete_data(string bucket_key)");

	dXSTARG;
	std::string key = (std::string) SvPV_nolen(ST(0));

	XSprePUSH;
	PUSHu((IV) DataBucket::DeleteData(key));

	XSRETURN(1);
}


XS(XS__IsClassicEnabled);
XS(XS__IsClassicEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_classic_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsClassicEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsTheRuinsOfKunarkEnabled);
XS(XS__IsTheRuinsOfKunarkEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_the_ruins_of_kunark_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsTheRuinsOfKunarkEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsTheScarsOfVeliousEnabled);
XS(XS__IsTheScarsOfVeliousEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_the_scars_of_velious_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsTheScarsOfVeliousEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsTheShadowsOfLuclinEnabled);
XS(XS__IsTheShadowsOfLuclinEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_the_shadows_of_luclin_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsTheShadowsOfLuclinEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsThePlanesOfPowerEnabled);
XS(XS__IsThePlanesOfPowerEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_the_planes_of_power_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsThePlanesOfPowerEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsTheLegacyOfYkeshaEnabled);
XS(XS__IsTheLegacyOfYkeshaEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_the_legacy_of_ykesha_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsTheLegacyOfYkeshaEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsLostDungeonsOfNorrathEnabled);
XS(XS__IsLostDungeonsOfNorrathEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_lost_dungeons_of_norrath_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsLostDungeonsOfNorrathEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsGatesOfDiscordEnabled);
XS(XS__IsGatesOfDiscordEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_gates_of_discord_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsGatesOfDiscordEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsOmensOfWarEnabled);
XS(XS__IsOmensOfWarEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_omens_of_war_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsOmensOfWarEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsDragonsOfNorrathEnabled);
XS(XS__IsDragonsOfNorrathEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_dragons_of_norrath_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsDragonsOfNorrathEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsDepthsOfDarkhollowEnabled);
XS(XS__IsDepthsOfDarkhollowEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_depths_of_darkhollow_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsDepthsOfDarkhollowEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsProphecyOfRoEnabled);
XS(XS__IsProphecyOfRoEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_prophecy_of_ro_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsProphecyOfRoEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsTheSerpentsSpineEnabled);
XS(XS__IsTheSerpentsSpineEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_the_serpents_spine_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsTheSerpentsSpineEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsTheBuriedSeaEnabled);
XS(XS__IsTheBuriedSeaEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_the_buried_sea_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsTheBuriedSeaEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsSecretsOfFaydwerEnabled);
XS(XS__IsSecretsOfFaydwerEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_secrets_of_faydwer_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsSecretsOfFaydwerEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsSeedsOfDestructionEnabled);
XS(XS__IsSeedsOfDestructionEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_seeds_of_destruction_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsSeedsOfDestructionEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsUnderfootEnabled);
XS(XS__IsUnderfootEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_underfoot_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsUnderfootEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsHouseOfThuleEnabled);
XS(XS__IsHouseOfThuleEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_house_of_thule_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsHouseOfThuleEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsVeilOfAlarisEnabled);
XS(XS__IsVeilOfAlarisEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_veil_of_alaris_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsVeilOfAlarisEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsRainOfFearEnabled);
XS(XS__IsRainOfFearEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_rain_of_fear_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsRainOfFearEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCallOfTheForsakenEnabled);
XS(XS__IsCallOfTheForsakenEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_call_of_the_forsaken_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCallOfTheForsakenEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsTheDarkendSeaEnabled);
XS(XS__IsTheDarkendSeaEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_the_darkend_sea_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsTheDarkendSeaEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsTheBrokenMirrorEnabled);
XS(XS__IsTheBrokenMirrorEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_the_broken_mirror_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsTheBrokenMirrorEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsEmpiresOfKunarkEnabled);
XS(XS__IsEmpiresOfKunarkEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_empires_of_kunark_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsEmpiresOfKunarkEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsRingOfScaleEnabled);
XS(XS__IsRingOfScaleEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_ring_of_scale_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsRingOfScaleEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsTheBurningLandsEnabled);
XS(XS__IsTheBurningLandsEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_the_burning_lands_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsTheBurningLandsEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsTormentOfVeliousEnabled);
XS(XS__IsTormentOfVeliousEnabled) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_torment_of_velious_enabled()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsTormentOfVeliousEnabled();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionClassic);
XS(XS__IsCurrentExpansionClassic) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_classic()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionClassic();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionTheRuinsOfKunark);
XS(XS__IsCurrentExpansionTheRuinsOfKunark) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_the_ruins_of_kunark()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionTheRuinsOfKunark();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionTheScarsOfVelious);
XS(XS__IsCurrentExpansionTheScarsOfVelious) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_the_scars_of_velious()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionTheScarsOfVelious();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionTheShadowsOfLuclin);
XS(XS__IsCurrentExpansionTheShadowsOfLuclin) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_the_shadows_of_luclin()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionTheShadowsOfLuclin();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionThePlanesOfPower);
XS(XS__IsCurrentExpansionThePlanesOfPower) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_the_planes_of_power()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionThePlanesOfPower();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionTheLegacyOfYkesha);
XS(XS__IsCurrentExpansionTheLegacyOfYkesha) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_the_legacy_of_ykesha()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionTheLegacyOfYkesha();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionLostDungeonsOfNorrath);
XS(XS__IsCurrentExpansionLostDungeonsOfNorrath) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_lost_dungeons_of_norrath()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionLostDungeonsOfNorrath();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionGatesOfDiscord);
XS(XS__IsCurrentExpansionGatesOfDiscord) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_gates_of_discord()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionGatesOfDiscord();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionOmensOfWar);
XS(XS__IsCurrentExpansionOmensOfWar) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_omens_of_war()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionOmensOfWar();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionDragonsOfNorrath);
XS(XS__IsCurrentExpansionDragonsOfNorrath) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_dragons_of_norrath()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionDragonsOfNorrath();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionDepthsOfDarkhollow);
XS(XS__IsCurrentExpansionDepthsOfDarkhollow) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_depths_of_darkhollow()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionDepthsOfDarkhollow();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionProphecyOfRo);
XS(XS__IsCurrentExpansionProphecyOfRo) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_prophecy_of_ro()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionProphecyOfRo();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionTheSerpentsSpine);
XS(XS__IsCurrentExpansionTheSerpentsSpine) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_the_serpents_spine()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionTheSerpentsSpine();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionTheBuriedSea);
XS(XS__IsCurrentExpansionTheBuriedSea) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_the_buried_sea()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionTheBuriedSea();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionSecretsOfFaydwer);
XS(XS__IsCurrentExpansionSecretsOfFaydwer) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_secrets_of_faydwer()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionSecretsOfFaydwer();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionSeedsOfDestruction);
XS(XS__IsCurrentExpansionSeedsOfDestruction) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_seeds_of_destruction()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionSeedsOfDestruction();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionUnderfoot);
XS(XS__IsCurrentExpansionUnderfoot) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_underfoot()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionUnderfoot();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionHouseOfThule);
XS(XS__IsCurrentExpansionHouseOfThule) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_house_of_thule()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionHouseOfThule();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionVeilOfAlaris);
XS(XS__IsCurrentExpansionVeilOfAlaris) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_veil_of_alaris()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionVeilOfAlaris();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionRainOfFear);
XS(XS__IsCurrentExpansionRainOfFear) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_rain_of_fear()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionRainOfFear();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionCallOfTheForsaken);
XS(XS__IsCurrentExpansionCallOfTheForsaken) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_call_of_the_forsaken()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionCallOfTheForsaken();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionTheDarkendSea);
XS(XS__IsCurrentExpansionTheDarkendSea) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_the_darkend_sea()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionTheDarkendSea();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionTheBrokenMirror);
XS(XS__IsCurrentExpansionTheBrokenMirror) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_the_broken_mirror()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionTheBrokenMirror();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionEmpiresOfKunark);
XS(XS__IsCurrentExpansionEmpiresOfKunark) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_empires_of_kunark()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionEmpiresOfKunark();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionRingOfScale);
XS(XS__IsCurrentExpansionRingOfScale) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_ring_of_scale()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionRingOfScale();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionTheBurningLands);
XS(XS__IsCurrentExpansionTheBurningLands) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_the_burning_lands()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionTheBurningLands();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsCurrentExpansionTormentOfVelious);
XS(XS__IsCurrentExpansionTormentOfVelious) {
	dXSARGS;
	if (items >= 1) {
		Perl_croak(aTHX_ "Usage: quest::is_current_expansion_torment_of_velious()");
	}

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsCurrentExpansionTormentOfVelious();
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__IsContentFlagEnabled);
XS(XS__IsContentFlagEnabled) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: quest::is_content_flag_enabled(string flag_name)");
	}

	std::string flag_name = (std::string) SvPV_nolen(ST(0));

	bool RETVAL; dXSTARG;
	RETVAL = content_service.IsContentFlagEnabled(flag_name);
	XSprePUSH; PUSHu((IV) RETVAL); XSRETURN(1);
}

XS(XS__SetContentFlag);
XS(XS__SetContentFlag)
{
	dXSARGS;
	if (items != 2) {
		Perl_croak(aTHX_ "Usage: quest::set_content_flag(string flag_name, enabled)");
	}

	std::string flag_name = (std::string) SvPV_nolen(ST(0));
	bool        enabled   = (int) SvIV(ST(1)) != 0;
	ZoneStore::SetContentFlag(flag_name, enabled);
	XSRETURN_EMPTY;
}

XS(XS__get_expedition);
XS(XS__get_expedition) {
	dXSARGS;
	if (items != 0) {
		Perl_croak(aTHX_ "Usage: quest::get_expedition()");
	}

	Expedition* RETVAL = nullptr;
	if (zone && zone->GetInstanceID() != 0)
	{
		RETVAL = Expedition::FindCachedExpeditionByZoneInstance(zone->GetZoneID(), zone->GetInstanceID());
	}

	EXTEND(sp, 1); // grow stack, function had 0 arguments
	ST(0) = sv_newmortal(); // PUSHs(sv_newmortal());
	if (RETVAL) {
		sv_setref_pv(ST(0), "Expedition", (void*)RETVAL);
	}

	XSRETURN(1);
}

XS(XS__get_expedition_by_char_id);
XS(XS__get_expedition_by_char_id) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: quest::get_expedition_by_char_id(uint32 character_id)");
	}

	uint32 character_id = (int)SvUV(ST(0));

	Expedition* RETVAL = Expedition::FindCachedExpeditionByCharacterID(character_id);

	ST(0) = sv_newmortal();
	if (RETVAL) {
		sv_setref_pv(ST(0), "Expedition", (void*)RETVAL);
	}

	XSRETURN(1);
}

XS(XS__get_expedition_by_dz_id);
XS(XS__get_expedition_by_dz_id) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: quest::get_expedition_by_dz_id(uint32 dynamic_zone_id)");
	}

	uint32 dz_id = (int)SvUV(ST(0));

	Expedition* RETVAL = Expedition::FindCachedExpeditionByDynamicZoneID(dz_id);

	ST(0) = sv_newmortal();
	if (RETVAL) {
		sv_setref_pv(ST(0), "Expedition", (void*)RETVAL);
	}

	XSRETURN(1);
}

XS(XS__get_expedition_by_zone_instance);
XS(XS__get_expedition_by_zone_instance) {
	dXSARGS;
	if (items != 2) {
		Perl_croak(aTHX_ "Usage: quest::GetExpeditionByZoneInstance(uint16 zone_id, uint16 instance_id)");
	}

	uint16 zone_id = (uint16)SvUV(ST(0));
	uint16 instance_id = (uint16)SvUV(ST(1));

	Expedition* RETVAL = Expedition::FindCachedExpeditionByZoneInstance(zone_id, instance_id);

	ST(0) = sv_newmortal();
	if (RETVAL) {
		sv_setref_pv(ST(0), "Expedition", (void*)RETVAL);
	}

	XSRETURN(1);
}

XS(XS__get_expedition_lockout_by_char_id);
XS(XS__get_expedition_lockout_by_char_id) {
	dXSARGS;
	if (items != 3) {
		Perl_croak(aTHX_ "Usage: quest::get_expedition_lockout_by_char_id(uint32 character_id, string expedition_name, string event_name)");
	}

	uint32_t character_id = static_cast<uint32_t>(SvUV(ST(0)));
	std::string expedition_name = SvPV_nolen(ST(1));
	std::string event_name = SvPV_nolen(ST(2));

	auto lockouts = Expedition::GetExpeditionLockoutsByCharacterID(character_id);
	auto it = std::find_if(lockouts.begin(), lockouts.end(), [&](const ExpeditionLockoutTimer& lockout) {
		return lockout.IsSameLockout(expedition_name, event_name);
	});

	// mortalize so its refcnt is auto decremented on function exit to avoid leak
	HV* hash = (HV*)sv_2mortal((SV*)newHV()); // hash refcnt +1 (mortal -1)

	if (it != lockouts.end())
	{
		hv_store(hash, "remaining", strlen("remaining"), newSVuv(it->GetSecondsRemaining()), 0);
		hv_store(hash, "uuid", strlen("uuid"), newSVpv(it->GetExpeditionUUID().c_str(), 0), 0);
	}

	ST(0) = sv_2mortal(newRV((SV*)hash)); // hash refcnt: 2 (-1 mortal), reference: 1 (-1 mortal)
	XSRETURN(1);
}

XS(XS__get_expedition_lockouts_by_char_id);
XS(XS__get_expedition_lockouts_by_char_id) {
	dXSARGS;
	if (items != 1 && items != 2) {
		Perl_croak(aTHX_ "Usage: quest::get_expedition_lockouts_by_char_id(uint32 character_id, [string expedition_name])");
	}

	HV* hash = newHV(); // hash refcnt +1 (non-mortal, newRV_noinc to not inc)
	SV* hash_ref = nullptr; // for expedition event hash if filtering on expedition

	uint32_t character_id = static_cast<uint32_t>(SvUV(ST(0)));
	std::string expedition_name;
	if (items == 2)
	{
		expedition_name = SvPV_nolen(ST(1));
	}

	auto lockouts = Expedition::GetExpeditionLockoutsByCharacterID(character_id);

	for (const auto& lockout : lockouts)
	{
		uint32_t name_len = static_cast<uint32_t>(lockout.GetExpeditionName().size());
		uint32_t event_len = static_cast<uint32_t>(lockout.GetEventName().size());

		// hashes are stored through references inside other hashes/arrays. we need
		// to wrap newHV in newRV references when inserting nested hash values.
		// we use newRV_noinc to not increment the hash's ref count; rv will own it

		SV** entry = hv_fetch(hash, lockout.GetExpeditionName().c_str(), name_len, false);
		if (!entry)
		{
			// create expedition entry in hash with its value as ref to event hash
			SV* event_hash_ref = newRV_noinc((SV*)newHV()); // ref takes ownership
			if (!expedition_name.empty() && lockout.GetExpeditionName() == expedition_name)
			{
				hash_ref = event_hash_ref; // save ref for filtered expedition return
			}
			entry = hv_store(hash, lockout.GetExpeditionName().c_str(), name_len, event_hash_ref, 0);
		}

		// *entry is a reference to expedition's event hash (which it owns). the
		// event entry in the hash will contain ref to a lockout detail hash
		if (entry && SvROK(*entry) && SvTYPE(SvRV(*entry)) == SVt_PVHV) // is ref to hash type
		{
			HV* details_hash = newHV(); // refcnt +1, reference will take ownership
			hv_store(details_hash, "remaining", strlen("remaining"), newSVuv(lockout.GetSecondsRemaining()), 0);
			hv_store(details_hash, "uuid", strlen("uuid"), newSVpv(lockout.GetExpeditionUUID().c_str(), 0), 0);

			HV* event_hash = (HV*)SvRV(*entry);
			hv_store(event_hash, lockout.GetEventName().c_str(), event_len,
				(SV*)newRV_noinc((SV*)details_hash), 0);
		}
	}

	SV* rv = &PL_sv_undef;

	if (!expedition_name.empty())
	{
		rv = hash_ref ? sv_2mortal(hash_ref) : &PL_sv_undef; // ref that owns event hash for expedition
	}
	else
	{
		rv = sv_2mortal(newRV_noinc((SV*)hash)); // takes ownership of expedition hash
	}

	ST(0) = rv;
	XSRETURN(1);
}

XS(XS__add_expedition_lockout_all_clients);
XS(XS__add_expedition_lockout_all_clients) {
	dXSARGS;
	if (items != 3 && items != 4) {
		Perl_croak(aTHX_ "Usage: quest::add_expedition_lockout_all_clients(string expedition_name, string event_name, uint32 seconds, [string uuid])");
	}

	std::string expedition_name = SvPV_nolen(ST(0));
	std::string event_name = SvPV_nolen(ST(1));
	uint32_t seconds = static_cast<uint32_t>(SvUV(ST(2)));
	std::string uuid;

	if (items == 4)
	{
		uuid = SvPV_nolen(ST(3));
	}

	auto lockout = ExpeditionLockoutTimer::CreateLockout(expedition_name, event_name, seconds, uuid);
	Expedition::AddLockoutClients(lockout);

	XSRETURN_EMPTY;
}

XS(XS__add_expedition_lockout_by_char_id);
XS(XS__add_expedition_lockout_by_char_id) {
	dXSARGS;
	if (items != 4 && items != 5) {
		Perl_croak(aTHX_ "Usage: quest::add_expedition_lockout_by_char_id(uint32 character_id, string expedition_name, string event_name, uint32 seconds, [string uuid])");
	}

	std::string uuid;
	if (items == 5)
	{
		uuid = SvPV_nolen(ST(4));
	}

	uint32_t character_id = static_cast<uint32_t>(SvUV(ST(0)));
	std::string expedition_name = SvPV_nolen(ST(1));
	std::string event_name = SvPV_nolen(ST(2));
	uint32_t seconds = static_cast<uint32_t>(SvUV(ST(3)));

	Expedition::AddLockoutByCharacterID(character_id, expedition_name, event_name, seconds, uuid);

	XSRETURN_EMPTY;
}

XS(XS__remove_expedition_lockout_by_char_id);
XS(XS__remove_expedition_lockout_by_char_id) {
	dXSARGS;
	if (items != 3) {
		Perl_croak(aTHX_ "Usage: quest::remove_expedition_lockout_by_char_id(uint32 character_id, string expedition_name, string event_name)");
	}

	uint32_t character_id = static_cast<uint32_t>(SvUV(ST(0)));
	std::string expedition_name = SvPV_nolen(ST(1));
	std::string event_name = SvPV_nolen(ST(2));

	Expedition::RemoveLockoutsByCharacterID(character_id, expedition_name, event_name);

	XSRETURN_EMPTY;
}

XS(XS__remove_all_expedition_lockouts_by_char_id);
XS(XS__remove_all_expedition_lockouts_by_char_id) {
	dXSARGS;
	if (items != 1 && items != 2) {
		Perl_croak(aTHX_ "Usage: quest::remove_expedition_lockout_by_char_id(uint32 character_id, [string expedition_name])");
	}

	std::string expedition_name;
	if (items == 2)
	{
		expedition_name = SvPV_nolen(ST(1));
	}

	uint32_t character_id = static_cast<uint32_t>(SvUV(ST(0)));
	Expedition::RemoveLockoutsByCharacterID(character_id, expedition_name);

	XSRETURN_EMPTY;
}

XS(XS__createitem);
XS(XS__createitem) {
	dXSARGS;
	if (items < 1 || items > 9) {
		Perl_croak(aTHX_ "Usage: quest::createitem(uint32 item_id, [int16 charges = 0, uint32 augment_one = 0, uint32 augment_two = 0, uint32 augment_three = 0, uint32 augment_four = 0, uint32 augment_five = 0, uint32 augment_six = 0, bool attuned = false])");
	}

	EQ::ItemInstance* RETVAL = nullptr;
	uint32 item_id = (uint32)SvUV(ST(0));
	int16 charges = 0;
	uint32 augment_one = 0;
	uint32 augment_two = 0;
	uint32 augment_three = 0;
	uint32 augment_four = 0;
	uint32 augment_five = 0;
	uint32 augment_six = 0;
	bool attuned = false;
	if (items > 1)
		charges = (int16)SvIV(ST(1));
	if (items > 2)
		augment_one = (uint32)SvUV(ST(2));
	if (items > 3)
		augment_two = (uint32)SvUV(ST(3));
	if (items > 4)
		augment_three = (uint32)SvUV(ST(4));
	if (items > 5)
		augment_four = (uint32)SvUV(ST(5));
	if (items > 6)
		augment_five = (uint32)SvUV(ST(6));
	if (items > 7)
		augment_six = (uint32)SvUV(ST(7));
	if (items > 8)
		attuned = (bool)SvNV(ST(8));

	if (database.GetItem(item_id)) {
		RETVAL = database.CreateItem(item_id, charges, augment_one, augment_two, augment_three, augment_four, augment_five, augment_six, attuned);
	}

	ST(0) = sv_newmortal();
	if (RETVAL) {
		sv_setref_pv(ST(0), "QuestItem", (void*)RETVAL);
	}
	XSRETURN(1);
}

XS(XS__secondstotime);
XS(XS__secondstotime) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: quest::secondstotime(int duration)");
	}

	dXSTARG;
	std::string time_string;
	int duration = (int) SvIV(ST(0));
	time_string = quest_manager.secondstotime(duration);
	sv_setpv(TARG, time_string.c_str());
	XSprePUSH;
	PUSHTARG;
	XSRETURN(1);
}

XS(XS__gethexcolorcode);
XS(XS__gethexcolorcode) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: quest::gethexcolorcode(std::string color_name)");
	}

	dXSTARG;
	std::string hex_color_code;
	std::string color_name = SvPV_nolen(ST(0));
	hex_color_code = quest_manager.gethexcolorcode(color_name);
	sv_setpv(TARG, hex_color_code.c_str());
	XSprePUSH;
	PUSHTARG;
	XSRETURN(1);	
}

XS(XS__getaaexpmodifierbycharid);
XS(XS__getaaexpmodifierbycharid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::getaaexpmodifierbycharid(uint32 character_id, uint32 zone_id)");
		
	dXSTARG;
	double aa_modifier;
	uint32 character_id = (uint32) SvUV(ST(0));
	uint32 zone_id = (uint32) SvUV(ST(1));
	aa_modifier = quest_manager.GetAAEXPModifierByCharID(character_id, zone_id);
	XSprePUSH;
	PUSHn((double) aa_modifier);
	XSRETURN(1);
}

XS(XS__getexpmodifierbycharid);
XS(XS__getexpmodifierbycharid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::getexpmodifierbycharid(uint32 character_id, uint32 zone_id)");
		
	dXSTARG;
	double exp_modifier;
	uint32 character_id = (uint32) SvUV(ST(0));
	uint32 zone_id = (uint32) SvUV(ST(1));
	exp_modifier = quest_manager.GetEXPModifierByCharID(character_id, zone_id);
	XSprePUSH;
	PUSHn((double) exp_modifier);
	XSRETURN(1);
}

XS(XS__setaaexpmodifierbycharid);
XS(XS__setaaexpmodifierbycharid) {
	dXSARGS;
	if (items != 3) {
		Perl_croak(aTHX_ "Usage: quest::setaaexpmodifierbycharid(uint32 character_id, uint32 zone_id, float aa_modifier)");
	}
	uint32 character_id = (uint32) SvUV(ST(0));
	uint32 zone_id = (uint32) SvUV(ST(1));
	double aa_modifier = (double) SvNV(ST(2));
	quest_manager.SetAAEXPModifierByCharID(character_id, zone_id, aa_modifier);
	XSRETURN_EMPTY;
}

XS(XS__setexpmodifierbycharid);
XS(XS__setexpmodifierbycharid) {
	dXSARGS;
	if (items != 3) {
		Perl_croak(aTHX_ "Usage: quest::setexpmodifierbycharid(uint32 character_id, uint32 zone_id, float exp_modifier)");
	}
	uint32 character_id = (uint32) SvUV(ST(0));
	uint32 zone_id = (uint32) SvUV(ST(1));
	double exp_modifier = (double) SvNV(ST(2));
	quest_manager.SetEXPModifierByCharID(character_id, zone_id, exp_modifier);
	XSRETURN_EMPTY;
}

XS(XS__crosszoneaddldonlossbycharid);
XS(XS__crosszoneaddldonlossbycharid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszoneaddldonlossbycharid(int character_id, uint32 theme_id)");

	uint8 update_type = CZLDoNUpdateType_Character;
	uint8 update_subtype = CZLDoNUpdateSubtype_Loss;
	int character_id = (int) SvIV(ST(0));
	uint32 theme_id = (uint32) SvUV(ST(1));
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, character_id, theme_id);
	XSRETURN_EMPTY;
}

XS(XS__crosszoneaddldonpointsbycharid);
XS(XS__crosszoneaddldonpointsbycharid) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: quest::crosszoneaddldonpointsbycharid(int character_id, uint32 theme_id, int points)");

	uint8 update_type = CZLDoNUpdateType_Character;
	uint8 update_subtype = CZLDoNUpdateSubtype_Points;
	int character_id = (int) SvIV(ST(0));
	uint32 theme_id = (uint32) SvUV(ST(1));
	int points = (int) SvIV(ST(2));
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, character_id, theme_id, points);
	XSRETURN_EMPTY;
}

XS(XS__crosszoneaddldonwinbycharid);
XS(XS__crosszoneaddldonwinbycharid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszoneaddldonwinbycharid(int character_id, uint32 theme_id)");

	uint8 update_type = CZLDoNUpdateType_Character;
	uint8 update_subtype = CZLDoNUpdateSubtype_Win;
	int character_id = (int) SvIV(ST(0));
	uint32 theme_id = (uint32) SvUV(ST(1));
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, character_id, theme_id);
	XSRETURN_EMPTY;
}

XS(XS__crosszoneaddldonlossbygroupid);
XS(XS__crosszoneaddldonlossbygroupid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszoneaddldonlossbygroupid(int group_id, uint32 theme_id)");

	uint8 update_type = CZLDoNUpdateType_Group;
	uint8 update_subtype = CZLDoNUpdateSubtype_Loss;
	int group_id = (int) SvIV(ST(0));
	uint32 theme_id = (uint32) SvUV(ST(1));
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, group_id, theme_id);
	XSRETURN_EMPTY;
}

XS(XS__crosszoneaddldonpointsbygroupid);
XS(XS__crosszoneaddldonpointsbygroupid) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: quest::crosszoneaddldonpointsbygroupid(int group_id, uint32 theme_id, int points)");

	uint8 update_type = CZLDoNUpdateType_Group;
	uint8 update_subtype = CZLDoNUpdateSubtype_Points;
	int group_id = (int) SvIV(ST(0));
	uint32 theme_id = (uint32) SvUV(ST(1));
	int points = (int) SvIV(ST(2));
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, group_id, theme_id, points);
	XSRETURN_EMPTY;
}

XS(XS__crosszoneaddldonwinbygroupid);
XS(XS__crosszoneaddldonwinbygroupid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszoneaddldonwinbygroupid(int group_id, uint32 theme_id)");

	uint8 update_type = CZLDoNUpdateType_Group;
	uint8 update_subtype = CZLDoNUpdateSubtype_Win;
	int group_id = (int) SvIV(ST(0));
	uint32 theme_id = (uint32) SvUV(ST(1));
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, group_id, theme_id);
	XSRETURN_EMPTY;
}

XS(XS__crosszoneaddldonlossbyraidid);
XS(XS__crosszoneaddldonlossbyraidid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszoneaddldonlossbyraidid(int raid_id, uint32 theme_id)");

	uint8 update_type = CZLDoNUpdateType_Raid;
	uint8 update_subtype = CZLDoNUpdateSubtype_Loss;
	int raid_id = (int) SvIV(ST(0));
	uint32 theme_id = (uint32) SvUV(ST(1));
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, raid_id, theme_id);
	XSRETURN_EMPTY;
}

XS(XS__crosszoneaddldonpointsbyraidid);
XS(XS__crosszoneaddldonpointsbyraidid) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: quest::crosszoneaddldonpointsbyraidid(int raid_id, uint32 theme_id, int points)");

	uint8 update_type = CZLDoNUpdateType_Raid;
	uint8 update_subtype = CZLDoNUpdateSubtype_Points;
	int raid_id = (int) SvIV(ST(0));
	uint32 theme_id = (uint32) SvUV(ST(1));
	int points = (int) SvIV(ST(2));
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, raid_id, theme_id, points);
	XSRETURN_EMPTY;
}

XS(XS__crosszoneaddldonwinbyraidid);
XS(XS__crosszoneaddldonwinbyraidid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszoneaddldonwinbyraidid(int raid_id, uint32 theme_id)");

	uint8 update_type = CZLDoNUpdateType_Raid;
	uint8 update_subtype = CZLDoNUpdateSubtype_Win;
	int raid_id = (int) SvIV(ST(0));
	uint32 theme_id = (uint32) SvUV(ST(1));
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, raid_id, theme_id);
	XSRETURN_EMPTY;
}

XS(XS__crosszoneaddldonlossbyguildid);
XS(XS__crosszoneaddldonlossbyguildid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszoneaddldonlossbyguildid(int guild_id, uint32 theme_id)");

	uint8 update_type = CZLDoNUpdateType_Guild;
	uint8 update_subtype = CZLDoNUpdateSubtype_Loss;
	int guild_id = (int) SvIV(ST(0));
	uint32 theme_id = (uint32) SvUV(ST(1));
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, guild_id, theme_id);
	XSRETURN_EMPTY;
}

XS(XS__crosszoneaddldonpointsbyguildid);
XS(XS__crosszoneaddldonpointsbyguildid) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: quest::crosszoneaddldonpointsbyguildid(int guild_id, uint32 theme_id, int points)");

	uint8 update_type = CZLDoNUpdateType_Guild;
	uint8 update_subtype = CZLDoNUpdateSubtype_Points;
	int guild_id = (int) SvIV(ST(0));
	uint32 theme_id = (uint32) SvUV(ST(1));
	int points = (int) SvIV(ST(2));
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, guild_id, theme_id, points);
	XSRETURN_EMPTY;
}

XS(XS__crosszoneaddldonwinbyguildid);
XS(XS__crosszoneaddldonwinbyguildid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszoneaddldonwinbyguildid(int guild_id, uint32 theme_id)");

	uint8 update_type = CZLDoNUpdateType_Guild;
	uint8 update_subtype = CZLDoNUpdateSubtype_Win;
	int guild_id = (int) SvIV(ST(0));
	uint32 theme_id = (uint32) SvUV(ST(1));
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, guild_id, theme_id);
	XSRETURN_EMPTY;
}

XS(XS__crosszoneaddldonlossbyexpeditionid);
XS(XS__crosszoneaddldonlossbyexpeditionid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszoneaddldonlossbyexpeditionid(uint32 expedition_id, uint32 theme_id)");

	uint8 update_type = CZLDoNUpdateType_Expedition;
	uint8 update_subtype = CZLDoNUpdateSubtype_Loss;
	uint32 expedition_id = (uint32) SvUV(ST(0));
	uint32 theme_id = (uint32) SvUV(ST(1));
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, expedition_id, theme_id);
	XSRETURN_EMPTY;
}

XS(XS__crosszoneaddldonpointsbyexpeditionid);
XS(XS__crosszoneaddldonpointsbyexpeditionid) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: quest::crosszoneaddldonpointsbyexpeditionid(uint32 expedition_id, uint32 theme_id, int points)");

	uint8 update_type = CZLDoNUpdateType_Expedition;
	uint8 update_subtype = CZLDoNUpdateSubtype_Points;
	uint32 expedition_id = (uint32) SvUV(ST(0));
	uint32 theme_id = (uint32) SvUV(ST(1));
	int points = (int) SvIV(ST(2));
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, expedition_id, theme_id, points);
	XSRETURN_EMPTY;
}

XS(XS__crosszoneaddldonwinbyexpeditionid);
XS(XS__crosszoneaddldonwinbyexpeditionid) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: quest::crosszoneaddldonwinbyexpeditionid(uint32 expedition_id, uint32 theme_id)");

	uint8 update_type = CZLDoNUpdateType_Expedition;
	uint8 update_subtype = CZLDoNUpdateSubtype_Win;
	uint32 expedition_id = (uint32) SvUV(ST(0));
	uint32 theme_id = (uint32) SvUV(ST(1));
	quest_manager.CrossZoneLDoNUpdate(update_type, update_subtype, expedition_id, theme_id);
	XSRETURN_EMPTY;
}

XS(XS__getcleannpcnamebyid);
XS(XS__getcleannpcnamebyid) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::getcleannpcnamebyid(uint32 npc_id)");

	dXSTARG;
	uint32 npc_id = (uint32) SvUV(ST(0));
	auto npc_name = quest_manager.getcleannpcnamebyid(npc_id);
	sv_setpv(TARG, npc_name.c_str());
	XSprePUSH;
	PUSHTARG;
	XSRETURN(1);
}

XS(XS__getgendername);
XS(XS__getgendername) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::getgendername(uint32 gender_id)");

	dXSTARG;
	uint32 gender_id = (uint32) SvUV(ST(0));
	auto gender_name = quest_manager.getgendername(gender_id);
	sv_setpv(TARG, gender_name.c_str());
}

XS(XS__getdeityname);
XS(XS__getdeityname) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::getdeityname(uint32 deity_id)");

	dXSTARG;
	uint32 deity_id = (uint32) SvUV(ST(0));
	auto deity_name = quest_manager.getdeityname(deity_id);
	sv_setpv(TARG, deity_name.c_str());
}

XS(XS__getinventoryslotname);
XS(XS__getinventoryslotname) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::getinventoryslotname(int16 slot_id)");

	dXSTARG;
	int16 slot_id = (int16) SvIV(ST(0));
	auto slot_name = quest_manager.getinventoryslotname(slot_id);
	sv_setpv(TARG, slot_name.c_str());
	XSprePUSH;
	PUSHTARG;
	XSRETURN(1);
}

XS(XS__rename);
XS(XS__rename) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::rename(string name)");

	std::string name = (std::string) SvPV_nolen(ST(0));
	quest_manager.rename(name);
	XSRETURN_EMPTY;
}

XS(XS__get_data_remaining);
XS(XS__get_data_remaining) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: quest::get_data_remaining(string bucket_name)");

	dXSTARG;
	std::string bucket_name = (std::string) SvPV_nolen(ST(0));

	sv_setpv(TARG, DataBucket::GetDataRemaining(bucket_name).c_str());
	XSprePUSH;
	PUSHTARG;
	XSRETURN(1);
}

/*
This is the callback perl will look for to setup the
quest package's XSUBs
*/
EXTERN_C XS(boot_quest); // prototype to pass -Wmissing-prototypes
EXTERN_C XS(boot_quest) {
	dXSARGS;
	char file[256];
	strncpy(file, __FILE__, 256);
	file[255] = '\0';

	if (items != 1)
		LogError("boot_quest does not take any arguments");

	char buf[128];    //shouldent have any function names longer than this.

	//add the strcpy stuff to get rid of const warnings....

	XS_VERSION_BOOTCHECK;

#ifdef BOTS
	newXS(strcpy(buf, "botquest"), XS__botquest, file);
	newXS(strcpy(buf, "spawnbotcount"), XS__spawnbotcount, file);
	newXS(strcpy(buf, "createbotcount"), XS__createbotcount, file);
	newXS(strcpy(buf, "createBot"), XS__createBot, file);
#endif //BOTS

	newXS(strcpy(buf, "AssignGroupToInstance"), XS__AssignGroupToInstance, file);
	newXS(strcpy(buf, "AssignRaidToInstance"), XS__AssignRaidToInstance, file);
	newXS(strcpy(buf, "AssignToInstance"), XS__AssignToInstance, file);
	newXS(strcpy(buf, "AssignToInstanceByCharID"), XS__AssignToInstanceByCharID, file);
	newXS(strcpy(buf, "ChooseRandom"), XS__ChooseRandom, file);
	newXS(strcpy(buf, "CreateInstance"), XS__CreateInstance, file);
	newXS(strcpy(buf, "DestroyInstance"), XS__DestroyInstance, file);
	newXS(strcpy(buf, "UpdateInstanceTimer"), XS__UpdateInstanceTimer, file);
	newXS(strcpy(buf, "GetInstanceTimer"), XS__GetInstanceTimer, file);
	newXS(strcpy(buf, "GetInstanceTimerByID"), XS__GetInstanceTimerByID, file);
	newXS(strcpy(buf, "FlagInstanceByGroupLeader"), XS__FlagInstanceByGroupLeader, file);
	newXS(strcpy(buf, "FlagInstanceByRaidLeader"), XS__FlagInstanceByRaidLeader, file);
	newXS(strcpy(buf, "FlyMode"), XS__FlyMode, file);
	newXS(strcpy(buf, "GetCharactersInInstance"), XS__GetCharactersInInstance, file);
	newXS(strcpy(buf, "GetInstanceID"), XS__GetInstanceID, file);
	newXS(strcpy(buf, "GetInstanceIDByCharID"), XS__GetInstanceIDByCharID, file);
	newXS(strcpy(buf, "GetSpellResistType"), XS__GetSpellResistType, file);
	newXS(strcpy(buf, "GetSpellTargetType"), XS__GetSpellTargetType, file);
	newXS(strcpy(buf, "GetTimeSeconds"), XS__GetTimeSeconds, file);
	newXS(strcpy(buf, "GetZoneID"), XS__GetZoneID, file);
	newXS(strcpy(buf, "GetZoneLongName"), XS__GetZoneLongName, file);
	newXS(strcpy(buf, "GetZoneLongNameByID"), XS__GetZoneLongNameByID, file);
	newXS(strcpy(buf, "GetZoneShortName"), XS__GetZoneShortName, file);
	newXS(strcpy(buf, "set_rule"), XS__set_rule, file);
	newXS(strcpy(buf, "get_rule"), XS__get_rule, file);
	newXS(strcpy(buf, "get_data"), XS__get_data, file);
	newXS(strcpy(buf, "get_data_expires"), XS__get_data_expires, file);
	newXS(strcpy(buf, "get_data_remaining"), XS__get_data_remaining, file);
	newXS(strcpy(buf, "set_data"), XS__set_data, file);
	newXS(strcpy(buf, "delete_data"), XS__delete_data, file);
	newXS(strcpy(buf, "IsBeneficialSpell"), XS__IsBeneficialSpell, file);
	newXS(strcpy(buf, "IsEffectInSpell"), XS__IsEffectInSpell, file);
	newXS(strcpy(buf, "IsRunning"), XS__IsRunning, file);
	newXS(strcpy(buf, "LearnRecipe"), XS__LearnRecipe, file);
	newXS(strcpy(buf, "MerchantCountItem"), XS__MerchantCountItem, file);
	newXS(strcpy(buf, "MerchantSetItem"), XS__MerchantSetItem, file);
	newXS(strcpy(buf, "MovePCInstance"), XS__MovePCInstance, file);
	newXS(strcpy(buf, "RemoveAllFromInstance"), XS__RemoveAllFromInstance, file);
	newXS(strcpy(buf, "RemoveFromInstance"), XS__RemoveFromInstance, file);
	newXS(strcpy(buf, "RemoveFromInstanceByCharID"), XS__RemoveFromInstanceByCharID, file);
	newXS(strcpy(buf, "CheckInstanceByCharID"), XS__CheckInstanceByCharID, file);
	newXS(strcpy(buf, "SendMail"), XS__SendMail, file);
	newXS(strcpy(buf, "SetRunning"), XS__SetRunning, file);
	newXS(strcpy(buf, "activespeakactivity"), XS__activespeakactivity, file);
	newXS(strcpy(buf, "activespeaktask"), XS__activespeaktask, file);
	newXS(strcpy(buf, "activetasksinset"), XS__activetasksinset, file);
	newXS(strcpy(buf, "add_expedition_lockout_all_clients"), XS__add_expedition_lockout_all_clients, file);
	newXS(strcpy(buf, "add_expedition_lockout_by_char_id"), XS__add_expedition_lockout_by_char_id, file);
	newXS(strcpy(buf, "addldonloss"), XS__addldonloss, file);
	newXS(strcpy(buf, "addldonpoints"), XS__addldonpoints, file);
	newXS(strcpy(buf, "addldonwin"), XS__addldonwin, file);
	newXS(strcpy(buf, "addloot"), XS__addloot, file);
	newXS(strcpy(buf, "addskill"), XS__addskill, file);
	newXS(strcpy(buf, "assigntask"), XS__assigntask, file);
	newXS(strcpy(buf, "attack"), XS__attack, file);
	newXS(strcpy(buf, "attacknpc"), XS__attacknpc, file);
	newXS(strcpy(buf, "attacknpctype"), XS__attacknpctype, file);
	newXS(strcpy(buf, "buryplayercorpse"), XS__buryplayercorpse, file);
	newXS(strcpy(buf, "castspell"), XS__castspell, file);
	newXS(strcpy(buf, "changedeity"), XS__changedeity, file);
	newXS(strcpy(buf, "checktitle"), XS__checktitle, file);
	newXS(strcpy(buf, "clear_npctype_cache"), XS__clear_npctype_cache, file);
	newXS(strcpy(buf, "clear_proximity"), XS__clear_proximity, file);
	newXS(strcpy(buf, "clear_zone_flag"), XS__clear_zone_flag, file);
	newXS(strcpy(buf, "clearspawntimers"), XS__clearspawntimers, file);
	newXS(strcpy(buf, "collectitems"), XS__collectitems, file);
	newXS(strcpy(buf, "completedtasksinset"), XS__completedtasksinset, file);
	newXS(strcpy(buf, "countitem"), XS__countitem, file);
	newXS(strcpy(buf, "createdoor"), XS__CreateDoor, file);
	newXS(strcpy(buf, "creategroundobject"), XS__CreateGroundObject, file);
	newXS(strcpy(buf, "creategroundobjectfrommodel"), XS__CreateGroundObjectFromModel, file);
	newXS(strcpy(buf, "createguild"), XS__createguild, file);
	newXS(strcpy(buf, "createitem"), XS__createitem, file);
	newXS(strcpy(buf, "crosszoneaddldonlossbycharid"), XS__crosszoneaddldonlossbycharid, file);
	newXS(strcpy(buf, "crosszoneaddldonlossbygroupid"), XS__crosszoneaddldonlossbygroupid, file);
	newXS(strcpy(buf, "crosszoneaddldonlossbyraidid"), XS__crosszoneaddldonlossbyraidid, file);
	newXS(strcpy(buf, "crosszoneaddldonlossbyguildid"), XS__crosszoneaddldonlossbyguildid, file);
	newXS(strcpy(buf, "crosszoneaddldonlossbyexpeditionid"), XS__crosszoneaddldonlossbyexpeditionid, file);
	newXS(strcpy(buf, "crosszoneaddldonpointsbycharid"), XS__crosszoneaddldonpointsbycharid, file);
	newXS(strcpy(buf, "crosszoneaddldonpointsbygroupid"), XS__crosszoneaddldonpointsbygroupid, file);
	newXS(strcpy(buf, "crosszoneaddldonpointsbyraidid"), XS__crosszoneaddldonpointsbyraidid, file);
	newXS(strcpy(buf, "crosszoneaddldonpointsbyguildid"), XS__crosszoneaddldonpointsbyguildid, file);
	newXS(strcpy(buf, "crosszoneaddldonpointsbyexpeditionid"), XS__crosszoneaddldonpointsbyexpeditionid, file);
	newXS(strcpy(buf, "crosszoneaddldonwinbycharid"), XS__crosszoneaddldonwinbycharid, file);
	newXS(strcpy(buf, "crosszoneaddldonwinbygroupid"), XS__crosszoneaddldonwinbygroupid, file);
	newXS(strcpy(buf, "crosszoneaddldonwinbyraidid"), XS__crosszoneaddldonwinbyraidid, file);
	newXS(strcpy(buf, "crosszoneaddldonwinbyguildid"), XS__crosszoneaddldonwinbyguildid, file);
	newXS(strcpy(buf, "crosszoneaddldonwinbyexpeditionid"), XS__crosszoneaddldonwinbyexpeditionid, file);
	newXS(strcpy(buf, "crosszoneassigntaskbycharid"), XS__crosszoneassigntaskbycharid, file);
	newXS(strcpy(buf, "crosszoneassigntaskbygroupid"), XS__crosszoneassigntaskbygroupid, file);
	newXS(strcpy(buf, "crosszoneassigntaskbyraidid"), XS__crosszoneassigntaskbyraidid, file);
	newXS(strcpy(buf, "crosszoneassigntaskbyguildid"), XS__crosszoneassigntaskbyguildid, file);
	newXS(strcpy(buf, "crosszonecastspellbycharid"), XS__crosszonecastspellbycharid, file);
	newXS(strcpy(buf, "crosszonecastspellbygroupid"), XS__crosszonecastspellbygroupid, file);
	newXS(strcpy(buf, "crosszonecastspellbyraidid"), XS__crosszonecastspellbyraidid, file);
	newXS(strcpy(buf, "crosszonecastspellbyguildid"), XS__crosszonecastspellbyguildid, file);
	newXS(strcpy(buf, "crosszonedisabletaskbycharid"), XS__crosszonedisabletaskbycharid, file);
	newXS(strcpy(buf, "crosszonedisabletaskbygroupid"), XS__crosszonedisabletaskbygroupid, file);
	newXS(strcpy(buf, "crosszonedisabletaskbyraidid"), XS__crosszonedisabletaskbyraidid, file);
	newXS(strcpy(buf, "crosszonedisabletaskbyguildid"), XS__crosszonedisabletaskbyguildid, file);
	newXS(strcpy(buf, "crosszoneenabletaskbycharid"), XS__crosszoneenabletaskbycharid, file);
	newXS(strcpy(buf, "crosszoneenabletaskbygroupid"), XS__crosszoneenabletaskbygroupid, file);
	newXS(strcpy(buf, "crosszoneenabletaskbyraidid"), XS__crosszoneenabletaskbyraidid, file);
	newXS(strcpy(buf, "crosszoneenabletaskbyguildid"), XS__crosszoneenabletaskbyguildid, file);
	newXS(strcpy(buf, "crosszonefailtaskbycharid"), XS__crosszonefailtaskbycharid, file);
	newXS(strcpy(buf, "crosszonefailtaskbygroupid"), XS__crosszonefailtaskbygroupid, file);
	newXS(strcpy(buf, "crosszonefailtaskbyraidid"), XS__crosszonefailtaskbyraidid, file);
	newXS(strcpy(buf, "crosszonefailtaskbyguildid"), XS__crosszonefailtaskbyguildid, file);
	newXS(strcpy(buf, "crosszonemarqueebycharid"), XS__crosszonemarqueebycharid, file);
	newXS(strcpy(buf, "crosszonemarqueebygroupid"), XS__crosszonemarqueebygroupid, file);
	newXS(strcpy(buf, "crosszonemarqueebyraidid"), XS__crosszonemarqueebyraidid, file);
	newXS(strcpy(buf, "crosszonemarqueebyguildid"), XS__crosszonemarqueebyguildid, file);
	newXS(strcpy(buf, "crosszonemessageplayerbyname"), XS__crosszonemessageplayerbyname, file);
	newXS(strcpy(buf, "crosszonemessageplayerbygroupid"), XS__crosszonemessageplayerbygroupid, file);
	newXS(strcpy(buf, "crosszonemessageplayerbyraidid"), XS__crosszonemessageplayerbyraidid, file);
	newXS(strcpy(buf, "crosszonemessageplayerbyguildid"), XS__crosszonemessageplayerbyguildid, file);
	newXS(strcpy(buf, "crosszonemoveplayerbycharid"), XS__crosszonemoveplayerbycharid, file);
	newXS(strcpy(buf, "crosszonemoveplayerbygroupid"), XS__crosszonemoveplayerbygroupid, file);
	newXS(strcpy(buf, "crosszonemoveplayerbyraidid"), XS__crosszonemoveplayerbyraidid, file);
	newXS(strcpy(buf, "crosszonemoveplayerbyguildid"), XS__crosszonemoveplayerbyguildid, file);
	newXS(strcpy(buf, "crosszonemoveinstancebycharid"), XS__crosszonemoveinstancebycharid, file);
	newXS(strcpy(buf, "crosszonemoveinstancebygroupid"), XS__crosszonemoveinstancebygroupid, file);
	newXS(strcpy(buf, "crosszonemoveinstancebyraidid"), XS__crosszonemoveinstancebyraidid, file);
	newXS(strcpy(buf, "crosszonemoveinstancebyguildid"), XS__crosszonemoveinstancebyguildid, file);
	newXS(strcpy(buf, "crosszoneremovespellbycharid"), XS__crosszoneremovespellbycharid, file);
	newXS(strcpy(buf, "crosszoneremovespellbygroupid"), XS__crosszoneremovespellbygroupid, file);
	newXS(strcpy(buf, "crosszoneremovespellbyraidid"), XS__crosszoneremovespellbyraidid, file);
	newXS(strcpy(buf, "crosszoneremovespellbyguildid"), XS__crosszoneremovespellbyguildid, file);
	newXS(strcpy(buf, "crosszoneremovetaskbycharid"), XS__crosszoneremovetaskbycharid, file);
	newXS(strcpy(buf, "crosszoneremovetaskbygroupid"), XS__crosszoneremovetaskbygroupid, file);
	newXS(strcpy(buf, "crosszoneremovetaskbyraidid"), XS__crosszoneremovetaskbyraidid, file);
	newXS(strcpy(buf, "crosszoneremovetaskbyguildid"), XS__crosszoneremovetaskbyguildid, file);
	newXS(strcpy(buf, "crosszoneresetactivitybycharid"), XS__crosszoneresetactivitybycharid, file);
	newXS(strcpy(buf, "crosszoneresetactivitybygroupid"), XS__crosszoneresetactivitybygroupid, file);
	newXS(strcpy(buf, "crosszoneresetactivitybyraidid"), XS__crosszoneresetactivitybyraidid, file);
	newXS(strcpy(buf, "crosszoneresetactivitybyguildid"), XS__crosszoneresetactivitybyguildid, file);
	newXS(strcpy(buf, "crosszonesetentityvariablebynpctypeid"), XS__crosszonesetentityvariablebynpctypeid, file);
	newXS(strcpy(buf, "crosszonesetentityvariablebyclientname"), XS__crosszonesetentityvariablebyclientname, file);
	newXS(strcpy(buf, "crosszonesetentityvariablebygroupid"), XS__crosszonesetentityvariablebygroupid, file);
	newXS(strcpy(buf, "crosszonesetentityvariablebyraidid"), XS__crosszonesetentityvariablebyraidid, file);
	newXS(strcpy(buf, "crosszonesetentityvariablebyguildid"), XS__crosszonesetentityvariablebyguildid, file);
	newXS(strcpy(buf, "crosszonesignalclientbycharid"), XS__crosszonesignalclientbycharid, file);
	newXS(strcpy(buf, "crosszonesignalclientbygroupid"), XS__crosszonesignalclientbygroupid, file);
	newXS(strcpy(buf, "crosszonesignalclientbyraidid"), XS__crosszonesignalclientbyraidid, file);
	newXS(strcpy(buf, "crosszonesignalclientbyguildid"), XS__crosszonesignalclientbyguildid, file);
	newXS(strcpy(buf, "crosszonesignalclientbyname"), XS__crosszonesignalclientbyname, file);
	newXS(strcpy(buf, "crosszonesignalnpcbynpctypeid"), XS__crosszonesignalnpcbynpctypeid, file);
	newXS(strcpy(buf, "crosszoneupdateactivitybycharid"), XS__crosszoneupdateactivitybycharid, file);
	newXS(strcpy(buf, "crosszoneupdateactivitybygroupid"), XS__crosszoneupdateactivitybygroupid, file);
	newXS(strcpy(buf, "crosszoneupdateactivitybyraidid"), XS__crosszoneupdateactivitybyraidid, file);
	newXS(strcpy(buf, "crosszoneupdateactivitybyguildid"), XS__crosszoneupdateactivitybyguildid, file);
	newXS(strcpy(buf, "worldwidecastspell"), XS__worldwidecastspell, file);
	newXS(strcpy(buf, "worldwidedisabletask"), XS__worldwidedisabletask, file);
	newXS(strcpy(buf, "worldwideenabletask"), XS__worldwideenabletask, file);
	newXS(strcpy(buf, "worldwidefailtask"), XS__worldwidefailtask, file);
	newXS(strcpy(buf, "worldwidemarquee"), XS__worldwidemarquee, file);
	newXS(strcpy(buf, "worldwidemessage"), XS__worldwidemessage, file);
	newXS(strcpy(buf, "worldwidemove"), XS__worldwidemove, file);
	newXS(strcpy(buf, "worldwidemoveinstance"), XS__worldwidemoveinstance, file);
	newXS(strcpy(buf, "worldwideremovespell"), XS__worldwideremovespell, file);
	newXS(strcpy(buf, "worldwideremovetask"), XS__worldwideremovetask, file);
	newXS(strcpy(buf, "worldwideresetactivity"), XS__worldwideresetactivity, file);
	newXS(strcpy(buf, "worldwidesetentityvariableclient"), XS__worldwidesetentityvariableclient, file);
	newXS(strcpy(buf, "worldwidesetentityvariablenpc"), XS__worldwidesetentityvariablenpc, file);
	newXS(strcpy(buf, "worldwidesignalclient"), XS__worldwidesignalclient, file);
	newXS(strcpy(buf, "worldwidesignalnpc"), XS__worldwidesignalnpc, file);
	newXS(strcpy(buf, "worldwideupdateactivity"), XS__worldwideupdateactivity, file);
	newXS(strcpy(buf, "debug"), XS__debug, file);
	newXS(strcpy(buf, "delglobal"), XS__delglobal, file);
	newXS(strcpy(buf, "depop"), XS__depop, file);
	newXS(strcpy(buf, "depop_withtimer"), XS__depop_withtimer, file);
	newXS(strcpy(buf, "depopall"), XS__depopall, file);
	newXS(strcpy(buf, "depopzone"), XS__depopzone, file);
	newXS(strcpy(buf, "ding"), XS__ding, file);
	newXS(strcpy(buf, "disable_proximity_say"), XS__disable_proximity_say, file);
	newXS(strcpy(buf, "disable_spawn2"), XS__disable_spawn2, file);
	newXS(strcpy(buf, "disablerecipe"), XS__disablerecipe, file);
	newXS(strcpy(buf, "disabletask"), XS__disabletask, file);
	newXS(strcpy(buf, "doanim"), XS__doanim, file);
	newXS(strcpy(buf, "echo"), XS__echo, file);
	newXS(strcpy(buf, "emote"), XS__emote, file);
	newXS(strcpy(buf, "enable_proximity_say"), XS__enable_proximity_say, file);
	newXS(strcpy(buf, "enable_spawn2"), XS__enable_spawn2, file);
	newXS(strcpy(buf, "enabledtaskcount"), XS__enabledtaskcount, file);
	newXS(strcpy(buf, "enablerecipe"), XS__enablerecipe, file);
	newXS(strcpy(buf, "enabletask"), XS__enabletask, file);
	newXS(strcpy(buf, "enabletitle"), XS__enabletitle, file);
	newXS(strcpy(buf, "exp"), XS__exp, file);
	newXS(strcpy(buf, "faction"), XS__faction, file);
	newXS(strcpy(buf, "factionvalue"), XS_FactionValue, file);
	newXS(strcpy(buf, "failtask"), XS__failtask, file);
	newXS(strcpy(buf, "firsttaskinset"), XS__firsttaskinset, file);
	newXS(strcpy(buf, "follow"), XS__follow, file);
	newXS(strcpy(buf, "forcedoorclose"), XS__forcedoorclose, file);
	newXS(strcpy(buf, "forcedooropen"), XS__forcedooropen, file);
	newXS(strcpy(buf, "getaaexpmodifierbycharid"), XS__getaaexpmodifierbycharid, file);
	newXS(strcpy(buf, "getcharidbyname"), XS__getcharidbyname, file);
	newXS(strcpy(buf, "getclassname"), XS__getclassname, file);
	newXS(strcpy(buf, "getcleannpcnamebyid"), XS__getcleannpcnamebyid, file);
	newXS(strcpy(buf, "gethexcolorcode"), XS__gethexcolorcode, file);
	newXS(strcpy(buf, "getcurrencyid"), XS__getcurrencyid, file);
	newXS(strcpy(buf, "getexpmodifierbycharid"), XS__getexpmodifierbycharid, file);
	newXS(strcpy(buf, "get_expedition"), XS__get_expedition, file);
	newXS(strcpy(buf, "get_expedition_by_char_id"), XS__get_expedition_by_char_id, file);
	newXS(strcpy(buf, "get_expedition_by_dz_id"), XS__get_expedition_by_dz_id, file);
	newXS(strcpy(buf, "get_expedition_by_zone_instance"), XS__get_expedition_by_zone_instance, file);
	newXS(strcpy(buf, "get_expedition_lockout_by_char_id"), XS__get_expedition_lockout_by_char_id, file);
	newXS(strcpy(buf, "get_expedition_lockouts_by_char_id"), XS__get_expedition_lockouts_by_char_id, file);
	newXS(strcpy(buf, "getinventoryslotid"), XS__getinventoryslotid, file);
	newXS(strcpy(buf, "getitemname"), XS__getitemname, file);
	newXS(strcpy(buf, "getItemName"), XS_qc_getItemName, file);
	newXS(strcpy(buf, "getnpcnamebyid"), XS__getnpcnamebyid, file);
	newXS(strcpy(buf, "get_spawn_condition"), XS__get_spawn_condition, file);
	newXS(strcpy(buf, "getcharnamebyid"), XS__getcharnamebyid, file);
	newXS(strcpy(buf, "getcurrencyitemid"), XS__getcurrencyitemid, file);
	newXS(strcpy(buf, "getgendername"), XS__getgendername, file);
	newXS(strcpy(buf, "getdeityname"), XS__getdeityname, file);
	newXS(strcpy(buf, "getguildnamebyid"), XS__getguildnamebyid, file);
	newXS(strcpy(buf, "getguildidbycharid"), XS__getguildidbycharid, file);
	newXS(strcpy(buf, "getgroupidbycharid"), XS__getgroupidbycharid, file);
	newXS(strcpy(buf, "getinventoryslotname"), XS__getinventoryslotname, file);
	newXS(strcpy(buf, "getraididbycharid"), XS__getraididbycharid, file);
	newXS(strcpy(buf, "getracename"), XS__getracename, file);
	newXS(strcpy(buf, "getspellname"), XS__getspellname, file);
	newXS(strcpy(buf, "get_spell_level"), XS__get_spell_level, file);
	newXS(strcpy(buf, "getskillname"), XS__getskillname, file);
	newXS(strcpy(buf, "getlevel"), XS__getlevel, file);
	newXS(strcpy(buf, "getplayerburiedcorpsecount"), XS__getplayerburiedcorpsecount, file);
	newXS(strcpy(buf, "getplayercorpsecount"), XS__getplayercorpsecount, file);
	newXS(strcpy(buf, "getplayercorpsecountbyzoneid"), XS__getplayercorpsecountbyzoneid, file);
	newXS(strcpy(buf, "gettaskactivitydonecount"), XS__gettaskactivitydonecount, file);
	newXS(strcpy(buf, "gettaskname"), XS__gettaskname, file);
	newXS(strcpy(buf, "givecash"), XS__givecash, file);
	newXS(strcpy(buf, "gmmove"), XS__gmmove, file);
	newXS(strcpy(buf, "gmsay"), XS__gmsay, file);
	newXS(strcpy(buf, "has_zone_flag"), XS__has_zone_flag, file);
	newXS(strcpy(buf, "incstat"), XS__incstat, file);
	newXS(strcpy(buf, "isdisctome"), XS__isdisctome, file);
	newXS(strcpy(buf, "isdooropen"), XS__isdooropen, file);
	newXS(strcpy(buf, "istaskactive"), XS__istaskactive, file);
	newXS(strcpy(buf, "istaskactivityactive"), XS__istaskactivityactive, file);
	newXS(strcpy(buf, "istaskappropriate"), XS__istaskappropriate, file);
	newXS(strcpy(buf, "istaskcompleted"), XS__istaskcompleted, file);
	newXS(strcpy(buf, "istaskenabled"), XS__istaskenabled, file);
	newXS(strcpy(buf, "itemlink"), XS__itemlink, file);
	newXS(strcpy(buf, "lasttaskinset"), XS__lasttaskinset, file);
	newXS(strcpy(buf, "level"), XS__level, file);
	newXS(strcpy(buf, "log"), XS__log, file);
	newXS(strcpy(buf, "log_combat"), XS__log_combat, file);
	newXS(strcpy(buf, "me"), XS__me, file);
	newXS(strcpy(buf, "message"), XS__message, file);
	newXS(strcpy(buf, "modifynpcstat"), XS__ModifyNPCStat, file);
	newXS(strcpy(buf, "movegrp"), XS__movegrp, file);
	newXS(strcpy(buf, "movepc"), XS__movepc, file);
	newXS(strcpy(buf, "moveto"), XS__moveto, file);
	newXS(strcpy(buf, "nexttaskinset"), XS__nexttaskinset, file);
	newXS(strcpy(buf, "npcfeature"), XS__npcfeature, file);
	newXS(strcpy(buf, "npcgender"), XS__npcgender, file);
	newXS(strcpy(buf, "npcrace"), XS__npcrace, file);
	newXS(strcpy(buf, "npcsize"), XS__npcsize, file);
	newXS(strcpy(buf, "npctexture"), XS__npctexture, file);
	newXS(strcpy(buf, "pause"), XS__pause, file);
	newXS(strcpy(buf, "permaclass"), XS__permaclass, file);
	newXS(strcpy(buf, "permagender"), XS__permagender, file);
	newXS(strcpy(buf, "permarace"), XS__permarace, file);
	newXS(strcpy(buf, "playerfeature"), XS__playerfeature, file);
	newXS(strcpy(buf, "playergender"), XS__playergender, file);
	newXS(strcpy(buf, "playerrace"), XS__playerrace, file);
	newXS(strcpy(buf, "playersize"), XS__playersize, file);
	newXS(strcpy(buf, "playertexture"), XS__playertexture, file);
	newXS(strcpy(buf, "popup"), XS__popup, file);
	newXS(strcpy(buf, "processmobswhilezoneempty"), XS__processmobswhilezoneempty, file);
	newXS(strcpy(buf, "pvp"), XS__pvp, file);
	newXS(strcpy(buf, "qs_player_event"), XS__qs_player_event, file);
	newXS(strcpy(buf, "qs_send_query"), XS__qs_send_query, file);
	newXS(strcpy(buf, "rain"), XS__rain, file);
	newXS(strcpy(buf, "rebind"), XS__rebind, file);
	newXS(strcpy(buf, "reloadzonestaticdata"), XS__reloadzonestaticdata, file);
	newXS(strcpy(buf, "remove_all_expedition_lockouts_by_char_id"), XS__remove_all_expedition_lockouts_by_char_id, file);
	newXS(strcpy(buf, "remove_expedition_lockout_by_char_id"), XS__remove_expedition_lockout_by_char_id, file);
	newXS(strcpy(buf, "removeitem"), XS__removeitem, file);
	newXS(strcpy(buf, "removetitle"), XS__removetitle, file);
	newXS(strcpy(buf, "rename"), XS__rename, file);
	newXS(strcpy(buf, "repopzone"), XS__repopzone, file);
	newXS(strcpy(buf, "resettaskactivity"), XS__resettaskactivity, file);
	newXS(strcpy(buf, "respawn"), XS__respawn, file);
	newXS(strcpy(buf, "resume"), XS__resume, file);
	newXS(strcpy(buf, "safemove"), XS__safemove, file);
	newXS(strcpy(buf, "save"), XS__save, file);
	newXS(strcpy(buf, "say"), XS__say, file);
	newXS(strcpy(buf, "saylink"), XS__saylink, file);
	newXS(strcpy(buf, "scribespells"), XS__scribespells, file);
	newXS(strcpy(buf, "secondstotime"), XS__secondstotime, file);
	newXS(strcpy(buf, "selfcast"), XS__selfcast, file);
	newXS(strcpy(buf, "setaaexpmodifierbycharid"), XS__setaaexpmodifierbycharid, file);
	newXS(strcpy(buf, "set_proximity"), XS__set_proximity, file);
	newXS(strcpy(buf, "set_zone_flag"), XS__set_zone_flag, file);
	newXS(strcpy(buf, "setallskill"), XS__setallskill, file);
	newXS(strcpy(buf, "setanim"), XS__setanim, file);
	newXS(strcpy(buf, "setexpmodifierbycharid"), XS__setexpmodifierbycharid, file);
	newXS(strcpy(buf, "setglobal"), XS__setglobal, file);
	newXS(strcpy(buf, "setguild"), XS__setguild, file);
	newXS(strcpy(buf, "sethp"), XS__sethp, file);
	newXS(strcpy(buf, "setlanguage"), XS__setlanguage, file);
	newXS(strcpy(buf, "setnexthpevent"), XS__setnexthpevent, file);
	newXS(strcpy(buf, "setnextinchpevent"), XS__setnextinchpevent, file);
	newXS(strcpy(buf, "setskill"), XS__setskill, file);
	newXS(strcpy(buf, "setsky"), XS__setsky, file);
	newXS(strcpy(buf, "setstat"), XS__setstat, file);
	newXS(strcpy(buf, "settarget"), XS__settarget, file);
	newXS(strcpy(buf, "settime"), XS__settime, file);
	newXS(strcpy(buf, "settimer"), XS__settimer, file);
	newXS(strcpy(buf, "settimerMS"), XS__settimerMS, file);
	newXS(strcpy(buf, "sfollow"), XS__sfollow, file);
	newXS(strcpy(buf, "shout"), XS__shout, file);
	newXS(strcpy(buf, "shout2"), XS__shout2, file);
	newXS(strcpy(buf, "showgrid"), XS__showgrid, file);
	newXS(strcpy(buf, "signal"), XS__signal, file);
	newXS(strcpy(buf, "signalwith"), XS__signalwith, file);
	newXS(strcpy(buf, "snow"), XS__snow, file);
	newXS(strcpy(buf, "spawn"), XS__spawn, file);
	newXS(strcpy(buf, "spawn2"), XS__spawn2, file);
	newXS(strcpy(buf, "spawn_condition"), XS__spawn_condition, file);
	newXS(strcpy(buf, "spawn_from_spawn2"), XS__spawn_from_spawn2, file);
	newXS(strcpy(buf, "start"), XS__start, file);
	newXS(strcpy(buf, "stop"), XS__stop, file);
	newXS(strcpy(buf, "stopalltimers"), XS__stopalltimers, file);
	newXS(strcpy(buf, "stoptimer"), XS__stoptimer, file);
	newXS(strcpy(buf, "summonallplayercorpses"), XS__summonallplayercorpses, file);
	newXS(strcpy(buf, "summonburiedplayercorpse"), XS__summonburiedplayercorpse, file);
	newXS(strcpy(buf, "summonitem"), XS__summonitem, file);
	newXS(strcpy(buf, "surname"), XS__surname, file);
	newXS(strcpy(buf, "targlobal"), XS__targlobal, file);
	newXS(strcpy(buf, "taskexploredarea"), XS__taskexploredarea, file);
	newXS(strcpy(buf, "taskselector"), XS__taskselector, file);
	newXS(strcpy(buf, "task_setselector"), XS__task_setselector, file);
	newXS(strcpy(buf, "tasktimeleft"), XS__tasktimeleft, file);
	newXS(strcpy(buf, "toggle_spawn_event"), XS__toggle_spawn_event, file);
	newXS(strcpy(buf, "toggledoorstate"), XS__toggledoorstate, file);
	newXS(strcpy(buf, "traindisc"), XS__traindisc, file);
	newXS(strcpy(buf, "traindiscs"), XS__traindiscs, file);
	newXS(strcpy(buf, "unique_spawn"), XS__unique_spawn, file);
	newXS(strcpy(buf, "unscribespells"), XS__unscribespells, file);
	newXS(strcpy(buf, "untraindiscs"), XS__untraindiscs, file);
	newXS(strcpy(buf, "updatespawntimer"), XS__UpdateSpawnTimer, file);
	newXS(strcpy(buf, "updatetaskactivity"), XS__updatetaskactivity, file);
	newXS(strcpy(buf, "UpdateZoneHeader"), XS__UpdateZoneHeader, file);
	newXS(strcpy(buf, "varlink"), XS__varlink, file);
	newXS(strcpy(buf, "voicetell"), XS__voicetell, file);
	newXS(strcpy(buf, "we"), XS__we, file);
	newXS(strcpy(buf, "wearchange"), XS__wearchange, file);
	newXS(strcpy(buf, "whisper"), XS__whisper, file);
	newXS(strcpy(buf, "write"), XS__write, file);
	newXS(strcpy(buf, "ze"), XS__ze, file);
	newXS(strcpy(buf, "zone"), XS__zone, file);
	newXS(strcpy(buf, "zonegroup"), XS__zonegroup, file);
	newXS(strcpy(buf, "zoneraid"), XS__zoneraid, file);

	/**
	 * Expansions
	 */
	newXS(strcpy(buf, "is_classic_enabled"), XS__IsClassicEnabled, file);
	newXS(strcpy(buf, "is_the_ruins_of_kunark_enabled"), XS__IsTheRuinsOfKunarkEnabled, file);
	newXS(strcpy(buf, "is_the_scars_of_velious_enabled"), XS__IsTheScarsOfVeliousEnabled, file);
	newXS(strcpy(buf, "is_the_shadows_of_luclin_enabled"), XS__IsTheShadowsOfLuclinEnabled, file);
	newXS(strcpy(buf, "is_the_planes_of_power_enabled"), XS__IsThePlanesOfPowerEnabled, file);
	newXS(strcpy(buf, "is_the_legacy_of_ykesha_enabled"), XS__IsTheLegacyOfYkeshaEnabled, file);
	newXS(strcpy(buf, "is_lost_dungeons_of_norrath_enabled"), XS__IsLostDungeonsOfNorrathEnabled, file);
	newXS(strcpy(buf, "is_gates_of_discord_enabled"), XS__IsGatesOfDiscordEnabled, file);
	newXS(strcpy(buf, "is_omens_of_war_enabled"), XS__IsOmensOfWarEnabled, file);
	newXS(strcpy(buf, "is_dragons_of_norrath_enabled"), XS__IsDragonsOfNorrathEnabled, file);
	newXS(strcpy(buf, "is_depths_of_darkhollow_enabled"), XS__IsDepthsOfDarkhollowEnabled, file);
	newXS(strcpy(buf, "is_prophecy_of_ro_enabled"), XS__IsProphecyOfRoEnabled, file);
	newXS(strcpy(buf, "is_the_serpents_spine_enabled"), XS__IsTheSerpentsSpineEnabled, file);
	newXS(strcpy(buf, "is_the_buried_sea_enabled"), XS__IsTheBuriedSeaEnabled, file);
	newXS(strcpy(buf, "is_secrets_of_faydwer_enabled"), XS__IsSecretsOfFaydwerEnabled, file);
	newXS(strcpy(buf, "is_seeds_of_destruction_enabled"), XS__IsSeedsOfDestructionEnabled, file);
	newXS(strcpy(buf, "is_underfoot_enabled"), XS__IsUnderfootEnabled, file);
	newXS(strcpy(buf, "is_house_of_thule_enabled"), XS__IsHouseOfThuleEnabled, file);
	newXS(strcpy(buf, "is_veil_of_alaris_enabled"), XS__IsVeilOfAlarisEnabled, file);
	newXS(strcpy(buf, "is_rain_of_fear_enabled"), XS__IsRainOfFearEnabled, file);
	newXS(strcpy(buf, "is_call_of_the_forsaken_enabled"), XS__IsCallOfTheForsakenEnabled, file);
	newXS(strcpy(buf, "is_the_darkend_sea_enabled"), XS__IsTheDarkendSeaEnabled, file);
	newXS(strcpy(buf, "is_the_broken_mirror_enabled"), XS__IsTheBrokenMirrorEnabled, file);
	newXS(strcpy(buf, "is_empires_of_kunark_enabled"), XS__IsEmpiresOfKunarkEnabled, file);
	newXS(strcpy(buf, "is_ring_of_scale_enabled"), XS__IsRingOfScaleEnabled, file);
	newXS(strcpy(buf, "is_the_burning_lands_enabled"), XS__IsTheBurningLandsEnabled, file);
	newXS(strcpy(buf, "is_torment_of_velious_enabled"), XS__IsTormentOfVeliousEnabled, file);
	newXS(strcpy(buf, "is_current_expansion_classic"), XS__IsCurrentExpansionClassic, file);
	newXS(strcpy(buf, "is_current_expansion_the_ruins_of_kunark"), XS__IsCurrentExpansionTheRuinsOfKunark, file);
	newXS(strcpy(buf, "is_current_expansion_the_scars_of_velious"), XS__IsCurrentExpansionTheScarsOfVelious, file);
	newXS(strcpy(buf, "is_current_expansion_the_shadows_of_luclin"), XS__IsCurrentExpansionTheShadowsOfLuclin, file);
	newXS(strcpy(buf, "is_current_expansion_the_planes_of_power"), XS__IsCurrentExpansionThePlanesOfPower, file);
	newXS(strcpy(buf, "is_current_expansion_the_legacy_of_ykesha"), XS__IsCurrentExpansionTheLegacyOfYkesha, file);
	newXS(strcpy(buf, "is_current_expansion_lost_dungeons_of_norrath"), XS__IsCurrentExpansionLostDungeonsOfNorrath, file);
	newXS(strcpy(buf, "is_current_expansion_gates_of_discord"), XS__IsCurrentExpansionGatesOfDiscord, file);
	newXS(strcpy(buf, "is_current_expansion_omens_of_war"), XS__IsCurrentExpansionOmensOfWar, file);
	newXS(strcpy(buf, "is_current_expansion_dragons_of_norrath"), XS__IsCurrentExpansionDragonsOfNorrath, file);
	newXS(strcpy(buf, "is_current_expansion_depths_of_darkhollow"), XS__IsCurrentExpansionDepthsOfDarkhollow, file);
	newXS(strcpy(buf, "is_current_expansion_prophecy_of_ro"), XS__IsCurrentExpansionProphecyOfRo, file);
	newXS(strcpy(buf, "is_current_expansion_the_serpents_spine"), XS__IsCurrentExpansionTheSerpentsSpine, file);
	newXS(strcpy(buf, "is_current_expansion_the_buried_sea"), XS__IsCurrentExpansionTheBuriedSea, file);
	newXS(strcpy(buf, "is_current_expansion_secrets_of_faydwer"), XS__IsCurrentExpansionSecretsOfFaydwer, file);
	newXS(strcpy(buf, "is_current_expansion_seeds_of_destruction"), XS__IsCurrentExpansionSeedsOfDestruction, file);
	newXS(strcpy(buf, "is_current_expansion_underfoot"), XS__IsCurrentExpansionUnderfoot, file);
	newXS(strcpy(buf, "is_current_expansion_house_of_thule"), XS__IsCurrentExpansionHouseOfThule, file);
	newXS(strcpy(buf, "is_current_expansion_veil_of_alaris"), XS__IsCurrentExpansionVeilOfAlaris, file);
	newXS(strcpy(buf, "is_current_expansion_rain_of_fear"), XS__IsCurrentExpansionRainOfFear, file);
	newXS(strcpy(buf, "is_current_expansion_call_of_the_forsaken"), XS__IsCurrentExpansionCallOfTheForsaken, file);
	newXS(strcpy(buf, "is_current_expansion_the_darkend_sea"), XS__IsCurrentExpansionTheDarkendSea, file);
	newXS(strcpy(buf, "is_current_expansion_the_broken_mirror"), XS__IsCurrentExpansionTheBrokenMirror, file);
	newXS(strcpy(buf, "is_current_expansion_empires_of_kunark"), XS__IsCurrentExpansionEmpiresOfKunark, file);
	newXS(strcpy(buf, "is_current_expansion_ring_of_scale"), XS__IsCurrentExpansionRingOfScale, file);
	newXS(strcpy(buf, "is_current_expansion_the_burning_lands"), XS__IsCurrentExpansionTheBurningLands, file);
	newXS(strcpy(buf, "is_current_expansion_torment_of_velious"), XS__IsCurrentExpansionTormentOfVelious, file);

	/**
	 * Content flags
	 */
	newXS(strcpy(buf, "is_content_flag_enabled"), XS__IsContentFlagEnabled, file);
	newXS(strcpy(buf, "set_content_flag"), XS__SetContentFlag, file);

	XSRETURN_YES;
}

#endif
#endif
