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

#ifdef EMBPERL
#ifdef EMBPERL_XS

#include "../common/global_define.h"
#include "../common/misc_functions.h"
#include "../common/eqemu_logsys.h"

#include "embparser.h"
#include "embxs.h"
#include "entity.h"
#include "queryserv.h"
#include "questmgr.h"
#include "zone.h"

extern Zone* zone;
extern QueryServ* QServ;

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
XS(XS_Client_new)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Client::new()");
	{
		Client *		RETVAL;

		RETVAL = quest_manager.GetInitiator();
		ST(0) = sv_newmortal();
		if(RETVAL)
			sv_setref_pv(ST(0), "Client", (void*)RETVAL);
	}
	XSRETURN(1);
}

//Any creation of new NPC objects gets the current quest NPC
XS(XS_NPC_new);
XS(XS_NPC_new)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: NPC::new()");
	{
		NPC *		RETVAL;

		RETVAL = quest_manager.GetNPC();
		ST(0) = sv_newmortal();
		if(RETVAL)
			sv_setref_pv(ST(0), "NPC", (void*)RETVAL);
	}
	XSRETURN(1);
}

//Any creation of new NPC objects gets the current quest NPC
XS(XS_EntityList_new);
XS(XS_EntityList_new)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: EntityList::new()");
	{
		EntityList *		RETVAL;

		RETVAL = &entity_list;
		ST(0) = sv_newmortal();
		if(RETVAL)
			sv_setref_pv(ST(0), "EntityList", (void*)RETVAL);
	}
	XSRETURN(1);
}

//Any creation of new quest items gets the current quest item
XS(XS_QuestItem_new);
XS(XS_QuestItem_new)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: QuestItem::new()");

	ItemInst* RETVAL;

	RETVAL = quest_manager.GetQuestItem();
	ST(0) = sv_newmortal();
	if(RETVAL)
		sv_setref_pv(ST(0), "QuestItem", (void*)RETVAL);

	XSRETURN(1);
}

//Any creation of new quest items gets the current quest item
XS(XS_MobList_new);
XS(XS_MobList_new)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: MobList::new()");

	ListElement<Mob*>* RETVAL;

	RETVAL = nullptr;
	ST(0) = sv_newmortal();
	if(RETVAL)
		sv_setref_pv(ST(0), "MobList", (void*)RETVAL);

	XSRETURN(1);
}

#endif //EMBPERL_XS_CLASSES


XS(XS__echo); // prototype to pass -Wmissing-prototypes
XS(XS__echo) {
	dXSARGS;

   if (items != 2)
	  Perl_croak(aTHX_ "Usage: echo(id#, str)");

	quest_manager.echo(SvUV(ST(0)), SvPV_nolen(ST(1)));

	XSRETURN_EMPTY;
}

XS(XS__say); // prototype to pass -Wmissing-prototypes
XS(XS__say) {
	dXSARGS;

	if (items == 1)
		quest_manager.say(SvPV_nolen(ST(0)));
	else if (items == 2)
		quest_manager.say(SvPV_nolen(ST(0)), (int)SvIV(ST(1)));
	else
		Perl_croak(aTHX_ "Usage: say(str [, language])");

	XSRETURN_EMPTY;
}

XS(XS__me); // prototype to pass -Wmissing-prototypes
XS(XS__me) {
	dXSARGS;

	if (items != 1)
		Perl_croak(aTHX_ "Usage: %s(str)", "me");

	quest_manager.me(SvPV_nolen(ST(0)));

	XSRETURN_EMPTY;
}

XS(XS__summonitem); // prototype to pass -Wmissing-prototypes
XS(XS__summonitem)
{
	dXSARGS;
	if (items == 1)
		quest_manager.summonitem(SvUV(ST(0)));
	else if(items == 2)
		quest_manager.summonitem(SvUV(ST(0)), SvUV(ST(1)));
	else
		Perl_croak(aTHX_ "Usage: summonitem(itemid, [charges])");
	XSRETURN_EMPTY;
}

XS(XS__write);
XS(XS__write)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: write(file, str)");

	char *		file = (char *)SvPV_nolen(ST(0));
	char *		str = (char *)SvPV_nolen(ST(1));

	quest_manager.write(file, str);

	XSRETURN_EMPTY;
}

XS(XS__spawn);
XS(XS__spawn)
{
	dXSARGS;
	if (items != 6)
		Perl_croak(aTHX_ "Usage: spawn(npc_type, grid, unused, x, y, z)");

	uint16		RETVAL;
	dXSTARG;

	int	npc_type = (int)SvIV(ST(0));
	int	grid = (int)SvIV(ST(1));
	int	unused = (int)SvIV(ST(2));
	auto position = glm::vec4((float)SvNV(ST(3)), (float)SvNV(ST(4)), (float)SvNV(ST(5)), 0.0f);

	Mob *r = quest_manager.spawn2(npc_type, grid, unused, position);
	RETVAL = (r != nullptr) ? r->GetID() : 0;
	XSprePUSH; PUSHu((UV)RETVAL);

	XSRETURN(1);
}

XS(XS__spawn2);
XS(XS__spawn2)
{
	dXSARGS;
	if (items != 7)
		Perl_croak(aTHX_ "Usage: spawn2(npc_type, grid, unused, x, y, z, heading)");

	uint16		RETVAL;
	dXSTARG;

	int	npc_type = (int)SvIV(ST(0));
	int	grid = (int)SvIV(ST(1));
	int	unused = (int)SvIV(ST(2));
	auto position = glm::vec4((float)SvNV(ST(3)), (float)SvNV(ST(4)), (float)SvNV(ST(5)), (float)SvNV(ST(6)));

	Mob *r = quest_manager.spawn2(npc_type, grid, unused, position);
	RETVAL = (r != nullptr) ? r->GetID() : 0;
	XSprePUSH; PUSHu((UV)RETVAL);

	XSRETURN(1);
}

XS(XS__unique_spawn);
XS(XS__unique_spawn)
{
	dXSARGS;
	if (items != 6 && items != 7)
		Perl_croak(aTHX_ "Usage: unique_spawn(npc_type, grid, unused, x, y, z, [heading])");

	uint16		RETVAL;
	dXSTARG;

	int	npc_type = (int)SvIV(ST(0));
	int	grid = (int)SvIV(ST(1));
	int	unused = (int)SvIV(ST(2));
	float	x = (float)SvNV(ST(3));
	float	y = (float)SvNV(ST(4));
	float	z = (float)SvNV(ST(5));
	float	heading = 0;
	if(items == 7)
		heading = (float)SvNV(ST(6));

	Mob *r =  quest_manager.unique_spawn(npc_type, grid, unused, glm::vec4(x, y, z, heading));
	RETVAL = (r != nullptr) ? r->GetID() : 0;

	XSprePUSH; PUSHu((UV)RETVAL);

	XSRETURN(1);
}

XS(XS__spawn_from_spawn2);
XS(XS__spawn_from_spawn2)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: spawn_from_spawn2(spawn2_id)");

	uint16		RETVAL;
	dXSTARG;

	int	spawn2_id = (int)SvIV(ST(0));

	Mob *r =  quest_manager.spawn_from_spawn2(spawn2_id);
	RETVAL = (r != nullptr) ? r->GetID() : 0;

	XSprePUSH; PUSHu((UV)RETVAL);

	XSRETURN(1);
}

XS(XS__enable_spawn2);
XS(XS__enable_spawn2)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: enable_spawn2(spawn2_id)");

	int	spawn2_id = (int)SvIV(ST(0));

	quest_manager.enable_spawn2(spawn2_id);
	XSRETURN_EMPTY;
}

XS(XS__disable_spawn2);
XS(XS__disable_spawn2)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: disable_spawn2(spawn2_id)");

	int	spawn2_id = (int)SvIV(ST(0));

	quest_manager.disable_spawn2(spawn2_id);
	XSRETURN_EMPTY;
}

XS(XS__setstat);
XS(XS__setstat)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: setstat(stat, value)");

	int	stat = (int)SvIV(ST(0));
	int	value = (int)SvIV(ST(1));

	quest_manager.setstat(stat, value);

	XSRETURN_EMPTY;
}

XS(XS__incstat);  //old setstat command aza
XS(XS__incstat)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: incstat(stat, value)");

	int	stat = (int)SvIV(ST(0));
	int	value = (int)SvIV(ST(1));

	quest_manager.incstat(stat, value);

	XSRETURN_EMPTY;
}

XS(XS__castspell);
XS(XS__castspell)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: castspell(spell_id, target_id)");

	int	spell_id = (int)SvIV(ST(0));
	int	target_id = (int)SvIV(ST(1));

	quest_manager.castspell(spell_id, target_id);

	XSRETURN_EMPTY;
}

XS(XS__selfcast);
XS(XS__selfcast)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: selfcast(spell_id)");

	int	spell_id = (int)SvIV(ST(0));

	quest_manager.selfcast(spell_id);

	XSRETURN_EMPTY;
}

XS(XS__addloot);
XS(XS__addloot)
{
	dXSARGS;
	if(items < 1 || items > 3)
		Perl_croak(aTHX_ "Usage: addloot(item_id, charges = 0, equipitem = true)");

	uint32	itemid = (uint32)SvUV(ST(0));
	uint16	charges = 0;
	bool	equipitem = true;

	if (items > 1)
		charges = (uint16)SvUV(ST(1));
	if (items > 2)
		equipitem = (bool)SvTRUE(ST(2));

	quest_manager.addloot(itemid, charges, equipitem);

	XSRETURN_EMPTY;
}

XS(XS__zone);
XS(XS__zone)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: zone(zone_name)");

	char *		zone_name = (char *)SvPV_nolen(ST(0));

	quest_manager.Zone(zone_name);

	XSRETURN_EMPTY;
}

XS(XS__settimer);
XS(XS__settimer)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: settimer(timer_name, seconds)");

	char *		timer_name = (char *)SvPV_nolen(ST(0));
	int	seconds = (int)SvIV(ST(1));

	quest_manager.settimer(timer_name, seconds);

	XSRETURN_EMPTY;
}

XS(XS__settimerMS);
XS(XS__settimerMS)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: settimerMS(timer_name, milliseconds)");

	char *		timer_name = (char *)SvPV_nolen(ST(0));
	int	milliseconds = (int)SvIV(ST(1));

	quest_manager.settimerMS(timer_name, milliseconds);

	XSRETURN_EMPTY;
}

XS(XS__stoptimer);
XS(XS__stoptimer)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: stoptimer(timer_name)");

	char *		timer_name = (char *)SvPV_nolen(ST(0));

	quest_manager.stoptimer(timer_name);

	XSRETURN_EMPTY;
}

XS(XS__stopalltimers);
XS(XS__stopalltimers)
{
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: stopalltimers()");

	quest_manager.stopalltimers();

	XSRETURN_EMPTY;
}

XS(XS__emote);
XS(XS__emote)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: emote(str)");

	char *		str = (char *)SvPV_nolen(ST(0));

	quest_manager.emote(str);

	XSRETURN_EMPTY;
}

XS(XS__shout);
XS(XS__shout)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: shout(str)");

	char *		str = (char *)SvPV_nolen(ST(0));

	quest_manager.shout(str);

	XSRETURN_EMPTY;
}

XS(XS__shout2);
XS(XS__shout2)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: shout2(str)");

	char *		str = (char *)SvPV_nolen(ST(0));

	quest_manager.shout2(str);

	XSRETURN_EMPTY;
}

