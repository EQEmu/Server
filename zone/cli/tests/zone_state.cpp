extern Zone *zone;

#include "../../common/repositories/npc_types_repository.h"
#include "../../corpse.h"

inline void ClearState()
{
	ZoneStateSpawnsRepository::DeleteWhere(database, "zone_id = 32 and instance_id = 0");
}

inline std::vector<ZoneStateSpawnsRepository::ZoneStateSpawns> GetStateSpawns()
{
	return ZoneStateSpawnsRepository::GetWhere(database, "zone_id = 32 and instance_id = 0");
}

inline void PrintZoneNpcs()
{
	for (auto &npc: entity_list.GetNPCList()) {
		std::cout << npc.second->GetNPCTypeID() << " " << npc.second->GetCleanName() << std::endl;
	}
}

inline void SetupStateZone()
{
	SetupZone("soldungb");
	zone->Process();
	// depop the zone controller
	entity_list.GetNPCByNPCTypeID(ZONE_CONTROLLER_NPC_ID)->Depop();
	entity_list.MobProcess(); // process the depop
}

inline int GetStateSpawnSpawn2Count()
{
	int       count = 0;
	for (auto &e: GetStateSpawns()) {
		if (e.spawn2_id > 0) {
			count++;
		}
	}
	return count;
}

inline uint32_t SeedLootTable()
{
	const std::string table_name = "zone_state_test";

	auto entries = LoottableRepository::GetWhere(database, fmt::format("name = '{}'", table_name));
	if (!entries.empty()) {
		zone->LoadLootTable(entries[0].id);
		return entries[0].id;
	}

	// seed the loot table
	auto loot_table = LoottableRepository::NewEntity();
	loot_table.name = table_name;
	auto inserted = LoottableRepository::InsertOne(database, loot_table);

	auto loot_drop = LootdropRepository::NewEntity();
	loot_drop.name = table_name;
	auto inserted_loot_drop = LootdropRepository::InsertOne(database, loot_drop);

	auto loot_table_entries = LoottableEntriesRepository::NewEntity();
	loot_table_entries.lootdrop_id  = inserted_loot_drop.id;
	loot_table_entries.loottable_id = inserted.id;
	LoottableEntriesRepository::InsertOne(database, loot_table_entries);

	auto loot_drop_entries = LootdropEntriesRepository::NewEntity();
	loot_drop_entries.lootdrop_id = inserted_loot_drop.id;
	loot_drop_entries.item_id     = 11621; // cloak of flames
	loot_drop_entries.chance      = 100;
	LootdropEntriesRepository::InsertOne(database, loot_drop_entries);

	zone->LoadLootTable(inserted.id);

	return inserted.id;
}

