#include "../client.h"

void command_memspell(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (
		!arguments ||
		!sep->IsNumber(1) ||
		!sep->IsNumber(2)
	) {
		c->Message(Chat::White, "Usage: #memspell [Slot] [Spell ID]");
		return;
	}

	Client* target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
		target = c->GetTarget()->CastToClient();
	}

	uint32 spell_gem = std::stoul(sep->arg[1]);
	uint32 slot = (spell_gem - 1);
	uint16 spell_id = static_cast<uint16>(std::stoul(sep->arg[2]));
	if (
		IsValidSpell(spell_id) &&
		slot < EQ::spells::SPELL_GEM_COUNT
	)  {			
		target->MemSpell(spell_id, slot);
		c->Message(
			Chat::White,
			fmt::format(
				"{} ({}) has been memorized to Spell Gem {} ({}) for {}.",
				GetSpellName(spell_id),
				spell_id,
				spell_gem,
				slot,
				(
					c == target ?
					"yourself" :
					fmt::format(
						"{} ({})",
						target->GetCleanName(),
						target->GetID()
					)
				)
			).c_str()
		);
	}
}