XS(XS__gmsay);
XS(XS__gmsay)
{
	dXSARGS;
	if ((items < 1) || (items > 5))
		Perl_croak(aTHX_ "Usage: gmsay(str, color, send_to_world?)");

	char * str = (char *)SvPV_nolen(ST(0));
	int	color = 7;
	bool send_to_world = 0;
	uint32 to_guilddbid = 0;
	uint16 to_minstatus = 80;

	if (items > 1) {
		color = (int)SvIV(ST(1));
	}

	if (items > 2) {
		send_to_world = ((int)SvIV(ST(2))) == 0?false:true;
	}

	if (items > 3)
		to_guilddbid = (int)SvUV(ST(3));

	if (items > 4)
		to_minstatus = (int)SvUV(ST(4));

	quest_manager.gmsay(str, color, send_to_world, to_guilddbid, to_minstatus);

	XSRETURN_EMPTY;
}

XS(XS__depop);
XS(XS__depop)
{
	dXSARGS;
	if (items < 0 || items > 1)
		Perl_croak(aTHX_ "Usage: depop(npc_type= 0)");

	int	npc_type;

	if (items < 1)
		npc_type = 0;
	else
		npc_type = (int)SvIV(ST(0));


	quest_manager.depop(npc_type);

	XSRETURN_EMPTY;
}

XS(XS__depop_withtimer);
XS(XS__depop_withtimer)
{
	dXSARGS;
	if (items < 0 || items > 1)
		Perl_croak(aTHX_ "Usage: depop_withtimer(npc_type= 0)");

	int	npc_type;

	if (items < 1)
		npc_type = 0;
	else
		npc_type = (int)SvIV(ST(0));


	quest_manager.depop_withtimer(npc_type);

	XSRETURN_EMPTY;
}

XS(XS__depopall);
XS(XS__depopall)
{
	dXSARGS;
	if (items < 0 || items > 1)
		Perl_croak(aTHX_ "Usage: depopall(npc_type= 0)");

	int	npc_type;

	if (items < 1)
		npc_type = 0;
	else
		npc_type = (int)SvIV(ST(0));


	quest_manager.depopall(npc_type);

	XSRETURN_EMPTY;
}

XS(XS__settarget);
XS(XS__settarget)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: settarget(type, target_id)");

	char *		type = (char *)SvPV_nolen(ST(0));
	int	target_id = (int)SvIV(ST(1));

	quest_manager.settarget(type, target_id);

	XSRETURN_EMPTY;
}

XS(XS__follow);
XS(XS__follow)
{
	dXSARGS;
	if (items != 1 && items != 2)
		Perl_croak(aTHX_ "Usage: follow(entity_id, [distance])");

	int	entity_id = (int)SvIV(ST(0));
	int	distance;

	if (items == 2)
		distance = (int)SvIV(ST(1));
	else
		distance = 10;

	quest_manager.follow(entity_id, distance);

	XSRETURN_EMPTY;
}

XS(XS__sfollow);
XS(XS__sfollow)
{
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: sfollow()");


	quest_manager.sfollow();

	XSRETURN_EMPTY;
}

XS(XS__changedeity);
XS(XS__changedeity)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: changedeity(diety_id)");

	int	diety_id = (int)SvIV(ST(0));

	quest_manager.changedeity(diety_id);

	XSRETURN_EMPTY;
}

XS(XS__exp);
XS(XS__exp)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: exp(amt)");

	int	amt = (int)SvIV(ST(0));

	quest_manager.exp(amt);

	XSRETURN_EMPTY;
}

XS(XS__level);
XS(XS__level)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: level(newlevel)");

	int	newlevel = (int)SvIV(ST(0));

	quest_manager.level(newlevel);

	XSRETURN_EMPTY;
}

XS(XS__traindisc);
XS(XS__traindisc)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: traindisc(discipline_tome_item_id)");

	int	discipline_tome_item_id = (int)SvIV(ST(0));

	quest_manager.traindisc(discipline_tome_item_id);

	XSRETURN_EMPTY;
}

XS(XS__isdisctome);
XS(XS__isdisctome)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: isdisctome(item_id)");

	bool RETVAL;
	int	item_id = (int)SvIV(ST(0));

	RETVAL = quest_manager.isdisctome(item_id);

	ST(0) = boolSV(RETVAL);
	sv_2mortal(ST(0));
	XSRETURN(1);
}

XS(XS__safemove);
XS(XS__safemove)
{
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: safemove()");


	quest_manager.safemove();

	XSRETURN_EMPTY;
}

XS(XS__rain);
XS(XS__rain)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: rain(weather)");

	int	weather = (int)SvIV(ST(0));

	quest_manager.rain(weather);

	XSRETURN_EMPTY;
}

XS(XS__snow);
XS(XS__snow)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: snow(weather)");

	int	weather = (int)SvIV(ST(0));

	quest_manager.snow(weather);

	XSRETURN_EMPTY;
}

XS(XS__surname);
XS(XS__surname)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: surname(name)");

	char *		name = (char *)SvPV_nolen(ST(0));

	quest_manager.surname(name);

	XSRETURN_EMPTY;
}

XS(XS__permaclass);
XS(XS__permaclass)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: permaclass(class_id)");

	int	class_id = (int)SvIV(ST(0));

	quest_manager.permaclass(class_id);

	XSRETURN_EMPTY;
}

XS(XS__permarace);
XS(XS__permarace)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: permarace(race_id)");

	int	race_id = (int)SvIV(ST(0));

	quest_manager.permarace(race_id);

	XSRETURN_EMPTY;
}

XS(XS__permagender);
XS(XS__permagender)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: permagender(gender_id)");

	int	gender_id = (int)SvIV(ST(0));

	quest_manager.permagender(gender_id);

	XSRETURN_EMPTY;
}

XS(XS__scribespells);
XS(XS__scribespells)
{
	dXSARGS;
	if (items < 1)
		Perl_croak(aTHX_ "Usage: scribespells(max_level, min_level = 1)");

	uint16	RETVAL;
	dXSTARG;

	uint8 max_level = (uint8)SvIV(ST(0));
	uint8 min_level = (uint8)SvIV(ST(1));

	if (min_level)
		RETVAL = quest_manager.scribespells(max_level, min_level);
	else
		RETVAL = quest_manager.scribespells(max_level);

	XSprePUSH; PUSHu((IV)RETVAL);
	XSRETURN(1);
}

XS(XS__traindiscs);
XS(XS__traindiscs)
{
	dXSARGS;
	if (items < 1)
		Perl_croak(aTHX_ "Usage: traindiscs(max_level, min_level = 1)");

	uint16	RETVAL;
	dXSTARG;

	uint8 max_level = (uint8)SvIV(ST(0));
	uint8 min_level = (uint8)SvIV(ST(1));

	if (min_level)
		RETVAL = quest_manager.traindiscs(max_level, min_level);
	else
		RETVAL = quest_manager.traindiscs(max_level);

	XSprePUSH; PUSHu((IV)RETVAL);
	XSRETURN(1);
}

XS(XS__unscribespells);
XS(XS__unscribespells)
{
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: unscribespells()");


	quest_manager.unscribespells();

	XSRETURN_EMPTY;
}

XS(XS__untraindiscs);
XS(XS__untraindiscs)
{
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: untraindiscs()");


	quest_manager.untraindiscs();

	XSRETURN_EMPTY;
}

XS(XS__givecash);
XS(XS__givecash)
{
	dXSARGS;
	if (items != 4)
		Perl_croak(aTHX_ "Usage: givecash(copper, silver, gold, platinum)");

	int	copper = (int)SvIV(ST(0));
	int	silver = (int)SvIV(ST(1));
	int	gold = (int)SvIV(ST(2));
	int	platinum = (int)SvIV(ST(3));

	quest_manager.givecash(copper, silver, gold, platinum);

	XSRETURN_EMPTY;
}

XS(XS__pvp);
XS(XS__pvp)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: pvp(mode)");

	char *		mode = (char *)SvPV_nolen(ST(0));

	quest_manager.pvp(mode);

	XSRETURN_EMPTY;
}

XS(XS__movepc);
XS(XS__movepc)
{
	dXSARGS;
	if (items != 4 && items != 5)
		Perl_croak(aTHX_ "Usage: movepc(zone_id, x, y, z [,heading])");

	int	zoneid = (int)SvIV(ST(0));
	float	x = (float)SvNV(ST(1));
	float	y = (float)SvNV(ST(2));
	float	z = (float)SvNV(ST(3));

	if (items == 4)

	quest_manager.movepc(zoneid, x, y, z, 0.0f);

	else {
	float	heading = (float)SvNV(ST(4));
	quest_manager.movepc(zoneid, x, y, z, heading);
	}

	XSRETURN_EMPTY;
}

XS(XS__gmmove);
XS(XS__gmmove)
{
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: gmmove(x, y, z)");

	float	x = (float)SvNV(ST(0));
	float	y = (float)SvNV(ST(1));
	float	z = (float)SvNV(ST(2));

	quest_manager.gmmove(x, y, z);

	XSRETURN_EMPTY;
}

XS(XS__movegrp);
XS(XS__movegrp)
{
	dXSARGS;
	if (items != 4)
		Perl_croak(aTHX_ "Usage: movegrp(zoneid, x, y, z)");

	int	zoneid = (int)SvIV(ST(0));
	float	x = (float)SvNV(ST(1));
	float	y = (float)SvNV(ST(2));
	float	z = (float)SvNV(ST(3));

	quest_manager.movegrp(zoneid, x, y, z);

	XSRETURN_EMPTY;
}

XS(XS__doanim);
XS(XS__doanim)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: doanim(anim_id)");

	int	anim_id = (int)SvIV(ST(0));

	quest_manager.doanim(anim_id);

	XSRETURN_EMPTY;
}

XS(XS__addskill);
XS(XS__addskill)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: addskill(skill_id, value)");

	int	skill_id = (int)SvIV(ST(0));
	int	value = (int)SvIV(ST(1));

	quest_manager.addskill(skill_id, value);

	XSRETURN_EMPTY;
}

XS(XS__setlanguage);
XS(XS__setlanguage)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: setlanguage(skill_id, value)");

	int	skill_id = (int)SvIV(ST(0));
	int	value = (int)SvIV(ST(1));

	quest_manager.setlanguage(skill_id, value);

	XSRETURN_EMPTY;
}

XS(XS__setskill);
XS(XS__setskill)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: setskill(skill_id, value)");

	int	skill_id = (int)SvIV(ST(0));
	int	value = (int)SvIV(ST(1));

	quest_manager.setskill(skill_id, value);

	XSRETURN_EMPTY;
}

XS(XS__setallskill);
XS(XS__setallskill)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: setallskill(value)");

	int	value = (int)SvIV(ST(0));

	quest_manager.setallskill(value);

	XSRETURN_EMPTY;
}

XS(XS__attack);
XS(XS__attack)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: attack(client_name)");

	char *		client_name = (char *)SvPV_nolen(ST(0));

	quest_manager.attack(client_name);

	XSRETURN_EMPTY;
}

XS(XS__attacknpc);
XS(XS__attacknpc)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: attacknpc(npc_entity_id)");

	int	npc_entity_id = (int)SvIV(ST(0));

	quest_manager.attacknpc(npc_entity_id);

	XSRETURN_EMPTY;
}

XS(XS__attacknpctype);
XS(XS__attacknpctype)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: attacknpctype(npc_type_id)");

	int	npc_type_id = (int)SvIV(ST(0));

	quest_manager.attacknpctype(npc_type_id);

	XSRETURN_EMPTY;
}

XS(XS__save);
XS(XS__save)
{
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: save()");


	quest_manager.save();

	XSRETURN_EMPTY;
}

XS(XS__faction);
XS(XS__faction)
{
	dXSARGS;
	if (items < 2 || items > 3)
		Perl_croak(aTHX_ "Usage: faction(faction_id, faction_value, temp)");

	int	faction_id = (int)SvIV(ST(0));
	int	faction_value = (int)SvIV(ST(1));
	int temp;

	if(items == 2)
		temp = 0;
	else
		temp = (int)SvIV(ST(2));

	quest_manager.faction(faction_id, faction_value, temp);

	XSRETURN_EMPTY;
}

