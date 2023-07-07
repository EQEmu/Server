#include "../client.h"
#include "set/aa_exp.cpp"
#include "set/aa_points.cpp"
#include "set/adventure_points.cpp"
#include "set/alternate_currency.cpp"
#include "set/animation.cpp"
#include "set/anon.cpp"
#include "set/bind_point.cpp"
#include "set/checksum.cpp"
#include "set/class_permanent.cpp"
#include "set/crystals.cpp"
#include "set/date.cpp"
#include "set/endurance.cpp"
#include "set/endurance_full.cpp"
#include "set/exp.cpp"
#include "set/flymode.cpp"
#include "set/frozen.cpp"
#include "set/gender.cpp"
#include "set/gender_permanent.cpp"
#include "set/gm.cpp"
#include "set/gm_speed.cpp"
#include "set/gm_status.cpp"
#include "set/god_mode.cpp"
#include "set/haste.cpp"
#include "set/hero_model.cpp"
#include "set/hide_me.cpp"
#include "set/hp.cpp"
#include "set/hp_full.cpp"
#include "set/invulnerable.cpp"
#include "set/language.cpp"
#include "set/last_name.cpp"
#include "set/level.cpp"
#include "set/loginserver_info.cpp"
#include "set/mana.cpp"
#include "set/mana_full.cpp"
#include "set/name.cpp"
#include "set/ooc_mute.cpp"
#include "set/password.cpp"
#include "set/pvp.cpp"
#include "set/pvp_points.cpp"
#include "set/race.cpp"
#include "set/race_permanent.cpp"
#include "set/server_locked.cpp"
#include "set/skill.cpp"
#include "set/skill_all.cpp"
#include "set/skill_all_max.cpp"
#include "set/start_zone.cpp"
#include "set/temporary_name.cpp"
#include "set/texture.cpp"
#include "set/time.cpp"
#include "set/time_zone.cpp"
#include "set/title.cpp"
#include "set/title_suffix.cpp"
#include "set/weather.cpp"
#include "set/zone_clipping.cpp"
#include "set/zone_color.cpp"
#include "set/zone_header.cpp"
#include "set/zone_locked.cpp"
#include "set/zone_safe_coordinates.cpp"
#include "set/zone_sky.cpp"
#include "set/zone_underworld.cpp"

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
		Cmd{.cmd = "anon", .u = "anon [Character ID] [Anonymous Flag] or #set anon [Anonymous Flag]", .fn = SetAnon, .a = {"#setanon"}},
		Cmd{.cmd = "bind_point", .u = "bind_point", .fn = SetBindPoint, .a = {"#setbind"}},
		Cmd{.cmd = "checksum", .u = "checksum", .fn = SetChecksum, .a = {"#updatechecksum"}},
		Cmd{.cmd = "class_permanent", .u = "class_permanent [Class ID]", .fn = SetClassPermanent, .a = {"#permaclass"}},
		Cmd{.cmd = "crystals", .u = "crystals [ebon|radiant] [Amount]", .fn = SetCrystals, .a = {"#setcrystals"}},
		Cmd{.cmd = "date", .u = "date [Year] [Month] [Day] [Hour] [Minute] (Hour and Minute are optional)", .fn = SetDate, .a = {"#date"}},
		Cmd{.cmd = "endurance", .u = "endurance [Amount]", .fn = SetEndurance, .a = {"#setendurance"}},
		Cmd{.cmd = "endurance_full", .u = "endurance_full", .fn = SetEnduranceFull, .a = {"#endurance"}},
		Cmd{.cmd = "exp", .u = "exp [Amount]", .fn = SetEXP, .a = {"#setxp"}},
		Cmd{.cmd = "flymode", .u = "flymode [Flymode ID]", .fn = SetFlymode, .a = {"#flymode"}},
		Cmd{.cmd = "frozen", .u = "frozen [on|off]", .fn = SetFrozen, .a = {"#freeze", "#unfreeze"}},
		Cmd{.cmd = "gender", .u = "gender [Gender ID]", .fn = SetGender, .a = {"#gender"}},
		Cmd{.cmd = "gender_permanent", .u = "gender_permanent [Flymode ID]", .fn = SetGenderPermanent, .a = {"#permagender"}},
		Cmd{.cmd = "gm", .u = "gm [on|off]", .fn = SetGM, .a = {"#flymode"}},
		Cmd{.cmd = "gm_speed", .u = "gm_speed [on|off]", .fn = SetGMSpeed, .a = {"#gmspeed"}},
		Cmd{.cmd = "gm_status", .u = "gm_status [GM Status] [Account]", .fn = SetGMStatus, .a = {"#flag"}},
		Cmd{.cmd = "god_mode", .u = "god_mode [on|off]", .fn = SetGodMode, .a = {"#godmode"}},
		Cmd{.cmd = "haste", .u = "haste [Percentage]", .fn = SetHaste, .a = {"#haste"}},
		Cmd{.cmd = "hide_me", .u = "hide_me [on|off]", .fn = SetHideMe, .a = {"#hideme"}},
		Cmd{.cmd = "hero_model", .u = "hero_model [Hero Model] [Slot] (Slot is optional)", .fn = SetHeroModel, .a = {"#heromodel"}},
		Cmd{.cmd = "hp", .u = "hp [Amount]", .fn = SetHP, .a = {"#sethp"}},
		Cmd{.cmd = "hp_full", .u = "hp_full", .fn = SetHPFull, .a = {"#heal"}},
		Cmd{.cmd = "invulnerable", .u = "invulnerable", .fn = SetInvulnerable, .a = {"#invul"}},
		Cmd{.cmd = "language", .u = "language [Language ID] [Language Level]", .fn = SetLanguage, .a = {"#setlanguage"}},
		Cmd{.cmd = "last_name", .u = "last_name [Last Name]", .fn = SetLastName, .a = {"#lastname"}},
		Cmd{.cmd = "level", .u = "level [Level]", .fn = SetLevel, .a = {"#level"}},
		Cmd{.cmd = "loginserver_info", .u = "loginserver_info [Email] [Password]", .fn = SetLoginserverInfo, .a = {"#setlsinfo"}},
		Cmd{.cmd = "mana", .u = "mana [Amount]", .fn = SetMana, .a = {"#setmana"}},
		Cmd{.cmd = "mana_full", .u = "mana_full", .fn = SetManaFull, .a = {"#mana"}},
		Cmd{.cmd = "name", .u = "name", .fn = SetName, .a = {"#name"}},
		Cmd{.cmd = "ooc_mute", .u = "ooc_mute", .fn = SetOOCMute, .a = {"#oocmute"}},
		Cmd{.cmd = "password", .u = "password [Account Name] [Password]", .fn = SetPassword, .a = {"#setpass"}},
		Cmd{.cmd = "pvp", .u = "pvp [on|off]", .fn = SetPVP, .a = {"#pvp"}},
		Cmd{.cmd = "pvp_points", .u = "pvp_points [Amount]", .fn = SetPVPPoints, .a = {"#setpvppoints"}},
		Cmd{.cmd = "race", .u = "race [Race ID]", .fn = SetRace, .a = {"#race"}},
		Cmd{.cmd = "race_permanent", .u = "race_permanent [Race ID]", .fn = SetRacePermanent, .a = {"#permarace"}},
		Cmd{.cmd = "server_locked", .u = "server_locked [on|off]", .fn = SetServerLocked, .a = {"#serverlock"}},
		Cmd{.cmd = "skill", .u = "skill [Skill ID] [Skill Level]", .fn = SetSkill, .a = {"#setskill"}},
		Cmd{.cmd = "skill_all", .u = "skill_all [Skill Level]", .fn = SetSkillAll, .a = {"#setskillall"}},
		Cmd{.cmd = "skill_all_max", .u = "skill_all_max", .fn = SetSkillAllMax, .a = {"#maxskills"}},
		Cmd{.cmd = "start_zone", .u = "endurance [Amount]", .fn = SetStartZone, .a = {"#setstartzone"}},
		Cmd{.cmd = "temporary_name", .u = "temporary_name [Name]", .fn = SetTemporaryName, .a = {"#tempname"}},
		Cmd{.cmd = "texture", .u = "texture [Texture ID]", .fn = SetTexture, .a = {"#texture"}},
		Cmd{.cmd = "time", .u = "time [Hour] [Minute]", .fn = SetTime, .a = {"#time"}},
		Cmd{.cmd = "time_zone", .u = "time_zone [Hour] [Minute]", .fn = SetTimeZone, .a = {"#timezone"}},
		Cmd{.cmd = "title", .u = "title [Title]", .fn = SetTitle, .a = {"#title"}},
		Cmd{.cmd = "title_suffix", .u = "title_suffix [Title Suffix]", .fn = SetTitleSuffix, .a = {"#titlesuffix"}},
		Cmd{.cmd = "temporary_name", .u = "temporary_name [Name]", .fn = SetTemporaryName, .a = {"#tempname"}},
		Cmd{.cmd = "weather", .u = "weather [0|1|2|3]", .fn = SetWeather, .a = {"#weather"}},
		Cmd{.cmd = "zone_clipping", .u = "zone_clipping [Minimum Clip] [Maximum Clip] [Fog Minimum Clip] [Fog Maximum Clip] [Permanent (0 = False, 1 = True)] (Permanent is optional)", .fn = SetZoneClipping, .a = {"#zclip"}},
		Cmd{.cmd = "zone_color", .u = "zone_color [Red] [Green] [Blue] [Permanent (0 = False, 1 = True)] (Permanent is optional)", .fn = SetZoneColor, .a = {"#zcolor"}},
		Cmd{.cmd = "zone_header", .u = "zone_header [Zone ID|Zone Short Name] [Version]", .fn = SetZoneHeader, .a = {"#zcolor"}},
		Cmd{.cmd = "zone_locked", .u = "zone_locked [on|off] [Zone ID|Zone Short Name]", .fn = SetZoneLocked, .a = {"#zonelock"}},
		Cmd{.cmd = "zone_safe_coordinates", .u = "zone_safe_coordinates [X] [Y] [Z] [Heading] [Permanent (0 = False, 1 = True)] (Permanent is optional)", .fn = SetZoneSafeCoordinates, .a = {"#zsafecoords"}},
		Cmd{.cmd = "zone_sky", .u = "zone_sky [Sky Type] [Permanent (0 = False, 1 = True)] (Permanent is optional)", .fn = SetZoneSky, .a = {"#zsky"}},
		Cmd{.cmd = "zone_underworld", .u = "zone_underworld [Z] [Permanent (0 = False, 1 = True)] (Permanent is optional)", .fn = SetZoneUnderworld, .a = {"#zunderworld"}},
	};

	// Check for arguments
	const auto arguments = sep->argnum;

	// look for alias or command
	for (const auto &cmd: commands) {
		c->Message(Chat::White, fmt::format("CMD: {}", cmd.cmd).c_str());
		// Check for alias first
		for (const auto &alias: cmd.a) {
			if (!alias.empty() && Strings::EqualFold(alias, sep->arg[0])) {
				c->Message(Chat::White, fmt::format("CMD: {} Alias: {}", cmd.cmd, alias).c_str());
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
			c->Message(Chat::White, fmt::format("CMD: {} found", cmd.cmd).c_str());
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
