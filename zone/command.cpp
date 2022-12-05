
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
#include "../common/strings.h"
#include "../common/say_link.h"
#include "../common/net/eqstream.h"
#include "../common/file.h"
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


int command_count; // how many commands we have

// this is the pointer to the dispatch function, updated once
// init has been performed to point at the real function
int (*command_dispatch)(Client *, std::string, bool) = command_notavail;

std::map<std::string, CommandRecord *> commandlist;
std::map<std::string, std::string> commandaliases;
std::vector<CommandRecord *> command_delete_list;
std::map<std::string, uint8> commands_map;

/*
 * command_notavail
 * This is the default dispatch function when commands aren't loaded.
 *
 * Parameters:
 *	not used
 *
 */
int command_notavail(Client *c, std::string message, bool ignore_status)
{
	c->Message(Chat::White, "Commands not available.");
	return -1;
}

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
	if (!commandaliases.empty()) {
		command_deinit();
	}

	if (
		command_add("acceptrules", "[acceptrules] - Accept the EQEmu Agreement", AccountStatus::Player, command_acceptrules) ||
		command_add("advnpcspawn", "[maketype|makegroup|addgroupentry|addgroupspawn][removegroupspawn|movespawn|editgroupbox|cleargroupbox]", AccountStatus::GMLeadAdmin, command_advnpcspawn) ||
		command_add("aggro", "[Distance] [-v] - Display aggro information for all mobs 'Distance' distance from your target. (-v is verbose Faction Information)", AccountStatus::QuestTroupe, command_aggro) ||
		command_add("aggrozone", "[aggro] - Aggro every mob in the zone with X aggro. Default is 0. Not recommend if you're not invulnerable.", AccountStatus::GMAdmin, command_aggrozone) ||
		command_add("ai", "[factionid/spellslist/con/guard/roambox/stop/start] - Modify AI on NPC target", AccountStatus::GMAdmin, command_ai) ||
		command_add("appearance", "[type] [value] - Send an appearance packet for you or your target", AccountStatus::GMLeadAdmin, command_appearance) ||
		command_add("appearanceeffects", "[view] [set] [remove] appearance effects.", AccountStatus::GMAdmin, command_appearanceeffects) ||
		command_add("apply_shared_memory", "[shared_memory_name] - Tells every zone and world to apply a specific shared memory segment by name.", AccountStatus::GMImpossible, command_apply_shared_memory) ||
		command_add("attack", "[Entity Name] - Make your NPC target attack an entity by name", AccountStatus::GMLeadAdmin, command_attack) ||
		command_add("augmentitem", "Force augments an item. Must have the augment item window open.", AccountStatus::GMImpossible, command_augmentitem) ||
		command_add("ban", "[Character Name] [Reason] - Ban by character name", AccountStatus::GMLeadAdmin, command_ban) ||
		command_add("bind", "Sets your targets bind spot to their current location", AccountStatus::GMMgmt, command_bind) ||
		command_add("bugs", "[Close|Delete|Review|Search|View] - Handles player bug reports", AccountStatus::QuestTroupe, command_bugs) ||

#ifdef BOTS
		command_add("bot", "Type \"#bot help\" or \"^help\" to the see the list of available commands for bots.", AccountStatus::Player, command_bot) ||
#endif

		command_add("camerashake", "[Duration (Milliseconds)] [Intensity (1-10)] - Shakes the camera on everyone's screen globally.", AccountStatus::QuestTroupe, command_camerashake) ||
		command_add("castspell", "[Spell ID] [Instant (0 = False, 1 = True, Default is 1 if Unused)] - Cast a spell", AccountStatus::Guide, command_castspell) ||
		command_add("chat", "[Channel ID] [Message] - Send a channel message to all zones", AccountStatus::GMMgmt, command_chat) ||
		command_add("checklos", "Check for line of sight to your target", AccountStatus::Guide, command_checklos) ||
		command_add("copycharacter", "[source_char_name] [dest_char_name] [dest_account_name] - Copies character to destination account", AccountStatus::GMImpossible, command_copycharacter) ||
		command_add("corpse", "Manipulate corpses, use with no arguments for help", AccountStatus::Guide, command_corpse) ||
		command_add("corpsefix", "Attempts to bring corpses from underneath the ground within close proximity of the player", AccountStatus::Player, command_corpsefix) ||
		command_add("countitem", "[Item ID] - Counts the specified Item ID in your or your target's inventory", AccountStatus::GMLeadAdmin, command_countitem) ||
		command_add("cvs", "Summary of client versions currently online.", AccountStatus::GMMgmt, command_cvs) ||
		command_add("damage", "[Amount] - Damage yourself or your target", AccountStatus::GMAdmin, command_damage) ||
		command_add("databuckets", "View|Delete [key] [limit]- View data buckets, limit 50 default or Delete databucket by key", AccountStatus::QuestTroupe, command_databuckets) ||
		command_add("date", "[Year] [Month] [Day] [Hour] [Minute] - Set EQ time (Hour and Minute are optional)", AccountStatus::EQSupport, command_date) ||
		command_add("dbspawn2", "[Spawngroup ID] [Respawn] [Variance] [Condition ID] [Condition Minimum] - Spawn an NPC from a predefined row in the spawn2 table, Respawn and Variance are in Seconds (condition is optional)", AccountStatus::GMAdmin, command_dbspawn2) ||
		command_add("delacct", "[Account ID|Account Name] - Delete an account by ID or Name", AccountStatus::GMLeadAdmin, command_delacct) ||
		command_add("delpetition", "[petition number] - Delete a petition", AccountStatus::ApprenticeGuide, command_delpetition) ||
		command_add("depop", "[Start Spawn Timer] - Depop your NPC target and optionally start their spawn timer (false by default)", AccountStatus::Guide, command_depop) ||
		command_add("depopzone", "[Start Spawn Timers] - Depop the zone and optionally start spawn timers (false by default)", AccountStatus::GMAdmin, command_depopzone) ||
		command_add("devtools", "[Enable|Disable] - Manages Developer Tools (send no parameter for menu)", AccountStatus::GMMgmt, command_devtools) ||
		command_add("disablerecipe", "[Recipe ID] - Disables a Recipe", AccountStatus::QuestTroupe, command_disablerecipe) ||
		command_add("disarmtrap", "Analog for ldon disarm trap for the newer clients since we still don't have it working.", AccountStatus::QuestTroupe, command_disarmtrap) ||
		command_add("distance", "Reports the distance between you and your target.", AccountStatus::QuestTroupe, command_distance) ||
		command_add("door", "Door editing command", AccountStatus::QuestTroupe, command_door) ||
		command_add("doanim", "[Animation ID|Animation Name] [Speed] - Send an animation by ID or name at the specified speed to you or your target (Speed is optional)", AccountStatus::Guide, command_doanim) ||
		command_add("dye", "[slot|'help'] [red] [green] [blue] [use_tint] - Dyes the specified armor slot to Red, Green, and Blue provided, allows you to bypass darkness limits.", AccountStatus::ApprenticeGuide, command_dye) ||
		command_add("dz", "Manage expeditions and dynamic zone instances", AccountStatus::QuestTroupe, command_dz) ||
		command_add("dzkickplayers", "Removes all players from current expedition. (/kickplayers alternative for pre-RoF clients)", AccountStatus::Player, command_dzkickplayers) ||
		command_add("editmassrespawn", "[name-search] [second-value] - Mass (Zone wide) NPC respawn timer editing command", AccountStatus::GMAdmin, command_editmassrespawn) ||
		command_add("emote", "[Name|World|Zone] [type] [message] - Send an emote message by name, to the world, or to your zone (^ separator allows multiple messages to be sent at once)", AccountStatus::QuestTroupe, command_emote) ||
		command_add("emotesearch", "[Search Criteria] - Search for NPC Emotes", AccountStatus::QuestTroupe, command_emotesearch) ||
		command_add("emoteview", "Lists all NPC Emotes", AccountStatus::QuestTroupe, command_emoteview) ||
		command_add("emptyinventory", "Clears your or your target's entire inventory (Equipment, General, Bank, and Shared Bank)", AccountStatus::GMImpossible, command_emptyinventory) ||
		command_add("enablerecipe", "[Recipe ID] - Enables a Recipe", AccountStatus::QuestTroupe, command_enablerecipe) ||
		command_add("endurance", "Restores your or your target's endurance.", AccountStatus::Guide, command_endurance) ||
		command_add("equipitem", "[slotid(0-21)] - Equip the item on your cursor into the specified slot", AccountStatus::Guide, command_equipitem) ||
		command_add("faction", "[Find (criteria | all ) | Review (criteria | all) | Reset (id)] - Resets Player's Faction", AccountStatus::QuestTroupe, command_faction) ||
		command_add("factionassociation", "[factionid] [amount] - triggers a faction hits via association", AccountStatus::GMLeadAdmin, command_faction_association) ||
		command_add("feature", "Change your or your target's feature's temporarily", AccountStatus::QuestTroupe, command_feature) ||
		command_add("findaa", "[Search Criteria] - Search for an AA", AccountStatus::Guide, command_findaa) ||
		command_add("findaliases", "[Search Criteria]- Searches for available command aliases, by alias or command", AccountStatus::Player, command_findaliases) ||
		command_add("findclass", "[Search Criteria] - Search for a class", AccountStatus::Guide, command_findclass) ||
		command_add("findfaction", "[Search Criteria] - Search for a faction", AccountStatus::Guide, command_findfaction) ||
		command_add("findnpctype", "[Search Criteria] - Search database NPC types", AccountStatus::GMAdmin, command_findnpctype) ||
		command_add("findrace", "[Search Criteria] - Search for a race", AccountStatus::Guide, command_findrace) ||
		command_add("findrecipe", "[Search Criteria] - Search for a recipe", AccountStatus::Guide, command_findrecipe) ||
		command_add("findskill", "[Search Criteria] - Search for a skill", AccountStatus::Guide, command_findskill) ||
		command_add("findspell", "[Search Criteria] - Search for a spell", AccountStatus::Guide, command_findspell) ||
		command_add("findtask", "[Search Criteria] - Search for a task", AccountStatus::Guide, command_findtask) ||
		command_add("findzone", "[Search Criteria] - Search database zones", AccountStatus::GMAdmin, command_findzone) ||
		command_add("fixmob", "[race|gender|texture|helm|face|hair|haircolor|beard|beardcolor|heritage|tattoo|detail] [next|prev] - Manipulate appearance of your target", AccountStatus::QuestTroupe, command_fixmob) ||
		command_add("flag", "[Status] [Account Name] - Refresh your admin status, or set an account's Admin status if arguments provided", AccountStatus::Player, command_flag) ||
		command_add("flagedit", "Edit zone flags on your target. Use #flagedit help for more info.", AccountStatus::GMAdmin, command_flagedit) ||
		command_add("flags", "displays the Zone Flags of you or your target", AccountStatus::Player, command_flags) ||
		command_add("flymode", "[0/1/2/3/4/5] - Set your or your player target's flymode to ground/flying/levitate/water/floating/levitate_running", AccountStatus::Guide, command_flymode) ||
		command_add("fov", "Check wether you're behind or in your target's field of view", AccountStatus::QuestTroupe, command_fov) ||
		command_add("freeze", "Freeze your target", AccountStatus::QuestTroupe, command_freeze) ||
		command_add("gassign", "[Grid ID] - Assign targetted NPC to predefined wandering grid id", AccountStatus::GMAdmin, command_gassign) ||
		command_add("gearup", "Developer tool to quickly equip yourself or your target", AccountStatus::GMMgmt, command_gearup) ||
		command_add("gender", "[0/1/2] - Change your or your target's gender to male/female/neuter", AccountStatus::Guide, command_gender) ||
		command_add("getplayerburiedcorpsecount", "Get your or your target's total number of buried player corpses.", AccountStatus::GMAdmin, command_getplayerburiedcorpsecount) ||
		command_add("getvariable", "[Variable Name] - Get the value of a variable from the database", AccountStatus::GMMgmt, command_getvariable) ||
		command_add("ginfo", "get group info on target.", AccountStatus::ApprenticeGuide, command_ginfo) ||
		command_add("giveitem", "[itemid] [charges] - Summon an item onto your target's cursor. Charges are optional.", AccountStatus::GMMgmt, command_giveitem) ||
		command_add("givemoney", "[Platinum] [Gold] [Silver] [Copper] - Gives specified amount of money to you or your player target", AccountStatus::GMMgmt, command_givemoney) ||
		command_add("globalview", "Lists all qglobals in cache if you were to do a quest with this target.", AccountStatus::QuestTroupe, command_globalview) ||
		command_add("gm", "[On|Off] - Modify your or your target's GM Flag", AccountStatus::QuestTroupe, command_gm) ||
		command_add("gmspeed", "[On|Off] - Turn GM Speed On or Off for you or your player target", AccountStatus::GMAdmin, command_gmspeed) ||
		command_add("gmzone", "[Zone ID|Zone Short Name] [Version] [Instance Identifier] - Zones to a private GM instance (Version defaults to 0 and Instance Identifier defaults to 'gmzone' if not used)", AccountStatus::GMAdmin, command_gmzone) ||
		command_add("godmode", "[on/off] - Turns on/off hideme, gmspeed, invul, and flymode.", AccountStatus::GMMgmt, command_godmode) ||
		command_add("goto", "[playername] or [x y z] [h] - Teleport to the provided coordinates or to your target", AccountStatus::Steward, command_goto) ||
		command_add("grid", "[add/delete] [grid_num] [wandertype] [pausetype] - Create/delete a wandering grid", AccountStatus::GMAreas, command_grid) ||
		command_add("guild", "Guild manipulation commands. Use argument help for more info.", AccountStatus::Steward, command_guild) ||
		command_add("guildapprove", "[guildapproveid] - Approve a guild with specified ID (guild creator receives the id)", AccountStatus::Player, command_guildapprove) ||
		command_add("guildcreate", "[guildname] - Creates an approval setup for guild name specified", AccountStatus::Player, command_guildcreate) ||
		command_add("guildlist", "[guildapproveid] - Lists character names who have approved the guild specified by the approve id", AccountStatus::Player, command_guildlist) ||
		command_add("haste", "[percentage] - Set your haste percentage", AccountStatus::GMAdmin, command_haste) ||
		command_add("hatelist", "Display hate list for NPC.", AccountStatus::QuestTroupe, command_hatelist) ||
		command_add("heal", "Completely heal your target", AccountStatus::Steward, command_heal) ||
		command_add("help", "[Search Criteria] - List available commands and their description, specify partial command as argument to search", AccountStatus::Player, command_help) ||
		command_add("heromodel", "[Hero Model] [Slot] - Set your or your target's appearance to a full set of Hero's Forge Armor, if slot is set, sends exact model just to slot.", AccountStatus::GMMgmt, command_heromodel) ||
		command_add("hideme", "[on/off] - Hide yourself from spawn lists.", AccountStatus::QuestTroupe, command_hideme) ||
		command_add("hotfix", "[hotfix_name] - Reloads shared memory into a hotfix, equiv to load_shared_memory followed by apply_shared_memory", AccountStatus::GMImpossible, command_hotfix) ||
		command_add("hp", "Refresh your HP bar from the server.", AccountStatus::Player, command_hp) ||
		command_add("incstat", "Increases or Decreases a client's stats permanently.", AccountStatus::GMMgmt, command_incstat) ||
		command_add("instance", "Modify Instances", AccountStatus::GMMgmt, command_instance) ||
		command_add("interrogateinv", "use [help] argument for available options", AccountStatus::Player, command_interrogateinv) ||
		command_add("interrupt", "[message id] [color] - Interrupt your casting. Arguments are optional.", AccountStatus::Guide, command_interrupt) ||
		command_add("invsnapshot", "Manipulates inventory snapshots for your current target", AccountStatus::QuestTroupe, command_invsnapshot) ||
		command_add("invul", "[On|Off]] - Turn player target's or your invulnerable flag on or off", AccountStatus::QuestTroupe, command_invul) ||
		command_add("ipban", "[IP] - Ban IP", AccountStatus::GMMgmt, command_ipban) ||
		command_add("iplookup", "[charname] - Look up IP address of charname", AccountStatus::GMMgmt, command_iplookup) ||
		command_add("itemsearch", "[Search Criteria] - Search for an item", AccountStatus::Steward, command_itemsearch) ||
		command_add("kick", "[Character Name] - Disconnect a player by name", AccountStatus::GMLeadAdmin, command_kick) ||
		command_add("kill", "Kill your target", AccountStatus::GMAdmin, command_kill) ||
		command_add("killallnpcs", "[npc_name] - Kills all npcs by search name, leave blank for all attackable NPC's", AccountStatus::GMMgmt, command_killallnpcs) ||
		command_add("lastname", "[Last Name] - Set your or your player target's last name (use \"-1\" to remove last name)", AccountStatus::Guide, command_lastname) ||
		command_add("level", "[Level] - Set your target's level", AccountStatus::Steward, command_level) ||
		command_add("list", "[npcs|players|corpses|doors|objects] [search] - Search entities", AccountStatus::ApprenticeGuide, command_list) ||
		command_add("listpetition", "List petitions", AccountStatus::Guide, command_listpetition) ||
		command_add("lootsim", "[npc_type_id] [loottable_id] [iterations] - Runs benchmark simulations using real loot logic to report numbers and data", AccountStatus::GMImpossible, command_lootsim) ||
		command_add("load_shared_memory", "[shared_memory_name] - Reloads shared memory and uses the input as output", AccountStatus::GMImpossible, command_load_shared_memory) ||
		command_add("loc", "Print out your or your target's current location and heading", AccountStatus::Player, command_loc) ||
		command_add("logs", "Manage anything to do with logs", AccountStatus::GMImpossible, command_logs) ||
		command_add("makepet", "[Pet Name] - Make a pet", AccountStatus::Guide, command_makepet) ||
		command_add("mana", "Fill your or your target's mana", AccountStatus::Guide, command_mana) ||
		command_add("maxskills", "Maxes skills for you.", AccountStatus::GMMgmt, command_max_all_skills) ||
		command_add("memspell", "[Spell ID] [Spell Gem] - Memorize a Spell by ID to the specified Spell Gem for you or your target", AccountStatus::Guide, command_memspell) ||
		command_add("merchant_close_shop", "Closes a merchant shop", AccountStatus::GMAdmin, command_merchantcloseshop) ||
		command_add("merchant_open_shop", "Opens a merchants shop", AccountStatus::GMAdmin, command_merchantopenshop) ||
		command_add("modifynpcstat", "[Stat] [Value] - Modifies an NPC's stats temporarily.", AccountStatus::GMLeadAdmin, command_modifynpcstat) ||
		command_add("motd", "[Message of the Day] - Set Message of the Day (leave empty to have no Message of the Day)", AccountStatus::GMLeadAdmin, command_motd) ||
		command_add("movechar", "[Character ID|Character Name] [Zone ID|Zone Short Name] - Move an offline character to the specified zone", AccountStatus::Guide, command_movechar) ||
		command_add("movement", "Various movement commands", AccountStatus::GMMgmt, command_movement) ||
		command_add("myskills", "Show details about your current skill levels", AccountStatus::Player, command_myskills) ||
		command_add("mysql", "[Help|Query] [SQL Query] - Mysql CLI, see 'Help' for options.", AccountStatus::GMImpossible, command_mysql) ||
		command_add("mystats", "Show details about you or your pet", AccountStatus::Guide, command_mystats) ||
		command_add("name", "[New Name] - Rename your player target", AccountStatus::GMLeadAdmin, command_name) ||
		command_add("netstats", "Gets the network stats for a stream.", AccountStatus::GMMgmt, command_netstats) ||
		command_add("network", "Admin commands for the udp network interface.", AccountStatus::GMImpossible, command_network) ||
		command_add("npccast", "[targetname/entityid] [spellid] - Causes NPC target to cast spellid on targetname/entityid", AccountStatus::QuestTroupe, command_npccast) ||
		command_add("npcedit", "[column] [value] - Mega NPC editing command", AccountStatus::GMAdmin, command_npcedit) ||
		command_add("npceditmass", "[name-search] [column] [value] - Mass (Zone wide) NPC data editing command", AccountStatus::GMAdmin, command_npceditmass) ||
		command_add("npcemote", "[Message] - Make your NPC target emote a message.", AccountStatus::GMLeadAdmin, command_npcemote) ||
		command_add("npcloot", "Manipulate the loot an NPC is carrying. Use #npcloot help for more information.", AccountStatus::QuestTroupe, command_npcloot) ||
		command_add("npcsay", "[Message] - Make your NPC target say a message.", AccountStatus::GMLeadAdmin, command_npcsay) ||
		command_add("npcshout", "[Message] - Make your NPC target shout a message.", AccountStatus::GMLeadAdmin, command_npcshout) ||
		command_add("npcspawn", "[create/add/update/remove/delete] - Manipulate spawn DB", AccountStatus::GMAreas, command_npcspawn) ||
		command_add("npcstats", "Show stats about target NPC", AccountStatus::QuestTroupe, command_npcstats) ||
		command_add("npctypespawn", "[NPC ID] [Faction ID] - Spawn an NPC by ID from the database with an option of setting its Faction ID", AccountStatus::Steward, command_npctypespawn) ||
		command_add("nudge", "Nudge your target's current position by specific values", AccountStatus::QuestTroupe, command_nudge) ||
		command_add("nukebuffs", "[Beneficial|Detrimental|Help] - Strip all buffs by type on you or your target (no argument to remove all buffs)", AccountStatus::Guide, command_nukebuffs) ||
		command_add("nukeitem", "[Item ID] - Removes the specified Item ID from you or your player target's inventory", AccountStatus::GMLeadAdmin, command_nukeitem) ||
		command_add("object", "List|Add|Edit|Move|Rotate|Copy|Save|Undo|Delete - Manipulate static and tradeskill objects within the zone", AccountStatus::GMAdmin, command_object) ||
		command_add("oocmute", "[0|1] - Enable or Disable Server OOC", AccountStatus::GMMgmt, command_oocmute) ||
		command_add("opcode", "Reloads all server patches", AccountStatus::GMImpossible, command_opcode) ||
		command_add("path", "view and edit pathing", AccountStatus::GMMgmt, command_path) ||
		command_add("peekinv", "[equip/gen/cursor/poss/limbo/curlim/trib/bank/shbank/allbank/trade/world/all] - Print out contents of your player target's inventory", AccountStatus::GMAdmin, command_peekinv) ||
		command_add("peqzone", "[Zone ID|Zone Short Name] - Teleports you to the specified zone if you meet the requirements.", AccountStatus::Player, command_peqzone) ||
		command_add("peqzone_flags", "displays the PEQZone Flags of you or your target", AccountStatus::Player, command_peqzone_flags) ||
		command_add("permaclass", "[Class ID] - Change your or your player target's class, changed client is disconnected", AccountStatus::QuestTroupe, command_permaclass) ||
		command_add("permagender", "[Gender ID] - Change your or your player target's gender", AccountStatus::QuestTroupe, command_permagender) ||
		command_add("permarace", "[Race ID] - Change your or your player target's race", AccountStatus::QuestTroupe, command_permarace) ||
		command_add("petitems", "View your pet's items if you have one", AccountStatus::ApprenticeGuide, command_petitems) ||
		command_add("petitioninfo", "[petition number] - Get info about a petition", AccountStatus::ApprenticeGuide, command_petitioninfo) ||
		command_add("picklock", "Analog for ldon pick lock for the newer clients since we still don't have it working.", AccountStatus::Player, command_picklock) ||
		command_add("profanity", "Manage censored language.", AccountStatus::GMLeadAdmin, command_profanity) ||
		command_add("push", "[Back Push] [Up Push] - Lets you do spell push on an NPC", AccountStatus::GMLeadAdmin, command_push) ||
		command_add("proximity", "Shows NPC proximity", AccountStatus::GMLeadAdmin, command_proximity) ||
		command_add("pvp", "[On|Off] - Set you or your player target's PVP status", AccountStatus::GMAdmin, command_pvp) ||
		command_add("qglobal", "[On|Off|View] - Toggles quest global functionality for your NPC target", AccountStatus::GMAdmin, command_qglobal) ||
		command_add("questerrors", "Shows quest errors.", AccountStatus::GMAdmin, command_questerrors) ||
		command_add("race", "[racenum] - Change your or your target's race. Use racenum 0 to return to normal", AccountStatus::Guide, command_race) ||
		command_add("raidloot", "[All|GroupLeader|RaidLeader|Selected] - Sets your Raid Loot Type if you have permission to do so.", AccountStatus::Player, command_raidloot) ||
		command_add("randomfeatures", "Temporarily randomizes the Facial Features of your target", AccountStatus::QuestTroupe, command_randomfeatures) ||
		command_add("refreshgroup", "Refreshes Group for you or your player target.", AccountStatus::Player, command_refreshgroup) ||
		command_add("reload", "Reloads different types of server data globally, use no argument for help menu.", AccountStatus::GMMgmt, command_reload) ||
		command_add("removeitem", "[Item ID] [Amount] - Removes the specified Item ID by Amount from you or your player target's inventory (Amount defaults to 1 if not used)", AccountStatus::GMAdmin, command_removeitem) ||
		command_add("repop", "[Force] - Repop the zone with optional force repop", AccountStatus::GMAdmin, command_repop) ||
		command_add("resetaa", "Resets a Player's AA in their profile and refunds spent AA's to unspent, may disconnect player.", AccountStatus::GMMgmt, command_resetaa) ||
		command_add("resetaa_timer", "[All|Timer ID] - Command to reset AA cooldown timers for you or your player target.", AccountStatus::GMMgmt, command_resetaa_timer) ||
		command_add("resetdisc_timer", "[All|Timer ID] - Command to reset discipline timers.", AccountStatus::GMMgmt, command_resetdisc_timer) ||
		command_add("revoke", "[Character Name] [0|1] - Revokes or unrevokes a player's ability to talk in OOC by name (0 = Unrevoke, 1 = Revoke)", AccountStatus::GMMgmt, command_revoke) ||
		command_add("roambox", "[Remove|Set] [Box Size] [Delay (Milliseconds)] - Remove or set an NPC's roambox size and delay", AccountStatus::GMMgmt, command_roambox) ||
		command_add("rules", "(subcommand) - Manage server rules", AccountStatus::GMImpossible, command_rules) ||
		command_add("save", "Force your player or player corpse target to be saved to the database", AccountStatus::Guide, command_save) ||
		command_add("scale", "Handles npc scaling", AccountStatus::GMLeadAdmin, command_scale) ||
		command_add("scribespell", "[Spell ID] - Scribe a spell by ID to your or your target's spell book.", AccountStatus::GMCoder, command_scribespell) ||
		command_add("scribespells", "[Max level] [Min level] - Scribe all spells for you or your player target that are usable by them, up to level specified. (may freeze client for a few seconds)", AccountStatus::GMLeadAdmin, command_scribespells) ||
		command_add("sendzonespawns", "Refresh spawn list for all clients in zone", AccountStatus::GMLeadAdmin, command_sendzonespawns) ||
		command_add("sensetrap", "Analog for ldon sense trap for the newer clients since we still don't have it working.", AccountStatus::Player, command_sensetrap) ||
		command_add("serverinfo", "Get CPU, Operating System, and Process Information about the server", AccountStatus::GMMgmt, command_serverinfo) ||
		command_add("serverlock", "[0|1] - Lock or Unlock the World Server (0 = Unlocked, 1 = Locked)", AccountStatus::GMLeadAdmin, command_serverlock) ||
		command_add("serverrules", "Read this server's rules", AccountStatus::Player, command_serverrules) ||
		command_add("setaapts", "[AA|Group|Raid] [AA Amount] - Set your or your player target's Available AA Points by Type", AccountStatus::GMAdmin, command_setaapts) ||
		command_add("setaaxp", "[AA|Group|Raid] [AA Experience] - Set your or your player target's AA Experience by Type", AccountStatus::GMAdmin, command_setaaxp) ||
		command_add("setadventurepoints", "[Theme] [Points] - Set your or your player target's available Adventure Points by Theme", AccountStatus::GMLeadAdmin, command_set_adventure_points) ||
		command_add("setaltcurrency", "[Currency ID] [Amount] - Set your or your target's available Alternate Currency by Currency ID", AccountStatus::GMAdmin, command_setaltcurrency) ||
		command_add("setanim", "[Animation ID (IDs are 0 to 4)] - Set target's appearance to Animation ID", AccountStatus::GMMgmt, command_setanim) ||
		command_add("setcrystals", "[value] - Set your or your player target's available radiant or ebon crystals", AccountStatus::GMAdmin, command_setcrystals) ||
		command_add("setendurance", "[Endurance] - Set your or your target's Endurance", AccountStatus::GMAdmin, command_setendurance) ||
		command_add("setfaction", "[Faction ID] - Sets targeted NPC's faction in the database", AccountStatus::GMAreas, command_setfaction) ||
		command_add("sethp", "[Health] - Set your or your target's Health", AccountStatus::GMAdmin, command_sethp) ||
		command_add("setlanguage", "[Language ID] [Value] - Set your or your target's Language by ID to Value", AccountStatus::Guide, command_setlanguage) ||
		command_add("setlsinfo", "[Email] [Password] - Set loginserver email address and password (if supported by loginserver)", AccountStatus::Steward, command_setlsinfo) ||
		command_add("setmana", "[Mana] - Set your or your target's Mana", AccountStatus::GMAdmin, command_setmana) ||
		command_add("setpass", "[Account Name] [Password] - Set local password by account name", AccountStatus::GMLeadAdmin, command_setpass) ||
		command_add("setpvppoints", "[Amount] - Set your or your player target's PVP points", AccountStatus::GMAdmin, command_setpvppoints) ||
		command_add("setskill", "[skillnum] [value] - Set your target's skill skillnum to value", AccountStatus::Guide, command_setskill) ||
		command_add("setskillall", "[Skill Level] - Set all of your or your target's skills to the specified skill level", AccountStatus::Guide, command_setskillall) ||
		command_add("setstartzone", "[Zone ID|Zone Short Name] - Sets your or your target's starting zone (Use '0' or 'Reset' to allow the player use of /setstartcity)", AccountStatus::QuestTroupe, command_setstartzone) ||
		command_add("setstat", "Sets the stats to a specific value.", AccountStatus::Max, command_setstat) ||
		command_add("setxp", "[value] - Set your or your player target's experience", AccountStatus::GMAdmin, command_setxp) ||
		command_add("showbonusstats", "[item|spell|all] Shows bonus stats for target from items or spells. Shows both by default.", AccountStatus::Guide, command_showbonusstats) ||
		command_add("showbuffs", "List buffs active on your target or you if no target", AccountStatus::Guide, command_showbuffs) ||
		command_add("shownumhits", "Shows buffs numhits for yourself.", AccountStatus::Player, command_shownumhits) ||
		command_add("shownpcgloballoot", "Show global loot entries for your target NPC", AccountStatus::Guide, command_shownpcgloballoot) ||
		command_add("showskills", "[Start Skill ID] [All] - Show the values of your or your player target's skills in a popup 50 at a time, use 'all' as second argument to show non-usable skill's values", AccountStatus::Guide, command_showskills) ||
		command_add("showspellslist", "Shows spell list of targeted NPC", AccountStatus::GMAdmin, command_showspellslist) ||
		command_add("showstats", "Show details about you or your target", AccountStatus::Guide, command_showstats) ||
		command_add("showzonegloballoot", "Show global loot entries for your current zone", AccountStatus::Guide, command_showzonegloballoot) ||
		command_add("showzonepoints", "Show zone points for current zone", AccountStatus::Guide, command_showzonepoints) ||
		command_add("shutdown", "Shut this zone process down", AccountStatus::GMLeadAdmin, command_shutdown) ||
		command_add("spawn", "[name] [race] [level] [material] [hp] [gender] [class] [priweapon] [secweapon] [merchantid] - Spawn an NPC", AccountStatus::Steward, command_spawn) ||
		command_add("spawneditmass", "[Search Criteria] [Edit Option] [Edit Value] [Apply] Mass editing spawn command (Apply is optional, 0 = False, 1 = True, default is False)", AccountStatus::GMLeadAdmin, command_spawneditmass) ||
		command_add("spawnfix", "Find targeted NPC in database based on its X/Y/heading and update the database to make it spawn at your current location/heading.", AccountStatus::GMAreas, command_spawnfix) ||
		command_add("spawnstatus", "[All|Disabled|Enabled|Spawn ID] - Show respawn timer status", AccountStatus::GMAdmin, command_spawnstatus) ||
		command_add("spellinfo", "[spellid] - Get detailed info about a spell", AccountStatus::Steward, command_spellinfo) ||
		command_add("stun", "[duration] - Stuns you or your target for duration", AccountStatus::GMAdmin, command_stun) ||
		command_add("summon", "[Character Name] - Summons your corpse, NPC, or player target, or by character name if specified", AccountStatus::QuestTroupe, command_summon) ||
		command_add("summonburiedplayercorpse", "Summons the target's oldest buried corpse, if any exist.", AccountStatus::GMAdmin, command_summonburiedplayercorpse) ||
		command_add("summonitem", "[itemid] [charges] - Summon an item onto your cursor. Charges are optional.", AccountStatus::GMMgmt, command_summonitem) ||
		command_add("suspend", "[name] [days] [reason] - Suspend by character name and for specificed number of days", AccountStatus::GMLeadAdmin, command_suspend) ||
		command_add("suspendmulti", "[Character Name One|Character Name Two|etc] [Days] [reason] - Suspend multiple characters by name for specified number of days", AccountStatus::GMLeadAdmin, command_suspendmulti) ||
		command_add("task", "(subcommand) - Task system commands", AccountStatus::GMLeadAdmin, command_task) ||
		command_add("tempname", "[newname] - Temporarily renames your target. Leave name blank to restore the original name.", AccountStatus::GMAdmin, command_tempname) ||
		command_add("petname", "[newname] - Temporarily renames your pet. Leave name blank to restore the original name.", AccountStatus::GMAdmin, command_petname) ||
		command_add("texture", "[Texture] [Helmet Texture] - Change your or your target's texture (Helmet Texture defaults to 0 if not used)", AccountStatus::Steward, command_texture) ||
		command_add("time", "[Hour] [Minute] - Set world time to specified time", AccountStatus::EQSupport, command_time) ||
		command_add("timers", "Display persistent timers for target", AccountStatus::GMMgmt, command_timers) ||
		command_add("timezone", "[Hour] [Minutes] - Set timezone (Minutes are optional)", AccountStatus::EQSupport, command_timezone) ||
		command_add("title", "[Title] - Set your or your player target's title (use \"-1\" to remove title)", AccountStatus::Guide, command_title) ||
		command_add("titlesuffix", "[Title Suffix] - Set your or your player target's title suffix (use \"-1\" to remove title suffix)", AccountStatus::Guide, command_titlesuffix) ||
		command_add("traindisc", "[level] - Trains all the disciplines usable by the target, up to level specified. (may freeze client for a few seconds)", AccountStatus::GMLeadAdmin, command_traindisc) ||
		command_add("trapinfo", "Gets infomation about the traps currently spawned in the zone.", AccountStatus::QuestTroupe, command_trapinfo) ||
		command_add("tune", "Calculate statistical values related to combat.", AccountStatus::GMAdmin, command_tune) ||
		command_add("undye", "Remove dye from all of your or your target's armor slots", AccountStatus::GMAdmin, command_undye) ||
		command_add("undyeme", "Remove dye from all of your armor slots", AccountStatus::Player, command_undyeme) ||
		command_add("unfreeze", "Unfreeze your target", AccountStatus::QuestTroupe, command_unfreeze) ||
		command_add("unmemspell", "[Spell ID] - Unmemorize a Spell by ID for you or your target", AccountStatus::Guide, command_unmemspell) ||
		command_add("unmemspells", " Unmemorize all spells for you or your target", AccountStatus::Guide, command_unmemspells) ||
		command_add("unscribespell", "[Spell ID] - Unscribe a spell from your or your target's spell book by Spell ID", AccountStatus::GMCoder, command_unscribespell) ||
		command_add("unscribespells", "Clear out your or your player target's spell book.", AccountStatus::GMCoder, command_unscribespells) ||
		command_add("untraindisc", "[Spell ID] - Untrain your or your target's discipline by Spell ID", AccountStatus::GMCoder, command_untraindisc) ||
		command_add("untraindiscs", "Untrains all disciplines from your target.", AccountStatus::GMCoder, command_untraindiscs) ||
		command_add("updatechecksum", "update client checksum", AccountStatus::GMImpossible, command_updatechecksum) ||
		command_add("uptime", "[zone server id] - Get uptime of worldserver, or zone server if argument provided", AccountStatus::Steward, command_uptime) ||
		command_add("version", "Display current version of EQEmu server", AccountStatus::Player, command_version) ||
		command_add("viewcurrencies", "View your or your target's currencies", AccountStatus::GMAdmin, command_viewcurrencies) ||
		command_add("viewnpctype", "[NPC ID] - Show stats for an NPC by NPC ID", AccountStatus::GMAdmin, command_viewnpctype) ||
		command_add("viewpetition", "[petition number] - View a petition", AccountStatus::ApprenticeGuide, command_viewpetition) ||
		command_add("viewrecipe", "[Recipe ID] - Show a recipe's entries", AccountStatus::GMAdmin, command_viewrecipe) ||
		command_add("viewzoneloot", "[item id] - Allows you to search a zone's loot for a specific item ID. (0 shows all loot in the zone)", AccountStatus::QuestTroupe, command_viewzoneloot) ||
		command_add("wc", "[wear slot] [material] - Sends an OP_WearChange for your target", AccountStatus::GMMgmt, command_wc) ||
		command_add("weather", "[0/1/2/3] (Off/Rain/Snow/Manual) - Change the weather", AccountStatus::QuestTroupe, command_weather) ||
		command_add("who", "[search]", AccountStatus::ApprenticeGuide, command_who) ||
		command_add("worldshutdown", "Shut down world and all zones", AccountStatus::GMMgmt, command_worldshutdown) ||
		command_add("wp", "[add|delete] [grid_id] [pause] [waypoint_id] [-h] - Add or delete a waypoint by grid ID. (-h to use current heading)", AccountStatus::GMAreas, command_wp) ||
		command_add("wpadd", "[pause] [-h] - Add your current location as a waypoint to your NPC target's AI path. (-h to use current heading)", AccountStatus::GMAreas, command_wpadd) ||
		command_add("wpinfo", "Show waypoint info about your NPC target", AccountStatus::GMAreas, command_wpinfo) ||
		command_add("worldwide", "Performs world-wide GM functions such as cast (can be extended for other commands). Use caution", AccountStatus::GMImpossible, command_worldwide) ||
		command_add("xtargets", "[New Max XTargets] - Show your or your target's XTargets and optionally set max XTargets.", AccountStatus::GMImpossible, command_xtargets) ||
		command_add("zclip", "[Minimum Clip] [Maximum Clip] [Fog Minimum Clip] [Fog Maximum Clip] [Permanent (0 = False, 1 = True)] - Change zone clipping", AccountStatus::QuestTroupe, command_zclip) ||
		command_add("zcolor", "[Red] [Green] [Blue] [Permanent (0 = False, 1 = True)] - Change sky color", AccountStatus::QuestTroupe, command_zcolor) ||
		command_add("zheader", "[Zone ID|Zone Short Name] [Version] - Load a zone header from the database", AccountStatus::QuestTroupe, command_zheader) ||
		command_add("zone", "[Zone ID|Zone Short Name] [X] [Y] [Z] - Teleport to specified Zone by ID or Short Name (coordinates are optional)", AccountStatus::Guide, command_zone) ||
		command_add("zonebootup", "[ZoneServerID] [shortname] - Make a zone server boot a specific zone", AccountStatus::GMLeadAdmin, command_zonebootup) ||
		command_add("zoneinstance", "[Instance ID] [X] [Y] [Z] - Teleport to specified Instance by ID (coordinates are optional)", AccountStatus::Guide, command_zone_instance) ||
		command_add("zonelock", "[List|Lock|Unlock] [Zone ID|Zone Short Name] - Set or get lock status of a Zone by ID or Short Name", AccountStatus::GMAdmin, command_zonelock) ||
		command_add("zoneshutdown", "[shortname] - Shut down a zone server", AccountStatus::GMLeadAdmin, command_zoneshutdown) ||
		command_add("zonestatus", "Show connected zoneservers, synonymous with /servers", AccountStatus::GMLeadAdmin, command_zonestatus) ||
		command_add("zopp", "Troubleshooting command - Sends a fake item packet to you. No server reference is created.", AccountStatus::GMImpossible, command_zopp) ||
		command_add("zsafecoords", "[X] [Y] [Z] [Heading] [Permanent (0 = False, 1 = True)] - Set the current zone's safe coordinates", AccountStatus::QuestTroupe, command_zsafecoords) ||
		command_add("zsave", " Saves zheader to the database", AccountStatus::QuestTroupe, command_zsave) ||
		command_add("zsky", "[Sky Type] [Permanent (0 = False, 1 = True)] - Change zone sky type", AccountStatus::QuestTroupe, command_zsky) ||
		command_add("zstats", "Show info about zone header", AccountStatus::QuestTroupe, command_zstats) ||
		command_add("zunderworld", "[Z] [Permanent (0 = False, 1 = True)] - Change zone underworld Z", AccountStatus::QuestTroupe, command_zunderworld)
	) {
		command_deinit();
		return -1;
	}

	std::map<std::string, std::pair<uint8, std::vector<std::string>>> command_settings;
	database.GetCommandSettings(command_settings);

	std::vector<std::pair<std::string, uint8>> injected_command_settings;
	std::vector<std::string> orphaned_command_settings;

	for (const auto& cs : command_settings) {
		auto cl = commandlist.find(cs.first);
		if (cl == commandlist.end()) {
			orphaned_command_settings.push_back(cs.first);
			LogInfo(
				"Command [{}] no longer exists... Deleting orphaned entry from `command_settings` table...",
				cs.first
			);
		}
	}

	if (orphaned_command_settings.size()) {
		if (!database.UpdateOrphanedCommandSettings(orphaned_command_settings)) {
			LogInfo("Failed to process 'Orphaned Commands' update operation.");
		}
	}

	auto working_cl = commandlist;
	for (const auto& w : working_cl) {
		auto cs = command_settings.find(w.first);
		if (cs == command_settings.end()) {
			injected_command_settings.push_back(std::pair<std::string, uint8>(w.first, w.second->admin));
			LogInfo(
				"New Command [{}] found... Adding to `command_settings` table with admin [{}]...",
				w.first,
				w.second->admin
			);

			if (w.second->admin == AccountStatus::Player) {
				LogCommands(
					"command_init(): Warning: Command [{}] defaulting to admin level 0!",
					w.first
				);
			}

			continue;
		}

		w.second->admin = cs->second.first;
		LogCommands(
			"command_init(): - Command [{}] set to admin level [{}]",
			w.first,
			cs->second.first
		);

		if (cs->second.second.empty()) {
			continue;
		}

		for (const auto& a : cs->second.second) {
			if (a.empty()) {
				continue;
			}

			if (commandlist.find(a) != commandlist.end()) {
				LogCommands(
					"command_init(): Warning: Alias [{}] already exists as a command - skipping!",
					a
				);

				continue;
			}

			commandlist[a] = w.second;
			commandaliases[a] = w.first;

			LogCommands(
				"command_init(): - Alias [{}] added to command [{}]",
				a,
				commandaliases[a]
			);
		}
	}

	if (injected_command_settings.size()) {
		if (!database.UpdateInjectedCommandSettings(injected_command_settings)) {
			LogInfo("Failed to process 'Injected Commands' update operation.");
		}
	}

	command_dispatch = command_realdispatch;

	return command_count;
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
	for (auto &c : command_delete_list) {
		delete c;
	}

	command_delete_list.clear();
	commandlist.clear();
	commandaliases.clear();

	command_dispatch = command_notavail;
	command_count = 0;
}

