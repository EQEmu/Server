
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
#include "../common/ptimer.h"
#include "../common/rulesys.h"
#include "../common/string_util.h"
#include "../common/say_link.h"
#include "../common/net/eqstream.h"
#include "../common/file_util.h"
#include "../common/repositories/dynamic_zones_repository.h"

#include "data_bucket.h"
#include "command.h"
#include "dynamic_zone.h"
#include "expedition.h"
#include "guild_mgr.h"
#include "qglobals.h"
#include "queryserv.h"
#include "quest_parser_collection.h"
#include "titles.h"
#include "water_map.h"
#include "worldserver.h"
#include "fastmath.h"
#include "mob_movement_manager.h"
#include "npc_scale_manager.h"

extern QueryServ* QServ;
extern WorldServer worldserver;
extern TaskManager *task_manager;
extern FastMath g_Math;
void CatchSignal(int sig_num);


int commandcount;					// how many commands we have

// this is the pointer to the dispatch function, updated once
// init has been performed to point at the real function
int (*command_dispatch)(Client *,char const *)=command_notavail;

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
		command_add("acceptrules", "[acceptrules] - Accept the EQEmu Agreement", AccountStatus::Player, command_acceptrules) ||
		command_add("advnpcspawn", "[maketype|makegroup|addgroupentry|addgroupspawn][removegroupspawn|movespawn|editgroupbox|cleargroupbox]", AccountStatus::GMLeadAdmin, command_advnpcspawn) ||
		command_add("aggro", "[Distance] [-v] - Display aggro information for all mobs 'Distance' distance from your target. (-v is verbose Faction Information)", AccountStatus::QuestTroupe, command_aggro) ||
		command_add("aggrozone", "[aggro] - Aggro every mob in the zone with X aggro. Default is 0. Not recommend if you're not invulnerable.", AccountStatus::GMAdmin, command_aggrozone) ||
		command_add("ai", "[factionid/spellslist/con/guard/roambox/stop/start] - Modify AI on NPC target", AccountStatus::GMAdmin, command_ai) ||
		command_add("appearance", "[type] [value] - Send an appearance packet for you or your target", AccountStatus::GMLeadAdmin, command_appearance) ||
		command_add("apply_shared_memory", "[shared_memory_name] - Tells every zone and world to apply a specific shared memory segment by name.", AccountStatus::GMImpossible, command_apply_shared_memory) ||
		command_add("attack", "[targetname] - Make your NPC target attack targetname", AccountStatus::GMLeadAdmin, command_attack) ||
		command_add("augmentitem",  "Force augments an item. Must have the augment item window open.", AccountStatus::GMImpossible, command_augmentitem) ||
		command_add("ban", "[name] [reason]- Ban by character name", AccountStatus::GMLeadAdmin, command_ban) ||
		command_add("beard", "- Change the beard of your target", AccountStatus::QuestTroupe, command_beard) ||
		command_add("beardcolor", "- Change the beard color of your target", AccountStatus::QuestTroupe, command_beardcolor) ||
		command_add("bestz", "- Ask map for a good Z coord for your x,y coords.", AccountStatus::Player, command_bestz) ||
		command_add("bind", "- Sets your targets bind spot to their current location", AccountStatus::GMMgmt, command_bind) ||

#ifdef BOTS
		command_add("bot", "- Type \"#bot help\" or \"^help\" to the see the list of available commands for bots.", AccountStatus::Player, command_bot) ||
