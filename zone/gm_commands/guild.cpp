#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

#include "../guild_mgr.h"
#include "../doors.h"

void command_guild(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "#guild create [Character ID|Character Name] [Guild Name]");
		c->Message(Chat::White, "#guild delete [Guild ID]");
		c->Message(Chat::White, "#guild help");
		c->Message(Chat::White, "#guild info [Guild ID]");
		c->Message(Chat::White, "#guild list");
		c->Message(Chat::White, "#guild rename [Guild ID] [New Name]");
		c->Message(Chat::White, "#guild set [Character ID|Character Name] [Guild ID] (Guild ID 0 is Guildless)");
		c->Message(Chat::White, "#guild setleader [Guild ID] [Character ID|Character Name]");
		c->Message(Chat::White, "#guild setrank [Character ID|Character Name] [Rank]");
		c->Message(Chat::White, "#guild status [Character ID|Character Name]");
		return;
	}

	auto target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	bool is_create = !strcasecmp(sep->arg[1], "create");
	bool is_delete = !strcasecmp(sep->arg[1], "delete");
	bool is_help = !strcasecmp(sep->arg[1], "help");
	bool is_info = !strcasecmp(sep->arg[1], "info");
	bool is_list = !strcasecmp(sep->arg[1], "list");
	bool is_rename = !strcasecmp(sep->arg[1], "rename");
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
		!is_set &&
		!is_set_leader &&
		!is_set_rank &&
		!is_status
	) {
		c->Message(Chat::White, "#guild create [Character ID|Character Name] [Guild Name]");
		c->Message(Chat::White, "#guild delete [Guild ID]");
		c->Message(Chat::White, "#guild help");
		c->Message(Chat::White, "#guild info [Guild ID]");
		c->Message(Chat::White, "#guild list");
		c->Message(Chat::White, "#guild rename [Guild ID] [New Name]");
		c->Message(Chat::White, "#guild set [Character ID|Character Name] [Guild ID] (Guild ID 0 is Guildless)");
		c->Message(Chat::White, "#guild setleader [Guild ID] [Character ID|Character Name]");
		c->Message(Chat::White, "#guild setrank [Character ID|Character Name] [Rank]");
		c->Message(Chat::White, "#guild status [Character ID|Character Name]");
		return;
	}

	if (is_create) {
		if (arguments < 3) {
			c->Message(Chat::White, "Usage: #guild create [Character ID|Character Name] [Guild Name]");
		} else {
			auto leader_id = (
				sep->IsNumber(2) ?
				std::stoul(sep->arg[2]) :
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
				if (c->Admin() < minStatusToEditOtherGuilds) {
					c->Message(Chat::White, "You cannot edit other peoples' guilds.");
					return;
				}

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
			auto guild_id = std::stoul(sep->arg[2]);
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

			if (c->Admin() < minStatusToEditOtherGuilds) {
				if (c->GuildID() != guild_id) {
					c->Message(Chat::White, "You cannot edit other peoples' guilds.");
					return;
				} else if (!guild_mgr.CheckGMStatus(guild_id, c->Admin())) {
					c->Message(Chat::White, "You cannot edit your current guild, your status is not high enough.");
					return;
				}
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
		c->Message(Chat::White, "#guild create [Character ID|Character Name] [Guild Name]");
		c->Message(Chat::White, "#guild delete [Guild ID]");
		c->Message(Chat::White, "#guild help");
		c->Message(Chat::White, "#guild info [Guild ID]");
		c->Message(Chat::White, "#guild list");
		c->Message(Chat::White, "#guild rename [Guild ID] [New Name]");
		c->Message(Chat::White, "#guild set [Character ID|Character Name] [Guild ID] (Guild ID 0 is Guildless)");
		c->Message(Chat::White, "#guild setleader [Guild ID] [Character ID|Character Name]");
		c->Message(Chat::White, "#guild setrank [Character ID|Character Name] [Rank]");
		c->Message(Chat::White, "#guild status [Character ID|Character Name]");
	} else if (is_info) {
		if (arguments != 2 && c->IsInAGuild()) {
			if (c->Admin() >= minStatusToEditOtherGuilds) {
				c->Message(Chat::White, "#guild info [Guild ID]");
			} else {
				c->Message(Chat::White, "You cannot edit other peoples' guilds.");
			}
		} else {
			auto guild_id = GUILD_NONE;
			if (arguments != 2 || !sep->IsNumber(2)) {
				guild_id = c->GuildID();
			} else if (c->Admin() >= minStatusToEditOtherGuilds) {
				guild_id = std::stoul(sep->arg[2]);
			}

			if (guild_id != GUILD_NONE) {
				guild_mgr.DescribeGuild(c, guild_id);
			}
		}
	} else if (is_list) {
		if (c->Admin() < minStatusToEditOtherGuilds) {
			c->Message(Chat::White, "You cannot edit other peoples' guilds.");
			return;
		}

		guild_mgr.ListGuilds(c);
	} else if (is_rename) {
		if (!sep->IsNumber(2)) {
			c->Message(Chat::White, "Usage: #guild rename [Guild ID] [New Guild Name]");
		} else {
			auto guild_id = std::stoul(sep->arg[2]);
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

			if (c->Admin() < minStatusToEditOtherGuilds) {
				if (c->GuildID() != guild_id) {
					c->Message(Chat::White, "You cannot edit other peoples' guilds.");
					return;
				} else if (!guild_mgr.CheckGMStatus(guild_id, c->Admin())) {
					c->Message(Chat::White, "You cannot edit your current guild, your status is not high enough.");
					return;
				}
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
	} else if (is_set) {
		if (
			arguments != 3 ||
			!sep->IsNumber(3)
		) {
			c->Message(Chat::White, "#guild set [Character ID|Character Name] [Guild ID] (Guild ID 0 is Guildless)");
			return;
		} else {
			auto guild_id = std::stoul(sep->arg[3]);
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
				std::stoul(sep->arg[2]) :
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

			if (c->Admin() < minStatusToEditOtherGuilds && guild_id != c->GuildID()) {
				c->Message(Chat::White, "You cannot edit other peoples' guilds.");
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
				std::stoul(sep->arg[2]) :
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
				auto guild_id = std::stoul(sep->arg[2]);
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

				if (c->Admin() < minStatusToEditOtherGuilds) {					
					if (c->GuildID() != guild_id) {
						c->Message(Chat::White, "You cannot edit other peoples' guilds.");
						return;
					} else if (!guild_mgr.CheckGMStatus(guild_id, c->Admin())) {
						c->Message(Chat::White, "You cannot edit your current guild, your status is not high enough.");
						return;
					}
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
		auto rank = static_cast<uint8>(std::stoul(sep->arg[3]));
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
				std::stoul(sep->arg[2]) :
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

			if (c->Admin() < minStatusToEditOtherGuilds && character_id != c->CharacterID()) {
				c->Message(Chat::White, "You cannot edit other peoples' guilds.");
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
			target ?
			target :
			(
				arguments == 2 ?
				entity_list.GetClientByName(sep->arg[2]) :
				c
			)
		);
		if (!client) {
			c->Message(Chat::White, "You must target someone or specify a character name.");
		} else if (c->Admin() < minStatusToEditOtherGuilds && client->GuildID() != c->GuildID()) {
			c->Message(Chat::White, "You cannot edit other peoples' guilds.");
		} else {
			if (!client->IsInAGuild()) {
				c->Message(
					Chat::White,
					fmt::format(
						"{} {} not in a guild.",
						c->GetTargetDescription(client, TargetDescriptionType::UCYou),
						c == target ? "are" : "is"
					).c_str()
				);
			} else if (guild_mgr.IsGuildLeader(client->GuildID(), client->CharacterID())) {
				c->Message(
					Chat::White,
					fmt::format(
						"{} {} the leader of {}.",
						c->GetTargetDescription(client, TargetDescriptionType::UCYou),
						c == target ? "are" : "is",
						guild_mgr.GetGuildNameByID(client->GuildID())
					).c_str()
				);
			} else {
				c->Message( 
					Chat::White,
					fmt::format(
						"{} {} a(n) {} of {}.",
						c->GetTargetDescription(client, TargetDescriptionType::UCYou),
						c == target ? "are" : "is",
						guild_mgr.GetRankName(client->GuildID(), client->GuildRank()),
						guild_mgr.GetGuildNameByID(client->GuildID())
					).c_str()
				);
			}
		}
	}
}
