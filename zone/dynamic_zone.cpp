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

#include "dynamic_zone.h"
#include "client.h"
#include "expedition.h"
#include "worldserver.h"
#include "../common/eqemu_logsys.h"

extern WorldServer worldserver;

// message string 8312 added in September 08 2020 Test patch (used by both dz and shared tasks)
const char* const CREATE_NOT_ALL_ADDED       = "Not all players in your {} were added to the {}. The {} can take a maximum of {} players, and your {} has {}.";

DynamicZone::DynamicZone(
	uint32_t zone_id, uint32_t version, uint32_t duration, DynamicZoneType type)
{
	m_zone_id = zone_id;
	m_zone_version = version;
	m_duration = std::chrono::seconds(duration);
	m_type = type;
}

Database& DynamicZone::GetDatabase()
{
	return database;
}

uint16_t DynamicZone::GetCurrentInstanceID()
{
	return zone ? static_cast<uint16_t>(zone->GetInstanceID()) : 0;
}

uint16_t DynamicZone::GetCurrentZoneID()
{
	return zone ? static_cast<uint16_t>(zone->GetZoneID()) : 0;
}

DynamicZone* DynamicZone::FindDynamicZoneByID(uint32_t dz_id)
{
	auto expedition = Expedition::FindCachedExpeditionByDynamicZoneID(dz_id);
	if (expedition)
	{
		return &expedition->GetDynamicZone();
	}
	// todo: other system caches
	return nullptr;
}

void DynamicZone::StartAllClientRemovalTimers()
{
	for (const auto& client_iter : entity_list.GetClientList())
	{
		if (client_iter.second)
		{
			client_iter.second->SetDzRemovalTimer(true);
		}
	}
}

void DynamicZone::SendInstanceRemoveAllCharacters()
{
	// just remove all clients in bulk instead of only characters assigned to the instance
	if (IsCurrentZoneDzInstance())
	{
		DynamicZone::StartAllClientRemovalTimers();
	}
	else if (GetInstanceID() != 0)
	{
		auto pack = CreateServerRemoveAllCharactersPacket();
		worldserver.SendPacket(pack.get());
	}
}

void DynamicZone::SendInstanceAddRemoveCharacter(uint32_t character_id, bool removed)
{
	// if removing, sets removal timer on client inside the instance
	if (IsCurrentZoneDzInstance())
	{
		Client* client = entity_list.GetClientByCharID(character_id);
		if (client)
		{
			client->SetDzRemovalTimer(removed);
		}
	}
	else if (GetInstanceID() != 0)
	{
		auto pack = CreateServerAddRemoveCharacterPacket(character_id, removed);
		worldserver.SendPacket(pack.get());
	}
}

bool DynamicZone::IsCurrentZoneDzInstance() const
{
	return (zone && zone->GetInstanceID() != 0 && zone->GetInstanceID() == GetInstanceID());
}

void DynamicZone::SetSecondsRemaining(uint32_t seconds_remaining)
{
	// async
	constexpr uint32_t pack_size = sizeof(ServerDzSetDuration_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzSetSecondsRemaining, pack_size);
	auto buf = reinterpret_cast<ServerDzSetDuration_Struct*>(pack->pBuffer);
	buf->dz_id = GetID();
	buf->seconds = seconds_remaining;
	worldserver.SendPacket(pack.get());
}

void DynamicZone::SetUpdatedDuration(uint32_t new_duration)
{
	// preserves original start time, just modifies duration and expire time
	m_duration = std::chrono::seconds(new_duration);
	m_expire_time = m_start_time + m_duration;

	LogDynamicZones("Updated zone [{}]:[{}] seconds remaining: [{}]",
		m_zone_id, m_instance_id, GetSecondsRemaining());

	if (zone && IsCurrentZoneDzInstance())
	{
		zone->SetInstanceTimer(GetSecondsRemaining());
	}
}

void DynamicZone::SendGlobalLocationChange(uint16_t server_opcode, const DynamicZoneLocation& location)
{
	auto pack = CreateServerDzLocationPacket(server_opcode, location);
	worldserver.SendPacket(pack.get());
}

void DynamicZone::HandleWorldMessage(ServerPacket* pack)
{
	switch (pack->opcode)
	{
	case ServerOP_DzAddRemoveCharacter:
	{
		auto buf = reinterpret_cast<ServerDzCharacter_Struct*>(pack->pBuffer);
		Client* client = entity_list.GetClientByCharID(buf->character_id);
		if (client)
		{
			client->SetDzRemovalTimer(buf->remove); // instance kick timer
		}
		break;
	}
	case ServerOP_DzRemoveAllCharacters:
	{
		auto buf = reinterpret_cast<ServerDzCharacter_Struct*>(pack->pBuffer);
		if (buf->remove)
		{
			DynamicZone::StartAllClientRemovalTimers();
		}
		break;
	}
	case ServerOP_DzDurationUpdate:
	{
		auto buf = reinterpret_cast<ServerDzSetDuration_Struct*>(pack->pBuffer);
		auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
		if (dz)
		{
			dz->SetUpdatedDuration(buf->seconds);
		}
		break;
	}
	case ServerOP_DzSetCompass:
	case ServerOP_DzSetSafeReturn:
	case ServerOP_DzSetZoneIn:
	{
		auto buf = reinterpret_cast<ServerDzLocation_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->sender_zone_id, buf->sender_instance_id))
		{
			auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
			if (dz)
			{
				if (pack->opcode == ServerOP_DzSetCompass)
				{
					dz->SetCompass(buf->zone_id, buf->x, buf->y, buf->z, false);
				}
				else if (pack->opcode == ServerOP_DzSetSafeReturn)
				{
					dz->SetSafeReturn(buf->zone_id, buf->x, buf->y, buf->z, buf->heading, false);
				}
				else if (pack->opcode == ServerOP_DzSetZoneIn)
				{
					dz->SetZoneInLocation(buf->x, buf->y, buf->z, buf->heading, false);
				}
			}
		}
		break;
	}
	}
}

