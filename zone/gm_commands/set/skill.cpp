#include "../../client.h"

void command_setskill(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;

	if (arguments < 2 || !sep->IsNumber(1) || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #setskill [Skill ID] [Skill Value]");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const auto skill_id    = Strings::ToInt(sep->arg[1]);
	const auto skill_value = Strings::ToInt(sep->arg[2]);

	if (!EQ::ValueWithin(skill_id, EQ::skills::Skill1HBlunt, EQ::skills::HIGHEST_SKILL)) {
		c->Message(Chat::White, "Usage: #setskill [Skill ID] [Skill Value]");
		c->Message(Chat::White, fmt::format("Skill ID: 0 to {}", EQ::skills::HIGHEST_SKILL).c_str());
		return;
	}

	const auto skill_type = static_cast<EQ::skills::SkillType>(skill_id);

	t->SetSkill(
		skill_type,
		skill_value > t->MaxSkill(skill_type) ? t->MaxSkill(skill_type) : skill_value
	);

	if (c != t) {
		c->Message(
			Chat::White,
			fmt::format(
				"Set {} ({}) to {} for {}.",
				EQ::skills::GetSkillName(skill_type),
				skill_id,
				skill_value > t->MaxSkill(skill_type) ? t->MaxSkill(skill_type) : skill_value,
				c->GetTargetDescription(t)
			).c_str()
		);
	}
}

