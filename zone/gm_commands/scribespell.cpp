#include "../client.h"

void command_scribespell(Client *c, const Seperator *sep)
{
	uint16 spell_id  = 0;
	uint16 book_slot = -1;
	Client *t        = c;

	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
		t = c->GetTarget()->CastToClient();
	}

	if (!sep->arg[1][0]) {
		c->Message(Chat::White, "FORMAT: #scribespell <spellid>");
		return;
	}

	spell_id = atoi(sep->arg[1]);

	if (IsValidSpell(spell_id)) {
		t->Message(Chat::White, "Scribing spell: %s (%i) to spellbook.", spells[spell_id].name, spell_id);

		if (t != c) {
			c->Message(Chat::White, "Scribing spell: %s (%i) for %s.", spells[spell_id].name, spell_id, t->GetName());
		}

		LogInfo("Scribe spell: [{}] ([{}]) request for [{}] from [{}]",
				spells[spell_id].name,
				spell_id,
				t->GetName(),
				c->GetName());

		if (spells[spell_id].classes[WARRIOR] != 0 && spells[spell_id].skill != 52 &&
			spells[spell_id].classes[t->GetPP().class_ - 1] > 0 && !IsDiscipline(spell_id)) {
			book_slot = t->GetNextAvailableSpellBookSlot();

			if (book_slot >= 0 && t->FindSpellBookSlotBySpellID(spell_id) < 0) {
				t->ScribeSpell(spell_id, book_slot);
			}
			else {
				t->Message(
					Chat::Red,
					"Unable to scribe spell: %s (%i) to your spellbook.",
					spells[spell_id].name,
					spell_id
				);

				if (t != c) {
					c->Message(
						Chat::Red,
						"Unable to scribe spell: %s (%i) for %s.",
						spells[spell_id].name,
						spell_id,
						t->GetName());
				}
			}
		}
		else {
			c->Message(Chat::Red, "Your target can not scribe this spell.");
		}
	}
	else {
		c->Message(Chat::Red, "Spell ID: %i is an unknown spell and cannot be scribed.", spell_id);
	}
}

