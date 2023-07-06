#include "../client.h"
#include "show/buffs.cpp"
#include "show/buried_corpse_count.cpp"
#include "show/client_version_summary.cpp"
#include "show/currencies.cpp"
#include "show/distance.cpp"
#include "show/emotes.cpp"
#include "show/field_of_view.cpp"
#include "show/flags.cpp"
#include "show/group_info.cpp"
#include "show/hatelist.cpp"
#include "show/inventory.cpp"
#include "show/ip_lookup.cpp"
#include "show/line_of_sight.cpp"
#include "show/network.cpp"
#include "show/network_stats.cpp"
#include "show/npc_global_loot.cpp"
#include "show/npc_stats.cpp"
#include "show/npc_type.cpp"
#include "show/peqzone_flags.cpp"
#include "show/petition.cpp"
#include "show/petition_info.cpp"
#include "show/proximity.cpp"
#include "show/quest_errors.cpp"
#include "show/quest_globals.cpp"
#include "show/recipe.cpp"
#include "show/server_info.cpp"
#include "show/skills.cpp"
#include "show/spawn_status.cpp"
#include "show/spells.cpp"
#include "show/spells_list.cpp"
#include "show/stats.cpp"
#include "show/timers.cpp"
#include "show/traps.cpp"
#include "show/uptime.cpp"
#include "show/variable.cpp"
#include "show/version.cpp"
#include "show/waypoints.cpp"
#include "show/xtargets.cpp"
#include "show/zone_data.cpp"
#include "show/zone_global_loot.cpp"
#include "show/zone_loot.cpp"
#include "show/zone_points.cpp"
#include "show/zone_status.cpp"

