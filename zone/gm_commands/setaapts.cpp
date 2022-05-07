#include "../client.h"
#include "../groups.h"
#include "../raids.h"
#include "../raids.h"

void command_setaapts(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (arguments <= 1 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #setaapts [AA|Group|Raid] [AA Amount]");
		return;
	}

	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	std::string aa_type   = str_tolower(sep->arg[1]);
	std::string group_raid_string;
	uint32 aa_points = static_cast<uint32>(std::min(std::stoull(sep->arg[2]), (unsigned long long) 2000000000));
	bool is_aa = aa_type.find("aa") != std::string::npos;
	bool is_group = aa_type.find("group") != std::string::npos;
	bool is_raid = aa_type.find("raid") != std::string::npos;
	if (!is_aa && !is_group && !is_raid) {
		c->Message(Chat::White, "Usage: #setaapts [AA|Group|Raid] [AA Amount]");
		return;
	}

	if (is_aa) {
		target->GetPP().aapoints = aa_points;
		target->GetPP().expAA    = 0;
		target->SendAlternateAdvancementStats();
	}
	else if (is_group || is_raid) {
		if (is_group) {
			group_raid_string = "Group ";
			target->GetPP().group_leadership_points = aa_points;
			target->GetPP().group_leadership_exp    = 0;
		}
		else if (is_raid) {
			group_raid_string = "Raid ";
			target->GetPP().raid_leadership_points = aa_points;
			target->GetPP().raid_leadership_exp    = 0;
		}
		target->SendLeadershipEXPUpdate();
	}

	std::string aa_message = fmt::format(
		"{} now {} {} {}AA Point{}.",
		c->GetTargetDescription(target, TargetDescriptionType::UCYou),
		c == target ? "have" : "has",
		aa_points,
		group_raid_string,
		aa_points != 1 ? "s" : ""

	);
	c->Message(
		Chat::White,
		aa_message.c_str()
	);
}

