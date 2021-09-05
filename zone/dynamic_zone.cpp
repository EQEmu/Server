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
#include "string_ids.h"
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

bool DynamicZone::SendServerPacket(ServerPacket* packet)
{
	return worldserver.SendPacket(packet);
}

uint16_t DynamicZone::GetCurrentInstanceID()
{
	return zone ? static_cast<uint16_t>(zone->GetInstanceID()) : 0;
}

uint16_t DynamicZone::GetCurrentZoneID()
{
	return zone ? static_cast<uint16_t>(zone->GetZoneID()) : 0;
}

DynamicZone* DynamicZone::CreateNew(DynamicZone& dz_request, const std::vector<DynamicZoneMember>& members)
{
	if (!zone || dz_request.GetID() != 0)
	{
		return nullptr;
	}

	// this creates a new dz instance and saves it to both db and cache
	uint32_t dz_id = dz_request.Create();
	if (dz_id == 0)
	{
		LogDynamicZones("Failed to create dynamic zone for zone [{}]", dz_request.GetZoneID());
		return nullptr;
	}

	auto dz = std::make_unique<DynamicZone>(dz_request);
	if (!members.empty())
	{
		dz->SaveMembers(members);
	}

	LogDynamicZones("Created new dz [{}] for zone [{}]", dz_id, dz_request.GetZoneID());

	// world must be notified before we request async member updates
	auto pack = dz->CreateServerDzCreatePacket(zone->GetZoneID(), zone->GetInstanceID());
	worldserver.SendPacket(pack.get());

	auto inserted = zone->dynamic_zone_cache.emplace(dz_id, std::move(dz));

	// expeditions invoke their own updates after installing client update callbacks
	if (inserted.first->second->GetType() != DynamicZoneType::Expedition)
	{
		inserted.first->second->DoAsyncZoneMemberUpdates();
	}

	return inserted.first->second.get();
}

void DynamicZone::CacheNewDynamicZone(ServerPacket* pack)
{
	auto buf = reinterpret_cast<ServerDzCreateSerialized_Struct*>(pack->pBuffer);

	// caching new dz created in world or another zone (has member statuses set by world)
	auto dz = std::make_unique<DynamicZone>();
	dz->LoadSerializedDzPacket(buf->cereal_data, buf->cereal_size);

	uint32_t dz_id = dz->GetID();
	auto inserted = zone->dynamic_zone_cache.emplace(dz_id, std::move(dz));

	// expeditions invoke their own updates after installing client update callbacks
	if (inserted.first->second->GetType() != DynamicZoneType::Expedition)
	{
		inserted.first->second->DoAsyncZoneMemberUpdates();
	}

	LogDynamicZones("Cached new dynamic zone [{}]", dz_id);
}

void DynamicZone::CacheAllFromDatabase()
{
	if (!zone)
	{
		return;
	}

	BenchTimer bench;

	auto dynamic_zones = DynamicZonesRepository::AllWithInstanceNotExpired(database);
	auto dynamic_zone_members = DynamicZoneMembersRepository::GetAllWithNames(database);

	zone->dynamic_zone_cache.clear();
	zone->dynamic_zone_cache.reserve(dynamic_zones.size());

	for (auto& entry : dynamic_zones)
	{
		uint32_t dz_id = entry.id;
		auto dz = std::make_unique<DynamicZone>(std::move(entry));

		for (auto& member : dynamic_zone_members)
		{
			if (member.dynamic_zone_id == dz_id)
			{
				dz->AddMemberFromRepositoryResult(std::move(member));
			}
		}

		zone->dynamic_zone_cache.emplace(dz_id, std::move(dz));
	}

	LogDynamicZones("Caching [{}] dynamic zone(s) took [{}s]", zone->dynamic_zone_cache.size(), bench.elapsed());
}

DynamicZone* DynamicZone::FindDynamicZoneByID(uint32_t dz_id)
{
	if (!zone)
	{
		return nullptr;
	}

	auto dz = zone->dynamic_zone_cache.find(dz_id);
	if (dz != zone->dynamic_zone_cache.end())
	{
		return dz->second.get();
	}

	return nullptr;
}

void DynamicZone::RegisterOnClientAddRemove(std::function<void(Client*, bool, bool)> on_client_addremove)
{
	m_on_client_addremove = std::move(on_client_addremove);
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

	LogDynamicZones("Updated dz [{}] zone [{}]:[{}] seconds remaining: [{}]",
		m_id, m_zone_id, m_instance_id, GetSecondsRemaining());

	if (zone && IsCurrentZoneDzInstance())
	{
		zone->SetInstanceTimer(GetSecondsRemaining());
	}
}

