#include "../client.h"

void command_permarace(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #permarace [Race ID]");
		c->Message(
			Chat::White,
			"NOTE: Not all models are global. If a model is not global, it will appear as a human on character select and in zones without the model."
		);
		return;
	}

	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	auto race_id = std::stoi(sep->arg[1]);
	auto gender_id = Mob::GetDefaultGender(race_id, target->GetBaseGender());
	
	LogInfo("Race changed by {} for {} to {} ({})",
		c->GetCleanName(),
		c->GetTargetDescription(target),
		GetRaceIDName(race_id),
		race_id
	);

	target->SetBaseRace(race_id);
	target->SetBaseGender(gender_id);
	target->Save();
	target->SendIllusionPacket(race_id, gender_id);

	c->Message(
		Chat::White,
		fmt::format(
			"Race changed for {} to {} ({}).",
			c->GetTargetDescription(target),
			GetRaceIDName(race_id),
			race_id
		).c_str()
	);
}
