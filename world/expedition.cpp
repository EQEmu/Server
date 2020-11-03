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

extern ClientList client_list;
extern ZSList zoneserver_list;

Expedition::Expedition(uint32_t expedition_id, uint32_t dz_id, uint32_t dz_instance_id,
	uint32_t dz_zone_id, uint32_t start_time, uint32_t duration, uint32_t leader_id
) :
	m_expedition_id(expedition_id),
	m_dz_id(dz_id),
	m_dz_instance_id(dz_instance_id),
	m_dz_zone_id(dz_zone_id),
	m_start_time(std::chrono::system_clock::from_time_t(start_time)),
	m_duration(duration),
	m_leader_id(leader_id)
{
	m_expire_time = m_start_time + m_duration;
	m_warning_cooldown_timer.Enable();
}

void Expedition::AddMember(uint32_t character_id)
{
	auto it = std::find_if(m_member_ids.begin(), m_member_ids.end(),
		[&](uint32_t member_id) { return member_id == character_id; });

	if (it == m_member_ids.end())
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

	if (!m_member_ids.empty() && character_id == m_leader_id)
	{
		ChooseNewLeader();
	}
}

void Expedition::ChooseNewLeader()
{
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

	if (it != m_member_ids.end())
	{
		SetNewLeader(*it);
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
	auto pack = std::unique_ptr<ServerPacket>(new ServerPacket(ServerOP_ExpeditionDeleted, pack_size));
	auto buf = reinterpret_cast<ServerExpeditionID_Struct*>(pack->pBuffer);
	buf->expedition_id = GetID();
	zoneserver_list.SendPacket(pack.get());
}

void Expedition::SendZonesDurationUpdate()
{
	uint32_t packsize = sizeof(ServerExpeditionUpdateDuration_Struct);
	auto pack = std::unique_ptr<ServerPacket>(new ServerPacket(ServerOP_ExpeditionDzDuration, packsize));
	auto packbuf = reinterpret_cast<ServerExpeditionUpdateDuration_Struct*>(pack->pBuffer);
	packbuf->expedition_id = GetID();
	packbuf->new_duration_seconds = static_cast<uint32_t>(m_duration.count());
	zoneserver_list.SendPacket(pack.get());
}

void Expedition::SendZonesExpireWarning(uint32_t minutes_remaining)
{
	uint32_t pack_size = sizeof(ServerExpeditionExpireWarning_Struct);
	auto pack = std::unique_ptr<ServerPacket>(new ServerPacket(ServerOP_ExpeditionExpireWarning, pack_size));
	auto buf = reinterpret_cast<ServerExpeditionExpireWarning_Struct*>(pack->pBuffer);
	buf->expedition_id = GetID();
	buf->minutes_remaining = minutes_remaining;
	zoneserver_list.SendPacket(pack.get());
}

void Expedition::SendZonesLeaderChanged()
{
	uint32_t pack_size = sizeof(ServerExpeditionLeaderID_Struct);
	auto pack = std::unique_ptr<ServerPacket>(new ServerPacket(ServerOP_ExpeditionLeaderChanged, pack_size));
	auto buf = reinterpret_cast<ServerExpeditionLeaderID_Struct*>(pack->pBuffer);
	buf->expedition_id = GetID();
	buf->leader_id = m_leader_id;
	zoneserver_list.SendPacket(pack.get());
}

void Expedition::UpdateDzSecondsRemaining(uint32_t seconds_remaining)
{
	auto now = std::chrono::system_clock::now();
	auto update_time = std::chrono::seconds(seconds_remaining);

	auto current_remaining = m_expire_time - now;
	if (current_remaining > update_time) // reduce only
	{
		LogExpeditionsDetail(
			"Updating expedition [{}] dz instance [{}] seconds remaining to [{}]s",
			GetID(), GetInstanceID(), seconds_remaining
		);

		// preserve original start time and adjust duration instead
		m_expire_time = now + update_time;
		m_duration = std::chrono::duration_cast<std::chrono::seconds>(m_expire_time - m_start_time);

		ExpeditionDatabase::UpdateDzDuration(GetInstanceID(), static_cast<uint32_t>(m_duration.count()));

		// update zone level caches and update the actual dz instance's timer
		SendZonesDurationUpdate();
	}
}

std::chrono::system_clock::duration Expedition::GetRemainingDuration() const
{
	return m_expire_time - std::chrono::system_clock::now();
}

void Expedition::CheckExpireWarning()
{
	if (m_warning_cooldown_timer.Check(false))
	{
		using namespace std::chrono_literals;
		auto remaining = GetRemainingDuration();
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