XS(XS__setsky);
XS(XS__setsky)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: setsky(new_sky)");

		unsigned char		new_sky = (unsigned char)SvUV(ST(0));

	quest_manager.setsky(new_sky);

	XSRETURN_EMPTY;
}

XS(XS__setguild);
XS(XS__setguild)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: setguild(new_guild_id, new_rank)");

		unsigned long		new_guild_id = (unsigned long)SvUV(ST(0));
	int	new_rank = (int)SvIV(ST(1));

	quest_manager.setguild(new_guild_id, new_rank);

	XSRETURN_EMPTY;
}

XS(XS__createguild);
XS(XS__createguild)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: createguild(guild_name, leader)");

		char *	guild_name = (char *)SvPV_nolen(ST(0));
		char *	leader = (char *)SvPV_nolen(ST(1));

	quest_manager.CreateGuild(guild_name, leader);

	XSRETURN_EMPTY;
}

XS(XS__settime);
XS(XS__settime)
{
	dXSARGS;
	if (items < 2)
		Perl_croak(aTHX_ "Usage: settime(new_hour, new_min, [update_world = true])");

	if (items == 2){
		int	new_hour = (int)SvIV(ST(0));
		int	new_min = (int)SvIV(ST(1));
		quest_manager.settime(new_hour, new_min, true);
	}
	else if (items == 3){
		int	new_hour = (int)SvIV(ST(0));
		int	new_min = (int)SvIV(ST(1));

		int	update_world = (int)SvIV(ST(2));
		if (update_world == 1){
			quest_manager.settime(new_hour, new_min, true);
		}
		else{
			quest_manager.settime(new_hour, new_min, false);
		}
	}

	XSRETURN_EMPTY;
}

XS(XS__itemlink);
XS(XS__itemlink)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: itemlink(item_id)");

	int	item_id = (int)SvIV(ST(0));

	quest_manager.itemlink(item_id);

	XSRETURN_EMPTY;
}

XS(XS__signalwith);
XS(XS__signalwith)
{
	dXSARGS;

	if (items == 2) {
		int	npc_id = (int)SvIV(ST(0));
		int	signal_id = (int)SvIV(ST(1));
		quest_manager.signalwith(npc_id, signal_id);
	} else if(items == 3) {
		int	npc_id = (int)SvIV(ST(0));
		int	signal_id = (int)SvIV(ST(1));
		int	wait = (int)SvIV(ST(2));
		quest_manager.signalwith(npc_id, signal_id, wait);
	} else {
		Perl_croak(aTHX_ "Usage: signalwith(npc_id,signal_id[,wait_ms])");
	}

	XSRETURN_EMPTY;
}

XS(XS__signal);
XS(XS__signal)
{
	dXSARGS;

	if (items == 1) {
		int	npc_id = (int)SvIV(ST(0));
		quest_manager.signal(npc_id);
	} else if(items == 2) {
		int	npc_id = (int)SvIV(ST(0));
		int	wait = (int)SvIV(ST(1));
		quest_manager.signal(npc_id, wait);
	} else {
		Perl_croak(aTHX_ "Usage: signal(npc_id[,wait_ms])");
	}

	XSRETURN_EMPTY;
}

XS(XS__setglobal);
XS(XS__setglobal)
{
	dXSARGS;
	if (items != 4)
		Perl_croak(aTHX_ "Usage: setglobal(varname, newvalue, options, duration)");

	char *		varname = (char *)SvPV_nolen(ST(0));
	char *		newvalue = (char *)SvPV_nolen(ST(1));
	int	options = (int)SvIV(ST(2));
	char *		duration = (char *)SvPV_nolen(ST(3));

	quest_manager.setglobal(varname, newvalue, options, duration);

	XSRETURN_EMPTY;
}

XS(XS__targlobal);
XS(XS__targlobal)
{
	dXSARGS;
	if (items != 6)
		Perl_croak(aTHX_ "Usage: targlobal(varname, value, duration, npcid, charid, zoneid)");

	char *		varname = (char *)SvPV_nolen(ST(0));
	char *		value = (char *)SvPV_nolen(ST(1));
	char *		duration = (char *)SvPV_nolen(ST(2));
	int	npcid = (int)SvIV(ST(3));
	int	charid = (int)SvIV(ST(4));
	int	zoneid = (int)SvIV(ST(5));

	quest_manager.targlobal(varname, value, duration, npcid, charid, zoneid);

	XSRETURN_EMPTY;
}

XS(XS__delglobal);
XS(XS__delglobal)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: delglobal(varname)");

	char *		varname = (char *)SvPV_nolen(ST(0));

	quest_manager.delglobal(varname);

	XSRETURN_EMPTY;
}

XS(XS__ding);
XS(XS__ding)
{
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: ding()");


	quest_manager.ding();

	XSRETURN_EMPTY;
}

XS(XS__rebind);
XS(XS__rebind)
{
	dXSARGS;
	if (items != 4)
		Perl_croak(aTHX_ "Usage: rebind(zoneid, x, y, z)");

	int	zoneid = (int)SvIV(ST(0));
	auto location = glm::vec3((float)SvNV(ST(1)),(float)SvNV(ST(2)),(float)SvNV(ST(3)));

	quest_manager.rebind(zoneid, location);

	XSRETURN_EMPTY;
}

XS(XS__start);
XS(XS__start)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: start(wp)");

	int	wp = (int)SvIV(ST(0));

	quest_manager.start(wp);

	XSRETURN_EMPTY;
}

XS(XS__stop);
XS(XS__stop)
{
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: stop()");


	quest_manager.stop();

	XSRETURN_EMPTY;
}

XS(XS__pause);
XS(XS__pause)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: pause(duration)");

	int	duration = (int)SvIV(ST(0));

	quest_manager.pause(duration);

	XSRETURN_EMPTY;
}

XS(XS__moveto);
XS(XS__moveto)
{
	dXSARGS;
	if (items != 3 && items != 4 && items != 5)
		Perl_croak(aTHX_ "Usage: moveto(x, y, z, [mth, saveguard?])");

	float	x = (float)SvNV(ST(0));
	float	y = (float)SvNV(ST(1));
	float	z = (float)SvNV(ST(2));
	float	h;
	bool	saveguard;

	if(items > 3)
		h = (float)SvNV(ST(3));
	else
		h = 0;

	if(items > 4)
		saveguard = (bool)SvTRUE(ST(4));
	else
		saveguard = false;

	quest_manager.moveto(glm::vec4(x, y, z, h), saveguard);

	XSRETURN_EMPTY;
}

XS(XS__resume);
XS(XS__resume)
{
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: resume()");


	quest_manager.resume();

	XSRETURN_EMPTY;
}

XS(XS__addldonpoints);
XS(XS__addldonpoints)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: addldonpoints(points, theme)");

	long	points = (long)SvIV(ST(0));
	unsigned long		theme = (unsigned long)SvUV(ST(1));

	quest_manager.addldonpoints(points, theme);

	XSRETURN_EMPTY;
}

XS(XS__addldonwin);
XS(XS__addldonwin)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: addldonwin(wins, theme)");

	long	wins = (long)SvIV(ST(0));
	unsigned long		theme = (unsigned long)SvUV(ST(1));

	quest_manager.addldonwin(wins, theme);

	XSRETURN_EMPTY;
}

XS(XS__addldonloss);
XS(XS__addldonloss)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: addldonloss(losses, theme)");

	long	losses = (long)SvIV(ST(0));
	unsigned long		theme = (unsigned long)SvUV(ST(1));

	quest_manager.addldonloss(losses, theme);

	XSRETURN_EMPTY;
}

XS(XS__setnexthpevent);
XS(XS__setnexthpevent)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: setnexthpevent(at)");

	int	at = (int)SvIV(ST(0));

	quest_manager.setnexthpevent(at);

	XSRETURN_EMPTY;
}

XS(XS__setnextinchpevent);
XS(XS__setnextinchpevent)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: setnextinchpevent(at)");

	int	at = (int)SvIV(ST(0));

	quest_manager.setnextinchpevent(at);

	XSRETURN_EMPTY;
}

XS(XS__sethp);
XS(XS__sethp)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: sethp(percentage)");

	int	hpperc = (int)SvIV(ST(0));

	quest_manager.sethp(hpperc);

	XSRETURN_EMPTY;
}

XS(XS__respawn);
XS(XS__respawn)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: respawn(npc_type, grid)");

	int	npc_type = (int)SvIV(ST(0));
	int	grid = (int)SvIV(ST(1));

	quest_manager.respawn(npc_type, grid);

	XSRETURN_EMPTY;
}

XS(XS__ChooseRandom);
XS(XS__ChooseRandom)
{
	dXSARGS;
	if (items < 1)
		Perl_croak(aTHX_ "Usage: ChooseRandom(... list ...)");

	int index = zone->random.Int(0, items-1);

	SV *tmp = ST(0);
	ST(0) = ST(index);
	ST(index) = tmp;

	XSRETURN(1);	//return 1 element from the stack (ST(0))
}

XS(XS__set_proximity);
XS(XS__set_proximity)
{
	dXSARGS;
	if (items != 4 && items != 6)
		Perl_croak(aTHX_ "Usage: set_proximity(minx, maxx, miny, maxy [, minz, maxz])");

	float minx = (float)SvNV(ST(0));
	float maxx = (float)SvNV(ST(1));
	float miny = (float)SvNV(ST(2));
	float maxy = (float)SvNV(ST(3));

	if(items == 4)
		quest_manager.set_proximity(minx, maxx, miny, maxy);
	else {
		float minz = (float)SvNV(ST(4));
		float maxz = (float)SvNV(ST(5));
		quest_manager.set_proximity(minx, maxx, miny, maxy, minz, maxz);
	}

	XSRETURN_EMPTY;
}

XS(XS__clear_proximity);
XS(XS__clear_proximity)
{
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: clear_proximity()");

	quest_manager.clear_proximity();

	XSRETURN_EMPTY;
}

XS(XS__enable_proximity_say);
XS(XS__enable_proximity_say)
{
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: enable_proximity_say()");

	quest_manager.enable_proximity_say();

	XSRETURN_EMPTY;
}

XS(XS__disable_proximity_say);
XS(XS__disable_proximity_say)
{
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: disable_proximity_say()");

	quest_manager.disable_proximity_say();

	XSRETURN_EMPTY;
}

XS(XS__setanim);
XS(XS__setanim) //Cisyouc: mob->setappearance() addition
{
	dXSARGS;
	if(items != 2)
		Perl_croak(aTHX_ "Usage: quest::setanim(npc_type, animnum);");

	quest_manager.setanim(SvUV(ST(0)), SvUV(ST(1)));

	XSRETURN_EMPTY;
}

XS(XS__showgrid);
XS(XS__showgrid)
{
	dXSARGS;
	if(items != 1)
		Perl_croak(aTHX_ "Usage: quest::showgrid(grid_id);");

	quest_manager.showgrid(SvUV(ST(0)));

	XSRETURN_EMPTY;
}

XS(XS__spawn_condition);
XS(XS__spawn_condition)
{
	dXSARGS;
	if (items < 3 || items > 4)
		Perl_croak(aTHX_ "Usage: spawn_condition(zone_short, [instance_id], condition_id, value)");

	if(items == 3)
	{
		char *	zone_short = (char *)SvPV_nolen(ST(0));
		uint16	cond_id = (int)SvUV(ST(1));
		int16	value = (int)SvIV(ST(2));

		quest_manager.spawn_condition(zone_short, zone->GetInstanceID(), cond_id, value);
	}
	else
	{
		char *	zone_short = (char *)SvPV_nolen(ST(0));
		uint32	instance_id = (int)SvUV(ST(1));
		uint16	cond_id = (int)SvUV(ST(2));
		int16	value = (int)SvIV(ST(3));

		quest_manager.spawn_condition(zone_short, instance_id, cond_id, value);
	}
	XSRETURN_EMPTY;
}

