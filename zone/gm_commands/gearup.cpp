#include "../client.h"
#include "../../common/http/httplib.h"
#include "../../common/content/world_content_service.h"

void command_gearup(Client *c, const Seperator *sep)
{
	std::string tool_table_name = "tool_gearup_armor_sets";
	if (!database.DoesTableExist(tool_table_name)) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"Table [{}] does not exist. Downloading from Github and installing...",
				tool_table_name
			).c_str()
		);

		// http get request
		httplib::Client cli("https://raw.githubusercontent.com");
		cli.set_connection_timeout(0, 15000000); // 15 sec
		cli.set_read_timeout(15, 0); // 15 seconds
		cli.set_write_timeout(15, 0); // 15 seconds

		int         sourced_queries = 0;
		std::string url             = "/EQEmu/Server/master/utils/sql/git/optional/2020_07_20_tool_gearup_armor_sets.sql";

		if (auto res = cli.Get(url.c_str())) {
			if (res->status == 200) {
				for (auto &s: SplitString(res->body, ';')) {
					if (!trim(s).empty()) {
						auto results = database.QueryDatabase(s);
						if (!results.ErrorMessage().empty()) {
							c->Message(
								Chat::Yellow,
								fmt::format(
									"Error sourcing SQL [{}]", results.ErrorMessage()
								).c_str()
							);
							return;
						}
						sourced_queries++;
					}
				}
			}
		}
		else {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"Error retrieving URL [{}]",
					url
				).c_str()
			);
		}

		c->Message(
			Chat::Yellow,
			fmt::format(
				"Table [{}] installed. Sourced [{}] queries",
				tool_table_name, sourced_queries
			).c_str()
		);
	}

	std::string expansion_arg = sep->arg[1];
	std::string expansion_filter;
	if (expansion_arg.length() > 0) {
		expansion_filter = fmt::format("and `expansion` = {}", expansion_arg);
	}

	auto results = database.QueryDatabase(
		fmt::format(
			SQL (
				select
				item_id,
				slot
					from
					{}
					where
					`class` = {}
					and `level` = {}
				{}
					order by score desc, expansion desc
			),
			tool_table_name,
			c->GetClass(),
			c->GetLevel(),
			expansion_filter
		)
	);

	int           items_equipped     = 0;
	int           items_already_have = 0;
	std::set<int> equipped;

	for (auto row = results.begin(); row != results.end(); ++row) {
		int item_id = atoi(row[0]);
		int slot_id = atoi(row[1]);

		if (equipped.find(slot_id) != equipped.end()) {
			if (slot_id == EQ::invslot::slotEar1) {
				slot_id = EQ::invslot::slotEar2;
			}
			if (slot_id == EQ::invslot::slotFinger1) {
				slot_id = EQ::invslot::slotFinger2;
			}
			if (slot_id == EQ::invslot::slotWrist1) {
				slot_id = EQ::invslot::slotWrist2;
			}
		}

		if (equipped.find(slot_id) == equipped.end()) {
			const EQ::ItemData *item         = database.GetItem(item_id);
			bool               has_item      = (c->GetInv().HasItem(item_id, 1, invWhereWorn) != INVALID_INDEX);
			bool               can_wear_item = !c->CheckLoreConflict(item) && !has_item;
			if (!can_wear_item) {
				items_already_have++;
			}

			if (c->CastToMob()->CanClassEquipItem(item_id) && can_wear_item) {
				equipped.insert(slot_id);
				c->SummonItem(
					item_id,
					0, 0, 0, 0, 0, 0, 0, 0,
					slot_id
				);
				items_equipped++;
			}
		}
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Equipped items [{}] already had [{}] items equipped",
			items_equipped,
			items_already_have
		).c_str()
	);

	if (expansion_arg.empty()) {
		results         = database.QueryDatabase(
			fmt::format(
				SQL (
					select
						expansion
					from
					{}
					where
					class = {}
					and level = {}
					group by
					expansion;
				),
				tool_table_name,
				c->GetClass(),
				c->GetLevel()
			)
		);

		c->Message(Chat::White, "Choose armor from a specific era");
		std::string message;
		for (auto   row = results.begin(); row != results.end(); ++row) {
			int expansion = atoi(row[0]);
			message += "[" + EQ::SayLinkEngine::GenerateQuestSaylink(
				fmt::format("#gearup {}", expansion),
				false,
				Expansion::ExpansionName[expansion]
			) + "] ";

			if (message.length() > 2000) {
				c->Message(Chat::White, message.c_str());
				message = "";
			}
		}
		if (message.length() > 0) {
			c->Message(Chat::White, message.c_str());
		}
	}

}

