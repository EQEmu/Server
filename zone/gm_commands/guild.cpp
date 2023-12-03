#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

#include "../guild_mgr.h"
#include "../doors.h"

void command_guild(Client *c, const Seperator *sep)
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
		!is_status
	) {
		SendGuildSubCommands(c);
		return;
	}

	if (is_create) {
		if (arguments < 3) {
			c->Message(Chat::White, "Usage: #guild create [Character ID|Character Name] [Guild Name]");
		} else {
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
				} else {
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
	} else if (is_delete) {
		if (!sep->IsNumber(2)) {
			c->Message(Chat::White, "Usage: #guild delete [Guild ID]");
		} else {
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
	} else if (is_help) {
		SendGuildSubCommands(c);
	} else if (is_info) {
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
	} else if (is_rename) {
		if (!sep->IsNumber(2)) {
			c->Message(Chat::White, "Usage: #guild rename [Guild ID] [New Guild Name]");
		} else {
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
	} else if (is_search) {
		if (Strings::IsNumber(sep->arg[2])) {
			const auto guild_id = Strings::ToUnsignedInt(sep->arg[2]);

			guild_mgr.ListGuilds(c, guild_id);
		} else {
			const std::string search_criteria = sep->argplus[2];

			guild_mgr.ListGuilds(c, search_criteria);
		}
	} else if (is_set) {
		if (
			arguments != 3 ||
			!sep->IsNumber(3)
		) {
			c->Message(Chat::White, "#guild set [Character ID|Character Name] [Guild ID] (Guild ID 0 is Guildless)");
			return;
		} else {
			auto guild_id = Strings::ToUnsignedInt(sep->arg[3]);
			if (!guild_id) {
				guild_id = GUILD_NONE;
			} else if (!guild_mgr.GuildExists(guild_id)) {
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
			} else {
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
	} else if (is_set_leader) {
		if (
			arguments != 3 ||
			!sep->IsNumber(2)
		) {
			c->Message(Chat::White, "Usage: #guild setleader [Guild ID] [Character ID|Character Name]");
		} else {
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
	} else if (is_set_rank) {
		auto rank = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[3]));
		if (!sep->IsNumber(3)) {
			c->Message(Chat::White, "#guild setrank [Character ID|Character Name] [Rank]");
		} else if (rank < 0 || rank > GUILD_MAX_RANK) {
			c->Message(
				Chat::White,
				fmt::format(
					"Guild Ranks are from 0 to {}.",
					GUILD_MAX_RANK
				).c_str()
			);
		} else {
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
	} else if (is_status) {
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
			} else if (guild_mgr.IsGuildLeader(client->GuildID(), client->CharacterID())) {
				c->Message(
					Chat::White,
					fmt::format(
						"{} {} the leader of {}.",
						c->GetTargetDescription(client, TargetDescriptionType::UCYou),
						c == t ? "are" : "is",
						guild_mgr.GetGuildNameByID(client->GuildID())
					).c_str()
				);
			} else {
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
}

void SendGuildSubCommands(Client *c)
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
