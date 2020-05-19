/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include "expedition.h"
#include "clientlist.h"
#include "cliententry.h"
#include "zonelist.h"
#include "zoneserver.h"
#include "worlddb.h"
#include "../common/servertalk.h"
#include "../common/string_util.h"

extern ClientList client_list;
extern ZSList zoneserver_list;

void Expedition::PurgeExpiredExpeditions()
{
	std::string query = SQL(
		DELETE expedition FROM expedition_details expedition
			LEFT JOIN instance_list ON expedition.instance_id = instance_list.id
			LEFT JOIN (
				SELECT expedition_id, COUNT(IF(is_current_member = TRUE, 1, NULL)) member_count
				FROM expedition_members
				GROUP BY expedition_id
			) AS expedition_members
			ON expedition_members.expedition_id = expedition.id
		WHERE
			expedition.instance_id IS NULL
			OR expedition_members.member_count = 0
			OR (instance_list.start_time + instance_list.duration) <= UNIX_TIMESTAMP();
	);

	auto results = database.QueryDatabase(query);
	if (!results.Success())
	{
		LogExpeditions("Failed to purge expired and empty expeditions");
	}
}

void Expedition::PurgeExpiredCharacterLockouts()
{
	std::string query = SQL(
		DELETE FROM expedition_character_lockouts
		WHERE expire_time <= NOW();
	);

	auto results = database.QueryDatabase(query);
	if (!results.Success())
	{
		LogExpeditions("Failed to purge expired lockouts");
	}
}

void Expedition::HandleZoneMessage(ServerPacket* pack)
{
	switch (pack->opcode)
	{
	case ServerOP_ExpeditionGetOnlineMembers:
	{
		Expedition::GetOnlineMembers(pack);
		break;
	}
	case ServerOP_ExpeditionDzAddPlayer:
	{
		Expedition::AddPlayer(pack);
		break;
	}
	case ServerOP_ExpeditionDzMakeLeader:
	{
		Expedition::MakeLeader(pack);
		break;
	}
	case ServerOP_ExpeditionRemoveCharLockouts:
	{
		auto buf = reinterpret_cast<ServerExpeditionCharacterName_Struct*>(pack->pBuffer);
		client_list.SendPacket(buf->character_name, pack);
		break;
	}
	case ServerOP_ExpeditionSaveInvite:
	{
		Expedition::SaveInvite(pack);
		break;
	}
	case ServerOP_ExpeditionRequestInvite:
	{
		Expedition::RequestInvite(pack);
		break;
	}
	}
}

void Expedition::AddPlayer(ServerPacket* pack)
{
	auto buf = reinterpret_cast<ServerDzCommand_Struct*>(pack->pBuffer);

	ClientListEntry* invited_cle = client_list.FindCharacter(buf->target_name);
	if (invited_cle && invited_cle->Server())
	{
		// continue in the add target's zone
		buf->is_char_online = true;
		invited_cle->Server()->SendPacket(pack);
	}
	else
	{
		// add target not online, return to inviter
		ClientListEntry* inviter_cle = client_list.FindCharacter(buf->requester_name);
		if (inviter_cle && inviter_cle->Server())
		{
			inviter_cle->Server()->SendPacket(pack);
		}
	}
}

void Expedition::MakeLeader(ServerPacket* pack)
{
	auto buf = reinterpret_cast<ServerDzCommand_Struct*>(pack->pBuffer);

	// notify requester (old leader) and new leader of the result
	ZoneServer* new_leader_zs = nullptr;
	ClientListEntry* new_leader_cle = client_list.FindCharacter(buf->target_name);
	if (new_leader_cle && new_leader_cle->Server())
	{
		buf->is_char_online = true;
		new_leader_zs = new_leader_cle->Server();
		new_leader_zs->SendPacket(pack);
	}

	// if old and new leader are in the same zone only send one message
	ClientListEntry* requester_cle = client_list.FindCharacter(buf->requester_name);
	if (requester_cle && requester_cle->Server() && requester_cle->Server() != new_leader_zs)
	{
		requester_cle->Server()->SendPacket(pack);
	}
}

void Expedition::GetOnlineMembers(ServerPacket* pack)
{
	auto buf = reinterpret_cast<ServerExpeditionCharacters_Struct*>(pack->pBuffer);

	// not efficient but only requested during caching
	char zone_name[64] = {0};
	std::vector<ClientListEntry*> all_clients;
	all_clients.reserve(client_list.GetClientCount());
	client_list.GetClients(zone_name, all_clients);

	for (uint32_t i = 0; i < buf->count; ++i)
	{
		for (const auto& cle : all_clients)
		{
			if (cle && cle->CharID() == buf->entries[i].character_id)
			{
				buf->entries[i].character_zone_id = cle->zone();
				buf->entries[i].character_instance_id = cle->instance();
				buf->entries[i].character_online = true;
				break;
			}
		}
	}

	zoneserver_list.SendPacket(buf->sender_zone_id, buf->sender_instance_id, pack);
}

void Expedition::SaveInvite(ServerPacket* pack)
{
	auto buf = reinterpret_cast<ServerDzCommand_Struct*>(pack->pBuffer);

	ClientListEntry* invited_cle = client_list.FindCharacter(buf->target_name);
	if (invited_cle)
	{
		// store packet on cle and re-send it when client requests it
		buf->is_char_online = true;
		pack->opcode = ServerOP_ExpeditionDzAddPlayer;
		invited_cle->SetPendingExpeditionInvite(pack);
	}
}

void Expedition::RequestInvite(ServerPacket* pack)
{
	auto buf = reinterpret_cast<ServerExpeditionCharacterID_Struct*>(pack->pBuffer);
	ClientListEntry* cle = client_list.FindCLEByCharacterID(buf->character_id);
	if (cle)
	{
		auto invite_pack = cle->GetPendingExpeditionInvite();
		if (invite_pack && cle->Server())
		{
			cle->Server()->SendPacket(invite_pack.get());
		}
	}
}
