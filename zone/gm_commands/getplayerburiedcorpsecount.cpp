#include "../client.h"
#include "../corpse.h"

void command_getplayerburiedcorpsecount(Client *c, const Seperator *sep)
{
	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
		target = c->GetTarget()->CastToClient();
	}

	uint32 corpse_count = database.GetCharacterBuriedCorpseCount(target->CharacterID());
	c->Message(
		Chat::White,
		fmt::format(
			"{} {} buried corpse{}.",
			(
				c == target ?
				"You have" :
				fmt::format(
					"{} ({}) has",
					target->GetCleanName(),
					target->GetID()
				)
			),
			(
				corpse_count ?
				std::to_string(corpse_count) :
				"no"
			),
			corpse_count != 1 ? "s" : ""
		).c_str()
	);
}
