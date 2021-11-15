#include "../client.h"
#include "../corpse.h"

void command_corpse(Client *c, const Seperator *sep)
{
	Mob *target = c->GetTarget();

	if (strcasecmp(sep->arg[1], "DeletePlayerCorpses") == 0 && c->Admin() >= commandEditPlayerCorpses) {
		int32 tmp = entity_list.DeletePlayerCorpses();
		if (tmp >= 0) {
			c->Message(Chat::White, "%i corpses deleted.", tmp);
		}
		else {
			c->Message(Chat::White, "DeletePlayerCorpses Error #%i", tmp);
		}
	}
	else if (strcasecmp(sep->arg[1], "delete") == 0) {
		if (target == 0 || !target->IsCorpse()) {
			c->Message(Chat::White, "Error: Target the corpse you wish to delete");
		}
		else if (target->IsNPCCorpse()) {

			c->Message(Chat::White, "Depoping %s.", target->GetName());
			target->CastToCorpse()->Delete();
		}
		else if (c->Admin() >= commandEditPlayerCorpses) {
			c->Message(Chat::White, "Deleting %s.", target->GetName());
			target->CastToCorpse()->Delete();
		}
		else {
			c->Message(Chat::White, "Insufficient status to delete player corpse.");
		}
	}
	else if (strcasecmp(sep->arg[1], "ListNPC") == 0) {
		entity_list.ListNPCCorpses(c);
	}
	else if (strcasecmp(sep->arg[1], "ListPlayer") == 0) {
		entity_list.ListPlayerCorpses(c);
	}
	else if (strcasecmp(sep->arg[1], "DeleteNPCCorpses") == 0) {
		int32 tmp = entity_list.DeleteNPCCorpses();
		if (tmp >= 0) {
			c->Message(Chat::White, "%d corpses deleted.", tmp);
		}
		else {
			c->Message(Chat::White, "DeletePlayerCorpses Error #%d", tmp);
		}
	}
	else if (strcasecmp(sep->arg[1], "charid") == 0 && c->Admin() >= commandEditPlayerCorpses) {
		if (target == 0 || !target->IsPlayerCorpse()) {
			c->Message(Chat::White, "Error: Target must be a player corpse.");
		}
		else if (!sep->IsNumber(2)) {
			c->Message(Chat::White, "Error: charid must be a number.");
		}
		else {
			c->Message(
				Chat::White,
				"Setting CharID=%u on PlayerCorpse '%s'",
				target->CastToCorpse()->SetCharID(atoi(sep->arg[2])),
				target->GetName());
		}
	}
	else if (strcasecmp(sep->arg[1], "ResetLooter") == 0) {
		if (target == 0 || !target->IsCorpse()) {
			c->Message(Chat::White, "Error: Target the corpse you wish to reset");
		}
		else {
			target->CastToCorpse()->ResetLooter();
		}
	}
	else if (strcasecmp(sep->arg[1], "RemoveCash") == 0) {
		if (target == 0 || !target->IsCorpse()) {
			c->Message(Chat::White, "Error: Target the corpse you wish to remove the cash from");
		}
		else if (!target->IsPlayerCorpse() || c->Admin() >= commandEditPlayerCorpses) {
			c->Message(Chat::White, "Removing Cash from %s.", target->GetName());
			target->CastToCorpse()->RemoveCash();
		}
		else {
			c->Message(Chat::White, "Insufficient status to modify player corpse.");
		}
	}
	else if (strcasecmp(sep->arg[1], "InspectLoot") == 0) {
		if (target == 0 || !target->IsCorpse()) {
			c->Message(Chat::White, "Error: Target must be a corpse.");
		}
		else {
			target->CastToCorpse()->QueryLoot(c);
		}
	}
	else if (strcasecmp(sep->arg[1], "lock") == 0) {
		if (target == 0 || !target->IsCorpse()) {
			c->Message(Chat::White, "Error: Target must be a corpse.");
		}
		else {
			target->CastToCorpse()->Lock();
			c->Message(Chat::White, "Locking %s...", target->GetName());
		}
	}
	else if (strcasecmp(sep->arg[1], "unlock") == 0) {
		if (target == 0 || !target->IsCorpse()) {
			c->Message(Chat::White, "Error: Target must be a corpse.");
		}
		else {
			target->CastToCorpse()->UnLock();
			c->Message(Chat::White, "Unlocking %s...", target->GetName());
		}
	}
	else if (strcasecmp(sep->arg[1], "depop") == 0) {
		if (target == 0 || !target->IsPlayerCorpse()) {
			c->Message(Chat::White, "Error: Target must be a player corpse.");
		}
		else if (c->Admin() >= commandEditPlayerCorpses && target->IsPlayerCorpse()) {
			c->Message(Chat::White, "Depoping %s.", target->GetName());
			target->CastToCorpse()->DepopPlayerCorpse();
			if (!sep->arg[2][0] || atoi(sep->arg[2]) != 0) {
				target->CastToCorpse()->Bury();
			}
		}
		else {
			c->Message(Chat::White, "Insufficient status to depop player corpse.");
		}
	}
	else if (strcasecmp(sep->arg[1], "depopall") == 0) {
		if (target == 0 || !target->IsClient()) {
			c->Message(Chat::White, "Error: Target must be a player.");
		}
		else if (c->Admin() >= commandEditPlayerCorpses && target->IsClient()) {
			c->Message(Chat::White, "Depoping %s\'s corpses.", target->GetName());
			target->CastToClient()->DepopAllCorpses();
			if (!sep->arg[2][0] || atoi(sep->arg[2]) != 0) {
				target->CastToClient()->BuryPlayerCorpses();
			}
		}
		else {
			c->Message(Chat::White, "Insufficient status to depop player corpse.");
		}

	}
	else if (strcasecmp(sep->arg[1], "moveallgraveyard") == 0) {
		int count = entity_list.MovePlayerCorpsesToGraveyard(true);
		c->Message(Chat::White, "Moved [%d] player corpse(s) to zone graveyard", count);
	}
	else if (sep->arg[1][0] == 0 || strcasecmp(sep->arg[1], "help") == 0) {
		c->Message(Chat::White, "#Corpse Sub-Commands:");
		c->Message(Chat::White, "  DeleteNPCCorpses");
		c->Message(Chat::White, "  Delete - Delete targetted corpse");
		c->Message(Chat::White, "  ListNPC");
		c->Message(Chat::White, "  ListPlayer");
		c->Message(Chat::White, "  Lock - GM locks the corpse - cannot be looted by non-GM");
		c->Message(Chat::White, "  MoveAllGraveyard - move all player corpses to zone's graveyard or non-instance");
		c->Message(Chat::White, "  UnLock");
		c->Message(Chat::White, "  RemoveCash");
		c->Message(Chat::White, "  InspectLoot");
		c->Message(Chat::White, "  [to remove items from corpses, loot them]");
		c->Message(Chat::White, "Lead-GM status required to delete/modify player corpses");
		c->Message(Chat::White, "  DeletePlayerCorpses");
		c->Message(Chat::White, "  CharID [charid] - change player corpse's owner");
		c->Message(Chat::White, "  Depop [bury] - Depops single target corpse.");
		c->Message(Chat::White, "  Depopall [bury] - Depops all target player's corpses.");
		c->Message(Chat::White, "Set bury to 0 to skip burying the corpses.");
	}
	else {
		c->Message(Chat::White, "Error, #corpse sub-command not found");
	}
}

