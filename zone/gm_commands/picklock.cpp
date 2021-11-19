#include "../client.h"

void command_picklock(Client *c, const Seperator *sep)
{
	Mob *target = c->GetTarget();
	if (!target) {
		c->Message(Chat::Red, "You must have a target.");
		return;
	}

	if (target->IsNPC()) {
		if (c->HasSkill(EQ::skills::SkillPickLock)) {
			if (DistanceSquaredNoZ(c->GetPosition(), target->GetPosition()) > RuleI(Adventure, LDoNTrapDistanceUse)) {
				c->Message(Chat::Red, "%s is too far away.", target->GetCleanName());
				return;
			}
			c->HandleLDoNPickLock(target->CastToNPC(), c->GetSkill(EQ::skills::SkillPickLock), LDoNTypeMechanical);
		}
		else {
			c->Message(Chat::Red, "You do not have the pick locks skill.");
		}
	}
}