XS(XS__get_spawn_condition);
XS(XS__get_spawn_condition)
{
	dXSARGS;
	if (items < 2 || items > 3)
		Perl_croak(aTHX_ "Usage: get_spawn_condition(zone_short, [instance_id], condition_id)");

	if(items == 2)
	{
		int16		RETVAL;
		dXSTARG;

		char *	zone_short = (char *)SvPV_nolen(ST(0));
		uint16	cond_id = (int)SvIV(ST(1));

		RETVAL = quest_manager.get_spawn_condition(zone_short, zone->GetInstanceID(), cond_id);
		XSprePUSH; PUSHu((IV)RETVAL);

		XSRETURN(1);
	}
	else
	{
		int16		RETVAL;
		dXSTARG;

		char *	zone_short = (char *)SvPV_nolen(ST(0));
		uint16	instance_id = (int)SvIV(ST(1));
		uint16	cond_id = (int)SvIV(ST(2));

		RETVAL = quest_manager.get_spawn_condition(zone_short, instance_id, cond_id);
		XSprePUSH; PUSHu((IV)RETVAL);

		XSRETURN(1);
	}
}

XS(XS__toggle_spawn_event);
XS(XS__toggle_spawn_event)
{
	dXSARGS;
	if (items != 4)
		Perl_croak(aTHX_ "Usage: toggle_spawn_event(event_id, enabled?, strict, reset_base)");

	uint32	event_id = (int)SvIV(ST(0));
	bool	enabled = ((int)SvIV(ST(1))) == 0?false:true;
	bool	strict = ((int)SvIV(ST(2))) == 0?false:true;
	bool	reset_base = ((int)SvIV(ST(3))) == 0?false:true;

	quest_manager.toggle_spawn_event(event_id, enabled, strict, reset_base);

	XSRETURN_EMPTY;
}

XS(XS__has_zone_flag);
XS(XS__has_zone_flag)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: has_zone_flag(zone_id)");

	int16		RETVAL;
	dXSTARG;

	uint32	zone_id = (int)SvIV(ST(0));

	RETVAL = quest_manager.has_zone_flag(zone_id);
	XSprePUSH; PUSHu((IV)RETVAL);

	XSRETURN(1);

}

XS(XS__set_zone_flag);
XS(XS__set_zone_flag)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: set_zone_flag(zone_id)");

	uint32	zone_id = (int)SvIV(ST(0));

	quest_manager.set_zone_flag(zone_id);

	XSRETURN_EMPTY;
}

XS(XS__clear_zone_flag);
XS(XS__clear_zone_flag)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: clear_zone_flag(zone_id)");

	uint32	zone_id = (int)SvIV(ST(0));

	quest_manager.clear_zone_flag(zone_id);

	XSRETURN_EMPTY;
}

XS(XS__summonburiedplayercorpse);
XS(XS__summonburiedplayercorpse)
{
	dXSARGS;
	if (items != 5)
		Perl_croak(aTHX_ "Usage: summonburiedplayercorpse(char_id,dest_x,dest_y,dest_z,dest_heading)");

	bool RETVAL;
	uint32	char_id = (int)SvIV(ST(0));
	auto position = glm::vec4((float)SvIV(ST(1)), (float)SvIV(ST(2)), (float)SvIV(ST(3)),(float)SvIV(ST(4)));

	RETVAL = quest_manager.summonburiedplayercorpse(char_id, position);

	ST(0) = boolSV(RETVAL);
	sv_2mortal(ST(0));
	XSRETURN(1);
}

XS(XS__summonallplayercorpses);
XS(XS__summonallplayercorpses)
{
	dXSARGS;
	if (items != 5)
		Perl_croak(aTHX_ "Usage: summonallplayercorpses(char_id,dest_x,dest_y,dest_z,dest_heading)");

	bool RETVAL;
	uint32	char_id = (int)SvIV(ST(0));
	auto position = glm::vec4((float)SvIV(ST(1)),(float)SvIV(ST(2)),(float)SvIV(ST(3)),(float)SvIV(ST(4)));

	RETVAL = quest_manager.summonallplayercorpses(char_id, position);

	ST(0) = boolSV(RETVAL);
	sv_2mortal(ST(0));
	XSRETURN(1);
}

XS(XS__getplayerburiedcorpsecount);
XS(XS__getplayerburiedcorpsecount)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: getplayerburiedcorpsecount(char_id)");

	uint32		RETVAL;
	dXSTARG;

	uint32	char_id = (int)SvIV(ST(0));

	RETVAL = quest_manager.getplayerburiedcorpsecount(char_id);
	XSprePUSH; PUSHu((IV)RETVAL);

	XSRETURN(1);
}

XS(XS__buryplayercorpse);
XS(XS__buryplayercorpse)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: buryplayercorpse(char_id)");

	uint32		RETVAL;
	dXSTARG;

	uint32	char_id = (int)SvIV(ST(0));

	RETVAL = quest_manager.buryplayercorpse(char_id);
	XSprePUSH; PUSHu((IV)RETVAL);

	XSRETURN(1);
}

XS(XS__forcedooropen);
XS(XS__forcedooropen)
{
	dXSARGS;
	if (items < 1 || items > 2)
		Perl_croak(aTHX_ "Usage: forcedooropen(doorid [, altmode=0])");

	if (items == 1)
	{
	uint32	did = (int)SvIV(ST(0));

	quest_manager.forcedooropen(did, false);

	XSRETURN_EMPTY;
	}
	else
	{
	uint32	did = (int)SvIV(ST(0));
	bool	am = (int)SvIV(ST(1)) == 0?false:true;

	quest_manager.forcedooropen(did, am);

	XSRETURN_EMPTY;
	}
}

XS(XS__forcedoorclose);
XS(XS__forcedoorclose)
{
	dXSARGS;
	if (items < 1 || items > 2)
		Perl_croak(aTHX_ "Usage: forcedoorclose(doorid [, altmode=0])");

	if (items == 1)
	{
	uint32	did = (int)SvIV(ST(0));

	quest_manager.forcedoorclose(did, false);

	XSRETURN_EMPTY;
	}
	else
	{
	uint32	did = (int)SvIV(ST(0));
	bool	am = (int)SvIV(ST(1)) == 0?false:true;

	quest_manager.forcedoorclose(did, am);

	XSRETURN_EMPTY;
	}
}

XS(XS__toggledoorstate);
XS(XS__toggledoorstate)
{
	dXSARGS;
	if (items !=1)
			Perl_croak(aTHX_ "Usage: toggledoorstate(doorid)");

	uint32	did = (int)SvIV(ST(0));

	quest_manager.toggledoorstate(did);

	XSRETURN_EMPTY;
}

XS(XS__isdooropen);
XS(XS__isdooropen)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: isdooropen(doorid)");

	bool		RETVAL;
	dXSTARG;

	uint32	doorid = (int)SvIV(ST(0));

	RETVAL = quest_manager.isdooropen(doorid);
	XSprePUSH; PUSHu((IV)RETVAL);

	XSRETURN(1);
}

XS(XS__depopzone);
XS(XS__depopzone)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: depopzone(StartSpawnStatus)");

	bool StartSpawnStatus = ((int)SvIV(ST(0))) == 0?false:true;

	quest_manager.depopzone(StartSpawnStatus);

	XSRETURN_EMPTY;
}

XS(XS__repopzone);
XS(XS__repopzone)
{
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: repopzone()");

	quest_manager.repopzone();

	XSRETURN_EMPTY;
}

XS(XS__ConnectNodeToNode);
XS(XS__ConnectNodeToNode)
{
	dXSARGS;
	if (items != 4)
		Perl_croak(aTHX_ "Usage: ConnectNodeToNode(node1, node2, teleport, doorid)");

	int	node1 = (int)SvIV(ST(0));
	int	node2 = (int)SvIV(ST(1));
	int	teleport = (int)SvIV(ST(2));
	int	doorid = (int)SvIV(ST(3));

	quest_manager.ConnectNodeToNode(node1, node2, teleport, doorid);

	XSRETURN_EMPTY;
}

XS(XS__AddNode);
XS(XS__AddNode)
{
	dXSARGS;
	//void QuestManager::AddNode(float x, float y, float z, float best_z, int32 requested_id);
	if (items < 3 || items > 5)
		Perl_croak(aTHX_ "Usage: AddNode(x, y, z, [best_z], [requested_id])");

	int	x = (int)SvIV(ST(0));
	int	y = (int)SvIV(ST(1));
	int	z = (int)SvIV(ST(2));
	int	best_z = 0;
	int requested_id = 0;

	if (items == 4)
	{
		best_z = (int)SvIV(ST(3));
	}
	else if (items == 5)
	{
		best_z = (int)SvIV(ST(3));
		requested_id = (int)SvIV(ST(4));
	}

	quest_manager.AddNode(x, y, z, best_z, requested_id);

	XSRETURN_EMPTY;
}

XS(XS__npcrace);
XS(XS__npcrace)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: npcrace(race_id)");

	int	race_id = (int)SvIV(ST(0));

	quest_manager.npcrace(race_id);

	XSRETURN_EMPTY;
}

XS(XS__npcgender);
XS(XS__npcgender)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: npcgender(gender_id)");

	int	gender_id= (int)SvIV(ST(0));

	quest_manager.npcgender(gender_id);

	XSRETURN_EMPTY;
}

XS(XS__npcsize);
XS(XS__npcsize)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: npcsize(newsize)");

	int	newsize = (int)SvIV(ST(0));

	quest_manager.npcsize(newsize);

	XSRETURN_EMPTY;
}

XS(XS__npctexture);
XS(XS__npctexture)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: npctexture(newtexture)");

	int	newtexture = (int)SvIV(ST(0));

	quest_manager.npctexture(newtexture);

	XSRETURN_EMPTY;
}

XS(XS__playerrace);
XS(XS__playerrace)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: playerrace(race_id)");

	int	race_id = (int)SvIV(ST(0));

	quest_manager.playerrace(race_id);

	XSRETURN_EMPTY;
}

XS(XS__playergender);
XS(XS__playergender)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: playergender(gender_id)");

	int	gender_id= (int)SvIV(ST(0));

	quest_manager.playergender(gender_id);

	XSRETURN_EMPTY;
}

XS(XS__playersize);
XS(XS__playersize)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: playersize(newsize)");

	int	newsize = (int)SvIV(ST(0));

	quest_manager.playersize(newsize);

	XSRETURN_EMPTY;
}

XS(XS__playertexture);
XS(XS__playertexture)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: playertexture(newtexture)");

	int	newtexture = (int)SvIV(ST(0));

	quest_manager.playertexture(newtexture);

	XSRETURN_EMPTY;
}

XS(XS__playerfeature);
XS(XS__playerfeature)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: playerfeature(feature, setting)");

	char *	feature	= (char *)SvPV_nolen(ST(0));
	int		setting	= (int)SvIV(ST(1));

	quest_manager.playerfeature(feature, setting);

	XSRETURN_EMPTY;
}

XS(XS__npcfeature);
XS(XS__npcfeature)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: npcfeature(feature, setting)");

	char *	feature	= (char *)SvPV_nolen(ST(0));
	int		setting	= (int)SvIV(ST(1));

	quest_manager.npcfeature(feature, setting);

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
		Perl_croak(aTHX_ "Usage: createBot(firstname, lastname, level, race, class, gender)");
	}

	char *firstname = (char *)SvPV_nolen(ST(0));
	char *lastname = (char *)SvPV_nolen(ST(1));
	int level = (int) SvIV(ST(2));
	int race = (int) SvIV(ST(3));
	int botclass = (int) SvIV(ST(4));
	int gender = (int) SvIV(ST(5));

	RETVAL = quest_manager.createBot(firstname, lastname, level, race, botclass, gender);
	XSprePUSH; PUSHu((IV)RETVAL);
	XSRETURN(1);
}

