#include "../client.h"

void command_permaclass(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #permaclass [Class ID]");
		return;
	}

	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	auto class_id = std::stoi(sep->arg[1]);
	
	LogInfo("Class changed by {} for {} to {} ({})",
		c->GetCleanName(),
		c->GetTargetDescription(target),
		GetClassIDName(class_id),
		class_id
	);

	target->SetBaseClass(class_id);
	target->Save();
	target->Kick("Class was changed.");

	if (c != target) {
		c->Message(
			Chat::White,
			fmt::format(
				"Class changed for {} to {} ({}).",
				c->GetTargetDescription(target),
				GetClassIDName(class_id),
				class_id
			).c_str()
		);
	}
}