void DynamicZone::HandleWorldMessage(ServerPacket* pack)
{
	switch (pack->opcode)
	{
	case ServerOP_DzCreated:
	{
		auto buf = reinterpret_cast<ServerDzCreateSerialized_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->origin_zone_id, buf->origin_instance_id))
		{
			DynamicZone::CacheNewDynamicZone(pack);
		}
		break;
	}
	case ServerOP_DzDeleted:
	{
		// sent by world when it deletes an expired or empty dz
		// any system that held a reference to the dz should have already been notified
		auto buf = reinterpret_cast<ServerDzID_Struct*>(pack->pBuffer);
		auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
		if (zone && dz)
		{
			dz->SendUpdatesToZoneMembers(true, true); // members silently removed

			// manually handle expeditions to remove any references before the dz is deleted
			if (dz->GetType() == DynamicZoneType::Expedition)
			{
				auto expedition = Expedition::FindCachedExpeditionByDynamicZoneID(dz->GetID());
				if (expedition)
				{
					LogExpeditionsModerate("Deleting expedition [{}] from zone cache", expedition->GetID());
					zone->expedition_cache.erase(expedition->GetID());
				}
			}

			LogDynamicZonesDetail("Deleting dynamic zone [{}] from zone cache", buf->dz_id);
			zone->dynamic_zone_cache.erase(buf->dz_id);
		}
		break;
	}
	case ServerOP_DzAddRemoveMember:
	{
		auto buf = reinterpret_cast<ServerDzMember_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->sender_zone_id, buf->sender_instance_id))
		{
			auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
			if (dz)
			{
				auto status = static_cast<DynamicZoneMemberStatus>(buf->character_status);
				dz->ProcessMemberAddRemove({ buf->character_id, buf->character_name, status }, buf->removed);
			}
		}

		if (zone && zone->IsZone(buf->dz_zone_id, buf->dz_instance_id))
		{
			// cache independent redundancy to kick removed members from dz's instance
			Client* client = entity_list.GetClientByCharID(buf->character_id);
			if (client)
			{
				client->SetDzRemovalTimer(buf->removed);
			}
		}
		break;
	}
	case ServerOP_DzSwapMembers:
	{
		auto buf = reinterpret_cast<ServerDzMemberSwap_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->sender_zone_id, buf->sender_instance_id))
		{
			auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
			if (dz)
			{
				auto status = static_cast<DynamicZoneMemberStatus>(buf->add_character_status);
				dz->ProcessMemberAddRemove({ buf->remove_character_id, buf->remove_character_name }, true);
				dz->ProcessMemberAddRemove({ buf->add_character_id, buf->add_character_name, status }, false);
			}
		}

		if (zone && zone->IsZone(buf->dz_zone_id, buf->dz_instance_id))
		{
			// cache independent redundancy to kick removed members from dz's instance
			Client* removed_client = entity_list.GetClientByCharID(buf->remove_character_id);
			if (removed_client)
			{
				removed_client->SetDzRemovalTimer(true);
			}

			Client* added_client = entity_list.GetClientByCharID(buf->add_character_id);
			if (added_client)
			{
				added_client->SetDzRemovalTimer(false);
			}
		}
		break;
	}
	case ServerOP_DzRemoveAllMembers:
	{
		auto buf = reinterpret_cast<ServerDzID_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->sender_zone_id, buf->sender_instance_id))
		{
			auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
			if (dz)
			{
				dz->ProcessRemoveAllMembers();
			}
		}

		if (zone && zone->IsZone(buf->dz_zone_id, buf->dz_instance_id))
		{
			// cache independent redundancy to kick removed members from dz's instance
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
	case ServerOP_DzGetMemberStatuses:
	{
		// reply from world for online member statuses request for async zone member updates
		auto buf = reinterpret_cast<ServerDzMemberStatuses_Struct*>(pack->pBuffer);
		auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
		if (dz)
		{
			for (uint32_t i = 0; i < buf->count; ++i)
			{
				auto status = static_cast<DynamicZoneMemberStatus>(buf->entries[i].online_status);
				dz->SetInternalMemberStatus(buf->entries[i].character_id, status);
			}
			dz->m_has_member_statuses = true;
			dz->SendUpdatesToZoneMembers(false, true);
		}
		break;
	}
	case ServerOP_DzUpdateMemberStatus:
	{
		auto buf = reinterpret_cast<ServerDzMemberStatus_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->sender_zone_id, buf->sender_instance_id))
		{
			auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
			if (dz)
			{
				auto status = static_cast<DynamicZoneMemberStatus>(buf->status);
				dz->ProcessMemberStatusChange(buf->character_id, status);
			}
		}
		break;
	}
	case ServerOP_DzLeaderChanged:
	{
		auto buf = reinterpret_cast<ServerDzLeaderID_Struct*>(pack->pBuffer);
		auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
		if (dz)
		{
			dz->ProcessLeaderChanged(buf->leader_id);
		}
		break;
	}
	case ServerOP_DzExpireWarning:
	{
		auto buf = reinterpret_cast<ServerDzExpireWarning_Struct*>(pack->pBuffer);
		auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
		if (dz)
		{
			dz->SendMembersExpireWarning(buf->minutes_remaining);
		}
		break;
	}
	}
}

