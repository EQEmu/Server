#include "../../common/http/httplib.h"
#include "../../common/eqemu_logsys.h"
#include "../../common/platform.h"
#include "../zone.h"
#include "../client.h"
#include "../../common/net/eqstream.h"

extern Zone *zone;

void ZoneCLI::NpcHandins(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	if (cmd[{"-h", "--help"}]) {
		return;
	}

	uint32 break_length = 50;
	int failed_count = 0;

	RegisterExecutablePlatform(EQEmuExePlatform::ExePlatformZoneSidecar);

	LogInfo("{}", Strings::Repeat("-", break_length));
	LogInfo("Booting test zone for NPC handins");
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
				.description = "Test basic cloth-cap hand-in",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "1001", .count = 1},
					},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "1001", .count = 1},
					},
				},
				.returned = {},
				.handin_check_result = true,
			},
			TestCase{
				.description = "Test basic cloth-cap hand-in failure",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "9997", .count = 1},
					},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "1001", .count = 1},
					},
				},
				.returned = {
					.items = {
						HandinEntry{.item_id = "9997", .count = 1},
					},
				},
				.handin_check_result = false,
			},
			TestCase{
				.description = "Test basic cloth-cap hand-in failure from handing in too many",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "9997", .count = 1},
						HandinEntry{.item_id = "9997", .count = 1},
					},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "1001", .count = 1},
					},
				},
				.returned = {
					.items = {
						HandinEntry{.item_id = "9997", .count = 1},
						HandinEntry{.item_id = "9997", .count = 1},
					},
				},
				.handin_check_result = false,
			},
			TestCase{
				.description = "Test handing in money",
				.hand_in = {
					.items = {},
					.money = {.platinum = 1},
				},
				.required = {
					.items = {},
					.money = {.platinum = 1},
				},
				.returned = {},
				.handin_check_result = true,
			},
			TestCase{
				.description = "Test handing in money, but not enough",
				.hand_in = {
					.items = {},
					.money = {.platinum = 1},
				},
				.required = {
					.items = {},
					.money = {.platinum = 100},
				},
				.returned = {},
				.handin_check_result = false,
			},
			TestCase{
				.description = "Test handing in money, but not enough of any type",
				.hand_in = {
					.items = {},
					.money = {.platinum = 1, .gold = 1, .silver = 1, .copper = 1},
				},
				.required = {
					.items = {},
					.money = {.platinum = 100, .gold = 100, .silver = 100, .copper = 100},
				},
				.returned = {},
				.handin_check_result = false,
			},
			TestCase{
				.description = "Test handing in money of all types",
				.hand_in = {
					.items = {},
					.money = {.platinum = 1, .gold = 1, .silver = 1, .copper = 1},
				},
				.required = {
					.items = {},
					.money = {.platinum = 1, .gold = 1, .silver = 1, .copper = 1},
				},
				.returned = {},
				.handin_check_result = true,
			},
			TestCase{
				.description = "Test handing in platinum with items with success",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "1001", .count = 1},
					},
					.money = {.platinum = 1},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "1001", .count = 1},
					},
					.money = {.platinum = 1},
				},
				.returned = {},
				.handin_check_result = true,
			},
			TestCase{
				.description = "Test handing in platinum with items with failure",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "1001", .count = 1},
					},
					.money = {.platinum = 1},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "1001", .count = 1},
					},
					.money = {.platinum = 100},
				},
				.returned = {
					.items = {
						HandinEntry{.item_id = "1001", .count = 1},
					},
				},
				.handin_check_result = false,
			},
			TestCase{
				.description = "Test returning money and items",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "1007", .count = 1},
					},
					.money = {
						.platinum = 1,
						.gold    = 666,
						.silver  = 234,
						.copper  = 444,
					},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "1001", .count = 1},
					},
					.money = {.platinum = 100},
				},
				.returned = {
					.items = {
						HandinEntry{.item_id = "1007", .count = 1},
					},
					.money = {
						.platinum = 1,
						.gold    = 666,
						.silver  = 234,
						.copper  = 444,
					},
				},
				.handin_check_result = false,
			},
			TestCase{
				.description = "Test returning money",
				.hand_in = {
					.items = {},
					.money = {
						.platinum = 1,
						.gold    = 666,
						.silver  = 234,
						.copper  = 444,
					},
				},
				.required = {
					.items = {},
					.money = {.platinum = 100},
				},
				.returned = {
					.items = {
					},
					.money = {
						.platinum = 1,
						.gold    = 666,
						.silver  = 234,
						.copper  = 444,
					},
				},
				.handin_check_result = false,
			},
			TestCase{
				.description = "Test handing in many items of the same required item",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "1007", .count = 1},
						HandinEntry{.item_id = "1007", .count = 1},
						HandinEntry{.item_id = "1007", .count = 1},
						HandinEntry{.item_id = "1007", .count = 1},
					},
					.money = {},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "1007", .count = 1},
					},
					.money = {},
				},
				.returned = {
					.items = {
						HandinEntry{.item_id = "1007", .count = 1},
						HandinEntry{.item_id = "1007", .count = 1},
						HandinEntry{.item_id = "1007", .count = 1},
					},
					.money = {
						.platinum = 1,
						.gold    = 666,
						.silver  = 234,
						.copper  = 444,
					},
				},
				.handin_check_result = true,
			},
			TestCase{
				.description = "Test handing in item of a stack",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "13005", .count = 20},
					},
					.money = {},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "13005", .count = 20},
					},
					.money = {},
				},
				.returned = {
					.items = {},
					.money = {},
				},
				.handin_check_result = true,
			},
			TestCase{
				.description = "Test handing in item of a stack but not enough",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "13005", .count = 10},
					},
					.money = {},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "13005", .count = 20},
					},
					.money = {},
				},
				.returned = {
					.items = {
						HandinEntry{.item_id = "13005", .count = 10},
					},
					.money = {},
				},
				.handin_check_result = false,
			},
			TestCase{
				.description = "Test handing in 4 non-stacking helmets when 4 are required",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "29062", .count = 1},
						HandinEntry{.item_id = "29062", .count = 1},
						HandinEntry{.item_id = "29062", .count = 1},
						HandinEntry{.item_id = "29062", .count = 1},
					},
					.money = {},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "29062", .count = 4},
					},
					.money = {},
				},
				.returned = {
					.items = {
					},
					.money = {},
				},
				.handin_check_result = true,
			},
			TestCase{
				.description = "Test handing in Soulfire that has 5 charges and have it count as 1 item",
				.hand_in = {
					.items = {
						HandinEntry{.item_id = "5504", .count = 1},
					},
					.money = {},
				},
				.required = {
					.items = {
						HandinEntry{.item_id = "5504", .count = 1},
					},
					.money = {},
				},
				.returned = {
					.items = {
					},
					.money = {},
				},
				.handin_check_result = true,
			},
		};

		std::map<std::string, uint32>         hand_ins;
		std::map<std::string, uint32>         required;
		std::vector<EQ::ItemInstance *> items;

		LogSys.EnableConsoleLogging();

		// turn this on to see debugging output
		LogSys.log_settings[Logs::NpcHandin].log_to_console = std::getenv("DEBUG") ? 3 : 0;

		LogInfo("{}", Strings::Repeat("-", break_length));

		for (auto &test_case: test_cases) {
			hand_ins.clear();
			required.clear();
			items.clear();

			for (auto &hand_in: test_case.hand_in.items) {
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
				for (auto &hand_in: hand_ins) {
					LogError(" > Item [{}] count [{}]", hand_in.first, hand_in.second);
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
