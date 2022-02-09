#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_suspend(Client *c, const Seperator *sep)
{
	if ((sep->arg[1][0] == 0) || (sep->arg[2][0] == 0)) {
		c->Message(
			Chat::White,
			"Usage: #suspend <charname> <days> (Specify 0 days to lift the suspension immediately) <message>"
		);
		return;
	}

	int duration = atoi(sep->arg[2]);

	if (duration < 0) {
		duration = 0;
	}

	std::string message;

	if (duration > 0) {
		int i = 3;
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
			c->Message(
				Chat::White,
				"Usage: #suspend <charname> <days>(Specify 0 days to lift the suspension immediately) <message>"
			);
			return;
		}
	}

	auto escName = new char[strlen(sep->arg[1]) * 2 + 1];
	database.DoEscapeString(escName, sep->arg[1], strlen(sep->arg[1]));
	int accountID = database.GetAccountIDByChar(escName);
	safe_delete_array(escName);

	if (accountID <= 0) {
		c->Message(Chat::Red, "Character does not exist.");
		return;
	}

	std::string query   = StringFormat(
		"UPDATE `account` SET `suspendeduntil` = DATE_ADD(NOW(), INTERVAL %i DAY), "
		"suspend_reason = '%s' WHERE `id` = %i",
		duration, EscapeString(message).c_str(), accountID
	);
	auto        results = database.QueryDatabase(query);

	if (duration) {
		c->Message(
			Chat::Red,
			"Account number %i with the character %s has been temporarily suspended for %i day(s).",
			accountID,
			sep->arg[1],
			duration
		);
	}
	else {
		c->Message(
			Chat::Red,
			"Account number %i with the character %s is no longer suspended.",
			accountID,
			sep->arg[1]
		);
	}

	Client *bannedClient = entity_list.GetClientByName(sep->arg[1]);

	if (bannedClient) {
		bannedClient->WorldKick();
		return;
	}

	auto                    pack = new ServerPacket(ServerOP_KickPlayer, sizeof(ServerKickPlayer_Struct));
	ServerKickPlayer_Struct *sks = (ServerKickPlayer_Struct *) pack->pBuffer;

	strn0cpy(sks->adminname, c->GetName(), sizeof(sks->adminname));
	strn0cpy(sks->name, sep->arg[1], sizeof(sks->name));
	sks->adminrank = c->Admin();

	worldserver.SendPacket(pack);

	safe_delete(pack);
}

