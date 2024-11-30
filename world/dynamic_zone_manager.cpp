#include "dynamic_zone_manager.h"
#include "dynamic_zone.h"
#include "cliententry.h"
#include "clientlist.h"
#include "worlddb.h"
#include "zonelist.h"
#include "zoneserver.h"
#include "../common/rulesys.h"
#include "../common/repositories/dynamic_zone_lockouts_repository.h"

extern ClientList client_list;
extern ZSList zoneserver_list;

DynamicZoneManager dynamic_zone_manager;

DynamicZoneManager::DynamicZoneManager() :
	m_process_throttle_timer{ static_cast<uint32_t>(RuleI(DynamicZone, WorldProcessRate)) }
{
}

void DynamicZoneManager::PurgeExpiredDynamicZones()
{
	// purge when no members, instance is expired, or instance doesn't exist.
	// this prevents characters remaining members of dzs that expired while
	// server was offline but delayed instance purging hasn't cleaned yet
	auto dz_ids = DynamicZonesRepository::GetStaleIDs(database);

	if (!dz_ids.empty())
	{
		LogDynamicZones("Purging [{}] dynamic zone(s)", dz_ids.size());

		DynamicZoneLockoutsRepository::DeleteWhere(database,
			fmt::format("dynamic_zone_id IN ({})", fmt::join(dz_ids, ",")));
		DynamicZoneMembersRepository::DeleteWhere(database,
			fmt::format("dynamic_zone_id IN ({})", Strings::Join(dz_ids, ",")));
		DynamicZonesRepository::DeleteWhere(database,
			fmt::format("id IN ({})", Strings::Join(dz_ids, ",")));
	}
}

DynamicZone* DynamicZoneManager::TryCreate(DynamicZone& dz_request, const std::vector<DynamicZoneMember>& members)
{
	// this creates a new dz instance and saves it to both db and cache
	uint32_t dz_id = dz_request.Create();
	if (dz_id == 0)
	{
		LogDynamicZones("Failed to create dynamic zone for zone [{}]", dz_request.GetZoneID());
		return nullptr;
	}

	auto dz = std::make_unique<DynamicZone>(dz_request);
	dz->SaveMembers(members);
	dz->CacheMemberStatuses();

	LogDynamicZones("Created new dz [{}] for zone [{}]", dz_id, dz_request.GetZoneID());

	auto pack = dz->CreateServerPacket(0, 0);
	zoneserver_list.SendPacket(pack.get());

	auto inserted = dynamic_zone_cache.emplace(dz_id, std::move(dz));
	return inserted.first->second.get();
}

void DynamicZoneManager::CacheNewDynamicZone(ServerPacket* pack)
{
	auto buf = reinterpret_cast<ServerDzCreate_Struct*>(pack->pBuffer);

	auto new_dz = std::make_unique<DynamicZone>();
	new_dz->Unserialize({ buf->cereal_data, buf->cereal_size });
	new_dz->CacheMemberStatuses();

	// reserialize with member statuses cached before forwarding (restore origin zone)
	auto repack = new_dz->CreateServerPacket(buf->origin_zone_id, buf->origin_instance_id);

	dynamic_zone_cache.emplace(buf->dz_id, std::move(new_dz));
	LogDynamicZones("Cached new dynamic zone [{}]", buf->dz_id);

	zoneserver_list.SendPacket(repack.get());
}

void DynamicZoneManager::CacheAllFromDatabase()
{
	BenchTimer bench;

	auto dzs = DynamicZonesRepository::AllWithInstanceNotExpired(database);
	auto members = DynamicZoneMembersRepository::AllWithNames(database);
	auto lockouts = DynamicZoneLockoutsRepository::All(database);

	dynamic_zone_cache.clear();
	dynamic_zone_cache.reserve(dzs.size());

	for (auto& entry : dzs)
	{
		uint32_t dz_id = entry.id;
		auto dz = std::make_unique<DynamicZone>(std::move(entry));

		for (auto& member : members)
		{
			if (member.dynamic_zone_id == dz_id)
			{
				dz->AddMemberFromRepositoryResult(std::move(member));
			}
		}

		for (auto& lockout : lockouts)
		{
			if (lockout.dynamic_zone_id == dz->GetID())
			{
				dz->m_lockouts.emplace_back(dz->GetName(), std::move(lockout));
			}
		}

		// note leader status won't be updated here until leader is set by owning system (expeditions)
		dz->CacheMemberStatuses();

		dynamic_zone_cache.emplace(dz_id, std::move(dz));
	}

	LogDynamicZones("Caching [{}] dynamic zone(s) took [{}s]", dynamic_zone_cache.size(), bench.elapsed());
}

