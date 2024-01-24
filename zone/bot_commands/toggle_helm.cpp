#include "../client.h"
#include "../bot_command.h"

void bot_command_toggle_helm(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_toggle_helm", sep->arg[0], "bottogglehelm"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s ([option: on | off]) ([actionable: target | byname | ownergroup | targetgroup | namesgroup | healrotation | spawned] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = ActionableBots::ABM_NoFilter;

	std::string arg1 = sep->arg[1];

	bool helm_state = false;
	bool toggle_helm = true;
	int ab_arg = 1;
	if (!arg1.compare("on")) {
		helm_state = true;
		toggle_helm = false;
		ab_arg = 2;
	}
	else if (!arg1.compare("off")) {
		toggle_helm = false;
		ab_arg = 2;
	}

	std::list<Bot*> sbl;
	auto ab_type = ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, sep->arg[(ab_arg + 1)]);
	if (ab_type == ActionableBots::ABT_None)
		return;

	int bot_count = 0;
	for (auto bot_iter : sbl) {
		if (!bot_iter)
			continue;

		if (toggle_helm)
			bot_iter->SetShowHelm(!bot_iter->GetShowHelm());
		else
			bot_iter->SetShowHelm(helm_state);

		if (ab_type != ActionableBots::ABT_All) {
			if (!database.botdb.SaveHelmAppearance(c->CharacterID(), bot_iter->GetBotID(), bot_iter->GetShowHelm())) {
				c->Message(Chat::White, "%s for '%s'", bot_iter->GetCleanName());
				return;
			}

			EQApplicationPacket* outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
			SpawnAppearance_Struct* saptr = (SpawnAppearance_Struct*)outapp->pBuffer;
			saptr->spawn_id = bot_iter->GetID();
			saptr->type = AppearanceType::ShowHelm;
			saptr->parameter = bot_iter->GetShowHelm();

			entity_list.QueueClients(bot_iter, outapp);
			safe_delete(outapp);

			//helper_bot_appearance_form_update(bot_iter);
		}
		++bot_count;
	}

	if (ab_type == ActionableBots::ABT_All) {
		if (toggle_helm) {
			if (!database.botdb.ToggleAllHelmAppearances(c->CharacterID()))
				c->Message(Chat::White, "%s", BotDatabase::fail::ToggleAllHelmAppearances());
		}
		else {
			if (!database.botdb.SaveAllHelmAppearances(c->CharacterID(), helm_state))
				c->Message(Chat::White, "%s", BotDatabase::fail::SaveAllHelmAppearances());
		}

		c->Message(Chat::White, "%s all of your bot show helm flags", toggle_helm ? "Toggled" : (helm_state ? "Set" : "Cleared"));
	}
	else {
		c->Message(Chat::White, "%s %i of your spawned bot show helm flags", toggle_helm ? "Toggled" : (helm_state ? "Set" : "Cleared"), bot_count);
	}

	// Notes:
	/*
	[CLIENT OPCODE TEST]
	[10-16-2015 :: 14:57:56] [Packet :: Client -> Server (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A4 02 [2B 00] 01 00 00 00 - showhelm = true (client)
	[10-16-2015 :: 14:57:56] [Packet :: Server -> Client (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A4 02 [2B 00] 01 00 00 00 - showhelm = true (client)

	[10-16-2015 :: 14:58:02] [Packet :: Client -> Server (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A4 02 [2B 00] 00 00 00 00 - showhelm = false (client)
	[10-16-2015 :: 14:58:02] [Packet :: Server -> Client (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A4 02 [2B 00] 00 00 00 00 - showhelm = false (client)

	[BOT OPCODE TEST]
	[10-16-2015 :: 22:15:34] [Packet :: Client -> Server (Dump)] [OP_ChannelMessage - 0x0045] [Size: 167]
	0: 43 6C 65 72 69 63 62 6F - 74 00 00 00 00 00 00 00  | Clericbot.......
	16: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	32: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	48: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	64: 43 6C 65 72 69 63 62 6F - 74 00 00 00 00 00 00 00  | Clericbot.......
	80: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	96: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	112: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	128: 00 00 00 00 08 00 00 00 - CD CD CD CD CD CD CD CD  | ................
	144: 64 00 00 00 23 62 6F 74 - 20 73 68 6F 77 68 65 6C  | d...#bot showhel
	160: 6D 20 6F 6E 00                                     | m on.

	[10-16-2015 :: 22:15:34] [Packet :: Server -> Client (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A2 02 2B 00 01 00 00 00 - showhelm = true

	[10-16-2015 :: 22:15:40] [Packet :: Client -> Server (Dump)] [OP_ChannelMessage - 0x0045] [Size: 168]
	0: 43 6C 65 72 69 63 62 6F - 74 00 00 00 00 00 00 00  | Clericbot.......
	16: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	32: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	48: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	64: 43 6C 65 72 69 63 62 6F - 74 00 00 00 00 00 00 00  | Clericbot.......
	80: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	96: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	112: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
	128: 00 00 00 00 08 00 00 00 - CD CD CD CD CD CD CD CD  | ................
	144: 64 00 00 00 23 62 6F 74 - 20 73 68 6F 77 68 65 6C  | d...#bot showhel
	160: 6D 20 6F 66 66 00                                  | m off.

	[10-16-2015 :: 22:15:40] [Packet :: Server -> Client (Dump)] [OP_SpawnAppearance - 0x01d1] [Size: 10]
	0: A2 02 2B 00 00 00 00 00 - showhelm = false

	*** Bot did not update using the OP_SpawnAppearance packet with AppearanceType::ShowHelm appearance type ***
	*/
}
