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
#include "expedition_database.h"
#include "cliententry.h"
#include "clientlist.h"
#include "zonelist.h"
#include "zoneserver.h"
#include "../common/eqemu_logsys.h"
#include "../common/rulesys.h"

extern ClientList client_list;
extern ZSList zoneserver_list;

Expedition::Expedition() :
	m_choose_leader_cooldown_timer{ static_cast<uint32_t>(RuleI(Expedition, ChooseLeaderCooldownTime)) }
{
	m_warning_cooldown_timer.Enable();
}

void Expedition::SetDynamicZone(DynamicZone&& dz)
{
	dz.SetName(GetName());
	dz.SetLeader(GetLeader());

	m_dynamic_zone = std::move(dz);
}

void Expedition::RemoveMember(uint32_t character_id)
{
	GetDynamicZone().RemoveInternalMember(character_id);

	if (character_id == m_leader.id)
	{
		ChooseNewLeader();
	}
}

void Expedition::ChooseNewLeader()
{
	const auto& members = GetDynamicZone().GetMembers();
	if (members.empty() || !m_choose_leader_cooldown_timer.Check())
	{
		m_choose_leader_needed = true;
		return;
	}

	auto it = std::find_if(members.begin(), members.end(), [&](const DynamicZoneMember& member) {
		if (member.id != m_leader.id && member.IsOnline()) {
			auto member_cle = client_list.FindCLEByCharacterID(member.id);
			return (member_cle && member_cle->GetOnline() == CLE_Status::InZone);
		}
		return false;
	});

	if (it == members.end())
	{
		// no online members found, fallback to choosing any member
		it = std::find_if(members.begin(), members.end(),
			[&](const DynamicZoneMember& member) { return (member.id != m_leader.id); });
	}

	if (it != members.end() && SetNewLeader(*it))
	{
		m_choose_leader_needed = false;
	}
}

bool Expedition::SetNewLeader(const DynamicZoneMember& member)
{
	if (!GetDynamicZone().HasMember(member.id))
	{
		return false;
	}

	LogExpeditionsModerate("Replacing [{}] leader [{}] with [{}]", m_id, m_leader.name, member.name);
	ExpeditionDatabase::UpdateLeaderID(m_id, member.id);
	m_leader = member;
	m_dynamic_zone.SetLeader(m_leader);
	SendZonesLeaderChanged();
	return true;
}

void Expedition::SendZonesExpeditionDeleted()
{
	uint32_t pack_size = sizeof(ServerExpeditionID_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_ExpeditionDeleted, pack_size);
	auto buf = reinterpret_cast<ServerExpeditionID_Struct*>(pack->pBuffer);
	buf->expedition_id = GetID();
	zoneserver_list.SendPacket(pack.get());
}

void Expedition::SendZonesExpireWarning(uint32_t minutes_remaining)
{
	uint32_t pack_size = sizeof(ServerExpeditionExpireWarning_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_ExpeditionExpireWarning, pack_size);
	auto buf = reinterpret_cast<ServerExpeditionExpireWarning_Struct*>(pack->pBuffer);
	buf->expedition_id = GetID();
	buf->minutes_remaining = minutes_remaining;
	zoneserver_list.SendPacket(pack.get());
}

void Expedition::SendZonesLeaderChanged()
{
	uint32_t pack_size = sizeof(ServerExpeditionLeaderID_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_ExpeditionLeaderChanged, pack_size);
	auto buf = reinterpret_cast<ServerExpeditionLeaderID_Struct*>(pack->pBuffer);
	buf->expedition_id = GetID();
	buf->leader_id = m_leader.id;
	zoneserver_list.SendPacket(pack.get());
}

void Expedition::CheckExpireWarning()
{
	if (m_warning_cooldown_timer.Check(false))
	{
		using namespace std::chrono_literals;
		auto remaining = GetDynamicZone().GetDurationRemaining();
		if ((remaining > 14min && remaining < 15min) ||
		    (remaining > 4min && remaining < 5min) ||
		    (remaining > 0min && remaining < 1min))
		{
			int minutes = std::chrono::duration_cast<std::chrono::minutes>(remaining).count() + 1;
			SendZonesExpireWarning(minutes);
			m_warning_cooldown_timer.Start(70000); // 1 minute 10 seconds
		}
	}
}

void Expedition::CheckLeader()
{
	if (m_choose_leader_needed)
	{
		ChooseNewLeader();
	}
}

bool Expedition::Process()
{
	// returns true if expedition needs to be deleted from world cache and db
	// expedition is not deleted until its dz has no clients to prevent exploits
	auto status = m_dynamic_zone.Process();
	if (status == DynamicZoneStatus::ExpiredEmpty)
	{
		LogExpeditions("Expedition [{}] expired or empty, notifying zones and deleting", GetID());
		SendZonesExpeditionDeleted();
		return true;
	}

	CheckExpireWarning();
	CheckLeader();

	return false;
}

void Expedition::UpdateMemberStatus(uint32_t character_id, DynamicZoneMemberStatus status)
{
	GetDynamicZone().SetInternalMemberStatus(character_id, status);

	// temporary until move to using dz leader object completely
	if (character_id == m_leader.id)
	{
		m_leader.status = GetDynamicZone().GetLeader().status;
	}

	// any member status update will trigger a leader fix if leader was offline
	if (m_leader.status == DynamicZoneMemberStatus::Offline)
	{
		ChooseNewLeader();
	}
}

void Expedition::SendZoneMemberStatuses(uint16_t zone_id, uint16_t instance_id)
{
	const auto& members = GetDynamicZone().GetMembers();

	uint32_t members_count = static_cast<uint32_t>(members.size());
	uint32_t entries_size = sizeof(ServerExpeditionMemberStatusEntry_Struct) * members_count;
	uint32_t pack_size = sizeof(ServerExpeditionMemberStatuses_Struct) + entries_size;
	auto pack = std::make_unique<ServerPacket>(ServerOP_ExpeditionGetMemberStatuses, pack_size);
	auto buf = reinterpret_cast<ServerExpeditionMemberStatuses_Struct*>(pack->pBuffer);
	buf->expedition_id = GetID();
	buf->count = members_count;

	for (int i = 0; i < members.size(); ++i)
	{
		buf->entries[i].character_id = members[i].id;
		buf->entries[i].online_status = static_cast<uint8_t>(members[i].status);
	}

	zoneserver_list.SendPacket(zone_id, instance_id, pack.get());
}

void Expedition::CacheMemberStatuses()
{
	// called when a new expedition is cached to fill member statuses
	std::string zone_name{};
	std::vector<ClientListEntry*> all_clients;
	all_clients.reserve(client_list.GetClientCount());
	client_list.GetClients(zone_name.c_str(), all_clients);

	for (const auto& member : GetDynamicZone().GetMembers())
	{
		auto it = std::find_if(all_clients.begin(), all_clients.end(),
			[&](const ClientListEntry* cle) { return (cle && cle->CharID() == member.id); });

		auto status = DynamicZoneMemberStatus::Offline;
		if (it != all_clients.end())
		{
			status = DynamicZoneMemberStatus::Online;
			if (GetDynamicZone().IsSameDz((*it)->zone(), (*it)->instance()))
			{
				status = DynamicZoneMemberStatus::InDynamicZone;
			}
		}

		GetDynamicZone().SetInternalMemberStatus(member.id, status);
	}
}
