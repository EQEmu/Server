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
#include "expedition_request.h"
#include "client.h"
#include "string_ids.h"
#include "worldserver.h"
#include "zonedb.h"
#include "../common/eqemu_logsys.h"
#include "../common/expedition_lockout_timer.h"
#include "../common/repositories/dynamic_zone_members_repository.h"
#include "../common/repositories/expedition_lockouts_repository.h"

extern WorldServer worldserver;
extern Zone* zone;

// message string 8271 (not in emu clients)
const char* const DZ_YOU_NOT_ASSIGNED        = "You could not use this command because you are not currently assigned to a dynamic zone.";
// message string 9265 (not in emu clients)
const char* const EXPEDITION_OTHER_BELONGS   = "{} attempted to create an expedition but {} already belongs to one.";
// lockout warnings were added to live in March 11 2020 patch
const char* const DZADD_INVITE_WARNING       = "Warning! You will be given replay timers for the following events if you enter %s:";
const char* const DZADD_INVITE_WARNING_TIMER = "%s - %sD:%sH:%sM";
// various expeditions re-use these strings when locking
constexpr char LOCK_CLOSE[]                  = "Your expedition is nearing its close. You cannot bring any additional people into your expedition at this time.";
constexpr char LOCK_BEGIN[]                  = "The trial has begun. You cannot bring any additional people into your expedition at this time.";

const int32_t Expedition::REPLAY_TIMER_ID = -1;
const int32_t Expedition::EVENT_TIMER_ID  = 1;

Expedition::Expedition(DynamicZone* dz, uint32_t id, uint32_t dz_id) :
	m_dynamic_zone(dz),
	m_id(id),
	m_dynamic_zone_id(dz_id)
{
	assert(m_dynamic_zone != nullptr); // dz must remain valid for lifetime of expedition
}

void Expedition::LoadRepositoryResult(const ExpeditionsRepository::Expeditions& entry)
{
	m_id                 = entry.id;
	m_dynamic_zone_id    = entry.dynamic_zone_id;
	m_add_replay_on_join = entry.add_replay_on_join;
	m_is_locked          = entry.is_locked;
}

void Expedition::RegisterDynamicZoneCallbacks()
{
	uint32_t expedition_id = GetID();

	GetDynamicZone()->RegisterOnClientAddRemove(
		[expedition_id](Client* client, bool removed, bool silent) {
			auto expedition = Expedition::FindCachedExpeditionByID(expedition_id);
			if (expedition) {
				expedition->OnClientAddRemove(client, removed, silent);
			}
		});
}

Expedition* Expedition::TryCreate(Client* requester, DynamicZone& dz_request, bool disable_messages)
{
	if (!requester || !zone)
	{
		return nullptr;
	}

	ExpeditionRequest request{ dz_request, disable_messages };

	// request parses leader, members list, and lockouts while validating
	if (!request.Validate(requester))
	{
		LogExpeditionsModerate("[{}] request by [{}] denied", request.GetExpeditionName(), requester->GetName());
		return nullptr;
	}

	dz_request.SetLeader({ request.GetLeaderID(), request.GetLeaderName(), DynamicZoneMemberStatus::Online });

	auto dz = DynamicZone::CreateNew(dz_request, request.GetMembers());
	if (!dz)
	{
		// live uses this message when trying to enter an instance that isn't ready
		// we can use it as the client error message if instance creation fails
		requester->MessageString(Chat::Red, DZ_PREVENT_ENTERING);
		LogExpeditions("Failed to create a dynamic zone instance for expedition");
		return nullptr;
	}

	// unique expedition ids are created from database via auto-increment column
	auto expedition_id = ExpeditionDatabase::InsertExpedition(dz->GetID());

	if (expedition_id)
	{
		auto expedition = std::make_unique<Expedition>(dz, expedition_id, dz->GetID());

		LogExpeditions(
			"Created [{}] [{}] dz: [{}] instance id: [{}] leader: [{}] minplayers: [{}] maxplayers: [{}]",
			expedition->GetID(),
			expedition->GetName(),
			dz->GetID(),
			dz->GetInstanceID(),
			expedition->GetLeaderName(),
			dz->GetMinPlayers(),
			dz->GetMaxPlayers()
		);

		expedition->SaveLockouts(request);
		expedition->RegisterDynamicZoneCallbacks();

		auto inserted = zone->expedition_cache.emplace(expedition_id, std::move(expedition));

		inserted.first->second->SendWorldExpeditionUpdate(ServerOP_ExpeditionCreate); // cache in other zones
		inserted.first->second->SendLeaderMessage(request.GetLeaderClient(),
			Chat::System, EXPEDITION_AVAILABLE, { request.GetExpeditionName() });

		if (!request.GetNotAllAddedMessage().empty())
		{
			Client::SendCrossZoneMessage(request.GetLeaderClient(), request.GetLeaderName(),
				Chat::System, request.GetNotAllAddedMessage());
		}

		dz->DoAsyncZoneMemberUpdates();

		return inserted.first->second.get();
	}

	return nullptr;
}

