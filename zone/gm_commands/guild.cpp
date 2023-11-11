#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

#include "../guild_mgr.h"
#include "../doors.h"

void command_guild(Client* c, const Seperator* sep)
{
	const auto arguments = sep->argnum;
	if (!arguments) {
		SendGuildSubCommands(c);
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	bool is_create = !strcasecmp(sep->arg[1], "create");
	bool is_delete = !strcasecmp(sep->arg[1], "delete");
	bool is_help = !strcasecmp(sep->arg[1], "help");
	bool is_info = !strcasecmp(sep->arg[1], "info");
	bool is_list = !strcasecmp(sep->arg[1], "list");
	bool is_rename = !strcasecmp(sep->arg[1], "rename");
	bool is_search = !strcasecmp(sep->arg[1], "search");
	bool is_set = !strcasecmp(sep->arg[1], "set");
	bool is_set_leader = !strcasecmp(sep->arg[1], "setleader");
	bool is_set_rank = !strcasecmp(sep->arg[1], "setrank");
	bool is_status = !strcasecmp(sep->arg[1], "status");
	bool is_details = !strcasecmp(sep->arg[1], "details");
	bool is_test = !strcasecmp(sep->arg[1], "test");
	if (
		!is_create &&
		!is_delete &&
		!is_help &&
		!is_info &&
		!is_list &&
		!is_rename &&
		!is_search &&
		!is_set &&
		!is_set_leader &&
		!is_set_rank &&
		!is_status &&
		!is_details &&
		!is_test
		) {
		SendGuildSubCommands(c);
		return;
	}

	if (is_create) {
		if (arguments < 3) {
			c->Message(Chat::White, "Usage: #guild create [Character ID|Character Name] [Guild Name]");
		}
		else {
			auto leader_id = (
				sep->IsNumber(2) ?
				Strings::ToUnsignedInt(sep->arg[2]) :
				database.GetCharacterID(sep->arg[2])
				);
			auto leader_name = database.GetCharNameByID(leader_id);
			if (!leader_id || leader_name.empty()) {
				c->Message(
					Chat::White,
					fmt::format(
						"Character ID {} could not be found.",
						leader_id
					).c_str()
				);
				return;
			}

			auto guild_id = guild_mgr.FindGuildByLeader(leader_id);
			if (guild_id != GUILD_NONE) {
				c->Message(
					Chat::White,
					fmt::format(
						"{} ({}) is already the leader of {} ({}).",
						leader_name,
						leader_id,
						guild_mgr.GetGuildNameByID(guild_id),
						guild_id
					).c_str()
				);
			} else {
				auto guild_name = sep->argplus[3];
				auto guild_id = guild_mgr.CreateGuild(sep->argplus[3], leader_id);

				LogGuilds(
					"[{}]: Creating guild [{}] with leader [{}] with GM command. It was given id [{}]",
					c->GetName(),
					guild_name,
					leader_id,
					guild_id
				);

				if (guild_id == GUILD_NONE) {
					c->Message(Chat::White, "Guild creation failed.");
				}
				else {
					c->Message(
						Chat::White,
						fmt::format(
							"Guild Created | Name: {} ({}) Leader: {} ({})",
							guild_name,
							guild_id,
							leader_name,
							leader_id
						).c_str()
					);

					if (!guild_mgr.SetGuild(leader_id, guild_id, GUILD_LEADER)) {
						c->Message(
							Chat::White,
							fmt::format(
								"Unable to put {} ({}) in to {} ({}) in the database.",
								leader_name,
								leader_id,
								guild_name,
								guild_id
							).c_str()
						);

						c->Message(Chat::White, "Note: Run #guild set to resolve this.");
					}
				}
			}
		}
	}
	else if (is_delete) {
		if (!sep->IsNumber(2)) {
			c->Message(Chat::White, "Usage: #guild delete [Guild ID]");
		}
		else {
			auto guild_id = Strings::ToUnsignedInt(sep->arg[2]);
			if (!guild_mgr.GuildExists(guild_id)) {
				c->Message(
					Chat::White,
					fmt::format(
						"Guild  ID {} could not be found.",
						guild_id
					).c_str()
				);
				return;
			}

			LogGuilds(
				"[{}]: Deleting guild [{}] ([{}]) with GM command",
				c->GetName(),
				guild_mgr.GetGuildNameByID(guild_id),
				guild_id
			);

			c->Message(
				Chat::White,
				fmt::format(
					"Guild {} Deleted | Name: {} ({})",
					guild_mgr.DeleteGuild(guild_id) ? "Successfully" : "Unsuccessfully",
					guild_mgr.GetGuildNameByID(guild_id),
					guild_id
				).c_str()
			);
		}
	}
	else if (is_help) {
		SendGuildSubCommands(c);
	}
	else if (is_info) {
		if (arguments != 2 && c->IsInAGuild()) {
			c->Message(Chat::White, "#guild info [Guild ID]");
		} else {
			auto guild_id = GUILD_NONE;
			if (sep->IsNumber(2)) {
				guild_id = Strings::ToUnsignedInt(sep->arg[2]);
			}

			if (guild_id != GUILD_NONE) {
				guild_mgr.DescribeGuild(c, guild_id);
			}
		}
	} else if (is_list) {
		guild_mgr.ListGuilds(c, std::string());
	}
	else if (is_rename) {
		if (!sep->IsNumber(2)) {
			c->Message(Chat::White, "Usage: #guild rename [Guild ID] [New Guild Name]");
		}
		else {
			auto guild_id = Strings::ToUnsignedInt(sep->arg[2]);
			if (!guild_mgr.GuildExists(guild_id)) {
				c->Message(
					Chat::White,
					fmt::format(
						"Guild ID {} could not be found.",
						guild_id
					).c_str()
				);
				return;
			}

			auto new_guild_name = sep->argplus[3];
			LogGuilds(
				"[{}]: Renaming guild [{}] ([{}]) to [{}] with GM command",
				c->GetName(),
				guild_mgr.GetGuildNameByID(guild_id),
				guild_id,
				new_guild_name
			);

			c->Message(
				Chat::White,
				fmt::format(
					"Guild {} Renamed | Name: {} ({})",
					guild_mgr.RenameGuild(guild_id, new_guild_name) ? "Successfully" : "Unsuccessfully",
					new_guild_name,
					guild_id
				).c_str()
			);
		}
	}
	else if (is_search) {
		if (Strings::IsNumber(sep->arg[2])) {
			const auto guild_id = Strings::ToUnsignedInt(sep->arg[2]);

			guild_mgr.ListGuilds(c, guild_id);
		}
		else {
			const std::string search_criteria = sep->argplus[2];

			guild_mgr.ListGuilds(c, search_criteria);
		}
	}
	else if (is_set) {
		if (
			arguments != 3 ||
			!sep->IsNumber(3)
			) {
			c->Message(Chat::White, "#guild set [Character ID|Character Name] [Guild ID] (Guild ID 0 is Guildless)");
			return;
		}
		else {
			auto guild_id = Strings::ToUnsignedInt(sep->arg[3]);
			if (!guild_id) {
				guild_id = GUILD_NONE;
			}
			else if (!guild_mgr.GuildExists(guild_id)) {
				c->Message(
					Chat::White,
					fmt::format(
						"Guild ID {} could not be found.",
						guild_id
					).c_str()
				);
				return;
			}

			auto character_id = (
				sep->IsNumber(2) ?
				Strings::ToUnsignedInt(sep->arg[2]) :
				database.GetCharacterID(sep->arg[2])
				);
			auto character_name = database.GetCharNameByID(character_id);
			if (!character_id || character_name.empty()) {
				c->Message(
					Chat::White,
					fmt::format(
						"Character ID {} could not be found.",
						character_id
					).c_str()
				);
				return;
			}

			if (!guild_id || guild_id == GUILD_NONE) {
				LogGuilds(
					"[{}]: Removing [{}] ([{}]) from guild with GM command",
					c->GetName(),
					character_name,
					character_id
				);
			}
			else {
				LogGuilds(
					"[{}]: Putting [{}] ([{}]) into guild [{}] ([{}]) with GM command",
					c->GetName(),
					character_name,
					character_id,
					guild_mgr.GetGuildNameByID(guild_id),
					guild_id
				);
			}

			if (guild_id && guild_id != GUILD_NONE) {
				c->Message(
					Chat::White,
					fmt::format(
						"{} ({}) has {} put into {} ({}).",
						character_name,
						character_id,
						guild_mgr.SetGuild(character_id, guild_id, GUILD_MEMBER) ? "been" : "failed to be",
						guild_mgr.GetGuildNameByID(guild_id),
						guild_id
					).c_str()
				);
			} else {
				guild_mgr.SetGuild(character_id, GUILD_NONE, 0);
				c->Message(
					Chat::White,
					fmt::format(
						"{} ({}) is now Guildless.",
						character_name,
						character_id
					).c_str()
				);
			}
		}
	}
	else if (is_set_leader) {
		if (
			arguments != 3 ||
			!sep->IsNumber(2)
			) {
			c->Message(Chat::White, "Usage: #guild setleader [Guild ID] [Character ID|Character Name]");
		}
		else {
			auto leader_id = (
				sep->IsNumber(2) ?
				Strings::ToUnsignedInt(sep->arg[2]) :
				database.GetCharacterID(sep->arg[2])
				);
			auto leader_name = database.GetCharNameByID(leader_id);
			if (!leader_id || leader_name.empty()) {
				c->Message(
					Chat::White,
					fmt::format(
						"Character ID {} could not be found.",
						leader_id
					).c_str()
				);
				return;
			}

			auto guild_id = guild_mgr.FindGuildByLeader(leader_id);
			if (guild_id != 0) {
				c->Message(
					Chat::White,
					fmt::format(
						"{} ({}) is already the leader of {} ({}).",
						leader_name,
						leader_id,
						guild_mgr.GetGuildNameByID(guild_id),
						guild_id
					).c_str()
				);
			}
			else {
				auto guild_id = Strings::ToUnsignedInt(sep->arg[2]);
				if (!guild_mgr.GuildExists(guild_id)) {
					c->Message(
						Chat::White,
						fmt::format(
							"Guild ID {} could not be found.",
							guild_id
						).c_str()
					);
					return;
				}

				LogGuilds(
					"[{}]: Setting leader of guild [{}] ([{}]) to [{}] with GM command",
					c->GetName(),
					guild_mgr.GetGuildNameByID(guild_id),
					guild_id,
					leader_id
				);

				c->Message(
					Chat::White,
					fmt::format(
						"Guild Leader {} Changed | Name: {} ({})",
						guild_mgr.SetGuildLeader(guild_id, leader_id) ? "Successfully" : "Unsuccessfully",
						guild_mgr.GetGuildNameByID(guild_id),
						guild_id
					).c_str()
				);
			}
		}
	}
	else if (is_set_rank) {
		auto rank = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[3]));
		if (!sep->IsNumber(3)) {
			c->Message(Chat::White, "#guild setrank [Character ID|Character Name] [Rank]");
		}
		else if (rank < 0 || rank > GUILD_MAX_RANK) {
			c->Message(
				Chat::White,
				fmt::format(
					"Guild Ranks are from 0 to {}.",
					GUILD_MAX_RANK
				).c_str()
			);
		}
		else {
			auto character_id = (
				sep->IsNumber(2) ?
				Strings::ToUnsignedInt(sep->arg[2]) :
				database.GetCharacterID(sep->arg[2])
				);
			auto character_name = database.GetCharNameByID(character_id);
			if (!character_id || character_name.empty()) {
				c->Message(
					Chat::White,
					fmt::format(
						"Character ID {} could not be found.",
						character_id
					).c_str()
				);
				return;
			}

			if (!guild_mgr.IsCharacterInGuild(character_id)) {
				c->Message(
					Chat::White,
					fmt::format(
						"{} ({}) is not in a guild.",
						character_name,
						character_id
					).c_str()
				);
				return;
			}

			LogGuilds(
				"[{}]: Setting [{}] ([{}])'s guild rank to [{}] with GM command",
				c->GetName(),
				sep->arg[2],
				character_id,
				rank
			);

			c->Message(
				Chat::White,
				fmt::format(
					"{} ({}) has {} to rank {} ({}).",
					character_name,
					character_id,
					guild_mgr.SetGuildRank(character_id, rank) ? "been set" : "failed to be set",
					!guild_mgr.GetGuildRankName(guild_mgr.GetGuildIDByCharacterID(character_id), rank).empty() ? guild_mgr.GetGuildRankName(guild_mgr.GetGuildIDByCharacterID(character_id), rank) : "Nameless",
					rank
				).c_str()
			);
		}
	}
	else if (is_status) {
		auto client = (
			t ?
			t :
			(
				arguments == 2 ?
				entity_list.GetClientByName(sep->arg[2]) :
				c
				)
			);
		if (!client) {
			c->Message(Chat::White, "You must target someone or specify a character name.");
		} else {
			if (!client->IsInAGuild()) {
				c->Message(
					Chat::White,
					fmt::format(
						"{} {} not in a guild.",
						c->GetTargetDescription(client, TargetDescriptionType::UCYou),
						c == t ? "are" : "is"
					).c_str()
				);
			}
			else if (guild_mgr.IsGuildLeader(client->GuildID(), client->CharacterID())) {
				c->Message(
					Chat::White,
					fmt::format(
						"{} {} the leader of {}.",
						c->GetTargetDescription(client, TargetDescriptionType::UCYou),
						c == t ? "are" : "is",
						guild_mgr.GetGuildNameByID(client->GuildID())
					).c_str()
				);
			}
			else {
				c->Message(
					Chat::White,
					fmt::format(
						"{} {} a(n) {} of {}.",
						c->GetTargetDescription(client, TargetDescriptionType::UCYou),
						c == t ? "are" : "is",
						guild_mgr.GetRankName(client->GuildID(), client->GuildRank()),
						guild_mgr.GetGuildNameByID(client->GuildID())
					).c_str()
				);
			}
		}
	}
	else if (is_details)
	{
		auto guild_id = Strings::ToUnsignedInt(sep->arg[2]);
		auto guild = guild_mgr.GetGuildByGuildID(guild_id);
		if (!guild) {
			c->Message(Chat::Yellow, fmt::format("Guild {} not found.  #guild list can be used to get guild ids.", guild_id).c_str());
			return;
		}

		if (guild) {
			c->Message(Chat::Yellow, fmt::format("Guild ID:         {}.", sep->arg[2]).c_str());
			c->Message(Chat::Yellow, fmt::format("Guild Name:       {}.", guild->name.c_str()).c_str());
			c->Message(Chat::Yellow, fmt::format("Guild Leader ID:  {}.", guild->leader).c_str());
			c->Message(Chat::Yellow, fmt::format("Guild MinStatus:  {}.", guild->minstatus).c_str());
			c->Message(Chat::Yellow, fmt::format("Guild MOTD:       {}.", guild->motd.c_str()).c_str());
			c->Message(Chat::Yellow, fmt::format("Guild MOTD Setter:{}.", guild->motd_setter.c_str()).c_str());
			c->Message(Chat::Yellow, fmt::format("Guild Channel:    {}.", guild->channel.c_str()).c_str());
			c->Message(Chat::Yellow, fmt::format("Guild URL:        {}.", guild->url.c_str()).c_str());

			for (int i = 1; i <= GUILD_MAX_RANK; i++) {
				c->Message(Chat::Yellow, fmt::format("Guild Rank:       {} - {}.", i, guild->rank_names[i].c_str()).c_str());
			}

			c->Message(Chat::Yellow, "Guild Functions:   {db_id} - {guild_id} - {perm_id} - {perm_value}.");
			for (int i = 1; i <= GUILD_MAX_FUNCTIONS; i++) {
				c->Message(Chat::Yellow, fmt::format("Guild Function:   {} - {} - {} - {}.",
					guild->functions[i].id,
					guild->functions[i].guild_id,
					guild->functions[i].perm_id,
					guild->functions[i].perm_value
				).c_str());
			}
			c->Message(Chat::Yellow, fmt::format("Guild Tribute:  Favor     {}", guild->tribute.favor).c_str());
			c->Message(Chat::Yellow, fmt::format("Guild Tribute:  Tribute 1 {}/{}   - Tribute 2 {}/{}",
				guild->tribute.id_1,
				guild->tribute.id_1_tier,
				guild->tribute.id_2,
				guild->tribute.id_2_tier
			).c_str());
			c->Message(Chat::Yellow, fmt::format("Guild Tribute:  Time Remaining {} - Enabled {}",
				guild->tribute.time_remaining,
				guild->tribute.enabled
			).c_str());
		}
		for (auto& c1 : entity_list.GetClientList()) {
			if (c1.second->GuildID() == guild_id) {
				c->Message(Chat::Yellow, fmt::format("PlayerName: {} ID: {} Rank: {} OptIn: {} DirtyList: {}.",
					c1.second->GetCleanName(),
					c1.second->GuildID(),
					c1.second->GuildRank(),
					c1.second->GuildTributeOptIn(),
					c1.second->GetGuildListDirty()
				).c_str());
			}
		}
	}
	else if (is_test)
	{
		auto guild_id = Strings::ToUnsignedInt(sep->arg[2]);
		auto command3 = Strings::ToUnsignedInt(sep->arg[3]);
		auto command4 = Strings::ToUnsignedInt(sep->arg[4]);
		auto command5 = Strings::ToUnsignedInt(sep->arg[5]);
		auto command6 = std::string(sep->arg[6]);
		auto command7 = std::string(sep->arg[7]);

		auto target = c->GetTarget();
		if (!target) {
			return;
		}

		if (command3 == 1) {
			target->SendAppearancePacket(command4, command5);
			c->Message(Chat::Yellow, "SendAppearancePacket sent.");
		}
		if (command3 == 2) {
			target->SendAppearancePacket(command4, command5, true, false, target->CastToClient(), false);
			c->Message(Chat::Yellow, "SendAppearancePacket sent.");
		}
		if (command3 == 3) {
			c->SendGuildTributes();
			c->Message(Chat::Yellow, "Guild Tributes sent.");
		}
		if (command3 == 4) {
			//#guild test 2 4 60 1
			c->Message(Chat::Yellow, "Guild Tribute Item sent to client.");
			c->DoGuildTributeUpdate();
		}
		if (command3 == 5) {
			//#guild test 2 4 60 1
			c->Message(Chat::Yellow, "Put item {} in slot {} ");
			const EQ::ItemInstance* inst = database.CreateItem(command4, 1);
			c->PutItemInInventory(EQ::invslot::GUILD_TRIBUTE_BEGIN + command5, *inst);
			c->SendItemPacket(EQ::invslot::GUILD_TRIBUTE_BEGIN + command5, inst, (ItemPacketType)guild_id);
		}
		if (command3 == 6) {
			//#guild test 2 6 4 450/451
			c->Message(Chat::Yellow, "Delete item in slot {} ", command4);
			c->DeleteItemInInventory(command5);
		}
		if (command3 == 7) {
			//#guild test 2 7 450/451 0
			c->Message(Chat::Yellow, "Send Delete packet for item in slot {} ", command4);
			//c->DeleteItemInInventory(command4, 0, true, false);
			EQApplicationPacket* outapp;
			outapp = new EQApplicationPacket(OP_DeleteItem, sizeof(DeleteItem_Struct));
			DeleteItem_Struct* delitem = (DeleteItem_Struct*)outapp->pBuffer;
			delitem->from_slot = command4;
			delitem->to_slot = 0xFFFFFFFF;
			delitem->number_in_stack = 0xFFFFFFFF;
			c->QueuePacket(outapp);
			safe_delete(outapp);
		}
		if (command3 == 8) {
			//#guild test 150022 8 450 108
			const EQ::ItemInstance* inst = database.CreateItem(guild_id, 1);
			c->Message(Chat::Yellow, "Put item {} in slot {} ", guild_id, command4);
			//			c->PutItemInInventory(command4, *inst, command5);

			std::string packet = inst->Serialize(command4);
			EmuOpcode opcode = OP_Unknown;
			EQApplicationPacket* outapp = nullptr;
			ItemPacket_Struct* itempacket = nullptr;

			// Construct packet
			opcode = OP_ItemPacket;
			outapp = new EQApplicationPacket(opcode, packet.length() + sizeof(ItemPacket_Struct));
			itempacket = (ItemPacket_Struct*)outapp->pBuffer;
			memcpy(itempacket->SerializedItem, packet.c_str(), packet.length());
			itempacket->PacketType = (ItemPacketType)command5;
		}
		if (command3 == 9) {
			//#guild test 2 6 4 450/451
			c->Message(Chat::Yellow, "Send Guild List");
			c->SendGuildMembers();
		}
		if (command3 == 10) {
			//#guild test 2 6 4 450/451
			c->Message(Chat::Yellow, "Send Guild MoTD");
			c->SendGuildMOTD();
		}
		if (command3 == 11) {
			//#guild test guild_id 11
			c->Message(Chat::Yellow, "11 - Send GuildMembersList()");
			c->SendGuildMembersList();
		}
		if (command3 == 12) {
			//#guild test guild_id 12 rank zone_id name
			c->Message(Chat::Yellow, "12 - SendGuildMemberAdd(uint32 guild_id, uint32 level, uint32 _class, uint32 rank, uint32 spirit, uint32 zone_id, std::string player_name)");
			c->SendGuildMemberAdd(guild_id, 60, 2, command4, 0, command5, command6);
		}
		if (command3 == 13) {
			//#guild test guild_id 13 0 0 name new_name
			c->Message(Chat::Yellow, "13 - SendGuildMemberRename(uint32 guild_id, std::string player_name, std::string new_player_name)");
			c->SendGuildMemberRename(guild_id, command6, command7);
		}
		if (command3 == 14) {
			//#guild test guild_id 14 0 0 name
			c->Message(Chat::Yellow, "14 - SendGuildMemberDelete(uint32 guild_id, std::string player_name)");
			c->SendGuildMemberDelete(guild_id, command6);
		}
		if (command3 == 15) {
			//#guild test guild_id 15 level 0 name
			c->Message(Chat::Yellow, "15 - SendGuildMemberLevel(uint32 guild_id, uint32 level, std::string player_name)");
			c->SendGuildMemberLevel(guild_id, command4, command6);
		}
		if (command3 == 16) {
			//#guild test guild_id 16 rank alt name
			c->Message(Chat::Yellow, "16 - SendGuildMemberRankAltBanker(uint32 guild_id, uint32 rank, std::string player_name, bool alt, bool banker)");
			c->SendGuildMemberRankAltBanker(guild_id, command4, command6, command5, 0);
		}
		if (command3 == 17) {
			//#guild test guild_id 17 0 0 name note
			c->Message(Chat::Yellow, "17 - SendGuildMemberPublicNote(uint32 guild_id, std::string player_name, std::string public_note)");
			c->SendGuildMemberPublicNote(guild_id, command6, command7);
		}
		if (command3 == 18) {
			//#guild test guild_id 18 zone offline name
			c->Message(Chat::Yellow, "18 - SendGuildMemberDetails(uint32 guild_id, uint32 zone_id, uint32 offline_mode, std::string player_name)");
			c->SendGuildMemberDetails(guild_id, command4, command5, command6);
		}
		if (command3 == 19) {
			//#guild test guild_id 19 0 0 guildname
			c->Message(Chat::Yellow, "19 - SendGuildRenameGuild(uint32 guild_id, std::string new_guild_name)");
			c->SendGuildRenameGuild(guild_id, command6);
		}
		if (command3 == 20) {
			//#guild test guild_id 20 reply
			c->Message(Chat::Yellow, "20 - SendGuildMOTD(bool GetGuildMOTDReply)");
			c->SendGuildMOTD(command4);
		}
		if (command3 == 21) {
			//#guild test guild_id 21
			c->Message(Chat::Yellow, "21 - SendGuildURL();");
			c->SendGuildURL();
		}
		if (command3 == 22) {
			//#guild test guild_id 22
			c->Message(Chat::Yellow, "22 - SendGuildChannel();");
			c->SendGuildChannel();
		}
		if (command3 == 23) {
			//#guild test guild_id 22
			c->Message(Chat::Yellow, "22 - SendGuildChannel();");
			c->SendGuildList();
		}
}
}

void SendGuildSubCommands(Client* c)
{
	c->Message(Chat::White, "#guild create [Character ID|Character Name] [Guild Name]");
	c->Message(Chat::White, "#guild delete [Guild ID]");
	c->Message(Chat::White, "#guild help");
	c->Message(Chat::White, "#guild info [Guild ID]");
	c->Message(Chat::White, "#guild list");
	c->Message(Chat::White, "#guild rename [Guild ID] [New Name]");
	c->Message(Chat::White, "#guild search [Search Criteria]");
	c->Message(Chat::White, "#guild set [Character ID|Character Name] [Guild ID] (Guild ID 0 is Guildless)");
	c->Message(Chat::White, "#guild setleader [Guild ID] [Character ID|Character Name]");
	c->Message(Chat::White, "#guild setrank [Character ID|Character Name] [Rank]");
	c->Message(Chat::White, "#guild status [Character ID|Character Name]");
}
