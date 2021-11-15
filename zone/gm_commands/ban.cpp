#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_ban(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0 || sep->arg[2][0] == 0) {
		c->Message(Chat::White, "Usage: #ban <charname> <message>");
		return;
	}

	auto account_id = database.GetAccountIDByChar(sep->arg[1]);

	std::string message;
	int         i   = 2;
	while (1) {
		if (sep->arg[i][0] == 0) {
			break;
		}

		if (message.length() > 0) {
			message.push_back(' ');
		}

		message += sep->arg[i];
		++i;
	}

	if (message.length() == 0) {
		c->Message(Chat::White, "Usage: #ban <charname> <message>");
		return;
	}

	if (account_id == 0) {
		c->Message(Chat::Red, "Character does not exist.");
		return;
	}

	std::string query   = StringFormat(
		"UPDATE account SET status = -2, ban_reason = '%s' "
		"WHERE id = %i", EscapeString(message).c_str(), account_id
	);
	auto        results = database.QueryDatabase(query);

	c->Message(
		Chat::Red,
		"Account number %i with the character %s has been banned with message: \"%s\"",
		account_id,
		sep->arg[1],
		message.c_str());

	ServerPacket flagUpdatePack(ServerOP_FlagUpdate, 6);
	*((uint32 *) &flagUpdatePack.pBuffer[0]) = account_id;
	*((int16 *) &flagUpdatePack.pBuffer[4])  = -2;
	worldserver.SendPacket(&flagUpdatePack);

	Client *client = nullptr;
	client                       = entity_list.GetClientByName(sep->arg[1]);
	if (client) {
		client->WorldKick();
		return;
	}

	ServerPacket            kickPlayerPack(ServerOP_KickPlayer, sizeof(ServerKickPlayer_Struct));
	ServerKickPlayer_Struct *skp = (ServerKickPlayer_Struct *) kickPlayerPack.pBuffer;
	strcpy(skp->adminname, c->GetName());
	strcpy(skp->name, sep->arg[1]);
	skp->adminrank = c->Admin();
	worldserver.SendPacket(&kickPlayerPack);
}

