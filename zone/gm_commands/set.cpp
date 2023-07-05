#include "../client.h"
#include "set/aa_exp.cpp"
#include "set/aa_points.cpp"
#include "set/adventure_points.cpp"
#include "set/alternate_currency.cpp"
#include "set/animation.cpp"
#include "set/anon.cpp"
#include "set/crystals.cpp"
#include "set/endurance.cpp"
#include "set/endurance_full.cpp"
#include "set/exp.cpp"
#include "set/faction.cpp"
#include "set/hp.cpp"
#include "set/hp_full.cpp"
#include "set/language.cpp"
#include "set/loginserver_info.cpp"
#include "set/mana.cpp"
#include "set/mana_full.cpp"
#include "set/password.cpp"
#include "set/pvp_points.cpp"
#include "set/skill.cpp"
#include "set/skill_all.cpp"
#include "set/start_zone.cpp"
#include "set/stat.cpp"

void command_set(Client *c, const Seperator *sep)
{
	struct Cmd {
		std::string cmd{}; // command
		std::string u{}; // usage
		void (*fn)(Client *c, const Seperator *sep) = nullptr; // function
		std::vector<std::string> a{}; // aliases
	};

	std::vector<Cmd> commands = {
		Cmd{.cmd = "aa_exp", .u = "aa_exp [aa|group|raid] [Amount]", .fn = SetAAEXP, .a = {"#setaaxp"}},
		Cmd{.cmd = "aa_points", .u = "aa_points [aa|group|raid] [Amount]", .fn = SetAAPoints, .a = {"#setaapts"}},
		Cmd{.cmd = "adventure_points", .u = "adventure_points [Theme ID] [Amount]", .fn = SetAdventurePoints, .a = {"#set_adventure_points"}},
		Cmd{.cmd = "alternate_currency", .u = "alternate_currency [Currency ID] [Amount]", .fn = SetAlternateCurrency, .a = {"#setaltcurrency"}},
		Cmd{.cmd = "animation", .u = "animation [Animation ID]", .fn = SetAnimation, .a = {"#setanim"}},
		Cmd{.cmd = "anonymous", .u = "anon [Character ID] [Anonymous Flag] or #set anon [Anonymous Flag]", .fn = SetAnon, .a = {"#setanon"}},
		Cmd{.cmd = "crystals", .u = "crystals [ebon|radiant] [Amount]", .fn = SetCrystals, .a = {"#setcrystals"}},
		Cmd{.cmd = "endurance", .u = "endurance [Amount]", .fn = SetEndurance, .a = {"#setendurance"}},
		Cmd{.cmd = "endurance_full", .u = "endurance_full", .fn = SetEnduranceFull, .a = {"#endurance"}},
		Cmd{.cmd = "faction", .u = "faction [Faction ID]", .fn = SetFaction, .a = {"#setfaction"}},
		Cmd{.cmd = "hp", .u = "hp [Amount]", .fn = SetHP, .a = {"#sethp"}},
		Cmd{.cmd = "hp_full", .u = "hp_full", .fn = SetHPFull, .a = {"#heal"}},
		Cmd{.cmd = "language", .u = "language [Language ID] [Language Level]", .fn = SetLanguage, .a = {"#setlanguage"}},
		Cmd{.cmd = "loginserver_info", .u = "loginserver_info [Email] [Password]", .fn = SetLoginserverInfo, .a = {"#setlsinfo"}},
		Cmd{.cmd = "mana", .u = "mana [Amount]", .fn = SetMana, .a = {"#setmana"}},
		Cmd{.cmd = "mana_full", .u = "mana_full", .fn = SetManaFull, .a = {"#mana"}},
		Cmd{.cmd = "password", .u = "password [Account Name] [Password]", .fn = SetPassword, .a = {"#setpass"}},
		Cmd{.cmd = "pvp_points", .u = "endurance [Amount]", .fn = SetPVPPoints, .a = {"#setpvppoints"}},
		Cmd{.cmd = "skill", .u = "skill [Skill ID] [Skill Level]", .fn = SetSkill, .a = {"#setskill"}},
		Cmd{.cmd = "skill_all", .u = "skill_all [Skill Level]", .fn = SetSkillAll, .a = {"#setskillall"}},
		Cmd{.cmd = "start_zone", .u = "endurance [Amount]", .fn = SetEndurance, .a = {"#setstartzone"}},
		Cmd{.cmd = "stat", .u = "endurance [Amount]", .fn = SetEndurance, .a = {"#setstat"}},
		Cmd{.cmd = "exp", .u = "endurance [Amount]", .fn = SetEndurance, .a = {"#setxp"}},
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
				const std::string& rewrite = fmt::format("#set {} {}", cmd.cmd, Strings::Join(args, " "));

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
	c->Message(Chat::White, "Command not found. Usage: #set [command]");
	for (const auto &cmd: commands) {
		c->Message(Chat::White, fmt::format("Usage: #set {}", cmd.u).c_str());
	}
}