void ZoneCLI::TestZoneState(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	if (cmd[{"-h", "--help"}]) {
		return;
	}

	ClearState(); // clean slate
	SetupStateZone();
	zone->Repop(true);

	std::cout << "===========================================\n";
	std::cout << "⚙\uFE0F> Running Zone State Tests... (soldungb)\n";
	std::cout << "===========================================\n\n";

	RunTest("Ensure no state spawns exist before shutdown", 0, (int) GetStateSpawns().size());

	zone->Shutdown();

	auto entries = GetStateSpawns().size();
	RunTest(fmt::format("State exists after shutdown, entries ({})", entries), true, entries > 0);

	SetupStateZone();

	entries = GetStateSpawns().size();
	RunTest(fmt::format("State exists after bootup, entries ({})", entries), true, entries > 0);

	zone->Shutdown();
	SetupStateZone();

	entries = GetStateSpawns().size();
	RunTest(
		fmt::format("State is the same after shutdown/bootup (2nd time), entries ({})", entries),
		true,
		entries > 0
	);

	// need to compare the state spawns to the actual spawns
	bool      all_exist = true;
	for (auto &state_spawn: GetStateSpawns()) {
		auto npc = entity_list.GetNPCByNPCTypeID(state_spawn.npc_id);
		if (!npc) {
			all_exist = false;
			break;
		}
	}

	RunTest("All state spawns (Spawn2) exist in entity list", true, all_exist);
	RunTest(
		fmt::format("All state spawns (Spawn2) exist in entity list, count [{}]", GetStateSpawns().size()),
		true,
		GetStateSpawns().size() == entity_list.GetNPCList().size()
	);
	RunTest(
		fmt::format(
			"Same count of state spawns and entity list | state spawns [{}] entity list [{}]",
			GetStateSpawns().size(),
			entity_list.GetNPCList().size()),
		true,
		GetStateSpawns().size() == entity_list.GetNPCList().size()
	);

	std::vector<std::pair<std::string, std::string>> test_variables = {
		{"test_variable",  "test_value"},
		{"test_variable2", "test_value2"}
	};

	// Set variables
	for (const auto &[key, value]: test_variables) {
		zone->SetVariable(key, value);
		RunTest("Zone variable (" + key + ") set", value, zone->GetVariable(key));
	}

	// Simulate shutdown and restart twice
	for (int i = 1; i <= 2; ++i) {
		zone->Shutdown();
		SetupStateZone();

		for (const auto &[key, value]: test_variables) {
			RunTest(
				"Zone variable (" + key + ") persists after shutdown/bootup (" + std::to_string(i) + ")",
				value,
				zone->GetVariable(key)
			);
		}
	}

	// Delete one variable
	zone->DeleteVariable("test_variable");

	RunTest("Zone variable (test_variable) delete is empty", "", zone->GetVariable("test_variable"));
	RunTest(
		"Zone variable (test_variable2) delete second one still exists",
		"test_value2",
		zone->GetVariable("test_variable2")
	);

	// Final shutdown and restart check
	zone->Shutdown();
	SetupStateZone();

	for (const auto &[key, value]: test_variables) {
		std::string expected_value = (key == "test_variable") ? "" : value;
		RunTest("Zone variable (" + key + ") after shutdown/bootup", expected_value, zone->GetVariable(key));
	}

	std::vector<uint32_t> ids = {};
	for (auto             &npc: entity_list.GetNPCList()) {
		ids.push_back(npc.second->GetNPCTypeID());
	}

	auto npc_types = NpcTypesRepository::GetWhere(database, fmt::format(" id IN ({})", Strings::Join(ids, ",")));

	// validate that hp / mana / end equal that of what's on the npc types row for all rows
	// dont run tests in the loop, just collect the data
	std::vector<std::pair<uint32_t, bool>> hp_mismatch   = {};
	std::vector<std::pair<uint32_t, bool>> mana_mismatch = {};
	for (auto                              &e: entity_list.GetNPCList()) {
		auto      npc = e.second;
		for (auto &npc_type: npc_types) {
			if (npc->GetNPCTypeID() != npc_type.id) {
				continue;
			}

			// we need to make sure the values are not 0 because of auto scaling
			if (npc->GetHP() != npc_type.hp && npc_type.hp > 0) {
				hp_mismatch.emplace_back(npc->GetNPCTypeID(), true);
			}
			if (npc->GetMana() != npc_type.mana && npc_type.mana > 0) {
				mana_mismatch.emplace_back(npc->GetNPCTypeID(), true);
			}
		}
	}

	RunTest("HP Restore | Ensure default state matches data in npc_types row", 0, (int) hp_mismatch.size());
	RunTest("Mana Restore | Ensure default state matches data in npc_types row", 0, (int) mana_mismatch.size());

	// do damage to NPC's and make sure they restore to their original values
	for (auto &e: entity_list.GetNPCList()) {
		auto npc = e.second;
		npc->SetHP(1);
		npc->SetMana(1);
	}

	zone->Shutdown();
	SetupStateZone();

	hp_mismatch.clear();
	mana_mismatch.clear();

	// compare state values versus what's on the entity list NPC object
	for (auto &e: entity_list.GetNPCList()) {
		auto      npc = e.second;
		for (auto &state: GetStateSpawns()) {
			if (npc->GetNPCTypeID() != state.npc_id) {
				continue;
			}

			// we need to make sure the values are not 0 because of auto scaling
			if (npc->GetHP() != state.hp && state.hp > 0) {
				hp_mismatch.emplace_back(npc->GetNPCTypeID(), true);
			}
			if (npc->GetMana() != state.mana && state.mana > 0) {
				mana_mismatch.emplace_back(npc->GetNPCTypeID(), true);
			}
		}
	}

	RunTest("HP Restore | Ensure restored state matches data on the NPC object", 0, (int) hp_mismatch.size());
	RunTest("Mana Restore | Ensure restored state matches data on the NPC object", 0, (int) mana_mismatch.size());

	// Test that state spawns are where we moved them to if we moved them, move them slightly in a predictable way so
	// we can test for it after restore
	for (auto &e: entity_list.GetNPCList()) {
		auto npc = e.second;
		if (!npc) {
			continue;
		}

		// set all to -870	-1394	106.58 (nagafen area)
		npc->SetPosition(-870, -1394, 106);
	}

	zone->Shutdown();
	SetupStateZone();

	bool all_moved = true;

	for (auto &e: entity_list.GetNPCList()) {
		for (auto &state: GetStateSpawns()) {
			if (e.second->GetNPCTypeID() != state.npc_id) {
				continue;
			}

			auto npc = e.second;
			if (npc->GetSpawnGroupId() != state.spawngroup_id && npc->GetSpawn()->GetID() != state.spawn2_id) {
				continue;
			}

			// z gets auto adjusted to the ground, so we dont need to check it
			if (e.second->GetX() != state.x || e.second->GetY() != state.y) {
				std::cout << "NPC ID: " << e.second->GetNPCTypeID() << " X: " << e.second->GetX() << " Y: "
						  << e.second->GetY() << std::endl;
				std::cout << "State ID " << state.npc_id << " X: " << state.x << " Y: " << state.y << std::endl;
				std::cout << "-----------------------------------\n";
				all_moved = false;
				break;
			}
			else {
//				std::cout << "NPC ID: " << e.second->GetNPCTypeID() << " X: " << e.second->GetX() << " Y: " << e.second->GetY() << std::endl;
//				std::cout << "State ID " << state.npc_id << " X: " << state.x << " Y: " << state.y << std::endl;
//				std::cout << "-----------------------------------\n";
			}
		}
	}

	RunTest("State spawns are where we moved them to after restore", true, all_moved);

	// test entity variables
	std::map<std::string, std::string> test_entity_variables = {
		{"test_entity_variable",  "test_entity_value"},
		{"test_entity_variable2", "test_entity_value2"}
	};

	// Set entity variables
	for (const auto &[key, value]: test_entity_variables) {
		for (auto &e: entity_list.GetNPCList()) {
			e.second->SetEntityVariable(key, value);
		}
	}

	zone->Shutdown();
	SetupStateZone();

	// Check entity variables
	bool missing_entity_variables = false;
	for (const auto &[key, value]: test_entity_variables) {
		for (auto &e: entity_list.GetNPCList()) {
			if (e.second->GetEntityVariable(key) != value) {
				missing_entity_variables = true;
				break;
			}
		}
	}

	RunTest("Entity variables persist after shutdown/bootup", false, missing_entity_variables);

	// buffs
	for (auto &e: entity_list.GetNPCList()) {
		auto npc = e.second;
		if (npc->GetNPCTypeID() == 0) {
			continue;
		}

		npc->CastSpell(6824, npc->GetID(), (EQ::spells::CastingSlot) 0, 0, 0);
	}

	zone->Shutdown();
	SetupStateZone();

	// Check buffs
	bool missing_buffs = false;

	for (auto &e: entity_list.GetNPCList()) {
		auto npc = e.second;
		if (npc->GetNPCTypeID() == 0) {
			continue;
		}

		if (!npc->FindBuff(6824)) {
			missing_buffs = true;
			break;
		}
	}

	RunTest("Buffs persist after shutdown/bootup", false, missing_buffs);

	// loot
	uint32_t table_id = SeedLootTable();

	for (auto &e: entity_list.GetNPCList()) {
		e.second->ClearLootItems();
		e.second->SetResumedFromZoneSuspend(false);
		e.second->AddLootTable(table_id);
		e.second->SetResumedFromZoneSuspend(true);
	}

	bool missing_loot = false;

	for (auto &e: entity_list.GetNPCList()) {
		auto npc = e.second;
		if (npc->GetNPCTypeID() == 0) {
			continue;
		}

		// cloak of flames
		if (npc->CountItem(11621) == 0) {
			missing_loot = true;
			break;
		}
	}

	RunTest("Loot | Cloak of Flames added to all NPC's via Loottable before shutdown", false, missing_loot);

	zone->Shutdown();
	SetupStateZone();

	missing_loot = false;
	for (auto &e: entity_list.GetNPCList()) {
		auto npc = e.second;
		if (npc->GetNPCTypeID() == 0) {
			continue;
		}

		// cloak of flames
		if (npc->CountItem(11621) == 0) {
			missing_loot = true;
			break;
		}
	}

	RunTest("Loot | Cloak of Flames added to all NPC's via Loottable after shutdown/bootup", false, missing_loot);

	// make sure no duplicates are added
	bool duplicates = false;

	for (auto &e: entity_list.GetNPCList()) {
		auto npc = e.second;
		if (npc->GetNPCTypeID() == 0) {
			continue;
		}

		if (npc->CountItem(11621) > 1) {
			duplicates = true;
			break;
		}
	}

	RunTest("Loot | No duplicates added when adding item to NPC", false, duplicates);

	// kill all NPC's
	std::vector<NPC*> npcs_to_kill;

	// Collect NPCs first
	for (const auto& e : entity_list.GetNPCList()) {
		if (e.second) {
			npcs_to_kill.push_back(e.second);
		}
	}

	// Now safely process them
	for (auto* npc : npcs_to_kill) {
		npc->SetQueuedToCorpse();
		npc->Death(npc, npc->GetHP() + 1, SPELL_UNKNOWN, EQ::skills::SkillHandtoHand);
	}

	// make sure all of the corpses have "Cloak of Flames"
	bool missing_loot_corpse = false;
	for (auto &e: entity_list.GetCorpseList()) {
		auto corpse = e.second;
		if (corpse->GetNPCTypeID() == 0) {
			continue;
		}

		if (corpse->CountItem(11621) == 0) {
			missing_loot_corpse = true;
			break;
		}
	}

	RunTest("Loot | Cloak of Flames added to all Corpse's via Loottable before shutdown/bootup", false, missing_loot_corpse);

	zone->Shutdown();
	SetupStateZone();

	missing_loot_corpse = false;
	for (auto &e: entity_list.GetCorpseList()) {
		auto corpse = e.second;
		if (corpse->GetNPCTypeID() == 0) {
			continue;
		}

		if (corpse->CountItem(11621) == 0) {
			missing_loot_corpse = true;
			break;
		}
	}

	RunTest("Loot | Cloak of Flames added to all Corpse's via Loottable after shutdown/bootup", false, missing_loot_corpse);

	// make sure no duplicates are added
	bool duplicates_corpse = false;
	for (auto &e: entity_list.GetCorpseList()) {
		auto corpse = e.second;
		if (corpse->GetNPCTypeID() == 0) {
			continue;
		}

		if (corpse->CountItem(11621) > 1) {
			duplicates_corpse = true;
			break;
		}
	}

	RunTest("Loot | No duplicates added when adding item to Corpse", false, duplicates_corpse);


//	ClearState();

//	zone->Repop();
//	entries = GetStateSpawns().size();
//	RunTest(fmt::format("State does not exist after repop, entries ({})", entries), true, entries == 0);




	std::cout << "\n===========================================\n";
	std::cout << "✅ All Zone State Tests Completed!\n";
	std::cout << "===========================================\n";
}
