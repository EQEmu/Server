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
#include <fmt/format.h>

extern WorldServer worldserver;

struct ExpeditionRequestConflict
{
	std::string character_name;
	ExpeditionLockoutTimer lockout;
};

ExpeditionRequest::ExpeditionRequest(
	std::string expedition_name, uint32_t min_players, uint32_t max_players,
	bool has_replay_timer, bool disable_messages
) :
	m_expedition_name(expedition_name),
	m_min_players(min_players),
	m_max_players(max_players),
	m_has_replay_timer(has_replay_timer),
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
		requirements_met = ValidateMembers(fmt::format("'{}'", m_leader_name), 1);
	}

	auto elapsed = benchmark.elapsed();
	LogExpeditions("Create validation for [{}] members took {}s", m_members.size(), elapsed);

	return requirements_met;
}

bool ExpeditionRequest::CanRaidRequest(Raid* raid)
{
	m_leader = raid->GetLeader();
	m_leader_name = raid->leadername;
	m_leader_id = m_leader ? m_leader->CharacterID() : database.GetCharacterID(raid->leadername);

	uint32_t count = 0;
	std::string query_member_names;
	for (int i = 0; i < MAX_RAID_MEMBERS; ++i)
	{
		if (raid->members[i].membername[0])
		{
			fmt::format_to(std::back_inserter(query_member_names), "'{}',", raid->members[i].membername);
			++count;
		}
	}

	if (!query_member_names.empty())
	{
		query_member_names.pop_back(); // trailing comma
	}

	return ValidateMembers(query_member_names, count);
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

	uint32_t count = 0;
	std::string query_member_names;
	for (int i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (group->membername[i][0])
		{
			fmt::format_to(std::back_inserter(query_member_names), "'{}',", group->membername[i]);
			++count;
		}
	}

	if (!query_member_names.empty())
	{
		query_member_names.pop_back(); // trailing comma
	}

	return ValidateMembers(query_member_names, count);
}

std::string ExpeditionRequest::GetGroupLeaderName(uint32_t group_id)
{
	char leader_name_buffer[64] = { 0 };
	database.GetGroupLeadershipInfo(group_id, leader_name_buffer);
	return std::string(leader_name_buffer);
}

bool ExpeditionRequest::ValidateMembers(const std::string& query_member_names, uint32_t member_count)
{
	if (query_member_names.empty() || !LoadLeaderLockouts())
	{
		return false;
	}

	// get character ids for all members through database since some may be out
	// of zone. also gets each member's existing expeditions and/or lockouts
	auto results = ExpeditionDatabase::LoadValidationData(query_member_names, m_expedition_name);
	if (!results.Success())
	{
		LogExpeditions("Failed to load data to verify members for expedition request");
		return false;
	}

	bool requirements_met = true;

	bool is_solo = (member_count == 1);
	bool has_conflicts = CheckMembersForConflicts(results, is_solo);
	if (has_conflicts)
	{
		requirements_met = false;
	}

	// live only checks player count requirement after other expensive checks pass (?)
	// maybe it's done intentionally as a way to preview lockout conflicts
	if (requirements_met)
	{
		requirements_met = IsPlayerCountValidated(member_count);
	}

	return requirements_met;
}

bool ExpeditionRequest::LoadLeaderLockouts()
{
	// leader's lockouts are used to check member conflicts and later stored in expedition
	auto results = ExpeditionDatabase::LoadCharacterLockouts(m_leader_id, m_expedition_name);
	if (!results.Success())
	{
		LogExpeditions("Failed to load leader id [{}] lockouts ([{}])", m_leader_id, m_leader_name);
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row)
	{
		uint64_t expire_time = strtoull(row[0], nullptr, 10);
		uint32_t duration = strtoul(row[1], nullptr, 10);
		ExpeditionLockoutTimer lockout{m_expedition_name, row[2], expire_time, duration, true};

		// client window hides timers with less than 60s remaining, optionally count them as expired
		if (lockout.GetSecondsRemaining() <= RuleI(Expedition, RequestExpiredLockoutLeewaySeconds))
		{
			LogExpeditionsModerate(
				"Ignoring leader [{}] lockout [{}] with [{}] seconds remaining due to expired leeway rule",
				m_leader_id, lockout.GetEventName(), lockout.GetSecondsRemaining()
			);
		}
		else
		{
			m_lockouts.emplace(row[2], lockout);

			// on live if leader has a replay lockout it never bothers checking for event conflicts
			if (m_check_event_lockouts && m_has_replay_timer && strcmp(row[2], DZ_REPLAY_TIMER_NAME) == 0)
			{
				m_check_event_lockouts = false;
			}
		}
	}

	return true;
}