void command_show(Client *c, const Seperator *sep)
{
	struct Cmd {
		std::string cmd{}; // command
		std::string u{}; // usage
		void (*fn)(Client *c, const Seperator *sep) = nullptr; // function
		std::vector<std::string> a{}; // aliases
	};

	std::vector<Cmd> commands = {
		Cmd{.cmd = "buffs", .u = "buffs", .fn = ShowBuffs, .a = {"#showbuffs"}},
		Cmd{.cmd = "buried_corpse_count", .u = "buried_corpse_count", .fn = ShowBuriedCorpseCount, .a = {"#getplayerburiedcorpsecount"}},
		Cmd{.cmd = "client_version_summary", .u = "client_version_summary", .fn = ShowClientVersionSummary, .a = {"#cvs"}},
		Cmd{.cmd = "currencies", .u = "currencies", .fn = ShowCurrencies, .a = {"#viewcurrencies"}},
		Cmd{.cmd = "distance", .u = "distance", .fn = ShowDistance, .a = {"#distance"}},
		Cmd{.cmd = "emotes", .u = "emotes", .fn = ShowEmotes, .a = {"#emoteview"}},
		Cmd{.cmd = "field_of_view", .u = "field_of_view", .fn = ShowFieldOfView, .a = {"#fov"}},
		Cmd{.cmd = "flags", .u = "flags", .fn = ShowFlags, .a = {"#flags"}},
		Cmd{.cmd = "group_info", .u = "group_info", .fn = ShowGroupInfo, .a = {"#ginfo"}},
		Cmd{.cmd = "hatelist", .u = "hatelist", .fn = ShowHateList, .a = {"#hatelist"}},
		Cmd{.cmd = "inventory", .u = "inventory", .fn = ShowInventory, .a = {"#peekinv"}},
		Cmd{.cmd = "ip_lookup", .u = "ip_lookup", .fn = ShowIPLookup, .a = {"#iplookup"}},
		Cmd{.cmd = "line_of_sight", .u = "line_of_sight", .fn = ShowLineOfSight, .a = {"#checklos"}},
		Cmd{.cmd = "network", .u = "network", .fn = ShowNetwork, .a = {"#network"}},
		Cmd{.cmd = "network_stats", .u = "network_stats", .fn = ShowNetworkStats, .a = {"#netstats"}},
		Cmd{.cmd = "npc_global_loot", .u = "npc_global_loot", .fn = ShowNPCGlobalLoot, .a = {"#shownpcgloballoot"}},
		Cmd{.cmd = "npc_stats", .u = "npc_stats", .fn = ShowNPCStats, .a = {"#npcstats"}},
		Cmd{.cmd = "npc_type", .u = "npc_type [NPC ID]", .fn = ShowNPCType, .a = {"#viewnpctype"}},
		Cmd{.cmd = "peqzone_flags", .u = "peqzone_flags", .fn = ShowPEQZoneFlags, .a = {"#peqzone_flags"}},
		Cmd{.cmd = "petition", .u = "petition", .fn = ShowPetition, .a = {"#listpetition", "#viewpetition"}},
		Cmd{.cmd = "petition_info", .u = "petition_info", .fn = ShowPetitionInfo, .a = {"#petitioninfo"}},
		Cmd{.cmd = "proximity", .u = "proximity", .fn = ShowProximity, .a = {"#proximity"}},
		Cmd{.cmd = "quest_errors", .u = "quest_errors", .fn = ShowQuestErrors, .a = {"#questerrors"}},
		Cmd{.cmd = "quest_globals", .u = "quest_globals", .fn = ShowQuestGlobals, .a = {"#globalview"}},
		Cmd{.cmd = "recipe", .u = "recipe [Recipe ID]", .fn = ShowRecipe, .a = {"#viewrecipe"}},
		Cmd{.cmd = "server_info", .u = "server_info", .fn = ShowServerInfo, .a = {"#serverinfo"}},
		Cmd{.cmd = "skills", .u = "skills", .fn = ShowSkills, .a = {"#showskills"}},
		Cmd{.cmd = "spawn_status", .u = "spawn_status [all|disabled|enabled|Spawn ID]", .fn = ShowSpawnStatus, .a = {"#spawnstatus"}},
		Cmd{.cmd = "spells", .u = "spells [disciplines|spells]", .fn = ShowSpells, .a = {"#showspells"}},
		Cmd{.cmd = "spells_list", .u = "spells_list", .fn = ShowSpellsList, .a = {"#showspellslist"}},
		Cmd{.cmd = "stats", .u = "stats", .fn = ShowStats, .a = {"#showstats"}},
		Cmd{.cmd = "timers", .u = "timers", .fn = ShowTimers, .a = {"#timers"}},
		Cmd{.cmd = "traps", .u = "traps", .fn = ShowTraps, .a = {"#trapinfo"}},
		Cmd{.cmd = "uptime", .u = "uptime [Zone Server ID] (Zone Server ID is optional)", .fn = ShowUptime, .a = {"#uptime"}},
		Cmd{.cmd = "variable", .u = "variable [Variable Name]", .fn = ShowVariable, .a = {"#getvariable"}},
		Cmd{.cmd = "version", .u = "version", .fn = ShowVersion, .a = {"#version"}},
		Cmd{.cmd = "waypoints", .u = "waypoints", .fn = ShowWaypoints, .a = {"#wpinfo"}},
		Cmd{.cmd = "xtargets", .u = "xtargets [Amount] (Amount is optional)", .fn = ShowXTargets, .a = {"#xtargets"}},
		Cmd{.cmd = "zone_data", .u = "zone_data", .fn = ShowZoneData, .a = {"#zstats"}},
		Cmd{.cmd = "zone_global_loot", .u = "zone_global_loot", .fn = ShowZoneGlobalLoot, .a = {"#showzonegloballoot"}},
		Cmd{.cmd = "zone_loot", .u = "zone_loot", .fn = ShowZoneLoot, .a = {"#viewzoneloot"}},
		Cmd{.cmd = "zone_points", .u = "zone_points", .fn = ShowZonePoints, .a = {"#showzonepoints"}},
		Cmd{.cmd = "zone_status", .u = "zone_status", .fn = ShowZoneStatus, .a = {"#zonestatus"}},
	};

	// Check for arguments
	const auto arguments = sep->argnum;

	// look for alias or command
	for (const auto &cmd: commands) {
		// Check for alias first
		for (const auto &alias: cmd.a) {
			if (!alias.empty() && Strings::EqualFold(alias, sep->arg[0])) {
				// build string from sep args
				std::vector<std::string> args = {};

				// skip the first arg
				for (auto i = 1; i <= arguments; i++) {
					args.emplace_back(sep->arg[i]);
				}

				// build the rewrite string
				const std::string& rewrite = fmt::format("#show {} {}", cmd.cmd, Strings::Join(args, " "));

				// rewrite to #show <sub-command <args>
				c->SendGMCommand(rewrite);
				return;
			}
		}

		// Check for command
		if (cmd.cmd == Strings::ToLower(sep->arg[1])) {
			cmd.fn(c, sep);
			return;
		}
	}

	// Command not found
	c->Message(Chat::White, "Command not found. Usage: #show [command]");
	for (const auto &cmd: commands) {
		c->Message(Chat::White, fmt::format("Usage: #show {}", cmd.u).c_str());
	}
}
