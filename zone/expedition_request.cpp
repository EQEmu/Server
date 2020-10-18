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

#include "expedition_request.h"
#include "client.h"
#include "expedition.h"
#include "expedition_database.h"
#include "expedition_lockout_timer.h"
#include "groups.h"
#include "raids.h"
#include "string_ids.h"
#include "worldserver.h"

extern WorldServer worldserver;

constexpr char SystemName[] = "expedition";

struct ExpeditionRequestConflict
{
	std::string character_name;
	ExpeditionLockoutTimer lockout;
};

ExpeditionRequest::ExpeditionRequest(
	std::string expedition_name, uint32_t min_players, uint32_t max_players, bool disable_messages
) :
	m_expedition_name(expedition_name),
	m_min_players(min_players),
	m_max_players(max_players),
	m_disable_messages(disable_messages)
{
}

bool ExpeditionRequest::Validate(Client* requester)
{
	m_requester = requester;
	if (!m_requester)
	{
		return false;
	}

	// a message is sent to leader for every member that fails a requirement

	BenchTimer benchmark;

	bool requirements_met = false;

	Raid* raid = m_requester->GetRaid();
	Group* group = m_requester->GetGroup();
	if (raid)
	{
		requirements_met = CanRaidRequest(raid);
	}
	else if (group)
	{
		requirements_met = CanGroupRequest(group);
	}
	else // solo request
	{
		m_leader = m_requester;
		m_leader_id = m_requester->CharacterID();
		m_leader_name = m_requester->GetName();
		requirements_met = CanMembersJoin({m_leader_name});
	}

	auto elapsed = benchmark.elapsed();
	LogExpeditions("Create validation for [{}] members took [{}s]", m_members.size(), elapsed);

	return requirements_met;
}

bool ExpeditionRequest::CanRaidRequest(Raid* raid)
{
	m_leader = raid->GetLeader();
	m_leader_name = raid->leadername;
	m_leader_id = m_leader ? m_leader->CharacterID() : database.GetCharacterID(raid->leadername);

	// live (as of September 16, 2020) supports creation even if raid count exceeds
	// expedition max. members are added up to the max ordered by group number.
	auto raid_members = raid->GetMembers();

	if (raid_members.size() > m_max_players)
	{
		// stable_sort not needed, order within a raid group may not be what is displayed
		std::sort(raid_members.begin(), raid_members.end(),
			[&](const RaidMember& lhs, const RaidMember& rhs) {
				if (m_leader_name == lhs.membername) { // leader always added first
					return true;
				} else if (m_leader_name == rhs.membername) {
					return false;
				}
				return lhs.GroupNumber < rhs.GroupNumber;
			});

		m_not_all_added_msg = fmt::format(CREATE_NOT_ALL_ADDED, "raid", SystemName,
			SystemName, m_max_players, "raid", raid_members.size());
	}

	// live still performs conflict checks for all members even those beyond max
	std::vector<std::string> member_names;
	for (int i = 0; i < raid_members.size(); ++i)
	{
		member_names.emplace_back(raid_members[i].membername);
	}

	return CanMembersJoin(member_names);
}

bool ExpeditionRequest::CanGroupRequest(Group* group)
{
	m_leader = nullptr;
	if (group->GetLeader() && group->GetLeader()->IsClient())
	{
		m_leader = group->GetLeader()->CastToClient();
	}

	// Group::GetLeaderName() is broken if group formed across zones, ask database instead
	m_leader_name = m_leader ? m_leader->GetName() : GetGroupLeaderName(group->GetID()); // group->GetLeaderName();
	m_leader_id = m_leader ? m_leader->CharacterID() : database.GetCharacterID(m_leader_name.c_str());

	std::vector<std::string> member_names;
	member_names.emplace_back(m_leader_name); // leader always added first

	for (int i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (group->membername[i][0] && m_leader_name != group->membername[i])
		{
			member_names.emplace_back(group->membername[i]);
		}
	}

	if (member_names.size() > m_max_players)
	{
		m_not_all_added_msg = fmt::format(CREATE_NOT_ALL_ADDED, "group", SystemName,
			SystemName, m_max_players, "group", member_names.size());
	}

	return CanMembersJoin(member_names);
}

