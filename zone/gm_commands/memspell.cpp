#include "../client.h"

void command_memspell(Client *c, const Seperator *sep)
{
	uint32 slot;
	uint16 spell_id;

	if (!(sep->IsNumber(1) && sep->IsNumber(2))) {
		c->Message(Chat::White, "Usage: #MemSpell slotid spellid");
	}
	else {
		slot     = atoi(sep->arg[1]) - 1;
		spell_id = atoi(sep->arg[2]);
		if (slot > EQ::spells::SPELL_GEM_COUNT || spell_id >= SPDAT_RECORDS) {
			c->Message(Chat::White, "Error: #MemSpell: Arguement out of range");
		}
		else {
			c->MemSpell(spell_id, slot);
			c->Message(Chat::White, "Spell slot changed, have fun!");
		}
	}
}
