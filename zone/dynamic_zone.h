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

#ifndef DYNAMIC_ZONE_H
#define DYNAMIC_ZONE_H

#include "../common/dynamic_zone_base.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class Client;
class Database;
class EQApplicationPacket;
class ServerPacket;

extern const char* const CREATE_NOT_ALL_ADDED;

class DynamicZone : public DynamicZoneBase
{
public:
	using DynamicZoneBase::DynamicZoneBase; // inherit base constructors

	DynamicZone() = default;
	DynamicZone(uint32_t zone_id, uint32_t version, uint32_t duration, DynamicZoneType type);

	static void CacheAllFromDatabase();
	static void CacheNewDynamicZone(ServerPacket* pack);
	static DynamicZone* CreateNew(DynamicZone& dz_details, const std::vector<DynamicZoneMember>& members);
	static DynamicZone* FindDynamicZoneByID(uint32_t dz_id);
	static void HandleWorldMessage(ServerPacket* pack);

	void SetSecondsRemaining(uint32_t seconds_remaining) override;

	void DoAsyncZoneMemberUpdates();
	bool CanClientLootCorpse(Client* client, uint32_t npc_type_id, uint32_t entity_id);
	bool IsCurrentZoneDzInstance() const;
	void RegisterOnClientAddRemove(std::function<void(Client* client, bool removed, bool silent)> on_client_addremove);
	void SendClientWindowUpdate(Client* client);
	void SendLeaderNameToZoneMembers();
	void SendMemberListToZoneMembers();
	void SendMemberListNameToZoneMembers(const std::string& char_name, bool remove);
	void SendMemberListStatusToZoneMembers(const DynamicZoneMember& member);
	void SendRemoveAllMembersToZoneMembers(bool silent) { ProcessRemoveAllMembers(silent); }

	std::unique_ptr<EQApplicationPacket> CreateExpireWarningPacket(uint32_t minutes_remaining);
	std::unique_ptr<EQApplicationPacket> CreateInfoPacket(bool clear = false);
	std::unique_ptr<EQApplicationPacket> CreateLeaderNamePacket();
	std::unique_ptr<EQApplicationPacket> CreateMemberListPacket(bool clear = false);
	std::unique_ptr<EQApplicationPacket> CreateMemberListNamePacket(const std::string& name, bool remove_name);
	std::unique_ptr<EQApplicationPacket> CreateMemberListStatusPacket(const std::string& name, DynamicZoneMemberStatus status);

protected:
	uint16_t GetCurrentInstanceID() override;
	uint16_t GetCurrentZoneID() override;
	Database& GetDatabase() override;
	void ProcessCompassChange(const DynamicZoneLocation& location) override;
	void ProcessMemberAddRemove(const DynamicZoneMember& member, bool removed) override;
	bool ProcessMemberStatusChange(uint32_t member_id, DynamicZoneMemberStatus status) override;
	void ProcessRemoveAllMembers(bool silent = false) override;
	bool SendServerPacket(ServerPacket* packet) override;

private:
	static void StartAllClientRemovalTimers();
	void ProcessLeaderChanged(uint32_t new_leader_id);
	void SendCompassUpdateToZoneMembers();
	void SendMembersExpireWarning(uint32_t minutes);
	void SendUpdatesToZoneMembers(bool removing_all = false, bool silent = true);
	void SetUpdatedDuration(uint32_t seconds);

	std::function<void(Client*, bool, bool)> m_on_client_addremove;
};

#endif
