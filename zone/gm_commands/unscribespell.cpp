#include "../client.h"

void command_unscribespell(Client *c, const Seperator *sep)
{
	uint16 spell_id  = 0;
	uint16 book_slot = -1;
	Client *t        = c;

	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
		t = c->GetTarget()->CastToClient();
	}

	if (!sep->arg[1][0]) {
		c->Message(Chat::White, "FORMAT: #unscribespell <spellid>");
		return;
	}

	spell_id = atoi(sep->arg[1]);

	if (IsValidSpell(spell_id)) {
		book_slot = t->FindSpellBookSlotBySpellID(spell_id);

		if (book_slot >= 0) {
			t->UnscribeSpell(book_slot);

			t->Message(Chat::White, "Unscribing spell: %s (%i) from spellbook.", spells[spell_id].name, spell_id);

			if (t != c) {
				c->Message(
					Chat::White,
					"Unscribing spell: %s (%i) for %s.",
					spells[spell_id].name,
					spell_id,
					t->GetName());
			}

			LogInfo("Unscribe spell: [{}] ([{}]) request for [{}] from [{}]",
					spells[spell_id].name,
					spell_id,
					t->GetName(),
					c->GetName());
		}
		else {
			t->Message(
				Chat::Red,
				"Unable to unscribe spell: %s (%i) from your spellbook. This spell is not scribed.",
				spells[spell_id].name,
				spell_id
			);

			if (t != c) {
				c->Message(
					Chat::Red,
					"Unable to unscribe spell: %s (%i) for %s due to spell not scribed.",
					spells[spell_id].name,
					spell_id,
					t->GetName());
			}
		}
	}
}

