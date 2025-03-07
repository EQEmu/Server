#include <string>
#include <cereal/archives/json.hpp>
#include <cereal/types/map.hpp>
#include "npc.h"
#include "corpse.h"
#include "zone.h"
#include "../common/repositories/zone_state_spawns_repository.h"
#include "../common/repositories/spawn2_disabled_repository.h"

struct LootEntryStateData {
	uint32   item_id;
	uint32_t lootdrop_id;
	uint16   charges = 0; // used in dynamically added loot (AddItem)

	// cereal
	template<class Archive>
	void serialize(Archive &ar)
	{
		ar(
			CEREAL_NVP(item_id),
			CEREAL_NVP(lootdrop_id),
			CEREAL_NVP(charges)
		);
	}
};

struct LootStateData {
	uint32                          copper   = 0;
	uint32                          silver   = 0;
	uint32                          gold     = 0;
	uint32                          platinum = 0;
	std::vector<LootEntryStateData> entries  = {};

	// cereal
	template<class Archive>
	void serialize(Archive &ar)
	{
		ar(
			CEREAL_NVP(copper),
			CEREAL_NVP(silver),
			CEREAL_NVP(gold),
			CEREAL_NVP(platinum),
			CEREAL_NVP(entries)
		);
	}
};

inline bool IsValidJson(const std::string& json) {
	rapidjson::Document doc;
	rapidjson::ParseResult result = doc.Parse(json.c_str());

	return result;
}

inline void LoadLootStateData(Zone *zone, NPC *npc, const std::string &loot_data)
{
	LootStateData l{};

	if (!IsValidJson(loot_data)) {
		LogZoneState("Invalid JSON data for NPC [{}]", npc->GetNPCTypeID());
		return;
	}

	try {
		std::stringstream ss;
		{
			ss << loot_data;
			cereal::JSONInputArchive ar(ss);
			l.serialize(ar);
		}
	} catch (const std::exception &e) {
		LogZoneState("Failed to load loot state data for NPC [{}] [{}]", npc->GetNPCTypeID(), e.what());
		return;
	}

	npc->AddLootCash(l.copper, l.silver, l.gold, l.platinum);

	for (auto &e: l.entries) {
		const auto *db_item = database.GetItem(e.item_id);
		if (!db_item) {
			continue;
		}

		// dynamically added via AddItem
		if (e.lootdrop_id == 0) {
			npc->AddItem(e.item_id, e.charges);
			continue;
		}

		const auto entries = zone->GetLootdropEntries(e.lootdrop_id);
		if (entries.empty()) {
			continue;
		}

		LootdropEntriesRepository::LootdropEntries lootdrop_entry;

		for (auto &le: entries) {
			if (e.item_id == le.item_id) {
				lootdrop_entry = le;
				break;
			}
		}

		npc->AddLootDrop(db_item, lootdrop_entry);
	}
}

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
				.charges = l->charges,
			}
		);
	}

	try {
		std::stringstream ss;
		{
			cereal::JSONOutputArchiveSingleLine ar(ss);
			ls.serialize(ar);
		}

		return ss.str();
	} catch (const std::exception &e) {
		LogZoneState("Failed to serialize loot data for NPC [{}] [{}]", npc->GetNPCTypeID(), e.what());
		return "";
	}

	return "";
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

	try {
		std::stringstream ss;
		{
			cereal::JSONOutputArchiveSingleLine ar(ss);
			ls.serialize(ar);
		}

		return ss.str();
	} catch (const std::exception &e) {
		LogZoneState("Failed to serialize loot data for Corpse [{}] [{}]", c->GetID(), e.what());
		return "";
	}

	return "";
}

inline void LoadNPCEntityVariables(NPC *n, const std::string &entity_variables)
{
	if (!IsValidJson(entity_variables)) {
		LogZoneState("Invalid JSON data for NPC [{}]", n->GetNPCTypeID());
		return;
	}

	std::map<std::string, std::string> deserialized_map;
	try {
		std::istringstream is(entity_variables);
		{
			cereal::JSONInputArchive archive(is);
			archive(deserialized_map);
		}
	}
	catch (const std::exception &e) {
		LogZoneState("Failed to load entity variables for NPC [{}] [{}]", n->GetNPCTypeID(), e.what());
		return;
	}

	for (const auto &[key, value]: deserialized_map) {
		n->SetEntityVariable(key, value);
	}
}

