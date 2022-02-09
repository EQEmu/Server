#include "../client.h"

void command_setskillall(Client *c, const Seperator *sep)
{
	if (c->GetTarget() == 0) {
		c->Message(Chat::White, "Error: #setallskill: No target.");
	}
	else if (!c->GetTarget()->IsClient()) {
		c->Message(Chat::White, "Error: #setskill: Target must be a client.");
	}
	else if (!sep->IsNumber(1) || atoi(sep->arg[1]) < 0 || atoi(sep->arg[1]) > HIGHEST_CAN_SET_SKILL) {
		c->Message(Chat::White, "Usage: #setskillall value ");
		c->Message(Chat::White, "       value = 0 to %d", HIGHEST_CAN_SET_SKILL);
	}
	else {
		if (c->Admin() >= commandSetSkillsOther || c->GetTarget() == c || c->GetTarget() == 0) {
			LogInfo("Set ALL skill request from [{}], target:[{}]", c->GetName(), c->GetTarget()->GetName());
			uint16                     level     = atoi(sep->arg[1]);
			for (EQ::skills::SkillType skill_num = EQ::skills::Skill1HBlunt;
				skill_num <= EQ::skills::HIGHEST_SKILL;
				skill_num = (EQ::skills::SkillType) (skill_num + 1)) {
				c->GetTarget()->CastToClient()->SetSkill(skill_num, level);
			}
		}
		else {
			c->Message(Chat::White, "Error: Your status is not high enough to set anothers skills");
		}
	}
}