std::string ExpeditionRequest::GetGroupLeaderName(uint32_t group_id)
{
	char leader_name_buffer[64] = { 0 };
	database.GetGroupLeadershipInfo(group_id, leader_name_buffer);
	return std::string(leader_name_buffer);
}

bool ExpeditionRequest::CanMembersJoin(const std::vector<std::string>& member_names)
{
	if (member_names.empty())
	{
		return false;
	}

	bool requirements_met = true;

	if (CheckMembersForConflicts(member_names))
	{
		requirements_met = false;
	}

	// live only checks player count requirement after other expensive checks pass (?)
	// maybe it's done intentionally as a way to preview lockout conflicts
	if (requirements_met)
	{
		requirements_met = IsPlayerCountValidated();
	}

	return requirements_met;
}

bool ExpeditionRequest::LoadLeaderLockouts()
{
	// leader's lockouts are used to check member conflicts and later stored in expedition
	auto lockouts = ExpeditionDatabase::LoadCharacterLockouts(m_leader_id, m_expedition_name);

	for (auto& lockout : lockouts)
	{
		if (!lockout.IsExpired())
		{
			m_lockouts.emplace(lockout.GetEventName(), lockout);

			// on live if leader has a replay lockout it never bothers checking for event conflicts
			if (m_check_event_lockouts && lockout.IsReplayTimer())
			{
				m_check_event_lockouts = false;
			}
		}
	}

	return true;
}

bool ExpeditionRequest::CheckMembersForConflicts(const std::vector<std::string>& member_names)
{
	// load data for each member and compare with leader lockouts
	auto results = ExpeditionDatabase::LoadMembersForCreateRequest(member_names, m_expedition_name);
	if (!results.Success() || !LoadLeaderLockouts())
	{
		LogExpeditions("Failed to load data to verify members for expedition request");
		return true;
	}

	bool is_solo = (member_names.size() == 1);
	bool has_conflicts = false;

	using col = LoadMembersForCreateRequestColumns::eLoadMembersForCreateRequestColumns;

	std::vector<ExpeditionRequestConflict> member_lockout_conflicts;

	uint32_t last_character_id = 0;
	for (auto row = results.begin(); row != results.end(); ++row)
	{
		uint32_t character_id      = std::strtoul(row[col::character_id], nullptr, 10);
		std::string character_name = row[col::character_name];
		bool has_expedition        = (row[col::character_expedition_id] != nullptr);

		if (character_id != last_character_id)
		{
			// defaults to online status, if offline group members implemented this needs to change
			m_members.emplace_back(character_id, character_name);

			// process event lockout conflict messages from the previous character
			for (const auto& member_lockout : member_lockout_conflicts)
			{
				SendLeaderMemberEventLockout(member_lockout.character_name, member_lockout.lockout);
			}
			member_lockout_conflicts.clear();

			if (has_expedition)
			{
				has_conflicts = true;
				SendLeaderMemberInExpedition(character_name, is_solo);

				// solo requests break out early if requester in an expedition
				if (is_solo)
				{
					return has_conflicts;
				}
			}
		}

		last_character_id = character_id;

		// compare member lockouts with leader lockouts
		if (row[col::lockout_uuid]) // lockout results may be null
		{
			auto expire_time = strtoull(row[col::lockout_expire_time], nullptr, 10);
			uint32_t duration = strtoul(row[col::lockout_duration], nullptr, 10);

			ExpeditionLockoutTimer lockout{
				row[col::lockout_uuid], m_expedition_name, row[col::lockout_event_name], expire_time, duration
			};

			if (!lockout.IsExpired())
			{
				if (lockout.IsReplayTimer())
				{
					// replay timer conflict messages always show up before event conflicts
					has_conflicts = true;
					SendLeaderMemberReplayLockout(character_name, lockout, is_solo);
				}
				else if (m_check_event_lockouts && character_id != m_leader_id)
				{
					if (m_lockouts.find(lockout.GetEventName()) == m_lockouts.end())
					{
						// leader doesn't have this lockout. queue instead of messaging
						// now so message comes after any replay lockout messages
						has_conflicts = true;
						member_lockout_conflicts.push_back({character_name, lockout});
					}
				}
			}
		}
	}

	// event lockout messages for last processed character
	for (const auto& member_lockout : member_lockout_conflicts)
	{
		SendLeaderMemberEventLockout(member_lockout.character_name, member_lockout.lockout);
	}

	return has_conflicts;
}

