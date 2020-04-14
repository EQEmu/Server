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

#include "expedition_lockout_timer.h"
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Client;
class EQApplicationPacket;
class ExpeditionRequest;
class MySQLRequestResult;
class ServerPacket;

extern const char* const DZ_YOU_NOT_ASSIGNED;
extern const char* const EXPEDITION_OTHER_BELONGS;

enum class DynamicZoneType : uint8_t // DynamicZoneActiveType
{
	None = 0,
	Expedition,
	Tutorial,
	Task,
	Mission,
	Quest
};

enum class ExpeditionMemberStatus : uint8_t
{
	Unknown = 0,
	Online,
	Offline,
	InDynamicZone,
	LinkDead
};

struct ExpeditionMember
{
	uint32_t char_id = 0;
	std::string name;
	ExpeditionMemberStatus status = ExpeditionMemberStatus::Online;

	ExpeditionMember() {}
	ExpeditionMember(uint32_t char_id_, const std::string& name_) : char_id(char_id_), name(name_) {}
	ExpeditionMember(uint32_t char_id_, const std::string& name_, ExpeditionMemberStatus status_)
		: char_id(char_id_), name(name_), status(status_) {}
};

class Expedition
{
public:
	Expedition() = delete;
	Expedition(uint32_t id, std::string expedition_name, const ExpeditionMember& leader,
		uint32_t min_players, uint32_t max_players, bool replay_timer);

	static Expedition* TryCreate(
		Client* requester, std::string name, uint32_t min_players, uint32_t max_players, bool replay_timer);
	static void CacheFromDatabase(uint32_t expedition_id);
	static bool CacheAllFromDatabase();
	static void CacheExpeditions(MySQLRequestResult& results);
	static void LoadAllClientLockouts(Client* client);
	static Expedition* FindCachedExpeditionByCharacterID(uint32_t character_id);
	static Expedition* FindCachedExpeditionByCharacterName(const std::string& char_name);
	static Expedition* FindCachedExpeditionByID(uint32_t expedition_id);
	static Expedition* FindExpeditionByInstanceID(uint32_t instance_id);
	static void HandleWorldMessage(ServerPacket* pack);

	uint32_t GetID() const { return m_id; }
	uint32_t GetLeaderID() const { return m_leader.char_id; }
	uint32_t GetMinPlayers() const { return m_min_players; }
	uint32_t GetMaxPlayers() const { return m_max_players; }
	uint32_t GetMemberCount() const { return static_cast<uint32_t>(m_members.size()); }
	const std::string& GetName() const { return m_expedition_name; }
	const std::string& GetLeaderName() const { return m_leader.name; }
	const std::unordered_map<std::string, ExpeditionLockoutTimer>& GetLockouts() const { return m_lockouts; }
	const std::vector<ExpeditionMember>& GetMembers() const { return m_members; }

	bool AddMember(const std::string& add_char_name, uint32_t add_char_id);
	bool HasMember(const std::string& name);
	bool HasMember(uint32_t character_id);
	void RemoveAllMembers();
	bool RemoveMember(const std::string& remove_char_name);
	void SetMemberStatus(Client* client, ExpeditionMemberStatus status);
	void SetNewLeader(uint32_t new_leader_id, const std::string& new_leader_name);
	void SwapMember(Client* add_client, const std::string& remove_char_name);

	void AddLockout(const std::string& event_name, uint32_t seconds);
	void AddReplayLockout(uint32_t seconds);
	bool HasLockout(const std::string& event_name);
	bool HasReplayLockout();
	void RemoveLockout(const std::string& event_name);

	void SendClientExpeditionInfo(Client* client);

	void DzAddPlayer(Client* requester, std::string add_char_name, std::string swap_remove_name = {});
	void DzAddPlayerContinue(std::string leader_name, std::string add_char_name, std::string swap_remove_name = {});
	void DzInviteResponse(Client* add_client, bool accepted, bool has_swap_name, std::string swap_remove_name);
	void DzMakeLeader(Client* requester, std::string new_leader_name);
	void DzPlayerList(Client* requester);
	void DzRemovePlayer(Client* requester, std::string remove_char_name);
	void DzSwapPlayer(Client* requester, std::string remove_char_name, std::string add_char_name);
	void DzQuit(Client* requester);
	void DzKickPlayers(Client* requester);

#if 0
	bool AssignInstance(uint32_t instance_id, bool update_db = true);
	uint32_t CreateInstance(std::string zone, uint32_t version, uint32_t duration); // m_dynamiczone
#endif
	uint32_t GetInstanceID() const { return 77; /*return m_instance_id;*/ } // todo: GetDynamicZoneID()
	DynamicZoneType GetType() const { return DynamicZoneType::Expedition; } // m_dynamiczone

