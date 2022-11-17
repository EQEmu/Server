#include "../client.h"
#include "../../common/data_verification.h"

void command_xtargets(Client *c, const Seperator *sep)
{
	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	auto arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		t->ShowXTargets(c);
		return;
	}

	const auto new_max = static_cast<uint8>(std::stoul(sep->arg[1]));

	if (!EQ::ValueWithin(new_max, 5, XTARGET_HARDCAP)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Number of XTargets must be between 5 and {}.",
				XTARGET_HARDCAP
			).c_str()
		);
		return;
	}

	t->SetMaxXTargets(new_max);

	c->Message(
		Chat::White,
		fmt::format(
			"Max number of XTargets set to {} for {}.",
			new_max,
			c->GetTargetDescription(t, TargetDescriptionType::LCSelf)
		).c_str()
	);
}