void ExpeditionRequest::SendLeaderMessage(
	uint16_t chat_type, uint32_t string_id, const std::initializer_list<std::string>& args)
{
	if (!m_disable_messages)
	{
		Client::SendCrossZoneMessageString(m_leader, m_leader_name, chat_type, string_id, args);
	}
}

void ExpeditionRequest::SendLeaderMemberInExpedition(const std::string& member_name, bool is_solo)
{
	if (m_disable_messages)
	{
		return;
	}

	if (is_solo)
	{
		SendLeaderMessage(Chat::Red, EXPEDITION_YOU_BELONG);
	}
	else if (m_requester)
	{
		std::string message = fmt::format(EXPEDITION_OTHER_BELONGS, m_requester->GetName(), member_name);
		Client::SendCrossZoneMessage(m_leader, m_leader_name, Chat::Red, message);
	}
}

void ExpeditionRequest::SendLeaderMemberReplayLockout(
	const std::string& member_name, const ExpeditionLockoutTimer& lockout, bool is_solo)
{
	if (m_disable_messages)
	{
		return;
	}

	auto time_remaining = lockout.GetDaysHoursMinutesRemaining();
	if (is_solo)
	{
		SendLeaderMessage(Chat::Red, EXPEDITION_YOU_PLAYED_HERE, {
			time_remaining.days, time_remaining.hours, time_remaining.mins
		});
	}
	else
	{
		SendLeaderMessage(Chat::Red, EXPEDITION_REPLAY_TIMER, {
			member_name, time_remaining.days, time_remaining.hours, time_remaining.mins
		});
	}
}

void ExpeditionRequest::SendLeaderMemberEventLockout(
	const std::string& member_name, const ExpeditionLockoutTimer& lockout)
{
	if (m_disable_messages)
	{
		return;
	}

	auto time_remaining = lockout.GetDaysHoursMinutesRemaining();
	SendLeaderMessage(Chat::Red, EXPEDITION_EVENT_TIMER, {
		member_name,
		lockout.GetEventName(),
		time_remaining.days,
		time_remaining.hours,
		time_remaining.mins,
		lockout.GetEventName()
	});
}

bool ExpeditionRequest::IsPlayerCountValidated()
{
	// note: offline group members count towards requirement but not added to expedition
	bool requirements_met = true;

	auto bypass_status = RuleI(Expedition, MinStatusToBypassPlayerCountRequirements);
	auto gm_bypass = (m_requester && m_requester->GetGM() && m_requester->Admin() >= bypass_status);

	if (m_members.size() > m_max_players)
	{
		// members were sorted at start, truncate after conflict checks to act like live
		m_members.resize(m_max_players);
	}
	else if (!gm_bypass && m_members.size() < m_min_players)
	{
		requirements_met = false;

		SendLeaderMessage(Chat::System, REQUIRED_PLAYER_COUNT, {
			fmt::format_int(m_members.size()).str(),
			fmt::format_int(m_min_players).str(),
			fmt::format_int(m_max_players).str()
		});
	}

	return requirements_met;
}
