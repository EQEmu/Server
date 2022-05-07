#include "../client.h"

void command_castspell(Client *c, const Seperator *sep)
{
	if (SPDAT_RECORDS <= 0) {
		c->Message(Chat::White, "Spells not loaded.");
		return;
	}

	Mob *target = c;
	if (c->GetTarget()) {
		target = c->GetTarget();
	}

	if (!sep->IsNumber(1)) {
		c->Message(
			Chat::White,
			"Usage: #castspell [Spell ID]  [Instant (0 = False, 1 = True, Default is 1 if Unused)]"
		);
	}
	else {
		uint16 spell_id = std::stoul(sep->arg[1]);

		if (CastRestrictedSpell(spell_id) && c->Admin() < commandCastSpecials) {
			c->Message(Chat::Red, "Unable to cast spell.");
		}
		else if (spell_id >= SPDAT_RECORDS) {
			c->Message(Chat::White, "Invalid Spell ID.");
		}
		else {
			bool instant_cast = (c->Admin() >= commandInstacast ? true : false);
			if (instant_cast && sep->IsNumber(2)) {
				instant_cast = std::stoi(sep->arg[2]) ? true : false;
				c->Message(Chat::White, fmt::format("{}", std::stoi(sep->arg[2])).c_str());
			}

			if (c->Admin() >= commandInstacast && instant_cast) {
				c->SpellFinished(
					spell_id,
					target,
					EQ::spells::CastingSlot::Item,
					0,
					-1,
					spells[spell_id].resist_difficulty
				);
			}
			else {
				c->CastSpell(spell_id, target->GetID(), EQ::spells::CastingSlot::Item, spells[spell_id].cast_time);
			}

			c->Message(
				Chat::White,
				fmt::format(
					"Cast {} ({}) on {}{}.",
					GetSpellName(spell_id),
					spell_id,
					c->GetTargetDescription(target),
					instant_cast ? " instantly" : ""
				).c_str()
			);
		}
	}
}

