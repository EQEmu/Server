#include "../client.h"
#include "../../common/data_verification.h"

void command_unscribespell(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #unscribespell [Spell ID] - Unscribe a spell from your or your target's spell book by Spell ID");
		return;
	}

	auto target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
		target = c->GetTarget()->CastToClient();
	}

	uint16 spell_id = EQ::Clamp(std::stoi(sep->arg[1]), 0, 65535);

	if (!IsValidSpell(spell_id)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Spell ID {} could not be found.",
				spell_id
			).c_str()
		);
		return;
	}

	auto spell_name = GetSpellName(spell_id);
	
	if (target->HasSpellScribed(spell_id)) {
		target->UnscribeSpellBySpellID(spell_id);

		c->Message(
			Chat::White,
			fmt::format(
				"Unscribing {} ({}) for {}.",
				spell_name,
				spell_id,
				c->GetTargetDescription(target)
			).c_str()
		);
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} not have {} ({}) scribed.",
				c->GetTargetDescription(target),
				c == target ? "do" : "does",
				spell_name,
				spell_id
			).c_str()
		);
	}
}

