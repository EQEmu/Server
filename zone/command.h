/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef COMMAND_H
#define COMMAND_H

#include "../common/seperator.h"
#include "../common/EQStream.h"
#include "client.h"

#define	COMMAND_CHAR	'#'
#define CMDALIASES	5

typedef void (*CmdFuncPtr)(Client *,const Seperator *);
typedef struct {
	const char *command[CMDALIASES];
	int access;
	const char *desc;
	CmdFuncPtr function;
} CommandRecord;

extern int (*command_dispatch)(Client *,char const*);
extern int commandcount;

int command_init(void);
void command_deinit(void);
int command_add(const char *command_string, const char *desc, int access, CmdFuncPtr function);
int command_notavail(Client *c, const char *message);
int command_realdispatch(Client *c, char const *message);
void command_logcommand(Client *c, const char *message);
void command_resetaa(Client* c,const Seperator *sep);
void command_setstat(Client *c, const Seperator *sep);
void command_increasestat(Client *c, const Seperator *sep);
void command_help(Client *c, const Seperator *sep);
void command_setfaction(Client *c, const Seperator *sep);
void command_wearchange(Client *c, const Seperator *sep);
void command_setanimation(Client *c, const Seperator *sep);
void command_npcloot(Client *c, const Seperator *sep);
void command_log(Client *c, const Seperator *sep);
void command_gm(Client *c, const Seperator *sep);
void command_summon(Client *c, const Seperator *sep);
void command_zone(Client *c, const Seperator *sep);
void command_zone_instance(Client *c, const Seperator *sep);
void command_teleport(Client *c, const Seperator *sep);
void command_movecharacter(Client *c, const Seperator *sep);
void command_listnpcs(Client *c, const Seperator *sep);
void command_date(Client *c, const Seperator *sep);
void command_timezone(Client *c, const Seperator *sep);
void command_timeofday(Client *c, const Seperator *sep);
void command_invulnerable(Client *c, const Seperator *sep);
void command_gmhideme(Client *c, const Seperator *sep);
void command_emote(Client *c, const Seperator *sep);
void command_fieldofview(Client *c, const Seperator *sep);
void command_npcstats(Client *c, const Seperator *sep);
void command_npccast(Client *c, const Seperator *sep);
void command_changeclass(Client *c, const Seperator *sep);
void command_changerace(Client *c, const Seperator *sep);
void command_changegender(Client *c, const Seperator *sep);
void command_weather(Client *c, const Seperator *sep);
void command_gridassign(Client *c, const Seperator *sep);
void command_worldshutdown(Client *c, const Seperator *sep);
void command_sendzonespawns(Client *c, const Seperator *sep);
void command_dbspawn(Client *c, const Seperator *sep);
void command_copycharacter(Client *c, const Seperator *sep);
void command_shutdown(Client *c, const Seperator *sep);
void command_deleteaccount(Client *c, const Seperator *sep);
void command_setpassword(Client *c, const Seperator *sep);
void command_setinfo(Client *c, const Seperator *sep);
void command_grid(Client *c, const Seperator *sep);
void command_waypoint(Client *c, const Seperator *sep);
void command_iplookup(Client *c, const Seperator *sep);
void command_size(Client *c, const Seperator *sep);
void command_flymode(Client *c, const Seperator *sep);
void command_findspell(Client *c, const Seperator *sep);
void command_cast(Client *c, const Seperator *sep);
void command_setlanguage(Client *c, const Seperator *sep);
void command_setskill(Client *c, const Seperator *sep);
void command_setallskills(Client *c, const Seperator *sep);
void command_race(Client *c, const Seperator *sep);
void command_gender(Client *c, const Seperator *sep);
void command_makepet(Client *c, const Seperator *sep);
void command_level(Client *c, const Seperator *sep);
void command_spawn(Client *c, const Seperator *sep);
void command_texture(Client *c, const Seperator *sep);
void command_npctypespawn(Client *c, const Seperator *sep);
void command_heal(Client *c, const Seperator *sep);
void command_appearance(Client *c, const Seperator *sep);
void command_nukeitem(Client *c, const Seperator *sep);
void command_peek(Client *c, const Seperator *sep);
void command_findnpctype(Client *c, const Seperator *sep);
void command_findzone(Client *c, const Seperator *sep);
void command_viewnpctype(Client *c, const Seperator *sep);
void command_reloadquest(Client *c, const Seperator *sep);
void command_reloadworld(Client *c, const Seperator *sep);
void command_reloadzonepoints(Client *c, const Seperator *sep);
void command_zoneshutdown(Client *c, const Seperator *sep);
void command_zonebootup(Client *c, const Seperator *sep);
void command_kick(Client *c, const Seperator *sep);
void command_attack(Client *c, const Seperator *sep);
void command_lock(Client *c, const Seperator *sep);
void command_unlock(Client *c, const Seperator *sep);
void command_motd(Client *c, const Seperator *sep);
void command_equipitem(Client *c, const Seperator *sep);
void command_zonelock(Client *c, const Seperator *sep);
void command_corpse(Client *c, const Seperator *sep);
void command_fixmob(Client *c, const Seperator *sep);
void command_gmspeed(Client *c, const Seperator *sep);
void command_title(Client *c, const Seperator *sep);
void command_titlesuffix(Client *c, const Seperator *sep);
void command_spellinfo(Client *c, const Seperator *sep);
void command_lastname(Client *c, const Seperator *sep);
void command_memorizespell(Client *c, const Seperator *sep);
void command_showstats(Client *c, const Seperator *sep);
void command_mystats(Client *c, const Seperator *sep);
void command_myskills(Client *c, const Seperator *sep);
void command_depopulate(Client *c, const Seperator *sep);
void command_depopulatezone(Client *c, const Seperator *sep);
void command_repopulate(Client *c, const Seperator *sep);
void command_spawnstatus(Client *c, const Seperator *sep);
void command_nukebuffs(Client *c, const Seperator *sep);
void command_freeze(Client *c, const Seperator *sep);
void command_unfreeze(Client *c, const Seperator *sep);
void command_pvp(Client *c, const Seperator *sep);
void command_setexp(Client *c, const Seperator *sep);
void command_setpvppoints(Client *c, const Seperator *sep);
void command_name(Client *c, const Seperator *sep);
void command_temporaryname(Client *c, const Seperator *sep);
void command_npcspecialattack(Client *c, const Seperator *sep);
void command_kill(Client *c, const Seperator *sep);
void command_haste(Client *c, const Seperator *sep);
void command_damage(Client *c, const Seperator *sep);
void command_npcspawn(Client *c, const Seperator *sep);
void command_spawnfix(Client *c, const Seperator *sep);
void command_location(Client *c, const Seperator *sep);
void command_goto(Client *c, const Seperator *sep);
void command_iteminfo(Client *c, const Seperator *sep);
void command_uptime(Client *c, const Seperator *sep);
void command_flag(Client *c, const Seperator *sep);
void command_time(Client *c, const Seperator *sep);
void command_guild(Client *c, const Seperator *sep);
bool helper_guild_edit(Client *c, uint32 dbid, uint32 eqid, uint8 rank, const char* what, const char* value);
void command_manaburn(Client *c, const Seperator *sep);
void command_doanimation(Client *c, const Seperator *sep);
void command_randomfeatures(Client *c, const Seperator *sep);
void command_face(Client *c, const Seperator *sep);
void command_helmet(Client *c, const Seperator *sep);
void command_hair(Client *c, const Seperator *sep);
void command_haircolor(Client *c, const Seperator *sep);
void command_beard(Client *c, const Seperator *sep);
void command_beardcolor(Client *c, const Seperator *sep);
void command_tattoo(Client *c, const Seperator *sep);
void command_heritage(Client *c, const Seperator *sep);
void command_details(Client *c, const Seperator *sep);
void command_scribespells(Client *c, const Seperator *sep);
void command_unscribespells(Client *c, const Seperator *sep);
void command_waypointinfo(Client *c, const Seperator *sep);
void command_waypointadd(Client *c, const Seperator *sep);
void command_interrupt(Client *c, const Seperator *sep);
void command_summonitem(Client *c, const Seperator *sep);
void command_giveitem(Client *c, const Seperator *sep);
void command_givemoney(Client *c, const Seperator *sep);
void command_itemsearch(Client *c, const Seperator *sep);
void command_setaaexp(Client *c, const Seperator *sep);
void command_setaapoints(Client *c, const Seperator *sep);
void command_setcrystals(Client *c, const Seperator *sep);
void command_stun(Client *c, const Seperator *sep);
void command_ban(Client *c, const Seperator *sep);
void command_suspend(Client *c, const Seperator *sep);
void command_ipban(Client *c, const Seperator *sep);
void command_oocmute(Client *c, const Seperator *sep);
void command_revoke(Client *c, const Seperator *sep);
void command_checklos(Client *c, const Seperator *sep);
void command_set_adventure_points(Client *c, const Seperator *sep);
void command_npcemote(Client *c, const Seperator *sep);
void command_npcedit(Client *c, const Seperator *sep);
void command_timers(Client *c, const Seperator *sep);
void command_undye(Client *c, const Seperator *sep);
void command_undyeme(Client *c, const Seperator *sep);
void command_groupinfo(Client *c, const Seperator *sep);
void command_subscribe(Client *c, const Seperator *sep);
void command_unsubscribe(Client *c, const Seperator *sep);
void command_logsql(Client *c, const Seperator *sep);
void command_qglobal(Client *c, const Seperator *sep);
void command_path(Client *c, const Seperator *sep);
void command_aggro(Client *c, const Seperator *sep);
void command_hatelist(Client *c, const Seperator *sep);
void command_aggrozone(Client *c, const Seperator *sep);
void command_reloadstatic(Client *c, const Seperator *sep);
void command_flags(Client *c, const Seperator *sep);
void command_flagedit(Client *c, const Seperator *sep);
void command_managelogs(Client *c, const Seperator *sep);
void command_task(Client *c, const Seperator *sep);
void command_reloadtitles(Client *c, const Seperator *sep);
void command_altactivate(Client *c, const Seperator *sep);
void command_refundaa(Client *c, const Seperator *sep);
void command_traindisc(Client *c, const Seperator *sep);
void command_deletegraveyard(Client *c, const Seperator *sep);
void command_setgraveyard(Client *c, const Seperator *sep);
void command_getplayerburiedcorpsecount(Client *c, const Seperator *sep);
void command_summonburiedplayercorpse(Client *c, const Seperator *sep);
void command_unscribespell(Client *c, const Seperator *sep);
void command_scribespell(Client *c, const Seperator *sep);
void command_refreshgroup(Client *c, const Seperator *sep);
void command_advancednpcspawn(Client *c, const Seperator *sep);
void command_modifynpcstat(Client *c, const Seperator *sep);
void command_instance(Client *c, const Seperator *sep);
void command_setstartzone(Client *c, const Seperator *sep);
void command_object(Client* c, const Seperator *sep);
void command_raidloot(Client* c, const Seperator *sep);
void command_globalview(Client* c, const Seperator *sep);
void command_emoteview(Client* c, const Seperator *sep);
void command_reloademote(Client* c, const Seperator *sep);
void command_emotesearch(Client* c, const Seperator *sep);
void command_distance(Client *c, const Seperator *sep);
void command_max_all_skills(Client *c, const Seperator *sep);
void command_showbonusstats(Client *c, const Seperator *sep);
void command_reloadallrules(Client *c, const Seperator *sep);
void command_reloadworldrules(Client *c, const Seperator *sep);
void command_reloadlevelmods(Client *c, const Seperator *sep);
void command_camerashake(Client *c, const Seperator *sep);
void command_mysql(Client *c, const Seperator *sep);
void command_xtargets(Client *c, const Seperator *sep);
void command_augmentitem(Client *c, const Seperator *sep);
void command_enablerecipe(Client *c, const Seperator *sep);
void command_disablerecipe(Client *c, const Seperator *sep);
void command_showspellslist(Client *c, const Seperator *sep);
void command_npctype_cache(Client *c, const Seperator *sep);
void command_merchantopenshop(Client *c, const Seperator *sep);
void command_merchantcloseshop(Client *c, const Seperator *sep);

#endif