#endif //BOTS

XS(XS__taskselector);
XS(XS__taskselector)
{
	dXSARGS;
	if((items >= 1) && (items <=MAXCHOOSERENTRIES)) {
		int tasks[MAXCHOOSERENTRIES];
		for(int i=0; i< items; i++) {
			tasks[i] = (int)SvIV(ST(i));
		}
		quest_manager.taskselector(items, tasks);
	} else {
		Perl_croak(aTHX_ "Usage: taskselector(taskid1, taskid2, ..., taskid%i)", MAXCHOOSERENTRIES);
	}

	XSRETURN_EMPTY;
}
XS(XS__tasksetselector);
XS(XS__tasksetselector)
{
	dXSARGS;
	if(items == 1) {
		int tasksetid = (int)SvIV(ST(0));
		quest_manager.tasksetselector(tasksetid);
	} else {
		Perl_croak(aTHX_ "Usage: tasksetselector(tasksetid)");
	}

	XSRETURN_EMPTY;
}
XS(XS__enabletask);
XS(XS__enabletask)
{
	dXSARGS;
	if((items >= 1) && (items <=10)) {
		int tasks[10];
		for(int i=0; i< items; i++) {
			tasks[i] = (int)SvIV(ST(i));
		}
		quest_manager.enabletask(items, tasks);
	} else {
		Perl_croak(aTHX_ "Usage: enabletask(taskid1, taskid2, ..., taskid10");
	}

	XSRETURN_EMPTY;
}
XS(XS__disabletask);
XS(XS__disabletask)
{
	dXSARGS;
	if((items >= 1) && (items <=10)) {
		int tasks[10];
		for(int i=0; i< items; i++) {
			tasks[i] = (int)SvIV(ST(i));
		}
		quest_manager.disabletask(items, tasks);
	} else {
		Perl_croak(aTHX_ "Usage: disabletask(taskid1, taskid2, ..., taskid10");
	}

	XSRETURN_EMPTY;
}

XS(XS__istaskenabled);
XS(XS__istaskenabled)
{
	dXSARGS;
	bool		RETVAL;
	dXSTARG;

	if(items == 1) {
		unsigned int taskid = (int)SvIV(ST(0));
		RETVAL = quest_manager.istaskenabled(taskid);
	} else {
		Perl_croak(aTHX_ "Usage: istaskenabled(taskid)");
	}

	XSprePUSH; PUSHu((IV)RETVAL);

	XSRETURN(1);
}
XS(XS__istaskactive);
XS(XS__istaskactive)
{
	dXSARGS;
	bool		RETVAL;
	dXSTARG;

	if(items == 1) {
		unsigned int task = (int)SvIV(ST(0));
		RETVAL = quest_manager.istaskactive(task);
	} else {
		Perl_croak(aTHX_ "Usage: istaskactive(task)");
	}

	XSprePUSH; PUSHu((IV)RETVAL);

	XSRETURN(1);
}
XS(XS__istaskactivityactive);
XS(XS__istaskactivityactive)
{
	dXSARGS;
	bool		RETVAL;
	dXSTARG;

	if(items == 2) {
		unsigned int task = (int)SvIV(ST(0));
		unsigned int activity = (int)SvIV(ST(1));
		RETVAL = quest_manager.istaskactivityactive(task, activity);
	} else {
		Perl_croak(aTHX_ "Usage: istaskactivityactive(task,activity)");
	}

	XSprePUSH; PUSHu((IV)RETVAL);

	XSRETURN(1);
}
XS(XS__gettaskactivitydonecount);
XS(XS__gettaskactivitydonecount)
{
	dXSARGS;
	uint32		RETVAL;
	dXSTARG;

	if(items == 2) {
		unsigned int task = (int)SvIV(ST(0));
		unsigned int activity = (int)SvIV(ST(1));
		RETVAL = quest_manager.gettaskactivitydonecount(task, activity);
		XSprePUSH; PUSHu((UV)RETVAL);
	} else {
		Perl_croak(aTHX_ "Usage: gettaskactivitydonecount(task,activity)");
	}

	XSRETURN(1);
}
XS(XS__updatetaskactivity);
XS(XS__updatetaskactivity)
{
	dXSARGS;
	unsigned int task, activity;
	int count = 1;
	bool ignore_quest_update = false;
	if(items == 2) {
		task = (int)SvIV(ST(0));
		activity = (int)SvIV(ST(1));
		quest_manager.updatetaskactivity(task, activity, count, false);
	}
	else if (items == 3 || items == 4) {
		task = (int)SvIV(ST(0));
		activity = (int)SvIV(ST(1));
		count = (int)SvIV(ST(2));
		if (items == 4){
			bool	ignore_quest_update = (bool)SvTRUE(ST(3)); 
		}
		quest_manager.updatetaskactivity(task, activity, count, ignore_quest_update);
	} else {
		Perl_croak(aTHX_ "Usage: updatetaskactivity(task, activity, [count], [ignore_quest_update])");
	}

	XSRETURN_EMPTY;
}

XS(XS__resettaskactivity);
XS(XS__resettaskactivity)
{
	dXSARGS;
	unsigned int task, activity;
	if(items == 2) {
		task = (int)SvIV(ST(0));
		activity = (int)SvIV(ST(1));
		quest_manager.resettaskactivity(task, activity);
	} else {
		Perl_croak(aTHX_ "Usage: resettaskactivity(task, activity)");
	}

	XSRETURN_EMPTY;
}

XS(XS__taskexploredarea);
XS(XS__taskexploredarea)
{
	dXSARGS;
	unsigned int exploreid;
	if(items == 1) {
		exploreid = (int)SvIV(ST(0));
		quest_manager.taskexploredarea(exploreid);
	} else {
		Perl_croak(aTHX_ "Usage: taskexplorearea(exploreid)");
	}

	XSRETURN_EMPTY;
}

XS(XS__assigntask);
XS(XS__assigntask)
{
	dXSARGS;
	unsigned int taskid;
	bool enforce_level_requirement = false;
	if(items == 1 || items == 2) {
		taskid = (int)SvIV(ST(0));
		if (items == 2)
		{
			if ((int)SvIV(ST(1)) == 1)
			{
				enforce_level_requirement = true;
			}
		}
		quest_manager.assigntask(taskid, enforce_level_requirement);
	} else {
		Perl_croak(aTHX_ "Usage: assigntask(taskid, enforce_level_requirement)");
	}

	XSRETURN_EMPTY;
}

XS(XS__failtask);
XS(XS__failtask)
{
	dXSARGS;
	unsigned int taskid;
	if(items == 1) {
		taskid = (int)SvIV(ST(0));
		quest_manager.failtask(taskid);
	} else {
		Perl_croak(aTHX_ "Usage: failtask(taskid)");
	}

	XSRETURN_EMPTY;
}

XS(XS__tasktimeleft);
XS(XS__tasktimeleft)
{
	dXSARGS;
	int		RETVAL;
	dXSTARG;

	if(items == 1) {
		unsigned int taskid = (int)SvIV(ST(0));
		RETVAL = quest_manager.tasktimeleft(taskid);
	} else {
		Perl_croak(aTHX_ "Usage: tasktimeleft(taskid)");
	}

	XSprePUSH; PUSHi((IV)RETVAL);

	XSRETURN(1);
}

XS(XS__istaskcompleted);
XS(XS__istaskcompleted)
{
	dXSARGS;
	int		RETVAL;
	dXSTARG;

	if(items == 1) {
		unsigned int taskid = (int)SvIV(ST(0));
		RETVAL = quest_manager.istaskcompleted(taskid);
	} else {
		Perl_croak(aTHX_ "Usage: istaskcompleted(taskid)");
	}

	XSprePUSH; PUSHi((IV)RETVAL);

	XSRETURN(1);
}

XS(XS__enabledtaskcount);
XS(XS__enabledtaskcount)
{
	dXSARGS;
	int		RETVAL;
	dXSTARG;

	if(items == 1) {
		unsigned int taskset = (int)SvIV(ST(0));
		RETVAL = quest_manager.enabledtaskcount(taskset);
	} else {
		Perl_croak(aTHX_ "Usage: enabledtaskcount(taskset)");
	}

	XSprePUSH; PUSHi((IV)RETVAL);

	XSRETURN(1);
}

XS(XS__firsttaskinset);
XS(XS__firsttaskinset)
{
	dXSARGS;
	int		RETVAL;
	dXSTARG;

	if(items == 1) {
		unsigned int taskset = (int)SvIV(ST(0));
		RETVAL = quest_manager.firsttaskinset(taskset);
	} else {
		Perl_croak(aTHX_ "Usage: firsttaskinset(taskset)");
	}

	XSprePUSH; PUSHi((IV)RETVAL);

	XSRETURN(1);
}

XS(XS__lasttaskinset);
XS(XS__lasttaskinset)
{
	dXSARGS;
	int		RETVAL;
	dXSTARG;

	if(items == 1) {
		unsigned int taskset = (int)SvIV(ST(0));
		RETVAL = quest_manager.lasttaskinset(taskset);
	} else {
		Perl_croak(aTHX_ "Usage: lasttaskinset(taskset)");
	}

	XSprePUSH; PUSHi((IV)RETVAL);

	XSRETURN(1);
}

XS(XS__nexttaskinset);
XS(XS__nexttaskinset)
{
	dXSARGS;
	int		RETVAL;
	dXSTARG;

	if(items == 2) {
		unsigned int taskset = (int)SvIV(ST(0));
		unsigned int taskid = (int)SvIV(ST(1));
		RETVAL = quest_manager.nexttaskinset(taskset, taskid);
	} else {
		Perl_croak(aTHX_ "Usage: nexttaskinset(taskset, taskid)");
	}

	XSprePUSH; PUSHi((IV)RETVAL);

	XSRETURN(1);
}
XS(XS__activespeaktask);
XS(XS__activespeaktask)
{
	dXSARGS;
	int		RETVAL;
	dXSTARG;

	if(items == 0) {
		RETVAL = quest_manager.activespeaktask();
	} else {
		Perl_croak(aTHX_ "Usage: activespeaktask()");
	}

	XSprePUSH; PUSHi((IV)RETVAL);

	XSRETURN(1);
}

XS(XS__activespeakactivity);
XS(XS__activespeakactivity)
{
	dXSARGS;
	int		RETVAL;
	dXSTARG;

	if(items == 1) {
		unsigned int taskid = (int)SvIV(ST(0));
		RETVAL = quest_manager.activespeakactivity(taskid);
	} else {
		Perl_croak(aTHX_ "Usage: activespeakactivity(taskid)");
	}

	XSprePUSH; PUSHi((IV)RETVAL);

	XSRETURN(1);
}

XS(XS__activetasksinset);
XS(XS__activetasksinset)
{
	dXSARGS;
	int		RETVAL;
	dXSTARG;

	if(items == 1) {
		unsigned int taskset = (int)SvIV(ST(0));
		RETVAL = quest_manager.activetasksinset(taskset);
	} else {
		Perl_croak(aTHX_ "Usage: activetasksinset(taskset)");
	}

	XSprePUSH; PUSHi((IV)RETVAL);

	XSRETURN(1);
}

XS(XS__completedtasksinset);
XS(XS__completedtasksinset)
{
	dXSARGS;
	int		RETVAL;
	dXSTARG;

	if(items == 1) {
		unsigned int taskset = (int)SvIV(ST(0));
		RETVAL = quest_manager.completedtasksinset(taskset);
	} else {
		Perl_croak(aTHX_ "Usage: completedtasksinset(taskset)");
	}

	XSprePUSH; PUSHi((IV)RETVAL);

	XSRETURN(1);
}