/*
 * command_add
 * adds a command to the command list; used by command_init
 *
 * Parameters:
 *	command_name - the command ex: "spawn"
 *	description - text description of command for #help
 *	admin - default admin level required to use command
 *	function - pointer to function that handles command
 *
 */
int command_add(std::string command_name, std::string description, uint8 admin, CmdFuncPtr function)
{
	if (command_name.empty()) {
		LogError("command_add() - Command added with empty name string - check command.cpp");
		return -1;
	}

	if (!function) {
		LogError("command_add() - Command [{}] added without a valid function pointer - check command.cpp", command_name);
		return -1;
	}

	if (commandlist.count(command_name)) {
		LogError("command_add() - Command [{}] is a duplicate command name - check command.cpp", command_name);
		return -1;
	}

	for (const auto& c : commandlist) {
		if (c.second->function != function) {
			continue;
		}

		LogError("command_add() - Command [{}] equates to an alias of [{}] - check command.cpp", command_name, c.first);
		return -1;
	}

	auto c = new CommandRecord;
	c->admin = admin;
	c->description = description;
	c->function = function;

	commands_map[command_name] = admin;

	commandlist[command_name] = c;
	commandaliases[command_name] = command_name;
	command_delete_list.push_back(c);
	command_count++;

	return 0;
}

