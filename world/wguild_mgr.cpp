/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "../common/eqemu_logsys.h"
#include "../common/global_define.h"
#include "wguild_mgr.h"
#include "../common/servertalk.h"
#include "clientlist.h"
#include "zonelist.h"


extern ClientList client_list;
extern ZSList zoneserver_list;



WorldGuildManager guild_mgr;



void WorldGuildManager::SendGuildRefresh(uint32 guild_id, bool name, bool motd, bool rank, bool relation) {
	LogGuilds("Broadcasting guild refresh for [{}], changes: name=[{}], motd=[{}], rank=d, relation=[{}]", guild_id, name, motd, rank, relation);
	auto pack = new ServerPacket(ServerOP_RefreshGuild, sizeof(ServerGuildRefresh_Struct));
	ServerGuildRefresh_Struct *s = (ServerGuildRefresh_Struct *) pack->pBuffer;
	s->guild_id = guild_id;
	s->name_change = name;
	s->motd_change = motd;
	s->rank_change = rank;
	s->relation_change = relation;
	zoneserver_list.SendPacket(pack);
	safe_delete(pack);
}

void WorldGuildManager::SendCharRefresh(uint32 old_guild_id, uint32 guild_id, uint32 charid) {
	LogGuilds("Broadcasting char refresh for [{}] from guild [{}] to world", charid, guild_id);
	auto pack = new ServerPacket(ServerOP_GuildCharRefresh, sizeof(ServerGuildCharRefresh_Struct));
	ServerGuildCharRefresh_Struct *s = (ServerGuildCharRefresh_Struct *) pack->pBuffer;
	s->guild_id = guild_id;
	s->old_guild_id = old_guild_id;
	s->char_id = charid;
	zoneserver_list.SendPacket(pack);
	safe_delete(pack);
}

void WorldGuildManager::SendGuildDelete(uint32 guild_id) {
	LogGuilds("Broadcasting guild delete for guild [{}] to world", guild_id);
	auto pack = new ServerPacket(ServerOP_DeleteGuild, sizeof(ServerGuildID_Struct));
	ServerGuildID_Struct *s = (ServerGuildID_Struct *) pack->pBuffer;
	s->guild_id = guild_id;
	zoneserver_list.SendPacket(pack);
	safe_delete(pack);
}

void WorldGuildManager::ProcessZonePacket(ServerPacket *pack) {
	switch(pack->opcode) {

	case ServerOP_RefreshGuild: {
		if(pack->size != sizeof(ServerGuildRefresh_Struct)) {
			LogGuilds("Received ServerOP_RefreshGuild of incorrect size [{}], expected [{}]", pack->size, sizeof(ServerGuildRefresh_Struct));
			return;
		}
		ServerGuildRefresh_Struct *s = (ServerGuildRefresh_Struct *) pack->pBuffer;
		LogGuilds("Received and broadcasting guild refresh for [{}], changes: name=[{}], motd=[{}], rank=d, relation=[{}]", s->guild_id, s->name_change, s->motd_change, s->rank_change, s->relation_change);

		//broadcast this packet to all zones.
		zoneserver_list.SendPacket(pack);

		//preform a local refresh.
		if(!RefreshGuild(s->guild_id)) {
			LogGuilds("Unable to preform local refresh on guild [{}]", s->guild_id);
			//can we do anything?
		}

		break;
	}

	case ServerOP_GuildCharRefresh: {
		if(pack->size != sizeof(ServerGuildCharRefresh_Struct)) {
			LogGuilds("Received ServerOP_RefreshGuild of incorrect size [{}], expected [{}]", pack->size, sizeof(ServerGuildCharRefresh_Struct));
			return;
		}
		ServerGuildCharRefresh_Struct *s = (ServerGuildCharRefresh_Struct *) pack->pBuffer;
		LogGuilds("Received and broadcasting guild member refresh for char [{}] to all zones with members of guild [{}]", s->char_id, s->guild_id);

		//preform the local update
		client_list.UpdateClientGuild(s->char_id, s->guild_id);

		//broadcast this update to any zone with a member in this guild.
		//client_list.SendGuildPacket(s->guild_id, pack);
		//because im sick of this not working, sending it to all zones, just spends a bit more bandwidth.
		zoneserver_list.SendPacket(pack);

		break;
	}

	case ServerOP_DeleteGuild: {
		if(pack->size != sizeof(ServerGuildID_Struct)) {
			LogGuilds("Received ServerOP_DeleteGuild of incorrect size [{}], expected [{}]", pack->size, sizeof(ServerGuildID_Struct));
			return;
		}
		ServerGuildID_Struct *s = (ServerGuildID_Struct *) pack->pBuffer;
		LogGuilds("Received and broadcasting guild delete for guild [{}]", s->guild_id);

		//broadcast this packet to all zones.
		zoneserver_list.SendPacket(pack);

		//preform a local refresh.
		if(!LocalDeleteGuild(s->guild_id)) {
			LogGuilds("Unable to preform local delete on guild [{}]", s->guild_id);
			//can we do anything?
		}

		break;
	}

	case ServerOP_GuildMemberUpdate: {
		if(pack->size != sizeof(ServerGuildMemberUpdate_Struct))
		{
			LogGuilds("Received ServerOP_GuildMemberUpdate of incorrect size [{}], expected [{}]", pack->size, sizeof(ServerGuildMemberUpdate_Struct));
			return;
		}

		zoneserver_list.SendPacket(pack);

		break;
	}

	default:
		LogGuilds("Unknown packet {:#04x} received from zone??", pack->opcode);
		break;
	}
}

