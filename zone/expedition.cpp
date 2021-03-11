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
#include "expedition_lockout_timer.h"
#include "expedition_request.h"
#include "client.h"
#include "string_ids.h"
#include "worldserver.h"
#include "zonedb.h"
#include "../common/eqemu_logsys.h"
#include "../common/util/uuid.h"

extern WorldServer worldserver;
extern Zone* zone;

// message string 8271 (not in emu clients)
const char* const DZ_YOU_NOT_ASSIGNED        = "You could not use this command because you are not currently assigned to a dynamic zone.";
// message string 9265 (not in emu clients)
const char* const EXPEDITION_OTHER_BELONGS   = "{} attempted to create an expedition but {} already belongs to one.";
// lockout warnings were added to live in March 11 2020 patch
const char* const DZADD_INVITE_WARNING       = "Warning! You will be given replay timers for the following events if you enter %s:";
const char* const DZADD_INVITE_WARNING_TIMER = "%s - %sD:%sH:%sM";
const char* const KICKPLAYERS_EVERYONE       = "Everyone";
// message string 8312 added in September 08 2020 Test patch (used by both dz and shared tasks)
const char* const CREATE_NOT_ALL_ADDED       = "Not all players in your {} were added to the {}. The {} can take a maximum of {} players, and your {} has {}.";
// various expeditions re-use these strings when locking
constexpr char LOCK_CLOSE[]                  = "Your expedition is nearing its close. You cannot bring any additional people into your expedition at this time.";
constexpr char LOCK_BEGIN[]                  = "The trial has begun. You cannot bring any additional people into your expedition at this time.";

const int32_t Expedition::REPLAY_TIMER_ID = -1;
const int32_t Expedition::EVENT_TIMER_ID  = 1;

Expedition::Expedition(
	uint32_t id, const std::string& uuid, const DynamicZone& dynamic_zone, const std::string& expedition_name,
	const ExpeditionMember& leader, uint32_t min_players, uint32_t max_players
) :
	m_id(id),
	m_uuid(uuid),
	m_dynamiczone(dynamic_zone),
	m_expedition_name(expedition_name),
	m_leader(leader),
	m_min_players(min_players),
	m_max_players(max_players)
{
}

Expedition* Expedition::TryCreate(
	Client* requester, DynamicZone& dynamiczone, ExpeditionRequest& request)
{
	if (!requester || !zone)
	{
		return nullptr;
	}

	// request parses leader, members list, and lockouts while validating
	if (!request.Validate(requester))
	{
		LogExpeditionsModerate("[{}] request by [{}] denied", request.GetExpeditionName(), requester->GetName());
		return nullptr;
	}

	auto dynamic_zone_id = dynamiczone.Create();
	if (dynamic_zone_id == 0)
	{
		// live uses this message when trying to enter an instance that isn't ready
		// we can use it as the client error message if instance creation fails
		requester->MessageString(Chat::Red, DZ_PREVENT_ENTERING);
		LogExpeditions("Failed to create a dynamic zone instance for expedition");
		return nullptr;
	}

	std::string expedition_uuid = EQ::Util::UUID::Generate().ToString();

	// unique expedition ids are created from database via auto-increment column
	auto expedition_id = ExpeditionDatabase::InsertExpedition(
		expedition_uuid,
		dynamiczone.GetID(),
		request.GetExpeditionName(),
		request.GetLeaderID(),
		request.GetMinPlayers(),
		request.GetMaxPlayers()
	);

	if (expedition_id)
	{
		auto expedition = std::make_unique<Expedition>(
			expedition_id,
			expedition_uuid,
			dynamiczone,
			request.GetExpeditionName(),
			ExpeditionMember{ request.GetLeaderID(), request.GetLeaderName() },
			request.GetMinPlayers(),
			request.GetMaxPlayers()
		);

		LogExpeditions(
			"Created [{}] [{}] instance id: [{}] leader: [{}] minplayers: [{}] maxplayers: [{}]",
			expedition->GetID(),
			expedition->GetName(),
			expedition->GetInstanceID(),
			expedition->GetLeaderName(),
			expedition->GetMinPlayers(),
			expedition->GetMaxPlayers()
		);

		expedition->SaveMembers(request);
		expedition->SaveLockouts(request);

		auto inserted = zone->expedition_cache.emplace(expedition_id, std::move(expedition));

		inserted.first->second->SendUpdatesToZoneMembers();
		inserted.first->second->SendWorldExpeditionUpdate(ServerOP_ExpeditionCreate); // cache in other zones
		inserted.first->second->SendLeaderMessage(request.GetLeaderClient(),
			Chat::System, EXPEDITION_AVAILABLE, { request.GetExpeditionName() });

		if (!request.GetNotAllAddedMessage().empty())
		{
			Client::SendCrossZoneMessage(request.GetLeaderClient(), request.GetLeaderName(),
				Chat::System, request.GetNotAllAddedMessage());
		}

		return inserted.first->second.get();
	}

	return nullptr;
}

void Expedition::CacheExpeditions(MySQLRequestResult& results)
{
	if (!results.Success() || !zone)
	{
		return;
	}

	std::vector<uint32_t> expedition_ids;
	std::vector<uint32_t> dynamic_zone_ids;;
	std::vector<std::pair<uint32_t, uint32_t>> expedition_character_ids;

	using col = LoadExpeditionColumns::eLoadExpeditionColumns;

	uint32_t last_expedition_id = 0;

	for (auto row = results.begin(); row != results.end(); ++row)
	{
		auto expedition_id = strtoul(row[col::id], nullptr, 10);

		if (expedition_id != last_expedition_id)
		{
			expedition_ids.emplace_back(expedition_id);

			uint32_t leader_id = strtoul(row[col::leader_id], nullptr, 10);
			uint32_t dynamic_zone_id = strtoul(row[col::dz_id], nullptr, 10);

			dynamic_zone_ids.emplace_back(dynamic_zone_id);

			std::unique_ptr<Expedition> expedition = std::make_unique<Expedition>(
				expedition_id,
				row[col::uuid],                                         // expedition uuid
				DynamicZone{ dynamic_zone_id },
				row[col::expedition_name],                              // expedition name
				ExpeditionMember{ leader_id, row[col::leader_name] },   // expedition leader id, name
				strtoul(row[col::min_players], nullptr, 10),            // min_players
				strtoul(row[col::max_players], nullptr, 10)             // max_players
			);

			bool add_replay_on_join = (strtoul(row[col::add_replay_on_join], nullptr, 10) != 0);
			bool is_locked = (strtoul(row[col::is_locked], nullptr, 10) != 0);

			expedition->SetReplayLockoutOnMemberJoin(add_replay_on_join);
			expedition->SetLocked(is_locked, ExpeditionLockMessage::None);

			zone->expedition_cache.emplace(expedition_id, std::move(expedition));
		}

		last_expedition_id = expedition_id;

		// looping expedition members
		auto current_expedition = Expedition::FindCachedExpeditionByID(last_expedition_id);
		if (current_expedition)
		{
			auto member_id = strtoul(row[col::member_id], nullptr, 10);
			current_expedition->AddInternalMember(
				row[col::member_name], member_id, ExpeditionMemberStatus::Offline);
			expedition_character_ids.emplace_back(expedition_id, member_id);
		}
	}

	// ask world for online members from all cached expeditions at once
	Expedition::SendWorldGetOnlineMembers(expedition_character_ids);

	// bulk load dynamic zone data and expedition lockouts for cached expeditions
	auto dynamic_zones = DynamicZone::LoadMultipleDzFromDatabase(dynamic_zone_ids);
	auto expedition_lockouts = ExpeditionDatabase::LoadMultipleExpeditionLockouts(expedition_ids);

	for (const auto& expedition_id : expedition_ids)
	{
		auto expedition = Expedition::FindCachedExpeditionByID(expedition_id);
		if (expedition)
		{
			auto dz_iter = dynamic_zones.find(expedition->GetDynamicZoneID());
			if (dz_iter != dynamic_zones.end())
			{
				expedition->m_dynamiczone = dz_iter->second;
			}

			auto lockout_iter = expedition_lockouts.find(expedition->GetID());
			if (lockout_iter != expedition_lockouts.end())
			{
				expedition->m_lockouts = lockout_iter->second;
			}

			// send member updates now that all data is loaded for the cached expedition
			expedition->SendUpdatesToZoneMembers();
		}
	}
}

void Expedition::CacheFromDatabase(uint32_t expedition_id)
{
	if (zone)
	{
		BenchTimer benchmark;

		auto results = ExpeditionDatabase::LoadExpedition(expedition_id);
		if (!results.Success())
		{
			LogExpeditions("Failed to load Expedition [{}] for zone cache", expedition_id);
			return;
		}

		CacheExpeditions(results);

		auto elapsed = benchmark.elapsed();
		LogExpeditions("Caching new expedition [{}] took [{}s]", expedition_id, elapsed);
	}
}

bool Expedition::CacheAllFromDatabase()
{
	if (!zone)
	{
		return false;
	}

	BenchTimer benchmark;

	zone->expedition_cache.clear();

	// load all active expeditions and members to current zone cache
	auto results = ExpeditionDatabase::LoadAllExpeditions();
	if (!results.Success())
	{
		LogExpeditions("Failed to load Expeditions for zone cache");
		return false;
	}

	CacheExpeditions(results);

	auto elapsed = benchmark.elapsed();
	LogExpeditions("Caching [{}] expedition(s) took [{}s]", zone->expedition_cache.size(), elapsed);

	return true;
}

void Expedition::SaveLockouts(ExpeditionRequest& request)
{
	m_lockouts = request.GetLockouts();
	ExpeditionDatabase::InsertLockouts(m_id, m_lockouts);
}

void Expedition::SaveMembers(ExpeditionRequest& request)
{
	m_members = request.GetMembers();

	std::vector<uint32_t> member_ids;
	for (const auto& member : m_members)
	{
		member_ids.emplace_back(member.char_id);
	}

	ExpeditionDatabase::InsertMembers(m_id, m_members);
	m_dynamiczone.SaveInstanceMembersToDatabase(member_ids);
}

