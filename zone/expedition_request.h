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

#ifndef EXPEDITION_REQUEST_H
#define EXPEDITION_REQUEST_H

#include "expedition.h"
#include "expedition_lockout_timer.h"
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

class Client;
class Group;
class MySQLRequestResult;
class Raid;
class ServerPacket;

class ExpeditionRequest
{
public:
	ExpeditionRequest(
		std::string expedition_name, uint32_t min_players, uint32_t max_players,
		bool disable_messages = false);

	bool Validate(Client* requester);

	const std::string& GetExpeditionName() const { return m_expedition_name; }
	Client* GetLeaderClient() const { return m_leader; }
	uint32_t GetLeaderID() const { return m_leader_id; }
	const std::string& GetLeaderName() const { return m_leader_name; }
	const std::string& GetNotAllAddedMessage() const { return m_not_all_added_msg; }
	uint32_t GetMinPlayers() const { return m_min_players; }
	uint32_t GetMaxPlayers() const { return m_max_players; }
	std::vector<ExpeditionMember> GetMembers() const { return m_members; }
	std::unordered_map<std::string, ExpeditionLockoutTimer> GetLockouts() const { return m_lockouts; }

private:
	bool CanMembersJoin(const std::vector<std::string>& member_names);
	bool CanRaidRequest(Raid* raid);
	bool CanGroupRequest(Group* group);
	bool CheckMembersForConflicts(const std::vector<std::string>& member_names);
	std::string GetGroupLeaderName(uint32_t group_id);
	bool IsPlayerCountValidated();
	bool LoadLeaderLockouts();
	void SendLeaderMemberInExpedition(const std::string& member_name, bool is_solo);
	void SendLeaderMemberReplayLockout(const std::string& member_name, const ExpeditionLockoutTimer& lockout, bool is_solo);
	void SendLeaderMemberEventLockout(const std::string& member_name, const ExpeditionLockoutTimer& lockout);
	void SendLeaderMessage(uint16_t chat_type, uint32_t string_id, const std::initializer_list<std::string>& args = {});

	Client*  m_requester            = nullptr;
	Client*  m_leader               = nullptr;
	uint32_t m_leader_id            = 0;
	uint32_t m_min_players          = 0;
	uint32_t m_max_players          = 0;
	bool     m_check_event_lockouts = true;
	bool     m_disable_messages     = false;
	std::string m_expedition_name;
	std::string m_leader_name;
	std::string m_not_all_added_msg;
	std::vector<ExpeditionMember> m_members;
	std::unordered_map<std::string, ExpeditionLockoutTimer> m_lockouts;
};

#endif
