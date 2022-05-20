#include "../client.h"

void command_setcrystals(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (arguments <= 1 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #setcrystals [Ebon|Radiant] [Crystal Amount]");
		return;
	}

	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	std::string crystal_type = str_tolower(sep->arg[1]);
	uint32 crystal_amount = static_cast<uint32>(std::min(
		std::stoull(sep->arg[2]),
		(unsigned long long) 2000000000
	));
	bool is_ebon = crystal_type.find("ebon") != std::string::npos;
	bool is_radiant = crystal_type.find("radiant") != std::string::npos;
	if (!is_ebon && !is_radiant) {
		c->Message(Chat::White, "Usage: #setcrystals [Ebon|Radiant] [Crystal Amount]");
		return;
	}

	uint32 crystal_item_id = (
		is_ebon ?
		RuleI(Zone, EbonCrystalItemID) :
		RuleI(Zone, RadiantCrystalItemID)
	);

	auto crystal_link = database.CreateItemLink(crystal_item_id);
	if (is_radiant) {
		target->SetRadiantCrystals(crystal_amount);
	} else {
		target->SetEbonCrystals(crystal_amount);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} now {} {} {}.",
			c->GetTargetDescription(target, TargetDescriptionType::UCYou),
			c == target ? "have" : "has",
			crystal_amount,
			crystal_link
		).c_str()
	);
}

