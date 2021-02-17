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
	m_dynamic_zone = std::move(dz);
	m_dynamic_zone.RegisterOnMemberAddRemove(
		[this](const DynamicZoneMember& member, bool removed) { OnMemberAddRemove(member, removed); });
	m_dynamic_zone.RegisterOnStatusChanged(
		[this](const DynamicZoneMember& member) { OnMemberStatusChanged(member); });
}

void Expedition::OnMemberAddRemove(const DynamicZoneMember& member, bool removed)
{
	if (removed && member.id == GetLeaderID())
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
		if (member.id != GetLeaderID() && member.IsOnline()) {
			auto member_cle = client_list.FindCLEByCharacterID(member.id);
			return (member_cle && member_cle->GetOnline() == CLE_Status::InZone);
		}
		return false;
	});

	if (it == members.end())
	{
		// no online members found, fallback to choosing any member
		it = std::find_if(members.begin(), members.end(),
			[&](const DynamicZoneMember& member) { return member.id != GetLeaderID(); });
	}

	if (it != members.end() && SetNewLeader(*it))
	{
		m_choose_leader_needed = false;
	}
}

bool Expedition::SetNewLeader(const DynamicZoneMember& member)
{
	auto new_leader = GetDynamicZone().GetMemberData(member.id);
	if (!new_leader.IsValid())
	{
		return false;
	}

	LogExpeditionsModerate("Replacing [{}] leader [{}] with [{}]", m_id, GetLeaderName(), new_leader.name);
	m_dynamic_zone.SetLeader(new_leader, true);
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
	buf->leader_id = GetLeaderID();
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

void Expedition::OnMemberStatusChanged(const DynamicZoneMember& member)
{
	// any member status update will trigger a leader fix if leader was offline
	if (GetLeader().status == DynamicZoneMemberStatus::Offline && GetDynamicZone().GetMemberCount() > 1)
	{
		ChooseNewLeader();
	}
}