void DynamicZoneManager::Process()
{
	if (!m_process_throttle_timer.Check())
	{
		return;
	}

	std::vector<uint32_t> dynamic_zone_ids;

	for (const auto& dz_iter : dynamic_zone_cache)
	{
		DynamicZone* dz = dz_iter.second.get();

		// dynamic zone is not deleted until its zone has no clients to prevent exploits
		// clients should be removed by zone-based kick timers if expired but not empty
		DynamicZoneStatus status = dz->Process();
		if (status == DynamicZoneStatus::ExpiredEmpty)
		{
			LogDynamicZones("[{}] expired with [{}] members, notifying zones and deleting", dz->GetID(), dz->GetMemberCount());
			dynamic_zone_ids.emplace_back(dz->GetID());
			dz->SendZonesDynamicZoneDeleted(); // delete dz from zone caches
		}
	}

	if (!dynamic_zone_ids.empty())
	{
		for (const auto& dz_id : dynamic_zone_ids)
		{
			dynamic_zone_cache.erase(dz_id);
		}

		DynamicZoneLockoutsRepository::DeleteWhere(database,
			fmt::format("dynamic_zone_id IN ({})", fmt::join(dynamic_zone_ids, ",")));
		DynamicZoneMembersRepository::DeleteWhere(database,
			fmt::format("dynamic_zone_id IN ({})", fmt::join(dynamic_zone_ids, ",")));
		DynamicZonesRepository::DeleteWhere(database,
			fmt::format("id IN ({})", Strings::Join(dynamic_zone_ids, ",")));
	}
}

void DynamicZoneManager::LoadTemplates()
{
	m_dz_templates.clear();
	auto dz_templates = DynamicZoneTemplatesRepository::All(content_db);
	for (const auto& dz_template : dz_templates)
	{
		m_dz_templates[dz_template.id] = dz_template;
	}
}