Expedition* Expedition::FindCachedExpeditionByCharacterID(uint32_t character_id)
{
	if (zone)
	{
		for (const auto& expedition : zone->expedition_cache)
		{
			if (expedition.second->HasMember(character_id))
			{
				return expedition.second.get();
			}
		}
	}
	return nullptr;
}

Expedition* Expedition::FindCachedExpeditionByCharacterName(const std::string& char_name)
{
	if (zone)
	{
		for (const auto& expedition : zone->expedition_cache)
		{
			if (expedition.second->HasMember(char_name))
			{
				return expedition.second.get();
			}
		}
	}
	return nullptr;
}

Expedition* Expedition::FindCachedExpeditionByDynamicZoneID(uint32_t dz_id)
{
	if (zone && dz_id != 0)
	{
		for (const auto& cached_expedition : zone->expedition_cache)
		{
			if (cached_expedition.second->GetDynamicZone().GetID() == dz_id)
			{
				return cached_expedition.second.get();
			}
		}
	}
	return nullptr;
}

Expedition* Expedition::FindCachedExpeditionByID(uint32_t expedition_id)
{
	if (zone && expedition_id)
	{
		auto expedition_cache_iter = zone->expedition_cache.find(expedition_id);
		if (expedition_cache_iter != zone->expedition_cache.end())
		{
			return expedition_cache_iter->second.get();
		}
	}
	return nullptr;
}

Expedition* Expedition::FindCachedExpeditionByZoneInstance(uint32_t zone_id, uint32_t instance_id)
{
	if (zone && zone_id != 0 && instance_id != 0)
	{
		for (const auto& cached_expedition : zone->expedition_cache)
		{
			if (cached_expedition.second->GetDynamicZone().GetZoneID() == zone_id &&
			    cached_expedition.second->GetDynamicZone().GetInstanceID() == instance_id)
			{
				return cached_expedition.second.get();
			}
		}
	}
	return nullptr;
}

bool Expedition::HasLockout(const std::string& event_name)
{
	return (m_lockouts.find(event_name) != m_lockouts.end());
}

bool Expedition::HasReplayLockout()
{
	return HasLockout(DZ_REPLAY_TIMER_NAME);
}

bool Expedition::HasMember(uint32_t character_id)
{
	return std::any_of(m_members.begin(), m_members.end(), [&](const ExpeditionMember& member) {
		return member.char_id == character_id;
	});
}

bool Expedition::HasMember(const std::string& character_name)
{
	return std::any_of(m_members.begin(), m_members.end(), [&](const ExpeditionMember& member) {
		return (strcasecmp(member.name.c_str(), character_name.c_str()) == 0);
	});
}

ExpeditionMember Expedition::GetMemberData(uint32_t character_id)
{
	auto it = std::find_if(m_members.begin(), m_members.end(), [&](const ExpeditionMember& member) {
		return member.char_id == character_id;
	});

	ExpeditionMember member_data;
	if (it != m_members.end())
	{
		member_data = *it;
	}
	return member_data;
}

ExpeditionMember Expedition::GetMemberData(const std::string& character_name)
{
	auto it = std::find_if(m_members.begin(), m_members.end(), [&](const ExpeditionMember& member) {
		return (strcasecmp(member.name.c_str(), character_name.c_str()) == 0);
	});

	ExpeditionMember member_data;
	if (it != m_members.end())
	{
		member_data = *it;
	}
	return member_data;
}

void Expedition::SetReplayLockoutOnMemberJoin(bool add_on_join, bool update_db)
{
	m_add_replay_on_join = add_on_join;

	if (update_db)
	{
		ExpeditionDatabase::UpdateReplayLockoutOnJoin(m_id, add_on_join);
		SendWorldSettingChanged(ServerOP_ExpeditionReplayOnJoin, m_add_replay_on_join);
	}
}

void Expedition::AddReplayLockout(uint32_t seconds)
{
	AddLockout(DZ_REPLAY_TIMER_NAME, seconds);
}

void Expedition::AddLockout(const std::string& event_name, uint32_t seconds)
{
	auto lockout = ExpeditionLockoutTimer::CreateLockout(m_expedition_name, event_name, seconds, m_uuid);
	AddLockout(lockout);
}

void Expedition::AddLockout(const ExpeditionLockoutTimer& lockout, bool members_only)
{
	if (!members_only)
	{
		ExpeditionDatabase::InsertLockout(m_id, lockout);
	}
	ExpeditionDatabase::InsertMembersLockout(m_members, lockout);

	ProcessLockoutUpdate(lockout, false, members_only);
	SendWorldLockoutUpdate(lockout, false, members_only);
}

void Expedition::AddLockoutDuration(const std::string& event_name, int seconds, bool members_only)
{
	// lockout timers use unsigned durations to define intent but we may need
	// to insert a new lockout while still supporting timer reductions
	auto lockout = ExpeditionLockoutTimer::CreateLockout(
		m_expedition_name, event_name, std::max(0, seconds), m_uuid);

	if (!members_only)
	{
		auto it = m_lockouts.find(event_name);
		if (it != m_lockouts.end())
		{
			it->second.AddLockoutTime(seconds);
			ExpeditionDatabase::InsertLockout(m_id, it->second); // replaces current one
		}
		else
		{
			ExpeditionDatabase::InsertLockout(m_id, lockout);
		}
	}

	// processing lockout duration applies multiplier again in client methods,
	// update database with modified value now but pass original on
	int modified_seconds = static_cast<int>(seconds * RuleR(Expedition, LockoutDurationMultiplier));
	ExpeditionDatabase::AddLockoutDuration(m_members, lockout, modified_seconds);

	ProcessLockoutDuration(lockout, seconds, members_only);
	SendWorldLockoutDuration(lockout, seconds, members_only);
}

void Expedition::AddReplayLockoutDuration(int seconds, bool members_only)
{
	AddLockoutDuration(DZ_REPLAY_TIMER_NAME, seconds, members_only);
}

void Expedition::UpdateLockoutDuration(
	const std::string& event_name, uint32_t seconds, bool members_only)
{
	// some live expeditions update existing lockout timers during progression
	auto it = m_lockouts.find(event_name);
	if (it != m_lockouts.end())
	{
		seconds = static_cast<uint32_t>(seconds * RuleR(Expedition, LockoutDurationMultiplier));

		uint64_t expire_time = it->second.GetStartTime() + seconds;
		AddLockout({ m_uuid, m_expedition_name, event_name, expire_time, seconds }, members_only);
	}
}

void Expedition::RemoveLockout(const std::string& event_name)
{
	ExpeditionDatabase::DeleteLockout(m_id, event_name);
	ExpeditionDatabase::DeleteMembersLockout(m_members, m_expedition_name, event_name);

	ExpeditionLockoutTimer lockout{m_uuid, m_expedition_name, event_name, 0, 0};
	ProcessLockoutUpdate(lockout, true);
	SendWorldLockoutUpdate(lockout, true);
}

void Expedition::AddInternalMember(
	const std::string& char_name, uint32_t character_id, ExpeditionMemberStatus status)
{
	auto it = std::find_if(m_members.begin(), m_members.end(),
		[character_id](const ExpeditionMember& member) {
			return member.char_id == character_id;
		});

	if (it == m_members.end())
	{
		m_members.emplace_back(character_id, char_name, status);
	}
}

bool Expedition::AddMember(const std::string& add_char_name, uint32_t add_char_id)
{
	if (HasMember(add_char_id))
	{
		return false;
	}

	ExpeditionDatabase::InsertMember(m_id, add_char_id);
	m_dynamiczone.AddCharacter(add_char_id);

	ProcessMemberAdded(add_char_name, add_char_id);
	SendWorldMemberChanged(add_char_name, add_char_id, false);

	return true;
}

void Expedition::RemoveAllMembers(bool enable_removal_timers)
{
	m_dynamiczone.RemoveAllCharacters(enable_removal_timers);

	ExpeditionDatabase::DeleteAllMembers(m_id);

	SendUpdatesToZoneMembers(true);
	SendWorldExpeditionUpdate(ServerOP_ExpeditionMembersRemoved);

	m_members.clear();
}

bool Expedition::RemoveMember(const std::string& remove_char_name)
{
	auto member = GetMemberData(remove_char_name);
	if (!member.IsValid())
	{
		return false;
	}

	ExpeditionDatabase::DeleteMember(m_id, member.char_id);
	m_dynamiczone.RemoveCharacter(member.char_id);

	ProcessMemberRemoved(member.name, member.char_id);
	SendWorldMemberChanged(member.name, member.char_id, true);

	return true;
}

void Expedition::SwapMember(Client* add_client, const std::string& remove_char_name)
{
	if (!add_client || remove_char_name.empty())
	{
		return;
	}

	auto member = GetMemberData(remove_char_name);
	if (!member.IsValid())
	{
		return;
	}

	// make remove and add atomic to avoid racing with separate world messages
	ExpeditionDatabase::DeleteMember(m_id, member.char_id);
	ExpeditionDatabase::InsertMember(m_id, add_client->CharacterID());
	m_dynamiczone.RemoveCharacter(member.char_id);
	m_dynamiczone.AddCharacter(add_client->CharacterID());

	ProcessMemberRemoved(member.name, member.char_id);
	ProcessMemberAdded(add_client->GetName(), add_client->CharacterID());
	SendWorldMemberSwapped(member.name, member.char_id, add_client->GetName(), add_client->CharacterID());
}

void Expedition::SetMemberStatus(Client* client, ExpeditionMemberStatus status)
{
	if (client)
	{
		UpdateMemberStatus(client->CharacterID(), status);
		SendWorldMemberStatus(client->CharacterID(), status);

		// world could detect this itself but it'd have to process member status updates
		// a member coming online will trigger a leader change if all members were offline
		if (m_leader.status == ExpeditionMemberStatus::Offline)
		{
			SendWorldExpeditionUpdate(ServerOP_ExpeditionChooseNewLeader);
		}
	}
}

