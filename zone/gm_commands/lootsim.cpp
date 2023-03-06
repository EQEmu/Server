#include "../client.h"

void command_lootsim(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (arguments < 3 || !sep->IsNumber(1) || !sep->IsNumber(2) || !sep->IsNumber(3)) {
		c->Message(Chat::White, "Usage: #lootsim [npc_type_id] [loottable_id] [iterations] [loot_log_enabled=0]");
		return;
	}

	auto npc_id       = Strings::ToUnsignedInt(sep->arg[1]);
	auto loottable_id = Strings::ToUnsignedInt(sep->arg[2]);
	auto iterations   = Strings::ToUnsignedInt(sep->arg[3]) > 1000 ? 1000 : Strings::ToUnsignedInt(sep->arg[3]);
	auto log_enabled  = arguments > 3 ? Strings::ToUnsignedInt(sep->arg[4]) : false;

	// temporarily disable loot logging unless set explicitly
	LogSys.log_settings[Logs::Loot].log_to_console = log_enabled ? LogSys.log_settings[Logs::Loot].log_to_console : 0;
	LogSys.log_settings[Logs::Loot].log_to_file    = log_enabled ? LogSys.log_settings[Logs::Loot].log_to_file : 0;
	LogSys.log_settings[Logs::Loot].log_to_gmsay   = log_enabled ? LogSys.log_settings[Logs::Loot].log_to_gmsay : 0;

	auto npc_type = content_db.LoadNPCTypesData(npc_id);
	if (npc_type) {
		auto npc = new NPC(npc_type, nullptr, c->GetPosition(), GravityBehavior::Water);
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

			c->SendChatLineBreak();
			c->Message(
				Chat::White,
				fmt::format(
					"# [Loot Simulator] NPC [{}] ({}) Loot Table ID [{}] Dropped Items [{}] iterations [{}]",
					npc->GetCleanName(),
					npc_id,
					loottable_id,
					npc->GetRolledItems().size(),
					iterations
				).c_str()
			);
			c->SendChatLineBreak();

			// npc level loot table
			auto loot_table = database.GetLootTable(loottable_id);
			if (!loot_table) {
				c->Message(Chat::Red, "Loot table not found");
				return;
			}

			for (uint32 i = 0; i < loot_table->NumEntries; i++) {
				auto le = loot_table->Entries[i];

				c->Message(
					Chat::White,
					fmt::format(
						"# Lootdrop ID [{}] drop_limit [{}] min_drop [{}] mult [{}] probability [{}]",
						le.lootdrop_id,
						le.droplimit,
						le.mindrop,
						le.multiplier,
						le.probability
					).c_str()
				);

				auto loot_drop = database.GetLootDrop(le.lootdrop_id);
				if (!loot_drop) {
					continue;
				}

				for (uint32 ei = 0; ei < loot_drop->NumEntries; ei++) {
					auto               e            = loot_drop->Entries[ei];
					int                rolled_count = npc->GetRolledItemCount(e.item_id);
					const EQ::ItemData *item        = database.GetItem(e.item_id);

					EQ::SayLinkEngine linker;
					linker.SetLinkType(EQ::saylink::SayLinkItemData);
					linker.SetItemData(item);

					auto rolled_percentage = (float) ((float) ((float) rolled_count / (float) iterations) * 100);

					c->Message(
						Chat::White,
						fmt::format(
							"-- [{}] item_id [{}] chance [{}] rolled_count [{}] ({:.2f}%) name [{}]",
							ei,
							e.item_id,
							e.chance,
							rolled_count,
							rolled_percentage,
							linker.GenerateLink()
						).c_str()
					);
				}
			}


			// global loot
			auto tables = zone->GetGlobalLootTables(npc);
			if (!tables.empty()) {
				c->SendChatLineBreak();
				c->Message(Chat::White, "# [Loot Simulator] Global Loot");
			}

			for (auto &id: tables) {
				c->SendChatLineBreak();
				c->Message(Chat::White, fmt::format("# Global Loot Table ID [{}]", id).c_str());
				c->SendChatLineBreak();

				loot_table = database.GetLootTable(id);
				if (!loot_table) {
					c->Message(Chat::Red, fmt::format("Global Loot table not found [{}]", id).c_str());
					continue;
				}

				for (uint32 i = 0; i < loot_table->NumEntries; i++) {
					auto le = loot_table->Entries[i];

					c->Message(
						Chat::White,
						fmt::format(
							"# Lootdrop ID [{}] drop_limit [{}] min_drop [{}] mult [{}] probability [{}]",
							le.lootdrop_id,
							le.droplimit,
							le.mindrop,
							le.multiplier,
							le.probability
						).c_str()
					);

					auto loot_drop = database.GetLootDrop(le.lootdrop_id);
					if (!loot_drop) {
						continue;
					}

					for (uint32 ei = 0; ei < loot_drop->NumEntries; ei++) {
						auto               e            = loot_drop->Entries[ei];
						int                rolled_count = npc->GetRolledItemCount(e.item_id);
						const EQ::ItemData *item        = database.GetItem(e.item_id);

						EQ::SayLinkEngine linker;
						linker.SetLinkType(EQ::saylink::SayLinkItemData);
						linker.SetItemData(item);

						auto rolled_percentage = (float) ((float) ((float) rolled_count / (float) iterations) * 100);

						c->Message(
							Chat::White,
							fmt::format(
								"-- [{}] item_id [{}] chance [{}] rolled_count [{}] ({:.2f}%) name [{}]",
								ei,
								e.item_id,
								e.chance,
								rolled_count,
								rolled_percentage,
								linker.GenerateLink()
							).c_str()
						);
					}
				}
			}


			c->SendChatLineBreak();
			c->Message(
				Chat::White,
				fmt::format(
					"# Global Loot Benchmark End [{}] iterations took [{}](s)",
					iterations,
					benchmark.elapsed()
				).c_str()
			);
			c->SendChatLineBreak();

			LogSys.LoadLogDatabaseSettings();
		}
	}
	else {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to spawn NPC ID {}.",
				npc_id
			).c_str()
		);
	}
}


