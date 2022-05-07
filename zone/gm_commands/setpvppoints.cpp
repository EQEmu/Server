#include "../client.h"

void command_setpvppoints(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Command Syntax: #setpvppoints [Amount]");
		return;
	}

	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	uint32 pvp_points = static_cast<uint32>(std::min(std::stoull(sep->arg[1]), (unsigned long long) 2000000000));
	target->SetPVPPoints(pvp_points);
	target->Save();
	target->SendPVPStats();
	std::string pvp_message = fmt::format(
		"{} now {} {} PVP Point{}.",
		c->GetTargetDescription(target, TargetDescriptionType::UCYou),
		c == target ? "have" : "has",
		pvp_points,
		pvp_points != 1 ? "s" : ""
	);
	c->Message(
		Chat::White,
		pvp_message.c_str()
	);
}

