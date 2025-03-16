extern Zone *zone;

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

void ZoneCLI::TestZoneState(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	if (cmd[{"-h", "--help"}]) {
		return;
	}

	ClearState(); // clean slate
	SetupStateZone();

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


//	zone->Repop();
//	entries = GetStateSpawns().size();
//	RunTest(fmt::format("State does not exist after repop, entries ({})", entries), true, entries == 0);




	std::cout << "\n===========================================\n";
	std::cout << "✅ All Zone State Tests Completed!\n";
	std::cout << "===========================================\n";
}
