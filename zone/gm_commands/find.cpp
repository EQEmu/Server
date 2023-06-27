#include "../client.h"
#include "find/aa.cpp"
#include "find/character.cpp"
#include "find/class.cpp"
#include "find/currency.cpp"
#include "find/deity.cpp"
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
	struct FindCommand {
		std::string command{};
		std::string usage{};
		void (*function)(Client *c, const Seperator *sep) = nullptr;
		std::vector<std::string> aliases{};
	};

	std::vector<FindCommand> commands = {
		FindCommand{.command = "aa", .usage = "aa [Search Criteria]", .function = FindAA, .aliases = {"#findaa"}},
		FindCommand{.command = "character", .usage = "character [Search Criteria]", .function = FindCharacter, .aliases = {"#findcharacter"}},
		FindCommand{.command = "class", .usage = "class [Search Criteria]", .function = FindClass, .aliases = {"#findclass"}},
		FindCommand{.command = "currency", .usage = "currency [Search Criteria]", .function = FindCurrency, .aliases = {"#findcurrency"}},
		FindCommand{.command = "deity", .usage = "deity [Search Criteria]", .function = FindDeity, .aliases = {"#finddeity"}},
		FindCommand{.command = "faction", .usage = "faction [Search Criteria]", .function = FindFaction, .aliases = {"#findfaction"}},
		FindCommand{.command = "item", .usage = "item [Search Criteria]", .function = FindItem, .aliases = {"#fi", "#finditem"}},
		FindCommand{.command = "language", .usage = "language [Search Criteria]", .function = FindLanguage, .aliases = {"#findlanguage"}},
		FindCommand{.command = "npctype", .usage = "npctype [Search Criteria]", .function = FindNPCType, .aliases = {"#fn", "#findnpc", "#findnpctype"}},
		FindCommand{.command = "race", .usage = "race [Search Criteria]", .function = FindRace, .aliases = {"#findrace"}},
		FindCommand{.command = "recipe", .usage = "recipe [Search Criteria]", .function = FindRecipe, .aliases = {"#findrecipe"}},
		FindCommand{.command = "skill", .usage = "skill [Search Criteria]", .function = FindSkill, .aliases = {"#findskill"}},
		FindCommand{.command = "spell", .usage = "spell [Search Criteria]", .function = FindSpell, .aliases = {"#fs", "#findspell"}},
		FindCommand{.command = "task", .usage = "task [Search Criteria]", .function = FindTask, .aliases = {"#findtask"}},
		FindCommand{.command = "zone", .usage = "zone [Search Criteria]", .function = FindZone, .aliases = {"#fz", "#findzone"}},
	};

	// Check for arguments
	const auto arguments = sep->argnum;
	if (!arguments) {
		for (const auto &cmd: commands) {
			c->Message(Chat::White, fmt::format("Usage: #find {}", cmd.usage).c_str());
		}
		return;
	}

	// look for alias or command
	for (const auto &cmd: commands) {
		// Check for alias first
		for (const auto &alias: cmd.aliases) {
			if (!alias.empty() && alias == Strings::ToLower(sep->arg[0])) {
				// build string from sep args
				std::vector<std::string> args = {};

				// skip the first arg
				for (auto i = 1; i <= arguments; i++) {
					args.emplace_back(sep->arg[i]);
				}

				// build the rewrite string
				std::string rewrite = fmt::format("#find {} {}", cmd.command, Strings::Join(args, " "));

				// rewrite to #find <sub-command <args>
				c->SendGMCommand(rewrite);
				return;
			}
		}

		// Check for command
		if (cmd.command == Strings::ToLower(sep->arg[1])) {
			cmd.function(c, sep);
			return;
		}
	}

	// Command not found
	c->Message(Chat::White, "Command not found. Usage: #find [command]");
	for (const auto &cmd: commands) {
		c->Message(Chat::White, fmt::format("Usage: #find {}", cmd.usage).c_str());
	}
}
