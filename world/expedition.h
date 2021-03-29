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

#ifndef WORLD_EXPEDITION_H
#define WORLD_EXPEDITION_H

#include "dynamic_zone.h"
#include "../common/timer.h"
#include <chrono>
#include <cstdint>
#include <vector>

class Expedition
{
public:
	Expedition() = default;
	Expedition(uint32_t expedition_id, const DynamicZone& dz, uint32_t leader_id);

	void AddMember(uint32_t character_id);
	void RemoveMember(uint32_t character_id);
	void RemoveAllMembers() { m_member_ids.clear(); }
	void CheckExpireWarning();
	void CheckLeader();
	void ChooseNewLeader();
	DynamicZone& GetDynamicZone() { return m_dynamic_zone; }
	uint32_t GetID() const { return m_expedition_id; }
	bool HasMember(uint32_t character_id);
	bool IsEmpty() const { return m_member_ids.empty(); }
	bool IsValid() const { return m_expedition_id != 0; }
	bool Process();
	void SendZonesExpeditionDeleted();
	void SendZonesExpireWarning(uint32_t minutes_remaining);
	bool SetNewLeader(uint32_t new_leader_id);

private:
	void SendZonesLeaderChanged();

	uint32_t m_expedition_id  = 0;
	uint32_t m_leader_id      = 0;
	bool m_choose_leader_needed = false;
	Timer m_choose_leader_cooldown_timer;
	Timer m_warning_cooldown_timer;
	DynamicZone m_dynamic_zone;
	std::vector<uint32_t> m_member_ids;
};

#endif
