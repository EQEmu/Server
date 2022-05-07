#include "../client.h"
#include "../corpse.h"

void command_save(Client *c, const Seperator *sep)
{
	if (
		!c->GetTarget() ||
		(
			c->GetTarget() &&
			!c->GetTarget()->IsClient() &&
			!c->GetTarget()->IsPlayerCorpse()
		)
	) {
		c->Message(Chat::White, "You must target a player or player corpse to use this command.");
		return;
	}

	if (c->GetTarget()->IsClient()) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} ({}) {} saved.",
				c->GetTarget()->GetCleanName(),
				c->GetTarget()->GetID(),
				c->GetTarget()->CastToClient()->Save(2) ? "successfully" : "failed to be"
			).c_str()
		);
	} else if (c->GetTarget()->IsPlayerCorpse()) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} ({}) {} saved.",
				c->GetTarget()->GetCleanName(),
				c->GetTarget()->CastToCorpse()->GetCorpseDBID(),
				c->GetTarget()->CastToMob()->Save() ? "successfully" : "failed to be"
			).c_str()
		);
	}
}

