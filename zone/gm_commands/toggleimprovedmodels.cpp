#include "../client.h"

void command_toggleimprovedmodels(Client *c, const Seperator *sep)
{
	std::string cur_val = c->GetBucket("DisableFancyModels");

	if (cur_val.empty()) {
		c->SetBucket("DisableFancyModels", "I have no taste.");
		c->Message(Chat::System, "You have disabled improved models. Please immediately use #tim to restore proper function.");
	} else {
		c->DeleteBucket("DisableFancyModels");
		c->Message(Chat::System, "You have regained your senses and re-enabled improved models. Never use this command again.");
	}

	for (auto npc : entity_list.GetNPCList()) {
			EQApplicationPacket depop_packet;
			npc.second->CreateDespawnPacket(&depop_packet, true);
			c->QueuePacket(&depop_packet);

			EQApplicationPacket repop_packet;
			npc.second->CreateSpawnPacket(&repop_packet, npc.second);
			c->QueuePacket(&repop_packet);
	}
}