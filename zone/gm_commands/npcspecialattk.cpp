#include "../client.h"

void command_npcspecialattk(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(
			Chat::White,
			"Usage: #npcspecialattk [Flag] [Permanent] - Set an NPC's special attack flag, use permanent to set permanently."
		);
		c->Message(
			Chat::White,
			"Usage: #npcspecialattk help - List all the special attack identifiers"
		);
		return;
	} else if (arguments == 1) {
		if (!strcasecmp(sep->arg[1], "help")) {		
			c->SendPopupToClient(
				"Special Attack Flags",
				"<table><tr><td>Flag</td><td>Name</td></tr>"
				"<tr><td>A</td><td>Immune to Melee</td></tr>"
				"<tr><td>B</td><td>Immune to Magic</td></tr>"
				"<tr><td>b</td><td>Bane Attack</td></tr>"
				"<tr><td>C</td><td>Immune to Charm</td></tr>"
				"<tr><td>D</td><td>Immune to Fear</td></tr>"
				"<tr><td>d</td><td>Immune to Feign Death</td></tr>"
				"<tr><td>E</td><td>Enrage</td></tr>"
				"<tr><td>e</td><td>Always Flee</td></tr>"
				"<tr><td>F</td><td>Flurry</td></tr>"
				"<tr><td>f</td><td>Immune to Fleeing</td></tr>"
				"<tr><td>G</td><td>Immune to being Aggroed</td></tr>"
				"<tr><td>g</td><td>Immune to casting from Range</td></tr>"
				"<tr><td>H</td><td>Immune to Aggro</td></tr>"
				"<tr><td>h</td><td>Flee Percentage</td></tr>"
				"<tr><td>I</td><td>Immune to Snare</td></tr>"
				"<tr><td>i</td><td>Immune to Taunt</td></tr>"
				"<tr><td>J</td><td>Leash</td></tr>"
				"<tr><td>j</td><td>Tether</td></tr>"
				"<tr><td>K</td><td>Immune to Dispel</td></tr>"
				"<tr><td>L</td><td>Innate Dual Wield</td></tr>"
				"<tr><td>M</td><td>Immune to Mesmerize</td></tr>"
				"<tr><td>m</td><td>Magical Attack</td></tr>"
				"<tr><td>N</td><td>Immune to Stun</td></tr>"
				"<tr><td>n</td><td>No Buff or Heal Friends</td></tr>"
				"<tr><td>O</td><td>Immune to non-Bane Damage</td></tr>"
				"<tr><td>o</td><td>Destructible Object</td></tr>"
				"<tr><td>p</td><td>Immune to Pacification</td></tr>"
				"<tr><td>Q</td><td>Quadruple Attack</td></tr>"
				"<tr><td>R</td><td>Rampage</td></tr>"
				"<tr><td>r</td><td>Area Rampage</td></tr>"
				"<tr><td>S</td><td>Summon</td></tr>"
				"<tr><td>T</td><td>Triple Attack</td></tr>"
				"<tr><td>t</td><td>Tunnel Vision</td></tr>"
				"<tr><td>U</td><td>Immune to Slow</td></tr>"
				"<tr><td>W</td><td>Immune to non-Magical Damage</td></tr>"
				"<tr><td>Y</td><td>Ranged Attack</td></tr>"
				"<tr><td>Z</td><td>Immune to Harm from Client</td></tr>"
				"</table>"
			);
			return;
		}
	}
	
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	std::string flag = sep->arg[1];
	if (flag.empty()) {
		c->Message(
			Chat::White,
			"Usage: #npcspecialattk [Flag] [Permanent] - Set an NPC's special attack flag, use permanent to set permanently."
		);
		c->Message(
			Chat::White,
			"Usage: #npcspecialattk help - List all the special attack identifiers"
		);
		return;
	}

	auto permanent_flag = std::stoi(sep->arg[2]);
	
	c->GetTarget()->CastToNPC()->NPCSpecialAttacks(flag.c_str(), permanent_flag);
	c->Message(
		Chat::White,
		fmt::format(
			"NPC Special Attack Flag '{}' set {}.",
			flag,
			permanent_flag ? "permanently" : "temporarily"
		).c_str()
	);
}

