#include "../client.h"
#include "../corpse.h"

void command_summonburiedplayercorpse(Client *c, const Seperator *sep)
{
	Client *t = c;

	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
		t = c->GetTarget()->CastToClient();
	}
	else {
		c->Message(Chat::White, "You must first select a target!");
		return;
	}

	Corpse *PlayerCorpse = database.SummonBuriedCharacterCorpses(
		t->CharacterID(),
		t->GetZoneID(),
		zone->GetInstanceID(),
		t->GetPosition());

	if (!PlayerCorpse) {
		c->Message(Chat::White, "Your target doesn't have any buried corpses.");
	}

	return;
}

