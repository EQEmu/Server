#include <string>
#include "npc.h"
#include "corpse.h"
#include "zone.h"
#include "../common/repositories/zone_state_spawns_repository.h"

inline std::string GetLootSerialized(NPC *npc)
{
	LootStateData ls         = {};
	auto          loot_items = npc->GetLootItems(); // Assuming this returns a list of loot items
	ls.copper   = npc->GetCopper();
	ls.silver   = npc->GetSilver();
	ls.gold     = npc->GetGold();
	ls.platinum = npc->GetPlatinum();
	ls.entries.reserve(loot_items.size());

	for (auto &l: loot_items) {
		ls.entries.emplace_back(
			LootEntryStateData{
				.item_id = l->item_id,
				.lootdrop_id = l->lootdrop_id,
			}
		);
	}

	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		ls.serialize(ar);
	}

	return ss.str();
}

inline std::string GetLootSerialized(Corpse *c)
{
	LootStateData ls         = {};
	auto          loot_items = c->GetLootItems(); // Assuming this returns a list of loot items
	ls.copper   = c->GetCopper();
	ls.silver   = c->GetSilver();
	ls.gold     = c->GetGold();
	ls.platinum = c->GetPlatinum();
	ls.entries.reserve(loot_items.size());

	for (auto &l: loot_items) {
		ls.entries.emplace_back(
			LootEntryStateData{
				.item_id = l->item_id,
				.lootdrop_id = l->lootdrop_id,
			}
		);
	}

	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		ls.serialize(ar);
	}

	return ss.str();
}

void Zone::SaveZoneState()
{
	// spawns
	std::vector<ZoneStateSpawnsRepository::ZoneStateSpawns> spawns = {};
	LinkedListIterator<Spawn2 *>                            iterator(spawn2_list);
	iterator.Reset();
	while (iterator.MoreElements()) {
		Spawn2 *sp = iterator.GetData();
		auto   s   = ZoneStateSpawnsRepository::NewEntity();
		s.zone_id             = GetZoneID();
		s.instance_id         = GetInstanceID();
		s.npc_id              = sp->CurrentNPCID();
		s.spawn2_id           = sp->GetID();
		s.spawngroup_id       = sp->SpawnGroupID();
		s.x                   = sp->GetX();
		s.y                   = sp->GetY();
		s.z                   = sp->GetZ();
		s.heading             = sp->GetHeading();
		s.respawn_time        = sp->RespawnTimer();
		s.variance            = sp->GetVariance();
		s.grid                = sp->GetGrid();
		s.path_when_zone_idle = sp->GetPathWhenZoneIdle() ? 1 : 0;
		s.condition_id        = sp->GetSpawnCondition();
		s.condition_min_value = sp->GetConditionMinValue();
		s.enabled             = sp->Enabled() ? 1 : 0;
		s.anim                = sp->GetAnimation();
		s.created_at          = std::time(nullptr);
		spawns.emplace_back(s);
		iterator.Advance();
	}

	// npcs that are not in the spawn2 list
	for (auto &n: entity_list.GetNPCList()) {

		// update loot data for existing spawns
		for (auto &s: spawns) {
			bool is_same_npc =
					 s.npc_id == n.second->GetNPCTypeID() &&
					 s.spawn2_id == n.second->GetSpawnPointID() &&
					 s.spawngroup_id == n.second->GetSpawnGroupId();
			if (is_same_npc) {
				s.loot_data = GetLootSerialized(n.second);
			}
		}

		if (n.second->GetSpawnGroupId() > 0) {
			continue;
		}

		if (n.second->GetNPCTypeID() < 100) {
			continue;
		}

		if (n.second->HasOwner()) {
			continue;
		}

		auto s = ZoneStateSpawnsRepository::NewEntity();
		s.zone_id             = GetZoneID();
		s.instance_id         = GetInstanceID();
		s.npc_id              = n.second->GetNPCTypeID();
		s.spawn2_id           = 0;
		s.spawngroup_id       = 0;
		s.x                   = n.second->GetX();
		s.y                   = n.second->GetY();
		s.z                   = n.second->GetZ();
		s.heading             = n.second->GetHeading();
		s.respawn_time        = 0;
		s.variance            = 0;
		s.grid                = n.second->GetGrid();
		s.path_when_zone_idle = 0;
		s.condition_id        = 0;
		s.condition_min_value = 0;
		s.enabled             = 1;
		s.anim                = 0;
		s.created_at          = std::time(nullptr);
		s.loot_data           = GetLootSerialized(n.second);

		spawns.emplace_back(s);
	}

	for (auto &n: entity_list.GetCorpseList()) {
		if (!n.second->IsNPCCorpse()) {
			continue;
		}

		auto s = ZoneStateSpawnsRepository::NewEntity();
		s.zone_id             = GetZoneID();
		s.instance_id         = GetInstanceID();
		s.npc_id              = n.second->GetNPCTypeID();
		s.is_corpse           = 1;
		s.spawn2_id           = 0;
		s.spawngroup_id       = 0;
		s.x                   = n.second->GetX();
		s.y                   = n.second->GetY();
		s.z                   = n.second->GetZ();
		s.heading             = n.second->GetHeading();
		s.respawn_time        = 0;
		s.variance            = 0;
		s.grid                = 0;
		s.path_when_zone_idle = 0;
		s.condition_id        = 0;
		s.condition_min_value = 0;
		s.enabled             = 1;
		s.anim                = 0;
		s.created_at          = std::time(nullptr);
		s.loot_data           = GetLootSerialized(n.second);
		s.decay_in_seconds    = (int) (n.second->GetDecayTime() / 1000);

		spawns.emplace_back(s);
	}

	ZoneStateSpawnsRepository::DeleteWhere(
		database,
		fmt::format(
			"`zone_id` = {} AND `instance_id` = {}",
			GetZoneID(),
			GetInstanceID()
		)
	);

	ZoneStateSpawnsRepository::InsertMany(database, spawns);

	LogInfo("Saved [{}] zone state spawns", Strings::Commify(spawns.size()));
}

void Zone::ClearZoneState(uint32 zone_id, uint32 instance_id)
{
	ZoneStateSpawnsRepository::DeleteWhere(
		database,
		fmt::format(
			"`zone_id` = {} AND `instance_id` = {}",
			zone_id,
			instance_id
		)
	);
}