XS(XS__istaskappropriate);
XS(XS__istaskappropriate)
{
	dXSARGS;
	bool		RETVAL;
	dXSTARG;

	if(items == 1) {
		unsigned int task = (int)SvIV(ST(0));
		RETVAL = quest_manager.istaskappropriate(task);
	} else {
		Perl_croak(aTHX_ "Usage: istaskaappropriate(task)");
	}

	XSprePUSH; PUSHu((IV)RETVAL);

	XSRETURN(1);
}

 XS(XS__popup); // prototype to pass -Wmissing-prototypes
 XS(XS__popup) {
		dXSARGS;
	int popupid = 0;
	int buttons = 0;
	int duration = 0;

	if((items < 2) || (items > 5))
				Perl_croak(aTHX_ "Usage: popup(windowtitle, text, popupid, buttons, duration)");

	if(items >= 3)
		popupid = (int)SvIV(ST(2));

	if(items >= 4)
		buttons = (int)SvIV(ST(3));

	if(items == 5)
		duration = (int)SvIV(ST(4));

		quest_manager.popup(SvPV_nolen(ST(0)), SvPV_nolen(ST(1)), popupid, buttons, duration);

		XSRETURN_EMPTY;
}

XS(XS__clearspawntimers);
XS(XS__clearspawntimers)
{
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: clearspawntimers()");

	quest_manager.clearspawntimers();

	XSRETURN_EMPTY;
}

XS(XS__ze);
XS(XS__ze)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: ze(type, str)");

	int	type = (int)SvIV(ST(0));
	char *		str = (char *)SvPV_nolen(ST(1));

	quest_manager.ze(type, str);

	XSRETURN_EMPTY;
}

XS(XS__we);
XS(XS__we)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: we(type, str)");

	int	type = (int)SvIV(ST(0));
	char *		str = (char *)SvPV_nolen(ST(1));

	quest_manager.we(type, str);

	XSRETURN_EMPTY;
}

XS(XS__getlevel);
XS(XS__getlevel)
{
	dXSARGS;
	if (items > 1)
		Perl_croak(aTHX_ "Usage: getlevel(type)");

	int	RETVAL;
	dXSTARG;

	int	type;
	if (items == 1)
		type = (int)SvIV(ST(0));
	else
		type = 0;

	RETVAL = quest_manager.getlevel(type);
	XSprePUSH; PUSHu((IV)RETVAL);

	XSRETURN(1);
}

XS(XS__CreateGroundObject);
XS(XS__CreateGroundObject)
{
	dXSARGS;
	if (items != 5 && items != 6)
		Perl_croak(aTHX_ "Usage: creategroundobject(itemid, x, y, z, heading, [decay_time])");

	int	itemid = (int)SvIV(ST(0));
	float x = (float)SvNV(ST(1));
	float y = (float)SvNV(ST(2));
	float z = (float)SvNV(ST(3));
	float heading = (float)SvNV(ST(4));
	uint16 id = 0;

	if(items == 5)
		id = quest_manager.CreateGroundObject(itemid, glm::vec4(x, y, z, heading));
	else{
		uint32 decay_time = (uint32)SvIV(ST(5));
		id = quest_manager.CreateGroundObject(itemid, glm::vec4(x, y, z, heading), decay_time);
	}

	XSRETURN_IV(id);
}

XS(XS__CreateGroundObjectFromModel);
XS(XS__CreateGroundObjectFromModel)
{
	dXSARGS;
	if (items < 5 || items > 7)
		Perl_croak(aTHX_ "Usage: creategroundobjectfrommodel(modelname, x, y, z, heading, [type], [decay_time])");

	char *		modelname = (char *)SvPV_nolen(ST(0));
	float x = (float)SvNV(ST(1));
	float y = (float)SvNV(ST(2));
	float z = (float)SvNV(ST(3));
	float heading = (float)SvNV(ST(4));
	uint32 type = 0;
	uint32 decay_time = 0;
	uint16 id = 0;

	if (items > 5)
		type = (uint32)SvIV(ST(5));

	if (items > 6)
		decay_time = (uint32)SvIV(ST(6));

	id = quest_manager.CreateGroundObjectFromModel(modelname, glm::vec4(x, y, z, heading), type, decay_time);
	XSRETURN_IV(id);
}

XS(XS__CreateDoor);
XS(XS__CreateDoor)
{
	dXSARGS;
	if (items < 5 || items > 7)
		Perl_croak(aTHX_ "Usage: createdoor(modelname, x, y, z, heading, [type], [size])");

	char *		modelname = (char *)SvPV_nolen(ST(0));
	float x = (float)SvNV(ST(1));
	float y = (float)SvNV(ST(2));
	float z = (float)SvNV(ST(3));
	float heading = (float)SvNV(ST(4));
	uint32 type = 58;
	uint32 size = 100;
	uint16 id = 0;

	if (items > 5)
		type = (uint32)SvIV(ST(5));

	if (items > 6)
		size = (uint32)SvIV(ST(6));

	id = quest_manager.CreateDoor(modelname, x, y, z, heading, type, size);
	XSRETURN_IV(id);
}

XS(XS__ModifyNPCStat);
XS(XS__ModifyNPCStat)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: ModifyNPCStat(identifier, newValue)");

	quest_manager.ModifyNPCStat(SvPV_nolen(ST(0)), SvPV_nolen(ST(1)));

	XSRETURN_EMPTY;
}

XS(XS__collectitems);
XS(XS__collectitems)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: collectitems(item_id, remove?)");

	uint32 item_id = (int)SvIV(ST(0));
	bool remove = ((int)SvIV(ST(1))) == 0?false:true;

	int quantity =
		quest_manager.collectitems(item_id, remove);

	XSRETURN_IV(quantity);
}

XS(XS__UpdateSpawnTimer);
XS(XS__UpdateSpawnTimer)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: UpdateSpawnTimer(Spawn2_ID, Updated_Time_Till_Repop)");

	uint32 id = (int)SvIV(ST(0));
	uint32 duration = (int)SvIV(ST(1));

	quest_manager.UpdateSpawnTimer(id, duration);

	XSRETURN_EMPTY;
}

XS(XS__MerchantSetItem);
XS(XS__MerchantSetItem) {
	dXSARGS;
	if (items != 2 && items != 3)
		Perl_croak(aTHX_ "Usage: MerchantSetItem(NPCid, itemid [, quantity])");

	uint32 NPCid = (int)SvUV(ST(0));
	uint32 itemid = (int)SvUV(ST(1));
	uint32 quantity = 0;
	if (items == 3)
		quantity = (int)SvUV(ST(2));

	quest_manager.MerchantSetItem(NPCid, itemid, quantity);

	XSRETURN_EMPTY;
}

XS(XS__MerchantCountItem);
XS(XS__MerchantCountItem) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: MerchantCountItem(NPCid, itemid)");

	uint32 NPCid = (int)SvUV(ST(0));
	uint32 itemid = (int)SvUV(ST(1));
	uint32 quantity = quest_manager.MerchantCountItem(NPCid, itemid);

	XSRETURN_UV(quantity);
}

XS(XS__varlink);
XS(XS__varlink) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: varlink(itemID)");
	dXSTARG;

	Const_char * RETVAL;
	char text[250];
	uint32 itemID;
	itemID = (int)SvUV(ST(0));

	RETVAL = quest_manager.varlink(text, itemID);

	sv_setpv(TARG, RETVAL); XSprePUSH; PUSHTARG;
	XSRETURN(1);
}

XS(XS__CreateInstance);
XS(XS__CreateInstance) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: CreateInstance(zone_name, version, duration)");

	char * zone = (char *)SvPV_nolen(ST(0));
	uint16 version = (int)SvUV(ST(1));
	uint32 duration = (int)SvUV(ST(2));
	uint32 id = quest_manager.CreateInstance(zone, version, duration);

	XSRETURN_UV(id);
}

XS(XS__DestroyInstance);
XS(XS__DestroyInstance) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: DestroyInstance(id)");

	uint16 id = (int)SvUV(ST(0));
	quest_manager.DestroyInstance(id);

	XSRETURN_EMPTY;
}

XS(XS__UpdateInstanceTimer);
XS(XS__UpdateInstanceTimer) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: UpdateInstanceTimer(instance_id, new_duration)");

	uint16 instance_id = (uint16)SvUV(ST(0));
	uint32 new_duration = (uint32)SvUV(ST(1));
	quest_manager.UpdateInstanceTimer(instance_id, new_duration);

	XSRETURN_EMPTY;
}

XS(XS__GetInstanceTimer);
XS(XS__GetInstanceTimer) {
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: GetInstanceTimer()");
	
	uint32 timer = quest_manager.GetInstanceTimer();
	
	XSRETURN_UV(timer);
}

XS(XS__GetInstanceTimerByID);
XS(XS__GetInstanceTimerByID) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: GetInstanceTimerByID(instance_id)");
	
	uint16 instance_id = (uint16)SvUV(ST(0));
	uint32 timer = quest_manager.GetInstanceTimerByID(instance_id);
	
	XSRETURN_UV(timer);
}

XS(XS__GetInstanceID);
XS(XS__GetInstanceID) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: GetInstanceID(zone_name, version)");

	char * zone = (char *)SvPV_nolen(ST(0));
	uint16 version = (int)SvUV(ST(1));
	uint16 id = quest_manager.GetInstanceID(zone, version);

	XSRETURN_UV(id);
}

XS(XS__GetCharactersInInstance);
XS(XS__GetCharactersInInstance) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: GetCharactersInInstance(instance_id)");
	dXSTARG;

	Const_char * RETVAL;
	uint16 instance_id = (int)SvUV(ST(0));
	std::list<uint32> charid_list;
	std::string charid_string;

	database.GetCharactersInInstance(instance_id, charid_list);

	if (charid_list.size() > 0)
	{
		charid_string = itoa(charid_list.size());
		charid_string += " player(s) in instance: ";
		auto iter = charid_list.begin();
		while (iter != charid_list.end())
		{
			char char_name[64];
			database.GetCharName(*iter, char_name);
			charid_string += char_name;
			charid_string += "(";
			charid_string += itoa(*iter);
			charid_string += ")";
			++iter;
			if (iter != charid_list.end())
				charid_string += ", ";
		}
		RETVAL = charid_string.c_str();
	}
	else
		RETVAL = "No players in that instance.";

	sv_setpv(TARG, RETVAL); XSprePUSH; PUSHTARG;
	XSRETURN(1);
}

XS(XS__AssignToInstance);
XS(XS__AssignToInstance) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: AssignToInstance(instance_id)");

	uint16 instance_id = (int)SvUV(ST(0));
	quest_manager.AssignToInstance(instance_id);

	XSRETURN_EMPTY;
}

XS(XS__AssignGroupToInstance);
XS(XS__AssignGroupToInstance) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: AssignGroupToInstance(instance_id)");

	uint16 instance_id = (int)SvUV(ST(0));
	quest_manager.AssignGroupToInstance(instance_id);

	XSRETURN_EMPTY;
}

XS(XS__AssignRaidToInstance);
XS(XS__AssignRaidToInstance) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: AssignRaidToInstance(instance_id)");

	uint16 instance_id = (int)SvUV(ST(0));
	quest_manager.AssignRaidToInstance(instance_id);

	XSRETURN_EMPTY;
}

XS(XS__RemoveFromInstance);
XS(XS__RemoveFromInstance) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: RemoveFromInstance(instance_id)");

	uint16 instance_id = (int)SvUV(ST(0));
	quest_manager.RemoveFromInstance(instance_id);

	XSRETURN_EMPTY;
}

XS(XS__RemoveAllFromInstance);
XS(XS__RemoveAllFromInstance) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: RemoveAllFromInstance(instance_id)");

	uint16 instance_id = (int)SvUV(ST(0));
	quest_manager.RemoveAllFromInstance(instance_id);

	XSRETURN_EMPTY;
}

