#include "../../common/http/httplib.h"
#include "../../common/eqemu_logsys.h"
#include "../../common/platform.h"
#include "../zone.h"
#include "../client.h"
#include "../../common/net/eqstream.h"

extern Zone *zone;

void ZoneCLI::NpcHandinsMultiQuest(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	if (cmd[{"-h", "--help"}]) {
		return;
	}

	uint32 break_length = 50;
	int    failed_count = 0;

	RegisterExecutablePlatform(EQEmuExePlatform::ExePlatformZoneSidecar);

	LogInfo("{}", Strings::Repeat("-", break_length));
	LogInfo("Booting test zone for NPC handins (MultiQuest)");
	LogInfo("{}", Strings::Repeat("-", break_length));

	LogSys.SilenceConsoleLogging();

	Zone::Bootup(ZoneID("qrg"), 0, false);
	zone->StopShutdownTimer();

	entity_list.Process();
	entity_list.MobProcess();

	LogInfo("{}", Strings::Repeat("-", break_length));
	LogInfo("> Done booting test zone");
	LogInfo("{}", Strings::Repeat("-", break_length));

	Client *c       = new Client();
	auto   npc_type = content_db.LoadNPCTypesData(754008);
	if (npc_type) {
		auto npc = new NPC(
			npc_type,
			nullptr,
			glm::vec4(0, 0, 0, 0),
			GravityBehavior::Water
		);

		entity_list.AddNPC(npc);
		npc->MultiQuestEnable();

		LogInfo("> Spawned NPC [{}]", npc->GetCleanName());
		LogInfo("> Spawned client [{}]", c->GetCleanName());

		struct HandinEntry {
			std::string            item_id            = "0";
			uint32                 count              = 0;
			const EQ::ItemInstance *item              = nullptr;
			bool                   is_multiquest_item = false; // state
		};

		struct HandinMoney {
			uint32 platinum = 0;
			uint32 gold     = 0;
			uint32 silver   = 0;
			uint32 copper   = 0;
		};

		struct Handin {
			std::vector<HandinEntry> items = {}; // items can be removed from this set as successful handins are made
			HandinMoney              money = {}; // money can be removed from this set as successful handins are made
		};

		struct TestCase {
			std::string description = "";
			Handin      hand_in;
			Handin      required;
			Handin      returned;
			bool        handin_check_result;
		};

		std::vector<TestCase> test_cases = {
			TestCase{
				.description = "Journeyman's Boots",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "12268", .count = 1},
						HandinEntry{.item_id = "7100", .count = 1},
					},
					.money = {.platinum = 325},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "12268", .count = 1},
						HandinEntry{.item_id = "7100", .count = 1},
					},
					.money = {.platinum = 325},
				},
				.returned = {},
				.handin_check_result = true,
			},
		};

		std::map<std::string, uint32>   hand_ins;
		std::map<std::string, uint32>   required;
		std::vector<EQ::ItemInstance *> items;

		LogSys.EnableConsoleLogging();

		// turn this on to see debugging output
		LogSys.log_settings[Logs::NpcHandin].log_to_console = std::getenv("DEBUG") ? 3 : 0;

		LogInfo("{}", Strings::Repeat("-", break_length));

		for (auto &test_case: test_cases) {
			required.clear();

			for (auto &hand_in: test_case.hand_in.items) {
				hand_ins.clear();
				items.clear();

				auto             item_id = Strings::ToInt(hand_in.item_id);
				EQ::ItemInstance *inst   = database.CreateItem(item_id);
				if (inst->IsStackable()) {
					inst->SetCharges(hand_in.count);
				}

				if (inst->GetItem()->MaxCharges > 0) {
					inst->SetCharges(inst->GetItem()->MaxCharges);
				}

				hand_ins[hand_in.item_id] = inst->GetCharges();
				items.push_back(inst);

				npc->CheckHandin(c, hand_ins, required, items);
				npc->ResetHandin();
			}

			// money
			if (test_case.hand_in.money.platinum > 0) {
				hand_ins["platinum"] = test_case.hand_in.money.platinum;
			}
			if (test_case.hand_in.money.gold > 0) {
				hand_ins["gold"] = test_case.hand_in.money.gold;
			}
			if (test_case.hand_in.money.silver > 0) {
				hand_ins["silver"] = test_case.hand_in.money.silver;
			}
			if (test_case.hand_in.money.copper > 0) {
				hand_ins["copper"] = test_case.hand_in.money.copper;
			}

			for (auto &req: test_case.required.items) {
				required[req.item_id] = req.count;
			}

			// money
			if (test_case.required.money.platinum > 0) {
				required["platinum"] = test_case.required.money.platinum;
			}
			if (test_case.required.money.gold > 0) {
				required["gold"] = test_case.required.money.gold;
			}
			if (test_case.required.money.silver > 0) {
				required["silver"] = test_case.required.money.silver;
			}
			if (test_case.required.money.copper > 0) {
				required["copper"] = test_case.required.money.copper;
			}

			auto result = npc->CheckHandin(c, hand_ins, required, items);
			if (result != test_case.handin_check_result) {
				failed_count++;
				LogError("FAIL [{}]", test_case.description);
				// print out the hand-ins
				LogError("Hand-ins >");
				for (auto &item: npc->GetHandin().items) {
					LogError(" > Item [{}] count [{}]", item.item_id, item.count);
				}
				LogError("Required >");
				for (auto &req: required) {
					LogError(" > Item [{}] count [{}]", req.first, req.second);
				}
				LogError("Expected [{}] got [{}]", test_case.handin_check_result, result);
			}
			else {
				LogInfo("PASS [{}]", test_case.description);
			}

			auto returned = npc->ReturnHandinItems(c);

			// assert that returned items are expected
			for (auto &item: test_case.returned.items) {
				auto      found = false;
				for (auto &ret: returned.items) {
					if (ret.item_id == item.item_id) {
						found = true;
						break;
					}
				}
				if (!found) {
					LogError("Returned item [{}] not expected", item.item_id);
				}
			}

			npc->ResetHandin();

			if (LogSys.log_settings[Logs::NpcHandin].log_to_console > 0) {
				std::cout << std::endl;
			}
		}
	}

	if (failed_count > 0) {
		LogError("Failed [{}] tests", failed_count);
		std::exit(1);
	}
	else {
		LogInfo("All tests passed");
	}
}
