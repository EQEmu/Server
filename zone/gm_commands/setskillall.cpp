#include "../client.h"

void command_setskillall(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;

	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #setskillall [Skill Level] - Set all of your or your target's skills to the specified skill level");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	if (c->Admin() < commandSetSkillsOther && t != c) {
		c->Message(Chat::White, "Your status is not high enough to set another player's skills.");
		return;
	}

	auto skill_level = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[1]));

	for (const auto& s : EQ::skills::GetSkillTypeMap()) {
		if (c != t) {
			c->Message(
				Chat::White,
				fmt::format(
					"Setting {} ({}) to {} for {}.",
					s.second,
					s.first,
					skill_level > t->MaxSkill(s.first) ? t->MaxSkill(s.first) : skill_level,
					c->GetTargetDescription(t)
				).c_str()
			);
		}

		t->SetSkill(
			s.first,
			skill_level > t->MaxSkill(s.first) ? t->MaxSkill(s.first) : skill_level
		);
	}
}