inline void LoadNPCBuffs(NPC *n, const std::string &buffs)
{
	if (!IsValidJson(buffs)) {
		LogZoneState("Invalid JSON data for NPC [{}]", n->GetNPCTypeID());
		return;
	}

	std::vector<Buffs_Struct> valid_buffs;
	try {
		std::istringstream is(buffs);
		{
			cereal::JSONInputArchive archive(is);
			archive(cereal::make_nvp("buffs", valid_buffs));
		}
	}
	catch (const std::exception &e) {
		LogZoneState("Failed to load entity variables for NPC [{}] [{}]", n->GetNPCTypeID(), e.what());
		return;
	}

	for (const auto &b: valid_buffs) {
		// int AddBuff(Mob *caster, const uint16 spell_id, int duration = 0, int32 level_override = -1, bool disable_buff_overwrite = false);
		n->AddBuff(n, b.spellid, b.ticsremaining, b.casterlevel, false);
	}
}

inline std::vector<uint32_t> GetLootdropIds(const std::vector<ZoneStateSpawnsRepository::ZoneStateSpawns> &spawn_states)
{
	LogInfo("Loading lootdrop ids for zone state spawns");

	std::vector<uint32_t> lootdrop_ids;
	for (auto             &s: spawn_states) {
		if (s.loot_data.empty()) {
			continue;
		}

		LootStateData l{};
		try {
			std::stringstream ss;
			{
				ss << s.loot_data;
				cereal::JSONInputArchive ar(ss);
				l.serialize(ar);
			}
		}
		catch (const std::exception &e) {
			LogZoneState("Failed to load loot state data for spawn2 [{}] [{}]", s.id, e.what());
			continue;
		}

		for (auto &e: l.entries) {
			// make sure it isn't already in the list
			if (std::find(lootdrop_ids.begin(), lootdrop_ids.end(), e.lootdrop_id) == lootdrop_ids.end()) {
				lootdrop_ids.push_back(e.lootdrop_id);
			}
		}
	}

	LogInfo("Loaded [{}] lootdrop id(s)", lootdrop_ids.size());

	return lootdrop_ids;
}

inline void LoadNPCState(Zone *zone, NPC *n, ZoneStateSpawnsRepository::ZoneStateSpawns &s)
{
	n->SetHP(s.hp);
	n->SetMana(s.mana);
	n->SetEndurance(s.endurance);

	if (s.grid) {
		n->AssignWaypoints(s.grid, s.current_waypoint);
	}

	LoadLootStateData(zone, n, s.loot_data);
	LoadNPCEntityVariables(n, s.entity_variables);
	LoadNPCBuffs(n, s.buffs);

	if (s.is_corpse) {
		auto decay_time = s.decay_in_seconds * 1000;
		if (decay_time > 0) {
			n->SetQueuedToCorpse();
			n->SetCorpseDecayTime(decay_time);
		}
		else {
			n->Depop();
		}
	}

	n->SetResumedFromZoneSuspend(true);
}

bool Zone::LoadZoneState(
	std::unordered_map<uint32, uint32> spawn_times,
	std::vector<Spawn2DisabledRepository::Spawn2Disabled> disabled_spawns
)
{
	auto spawn_states = ZoneStateSpawnsRepository::GetWhere(
		database,
		fmt::format(
			"zone_id = {} AND instance_id = {}",
			zoneid,
			zone->GetInstanceID()
		)
	);

	LogInfo("Loading zone state spawns for zone [{}] spawns [{}]", GetShortName(), spawn_states.size());

	std::vector<uint32_t> lootdrop_ids = GetLootdropIds(spawn_states);
	zone->LoadLootDrops(lootdrop_ids);

	// we have to load grids first otherwise setting grid/wp will not work
	zone->initgrids_timer.Trigger();
	zone->Process();

	for (auto &s: spawn_states) {
		if (s.spawngroup_id == 0) {
			continue;
		}

		if (s.is_corpse) {
			continue;
		}

		uint32 spawn_time_left = 0;
		if (spawn_times.count(s.spawn2_id) != 0) {
			spawn_time_left = spawn_times[s.spawn2_id];
			LogInfo("Spawn2 [{}] Respawn time left [{}]", s.spawn2_id, spawn_time_left);
		}

		// load from spawn2_disabled
		bool spawn_enabled = true;

		// check if spawn is disabled
		for (auto &ds: disabled_spawns) {
			if (ds.spawn2_id == s.spawn2_id) {
				spawn_enabled = !ds.disabled;
			}
		}

		auto new_spawn = new Spawn2(
			s.spawn2_id,
			s.spawngroup_id,
			s.x,
			s.y,
			s.z,
			s.heading,
			s.respawn_time,
			s.variance,
			spawn_time_left,
			s.grid,
			(bool) s.path_when_zone_idle,
			s.condition_id,
			s.condition_min_value,
			spawn_enabled,
			(EmuAppearance) s.anim
		);

		if (spawn_time_left == 0) {
			new_spawn->SetCurrentNPCID(s.npc_id);
		}

		spawn2_list.Insert(new_spawn);
		new_spawn->Process();
		auto n = new_spawn->GetNPC();
		if (n) {
			n->ClearLootItems();
			if (s.grid > 0) {
				n->AssignWaypoints(s.grid, s.current_waypoint);
			}
		}
	}

	// dynamic spawns, quest spawns, triggers etc.
	for (auto &s: spawn_states) {
		if (s.spawngroup_id > 0) {
			continue;
		}

		auto npc_type = content_db.LoadNPCTypesData(s.npc_id);
		if (!npc_type) {
			LogZoneState("Failed to load NPC type data for npc_id [{}]", s.npc_id);
			continue;
		}

		auto npc = new NPC(
			npc_type,
			nullptr,
			glm::vec4(s.x, s.y, s.z, s.heading),
			GravityBehavior::Water
		);

		entity_list.AddNPC(npc, true, true);

		LoadNPCState(zone, npc, s);
	}

	// any NPC that is spawned by the spawn system
	for (auto &e: entity_list.GetNPCList()) {
		auto npc = e.second;
		if (npc->GetSpawnGroupId() == 0) {
			continue;
		}

		for (auto &s: spawn_states) {
			bool is_same_npc =
					 s.npc_id == npc->GetNPCTypeID() &&
					 s.spawn2_id == npc->GetSpawnPointID() &&
					 s.spawngroup_id == npc->GetSpawnGroupId();
			if (is_same_npc) {
				LoadNPCState(zone, npc, s);
			}
		}
	}

	return !spawn_states.empty();
}

