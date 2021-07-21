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

#ifndef EXPEDITION_H
#define EXPEDITION_H

#include "dynamic_zone.h"
#include "../common/expedition_lockout_timer.h"
#include "../common/repositories/expeditions_repository.h"
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Client;
class EQApplicationPacket;
struct ExpeditionInvite;
class ExpeditionRequest;
class ServerPacket;

extern const char* const DZ_YOU_NOT_ASSIGNED;
extern const char* const EXPEDITION_OTHER_BELONGS;

enum class ExpeditionLockMessage : uint8_t
{
	None = 0,
	Close,
	Begin
};

class Expedition
{
public:
	Expedition() = delete;
	Expedition(DynamicZone* dz) : m_dynamic_zone(dz) { assert(m_dynamic_zone != nullptr); }
	Expedition(DynamicZone* dz, uint32_t id, uint32_t dz_id);

	static Expedition* TryCreate(Client* requester, DynamicZone& dynamiczone, bool disable_messages);

	static void CacheFromDatabase(uint32_t expedition_id);
	static bool CacheAllFromDatabase();
	static Expedition* FindCachedExpeditionByCharacterID(uint32_t character_id);
	static Expedition* FindCachedExpeditionByCharacterName(const std::string& char_name);
	static Expedition* FindCachedExpeditionByDynamicZoneID(uint32_t dz_id);
	static Expedition* FindCachedExpeditionByID(uint32_t expedition_id);
	static Expedition* FindCachedExpeditionByZoneInstance(uint32_t zone_id, uint32_t instance_id);
	static std::vector<ExpeditionLockoutTimer> GetExpeditionLockoutsByCharacterID(uint32_t character_id);
	static void HandleWorldMessage(ServerPacket* pack);
	static void AddLockoutByCharacterID(uint32_t character_id, const std::string& expedition_name,
		const std::string& event_name, uint32_t seconds, const std::string& uuid = {});
	static void AddLockoutByCharacterName(const std::string& character_name, const std::string& expedition_name,
		const std::string& event_name, uint32_t seconds, const std::string& uuid = {});
	static bool HasLockoutByCharacterID(uint32_t character_id,
		const std::string& expedition_name, const std::string& event_name);
	static bool HasLockoutByCharacterName(const std::string& character_name,
		const std::string& expedition_name, const std::string& event_name);
	static void RemoveLockoutsByCharacterID(uint32_t character_id,
		const std::string& expedition_name = {}, const std::string& event_name = {});
	static void RemoveLockoutsByCharacterName(const std::string& character_name,
		const std::string& expedition_name = {}, const std::string& event_name = {});
	static void AddLockoutClients(const ExpeditionLockoutTimer& lockout, uint32_t exclude_id = 0);

	uint32_t GetID() const { return m_id; }
	uint32_t GetDynamicZoneID() const { return m_dynamic_zone_id; }
	DynamicZone* GetDynamicZone() const { return m_dynamic_zone; }
	const DynamicZoneMember& GetLeader() { return GetDynamicZone()->GetLeader(); }
	uint32_t GetLeaderID() { return GetDynamicZone()->GetLeaderID(); }
	const std::string& GetLeaderName() { return GetDynamicZone()->GetLeaderName(); }
	const std::unordered_map<std::string, ExpeditionLockoutTimer>& GetLockouts() const { return m_lockouts; }
	const std::string& GetName() { return GetDynamicZone()->GetName(); }
	void RegisterDynamicZoneCallbacks();

	bool IsLocked() const { return m_is_locked; }
	void SetLocked(bool lock_expedition, ExpeditionLockMessage lock_msg,
		bool update_db = false, uint32_t msg_color = Chat::Yellow);

	void AddLockout(const std::string& event_name, uint32_t seconds);
	void AddLockoutDuration(const std::string& event_name, int seconds, bool members_only = true);
	void AddReplayLockout(uint32_t seconds);
	void AddReplayLockoutDuration(int seconds, bool members_only = true);
	bool HasLockout(const std::string& event_name);
	bool HasReplayLockout();
	void RemoveLockout(const std::string& event_name);
	void SetReplayLockoutOnMemberJoin(bool add_on_join, bool update_db = false);
	void SyncCharacterLockouts(uint32_t character_id, std::vector<ExpeditionLockoutTimer>& client_lockouts);
	void UpdateLockoutDuration(const std::string& event_name, uint32_t seconds, bool members_only = true);