void DynamicZoneManager::HandleZoneMessage(ServerPacket* pack)
{
	switch (pack->opcode)
	{
	case ServerOP_DzCreated:
	{
		CacheNewDynamicZone(pack);
		break;
	}
	case ServerOP_DzAddPlayer:
	{
		auto buf = reinterpret_cast<ServerDzCommand_Struct*>(pack->pBuffer);

		ClientListEntry* cle = client_list.FindCharacter(buf->target_name);
		if (cle && cle->Server())
		{
			// continue in the add target's zone
			buf->is_char_online = true;
			cle->Server()->SendPacket(pack);
		}
		else
		{
			// add target not online, return to inviter
			ClientListEntry* inviter_cle = client_list.FindCharacter(buf->requester_name);
			if (inviter_cle && inviter_cle->Server())
			{
				inviter_cle->Server()->SendPacket(pack);
			}
		}
		break;
	}
	case ServerOP_DzSaveInvite:
	{
		auto buf = reinterpret_cast<ServerDzCommand_Struct*>(pack->pBuffer);
		if (ClientListEntry* cle = client_list.FindCharacter(buf->target_name))
		{
			// store packet on cle and re-send it when client requests it
			buf->is_char_online = true;
			pack->opcode = ServerOP_DzAddPlayer;
			cle->SetPendingDzInvite(pack);
		}
		break;
	}
	case ServerOP_DzRequestInvite:
	{
		auto buf = reinterpret_cast<ServerCharacterID_Struct*>(pack->pBuffer);
		if (ClientListEntry* cle = client_list.FindCLEByCharacterID(buf->char_id))
		{
			auto invite_pack = cle->GetPendingDzInvite();
			if (invite_pack && cle->Server())
			{
				cle->Server()->SendPacket(invite_pack.get());
			}
		}
		break;
	}
	case ServerOP_DzMakeLeader:
	{
		auto buf = reinterpret_cast<ServerDzCommandMakeLeader_Struct*>(pack->pBuffer);

		// notify requester (old leader) and new leader of the result
		ZoneServer* new_leader_zs = nullptr;
		ClientListEntry* leader_cle = client_list.FindCharacter(buf->new_leader_name);
		if (leader_cle && leader_cle->Server())
		{
			auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
			if (dz && dz->GetLeaderID() == buf->requester_id)
			{
				buf->is_success = dz->SetNewLeader(leader_cle->CharID());
			}

			buf->is_online = true;
			new_leader_zs = leader_cle->Server();
			new_leader_zs->SendPacket(pack);
		}

		// if old and new leader are in the same zone only send one message
		ClientListEntry* requester_cle = client_list.FindCLEByCharacterID(buf->requester_id);
		if (requester_cle && requester_cle->Server() && requester_cle->Server() != new_leader_zs)
		{
			requester_cle->Server()->SendPacket(pack);
		}
		break;
	}
	case ServerOP_DzSetCompass:
	case ServerOP_DzSetSafeReturn:
	case ServerOP_DzSetZoneIn:
	{
		auto buf = reinterpret_cast<ServerDzLocation_Struct*>(pack->pBuffer);
		if (auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id))
		{
			if (pack->opcode == ServerOP_DzSetCompass)
			{
				dz->SetCompass(buf->zone_id, buf->x, buf->y, buf->z, false);
			}
			else if (pack->opcode == ServerOP_DzSetSafeReturn)
			{
				dz->SetSafeReturn(buf->zone_id, buf->x, buf->y, buf->z, buf->heading, false);
			}
			else if (pack->opcode == ServerOP_DzSetZoneIn)
			{
				dz->SetZoneInLocation(buf->x, buf->y, buf->z, buf->heading, false);
			}
		}
		zoneserver_list.SendPacket(pack);
		break;
	}
	case ServerOP_DzSetSwitchID:
	{
		auto buf = reinterpret_cast<ServerDzSwitchID_Struct*>(pack->pBuffer);
		if (auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id))
		{
			dz->ProcessSetSwitchID(buf->dz_switch_id);
		}
		zoneserver_list.SendPacket(pack);
		break;
	}
	case ServerOP_DzAddRemoveMember:
	{
		auto buf = reinterpret_cast<ServerDzMember_Struct*>(pack->pBuffer);
		if (auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id))
		{
			auto status = static_cast<DynamicZoneMemberStatus>(buf->character_status);
			dz->ProcessMemberAddRemove({ buf->character_id, buf->character_name, status }, buf->removed);
		}
		zoneserver_list.SendPacket(pack);
		break;
	}
	case ServerOP_DzSwapMembers:
	{
		auto buf = reinterpret_cast<ServerDzMemberSwap_Struct*>(pack->pBuffer);
		if (auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id))
		{
			// we add first in world so new member can be chosen if leader is removed
			auto status = static_cast<DynamicZoneMemberStatus>(buf->add_character_status);
			dz->ProcessMemberAddRemove({ buf->add_character_id, buf->add_character_name, status }, false);
			dz->ProcessMemberAddRemove({ buf->remove_character_id, buf->remove_character_name }, true);
		}
		zoneserver_list.SendPacket(pack);
		break;
	}
	case ServerOP_DzRemoveAllMembers:
	{
		auto buf = reinterpret_cast<ServerDzID_Struct*>(pack->pBuffer);
		if (auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id))
		{
			dz->ProcessRemoveAllMembers();
		}
		zoneserver_list.SendPacket(pack);
		break;
	}
	case ServerOP_DzSetSecondsRemaining:
	{
		auto buf = reinterpret_cast<ServerDzSetDuration_Struct*>(pack->pBuffer);
		if (auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id))
		{
			dz->SetSecondsRemaining(buf->seconds);
		}
		break;
	}
	case ServerOP_DzGetMemberStatuses:
	{
		auto buf = reinterpret_cast<ServerDzID_Struct*>(pack->pBuffer);
		if (auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id))
		{
			dz->SendZoneMemberStatuses(buf->sender_zone_id, buf->sender_instance_id);
		}
		break;
	}
	case ServerOP_DzUpdateMemberStatus:
	{
		auto buf = reinterpret_cast<ServerDzMemberStatus_Struct*>(pack->pBuffer);
		if (auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id))
		{
			auto status = static_cast<DynamicZoneMemberStatus>(buf->status);
			dz->ProcessMemberStatusChange(buf->character_id, status);
		}
		zoneserver_list.SendPacket(pack);
		break;
	}
	case ServerOP_DzMovePC:
	{
		auto buf = reinterpret_cast<ServerDzMovePC_Struct*>(pack->pBuffer);
		auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
		if (dz && dz->HasMember(buf->character_id))
		{
			zoneserver_list.SendPacket(buf->sender_zone_id, buf->sender_instance_id, pack);
		}
		break;
	}
	case ServerOP_DzLock:
	{
		auto buf = reinterpret_cast<ServerDzLock_Struct*>(pack->pBuffer);
		if (auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id))
		{
			dz->SetLocked(buf->lock);
		}
		zoneserver_list.SendPacket(pack);
		break;
	}
	case ServerOP_DzReplayOnJoin:
	{
		auto buf = reinterpret_cast<ServerDzBool_Struct*>(pack->pBuffer);
		if (auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id))
		{
			dz->SetReplayOnJoin(buf->enabled);
		}
		zoneserver_list.SendPacket(pack);
		break;
	}
	case ServerOP_DzLockout:
	{
		auto buf = reinterpret_cast<ServerDzLockout_Struct*>(pack->pBuffer);
		if (auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id))
		{
			DzLockout lockout{ dz->GetUUID(), dz->GetName(), buf->event_name, buf->expire_time, buf->duration };
			dz->HandleLockoutUpdate(lockout, buf->remove, buf->members_only);
		}
		zoneserver_list.SendPacket(pack);
		break;
	}
	case ServerOP_DzLockoutDuration:
	{
		auto buf = reinterpret_cast<ServerDzLockout_Struct*>(pack->pBuffer);
		if (auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id))
		{
			DzLockout lockout{ dz->GetUUID(), dz->GetName(), buf->event_name, buf->expire_time, buf->duration };
			dz->HandleLockoutDuration(lockout, buf->seconds, buf->members_only, false);
		}
		zoneserver_list.SendPacket(pack);
		break;
	}
	case ServerOP_DzCharacterLockout:
	{
		auto buf = reinterpret_cast<ServerDzCharacterLockout_Struct*>(pack->pBuffer);
		auto cle = client_list.FindCLEByCharacterID(buf->char_id);
		if (cle && cle->Server())
		{
			cle->Server()->SendPacket(pack);
		}
		break;
	}
	default:
		break;
	};
}