bool ExpeditionRequest::CheckMembersForConflicts(MySQLRequestResult& results, bool is_solo)
{
	// leader lockouts were pre-loaded to compare with members below
	bool has_conflicts = false;

	std::vector<ExpeditionRequestConflict> member_lockout_conflicts;

	bool leader_processed = false;
	uint32_t last_character_id = 0;
	for (auto row = results.begin(); row != results.end(); ++row)
	{
		auto character_id = static_cast<uint32_t>(std::strtoul(row[0], nullptr, 10));
		std::string character_name(row[1]);

		if (character_id != last_character_id)
		{
			// defaults to online status, if offline group members implemented this needs to change
			m_members.emplace_back(ExpeditionMember{character_id, character_name});

			// process event lockout conflict messages from the previous character
			for (const auto& member_lockout : member_lockout_conflicts)
			{
				SendLeaderMemberEventLockout(member_lockout.character_name, member_lockout.lockout);
			}
			member_lockout_conflicts.clear();

			// current character existing expedition check
			if (row[2])
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
		if (row[3] && row[4] && row[5])
		{
			auto expire_time = strtoull(row[3], nullptr, 10);
			auto original_duration = strtoul(row[4], nullptr, 10);
			std::string event_name(row[5]);

			ExpeditionLockoutTimer lockout(m_expedition_name, event_name, expire_time, original_duration);

			// client window hides timers with less than 60s remaining, optionally count them as expired
			if (lockout.GetSecondsRemaining() <= RuleI(Expedition, RequestExpiredLockoutLeewaySeconds))
			{
				LogExpeditionsModerate(
					"Ignoring character [{}] lockout [{}] with [{}] seconds remaining due to expired leeway rule",
					character_id, lockout.GetEventName(), lockout.GetSecondsRemaining()
				);
			}
			else
			{
				// replay timer conflict messages always show up before event conflicts
				if (/*m_has_replay_timer && */event_name == DZ_REPLAY_TIMER_NAME)
				{
					has_conflicts = true;
					SendLeaderMemberReplayLockout(character_name, lockout, is_solo);
					// replay timers no longer also show up as event conflicts
					//SendLeaderMemberEventLockout(character_name, lockout);
				}
				else if (m_check_event_lockouts && character_id != m_leader_id)
				{
					if (m_lockouts.find(event_name) == m_lockouts.end())
					{
						// leader doesn't have this lockout
						// queue instead of messaging now so they come after any replay lockout messages
						has_conflicts = true;
						member_lockout_conflicts.emplace_back(ExpeditionRequestConflict{character_name, lockout});
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
	uint16_t chat_type, uint32_t string_id, const std::initializer_list<std::string>& parameters)
{
	if (!m_disable_messages)
	{
		Client::SendCrossZoneMessageString(m_leader, m_leader_name, chat_type, string_id, parameters);
	}
}

void ExpeditionRequest::SendLeaderMemberInExpedition(const std::string& member_name, bool is_solo)
{
	if (m_disable_messages) {
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
	if (m_disable_messages || lockout.GetSecondsRemaining() <= 0)
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
	if (m_disable_messages || lockout.GetSecondsRemaining() <= 0)
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

bool ExpeditionRequest::IsPlayerCountValidated(uint32_t member_count)
{
	// note: offline group members count towards requirement but not added to expedition
	bool requirements_met = true;

	auto bypass_status = RuleI(Expedition, MinStatusToBypassPlayerCountRequirements);
	auto gm_bypass = (m_requester && m_requester->GetGM() && m_requester->Admin() >= bypass_status);

	if (!gm_bypass && (member_count < m_min_players || member_count > m_max_players))
	{
		requirements_met = false;

		SendLeaderMessage(Chat::Red, REQUIRED_PLAYER_COUNT, {
			fmt::format_int(member_count).str(),
			fmt::format_int(m_min_players).str(),
			fmt::format_int(m_max_players).str()
		});
	}

	return requirements_met;
}