	bool CanClientLootCorpse(Client* client, uint32_t npc_type_id, uint32_t spawn_id);
	std::string GetLootEventByNPCTypeID(uint32_t npc_id);
	std::string GetLootEventBySpawnID(uint32_t spawn_id);
	void SetLootEventByNPCTypeID(uint32_t npc_type_id, const std::string& event_name);
	void SetLootEventBySpawnID(uint32_t spawn_id, const std::string& event_name);

	void SendWorldMakeLeaderRequest(uint32_t requester_id, const std::string& new_leader_name);
	void SendWorldPendingInvite(const ExpeditionInvite& invite, const std::string& add_name);

	void DzAddPlayer(Client* requester, const std::string& add_char_name, const std::string& swap_remove_name = {});
	void DzAddPlayerContinue(std::string leader_name, std::string add_char_name, std::string swap_remove_name = {});
	void DzInviteResponse(Client* add_client, bool accepted, const std::string& swap_remove_name);
	void DzMakeLeader(Client* requester, std::string new_leader_name);
	void DzPlayerList(Client* requester);
	void DzRemovePlayer(Client* requester, std::string remove_char_name);
	void DzSwapPlayer(Client* requester, std::string remove_char_name, std::string add_char_name);
	void DzQuit(Client* requester);
	void DzKickPlayers(Client* requester);

	static const int32_t REPLAY_TIMER_ID;
	static const int32_t EVENT_TIMER_ID;

private:
	static void CacheExpeditions(std::vector<ExpeditionsRepository::Expeditions>&& expeditions);
	static void SendWorldCharacterLockout(uint32_t character_id, const ExpeditionLockoutTimer& lockout, bool remove);

	void AddLockout(const ExpeditionLockoutTimer& lockout, bool members_only = false);
	void AddLockoutDurationClients(const ExpeditionLockoutTimer& lockout, int seconds, uint32_t exclude_id = 0);
	bool ConfirmLeaderCommand(Client* requester);
	void OnClientAddRemove(Client* client, bool removed, bool silent);
	void LoadRepositoryResult(const ExpeditionsRepository::Expeditions& entry);
	bool ProcessAddConflicts(Client* leader_client, Client* add_client, bool swapping);
	void ProcessLockoutDuration(const ExpeditionLockoutTimer& lockout, int seconds, bool members_only = false);
	void ProcessLockoutUpdate(const ExpeditionLockoutTimer& lockout, bool remove, bool members_only = false);
	void SaveLockouts(ExpeditionRequest& request);
	void SendClientExpeditionInvite(
		Client* client, const std::string& inviter_name, const std::string& swap_remove_name);
	void SendLeaderMessage(Client* leader_client, uint16_t chat_type, uint32_t string_id,
		const std::initializer_list<std::string>& args = {});
	void SendWorldExpeditionUpdate(uint16_t server_opcode);
	void SendWorldAddPlayerInvite(const std::string& inviter_name, const std::string& swap_remove_name,
		const std::string& add_name, bool pending = false);
	void SendWorldLockoutDuration(
		const ExpeditionLockoutTimer& lockout, int seconds, bool members_only = false);
	void SendWorldLockoutUpdate(
		const ExpeditionLockoutTimer& lockout, bool remove, bool members_only = false);
	void SendWorldSettingChanged(uint16_t server_opcode, bool setting_value);
	void SetDynamicZone(DynamicZone&& dz);
	void TryAddClient(Client* add_client, const std::string& inviter_name,
		const std::string& swap_remove_name, Client* leader_client = nullptr);

	std::unique_ptr<EQApplicationPacket> CreateInvitePacket(const std::string& inviter_name, const std::string& swap_remove_name);

	uint32_t m_id = 0;
	uint32_t m_dynamic_zone_id = 0;
	bool m_is_locked = false;
	bool m_add_replay_on_join = true;
	DynamicZone* m_dynamic_zone = nullptr; // should never be null, will exist for lifetime of expedition
	std::unordered_map<std::string, ExpeditionLockoutTimer> m_lockouts;
	std::unordered_map<uint32_t, std::string> m_npc_loot_events;   // only valid inside dz zone
	std::unordered_map<uint32_t, std::string> m_spawn_loot_events; // only valid inside dz zone
};

#endif
