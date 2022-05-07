#include "../client.h"

void command_setskill(Client *c, const Seperator *sep)
{
	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	auto skill_id    = sep->IsNumber(1) ? std::stoi(sep->arg[1]) : -1;
	auto skill_value = sep->IsNumber(2) ? std::stoi(sep->arg[2]) : -1;
	if (
		skill_id < 0 ||
		skill_id > EQ::skills::HIGHEST_SKILL ||
		skill_value < 0 ||
		skill_value > HIGHEST_CAN_SET_SKILL
		) {
		c->Message(Chat::White, "Usage: #setskill [Skill ID] [Skill Value]");
		c->Message(Chat::White, fmt::format("Skill ID = 0 to {}", EQ::skills::HIGHEST_SKILL).c_str());
		c->Message(Chat::White, fmt::format("Skill Value = 0 to {}", HIGHEST_CAN_SET_SKILL).c_str());
	}
	else {
		LogInfo(
			"Set skill request from [{}], Target: [{}] Skill ID: [{}] Skill Value: [{}]",
			c->GetCleanName(),
			c->GetTargetDescription(target),
			skill_id,
			skill_value
		);

		if (
			skill_id >= EQ::skills::Skill1HBlunt &&
			skill_id <= EQ::skills::HIGHEST_SKILL
			) {
			target->SetSkill(
				(EQ::skills::SkillType) skill_id,
				skill_value
			);

			if (c != target) {
				c->Message(
					Chat::White,
					fmt::format(
						"Set {} ({}) to {} for {}.",
						EQ::skills::GetSkillName((EQ::skills::SkillType) skill_id),
						skill_id,
						skill_value,
						c->GetTargetDescription(target)
					).c_str()
				);
			}
		}
	}
}

