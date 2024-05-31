#include "../client.h"

void command_disarmtrap(Client *c, const Seperator *sep)
{
	Mob *target = c->GetTarget();

	if (!c->HasSkill(Skill::DisarmTraps)) {
		c->Message(Chat::White, "You do not have the Disarm Trap skill.");
		return;
	}

	if (target->IsNPC()) {
		if (c->HasSkill(EQ::skills::SkillDisarmTraps)) {
			if (DistanceSquaredNoZ(c->GetPosition(), target->GetPosition()) > RuleI(Adventure, LDoNTrapDistanceUse)) {
				c->Message(Chat::Red, "%s is too far away.", target->GetCleanName());
				return;
			}
			c->HandleLDoNDisarm(target->CastToNPC(), c->GetSkill(EQ::skills::SkillDisarmTraps), LDoNTypeMechanical);
		}
		else {
			c->Message(Chat::Red, "You do not have the disarm trap skill.");
		}
	}

	c->HandleLDoNDisarm(t->CastToNPC(), c->GetSkill(Skill::DisarmTraps), LDoNTypeMechanical);
}