#endif

		command_add("camerashake",  "[Duration (Milliseconds)] [Intensity (1-10)] - Shakes the camera on everyone's screen globally.", AccountStatus::QuestTroupe, command_camerashake) ||
		command_add("castspell", "[Spell ID] [Instant (0 = False, 1 = True, Default is 1 if Unused)] - Cast a spell", AccountStatus::Guide, command_castspell) ||
		command_add("chat", "[channel num] [message] - Send a channel message to all zones", AccountStatus::GMMgmt, command_chat) ||
		command_add("checklos", "- Check for line of sight to your target", AccountStatus::Guide, command_checklos) ||
		command_add("copycharacter", "[source_char_name] [dest_char_name] [dest_account_name] Copies character to destination account", AccountStatus::GMImpossible, command_copycharacter) ||
		command_add("corpse", "- Manipulate corpses, use with no arguments for help", AccountStatus::Guide, command_corpse) ||
		command_add("corpsefix", "Attempts to bring corpses from underneath the ground within close proximity of the player", AccountStatus::Player, command_corpsefix) ||
		command_add("countitem", "[Item ID] - Counts the specified Item ID in your or your target's inventory", AccountStatus::GMLeadAdmin, command_countitem) ||
		command_add("cvs", "- Summary of client versions currently online.", AccountStatus::GMMgmt, command_cvs) ||
		command_add("damage", "[Amount] - Damage yourself or your target", AccountStatus::GMAdmin, command_damage) ||
		command_add("databuckets", "View|Delete [key] [limit]- View data buckets, limit 50 default or Delete databucket by key", AccountStatus::QuestTroupe, command_databuckets) ||
		command_add("date", "[yyyy] [mm] [dd] [HH] [MM] - Set EQ time", AccountStatus::EQSupport, command_date) ||
		command_add("dbspawn2", "[spawngroup] [respawn] [variance] - Spawn an NPC from a predefined row in the spawn2 table", AccountStatus::GMAdmin, command_dbspawn2) ||
		command_add("delacct", "[accountname] - Delete an account", AccountStatus::GMLeadAdmin, command_delacct) ||
		command_add("deletegraveyard", "[zone name] - Deletes the graveyard for the specified zone.", AccountStatus::GMMgmt, command_deletegraveyard) ||
		command_add("delpetition", "[petition number] - Delete a petition", AccountStatus::ApprenticeGuide, command_delpetition) ||
		command_add("depop", "- Depop your NPC target", AccountStatus::Guide, command_depop) ||
		command_add("depopzone", "- Depop the zone", AccountStatus::GMAdmin, command_depopzone) ||
		command_add("devtools", "- Manages devtools", AccountStatus::GMMgmt, command_devtools) ||
		command_add("details", "- Change the details of your target (Drakkin Only)", AccountStatus::QuestTroupe, command_details) ||
		command_add("disablerecipe",  "[Recipe ID] - Disables a Recipe", AccountStatus::QuestTroupe, command_disablerecipe) ||
		command_add("disarmtrap",  "Analog for ldon disarm trap for the newer clients since we still don't have it working.", AccountStatus::QuestTroupe, command_disarmtrap) ||
		command_add("distance", "- Reports the distance between you and your target.", AccountStatus::QuestTroupe, command_distance) ||
		command_add("door", "Door editing command", AccountStatus::QuestTroupe, command_door) ||
		command_add("doanim", "[animnum] [type] - Send an EmoteAnim for you or your target", AccountStatus::Guide, command_doanim) ||
		command_add("dye", "[slot|'help'] [red] [green] [blue] [use_tint] - Dyes the specified armor slot to Red, Green, and Blue provided, allows you to bypass darkness limits.", AccountStatus::ApprenticeGuide, command_dye) ||
		command_add("dz", "Manage expeditions and dynamic zone instances", AccountStatus::QuestTroupe, command_dz) ||
		command_add("dzkickplayers", "Removes all players from current expedition. (/kickplayers alternative for pre-RoF clients)", AccountStatus::Player, command_dzkickplayers) ||
		command_add("editmassrespawn", "[name-search] [second-value] - Mass (Zone wide) NPC respawn timer editing command", AccountStatus::GMAdmin, command_editmassrespawn) ||
		command_add("emote", "['name'/'world'/'zone'] [type] [message] - Send an emote message", AccountStatus::QuestTroupe, command_emote) ||
		command_add("emotesearch", "Searches NPC Emotes", AccountStatus::QuestTroupe, command_emotesearch) ||
		command_add("emoteview", "Lists all NPC Emotes", AccountStatus::QuestTroupe, command_emoteview) ||
		command_add("emptyinventory", "- Clears your or your target's entire inventory (Equipment, General, Bank, and Shared Bank)", AccountStatus::GMImpossible, command_emptyinventory) ||
		command_add("enablerecipe",  "[Recipe ID] - Enables a Recipe", AccountStatus::QuestTroupe, command_enablerecipe) ||
		command_add("endurance", "Restores your or your target's endurance.", AccountStatus::Guide, command_endurance) ||
		command_add("equipitem", "[slotid(0-21)] - Equip the item on your cursor into the specified slot", AccountStatus::Guide, command_equipitem) ||
		command_add("face", "- Change the face of your target", AccountStatus::QuestTroupe, command_face) ||
		command_add("faction", "[Find (criteria | all ) | Review (criteria | all) | Reset (id)] - Resets Player's Faction", AccountStatus::QuestTroupe, command_faction) ||
		command_add("findaliases", "[search criteria]- Searches for available command aliases, by alias or command", AccountStatus::Player, command_findaliases) ||
		command_add("findclass", "[search criteria] - Search for a class", AccountStatus::Guide, command_findclass) ||
		command_add("findfaction", "[search criteria] - Search for a faction", AccountStatus::Guide, command_findfaction) ||
		command_add("findnpctype", "[search criteria] - Search database NPC types", AccountStatus::GMAdmin, command_findnpctype) ||
		command_add("findrace", "[search criteria] - Search for a race", AccountStatus::Guide, command_findrace) ||
		command_add("findskill", "[search criteria] - Search for a skill", AccountStatus::Guide, command_findskill) ||
		command_add("findspell", "[search criteria] - Search for a spell", AccountStatus::Guide, command_findspell) ||
		command_add("findtask", "[search criteria] - Search for a task", AccountStatus::Guide, command_findtask) ||
		command_add("findzone", "[search criteria] - Search database zones", AccountStatus::GMAdmin, command_findzone) ||
		command_add("fixmob", "[race|gender|texture|helm|face|hair|haircolor|beard|beardcolor|heritage|tattoo|detail] [next|prev] - Manipulate appearance of your target", AccountStatus::QuestTroupe, command_fixmob) ||
		command_add("flag", "[status] [acctname] - Refresh your admin status, or set an account's admin status if arguments provided", AccountStatus::Player, command_flag) ||
		command_add("flagedit", "- Edit zone flags on your target", AccountStatus::GMAdmin, command_flagedit) ||
		command_add("flags", "- displays the flags of you or your target", AccountStatus::Player, command_flags) ||
		command_add("flymode", "[0/1/2/3/4/5] - Set your or your player target's flymode to ground/flying/levitate/water/floating/levitate_running", AccountStatus::Guide, command_flymode) ||
		command_add("fov", "- Check wether you're behind or in your target's field of view", AccountStatus::QuestTroupe, command_fov) ||
		command_add("freeze", "- Freeze your target", AccountStatus::QuestTroupe, command_freeze) ||
		command_add("gassign", "[id] - Assign targetted NPC to predefined wandering grid id", AccountStatus::GMAdmin, command_gassign) ||
		command_add("gearup", "Developer tool to quickly equip a character", AccountStatus::GMMgmt, command_gearup) ||
		command_add("gender", "[0/1/2] - Change your or your target's gender to male/female/neuter", AccountStatus::Guide, command_gender) ||
		command_add("getplayerburiedcorpsecount", "- Get your or your target's total number of buried player corpses.", AccountStatus::GMAdmin, command_getplayerburiedcorpsecount) ||
		command_add("getvariable", "[varname] - Get the value of a variable from the database", AccountStatus::GMMgmt, command_getvariable) ||
		command_add("ginfo", "- get group info on target.", AccountStatus::ApprenticeGuide, command_ginfo) ||
		command_add("giveitem", "[itemid] [charges] - Summon an item onto your target's cursor. Charges are optional.", AccountStatus::GMMgmt, command_giveitem) ||
		command_add("givemoney", "[Platinum] [Gold] [Silver] [Copper] - Gives specified amount of money to you or your player target", AccountStatus::GMMgmt, command_givemoney) ||
		command_add("globalview", "Lists all qglobals in cache if you were to do a quest with this target.", AccountStatus::QuestTroupe, command_globalview) ||
		command_add("gm", "[On|Off] - Modify your or your target's GM Flag", AccountStatus::QuestTroupe, command_gm) ||
		command_add("gmspeed", "[On|Off] - Turn GM Speed On or Off for you or your player target", AccountStatus::GMAdmin, command_gmspeed) ||
		command_add("gmzone", "[Zone ID|Zone Short Name] [Version] [Instance Identifier] - Zones to a private GM instance (Version defaults to 0 and Instance Identifier defaults to 'gmzone' if not used)", AccountStatus::GMAdmin, command_gmzone) ||
		command_add("goto", "[playername] or [x y z] [h] - Teleport to the provided coordinates or to your target", AccountStatus::Steward, command_goto) ||
		command_add("grid", "[add/delete] [grid_num] [wandertype] [pausetype] - Create/delete a wandering grid", AccountStatus::GMAreas, command_grid) ||
		command_add("guild", "- Guild manipulation commands. Use argument help for more info.", AccountStatus::Steward, command_guild) ||
		command_add("guildapprove", "[guildapproveid] - Approve a guild with specified ID (guild creator receives the id)", AccountStatus::Player, command_guildapprove) ||
		command_add("guildcreate", "[guildname] - Creates an approval setup for guild name specified", AccountStatus::Player, command_guildcreate) ||
		command_add("guildlist", "[guildapproveid] - Lists character names who have approved the guild specified by the approve id", AccountStatus::Player, command_guildlist) ||
		command_add("hair", "- Change the hair style of your target", AccountStatus::QuestTroupe, command_hair) ||
		command_add("haircolor", "- Change the hair color of your target", AccountStatus::QuestTroupe, command_haircolor) ||
		command_add("haste", "[percentage] - Set your haste percentage", AccountStatus::GMAdmin, command_haste) ||
		command_add("hatelist", " - Display hate list for target.", AccountStatus::QuestTroupe, command_hatelist) ||
		command_add("heal", "- Completely heal your target", AccountStatus::Steward, command_heal) ||
		command_add("helm", "- Change the helm of your target", AccountStatus::QuestTroupe, command_helm) ||
		command_add("help", "[search term] - List available commands and their description, specify partial command as argument to search", AccountStatus::Player, command_help) ||
		command_add("heritage", "- Change the heritage of your target (Drakkin Only)", AccountStatus::QuestTroupe, command_heritage) ||
		command_add("heromodel",  "[hero model] [slot] - Full set of Hero's Forge Armor appearance. If slot is set, sends exact model just to slot.", AccountStatus::GMMgmt, command_heromodel) ||
		command_add("hideme", "[on/off] - Hide yourself from spawn lists.", AccountStatus::QuestTroupe, command_hideme) ||
		command_add("hotfix", "[hotfix_name] - Reloads shared memory into a hotfix, equiv to load_shared_memory followed by apply_shared_memory", AccountStatus::GMImpossible, command_hotfix) ||
		command_add("hp", "- Refresh your HP bar from the server.", AccountStatus::Player, command_hp) ||
		command_add("incstat", "- Increases or Decreases a client's stats permanently.", AccountStatus::GMMgmt, command_incstat) ||
		command_add("instance", "- Modify Instances", AccountStatus::GMMgmt, command_instance) ||
		command_add("interrogateinv", "- use [help] argument for available options", AccountStatus::Player, command_interrogateinv) ||
		command_add("interrupt", "[message id] [color] - Interrupt your casting. Arguments are optional.", AccountStatus::Guide, command_interrupt) ||
		command_add("invsnapshot", "- Manipulates inventory snapshots for your current target", AccountStatus::QuestTroupe, command_invsnapshot) ||
		command_add("invul", "[On|Off]] - Turn player target's or your invulnerable flag on or off", AccountStatus::QuestTroupe, command_invul) ||
		command_add("ipban", "[IP address] - Ban IP by character name", AccountStatus::GMMgmt, command_ipban) ||
		command_add("iplookup", "[charname] - Look up IP address of charname", AccountStatus::GMMgmt, command_iplookup) ||
		command_add("iteminfo", "- Get information about the item on your cursor", AccountStatus::Steward, command_iteminfo) ||
		command_add("itemsearch", "[search criteria] - Search for an item", AccountStatus::Steward, command_itemsearch) ||
		command_add("kick", "[charname] - Disconnect charname", AccountStatus::GMLeadAdmin, command_kick) ||
		command_add("kill", "- Kill your target", AccountStatus::GMAdmin, command_kill) ||
		command_add("killallnpcs", " [npc_name] Kills all npcs by search name, leave blank for all attackable NPC's", AccountStatus::GMMgmt, command_killallnpcs) ||
		command_add("lastname", "[Last Name] - Set you or your player target's lastname", AccountStatus::Guide, command_lastname) ||
		command_add("level", "[level] - Set your or your target's level", AccountStatus::Steward, command_level) ||
		command_add("list", "[npcs|players|corpses|doors|objects] [search] - Search entities", AccountStatus::ApprenticeGuide, command_list) ||
		command_add("listpetition", "- List petitions", AccountStatus::Guide, command_listpetition) ||
		command_add("load_shared_memory", "[shared_memory_name] - Reloads shared memory and uses the input as output", AccountStatus::GMImpossible, command_load_shared_memory) ||
		command_add("loc", "- Print out your or your target's current location and heading", AccountStatus::Player, command_loc) ||
		command_add("lock", "- Lock the worldserver", AccountStatus::GMLeadAdmin, command_lock) ||
		command_add("logs",  "Manage anything to do with logs", AccountStatus::GMImpossible, command_logs) ||
		command_add("makepet", "[level] [class] [race] [texture] - Make a pet", AccountStatus::Guide, command_makepet) ||
		command_add("mana", "- Fill your or your target's mana", AccountStatus::Guide, command_mana) ||
		command_add("maxskills", "Maxes skills for you.", AccountStatus::GMMgmt, command_max_all_skills) ||
		command_add("memspell", "[Slot] [Spell ID] - Memorize a Spell by ID in the specified Slot", AccountStatus::Guide, command_memspell) ||
		command_add("merchant_close_shop",  "Closes a merchant shop", AccountStatus::GMAdmin, command_merchantcloseshop) ||
		command_add("merchant_open_shop",  "Opens a merchants shop", AccountStatus::GMAdmin, command_merchantopenshop) ||
		command_add("modifynpcstat", "- Modifys a NPC's stats", AccountStatus::GMLeadAdmin, command_modifynpcstat) ||
		command_add("motd", "[new motd] - Set message of the day", AccountStatus::GMLeadAdmin, command_motd) ||
		command_add("movechar", "[Character ID|Character Name] [Zone ID|Zone Short Name] - Move an offline character to the specified zone", AccountStatus::Guide, command_movechar) ||
		command_add("movement", "Various movement commands", AccountStatus::GMMgmt, command_movement) ||
		command_add("myskills", "- Show details about your current skill levels", AccountStatus::Player, command_myskills) ||
		command_add("mysql", "[Help|Query] [SQL Query] - Mysql CLI, see 'Help' for options.", AccountStatus::GMImpossible, command_mysql) ||
		command_add("mystats", "- Show details about you or your pet", AccountStatus::Guide, command_mystats) ||
		command_add("name", "[newname] - Rename your player target", AccountStatus::GMLeadAdmin, command_name) ||
		command_add("netstats", "- Gets the network stats for a stream.", AccountStatus::GMMgmt, command_netstats) ||
		command_add("network", "- Admin commands for the udp network interface.", AccountStatus::GMImpossible, command_network) ||
		command_add("npccast", "[targetname/entityid] [spellid] - Causes NPC target to cast spellid on targetname/entityid", AccountStatus::QuestTroupe, command_npccast) ||
		command_add("npcedit", "[column] [value] - Mega NPC editing command", AccountStatus::GMAdmin, command_npcedit) ||
		command_add("npceditmass", "[name-search] [column] [value] - Mass (Zone wide) NPC data editing command", AccountStatus::GMAdmin, command_npceditmass) ||
		command_add("npcemote", "[message] - Make your NPC target emote a message.", AccountStatus::GMLeadAdmin, command_npcemote) ||
		command_add("npcloot", "[show/money/add/remove] [itemid/all/money: pp gp sp cp] - Manipulate the loot an NPC is carrying", AccountStatus::QuestTroupe, command_npcloot) ||
		command_add("npcsay", "[message] - Make your NPC target say a message.", AccountStatus::GMLeadAdmin, command_npcsay) ||
		command_add("npcshout", "[message] - Make your NPC target shout a message.", AccountStatus::GMLeadAdmin, command_npcshout) ||
		command_add("npcspawn", "[create/add/update/remove/delete] - Manipulate spawn DB", AccountStatus::GMAreas, command_npcspawn) ||
		command_add("npcspecialattk", "[flagchar] [perm] - Set NPC special attack flags. Flags are E(nrage) F(lurry) R(ampage) S(ummon).", AccountStatus::QuestTroupe, command_npcspecialattk) ||
		command_add("npcstats", "- Show stats about target NPC", AccountStatus::QuestTroupe, command_npcstats) ||
		command_add("npctype_cache",  "[id] or all - Clears the npc type cache for either the id or all npcs.", AccountStatus::GMImpossible, command_npctype_cache) ||
		command_add("npctypespawn", "[npctypeid] [factionid] - Spawn an NPC from the db", AccountStatus::Steward, command_npctypespawn) ||
		command_add("nudge", "- Nudge your target's current position by specific values", AccountStatus::QuestTroupe, command_nudge) ||
		command_add("nukebuffs", "[Beneficial|Detrimental|Help] - Strip all buffs by type on you or your target (no argument to remove all buffs)", AccountStatus::Guide, command_nukebuffs) ||
		command_add("nukeitem", "[Item ID] - Removes the specified Item ID from you or your player target's inventory", AccountStatus::GMLeadAdmin, command_nukeitem) ||
		command_add("object", "List|Add|Edit|Move|Rotate|Copy|Save|Undo|Delete - Manipulate static and tradeskill objects within the zone", AccountStatus::GMAdmin, command_object) ||
		command_add("oocmute", "[1/0] - Mutes OOC chat", AccountStatus::GMMgmt, command_oocmute) ||
		command_add("opcode", "- opcode management", AccountStatus::GMImpossible, command_opcode) ||
		command_add("path", "- view and edit pathing", AccountStatus::GMMgmt, command_path) ||
		command_add("peekinv", "[equip/gen/cursor/poss/limbo/curlim/trib/bank/shbank/allbank/trade/world/all] - Print out contents of your player target's inventory", AccountStatus::GMAdmin, command_peekinv) ||
		command_add("peqzone", "[Zone ID|Zone Short Name] - Teleports you to the specified zone if you meet the requirements.", AccountStatus::Player, command_peqzone) ||
		command_add("permaclass", "[Class ID] - Change your or your player target's class, changed client is disconnected", AccountStatus::QuestTroupe, command_permaclass) ||
		command_add("permagender", "[Gender ID] - Change your or your player target's gender", AccountStatus::QuestTroupe, command_permagender) ||
		command_add("permarace", "[Race ID] - Change your or your player target's race", AccountStatus::QuestTroupe, command_permarace) ||
		command_add("petitems", "- View your pet's items if you have one", AccountStatus::ApprenticeGuide, command_petitems) ||
		command_add("petitioninfo", "[petition number] - Get info about a petition", AccountStatus::ApprenticeGuide, command_petitioninfo) ||
		command_add("pf", "- Display additional mob coordinate and wandering data", AccountStatus::Player, command_pf) ||
		command_add("picklock",  "Analog for ldon pick lock for the newer clients since we still don't have it working.", AccountStatus::Player, command_picklock) ||
		command_add("profanity", "Manage censored language.", AccountStatus::GMLeadAdmin, command_profanity) ||
		command_add("push", "Lets you do spell push", AccountStatus::GMLeadAdmin, command_push) ||
		command_add("proximity", "Shows NPC proximity", AccountStatus::GMLeadAdmin, command_proximity) ||
		command_add("pvp", "[On|Off] - Set you or your player target's PVP status", AccountStatus::GMAdmin, command_pvp) ||
		command_add("qglobal", "[on/off/view] - Toggles qglobal functionality on an NPC", AccountStatus::GMAdmin, command_qglobal) ||
		command_add("questerrors", "Shows quest errors.", AccountStatus::GMAdmin, command_questerrors) ||
		command_add("race", "[racenum] - Change your or your target's race. Use racenum 0 to return to normal", AccountStatus::Guide, command_race) ||
		command_add("raidloot", "[All|GroupLeader|RaidLeader|Selected] - Sets your Raid Loot Type if you have permission to do so.", AccountStatus::Player, command_raidloot) ||
		command_add("randomfeatures", "- Temporarily randomizes the Facial Features of your target", AccountStatus::QuestTroupe, command_randomfeatures) ||
		command_add("refreshgroup", "- Refreshes Group.", AccountStatus::Player, command_refreshgroup) ||
		command_add("reloadaa", "Reloads AA data", AccountStatus::GMMgmt, command_reloadaa) ||
		command_add("reloadallrules", "Executes a reload of all rules.", AccountStatus::QuestTroupe, command_reloadallrules) ||
		command_add("reloademote", "Reloads NPC Emotes", AccountStatus::QuestTroupe, command_reloademote) ||
		command_add("reloadlevelmods", nullptr, AccountStatus::Max, command_reloadlevelmods) ||
		command_add("reloadmerchants", nullptr, AccountStatus::Max, command_reloadmerchants) ||
		command_add("reloadperlexportsettings", nullptr, AccountStatus::Max, command_reloadperlexportsettings) ||
		command_add("reloadqst", " - Clear quest cache (any argument causes it to also stop all timers)", AccountStatus::GMLeadAdmin, command_reloadqst) ||
		command_add("reloadrulesworld", "Executes a reload of all rules in world specifically.", AccountStatus::QuestTroupe, command_reloadworldrules) ||
		command_add("reloadstatic", "- Reload Static Zone Data", AccountStatus::GMLeadAdmin, command_reloadstatic) ||
		command_add("reloadtraps", "- Repops all traps in the current zone.", AccountStatus::QuestTroupe, command_reloadtraps) ||
		command_add("reloadtitles", "- Reload player titles from the database", AccountStatus::GMLeadAdmin, command_reloadtitles) ||
		command_add("reloadworld", "[0|1] - Clear quest cache (0 - no repop, 1 - repop)", AccountStatus::Max, command_reloadworld) ||
		command_add("reloadzps", "- Reload zone points from database", AccountStatus::GMLeadAdmin, command_reloadzps) ||
		command_add("removeitem", "[Item ID] [Amount] - Removes the specified Item ID by Amount from you or your player target's inventory (Amount defaults to 1 if not used)", AccountStatus::GMAdmin, command_removeitem) ||
		command_add("repop", "[delay] - Repop the zone with optional delay", AccountStatus::GMAdmin, command_repop) ||
		command_add("resetaa", "- Resets a Player's AA in their profile and refunds spent AA's to unspent, may disconnect player.", AccountStatus::GMMgmt, command_resetaa) ||
		command_add("resetaa_timer", "Command to reset AA cooldown timers.", AccountStatus::GMMgmt, command_resetaa_timer) ||
		command_add("resetdisc_timer", "Command to reset all discipline cooldown timers.", AccountStatus::GMMgmt, command_resetdisc_timer) ||
		command_add("revoke", "[charname] [1/0] - Makes charname unable to talk on OOC", AccountStatus::GMMgmt, command_revoke) ||
		command_add("roambox", "Manages roambox settings for an NPC", AccountStatus::GMMgmt, command_roambox) ||
		command_add("rules", "(subcommand) - Manage server rules", AccountStatus::GMImpossible, command_rules) ||
		command_add("save", "- Force your player or player corpse target to be saved to the database", AccountStatus::Guide, command_save) ||
		command_add("scale", "- Handles npc scaling", AccountStatus::GMLeadAdmin, command_scale) ||
		command_add("scribespell", "[spellid] - Scribe specified spell in your target's spell book.", AccountStatus::GMCoder, command_scribespell) ||
		command_add("scribespells", "[max level] [min level] - Scribe all spells for you or your player target that are usable by them, up to level specified. (may freeze client for a few seconds)", AccountStatus::GMLeadAdmin, command_scribespells) ||
		command_add("sendzonespawns", "- Refresh spawn list for all clients in zone", AccountStatus::GMLeadAdmin, command_sendzonespawns) ||
		command_add("sensetrap",  "Analog for ldon sense trap for the newer clients since we still don't have it working.", AccountStatus::Player, command_sensetrap) ||
		command_add("serverinfo", "- Get OS info about server host", AccountStatus::GMMgmt, command_serverinfo) ||
		command_add("serverrules", "- Read this server's rules", AccountStatus::Player, command_serverrules) ||
		command_add("setaapts", "[AA|Group|Raid] [AA Amount] - Set your or your player target's Available AA Points by Type", AccountStatus::GMAdmin, command_setaapts) ||
		command_add("setaaxp", "[AA|Group|Raid] [AA Experience] - Set your or your player target's AA Experience by Type", AccountStatus::GMAdmin, command_setaaxp) ||
		command_add("setadventurepoints", "- Set your or your player target's available adventure points", AccountStatus::GMLeadAdmin, command_set_adventure_points) ||
		command_add("setanim", "[Animation ID (IDs are 0 to 4)] - Set target's appearance to Animation ID", AccountStatus::GMMgmt, command_setanim) ||
		command_add("setcrystals", "[value] - Set your or your player target's available radiant or ebon crystals", AccountStatus::GMAdmin, command_setcrystals) ||
		command_add("setendurance", "[Endurance] - Set your or your target's Endurance", AccountStatus::GMAdmin, command_setendurance) ||
		command_add("setfaction", "[Faction ID] - Sets targeted NPC's faction in the database", AccountStatus::GMAreas, command_setfaction) ||
		command_add("setgraveyard", "[zone name] - Creates a graveyard for the specified zone based on your target's LOC.", AccountStatus::GMMgmt, command_setgraveyard) ||
		command_add("sethp", "[Health] - Set your or your target's Health", AccountStatus::GMAdmin, command_sethp) ||
		command_add("setlanguage", "[language ID] [value] - Set your target's language skillnum to value", AccountStatus::Guide, command_setlanguage) ||
		command_add("setlsinfo", "[email] [password] - Set login server email address and password (if supported by login server)", AccountStatus::Steward, command_setlsinfo) ||
		command_add("setmana", "[Mana] - Set your or your target's Mana", AccountStatus::GMAdmin, command_setmana) ||
		command_add("setpass", "[accountname] [password] - Set local password for accountname", AccountStatus::GMLeadAdmin, command_setpass) ||
		command_add("setpvppoints", "[Amount] - Set your or your player target's PVP points", AccountStatus::GMAdmin, command_setpvppoints) ||
		command_add("setskill", "[skillnum] [value] - Set your target's skill skillnum to value", AccountStatus::Guide, command_setskill) ||
		command_add("setskillall", "[value] - Set all of your target's skills to value", AccountStatus::Guide, command_setskillall) ||
		command_add("setstartzone", "[zoneid] - Set target's starting zone. Set to zero to allow the player to use /setstartcity", AccountStatus::QuestTroupe, command_setstartzone) ||
		command_add("setstat", "- Sets the stats to a specific value.", AccountStatus::Max, command_setstat) ||
		command_add("setxp", "[value] - Set your or your player target's experience", AccountStatus::GMAdmin, command_setxp) ||
		command_add("showbonusstats", "[item|spell|all] Shows bonus stats for target from items or spells. Shows both by default.", AccountStatus::Guide, command_showbonusstats) ||
		command_add("showbuffs", "- List buffs active on your target or you if no target", AccountStatus::Guide, command_showbuffs) ||
		command_add("shownumhits",  "Shows buffs numhits for yourself.", AccountStatus::Player, command_shownumhits) ||
		command_add("shownpcgloballoot", "Show GlobalLoot entires on this npc", AccountStatus::Guide, command_shownpcgloballoot) ||
		command_add("showskills", "- Show the values of your or your player target's skills", AccountStatus::Guide, command_showskills) ||
		command_add("showspellslist", "Shows spell list of targeted NPC", AccountStatus::GMAdmin, command_showspellslist) ||
		command_add("showstats", "- Show details about you or your target", AccountStatus::Guide, command_showstats) ||
		command_add("showzonegloballoot", "Show GlobalLoot entires on this zone", AccountStatus::Guide, command_showzonegloballoot) ||
		command_add("showzonepoints", "Show zone points for current zone", AccountStatus::Guide, command_showzonepoints) ||
		command_add("shutdown", "- Shut this zone process down", AccountStatus::GMLeadAdmin, command_shutdown) ||
		command_add("size", "[size] - Change size of you or your target", AccountStatus::Guide, command_size) ||
		command_add("spawn", "[name] [race] [level] [material] [hp] [gender] [class] [priweapon] [secweapon] [merchantid] - Spawn an NPC", AccountStatus::Steward, command_spawn) ||
		command_add("spawneditmass", "Mass editing spawn command", AccountStatus::GMLeadAdmin, command_spawneditmass) ||
		command_add("spawnfix", "- Find targeted NPC in database based on its X/Y/heading and update the database to make it spawn at your current location/heading.", AccountStatus::GMAreas, command_spawnfix) ||
		command_add("spawnstatus", "- Show respawn timer status", AccountStatus::GMAdmin, command_spawnstatus) ||
		command_add("spellinfo", "[spellid] - Get detailed info about a spell", AccountStatus::Steward, command_spellinfo) ||
		command_add("stun", "[duration] - Stuns you or your target for duration", AccountStatus::GMAdmin, command_stun) ||
		command_add("summon", "[charname] - Summons your player/npc/corpse target, or charname if specified", AccountStatus::QuestTroupe, command_summon) ||
		command_add("summonburiedplayercorpse", "- Summons the target's oldest buried corpse, if any exist.", AccountStatus::GMAdmin, command_summonburiedplayercorpse) ||
		command_add("summonitem", "[itemid] [charges] - Summon an item onto your cursor. Charges are optional.", AccountStatus::GMMgmt, command_summonitem) ||
		command_add("suspend", "[name] [days] [reason] - Suspend by character name and for specificed number of days", AccountStatus::GMLeadAdmin, command_suspend) ||
		command_add("task", "(subcommand) - Task system commands", AccountStatus::GMLeadAdmin, command_task) ||
		command_add("tattoo", "- Change the tattoo of your target (Drakkin Only)", AccountStatus::QuestTroupe, command_tattoo) ||
		command_add("tempname", "[newname] - Temporarily renames your target. Leave name blank to restore the original name.", AccountStatus::GMAdmin, command_tempname) ||
		command_add("petname", "[newname] - Temporarily renames your pet. Leave name blank to restore the original name.", AccountStatus::GMAdmin, command_petname) ||
		command_add("texture", "[Texture] [Helmet Texture] - Change your or your target's texture (Helmet Texture defaults to 0 if not used)", AccountStatus::Steward, command_texture) ||
		command_add("time", "[HH] [MM] - Set EQ time", AccountStatus::EQSupport, command_time) ||
		command_add("timers", "- Display persistent timers for target", AccountStatus::GMMgmt, command_timers) ||
		command_add("timezone", "[HH] [MM] - Set timezone. Minutes are optional", AccountStatus::EQSupport, command_timezone) ||
		command_add("title", "[Remove|Title] [Save (0 = False, 1 = True)] - Set your or your player target's title (use remove to remove title, Save defaults to false if not used)", AccountStatus::Guide, command_title) ||
		command_add("titlesuffix", "[Remove|Title Suffix] [Save (0 = False, 1 = True)] - Set your or your player target's title suffix (use remove to remove title suffix, Save defaults to false if not used)", AccountStatus::Guide, command_titlesuffix) ||
		command_add("traindisc", "[level] - Trains all the disciplines usable by the target, up to level specified. (may freeze client for a few seconds)", AccountStatus::GMLeadAdmin, command_traindisc) ||
		command_add("trapinfo", "- Gets infomation about the traps currently spawned in the zone.", AccountStatus::QuestTroupe, command_trapinfo) ||
		command_add("tune",  "Calculate statistical values related to combat.", AccountStatus::GMAdmin, command_tune) ||
		command_add("ucs", "- Attempts to reconnect to the UCS server", AccountStatus::Player, command_ucs) ||
		command_add("undyeme", "- Remove dye from all of your armor slots", AccountStatus::Player, command_undyeme) ||
		command_add("unfreeze", "- Unfreeze your target", AccountStatus::QuestTroupe, command_unfreeze) ||
		command_add("unlock", "- Unlock the worldserver", AccountStatus::GMLeadAdmin, command_unlock) ||
		command_add("unscribespell", "[spellid] - Unscribe specified spell from your target's spell book.", AccountStatus::GMCoder, command_unscribespell) ||
		command_add("unscribespells", "- Clear out your or your player target's spell book.", AccountStatus::GMCoder, command_unscribespells) ||
		command_add("untraindisc", "[spellid] - Untrain specified discipline from your target.", AccountStatus::GMCoder, command_untraindisc) ||
		command_add("untraindiscs", "- Untrains all disciplines from your target.", AccountStatus::GMCoder, command_untraindiscs) ||
		command_add("uptime", "[zone server id] - Get uptime of worldserver, or zone server if argument provided", AccountStatus::Steward, command_uptime) ||
		command_add("version", "- Display current version of EQEmu server", AccountStatus::Player, command_version) ||
		command_add("viewcurrencies", "- View your or your target's currencies", AccountStatus::GMAdmin, command_viewcurrencies) ||
		command_add("viewnpctype", "[NPC ID] - Show stats for an NPC by NPC ID", AccountStatus::GMAdmin, command_viewnpctype) ||
		command_add("viewpetition", "[petition number] - View a petition", AccountStatus::ApprenticeGuide, command_viewpetition) ||
		command_add("viewzoneloot", "[item id] - Allows you to search a zone's loot for a specific item ID. (0 shows all loot in the zone)", AccountStatus::QuestTroupe, command_viewzoneloot) ||
		command_add("wc", "[wear slot] [material] - Sends an OP_WearChange for your target", AccountStatus::GMMgmt, command_wc) ||
		command_add("weather", "[0/1/2/3] (Off/Rain/Snow/Manual) - Change the weather", AccountStatus::QuestTroupe, command_weather) ||
		command_add("who", "[search]", AccountStatus::ApprenticeGuide, command_who) ||
		command_add("worldshutdown", "- Shut down world and all zones", AccountStatus::GMMgmt, command_worldshutdown) ||
		command_add("wp", "[add|delete] [grid_id] [pause] [waypoint_id] [-h] - Add or delete a waypoint by grid ID. (-h to use current heading)", AccountStatus::GMAreas, command_wp) ||
		command_add("wpadd", "[pause] [-h] - Add your current location as a waypoint to your NPC target's AI path. (-h to use current heading)", AccountStatus::GMAreas, command_wpadd) ||
		command_add("wpinfo", "- Show waypoint info about your NPC target", AccountStatus::GMAreas, command_wpinfo) ||
		command_add("worldwide", "Performs world-wide GM functions such as cast (can be extended for other commands). Use caution", AccountStatus::GMImpossible, command_worldwide) ||
		command_add("xtargets",  "Show your targets Extended Targets and optionally set how many xtargets they can have.", AccountStatus::GMImpossible, command_xtargets) ||
		command_add("zclip", "[Minimum Clip] [Maximum Clip] [Fog Minimum Clip] [Fog Maximum Clip] [Permanent (0 = False, 1 = True)] - Change zone clipping", AccountStatus::QuestTroupe, command_zclip) ||
		command_add("zcolor", "[Red] [Green] [Blue] [Permanent (0 = False, 1 = True)] - Change sky color", AccountStatus::QuestTroupe, command_zcolor) ||
		command_add("zheader", "[Zone ID|Zone Short Name] [Version] - Load a zone header from the database", AccountStatus::QuestTroupe, command_zheader) ||
		command_add("zone", "[zonename] [x] [y] [z] - Go to specified zone (coords optional)", AccountStatus::Guide, command_zone) ||
		command_add("zonebootup", "[ZoneServerID] [shortname] - Make a zone server boot a specific zone", AccountStatus::GMLeadAdmin, command_zonebootup) ||
		command_add("zoneinstance", "[instanceid] [x] [y] [z] - Go to specified instance zone (coords optional)", AccountStatus::Guide, command_zone_instance) ||
		command_add("zonelock", "[List|Lock|Unlock] [Zone ID|Zone Short Name] - Set or get lock status of a Zone by ID or Short Name", AccountStatus::GMAdmin, command_zonelock) ||
		command_add("zoneshutdown", "[shortname] - Shut down a zone server", AccountStatus::GMLeadAdmin, command_zoneshutdown) ||
		command_add("zonestatus", "- Show connected zoneservers, synonymous with /servers", AccountStatus::GMLeadAdmin, command_zonestatus) ||
		command_add("zopp",  "Troubleshooting command - Sends a fake item packet to you. No server reference is created.", AccountStatus::GMImpossible, command_zopp) ||
		command_add("zsafecoords", "[X] [Y] [Z] [Heading] [Permanent (0 = False, 1 = True)] - Set the current zone's safe coordinates", AccountStatus::QuestTroupe, command_zsafecoords) ||
		command_add("zsave", " - Saves zheader to the database", AccountStatus::QuestTroupe, command_zsave) ||
		command_add("zsky", "[Sky Type] [Permanent (0 = False, 1 = True)] - Change zone sky type", AccountStatus::QuestTroupe, command_zsky) ||
		command_add("zstats", "- Show info about zone header", AccountStatus::QuestTroupe, command_zstats) ||
		command_add("zunderworld", "[Z] [Permanent (0 = False, 1 = True)] - Change zone underworld Z", AccountStatus::QuestTroupe, command_zunderworld)
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

