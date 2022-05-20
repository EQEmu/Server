#include "../client.h"

void command_permagender(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #permagender [Gender ID]");
		c->Message(Chat::White, "Genders: 0 = Male, 1 = Female, 2 = Neuter");
		return;
	}

	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	auto gender_id = std::stoi(sep->arg[1]);
	if (gender_id < 0 || gender_id > 2) {
		c->Message(Chat::White, "Usage: #permagender [Gender ID]");
		c->Message(Chat::White, "Genders: 0 = Male, 1 = Female, 2 = Neuter");
		return;
	}
	
	LogInfo("Gender changed by {} for {} to {} ({})",
		c->GetCleanName(),
		c->GetTargetDescription(target),
		GetGenderName(gender_id),
		gender_id
	);

	target->SetBaseGender(gender_id);
	target->Save();
	target->SendIllusionPacket(target->GetRace(), gender_id);

	c->Message(
		Chat::White,
		fmt::format(
			"Gender changed for {} to {} ({}).",
			c->GetTargetDescription(target),
			GetGenderName(gender_id),
			gender_id
		).c_str()
	);
}