void Expedition::CacheExpeditions(
	std::vector<ExpeditionsRepository::Expeditions>&& expedition_entries)
{
	if (!zone)
	{
		return;
	}

	// bulk load expedition internal lockouts before caching
	std::vector<uint32_t> expedition_ids;
	for (const auto& entry : expedition_entries)
	{
		expedition_ids.emplace_back(entry.id);
	}

	// todo: lockouts need be moved to dynamic zone
	auto expedition_lockouts = ExpeditionLockoutsRepository::GetWithTimestamp(database, expedition_ids);

	for (auto& entry : expedition_entries)
	{
		// the expedition's dynamic zone and members should already be in cache
		auto dynamic_zone = DynamicZone::FindDynamicZoneByID(entry.dynamic_zone_id);
		if (!dynamic_zone)
		{
			LogExpeditions("[Warning] Expedition [{}] dz [{}] not found during caching", entry.id, entry.dynamic_zone_id);
			continue;
		}

		auto expedition = std::make_unique<Expedition>(dynamic_zone);
		expedition->LoadRepositoryResult(entry);
		expedition->RegisterDynamicZoneCallbacks();

		for (auto& lockout_entry : expedition_lockouts)
		{
			if (lockout_entry.expedition_id == expedition->GetID())
			{
				ExpeditionLockoutTimer lockout{
					std::move(lockout_entry.from_expedition_uuid),
					expedition->GetName(),
					std::move(lockout_entry.event_name),
					static_cast<uint64_t>(lockout_entry.expire_time),
					static_cast<uint32_t>(lockout_entry.duration)
				};

				std::string event_name = lockout.GetEventName(); // copy for key since we're moving it
				expedition->m_lockouts.emplace(std::move(event_name), std::move(lockout));
			}
		}

		zone->expedition_cache.emplace(entry.id, std::move(expedition));
		dynamic_zone->DoAsyncZoneMemberUpdates();
	}
}

void Expedition::CacheFromDatabase(uint32_t expedition_id)
{
	if (zone && expedition_id != 0)
	{
		BenchTimer benchmark;

		auto expedition = ExpeditionsRepository::GetWhere(database, fmt::format("id = {}", expedition_id));
		CacheExpeditions({ std::move(expedition) });

		LogExpeditions("Caching new expedition [{}] took [{}s]", expedition_id, benchmark.elapsed());
	}
}

bool Expedition::CacheAllFromDatabase()
{
	if (!zone)
	{
		return false;
	}

	BenchTimer benchmark;

	auto expeditions = ExpeditionsRepository::All(database);
	zone->expedition_cache.clear();
	zone->expedition_cache.reserve(expeditions.size());

	CacheExpeditions(std::move(expeditions));

	LogExpeditions("Caching [{}] expedition(s) took [{}s]", zone->expedition_cache.size(), benchmark.elapsed());

	return true;
}

