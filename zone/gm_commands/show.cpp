#include "../client.h"
#include "show/buffs.cpp"
#include "show/npc_global_loot.cpp"
#include "show/skills.cpp"
#include "show/spells.cpp"
#include "show/spells_list.cpp"
#include "show/stats.cpp"
#include "show/zone_global_loot.cpp"
#include "show/zone_points.cpp"

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
		Cmd{.cmd = "npc_global_loot", .u = "npc_global_loot", .fn = ShowNPCGlobalLoot, .a = {"#shownpcgloballoot"}},
		Cmd{.cmd = "skills", .u = "skills", .fn = ShowSkills, .a = {"#showskills"}},
		Cmd{.cmd = "spells", .u = "spells [disciplines|spells]", .fn = ShowSpells, .a = {"#showspells"}},
		Cmd{.cmd = "spells_list", .u = "spells_list", .fn = ShowSpellsList, .a = {"#showspellslist"}},
		Cmd{.cmd = "stats", .u = "stats", .fn = ShowStats, .a = {"#showstats"}},
		Cmd{.cmd = "zone_global_loot", .u = "zone_global_loot", .fn = ShowZoneGlobalLoot, .a = {"#showzonegloballoot"}},
		Cmd{.cmd = "zone_points", .u = "zone_points", .fn = ShowZonePoints, .a = {"#showzonepoints"}},
	};

	// Check for arguments
	const auto arguments = sep->argnum;
	if (!arguments && Strings::EqualFold(sep->arg[0], "spells")) {
		for (const auto &cmd: commands) {
			c->Message(Chat::White, fmt::format("Usage: #show {}", cmd.u).c_str());
		}
		return;
	}

	// look for alias or command
	for (const auto &cmd: commands) {
		// Check for alias first
		for (const auto &alias: cmd.a) {
			if (!alias.empty() && alias == Strings::ToLower(sep->arg[0])) {
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