std::unique_ptr<EQApplicationPacket> DynamicZone::CreateExpireWarningPacket(uint32_t minutes_remaining)
{
	uint32_t outsize = sizeof(ExpeditionExpireWarning);
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzExpeditionEndsWarning, outsize);
	auto buf = reinterpret_cast<ExpeditionExpireWarning*>(outapp->pBuffer);
	buf->minutes_remaining = minutes_remaining;
	return outapp;
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

void DynamicZone::SendLeaderNameToZoneMembers()
{
	auto outapp_leader = CreateLeaderNamePacket();

	for (const auto& member : m_members)
	{
		Client* member_client = entity_list.GetClientByCharID(member.id);
		if (member_client)
		{
			member_client->QueuePacket(outapp_leader.get());

			if (member.id == m_leader.id && RuleB(Expedition, AlwaysNotifyNewLeaderOnChange))
			{
				member_client->MessageString(Chat::Yellow, DZMAKELEADER_YOU);
			}
		}
	}
}

void DynamicZone::SendMembersExpireWarning(uint32_t minutes_remaining)
{
	// expeditions warn members in all zones not just the dz
	auto outapp = CreateExpireWarningPacket(minutes_remaining);
	for (const auto& member : GetMembers())
	{
		Client* member_client = entity_list.GetClientByCharID(member.id);
		if (member_client)
		{
			member_client->QueuePacket(outapp.get());

			// live doesn't actually send the chat message with it
			member_client->MessageString(Chat::Yellow, EXPEDITION_MIN_REMAIN,
				fmt::format_int(minutes_remaining).c_str());
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

void DynamicZone::SendMemberListNameToZoneMembers(const std::string& char_name, bool remove)
{
	auto outapp_member_name = CreateMemberListNamePacket(char_name, remove);

	for (const auto& member : m_members)
	{
		Client* member_client = entity_list.GetClientByCharID(member.id);
		if (member_client)
		{
			member_client->QueuePacket(outapp_member_name.get());
		}
	}
}

void DynamicZone::SendMemberListStatusToZoneMembers(const DynamicZoneMember& update_member)
{
	auto outapp_member_status = CreateMemberListStatusPacket(update_member.name, update_member.status);

	for (const auto& member : m_members)
	{
		Client* member_client = entity_list.GetClientByCharID(member.id);
		if (member_client)
		{
			member_client->QueuePacket(outapp_member_status.get());
		}
	}
}

void DynamicZone::SendClientWindowUpdate(Client* client)
{
	if (client)
	{
		client->QueuePacket(CreateInfoPacket().get());
		client->QueuePacket(CreateMemberListPacket().get());
	}
}

void DynamicZone::SendUpdatesToZoneMembers(bool removing_all, bool silent)
{
	// performs a full update on all members (usually for dz creation or removing all)
	if (!HasMembers())
	{
		return;
	}

	std::unique_ptr<EQApplicationPacket> outapp_info = nullptr;
	std::unique_ptr<EQApplicationPacket> outapp_members = nullptr;

	// only expeditions use the dz window. on live the window is filled by non
	// expeditions when first created but never kept updated. that behavior could
	// be replicated in the future by flagging this as a creation update
	if (m_type == DynamicZoneType::Expedition)
	{
		// clearing info also clears member list, no need to send both when removing
		outapp_info = CreateInfoPacket(removing_all);
		outapp_members = removing_all ? nullptr : CreateMemberListPacket();
	}

	for (const auto& member : GetMembers())
	{
		Client* client = entity_list.GetClientByCharID(member.id);
		if (client)
		{
			if (removing_all) {
				client->RemoveDynamicZoneID(GetID());
			} else {
				client->AddDynamicZoneID(GetID());
			}

			client->SendDzCompassUpdate();

			if (outapp_info)
			{
				client->QueuePacket(outapp_info.get());
			}

			if (outapp_members)
			{
				client->QueuePacket(outapp_members.get());
			}

			// callback to the dz system so it can perform any messages or set client data
			if (m_on_client_addremove)
			{
				m_on_client_addremove(client, removing_all, silent);
			}
		}
	}
}

void DynamicZone::ProcessMemberAddRemove(const DynamicZoneMember& member, bool removed)
{
	DynamicZoneBase::ProcessMemberAddRemove(member, removed);

	// the affected client always gets a full compass update. for expeditions
	// client also gets window info update and all members get a member list update
	Client* client = entity_list.GetClientByCharID(member.id);
	if (client)
	{
		if (!removed) {
			client->AddDynamicZoneID(GetID());
		} else {
			client->RemoveDynamicZoneID(GetID());
		}

		client->SendDzCompassUpdate();

		if (m_type == DynamicZoneType::Expedition)
		{
			// sending clear info also clears member list for removed members
			client->QueuePacket(CreateInfoPacket(removed).get());
		}

		if (m_on_client_addremove)
		{
			m_on_client_addremove(client, removed, false);
		}
	}

	if (m_type == DynamicZoneType::Expedition)
	{
		// send full list when adding (MemberListName adds with "unknown" status)
		if (!removed) {
			SendMemberListToZoneMembers();
		} else {
			SendMemberListNameToZoneMembers(member.name, true);
		}
	}
}

void DynamicZone::ProcessRemoveAllMembers(bool silent)
{
	SendUpdatesToZoneMembers(true, silent);
	DynamicZoneBase::ProcessRemoveAllMembers(silent);
}

void DynamicZone::DoAsyncZoneMemberUpdates()
{
	// gets member statuses from world and performs zone member updates on reply
	// if we've already received member statuses we can just update immediately
	if (m_has_member_statuses)
	{
		SendUpdatesToZoneMembers();
		return;
	}

	constexpr uint32_t pack_size = sizeof(ServerDzID_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzGetMemberStatuses, pack_size);
	auto buf = reinterpret_cast<ServerDzID_Struct*>(pack->pBuffer);
	buf->dz_id = GetID();
	buf->sender_zone_id = zone ? zone->GetZoneID() : 0;
	buf->sender_instance_id = zone ? zone->GetInstanceID() : 0;
	worldserver.SendPacket(pack.get());
}

bool DynamicZone::ProcessMemberStatusChange(uint32_t member_id, DynamicZoneMemberStatus status)
{
	bool changed = DynamicZoneBase::ProcessMemberStatusChange(member_id, status);

	if (changed && m_type == DynamicZoneType::Expedition)
	{
		auto member = GetMemberData(member_id);
		if (member.IsValid())
		{
			SendMemberListStatusToZoneMembers(member);
		}
	}

	return changed;
}

void DynamicZone::ProcessLeaderChanged(uint32_t new_leader_id)
{
	auto new_leader = GetMemberData(new_leader_id);
	if (!new_leader.IsValid())
	{
		LogDynamicZones("Processed invalid new leader id [{}] for dz [{}]", new_leader_id, m_id);
		return;
	}

	LogDynamicZones("Replaced [{}] leader [{}] with [{}]", m_id, GetLeaderName(), new_leader.name);

	SetLeader(new_leader);
	if (GetType() == DynamicZoneType::Expedition)
	{
		SendLeaderNameToZoneMembers();
	}
}

bool DynamicZone::CanClientLootCorpse(Client* client, uint32_t npc_type_id, uint32_t entity_id)
{
	// non-members of a dz cannot loot corpses inside the dz
	if (!HasMember(client->CharacterID()))
	{
		return false;
	}

	// expeditions may prevent looting based on client's lockouts
	if (GetType() == DynamicZoneType::Expedition)
	{
		auto expedition = Expedition::FindCachedExpeditionByZoneInstance(zone->GetZoneID(), zone->GetInstanceID());
		if (expedition && !expedition->CanClientLootCorpse(client, npc_type_id, entity_id))
		{
			return false;
		}
	}

	return true;
}
