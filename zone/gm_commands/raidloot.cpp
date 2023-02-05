#include "../client.h"
#include "../groups.h"
#include "../raids.h"
#include "../raids.h"

void command_raidloot(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #raidloot [All|GroupLeader|RaidLeader|Selected]");
		return;
	}

	auto client_raid = c->GetRaid();
	if (!client_raid) {
		c->Message(Chat::White, "You must be in a Raid to use this command.");
		return;
	}

	if (!client_raid->IsLeader(c)) {
		c->Message(Chat::White, "You must be the Raid Leader to use this command.");
		return;
	}

	std::string raid_loot_type  = Strings::ToLower(sep->arg[1]);
	bool        is_all          = raid_loot_type.find("all") != std::string::npos;
	bool        is_group_leader = raid_loot_type.find("groupleader") != std::string::npos;
	bool        is_raid_leader  = raid_loot_type.find("raidleader") != std::string::npos;
	bool        is_selected     = raid_loot_type.find("selected") != std::string::npos;
	if (
		!is_all &&
		!is_group_leader &&
		!is_raid_leader &&
		!is_selected
		) {
		c->Message(Chat::White, "Usage: #raidloot [All|GroupLeader|RaidLeader|Selected]");
		return;
	}

	std::map<uint32, std::string> loot_types = {
		{RaidLootTypes::All,         "All"},
		{RaidLootTypes::GroupLeader, "GroupLeader"},
		{RaidLootTypes::RaidLeader,  "RaidLeader"},
		{RaidLootTypes::Selected,    "Selected"}
	};

	uint32 loot_type;
	if (is_all) {
		loot_type = RaidLootTypes::All;
	}
	else if (is_group_leader) {
		loot_type = RaidLootTypes::GroupLeader;
	}
	else if (is_raid_leader) {
		loot_type = RaidLootTypes::RaidLeader;
	}
	else if (is_selected) {
		loot_type = RaidLootTypes::Selected;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Loot type changed to {} ({}).",
			loot_types[loot_type],
			loot_type
		).c_str()
	);
}

void command_raidgroupsay(Client* c, const Seperator* sep)
{
	//	const char* msg, Client* c, uint8 language, uint8 lang_skill

	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #raidgroupsay");
		return;
	}
	uint8 language = atoi(sep->arg[1]);
	uint8 lang_skill = atoi(sep->arg[2]);
	uint32 groupToUse = atoi(sep->arg[3]);
	char msg[1000];
	strcpy(msg, sep->arg[4]);

	auto raid = c->GetRaid();
	if (!raid) {
		c->Message(Chat::White, "You must be in a Raid to use this command.");
		return;
	}

	raid->RaidGroupSay(msg, c, 0, 100);

	auto pack = new ServerPacket(ServerOP_RaidGroupSay, sizeof(ServerRaidMessage_Struct) + strlen(msg) + 1);
	ServerRaidMessage_Struct* rga = (ServerRaidMessage_Struct*)pack->pBuffer;
	rga->rid = raid->GetID();
	rga->gid = groupToUse;
	rga->language = language;
	rga->lang_skill = lang_skill;
	strn0cpy(rga->from, c->GetName(), 64);

	strcpy(rga->message, msg); // this is safe because we are allocating enough space for the entire msg above

	worldserver.SendPacket(pack);
	safe_delete(pack);
}
void command_raidsay(Client* c, const Seperator* sep)
{
	//	const char* msg, Client* c, uint8 language, uint8 lang_skill

	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #raidgroupsay");
		return;
	}
	uint8 language = atoi(sep->arg[1]);
	uint8 lang_skill = atoi(sep->arg[2]);
	uint32 groupToUse = atoi(sep->arg[3]);
	char msg[1000];
	strcpy(msg, sep->arg[4]);

	auto raid = c->GetRaid();
	if (!raid) {
		c->Message(Chat::White, "You must be in a Raid to use this command.");
		return;
	}

	raid->RaidSay(msg, c, 0, 100);

	if (!c)
		return;

	auto pack = new ServerPacket(ServerOP_RaidSay, sizeof(ServerRaidMessage_Struct) + strlen(msg) + 1);
	ServerRaidMessage_Struct* rga = (ServerRaidMessage_Struct*)pack->pBuffer;
	rga->rid = raid->GetID();
	rga->gid = 0xFFFFFFFF;
	rga->language = language;
	rga->lang_skill = lang_skill;
	strn0cpy(rga->from, c->GetName(), 64);

	strcpy(rga->message, msg); // this is safe because we are allocating enough space for the entire msg above

	worldserver.SendPacket(pack);
	safe_delete(pack);
}
