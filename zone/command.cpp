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
#include <fmt/format.h>

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
#include "../common/say_link.h"
#include "../common/eqemu_logsys.h"
#include "../common/profanity_manager.h"
#include "../common/net/eqstream.h"
#include "../common/repositories/dynamic_zones_repository.h"

#include "data_bucket.h"
#include "command.h"
#include "expedition.h"
#include "guild_mgr.h"
#include "map.h"
#include "qglobals.h"
#include "queryserv.h"
#include "quest_parser_collection.h"
#include "string_ids.h"
#include "titles.h"
#include "water_map.h"
#include "worldserver.h"
#include "fastmath.h"
#include "mob_movement_manager.h"
#include "npc_scale_manager.h"
#include "../common/content/world_content_service.h"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "../common/http/httplib.h"

extern QueryServ* QServ;
extern WorldServer worldserver;
extern TaskManager *task_manager;
extern FastMath g_Math;
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
	c->Message(Chat::Red, "Commands not available.");
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
		command_add("copycharacter", "[source_char_name] [dest_char_name] [dest_account_name] Copies character to destination account", 250, command_copycharacter) ||
		command_add("corpse", "- Manipulate corpses, use with no arguments for help", 50, command_corpse) ||
		command_add("corpsefix", "Attempts to bring corpses from underneath the ground within close proximity of the player", 0, command_corpsefix) ||
		command_add("crashtest", "- Crash the zoneserver", 255, command_crashtest) ||
		command_add("cvs", "- Summary of client versions currently online.", 200, command_cvs) ||
		command_add("damage", "[amount] - Damage your target", 100, command_damage) ||
		command_add("databuckets", "View|Delete [key] [limit]- View data buckets, limit 50 default or Delete databucket by key", 80, command_databuckets) ||
		command_add("date", "[yyyy] [mm] [dd] [HH] [MM] - Set EQ time", 90, command_date) ||
		command_add("dbspawn2", "[spawngroup] [respawn] [variance] - Spawn an NPC from a predefined row in the spawn2 table", 100, command_dbspawn2) ||
		command_add("delacct", "[accountname] - Delete an account", 150, command_delacct) ||
		command_add("deletegraveyard", "[zone name] - Deletes the graveyard for the specified zone.",  200, command_deletegraveyard) ||
		command_add("delpetition", "[petition number] - Delete a petition", 20, command_delpetition) ||
		command_add("depop", "- Depop your NPC target", 50, command_depop) ||
		command_add("depopzone", "- Depop the zone", 100, command_depopzone) ||
		command_add("devtools", "- Manages devtools", 200, command_devtools) ||
		command_add("details", "- Change the details of your target (Drakkin Only)", 80, command_details) ||
		command_add("disablerecipe",  "[recipe_id] - Disables a recipe using the recipe id.",  80, command_disablerecipe) ||
		command_add("disarmtrap",  "Analog for ldon disarm trap for the newer clients since we still don't have it working.", 80, command_disarmtrap) ||
		command_add("distance", "- Reports the distance between you and your target.",  80, command_distance) ||
		command_add("doanim", "[animnum] [type] - Send an EmoteAnim for you or your target", 50, command_doanim) ||
		command_add("dz", "Manage expeditions and dynamic zone instances", 80, command_dz) ||
		command_add("dzkickplayers", "Removes all players from current expedition. (/kickplayers alternative for pre-RoF clients)", 0, command_dzkickplayers) ||
		command_add("editmassrespawn", "[name-search] [second-value] - Mass (Zone wide) NPC respawn timer editing command", 100, command_editmassrespawn) ||
		command_add("emote", "['name'/'world'/'zone'] [type] [message] - Send an emote message", 80, command_emote) ||
		command_add("emotesearch", "Searches NPC Emotes", 80, command_emotesearch) ||
		command_add("emoteview", "Lists all NPC Emotes", 80, command_emoteview) ||
		command_add("enablerecipe",  "[recipe_id] - Enables a recipe using the recipe id.",  80, command_enablerecipe) ||
		command_add("endurance", "Restores you or your target's endurance.", 50, command_endurance) ||
		command_add("equipitem", "[slotid(0-21)] - Equip the item on your cursor into the specified slot", 50, command_equipitem) ||
		command_add("face", "- Change the face of your target", 80, command_face) ||
		command_add("faction", "[Find (criteria | all ) | Review (criteria | all) | Reset (id)] - Resets Player's Faction", 80, command_faction) ||
		command_add("findaliases", "[search criteria]- Searches for available command aliases, by alias or command", 0, command_findaliases) ||
		command_add("findclass", "[search criteria] - Search for a class", 50, command_findclass) ||
		command_add("findnpctype", "[search criteria] - Search database NPC types", 100, command_findnpctype) ||
		command_add("findrace", "[search criteria] - Search for a race", 50, command_findrace) ||
		command_add("findspell", "[search criteria] - Search for a spell", 50, command_findspell) ||
		command_add("findzone", "[search criteria] - Search database zones", 100, command_findzone) ||
		command_add("fixmob", "[race|gender|texture|helm|face|hair|haircolor|beard|beardcolor|heritage|tattoo|detail] [next|prev] - Manipulate appearance of your target", 80, command_fixmob) ||
		command_add("flag", "[status] [acctname] - Refresh your admin status, or set an account's admin status if arguments provided", 0, command_flag) ||
		command_add("flagedit", "- Edit zone flags on your target", 100, command_flagedit) ||
		command_add("flags", "- displays the flags of you or your target", 0, command_flags) ||
		command_add("flymode", "[0/1/2/3/4/5] - Set your or your player target's flymode to ground/flying/levitate/water/floating/levitate_running", 50, command_flymode) ||
		command_add("fov", "- Check wether you're behind or in your target's field of view", 80, command_fov) ||
		command_add("freeze", "- Freeze your target", 80, command_freeze) ||
		command_add("gassign", "[id] - Assign targetted NPC to predefined wandering grid id", 100, command_gassign) ||
		command_add("gearup", "Developer tool to quickly equip a character", 200, command_gearup) ||
		command_add("gender", "[0/1/2] - Change your or your target's gender to male/female/neuter", 50, command_gender) ||
		command_add("getplayerburiedcorpsecount", "- Get the target's total number of buried player corpses.",  100, command_getplayerburiedcorpsecount) ||
		command_add("getvariable", "[varname] - Get the value of a variable from the database", 200, command_getvariable) ||
		command_add("ginfo", "- get group info on target.", 20, command_ginfo) ||
		command_add("giveitem", "[itemid] [charges] - Summon an item onto your target's cursor. Charges are optional.", 200, command_giveitem) ||
		command_add("givemoney", "[pp] [gp] [sp] [cp] - Gives specified amount of money to the target player.", 200, command_givemoney) ||
		command_add("globalview", "Lists all qglobals in cache if you were to do a quest with this target.", 80, command_globalview) ||
		command_add("gm", "- Turn player target's or your GM flag on or off", 80, command_gm) ||
		command_add("gmspeed", "[on/off] - Turn GM speed hack on/off for you or your player target", 100, command_gmspeed) ||
		command_add("gmzone", "[zone_short_name] [zone_version=0] [identifier=gmzone] - Zones to a private GM instance", 100, command_gmzone) ||
		command_add("goto", "[playername] or [x y z] [h] - Teleport to the provided coordinates or to your target", 10, command_goto) ||
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
		command_add("invsnapshot", "- Manipulates inventory snapshots for your current target", 80, command_invsnapshot) ||
		command_add("invul", "[on/off] - Turn player target's or your invulnerable flag on or off", 80, command_invul) ||
		command_add("ipban", "[IP address] - Ban IP by character name", 200, command_ipban) ||
		command_add("iplookup", "[charname] - Look up IP address of charname", 200, command_iplookup) ||
		command_add("iteminfo", "- Get information about the item on your cursor", 10, command_iteminfo) ||
		command_add("itemsearch", "[search criteria] - Search for an item", 10, command_itemsearch) ||
		command_add("kick", "[charname] - Disconnect charname", 150, command_kick) ||
		command_add("kill", "- Kill your target", 100, command_kill) ||
		command_add("killallnpcs", " [npc_name] Kills all npcs by search name, leave blank for all attackable NPC's", 200, command_killallnpcs) ||
		command_add("lastname", "[new lastname] - Set your or your player target's lastname", 50, command_lastname) ||
		command_add("level", "[level] - Set your or your target's level", 10, command_level) ||
		command_add("listnpcs", "[name/range] - Search NPCs", 20, command_listnpcs) ||
		command_add("list", "[npcs|players|corpses|doors|objects] [search] - Search entities", 20, command_list) ||
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
		command_add("movement", "Various movement commands", 200, command_movement) ||
		command_add("myskills", "- Show details about your current skill levels", 0, command_myskills) ||
		command_add("mysqltest", "Akkadius MySQL Bench Test", 250, command_mysqltest) ||
		command_add("mysql", "Mysql CLI, see 'help' for options.", 250, command_mysql) ||
		command_add("mystats", "- Show details about you or your pet", 50, command_mystats) ||
		command_add("name", "[newname] - Rename your player target", 150, command_name) ||
		command_add("netstats", "- Gets the network stats for a stream.", 200, command_netstats) ||
		command_add("network", "- Admin commands for the udp network interface.", 250, command_network) ||
		command_add("npccast", "[targetname/entityid] [spellid] - Causes NPC target to cast spellid on targetname/entityid", 80, command_npccast) ||
		command_add("npcedit", "[column] [value] - Mega NPC editing command", 100, command_npcedit) ||
		command_add("npceditmass", "[name-search] [column] [value] - Mass (Zone wide) NPC data editing command", 100, command_npceditmass) ||
		command_add("npcemote", "[message] - Make your NPC target emote a message.", 150, command_npcemote) ||
		command_add("npcloot", "[show/money/add/remove] [itemid/all/money: pp gp sp cp] - Manipulate the loot an NPC is carrying", 80, command_npcloot) ||
		command_add("npcsay", "[message] - Make your NPC target say a message.", 150, command_npcsay) ||
		command_add("npcshout", "[message] - Make your NPC target shout a message.", 150, command_npcshout) ||
		command_add("npcspawn", "[create/add/update/remove/delete] - Manipulate spawn DB", 170, command_npcspawn) ||
		command_add("npcspecialattk", "[flagchar] [perm] - Set NPC special attack flags. Flags are E(nrage) F(lurry) R(ampage) S(ummon).", 80, command_npcspecialattk) ||
		command_add("npcstats", "- Show stats about target NPC", 80, command_npcstats) ||
		command_add("npctype_cache",  "[id] or all - Clears the npc type cache for either the id or all npcs.",  250, command_npctype_cache) ||
		command_add("npctypespawn", "[npctypeid] [factionid] - Spawn an NPC from the db", 10, command_npctypespawn) ||
		command_add("nudge", "- Nudge your target's current position by specific values", 80, command_nudge) ||
		command_add("nukebuffs", "- Strip all buffs on you or your target", 50, command_nukebuffs) ||
		command_add("nukeitem", "[itemid] - Remove itemid from your player target's inventory", 150, command_nukeitem) ||
		command_add("object", "List|Add|Edit|Move|Rotate|Copy|Save|Undo|Delete - Manipulate static and tradeskill objects within the zone", 100, command_object) ||
		command_add("oocmute", "[1/0] - Mutes OOC chat", 200, command_oocmute) ||
		command_add("opcode", "- opcode management", 250, command_opcode) ||

#ifdef PACKET_PROFILER
		command_add("packetprofile", "- Dump packet profile for target or self.", 250, command_packetprofile) ||
#endif

		command_add("path", "- view and edit pathing", 200, command_path) ||
		command_add("peekinv", "[equip/gen/cursor/poss/limbo/curlim/trib/bank/shbank/allbank/trade/world/all] - Print out contents of your player target's inventory", 100, command_peekinv) ||
		command_add("peqzone", "[zonename] - Go to specified zone, if you have > 75% health", 0, command_peqzone) ||
		command_add("permaclass", "[classnum] - Change your or your player target's class (target is disconnected)", 80, command_permaclass) ||
		command_add("permagender", "[gendernum] - Change your or your player target's gender (zone to take effect)", 80, command_permagender) ||
		command_add("permarace", "[racenum] - Change your or your player target's race (zone to take effect)", 80, command_permarace) ||
		command_add("petitioninfo", "[petition number] - Get info about a petition", 20, command_petitioninfo) ||
		command_add("pf", "- Display additional mob coordinate and wandering data", 0, command_pf) ||
		command_add("picklock",  "Analog for ldon pick lock for the newer clients since we still don't have it working.",  0, command_picklock) ||
		command_add("profanity", "Manage censored language.", 150, command_profanity) ||

#ifdef EQPROFILE
		command_add("profiledump", "- Dump profiling info to logs", 250, command_profiledump) ||
		command_add("profilereset", "- Reset profiling info", 250, command_profilereset) ||
#endif

		command_add("push", "Lets you do spell push", 150, command_push) ||
		command_add("proximity", "Shows NPC proximity", 150, command_proximity) ||
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
		command_add("reloadmerchants", nullptr, 255, command_reloadmerchants) ||
		command_add("reloadperlexportsettings", nullptr, 255, command_reloadperlexportsettings) ||
		command_add("reloadqst", " - Clear quest cache (any argument causes it to also stop all timers)", 150, command_reloadqst) ||
		command_add("reloadrulesworld", "Executes a reload of all rules in world specifically.", 80, command_reloadworldrules) ||
		command_add("reloadstatic", "- Reload Static Zone Data", 150, command_reloadstatic) ||
		command_add("reloadtraps", "- Repops all traps in the current zone.", 80, command_reloadtraps) ||
		command_add("reloadtitles", "- Reload player titles from the database",  150, command_reloadtitles) ||
		command_add("reloadworld", "[0|1] - Clear quest cache (0 - no repop, 1 - repop)", 255, command_reloadworld) ||
		command_add("reloadzps", "- Reload zone points from database", 150, command_reloadzps) ||
		command_add("repop", "[delay] - Repop the zone with optional delay", 100, command_repop) ||
		command_add("resetaa", "- Resets a Player's AA in their profile and refunds spent AA's to unspent, may disconnect player.", 200, command_resetaa) ||
		command_add("resetaa_timer", "Command to reset AA cooldown timers.", 200, command_resetaa_timer) ||
		command_add("revoke", "[charname] [1/0] - Makes charname unable to talk on OOC", 200, command_revoke) ||
		command_add("roambox", "Manages roambox settings for an NPC", 200, command_roambox) ||
		command_add("rules", "(subcommand) - Manage server rules", 250, command_rules) ||
		command_add("save", "- Force your player or player corpse target to be saved to the database", 50, command_save) ||
		command_add("scale", "- Handles npc scaling", 150, command_scale) ||
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
		command_add("shownpcgloballoot", "Show GlobalLoot entires on this npc", 50, command_shownpcgloballoot) ||
		command_add("showskills", "- Show the values of your or your player target's skills", 50, command_showskills) ||
		command_add("showspellslist", "Shows spell list of targeted NPC", 100, command_showspellslist) ||
		command_add("showstats", "- Show details about you or your target", 50, command_showstats) ||
		command_add("showzonegloballoot", "Show GlobalLoot entires on this zone", 50, command_showzonegloballoot) ||
		command_add("showzonepoints", "Show zone points for current zone", 50, command_showzonepoints) ||
		command_add("shutdown", "- Shut this zone process down", 150, command_shutdown) ||
		command_add("size", "[size] - Change size of you or your target", 50, command_size) ||
		command_add("spawn", "[name] [race] [level] [material] [hp] [gender] [class] [priweapon] [secweapon] [merchantid] - Spawn an NPC", 10, command_spawn) ||
		command_add("spawneditmass", "Mass editing spawn command", 150, command_spawneditmass) ||
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
		command_add("test", "Test command", 200, command_test) ||
		command_add("texture", "[texture] [helmtexture] - Change your or your target's appearance, use 255 to show equipment", 10, command_texture) ||
		command_add("time", "[HH] [MM] - Set EQ time", 90, command_time) ||
		command_add("timers", "- Display persistent timers for target", 200, command_timers) ||
		command_add("timezone", "[HH] [MM] - Set timezone. Minutes are optional", 90, command_timezone) ||
		command_add("title", "[text] [1 = create title table row] - Set your or your player target's title", 50, command_title) ||
		command_add("titlesuffix", "[text] [1 = create title table row] - Set your or your player target's title suffix", 50, command_titlesuffix) ||
		command_add("traindisc", "[level] - Trains all the disciplines usable by the target, up to level specified. (may freeze client for a few seconds)", 150, command_traindisc) ||
		command_add("trapinfo", "- Gets infomation about the traps currently spawned in the zone.", 81, command_trapinfo) ||
		command_add("tune",  "Calculate ideal statical values related to combat.",  100, command_tune) ||
		command_add("ucs", "- Attempts to reconnect to the UCS server", 0, command_ucs) ||
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
		command_add("viewzoneloot", "[item id] - Allows you to search a zone's loot for a specific item ID. (0 shows all loot in the zone)", 80, command_viewzoneloot) ||
		command_add("wc", "[wear slot] [material] - Sends an OP_WearChange for your target", 200, command_wc) ||
		command_add("weather", "[0/1/2/3] (Off/Rain/Snow/Manual) - Change the weather", 80, command_weather) ||
		command_add("who", "[search]", 20, command_who) ||
		command_add("worldshutdown", "- Shut down world and all zones", 200, command_worldshutdown) ||
		command_add("wp", "[add|delete] [grid_id] [pause] [waypoint_id] [-h] - Add or delete a waypoint by grid ID. (-h to use current heading)", 170, command_wp) ||
		command_add("wpadd", "[pause] [-h] - Add your current location as a waypoint to your NPC target's AI path. (-h to use current heading)", 170, command_wpadd) ||
		command_add("wpinfo", "- Show waypoint info about your NPC target", 170, command_wpinfo) ||
		command_add("worldwide", "Performs world-wide GM functions such as cast (can be extended for other commands). Use caution", 250, command_worldwide) ||
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

	std::vector<std::pair<std::string, uint8>> injected_command_settings;
	std::vector<std::string> orphaned_command_settings;

	for (auto cs_iter : command_settings) {

		auto cl_iter = commandlist.find(cs_iter.first);
		if (cl_iter == commandlist.end()) {

			orphaned_command_settings.push_back(cs_iter.first);
			LogInfo(
				"Command [{}] no longer exists... Deleting orphaned entry from `command_settings` table...",
				cs_iter.first.c_str()
			);
		}
	}

	if (orphaned_command_settings.size()) {
		if (!database.UpdateOrphanedCommandSettings(orphaned_command_settings)) {
			LogInfo("Failed to process 'Orphaned Commands' update operation.");
		}
	}

	auto working_cl = commandlist;
	for (auto working_cl_iter : working_cl) {

		auto cs_iter = command_settings.find(working_cl_iter.first);
		if (cs_iter == command_settings.end()) {

			injected_command_settings.push_back(std::pair<std::string, uint8>(working_cl_iter.first, working_cl_iter.second->access));
			LogInfo(
				"New Command [{}] found... Adding to `command_settings` table with access [{}]...",
				working_cl_iter.first.c_str(),
				working_cl_iter.second->access
			);

			if (working_cl_iter.second->access == 0) {
				LogCommands(
					"command_init(): Warning: Command [{}] defaulting to access level 0!",
					working_cl_iter.first.c_str()
				);
			}

			continue;
		}

		working_cl_iter.second->access = cs_iter->second.first;
		LogCommands(
			"command_init(): - Command [{}] set to access level [{}]",
			working_cl_iter.first.c_str(),
			cs_iter->second.first
		);

		if (cs_iter->second.second.empty()) {
			continue;
		}

		for (auto alias_iter : cs_iter->second.second) {
			if (alias_iter.empty()) {
				continue;
			}

			if (commandlist.find(alias_iter) != commandlist.end()) {
				LogCommands(
					"command_init(): Warning: Alias [{}] already exists as a command - skipping!",
					alias_iter.c_str()
				);

				continue;
			}

			commandlist[alias_iter] = working_cl_iter.second;
			commandaliases[alias_iter] = working_cl_iter.first;

			LogCommands(
				"command_init(): - Alias [{}] added to command [{}]",
				alias_iter.c_str(),
				commandaliases[alias_iter].c_str()
			);
		}
	}

	if (injected_command_settings.size()) {
		if (!database.UpdateInjectedCommandSettings(injected_command_settings)) {
			LogInfo("Failed to process 'Injected Commands' update operation.");
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
		LogError("command_add() - Command added with empty name string - check command.cpp");
		return -1;
	}
	if (function == nullptr) {
		LogError("command_add() - Command [{}] added without a valid function pointer - check command.cpp", command_name.c_str());
		return -1;
	}
	if (commandlist.count(command_name) != 0) {
		LogError("command_add() - Command [{}] is a duplicate command name - check command.cpp", command_name.c_str());
		return -1;
	}
	for (auto iter = commandlist.begin(); iter != commandlist.end(); ++iter) {
		if (iter->second->function != function)
			continue;
		LogError("command_add() - Command [{}] equates to an alias of [{}] - check command.cpp", command_name.c_str(), iter->first.c_str());
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
		c->Message(Chat::Red,"Your access level is not high enough to use this command.");
		return(-1);
	}

	/* QS: Player_Log_Issued_Commands */
	if (RuleB(QueryServ, PlayerLogIssuedCommandes)){
		std::string event_desc = StringFormat("Issued command :: '%s' in zoneid:%i instid:%i",  message, c->GetZoneID(), c->GetInstanceID());
		QServ->PlayerLogEvent(Player_Log_Issued_Commands, c->CharacterID(), event_desc);
	}

	if(cur->access >= COMMANDS_LOGGING_MIN_STATUS) {
		LogCommands("[{}] ([{}]) used command: [{}] (target=[{}])",  c->GetName(), c->AccountName(), message, c->GetTarget()?c->GetTarget()->GetName():"NONE");
	}

	if(cur->function == nullptr) {
		LogError("Command [{}] has a null function\n", cstr.c_str());
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
void command_worldwide(Client *c, const Seperator *sep)
{
	std::string sub_command;
	if (sep->arg[1]) {
		sub_command = sep->arg[1];
	}

	if (sub_command == "cast") {
		if (sep->arg[2][0] && Seperator::IsNumber(sep->arg[2])) {
			int spell_id = atoi(sep->arg[2]);
			quest_manager.WorldWideCastSpell(spell_id, 0, 0);
			worldserver.SendEmoteMessage(0, 0, 15, fmt::format("<SYSTEMWIDE MESSAGE> A GM has cast [{}] world-wide!", GetSpellName(spell_id)).c_str());
		}
		else {
			c->Message(Chat::Yellow, "Usage: #worldwide cast [spellid]");
		}
	}

	if (!sep->arg[1]) {
		c->Message(Chat::White, "This command is used to perform world-wide tasks");
		c->Message(Chat::White, "Usage: #worldwide cast [spellid]");
	}
}
void command_endurance(Client *c, const Seperator *sep)
{
	Mob *t;

	t = c->GetTarget() ? c->GetTarget() : c;

	if (t->IsClient())
		t->CastToClient()->SetEndurance(t->CastToClient()->GetMaxEndurance());
	else
		t->SetEndurance(t->GetMaxEndurance());

	t->Message(Chat::White, "Your endurance has been refilled.");
}
void command_setstat(Client* c, const Seperator* sep){
	if(sep->arg[1][0] && sep->arg[2][0] && c->GetTarget()!=0 && c->GetTarget()->IsClient()){
		c->GetTarget()->CastToClient()->SetStats(atoi(sep->arg[1]),atoi(sep->arg[2]));
	}
	else{
		c->Message(Chat::White,"This command is used to permanently increase or decrease a players stats.");
		c->Message(Chat::White,"Usage: #setstat {type} {value the stat should be}");
		c->Message(Chat::White,"Types: Str: 0, Sta: 1, Agi: 2, Dex: 3, Int: 4, Wis: 5, Cha: 6");
	}
}

void command_incstat(Client* c, const Seperator* sep){
	if(sep->arg[1][0] && sep->arg[2][0] && c->GetTarget()!=0 && c->GetTarget()->IsClient()){
		c->GetTarget()->CastToClient()->IncStats(atoi(sep->arg[1]),atoi(sep->arg[2]));
	}
	else{
		c->Message(Chat::White,"This command is used to permanently increase or decrease a players stats.");
		c->Message(Chat::White,"Usage: #setstat {type} {value by which to increase or decrease}");
		c->Message(Chat::White,"Note: The value is in increments of 2, so a value of 3 will actually increase the stat by 6");
		c->Message(Chat::White,"Types: Str: 0, Sta: 1, Agi: 2, Dex: 3, Int: 4, Wis: 5, Cha: 6");
	}
}

void command_resetaa(Client* c,const Seperator *sep) {
	if(c->GetTarget() && c->GetTarget()->IsClient()){
		c->GetTarget()->CastToClient()->ResetAA();
		c->Message(Chat::Red,"Successfully reset %s's AAs", c->GetTarget()->GetName());
	}
	else
		c->Message(Chat::White,"Usage: Target a client and use #resetaa to reset the AA data in their Profile.");
}

void command_help(Client *c, const Seperator *sep)
{
	int commands_shown=0;

	c->Message(Chat::White, "Available EQEMu commands:");

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
		c->Message(Chat::White, "	%c%s %s",  COMMAND_CHAR, cur->first.c_str(), cur->second->desc == nullptr?"":cur->second->desc);
	}
	if (parse->PlayerHasQuestSub(EVENT_COMMAND)) {
		int i = parse->EventPlayer(EVENT_COMMAND, c, sep->msg, 0);
		if (i >= 1) {
			commands_shown += i;
		}
	}
	c->Message(Chat::White, "%d command%s listed.",  commands_shown, commands_shown!=1?"s":"");

}

void command_version(Client *c, const Seperator *sep)
{
	c->Message(Chat::White, "Current version information.");
	c->Message(Chat::White, "	%s",  CURRENT_VERSION);
	c->Message(Chat::White, "	Compiled on: %s at %s",  COMPILE_DATE, COMPILE_TIME);
	c->Message(Chat::White, "	Last modified on: %s",  LAST_MODIFIED);
}

void command_setfaction(Client *c, const Seperator *sep)
{
	if((sep->arg[1][0] == 0 || strcasecmp(sep->arg[1],"*")==0) || ((c->GetTarget()==0) || (c->GetTarget()->IsClient()))) {
		c->Message(Chat::White, "Usage: #setfaction [faction number]");
		return;
    }

    auto npcTypeID = c->GetTarget()->CastToNPC()->GetNPCTypeID();
    c->Message(Chat::Yellow,"Setting NPC %u to faction %i",  npcTypeID, atoi(sep->argplus[1]));

    std::string query = StringFormat("UPDATE npc_types SET npc_faction_id = %i WHERE id = %i",
                                    atoi(sep->argplus[1]), npcTypeID);
    content_db.QueryDatabase(query);
}

void command_serversidename(Client *c, const Seperator *sep)
{
	if(c->GetTarget())
		c->Message(Chat::White, c->GetTarget()->GetName());
	else
		c->Message(Chat::White, "Error: no target");
}

void command_wc(Client *c, const Seperator *sep)
{
	if (sep->argnum < 2) {
		c->Message(
			0,
			"Usage: #wc [wear slot] [material] [ [hero_forge_model] [elite_material] [unknown06] [unknown18] ]"
		);
	}
	else if (c->GetTarget() == nullptr) {
		c->Message(Chat::Red, "You must have a target to do a wear change.");
	}
	else {
		uint32 hero_forge_model = 0;
		uint32 wearslot         = atoi(sep->arg[1]);

		// Hero Forge
		if (sep->argnum > 2) {
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
		c->GetTarget()->SendTextureWC(
			wearslot,
			atoi(sep->arg[2]),
			hero_forge_model,
			atoi(sep->arg[4]),
			atoi(sep->arg[5]),
			atoi(sep->arg[6]));
	}
}

void command_heromodel(Client *c, const Seperator *sep)
{
	if (sep->argnum < 1) {
		c->Message(Chat::White, "Usage: #heromodel [hero forge model] [ [slot] ] (example: #heromodel 63)");
	}
	else if (c->GetTarget() == nullptr) {
		c->Message(Chat::Red, "You must have a target to do a wear change for Hero's Forge Models.");
	}
	else {
		uint32 hero_forge_model = atoi(sep->arg[1]);

		if (sep->argnum > 1) {
			uint8 wearslot = (uint8) atoi(sep->arg[2]);
			c->GetTarget()->SendTextureWC(wearslot, 0, hero_forge_model, 0, 0, 0);
		}
		else {
			if (hero_forge_model > 0) {
				// Conversion to simplify the command arguments
				// Hero's Forge model is actually model * 1000 + texture * 100 + wearslot
				// Hero's Forge Model slot 7 is actually for Robes, but it still needs to use wearslot 1 in the packet
				hero_forge_model *= 100;

				for (uint8 wearslot = 0; wearslot < 7; wearslot++) {
					c->GetTarget()->SendTextureWC(wearslot, 0, (hero_forge_model + wearslot), 0, 0, 0);
				}
			}
			else {
				c->Message(Chat::Red, "Hero's Forge Model must be greater than 0.");
			}
		}
	}
}

void command_setanim(Client *c, const Seperator *sep)
{
	if (c->GetTarget() && sep->IsNumber(1)) {
		int num = atoi(sep->arg[1]);
		if (num < 0 || num >= _eaMaxAppearance) {
			c->Message(Chat::White, "Invalid animation number, between 0 and %d", _eaMaxAppearance - 1);
		}
		c->GetTarget()->SetAppearance(EmuAppearance(num));
	}
	else {
		c->Message(Chat::White, "Usage: #setanim [animnum]");
	}
}

void command_serverinfo(Client *c, const Seperator *sep)
{
	auto os = EQ::GetOS();
	auto cpus = EQ::GetCPUs();
	auto pid = EQ::GetPID();
	auto rss = EQ::GetRSS();
	auto uptime = EQ::GetUptime();

	c->Message(Chat::White, "Operating System Information");
	c->Message(Chat::White, "==================================================");
	c->Message(Chat::White, "System: %s", os.sysname.c_str());
	c->Message(Chat::White, "Release: %s", os.release.c_str());
	c->Message(Chat::White, "Version: %s", os.version.c_str());
	c->Message(Chat::White, "Machine: %s", os.machine.c_str());
	c->Message(Chat::White, "Uptime: %.2f seconds", uptime);
	c->Message(Chat::White, "==================================================");
	c->Message(Chat::White, "CPU Information");
	c->Message(Chat::White, "==================================================");
	for (size_t i = 0; i < cpus.size(); ++i) {
		auto &cp = cpus[i];
		c->Message(Chat::White, "CPU #%i: %s, Speed: %.2fGhz", i, cp.model.c_str(), cp.speed);
	}
	c->Message(Chat::White, "==================================================");
	c->Message(Chat::White, "Process Information");
	c->Message(Chat::White, "==================================================");
	c->Message(Chat::White, "PID: %u", pid);
	c->Message(Chat::White, "RSS: %.2f MB", rss / 1048576.0);
	c->Message(Chat::White, "==================================================");
}

void command_getvariable(Client *c, const Seperator *sep)
{
	std::string tmp;
	if (database.GetVariable(sep->argplus[1], tmp))
		c->Message(Chat::White, "%s = %s",  sep->argplus[1], tmp.c_str());
	else
		c->Message(Chat::White, "GetVariable(%s) returned false",  sep->argplus[1]);
}

void command_chat(Client *c, const Seperator *sep)
{
	if (sep->arg[2][0] == 0)
		c->Message(Chat::White, "Usage: #chat [channum] [message]");
	else
		if (!worldserver.SendChannelMessage(0, 0, (uint8) atoi(sep->arg[1]), 0, 0, 100, sep->argplus[2]))
			c->Message(Chat::White, "Error: World server disconnected");
}

void command_npcloot(Client *c, const Seperator *sep)
{
	if (c->GetTarget() == 0)
		c->Message(Chat::White, "Error: No target");
	// #npcloot show
	else if (strcasecmp(sep->arg[1], "show") == 0)
	{
		if (c->GetTarget()->IsNPC())
			c->GetTarget()->CastToNPC()->QueryLoot(c);
		else if (c->GetTarget()->IsCorpse())
			c->GetTarget()->CastToCorpse()->QueryLoot(c);
		else
			c->Message(Chat::White, "Error: Target's type doesnt have loot");
	}
	// These 2 types are *BAD* for the next few commands
	else if (c->GetTarget()->IsClient() || c->GetTarget()->IsCorpse())
		c->Message(Chat::White, "Error: Invalid target type, try a NPC =).");
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
				c->Message(Chat::White, "Added item(%i) to the %s's loot.",  item, c->GetTarget()->GetName());
			}
			else
				c->Message(Chat::White, "Error: #npcloot add: Item(%i) does not exist!",  item);
		}
		else if (!sep->IsNumber(2))
			c->Message(Chat::White, "Error: #npcloot add: Itemid must be a number.");
		else
			c->Message(Chat::White, "Error: #npcloot add: This is not a valid target.");
	}
	// #npcloot remove
	else if (strcasecmp(sep->arg[1], "remove") == 0)
	{
		//#npcloot remove all
		if (strcasecmp(sep->arg[2], "all") == 0)
			c->Message(Chat::White, "Error: #npcloot remove all: Not yet implemented.");
		//#npcloot remove itemid
		else
		{
			if(c->GetTarget()->IsNPC() && sep->IsNumber(2))
			{
				uint32 item = atoi(sep->arg[2]);
				c->GetTarget()->CastToNPC()->RemoveItem(item);
				c->Message(Chat::White, "Removed item(%i) from the %s's loot.",  item, c->GetTarget()->GetName());
			}
			else if (!sep->IsNumber(2))
				c->Message(Chat::White, "Error: #npcloot remove: Item must be a number.");
			else
				c->Message(Chat::White, "Error: #npcloot remove: This is not a valid target.");
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
				c->Message(Chat::White, "Set %i Platinum, %i Gold, %i Silver, and %i Copper as %s's money.",  atoi(sep->arg[2]), atoi(sep->arg[3]), atoi(sep->arg[4]), atoi(sep->arg[5]), c->GetTarget()->GetName());
			}
			else
				c->Message(Chat::White, "Error: #npcloot money: Values must be between 0-34465.");
		}
		else
			c->Message(Chat::White, "Usage: #npcloot money platinum gold silver copper");
	}
	else
		c->Message(Chat::White, "Usage: #npcloot [show/money/add/remove] [itemid/all/money: pp gp sp cp]");
}

void command_gm(Client *c, const Seperator *sep)
{
	bool state=atobool(sep->arg[1]);
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	if(sep->arg[1][0] != 0) {
		t->SetGM(state);
		c->Message(Chat::White, "%s is %s a GM.",  t->GetName(), state?"now":"no longer");
	}
	else
		c->Message(Chat::White, "Usage: #gm [on/off]");
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
				c->Message(Chat::White, "Error: World server disconnected.");
			else
			{ // player is in another zone
				//Taking this command out until we test the factor of 8 in ServerOP_ZonePlayer
				//c->Message(Chat::White, "Summoning player from another zone not yet implemented.");
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
			c->Message(Chat::White, "You need a NPC/corpse target for this command");
		else*/
			c->Message(Chat::White, "Usage: #summon [charname] Either target or charname is required");
		return;
	}

	if(!t)
		return;

	if (t->IsNPC())
	{ // npc target
		c->Message(Chat::White, "Summoning NPC %s to %1.1f, %1.1f, %1.1f",  t->GetName(), c->GetX(), c->GetY(), c->GetZ());
		t->CastToNPC()->GMMove(c->GetX(), c->GetY(), c->GetZ(), c->GetHeading());
		t->CastToNPC()->SaveGuardSpot(glm::vec4(0.0f));
	}
	else if (t->IsCorpse())
	{ // corpse target
		c->Message(Chat::White, "Summoning corpse %s to %1.1f, %1.1f, %1.1f",  t->GetName(), c->GetX(), c->GetY(), c->GetZ());
		t->CastToCorpse()->GMMove(c->GetX(), c->GetY(), c->GetZ(), c->GetHeading());
	}
	else if (t->IsClient())
	{
		/*if(c->Admin() < 150)
		{
			c->Message(Chat::White, "You may not summon a player.");
			return;
		}*/
		c->Message(Chat::White, "Summoning player %s to %1.1f, %1.1f, %1.1f",  t->GetName(), c->GetX(), c->GetY(), c->GetZ());
		t->CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), c->GetX(), c->GetY(), c->GetZ(), c->GetHeading(), 2, GMSummon);
	}
}

void command_zone(Client *c, const Seperator *sep)
{
	if(c->Admin() < commandZoneToCoords &&
		(sep->IsNumber(2) || sep->IsNumber(3) || sep->IsNumber(4))) {
		c->Message(Chat::White, "Your status is not high enough to zone to specific coordinates.");
		return;
	}

	uint16 zoneid = 0;

	if (sep->IsNumber(1))
	{
		if(atoi(sep->arg[1])==26 && (c->Admin() < commandZoneToSpecials)){ //cshome
				c->Message(Chat::White, "Only Guides and above can goto that zone.");
				return;
		}
		zoneid = atoi(sep->arg[1]);
	}
	else if (sep->arg[1][0] == 0)
	{
		c->Message(Chat::White, "Usage: #zone [zonename]");
		c->Message(Chat::White, "Optional Usage: #zone [zonename] y x z");
		return;
	}
	else if (zone->GetZoneID() == 184 && c->Admin() < commandZoneToSpecials) {	// Zone: 'Load'
		c->Message(Chat::White, "The Gods brought you here, only they can send you away.");
		return;
	} else {
		if((strcasecmp(sep->arg[1], "cshome")==0) && (c->Admin() < commandZoneToSpecials)){
			c->Message(Chat::White, "Only Guides and above can goto that zone.");
			return;
		}

		zoneid = ZoneID(sep->arg[1]);
		if(zoneid == 0) {
			c->Message(Chat::White, "Unable to locate zone '%s'",  sep->arg[1]);
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
		c->Message(Chat::White, "Your status is not high enough to zone to specific coordinates.");
		return;
	}

	if (sep->arg[1][0] == 0)
	{
		c->Message(Chat::White, "Usage: #zoneinstance [instance id]");
		c->Message(Chat::White, "Optional Usage: #zoneinstance [instance id] y x z");
		return;
	}

	uint16 zoneid = 0;
	uint16 instanceid = 0;

	if(sep->IsNumber(1))
	{
		instanceid = atoi(sep->arg[1]);
		if(!instanceid)
		{
			c->Message(Chat::White, "Must enter a valid instance id.");
			return;
		}

		zoneid = database.ZoneIDFromInstanceID(instanceid);
		if(!zoneid)
		{
			c->Message(Chat::White, "Instance not found or zone is set to null.");
			return;
		}
	}
	else
	{
		c->Message(Chat::White, "Must enter a valid instance id.");
		return;
	}

	if(!database.VerifyInstanceAlive(instanceid, c->CharacterID()))
	{
		c->Message(Chat::White, "Instance ID expiried or you are not apart of this instance.");
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
		c->Message(Chat::Red,"You must wait %i minute(s) before using this ability again.",  timeleft);
		return;
	}

	if(c->GetHPRatio() < 75) {
		c->Message(Chat::White, "You cannot use this command with less than 75 percent health.");
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
		c->Message(Chat::White, "You cannot use this command in your current state. Settle down and wait.");
		return;
	}

	uint16 zoneid = 0;
	uint8 destzone = 0;
	if (sep->IsNumber(1))
	{
		zoneid = atoi(sep->arg[1]);
		destzone = content_db.GetPEQZone(zoneid, 0);
		if(destzone == 0){
			c->Message(Chat::Red, "You cannot use this command to enter that zone!");
			return;
		}
		if(zoneid == zone->GetZoneID()) {
			c->Message(Chat::Red, "You cannot use this command on the zone you are in!");
			return;
		}
	}
	else if (sep->arg[1][0] == 0 || sep->IsNumber(2) || sep->IsNumber(3) || sep->IsNumber(4) || sep->IsNumber(5))
	{
		c->Message(Chat::White, "Usage: #peqzone [zonename]");
		c->Message(Chat::White, "Optional Usage: #peqzone [zoneid]");
		return;
	}
	else {
		zoneid = ZoneID(sep->arg[1]);
		destzone = content_db.GetPEQZone(zoneid, 0);
		if(zoneid == 0) {
			c->Message(Chat::White, "Unable to locate zone '%s'",  sep->arg[1]);
			return;
		}
		if(destzone == 0){
			c->Message(Chat::Red, "You cannot use this command to enter that zone!");
			return;
		}
		if(zoneid == zone->GetZoneID()) {
			c->Message(Chat::Red, "You cannot use this command on the zone you are in!");
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
		c->Message(Chat::White, "Usage: #movechar [charactername] [zonename]");
	else if (c->Admin() < commandMovecharToSpecials && strcasecmp(sep->arg[2], "cshome") == 0 || strcasecmp(sep->arg[2], "load") == 0 || strcasecmp(sep->arg[2], "load2") == 0)
		c->Message(Chat::White, "Invalid zone name");
	else
	{
		uint32 tmp = database.GetAccountIDByChar(sep->arg[1]);
		if (tmp)
		{
			if (c->Admin() >= commandMovecharSelfOnly || tmp == c->AccountID())
				if (!database.MoveCharacterToZone((char*) sep->arg[1], ZoneID(sep->arg[2])))
					c->Message(Chat::White, "Character Move Failed!");
				else
					c->Message(Chat::White, "Character has been moved.");
			else
				c->Message(Chat::Red,"You cannot move characters that are not on your account.");
		}
		else
			c->Message(Chat::White, "Character Does Not Exist");
	}
}

void command_movement(Client *c, const Seperator *sep)
{
	auto &mgr = MobMovementManager::Get();

	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #movement stats/clearstats/walkto/runto/rotateto/stop/packet");
		return;
	}

	if (strcasecmp(sep->arg[1], "stats") == 0)
	{
		mgr.DumpStats(c);
	}
	else if (strcasecmp(sep->arg[1], "clearstats") == 0)
	{
		mgr.ClearStats();
	}
	else if (strcasecmp(sep->arg[1], "walkto") == 0)
	{
		auto target = c->GetTarget();
		if (target == nullptr) {
			c->Message(Chat::White, "No target found.");
			return;
		}

		target->WalkTo(c->GetX(), c->GetY(), c->GetZ());
	}
	else if (strcasecmp(sep->arg[1], "runto") == 0)
	{
		auto target = c->GetTarget();
		if (target == nullptr) {
			c->Message(Chat::White, "No target found.");
			return;
		}

		target->RunTo(c->GetX(), c->GetY(), c->GetZ());
	}
	else if (strcasecmp(sep->arg[1], "rotateto") == 0)
	{
		auto target = c->GetTarget();
		if (target == nullptr) {
			c->Message(Chat::White, "No target found.");
			return;
		}

		target->RotateToWalking(target->CalculateHeadingToTarget(c->GetX(), c->GetY()));
	}
	else if (strcasecmp(sep->arg[1], "stop") == 0)
	{
		auto target = c->GetTarget();
		if (target == nullptr) {
			c->Message(Chat::White, "No target found.");
			return;
		}

		target->StopNavigation();
	}
	else if (strcasecmp(sep->arg[1], "packet") == 0)
	{
		auto target = c->GetTarget();
		if (target == nullptr) {
			c->Message(Chat::White, "No target found.");
			return;
		}

		mgr.SendCommandToClients(target, atof(sep->arg[2]), atof(sep->arg[3]), atof(sep->arg[4]), atof(sep->arg[5]), atoi(sep->arg[6]), ClientRangeAny);
	}
	else {
		c->Message(Chat::White, "Usage: #movement stats/clearstats/walkto/runto/rotateto/stop/packet");
	}
}

void command_viewpetition(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #viewpetition (petition number) Type #listpetition for a list");
		return;
    }

    c->Message(Chat::Red,"	ID : Character Name , Petition Text");

    std::string query = "SELECT petid, charname, petitiontext FROM petitions ORDER BY petid";
    auto results = database.QueryDatabase(query);
    if (!results.Success())
        return;

    LogInfo("View petition request from [{}], petition number: [{}]",  c->GetName(), atoi(sep->argplus[1]) );

    if (results.RowCount() == 0) {
        c->Message(Chat::Red,"There was an error in your request: ID not found! Please check the Id and try again.");
        return;
    }

    for (auto row = results.begin(); row != results.end(); ++row)
        if (strcasecmp(row[0], sep->argplus[1]) == 0)
			c->Message(Chat::Yellow, " %s:	%s , %s ",  row[0], row[1], row[2]);

}

void command_petitioninfo(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #petitioninfo (petition number) Type #listpetition for a list");
		return;
    }

    std::string query = "SELECT petid, charname, accountname, zone, charclass, charrace, charlevel FROM petitions ORDER BY petid";
    auto results = database.QueryDatabase(query);
    if (!results.Success())
        return;

    LogInfo("Petition information request from [{}], petition number:",  c->GetName(), atoi(sep->argplus[1]) );

    if (results.RowCount() == 0) {
		c->Message(Chat::Red,"There was an error in your request: ID not found! Please check the Id and try again.");
        return;
    }

	for (auto row = results.begin(); row != results.end(); ++row)
		if (strcasecmp(row[0],sep->argplus[1])== 0)
			c->Message(Chat::Red,"	ID : %s Character Name: %s Account Name: %s Zone: %s Character Class: %s Character Race: %s Character Level: %s", row[0],row[1],row[2],row[3],row[4],row[5],row[6]);

}

void command_delpetition(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0 || strcasecmp(sep->arg[1],"*") == 0) {
		c->Message(Chat::White, "Usage: #delpetition (petition number) Type #listpetition for a list");
		return;
    }

	c->Message(Chat::Red,"Attempting to delete petition number: %i", atoi(sep->argplus[1]));
	std::string query = StringFormat("DELETE FROM petitions WHERE petid = %i",  atoi(sep->argplus[1]));
	auto results = database.QueryDatabase(query);
	if (!results.Success())
        return;

    LogInfo("Delete petition request from [{}], petition number:",  c->GetName(), atoi(sep->argplus[1]) );

}

void command_listnpcs(Client *c, const Seperator *sep)
{
	c->Message(Chat::White, "Deprecated, use the #list command (#list npcs <search>)");
}

void command_list(Client *c, const Seperator *sep)
{
	std::string search_type;
	if (strcasecmp(sep->arg[1], "npcs") == 0) {
		search_type = "npcs";
	}

	if (strcasecmp(sep->arg[1], "players") == 0) {
		search_type = "players";
	}

	if (strcasecmp(sep->arg[1], "corpses") == 0) {
		search_type = "corpses";
	}

	if (strcasecmp(sep->arg[1], "doors") == 0) {
		search_type = "doors";
	}

	if (strcasecmp(sep->arg[1], "objects") == 0) {
		search_type = "objects";
	}

	if (search_type.length() > 0) {

		int entity_count = 0;
		int found_count  = 0;

		std::string search_string;

		if (sep->arg[2]) {
			search_string = sep->arg[2];
		}

		/**
		 * NPC
		 */
		if (search_type.find("npcs") != std::string::npos) {
			auto &entity_list_search = entity_list.GetMobList();

			for (auto &itr : entity_list_search) {
				Mob *entity = itr.second;
				if (!entity->IsNPC()) {
					continue;
				}

				entity_count++;

				std::string entity_name = entity->GetName();

				/**
				 * Filter by name
				 */
				if (search_string.length() > 0 && entity_name.find(search_string) == std::string::npos) {
					continue;
				}

				std::string saylink = StringFormat(
					"#goto %.0f %0.f %.0f",
					entity->GetX(),
					entity->GetY(),
					entity->GetZ() + (entity->IsBoat() ? 50 : 0));

				c->Message(
					0,
					"| %s | ID %5d | %s | x %.0f | y %0.f | z %.0f",
					EQ::SayLinkEngine::GenerateQuestSaylink(saylink, false, "Goto").c_str(),
					entity->GetID(),
					entity->GetName(),
					entity->GetX(),
					entity->GetY(),
					entity->GetZ()
				);

				found_count++;
			}
		}

		/**
		 * Client
		 */
		if (search_type.find("players") != std::string::npos) {
			auto &entity_list_search = entity_list.GetClientList();

			for (auto &itr : entity_list_search) {
				Client *entity = itr.second;

				entity_count++;

				std::string entity_name = entity->GetName();

				/**
				 * Filter by name
				 */
				if (search_string.length() > 0 && entity_name.find(search_string) == std::string::npos) {
					continue;
				}

				std::string saylink = StringFormat(
					"#goto %.0f %0.f %.0f",
					entity->GetX(),
					entity->GetY(),
					entity->GetZ());

				c->Message(
					0,
					"| %s | ID %5d | %s | x %.0f | y %0.f | z %.0f",
					EQ::SayLinkEngine::GenerateQuestSaylink(saylink, false, "Goto").c_str(),
					entity->GetID(),
					entity->GetName(),
					entity->GetX(),
					entity->GetY(),
					entity->GetZ()
				);

				found_count++;
			}
		}

		/**
		 * Corpse
		 */
		if (search_type.find("corpses") != std::string::npos) {
			auto &entity_list_search = entity_list.GetCorpseList();

			for (auto &itr : entity_list_search) {
				Corpse *entity = itr.second;

				entity_count++;

				std::string entity_name = entity->GetName();

				/**
				 * Filter by name
				 */
				if (search_string.length() > 0 && entity_name.find(search_string) == std::string::npos) {
					continue;
				}

				std::string saylink = StringFormat(
					"#goto %.0f %0.f %.0f",
					entity->GetX(),
					entity->GetY(),
					entity->GetZ());

				c->Message(
					0,
					"| %s | ID %5d | %s | x %.0f | y %0.f | z %.0f",
					EQ::SayLinkEngine::GenerateQuestSaylink(saylink, false, "Goto").c_str(),
					entity->GetID(),
					entity->GetName(),
					entity->GetX(),
					entity->GetY(),
					entity->GetZ()
				);

				found_count++;
			}
		}

		/**
		 * Doors
		 */
		if (search_type.find("doors") != std::string::npos) {
			auto &entity_list_search = entity_list.GetDoorsList();

			for (auto &itr : entity_list_search) {
				Doors * entity = itr.second;

				entity_count++;

				std::string entity_name = entity->GetDoorName();

				/**
				 * Filter by name
				 */
				if (search_string.length() > 0 && entity_name.find(search_string) == std::string::npos) {
					continue;
				}

				std::string saylink = StringFormat(
					"#goto %.0f %0.f %.0f",
					entity->GetX(),
					entity->GetY(),
					entity->GetZ());

				c->Message(
					0,
					"| %s | Entity ID %5d | Door ID %i | %s | x %.0f | y %0.f | z %.0f",
					EQ::SayLinkEngine::GenerateQuestSaylink(saylink, false, "Goto").c_str(),
					entity->GetID(),
					entity->GetDoorID(),
					entity->GetDoorName(),
					entity->GetX(),
					entity->GetY(),
					entity->GetZ()
				);

				found_count++;
			}
		}

		/**
		 * Objects
		 */
		if (search_type.find("objects") != std::string::npos) {
			auto &entity_list_search = entity_list.GetObjectList();

			for (auto &itr : entity_list_search) {
				Object * entity = itr.second;

				entity_count++;

				std::string entity_name = entity->GetModelName();

				/**
				 * Filter by name
				 */
				if (search_string.length() > 0 && entity_name.find(search_string) == std::string::npos) {
					continue;
				}

				std::string saylink = StringFormat(
					"#goto %.0f %0.f %.0f",
					entity->GetX(),
					entity->GetY(),
					entity->GetZ());

				c->Message(
					0,
					"| %s | Entity ID %5d | Object DBID %i | %s | x %.0f | y %0.f | z %.0f",
					EQ::SayLinkEngine::GenerateQuestSaylink(saylink, false, "Goto").c_str(),
					entity->GetID(),
					entity->GetDBID(),
					entity->GetModelName(),
					entity->GetX(),
					entity->GetY(),
					entity->GetZ()
				);

				found_count++;
			}
		}

		if (found_count) {
			c->Message(
				0, "Found (%i) of type (%s) in zone (%i) total",
				found_count,
				search_type.c_str(),
				entity_count
			);
		}
	}
	else {
		c->Message(Chat::White, "Usage of #list");
		c->Message(Chat::White, "- #list [npcs|players|corpses|doors|objects] [search]");
		c->Message(Chat::White, "- Example: #list npcs (Blank for all)");
	}
}

void command_date(Client *c, const Seperator *sep)
{
	//yyyy mm dd hh mm local
	if(sep->arg[3][0]==0 || !sep->IsNumber(1) || !sep->IsNumber(2) || !sep->IsNumber(3)) {
		c->Message(Chat::Red, "Usage: #date yyyy mm dd [HH MM]");
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
		c->Message(Chat::Red, "Setting world time to %s-%s-%s %i:%i...",  sep->arg[1], sep->arg[2], sep->arg[3], h, m);
		zone->SetDate(atoi(sep->arg[1]), atoi(sep->arg[2]), atoi(sep->arg[3]), h, m);
	}
}

void command_timezone(Client *c, const Seperator *sep)
{
	if(sep->arg[1][0]==0 && !sep->IsNumber(1)) {
		c->Message(Chat::Red, "Usage: #timezone HH [MM]");
		c->Message(Chat::Red, "Current timezone is: %ih %im",  zone->zone_time.getEQTimeZoneHr(), zone->zone_time.getEQTimeZoneMin());
	}
	else {
		uint8 hours = atoi(sep->arg[1]);
		uint8 minutes = atoi(sep->arg[2]);
		if(!sep->IsNumber(2))
			minutes = 0;
		c->Message(Chat::Red, "Setting timezone to %i h %i m",  hours, minutes);
		uint32 ntz=(hours*60)+minutes;
		zone->zone_time.setEQTimeZone(ntz);
		content_db.SetZoneTZ(zone->GetZoneID(), zone->GetInstanceVersion(), ntz);

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
		c->Message(Chat::White, "%s is %s invulnerable from attack.",  t->GetName(), state?"now":"no longer");
	}
	else
		c->Message(Chat::White, "Usage: #invulnerable [on/off]");
}

void command_hideme(Client *c, const Seperator *sep)
{
	bool state=atobool(sep->arg[1]);

	if(sep->arg[1][0]==0)
		c->Message(Chat::White, "Usage: #hideme [on/off]");
	else
	{
		c->SetHideMe(state);
		c->MessageString(Chat::Broadcasts, c->GetHideMe() ? NOW_INVISIBLE : NOW_VISIBLE, c->GetName());
	}
}

void command_emote(Client *c, const Seperator *sep)
{
	if (sep->arg[3][0] == 0)
		c->Message(Chat::White, "Usage: #emote [name | world | zone] type# message");
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
			c->Message(Chat::White, "Error: World server disconnected");
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
			c->Message(Chat::White, "You are behind mob %s, it is looking to %d",  c->GetTarget()->GetName(), c->GetTarget()->GetHeading());
		else
			c->Message(Chat::White, "You are NOT behind mob %s, it is looking to %d",  c->GetTarget()->GetName(), c->GetTarget()->GetHeading());
	else
		c->Message(Chat::White, "I Need a target!");
}

void command_npcstats(Client *c, const Seperator *sep)
{
	if (c->GetTarget() == 0)
		c->Message(Chat::White, "ERROR: No target!");
	else if (!c->GetTarget()->IsNPC())
		c->Message(Chat::White, "ERROR: Target is not a NPC!");
	else {
		auto target_npc = c->GetTarget()->CastToNPC();
		c->Message(Chat::White, "# NPC Stats");
		c->Message(Chat::White, "- Name: %s   NpcID: %u", target_npc->GetName(), target_npc->GetNPCTypeID());
		c->Message(Chat::White, "- Race: %i  Level: %i  Class: %i  Material: %i", target_npc->GetRace(), target_npc->GetLevel(), target_npc->GetClass(), target_npc->GetTexture());
		c->Message(Chat::White, "- Current HP: %i  Max HP: %i", target_npc->GetHP(), target_npc->GetMaxHP());
		//c->Message(Chat::White, "Weapon Item Number: %s", target_npc->GetWeapNo());
		c->Message(Chat::White, "- Gender: %i  Size: %f  Bodytype: %d", target_npc->GetGender(), target_npc->GetSize(), target_npc->GetBodyType());
		c->Message(Chat::White, "- Runspeed: %.3f  Walkspeed: %.3f", static_cast<float>(0.025f * target_npc->GetRunspeed()), static_cast<float>(0.025f * target_npc->GetWalkspeed()));
		c->Message(Chat::White, "- Spawn Group: %i  Grid: %i", target_npc->GetSpawnGroupId(), target_npc->GetGrid());
		if (target_npc->proximity) {
			c->Message(Chat::White, "- Proximity: Enabled");
			c->Message(Chat::White, "-- Cur_X: %1.3f, Cur_Y: %1.3f, Cur_Z: %1.3f", target_npc->GetX(), target_npc->GetY(), target_npc->GetZ());
			c->Message(Chat::White, "-- Min_X: %1.3f(%1.3f), Max_X: %1.3f(%1.3f), X_Range: %1.3f", target_npc->proximity->min_x, (target_npc->proximity->min_x - target_npc->GetX()), target_npc->proximity->max_x, (target_npc->proximity->max_x - target_npc->GetX()), (target_npc->proximity->max_x - target_npc->proximity->min_x));
			c->Message(Chat::White, "-- Min_Y: %1.3f(%1.3f), Max_Y: %1.3f(%1.3f), Y_Range: %1.3f", target_npc->proximity->min_y, (target_npc->proximity->min_y - target_npc->GetY()), target_npc->proximity->max_y, (target_npc->proximity->max_y - target_npc->GetY()), (target_npc->proximity->max_y - target_npc->proximity->min_y));
			c->Message(Chat::White, "-- Min_Z: %1.3f(%1.3f), Max_Z: %1.3f(%1.3f), Z_Range: %1.3f", target_npc->proximity->min_z, (target_npc->proximity->min_z - target_npc->GetZ()), target_npc->proximity->max_z, (target_npc->proximity->max_z - target_npc->GetZ()), (target_npc->proximity->max_z - target_npc->proximity->min_z));
			c->Message(Chat::White, "-- Say: %s", (target_npc->proximity->say ? "Enabled" : "Disabled"));
		}
		else {
			c->Message(Chat::White, "-Proximity: Disabled");
		}
		c->Message(Chat::White, "");
		c->Message(Chat::White, "EmoteID: %i", target_npc->GetEmoteID());
		target_npc->QueryLoot(c);
	}
}

void command_zclip(Client *c, const Seperator *sep)
{
	// modifys and resends zhdr packet
	if(sep->arg[2][0]==0)
		c->Message(Chat::White, "Usage: #zclip <min clip> <max clip>");
	else if(atoi(sep->arg[1])<=0)
		c->Message(Chat::White, "ERROR: Min clip can not be zero or less!");
	else if(atoi(sep->arg[2])<=0)
		c->Message(Chat::White, "ERROR: Max clip can not be zero or less!");
	else if(atoi(sep->arg[1])>atoi(sep->arg[2]))
		c->Message(Chat::White, "ERROR: Min clip is greater than max clip!");
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
			c->Message(Chat::White, "Error: %s not found",  sep->arg[1]);
	}
	else if (c->GetTarget() && c->GetTarget()->IsNPC() && sep->IsNumber(1) && sep->IsNumber(2) ) {
		Mob* spelltar = entity_list.GetMob(atoi(sep->arg[1]));
		if (spelltar)
			c->GetTarget()->CastSpell(atoi(sep->arg[2]), spelltar->GetID());
		else
			c->Message(Chat::White, "Error: target ID %i not found",  atoi(sep->arg[1]));
	}
	else
		c->Message(Chat::White, "Usage: (needs NPC targeted) #npccast targetname/entityid spellid");
}

void command_zstats(Client *c, const Seperator *sep)
{
	c->Message(Chat::White, "Zone Header Data:");
	c->Message(Chat::White, "Sky Type: %i",  zone->newzone_data.sky);
	c->Message(Chat::White, "Fog Colour: Red: %i; Blue: %i; Green %i",  zone->newzone_data.fog_red[0], zone->newzone_data.fog_green[0], zone->newzone_data.fog_blue[0]);
	c->Message(Chat::White, "Safe Coords: %f, %f, %f",  zone->newzone_data.safe_x, zone->newzone_data.safe_y, zone->newzone_data.safe_z);
	c->Message(Chat::White, "Underworld Coords: %f",  zone->newzone_data.underworld);
	c->Message(Chat::White, "Clip Plane: %f - %f",  zone->newzone_data.minclip, zone->newzone_data.maxclip);
}

void command_permaclass(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	if(sep->arg[1][0]==0) {
		c->Message(Chat::White,"Usage: #permaclass <classnum>");
	}
	else if(!t->IsClient())
		c->Message(Chat::White,"Target is not a client.");
	else {
		c->Message(Chat::White, "Setting %s's class...Sending to char select.",  t->GetName());
		LogInfo("Class change request from [{}] for [{}], requested class:[{}]",  c->GetName(), t->GetName(), atoi(sep->arg[1]) );
		t->SetBaseClass(atoi(sep->arg[1]));
		t->Save();
		t->Kick("Class was changed.");
	}
}

void command_permarace(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	if(sep->arg[1][0]==0) {
		c->Message(Chat::White,"Usage: #permarace <racenum>");
		c->Message(Chat::White,"NOTE: Not all models are global. If a model is not global, it will appear as a human on character select and in zones without the model.");
	}
	else if(!t->IsClient())
		c->Message(Chat::White,"Target is not a client.");
	else {
		c->Message(Chat::White, "Setting %s's race - zone to take effect", t->GetName());
		LogInfo("Permanant race change request from [{}] for [{}], requested race:[{}]",  c->GetName(), t->GetName(), atoi(sep->arg[1]) );
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
		c->Message(Chat::White,"Usage: #permagender <gendernum>");
		c->Message(Chat::White,"Gender Numbers: 0=Male, 1=Female, 2=Neuter");
	}
	else if(!t->IsClient())
		c->Message(Chat::White,"Target is not a client.");
	else {
		c->Message(Chat::White, "Setting %s's gender - zone to take effect", t->GetName());
		LogInfo("Permanant gender change request from [{}] for [{}], requested gender:[{}]",  c->GetName(), t->GetName(), atoi(sep->arg[1]) );
		t->SetBaseGender(atoi(sep->arg[1]));
		t->Save();
		t->SendIllusionPacket(atoi(sep->arg[1]));
	}
}

void command_weather(Client *c, const Seperator *sep)
{
	if (!(sep->arg[1][0] == '0' || sep->arg[1][0] == '1' || sep->arg[1][0] == '2' || sep->arg[1][0] == '3')) {
		c->Message(Chat::White, "Usage: #weather <0/1/2/3> - Off/Rain/Snow/Manual.");
	}
	else if(zone->zone_weather == 0) {
		if(sep->arg[1][0] == '3')	{ // Put in modifications here because it had a very good chance at screwing up the client's weather system if rain was sent during snow -T7
			if(sep->arg[2][0] != 0 && sep->arg[3][0] != 0) {
				c->Message(Chat::White, "Sending weather packet... TYPE=%s, INTENSITY=%s",  sep->arg[2], sep->arg[3]);
				zone->zone_weather = atoi(sep->arg[2]);
				auto outapp = new EQApplicationPacket(OP_Weather, 8);
				outapp->pBuffer[0] = atoi(sep->arg[2]);
				outapp->pBuffer[4] = atoi(sep->arg[3]); // This number changes in the packets, intensity?
				entity_list.QueueClients(c, outapp);
				safe_delete(outapp);
			}
			else {
				c->Message(Chat::White, "Manual Usage: #weather 3 <type> <intensity>");
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
		c->Message(Chat::White, "Usage: #zheader <zone name>");
	}
	else if(ZoneID(sep->argplus[1])==0)
		c->Message(Chat::White, "Invalid Zone Name: %s",  sep->argplus[1]);
	else {

		if (zone->LoadZoneCFG(sep->argplus[1], 0))
			c->Message(Chat::White, "Successfully loaded zone header for %s from database.",  sep->argplus[1]);
		else
			c->Message(Chat::White, "Failed to load zone header %s from database",  sep->argplus[1]);
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
		c->Message(Chat::White, "Usage: #zsky <sky type>");
	else if(atoi(sep->arg[1])<0||atoi(sep->arg[1])>255)
		c->Message(Chat::White, "ERROR: Sky type can not be less than 0 or greater than 255!");
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
		c->Message(Chat::White, "Usage: #zcolor <red> <green> <blue>");
	else if (atoi(sep->arg[1])<0||atoi(sep->arg[1])>255)
		c->Message(Chat::White, "ERROR: Red can not be less than 0 or greater than 255!");
	else if (atoi(sep->arg[2])<0||atoi(sep->arg[2])>255)
		c->Message(Chat::White, "ERROR: Green can not be less than 0 or greater than 255!");
	else if (atoi(sep->arg[3])<0||atoi(sep->arg[3])>255)
		c->Message(Chat::White, "ERROR: Blue can not be less than 0 or greater than 255!");
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

void command_gassign(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(1) && c->GetTarget() && c->GetTarget()->IsNPC() && c->GetTarget()->CastToNPC()->GetSpawnPointID() > 0) {
		int spawn2id = c->GetTarget()->CastToNPC()->GetSpawnPointID();
		database.AssignGrid(c, atoi(sep->arg[1]), spawn2id);
	}
	else
		c->Message(Chat::White, "Usage: #gassign [num] - must have an npc target!");
}

void command_ai(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();

	if (strcasecmp(sep->arg[1], "factionid") == 0) {
		if (target && sep->IsNumber(2)) {
			if (target->IsNPC())
				target->CastToNPC()->SetNPCFactionID(atoi(sep->arg[2]));
			else
				c->Message(Chat::White, "%s is not an NPC.",  target->GetName());
		}
		else
			c->Message(Chat::White, "Usage: (targeted) #ai factionid [factionid]");
	}
	else if (strcasecmp(sep->arg[1], "spellslist") == 0) {
		if (target && sep->IsNumber(2) && atoi(sep->arg[2]) >= 0) {
			if (target->IsNPC())
				target->CastToNPC()->AI_AddNPCSpells(atoi(sep->arg[2]));
			else
				c->Message(Chat::White, "%s is not an NPC.",  target->GetName());
		}
		else
			c->Message(Chat::White, "Usage: (targeted) #ai spellslist [npc_spells_id]");
	}
	else if (strcasecmp(sep->arg[1], "con") == 0) {
		if (target && sep->arg[2][0] != 0) {
			Mob* tar2 = entity_list.GetMob(sep->arg[2]);
			if (tar2)
				c->Message(Chat::White, "%s considering %s: %i",  target->GetName(), tar2->GetName(), tar2->GetReverseFactionCon(target));
			else
				c->Message(Chat::White, "Error: %s not found.",  sep->arg[2]);
		}
		else
			c->Message(Chat::White, "Usage: (targeted) #ai con [mob name]");
	}
	else if (strcasecmp(sep->arg[1], "guard") == 0) {
		if (target && target->IsNPC())
			target->CastToNPC()->SaveGuardSpot(target->GetPosition());
		else
			c->Message(Chat::White, "Usage: (targeted) #ai guard - sets npc to guard the current location (use #summon to move)");
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
				c->Message(Chat::White, "Usage: #ai roambox dist max_x min_x max_y min_y [delay] [mindelay]");
				c->Message(Chat::White, "Usage: #ai roambox dist roamdist [delay] [mindelay]");
			}
		}
		else
			c->Message(Chat::White, "You need a AI NPC targeted");
	}
	else if (strcasecmp(sep->arg[1], "stop") == 0 && c->Admin() >= commandToggleAI) {
		if (target) {
			if (target->IsAIControlled())
				target->AI_Stop();
			else
				c->Message(Chat::White, "Error: Target is not AI controlled");
		}
		else
			c->Message(Chat::White, "Usage: Target a Mob with AI enabled and use this to turn off their AI.");
	}
	else if (strcasecmp(sep->arg[1], "start") == 0 && c->Admin() >= commandToggleAI) {
		if (target) {
			if (!target->IsAIControlled())
				target->AI_Start();
			else
				c->Message(Chat::White, "Error: Target is already AI controlled");
		}
		else
			c->Message(Chat::White, "Usage: Target a Mob with AI disabled and use this to turn on their AI.");
	}
	else {
		c->Message(Chat::White, "#AI Sub-commands");
		c->Message(Chat::White, "  factionid");
		c->Message(Chat::White, "  spellslist");
		c->Message(Chat::White, "  con");
		c->Message(Chat::White, "  guard");
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
			c->Message(Chat::White, "Sending shutdown packet now, World will shutdown in: %i minutes with an interval of: %i seconds",  (time / 60), interval);
			auto pack = new ServerPacket(ServerOP_ShutdownAll, sizeof(WorldShutDown_Struct));
			WorldShutDown_Struct* wsd = (WorldShutDown_Struct*)pack->pBuffer;
			wsd->time=time*1000;
			wsd->interval=(interval*1000);
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
		else if(strcasecmp(sep->arg[1], "now") == 0){
			worldserver.SendEmoteMessage(0,0,15,"<SYSTEMWIDE MESSAGE>:SYSTEM MSG:World coming down, everyone log out now.");
			c->Message(Chat::White, "Sending shutdown packet");
			auto pack = new ServerPacket;
			pack->opcode = ServerOP_ShutdownAll;
			pack->size=0;
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
		else if(strcasecmp(sep->arg[1], "disable") == 0){
			c->Message(Chat::White, "Shutdown prevented, next time I may not be so forgiving...");
			auto pack = new ServerPacket(ServerOP_ShutdownAll, sizeof(WorldShutDown_Struct));
			WorldShutDown_Struct* wsd = (WorldShutDown_Struct*)pack->pBuffer;
			wsd->time=0;
			wsd->interval=0;
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
		else{
			c->Message(Chat::White,"#worldshutdown - Shuts down the server and all zones.");
			c->Message(Chat::White,"Usage: #worldshutdown now - Shuts down the server and all zones immediately.");
			c->Message(Chat::White,"Usage: #worldshutdown disable - Stops the server from a previously scheduled shut down.");
			c->Message(Chat::White,"Usage: #worldshutdown [timer] [interval] - Shuts down the server and all zones after [timer] seconds and sends warning every [interval] seconds.");
		}
	}
	else
		c->Message(Chat::White, "Error: World server disconnected");
}

void command_sendzonespawns(Client *c, const Seperator *sep)
{
	entity_list.SendZoneSpawns(c);
}

void command_zsave(Client *c, const Seperator *sep)
{
	if (zone->SaveZoneCFG()) {
		c->Message(Chat::Red, "Zone header saved successfully.");
	}
	else {
		c->Message(Chat::Red, "ERROR: Zone header data was NOT saved.");
	}
}

void command_dbspawn2(Client *c, const Seperator *sep)
{

	if (sep->IsNumber(1) && sep->IsNumber(2) && sep->IsNumber(3)) {
		LogInfo("Spawning database spawn");
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
		c->Message(Chat::White, "Usage: #dbspawn2 spawngroup respawn variance [condition_id] [condition_min]");
	}
}

void command_shutdown(Client *c, const Seperator *sep)
{
	CatchSignal(2);
}

void command_delacct(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0)
		c->Message(Chat::White, "Format: #delacct accountname");
	else {
		std::string user;
		std::string loginserver;
		ParseAccountString(sep->arg[1], user, loginserver);

		if (database.DeleteAccount(user.c_str(), loginserver.c_str()))
			c->Message(Chat::White, "The account was deleted.");
		else
			c->Message(Chat::White, "Unable to delete account.");
	}
}

void command_setpass(Client *c, const Seperator *sep)
{
	if(sep->argnum != 2)
		c->Message(Chat::White, "Format: #setpass accountname password");
	else {
		std::string user;
		std::string loginserver;
		ParseAccountString(sep->arg[1], user, loginserver);

		int16 tmpstatus = 0;
		uint32 tmpid = database.GetAccountIDByName(user.c_str(), loginserver.c_str(), &tmpstatus);
		if (!tmpid)
			c->Message(Chat::White, "Error: Account not found");
		else if (tmpstatus > c->Admin())
			c->Message(Chat::White, "Cannot change password: Account's status is higher than yours");
		else if (database.SetLocalPassword(tmpid, sep->arg[2]))
			c->Message(Chat::White, "Password changed.");
		else
			c->Message(Chat::White, "Error changing password.");
	}
}

void command_setlsinfo(Client *c, const Seperator *sep)
{
	if(sep->argnum != 2)
		c->Message(Chat::White, "Format: #setlsinfo email password");
	else {
		auto pack = new ServerPacket(ServerOP_LSAccountUpdate, sizeof(ServerLSAccountUpdate_Struct));
		ServerLSAccountUpdate_Struct* s = (ServerLSAccountUpdate_Struct *) pack->pBuffer;
		s->useraccountid = c->LSAccountID();
		strn0cpy(s->useraccount, c->AccountName(), 30);
		strn0cpy(s->user_email, sep->arg[1], 100);
		strn0cpy(s->userpassword, sep->arg[2], 50);
		worldserver.SendPacket(pack);
		c->Message(Chat::White, "Login Server update packet sent.");
	}
}

void command_grid(Client *c, const Seperator *sep)
{
	auto command_type = sep->arg[1];
	auto zone_id = zone->GetZoneID();
	if (strcasecmp("max", command_type) == 0) {
		c->Message(
			Chat::White,
			fmt::format(
				"Highest grid ID in this zone is {}.",
				content_db.GetHighestGrid(zone_id)
			).c_str()
		);
	} else if (strcasecmp("add", command_type) == 0) {
		auto grid_id = atoi(sep->arg[2]);
		auto wander_type = atoi(sep->arg[3]);
		auto pause_type = atoi(sep->arg[4]);
		if (!content_db.GridExistsInZone(zone_id, grid_id)) {
			content_db.ModifyGrid(c, false, grid_id, wander_type, pause_type, zone_id);
			c->Message(
				Chat::White,
				fmt::format(
					"Grid {} added to zone ID {} with wander type {} and pause type {}.",
					grid_id,
					zone_id,
					wander_type,
					pause_type
				).c_str()
			);
		} else {
			c->Message(
				Chat::White,
				fmt::format(
					"Grid {} already exists in zone ID {}.",
					grid_id,
					zone_id
				).c_str()
			);
			return;
		}
	} else if (strcasecmp("show", command_type) == 0) {
		Mob *target = c->GetTarget();
		if (!target || !target->IsNPC()) {
			c->Message(Chat::White, "You need to target an NPC!");
			return;
		}

		auto grid_id = target->CastToNPC()->GetGrid();
		std::string query = fmt::format(
			"SELECT `x`, `y`, `z`, `heading`, `number` "
			"FROM `grid_entries` "
			"WHERE `zoneid` = {} AND `gridid` = {} "
			"ORDER BY `number`",
			zone_id,
			grid_id
		);

		auto results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			c->Message(Chat::White, "Error querying database.");
			c->Message(Chat::White, query.c_str());
		}

		if (results.RowCount() == 0) {
			c->Message(Chat::White, "No grid found.");
			return;
		}

		// Depop any node npc's already spawned
		entity_list.DespawnGridNodes(grid_id);

		// Spawn grid nodes
		std::map<std::vector<float>, int32> zoffset;
		for (auto row : results) {
			glm::vec4 node_position = glm::vec4(atof(row[0]), atof(row[1]), atof(row[2]), atof(row[3]));
			std::vector<float> node_loc {
				node_position.x,
				node_position.y,
				node_position.z
			};

			// If we already have a node at this location, set the z offset
			// higher from the existing one so we can see it.  Adjust so if
			// there is another at the same spot we adjust again.
			auto search = zoffset.find(node_loc);
			if (search != zoffset.end()) {
				search->second = search->second + 3;
			} else {
				zoffset[node_loc] = 0.0;
			}

			node_position.z += zoffset[node_loc];
			NPC::SpawnGridNodeNPC(node_position, grid_id, atoi(row[4]), zoffset[node_loc]);
		}
		c->Message(
			Chat::White,
			fmt::format(
				"Spawning nodes for grid {}.",
				grid_id
			).c_str()
		);
	} else if (strcasecmp("hide", command_type) == 0) {
		Mob* target = c->GetTarget();
		if (!target || !target->IsNPC()) {
			c->Message(Chat::White, "You need to target an NPC!");
			return;
		}

		auto grid_id = target->CastToNPC()->GetGrid();
		entity_list.DespawnGridNodes(grid_id);
		c->Message(
			Chat::White,
			fmt::format(
				"Depawning nodes for grid {}.",
				grid_id
			).c_str()
		);
	} else if (strcasecmp("delete", command_type) == 0) {
		auto grid_id = atoi(sep->arg[2]);
		content_db.ModifyGrid(c, true, grid_id, 0, 0, zone_id);
		c->Message(
			Chat::White,
			fmt::format(
				"Grid {} deleted from zone ID {}.",
				grid_id,
				zone_id
			).c_str()
		);
	} else {
		c->Message(Chat::White, "Usage: #grid [add|delete] [grid_id] [wander_type] [pause_type]");
		c->Message(Chat::White, "Usage: #grid [max] - displays the highest grid ID used in this zone (for add)");
		c->Message(Chat::White, "Usage: #grid [show] - displays wp nodes as boxes");
	}
}

void command_wp(Client *c, const Seperator *sep)
{
	auto command_type = sep->arg[1];
	auto grid_id = atoi(sep->arg[2]);
	if (grid_id != 0) {
		auto pause = atoi(sep->arg[3]);
		auto waypoint = atoi(sep->arg[4]);
		auto zone_id = zone->GetZoneID();
		if (strcasecmp("add", command_type) == 0) {
			if (waypoint == 0) { // Default to highest if it's left blank, or we enter 0
				waypoint = (content_db.GetHighestWaypoint(zone_id, grid_id)  + 1);
			}

			if (strcasecmp("-h", sep->arg[5]) == 0) {
				content_db.AddWP(c, grid_id, waypoint, c->GetPosition(), pause, zone_id);
			} else {
    	        auto position = c->GetPosition();
    	        position.w = -1;
				content_db.AddWP(c, grid_id, waypoint, position, pause, zone_id);
			}
			c->Message(
				Chat::White,
				fmt::format(
					"Waypoint {} added to grid {} with a pause of {} {}.",
					waypoint,
					grid_id,
					pause,
					(pause == 1 ? "second" : "seconds")
				).c_str()
			);
		} else if (strcasecmp("delete", command_type) == 0) {
			content_db.DeleteWaypoint(c, grid_id, waypoint, zone_id);
			c->Message(
				Chat::White,
				fmt::format(
					"Waypoint {} deleted from grid {}.",
					waypoint,
					grid_id
				).c_str()
			);
		}
	} else {
		c->Message(Chat::White,"Usage: #wp [add|delete] [grid_id] [pause] [waypoint_id] [-h]");
	}
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
		c->Message(Chat::White, "Usage: #size [0 - 255] (Decimal increments are allowed)");
	else {
		float newsize = atof(sep->arg[1]);
		if (newsize > 255)
			c->Message(Chat::White, "Error: #size: Size can not be greater than 255.");
		else if (newsize < 0)
			c->Message(Chat::White, "Error: #size: Size can not be less than 0.");
		else if (!target)
			c->Message(Chat::White,"Error: this command requires a target");
		else {
			uint16 Race = target->GetModel();
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

			c->Message(Chat::White,"Size = %f",  atof(sep->arg[1]));
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
	Mob *t = c;

	if (strlen(sep->arg[1]) == 1 && sep->IsNumber(1) && atoi(sep->arg[1]) >= 0 && atoi(sep->arg[1]) <= 5) {
		if (c->GetTarget()) {
			t = c->GetTarget();
		}

		int fm = atoi(sep->arg[1]);

		t->SetFlyMode(static_cast<GravityBehavior>(fm));
		t->SendAppearancePacket(AT_Levitate, fm);
		if (sep->arg[1][0] == '0') {
			c->Message(Chat::White, "Setting %s to Grounded", t->GetName());
		}
		else if (sep->arg[1][0] == '1') {
			c->Message(Chat::White, "Setting %s to Flying", t->GetName());
		}
		else if (sep->arg[1][0] == '2') {
			c->Message(Chat::White, "Setting %s to Levitating", t->GetName());
		}
		else if (sep->arg[1][0] == '3') {
			c->Message(Chat::White, "Setting %s to In Water", t->GetName());
		}
		else if (sep->arg[1][0] == '4') {
			c->Message(Chat::White, "Setting %s to Floating(Boat)", t->GetName());
		}
		else if (sep->arg[1][0] == '5') {
			c->Message(Chat::White, "Setting %s to Levitating While Running", t->GetName());
		}
	} else {
		c->Message(Chat::White, "#flymode [0/1/2/3/4/5]");
	}
}


void command_showskills(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	c->Message(Chat::White, "Skills for %s",  t->GetName());
	for (EQ::skills::SkillType i = EQ::skills::Skill1HBlunt; i <= EQ::skills::HIGHEST_SKILL; i = (EQ::skills::SkillType)(i + 1))
		c->Message(Chat::White, "Skill [%d] is at [%d] - %u",  i, t->GetSkill(i), t->GetRawSkill(i));
}

void command_findclass(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #findclass [search criteria]");
	} else if (Seperator::IsNumber(sep->argplus[1])) {
		int search_id = atoi(sep->argplus[1]);
		std::string class_name = GetClassIDName(search_id);
		if (class_name.length() > 0) {
			c->Message(
				Chat::White,
				fmt::format(
					"Class {}: {}",
					search_id,
					class_name
				).c_str()
			);
			return;
		}
	} else {
		std::string search_criteria = str_tolower(sep->argplus[1]);
		int found_count = 0;
		for (int class_id = WARRIOR; class_id <= MERCERNARY_MASTER; class_id++) {
			std::string class_name = GetClassIDName(class_id);
			std::string class_name_lower = str_tolower(class_name);
			if (search_criteria.length() > 0 && class_name_lower.find(search_criteria) == std::string::npos) {
				continue;
			}

			c->Message(
				Chat::White,
				fmt::format(
					"Class {}: {}",
					class_id,
					class_name
				).c_str()
			);
			found_count++;

			if (found_count == 20) {
				break;
			}
		}

		if (found_count == 20) {
			c->Message(Chat::White, "20 Classes found... max reached.");
		} else {
			c->Message(
				Chat::White,
				fmt::format(
					"{} Class(es) found.",
					found_count
				).c_str()
			);
		}
	}
}

void command_findrace(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #findrace [search criteria]");
	} else if (Seperator::IsNumber(sep->argplus[1])) {
		int search_id = atoi(sep->argplus[1]);
		std::string race_name = GetRaceIDName(search_id);
		if (race_name.length() > 0) {
			c->Message(
				Chat::White,
				fmt::format(
					"Race {}: {}",
					search_id,
					race_name
				).c_str()
			);
			return;
		}
	} else {
		std::string search_criteria = str_tolower(sep->argplus[1]);
		int found_count = 0;
		for (int race_id = RACE_HUMAN_1; race_id <= RT_PEGASUS_3; race_id++) {
			std::string race_name = GetRaceIDName(race_id);
			std::string race_name_lower = str_tolower(race_name);
			if (search_criteria.length() > 0 && race_name_lower.find(search_criteria) == std::string::npos) {
				continue;
			}

			c->Message(
				Chat::White,
				fmt::format(
					"Race {}: {}",
					race_id,
					race_name
				).c_str()
			);
			found_count++;

			if (found_count == 20) {
				break;
			}
		}
		if (found_count == 20) {
			c->Message(Chat::White, "20 Races found... max reached.");
		} else {
			c->Message(
				Chat::White,
				fmt::format(
					"{} Race(s) found.",
					found_count
				).c_str()
			);
		}
	}
}

void command_findspell(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #findspell [search criteria]");
	} else if (SPDAT_RECORDS <= 0) {
		c->Message(Chat::White, "Spells not loaded");
	} else if (Seperator::IsNumber(sep->argplus[1])) {
		int spell_id = atoi(sep->argplus[1]);
		if (!IsValidSpell(spell_id)) {
			c->Message(Chat::White, "Error: Invalid Spell");
		} else {
			c->Message(
				Chat::White,
				fmt::format(
					"{}: {}",
					spell_id,
					spells[spell_id].name
				).c_str()
			);
		}
	}
	else {
		std::string search_criteria = str_tolower(sep->argplus[1]);
		int found_count = 0;
		for (int i = 0; i < SPDAT_RECORDS; i++) {
			auto current_spell = spells[i];
			if (current_spell.name[0] != 0) {
				std::string spell_name = current_spell.name;
				std::string spell_name_lower = str_tolower(spell_name);
				if (search_criteria.length() > 0 && spell_name_lower.find(search_criteria) == std::string::npos) {
					continue;
				}

				c->Message(
					Chat::White,
					fmt::format(
						"{}: {}",
						i,
						spell_name
					).c_str()
				);
				found_count++;

				if (found_count == 20) {
					break;
				}
			}
		}

		if (found_count == 20) {
			c->Message(Chat::White, "20 Spells found... max reached.");
		} else {
			c->Message(
				Chat::White,
				fmt::format(
					"{} Spell(s) found.",
					found_count
				).c_str()
			);
		}
	}
}

inline bool CastRestrictedSpell(int spellid)
{
	switch (spellid) {
		case SPELL_TOUCH_OF_VINITRAS:
		case SPELL_DESPERATE_HOPE:
		case SPELL_CHARM:
		case SPELL_METAMORPHOSIS65:
		case SPELL_JT_BUFF:
		case SPELL_CAN_O_WHOOP_ASS:
		case SPELL_PHOENIX_CHARM:
		case SPELL_CAZIC_TOUCH:
		case SPELL_AVATAR_KNOCKBACK:
		case SPELL_SHAPECHANGE65:
		case SPELL_SUNSET_HOME1218:
		case SPELL_SUNSET_HOME819:
		case SPELL_SHAPECHANGE75:
		case SPELL_SHAPECHANGE80:
		case SPELL_SHAPECHANGE85:
		case SPELL_SHAPECHANGE90:
		case SPELL_SHAPECHANGE95:
		case SPELL_SHAPECHANGE100:
		case SPELL_SHAPECHANGE25:
		case SPELL_SHAPECHANGE30:
		case SPELL_SHAPECHANGE35:
		case SPELL_SHAPECHANGE40:
		case SPELL_SHAPECHANGE45:
		case SPELL_SHAPECHANGE50:
		case SPELL_NPC_AEGOLISM:
		case SPELL_SHAPECHANGE55:
		case SPELL_SHAPECHANGE60:
		case SPELL_COMMAND_OF_DRUZZIL:
		case SPELL_SHAPECHANGE70:
			return true;
		default:
			return false;
	}
}

void command_castspell(Client *c, const Seperator *sep)
{
	if (!sep->IsNumber(1))
		c->Message(Chat::White, "Usage: #CastSpell spellid");
	else {
		uint16 spellid = atoi(sep->arg[1]);
		/*
		Spell restrictions.
		*/
		if (CastRestrictedSpell(spellid) && c->Admin() < commandCastSpecials)
			c->Message(Chat::Red, "Unable to cast spell.");
		else if (spellid >= SPDAT_RECORDS)
			c->Message(Chat::White, "Error: #CastSpell: Argument out of range");
		else
			if (c->GetTarget() == 0)
				if(c->Admin() >= commandInstacast)
					c->SpellFinished(spellid, 0, EQ::spells::CastingSlot::Item, 0, -1, spells[spellid].ResistDiff);
				else
					c->CastSpell(spellid, 0, EQ::spells::CastingSlot::Item, 0);
			else
				if(c->Admin() >= commandInstacast)
					c->SpellFinished(spellid, c->GetTarget(), EQ::spells::CastingSlot::Item, 0, -1, spells[spellid].ResistDiff);
				else
					c->CastSpell(spellid, c->GetTarget()->GetID(), EQ::spells::CastingSlot::Item, 0);
	}
}

void command_setlanguage(Client *c, const Seperator *sep)
{
	if (strcasecmp(sep->arg[1], "list" ) == 0 )
	{
		c->Message(Chat::White, "Languages:");
		c->Message(Chat::White, "(0) Common Tongue");
		c->Message(Chat::White, "(1) Barbarian");
		c->Message(Chat::White, "(2) Erudian");
		c->Message(Chat::White, "(3) Elvish");
		c->Message(Chat::White, "(4) Dark Elvish");
		c->Message(Chat::White, "(5) Dwarvish");
		c->Message(Chat::White, "(6) Troll");
		c->Message(Chat::White, "(7) Ogre");
		c->Message(Chat::White, "(8) Gnomish");
		c->Message(Chat::White, "(9) Halfling");
		c->Message(Chat::White, "(10) Thieves Cant");
		c->Message(Chat::White, "(11) Old Erudian");
		c->Message(Chat::White, "(12) Elder Elvish");
		c->Message(Chat::White, "(13) Froglok");
		c->Message(Chat::White, "(14) Goblin");
		c->Message(Chat::White, "(15) Gnoll");
		c->Message(Chat::White, "(16) Combine Tongue");
		c->Message(Chat::White, "(17) Elder Teir`Dal");
		c->Message(Chat::White, "(18) Lizardman");
		c->Message(Chat::White, "(19) Orcish");
		c->Message(Chat::White, "(20) Faerie");
		c->Message(Chat::White, "(21) Dragon");
		c->Message(Chat::White, "(22) Elder Dragon");
		c->Message(Chat::White, "(23) Dark Speech");
		c->Message(Chat::White, "(24) Vah Shir");
		c->Message(Chat::White, "(25) Alaran");
		c->Message(Chat::White, "(26) Hadal");
		c->Message(Chat::White, "(27) Unknown1");
	}
	else if( c->GetTarget() == 0 )
	{
		c->Message(Chat::White, "Error: #setlanguage: No target.");
	}
	else if( !c->GetTarget()->IsClient() )
	{
		c->Message(Chat::White, "Error: Target must be a player.");
	}
	else if (
				!sep->IsNumber(1) || atoi(sep->arg[1]) < 0 || atoi(sep->arg[1]) > 27 ||
				!sep->IsNumber(2) || atoi(sep->arg[2]) < 0 || atoi(sep->arg[2]) > 100
			)
	{
		c->Message(Chat::White, "Usage: #setlanguage [language ID] [value] (0-27, 0-100)");
		c->Message(Chat::White, "Try #setlanguage list for a list of language IDs");
	}
	else
	{
		LogInfo("Set language request from [{}], target:[{}] lang_id:[{}] value:[{}]",  c->GetName(), c->GetTarget()->GetName(), atoi(sep->arg[1]), atoi(sep->arg[2]) );
		uint8 langid = (uint8)atoi(sep->arg[1]);
		uint8 value = (uint8)atoi(sep->arg[2]);
		c->GetTarget()->CastToClient()->SetLanguageSkill( langid, value );
	}
}

void command_setskill(Client *c, const Seperator *sep)
{
	if (c->GetTarget() == nullptr) {
		c->Message(Chat::White, "Error: #setskill: No target.");
	}
	else if (!c->GetTarget()->IsClient()) {
		c->Message(Chat::White, "Error: #setskill: Target must be a client.");
	}
	else if (
		!sep->IsNumber(1) || atoi(sep->arg[1]) < 0 || atoi(sep->arg[1]) > EQ::skills::HIGHEST_SKILL ||
						!sep->IsNumber(2) || atoi(sep->arg[2]) < 0 || atoi(sep->arg[2]) > HIGHEST_CAN_SET_SKILL
					)
	{
		c->Message(Chat::White, "Usage: #setskill skill x ");
		c->Message(Chat::White, "       skill = 0 to %d", EQ::skills::HIGHEST_SKILL);
		c->Message(Chat::White, "       x = 0 to %d",  HIGHEST_CAN_SET_SKILL);
	}
	else {
		LogInfo("Set skill request from [{}], target:[{}] skill_id:[{}] value:[{}]",  c->GetName(), c->GetTarget()->GetName(), atoi(sep->arg[1]), atoi(sep->arg[2]) );
		int skill_num = atoi(sep->arg[1]);
		uint16 skill_value = atoi(sep->arg[2]);
		if (skill_num <= EQ::skills::HIGHEST_SKILL)
			c->GetTarget()->CastToClient()->SetSkill((EQ::skills::SkillType)skill_num, skill_value);
	}
}

void command_setskillall(Client *c, const Seperator *sep)
{
	if (c->GetTarget() == 0)
		c->Message(Chat::White, "Error: #setallskill: No target.");
	else if (!c->GetTarget()->IsClient())
		c->Message(Chat::White, "Error: #setskill: Target must be a client.");
	else if (!sep->IsNumber(1) || atoi(sep->arg[1]) < 0 || atoi(sep->arg[1]) > HIGHEST_CAN_SET_SKILL) {
		c->Message(Chat::White, "Usage: #setskillall value ");
		c->Message(Chat::White, "       value = 0 to %d",  HIGHEST_CAN_SET_SKILL);
	}
	else {
		if (c->Admin() >= commandSetSkillsOther || c->GetTarget()==c || c->GetTarget()==0) {
			LogInfo("Set ALL skill request from [{}], target:[{}]",  c->GetName(), c->GetTarget()->GetName());
			uint16 level = atoi(sep->arg[1]);
			for (EQ::skills::SkillType skill_num = EQ::skills::Skill1HBlunt; skill_num <= EQ::skills::HIGHEST_SKILL; skill_num = (EQ::skills::SkillType)(skill_num + 1)) {
				c->GetTarget()->CastToClient()->SetSkill(skill_num, level);
			}
		}
		else
			c->Message(Chat::White, "Error: Your status is not high enough to set anothers skills");
	}
}

void command_race(Client *c, const Seperator *sep)
{
	Mob *target = c->CastToMob();

	if (sep->IsNumber(1)) {
		auto race = atoi(sep->arg[1]);
		if ((race >= 0 && race <= 732) || (race >= 2253 && race <= 2259)) {
			if ((c->GetTarget()) && c->Admin() >= commandRaceOthers) {
				target = c->GetTarget();
			}
			target->SendIllusionPacket(race);
		}
		else {
			c->Message(Chat::White, "Usage: #race [0-732, 2253-2259] (0 for back to normal)");
		}
	}
	else {
		c->Message(Chat::White, "Usage: #race [0-732, 2253-2259] (0 for back to normal)");
	}
}

void command_gearup(Client *c, const Seperator *sep)
{
	std::string tool_table_name = "tool_gearup_armor_sets";
	if (!database.DoesTableExist(tool_table_name)) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"Table [{}] does not exist. Downloading from Github and installing...",
				tool_table_name
			).c_str()
		);

		// http get request
		httplib::Client cli("https://raw.githubusercontent.com");
		cli.set_connection_timeout(0, 15000000); // 15 sec
		cli.set_read_timeout(15, 0); // 15 seconds
		cli.set_write_timeout(15, 0); // 15 seconds

		int         sourced_queries = 0;
		std::string url             = "/EQEmu/Server/master/utils/sql/git/optional/2020_07_20_tool_gearup_armor_sets.sql";

		if (auto res = cli.Get(url.c_str())) {
			if (res->status == 200) {
				for (auto &s: SplitString(res->body, ';')) {
					if (!trim(s).empty()) {
						auto results = database.QueryDatabase(s);
						if (!results.ErrorMessage().empty()) {
							c->Message(
								Chat::Yellow,
								fmt::format(
									"Error sourcing SQL [{}]", results.ErrorMessage()
								).c_str()
							);
							return;
						}
						sourced_queries++;
					}
				}
			}
		}
		else {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"Error retrieving URL [{}]",
					url
				).c_str()
			);
		}

		c->Message(
			Chat::Yellow,
			fmt::format(
				"Table [{}] installed. Sourced [{}] queries",
				tool_table_name, sourced_queries
			).c_str()
		);
	}

	std::string expansion_arg = sep->arg[1];
	std::string expansion_filter;
	if (expansion_arg.length() > 0) {
		expansion_filter = fmt::format("and `expansion` = {}", expansion_arg);
	}

	auto results = database.QueryDatabase(
		fmt::format(
			SQL (
				select
				item_id,
				slot
				from
				{}
				where
				`class` = {}
				and `level` = {}
				{}
				order by score desc, expansion desc
			),
			tool_table_name,
			c->GetClass(),
			c->GetLevel(),
			expansion_filter
		)
	);

	int           items_equipped     = 0;
	int           items_already_have = 0;
	std::set<int> equipped;

	for (auto row = results.begin(); row != results.end(); ++row) {
		int item_id = atoi(row[0]);
		int slot_id = atoi(row[1]);

		if (equipped.find(slot_id) != equipped.end()) {
			if (slot_id == EQ::invslot::slotEar1) {
				slot_id = EQ::invslot::slotEar2;
			}
			if (slot_id == EQ::invslot::slotFinger1) {
				slot_id = EQ::invslot::slotFinger2;
			}
			if (slot_id == EQ::invslot::slotWrist1) {
				slot_id = EQ::invslot::slotWrist2;
			}
		}

		if (equipped.find(slot_id) == equipped.end()) {
			const EQ::ItemData *item         = database.GetItem(item_id);
			bool               has_item      = (c->GetInv().HasItem(item_id, 1, invWhereWorn) != INVALID_INDEX);
			bool               can_wear_item = !c->CheckLoreConflict(item) && !has_item;
			if (!can_wear_item) {
				items_already_have++;
			}

			if (c->CastToMob()->CanClassEquipItem(item_id) && can_wear_item) {
				equipped.insert(slot_id);
				c->SummonItem(
					item_id,
					0, 0, 0, 0, 0, 0, 0, 0,
					slot_id
				);
				items_equipped++;
			}
		}
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Equipped items [{}] already had [{}] items equipped",
			items_equipped,
			items_already_have
		).c_str()
	);

	if (expansion_arg.empty()) {
		results = database.QueryDatabase(
			fmt::format(
				SQL (
					select
					expansion
					from
					{}
					where
					class = {}
					and level = {}
					group by
					expansion;
				),
				tool_table_name,
				c->GetClass(),
				c->GetLevel()
			)
		);

		c->Message(Chat::White, "Choose armor from a specific era");
		std::string message;
		for (auto   row = results.begin(); row != results.end(); ++row) {
			int expansion = atoi(row[0]);
			message += "[" + EQ::SayLinkEngine::GenerateQuestSaylink(
				fmt::format("#gearup {}", expansion),
				false,
				Expansion::ExpansionName[expansion]
			) + "] ";

			if (message.length() > 2000) {
				c->Message(Chat::White, message.c_str());
				message = "";
			}
		}
		if (message.length() > 0) {
			c->Message(Chat::White, message.c_str());
		}
	}

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
		c->Message(Chat::White, "Usage: #gender [0/1/2]");
}

void command_makepet(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == '\0')
		c->Message(Chat::White, "Usage: #makepet pet_type_name (will not survive across zones)");
	else
		c->MakePet(0, sep->arg[1]);
}

void command_level(Client *c, const Seperator *sep)
{
	uint16 level = atoi(sep->arg[1]);

	if ((level <= 0) || ((level > RuleI(Character, MaxLevel)) && (c->Admin() < commandLevelAboveCap))) {
		c->Message(Chat::White, "Error: #Level: Invalid Level");
	}
	else if (c->Admin() < RuleI(GM, MinStatusToLevelTarget)) {
		c->SetLevel(level, true);
#ifdef BOTS
		if(RuleB(Bots, BotLevelsWithOwner))
			Bot::LevelBotWithClient(c, level, true);
#endif
	}
	else if (!c->GetTarget()) {
		c->Message(Chat::White, "Error: #Level: No target");
	}
	else {
		if (!c->GetTarget()->IsNPC() && ((c->Admin() < commandLevelNPCAboveCap) && (level > RuleI(Character, MaxLevel)))) {
			c->Message(Chat::White, "Error: #Level: Invalid Level");
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

void command_spawneditmass(Client *c, const Seperator *sep)
{
	std::string query = fmt::format(
		SQL(
			SELECT
			npc_types.id,
			npc_types.name,
			spawn2.respawntime,
			spawn2.id
				FROM
					npc_types
				JOIN spawnentry ON spawnentry.npcID = npc_types.id
				JOIN spawn2 ON spawn2.spawngroupID = spawnentry.spawngroupID
				WHERE
				spawn2.zone = '{0}' and spawn2.version = {1}
				GROUP BY npc_types.id
		),
		zone->GetShortName(),
		zone->GetInstanceVersion()
	);

	std::string status = "(Searching)";

	if (strcasecmp(sep->arg[4], "apply") == 0) {
		status = "(Applying)";
	}

	std::string search_value;
	std::string edit_option;
	std::string edit_value;
	std::string apply_set;

	if (sep->arg[1]) {
		search_value = sep->arg[1];
	}

	if (sep->arg[2]) {
		edit_option = sep->arg[2];
	}

	if (sep->arg[3]) {
		edit_value = sep->arg[3];
	}

	if (sep->arg[4]) {
		apply_set = sep->arg[4];
	}

	if (!edit_option.empty() && edit_value.empty()) {
		c->Message(Chat::Yellow, "Please specify an edit option value | #npceditmass <search> <option> <value>");
		return;
	}

	std::vector<std::string> npc_ids;
	std::vector<std::string> spawn2_ids;

	int  found_count = 0;
	auto results     = database.QueryDatabase(query);

	for (auto row = results.begin(); row != results.end(); ++row) {

		std::string npc_id       = row[0];
		std::string npc_name     = row[1];
		std::string respawn_time = row[2];
		std::string spawn2_id    = row[3];

		if (npc_name.find(search_value) == std::string::npos) {
			continue;
		}

		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ({0}) [{1}] respawn_time [{2}] {3}",
				npc_id,
				npc_name,
				respawn_time,
				status
			).c_str()
		);

		npc_ids.push_back(npc_id);
		spawn2_ids.push_back(spawn2_id);

		found_count++;
	}

	c->Message(Chat::Yellow, "Found [%i] NPC Spawn2 entries that match this criteria in this zone", found_count);
	if (edit_option.empty()) {
		c->Message(Chat::Yellow, "Please specify an edit option | #npceditmass <search> <option>");
		c->Message(Chat::Yellow, "Options [respawn_time]");
		return;
	}

	std::string saylink = fmt::format(
		"#spawneditmass {} {} {} apply",
		search_value,
		edit_option,
		edit_value
	);

	if (found_count > 0) {
		c->Message(
			Chat::Yellow, "To apply these changes, click <%s> or type [%s]",
			EQ::SayLinkEngine::GenerateQuestSaylink(saylink, false, "Apply").c_str(),
			saylink.c_str()
		);
	}

	if (edit_option == "respawn_time" && apply_set == "apply") {
		std::string spawn2_ids_string = implode(",", spawn2_ids);
		if (spawn2_ids_string.empty()) {
			c->Message(Chat::Red, "Error: Ran into an unknown error compiling Spawn2 IDs");
			return;
		}

		database.QueryDatabase(
			fmt::format(
				SQL(
					UPDATE spawn2 SET respawntime = {} WHERE id IN({})
				),
				std::stoi(edit_value),
				spawn2_ids_string
			)
		);

		c->Message(Chat::Yellow, "Updated [%i] spawns", found_count);
	}
}

void command_spawn(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] != 0){
		Client* client = entity_list.GetClientByName(sep->arg[1]);
		if(client){
				c->Message(Chat::White,"You cannot spawn a mob with the same name as a character!");
				return;
		}
	}

	NPC* npc = NPC::SpawnNPC(sep->argplus[1], c->GetPosition(), c);
	if (!npc) {
		c->Message(Chat::White, "Format: #spawn name race level material hp gender class priweapon secweapon merchantid bodytype - spawns a npc those parameters.");
		c->Message(Chat::White, "Name Format: NPCFirstname_NPCLastname - All numbers in a name are stripped and \"_\" characters become a space.");
		c->Message(Chat::White, "Note: Using \"-\" for gender will autoselect the gender for the race. Using \"-\" for HP will use the calculated maximum HP.");
	}
}

void command_test(Client *c, const Seperator *sep)
{
	c->Message(Chat::Yellow, "Triggering test command");

	if (sep->arg[1]) {
		c->SetPrimaryWeaponOrnamentation(atoi(sep->arg[1]));
	}
	if (sep->arg[2]) {
		c->SetSecondaryWeaponOrnamentation(atoi(sep->arg[2]));
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
			for (i = EQ::textures::textureBegin; i <= EQ::textures::LastTintableTexture; i++)
			{
				c->SendTextureWC(i, texture);
			}
		else if ((c->GetTarget()->GetModel() > 0 && c->GetTarget()->GetModel() <= 12) ||
			c->GetTarget()->GetModel() == 128 || c->GetTarget()->GetModel() == 130 ||
			c->GetTarget()->GetModel() == 330 || c->GetTarget()->GetModel() == 522) {
			for (i = EQ::textures::textureBegin; i <= EQ::textures::LastTintableTexture; i++)
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
				c->GetTarget()->SendIllusionPacket(c->GetTarget()->GetModel(), 0xFF, texture, helm);
			else
				c->SendIllusionPacket(c->GetRace(), 0xFF, texture, helm);
		}
	}
	else
		c->Message(Chat::White, "Usage: #texture [texture] [helmtexture] (0-255, 255 for show equipment)");
}

void command_npctypespawn(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(1)) {
		const NPCType* tmp = 0;
		if ((tmp = content_db.LoadNPCTypesData(atoi(sep->arg[1])))) {
			//tmp->fixedZ = 1;
			auto npc = new NPC(tmp, 0, c->GetPosition(), GravityBehavior::Water);
			if (npc && sep->IsNumber(2))
				npc->SetNPCFactionID(atoi(sep->arg[2]));

			npc->AddLootTable();
			if (npc->DropsGlobalLoot())
				npc->CheckGlobalLootTables();
			entity_list.AddNPC(npc);
		}
		else
			c->Message(Chat::White, "NPC Type %i not found",  atoi(sep->arg[1]));
	}
	else
		c->Message(Chat::White, "Usage: #npctypespawn npctypeid factionid");

}

void command_nudge(Client* c, const Seperator* sep)
{
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #nudge [x=f] [y=f] [z=f] [h=f] (partial/mixed arguments allowed)");
	}
	else {

		auto target = c->GetTarget();
		if (!target) {

			c->Message(Chat::Yellow, "This command requires a target.");
			return;
		}
		if (target->IsMoving()) {

			c->Message(Chat::Yellow, "This command requires a stationary target.");
			return;
		}

		glm::vec4 position_offset(0.0f, 0.0f, 0.0f, 0.0f);
		for (auto index = 1; index <= 4; ++index) {

			if (!sep->arg[index]) {
				continue;
			}

			Seperator argsep(sep->arg[index], '=');
			if (!argsep.arg[1][0]) {
				continue;
			}

			switch (argsep.arg[0][0]) {
			case 'x':
				position_offset.x = atof(argsep.arg[1]);
				break;
			case 'y':
				position_offset.y = atof(argsep.arg[1]);
				break;
			case 'z':
				position_offset.z = atof(argsep.arg[1]);
				break;
			case 'h':
				position_offset.w = atof(argsep.arg[1]);
				break;
			default:
				break;
			}
		}

		const auto& current_position = target->GetPosition();
		glm::vec4 new_position(
			(current_position.x + position_offset.x),
			(current_position.y + position_offset.y),
			(current_position.z + position_offset.z),
			(current_position.w + position_offset.w)
		);

		target->GMMove(new_position.x, new_position.y, new_position.z, new_position.w);

		c->Message(
			Chat::White,
			"Nudging '%s' to {%1.3f, %1.3f, %1.3f, %1.2f} (adjustment: {%1.3f, %1.3f, %1.3f, %1.2f})",
			target->GetName(),
			new_position.x,
			new_position.y,
			new_position.z,
			new_position.w,
			position_offset.x,
			position_offset.y,
			position_offset.z,
			position_offset.w
		);
	}
}

void command_heal(Client *c, const Seperator *sep)
{
	if (c->GetTarget()==0)
		c->Message(Chat::White, "Error: #Heal: No Target.");
	else
		c->GetTarget()->Heal();
}

void command_appearance(Client *c, const Seperator *sep)
{
	Mob *t=c->CastToMob();

	// sends any appearance packet
	// Dev debug command, for appearance types
	if (sep->arg[2][0] == 0)
		c->Message(Chat::White, "Usage: #appearance type value");
	else {
		if ((c->GetTarget()))
			t=c->GetTarget();
		t->SendAppearancePacket(atoi(sep->arg[1]), atoi(sep->arg[2]));
		c->Message(Chat::White, "Sending appearance packet: target=%s, type=%s, value=%s",  t->GetName(), sep->arg[1], sep->arg[2]);
	}
}

void command_nukeitem(Client *c, const Seperator *sep)
{
	int numitems, itemid;

	if (c->GetTarget() && c->GetTarget()->IsClient() && (sep->IsNumber(1) || sep->IsHexNumber(1))) {
		itemid=sep->IsNumber(1)?atoi(sep->arg[1]):hextoi(sep->arg[1]);
		numitems = c->GetTarget()->CastToClient()->NukeItem(itemid);
		c->Message(Chat::White, " %u items deleted",  numitems);
	}
	else
		c->Message(Chat::White, "Usage: (targted) #nukeitem itemnum - removes the item from the player's inventory");
}

void command_peekinv(Client *c, const Seperator *sep)
{
	// this can be cleaned up once inventory is cleaned up
	enum {
		peekNone = 0x0000,
		peekEquip = 0x0001,
		peekGen = 0x0002,
		peekCursor = 0x0004,
		peekLimbo = 0x0008,
		peekTrib = 0x0010,
		peekBank = 0x0020,
		peekShBank = 0x0040,
		peekTrade = 0x0080,
		peekWorld = 0x0100,
		peekOutOfScope = (peekWorld * 2) // less than
	};

	static const char* scope_prefix[] = { "equip", "gen", "cursor", "limbo", "trib", "bank", "shbank", "trade", "world" };

	static const int16 scope_range[][2] = {
		{ EQ::invslot::EQUIPMENT_BEGIN, EQ::invslot::EQUIPMENT_END },
		{ EQ::invslot::GENERAL_BEGIN, EQ::invslot::GENERAL_END },
		{ EQ::invslot::slotCursor, EQ::invslot::slotCursor },
		{ EQ::invslot::SLOT_INVALID, EQ::invslot::SLOT_INVALID },
		{ EQ::invslot::TRIBUTE_BEGIN, EQ::invslot::TRIBUTE_END },
		{ EQ::invslot::BANK_BEGIN, EQ::invslot::BANK_END },
		{ EQ::invslot::SHARED_BANK_BEGIN, EQ::invslot::SHARED_BANK_END },
		{ EQ::invslot::TRADE_BEGIN, EQ::invslot::TRADE_END },
		{ EQ::invslot::SLOT_BEGIN, (EQ::invtype::WORLD_SIZE - 1) }
	};

	static const bool scope_bag[] = { false, true, true, true, false, true, true, true, true };

	if (!c)
		return;

	if (c->GetTarget() && !c->GetTarget()->IsClient()) {
		c->Message(Chat::White, "You must target a PC for this command.");
		return;
	}

	int scopeMask = peekNone;

	if (strcasecmp(sep->arg[1], "all") == 0) { scopeMask = (peekOutOfScope - 1); }
	else if (strcasecmp(sep->arg[1], "equip") == 0) { scopeMask |= peekEquip; }
	else if (strcasecmp(sep->arg[1], "gen") == 0) { scopeMask |= peekGen; }
	else if (strcasecmp(sep->arg[1], "cursor") == 0) { scopeMask |= peekCursor; }
	else if (strcasecmp(sep->arg[1], "poss") == 0) { scopeMask |= (peekEquip | peekGen | peekCursor); }
	else if (strcasecmp(sep->arg[1], "limbo") == 0) { scopeMask |= peekLimbo; }
	else if (strcasecmp(sep->arg[1], "curlim") == 0) { scopeMask |= (peekCursor | peekLimbo); }
	else if (strcasecmp(sep->arg[1], "trib") == 0) { scopeMask |= peekTrib; }
	else if (strcasecmp(sep->arg[1], "bank") == 0) { scopeMask |= peekBank; }
	else if (strcasecmp(sep->arg[1], "shbank") == 0) { scopeMask |= peekShBank; }
	else if (strcasecmp(sep->arg[1], "allbank") == 0) { scopeMask |= (peekBank | peekShBank); }
	else if (strcasecmp(sep->arg[1], "trade") == 0) { scopeMask |= peekTrade; }
	else if (strcasecmp(sep->arg[1], "world") == 0) { scopeMask |= peekWorld; }

	if (!scopeMask) {
		c->Message(Chat::White, "Usage: #peekinv [equip|gen|cursor|poss|limbo|curlim|trib|bank|shbank|allbank|trade|world|all]");
		c->Message(Chat::White, "- Displays a portion of the targeted user's inventory");
		c->Message(Chat::White, "- Caution: 'all' is a lot of information!");
		return;
	}

	Client* targetClient = c;
	if (c->GetTarget())
		targetClient = c->GetTarget()->CastToClient();

	const EQ::ItemInstance* inst_main = nullptr;
	const EQ::ItemInstance* inst_sub = nullptr;
	const EQ::ItemInstance* inst_aug = nullptr;
	const EQ::ItemData* item_data = nullptr;

	EQ::SayLinkEngine linker;
	linker.SetLinkType(EQ::saylink::SayLinkItemInst);

	c->Message(Chat::White, "Displaying inventory for %s...", targetClient->GetName());

	Object* objectTradeskill = targetClient->GetTradeskillObject();

	bool itemsFound = false;

	for (int scopeIndex = 0, scopeBit = peekEquip; scopeBit < peekOutOfScope; ++scopeIndex, scopeBit <<= 1) {
		if (scopeBit & ~scopeMask)
			continue;

		if (scopeBit & peekWorld) {
			if (objectTradeskill == nullptr) {
				c->Message(Chat::Default, "No world tradeskill object selected...");
				continue;
			}
			else {
				c->Message(Chat::White, "[WorldObject DBID: %i (entityid: %i)]", objectTradeskill->GetDBID(), objectTradeskill->GetID());
			}
		}

		for (int16 indexMain = scope_range[scopeIndex][0]; indexMain <= scope_range[scopeIndex][1]; ++indexMain) {
			if (indexMain == EQ::invslot::SLOT_INVALID)
				continue;

			inst_main = ((scopeBit & peekWorld) ? objectTradeskill->GetItem(indexMain) : targetClient->GetInv().GetItem(indexMain));
			if (inst_main) {
				itemsFound = true;
				item_data = inst_main->GetItem();
			}
			else {
				item_data = nullptr;
			}

			linker.SetItemInst(inst_main);

			c->Message(
				(item_data == nullptr),
				"%sSlot: %i, Item: %i (%s), Charges: %i",
				scope_prefix[scopeIndex],
				((scopeBit & peekWorld) ? (EQ::invslot::WORLD_BEGIN + indexMain) : indexMain),
				((item_data == nullptr) ? 0 : item_data->ID),
				linker.GenerateLink().c_str(),
				((inst_main == nullptr) ? 0 : inst_main->GetCharges())
			);

			if (inst_main && inst_main->IsClassCommon()) {
				for (uint8 indexAug = EQ::invaug::SOCKET_BEGIN; indexAug <= EQ::invaug::SOCKET_END; ++indexAug) {
					inst_aug = inst_main->GetItem(indexAug);
					if (!inst_aug) // extant only
						continue;

					item_data = inst_aug->GetItem();
					linker.SetItemInst(inst_aug);

					c->Message(
						(item_data == nullptr),
						".%sAugSlot: %i (Slot #%i, Aug idx #%i), Item: %i (%s), Charges: %i",
						scope_prefix[scopeIndex],
						INVALID_INDEX,
						((scopeBit & peekWorld) ? (EQ::invslot::WORLD_BEGIN + indexMain) : indexMain),
						indexAug,
						((item_data == nullptr) ? 0 : item_data->ID),
						linker.GenerateLink().c_str(),
						((inst_sub == nullptr) ? 0 : inst_sub->GetCharges())
					);
				}
			}

			if (!scope_bag[scopeIndex] || !(inst_main && inst_main->IsClassBag()))
				continue;

			for (uint8 indexSub = EQ::invbag::SLOT_BEGIN; indexSub <= EQ::invbag::SLOT_END; ++indexSub) {
				inst_sub = inst_main->GetItem(indexSub);
				if (!inst_sub) // extant only
					continue;

				item_data = inst_sub->GetItem();
				linker.SetItemInst(inst_sub);

				c->Message(
					(item_data == nullptr),
					"..%sBagSlot: %i (Slot #%i, Bag idx #%i), Item: %i (%s), Charges: %i",
					scope_prefix[scopeIndex],
					((scopeBit & peekWorld) ? INVALID_INDEX : EQ::InventoryProfile::CalcSlotId(indexMain, indexSub)),
					((scopeBit & peekWorld) ? (EQ::invslot::WORLD_BEGIN + indexMain) : indexMain),
					indexSub,
					((item_data == nullptr) ? 0 : item_data->ID),
					linker.GenerateLink().c_str(),
					((inst_sub == nullptr) ? 0 : inst_sub->GetCharges())
				);

				if (inst_sub->IsClassCommon()) {
					for (uint8 indexAug = EQ::invaug::SOCKET_BEGIN; indexAug <= EQ::invaug::SOCKET_END; ++indexAug) {
						inst_aug = inst_sub->GetItem(indexAug);
						if (!inst_aug) // extant only
							continue;

						item_data = inst_aug->GetItem();
						linker.SetItemInst(inst_aug);

						c->Message(
							(item_data == nullptr),
							"...%sAugSlot: %i (Slot #%i, Sub idx #%i, Aug idx #%i), Item: %i (%s), Charges: %i",
							scope_prefix[scopeIndex],
							INVALID_INDEX,
							((scopeBit & peekWorld) ? INVALID_INDEX : EQ::InventoryProfile::CalcSlotId(indexMain, indexSub)),
							indexSub,
							indexAug,
							((item_data == nullptr) ? 0 : item_data->ID),
							linker.GenerateLink().c_str(),
							((inst_sub == nullptr) ? 0 : inst_sub->GetCharges())
						);
					}
				}
			}
		}

		if (scopeBit & peekLimbo) {
			int limboIndex = 0;
			for (auto it = targetClient->GetInv().cursor_cbegin(); (it != targetClient->GetInv().cursor_cend()); ++it, ++limboIndex) {
				if (it == targetClient->GetInv().cursor_cbegin())
					continue;

				inst_main = *it;
				if (inst_main) {
					itemsFound = true;
					item_data = inst_main->GetItem();
				}
				else {
					item_data = nullptr;
				}

				linker.SetItemInst(inst_main);

				c->Message(
					(item_data == nullptr),
					"%sSlot: %i, Item: %i (%s), Charges: %i",
					scope_prefix[scopeIndex],
					(8000 + limboIndex),
					((item_data == nullptr) ? 0 : item_data->ID),
					linker.GenerateLink().c_str(),
					((inst_main == nullptr) ? 0 : inst_main->GetCharges())
				);

				if (inst_main && inst_main->IsClassCommon()) {
					for (uint8 indexAug = EQ::invaug::SOCKET_BEGIN; indexAug <= EQ::invaug::SOCKET_END; ++indexAug) {
						inst_aug = inst_main->GetItem(indexAug);
						if (!inst_aug) // extant only
							continue;

						item_data = inst_aug->GetItem();
						linker.SetItemInst(inst_aug);

						c->Message(
							(item_data == nullptr),
							".%sAugSlot: %i (Slot #%i, Aug idx #%i), Item: %i (%s), Charges: %i",
							scope_prefix[scopeIndex],
							INVALID_INDEX,
							(8000 + limboIndex),
							indexAug,
							((item_data == nullptr) ? 0 : item_data->ID),
							linker.GenerateLink().c_str(),
							((inst_sub == nullptr) ? 0 : inst_sub->GetCharges())
						);
					}
				}

				if (!scope_bag[scopeIndex] || !(inst_main && inst_main->IsClassBag()))
					continue;

				for (uint8 indexSub = EQ::invbag::SLOT_BEGIN; indexSub <= EQ::invbag::SLOT_END; ++indexSub) {
					inst_sub = inst_main->GetItem(indexSub);
					if (!inst_sub)
						continue;

					item_data = (inst_sub == nullptr) ? nullptr : inst_sub->GetItem();

					linker.SetItemInst(inst_sub);

					c->Message(
						(item_data == nullptr),
						"..%sBagSlot: %i (Slot #%i, Bag idx #%i), Item: %i (%s), Charges: %i",
						scope_prefix[scopeIndex],
						INVALID_INDEX,
						(8000 + limboIndex),
						indexSub,
						((item_data == nullptr) ? 0 : item_data->ID),
						linker.GenerateLink().c_str(),
						((inst_sub == nullptr) ? 0 : inst_sub->GetCharges())
					);

					if (inst_sub->IsClassCommon()) {
						for (uint8 indexAug = EQ::invaug::SOCKET_BEGIN; indexAug <= EQ::invaug::SOCKET_END; ++indexAug) {
							inst_aug = inst_sub->GetItem(indexAug);
							if (!inst_aug) // extant only
								continue;

							item_data = inst_aug->GetItem();
							linker.SetItemInst(inst_aug);

							c->Message(
								(item_data == nullptr),
								"...%sAugSlot: %i (Slot #%i, Sub idx #%i, Aug idx #%i), Item: %i (%s), Charges: %i",
								scope_prefix[scopeIndex],
								INVALID_INDEX,
								(8000 + limboIndex),
								indexSub,
								indexAug,
								((item_data == nullptr) ? 0 : item_data->ID),
								linker.GenerateLink().c_str(),
								((inst_sub == nullptr) ? 0 : inst_sub->GetCharges())
							);
						}
					}
				}
			}
		}
	}

	if (!itemsFound)
		c->Message(Chat::White, "No items found.");
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
			c->Message(Chat::White, "Usage: #interrogateinv");
			c->Message(Chat::White, "  Displays your inventory's current in-memory nested storage references");
		}
		else {
			c->Message(Chat::White, "Usage: #interrogateinv [log] [silent]");
			c->Message(Chat::White, "  Displays your or your Player target inventory's current in-memory nested storage references");
			c->Message(Chat::White, "  [log] - Logs interrogation to file");
			c->Message(Chat::White, "  [silent] - Omits the in-game message portion of the interrogation");
		}
		return;
	}

	Client* target = nullptr;
	std::map<int16, const EQ::ItemInstance*> instmap;
	bool log = false;
	bool silent = false;
	bool error = false;
	bool allowtrip = false;

	if (c->Admin() < commandInterrogateInv) {
		if (c->GetInterrogateInvState()) {
			c->Message(Chat::Red, "The last use of #interrogateinv on this inventory instance discovered an error...");
			c->Message(Chat::Red, "Logging out, zoning or re-arranging items at this point will result in item loss!");
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
			c->Message(Chat::Default, "Use of this command is limited to Client entities");
			return;
		}

		if (strcasecmp(sep->arg[1], "log") == 0)
			log = true;
		if (strcasecmp(sep->arg[2], "silent") == 0)
			silent = true;
	}

	bool success = target->InterrogateInventory(c, log, silent, allowtrip, error);

	if (!success)
		c->Message(Chat::Red, "An unknown error occurred while processing Client::InterrogateInventory()");
}

void command_invsnapshot(Client *c, const Seperator *sep)
{
	if (!c)
		return;

	if (sep->argnum == 0 || strcmp(sep->arg[1], "help") == 0) {
		std::string window_title = "Inventory Snapshot Argument Help Menu";

		std::string window_text =
			"<table>"
				"<tr>"
					"<td><c \"#FFFFFF\">Usage:</td>"
					"<td></td>"
					"<td>#invsnapshot arguments<br>(<c \"#00FF00\">required <c \"#FFFF00\">optional<c \"#FFFFFF\">)</td>"
				"</tr>"
				"<tr>"
					"<td><c \"#FFFF00\">help</td>"
					"<td></td>"
					"<td><c \"#AAAAAA\">this menu</td>"
				"</tr>"
				"<tr>"
					"<td><c \"#00FF00\">capture</td>"
					"<td></td>"
					"<td><c \"#AAAAAA\">takes snapshot of character inventory</td>"
				"</tr>";

		if (c->Admin() >= commandInvSnapshot)
			window_text.append(
				"<tr>"
					"<td><c \"#00FF00\">gcount</td>"
					"<td></td>"
					"<td><c \"#AAAAAA\">returns global snapshot count</td>"
				"</tr>"
				"<tr>"
					"<td><c \"#00FF00\">gclear</td>"
					"<td><c \"#FFFF00\"><br>now</td>"
					"<td><c \"#AAAAAA\">delete all snapshots - rule<br>delete all snapshots - now</td>"
				"</tr>"
				"<tr>"
					"<td><c \"#00FF00\">count</td>"
					"<td></td>"
					"<td><c \"#AAAAAA\">returns character snapshot count</td>"
				"</tr>"
				"<tr>"
					"<td><c \"#00FF00\">clear</td>"
					"<td><c \"#FFFF00\"><br>now</td>"
					"<td><c \"#AAAAAA\">delete character snapshots - rule<br>delete character snapshots - now</td>"
				"</tr>"
				"<tr>"
					"<td><c \"#00FF00\">list</td>"
					"<td><br><c \"#FFFF00\">count</td>"
					"<td><c \"#AAAAAA\">lists entry ids for current character<br>limits to count</td>"
				"</tr>"
				"<tr>"
					"<td><c \"#00FF00\">parse</td>"
					"<td><c \"#00FF00\">tstmp</td>"
					"<td><c \"#AAAAAA\">displays slots and items in snapshot</td>"
				"</tr>"
				"<tr>"
					"<td><c \"#00FF00\">compare</td>"
					"<td><c \"#00FF00\">tstmp</td>"
					"<td><c \"#AAAAAA\">compares inventory against snapshot</td>"
				"</tr>"
				"<tr>"
					"<td><c \"#00FF00\">restore</td>"
					"<td><c \"#00FF00\">tstmp</td>"
					"<td><c \"#AAAAAA\">restores slots and items in snapshot</td>"
				"</tr>"
			);

		window_text.append(
			"</table>"
		);

		c->SendPopupToClient(window_title.c_str(), window_text.c_str());

		return;
	}

	if (c->Admin() >= commandInvSnapshot) { // global arguments

		if (strcmp(sep->arg[1], "gcount") == 0) {
			auto is_count = database.CountInvSnapshots();
			c->Message(Chat::White, "There %s %i inventory snapshot%s.", (is_count == 1 ? "is" : "are"), is_count, (is_count == 1 ? "" : "s"));

			return;
		}

		if (strcmp(sep->arg[1], "gclear") == 0) {
			if (strcmp(sep->arg[2], "now") == 0) {
				database.ClearInvSnapshots(true);
				c->Message(Chat::White, "Inventory snapshots cleared using current time.");
			}
			else {
				database.ClearInvSnapshots();
				c->Message(Chat::White, "Inventory snapshots cleared using RuleI(Character, InvSnapshotHistoryD) (%i day%s).",
					RuleI(Character, InvSnapshotHistoryD), (RuleI(Character, InvSnapshotHistoryD) == 1 ? "" : "s"));
			}

			return;
		}
	}

	if (!c->GetTarget() || !c->GetTarget()->IsClient()) {
		c->Message(Chat::White, "Target must be a client.");
		return;
	}

	auto tc = (Client*)c->GetTarget();

	if (strcmp(sep->arg[1], "capture") == 0) {
		if (database.SaveCharacterInvSnapshot(tc->CharacterID())) {
			tc->SetNextInvSnapshot(RuleI(Character, InvSnapshotMinIntervalM));
			c->Message(Chat::White, "Successful inventory snapshot taken of %s - setting next interval for %i minute%s.",
				tc->GetName(), RuleI(Character, InvSnapshotMinIntervalM), (RuleI(Character, InvSnapshotMinIntervalM) == 1 ? "" : "s"));
		}
		else {
			tc->SetNextInvSnapshot(RuleI(Character, InvSnapshotMinRetryM));
			c->Message(Chat::White, "Failed to take inventory snapshot of %s - retrying in %i minute%s.",
				tc->GetName(), RuleI(Character, InvSnapshotMinRetryM), (RuleI(Character, InvSnapshotMinRetryM) == 1 ? "" : "s"));
		}

		return;
	}

	if (c->Admin() >= commandInvSnapshot) {
		if (strcmp(sep->arg[1], "count") == 0) {
			auto is_count = database.CountCharacterInvSnapshots(tc->CharacterID());
			c->Message(Chat::White, "%s (id: %u) has %i inventory snapshot%s.", tc->GetName(), tc->CharacterID(), is_count, (is_count == 1 ? "" : "s"));

			return;
		}

		if (strcmp(sep->arg[1], "clear") == 0) {
			if (strcmp(sep->arg[2], "now") == 0) {
				database.ClearCharacterInvSnapshots(tc->CharacterID(), true);
				c->Message(Chat::White, "%s\'s (id: %u) inventory snapshots cleared using current time.", tc->GetName(), tc->CharacterID());
			}
			else {
				database.ClearCharacterInvSnapshots(tc->CharacterID());
				c->Message(Chat::White, "%s\'s (id: %u) inventory snapshots cleared using RuleI(Character, InvSnapshotHistoryD) (%i day%s).",
					tc->GetName(), tc->CharacterID(), RuleI(Character, InvSnapshotHistoryD), (RuleI(Character, InvSnapshotHistoryD) == 1 ? "" : "s"));
			}

			return;
		}

		if (strcmp(sep->arg[1], "list") == 0) {
			std::list<std::pair<uint32, int>> is_list;
			database.ListCharacterInvSnapshots(tc->CharacterID(), is_list);

			if (is_list.empty()) {
				c->Message(Chat::White, "No inventory snapshots for %s (id: %u)", tc->GetName(), tc->CharacterID());
				return;
			}

			auto list_count = 0;
			if (sep->IsNumber(2))
				list_count = atoi(sep->arg[2]);
			if (list_count < 1 || list_count > is_list.size())
				list_count = is_list.size();

			std::string window_title = StringFormat("Snapshots for %s", tc->GetName());

			std::string window_text =
				"<table>"
					"<tr>"
						"<td>Timestamp</td>"
						"<td>Entry Count</td>"
					"</tr>";

			for (auto iter : is_list) {
				if (!list_count)
					break;

				window_text.append(StringFormat(
					"<tr>"
						"<td>%u</td>"
						"<td>%i</td>"
					"</tr>",
					iter.first,
					iter.second
				));

				--list_count;
			}

			window_text.append(
				"</table>"
			);

			c->SendPopupToClient(window_title.c_str(), window_text.c_str());

			return;
		}

		if (strcmp(sep->arg[1], "parse") == 0) {
			if (!sep->IsNumber(2)) {
				c->Message(Chat::White, "A timestamp is required to use this option.");
				return;
			}

			uint32 timestamp = atoul(sep->arg[2]);

			if (!database.ValidateCharacterInvSnapshotTimestamp(tc->CharacterID(), timestamp)) {
				c->Message(Chat::White, "No inventory snapshots for %s (id: %u) exist at %u.", tc->GetName(), tc->CharacterID(), timestamp);
				return;
			}

			std::list<std::pair<int16, uint32>> parse_list;
			database.ParseCharacterInvSnapshot(tc->CharacterID(), timestamp, parse_list);

			std::string window_title = StringFormat("Snapshot Parse for %s @ %u", tc->GetName(), timestamp);

			std::string window_text = "Slot: ItemID - Description<br>";

			for (auto iter : parse_list) {
				auto item_data = database.GetItem(iter.second);
				std::string window_line = StringFormat("%i: %u - %s<br>", iter.first, iter.second, (item_data ? item_data->Name : "[error]"));

				if (window_text.length() + window_line.length() < 4095) {
					window_text.append(window_line);
				}
				else {
					c->Message(Chat::White, "Too many snapshot entries to list...");
					break;
				}
			}

			c->SendPopupToClient(window_title.c_str(), window_text.c_str());

			return;
		}

		if (strcmp(sep->arg[1], "compare") == 0) {
			if (!sep->IsNumber(2)) {
				c->Message(Chat::White, "A timestamp is required to use this option.");
				return;
			}

			uint32 timestamp = atoul(sep->arg[2]);

			if (!database.ValidateCharacterInvSnapshotTimestamp(tc->CharacterID(), timestamp)) {
				c->Message(Chat::White, "No inventory snapshots for %s (id: %u) exist at %u.", tc->GetName(), tc->CharacterID(), timestamp);
				return;
			}

			std::list<std::pair<int16, uint32>> inv_compare_list;
			database.DivergeCharacterInventoryFromInvSnapshot(tc->CharacterID(), timestamp, inv_compare_list);

			std::list<std::pair<int16, uint32>> iss_compare_list;
			database.DivergeCharacterInvSnapshotFromInventory(tc->CharacterID(), timestamp, iss_compare_list);

			std::string window_title = StringFormat("Snapshot Comparison for %s @ %u", tc->GetName(), timestamp);

			std::string window_text = "Slot: (action) Snapshot -&gt; Inventory<br>";

			auto inv_iter = inv_compare_list.begin();
			auto iss_iter = iss_compare_list.begin();

			while (true) {
				std::string window_line;

				if (inv_iter == inv_compare_list.end() && iss_iter == iss_compare_list.end()) {
					break;
				}
				else if (inv_iter != inv_compare_list.end() && iss_iter == iss_compare_list.end()) {
					window_line = StringFormat("%i: (delete) [empty] -&gt; %u<br>", inv_iter->first, inv_iter->second);
					++inv_iter;
				}
				else if (inv_iter == inv_compare_list.end() && iss_iter != iss_compare_list.end()) {
					window_line = StringFormat("%i: (insert) %u -&gt; [empty]<br>", iss_iter->first, iss_iter->second);
					++iss_iter;
				}
				else {
					if (inv_iter->first < iss_iter->first) {
						window_line = StringFormat("%i: (delete) [empty] -&gt; %u<br>", inv_iter->first, inv_iter->second);
						++inv_iter;
					}
					else if (inv_iter->first > iss_iter->first) {
						window_line = StringFormat("%i: (insert) %u -&gt; [empty]<br>", iss_iter->first, iss_iter->second);
						++iss_iter;
					}
					else {
						window_line = StringFormat("%i: (replace) %u -&gt; %u<br>", iss_iter->first, iss_iter->second, inv_iter->second);
						++inv_iter;
						++iss_iter;
					}
				}

				if (window_text.length() + window_line.length() < 4095) {
					window_text.append(window_line);
				}
				else {
					c->Message(Chat::White, "Too many comparison entries to list...");
					break;
				}
			}

			c->SendPopupToClient(window_title.c_str(), window_text.c_str());

			return;
		}

		if (strcmp(sep->arg[1], "restore") == 0) {
			if (!sep->IsNumber(2)) {
				c->Message(Chat::White, "A timestamp is required to use this option.");
				return;
			}

			uint32 timestamp = atoul(sep->arg[2]);

			if (!database.ValidateCharacterInvSnapshotTimestamp(tc->CharacterID(), timestamp)) {
				c->Message(Chat::White, "No inventory snapshots for %s (id: %u) exist at %u.", tc->GetName(), tc->CharacterID(), timestamp);
				return;
			}

			if (database.SaveCharacterInvSnapshot(tc->CharacterID())) {
				tc->SetNextInvSnapshot(RuleI(Character, InvSnapshotMinIntervalM));
			}
			else {
				c->Message(Chat::Red, "Failed to take pre-restore inventory snapshot of %s (id: %u).",
					tc->GetName(), tc->CharacterID());
				return;
			}

			if (database.RestoreCharacterInvSnapshot(tc->CharacterID(), timestamp)) {
				// cannot delete all valid item slots from client..so, we worldkick
				tc->WorldKick(); // self restores update before the 'kick' is processed

				c->Message(Chat::White, "Successfully applied snapshot %u to %s's (id: %u) inventory.",
					timestamp, tc->GetName(), tc->CharacterID());
			}
			else {
				c->Message(Chat::Red, "Failed to apply snapshot %u to %s's (id: %u) inventory.",
					timestamp, tc->GetName(), tc->CharacterID());
			}

			return;
		}
	}
}

void command_findnpctype(Client *c, const Seperator *sep)
{
	if(sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #findnpctype [search criteria]");
		return;
    }

	std::string query;

	int id = atoi((const char *)sep->arg[1]);
	if (id == 0) // If id evaluates to 0, then search as if user entered a string.
		query = StringFormat("SELECT id, name FROM npc_types WHERE name LIKE '%%%s%%'",  sep->arg[1]);
	else // Otherwise, look for just that npc id.
		query = StringFormat("SELECT id, name FROM npc_types WHERE id = %i",  id);

    auto results = content_db.QueryDatabase(query);
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

void command_faction(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #faction -- Displays Target NPC's Primary faction");
		c->Message(Chat::White, "Usage: #faction Find [criteria | all] -- Displays factions name & id");
		c->Message(Chat::White, "Usage: #faction Review [criteria | all] -- Review Targeted Players faction hits");
		c->Message(Chat::White, "Usage: #faction Reset [id] -- Reset Targeted Players specified faction to base");
		uint32 npcfac;
		std::string npcname;
		if (c->GetTarget() && c->GetTarget()->IsNPC()) {
			npcfac = c->GetTarget()->CastToNPC()->GetPrimaryFaction();
			npcname = c->GetTarget()->CastToNPC()->GetCleanName();
			std::string blurb = fmt::format("( Target Npc: {} : has primary faction id: {} )", npcname, npcfac);
			c->Message(Chat::Yellow, blurb.c_str());
			c->Message(Chat::White, "Use: #setfaction [id] - to alter an NPC's faction");
		}
		return;
	}

	std::string faction_filter;
	if (sep->arg[2]) {
		faction_filter = str_tolower(sep->arg[2]);
	}
	if (strcasecmp(sep->arg[1], "find") == 0) {
		std::string query;
		if (strcasecmp(sep->arg[2], "all") == 0) {

			query = "SELECT `id`,`name` FROM `faction_list`";
		}
		else {
			query = fmt::format("SELECT `id`,`name` FROM `faction_list` WHERE `name` LIKE '%{}%'", faction_filter.c_str());
		}
		auto results = content_db.QueryDatabase(query);
		if (!results.Success())
			return;
		if (results.RowCount() == 0) {
			c->Message(Chat::Yellow, "No factions found with specified criteria");
			return;
		}
		int _ctr = 0;
		for (auto row = results.begin(); row != results.end(); ++row) {
			auto    id = static_cast<uint32>(atoi(row[0]));
			std::string name = row[1];
			_ctr++;
			c->Message(Chat::Yellow, "%s : id: %s", name.c_str(), std::to_string(id).c_str());
		}
		std::string response = _ctr > 0 ? fmt::format("Found {} matching factions", _ctr).c_str() : "No factions found.";
		c->Message(Chat::Yellow, response.c_str());
	}
	if (strcasecmp(sep->arg[1], "review") == 0) {
		if (!(c->GetTarget() && c->GetTarget()->IsClient())) {
			c->Message(Chat::Red, "Player Target Required for faction review");
			return;
		}
		uint32 charid = c->GetTarget()->CastToClient()->CharacterID();
		std::string revquery;
		if (strcasecmp(sep->arg[2], "all") == 0) {
			revquery = fmt::format(
				"SELECT id,`name`, current_value FROM faction_list INNER JOIN faction_values ON faction_list.id = faction_values.faction_id WHERE char_id = {}", charid);
		}
		else
		{
			revquery = fmt::format(
				"SELECT id,`name`, current_value FROM faction_list INNER JOIN faction_values ON faction_list.id = faction_values.faction_id WHERE `name` like '%{}%' and char_id = {}", faction_filter.c_str(), charid);
		}
		auto revresults = content_db.QueryDatabase(revquery);
		if (!revresults.Success())
			return;
		if (revresults.RowCount() == 0) {
			c->Message(Chat::Yellow, "No faction hits found. All are at base level");
			return;
		}
		int _ctr2 = 0;
		for (auto rrow = revresults.begin(); rrow != revresults.end(); ++rrow) {
			auto    f_id = static_cast<uint32>(atoi(rrow[0]));
			std::string cname = rrow[1];
			std::string fvalue = rrow[2];
			_ctr2++;
			std::string resetlink = fmt::format("#faction reset {}", f_id);
			c->Message(Chat::Yellow, "Reset: %s         id: %s (%s)", EQ::SayLinkEngine::GenerateQuestSaylink(resetlink, false, cname.c_str()).c_str(), std::to_string(f_id).c_str(), fvalue.c_str());
		}
		std::string response = _ctr2 > 0 ? fmt::format("Found {} matching factions", _ctr2).c_str() : "No faction hits found.";
		c->Message(Chat::Yellow, response.c_str());
	}
	else if (strcasecmp(sep->arg[1], "reset") == 0)
	{
		if (!(faction_filter == "")) {
			if (c->GetTarget() && c->GetTarget()->IsClient())
			{
				if (!c->CastToClient()->GetFeigned() && c->CastToClient()->GetAggroCount() == 0)
				{
					uint32 charid = c->GetTarget()->CastToClient()->CharacterID();
					uint32 factionid = atoi(faction_filter.c_str());

					if (c->GetTarget()->CastToClient()->ReloadCharacterFaction(c->GetTarget()->CastToClient(), factionid, charid))
						c->Message(Chat::Yellow, "faction %u was cleared.", factionid);
					else
						c->Message(Chat::Red, "An error occurred clearing faction %u", factionid);
				}
				else
				{
					c->Message(Chat::Red, "Cannot be in Combat");
					return;
				}
			}
			else {
				c->Message(Chat::Red, "Player Target Required (whose not feigning death)");
				return;
			}
		}
		else
			c->Message(Chat::Red, "No faction id entered");
	}
}

void command_findzone(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #findzone [search criteria]");
		c->Message(Chat::White, "Usage: #findzone expansion [expansion number]");
		return;
	}

	std::string query;
	int         id = atoi((const char *) sep->arg[1]);

	std::string arg1 = sep->arg[1];

	if (arg1 == "expansion") {
		query = fmt::format(
			"SELECT zoneidnumber, short_name, long_name, version FROM zone WHERE expansion = {}",
			sep->arg[2]
		);
	}
	else {

		/**
		 * If id evaluates to 0, then search as if user entered a string
		 */
		if (id == 0) {
			query = fmt::format(
				"SELECT zoneidnumber, short_name, long_name, version FROM zone WHERE long_name LIKE '%{}%' OR `short_name` LIKE '%{}%'",
				EscapeString(sep->arg[1]),
				EscapeString(sep->arg[1])
			);
		}
		else {
			query = fmt::format(
				"SELECT zoneidnumber, short_name, long_name, version FROM zone WHERE zoneidnumber = {}",
				id
			);
		}
	}

	auto results = content_db.QueryDatabase(query);
	if (!results.Success()) {
		c->Message(Chat::White, "Error querying database.");
		c->Message(Chat::White, query.c_str());
		return;
	}

	int       count   = 0;
	const int maxrows = 100;

	for (auto row = results.begin(); row != results.end(); ++row) {
		std::string zone_id    = row[0];
		std::string short_name = row[1];
		std::string long_name  = row[2];
		int         version    = atoi(row[3]);

		if (++count > maxrows) {
			c->Message(Chat::White, "%i zones shown. Too many results.", maxrows);
			break;
		}

		std::string command_zone   = EQ::SayLinkEngine::GenerateQuestSaylink("#zone " + short_name, false, "zone");
		std::string command_gmzone = EQ::SayLinkEngine::GenerateQuestSaylink(
			fmt::format("#gmzone {} {}", short_name, version),
			false,
			"gmzone"
		);

		c->Message(
			Chat::White,
			fmt::format(
				"[{}] [{}] [{}] ID ({}) Version ({}) [{}]",
				(version == 0 ? command_zone : "zone"),
				command_gmzone,
				short_name,
				zone_id,
				version,
				long_name
			).c_str()
		);
	}

	if (count <= maxrows) {
		c->Message(
			Chat::White,
			"Query complete. %i rows shown. %s",
			count,
			(arg1 == "expansion" ? "(expansion search)" : ""));
	}
	else if (count == 0) {
		c->Message(Chat::White, "No matches found for %s.", sep->arg[1]);
	}
}

void command_viewnpctype(Client *c, const Seperator *sep)
{
	if (!sep->IsNumber(1))
		c->Message(Chat::White, "Usage: #viewnpctype [npctype id]");
	else
	{
		uint32 npctypeid=atoi(sep->arg[1]);
		const NPCType* npct = content_db.LoadNPCTypesData(npctypeid);
		if (npct) {
			c->Message(Chat::White, " NPCType Info, ");
			c->Message(Chat::White, "  NPCTypeID: %u",  npct->npc_id);
			c->Message(Chat::White, "  Name: %s",  npct->name);
			c->Message(Chat::White, "  Level: %i",  npct->level);
			c->Message(Chat::White, "  Race: %i",  npct->race);
			c->Message(Chat::White, "  Class: %i",  npct->class_);
			c->Message(Chat::White, "  MinDmg: %i",  npct->min_dmg);
			c->Message(Chat::White, "  MaxDmg: %i",  npct->max_dmg);
			c->Message(Chat::White, "  Special Abilities: %s",  npct->special_abilities);
			c->Message(Chat::White, "  Spells: %i",  npct->npc_spells_id);
			c->Message(Chat::White, "  Loot Table: %i",  npct->loottable_id);
			c->Message(Chat::White, "  NPCFactionID: %i",  npct->npc_faction_id);
		}
		else
			c->Message(Chat::White, "NPC #%d not found",  npctypeid);
	}
}

void command_reloadqst(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0)
	{
		c->Message(Chat::White, "Clearing quest memory cache.");
		entity_list.ClearAreas();
		parse->ReloadQuests();
	}
	else
	{
		c->Message(Chat::White, "Clearing quest memory cache and stopping timers.");
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
	int world_repop = atoi(sep->arg[1]);
	if (world_repop == 0)
		c->Message(Chat::White, "Reloading quest cache worldwide.");
	else
		c->Message(Chat::White, "Reloading quest cache and repopping zones worldwide.");

	auto pack = new ServerPacket(ServerOP_ReloadWorld, sizeof(ReloadWorld_Struct));
	ReloadWorld_Struct* RW = (ReloadWorld_Struct*) pack->pBuffer;
	RW->Option = world_repop;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void command_reloadmerchants(Client *c, const Seperator *sep) {
	entity_list.ReloadMerchants();
	c->Message(Chat::Yellow, "Reloading merchants.");
}

void command_reloadlevelmods(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0)
	{
		if(RuleB(Zone, LevelBasedEXPMods)){
			zone->LoadLevelEXPMods();
			c->Message(Chat::Yellow, "Level based EXP Mods have been reloaded zonewide");
		}else{
			c->Message(Chat::Yellow, "Level based EXP Mods are disabled in rules!");
		}
	}
}

void command_reloadzps(Client *c, const Seperator *sep)
{
	content_db.LoadStaticZonePoints(&zone->zone_point_list, zone->GetShortName(), zone->GetInstanceVersion());
	c->Message(Chat::White, "Reloading server zone_points.");
}

void command_zoneshutdown(Client *c, const Seperator *sep)
{
	if (!worldserver.Connected())
		c->Message(Chat::White, "Error: World server disconnected");
	else if (sep->arg[1][0] == 0)
		c->Message(Chat::White, "Usage: #zoneshutdown zoneshortname");
	else {
		auto pack = new ServerPacket(ServerOP_ZoneShutdown, sizeof(ServerZoneStateChange_struct));
		ServerZoneStateChange_struct* s = (ServerZoneStateChange_struct *) pack->pBuffer;
		strcpy(s->adminname, c->GetName());
		if (sep->arg[1][0] >= '0' && sep->arg[1][0] <= '9')
			s->ZoneServerID = atoi(sep->arg[1]);
		else
			s->zoneid = ZoneID(sep->arg[1]);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void command_zonebootup(Client *c, const Seperator *sep)
{
	if (!worldserver.Connected())
		c->Message(Chat::White, "Error: World server disconnected");
	else if (sep->arg[2][0] == 0) {
		c->Message(Chat::White, "Usage: #zonebootup ZoneServerID# zoneshortname");
	}
	else {
		auto pack = new ServerPacket(ServerOP_ZoneBootup, sizeof(ServerZoneStateChange_struct));
		ServerZoneStateChange_struct* s = (ServerZoneStateChange_struct *) pack->pBuffer;
		s->ZoneServerID = atoi(sep->arg[1]);
		strcpy(s->adminname, c->GetName());
		s->zoneid = ZoneID(sep->arg[2]);
		s->makestatic = (bool) (strcasecmp(sep->arg[3], "static") == 0);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void command_kick(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0)
		c->Message(Chat::White, "Usage: #kick [charname]");
	else {
		Client* client = entity_list.GetClientByName(sep->arg[1]);
		if (client != 0) {
			if (client->Admin() <= c->Admin()) {
				client->Message(Chat::White, "You have been kicked by %s", c->GetName());
				auto outapp = new EQApplicationPacket(OP_GMKick, 0);
				client->QueuePacket(outapp);
				client->Kick("Ordered kicked by command");
				c->Message(Chat::White, "Kick: local: kicking %s",  sep->arg[1]);
			}
		}
		else if (!worldserver.Connected())
			c->Message(Chat::White, "Error: World server disconnected");
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
			c->Message(Chat::White, "Error: %s not found",  sep->arg[1]);
	}
	else
		c->Message(Chat::White, "Usage: (needs NPC targeted) #attack targetname");
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

    LogInfo("Petition list requested by [{}]",  c->GetName());

    if (results.RowCount() == 0)
        return;

    c->Message(Chat::Red,"	ID : Character Name , Account Name");

    for (auto row = results.begin(); row != results.end(); ++row)
        c->Message(Chat::Yellow, " %s:	%s , %s ", row[0],row[1],row[2]);
}

void command_equipitem(Client *c, const Seperator *sep)
{
	uint32 slot_id = atoi(sep->arg[1]);
	if (sep->IsNumber(1) && (slot_id >= EQ::invslot::EQUIPMENT_BEGIN && slot_id <= EQ::invslot::EQUIPMENT_END)) {
		const EQ::ItemInstance* from_inst = c->GetInv().GetItem(EQ::invslot::slotCursor);
		const EQ::ItemInstance* to_inst = c->GetInv().GetItem(slot_id); // added (desync issue when forcing stack to stack)
		bool partialmove = false;
		int16 movecount;

		if (from_inst && from_inst->IsClassCommon()) {
			auto outapp = new EQApplicationPacket(OP_MoveItem, sizeof(MoveItem_Struct));
			MoveItem_Struct* mi	= (MoveItem_Struct*)outapp->pBuffer;
			mi->from_slot = EQ::invslot::slotCursor;
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
				c->Message(Chat::Red, "Error: Partial stack added to existing stack exceeds allowable stacksize");
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

				//	c->Message(Chat::White, "Deleting %i charges from stack",  movecount); // debug line..delete

				//	for (int16 deletecount=0; deletecount < movecount; deletecount++)
						// have to use 'movecount' because mi->number_in_stack is 'ENCODED' at this point (i.e., 99 charges returns 22...)
				//		c->QueuePacket(outapp2);

				//	safe_delete(outapp2);
				//}
			}
			else {
				c->Message(Chat::Red, "Error: Unable to equip current item");
			}
			safe_delete(outapp);

			// also send out a wear change packet?
		}
		else if (from_inst == nullptr)
			c->Message(Chat::Red, "Error: There is no item on your cursor");
		else
			c->Message(Chat::Red, "Error: Item on your cursor cannot be equipped");
	}
	else
		c->Message(Chat::White, "Usage: #equipitem slotid[0-21] - equips the item on your cursor to the position");
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
		uint16 tmp = ZoneID(sep->arg[2]);
		if (tmp) {
			s->op = 1;
			s->zoneID = tmp;
			worldserver.SendPacket(pack);
		}
		else
			c->Message(Chat::White, "Usage: #zonelock lock [zonename]");
	}
	else if (strcasecmp(sep->arg[1], "unlock") == 0 && c->Admin() >= commandLockZones) {
		uint16 tmp = ZoneID(sep->arg[2]);
		if (tmp) {
			s->op = 2;
			s->zoneID = tmp;
			worldserver.SendPacket(pack);
		}
		else
			c->Message(Chat::White, "Usage: #zonelock unlock [zonename]");
	}
	else {
		c->Message(Chat::White, "#zonelock sub-commands");
		c->Message(Chat::White, "  list");
		if(c->Admin() >= commandLockZones)
		{
			c->Message(Chat::White, "  lock [zonename]");
			c->Message(Chat::White, "  unlock [zonename]");
		}
	}
	safe_delete(pack);
}

void command_copycharacter(Client *c, const Seperator *sep)
{
	if (sep->argnum < 3) {
		c->Message(
			Chat::White,
			"Usage: [source_character_name] [destination_character_name] [destination_account_name]"
		);
		return;
	}

	std::string source_character_name      = sep->arg[1];
	std::string destination_character_name = sep->arg[2];
	std::string destination_account_name   = sep->arg[3];

	bool result = database.CopyCharacter(
		source_character_name,
		destination_character_name,
		destination_account_name
	);

	c->Message(
		Chat::Yellow,
		fmt::format(
			"Character Copy [{}] to [{}] via account [{}] [{}]",
			source_character_name,
			destination_character_name,
			destination_account_name,
			result ? "Success" : "Failed"
		).c_str()
	);
}

void command_corpse(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();

	if (strcasecmp(sep->arg[1], "DeletePlayerCorpses") == 0 && c->Admin() >= commandEditPlayerCorpses) {
		int32 tmp = entity_list.DeletePlayerCorpses();
		if (tmp >= 0)
			c->Message(Chat::White, "%i corpses deleted.",  tmp);
		else
			c->Message(Chat::White, "DeletePlayerCorpses Error #%i",  tmp);
	}
	else if (strcasecmp(sep->arg[1], "delete") == 0) {
		if (target == 0 || !target->IsCorpse())
			c->Message(Chat::White, "Error: Target the corpse you wish to delete");
		else if (target->IsNPCCorpse()) {

			c->Message(Chat::White, "Depoping %s.",  target->GetName());
			target->CastToCorpse()->Delete();
		}
		else if (c->Admin() >= commandEditPlayerCorpses) {
			c->Message(Chat::White, "Deleting %s.",  target->GetName());
			target->CastToCorpse()->Delete();
		}
		else
			c->Message(Chat::White, "Insufficient status to delete player corpse.");
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
			c->Message(Chat::White, "%d corpses deleted.",  tmp);
		else
			c->Message(Chat::White, "DeletePlayerCorpses Error #%d",  tmp);
	}
	else if (strcasecmp(sep->arg[1], "charid") == 0 && c->Admin() >= commandEditPlayerCorpses) {
		if (target == 0 || !target->IsPlayerCorpse())
			c->Message(Chat::White, "Error: Target must be a player corpse.");
		else if (!sep->IsNumber(2))
			c->Message(Chat::White, "Error: charid must be a number.");
		else
			c->Message(Chat::White, "Setting CharID=%u on PlayerCorpse '%s'",  target->CastToCorpse()->SetCharID(atoi(sep->arg[2])), target->GetName());
	}
	else if (strcasecmp(sep->arg[1], "ResetLooter") == 0) {
		if (target == 0 || !target->IsCorpse())
			c->Message(Chat::White, "Error: Target the corpse you wish to reset");
		else
			target->CastToCorpse()->ResetLooter();
	}
	else if (strcasecmp(sep->arg[1], "RemoveCash") == 0) {
		if (target == 0 || !target->IsCorpse())
			c->Message(Chat::White, "Error: Target the corpse you wish to remove the cash from");
		else if (!target->IsPlayerCorpse() || c->Admin() >= commandEditPlayerCorpses) {
			c->Message(Chat::White, "Removing Cash from %s.",  target->GetName());
			target->CastToCorpse()->RemoveCash();
		}
		else
			c->Message(Chat::White, "Insufficient status to modify player corpse.");
	}
	else if (strcasecmp(sep->arg[1], "InspectLoot") == 0) {
		if (target == 0 || !target->IsCorpse())
			c->Message(Chat::White, "Error: Target must be a corpse.");
		else
			target->CastToCorpse()->QueryLoot(c);
	}
	else if (strcasecmp(sep->arg[1], "lock") == 0) {
		if (target == 0 || !target->IsCorpse())
			c->Message(Chat::White, "Error: Target must be a corpse.");
		else {
			target->CastToCorpse()->Lock();
			c->Message(Chat::White, "Locking %s...",  target->GetName());
		}
	}
	else if (strcasecmp(sep->arg[1], "unlock") == 0) {
		if (target == 0 || !target->IsCorpse())
			c->Message(Chat::White, "Error: Target must be a corpse.");
		else {
			target->CastToCorpse()->UnLock();
			c->Message(Chat::White, "Unlocking %s...",  target->GetName());
		}
	}
	else if (strcasecmp(sep->arg[1], "depop") == 0) {
		if (target == 0 || !target->IsPlayerCorpse())
			c->Message(Chat::White, "Error: Target must be a player corpse.");
		else if (c->Admin() >= commandEditPlayerCorpses && target->IsPlayerCorpse()) {
			c->Message(Chat::White, "Depoping %s.",  target->GetName());
			target->CastToCorpse()->DepopPlayerCorpse();
			if(!sep->arg[2][0] || atoi(sep->arg[2]) != 0)
				target->CastToCorpse()->Bury();
		}
		else
			c->Message(Chat::White, "Insufficient status to depop player corpse.");
	}
	else if (strcasecmp(sep->arg[1], "depopall") == 0) {
		if (target == 0 || !target->IsClient())
			c->Message(Chat::White, "Error: Target must be a player.");
		else if (c->Admin() >= commandEditPlayerCorpses && target->IsClient()) {
			c->Message(Chat::White, "Depoping %s\'s corpses.",  target->GetName());
			target->CastToClient()->DepopAllCorpses();
			if(!sep->arg[2][0] || atoi(sep->arg[2]) != 0)
				target->CastToClient()->BuryPlayerCorpses();
		}
		else
			c->Message(Chat::White, "Insufficient status to depop player corpse.");

	}
	else if (strcasecmp(sep->arg[1], "moveallgraveyard") == 0) {
		int count = entity_list.MovePlayerCorpsesToGraveyard(true);
		c->Message(Chat::White, "Moved [%d] player corpse(s) to zone graveyard", count);
	}
	else if (sep->arg[1][0] == 0 || strcasecmp(sep->arg[1], "help") == 0) {
		c->Message(Chat::White, "#Corpse Sub-Commands:");
		c->Message(Chat::White, "  DeleteNPCCorpses");
		c->Message(Chat::White, "  Delete - Delete targetted corpse");
		c->Message(Chat::White, "  ListNPC");
		c->Message(Chat::White, "  ListPlayer");
		c->Message(Chat::White, "  Lock - GM locks the corpse - cannot be looted by non-GM");
		c->Message(Chat::White, "  MoveAllGraveyard - move all player corpses to zone's graveyard or non-instance");
		c->Message(Chat::White, "  UnLock");
		c->Message(Chat::White, "  RemoveCash");
		c->Message(Chat::White, "  InspectLoot");
		c->Message(Chat::White, "  [to remove items from corpses, loot them]");
		c->Message(Chat::White, "Lead-GM status required to delete/modify player corpses");
		c->Message(Chat::White, "  DeletePlayerCorpses");
		c->Message(Chat::White, "  CharID [charid] - change player corpse's owner");
		c->Message(Chat::White, "  Depop [bury] - Depops single target corpse.");
		c->Message(Chat::White, "  Depopall [bury] - Depops all target player's corpses.");
		c->Message(Chat::White, "Set bury to 0 to skip burying the corpses.");
	}
	else
		c->Message(Chat::White, "Error, #corpse sub-command not found");
}

void command_fixmob(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	const char* Usage = "Usage: #fixmob [race|gender|texture|helm|face|hair|haircolor|beard|beardcolor|heritage|tattoo|detail] [next|prev]";

	if (!sep->arg[1])
		c->Message(Chat::White,Usage);
	else if (!target)
		c->Message(Chat::White,"Error: this command requires a target");
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
			c->Message(Chat::White,Usage);
		}
		else
		{
			target->SendIllusionPacket(Race, Gender, Texture, HelmTexture, HairColor, BeardColor,
										EyeColor1, EyeColor2, HairStyle, LuclinFace, Beard, 0xFF,
										DrakkinHeritage, DrakkinTattoo, DrakkinDetails);

			c->Message(Chat::White, "%s=%i",  ChangeType, ChangeSetting);
		}
	}
}

void command_gmspeed(Client *c, const Seperator *sep)
{
	bool   state = atobool(sep->arg[1]);
	Client *t    = c;

	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	if (sep->arg[1][0] != 0) {
		database.SetGMSpeed(t->AccountID(), state ? 1 : 0);
		c->Message(Chat::White, "Turning GMSpeed %s for %s (zone to take effect)", state ? "On" : "Off", t->GetName());
	}
	else {
		c->Message(Chat::White, "Usage: #gmspeed [on/off]");
	}
}

void command_gmzone(Client *c, const Seperator *sep)
{
	if (!sep->arg[1]) {
		c->Message(Chat::White, "Usage");
		c->Message(Chat::White, "-------");
		c->Message(Chat::White, "#gmzone [zone_short_name] [zone_version=0]");
		return;
	}

	std::string zone_short_name_string = sep->arg[1];
	const char  *zone_short_name       = sep->arg[1];
	auto        zone_version           = static_cast<uint32>(sep->arg[2] ? atoi(sep->arg[2]) : 0);
	std::string identifier             = "gmzone";
	uint32      zone_id                = ZoneID(zone_short_name);
	uint32      duration               = 100000000;
	uint16      instance_id            = 0;

	if (zone_id == 0) {
		c->Message(Chat::Red, "Invalid zone specified");
		return;
	}

	if (sep->arg[3] && sep->arg[3][0]) {
		identifier = sep->arg[3];
	}

	std::string bucket_key             = StringFormat("%s-%s-%u-instance", zone_short_name, identifier.c_str(), zone_version);
	std::string existing_zone_instance = DataBucket::GetData(bucket_key);

	if (existing_zone_instance.length() > 0) {
		instance_id = std::stoi(existing_zone_instance);

		c->Message(Chat::Yellow, "Found already created instance (%s) (%u)", zone_short_name, instance_id);
	}

	if (instance_id == 0) {
		if (!database.GetUnusedInstanceID(instance_id)) {
			c->Message(Chat::Red, "Server was unable to find a free instance id.");
			return;
		}

		if (!database.CreateInstance(instance_id, zone_id, zone_version, duration)) {
			c->Message(Chat::Red, "Server was unable to create a new instance.");
			return;
		}

		c->Message(Chat::Yellow, "New private GM instance %s was created with id %lu.", zone_short_name, (unsigned long) instance_id);
		DataBucket::SetData(bucket_key, std::to_string(instance_id));
	}

	if (instance_id > 0) {
		float target_x   = -1, target_y = -1, target_z = -1, target_heading = -1;
		int16 min_status = 0;
		uint8 min_level  = 0;

		if (!content_db.GetSafePoints(
			zone_short_name,
			zone_version,
			&target_x,
			&target_y,
			&target_z,
			&target_heading,
			&min_status,
			&min_level
		)) {
			c->Message(Chat::Red, "Failed to find safe coordinates for specified zone");
		}

		c->Message(Chat::Yellow, "Zoning to private GM instance (%s) (%u)", zone_short_name, instance_id);

		c->AssignToInstance(instance_id);
		c->MovePC(zone_id, instance_id, target_x, target_y, target_z, target_heading, 1);
	}
}

void command_title(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0]==0)
		c->Message(Chat::White, "Usage: #title [remove|text] [1 = Create row in title table] - remove or set title to 'text'");
	else {
		bool Save = (atoi(sep->arg[2]) == 1);

		Mob *target_mob = c->GetTarget();
		if(!target_mob)
			target_mob = c;
		if(!target_mob->IsClient()) {
			c->Message(Chat::Red, "#title only works on players.");
			return;
		}
		Client *t = target_mob->CastToClient();

		if(strlen(sep->arg[1]) > 31) {
			c->Message(Chat::Red, "Title must be 31 characters or less.");
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
			c->Message(Chat::Red, "%s's title has been removed.",  t->GetName(), sep->arg[1]);
			if(t != c)
				t->Message(Chat::Red, "Your title has been removed.",  sep->arg[1]);
		} else {
			c->Message(Chat::Red, "%s's title has been changed to '%s'.",  t->GetName(), sep->arg[1]);
			if(t != c)
				t->Message(Chat::Red, "Your title has been changed to '%s'.",  sep->arg[1]);
		}
	}
}


void command_titlesuffix(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0]==0)
		c->Message(Chat::White, "Usage: #titlesuffix [remove|text] [1 = create row in title table] - remove or set title suffix to 'text'");
	else {
		bool Save = (atoi(sep->arg[2]) == 1);

		Mob *target_mob = c->GetTarget();
		if(!target_mob)
			target_mob = c;
		if(!target_mob->IsClient()) {
			c->Message(Chat::Red, "#titlesuffix only works on players.");
			return;
		}
		Client *t = target_mob->CastToClient();

		if(strlen(sep->arg[1]) > 31) {
			c->Message(Chat::Red, "Title suffix must be 31 characters or less.");
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
			c->Message(Chat::Red, "%s's title suffix has been removed.",  t->GetName(), sep->arg[1]);
			if(t != c)
				t->Message(Chat::Red, "Your title suffix has been removed.",  sep->arg[1]);
		} else {
			c->Message(Chat::Red, "%s's title suffix has been changed to '%s'.",  t->GetName(), sep->arg[1]);
			if(t != c)
				t->Message(Chat::Red, "Your title suffix has been changed to '%s'.",  sep->arg[1]);
		}
	}
}

void command_spellinfo(Client *c, const Seperator *sep)
{
	if(sep->arg[1][0]==0)
		c->Message(Chat::White, "Usage: #spellinfo [spell_id]");
	else {
		short int spell_id=atoi(sep->arg[1]);
		const struct SPDat_Spell_Struct *s=&spells[spell_id];
		c->Message(Chat::White, "Spell info for spell #%d:",  spell_id);
		c->Message(Chat::White, "  name: %s",  s->name);
		c->Message(Chat::White, "  player_1: %s",  s->player_1);
		c->Message(Chat::White, "  teleport_zone: %s",  s->teleport_zone);
		c->Message(Chat::White, "  you_cast: %s",  s->you_cast);
		c->Message(Chat::White, "  other_casts: %s",  s->other_casts);
		c->Message(Chat::White, "  cast_on_you: %s",  s->cast_on_you);
		c->Message(Chat::White, "  spell_fades: %s",  s->spell_fades);
		c->Message(Chat::White, "  range: %f",  s->range);
		c->Message(Chat::White, "  aoerange: %f",  s->aoerange);
		c->Message(Chat::White, "  pushback: %f",  s->pushback);
		c->Message(Chat::White, "  pushup: %f",  s->pushup);
		c->Message(Chat::White, "  cast_time: %d",  s->cast_time);
		c->Message(Chat::White, "  recovery_time: %d",  s->recovery_time);
		c->Message(Chat::White, "  recast_time: %d",  s->recast_time);
		c->Message(Chat::White, "  buffdurationformula: %d",  s->buffdurationformula);
		c->Message(Chat::White, "  buffduration: %d",  s->buffduration);
		c->Message(Chat::White, "  AEDuration: %d",  s->AEDuration);
		c->Message(Chat::White, "  mana: %d",  s->mana);
		c->Message(Chat::White, "  base[12]: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",  s->base[0], s->base[1], s->base[2], s->base[3], s->base[4], s->base[5], s->base[6], s->base[7], s->base[8], s->base[9], s->base[10], s->base[11]);
		c->Message(Chat::White, "  base22[12]: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",  s->base2[0], s->base2[1], s->base2[2], s->base2[3], s->base2[4], s->base2[5], s->base2[6], s->base2[7], s->base2[8], s->base2[9], s->base2[10], s->base2[11]);
		c->Message(Chat::White, "  max[12]: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",  s->max[0], s->max[1], s->max[2], s->max[3], s->max[4], s->max[5], s->max[6], s->max[7], s->max[8], s->max[9], s->max[10], s->max[11]);
		c->Message(Chat::White, "  components[4]: %d, %d, %d, %d",  s->components[0], s->components[1], s->components[2], s->components[3]);
		c->Message(Chat::White, "  component_counts[4]: %d, %d, %d, %d",  s->component_counts[0], s->component_counts[1], s->component_counts[2], s->component_counts[3]);
		c->Message(Chat::White, "  NoexpendReagent[4]: %d, %d, %d, %d",  s->NoexpendReagent[0], s->NoexpendReagent[1], s->NoexpendReagent[2], s->NoexpendReagent[3]);
		c->Message(Chat::White, "  formula[12]: 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x",  s->formula[0], s->formula[1], s->formula[2], s->formula[3], s->formula[4], s->formula[5], s->formula[6], s->formula[7], s->formula[8], s->formula[9], s->formula[10], s->formula[11]);
		c->Message(Chat::White, "  goodEffect: %d",  s->goodEffect);
		c->Message(Chat::White, "  Activated: %d",  s->Activated);
		c->Message(Chat::White, "  resisttype: %d",  s->resisttype);
		c->Message(Chat::White, "  effectid[12]: 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x",  s->effectid[0], s->effectid[1], s->effectid[2], s->effectid[3], s->effectid[4], s->effectid[5], s->effectid[6], s->effectid[7], s->effectid[8], s->effectid[9], s->effectid[10], s->effectid[11]);
		c->Message(Chat::White, "  targettype: %d",  s->targettype);
		c->Message(Chat::White, "  basediff: %d",  s->basediff);
		c->Message(Chat::White, "  skill: %d",  s->skill);
		c->Message(Chat::White, "  zonetype: %d",  s->zonetype);
		c->Message(Chat::White, "  EnvironmentType: %d",  s->EnvironmentType);
		c->Message(Chat::White, "  TimeOfDay: %d",  s->TimeOfDay);
		c->Message(Chat::White, "  classes[15]: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
			s->classes[0], s->classes[1], s->classes[2], s->classes[3], s->classes[4],
			s->classes[5], s->classes[6], s->classes[7], s->classes[8], s->classes[9],
			s->classes[10], s->classes[11], s->classes[12], s->classes[13], s->classes[14]);
		c->Message(Chat::White, "  CastingAnim: %d",  s->CastingAnim);
		c->Message(Chat::White, "  SpellAffectIndex: %d",  s->SpellAffectIndex);
		c->Message(Chat::White, " RecourseLink: %d",  s->RecourseLink);
	}
}

void command_lastname(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();
	LogInfo("#lastname request from [{}] for [{}]",  c->GetName(), t->GetName());

	if(strlen(sep->arg[1]) <= 70)
		t->ChangeLastName(sep->arg[1]);
	else
		c->Message(Chat::White, "Usage: #lastname <lastname> where <lastname> is less than 70 chars long");
}

void command_memspell(Client *c, const Seperator *sep)
{
	uint32 slot;
	uint16 spell_id;

	if (!(sep->IsNumber(1) && sep->IsNumber(2)))
	{
		c->Message(Chat::White, "Usage: #MemSpell slotid spellid");
	}
	else
	{
		slot = atoi(sep->arg[1]) - 1;
		spell_id = atoi(sep->arg[2]);
		if (slot > EQ::spells::SPELL_GEM_COUNT || spell_id >= SPDAT_RECORDS)
		{
			c->Message(Chat::White, "Error: #MemSpell: Arguement out of range");
		}
		else
		{
			c->MemSpell(spell_id, slot);
			c->Message(Chat::White, "Spell slot changed, have fun!");
		}
	}
}
void command_save(Client *c, const Seperator *sep)
{
	if (c->GetTarget() == 0)
		c->Message(Chat::White, "Error: no target");
	else if (c->GetTarget()->IsClient()) {
		if (c->GetTarget()->CastToClient()->Save(2))
			c->Message(Chat::White, "%s successfully saved.",  c->GetTarget()->GetName());
		else
			c->Message(Chat::White, "Manual save for %s failed.",  c->GetTarget()->GetName());
	}
	else if (c->GetTarget()->IsPlayerCorpse()) {
		if (c->GetTarget()->CastToMob()->Save())
			c->Message(Chat::White, "%s successfully saved. (dbid=%u)",  c->GetTarget()->GetName(), c->GetTarget()->CastToCorpse()->GetCorpseDBID());
		else
			c->Message(Chat::White, "Manual save for %s failed.",  c->GetTarget()->GetName());
	}
	else
		c->Message(Chat::White, "Error: target not a Client/PlayerCorpse");
}

void command_showstats(Client *c, const Seperator *sep)
{
	if (c->GetTarget() != 0 )
		c->GetTarget()->ShowStats(c);
	else
		c->ShowStats(c);
}

void command_showzonegloballoot(Client *c, const Seperator *sep)
{
	c->Message(Chat::White, "GlobalLoot for %s (%d:%d)", zone->GetShortName(), zone->GetZoneID(), zone->GetInstanceVersion());
	zone->ShowZoneGlobalLoot(c);
}

void command_showzonepoints(Client *c, const Seperator *sep)
{
	auto      &mob_list = entity_list.GetMobList();
	for (auto itr : mob_list) {
		Mob *mob = itr.second;
		if (mob->IsNPC() && mob->GetRace() == 2254) {
			mob->Depop();
		}
	}

	int found_zone_points = 0;

	c->Message(Chat::White, "Listing zone points...");
	c->SendChatLineBreak();

	for (auto &virtual_zone_point : zone->virtual_zone_point_list) {
		std::string zone_long_name = zone_store.GetZoneLongName(virtual_zone_point.target_zone_id);

		c->Message(
			Chat::White,
			fmt::format(
				"Virtual Zone Point x [{}] y [{}] z [{}] h [{}] width [{}] height [{}] | To [{}] ({}) x [{}] y [{}] z [{}] h [{}]",
				virtual_zone_point.x,
				virtual_zone_point.y,
				virtual_zone_point.z,
				virtual_zone_point.heading,
				virtual_zone_point.width,
				virtual_zone_point.height,
				zone_long_name.c_str(),
				virtual_zone_point.target_zone_id,
				virtual_zone_point.target_x,
				virtual_zone_point.target_y,
				virtual_zone_point.target_z,
				virtual_zone_point.target_heading
			).c_str()
		);

		std::string node_name = fmt::format("ZonePoint To [{}]", zone_long_name);

		float half_width = ((float) virtual_zone_point.width / 2);

		NPC::SpawnZonePointNodeNPC(node_name, glm::vec4(
			(float) virtual_zone_point.x + half_width,
			(float) virtual_zone_point.y + half_width,
			virtual_zone_point.z,
			virtual_zone_point.heading
		));

		NPC::SpawnZonePointNodeNPC(node_name, glm::vec4(
			(float) virtual_zone_point.x + half_width,
			(float) virtual_zone_point.y - half_width,
			virtual_zone_point.z,
			virtual_zone_point.heading
		));

		NPC::SpawnZonePointNodeNPC(node_name, glm::vec4(
			(float) virtual_zone_point.x - half_width,
			(float) virtual_zone_point.y - half_width,
			virtual_zone_point.z,
			virtual_zone_point.heading
		));

		NPC::SpawnZonePointNodeNPC(node_name, glm::vec4(
			(float) virtual_zone_point.x - half_width,
			(float) virtual_zone_point.y + half_width,
			virtual_zone_point.z,
			virtual_zone_point.heading
		));

		NPC::SpawnZonePointNodeNPC(node_name, glm::vec4(
			(float) virtual_zone_point.x + half_width,
			(float) virtual_zone_point.y + half_width,
			(float) virtual_zone_point.z + (float) virtual_zone_point.height,
			virtual_zone_point.heading
		));

		NPC::SpawnZonePointNodeNPC(node_name, glm::vec4(
			(float) virtual_zone_point.x + half_width,
			(float) virtual_zone_point.y - half_width,
			(float) virtual_zone_point.z + (float) virtual_zone_point.height,
			virtual_zone_point.heading
		));

		NPC::SpawnZonePointNodeNPC(node_name, glm::vec4(
			(float) virtual_zone_point.x - half_width,
			(float) virtual_zone_point.y - half_width,
			(float) virtual_zone_point.z + (float) virtual_zone_point.height,
			virtual_zone_point.heading
		));

		NPC::SpawnZonePointNodeNPC(node_name, glm::vec4(
			(float) virtual_zone_point.x - half_width,
			(float) virtual_zone_point.y + half_width,
			(float) virtual_zone_point.z + (float) virtual_zone_point.height,
			virtual_zone_point.heading
		));

		found_zone_points++;
	}

	LinkedListIterator<ZonePoint *> iterator(zone->zone_point_list);
	iterator.Reset();
	while (iterator.MoreElements()) {
		ZonePoint   *zone_point    = iterator.GetData();
		std::string zone_long_name = zone_store.GetZoneLongName(zone_point->target_zone_id);
		std::string node_name      = fmt::format("ZonePoint To [{}]", zone_long_name);

		NPC::SpawnZonePointNodeNPC(
			node_name, glm::vec4(
				zone_point->x,
				zone_point->y,
				zone_point->z,
				zone_point->heading
			)
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Client Side Zone Point x [{}] y [{}] z [{}] h [{}] number [{}] | To [{}] ({}) x [{}] y [{}] z [{}] h [{}]",
				zone_point->x,
				zone_point->y,
				zone_point->z,
				zone_point->heading,
				zone_point->number,
				zone_long_name.c_str(),
				zone_point->target_zone_id,
				zone_point->target_x,
				zone_point->target_y,
				zone_point->target_z,
				zone_point->target_heading
			).c_str()
		);

		iterator.Advance();

		found_zone_points++;
	}

	if (found_zone_points == 0) {
		c->Message(Chat::White, "There were no zone points found...");
	}

	c->SendChatLineBreak();

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
			c->Message(Chat::White, "Error: target not a Player");
	} else
		c->SetBindPoint();
}

void command_depop(Client *c, const Seperator *sep)
{
	if (c->GetTarget() == 0 || !(c->GetTarget()->IsNPC() || c->GetTarget()->IsNPCCorpse()))
		c->Message(Chat::White, "You must have a NPC target for this command. (maybe you meant #depopzone?)");
	else {
		c->Message(Chat::White, "Depoping '%s'.",  c->GetTarget()->GetName());
		c->GetTarget()->Depop();
	}
}

void command_depopzone(Client *c, const Seperator *sep)
{
	zone->Depop();
	c->Message(Chat::White, "Zone depoped.");
}

void command_devtools(Client *c, const Seperator *sep)
{
	std::string dev_tools_key = StringFormat("%i-dev-tools-disabled", c->AccountID());

	/**
	 * Handle window toggle
	 */
	if (strcasecmp(sep->arg[1], "disable") == 0) {
		DataBucket::SetData(dev_tools_key, "true");
		c->SetDevToolsEnabled(false);
	}
	if (strcasecmp(sep->arg[1], "enable") == 0) {
		DataBucket::DeleteData(dev_tools_key);
		c->SetDevToolsEnabled(true);
	}

	c->ShowDevToolsMenu();
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
		c->Message(Chat::White, "Zone depop: Force resetting spawn timers.");
	}

	if (!sep->IsNumber(timearg)) {
        c->Message(Chat::White, "Zone depopped - repopping now.");

		zone->Repop();

		/* Force a spawn2 timer trigger so we don't delay actually spawning the NPC's */
		zone->spawn2_timer.Trigger();
		return;
	}

    c->Message(Chat::White, "Zone depoped. Repop in %i seconds",  atoi(sep->arg[timearg]));
	zone->Repop(atoi(sep->arg[timearg]) * 1000);

	zone->spawn2_timer.Trigger();
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
		c->Message(Chat::White, "Usage: #spawnstatus <[a]ll | [d]isabled | [e]nabled | {Spawn2 ID}>");
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
		c->Message(Chat::White, "Usage: #zuwcoords <under world coords>");
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
		c->Message(Chat::White, "Usage: #zunderworld <zcoord>");
	else {
		zone->newzone_data.underworld = atof(sep->arg[1]);
	}
}

void command_zsafecoords(Client *c, const Seperator *sep)
{
	// modifys and resends zhdr packet
	if(sep->arg[3][0]==0)
		c->Message(Chat::White, "Usage: #zsafecoords <safe x> <safe y> <safe z>");
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
		c->Message(Chat::White, "ERROR: Freeze requires a target.");
}

void command_unfreeze(Client *c, const Seperator *sep)
{
	if (c->GetTarget() != 0)
		c->GetTarget()->SendAppearancePacket(AT_Anim, ANIM_STAND);
	else
		c->Message(Chat::White, "ERROR: Unfreeze requires a target.");
}

void command_push(Client *c, const Seperator *sep)
{
	Mob *t = c;
	if (c->GetTarget() != nullptr)
		t = c->GetTarget();

	if (!sep->arg[1] || !sep->IsNumber(1)) {
		c->Message(Chat::White, "ERROR: Must provide at least a push back.");
		return;
	}

	float back = atof(sep->arg[1]);
	float up = 0.0f;

	if (sep->arg[2] && sep->IsNumber(2))
		up = atof(sep->arg[2]);

	if (t->IsNPC()) {
		t->IncDeltaX(back * g_Math.FastSin(c->GetHeading()));
		t->IncDeltaY(back * g_Math.FastCos(c->GetHeading()));
		t->IncDeltaZ(up);
		t->SetForcedMovement(6);
	} else if (t->IsClient()) {
		// TODO: send packet to push
	}
}

void command_proximity(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() && !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC");
		return;
	}

	for (auto &iter : entity_list.GetNPCList()) {
		auto        npc  = iter.second;
		std::string name = npc->GetName();

		if (name.find("Proximity") != std::string::npos) {
			npc->Depop();
		}
	}

	NPC *npc = c->GetTarget()->CastToNPC();

	std::vector<FindPerson_Point> points;

	FindPerson_Point p{};

	if (npc->IsProximitySet()) {
		glm::vec4 position;
		position.w = npc->GetHeading();
		position.x = npc->GetProximityMinX();
		position.y = npc->GetProximityMinY();
		position.z = npc->GetZ();

		position.x = npc->GetProximityMinX();
		position.y = npc->GetProximityMinY();
		NPC::SpawnNodeNPC("Proximity", "", position);

		position.x = npc->GetProximityMinX();
		position.y = npc->GetProximityMaxY();
		NPC::SpawnNodeNPC("Proximity", "", position);

		position.x = npc->GetProximityMaxX();
		position.y = npc->GetProximityMinY();
		NPC::SpawnNodeNPC("Proximity", "", position);

		position.x = npc->GetProximityMaxX();
		position.y = npc->GetProximityMaxY();
		NPC::SpawnNodeNPC("Proximity", "", position);

		p.x = npc->GetProximityMinX();
		p.y = npc->GetProximityMinY();
		p.z = npc->GetZ();
		points.push_back(p);

		p.x = npc->GetProximityMinX();
		p.y = npc->GetProximityMaxY();
		points.push_back(p);

		p.x = npc->GetProximityMaxX();
		p.y = npc->GetProximityMaxY();
		points.push_back(p);

		p.x = npc->GetProximityMaxX();
		p.y = npc->GetProximityMinY();
		points.push_back(p);

		p.x = npc->GetProximityMinX();
		p.y = npc->GetProximityMinY();
		points.push_back(p);
	}

	if (c->ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		c->SendPathPacket(points);
	}
}

void command_pvp(Client *c, const Seperator *sep)
{
	bool state=atobool(sep->arg[1]);
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	if(sep->arg[1][0] != 0) {
		t->SetPVP(state);
		c->Message(Chat::White, "%s now follows the ways of %s.",  t->GetName(), state?"discord":"order");
	}
	else
		c->Message(Chat::White, "Usage: #pvp [on/off]");
}

void command_setxp(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	if (sep->IsNumber(1)) {
		if (atoi(sep->arg[1]) > 9999999)
			c->Message(Chat::White, "Error: Value too high.");
		else
			t->AddEXP(atoi(sep->arg[1]));
	}
	else
		c->Message(Chat::White, "Usage: #setxp number");
}

void command_setpvppoints(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	if (sep->IsNumber(1)) {
		if (atoi(sep->arg[1]) > 9999999)
			c->Message(Chat::White, "Error: Value too high.");
		else
		{
			t->SetPVPPoints(atoi(sep->arg[1]));
			t->Save();
			t->SendPVPStats();
		}
	}
	else
		c->Message(Chat::White, "Usage: #setpvppoints number");
}

void command_name(Client *c, const Seperator *sep)
{
	Client *target;

	if( (strlen(sep->arg[1]) == 0) || (!(c->GetTarget() && c->GetTarget()->IsClient())) )
		c->Message(Chat::White, "Usage: #name newname (requires player target)");
	else
	{
		target = c->GetTarget()->CastToClient();
		char *oldname = strdup(target->GetName());
		if(target->ChangeFirstName(sep->arg[1], c->GetName()))
		{
			c->Message(Chat::White, "Successfully renamed %s to %s",  oldname, sep->arg[1]);
			// until we get the name packet working right this will work
			c->Message(Chat::White, "Sending player to char select.");
			target->Kick("Name was changed");
		}
		else
			c->Message(Chat::Red, "ERROR: Unable to rename %s. Check that the new name '%s' isn't already taken.",  oldname, sep->arg[2]);
		free(oldname);
	}
}

void command_tempname(Client *c, const Seperator *sep)
{
	Mob *target;
	target = c->GetTarget();

	if(!target)
		c->Message(Chat::White, "Usage: #tempname newname (requires a target)");
	else if(strlen(sep->arg[1]) > 0)
	{
		char *oldname = strdup(target->GetName());
		target->TempName(sep->arg[1]);
		c->Message(Chat::White, "Renamed %s to %s",  oldname, sep->arg[1]);
		free(oldname);
	}
	else {
		target->TempName();
		c->Message(Chat::White, "Restored the original name");
	}
}

void command_petname(Client *c, const Seperator *sep)
{
	Mob *target;
	target = c->GetTarget();

	if(!target)
		c->Message(Chat::White, "Usage: #petname newname (requires a target)");
	else if(target->IsPet() && (target->GetOwnerID() == c->GetID()) && strlen(sep->arg[1]) > 0)
	{
		char *oldname = strdup(target->GetName());
		target->TempName(sep->arg[1]);
		c->Message(Chat::White, "Renamed %s to %s",  oldname, sep->arg[1]);
		free(oldname);
	}
	else {
		target->TempName();
		c->Message(Chat::White, "Restored the original name");
	}
}

void command_npcspecialattk(Client *c, const Seperator *sep)
{
	if (c->GetTarget()==0 || c->GetTarget()->IsClient() || strlen(sep->arg[1]) <= 0 || strlen(sep->arg[2]) <= 0)
		c->Message(Chat::White, "Usage: #npcspecialattk *flagchar* *permtag* (Flags are E(nrage) F(lurry) R(ampage) S(ummon), permtag is 1 = True, 0 = False).");
	else {
		c->GetTarget()->CastToNPC()->NPCSpecialAttacks(sep->arg[1],atoi(sep->arg[2]));
		c->Message(Chat::White, "NPC Special Attack set.");
	}
}

void command_kill(Client *c, const Seperator *sep)
{
	if (!c->GetTarget()) {
		c->Message(Chat::White, "Error: #Kill: No target.");
	}
	else
		if (!c->GetTarget()->IsClient() || c->GetTarget()->CastToClient()->Admin() <= c->Admin())
			c->GetTarget()->Kill();
}

void command_killallnpcs(Client *c, const Seperator *sep)
{
	std::string search_string;
	if (sep->arg[1]) {
		search_string = sep->arg[1];
	}

	int count = 0;
	for (auto &itr : entity_list.GetMobList()) {
		Mob *entity = itr.second;
		if (!entity->IsNPC()) {
			continue;
		}

		std::string entity_name = entity->GetName();

		/**
		 * Filter by name
		 */
		if (search_string.length() > 0 && entity_name.find(search_string) == std::string::npos) {
			continue;
		}

		bool is_not_attackable =
				 (
					 entity->IsInvisible() ||
					 !entity->IsAttackAllowed(c) ||
					 entity->GetRace() == 127 ||
					 entity->GetRace() == 240
				 );

		if (is_not_attackable) {
			continue;
		}

		entity->Damage(c, 1000000000, 0, EQ::skills::SkillDragonPunch);

		count++;
	}

	c->Message(Chat::Yellow, "Killed (%i) npc(s)", count);
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
		c->Message(Chat::White, "Haste set to %d%% - Need to re-equip primary weapon before it takes effect",  Haste);
	}
	else
		c->Message(Chat::White, "Usage: #haste [percentage]");
}

void command_damage(Client *c, const Seperator *sep)
{
	if (c->GetTarget()==0)
		c->Message(Chat::White, "Error: #Damage: No Target.");
	else if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #damage x");
	}
	else {
		int32 nkdmg = atoi(sep->arg[1]);
		if (nkdmg > 2100000000)
			c->Message(Chat::White, "Enter a value less then 2,100,000,000.");
		else
			c->GetTarget()->Damage(c, nkdmg, SPELL_UNKNOWN, EQ::skills::SkillHandtoHand, false);
	}
}

void command_zonespawn(Client *c, const Seperator *sep)
{
	c->Message(Chat::White, "This command is not yet implemented.");
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
			content_db.NPCSpawnDB(0, zone->GetShortName(), zone->GetInstanceVersion(), c, target->CastToNPC(), extra);
			c->Message(Chat::White, "%s created successfully!",  target->GetName());
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
			content_db.NPCSpawnDB(1, zone->GetShortName(), zone->GetInstanceVersion(), c, target->CastToNPC(), extra);
			c->Message(Chat::White, "%s added successfully!",  target->GetName());
		}
		else if (strcasecmp(sep->arg[1], "update") == 0) {
			content_db.NPCSpawnDB(2, zone->GetShortName(), zone->GetInstanceVersion(), c, target->CastToNPC());
			c->Message(Chat::White, "%s updated!",  target->GetName());
		}
		else if (strcasecmp(sep->arg[1], "remove") == 0) {
			content_db.NPCSpawnDB(3, zone->GetShortName(), zone->GetInstanceVersion(), c, target->CastToNPC());
			c->Message(Chat::White, "%s removed successfully from database!",  target->GetName());
			target->Depop(false);
		}
		else if (strcasecmp(sep->arg[1], "delete") == 0) {
			content_db.NPCSpawnDB(4, zone->GetShortName(), zone->GetInstanceVersion(), c, target->CastToNPC());
			c->Message(Chat::White, "%s deleted from database!",  target->GetName());
			target->Depop(false);
		}
		else {
			c->Message(Chat::White, "Error: #npcspawn: Invalid command.");
			c->Message(Chat::White, "Usage: #npcspawn [create|add|update|remove|delete]");
		}
	}
	else
		c->Message(Chat::White, "Error: #npcspawn: You must have a NPC targeted!");
}

void command_spawnfix(Client *c, const Seperator *sep) {
	Mob *targetMob = c->GetTarget();
	if (!targetMob || !targetMob->IsNPC()) {
		c->Message(Chat::White, "Error: #spawnfix: Need an NPC target.");
		return;
    }

    Spawn2* s2 = targetMob->CastToNPC()->respawn2;

    if(!s2) {
        c->Message(Chat::White, "#spawnfix FAILED -- cannot determine which spawn entry in the database this mob came from.");
        return;
    }

    std::string query = StringFormat("UPDATE spawn2 SET x = '%f', y = '%f', z = '%f', heading = '%f' WHERE id = '%i'",
                                    c->GetX(), c->GetY(), c->GetZ(), c->GetHeading(),s2->GetID());
    auto results = content_db.QueryDatabase(query);
    if (!results.Success()) {
        c->Message(Chat::Red, "Update failed! MySQL gave the following error:");
        c->Message(Chat::Red, results.ErrorMessage().c_str());
        return;
    }

    c->Message(Chat::White, "Updating coordinates successful.");
    targetMob->Depop(false);
}

void command_loc(Client *c, const Seperator *sep)
{
	Mob *t=c->GetTarget()?c->GetTarget():c->CastToMob();

	c->Message(Chat::White, "%s's Location (XYZ): %1.2f, %1.2f, %1.2f; heading=%1.1f",  t->GetName(), t->GetX(), t->GetY(), t->GetZ(), t->GetHeading());
}

void command_goto(Client *c, const Seperator *sep)
{
	std::string arg1 = sep->arg[1];

	bool goto_via_target_no_args = sep->arg[1][0] == '\0' && c->GetTarget();
	bool goto_via_player_name    = !sep->IsNumber(1) && !arg1.empty();
	bool goto_via_x_y_z          = sep->IsNumber(1) && sep->IsNumber(2) && sep->IsNumber(3);

	if (goto_via_target_no_args) {
		c->MovePC(
			zone->GetZoneID(),
			zone->GetInstanceID(),
			c->GetTarget()->GetX(),
			c->GetTarget()->GetY(),
			c->GetTarget()->GetZ(),
			c->GetTarget()->GetHeading()
		);
	}
	else if (goto_via_player_name) {

		/**
		 * Find them in zone first
		 */
		const char  *player_name       = sep->arg[1];
		std::string player_name_string = sep->arg[1];
		Client      *client            = entity_list.GetClientByName(player_name);
		if (client) {
			c->MovePC(
				zone->GetZoneID(),
				zone->GetInstanceID(),
				client->GetX(),
				client->GetY(),
				client->GetZ(),
				client->GetHeading()
			);

			c->Message(Chat::Yellow, "Goto player '%s' same zone", player_name_string.c_str());
		}
		else if (c->GotoPlayer(player_name_string)) {
			c->Message(Chat::Yellow, "Goto player '%s' different zone", player_name_string.c_str());
		}
		else {
			c->Message(Chat::Yellow, "Player '%s' not found", player_name_string.c_str());
		}
	}
	else if (goto_via_x_y_z) {
		c->MovePC(
			zone->GetZoneID(),
			zone->GetInstanceID(),
			atof(sep->arg[1]),
			atof(sep->arg[2]),
			atof(sep->arg[3]),
			(sep->arg[4] ? atof(sep->arg[4]) : c->GetHeading())
		);
	}
	else {
		c->Message(Chat::White, "Usage: #goto [x y z] [h]");
		c->Message(Chat::White, "Usage: #goto [player_name]");
	}
}

void command_iteminfo(Client *c, const Seperator *sep)
{
	auto inst = c->GetInv()[EQ::invslot::slotCursor];
	if (!inst) {
		c->Message(Chat::Red, "Error: You need an item on your cursor for this command");
		return;
	}
	auto item = inst->GetItem();
	if (!item) {
		LogInventory("([{}]) Command #iteminfo processed an item with no data pointer");
		c->Message(Chat::Red, "Error: This item has no data reference");
		return;
	}

	EQ::SayLinkEngine linker;
	linker.SetLinkType(EQ::saylink::SayLinkItemInst);
	linker.SetItemInst(inst);

	c->Message(Chat::White, "*** Item Info for [%s] ***", linker.GenerateLink().c_str());
	c->Message(Chat::White, ">> ID: %u, ItemUseType: %u, ItemClassType: %u", item->ID, item->ItemType, item->ItemClass);
	c->Message(Chat::White, ">> IDFile: '%s', IconID: %u", item->IDFile, item->Icon);
	c->Message(Chat::White, ">> Size: %u, Weight: %u, Price: %u, LDoNPrice: %u", item->Size, item->Weight, item->Price, item->LDoNPrice);
	c->Message(Chat::White, ">> Material: 0x%02X, Color: 0x%08X, Tint: 0x%08X, Light: 0x%02X", item->Material, item->Color, inst->GetColor(), item->Light);
	c->Message(Chat::White, ">> IsLore: %s, LoreGroup: %u, Lore: '%s'", (item->LoreFlag ? "TRUE" : "FALSE"), item->LoreGroup, item->Lore);
	c->Message(Chat::White, ">> NoDrop: %u, NoRent: %u, NoPet: %u, NoTransfer: %u, FVNoDrop: %u",
		item->NoDrop, item->NoRent, (uint8)item->NoPet, (uint8)item->NoTransfer, item->FVNoDrop);

	if (item->IsClassBook()) {
		c->Message(Chat::White, "*** This item is a Book (filename:'%s') ***", item->Filename);
	}
	else if (item->IsClassBag()) {
		c->Message(Chat::White, "*** This item is a Container (%u slots) ***", item->BagSlots);
	}
	else {
		c->Message(Chat::White, "*** This item is Common ***");
		c->Message(Chat::White, ">> Classes: %u, Races: %u, Slots: %u", item->Classes, item->Races, item->Slots);
		c->Message(Chat::White, ">> ReqSkill: %u, ReqLevel: %u, RecLevel: %u", item->RecSkill, item->ReqLevel, item->RecLevel);
		c->Message(Chat::White, ">> SkillModType: %u, SkillModValue: %i", item->SkillModType, item->SkillModValue);
		c->Message(Chat::White, ">> BaneRaceType: %u, BaneRaceDamage: %u, BaneBodyType: %u, BaneBodyDamage: %i",
			item->BaneDmgRace, item->BaneDmgRaceAmt, item->BaneDmgBody, item->BaneDmgAmt);
		c->Message(Chat::White, ">> Magic: %s, SpellID: %i, ProcLevel: %u, Charges: %u, MaxCharges: %u",
			(item->Magic ? "TRUE" : "FALSE"), item->Click.Effect, item->Click.Level, inst->GetCharges(), item->MaxCharges);
		c->Message(Chat::White, ">> EffectType: 0x%02X, CastTime: %.2f", (uint8)item->Click.Type, ((double)item->CastTime / 1000));
	}

	if (c->Admin() >= 200)
		c->Message(Chat::White, ">> MinStatus: %u", item->MinStatus);
}

void command_uptime(Client *c, const Seperator *sep)
{
	if (!worldserver.Connected())
		c->Message(Chat::White, "Error: World server disconnected");
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
			c->Message(Chat::White, "Refreshed your admin flag from DB.");
		} else if (c->GetTarget() && c->GetTarget() != c && c->GetTarget()->IsClient()) {
			c->GetTarget()->CastToClient()->UpdateAdmin();
			c->Message(Chat::White, "%s's admin flag has been refreshed.", c->GetTarget()->GetName());
			c->GetTarget()->Message(Chat::White, "%s refreshed your admin flag.", c->GetName());
		}
	}
	else if (!sep->IsNumber(1) || atoi(sep->arg[1]) < -2 || atoi(sep->arg[1]) > 255 || strlen(sep->arg[2]) == 0)
		c->Message(Chat::White, "Usage: #flag [status] [acctname]");

	else if (c->Admin() < commandChangeFlags) {
		//this check makes banning players by less than this level
		//impossible, but i'll leave it in anyways
		c->Message(Chat::White, "You may only refresh your own flag, doing so now.");
		c->UpdateAdmin();
	}
	else {
		if (atoi(sep->arg[1]) > c->Admin())
			c->Message(Chat::White, "You cannot set people's status to higher than your own");
		else if (atoi(sep->arg[1]) < 0 && c->Admin() < commandBanPlayers)
			c->Message(Chat::White, "You have too low of status to suspend/ban");
		else if (!database.SetAccountStatus(sep->argplus[2], atoi(sep->arg[1])))
			c->Message(Chat::White, "Unable to set GM Flag.");
		else {
			c->Message(Chat::White, "Set GM Flag on account.");

			std::string user;
			std::string loginserver;
			ParseAccountString(sep->argplus[2], user, loginserver);

			ServerPacket pack(ServerOP_FlagUpdate, 6);
			*((uint32*) pack.pBuffer) = database.GetAccountIDByName(user.c_str(), loginserver.c_str());
			*((int16*) &pack.pBuffer[4]) = atoi(sep->arg[1]);
			worldserver.SendPacket(&pack);
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
		c->Message(Chat::Red, "Setting world time to %s:%i (Timezone: 0)...",  sep->arg[1], minutes);
		zone->SetTime(atoi(sep->arg[1])+1, minutes);
		LogInfo("{} :: Setting world time to {}:{} (Timezone: 0)...", c->GetCleanName(), sep->arg[1], minutes);
	}
	else {
		c->Message(Chat::Red, "To set the Time: #time HH [MM]");
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
		c->Message(Chat::Red, "It is now %s.", timeMessage);
		LogInfo("Current Time is: {}", timeMessage);
	}
}

void command_guild(Client *c, const Seperator *sep)
{
	int admin=c->Admin();
	Mob *target=c->GetTarget();

	if (strcasecmp(sep->arg[1], "help") == 0) {
		c->Message(Chat::White, "GM Guild commands:");
		c->Message(Chat::White, "  #guild list - lists all guilds on the server");
		c->Message(Chat::White, "  #guild create {guildleader charname or CharID} guildname");
		c->Message(Chat::White, "  #guild delete guildID");
		c->Message(Chat::White, "  #guild rename guildID newname");
		c->Message(Chat::White, "  #guild set charname guildID    (0=no guild)");
		c->Message(Chat::White, "  #guild setrank charname rank");
		c->Message(Chat::White, "  #guild setleader guildID {guildleader charname or CharID}");
	}
	else if (strcasecmp(sep->arg[1], "status") == 0 || strcasecmp(sep->arg[1], "stat") == 0) {
		Client* client = 0;
		if (sep->arg[2][0] != 0)
			client = entity_list.GetClientByName(sep->argplus[2]);
		else if (target != 0 && target->IsClient())
			client = target->CastToClient();
		if (client == 0)
			c->Message(Chat::White, "You must target someone or specify a character name");
		else if ((client->Admin() >= minStatusToEditOtherGuilds && admin < minStatusToEditOtherGuilds) && client->GuildID() != c->GuildID()) // no peeping for GMs, make sure tell message stays the same
			c->Message(Chat::White, "You must target someone or specify a character name.");
		else {
			if (client->IsInAGuild())
				c->Message(Chat::White, "%s is not in a guild.",  client->GetName());
			else if (guild_mgr.IsGuildLeader(client->GuildID(), client->CharacterID()))
				c->Message(Chat::White, "%s is the leader of <%s> rank: %s",  client->GetName(), guild_mgr.GetGuildName(client->GuildID()), guild_mgr.GetRankName(client->GuildID(), client->GuildRank()));
			else
				c->Message(Chat::White, "%s is a member of <%s> rank: %s",  client->GetName(), guild_mgr.GetGuildName(client->GuildID()), guild_mgr.GetRankName(client->GuildID(), client->GuildRank()));
		}
	}
	else if (strcasecmp(sep->arg[1], "info") == 0) {
		if (sep->arg[2][0] == 0 && c->IsInAGuild()) {
			if (admin >= minStatusToEditOtherGuilds)
				c->Message(Chat::White, "Usage: #guildinfo guild_id");
			else
				c->Message(Chat::White, "You're not in a guild");
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
			c->Message(Chat::White, "You arent in a guild!");
		else if (!sep->IsNumber(2))
			c->Message(Chat::White, "Error: invalid rank #.");
		else if (atoi(sep->arg[2]) < 0 || atoi(sep->arg[2]) > GUILD_MAX_RANK)
			c->Message(Chat::White, "Error: invalid rank #.");
		else if (!c->GuildRank() == 0)
			c->Message(Chat::White, "You must be rank %s to use edit.",  guilds[c->GuildEQID()].rank[0].rankname);
		else if (!worldserver.Connected())
			c->Message(Chat::White, "Error: World server dirconnected");
		else {
			if (!helper_guild_edit(c, c->GuildDBID(), c->GuildEQID(), atoi(sep->arg[2]), sep->arg[3], sep->argplus[4])) {
				c->Message(Chat::White, "  #guild edit rank title newtitle");
				c->Message(Chat::White, "  #guild edit rank permission 0/1");
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
			c->Message(Chat::White, "Error: invalid guilddbid.");
		else if (!sep->IsNumber(3))
			c->Message(Chat::White, "Error: invalid rank #.");
		else if (atoi(sep->arg[3]) < 0 || atoi(sep->arg[3]) > GUILD_MAX_RANK)
			c->Message(Chat::White, "Error: invalid rank #.");
		else if (!worldserver.Connected())
			c->Message(Chat::White, "Error: World server dirconnected");
		else {
			uint32 eqid = database.GetGuildEQID(atoi(sep->arg[2]));
			if (eqid == GUILD_NONE)
				c->Message(Chat::White, "Error: Guild not found");
			else if (!helper_guild_edit(c, atoi(sep->arg[2]), eqid, atoi(sep->arg[3]), sep->arg[4], sep->argplus[5])) {
				c->Message(Chat::White, "  #guild gmedit guilddbid rank title newtitle");
				c->Message(Chat::White, "  #guild gmedit guilddbid rank permission 0/1");
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
			c->Message(Chat::White, "Usage: #guild set charname guildgbid (0 = clear guildtag)");
		else {
			uint32 guild_id = atoi(sep->arg[3]);

			if(guild_id == 0)
				guild_id = GUILD_NONE;
			else if(!guild_mgr.GuildExists(guild_id)) {
				c->Message(Chat::Red, "Guild %d does not exist.",  guild_id);
				return;
			}

			uint32 charid = database.GetCharacterID(sep->arg[2]);
			if(charid == 0) {
				c->Message(Chat::Red, "Unable to find character '%s'",  charid);
				return;
			}

			//we could do the checking we need for guild_mgr.CheckGMStatus, but im lazy right now
			if(admin < minStatusToEditOtherGuilds) {
				c->Message(Chat::Red, "Access denied.");
				return;
			}

			if(guild_id == GUILD_NONE) {
				LogGuilds("[{}]: Removing [{}] ([{}]) from guild with GM command",  c->GetName(), sep->arg[2], charid);
			} else {
				LogGuilds("[{}]: Putting [{}] ([{}]) into guild [{}] ([{}]) with GM command",  c->GetName(), sep->arg[2], charid, guild_mgr.GetGuildName(guild_id), guild_id);
			}

			if(!guild_mgr.SetGuild(charid, guild_id, GUILD_MEMBER)) {
				c->Message(Chat::Red, "Error putting '%s' into guild %d",  sep->arg[2], guild_id);
			} else {
				c->Message(Chat::White, "%s has been put into guild %d",  sep->arg[2], guild_id);
			}
		}
	}
	/*else if (strcasecmp(sep->arg[1], "setdoor") == 0 && admin >= minStatusToEditOtherGuilds) {

		if (!sep->IsNumber(2))
			c->Message(Chat::White, "Usage: #guild setdoor guildEQid (0 = delete guilddoor)");
		else {
// guild doors
			if((!guilds[atoi(sep->arg[2])].databaseID) && (atoi(sep->arg[2])!=0) )
			{

				c->Message(Chat::White, "These is no guild with this guildEQid");
			}
			else {
				c->SetIsSettingGuildDoor(true);
				c->Message(Chat::White, "Click on a door you want to become a guilddoor");
				c->SetSetGuildDoorID(atoi(sep->arg[2]));
			}
		}
	}*/
	else if (strcasecmp(sep->arg[1], "setrank") == 0) {
		int rank = atoi(sep->arg[3]);
		if (!sep->IsNumber(3))
			c->Message(Chat::White, "Usage: #guild setrank charname rank");
		else if (rank < 0 || rank > GUILD_MAX_RANK)
			c->Message(Chat::White, "Error: invalid rank #.");
		else {
			uint32 charid = database.GetCharacterID(sep->arg[2]);
			if(charid == 0) {
				c->Message(Chat::Red, "Unable to find character '%s'",  charid);
				return;
			}

			//we could do the checking we need for guild_mgr.CheckGMStatus, but im lazy right now
			if(admin < minStatusToEditOtherGuilds) {
				c->Message(Chat::Red, "Access denied.");
				return;
			}

			LogGuilds("[{}]: Setting [{}] ([{}])'s guild rank to [{}] with GM command", c->GetName(), sep->arg[2], charid, rank);

			if(!guild_mgr.SetGuildRank(charid, rank))
				c->Message(Chat::Red, "Error while setting rank %d on '%s'.",  rank, sep->arg[2]);
			else
				c->Message(Chat::White, "%s has been set to rank %d",  sep->arg[2], rank);
		}
	}
	else if (strcasecmp(sep->arg[1], "create") == 0) {
		if (sep->arg[3][0] == 0)
			c->Message(Chat::White, "Usage: #guild create {guildleader charname or CharID} guild name");
		else if (!worldserver.Connected())
			c->Message(Chat::White, "Error: World server dirconnected");
		else {
			uint32 leader = 0;
			if (sep->IsNumber(2)) {
				leader = atoi(sep->arg[2]);
			} else if((leader=database.GetCharacterID(sep->arg[2])) != 0) {
				//got it from the db..
			} else {
				c->Message(Chat::Red, "Unable to find char '%s'",  sep->arg[2]);
				return;
			}
			if (leader == 0) {
				c->Message(Chat::White, "Guild leader not found.");
				return;
			}

			uint32 tmp = guild_mgr.FindGuildByLeader(leader);
			if (tmp != GUILD_NONE) {
				c->Message(Chat::White, "Error: %s already is the leader of DB# %i '%s'.",  sep->arg[2], tmp, guild_mgr.GetGuildName(tmp));
			}
			else {

				if(admin < minStatusToEditOtherGuilds) {
					c->Message(Chat::Red, "Access denied.");
					return;
				}

				uint32 id = guild_mgr.CreateGuild(sep->argplus[3], leader);

				LogGuilds("[{}]: Creating guild [{}] with leader [{}] with GM command. It was given id [{}]",  c->GetName(),
					sep->argplus[3], leader, (unsigned long)id);

				if (id == GUILD_NONE)
					c->Message(Chat::White, "Guild creation failed.");
				else {
					c->Message(Chat::White, "Guild created: Leader: %i, number %i: %s",  leader, id, sep->argplus[3]);

					if(!guild_mgr.SetGuild(leader, id, GUILD_LEADER))
						c->Message(Chat::White, "Unable to set guild leader's guild in the database. Your going to have to run #guild set");
				}

			}
		}
	}
	else if (strcasecmp(sep->arg[1], "delete") == 0) {
		if (!sep->IsNumber(2))
			c->Message(Chat::White, "Usage: #guild delete guildID");
		else if (!worldserver.Connected())
			c->Message(Chat::White, "Error: World server dirconnected");
		else {
			uint32 id = atoi(sep->arg[2]);

			if(!guild_mgr.GuildExists(id)) {
				c->Message(Chat::White, "Guild %d does not exist!",  id);
				return;
			}

			if(admin < minStatusToEditOtherGuilds) {
				//this person is not allowed to just edit any guild, check this guild's min status.
				if(c->GuildID() != id) {
					c->Message(Chat::Red, "Access denied to edit other people's guilds");
					return;
				} else if(!guild_mgr.CheckGMStatus(id, admin)) {
					c->Message(Chat::Red, "Access denied to edit your guild with GM commands.");
					return;
				}
			}

			LogGuilds("[{}]: Deleting guild [{}] ([{}]) with GM command",  c->GetName(),
				guild_mgr.GetGuildName(id), id);

			if (!guild_mgr.DeleteGuild(id))
				c->Message(Chat::White, "Guild delete failed.");
			else {
				c->Message(Chat::White, "Guild %d deleted.",  id);
			}
		}
	}
	else if (strcasecmp(sep->arg[1], "rename") == 0) {
		if ((!sep->IsNumber(2)) || sep->arg[3][0] == 0)
			c->Message(Chat::White, "Usage: #guild rename guildID newname");
		else if (!worldserver.Connected())
			c->Message(Chat::White, "Error: World server dirconnected");
		else {
			uint32 id = atoi(sep->arg[2]);

			if(!guild_mgr.GuildExists(id)) {
				c->Message(Chat::White, "Guild %d does not exist!",  id);
				return;
			}

			if(admin < minStatusToEditOtherGuilds) {
				//this person is not allowed to just edit any guild, check this guild's min status.
				if(c->GuildID() != id) {
					c->Message(Chat::Red, "Access denied to edit other people's guilds");
					return;
				} else if(!guild_mgr.CheckGMStatus(id, admin)) {
					c->Message(Chat::Red, "Access denied to edit your guild with GM commands.");
					return;
				}
			}

			LogGuilds("[{}]: Renaming guild [{}] ([{}]) to [{}] with GM command",  c->GetName(),
				guild_mgr.GetGuildName(id), id, sep->argplus[3]);

			if (!guild_mgr.RenameGuild(id, sep->argplus[3]))
				c->Message(Chat::White, "Guild rename failed.");
			else {
				c->Message(Chat::White, "Guild %d renamed to %s",  id, sep->argplus[3]);
			}
		}
	}
	else if (strcasecmp(sep->arg[1], "setleader") == 0) {
		if (sep->arg[3][0] == 0 || !sep->IsNumber(2))
			c->Message(Chat::White, "Usage: #guild setleader guild_id {guildleader charname or CharID}");
		else if (!worldserver.Connected())
			c->Message(Chat::White, "Error: World server dirconnected");
		else {
			uint32 leader = 0;
			if (sep->IsNumber(2)) {
				leader = atoi(sep->arg[2]);
			} else if((leader=database.GetCharacterID(sep->arg[2])) != 0) {
				//got it from the db..
			} else {
				c->Message(Chat::Red, "Unable to find char '%s'",  sep->arg[2]);
				return;
			}

			uint32 tmpdb = guild_mgr.FindGuildByLeader(leader);
			if (leader == 0)
				c->Message(Chat::White, "New leader not found.");
			else if (tmpdb != 0) {
				c->Message(Chat::White, "Error: %s already is the leader of guild # %i",  sep->arg[2], tmpdb);
			}
			else {
				uint32 id = atoi(sep->arg[2]);

				if(!guild_mgr.GuildExists(id)) {
					c->Message(Chat::White, "Guild %d does not exist!",  id);
					return;
				}

				if(admin < minStatusToEditOtherGuilds) {
					//this person is not allowed to just edit any guild, check this guild's min status.
					if(c->GuildID() != id) {
						c->Message(Chat::Red, "Access denied to edit other people's guilds");
						return;
					} else if(!guild_mgr.CheckGMStatus(id, admin)) {
						c->Message(Chat::Red, "Access denied to edit your guild with GM commands.");
						return;
					}
				}

				LogGuilds("[{}]: Setting leader of guild [{}] ([{}]) to [{}] with GM command",  c->GetName(),
					guild_mgr.GetGuildName(id), id, leader);

				if(!guild_mgr.SetGuildLeader(id, leader))
					c->Message(Chat::White, "Guild leader change failed.");
				else {
					c->Message(Chat::White, "Guild leader changed: guild # %d, Leader: %s",  id, sep->argplus[3]);
				}
			}
		}
	}
	else if (strcasecmp(sep->arg[1], "list") == 0) {
		if(admin < minStatusToEditOtherGuilds) {
			c->Message(Chat::Red, "Access denied.");
			return;
		}
		guild_mgr.ListGuilds(c);
	}
	else {
		c->Message(Chat::White, "Unknown guild command, try #guild help");
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
			c->Message(Chat::White, "Error: Title has a maxium length of 100 characters.");
		else
			strcpy(grl.rankname, value);
	}
	else if (rank == 0)
		c->Message(Chat::White, "Error: Rank 0's permissions can not be changed.");
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
			c->Message(Chat::White, "Error: Permission name not recognized.");
	}
	if (!database.EditGuild(dbid, rank, &grl))
		c->Message(Chat::White, "Error: database.EditGuild() failed");
	return true;
}*/

void command_zonestatus(Client *c, const Seperator *sep)
{
	if (!worldserver.Connected())
		c->Message(Chat::White, "Error: World server disconnected");
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
		c->Message(Chat::White, "Usage: #DoAnim [number]");
	else
		if (c->Admin() >= commandDoAnimOthers)
			if (c->GetTarget() == 0)
				c->Message(Chat::White, "Error: You need a target.");
			else
				c->GetTarget()->DoAnim(atoi(sep->arg[1]),atoi(sep->arg[2]));
		else
			c->DoAnim(atoi(sep->arg[1]),atoi(sep->arg[2]));
}

void command_dz(Client* c, const Seperator* sep)
{
	if (!c || !zone) {
		return;
	}

	if (strcasecmp(sep->arg[1], "expedition") == 0)
	{
		if (strcasecmp(sep->arg[2], "list") == 0)
		{
			std::vector<Expedition*> expeditions;
			for (const auto& expedition : zone->expedition_cache)
			{
				expeditions.emplace_back(expedition.second.get());
			}

			std::sort(expeditions.begin(), expeditions.end(),
				[](const Expedition* lhs, const Expedition* rhs) {
					return lhs->GetID() < rhs->GetID();
				});

			c->Message(Chat::White, fmt::format("Total Active Expeditions: [{}]", expeditions.size()).c_str());
			for (const auto& expedition : expeditions)
			{
				auto leader_saylink = EQ::SayLinkEngine::GenerateQuestSaylink(fmt::format(
					"#goto {}", expedition->GetLeaderName()), false, expedition->GetLeaderName());
				auto zone_saylink = EQ::SayLinkEngine::GenerateQuestSaylink(fmt::format(
					"#zoneinstance {}", expedition->GetDynamicZone().GetInstanceID()), false, "zone");

				auto seconds = expedition->GetDynamicZone().GetSecondsRemaining();

				c->Message(Chat::White, fmt::format(
					"expedition id: [{}] dz id: [{}] name: [{}] leader: [{}] {}: [{}]:[{}]:[{}]:[{}] members: [{}] remaining: [{:02}:{:02}:{:02}]",
					expedition->GetID(),
					expedition->GetDynamicZone().GetID(),
					expedition->GetName(),
					leader_saylink,
					zone_saylink,
					ZoneName(expedition->GetDynamicZone().GetZoneID()),
					expedition->GetDynamicZone().GetZoneID(),
					expedition->GetDynamicZone().GetInstanceID(),
					expedition->GetDynamicZone().GetZoneVersion(),
					expedition->GetDynamicZone().GetMemberCount(),
					seconds / 3600,      // hours
					(seconds / 60) % 60, // minutes
					seconds % 60         // seconds
				).c_str());
			}
		}
		else if (strcasecmp(sep->arg[2], "reload") == 0)
		{
			Expedition::CacheAllFromDatabase();
			c->Message(Chat::White, fmt::format(
				"Reloaded [{}] expeditions to cache from database.", zone->expedition_cache.size()
			).c_str());
		}
		else if (strcasecmp(sep->arg[2], "destroy") == 0 && sep->IsNumber(3))
		{
			auto expedition_id = std::strtoul(sep->arg[3], nullptr, 10);
			auto expedition = Expedition::FindCachedExpeditionByID(expedition_id);
			if (expedition)
			{
				c->Message(Chat::White, fmt::format("Destroying expedition [{}] ({})",
					expedition_id, expedition->GetName()).c_str());
				expedition->RemoveAllMembers();
			}
			else
			{
				c->Message(Chat::Red, fmt::format("Failed to destroy expedition [{}]", sep->arg[3]).c_str());
			}
		}
		else if (strcasecmp(sep->arg[2], "unlock") == 0 && sep->IsNumber(3))
		{
			auto expedition_id = std::strtoul(sep->arg[3], nullptr, 10);
			auto expedition = Expedition::FindCachedExpeditionByID(expedition_id);
			if (expedition)
			{
				c->Message(Chat::White, fmt::format("Unlocking expedition [{}]", expedition_id).c_str());
				expedition->SetLocked(false, ExpeditionLockMessage::None, true);
			}
			else
			{
				c->Message(Chat::Red, fmt::format("Failed to find expedition [{}]", sep->arg[3]).c_str());
			}
		}
	}
	else if (strcasecmp(sep->arg[1], "list") == 0)
	{
		auto dz_list = DynamicZonesRepository::AllDzInstancePlayerCounts(database);
		c->Message(Chat::White, fmt::format("Total Dynamic Zones: [{}]", dz_list.size()).c_str());

		auto now = std::chrono::system_clock::now();

		for (const auto& dz : dz_list)
		{
			auto expire_time = std::chrono::system_clock::from_time_t(dz.start_time + dz.duration);
			auto remaining = std::chrono::duration_cast<std::chrono::seconds>(expire_time - now);
			auto seconds = std::max(0, static_cast<int>(remaining.count()));
			bool is_expired = now > expire_time;

			if (!is_expired || strcasecmp(sep->arg[2], "all") == 0)
			{
				auto zone_saylink = is_expired ? "zone" : EQ::SayLinkEngine::GenerateQuestSaylink(
					fmt::format("#zoneinstance {}", dz.instance), false, "zone");

				c->Message(Chat::White, fmt::format(
					"dz id: [{}] type: [{}] {}: [{}]:[{}]:[{}] members: [{}] remaining: [{:02}:{:02}:{:02}]",
					dz.id,
					DynamicZone::GetDynamicZoneTypeName(static_cast<DynamicZoneType>(dz.type)),
					zone_saylink,
					dz.zone,
					dz.instance,
					dz.version,
					dz.member_count,
					seconds / 3600,      // hours
					(seconds / 60) % 60, // minutes
					seconds % 60         // seconds
				).c_str());
			}
		}
	}
	else if (strcasecmp(sep->arg[1], "lockouts") == 0)
	{
		if (strcasecmp(sep->arg[2], "remove") == 0 && sep->arg[3][0] != '\0')
		{
			if (sep->arg[5][0] == '\0')
			{
				c->Message(Chat::White, fmt::format(
					"Removing [{}] lockouts on [{}].", sep->arg[4][0] ? sep->arg[4] : "all", sep->arg[3]
				).c_str());
			}
			else
			{
				c->Message(Chat::White, fmt::format(
					"Removing [{}]:[{}] lockout on [{}].", sep->arg[4], sep->arg[5], sep->arg[3]
				).c_str());
			}
			Expedition::RemoveLockoutsByCharacterName(sep->arg[3], sep->arg[4], sep->arg[5]);
		}
	}
	else if (strcasecmp(sep->arg[1], "makeleader") == 0 && sep->IsNumber(2) && sep->arg[3][0] != '\0')
	{
		auto expedition_id = std::strtoul(sep->arg[2], nullptr, 10);
		auto expedition = Expedition::FindCachedExpeditionByID(expedition_id);
		if (expedition)
		{
			auto char_name = FormatName(sep->arg[3]);
			c->Message(Chat::White, fmt::format("Setting expedition [{}] leader to [{}]", expedition_id, char_name).c_str());
			expedition->SendWorldMakeLeaderRequest(c->CharacterID(), char_name);
		}
		else
		{
			c->Message(Chat::Red, fmt::format("Failed to find expedition [{}]", expedition_id).c_str());
		}
	}
	else
	{
		c->Message(Chat::White, "#dz usage:");
		c->Message(Chat::White, "#dz expedition list - list expeditions in current zone cache");
		c->Message(Chat::White, "#dz expedition reload - reload expedition zone cache from database");
		c->Message(Chat::White, "#dz expedition destroy <expedition_id> - destroy expedition globally (must be in cache)");
		c->Message(Chat::White, "#dz expedition unlock <expedition_id> - unlock expedition");
		c->Message(Chat::White, "#dz list [all] - list dynamic zone instances from database -- 'all' includes expired");
		c->Message(Chat::White, "#dz lockouts remove <char_name> - delete all of character's expedition lockouts");
		c->Message(Chat::White, "#dz lockouts remove <char_name> \"<expedition_name>\" - delete lockouts by expedition");
		c->Message(Chat::White, "#dz lockouts remove <char_name> \"<expedition_name>\" \"<event_name>\" - delete lockout by expedition event");
		c->Message(Chat::White, "#dz makeleader <expedition_id> <character_name> - set new expedition leader");
	}
}

void command_dzkickplayers(Client* c, const Seperator* sep)
{
	if (c)
	{
		auto expedition = c->GetExpedition();
		if (expedition)
		{
			expedition->DzKickPlayers(c);
		}
	}
}

void command_editmassrespawn(Client* c, const Seperator* sep)
{
	if (strcasecmp(sep->arg[1], "usage") == 0) {
		c->Message(Chat::White, "#editmassrespawn [exact_match: =]npc_type_name new_respawn_seconds (apply)");
		return;
	}

	std::string search_npc_type;
	if (sep->arg[1]) {
		search_npc_type = sep->arg[1];
	}

	int change_respawn_seconds = 0;
	if (sep->arg[2] && sep->IsNumber(2)) {
		change_respawn_seconds = atoi(sep->arg[2]);
	}

	bool change_apply = false;
	if (sep->arg[3] && strcasecmp(sep->arg[3], "apply") == 0) {
		change_apply = true;
	}

	std::string search_encapsulator = "%";
	if (search_npc_type[0] == '=') {

		search_npc_type = search_npc_type.substr(1);
		search_encapsulator = "";
	}

	std::string query = fmt::format(
		SQL(
			SELECT npc_types.id, spawn2.spawngroupID, spawn2.id, npc_types.name, spawn2.respawntime
			FROM spawn2
			INNER JOIN spawnentry ON spawn2.spawngroupID = spawnentry.spawngroupID
			INNER JOIN npc_types ON spawnentry.npcID = npc_types.id
			WHERE spawn2.zone LIKE '{}'
			AND spawn2.version = '{}'
			AND npc_types.name LIKE '{}{}{}'
			ORDER BY npc_types.id, spawn2.spawngroupID, spawn2.id
		),
		zone->GetShortName(),
		zone->GetInstanceVersion(),
		search_encapsulator,
		search_npc_type,
		search_encapsulator
	);

	std::string status = "(Searching)";
	if (change_apply) {
		status = "(Applying)";
	}

	int results_count = 0;

	auto results = content_db.QueryDatabase(query);
	if (results.Success() && results.RowCount()) {

		results_count = results.RowCount();

		for (auto row : results) {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"NPC (npcid:{}) (sgid:{}) (s2id:{}) [{}] Respawn: Current [{}] New [{}] {}",
					row[0],
					row[1],
					row[2],
					row[3],
					row[4],
					change_respawn_seconds,
					status
				).c_str()
			);
		}

		c->Message(Chat::Yellow, "Found (%i) NPC's that match this search...", results_count);

		if (change_respawn_seconds > 0) {

			if (change_apply) {

				results = content_db.QueryDatabase(
					fmt::format(
						SQL(
							UPDATE spawn2
							SET respawntime = '{}'
							WHERE id IN (
								SELECT spawn2.id
								FROM spawn2
								INNER JOIN spawnentry ON spawn2.spawngroupID = spawnentry.spawngroupID
								INNER JOIN npc_types ON spawnentry.npcID = npc_types.id
								WHERE spawn2.zone LIKE '{}'
								AND spawn2.version = '{}'
								AND npc_types.name LIKE '{}{}{}'
							)
						),
						change_respawn_seconds,
						zone->GetShortName(),
						zone->GetInstanceVersion(),
						search_encapsulator,
						search_npc_type,
						search_encapsulator
					)
				);

				if (results.Success()) {

					c->Message(Chat::Yellow, "Changes applied to (%i) NPC 'Spawn2' entries", results_count);
					zone->Repop();
				}
				else {

					c->Message(Chat::Yellow, "Found (0) NPC's that match this search...");
				}
			}
			else {

				std::string saylink = fmt::format(
					"#editmassrespawn {}{} {} apply",
					(search_encapsulator.empty() ? "=" : ""),
					search_npc_type,
					change_respawn_seconds
				);

				c->Message(
					Chat::Yellow, "To apply these changes, click <%s> or type [%s]",
					EQ::SayLinkEngine::GenerateQuestSaylink(saylink, false, "Apply").c_str(),
					saylink.c_str()
				);
			}
		}
	}
	else {

		c->Message(Chat::Yellow, "Found (0) NPC's that match this search...");
	}
}

void command_randomfeatures(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	if (!target)
		c->Message(Chat::White,"Error: This command requires a target");
	else
	{
		if (target->RandomizeFeatures())
			c->Message(Chat::White,"Features Randomized");
		else
			c->Message(Chat::White,"This command requires a Playable Race as the target");
	}
}

void command_face(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	if (!sep->IsNumber(1))
		c->Message(Chat::White,"Usage: #face [number of face]");
	else if (!target)
		c->Message(Chat::White,"Error: this command requires a target");
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

		c->Message(Chat::White,"Face = %i",  atoi(sep->arg[1]));
	}
}

void command_findaliases(Client *c, const Seperator *sep)
{
	if (!sep->arg[1][0]) {
		c->Message(Chat::White, "Usage: #findaliases [alias | command]");
		return;
	}

	auto find_iter = commandaliases.find(sep->arg[1]);
	if (find_iter == commandaliases.end()) {
		c->Message(Chat::Yellow, "No commands or aliases match '%s'", sep->arg[1]);
		return;
	}

	auto command_iter = commandlist.find(find_iter->second);
	if (find_iter->second.empty() || command_iter == commandlist.end()) {
		c->Message(Chat::White, "An unknown condition occurred...");
		return;
	}

	c->Message(Chat::White, "Available command aliases for '%s':", command_iter->first.c_str());

	int commandaliasesshown = 0;
	for (auto alias_iter = commandaliases.begin(); alias_iter != commandaliases.end(); ++alias_iter) {
		if (strcasecmp(find_iter->second.c_str(), alias_iter->second.c_str()) || c->Admin() < command_iter->second->access)
			continue;

		c->Message(Chat::White, "%c%s", COMMAND_CHAR, alias_iter->first.c_str());
		++commandaliasesshown;
	}
	c->Message(Chat::White, "%d command alias%s listed.", commandaliasesshown, commandaliasesshown != 1 ? "es" : "");
}

void command_details(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	if (!sep->IsNumber(1))
		c->Message(Chat::White,"Usage: #details [number of drakkin detail]");
	else if (!target)
		c->Message(Chat::White,"Error: this command requires a target");
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

		c->Message(Chat::White,"Details = %i",  atoi(sep->arg[1]));
	}
}

void command_heritage(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	if (!sep->IsNumber(1))
		c->Message(Chat::White,"Usage: #heritage [number of Drakkin heritage]");
	else if (!target)
		c->Message(Chat::White,"Error: this command requires a target");
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

		c->Message(Chat::White,"Heritage = %i",  atoi(sep->arg[1]));
	}
}

void command_tattoo(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	if (!sep->IsNumber(1))
		c->Message(Chat::White,"Usage: #tattoo [number of Drakkin tattoo]");
	else if (!target)
		c->Message(Chat::White,"Error: this command requires a target");
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

		c->Message(Chat::White,"Tattoo = %i",  atoi(sep->arg[1]));
	}
}

void command_helm(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	if (!sep->IsNumber(1))
		c->Message(Chat::White,"Usage: #helm [number of helm texture]");
	else if (!target)
		c->Message(Chat::White,"Error: this command requires a target");
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

		c->Message(Chat::White,"Helm = %i",  atoi(sep->arg[1]));
	}
}

void command_hair(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	if (!sep->IsNumber(1))
		c->Message(Chat::White,"Usage: #hair [number of hair style]");
	else if (!target)
		c->Message(Chat::White,"Error: this command requires a target");
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

		c->Message(Chat::White,"Hair = %i",  atoi(sep->arg[1]));
	}
}

void command_haircolor(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	if (!sep->IsNumber(1))
		c->Message(Chat::White,"Usage: #haircolor [number of hair color]");
	else if (!target)
		c->Message(Chat::White,"Error: this command requires a target");
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

		c->Message(Chat::White,"Hair Color = %i",  atoi(sep->arg[1]));
	}
}

void command_beard(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	if (!sep->IsNumber(1))
		c->Message(Chat::White,"Usage: #beard [number of beard style]");
	else if (!target)
		c->Message(Chat::White,"Error: this command requires a target");
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

		c->Message(Chat::White,"Beard = %i",  atoi(sep->arg[1]));
	}
}

void command_beardcolor(Client *c, const Seperator *sep)
{
	Mob *target=c->GetTarget();
	if (!sep->IsNumber(1))
		c->Message(Chat::White,"Usage: #beardcolor [number of beard color]");
	else if (!target)
		c->Message(Chat::White,"Error: this command requires a target");
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

		c->Message(Chat::White,"Beard Color = %i",  atoi(sep->arg[1]));
	}
}

void command_scribespells(Client *c, const Seperator *sep)
{
	Client *t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM())
		t = c->GetTarget()->CastToClient();

	if(sep->argnum < 1 || !sep->IsNumber(1)) {
		c->Message(Chat::White, "FORMAT: #scribespells <max level> <min level>");
		return;
	}

	uint8 max_level = (uint8)atol(sep->arg[1]);
	if (!c->GetGM() && max_level > (uint8)RuleI(Character, MaxLevel))
		max_level = (uint8)RuleI(Character, MaxLevel); // default to Character:MaxLevel if we're not a GM & it's higher than the max level

	uint8 min_level = (sep->IsNumber(2) ? (uint8)atol(sep->arg[2]) : 1); // default to 1 if there isn't a 2nd argument
	if (!c->GetGM() && min_level > (uint8)RuleI(Character, MaxLevel))
		min_level = (uint8)RuleI(Character, MaxLevel); // default to Character:MaxLevel if we're not a GM & it's higher than the max level

	if(max_level < 1 || min_level < 1) {
		c->Message(Chat::White, "ERROR: Level must be greater than 1.");
		return;
	}
	if (min_level > max_level) {
		c->Message(Chat::White, "ERROR: Min Level must be less than or equal to Max Level.");
		return;
	}

	t->Message(Chat::White, "Scribing spells to spellbook.");
	if(t != c)
		c->Message(Chat::White, "Scribing spells for %s.",  t->GetName());
	LogInfo("Scribe spells request for [{}] from [{}], levels: [{}] -> [{}]",  t->GetName(), c->GetName(), min_level, max_level);

	int book_slot = t->GetNextAvailableSpellBookSlot();
	int spell_id = 0;
	int count = 0;

	for ( ; spell_id < SPDAT_RECORDS && book_slot < EQ::spells::SPELLBOOK_SIZE; ++spell_id) {
		if (book_slot == -1) {
			t->Message(
				13,
				"Unable to scribe spell %s (%i) to spellbook: no more spell book slots available.",
				((spell_id >= 0 && spell_id < SPDAT_RECORDS) ? spells[spell_id].name : "Out-of-range"),
				spell_id
			);
			if (t != c)
				c->Message(
					13,
					"Error scribing spells: %s ran out of spell book slots on spell %s (%i)",
					t->GetName(),
					((spell_id >= 0 && spell_id < SPDAT_RECORDS) ? spells[spell_id].name : "Out-of-range"),
					spell_id
				);

			break;
		}
		if (spell_id < 0 || spell_id >= SPDAT_RECORDS) {
			c->Message(Chat::Red, "FATAL ERROR: Spell id out-of-range (id: %i, min: 0, max: %i)", spell_id, SPDAT_RECORDS);
			return;
		}
		if (book_slot < 0 || book_slot >= EQ::spells::SPELLBOOK_SIZE) {
			c->Message(Chat::Red, "FATAL ERROR: Book slot out-of-range (slot: %i, min: 0, max: %i)", book_slot, EQ::spells::SPELLBOOK_SIZE);
			return;
		}

		while (true) {
			if (spells[spell_id].classes[WARRIOR] == 0) // check if spell exists
				break;
			if (spells[spell_id].classes[t->GetPP().class_ - 1] > max_level) // maximum level
				break;
			if (spells[spell_id].classes[t->GetPP().class_ - 1] < min_level) // minimum level
				break;
			if (spells[spell_id].skill == 52)
				break;

			uint16 spell_id_ = (uint16)spell_id;
			if ((spell_id_ != spell_id) || (spell_id != spell_id_)) {
				c->Message(Chat::Red, "FATAL ERROR: Type conversion data loss with spell_id (%i != %u)", spell_id, spell_id_);
				return;
			}

			if (!IsDiscipline(spell_id_) && !t->HasSpellScribed(spell_id)) { // isn't a discipline & we don't already have it scribed
				t->ScribeSpell(spell_id_, book_slot);
				++count;
			}

			break;
		}

		book_slot = t->GetNextAvailableSpellBookSlot(book_slot);
	}

	if (count > 0) {
		t->Message(Chat::White, "Successfully scribed %i spells.",  count);
		if (t != c)
			c->Message(Chat::White, "Successfully scribed %i spells for %s.",  count, t->GetName());
	}
	else {
		t->Message(Chat::White, "No spells scribed.");
		if (t != c)
			c->Message(Chat::White, "No spells scribed for %s.",  t->GetName());
	}
}

void command_scribespell(Client *c, const Seperator *sep) {
	uint16 spell_id = 0;
	uint16 book_slot = -1;
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM())
		t=c->GetTarget()->CastToClient();

	if(!sep->arg[1][0]) {
		c->Message(Chat::White, "FORMAT: #scribespell <spellid>");
		return;
	}

	spell_id = atoi(sep->arg[1]);

	if(IsValidSpell(spell_id)) {
		t->Message(Chat::White, "Scribing spell: %s (%i) to spellbook.",  spells[spell_id].name, spell_id);

		if(t != c)
			c->Message(Chat::White, "Scribing spell: %s (%i) for %s.",  spells[spell_id].name, spell_id, t->GetName());

		LogInfo("Scribe spell: [{}] ([{}]) request for [{}] from [{}]",  spells[spell_id].name, spell_id, t->GetName(), c->GetName());

		if (spells[spell_id].classes[WARRIOR] != 0 && spells[spell_id].skill != 52 && spells[spell_id].classes[t->GetPP().class_ - 1] > 0 && !IsDiscipline(spell_id)) {
			book_slot = t->GetNextAvailableSpellBookSlot();

			if(book_slot >= 0 && t->FindSpellBookSlotBySpellID(spell_id) < 0)
				t->ScribeSpell(spell_id, book_slot);
			else {
				t->Message(Chat::Red, "Unable to scribe spell: %s (%i) to your spellbook.",  spells[spell_id].name, spell_id);

				if(t != c)
					c->Message(Chat::Red, "Unable to scribe spell: %s (%i) for %s.",  spells[spell_id].name, spell_id, t->GetName());
			}
		}
		else
			c->Message(Chat::Red, "Your target can not scribe this spell.");
	}
	else
		c->Message(Chat::Red, "Spell ID: %i is an unknown spell and cannot be scribed.",  spell_id);
}

void command_unscribespell(Client *c, const Seperator *sep) {
	uint16 spell_id = 0;
	uint16 book_slot = -1;
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM())
		t=c->GetTarget()->CastToClient();

	if(!sep->arg[1][0]) {
		c->Message(Chat::White, "FORMAT: #unscribespell <spellid>");
		return;
	}

	spell_id = atoi(sep->arg[1]);

	if(IsValidSpell(spell_id)) {
		book_slot = t->FindSpellBookSlotBySpellID(spell_id);

		if(book_slot >= 0) {
			t->UnscribeSpell(book_slot);

			t->Message(Chat::White, "Unscribing spell: %s (%i) from spellbook.",  spells[spell_id].name, spell_id);

			if(t != c)
				c->Message(Chat::White, "Unscribing spell: %s (%i) for %s.",  spells[spell_id].name, spell_id, t->GetName());

			LogInfo("Unscribe spell: [{}] ([{}]) request for [{}] from [{}]",  spells[spell_id].name, spell_id, t->GetName(), c->GetName());
		}
		else {
			t->Message(Chat::Red, "Unable to unscribe spell: %s (%i) from your spellbook. This spell is not scribed.",  spells[spell_id].name, spell_id);

			if(t != c)
				c->Message(Chat::Red, "Unable to unscribe spell: %s (%i) for %s due to spell not scribed.",  spells[spell_id].name, spell_id, t->GetName());
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
		c->Message(Chat::White,"You must target an NPC to use this.");
		return;
	}

	NPC *n = t->CastToNPC();
	n->DisplayWaypointInfo(c);
}

void command_wpadd(Client *c, const Seperator *sep)
{
	int type1 = 0, type2 = 0, pause = 0; // Defaults for a new grid
	Mob *target = c->GetTarget();
	if (target && target->IsNPC()) {
		Spawn2 *s2info = target->CastToNPC()->respawn2;
		if (s2info == nullptr) {
			c->Message(
				Chat::White,
				"#wpadd Failed, you must target a valid spawn."
			);
			return;
		}

		if (sep->arg[1][0]) {
			if (atoi(sep->arg[1]) >= 0) {
				pause = atoi(sep->arg[1]);
			} else {
				c->Message(Chat::White, "Usage: #wpadd [pause] [-h]");
				return;
			}
		}
		auto position = c->GetPosition();
		if (strcmp("-h", sep->arg[2]) != 0) {
			position.w = -1;
		}

		auto zone_id = zone->GetZoneID();
		uint32 tmp_grid = content_db.AddWPForSpawn(c, s2info->GetID(), position, pause, type1, type2, zone_id);
		if (tmp_grid) {
			target->CastToNPC()->SetGrid(tmp_grid);
		}

		auto grid_id = target->CastToNPC()->GetGrid();
		target->CastToNPC()->AssignWaypoints(grid_id);
		c->Message(
			Chat::White,
			fmt::format(
				"Waypoint added to grid {} in zone ID {}. Use #wpinfo to see waypoints for this NPC (may need to #repop first).",
				grid_id,
				zone_id
			).c_str()
		);
	} else {
		c->Message(Chat::White, "You must target an NPC to use this.");
	}
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
	uint32 item_id = 0;
	int16 charges = -1;
	uint32 augment_one = 0;
	uint32 augment_two = 0;
	uint32 augment_three = 0;
	uint32 augment_four = 0;
	uint32 augment_five = 0;
	uint32 augment_six = 0;
	int arguments = sep->argnum;
	std::string cmd_msg = sep->msg;
	size_t link_open = cmd_msg.find('\x12');
	size_t link_close = cmd_msg.find_last_of('\x12');
	if (link_open != link_close && (cmd_msg.length() - link_open) > EQ::constants::SAY_LINK_BODY_SIZE) {
		EQ::SayLinkBody_Struct link_body;
		EQ::saylink::DegenerateLinkBody(link_body, cmd_msg.substr(link_open + 1, EQ::constants::SAY_LINK_BODY_SIZE));
		item_id = link_body.item_id;
		augment_one = link_body.augment_1;
		augment_two = link_body.augment_2;
		augment_three = link_body.augment_3;
		augment_four = link_body.augment_4;
		augment_five = link_body.augment_5;
		augment_six = link_body.augment_6;
	} else if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #summonitem [item id | link] [charges] [augment_one_id] [augment_two_id] [augment_three_id] [augment_four_id] [augment_five_id] [augment_six_id] (Charges are optional.)");
		return;
	} else {
		item_id = atoi(sep->arg[1]);
	}

	if (!item_id) {
		c->Message(Chat::White, "Enter a valid item ID.");
		return;
	}

	uint8 item_status = 0;
	uint8 current_status = c->Admin();
	const EQ::ItemData* item = database.GetItem(item_id);
	if (item) {
		item_status = item->MinStatus;
	}

	if (item_status > current_status) {
		c->Message(
			Chat::White,
			fmt::format(
				"Insufficient status to summon this item, current status is {}, required status is {}.",
				current_status,
				item_status
			).c_str()
		);
	}
	
	if (arguments >= 2 && sep->IsNumber(2)) {
		charges = atoi(sep->arg[2]);
	}
	
	if (arguments >= 3 && sep->IsNumber(3)) {
		augment_one = atoi(sep->arg[3]);
	}
	
	if (arguments >= 4 && sep->IsNumber(4)) {
		augment_two = atoi(sep->arg[4]);
	}
	
	if (arguments >= 5 && sep->IsNumber(5)) {
		augment_three = atoi(sep->arg[5]);
	}
	
	if (arguments >= 6 && sep->IsNumber(6)) {
		augment_four = atoi(sep->arg[6]);
	}
	
	if (arguments >= 7 && sep->IsNumber(7)) {
		augment_five = atoi(sep->arg[7]);
	}
	
	if (arguments == 8 && sep->IsNumber(8)) {
		augment_six = atoi(sep->arg[8]);
	}

	c->SummonItem(item_id, charges, augment_one, augment_two, augment_three, augment_four, augment_five, augment_six);
}

void command_giveitem(Client *c, const Seperator *sep)
{
	uint32 item_id = 0;
	int16 charges = -1;
	uint32 augment_one = 0;
	uint32 augment_two = 0;
	uint32 augment_three = 0;
	uint32 augment_four = 0;
	uint32 augment_five = 0;
	uint32 augment_six = 0;
	int arguments = sep->argnum;
	std::string cmd_msg = sep->msg;
	size_t link_open = cmd_msg.find('\x12');
	size_t link_close = cmd_msg.find_last_of('\x12');
	if (c->GetTarget()) {
 		if (!c->GetTarget()->IsClient()) {
			c->Message(Chat::Red, "You can only give items to players with this command.");
			return;
		}

		if (link_open != link_close && (cmd_msg.length() - link_open) > EQ::constants::SAY_LINK_BODY_SIZE) {
			EQ::SayLinkBody_Struct link_body;
			EQ::saylink::DegenerateLinkBody(link_body, cmd_msg.substr(link_open + 1, EQ::constants::SAY_LINK_BODY_SIZE));
			item_id = link_body.item_id;
			augment_one = link_body.augment_1;
			augment_two = link_body.augment_2;
			augment_three = link_body.augment_3;
			augment_four = link_body.augment_4;
			augment_five = link_body.augment_5;
			augment_six = link_body.augment_6;
		} else if (sep->IsNumber(1)) {
			item_id = atoi(sep->arg[1]);
		} else if (!sep->IsNumber(1)) {
			c->Message(Chat::Red, "Usage: #giveitem [item id | link] [charges] [augment_one_id] [augment_two_id] [augment_three_id] [augment_four_id] [augment_five_id] [augment_six_id] (Charges are optional.)");
			return;
		}
		
		Client *client_target = c->GetTarget()->CastToClient();
		uint8 item_status = 0;
		uint8 current_status = c->Admin();
		const EQ::ItemData* item = database.GetItem(item_id);
		if (item) {
			item_status = item->MinStatus;
		}
		
		if (item_status > current_status) {
			c->Message(
				Chat::White,
				fmt::format(
					"Insufficient status to summon this item, current status is {}, required status is {}.",
					current_status,
					item_status
				).c_str()
			);
			return;
		}
			
		if (arguments >= 2 && sep->IsNumber(2)) {
			charges = atoi(sep->arg[2]);
		}
		
		if (arguments >= 3 && sep->IsNumber(3)) {
			augment_one = atoi(sep->arg[3]);
		}
		
		if (arguments >= 4 && sep->IsNumber(4)) {
			augment_two = atoi(sep->arg[4]);
		}
		
		if (arguments >= 5 && sep->IsNumber(5)) {
			augment_three = atoi(sep->arg[5]);
		}
		
		if (arguments >= 6 && sep->IsNumber(6)) {
			augment_four = atoi(sep->arg[6]);
		}

		if (arguments >= 7 && sep->IsNumber(7)) {
			augment_five = atoi(sep->arg[7]);
		}

		if (arguments == 8 && sep->IsNumber(8)) {
			augment_six = atoi(sep->arg[8]);
		}

		client_target->SummonItem(item_id, charges, augment_one, augment_two, augment_three, augment_four, augment_five, augment_six);
	} else {
		c->Message(Chat::Red, "You must target a client to give the item to.");
		return;
	}
}

void command_givemoney(Client *c, const Seperator *sep)
{
	if (!sep->IsNumber(1)) {	//as long as the first one is a number, we'll just let atoi convert the rest to 0 or a number
		c->Message(Chat::Red, "Usage: #Usage: #givemoney [pp] [gp] [sp] [cp]");
	}
	else if(c->GetTarget() == nullptr) {
		c->Message(Chat::Red, "You must target a player to give money to.");
	}
	else if(!c->GetTarget()->IsClient()) {
		c->Message(Chat::Red, "You can only give money to players with this command.");
	}
	else {
		//TODO: update this to the client, otherwise the client doesn't show any weight change until you zone, move an item, etc
		c->GetTarget()->CastToClient()->AddMoneyToPP(atoi(sep->arg[4]), atoi(sep->arg[3]), atoi(sep->arg[2]), atoi(sep->arg[1]), true);
		c->Message(Chat::White, "Added %i Platinum, %i Gold, %i Silver, and %i Copper to %s's inventory.",  atoi(sep->arg[1]), atoi(sep->arg[2]), atoi(sep->arg[3]), atoi(sep->arg[4]), c->GetTarget()->GetName());
	}
}

void command_itemsearch(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0)
		c->Message(Chat::White, "Usage: #itemsearch [search string]");
	else
	{
		const char *search_criteria=sep->argplus[1];

		const EQ::ItemData* item = nullptr;
		EQ::SayLinkEngine linker;
		linker.SetLinkType(EQ::saylink::SayLinkItemData);

		if (Seperator::IsNumber(search_criteria)) {
			item = database.GetItem(atoi(search_criteria));
			if (item) {
				linker.SetItemData(item);

				c->Message(Chat::White, "%u: %s",  item->ID, linker.GenerateLink().c_str());
			}
			else {
				c->Message(Chat::White, "Item #%s not found",  search_criteria);
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
				std::string item_id = std::to_string(item->ID);
				std::string saylink_commands =
					"[" +
					EQ::SayLinkEngine::GenerateQuestSaylink(
						"#si " + item_id,
						false,
						"X"
					) +
					"] ";
				if (item->Stackable && item->StackSize > 1) {
					std::string stack_size = std::to_string(item->StackSize);
					saylink_commands +=
					"[" +
					EQ::SayLinkEngine::GenerateQuestSaylink(
						"#si " + item_id + " " + stack_size,
						false,
						stack_size
					) +
					"]";
				}

				c->Message(
					Chat::White,
					fmt::format(
						" Summon {} [{}] [{}]",
						saylink_commands,
						linker.GenerateLink(),
						item->ID
					).c_str()
				);

				++count;
			}

			if (count == 50)
				break;
		}

		if (count == 50)
			c->Message(Chat::White, "50 items shown...too many results.");
		else
			c->Message(Chat::White, "%i items found",  count);

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
		c->Message(Chat::White, "Usage: #setaaxp <new AA XP value> (<new Group AA XP value> <new Raid XP value>)");
}

void command_setaapts(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	if(sep->arg[1][0] == '\0' || sep->arg[2][0] == '\0')
		c->Message(Chat::White, "Usage: #setaapts <AA|group|raid> <new AA points value>");
	else if(atoi(sep->arg[2]) <= 0 || atoi(sep->arg[2]) > 5000)
		c->Message(Chat::White, "You must have a number greater than 0 for points and no more than 5000.");
	else if(!strcasecmp(sep->arg[1], "group")) {
		t->GetPP().group_leadership_points = atoi(sep->arg[2]);
		t->GetPP().group_leadership_exp = 0;
		t->Message(Chat::Experience, "Setting Group AA points to %u", t->GetPP().group_leadership_points);
		t->SendLeadershipEXPUpdate();
	} else if(!strcasecmp(sep->arg[1], "raid")) {
		t->GetPP().raid_leadership_points = atoi(sep->arg[2]);
		t->GetPP().raid_leadership_exp = 0;
		t->Message(Chat::Experience, "Setting Raid AA points to %u", t->GetPP().raid_leadership_points);
		t->SendLeadershipEXPUpdate();
	} else {
		t->GetPP().aapoints = atoi(sep->arg[2]);
		t->GetPP().expAA = 0;
		t->Message(Chat::Experience, "Setting personal AA points to %u", t->GetPP().aapoints);
		t->SendAlternateAdvancementStats();
	}
}

void command_setcrystals(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	if(sep->arg[1][0] == '\0' || sep->arg[2][0] == '\0')
		c->Message(Chat::White, "Usage: #setcrystals <radiant|ebon> <new crystal count value>");
	else if(atoi(sep->arg[2]) <= 0 || atoi(sep->arg[2]) > 100000)
		c->Message(Chat::White, "You must have a number greater than 0 for crystals and no more than 100000.");
	else if(!strcasecmp(sep->arg[1], "radiant"))
	{
		t->SetRadiantCrystals(atoi(sep->arg[2]));
	}
	else if(!strcasecmp(sep->arg[1], "ebon"))
	{
		t->SetEbonCrystals(atoi(sep->arg[2]));
	}
	else
	{
		c->Message(Chat::White, "Usage: #setcrystals <radiant|ebon> <new crystal count value>");
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
		c->Message(Chat::White, "Usage: #stun [duration]");
}


void command_ban(Client *c, const Seperator *sep)
{
if(sep->arg[1][0] == 0 || sep->arg[2][0] == 0) {
		c->Message(Chat::White, "Usage: #ban <charname> <message>");
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
        c->Message(Chat::White, "Usage: #ban <charname> <message>");
        return;
    }

    if(account_id == 0) {
        c->Message(Chat::Red, "Character does not exist.");
        return;
    }

    std::string query = StringFormat("UPDATE account SET status = -2, ban_reason = '%s' "
                                    "WHERE id = %i",  EscapeString(message).c_str(), account_id);
    auto results = database.QueryDatabase(query);

    c->Message(Chat::Red, "Account number %i with the character %s has been banned with message: \"%s\"",  account_id, sep->arg[1], message.c_str());

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
		c->Message(Chat::White, "Usage: #suspend <charname> <days> (Specify 0 days to lift the suspension immediately) <message>");
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
            c->Message(Chat::White, "Usage: #suspend <charname> <days>(Specify 0 days to lift the suspension immediately) <message>");
            return;
        }
    }

    auto escName = new char[strlen(sep->arg[1]) * 2 + 1];
    database.DoEscapeString(escName, sep->arg[1], strlen(sep->arg[1]));
    int accountID = database.GetAccountIDByChar(escName);
    safe_delete_array(escName);

    if (accountID <= 0) {
        c->Message(Chat::Red,"Character does not exist.");
        return;
    }

	std::string query = StringFormat("UPDATE `account` SET `suspendeduntil` = DATE_ADD(NOW(), INTERVAL %i DAY), "
                                    "suspend_reason = '%s' WHERE `id` = %i",
                                    duration, EscapeString(message).c_str(), accountID);
    auto results = database.QueryDatabase(query);

    if(duration)
        c->Message(Chat::Red,"Account number %i with the character %s has been temporarily suspended for %i day(s).",  accountID, sep->arg[1], duration);
    else
        c->Message(Chat::Red,"Account number %i with the character %s is no longer suspended.",  accountID, sep->arg[1]);

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
		c->Message(Chat::White, "Usage: #ipban [xxx.xxx.xxx.xxx]");
	} else {
		if(database.AddBannedIP(sep->arg[1], c->GetName())) {
			c->Message(Chat::White, "%s has been successfully added to the banned_ips table by %s", sep->arg[1], c->GetName());
		} else {
			c->Message(Chat::White, "IPBan Failed (IP address is possibly already in the table?)");
		}
	}
}

void command_revoke(Client *c, const Seperator *sep)
{
	if(sep->arg[1][0] == 0 || sep->arg[2][0] == 0) {
		c->Message(Chat::White, "Usage: #revoke [charname] [1/0]");
		return;
	}

    uint32 characterID = database.GetAccountIDByChar(sep->arg[1]);
    if(characterID == 0) {
        c->Message(Chat::Red,"Character does not exist.");
        return;
    }

    int flag = sep->arg[2][0] == '1' ? true : false;
    std::string query = StringFormat("UPDATE account SET revoked = %d WHERE id = %i",  flag, characterID);
    auto results = database.QueryDatabase(query);

    c->Message(Chat::Red,"%s account number %i with the character %s.",  flag? "Revoking": "Unrevoking",  characterID, sep->arg[1]);

    Client* revokee = entity_list.GetClientByAccID(characterID);
    if(revokee) {
        c->Message(Chat::White, "Found %s in this zone.",  revokee->GetName());
        revokee->SetRevoked(flag);
        return;
    }

	c->Message(Chat::Red, "#revoke: Couldn't find %s in this zone, passing request to worldserver.", sep->arg[1]);

	auto outapp = new ServerPacket(ServerOP_Revoke, sizeof(RevokeStruct));
	RevokeStruct *revoke = (RevokeStruct *)outapp->pBuffer;
	strn0cpy(revoke->adminname, c->GetName(), 64);
	strn0cpy(revoke->name, sep->arg[1], 64);
	revoke->toggle = flag;
	worldserver.SendPacket(outapp);
	safe_delete(outapp);
}

void command_roambox(Client *c, const Seperator *sep)
{
	std::string arg1  = sep->arg[1];

	Mob *target = c->GetTarget();
	if (!target || !target->IsNPC()) {
		c->Message(Chat::Red, "You need a valid NPC target for this command");
		return;
	}

	NPC *npc           = dynamic_cast<NPC *>(target);
	int spawn_group_id = npc->GetSpawnGroupId();
	if (spawn_group_id <= 0) {
		c->Message(Chat::Red, "NPC needs a valid SpawnGroup!");
		return;
	}

	if (arg1 == "set") {
		int box_size = (sep->arg[2] ? atoi(sep->arg[2]) : 0);
		int delay = (sep->arg[3] ? atoi(sep->arg[3]) : 15000);
		if (box_size > 0) {
			std::string query = fmt::format(
				SQL(
					UPDATE spawngroup SET
					dist = {},
					min_x = {},
					max_x = {},
					min_y = {},
					max_y = {},
					delay = {}
					WHERE id = {}
				),
				(box_size / 2),
				npc->GetX() - (box_size / 2),
				npc->GetX() + (box_size / 2),
				npc->GetY() - (box_size / 2),
				npc->GetY() + (box_size / 2),
				delay,
				spawn_group_id
			);

			database.QueryDatabase(query);

			c->Message(
				Chat::Yellow,
				"NPC (%s) Roam Box set to box size of [%i] SpawnGroupId [%i] delay [%i]",
				npc->GetCleanName(),
				box_size,
				spawn_group_id,
				delay
			);

			return;
		}

		c->Message(Chat::Red, "Box size must be set!");
	}

	if (arg1 == "remove") {
		std::string query = fmt::format(
			SQL(
				UPDATE spawngroup SET
				dist = 0,
				min_x = 0,
				max_x = 0,
				min_y = 0,
				max_y = 0,
				delay = 0
					WHERE id = {}
			),
			spawn_group_id
		);

		database.QueryDatabase(query);

		c->Message(
			Chat::Yellow,
			"NPC (%s) Roam Box has been removed from SpawnGroupID [%i]",
			npc->GetCleanName(),
			spawn_group_id
		);

		return;
	}

	c->Message(Chat::Yellow, "> Command Usage");
	c->Message(Chat::Yellow, "#roambox set box_size [delay = 0]");
	c->Message(Chat::Yellow, "#roambox remove");
}

void command_oocmute(Client *c, const Seperator *sep)
{
	if(sep->arg[1][0] == 0 || !(sep->arg[1][0] == '1' || sep->arg[1][0] == '0'))
		c->Message(Chat::White, "Usage: #oocmute [1/0]");
	else {
		auto outapp = new ServerPacket(ServerOP_OOCMute, 1);
		*(outapp->pBuffer) = atoi(sep->arg[1]);
		worldserver.SendPacket(outapp);
		safe_delete(outapp);
	}
}

void command_checklos(Client *c, const Seperator *sep)
{
	if (c->GetTarget()) {
		if (c->CheckLosFN(c->GetTarget())) {
			c->Message(Chat::White, "You have LOS to %s", c->GetTarget()->GetName());
		}
		else {
			c->Message(Chat::White, "You do not have LOS to %s", c->GetTarget()->GetName());
		}
	}
	else {
		c->Message(Chat::White, "ERROR: Target required");
	}
}

void command_set_adventure_points(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient())
		t=c->GetTarget()->CastToClient();

	if(!sep->arg[1][0])
	{
		c->Message(Chat::White, "Usage: #setadventurepoints [theme] [points]");
		return;
	}

	if(!sep->IsNumber(1) || !sep->IsNumber(2))
	{
		c->Message(Chat::White, "Usage: #setadventurepoints [theme] [points]");
		return;
	}

	c->Message(Chat::White, "Updating adventure points for %s",  t->GetName());
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
		c->Message(Chat::White, "Usage: #npcsay message (requires NPC target");
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
		c->Message(Chat::White, "Usage: #npcshout message (requires NPC target");
	}
}

void command_timers(Client *c, const Seperator *sep) {
	if(!c->GetTarget() || !c->GetTarget()->IsClient()) {
		c->Message(Chat::White,"Need a player target for timers.");
		return;
	}
	Client *them = c->GetTarget()->CastToClient();

	std::vector< std::pair<pTimerType, PersistentTimer *> > res;
	them->GetPTimers().ToVector(res);

	c->Message(Chat::White,"Timers for target:");

	int r;
	int l = res.size();
	for(r = 0; r < l; r++) {
		c->Message(Chat::White,"Timer %d: %d seconds remain.",  res[r].first, res[r].second->GetRemainingTime());
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
		c->Message(Chat::White, "Usage: #npcemote message (requires NPC target");
	}
}

void command_npceditmass(Client *c, const Seperator *sep)
{
	if (strcasecmp(sep->arg[1], "usage") == 0) {
		c->Message(Chat::White, "#npceditmass search_column [exact_match: =]search_value change_column change_value (apply)");
		return;
	}

	std::string query = SQL(
		SELECT
				COLUMN_NAME
		FROM
		INFORMATION_SCHEMA.COLUMNS
		WHERE
			table_name = 'npc_types'
		AND
		COLUMN_NAME != 'id'
	);

	std::string search_column, search_value, change_column, change_value;
	if (sep->arg[1]) {
		search_column = sep->arg[1];
	}
	if (sep->arg[2]) {
		search_value = sep->arg[2];
	}
	if (sep->arg[3]) {
		change_column = sep->arg[3];
	}
	if (sep->arg[4]) {
		change_value = sep->arg[4];
	}

	bool valid_change_column = false;
	bool valid_search_column = false;
	auto results             = content_db.QueryDatabase(query);

	std::vector <std::string> possible_column_options;

	for (auto row = results.begin(); row != results.end(); ++row) {
		if (row[0] == change_column) {
			valid_change_column = true;
		}
		if (row[0] == search_column) {
			valid_search_column = true;
		}

		possible_column_options.push_back(row[0]);
	}

	std::string options_glue = ", ";

	if (!valid_search_column) {
		c->Message(Chat::Red, "You must specify a valid search column. [%s] is not valid", search_column.c_str());
		c->Message(Chat::Yellow, "Possible columns [%s]", implode(options_glue, possible_column_options).c_str());
		return;
	}

	if (!valid_change_column) {
		c->Message(Chat::Red, "You must specify a valid change column. [%s] is not valid", change_column.c_str());
		c->Message(Chat::Yellow, "Possible columns [%s]", implode(options_glue, possible_column_options).c_str());
		return;
	}

	if (!valid_search_column || !valid_change_column) {
		c->Message(Chat::Red, "One requested column is invalid");
		return;
	}

	query = fmt::format(
		SQL(
			select
			id,
			name,
			{0},
			{1}
				from
				npc_types
				where
				id IN(
					select
				spawnentry.npcID
				from
					spawnentry
				join spawn2 on spawn2.spawngroupID = spawnentry.spawngroupID
				where
				spawn2.zone = '{2}' and spawn2.version = {3}
			)
		),
		search_column,
		change_column,
		zone->GetShortName(),
		zone->GetInstanceVersion()
	);

	std::string status = "(Searching)";

	if (strcasecmp(sep->arg[5], "apply") == 0) {
		status = "(Applying)";
	}

	std::vector <std::string> npc_ids;

	bool exact_match = false;
	if (search_value[0] == '=') {
		exact_match = true;
		search_value = search_value.substr(1);
	}

	int found_count = 0;
	results = content_db.QueryDatabase(query);
	for (auto row = results.begin(); row != results.end(); ++row) {

		std::string npc_id                      = row[0];
		std::string npc_name                    = row[1];
		std::string search_column_value         = str_tolower(row[2]);
		std::string change_column_current_value = row[3];

		if (exact_match) {
			if (search_column_value.compare(search_value) != 0) {
				continue;
			}
		}
		else {
			if (search_column_value.find(search_value) == std::string::npos) {
				continue;
			}
		}

		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ({0}) [{1}] ({2}) [{3}] Current ({4}) [{5}] New [{6}] {7}",
				npc_id,
				npc_name,
				search_column,
				search_column_value,
				change_column,
				change_column_current_value,
				change_value,
				status
			).c_str()
		);

		npc_ids.push_back(npc_id);

		found_count++;
	}

	std::string saylink = fmt::format(
		"#npceditmass {} {}{} {} {} apply",
		search_column,
		(exact_match ? "=" : ""),
		search_value,
		change_column,
		change_value
	);

	if (strcasecmp(sep->arg[5], "apply") == 0) {
		std::string npc_ids_string = implode(",", npc_ids);
		if (npc_ids_string.empty()) {
			c->Message(Chat::Red, "Error: Ran into an unknown error compiling NPC IDs");
			return;
		}

		content_db.QueryDatabase(
			fmt::format(
				"UPDATE `npc_types` SET {} = '{}' WHERE id IN ({})",
				change_column,
				change_value,
				npc_ids_string
			)
		);

		c->Message(Chat::Yellow, "Changes applied to (%i) NPC's", found_count);
		zone->Repop();
	}
	else {
		c->Message(Chat::Yellow, "Found (%i) NPC's that match this search...", found_count);

		if (found_count > 0) {
			c->Message(
				Chat::Yellow, "To apply these changes, click <%s> or type [%s]",
				EQ::SayLinkEngine::GenerateQuestSaylink(saylink, false, "Apply").c_str(),
				saylink.c_str()
			);
		}
	}
}

void command_npcedit(Client *c, const Seperator *sep)
{	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "Error: Must have NPC targeted");
		return;
	}

	if (strcasecmp(sep->arg[1], "help") == 0) {

		c->Message(Chat::White, "Help File for #npcedit. Syntax for commands are:");
		c->Message(Chat::White, "#npcedit Name - Sets an NPC's name");
		c->Message(Chat::White, "#npcedit Lastname - Sets an NPC's lastname");
		c->Message(Chat::White, "#npcedit Level - Sets an NPC's level");
		c->Message(Chat::White, "#npcedit Maxlevel - Sets an NPC's maximum level");
		c->Message(Chat::White, "#npcedit Race - Sets an NPC's race");
		c->Message(Chat::White, "#npcedit Class - Sets an NPC's class");
		c->Message(Chat::White, "#npcedit Bodytype - Sets an NPC's bodytype");
		c->Message(Chat::White, "#npcedit HP - Sets an NPC's hitpoints");
		c->Message(Chat::White, "#npcedit Gender - Sets an NPC's gender");
		c->Message(Chat::White, "#npcedit Texture - Sets an NPC's texture");
		c->Message(Chat::White, "#npcedit Helmtexture - Sets an NPC's helmet texture");
		c->Message(Chat::White, "#npcedit Armtexture - Sets an NPC's arm texture");
		c->Message(Chat::White, "#npcedit Bracertexture - Sets an NPC's bracer texture");
		c->Message(Chat::White, "#npcedit Handtexture - Sets an NPC's hand texture");
		c->Message(Chat::White, "#npcedit Legtexture - Sets an NPC's leg texture");
		c->Message(Chat::White, "#npcedit Feettexture - Sets an NPC's feettexture");
		c->Message(Chat::White, "#npcedit Herosforgemodel - Sets an NPC's Hero's Forge Model");
		c->Message(Chat::White, "#npcedit Size - Sets an NPC's size");
		c->Message(Chat::White, "#npcedit Hpregen - Sets an NPC's hitpoint regen rate per tick");
		c->Message(Chat::White, "#npcedit Manaregen - Sets an NPC's mana regen rate per tick");
		c->Message(Chat::White, "#npcedit Loottable - Sets the loottable ID for an NPC ");
		c->Message(Chat::White, "#npcedit Merchantid - Sets the merchant ID for an NPC");
		c->Message(Chat::White, "#npcedit alt_currency_id - Sets the Alternate Currency ID for an alterative currency Merchant");
		c->Message(Chat::White, "#npcedit npc_spells_effects_id - Sets the NPC Spell Effects ID");
		c->Message(Chat::White, "#npcedit adventure_template_id - Sets the NPC's Adventure Template ID");
		c->Message(Chat::White, "#npcedit trap_template - Sets the NPC's Trap Template ID");
		c->Message(Chat::White, "#npcedit special_abilities - Sets the NPC's Special Abilities");
		c->Message(Chat::White, "#npcedit Spell - Sets the npc spells list ID for an NPC");
		c->Message(Chat::White, "#npcedit Faction - Sets the NPC's faction id");
		c->Message(Chat::White, "#npcedit Damage - Sets an NPC's damage");
		c->Message(Chat::White, "#npcedit Meleetype - Sets an NPC's melee types");
		c->Message(Chat::White, "#npcedit Rangedtype - Sets an NPC's ranged type");
		c->Message(Chat::White, "#npcedit Ammoidfile - Sets an NPC's ammo id file");
		c->Message(Chat::White, "#npcedit Aggroradius - Sets an NPC's aggro radius");
		c->Message(Chat::White, "#npcedit Assistradius - Sets an NPC's assist radius");
		c->Message(Chat::White, "#npcedit Social - Set to 1 if an NPC should assist others on its faction");
		c->Message(Chat::White, "#npcedit Runspeed - Sets an NPC's run speed");
		c->Message(Chat::White, "#npcedit Walkspeed - Sets an NPC's walk speed");
		c->Message(Chat::White, "#npcedit AGI - Sets an NPC's Agility");
		c->Message(Chat::White, "#npcedit CHA - Sets an NPC's Charisma");
		c->Message(Chat::White, "#npcedit DEX - Sets an NPC's Dexterity");
		c->Message(Chat::White, "#npcedit INT - Sets an NPC's Intelligence");
		c->Message(Chat::White, "#npcedit STA - Sets an NPC's Stamina");
		c->Message(Chat::White, "#npcedit STR - Sets an NPC's Strength");
		c->Message(Chat::White, "#npcedit WIS - Sets an NPC's Wisdom");
		c->Message(Chat::White, "#npcedit MR - Sets an NPC's Magic Resistance");
		c->Message(Chat::White, "#npcedit PR - Sets an NPC's Poison Resistance");
		c->Message(Chat::White, "#npcedit DR - Sets an NPC's Disease Resistance");
		c->Message(Chat::White, "#npcedit FR - Sets an NPC's Fire Resistance");
		c->Message(Chat::White, "#npcedit CR - Sets an NPC's Cold Resistance");
		c->Message(Chat::White, "#npcedit Corrup - Sets an NPC's Corruption Resistance");
		c->Message(Chat::White, "#npcedit PhR - Sets and NPC's Physical Resistance");
		c->Message(Chat::White, "#npcedit Seeinvis - Sets an NPC's ability to see invis");
		c->Message(Chat::White, "#npcedit Seeinvisundead - Sets an NPC's ability to see through invis vs. undead");
		c->Message(Chat::White, "#npcedit Seehide - Sets an NPC's ability to see through hide");
		c->Message(Chat::White, "#npcedit Seeimprovedhide - Sets an NPC's ability to see through improved hide");
		c->Message(Chat::White, "#npcedit AC - Sets an NPC's Armor Class");
		c->Message(Chat::White, "#npcedit ATK - Sets an NPC's Attack");
		c->Message(Chat::White, "#npcedit Accuracy - Sets an NPC's Accuracy");
		c->Message(Chat::White, "#npcedit Avoidance - Sets an NPC's Avoidance");
		c->Message(Chat::White, "#npcedit npcaggro - Sets an NPC's npc_aggro flag");
		c->Message(Chat::White, "#npcedit qglobal - Sets an NPC's quest global flag");
		c->Message(Chat::White, "#npcedit spawn_limit - Sets an NPC's spawn limit counter");
		c->Message(Chat::White, "#npcedit Attackspeed - Sets an NPC's attack speed modifier");
		c->Message(Chat::White, "#npcedit Attackdelay - Sets an NPC's attack delay");
		c->Message(Chat::White, "#npcedit Attackcount - Sets an NPC's attack count");
		c->Message(Chat::White, "#npcedit findable - Sets an NPC's findable flag");
		c->Message(Chat::White, "#npcedit trackable - Sets an NPC's trackable flag");
		c->Message(Chat::White, "#npcedit weapon - Sets an NPC's primary and secondary weapon model");
		c->Message(Chat::White, "#npcedit featuresave - Saves all current facial features to the database");
		c->Message(Chat::White, "#npcedit color - Sets an NPC's red, green, and blue armor tint");
		c->Message(Chat::White, "#npcedit armortint_id - Set an NPC's Armor tint ID");
		c->Message(Chat::White, "#npcedit setanimation - Set an NPC's animation on spawn (Stored in spawn2 table)");
		c->Message(Chat::White, "#npcedit scalerate - Set an NPC's scaling rate");
		c->Message(Chat::White, "#npcedit healscale - Set an NPC's heal scaling rate");
		c->Message(Chat::White, "#npcedit spellscale - Set an NPC's spell scaling rate");
		c->Message(Chat::White, "#npcedit no_target - Set an NPC's ability to be targeted with the target hotkey");
		c->Message(Chat::White, "#npcedit version - Set an NPC's version");
		c->Message(Chat::White, "#npcedit slow_mitigation - Set an NPC's slow mitigation");
		c->Message(Chat::White, "#npcedit flymode - Set an NPC's flymode [0 = ground, 1 = flying, 2 = levitate, 3 = water, 4 = floating]");
		c->Message(Chat::White, "#npcedit raidtarget - Set an NPCs raid_target field");
		c->Message(Chat::White, "#npcedit rarespawn - Set an NPCs rare flag");
		c->Message(Chat::White, "#npcedit respawntime - Set an NPCs respawn timer in seconds");

	}

	uint32 npcTypeID = c->GetTarget()->CastToNPC()->GetNPCTypeID();
	if (strcasecmp(sep->arg[1], "name") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has the name %s.", npcTypeID, sep->argplus[2]);
		std::string query = StringFormat("UPDATE npc_types SET name = '%s' WHERE id = %i",  sep->argplus[2],npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "lastname") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has the lastname %s.", npcTypeID, sep->argplus[2]);
		std::string query = StringFormat("UPDATE npc_types SET lastname = '%s' WHERE id = %i", sep->argplus[2],npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "flymode") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has flymode [%s]", npcTypeID, sep->argplus[2]);
		std::string query = StringFormat("UPDATE npc_types SET flymode = '%s' WHERE id = %i",  sep->argplus[2],npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "race") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has the race %i.", npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET race = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "class") == 0) {
        c->Message(Chat::Yellow,"NPCID %u is now class %i.", npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET class = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "bodytype") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has type %i bodytype.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET bodytype = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "hp") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has %i Hitpoints.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET hp = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "gender") == 0) {
        c->Message(Chat::Yellow,"NPCID %u is now gender %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET gender = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "texture") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now uses texture %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET texture = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "helmtexture") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now uses helmtexture %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET helmtexture = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "armtexture") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now uses armtexture %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET armtexture = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "bracertexture") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now uses bracertexture %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET bracertexture = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "handtexture") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now uses handtexture %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET handtexture = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "legtexture") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now uses legtexture %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET legtexture = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "feettexture") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now uses feettexture %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET feettexture = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "herosforgemodel") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now uses herosforgemodel %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET herosforgemodel = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "size") == 0) {
        c->Message(Chat::Yellow,"NPCID %u is now size %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET size = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "hpregen") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now regens %i hitpoints per tick.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET hp_regen_rate = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "manaregen") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now regens %i mana per tick.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET mana_regen_rate = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

    if (strcasecmp(sep->arg[1], "loottable") == 0) {
        c->Message(Chat::Yellow,"NPCID %u is now on loottable_id %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET loottable_id = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "merchantid") == 0) {
        c->Message(Chat::Yellow,"NPCID %u is now merchant_id %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET merchant_id = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "alt_currency_id") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has field 'alt_currency_id' set to %s.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET alt_currency_id = '%s' WHERE id = %i",  sep->argplus[2],npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "npc_spells_effects_id") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has field 'npc_spells_effects_id' set to %s.",  npcTypeID, sep->argplus[2]);
		std::string query = StringFormat("UPDATE npc_types SET npc_spells_effects_id = '%s' WHERE id = %i",  sep->argplus[2],npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "adventure_template_id") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has field 'adventure_template_id' set to %s.",  npcTypeID, sep->argplus[2]);
		std::string query = StringFormat("UPDATE npc_types SET adventure_template_id = '%s' WHERE id = %i",  sep->argplus[2],npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "trap_template") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has field 'trap_template' set to %s.",  npcTypeID, sep->argplus[2]);
		std::string query = StringFormat("UPDATE npc_types SET trap_template = '%s' WHERE id = %i",  sep->argplus[2],npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "special_abilities") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has field 'special_abilities' set to %s.",  npcTypeID, sep->argplus[2]);
		std::string query = StringFormat("UPDATE npc_types SET special_abilities = '%s' WHERE id = %i",  sep->argplus[2],npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "spell") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now uses spell list %i",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET npc_spells_id = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "faction") == 0) {
        c->Message(Chat::Yellow,"NPCID %u is now faction %i",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET npc_faction_id = %i WHERE id = %i",  atoi(sep->argplus[2]),npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "damage") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now hits from %i to %i",  npcTypeID, atoi(sep->arg[2]), atoi(sep->arg[3]));
		std::string query = StringFormat("UPDATE npc_types SET mindmg = %i, maxdmg = %i WHERE id = %i",  atoi(sep->arg[2]), atoi(sep->arg[3]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "meleetype") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has a primary melee type of %i and a secondary melee type of %i.",  npcTypeID, atoi(sep->arg[2]), atoi(sep->arg[3]));
		std::string query = StringFormat("UPDATE npc_types SET prim_melee_type = %i, sec_melee_type = %i WHERE id = %i",  atoi(sep->arg[2]), atoi(sep->arg[3]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "rangedtype") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has a ranged type of %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET ranged_type = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "ammoidfile") == 0) {
        c->Message(Chat::Yellow,"NPCID %u's ammo id file is now %i",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET ammoidfile = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "aggroradius") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has an aggro radius of %i",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET aggroradius = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "assistradius") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has an assist radius of %i",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET assistradius = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "social") == 0) {
        c->Message(Chat::Yellow,"NPCID %u social status is now %i",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET social = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "runspeed") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now runs at %f",  npcTypeID, atof(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET runspeed = %f WHERE id = %i",  atof(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "walkspeed") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now walks at %f",  npcTypeID, atof(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET walkspeed = %f WHERE id = %i",  atof(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "AGI") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has %i Agility.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET AGI = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "CHA") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has %i Charisma.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET CHA = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "DEX") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has %i Dexterity.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET DEX = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "INT") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has %i Intelligence.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET _INT = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "STA") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has %i Stamina.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET STA = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "STR") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has %i Strength.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET STR = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "WIS") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has a Magic Resistance of %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET WIS = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "MR") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has a Magic Resistance of %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET MR = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "DR") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has a Disease Resistance of %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET DR = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "CR") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has a Cold Resistance of %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET CR = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

    if (strcasecmp(sep->arg[1], "FR") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has a Fire Resistance of %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET FR = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

    if (strcasecmp(sep->arg[1], "PR") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has a Poison Resistance of %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET PR = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "Corrup") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has a Corruption Resistance of %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET corrup = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "PhR") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has a Physical Resistance of %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET PhR = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "seeinvis") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has seeinvis set to %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET see_invis = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "seeinvisundead") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has seeinvisundead set to %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET see_invis_undead = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "seehide") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has seehide set to %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET see_hide = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "seeimprovedhide") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has seeimprovedhide set to %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET see_improved_hide = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "AC") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has %i Armor Class.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET ac = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "ATK") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has %i Attack.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET atk = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "Accuracy") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has %i Accuracy.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET accuracy = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "Avoidance") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has %i Avoidance.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET avoidance = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "level") == 0) {
        c->Message(Chat::Yellow,"NPCID %u is now level %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET level = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "maxlevel") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has a maximum level of %i.",  npcTypeID, atoi(sep->argplus[2]));
		std::string query = StringFormat("UPDATE npc_types SET maxlevel = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "qglobal") == 0) {
        c->Message(Chat::Yellow,"Quest globals have been %s for NPCID %u",  atoi(sep->arg[2]) == 0 ? "disabled" : "enabled",  npcTypeID);
		std::string query = StringFormat("UPDATE npc_types SET qglobal = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "npcaggro") == 0) {
        c->Message(Chat::Yellow,"NPCID %u will now %s other NPCs with negative faction npc_value",  npcTypeID, atoi(sep->arg[2]) == 0? "not aggro": "aggro");
		std::string query = StringFormat("UPDATE npc_types SET npc_aggro = %i WHERE id = %i",  atoi(sep->argplus[2]) == 0? 0: 1, npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "spawn_limit") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has a spawn limit of %i",  npcTypeID, atoi(sep->arg[2]));
		std::string query = StringFormat("UPDATE npc_types SET spawn_limit = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "Attackspeed") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has attack_speed set to %f",  npcTypeID, atof(sep->arg[2]));
		std::string query = StringFormat("UPDATE npc_types SET attack_speed = %f WHERE id = %i",  atof(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "Attackdelay") == 0) {
		c->Message(Chat::Yellow,"NPCID %u now has attack_delay set to %i", npcTypeID,atoi(sep->arg[2]));
		std::string query = StringFormat("UPDATE npc_types SET attack_delay = %i WHERE id = %i", atoi(sep->argplus[2]),npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "Attackcount") == 0) {
		c->Message(Chat::Yellow,"NPCID %u now has attack_count set to %i", npcTypeID,atoi(sep->arg[2]));
		std::string query = StringFormat("UPDATE npc_types SET attack_count = %i WHERE id = %i", atoi(sep->argplus[2]),npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "findable") == 0) {
        c->Message(Chat::Yellow,"NPCID %u is now %s",  npcTypeID, atoi(sep->arg[2]) == 0? "not findable": "findable");
		std::string query = StringFormat("UPDATE npc_types SET findable = %i WHERE id = %i",  atoi(sep->argplus[2]) == 0? 0: 1, npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "trackable") == 0) {
        c->Message(Chat::Yellow,"NPCID %u is now %s",  npcTypeID, atoi(sep->arg[2]) == 0? "not trackable": "trackable");
		std::string query = StringFormat("UPDATE npc_types SET trackable = %i WHERE id = %i",  atoi(sep->argplus[2]) == 0? 0: 1, npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "weapon") == 0) {
        c->Message(Chat::Yellow,"NPCID %u will have item graphic %i set to his primary and item graphic %i set to his secondary on repop.",   npcTypeID, atoi(sep->arg[2]), atoi(sep->arg[3]));
		std::string query = StringFormat("UPDATE npc_types SET d_melee_texture1 = %i, d_melee_texture2 = %i WHERE id = %i",  atoi(sep->arg[2]), atoi(sep->arg[3]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "featuresave") == 0) {
        c->Message(Chat::Yellow,"NPCID %u saved with all current facial feature settings",  npcTypeID);
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
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "color") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has %i red, %i green, and %i blue tinting on their armor.",  npcTypeID, atoi(sep->arg[2]), atoi(sep->arg[3]), atoi(sep->arg[4]));
		std::string query = StringFormat("UPDATE npc_types SET armortint_red = %i, armortint_green = %i, armortint_blue = %i WHERE id = %i",  atoi(sep->arg[2]), atoi(sep->arg[3]), atoi(sep->arg[4]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "armortint_id") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has field 'armortint_id' set to %s",  npcTypeID, sep->arg[2]);
		std::string query = StringFormat("UPDATE npc_types SET armortint_id = '%s' WHERE id = %i",  sep->argplus[2], npcTypeID);
		content_db.QueryDatabase(query);
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
			c->Message(Chat::White, "You must specifiy an animation stand, sit, crouch, dead, loot (0-4)");
			c->Message(Chat::White, "Example: #npcedit setanimation sit");
			c->Message(Chat::White, "Example: #npcedit setanimation 0");
			return;
		}

		c->Message(Chat::Yellow,"NPCID %u now has the animation set to %i on spawn with spawngroup %i",  npcTypeID, animation,
				   c->GetTarget()->CastToNPC()->GetSpawnGroupId() );
		std::string query = StringFormat("UPDATE spawn2 SET animation = %i " "WHERE spawngroupID = %i",  animation,
										 c->GetTarget()->CastToNPC()->GetSpawnGroupId());
		content_db.QueryDatabase(query);

		c->GetTarget()->SetAppearance(EmuAppearance(animation));
		return;
	}

	if (strcasecmp(sep->arg[1], "scalerate") == 0) {
        c->Message(Chat::Yellow,"NPCID %u now has a scaling rate of %i.",  npcTypeID, atoi(sep->arg[2]));
		std::string query = StringFormat("UPDATE npc_types SET scalerate = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "healscale") == 0) {
        c->Message(Chat::Yellow, "NPCID %u now has a heal scaling rate of %i.",  npcTypeID, atoi(sep->arg[2]));
		std::string query = StringFormat("UPDATE npc_types SET healscale = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "spellscale") == 0) {
        c->Message(Chat::Yellow, "NPCID %u now has a spell scaling rate of %i.",  npcTypeID, atoi(sep->arg[2]));
		std::string query = StringFormat("UPDATE npc_types SET spellscale = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "no_target") == 0) {
        c->Message(Chat::Yellow, "NPCID %u is now %s.",  npcTypeID, atoi(sep->arg[2]) == 0? "targetable": "untargetable");
		std::string query = StringFormat("UPDATE npc_types SET no_target_hotkey = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "version") == 0) {
        c->Message(Chat::Yellow, "NPCID %u is now version %i.",  npcTypeID, atoi(sep->arg[2]));
		std::string query = StringFormat("UPDATE npc_types SET version = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "slow_mitigation") == 0) {
        c->Message(Chat::Yellow, "NPCID %u's slow mitigation limit is now %i.",  npcTypeID, atoi(sep->arg[2]));
		std::string query = StringFormat("UPDATE npc_types SET slow_mitigation = %i WHERE id = %i",  atoi(sep->argplus[2]), npcTypeID);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "raidtarget") == 0) {
		if (sep->arg[2][0] && sep->IsNumber(sep->arg[2]) && atoi(sep->arg[2]) >= 0) {
			c->Message(Chat::Yellow, "NPCID %u is %s as a raid target.", npcTypeID, atoi(sep->arg[2]) == 0 ? "no longer designated" : "now designated");
			std::string query = StringFormat("UPDATE npc_types SET raid_target = %i WHERE id = %i", atoi(sep->arg[2]), npcTypeID);
			content_db.QueryDatabase(query);
			return;
		}
	}

	if (strcasecmp(sep->arg[1], "rarespawn") == 0) {
		if (sep->arg[2][0] && sep->IsNumber(sep->arg[2]) && atoi(sep->arg[2]) >= 0) {
			c->Message(Chat::Yellow, "NPCID %u is %s as a rare spawn.", npcTypeID, atoi(sep->arg[2]) == 0 ? "no longer designated" : "now designated");
			std::string query = StringFormat("UPDATE npc_types SET rare_spawn = %i WHERE id = %i", atoi(sep->arg[2]), npcTypeID);
			content_db.QueryDatabase(query);
			return;
		}
	}

	if (strcasecmp(sep->arg[1], "respawntime") == 0) {
		if (sep->arg[2][0] && sep->IsNumber(sep->arg[2]) && atoi(sep->arg[2]) > 0) {
			c->Message(Chat::Yellow, "NPCID %u (spawngroup %i) respawn time set to %i.", npcTypeID, c->GetTarget()->CastToNPC()->GetSpawnGroupId(), atoi(sep->arg[2]));
			std::string query = StringFormat("UPDATE spawn2 SET respawntime = %i WHERE spawngroupID = %i AND version = %i", atoi(sep->arg[2]), c->GetTarget()->CastToNPC()->GetSpawnGroupId(), zone->GetInstanceVersion());
			content_db.QueryDatabase(query);
			return;
		}
	}

	if((sep->arg[1][0] == 0 || strcasecmp(sep->arg[1],"*")==0) || ((c->GetTarget()==0) || (c->GetTarget()->IsClient())))
		c->Message(Chat::White, "Type #npcedit help for more info");

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
		c->Message(Chat::White, "Opcodes for all patches have been reloaded");
	}
}

void command_qglobal(Client *c, const Seperator *sep) {
	//In-game switch for qglobal column
	if(sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Syntax: #qglobal [on/off/view]. Requires NPC target.");
		return;
	}

	Mob *target = c->GetTarget();

	if(!target || !target->IsNPC()) {
		c->Message(Chat::Red, "NPC Target Required!");
		return;
	}

	if(!strcasecmp(sep->arg[1], "on")) {
        std::string query = StringFormat("UPDATE npc_types SET qglobal = 1 WHERE id = '%i'",
                                        target->GetNPCTypeID());
        auto results = content_db.QueryDatabase(query);
		if(!results.Success()) {
			c->Message(Chat::Yellow, "Could not update database.");
			return;
		}

        c->Message(Chat::Yellow, "Success! Changes take effect on zone reboot.");
		return;
	}

	if(!strcasecmp(sep->arg[1], "off")) {
        std::string query = StringFormat("UPDATE npc_types SET qglobal = 0 WHERE id = '%i'",
                                        target->GetNPCTypeID());
        auto results = content_db.QueryDatabase(query);
		if(!results.Success()) {
			c->Message(Chat::Yellow, "Could not update database.");
			return;
		}

        c->Message(Chat::Yellow, "Success! Changes take effect on zone reboot.");
		return;
	}

	if(!strcasecmp(sep->arg[1], "view")) {
		const NPCType *type = content_db.LoadNPCTypesData(target->GetNPCTypeID());
		if(!type)
			c->Message(Chat::Yellow, "Invalid NPC type.");
		else if(type->qglobal)
			c->Message(Chat::Yellow, "This NPC has quest globals active.");
		else
			c->Message(Chat::Yellow, "This NPC has quest globals disabled.");
		return;
	}

    c->Message(Chat::Yellow, "Invalid action specified.");
}

void command_path(Client *c, const Seperator *sep)
{
	if (zone->pathing) {
		zone->pathing->DebugCommand(c, sep);
	}
}

void Client::Undye() {
	for (int cur_slot = EQ::textures::textureBegin; cur_slot <= EQ::textures::LastTexture; cur_slot++) {
		uint8 slot2=SlotConvert(cur_slot);
		EQ::ItemInstance* inst = m_inv.GetItem(slot2);

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
		c->Message(Chat::White, "ERROR: Client target required");
	}
}

void command_ucs(Client *c, const Seperator *sep)
{
	if (!c)
		return;

	LogInfo("Character [{}] attempting ucs reconnect while ucs server is [{}] available",
		c->GetName(), (zone->IsUCSServerAvailable() ? "" : "un"));

	if (zone->IsUCSServerAvailable()) {
		EQApplicationPacket* outapp = nullptr;
		std::string buffer;

		std::string MailKey = database.GetMailKey(c->CharacterID(), true);
		EQ::versions::UCSVersion ConnectionType = EQ::versions::ucsUnknown;

		// chat server packet
		switch (c->ClientVersion()) {
		case EQ::versions::ClientVersion::Titanium:
			ConnectionType = EQ::versions::ucsTitaniumChat;
			break;
		case EQ::versions::ClientVersion::SoF:
			ConnectionType = EQ::versions::ucsSoFCombined;
			break;
		case EQ::versions::ClientVersion::SoD:
			ConnectionType = EQ::versions::ucsSoDCombined;
			break;
		case EQ::versions::ClientVersion::UF:
			ConnectionType = EQ::versions::ucsUFCombined;
			break;
		case EQ::versions::ClientVersion::RoF:
			ConnectionType = EQ::versions::ucsRoFCombined;
			break;
		case EQ::versions::ClientVersion::RoF2:
			ConnectionType = EQ::versions::ucsRoF2Combined;
			break;
		default:
			ConnectionType = EQ::versions::ucsUnknown;
			break;
		}

		buffer = StringFormat("%s,%i,%s.%s,%c%s",
			Config->ChatHost.c_str(),
			Config->ChatPort,
			Config->ShortName.c_str(),
			c->GetName(),
			ConnectionType,
			MailKey.c_str()
		);

		outapp = new EQApplicationPacket(OP_SetChatServer, (buffer.length() + 1));
		memcpy(outapp->pBuffer, buffer.c_str(), buffer.length());
		outapp->pBuffer[buffer.length()] = '\0';

		c->QueuePacket(outapp);
		safe_delete(outapp);

		// mail server packet
		switch (c->ClientVersion()) {
		case EQ::versions::ClientVersion::Titanium:
			ConnectionType = EQ::versions::ucsTitaniumMail;
			break;
		default:
			// retain value from previous switch
			break;
		}

		buffer = StringFormat("%s,%i,%s.%s,%c%s",
			Config->MailHost.c_str(),
			Config->MailPort,
			Config->ShortName.c_str(),
			c->GetName(),
			ConnectionType,
			MailKey.c_str()
		);

		outapp = new EQApplicationPacket(OP_SetChatServer2, (buffer.length() + 1));
		memcpy(outapp->pBuffer, buffer.c_str(), buffer.length());
		outapp->pBuffer[buffer.length()] = '\0';

		c->QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void command_undyeme(Client *c, const Seperator *sep)
{
	if(c) {
		c->Undye();
		c->Message(Chat::Red, "Dye removed from all slots. Please zone for the process to complete.");
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
		c->Message(Chat::White, "This client is not in a group");
		return;
	}

	c->Message(Chat::White, "Player: %s is in Group #%lu: with %i members",  t->GetName(), (unsigned long)g->GetID(), g->GroupCount());

	uint32 r;
	for(r = 0; r < MAX_GROUP_MEMBERS; r++) {
		if(g->members[r] == nullptr) {
			if(g->membername[r][0] == '\0')
				continue;
			c->Message(Chat::White, "...Zoned Member: %s, Roles: %s %s %s",  g->membername[r],
				(g->MemberRoles[r] & RoleAssist) ? "Assist" : "",
				(g->MemberRoles[r] & RoleTank) ? "Tank" : "",
				(g->MemberRoles[r] & RolePuller) ? "Puller" : "");
		} else {
			c->Message(Chat::White, "...In-Zone Member: %s (0x%x) Roles: %s %s %s",  g->membername[r], g->members[r],
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
		c->Message(Chat::White, "Error: you must have an NPC target.");
		return;
	}
	float d = atof(sep->arg[1]);
	if(d == 0.0f) {
		c->Message(Chat::Red, "Error: distance argument required.");
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
		c->Message(Chat::White, "POS: (%.2f, %.2f, %.2f)",  who->GetX(), who->GetY(), who->GetZ());
		c->Message(Chat::White, "WP: %s (%d/%d)",  to_string(who->GetCurrentWayPoint()).c_str(), who->IsNPC()?who->CastToNPC()->GetMaxWp():-1);
		c->Message(Chat::White, "pause=%d RAspeed=%d",  who->GetCWPP(), who->GetRunAnimSpeed());
		//who->DumpMovement(c);
	} else {
		c->Message(Chat::White, "ERROR: target required");
	}
}

void command_bestz(Client *c, const Seperator *sep) {
	if (zone->zonemap == nullptr) {
		c->Message(Chat::White,"Map not loaded for this zone");
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
			c->Message(Chat::White, "Z is %.3f at (%.3f, %.3f).",  best_z, me.x, me.y);
		}
		else
		{
			c->Message(Chat::White, "Found no Z.");
		}
	}

	if(zone->watermap == nullptr) {
		c->Message(Chat::White,"Water Region Map not loaded for this zone");
	} else {
		WaterRegionType RegionType;
		float z;

		if(c->GetTarget()) {
			z=c->GetTarget()->GetZ();
			auto position = glm::vec3(c->GetTarget()->GetX(), c->GetTarget()->GetY(), z);
			RegionType = zone->watermap->ReturnRegionType(position);
			c->Message(Chat::White,"InWater returns %d",  zone->watermap->InWater(position));
			c->Message(Chat::White,"InLava returns %d",  zone->watermap->InLava(position));

		}
		else {
			z=c->GetZ();
			auto position = glm::vec3(c->GetX(), c->GetY(), z);
			RegionType = zone->watermap->ReturnRegionType(position);
			c->Message(Chat::White,"InWater returns %d",  zone->watermap->InWater(position));
			c->Message(Chat::White,"InLava returns %d",  zone->watermap->InLava(position));

		}

		switch(RegionType) {
			case RegionTypeNormal:	{ c->Message(Chat::White,"There is nothing special about the region you are in!"); break; }
			case RegionTypeWater:	{ c->Message(Chat::White,"You/your target are in Water."); break; }
			case RegionTypeLava:	{ c->Message(Chat::White,"You/your target are in Lava."); break; }
			case RegionTypeVWater:	{ c->Message(Chat::White,"You/your target are in VWater (Icy Water?)."); break; }
			case RegionTypePVP:	{ c->Message(Chat::White, "You/your target are in a pvp enabled area."); break; }
			case RegionTypeSlime:	{ c->Message(Chat::White, "You/your target are in slime."); break; }
			case RegionTypeIce:	{ c->Message(Chat::White, "You/your target are in ice."); break; }
			default: c->Message(Chat::White,"You/your target are in an unknown region type.");
		}
	}


}


void command_reloadstatic(Client *c, const Seperator *sep) {
	c->Message(Chat::White, "Reloading zone static data...");
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
		c->Message(Chat::White, "Syntax: #flagedit [lockzone|unlockzone|listzones|give|take].");
		c->Message(Chat::White, "...lockzone [zone id/short] [flag name] - Set the specified flag name on the zone, locking the zone");
		c->Message(Chat::White, "...unlockzone [zone id/short] - Removes the flag requirement from the specified zone");
		c->Message(Chat::White, "...listzones - List all zones which require a flag, and their flag's name");
		c->Message(Chat::White, "...give [zone id/short] - Give your target the zone flag for the specified zone.");
		c->Message(Chat::White, "...take [zone id/short] - Take the zone flag for the specified zone away from your target");
		c->Message(Chat::White, "...Note: use #flags to view flags on a person");
		return;
	}

	if(!strcasecmp(sep->arg[1], "lockzone")) {
		uint32 zoneid = 0;
		if(sep->arg[2][0] != '\0') {
			zoneid = atoi(sep->arg[2]);
			if(zoneid < 1) {
				zoneid = ZoneID(sep->arg[2]);
			}
		}
		if(zoneid < 1) {
			c->Message(Chat::Red, "zone required. see help.");
			return;
		}

		char flag_name[128];
		if(sep->argplus[3][0] == '\0') {
			c->Message(Chat::Red, "flag name required. see help.");
			return;
		}
		database.DoEscapeString(flag_name, sep->argplus[3], 64);
		flag_name[127] = '\0';

        std::string query = StringFormat("UPDATE zone SET flag_needed = '%s' "
                                        "WHERE zoneidnumber = %d AND version = %d",
                                        flag_name, zoneid, zone->GetInstanceVersion());
        auto results = content_db.QueryDatabase(query);
		if(!results.Success()) {
			c->Message(Chat::Red, "Error updating zone: %s",  results.ErrorMessage().c_str());
			return;
		}

        c->Message(Chat::Yellow, "Success! Zone %s now requires a flag, named %s",  ZoneName(zoneid), flag_name);
        return;
	}

	if(!strcasecmp(sep->arg[1], "unlockzone")) {
		uint32 zoneid = 0;
		if(sep->arg[2][0] != '\0') {
			zoneid = atoi(sep->arg[2]);
			if(zoneid < 1) {
				zoneid = ZoneID(sep->arg[2]);
			}
		}

		if(zoneid < 1) {
			c->Message(Chat::Red, "zone required. see help.");
			return;
		}

        std::string query = StringFormat("UPDATE zone SET flag_needed = '' "
                                        "WHERE zoneidnumber = %d AND version = %d",
                                        zoneid, zone->GetInstanceVersion());
        auto results = content_db.QueryDatabase(query);
		if(!results.Success()) {
			c->Message(Chat::Yellow, "Error updating zone: %s",  results.ErrorMessage().c_str());
			return;
		}

        c->Message(Chat::Yellow, "Success! Zone %s no longer requires a flag.",  ZoneName(zoneid));
        return;
	}

	if(!strcasecmp(sep->arg[1], "listzones")) {
        std::string query = "SELECT zoneidnumber, short_name, long_name, version, flag_needed "
                            "FROM zone WHERE flag_needed != ''";
        auto results = content_db.QueryDatabase(query);
		if (!results.Success()) {
            return;
        }

        c->Message(Chat::White, "Zones which require flags:");
        for (auto row = results.begin(); row != results.end(); ++row)
            c->Message(Chat::White, "Zone %s (%s,%s) version %s requires key %s",  row[2], row[0], row[1], row[3], row[4]);

        return;
	}

	if(!strcasecmp(sep->arg[1], "give")) {
		uint32 zoneid = 0;
		if(sep->arg[2][0] != '\0') {
			zoneid = atoi(sep->arg[2]);
			if(zoneid < 1) {
				zoneid = ZoneID(sep->arg[2]);
			}
		}
		if(zoneid < 1) {
			c->Message(Chat::Red, "zone required. see help.");
			return;
		}

		Mob *t = c->GetTarget();
		if(t == nullptr || !t->IsClient()) {
			c->Message(Chat::Red, "client target required");
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
				zoneid = ZoneID(sep->arg[2]);
			}
		}
		if(zoneid < 1) {
			c->Message(Chat::Red, "zone required. see help.");
			return;
		}

		Mob *t = c->GetTarget();
		if(t == nullptr || !t->IsClient()) {
			c->Message(Chat::Red, "client target required");
			return;
		}

		t->CastToClient()->ClearZoneFlag(zoneid);
		return;
	}

    c->Message(Chat::Yellow, "Invalid action specified. use '#flagedit help' for help");
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
		c->Message(Chat::White,"It is recorded you have agreed to the rules.");
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
		c->Message(Chat::White,"Guild name must be more than 4 characters and less than 16.");
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
		c->Message(Chat::White,"Could not find reference id.");
}

void command_hatelist(Client *c, const Seperator *sep) {
	Mob *target = c->GetTarget();
	if(target == nullptr) {
		c->Message(Chat::White, "Error: you must have a target.");
		return;
	}

	c->Message(Chat::White, "Display hate list for %s..",  target->GetName());
	target->PrintHateListToClient(c);
}


void command_rules(Client *c, const Seperator *sep) {
	//super-command for managing rules settings
	if(sep->arg[1][0] == '\0' || !strcasecmp(sep->arg[1], "help")) {
		c->Message(Chat::White, "Syntax: #rules [subcommand].");
		c->Message(Chat::White, "-- Rule Set Manipulation --");
		c->Message(Chat::White, "...listsets - List avaliable rule sets");
		c->Message(Chat::White, "...current - gives the name of the ruleset currently running in this zone");
		c->Message(Chat::White, "...reload - Reload the selected ruleset in this zone");
		c->Message(Chat::White, "...switch (ruleset name) - Change the selected ruleset and load it");
		c->Message(Chat::White, "...load (ruleset name) - Load a ruleset in just this zone without changing the selected set");
//too lazy to write this right now:
//		c->Message(Chat::White, "...wload (ruleset name) - Load a ruleset in all zones without changing the selected set");
		c->Message(Chat::White, "...store [ruleset name] - Store the running ruleset as the specified name");
		c->Message(Chat::White, "---------------------");
		c->Message(Chat::White, "-- Running Rule Manipulation --");
		c->Message(Chat::White, "...reset - Reset all rules to their default values");
		c->Message(Chat::White, "...get [rule] - Get the specified rule's local value");
		c->Message(Chat::White, "...set (rule) (value) - Set the specified rule to the specified value locally only");
		c->Message(Chat::White, "...setdb (rule) (value) - Set the specified rule to the specified value locally and in the DB");
		c->Message(Chat::White, "...list [catname] - List all rules in the specified category (or all categiries if omitted)");
		c->Message(Chat::White, "...values [catname] - List the value of all rules in the specified category");
		return;
	}

	if(!strcasecmp(sep->arg[1], "current")) {
		c->Message(Chat::White, "Currently running ruleset '%s' (%d)",  RuleManager::Instance()->GetActiveRuleset(),
			RuleManager::Instance()->GetActiveRulesetID());
	} else if(!strcasecmp(sep->arg[1], "listsets")) {
		std::map<int, std::string> sets;
		if(!RuleManager::Instance()->ListRulesets(&database, sets)) {
			c->Message(Chat::Red, "Failed to list rule sets!");
			return;
		}

		c->Message(Chat::White, "Avaliable rule sets:");
		std::map<int, std::string>::iterator cur, end;
		cur = sets.begin();
		end = sets.end();
		for(; cur != end; ++cur) {
			c->Message(Chat::White, "(%d) %s",  cur->first, cur->second.c_str());
		}
	} else if(!strcasecmp(sep->arg[1], "reload")) {
		RuleManager::Instance()->LoadRules(&database, RuleManager::Instance()->GetActiveRuleset(), true);
		c->Message(Chat::White, "The active ruleset (%s (%d)) has been reloaded",  RuleManager::Instance()->GetActiveRuleset(),
			RuleManager::Instance()->GetActiveRulesetID());
	} else if(!strcasecmp(sep->arg[1], "switch")) {
		//make sure this is a valid rule set..
		int rsid = RuleManager::Instance()->GetRulesetID(&database, sep->arg[2]);
		if(rsid < 0) {
			c->Message(Chat::Red, "Unknown rule set '%s'",  sep->arg[2]);
			return;
		}
		if(!database.SetVariable("RuleSet",  sep->arg[2])) {
			c->Message(Chat::Red, "Failed to update variables table to change selected rule set");
			return;
		}

		//TODO: we likely want to reload this ruleset everywhere...
		RuleManager::Instance()->LoadRules(&database, sep->arg[2], true);

		c->Message(Chat::White, "The selected ruleset has been changed to (%s (%d)) and reloaded locally",  sep->arg[2], rsid);
	} else if(!strcasecmp(sep->arg[1], "load")) {
		//make sure this is a valid rule set..
		int rsid = RuleManager::Instance()->GetRulesetID(&database, sep->arg[2]);
		if(rsid < 0) {
			c->Message(Chat::Red, "Unknown rule set '%s'",  sep->arg[2]);
			return;
		}
		RuleManager::Instance()->LoadRules(&database, sep->arg[2], true);
		c->Message(Chat::White, "Loaded ruleset '%s' (%d) locally",  sep->arg[2], rsid);
	} else if(!strcasecmp(sep->arg[1], "store")) {
		if(sep->argnum == 1) {
			//store current rule set.
			RuleManager::Instance()->SaveRules(&database);
			c->Message(Chat::White, "Rules saved");
		} else if(sep->argnum == 2) {
			RuleManager::Instance()->SaveRules(&database, sep->arg[2]);
			int prersid = RuleManager::Instance()->GetActiveRulesetID();
			int rsid = RuleManager::Instance()->GetRulesetID(&database, sep->arg[2]);
			if(rsid < 0) {
				c->Message(Chat::Red, "Unable to query ruleset ID after store, it most likely failed.");
			} else {
				c->Message(Chat::White, "Stored rules as ruleset '%s' (%d)",  sep->arg[2], rsid);
				if(prersid != rsid) {
					c->Message(Chat::White, "Rule set %s (%d) is now active in this zone",  sep->arg[2], rsid);
				}
			}
		} else {
			c->Message(Chat::Red, "Invalid argument count, see help.");
			return;
		}
	} else if(!strcasecmp(sep->arg[1], "reset")) {
		RuleManager::Instance()->ResetRules(true);
		c->Message(Chat::White, "The running ruleset has been set to defaults");

	} else if(!strcasecmp(sep->arg[1], "get")) {
		if(sep->argnum != 2) {
			c->Message(Chat::Red, "Invalid argument count, see help.");
			return;
		}
		std::string value;
		if(!RuleManager::Instance()->GetRule(sep->arg[2], value))
			c->Message(Chat::Red, "Unable to find rule %s",  sep->arg[2]);
		else
			c->Message(Chat::White, "%s - %s",  sep->arg[2], value.c_str());

	} else if(!strcasecmp(sep->arg[1], "set")) {
		if(sep->argnum != 3) {
			c->Message(Chat::Red, "Invalid argument count, see help.");
			return;
		}
		if(!RuleManager::Instance()->SetRule(sep->arg[2], sep->arg[3], nullptr, false, true)) {
			c->Message(Chat::Red, "Failed to modify rule");
		} else {
			c->Message(Chat::White, "Rule modified locally.");
		}
	} else if(!strcasecmp(sep->arg[1], "setdb")) {
		if(sep->argnum != 3) {
			c->Message(Chat::Red, "Invalid argument count, see help.");
			return;
		}
		if(!RuleManager::Instance()->SetRule(sep->arg[2], sep->arg[3], &database, true, true)) {
			c->Message(Chat::Red, "Failed to modify rule");
		} else {
			c->Message(Chat::White, "Rule modified locally and in the database.");
		}
	} else if(!strcasecmp(sep->arg[1], "list")) {
		if(sep->argnum == 1) {
			std::vector<const char *> rule_list;
			if(!RuleManager::Instance()->ListCategories(rule_list)) {
				c->Message(Chat::Red, "Failed to list categories!");
				return;
			}
			c->Message(Chat::White, "Rule Categories:");
			std::vector<const char *>::iterator cur, end;
			cur = rule_list.begin();
			end = rule_list.end();
			for(; cur != end; ++cur) {
				c->Message(Chat::White, " %s",  *cur);
			}
		} else if(sep->argnum == 2) {
			const char *catfilt = nullptr;
			if(std::string("all") != sep->arg[2])
				catfilt = sep->arg[2];
			std::vector<const char *> rule_list;
			if(!RuleManager::Instance()->ListRules(catfilt, rule_list)) {
				c->Message(Chat::Red, "Failed to list rules!");
				return;
			}
			c->Message(Chat::White, "Rules in category %s:",  sep->arg[2]);
			std::vector<const char *>::iterator cur, end;
			cur = rule_list.begin();
			end = rule_list.end();
			for(; cur != end; ++cur) {
				c->Message(Chat::White, " %s",  *cur);
			}
		} else {
			c->Message(Chat::Red, "Invalid argument count, see help.");
		}
	} else if(!strcasecmp(sep->arg[1], "values")) {
		if(sep->argnum != 2) {
			c->Message(Chat::Red, "Invalid argument count, see help.");
			return;
		} else {
			const char *catfilt = nullptr;
			if(std::string("all") != sep->arg[2])
				catfilt = sep->arg[2];
			std::vector<const char *> rule_list;
			if(!RuleManager::Instance()->ListRules(catfilt, rule_list)) {
				c->Message(Chat::Red, "Failed to list rules!");
				return;
			}
			c->Message(Chat::White, "Rules & values in category %s:",  sep->arg[2]);
			std::vector<const char *>::iterator cur, end;
			cur = rule_list.begin();
			end = rule_list.end();
			for(std::string tmp_value; cur != end; ++cur) {
				if (RuleManager::Instance()->GetRule(*cur, tmp_value))
					c->Message(Chat::White, " %s - %s",  *cur, tmp_value.c_str());
			}
		}

	} else {
		c->Message(Chat::Yellow, "Invalid action specified. use '#rules help' for help");
	}
}


void command_task(Client *c, const Seperator *sep) {
	//super-command for managing tasks
	if(sep->arg[1][0] == '\0' || !strcasecmp(sep->arg[1], "help")) {
		c->Message(Chat::White, "Syntax: #task [subcommand]");
		c->Message(Chat::White, "------------------------------------------------");
		c->Message(Chat::White, "# Task System Commands");
		c->Message(Chat::White, "------------------------------------------------");
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] List active tasks for a client",
				EQ::SayLinkEngine::GenerateQuestSaylink("#task show", false, "show")
			).c_str()
		);
		c->Message(Chat::White, "--- update <task_id> <activity_id> [count] | Updates task");
		c->Message(Chat::White, "--- assign <task_id> | Assigns task to client");
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Reload all Task information from the database",
				EQ::SayLinkEngine::GenerateQuestSaylink("#task reloadall", false, "reloadall")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] <task_id> Reload Task and Activity information for a single task",
				EQ::SayLinkEngine::GenerateQuestSaylink("#task reload task", false, "reload task")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Reload goal/reward list information",
				EQ::SayLinkEngine::GenerateQuestSaylink("#task reload lists", false, "reload lists")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Reload proximity information",
				EQ::SayLinkEngine::GenerateQuestSaylink("#task reload prox", false, "reload prox")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Reload task set information",
				EQ::SayLinkEngine::GenerateQuestSaylink("#task reload sets", false, "reload sets")
			).c_str()
		);
		return;
	}

	Client *client_target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		client_target = c->GetTarget()->CastToClient();
	}

	if (!strcasecmp(sep->arg[1], "show")) {
		c->ShowClientTasks(client_target);
		return;
	}

	if (!strcasecmp(sep->arg[1], "update")) {
		if (sep->argnum >= 3) {
			int task_id     = atoi(sep->arg[2]);
			int activity_id = atoi(sep->arg[3]);
			int count       = 1;

			if (sep->argnum >= 4) {
				count = atoi(sep->arg[4]);
				if (count <= 0) {
					count = 1;
				}
			}
			c->Message(
				Chat::Yellow,
				"Updating Task [%i] Activity [%i] Count [%i] for client [%s]",
				task_id,
				activity_id,
				count,
				client_target->GetCleanName()
			);
			client_target->UpdateTaskActivity(task_id, activity_id, count);
			c->ShowClientTasks(client_target);
		}
		return;
	}

	if (!strcasecmp(sep->arg[1], "assign")) {
		int task_id = atoi(sep->arg[2]);
		if ((task_id > 0) && (task_id < MAXTASKS)) {
			client_target->AssignTask(task_id, 0, false);
			c->Message(Chat::Yellow, "Assigned task [%i] to [%s]", task_id, client_target->GetCleanName());
		}
		return;
	}

	if (!strcasecmp(sep->arg[1], "reloadall")) {
		c->Message(Chat::Yellow, "Sending reloadtasks to world");
		worldserver.SendReloadTasks(RELOADTASKS);
		c->Message(Chat::Yellow, "Back again");
		return;
	}

	if (!strcasecmp(sep->arg[1], "reload")) {
		if (sep->arg[2][0] != '\0') {
			if (!strcasecmp(sep->arg[2], "lists")) {
				c->Message(Chat::Yellow, "Sending reload lists to world");
				worldserver.SendReloadTasks(RELOADTASKGOALLISTS);
				c->Message(Chat::Yellow, "Reloaded");
				return;
			}
			if (!strcasecmp(sep->arg[2], "prox")) {
				c->Message(Chat::Yellow, "Sending reload proximities to world");
				worldserver.SendReloadTasks(RELOADTASKPROXIMITIES);
				c->Message(Chat::Yellow, "Reloaded");
				return;
			}
			if (!strcasecmp(sep->arg[2], "sets")) {
				c->Message(Chat::Yellow, "Sending reload task sets to world");
				worldserver.SendReloadTasks(RELOADTASKSETS);
				c->Message(Chat::Yellow, "Reloaded");
				return;
			}
			if (!strcasecmp(sep->arg[2], "task") && (sep->arg[3][0] != '\0')) {
				int task_id = atoi(sep->arg[3]);
				if ((task_id > 0) && (task_id < MAXTASKS)) {
					c->Message(Chat::Yellow, "Sending reload task %i to world", task_id);
					worldserver.SendReloadTasks(RELOADTASKS, task_id);
					c->Message(Chat::Yellow, "Reloaded");
					return;
				}
			}
		}

	}
	c->Message(Chat::White, "Unable to interpret command. Type #task help");

}
void command_reloadtitles(Client *c, const Seperator *sep)
{
	auto pack = new ServerPacket(ServerOP_ReloadTitles, 0);
	worldserver.SendPacket(pack);
	safe_delete(pack);
	c->Message(Chat::Yellow, "Player Titles Reloaded.");

}

void command_traindisc(Client *c, const Seperator *sep)
{
	Client *t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM())
		t = c->GetTarget()->CastToClient();

	if (sep->argnum < 1 || !sep->IsNumber(1)) {
		c->Message(Chat::White, "FORMAT: #traindisc <max level> <min level>");
		return;
	}

	uint8 max_level = (uint8)atol(sep->arg[1]);
	if (!c->GetGM() && max_level >(uint8)RuleI(Character, MaxLevel))
		max_level = (uint8)RuleI(Character, MaxLevel); // default to Character:MaxLevel if we're not a GM & it's higher than the max level

	uint8 min_level = (sep->IsNumber(2) ? (uint8)atol(sep->arg[2]) : 1); // default to 1 if there isn't a 2nd argument
	if (!c->GetGM() && min_level > (uint8)RuleI(Character, MaxLevel))
		min_level = (uint8)RuleI(Character, MaxLevel); // default to Character:MaxLevel if we're not a GM & it's higher than the max level

	if(max_level < 1 || min_level < 1) {
		c->Message(Chat::White, "ERROR: Level must be greater than 1.");
		return;
	}
	if (min_level > max_level) {
		c->Message(Chat::White, "Error: Min Level must be less than or equal to Max Level.");
		return;
	}

	t->Message(Chat::White, "Training disciplines");
	if(t != c)
		c->Message(Chat::White, "Training disciplines for %s.",  t->GetName());
	LogInfo("Train disciplines request for [{}] from [{}], levels: [{}] -> [{}]",  t->GetName(), c->GetName(), min_level, max_level);

	int spell_id = 0;
	int count = 0;

	bool change = false;

	for( ; spell_id < SPDAT_RECORDS; ++spell_id) {
		if (spell_id < 0 || spell_id >= SPDAT_RECORDS) {
			c->Message(Chat::Red, "FATAL ERROR: Spell id out-of-range (id: %i, min: 0, max: %i)", spell_id, SPDAT_RECORDS);
			return;
		}

		while (true) {
			if (spells[spell_id].classes[WARRIOR] == 0) // check if spell exists
				break;
			if (spells[spell_id].classes[t->GetPP().class_ - 1] > max_level) // maximum level
				break;
			if (spells[spell_id].classes[t->GetPP().class_ - 1] < min_level) // minimum level
				break;
			if (spells[spell_id].skill == 52)
				break;

			uint16 spell_id_ = (uint16)spell_id;
			if ((spell_id_ != spell_id) || (spell_id != spell_id_)) {
				c->Message(Chat::Red, "FATAL ERROR: Type conversion data loss with spell_id (%i != %u)", spell_id, spell_id_);
				return;
			}

			if (!IsDiscipline(spell_id_))
				break;

			for (uint32 r = 0; r < MAX_PP_DISCIPLINES; ++r) {
				if (t->GetPP().disciplines.values[r] == spell_id_) {
					t->Message(Chat::Red, "You already know this discipline.");
					break; // continue the 1st loop
				}
				else if (t->GetPP().disciplines.values[r] == 0) {
					t->GetPP().disciplines.values[r] = spell_id_;
					database.SaveCharacterDisc(t->CharacterID(), r, spell_id_);
					change = true;
					t->Message(Chat::White, "You have learned a new discipline!");
					++count; // success counter
					break; // continue the 1st loop
				} // if we get to this point, there's already a discipline in this slot, so we continue onto the next slot
			}

			break;
		}
	}

	if (change)
		t->SendDisciplineUpdate();

	if (count > 0) {
		t->Message(Chat::White, "Successfully trained %u disciplines.",  count);
		if (t != c)
			c->Message(Chat::White, "Successfully trained %u disciplines for %s.",  count, t->GetName());
	} else {
		t->Message(Chat::White, "No disciplines trained.");
		if (t != c)
			c->Message(Chat::White, "No disciplines trained for %s.",  t->GetName());
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
		c->Message(Chat::White, "Usage: #setgraveyard [zonename]");
		return;
	}

	zoneid = ZoneID(sep->arg[1]);

	if(zoneid > 0) {
		graveyard_id = content_db.CreateGraveyardRecord(zoneid, t->GetPosition());

		if(graveyard_id > 0) {
			c->Message(Chat::White, "Successfuly added a new record for this graveyard!");
			if(content_db.AddGraveyardIDToZone(zoneid, graveyard_id) > 0) {
				c->Message(Chat::White, "Successfuly added this new graveyard for the zone %s.",  sep->arg[1]);
				// TODO: Set graveyard data to the running zone process.
				c->Message(Chat::White, "Done!");
			}
			else
				c->Message(Chat::White, "Unable to add this new graveyard to the zone %s.",  sep->arg[1]);
		}
		else {
			c->Message(Chat::White, "Unable to create a new graveyard record in the database.");
		}
	}
	else {
		c->Message(Chat::White, "Unable to retrieve a ZoneID for the zone: %s",  sep->arg[1]);
	}

	return;
}

void command_deletegraveyard(Client *c, const Seperator *sep)
{
	uint32 zoneid = 0;
	uint32 graveyard_id = 0;

	if(!sep->arg[1][0]) {
		c->Message(Chat::White, "Usage: #deletegraveyard [zonename]");
		return;
	}

	zoneid = ZoneID(sep->arg[1]);
	graveyard_id = content_db.GetZoneGraveyardID(zoneid, 0);

	if(zoneid > 0 && graveyard_id > 0) {
		if(content_db.DeleteGraveyard(zoneid, graveyard_id))
			c->Message(Chat::White, "Successfuly deleted graveyard %u for zone %s.",  graveyard_id, sep->arg[1]);
		else
			c->Message(Chat::White, "Unable to delete graveyard %u for zone %s.",  graveyard_id, sep->arg[1]);
	}
	else {
		if(zoneid <= 0)
			c->Message(Chat::White, "Unable to retrieve a ZoneID for the zone: %s",  sep->arg[1]);
		else if(graveyard_id <= 0)
			c->Message(Chat::White, "Unable to retrieve a valid GraveyardID for the zone: %s",  sep->arg[1]);
	}

	return;
}

void command_summonburiedplayercorpse(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM())
		t = c->GetTarget()->CastToClient();
	else {
		c->Message(Chat::White, "You must first select a target!");
		return;
	}

	Corpse* PlayerCorpse = database.SummonBuriedCharacterCorpses(t->CharacterID(), t->GetZoneID(), zone->GetInstanceID(), t->GetPosition());

	if(!PlayerCorpse)
		c->Message(Chat::White, "Your target doesn't have any buried corpses.");

	return;
}

void command_getplayerburiedcorpsecount(Client *c, const Seperator *sep)
{
	Client *t=c;

	if(c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM())
		t = c->GetTarget()->CastToClient();
	else {
		c->Message(Chat::White, "You must first select a target!");
		return;
	}

	uint32 CorpseCount = database.GetCharacterBuriedCorpseCount(t->CharacterID());

	if(CorpseCount > 0)
		c->Message(Chat::White, "Your target has a total of %u buried corpses.",  CorpseCount);
	else
		c->Message(Chat::White, "Your target doesn't have any buried corpses.");

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
            c->Message(Chat::White, "Target Required!");
            return;
        }

        content_db.NPCSpawnDB(6, zone->GetShortName(), zone->GetInstanceVersion(), c, target->CastToNPC());
        return;
    }

    if (strcasecmp(sep->arg[1], "makegroup") == 0) {
        if(!sep->arg[2]) {
            c->Message(Chat::White, "Format: #advnpdspawn makegroup <name> [spawn limit] [dist] [max x] [min x] [max y] [min y] [delay]");
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
        auto results = content_db.QueryDatabase(query);
        if (!results.Success()) {
            c->Message(Chat::White, "Invalid Arguments -- MySQL gave the following error:");
            c->Message(Chat::Red, results.ErrorMessage().c_str());
            return;
        }

        c->Message(Chat::White, "Group ID %i created successfully!",  results.LastInsertedID());
        return;
    }

	if (strcasecmp(sep->arg[1], "addgroupentry") == 0) {
        if(!atoi(sep->arg[2]) || !atoi(sep->arg[3]) || !atoi(sep->arg[4])) {
            c->Message(Chat::White, "Format: #advnpdspawn addgroupentry <spawnggroupID> <npcID> <chance>");
            return;
        }

        std::string query = StringFormat("INSERT INTO spawnentry (spawngroupID, npcID, chance) "
                                        "VALUES (%i, %i, %i)",
                                        atoi(sep->arg[2]), atoi(sep->arg[3]), atoi(sep->arg[4]));
        auto results = content_db.QueryDatabase(query);
        if (!results.Success()) {
            c->Message(Chat::White, "Invalid Arguments -- MySQL gave the following error:");
            c->Message(Chat::Red, results.ErrorMessage().c_str());
            return;
        }

        c->Message(Chat::White, "NPC %i added to group %i with %i chance!",  atoi(sep->arg[3]), atoi(sep->arg[2]), atoi(sep->arg[4]) );

        return;
    }

    if (strcasecmp(sep->arg[1], "editgroupbox") == 0) {
        if(!atof(sep->arg[2]) || !atof(sep->arg[3]) || !atof(sep->arg[4]) || !atof(sep->arg[5]) || !atof(sep->arg[6]) || !atof(sep->arg[7]) || !atof(sep->arg[8])) {
            c->Message(Chat::White, "Format: #advnpdspawn editgroupbox <spawngroupID> <dist> <max x> <min x> <max y> <min y> <delay>");
            return;
        }

        std::string query = StringFormat("UPDATE spawngroup SET dist = '%f', max_x = '%f', min_x = '%f', "
                                        "max_y = '%f', min_y = '%f', delay = '%i' WHERE id = '%i'",
                                        atof(sep->arg[3]), atof(sep->arg[4]), atof(sep->arg[5]),
                                        atof(sep->arg[6]), atof(sep->arg[7]), atoi(sep->arg[8]),
                                        atoi(sep->arg[2]));
        auto results = content_db.QueryDatabase(query);
        if (!results.Success()) {
            c->Message(Chat::White, "Invalid Arguments -- MySQL gave the following error:");
            c->Message(Chat::Red, results.ErrorMessage().c_str());
            return;
        }

        c->Message(Chat::White, "Group ID %i created successfully!",  results.LastInsertedID());

        return;
    }

	if (strcasecmp(sep->arg[1], "cleargroupbox") == 0) {
        if(!atoi(sep->arg[2])) {
            c->Message(Chat::White, "Format: #advnpdspawn cleargroupbox <spawngroupID>");
            return;
        }

        std::string query = StringFormat("UPDATE spawngroup "
                                        "SET dist = '0', max_x = '0', min_x = '0', "
                                        "max_y = '0', min_y = '0', delay = '0' "
                                        "WHERE id = '%i' ",  atoi(sep->arg[2]));
        auto results = content_db.QueryDatabase(query);
        if (!results.Success()) {
            c->Message(Chat::White, "Invalid Arguments -- MySQL gave the following error:");
            c->Message(Chat::Red, results.ErrorMessage().c_str());
            return;
        }

        c->Message(Chat::White, "Group ID %i created successfully!",  results.LastInsertedID());

        return;
    }

	if (strcasecmp(sep->arg[1], "addgroupspawn") == 0 && atoi(sep->arg[2])!=0) {
        content_db.NPCSpawnDB(5, zone->GetShortName(), zone->GetInstanceVersion(), c, 0, atoi(sep->arg[2]));
        c->Message(Chat::White, "Mob of group %i added successfully!",  atoi(sep->arg[2]));
        return;
    }

	if (strcasecmp(sep->arg[1], "removegroupspawn") == 0) {
        if (!target || !target->IsNPC()) {
            c->Message(Chat::White, "Error: Need an NPC target.");
            return;
        }

        Spawn2* s2 = target->CastToNPC()->respawn2;

        if(!s2) {
            c->Message(Chat::White, "removegroupspawn FAILED -- cannot determine which spawn entry in the database this mob came from.");
            return;
        }

        std::string query = StringFormat("DELETE FROM spawn2 WHERE id = '%i'",  s2->GetID());
        auto results = content_db.QueryDatabase(query);
        if(!results.Success()) {
            c->Message(Chat::Red, "Update failed! MySQL gave the following error:");
            c->Message(Chat::Red, results.ErrorMessage().c_str());
            return;
        }

        c->Message(Chat::White, "Spawnpoint Removed successfully.");
        target->Depop(false);

        return;
    }

	if (strcasecmp(sep->arg[1], "movespawn") == 0) {
        if (!target || !target->IsNPC()) {
            c->Message(Chat::White, "Error: Need an NPC target.");
            return;
        }

        Spawn2* s2 = target->CastToNPC()->respawn2;

        if(!s2) {
            c->Message(Chat::White, "movespawn FAILED -- cannot determine which spawn entry in the database this mob came from.");
            return;
        }

        std::string query = StringFormat("UPDATE spawn2 SET x = '%f', y = '%f', z = '%f', heading = '%f' "
                                        "WHERE id = '%i'",
                                        c->GetX(), c->GetY(), c->GetZ(), c->GetHeading(),s2->GetID());
        auto results = content_db.QueryDatabase(query);
        if (!results.Success()) {
            c->Message(Chat::Red, "Update failed! MySQL gave the following error:");
            c->Message(Chat::Red, results.ErrorMessage().c_str());
            return;
        }

        c->Message(Chat::White, "Updating coordinates successful.");
        target->GMMove(c->GetX(), c->GetY(), c->GetZ(), c->GetHeading());

        return;
    }

	if (strcasecmp(sep->arg[1], "editrespawn") == 0) {
        if (!target || !target->IsNPC()) {
            c->Message(Chat::White, "Error: Need an NPC target.");
            return;
        }

        Spawn2* s2 = target->CastToNPC()->respawn2;

        uint32 new_rs = 0;
        uint32 new_var = s2->GetVariance();
        if(!sep->IsNumber(2)) {
            c->Message(Chat::White, "editrespawn FAILED -- cannot set respawn to be 0");
            return;
        }

        new_rs = atoi(sep->arg[2]);

        if(sep->IsNumber(3))
            new_var = atoi(sep->arg[3]);

        if(!s2) {
            c->Message(Chat::White, "editrespawn FAILED -- cannot determine which spawn entry in the database this mob came from.");
            return;
        }

        std::string query = StringFormat("UPDATE spawn2 SET respawntime = %u, variance = %u "
                                        "WHERE id = '%i'",  new_rs, new_var, s2->GetID());
        auto results = content_db.QueryDatabase(query);
        if (!results.Success()) {
            c->Message(Chat::Red, "Update failed! MySQL gave the following error:");
            c->Message(Chat::Red, results.ErrorMessage().c_str());
            return;
        }

        c->Message(Chat::White, "Updating respawn timer successful.");
        s2->SetRespawnTimer(new_rs);
        s2->SetVariance(new_var);

        return;
    }

	if (strcasecmp(sep->arg[1], "setversion") == 0) {
        if (!target || !target->IsNPC()) {
            c->Message(Chat::White, "Error: Need an NPC target.");
            return;
        }

        if(!sep->IsNumber(2)) {
            c->Message(Chat::White, "setversion FAILED -- You must set a version number");
            return;
        }

        int16 version = atoi(sep->arg[2]);
        std::string query = StringFormat("UPDATE spawn2 SET version = %i "
                                        "WHERE spawngroupID = '%i'",
                                        version, c->GetTarget()->CastToNPC()->GetSpawnGroupId());
        auto results = content_db.QueryDatabase(query);
        if (!results.Success()) {
            c->Message(Chat::Red, "Update failed! MySQL gave the following error:");
            c->Message(Chat::Red, results.ErrorMessage().c_str());
            return;
        }

        c->Message(Chat::White, "Version change to %i was successful from SpawnGroupID %i",  version,
				   c->GetTarget()->CastToNPC()->GetSpawnGroupId());
        c->GetTarget()->Depop(false);

        return;
    }

	if (strcasecmp(sep->arg[1], "testload") == 0 && atoi(sep->arg[2])!=0) {
        content_db.LoadSpawnGroupsByID(atoi(sep->arg[2]),&zone->spawn_group_list);
        c->Message(Chat::White, "Group %i loaded successfully!",  atoi(sep->arg[2]));
        return;
    }

    c->Message(Chat::White, "Error: #advnpcspawn: Invalid command.");
    c->Message(Chat::White, "Usage: #advnpcspawn [maketype|makegroup|addgroupentry|addgroupspawn|setversion]");
    c->Message(Chat::White, "Usage: #advnpcspawn [removegroupspawn|movespawn|editrespawn|editgroupbox|cleargroupbox]");
}

void command_aggrozone(Client *c, const Seperator *sep) {
	if(!c)
		return;

	Mob *m = c->CastToMob();

	if (!m)
		return;

	uint32 hate = atoi(sep->arg[1]); //should default to 0 if we don't enter anything
	entity_list.AggroZone(m, hate);
	c->Message(Chat::White, "Train to you! Last chance to go invulnerable...");
}

void command_modifynpcstat(Client *c, const Seperator *sep)
{
	if(!c)
		return;

	if(sep->arg[1][0] == '\0')
	{
		c->Message(Chat::White, "usage #modifynpcstat arg value");
		c->Message(Chat::White, "Args: ac, str, sta, agi, dex, wis, _int, cha, max_hp, mr, fr, cr, pr, dr, runspeed, special_attacks, "
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
		c->Message(Chat::White, "#instance usage:");
		c->Message(Chat::White, "#instance create zone_id version duration - Creates an instance of version 'version' in the "
			"zone with id matching zone_id, will last for duration seconds.");
		c->Message(Chat::White, "#instance destroy instance_id - Destroys the instance with id matching instance_id.");
		c->Message(Chat::White, "#instance add instance_id player_name - adds the player 'player_name' to the instance "
			"with id matching instance_id.");
		c->Message(Chat::White, "#instance remove instance_id player_name - removes the player 'player_name' from the "
			"instance with id matching instance_id.");
		c->Message(Chat::White, "#instance list player_name - lists all the instances 'player_name' is apart of.");
		return;
	}
	else if(strcasecmp(sep->arg[1], "create") == 0)
	{
		if(!sep->IsNumber(3) || !sep->IsNumber(4))
		{
			c->Message(Chat::White, "#instance create zone_id version duration - Creates an instance of version 'version' in the "
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
			zone_id = ZoneID(sep->arg[2]);
		}

		uint32 version = atoi(sep->arg[3]);
		uint32 duration = atoi(sep->arg[4]);
		zn = ZoneName(zone_id);

		if(!zn)
		{
			c->Message(Chat::White, "Zone with id %lu was not found by the server.",  (unsigned long)zone_id);
			return;
		}

		uint16 id = 0;
		if(!database.GetUnusedInstanceID(id))
		{
			c->Message(Chat::White, "Server was unable to find a free instance id.");
			return;
		}

		if(!database.CreateInstance(id, zone_id, version, duration))
		{
			c->Message(Chat::White, "Server was unable to create a new instance.");
			return;
		}

		c->Message(Chat::White, "New instance %s was created with id %lu.",  zn, (unsigned long)id);
	}
	else if(strcasecmp(sep->arg[1], "destroy") == 0)
	{
		if(!sep->IsNumber(2))
		{
			c->Message(Chat::White, "#instance destroy instance_id - Destroys the instance with id matching instance_id.");
			return;
		}

		uint16 id = atoi(sep->arg[2]);
		database.DeleteInstance(id);
		c->Message(Chat::White, "Destroyed instance with id %lu.",  (unsigned long)id);
	}
	else if(strcasecmp(sep->arg[1], "add") == 0)
	{
		if(!sep->IsNumber(2))
		{
			c->Message(Chat::White, "#instance add instance_id player_name - adds the player 'player_name' to the instance "
				"with id matching instance_id.");
			return;
		}

		uint16 id = atoi(sep->arg[2]);
		uint32 charid = database.GetCharacterID(sep->arg[3]);

		if(id <= 0 || charid <= 0)
		{
			c->Message(Chat::White, "Must enter a valid instance id and player name.");
			return;
		}

		if(!database.CheckInstanceExists(id))
		{
			c->Message(Chat::White, "Instance does not exist.");
			return;
		}

		uint32 zone_id = database.ZoneIDFromInstanceID(id);
		uint32 version = database.VersionFromInstanceID(id);
		uint32 cur_id = database.GetInstanceID(zone_id, charid, version);
		if(cur_id == 0)
		{
			if(database.AddClientToInstance(id, charid))
			{
				c->Message(Chat::White, "Added client to instance.");
			}
			else
			{
				c->Message(Chat::White, "Failed to add client to instance.");
			}
		}
		else
		{
			c->Message(Chat::White, "Client was already saved to %u which has uses the same zone and version as that instance.",  cur_id);
		}
	}
	else if(strcasecmp(sep->arg[1], "remove") == 0)
	{
		if(!sep->IsNumber(2))
		{
			c->Message(Chat::White, "#instance remove instance_id player_name - removes the player 'player_name' from the "
				"instance with id matching instance_id.");
			return;
		}

		uint16 id = atoi(sep->arg[2]);
		uint32 charid = database.GetCharacterID(sep->arg[3]);

		if(id <= 0 || charid <= 0)
		{
			c->Message(Chat::White, "Must enter a valid instance id and player name.");
		}

		if(database.RemoveClientFromInstance(id, charid))
		{
			c->Message(Chat::White, "Removed client from instance.");
		}
		else
		{
			c->Message(Chat::White, "Failed to remove client from instance.");
		}
	}
	else if(strcasecmp(sep->arg[1], "list") == 0)
	{
		uint32 charid = database.GetCharacterID(sep->arg[2]);
		if(charid <= 0)
		{
			if(c->GetTarget() == nullptr || (c->GetTarget() && !c->GetTarget()->IsClient()))
			{
				c->Message(Chat::White, "Character not found.");
				return;
			}
			else
				charid = c->GetTarget()->CastToClient()->CharacterID();
		}

		database.ListAllInstances(c, charid);
	}
	else
	{
		c->Message(Chat::White, "Invalid Argument.");
		c->Message(Chat::White, "#instance usage:");
		c->Message(Chat::White, "#instance create zone_id version duration - Creates an instance of version 'version' in the "
			"zone with id matching zone_id, will last for duration seconds.");
		c->Message(Chat::White, "#instance destroy instance_id - Destroys the instance with id matching instance_id.");
		c->Message(Chat::White, "#instance add instance_id player_name - adds the player 'player_name' to the instance "
			"with id matching instance_id.");
		c->Message(Chat::White, "#instance remove instance_id player_name - removes the player 'player_name' from the "
			"instance with id matching instance_id.");
		c->Message(Chat::White, "#instance list player_name - lists all the instances 'player_name' is apart of.");
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
		c->Message(Chat::White, "Usage: (needs PC target) #setstartzone zonename");
		c->Message(Chat::White, "Optional Usage: Use '#setstartzone reset' or '#setstartzone 0' to clear a starting zone. Player can select a starting zone using /setstartcity");
		return;
	}

	if(sep->IsNumber(1)) {
		startzone = atoi(sep->arg[1]);
	}
	else if(strcasecmp(sep->arg[1],"reset") == 0) {
		startzone = 0;
	}
	else {
		startzone = ZoneID(sep->arg[1]);
		if(startzone == 0) {
			c->Message(Chat::White, "Unable to locate zone '%s'",  sep->arg[1]);
			return;
		}
	}

	target->SetStartZone(startzone);
}

void command_netstats(Client *c, const Seperator *sep)
{
	if(c)
	{
		auto client = c;
		if (c->GetTarget() && c->GetTarget()->IsClient()) {
			client = c->GetTarget()->CastToClient();
		}

		if (strcasecmp(sep->arg[1], "reset") == 0) {
			auto connection = c->Connection();
			c->Message(Chat::White, "Resetting client stats (packet loss will not read correctly after reset).");
			connection->ResetStats();
			return;
		}

		auto connection = c->Connection();
		auto opts = connection->GetManager()->GetOptions();
		auto eqs_stats = connection->GetStats();
		auto &stats = eqs_stats.DaybreakStats;
		auto now = EQ::Net::Clock::now();
		auto sec_since_stats_reset = std::chrono::duration_cast<std::chrono::duration<double>>(now - stats.created).count();

		c->Message(Chat::White, "Netstats:");
		c->Message(Chat::White, "--------------------------------------------------------------------");
		c->Message(Chat::White, "Sent Bytes: %u (%.2f/sec)", stats.sent_bytes, stats.sent_bytes / sec_since_stats_reset);
		c->Message(Chat::White, "Recv Bytes: %u (%.2f/sec)", stats.recv_bytes, stats.recv_bytes / sec_since_stats_reset);
		c->Message(Chat::White, "Bytes Before Encode (Sent): %u, Compression Rate: %.2f%%", stats.bytes_before_encode,
			static_cast<double>(stats.bytes_before_encode - stats.sent_bytes) / static_cast<double>(stats.bytes_before_encode) * 100.0);
		c->Message(Chat::White, "Bytes After Decode (Recv): %u, Compression Rate: %.2f%%", stats.bytes_after_decode,
			static_cast<double>(stats.bytes_after_decode - stats.recv_bytes) / static_cast<double>(stats.bytes_after_decode) * 100.0);
		c->Message(Chat::White, "Min Ping: %u", stats.min_ping);
		c->Message(Chat::White, "Max Ping: %u", stats.max_ping);
		c->Message(Chat::White, "Last Ping: %u", stats.last_ping);
		c->Message(Chat::White, "Average Ping: %u", stats.avg_ping);
		c->Message(Chat::White, "--------------------------------------------------------------------");
		c->Message(Chat::White, "(Realtime) Recv Packets: %u (%.2f/sec)", stats.recv_packets, stats.recv_packets / sec_since_stats_reset);
		c->Message(Chat::White, "(Realtime) Sent Packets: %u (%.2f/sec)", stats.sent_packets, stats.sent_packets / sec_since_stats_reset);
		c->Message(Chat::White, "(Sync) Recv Packets: %u", stats.sync_recv_packets);
		c->Message(Chat::White, "(Sync) Sent Packets: %u", stats.sync_sent_packets);
		c->Message(Chat::White, "(Sync) Remote Recv Packets: %u", stats.sync_remote_recv_packets);
		c->Message(Chat::White, "(Sync) Remote Sent Packets: %u", stats.sync_remote_sent_packets);
		c->Message(Chat::White, "Packet Loss In: %.2f%%", 100.0 * (1.0 - static_cast<double>(stats.sync_recv_packets) / static_cast<double>(stats.sync_remote_sent_packets)));
		c->Message(Chat::White, "Packet Loss Out: %.2f%%", 100.0 * (1.0 - static_cast<double>(stats.sync_remote_recv_packets) / static_cast<double>(stats.sync_sent_packets)));
		c->Message(Chat::White, "--------------------------------------------------------------------");
		c->Message(Chat::White, "Resent Packets: %u (%.2f/sec)", stats.resent_packets, stats.resent_packets / sec_since_stats_reset);
		c->Message(Chat::White, "Resent Fragments: %u (%.2f/sec)", stats.resent_fragments, stats.resent_fragments / sec_since_stats_reset);
		c->Message(Chat::White, "Resent Non-Fragments: %u (%.2f/sec)", stats.resent_full, stats.resent_full / sec_since_stats_reset);
		c->Message(Chat::White, "Dropped Datarate Packets: %u (%.2f/sec)", stats.dropped_datarate_packets, stats.dropped_datarate_packets / sec_since_stats_reset);

		if (opts.daybreak_options.outgoing_data_rate > 0.0) {
			c->Message(Chat::White, "Outgoing Link Saturation %.2f%% (%.2fkb/sec)", 100.0 * (1.0 - ((opts.daybreak_options.outgoing_data_rate - stats.datarate_remaining) / opts.daybreak_options.outgoing_data_rate)), opts.daybreak_options.outgoing_data_rate);
		}

		if (strcasecmp(sep->arg[1], "full") == 0) {
			c->Message(Chat::White, "--------------------------------------------------------------------");
			c->Message(Chat::White, "Sent Packet Types");
			for (auto i = 0; i < _maxEmuOpcode; ++i) {
				auto cnt = eqs_stats.SentCount[i];
				if (cnt > 0) {
					c->Message(Chat::White, "%s: %u (%.2f / sec)", OpcodeNames[i], cnt, cnt / sec_since_stats_reset);
				}
			}

			c->Message(Chat::White, "--------------------------------------------------------------------");
			c->Message(Chat::White, "Recv Packet Types");
			for (auto i = 0; i < _maxEmuOpcode; ++i) {
				auto cnt = eqs_stats.RecvCount[i];
				if (cnt > 0) {
					c->Message(Chat::White, "%s: %u (%.2f / sec)", OpcodeNames[i], cnt, cnt / sec_since_stats_reset);
				}
			}
		}

		c->Message(Chat::White, "--------------------------------------------------------------------");
	}
}

void command_object(Client *c, const Seperator *sep)
{
	if (!c)
		return; // Crash Suppressant: No client. How did we get here?

	// Save it here. We sometimes have need to refer to it in multiple places.
	const char *usage_string = "Usage: #object List|Add|Edit|Move|Rotate|Save|Copy|Delete|Undo";

	if ((!sep) || (sep->argnum == 0)) {
		c->Message(Chat::White, usage_string);
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
			c->Message(Chat::White, "Usage: #object List All|(radius)");
			return;
		}

		if ((sep->arg[2][0] & 0xDF) == 'A')
			radius = 0; // List All
		else if ((radius = atoi(sep->arg[2])) <= 0)
			radius = 500; // Invalid radius. Default to 500 units.

		if (radius == 0)
			c->Message(Chat::White, "Objects within this zone:");
		else
			c->Message(Chat::White, "Objects within %u units of your current location:", radius);

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

		auto results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			c->Message(Chat::White, "Error in objects query");
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

				c->Message(Chat::White, "- STATIC Object (%s): id %u, x %.1f, y %.1f, z %.1f, h %.1f, model %s, "
					      "size %u, solidtype %u, incline %u",
					   (od.object_type == 0) ? "locked" : "unlocked", id, od.x, od.y, od.z,
					   od.heading, od.object_name, od.size, od.solidtype, od.unknown020);
				break;

			case OT_DROPPEDITEM: // Ground Spawn
				c->Message(Chat::White, "- TEMPORARY Object: id %u, x %.1f, y %.1f, z %.1f, h %.1f, itemid %u, "
					      "model %s, icon %u",
					   id, od.x, od.y, od.z, od.heading, itemid, od.object_name, icon);
				break;

			default: // All others == Tradeskill Objects
				c->Message(Chat::White, "- TRADESKILL Object: id %u, x %.1f, y %.1f, z %.1f, h %.1f, model %s, "
					      "type %u, icon %u",
					   id, od.x, od.y, od.z, od.heading, od.object_name, od.object_type, icon);
				break;
			}
		}

		c->Message(Chat::White, "%u object%s found", results.RowCount(), (results.RowCount() == 1) ? "" : "s");
		return;
	}

	if (strcasecmp(sep->arg[1], "add") == 0) {
		// Insufficient or invalid arguments
		if ((sep->argnum < 3) ||
		    ((sep->arg[3][0] == '\0') && (sep->arg[4][0] < '0') && (sep->arg[4][0] > '9'))) {
			c->Message(Chat::White, "Usage: (Static Object): #object Add [ObjectID] 0 Model [SizePercent] "
				      "[SolidType] [Incline]");
			c->Message(Chat::White, "Usage: (Tradeskill Object): #object Add [ObjectID] TypeNum Model Icon");
			c->Message(Chat::White, "- Notes: Model must start with a letter, max length 16. SolidTypes = 0 (Solid), "
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
			c->Message(Chat::White, "ERROR: Object Type 1 is used for temporarily spawned ground spawns and dropped "
				      "items, which are not supported with #object. See the 'ground_spawns' table in "
				      "the database.");
			return;

		default: // Everything else == Tradeskill Object
			icon = ((sep->argnum - col) > 3) ? atoi(sep->arg[4 + col]) : 0;

			if (icon == 0) {
				c->Message(Chat::White, "ERROR: Required property 'Icon' not specified for Tradeskill Object");
				return;
			}

			break;
		}

		od.x = c->GetX();
		od.y = c->GetY();
		od.z = c->GetZ() - (c->GetSize() * 0.625f);
		od.heading = c->GetHeading();

		std::string query;
		if (id) {
			// ID specified. Verify that it doesn't already exist.
			query = StringFormat("SELECT COUNT(*) FROM object WHERE ID = %u", id);
			auto results = content_db.QueryDatabase(query);
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
				c->Message(Chat::White, "ERROR: An object already exists with the id %u", atoi(sep->arg[2]));
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

		auto results = content_db.QueryDatabase(query);
		if (results.Success() && results.RowCount() != 0) {
			auto row = results.begin();
			objectsFound = atoi(row[0]); // Number of nearby objects from database
		}

		// No objects found in database too close. How about spawned but not yet saved?
		if (objectsFound == 0 && entity_list.FindNearbyObject(od.x, od.y, od.z, 0.2f))
			objectsFound = 1;

		if (objectsFound) {
			c->Message(Chat::White, "ERROR: Object already at this location.");
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
			c->Message(Chat::White, "ERROR: Model name must start with a letter.");
			return;
		}

		if (id == 0) {
			// No ID specified. Get a best-guess next number from the database
			// If there's a problem retrieving an ID from the database, it'll end up being object # 1. No
			// biggie.

			query = "SELECT MAX(id) FROM object";
			results = content_db.QueryDatabase(query);
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

		x2 = 10.0f * sin(c->GetHeading() / 256.0f * 3.14159265f);
		y2 = 10.0f * cos(c->GetHeading() / 256.0f * 3.14159265f);
		c->MovePC(c->GetX() - x2, c->GetY() - y2, c->GetZ(), c->GetHeading());

		c->Message(Chat::White, "Spawning object with tentative id %u at location (%.1f, %.1f, %.1f heading %.1f). Use "
			      "'#object Save' to save to database when satisfied with placement.",
			   id, od.x, od.y, od.z, od.heading);

		// Temporary Static Object
		if (od.object_type == staticType)
			c->Message(Chat::White, "- Note: Static Object will act like a tradeskill container and will not reflect "
				      "size, solidtype, or incline values until you commit with '#object Save', after "
				      "which it will be unchangeable until you use '#object Edit' and zone back in.");

		return;
	}

	if (strcasecmp(sep->arg[1], "edit") == 0) {

		if ((sep->argnum < 2) || ((id = atoi(sep->arg[2])) < 1)) {
			c->Message(Chat::White, "Usage: #object Edit (ObjectID) [PropertyName] [NewValue]");
			c->Message(Chat::White, "- Static Object (Type 0) Properties: model, type, size, solidtype, incline");
			c->Message(Chat::White, "- Tradeskill Object (Type 2+) Properties: model, type, icon");

			return;
		}

		o = entity_list.FindObject(id);

		// Object already available in-zone?
		if (o) {
			// Yep, looks like we can make real-time changes.
			if (sep->argnum < 4) {
				// Or not. '#object Edit (ObjectID)' called without PropertyName and NewValue
				c->Message(Chat::White, "Note: Object %u already unlocked and ready for changes", id);
				return;
			}
		} else {
			// Object not found in-zone in a modifiable form. Check for valid matching circumstances.
			std::string query = StringFormat("SELECT zoneid, version, type FROM object WHERE id = %u", id);
			auto results = content_db.QueryDatabase(query);
			if (!results.Success() || results.RowCount() == 0) {
				c->Message(Chat::White, "ERROR: Object %u not found", id);
				return;
			}

			auto row = results.begin();
			od.zone_id = atoi(row[0]);
			od.zone_instance = atoi(row[1]);
			od.object_type = atoi(row[2]);
			uint32 objectsFound = 1;

			// Object not in this zone?
			if (od.zone_id != zone->GetZoneID()) {
				c->Message(Chat::White, "ERROR: Object %u not in this zone.", id);
				return;
			}

			// Object not in this instance?
			if (od.zone_instance != zone->GetInstanceVersion()) {
				c->Message(Chat::White, "ERROR: Object %u not part of this instance version.", id);
				return;
			}

			switch (od.object_type) {
			case 0: // Static object needing unlocking
				// Convert to tradeskill object temporarily for changes
				query = StringFormat("UPDATE object SET type = %u WHERE id = %u", staticType, id);

				content_db.QueryDatabase(query);

				c->Message(Chat::White, "Static Object %u unlocked for editing. You must zone out and back in to "
					      "make your changes, then commit them with '#object Save'.",
					   id);
				if (sep->argnum >= 4)
					c->Message(Chat::White, "NOTE: The change you specified has not been applied, since the "
						      "static object had not been unlocked for editing yet.");
				return;

			case OT_DROPPEDITEM:
				c->Message(Chat::White, "ERROR: Object %u is a temporarily spawned ground spawn or dropped item, "
					      "which cannot be manipulated with #object. See the 'ground_spawns' table "
					      "in the database.",
					   id);
				return;

			case staticType:
				c->Message(Chat::White, "ERROR: Object %u has been unlocked for editing, but you must zone out "
					      "and back in for your client to refresh its object table before you can "
					      "make changes to it.",
					   id);
				return;

			default:
				// Unknown error preventing us from seeing the object in the zone.
				c->Message(Chat::White, "ERROR: Unknown problem attempting to manipulate object %u", id);
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
				c->Message(Chat::White, "ERROR: Model names must begin with a letter.");
				return;
			}

			strn0cpy(od.object_name, sep->arg[4], sizeof(od.object_name));

			o->SetObjectData(&od);

			c->Message(Chat::White, "Object %u now being rendered with model '%s'", id, od.object_name);
		} else if (strcmp(sep->arg[3], "type") == 0) {
			if ((sep->arg[4][0] < '0') || (sep->arg[4][0] > '9')) {
				c->Message(Chat::White, "ERROR: Invalid type number");
				return;
			}

			od.object_type = atoi(sep->arg[4]);

			switch (od.object_type) {
			case 0:
				// Convert Static Object to temporary changeable type
				od.object_type = staticType;
				c->Message(Chat::White, "Note: Static Object will still act like tradeskill object and will not "
					      "reflect size, solidtype, or incline settings until committed to the "
					      "database with '#object Save', after which it will be unchangeable until "
					      "it is unlocked again with '#object Edit'.");
				break;

			case OT_DROPPEDITEM:
				c->Message(Chat::White, "ERROR: Object Type 1 is used for temporarily spawned ground spawns and "
					      "dropped items, which are not supported with #object. See the "
					      "'ground_spawns' table in the database.");
				return;

			default:
				c->Message(Chat::White, "Object %u changed to Tradeskill Object Type %u", id, od.object_type);
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
				c->Message(Chat::White, "ERROR: Invalid size specified. Please enter a number.");
				return;
			}

			od.size = atoi(sep->arg[4]);
			o->SetObjectData(&od);

			if (od.size == 0) // 0 == unspecified == 100%
				od.size = 100;

			c->Message(Chat::White, "Static Object %u set to %u%% size. Size will take effect when you commit to the "
				      "database with '#object Save', after which the object will be unchangeable until "
				      "you unlock it again with '#object Edit' and zone out and back in.",
				   id, od.size);
		} else if (strcmp(sep->arg[3], "solidtype") == 0) {

			if (od.object_type != staticType) {
				c->Message(Chat::White, "ERROR: Object %u is not a Static Object and does not support the "
					      "SolidType property",
					   id);
				return;
			}

			if ((sep->arg[4][0] < '0') || (sep->arg[4][0] > '9')) {
				c->Message(Chat::White, "ERROR: Invalid solidtype specified. Please enter a number.");
				return;
			}

			od.solidtype = atoi(sep->arg[4]);
			o->SetObjectData(&od);

			c->Message(Chat::White, "Static Object %u set to SolidType %u. Change will take effect when you commit "
				      "to the database with '#object Save'. Support for this property is on a "
				      "per-model basis, mostly seen in smaller objects such as chests and tables.",
					  id, od.solidtype);
		} else if (strcmp(sep->arg[3], "icon") == 0) {

			if ((od.object_type < 2) || (od.object_type == staticType)) {
				c->Message(Chat::White, "ERROR: Object %u is not a Tradeskill Object and does not support the "
					      "Icon property",
					   id);
				return;
			}

			if ((icon = atoi(sep->arg[4])) == 0) {
				c->Message(Chat::White, "ERROR: Invalid Icon specified. Please enter an icon number.");
				return;
			}

			o->SetIcon(icon);
			c->Message(Chat::White, "Tradeskill Object %u icon set to %u", id, icon);
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

			c->Message(Chat::White, "Static Object %u set to %u incline. Incline will take effect when you commit to "
				      "the database with '#object Save', after which the object will be unchangeable "
				      "until you unlock it again with '#object Edit' and zone out and back in.",
				   id, od.unknown020);
		} else {
			c->Message(Chat::White, "ERROR: Unrecognized property name: %s", sep->arg[3]);
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
			c->Message(Chat::White, "Usage: #object Move (ObjectID) ToMe|(x y z [h])");
			return;
		}

		if (!(o = entity_list.FindObject(id))) {
			std::string query = StringFormat("SELECT zoneid, version, type FROM object WHERE id = %u", id);
			auto results = content_db.QueryDatabase(query);
			if (!results.Success() || results.RowCount() == 0) {
				c->Message(Chat::White, "ERROR: Object %u not found", id);
				return;
			}

			auto row = results.begin();
			od.zone_id = atoi(row[0]);
			od.zone_instance = atoi(row[1]);
			od.object_type = atoi(row[2]);

			if (od.zone_id != zone->GetZoneID()) {
				c->Message(Chat::White, "ERROR: Object %u is not in this zone", id);
				return;
			}

			if (od.zone_instance != zone->GetInstanceVersion()) {
				c->Message(Chat::White, "ERROR: Object %u is not in this instance version", id);
				return;
			}

			switch (od.object_type) {
			case 0:
				c->Message(Chat::White, "ERROR: Object %u is not yet unlocked for editing. Use '#object Edit' "
					      "then zone out and back in to move it.",
					   id);
				return;

			case staticType:
				c->Message(Chat::White, "ERROR: Object %u has been unlocked for editing, but you must zone out "
					      "and back in before your client sees the change and will allow you to "
					      "move it.",
					   id);
				return;

			case 1:
				c->Message(Chat::White, "ERROR: Object %u is a temporary spawned object and cannot be "
					      "manipulated with #object. See the 'ground_spawns' table in the "
					      "database.",
					   id);
				return;

			default:
				c->Message(Chat::White, "ERROR: Object %u not located in zone.", id);
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

			o->SetHeading(c->GetHeading());

			// Bump player back to avoid getting stuck inside object

			x2 = 10.0f * std::sin(c->GetHeading() / 256.0f * 3.14159265f);
			y2 = 10.0f * std::cos(c->GetHeading() / 256.0f * 3.14159265f);
			c->MovePC(c->GetX() - x2, c->GetY() - y2, c->GetZ(), c->GetHeading());
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
			c->Message(Chat::White, "Usage: #object Rotate (ObjectID) (Heading, 0-512)");
			return;
		}

		if ((o = entity_list.FindObject(id)) == nullptr) {
			c->Message(Chat::White, "ERROR: Object %u not found in zone, or is a static object not yet unlocked with "
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
			c->Message(Chat::White, "Usage: #object Save (ObjectID)");
			return;
		}

		o = entity_list.FindObject(id);

		od.zone_id = 0;
		od.zone_instance = 0;
		od.object_type = 0;

		// If this ID isn't in the database yet, it's a new object
		bNewObject = true;
		std::string query = StringFormat("SELECT zoneid, version, type FROM object WHERE id = %u", id);
		auto results = content_db.QueryDatabase(query);
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
				c->Message(Chat::White, "ERROR: Object %u not found", id);
				return;
			}

			if (od.zone_id != zone->GetZoneID()) {
				c->Message(Chat::White, "ERROR: Wrong Object ID. %u is not part of this zone.", id);
				return;
			}

			if (od.zone_instance != zone->GetInstanceVersion()) {
				c->Message(Chat::White, "ERROR: Wrong Object ID. %u is not part of this instance version.", id);
				return;
			}

			if (od.object_type == 0) {
				c->Message(Chat::White, "ERROR: Static Object %u has already been committed. Use '#object Edit "
					      "%u' and zone out and back in to make changes.",
					   id, id);
				return;
			}

			if (od.object_type == 1) {
				c->Message(Chat::White, "ERROR: Object %u is a temporarily spawned ground spawn or dropped item, "
					      "which is not supported with #object. See the 'ground_spawns' table in "
					      "the database.",
					   id);
				return;
			}

			c->Message(Chat::White, "ERROR: Object %u not found.", id);
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

		results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			c->Message(Chat::White, "Database Error: %s", results.ErrorMessage().c_str());
			return;
		}

		if (results.RowsAffected() == 0) {
			// No change made, but no error message given
			c->Message(Chat::White, "Database Error: Could not save change to Object %u", id);
			return;
		}

		if (bNewObject) {
			if (newid == results.LastInsertedID()) {
				c->Message(Chat::White, "Saved new Object %u to database", id);
				return;
			}

			c->Message(Chat::White, "Saved Object. NOTE: Database returned a new ID number for object: %u", newid);
			id = newid;
			return;
		}

		c->Message(Chat::White, "Saved changes to Object %u", id);
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

			c->Message(Chat::White, "NOTE: Object %u is now a static object, and is unchangeable. To make future "
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
			c->Message(Chat::White, "Usage: #object Copy All|(ObjectID) (InstanceVersion)");
			c->Message(Chat::White, "- Note: Only objects saved in the database can be copied to another instance.");
			return;
		}

		od.zone_instance = atoi(sep->arg[3]);

		if (od.zone_instance == zone->GetInstanceVersion()) {
			c->Message(Chat::White, "ERROR: Source and destination instance versions are the same.");
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
			auto results = content_db.QueryDatabase(query);
			if (!results.Success()) {
				c->Message(Chat::White, "Database Error: %s", results.ErrorMessage().c_str());
				return;
			}

			c->Message(Chat::White, "Copied %u object%s into instance version %u", results.RowCount(),
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
		auto results = content_db.QueryDatabase(query);
		if (results.Success() && results.RowsAffected() > 0) {
			c->Message(Chat::White, "Copied Object %u into instance version %u", id, od.zone_instance);
			return;
		}

		// Couldn't copy the object.

		// got an error message
		if (!results.Success()) {
			c->Message(Chat::White, "Database Error: %s", results.ErrorMessage().c_str());
			return;
		}

		// No database error returned. See if we can figure out why.

		query = StringFormat("SELECT zoneid, version FROM object WHERE id = %u", id);
		results = content_db.QueryDatabase(query);
		if (!results.Success())
			return;

		if (results.RowCount() == 0) {
			c->Message(Chat::White, "ERROR: Object %u not found", id);
			return;
		}

		auto row = results.begin();
		// Wrong ZoneID?
		if (atoi(row[0]) != zone->GetZoneID()) {
			c->Message(Chat::White, "ERROR: Object %u is not part of this zone.", id);
			return;
		}

		// Wrong Instance Version?
		if (atoi(row[1]) != zone->GetInstanceVersion()) {
			c->Message(Chat::White, "ERROR: Object %u is not part of this instance version.", id);
			return;
		}

		// Well, NO clue at this point. Just let 'em know something screwed up.
		c->Message(Chat::White, "ERROR: Unknown database error copying Object %u to instance version %u", id,
			   od.zone_instance);
		return;
	}

	if (strcasecmp(sep->arg[1], "delete") == 0) {

		if ((sep->argnum < 2) || ((id = atoi(sep->arg[2])) <= 0)) {
			c->Message(Chat::White, "Usage: #object Delete (ObjectID) -- NOTE: Object deletions are permanent and "
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
			auto results = content_db.QueryDatabase(query);

			c->Message(Chat::White, "Object %u deleted", id);
			return;
		}

		// Object not found in zone.
		std::string query = StringFormat("SELECT type FROM object "
						 "WHERE id = %u AND zoneid = %u "
						 "AND version = %u LIMIT 1",
						 id, zone->GetZoneID(), zone->GetInstanceVersion());
		auto results = content_db.QueryDatabase(query);
		if (!results.Success())
			return;

		if (results.RowCount() == 0) {
			c->Message(Chat::White, "ERROR: Object %u not found in this zone or instance!", id);
			return;
		}

		auto row = results.begin();

		switch (atoi(row[0])) {
		case 0: // Static Object
			query = StringFormat("DELETE FROM object WHERE id = %u "
					     "AND zoneid = %u AND version = %u LIMIT 1",
					     id, zone->GetZoneID(), zone->GetInstanceVersion());
			results = content_db.QueryDatabase(query);

			c->Message(Chat::White, "Object %u deleted. NOTE: This static object will remain for anyone currently in "
				      "the zone until they next zone out and in.",
				   id);
			return;

		case 1: // Temporary Spawn
			c->Message(Chat::White, "ERROR: Object %u is a temporarily spawned ground spawn or dropped item, which "
				      "is not supported with #object. See the 'ground_spawns' table in the database.",
				   id);
			return;
		}

		return;
	}

	if (strcasecmp(sep->arg[1], "undo") == 0) {
		// Insufficient or invalid arguments
		if ((sep->argnum < 2) || ((id = atoi(sep->arg[2])) == 0)) {
			c->Message(Chat::White, "Usage: #object Undo (ObjectID) -- Reload object from database, undoing any "
				      "changes you have made");
			return;
		}

		o = entity_list.FindObject(id);

		if (!o) {
			c->Message(Chat::White, "ERROR: Object %u not found in zone in a manipulable form. No changes to undo.",
				   id);
			return;
		}

		if (o->GetType() == OT_DROPPEDITEM) {
			c->Message(Chat::White, "ERROR: Object %u is a temporary spawned item and cannot be manipulated with "
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
		auto results = content_db.QueryDatabase(query);
		if (!results.Success() || results.RowCount() == 0) {
			c->Message(Chat::White, "Database Error: %s", results.ErrorMessage().c_str());
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

		c->Message(Chat::White, "Object %u reloaded from database.", id);
		return;
	}

	c->Message(Chat::White, usage_string);
}

void command_showspellslist(Client *c, const Seperator *sep)
{
	Mob *target = c->GetTarget();

	if (!target) {
		c->Message(Chat::White, "Must target an NPC.");
		return;
	}

	if (!target->IsNPC()) {
		c->Message(Chat::White, "%s is not an NPC.",  target->GetName());
		return;
	}

	target->CastToNPC()->AISpellsList(c);

	return;
}

void command_raidloot(Client *c, const Seperator *sep)
{
	if(!sep->arg[1][0]) {
		c->Message(Chat::White, "Usage: #raidloot [LEADER/GROUPLEADER/SELECTED/ALL]");
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
					c->Message(Chat::White, "You must be the raid leader to use this command.");
				}
				else
				{
					break;
				}
			}
		}

		if(strcasecmp(sep->arg[1], "LEADER") == 0)
		{
			c->Message(Chat::Yellow, "Loot type changed to: 1");
			r->ChangeLootType(1);
		}
		else if(strcasecmp(sep->arg[1], "GROUPLEADER") == 0)
		{
			c->Message(Chat::Yellow, "Loot type changed to: 2");
			r->ChangeLootType(2);
		}
		else if(strcasecmp(sep->arg[1], "SELECTED") == 0)
		{
			c->Message(Chat::Yellow, "Loot type changed to: 3");
			r->ChangeLootType(3);
		}
		else if(strcasecmp(sep->arg[1], "ALL") == 0)
		{
			c->Message(Chat::Yellow, "Loot type changed to: 4");
			r->ChangeLootType(4);
		}
		else
		{
			c->Message(Chat::White, "Usage: #raidloot [LEADER/GROUPLEADER/SELECTED/ALL]");
		}
	}
	else
	{
		c->Message(Chat::White, "You must be in a raid to use that command.");
	}
}

void command_emoteview(Client *c, const Seperator *sep)
{
	if(!c->GetTarget() || !c->GetTarget()->IsNPC())
	{
		c->Message(Chat::White, "You must target a NPC to view their emotes.");
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
				c->Message(Chat::White, "EmoteID: %i Event: %i Type: %i Text: %s",  nes->emoteid, nes->event_, nes->type, nes->text);
				count++;
			}
			iterator.Advance();
		}
		if (count == 0)
			c->Message(Chat::White, "No emotes found.");
		else
			c->Message(Chat::White, "%i emote(s) found",  count);
	}
}

void command_emotesearch(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0)
		c->Message(Chat::White, "Usage: #emotesearch [search string or emoteid]");
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
					c->Message(Chat::White, "EmoteID: %i Event: %i Type: %i Text: %s",  nes->emoteid, nes->event_, nes->type, nes->text);
					count++;
				}
				iterator.Advance();
			}
			if (count == 0)
				c->Message(Chat::White, "No emotes found.");
			else
				c->Message(Chat::White, "%i emote(s) found",  count);
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
					c->Message(Chat::White, "EmoteID: %i Event: %i Type: %i Text: %s",  nes->emoteid, nes->event_, nes->type, nes->text);
					count++;
				}
				if (count == 50)
					break;

				iterator.Advance();
			}
			if (count == 50)
				c->Message(Chat::White, "50 emotes shown...too many results.");
			else
				c->Message(Chat::White, "%i emote(s) found",  count);
		}
	}
}

void command_reloademote(Client *c, const Seperator *sep)
{
	zone->NPCEmoteList.Clear();
	zone->LoadNPCEmotes(&zone->NPCEmoteList);
	c->Message(Chat::White, "NPC emotes reloaded.");
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

		c->Message(Chat::White, "Name, Value");
		while(iter != globalMap.end())
		{
			c->Message(Chat::White, "%s %s",  (*iter).name.c_str(), (*iter).value.c_str());
			++iter;
			++gcount;
		}
		c->Message(Chat::White, "%u globals loaded.",  gcount);
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

		c->Message(Chat::White, "Name, Value");
		while(iter != globalMap.end())
		{
			c->Message(Chat::White, "%s %s",  (*iter).name.c_str(), (*iter).value.c_str());
			++iter;
			++gcount;
		}
		c->Message(Chat::White, "%u globals loaded.",  gcount);
	}
}

void command_distance(Client *c, const Seperator *sep) {
	if(c && c->GetTarget()) {
		Mob* target = c->GetTarget();

		c->Message(Chat::White, "Your target, %s, is %1.1f units from you.",  c->GetTarget()->GetName(), Distance(c->GetPosition(), target->GetPosition()));
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
		for (int i = 0; i <= EQ::skills::HIGHEST_SKILL; ++i)
		{
			if (i >= EQ::skills::SkillSpecializeAbjure && i <= EQ::skills::SkillSpecializeEvocation)
			{
				c->SetSkill((EQ::skills::SkillType)i, 50);
			}
			else
			{
				int max_skill_level = content_db.GetSkillCap(c->GetClass(), (EQ::skills::SkillType)i, c->GetLevel());
				c->SetSkill((EQ::skills::SkillType)i, max_skill_level);
			}
		}
	}
}

void command_showbonusstats(Client *c, const Seperator *sep)
{
	if (c->GetTarget() == 0)
		c->Message(Chat::White, "ERROR: No target!");
	else if (!c->GetTarget()->IsMob() && !c->GetTarget()->IsClient())
		c->Message(Chat::White, "ERROR: Target is not a Mob or Player!");
	else {
		bool bAll = false;
		if(sep->arg[1][0] == '\0' || strcasecmp(sep->arg[1], "all") == 0)
			bAll = true;
		if (bAll || (strcasecmp(sep->arg[1], "item")==0)) {
			c->Message(Chat::White, "Target Item Bonuses:");
			c->Message(Chat::White, "  Accuracy: %i%%   Divine Save: %i%%", c->GetTarget()->GetItemBonuses().Accuracy, c->GetTarget()->GetItemBonuses().DivineSaveChance);
			c->Message(Chat::White, "  Flurry: %i%%     HitChance: %i%%", c->GetTarget()->GetItemBonuses().FlurryChance, c->GetTarget()->GetItemBonuses().HitChance / 15);
		}
		if (bAll || (strcasecmp(sep->arg[1], "spell")==0)) {
			c->Message(Chat::White, "  Target Spell Bonuses:");
			c->Message(Chat::White, "  Accuracy: %i%%   Divine Save: %i%%", c->GetTarget()->GetSpellBonuses().Accuracy, c->GetTarget()->GetSpellBonuses().DivineSaveChance);
			c->Message(Chat::White, "  Flurry: %i%%     HitChance: %i%% ", c->GetTarget()->GetSpellBonuses().FlurryChance, c->GetTarget()->GetSpellBonuses().HitChance / 15);
		}
		c->Message(Chat::White, "  Effective Casting Level: %i", c->GetTarget()->GetCasterLevel(0));
	}
}

void command_reloadallrules(Client *c, const Seperator *sep)
{
	if(c)
	{
		auto pack = new ServerPacket(ServerOP_ReloadRules, 0);
		worldserver.SendPacket(pack);
		c->Message(Chat::Red, "Successfully sent the packet to world to reload rules globally. (including world)");
		safe_delete(pack);

	}
}

void command_reloadworldrules(Client *c, const Seperator *sep)
{
	if(c)
	{
		auto pack = new ServerPacket(ServerOP_ReloadRulesWorld, 0);
		worldserver.SendPacket(pack);
		c->Message(Chat::Red, "Successfully sent the packet to world to reload rules. (only world)");
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
			c->Message(Chat::Red, "Successfully sent the packet to world! Shake it, world, shake it!");
			safe_delete(pack);
		}
		else {
			c->Message(Chat::Red, "Usage -- #camerashake [duration], [intensity [1-10])");
		}
	}
	return;
}

void command_disarmtrap(Client *c, const Seperator *sep)
{
	Mob *target = c->GetTarget();

	if(!target)
	{
		c->Message(Chat::Red, "You must have a target.");
		return;
	}

	if(target->IsNPC())
	{
		if (c->HasSkill(EQ::skills::SkillDisarmTraps))
		{
			if(DistanceSquaredNoZ(c->GetPosition(), target->GetPosition()) > RuleI(Adventure, LDoNTrapDistanceUse))
			{
				c->Message(Chat::Red, "%s is too far away.",  target->GetCleanName());
				return;
			}
			c->HandleLDoNDisarm(target->CastToNPC(), c->GetSkill(EQ::skills::SkillDisarmTraps), LDoNTypeMechanical);
		}
		else
			c->Message(Chat::Red, "You do not have the disarm trap skill.");
	}
}

void command_sensetrap(Client *c, const Seperator *sep)
{
	Mob * target = c->GetTarget();
	if(!target)
	{
		c->Message(Chat::Red, "You must have a target.");
		return;
	}

	if(target->IsNPC())
	{
		if (c->HasSkill(EQ::skills::SkillSenseTraps))
		{
			if(DistanceSquaredNoZ(c->GetPosition(), target->GetPosition()) > RuleI(Adventure, LDoNTrapDistanceUse))
			{
				c->Message(Chat::Red, "%s is too far away.",  target->GetCleanName());
				return;
			}
			c->HandleLDoNSenseTraps(target->CastToNPC(), c->GetSkill(EQ::skills::SkillSenseTraps), LDoNTypeMechanical);
		}
		else
			c->Message(Chat::Red, "You do not have the sense traps skill.");
	}
}

void command_picklock(Client *c, const Seperator *sep)
{
	Mob * target = c->GetTarget();
	if(!target)
	{
		c->Message(Chat::Red, "You must have a target.");
		return;
	}

	if(target->IsNPC())
	{
		if (c->HasSkill(EQ::skills::SkillPickLock))
		{
			if(DistanceSquaredNoZ(c->GetPosition(), target->GetPosition()) > RuleI(Adventure, LDoNTrapDistanceUse))
			{
				c->Message(Chat::Red, "%s is too far away.",  target->GetCleanName());
				return;
			}
			c->HandleLDoNPickLock(target->CastToNPC(), c->GetSkill(EQ::skills::SkillPickLock), LDoNTypeMechanical);
		}
		else
			c->Message(Chat::Red, "You do not have the pick locks skill.");
	}
}

void command_profanity(Client *c, const Seperator *sep)
{
	std::string arg1(sep->arg[1]);

	while (true) {
		if (arg1.compare("list") == 0) {
			// do nothing
		}
		else if (arg1.compare("clear") == 0) {
			EQ::ProfanityManager::DeleteProfanityList(&database);
			auto pack = new ServerPacket(ServerOP_RefreshCensorship);
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
		else if (arg1.compare("add") == 0) {
			if (!EQ::ProfanityManager::AddProfanity(&database, sep->arg[2]))
				c->Message(Chat::Red, "Could not add '%s' to the profanity list.", sep->arg[2]);
			auto pack = new ServerPacket(ServerOP_RefreshCensorship);
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
		else if (arg1.compare("del") == 0) {
			if (!EQ::ProfanityManager::RemoveProfanity(&database, sep->arg[2]))
				c->Message(Chat::Red, "Could not delete '%s' from the profanity list.", sep->arg[2]);
			auto pack = new ServerPacket(ServerOP_RefreshCensorship);
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
		else if (arg1.compare("reload") == 0) {
			if (!EQ::ProfanityManager::UpdateProfanityList(&database))
				c->Message(Chat::Red, "Could not reload the profanity list.");
			auto pack = new ServerPacket(ServerOP_RefreshCensorship);
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
		else {
			break;
		}

		std::string popup;
		const auto &list = EQ::ProfanityManager::GetProfanityList();
		for (const auto &iter : list) {
			popup.append(iter);
			popup.append("<br>");
		}
		if (list.empty())
			popup.append("** Censorship Inactive **<br>");
		else
			popup.append("** End of List **<br>");

		c->SendPopupToClient("Profanity List", popup.c_str());

		return;
	}

	c->Message(Chat::White, "Usage: #profanity [list] - shows profanity list");
	c->Message(Chat::White, "Usage: #profanity [clear] - deletes all entries");
	c->Message(Chat::White, "Usage: #profanity [add] [<word>] - adds entry");
	c->Message(Chat::White, "Usage: #profanity [del] [<word>] - deletes entry");
	c->Message(Chat::White, "Usage: #profanity [reload] - reloads profanity list");
}

void command_mysql(Client *c, const Seperator *sep)
{
	if(!sep->arg[1][0] || !sep->arg[2][0]) {
		c->Message(Chat::White, "Usage: #mysql query \"Query here\"");
		return;
	}

	if (strcasecmp(sep->arg[1], "help") == 0) {
		c->Message(Chat::White, "MYSQL In-Game CLI Interface:");
		c->Message(Chat::White, "Example: #mysql query \"Query goes here quoted\" -s -h");
		c->Message(Chat::White, "To use 'like \"%%something%%\" replace the %% with #");
		c->Message(Chat::White, "Example: #mysql query \"select * from table where name like \"#something#\"");
		c->Message(Chat::White, "-s - Spaces select entries apart");
		c->Message(Chat::White, "-h - Colors every other select result");
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
                    c->Message(Chat::Yellow, "%s, there is no option '%c'",  c->GetName(), sep->arg[argnum][1]);
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
        c->Message(Chat::Yellow, "---Running query: '%s'",  query.c_str());

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
                c->Message(Chat::White, " ");
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
			c->Message(Chat::Red, "Number of XTargets must be between 5 and %i",  XTARGET_HARDCAP);
			return;
		}
		t->SetMaxXTargets(NewMax);
		c->Message(Chat::White, "Max number of XTargets set to %i",  NewMax);
	}
	else
		t->ShowXTargets(c);
}

void command_zopp(Client *c, const Seperator *sep)
{ // - Owner only command..non-targetable to eliminate malicious or mischievious activities.
	if (!c)
		return;
	else if (sep->argnum < 3 || sep->argnum > 4)
		c->Message(Chat::White, "Usage: #zopp [trade/summon] [slot id] [item id] [*charges]");
	else if (!strcasecmp(sep->arg[1], "trade") == 0 && !strcasecmp(sep->arg[1], "t") == 0 && !strcasecmp(sep->arg[1], "summon") == 0 && !strcasecmp(sep->arg[1], "s") == 0)
		c->Message(Chat::White, "Usage: #zopp [trade/summon] [slot id] [item id] [*charges]");
	else if (!sep->IsNumber(2) || !sep->IsNumber(3) || (sep->argnum == 4 && !sep->IsNumber(4)))
		c->Message(Chat::White, "Usage: #zopp [trade/summon] [slot id] [item id] [*charges]");
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

		const EQ::ItemData* FakeItem = database.GetItem(itemid);

		if (!FakeItem) {
			c->Message(Chat::Red, "Error: Item [%u] is not a valid item id.",  itemid);
			return;
		}

		int16 item_status = 0;
		const EQ::ItemData* item = database.GetItem(itemid);
		if(item) {
			item_status = static_cast<int16>(item->MinStatus);
		}
		if (item_status > c->Admin()) {
			c->Message(Chat::Red, "Error: Insufficient status to use this command.");
			return;
		}

		if (charges < 0 || charges > FakeItem->StackSize) {
			c->Message(Chat::Red, "Warning: The specified charge count does not meet expected criteria!");
			c->Message(Chat::White, "Processing request..results may cause unpredictable behavior.");
		}

		EQ::ItemInstance* FakeItemInst = database.CreateItem(FakeItem, charges);
		c->SendItemPacket(slotid, FakeItemInst, packettype);
		c->Message(Chat::White, "Sending zephyr op packet to client - [%s] %s (%u) with %i %s to slot %i.",
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
	c->Message(Chat::White, "Current Quest Errors:");

	auto iter = err.begin();
	int i = 0;
	while(iter != err.end()) {
		if(i >= 30) {
			c->Message(Chat::White, "Maximum of 30 Errors shown...");
			break;
		}

		c->Message(Chat::White, iter->c_str());
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
			c->Message(Chat::White, "Invalid number of arguments.\nUsage: #enablerecipe recipe_id");
			return;
		}
		if (recipe_id > 0) {
			success = content_db.EnableRecipe(recipe_id);
			if (success) {
				c->Message(Chat::White, "Recipe enabled.");
			}
			else {
				c->Message(Chat::White, "Recipe not enabled.");
			}
		}
		else {
			c->Message(Chat::White, "Invalid recipe id.\nUsage: #enablerecipe recipe_id");
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
			c->Message(Chat::White, "Invalid number of arguments.\nUsage: #disablerecipe recipe_id");
			return;
		}
		if (recipe_id > 0) {
			success = content_db.DisableRecipe(recipe_id);
			if (success) {
				c->Message(Chat::White, "Recipe disabled.");
			}
			else {
				c->Message(Chat::White, "Recipe not disabled.");
			}
		}
		else {
			c->Message(Chat::White, "Invalid recipe id.\nUsage: #disablerecipe recipe_id");
		}
	}
}

void command_npctype_cache(Client *c, const Seperator *sep)
{
	if (sep->argnum > 0) {
		for (int i = 0; i < sep->argnum; ++i) {
			if (strcasecmp(sep->arg[i + 1], "all") == 0) {
				c->Message(Chat::White, "Clearing all npc types from the cache.");
				zone->ClearNPCTypeCache(-1);
			}
			else {
				int id = atoi(sep->arg[i + 1]);
				if (id > 0) {
					c->Message(Chat::White, "Clearing npc type %d from the cache.",  id);
					zone->ClearNPCTypeCache(id);
					return;
				}
			}
		}
	}
	else {
		c->Message(Chat::White, "Usage:");
		c->Message(Chat::White, "#npctype_cache [npctype_id] ...");
		c->Message(Chat::White, "#npctype_cache all");
	}
}

void command_merchantopenshop(Client *c, const Seperator *sep)
{
	Mob *merchant = c->GetTarget();
	if (!merchant || merchant->GetClass() != MERCHANT) {
		c->Message(Chat::White, "You must target a merchant to open their shop.");
		return;
	}

	merchant->CastToNPC()->MerchantOpenShop();
}

void command_merchantcloseshop(Client *c, const Seperator *sep)
{
	Mob *merchant = c->GetTarget();
	if (!merchant || merchant->GetClass() != MERCHANT) {
		c->Message(Chat::White, "You must target a merchant to close their shop.");
		return;
	}

	merchant->CastToNPC()->MerchantCloseShop();
}

void command_shownumhits(Client *c, const Seperator *sep)
{
	c->ShowNumHits();
	return;
}

void command_shownpcgloballoot(Client *c, const Seperator *sep)
{
	auto tar = c->GetTarget();

	if (!tar || !tar->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	auto npc = tar->CastToNPC();
	c->Message(Chat::White, "GlobalLoot for %s (%d)", npc->GetName(), npc->GetNPCTypeID());
	zone->ShowNPCGlobalLoot(c, npc);
}

void command_tune(Client *c, const Seperator *sep)
{
	//Work in progress - Kayen

	if(sep->arg[1][0] == '\0' || !strcasecmp(sep->arg[1], "help")) {
		c->Message(Chat::White, "Syntax: #tune [subcommand].");
		c->Message(Chat::White, "-- Tune System Commands --");
		c->Message(Chat::White, "-- Usage: Returning recommended combat statistical values based on a desired outcome.");
		c->Message(Chat::White, "-- Note: If targeted mob does not have a target (ie not engaged in combat), YOU will be considered the target.");
		c->Message(Chat::White, "-- Warning: The calculations done in this process are intense and can potentially cause zone crashes depending on parameters set, use with caution!");
		c->Message(Chat::White, "-- Below are OPTIONAL parameters.");
		c->Message(Chat::White, "-- Note: [interval] Determines how fast the stat being checked increases/decreases till it finds the best result. Default [ATK/AC 50][Acc/Avoid 10] ");
		c->Message(Chat::White, "-- Note: [loop_max] Determines how many iterations are done to increases/decreases the stat till it finds the best result. Default [ATK/AC 100][Acc/Avoid 1000]");
		c->Message(Chat::White, "-- Note: [Stat Override] Will override that stat on mob being checkd with the specified value. Default=0");
		c->Message(Chat::White, "-- Note: [Info Level] How much statistical detail is displayed[0 - 3]. Default=0 ");
		c->Message(Chat::White, "-- Note: Results are only approximations usually accurate to +/- 2 intervals.");

		c->Message(Chat::White, "... ");
		c->Message(Chat::White, "...### Category A ### Target = ATTACKER ### YOU or Target's Target = DEFENDER ###");
		c->Message(Chat::White, "...### Category B ### Target = DEFENDER ### YOU or Target's Target = ATTACKER ###");
		c->Message(Chat::White, "... ");
		c->Message(Chat::White, "...#Returns recommended ATK adjustment +/- on ATTACKER that will result in an average mitigation pct on DEFENDER. ");
		c->Message(Chat::White, "...tune FindATK [A/B] [pct mitigation] [interval][loop_max][AC Overwride][Info Level]");
		c->Message(Chat::White, "... ");
		c->Message(Chat::White, "...#Returns recommended AC adjustment +/- on DEFENDER for an average mitigation pct from ATTACKER. ");
		c->Message(Chat::White, "...tune FindAC [A/B] [pct mitigation] [interval][loop_max][ATK Overwride][Info Level] ");
		c->Message(Chat::White, "... ");
		c->Message(Chat::White, "...#Returns recommended Accuracy adjustment +/- on ATTACKER that will result in a hit chance pct on DEFENDER. ");
		c->Message(Chat::White, "...tune FindAccuracy [A/B] [hit chance] [interval][loop_max][Avoidance Overwride][Info Level]");
		c->Message(Chat::White, "... ");
		c->Message(Chat::White, "...#Returns recommended Avoidance adjustment +/- on DEFENDER for in a hit chance pct from ATTACKER. ");
		c->Message(Chat::White, "...tune FindAvoidance [A/B] [pct mitigation] [interval][loop_max][Accuracy Overwride][Info Level] ");

		return;
	}
	//Default is category A for attacker/defender settings, which then are swapped under category B.
	Mob* defender = c;
	Mob* attacker = c->GetTarget();

	if (!attacker)
	{
		c->Message(Chat::White, "#Tune - Error no target selected. [#Tune help]");
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
			c->Message(Chat::Red, "#Tune - Error must enter the desired percent mitigation on defender. Ie. Defender to mitigate on average 20 pct of max damage.");
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
			c->Message(Chat::White, "#Tune - Error no category selcted. [#Tune help]");
			c->Message(Chat::White, "Usage #tune FindATK [A/B] [pct mitigation] [interval][loop_max][AC Overwride][Info Level] ");
			c->Message(Chat::White, "Example #tune FindATK A 60");
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
			c->Message(Chat::Red, "#Tune - Error must enter the desired percent mitigation on defender. Ie. Defender to mitigate on average 20 pct of max damage.");
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
			c->Message(Chat::White, "#Tune - Error no category selcted. [#Tune help]");
			c->Message(Chat::White, "Usage #tune FindAC [A/B] [pct mitigation] [interval][loop_max][ATK Overwride][Info Level] ");
			c->Message(Chat::White, "Example #tune FindAC A 60");
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
			c->Message(Chat::NPCQuestSay, "#Tune - Error must enter the desired percent mitigation on defender. Ie. Defender to mitigate on average 20 pct of max damage.");
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
			c->Message(Chat::NPCQuestSay, "#Tune - Error hit chance out of bounds. [Max %.2f Min .2f]",  RuleR(Combat,MaxChancetoHit),RuleR(Combat,MinChancetoHit));
			return;
		}

		if(!strcasecmp(sep->arg[2], "A"))
			c->Tune_FindAccuaryByHitChance(defender, attacker, hit_chance, interval, max_loop,avoid_override,info_level);
		else if(!strcasecmp(sep->arg[2], "B"))
			c->Tune_FindAccuaryByHitChance(attacker, defender, hit_chance, interval, max_loop,avoid_override,info_level);
		else {
			c->Message(Chat::White, "#Tune - Error no category selcted. [#Tune help]");
			c->Message(Chat::White, "Usage #tune FindAcccuracy [A/B] [hit chance] [interval][loop_max][Avoidance Overwride][Info Level]");
			c->Message(Chat::White, "Exampled #tune FindAccuracy B 30");
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
			c->Message(Chat::White, "#Tune - Error must enter the desired hit chance on defender. Ie. Defender to have hit chance of 40 pct.");
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
			c->Message(Chat::NPCQuestSay, "#Tune - Error hit chance out of bounds. [Max %.2f Min .2f]",  RuleR(Combat,MaxChancetoHit),RuleR(Combat,MinChancetoHit));
			return;
		}

		if(!strcasecmp(sep->arg[2], "A"))
			c->Tune_FindAvoidanceByHitChance(defender, attacker, hit_chance, interval, max_loop,acc_override, info_level);
		else if(!strcasecmp(sep->arg[2], "B"))
			c->Tune_FindAvoidanceByHitChance(attacker, defender, hit_chance, interval, max_loop,acc_override, info_level);
		else {
			c->Message(Chat::White, "#Tune - Error no category selcted. [#Tune help]");
			c->Message(Chat::White, "Usage #tune FindAvoidance [A/B] [hit chance] [interval][loop_max][Accuracy Overwride][Info Level]");
			c->Message(Chat::White, "Exampled #tune FindAvoidance B 30");
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
			LogDebug("[[{}]] Test #2 Took [{}] seconds", i, ((float)(std::clock() - t)) / CLOCKS_PER_SEC);
		}

	}
}

void command_crashtest(Client *c, const Seperator *sep)
{
	c->Message(Chat::White, "Alright, now we get an GPF ;) ");
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
			c->Message(Chat::Red, "Successfully sent the packet to world to reload log settings from the database for all zones");
			safe_delete(pack);
		}
		/* #logs list_settings */
		if (strcasecmp(sep->arg[1], "list_settings") == 0 ||
			(strcasecmp(sep->arg[1], "set") == 0 && strcasecmp(sep->arg[3], "") == 0)) {
			c->Message(Chat::White, "[Category ID | console | file | gmsay | Category Description]");
			int redisplay_columns = 0;
			for (int i            = 0; i < Logs::LogCategory::MaxCategoryID; i++) {
				if (redisplay_columns == 10) {
					c->Message(Chat::White, "[Category ID | console | file | gmsay | Category Description]");
					redisplay_columns = 0;
				}
				c->Message(
					0,
					StringFormat(
						"--- %i | %u | %u | %u | %s",
						i,
						LogSys.log_settings[i].log_to_console,
						LogSys.log_settings[i].log_to_file,
						LogSys.log_settings[i].log_to_gmsay,
						Logs::LogCategoryName[i]
					).c_str());
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
				c->Message(Chat::White, "--- #logs set [console|file|gmsay] <category_id> <debug_level (1-3)> - Sets log settings during the lifetime of the zone");
				c->Message(Chat::White, "--- #logs set gmsay 20 1 - Would output Quest errors to gmsay");
			}
			if (logs_set == 1){
				c->Message(Chat::Yellow, "Your Log Settings have been applied");
				c->Message(Chat::Yellow, "Output Method: %s :: Debug Level: %i - Category: %s",  sep->arg[2], atoi(sep->arg[4]), Logs::LogCategoryName[atoi(sep->arg[3])]);
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
		c->Message(Chat::White, "#logs usage:");
		c->Message(Chat::White, "--- #logs reload_all - Reload all settings in world and all zone processes with what is defined in the database");
		c->Message(Chat::White, "--- #logs list_settings - Shows current log settings and categories loaded into the current process' memory");
		c->Message(Chat::White, "--- #logs set [console|file|gmsay] <category_id> <debug_level (1-3)> - Sets log settings during the lifetime of the zone");
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
			auto results = content_db.QueryDatabase(query);
		}
	}
	LogDebug("MySQL Test Took [{}] seconds", ((float)(std::clock() - t)) / CLOCKS_PER_SEC);
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
		c->Message(Chat::White, "Reset of timer %i for %s", timer_id, c->GetName());
		c->ResetAlternateAdvancementTimer(timer_id);
	}
	else if(!strcasecmp(sep->arg[1], "all"))
	{
		c->Message(Chat::White, "Reset all timers for %s", c->GetName());
		c->ResetAlternateAdvancementTimers();
	}
	else
	{
		c->Message(Chat::White, "usage: #resetaa_timer [all | timer_id]");
	}
}

void command_reloadaa(Client *c, const Seperator *sep) {
	c->Message(Chat::White, "Reloading Alternate Advancement Data...");
	zone->LoadAlternateAdvancement();
	c->Message(Chat::White, "Alternate Advancement Data Reloaded");
	entity_list.SendAlternateAdvancementStats();
}

inline bool file_exists(const std::string& name) {
	std::ifstream f(name.c_str());
	return f.good();
}

void command_hotfix(Client *c, const Seperator *sep)
{
	std::string hotfix;
	database.GetVariable("hotfix_name", hotfix);

	std::string hotfix_name;
	if (!strcasecmp(hotfix.c_str(), "hotfix_")) {
		hotfix_name = "";
	}
	else {
		hotfix_name = "hotfix_";
	}

	c->Message(Chat::White, "Creating and applying hotfix");
	std::thread t1(
		[c, hotfix_name]() {

			std::string shared_memory_path;

#ifdef WIN32
			shared_memory_path = "shared_memory";
			if (file_exists("bin/shared_memory.exe")) {
				shared_memory_path = "bin\\shared_memory.exe";
			}

			std::string hotfix_command;
			if (hotfix_name.length() > 0) {
				hotfix_command = fmt::format("\"{}\" -hotfix={}", shared_memory_path, hotfix_name);
			}
			else {
				hotfix_command = fmt::format("\"{}\"", shared_memory_path, hotfix_name);
			}

			LogInfo("Running hotfix command [{}]", hotfix_command);
			if (system(hotfix_command.c_str())) {}
#else
			shared_memory_path = "./shared_memory";
			if (file_exists("./bin/shared_memory")) {
				shared_memory_path = "./bin/shared_memory";
			}

			if (hotfix_name.length() > 0) {
				if (system(StringFormat("%s -hotfix=%s", shared_memory_path.c_str(), hotfix_name.c_str()).c_str())) {}
			}
			else {
				if (system(StringFormat("%s", shared_memory_path.c_str()).c_str())) {}
			}
#endif
			database.SetVariable("hotfix_name", hotfix_name);

			ServerPacket pack(ServerOP_ChangeSharedMem, hotfix_name.length() + 1);
			if (hotfix_name.length() > 0) {
				strcpy((char *) pack.pBuffer, hotfix_name.c_str());
			}
			worldserver.SendPacket(&pack);

			if (c) { c->Message(Chat::White, "Hotfix applied"); }
		}
	);

	t1.detach();
}

void command_load_shared_memory(Client *c, const Seperator *sep) {
	std::string hotfix;
	database.GetVariable("hotfix_name", hotfix);

	std::string hotfix_name;
	if(strcasecmp(hotfix.c_str(), sep->arg[1]) == 0) {
		c->Message(Chat::White, "Cannot attempt to load this shared memory segment as it is already loaded.");
		return;
	}

	hotfix_name = sep->arg[1];
	c->Message(Chat::White, "Loading shared memory segment %s", hotfix_name.c_str());
	std::thread t1([c,hotfix_name]() {
#ifdef WIN32
		if(hotfix_name.length() > 0) {
			if(system(StringFormat("shared_memory -hotfix=%s", hotfix_name.c_str()).c_str()));
		} else {
			if(system(StringFormat("shared_memory").c_str()));
		}
#else
		if(hotfix_name.length() > 0) {
			if(system(StringFormat("./shared_memory -hotfix=%s", hotfix_name.c_str()).c_str()));
		}
		else {
			if(system(StringFormat("./shared_memory").c_str()));
		}
#endif
		c->Message(Chat::White, "Shared memory segment finished loading.");
	});

	t1.detach();
}

void command_apply_shared_memory(Client *c, const Seperator *sep) {
	std::string hotfix;
	database.GetVariable("hotfix_name", hotfix);
	std::string hotfix_name = sep->arg[1];

	c->Message(Chat::White, "Applying shared memory segment %s", hotfix_name.c_str());
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
		c->Message(Chat::Red, "Successfully sent the packet to world to reload Perl Export settings");
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
	c->Message(Chat::Default, "Traps reloaded for %s.", zone->GetShortName());
}

void command_scale(Client *c, const Seperator *sep)
{
	if (sep->argnum == 0) {
		c->Message(Chat::Yellow, "# Usage # ");
		c->Message(Chat::Yellow, "#scale [static/dynamic] (With targeted NPC)");
		c->Message(Chat::Yellow, "#scale [npc_name_search] [static/dynamic] (To make zone-wide changes)");
		c->Message(Chat::Yellow, "#scale all [static/dynamic]");
		return;
	}

	/**
	 * Targeted changes
	 */
	if (c->GetTarget() && c->GetTarget()->IsNPC() && sep->argnum < 2) {
		NPC * npc = c->GetTarget()->CastToNPC();

		bool apply_status = false;
		if (strcasecmp(sep->arg[1], "dynamic") == 0) {
			c->Message(Chat::Yellow, "Applying global base scaling to npc dynamically (All stats set to zeroes)...");
			apply_status = npc_scale_manager->ApplyGlobalBaseScalingToNPCDynamically(npc);
		}
		else if (strcasecmp(sep->arg[1], "static") == 0) {
			c->Message(Chat::Yellow, "Applying global base scaling to npc statically (Copying base stats onto NPC)...");
			apply_status = npc_scale_manager->ApplyGlobalBaseScalingToNPCStatically(npc);
		}
		else {
			return;
		}

		if (apply_status) {
			c->Message(Chat::Yellow, "Applied to NPC '%s' successfully!", npc->GetName());
		}
		else {
			c->Message(Chat::Yellow, "Failed to load scaling data from the database "
						   "for this npc / type, see 'NPCScaling' log for more info");
		}
	}
	else if (c->GetTarget() && sep->argnum < 2) {
		c->Message(Chat::Yellow, "Target must be an npc!");
	}

	/**
	 * Zonewide
	 */
	if (sep->argnum > 1) {

		std::string scale_type;
		if (strcasecmp(sep->arg[2], "dynamic") == 0) {
			scale_type = "dynamic";
		}
		else if (strcasecmp(sep->arg[2], "static") == 0) {
			scale_type = "static";
		}

		if (scale_type.length() <= 0) {
			c->Message(Chat::Yellow, "You must first set if you intend on using static versus dynamic for these changes");
			c->Message(Chat::Yellow, "#scale [npc_name_search] [static/dynamic]");
			c->Message(Chat::Yellow, "#scale all [static/dynamic]");
			return;
		}

		std::string search_string = sep->arg[1];

		auto &entity_list_search = entity_list.GetNPCList();

		int found_count = 0;
		for (auto &itr : entity_list_search) {
			NPC *entity = itr.second;

			std::string entity_name = entity->GetName();

			/**
			 * Filter by name
			 */
			if (search_string.length() > 0 && entity_name.find(search_string) == std::string::npos && strcasecmp(sep->arg[1], "all") != 0) {
				continue;
			}

			std::string status = "(Searching)";

			if (strcasecmp(sep->arg[3], "apply") == 0) {
				status = "(Applying)";

				if (strcasecmp(sep->arg[2], "dynamic") == 0) {
					npc_scale_manager->ApplyGlobalBaseScalingToNPCDynamically(entity);
				}
				if (strcasecmp(sep->arg[2], "static") == 0) {
					npc_scale_manager->ApplyGlobalBaseScalingToNPCStatically(entity);
				}
			}

			c->Message(
				15,
				"| ID %5d | %s | x %.0f | y %0.f | z %.0f | DBID %u %s",
				entity->GetID(),
				entity->GetName(),
				entity->GetX(),
				entity->GetY(),
				entity->GetZ(),
				entity->GetNPCTypeID(),
				status.c_str()
			);

			found_count++;
		}

		if (strcasecmp(sep->arg[3], "apply") == 0) {
			c->Message(Chat::Yellow, "%s scaling applied against (%i) NPC's", sep->arg[2], found_count);
		}
		else {

			std::string saylink = StringFormat(
				"#scale %s %s apply",
				sep->arg[1],
				sep->arg[2]
			);

			c->Message(Chat::Yellow, "Found (%i) NPC's that match this search...", found_count);
			c->Message(
				Chat::Yellow, "To apply these changes, click <%s> or type %s",
				EQ::SayLinkEngine::GenerateQuestSaylink(saylink, false, "Apply").c_str(),
				saylink.c_str()
			);
		}
	}
}

void command_databuckets(Client *c, const Seperator *sep)
 {
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::Yellow, "Usage: #databuckets view (partial key)|(limit) OR #databuckets delete (key)");
		return;
	}
	if (strcasecmp(sep->arg[1], "view") == 0) {

		std::string key_filter;
		uint8 limit = 50;
		for (int i = 2; i < 4; i++) {
			if (sep->arg[i][0] == '\0')
				break;
			if (strcasecmp(sep->arg[i], "limit") == 0) {
				limit = (uint8)atoi(sep->arg[i + 1]);
				continue;
			}
		}
		if (sep->arg[2]) {
			key_filter = str_tolower(sep->arg[2]);
		}
		std::string query = "SELECT `id`, `key`, `value`, `expires` FROM data_buckets";
		if (!key_filter.empty())  query += StringFormat(" WHERE `key` LIKE '%%%s%%'", key_filter.c_str());
		query += StringFormat(" LIMIT %u", limit);
		auto results = database.QueryDatabase(query);
		if (!results.Success())
			return;
		if (results.RowCount() == 0) {
			c->Message(Chat::Yellow, "No data_buckets found");
			return;
		}
		int _ctr = 0;
		// put in window for easier readability in case want command line for something else
		std::string window_title = "Data Buckets";
		std::string window_text =
			"<table>"
			"<tr>"
			"<td>ID</td>"
			"<td>Expires</td>"
			"<td>Key</td>"
			"<td>Value</td>"
			"</tr>";
		for (auto row = results.begin(); row != results.end(); ++row) {
			auto        id = static_cast<uint32>(atoi(row[0]));
			std::string key = row[1];
			std::string value = row[2];
			std::string expires = row[3];
			window_text.append(StringFormat(
				"<tr>"
				"<td>%u</td>"
				"<td>%s</td>"
				"<td>%s</td>"
				"<td>%s</td>"
				"</tr>",
				id,
				expires.c_str(),
				key.c_str(),
				value.c_str()
			));
			_ctr++;
			std::string	del_saylink = StringFormat("#databuckets delete %s", key.c_str());
			c->Message(Chat::White, "%s : %s",
				EQ::SayLinkEngine::GenerateQuestSaylink(del_saylink, false, "Delete").c_str(), key.c_str(), "  Value:  ", value.c_str());
		}
		window_text.append("</table>");
		c->SendPopupToClient(window_title.c_str(), window_text.c_str());
		std::string response = _ctr > 0 ? StringFormat("Found %i matching data buckets", _ctr).c_str() : "No Databuckets found.";
		c->Message(Chat::Yellow, response.c_str());
	}
	else if (strcasecmp(sep->arg[1], "delete") == 0)
	{
		if (DataBucket::DeleteData(sep->argplus[2]))
			c->Message(Chat::Yellow, "data bucket %s deleted.", sep->argplus[2]);
		else
			c->Message(Chat::Red, "An error occurred deleting data bucket %s", sep->argplus[2]);
		return;
	}
}

void command_who(Client *c, const Seperator *sep)
{
	std::string query =
		SQL (
			SELECT
			  character_data.account_id,
			  character_data.name,
			  character_data.zone_id,
			  character_data.zone_instance,
			  COALESCE(
				(
				  select
					guilds.name
				  from
					guilds
				  where
					id = (
					  (
						select
						  guild_id
						from
						  guild_members
						where
						  char_id = character_data.id
					  )
					)
				),
				""
			  ) as guild_name,
			  character_data.level,
			  character_data.race,
			  character_data.class,
			  COALESCE(
				(
				  select
					account.status
				  from
					account
				  where
					account.id = character_data.account_id
				  LIMIT
					1
				), 0
			  ) as account_status,
			  COALESCE(
				(
				  select
					account.name
				  from
					account
				  where
					account.id = character_data.account_id
				  LIMIT
					1
				),
				0
			  ) as account_name,
			  COALESCE(
				(
				  select
					account_ip.ip
				  from
					account_ip
				  where
					account_ip.accid = character_data.account_id
				  ORDER BY
					account_ip.lastused DESC
				  LIMIT
					1
				),
				""
			  ) as account_ip
			FROM
			  character_data
			WHERE
			  last_login > (UNIX_TIMESTAMP() - 600)
			ORDER BY
			  character_data.name;
			)
		;

	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return;

	if (results.RowCount() == 0) {
		c->Message(Chat::Yellow, "No results found");
		return;
	}

	std::string search_string;

	if (sep->arg[1]) {
		search_string = str_tolower(sep->arg[1]);
	}

	int found_count = 0;

	c->Message(Chat::Magenta, "Players in EverQuest");
	c->Message(Chat::Magenta, "--------------------");

	for (auto row = results.begin(); row != results.end(); ++row) {
		auto        account_id          = static_cast<uint32>(atoi(row[0]));
		std::string player_name         = row[1];
		auto        zone_id             = static_cast<uint32>(atoi(row[2]));
		std::string zone_short_name     = ZoneName(zone_id);
		auto        zone_instance       = static_cast<uint32>(atoi(row[3]));
		std::string guild_name          = row[4];
		auto        player_level        = static_cast<uint32>(atoi(row[5]));
		auto        player_race         = static_cast<uint32>(atoi(row[6]));
		auto        player_class        = static_cast<uint32>(atoi(row[7]));
		auto        account_status      = static_cast<uint32>(atoi(row[8]));
		std::string account_name        = row[9];
		std::string account_ip          = row[10];
		std::string base_class_name     = GetClassIDName(static_cast<uint8>(player_class), 1);
		std::string displayed_race_name = GetRaceIDName(static_cast<uint16>(player_race));

		if (search_string.length() > 0) {
			bool found_search_term =
					 (
						 str_tolower(player_name).find(search_string) != std::string::npos ||
						 str_tolower(zone_short_name).find(search_string) != std::string::npos ||
						 str_tolower(displayed_race_name).find(search_string) != std::string::npos ||
						 str_tolower(base_class_name).find(search_string) != std::string::npos ||
						 str_tolower(guild_name).find(search_string) != std::string::npos ||
						 str_tolower(account_name).find(search_string) != std::string::npos ||
						 str_tolower(account_ip).find(search_string) != std::string::npos
					 );

			if (!found_search_term) {
				continue;
			}
		}

		std::string displayed_guild_name;
		if (guild_name.length() > 0) {
			displayed_guild_name = EQ::SayLinkEngine::GenerateQuestSaylink(
				StringFormat(
					"#who \"%s\"",
					guild_name.c_str()),
				false,
				StringFormat("<%s>", guild_name.c_str())
			);
		}

		std::string goto_saylink = EQ::SayLinkEngine::GenerateQuestSaylink(
			StringFormat("#goto %s", player_name.c_str()), false, "Goto"
		);

		std::string display_class_name = GetClassIDName(static_cast<uint8>(player_class), static_cast<uint8>(player_level));

		c->Message(
			5, "%s[%u %s] %s (%s) %s ZONE: %s (%u) (%s) (%s) (%s)",
			(account_status > 0 ? "* GM * " : ""),
			player_level,
			EQ::SayLinkEngine::GenerateQuestSaylink(StringFormat("#who %s", base_class_name.c_str()), false, display_class_name).c_str(),
			player_name.c_str(),
			EQ::SayLinkEngine::GenerateQuestSaylink(StringFormat("#who %s", displayed_race_name.c_str()), false, displayed_race_name).c_str(),
			displayed_guild_name.c_str(),
			EQ::SayLinkEngine::GenerateQuestSaylink(StringFormat("#who %s", zone_short_name.c_str()), false, zone_short_name).c_str(),
			zone_instance,
			goto_saylink.c_str(),
			EQ::SayLinkEngine::GenerateQuestSaylink(StringFormat("#who %s", account_name.c_str()), false, account_name).c_str(),
			EQ::SayLinkEngine::GenerateQuestSaylink(StringFormat("#who %s", account_ip.c_str()), false, account_ip).c_str()
		);

		found_count++;
	}

	std::string message = (
		found_count > 0 ?
			StringFormat("There is %i player(s) in EverQuest", found_count).c_str() :
			"There are no players in EverQuest that match those who filters."
	);

	c->Message(Chat::Magenta, message.c_str());
}

void command_network(Client *c, const Seperator *sep)
{
	if (!strcasecmp(sep->arg[1], "getopt"))
	{
		auto eqsi = c->Connection();
		auto manager = eqsi->GetManager();
		auto opts = manager->GetOptions();

		if (!strcasecmp(sep->arg[2], "all"))
		{
			c->Message(Chat::White, "max_packet_size: %llu", (uint64_t)opts.daybreak_options.max_packet_size);
			c->Message(Chat::White, "max_connection_count: %llu", (uint64_t)opts.daybreak_options.max_connection_count);
			c->Message(Chat::White, "keepalive_delay_ms: %llu", (uint64_t)opts.daybreak_options.keepalive_delay_ms);
			c->Message(Chat::White, "resend_delay_factor: %.2f", opts.daybreak_options.resend_delay_factor);
			c->Message(Chat::White, "resend_delay_ms: %llu", (uint64_t)opts.daybreak_options.resend_delay_ms);
			c->Message(Chat::White, "resend_delay_min: %llu", (uint64_t)opts.daybreak_options.resend_delay_min);
			c->Message(Chat::White, "resend_delay_max: %llu", (uint64_t)opts.daybreak_options.resend_delay_max);
			c->Message(Chat::White, "connect_delay_ms: %llu", (uint64_t)opts.daybreak_options.connect_delay_ms);
			c->Message(Chat::White, "connect_stale_ms: %llu", (uint64_t)opts.daybreak_options.connect_stale_ms);
			c->Message(Chat::White, "stale_connection_ms: %llu", (uint64_t)opts.daybreak_options.stale_connection_ms);
			c->Message(Chat::White, "crc_length: %llu", (uint64_t)opts.daybreak_options.crc_length);
			c->Message(Chat::White, "hold_size: %llu", (uint64_t)opts.daybreak_options.hold_size);
			c->Message(Chat::White, "hold_length_ms: %llu", (uint64_t)opts.daybreak_options.hold_length_ms);
			c->Message(Chat::White, "simulated_in_packet_loss: %llu", (uint64_t)opts.daybreak_options.simulated_in_packet_loss);
			c->Message(Chat::White, "simulated_out_packet_loss: %llu", (uint64_t)opts.daybreak_options.simulated_out_packet_loss);
			c->Message(Chat::White, "tic_rate_hertz: %.2f", opts.daybreak_options.tic_rate_hertz);
			c->Message(Chat::White, "resend_timeout: %llu", (uint64_t)opts.daybreak_options.resend_timeout);
			c->Message(Chat::White, "connection_close_time: %llu", (uint64_t)opts.daybreak_options.connection_close_time);
			c->Message(Chat::White, "encode_passes[0]: %llu", (uint64_t)opts.daybreak_options.encode_passes[0]);
			c->Message(Chat::White, "encode_passes[1]: %llu", (uint64_t)opts.daybreak_options.encode_passes[1]);
			c->Message(Chat::White, "port: %llu", (uint64_t)opts.daybreak_options.port);
		}
		else {
			c->Message(Chat::White, "Unknown get option: %s", sep->arg[2]);
			c->Message(Chat::White, "Available options:");
			//Todo the rest of these when im less lazy.
			//c->Message(Chat::White, "max_packet_size");
			//c->Message(Chat::White, "max_connection_count");
			//c->Message(Chat::White, "keepalive_delay_ms");
			//c->Message(Chat::White, "resend_delay_factor");
			//c->Message(Chat::White, "resend_delay_ms");
			//c->Message(Chat::White, "resend_delay_min");
			//c->Message(Chat::White, "resend_delay_max");
			//c->Message(Chat::White, "connect_delay_ms");
			//c->Message(Chat::White, "connect_stale_ms");
			//c->Message(Chat::White, "stale_connection_ms");
			//c->Message(Chat::White, "crc_length");
			//c->Message(Chat::White, "hold_size");
			//c->Message(Chat::White, "hold_length_ms");
			//c->Message(Chat::White, "simulated_in_packet_loss");
			//c->Message(Chat::White, "simulated_out_packet_loss");
			//c->Message(Chat::White, "tic_rate_hertz");
			//c->Message(Chat::White, "resend_timeout");
			//c->Message(Chat::White, "connection_close_time");
			//c->Message(Chat::White, "encode_passes[0]");
			//c->Message(Chat::White, "encode_passes[1]");
			//c->Message(Chat::White, "port");
			c->Message(Chat::White, "all");
		}
	}
	else if (!strcasecmp(sep->arg[1], "setopt"))
	{
		auto eqsi = c->Connection();
		auto manager = eqsi->GetManager();
		auto opts = manager->GetOptions();

		if (!strcasecmp(sep->arg[3], ""))
		{
			c->Message(Chat::White, "Missing value for set");
			return;
		}

		std::string value = sep->arg[3];
		if (!strcasecmp(sep->arg[2], "max_connection_count"))
		{
			opts.daybreak_options.max_connection_count = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "keepalive_delay_ms"))
		{
			opts.daybreak_options.keepalive_delay_ms = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "resend_delay_factor"))
		{
			opts.daybreak_options.resend_delay_factor = std::stod(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "resend_delay_ms"))
		{
			opts.daybreak_options.resend_delay_ms = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "resend_delay_min"))
		{
			opts.daybreak_options.resend_delay_min = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "resend_delay_max"))
		{
			opts.daybreak_options.resend_delay_max = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "connect_delay_ms"))
		{
			opts.daybreak_options.connect_delay_ms = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "connect_stale_ms"))
		{
			opts.daybreak_options.connect_stale_ms = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "stale_connection_ms"))
		{
			opts.daybreak_options.stale_connection_ms = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "hold_size"))
		{
			opts.daybreak_options.hold_size = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "hold_length_ms"))
		{
			opts.daybreak_options.hold_length_ms = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "simulated_in_packet_loss"))
		{
			opts.daybreak_options.simulated_in_packet_loss = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "simulated_out_packet_loss"))
		{
			opts.daybreak_options.simulated_out_packet_loss = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "resend_timeout"))
		{
			opts.daybreak_options.resend_timeout = std::stoull(value);
			manager->SetOptions(opts);
		}
		else if (!strcasecmp(sep->arg[2], "connection_close_time"))
		{
			opts.daybreak_options.connection_close_time = std::stoull(value);
			manager->SetOptions(opts);
		}
		else {
			c->Message(Chat::White, "Unknown set option: %s", sep->arg[2]);
			c->Message(Chat::White, "Available options:");
			c->Message(Chat::White, "max_connection_count");
			c->Message(Chat::White, "keepalive_delay_ms");
			c->Message(Chat::White, "resend_delay_factor");
			c->Message(Chat::White, "resend_delay_ms");
			c->Message(Chat::White, "resend_delay_min");
			c->Message(Chat::White, "resend_delay_max");
			c->Message(Chat::White, "connect_delay_ms");
			c->Message(Chat::White, "connect_stale_ms");
			c->Message(Chat::White, "stale_connection_ms");
			c->Message(Chat::White, "hold_size");
			c->Message(Chat::White, "hold_length_ms");
			c->Message(Chat::White, "simulated_in_packet_loss");
			c->Message(Chat::White, "simulated_out_packet_loss");
			c->Message(Chat::White, "resend_timeout");
			c->Message(Chat::White, "connection_close_time");
		}
	}
	else {
		c->Message(Chat::White, "Unknown command: %s", sep->arg[1]);
		c->Message(Chat::White, "Network commands avail:");
		c->Message(Chat::White, "getopt optname - Retrieve the current option value set.");
		c->Message(Chat::White, "setopt optname - Set the current option allowed.");
	}
}

void command_viewzoneloot(Client *c, const Seperator *sep)
{
	std::map<uint32,ItemList> zone_loot_list;
	auto npc_list = entity_list.GetNPCList();
	uint32 loot_amount = 0, loot_id = 1, search_item_id = 0;
	if (sep->argnum == 1 && sep->IsNumber(1)) {
		search_item_id = atoi(sep->arg[1]);
	} else if (sep->argnum == 1 && !sep->IsNumber(1)) {
		c->Message(
			Chat::Yellow,
			"Usage: #viewzoneloot [item id]"
		);
		return;
	}
	for (auto npc_entity : npc_list) {
		auto current_npc_item_list = npc_entity.second->GetItemList();
		zone_loot_list.insert({ npc_entity.second->GetID(), current_npc_item_list });
	}
	for (auto loot_item : zone_loot_list) {
		uint32 current_entity_id = loot_item.first;
		auto current_item_list = loot_item.second;
		auto current_npc = entity_list.GetNPCByID(current_entity_id);
		std::string npc_link;
		if (current_npc) {
			std::string npc_name = current_npc->GetCleanName();
			uint32 instance_id = zone->GetInstanceID();
			uint32 zone_id = zone->GetZoneID();
			std::string command_link = EQ::SayLinkEngine::GenerateQuestSaylink(
				fmt::format(
					"#{} {} {} {} {}",
					(instance_id != 0 ? "zoneinstance" : "zone"),
					(instance_id != 0 ? instance_id : zone_id),
					current_npc->GetX(),
					current_npc->GetY(),
					current_npc->GetZ()
				),
				false,
				"Goto"
			);
			npc_link = fmt::format(
				" NPC: {} (ID {}) [{}]",
				npc_name,
				current_entity_id,
				command_link
			);
		}

		for (auto current_item : current_item_list) {
			if (search_item_id == 0 || current_item->item_id == search_item_id) {
				EQ::SayLinkEngine linker;
				linker.SetLinkType(EQ::saylink::SayLinkLootItem);
				linker.SetLootData(current_item);
				c->Message(
					Chat::White,
					fmt::format(
						"{}. {} ({}){}",
						loot_id,
						linker.GenerateLink(),
						current_item->item_id,
						npc_link
					).c_str()
				);
				loot_id++;
				loot_amount++;
			}
		}
	}


	if (search_item_id != 0) {
		std::string drop_string = (
			loot_amount > 0 ?
			fmt::format(
				"dropping in {} {}",
				loot_amount,
				(loot_amount > 1 ? "places" : "place")
			) :
			"not dropping"
		);
		c->Message(
			Chat::White,
			fmt::format(
				"{} ({}) is {}.",
				database.CreateItemLink(search_item_id),
				search_item_id,
				drop_string
			).c_str()
		);
	} else {
		std::string drop_string = (
			loot_amount > 0 ?
			fmt::format(
				"{} {} {}",
				(loot_amount > 1 ? "items" : "item"),
				(loot_amount > 1 ? "are" : "is"),
				(loot_amount > 1 ? "dropping" : "not dropping")
			) :
			"items are dropping"
		);
		c->Message(
			Chat::White,
			fmt::format(
				"{} {}.",
				loot_amount,
				drop_string
			).c_str()
		);
	}
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
		if (parse->PlayerHasQuestSub(EVENT_BOT_COMMAND)) {
			int i = parse->EventPlayer(EVENT_BOT_COMMAND, c, bot_message, 0);
			if (i == 0 && !RuleB(Chat, SuppressCommandErrors)) {
				c->Message(Chat::Red, "Bot command '%s' not recognized.", bot_message.c_str());
			}
		}
		else {
			if (!RuleB(Chat, SuppressCommandErrors))
				c->Message(Chat::Red, "Bot command '%s' not recognized.", bot_message.c_str());
		}
	}
}
#endif
