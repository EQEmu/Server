#include "../client.h"
#include "../corpse.h"

void command_save(Client *c, const Seperator *sep)
{
	if (c->GetTarget() == 0) {
		c->Message(Chat::White, "Error: no target");
	}
	else if (c->GetTarget()->IsClient()) {
		if (c->GetTarget()->CastToClient()->Save(2)) {
			c->Message(Chat::White, "%s successfully saved.", c->GetTarget()->GetName());
		}
		else {
			c->Message(Chat::White, "Manual save for %s failed.", c->GetTarget()->GetName());
		}
	}
	else if (c->GetTarget()->IsPlayerCorpse()) {
		if (c->GetTarget()->CastToMob()->Save()) {
			c->Message(
				Chat::White,
				"%s successfully saved. (dbid=%u)",
				c->GetTarget()->GetName(),
				c->GetTarget()->CastToCorpse()->GetCorpseDBID());
		}
		else {
			c->Message(Chat::White, "Manual save for %s failed.", c->GetTarget()->GetName());
		}
	}
	else {
		c->Message(Chat::White, "Error: target not a Client/PlayerCorpse");
	}
}