void Expedition::UpdateMemberStatus(uint32_t update_member_id, ExpeditionMemberStatus status)
{
	auto member_data = GetMemberData(update_member_id);
	if (!member_data.IsValid())
	{
		return;
	}

	if (status == ExpeditionMemberStatus::InDynamicZone && !RuleB(Expedition, EnableInDynamicZoneStatus))
	{
		status = ExpeditionMemberStatus::Online;
	}

	if (update_member_id == m_leader.char_id)
	{
		m_leader.status = status;
	}

	// if zone already had this member status cached avoid packet update to clients
	auto it = std::find_if(m_members.begin(), m_members.end(),
		[&](const ExpeditionMember& member) { return member.char_id == update_member_id; });

	if (it != m_members.end() && it->status == status)
	{
		return;
	}

	auto outapp_member_status = CreateMemberListStatusPacket(member_data.name, status);

	for (auto& member : m_members)
	{
		if (member.char_id == update_member_id)
		{
			member.status = status;
		}

		Client* member_client = entity_list.GetClientByCharID(member.char_id);
		if (member_client)
		{
			member_client->QueuePacket(outapp_member_status.get());
		}
	}
}

void Expedition::SendClientExpeditionInvite(
	Client* client, const std::string& inviter_name, const std::string& swap_remove_name)
{
	if (!client)
	{
		return;
	}

	LogExpeditionsModerate(
		"Sending expedition [{}] invite to player [{}] inviter [{}] swap name [{}]",
		m_id, client->GetName(), inviter_name, swap_remove_name
	);

	client->SetPendingExpeditionInvite({ m_id, inviter_name, swap_remove_name });

	client->MessageString(Chat::System, EXPEDITION_ASKED_TO_JOIN,
		m_leader.name.c_str(), m_expedition_name.c_str());

	// live (as of March 11 2020 patch) sends warnings for lockouts added
	// during current expedition that client would receive on entering dz
	bool warned = false;
	for (const auto& lockout_iter : m_lockouts)
	{
		// live doesn't issue a warning for the dz's replay timer
		const ExpeditionLockoutTimer& lockout = lockout_iter.second;
		if (!lockout.IsReplayTimer() && !lockout.IsExpired() && lockout.IsFromExpedition(m_uuid) &&
		    !client->HasExpeditionLockout(m_expedition_name, lockout.GetEventName()))
		{
			if (!warned)
			{
				client->Message(Chat::System, DZADD_INVITE_WARNING, m_expedition_name.c_str());
				warned = true;
			}

			auto time_remaining = lockout.GetDaysHoursMinutesRemaining();
			client->Message(
				Chat::System, DZADD_INVITE_WARNING_TIMER,
				lockout.GetEventName().c_str(),
				time_remaining.days.c_str(),
				time_remaining.hours.c_str(),
				time_remaining.mins.c_str()
			);
		}
	}

	auto outapp = CreateInvitePacket(inviter_name, swap_remove_name);
	client->QueuePacket(outapp.get());
}

void Expedition::SendLeaderMessage(
	Client* leader_client, uint16_t chat_type, uint32_t string_id, const std::initializer_list<std::string>& args)
{
	Client::SendCrossZoneMessageString(leader_client, m_leader.name, chat_type, string_id, args);
}

bool Expedition::ProcessAddConflicts(Client* leader_client, Client* add_client, bool swapping)
{
	if (!add_client) // a null leader_client is handled by SendLeaderMessage fallback
	{
		return true;
	}

	bool has_conflict = false;

	if (m_dynamiczone.IsCurrentZoneDzInstance())
	{
		SendLeaderMessage(leader_client, Chat::Red, DZADD_LEAVE_ZONE_FIRST, { add_client->GetName() });
		has_conflict = true;
	}

	auto expedition_id = add_client->GetExpeditionID();
	if (expedition_id)
	{
		auto string_id = (expedition_id == GetID()) ? DZADD_ALREADY_PART : DZADD_ALREADY_ASSIGNED;
		SendLeaderMessage(leader_client, Chat::Red, string_id, { add_client->GetName() });
		has_conflict = true;
	}

	// check any extra event lockouts for this expedition that the client has and expedition doesn't
	auto client_lockouts = add_client->GetExpeditionLockouts(m_expedition_name);
	for (const auto& client_lockout : client_lockouts)
	{
		if (client_lockout.IsReplayTimer())
		{
			// client with a replay lockout is allowed only if the replay timer was from this expedition
			if (client_lockout.GetExpeditionUUID() != GetUUID())
			{
				has_conflict = true;

				auto time_remaining = client_lockout.GetDaysHoursMinutesRemaining();
				SendLeaderMessage(leader_client, Chat::Red, DZADD_REPLAY_TIMER, {
					add_client->GetName(),
					time_remaining.days,
					time_remaining.hours,
					time_remaining.mins
				});
			}
		}
		else
		{
			bool is_missing_lockout = (m_lockouts.find(client_lockout.GetEventName()) == m_lockouts.end());
			if (is_missing_lockout)
			{
				has_conflict = true;

				auto time_remaining = client_lockout.GetDaysHoursMinutesRemaining();
				SendLeaderMessage(leader_client, Chat::Red, DZADD_EVENT_TIMER, {
					add_client->GetName(),
					client_lockout.GetEventName(),
					time_remaining.days,
					time_remaining.hours,
					time_remaining.mins,
					client_lockout.GetEventName()
				});
			}
		}
	}

	// member swapping integrity is handled by invite response
	if (!swapping)
	{
		auto member_count = ExpeditionDatabase::GetMemberCount(m_id);
		if (member_count == 0)
		{
			has_conflict = true;
		}
		else if (member_count >= m_max_players)
		{
			SendLeaderMessage(leader_client, Chat::Red, DZADD_EXCEED_MAX, { fmt::format_int(m_max_players).str() });
			has_conflict = true;
		}
	}

	auto invite_id = add_client->GetPendingExpeditionInviteID();
	if (invite_id)
	{
		auto string_id = (invite_id == GetID()) ? DZADD_PENDING : DZADD_PENDING_OTHER;
		SendLeaderMessage(leader_client, Chat::Red, string_id, { add_client->GetName() });
		has_conflict = true;
	}

	return has_conflict;
}

void Expedition::DzInviteResponse(Client* add_client, bool accepted, const std::string& swap_remove_name)
{
	if (!add_client)
	{
		return;
	}

	LogExpeditionsModerate(
		"Invite response by [{}] accepted [{}] swap_name [{}]",
		add_client->GetName(), accepted, swap_remove_name
	);

	// a null leader_client is handled by SendLeaderMessage fallbacks
	// note current leader receives invite reply messages (if leader changed)
	Client* leader_client = entity_list.GetClientByCharID(m_leader.char_id);

	if (!accepted)
	{
		SendLeaderMessage(leader_client, Chat::Red, EXPEDITION_INVITE_DECLINED, { add_client->GetName() });
		return;
	}

	bool was_swap_invite = !swap_remove_name.empty();
	bool has_conflicts = m_is_locked;

	if (m_is_locked)
	{
		SendLeaderMessage(leader_client, Chat::Red, DZADD_NOT_ALLOWING);
	}
	else
	{
		has_conflicts = ProcessAddConflicts(leader_client, add_client, was_swap_invite);
	}

	// error if swapping and character was already removed before the accept
	if (was_swap_invite)
	{
		auto swap_member = GetMemberData(swap_remove_name);
		if (!swap_member.IsValid() || !ExpeditionDatabase::HasMember(m_id, swap_member.char_id))
		{
			has_conflicts = true;
		}
	}

	if (has_conflicts)
	{
		SendLeaderMessage(leader_client, Chat::Red, EXPEDITION_INVITE_ERROR, { add_client->GetName() });
	}
	else
	{
		SendLeaderMessage(leader_client, Chat::Yellow, EXPEDITION_INVITE_ACCEPTED, { add_client->GetName() });

		// replay timers are optionally added to new members on join with fresh expire time
		if (m_add_replay_on_join)
		{
			auto replay_lockout = m_lockouts.find(DZ_REPLAY_TIMER_NAME);
			if (replay_lockout != m_lockouts.end() &&
			    replay_lockout->second.IsFromExpedition(m_uuid) &&
			    !add_client->HasExpeditionLockout(m_expedition_name, DZ_REPLAY_TIMER_NAME))
			{
				ExpeditionLockoutTimer replay_timer = replay_lockout->second; // copy
				replay_timer.Reset();
				add_client->AddExpeditionLockout(replay_timer, true);
			}
		}

		if (was_swap_invite)
		{
			SwapMember(add_client, swap_remove_name);
		}
		else
		{
			AddMember(add_client->GetName(), add_client->CharacterID());
		}
	}
}

bool Expedition::ConfirmLeaderCommand(Client* requester)
{
	if (!requester)
	{
		return false;
	}

	if (!m_leader.IsValid())
	{
		requester->MessageString(Chat::Red, UNABLE_RETRIEVE_LEADER); // unconfirmed message
		return false;
	}

	if (m_leader.char_id != requester->CharacterID())
	{
		requester->MessageString(Chat::System, EXPEDITION_NOT_LEADER, m_leader.name.c_str());
		return false;
	}

	return true;
}

void Expedition::TryAddClient(
	Client* add_client, const std::string& inviter_name, const std::string& swap_remove_name,
	Client* leader_client)
{
	if (!add_client)
	{
		return;
	}

	LogExpeditionsModerate(
		"Add player request for expedition [{}] by inviter [{}] add name [{}] swap name [{}]",
		m_id, inviter_name, add_client->GetName(), swap_remove_name
	);

	// null leader client handled by ProcessAddConflicts/SendLeaderMessage fallbacks
	if (!leader_client)
	{
		leader_client = entity_list.GetClientByName(inviter_name.c_str());
	}

	bool has_conflicts = ProcessAddConflicts(leader_client, add_client, !swap_remove_name.empty());
	if (!has_conflicts)
	{
		// live uses the original unsanitized input string in invite messages
		uint32_t string_id = swap_remove_name.empty() ? DZADD_INVITE : DZSWAP_INVITE;
		SendLeaderMessage(leader_client, Chat::Yellow, string_id, { add_client->GetName() });
		SendClientExpeditionInvite(add_client, inviter_name.c_str(), swap_remove_name);
	}
	else if (swap_remove_name.empty()) // swap command doesn't result in this message
	{
		SendLeaderMessage(leader_client, Chat::Red, DZADD_INVITE_FAIL, { add_client->GetName() });
	}
}

