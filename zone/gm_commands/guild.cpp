#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

#include "../guild_mgr.h"
#include "../doors.h"

void command_guild(Client *c, const Seperator *sep)
{
	int admin   = c->Admin();
	Mob *target = c->GetTarget();

	if (strcasecmp(sep->arg[1], "help") == 0) {
		c->Message(Chat::White, "GM Guild commands:");
		c->Message(Chat::White, "  #guild list - lists all guilds on the server");
		c->Message(Chat::White, "  #guild create {guildleader charname or CharID} guildname");
		c->Message(Chat::White, "  #guild delete guildID");
		c->Message(Chat::White, "  #guild rename guildID newname");
		c->Message(Chat::White, "  #guild set charname guildID    (0=no guild)");
		c->Message(Chat::White, "  #guild setrank charname rank");
		c->Message(Chat::White, "  #guild setleader guildID {guildleader charname or CharID}");
	}
	else if (strcasecmp(sep->arg[1], "status") == 0 || strcasecmp(sep->arg[1], "stat") == 0) {
		Client *client = 0;
		if (sep->arg[2][0] != 0) {
			client = entity_list.GetClientByName(sep->argplus[2]);
		}
		else if (target != 0 && target->IsClient()) {
			client = target->CastToClient();
		}
		if (client == 0) {
			c->Message(Chat::White, "You must target someone or specify a character name");
		}
		else if ((client->Admin() >= minStatusToEditOtherGuilds && admin < minStatusToEditOtherGuilds) &&
				 client->GuildID() != c->GuildID()) { // no peeping for GMs, make sure tell message stays the same
			c->Message(Chat::White, "You must target someone or specify a character name.");
		}
		else {
			if (client->IsInAGuild()) {
				c->Message(Chat::White, "%s is not in a guild.", client->GetName());
			}
			else if (guild_mgr.IsGuildLeader(client->GuildID(), client->CharacterID())) {
				c->Message(
					Chat::White,
					"%s is the leader of <%s> rank: %s",
					client->GetName(),
					guild_mgr.GetGuildName(client->GuildID()),
					guild_mgr.GetRankName(client->GuildID(), client->GuildRank()));
			}
			else {
				c->Message(
					Chat::White,
					"%s is a member of <%s> rank: %s",
					client->GetName(),
					guild_mgr.GetGuildName(client->GuildID()),
					guild_mgr.GetRankName(client->GuildID(), client->GuildRank()));
			}
		}
	}
	else if (strcasecmp(sep->arg[1], "info") == 0) {
		if (sep->arg[2][0] == 0 && c->IsInAGuild()) {
			if (admin >= minStatusToEditOtherGuilds) {
				c->Message(Chat::White, "Usage: #guildinfo guild_id");
			}
			else {
				c->Message(Chat::White, "You're not in a guild");
			}
		}
		else {
			uint32 tmp = GUILD_NONE;
			if (sep->arg[2][0] == 0) {
				tmp = c->GuildID();
			}
			else if (admin >= minStatusToEditOtherGuilds) {
				tmp = atoi(sep->arg[2]);
			}

			if (tmp != GUILD_NONE) {
				guild_mgr.DescribeGuild(c, tmp);
			}
		}
	}
	else if (strcasecmp(sep->arg[1], "set") == 0) {
		if (!sep->IsNumber(3)) {
			c->Message(Chat::White, "Usage: #guild set charname guildgbid (0 = clear guildtag)");
		}
		else {
			uint32 guild_id = atoi(sep->arg[3]);

			if (guild_id == 0) {
				guild_id = GUILD_NONE;
			}
			else if (!guild_mgr.GuildExists(guild_id)) {
				c->Message(Chat::Red, "Guild %d does not exist.", guild_id);
				return;
			}

			uint32 charid = database.GetCharacterID(sep->arg[2]);
			if (charid == 0) {
				c->Message(Chat::Red, "Unable to find character '%s'", charid);
				return;
			}

			//we could do the checking we need for guild_mgr.CheckGMStatus, but im lazy right now
			if (admin < minStatusToEditOtherGuilds) {
				c->Message(Chat::Red, "Access denied.");
				return;
			}

			if (guild_id == GUILD_NONE) {
				LogGuilds("[{}]: Removing [{}] ([{}]) from guild with GM command", c->GetName(), sep->arg[2], charid);
			}
			else {
				LogGuilds("[{}]: Putting [{}] ([{}]) into guild [{}] ([{}]) with GM command",
						  c->GetName(),
						  sep->arg[2],
						  charid,
						  guild_mgr.GetGuildName(guild_id),
						  guild_id);
			}

			if (!guild_mgr.SetGuild(charid, guild_id, GUILD_MEMBER)) {
				c->Message(Chat::Red, "Error putting '%s' into guild %d", sep->arg[2], guild_id);
			}
			else {
				c->Message(Chat::White, "%s has been put into guild %d", sep->arg[2], guild_id);
			}
		}
	}
		/*else if (strcasecmp(sep->arg[1], "setdoor") == 0 && admin >= minStatusToEditOtherGuilds) {

			if (!sep->IsNumber(2))
				c->Message(Chat::White, "Usage: #guild setdoor guildEQid (0 = delete guilddoor)");
			else {
	// guild doors
				if((!guilds[atoi(sep->arg[2])].databaseID) && (atoi(sep->arg[2])!=0) )
				{

					c->Message(Chat::White, "These is no guild with this guildEQid");
				}
				else {
					c->SetIsSettingGuildDoor(true);
					c->Message(Chat::White, "Click on a door you want to become a guilddoor");
					c->SetSetGuildDoorID(atoi(sep->arg[2]));
				}
			}
		}*/
	else if (strcasecmp(sep->arg[1], "setrank") == 0) {
		int rank = atoi(sep->arg[3]);
		if (!sep->IsNumber(3)) {
			c->Message(Chat::White, "Usage: #guild setrank charname rank");
		}
		else if (rank < 0 || rank > GUILD_MAX_RANK) {
			c->Message(Chat::White, "Error: invalid rank #.");
		}
		else {
			uint32 charid = database.GetCharacterID(sep->arg[2]);
			if (charid == 0) {
				c->Message(Chat::Red, "Unable to find character '%s'", charid);
				return;
			}

			//we could do the checking we need for guild_mgr.CheckGMStatus, but im lazy right now
			if (admin < minStatusToEditOtherGuilds) {
				c->Message(Chat::Red, "Access denied.");
				return;
			}

			LogGuilds("[{}]: Setting [{}] ([{}])'s guild rank to [{}] with GM command",
					  c->GetName(),
					  sep->arg[2],
					  charid,
					  rank);

			if (!guild_mgr.SetGuildRank(charid, rank)) {
				c->Message(Chat::Red, "Error while setting rank %d on '%s'.", rank, sep->arg[2]);
			}
			else {
				c->Message(Chat::White, "%s has been set to rank %d", sep->arg[2], rank);
			}
		}
	}
	else if (strcasecmp(sep->arg[1], "create") == 0) {
		if (sep->arg[3][0] == 0) {
			c->Message(Chat::White, "Usage: #guild create {guildleader charname or CharID} guild name");
		}
		else if (!worldserver.Connected()) {
			c->Message(Chat::White, "Error: World server dirconnected");
		}
		else {
			uint32 leader = 0;
			if (sep->IsNumber(2)) {
				leader = atoi(sep->arg[2]);
			}
			else if ((leader = database.GetCharacterID(sep->arg[2])) != 0) {
				//got it from the db..
			}
			else {
				c->Message(Chat::Red, "Unable to find char '%s'", sep->arg[2]);
				return;
			}
			if (leader == 0) {
				c->Message(Chat::White, "Guild leader not found.");
				return;
			}

			uint32 tmp = guild_mgr.FindGuildByLeader(leader);
			if (tmp != GUILD_NONE) {
				c->Message(
					Chat::White,
					"Error: %s already is the leader of DB# %i '%s'.",
					sep->arg[2],
					tmp,
					guild_mgr.GetGuildName(tmp));
			}
			else {

				if (admin < minStatusToEditOtherGuilds) {
					c->Message(Chat::Red, "Access denied.");
					return;
				}

				uint32 id = guild_mgr.CreateGuild(sep->argplus[3], leader);

				LogGuilds("[{}]: Creating guild [{}] with leader [{}] with GM command. It was given id [{}]",
						  c->GetName(),
						  sep->argplus[3],
						  leader,
						  (unsigned long) id);

				if (id == GUILD_NONE) {
					c->Message(Chat::White, "Guild creation failed.");
				}
				else {
					c->Message(Chat::White, "Guild created: Leader: %i, number %i: %s", leader, id, sep->argplus[3]);

					if (!guild_mgr.SetGuild(leader, id, GUILD_LEADER)) {
						c->Message(
							Chat::White,
							"Unable to set guild leader's guild in the database. Your going to have to run #guild set"
						);
					}
				}

			}
		}
	}
	else if (strcasecmp(sep->arg[1], "delete") == 0) {
		if (!sep->IsNumber(2)) {
			c->Message(Chat::White, "Usage: #guild delete guildID");
		}
		else if (!worldserver.Connected()) {
			c->Message(Chat::White, "Error: World server dirconnected");
		}
		else {
			uint32 id = atoi(sep->arg[2]);

			if (!guild_mgr.GuildExists(id)) {
				c->Message(Chat::White, "Guild %d does not exist!", id);
				return;
			}

			if (admin < minStatusToEditOtherGuilds) {
				//this person is not allowed to just edit any guild, check this guild's min status.
				if (c->GuildID() != id) {
					c->Message(Chat::Red, "Access denied to edit other people's guilds");
					return;
				}
				else if (!guild_mgr.CheckGMStatus(id, admin)) {
					c->Message(Chat::Red, "Access denied to edit your guild with GM commands.");
					return;
				}
			}

			LogGuilds("[{}]: Deleting guild [{}] ([{}]) with GM command", c->GetName(),
					  guild_mgr.GetGuildName(id), id);

			if (!guild_mgr.DeleteGuild(id)) {
				c->Message(Chat::White, "Guild delete failed.");
			}
			else {
				c->Message(Chat::White, "Guild %d deleted.", id);
			}
		}
	}
	else if (strcasecmp(sep->arg[1], "rename") == 0) {
		if ((!sep->IsNumber(2)) || sep->arg[3][0] == 0) {
			c->Message(Chat::White, "Usage: #guild rename guildID newname");
		}
		else if (!worldserver.Connected()) {
			c->Message(Chat::White, "Error: World server dirconnected");
		}
		else {
			uint32 id = atoi(sep->arg[2]);

			if (!guild_mgr.GuildExists(id)) {
				c->Message(Chat::White, "Guild %d does not exist!", id);
				return;
			}

			if (admin < minStatusToEditOtherGuilds) {
				//this person is not allowed to just edit any guild, check this guild's min status.
				if (c->GuildID() != id) {
					c->Message(Chat::Red, "Access denied to edit other people's guilds");
					return;
				}
				else if (!guild_mgr.CheckGMStatus(id, admin)) {
					c->Message(Chat::Red, "Access denied to edit your guild with GM commands.");
					return;
				}
			}

			LogGuilds("[{}]: Renaming guild [{}] ([{}]) to [{}] with GM command", c->GetName(),
					  guild_mgr.GetGuildName(id), id, sep->argplus[3]);

			if (!guild_mgr.RenameGuild(id, sep->argplus[3])) {
				c->Message(Chat::White, "Guild rename failed.");
			}
			else {
				c->Message(Chat::White, "Guild %d renamed to %s", id, sep->argplus[3]);
			}
		}
	}
	else if (strcasecmp(sep->arg[1], "setleader") == 0) {
		if (sep->arg[3][0] == 0 || !sep->IsNumber(2)) {
			c->Message(Chat::White, "Usage: #guild setleader guild_id {guildleader charname or CharID}");
		}
		else if (!worldserver.Connected()) {
			c->Message(Chat::White, "Error: World server dirconnected");
		}
		else {
			uint32 leader = 0;
			if (sep->IsNumber(2)) {
				leader = atoi(sep->arg[2]);
			}
			else if ((leader = database.GetCharacterID(sep->arg[2])) != 0) {
				//got it from the db..
			}
			else {
				c->Message(Chat::Red, "Unable to find char '%s'", sep->arg[2]);
				return;
			}

			uint32 tmpdb = guild_mgr.FindGuildByLeader(leader);
			if (leader == 0) {
				c->Message(Chat::White, "New leader not found.");
			}
			else if (tmpdb != 0) {
				c->Message(Chat::White, "Error: %s already is the leader of guild # %i", sep->arg[2], tmpdb);
			}
			else {
				uint32 id = atoi(sep->arg[2]);

				if (!guild_mgr.GuildExists(id)) {
					c->Message(Chat::White, "Guild %d does not exist!", id);
					return;
				}

				if (admin < minStatusToEditOtherGuilds) {
					//this person is not allowed to just edit any guild, check this guild's min status.
					if (c->GuildID() != id) {
						c->Message(Chat::Red, "Access denied to edit other people's guilds");
						return;
					}
					else if (!guild_mgr.CheckGMStatus(id, admin)) {
						c->Message(Chat::Red, "Access denied to edit your guild with GM commands.");
						return;
					}
				}

				LogGuilds("[{}]: Setting leader of guild [{}] ([{}]) to [{}] with GM command", c->GetName(),
						  guild_mgr.GetGuildName(id), id, leader);

				if (!guild_mgr.SetGuildLeader(id, leader)) {
					c->Message(Chat::White, "Guild leader change failed.");
				}
				else {
					c->Message(Chat::White, "Guild leader changed: guild # %d, Leader: %s", id, sep->argplus[3]);
				}
			}
		}
	}
	else if (strcasecmp(sep->arg[1], "list") == 0) {
		if (admin < minStatusToEditOtherGuilds) {
			c->Message(Chat::Red, "Access denied.");
			return;
		}
		guild_mgr.ListGuilds(c);
	}
	else {
		c->Message(Chat::White, "Unknown guild command, try #guild help");
	}
}
/*
bool helper_guild_edit(Client *c, uint32 dbid, uint32 eqid, uint8 rank, const char* what, const char* value) {
	struct GuildRankLevel_Struct grl;
	strcpy(grl.rankname, guild_mgr.GetRankName(eqid, rank));
	grl.demote = guilds[eqid].rank[rank].demote;
	grl.heargu = guilds[eqid].rank[rank].heargu;
	grl.invite = guilds[eqid].rank[rank].invite;
	grl.motd = guilds[eqid].rank[rank].motd;
	grl.promote = guilds[eqid].rank[rank].promote;
	grl.remove = guilds[eqid].rank[rank].remove;
	grl.speakgu = guilds[eqid].rank[rank].speakgu;
	grl.warpeace = guilds[eqid].rank[rank].warpeace;

	if (strcasecmp(what, "title") == 0) {
		if (strlen(value) > 100)
			c->Message(Chat::White, "Error: Title has a maxium length of 100 characters.");
		else
			strcpy(grl.rankname, value);
	}
	else if (rank == 0)
		c->Message(Chat::White, "Error: Rank 0's permissions can not be changed.");
	else {
		if (!(strlen(value) == 1 && (value[0] == '0' || value[0] == '1')))

			return false;
		if (strcasecmp(what, "demote") == 0)
			grl.demote = (value[0] == '1');
		else if (strcasecmp(what, "heargu") == 0)
			grl.heargu = (value[0] == '1');
		else if (strcasecmp(what, "invite") == 0)
			grl.invite = (value[0] == '1');
		else if (strcasecmp(what, "motd") == 0)
			grl.motd = (value[0] == '1');
		else if (strcasecmp(what, "promote") == 0)
			grl.promote = (value[0] == '1');
		else if (strcasecmp(what, "remove") == 0)

			grl.remove = (value[0] == '1');
		else if (strcasecmp(what, "speakgu") == 0)
			grl.speakgu = (value[0] == '1');
		else if (strcasecmp(what, "warpeace") == 0)
			grl.warpeace = (value[0] == '1');
		else
			c->Message(Chat::White, "Error: Permission name not recognized.");
	}
	if (!database.EditGuild(dbid, rank, &grl))
		c->Message(Chat::White, "Error: database.EditGuild() failed");
	return true;
}*/

