#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

#include "../../common/profanity_manager.h"

void command_profanity(Client *c, const Seperator *sep)
{
	std::string arg1(sep->arg[1]);

	while (true) {
		if (arg1.compare("list") == 0) {
			// do nothing
		}
		else if (arg1.compare("clear") == 0) {
			EQ::ProfanityManager::DeleteProfanityList(&database);
			auto pack = new ServerPacket(ServerOP_RefreshCensorship);
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
		else if (arg1.compare("add") == 0) {
			if (!EQ::ProfanityManager::AddProfanity(&database, sep->arg[2])) {
				c->Message(Chat::Red, "Could not add '%s' to the profanity list.", sep->arg[2]);
			}
			auto pack = new ServerPacket(ServerOP_RefreshCensorship);
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
		else if (arg1.compare("del") == 0) {
			if (!EQ::ProfanityManager::RemoveProfanity(&database, sep->arg[2])) {
				c->Message(Chat::Red, "Could not delete '%s' from the profanity list.", sep->arg[2]);
			}
			auto pack = new ServerPacket(ServerOP_RefreshCensorship);
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
		else if (arg1.compare("reload") == 0) {
			if (!EQ::ProfanityManager::UpdateProfanityList(&database)) {
				c->Message(Chat::Red, "Could not reload the profanity list.");
			}
			auto pack = new ServerPacket(ServerOP_RefreshCensorship);
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
		else {
			break;
		}

		std::string     popup;
		const auto      &list = EQ::ProfanityManager::GetProfanityList();
		for (const auto &iter : list) {
			popup.append(iter);
			popup.append("<br>");
		}
		if (list.empty()) {
			popup.append("** Censorship Inactive **<br>");
		}
		else {
			popup.append("** End of List **<br>");
		}

		c->SendPopupToClient("Profanity List", popup.c_str());

		return;
	}

	c->Message(Chat::White, "Usage: #profanity [list] - shows profanity list");
	c->Message(Chat::White, "Usage: #profanity [clear] - deletes all entries");
	c->Message(Chat::White, "Usage: #profanity [add] [<word>] - adds entry");
	c->Message(Chat::White, "Usage: #profanity [del] [<word>] - deletes entry");
	c->Message(Chat::White, "Usage: #profanity [reload] - reloads profanity list");
}

