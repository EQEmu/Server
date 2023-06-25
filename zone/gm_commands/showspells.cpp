#include "../client.h"

void command_showspells(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usages: #showspells disciplines - Show your or your target's learned disciplines");
		c->Message(Chat::White, "Usages: #showspells spells - Show your or your target's memorized spells");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const auto is_disciplines = !strcasecmp(sep->arg[1], "disciplines");
	const auto is_spells      = !strcasecmp(sep->arg[1], "spells");
	if (
		!is_disciplines &&
		!is_spells
	) {
		c->Message(Chat::White, "Usages: #showspells disciplines - Show your or your target's learned disciplines");
		c->Message(Chat::White, "Usages: #showspells spells - Show your or your target's memorized spells");
	}

	ShowSpellType show_spell_type;

	if (is_disciplines) {
		show_spell_type = ShowSpellType::Disciplines;
	} else if (is_spells) {
		show_spell_type = ShowSpellType::Spells;
	}

	t->ShowSpells(c, show_spell_type);
}

