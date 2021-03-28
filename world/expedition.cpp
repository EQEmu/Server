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

Expedition::Expedition(uint32_t expedition_id, const DynamicZone& dz, uint32_t leader_id
) :
	m_expedition_id(expedition_id),
	m_dynamic_zone(dz),
	m_leader_id(leader_id),
	m_choose_leader_cooldown_timer{ static_cast<uint32_t>(RuleI(Expedition, ChooseLeaderCooldownTime)) }
{
	m_warning_cooldown_timer.Enable();
}

void Expedition::AddMember(uint32_t character_id)
{
	if (!HasMember(character_id))
	{
		m_member_ids.emplace_back(character_id);
	}
}

bool Expedition::HasMember(uint32_t character_id)
{
	return std::any_of(m_member_ids.begin(), m_member_ids.end(),
		[&](uint32_t member_id) { return member_id == character_id; });
}

void Expedition::RemoveMember(uint32_t character_id)
{
	m_member_ids.erase(std::remove_if(m_member_ids.begin(), m_member_ids.end(),
		[&](uint32_t member_id) { return member_id == character_id; }
	), m_member_ids.end());

	if (character_id == m_leader_id)
	{
		ChooseNewLeader();
	}
}

void Expedition::ChooseNewLeader()
{
	if (m_member_ids.empty() || !m_choose_leader_cooldown_timer.Check())
	{
		m_choose_leader_needed = true;
		return;
	}

	// we don't track expedition member status in world so may choose a linkdead member
	// this is fine since it will trigger another change when that member goes offline
	auto it = std::find_if(m_member_ids.begin(), m_member_ids.end(), [&](uint32_t member_id) {
		auto member_cle = (member_id != m_leader_id) ? client_list.FindCLEByCharacterID(member_id) : nullptr;
		return (member_id != m_leader_id && member_cle && member_cle->GetOnline() == CLE_Status::InZone);
	});

	if (it == m_member_ids.end())
	{
		// no online members found, fallback to choosing any member
		it = std::find_if(m_member_ids.begin(), m_member_ids.end(),
			[&](uint32_t member_id) { return (member_id != m_leader_id); });
	}

	if (it != m_member_ids.end() && SetNewLeader(*it))
	{
		m_choose_leader_needed = false;
	}
}

bool Expedition::SetNewLeader(uint32_t character_id)
{
	if (!HasMember(character_id))
	{
		return false;
	}

	LogExpeditionsModerate("Replacing [{}] leader [{}] with [{}]", m_expedition_id, m_leader_id, character_id);
	ExpeditionDatabase::UpdateLeaderID(m_expedition_id, character_id);
	m_leader_id = character_id;
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
	buf->leader_id = m_leader_id;
	zoneserver_list.SendPacket(pack.get());
}

void Expedition::CheckExpireWarning()
{
	if (m_warning_cooldown_timer.Check(false))
	{
		using namespace std::chrono_literals;
		auto remaining = GetDynamicZone().GetRemainingDuration();
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
