#include "../client.h"

void command_setskillall(Client *c, const Seperator *sep)
{
	auto target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #setskillall [Skill Level] - Set all of your or your target's skills to the specified skill level");
		c->Message(
			Chat::White,
			fmt::format(
				"Note: Skill Level ranges from 0 to {}",
				HIGHEST_CAN_SET_SKILL
			).c_str()
		);
	} else {
		if (c->Admin() >= commandSetSkillsOther || c->GetTarget() == c) {
			LogInfo(
				"Set ALL skill request from [{}], target:[{}]",
				c->GetCleanName(),
				c->GetTargetDescription(target)
			);

			auto skill_level = static_cast<uint16>(std::stoul(sep->arg[1]));

			c->Message(
				Chat::White,
				fmt::format(
					"Setting all skills to {} for {}.",
					skill_level,
					c->GetTargetDescription(target)
				).c_str()
			);

			for (EQ::skills::SkillType skill_num = EQ::skills::Skill1HBlunt; skill_num <= EQ::skills::HIGHEST_SKILL; skill_num = (EQ::skills::SkillType) (skill_num + 1)) {
				target->SetSkill(skill_num, skill_level);
			}
		} else {
			c->Message(Chat::White, "Your status is not high enough to set another player's skills.");
		}
	}
}

