#ifndef COMMAND_H
#define COMMAND_H

class Client;
class Seperator;

#include "../common/types.h"
#include <string>

#define    COMMAND_CHAR '#'

typedef void (*CmdFuncPtr)(Client *, const Seperator *);

typedef struct {
	uint8 admin;
	std::string description;
	CmdFuncPtr function; // null means perl function
} CommandRecord;

extern int (*command_dispatch)(Client *, std::string, bool);
extern int command_count; // Commands Loaded Count

// Command Utilities
int command_init(void);
void command_deinit(void);
int command_add(std::string command_name, std::string description, uint8 admin, CmdFuncPtr function);
int command_notavail(Client *c, std::string message, bool ignore_status);
int command_realdispatch(Client *c, std::string message, bool ignore_status);
uint8 GetCommandStatus(std::string command_name);
void ListModifyNPCStatMap(Client *c);
std::map<std::string, std::string> GetModifyNPCStatMap();
std::string GetModifyNPCStatDescription(std::string stat);
void SendFeatureSubCommands(Client *c);
void SendNPCEditSubCommands(Client *c);
void SendRuleSubCommands(Client *c);
void SendGuildSubCommands(Client *c);
void SendShowInventorySubCommands(Client *c);

// Commands
void command_acceptrules(Client *c, const Seperator *sep);
void command_advnpcspawn(Client *c, const Seperator *sep);
void command_aggrozone(Client *c, const Seperator *sep);
void command_ai(Client *c, const Seperator *sep);
void command_appearance(Client *c, const Seperator *sep);
void command_appearanceeffects(Client *c, const Seperator *sep);
void command_apply_shared_memory(Client *c, const Seperator *sep);
void command_attack(Client *c, const Seperator *sep);
void command_augmentitem(Client *c, const Seperator *sep);
void command_ban(Client *c, const Seperator *sep);
void command_bugs(Client *c, const Seperator *sep);
void command_camerashake(Client *c, const Seperator *sep);
void command_castspell(Client *c, const Seperator *sep);
void command_chat(Client *c, const Seperator *sep);
void command_copycharacter(Client *c, const Seperator *sep);
void command_corpse(Client *c, const Seperator *sep);
void command_corpsefix(Client *c, const Seperator *sep);
void command_countitem(Client *c, const Seperator *sep);
void command_damage(Client *c, const Seperator *sep);
void command_databuckets(Client *c, const Seperator *sep);
void command_dbspawn2(Client *c, const Seperator *sep);
void command_delacct(Client *c, const Seperator *sep);
void command_delpetition(Client *c, const Seperator *sep);
void command_depop(Client *c, const Seperator *sep);
void command_depopzone(Client *c, const Seperator *sep);
void command_devtools(Client *c, const Seperator *sep);
void command_disablerecipe(Client *c, const Seperator *sep);
void command_disarmtrap(Client *c, const Seperator *sep);
void command_door(Client *c, const Seperator *sep);
void command_doanim(Client *c, const Seperator *sep);
void command_dye(Client *c, const Seperator *sep);
void command_dz(Client *c, const Seperator *sep);
void command_dzkickplayers(Client *c, const Seperator *sep);
void command_editmassrespawn(Client *c, const Seperator *sep);
void command_emote(Client *c, const Seperator *sep);
void command_emptyinventory(Client *c, const Seperator *sep);
void command_enablerecipe(Client *c, const Seperator *sep);
void command_entityvariable(Client *c, const Seperator *sep);
void command_exptoggle(Client *c, const Seperator *sep);
void command_faction(Client *c, const Seperator *sep);
void command_faction_association(Client *c, const Seperator *sep);
void command_feature(Client *c, const Seperator *sep);
void command_find(Client *c, const Seperator *sep);
void command_fixmob(Client *c, const Seperator *sep);
void command_flagedit(Client *c, const Seperator *sep);
void command_gearup(Client *c, const Seperator *sep);
void command_giveitem(Client *c, const Seperator *sep);
void command_givemoney(Client *c, const Seperator *sep);
void command_gmzone(Client *c, const Seperator *sep);
void command_goto(Client *c, const Seperator *sep);
void command_grantaa(Client* c, const Seperator* sep);
void command_grid(Client *c, const Seperator *sep);
void command_guild(Client *c, const Seperator *sep);
void command_help(Client *c, const Seperator *sep);
void command_hotfix(Client *c, const Seperator *sep);
void command_hp(Client *c, const Seperator *sep);
void command_instance(Client *c, const Seperator *sep);
void command_interrogateinv(Client *c, const Seperator *sep);
void command_interrupt(Client *c, const Seperator *sep);
void command_invsnapshot(Client *c, const Seperator *sep);
void command_ipban(Client *c, const Seperator *sep);
void command_kick(Client *c, const Seperator *sep);
void command_killallnpcs(Client *c, const Seperator *sep);
void command_kill(Client *c, const Seperator *sep);
void command_level(Client *c, const Seperator *sep);
void command_list(Client *c, const Seperator *sep);
void command_lootsim(Client *c, const Seperator *sep);
void command_load_shared_memory(Client *c, const Seperator *sep);
void command_loc(Client *c, const Seperator *sep);
void command_logs(Client *c, const Seperator *sep);
void command_makepet(Client *c, const Seperator *sep);
void command_memspell(Client *c, const Seperator *sep);
void command_merchantshop(Client *c, const Seperator *sep);
void command_modifynpcstat(Client *c, const Seperator *sep);
void command_movechar(Client *c, const Seperator *sep);
void command_movement(Client *c, const Seperator *sep);
void command_myskills(Client *c, const Seperator *sep);
void command_mysql(Client *c, const Seperator *sep);
void command_mystats(Client *c, const Seperator *sep);
void command_npccast(Client *c, const Seperator *sep);
void command_npcedit(Client *c, const Seperator *sep);
void command_npceditmass(Client *c, const Seperator *sep);
void command_npcemote(Client *c, const Seperator *sep);
void command_npcloot(Client *c, const Seperator *sep);
void command_npcsay(Client *c, const Seperator *sep);
void command_npcshout(Client *c, const Seperator *sep);
void command_npcspawn(Client *c, const Seperator *sep);
void command_npctypespawn(Client *c, const Seperator *sep);
void command_nudge(Client *c, const Seperator *sep);
void command_nukebuffs(Client *c, const Seperator *sep);
void command_nukeitem(Client *c, const Seperator *sep);
void command_object(Client *c, const Seperator *sep);
void command_oocmute(Client *c, const Seperator *sep);
void command_path(Client *c, const Seperator *sep);
void command_peqzone(Client *c, const Seperator *sep);
void command_petitems(Client *c, const Seperator *sep);
void command_picklock(Client *c, const Seperator *sep);
void command_profanity(Client *c, const Seperator *sep);
void command_push(Client *c, const Seperator *sep);
void command_pvp(Client *c, const Seperator *sep);
void command_raidloot(Client* c, const Seperator* sep);
void command_randomfeatures(Client *c, const Seperator *sep);
void command_refreshgroup(Client *c, const Seperator *sep);
void command_reload(Client *c, const Seperator *sep);
void command_removeitem(Client *c, const Seperator *sep);
void command_repop(Client *c, const Seperator *sep);
void command_resetaa(Client *c, const Seperator *sep);
void command_resetaa_timer(Client *c, const Seperator *sep);
void command_resetdisc_timer(Client *c, const Seperator *sep);
void command_revoke(Client *c, const Seperator *sep);
void command_roambox(Client *c, const Seperator *sep);
void command_rules(Client *c, const Seperator *sep);
void command_save(Client *c, const Seperator *sep);
void command_scale(Client *c, const Seperator *sep);
void command_scribespell(Client *c, const Seperator *sep);
void command_scribespells(Client *c, const Seperator *sep);
void command_sendzonespawns(Client *c, const Seperator *sep);
void command_sensetrap(Client *c, const Seperator *sep);
void command_serverrules(Client *c, const Seperator *sep);
void command_set(Client *c, const Seperator *sep);
void command_show(Client *c, const Seperator *sep);
void command_shutdown(Client *c, const Seperator *sep);
void command_spawn(Client *c, const Seperator *sep);
void command_spawneditmass(Client *c, const Seperator *sep);
void command_spawnfix(Client *c, const Seperator *sep);
void command_stun(Client *c, const Seperator *sep);
void command_summon(Client *c, const Seperator *sep);
void command_summonburiedplayercorpse(Client *c, const Seperator *sep);
void command_summonitem(Client *c, const Seperator *sep);
void command_suspend(Client *c, const Seperator *sep);
void command_suspendmulti(Client *c, const Seperator *sep);
void command_takeplatinum(Client* c, const Seperator* sep);
void command_task(Client *c, const Seperator *sep);
void command_petname(Client *c, const Seperator *sep);
void command_traindisc(Client *c, const Seperator *sep);
void command_tune(Client *c, const Seperator *sep);
void command_undye(Client *c, const Seperator *sep);
void command_unmemspell(Client *c, const Seperator *sep);
void command_unmemspells(Client *c, const Seperator *sep);
void command_unscribespell(Client *c, const Seperator *sep);
void command_unscribespells(Client *c, const Seperator *sep);
void command_untraindisc(Client *c, const Seperator *sep);
void command_untraindiscs(Client *c, const Seperator *sep);
void command_wc(Client *c, const Seperator *sep);
void command_worldshutdown(Client *c, const Seperator *sep);
void command_wp(Client *c, const Seperator *sep);
void command_wpadd(Client *c, const Seperator *sep);
void command_worldwide(Client *c, const Seperator *sep);
void command_zone(Client *c, const Seperator *sep);
void command_zone_instance(Client *c, const Seperator *sep);
void command_zonebootup(Client *c, const Seperator *sep);
void command_zoneshutdown(Client *c, const Seperator *sep);
void command_zopp(Client *c, const Seperator *sep);
void command_zsafecoords(Client *c, const Seperator *sep);
void command_zsave(Client *c, const Seperator *sep);

#include "bot.h"
void command_bot(Client*c, const Seperator *sep);

#endif
