#include "../client.h"

void command_scribespells(Client *c, const Seperator *sep)
{
	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
		target = c->GetTarget()->CastToClient();
	}

	if (sep->argnum < 1 || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #scribespells [Max Level] [Min Level]");
		return;
	}

	uint8 rule_max_level = (uint8) RuleI(Character, MaxLevel);
	uint8 max_level = (uint8) std::stoi(sep->arg[1]);
	uint8 min_level = (
		sep->IsNumber(2) ?
		(uint8) std::stoi(sep->arg[2]) :
		1
	); // Default to Level 1 if there isn't a 2nd argument

	if (!c->GetGM()) { // Default to Character:MaxLevel if we're not a GM and Level is higher than the max level
		if (max_level > rule_max_level) {
			max_level = rule_max_level;
		}

		if (min_level > rule_max_level) {
			min_level = rule_max_level;
		}
	}

	if (max_level < 1 || min_level < 1) {
		c->Message(Chat::White, "Level must be greater than or equal to 1.");
		return;
	}

	if (min_level > max_level) {
		c->Message(Chat::White, "Minimum Level must be less than or equal to Maximum Level.");
		return;
	}

	uint16 scribed_spells = target->ScribeSpells(min_level, max_level);
	if (c != target) {
		std::string spell_message = (
			scribed_spells > 0 ?
			(
				scribed_spells == 1 ?
				"A new spell" :
				fmt::format(
					"{} New spells",
					scribed_spells
				)
			) :
			"No new spells"
		);
		c->Message(
			Chat::White,
			fmt::format(
				"{} scribed for {}.",
				spell_message,
				c->GetTargetDescription(target)
			).c_str()
		);
	}
}