void Expedition::DzAddPlayer(
	Client* requester, const std::string& add_char_name, const std::string& swap_remove_name)
{
	if (!requester || !ConfirmLeaderCommand(requester))
	{
		return;
	}

	bool invite_failed = false;

	if (m_is_locked)
	{
		requester->MessageString(Chat::Red, DZADD_NOT_ALLOWING);
		invite_failed = true;
	}
	else if (add_char_name.empty())
	{
		requester->MessageString(Chat::Red, DZADD_NOT_ONLINE, add_char_name.c_str());
		invite_failed = true;
	}
	else
	{
		auto member_data = GetMemberData(add_char_name);
		if (member_data.IsValid())
		{
			// live prioritizes offline message before already a member message
			if (member_data.status == ExpeditionMemberStatus::Offline)
			{
				requester->MessageString(Chat::Red, DZADD_NOT_ONLINE, add_char_name.c_str());
			}
			else
			{
				requester->MessageString(Chat::Red, DZADD_ALREADY_PART, add_char_name.c_str());
			}
			invite_failed = true;
		}
	}

	if (invite_failed)
	{
		requester->MessageString(Chat::Red, DZADD_INVITE_FAIL, FormatName(add_char_name).c_str());
		return;
	}

	Client* add_client = entity_list.GetClientByName(add_char_name.c_str());
	if (add_client)
	{
		// client is online in this zone
		TryAddClient(add_client, requester->GetName(), swap_remove_name, requester);
	}
	else
	{
		// forward to world to check if client is online and perform cross-zone invite
		SendWorldAddPlayerInvite(requester->GetName(), swap_remove_name, FormatName(add_char_name));
	}
}

void Expedition::DzAddPlayerContinue(
	std::string inviter_name, std::string add_name, std::string swap_remove_name)
{
	// continuing expedition invite from leader in another zone
	Client* add_client = entity_list.GetClientByName(add_name.c_str());
	if (add_client)
	{
		TryAddClient(add_client, inviter_name, swap_remove_name);
	}
}

void Expedition::DzMakeLeader(Client* requester, std::string new_leader_name)
{
	if (!requester || !ConfirmLeaderCommand(requester))
	{
		return;
	}

	if (new_leader_name.empty())
	{
		requester->MessageString(Chat::Red, DZMAKELEADER_NOT_ONLINE, new_leader_name.c_str());
		return;
	}

	// leader can only be changed by world
	SendWorldMakeLeaderRequest(requester->CharacterID(), FormatName(new_leader_name));
}

void Expedition::DzRemovePlayer(Client* requester, std::string char_name)
{
	if (!requester || !ConfirmLeaderCommand(requester))
	{
		return;
	}

	// live only seems to enforce min_players for requesting expeditions, no need to check here
	bool removed = RemoveMember(char_name);
	if (!removed)
	{
		requester->MessageString(Chat::Red, EXPEDITION_NOT_MEMBER, FormatName(char_name).c_str());
	}
	else
	{
		requester->MessageString(Chat::Yellow, EXPEDITION_REMOVED, FormatName(char_name).c_str(), m_expedition_name.c_str());
	}
}

void Expedition::DzQuit(Client* requester)
{
	if (requester)
	{
		RemoveMember(requester->GetName());
	}
}

void Expedition::DzSwapPlayer(
	Client* requester, std::string remove_char_name, std::string add_char_name)
{
	if (!requester || !ConfirmLeaderCommand(requester))
	{
		return;
	}

	if (remove_char_name.empty() || !HasMember(remove_char_name))
	{
		requester->MessageString(Chat::Red, DZSWAP_CANNOT_REMOVE, FormatName(remove_char_name).c_str());
		return;
	}

	DzAddPlayer(requester, add_char_name, remove_char_name);
}

void Expedition::DzPlayerList(Client* requester)
{
	if (requester)
	{
		requester->MessageString(Chat::Yellow, EXPEDITION_LEADER, m_leader.name.c_str());

		std::string member_names;
		for (const auto& member : m_members)
		{
			fmt::format_to(std::back_inserter(member_names), "{}, ", member.name);
		}

		if (member_names.size() > 1)
		{
			member_names.erase(member_names.length() - 2); // trailing comma and space
		}

		requester->MessageString(Chat::Yellow, EXPEDITION_MEMBERS, member_names.c_str());
	}
}

void Expedition::DzKickPlayers(Client* requester)
{
	if (!requester || !ConfirmLeaderCommand(requester))
	{
		return;
	}

	RemoveAllMembers();
	requester->MessageString(Chat::Red, EXPEDITION_REMOVED, KICKPLAYERS_EVERYONE, m_expedition_name.c_str());
}

void Expedition::SetLocked(
	bool lock_expedition, ExpeditionLockMessage lock_msg, bool update_db, uint32_t msg_color)
{
	m_is_locked = lock_expedition;

	if (m_is_locked && lock_msg != ExpeditionLockMessage::None && m_dynamiczone.IsCurrentZoneDzInstance())
	{
		auto msg = (lock_msg == ExpeditionLockMessage::Close) ? LOCK_CLOSE : LOCK_BEGIN;
		for (const auto& client_iter : entity_list.GetClientList())
		{
			if (client_iter.second)
			{
				client_iter.second->Message(msg_color, msg);
			}
		}
	}

	if (update_db)
	{
		ExpeditionDatabase::UpdateLockState(m_id, lock_expedition);
		SendWorldSettingChanged(ServerOP_ExpeditionLockState, m_is_locked);
	}
}

void Expedition::ProcessLeaderChanged(uint32_t new_leader_id)
{
	auto new_leader = GetMemberData(new_leader_id);
	if (!new_leader.IsValid())
	{
		LogExpeditions("Processed invalid new leader id [{}] for expedition [{}]", new_leader_id, m_id);
		return;
	}

	LogExpeditionsModerate("Replaced [{}] leader [{}] with [{}]", m_id, m_leader.name, new_leader.name);

	m_leader = new_leader;

	// update each client's expedition window in this zone
	auto outapp_leader = CreateLeaderNamePacket();
	for (const auto& member : m_members)
	{
		Client* member_client = entity_list.GetClientByCharID(member.char_id);
		if (member_client)
		{
			member_client->QueuePacket(outapp_leader.get());

			if (member.char_id == new_leader_id && RuleB(Expedition, AlwaysNotifyNewLeaderOnChange))
			{
				member_client->MessageString(Chat::Yellow, DZMAKELEADER_YOU);
			}
		}
	}
}

void Expedition::ProcessMakeLeader(Client* old_leader_client, Client* new_leader_client,
	const std::string& new_leader_name, bool is_success, bool is_online)
{
	if (old_leader_client)
	{
		// success flag is set by world to indicate new leader set to an online member
		if (is_success)
		{
			old_leader_client->MessageString(Chat::Yellow, DZMAKELEADER_NAME, new_leader_name.c_str());
		}
		else if (!is_online)
		{
			old_leader_client->MessageString(Chat::Red, DZMAKELEADER_NOT_ONLINE, new_leader_name.c_str());
		}
		else
		{
			old_leader_client->MessageString(Chat::Red, EXPEDITION_NOT_MEMBER, new_leader_name.c_str());
		}
	}

	if (is_success && new_leader_client && !RuleB(Expedition, AlwaysNotifyNewLeaderOnChange))
	{
		new_leader_client->MessageString(Chat::Yellow, DZMAKELEADER_YOU);
	}
}

void Expedition::ProcessMemberAdded(const std::string& char_name, uint32_t added_char_id)
{
	// adds the member to this expedition and notifies both leader and new member
	Client* leader_client = entity_list.GetClientByCharID(m_leader.char_id);
	if (leader_client)
	{
		leader_client->MessageString(Chat::Yellow, EXPEDITION_MEMBER_ADDED, char_name.c_str(), m_expedition_name.c_str());
	}

	AddInternalMember(char_name, added_char_id, ExpeditionMemberStatus::Online);

	Client* member_client = entity_list.GetClientByCharID(added_char_id);
	if (member_client)
	{
		member_client->SetExpeditionID(GetID());
		member_client->SendDzCompassUpdate();
		SendClientExpeditionInfo(member_client);
		member_client->MessageString(Chat::Yellow, EXPEDITION_MEMBER_ADDED, char_name.c_str(), m_expedition_name.c_str());
	}

	SendNewMemberAddedToZoneMembers(char_name);
}

void Expedition::ProcessMemberRemoved(const std::string& removed_char_name, uint32_t removed_char_id)
{
	if (m_members.empty())
	{
		return;
	}

	auto outapp_member_name = CreateMemberListNamePacket(removed_char_name, true);

	for (auto it = m_members.begin(); it != m_members.end();)
	{
		bool is_removed = (it->name == removed_char_name);

		Client* member_client = entity_list.GetClientByCharID(it->char_id);
		if (member_client)
		{
			// all members receive the removed player name packet
			member_client->QueuePacket(outapp_member_name.get());

			if (is_removed)
			{
				// live doesn't clear expedition info on clients removed while inside dz.
				// it instead let's the dz kick timer do it even if character zones out
				// before it triggers. for simplicity we'll always clear immediately
				member_client->SetExpeditionID(0);
				member_client->SendDzCompassUpdate();
				member_client->QueuePacket(CreateInfoPacket(true).get());
				member_client->MessageString(Chat::Yellow, EXPEDITION_REMOVED,
					it->name.c_str(), m_expedition_name.c_str());
			}
		}

		it = is_removed ? m_members.erase(it) : it + 1;
	}

	LogExpeditionsDetail(
		"Processed member [{}] ({}) removal from [{}], cache member count: [{}]",
		removed_char_name, removed_char_id, m_id, m_members.size()
	);
}