uint8 GetCommandStatus(Client *c, std::string command_name) {
	auto command_status = commands_map[command_name];
	return command_status;
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
int command_realdispatch(Client *c, std::string message, bool ignore_status)
{
	Seperator sep(message.c_str(), ' ', 10, 100, true); // "three word argument" should be considered 1 arg

	command_logcommand(c, message.c_str());

	std::string cstr(sep.arg[0] + 1);

	if (commandlist.count(cstr) != 1) {
		return -2;
	}

	auto cur = commandlist[cstr];
	if (!ignore_status && c->Admin() < cur->admin) {
		c->Message(Chat::White, "Your status is not high enough to use this command.");
		return -1;
	}

	/* QS: Player_Log_Issued_Commands */
	if (RuleB(QueryServ, PlayerLogIssuedCommandes)) {
		auto event_desc = fmt::format(
			"Issued command :: '{}' in Zone ID: {} Instance ID: {}",
			message,
			c->GetZoneID(),
			c->GetInstanceID()
		);
		QServ->PlayerLogEvent(Player_Log_Issued_Commands, c->CharacterID(), event_desc);
	}

	if (cur->admin >= COMMANDS_LOGGING_MIN_STATUS) {
		LogCommands(
			"[{}] ([{}]) used command: [{}] (target=[{}])",
			c->GetName(),
			c->AccountName(),
			message,
			c->GetTarget() ? c->GetTarget()->GetName() : "NONE"
		);
	}

	if (!cur->function) {
		LogError("Command [{}] has a null function", cstr);
		return -1;
	}

	cur->function(c, &sep);	// Dispatch C++ Command

	return 0;
}

void command_help(Client *c, const Seperator *sep)
{
	int found_count = 0;
	std::string command_link;
	std::string search_criteria = Strings::ToLower(sep->argplus[1]);

	for (const auto& cur : commandlist) {
		if (!search_criteria.empty()) {
			if (cur.first.find(search_criteria) == std::string::npos) {
				continue;
			}
		}

		if (c->Admin() < cur.second->admin) {
			continue;
		}

		command_link = Saylink::Silent(
			fmt::format(
				"{}{}",
				COMMAND_CHAR,
				cur.first
			)
		);

		c->Message(
			Chat::White,
			fmt::format(
				"{} | Status: {} | {}",
				command_link,
				cur.second->admin,
				!cur.second->description.empty() ? cur.second->description : ""
			).c_str()
		);

		found_count++;
	}

	if (parse->PlayerHasQuestSub(EVENT_COMMAND)) {
		auto event_parse = parse->EventPlayer(EVENT_COMMAND, c, sep->msg, 0);
		if (event_parse >= 1) {
			found_count += event_parse;
		}
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Command{} listed{}.",
			found_count,
			found_count != 1 ? "s" : "",
			(
				!search_criteria.empty() ?
				fmt::format(
					" matching '{}'",
					search_criteria
				) :
				""
			)
		).c_str()
	);
}