inline void SaveNPCState(NPC *n, ZoneStateSpawnsRepository::ZoneStateSpawns &s)
{
	// entity variables
	std::map<std::string, std::string> variables;
	for (const auto                    &k: n->GetEntityVariables()) {
		variables[k] = n->GetEntityVariable(k);
	}

	try {
		std::ostringstream os;
		{
			cereal::JSONOutputArchiveSingleLine archive(os);
			archive(variables);
		}
		s.entity_variables = os.str();
	}
	catch (const std::exception &e) {
		LogZoneState("Failed to serialize entity variables for NPC [{}] [{}]", n->GetNPCTypeID(), e.what());
		return;
	}

	// buffs
	auto buffs = n->GetBuffs();
	if (!buffs) {
		return;
	}

	std::vector<Buffs_Struct> valid_buffs;

	for (int index = 0; index < n->GetMaxBuffSlots(); index++) {
		if (buffs[index].spellid != 0 && buffs[index].spellid != 65535) {
			valid_buffs.push_back(buffs[index]);
		}
	}

	try {
		std::ostringstream os = std::ostringstream();
		{
			cereal::JSONOutputArchiveSingleLine archive(os);
			archive(cereal::make_nvp("buffs", valid_buffs));
		}
		s.buffs = os.str();
	}
	catch (const std::exception &e) {
		LogZoneState("Failed to serialize buffs for NPC [{}] [{}]", n->GetNPCTypeID(), e.what());
		return;
	}

	// rest
	s.npc_id           = n->GetNPCTypeID();
	s.loot_data        = GetLootSerialized(n);
	s.hp               = n->GetHP();
	s.mana             = n->GetMana();
	s.endurance        = n->GetEndurance();
	s.grid             = n->GetGrid();
	s.current_waypoint = n->GetGrid() > 0 ? n->GetCWP() : 0;
	s.x                = n->GetX();
	s.y                = n->GetY();
	s.z                = n->GetZ();
	s.heading          = n->GetHeading();
	s.created_at       = std::time(nullptr);
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

		auto n = sp->GetNPC();
		if (n) {
			SaveNPCState(n, s);
		}

		spawns.emplace_back(s);
		iterator.Advance();
	}

	// npcs that are not in the spawn2 list
	for (auto &n: entity_list.GetNPCList()) {
		// everything below here is dynamically spawned
		bool ignore_npcs =
				 n.second->GetSpawnGroupId() > 0 ||
				 n.second->GetNPCTypeID() < 100 ||
				 n.second->HasOwner();
		if (ignore_npcs) {
			continue;
		}

		auto s = ZoneStateSpawnsRepository::NewEntity();
		s.zone_id     = GetZoneID();
		s.instance_id = GetInstanceID();

		SaveNPCState(n.second, s);

		spawns.emplace_back(s);
	}

	// corpses
	for (auto &n: entity_list.GetCorpseList()) {
		if (!n.second->IsNPCCorpse()) {
			continue;
		}

		auto s = ZoneStateSpawnsRepository::NewEntity();
		s.zone_id          = GetZoneID();
		s.instance_id      = GetInstanceID();
		s.npc_id           = n.second->GetNPCTypeID();
		s.is_corpse        = 1;
		s.x                = n.second->GetX();
		s.y                = n.second->GetY();
		s.z                = n.second->GetZ();
		s.heading          = n.second->GetHeading();
		s.created_at       = std::time(nullptr);
		s.loot_data        = GetLootSerialized(n.second);
		s.decay_in_seconds = (int) (n.second->GetDecayTime() / 1000);

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