void Expedition::ProcessLockoutDuration(
	const ExpeditionLockoutTimer& lockout, int seconds, bool members_only)
{
	if (!members_only)
	{
		auto it = m_lockouts.find(lockout.GetEventName());
		if (it != m_lockouts.end())
		{
			it->second.AddLockoutTime(seconds);
		}
		else
		{
			m_lockouts[lockout.GetEventName()] = lockout;
		}
	}

	for (const auto& member : m_members)
	{
		Client* member_client = entity_list.GetClientByCharID(member.char_id);
		if (member_client)
		{
			member_client->AddExpeditionLockoutDuration(m_expedition_name,
				lockout.GetEventName(), seconds, m_uuid);
		}
	}

	if (m_dynamiczone.IsCurrentZoneDzInstance())
	{
		AddLockoutDurationClients(lockout, seconds, GetID());
	}
}

void Expedition::AddLockoutDurationClients(
	const ExpeditionLockoutTimer& lockout, int seconds, uint32_t exclude_id)
{
	std::vector<ExpeditionMember> lockout_clients;
	for (const auto& client_iter : entity_list.GetClientList())
	{
		Client* client = client_iter.second;
		if (client && (exclude_id == 0 || client->GetExpeditionID() != exclude_id))
		{
			lockout_clients.emplace_back(client->CharacterID(), client->GetName());
			client->AddExpeditionLockoutDuration(m_expedition_name,
				lockout.GetEventName(), seconds, m_uuid);
		}
	}

	if (!lockout_clients.empty())
	{
		int modified_seconds = static_cast<int>(seconds * RuleR(Expedition, LockoutDurationMultiplier));
		ExpeditionDatabase::AddLockoutDuration(lockout_clients, lockout, modified_seconds);
	}
}

void Expedition::ProcessLockoutUpdate(
	const ExpeditionLockoutTimer& lockout, bool remove, bool members_only)
{
	if (!members_only)
	{
		if (!remove)
		{
			m_lockouts[lockout.GetEventName()] = lockout;
		}
		else
		{
			m_lockouts.erase(lockout.GetEventName());
		}
	}

	for (const auto& member : m_members)
	{
		Client* member_client = entity_list.GetClientByCharID(member.char_id);
		if (member_client)
		{
			if (!remove)
			{
				member_client->AddExpeditionLockout(lockout);
			}
			else
			{
				member_client->RemoveExpeditionLockout(m_expedition_name, lockout.GetEventName());
			}
		}
	}

	// if this is the expedition's dz instance, all clients inside the zone need
	// to receive added lockouts. this is done on live to avoid exploits where
	// members leave the expedition but haven't been kicked from zone yet
	if (!remove && m_dynamiczone.IsCurrentZoneDzInstance())
	{
		AddLockoutClients(lockout, GetID());
	}
}

void Expedition::AddLockoutClients(
	const ExpeditionLockoutTimer& lockout, uint32_t exclude_expedition_id)
{
	std::vector<ExpeditionMember> lockout_clients;
	for (const auto& client_iter : entity_list.GetClientList())
	{
		Client* client = client_iter.second;
		if (client && (exclude_expedition_id == 0 || client->GetExpeditionID() != exclude_expedition_id))
		{
			lockout_clients.emplace_back(client->CharacterID(), client->GetName());
			client->AddExpeditionLockout(lockout);
		}
	}

	if (!lockout_clients.empty())
	{
		ExpeditionDatabase::InsertMembersLockout(lockout_clients, lockout);
	}
}

void Expedition::SendNewMemberAddedToZoneMembers(const std::string& added_name)
{
	// live only sends MemberListName when members are added from a swap, otherwise
	// it sends expedition info (unnecessary) and the full member list
	// we send a full member list update for both cases since MemberListName adds as
	// "unknown" status (either due to unknown packet fields or future client change)
	auto outapp_members = CreateMemberListPacket(false);

	for (const auto& member : m_members)
	{
		if (member.name != added_name) // new member already updated
		{
			Client* member_client = entity_list.GetClientByCharID(member.char_id);
			if (member_client)
			{
				member_client->QueuePacket(outapp_members.get());
			}
		}
	}
}

void Expedition::SendUpdatesToZoneMembers(bool clear, bool message_on_clear)
{
	if (!m_members.empty())
	{
		auto outapp_info = CreateInfoPacket(clear);
		auto outapp_members = CreateMemberListPacket(clear);

		for (const auto& member : m_members)
		{
			Client* member_client = entity_list.GetClientByCharID(member.char_id);
			if (member_client)
			{
				member_client->SetExpeditionID(clear ? 0 : GetID());
				member_client->SendDzCompassUpdate();
				member_client->QueuePacket(outapp_info.get());
				member_client->QueuePacket(outapp_members.get());
				member_client->SendExpeditionLockoutTimers();
				if (clear && message_on_clear)
				{
					member_client->MessageString(Chat::Yellow, EXPEDITION_REMOVED,
						member_client->GetName(), m_expedition_name.c_str());
				}
			}
		}
	}
}

void Expedition::SendClientExpeditionInfo(Client* client)
{
	if (client)
	{
		client->QueuePacket(CreateInfoPacket().get());
		client->QueuePacket(CreateMemberListPacket().get());
	}
}

void Expedition::SendWorldPendingInvite(const ExpeditionInvite& invite, const std::string& add_name)
{
	LogExpeditions(
		"Character [{}] saving pending invite from [{}] to expedition [{}] in world",
		add_name, invite.inviter_name, invite.expedition_id
	);

	SendWorldAddPlayerInvite(invite.inviter_name, invite.swap_remove_name, add_name, true);
}

std::unique_ptr<EQApplicationPacket> Expedition::CreateExpireWarningPacket(uint32_t minutes_remaining)
{
	uint32_t outsize = sizeof(ExpeditionExpireWarning);
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzExpeditionEndsWarning, outsize);
	auto buf = reinterpret_cast<ExpeditionExpireWarning*>(outapp->pBuffer);
	buf->minutes_remaining = minutes_remaining;
	return outapp;
}

std::unique_ptr<EQApplicationPacket> Expedition::CreateInfoPacket(bool clear)
{
	uint32_t outsize = sizeof(ExpeditionInfo_Struct);
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzExpeditionInfo, outsize);
	auto info = reinterpret_cast<ExpeditionInfo_Struct*>(outapp->pBuffer);
	if (!clear)
	{
		info->assigned = true;
		strn0cpy(info->expedition_name, m_expedition_name.c_str(), sizeof(info->expedition_name));
		strn0cpy(info->leader_name, m_leader.name.c_str(), sizeof(info->leader_name));
		info->max_players = m_max_players;
	}
	return outapp;
}

std::unique_ptr<EQApplicationPacket> Expedition::CreateInvitePacket(
	const std::string& inviter_name, const std::string& swap_remove_name)
{
	uint32_t outsize = sizeof(ExpeditionInvite_Struct);
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzExpeditionInvite, outsize);
	auto outbuf = reinterpret_cast<ExpeditionInvite_Struct*>(outapp->pBuffer);
	strn0cpy(outbuf->inviter_name, inviter_name.c_str(), sizeof(outbuf->inviter_name));
	strn0cpy(outbuf->expedition_name, m_expedition_name.c_str(), sizeof(outbuf->expedition_name));
	strn0cpy(outbuf->swap_name, swap_remove_name.c_str(), sizeof(outbuf->swap_name));
	outbuf->swapping = !swap_remove_name.empty();
	outbuf->dz_zone_id = m_dynamiczone.GetZoneID();
	outbuf->dz_instance_id = m_dynamiczone.GetInstanceID();
	return outapp;
}

std::unique_ptr<EQApplicationPacket> Expedition::CreateMemberListPacket(bool clear)
{
	uint32_t member_count = clear ? 0 : static_cast<uint32_t>(m_members.size());
	uint32_t member_entries_size = sizeof(ExpeditionMemberEntry_Struct) * member_count;
	uint32_t outsize = sizeof(ExpeditionMemberList_Struct) + member_entries_size;
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzMemberList, outsize);
	auto buf = reinterpret_cast<ExpeditionMemberList_Struct*>(outapp->pBuffer);

	buf->member_count = member_count;

	if (!clear)
	{
		for (auto i = 0; i < m_members.size(); ++i)
		{
			strn0cpy(buf->members[i].name, m_members[i].name.c_str(), sizeof(buf->members[i].name));
			buf->members[i].expedition_status = static_cast<uint8_t>(m_members[i].status);
		}
	}

	return outapp;
}

std::unique_ptr<EQApplicationPacket> Expedition::CreateMemberListNamePacket(
	const std::string& name, bool remove_name)
{
	uint32_t outsize = sizeof(ExpeditionMemberListName_Struct);
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzMemberListName, outsize);
	auto buf = reinterpret_cast<ExpeditionMemberListName_Struct*>(outapp->pBuffer);
	buf->add_name = !remove_name;
	strn0cpy(buf->name, name.c_str(), sizeof(buf->name));
	return outapp;
}

std::unique_ptr<EQApplicationPacket> Expedition::CreateMemberListStatusPacket(
	const std::string& name, ExpeditionMemberStatus status)
{
	// member list status uses member list struct with a single entry
	uint32_t outsize = sizeof(ExpeditionMemberList_Struct) + sizeof(ExpeditionMemberEntry_Struct);
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzMemberListStatus, outsize);
	auto buf = reinterpret_cast<ExpeditionMemberList_Struct*>(outapp->pBuffer);
	buf->member_count = 1;

	auto entry = reinterpret_cast<ExpeditionMemberEntry_Struct*>(buf->members);
	strn0cpy(entry->name, name.c_str(), sizeof(entry->name));
	entry->expedition_status = static_cast<uint8_t>(status);

	return outapp;
}

std::unique_ptr<EQApplicationPacket> Expedition::CreateLeaderNamePacket()
{
	uint32_t outsize = sizeof(ExpeditionSetLeaderName_Struct);
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzSetLeaderName, outsize);
	auto buf = reinterpret_cast<ExpeditionSetLeaderName_Struct*>(outapp->pBuffer);
	strn0cpy(buf->leader_name, m_leader.name.c_str(), sizeof(buf->leader_name));
	return outapp;
}