	static const uint32_t REPLAY_TIMER_ID;
	static const uint32_t EVENT_TIMER_ID;

private:
	void AddInternalLockout(ExpeditionLockoutTimer&& lockout_timer);
	void AddInternalMember(const std::string& char_name, uint32_t char_id, bool is_current_member = true, bool offline = false);
	bool ChooseNewLeader();
	bool ConfirmLeaderCommand(Client* requester);
	void LoadMembers();
	bool ProcessAddConflicts(Client* leader_client, Client* add_client, bool swapping);
	void ProcessLeaderChanged(uint32_t new_leader_id, const std::string& new_leader_name);
	void ProcessLockoutUpdate(const std::string& event_name, uint64_t expire_time, uint32_t duration, bool remove);
	void ProcessMakeLeader(Client* old_leader, Client* new_leader, const std::string& new_leader_name, bool is_online);
	void ProcessMemberAdded(std::string added_char_name, uint32_t added_char_id);
	void ProcessMemberRemoved(std::string removed_char_name, uint32_t removed_char_id);
	void SaveLockouts(ExpeditionRequest& request);
	void SaveMembers(ExpeditionRequest& request);
	void SendClientExpeditionInvite(Client* client, const std::string& inviter_name, const std::string& swap_remove_name);
	void SendLeaderMessage(Client* leader_client, uint16_t chat_type, uint32_t string_id, const std::initializer_list<std::string>& parameters = {});
	void SendUpdatesToZoneMembers(bool clear = false);
	void SendWorldExpeditionUpdate(bool destroyed = false);
	void SendWorldGetOnlineMembers();
	void SendWorldAddPlayerInvite(const std::string& inviter_name, const std::string& swap_remove_name, const std::string& add_name);
	void SendWorldLeaderChanged();
	void SendWorldLockoutUpdate(const std::string& event_name, uint64_t expire_time, uint32_t duration, bool remove = false);
	void SendWorldMakeLeaderRequest(const std::string& requester_name, const std::string& new_leader_name);
	void SendWorldMemberChanged(const std::string& char_name, uint32_t char_id, bool remove);
	void SendWorldMemberStatus(uint32_t character_id, ExpeditionMemberStatus status);
	void SendWorldMemberSwapped(const std::string& remove_char_name, uint32_t remove_char_id, const std::string& add_char_name, uint32_t add_char_id);
	void TryAddClient(Client* add_client, std::string inviter_name, std::string orig_add_name, std::string swap_remove_name, Client* leader_client = nullptr);
	void UpdateMemberStatus(uint32_t update_character_id, ExpeditionMemberStatus status);

	ExpeditionMember GetMemberData(uint32_t character_id);
	ExpeditionMember GetMemberData(const std::string& character_name);
	std::unique_ptr<EQApplicationPacket> CreateInfoPacket(bool clear = false);
	std::unique_ptr<EQApplicationPacket> CreateInvitePacket(const std::string& inviter_name, const std::string& swap_remove_name);
	std::unique_ptr<EQApplicationPacket> CreateMemberListPacket(bool clear = false);
	std::unique_ptr<EQApplicationPacket> CreateMemberListNamePacket(const std::string& name, bool remove_name);
	std::unique_ptr<EQApplicationPacket> CreateMemberListStatusPacket(const std::string& name, ExpeditionMemberStatus status);
	std::unique_ptr<EQApplicationPacket> CreateLeaderNamePacket();

	uint32_t    m_id               = 0;
	//uint32_t    m_instance_id      = 0; // todo: DynamicZone m_dynamiczone
	uint32_t    m_min_players      = 0;
	uint32_t    m_max_players      = 0;
	bool        m_has_replay_timer = false;
	std::string m_expedition_name;
	ExpeditionMember m_leader;
	std::vector<ExpeditionMember> m_members; // current members
	std::unordered_set<uint32_t> m_member_id_history; // track past members to allow invites for replay timer bypass
	std::unordered_map<std::string, ExpeditionLockoutTimer> m_lockouts;
};

#endif
