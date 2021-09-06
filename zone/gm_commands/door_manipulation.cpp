#include "door_manipulation.h"
#include "../doors.h"
#include "../../common/repositories/tool_game_objects_repository.h"

void DoorManipulation::CommandHandler(Client *c, const Seperator *sep)
{
	// this should never happen
	if (!c) {
		return;
	}

	// args
	std::string arg1(sep->arg[1]);
	std::string arg2(sep->arg[2]);
	std::string arg3(sep->arg[3]);

	// table check
	std::string table_name = "tool_game_objects";
	std::string url        = "https://raw.githubusercontent.com/EQEmu/database-tool-sqls/main/tool_game_objects.sql";
	if (!database.DoesTableExist(table_name)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Table [{}] does not exist. Downloading from [{}] and installing locally",
				table_name,
				url
			).c_str()
		);
		database.SourceDatabaseTableFromUrl(
			table_name,
			url
		);
	}

	// option
	if (arg1.empty()) {
		DoorManipulation::CommandHeader(c);
		c->Message(Chat::White, "#door setincline <incline> | Sets selected door incline");
		c->Message(Chat::White, "#door opentype <opentype> | Sets selected door opentype");
		c->Message(Chat::White, "#door model <modelname> | Changes door model for selected door");
		c->Message(Chat::White, "#door save | Creates database entry for highlighted door");
		c->Message(
			Chat::White,
			fmt::format(
				"{} - lists doors in zone",
				EQ::SayLinkEngine::GenerateQuestSaylink("#door list", false, "#door list")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"{} - Brings up editing interface for selected door",
				EQ::SayLinkEngine::GenerateQuestSaylink("#door edit", false, "#door edit")
			).c_str()
		);;
		c->Message(
			Chat::White,
			// "#door model <modelname> or select from " . quest::saylink("#door showmodelszone", 1, "Local Zone") . " " . quest::saylink("#door showmodelsglobal", 1, "Global")
			fmt::format(
				"{} - Brings up editing interface for selected door",
				EQ::SayLinkEngine::GenerateQuestSaylink("#door showmodelszone", false, "#door showmodelszone")
			).c_str()
		);

		return;
	}

	if (arg1 == "edit") {
		Doors *door = entity_list.GetDoorsByID(c->GetDoorToolEntityId());
		if (door) {
			c->Message(
				Chat::White,
				fmt::format(
					"Door Selected ID [{}] Name [{}] OpenType [{}] Invertstate [{}]",
					c->GetDoorToolEntityId(),
					door->GetDoorName(),
					door->GetOpenType(),
					door->GetInvertState()
				).c_str()
			);

			const std::string move_x_action   = "move_x";
			const std::string move_y_action   = "move_y";
			const std::string move_z_action   = "move_z";
			const std::string move_h_action   = "move_h";
			const std::string set_size_action = "set_size";

			std::vector<std::string> move_options = {
				move_x_action,
				move_y_action,
				move_z_action,
				move_h_action,
				set_size_action
			};
			std::vector<std::string> move_x_options_positive;
			std::vector<std::string> move_x_options_negative;
			std::vector<std::string> move_y_options_positive;
			std::vector<std::string> move_y_options_negative;
			std::vector<std::string> move_z_options_positive;
			std::vector<std::string> move_z_options_negative;
			std::vector<std::string> move_h_options_positive;
			std::vector<std::string> move_h_options_negative;
			std::vector<std::string> set_size_options_positive;
			std::vector<std::string> set_size_options_negative;
			for (const auto          &move_option : move_options) {
				if (move_option == move_x_action) {
					move_x_options_positive.emplace_back(
						EQ::SayLinkEngine::GenerateQuestSaylink(
							fmt::format("#door edit {} .25", move_option),
							false,
							".25"
						)
					);

					for (int move_index = 0; move_index <= 15; move_index += 5) {
						int value = (move_index == 0 ? 1 : move_index);
						move_x_options_positive.emplace_back(
							EQ::SayLinkEngine::GenerateQuestSaylink(
								fmt::format("#door edit {} {}", move_option, value),
								false,
								fmt::format("{}", std::abs(value))
							)
						);
					}

					for (int move_index = -15; move_index <= 0; move_index += 5) {
						int value = (move_index == 0 ? 1 : move_index);
						move_x_options_negative.emplace_back(
							EQ::SayLinkEngine::GenerateQuestSaylink(
								fmt::format("#door edit {} {}", move_option, value),
								false,
								fmt::format("{}", std::abs(value))
							)
						);
					}

					move_x_options_negative.emplace_back(
						EQ::SayLinkEngine::GenerateQuestSaylink(
							fmt::format("#door edit {} -.25", move_option),
							false,
							"-.25"
						)
					);
				}
				else if (move_option == move_y_action) {
					move_y_options_positive.emplace_back(
						EQ::SayLinkEngine::GenerateQuestSaylink(
							fmt::format("#door edit {} .25", move_option),
							false,
							".25"
						)
					);

					for (int move_index = 0; move_index <= 15; move_index += 5) {
						int value = (move_index == 0 ? 1 : move_index);
						move_y_options_positive.emplace_back(
							EQ::SayLinkEngine::GenerateQuestSaylink(
								fmt::format("#door edit {} {}", move_option, value),
								false,
								fmt::format("{}", std::abs(value))
							)
						);
					}

					for (int move_index = -15; move_index <= 0; move_index += 5) {
						int value = (move_index == 0 ? 1 : move_index);
						move_y_options_negative.emplace_back(
							EQ::SayLinkEngine::GenerateQuestSaylink(
								fmt::format("#door edit {} {}", move_option, value),
								false,
								fmt::format("{}", std::abs(value))
							)
						);
					}

					move_y_options_negative.emplace_back(
						EQ::SayLinkEngine::GenerateQuestSaylink(
							fmt::format("#door edit {} -.25", move_option),
							false,
							"-.25"
						)
					);
				}
				else if (move_option == move_z_action) {
					move_z_options_positive.emplace_back(
						EQ::SayLinkEngine::GenerateQuestSaylink(
							fmt::format("#door edit {} .25", move_option),
							false,
							".25"
						)
					);

					for (int move_index = 0; move_index <= 15; move_index += 5) {
						int value = (move_index == 0 ? 1 : move_index);
						move_z_options_positive.emplace_back(
							EQ::SayLinkEngine::GenerateQuestSaylink(
								fmt::format("#door edit {} {}", move_option, value),
								false,
								fmt::format("{}", std::abs(value))
							)
						);
					}

					for (int move_index = -15; move_index <= 0; move_index += 5) {
						int value = (move_index == 0 ? 1 : move_index);
						move_z_options_negative.emplace_back(
							EQ::SayLinkEngine::GenerateQuestSaylink(
								fmt::format("#door edit {} {}", move_option, value),
								false,
								fmt::format("{}", std::abs(value))
							)
						);
					}

					move_z_options_negative.emplace_back(
						EQ::SayLinkEngine::GenerateQuestSaylink(
							fmt::format("#door edit {} -.25", move_option),
							false,
							"-.25"
						)
					);
				}
				else if (move_option == move_h_action) {
					for (int move_index = 0; move_index <= 50; move_index += 5) {
						int value = (move_index == 0 ? 1 : move_index);
						move_h_options_positive.emplace_back(
							EQ::SayLinkEngine::GenerateQuestSaylink(
								fmt::format("#door edit {} {}", move_option, value),
								false,
								fmt::format("{}", std::abs(value))
							)
						);
					}

					for (int move_index = -50; move_index <= 0; move_index += 5) {
						int value = (move_index == 0 ? 1 : move_index);
						move_h_options_negative.emplace_back(
							EQ::SayLinkEngine::GenerateQuestSaylink(
								fmt::format("#door edit {} {}", move_option, value),
								false,
								fmt::format("{}", std::abs(value))
							)
						);
					}
				}
				else if (move_option == set_size_action) {
					for (int move_index = 0; move_index <= 100; move_index += 10) {
						int value = (move_index == 0 ? 1 : move_index);
						set_size_options_positive.emplace_back(
							EQ::SayLinkEngine::GenerateQuestSaylink(
								fmt::format("#door edit {} {}", move_option, value),
								false,
								fmt::format("{}", std::abs(value))
							)
						);
					}

					for (int move_index = -100; move_index <= 0; move_index += 10) {
						int value = (move_index == 0 ? 1 : move_index);
						set_size_options_negative.emplace_back(
							EQ::SayLinkEngine::GenerateQuestSaylink(
								fmt::format("#door edit {} {}", move_option, value),
								false,
								fmt::format("{}", std::abs(value))
							)
						);
					}
				}
			}

			// we're passing a move action here
			if (!arg3.empty() && StringIsNumber(arg3)) {
				int x_move   = 0;
				int y_move   = 0;
				int z_move   = 0;
				int h_move   = 0;
				int set_size = 0;

				if (arg2 == move_x_action) {
					x_move = std::atoi(arg3.c_str());
				}
				if (arg2 == move_y_action) {
					y_move = std::atoi(arg3.c_str());
				}
				if (arg2 == move_z_action) {
					z_move = std::atoi(arg3.c_str());
				}
				if (arg2 == move_h_action) {
					h_move = std::atoi(arg3.c_str());
				}
				if (arg2 == set_size_action) {
					set_size = std::atoi(arg3.c_str());
				}

				door->SetLocation(
					door->GetX() + x_move,
					door->GetY() + y_move,
					door->GetZ() + z_move
				);

				glm::vec4 door_position = door->GetPosition();
				door_position.w = door_position.w + h_move;
				door->SetPosition(door_position);
				door->SetSize(door->GetSize() + set_size);
			}

			// spawn and move helpers
			uint16 helper_mob_x_negative = 0;
			uint16 helper_mob_x_positive = 0;
			uint16 helper_mob_y_positive = 0;
			uint16 helper_mob_y_negative = 0;

			for (auto &n: entity_list.GetNPCList()) {
				NPC * npc = n.second;
				std::string npc_name = npc->GetName();
				if (npc_name.find("-X") != std::string::npos) {
					helper_mob_x_negative = npc->GetID();
				}
				if (npc_name.find("-Y") != std::string::npos) {
					helper_mob_y_negative = npc->GetID();
				}
				if (npc_name.find("+X") != std::string::npos) {
					helper_mob_x_positive = npc->GetID();
				}
				if (npc_name.find("+Y") != std::string::npos) {
					helper_mob_y_positive = npc->GetID();
				}
			}

			// -X
			glm::vec4 door_position = door->GetPosition();
			if (helper_mob_x_negative == 0) {
				door_position.x = door_position.x - 15;
				auto node = NPC::SpawnNodeNPC("-X", "", door_position);
				helper_mob_x_negative = node->GetID();
			} else {
				auto n = entity_list.GetNPCByID(helper_mob_x_negative);
				n->GMMove(door->GetX() - 15, door->GetY(), door->GetZ(), n->GetHeading());
			}

			// +X
			door_position = door->GetPosition();
			if (helper_mob_x_positive == 0) {
				door_position.x = door_position.x + 15;
				auto node = NPC::SpawnNodeNPC("+X", "", door_position);
				helper_mob_x_positive = node->GetID();
			} else {
				auto n = entity_list.GetNPCByID(helper_mob_x_positive);
				n->GMMove(door->GetX() + 15, door->GetY(), door->GetZ(), n->GetHeading());
			}

			// -Y
			door_position = door->GetPosition();
			if (helper_mob_y_negative == 0) {
				door_position.y = door_position.y - 15;
				auto node = NPC::SpawnNodeNPC("-Y", "", door_position);
				helper_mob_y_negative = node->GetID();
			} else {
				auto n = entity_list.GetNPCByID(helper_mob_y_negative);
				n->GMMove(door->GetX(), door->GetY() - 15, door->GetZ(), n->GetHeading());
			}

			// +Y
			door_position = door->GetPosition();
			if (helper_mob_y_positive == 0) {
				door_position.y = door_position.y + 15;
				auto node = NPC::SpawnNodeNPC("+Y", "", door_position);
				helper_mob_y_positive = node->GetID();
			} else {
				auto n = entity_list.GetNPCByID(helper_mob_y_positive);
				n->GMMove(door->GetX(), door->GetY() + 15, door->GetZ(), n->GetHeading());
			}

			c->Message(
				Chat::White,
				fmt::format(
					"Name [{}] [{}] [{}] [{}]",
					door->GetDoorName(),
					EQ::SayLinkEngine::GenerateQuestSaylink(
						"#door save",
						false,
						"Save"
					),
					EQ::SayLinkEngine::GenerateQuestSaylink(
						"#door changemodelqueue",
						false,
						"Change Model"
					),
					EQ::SayLinkEngine::GenerateQuestSaylink(
						"#door setinclineinc",
						false,
						"Incline"
					)
				).c_str()
			);
			c->Message(
				Chat::White,
				fmt::format(
					"[{}] - [X] + [{}]",
					implode(" | ", move_x_options_negative),
					implode(" | ", move_x_options_positive)
				).c_str()
			);
			c->Message(
				Chat::White,
				fmt::format(
					"[{}] - [Y] + [{}]",
					implode(" | ", move_y_options_negative),
					implode(" | ", move_y_options_positive)
				).c_str()
			);
			c->Message(
				Chat::White,
				fmt::format(
					"[{}] - [Z] + [{}]",
					implode(" | ", move_z_options_negative),
					implode(" | ", move_z_options_positive)
				).c_str()
			);
			c->Message(
				Chat::White,
				fmt::format(
					"[{}] - [H] + [{}]",
					implode(" | ", move_h_options_negative),
					implode(" | ", move_h_options_positive)
				).c_str()
			);
			c->Message(
				Chat::White,
				fmt::format(
					"[{}] - [Size] + [{}]",
					implode(" | ", set_size_options_negative),
					implode(" | ", set_size_options_positive)
				).c_str()
			);

			return;
		}

		c->Message(Chat::Red, "Door selection invalid...");
	}

	//	if (arg1 == "showmodelszone") {
	//		auto game_objects = ToolGameObjectsRepository::GetWhere(
	//			database,
	//			fmt::format("zoneid = {}", zone->GetZoneID())
	//		);
	//
	//		if (game_objects.empty()) {
	//			c->Message(Chat::White, "There are no models for this zone...");
	//		}
	//
	//		for (auto &g: game_objects) {
	//			c->Message(Chat::White, g.object_name.c_str());
	//		}
	//	}

	if (arg1 == "create") {
		std::string model     = str_toupper(arg2);
		uint16      entity_id = entity_list.CreateDoor(
			model.c_str(),
			c->GetPosition(),
			58,
			100
		);

		c->Message(
			Chat::White,
			fmt::format("Creating door entity_id [{}] with model [{}]", entity_id, model).c_str());
		c->SetDoorToolEntityId(entity_id);
	}

	if (arg1 == "model") {
		Doors       *door = entity_list.GetDoorsByID(c->GetDoorToolEntityId());
		std::string model = str_toupper(arg2);
		if (door) {
			door->SetDoorName(model.c_str());
		}
	}

	if (arg1 == "showmodelszone") {
		auto game_objects = ToolGameObjectsRepository::GetWhere(
			database,
			fmt::format("zoneid = {}", zone->GetZoneID())
		);

		if (game_objects.empty()) {
			c->Message(Chat::White, "There are no models for this zone...");
		}

		c->Message(Chat::White, "------------------------------------------------");
		c->Message(Chat::White, "# Doors from zone");
		c->Message(Chat::White, "------------------------------------------------");

		std::vector<std::string> objects;
		for (auto                &g: game_objects) {
			objects.emplace_back(g.object_name);
		}

		int                      character_length = 0;
		std::vector<std::string> object_names;
		for (auto                &o: objects) {
			character_length += o.length();
			object_names.emplace_back(o);

			if (character_length > 500) {
				std::string message_buffer;
				for (auto   &object_name: object_names) {
					message_buffer += fmt::format(
						"[{}] ",
						EQ::SayLinkEngine::GenerateQuestSaylink(
							fmt::format("#door model {}", object_name),
							false,
							object_name
						)
					);
				}

				c->Message(Chat::White, message_buffer.c_str());

				character_length = 0;
				object_names     = {};
			}
		}

		std::string message_buffer;
		for (auto   &object_name: object_names) {
			message_buffer += fmt::format(
				"[{}] ",
				EQ::SayLinkEngine::GenerateQuestSaylink(
					fmt::format("#door model {}", object_name),
					false,
					object_name
				)
			);
		}

		c->Message(Chat::White, message_buffer.c_str());


	}

}

void DoorManipulation::CommandHeader(Client *c)
{
	c->Message(Chat::White, "------------------------------------------------");
	c->Message(Chat::White, "# Door Commands");
	c->Message(Chat::White, "------------------------------------------------");
}