void Expedition::SendWorldExpeditionUpdate(uint16_t server_opcode)
{
	uint32_t pack_size = sizeof(ServerExpeditionID_Struct);
	auto pack = std::make_unique<ServerPacket>(server_opcode, pack_size);
	auto buf = reinterpret_cast<ServerExpeditionID_Struct*>(pack->pBuffer);
	buf->expedition_id = GetID();
	buf->sender_zone_id = zone ? zone->GetZoneID() : 0;
	buf->sender_instance_id = zone ? zone->GetInstanceID() : 0;
	worldserver.SendPacket(pack.get());
}

void Expedition::SendWorldAddPlayerInvite(
	const std::string& inviter_name, const std::string& swap_remove_name, const std::string& add_name, bool pending)
{
	auto server_opcode = pending ? ServerOP_ExpeditionSaveInvite : ServerOP_ExpeditionDzAddPlayer;
	uint32_t pack_size = sizeof(ServerDzCommand_Struct);
	auto pack = std::make_unique<ServerPacket>(server_opcode, pack_size);
	auto buf = reinterpret_cast<ServerDzCommand_Struct*>(pack->pBuffer);
	buf->expedition_id = GetID();
	buf->is_char_online = false;
	strn0cpy(buf->requester_name, inviter_name.c_str(), sizeof(buf->requester_name));
	strn0cpy(buf->target_name, add_name.c_str(), sizeof(buf->target_name));
	strn0cpy(buf->remove_name, swap_remove_name.c_str(), sizeof(buf->remove_name));
	worldserver.SendPacket(pack.get());
}

void Expedition::SendWorldLockoutDuration(
	const ExpeditionLockoutTimer& lockout, int seconds, bool members_only)
{
	uint32_t pack_size = sizeof(ServerExpeditionLockout_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_ExpeditionLockoutDuration, pack_size);
	auto buf = reinterpret_cast<ServerExpeditionLockout_Struct*>(pack->pBuffer);
	buf->expedition_id = GetID();
	buf->expire_time = lockout.GetExpireTime();
	buf->duration = lockout.GetDuration();
	buf->sender_zone_id = zone ? zone->GetZoneID() : 0;
	buf->sender_instance_id = zone ? zone->GetInstanceID() : 0;
	buf->members_only = members_only;
	buf->seconds_adjust = seconds;
	strn0cpy(buf->event_name, lockout.GetEventName().c_str(), sizeof(buf->event_name));
	worldserver.SendPacket(pack.get());
}

void Expedition::SendWorldLockoutUpdate(
	const ExpeditionLockoutTimer& lockout, bool remove, bool members_only)
{
	uint32_t pack_size = sizeof(ServerExpeditionLockout_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_ExpeditionLockout, pack_size);
	auto buf = reinterpret_cast<ServerExpeditionLockout_Struct*>(pack->pBuffer);
	buf->expedition_id = GetID();
	buf->expire_time = lockout.GetExpireTime();
	buf->duration = lockout.GetDuration();
	buf->sender_zone_id = zone ? zone->GetZoneID() : 0;
	buf->sender_instance_id = zone ? zone->GetInstanceID() : 0;
	buf->remove = remove;
	buf->members_only = members_only;
	strn0cpy(buf->event_name, lockout.GetEventName().c_str(), sizeof(buf->event_name));
	worldserver.SendPacket(pack.get());
}

void Expedition::SendWorldMakeLeaderRequest(uint32_t requester_id, const std::string& new_leader_name)
{
	uint32_t pack_size = sizeof(ServerDzCommandMakeLeader_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_ExpeditionDzMakeLeader, pack_size);
	auto buf = reinterpret_cast<ServerDzCommandMakeLeader_Struct*>(pack->pBuffer);
	buf->expedition_id = GetID();
	buf->requester_id = requester_id;
	strn0cpy(buf->new_leader_name, new_leader_name.c_str(), sizeof(buf->new_leader_name));
	worldserver.SendPacket(pack.get());
}

void Expedition::SendWorldMemberChanged(const std::string& char_name, uint32_t char_id, bool remove)
{
	// notify other zones of added or removed member
	uint32_t pack_size = sizeof(ServerExpeditionMemberChange_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_ExpeditionMemberChange, pack_size);
	auto buf = reinterpret_cast<ServerExpeditionMemberChange_Struct*>(pack->pBuffer);
	buf->expedition_id = GetID();
	buf->sender_zone_id = zone ? zone->GetZoneID() : 0;
	buf->sender_instance_id = zone ? zone->GetInstanceID() : 0;
	buf->removed = remove;
	buf->char_id = char_id;
	strn0cpy(buf->char_name, char_name.c_str(), sizeof(buf->char_name));
	worldserver.SendPacket(pack.get());
}

void Expedition::SendWorldMemberStatus(uint32_t character_id, ExpeditionMemberStatus status)
{
	uint32_t pack_size = sizeof(ServerExpeditionMemberStatus_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_ExpeditionMemberStatus, pack_size);
	auto buf = reinterpret_cast<ServerExpeditionMemberStatus_Struct*>(pack->pBuffer);
	buf->expedition_id = GetID();
	buf->sender_zone_id = zone ? zone->GetZoneID() : 0;
	buf->sender_instance_id = zone ? zone->GetInstanceID() : 0;
	buf->status = static_cast<uint8_t>(status);
	buf->character_id = character_id;
	worldserver.SendPacket(pack.get());
}

void Expedition::SendWorldDzLocationUpdate(uint16_t server_opcode, const DynamicZoneLocation& location)
{
	uint32_t pack_size = sizeof(ServerDzLocation_Struct);
	auto pack = std::make_unique<ServerPacket>(server_opcode, pack_size);
	auto buf = reinterpret_cast<ServerDzLocation_Struct*>(pack->pBuffer);
	buf->owner_id = GetID();
	buf->dz_zone_id = m_dynamiczone.GetZoneID();
	buf->dz_instance_id = m_dynamiczone.GetInstanceID();
	buf->sender_zone_id = zone ? zone->GetZoneID() : 0;
	buf->sender_instance_id = zone ? zone->GetInstanceID() : 0;
	buf->zone_id = location.zone_id;
	buf->x = location.x;
	buf->y = location.y;
	buf->z = location.z;
	buf->heading = location.heading;
	worldserver.SendPacket(pack.get());
}

void Expedition::SendWorldMemberSwapped(
	const std::string& remove_char_name, uint32_t remove_char_id, const std::string& add_char_name, uint32_t add_char_id)
{
	uint32_t pack_size = sizeof(ServerExpeditionMemberSwap_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_ExpeditionMemberSwap, pack_size);
	auto buf = reinterpret_cast<ServerExpeditionMemberSwap_Struct*>(pack->pBuffer);
	buf->expedition_id = GetID();
	buf->sender_zone_id = zone ? zone->GetZoneID() : 0;
	buf->sender_instance_id = zone ? zone->GetInstanceID() : 0;
	buf->add_char_id = add_char_id;
	buf->remove_char_id = remove_char_id;
	strn0cpy(buf->add_char_name, add_char_name.c_str(), sizeof(buf->add_char_name));
	strn0cpy(buf->remove_char_name, remove_char_name.c_str(), sizeof(buf->remove_char_name));
	worldserver.SendPacket(pack.get());
}

void Expedition::SendWorldSettingChanged(uint16_t server_opcode, bool setting_value)
{
	uint32_t pack_size = sizeof(ServerExpeditionSetting_Struct);
	auto pack = std::make_unique<ServerPacket>(server_opcode, pack_size);
	auto buf = reinterpret_cast<ServerExpeditionSetting_Struct*>(pack->pBuffer);
	buf->expedition_id = GetID();
	buf->sender_zone_id = zone ? zone->GetZoneID() : 0;
	buf->sender_instance_id = zone ? zone->GetInstanceID() : 0;
	buf->enabled = setting_value;
	worldserver.SendPacket(pack.get());
}

void Expedition::SendWorldGetOnlineMembers(
	const std::vector<std::pair<uint32_t, uint32_t>>& expedition_character_ids)
{
	// request online status of characters
	uint32_t count = static_cast<uint32_t>(expedition_character_ids.size());
	uint32_t entries_size = sizeof(ServerExpeditionCharacterEntry_Struct) * count;
	uint32_t pack_size = sizeof(ServerExpeditionCharacters_Struct) + entries_size;
	auto pack = std::make_unique<ServerPacket>(ServerOP_ExpeditionGetOnlineMembers, pack_size);
	auto buf = reinterpret_cast<ServerExpeditionCharacters_Struct*>(pack->pBuffer);
	buf->sender_zone_id = zone ? zone->GetZoneID() : 0;
	buf->sender_instance_id = zone ? zone->GetInstanceID() : 0;
	buf->count = count;
	for (uint32_t i = 0; i < buf->count; ++i)
	{
		buf->entries[i].expedition_id = expedition_character_ids[i].first;
		buf->entries[i].character_id = expedition_character_ids[i].second;
		buf->entries[i].character_zone_id = 0;
		buf->entries[i].character_instance_id = 0;
		buf->entries[i].character_online = false;
	}
	worldserver.SendPacket(pack.get());
}

void Expedition::SendWorldCharacterLockout(
	uint32_t character_id, const ExpeditionLockoutTimer& lockout, bool remove)
{
	uint32_t pack_size = sizeof(ServerExpeditionCharacterLockout_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_ExpeditionCharacterLockout, pack_size);
	auto buf = reinterpret_cast<ServerExpeditionCharacterLockout_Struct*>(pack->pBuffer);
	buf->remove = remove;
	buf->character_id = character_id;
	buf->expire_time = lockout.GetExpireTime();
	buf->duration = lockout.GetDuration();
	strn0cpy(buf->uuid, lockout.GetExpeditionUUID().c_str(), sizeof(buf->uuid));
	strn0cpy(buf->expedition_name, lockout.GetExpeditionName().c_str(), sizeof(buf->expedition_name));
	strn0cpy(buf->event_name, lockout.GetEventName().c_str(), sizeof(buf->event_name));
	worldserver.SendPacket(pack.get());
}

