#include "../client.h"
#include "../corpse.h"

void command_getplayerburiedcorpsecount(Client *c, const Seperator *sep)
{
	Client *t = c;

	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
		t = c->GetTarget()->CastToClient();
	}
	else {
		c->Message(Chat::White, "You must first select a target!");
		return;
	}

	uint32 CorpseCount = database.GetCharacterBuriedCorpseCount(t->CharacterID());

	if (CorpseCount > 0) {
		c->Message(Chat::White, "Your target has a total of %u buried corpses.", CorpseCount);
	}
	else {
		c->Message(Chat::White, "Your target doesn't have any buried corpses.");
	}

	return;
}

