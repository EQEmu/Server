#include "dynamic_zone_manager.h"
#include "dynamic_zone.h"
#include "worlddb.h"
#include "zonelist.h"
#include "zoneserver.h"
#include "../common/rulesys.h"
#include "../common/repositories/expeditions_repository.h"
#include "../common/repositories/expedition_lockouts_repository.h"

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

		DynamicZoneMembersRepository::DeleteWhere(database,
			fmt::format("dynamic_zone_id IN ({})", fmt::join(dz_ids, ",")));
		DynamicZonesRepository::DeleteWhere(database,
			fmt::format("id IN ({})", fmt::join(dz_ids, ",")));
	}
}

DynamicZone* DynamicZoneManager::CreateNew(
	DynamicZone& dz_request, const std::vector<DynamicZoneMember>& members)
{
	// this creates a new dz instance and saves it to both db and cache
	uint32_t dz_id = dz_request.Create();
	if (dz_id == 0)
	{
		LogDynamicZones("Failed to create dynamic zone for zone [{}]", dz_request.GetZoneID());
		return nullptr;
	}

	auto dz = std::make_unique<DynamicZone>(dz_request);
	if (!members.empty())
	{
		dz->SaveMembers(members);
		dz->CacheMemberStatuses();
	}

	LogDynamicZones("Created new dz [{}] for zone [{}]", dz_id, dz_request.GetZoneID());

	auto pack = dz->CreateServerDzCreatePacket(0, 0);
	zoneserver_list.SendPacket(pack.get());

	auto inserted = dynamic_zone_cache.emplace(dz_id, std::move(dz));
	return inserted.first->second.get();
}

void DynamicZoneManager::CacheNewDynamicZone(ServerPacket* pack)
{
	auto buf = reinterpret_cast<ServerDzCreateSerialized_Struct*>(pack->pBuffer);

	auto new_dz = std::make_unique<DynamicZone>();
	new_dz->LoadSerializedDzPacket(buf->cereal_data, buf->cereal_size);
	new_dz->CacheMemberStatuses();

	// reserialize with member statuses cached before forwarding (restore origin zone)
	auto repack = new_dz->CreateServerDzCreatePacket(buf->origin_zone_id, buf->origin_instance_id);

	uint32_t dz_id = new_dz->GetID();
	dynamic_zone_cache.emplace(dz_id, std::move(new_dz));
	LogDynamicZones("Cached new dynamic zone [{}]", dz_id);

	zoneserver_list.SendPacket(repack.get());
}

void DynamicZoneManager::CacheAllFromDatabase()
{
	BenchTimer bench;

	auto dynamic_zones = DynamicZonesRepository::AllWithInstanceNotExpired(database);
	auto dynamic_zone_members = DynamicZoneMembersRepository::GetAllWithNames(database);

	dynamic_zone_cache.clear();
	dynamic_zone_cache.reserve(dynamic_zones.size());

	for (auto& entry : dynamic_zones)
	{
		uint32_t dz_id = entry.id;
		auto dz = std::make_unique<DynamicZone>(std::move(entry));

		for (auto& member : dynamic_zone_members)
		{
			if (member.dynamic_zone_id == dz_id)
			{
				dz->AddMemberFromRepositoryResult(std::move(member));
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

		// need to look up expedition ids until lockouts are moved to dynamic zones
		std::vector<uint32_t> expedition_ids;
		auto expeditions = ExpeditionsRepository::GetWhere(database,
			fmt::format("dynamic_zone_id IN ({})", fmt::join(dynamic_zone_ids, ",")));

		if (!expeditions.empty())
		{
			for (const auto& expedition : expeditions)
			{
				expedition_ids.emplace_back(expedition.id);
			}
			ExpeditionLockoutsRepository::DeleteWhere(database,
				fmt::format("expedition_id IN ({})", fmt::join(expedition_ids, ",")));
		}

		ExpeditionsRepository::DeleteWhere(database,
			fmt::format("dynamic_zone_id IN ({})", fmt::join(dynamic_zone_ids, ",")));
		DynamicZoneMembersRepository::RemoveAllMembers(database, dynamic_zone_ids);
		DynamicZonesRepository::DeleteWhere(database,
			fmt::format("id IN ({})", fmt::join(dynamic_zone_ids, ",")));
	}
}