void Expedition::SendWorldSetSecondsRemaining(uint32_t seconds_remaining)
{
	uint32_t pack_size = sizeof(ServerExpeditionUpdateDuration_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_ExpeditionSecondsRemaining, pack_size);
	auto buf = reinterpret_cast<ServerExpeditionUpdateDuration_Struct*>(pack->pBuffer);
	buf->expedition_id = GetID();
	buf->new_duration_seconds = seconds_remaining;
	worldserver.SendPacket(pack.get());
}

void Expedition::AddLockoutByCharacterID(
	uint32_t character_id, const std::string& expedition_name, const std::string& event_name,
	uint32_t seconds, const std::string& uuid)
{
	if (character_id)
	{
		auto lockout = ExpeditionLockoutTimer::CreateLockout(expedition_name, event_name, seconds, uuid);
		ExpeditionDatabase::InsertCharacterLockouts(character_id, { lockout });
		SendWorldCharacterLockout(character_id, lockout, false);
	}
}

void Expedition::AddLockoutByCharacterName(
	const std::string& character_name, const std::string& expedition_name, const std::string& event_name,
	uint32_t seconds, const std::string& uuid)
{
	if (!character_name.empty())
	{
		uint32_t character_id = database.GetCharacterID(character_name.c_str());
		AddLockoutByCharacterID(character_id, expedition_name, event_name, seconds, uuid);
	}
}

bool Expedition::HasLockoutByCharacterID(
	uint32_t character_id, const std::string& expedition_name, const std::string& event_name)
{
	auto lockouts = Expedition::GetExpeditionLockoutsByCharacterID(character_id);
	return std::any_of(lockouts.begin(), lockouts.end(), [&](const ExpeditionLockoutTimer& lockout) {
		return lockout.IsSameLockout(expedition_name, event_name);
	});
}

bool Expedition::HasLockoutByCharacterName(
	const std::string& character_name, const std::string& expedition_name, const std::string& event_name)
{
	if (!character_name.empty())
	{
		uint32_t character_id = database.GetCharacterID(character_name.c_str());
		return HasLockoutByCharacterID(character_id, expedition_name, event_name);
	}
	return false;
}

void Expedition::RemoveLockoutsByCharacterID(
	uint32_t character_id, const std::string& expedition_name, const std::string& event_name)
{
	if (character_id)
	{
		if (!event_name.empty())
		{
			ExpeditionDatabase::DeleteCharacterLockout(character_id, expedition_name, event_name);
		}
		else if (!expedition_name.empty())
		{
			ExpeditionDatabase::DeleteAllCharacterLockouts(character_id, expedition_name);
		}
		else
		{
			ExpeditionDatabase::DeleteAllCharacterLockouts(character_id);
		}

		ExpeditionLockoutTimer lockout{{}, expedition_name, event_name, 0, 0};
		SendWorldCharacterLockout(character_id, lockout, true);
	}
}

void Expedition::RemoveLockoutsByCharacterName(
	const std::string& character_name, const std::string& expedition_name, const std::string& event_name)
{
	if (!character_name.empty())
	{
		uint32_t character_id = database.GetCharacterID(character_name.c_str());
		RemoveLockoutsByCharacterID(character_id, expedition_name, event_name);
	}
}

void Expedition::HandleWorldMessage(ServerPacket* pack)
{
	switch (pack->opcode)
	{
	case ServerOP_ExpeditionCreate:
	{
		auto buf = reinterpret_cast<ServerExpeditionID_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->sender_zone_id, buf->sender_instance_id))
		{
			Expedition::CacheFromDatabase(buf->expedition_id);
		}
		break;
	}
	case ServerOP_ExpeditionDeleted:
	{
		// sent by world when it deletes expired or empty expeditions
		auto buf = reinterpret_cast<ServerExpeditionID_Struct*>(pack->pBuffer);
		auto expedition = Expedition::FindCachedExpeditionByID(buf->expedition_id);
		if (zone && expedition)
		{
			expedition->SendUpdatesToZoneMembers(true, false); // any members silently removed

			LogExpeditionsModerate("Deleting expedition [{}] from zone cache", buf->expedition_id);
			zone->expedition_cache.erase(buf->expedition_id);
		}
		break;
	}
	case ServerOP_ExpeditionMembersRemoved:
	{
		auto buf = reinterpret_cast<ServerExpeditionID_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->sender_zone_id, buf->sender_instance_id))
		{
			auto expedition = Expedition::FindCachedExpeditionByID(buf->expedition_id);
			if (expedition)
			{
				expedition->SendUpdatesToZoneMembers(true);
				expedition->m_members.clear();
			}
		}
		break;
	}
	case ServerOP_ExpeditionLeaderChanged:
	{
		auto buf = reinterpret_cast<ServerExpeditionLeaderID_Struct*>(pack->pBuffer);
		auto expedition = Expedition::FindCachedExpeditionByID(buf->expedition_id);
		if (expedition)
		{
			expedition->ProcessLeaderChanged(buf->leader_id);
		}
		break;
	}
	case ServerOP_ExpeditionLockout:
	case ServerOP_ExpeditionLockoutDuration:
	{
		auto buf = reinterpret_cast<ServerExpeditionLockout_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->sender_zone_id, buf->sender_instance_id))
		{
			auto expedition = Expedition::FindCachedExpeditionByID(buf->expedition_id);
			if (expedition)
			{
				ExpeditionLockoutTimer lockout{ expedition->GetUUID(), expedition->GetName(),
					buf->event_name, buf->expire_time, buf->duration };

				if (pack->opcode == ServerOP_ExpeditionLockout)
				{
					expedition->ProcessLockoutUpdate(lockout, buf->remove, buf->members_only);
				}
				else if (pack->opcode == ServerOP_ExpeditionLockoutDuration)
				{
					expedition->ProcessLockoutDuration(lockout, buf->seconds_adjust, buf->members_only);
				}
			}
		}
		break;
	}
	case ServerOP_ExpeditionMemberChange:
	{
		auto buf = reinterpret_cast<ServerExpeditionMemberChange_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->sender_zone_id, buf->sender_instance_id))
		{
			auto expedition = Expedition::FindCachedExpeditionByID(buf->expedition_id);
			if (expedition)
			{
				if (buf->removed)
				{
					expedition->ProcessMemberRemoved(buf->char_name, buf->char_id);
				}
				else
				{
					expedition->ProcessMemberAdded(buf->char_name, buf->char_id);
				}
			}
		}
		break;
	}
	case ServerOP_ExpeditionMemberSwap:
	{
		auto buf = reinterpret_cast<ServerExpeditionMemberSwap_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->sender_zone_id, buf->sender_instance_id))
		{
			auto expedition = Expedition::FindCachedExpeditionByID(buf->expedition_id);
			if (expedition)
			{
				expedition->ProcessMemberRemoved(buf->remove_char_name, buf->remove_char_id);
				expedition->ProcessMemberAdded(buf->add_char_name, buf->add_char_id);
			}
		}
		break;
	}
	case ServerOP_ExpeditionMemberStatus:
	{
		auto buf = reinterpret_cast<ServerExpeditionMemberStatus_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->sender_zone_id, buf->sender_instance_id))
		{
			auto expedition = Expedition::FindCachedExpeditionByID(buf->expedition_id);
			if (expedition)
			{
				expedition->UpdateMemberStatus(buf->character_id, static_cast<ExpeditionMemberStatus>(buf->status));
			}
		}
		break;
	}
	case ServerOP_ExpeditionLockState:
	{
		auto buf = reinterpret_cast<ServerExpeditionLockState_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->sender_zone_id, buf->sender_instance_id))
		{
			auto expedition = Expedition::FindCachedExpeditionByID(buf->expedition_id);
			if (expedition)
			{
				expedition->SetLocked(buf->enabled, static_cast<ExpeditionLockMessage>(buf->lock_msg));
			}
		}
		break;
	}
	case ServerOP_ExpeditionReplayOnJoin:
	{
		auto buf = reinterpret_cast<ServerExpeditionSetting_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->sender_zone_id, buf->sender_instance_id))
		{
			auto expedition = Expedition::FindCachedExpeditionByID(buf->expedition_id);
			if (expedition)
			{
				expedition->SetReplayLockoutOnMemberJoin(buf->enabled);
			}
		}
		break;
	}
	case ServerOP_ExpeditionGetOnlineMembers:
	{
		// reply from world for online member statuses request (for multiple expeditions)
		auto buf = reinterpret_cast<ServerExpeditionCharacters_Struct*>(pack->pBuffer);
		for (uint32_t i = 0; i < buf->count; ++i)
		{
			auto member = reinterpret_cast<ServerExpeditionCharacterEntry_Struct*>(&buf->entries[i]);
			auto expedition = Expedition::FindCachedExpeditionByID(member->expedition_id);
			if (expedition)
			{
				auto is_online = member->character_online;
				auto status = is_online ? ExpeditionMemberStatus::Online : ExpeditionMemberStatus::Offline;
				if (is_online && expedition->GetDynamicZone().IsInstanceID(member->character_instance_id))
				{
					status = ExpeditionMemberStatus::InDynamicZone;
				}
				expedition->UpdateMemberStatus(member->character_id, status);
			}
		}
		break;
	}
	case ServerOP_ExpeditionDzAddPlayer:
	{
		auto buf = reinterpret_cast<ServerDzCommand_Struct*>(pack->pBuffer);
		if (buf->is_char_online)
		{
			auto expedition = Expedition::FindCachedExpeditionByID(buf->expedition_id);
			if (expedition)
			{
				expedition->DzAddPlayerContinue(buf->requester_name, buf->target_name, buf->remove_name);
			}
		}
		else
		{
			Client* leader = entity_list.GetClientByName(buf->requester_name);
			if (leader)
			{
				std::string target_name = FormatName(buf->target_name);
				leader->MessageString(Chat::Red, DZADD_NOT_ONLINE, target_name.c_str());
				leader->MessageString(Chat::Red, DZADD_INVITE_FAIL, target_name.c_str());
			}
		}
		break;
	}
	case ServerOP_ExpeditionDzMakeLeader:
	{
		auto buf = reinterpret_cast<ServerDzCommandMakeLeader_Struct*>(pack->pBuffer);
		auto expedition = Expedition::FindCachedExpeditionByID(buf->expedition_id);
		if (expedition)
		{
			auto old_leader_client = entity_list.GetClientByCharID(buf->requester_id);
			auto new_leader_client = entity_list.GetClientByName(buf->new_leader_name);
			expedition->ProcessMakeLeader(old_leader_client, new_leader_client,
				buf->new_leader_name, buf->is_success, buf->is_online);
		}
		break;
	}
	case ServerOP_ExpeditionDzCompass:
	case ServerOP_ExpeditionDzSafeReturn:
	case ServerOP_ExpeditionDzZoneIn:
	{
		auto buf = reinterpret_cast<ServerDzLocation_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->sender_zone_id, buf->sender_instance_id))
		{
			auto expedition = Expedition::FindCachedExpeditionByID(buf->owner_id);
			if (expedition)
			{
				if (pack->opcode == ServerOP_ExpeditionDzCompass)
				{
					expedition->SetDzCompass(buf->zone_id, buf->x, buf->y, buf->z, false);
				}
				else if (pack->opcode == ServerOP_ExpeditionDzSafeReturn)
				{
					expedition->SetDzSafeReturn(buf->zone_id, buf->x, buf->y, buf->z, buf->heading, false);
				}
				else if (pack->opcode == ServerOP_ExpeditionDzZoneIn)
				{
					expedition->SetDzZoneInLocation(buf->x, buf->y, buf->z, buf->heading, false);
				}
			}
		}
		break;
	}
	case ServerOP_ExpeditionCharacterLockout:
	{
		auto buf = reinterpret_cast<ServerExpeditionCharacterLockout_Struct*>(pack->pBuffer);
		Client* client = entity_list.GetClientByCharID(buf->character_id);
		if (client)
		{
			if (!buf->remove)
			{
				client->AddExpeditionLockout(ExpeditionLockoutTimer{
					buf->uuid, buf->expedition_name, buf->event_name, buf->expire_time, buf->duration
				});
			}
			else if (buf->event_name[0] != '\0')
			{
				client->RemoveExpeditionLockout(buf->expedition_name, buf->event_name);
			}
			else
			{
				client->RemoveAllExpeditionLockouts(buf->expedition_name);
			}
		}
		break;
	}
	case ServerOP_ExpeditionDzDuration:
	{
		auto buf = reinterpret_cast<ServerExpeditionUpdateDuration_Struct*>(pack->pBuffer);
		auto expedition = Expedition::FindCachedExpeditionByID(buf->expedition_id);
		if (expedition)
		{
			expedition->UpdateDzDuration(buf->new_duration_seconds);
		}
		break;
	}
	case ServerOP_ExpeditionExpireWarning:
	{
		auto buf = reinterpret_cast<ServerExpeditionExpireWarning_Struct*>(pack->pBuffer);
		auto expedition = Expedition::FindCachedExpeditionByID(buf->expedition_id);
		if (expedition)
		{
			expedition->SendMembersExpireWarning(buf->minutes_remaining);
		}
		break;
	}
	}
}