XS(XS__MovePCInstance);
XS(XS__MovePCInstance)
{
	dXSARGS;
	if (items != 5 && items != 6)
		Perl_croak(aTHX_ "Usage: MovePCInstance(zone_id, instance_id, x, y, z [,heading])");

	int	zoneid = (int)SvIV(ST(0));
	int	instanceid = (int)SvIV(ST(1));
	float	x = (float)SvNV(ST(2));
	float	y = (float)SvNV(ST(3));
	float	z = (float)SvNV(ST(4));

	if (items == 4)
	{
		quest_manager.MovePCInstance(zoneid, instanceid, glm::vec4(x, y, z, 0.0f));
	}
	else
	{
		float heading = (float)SvNV(ST(5));
		quest_manager.MovePCInstance(zoneid, instanceid, glm::vec4(x, y, z, heading));
	}

	XSRETURN_EMPTY;
}

XS(XS__FlagInstanceByGroupLeader);
XS(XS__FlagInstanceByGroupLeader) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: FlagInstanceByGroupLeader(zone, version)");

	uint32 zone = (int)SvUV(ST(0));
	uint16 version = (int)SvUV(ST(1));
	quest_manager.FlagInstanceByGroupLeader(zone, version);

	XSRETURN_EMPTY;
}

XS(XS__FlagInstanceByRaidLeader);
XS(XS__FlagInstanceByRaidLeader) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: FlagInstanceByRaidLeader(zone, version)");

	uint32 zone = (int)SvUV(ST(0));
	uint16 version = (int)SvUV(ST(1));
	quest_manager.FlagInstanceByRaidLeader(zone, version);

	XSRETURN_EMPTY;
}

XS(XS__saylink);
XS(XS__saylink) {
	dXSARGS;
	if (items < 1 || items > 3)
		Perl_croak(aTHX_ "Usage: saylink(phrase,[silent?],[linkname])");
	dXSTARG;

	Const_char * RETVAL;
	char text[250];
	char text2[250];
	bool silent = false;
	strcpy(text,(char *)SvPV_nolen(ST(0)));
	if(items >= 2) {
		silent = ((int)SvIV(ST(1))) == 0 ? false : true;
	}
	if (items == 3)
		strcpy(text2,(char *)SvPV_nolen(ST(2)));
	else
		strcpy(text2,text);

	RETVAL = quest_manager.saylink(text, silent, text2);
	sv_setpv(TARG, RETVAL); XSprePUSH; PUSHTARG;
	XSRETURN(1);
}

XS(XS__getguildnamebyid);
XS(XS__getguildnamebyid) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: getguildnamebyid(guild_id)");
	dXSTARG;

	Const_char * RETVAL;
	uint32 guild_id = (int)SvUV(ST(0));

	RETVAL = quest_manager.getguildnamebyid(guild_id);

	sv_setpv(TARG, RETVAL); XSprePUSH; PUSHTARG;
	XSRETURN(1);
}

XS(XS__SetRunning);
XS(XS__SetRunning)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: SetRunning(val)");

	bool val = ((int)SvIV(ST(0))) == 0?false:true;

	quest_manager.SetRunning(val);

	XSRETURN_EMPTY;
}

XS(XS__IsRunning);
XS(XS__IsRunning)
{
	dXSARGS;
	if (items >= 1)
		Perl_croak(aTHX_ "Usage: IsRunning()");

	bool		RETVAL;
	dXSTARG;


	RETVAL = quest_manager.IsRunning();
	XSprePUSH; PUSHu((IV)RETVAL);

	XSRETURN(1);
}

XS(XS__IsEffectInSpell);
XS(XS__IsEffectInSpell)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: IsEffectInSpell(spell_id, effect_id)");

	uint32		spell_id = (uint32)SvUV(ST(0));
	uint32		effect_id = (uint32)SvUV(ST(1));
	bool		RETVAL;
	dXSTARG;


	RETVAL = IsEffectInSpell(spell_id, effect_id);
	XSprePUSH; PUSHu((IV)RETVAL);

	XSRETURN(1);
}

XS(XS__IsBeneficialSpell);
XS(XS__IsBeneficialSpell)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: IsBeneficialSpell(spell_id)");

	uint32		spell_id = (uint32)SvUV(ST(0));
	bool		RETVAL;
	dXSTARG;


	RETVAL = BeneficialSpell(spell_id);
	XSprePUSH; PUSHu((IV)RETVAL);

	XSRETURN(1);
}

XS(XS__GetSpellResistType);
XS(XS__GetSpellResistType)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: GetSpellResistType(spell_id)");

	uint32		spell_id = (uint32)SvUV(ST(0));
	int32		spell_val = 0;
	dXSTARG;

	spell_val = GetSpellResistType(spell_id);
	XSRETURN_UV(spell_val);
}

XS(XS__GetSpellTargetType);
XS(XS__GetSpellTargetType)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: GetSpellTargetType(spell_id)");

	uint32		spell_id = (uint32)SvUV(ST(0));
	int32		spell_val = 0;
	dXSTARG;

	spell_val = GetSpellTargetType(spell_id);
	XSRETURN_UV(spell_val);
}

XS(XS__FlyMode);
XS(XS__FlyMode) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: FlyMode([0/1/2])");

	uint8 flymode = (int)SvUV(ST(0));
	quest_manager.FlyMode(flymode);

	XSRETURN_EMPTY;
}

XS(XS_FactionValue);
XS(XS_FactionValue) {
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: factionvalue()");

	uint8 fac = quest_manager.FactionValue();
	XSRETURN_UV(fac);
}

XS(XS__enabletitle);
XS(XS__enabletitle)
{
   dXSARGS;
   if (items != 1)
	  Perl_croak(aTHX_ "Usage: enabletitle(titleset)");

   int   titleset = (int)SvIV(ST(0));

   quest_manager.enabletitle(titleset);

   XSRETURN_EMPTY;
}

XS(XS__checktitle);
XS(XS__checktitle)
{
   dXSARGS;
   if (items != 1)
	  Perl_croak(aTHX_ "Usage: checktitle(titleset)");

   bool RETVAL;
   int   titleset = (int)SvIV(ST(0));

   RETVAL = quest_manager.checktitle(titleset);

	ST(0) = boolSV(RETVAL);
	sv_2mortal(ST(0));
	XSRETURN(1);
}

XS(XS__removetitle);
XS(XS__removetitle)
{
   dXSARGS;
   if (items != 1)
	  Perl_croak(aTHX_ "Usage: removetitle(titleset)");

   int   titleset = (int)SvIV(ST(0));

   quest_manager.removetitle(titleset);

   XSRETURN_EMPTY;
}

XS(XS__wearchange);
XS(XS__wearchange)
{
	dXSARGS;
	if (items < 2)
		Perl_croak(aTHX_ "Usage: wearchange(slot, texture, [hero_forge_model], [elite_material])");

	uint8		slot = (int)SvUV(ST(0));
	uint16		texture = (int)SvUV(ST(1));

	uint32 hero_forge_model = 0;
	uint32 elite_material = 0;

	if (items > 2)
		hero_forge_model = (int)SvUV(ST(2));

	if (items > 3)
		elite_material = (int)SvUV(ST(3));

	quest_manager.wearchange(slot, texture, hero_forge_model, elite_material);

	XSRETURN_EMPTY;
}

XS(XS__voicetell);
XS(XS__voicetell)
{
	dXSARGS;
	if (items != 4)
		Perl_croak(aTHX_ "Usage: voicetell(clientname, type, race, gender)");

	char *	str = (char *)SvPV_nolen(ST(0));
	int	macronum = (int)SvIV(ST(1));
	int	racenum = (int)SvIV(ST(2));
	int	gendernum = (int)SvIV(ST(3));

	quest_manager.voicetell(str, macronum, racenum, gendernum);

	XSRETURN_EMPTY;
}

XS(XS__LearnRecipe);
XS(XS__LearnRecipe)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: LearnRecipe(recipe_id)");

	uint32 recipe_id = (uint32)SvIV(ST(0));

	quest_manager.LearnRecipe(recipe_id);

	XSRETURN_EMPTY;
}

XS(XS__SendMail);
XS(XS__SendMail)
{
	dXSARGS;
	if (items != 4)
		Perl_croak(aTHX_ "Usage: SendMail(to, from, subject, message)");

	char *to = (char *)SvPV_nolen(ST(0));
	char *from = (char *)SvPV_nolen(ST(1));
	char *subject = (char *)SvPV_nolen(ST(2));
	char *message = (char *)SvPV_nolen(ST(3));

	quest_manager.SendMail(to, from, subject, message);

	XSRETURN_EMPTY;
}

XS(XS__GetZoneID);
XS(XS__GetZoneID)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: GetZoneID(zone)");

	char *zone = (char *)SvPV_nolen(ST(0));
	int32 id = quest_manager.GetZoneID(zone);

	XSRETURN_IV(id);
}

XS(XS__GetZoneLongName);
XS(XS__GetZoneLongName)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: GetZoneLongName(zone)");
	dXSTARG;
	char *zone = (char *)SvPV_nolen(ST(0));
	Const_char* RETVAL = quest_manager.GetZoneLongName(zone);

	sv_setpv(TARG, RETVAL); XSprePUSH; PUSHTARG;
	XSRETURN(1);
}

XS(XS__GetTimeSeconds);
XS(XS__GetTimeSeconds)
{
	dXSARGS;
	if (items != 0)
		Perl_croak(aTHX_ "Usage: GetTimeSeconds()");

	uint32		seconds = 0;
	dXSTARG;

	seconds = Timer::GetTimeSeconds();
	XSRETURN_UV(seconds);
}

