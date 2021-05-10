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
#include "../common/expedition_base.h"
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

class Expedition : public ExpeditionBase
{
public:
	Expedition() = default;
	Expedition(uint32_t id, const std::string& uuid, DynamicZone&& dz, const std::string& expedition_name,
		const DynamicZoneMember& leader, uint32_t min_players, uint32_t max_players);

	static Expedition* TryCreate(Client* requester, DynamicZone& dynamiczone, ExpeditionRequest& request);

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

	DynamicZone& GetDynamicZone() { return m_dynamiczone; }
	const std::unordered_map<std::string, ExpeditionLockoutTimer>& GetLockouts() const { return m_lockouts; }

	bool AddMember(const std::string& add_char_name, uint32_t add_char_id);
	void RemoveAllMembers(bool enable_removal_timers = true);
	bool RemoveMember(const std::string& remove_char_name);
	void SetMemberStatus(Client* client, DynamicZoneMemberStatus status);
	void SwapMember(Client* add_client, const std::string& remove_char_name);

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

	void SendClientExpeditionInfo(Client* client);
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
	static void CacheExpeditions(std::vector<ExpeditionsRepository::ExpeditionWithLeader>&& expeditions);
	static void SendWorldCharacterLockout(uint32_t character_id, const ExpeditionLockoutTimer& lockout, bool remove);

	void AddLockout(const ExpeditionLockoutTimer& lockout, bool members_only = false);
	void AddLockoutDurationClients(const ExpeditionLockoutTimer& lockout, int seconds, uint32_t exclude_id = 0);
	bool ConfirmLeaderCommand(Client* requester);
	bool ProcessAddConflicts(Client* leader_client, Client* add_client, bool swapping);
	void ProcessLeaderChanged(uint32_t new_leader_id);
	void ProcessLockoutDuration(const ExpeditionLockoutTimer& lockout, int seconds, bool members_only = false);
	void ProcessLockoutUpdate(const ExpeditionLockoutTimer& lockout, bool remove, bool members_only = false);
	void ProcessMakeLeader(Client* old_leader, Client* new_leader,
		const std::string& new_leader_name, bool is_success, bool is_online);
	void ProcessMemberAdded(const std::string& added_char_name, uint32_t added_char_id);
	void ProcessMemberRemoved(const std::string& removed_char_name, uint32_t removed_char_id);
	void SaveLockouts(ExpeditionRequest& request);
	void SaveMembers(ExpeditionRequest& request);
	void SendClientExpeditionInvite(
		Client* client, const std::string& inviter_name, const std::string& swap_remove_name);
	void SendLeaderMessage(Client* leader_client, uint16_t chat_type, uint32_t string_id,
		const std::initializer_list<std::string>& args = {});
	void SendMemberListToZoneMembers();
	void SendMemberStatusToZoneMembers(uint32_t update_character_id, DynamicZoneMemberStatus status);
	void SendMembersExpireWarning(uint32_t minutes);
	void SendUpdatesToZoneMembers(bool clear = false, bool message_on_clear = true);
	void SendCompassUpdateToZoneMembers();
	void SendWorldExpeditionUpdate(uint16_t server_opcode);
	void SendWorldAddPlayerInvite(const std::string& inviter_name, const std::string& swap_remove_name,
		const std::string& add_name, bool pending = false);
	void SendWorldLockoutDuration(
		const ExpeditionLockoutTimer& lockout, int seconds, bool members_only = false);
	void SendWorldLockoutUpdate(
		const ExpeditionLockoutTimer& lockout, bool remove, bool members_only = false);
	void SendWorldMemberChanged(const std::string& char_name, uint32_t char_id, bool remove);
	void SendWorldMemberStatus(uint32_t character_id, DynamicZoneMemberStatus status);
	void SendWorldMemberSwapped(const std::string& remove_char_name, uint32_t remove_char_id,
		const std::string& add_char_name, uint32_t add_char_id);
	void SendWorldSettingChanged(uint16_t server_opcode, bool setting_value);
	void SetDynamicZone(DynamicZone&& dz);
	void TryAddClient(Client* add_client, const std::string& inviter_name,
		const std::string& swap_remove_name, Client* leader_client = nullptr);

	std::unique_ptr<EQApplicationPacket> CreateExpireWarningPacket(uint32_t minutes_remaining);
	std::unique_ptr<EQApplicationPacket> CreateInfoPacket(bool clear = false);
	std::unique_ptr<EQApplicationPacket> CreateInvitePacket(const std::string& inviter_name, const std::string& swap_remove_name);
	std::unique_ptr<EQApplicationPacket> CreateMemberListPacket(bool clear = false);
	std::unique_ptr<EQApplicationPacket> CreateMemberListNamePacket(const std::string& name, bool remove_name);
	std::unique_ptr<EQApplicationPacket> CreateMemberListStatusPacket(const std::string& name, DynamicZoneMemberStatus status);
	std::unique_ptr<EQApplicationPacket> CreateLeaderNamePacket();

	DynamicZone m_dynamiczone { DynamicZoneType::Expedition };
	std::unordered_map<std::string, ExpeditionLockoutTimer> m_lockouts;
	std::unordered_map<uint32_t, std::string> m_npc_loot_events;   // only valid inside dz zone
	std::unordered_map<uint32_t, std::string> m_spawn_loot_events; // only valid inside dz zone
};

#endif
