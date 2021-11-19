#include "../client.h"

void command_modifynpcstat(Client *c, const Seperator *sep)
{
	if (!c) {
		return;
	}

	if (sep->arg[1][0] == '\0') {
		c->Message(Chat::White, "usage #modifynpcstat arg value");
		c->Message(
			Chat::White,
			"Args: ac, str, sta, agi, dex, wis, _int, cha, max_hp, mr, fr, cr, pr, dr, runspeed, special_attacks, "
			"attack_speed, atk, accuracy, trackable, min_hit, max_hit, see_invis_undead, see_hide, see_improved_hide, "
			"hp_regen, mana_regen, aggro, assist, slow_mitigation, loottable_id, healscale, spellscale"
		);
		return;
	}

	if (!c->GetTarget()) {
		return;
	}

	if (!c->GetTarget()->IsNPC()) {
		return;
	}

	c->GetTarget()->CastToNPC()->ModifyNPCStat(sep->arg[1], sep->arg[2]);
}

