#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_revoke(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0 || sep->arg[2][0] == 0) {
		c->Message(Chat::White, "Usage: #revoke [charname] [1/0]");
		return;
	}

	uint32 characterID = database.GetAccountIDByChar(sep->arg[1]);
	if (characterID == 0) {
		c->Message(Chat::Red, "Character does not exist.");
		return;
	}

	int         flag    = sep->arg[2][0] == '1' ? true : false;
	std::string query   = StringFormat("UPDATE account SET revoked = %d WHERE id = %i", flag, characterID);
	auto        results = database.QueryDatabase(query);

	c->Message(
		Chat::Red,
		"%s account number %i with the character %s.",
		flag ? "Revoking" : "Unrevoking",
		characterID,
		sep->arg[1]
	);

	Client *revokee = entity_list.GetClientByAccID(characterID);
	if (revokee) {
		c->Message(Chat::White, "Found %s in this zone.", revokee->GetName());
		revokee->SetRevoked(flag);
		return;
	}

	c->Message(Chat::Red, "#revoke: Couldn't find %s in this zone, passing request to worldserver.", sep->arg[1]);

	auto         outapp  = new ServerPacket(ServerOP_Revoke, sizeof(RevokeStruct));
	RevokeStruct *revoke = (RevokeStruct *) outapp->pBuffer;
	strn0cpy(revoke->adminname, c->GetName(), 64);
	strn0cpy(revoke->name, sep->arg[1], 64);
	revoke->toggle = flag;
	worldserver.SendPacket(outapp);
	safe_delete(outapp);
}

