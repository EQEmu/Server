#include "../client.h"

void command_showskills(Client *c, const Seperator *sep)
{
	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	bool show_all = false;

	if (!strcasecmp("all", sep->arg[1])) {
		show_all = true;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Skills | Name: {}",
			target->GetCleanName()
		).c_str()
	);

	for (
		EQ::skills::SkillType skill_type = EQ::skills::Skill1HBlunt;
		skill_type <= EQ::skills::HIGHEST_SKILL;
		skill_type = (EQ::skills::SkillType) (skill_type + 1)
		) {
		if (show_all || (target->CanHaveSkill(skill_type) && target->MaxSkill(skill_type))) {
			c->Message(
				Chat::White,
				fmt::format(
					"{} ({}) | Current: {} Max: {} Raw: {}",
					EQ::skills::GetSkillName(skill_type),
					skill_type,
					target->GetSkill(skill_type),
					target->MaxSkill(skill_type),
					target->GetRawSkill(skill_type)
				).c_str()
			);
		}
	}
}