void command_findaliases(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #findaliases [Search Critera]");
		return;
	}

	std::string search_criteria = Strings::ToLower(sep->argplus[1]);

	auto find_iter = commandaliases.find(search_criteria);
	if (find_iter == commandaliases.end()) {
		c->Message(
			Chat::White,
			fmt::format(
				"No commands or aliases found matching '{}'.",
				search_criteria
			).c_str()
		);
		return;
	}

	auto command_iter = commandlist.find(find_iter->second);
	if (
		find_iter->second.empty() ||
		command_iter == commandlist.end()
	) {
		c->Message(Chat::White, "An unknown condition occurred.");
		return;
	}

	auto current_commmand_link = Saylink::Silent(
		fmt::format(
			"{}{}",
			COMMAND_CHAR,
			command_iter->first
		)
	);

	int alias_count = 0;
	int alias_number = 1;
	std::string alias_link;
	for (const auto& a : commandaliases) {
		if (
			find_iter->second != a.second ||
			c->Admin() < command_iter->second->admin
		) {
			continue;
		}

		alias_link = Saylink::Silent(
			fmt::format(
				"{}{}",
				COMMAND_CHAR,
				a.first
			)
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Alias {} | {}",
				alias_number,
				alias_link
			).c_str()
		);

		alias_count++;
		alias_number++;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Alias{} listed for {}.",
			alias_count,
			alias_count != 1 ? "es" : "",
			current_commmand_link
		).c_str()
	);
}

