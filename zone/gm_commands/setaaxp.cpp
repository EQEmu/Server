#include "../client.h"
#include "../groups.h"
#include "../raids.h"
#include "../raids.h"

void command_setaaxp(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (arguments <= 1 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #setaaxp [AA|Group|Raid] [AA Experience]");
		return;
	}

	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	std::string aa_type = str_tolower(sep->arg[1]);
	std::string group_raid_string;
	uint32 aa_experience = static_cast<uint32>(std::min(
		std::stoull(sep->arg[2]),
		(unsigned long long) 2000000000
	));
	bool is_aa = aa_type.find("aa") != std::string::npos;
	bool is_group = aa_type.find("group") != std::string::npos;
	bool is_raid = aa_type.find("raid") != std::string::npos;
	if (!is_aa && !is_group && !is_raid) {
		c->Message(Chat::White, "Usage: #setaaxp [AA|Group|Raid] [AA Experience]");
		return;
	}

	if (is_aa) {
		target->SetEXP(
			target->GetEXP(),
			aa_experience,
			false
		);
	}
	else if (is_group) {
		group_raid_string = "Group ";
		target->SetLeadershipEXP(
			aa_experience,
			target->GetRaidEXP()
		);
	}
	else if (is_raid) {
		group_raid_string = "Raid ";
		target->SetLeadershipEXP(
			target->GetGroupEXP(),
			aa_experience
		);
	}

	std::string aa_exp_message = fmt::format(
		"{} now {} {} {}AA Experience.",
		c->GetTargetDescription(target, TargetDescriptionType::UCYou),
		c == target ? "have" : "has",
		aa_experience,
		group_raid_string
	);
	c->Message(
		Chat::White,
		aa_exp_message.c_str()
	);
}

