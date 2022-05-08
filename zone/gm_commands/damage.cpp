#include "../client.h"

void command_damage(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #damage [Amount]");
		return;
	}

	Mob* target = c;
	if (c->GetTarget()) {
		target = c->GetTarget();
	}

	int64 damage = std::stoll(sep->arg[1], nullptr, 10);
	target->Damage(c, damage, SPELL_UNKNOWN, EQ::skills::SkillHandtoHand, false);
}
