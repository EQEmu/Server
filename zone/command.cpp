/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.org)

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

/*

	To add a new command 3 things must be done:

	1.	At the bottom of command.h you must add a prototype for it.
	2.	Add the function in this file.
	3.	In the command_init function you must add a call to command_add
		for your function.
		
	Notes: If you want an alias for your command, add an entry to the
	`command_settings` table in your database. The access level you
	set with command_add is the default setting if the command isn't
	listed in the `command_settings` db table.

*/

#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <thread>

#ifdef _WINDOWS
#define strcasecmp _stricmp
#endif

#include "../common/global_define.h"
#include "../common/eq_packet.h"
#include "../common/features.h"
#include "../common/guilds.h"
#include "../common/patches/patches.h"
#include "../common/ptimer.h"
#include "../common/rulesys.h"
#include "../common/serverinfo.h"
#include "../common/string_util.h"
#include "../say_link.h"
#include "../common/eqemu_logsys.h"


#include "command.h"
#include "guild_mgr.h"
#include "map.h"
#include "pathing.h"
#include "qglobals.h"
#include "queryserv.h"
#include "quest_parser_collection.h"
#include "string_ids.h"
#include "titles.h"
#include "water_map.h"
#include "worldserver.h"

extern QueryServ* QServ;
extern WorldServer worldserver;
extern TaskManager *taskmanager;
void CatchSignal(int sig_num);


int commandcount;					// how many commands we have

// this is the pointer to the dispatch function, updated once
// init has been performed to point at the real function
int (*command_dispatch)(Client *,char const *)=command_notavail;


void command_bestz(Client *c, const Seperator *message);
void command_pf(Client *c, const Seperator *message);

std::map<std::string, CommandRecord *> commandlist;
std::map<std::string, std::string> commandaliases;

// All allocated CommandRecords get put in here so they get deleted on shutdown
LinkedList<CommandRecord *> cleanup_commandlist;

/*
 * command_notavail
 * This is the default dispatch function when commands aren't loaded.
 *
 * Parameters:
 *	not used
 *
 */
int command_notavail(Client *c, const char *message)
{
	c->Message(13, "Commands not available.");
	return -1;
}

/**************************************************************************
/* the rest below here could be in a dynamically loaded module eventually *
/*************************************************************************/

/*

Access Levels:

0		Normal
10	* Steward *
20	* Apprentice Guide *
50	* Guide *
80	* QuestTroupe *
81	* Senior Guide *
85	* GM-Tester *
90	* EQ Support *
95	* GM-Staff *
100	* GM-Admin *
150	* GM-Lead Admin *
160	* QuestMaster *
170	* GM-Areas *
180	* GM-Coder *
200	* GM-Mgmt *
250	* GM-Impossible *

*/

/*
 * command_init
 * initializes the command list, call at startup
 *
 * Parameters:
 *	none
 *
 * When adding a new command, only hard-code 'real' commands -
 * all command aliases are added later through a database call
 *
 */

int command_init(void)
{
	commandaliases.clear();
	
	if (
		command_add("acceptrules", "[acceptrules] - Accept the EQEmu Agreement", 0, command_acceptrules) ||
		command_add("advnpcspawn", "[maketype|makegroup|addgroupentry|addgroupspawn][removegroupspawn|movespawn|editgroupbox|cleargroupbox]", 150, command_advnpcspawn) ||
		command_add("aggro", "(range) [-v] - Display aggro information for all mobs 'range' distance from your target. -v is verbose faction info.", 80, command_aggro) ||
		command_add("aggrozone", "[aggro] - Aggro every mob in the zone with X aggro. Default is 0. Not recommend if you're not invulnerable.", 100, command_aggrozone) ||
		command_add("ai", "[factionid/spellslist/con/guard/roambox/stop/start] - Modify AI on NPC target", 100, command_ai) ||
		command_add("appearance", "[type] [value] - Send an appearance packet for you or your target", 150, command_appearance) ||
		command_add("apply_shared_memory", "[shared_memory_name] - Tells every zone and world to apply a specific shared memory segment by name.", 250, command_apply_shared_memory) ||
		command_add("attack", "[targetname] - Make your NPC target attack targetname", 150, command_attack) ||
		command_add("augmentitem",  "Force augments an item. Must have the augment item window open.",  250, command_augmentitem) ||
		command_add("ban", "[name] [reason]- Ban by character name", 150, command_ban) ||
		command_add("beard", "- Change the beard of your target", 80, command_beard) ||
		command_add("beardcolor", "- Change the beard color of your target", 80, command_beardcolor) ||
		command_add("bestz", "- Ask map for a good Z coord for your x,y coords.", 0, command_bestz) ||
		command_add("bind", "- Sets your targets bind spot to their current location", 200, command_bind) ||

#ifdef BOTS
		command_add("bot", "- Type \"#bot help\" or \"^help\" to the see the list of available commands for bots.", 0, command_bot) ||
#endif

		command_add("camerashake",  "Shakes the camera on everyone's screen globally.",  80, command_camerashake) ||
		command_add("castspell", "[spellid] - Cast a spell", 50, command_castspell) ||
		command_add("chat", "[channel num] [message] - Send a channel message to all zones", 200, command_chat) ||
		command_add("checklos", "- Check for line of sight to your target", 50, command_checklos) ||
		command_add("clearinvsnapshots", "[use rule] - Clear inventory snapshot history (true - elapsed entries, false - all entries)", 200, command_clearinvsnapshots) ||
		command_add("corpse", "- Manipulate corpses, use with no arguments for help", 50, command_corpse) ||
		command_add("corpsefix", "Attempts to bring corpses from underneath the ground within close proximity of the player", 0, command_corpsefix) ||
		command_add("crashtest", "- Crash the zoneserver", 255, command_crashtest) ||
		command_add("cvs", "- Summary of client versions currently online.", 200, command_cvs) ||
		command_add("damage", "[amount] - Damage your target", 100, command_damage) ||
		command_add("date", "[yyyy] [mm] [dd] [HH] [MM] - Set EQ time", 90, command_date) ||
		command_add("dbspawn2", "[spawngroup] [respawn] [variance] - Spawn an NPC from a predefined row in the spawn2 table", 100, command_dbspawn2) ||
		command_add("delacct", "[accountname] - Delete an account", 150, command_delacct) ||
		command_add("deletegraveyard", "[zone name] - Deletes the graveyard for the specified zone.",  200, command_deletegraveyard) ||
		command_add("delpetition", "[petition number] - Delete a petition", 20, command_delpetition) ||
		command_add("depop", "- Depop your NPC target", 50, command_depop) ||
		command_add("depopzone", "- Depop the zone", 100, command_depopzone) ||
		command_add("details", "- Change the details of your target (Drakkin Only)", 80, command_details) ||
		command_add("disablerecipe",  "[recipe_id] - Disables a recipe using the recipe id.",  80, command_disablerecipe) ||
		command_add("disarmtrap",  "Analog for ldon disarm trap for the newer clients since we still don't have it working.", 80, command_disarmtrap) ||
		command_add("distance", "- Reports the distance between you and your target.",  80, command_distance) ||
		command_add("doanim", "[animnum] [type] - Send an EmoteAnim for you or your target", 50, command_doanim) ||
		command_add("emote", "['name'/'world'/'zone'] [type] [message] - Send an emote message", 80, command_emote) ||
		command_add("emotesearch", "Searches NPC Emotes", 80, command_emotesearch) ||
		command_add("emoteview", "Lists all NPC Emotes", 80, command_emoteview) ||
		command_add("enablerecipe",  "[recipe_id] - Enables a recipe using the recipe id.",  80, command_enablerecipe) ||
		command_add("equipitem", "[slotid(0-21)] - Equip the item on your cursor into the specified slot", 50, command_equipitem) ||
		command_add("face", "- Change the face of your target", 80, command_face) ||
		command_add("findaliases", "[search term]- Searches for available command aliases, by alias or command", 0, command_findaliases) ||
		command_add("findnpctype", "[search criteria] - Search database NPC types", 100, command_findnpctype) ||
		command_add("findspell", "[searchstring] - Search for a spell", 50, command_findspell) ||
		command_add("findzone", "[search criteria] - Search database zones", 100, command_findzone) ||
		command_add("fixmob", "[race|gender|texture|helm|face|hair|haircolor|beard|beardcolor|heritage|tattoo|detail] [next|prev] - Manipulate appearance of your target", 80, command_fixmob) ||
		command_add("flag", "[status] [acctname] - Refresh your admin status, or set an account's admin status if arguments provided", 0, command_flag) ||
		command_add("flagedit", "- Edit zone flags on your target", 100, command_flagedit) ||
		command_add("flags", "- displays the flags of you or your target", 0, command_flags) ||
		command_add("flymode", "[0/1/2] - Set your or your player target's flymode to off/on/levitate", 50, command_flymode) ||
		command_add("fov", "- Check wether you're behind or in your target's field of view", 80, command_fov) ||
		command_add("freeze", "- Freeze your target", 80, command_freeze) ||
		command_add("gassign", "[id] - Assign targetted NPC to predefined wandering grid id", 100, command_gassign) ||
		command_add("gender", "[0/1/2] - Change your or your target's gender to male/female/neuter", 50, command_gender) ||
		command_add("getplayerburiedcorpsecount", "- Get the target's total number of buried player corpses.",  100, command_getplayerburiedcorpsecount) ||
		command_add("getvariable", "[varname] - Get the value of a variable from the database", 200, command_getvariable) ||
		command_add("ginfo", "- get group info on target.", 20, command_ginfo) ||
		command_add("giveitem", "[itemid] [charges] - Summon an item onto your target's cursor. Charges are optional.", 200, command_giveitem) ||
		command_add("givemoney", "[pp] [gp] [sp] [cp] - Gives specified amount of money to the target player.", 200, command_givemoney) ||
		command_add("globalview", "Lists all qglobals in cache if you were to do a quest with this target.", 80, command_globalview) ||
		command_add("gm", "- Turn player target's or your GM flag on or off", 80, command_gm) ||
		command_add("gmspeed", "[on/off] - Turn GM speed hack on/off for you or your player target", 100, command_gmspeed) ||
		command_add("goto", "[x] [y] [z] - Teleport to the provided coordinates or to your target", 10, command_goto) ||
		command_add("grid", "[add/delete] [grid_num] [wandertype] [pausetype] - Create/delete a wandering grid", 170, command_grid) ||
		command_add("guild", "- Guild manipulation commands. Use argument help for more info.", 10, command_guild) ||
		command_add("guildapprove", "[guildapproveid] - Approve a guild with specified ID (guild creator receives the id)", 0, command_guildapprove) ||
		command_add("guildcreate", "[guildname] - Creates an approval setup for guild name specified", 0, command_guildcreate) ||
		command_add("guildlist", "[guildapproveid] - Lists character names who have approved the guild specified by the approve id", 0, command_guildlist) ||
		command_add("hair", "- Change the hair style of your target", 80, command_hair) ||
		command_add("haircolor", "- Change the hair color of your target", 80, command_haircolor) ||
		command_add("haste", "[percentage] - Set your haste percentage", 100, command_haste) ||
		command_add("hatelist", " - Display hate list for target.",  80, command_hatelist) ||
		command_add("heal", "- Completely heal your target", 10, command_heal) ||
		command_add("helm", "- Change the helm of your target", 80, command_helm) ||
		command_add("help", "[search term] - List available commands and their description, specify partial command as argument to search", 0, command_help) ||
		command_add("heritage", "- Change the heritage of your target (Drakkin Only)", 80, command_heritage) ||
		command_add("heromodel",  "[hero model] [slot] - Full set of Hero's Forge Armor appearance. If slot is set, sends exact model just to slot.",  200, command_heromodel) ||
		command_add("hideme", "[on/off] - Hide yourself from spawn lists.", 80, command_hideme) ||
		command_add("hotfix", "[hotfix_name] - Reloads shared memory into a hotfix, equiv to load_shared_memory followed by apply_shared_memory", 250, command_hotfix) ||
		command_add("hp", "- Refresh your HP bar from the server.", 0, command_hp) ||
		command_add("incstat", "- Increases or Decreases a client's stats permanently.", 200, command_incstat) ||
		command_add("instance", "- Modify Instances", 200, command_instance) ||
		command_add("interrogateinv", "- use [help] argument for available options", 0, command_interrogateinv) ||
		command_add("interrupt", "[message id] [color] - Interrupt your casting. Arguments are optional.", 50, command_interrupt) ||
		command_add("invsnapshot", "- Takes an inventory snapshot of your current target", 80, command_invsnapshot) ||
		command_add("invul", "[on/off] - Turn player target's or your invulnerable flag on or off", 80, command_invul) ||
		command_add("ipban", "[IP address] - Ban IP by character name", 200, command_ipban) ||
		command_add("iplookup", "[charname] - Look up IP address of charname", 200, command_iplookup) ||
		command_add("iteminfo", "- Get information about the item on your cursor", 10, command_iteminfo) ||
		command_add("itemsearch", "[search criteria] - Search for an item", 10, command_itemsearch) ||
		command_add("kick", "[charname] - Disconnect charname", 150, command_kick) ||
		command_add("kill", "- Kill your target", 100, command_kill) ||
		command_add("lastname", "[new lastname] - Set your or your player target's lastname", 50, command_lastname) ||
		command_add("level", "[level] - Set your or your target's level", 10, command_level) ||
		command_add("listnpcs", "[name/range] - Search NPCs", 20, command_listnpcs) ||
		command_add("listpetition", "- List petitions", 50, command_listpetition) ||
		command_add("load_shared_memory", "[shared_memory_name] - Reloads shared memory and uses the input as output", 250, command_load_shared_memory) ||
		command_add("loc", "- Print out your or your target's current location and heading", 0, command_loc) ||
		command_add("lock", "- Lock the worldserver", 150, command_lock) ||
		command_add("logs",  "Manage anything to do with logs",  250, command_logs) ||
		command_add("logtest",  "Performs log performance testing.",  250, command_logtest) ||
		command_add("makepet", "[level] [class] [race] [texture] - Make a pet", 50, command_makepet) ||
		command_add("mana", "- Fill your or your target's mana", 50, command_mana) ||
		command_add("maxskills", "Maxes skills for you.", 200, command_max_all_skills) ||
		command_add("memspell", "[slotid] [spellid] - Memorize spellid in the specified slot", 50, command_memspell) ||
		command_add("merchant_close_shop",  "Closes a merchant shop",  100, command_merchantcloseshop) ||
		command_add("merchant_open_shop",  "Opens a merchants shop",  100, command_merchantopenshop) ||
		command_add("modifynpcstat", "- Modifys a NPC's stats", 150, command_modifynpcstat) ||
		command_add("motd", "[new motd] - Set message of the day", 150, command_motd) ||
		command_add("movechar", "[charname] [zonename] - Move charname to zonename", 50, command_movechar) ||
		command_add("myskills", "- Show details about your current skill levels", 0, command_myskills) ||
		command_add("mysqltest", "Akkadius MySQL Bench Test", 250, command_mysqltest) ||
		command_add("mysql", "Mysql CLI, see 'help' for options.", 250, command_mysql) ||
		command_add("mystats", "- Show details about you or your pet", 50, command_mystats) ||
		command_add("name", "[newname] - Rename your player target", 150, command_name) ||
		command_add("netstats", "- Gets the network stats for a stream.", 200, command_netstats) ||
		command_add("npccast", "[targetname/entityid] [spellid] - Causes NPC target to cast spellid on targetname/entityid", 80, command_npccast) ||
		command_add("npcedit", "[column] [value] - Mega NPC editing command", 100, command_npcedit) ||
		command_add("npcemote", "[message] - Make your NPC target emote a message.", 150, command_npcemote) ||
		command_add("npcloot", "[show/money/add/remove] [itemid/all/money: pp gp sp cp] - Manipulate the loot an NPC is carrying", 80, command_npcloot) ||
		command_add("npcsay", "[message] - Make your NPC target say a message.", 150, command_npcsay) ||
		command_add("npcshout", "[message] - Make your NPC target shout a message.", 150, command_npcshout) ||
		command_add("npcspawn", "[create/add/update/remove/delete] - Manipulate spawn DB", 170, command_npcspawn) ||
		command_add("npcspecialattk", "[flagchar] [perm] - Set NPC special attack flags. Flags are E(nrage) F(lurry) R(ampage) S(ummon).", 80, command_npcspecialattk) ||
		command_add("npcstats", "- Show stats about target NPC", 80, command_npcstats) ||
		command_add("npctype_cache",  "[id] or all - Clears the npc type cache for either the id or all npcs.",  250, command_npctype_cache) ||
		command_add("npctypespawn", "[npctypeid] [factionid] - Spawn an NPC from the db", 10, command_npctypespawn) ||
		command_add("nukebuffs", "- Strip all buffs on you or your target", 50, command_nukebuffs) ||
		command_add("nukeitem", "[itemid] - Remove itemid from your player target's inventory", 150, command_nukeitem) ||
		command_add("object", "List|Add|Edit|Move|Rotate|Copy|Save|Undo|Delete - Manipulate static and tradeskill objects within the zone", 100, command_object) ||
		command_add("oocmute", "[1/0] - Mutes OOC chat", 200, command_oocmute) ||
		command_add("opcode", "- opcode management", 250, command_opcode) ||

#ifdef PACKET_PROFILER
		command_add("packetprofile", "- Dump packet profile for target or self.", 250, command_packetprofile) ||
#endif

		command_add("path", "- view and edit pathing", 200, command_path) ||
		command_add("peekinv", "[worn/inv/cursor/trib/bank/trade/world/all] - Print out contents of your player target's inventory", 100, command_peekinv) ||
		command_add("peqzone", "[zonename] - Go to specified zone, if you have > 75% health", 0, command_peqzone) ||
		command_add("permaclass", "[classnum] - Change your or your player target's class (target is disconnected)", 80, command_permaclass) ||
		command_add("permagender", "[gendernum] - Change your or your player target's gender (zone to take effect)", 80, command_permagender) ||
		command_add("permarace", "[racenum] - Change your or your player target's race (zone to take effect)", 80, command_permarace) ||
		command_add("petitioninfo", "[petition number] - Get info about a petition", 20, command_petitioninfo) ||
		command_add("pf", "- Display additional mob coordinate and wandering data", 0, command_pf) ||
		command_add("picklock",  "Analog for ldon pick lock for the newer clients since we still don't have it working.",  0, command_picklock) ||

#ifdef EQPROFILE
		command_add("profiledump", "- Dump profiling info to logs", 250, command_profiledump) ||
		command_add("profilereset", "- Reset profiling info", 250, command_profilereset) ||
#endif

		command_add("pvp", "[on/off] - Set your or your player target's PVP status", 100, command_pvp) ||
		command_add("qglobal", "[on/off/view] - Toggles qglobal functionality on an NPC", 100, command_qglobal) ||
		command_add("questerrors", "Shows quest errors.", 100, command_questerrors) ||
		command_add("race", "[racenum] - Change your or your target's race. Use racenum 0 to return to normal", 50, command_race) ||
		command_add("raidloot", "LEADER|GROUPLEADER|SELECTED|ALL - Sets your raid loot settings if you have permission to do so.", 0, command_raidloot) ||
		command_add("randomfeatures", "- Temporarily randomizes the Facial Features of your target", 80, command_randomfeatures) ||
		command_add("refreshgroup", "- Refreshes Group.",  0, command_refreshgroup) ||
		command_add("reloadaa", "Reloads AA data", 200, command_reloadaa) ||
		command_add("reloadallrules", "Executes a reload of all rules.", 80, command_reloadallrules) ||
		command_add("reloademote", "Reloads NPC Emotes", 80, command_reloademote) ||
		command_add("reloadlevelmods", nullptr, 255, command_reloadlevelmods) ||
		command_add("reloadperlexportsettings", nullptr, 255, command_reloadperlexportsettings) ||
		command_add("reloadqst", " - Clear quest cache (any argument causes it to also stop all timers)", 150, command_reloadqst) ||
		command_add("reloadrulesworld", "Executes a reload of all rules in world specifically.", 80, command_reloadworldrules) ||
		command_add("reloadstatic", "- Reload Static Zone Data", 150, command_reloadstatic) ||
		command_add("reloadtraps", "- Repops all traps in the current zone.", 80, command_reloadtraps) ||
		command_add("reloadtitles", "- Reload player titles from the database",  150, command_reloadtitles) ||
		command_add("reloadworld", "[0|1] - Clear quest cache (0 - no repop, 1 - repop)", 255, command_reloadworld) ||
		command_add("reloadzps", "- Reload zone points from database", 150, command_reloadzps) ||
		command_add("repop", "[delay] - Repop the zone with optional delay", 100, command_repop) ||
		command_add("repopclose", "[distance in units] Repops only NPC's nearby for fast development purposes", 100, command_repopclose) ||
		command_add("resetaa", "- Resets a Player's AA in their profile and refunds spent AA's to unspent, may disconnect player.", 200, command_resetaa) ||
		command_add("resetaa_timer", "Command to reset AA cooldown timers.", 200, command_resetaa_timer) ||
		command_add("revoke", "[charname] [1/0] - Makes charname unable to talk on OOC", 200, command_revoke) ||
		command_add("rules", "(subcommand) - Manage server rules", 250, command_rules) ||
		command_add("save", "- Force your player or player corpse target to be saved to the database", 50, command_save) ||
		command_add("scribespell", "[spellid] - Scribe specified spell in your target's spell book.", 180, command_scribespell) ||
		command_add("scribespells", "[max level] [min level] - Scribe all spells for you or your player target that are usable by them, up to level specified. (may freeze client for a few seconds)", 150, command_scribespells) ||
		command_add("sendzonespawns", "- Refresh spawn list for all clients in zone", 150, command_sendzonespawns) ||
		command_add("sensetrap",  "Analog for ldon sense trap for the newer clients since we still don't have it working.",  0, command_sensetrap) ||
		command_add("serverinfo", "- Get OS info about server host", 200, command_serverinfo) ||
		command_add("serverrules", "- Read this server's rules", 0, command_serverrules) ||
		command_add("setaapts", "[value] - Set your or your player target's available AA points", 100, command_setaapts) ||
		command_add("setaaxp", "[value] - Set your or your player target's AA experience", 100, command_setaaxp) ||
		command_add("setadventurepoints", "- Set your or your player target's available adventure points", 150, command_set_adventure_points) ||
		command_add("setanim", "[animnum] - Set target's appearance to animnum", 200, command_setanim) ||
		command_add("setcrystals", "[value] - Set your or your player target's available radiant or ebon crystals", 100, command_setcrystals) ||
		command_add("setfaction", "[faction number] - Sets targeted NPC's faction in the database", 170, command_setfaction) ||
		command_add("setgraveyard", "[zone name] - Creates a graveyard for the specified zone based on your target's LOC.",  200, command_setgraveyard) ||
		command_add("setlanguage", "[language ID] [value] - Set your target's language skillnum to value", 50, command_setlanguage) ||
		command_add("setlsinfo", "[email] [password] - Set login server email address and password (if supported by login server)", 10, command_setlsinfo) ||
		command_add("setpass", "[accountname] [password] - Set local password for accountname", 150, command_setpass) ||
		command_add("setpvppoints", "[value] - Set your or your player target's PVP points", 100, command_setpvppoints) ||
		command_add("setskill", "[skillnum] [value] - Set your target's skill skillnum to value", 50, command_setskill) ||
		command_add("setskillall", "[value] - Set all of your target's skills to value", 50, command_setskillall) ||
		command_add("setstartzone", "[zoneid] - Set target's starting zone. Set to zero to allow the player to use /setstartcity", 80, command_setstartzone) ||
		command_add("setstat", "- Sets the stats to a specific value.", 255, command_setstat) ||
		command_add("setxp", "[value] - Set your or your player target's experience", 100, command_setxp) ||
		command_add("showbonusstats", "[item|spell|all] Shows bonus stats for target from items or spells. Shows both by default.", 50, command_showbonusstats) ||
		command_add("showbuffs", "- List buffs active on your target or you if no target", 50, command_showbuffs) ||
		command_add("shownumhits",  "Shows buffs numhits for yourself.",  0, command_shownumhits) ||
		command_add("showskills", "- Show the values of your or your player target's skills", 50, command_showskills) ||
		command_add("showspellslist", "Shows spell list of targeted NPC", 100, command_showspellslist) ||
		command_add("showstats", "- Show details about you or your target", 50, command_showstats) ||
		command_add("shutdown", "- Shut this zone process down", 150, command_shutdown) ||
		command_add("size", "[size] - Change size of you or your target", 50, command_size) ||
		command_add("spawn", "[name] [race] [level] [material] [hp] [gender] [class] [priweapon] [secweapon] [merchantid] - Spawn an NPC", 10, command_spawn) ||
		command_add("spawnfix", "- Find targeted NPC in database based on its X/Y/heading and update the database to make it spawn at your current location/heading.", 170, command_spawnfix) ||
		command_add("spawnstatus", "- Show respawn timer status", 100, command_spawnstatus) ||
		command_add("spellinfo", "[spellid] - Get detailed info about a spell", 10, command_spellinfo) ||
		command_add("spoff", "- Sends OP_ManaChange", 80, command_spoff) ||
		command_add("spon", "- Sends OP_MemorizeSpell", 80, command_spon) ||
		command_add("stun", "[duration] - Stuns you or your target for duration", 100, command_stun) ||
		command_add("summon", "[charname] - Summons your player/npc/corpse target, or charname if specified", 80, command_summon) ||
		command_add("summonburiedplayercorpse", "- Summons the target's oldest buried corpse, if any exist.",  100, command_summonburiedplayercorpse) ||
		command_add("summonitem", "[itemid] [charges] - Summon an item onto your cursor. Charges are optional.", 200, command_summonitem) ||
		command_add("suspend", "[name] [days] [reason] - Suspend by character name and for specificed number of days", 150, command_suspend) ||
		command_add("task", "(subcommand) - Task system commands",  150, command_task) ||
		command_add("tattoo", "- Change the tattoo of your target (Drakkin Only)", 80, command_tattoo) ||
		command_add("tempname", "[newname] - Temporarily renames your target. Leave name blank to restore the original name.", 100, command_tempname) ||
		command_add("petname", "[newname] - Temporarily renames your pet. Leave name blank to restore the original name.", 100, command_petname) ||
		command_add("texture", "[texture] [helmtexture] - Change your or your target's appearance, use 255 to show equipment", 10, command_texture) ||
		command_add("time", "[HH] [MM] - Set EQ time", 90, command_time) ||
		command_add("timers", "- Display persistent timers for target", 200, command_timers) ||
		command_add("timezone", "[HH] [MM] - Set timezone. Minutes are optional", 90, command_timezone) ||
		command_add("title", "[text] [1 = create title table row] - Set your or your player target's title", 50, command_title) ||
		command_add("titlesuffix", "[text] [1 = create title table row] - Set your or your player target's title suffix", 50, command_titlesuffix) ||
		command_add("traindisc", "[level] - Trains all the disciplines usable by the target, up to level specified. (may freeze client for a few seconds)", 150, command_traindisc) ||
		command_add("trapinfo", "- Gets infomation about the traps currently spawned in the zone.", 81, command_trapinfo) ||
		command_add("tune",  "Calculate ideal statical values related to combat.",  100, command_tune) ||
		command_add("undyeme", "- Remove dye from all of your armor slots", 0, command_undyeme) ||
		command_add("unfreeze", "- Unfreeze your target", 80, command_unfreeze) ||
		command_add("unlock", "- Unlock the worldserver", 150, command_unlock) ||
		command_add("unscribespell", "[spellid] - Unscribe specified spell from your target's spell book.", 180, command_unscribespell) ||
		command_add("unscribespells", "- Clear out your or your player target's spell book.", 180, command_unscribespells) ||
		command_add("untraindisc", "[spellid] - Untrain specified discipline from your target.", 180, command_untraindisc) ||
		command_add("untraindiscs", "- Untrains all disciplines from your target.", 180, command_untraindiscs) ||
		command_add("uptime", "[zone server id] - Get uptime of worldserver, or zone server if argument provided", 10, command_uptime) ||
		command_add("version", "- Display current version of EQEmu server", 0, command_version) ||
		command_add("viewnpctype", "[npctype id] - Show info about an npctype", 100, command_viewnpctype) ||
		command_add("viewpetition", "[petition number] - View a petition", 20, command_viewpetition) ||
		command_add("wc", "[wear slot] [material] - Sends an OP_WearChange for your target", 200, command_wc) ||
		command_add("weather", "[0/1/2/3] (Off/Rain/Snow/Manual) - Change the weather", 80, command_weather) ||
		command_add("worldshutdown", "- Shut down world and all zones", 200, command_worldshutdown) ||
		command_add("wp", "[add/delete] [grid_num] [pause] [wp_num] [-h] - Add/delete a waypoint to/from a wandering grid", 170, command_wp) ||
		command_add("wpadd", "[pause] [-h] - Add your current location as a waypoint to your NPC target's AI path", 170, command_wpadd) ||
		command_add("wpinfo", "- Show waypoint info about your NPC target", 170, command_wpinfo) ||
		command_add("xtargets",  "Show your targets Extended Targets and optionally set how many xtargets they can have.",  250, command_xtargets) ||
		command_add("zclip", "[min] [max] - modifies and resends zhdr packet", 80, command_zclip) ||
		command_add("zcolor", "[red] [green] [blue] - Change sky color", 80, command_zcolor) ||
		command_add("zheader", "[zonename] - Load zheader for zonename from the database", 80, command_zheader) ||
		command_add("zone", "[zonename] [x] [y] [z] - Go to specified zone (coords optional)", 50, command_zone) ||
		command_add("zonebootup", "[ZoneServerID] [shortname] - Make a zone server boot a specific zone", 150, command_zonebootup) ||
		command_add("zoneinstance", "[instanceid] [x] [y] [z] - Go to specified instance zone (coords optional)", 50, command_zone_instance) ||
		command_add("zonelock", "[list/lock/unlock] - Set/query lock flag for zoneservers", 100, command_zonelock) ||
		command_add("zoneshutdown", "[shortname] - Shut down a zone server", 150, command_zoneshutdown) ||
		command_add("zonespawn", "- Not implemented", 250, command_zonespawn) ||
		command_add("zonestatus", "- Show connected zoneservers, synonymous with /servers", 150, command_zonestatus) ||
		command_add("zopp",  "Troubleshooting command - Sends a fake item packet to you. No server reference is created.",  250, command_zopp) ||
		command_add("zsafecoords", "[x] [y] [z] - Set safe coords", 80, command_zsafecoords) ||
		command_add("zsave", " - Saves zheader to the database", 80, command_zsave) ||
		command_add("zsky", "[skytype] - Change zone sky type", 80, command_zsky) ||
		command_add("zstats", "- Show info about zone header", 80, command_zstats) ||
		command_add("zunderworld", "[zcoord] - Sets the underworld using zcoord", 80, command_zunderworld) ||
		command_add("zuwcoords", "[z coord] - Set underworld coord", 80, command_zuwcoords)
	) {
		command_deinit();
		return -1;
	}
	
	std::map<std::string, std::pair<uint8, std::vector<std::string>>> command_settings;
	database.GetCommandSettings(command_settings);

	std::map<std::string, CommandRecord *> working_cl = commandlist;
	for (auto iter_cl = working_cl.begin(); iter_cl != working_cl.end(); ++iter_cl) {
		auto iter_cs = command_settings.find(iter_cl->first);
		if (iter_cs == command_settings.end()) {
			if (iter_cl->second->access == 0)
				Log(Logs::General, Logs::Commands, "command_init(): Warning: Command '%s' defaulting to access level 0!", iter_cl->first.c_str());
			continue;
		}

		iter_cl->second->access = iter_cs->second.first;
		Log(Logs::General, Logs::Commands, "command_init(): - Command '%s' set to access level %d.", iter_cl->first.c_str(), iter_cs->second.first);
		if (iter_cs->second.second.empty())
			continue;

		for (auto iter_aka = iter_cs->second.second.begin(); iter_aka != iter_cs->second.second.end();
		     ++iter_aka) {
			if (iter_aka->empty())
				continue;
			if (commandlist.find(*iter_aka) != commandlist.end()) {
				Log(Logs::General, Logs::Commands, "command_init(): Warning: Alias '%s' already exists as a command - skipping!", iter_aka->c_str());
				continue;
			}

			commandlist[*iter_aka] = iter_cl->second;
			commandaliases[*iter_aka] = iter_cl->first;

			Log(Logs::General, Logs::Commands, "command_init(): - Alias '%s' added to command '%s'.", iter_aka->c_str(), commandaliases[*iter_aka].c_str());
		}
	}

	command_dispatch = command_realdispatch;

	return commandcount;
}

/*
 * command_deinit
 * clears the command list, freeing resources
 *
 * Parameters:
 *	none
 *
 */
void command_deinit(void)
{
	commandlist.clear();
	commandaliases.clear();

	command_dispatch = command_notavail;
	commandcount = 0;
}

/*
 * command_add
 * adds a command to the command list; used by command_init
 *
 * Parameters:
 *	command_name	- the command ex: "spawn"
 *	desc			- text description of command for #help
 *	access			- default access level required to use command
 *	function		- pointer to function that handles command
 *
 */
int command_add(std::string command_name, const char *desc, int access, CmdFuncPtr function)
{
	if (command_name.empty()) {
		Log(Logs::General, Logs::Error, "command_add() - Command added with empty name string - check command.cpp.");
		return -1;
	}
	if (function == nullptr) {
		Log(Logs::General, Logs::Error, "command_add() - Command '%s' added without a valid function pointer - check command.cpp.", command_name.c_str());
		return -1;
	}
	if (commandlist.count(command_name) != 0) {
		Log(Logs::General, Logs::Error, "command_add() - Command '%s' is a duplicate command name - check command.cpp.", command_name.c_str());
		return -1;
	}
	for (auto iter = commandlist.begin(); iter != commandlist.end(); ++iter) {
		if (iter->second->function != function)
			continue;
		Log(Logs::General, Logs::Error, "command_add() - Command '%s' equates to an alias of '%s' - check command.cpp.", command_name.c_str(), iter->first.c_str());
		return -1;
	}

	auto c = new CommandRecord;
	c->access = access;
	c->desc = desc;
	c->function = function;

	commandlist[command_name] = c;
	commandaliases[command_name] = command_name;
	cleanup_commandlist.Append(c);
	commandcount++;

	return 0;
}

/*
 *
 * command_realdispatch
 * Calls the correct function to process the client's command string.
 * Called from Client::ChannelMessageReceived if message starts with
 * command character (#).
 *
 * Parameters:
 *	c			- pointer to the calling client object
 *	message		- what the client typed
 *
 */
int command_realdispatch(Client *c, const char *message)
{
	Seperator sep(message, ' ', 10, 100, true); // "three word argument" should be considered 1 arg

	command_logcommand(c, message);

	std::string cstr(sep.arg[0]+1);

	if(commandlist.count(cstr) != 1) {
		return(-2);
	}

	CommandRecord *cur = commandlist[cstr];
	if(c->Admin() < cur->access){
		c->Message(13,"Your access level is not high enough to use this command.");
		return(-1);
	}

	/* QS: Player_Log_Issued_Commands */
	if (RuleB(QueryServ, PlayerLogIssuedCommandes)){
		std::string event_desc = StringFormat("Issued command :: '%s' in zoneid:%i instid:%i",  message, c->GetZoneID(), c->GetInstanceID());
		QServ->PlayerLogEvent(Player_Log_Issued_Commands, c->CharacterID(), event_desc);
	}

	if(cur->access >= COMMANDS_LOGGING_MIN_STATUS) {
		Log(Logs::General, Logs::Commands, "%s (%s) used command: %s (target=%s)",  c->GetName(), c->AccountName(), message, c->GetTarget()?c->GetTarget()->GetName():"NONE");
	}

	if(cur->function == nullptr) {
		Log(Logs::General, Logs::Error, "Command '%s' has a null function\n",  cstr.c_str());
		return(-1);
	} else {
		//dispatch C++ command
		cur->function(c, &sep);	// dispatch command
	}
	return 0;

}

void command_logcommand(Client *c, const char *message)
{
	int admin=c->Admin();

	bool continueevents=false;
	switch (zone->loglevelvar){ //catch failsafe
		case 9: { // log only LeadGM
			if ((admin>= 150) && (admin <200))
				continueevents=true;
			break;
		}
		case 8: { // log only GM
			if ((admin>= 100) && (admin <150))
				continueevents=true;
			break;
		}
		case 1: {
			if ((admin>= 200))
				continueevents=true;
			break;
		}
		case 2: {
			if ((admin>= 150))
				continueevents=true;
			break;
		}
		case 3: {
			if ((admin>= 100))
				continueevents=true;
			break;
		}
		case 4: {
			if ((admin>= 80))
				continueevents=true;
			break;
		}
		case 5: {
			if ((admin>= 20))
				continueevents=true;
			break;
		}
		case 6: {
			if ((admin>= 10))
				continueevents=true;
			break;
		}
		case 7: {
				continueevents=true;
				break;
		}
	}

	if (continueevents)
		database.logevents(
			c->AccountName(),
			c->AccountID(),
			admin,c->GetName(),
			c->GetTarget()?c->GetTarget()->GetName():"None",
			"Command",
			message,
			1
		);
}


/*
 * commands go below here
 */
void command_setstat(Client* c, const Seperator* sep){
	if(sep->arg[1][0] && sep->arg[2][0] && c->GetTarget()!=0 && c->GetTarget()->IsClient()){
		c->GetTarget()->CastToClient()->SetStats(atoi(sep->arg[1]),atoi(sep->arg[2]));
	}
	else{
		c->Message(0,"This command is used to permanently increase or decrease a players stats.");
		c->Message(0,"Usage: #setstat {type} {value the stat should be}");
		c->Message(0,"Types: Str: 0, Sta: 1, Agi: 2, Dex: 3, Int: 4, Wis: 5, Cha: 6");
	}
}

void command_incstat(Client* c, const Seperator* sep){
	if(sep->arg[1][0] && sep->arg[2][0] && c->GetTarget()!=0 && c->GetTarget()->IsClient()){
		c->GetTarget()->CastToClient()->IncStats(atoi(sep->arg[1]),atoi(sep->arg[2]));
	}
	else{
		c->Message(0,"This command is used to permanently increase or decrease a players stats.");
		c->Message(0,"Usage: #setstat {type} {value by which to increase or decrease}");
		c->Message(0,"Note: The value is in increments of 2, so a value of 3 will actually increase the stat by 6");
		c->Message(0,"Types: Str: 0, Sta: 1, Agi: 2, Dex: 3, Int: 4, Wis: 5, Cha: 6");
	}
}

void command_resetaa(Client* c,const Seperator *sep) {
	if(c->GetTarget() && c->GetTarget()->IsClient()){
		c->GetTarget()->CastToClient()->ResetAA();
		c->Message(13,"Successfully reset %s's AAs", c->GetTarget()->GetName());
	}
	else
		c->Message(0,"Usage: Target a client and use #resetaa to reset the AA data in their Profile.");
}

void command_help(Client *c, const Seperator *sep)
{
	int commands_shown=0;

	c->Message(0, "Available EQEMu commands:");

	std::map<std::string, CommandRecord *>::iterator cur,end;
	cur = commandlist.begin();
	end = commandlist.end();

	for(; cur != end; ++cur) {
		if(sep->arg[1][0]) {
			if(cur->first.find(sep->arg[1]) == std::string::npos) {
				continue;
			}
		}

		if(c->Admin() < cur->second->access)
			continue;
		commands_shown++;
		c->Message(0, "	%c%s %s",  COMMAND_CHAR, cur->first.c_str(), cur->second->desc == nullptr?"":cur->second->desc);
	}
	c->Message(0, "%d command%s listed.",  commands_shown, commands_shown!=1?"s":"");

}

void command_version(Client *c, const Seperator *sep)
{
	c->Message(0, "Current version information.");
	c->Message(0, "	%s",  CURRENT_VERSION);
	c->Message(0, "	Compiled on: %s at %s",  COMPILE_DATE, COMPILE_TIME);
	c->Message(0, "	Last modified on: %s",  LAST_MODIFIED);
}

void command_setfaction(Client *c, const Seperator *sep)
{
	if((sep->arg[1][0] == 0 || strcasecmp(sep->arg[1],"*")==0) || ((c->GetTarget()==0) || (c->GetTarget()->IsClient()))) {
		c->Message(0, "Usage: #setfaction [faction number]");
		return;
    }

    auto npcTypeID = c->GetTarget()->CastToNPC()->GetNPCTypeID();
    c->Message(15,"Setting NPC %u to faction %i",  npcTypeID, atoi(sep->argplus[1]));

    std::string query = StringFormat("UPDATE npc_types SET npc_faction_id = %i WHERE id = %i",
                                    atoi(sep->argplus[1]), npcTypeID);
    database.QueryDatabase(query);
}

void command_serversidename(Client *c, const Seperator *sep)
{
	if(c->GetTarget())
		c->Message(0, c->GetTarget()->GetName());
	else
		c->Message(0, "Error: no target");
}

void command_wc(Client *c, const Seperator *sep)
{
	if(sep->argnum < 2)
	{
		c->Message(0, "Usage: #wc [wear slot] [material] [ [hero_forge_model] [elite_material] [unknown06] [unknown18] ]");
	}
	else if(c->GetTarget() == nullptr) {
		c->Message(13, "You must have a target to do a wear change.");
	}
	else
	{
		uint32 hero_forge_model = 0;
		uint32 wearslot = atoi(sep->arg[1]);

		// Hero Forge
		if (sep->argnum > 2)
		{
			hero_forge_model = atoi(sep->arg[3]);

			if (hero_forge_model != 0 && hero_forge_model < 1000) {
				// Shorthand Hero Forge ID. Otherwise use the value the user entered.
				hero_forge_model = (hero_forge_model * 100) + wearslot;
			}
		}
		/*
		// Leaving here to add color option to the #wc command eventually
		uint32 Color;
		if (c->GetTarget()->IsClient())
			Color = c->GetTarget()->GetEquipmentColor(atoi(sep->arg[1]));
		else
			Color = c->GetTarget()->GetArmorTint(atoi(sep->arg[1]));
		*/
		c->GetTarget()->SendTextureWC(wearslot, atoi(sep->arg[2]), hero_forge_model, atoi(sep->arg[4]), atoi(sep->arg[5]), atoi(sep->arg[6]));
	}
}

void command_heromodel(Client *c, const Seperator *sep)
{
	if (sep->argnum < 1)
	{
		c->Message(0, "Usage: #heromodel [hero forge model] [ [slot] ] (example: #heromodel 63)");
	}
	else if (c->GetTarget() == nullptr)
	{
		c->Message(13, "You must have a target to do a wear change for Hero's Forge Models.");
	}
	else
	{
		uint32 hero_forge_model = atoi(sep->arg[1]);

		if (sep->argnum > 1)
		{
			uint8 wearslot = (uint8)atoi(sep->arg[2]);
			c->GetTarget()->SendTextureWC(wearslot, 0, hero_forge_model, 0, 0, 0);
		}
		else
		{
			if (hero_forge_model > 0)
			{
				// Conversion to simplify the command arguments
				// Hero's Forge model is actually model * 1000 + texture * 100 + wearslot
				// Hero's Forge Model slot 7 is actually for Robes, but it still needs to use wearslot 1 in the packet
				hero_forge_model *= 100;

				for (uint8 wearslot = 0; wearslot < 7; wearslot++)
				{
					c->GetTarget()->SendTextureWC(wearslot, 0, (hero_forge_model + wearslot), 0, 0, 0);
				}
			}
			else
			{
				c->Message(13, "Hero's Forge Model must be greater than 0.");
			}
		}
	}
}

void command_setanim(Client *c, const Seperator *sep)
{
	if (c->GetTarget() && sep->IsNumber(1)) {
		int num = atoi(sep->arg[1]);
		if(num < 0 || num >= _eaMaxAppearance) {
		c->Message(0, "Invalid animation number, between 0 and %d",  _eaMaxAppearance-1);
		}
		c->GetTarget()->SetAppearance(EmuAppearance(num));
	} else
		c->Message(0, "Usage: #setanim [animnum]");
}

void command_serverinfo(Client *c, const Seperator *sep)
{
#ifdef _WINDOWS
	char intbuffer [sizeof(unsigned long)];
	c->Message(0, "Operating system information.");
	c->Message(0, "	%s",  Ver_name);
	c->Message(0, "	Build number: %s",  ultoa(Ver_build, intbuffer, 10));
	c->Message(0, "	Minor version: %s",  ultoa(Ver_min, intbuffer, 10));
	c->Message(0, "	Major version: %s",  ultoa(Ver_maj, intbuffer, 10));
	c->Message(0, "	Platform Id: %s",  ultoa(Ver_pid, intbuffer, 10));
#else
char buffer[255];
	c->Message(0, "Operating system information: %s", GetOS(buffer));
#endif
}

void command_getvariable(Client *c, const Seperator *sep)
{
	std::string tmp;
	if (database.GetVariable(sep->argplus[1], tmp))
		c->Message(0, "%s = %s",  sep->argplus[1], tmp.c_str());
	else
		c->Message(0, "GetVariable(%s) returned false",  sep->argplus[1]);
}

void command_chat(Client *c, const Seperator *sep)
{
	if (sep->arg[2][0] == 0)
		c->Message(0, "Usage: #chat [channum] [message]");
	else
		if (!worldserver.SendChannelMessage(0, 0, (uint8) atoi(sep->arg[1]), 0, 0, sep->argplus[2]))
			c->Message(0, "Error: World server disconnected");
}

void command_npcloot(Client *c, const Seperator *sep)
{
	if (c->GetTarget() == 0)
		c->Message(0, "Error: No target");
	// #npcloot show
	else if (strcasecmp(sep->arg[1], "show") == 0)
	{
		if (c->GetTarget()->IsNPC())
			c->GetTarget()->CastToNPC()->QueryLoot(c);
		else if (c->GetTarget()->IsCorpse())
			c->GetTarget()->CastToCorpse()->QueryLoot(c);
		else
			c->Message(0, "Error: Target's type doesnt have loot");
	}
	// These 2 types are *BAD* for the next few commands
	else if (c->GetTarget()->IsClient() || c->GetTarget()->IsCorpse())
		c->Message(0, "Error: Invalid target type, try a NPC =).");
	// #npcloot add
	else if (strcasecmp(sep->arg[1], "add") == 0)
	{
		// #npcloot add item
		if (c->GetTarget()->IsNPC() && sep->IsNumber(2))
		{
			uint32 item = atoi(sep->arg[2]);
			if (database.GetItem(item))
			{
				if (sep->arg[3][0] != 0 && sep->IsNumber(3))
					c->GetTarget()->CastToNPC()->AddItem(item, atoi(sep->arg[3]), 0);
				else
					c->GetTarget()->CastToNPC()->AddItem(item, 1, 0);
				c->Message(0, "Added item(%i) to the %s's loot.",  item, c->GetTarget()->GetName());
			}
			else
				c->Message(0, "Error: #npcloot add: Item(%i) does not exist!",  item);
		}
		else if (!sep->IsNumber(2))
			c->Message(0, "Error: #npcloot add: Itemid must be a number.");
		else
			c->Message(0, "Error: #npcloot add: This is not a valid target.");
	}
	// #npcloot remove
	else if (strcasecmp(sep->arg[1], "remove") == 0)
	{
		//#npcloot remove all
		if (strcasecmp(sep->arg[2], "all") == 0)
			c->Message(0, "Error: #npcloot remove all: Not yet implemented.");
		//#npcloot remove itemid
		else
		{
			if(c->GetTarget()->IsNPC() && sep->IsNumber(2))
			{
				uint32 item = atoi(sep->arg[2]);
				c->GetTarget()->CastToNPC()->RemoveItem(item);
				c->Message(0, "Removed item(%i) from the %s's loot.",  item, c->GetTarget()->GetName());
			}
			else if (!sep->IsNumber(2))
				c->Message(0, "Error: #npcloot remove: Item must be a number.");
			else
				c->Message(0, "Error: #npcloot remove: This is not a valid target.");
		}
	}
	// #npcloot money
	else if (strcasecmp(sep->arg[1], "money") == 0)
	{
		if (c->GetTarget()->IsNPC() && sep->IsNumber(2) && sep->IsNumber(3) && sep->IsNumber(4) && sep->IsNumber(5))
		{
			if ((atoi(sep->arg[2]) < 34465 && atoi(sep->arg[2]) >= 0) && (atoi(sep->arg[3]) < 34465 && atoi(sep->arg[3]) >= 0) && (atoi(sep->arg[4]) < 34465 && atoi(sep->arg[4]) >= 0) && (atoi(sep->arg[5]) < 34465 && atoi(sep->arg[5]) >= 0))
			{
				c->GetTarget()->CastToNPC()->AddCash(atoi(sep->arg[5]), atoi(sep->arg[4]), atoi(sep->arg[3]), atoi(sep->arg[2]));
				c->Message(0, "Set %i Platinum, %i Gold, %i Silver, and %i Copper as %s's money.",  atoi(sep->arg[2]), atoi(sep->arg[3]), atoi(sep->arg[4]), atoi(sep->arg[5]), c->GetTarget()->GetName());
			}
			else
				c->Message(0, "Error: #npcloot money: Values must be between 0-34465.");
		}
		else
			c->Message(0, "Usage: #npcloot money platinum gold silver copper");
	}
	else
		c->Message(0, "Usage: #npcloot [show/money/add/remove] [itemid/all/money: pp gp sp cp]");
}

void command_gm(Client *c, const Seperator *sep)
{
	bool state=atobool(sep->arg[1]);
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	if(sep->arg[1][0] != 0) {
		t->SetGM(state);
		c->Message(0, "%s is %s a GM.",  t->GetName(), state?"now":"no longer");
	}
	else
		c->Message(0, "Usage: #gm [on/off]");
}

// there's no need for this, as /summon already takes care of it
// this command is here for reference but it is not added to the
// list above

//To whoever wrote the above: And what about /kill, /zone, /zoneserver, etc?
//There is a reason for the # commands: so that admins can specifically enable certain
//commands for their users. Some might want users to #summon but not to /kill. Cant do that if they are a GM
void command_summon(Client *c, const Seperator *sep)
{
	Mob *t;

	if(sep->arg[1][0] != 0)		// arg specified
	{
		Client* client = entity_list.GetClientByName(sep->arg[1]);
		if (client != 0)	// found player in zone
			t=client->CastToMob();
		else
		{
			if (!worldserver.Connected())
				c->Message(0, "Error: World server disconnected.");
			else
			{ // player is in another zone
				//Taking this command out until we test the factor of 8 in ServerOP_ZonePlayer
				//c->Message(0, "Summoning player from another zone not yet implemented.");
				//return;

				auto pack = new ServerPacket(ServerOP_ZonePlayer, sizeof(ServerZonePlayer_Struct));
				ServerZonePlayer_Struct* szp = (ServerZonePlayer_Struct*) pack->pBuffer;
				strcpy(szp->adminname, c->GetName());
				szp->adminrank = c->Admin();
				szp->ignorerestrictions = 2;
				strcpy(szp->name, sep->arg[1]);
				strcpy(szp->zone, zone->GetShortName());
				szp->x_pos = c->GetX(); // May need to add a factor of 8 in here..
				szp->y_pos = c->GetY();
				szp->z_pos = c->GetZ();
				szp->instance_id = zone->GetInstanceID();
				worldserver.SendPacket(pack);
				safe_delete(pack);
			}
			return;
		}
	}
	else if(c->GetTarget())		// have target
		t=c->GetTarget();
	else
	{
		/*if(c->Admin() < 150)
			c->Message(0, "You need a NPC/corpse target for this command");
		else*/
			c->Message(0, "Usage: #summon [charname] Either target or charname is required");
		return;
	}

	if(!t)
		return;

	if (t->IsNPC())
	{ // npc target
		c->Message(0, "Summoning NPC %s to %1.1f, %1.1f, %1.1f",  t->GetName(), c->GetX(), c->GetY(), c->GetZ());
		t->CastToNPC()->GMMove(c->GetX(), c->GetY(), c->GetZ(), c->GetHeading());
		t->CastToNPC()->SaveGuardSpot(true);
	}
	else if (t->IsCorpse())
	{ // corpse target
		c->Message(0, "Summoning corpse %s to %1.1f, %1.1f, %1.1f",  t->GetName(), c->GetX(), c->GetY(), c->GetZ());
		t->CastToCorpse()->GMMove(c->GetX(), c->GetY(), c->GetZ(), c->GetHeading());
	}
	else if (t->IsClient())
	{
		/*if(c->Admin() < 150)
		{
			c->Message(0, "You may not summon a player.");
			return;
		}*/
		c->Message(0, "Summoning player %s to %1.1f, %1.1f, %1.1f",  t->GetName(), c->GetX(), c->GetY(), c->GetZ());
		t->CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), c->GetX(), c->GetY(), c->GetZ(), c->GetHeading(), 2, GMSummon);
	}
}

void command_zone(Client *c, const Seperator *sep)
{
	if(c->Admin() < commandZoneToCoords &&
		(sep->IsNumber(2) || sep->IsNumber(3) || sep->IsNumber(4))) {
		c->Message(0, "Your status is not high enough to zone to specific coordinates.");
		return;
	}

	uint16 zoneid = 0;

	if (sep->IsNumber(1))
	{
		if(atoi(sep->arg[1])==26 && (c->Admin() < commandZoneToSpecials)){ //cshome
				c->Message(0, "Only Guides and above can goto that zone.");
				return;
		}
		zoneid = atoi(sep->arg[1]);
	}
	else if (sep->arg[1][0] == 0)
	{
		c->Message(0, "Usage: #zone [zonename]");
		c->Message(0, "Optional Usage: #zone [zonename] y x z");
		return;
	}
	else if (zone->GetZoneID() == 184 && c->Admin() < commandZoneToSpecials) {	// Zone: 'Load'
		c->Message(0, "The Gods brought you here, only they can send you away.");
		return;
	} else {
		if((strcasecmp(sep->arg[1], "cshome")==0) && (c->Admin() < commandZoneToSpecials)){
			c->Message(0, "Only Guides and above can goto that zone.");
			return;
		}

		zoneid = database.GetZoneID(sep->arg[1]);
		if(zoneid == 0) {
			c->Message(0, "Unable to locate zone '%s'",  sep->arg[1]);
			return;
		}
	}

#ifdef BOTS
	// This block is necessary to clean up any bot objects owned by a Client
	if(zoneid != c->GetZoneID())
		Bot::ProcessClientZoneChange(c);
#endif

	if (sep->IsNumber(2) || sep->IsNumber(3) || sep->IsNumber(4)){
		//zone to specific coords
		c->MovePC(zoneid, (float)atof(sep->arg[2]), atof(sep->arg[3]), atof(sep->arg[4]), 0.0f, 0);
		}
	else
		//zone to safe coords
		c->MovePC(zoneid, 0.0f, 0.0f, 0.0f, 0.0f, 0, ZoneToSafeCoords);
}

//todo: fix this so it checks if you're in the instance set
void command_zone_instance(Client *c, const Seperator *sep)
{
	if(c->Admin() < commandZoneToCoords &&
		(sep->IsNumber(2) || sep->IsNumber(3) || sep->IsNumber(4))) {
		c->Message(0, "Your status is not high enough to zone to specific coordinates.");
		return;
	}

	if (sep->arg[1][0] == 0)
	{
		c->Message(0, "Usage: #zoneinstance [instance id]");
		c->Message(0, "Optional Usage: #zoneinstance [instance id] y x z");
		return;
	}

	uint16 zoneid = 0;
	uint16 instanceid = 0;

	if(sep->IsNumber(1))
	{
		instanceid = atoi(sep->arg[1]);
		if(!instanceid)
		{
			c->Message(0, "Must enter a valid instance id.");
			return;
		}

		zoneid = database.ZoneIDFromInstanceID(instanceid);
		if(!zoneid)
		{
			c->Message(0, "Instance not found or zone is set to null.");
			return;
		}
	}
	else
	{
		c->Message(0, "Must enter a valid instance id.");
		return;
	}

	if(!database.VerifyInstanceAlive(instanceid, c->CharacterID()))
	{
		c->Message(0, "Instance ID expiried or you are not apart of this instance.");
		return;
	}

	if (sep->IsNumber(2) || sep->IsNumber(3) || sep->IsNumber(4)){
		//zone to specific coords
		c->MovePC(zoneid, instanceid, atof(sep->arg[2]), atof(sep->arg[3]), atof(sep->arg[4]), 0.0f, 0);
	}
	else{
		c->MovePC(zoneid, instanceid, 0.0f, 0.0f, 0.0f, 0.0f, 0, ZoneToSafeCoords);
	}
}

void command_showbuffs(Client *c, const Seperator *sep)
{
	if (c->GetTarget() == 0)
		c->CastToMob()->ShowBuffs(c);
	else
		c->GetTarget()->CastToMob()->ShowBuffs(c);
}

void command_peqzone(Client *c, const Seperator *sep)
{
	uint32 timeleft = c->GetPTimers().GetRemainingTime(pTimerPeqzoneReuse)/60;

	if(!c->GetPTimers().Expired(&database, pTimerPeqzoneReuse, false)) {
		c->Message(13,"You must wait %i minute(s) before using this ability again.",  timeleft);
		return;
	}

	if(c->GetHPRatio() < 75) {
		c->Message(0, "You cannot use this command with less than 75 percent health.");
		return;
	}

	//this isnt perfect, but its better...
	if(
		c->IsInvisible(c)
		|| c->IsRooted()
		|| c->IsStunned()
		|| c->IsMezzed()
		|| c->AutoAttackEnabled()
		|| c->GetInvul()
	) {
		c->Message(0, "You cannot use this command in your current state. Settle down and wait.");
		return;
	}

	uint16 zoneid = 0;
	uint8 destzone = 0;
	if (sep->IsNumber(1))
	{
		zoneid = atoi(sep->arg[1]);
		destzone = database.GetPEQZone(zoneid, 0);
		if(destzone == 0){
			c->Message(13, "You cannot use this command to enter that zone!");
			return;
		}
		if(zoneid == zone->GetZoneID()) {
			c->Message(13, "You cannot use this command on the zone you are in!");
			return;
		}
	}
	else if (sep->arg[1][0] == 0 || sep->IsNumber(2) || sep->IsNumber(3) || sep->IsNumber(4) || sep->IsNumber(5))
	{
		c->Message(0, "Usage: #peqzone [zonename]");
		c->Message(0, "Optional Usage: #peqzone [zoneid]");
		return;
	}
	else {
		zoneid = database.GetZoneID(sep->arg[1]);
		destzone = database.GetPEQZone(zoneid, 0);
		if(zoneid == 0) {
			c->Message(0, "Unable to locate zone '%s'",  sep->arg[1]);
			return;
		}
		if(destzone == 0){
			c->Message(13, "You cannot use this command to enter that zone!");
			return;
		}
		if(zoneid == zone->GetZoneID()) {
			c->Message(13, "You cannot use this command on the zone you are in!");
			return;
		}
	}

	if(RuleB (Zone, UsePEQZoneDebuffs)){
		c->SpellOnTarget(RuleI(Zone, PEQZoneDebuff1), c);
		c->SpellOnTarget(RuleI(Zone, PEQZoneDebuff2), c);
	}

	//zone to safe coords
	c->GetPTimers().Start(pTimerPeqzoneReuse, RuleI(Zone, PEQZoneReuseTime));
	c->MovePC(zoneid, 0.0f, 0.0f, 0.0f, 0.0f, 0, ZoneToSafeCoords);
}

void command_movechar(Client *c, const Seperator *sep)
{
	if(sep->arg[1][0]==0 || sep->arg[2][0] == 0)
		c->Message(0, "Usage: #movechar [charactername] [zonename]");
	else if (c->Admin() < commandMovecharToSpecials && strcasecmp(sep->arg[2], "cshome") == 0 || strcasecmp(sep->arg[2], "load") == 0 || strcasecmp(sep->arg[2], "load2") == 0)
		c->Message(0, "Invalid zone name");
	else
	{
		uint32 tmp = database.GetAccountIDByChar(sep->arg[1]);
		if (tmp)
		{
			if (c->Admin() >= commandMovecharSelfOnly || tmp == c->AccountID())
				if (!database.MoveCharacterToZone((char*) sep->arg[1], (char*) sep->arg[2]))
					c->Message(0, "Character Move Failed!");
				else
					c->Message(0, "Character has been moved.");
			else
				c->Message(13,"You cannot move characters that are not on your account.");
		}
		else
			c->Message(0, "Character Does Not Exist");
	}
}

void command_viewpetition(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0) {
		c->Message(0, "Usage: #viewpetition (petition number) Type #listpetition for a list");
		return;
    }

    c->Message(13,"	ID : Character Name , Petition Text");

    std::string query = "SELECT petid, charname, petitiontext FROM petitions ORDER BY petid";
    auto results = database.QueryDatabase(query);
    if (!results.Success())
        return;

    Log(Logs::General, Logs::Normal, "View petition request from %s, petition number: %i",  c->GetName(), atoi(sep->argplus[1]) );

    if (results.RowCount() == 0) {
        c->Message(13,"There was an error in your request: ID not found! Please check the Id and try again.");
        return;
    }

    for (auto row = results.begin(); row != results.end(); ++row)
        if (strcasecmp(row[0], sep->argplus[1]) == 0)
			c->Message(15, " %s:	%s , %s ",  row[0], row[1], row[2]);

}

void command_petitioninfo(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0) {
		c->Message(0, "Usage: #petitioninfo (petition number) Type #listpetition for a list");
		return;
    }

    std::string query = "SELECT petid, charname, accountname, zone, charclass, charrace, charlevel FROM petitions ORDER BY petid";
    auto results = database.QueryDatabase(query);
    if (!results.Success())
        return;

    Log(Logs::General, Logs::Normal, "Petition information request from %s, petition number:",  c->GetName(), atoi(sep->argplus[1]) );

    if (results.RowCount() == 0) {
		c->Message(13,"There was an error in your request: ID not found! Please check the Id and try again.");
        return;
    }

	for (auto row = results.begin(); row != results.end(); ++row)
		if (strcasecmp(row[0],sep->argplus[1])== 0)
			c->Message(13,"	ID : %s Character Name: %s Account Name: %s Zone: %s Character Class: %s Character Race: %s Character Level: %s", row[0],row[1],row[2],row[3],row[4],row[5],row[6]);

}

void command_delpetition(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0 || strcasecmp(sep->arg[1],"*") == 0) {
		c->Message(0, "Usage: #delpetition (petition number) Type #listpetition for a list");
		return;
    }

	c->Message(13,"Attempting to delete petition number: %i", atoi(sep->argplus[1]));
	std::string query = StringFormat("DELETE FROM petitions WHERE petid = %i",  atoi(sep->argplus[1]));
	auto results = database.QueryDatabase(query);
	if (!results.Success())
        return;

    Log(Logs::General, Logs::Normal, "Delete petition request from %s, petition number:",  c->GetName(), atoi(sep->argplus[1]) );

}

void command_listnpcs(Client *c, const Seperator *sep)
{
	if (strcasecmp(sep->arg[1], "all") == 0)
		entity_list.ListNPCs(c,sep->arg[1],sep->arg[2],0);
	else if(sep->IsNumber(1) && sep->IsNumber(2))
		entity_list.ListNPCs(c,sep->arg[1],sep->arg[2],2);
	else if(sep->arg[1][0] != 0)
		entity_list.ListNPCs(c,sep->arg[1],sep->arg[2],1);
	else {
		c->Message(0, "Usage of #listnpcs:");
		c->Message(0, "#listnpcs [#] [#] (Each number would search by ID, ex. #listnpcs 1 30, searches 1-30)");
		c->Message(0, "#listnpcs [name] (Would search for a npc with [name])");
	}
}

void command_date(Client *c, const Seperator *sep)
{
	//yyyy mm dd hh mm local
	if(sep->arg[3][0]==0 || !sep->IsNumber(1) || !sep->IsNumber(2) || !sep->IsNumber(3)) {
		c->Message(13, "Usage: #date yyyy mm dd [HH MM]");
	}
	else {
		int h=0, m=0;
		TimeOfDay_Struct eqTime;
		zone->zone_time.GetCurrentEQTimeOfDay( time(0), &eqTime);
		if(!sep->IsNumber(4))
		h=eqTime.hour;
		else
			h=atoi(sep->arg[4]);
		if(!sep->IsNumber(5))
			m=eqTime.minute;
		else
			m=atoi(sep->arg[5]);
		c->Message(13, "Setting world time to %s-%s-%s %i:%i...",  sep->arg[1], sep->arg[2], sep->arg[3], h, m);
		zone->SetDate(atoi(sep->arg[1]), atoi(sep->arg[2]), atoi(sep->arg[3]), h, m);
	}
}

void command_timezone(Client *c, const Seperator *sep)
{
	if(sep->arg[1][0]==0 && !sep->IsNumber(1)) {
		c->Message(13, "Usage: #timezone HH [MM]");
		c->Message(13, "Current timezone is: %ih %im",  zone->zone_time.getEQTimeZoneHr(), zone->zone_time.getEQTimeZoneMin());
	}
	else {
		uint8 hours = atoi(sep->arg[1]);
		uint8 minutes = atoi(sep->arg[2]);
		if(!sep->IsNumber(2))
			minutes = 0;
		c->Message(13, "Setting timezone to %i h %i m",  hours, minutes);
		uint32 ntz=(hours*60)+minutes;
		zone->zone_time.setEQTimeZone(ntz);
		database.SetZoneTZ(zone->GetZoneID(), zone->GetInstanceVersion(), ntz);

		// Update all clients with new TZ.
		auto outapp = new EQApplicationPacket(OP_TimeOfDay, sizeof(TimeOfDay_Struct));
		TimeOfDay_Struct* tod = (TimeOfDay_Struct*)outapp->pBuffer;
		zone->zone_time.GetCurrentEQTimeOfDay(time(0), tod);
		entity_list.QueueClients(c, outapp);
		safe_delete(outapp);
	}
}

void command_invul(Client *c, const Seperator *sep)
{
	bool state=atobool(sep->arg[1]);
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	if(sep->arg[1][0] != 0) {
		t->SetInvul(state);
		c->Message(0, "%s is %s invulnerable from attack.",  t->GetName(), state?"now":"no longer");
	}
	else
		c->Message(0, "Usage: #invulnerable [on/off]");
}

void command_hideme(Client *c, const Seperator *sep)
{
	bool state=atobool(sep->arg[1]);

	if(sep->arg[1][0]==0)
		c->Message(0, "Usage: #hideme [on/off]");
	else
	{
		c->SetHideMe(state);
		c->Message_StringID(MT_Broadcasts, c->GetHideMe() ? NOW_INVISIBLE : NOW_VISIBLE, c->GetName());
	}
}

void command_emote(Client *c, const Seperator *sep)
{
	if (sep->arg[3][0] == 0)
		c->Message(0, "Usage: #emote [name | world | zone] type# message");
	else {
		if (strcasecmp(sep->arg[1], "zone") == 0){
			char* newmessage=0;
			if(strstr(sep->arg[3],"^")==0)
				entity_list.Message(0, atoi(sep->arg[2]), sep->argplus[3]);
			else{
				for(newmessage = strtok((char*)sep->arg[3],"^");newmessage!=nullptr;newmessage=strtok(nullptr, "^"))
					entity_list.Message(0, atoi(sep->arg[2]), newmessage);
			}
		}
		else if (!worldserver.Connected())
			c->Message(0, "Error: World server disconnected");
		else if (strcasecmp(sep->arg[1], "world") == 0)
			worldserver.SendEmoteMessage(0, 0, atoi(sep->arg[2]), sep->argplus[3]);
		else
			worldserver.SendEmoteMessage(sep->arg[1], 0, atoi(sep->arg[2]), sep->argplus[3]);
	}
}

void command_fov(Client *c, const Seperator *sep)
{
	if(c->GetTarget())
		if(c->BehindMob(c->GetTarget(), c->GetX(), c->GetY()))
			c->Message(0, "You are behind mob %s, it is looking to %d",  c->GetTarget()->GetName(), c->GetTarget()->GetHeading());
		else
			c->Message(0, "You are NOT behind mob %s, it is looking to %d",  c->GetTarget()->GetName(), c->GetTarget()->GetHeading());
	else
		c->Message(0, "I Need a target!");
}

void command_npcstats(Client *c, const Seperator *sep)
{
	if (c->GetTarget() == 0)
		c->Message(0, "ERROR: No target!");
	else if (!c->GetTarget()->IsNPC())
		c->Message(0, "ERROR: Target is not a NPC!");
	else {
		auto target_npc = c->GetTarget()->CastToNPC();
		c->Message(0, "NPC Stats:");
		c->Message(0, "Name: %s   NpcID: %u", target_npc->GetName(), target_npc->GetNPCTypeID());
		c->Message(0, "Race: %i  Level: %i  Class: %i  Material: %i", target_npc->GetRace(), target_npc->GetLevel(), target_npc->GetClass(), target_npc->GetTexture());
		c->Message(0, "Current HP: %i  Max HP: %i", target_npc->GetHP(), target_npc->GetMaxHP());
		//c->Message(0, "Weapon Item Number: %s", target_npc->GetWeapNo());
		c->Message(0, "Gender: %i  Size: %f  Bodytype: %d", target_npc->GetGender(), target_npc->GetSize(), target_npc->GetBodyType());
		c->Message(0, "Runspeed: %.3f  Walkspeed: %.3f", static_cast<float>(0.025f * target_npc->GetRunspeed()), static_cast<float>(0.025f * target_npc->GetWalkspeed()));
		c->Message(0, "Spawn Group: %i  Grid: %i", target_npc->GetSp2(), target_npc->GetGrid());
		if (target_npc->proximity) {
			c->Message(0, "Proximity: Enabled");
			c->Message(0, "Cur_X: %1.3f, Cur_Y: %1.3f, Cur_Z: %1.3f", target_npc->GetX(), target_npc->GetY(), target_npc->GetZ());
			c->Message(0, "Min_X: %1.3f(%1.3f), Max_X: %1.3f(%1.3f), X_Range: %1.3f", target_npc->proximity->min_x, (target_npc->proximity->min_x - target_npc->GetX()), target_npc->proximity->max_x, (target_npc->proximity->max_x - target_npc->GetX()), (target_npc->proximity->max_x - target_npc->proximity->min_x));
			c->Message(0, "Min_Y: %1.3f(%1.3f), Max_Y: %1.3f(%1.3f), Y_Range: %1.3f", target_npc->proximity->min_y, (target_npc->proximity->min_y - target_npc->GetY()), target_npc->proximity->max_y, (target_npc->proximity->max_y - target_npc->GetY()), (target_npc->proximity->max_y - target_npc->proximity->min_y));
			c->Message(0, "Min_Z: %1.3f(%1.3f), Max_Z: %1.3f(%1.3f), Z_Range: %1.3f", target_npc->proximity->min_z, (target_npc->proximity->min_z - target_npc->GetZ()), target_npc->proximity->max_z, (target_npc->proximity->max_z - target_npc->GetZ()), (target_npc->proximity->max_z - target_npc->proximity->min_z));
			c->Message(0, "Say: %s", (target_npc->proximity->say ? "Enabled" : "Disabled"));
		}
		else {
			c->Message(0, "Proximity: Disabled");
		}
		c->Message(0, "");
		c->Message(0, "EmoteID: %i", target_npc->GetEmoteID());
		target_npc->QueryLoot(c);
	}
}

void command_zclip(Client *c, const Seperator *sep)
{
	// modifys and resends zhdr packet
	if(sep->arg[2][0]==0)
		c->Message(0, "Usage: #zclip <min clip> <max clip>");
	else if(atoi(sep->arg[1])<=0)
		c->Message(0, "ERROR: Min clip can not be zero or less!");
	else if(atoi(sep->arg[2])<=0)
		c->Message(0, "ERROR: Max clip can not be zero or less!");
	else if(atoi(sep->arg[1])>atoi(sep->arg[2]))
		c->Message(0, "ERROR: Min clip is greater than max clip!");
	else {
		zone->newzone_data.minclip = atof(sep->arg[1]);
		zone->newzone_data.maxclip = atof(sep->arg[2]);
		if(sep->arg[3][0]!=0)
			zone->newzone_data.fog_minclip[0]=atof(sep->arg[3]);
		if(sep->arg[4][0]!=0)
			zone->newzone_data.fog_minclip[1]=atof(sep->arg[4]);
		if(sep->arg[5][0]!=0)
			zone->newzone_data.fog_maxclip[0]=atof(sep->arg[5]);
		if(sep->arg[6][0]!=0)
			zone->newzone_data.fog_maxclip[1]=atof(sep->arg[6]);
		auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
		memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
		entity_list.QueueClients(c, outapp);
		safe_delete(outapp);
	}
}

void command_npccast(Client *c, const Seperator *sep)
{
	if (c->GetTarget() && c->GetTarget()->IsNPC() && !sep->IsNumber(1) && sep->arg[1] != 0 && sep->IsNumber(2)) {
		Mob* spelltar = entity_list.GetMob(sep->arg[1]);
		if (spelltar)
			c->GetTarget()->CastSpell(atoi(sep->arg[2]), spelltar->GetID());
		else
			c->Message(0, "Error: %s not found",  sep->arg[1]);
	}
	else if (c->GetTarget() && c->GetTarget()->IsNPC() && sep->IsNumber(1) && sep->IsNumber(2) ) {
		Mob* spelltar = entity_list.GetMob(atoi(sep->arg[1]));
		if (spelltar)
			c->GetTarget()->CastSpell(atoi(sep->arg[2]), spelltar->GetID());
		else
			c->Message(0, "Error: target ID %i not found",  atoi(sep->arg[1]));
	}
	else
		c->Message(0, "Usage: (needs NPC targeted) #npccast targetname/entityid spellid");
}

void command_zstats(Client *c, const Seperator *sep)
{
	c->Message(0, "Zone Header Data:");
	c->Message(0, "Sky Type: %i",  zone->newzone_data.sky);
	c->Message(0, "Fog Colour: Red: %i; Blue: %i; Green %i",  zone->newzone_data.fog_red[0], zone->newzone_data.fog_green[0], zone->newzone_data.fog_blue[0]);
	c->Message(0, "Safe Coords: %f, %f, %f",  zone->newzone_data.safe_x, zone->newzone_data.safe_y, zone->newzone_data.safe_z);
	c->Message(0, "Underworld Coords: %f",  zone->newzone_data.underworld);
	c->Message(0, "Clip Plane: %f - %f",  zone->newzone_data.minclip, zone->newzone_data.maxclip);
}

void command_permaclass(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	if(sep->arg[1][0]==0) {
		c->Message(0,"Usage: #permaclass <classnum>");
	}
	else if(!t->IsClient())
		c->Message(0,"Target is not a client.");
	else {
		c->Message(0, "Setting %s's class...Sending to char select.",  t->GetName());
		Log(Logs::General, Logs::Normal, "Class change request from %s for %s, requested class:%i",  c->GetName(), t->GetName(), atoi(sep->arg[1]) );
		t->SetBaseClass(atoi(sep->arg[1]));
		t->Save();
		t->Kick();
	}
}

void command_permarace(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	if(sep->arg[1][0]==0) {
		c->Message(0,"Usage: #permarace <racenum>");
		c->Message(0,"NOTE: Not all models are global. If a model is not global, it will appear as a human on character select and in zones without the model.");
	}
	else if(!t->IsClient())
		c->Message(0,"Target is not a client.");
	else {
		c->Message(0, "Setting %s's race - zone to take effect", t->GetName());
		Log(Logs::General, Logs::Normal, "Permanant race change request from %s for %s, requested race:%i",  c->GetName(), t->GetName(), atoi(sep->arg[1]) );
		uint32 tmp = Mob::GetDefaultGender(atoi(sep->arg[1]), t->GetBaseGender());
		t->SetBaseRace(atoi(sep->arg[1]));
		t->SetBaseGender(tmp);
		t->Save();
		t->SendIllusionPacket(atoi(sep->arg[1]));
	}
}

void command_permagender(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	if(sep->arg[1][0]==0) {
		c->Message(0,"Usage: #permagender <gendernum>");
		c->Message(0,"Gender Numbers: 0=Male, 1=Female, 2=Neuter");
	}
	else if(!t->IsClient())
		c->Message(0,"Target is not a client.");
	else {
		c->Message(0, "Setting %s's gender - zone to take effect", t->GetName());
		Log(Logs::General, Logs::Normal, "Permanant gender change request from %s for %s, requested gender:%i",  c->GetName(), t->GetName(), atoi(sep->arg[1]) );
		t->SetBaseGender(atoi(sep->arg[1]));
		t->Save();
		t->SendIllusionPacket(atoi(sep->arg[1]));
	}
}

void command_weather(Client *c, const Seperator *sep)
{
	if (!(sep->arg[1][0] == '0' || sep->arg[1][0] == '1' || sep->arg[1][0] == '2' || sep->arg[1][0] == '3')) {
		c->Message(0, "Usage: #weather <0/1/2/3> - Off/Rain/Snow/Manual.");
	}
	else if(zone->zone_weather == 0) {
		if(sep->arg[1][0] == '3')	{ // Put in modifications here because it had a very good chance at screwing up the client's weather system if rain was sent during snow -T7
			if(sep->arg[2][0] != 0 && sep->arg[3][0] != 0) {
				c->Message(0, "Sending weather packet... TYPE=%s, INTENSITY=%s",  sep->arg[2], sep->arg[3]);
				zone->zone_weather = atoi(sep->arg[2]);
				auto outapp = new EQApplicationPacket(OP_Weather, 8);
				outapp->pBuffer[0] = atoi(sep->arg[2]);
				outapp->pBuffer[4] = atoi(sep->arg[3]); // This number changes in the packets, intensity?
				entity_list.QueueClients(c, outapp);
				safe_delete(outapp);
			}
			else {
				c->Message(0, "Manual Usage: #weather 3 <type> <intensity>");
			}
		}
		else if(sep->arg[1][0] == '2')	{
			entity_list.Message(0, 0, "Snowflakes begin to fall from the sky.");
			zone->zone_weather = 2;
			auto outapp = new EQApplicationPacket(OP_Weather, 8);
			outapp->pBuffer[0] = 0x01;
			outapp->pBuffer[4] = 0x02; // This number changes in the packets, intensity?
			entity_list.QueueClients(c, outapp);
			safe_delete(outapp);
		}
		else if(sep->arg[1][0] == '1')	{
			entity_list.Message(0, 0, "Raindrops begin to fall from the sky.");
			zone->zone_weather = 1;
			auto outapp = new EQApplicationPacket(OP_Weather, 8);
			outapp->pBuffer[4] = 0x01; // This is how it's done in Fear, and you can see a decent distance with it at this value
			entity_list.QueueClients(c, outapp);
			safe_delete(outapp);
		}
	}
	else {
		if(zone->zone_weather == 1)	{ // Doing this because if you have rain/snow on, you can only turn one off.
			entity_list.Message(0, 0, "The sky clears as the rain ceases to fall.");
			zone->zone_weather = 0;
			auto outapp = new EQApplicationPacket(OP_Weather, 8);
			// To shutoff weather you send an empty 8 byte packet (You get this everytime you zone even if the sky is clear)
			entity_list.QueueClients(c, outapp);
			safe_delete(outapp);
		}
		else if(zone->zone_weather == 2) {
			entity_list.Message(0, 0, "The sky clears as the snow stops falling.");
			zone->zone_weather = 0;
			auto outapp = new EQApplicationPacket(OP_Weather, 8);
			// To shutoff weather you send an empty 8 byte packet (You get this everytime you zone even if the sky is clear)
			outapp->pBuffer[0] = 0x01; // Snow has it's own shutoff packet
			entity_list.QueueClients(c, outapp);
			safe_delete(outapp);
		}
		else {
			entity_list.Message(0, 0, "The sky clears.");
			zone->zone_weather = 0;
			auto outapp = new EQApplicationPacket(OP_Weather, 8);
			// To shutoff weather you send an empty 8 byte packet (You get this everytime you zone even if the sky is clear)
			entity_list.QueueClients(c, outapp);
			safe_delete(outapp);
		}
	}
}

void command_zheader(Client *c, const Seperator *sep)
{
	// sends zhdr packet
	if(sep->arg[1][0]==0) {
		c->Message(0, "Usage: #zheader <zone name>");
	}
	else if(database.GetZoneID(sep->argplus[1])==0)
		c->Message(0, "Invalid Zone Name: %s",  sep->argplus[1]);
	else {

		if (zone->LoadZoneCFG(sep->argplus[1], true))
			c->Message(0, "Successfully loaded zone header for %s from database.",  sep->argplus[1]);
		else
			c->Message(0, "Failed to load zone header %s from database",  sep->argplus[1]);
		auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
		memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
		entity_list.QueueClients(c, outapp);
		safe_delete(outapp);
	}
}

void command_zsky(Client *c, const Seperator *sep)
{
	// modifys and resends zhdr packet
	if(sep->arg[1][0]==0)
		c->Message(0, "Usage: #zsky <sky type>");
	else if(atoi(sep->arg[1])<0||atoi(sep->arg[1])>255)
		c->Message(0, "ERROR: Sky type can not be less than 0 or greater than 255!");
	else {
		zone->newzone_data.sky = atoi(sep->arg[1]);
		auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
		memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
		entity_list.QueueClients(c, outapp);
		safe_delete(outapp);
	}
}

void command_zcolor(Client *c, const Seperator *sep)
{
	// modifys and resends zhdr packet
	if (sep->arg[3][0]==0)
		c->Message(0, "Usage: #zcolor <red> <green> <blue>");
	else if (atoi(sep->arg[1])<0||atoi(sep->arg[1])>255)
		c->Message(0, "ERROR: Red can not be less than 0 or greater than 255!");
	else if (atoi(sep->arg[2])<0||atoi(sep->arg[2])>255)
		c->Message(0, "ERROR: Green can not be less than 0 or greater than 255!");
	else if (atoi(sep->arg[3])<0||atoi(sep->arg[3])>255)
		c->Message(0, "ERROR: Blue can not be less than 0 or greater than 255!");
	else {
		for (int z=0; z<4; z++) {
			zone->newzone_data.fog_red[z] = atoi(sep->arg[1]);
			zone->newzone_data.fog_green[z] = atoi(sep->arg[2]);
			zone->newzone_data.fog_blue[z] = atoi(sep->arg[3]);
		}
		auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
		memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
		entity_list.QueueClients(c, outapp);
		safe_delete(outapp);
	}
}

void command_spon(Client *c, const Seperator *sep)
{
	c->MemorizeSpell(0, SPELLBAR_UNLOCK, memSpellSpellbar);
}

void command_spoff(Client *c, const Seperator *sep)
{
	auto outapp = new EQApplicationPacket(OP_ManaChange, 0);
	outapp->priority = 5;
	c->QueuePacket(outapp);
	safe_delete(outapp);
}

void command_itemtest(Client *c, const Seperator *sep)
{
	char chBuffer[8192] = {0};
	//Using this to determine new item layout
	FILE* f = nullptr;
	if (!(f = fopen("c:\\EQEMUcvs\\ItemDump.txt",  "rb"))) {
		c->Message(13, "Error: Could not open c:\\EQEMUcvs\\ItemDump.txt");
		return;
	}

	fread(chBuffer, sizeof(chBuffer), sizeof(char), f);
	fclose(f);

	auto outapp = new EQApplicationPacket(OP_ItemLinkResponse, strlen(chBuffer) + 5);
	memcpy(&outapp->pBuffer[4], chBuffer, strlen(chBuffer));
	c->QueuePacket(outapp);
	safe_delete(outapp);
}

void command_gassign(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(1) && c->GetTarget() && c->GetTarget()->IsNPC() && c->GetTarget()->CastToNPC()->GetSpawnPointID() > 0) {
		int spawn2id = c->GetTarget()->CastToNPC()->GetSpawnPointID();
		database.AssignGrid(c, atoi(sep->arg[1]), spawn2id);
	}
	else
		c->Message(0, "Usage: #gassign [num] - must have an npc target!");
}

void command_ai(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();

	if (strcasecmp(sep->arg[1], "factionid") == 0) {
		if (target && sep->IsNumber(2)) {
			if (target->IsNPC())
				target->CastToNPC()->SetNPCFactionID(atoi(sep->arg[2]));
			else
				c->Message(0, "%s is not an NPC.",  target->GetName());
		}
		else
			c->Message(0, "Usage: (targeted) #ai factionid [factionid]");
	}
	else if (strcasecmp(sep->arg[1], "spellslist") == 0) {
		if (target && sep->IsNumber(2) && atoi(sep->arg[2]) >= 0) {
			if (target->IsNPC())
				target->CastToNPC()->AI_AddNPCSpells(atoi(sep->arg[2]));
			else
				c->Message(0, "%s is not an NPC.",  target->GetName());
		}
		else
			c->Message(0, "Usage: (targeted) #ai spellslist [npc_spells_id]");
	}
	else if (strcasecmp(sep->arg[1], "con") == 0) {
		if (target && sep->arg[2][0] != 0) {
			Mob* tar2 = entity_list.GetMob(sep->arg[2]);
			if (tar2)
				c->Message(0, "%s considering %s: %i",  target->GetName(), tar2->GetName(), tar2->GetReverseFactionCon(target));
			else
				c->Message(0, "Error: %s not found.",  sep->arg[2]);
		}
		else
			c->Message(0, "Usage: (targeted) #ai con [mob name]");
	}
	else if (strcasecmp(sep->arg[1], "guard") == 0) {
		if (target && target->IsNPC())
			target->CastToNPC()->SaveGuardSpot();
		else
			c->Message(0, "Usage: (targeted) #ai guard - sets npc to guard the current location (use #summon to move)");
	}
	else if (strcasecmp(sep->arg[1], "roambox") == 0) {
		if (target && target->IsAIControlled() && target->IsNPC()) {
			if ((sep->argnum == 6 || sep->argnum == 7 || sep->argnum == 8) && sep->IsNumber(2) && sep->IsNumber(3) && sep->IsNumber(4) && sep->IsNumber(5) && sep->IsNumber(6)) {
				uint32 tmp = 2500;
				uint32 tmp2 = 2500;
				if (sep->IsNumber(7))
					tmp = atoi(sep->arg[7]);
				if (sep->IsNumber(8))
					tmp2 = atoi(sep->arg[8]);
				target->CastToNPC()->AI_SetRoambox(atof(sep->arg[2]), atof(sep->arg[3]), atof(sep->arg[4]), atof(sep->arg[5]), atof(sep->arg[6]), tmp, tmp2);
			}
			else if ((sep->argnum == 3 || sep->argnum == 4) && sep->IsNumber(2) && sep->IsNumber(3)) {
				uint32 tmp = 2500;
				uint32 tmp2 = 2500;
				if (sep->IsNumber(4))
					tmp = atoi(sep->arg[4]);
				if (sep->IsNumber(5))
					tmp2 = atoi(sep->arg[5]);
				target->CastToNPC()->AI_SetRoambox(atof(sep->arg[2]), atof(sep->arg[3]), tmp, tmp2);
			}
			else {
				c->Message(0, "Usage: #ai roambox dist max_x min_x max_y min_y [delay] [mindelay]");
				c->Message(0, "Usage: #ai roambox dist roamdist [delay] [mindelay]");
			}
		}
		else
			c->Message(0, "You need a AI NPC targeted");
	}
	else if (strcasecmp(sep->arg[1], "stop") == 0 && c->Admin() >= commandToggleAI) {
		if (target) {
			if (target->IsAIControlled())
				target->AI_Stop();
			else
				c->Message(0, "Error: Target is not AI controlled");
		}
		else
			c->Message(0, "Usage: Target a Mob with AI enabled and use this to turn off their AI.");
	}
	else if (strcasecmp(sep->arg[1], "start") == 0 && c->Admin() >= commandToggleAI) {
		if (target) {
			if (!target->IsAIControlled())
				target->AI_Start();
			else
				c->Message(0, "Error: Target is already AI controlled");
		}
		else
			c->Message(0, "Usage: Target a Mob with AI disabled and use this to turn on their AI.");
	}
	else {
		c->Message(0, "#AI Sub-commands");
		c->Message(0, "  factionid");
		c->Message(0, "  spellslist");
		c->Message(0, "  con");
		c->Message(0, "  guard");
	}
}

void command_worldshutdown(Client *c, const Seperator *sep)
{
	// GM command to shutdown world server and all zone servers
	uint32 time=0;
	uint32 interval=0;
	if (worldserver.Connected()) {
		if(sep->IsNumber(1) && sep->IsNumber(2) && ((time=atoi(sep->arg[1]))>0) && ((interval=atoi(sep->arg[2]))>0)) {
			worldserver.SendEmoteMessage(0,0,15,"<SYSTEMWIDE MESSAGE>:SYSTEM MSG:World coming down in %i minutes, everyone log out before this time.",  (time / 60 ));
			c->Message(0, "Sending shutdown packet now, World will shutdown in: %i minutes with an interval of: %i seconds",  (time / 60), interval);
			auto pack = new ServerPacket(ServerOP_ShutdownAll, sizeof(WorldShutDown_Struct));
			WorldShutDown_Struct* wsd = (WorldShutDown_Struct*)pack->pBuffer;
			wsd->time=time*1000;
			wsd->interval=(interval*1000);
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
		else if(strcasecmp(sep->arg[1], "now") == 0){
			worldserver.SendEmoteMessage(0,0,15,"<SYSTEMWIDE MESSAGE>:SYSTEM MSG:World coming down, everyone log out now.");
			c->Message(0, "Sending shutdown packet");
			auto pack = new ServerPacket;
			pack->opcode = ServerOP_ShutdownAll;
			pack->size=0;
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
		else if(strcasecmp(sep->arg[1], "disable") == 0){
			c->Message(0, "Shutdown prevented, next time I may not be so forgiving...");
			auto pack = new ServerPacket(ServerOP_ShutdownAll, sizeof(WorldShutDown_Struct));
			WorldShutDown_Struct* wsd = (WorldShutDown_Struct*)pack->pBuffer;
			wsd->time=0;
			wsd->interval=0;
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
		else{
			c->Message(0,"#worldshutdown - Shuts down the server and all zones.");
			c->Message(0,"Usage: #worldshutdown now - Shuts down the server and all zones immediately.");
			c->Message(0,"Usage: #worldshutdown disable - Stops the server from a previously scheduled shut down.");
			c->Message(0,"Usage: #worldshutdown [timer] [interval] - Shuts down the server and all zones after [timer] seconds and sends warning every [interval] seconds.");
		}
	}
	else
		c->Message(0, "Error: World server disconnected");
}

void command_sendzonespawns(Client *c, const Seperator *sep)
{
	entity_list.SendZoneSpawns(c);
}

void command_zsave(Client *c, const Seperator *sep)
{
	if(zone->SaveZoneCFG())
		c->Message(13, "Zone header saved successfully.");
		else
		c->Message(13, "ERROR: Zone header data was NOT saved.");
}

void command_dbspawn2(Client *c, const Seperator *sep)
{

	if (sep->IsNumber(1) && sep->IsNumber(2) && sep->IsNumber(3)) {
		Log(Logs::General, Logs::Normal, "Spawning database spawn");
		uint16 cond = 0;
		int16 cond_min = 0;
		if(sep->IsNumber(4)) {
			cond = atoi(sep->arg[4]);
			if(sep->IsNumber(5))
				cond_min = atoi(sep->arg[5]);
		}
		database.CreateSpawn2(c, atoi(sep->arg[1]), zone->GetShortName(), c->GetPosition(), atoi(sep->arg[2]), atoi(sep->arg[3]), cond, cond_min);
	}
	else {
		c->Message(0, "Usage: #dbspawn2 spawngroup respawn variance [condition_id] [condition_min]");
	}
}

void command_shutdown(Client *c, const Seperator *sep)
{
	CatchSignal(2);
}

void command_delacct(Client *c, const Seperator *sep)
{
	if(sep->arg[1][0] == 0)
		c->Message(0, "Format: #delacct accountname");
	else
		if (database.DeleteAccount(sep->arg[1]))
			c->Message(0, "The account was deleted.");
		else
			c->Message(0, "Unable to delete account.");
}

void command_setpass(Client *c, const Seperator *sep)
{
	if(sep->argnum != 2)
		c->Message(0, "Format: #setpass accountname password");
	else {
		int16 tmpstatus = 0;
		uint32 tmpid = database.GetAccountIDByName(sep->arg[1], &tmpstatus);
		if (!tmpid)
			c->Message(0, "Error: Account not found");
		else if (tmpstatus > c->Admin())
			c->Message(0, "Cannot change password: Account's status is higher than yours");
		else if (database.SetLocalPassword(tmpid, sep->arg[2]))
			c->Message(0, "Password changed.");
		else
			c->Message(0, "Error changing password.");
	}
}

void command_setlsinfo(Client *c, const Seperator *sep)
{
	if(sep->argnum != 2)
		c->Message(0, "Format: #setlsinfo email password");
	else {
		auto pack = new ServerPacket(ServerOP_LSAccountUpdate, sizeof(ServerLSAccountUpdate_Struct));
		ServerLSAccountUpdate_Struct* s = (ServerLSAccountUpdate_Struct *) pack->pBuffer;
		s->useraccountid = c->LSAccountID();
		strn0cpy(s->useraccount, c->AccountName(), 30);
		strn0cpy(s->useremail, sep->arg[1], 100);
		strn0cpy(s->userpassword, sep->arg[2], 50);
		worldserver.SendPacket(pack);
		c->Message(0, "Login Server update packet sent.");
	}
}

void command_grid(Client *c, const Seperator *sep)
{
	if (strcasecmp("max", sep->arg[1]) == 0)
		c->Message(0, "Highest grid ID in this zone: %d",  database.GetHighestGrid(zone->GetZoneID()));
	else if (strcasecmp("add", sep->arg[1]) == 0)
		database.ModifyGrid(c, false,atoi(sep->arg[2]),atoi(sep->arg[3]), atoi(sep->arg[4]),zone->GetZoneID());
	else if (strcasecmp("delete", sep->arg[1]) == 0)
		database.ModifyGrid(c, true,atoi(sep->arg[2]),0,0,zone->GetZoneID());
	else {
		c->Message(0,"Usage: #grid add/delete grid_num wandertype pausetype");
		c->Message(0,"Usage: #grid max - displays the highest grid ID used in this zone (for add)");
	}
}

void command_wp(Client *c, const Seperator *sep)
{
	int wp = atoi(sep->arg[4]);

	if (strcasecmp("add", sep->arg[1]) == 0) {
		if (wp == 0) //default to highest if it's left blank, or we enter 0
			wp = database.GetHighestWaypoint(zone->GetZoneID(), atoi(sep->arg[2])) + 1;
		if (strcasecmp("-h", sep->arg[5]) == 0) {
			database.AddWP(c, atoi(sep->arg[2]),wp, c->GetPosition(), atoi(sep->arg[3]),zone->GetZoneID());
		}
		else {
            auto position = c->GetPosition();
            position.w = -1;
			database.AddWP(c, atoi(sep->arg[2]),wp, position, atoi(sep->arg[3]),zone->GetZoneID());
		}
	}
	else if (strcasecmp("delete", sep->arg[1]) == 0)
		database.DeleteWaypoint(c, atoi(sep->arg[2]),wp,zone->GetZoneID());
	else
		c->Message(0,"Usage: #wp add/delete grid_num pause wp_num [-h]");
}

void command_iplookup(Client *c, const Seperator *sep)
{
	auto pack =
	    new ServerPacket(ServerOP_IPLookup, sizeof(ServerGenericWorldQuery_Struct) + strlen(sep->argplus[1]) + 1);
	ServerGenericWorldQuery_Struct* s = (ServerGenericWorldQuery_Struct *) pack->pBuffer;
	strcpy(s->from, c->GetName());
	s->admin = c->Admin();
	if (sep->argplus[1][0] != 0)
		strcpy(s->query, sep->argplus[1]);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void command_size(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	if (!sep->IsNumber(1))
		c->Message(0, "Usage: #size [0 - 255] (Decimal increments are allowed)");
	else {
		float newsize = atof(sep->arg[1]);
		if (newsize > 255)
			c->Message(0, "Error: #size: Size can not be greater than 255.");
		else if (newsize < 0)
			c->Message(0, "Error: #size: Size can not be less than 0.");
		else if (!target)
			c->Message(0,"Error: this command requires a target");
		else {
			uint16 Race = target->GetRace();
			uint8 Gender = target->GetGender();
			uint8 Texture = 0xFF;
			uint8 HelmTexture = 0xFF;
			uint8 HairColor = target->GetHairColor();
			uint8 BeardColor = target->GetBeardColor();
			uint8 EyeColor1 = target->GetEyeColor1();
			uint8 EyeColor2 = target->GetEyeColor2();
			uint8 HairStyle = target->GetHairStyle();
			uint8 LuclinFace = target->GetLuclinFace();
			uint8 Beard = target->GetBeard();
			uint32 DrakkinHeritage = target->GetDrakkinHeritage();
			uint32 DrakkinTattoo = target->GetDrakkinTattoo();
			uint32 DrakkinDetails = target->GetDrakkinDetails();

			target->SendIllusionPacket(Race, Gender, Texture, HelmTexture, HairColor, BeardColor,
										EyeColor1, EyeColor2, HairStyle, LuclinFace, Beard, 0xFF,
										DrakkinHeritage, DrakkinTattoo, DrakkinDetails, newsize);

			c->Message(0,"Size = %f",  atof(sep->arg[1]));
		}
	}
}

void command_mana(Client *c, const Seperator *sep)
{
	Mob *t;

	t = c->GetTarget() ? c->GetTarget() : c;

	if(t->IsClient())
		t->CastToClient()->SetMana(t->CastToClient()->CalcMaxMana());
	else
		t->SetMana(t->CalcMaxMana());
}

void command_flymode(Client *c, const Seperator *sep)
{
	Client *t=c;

	if (strlen(sep->arg[1]) == 1 && !(sep->arg[1][0] == '0' || sep->arg[1][0] == '1' || sep->arg[1][0] == '2'))
		c->Message(0, "#flymode [0/1/2]");
	else {
		if(c->GetTarget() && c->GetTarget()->IsClient())
			t=c->GetTarget()->CastToClient();
		t->SendAppearancePacket(AT_Levitate, atoi(sep->arg[1]));
		if (sep->arg[1][0] == '1')
			c->Message(0, "Turning %s's Flymode ON",  t->GetName());
		else if (sep->arg[1][0] == '2')
			c->Message(0, "Turning %s's Flymode LEV",  t->GetName());
		else
			c->Message(0, "Turning %s's Flymode OFF",  t->GetName());
	}
}

void command_showskills(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	c->Message(0, "Skills for %s",  t->GetName());
	for (EQEmu::skills::SkillType i = EQEmu::skills::Skill1HBlunt; i <= EQEmu::skills::HIGHEST_SKILL; i = (EQEmu::skills::SkillType)(i + 1))
		c->Message(0, "Skill [%d] is at [%d] - %u",  i, t->GetSkill(i), t->GetRawSkill(i));
}

void command_findspell(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0)
		c->Message(0, "Usage: #FindSpell [spellname]");
	else if (SPDAT_RECORDS <= 0)
		c->Message(0, "Spells not loaded");
	else if (Seperator::IsNumber(sep->argplus[1])) {
		int spellid = atoi(sep->argplus[1]);
		if (spellid <= 0 || spellid >= SPDAT_RECORDS) {
			c->Message(0, "Error: Number out of range");
		}
		else {
			c->Message(0, "  %i: %s",  spellid, spells[spellid].name);
		}
	}
	else {
		int count=0;
		//int iSearchLen = strlen(sep->argplus[1])+1;
		char sName[64];
		char sCriteria[65];
		strn0cpy(sCriteria, sep->argplus[1], 64);
		strupr(sCriteria);
		for (int i=0; i<SPDAT_RECORDS; i++) {
			if (spells[i].name[0] != 0) {
				strcpy(sName, spells[i].name);

				strupr(sName);
				char* pdest = strstr(sName, sCriteria);
				if ((pdest != nullptr) && (count <=20)) {
					c->Message(0, "  %i: %s",  i, spells[i].name);
					count++;
				}
				else if (count > 20)
					break;
			}
		}
		if (count > 20)
			c->Message(0, "20 spells found... max reached.");
		else
			c->Message(0, "%i spells found.",  count);
	}
}

void command_castspell(Client *c, const Seperator *sep)
{
	if (!sep->IsNumber(1))
		c->Message(0, "Usage: #CastSpell spellid");
	else {
		uint16 spellid = atoi(sep->arg[1]);
		/*
		Spell restrictions.
		*/
		if (((spellid == 2859) || (spellid == 841) || (spellid == 300) || (spellid == 2314) ||
			(spellid == 3716) || (spellid == 911) || (spellid == 3014) || (spellid == 982) ||
			(spellid == 905) || (spellid == 2079) || (spellid == 1218) || (spellid == 819) ||
			((spellid >= 780) && (spellid <= 785)) || ((spellid >= 1200) && (spellid <= 1205)) ||
			((spellid >= 1342) && (spellid <= 1348)) || (spellid == 1923) || (spellid == 1924) ||
			(spellid == 3355)) &&
			c->Admin() < commandCastSpecials)
			c->Message(13, "Unable to cast spell.");
		else if (spellid >= SPDAT_RECORDS)
			c->Message(0, "Error: #CastSpell: Argument out of range");
		else
			if (c->GetTarget() == 0)
				if(c->Admin() >= commandInstacast)
					c->SpellFinished(spellid, 0, EQEmu::CastingSlot::Item, 0, -1, spells[spellid].ResistDiff);
				else
					c->CastSpell(spellid, 0, EQEmu::CastingSlot::Item, 0);
			else
				if(c->Admin() >= commandInstacast)
					c->SpellFinished(spellid, c->GetTarget(), EQEmu::CastingSlot::Item, 0, -1, spells[spellid].ResistDiff);
				else
					c->CastSpell(spellid, c->GetTarget()->GetID(), EQEmu::CastingSlot::Item, 0);
	}
}

void command_setlanguage(Client *c, const Seperator *sep)
{
	if (strcasecmp(sep->arg[1], "list" ) == 0 )
	{
		c->Message(0, "Languages:");
		c->Message(0, "(0) Common Tongue");
		c->Message(0, "(1) Barbarian");
		c->Message(0, "(2) Erudian");
		c->Message(0, "(3) Elvish");
		c->Message(0, "(4) Dark Elvish");
		c->Message(0, "(5) Dwarvish");
		c->Message(0, "(6) Troll");
		c->Message(0, "(7) Ogre");
		c->Message(0, "(8) Gnomish");
		c->Message(0, "(9) Halfling");
		c->Message(0, "(10) Thieves Cant");
		c->Message(0, "(11) Old Erudian");
		c->Message(0, "(12) Elder Elvish");
		c->Message(0, "(13) Froglok");
		c->Message(0, "(14) Goblin");
		c->Message(0, "(15) Gnoll");
		c->Message(0, "(16) Combine Tongue");
		c->Message(0, "(17) Elder Teir`Dal");
		c->Message(0, "(18) Lizardman");
		c->Message(0, "(19) Orcish");
		c->Message(0, "(20) Faerie");
		c->Message(0, "(21) Dragon");
		c->Message(0, "(22) Elder Dragon");
		c->Message(0, "(23) Dark Speech");
		c->Message(0, "(24) Vah Shir");
		c->Message(0, "(25) Alaran");
		c->Message(0, "(26) Hadal");
		c->Message(0, "(27) Unknown1");
	}
	else if( c->GetTarget() == 0 )
	{
		c->Message(0, "Error: #setlanguage: No target.");
	}
	else if( !c->GetTarget()->IsClient() )
	{
		c->Message(0, "Error: Target must be a player.");
	}
	else if (
				!sep->IsNumber(1) || atoi(sep->arg[1]) < 0 || atoi(sep->arg[1]) > 27 ||
				!sep->IsNumber(2) || atoi(sep->arg[2]) < 0 || atoi(sep->arg[2]) > 100
			)
	{
		c->Message(0, "Usage: #setlanguage [language ID] [value] (0-27, 0-100)");
		c->Message(0, "Try #setlanguage list for a list of language IDs");
	}
	else
	{
		Log(Logs::General, Logs::Normal, "Set language request from %s, target:%s lang_id:%i value:%i",  c->GetName(), c->GetTarget()->GetName(), atoi(sep->arg[1]), atoi(sep->arg[2]) );
		uint8 langid = (uint8)atoi(sep->arg[1]);
		uint8 value = (uint8)atoi(sep->arg[2]);
		c->GetTarget()->CastToClient()->SetLanguageSkill( langid, value );
	}
}

void command_setskill(Client *c, const Seperator *sep)
{
	if (c->GetTarget() == nullptr) {
		c->Message(0, "Error: #setskill: No target.");
	}
	else if (!c->GetTarget()->IsClient()) {
		c->Message(0, "Error: #setskill: Target must be a client.");
	}
	else if (
		!sep->IsNumber(1) || atoi(sep->arg[1]) < 0 || atoi(sep->arg[1]) > EQEmu::skills::HIGHEST_SKILL ||
						!sep->IsNumber(2) || atoi(sep->arg[2]) < 0 || atoi(sep->arg[2]) > HIGHEST_CAN_SET_SKILL
					)
	{
		c->Message(0, "Usage: #setskill skill x ");
		c->Message(0, "       skill = 0 to %d", EQEmu::skills::HIGHEST_SKILL);
		c->Message(0, "       x = 0 to %d",  HIGHEST_CAN_SET_SKILL);
	}
	else {
		Log(Logs::General, Logs::Normal, "Set skill request from %s, target:%s skill_id:%i value:%i",  c->GetName(), c->GetTarget()->GetName(), atoi(sep->arg[1]), atoi(sep->arg[2]) );
		int skill_num = atoi(sep->arg[1]);
		uint16 skill_value = atoi(sep->arg[2]);
		if (skill_num <= EQEmu::skills::HIGHEST_SKILL)
			c->GetTarget()->CastToClient()->SetSkill((EQEmu::skills::SkillType)skill_num, skill_value);
	}
}

void command_setskillall(Client *c, const Seperator *sep)
{
	if (c->GetTarget() == 0)
		c->Message(0, "Error: #setallskill: No target.");
	else if (!c->GetTarget()->IsClient())
		c->Message(0, "Error: #setskill: Target must be a client.");
	else if (!sep->IsNumber(1) || atoi(sep->arg[1]) < 0 || atoi(sep->arg[1]) > HIGHEST_CAN_SET_SKILL) {
		c->Message(0, "Usage: #setskillall value ");
		c->Message(0, "       value = 0 to %d",  HIGHEST_CAN_SET_SKILL);
	}
	else {
		if (c->Admin() >= commandSetSkillsOther || c->GetTarget()==c || c->GetTarget()==0) {
			Log(Logs::General, Logs::Normal, "Set ALL skill request from %s, target:%s",  c->GetName(), c->GetTarget()->GetName());
			uint16 level = atoi(sep->arg[1]);
			for (EQEmu::skills::SkillType skill_num = EQEmu::skills::Skill1HBlunt; skill_num <= EQEmu::skills::HIGHEST_SKILL; skill_num = (EQEmu::skills::SkillType)(skill_num + 1)) {
				c->GetTarget()->CastToClient()->SetSkill(skill_num, level);
			}
		}
		else
			c->Message(0, "Error: Your status is not high enough to set anothers skills");
	}
}

void command_race(Client *c, const Seperator *sep)
{
	Mob *t=c->CastToMob();

	// Need to figure out max race for LoY/LDoN: going with upper bound of 500 now for testing
	if (sep->IsNumber(1) && atoi(sep->arg[1]) >= 0 && atoi(sep->arg[1]) <= 724) {
		if ((c->GetTarget()) && c->Admin() >= commandRaceOthers)
			t=c->GetTarget();
		t->SendIllusionPacket(atoi(sep->arg[1]));
	}
	else
		c->Message(0, "Usage: #race [0-724] (0 for back to normal)");
}

void command_gender(Client *c, const Seperator *sep)
{
	Mob *t=c->CastToMob();

	if (sep->IsNumber(1) && atoi(sep->arg[1]) >= 0 && atoi(sep->arg[1]) <= 500) {
		if ((c->GetTarget()) && c->Admin() >= commandGenderOthers)
			t=c->GetTarget();
		t->SendIllusionPacket(t->GetRace(), atoi(sep->arg[1]));
	}
	else
		c->Message(0, "Usage: #gender [0/1/2]");
}

void command_makepet(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == '\0')
		c->Message(0, "Usage: #makepet pet_type_name (will not survive across zones)");
	else
		c->MakePet(0, sep->arg[1]);
}

void command_level(Client *c, const Seperator *sep)
{
	uint16 level = atoi(sep->arg[1]);

	if ((level <= 0) || ((level > RuleI(Character, MaxLevel)) && (c->Admin() < commandLevelAboveCap))) {
		c->Message(0, "Error: #Level: Invalid Level");
	}
	else if (c->Admin() < 100) {
		c->SetLevel(level, true);
#ifdef BOTS
		if(RuleB(Bots, BotLevelsWithOwner))
			Bot::LevelBotWithClient(c, level, true);
#endif
	}
	else if (!c->GetTarget()) {
		c->Message(0, "Error: #Level: No target");
	}
	else {
		if (!c->GetTarget()->IsNPC() && ((c->Admin() < commandLevelNPCAboveCap) && (level > RuleI(Character, MaxLevel)))) {
			c->Message(0, "Error: #Level: Invalid Level");
		}
		else {
			c->GetTarget()->SetLevel(level, true);
			if(c->GetTarget()->IsClient()) {
				c->GetTarget()->CastToClient()->SendLevelAppearance();
#ifdef BOTS
				if(RuleB(Bots, BotLevelsWithOwner))
					Bot::LevelBotWithClient(c->GetTarget()->CastToClient(), level, true);
#endif
			}
		}
	}
}

void command_spawn(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] != 0){
		Client* client = entity_list.GetClientByName(sep->arg[1]);
		if(client){
				c->Message(0,"You cannot spawn a mob with the same name as a character!");
				return;
		}
	}

	NPC* npc = NPC::SpawnNPC(sep->argplus[1], c->GetPosition(), c);
	if (!npc) {
		c->Message(0, "Format: #spawn name race level material hp gender class priweapon secweapon merchantid bodytype - spawns a npc those parameters.");
		c->Message(0, "Name Format: NPCFirstname_NPCLastname - All numbers in a name are stripped and \"_\" characters become a space.");
		c->Message(0, "Note: Using \"-\" for gender will autoselect the gender for the race. Using \"-\" for HP will use the calculated maximum HP.");
	}
}

void command_texture(Client *c, const Seperator *sep)
{

	uint16 texture;
	if (sep->IsNumber(1) && atoi(sep->arg[1]) >= 0 && atoi(sep->arg[1]) <= 255) {
		texture = atoi(sep->arg[1]);
		uint8 helm = 0xFF;

		// Player Races Wear Armor, so Wearchange is sent instead
		int i;
		if (!c->GetTarget())
			for (i = EQEmu::textures::textureBegin; i <= EQEmu::textures::LastTintableTexture; i++)
			{
				c->SendTextureWC(i, texture);
			}
		else if ((c->GetTarget()->GetRace() > 0 && c->GetTarget()->GetRace() <= 12) ||
			c->GetTarget()->GetRace() == 128 || c->GetTarget()->GetRace() == 130 ||
			c->GetTarget()->GetRace() == 330 || c->GetTarget()->GetRace() == 522) {
			for (i = EQEmu::textures::textureBegin; i <= EQEmu::textures::LastTintableTexture; i++)
			{
				c->GetTarget()->SendTextureWC(i, texture);
			}
		}
		else	// Non-Player Races only need Illusion Packets to be sent for texture
		{
			if (sep->IsNumber(2) && atoi(sep->arg[2]) >= 0 && atoi(sep->arg[2]) <= 255)
				helm = atoi(sep->arg[2]);
			else
				helm = texture;

			if (texture == 255) {
				texture = 0xFFFF;	// Should be pulling these from the database instead
				helm = 0xFF;
			}

			if ((c->GetTarget()) && (c->Admin() >= commandTextureOthers))
				c->GetTarget()->SendIllusionPacket(c->GetTarget()->GetRace(), 0xFF, texture, helm);
			else
				c->SendIllusionPacket(c->GetRace(), 0xFF, texture, helm);
		}
	}
	else
		c->Message(0, "Usage: #texture [texture] [helmtexture] (0-255, 255 for show equipment)");
}

void command_npctypespawn(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(1)) {
		const NPCType* tmp = 0;
		if ((tmp = database.LoadNPCTypesData(atoi(sep->arg[1])))) {
			//tmp->fixedZ = 1;
			auto npc = new NPC(tmp, 0, c->GetPosition(), FlyMode3);
			if (npc && sep->IsNumber(2))
				npc->SetNPCFactionID(atoi(sep->arg[2]));

				npc->AddLootTable();
			entity_list.AddNPC(npc);
		}
		else
			c->Message(0, "NPC Type %i not found",  atoi(sep->arg[1]));
	}
	else
		c->Message(0, "Usage: #npctypespawn npctypeid factionid");

}

void command_heal(Client *c, const Seperator *sep)
{
	if (c->GetTarget()==0)
		c->Message(0, "Error: #Heal: No Target.");
	else
		c->GetTarget()->Heal();
}

void command_appearance(Client *c, const Seperator *sep)
{
	Mob *t=c->CastToMob();

	// sends any appearance packet
	// Dev debug command, for appearance types
	if (sep->arg[2][0] == 0)
		c->Message(0, "Usage: #appearance type value");
	else {
		if ((c->GetTarget()))
			t=c->GetTarget();
		t->SendAppearancePacket(atoi(sep->arg[1]), atoi(sep->arg[2]));
		c->Message(0, "Sending appearance packet: target=%s, type=%s, value=%s",  t->GetName(), sep->arg[1], sep->arg[2]);
	}
}

void command_nukeitem(Client *c, const Seperator *sep)
{
	int numitems, itemid;

	if (c->GetTarget() && c->GetTarget()->IsClient() && (sep->IsNumber(1) || sep->IsHexNumber(1))) {
		itemid=sep->IsNumber(1)?atoi(sep->arg[1]):hextoi(sep->arg[1]);
		numitems = c->GetTarget()->CastToClient()->NukeItem(itemid);
		c->Message(0, " %u items deleted",  numitems);
	}
	else
		c->Message(0, "Usage: (targted) #nukeitem itemnum - removes the item from the player's inventory");
}

void command_peekinv(Client *c, const Seperator *sep)
{
	enum {
		peekWorn = 0x01,
		peekInv = 0x02,
		peekCursor = 0x04,
		peekTrib = 0x08,
		peekBank = 0x10,
		peekTrade = 0x20,
		peekWorld = 0x40
	} ;

	if (!c->GetTarget() || !c->GetTarget()->IsClient()) {
		c->Message(0, "You must have a PC target selected for this command");
		return;
	}

	int scopeWhere = 0;

	if (strcasecmp(sep->arg[1], "all") == 0) { scopeWhere = ~0; }
	else if (strcasecmp(sep->arg[1], "worn") == 0) { scopeWhere |= peekWorn; }
	else if (strcasecmp(sep->arg[1], "inv") == 0) { scopeWhere |= peekInv; }
	else if (strcasecmp(sep->arg[1], "cursor") == 0) { scopeWhere |= peekCursor; }
	else if (strcasecmp(sep->arg[1], "trib") == 0) { scopeWhere |= peekTrib; }
	else if (strcasecmp(sep->arg[1], "bank") == 0) { scopeWhere |= peekBank; }
	else if (strcasecmp(sep->arg[1], "trade") == 0) { scopeWhere |= peekTrade; }
	else if (strcasecmp(sep->arg[1], "world") == 0) { scopeWhere |= peekWorld; }

	if (scopeWhere == 0) {
		c->Message(0, "Usage: #peekinv [worn|inv|cursor|trib|bank|trade|world|all]");
		c->Message(0, "  Displays a portion of the targeted user's inventory");
		c->Message(0, "  Caution: 'all' is a lot of information!");
		return;
	}

	Client* targetClient = c->GetTarget()->CastToClient();
	const EQEmu::ItemInstance* inst_main = nullptr;
	const EQEmu::ItemInstance* inst_sub = nullptr;
	const EQEmu::ItemData* item_data = nullptr;
	std::string item_link;
	EQEmu::SayLinkEngine linker;
	linker.SetLinkType(EQEmu::saylink::SayLinkItemInst);

	c->Message(0, "Displaying inventory for %s...",  targetClient->GetName());

	// worn
	for (int16 indexMain = EQEmu::legacy::EQUIPMENT_BEGIN; (scopeWhere & peekWorn) && (indexMain <= EQEmu::legacy::EQUIPMENT_END); ++indexMain) {
		inst_main = targetClient->GetInv().GetItem(indexMain);
		item_data = (inst_main == nullptr) ? nullptr : inst_main->GetItem();
		linker.SetItemInst(inst_main);

		item_link = linker.GenerateLink();

		c->Message((item_data == nullptr), "WornSlot: %i, Item: %i (%s), Charges: %i",
			indexMain, ((item_data == nullptr) ? 0 : item_data->ID), item_link.c_str(), ((inst_main == nullptr) ? 0 : inst_main->GetCharges()));
	}

	if ((scopeWhere & peekWorn) && (targetClient->ClientVersion() >= EQEmu::versions::ClientVersion::SoF)) {
		inst_main = targetClient->GetInv().GetItem(EQEmu::inventory::slotPowerSource);
		item_data = (inst_main == nullptr) ? nullptr : inst_main->GetItem();
		linker.SetItemInst(inst_main);

		item_link = linker.GenerateLink();

		c->Message((item_data == nullptr), "WornSlot: %i, Item: %i (%s), Charges: %i",
			EQEmu::inventory::slotPowerSource, ((item_data == nullptr) ? 0 : item_data->ID), item_link.c_str(), ((inst_main == nullptr) ? 0 : inst_main->GetCharges()));
	}

	// inv
	for (int16 indexMain = EQEmu::legacy::GENERAL_BEGIN; (scopeWhere & peekInv) && (indexMain <= EQEmu::legacy::GENERAL_END); ++indexMain) {
		inst_main = targetClient->GetInv().GetItem(indexMain);
		item_data = (inst_main == nullptr) ? nullptr : inst_main->GetItem();
		linker.SetItemInst(inst_main);

		item_link = linker.GenerateLink();

		c->Message((item_data == nullptr), "InvSlot: %i, Item: %i (%s), Charges: %i",
			indexMain, ((item_data == nullptr) ? 0 : item_data->ID), item_link.c_str(), ((inst_main == nullptr) ? 0 : inst_main->GetCharges()));

		for (uint8 indexSub = EQEmu::inventory::containerBegin; inst_main && inst_main->IsClassBag() && (indexSub < EQEmu::inventory::ContainerCount); ++indexSub) {
			inst_sub = inst_main->GetItem(indexSub);
			item_data = (inst_sub == nullptr) ? nullptr : inst_sub->GetItem();
			linker.SetItemInst(inst_sub);

			item_link = linker.GenerateLink();

			c->Message((item_data == nullptr), "  InvBagSlot: %i (Slot #%i, Bag #%i), Item: %i (%s), Charges: %i",
				EQEmu::InventoryProfile::CalcSlotId(indexMain, indexSub), indexMain, indexSub, ((item_data == nullptr) ? 0 : item_data->ID), item_link.c_str(), ((inst_sub == nullptr) ? 0 : inst_sub->GetCharges()));
		}
	}

	// cursor
	if (scopeWhere & peekCursor) {
		if (targetClient->GetInv().CursorEmpty()) {
			linker.SetItemInst(nullptr);

			item_link = linker.GenerateLink();

			c->Message(1, "CursorSlot: %i, Item: %i (%s), Charges: %i",
				EQEmu::inventory::slotCursor, 0, item_link.c_str(), 0);
		}
		else {
			int cursorDepth = 0;
			for (auto it = targetClient->GetInv().cursor_cbegin(); (it != targetClient->GetInv().cursor_cend()); ++it, ++cursorDepth) {
				inst_main = *it;
				item_data = (inst_main == nullptr) ? nullptr : inst_main->GetItem();
				linker.SetItemInst(inst_main);

				item_link = linker.GenerateLink();

				c->Message((item_data == nullptr), "CursorSlot: %i, Depth: %i, Item: %i (%s), Charges: %i",
					EQEmu::inventory::slotCursor, cursorDepth, ((item_data == nullptr) ? 0 : item_data->ID), item_link.c_str(), ((inst_main == nullptr) ? 0 : inst_main->GetCharges()));

				for (uint8 indexSub = EQEmu::inventory::containerBegin; (cursorDepth == 0) && inst_main && inst_main->IsClassBag() && (indexSub < EQEmu::inventory::ContainerCount); ++indexSub) {
					inst_sub = inst_main->GetItem(indexSub);
					item_data = (inst_sub == nullptr) ? nullptr : inst_sub->GetItem();
					linker.SetItemInst(inst_sub);

					item_link = linker.GenerateLink();

					c->Message((item_data == nullptr), "  CursorBagSlot: %i (Slot #%i, Bag #%i), Item: %i (%s), Charges: %i",
						EQEmu::InventoryProfile::CalcSlotId(EQEmu::inventory::slotCursor, indexSub), EQEmu::inventory::slotCursor, indexSub, ((item_data == nullptr) ? 0 : item_data->ID), item_link.c_str(), ((inst_sub == nullptr) ? 0 : inst_sub->GetCharges()));
				}
			}
		}
	}

	// trib
	for (int16 indexMain = EQEmu::legacy::TRIBUTE_BEGIN; (scopeWhere & peekTrib) && (indexMain <= EQEmu::legacy::TRIBUTE_END); ++indexMain) {
		inst_main = targetClient->GetInv().GetItem(indexMain);
		item_data = (inst_main == nullptr) ? nullptr : inst_main->GetItem();
		linker.SetItemInst(inst_main);

		item_link = linker.GenerateLink();

		c->Message((item_data == nullptr), "TributeSlot: %i, Item: %i (%s), Charges: %i",
			indexMain, ((item_data == nullptr) ? 0 : item_data->ID), item_link.c_str(), ((inst_main == nullptr) ? 0 : inst_main->GetCharges()));
	}

	// bank
	for (int16 indexMain = EQEmu::legacy::BANK_BEGIN; (scopeWhere & peekBank) && (indexMain <= EQEmu::legacy::BANK_END); ++indexMain) {
		inst_main = targetClient->GetInv().GetItem(indexMain);
		item_data = (inst_main == nullptr) ? nullptr : inst_main->GetItem();
		linker.SetItemInst(inst_main);

		item_link = linker.GenerateLink();

		c->Message((item_data == nullptr), "BankSlot: %i, Item: %i (%s), Charges: %i",
			indexMain, ((item_data == nullptr) ? 0 : item_data->ID), item_link.c_str(), ((inst_main == nullptr) ? 0 : inst_main->GetCharges()));

		for (uint8 indexSub = EQEmu::inventory::containerBegin; inst_main && inst_main->IsClassBag() && (indexSub < EQEmu::inventory::ContainerCount); ++indexSub) {
			inst_sub = inst_main->GetItem(indexSub);
			item_data = (inst_sub == nullptr) ? nullptr : inst_sub->GetItem();
			linker.SetItemInst(inst_sub);

			item_link = linker.GenerateLink();

			c->Message((item_data == nullptr), "  BankBagSlot: %i (Slot #%i, Bag #%i), Item: %i (%s), Charges: %i",
				EQEmu::InventoryProfile::CalcSlotId(indexMain, indexSub), indexMain, indexSub, ((item_data == nullptr) ? 0 : item_data->ID), item_link.c_str(), ((inst_sub == nullptr) ? 0 : inst_sub->GetCharges()));
		}
	}

	for (int16 indexMain = EQEmu::legacy::SHARED_BANK_BEGIN; (scopeWhere & peekBank) && (indexMain <= EQEmu::legacy::SHARED_BANK_END); ++indexMain) {
		inst_main = targetClient->GetInv().GetItem(indexMain);
		item_data = (inst_main == nullptr) ? nullptr : inst_main->GetItem();
		linker.SetItemInst(inst_main);

		item_link = linker.GenerateLink();

		c->Message((item_data == nullptr), "SharedBankSlot: %i, Item: %i (%s), Charges: %i",
			indexMain, ((item_data == nullptr) ? 0 : item_data->ID), item_link.c_str(), ((inst_main == nullptr) ? 0 : inst_main->GetCharges()));

		for (uint8 indexSub = EQEmu::inventory::containerBegin; inst_main && inst_main->IsClassBag() && (indexSub < EQEmu::inventory::ContainerCount); ++indexSub) {
			inst_sub = inst_main->GetItem(indexSub);
			item_data = (inst_sub == nullptr) ? nullptr : inst_sub->GetItem();
			linker.SetItemInst(inst_sub);

			item_link = linker.GenerateLink();

			c->Message((item_data == nullptr), "  SharedBankBagSlot: %i (Slot #%i, Bag #%i), Item: %i (%s), Charges: %i",
				EQEmu::InventoryProfile::CalcSlotId(indexMain, indexSub), indexMain, indexSub, ((item_data == nullptr) ? 0 : item_data->ID), item_link.c_str(), ((inst_sub == nullptr) ? 0 : inst_sub->GetCharges()));
		}
	}

	// trade
	for (int16 indexMain = EQEmu::legacy::TRADE_BEGIN; (scopeWhere & peekTrade) && (indexMain <= EQEmu::legacy::TRADE_END); ++indexMain) {
		inst_main = targetClient->GetInv().GetItem(indexMain);
		item_data = (inst_main == nullptr) ? nullptr : inst_main->GetItem();
		linker.SetItemInst(inst_main);

		item_link = linker.GenerateLink();

		c->Message((item_data == nullptr), "TradeSlot: %i, Item: %i (%s), Charges: %i",
			indexMain, ((item_data == nullptr) ? 0 : item_data->ID), item_link.c_str(), ((inst_main == nullptr) ? 0 : inst_main->GetCharges()));

		for (uint8 indexSub = EQEmu::inventory::containerBegin; inst_main && inst_main->IsClassBag() && (indexSub < EQEmu::inventory::ContainerCount); ++indexSub) {
			inst_sub = inst_main->GetItem(indexSub);
			item_data = (inst_sub == nullptr) ? nullptr : inst_sub->GetItem();
			linker.SetItemInst(inst_sub);

			item_link = linker.GenerateLink();

			c->Message((item_data == nullptr), "  TradeBagSlot: %i (Slot #%i, Bag #%i), Item: %i (%s), Charges: %i",
				EQEmu::InventoryProfile::CalcSlotId(indexMain, indexSub), indexMain, indexSub, ((item_data == nullptr) ? 0 : item_data->ID), item_link.c_str(), ((inst_sub == nullptr) ? 0 : inst_sub->GetCharges()));
		}
	}

	// world
	if (scopeWhere & peekWorld) {
		Object* objectTradeskill = targetClient->GetTradeskillObject();

		if (objectTradeskill == nullptr) {
			c->Message(1, "No world tradeskill object selected...");
		}
		else {
			c->Message(0, "[WorldObject DBID: %i (entityid: %i)]",  objectTradeskill->GetDBID(), objectTradeskill->GetID());

			for (int16 indexMain = EQEmu::inventory::slotBegin; indexMain < EQEmu::legacy::TYPE_WORLD_SIZE; ++indexMain) {
				inst_main = objectTradeskill->GetItem(indexMain);
				item_data = (inst_main == nullptr) ? nullptr : inst_main->GetItem();
				linker.SetItemInst(inst_main);

				item_link = linker.GenerateLink();

				c->Message((item_data == nullptr), "WorldSlot: %i, Item: %i (%s), Charges: %i",
					(EQEmu::legacy::WORLD_BEGIN + indexMain), ((item_data == nullptr) ? 0 : item_data->ID), item_link.c_str(), ((inst_main == nullptr) ? 0 : inst_main->GetCharges()));

				for (uint8 indexSub = EQEmu::inventory::containerBegin; inst_main && inst_main->IsType(EQEmu::item::ItemClassBag) && (indexSub < EQEmu::inventory::ContainerCount); ++indexSub) {
					inst_sub = inst_main->GetItem(indexSub);
					item_data = (inst_sub == nullptr) ? nullptr : inst_sub->GetItem();
					linker.SetItemInst(inst_sub);

					item_link = linker.GenerateLink();

					c->Message((item_data == nullptr), "  WorldBagSlot: %i (Slot #%i, Bag #%i), Item: %i (%s), Charges: %i",
						INVALID_INDEX, indexMain, indexSub, ((item_data == nullptr) ? 0 : item_data->ID), item_link.c_str(), ((inst_sub == nullptr) ? 0 : inst_sub->GetCharges()));
				}
			}
		}
	}
}

void command_interrogateinv(Client *c, const Seperator *sep)
{
	// 'command_interrogateinv' is an in-memory inventory interrogation tool only.
	//
	// it does not verify against actual database entries..but, the output can be
	// used to verify that something has been corrupted in a player's inventory.
	// any error condition should be assumed that the item in question will be
	// lost when the player logs out or zones (or incurrs any action that will
	// consume the Client-Inventory object instance in question.)
	//
	// any item instances located at a greater depth than a reported error should
	// be treated as an error themselves regardless of whether they report as the
	// same or not.

	if (strcasecmp(sep->arg[1], "help") == 0) {
		if (c->Admin() < commandInterrogateInv) {
			c->Message(0, "Usage: #interrogateinv");
			c->Message(0, "  Displays your inventory's current in-memory nested storage references");
		}
		else {
			c->Message(0, "Usage: #interrogateinv [log] [silent]");
			c->Message(0, "  Displays your or your Player target inventory's current in-memory nested storage references");
			c->Message(0, "  [log] - Logs interrogation to file");
			c->Message(0, "  [silent] - Omits the in-game message portion of the interrogation");
		}
		return;
	}

	Client* target = nullptr;
	std::map<int16, const EQEmu::ItemInstance*> instmap;
	bool log = false;
	bool silent = false;
	bool error = false;
	bool allowtrip = false;

	if (c->Admin() < commandInterrogateInv) {
		if (c->GetInterrogateInvState()) {
			c->Message(13, "The last use of #interrogateinv on this inventory instance discovered an error...");
			c->Message(13, "Logging out, zoning or re-arranging items at this point will result in item loss!");
			return;
		}
		target = c;
		allowtrip = true;
	}
	else {
		if (c->GetTarget() == nullptr) {
			target = c;
		}
		else if (c->GetTarget()->IsClient()) {
			target = c->GetTarget()->CastToClient();
		}
		else {
			c->Message(1, "Use of this command is limited to Client entities");
			return;
		}

		if (strcasecmp(sep->arg[1], "log") == 0)
			log = true;
		if (strcasecmp(sep->arg[2], "silent") == 0)
			silent = true;
	}

	bool success = target->InterrogateInventory(c, log, silent, allowtrip, error);

	if (!success)
		c->Message(13, "An unknown error occurred while processing Client::InterrogateInventory()");
}

void command_invsnapshot(Client *c, const Seperator *sep)
{
	auto t = c->GetTarget();
	if (!t || !t->IsClient()) {
		c->Message(0, "Target must be a client");
		return;
	}

	if (database.SaveCharacterInventorySnapshot(((Client*)t)->CharacterID())) {
		c->SetNextInvSnapshot(RuleI(Character, InvSnapshotMinIntervalM));
		c->Message(0, "Successful inventory snapshot taken of %s", t->GetName());
	}
	else {
		c->SetNextInvSnapshot(RuleI(Character, InvSnapshotMinRetryM));
		c->Message(0, "Failed to take inventory snapshot of %s", t->GetName());
	}
}

void command_clearinvsnapshots(Client *c, const Seperator *sep)
{
	if (strcmp(sep->arg[1], "false") == 0) {
		database.ClearInvSnapshots(false);
		c->Message(0, "Inventory snapshots cleared using current time");
	}
	else {
		database.ClearInvSnapshots();
		c->Message(0, "Inventory snapshots cleared using RuleI(Character, InvSnapshotHistoryD) (%i days)", RuleI(Character, InvSnapshotHistoryD));
	}
}

void command_findnpctype(Client *c, const Seperator *sep)
{
	if(sep->arg[1][0] == 0) {
		c->Message(0, "Usage: #findnpctype [search criteria]");
		return;
    }

	std::string query;

	int id = atoi((const char *)sep->arg[1]);
	if (id == 0) // If id evaluates to 0, then search as if user entered a string.
		query = StringFormat("SELECT id, name FROM npc_types WHERE name LIKE '%%%s%%'",  sep->arg[1]);
	else // Otherwise, look for just that npc id.
		query = StringFormat("SELECT id, name FROM npc_types WHERE id = %i",  id);

    auto results = database.QueryDatabase(query);
    if (!results.Success()) {
        c->Message (0, "Error querying database.");
		c->Message (0, query.c_str());
    }

    if (results.RowCount() == 0) // No matches found.
        c->Message (0, "No matches found for %s.",  sep->arg[1]);

    // If query runs successfully.
	int count = 0;
    const int maxrows = 20;

    // Process each row returned.
	for (auto row = results.begin(); row != results.end(); ++row) {
		// Limit to returning maxrows rows.
        if (++count > maxrows) {
            c->Message (0, "%i npc types shown. Too many results.",  maxrows);
            break;
        }

        c->Message (0, "  %s: %s",  row[0], row[1]);
    }

    // If we did not hit the maxrows limit.
    if (count <= maxrows)
        c->Message (0, "Query complete. %i rows shown.",  count);

}

void command_findzone(Client *c, const Seperator *sep)
{
	if(sep->arg[1][0] == 0) {
		c->Message(0, "Usage: #findzone [search criteria]");
        return;
    }

    std::string query;
    int id = atoi((const char *)sep->arg[1]);
    if (id == 0) { // If id evaluates to 0, then search as if user entered a string.
	    auto escName = new char[strlen(sep->arg[1]) * 2 + 1];
	    database.DoEscapeString(escName, sep->arg[1], strlen(sep->arg[1]));

	    query = StringFormat("SELECT zoneidnumber, short_name, long_name FROM zone "
				 "WHERE long_name RLIKE '%s' AND version = 0",
				 escName);
	    safe_delete_array(escName);
    }
    else // Otherwise, look for just that zoneidnumber.
		query = StringFormat("SELECT zoneidnumber, short_name, long_name FROM zone "
                            "WHERE zoneidnumber = %i AND version = 0",  id);

    auto results = database.QueryDatabase(query);
    if (!results.Success()) {
        c->Message (0, "Error querying database.");
        c->Message (0, query.c_str());
        return;
    }

    int count = 0;
    const int maxrows = 20;

    for(auto row = results.begin(); row != results.end(); ++row) {
        if (++count > maxrows) {
            c->Message (0, "%i zones shown. Too many results.",  maxrows);
            break;
        }

        c->Message (0, "  %s: %s, %s",  row[0], row[1], row[2]);
    }

    if (count <= maxrows)
        c->Message (0, "Query complete. %i rows shown.",  count);
    else if (count == 0)
        c->Message (0, "No matches found for %s.",  sep->arg[1]);
}

void command_viewnpctype(Client *c, const Seperator *sep)
{
	if (!sep->IsNumber(1))
		c->Message(0, "Usage: #viewnpctype [npctype id]");
	else
	{
		uint32 npctypeid=atoi(sep->arg[1]);
		const NPCType* npct = database.LoadNPCTypesData(npctypeid);
		if (npct) {
			c->Message(0, " NPCType Info, ");
			c->Message(0, "  NPCTypeID: %u",  npct->npc_id);
			c->Message(0, "  Name: %s",  npct->name);
			c->Message(0, "  Level: %i",  npct->level);
			c->Message(0, "  Race: %i",  npct->race);
			c->Message(0, "  Class: %i",  npct->class_);
			c->Message(0, "  MinDmg: %i",  npct->min_dmg);
			c->Message(0, "  MaxDmg: %i",  npct->max_dmg);
			c->Message(0, "  Special Abilities: %s",  npct->special_abilities);
			c->Message(0, "  Spells: %i",  npct->npc_spells_id);
			c->Message(0, "  Loot Table: %i",  npct->loottable_id);
			c->Message(0, "  NPCFactionID: %i",  npct->npc_faction_id);
		}
		else
			c->Message(0, "NPC #%d not found",  npctypeid);
	}
}

void command_reloadqst(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0)
	{
		c->Message(0, "Clearing quest memory cache.");
		entity_list.ClearAreas();
		parse->ReloadQuests();
	}
	else
	{
		c->Message(0, "Clearing quest memory cache and stopping timers.");
		entity_list.ClearAreas();
		parse->ReloadQuests(true);
	}

}

void command_corpsefix(Client *c, const Seperator *sep)
{
	entity_list.CorpseFix(c);
}

void command_reloadworld(Client *c, const Seperator *sep)
{
	c->Message(0, "Reloading quest cache and repopping zones worldwide.");
	auto pack = new ServerPacket(ServerOP_ReloadWorld, sizeof(ReloadWorld_Struct));
	ReloadWorld_Struct* RW = (ReloadWorld_Struct*) pack->pBuffer;
	RW->Option = ((atoi(sep->arg[1]) == 1) ? 1 : 0);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void command_reloadlevelmods(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0)
	{
		if(RuleB(Zone, LevelBasedEXPMods)){
			zone->LoadLevelEXPMods();
			c->Message(15, "Level based EXP Mods have been reloaded zonewide");
		}else{
			c->Message(15, "Level based EXP Mods are disabled in rules!");
		}
	}
}

void command_reloadzps(Client *c, const Seperator *sep)
{
	database.LoadStaticZonePoints(&zone->zone_point_list, zone->GetShortName(), zone->GetInstanceVersion());
	c->Message(0, "Reloading server zone_points.");
}

void command_zoneshutdown(Client *c, const Seperator *sep)
{
	if (!worldserver.Connected())
		c->Message(0, "Error: World server disconnected");
	else if (sep->arg[1][0] == 0)
		c->Message(0, "Usage: #zoneshutdown zoneshortname");
	else {
		auto pack = new ServerPacket(ServerOP_ZoneShutdown, sizeof(ServerZoneStateChange_struct));
		ServerZoneStateChange_struct* s = (ServerZoneStateChange_struct *) pack->pBuffer;
		strcpy(s->adminname, c->GetName());
		if (sep->arg[1][0] >= '0' && sep->arg[1][0] <= '9')
			s->ZoneServerID = atoi(sep->arg[1]);
		else
			s->zoneid = database.GetZoneID(sep->arg[1]);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void command_zonebootup(Client *c, const Seperator *sep)
{
	if (!worldserver.Connected())
		c->Message(0, "Error: World server disconnected");
	else if (sep->arg[2][0] == 0) {
		c->Message(0, "Usage: #zonebootup ZoneServerID# zoneshortname");
	}
	else {
		auto pack = new ServerPacket(ServerOP_ZoneBootup, sizeof(ServerZoneStateChange_struct));
		ServerZoneStateChange_struct* s = (ServerZoneStateChange_struct *) pack->pBuffer;
		s->ZoneServerID = atoi(sep->arg[1]);
		strcpy(s->adminname, c->GetName());
		s->zoneid = database.GetZoneID(sep->arg[2]);
		s->makestatic = (bool) (strcasecmp(sep->arg[3], "static") == 0);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void command_kick(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0)
		c->Message(0, "Usage: #kick [charname]");
	else {
		Client* client = entity_list.GetClientByName(sep->arg[1]);
		if (client != 0) {
			if (client->Admin() <= c->Admin()) {
				client->Message(0, "You have been kicked by %s", c->GetName());
				auto outapp = new EQApplicationPacket(OP_GMKick, 0);
				client->QueuePacket(outapp);
				client->Kick();
				c->Message(0, "Kick: local: kicking %s",  sep->arg[1]);
			}
		}
		else if (!worldserver.Connected())
			c->Message(0, "Error: World server disconnected");
		else {
			auto pack = new ServerPacket(ServerOP_KickPlayer, sizeof(ServerKickPlayer_Struct));
			ServerKickPlayer_Struct* skp = (ServerKickPlayer_Struct*) pack->pBuffer;
			strcpy(skp->adminname, c->GetName());
			strcpy(skp->name, sep->arg[1]);
			skp->adminrank = c->Admin();
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
	}
}

void command_attack(Client *c, const Seperator *sep)
{
	if (c->GetTarget() && c->GetTarget()->IsNPC() && sep->arg[1] != 0) {
		Mob* sictar = entity_list.GetMob(sep->argplus[1]);
		if (sictar)
			c->GetTarget()->CastToNPC()->AddToHateList(sictar, 1, 0);
		else
			c->Message(0, "Error: %s not found",  sep->arg[1]);
	}
	else
		c->Message(0, "Usage: (needs NPC targeted) #attack targetname");
}

void command_lock(Client *c, const Seperator *sep)
{
	auto outpack = new ServerPacket(ServerOP_Lock, sizeof(ServerLock_Struct));
	ServerLock_Struct* lss = (ServerLock_Struct*) outpack->pBuffer;
	strcpy(lss->myname, c->GetName());
	lss->mode = 1;
	worldserver.SendPacket(outpack);
	safe_delete(outpack);
}

void command_unlock(Client *c, const Seperator *sep)
{
	auto outpack = new ServerPacket(ServerOP_Lock, sizeof(ServerLock_Struct));
	ServerLock_Struct* lss = (ServerLock_Struct*) outpack->pBuffer;
	strcpy(lss->myname, c->GetName());
	lss->mode = 0;
	worldserver.SendPacket(outpack);
	safe_delete(outpack);
}

void command_motd(Client *c, const Seperator *sep)
{
	auto outpack = new ServerPacket(ServerOP_Motd, sizeof(ServerMotd_Struct));
	ServerMotd_Struct* mss = (ServerMotd_Struct*) outpack->pBuffer;
	strn0cpy(mss->myname, c->GetName(),64);
	strn0cpy(mss->motd, sep->argplus[1],512);
	worldserver.SendPacket(outpack);
	safe_delete(outpack);
}

void command_listpetition(Client *c, const Seperator *sep)
{
	std::string query = "SELECT petid, charname, accountname FROM petitions ORDER BY petid";
	auto results = database.QueryDatabase(query);
	if (!results.Success())
        return;

    Log(Logs::General, Logs::Normal, "Petition list requested by %s",  c->GetName());

    if (results.RowCount() == 0)
        return;

    c->Message(13,"	ID : Character Name , Account Name");

    for (auto row = results.begin(); row != results.end(); ++row)
        c->Message(15, " %s:	%s , %s ", row[0],row[1],row[2]);
}

void command_equipitem(Client *c, const Seperator *sep)
{
	uint32 slot_id = atoi(sep->arg[1]);
	if (sep->IsNumber(1) && ((slot_id >= EQEmu::legacy::EQUIPMENT_BEGIN) && (slot_id <= EQEmu::legacy::EQUIPMENT_END) || (slot_id == EQEmu::inventory::slotPowerSource))) {
		const EQEmu::ItemInstance* from_inst = c->GetInv().GetItem(EQEmu::inventory::slotCursor);
		const EQEmu::ItemInstance* to_inst = c->GetInv().GetItem(slot_id); // added (desync issue when forcing stack to stack)
		bool partialmove = false;
		int16 movecount;

		if (from_inst && from_inst->IsClassCommon()) {
			auto outapp = new EQApplicationPacket(OP_MoveItem, sizeof(MoveItem_Struct));
			MoveItem_Struct* mi	= (MoveItem_Struct*)outapp->pBuffer;
			mi->from_slot = EQEmu::inventory::slotCursor;
			mi->to_slot			= slot_id;
			// mi->number_in_stack	= from_inst->GetCharges(); // replaced with con check for stacking

			// crude stackable check to only 'move' the difference count on client instead of entire stack when applicable
			if (to_inst && to_inst->IsStackable() &&
				(to_inst->GetItem()->ID == from_inst->GetItem()->ID) &&
				(to_inst->GetCharges() < to_inst->GetItem()->StackSize) &&
				(from_inst->GetCharges() > to_inst->GetItem()->StackSize - to_inst->GetCharges())) {
					movecount = to_inst->GetItem()->StackSize - to_inst->GetCharges();
					mi->number_in_stack = (uint32)movecount;
					partialmove = true;
			}
			else
				mi->number_in_stack = from_inst->GetCharges();

			// Save move changes
			// Added conditional check to packet send..would have sent change even on a swap failure..whoops!

			if (partialmove) { // remove this con check if someone can figure out removing charges from cursor stack issue below
				// mi->number_in_stack is always from_inst->GetCharges() when partialmove is false
				c->Message(13, "Error: Partial stack added to existing stack exceeds allowable stacksize");
				safe_delete(outapp);
				return;
			}
			else if(c->SwapItem(mi)) {
				c->FastQueuePacket(&outapp);

				// if the below code is still needed..just send an an item trade packet to each slot..it should overwrite the client instance

				// below code has proper logic, but client does not like to have cursor charges changed
				// (we could delete the cursor item and resend, but issues would arise if there are queued items)
				//if (partialmove) {
				//	EQApplicationPacket* outapp2 = new EQApplicationPacket(OP_DeleteItem, sizeof(DeleteItem_Struct));
				//	DeleteItem_Struct* di	= (DeleteItem_Struct*)outapp2->pBuffer;
				//	di->from_slot			= SLOT_CURSOR;
				//	di->to_slot				= 0xFFFFFFFF;
				//	di->number_in_stack		= 0xFFFFFFFF;

				//	c->Message(0, "Deleting %i charges from stack",  movecount); // debug line..delete

				//	for (int16 deletecount=0; deletecount < movecount; deletecount++)
						// have to use 'movecount' because mi->number_in_stack is 'ENCODED' at this point (i.e., 99 charges returns 22...)
				//		c->QueuePacket(outapp2);

				//	safe_delete(outapp2);
				//}
			}
			else {
				c->Message(13, "Error: Unable to equip current item");
			}
			safe_delete(outapp);

			// also send out a wear change packet?
		}
		else if (from_inst == nullptr)
			c->Message(13, "Error: There is no item on your cursor");
		else
			c->Message(13, "Error: Item on your cursor cannot be equipped");
	}
	else
		c->Message(0, "Usage: #equipitem slotid[0-21] - equips the item on your cursor to the position");
}

void command_zonelock(Client *c, const Seperator *sep)
{
	auto pack = new ServerPacket(ServerOP_LockZone, sizeof(ServerLockZone_Struct));
	ServerLockZone_Struct* s = (ServerLockZone_Struct*) pack->pBuffer;
	strn0cpy(s->adminname, c->GetName(), sizeof(s->adminname));
	if (strcasecmp(sep->arg[1], "list") == 0) {
		s->op = 0;
		worldserver.SendPacket(pack);
	}
	else if (strcasecmp(sep->arg[1], "lock") == 0 && c->Admin() >= commandLockZones) {
		uint16 tmp = database.GetZoneID(sep->arg[2]);
		if (tmp) {
			s->op = 1;
			s->zoneID = tmp;
			worldserver.SendPacket(pack);
		}
		else
			c->Message(0, "Usage: #zonelock lock [zonename]");
	}
	else if (strcasecmp(sep->arg[1], "unlock") == 0 && c->Admin() >= commandLockZones) {
		uint16 tmp = database.GetZoneID(sep->arg[2]);
		if (tmp) {
			s->op = 2;
			s->zoneID = tmp;
			worldserver.SendPacket(pack);
		}
		else
			c->Message(0, "Usage: #zonelock unlock [zonename]");
	}
	else {
		c->Message(0, "#zonelock sub-commands");
		c->Message(0, "  list");
		if(c->Admin() >= commandLockZones)
		{
			c->Message(0, "  lock [zonename]");
			c->Message(0, "  unlock [zonename]");
		}
	}
	safe_delete(pack);
}

void command_corpse(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();

	if (strcasecmp(sep->arg[1], "DeletePlayerCorpses") == 0 && c->Admin() >= commandEditPlayerCorpses) {
		int32 tmp = entity_list.DeletePlayerCorpses();
		if (tmp >= 0)
			c->Message(0, "%i corpses deleted.",  tmp);
		else
			c->Message(0, "DeletePlayerCorpses Error #%i",  tmp);
	}
	else if (strcasecmp(sep->arg[1], "delete") == 0) {
		if (target == 0 || !target->IsCorpse())
			c->Message(0, "Error: Target the corpse you wish to delete");
		else if (target->IsNPCCorpse()) {

			c->Message(0, "Depoping %s.",  target->GetName());
			target->CastToCorpse()->Delete();
		}
		else if (c->Admin() >= commandEditPlayerCorpses) {
			c->Message(0, "Deleting %s.",  target->GetName());
			target->CastToCorpse()->Delete();
		}
		else
			c->Message(0, "Insufficient status to delete player corpse.");
	}
	else if (strcasecmp(sep->arg[1], "ListNPC") == 0) {
		entity_list.ListNPCCorpses(c);
	}
	else if (strcasecmp(sep->arg[1], "ListPlayer") == 0) {
		entity_list.ListPlayerCorpses(c);
	}
	else if (strcasecmp(sep->arg[1], "DeleteNPCCorpses") == 0) {
		int32 tmp = entity_list.DeleteNPCCorpses();
		if (tmp >= 0)
			c->Message(0, "%d corpses deleted.",  tmp);
		else
			c->Message(0, "DeletePlayerCorpses Error #%d",  tmp);
	}
	else if (strcasecmp(sep->arg[1], "charid") == 0 && c->Admin() >= commandEditPlayerCorpses) {
		if (target == 0 || !target->IsPlayerCorpse())
			c->Message(0, "Error: Target must be a player corpse.");
		else if (!sep->IsNumber(2))
			c->Message(0, "Error: charid must be a number.");
		else
			c->Message(0, "Setting CharID=%u on PlayerCorpse '%s'",  target->CastToCorpse()->SetCharID(atoi(sep->arg[2])), target->GetName());
	}
	else if (strcasecmp(sep->arg[1], "ResetLooter") == 0) {
		if (target == 0 || !target->IsCorpse())
			c->Message(0, "Error: Target the corpse you wish to reset");
		else
			target->CastToCorpse()->ResetLooter();
	}
	else if (strcasecmp(sep->arg[1], "RemoveCash") == 0) {
		if (target == 0 || !target->IsCorpse())
			c->Message(0, "Error: Target the corpse you wish to remove the cash from");
		else if (!target->IsPlayerCorpse() || c->Admin() >= commandEditPlayerCorpses) {
			c->Message(0, "Removing Cash from %s.",  target->GetName());
			target->CastToCorpse()->RemoveCash();
		}
		else
			c->Message(0, "Insufficient status to modify player corpse.");
	}
	else if (strcasecmp(sep->arg[1], "InspectLoot") == 0) {
		if (target == 0 || !target->IsCorpse())
			c->Message(0, "Error: Target must be a corpse.");
		else
			target->CastToCorpse()->QueryLoot(c);
	}
	else if (strcasecmp(sep->arg[1], "lock") == 0) {
		if (target == 0 || !target->IsCorpse())
			c->Message(0, "Error: Target must be a corpse.");
		else {
			target->CastToCorpse()->Lock();
			c->Message(0, "Locking %s...",  target->GetName());
		}
	}
	else if (strcasecmp(sep->arg[1], "unlock") == 0) {
		if (target == 0 || !target->IsCorpse())
			c->Message(0, "Error: Target must be a corpse.");
		else {
			target->CastToCorpse()->UnLock();
			c->Message(0, "Unlocking %s...",  target->GetName());
		}
	}
	else if (strcasecmp(sep->arg[1], "depop") == 0) {
		if (target == 0 || !target->IsPlayerCorpse())
			c->Message(0, "Error: Target must be a player corpse.");
		else if (c->Admin() >= commandEditPlayerCorpses && target->IsPlayerCorpse()) {
			c->Message(0, "Depoping %s.",  target->GetName());
			target->CastToCorpse()->DepopPlayerCorpse();
			if(!sep->arg[2][0] || atoi(sep->arg[2]) != 0)
				target->CastToCorpse()->Bury();
		}
		else
			c->Message(0, "Insufficient status to depop player corpse.");
	}
	else if (strcasecmp(sep->arg[1], "depopall") == 0) {
		if (target == 0 || !target->IsClient())
			c->Message(0, "Error: Target must be a player.");
		else if (c->Admin() >= commandEditPlayerCorpses && target->IsClient()) {
			c->Message(0, "Depoping %s\'s corpses.",  target->GetName());
			target->CastToClient()->DepopAllCorpses();
			if(!sep->arg[2][0] || atoi(sep->arg[2]) != 0)
				target->CastToClient()->BuryPlayerCorpses();
		}
		else
			c->Message(0, "Insufficient status to depop player corpse.");

	}
	else if (sep->arg[1][0] == 0 || strcasecmp(sep->arg[1], "help") == 0) {
		c->Message(0, "#Corpse Sub-Commands:");
		c->Message(0, "  DeleteNPCCorpses");
		c->Message(0, "  Delete - Delete targetted corpse");
		c->Message(0, "  ListNPC");
		c->Message(0, "  ListPlayer");
		c->Message(0, "  Lock - GM locks the corpse - cannot be looted by non-GM");
		c->Message(0, "  UnLock");
		c->Message(0, "  RemoveCash");
		c->Message(0, "  InspectLoot");
		c->Message(0, "  [to remove items from corpses, loot them]");
		c->Message(0, "Lead-GM status required to delete/modify player corpses");
		c->Message(0, "  DeletePlayerCorpses");
		c->Message(0, "  CharID [charid] - change player corpse's owner");
		c->Message(0, "  Depop [bury] - Depops single target corpse.");
		c->Message(0, "  Depopall [bury] - Depops all target player's corpses.");
		c->Message(0, "Set bury to 0 to skip burying the corpses.");
	}
	else
		c->Message(0, "Error, #corpse sub-command not found");
}

void command_fixmob(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	const char* Usage = "Usage: #fixmob [race|gender|texture|helm|face|hair|haircolor|beard|beardcolor|heritage|tattoo|detail] [next|prev]";

	if (!sep->arg[1])
		c->Message(0,Usage);
	else if (!target)
		c->Message(0,"Error: this command requires a target");
	else
	{

		uint32 Adjustment = 1;	// Previous or Next
		char codeMove = 0;

		if (sep->arg[2])
		{
			char* command2 = sep->arg[2];
			codeMove = (command2[0] | 0x20); // First character, lower-cased
			if (codeMove == 'n')
				Adjustment = 1;
			else if (codeMove == 'p')
				Adjustment = -1;
		}

		uint16 Race = target->GetRace();
		uint8 Gender = target->GetGender();
		uint8 Texture = 0xFF;
		uint8 HelmTexture = 0xFF;
		uint8 HairColor = target->GetHairColor();
		uint8 BeardColor = target->GetBeardColor();
		uint8 EyeColor1 = target->GetEyeColor1();
		uint8 EyeColor2 = target->GetEyeColor2();
		uint8 HairStyle = target->GetHairStyle();
		uint8 LuclinFace = target->GetLuclinFace();
		uint8 Beard = target->GetBeard();
		uint32 DrakkinHeritage = target->GetDrakkinHeritage();
		uint32 DrakkinTattoo = target->GetDrakkinTattoo();
		uint32 DrakkinDetails = target->GetDrakkinDetails();

		const char* ChangeType = nullptr; // If it's still nullptr after processing, they didn't send a valid command
		uint32 ChangeSetting;
		char* command = sep->arg[1];

		if (strcasecmp(command, "race") == 0)
		{
			if (Race == 1 && codeMove == 'p')
				Race = 724;
			else if (Race >= 724 && codeMove != 'p')
				Race = 1;
			else
				Race += Adjustment;
			ChangeType = "Race";
			ChangeSetting = Race;
		}
		else if (strcasecmp(command, "gender") == 0)
		{
			if (Gender == 0 && codeMove == 'p')
				Gender = 2;
			else if (Gender >= 2 && codeMove != 'p')
				Gender = 0;
			else
				Gender += Adjustment;
			ChangeType = "Gender";
			ChangeSetting = Gender;
		}
		else if (strcasecmp(command, "texture") == 0)
		{
			Texture = target->GetTexture();

			if (Texture == 0 && codeMove == 'p')
				Texture = 25;
			else if (Texture >= 25 && codeMove != 'p')
				Texture = 0;
			else
				Texture += Adjustment;
			ChangeType = "Texture";
			ChangeSetting = Texture;
		}
		else if (strcasecmp(command, "helm") == 0)
		{
			HelmTexture = target->GetHelmTexture();
			if (HelmTexture == 0 && codeMove == 'p')
				HelmTexture = 25;
			else if (HelmTexture >= 25 && codeMove != 'p')
				HelmTexture = 0;
			else
				HelmTexture += Adjustment;
			ChangeType = "HelmTexture";
			ChangeSetting = HelmTexture;
		}
		else if (strcasecmp(command, "face") == 0)
		{
			if (LuclinFace == 0 && codeMove == 'p')
				LuclinFace = 87;
			else if (LuclinFace >= 87 && codeMove != 'p')
				LuclinFace = 0;
			else
				LuclinFace += Adjustment;
			ChangeType = "LuclinFace";
			ChangeSetting = LuclinFace;
		}
		else if (strcasecmp(command, "hair") == 0)
		{
			if (HairStyle == 0 && codeMove == 'p')
				HairStyle = 8;
			else if (HairStyle >= 8 && codeMove != 'p')
				HairStyle = 0;
			else
				HairStyle += Adjustment;
			ChangeType = "HairStyle";
			ChangeSetting = HairStyle;
		}
		else if (strcasecmp(command, "haircolor") == 0)
		{
			if (HairColor == 0 && codeMove == 'p')
				HairColor = 24;
			else if (HairColor >= 24 && codeMove != 'p')
				HairColor = 0;
			else
				HairColor += Adjustment;
			ChangeType = "HairColor";
			ChangeSetting = HairColor;
		}
		else if (strcasecmp(command, "beard") == 0)
		{
			if (Beard == 0 && codeMove == 'p')
				Beard = 11;
			else if (Beard >= 11 && codeMove != 'p')
				Beard = 0;
			else
				Beard += Adjustment;
			ChangeType = "Beard";
			ChangeSetting = Beard;
		}
		else if (strcasecmp(command, "beardcolor") == 0)
		{
			if (BeardColor == 0 && codeMove == 'p')
				BeardColor = 24;
			else if (BeardColor >= 24 && codeMove != 'p')
				BeardColor = 0;
			else
				BeardColor += Adjustment;
			ChangeType = "BeardColor";
			ChangeSetting = BeardColor;
		}
		else if (strcasecmp(command, "heritage") == 0)
		{
			if (DrakkinHeritage == 0 && codeMove == 'p')
				DrakkinHeritage = 6;
			else if (DrakkinHeritage >= 6 && codeMove != 'p')
				DrakkinHeritage = 0;
			else
				DrakkinHeritage += Adjustment;
			ChangeType = "DrakkinHeritage";
			ChangeSetting = DrakkinHeritage;
		}
		else if (strcasecmp(command, "tattoo") == 0)
		{
			if (DrakkinTattoo == 0 && codeMove == 'p')
				DrakkinTattoo = 8;
			else if (DrakkinTattoo >= 8 && codeMove != 'p')
				DrakkinTattoo = 0;
			else
				DrakkinTattoo += Adjustment;
			ChangeType = "DrakkinTattoo";
			ChangeSetting = DrakkinTattoo;
		}
		else if (strcasecmp(command, "detail") == 0)
		{
			if (DrakkinDetails == 0 && codeMove == 'p')
				DrakkinDetails = 7;
			else if (DrakkinDetails >= 7 && codeMove != 'p')
				DrakkinDetails = 0;
			else
				DrakkinDetails += Adjustment;
			ChangeType = "DrakkinDetails";
			ChangeSetting = DrakkinDetails;
		}

		// Hack to fix some races that base features from face
		switch (Race)
		{
			case 2:	// Barbarian
				if (LuclinFace > 10) {
					LuclinFace -= ((DrakkinTattoo - 1) * 10);
				}
				LuclinFace += (DrakkinTattoo * 10);
				break;
			case 3: // Erudite
				if (LuclinFace > 10) {
					LuclinFace -= ((HairStyle - 1) * 10);
				}
				LuclinFace += (HairStyle * 10);
				break;
			case 5: // HighElf
			case 6: // DarkElf
			case 7: // HalfElf
				if (LuclinFace > 10) {
					LuclinFace -= ((Beard - 1) * 10);
				}
				LuclinFace += (Beard * 10);
				break;
			default:
				break;
		}


		if (ChangeType == nullptr)
		{
			c->Message(0,Usage);
		}
		else
		{
			target->SendIllusionPacket(Race, Gender, Texture, HelmTexture, HairColor, BeardColor,
										EyeColor1, EyeColor2, HairStyle, LuclinFace, Beard, 0xFF,
										DrakkinHeritage, DrakkinTattoo, DrakkinDetails);

			c->Message(0, "%s=%i",  ChangeType, ChangeSetting);
		}
	}
}

void command_gmspeed(Client *c, const Seperator *sep)
{
	bool state=atobool(sep->arg[1]);
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	if(sep->arg[1][0] != 0) {
		database.SetGMSpeed(t->AccountID(), state?1:0);
		c->Message(0, "Turning GMSpeed %s for %s (zone to take effect)",  state?"On":"Off", t->GetName());
	}
	else
		c->Message(0, "Usage: #gmspeed [on/off]");
}

void command_title(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0]==0)
		c->Message(0, "Usage: #title [remove|text] [1 = Create row in title table] - remove or set title to 'text'");
	else {
		bool Save = (atoi(sep->arg[2]) == 1);

		Mob *target_mob = c->GetTarget();
		if(!target_mob)
			target_mob = c;
		if(!target_mob->IsClient()) {
			c->Message(13, "#title only works on players.");
			return;
		}
		Client *t = target_mob->CastToClient();

		if(strlen(sep->arg[1]) > 31) {
			c->Message(13, "Title must be 31 characters or less.");
			return;
		}

		bool removed = false;
		if(!strcasecmp(sep->arg[1], "remove")) {
			t->SetAATitle("");
			removed = true;
		} else {
			for(unsigned int i=0; i<strlen(sep->arg[1]); i++)
				if(sep->arg[1][i]=='_')
					sep->arg[1][i] = ' ';
			if(!Save)
				t->SetAATitle(sep->arg[1]);
			else
				title_manager.CreateNewPlayerTitle(t, sep->arg[1]);
		}

		t->Save();

		if(removed) {
			c->Message(13, "%s's title has been removed.",  t->GetName(), sep->arg[1]);
			if(t != c)
				t->Message(13, "Your title has been removed.",  sep->arg[1]);
		} else {
			c->Message(13, "%s's title has been changed to '%s'.",  t->GetName(), sep->arg[1]);
			if(t != c)
				t->Message(13, "Your title has been changed to '%s'.",  sep->arg[1]);
		}
	}
}


void command_titlesuffix(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0]==0)
		c->Message(0, "Usage: #titlesuffix [remove|text] [1 = create row in title table] - remove or set title suffix to 'text'");
	else {
		bool Save = (atoi(sep->arg[2]) == 1);

		Mob *target_mob = c->GetTarget();
		if(!target_mob)
			target_mob = c;
		if(!target_mob->IsClient()) {
			c->Message(13, "#titlesuffix only works on players.");
			return;
		}
		Client *t = target_mob->CastToClient();

		if(strlen(sep->arg[1]) > 31) {
			c->Message(13, "Title suffix must be 31 characters or less.");
			return;
		}

		bool removed = false;
		if(!strcasecmp(sep->arg[1], "remove")) {
			t->SetTitleSuffix("");
			removed = true;
		} else {
			for(unsigned int i=0; i<strlen(sep->arg[1]); i++)
				if(sep->arg[1][i]=='_')
					sep->arg[1][i] = ' ';

			if(!Save)
				t->SetTitleSuffix(sep->arg[1]);
			else
				title_manager.CreateNewPlayerSuffix(t, sep->arg[1]);
		}

		t->Save();

		if(removed) {
			c->Message(13, "%s's title suffix has been removed.",  t->GetName(), sep->arg[1]);
			if(t != c)
				t->Message(13, "Your title suffix has been removed.",  sep->arg[1]);
		} else {
			c->Message(13, "%s's title suffix has been changed to '%s'.",  t->GetName(), sep->arg[1]);
			if(t != c)
				t->Message(13, "Your title suffix has been changed to '%s'.",  sep->arg[1]);
		}
	}
}

void command_spellinfo(Client *c, const Seperator *sep)
{
	if(sep->arg[1][0]==0)
		c->Message(0, "Usage: #spellinfo [spell_id]");
	else {
		short int spell_id=atoi(sep->arg[1]);
		const struct SPDat_Spell_Struct *s=&spells[spell_id];
		c->Message(0, "Spell info for spell #%d:",  spell_id);
		c->Message(0, "  name: %s",  s->name);
		c->Message(0, "  player_1: %s",  s->player_1);
		c->Message(0, "  teleport_zone: %s",  s->teleport_zone);
		c->Message(0, "  you_cast: %s",  s->you_cast);
		c->Message(0, "  other_casts: %s",  s->other_casts);
		c->Message(0, "  cast_on_you: %s",  s->cast_on_you);
		c->Message(0, "  spell_fades: %s",  s->spell_fades);
		c->Message(0, "  range: %f",  s->range);
		c->Message(0, "  aoerange: %f",  s->aoerange);
		c->Message(0, "  pushback: %f",  s->pushback);
		c->Message(0, "  pushup: %f",  s->pushup);
		c->Message(0, "  cast_time: %d",  s->cast_time);
		c->Message(0, "  recovery_time: %d",  s->recovery_time);
		c->Message(0, "  recast_time: %d",  s->recast_time);
		c->Message(0, "  buffdurationformula: %d",  s->buffdurationformula);
		c->Message(0, "  buffduration: %d",  s->buffduration);
		c->Message(0, "  AEDuration: %d",  s->AEDuration);
		c->Message(0, "  mana: %d",  s->mana);
		c->Message(0, "  base[12]: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",  s->base[0], s->base[1], s->base[2], s->base[3], s->base[4], s->base[5], s->base[6], s->base[7], s->base[8], s->base[9], s->base[10], s->base[11]);
		c->Message(0, "  base22[12]: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",  s->base2[0], s->base2[1], s->base2[2], s->base2[3], s->base2[4], s->base2[5], s->base2[6], s->base2[7], s->base2[8], s->base2[9], s->base2[10], s->base2[11]);
		c->Message(0, "  max[12]: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",  s->max[0], s->max[1], s->max[2], s->max[3], s->max[4], s->max[5], s->max[6], s->max[7], s->max[8], s->max[9], s->max[10], s->max[11]);
		c->Message(0, "  components[4]: %d, %d, %d, %d",  s->components[0], s->components[1], s->components[2], s->components[3]);
		c->Message(0, "  component_counts[4]: %d, %d, %d, %d",  s->component_counts[0], s->component_counts[1], s->component_counts[2], s->component_counts[3]);
		c->Message(0, "  NoexpendReagent[4]: %d, %d, %d, %d",  s->NoexpendReagent[0], s->NoexpendReagent[1], s->NoexpendReagent[2], s->NoexpendReagent[3]);
		c->Message(0, "  formula[12]: 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x",  s->formula[0], s->formula[1], s->formula[2], s->formula[3], s->formula[4], s->formula[5], s->formula[6], s->formula[7], s->formula[8], s->formula[9], s->formula[10], s->formula[11]);
		c->Message(0, "  goodEffect: %d",  s->goodEffect);
		c->Message(0, "  Activated: %d",  s->Activated);
		c->Message(0, "  resisttype: %d",  s->resisttype);
		c->Message(0, "  effectid[12]: 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x",  s->effectid[0], s->effectid[1], s->effectid[2], s->effectid[3], s->effectid[4], s->effectid[5], s->effectid[6], s->effectid[7], s->effectid[8], s->effectid[9], s->effectid[10], s->effectid[11]);
		c->Message(0, "  targettype: %d",  s->targettype);
		c->Message(0, "  basediff: %d",  s->basediff);
		c->Message(0, "  skill: %d",  s->skill);
		c->Message(0, "  zonetype: %d",  s->zonetype);
		c->Message(0, "  EnvironmentType: %d",  s->EnvironmentType);
		c->Message(0, "  TimeOfDay: %d",  s->TimeOfDay);
		c->Message(0, "  classes[15]: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
			s->classes[0], s->classes[1], s->classes[2], s->classes[3], s->classes[4],
			s->classes[5], s->classes[6], s->classes[7], s->classes[8], s->classes[9],
			s->classes[10], s->classes[11], s->classes[12], s->classes[13], s->classes[14]);
		c->Message(0, "  CastingAnim: %d",  s->CastingAnim);
		c->Message(0, "  SpellAffectIndex: %d",  s->SpellAffectIndex);
		c->Message(0, " RecourseLink: %d",  s->RecourseLink);
	}
}

void command_lastname(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();
	Log(Logs::General, Logs::Normal, "#lastname request from %s for %s",  c->GetName(), t->GetName());

	if(strlen(sep->arg[1]) <= 70)
		t->ChangeLastName(sep->arg[1]);
	else
		c->Message(0, "Usage: #lastname <lastname> where <lastname> is less than 70 chars long");
}

void command_memspell(Client *c, const Seperator *sep)
{
	uint32 slot;
	uint16 spell_id;

	if (!(sep->IsNumber(1) && sep->IsNumber(2)))
	{
		c->Message(0, "Usage: #MemSpell slotid spellid");
	}
	else
	{
		slot = atoi(sep->arg[1]) - 1;
		spell_id = atoi(sep->arg[2]);
		if (slot > MAX_PP_MEMSPELL || spell_id >= SPDAT_RECORDS)
		{
			c->Message(0, "Error: #MemSpell: Arguement out of range");
		}
		else
		{
			c->MemSpell(spell_id, slot);
			c->Message(0, "Spell slot changed, have fun!");
		}
	}
}

void command_save(Client *c, const Seperator *sep)
{
	if (c->GetTarget() == 0)
		c->Message(0, "Error: no target");
	else if (c->GetTarget()->IsClient()) {
		if (c->GetTarget()->CastToClient()->Save(2))
			c->Message(0, "%s successfully saved.",  c->GetTarget()->GetName());
		else
			c->Message(0, "Manual save for %s failed.",  c->GetTarget()->GetName());
	}
	else if (c->GetTarget()->IsPlayerCorpse()) {
		if (c->GetTarget()->CastToMob()->Save())
			c->Message(0, "%s successfully saved. (dbid=%u)",  c->GetTarget()->GetName(), c->GetTarget()->CastToCorpse()->GetCorpseDBID());
		else
			c->Message(0, "Manual save for %s failed.",  c->GetTarget()->GetName());
	}
	else
		c->Message(0, "Error: target not a Client/PlayerCorpse");
}

void command_showstats(Client *c, const Seperator *sep)
{
	if (c->GetTarget() != 0 )
		c->GetTarget()->ShowStats(c);
	else
		c->ShowStats(c);
}

void command_mystats(Client *c, const Seperator *sep)
{
	if (c->GetTarget() && c->GetPet()) {
		if (c->GetTarget()->IsPet() && c->GetTarget() == c->GetPet())
			c->GetTarget()->ShowStats(c);
		else
			c->ShowStats(c);
	}
	else
		c->ShowStats(c);
}

void command_myskills(Client *c, const Seperator *sep)
{
	c->ShowSkillsWindow();
}

void command_bind(Client *c, const Seperator *sep)
{
	if (c->GetTarget() != 0 ) {
		if (c->GetTarget()->IsClient())
			c->GetTarget()->CastToClient()->SetBindPoint();
		else
			c->Message(0, "Error: target not a Player");
	} else
		c->SetBindPoint();
}

void command_depop(Client *c, const Seperator *sep)
{
	if (c->GetTarget() == 0 || !(c->GetTarget()->IsNPC() || c->GetTarget()->IsNPCCorpse()))
		c->Message(0, "You must have a NPC target for this command. (maybe you meant #depopzone?)");
	else {
		c->Message(0, "Depoping '%s'.",  c->GetTarget()->GetName());
		c->GetTarget()->Depop();
	}
}

void command_depopzone(Client *c, const Seperator *sep)
{
	zone->Depop();
	c->Message(0, "Zone depoped.");
}

void command_repop(Client *c, const Seperator *sep)
{
	int timearg = 1;
	int delay = 0;

	if (sep->arg[1] && strcasecmp(sep->arg[1], "force") == 0) {
		timearg++;

		LinkedListIterator<Spawn2*> iterator(zone->spawn2_list);
		iterator.Reset();
		while (iterator.MoreElements()) {
			std::string query = StringFormat(
				"DELETE FROM respawn_times WHERE id = %lu AND instance_id = %lu",
				(unsigned long)iterator.GetData()->GetID(),
				(unsigned long)zone->GetInstanceID()
			);
			auto results = database.QueryDatabase(query);
			iterator.Advance();
		}
		c->Message(0, "Zone depop: Force resetting spawn timers.");
	}

	if (!sep->IsNumber(timearg)) {
        c->Message(0, "Zone depopped - repopping now.");

		zone->Repop();

		/* Force a spawn2 timer trigger so we don't delay actually spawning the NPC's */
		zone->spawn2_timer.Trigger();
		return;
	}

    c->Message(0, "Zone depoped. Repop in %i seconds",  atoi(sep->arg[timearg]));
	zone->Repop(atoi(sep->arg[timearg]) * 1000);

	zone->spawn2_timer.Trigger();
}

void command_repopclose(Client *c, const Seperator *sep)
{
	int repop_distance = 500;

	if (sep->arg[1] && strcasecmp(sep->arg[1], "force") == 0) {

		LinkedListIterator<Spawn2*> iterator(zone->spawn2_list);
		iterator.Reset();
		while (iterator.MoreElements()) {
			std::string query = StringFormat(
				"DELETE FROM respawn_times WHERE id = %lu AND instance_id = %lu",
				(unsigned long)iterator.GetData()->GetID(),
				(unsigned long)zone->GetInstanceID()
			);
			auto results = database.QueryDatabase(query);
			iterator.Advance();
		}
		c->Message(0, "Zone depop: Force resetting spawn timers.");
	}
	if (sep->IsNumber(1)) {
		repop_distance = atoi(sep->arg[1]);
	}

	c->Message(0, "Zone depoped. Repopping NPC's within %i distance units", repop_distance);
	zone->RepopClose(c->GetPosition(), repop_distance);
}

void command_spawnstatus(Client *c, const Seperator *sep)
{
	if((sep->arg[1][0] == 'e') | (sep->arg[1][0] == 'E'))
	{
		// show only enabled spawns
		zone->ShowEnabledSpawnStatus(c);
	}
	else if((sep->arg[1][0] == 'd') | (sep->arg[1][0] == 'D'))
	{
		// show only disabled spawns
		zone->ShowDisabledSpawnStatus(c);
	}
	else if((sep->arg[1][0] == 'a') | (sep->arg[1][0] == 'A'))
	{
		// show all spawn staus with no filters
		zone->SpawnStatus(c);
	}
	else if(sep->IsNumber(1))
	{
		// show spawn status by spawn2 id
		zone->ShowSpawnStatusByID(c, atoi(sep->arg[1]));
	}
	else if(strcmp(sep->arg[1], "help") == 0)
	{
		c->Message(0, "Usage: #spawnstatus <[a]ll | [d]isabled | [e]nabled | {Spawn2 ID}>");
	}
	else {
		zone->SpawnStatus(c);
	}
}

void command_nukebuffs(Client *c, const Seperator *sep)
{
	if (c->GetTarget() == 0)
		c->BuffFadeAll();
	else
		c->GetTarget()->BuffFadeAll();
}

void command_zuwcoords(Client *c, const Seperator *sep)
{
	// modifys and resends zhdr packet
	if(sep->arg[1][0]==0)
		c->Message(0, "Usage: #zuwcoords <under world coords>");
	else {
		zone->newzone_data.underworld = atof(sep->arg[1]);
		//float newdata = atof(sep->arg[1]);
		//memcpy(&zone->zone_header_data[130], &newdata, sizeof(float));
		auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
		memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
		entity_list.QueueClients(c, outapp);
		safe_delete(outapp);
	}
}

void command_zunderworld(Client *c, const Seperator *sep)
{
	if(sep->arg[1][0]==0)
		c->Message(0, "Usage: #zunderworld <zcoord>");
	else {
		zone->newzone_data.underworld = atof(sep->arg[1]);
	}
}

void command_zsafecoords(Client *c, const Seperator *sep)
{
	// modifys and resends zhdr packet
	if(sep->arg[3][0]==0)
		c->Message(0, "Usage: #zsafecoords <safe x> <safe y> <safe z>");
	else {
		zone->newzone_data.safe_x = atof(sep->arg[1]);
		zone->newzone_data.safe_y = atof(sep->arg[2]);
		zone->newzone_data.safe_z = atof(sep->arg[3]);
		//float newdatax = atof(sep->arg[1]);
		//float newdatay = atof(sep->arg[2]);
		//float newdataz = atof(sep->arg[3]);
		//memcpy(&zone->zone_header_data[114], &newdatax, sizeof(float));
		//memcpy(&zone->zone_header_data[118], &newdatay, sizeof(float));
		//memcpy(&zone->zone_header_data[122], &newdataz, sizeof(float));
		//zone->SetSafeCoords();
		auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
		memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
		entity_list.QueueClients(c, outapp);
		safe_delete(outapp);
	}
}

void command_freeze(Client *c, const Seperator *sep)
{
	if (c->GetTarget() != 0)
		c->GetTarget()->SendAppearancePacket(AT_Anim, ANIM_FREEZE);
	else
		c->Message(0, "ERROR: Freeze requires a target.");
}

void command_unfreeze(Client *c, const Seperator *sep)
{
	if (c->GetTarget() != 0)
		c->GetTarget()->SendAppearancePacket(AT_Anim, ANIM_STAND);
	else
		c->Message(0, "ERROR: Unfreeze requires a target.");
}

void command_pvp(Client *c, const Seperator *sep)
{
	bool state=atobool(sep->arg[1]);
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	if(sep->arg[1][0] != 0) {
		t->SetPVP(state);
		c->Message(0, "%s now follows the ways of %s.",  t->GetName(), state?"discord":"order");
	}
	else
		c->Message(0, "Usage: #pvp [on/off]");
}

void command_setxp(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	if (sep->IsNumber(1)) {
		if (atoi(sep->arg[1]) > 9999999)
			c->Message(0, "Error: Value too high.");
		else
			t->AddEXP(atoi(sep->arg[1]));
	}
	else
		c->Message(0, "Usage: #setxp number");
}

void command_setpvppoints(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	if (sep->IsNumber(1)) {
		if (atoi(sep->arg[1]) > 9999999)
			c->Message(0, "Error: Value too high.");
		else
		{
			t->SetPVPPoints(atoi(sep->arg[1]));
			t->Save();
			t->SendPVPStats();
		}
	}
	else
		c->Message(0, "Usage: #setpvppoints number");
}

void command_name(Client *c, const Seperator *sep)
{
	Client *target;

	if( (strlen(sep->arg[1]) == 0) || (!(c->GetTarget() && c->GetTarget()->IsClient())) )
		c->Message(0, "Usage: #name newname (requires player target)");
	else
	{
		target = c->GetTarget()->CastToClient();
		char *oldname = strdup(target->GetName());
		if(target->ChangeFirstName(sep->arg[1], c->GetName()))
		{
			c->Message(0, "Successfully renamed %s to %s",  oldname, sep->arg[1]);
			// until we get the name packet working right this will work
			c->Message(0, "Sending player to char select.");
			target->Kick();
		}
		else
			c->Message(13, "ERROR: Unable to rename %s. Check that the new name '%s' isn't already taken.",  oldname, sep->arg[2]);
		free(oldname);
	}
}

void command_tempname(Client *c, const Seperator *sep)
{
	Mob *target;
	target = c->GetTarget();

	if(!target)
		c->Message(0, "Usage: #tempname newname (requires a target)");
	else if(strlen(sep->arg[1]) > 0)
	{
		char *oldname = strdup(target->GetName());
		target->TempName(sep->arg[1]);
		c->Message(0, "Renamed %s to %s",  oldname, sep->arg[1]);
		free(oldname);
	}
	else {
		target->TempName();
		c->Message(0, "Restored the original name");
	}
}

void command_petname(Client *c, const Seperator *sep)
{
	Mob *target;
	target = c->GetTarget();

	if(!target)
		c->Message(0, "Usage: #petname newname (requires a target)");
	else if(target->IsPet() && (target->GetOwnerID() == c->GetID()) && strlen(sep->arg[1]) > 0)
	{
		char *oldname = strdup(target->GetName());
		target->TempName(sep->arg[1]);
		c->Message(0, "Renamed %s to %s",  oldname, sep->arg[1]);
		free(oldname);
	}
	else {
		target->TempName();
		c->Message(0, "Restored the original name");
	}
}

void command_npcspecialattk(Client *c, const Seperator *sep)
{
	if (c->GetTarget()==0 || c->GetTarget()->IsClient() || strlen(sep->arg[1]) <= 0 || strlen(sep->arg[2]) <= 0)
		c->Message(0, "Usage: #npcspecialattk *flagchar* *permtag* (Flags are E(nrage) F(lurry) R(ampage) S(ummon), permtag is 1 = True, 0 = False).");
	else {
		c->GetTarget()->CastToNPC()->NPCSpecialAttacks(sep->arg[1],atoi(sep->arg[2]));
		c->Message(0, "NPC Special Attack set.");
	}
}

void command_kill(Client *c, const Seperator *sep)
{
	if (!c->GetTarget()) {
		c->Message(0, "Error: #Kill: No target.");
	}
	else
		if (!c->GetTarget()->IsClient() || c->GetTarget()->CastToClient()->Admin() <= c->Admin())
			c->GetTarget()->Kill();
}

void command_haste(Client *c, const Seperator *sep)
{
	// #haste command to set client attack speed. Takes a percentage (100 = twice normal attack speed)
	if(sep->arg[1][0] != 0) {
		uint16 Haste = atoi(sep->arg[1]);
		if(Haste > 85)
			Haste = 85;
		c->SetExtraHaste(Haste);
		// SetAttackTimer must be called to make this take effect, so player needs to change
		// the primary weapon.
		c->Message(0, "Haste set to %d%% - Need to re-equip primary weapon before it takes effect",  Haste);
	}
	else
		c->Message(0, "Usage: #haste [percentage]");
}

void command_damage(Client *c, const Seperator *sep)
{
	if (c->GetTarget()==0)
		c->Message(0, "Error: #Damage: No Target.");
	else if (!sep->IsNumber(1)) {
		c->Message(0, "Usage: #damage x");
	}
	else {
		int32 nkdmg = atoi(sep->arg[1]);
		if (nkdmg > 2100000000)
			c->Message(0, "Enter a value less then 2,100,000,000.");
		else
			c->GetTarget()->Damage(c, nkdmg, SPELL_UNKNOWN, EQEmu::skills::SkillHandtoHand, false);
	}
}

void command_zonespawn(Client *c, const Seperator *sep)
{
	c->Message(0, "This command is not yet implemented.");
	return;

/* this was kept from client.cpp verbatim (it was commented out) */
	//	if (target && target->IsNPC()) {
	//		Message(0, "Inside main if.");
	//		if (strcasecmp(sep->arg[1], "add")==0) {
	//			Message(0, "Inside add if.");
	//			database.DBSpawn(1, StaticGetZoneName(this->GetPP().current_zone), target->CastToNPC());
	//		}
	//		else if (strcasecmp(sep->arg[1], "update")==0) {
	//			database.DBSpawn(2, StaticGetZoneName(this->GetPP().current_zone), target->CastToNPC());
	//		}
	//		else if (strcasecmp(sep->arg[1], "remove")==0) {
	//			if (strcasecmp(sep->arg[2], "all")==0) {
	//				database.DBSpawn(4, StaticGetZoneName(this->GetPP().current_zone));
	//			}
	//			else {
	//				if (database.DBSpawn(3, StaticGetZoneName(this->GetPP().current_zone), target->CastToNPC())) {
	//					Message(0, "#zonespawn: %s removed successfully!",  target->GetName());
	//					target->CastToNPC()->Death(target, target->GetHP());
	//				}
	//			}
	//		}
	//		else
	//			Message(0, "Error: #dbspawn: Invalid command. (Note: EDIT and REMOVE are NOT in yet.)");
	//		if (target->CastToNPC()->GetNPCTypeID() > 0) {
	//			Message(0, "Spawn is type %i",  target->CastToNPC()->GetNPCTypeID());
	//		}
	//	}
	//	else if(!target || !target->IsNPC())
	//		Message(0, "Error: #zonespawn: You must have a NPC targeted!");
	//	else
	//		Message(0, "Usage: #zonespawn [add|edit|remove|remove all]");
}

void command_npcspawn(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	uint32 extra = 0;

	if (target && target->IsNPC()) {
		if (strcasecmp(sep->arg[1], "create") == 0) {
			if (atoi(sep->arg[2]))
			{
				// Option to try to create the npc_type ID within the range for the current zone (zone_id * 1000)
				extra = 1;
			}
			database.NPCSpawnDB(0, zone->GetShortName(), zone->GetInstanceVersion(), c, target->CastToNPC(), extra);
			c->Message(0, "%s created successfully!",  target->GetName());
		}
		else if (strcasecmp(sep->arg[1], "add") == 0) {
			if (atoi(sep->arg[2]))
			{
				extra = atoi(sep->arg[2]);
			}
			else
			{
				// Respawn Timer default if not set
				extra = 1200;
			}
			database.NPCSpawnDB(1, zone->GetShortName(), zone->GetInstanceVersion(), c, target->CastToNPC(), extra);
			c->Message(0, "%s added successfully!",  target->GetName());
		}
		else if (strcasecmp(sep->arg[1], "update") == 0) {
			database.NPCSpawnDB(2, zone->GetShortName(), zone->GetInstanceVersion(), c, target->CastToNPC());
			c->Message(0, "%s updated!",  target->GetName());
		}
		else if (strcasecmp(sep->arg[1], "remove") == 0) {
			database.NPCSpawnDB(3, zone->GetShortName(), zone->GetInstanceVersion(), c, target->CastToNPC());
			c->Message(0, "%s removed successfully from database!",  target->GetName());
			target->Depop(false);
		}
		else if (strcasecmp(sep->arg[1], "delete") == 0) {
			database.NPCSpawnDB(4, zone->GetShortName(), zone->GetInstanceVersion(), c, target->CastToNPC());
			c->Message(0, "%s deleted from database!",  target->GetName());
			target->Depop(false);
		}
		else {
			c->Message(0, "Error: #npcspawn: Invalid command.");
			c->Message(0, "Usage: #npcspawn [create|add|update|remove|delete]");
		}
	}
	else
		c->Message(0, "Error: #npcspawn: You must have a NPC targeted!");
}

void command_spawnfix(Client *c, const Seperator *sep) {
	Mob *targetMob = c->GetTarget();
	if (!targetMob || !targetMob->IsNPC()) {
		c->Message(0, "Error: #spawnfix: Need an NPC target.");
		return;
    }

    Spawn2* s2 = targetMob->CastToNPC()->respawn2;

    if(!s2) {
        c->Message(0, "#spawnfix FAILED -- cannot determine which spawn entry in the database this mob came from.");
        return;
    }

    std::string query = StringFormat("UPDATE spawn2 SET x = '%f', y = '%f', z = '%f', heading = '%f' WHERE id = '%i'",
                                    c->GetX(), c->GetY(), c->GetZ(), c->GetHeading(),s2->GetID());
    auto results = database.QueryDatabase(query);
    if (!results.Success()) {
        c->Message(13, "Update failed! MySQL gave the following error:");
        c->Message(13, results.ErrorMessage().c_str());
        return;
    }

    c->Message(0, "Updating coordinates successful.");
    targetMob->Depop(false);
}

void command_loc(Client *c, const Seperator *sep)
{
	Mob *t=c->GetTarget()?c->GetTarget():c->CastToMob();

	c->Message(0, "%s's Location (XYZ): %1.2f, %1.2f, %1.2f; heading=%1.1f",  t->GetName(), t->GetX(), t->GetY(), t->GetZ(), t->GetHeading());
}

void command_goto(Client *c, const Seperator *sep)
{
	// goto function
	if (sep->arg[1][0] == '\0' && c->GetTarget())
		c->MovePC(zone->GetZoneID(), zone->GetInstanceID(), c->GetTarget()->GetX(), c->GetTarget()->GetY(), c->GetTarget()->GetZ(), c->GetTarget()->GetHeading());
	else if (!(sep->IsNumber(1) && sep->IsNumber(2) && sep->IsNumber(3)))
		c->Message(0, "Usage: #goto [x y z]");
	else
		c->MovePC(zone->GetZoneID(), zone->GetInstanceID(), atof(sep->arg[1]), atof(sep->arg[2]), atof(sep->arg[3]), 0.0f);
}

void command_iteminfo(Client *c, const Seperator *sep)
{
	auto inst = c->GetInv()[EQEmu::inventory::slotCursor];
	if (!inst) {
		c->Message(13, "Error: You need an item on your cursor for this command");
		return;
	}
	auto item = inst->GetItem();
	if (!item) {
		Log(Logs::General, Logs::Inventory, "(%s) Command #iteminfo processed an item with no data pointer");
		c->Message(13, "Error: This item has no data reference");
		return;
	}

	EQEmu::SayLinkEngine linker;
	linker.SetLinkType(EQEmu::saylink::SayLinkItemInst);
	linker.SetItemInst(inst);

	auto item_link = linker.GenerateLink();

	c->Message(0, "*** Item Info for [%s] ***", item_link.c_str());
	c->Message(0, ">> ID: %u, ItemUseType: %u, ItemClassType: %u", item->ID, item->ItemType, item->ItemClass);
	c->Message(0, ">> IDFile: '%s', IconID: %u", item->IDFile, item->Icon);
	c->Message(0, ">> Size: %u, Weight: %u, Price: %u, LDoNPrice: %u", item->Size, item->Weight, item->Price, item->LDoNPrice);
	c->Message(0, ">> Material: 0x%02X, Color: 0x%08X, Tint: 0x%08X, Light: 0x%02X", item->Material, item->Color, inst->GetColor(), item->Light);
	c->Message(0, ">> IsLore: %s, LoreGroup: %u, Lore: '%s'", (item->LoreFlag ? "TRUE" : "FALSE"), item->LoreGroup, item->Lore);
	c->Message(0, ">> NoDrop: %u, NoRent: %u, NoPet: %u, NoTransfer: %u, FVNoDrop: %u",
		item->NoDrop, item->NoRent, (uint8)item->NoPet, (uint8)item->NoTransfer, item->FVNoDrop);

	if (item->IsClassBook()) {
		c->Message(0, "*** This item is a Book (filename:'%s') ***", item->Filename);
	}
	else if (item->IsClassBag()) {
		c->Message(0, "*** This item is a Container (%u slots) ***", item->BagSlots);
	}
	else {
		c->Message(0, "*** This item is Common ***");
		c->Message(0, ">> Classes: %u, Races: %u, Slots: %u", item->Classes, item->Races, item->Slots);
		c->Message(0, ">> ReqSkill: %u, ReqLevel: %u, RecLevel: %u", item->RecSkill, item->ReqLevel, item->RecLevel);
		c->Message(0, ">> SkillModType: %u, SkillModValue: %i", item->SkillModType, item->SkillModValue);
		c->Message(0, ">> BaneRaceType: %u, BaneRaceDamage: %u, BaneBodyType: %u, BaneBodyDamage: %i",
			item->BaneDmgRace, item->BaneDmgRaceAmt, item->BaneDmgBody, item->BaneDmgAmt);
		c->Message(0, ">> Magic: %s, SpellID: %i, ProcLevel: %u, Charges: %u, MaxCharges: %u",
			(item->Magic ? "TRUE" : "FALSE"), item->Click.Effect, item->Click.Level, inst->GetCharges(), item->MaxCharges);
		c->Message(0, ">> EffectType: 0x%02X, CastTime: %.2f", (uint8)item->Click.Type, ((double)item->CastTime / 1000));
	}

	if (c->Admin() >= 200)
		c->Message(0, ">> MinStatus: %u", item->MinStatus);
}

void command_uptime(Client *c, const Seperator *sep)
{
	if (!worldserver.Connected())
		c->Message(0, "Error: World server disconnected");
	else
	{
		auto pack = new ServerPacket(ServerOP_Uptime, sizeof(ServerUptime_Struct));
		ServerUptime_Struct* sus = (ServerUptime_Struct*) pack->pBuffer;
		strcpy(sus->adminname, c->GetName());
		if (sep->IsNumber(1) && atoi(sep->arg[1]) > 0)
			sus->zoneserverid = atoi(sep->arg[1]);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void command_flag(Client *c, const Seperator *sep)
{
	if(sep->arg[2][0] == 0) {
		if (!c->GetTarget() || (c->GetTarget() && c->GetTarget() == c)) {
			c->UpdateAdmin();
			c->Message(0, "Refreshed your admin flag from DB.");
		} else if (c->GetTarget() && c->GetTarget() != c && c->GetTarget()->IsClient()) {
			c->GetTarget()->CastToClient()->UpdateAdmin();
			c->Message(0, "%s's admin flag has been refreshed.", c->GetTarget()->GetName());
			c->GetTarget()->Message(0, "%s refreshed your admin flag.", c->GetName());
		}
	}
	else if (!sep->IsNumber(1) || atoi(sep->arg[1]) < -2 || atoi(sep->arg[1]) > 255 || strlen(sep->arg[2]) == 0)
		c->Message(0, "Usage: #flag [status] [acctname]");

	else if (c->Admin() < commandChangeFlags) {
//this check makes banning players by less than this level
//impossible, but i'll leave it in anyways
		c->Message(0, "You may only refresh your own flag, doing so now.");
		c->UpdateAdmin();
	}
	else {
		if (atoi(sep->arg[1]) > c->Admin())
			c->Message(0, "You cannot set people's status to higher than your own");
		else if (atoi(sep->arg[1]) < 0 && c->Admin() < commandBanPlayers)
			c->Message(0, "You have too low of status to suspend/ban");
		else if (!database.SetAccountStatus(sep->argplus[2], atoi(sep->arg[1])))
			c->Message(0, "Unable to set GM Flag.");
		else {
			c->Message(0, "Set GM Flag on account.");
			auto pack = new ServerPacket(ServerOP_FlagUpdate, 6);
			*((uint32*) pack->pBuffer) = database.GetAccountIDByName(sep->argplus[2]);
			*((int16*) &pack->pBuffer[4]) = atoi(sep->arg[1]);
			worldserver.SendPacket(pack);
			delete pack;
		}
	}
}

void command_time(Client *c, const Seperator *sep)
{
	char timeMessage[255];
	int minutes=0;
	if(sep->IsNumber(1)) {
		if(sep->IsNumber(2)) {
			minutes=atoi(sep->arg[2]);
		}
		c->Message(13, "Setting world time to %s:%i (Timezone: 0)...",  sep->arg[1], minutes);
		zone->SetTime(atoi(sep->arg[1])+1, minutes);
		Log(Logs::General, Logs::Zone_Server, "%s :: Setting world time to %s:%i (Timezone: 0)...", c->GetCleanName(), sep->arg[1], minutes);
	}
	else {
		c->Message(13, "To set the Time: #time HH [MM]");
		TimeOfDay_Struct eqTime;
		zone->zone_time.GetCurrentEQTimeOfDay( time(0), &eqTime);
		sprintf(timeMessage,"%02d:%s%d %s (Timezone: %ih %im)",
			((eqTime.hour - 1) % 12) == 0 ? 12 : ((eqTime.hour - 1) % 12),
			(eqTime.minute < 10) ? "0" : "",
			eqTime.minute,
			(eqTime.hour >= 13) ? "pm" : "am",
			zone->zone_time.getEQTimeZoneHr(),
			zone->zone_time.getEQTimeZoneMin()
			);
		c->Message(13, "It is now %s.", timeMessage);
		Log(Logs::General, Logs::Zone_Server, "Current Time is: %s", timeMessage);
	}
}

void command_guild(Client *c, const Seperator *sep)
{
	int admin=c->Admin();
	Mob *target=c->GetTarget();

	if (strcasecmp(sep->arg[1], "help") == 0) {
		c->Message(0, "GM Guild commands:");
		c->Message(0, "  #guild list - lists all guilds on the server");
		c->Message(0, "  #guild create {guildleader charname or CharID} guildname");
		c->Message(0, "  #guild delete guildID");
		c->Message(0, "  #guild rename guildID newname");
		c->Message(0, "  #guild set charname guildID    (0=no guild)");
		c->Message(0, "  #guild setrank charname rank");
		c->Message(0, "  #guild setleader guildID {guildleader charname or CharID}");
	}
	else if (strcasecmp(sep->arg[1], "status") == 0 || strcasecmp(sep->arg[1], "stat") == 0) {
		Client* client = 0;
		if (sep->arg[2][0] != 0)
			client = entity_list.GetClientByName(sep->argplus[2]);
		else if (target != 0 && target->IsClient())
			client = target->CastToClient();
		if (client == 0)
			c->Message(0, "You must target someone or specify a character name");
		else if ((client->Admin() >= minStatusToEditOtherGuilds && admin < minStatusToEditOtherGuilds) && client->GuildID() != c->GuildID()) // no peeping for GMs, make sure tell message stays the same
			c->Message(0, "You must target someone or specify a character name.");
		else {
			if (client->IsInAGuild())
				c->Message(0, "%s is not in a guild.",  client->GetName());
			else if (guild_mgr.IsGuildLeader(client->GuildID(), client->CharacterID()))
				c->Message(0, "%s is the leader of <%s> rank: %s",  client->GetName(), guild_mgr.GetGuildName(client->GuildID()), guild_mgr.GetRankName(client->GuildID(), client->GuildRank()));
			else
				c->Message(0, "%s is a member of <%s> rank: %s",  client->GetName(), guild_mgr.GetGuildName(client->GuildID()), guild_mgr.GetRankName(client->GuildID(), client->GuildRank()));
		}
	}
	else if (strcasecmp(sep->arg[1], "info") == 0) {
		if (sep->arg[2][0] == 0 && c->IsInAGuild()) {
			if (admin >= minStatusToEditOtherGuilds)
				c->Message(0, "Usage: #guildinfo guild_id");
			else
				c->Message(0, "You're not in a guild");
		}
		else {
			uint32 tmp = GUILD_NONE;
			if (sep->arg[2][0] == 0)
				tmp = c->GuildID();
			else if (admin >= minStatusToEditOtherGuilds)
				tmp = atoi(sep->arg[2]);

			if(tmp != GUILD_NONE)
				guild_mgr.DescribeGuild(c, tmp);
		}
	}
	/*
	else if (strcasecmp(sep->arg[1], "edit") == 0) {
		if (c->GuildDBID() == 0)
			c->Message(0, "You arent in a guild!");
		else if (!sep->IsNumber(2))
			c->Message(0, "Error: invalid rank #.");
		else if (atoi(sep->arg[2]) < 0 || atoi(sep->arg[2]) > GUILD_MAX_RANK)
			c->Message(0, "Error: invalid rank #.");
		else if (!c->GuildRank() == 0)
			c->Message(0, "You must be rank %s to use edit.",  guilds[c->GuildEQID()].rank[0].rankname);
		else if (!worldserver.Connected())
			c->Message(0, "Error: World server dirconnected");
		else {
			if (!helper_guild_edit(c, c->GuildDBID(), c->GuildEQID(), atoi(sep->arg[2]), sep->arg[3], sep->argplus[4])) {
				c->Message(0, "  #guild edit rank title newtitle");
				c->Message(0, "  #guild edit rank permission 0/1");
			}
			else {
				ServerPacket* pack = new ServerPacket(ServerOP_RefreshGuild, 5);
				int32 geqid=c->GuildEQID();
				memcpy(pack->pBuffer, &geqid, 4);
				worldserver.SendPacket(pack);
				safe_delete(pack);
			}
		}
	}
	else if (strcasecmp(sep->arg[1], "gmedit") == 0 && admin >= 100) {
		if (!sep->IsNumber(2))
			c->Message(0, "Error: invalid guilddbid.");
		else if (!sep->IsNumber(3))
			c->Message(0, "Error: invalid rank #.");
		else if (atoi(sep->arg[3]) < 0 || atoi(sep->arg[3]) > GUILD_MAX_RANK)
			c->Message(0, "Error: invalid rank #.");
		else if (!worldserver.Connected())
			c->Message(0, "Error: World server dirconnected");
		else {
			uint32 eqid = database.GetGuildEQID(atoi(sep->arg[2]));
			if (eqid == GUILD_NONE)
				c->Message(0, "Error: Guild not found");
			else if (!helper_guild_edit(c, atoi(sep->arg[2]), eqid, atoi(sep->arg[3]), sep->arg[4], sep->argplus[5])) {
				c->Message(0, "  #guild gmedit guilddbid rank title newtitle");
				c->Message(0, "  #guild gmedit guilddbid rank permission 0/1");
			}
			else {
				ServerPacket* pack = new ServerPacket(ServerOP_RefreshGuild, 5);
				memcpy(pack->pBuffer, &eqid, 4);
				worldserver.SendPacket(pack);
				safe_delete(pack);
			}
		}
	}
	*/
	else if (strcasecmp(sep->arg[1], "set") == 0) {
		if (!sep->IsNumber(3))
			c->Message(0, "Usage: #guild set charname guildgbid (0 = clear guildtag)");
		else {
			uint32 guild_id = atoi(sep->arg[3]);

			if(guild_id == 0)
				guild_id = GUILD_NONE;
			else if(!guild_mgr.GuildExists(guild_id)) {
				c->Message(13, "Guild %d does not exist.",  guild_id);
				return;
			}

			uint32 charid = database.GetCharacterID(sep->arg[2]);
			if(charid == 0) {
				c->Message(13, "Unable to find character '%s'",  charid);
				return;
			}

			//we could do the checking we need for guild_mgr.CheckGMStatus, but im lazy right now
			if(admin < minStatusToEditOtherGuilds) {
				c->Message(13, "Access denied.");
				return;
			}

			if(guild_id == GUILD_NONE) {
				Log(Logs::Detail, Logs::Guilds, "%s: Removing %s (%d) from guild with GM command.",  c->GetName(),
					sep->arg[2], charid);
			} else {
				Log(Logs::Detail, Logs::Guilds, "%s: Putting %s (%d) into guild %s (%d) with GM command.",  c->GetName(),
					sep->arg[2], charid,
					guild_mgr.GetGuildName(guild_id), guild_id);
			}

			if(!guild_mgr.SetGuild(charid, guild_id, GUILD_MEMBER)) {
				c->Message(13, "Error putting '%s' into guild %d",  sep->arg[2], guild_id);
			} else {
				c->Message(0, "%s has been put into guild %d",  sep->arg[2], guild_id);
			}
		}
	}
	/*else if (strcasecmp(sep->arg[1], "setdoor") == 0 && admin >= minStatusToEditOtherGuilds) {

		if (!sep->IsNumber(2))
			c->Message(0, "Usage: #guild setdoor guildEQid (0 = delete guilddoor)");
		else {
// guild doors
			if((!guilds[atoi(sep->arg[2])].databaseID) && (atoi(sep->arg[2])!=0) )
			{

				c->Message(0, "These is no guild with this guildEQid");
			}
			else {
				c->SetIsSettingGuildDoor(true);
				c->Message(0, "Click on a door you want to become a guilddoor");
				c->SetSetGuildDoorID(atoi(sep->arg[2]));
			}
		}
	}*/
	else if (strcasecmp(sep->arg[1], "setrank") == 0) {
		int rank = atoi(sep->arg[3]);
		if (!sep->IsNumber(3))
			c->Message(0, "Usage: #guild setrank charname rank");
		else if (rank < 0 || rank > GUILD_MAX_RANK)
			c->Message(0, "Error: invalid rank #.");
		else {
			uint32 charid = database.GetCharacterID(sep->arg[2]);
			if(charid == 0) {
				c->Message(13, "Unable to find character '%s'",  charid);
				return;
			}

			//we could do the checking we need for guild_mgr.CheckGMStatus, but im lazy right now
			if(admin < minStatusToEditOtherGuilds) {
				c->Message(13, "Access denied.");
				return;
			}

			Log(Logs::Detail, Logs::Guilds, "%s: Setting %s (%d)'s guild rank to %d with GM command.",  c->GetName(),
				sep->arg[2], charid, rank);

			if(!guild_mgr.SetGuildRank(charid, rank))
				c->Message(13, "Error while setting rank %d on '%s'.",  rank, sep->arg[2]);
			else
				c->Message(0, "%s has been set to rank %d",  sep->arg[2], rank);
		}
	}
	else if (strcasecmp(sep->arg[1], "create") == 0) {
		if (sep->arg[3][0] == 0)
			c->Message(0, "Usage: #guild create {guildleader charname or CharID} guild name");
		else if (!worldserver.Connected())
			c->Message(0, "Error: World server dirconnected");
		else {
			uint32 leader = 0;
			if (sep->IsNumber(2)) {
				leader = atoi(sep->arg[2]);
			} else if((leader=database.GetCharacterID(sep->arg[2])) != 0) {
				//got it from the db..
			} else {
				c->Message(13, "Unable to find char '%s'",  sep->arg[2]);
				return;
			}
			if (leader == 0) {
				c->Message(0, "Guild leader not found.");
				return;
			}

			uint32 tmp = guild_mgr.FindGuildByLeader(leader);
			if (tmp != GUILD_NONE) {
				c->Message(0, "Error: %s already is the leader of DB# %i '%s'.",  sep->arg[2], tmp, guild_mgr.GetGuildName(tmp));
			}
			else {

				if(admin < minStatusToEditOtherGuilds) {
					c->Message(13, "Access denied.");
					return;
				}

				uint32 id = guild_mgr.CreateGuild(sep->argplus[3], leader);

				Log(Logs::Detail, Logs::Guilds, "%s: Creating guild %s with leader %d with GM command. It was given id %lu.",  c->GetName(),
					sep->argplus[3], leader, (unsigned long)id);

				if (id == GUILD_NONE)
					c->Message(0, "Guild creation failed.");
				else {
					c->Message(0, "Guild created: Leader: %i, number %i: %s",  leader, id, sep->argplus[3]);

					if(!guild_mgr.SetGuild(leader, id, GUILD_LEADER))
						c->Message(0, "Unable to set guild leader's guild in the database. Your going to have to run #guild set");
				}

			}
		}
	}
	else if (strcasecmp(sep->arg[1], "delete") == 0) {
		if (!sep->IsNumber(2))
			c->Message(0, "Usage: #guild delete guildID");
		else if (!worldserver.Connected())
			c->Message(0, "Error: World server dirconnected");
		else {
			uint32 id = atoi(sep->arg[2]);

			if(!guild_mgr.GuildExists(id)) {
				c->Message(0, "Guild %d does not exist!",  id);
				return;
			}

			if(admin < minStatusToEditOtherGuilds) {
				//this person is not allowed to just edit any guild, check this guild's min status.
				if(c->GuildID() != id) {
					c->Message(13, "Access denied to edit other people's guilds");
					return;
				} else if(!guild_mgr.CheckGMStatus(id, admin)) {
					c->Message(13, "Access denied to edit your guild with GM commands.");
					return;
				}
			}

			Log(Logs::Detail, Logs::Guilds, "%s: Deleting guild %s (%d) with GM command.",  c->GetName(),
				guild_mgr.GetGuildName(id), id);

			if (!guild_mgr.DeleteGuild(id))
				c->Message(0, "Guild delete failed.");
			else {
				c->Message(0, "Guild %d deleted.",  id);
			}
		}
	}
	else if (strcasecmp(sep->arg[1], "rename") == 0) {
		if ((!sep->IsNumber(2)) || sep->arg[3][0] == 0)
			c->Message(0, "Usage: #guild rename guildID newname");
		else if (!worldserver.Connected())
			c->Message(0, "Error: World server dirconnected");
		else {
			uint32 id = atoi(sep->arg[2]);

			if(!guild_mgr.GuildExists(id)) {
				c->Message(0, "Guild %d does not exist!",  id);
				return;
			}

			if(admin < minStatusToEditOtherGuilds) {
				//this person is not allowed to just edit any guild, check this guild's min status.
				if(c->GuildID() != id) {
					c->Message(13, "Access denied to edit other people's guilds");
					return;
				} else if(!guild_mgr.CheckGMStatus(id, admin)) {
					c->Message(13, "Access denied to edit your guild with GM commands.");
					return;
				}
			}

			Log(Logs::Detail, Logs::Guilds, "%s: Renaming guild %s (%d) to '%s' with GM command.",  c->GetName(),
				guild_mgr.GetGuildName(id), id, sep->argplus[3]);

			if (!guild_mgr.RenameGuild(id, sep->argplus[3]))
				c->Message(0, "Guild rename failed.");
			else {
				c->Message(0, "Guild %d renamed to %s",  id, sep->argplus[3]);
			}
		}
	}
	else if (strcasecmp(sep->arg[1], "setleader") == 0) {
		if (sep->arg[3][0] == 0 || !sep->IsNumber(2))
			c->Message(0, "Usage: #guild setleader guild_id {guildleader charname or CharID}");
		else if (!worldserver.Connected())
			c->Message(0, "Error: World server dirconnected");
		else {
			uint32 leader = 0;
			if (sep->IsNumber(2)) {
				leader = atoi(sep->arg[2]);
			} else if((leader=database.GetCharacterID(sep->arg[2])) != 0) {
				//got it from the db..
			} else {
				c->Message(13, "Unable to find char '%s'",  sep->arg[2]);
				return;
			}

			uint32 tmpdb = guild_mgr.FindGuildByLeader(leader);
			if (leader == 0)
				c->Message(0, "New leader not found.");
			else if (tmpdb != 0) {
				c->Message(0, "Error: %s already is the leader of guild # %i",  sep->arg[2], tmpdb);
			}
			else {
				uint32 id = atoi(sep->arg[2]);

				if(!guild_mgr.GuildExists(id)) {
					c->Message(0, "Guild %d does not exist!",  id);
					return;
				}

				if(admin < minStatusToEditOtherGuilds) {
					//this person is not allowed to just edit any guild, check this guild's min status.
					if(c->GuildID() != id) {
						c->Message(13, "Access denied to edit other people's guilds");
						return;
					} else if(!guild_mgr.CheckGMStatus(id, admin)) {
						c->Message(13, "Access denied to edit your guild with GM commands.");
						return;
					}
				}

				Log(Logs::Detail, Logs::Guilds, "%s: Setting leader of guild %s (%d) to %d with GM command.",  c->GetName(),
					guild_mgr.GetGuildName(id), id, leader);

				if(!guild_mgr.SetGuildLeader(id, leader))
					c->Message(0, "Guild leader change failed.");
				else {
					c->Message(0, "Guild leader changed: guild # %d, Leader: %s",  id, sep->argplus[3]);
				}
			}
		}
	}
	else if (strcasecmp(sep->arg[1], "list") == 0) {
		if(admin < minStatusToEditOtherGuilds) {
			c->Message(13, "Access denied.");
			return;
		}
		guild_mgr.ListGuilds(c);
	}
	else {
		c->Message(0, "Unknown guild command, try #guild help");
	}
}
/*
bool helper_guild_edit(Client *c, uint32 dbid, uint32 eqid, uint8 rank, const char* what, const char* value) {
	struct GuildRankLevel_Struct grl;
	strcpy(grl.rankname, guild_mgr.GetRankName(eqid, rank));
	grl.demote = guilds[eqid].rank[rank].demote;
	grl.heargu = guilds[eqid].rank[rank].heargu;
	grl.invite = guilds[eqid].rank[rank].invite;
	grl.motd = guilds[eqid].rank[rank].motd;
	grl.promote = guilds[eqid].rank[rank].promote;
	grl.remove = guilds[eqid].rank[rank].remove;
	grl.speakgu = guilds[eqid].rank[rank].speakgu;
	grl.warpeace = guilds[eqid].rank[rank].warpeace;

	if (strcasecmp(what, "title") == 0) {
		if (strlen(value) > 100)
			c->Message(0, "Error: Title has a maxium length of 100 characters.");
		else
			strcpy(grl.rankname, value);
	}
	else if (rank == 0)
		c->Message(0, "Error: Rank 0's permissions can not be changed.");
	else {
		if (!(strlen(value) == 1 && (value[0] == '0' || value[0] == '1')))

			return false;
		if (strcasecmp(what, "demote") == 0)
			grl.demote = (value[0] == '1');
		else if (strcasecmp(what, "heargu") == 0)
			grl.heargu = (value[0] == '1');
		else if (strcasecmp(what, "invite") == 0)
			grl.invite = (value[0] == '1');
		else if (strcasecmp(what, "motd") == 0)
			grl.motd = (value[0] == '1');
		else if (strcasecmp(what, "promote") == 0)
			grl.promote = (value[0] == '1');
		else if (strcasecmp(what, "remove") == 0)

			grl.remove = (value[0] == '1');
		else if (strcasecmp(what, "speakgu") == 0)
			grl.speakgu = (value[0] == '1');
		else if (strcasecmp(what, "warpeace") == 0)
			grl.warpeace = (value[0] == '1');
		else
			c->Message(0, "Error: Permission name not recognized.");
	}
	if (!database.EditGuild(dbid, rank, &grl))
		c->Message(0, "Error: database.EditGuild() failed");
	return true;
}*/

void command_zonestatus(Client *c, const Seperator *sep)
{
	if (!worldserver.Connected())
		c->Message(0, "Error: World server disconnected");
	else {
		auto pack = new ServerPacket(ServerOP_ZoneStatus, strlen(c->GetName()) + 2);
		memset(pack->pBuffer, (uint8) c->Admin(), 1);
		strcpy((char *) &pack->pBuffer[1], c->GetName());
		worldserver.SendPacket(pack);
		delete pack;
	}
}

void command_doanim(Client *c, const Seperator *sep)
{
	if (!sep->IsNumber(1))
		c->Message(0, "Usage: #DoAnim [number]");
	else
		if (c->Admin() >= commandDoAnimOthers)
			if (c->GetTarget() == 0)
				c->Message(0, "Error: You need a target.");
			else
				c->GetTarget()->DoAnim(atoi(sep->arg[1]),atoi(sep->arg[2]));
		else
			c->DoAnim(atoi(sep->arg[1]),atoi(sep->arg[2]));
}

void command_randomfeatures(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	if (!target)
		c->Message(0,"Error: This command requires a target");
	else
	{
		if (target->RandomizeFeatures())
			c->Message(0,"Features Randomized");
		else
			c->Message(0,"This command requires a Playable Race as the target");
	}
}

void command_face(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	if (!sep->IsNumber(1))
		c->Message(0,"Usage: #face [number of face]");
	else if (!target)
		c->Message(0,"Error: this command requires a target");
	else {
		uint16 Race = target->GetRace();
		uint8 Gender = target->GetGender();
		uint8 Texture = 0xFF;
		uint8 HelmTexture = 0xFF;
		uint8 HairColor = target->GetHairColor();
		uint8 BeardColor = target->GetBeardColor();
		uint8 EyeColor1 = target->GetEyeColor1();
		uint8 EyeColor2 = target->GetEyeColor2();
		uint8 HairStyle = target->GetHairStyle();
		uint8 LuclinFace = atoi(sep->arg[1]);
		uint8 Beard = target->GetBeard();
		uint32 DrakkinHeritage = target->GetDrakkinHeritage();
		uint32 DrakkinTattoo = target->GetDrakkinTattoo();
		uint32 DrakkinDetails = target->GetDrakkinDetails();

		target->SendIllusionPacket(Race, Gender, Texture, HelmTexture, HairColor, BeardColor,
									EyeColor1, EyeColor2, HairStyle, LuclinFace, Beard, 0xFF,
									DrakkinHeritage, DrakkinTattoo, DrakkinDetails);

		c->Message(0,"Face = %i",  atoi(sep->arg[1]));
	}
}

void command_findaliases(Client *c, const Seperator *sep)
{
	if (!sep->arg[1][0]) {
		c->Message(0, "Usage: #findaliases [alias | command]");
		return;
	}

	auto find_iter = commandaliases.find(sep->arg[1]);
	if (find_iter == commandaliases.end()) {
		c->Message(15, "No commands or aliases match '%s'", sep->arg[1]);
		return;
	}

	auto command_iter = commandlist.find(find_iter->second);
	if (find_iter->second.empty() || command_iter == commandlist.end()) {
		c->Message(0, "An unknown condition occurred...");
		return;
	}

	c->Message(0, "Available command aliases for '%s':", command_iter->first.c_str());

	int commandaliasesshown = 0;
	for (auto alias_iter = commandaliases.begin(); alias_iter != commandaliases.end(); ++alias_iter) {
		if (strcasecmp(find_iter->second.c_str(), alias_iter->second.c_str()) || c->Admin() < command_iter->second->access)
			continue;

		c->Message(0, "%c%s", COMMAND_CHAR, alias_iter->first.c_str());
		++commandaliasesshown;
	}
	c->Message(0, "%d command alias%s listed.", commandaliasesshown, commandaliasesshown != 1 ? "es" : "");
}

void command_details(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	if (!sep->IsNumber(1))
		c->Message(0,"Usage: #details [number of drakkin detail]");
	else if (!target)
		c->Message(0,"Error: this command requires a target");
	else {
		uint16 Race = target->GetRace();
		uint8 Gender = target->GetGender();
		uint8 Texture = 0xFF;
		uint8 HelmTexture = 0xFF;
		uint8 HairColor = target->GetHairColor();
		uint8 BeardColor = target->GetBeardColor();
		uint8 EyeColor1 = target->GetEyeColor1();
		uint8 EyeColor2 = target->GetEyeColor2();
		uint8 HairStyle = target->GetHairStyle();
		uint8 LuclinFace = target->GetLuclinFace();
		uint8 Beard = target->GetBeard();
		uint32 DrakkinHeritage = target->GetDrakkinHeritage();
		uint32 DrakkinTattoo = target->GetDrakkinTattoo();
		uint32 DrakkinDetails = atoi(sep->arg[1]);

		target->SendIllusionPacket(Race, Gender, Texture, HelmTexture, HairColor, BeardColor,
									EyeColor1, EyeColor2, HairStyle, LuclinFace, Beard, 0xFF,
									DrakkinHeritage, DrakkinTattoo, DrakkinDetails);

		c->Message(0,"Details = %i",  atoi(sep->arg[1]));
	}
}

void command_heritage(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	if (!sep->IsNumber(1))
		c->Message(0,"Usage: #heritage [number of Drakkin heritage]");
	else if (!target)
		c->Message(0,"Error: this command requires a target");
	else {
		uint16 Race = target->GetRace();
		uint8 Gender = target->GetGender();
		uint8 Texture = 0xFF;
		uint8 HelmTexture = 0xFF;
		uint8 HairColor = target->GetHairColor();
		uint8 BeardColor = target->GetBeardColor();
		uint8 EyeColor1 = target->GetEyeColor1();
		uint8 EyeColor2 = target->GetEyeColor2();
		uint8 HairStyle = target->GetHairStyle();
		uint8 LuclinFace = target->GetLuclinFace();
		uint8 Beard = target->GetBeard();
		uint32 DrakkinHeritage = atoi(sep->arg[1]);
		uint32 DrakkinTattoo = target->GetDrakkinTattoo();
		uint32 DrakkinDetails = target->GetDrakkinDetails();

		target->SendIllusionPacket(Race, Gender, Texture, HelmTexture, HairColor, BeardColor,
									EyeColor1, EyeColor2, HairStyle, LuclinFace, Beard, 0xFF,
									DrakkinHeritage, DrakkinTattoo, DrakkinDetails);

		c->Message(0,"Heritage = %i",  atoi(sep->arg[1]));
	}
}

void command_tattoo(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	if (!sep->IsNumber(1))
		c->Message(0,"Usage: #tattoo [number of Drakkin tattoo]");
	else if (!target)
		c->Message(0,"Error: this command requires a target");
	else {
		uint16 Race = target->GetRace();
		uint8 Gender = target->GetGender();
		uint8 Texture = 0xFF;
		uint8 HelmTexture = 0xFF;
		uint8 HairColor = target->GetHairColor();
		uint8 BeardColor = target->GetBeardColor();
		uint8 EyeColor1 = target->GetEyeColor1();
		uint8 EyeColor2 = target->GetEyeColor2();
		uint8 HairStyle = target->GetHairStyle();
		uint8 LuclinFace = target->GetLuclinFace();
		uint8 Beard = target->GetBeard();
		uint32 DrakkinHeritage = target->GetDrakkinHeritage();
		uint32 DrakkinTattoo = atoi(sep->arg[1]);
		uint32 DrakkinDetails = target->GetDrakkinDetails();

		target->SendIllusionPacket(Race, Gender, Texture, HelmTexture, HairColor, BeardColor,
									EyeColor1, EyeColor2, HairStyle, LuclinFace, Beard, 0xFF,
									DrakkinHeritage, DrakkinTattoo, DrakkinDetails);

		c->Message(0,"Tattoo = %i",  atoi(sep->arg[1]));
	}
}

void command_helm(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	if (!sep->IsNumber(1))
		c->Message(0,"Usage: #helm [number of helm texture]");
	else if (!target)
		c->Message(0,"Error: this command requires a target");
	else {
		uint16 Race = target->GetRace();
		uint8 Gender = target->GetGender();
		uint8 Texture = 0xFF;
		uint8 HelmTexture = atoi(sep->arg[1]);
		uint8 HairColor = target->GetHairColor();
		uint8 BeardColor = target->GetBeardColor();
		uint8 EyeColor1 = target->GetEyeColor1();
		uint8 EyeColor2 = target->GetEyeColor2();
		uint8 HairStyle = target->GetHairStyle();
		uint8 LuclinFace = target->GetLuclinFace();
		uint8 Beard = target->GetBeard();
		uint32 DrakkinHeritage = target->GetDrakkinHeritage();
		uint32 DrakkinTattoo = target->GetDrakkinTattoo();
		uint32 DrakkinDetails = target->GetDrakkinDetails();

		target->SendIllusionPacket(Race, Gender, Texture, HelmTexture, HairColor, BeardColor,
									EyeColor1, EyeColor2, HairStyle, LuclinFace, Beard, 0xFF,
									DrakkinHeritage, DrakkinTattoo, DrakkinDetails);

		c->Message(0,"Helm = %i",  atoi(sep->arg[1]));
	}
}

void command_hair(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	if (!sep->IsNumber(1))
		c->Message(0,"Usage: #hair [number of hair style]");
	else if (!target)
		c->Message(0,"Error: this command requires a target");
	else {
		uint16 Race = target->GetRace();
		uint8 Gender = target->GetGender();
		uint8 Texture = 0xFF;
		uint8 HelmTexture = 0xFF;
		uint8 HairColor = target->GetHairColor();
		uint8 BeardColor = target->GetBeardColor();
		uint8 EyeColor1 = target->GetEyeColor1();
		uint8 EyeColor2 = target->GetEyeColor2();
		uint8 HairStyle = atoi(sep->arg[1]);
		uint8 LuclinFace = target->GetLuclinFace();
		uint8 Beard = target->GetBeard();
		uint32 DrakkinHeritage = target->GetDrakkinHeritage();
		uint32 DrakkinTattoo = target->GetDrakkinTattoo();
		uint32 DrakkinDetails = target->GetDrakkinDetails();

		target->SendIllusionPacket(Race, Gender, Texture, HelmTexture, HairColor, BeardColor,
									EyeColor1, EyeColor2, HairStyle, LuclinFace, Beard, 0xFF,
									DrakkinHeritage, DrakkinTattoo, DrakkinDetails);

		c->Message(0,"Hair = %i",  atoi(sep->arg[1]));
	}
}

void command_haircolor(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	if (!sep->IsNumber(1))
		c->Message(0,"Usage: #haircolor [number of hair color]");
	else if (!target)
		c->Message(0,"Error: this command requires a target");
	else {
		uint16 Race = target->GetRace();
		uint8 Gender = target->GetGender();
		uint8 Texture = 0xFF;
		uint8 HelmTexture = 0xFF;
		uint8 HairColor = atoi(sep->arg[1]);
		uint8 BeardColor = target->GetBeardColor();
		uint8 EyeColor1 = target->GetEyeColor1();
		uint8 EyeColor2 = target->GetEyeColor2();
		uint8 HairStyle = target->GetHairStyle();
		uint8 LuclinFace = target->GetLuclinFace();
		uint8 Beard = target->GetBeard();
		uint32 DrakkinHeritage = target->GetDrakkinHeritage();
		uint32 DrakkinTattoo = target->GetDrakkinTattoo();
		uint32 DrakkinDetails = target->GetDrakkinDetails();

		target->SendIllusionPacket(Race, Gender, Texture, HelmTexture, HairColor, BeardColor,
									EyeColor1, EyeColor2, HairStyle, LuclinFace, Beard, 0xFF,
									DrakkinHeritage, DrakkinTattoo, DrakkinDetails);

		c->Message(0,"Hair Color = %i",  atoi(sep->arg[1]));
	}
}

void command_beard(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	if (!sep->IsNumber(1))
		c->Message(0,"Usage: #beard [number of beard style]");
	else if (!target)
		c->Message(0,"Error: this command requires a target");
	else {
		uint16 Race = target->GetRace();
		uint8 Gender = target->GetGender();
		uint8 Texture = 0xFF;
		uint8 HelmTexture = 0xFF;
		uint8 HairColor = target->GetHairColor();
		uint8 BeardColor = target->GetBeardColor();
		uint8 EyeColor1 = target->GetEyeColor1();
		uint8 EyeColor2 = target->GetEyeColor2();
		uint8 HairStyle = target->GetHairStyle();
		uint8 LuclinFace = target->GetLuclinFace();
		uint8 Beard = atoi(sep->arg[1]);
		uint32 DrakkinHeritage = target->GetDrakkinHeritage();
		uint32 DrakkinTattoo = target->GetDrakkinTattoo();
		uint32 DrakkinDetails = target->GetDrakkinDetails();

		target->SendIllusionPacket(Race, Gender, Texture, HelmTexture, HairColor, BeardColor,
									EyeColor1, EyeColor2, HairStyle, LuclinFace, Beard, 0xFF,
									DrakkinHeritage, DrakkinTattoo, DrakkinDetails);

		c->Message(0,"Beard = %i",  atoi(sep->arg[1]));
	}
}

void command_beardcolor(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	if (!sep->IsNumber(1))
		c->Message(0,"Usage: #beardcolor [number of beard color]");
	else if (!target)
		c->Message(0,"Error: this command requires a target");
	else {
		uint16 Race = target->GetRace();
		uint8 Gender = target->GetGender();
		uint8 Texture = 0xFF;
		uint8 HelmTexture = 0xFF;
		uint8 HairColor = target->GetHairColor();
		uint8 BeardColor = atoi(sep->arg[1]);
		uint8 EyeColor1 = target->GetEyeColor1();
		uint8 EyeColor2 = target->GetEyeColor2();
		uint8 HairStyle = target->GetHairStyle();
		uint8 LuclinFace = target->GetLuclinFace();
		uint8 Beard = target->GetBeard();
		uint32 DrakkinHeritage = target->GetDrakkinHeritage();
		uint32 DrakkinTattoo = target->GetDrakkinTattoo();
		uint32 DrakkinDetails = target->GetDrakkinDetails();

		target->SendIllusionPacket(Race, Gender, Texture, HelmTexture, HairColor, BeardColor,
									EyeColor1, EyeColor2, HairStyle, LuclinFace, Beard, 0xFF,
									DrakkinHeritage, DrakkinTattoo, DrakkinDetails);

		c->Message(0,"Beard Color = %i",  atoi(sep->arg[1]));
	}
}

void command_scribespells(Client *c, const Seperator *sep)
{
	uint8 max_level, min_level;
	uint16 book_slot, curspell, count;
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM())
		t=c->GetTarget()->CastToClient();

	if(!sep->arg[1][0])
	{
		c->Message(0, "FORMAT: #scribespells <max level> <min level>");
		return;
	}

	max_level = (uint8)atoi(sep->arg[1]);
	if (!c->GetGM() && max_level > RuleI(Character, MaxLevel))
		max_level = RuleI(Character, MaxLevel);	//default to Character:MaxLevel if we're not a GM & it's higher than the max level
	min_level = sep->arg[2][0] ? (uint8)atoi(sep->arg[2]) : 1;	//default to 1 if there isn't a 2nd argument
	if (!c->GetGM() && min_level > RuleI(Character, MaxLevel))
		min_level = RuleI(Character, MaxLevel);	//default to Character:MaxLevel if we're not a GM & it's higher than the max level


	if(max_level < 1 || min_level < 1)
	{
		c->Message(0, "ERROR: Level must be greater than 1.");
		return;
	}
	if (min_level > max_level) {
		c->Message(0, "Error: Min Level must be less than or equal to Max Level.");
		return;
	}

	t->Message(0, "Scribing spells to spellbook.");
	if(t != c)
		c->Message(0, "Scribing spells for %s.",  t->GetName());
	Log(Logs::General, Logs::Normal, "Scribe spells request for %s from %s, levels: %u -> %u",  t->GetName(), c->GetName(), min_level, max_level);

	for(curspell = 0, book_slot = t->GetNextAvailableSpellBookSlot(), count = 0; curspell < SPDAT_RECORDS && book_slot < MAX_PP_SPELLBOOK; curspell++, book_slot = t->GetNextAvailableSpellBookSlot(book_slot))
	{
		if
		(
			spells[curspell].classes[WARRIOR] != 0 && // check if spell exists
			spells[curspell].classes[t->GetPP().class_-1] <= max_level &&	//maximum level
			spells[curspell].classes[t->GetPP().class_-1] >= min_level &&	//minimum level
			spells[curspell].skill != 52
		)
		{
			if (book_slot == -1) {	//no more book slots
				t->Message(13, "Unable to scribe spell %s (%u) to spellbook: no more spell book slots available.",  spells[curspell].name, curspell);
				if (t != c)
					c->Message(13, "Error scribing spells: %s ran out of spell book slots on spell %s (%u)",  t->GetName(), spells[curspell].name, curspell);
				break;
			}
			if(!IsDiscipline(curspell) && !t->HasSpellScribed(curspell)) {	//isn't a discipline & we don't already have it scribed
				t->ScribeSpell(curspell, book_slot);
				count++;
			}
		}
	}

	if (count > 0) {
		t->Message(0, "Successfully scribed %u spells.",  count);
		if (t != c)
			c->Message(0, "Successfully scribed %u spells for %s.",  count, t->GetName());
	} else {
		t->Message(0, "No spells scribed.");
		if (t != c)
			c->Message(0, "No spells scribed for %s.",  t->GetName());
	}
}

void command_scribespell(Client *c, const Seperator *sep) {
	uint16 spell_id = 0;
	uint16 book_slot = -1;
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM())
		t=c->GetTarget()->CastToClient();

	if(!sep->arg[1][0]) {
		c->Message(0, "FORMAT: #scribespell <spellid>");
		return;
	}

	spell_id = atoi(sep->arg[1]);

	if(IsValidSpell(spell_id)) {
		t->Message(0, "Scribing spell: %s (%i) to spellbook.",  spells[spell_id].name, spell_id);

		if(t != c)
			c->Message(0, "Scribing spell: %s (%i) for %s.",  spells[spell_id].name, spell_id, t->GetName());

		Log(Logs::General, Logs::Normal, "Scribe spell: %s (%i) request for %s from %s.",  spells[spell_id].name, spell_id, t->GetName(), c->GetName());

		if (spells[spell_id].classes[WARRIOR] != 0 && spells[spell_id].skill != 52 && spells[spell_id].classes[t->GetPP().class_ - 1] > 0 && !IsDiscipline(spell_id)) {
			book_slot = t->GetNextAvailableSpellBookSlot();

			if(book_slot >= 0 && t->FindSpellBookSlotBySpellID(spell_id) < 0)
				t->ScribeSpell(spell_id, book_slot);
			else {
				t->Message(13, "Unable to scribe spell: %s (%i) to your spellbook.",  spells[spell_id].name, spell_id);

				if(t != c)
					c->Message(13, "Unable to scribe spell: %s (%i) for %s.",  spells[spell_id].name, spell_id, t->GetName());
			}
		}
		else
			c->Message(13, "Your target can not scribe this spell.");
	}
	else
		c->Message(13, "Spell ID: %i is an unknown spell and cannot be scribed.",  spell_id);
}

void command_unscribespell(Client *c, const Seperator *sep) {
	uint16 spell_id = 0;
	uint16 book_slot = -1;
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM())
		t=c->GetTarget()->CastToClient();

	if(!sep->arg[1][0]) {
		c->Message(0, "FORMAT: #unscribespell <spellid>");
		return;
	}

	spell_id = atoi(sep->arg[1]);

	if(IsValidSpell(spell_id)) {
		book_slot = t->FindSpellBookSlotBySpellID(spell_id);

		if(book_slot >= 0) {
			t->UnscribeSpell(book_slot);

			t->Message(0, "Unscribing spell: %s (%i) from spellbook.",  spells[spell_id].name, spell_id);

			if(t != c)
				c->Message(0, "Unscribing spell: %s (%i) for %s.",  spells[spell_id].name, spell_id, t->GetName());

			Log(Logs::General, Logs::Normal, "Unscribe spell: %s (%i) request for %s from %s.",  spells[spell_id].name, spell_id, t->GetName(), c->GetName());
		}
		else {
			t->Message(13, "Unable to unscribe spell: %s (%i) from your spellbook. This spell is not scribed.",  spells[spell_id].name, spell_id);

			if(t != c)
				c->Message(13, "Unable to unscribe spell: %s (%i) for %s due to spell not scribed.",  spells[spell_id].name, spell_id, t->GetName());
		}
	}
}

void command_unscribespells(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM())
		t=c->GetTarget()->CastToClient();

	t->UnscribeSpellAll();
}

void command_untraindisc(Client *c, const Seperator *sep) {
	Client *t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM())
		t = c->GetTarget()->CastToClient();
	
	for (int i = 0; i < MAX_PP_DISCIPLINES; i++) {
		if (t->GetPP().disciplines.values[i] == atoi(sep->arg[1])) {
			t->UntrainDisc(i, 1);
			return;
		}
	}
}

void command_untraindiscs(Client *c, const Seperator *sep) {
	Client *t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM())
		t = c->GetTarget()->CastToClient();
	
	t->UntrainDiscAll();
}

void command_wpinfo(Client *c, const Seperator *sep)
{
	Mob *t=c->GetTarget();

	if (t == nullptr || !t->IsNPC()) {
		c->Message(0,"You must target an NPC to use this.");
		return;
	}

	NPC *n = t->CastToNPC();
	n->DisplayWaypointInfo(c);
}

void command_wpadd(Client *c, const Seperator *sep)
{
	int	type1=0,
		type2=0,
		pause=0;	// Defaults for a new grid

	Mob *t=c->GetTarget();
	if (t && t->IsNPC())
	{
		Spawn2* s2info = t->CastToNPC()->respawn2;

		if(s2info == nullptr)	// Can't figure out where this mob's spawn came from... maybe a dynamic mob created by #spawn
		{
			c->Message(0,"#wpadd FAILED -- Can't determine which spawn record in the database this mob came from!");
			return;
		}

		if (sep->arg[1][0])
		{
			if (atoi(sep->arg[1]) >= 0)
				pause=atoi(sep->arg[1]);
			else
			{
				c->Message(0,"Usage: #wpadd [pause] [-h]");
				return;
			}
		}
		auto position = c->GetPosition();
		if (strcmp("-h",sep->arg[2]) != 0)
			position.w = -1;

		uint32 tmp_grid = database.AddWPForSpawn(c, s2info->GetID(), position, pause, type1, type2, zone->GetZoneID());
		if (tmp_grid)
			t->CastToNPC()->SetGrid(tmp_grid);

		t->CastToNPC()->AssignWaypoints(t->CastToNPC()->GetGrid());
		c->Message(0,"Waypoint added. Use #wpinfo to see waypoints for this NPC (may need to #repop first).");
	}
	else
		c->Message(0,"You must target an NPC to use this.");
}


void command_interrupt(Client *c, const Seperator *sep)
{
	uint16 ci_message=0x01b7, ci_color=0x0121;

	if(sep->arg[1][0])
		ci_message=atoi(sep->arg[1]);
	if(sep->arg[2][0])
		ci_color=atoi(sep->arg[2]);

	c->InterruptSpell(ci_message, ci_color);
}

void command_summonitem(Client *c, const Seperator *sep)
{
	uint32 itemid = 0;

	std::string cmd_msg = sep->msg;
	size_t link_open = cmd_msg.find('\x12');
	size_t link_close = cmd_msg.find_last_of('\x12');
	if (link_open != link_close && (cmd_msg.length() - link_open) > EQEmu::legacy::TEXT_LINK_BODY_LENGTH) {
		EQEmu::SayLinkBody_Struct link_body;
		EQEmu::saylink::DegenerateLinkBody(link_body, cmd_msg.substr(link_open + 1, EQEmu::legacy::TEXT_LINK_BODY_LENGTH));
		itemid = link_body.item_id;
	}
	else if (!sep->IsNumber(1)) {
		c->Message(0, "Usage: #summonitem [item id | link] [charges], charges are optional");
		return;
	}
	else {
		itemid = atoi(sep->arg[1]);
	}
	if (!itemid) {
		c->Message(0, "A valid item id number is required (derived: 0)");
		return;
	}

	int16 item_status = 0;
	const EQEmu::ItemData* item = database.GetItem(itemid);
	if (item) {
		item_status = static_cast<int16>(item->MinStatus);
	}

	if (item_status > c->Admin())
		c->Message(13, "Error: Insufficient status to summon this item.");
	else if (sep->argnum == 2 && sep->IsNumber(2))
		c->SummonItem(itemid, atoi(sep->arg[2]));
	else if (sep->argnum == 3)
		c->SummonItem(itemid, atoi(sep->arg[2]), atoi(sep->arg[3]));
	else if (sep->argnum == 4)
		c->SummonItem(itemid, atoi(sep->arg[2]), atoi(sep->arg[3]), atoi(sep->arg[4]));
	else if (sep->argnum == 5)
		c->SummonItem(itemid, atoi(sep->arg[2]), atoi(sep->arg[3]), atoi(sep->arg[4]), atoi(sep->arg[5]));
	else if (sep->argnum == 6)
		c->SummonItem(itemid, atoi(sep->arg[2]), atoi(sep->arg[3]), atoi(sep->arg[4]), atoi(sep->arg[5]), atoi(sep->arg[6]));
	else if (sep->argnum == 7)
		c->SummonItem(itemid, atoi(sep->arg[2]), atoi(sep->arg[3]), atoi(sep->arg[4]), atoi(sep->arg[5]), atoi(sep->arg[6]), atoi(sep->arg[7]));
	else if (sep->argnum == 8)
		c->SummonItem(itemid, atoi(sep->arg[2]), atoi(sep->arg[3]), atoi(sep->arg[4]), atoi(sep->arg[5]), atoi(sep->arg[6]), atoi(sep->arg[7]), atoi(sep->arg[8]));
	else {
		c->SummonItem(itemid);
	}
	
}

void command_giveitem(Client *c, const Seperator *sep)
{
	if (!sep->IsNumber(1)) {
		c->Message(13, "Usage: #summonitem [item id] [charges], charges are optional");
	} else if(c->GetTarget() == nullptr) {
		c->Message(13, "You must target a client to give the item to.");
	} else if(!c->GetTarget()->IsClient()) {
		c->Message(13, "You can only give items to players with this command.");
	} else {
		Client *t = c->GetTarget()->CastToClient();
		uint32 itemid = atoi(sep->arg[1]);
		int16 item_status = 0;
		const EQEmu::ItemData* item = database.GetItem(itemid);
		if(item) {
			item_status = static_cast<int16>(item->MinStatus);
		}

		if (item_status > c->Admin())
			c->Message(13, "Error: Insufficient status to summon this item.");
		else if (sep->argnum==2 && sep->IsNumber(2))
			t->SummonItem(itemid, atoi(sep->arg[2]));
		else if (sep->argnum==3)
			t->SummonItem(itemid, atoi(sep->arg[2]), atoi(sep->arg[3]));
		else if (sep->argnum==4)
			t->SummonItem(itemid, atoi(sep->arg[2]), atoi(sep->arg[3]), atoi(sep->arg[4]));
		else if (sep->argnum==5)
			t->SummonItem(itemid, atoi(sep->arg[2]), atoi(sep->arg[3]), atoi(sep->arg[4]), atoi(sep->arg[5]));
		else if (sep->argnum==6)
			t->SummonItem(itemid, atoi(sep->arg[2]), atoi(sep->arg[3]), atoi(sep->arg[4]), atoi(sep->arg[5]), atoi(sep->arg[6]));
		else if (sep->argnum==7)
			t->SummonItem(itemid, atoi(sep->arg[2]), atoi(sep->arg[3]), atoi(sep->arg[4]), atoi(sep->arg[5]), atoi(sep->arg[6]), atoi(sep->arg[7]));
		else if (sep->argnum == 7)
			t->SummonItem(itemid, atoi(sep->arg[2]), atoi(sep->arg[3]), atoi(sep->arg[4]), atoi(sep->arg[5]), atoi(sep->arg[6]), atoi(sep->arg[7]), atoi(sep->arg[8]));
		else {
			t->SummonItem(itemid);
		}
	}
}

void command_givemoney(Client *c, const Seperator *sep)
{
	if (!sep->IsNumber(1)) {	//as long as the first one is a number, we'll just let atoi convert the rest to 0 or a number
		c->Message(13, "Usage: #Usage: #givemoney [pp] [gp] [sp] [cp]");
	}
	else if(c->GetTarget() == nullptr) {
		c->Message(13, "You must target a player to give money to.");
	}
	else if(!c->GetTarget()->IsClient()) {
		c->Message(13, "You can only give money to players with this command.");
	}
	else {
		//TODO: update this to the client, otherwise the client doesn't show any weight change until you zone, move an item, etc
		c->GetTarget()->CastToClient()->AddMoneyToPP(atoi(sep->arg[4]), atoi(sep->arg[3]), atoi(sep->arg[2]), atoi(sep->arg[1]), true);
		c->Message(0, "Added %i Platinum, %i Gold, %i Silver, and %i Copper to %s's inventory.",  atoi(sep->arg[1]), atoi(sep->arg[2]), atoi(sep->arg[3]), atoi(sep->arg[4]), c->GetTarget()->GetName());
	}
}

void command_itemsearch(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0)
		c->Message(0, "Usage: #itemsearch [search string]");
	else
	{
		const char *search_criteria=sep->argplus[1];

		const EQEmu::ItemData* item = nullptr;
		std::string item_link;
		EQEmu::SayLinkEngine linker;
		linker.SetLinkType(EQEmu::saylink::SayLinkItemData);

		if (Seperator::IsNumber(search_criteria)) {
			item = database.GetItem(atoi(search_criteria));
			if (item) {
				linker.SetItemData(item);

				item_link = linker.GenerateLink();

				c->Message(0, "%u: %s",  item->ID, item_link.c_str());
			}
			else {
				c->Message(0, "Item #%s not found",  search_criteria);
			}

			return;
		}

		int count = 0;
		char sName[64];
		char sCriteria[255];
		strn0cpy(sCriteria, search_criteria, sizeof(sCriteria));
		strupr(sCriteria);
		char* pdest;
		uint32 it = 0;
		while ((item = database.IterateItems(&it))) {
			strn0cpy(sName, item->Name, sizeof(sName));
			strupr(sName);
			pdest = strstr(sName, sCriteria);
			if (pdest != nullptr) {
				linker.SetItemData(item);

				item_link = linker.GenerateLink();

				c->Message(0, "%u: %s",  item->ID, item_link.c_str());

				++count;
			}

			if (count == 50)
				break;
		}

		if (count == 50)
			c->Message(0, "50 items shown...too many results.");
		else
			c->Message(0, "%i items found",  count);

	}
}

void command_setaaxp(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	if (sep->IsNumber(1)) {
		t->SetEXP(t->GetEXP(), atoi(sep->arg[1]), false);
		if(sep->IsNumber(2) && sep->IsNumber(3)) {
			t->SetLeadershipEXP(atoi(sep->arg[2]), atoi(sep->arg[3]));
		}
	} else
		c->Message(0, "Usage: #setaaxp <new AA XP value> (<new Group AA XP value> <new Raid XP value>)");
}

void command_setaapts(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	if(sep->arg[1][0] == '\0' || sep->arg[2][0] == '\0')
		c->Message(0, "Usage: #setaapts <AA|group|raid> <new AA points value>");
	else if(atoi(sep->arg[2]) <= 0 || atoi(sep->arg[2]) > 5000)
		c->Message(0, "You must have a number greater than 0 for points and no more than 5000.");
	else if(!strcasecmp(sep->arg[1], "group")) {
		t->GetPP().group_leadership_points = atoi(sep->arg[2]);
		t->GetPP().group_leadership_exp = 0;
		t->Message(MT_Experience, "Setting Group AA points to %u", t->GetPP().group_leadership_points);
		t->SendLeadershipEXPUpdate();
	} else if(!strcasecmp(sep->arg[1], "raid")) {
		t->GetPP().raid_leadership_points = atoi(sep->arg[2]);
		t->GetPP().raid_leadership_exp = 0;
		t->Message(MT_Experience, "Setting Raid AA points to %u", t->GetPP().raid_leadership_points);
		t->SendLeadershipEXPUpdate();
	} else {
		t->GetPP().aapoints = atoi(sep->arg[2]);
		t->GetPP().expAA = 0;
		t->Message(MT_Experience, "Setting personal AA points to %u", t->GetPP().aapoints);
		t->SendAlternateAdvancementStats();
	}
}

void command_setcrystals(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	if(sep->arg[1][0] == '\0' || sep->arg[2][0] == '\0')
		c->Message(0, "Usage: #setcrystals <radiant|ebon> <new crystal count value>");
	else if(atoi(sep->arg[2]) <= 0 || atoi(sep->arg[2]) > 100000)
		c->Message(0, "You must have a number greater than 0 for crystals and no more than 100000.");
	else if(!strcasecmp(sep->arg[1], "radiant"))
	{
		t->SetRadiantCrystals(atoi(sep->arg[2]));
		t->SendCrystalCounts();
		t->SaveCurrency();
	}
	else if(!strcasecmp(sep->arg[1], "ebon"))
	{
		t->SetEbonCrystals(atoi(sep->arg[2]));
		t->SendCrystalCounts();
		t->SaveCurrency();
	}
	else
	{
		c->Message(0, "Usage: #setcrystals <radiant|ebon> <new crystal count value>");
	}
}

void command_stun(Client *c, const Seperator *sep)
{
	Mob *t=c->CastToMob();
	uint32 duration;

	if(sep->arg[1][0])
	{
		duration = atoi(sep->arg[1]);
		if(c->GetTarget())
			t=c->GetTarget();
		if(t->IsClient())
			t->CastToClient()->Stun(duration);
		else
			t->CastToNPC()->Stun(duration);
	}
	else
		c->Message(0, "Usage: #stun [duration]");
}


void command_ban(Client *c, const Seperator *sep)
{
if(sep->arg[1][0] == 0 || sep->arg[2][0] == 0) {
		c->Message(0, "Usage: #ban <charname> <message>");
		return;
	}

    auto account_id = database.GetAccountIDByChar(sep->arg[1]);

    std::string message;
    int i = 2;
    while(1) {
        if(sep->arg[i][0] == 0) {
            break;
        }

        if(message.length() > 0) {
            message.push_back(' ');
        }

        message += sep->arg[i];
        ++i;
    }

    if(message.length() == 0) {
        c->Message(0, "Usage: #ban <charname> <message>");
        return;
    }

    if(account_id == 0) {
        c->Message(13, "Character does not exist.");
        return;
    }

    std::string query = StringFormat("UPDATE account SET status = -2, ban_reason = '%s' "
                                    "WHERE id = %i",  EscapeString(message).c_str(), account_id);
    auto results = database.QueryDatabase(query);

    c->Message(13, "Account number %i with the character %s has been banned with message: \"%s\"",  account_id, sep->arg[1], message.c_str());

    ServerPacket flagUpdatePack(ServerOP_FlagUpdate, 6);
    *((uint32*)&flagUpdatePack.pBuffer[0]) = account_id;
    *((int16*)&flagUpdatePack.pBuffer[4]) = -2;
    worldserver.SendPacket(&flagUpdatePack);

    Client *client = nullptr;
    client = entity_list.GetClientByName(sep->arg[1]);
    if(client) {
        client->WorldKick();
        return;
    }

    ServerPacket kickPlayerPack(ServerOP_KickPlayer, sizeof(ServerKickPlayer_Struct));
    ServerKickPlayer_Struct* skp = (ServerKickPlayer_Struct*)kickPlayerPack.pBuffer;
    strcpy(skp->adminname, c->GetName());
    strcpy(skp->name, sep->arg[1]);
    skp->adminrank = c->Admin();
    worldserver.SendPacket(&kickPlayerPack);
}

void command_suspend(Client *c, const Seperator *sep)
{
	if((sep->arg[1][0] == 0) || (sep->arg[2][0] == 0)) {
		c->Message(0, "Usage: #suspend <charname> <days> (Specify 0 days to lift the suspension immediately) <message>");
		return;
    }

    int duration = atoi(sep->arg[2]);

	if(duration < 0)
		duration = 0;

    std::string message;

    if(duration > 0) {
        int i = 3;
        while(1) {
            if(sep->arg[i][0] == 0) {
                break;
            }

            if(message.length() > 0) {
                message.push_back(' ');
            }

            message += sep->arg[i];
            ++i;
        }

        if(message.length() == 0) {
            c->Message(0, "Usage: #suspend <charname> <days>(Specify 0 days to lift the suspension immediately) <message>");
            return;
        }
    }

    auto escName = new char[strlen(sep->arg[1]) * 2 + 1];
    database.DoEscapeString(escName, sep->arg[1], strlen(sep->arg[1]));
    int accountID = database.GetAccountIDByChar(escName);
    safe_delete_array(escName);

    if (accountID <= 0) {
        c->Message(13,"Character does not exist.");
        return;
    }

	std::string query = StringFormat("UPDATE `account` SET `suspendeduntil` = DATE_ADD(NOW(), INTERVAL %i DAY), "
                                    "suspend_reason = '%s' WHERE `id` = %i",
                                    duration, EscapeString(message).c_str(), accountID);
    auto results = database.QueryDatabase(query);

    if(duration)
        c->Message(13,"Account number %i with the character %s has been temporarily suspended for %i day(s).",  accountID, sep->arg[1], duration);
    else
        c->Message(13,"Account number %i with the character %s is no longer suspended.",  accountID, sep->arg[1]);

    Client *bannedClient = entity_list.GetClientByName(sep->arg[1]);

    if(bannedClient) {
        bannedClient->WorldKick();
        return;
    }

    auto pack = new ServerPacket(ServerOP_KickPlayer, sizeof(ServerKickPlayer_Struct));
    ServerKickPlayer_Struct *sks = (ServerKickPlayer_Struct *)pack->pBuffer;

    strn0cpy(sks->adminname, c->GetName(), sizeof(sks->adminname));
    strn0cpy(sks->name, sep->arg[1], sizeof(sks->name));
    sks->adminrank = c->Admin();

    worldserver.SendPacket(pack);

    safe_delete(pack);
}

void command_ipban(Client *c, const Seperator *sep)
{
	if(sep->arg[1] == 0)
	{
		c->Message(0, "Usage: #ipban [xxx.xxx.xxx.xxx]");
	} else {
		if(database.AddBannedIP(sep->arg[1], c->GetName())) {
			c->Message(0, "%s has been successfully added to the Banned_IPs table by %s", sep->arg[1], c->GetName());
		} else {
			c->Message(0, "IPBan Failed (IP address is possibly already in the table?)");
		}
	}
}

void command_revoke(Client *c, const Seperator *sep)
{
	if(sep->arg[1][0] == 0 || sep->arg[2][0] == 0) {
		c->Message(0, "Usage: #revoke [charname] [1/0]");
		return;
	}

    uint32 characterID = database.GetAccountIDByChar(sep->arg[1]);
    if(characterID == 0) {
        c->Message(13,"Character does not exist.");
        return;
    }

    int flag = sep->arg[2][0] == '1' ? true : false;
    std::string query = StringFormat("UPDATE account SET revoked = %d WHERE id = %i",  flag, characterID);
    auto results = database.QueryDatabase(query);

    c->Message(13,"%s account number %i with the character %s.",  flag? "Revoking": "Unrevoking",  characterID, sep->arg[1]);

    Client* revokee = entity_list.GetClientByAccID(characterID);
    if(revokee) {
        c->Message(0, "Found %s in this zone.",  revokee->GetName());
        revokee->SetRevoked(flag);
        return;
    }

	c->Message(13, "#revoke: Couldn't find %s in this zone, passing request to worldserver.", sep->arg[1]);

	auto outapp = new ServerPacket(ServerOP_Revoke, sizeof(RevokeStruct));
	RevokeStruct *revoke = (RevokeStruct *)outapp->pBuffer;
	strn0cpy(revoke->adminname, c->GetName(), 64);
	strn0cpy(revoke->name, sep->arg[1], 64);
	revoke->toggle = flag;
	worldserver.SendPacket(outapp);
	safe_delete(outapp);
}

void command_oocmute(Client *c, const Seperator *sep)
{
	if(sep->arg[1][0] == 0 || !(sep->arg[1][0] == '1' || sep->arg[1][0] == '0'))
		c->Message(0, "Usage: #oocmute [1/0]");
	else {
		auto outapp = new ServerPacket(ServerOP_OOCMute, 1);
		*(outapp->pBuffer) = atoi(sep->arg[1]);
		worldserver.SendPacket(outapp);
		safe_delete(outapp);
	}
}

void command_checklos(Client *c, const Seperator *sep)
{
	if(c->GetTarget())
	{
//		if(c->CheckLos(c->GetTarget()))
		if(c->CheckLosFN(c->GetTarget()))
			c->Message(0, "You have LOS to %s",  c->GetTarget()->GetName());
		else
			c->Message(0, "You do not have LOS to %s",  c->GetTarget()->GetName());
	}
	else
	{
		c->Message(0, "ERROR: Target required");
	}
}

void command_set_adventure_points(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	if(!sep->arg[1][0])
	{
		c->Message(0, "Usage: #setadventurepoints [points] [theme]");
		return;
	}

	if(!sep->IsNumber(1) || !sep->IsNumber(2))
	{
		c->Message(0, "Usage: #setadventurepoints [points] [theme]");
		return;
	}

	c->Message(0, "Updating adventure points for %s",  t->GetName());
	t->UpdateLDoNPoints(atoi(sep->arg[1]), atoi(sep->arg[2]));
}

void command_npcsay(Client *c, const Seperator *sep)
{
	if(c->GetTarget() && c->GetTarget()->IsNPC() && sep->arg[1][0])
	{
		c->GetTarget()->Say(sep->argplus[1]);
	}
	else
	{
		c->Message(0, "Usage: #npcsay message (requires NPC target");
	}
}

void command_npcshout(Client *c, const Seperator *sep)
{
	if(c->GetTarget() && c->GetTarget()->IsNPC() && sep->arg[1][0])
	{
		c->GetTarget()->Shout(sep->argplus[1]);
	}
	else
	{
		c->Message(0, "Usage: #npcshout message (requires NPC target");
	}
}

void command_timers(Client *c, const Seperator *sep) {
	if(!c->GetTarget() || !c->GetTarget()->IsClient()) {
		c->Message(0,"Need a player target for timers.");
		return;
	}
	Client *them = c->GetTarget()->CastToClient();

	std::vector< std::pair<pTimerType, PersistentTimer *> > res;
	them->GetPTimers().ToVector(res);

	c->Message(0,"Timers for target:");

	int r;
	int l = res.size();
	for(r = 0; r < l; r++) {
		c->Message(0,"Timer %d: %d seconds remain.",  res[r].first, res[r].second->GetRemainingTime());
	}
}

void command_npcemote(Client *c, const Seperator *sep)
{
	if(c->GetTarget() && c->GetTarget()->IsNPC() && sep->arg[1][0])
	{
		c->GetTarget()->Emote(sep->argplus[1]);
	}
	else
	{
		c->Message(0, "Usage: #npcemote message (requires NPC target");
	}
}

void command_npcedit(Client *c, const Seperator *sep)
{	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(0, "Error: Must have NPC targeted");
		return;
	}

	if (strcasecmp(sep->arg[1], "help") == 0) {

		c->Message(0, "Help File for #npcedit. Syntax for commands are:");
		c->Message(0, "#npcedit Name - Sets an NPC's name");
		c->Message(0, "#npcedit Lastname - Sets an NPC's lastname");
		c->Message(0, "#npcedit Level - Sets an NPC's level");
		c->Message(0, "#npcedit Maxlevel - Sets an NPC's maximum level");
		c->Message(0, "#npcedit Race - Sets an NPC's race");
		c->Message(0, "#npcedit Class - Sets an NPC's class");
		c->Message(0, "#npcedit Bodytype - Sets an NPC's bodytype");
		c->Message(0, "#npcedit HP - Sets an NPC's hitpoints");
		c->Message(0, "#npcedit Gender - Sets an NPC's gender");
		c->Message(0, "#npcedit Texture - Sets an NPC's texture");
		c->Message(0, "#npcedit Helmtexture - Sets an NPC's helmet texture");
		c->Message(0, "#npcedit Armtexture - Sets an NPC's arm texture");
		c->Message(0, "#npcedit Bracertexture - Sets an NPC's bracer texture");
		c->Message(0, "#npcedit Handtexture - Sets an NPC's hand texture");
		c->Message(0, "#npcedit Legtexture - Sets an NPC's leg texture");
		c->Message(0, "#npcedit Feettexture - Sets an NPC's feettexture");
		c->Message(0, "#npcedit Herosforgemodel - Sets an NPC's Hero's Forge Model");
		c->Message(0, "#npcedit Size - Sets an NPC's size");
		c->Message(0, "#npcedit Hpregen - Sets an NPC's hitpoint regen rate per tick");
		c->Message(0, "#npcedit Manaregen - Sets an NPC's mana regen rate per tick");
		c->Message(0, "#npcedit Loottable - Sets the loottable ID for an NPC ");
		c->Message(0, "#npcedit Merchantid - Sets the merchant ID for an NPC");
		c->Message(0, "#npcedit alt_currency_id - Sets the Alternate Currency ID for an alterative currency Merchant");
		c->Message(0, "#npcedit npc_spells_effects_id - Sets the NPC Spell Effects ID");
		c->Message(0, "#npcedit adventure_template_id - Sets the NPC's Adventure Template ID");
		c->Message(0, "#npcedit trap_template - Sets the NPC's Trap Template ID");
		c->Message(0, "#npcedit special_abilities - Sets the NPC's Special Abilities");
		c->Message(0, "#npcedit Spell - Sets the npc spells list ID for an NPC");
		c->Message(0, "#npcedit Faction - Sets the NPC's faction id");
		c->Message(0, "#npcedit Damage - Sets an NPC's damage");
		c->Message(0, "#npcedit Meleetype - Sets an NPC's melee types");
		c->Message(0, "#npcedit Rangedtype - Sets an NPC's ranged type");
		c->Message(0, "#npcedit Ammoidfile - Sets an NPC's ammo id file");
		c->Message(0, "#npcedit Aggroradius - Sets an NPC's aggro radius");
		c->Message(0, "#npcedit Assistradius - Sets an NPC's assist radius");
		c->Message(0, "#npcedit Social - Set to 1 if an NPC should assist others on its faction");
		c->Message(0, "#npcedit Runspeed - Sets an NPC's run speed");
		c->Message(0, "#npcedit Walkspeed - Sets an NPC's walk speed");
		c->Message(0, "#npcedit AGI - Sets an NPC's Agility");
		c->Message(0, "#npcedit CHA - Sets an NPC's Charisma");
		c->Message(0, "#npcedit DEX - Sets an NPC's Dexterity");
		c->Message(0, "#npcedit INT - Sets an NPC's Intelligence");
		c->Message(0, "#npcedit STA - Sets an NPC's Stamina");
		c->Message(0, "#npcedit STR - Sets an NPC's Strength");
		c->Message(0, "#npcedit WIS - Sets an NPC's Wisdom");
		c->Message(0, "#npcedit MR - Sets an NPC's Magic Resistance");
		c->Message(0, "#npcedit PR - Sets an NPC's Poison Resistance");
		c->Message(0, "#npcedit DR - Sets an NPC's Disease Resistance");
		c->Message(0, "#npcedit FR - Sets an NPC's Fire Resistance");
		c->Message(0, "#npcedit CR - Sets an NPC's Cold Resistance");
		c->Message(0, "#npcedit Corrup - Sets an NPC's Corruption Resistance");
		c->Message(0, "#npcedit PhR - Sets and NPC's Physical Resistance");
		c->Message(0, "#npcedit Seeinvis - Sets an NPC's ability to see invis");
		c->Message(0, "#npcedit Seeinvisundead - Sets an NPC's ability to see through invis vs. undead");
		c->Message(0, "#npcedit Seehide - Sets an NPC's ability to see through hide");
		c->Message(0, "#npcedit Seeimprovedhide - Sets an NPC's ability to see through improved hide");
		c->Message(0, "#npcedit AC - Sets an NPC's Armor Class");
		c->Message(0, "#npcedit ATK - Sets an NPC's Attack");
		c->Message(0, "#npcedit Accuracy - Sets an NPC's Accuracy");
		c->Message(0, "#npcedit Avoidance - Sets an NPC's Avoidance");
		c->Message(0, "#npcedit npcaggro - Sets an NPC's npc_aggro flag");
		c->Message(0, "#npcedit qglobal - Sets an NPC's quest global flag");
		c->Message(0, "#npcedit spawn_limit - Sets an NPC's spawn limit counter");
		c->Message(0, "#npcedit Attackspeed - Sets an NPC's attack speed modifier");
		c->Message(0, "#npcedit Attackdelay - Sets an NPC's attack delay");
		c->Message(0, "#npcedit Attackcount - Sets an NPC's attack count");
		c->Message(0, "#npcedit findable - Sets an NPC's findable flag");
		c->Message(0, "#npcedit trackable - Sets an NPC's trackable flag");
		c->Message(0, "#npcedit weapon - Sets an NPC's primary and secondary weapon model");
		c->Message(0, "#npcedit featuresave - Saves all current facial features to the database");
		c->Message(0, "#npcedit color - Sets an NPC's red, green, and blue armor tint");
		c->Message(0, "#npcedit armortint_id - Set an NPC's Armor tint ID");
		c->Message(0, "#npcedit setanimation - Set an NPC's animation on spawn (Stored in spawn2 table)");
		c->Message(0, "#npcedit scalerate - Set an NPC's scaling rate");
		c->Message(0, "#npcedit healscale - Set an NPC's heal scaling rate");
		c->Message(0, "#npcedit spellscale - Set an NPC's spell scaling rate");
		c->Message(0, "#npcedit no_target - Set an NPC's ability to be targeted with the target hotkey");
		c->Message(0, "#npcedit version - Set an NPC's version");
		c->Message(0, "#npcedit slow_mitigation - Set an NPC's slow mitigation");

	}

	uint32 npcTypeID = c->GetTarget()->CastToNPC()->GetNPCTypeID();
	if (strcasecmp(sep->arg[1], "name") == 0) {
        c->Message(15,"NPCID %u now has the name %s.", npcTypeID, sep->argplus[2]);
		std::string query = StringFormat("UPDATE npc_types SET name = '%s' WHERE id = %i",  sep->argplus[2],npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "lastname") == 0) {
        c->Message(15,"NPCID %u now has the lastname %s.", npcTypeID, sep->argplus[2]);
		std::string query = StringFormat("UPDATE npc_types SET lastname = '%s' WHERE id = %i",  sep->argplus[2],npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "race") == 0) {
        c->Message(15,"NPCID %u now has the race %i.", npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET race = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "class") == 0) {
        c->Message(15,"NPCID %u is now class %i.", npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET class = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "bodytype") == 0) {
        c->Message(15,"NPCID %u now has type %i bodytype.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET bodytype = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "hp") == 0) {
        c->Message(15,"NPCID %u now has %i Hitpoints.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET hp = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "gender") == 0) {
        c->Message(15,"NPCID %u is now gender %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET gender = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "texture") == 0) {
        c->Message(15,"NPCID %u now uses texture %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET texture = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "helmtexture") == 0) {
        c->Message(15,"NPCID %u now uses helmtexture %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET helmtexture = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "armtexture") == 0) {
        c->Message(15,"NPCID %u now uses armtexture %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET armtexture = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "bracertexture") == 0) {
        c->Message(15,"NPCID %u now uses bracertexture %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET bracertexture = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "handtexture") == 0) {
        c->Message(15,"NPCID %u now uses handtexture %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET handtexture = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "legtexture") == 0) {
        c->Message(15,"NPCID %u now uses legtexture %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET legtexture = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "feettexture") == 0) {
        c->Message(15,"NPCID %u now uses feettexture %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET feettexture = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "herosforgemodel") == 0) {
        c->Message(15,"NPCID %u now uses herosforgemodel %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET herosforgemodel = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "size") == 0) {
        c->Message(15,"NPCID %u is now size %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET size = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "hpregen") == 0) {
        c->Message(15,"NPCID %u now regens %i hitpoints per tick.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET hp_regen_rate = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "manaregen") == 0) {
        c->Message(15,"NPCID %u now regens %i mana per tick.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET mana_regen_rate = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		database.QueryDatabase(query);
		return;
	}

    if (strcasecmp(sep->arg[1], "loottable") == 0) {
        c->Message(15,"NPCID %u is now on loottable_id %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET loottable_id = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "merchantid") == 0) {
        c->Message(15,"NPCID %u is now merchant_id %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET merchant_id = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "alt_currency_id") == 0) {
        c->Message(15,"NPCID %u now has field 'alt_currency_id' set to %s.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET alt_currency_id = '%s' WHERE id = %i",  sep->argplus[2],npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "npc_spells_effects_id") == 0) {
        c->Message(15,"NPCID %u now has field 'npc_spells_effects_id' set to %s.",  npcTypeID, sep->argplus[2]);
		std::string query = StringFormat("UPDATE npc_types SET npc_spells_effects_id = '%s' WHERE id = %i",  sep->argplus[2],npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "adventure_template_id") == 0) {
        c->Message(15,"NPCID %u now has field 'adventure_template_id' set to %s.",  npcTypeID, sep->argplus[2]);
		std::string query = StringFormat("UPDATE npc_types SET adventure_template_id = '%s' WHERE id = %i",  sep->argplus[2],npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "trap_template") == 0) {
        c->Message(15,"NPCID %u now has field 'trap_template' set to %s.",  npcTypeID, sep->argplus[2]);
		std::string query = StringFormat("UPDATE npc_types SET trap_template = '%s' WHERE id = %i",  sep->argplus[2],npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "special_abilities") == 0) {
        c->Message(15,"NPCID %u now has field 'special_abilities' set to %s.",  npcTypeID, sep->argplus[2]);
		std::string query = StringFormat("UPDATE npc_types SET special_abilities = '%s' WHERE id = %i",  sep->argplus[2],npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "spell") == 0) {
        c->Message(15,"NPCID %u now uses spell list %i",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET npc_spells_id = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "faction") == 0) {
        c->Message(15,"NPCID %u is now faction %i",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET npc_faction_id = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "damage") == 0) {
        c->Message(15,"NPCID %u now hits from %i to %i",  npcTypeID, atoi(sep->arg[2]), atoi(sep->arg[3]));
		std::string query = StringFormat("UPDATE npc_types SET mindmg = %i, maxdmg = %i WHERE id = %i",  atoi(sep->arg[2]), atoi(sep->arg[3]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "meleetype") == 0) {
        c->Message(15,"NPCID %u now has a primary melee type of %i and a secondary melee type of %i.",  npcTypeID, atoi(sep->arg[2]), atoi(sep->arg[3]));
		std::string query = StringFormat("UPDATE npc_types SET prim_melee_type = %i, sec_melee_type = %i WHERE id = %i",  atoi(sep->arg[2]), atoi(sep->arg[3]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "rangedtype") == 0) {
        c->Message(15,"NPCID %u now has a ranged type of %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET ranged_type = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "ammoidfile") == 0) {
        c->Message(15,"NPCID %u's ammo id file is now %i",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET ammoidfile = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "aggroradius") == 0) {
        c->Message(15,"NPCID %u now has an aggro radius of %i",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET aggroradius = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "assistradius") == 0) {
        c->Message(15,"NPCID %u now has an assist radius of %i",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET assistradius = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "social") == 0) {
        c->Message(15,"NPCID %u social status is now %i",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET social = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "runspeed") == 0) {
        c->Message(15,"NPCID %u now runs at %f",  npcTypeID, atof(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET runspeed = %f WHERE id = %i",  atof(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "walkspeed") == 0) {
        c->Message(15,"NPCID %u now walks at %f",  npcTypeID, atof(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET walkspeed = %f WHERE id = %i",  atof(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "AGI") == 0) {
        c->Message(15,"NPCID %u now has %i Agility.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET AGI = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "CHA") == 0) {
        c->Message(15,"NPCID %u now has %i Charisma.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET CHA = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "DEX") == 0) {
        c->Message(15,"NPCID %u now has %i Dexterity.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET DEX = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "INT") == 0) {
        c->Message(15,"NPCID %u now has %i Intelligence.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET _INT = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "STA") == 0) {
        c->Message(15,"NPCID %u now has %i Stamina.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET STA = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "STR") == 0) {
        c->Message(15,"NPCID %u now has %i Strength.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET STR = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "WIS") == 0) {
        c->Message(15,"NPCID %u now has a Magic Resistance of %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET WIS = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "MR") == 0) {
        c->Message(15,"NPCID %u now has a Magic Resistance of %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET MR = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "DR") == 0) {
        c->Message(15,"NPCID %u now has a Disease Resistance of %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET DR = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "CR") == 0) {
        c->Message(15,"NPCID %u now has a Cold Resistance of %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET CR = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

    if (strcasecmp(sep->arg[1], "FR") == 0) {
        c->Message(15,"NPCID %u now has a Fire Resistance of %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET FR = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

    if (strcasecmp(sep->arg[1], "PR") == 0) {
        c->Message(15,"NPCID %u now has a Poison Resistance of %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET PR = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "Corrup") == 0) {
        c->Message(15,"NPCID %u now has a Corruption Resistance of %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET corrup = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "PhR") == 0) {
        c->Message(15,"NPCID %u now has a Physical Resistance of %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET PhR = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "seeinvis") == 0) {
        c->Message(15,"NPCID %u now has seeinvis set to %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET see_invis = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "seeinvisundead") == 0) {
        c->Message(15,"NPCID %u now has seeinvisundead set to %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET see_invis_undead = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "seehide") == 0) {
        c->Message(15,"NPCID %u now has seehide set to %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET see_hide = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "seeimprovedhide") == 0) {
        c->Message(15,"NPCID %u now has seeimprovedhide set to %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET see_improved_hide = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "AC") == 0) {
        c->Message(15,"NPCID %u now has %i Armor Class.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET ac = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "ATK") == 0) {
        c->Message(15,"NPCID %u now has %i Attack.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET atk = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "Accuracy") == 0) {
        c->Message(15,"NPCID %u now has %i Accuracy.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET accuracy = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "Avoidance") == 0) {
        c->Message(15,"NPCID %u now has %i Avoidance.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET avoidance = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "level") == 0) {
        c->Message(15,"NPCID %u is now level %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET level = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "maxlevel") == 0) {
        c->Message(15,"NPCID %u now has a maximum level of %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET maxlevel = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "qglobal") == 0) {
        c->Message(15,"Quest globals have been %s for NPCID %u",  atoi(sep->arg[2]) == 0 ? "disabled" : "enabled",  npcTypeID);
		std::string query = StringFormat("UPDATE npc_types SET qglobal = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "npcaggro") == 0) {
        c->Message(15,"NPCID %u will now %s other NPCs with negative faction npc_value",  npcTypeID, atoi(sep->arg[2]) == 0? "not aggro": "aggro");
		std::string query = StringFormat("UPDATE npc_types SET npc_aggro = %i WHERE id = %i",  atoi(sep->argplus[2]) == 0? 0: 1, npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "spawn_limit") == 0) {
        c->Message(15,"NPCID %u now has a spawn limit of %i",  npcTypeID, atoi(sep->arg[2]));
		std::string query = StringFormat("UPDATE npc_types SET spawn_limit = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "Attackspeed") == 0) {
        c->Message(15,"NPCID %u now has attack_speed set to %f",  npcTypeID, atof(sep->arg[2]));
		std::string query = StringFormat("UPDATE npc_types SET attack_speed = %f WHERE id = %i",  atof(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "Attackdelay") == 0) {
		c->Message(15,"NPCID %u now has attack_delay set to %i", npcTypeID,atoi(sep->arg[2]));
		std::string query = StringFormat("UPDATE npc_types SET attack_delay = %i WHERE id = %i", atoi(sep->argplus[2]),npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "Attackcount") == 0) {
		c->Message(15,"NPCID %u now has attack_count set to %i", npcTypeID,atoi(sep->arg[2]));
		std::string query = StringFormat("UPDATE npc_types SET attack_count = %i WHERE id = %i", atoi(sep->argplus[2]),npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "findable") == 0) {
        c->Message(15,"NPCID %u is now %s",  npcTypeID, atoi(sep->arg[2]) == 0? "not findable": "findable");
		std::string query = StringFormat("UPDATE npc_types SET findable = %i WHERE id = %i",  atoi(sep->argplus[2]) == 0? 0: 1, npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "trackable") == 0) {
        c->Message(15,"NPCID %u is now %s",  npcTypeID, atoi(sep->arg[2]) == 0? "not trackable": "trackable");
		std::string query = StringFormat("UPDATE npc_types SET trackable = %i WHERE id = %i",  atoi(sep->argplus[2]) == 0? 0: 1, npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "weapon") == 0) {
        c->Message(15,"NPCID %u will have item graphic %i set to his primary and item graphic %i set to his secondary on repop.",   npcTypeID, atoi(sep->arg[2]), atoi(sep->arg[3]));
		std::string query = StringFormat("UPDATE npc_types SET d_melee_texture1 = %i, d_melee_texture2 = %i WHERE id = %i",  atoi(sep->arg[2]), atoi(sep->arg[3]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "featuresave") == 0) {
        c->Message(15,"NPCID %u saved with all current facial feature settings",  npcTypeID);
        Mob* target = c->GetTarget();
		std::string query = StringFormat("UPDATE npc_types "
                                        "SET luclin_haircolor = %i, luclin_beardcolor = %i, "
                                        "luclin_hairstyle = %i, luclin_beard = %i, "
                                        "face = %i, drakkin_heritage = %i, "
                                        "drakkin_tattoo = %i, drakkin_details = %i "
                                        "WHERE id = %i",
                                        target->GetHairColor(), target->GetBeardColor(),
                                        target->GetHairStyle(), target->GetBeard(),
                                        target->GetLuclinFace(), target->GetDrakkinHeritage(),
                                        target->GetDrakkinTattoo(), target->GetDrakkinDetails(),
                                        npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "color") == 0) {
        c->Message(15,"NPCID %u now has %i red, %i green, and %i blue tinting on their armor.",  npcTypeID, atoi(sep->arg[2]), atoi(sep->arg[3]), atoi(sep->arg[4]));
		std::string query = StringFormat("UPDATE npc_types SET armortint_red = %i, armortint_green = %i, armortint_blue = %i WHERE id = %i",  atoi(sep->arg[2]), atoi(sep->arg[3]), atoi(sep->arg[4]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "armortint_id") == 0) {
        c->Message(15,"NPCID %u now has field 'armortint_id' set to %s",  npcTypeID, sep->arg[2]);
		std::string query = StringFormat("UPDATE npc_types SET armortint_id = '%s' WHERE id = %i",  sep->argplus[2], npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "setanimation") == 0) {
		int animation = 0;
		if(sep->arg[2] && atoi(sep->arg[2]) <= 4) {
			if((strcasecmp(sep->arg[2], "stand" ) == 0) || atoi(sep->arg[2]) == 0)
				animation = 0; //Stand
			if((strcasecmp(sep->arg[2], "sit" ) == 0) || atoi(sep->arg[2]) == 1)
				animation = 1; //Sit
			if((strcasecmp(sep->arg[2], "crouch" ) == 0) || atoi(sep->arg[2]) == 2)
				animation = 2; //Crouch
			if((strcasecmp(sep->arg[2], "dead" ) == 0) || atoi(sep->arg[2]) == 3)
				animation = 3; //Dead
			if((strcasecmp(sep->arg[2], "loot" ) == 0) || atoi(sep->arg[2]) == 4)
				animation = 4; //Looting Animation
		} else {
			c->Message(0, "You must specifiy an animation stand, sit, crouch, dead, loot (0-4)");
			c->Message(0, "Example: #npcedit setanimation sit");
			c->Message(0, "Example: #npcedit setanimation 0");
			return;
		}

		c->Message(15,"NPCID %u now has the animation set to %i on spawn with spawngroup %i",  npcTypeID, animation, c->GetTarget()->CastToNPC()->GetSp2() );
		std::string query = StringFormat("UPDATE spawn2 SET animation = %i " "WHERE spawngroupID = %i",  animation, c->GetTarget()->CastToNPC()->GetSp2());
		database.QueryDatabase(query);

		c->GetTarget()->SetAppearance(EmuAppearance(animation));
		return;
	}

	if (strcasecmp(sep->arg[1], "scalerate") == 0) {
        c->Message(15,"NPCID %u now has a scaling rate of %i.",  npcTypeID, atoi(sep->arg[2]));
		std::string query = StringFormat("UPDATE npc_types SET scalerate = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "healscale") == 0) {
        c->Message(15, "NPCID %u now has a heal scaling rate of %i.",  npcTypeID, atoi(sep->arg[2]));
		std::string query = StringFormat("UPDATE npc_types SET healscale = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "spellscale") == 0) {
        c->Message(15, "NPCID %u now has a spell scaling rate of %i.",  npcTypeID, atoi(sep->arg[2]));
		std::string query = StringFormat("UPDATE npc_types SET spellscale = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "no_target") == 0) {
        c->Message(15, "NPCID %u is now %s.",  npcTypeID, atoi(sep->arg[2]) == 0? "targetable": "untargetable");
		std::string query = StringFormat("UPDATE npc_types SET no_target_hotkey = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "version") == 0) {
        c->Message(15, "NPCID %u is now version %i.",  npcTypeID, atoi(sep->arg[2]));
		std::string query = StringFormat("UPDATE npc_types SET version = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "slow_mitigation") == 0) {
        c->Message(15, "NPCID %u's slow mitigation limit is now %i.",  npcTypeID, atoi(sep->arg[2]));
		std::string query = StringFormat("UPDATE npc_types SET slow_mitigation = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		database.QueryDatabase(query);
		return;
	}

	if((sep->arg[1][0] == 0 || strcasecmp(sep->arg[1],"*")==0) || ((c->GetTarget()==0) || (c->GetTarget()->IsClient())))
		c->Message(0, "Type #npcedit help for more info");

}

#ifdef PACKET_PROFILER
void command_packetprofile(Client *c, const Seperator *sep) {
	Client *t = c;
	if(c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}
	c->DumpPacketProfile();
}
#endif

#ifdef EQPROFILE
void command_profiledump(Client *c, const Seperator *sep) {
	DumpZoneProfile();
}

void command_profilereset(Client *c, const Seperator *sep) {
	ResetZoneProfile();
}
#endif

void command_opcode(Client *c, const Seperator *sep) {
	if(!strcasecmp(sep->arg[1], "reload" )) {
		ReloadAllPatches();
		c->Message(0, "Opcodes for all patches have been reloaded");
	}
}

void command_qglobal(Client *c, const Seperator *sep) {
	//In-game switch for qglobal column
	if(sep->arg[1][0] == 0) {
		c->Message(0, "Syntax: #qglobal [on/off/view]. Requires NPC target.");
		return;
	}

	Mob *target = c->GetTarget();

	if(!target || !target->IsNPC()) {
		c->Message(13, "NPC Target Required!");
		return;
	}

	if(!strcasecmp(sep->arg[1], "on")) {
        std::string query = StringFormat("UPDATE npc_types SET qglobal = 1 WHERE id = '%i'",
                                        target->GetNPCTypeID());
        auto results = database.QueryDatabase(query);
		if(!results.Success()) {
			c->Message(15, "Could not update database.");
			return;
		}

        c->Message(15, "Success! Changes take effect on zone reboot.");
		return;
	}

	if(!strcasecmp(sep->arg[1], "off")) {
        std::string query = StringFormat("UPDATE npc_types SET qglobal = 0 WHERE id = '%i'",
                                        target->GetNPCTypeID());
        auto results = database.QueryDatabase(query);
		if(!results.Success()) {
			c->Message(15, "Could not update database.");
			return;
		}

        c->Message(15, "Success! Changes take effect on zone reboot.");
		return;
	}

	if(!strcasecmp(sep->arg[1], "view")) {
		const NPCType *type = database.LoadNPCTypesData(target->GetNPCTypeID());
		if(!type)
			c->Message(15, "Invalid NPC type.");
		else if(type->qglobal)
			c->Message(15, "This NPC has quest globals active.");
		else
			c->Message(15, "This NPC has quest globals disabled.");
		return;
	}

    c->Message(15, "Invalid action specified.");
}

void command_path(Client *c, const Seperator *sep)
{
	if(sep->arg[1][0] == '\0' || !strcasecmp(sep->arg[1], "help"))
	{
		c->Message(0, "Syntax: #path shownodes: Spawns a npc to represent every npc node.");
		c->Message(0, "#path info node_id: Gives information about node info (requires shownode target).");
		c->Message(0, "#path dump file_name: Dumps the current zone->pathing to a file of your naming.");
		c->Message(0, "#path add [requested_id]: Adds a node at your current location will try to take the requested id if possible.");
		c->Message(0, "#path connect connect_to_id [is_teleport] [door_id]: Connects the currently targeted node to connect_to_id's node and connects that node back (requires shownode target).");
		c->Message(0, "#path sconnect connect_to_id [is_teleport] [door_id]: Connects the currently targeted node to connect_to_id's node (requires shownode target).");
		c->Message(0, "#path qconnect [set]: short cut connect, connects the targeted node to the node you set with #path qconnect set (requires shownode target).");
		c->Message(0, "#path disconnect [all]/disconnect_from_id: Disconnects the currently targeted node to disconnect from disconnect from id's node (requires shownode target), if passed all as the second argument it will disconnect this node from every other node.");
		c->Message(0, "#path move: Moves your targeted node to your current position");
		c->Message(0, "#path process file_name: processes the map file and tries to automatically generate a rudimentary path setup and then dumps the current zone->pathing to a file of your naming.");
		c->Message(0, "#path resort [nodes]: resorts the connections/nodes after you've manually altered them so they'll work.");
		return;
	}
	if(!strcasecmp(sep->arg[1], "shownodes"))
	{
		if(zone->pathing)
			zone->pathing->SpawnPathNodes();

		return;
	}

	if(!strcasecmp(sep->arg[1], "info"))
	{
		if(zone->pathing)
		{
			zone->pathing->NodeInfo(c);
		}
		return;
	}

	if(!strcasecmp(sep->arg[1], "dump"))
	{
		if(zone->pathing)
		{
			if(sep->arg[2][0] == '\0')
				return;

			zone->pathing->DumpPath(sep->arg[2]);
		}
		return;
	}

	if(!strcasecmp(sep->arg[1], "add"))
	{
		if(zone->pathing)
		{
			float px = c->GetX();
			float py = c->GetY();
			float pz = c->GetZ();
			float best_z;

			if(zone->zonemap)
			{
				glm::vec3 loc(px, py, pz);
				best_z = zone->zonemap->FindBestZ(loc, nullptr);
			}
			else
			{
				best_z = pz;
			}
			int32 res = zone->pathing->AddNode(px, py, pz, best_z, atoi(sep->arg[2]));
			if(res >= 0)
			{
				c->Message(0, "Added Path Node: %i",  res);
			}
			else
			{
				c->Message(0, "Failed to add Path Node");
			}
		}
		else
		{
			zone->pathing = new PathManager();
			float px = c->GetX();
			float py = c->GetY();
			float pz = c->GetZ();
			float best_z;

			if(zone->zonemap)
			{
				glm::vec3 loc(px, py, pz);
				best_z = zone->zonemap->FindBestZ(loc, nullptr);
			}
			else
			{
				best_z = pz;
			}
			int32 res = zone->pathing->AddNode(px, py, pz, best_z, atoi(sep->arg[2]));
			if(res >= 0)
			{
				c->Message(0, "Added Path Node: %i",  res);
			}
			else
			{
				c->Message(0, "Failed to add Path Node");
			}
		}
		return;
	}

	if(!strcasecmp(sep->arg[1], "remove"))
	{
		if(zone->pathing)
		{
			if(zone->pathing->DeleteNode(c))
			{
				c->Message(0, "Removed Node.");
			}
			else
			{
				c->Message(0, "Unable to Remove Node.");
			}
		}
		return;
	}

	if(!strcasecmp(sep->arg[1], "connect"))
	{
		if(zone->pathing)
		{
			zone->pathing->ConnectNodeToNode(c, atoi(sep->arg[2]), atoi(sep->arg[3]), atoi(sep->arg[4]));
		}
		return;
	}

	if(!strcasecmp(sep->arg[1], "sconnect"))
	{
		if(zone->pathing)
		{
			zone->pathing->ConnectNode(c, atoi(sep->arg[2]), atoi(sep->arg[3]), atoi(sep->arg[4]));
		}
		return;
	}

	if(!strcasecmp(sep->arg[1], "qconnect"))
	{
		if(zone->pathing)
		{
			if(!strcasecmp(sep->arg[2], "set"))
			{
				zone->pathing->QuickConnect(c, true);
			}
			else
			{
				zone->pathing->QuickConnect(c, false);
			}
		}
		return;
	}

	if(!strcasecmp(sep->arg[1], "disconnect"))
	{
		if(zone->pathing)
		{
			if(!strcasecmp(sep->arg[2], "all"))
			{
				zone->pathing->DisconnectAll(c);
			}
			else
			{
				zone->pathing->DisconnectNodeToNode(c, atoi(sep->arg[2]));
			}
		}
		return;
	}


	if(!strcasecmp(sep->arg[1], "move"))
	{
		if(zone->pathing)
		{
			zone->pathing->MoveNode(c);
		}
		return;
	}

	if(!strcasecmp(sep->arg[1], "process"))
	{
		if(zone->pathing)
		{
			if(sep->arg[2][0] == '\0')
				return;

			zone->pathing->ProcessNodesAndSave(sep->arg[2]);
			c->Message(0, "Path processed...");
		}
		return;
	}

	if(!strcasecmp(sep->arg[1], "resort"))
	{
		if(zone->pathing)
		{
			if(!strcasecmp(sep->arg[2], "nodes"))
			{
				zone->pathing->SortNodes();
				c->Message(0, "Nodes resorted...");
			}
			else
			{
				zone->pathing->ResortConnections();
				c->Message(0, "Connections resorted...");
			}
		}
		return;
	}

	if(!strcasecmp(sep->arg[1], "hazard"))
	{
		if(zone->pathing)
		{
			if(c && c->GetTarget())
			{
				if (zone->pathing->NoHazardsAccurate(glm::vec3(c->GetX(), c->GetY(), c->GetZ()),
					glm::vec3(c->GetTarget()->GetX(), c->GetTarget()->GetY(), c->GetTarget()->GetZ())))
				{
					c->Message(0, "No hazards.");
				}
				else
				{
					c->Message(0, "Hazard Detected...");
				}
			}
		}
		return;
	}

	if(!strcasecmp(sep->arg[1], "print"))
	{
		if(zone->pathing)
		{
			zone->pathing->PrintPathing();
		}
		return;
	}

	if(!strcasecmp(sep->arg[1], "showneighbours") || !strcasecmp(sep->arg[1], "showneighbors"))
	{
		if(!c->GetTarget())
		{
			c->Message(0, "First #path shownodes to spawn the pathnodes, and then target one of them.");
			return;
		}
		if(zone->pathing)
		{
			zone->pathing->ShowPathNodeNeighbours(c);
			return;
		}
	}
	if(!strcasecmp(sep->arg[1], "meshtest"))
	{
		if(zone->pathing)
		{
			if(!strcasecmp(sep->arg[2], "simple"))
			{
				c->Message(0, "You may go linkdead. Results will be in the log file.");
				zone->pathing->SimpleMeshTest();
				return;
			}
			else
			{
				c->Message(0, "You may go linkdead. Results will be in the log file.");
				zone->pathing->MeshTest();
				return;
			}
		}
	}

	if(!strcasecmp(sep->arg[1], "allspawns"))
	{
		if(zone->pathing)
		{
			c->Message(0, "You may go linkdead. Results will be in the log file.");
			entity_list.FindPathsToAllNPCs();
			return;
		}
	}

	if(!strcasecmp(sep->arg[1], "nearest"))
	{
		if(!c->GetTarget() || !c->GetTarget()->IsMob())
		{
			c->Message(0, "You must target something.");
			return;
		}

		if(zone->pathing)
		{
			Mob *m = c->GetTarget();

			glm::vec3 Position(m->GetX(), m->GetY(), m->GetZ());

			int Node = zone->pathing->FindNearestPathNode(Position);

			if(Node == -1)
				c->Message(0, "Unable to locate a path node within range.");
			else
				c->Message(0, "Nearest path node is %i",  Node);

			return;
		}
	}

	c->Message(0, "Unknown path command.");
}

void Client::Undye() {
	for (int cur_slot = EQEmu::textures::textureBegin; cur_slot <= EQEmu::textures::LastTexture; cur_slot++) {
		uint8 slot2=SlotConvert(cur_slot);
		EQEmu::ItemInstance* inst = m_inv.GetItem(slot2);

		if(inst != nullptr) {
			inst->SetColor(inst->GetItem()->Color);
			database.SaveInventory(CharacterID(), inst, slot2);
		}

		m_pp.item_tint.Slot[cur_slot].Color = 0;
		SendWearChange(cur_slot);
	}

	database.DeleteCharacterDye(this->CharacterID());
}

void command_undye(Client *c, const Seperator *sep)
{
	if(c->GetTarget() && c->GetTarget()->IsClient())
	{
		c->GetTarget()->CastToClient()->Undye();
	}
	else
	{
		c->Message(0, "ERROR: Client target required");
	}
}

void command_undyeme(Client *c, const Seperator *sep)
{
	if(c) {
		c->Undye();
		c->Message(13, "Dye removed from all slots. Please zone for the process to complete.");
	}
}

void command_ginfo(Client *c, const Seperator *sep)
{
	Client *t;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t = c->GetTarget()->CastToClient();
	else
		t = c;

	Group *g = t->GetGroup();
	if(!g) {
		c->Message(0, "This client is not in a group");
		return;
	}

	c->Message(0, "Player: %s is in Group #%lu: with %i members",  t->GetName(), (unsigned long)g->GetID(), g->GroupCount());

	uint32 r;
	for(r = 0; r < MAX_GROUP_MEMBERS; r++) {
		if(g->members[r] == nullptr) {
			if(g->membername[r][0] == '\0')
				continue;
			c->Message(0, "...Zoned Member: %s, Roles: %s %s %s",  g->membername[r],
				(g->MemberRoles[r] & RoleAssist) ? "Assist" : "",
				(g->MemberRoles[r] & RoleTank) ? "Tank" : "",
				(g->MemberRoles[r] & RolePuller) ? "Puller" : "");
		} else {
			c->Message(0, "...In-Zone Member: %s (0x%x) Roles: %s %s %s",  g->membername[r], g->members[r],
				(g->MemberRoles[r] & RoleAssist) ? "Assist" : "",
				(g->MemberRoles[r] & RoleTank) ? "Tank" : "",
				(g->MemberRoles[r] & RolePuller) ? "Puller" : "");

		}
	}
}

void command_hp(Client *c, const Seperator *sep)
{
	c->SendHPUpdate();
	c->CheckManaEndUpdate();
}

void command_aggro(Client *c, const Seperator *sep)
{
	if(c->GetTarget() == nullptr || !c->GetTarget()->IsNPC()) {
		c->Message(0, "Error: you must have an NPC target.");
		return;
	}
	float d = atof(sep->arg[1]);
	if(d == 0.0f) {
		c->Message(13, "Error: distance argument required.");
		return;
	}
	bool verbose = false;
	if(sep->arg[2][0] == '-' && sep->arg[2][1] == 'v' && sep->arg[2][2] == '\0') {
		verbose = true;
	}

	entity_list.DescribeAggro(c, c->GetTarget()->CastToNPC(), d, verbose);
}

void command_pf(Client *c, const Seperator *sep)
{
	if(c->GetTarget())
	{
		Mob *who = c->GetTarget();
		c->Message(0, "POS: (%.2f, %.2f, %.2f)",  who->GetX(), who->GetY(), who->GetZ());
		c->Message(0, "WP: %s (%d/%d)",  to_string(who->GetCurrentWayPoint()).c_str(), who->IsNPC()?who->CastToNPC()->GetMaxWp():-1);
		c->Message(0, "TAR: (%.2f, %.2f, %.2f)",  who->GetTarX(), who->GetTarY(), who->GetTarZ());
		c->Message(0, "TARV: (%.2f, %.2f, %.2f)",  who->GetTarVX(), who->GetTarVY(), who->GetTarVZ());
		c->Message(0, "|TV|=%.2f index=%d",  who->GetTarVector(), who->GetTarNDX());
		c->Message(0, "pause=%d RAspeed=%d",  who->GetCWPP(), who->GetRunAnimSpeed());
	} else {
		c->Message(0, "ERROR: target required");
	}
}

void command_bestz(Client *c, const Seperator *sep) {
	if (zone->zonemap == nullptr) {
		c->Message(0,"Map not loaded for this zone");
	} else {
		glm::vec3 me;
		me.x = c->GetX();
		me.y = c->GetY();
		me.z = c->GetZ() + (c->GetSize() == 0.0 ? 6 : c->GetSize()) * HEAD_POSITION;
		glm::vec3 hit;
		glm::vec3 bme(me);
		bme.z -= 500;

		float best_z = zone->zonemap->FindBestZ(me, &hit);

		if (best_z != BEST_Z_INVALID)
		{
			c->Message(0, "Z is %.3f at (%.3f, %.3f).",  best_z, me.x, me.y);
		}
		else
		{
			c->Message(0, "Found no Z.");
		}
	}

	if(zone->watermap == nullptr) {
		c->Message(0,"Water Region Map not loaded for this zone");
	} else {
		WaterRegionType RegionType;
		float z;

		if(c->GetTarget()) {
			z=c->GetTarget()->GetZ();
			auto position = glm::vec3(c->GetTarget()->GetX(), c->GetTarget()->GetY(), z);
			RegionType = zone->watermap->ReturnRegionType(position);
			c->Message(0,"InWater returns %d",  zone->watermap->InWater(position));
			c->Message(0,"InLava returns %d",  zone->watermap->InLava(position));

		}
		else {
			z=c->GetZ();
			auto position = glm::vec3(c->GetX(), c->GetY(), z);
			RegionType = zone->watermap->ReturnRegionType(position);
			c->Message(0,"InWater returns %d",  zone->watermap->InWater(position));
			c->Message(0,"InLava returns %d",  zone->watermap->InLava(position));

		}

		switch(RegionType) {
			case RegionTypeNormal:	{ c->Message(0,"There is nothing special about the region you are in!"); break; }
			case RegionTypeWater:	{ c->Message(0,"You/your target are in Water."); break; }
			case RegionTypeLava:	{ c->Message(0,"You/your target are in Lava."); break; }
			case RegionTypeVWater:	{ c->Message(0,"You/your target are in VWater (Icy Water?)."); break; }
			case RegionTypePVP:	{ c->Message(0, "You/your target are in a pvp enabled area."); break; }
			case RegionTypeSlime:	{ c->Message(0, "You/your target are in slime."); break; }
			case RegionTypeIce:	{ c->Message(0, "You/your target are in ice."); break; }
			default: c->Message(0,"You/your target are in an unknown region type.");
		}
	}


}


void command_reloadstatic(Client *c, const Seperator *sep) {
	c->Message(0, "Reloading zone static data...");
	zone->ReloadStaticData();
}

void command_flags(Client *c, const Seperator *sep) {
	Client *t = c;

	if(c->Admin() >= minStatusToSeeOthersZoneFlags) {
		Mob *tgt = c->GetTarget();
		if(tgt != nullptr && tgt->IsClient())
			t = tgt->CastToClient();
	}

	t->SendZoneFlagInfo(c);
}

void command_flagedit(Client *c, const Seperator *sep) {
	//super-command for editing zone flags
	if(sep->arg[1][0] == '\0' || !strcasecmp(sep->arg[1], "help")) {
		c->Message(0, "Syntax: #flagedit [lockzone|unlockzone|listzones|give|take].");
		c->Message(0, "...lockzone [zone id/short] [flag name] - Set the specified flag name on the zone, locking the zone");
		c->Message(0, "...unlockzone [zone id/short] - Removes the flag requirement from the specified zone");
		c->Message(0, "...listzones - List all zones which require a flag, and their flag's name");
		c->Message(0, "...give [zone id/short] - Give your target the zone flag for the specified zone.");
		c->Message(0, "...take [zone id/short] - Take the zone flag for the specified zone away from your target");
		c->Message(0, "...Note: use #flags to view flags on a person");
		return;
	}

	if(!strcasecmp(sep->arg[1], "lockzone")) {
		uint32 zoneid = 0;
		if(sep->arg[2][0] != '\0') {
			zoneid = atoi(sep->arg[2]);
			if(zoneid < 1) {
				zoneid = database.GetZoneID(sep->arg[2]);
			}
		}
		if(zoneid < 1) {
			c->Message(13, "zone required. see help.");
			return;
		}

		char flag_name[128];
		if(sep->argplus[3][0] == '\0') {
			c->Message(13, "flag name required. see help.");
			return;
		}
		database.DoEscapeString(flag_name, sep->argplus[3], 64);
		flag_name[127] = '\0';

        std::string query = StringFormat("UPDATE zone SET flag_needed = '%s' "
                                        "WHERE zoneidnumber = %d AND version = %d",
                                        flag_name, zoneid, zone->GetInstanceVersion());
        auto results = database.QueryDatabase(query);
		if(!results.Success()) {
			c->Message(13, "Error updating zone: %s",  results.ErrorMessage().c_str());
			return;
		}

        c->Message(15, "Success! Zone %s now requires a flag, named %s",  database.GetZoneName(zoneid), flag_name);
        return;
	}

	if(!strcasecmp(sep->arg[1], "unlockzone")) {
		uint32 zoneid = 0;
		if(sep->arg[2][0] != '\0') {
			zoneid = atoi(sep->arg[2]);
			if(zoneid < 1) {
				zoneid = database.GetZoneID(sep->arg[2]);
			}
		}

		if(zoneid < 1) {
			c->Message(13, "zone required. see help.");
			return;
		}

        std::string query = StringFormat("UPDATE zone SET flag_needed = '' "
                                        "WHERE zoneidnumber = %d AND version = %d",
                                        zoneid, zone->GetInstanceVersion());
        auto results = database.QueryDatabase(query);
		if(!results.Success()) {
			c->Message(15, "Error updating zone: %s",  results.ErrorMessage().c_str());
			return;
		}

        c->Message(15, "Success! Zone %s no longer requires a flag.",  database.GetZoneName(zoneid));
        return;
	}

	if(!strcasecmp(sep->arg[1], "listzones")) {
        std::string query = "SELECT zoneidnumber, short_name, long_name, version, flag_needed "
                            "FROM zone WHERE flag_needed != ''";
        auto results = database.QueryDatabase(query);
		if (!results.Success()) {
            return;
        }

        c->Message(0, "Zones which require flags:");
        for (auto row = results.begin(); row != results.end(); ++row)
            c->Message(0, "Zone %s (%s,%s) version %s requires key %s",  row[2], row[0], row[1], row[3], row[4]);

        return;
	}

	if(!strcasecmp(sep->arg[1], "give")) {
		uint32 zoneid = 0;
		if(sep->arg[2][0] != '\0') {
			zoneid = atoi(sep->arg[2]);
			if(zoneid < 1) {
				zoneid = database.GetZoneID(sep->arg[2]);
			}
		}
		if(zoneid < 1) {
			c->Message(13, "zone required. see help.");
			return;
		}

		Mob *t = c->GetTarget();
		if(t == nullptr || !t->IsClient()) {
			c->Message(13, "client target required");
			return;
		}

		t->CastToClient()->SetZoneFlag(zoneid);
		return;
	}

	if(!strcasecmp(sep->arg[1], "give")) {
		uint32 zoneid = 0;
		if(sep->arg[2][0] != '\0') {
			zoneid = atoi(sep->arg[2]);
			if(zoneid < 1) {
				zoneid = database.GetZoneID(sep->arg[2]);
			}
		}
		if(zoneid < 1) {
			c->Message(13, "zone required. see help.");
			return;
		}

		Mob *t = c->GetTarget();
		if(t == nullptr || !t->IsClient()) {
			c->Message(13, "client target required");
			return;
		}

		t->CastToClient()->ClearZoneFlag(zoneid);
		return;
	}

    c->Message(15, "Invalid action specified. use '#flagedit help' for help");
}

void command_serverrules(Client *c, const Seperator *sep)
{
	c->SendRules(c);
}

void command_acceptrules(Client *c, const Seperator *sep)
{
	if(!database.GetAgreementFlag(c->AccountID()))
	{
		database.SetAgreementFlag(c->AccountID());
		c->SendAppearancePacket(AT_Anim, ANIM_STAND);
		c->Message(0,"It is recorded you have agreed to the rules.");
	}
}

void command_guildcreate(Client *c, const Seperator *sep)
{
	if(strlen(sep->argplus[1])>4 && strlen(sep->argplus[1])<16)
	{
		guild_mgr.AddGuildApproval(sep->argplus[1],c);
	}
	else
	{
		c->Message(0,"Guild name must be more than 4 characters and less than 16.");
	}
}

void command_guildapprove(Client *c, const Seperator *sep)
{
	guild_mgr.AddMemberApproval(atoi(sep->arg[1]),c);
}

void command_guildlist(Client *c, const Seperator *sep)
{
	GuildApproval* tmp = guild_mgr.FindGuildByIDApproval(atoi(sep->arg[1]));
	if(tmp)
	{
		tmp->ApprovedMembers(c);
	}
	else
		c->Message(0,"Could not find reference id.");
}

void command_hatelist(Client *c, const Seperator *sep) {
	Mob *target = c->GetTarget();
	if(target == nullptr) {
		c->Message(0, "Error: you must have a target.");
		return;
	}

	c->Message(0, "Display hate list for %s..",  target->GetName());
	target->PrintHateListToClient(c);
}


void command_rules(Client *c, const Seperator *sep) {
	//super-command for managing rules settings
	if(sep->arg[1][0] == '\0' || !strcasecmp(sep->arg[1], "help")) {
		c->Message(0, "Syntax: #rules [subcommand].");
		c->Message(0, "-- Rule Set Manipulation --");
		c->Message(0, "...listsets - List avaliable rule sets");
		c->Message(0, "...current - gives the name of the ruleset currently running in this zone");
		c->Message(0, "...reload - Reload the selected ruleset in this zone");
		c->Message(0, "...switch (ruleset name) - Change the selected ruleset and load it");
		c->Message(0, "...load (ruleset name) - Load a ruleset in just this zone without changing the selected set");
//too lazy to write this right now:
//		c->Message(0, "...wload (ruleset name) - Load a ruleset in all zones without changing the selected set");
		c->Message(0, "...store [ruleset name] - Store the running ruleset as the specified name");
		c->Message(0, "---------------------");
		c->Message(0, "-- Running Rule Manipulation --");
		c->Message(0, "...reset - Reset all rules to their default values");
		c->Message(0, "...get [rule] - Get the specified rule's local value");
		c->Message(0, "...set (rule) (value) - Set the specified rule to the specified value locally only");
		c->Message(0, "...setdb (rule) (value) - Set the specified rule to the specified value locally and in the DB");
		c->Message(0, "...list [catname] - List all rules in the specified category (or all categiries if omitted)");
		c->Message(0, "...values [catname] - List the value of all rules in the specified category");
		return;
	}

	if(!strcasecmp(sep->arg[1], "current")) {
		c->Message(0, "Currently running ruleset '%s' (%d)",  RuleManager::Instance()->GetActiveRuleset(),
			RuleManager::Instance()->GetActiveRulesetID());
	} else if(!strcasecmp(sep->arg[1], "listsets")) {
		std::map<int, std::string> sets;
		if(!RuleManager::Instance()->ListRulesets(&database, sets)) {
			c->Message(13, "Failed to list rule sets!");
			return;
		}

		c->Message(0, "Avaliable rule sets:");
		std::map<int, std::string>::iterator cur, end;
		cur = sets.begin();
		end = sets.end();
		for(; cur != end; ++cur) {
			c->Message(0, "(%d) %s",  cur->first, cur->second.c_str());
		}
	} else if(!strcasecmp(sep->arg[1], "reload")) {
		RuleManager::Instance()->LoadRules(&database, RuleManager::Instance()->GetActiveRuleset());
		c->Message(0, "The active ruleset (%s (%d)) has been reloaded",  RuleManager::Instance()->GetActiveRuleset(),
			RuleManager::Instance()->GetActiveRulesetID());
	} else if(!strcasecmp(sep->arg[1], "switch")) {
		//make sure this is a valid rule set..
		int rsid = RuleManager::Instance()->GetRulesetID(&database, sep->arg[2]);
		if(rsid < 0) {
			c->Message(13, "Unknown rule set '%s'",  sep->arg[2]);
			return;
		}
		if(!database.SetVariable("RuleSet",  sep->arg[2])) {
			c->Message(13, "Failed to update variables table to change selected rule set");
			return;
		}

		//TODO: we likely want to reload this ruleset everywhere...
		RuleManager::Instance()->LoadRules(&database, sep->arg[2]);

		c->Message(0, "The selected ruleset has been changed to (%s (%d)) and reloaded locally",  sep->arg[2], rsid);
	} else if(!strcasecmp(sep->arg[1], "load")) {
		//make sure this is a valid rule set..
		int rsid = RuleManager::Instance()->GetRulesetID(&database, sep->arg[2]);
		if(rsid < 0) {
			c->Message(13, "Unknown rule set '%s'",  sep->arg[2]);
			return;
		}
		RuleManager::Instance()->LoadRules(&database, sep->arg[2]);
		c->Message(0, "Loaded ruleset '%s' (%d) locally",  sep->arg[2], rsid);
	} else if(!strcasecmp(sep->arg[1], "store")) {
		if(sep->argnum == 1) {
			//store current rule set.
			RuleManager::Instance()->SaveRules(&database);
			c->Message(0, "Rules saved");
		} else if(sep->argnum == 2) {
			RuleManager::Instance()->SaveRules(&database, sep->arg[2]);
			int prersid = RuleManager::Instance()->GetActiveRulesetID();
			int rsid = RuleManager::Instance()->GetRulesetID(&database, sep->arg[2]);
			if(rsid < 0) {
				c->Message(13, "Unable to query ruleset ID after store, it most likely failed.");
			} else {
				c->Message(0, "Stored rules as ruleset '%s' (%d)",  sep->arg[2], rsid);
				if(prersid != rsid) {
					c->Message(0, "Rule set %s (%d) is now active in this zone",  sep->arg[2], rsid);
				}
			}
		} else {
			c->Message(13, "Invalid argument count, see help.");
			return;
		}
	} else if(!strcasecmp(sep->arg[1], "reset")) {
		RuleManager::Instance()->ResetRules();
		c->Message(0, "The running ruleset has been set to defaults");

	} else if(!strcasecmp(sep->arg[1], "get")) {
		if(sep->argnum != 2) {
			c->Message(13, "Invalid argument count, see help.");
			return;
		}
		std::string value;
		if(!RuleManager::Instance()->GetRule(sep->arg[2], value))
			c->Message(13, "Unable to find rule %s",  sep->arg[2]);
		else
			c->Message(0, "%s - %s",  sep->arg[2], value.c_str());

	} else if(!strcasecmp(sep->arg[1], "set")) {
		if(sep->argnum != 3) {
			c->Message(13, "Invalid argument count, see help.");
			return;
		}
		if(!RuleManager::Instance()->SetRule(sep->arg[2], sep->arg[3])) {
			c->Message(13, "Failed to modify rule");
		} else {
			c->Message(0, "Rule modified locally.");
		}
	} else if(!strcasecmp(sep->arg[1], "setdb")) {
		if(sep->argnum != 3) {
			c->Message(13, "Invalid argument count, see help.");
			return;
		}
		if(!RuleManager::Instance()->SetRule(sep->arg[2], sep->arg[3], &database, true)) {
			c->Message(13, "Failed to modify rule");
		} else {
			c->Message(0, "Rule modified locally and in the database.");
		}
	} else if(!strcasecmp(sep->arg[1], "list")) {
		if(sep->argnum == 1) {
			std::vector<const char *> rule_list;
			if(!RuleManager::Instance()->ListCategories(rule_list)) {
				c->Message(13, "Failed to list categories!");
				return;
			}
			c->Message(0, "Rule Categories:");
			std::vector<const char *>::iterator cur, end;
			cur = rule_list.begin();
			end = rule_list.end();
			for(; cur != end; ++cur) {
				c->Message(0, " %s",  *cur);
			}
		} else if(sep->argnum == 2) {
			const char *catfilt = nullptr;
			if(std::string("all") != sep->arg[2])
				catfilt = sep->arg[2];
			std::vector<const char *> rule_list;
			if(!RuleManager::Instance()->ListRules(catfilt, rule_list)) {
				c->Message(13, "Failed to list rules!");
				return;
			}
			c->Message(0, "Rules in category %s:",  sep->arg[2]);
			std::vector<const char *>::iterator cur, end;
			cur = rule_list.begin();
			end = rule_list.end();
			for(; cur != end; ++cur) {
				c->Message(0, " %s",  *cur);
			}
		} else {
			c->Message(13, "Invalid argument count, see help.");
		}
	} else if(!strcasecmp(sep->arg[1], "values")) {
		if(sep->argnum != 2) {
			c->Message(13, "Invalid argument count, see help.");
			return;
		} else {
			const char *catfilt = nullptr;
			if(std::string("all") != sep->arg[2])
				catfilt = sep->arg[2];
			std::vector<const char *> rule_list;
			if(!RuleManager::Instance()->ListRules(catfilt, rule_list)) {
				c->Message(13, "Failed to list rules!");
				return;
			}
			c->Message(0, "Rules & values in category %s:",  sep->arg[2]);
			std::vector<const char *>::iterator cur, end;
			cur = rule_list.begin();
			end = rule_list.end();
			for(std::string tmp_value; cur != end; ++cur) {
				if (RuleManager::Instance()->GetRule(*cur, tmp_value))
					c->Message(0, " %s - %s",  *cur, tmp_value.c_str());
			}
		}

	} else {
		c->Message(15, "Invalid action specified. use '#rules help' for help");
	}
}


void command_task(Client *c, const Seperator *sep) {
	//super-command for managing tasks
	if(sep->arg[1][0] == '\0' || !strcasecmp(sep->arg[1], "help")) {
		c->Message(0, "Syntax: #task [subcommand].");
		c->Message(0, "-- Task System Commands --");
		c->Message(0, "...show - List active tasks for a client");
		c->Message(0, "...update <TaskID> <ActivityID> [Count]");
		c->Message(0, "...reloadall - Reload all Task information from the database");
		c->Message(0, "...reload task <TaskID> - Reload Task and Activity informnation for a single task");
		c->Message(0, "...reload lists - Reload goal/reward list information");
		c->Message(0, "...reload prox - Reload proximity information");
		c->Message(0, "...reload sets - Reload task set information");
		return;
	}

	if(!strcasecmp(sep->arg[1], "show")) {
		if(c->GetTarget() && c->GetTarget()->IsClient())
			c->GetTarget()->CastToClient()->ShowClientTasks();
		else
			c->ShowClientTasks();

		return;
	}

	if(!strcasecmp(sep->arg[1], "update")) {
		if(sep->argnum>=3) {
			int TaskID = atoi(sep->arg[2]);
			int ActivityID = atoi(sep->arg[3]);
			int Count=1;

			if(sep->argnum>=4) {
				Count = atoi(sep->arg[4]);
				if(Count <= 0)
					Count = 1;
			}
			c->Message(15, "Updating Task %i, Activity %i, Count %i",  TaskID, ActivityID, Count);
			c->UpdateTaskActivity(TaskID, ActivityID, Count);
		}
		return;
	}
	if(!strcasecmp(sep->arg[1], "reloadall")) {
		c->Message(15, "Sending reloadtasks to world");
		worldserver.SendReloadTasks(RELOADTASKS);
		c->Message(15, "Back again");
		return;
	}

	if(!strcasecmp(sep->arg[1], "reload")) {
		if(sep->arg[2][0] != '\0') {
			if(!strcasecmp(sep->arg[2], "lists")) {
				c->Message(15, "Sending reload lists to world");
				worldserver.SendReloadTasks(RELOADTASKGOALLISTS);
				c->Message(15, "Back again");
				return;
			}
			if(!strcasecmp(sep->arg[2], "prox")) {
				c->Message(15, "Sending reload proximities to world");
				worldserver.SendReloadTasks(RELOADTASKPROXIMITIES);
				c->Message(15, "Back again");
				return;
			}
			if(!strcasecmp(sep->arg[2], "sets")) {
				c->Message(15, "Sending reload task sets to world");
				worldserver.SendReloadTasks(RELOADTASKSETS);
				c->Message(15, "Back again");
				return;
			}
			if(!strcasecmp(sep->arg[2], "task") && (sep->arg[3][0] != '\0')) {
				int TaskID = atoi(sep->arg[3]);
				if((TaskID > 0) && (TaskID < MAXTASKS)) {
					c->Message(15, "Sending reload task %i to world");
					worldserver.SendReloadTasks(RELOADTASKS, TaskID);
					c->Message(15, "Back again");
					return;
				}
			}
		}

	}
	c->Message(0, "Unable to interpret command. Type #task help");

}
void command_reloadtitles(Client *c, const Seperator *sep)
{
	auto pack = new ServerPacket(ServerOP_ReloadTitles, 0);
	worldserver.SendPacket(pack);
	safe_delete(pack);
	c->Message(15, "Player Titles Reloaded.");

}

void command_traindisc(Client *c, const Seperator *sep)
{
	uint8 max_level, min_level;
	uint16 curspell, count;
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM())
		t=c->GetTarget()->CastToClient();

	if(!sep->arg[1][0])
	{
		c->Message(0, "FORMAT: #traindisc <max level> <min level>");
		return;
	}

	max_level = (uint8)atoi(sep->arg[1]);
	if (!c->GetGM() && max_level > RuleI(Character, MaxLevel))
		max_level = RuleI(Character, MaxLevel);	//default to Character:MaxLevel if we're not a GM & it's higher than the max level
	min_level = sep->arg[2][0] ? (uint8)atoi(sep->arg[2]) : 1;	//default to 1 if there isn't a 2nd argument
	if (!c->GetGM() && min_level > RuleI(Character, MaxLevel))
		min_level = RuleI(Character, MaxLevel);	//default to Character:MaxLevel if we're not a GM & it's higher than the max level

	if(max_level < 1 || min_level < 1)
	{
		c->Message(0, "ERROR: Level must be greater than 1.");
		return;
	}
	if (min_level > max_level) {
		c->Message(0, "Error: Min Level must be less than or equal to Max Level.");
		return;
	}

	t->Message(0, "Training disciplines");
	if(t != c)
		c->Message(0, "Training disciplines for %s.",  t->GetName());
	Log(Logs::General, Logs::Normal, "Train disciplines request for %s from %s, levels: %u -> %u",  t->GetName(), c->GetName(), min_level, max_level);

	for(curspell = 0, count = 0; curspell < SPDAT_RECORDS; curspell++)
	{
		if
		(
			spells[curspell].classes[WARRIOR] != 0 && // check if spell exists
			spells[curspell].classes[t->GetPP().class_-1] <= max_level &&	//maximum level
			spells[curspell].classes[t->GetPP().class_-1] >= min_level &&	//minimum level
			spells[curspell].skill != 52
		)
		{
			if(IsDiscipline(curspell)){
				//we may want to come up with a function like Client::GetNextAvailableSpellBookSlot() to help speed this up a little
				for(int r = 0; r < MAX_PP_DISCIPLINES; r++) {
					if(t->GetPP().disciplines.values[r] == curspell) {
						t->Message(13, "You already know this discipline.");
						break;	//continue the 1st loop
					} else if(t->GetPP().disciplines.values[r] == 0) {
						t->GetPP().disciplines.values[r] = curspell;
						database.SaveCharacterDisc(t->CharacterID(), r, curspell);
						t->SendDisciplineUpdate();
						t->Message(0, "You have learned a new discipline!");
						count++;	//success counter
						break;	//continue the 1st loop
					}	//if we get to this point, there's already a discipline in this slot, so we continue onto the next slot
				}
			}
		}
	}

	if (count > 0) {
		t->Message(0, "Successfully trained %u disciplines.",  count);
		if (t != c)
			c->Message(0, "Successfully trained %u disciplines for %s.",  count, t->GetName());
	} else {
		t->Message(0, "No disciplines trained.");
		if (t != c)
			c->Message(0, "No disciplines trained for %s.",  t->GetName());
	}
}

void command_setgraveyard(Client *c, const Seperator *sep)
{
	uint32 zoneid = 0;
	uint32 graveyard_id = 0;
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM())
		t=c->GetTarget()->CastToClient();

	if(!sep->arg[1][0]) {
		c->Message(0, "Usage: #setgraveyard [zonename]");
		return;
	}

	zoneid = database.GetZoneID(sep->arg[1]);

	if(zoneid > 0) {
		graveyard_id = database.CreateGraveyardRecord(zoneid, t->GetPosition());

		if(graveyard_id > 0) {
			c->Message(0, "Successfuly added a new record for this graveyard!");
			if(database.AddGraveyardIDToZone(zoneid, graveyard_id) > 0) {
				c->Message(0, "Successfuly added this new graveyard for the zone %s.",  sep->arg[1]);
				// TODO: Set graveyard data to the running zone process.
				c->Message(0, "Done!");
			}
			else
				c->Message(0, "Unable to add this new graveyard to the zone %s.",  sep->arg[1]);
		}
		else {
			c->Message(0, "Unable to create a new graveyard record in the database.");
		}
	}
	else {
		c->Message(0, "Unable to retrieve a ZoneID for the zone: %s",  sep->arg[1]);
	}

	return;
}

void command_deletegraveyard(Client *c, const Seperator *sep)
{
	uint32 zoneid = 0;
	uint32 graveyard_id = 0;

	if(!sep->arg[1][0]) {
		c->Message(0, "Usage: #deletegraveyard [zonename]");
		return;
	}

	zoneid = database.GetZoneID(sep->arg[1]);
	graveyard_id = database.GetZoneGraveyardID(zoneid, 0);

	if(zoneid > 0 && graveyard_id > 0) {
		if(database.DeleteGraveyard(zoneid, graveyard_id))
			c->Message(0, "Successfuly deleted graveyard %u for zone %s.",  graveyard_id, sep->arg[1]);
		else
			c->Message(0, "Unable to delete graveyard %u for zone %s.",  graveyard_id, sep->arg[1]);
	}
	else {
		if(zoneid <= 0)
			c->Message(0, "Unable to retrieve a ZoneID for the zone: %s",  sep->arg[1]);
		else if(graveyard_id <= 0)
			c->Message(0, "Unable to retrieve a valid GraveyardID for the zone: %s",  sep->arg[1]);
	}

	return;
}

void command_summonburiedplayercorpse(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM())
		t = c->GetTarget()->CastToClient();
	else {
		c->Message(0, "You must first select a target!");
		return;
	}

	Corpse* PlayerCorpse = database.SummonBuriedCharacterCorpses(t->CharacterID(), t->GetZoneID(), zone->GetInstanceID(), t->GetPosition());

	if(!PlayerCorpse)
		c->Message(0, "Your target doesn't have any buried corpses.");

	return;
}

void command_getplayerburiedcorpsecount(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM())
		t = c->GetTarget()->CastToClient();
	else {
		c->Message(0, "You must first select a target!");
		return;
	}

	uint32 CorpseCount = database.GetCharacterBuriedCorpseCount(t->CharacterID());

	if(CorpseCount > 0)
		c->Message(0, "Your target has a total of %u buried corpses.",  CorpseCount);
	else
		c->Message(0, "Your target doesn't have any buried corpses.");

	return;
}

void command_refreshgroup(Client *c, const Seperator *sep)
{
	if(!c)
		return;

	Group *g = c->GetGroup();

	if(!g)
		return;

	database.RefreshGroupFromDB(c);
	//g->SendUpdate(7, c);
}

void command_advnpcspawn(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();

    if (strcasecmp(sep->arg[1], "maketype") == 0) {
        if(!target || !target->IsNPC()) {
            c->Message(0, "Target Required!");
            return;
        }

        database.NPCSpawnDB(6, zone->GetShortName(), zone->GetInstanceVersion(), c, target->CastToNPC());
        return;
    }

    if (strcasecmp(sep->arg[1], "makegroup") == 0) {
        if(!sep->arg[2]) {
            c->Message(0, "Format: #advnpdspawn makegroup <name> [spawn limit] [dist] [max x] [min x] [max y] [min y] [delay]");
            return;
        }

        std::string query = StringFormat("INSERT INTO spawngroup "
                                        "(name, spawn_limit, dist, max_x, min_x, max_y, min_y, delay) "
                                        "VALUES (\"%s\",  %i, %f, %f, %f, %f, %f, %i)",
                                        sep->arg[2],
                                        (sep->arg[3]? atoi(sep->arg[3]): 0),
                                        (sep->arg[4]? atof(sep->arg[4]): 0),
                                        (sep->arg[5]? atof(sep->arg[5]): 0),
                                        (sep->arg[6]? atof(sep->arg[6]): 0),
                                        (sep->arg[7]? atof(sep->arg[7]): 0),
                                        (sep->arg[8]? atof(sep->arg[8]): 0),
                                        (sep->arg[9]? atoi(sep->arg[9]): 0));
        auto results = database.QueryDatabase(query);
        if (!results.Success()) {
            c->Message(0, "Invalid Arguments -- MySQL gave the following error:");
            c->Message(13, results.ErrorMessage().c_str());
            return;
        }

        c->Message(0, "Group ID %i created successfully!",  results.LastInsertedID());
        return;
    }

	if (strcasecmp(sep->arg[1], "addgroupentry") == 0) {
        if(!atoi(sep->arg[2]) || !atoi(sep->arg[3]) || !atoi(sep->arg[4])) {
            c->Message(0, "Format: #advnpdspawn addgroupentry <spawnggroupID> <npcID> <chance>");
            return;
        }

        std::string query = StringFormat("INSERT INTO spawnentry (spawngroupID, npcID, chance) "
                                        "VALUES (%i, %i, %i)",
                                        atoi(sep->arg[2]), atoi(sep->arg[3]), atoi(sep->arg[4]));
        auto results = database.QueryDatabase(query);
        if (!results.Success()) {
            c->Message(0, "Invalid Arguments -- MySQL gave the following error:");
            c->Message(13, results.ErrorMessage().c_str());
            return;
        }

        c->Message(0, "NPC %i added to group %i with %i chance!",  atoi(sep->arg[3]), atoi(sep->arg[2]), atoi(sep->arg[4]) );

        return;
    }

    if (strcasecmp(sep->arg[1], "editgroupbox") == 0) {
        if(!atof(sep->arg[2]) || !atof(sep->arg[3]) || !atof(sep->arg[4]) || !atof(sep->arg[5]) || !atof(sep->arg[6]) || !atof(sep->arg[7]) || !atof(sep->arg[8])) {
            c->Message(0, "Format: #advnpdspawn editgroupbox <spawngroupID> <dist> <max x> <min x> <max y> <min y> <delay>");
            return;
        }

        std::string query = StringFormat("UPDATE spawngroup SET dist = '%f', max_x = '%f', min_x = '%f', "
                                        "max_y = '%f', min_y = '%f', delay = '%i' WHERE id = '%i'",
                                        atof(sep->arg[3]), atof(sep->arg[4]), atof(sep->arg[5]),
                                        atof(sep->arg[6]), atof(sep->arg[7]), atoi(sep->arg[8]),
                                        atoi(sep->arg[2]));
        auto results = database.QueryDatabase(query);
        if (!results.Success()) {
            c->Message(0, "Invalid Arguments -- MySQL gave the following error:");
            c->Message(13, results.ErrorMessage().c_str());
            return;
        }

        c->Message(0, "Group ID %i created successfully!",  results.LastInsertedID());

        return;
    }

	if (strcasecmp(sep->arg[1], "cleargroupbox") == 0) {
        if(!atoi(sep->arg[2])) {
            c->Message(0, "Format: #advnpdspawn cleargroupbox <spawngroupID>");
            return;
        }

        std::string query = StringFormat("UPDATE spawngroup "
                                        "SET dist = '0', max_x = '0', min_x = '0', "
                                        "max_y = '0', min_y = '0', delay = '0' "
                                        "WHERE id = '%i' ",  atoi(sep->arg[2]));
        auto results = database.QueryDatabase(query);
        if (!results.Success()) {
            c->Message(0, "Invalid Arguments -- MySQL gave the following error:");
            c->Message(13, results.ErrorMessage().c_str());
            return;
        }

        c->Message(0, "Group ID %i created successfully!",  results.LastInsertedID());

        return;
    }

	if (strcasecmp(sep->arg[1], "addgroupspawn") == 0 && atoi(sep->arg[2])!=0) {
        database.NPCSpawnDB(5, zone->GetShortName(), zone->GetInstanceVersion(), c, 0, atoi(sep->arg[2]));
        c->Message(0, "Mob of group %i added successfully!",  atoi(sep->arg[2]));
        return;
    }

	if (strcasecmp(sep->arg[1], "removegroupspawn") == 0) {
        if (!target || !target->IsNPC()) {
            c->Message(0, "Error: Need an NPC target.");
            return;
        }

        Spawn2* s2 = target->CastToNPC()->respawn2;

        if(!s2) {
            c->Message(0, "removegroupspawn FAILED -- cannot determine which spawn entry in the database this mob came from.");
            return;
        }

        std::string query = StringFormat("DELETE FROM spawn2 WHERE id = '%i'",  s2->GetID());
        auto results = database.QueryDatabase(query);
        if(!results.Success()) {
            c->Message(13, "Update failed! MySQL gave the following error:");
            c->Message(13, results.ErrorMessage().c_str());
            return;
        }

        c->Message(0, "Spawnpoint Removed successfully.");
        target->Depop(false);

        return;
    }

	if (strcasecmp(sep->arg[1], "movespawn") == 0) {
        if (!target || !target->IsNPC()) {
            c->Message(0, "Error: Need an NPC target.");
            return;
        }

        Spawn2* s2 = target->CastToNPC()->respawn2;

        if(!s2) {
            c->Message(0, "movespawn FAILED -- cannot determine which spawn entry in the database this mob came from.");
            return;
        }

        std::string query = StringFormat("UPDATE spawn2 SET x = '%f', y = '%f', z = '%f', heading = '%f' "
                                        "WHERE id = '%i'",
                                        c->GetX(), c->GetY(), c->GetZ(), c->GetHeading(),s2->GetID());
        auto results = database.QueryDatabase(query);
        if (!results.Success()) {
            c->Message(13, "Update failed! MySQL gave the following error:");
            c->Message(13, results.ErrorMessage().c_str());
            return;
        }

        c->Message(0, "Updating coordinates successful.");
        target->CastToNPC()->GMMove(c->GetX(), c->GetY(), c->GetZ(), c->GetHeading());
        target->CastToNPC()->SaveGuardSpot(true);
        target->SendPosition();

        return;
    }

	if (strcasecmp(sep->arg[1], "editrespawn") == 0) {
        if (!target || !target->IsNPC()) {
            c->Message(0, "Error: Need an NPC target.");
            return;
        }

        Spawn2* s2 = target->CastToNPC()->respawn2;

        uint32 new_rs = 0;
        uint32 new_var = s2->GetVariance();
        if(!sep->IsNumber(2)) {
            c->Message(0, "editrespawn FAILED -- cannot set respawn to be 0");
            return;
        }

        new_rs = atoi(sep->arg[2]);

        if(sep->IsNumber(3))
            new_var = atoi(sep->arg[3]);

        if(!s2) {
            c->Message(0, "editrespawn FAILED -- cannot determine which spawn entry in the database this mob came from.");
            return;
        }

        std::string query = StringFormat("UPDATE spawn2 SET respawntime = %u, variance = %u "
                                        "WHERE id = '%i'",  new_rs, new_var, s2->GetID());
        auto results = database.QueryDatabase(query);
        if (!results.Success()) {
            c->Message(13, "Update failed! MySQL gave the following error:");
            c->Message(13, results.ErrorMessage().c_str());
            return;
        }

        c->Message(0, "Updating respawn timer successful.");
        s2->SetRespawnTimer(new_rs);
        s2->SetVariance(new_var);

        return;
    }

	if (strcasecmp(sep->arg[1], "setversion") == 0) {
        if (!target || !target->IsNPC()) {
            c->Message(0, "Error: Need an NPC target.");
            return;
        }

        if(!sep->IsNumber(2)) {
            c->Message(0, "setversion FAILED -- You must set a version number");
            return;
        }

        int16 version = atoi(sep->arg[2]);
        std::string query = StringFormat("UPDATE spawn2 SET version = %i "
                                        "WHERE spawngroupID = '%i'",
                                        version, c->GetTarget()->CastToNPC()->GetSp2());
        auto results = database.QueryDatabase(query);
        if (!results.Success()) {
            c->Message(13, "Update failed! MySQL gave the following error:");
            c->Message(13, results.ErrorMessage().c_str());
            return;
        }

        c->Message(0, "Version change to %i was successful from SpawnGroupID %i",  version, c->GetTarget()->CastToNPC()->GetSp2());
        c->GetTarget()->Depop(false);

        return;
    }

	if (strcasecmp(sep->arg[1], "testload") == 0 && atoi(sep->arg[2])!=0) {
        database.LoadSpawnGroupsByID(atoi(sep->arg[2]),&zone->spawn_group_list);
        c->Message(0, "Group %i loaded successfully!",  atoi(sep->arg[2]));
        return;
    }

    c->Message(0, "Error: #advnpcspawn: Invalid command.");
    c->Message(0, "Usage: #advnpcspawn [maketype|makegroup|addgroupentry|addgroupspawn|setversion]");
    c->Message(0, "Usage: #advnpcspawn [removegroupspawn|movespawn|editrespawn|editgroupbox|cleargroupbox]");
}

void command_aggrozone(Client *c, const Seperator *sep) {
	if(!c)
		return;

	Mob *m = c->CastToMob();

	if (!m)
		return;

	uint32 hate = atoi(sep->arg[1]); //should default to 0 if we don't enter anything
	entity_list.AggroZone(m, hate);
	c->Message(0, "Train to you! Last chance to go invulnerable...");
}

void command_modifynpcstat(Client *c, const Seperator *sep)
{
	if(!c)
		return;

	if(sep->arg[1][0] == '\0')
	{
		c->Message(0, "usage #modifynpcstat arg value");
		c->Message(0, "Args: ac, str, sta, agi, dex, wis, _int, cha, max_hp, mr, fr, cr, pr, dr, runspeed, special_attacks, "
			"attack_speed, atk, accuracy, trackable, min_hit, max_hit, see_invis_undead, see_hide, see_improved_hide, "
			"hp_regen, mana_regen, aggro, assist, slow_mitigation, loottable_id, healscale, spellscale");
		return;
	}

	if(!c->GetTarget())
		return;

	if(!c->GetTarget()->IsNPC())
		return;

	c->GetTarget()->CastToNPC()->ModifyNPCStat(sep->arg[1], sep->arg[2]);
}

void command_instance(Client *c, const Seperator *sep)
{
	if(!c)
		return;

	//options:
	//help
	//create [zone_id] [version]
	//destroy [instance_id]
	//add [instance_id] [player_name]
	//remove [instance_id] [player_name]
	//list [player_name]

	if(strcasecmp(sep->arg[1], "help") == 0)
	{
		c->Message(0, "#instance usage:");
		c->Message(0, "#instance create zone_id version duration - Creates an instance of version 'version' in the "
			"zone with id matching zone_id, will last for duration seconds.");
		c->Message(0, "#instance destroy instance_id - Destroys the instance with id matching instance_id.");
		c->Message(0, "#instance add instance_id player_name - adds the player 'player_name' to the instance "
			"with id matching instance_id.");
		c->Message(0, "#instance remove instance_id player_name - removes the player 'player_name' from the "
			"instance with id matching instance_id.");
		c->Message(0, "#instance list player_name - lists all the instances 'player_name' is apart of.");
		return;
	}
	else if(strcasecmp(sep->arg[1], "create") == 0)
	{
		if(!sep->IsNumber(3) || !sep->IsNumber(4))
		{
			c->Message(0, "#instance create zone_id version duration - Creates an instance of version 'version' in the "
				"zone with id matching zone_id, will last for duration seconds.");
			return;
		}

		const char * zn = nullptr;
		uint32 zone_id = 0;

		if(sep->IsNumber(2))
		{
			zone_id = atoi(sep->arg[2]);
		}
		else
		{
			zone_id = database.GetZoneID(sep->arg[2]);
		}

		uint32 version = atoi(sep->arg[3]);
		uint32 duration = atoi(sep->arg[4]);
		zn = database.GetZoneName(zone_id);

		if(!zn)
		{
			c->Message(0, "Zone with id %lu was not found by the server.",  (unsigned long)zone_id);
			return;
		}

		uint16 id = 0;
		if(!database.GetUnusedInstanceID(id))
		{
			c->Message(0, "Server was unable to find a free instance id.");
			return;
		}

		if(!database.CreateInstance(id, zone_id, version, duration))
		{
			c->Message(0, "Server was unable to create a new instance.");
			return;
		}

		c->Message(0, "New instance %s was created with id %lu.",  zn, (unsigned long)id);
	}
	else if(strcasecmp(sep->arg[1], "destroy") == 0)
	{
		if(!sep->IsNumber(2))
		{
			c->Message(0, "#instance destroy instance_id - Destroys the instance with id matching instance_id.");
			return;
		}

		uint16 id = atoi(sep->arg[2]);
		database.DeleteInstance(id);
		c->Message(0, "Destroyed instance with id %lu.",  (unsigned long)id);
	}
	else if(strcasecmp(sep->arg[1], "add") == 0)
	{
		if(!sep->IsNumber(2))
		{
			c->Message(0, "#instance add instance_id player_name - adds the player 'player_name' to the instance "
				"with id matching instance_id.");
			return;
		}

		uint16 id = atoi(sep->arg[2]);
		uint32 charid = database.GetCharacterID(sep->arg[3]);

		if(id <= 0 || charid <= 0)
		{
			c->Message(0, "Must enter a valid instance id and player name.");
			return;
		}

		if(!database.CheckInstanceExists(id))
		{
			c->Message(0, "Instance does not exist.");
			return;
		}

		uint32 zone_id = database.ZoneIDFromInstanceID(id);
		uint32 version = database.VersionFromInstanceID(id);
		uint32 cur_id = database.GetInstanceID(zone_id, charid, version);
		if(cur_id == 0)
		{
			if(database.AddClientToInstance(id, charid))
			{
				c->Message(0, "Added client to instance.");
			}
			else
			{
				c->Message(0, "Failed to add client to instance.");
			}
		}
		else
		{
			c->Message(0, "Client was already saved to %u which has uses the same zone and version as that instance.",  cur_id);
		}
	}
	else if(strcasecmp(sep->arg[1], "remove") == 0)
	{
		if(!sep->IsNumber(2))
		{
			c->Message(0, "#instance remove instance_id player_name - removes the player 'player_name' from the "
				"instance with id matching instance_id.");
			return;
		}

		uint16 id = atoi(sep->arg[2]);
		uint32 charid = database.GetCharacterID(sep->arg[3]);

		if(id <= 0 || charid <= 0)
		{
			c->Message(0, "Must enter a valid instance id and player name.");
		}

		if(database.RemoveClientFromInstance(id, charid))
		{
			c->Message(0, "Removed client from instance.");
		}
		else
		{
			c->Message(0, "Failed to remove client from instance.");
		}
	}
	else if(strcasecmp(sep->arg[1], "list") == 0)
	{
		uint32 charid = database.GetCharacterID(sep->arg[2]);
		if(charid <= 0)
		{
			if(c->GetTarget() == nullptr || (c->GetTarget() && !c->GetTarget()->IsClient()))
			{
				c->Message(0, "Character not found.");
				return;
			}
			else
				charid = c->GetTarget()->CastToClient()->CharacterID();
		}

		database.ListAllInstances(c, charid);
	}
	else
	{
		c->Message(0, "Invalid Argument.");
		c->Message(0, "#instance usage:");
		c->Message(0, "#instance create zone_id version duration - Creates an instance of version 'version' in the "
			"zone with id matching zone_id, will last for duration seconds.");
		c->Message(0, "#instance destroy instance_id - Destroys the instance with id matching instance_id.");
		c->Message(0, "#instance add instance_id player_name - adds the player 'player_name' to the instance "
			"with id matching instance_id.");
		c->Message(0, "#instance remove instance_id player_name - removes the player 'player_name' from the "
			"instance with id matching instance_id.");
		c->Message(0, "#instance list player_name - lists all the instances 'player_name' is apart of.");
		return;
	}
}

void command_setstartzone(Client *c, const Seperator *sep)
{
	uint32 startzone = 0;
	Client* target = nullptr;
	if(c->GetTarget() && c->GetTarget()->IsClient() && sep->arg[1][0] != 0)
		target = c->GetTarget()->CastToClient();
	else {
		c->Message(0, "Usage: (needs PC target) #setstartzone zonename");
		c->Message(0, "Optional Usage: Use '#setstartzone reset' or '#setstartzone 0' to clear a starting zone. Player can select a starting zone using /setstartcity");
		return;
	}

	if(sep->IsNumber(1)) {
		startzone = atoi(sep->arg[1]);
	}
	else if(strcasecmp(sep->arg[1],"reset") == 0) {
		startzone = 0;
	}
	else {
		startzone = database.GetZoneID(sep->arg[1]);
		if(startzone == 0) {
			c->Message(0, "Unable to locate zone '%s'",  sep->arg[1]);
			return;
		}
	}

	target->SetStartZone(startzone);
}

void command_netstats(Client *c, const Seperator *sep)
{
	if(c)
	{
		if(c->GetTarget() && c->GetTarget()->IsClient())
		{
			c->Message(0, "Sent:");
			c->Message(0, "Total: %u, per second: %u",  c->GetTarget()->CastToClient()->Connection()->GetBytesSent(),
				c->GetTarget()->CastToClient()->Connection()->GetBytesSentPerSecond());
			c->Message(0, "Recieved:");
			c->Message(0, "Total: %u, per second: %u",  c->GetTarget()->CastToClient()->Connection()->GetBytesRecieved(),
				c->GetTarget()->CastToClient()->Connection()->GetBytesRecvPerSecond());

		}
		else
		{
			c->Message(0, "Sent:");
			c->Message(0, "Total: %u, per second: %u",  c->Connection()->GetBytesSent(), c->Connection()->GetBytesSentPerSecond());
			c->Message(0, "Recieved:");
			c->Message(0, "Total: %u, per second: %u",  c->Connection()->GetBytesRecieved(), c->Connection()->GetBytesRecvPerSecond());
		}
	}
}

void command_object(Client *c, const Seperator *sep)
{
	if (!c)
		return; // Crash Suppressant: No client. How did we get here?

	// Save it here. We sometimes have need to refer to it in multiple places.
	const char *usage_string = "Usage: #object List|Add|Edit|Move|Rotate|Save|Copy|Delete|Undo";

	if ((!sep) || (sep->argnum == 0)) {
		c->Message(0, usage_string);
		return;
	}

	Object *o = nullptr;
	Object_Struct od;
	Door door;
	Doors *doors;
	Door_Struct *ds;
	uint32 id = 0;
	uint32 itemid = 0;
	uint32 icon = 0;
	uint32 instance = 0;
	uint32 newid = 0;
	uint16 radius;
	EQApplicationPacket *app;

	bool bNewObject = false;

	float x2;
	float y2;

	// Temporary object type for static objects to allow manipulation
	// NOTE: Zone::LoadZoneObjects() currently loads this as an uint8, so max value is 255!
	static const uint32 staticType = 255;

	// Case insensitive commands (List == list == LIST)
	strlwr(sep->arg[1]);

	if (strcasecmp(sep->arg[1], "list") == 0) {
		// Insufficient or invalid args
		if ((sep->argnum < 2) || (sep->arg[2][0] < '0') ||
		    ((sep->arg[2][0] > '9') && ((sep->arg[2][0] & 0xDF) != 'A'))) {
			c->Message(0, "Usage: #object List All|(radius)");
			return;
		}

		if ((sep->arg[2][0] & 0xDF) == 'A')
			radius = 0; // List All
		else if ((radius = atoi(sep->arg[2])) <= 0)
			radius = 500; // Invalid radius. Default to 500 units.

		if (radius == 0)
			c->Message(0, "Objects within this zone:");
		else
			c->Message(0, "Objects within %u units of your current location:", radius);

		std::string query;
		if (radius)
			query = StringFormat(
			    "SELECT id, xpos, ypos, zpos, heading, itemid, "
			    "objectname, type, icon, unknown08, unknown10, unknown20 "
			    "FROM object WHERE zoneid = %u AND version = %u "
			    "AND (xpos BETWEEN %.1f AND %.1f) "
			    "AND (ypos BETWEEN %.1f AND %.1f) "
			    "AND (zpos BETWEEN %.1f AND %.1f) "
			    "ORDER BY id",
			    zone->GetZoneID(), zone->GetInstanceVersion(),
			    c->GetX() - radius, // Yes, we're actually using a bounding box instead of a radius.
			    c->GetX() + radius, // Much less processing power used this way.
			    c->GetY() - radius, c->GetY() + radius, c->GetZ() - radius, c->GetZ() + radius);
		else
			query = StringFormat("SELECT id, xpos, ypos, zpos, heading, itemid, "
					     "objectname, type, icon, unknown08, unknown10, unknown20 "
					     "FROM object WHERE zoneid = %u AND version = %u "
					     "ORDER BY id",
					     zone->GetZoneID(), zone->GetInstanceVersion());

		auto results = database.QueryDatabase(query);
		if (!results.Success()) {
			c->Message(0, "Error in objects query");
			return;
		}

		for (auto row = results.begin(); row != results.end(); ++row) {
			id = atoi(row[0]);
			od.x = atof(row[1]);
			od.y = atof(row[2]);
			od.z = atof(row[3]);
			od.heading = atof(row[4]);
			itemid = atoi(row[5]);
			strn0cpy(od.object_name, row[6], sizeof(od.object_name));
			od.object_name[sizeof(od.object_name) - 1] =
			    '\0'; // Required if strlen(row[col++]) exactly == sizeof(object_name)

			od.object_type = atoi(row[7]);
			icon = atoi(row[8]);
			od.size = atoi(row[9]);
			od.solidtype = atoi(row[10]);
			od.unknown020 = atoi(row[11]);

			switch (od.object_type) {
			case 0:				// Static Object
			case staticType:		// Static Object unlocked for changes
				if (od.size == 0) // Unknown08 field is optional Size parameter for static objects
					od.size = 100; // Static object default Size is 100%

				c->Message(0, "- STATIC Object (%s): id %u, x %.1f, y %.1f, z %.1f, h %.1f, model %s, "
					      "size %u, solidtype %u, incline %u",
					   (od.object_type == 0) ? "locked" : "unlocked", id, od.x, od.y, od.z,
					   od.heading, od.object_name, od.size, od.solidtype, od.unknown020);
				break;

			case OT_DROPPEDITEM: // Ground Spawn
				c->Message(0, "- TEMPORARY Object: id %u, x %.1f, y %.1f, z %.1f, h %.1f, itemid %u, "
					      "model %s, icon %u",
					   id, od.x, od.y, od.z, od.heading, itemid, od.object_name, icon);
				break;

			default: // All others == Tradeskill Objects
				c->Message(0, "- TRADESKILL Object: id %u, x %.1f, y %.1f, z %.1f, h %.1f, model %s, "
					      "type %u, icon %u",
					   id, od.x, od.y, od.z, od.heading, od.object_name, od.object_type, icon);
				break;
			}
		}

		c->Message(0, "%u object%s found", results.RowCount(), (results.RowCount() == 1) ? "" : "s");
		return;
	}

	if (strcasecmp(sep->arg[1], "add") == 0) {
		// Insufficient or invalid arguments
		if ((sep->argnum < 3) ||
		    ((sep->arg[3][0] == '\0') && (sep->arg[4][0] < '0') && (sep->arg[4][0] > '9'))) {
			c->Message(0, "Usage: (Static Object): #object Add [ObjectID] 0 Model [SizePercent] "
				      "[SolidType] [Incline]");
			c->Message(0, "Usage: (Tradeskill Object): #object Add [ObjectID] TypeNum Model Icon");
			c->Message(0, "- Notes: Model must start with a letter, max length 16. SolidTypes = 0 (Solid), "
				      "1 (Sometimes Non-Solid)");
			return;
		}

		int col;

		if (sep->argnum > 3) { // Model name in arg3?
			if ((sep->arg[3][0] <= '9') && (sep->arg[3][0] >= '0')) {
				// Nope, user must have specified ObjectID. Extract it.
				id = atoi(sep->arg[2]);
				col = 1; // Bump all other arguments one to the right. Model is in arg4.
			} else {
				// Yep, arg3 is non-numeric, ObjectID must be omitted and model must be arg3
				id = 0;
				col = 0;
			}
		} else {
			// Nope, only 3 args. Object ID must be omitted and arg3 must be model.
			id = 0;
			col = 0;
		}

		memset(&od, 0, sizeof(od));

		od.object_type = atoi(sep->arg[2 + col]);

		switch (od.object_type) {
		case 0: // Static Object
			if ((sep->argnum - col) > 3) {
				od.size = atoi(sep->arg[4 + col]); // Size specified

				if ((sep->argnum - col) > 4) {
					od.solidtype = atoi(sep->arg[5 + col]); // SolidType specified

					if ((sep->argnum - col) > 5)
						od.unknown020 = atoi(sep->arg[6 + col]); // Incline specified
				}
			}
			break;

		case 1: // Ground Spawn
			c->Message(0, "ERROR: Object Type 1 is used for temporarily spawned ground spawns and dropped "
				      "items, which are not supported with #object. See the 'ground_spawns' table in "
				      "the database.");
			return;

		default: // Everything else == Tradeskill Object
			icon = ((sep->argnum - col) > 3) ? atoi(sep->arg[4 + col]) : 0;

			if (icon == 0) {
				c->Message(0, "ERROR: Required property 'Icon' not specified for Tradeskill Object");
				return;
			}

			break;
		}

		od.x = c->GetX();
		od.y = c->GetY();
		od.z = c->GetZ() - (c->GetSize() * 0.625f);
		od.heading = c->GetHeading() * 2.0f; // GetHeading() is half of actual. Compensate by doubling.

		std::string query;
		if (id) {
			// ID specified. Verify that it doesn't already exist.
			query = StringFormat("SELECT COUNT(*) FROM object WHERE ID = %u", id);
			auto results = database.QueryDatabase(query);
			if (results.Success() && results.RowCount() != 0) {
				auto row = results.begin();
				if (atoi(row[0]) > 0) // Yep, in database already.
					id = 0;
			}

			// Not in database. Already spawned, just not saved?
			// Yep, already spawned.
			if (id && entity_list.FindObject(id))
				id = 0;

			if (id == 0) {
				c->Message(0, "ERROR: An object already exists with the id %u", atoi(sep->arg[2]));
				return;
			}
		}

		int objectsFound = 0;
		// Verify no other objects already in this spot (accidental double-click of Hotkey?)
		query = StringFormat(
		    "SELECT COUNT(*) FROM object WHERE zoneid = %u "
		    "AND version=%u AND (xpos BETWEEN %.1f AND %.1f) "
		    "AND (ypos BETWEEN %.1f AND %.1f) "
		    "AND (zpos BETWEEN %.1f AND %.1f)",
		    zone->GetZoneID(), zone->GetInstanceVersion(), od.x - 0.2f,
		    od.x + 0.2f,	       // Yes, we're actually using a bounding box instead of a radius.
		    od.y - 0.2f, od.y + 0.2f,  // Much less processing power used this way.
		    od.z - 0.2f, od.z + 0.2f); // It's pretty forgiving, though, allowing for close-proximity objects

		auto results = database.QueryDatabase(query);
		if (results.Success() && results.RowCount() != 0) {
			auto row = results.begin();
			objectsFound = atoi(row[0]); // Number of nearby objects from database
		}

		// No objects found in database too close. How about spawned but not yet saved?
		if (objectsFound == 0 && entity_list.FindNearbyObject(od.x, od.y, od.z, 0.2f))
			objectsFound = 1;

		if (objectsFound) {
			c->Message(0, "ERROR: Object already at this location.");
			return;
		}

		// Strip any single quotes from objectname (SQL injection FTL!)
		strn0cpy(od.object_name, sep->arg[3 + col], sizeof(od.object_name));

		uint32 len = strlen(od.object_name);
		for (col = 0; col < (uint32)len; col++) {
			if (od.object_name[col] != '\'')
				continue;

			// Uh oh, 1337 h4x0r monkeying around! Strip that apostrophe!
			memcpy(&od.object_name[col], &od.object_name[col + 1], len - col);
			len--;
			col--;
		}

		strupr(od.object_name); // Model names are always upper-case.

		if ((od.object_name[0] < 'A') || (od.object_name[0] > 'Z')) {
			c->Message(0, "ERROR: Model name must start with a letter.");
			return;
		}

		if (id == 0) {
			// No ID specified. Get a best-guess next number from the database
			// If there's a problem retrieving an ID from the database, it'll end up being object # 1. No
			// biggie.

			query = "SELECT MAX(id) FROM object";
			results = database.QueryDatabase(query);
			if (results.Success() && results.RowCount() != 0) {
				auto row = results.begin();
				id = atoi(row[0]);
			}

			id++;
		}

		// Make sure not to overwrite already-spawned objects that haven't been saved yet.
		while (o = entity_list.FindObject(id))
			id++;

		// Static object
		if (od.object_type == 0)
			od.object_type = staticType; // Temporary. We'll make it 0 when we Save

		od.zone_id = zone->GetZoneID();
		od.zone_instance = zone->GetInstanceVersion();

		o = new Object(id, od.object_type, icon, od, nullptr);

		// Add to our zone entity list and spawn immediately for all clients
		entity_list.AddObject(o, true);

		// Bump player back to avoid getting stuck inside new object

		// GetHeading() returns half of the actual heading, for some reason, so we'll double it here for
		// computation
		x2 = 10.0f * sin(c->GetHeading() * 2.0f / 256.0f * 3.14159265f);
		y2 = 10.0f * cos(c->GetHeading() * 2.0f / 256.0f * 3.14159265f);
		c->MovePC(c->GetX() - x2, c->GetY() - y2, c->GetZ(), c->GetHeading() * 2);

		c->Message(0, "Spawning object with tentative id %u at location (%.1f, %.1f, %.1f heading %.1f). Use "
			      "'#object Save' to save to database when satisfied with placement.",
			   id, od.x, od.y, od.z, od.heading);

		// Temporary Static Object
		if (od.object_type == staticType)
			c->Message(0, "- Note: Static Object will act like a tradeskill container and will not reflect "
				      "size, solidtype, or incline values until you commit with '#object Save', after "
				      "which it will be unchangeable until you use '#object Edit' and zone back in.");

		return;
	}

	if (strcasecmp(sep->arg[1], "edit") == 0) {

		if ((sep->argnum < 2) || ((id = atoi(sep->arg[2])) < 1)) {
			c->Message(0, "Usage: #object Edit (ObjectID) [PropertyName] [NewValue]");
			c->Message(0, "- Static Object (Type 0) Properties: model, type, size, solidtype, incline");
			c->Message(0, "- Tradeskill Object (Type 2+) Properties: model, type, icon");

			return;
		}

		o = entity_list.FindObject(id);

		// Object already available in-zone?
		if (o) {
			// Yep, looks like we can make real-time changes.
			if (sep->argnum < 4) {
				// Or not. '#object Edit (ObjectID)' called without PropertyName and NewValue
				c->Message(0, "Note: Object %u already unlocked and ready for changes", id);
				return;
			}
		} else {
			// Object not found in-zone in a modifiable form. Check for valid matching circumstances.
			std::string query = StringFormat("SELECT zoneid, version, type FROM object WHERE id = %u", id);
			auto results = database.QueryDatabase(query);
			if (!results.Success() || results.RowCount() == 0) {
				c->Message(0, "ERROR: Object %u not found", id);
				return;
			}

			auto row = results.begin();
			od.zone_id = atoi(row[0]);
			od.zone_instance = atoi(row[1]);
			od.object_type = atoi(row[2]);
			uint32 objectsFound = 1;

			// Object not in this zone?
			if (od.zone_id != zone->GetZoneID()) {
				c->Message(0, "ERROR: Object %u not in this zone.", id);
				return;
			}

			// Object not in this instance?
			if (od.zone_instance != zone->GetInstanceVersion()) {
				c->Message(0, "ERROR: Object %u not part of this instance version.", id);
				return;
			}

			switch (od.object_type) {
			case 0: // Static object needing unlocking
				// Convert to tradeskill object temporarily for changes
				query = StringFormat("UPDATE object SET type = %u WHERE id = %u", staticType, id);

				database.QueryDatabase(query);

				c->Message(0, "Static Object %u unlocked for editing. You must zone out and back in to "
					      "make your changes, then commit them with '#object Save'.",
					   id);
				if (sep->argnum >= 4)
					c->Message(0, "NOTE: The change you specified has not been applied, since the "
						      "static object had not been unlocked for editing yet.");
				return;

			case OT_DROPPEDITEM:
				c->Message(0, "ERROR: Object %u is a temporarily spawned ground spawn or dropped item, "
					      "which cannot be manipulated with #object. See the 'ground_spawns' table "
					      "in the database.",
					   id);
				return;

			case staticType:
				c->Message(0, "ERROR: Object %u has been unlocked for editing, but you must zone out "
					      "and back in for your client to refresh its object table before you can "
					      "make changes to it.",
					   id);
				return;

			default:
				// Unknown error preventing us from seeing the object in the zone.
				c->Message(0, "ERROR: Unknown problem attempting to manipulate object %u", id);
				return;
			}
		}

		// If we're here, we have a manipulable object ready for changes.
		strlwr(sep->arg[3]); // Case insensitive PropertyName
		strupr(sep->arg[4]); // In case it's model name, which should always be upper-case

		// Read current object info for reference
		icon = o->GetIcon();
		o->GetObjectData(&od);

		// We'll be a little more picky with property names, to prevent errors. Check against the whole word.
		if (strcmp(sep->arg[3], "model") == 0) {

			if ((sep->arg[4][0] < 'A') || (sep->arg[4][0] > 'Z')) {
				c->Message(0, "ERROR: Model names must begin with a letter.");
				return;
			}

			strn0cpy(od.object_name, sep->arg[4], sizeof(od.object_name));

			o->SetObjectData(&od);

			c->Message(0, "Object %u now being rendered with model '%s'", id, od.object_name);
		} else if (strcmp(sep->arg[3], "type") == 0) {
			if ((sep->arg[4][0] < '0') || (sep->arg[4][0] > '9')) {
				c->Message(0, "ERROR: Invalid type number");
				return;
			}

			od.object_type = atoi(sep->arg[4]);

			switch (od.object_type) {
			case 0:
				// Convert Static Object to temporary changeable type
				od.object_type = staticType;
				c->Message(0, "Note: Static Object will still act like tradeskill object and will not "
					      "reflect size, solidtype, or incline settings until committed to the "
					      "database with '#object Save', after which it will be unchangeable until "
					      "it is unlocked again with '#object Edit'.");
				break;

			case OT_DROPPEDITEM:
				c->Message(0, "ERROR: Object Type 1 is used for temporarily spawned ground spawns and "
					      "dropped items, which are not supported with #object. See the "
					      "'ground_spawns' table in the database.");
				return;

			default:
				c->Message(0, "Object %u changed to Tradeskill Object Type %u", id, od.object_type);
				break;
			}

			o->SetType(od.object_type);
		} else if (strcmp(sep->arg[3], "size") == 0) {
			if (od.object_type != staticType) {
				c->Message(
				    0, "ERROR: Object %u is not a Static Object and does not support the Size property",
				    id);
				return;
			}

			if ((sep->arg[4][0] < '0') || (sep->arg[4][0] > '9')) {
				c->Message(0, "ERROR: Invalid size specified. Please enter a number.");
				return;
			}

			od.size = atoi(sep->arg[4]);
			o->SetObjectData(&od);

			if (od.size == 0) // 0 == unspecified == 100%
				od.size = 100;

			c->Message(0, "Static Object %u set to %u%% size. Size will take effect when you commit to the "
				      "database with '#object Save', after which the object will be unchangeable until "
				      "you unlock it again with '#object Edit' and zone out and back in.",
				   id, od.size);
		} else if (strcmp(sep->arg[3], "solidtype") == 0) {

			if (od.object_type != staticType) {
				c->Message(0, "ERROR: Object %u is not a Static Object and does not support the "
					      "SolidType property",
					   id);
				return;
			}

			if ((sep->arg[4][0] < '0') || (sep->arg[4][0] > '9')) {
				c->Message(0, "ERROR: Invalid solidtype specified. Please enter a number.");
				return;
			}

			od.solidtype = atoi(sep->arg[4]);
			o->SetObjectData(&od);

			c->Message(0, "Static Object %u set to SolidType %u. Change will take effect when you commit "
				      "to the database with '#object Save'. Support for this property is on a "
				      "per-model basis, mostly seen in smaller objects such as chests and tables.",
					  id, od.solidtype);
		} else if (strcmp(sep->arg[3], "icon") == 0) {

			if ((od.object_type < 2) || (od.object_type == staticType)) {
				c->Message(0, "ERROR: Object %u is not a Tradeskill Object and does not support the "
					      "Icon property",
					   id);
				return;
			}

			if ((icon = atoi(sep->arg[4])) == 0) {
				c->Message(0, "ERROR: Invalid Icon specified. Please enter an icon number.");
				return;
			}

			o->SetIcon(icon);
			c->Message(0, "Tradeskill Object %u icon set to %u", id, icon);
		} else if (strcmp(sep->arg[3], "incline") == 0) {
			if (od.object_type != staticType) {
				c->Message(
				    0,
				    "ERROR: Object %u is not a Static Object and does not support the Incline property",
				    id);
				return;
			}

			if ((sep->arg[4][0] < '0') || (sep->arg[4][0] > '9')) {
				c->Message(
				    0,
				    "ERROR: Invalid Incline specified. Please enter a number. Normal range is 0-512.");
				return;
			}

			od.unknown020 = atoi(sep->arg[4]);
			o->SetObjectData(&od);

			c->Message(0, "Static Object %u set to %u incline. Incline will take effect when you commit to "
				      "the database with '#object Save', after which the object will be unchangeable "
				      "until you unlock it again with '#object Edit' and zone out and back in.",
				   id, od.unknown020);
		} else {
			c->Message(0, "ERROR: Unrecognized property name: %s", sep->arg[3]);
			return;
		}

		// Repop object to have it reflect the change.
		app = new EQApplicationPacket();
		o->CreateDeSpawnPacket(app);
		entity_list.QueueClients(0, app);
		safe_delete(app);

		app = new EQApplicationPacket();
		o->CreateSpawnPacket(app);
		entity_list.QueueClients(0, app);
		safe_delete(app);
		return;
	}

	if (strcasecmp(sep->arg[1], "move") == 0) {

		if ((sep->argnum < 2) ||	       // Not enough arguments
		    ((id = atoi(sep->arg[2])) == 0) || // ID not specified
		    (((sep->arg[3][0] < '0') || (sep->arg[3][0] > '9')) && ((sep->arg[3][0] & 0xDF) != 'T') &&
		     (sep->arg[3][0] != '-') && (sep->arg[3][0] != '.'))) { // Location argument not specified correctly
			c->Message(0, "Usage: #object Move (ObjectID) ToMe|(x y z [h])");
			return;
		}

		if (!(o = entity_list.FindObject(id))) {
			std::string query = StringFormat("SELECT zoneid, version, type FROM object WHERE id = %u", id);
			auto results = database.QueryDatabase(query);
			if (!results.Success() || results.RowCount() == 0) {
				c->Message(0, "ERROR: Object %u not found", id);
				return;
			}

			auto row = results.begin();
			od.zone_id = atoi(row[0]);
			od.zone_instance = atoi(row[1]);
			od.object_type = atoi(row[2]);

			if (od.zone_id != zone->GetZoneID()) {
				c->Message(0, "ERROR: Object %u is not in this zone", id);
				return;
			}

			if (od.zone_instance != zone->GetInstanceVersion()) {
				c->Message(0, "ERROR: Object %u is not in this instance version", id);
				return;
			}

			switch (od.object_type) {
			case 0:
				c->Message(0, "ERROR: Object %u is not yet unlocked for editing. Use '#object Edit' "
					      "then zone out and back in to move it.",
					   id);
				return;

			case staticType:
				c->Message(0, "ERROR: Object %u has been unlocked for editing, but you must zone out "
					      "and back in before your client sees the change and will allow you to "
					      "move it.",
					   id);
				return;

			case 1:
				c->Message(0, "ERROR: Object %u is a temporary spawned object and cannot be "
					      "manipulated with #object. See the 'ground_spawns' table in the "
					      "database.",
					   id);
				return;

			default:
				c->Message(0, "ERROR: Object %u not located in zone.", id);
				return;
			}
		}

		// Move To Me
		if ((sep->arg[3][0] & 0xDF) == 'T') {
			od.x = c->GetX();
			od.y = c->GetY();
			od.z = c->GetZ() -
			       (c->GetSize() *
				0.625f); // Compensate for #loc bumping up Z coordinate by 62.5% of character's size.

			o->SetHeading(c->GetHeading() * 2.0f); // Compensate for GetHeading() returning half of actual

			// Bump player back to avoid getting stuck inside object

			// GetHeading() returns half of the actual heading, for some reason
			x2 = 10.0f * sin(c->GetHeading() * 2.0f / 256.0f * 3.14159265f);
			y2 = 10.0f * cos(c->GetHeading() * 2.0f / 256.0f * 3.14159265f);
			c->MovePC(c->GetX() - x2, c->GetY() - y2, c->GetZ(), c->GetHeading() * 2.0f);
		} // Move to x, y, z [h]
		else {
			od.x = atof(sep->arg[3]);
			if (sep->argnum > 3)
				od.y = atof(sep->arg[4]);
			else
				o->GetLocation(nullptr, &od.y, nullptr);

			if (sep->argnum > 4)
				od.z = atof(sep->arg[5]);
			else
				o->GetLocation(nullptr, nullptr, &od.z);

			if (sep->argnum > 5)
				o->SetHeading(atof(sep->arg[6]));
		}

		o->SetLocation(od.x, od.y, od.z);

		// Despawn and respawn object to reflect change
		app = new EQApplicationPacket();
		o->CreateDeSpawnPacket(app);
		entity_list.QueueClients(0, app);
		safe_delete(app);

		app = new EQApplicationPacket();
		o->CreateSpawnPacket(app);
		entity_list.QueueClients(0, app);
		safe_delete(app);
		return;
	}

	if (strcasecmp(sep->arg[1], "rotate") == 0) {
		// Insufficient or invalid arguments
		if ((sep->argnum < 3) || ((id = atoi(sep->arg[2])) == 0)) {
			c->Message(0, "Usage: #object Rotate (ObjectID) (Heading, 0-512)");
			return;
		}

		if ((o = entity_list.FindObject(id)) == nullptr) {
			c->Message(0, "ERROR: Object %u not found in zone, or is a static object not yet unlocked with "
				      "'#object Edit' for editing.",
				   id);
			return;
		}

		o->SetHeading(atof(sep->arg[3]));

		// Despawn and respawn object to reflect change
		app = new EQApplicationPacket();
		o->CreateDeSpawnPacket(app);
		entity_list.QueueClients(0, app);
		safe_delete(app);

		app = new EQApplicationPacket();
		o->CreateSpawnPacket(app);
		entity_list.QueueClients(0, app);
		safe_delete(app);
		return;
	}

	if (strcasecmp(sep->arg[1], "save") == 0) {
		// Insufficient or invalid arguments
		if ((sep->argnum < 2) || ((id = atoi(sep->arg[2])) == 0)) {
			c->Message(0, "Usage: #object Save (ObjectID)");
			return;
		}

		o = entity_list.FindObject(id);

		od.zone_id = 0;
		od.zone_instance = 0;
		od.object_type = 0;

		// If this ID isn't in the database yet, it's a new object
		bNewObject = true;
		std::string query = StringFormat("SELECT zoneid, version, type FROM object WHERE id = %u", id);
		auto results = database.QueryDatabase(query);
		if (results.Success() && results.RowCount() != 0) {
			auto row = results.begin();
			od.zone_id = atoi(row[0]);
			od.zone_instance = atoi(row[1]);
			od.object_type = atoi(row[2]);

			// ID already in database. Not a new object.
			bNewObject = false;
		}

		if (!o) {
			// Object not found in zone. Can't save an object we can't see.

			if (bNewObject) {
				c->Message(0, "ERROR: Object %u not found", id);
				return;
			}

			if (od.zone_id != zone->GetZoneID()) {
				c->Message(0, "ERROR: Wrong Object ID. %u is not part of this zone.", id);
				return;
			}

			if (od.zone_instance != zone->GetInstanceVersion()) {
				c->Message(0, "ERROR: Wrong Object ID. %u is not part of this instance version.", id);
				return;
			}

			if (od.object_type == 0) {
				c->Message(0, "ERROR: Static Object %u has already been committed. Use '#object Edit "
					      "%u' and zone out and back in to make changes.",
					   id, id);
				return;
			}

			if (od.object_type == 1) {
				c->Message(0, "ERROR: Object %u is a temporarily spawned ground spawn or dropped item, "
					      "which is not supported with #object. See the 'ground_spawns' table in "
					      "the database.",
					   id);
				return;
			}

			c->Message(0, "ERROR: Object %u not found.", id);
			return;
		}

		// Oops! Another GM already saved an object with our id from another zone.
		// We'll have to get a new one.
		if ((od.zone_id > 0) && (od.zone_id != zone->GetZoneID()))
			id = 0;

		// Oops! Another GM already saved an object with our id from another instance.
		// We'll have to get a new one.
		if ((id > 0) && (od.zone_instance != zone->GetInstanceVersion()))
			id = 0;

		// If we're asking for a new ID, it's a new object.
		bNewObject |= (id == 0);

		o->GetObjectData(&od);
		od.object_type = o->GetType();
		icon = o->GetIcon();

		// We're committing to the database now. Return temporary object type to actual.
		if (od.object_type == staticType)
			od.object_type = 0;

		if (!bNewObject)
			query = StringFormat("UPDATE object SET zoneid = %u, version = %u, "
					     "xpos = %.1f, ypos=%.1f, zpos=%.1f, heading=%.1f, "
					     "objectname = '%s', type = %u, icon = %u, "
					     "unknown08 = %u, unknown10 = %u, unknown20 = %u "
					     "WHERE ID = %u",
					     zone->GetZoneID(), zone->GetInstanceVersion(), od.x, od.y, od.z,
					     od.heading, od.object_name, od.object_type, icon, od.size,
						 od.solidtype, od.unknown020, id);
		else if (id == 0)
			query = StringFormat("INSERT INTO object "
					     "(zoneid, version, xpos, ypos, zpos, heading, objectname, "
					     "type, icon, unknown08, unknown10, unknown20) "
					     "VALUES (%u, %u, %.1f, %.1f, %.1f, %.1f, '%s', %u, %u, %u, %u, %u)",
					     zone->GetZoneID(), zone->GetInstanceVersion(), od.x, od.y, od.z,
					     od.heading, od.object_name, od.object_type, icon, od.size,
					     od.solidtype, od.unknown020);
		else
			query = StringFormat("INSERT INTO object "
					     "(id, zoneid, version, xpos, ypos, zpos, heading, objectname, "
					     "type, icon, unknown08, unknown10, unknown20) "
					     "VALUES (%u, %u, %u, %.1f, %.1f, %.1f, %.1f, '%s', %u, %u, %u, %u, %u)",
					     id, zone->GetZoneID(), zone->GetInstanceVersion(), od.x, od.y, od.z,
					     od.heading, od.object_name, od.object_type, icon, od.size,
					     od.solidtype, od.unknown020);

		results = database.QueryDatabase(query);
		if (!results.Success()) {
			c->Message(0, "Database Error: %s", results.ErrorMessage().c_str());
			return;
		}

		if (results.RowsAffected() == 0) {
			// No change made, but no error message given
			c->Message(0, "Database Error: Could not save change to Object %u", id);
			return;
		}

		if (bNewObject) {
			if (newid == results.LastInsertedID()) {
				c->Message(0, "Saved new Object %u to database", id);
				return;
			}

			c->Message(0, "Saved Object. NOTE: Database returned a new ID number for object: %u", newid);
			id = newid;
			return;
		}

		c->Message(0, "Saved changes to Object %u", id);
		newid = id;

		if (od.object_type == 0) {
			// Static Object - Respawn as nonfunctional door

			app = new EQApplicationPacket();
			o->CreateDeSpawnPacket(app);
			entity_list.QueueClients(0, app);
			safe_delete(app);

			entity_list.RemoveObject(o->GetID());

			memset(&door, 0, sizeof(door));

			strn0cpy(door.zone_name, zone->GetShortName(), sizeof(door.zone_name));

			door.db_id = 1000000000 + id; // Out of range of normal use for doors.id
			door.door_id = -1;	    // Client doesn't care if these are all the same door_id
			door.pos_x = od.x;	    // xpos
			door.pos_y = od.y;	    // ypos
			door.pos_z = od.z;	    // zpos
			door.heading = od.heading;    // heading

			strn0cpy(door.door_name, od.object_name, sizeof(door.door_name)); // objectname

			// Strip trailing "_ACTORDEF" if present. Client won't accept it for doors.
			uint32 len = strlen(door.door_name);
			if ((len > 9) && (memcmp(&door.door_name[len - 9], "_ACTORDEF", 10) == 0))
				door.door_name[len - 9] = '\0';

			memcpy(door.dest_zone, "NONE", 5);

			if ((door.size = od.size) == 0) // unknown08 = optional size percentage
				door.size = 100;

			switch (
			    door.opentype =
				od.solidtype) // unknown10 = optional request_nonsolid (0 or 1 or experimental number)
			{
			case 0:
				door.opentype = 31;
				break;

			case 1:
				door.opentype = 9;
				break;
			}

			door.incline = od.unknown020; // unknown20 = optional incline value
			door.client_version_mask = 0xFFFFFFFF;

			doors = new Doors(&door);
			entity_list.AddDoor(doors);

			app = new EQApplicationPacket(OP_SpawnDoor, sizeof(Door_Struct));
			ds = (Door_Struct *)app->pBuffer;

			memset(ds, 0, sizeof(Door_Struct));
			memcpy(ds->name, door.door_name, 32);
			ds->xPos = door.pos_x;
			ds->yPos = door.pos_y;
			ds->zPos = door.pos_z;
			ds->heading = door.heading;
			ds->incline = door.incline;
			ds->size = door.size;
			ds->doorId = door.door_id;
			ds->opentype = door.opentype;
			ds->unknown0052[9] = 1; // *ptr-1 and *ptr-3 from EntityList::MakeDoorSpawnPacket()
			ds->unknown0052[11] = 1;

			entity_list.QueueClients(0, app);
			safe_delete(app);

			c->Message(0, "NOTE: Object %u is now a static object, and is unchangeable. To make future "
				      "changes, use '#object Edit' to convert it to a changeable form, then zone out "
				      "and back in.",
				   id);
		}
		return;
	}

	if (strcasecmp(sep->arg[1], "copy") == 0) {
		// Insufficient or invalid arguments
		if ((sep->argnum < 3) ||
		    (((sep->arg[2][0] & 0xDF) != 'A') && ((sep->arg[2][0] < '0') || (sep->arg[2][0] > '9')))) {
			c->Message(0, "Usage: #object Copy All|(ObjectID) (InstanceVersion)");
			c->Message(0, "- Note: Only objects saved in the database can be copied to another instance.");
			return;
		}

		od.zone_instance = atoi(sep->arg[3]);

		if (od.zone_instance == zone->GetInstanceVersion()) {
			c->Message(0, "ERROR: Source and destination instance versions are the same.");
			return;
		}

		if ((sep->arg[2][0] & 0xDF) == 'A') {
			// Copy All

			std::string query =
			    StringFormat("INSERT INTO object "
					 "(zoneid, version, xpos, ypos, zpos, heading, itemid, "
					 "objectname, type, icon, unknown08, unknown10, unknown20) "
					 "SELECT zoneid, %u, xpos, ypos, zpos, heading, itemid, "
					 "objectname, type, icon, unknown08, unknown10, unknown20 "
					 "FROM object WHERE zoneid = %u) AND version = %u",
					 od.zone_instance, zone->GetZoneID(), zone->GetInstanceVersion());
			auto results = database.QueryDatabase(query);
			if (!results.Success()) {
				c->Message(0, "Database Error: %s", results.ErrorMessage().c_str());
				return;
			}

			c->Message(0, "Copied %u object%s into instance version %u", results.RowCount(),
				   (results.RowCount() == 1) ? "" : "s", od.zone_instance);
			return;
		}

		id = atoi(sep->arg[2]);

		std::string query = StringFormat("INSERT INTO object "
						 "(zoneid, version, xpos, ypos, zpos, heading, itemid, "
						 "objectname, type, icon, unknown08, unknown10, unknown20) "
						 "SELECT zoneid, %u, xpos, ypos, zpos, heading, itemid, "
						 "objectname, type, icon, unknown08, unknown10, unknown20 "
						 "FROM object WHERE id = %u AND zoneid = %u AND version = %u",
						 od.zone_instance, id, zone->GetZoneID(), zone->GetInstanceVersion());
		auto results = database.QueryDatabase(query);
		if (results.Success() && results.RowsAffected() > 0) {
			c->Message(0, "Copied Object %u into instance version %u", id, od.zone_instance);
			return;
		}

		// Couldn't copy the object.

		// got an error message
		if (!results.Success()) {
			c->Message(0, "Database Error: %s", results.ErrorMessage().c_str());
			return;
		}

		// No database error returned. See if we can figure out why.

		query = StringFormat("SELECT zoneid, version FROM object WHERE id = %u", id);
		results = database.QueryDatabase(query);
		if (!results.Success())
			return;

		if (results.RowCount() == 0) {
			c->Message(0, "ERROR: Object %u not found", id);
			return;
		}

		auto row = results.begin();
		// Wrong ZoneID?
		if (atoi(row[0]) != zone->GetZoneID()) {
			c->Message(0, "ERROR: Object %u is not part of this zone.", id);
			return;
		}

		// Wrong Instance Version?
		if (atoi(row[1]) != zone->GetInstanceVersion()) {
			c->Message(0, "ERROR: Object %u is not part of this instance version.", id);
			return;
		}

		// Well, NO clue at this point. Just let 'em know something screwed up.
		c->Message(0, "ERROR: Unknown database error copying Object %u to instance version %u", id,
			   od.zone_instance);
		return;
	}

	if (strcasecmp(sep->arg[1], "delete") == 0) {

		if ((sep->argnum < 2) || ((id = atoi(sep->arg[2])) <= 0)) {
			c->Message(0, "Usage: #object Delete (ObjectID) -- NOTE: Object deletions are permanent and "
				      "cannot be undone!");
			return;
		}

		o = entity_list.FindObject(id);

		if (o) {
			// Object found in zone.

			app = new EQApplicationPacket();
			o->CreateDeSpawnPacket(app);
			entity_list.QueueClients(nullptr, app);

			entity_list.RemoveObject(o->GetID());

			// Verifying ZoneID and Version in case someone else ended up adding an object with our ID
			// from a different zone/version. Don't want to delete someone else's work.
			std::string query = StringFormat("DELETE FROM object "
							 "WHERE id = %u AND zoneid = %u "
							 "AND version = %u LIMIT 1",
							 id, zone->GetZoneID(), zone->GetInstanceVersion());
			auto results = database.QueryDatabase(query);

			c->Message(0, "Object %u deleted", id);
			return;
		}

		// Object not found in zone.
		std::string query = StringFormat("SELECT type FROM object "
						 "WHERE id = %u AND zoneid = %u "
						 "AND version = %u LIMIT 1",
						 id, zone->GetZoneID(), zone->GetInstanceVersion());
		auto results = database.QueryDatabase(query);
		if (!results.Success())
			return;

		if (results.RowCount() == 0) {
			c->Message(0, "ERROR: Object %u not found in this zone or instance!", id);
			return;
		}

		auto row = results.begin();

		switch (atoi(row[0])) {
		case 0: // Static Object
			query = StringFormat("DELETE FROM object WHERE id = %u "
					     "AND zoneid = %u AND version = %u LIMIT 1",
					     id, zone->GetZoneID(), zone->GetInstanceVersion());
			results = database.QueryDatabase(query);

			c->Message(0, "Object %u deleted. NOTE: This static object will remain for anyone currently in "
				      "the zone until they next zone out and in.",
				   id);
			return;

		case 1: // Temporary Spawn
			c->Message(0, "ERROR: Object %u is a temporarily spawned ground spawn or dropped item, which "
				      "is not supported with #object. See the 'ground_spawns' table in the database.",
				   id);
			return;
		}

		return;
	}

	if (strcasecmp(sep->arg[1], "undo") == 0) {
		// Insufficient or invalid arguments
		if ((sep->argnum < 2) || ((id = atoi(sep->arg[2])) == 0)) {
			c->Message(0, "Usage: #object Undo (ObjectID) -- Reload object from database, undoing any "
				      "changes you have made");
			return;
		}

		o = entity_list.FindObject(id);

		if (!o) {
			c->Message(0, "ERROR: Object %u not found in zone in a manipulable form. No changes to undo.",
				   id);
			return;
		}

		if (o->GetType() == OT_DROPPEDITEM) {
			c->Message(0, "ERROR: Object %u is a temporary spawned item and cannot be manipulated with "
				      "#object. See the 'ground_spawns' table in the database.",
				   id);
			return;
		}

		// Despawn current item for reloading from database
		app = new EQApplicationPacket();
		o->CreateDeSpawnPacket(app);
		entity_list.QueueClients(0, app);
		entity_list.RemoveObject(o->GetID());
		safe_delete(app);

		std::string query = StringFormat("SELECT xpos, ypos, zpos, "
						 "heading, objectname, type, icon, "
						 "unknown08, unknown10, unknown20 "
						 "FROM object WHERE id = %u",
						 id);
		auto results = database.QueryDatabase(query);
		if (!results.Success() || results.RowCount() == 0) {
			c->Message(0, "Database Error: %s", results.ErrorMessage().c_str());
			return;
		}

		memset(&od, 0, sizeof(od));

		auto row = results.begin();

		od.x = atof(row[0]);
		od.y = atof(row[1]);
		od.z = atof(row[2]);
		od.heading = atof(row[3]);
		strn0cpy(od.object_name, row[4], sizeof(od.object_name));
		od.object_type = atoi(row[5]);
		icon = atoi(row[6]);
		od.size = atoi(row[7]);
		od.solidtype = atoi(row[8]);
		od.unknown020 = atoi(row[9]);

		if (od.object_type == 0)
			od.object_type = staticType;

		o = new Object(id, od.object_type, icon, od, nullptr);
		entity_list.AddObject(o, true);

		c->Message(0, "Object %u reloaded from database.", id);
		return;
	}

	c->Message(0, usage_string);
}

void command_showspellslist(Client *c, const Seperator *sep)
{
	Mob *target = c->GetTarget();

	if (!target) {
		c->Message(0, "Must target an NPC.");
		return;
	}

	if (!target->IsNPC()) {
		c->Message(0, "%s is not an NPC.",  target->GetName());
		return;
	}

	target->CastToNPC()->AISpellsList(c);

	return;
}

void command_raidloot(Client *c, const Seperator *sep)
{
	if(!sep->arg[1][0]) {
		c->Message(0, "Usage: #raidloot [LEADER/GROUPLEADER/SELECTED/ALL]");
		return;
	}

	Raid *r = c->GetRaid();
	if(r)
	{
		for(int x = 0; x < 72; ++x)
		{
			if(r->members[x].member == c)
			{
				if(r->members[x].IsRaidLeader == 0)
				{
					c->Message(0, "You must be the raid leader to use this command.");
				}
				else
				{
					break;
				}
			}
		}

		if(strcasecmp(sep->arg[1], "LEADER") == 0)
		{
			c->Message(15, "Loot type changed to: 1");
			r->ChangeLootType(1);
		}
		else if(strcasecmp(sep->arg[1], "GROUPLEADER") == 0)
		{
			c->Message(15, "Loot type changed to: 2");
			r->ChangeLootType(2);
		}
		else if(strcasecmp(sep->arg[1], "SELECTED") == 0)
		{
			c->Message(15, "Loot type changed to: 3");
			r->ChangeLootType(3);
		}
		else if(strcasecmp(sep->arg[1], "ALL") == 0)
		{
			c->Message(15, "Loot type changed to: 4");
			r->ChangeLootType(4);
		}
		else
		{
			c->Message(0, "Usage: #raidloot [LEADER/GROUPLEADER/SELECTED/ALL]");
		}
	}
	else
	{
		c->Message(0, "You must be in a raid to use that command.");
	}
}

void command_emoteview(Client *c, const Seperator *sep)
{
	if(!c->GetTarget() || !c->GetTarget()->IsNPC())
	{
		c->Message(0, "You must target a NPC to view their emotes.");
		return;
	}

	if(c->GetTarget() && c->GetTarget()->IsNPC())
	{
		int count=0;
		int emoteid = c->GetTarget()->CastToNPC()->GetEmoteID();

		LinkedListIterator<NPC_Emote_Struct*> iterator(zone->NPCEmoteList);
		iterator.Reset();
		while(iterator.MoreElements())
		{
			NPC_Emote_Struct* nes = iterator.GetData();
			if(emoteid == nes->emoteid)
			{
				c->Message(0, "EmoteID: %i Event: %i Type: %i Text: %s",  nes->emoteid, nes->event_, nes->type, nes->text);
				count++;
			}
			iterator.Advance();
		}
		if (count == 0)
			c->Message(0, "No emotes found.");
		else
			c->Message(0, "%i emote(s) found",  count);
	}
}

void command_emotesearch(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0)
		c->Message(0, "Usage: #emotesearch [search string or emoteid]");
	else
	{
		const char *search_criteria=sep->argplus[1];
		int count=0;

		if (Seperator::IsNumber(search_criteria))
		{
			uint16 emoteid = atoi(search_criteria);
			LinkedListIterator<NPC_Emote_Struct*> iterator(zone->NPCEmoteList);
			iterator.Reset();
			while(iterator.MoreElements())
			{
			NPC_Emote_Struct* nes = iterator.GetData();
				if(emoteid == nes->emoteid)
				{
					c->Message(0, "EmoteID: %i Event: %i Type: %i Text: %s",  nes->emoteid, nes->event_, nes->type, nes->text);
					count++;
				}
				iterator.Advance();
			}
			if (count == 0)
				c->Message(0, "No emotes found.");
			else
				c->Message(0, "%i emote(s) found",  count);
		}
		else
		{
			char sText[64];
			char sCriteria[515];
			strn0cpy(sCriteria, search_criteria, sizeof(sCriteria));
			strupr(sCriteria);
			char* pdest;

			LinkedListIterator<NPC_Emote_Struct*> iterator(zone->NPCEmoteList);
			iterator.Reset();
			while(iterator.MoreElements())
			{
			NPC_Emote_Struct* nes = iterator.GetData();
			strn0cpy(sText, nes->text, sizeof(sText));
			strupr(sText);
			pdest = strstr(sText, sCriteria);
				if (pdest != nullptr)
				{
					c->Message(0, "EmoteID: %i Event: %i Type: %i Text: %s",  nes->emoteid, nes->event_, nes->type, nes->text);
					count++;
				}
				if (count == 50)
					break;

				iterator.Advance();
			}
			if (count == 50)
				c->Message(0, "50 emotes shown...too many results.");
			else
				c->Message(0, "%i emote(s) found",  count);
		}
	}
}

void command_reloademote(Client *c, const Seperator *sep)
{
	zone->NPCEmoteList.Clear();
	zone->LoadNPCEmotes(&zone->NPCEmoteList);
	c->Message(0, "NPC emotes reloaded.");
}

void command_globalview(Client *c, const Seperator *sep)
{
	NPC * npcmob = nullptr;

	if(c->GetTarget() && c->GetTarget()->IsNPC())
	{
		npcmob = c->GetTarget()->CastToNPC();
		QGlobalCache *npc_c = nullptr;
		QGlobalCache *char_c = nullptr;
		QGlobalCache *zone_c = nullptr;

		if(npcmob)
			npc_c = npcmob->GetQGlobals();

		char_c = c->GetQGlobals();
		zone_c = zone->GetQGlobals();

		std::list<QGlobal> globalMap;
		uint32 ntype = 0;

		if(npcmob)
			ntype = npcmob->GetNPCTypeID();

		if(npc_c)
		{
			QGlobalCache::Combine(globalMap, npc_c->GetBucket(), ntype, c->CharacterID(), zone->GetZoneID());
		}

		if(char_c)
		{
			QGlobalCache::Combine(globalMap, char_c->GetBucket(), ntype, c->CharacterID(), zone->GetZoneID());
		}

		if(zone_c)
		{
			QGlobalCache::Combine(globalMap, zone_c->GetBucket(), ntype, c->CharacterID(), zone->GetZoneID());
		}

		auto iter = globalMap.begin();
		uint32 gcount = 0;

		c->Message(0, "Name, Value");
		while(iter != globalMap.end())
		{
			c->Message(0, "%s %s",  (*iter).name.c_str(), (*iter).value.c_str());
			++iter;
			++gcount;
		}
		c->Message(0, "%u globals loaded.",  gcount);
	}
	else
	{
		QGlobalCache *char_c = nullptr;
		QGlobalCache *zone_c = nullptr;

		char_c = c->GetQGlobals();
		zone_c = zone->GetQGlobals();

		std::list<QGlobal> globalMap;
		uint32 ntype = 0;

		if(char_c)
		{
			QGlobalCache::Combine(globalMap, char_c->GetBucket(), ntype, c->CharacterID(), zone->GetZoneID());
		}

		if(zone_c)
		{
			QGlobalCache::Combine(globalMap, zone_c->GetBucket(), ntype, c->CharacterID(), zone->GetZoneID());
		}

		auto iter = globalMap.begin();
		uint32 gcount = 0;

		c->Message(0, "Name, Value");
		while(iter != globalMap.end())
		{
			c->Message(0, "%s %s",  (*iter).name.c_str(), (*iter).value.c_str());
			++iter;
			++gcount;
		}
		c->Message(0, "%u globals loaded.",  gcount);
	}
}

void command_distance(Client *c, const Seperator *sep) {
	if(c && c->GetTarget()) {
		Mob* target = c->GetTarget();

		c->Message(0, "Your target, %s, is %1.1f units from you.",  c->GetTarget()->GetName(), Distance(c->GetPosition(), target->GetPosition()));
	}
}

void command_cvs(Client *c, const Seperator *sep)
{
	if(c)
	{
		auto pack =
		    new ServerPacket(ServerOP_ClientVersionSummary, sizeof(ServerRequestClientVersionSummary_Struct));

		ServerRequestClientVersionSummary_Struct *srcvss = (ServerRequestClientVersionSummary_Struct*)pack->pBuffer;

		strn0cpy(srcvss->Name, c->GetName(), sizeof(srcvss->Name));

		worldserver.SendPacket(pack);

		safe_delete(pack);

	}
}

void command_max_all_skills(Client *c, const Seperator *sep)
{
	if(c)
	{
		for (int i = 0; i <= EQEmu::skills::HIGHEST_SKILL; ++i)
		{
			if (i >= EQEmu::skills::SkillSpecializeAbjure && i <= EQEmu::skills::SkillSpecializeEvocation)
			{
				c->SetSkill((EQEmu::skills::SkillType)i, 50);
			}
			else
			{
				int max_skill_level = database.GetSkillCap(c->GetClass(), (EQEmu::skills::SkillType)i, c->GetLevel());
				c->SetSkill((EQEmu::skills::SkillType)i, max_skill_level);
			}
		}
	}
}

void command_showbonusstats(Client *c, const Seperator *sep)
{
	if (c->GetTarget() == 0)
		c->Message(0, "ERROR: No target!");
	else if (!c->GetTarget()->IsMob() && !c->GetTarget()->IsClient())
		c->Message(0, "ERROR: Target is not a Mob or Player!");
	else {
		bool bAll = false;
		if(sep->arg[1][0] == '\0' || strcasecmp(sep->arg[1], "all") == 0)
			bAll = true;
		if (bAll || (strcasecmp(sep->arg[1], "item")==0)) {
			c->Message(0, "Target Item Bonuses:");
			c->Message(0, "  Accuracy: %i%%   Divine Save: %i%%", c->GetTarget()->GetItemBonuses().Accuracy, c->GetTarget()->GetItemBonuses().DivineSaveChance);
			c->Message(0, "  Flurry: %i%%     HitChance: %i%%", c->GetTarget()->GetItemBonuses().FlurryChance, c->GetTarget()->GetItemBonuses().HitChance / 15);
		}
		if (bAll || (strcasecmp(sep->arg[1], "spell")==0)) {
			c->Message(0, "  Target Spell Bonuses:");
			c->Message(0, "  Accuracy: %i%%   Divine Save: %i%%", c->GetTarget()->GetSpellBonuses().Accuracy, c->GetTarget()->GetSpellBonuses().DivineSaveChance);
			c->Message(0, "  Flurry: %i%%     HitChance: %i%% ", c->GetTarget()->GetSpellBonuses().FlurryChance, c->GetTarget()->GetSpellBonuses().HitChance / 15);
		}
		c->Message(0, "  Effective Casting Level: %i", c->GetTarget()->GetCasterLevel(0));
	}
}

void command_reloadallrules(Client *c, const Seperator *sep)
{
	if(c)
	{
		auto pack = new ServerPacket(ServerOP_ReloadRules, 0);
		worldserver.SendPacket(pack);
		c->Message(13, "Successfully sent the packet to world to reload rules globally. (including world)");
		safe_delete(pack);

	}
}

void command_reloadworldrules(Client *c, const Seperator *sep)
{
	if(c)
	{
		auto pack = new ServerPacket(ServerOP_ReloadRulesWorld, 0);
		worldserver.SendPacket(pack);
		c->Message(13, "Successfully sent the packet to world to reload rules. (only world)");
		safe_delete(pack);
	}
}

void command_camerashake(Client *c, const Seperator *sep)
{
	if(c)
	{
		if(sep->arg[1][0] && sep->arg[2][0])
		{
			auto pack = new ServerPacket(ServerOP_CameraShake, sizeof(ServerCameraShake_Struct));
			ServerCameraShake_Struct* scss = (ServerCameraShake_Struct*) pack->pBuffer;
			scss->duration = atoi(sep->arg[1]);
			scss->intensity = atoi(sep->arg[2]);
			worldserver.SendPacket(pack);
			c->Message(13, "Successfully sent the packet to world! Shake it, world, shake it!");
			safe_delete(pack);
		}
		else {
			c->Message(13, "Usage -- #camerashake [duration], [intensity [1-10])");
		}
	}
	return;
}

void command_disarmtrap(Client *c, const Seperator *sep)
{
	Mob *target = c->GetTarget();

	if(!target)
	{
		c->Message(13, "You must have a target.");
		return;
	}

	if(target->IsNPC())
	{
		if (c->HasSkill(EQEmu::skills::SkillDisarmTraps))
		{
			if(DistanceSquaredNoZ(c->GetPosition(), target->GetPosition()) > RuleI(Adventure, LDoNTrapDistanceUse))
			{
				c->Message(13, "%s is too far away.",  target->GetCleanName());
				return;
			}
			c->HandleLDoNDisarm(target->CastToNPC(), c->GetSkill(EQEmu::skills::SkillDisarmTraps), LDoNTypeMechanical);
		}
		else
			c->Message(13, "You do not have the disarm trap skill.");
	}
}

void command_sensetrap(Client *c, const Seperator *sep)
{
	Mob * target = c->GetTarget();
	if(!target)
	{
		c->Message(13, "You must have a target.");
		return;
	}

	if(target->IsNPC())
	{
		if (c->HasSkill(EQEmu::skills::SkillSenseTraps))
		{
			if(DistanceSquaredNoZ(c->GetPosition(), target->GetPosition()) > RuleI(Adventure, LDoNTrapDistanceUse))
			{
				c->Message(13, "%s is too far away.",  target->GetCleanName());
				return;
			}
			c->HandleLDoNSenseTraps(target->CastToNPC(), c->GetSkill(EQEmu::skills::SkillSenseTraps), LDoNTypeMechanical);
		}
		else
			c->Message(13, "You do not have the sense traps skill.");
	}
}

void command_picklock(Client *c, const Seperator *sep)
{
	Mob * target = c->GetTarget();
	if(!target)
	{
		c->Message(13, "You must have a target.");
		return;
	}

	if(target->IsNPC())
	{
		if (c->HasSkill(EQEmu::skills::SkillPickLock))
		{
			if(DistanceSquaredNoZ(c->GetPosition(), target->GetPosition()) > RuleI(Adventure, LDoNTrapDistanceUse))
			{
				c->Message(13, "%s is too far away.",  target->GetCleanName());
				return;
			}
			c->HandleLDoNPickLock(target->CastToNPC(), c->GetSkill(EQEmu::skills::SkillPickLock), LDoNTypeMechanical);
		}
		else
			c->Message(13, "You do not have the pick locks skill.");
	}
}

void command_mysql(Client *c, const Seperator *sep)
{
	if(!sep->arg[1][0] || !sep->arg[2][0]) {
		c->Message(0, "Usage: #mysql query \"Query here\"");
		return;
	}

	if (strcasecmp(sep->arg[1], "help") == 0) {
		c->Message(0, "MYSQL In-Game CLI Interface:");
		c->Message(0, "Example: #mysql query \"Query goes here quoted\" -s -h");
		c->Message(0, "To use 'like \"%%something%%\" replace the %% with #");
		c->Message(0, "Example: #mysql query \"select * from table where name like \"#something#\"");
		c->Message(0, "-s - Spaces select entries apart");
		c->Message(0, "-h - Colors every other select result");
		return;
	}

	if (strcasecmp(sep->arg[1], "query") == 0) {
		///Parse switches here
		int argnum = 3;
		bool optionS = false;
		bool optionH = false;
		while(sep->arg[argnum] && strlen(sep->arg[argnum]) > 1){
			switch(sep->arg[argnum][1]){
				case 's': optionS = true; break;
				case 'h': optionH = true; break;
				default:
                    c->Message(15, "%s, there is no option '%c'",  c->GetName(), sep->arg[argnum][1]);
                    return;
			}
			++argnum;
		}

        int highlightTextIndex = 0;
        std::string query(sep->arg[2]);
        //swap # for % so like queries can work
        std::replace(query.begin(), query.end(), '#', '%');
        auto results = database.QueryDatabase(query);
        if (!results.Success()) {
            return;
        }

        //Using sep->arg[2] again, replace # with %% so it doesn't screw up when sent through vsnprintf in Message
        query = sep->arg[2];
        int pos = query.find('#');
        while(pos != std::string::npos) {
            query.erase(pos,1);
            query.insert(pos, "%%");
            pos = query.find('#');
        }
        c->Message(15, "---Running query: '%s'",  query.c_str());

        for (auto row = results.begin(); row != results.end(); ++row) {
            std::stringstream lineText;
            std::vector<std::string> lineVec;
            for(int i = 0; i < results.RowCount(); i++) {
                //split lines that could overflow the buffer in Client::Message and get cut off
                //This will crash MQ2 @ 4000 since their internal buffer is only 2048.
                //Reducing it to 2000 fixes that but splits more results from tables with a lot of columns.
                if(lineText.str().length() > 4000) {
                    lineVec.push_back(lineText.str());
                    lineText.str("");
                }
                lineText << results.FieldName(i) << ":" << "[" << (row[i] ? row[i] : "nullptr") << "] ";
            }

            lineVec.push_back(lineText.str());

            if(optionS) //This provides spacing for the space switch
                c->Message(0, " ");
            if(optionH) //This option will highlight every other row
                highlightTextIndex = 1 - highlightTextIndex;

            for(int lineNum = 0; lineNum < lineVec.size(); ++lineNum)
                    c->Message(highlightTextIndex, lineVec[lineNum].c_str());
        }
    }
}

void command_xtargets(Client *c, const Seperator *sep)
{
	Client *t;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t = c->GetTarget()->CastToClient();
	else
		t = c;

	if(sep->arg[1][0])
	{
		uint8 NewMax = atoi(sep->arg[1]);

		if((NewMax < 5) || (NewMax > XTARGET_HARDCAP))
		{
			c->Message(13, "Number of XTargets must be between 5 and %i",  XTARGET_HARDCAP);
			return;
		}
		t->SetMaxXTargets(NewMax);
		c->Message(0, "Max number of XTargets set to %i",  NewMax);
	}
	else
		t->ShowXTargets(c);
}

void command_zopp(Client *c, const Seperator *sep)
{ // - Owner only command..non-targetable to eliminate malicious or mischievious activities.
	if (!c)
		return;
	else if (sep->argnum < 3 || sep->argnum > 4)
		c->Message(0, "Usage: #zopp [trade/summon] [slot id] [item id] [*charges]");
	else if (!strcasecmp(sep->arg[1], "trade") == 0 && !strcasecmp(sep->arg[1], "t") == 0 && !strcasecmp(sep->arg[1], "summon") == 0 && !strcasecmp(sep->arg[1], "s") == 0)
		c->Message(0, "Usage: #zopp [trade/summon] [slot id] [item id] [*charges]");
	else if (!sep->IsNumber(2) || !sep->IsNumber(3) || (sep->argnum == 4 && !sep->IsNumber(4)))
		c->Message(0, "Usage: #zopp [trade/summon] [slot id] [item id] [*charges]");
	else {
		ItemPacketType packettype;

		if (strcasecmp(sep->arg[1], "trade") == 0 || strcasecmp(sep->arg[1], "t") == 0) {
			packettype = ItemPacketTrade;
		}
		else {
			packettype = ItemPacketLimbo;
		}

		int16 slotid = atoi(sep->arg[2]);
		uint32 itemid = atoi(sep->arg[3]);
		int16 charges = sep->argnum == 4 ? atoi(sep->arg[4]) : 1; // defaults to 1 charge if not specified

		const EQEmu::ItemData* FakeItem = database.GetItem(itemid);

		if (!FakeItem) {
			c->Message(13, "Error: Item [%u] is not a valid item id.",  itemid);
			return;
		}

		int16 item_status = 0;
		const EQEmu::ItemData* item = database.GetItem(itemid);
		if(item) {
			item_status = static_cast<int16>(item->MinStatus);
		}
		if (item_status > c->Admin()) {
			c->Message(13, "Error: Insufficient status to use this command.");
			return;
		}

		if (charges < 0 || charges > FakeItem->StackSize) {
			c->Message(13, "Warning: The specified charge count does not meet expected criteria!");
			c->Message(0, "Processing request..results may cause unpredictable behavior.");
		}

		EQEmu::ItemInstance* FakeItemInst = database.CreateItem(FakeItem, charges);
		c->SendItemPacket(slotid, FakeItemInst, packettype);
		c->Message(0, "Sending zephyr op packet to client - [%s] %s (%u) with %i %s to slot %i.",
			   packettype == ItemPacketTrade ? "Trade" : "Summon",  FakeItem->Name, itemid, charges,
			   std::abs(charges == 1) ? "charge" : "charges",  slotid);
		safe_delete(FakeItemInst);
	}
}

void command_augmentitem(Client *c, const Seperator *sep)
{
	if (!c)
		return;

	auto in_augment = new AugmentItem_Struct[sizeof(AugmentItem_Struct)];
	in_augment->container_slot = 1000; // <watch>
	in_augment->augment_slot = -1;
	if (c->GetTradeskillObject() != nullptr)
		Object::HandleAugmentation(c, in_augment, c->GetTradeskillObject());
		safe_delete_array(in_augment);
}

void command_questerrors(Client *c, const Seperator *sep)
{
	std::list<std::string> err;
	parse->GetErrors(err);
	c->Message(0, "Current Quest Errors:");

	auto iter = err.begin();
	int i = 0;
	while(iter != err.end()) {
		if(i >= 30) {
			c->Message(0, "Maximum of 30 Errors shown...");
			break;
		}

		c->Message(0, iter->c_str());
		++i;
		++iter;
	}
}

void command_enablerecipe(Client *c, const Seperator *sep)
{
	uint32 recipe_id = 0;
	bool success = false;
	if (c) {
		if (sep->argnum == 1) {
			recipe_id = atoi(sep->arg[1]);
		}
		else {
			c->Message(0, "Invalid number of arguments.\nUsage: #enablerecipe recipe_id");
			return;
		}
		if (recipe_id > 0) {
			success = database.EnableRecipe(recipe_id);
			if (success) {
				c->Message(0, "Recipe enabled.");
			}
			else {
				c->Message(0, "Recipe not enabled.");
			}
		}
		else {
			c->Message(0, "Invalid recipe id.\nUsage: #enablerecipe recipe_id");
		}
	}
}

void command_disablerecipe(Client *c, const Seperator *sep)
{
	uint32 recipe_id = 0;
	bool success = false;
	if (c) {
		if (sep->argnum == 1) {
			recipe_id = atoi(sep->arg[1]);
		}
		else {
			c->Message(0, "Invalid number of arguments.\nUsage: #disablerecipe recipe_id");
			return;
		}
		if (recipe_id > 0) {
			success = database.DisableRecipe(recipe_id);
			if (success) {
				c->Message(0, "Recipe disabled.");
			}
			else {
				c->Message(0, "Recipe not disabled.");
			}
		}
		else {
			c->Message(0, "Invalid recipe id.\nUsage: #disablerecipe recipe_id");
		}
	}
}

void command_npctype_cache(Client *c, const Seperator *sep)
{
	if (sep->argnum > 0) {
		for (int i = 0; i < sep->argnum; ++i) {
			if (strcasecmp(sep->arg[i + 1], "all") == 0) {
				c->Message(0, "Clearing all npc types from the cache.");
				zone->ClearNPCTypeCache(-1);
			}
			else {
				int id = atoi(sep->arg[i + 1]);
				if (id > 0) {
					c->Message(0, "Clearing npc type %d from the cache.",  id);
					zone->ClearNPCTypeCache(id);
					return;
				}
			}
		}
	}
	else {
		c->Message(0, "Usage:");
		c->Message(0, "#npctype_cache [npctype_id] ...");
		c->Message(0, "#npctype_cache all");
	}
}

void command_merchantopenshop(Client *c, const Seperator *sep)
{
	Mob *merchant = c->GetTarget();
	if (!merchant || merchant->GetClass() != MERCHANT) {
		c->Message(0, "You must target a merchant to open their shop.");
		return;
	}

	merchant->CastToNPC()->MerchantOpenShop();
}

void command_merchantcloseshop(Client *c, const Seperator *sep)
{
	Mob *merchant = c->GetTarget();
	if (!merchant || merchant->GetClass() != MERCHANT) {
		c->Message(0, "You must target a merchant to close their shop.");
		return;
	}

	merchant->CastToNPC()->MerchantCloseShop();
}

void command_shownumhits(Client *c, const Seperator *sep)
{
	c->ShowNumHits();
	return;
}

void command_tune(Client *c, const Seperator *sep)
{
	//Work in progress - Kayen

	if(sep->arg[1][0] == '\0' || !strcasecmp(sep->arg[1], "help")) {
		c->Message(0, "Syntax: #tune [subcommand].");
		c->Message(0, "-- Tune System Commands --");
		c->Message(0, "-- Usage: Returning recommended combat statistical values based on a desired outcome.");
		c->Message(0, "-- Note: If targeted mob does not have a target (ie not engaged in combat), YOU will be considered the target.");
		c->Message(0, "-- Warning: The calculations done in this process are intense and can potentially cause zone crashes depending on parameters set, use with caution!");
		c->Message(0, "-- Below are OPTIONAL parameters.");
		c->Message(0, "-- Note: [interval] Determines how fast the stat being checked increases/decreases till it finds the best result. Default [ATK/AC 50][Acc/Avoid 10] ");
		c->Message(0, "-- Note: [loop_max] Determines how many iterations are done to increases/decreases the stat till it finds the best result. Default [ATK/AC 100][Acc/Avoid 1000]");
		c->Message(0, "-- Note: [Stat Override] Will override that stat on mob being checkd with the specified value. Default=0");
		c->Message(0, "-- Note: [Info Level] How much statistical detail is displayed[0 - 3]. Default=0 ");
		c->Message(0, "-- Note: Results are only approximations usually accurate to +/- 2 intervals.");

		c->Message(0, "... ");
		c->Message(0, "...### Category A ### Target = ATTACKER ### YOU or Target's Target = DEFENDER ###");
		c->Message(0, "...### Category B ### Target = DEFENDER ### YOU or Target's Target = ATTACKER ###");
		c->Message(0, "... ");
		c->Message(0, "...#Returns recommended ATK adjustment +/- on ATTACKER that will result in an average mitigation pct on DEFENDER. ");
		c->Message(0, "...tune FindATK [A/B] [pct mitigation] [interval][loop_max][AC Overwride][Info Level]");
		c->Message(0, "... ");
		c->Message(0, "...#Returns recommended AC adjustment +/- on DEFENDER for an average mitigation pct from ATTACKER. ");
		c->Message(0, "...tune FindAC [A/B] [pct mitigation] [interval][loop_max][ATK Overwride][Info Level] ");
		c->Message(0, "... ");
		c->Message(0, "...#Returns recommended Accuracy adjustment +/- on ATTACKER that will result in a hit chance pct on DEFENDER. ");
		c->Message(0, "...tune FindAccuracy [A/B] [hit chance] [interval][loop_max][Avoidance Overwride][Info Level]");
		c->Message(0, "... ");
		c->Message(0, "...#Returns recommended Avoidance adjustment +/- on DEFENDER for in a hit chance pct from ATTACKER. ");
		c->Message(0, "...tune FindAvoidance [A/B] [pct mitigation] [interval][loop_max][Accuracy Overwride][Info Level] ");

		return;
	}
	//Default is category A for attacker/defender settings, which then are swapped under category B.
	Mob* defender = c;
	Mob* attacker = c->GetTarget();

	if (!attacker)
	{
		c->Message(0, "#Tune - Error no target selected. [#Tune help]");
		return;
	}

	Mob* ttarget = attacker->GetTarget();

	if (ttarget)
		defender = ttarget;

	if(!strcasecmp(sep->arg[1], "FindATK"))
	{
		float pct_mitigation = atof(sep->arg[3]);
		int interval = atoi(sep->arg[4]);
		int max_loop = atoi(sep->arg[5]);
		int ac_override = atoi(sep->arg[6]);
		int info_level = atoi(sep->arg[7]);

		if (!pct_mitigation)
		{
			c->Message(13, "#Tune - Error must enter the desired percent mitigation on defender. Ie. Defender to mitigate on average 20 pct of max damage.");
			return;
		}

		if (!interval)
			interval = 50;
		if (!max_loop)
			max_loop = 100;
		if(!ac_override)
			ac_override = 0;
		if (!info_level)
			info_level = 1;

		if(!strcasecmp(sep->arg[2], "A"))
			c->Tune_FindATKByPctMitigation(defender, attacker, pct_mitigation, interval, max_loop,ac_override,info_level);
		else if(!strcasecmp(sep->arg[2], "B"))
			c->Tune_FindATKByPctMitigation(attacker,defender, pct_mitigation, interval, max_loop,ac_override,info_level);
		else {
			c->Message(0, "#Tune - Error no category selcted. [#Tune help]");
			c->Message(0, "Usage #tune FindATK [A/B] [pct mitigation] [interval][loop_max][AC Overwride][Info Level] ");
			c->Message(0, "Example #tune FindATK A 60");
		}
		return;
	}

	if(!strcasecmp(sep->arg[1], "FindAC"))
	{
		float pct_mitigation = atof(sep->arg[3]);
		int interval = atoi(sep->arg[4]);
		int max_loop = atoi(sep->arg[5]);
		int atk_override = atoi(sep->arg[6]);
		int info_level = atoi(sep->arg[7]);

		if (!pct_mitigation)
		{
			c->Message(13, "#Tune - Error must enter the desired percent mitigation on defender. Ie. Defender to mitigate on average 20 pct of max damage.");
			return;
		}

		if (!interval)
			interval = 50;
		if (!max_loop)
			max_loop = 100;
		if(!atk_override)
			atk_override = 0;
		if (!info_level)
			info_level = 1;

		if(!strcasecmp(sep->arg[2], "A"))
			c->Tune_FindACByPctMitigation(defender, attacker, pct_mitigation, interval, max_loop,atk_override,info_level);
		else if(!strcasecmp(sep->arg[2], "B"))
			c->Tune_FindACByPctMitigation(attacker, defender, pct_mitigation, interval, max_loop,atk_override,info_level);
		else {
			c->Message(0, "#Tune - Error no category selcted. [#Tune help]");
			c->Message(0, "Usage #tune FindAC [A/B] [pct mitigation] [interval][loop_max][ATK Overwride][Info Level] ");
			c->Message(0, "Example #tune FindAC A 60");
		}

		return;
	}

	if(!strcasecmp(sep->arg[1], "FindAccuracy"))
	{
		float hit_chance = atof(sep->arg[3]);
		int interval = atoi(sep->arg[4]);
		int max_loop = atoi(sep->arg[5]);
		int avoid_override = atoi(sep->arg[6]);
		int info_level = atoi(sep->arg[7]);

		if (!hit_chance)
		{
			c->Message(10, "#Tune - Error must enter the desired percent mitigation on defender. Ie. Defender to mitigate on average 20 pct of max damage.");
			return;
		}

		if (!interval)
			interval = 10;
		if (!max_loop)
			max_loop = 1000;
		if(!avoid_override)
			avoid_override = 0;
		if (!info_level)
			info_level = 1;

		if (hit_chance > RuleR(Combat,MaxChancetoHit) || hit_chance < RuleR(Combat,MinChancetoHit))
		{
			c->Message(10, "#Tune - Error hit chance out of bounds. [Max %.2f Min .2f]",  RuleR(Combat,MaxChancetoHit),RuleR(Combat,MinChancetoHit));
			return;
		}

		if(!strcasecmp(sep->arg[2], "A"))
			c->Tune_FindAccuaryByHitChance(defender, attacker, hit_chance, interval, max_loop,avoid_override,info_level);
		else if(!strcasecmp(sep->arg[2], "B"))
			c->Tune_FindAccuaryByHitChance(attacker, defender, hit_chance, interval, max_loop,avoid_override,info_level);
		else {
			c->Message(0, "#Tune - Error no category selcted. [#Tune help]");
			c->Message(0, "Usage #tune FindAcccuracy [A/B] [hit chance] [interval][loop_max][Avoidance Overwride][Info Level]");
			c->Message(0, "Exampled #tune FindAccuracy B 30");
		}

		return;
	}

	if(!strcasecmp(sep->arg[1], "FindAvoidance"))
	{
		float hit_chance = atof(sep->arg[3]);
		int interval = atoi(sep->arg[4]);
		int max_loop = atoi(sep->arg[5]);
		int acc_override = atoi(sep->arg[6]);
		int info_level = atoi(sep->arg[7]);

		if (!hit_chance)
		{
			c->Message(0, "#Tune - Error must enter the desired hit chance on defender. Ie. Defender to have hit chance of 40 pct.");
			return;
		}

		if (!interval)
			interval = 10;
		if (!max_loop)
			max_loop = 1000;
		if(!acc_override)
			acc_override = 0;
		if (!info_level)
			info_level = 1;

		if (hit_chance > RuleR(Combat,MaxChancetoHit) || hit_chance < RuleR(Combat,MinChancetoHit))
		{
			c->Message(10, "#Tune - Error hit chance out of bounds. [Max %.2f Min .2f]",  RuleR(Combat,MaxChancetoHit),RuleR(Combat,MinChancetoHit));
			return;
		}

		if(!strcasecmp(sep->arg[2], "A"))
			c->Tune_FindAvoidanceByHitChance(defender, attacker, hit_chance, interval, max_loop,acc_override, info_level);
		else if(!strcasecmp(sep->arg[2], "B"))
			c->Tune_FindAvoidanceByHitChance(attacker, defender, hit_chance, interval, max_loop,acc_override, info_level);
		else {
			c->Message(0, "#Tune - Error no category selcted. [#Tune help]");
			c->Message(0, "Usage #tune FindAvoidance [A/B] [hit chance] [interval][loop_max][Accuracy Overwride][Info Level]");
			c->Message(0, "Exampled #tune FindAvoidance B 30");
		}

		return;
	}


	return;
}

void command_logtest(Client *c, const Seperator *sep){
	clock_t t = std::clock(); /* Function timer start */
	if (sep->IsNumber(1)){
		uint32 i = 0;
		t = std::clock();
		for (i = 0; i < atoi(sep->arg[1]); i++){
			Log(Logs::General, Logs::Debug, "[%u] Test #2... Took %f seconds", i, ((float)(std::clock() - t)) / CLOCKS_PER_SEC);
		}

	}
}

void command_crashtest(Client *c, const Seperator *sep)
{
	c->Message(0, "Alright, now we get an GPF ;) ");
	char* gpf = 0;
	memcpy(gpf, "Ready to crash",  30);
}

void command_logs(Client *c, const Seperator *sep){
	int logs_set = 0;
	if (sep->argnum > 0) {
		/* #logs reload_all */
		if (strcasecmp(sep->arg[1], "reload_all") == 0){
			auto pack = new ServerPacket(ServerOP_ReloadLogs, 0);
			worldserver.SendPacket(pack);
			c->Message(13, "Successfully sent the packet to world to reload log settings from the database for all zones");
			safe_delete(pack);
		}
		/* #logs list_settings */
		if (strcasecmp(sep->arg[1], "list_settings") == 0 || (strcasecmp(sep->arg[1], "set") == 0 && strcasecmp(sep->arg[3], "") == 0)){
			c->Message(0, "[Category ID | console | file | gmsay | Category Description]");
			int redisplay_columns = 0;
			for (int i = 0; i < Logs::LogCategory::MaxCategoryID; i++){
				if (redisplay_columns == 10){
					c->Message(0, "[Category ID | console | file | gmsay | Category Description]");
					redisplay_columns = 0;
				}
				c->Message(0, StringFormat("--- %i | %u | %u | %u | %s",  i, LogSys.log_settings[i].log_to_console, LogSys.log_settings[i].log_to_file, LogSys.log_settings[i].log_to_gmsay, Logs::LogCategoryName[i]).c_str());
				redisplay_columns++;
			}
		}
		/* #logs set */
		if (strcasecmp(sep->arg[1], "set") == 0){
			if (strcasecmp(sep->arg[2], "console") == 0){
				LogSys.log_settings[atoi(sep->arg[3])].log_to_console = atoi(sep->arg[4]);
				logs_set = 1;
			}
			else if (strcasecmp(sep->arg[2], "file") == 0){
				LogSys.log_settings[atoi(sep->arg[3])].log_to_file = atoi(sep->arg[4]);
				logs_set = 1;
			}
			else if (strcasecmp(sep->arg[2], "gmsay") == 0){
				LogSys.log_settings[atoi(sep->arg[3])].log_to_gmsay = atoi(sep->arg[4]);
				logs_set = 1;
			}
			else{
				c->Message(0, "--- #logs set [console|file|gmsay] <category_id> <debug_level (1-3)> - Sets log settings during the lifetime of the zone");
				c->Message(0, "--- #logs set gmsay 20 1 - Would output Quest errors to gmsay");
			}
			if (logs_set == 1){
				c->Message(15, "Your Log Settings have been applied");
				c->Message(15, "Output Method: %s :: Debug Level: %i - Category: %s",  sep->arg[2], atoi(sep->arg[4]), Logs::LogCategoryName[atoi(sep->arg[3])]);
			}
			/* We use a general 'is_category_enabled' now, let's update when we update any output settings
				This is used in hot places of code to check if its enabled in any way before triggering logs
			*/
			if (atoi(sep->arg[4]) > 0){
				LogSys.log_settings[atoi(sep->arg[3])].is_category_enabled = 1;
			}
			else{
				LogSys.log_settings[atoi(sep->arg[3])].is_category_enabled = 0;
			}
		}
	}
	else {
		c->Message(0, "#logs usage:");
		c->Message(0, "--- #logs reload_all - Reload all settings in world and all zone processes with what is defined in the database");
		c->Message(0, "--- #logs list_settings - Shows current log settings and categories loaded into the current process' memory");
		c->Message(0, "--- #logs set [console|file|gmsay] <category_id> <debug_level (1-3)> - Sets log settings during the lifetime of the zone");
	}
}

void command_mysqltest(Client *c, const Seperator *sep)
{
	clock_t t = std::clock(); /* Function timer start */
	if (sep->IsNumber(1)){
		uint32 i = 0;
		t = std::clock();
		for (i = 0; i < atoi(sep->arg[1]); i++){
			std::string query = "SELECT * FROM `zone`";
			auto results = database.QueryDatabase(query);
		}
	}
	Log(Logs::General, Logs::Debug, "MySQL Test... Took %f seconds", ((float)(std::clock() - t)) / CLOCKS_PER_SEC);
}

void command_resetaa_timer(Client *c, const Seperator *sep) {
	Client *target = nullptr;
	if(!c->GetTarget() || !c->GetTarget()->IsClient()) {
		target = c;
	} else {
		target = c->GetTarget()->CastToClient();
	}

	if(sep->IsNumber(1))
	{
		int timer_id = atoi(sep->arg[1]);
		c->Message(0, "Reset of timer %i for %s", timer_id, c->GetName());
		c->ResetAlternateAdvancementTimer(timer_id);
	}
	else if(!strcasecmp(sep->arg[1], "all"))
	{
		c->Message(0, "Reset all timers for %s", c->GetName());
		c->ResetAlternateAdvancementTimers();
	}
	else
	{
		c->Message(0, "usage: #resetaa_timer [all | timer_id]");
	}
}

void command_reloadaa(Client *c, const Seperator *sep) {
	c->Message(0, "Reloading Alternate Advancement Data...");
	zone->LoadAlternateAdvancement();
	c->Message(0, "Alternate Advancement Data Reloaded");
	entity_list.SendAlternateAdvancementStats();
}

void command_hotfix(Client *c, const Seperator *sep) {
	std::string hotfix;
	database.GetVariable("hotfix_name", hotfix);

	std::string hotfix_name;
	if(!strcasecmp(hotfix.c_str(), "hotfix_")) {
		hotfix_name = "";
	} else {
		hotfix_name = "hotfix_";
	}

	c->Message(0, "Creating and applying hotfix");
	std::thread t1([c,hotfix_name]() {
#ifdef WIN32
		if(hotfix_name.length() > 0) {
			system(StringFormat("shared_memory -hotfix=%s", hotfix_name.c_str()).c_str());
		} else {
			system(StringFormat("shared_memory").c_str());
		}
#else
		if(hotfix_name.length() > 0) {
			system(StringFormat("./shared_memory -hotfix=%s", hotfix_name.c_str()).c_str());
		}
		else {
			system(StringFormat("./shared_memory").c_str());
		}
#endif
		database.SetVariable("hotfix_name", hotfix_name);

		ServerPacket pack(ServerOP_ChangeSharedMem, hotfix_name.length() + 1);
		if(hotfix_name.length() > 0) {
			strcpy((char*)pack.pBuffer, hotfix_name.c_str());
		}
		worldserver.SendPacket(&pack);

		c->Message(0, "Hotfix applied");
	});

	t1.detach();
}

void command_load_shared_memory(Client *c, const Seperator *sep) {
	std::string hotfix;
	database.GetVariable("hotfix_name", hotfix);

	std::string hotfix_name;
	if(strcasecmp(hotfix.c_str(), sep->arg[1]) == 0) {
		c->Message(0, "Cannot attempt to load this shared memory segment as it is already loaded.");
		return;
	}

	hotfix_name = sep->arg[1];
	c->Message(0, "Loading shared memory segment %s", hotfix_name.c_str());
	std::thread t1([c,hotfix_name]() {
#ifdef WIN32
		if(hotfix_name.length() > 0) {
			system(StringFormat("shared_memory -hotfix=%s", hotfix_name.c_str()).c_str());
		} else {
			system(StringFormat("shared_memory").c_str());
		}
#else
		if(hotfix_name.length() > 0) {
			system(StringFormat("./shared_memory -hotfix=%s", hotfix_name.c_str()).c_str());
		}
		else {
			system(StringFormat("./shared_memory").c_str());
		}
#endif
		c->Message(0, "Shared memory segment finished loading.");
	});

	t1.detach();
}

void command_apply_shared_memory(Client *c, const Seperator *sep) {
	std::string hotfix;
	database.GetVariable("hotfix_name", hotfix);
	std::string hotfix_name = sep->arg[1];

	c->Message(0, "Applying shared memory segment %s", hotfix_name.c_str());
	database.SetVariable("hotfix_name", hotfix_name);

	ServerPacket pack(ServerOP_ChangeSharedMem, hotfix_name.length() + 1);
	if(hotfix_name.length() > 0) {
		strcpy((char*)pack.pBuffer, hotfix_name.c_str());
	}
	worldserver.SendPacket(&pack);
}

void command_reloadperlexportsettings(Client *c, const Seperator *sep)
{
	if (c)
	{
		auto pack = new ServerPacket(ServerOP_ReloadPerlExportSettings, 0);
		worldserver.SendPacket(pack);
		c->Message(13, "Successfully sent the packet to world to reload Perl Export settings");
		safe_delete(pack);

	}
}

void command_trapinfo(Client *c, const Seperator *sep)
{
	entity_list.GetTrapInfo(c);
}

void command_reloadtraps(Client *c, const Seperator *sep)
{
	entity_list.UpdateAllTraps(true, true);
	c->Message(CC_Default, "Traps reloaded for %s.", zone->GetShortName());
}

// All new code added to command.cpp should be BEFORE this comment line. Do no append code to this file below the BOTS code block.
#ifdef BOTS
#include "bot_command.h"
// Function delegate to support the command interface for Bots with the client.
void command_bot(Client *c, const Seperator *sep)
{
	std::string bot_message = sep->msg;
	if (bot_message.compare("#bot") == 0) {
		bot_message[0] = BOT_COMMAND_CHAR;
	}
	else {
		bot_message = bot_message.substr(bot_message.find_first_not_of("#bot"));
		bot_message[0] = BOT_COMMAND_CHAR;
	}
	
	if (bot_command_dispatch(c, bot_message.c_str()) == -2) {
		if (parse->PlayerHasQuestSub(EVENT_COMMAND)) {
			int i = parse->EventPlayer(EVENT_COMMAND, c, bot_message, 0);
			if (i == 0 && !RuleB(Chat, SuppressCommandErrors)) {
				c->Message(13, "Bot command '%s' not recognized.", bot_message.c_str());
			}
		}
		else {
			if (!RuleB(Chat, SuppressCommandErrors))
				c->Message(13, "Bot command '%s' not recognized.", bot_message.c_str());
		}
	}
}
#endif
