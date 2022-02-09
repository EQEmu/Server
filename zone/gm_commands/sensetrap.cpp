#include "../client.h"

void command_sensetrap(Client *c, const Seperator *sep)
{
	Mob *target = c->GetTarget();
	if (!target) {
		c->Message(Chat::Red, "You must have a target.");
		return;
	}

	if (target->IsNPC()) {
		if (c->HasSkill(EQ::skills::SkillSenseTraps)) {
			if (DistanceSquaredNoZ(c->GetPosition(), target->GetPosition()) > RuleI(Adventure, LDoNTrapDistanceUse)) {
				c->Message(Chat::Red, "%s is too far away.", target->GetCleanName());
				return;
			}
			c->HandleLDoNSenseTraps(target->CastToNPC(), c->GetSkill(EQ::skills::SkillSenseTraps), LDoNTypeMechanical);
		}
		else {
			c->Message(Chat::Red, "You do not have the sense traps skill.");
		}
	}
}