void Expedition::SaveLockouts(ExpeditionRequest& request)
{
	m_lockouts = request.GetLockouts();
	ExpeditionDatabase::InsertLockouts(m_id, m_lockouts);
}

Expedition* Expedition::FindCachedExpeditionByCharacterID(uint32_t character_id)
{
	if (zone)
	{
		for (const auto& expedition : zone->expedition_cache)
		{
			auto expedition_dz = expedition.second->GetDynamicZone();
			if (expedition_dz && expedition_dz->HasMember(character_id))
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
			auto expedition_dz = expedition.second->GetDynamicZone();
			if (expedition_dz && expedition_dz->HasMember(char_name))
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
			if (cached_expedition.second->GetDynamicZoneID() == dz_id)
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
			auto expedition_dz = cached_expedition.second->GetDynamicZone();
			if (expedition_dz && expedition_dz->IsSameDz(zone_id, instance_id))
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
	auto lockout = ExpeditionLockoutTimer::CreateLockout(GetName(),
		event_name, seconds, GetDynamicZone()->GetUUID());
	AddLockout(lockout);
}

void Expedition::AddLockout(const ExpeditionLockoutTimer& lockout, bool members_only)
{
	if (!members_only)
	{
		ExpeditionDatabase::InsertLockout(m_id, lockout);
	}
	ExpeditionDatabase::InsertMembersLockout(GetDynamicZone()->GetMembers(), lockout);

	ProcessLockoutUpdate(lockout, false, members_only);
	SendWorldLockoutUpdate(lockout, false, members_only);
}

void Expedition::AddLockoutDuration(const std::string& event_name, int seconds, bool members_only)
{
	// lockout timers use unsigned durations to define intent but we may need
	// to insert a new lockout while still supporting timer reductions
	auto lockout = ExpeditionLockoutTimer::CreateLockout(GetName(),
		event_name, std::max(0, seconds), GetDynamicZone()->GetUUID());

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
	ExpeditionDatabase::AddLockoutDuration(GetDynamicZone()->GetMembers(), lockout, modified_seconds);

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
		AddLockout({ GetDynamicZone()->GetUUID(), GetName(), event_name, expire_time, seconds }, members_only);
	}
}

void Expedition::RemoveLockout(const std::string& event_name)
{
	ExpeditionDatabase::DeleteLockout(m_id, event_name);
	ExpeditionDatabase::DeleteMembersLockout(GetDynamicZone()->GetMembers(), GetName(), event_name);

	ExpeditionLockoutTimer lockout{GetDynamicZone()->GetUUID(), GetName(), event_name, 0, 0};
	ProcessLockoutUpdate(lockout, true);
	SendWorldLockoutUpdate(lockout, true);
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
		GetLeaderName().c_str(), GetName().c_str());

	// live (as of March 11 2020 patch) sends warnings for lockouts added
	// during current expedition that client would receive on entering dz
	bool warned = false;
	for (const auto& lockout_iter : m_lockouts)
	{
		// live doesn't issue a warning for the dz's replay timer
		const ExpeditionLockoutTimer& lockout = lockout_iter.second;
		if (!lockout.IsReplayTimer() && !lockout.IsExpired() &&
		    lockout.IsFromExpedition(GetDynamicZone()->GetUUID()) &&
		    !client->HasExpeditionLockout(GetName(), lockout.GetEventName()))
		{
			if (!warned)
			{
				client->Message(Chat::System, DZADD_INVITE_WARNING, GetName().c_str());
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
	Client::SendCrossZoneMessageString(leader_client, GetLeaderName(), chat_type, string_id, args);
}

bool Expedition::ProcessAddConflicts(Client* leader_client, Client* add_client, bool swapping)
{
	if (!add_client) // a null leader_client is handled by SendLeaderMessage fallback
	{
		return true;
	}

	bool has_conflict = false;

	if (GetDynamicZone()->IsCurrentZoneDzInstance())
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
	auto client_lockouts = add_client->GetExpeditionLockouts(GetName());
	for (const auto& client_lockout : client_lockouts)
	{
		if (client_lockout.IsReplayTimer())
		{
			// client with a replay lockout is allowed only if the replay timer was from this expedition
			if (client_lockout.GetExpeditionUUID() != GetDynamicZone()->GetUUID())
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
		auto member_count = GetDynamicZone()->GetDatabaseMemberCount();
		if (member_count == 0)
		{
			has_conflict = true;
		}
		else if (member_count >= GetDynamicZone()->GetMaxPlayers())
		{
			SendLeaderMessage(leader_client, Chat::Red, DZADD_EXCEED_MAX, {
				fmt::format_int(GetDynamicZone()->GetMaxPlayers()).str() });
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

	LogExpeditionsModerate("Invite response by [{}] accepted [{}] swap_name [{}]",
		add_client->GetName(), accepted, swap_remove_name);

	// a null leader_client is handled by SendLeaderMessage fallbacks
	// note current leader receives invite reply messages (if leader changed)
	Client* leader_client = entity_list.GetClientByCharID(GetLeaderID());

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
		auto swap_member = GetDynamicZone()->GetMemberData(swap_remove_name);
		if (!swap_member.IsValid() || !GetDynamicZone()->HasDatabaseMember(swap_member.id))
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
			    replay_lockout->second.IsFromExpedition(GetDynamicZone()->GetUUID()) &&
			    !add_client->HasExpeditionLockout(GetName(), DZ_REPLAY_TIMER_NAME))
			{
				ExpeditionLockoutTimer replay_timer = replay_lockout->second; // copy
				replay_timer.Reset();
				add_client->AddExpeditionLockout(replay_timer, true);
			}
		}

		auto status = DynamicZoneMemberStatus::Online;
		DynamicZoneMember add_member{ add_client->CharacterID(), add_client->GetName(), status };

		bool success = false;
		if (was_swap_invite) {
			success = GetDynamicZone()->SwapMember(add_member, swap_remove_name);
		} else {
			success = GetDynamicZone()->AddMember(add_member);
		}

		if (success)
		{
			SendLeaderMessage(leader_client, Chat::Yellow, EXPEDITION_MEMBER_ADDED,
				{ add_client->GetName(), GetName().c_str() });
		}
	}
}

bool Expedition::ConfirmLeaderCommand(Client* requester)
{
	if (!requester)
	{
		return false;
	}

	if (!GetLeader().IsValid())
	{
		requester->MessageString(Chat::Red, UNABLE_RETRIEVE_LEADER); // unconfirmed message
		return false;
	}

	if (GetLeaderID() != requester->CharacterID())
	{
		requester->MessageString(Chat::System, EXPEDITION_NOT_LEADER, GetLeaderName().c_str());
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
		auto member_data = GetDynamicZone()->GetMemberData(add_char_name);
		if (member_data.IsValid())
		{
			// live prioritizes offline message before already a member message
			if (member_data.status == DynamicZoneMemberStatus::Offline)
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
	// note: on live members removed when inside a dz instance remain "temporary"
	// members for kick timer duration and still receive lockouts across zones (unimplemented)
	bool removed = GetDynamicZone()->RemoveMember(char_name);
	if (!removed)
	{
		requester->MessageString(Chat::Red, EXPEDITION_NOT_MEMBER, FormatName(char_name).c_str());
	}
	else
	{
		requester->MessageString(Chat::Yellow, EXPEDITION_REMOVED, FormatName(char_name).c_str(), GetName().c_str());
	}
}

void Expedition::DzQuit(Client* requester)
{
	if (requester)
	{
		GetDynamicZone()->RemoveMember(requester->GetName());
	}
}

void Expedition::DzSwapPlayer(
	Client* requester, std::string remove_char_name, std::string add_char_name)
{
	if (!requester || !ConfirmLeaderCommand(requester))
	{
		return;
	}

	if (remove_char_name.empty() || !GetDynamicZone()->HasMember(remove_char_name))
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
		requester->MessageString(Chat::Yellow, EXPEDITION_LEADER, GetLeaderName().c_str());

		std::string member_names;
		for (const auto& member : GetDynamicZone()->GetMembers())
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

	GetDynamicZone()->RemoveAllMembers();
	requester->MessageString(Chat::Red, EXPEDITION_REMOVED, "Everyone", GetName().c_str());
}

void Expedition::SetLocked(
	bool lock_expedition, ExpeditionLockMessage lock_msg, bool update_db, uint32_t msg_color)
{
	m_is_locked = lock_expedition;

	if (m_is_locked && lock_msg != ExpeditionLockMessage::None && GetDynamicZone()->IsCurrentZoneDzInstance())
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

void Expedition::OnClientAddRemove(Client* client, bool removed, bool silent)
{
	if (client)
	{
		client->SetExpeditionID(removed ? 0 : GetID());

		if (!silent)
		{
			auto string_id = removed ? EXPEDITION_REMOVED : EXPEDITION_MEMBER_ADDED;
			client->MessageString(Chat::Yellow, string_id, client->GetCleanName(), GetName().c_str());
		}
	}
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

	for (const auto& member : GetDynamicZone()->GetMembers())
	{
		Client* member_client = entity_list.GetClientByCharID(member.id);
		if (member_client)
		{
			member_client->AddExpeditionLockoutDuration(GetName(),
				lockout.GetEventName(), seconds, GetDynamicZone()->GetUUID());
		}
	}

	if (GetDynamicZone()->IsCurrentZoneDzInstance())
	{
		AddLockoutDurationClients(lockout, seconds, GetID());
	}
}

void Expedition::AddLockoutDurationClients(
	const ExpeditionLockoutTimer& lockout, int seconds, uint32_t exclude_id)
{
	std::vector<DynamicZoneMember> lockout_clients;
	for (const auto& client_iter : entity_list.GetClientList())
	{
		Client* client = client_iter.second;
		if (client && (exclude_id == 0 || client->GetExpeditionID() != exclude_id))
		{
			lockout_clients.emplace_back(client->CharacterID(), client->GetName());
			client->AddExpeditionLockoutDuration(GetName(),
				lockout.GetEventName(), seconds, GetDynamicZone()->GetUUID());
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

	for (const auto& member : GetDynamicZone()->GetMembers())
	{
		Client* member_client = entity_list.GetClientByCharID(member.id);
		if (member_client)
		{
			if (!remove)
			{
				member_client->AddExpeditionLockout(lockout);
			}
			else
			{
				member_client->RemoveExpeditionLockout(GetName(), lockout.GetEventName());
			}
		}
	}

	// if this is the expedition's dz instance, all clients inside the zone need
	// to receive added lockouts. this is done on live to avoid exploits where
	// members leave the expedition but haven't been kicked from zone yet
	if (!remove && GetDynamicZone()->IsCurrentZoneDzInstance())
	{
		AddLockoutClients(lockout, GetID());
	}
}

void Expedition::AddLockoutClients(
	const ExpeditionLockoutTimer& lockout, uint32_t exclude_expedition_id)
{
	std::vector<DynamicZoneMember> lockout_clients;
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

void Expedition::SendWorldPendingInvite(const ExpeditionInvite& invite, const std::string& add_name)
{
	LogExpeditions(
		"Character [{}] saving pending invite from [{}] to expedition [{}] in world",
		add_name, invite.inviter_name, invite.expedition_id
	);

	SendWorldAddPlayerInvite(invite.inviter_name, invite.swap_remove_name, add_name, true);
}

std::unique_ptr<EQApplicationPacket> Expedition::CreateInvitePacket(
	const std::string& inviter_name, const std::string& swap_remove_name)
{
	uint32_t outsize = sizeof(ExpeditionInvite_Struct);
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzExpeditionInvite, outsize);
	auto outbuf = reinterpret_cast<ExpeditionInvite_Struct*>(outapp->pBuffer);
	strn0cpy(outbuf->inviter_name, inviter_name.c_str(), sizeof(outbuf->inviter_name));
	strn0cpy(outbuf->expedition_name, GetName().c_str(), sizeof(outbuf->expedition_name));
	strn0cpy(outbuf->swap_name, swap_remove_name.c_str(), sizeof(outbuf->swap_name));
	outbuf->swapping = !swap_remove_name.empty();
	outbuf->dz_zone_id = GetDynamicZone()->GetZoneID();
	outbuf->dz_instance_id = GetDynamicZone()->GetInstanceID();
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
	buf->dz_id = GetDynamicZone()->GetID();
	buf->requester_id = requester_id;
	strn0cpy(buf->new_leader_name, new_leader_name.c_str(), sizeof(buf->new_leader_name));
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
	case ServerOP_ExpeditionLockout:
	case ServerOP_ExpeditionLockoutDuration:
	{
		auto buf = reinterpret_cast<ServerExpeditionLockout_Struct*>(pack->pBuffer);
		if (zone && !zone->IsZone(buf->sender_zone_id, buf->sender_instance_id))
		{
			auto expedition = Expedition::FindCachedExpeditionByID(buf->expedition_id);
			if (expedition)
			{
				ExpeditionLockoutTimer lockout{ expedition->GetDynamicZone()->GetUUID(), expedition->GetName(),
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
		auto old_leader_client = entity_list.GetClientByCharID(buf->requester_id);
		auto new_leader_client = entity_list.GetClientByName(buf->new_leader_name);

		if (old_leader_client)
		{
			// success flag is set by world to indicate new leader set to an online member
			if (buf->is_success) {
				old_leader_client->MessageString(Chat::Yellow, DZMAKELEADER_NAME, buf->new_leader_name);
			} else if (!buf->is_online) {
				old_leader_client->MessageString(Chat::Red, DZMAKELEADER_NOT_ONLINE, buf->new_leader_name);
			} else {
				old_leader_client->MessageString(Chat::Red, EXPEDITION_NOT_MEMBER, buf->new_leader_name);
			}
		}

		if (buf->is_success && new_leader_client && !RuleB(Expedition, AlwaysNotifyNewLeaderOnChange))
		{
			new_leader_client->MessageString(Chat::Yellow, DZMAKELEADER_YOU);
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
	}
}

bool Expedition::CanClientLootCorpse(Client* client, uint32_t npc_type_id, uint32_t spawn_id)
{
	if (client && GetDynamicZone()->IsCurrentZoneDzInstance())
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
			if (!client_lockout || client_lockout->GetExpeditionUUID() != GetDynamicZone()->GetUUID())
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
	if (npc_type_id && GetDynamicZone()->IsCurrentZoneDzInstance())
	{
		LogExpeditions("Setting loot event [{}] for npc type id [{}]", event_name, npc_type_id);
		m_npc_loot_events[npc_type_id] = event_name;
	}
}

void Expedition::SetLootEventBySpawnID(uint32_t spawn_id, const std::string& event_name)
{
	if (spawn_id && GetDynamicZone()->IsCurrentZoneDzInstance())
	{
		LogExpeditions("Setting loot event [{}] for entity id [{}]", event_name, spawn_id);
		m_spawn_loot_events[spawn_id] = event_name;
	}
}

std::string Expedition::GetLootEventByNPCTypeID(uint32_t npc_type_id)
{
	std::string event_name;

	if (npc_type_id && GetDynamicZone()->IsCurrentZoneDzInstance())
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

	if (spawn_id && GetDynamicZone()->IsCurrentZoneDzInstance())
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
		if (lockout.IsReplayTimer() || lockout.IsExpired() ||
		    lockout.GetExpeditionUUID() != GetDynamicZone()->GetUUID())
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
		         client_lockout_iter->GetExpeditionUUID() != GetDynamicZone()->GetUUID())
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