void Expedition::SetDzCompass(uint32_t zone_id, float x, float y, float z, bool update_db)
{
	DynamicZoneLocation location{ zone_id, x, y, z, 0.0f };
	m_dynamiczone.SetCompass(location, update_db);

	for (const auto& member : m_members)
	{
		Client* member_client = entity_list.GetClientByCharID(member.char_id);
		if (member_client)
		{
			member_client->SendDzCompassUpdate();
		}
	}

	if (update_db)
	{
		SendWorldDzLocationUpdate(ServerOP_ExpeditionDzCompass, location);
	}
}

void Expedition::SetDzCompass(const std::string& zone_name, float x, float y, float z, bool update_db)
{
	auto zone_id = ZoneID(zone_name.c_str());
	SetDzCompass(zone_id, x, y, z, update_db);
}

void Expedition::SetDzSafeReturn(uint32_t zone_id, float x, float y, float z, float heading, bool update_db)
{
	DynamicZoneLocation location{ zone_id, x, y, z, heading };

	m_dynamiczone.SetSafeReturn(location, update_db);

	if (update_db)
	{
		SendWorldDzLocationUpdate(ServerOP_ExpeditionDzSafeReturn, location);
	}
}

void Expedition::SetDzSafeReturn(const std::string& zone_name, float x, float y, float z, float heading, bool update_db)
{
	auto zone_id = ZoneID(zone_name.c_str());
	SetDzSafeReturn(zone_id, x, y, z, heading, update_db);
}

void Expedition::SetDzSecondsRemaining(uint32_t seconds_remaining)
{
	SendWorldSetSecondsRemaining(seconds_remaining); // async
}

void Expedition::SetDzZoneInLocation(float x, float y, float z, float heading, bool update_db)
{
	DynamicZoneLocation location{ 0, x, y, z, heading };

	m_dynamiczone.SetZoneInLocation(location, update_db);

	if (update_db)
	{
		SendWorldDzLocationUpdate(ServerOP_ExpeditionDzZoneIn, location);
	}
}

bool Expedition::CanClientLootCorpse(Client* client, uint32_t npc_type_id, uint32_t spawn_id)
{
	if (client && m_dynamiczone.IsCurrentZoneDzInstance())
	{
		// entity id takes priority, falls back to checking by npc type if not set
		std::string event_name = GetLootEventBySpawnID(spawn_id);
		if (event_name.empty())
		{
			event_name = GetLootEventByNPCTypeID(npc_type_id);
		}

		if (!event_name.empty())
		{
			auto client_lockout = client->GetExpeditionLockout(GetName(), event_name);
			if (!client_lockout || client_lockout->GetExpeditionUUID() != GetUUID())
			{
				// client lockout not received in this expedition, prevent looting
				LogExpeditions(
					"Character [{}] denied looting npc [{}] spawn [{}] for lockout event [{}]",
					client->CharacterID(), npc_type_id, spawn_id, event_name
				);
				return false;
			}
		}
	}

	return true;
}

void Expedition::SetLootEventByNPCTypeID(uint32_t npc_type_id, const std::string& event_name)
{
	if (npc_type_id && m_dynamiczone.IsCurrentZoneDzInstance())
	{
		LogExpeditions("Setting loot event [{}] for npc type id [{}]", event_name, npc_type_id);
		m_npc_loot_events[npc_type_id] = event_name;
	}
}

void Expedition::SetLootEventBySpawnID(uint32_t spawn_id, const std::string& event_name)
{
	if (spawn_id && m_dynamiczone.IsCurrentZoneDzInstance())
	{
		LogExpeditions("Setting loot event [{}] for entity id [{}]", event_name, spawn_id);
		m_spawn_loot_events[spawn_id] = event_name;
	}
}

std::string Expedition::GetLootEventByNPCTypeID(uint32_t npc_type_id)
{
	std::string event_name;

	if (npc_type_id && m_dynamiczone.IsCurrentZoneDzInstance())
	{
		auto it = m_npc_loot_events.find(npc_type_id);
		if (it != m_npc_loot_events.end())
		{
			event_name = it->second;
		}
	}

	return event_name;
}

std::string Expedition::GetLootEventBySpawnID(uint32_t spawn_id)
{
	std::string event_name;

	if (spawn_id && m_dynamiczone.IsCurrentZoneDzInstance())
	{
		auto it = m_spawn_loot_events.find(spawn_id);
		if (it != m_spawn_loot_events.end())
		{
			event_name = it->second;
		}
	}

	return event_name;
}

std::vector<ExpeditionLockoutTimer> Expedition::GetExpeditionLockoutsByCharacterID(uint32_t character_id)
{
	std::vector<ExpeditionLockoutTimer> lockouts;
	if (character_id == 0)
	{
		return lockouts;
	}

	auto client = entity_list.GetClientByCharID(character_id);
	if (client)
	{
		lockouts = client->GetExpeditionLockouts();
	}
	else
	{
		lockouts = ExpeditionDatabase::LoadCharacterLockouts(character_id);
	}

	return lockouts;
}

void Expedition::SendMembersExpireWarning(uint32_t minutes_remaining)
{
	// expeditions warn members in all zones not just the dz
	auto outapp = CreateExpireWarningPacket(minutes_remaining);
	for (const auto& member : m_members)
	{
		Client* member_client = entity_list.GetClientByCharID(member.char_id);
		if (member_client)
		{
			member_client->QueuePacket(outapp.get());
			member_client->MessageString(Chat::Yellow, EXPEDITION_MIN_REMAIN,
				fmt::format_int(minutes_remaining).c_str());
		}
	}
}

void Expedition::SyncCharacterLockouts(
	uint32_t character_id, std::vector<ExpeditionLockoutTimer>& client_lockouts)
{
	// adds missing event lockouts to client for this expedition and updates
	// client timers that are both shorter and from another expedition
	BenchTimer benchmark;

	bool modified = false;

	for (const auto& lockout_iter : m_lockouts)
	{
		const ExpeditionLockoutTimer& lockout = lockout_iter.second;
		if (lockout.IsReplayTimer() || lockout.IsExpired() || lockout.GetExpeditionUUID() != m_uuid)
		{
			continue;
		}

		auto client_lockout_iter = std::find_if(client_lockouts.begin(), client_lockouts.end(),
			[&](const ExpeditionLockoutTimer& client_lockout) {
				return client_lockout.IsSameLockout(lockout);
			});

		if (client_lockout_iter == client_lockouts.end())
		{
			modified = true;
			client_lockouts.emplace_back(lockout); // insert missing
		}
		else if (client_lockout_iter->GetSecondsRemaining() < lockout.GetSecondsRemaining() &&
		         client_lockout_iter->GetExpeditionUUID() != m_uuid)
		{
			// only update lockout timer not uuid so loot event apis still work
			modified = true;
			client_lockout_iter->SetDuration(lockout.GetDuration());
			client_lockout_iter->SetExpireTime(lockout.GetExpireTime());
		}
	}

	if (modified)
	{
		ExpeditionDatabase::InsertCharacterLockouts(character_id, client_lockouts);
	}

	LogExpeditionsDetail("Syncing character lockouts with expedition took [{}] s", benchmark.elapsed());
}
