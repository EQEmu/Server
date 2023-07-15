#include "../client.h"
#include "find/aa.cpp"
#include "find/character.cpp"
#include "find/class.cpp"
#include "find/currency.cpp"
#include "find/deity.cpp"
#include "find/emote.cpp"
#include "find/faction.cpp"
#include "find/item.cpp"
#include "find/language.cpp"
#include "find/npctype.cpp"
#include "find/race.cpp"
#include "find/recipe.cpp"
#include "find/skill.cpp"
#include "find/spell.cpp"
#include "find/task.cpp"
#include "find/zone.cpp"

void command_find(Client *c, const Seperator *sep)
{
	// Cmd represents a command
	// variables are short to save space horizontally
	// when adding a new sub-command, add it to the vector below
	struct Cmd {
		std::string cmd{}; // command
		std::string u{}; // usage
		void (*fn)(Client *c, const Seperator *sep) = nullptr; // function
		std::vector<std::string> a{}; // aliases
	};

	std::vector<Cmd> commands = {
		Cmd{.cmd = "aa", .u = "aa [Search Criteria]", .fn = FindAA, .a = {"#findaa"}},
		Cmd{.cmd = "character", .u = "character [Search Criteria]", .fn = FindCharacter, .a = {"#findcharacter"}},
		Cmd{.cmd = "class", .u = "class [Search Criteria]", .fn = FindClass, .a = {"#findclass"}},
		Cmd{.cmd = "currency", .u = "currency [Search Criteria]", .fn = FindCurrency, .a = {"#findcurrency"}},
		Cmd{.cmd = "deity", .u = "deity [Search Criteria]", .fn = FindDeity, .a = {"#finddeity"}},
		Cmd{.cmd = "emote", .u = "emote [Search Criteria]", .fn = FindEmote, .a = {"#findemote"}},
		Cmd{.cmd = "faction", .u = "faction [Search Criteria]", .fn = FindFaction, .a = {"#findfaction"}},
		Cmd{.cmd = "item", .u = "item [Search Criteria]", .fn = FindItem, .a = {"#fi", "#finditem"}},
		Cmd{.cmd = "language", .u = "language [Search Criteria]", .fn = FindLanguage, .a = {"#findlanguage"}},
		Cmd{
			.cmd = "npctype", .u = "npctype [Search Criteria]", .fn = FindNPCType, .a = {
				"#fn",
				"#findnpc",
				"#findnpctype"
			}
		},
		Cmd{.cmd = "race", .u = "race [Search Criteria]", .fn = FindRace, .a = {"#findrace"}},
		Cmd{.cmd = "recipe", .u = "recipe [Search Criteria]", .fn = FindRecipe, .a = {"#findrecipe"}},
		Cmd{.cmd = "skill", .u = "skill [Search Criteria]", .fn = FindSkill, .a = {"#findskill"}},
		Cmd{.cmd = "spell", .u = "spell [Search Criteria]", .fn = FindSpell, .a = {"#fs", "#findspell"}},
		Cmd{.cmd = "task", .u = "task [Search Criteria]", .fn = FindTask, .a = {"#findtask"}},
		Cmd{.cmd = "zone", .u = "zone [Search Criteria]", .fn = FindZone, .a = {"#fz", "#findzone"}},
	};

	// Check for arguments
	const auto arguments = sep->argnum;
	if (!arguments) {
		for (const auto &cmd: commands) {
			c->Message(Chat::White, fmt::format("Usage: #find {}", cmd.u).c_str());
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
				std::string rewrite = fmt::format("#find {} {}", cmd.cmd, Strings::Join(args, " "));

				// rewrite to #find <sub-command <args>>
				c->SendGMCommand(rewrite);

				c->Message(
					Chat::Gray,
					fmt::format(
						"{} is now located under {}, using {}.",
						sep->arg[0],
						Saylink::Silent("#find"),
						Saylink::Silent(rewrite)
					).c_str()
				);

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
	c->Message(Chat::White, "Command not found. Usage: #find [command]");
	for (const auto &cmd: commands) {
		c->Message(Chat::White, fmt::format("Usage: #find {}", cmd.u).c_str());
	}
}
