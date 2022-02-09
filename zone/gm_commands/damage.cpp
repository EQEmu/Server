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

	int damage = static_cast<int>(std::min(std::stoll(sep->arg[1]), (long long) 2000000000));
	target->Damage(c, damage, SPELL_UNKNOWN, EQ::skills::SkillHandtoHand, false);
}