std::unique_ptr<EQApplicationPacket> DynamicZone::CreateInfoPacket(bool clear)
{
	constexpr uint32_t outsize = sizeof(DynamicZoneInfo_Struct);
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzExpeditionInfo, outsize);
	if (!clear)
	{
		auto info = reinterpret_cast<DynamicZoneInfo_Struct*>(outapp->pBuffer);
		info->assigned = true;
		strn0cpy(info->dz_name, m_name.c_str(), sizeof(info->dz_name));
		strn0cpy(info->leader_name, m_leader.name.c_str(), sizeof(info->leader_name));
		info->max_players = m_max_players;
	}
	return outapp;
}

std::unique_ptr<EQApplicationPacket> DynamicZone::CreateMemberListPacket(bool clear)
{
	uint32_t member_count = clear ? 0 : static_cast<uint32_t>(m_members.size());
	uint32_t member_entries_size = sizeof(DynamicZoneMemberEntry_Struct) * member_count;
	uint32_t outsize = sizeof(DynamicZoneMemberList_Struct) + member_entries_size;
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzMemberList, outsize);
	auto buf = reinterpret_cast<DynamicZoneMemberList_Struct*>(outapp->pBuffer);

	buf->member_count = member_count;

	if (!clear)
	{
		for (auto i = 0; i < m_members.size(); ++i)
		{
			strn0cpy(buf->members[i].name, m_members[i].name.c_str(), sizeof(buf->members[i].name));
			buf->members[i].online_status = static_cast<uint8_t>(m_members[i].status);
		}
	}

	return outapp;
}

std::unique_ptr<EQApplicationPacket> DynamicZone::CreateMemberListNamePacket(
	const std::string& name, bool remove_name)
{
	constexpr uint32_t outsize = sizeof(DynamicZoneMemberListName_Struct);
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzMemberListName, outsize);
	auto buf = reinterpret_cast<DynamicZoneMemberListName_Struct*>(outapp->pBuffer);
	buf->add_name = !remove_name;
	strn0cpy(buf->name, name.c_str(), sizeof(buf->name));
	return outapp;
}

std::unique_ptr<EQApplicationPacket> DynamicZone::CreateMemberListStatusPacket(
	const std::string& name, DynamicZoneMemberStatus status)
{
	// member list status uses member list struct with a single entry
	constexpr uint32_t outsize = sizeof(DynamicZoneMemberList_Struct) + sizeof(DynamicZoneMemberEntry_Struct);
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzMemberListStatus, outsize);
	auto buf = reinterpret_cast<DynamicZoneMemberList_Struct*>(outapp->pBuffer);
	buf->member_count = 1;

	auto entry = static_cast<DynamicZoneMemberEntry_Struct*>(buf->members);
	strn0cpy(entry->name, name.c_str(), sizeof(entry->name));
	entry->online_status = static_cast<uint8_t>(status);

	return outapp;
}

std::unique_ptr<EQApplicationPacket> DynamicZone::CreateLeaderNamePacket()
{
	constexpr uint32_t outsize = sizeof(DynamicZoneLeaderName_Struct);
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzSetLeaderName, outsize);
	auto buf = reinterpret_cast<DynamicZoneLeaderName_Struct*>(outapp->pBuffer);
	strn0cpy(buf->leader_name, m_leader.name.c_str(), sizeof(buf->leader_name));
	return outapp;
}

void DynamicZone::ProcessCompassChange(const DynamicZoneLocation& location)
{
	DynamicZoneBase::ProcessCompassChange(location);
	SendCompassUpdateToZoneMembers();
}

void DynamicZone::SendCompassUpdateToZoneMembers()
{
	for (const auto& member : m_members)
	{
		Client* member_client = entity_list.GetClientByCharID(member.id);
		if (member_client)
		{
			member_client->SendDzCompassUpdate();
		}
	}
}

void DynamicZone::SendLeaderNameToZoneMembers(std::function<void(Client*)> on_leader_update)
{
	auto outapp_leader = CreateLeaderNamePacket();

	for (const auto& member : m_members)
	{
		Client* member_client = entity_list.GetClientByCharID(member.id);
		if (member_client)
		{
			member_client->QueuePacket(outapp_leader.get());

			if (member.id == m_leader.id && on_leader_update)
			{
				on_leader_update(member_client);
			}
		}
	}
}

void DynamicZone::SendMemberListToZoneMembers()
{
	auto outapp_members = CreateMemberListPacket(false);

	for (const auto& member : m_members)
	{
		Client* member_client = entity_list.GetClientByCharID(member.id);
		if (member_client)
		{
			member_client->QueuePacket(outapp_members.get());
		}
	}
}

void DynamicZone::SendMemberStatusToZoneMembers(uint32_t update_member_id, DynamicZoneMemberStatus status)
{
	if (!HasMember(update_member_id))
	{
		return;
	}

	// if zone already had this member status cached avoid packet update to clients
	bool changed = SetInternalMemberStatus(update_member_id, status);
	if (changed)
	{
		auto member_data = GetMemberData(update_member_id); // rules may override status
		auto outapp_member_status = CreateMemberListStatusPacket(member_data.name, member_data.status);
		for (const auto& member : m_members)
		{
			Client* member_client = entity_list.GetClientByCharID(member.id);
			if (member_client)
			{
				member_client->QueuePacket(outapp_member_status.get());
			}
		}
	}
}
