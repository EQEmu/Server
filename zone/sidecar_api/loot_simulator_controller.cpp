void SidecarApi::LootSimulatorController(const httplib::Request &req, httplib::Response &res)
{
	int  loottable_id = req.has_param("loottable_id") ? std::stoi(req.get_param_value("loottable_id")) : 4027;
	int  npc_id       = 32040; // lord nagafen
	auto iterations   = 100;
	auto log_enabled  = false;

	LogSys.log_settings[Logs::Loot].log_to_console = 0;

	nlohmann::json j;

	auto npc_type = content_db.LoadNPCTypesData(npc_id);
	if (npc_type) {
		auto npc = new NPC(
			npc_type,
			nullptr,
			glm::vec4(0, 0, 0, 0),
			GravityBehavior::Water
		);
		if (npc) {

			BenchTimer benchmark;

			npc->SetRecordLootStats(true);
			for (int i = 0; i < iterations; i++) {
				npc->AddLootTable(loottable_id);

				for (auto &id: zone->GetGlobalLootTables(npc)) {
					npc->AddLootTable(id);
				}
			}

			entity_list.AddNPC(npc);

			j["data"]["loottable_id"]       = loottable_id;
			j["data"]["npc_id"]             = npc_id;
			j["data"]["npc_name"]           = npc->GetCleanName();
			j["data"]["rolled_items_count"] = npc->GetRolledItems().size();
			j["data"]["iterations"]         = iterations;

			// npc level loot table
			auto loot_table = database.GetLootTable(loottable_id);
			if (!loot_table) {
				res.status = 400;
				j["error"] = fmt::format("Loot table not found [{}]", loottable_id);
				res.set_content(j.dump(), "application/json");
				return;
			}

			for (uint32 i = 0; i < loot_table->NumEntries; i++) {
				auto le = loot_table->Entries[i];

				nlohmann::json jle;
				jle["lootdrop_id"] = le.lootdrop_id;
				jle["droplimit"]   = le.droplimit;
				jle["mindrop"]     = le.mindrop;
				jle["multiplier"]  = le.multiplier;
				jle["probability"] = le.probability;

				auto loot_drop = database.GetLootDrop(le.lootdrop_id);
				if (!loot_drop) {
					continue;
				}

				for (uint32 ei = 0; ei < loot_drop->NumEntries; ei++) {
					auto               e            = loot_drop->Entries[ei];
					int                rolled_count = npc->GetRolledItemCount(e.item_id);
					const EQ::ItemData *item        = database.GetItem(e.item_id);


					auto rolled_percentage = (float) ((float) ((float) rolled_count / (float) iterations) * 100);

					nlohmann::json drop;
					drop["slot"]                       = ei;
					drop["item_id"]                    = e.item_id;
					drop["item_name"]                  = item->Name;
					drop["chance"]                     = fmt::format("{:.2f}", e.chance);
					drop["simulate_rolled_count"]      = rolled_count;
					drop["simulate_rolled_percentage"] = fmt::format("{:.2f}", rolled_percentage);
					jle["drops"].push_back(drop);
				}

				j["lootdrops"].push_back(jle);
			}

			// global loot
			for (auto &id: zone->GetGlobalLootTables(npc)) {
				loot_table = database.GetLootTable(id);
				if (!loot_table) {
					LogInfo("Global Loot table not found [{}]", id);
					continue;
				}

				for (uint32 i = 0; i < loot_table->NumEntries; i++) {
					auto le = loot_table->Entries[i];

					LogInfo(
						"# Lootdrop ID [{}] drop_limit [{}] min_drop [{}] mult [{}] probability [{}]",
						le.lootdrop_id,
						le.droplimit,
						le.mindrop,
						le.multiplier,
						le.probability
					);

					nlohmann::json jle;
					jle["lootdrop_id"] = le.lootdrop_id;
					jle["droplimit"]   = le.droplimit;
					jle["mindrop"]     = le.mindrop;
					jle["multiplier"]  = le.multiplier;
					jle["probability"] = le.probability;

					auto loot_drop = database.GetLootDrop(le.lootdrop_id);
					if (!loot_drop) {
						continue;
					}

					for (uint32 ei = 0; ei < loot_drop->NumEntries; ei++) {
						auto               e            = loot_drop->Entries[ei];
						int                rolled_count = npc->GetRolledItemCount(e.item_id);
						const EQ::ItemData *item        = database.GetItem(e.item_id);

						auto rolled_percentage = (float) ((float) ((float) rolled_count / (float) iterations) *
														  100);


						LogInfo(
							"-- [{}] item_id [{}] chance [{}] rolled_count [{}] ({:.2f}%) name [{}]",
							ei,
							e.item_id,
							e.chance,
							rolled_count,
							rolled_percentage,
							item->Name
						);

						nlohmann::json drop;
						drop["slot"]                       = ei;
						drop["item_id"]                    = e.item_id;
						drop["item_name"]                  = item->Name;
						drop["chance"]                     = fmt::format("{:.2f}", e.chance);
						drop["simulate_rolled_count"]      = rolled_count;
						drop["simulate_rolled_percentage"] = fmt::format("{:.2f}", rolled_percentage);
						jle["drops"].push_back(drop);

						j["global"]["lootdrops"].push_back(jle);
					}
				}
			}

			j["data"]["time"] = benchmark.elapsed();

			res.status = 200;
			res.set_content(j.dump(), "application/json");
		}
	}
	else {
		res.status = 400;
		j["error"] = fmt::format("Failed to spawn NPC ID [{}]", npc_id);
		res.set_content(j.dump(), "application/json");
	}
}