XS(XS__crosszonesignalclientbycharid);
XS(XS__crosszonesignalclientbycharid)
{
	dXSARGS;

	if (items != 2)
		Perl_croak(aTHX_ "Usage: crosszonesignalclientbycharid(char_id, data)");

	if (items == 2) {
		int	char_id = (int)SvIV(ST(0));
		uint32 data = (uint32)SvIV(ST(1));
		quest_manager.CrossZoneSignalPlayerByCharID(char_id, data);
	} else {
		Perl_croak(aTHX_ "Usage: crosszonesignalclientbycharid(char_id, data)");
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonesignalclientbyname);
XS(XS__crosszonesignalclientbyname)
{
	dXSARGS;

	if (items != 2)
		Perl_croak(aTHX_ "Usage: crosszonesignalclientbycharid(Name, data)");

	if (items == 2) {
		char *Name = (char *)SvPV_nolen(ST(0));
		uint32 data = (uint32)SvIV(ST(1));
		quest_manager.CrossZoneSignalPlayerByName(Name, data);
	} else {
		Perl_croak(aTHX_ "Usage: crosszonesignalclientbycharid(Name, data)");
	}

	XSRETURN_EMPTY;
}


XS(XS__crosszonemessageplayerbyname);
XS(XS__crosszonemessageplayerbyname)
{
	dXSARGS;

	if (items != 3)
		Perl_croak(aTHX_ "Usage: crosszonemessageplayerbyname(Type, Name, Message)");

	if (items == 3) {
		uint32 Type = (uint32)SvIV(ST(0));
		char *Name = (char *)SvPV_nolen(ST(1));
		char *Message = (char *)SvPV_nolen(ST(2));
		quest_manager.CrossZoneMessagePlayerByName(Type, Name, Message);
	} else {
		Perl_croak(aTHX_ "Usage: crosszonemessageplayerbyname(Type, Name, Message)");
	}

	XSRETURN_EMPTY;
}

XS(XS__enablerecipe);
XS(XS__enablerecipe)
{
	dXSARGS;
	bool success = false;

	if (items != 1) {
		Perl_croak(aTHX_ "Usage: enablerecipe(recipe_id)");
	}
	else {
		uint32 recipe_id = (uint32)SvIV(ST(0));
		success = quest_manager.EnableRecipe(recipe_id);
	}
	if (!success) {
		XSRETURN_NO;
	}

	XSRETURN_YES;
}

XS(XS__disablerecipe);
XS(XS__disablerecipe)
{
	dXSARGS;
	bool success = false;

	if (items != 1) {
		Perl_croak(aTHX_ "Usage: disablerecipe(recipe_id)");
	}
	else {
		uint32 recipe_id = (uint32)SvIV(ST(0));
		success = quest_manager.DisableRecipe(recipe_id);
	}
	if (!success) {
		XSRETURN_NO;
	}

	XSRETURN_YES;
}

XS(XS__clear_npctype_cache);
XS(XS__clear_npctype_cache)
{
	dXSARGS;

	if (items != 1) {
		Perl_croak(aTHX_ "Usage: clear_npctype_cache(npc_id)");
	}
	else {
		int32 npctype_id = (int32)SvIV(ST(0));
		quest_manager.ClearNPCTypeCache(npctype_id);
	}

	XSRETURN_EMPTY;
}

XS(XS__reloadzonestaticdata);
XS(XS__reloadzonestaticdata)
{
	dXSARGS;

	quest_manager.ReloadZoneStaticData();

	XSRETURN_EMPTY;
}

XS(XS__qs_send_query);
XS(XS__qs_send_query)
{
	dXSARGS;
	if (items != 1){
		Perl_croak(aTHX_ "Usage: qs_send_query(query)");
	}
	else{
		// char *Query = (char *)SvPV_nolen(ST(0));
		std::string Query = (std::string)SvPV_nolen(ST(0));
		QServ->SendQuery(Query);
	}
	XSRETURN_EMPTY;
}

XS(XS__qs_player_event);
XS(XS__qs_player_event)
{
	dXSARGS;
	if (items != 2){
		Perl_croak(aTHX_ "Usage: qs_player_event(char_id, event_desc)");
	}
	else{
		int	char_id = (int)SvIV(ST(0));
		std::string event_desc = (std::string)SvPV_nolen(ST(1));
		QServ->PlayerLogEvent(Player_Log_Quest, char_id, event_desc);
	}
	XSRETURN_EMPTY;
}

XS(XS__crosszonesetentityvariablebynpctypeid);
XS(XS__crosszonesetentityvariablebynpctypeid)
{
	dXSARGS;

	if (items != 3)
		Perl_croak(aTHX_ "Usage: crosszonesetentityvariablebynpctypeid(npctype_id, id, m_var)");

	if (items == 3) {
		uint32 npctype_id = (uint32)SvIV(ST(0));
		const char *id = (const char *)SvPV_nolen(ST(1));
		const char *m_var = (const char *)SvPV_nolen(ST(2));
		quest_manager.CrossZoneSetEntityVariableByNPCTypeID(npctype_id, id, m_var);
	}

	XSRETURN_EMPTY;
}

XS(XS__crosszonesignalnpcbynpctypeid);
XS(XS__crosszonesignalnpcbynpctypeid)
{
	dXSARGS;

	if (items != 2)
		Perl_croak(aTHX_ "Usage: crosszonesignalnpcbynpctypeid(npctype_id, data)");

	if (items == 2) {
		uint32 npctype_id = (uint32)SvIV(ST(0));
		uint32 data = (uint32)SvIV(ST(1));
		quest_manager.CrossZoneSignalNPCByNPCTypeID(npctype_id, data);
	}

	XSRETURN_EMPTY;
}

XS(XS__debug);
XS(XS__debug)
{
	dXSARGS;
	if (items != 1 && items != 2){
		Perl_croak(aTHX_ "Usage: debug(message, [debug_level])");
	}
	else{
		std::string log_message = (std::string)SvPV_nolen(ST(0));
		uint8 debug_level = 1;

		if (items == 2)
			debug_level = (uint8)SvIV(ST(1));

		if (debug_level > Logs::Detail)
			return;

		if (debug_level == Logs::General){
			Log.Out(Logs::General, Logs::QuestDebug, log_message);
		}
		else if (debug_level == Logs::Moderate){
			Log.Out(Logs::Moderate, Logs::QuestDebug, log_message);
		}
		else if (debug_level == Logs::Detail){
			Log.Out(Logs::Detail, Logs::QuestDebug, log_message);
		}
	}
	XSRETURN_EMPTY;
}

XS(XS__UpdateZoneHeader);
XS(XS__UpdateZoneHeader) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: UpdateZoneHeader(type, value)");
	
	std::string type = (std::string)SvPV_nolen(ST(0));
	std::string value = (std::string)SvPV_nolen(ST(1));
	quest_manager.UpdateZoneHeader(type, value);
	
	XSRETURN_EMPTY;
}


/*
This is the callback perl will look for to setup the
quest package's XSUBs
*/
EXTERN_C XS(boot_quest); // prototype to pass -Wmissing-prototypes
EXTERN_C XS(boot_quest)
{
	dXSARGS;
	char file[256];
	strncpy(file, __FILE__, 256);
	file[255] = '\0';

	if(items != 1)
		Log.Out(Logs::General, Logs::Error, "boot_quest does not take any arguments.");

	char buf[128];	//shouldent have any function names longer than this.

	//add the strcpy stuff to get rid of const warnings....

	XS_VERSION_BOOTCHECK ;

#ifdef BOTS
	newXS(strcpy(buf, "botquest"), XS__botquest, file);
	newXS(strcpy(buf, "spawnbotcount"), XS__spawnbotcount, file);
	newXS(strcpy(buf, "createbotcount"), XS__createbotcount, file);
	newXS(strcpy(buf, "createBot"), XS__createBot, file);
#endif //BOTS

		newXS(strcpy(buf, "AssignGroupToInstance"), XS__AssignGroupToInstance, file);
		newXS(strcpy(buf, "AssignRaidToInstance"), XS__AssignRaidToInstance, file);
		newXS(strcpy(buf, "AssignToInstance"), XS__AssignToInstance, file);
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
		newXS(strcpy(buf, "GetSpellResistType"), XS__GetSpellResistType, file);
		newXS(strcpy(buf, "GetSpellTargetType"), XS__GetSpellTargetType, file);
		newXS(strcpy(buf, "GetTimeSeconds"), XS__GetTimeSeconds, file);
		newXS(strcpy(buf, "GetZoneID"), XS__GetZoneID, file);
		newXS(strcpy(buf, "GetZoneLongName"), XS__GetZoneLongName, file);
		newXS(strcpy(buf, "IsBeneficialSpell"), XS__IsBeneficialSpell, file);
		newXS(strcpy(buf, "IsEffectInSpell"), XS__IsEffectInSpell, file);
		newXS(strcpy(buf, "IsRunning"), XS__IsRunning, file);
		newXS(strcpy(buf, "LearnRecipe"), XS__LearnRecipe, file);
		newXS(strcpy(buf, "MerchantCountItem"), XS__MerchantCountItem, file);
		newXS(strcpy(buf, "MerchantSetItem"), XS__MerchantSetItem, file);
		newXS(strcpy(buf, "MovePCInstance"), XS__MovePCInstance, file);
		newXS(strcpy(buf, "RemoveAllFromInstance"), XS__RemoveAllFromInstance, file);
		newXS(strcpy(buf, "RemoveFromInstance"), XS__RemoveFromInstance, file);
		newXS(strcpy(buf, "SendMail"), XS__SendMail, file);
		newXS(strcpy(buf, "SetRunning"), XS__SetRunning, file);
		newXS(strcpy(buf, "activespeakactivity"), XS__activespeakactivity, file);
		newXS(strcpy(buf, "activespeaktask"), XS__activespeaktask, file);
		newXS(strcpy(buf, "activetasksinset"), XS__activetasksinset, file);
		newXS(strcpy(buf, "addldonloss"), XS__addldonpoints, file);
		newXS(strcpy(buf, "addldonpoints"), XS__addldonpoints, file);
		newXS(strcpy(buf, "addldonwin"), XS__addldonpoints, file);
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
		newXS(strcpy(buf, "createdoor"), XS__CreateDoor, file);
		newXS(strcpy(buf, "creategroundobject"), XS__CreateGroundObject, file);
		newXS(strcpy(buf, "creategroundobjectfrommodel"), XS__CreateGroundObjectFromModel, file);
		newXS(strcpy(buf, "createguild"), XS__createguild, file);
		newXS(strcpy(buf, "crosszonemessageplayerbyname"), XS__crosszonemessageplayerbyname, file);
		newXS(strcpy(buf, "crosszonesetentityvariablebynpctypeid"), XS__crosszonesetentityvariablebynpctypeid, file);
		newXS(strcpy(buf, "crosszonesignalclientbycharid"), XS__crosszonesignalclientbycharid, file);
		newXS(strcpy(buf, "crosszonesignalclientbyname"), XS__crosszonesignalclientbyname, file);
		newXS(strcpy(buf, "crosszonesignalnpcbynpctypeid"), XS__crosszonesignalnpcbynpctypeid, file);
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
		newXS(strcpy(buf, "getItemName"), XS_qc_getItemName, file);
		newXS(strcpy(buf, "get_spawn_condition"), XS__get_spawn_condition, file);
		newXS(strcpy(buf, "getguildnamebyid"), XS__getguildnamebyid, file);
		newXS(strcpy(buf, "getlevel"), XS__getlevel, file);
		newXS(strcpy(buf, "getplayerburiedcorpsecount"), XS__getplayerburiedcorpsecount, file);
		newXS(strcpy(buf, "gettaskactivitydonecount"), XS__gettaskactivitydonecount, file);
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
		newXS(strcpy(buf, "me"), XS__me, file);
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
		newXS(strcpy(buf, "pvp"), XS__pvp, file);
		newXS(strcpy(buf, "qs_player_event"), XS__qs_player_event, file); 
		newXS(strcpy(buf, "qs_send_query"), XS__qs_send_query, file); 
		newXS(strcpy(buf, "rain"), XS__rain, file);
		newXS(strcpy(buf, "rebind"), XS__rebind, file);
		newXS(strcpy(buf, "reloadzonestaticdata"), XS__reloadzonestaticdata, file);
		newXS(strcpy(buf, "removetitle"), XS__removetitle, file);
		newXS(strcpy(buf, "repopzone"), XS__repopzone, file);
		newXS(strcpy(buf, "ConnectNodeToNode"), XS__ConnectNodeToNode, file);
		newXS(strcpy(buf, "AddNode"), XS__AddNode, file);
		newXS(strcpy(buf, "resettaskactivity"), XS__resettaskactivity, file);
		newXS(strcpy(buf, "respawn"), XS__respawn, file);
		newXS(strcpy(buf, "resume"), XS__resume, file);
		newXS(strcpy(buf, "safemove"), XS__safemove, file);
		newXS(strcpy(buf, "save"), XS__save, file);
		newXS(strcpy(buf, "say"), XS__say, file);
		newXS(strcpy(buf, "saylink"), XS__saylink, file);
		newXS(strcpy(buf, "scribespells"), XS__scribespells, file);
		newXS(strcpy(buf, "selfcast"), XS__selfcast, file);
		newXS(strcpy(buf, "set_proximity"), XS__set_proximity, file);
		newXS(strcpy(buf, "set_zone_flag"), XS__set_zone_flag, file);
		newXS(strcpy(buf, "setallskill"), XS__setallskill, file);
		newXS(strcpy(buf, "setanim"), XS__setanim, file);
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
		newXS(strcpy(buf, "tasksetselector"), XS__tasksetselector, file);
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
		newXS(strcpy(buf, "write"), XS__write, file);
		newXS(strcpy(buf, "ze"), XS__ze, file);
		newXS(strcpy(buf, "zone"), XS__zone, file);

		XSRETURN_YES;
}

#endif
#endif