void command_emptyinventory(Client *c, const Seperator *sep)
{
	Client *target = c;
	if (c->GetGM() && c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	EQ::ItemInstance *item = nullptr;
	static const int16 slots[][2] = {
		{ EQ::invslot::POSSESSIONS_BEGIN, EQ::invslot::POSSESSIONS_END },
		{ EQ::invbag::GENERAL_BAGS_BEGIN, EQ::invbag::GENERAL_BAGS_END },
		{ EQ::invbag::CURSOR_BAG_BEGIN, EQ::invbag::CURSOR_BAG_END},
		{ EQ::invslot::BANK_BEGIN, EQ::invslot::BANK_END },
		{ EQ::invbag::BANK_BAGS_BEGIN, EQ::invbag::BANK_BAGS_END },
		{ EQ::invslot::SHARED_BANK_BEGIN, EQ::invslot::SHARED_BANK_END },
		{ EQ::invbag::SHARED_BANK_BAGS_BEGIN, EQ::invbag::SHARED_BANK_BAGS_END },
	};
	int removed_count = 0;
	const size_t size = sizeof(slots) / sizeof(slots[0]);
	for (int slot_index = 0; slot_index < size; ++slot_index) {
		for (int slot_id = slots[slot_index][0]; slot_id <= slots[slot_index][1]; ++slot_id) {
			item = target->GetInv().GetItem(slot_id);
			if (item) {
				int stack_size = std::max(static_cast<int>(item->GetCharges()), 1);
				removed_count += stack_size;
				target->DeleteItemInInventory(slot_id, 0, true);
			}
		}
	}

	if (removed_count) {
		c->Message(
			Chat::White,
			fmt::format(
				"Inventory cleared for {}, {} items deleted.",
				(
					c == target ?
					"yourself" :
					fmt::format(
						"{} ({})",
						target->GetCleanName(),
						target->GetID()
					)
				),
				removed_count
			).c_str()
		);
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"{} no items to delete.",
				(
					c == target ?
					"You have" :
					fmt::format(
						"{} ({}) has",
						target->GetCleanName(),
						target->GetID()
					)
				)
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
