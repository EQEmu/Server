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

#include "../common/rulesys.h"
#include "../common/timer.h"
#include <chrono>
#include <unordered_set>
#include <vector>

extern class ExpeditionCache expedition_cache;

class Expedition;
class ServerPacket;

namespace ExpeditionDatabase
{
	void PurgeExpiredExpeditions();
	void PurgeExpiredCharacterLockouts();
	std::vector<Expedition> LoadExpeditions();
	Expedition LoadExpedition(uint32_t expedition_id);
	void DeleteExpeditions(const std::vector<uint32_t>& expedition_ids);
};

namespace ExpeditionMessage
{
	void HandleZoneMessage(ServerPacket* pack);
	void AddPlayer(ServerPacket* pack);
	void MakeLeader(ServerPacket* pack);
	void GetOnlineMembers(ServerPacket* pack);
	void SaveInvite(ServerPacket* pack);
	void RequestInvite(ServerPacket* pack);
};

class ExpeditionCache
{
public:
	void AddExpedition(uint32_t expedition_id);
	void RemoveExpedition(uint32_t expedition_id);
	void LoadActiveExpeditions();
	void MemberChange(uint32_t expedition_id, uint32_t character_id, bool remove);
	void RemoveAllMembers(uint32_t expedition_id);
	void Process();

private:
	std::vector<Expedition> m_expeditions;
	Timer m_process_throttle_timer{static_cast<uint32_t>(RuleI(Expedition, WorldExpeditionProcessRateMS))};
};

class Expedition
{
public:
	Expedition() = default;
	Expedition(
		uint32_t expedition_id, uint32_t instance_id, uint32_t dz_zone_id,
		uint32_t expire_time, uint32_t duration);

	void AddMember(uint32_t character_id) { m_member_ids.emplace(character_id); }
	void RemoveMember(uint32_t character_id) { m_member_ids.erase(character_id); }
	void RemoveAllMembers() { m_member_ids.clear(); }
	uint32_t GetID() const { return m_expedition_id; }
	uint16_t GetInstanceID() const { return static_cast<uint16_t>(m_dz_instance_id); }
	uint16_t GetZoneID() const { return static_cast<uint16_t>(m_dz_zone_id); }
	bool IsEmpty() const { return m_member_ids.empty(); }
	bool IsExpired() const { return m_expire_time < std::chrono::system_clock::now(); }
	void SendZonesExpeditionDeleted();

private:
	uint32_t m_expedition_id  = 0;
	uint32_t m_dz_instance_id = 0;
	uint32_t m_dz_zone_id     = 0;
	uint32_t m_start_time     = 0;
	uint32_t m_duration       = 0;
	std::unordered_set<uint32_t> m_member_ids;
	std::chrono::time_point<std::chrono::system_clock> m_expire_time;
};

#endif