void command_hotfix(Client *c, const Seperator *sep)
{
	auto items_count = database.GetItemsCount();
	auto shared_items_count = database.GetSharedItemsCount();
	if (items_count != shared_items_count) {
		c->Message(Chat::Yellow, "Your database does not have the same item count as your shared memory.");

		c->Message(
			Chat::Yellow,
			fmt::format(
				"Database Count: {} Shared Memory Count: {}",
				items_count,
				shared_items_count
			).c_str()
		);

		c->Message(Chat::Yellow, "If you want to be able to add new items to your server while it is online, you need to create placeholder entries in the database ahead of time and do not add or remove rows/entries. Only modify the existing placeholder rows/entries to safely use #hotfix.");

		return;
	}

	auto spells_count = database.GetSpellsCount();
	auto shared_spells_count = database.GetSharedSpellsCount();
	if (spells_count != shared_spells_count) {
		c->Message(Chat::Yellow, "Your database does not have the same spell count as your shared memory.");

		c->Message(
			Chat::Yellow,
			fmt::format(
				"Database Count: {} Shared Memory Count: {}",
				spells_count,
				shared_spells_count
			).c_str()
		);

		c->Message(Chat::Yellow, "If you want to be able to add new spells to your server while it is online, you need to create placeholder entries in the database ahead of time and do not add or remove rows/entries. Only modify the existing placeholder rows/entries to safely use #hotfix.");

		c->Message(Chat::Yellow, "Note: You may still have to distribute a spell file, even with dynamic changes.");

		return;
	}

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
			if (File::Exists("bin/shared_memory.exe")) {
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
			if (File::Exists("./bin/shared_memory")) {
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

#include "gm_commands/acceptrules.cpp"
#include "gm_commands/advnpcspawn.cpp"
#include "gm_commands/aggro.cpp"
#include "gm_commands/aggrozone.cpp"
#include "gm_commands/ai.cpp"
#include "gm_commands/appearance.cpp"
#include "gm_commands/appearanceeffects.cpp"
#include "gm_commands/attack.cpp"
#include "gm_commands/augmentitem.cpp"
#include "gm_commands/ban.cpp"
#include "gm_commands/bind.cpp"
#include "gm_commands/bugs.cpp"
#include "gm_commands/camerashake.cpp"
#include "gm_commands/castspell.cpp"
#include "gm_commands/chat.cpp"
#include "gm_commands/checklos.cpp"
#include "gm_commands/copycharacter.cpp"
#include "gm_commands/corpse.cpp"
#include "gm_commands/corpsefix.cpp"
#include "gm_commands/countitem.cpp"
#include "gm_commands/cvs.cpp"
#include "gm_commands/damage.cpp"
#include "gm_commands/databuckets.cpp"
#include "gm_commands/date.cpp"
#include "gm_commands/dbspawn2.cpp"
#include "gm_commands/delacct.cpp"
#include "gm_commands/delpetition.cpp"
#include "gm_commands/depop.cpp"
#include "gm_commands/depopzone.cpp"
#include "gm_commands/devtools.cpp"
#include "gm_commands/disablerecipe.cpp"
#include "gm_commands/disarmtrap.cpp"
#include "gm_commands/distance.cpp"
#include "gm_commands/doanim.cpp"
#include "gm_commands/door.cpp"
#include "gm_commands/door_manipulation.cpp"
#include "gm_commands/door_manipulation.h"
#include "gm_commands/dye.cpp"
#include "gm_commands/dz.cpp"
#include "gm_commands/dzkickplayers.cpp"
#include "gm_commands/editmassrespawn.cpp"
#include "gm_commands/emote.cpp"
#include "gm_commands/emotesearch.cpp"
#include "gm_commands/emoteview.cpp"
#include "gm_commands/emptyinventory.cpp"
#include "gm_commands/enablerecipe.cpp"
#include "gm_commands/endurance.cpp"
#include "gm_commands/equipitem.cpp"
#include "gm_commands/faction.cpp"
#include "gm_commands/feature.cpp"
#include "gm_commands/findaa.cpp"
#include "gm_commands/findclass.cpp"
#include "gm_commands/findfaction.cpp"
#include "gm_commands/findnpctype.cpp"
#include "gm_commands/findrace.cpp"
#include "gm_commands/findrecipe.cpp"
#include "gm_commands/findskill.cpp"
#include "gm_commands/findspell.cpp"
#include "gm_commands/findtask.cpp"
#include "gm_commands/findzone.cpp"
#include "gm_commands/fixmob.cpp"
#include "gm_commands/flag.cpp"
#include "gm_commands/flagedit.cpp"
#include "gm_commands/flags.cpp"
#include "gm_commands/flymode.cpp"
#include "gm_commands/fov.cpp"
#include "gm_commands/freeze.cpp"
#include "gm_commands/gassign.cpp"
#include "gm_commands/gearup.cpp"
#include "gm_commands/gender.cpp"
#include "gm_commands/getplayerburiedcorpsecount.cpp"
#include "gm_commands/getvariable.cpp"
#include "gm_commands/ginfo.cpp"
#include "gm_commands/giveitem.cpp"
#include "gm_commands/givemoney.cpp"
#include "gm_commands/globalview.cpp"
#include "gm_commands/gm.cpp"
#include "gm_commands/gmspeed.cpp"
#include "gm_commands/gmzone.cpp"
#include "gm_commands/godmode.cpp"
#include "gm_commands/goto.cpp"
#include "gm_commands/grid.cpp"
#include "gm_commands/guild.cpp"
#include "gm_commands/guildapprove.cpp"
#include "gm_commands/guildcreate.cpp"
#include "gm_commands/guildlist.cpp"
#include "gm_commands/haste.cpp"
#include "gm_commands/hatelist.cpp"
#include "gm_commands/heal.cpp"
#include "gm_commands/heromodel.cpp"
#include "gm_commands/hideme.cpp"
#include "gm_commands/hp.cpp"
#include "gm_commands/incstat.cpp"
#include "gm_commands/instance.cpp"
#include "gm_commands/interrogateinv.cpp"
#include "gm_commands/interrupt.cpp"
#include "gm_commands/invsnapshot.cpp"
#include "gm_commands/invul.cpp"
#include "gm_commands/ipban.cpp"
#include "gm_commands/iplookup.cpp"
#include "gm_commands/itemsearch.cpp"
#include "gm_commands/kick.cpp"
#include "gm_commands/kill.cpp"
#include "gm_commands/killallnpcs.cpp"
#include "gm_commands/lastname.cpp"
#include "gm_commands/level.cpp"
#include "gm_commands/list.cpp"
#include "gm_commands/listpetition.cpp"
#include "gm_commands/lootsim.cpp"
#include "gm_commands/loc.cpp"
#include "gm_commands/logcommand.cpp"
#include "gm_commands/logs.cpp"
#include "gm_commands/makepet.cpp"
#include "gm_commands/mana.cpp"
#include "gm_commands/max_all_skills.cpp"
#include "gm_commands/memspell.cpp"
#include "gm_commands/merchantcloseshop.cpp"
#include "gm_commands/merchantopenshop.cpp"
#include "gm_commands/modifynpcstat.cpp"
#include "gm_commands/motd.cpp"
#include "gm_commands/movechar.cpp"
#include "gm_commands/movement.cpp"
#include "gm_commands/myskills.cpp"
#include "gm_commands/mysql.cpp"
#include "gm_commands/mystats.cpp"
#include "gm_commands/name.cpp"
#include "gm_commands/netstats.cpp"
#include "gm_commands/network.cpp"
#include "gm_commands/npccast.cpp"
#include "gm_commands/npcedit.cpp"
#include "gm_commands/npceditmass.cpp"
#include "gm_commands/npcemote.cpp"
#include "gm_commands/npcloot.cpp"
#include "gm_commands/npcsay.cpp"
#include "gm_commands/npcshout.cpp"
#include "gm_commands/npcspawn.cpp"
#include "gm_commands/npcstats.cpp"
#include "gm_commands/npctypespawn.cpp"
#include "gm_commands/nudge.cpp"
#include "gm_commands/nukebuffs.cpp"
#include "gm_commands/nukeitem.cpp"
#include "gm_commands/object.cpp"
#include "gm_commands/oocmute.cpp"
#include "gm_commands/opcode.cpp"
#include "gm_commands/path.cpp"
#include "gm_commands/peekinv.cpp"
#include "gm_commands/peqzone.cpp"
#include "gm_commands/peqzone_flags.cpp"
#include "gm_commands/permaclass.cpp"
#include "gm_commands/permagender.cpp"
#include "gm_commands/permarace.cpp"
#include "gm_commands/petitems.cpp"
#include "gm_commands/petitioninfo.cpp"
#include "gm_commands/petname.cpp"
#include "gm_commands/picklock.cpp"
#include "gm_commands/profanity.cpp"
#include "gm_commands/proximity.cpp"
#include "gm_commands/push.cpp"
#include "gm_commands/pvp.cpp"
#include "gm_commands/qglobal.cpp"
#include "gm_commands/questerrors.cpp"
#include "gm_commands/race.cpp"
#include "gm_commands/raidloot.cpp"
#include "gm_commands/randomfeatures.cpp"
#include "gm_commands/refreshgroup.cpp"
#include "gm_commands/reload.cpp"
#include "gm_commands/removeitem.cpp"
#include "gm_commands/repop.cpp"
#include "gm_commands/resetaa.cpp"
#include "gm_commands/resetaa_timer.cpp"
#include "gm_commands/resetdisc_timer.cpp"
#include "gm_commands/revoke.cpp"
#include "gm_commands/roambox.cpp"
#include "gm_commands/rules.cpp"
#include "gm_commands/save.cpp"
#include "gm_commands/scale.cpp"
#include "gm_commands/scribespell.cpp"
#include "gm_commands/scribespells.cpp"
#include "gm_commands/sendzonespawns.cpp"
#include "gm_commands/sensetrap.cpp"
#include "gm_commands/serverinfo.cpp"
#include "gm_commands/serverlock.cpp"
#include "gm_commands/serverrules.cpp"
#include "gm_commands/set_adventure_points.cpp"
#include "gm_commands/setaapts.cpp"
#include "gm_commands/setaaxp.cpp"
#include "gm_commands/setaltcurrency.cpp"
#include "gm_commands/setanim.cpp"
#include "gm_commands/setcrystals.cpp"
#include "gm_commands/setendurance.cpp"
#include "gm_commands/setfaction.cpp"
#include "gm_commands/sethp.cpp"
#include "gm_commands/setlanguage.cpp"
#include "gm_commands/setlsinfo.cpp"
#include "gm_commands/setmana.cpp"
#include "gm_commands/setpass.cpp"
#include "gm_commands/setpvppoints.cpp"
#include "gm_commands/setskill.cpp"
#include "gm_commands/setskillall.cpp"
#include "gm_commands/setstartzone.cpp"
#include "gm_commands/setstat.cpp"
#include "gm_commands/setxp.cpp"
#include "gm_commands/showbonusstats.cpp"
#include "gm_commands/showbuffs.cpp"
#include "gm_commands/shownpcgloballoot.cpp"
#include "gm_commands/shownumhits.cpp"
#include "gm_commands/showskills.cpp"
#include "gm_commands/showspellslist.cpp"
#include "gm_commands/showstats.cpp"
#include "gm_commands/showzonegloballoot.cpp"
#include "gm_commands/showzonepoints.cpp"
#include "gm_commands/shutdown.cpp"
#include "gm_commands/spawn.cpp"
#include "gm_commands/spawneditmass.cpp"
#include "gm_commands/spawnfix.cpp"
#include "gm_commands/spawnstatus.cpp"
#include "gm_commands/spellinfo.cpp"
#include "gm_commands/faction_association.cpp"
#include "gm_commands/stun.cpp"
#include "gm_commands/summon.cpp"
#include "gm_commands/summonburiedplayercorpse.cpp"
#include "gm_commands/summonitem.cpp"
#include "gm_commands/suspend.cpp"
#include "gm_commands/suspendmulti.cpp"
#include "gm_commands/task.cpp"
#include "gm_commands/tempname.cpp"
#include "gm_commands/texture.cpp"
#include "gm_commands/time.cpp"
#include "gm_commands/timers.cpp"
#include "gm_commands/timezone.cpp"
#include "gm_commands/title.cpp"
#include "gm_commands/titlesuffix.cpp"
#include "gm_commands/traindisc.cpp"
#include "gm_commands/trapinfo.cpp"
#include "gm_commands/tune.cpp"
#include "gm_commands/undye.cpp"
#include "gm_commands/undyeme.cpp"
#include "gm_commands/unfreeze.cpp"
#include "gm_commands/unmemspell.cpp"
#include "gm_commands/unmemspells.cpp"
#include "gm_commands/unscribespell.cpp"
#include "gm_commands/unscribespells.cpp"
#include "gm_commands/updatechecksum.cpp"
#include "gm_commands/untraindisc.cpp"
#include "gm_commands/untraindiscs.cpp"
#include "gm_commands/uptime.cpp"
#include "gm_commands/version.cpp"
#include "gm_commands/viewcurrencies.cpp"
#include "gm_commands/viewnpctype.cpp"
#include "gm_commands/viewpetition.cpp"
#include "gm_commands/viewrecipe.cpp"
#include "gm_commands/viewzoneloot.cpp"
#include "gm_commands/wc.cpp"
#include "gm_commands/weather.cpp"
#include "gm_commands/who.cpp"
#include "gm_commands/worldshutdown.cpp"
#include "gm_commands/worldwide.cpp"
#include "gm_commands/wp.cpp"
#include "gm_commands/wpadd.cpp"
#include "gm_commands/wpinfo.cpp"
#include "gm_commands/xtargets.cpp"
#include "gm_commands/zclip.cpp"
#include "gm_commands/zcolor.cpp"
#include "gm_commands/zheader.cpp"
#include "gm_commands/zone.cpp"
#include "gm_commands/zonebootup.cpp"
#include "gm_commands/zonelock.cpp"
#include "gm_commands/zoneshutdown.cpp"
#include "gm_commands/zonestatus.cpp"
#include "gm_commands/zone_instance.cpp"
#include "gm_commands/zopp.cpp"
#include "gm_commands/zsafecoords.cpp"
#include "gm_commands/zsave.cpp"
#include "gm_commands/zsky.cpp"
#include "gm_commands/zstats.cpp"
#include "gm_commands/zunderworld.cpp"
