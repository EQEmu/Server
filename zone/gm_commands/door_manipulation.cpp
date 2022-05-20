#include "door_manipulation.h"
#include "../doors.h"
#include "../../common/misc_functions.h"

#define MAX_CLIENT_MESSAGE_LENGTH 2000

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
		c->Message(
			Chat::White,
			"#door create <modelname> | Creates a door from a model. (Example IT78 creates a campfire)"
		);
		c->Message(Chat::White, "#door setinvertstate [0|1] | Sets selected door invert state");
		c->Message(Chat::White, "#door setincline <incline> | Sets selected door incline");
		c->Message(Chat::White, "#door opentype <opentype> | Sets selected door opentype");
		c->Message(
			Chat::White,
			fmt::format(
				"#door model <modelname> | Changes door model for selected door or select from [{}] or [{}]",
				EQ::SayLinkEngine::GenerateQuestSaylink("#door showmodelszone", false, "local zone"),
				EQ::SayLinkEngine::GenerateQuestSaylink("#door showmodelsglobal", false, "global")
			).c_str()
		);
		c->Message(
			Chat::White,
			"#door showmodelsfromfile <file.eqg|file.s3d> | Shows models from s3d or eqg file. Example tssequip.eqg or wallet01.eqg"
		);

		c->Message(
			Chat::White,
			fmt::format(
				"{} | Shows available models in the current zone that you are in",
				EQ::SayLinkEngine::GenerateQuestSaylink("#door showmodelszone", false, "#door showmodelszone")
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"{} | Shows available models globally by first listing all global model files",
				EQ::SayLinkEngine::GenerateQuestSaylink("#door showmodelsglobal", false, "#door showmodelsglobal")
			).c_str()
		);

		c->Message(Chat::White, "#door save | Creates database entry for selected door");
		c->Message(
			Chat::White,
			fmt::format(
				"{} - Brings up editing interface for selected door",
				EQ::SayLinkEngine::GenerateQuestSaylink("#door edit", false, "#door edit")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"{} - lists doors in zone",
				EQ::SayLinkEngine::GenerateQuestSaylink("#list doors", false, "#list doors")
			).c_str()
		);

		return;
	}

	// edit menu
	if (arg1 == "edit") {
		Doors *door = entity_list.GetDoorsByID(c->GetDoorToolEntityId());
		if (door) {
			c->Message(
				Chat::White,
				fmt::format(
					"Door Selected ID [{}] Name [{}] OpenType [{}] Invertstate [{} | {}/{}] ",
					c->GetDoorToolEntityId(),
					door->GetDoorName(),
					door->GetOpenType(),
					door->GetInvertState(),
					EQ::SayLinkEngine::GenerateQuestSaylink("#door setinvertstate 0", false, "0"),
					EQ::SayLinkEngine::GenerateQuestSaylink("#door setinvertstate 1", false, "1")
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
							".25"
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
						int value = (move_index == 0 ? -1 : move_index);
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
							".25"
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
						int value = (move_index == 0 ? -1 : move_index);
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
							".25"
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
						int value = (move_index == 0 ? -1 : move_index);
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
						int value = (move_index == 0 ? -1 : move_index);
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
				float x_move   = 0.0f;
				float y_move   = 0.0f;
				float z_move   = 0.0f;
				float h_move   = 0.0f;
				float set_size = 0.0f;

				if (arg2 == move_x_action) {
					x_move = std::atof(arg3.c_str());
				}

				if (arg2 == move_y_action) {
					y_move = std::atof(arg3.c_str());
				}

				if (arg2 == move_z_action) {
					z_move = std::atof(arg3.c_str());
				}

				if (arg2 == move_h_action) {
					h_move = std::atof(arg3.c_str());
				}

				if (arg2 == set_size_action) {
					set_size = std::atof(arg3.c_str());
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
				NPC         *npc     = n.second;
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
				helper_mob_x_negative = NPC::SpawnNodeNPC("-X", "", door_position)->GetID();
			}
			else {
				auto n = entity_list.GetNPCByID(helper_mob_x_negative);
				n->GMMove(door->GetX() - 15, door->GetY(), door->GetZ(), n->GetHeading());
			}

			// +X
			door_position = door->GetPosition();
			if (helper_mob_x_positive == 0) {
				door_position.x = door_position.x + 15;
				helper_mob_x_positive = NPC::SpawnNodeNPC("+X", "", door_position)->GetID();
			}
			else {
				auto n = entity_list.GetNPCByID(helper_mob_x_positive);
				n->GMMove(door->GetX() + 15, door->GetY(), door->GetZ(), n->GetHeading());
			}

			// -Y
			door_position = door->GetPosition();
			if (helper_mob_y_negative == 0) {
				door_position.y = door_position.y - 15;
				helper_mob_y_negative = NPC::SpawnNodeNPC("-Y", "", door_position)->GetID();
			}
			else {
				auto n = entity_list.GetNPCByID(helper_mob_y_negative);
				n->GMMove(door->GetX(), door->GetY() - 15, door->GetZ(), n->GetHeading());
			}

			// +Y
			door_position = door->GetPosition();
			if (helper_mob_y_positive == 0) {
				door_position.y = door_position.y + 15;
				helper_mob_y_positive = NPC::SpawnNodeNPC("+Y", "", door_position)->GetID();
			}
			else {
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

	// create
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

	// set model
	if (arg1 == "model") {
		Doors       *door = entity_list.GetDoorsByID(c->GetDoorToolEntityId());
		std::string model = str_toupper(arg2);
		if (door) {
			door->SetDoorName(model.c_str());
		}
	}

	// change model queue
	if (arg1 == "changemodelqueue") {
		c->Message(
			Chat::White,
			fmt::format(
				"#door model <modelname> | Changes door model for selected door or select from [{}] or [{}]",
				EQ::SayLinkEngine::GenerateQuestSaylink("#door showmodelszone", false, "local zone"),
				EQ::SayLinkEngine::GenerateQuestSaylink("#door showmodelsglobal", false, "global")
			).c_str()
		);
	}

	// open type
	if (arg1 == "opentype" && !arg2.empty() && StringIsNumber(arg2)) {
		Doors *door = entity_list.GetDoorsByID(c->GetDoorToolEntityId());
		if (door) {
			door->SetOpenType(std::atoi(arg2.c_str()));
		}
	}

	// incline
	if (arg1 == "setincline" && !arg2.empty() && StringIsNumber(arg2)) {
		Doors *door = entity_list.GetDoorsByID(c->GetDoorToolEntityId());
		if (door) {
			door->SetIncline(std::atoi(arg2.c_str()));
		}
	}

	// incline
	if (arg1 == "setinvertstate" && !arg2.empty() && StringIsNumber(arg2)) {
		Doors *door = entity_list.GetDoorsByID(c->GetDoorToolEntityId());
		if (door) {
			door->SetInvertState(std::atoi(arg2.c_str()));
		}
	}

	// save
	if (arg1 == "save") {
		Doors *door = entity_list.GetDoorsByID(c->GetDoorToolEntityId());
		if (door) {
			door->CreateDatabaseEntry();
			c->Message(Chat::White, "Door saved");
		}
	}

	// incline incremental
	if (arg1 == "setinclineinc" && !arg2.empty() && StringIsNumber(arg2)) {
		Doors *door = entity_list.GetDoorsByID(c->GetDoorToolEntityId());
		if (door) {
			door->SetIncline(door->GetIncline() + std::atoi(arg2.c_str()));
		}
	}
	if (arg1 == "setinclineinc") {
		std::map<float, std::string> incline_values = {
			{.01,    "Upright"},
			{63.75,  "45 Degrees",},
			{130,    "90 Degrees"},
			{192.5,  "135 Degrees"},
			{255,    "180 Degrees"},
			{321.25, "225 Degrees"},
			{385,    "270 Degrees"},
			{448.75, "315 Degrees"},
			{512.5,  "360 Degrees"}
		};

		std::vector<std::string> incline_normal_options;
		std::vector<std::string> incline_positive_options;
		std::vector<std::string> incline_negative_options;
		for (auto                incline_value : incline_values) {
			incline_normal_options.emplace_back(
				EQ::SayLinkEngine::GenerateQuestSaylink(
					fmt::format(
						"#door setincline {}",
						incline_value.first
					),
					false,
					incline_value.second
				)
			);
		}

		for (int incline_index = 0; incline_index <= 100; incline_index += 10) {
			int incline_value = (incline_index == 0 ? 1 : incline_index);
			incline_positive_options.emplace_back(
				EQ::SayLinkEngine::GenerateQuestSaylink(
					fmt::format(
						"#door setinclineinc {}",
						incline_value
					),
					false,
					itoa(std::abs(incline_value))
				)
			);
		}

		for (int incline_index = -100; incline_index <= 1; incline_index += 10) {
			int incline_value = (incline_index == 0 ? -1 : incline_index);
			incline_negative_options.emplace_back(
				EQ::SayLinkEngine::GenerateQuestSaylink(
					fmt::format(
						"#door setinclineinc {}",
						incline_value
					),
					false,
					itoa(std::abs(incline_value))
				)
			);
		}

		c->Message(
			Chat::White,
			fmt::format(
				"[Incline] [{}]",
				implode(" | ", incline_normal_options)
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"[Incline Increments] [{}] - | + [{}]",
				implode(" | ", incline_negative_options),
				implode(" | ", incline_positive_options)
			).c_str()
		);
	}

	// show models in zone
	if (arg1 == "showmodelsglobal") {
		auto game_objects = ToolGameObjectsRepository::GetWhere(
			database,
			"object_name LIKE '%IT%' AND zoneid = 0 AND object_name NOT LIKE '%OBJ%' GROUP by file_from"
		);

		if (game_objects.empty()) {
			c->Message(Chat::White, "There are no models to display...");
		}

		c->Message(Chat::White, "------------------------------------------------");
		c->Message(Chat::White, "# Models (Global)");
		c->Message(Chat::White, "------------------------------------------------");

		DisplayModelsFromFileResults(c, game_objects);
	}

	// show models in zone
	if (arg1 == "showmodelszone") {
		auto game_objects = ToolGameObjectsRepository::GetWhere(
			database,
			fmt::format("zoneid = {}", zone->GetZoneID())
		);

		if (game_objects.empty()) {
			c->Message(Chat::White, "There are no models for this zone...");
		}

		c->Message(Chat::White, "------------------------------------------------");
		c->Message(Chat::White, "# Models from zone");
		c->Message(Chat::White, "------------------------------------------------");

		DisplayObjectResultToClient(c, game_objects);
	}

	// show models from file name
	if (arg1 == "showmodelsfromfile" && !arg2.empty()) {
		const std::string &file_name   = arg2;
		auto              game_objects = ToolGameObjectsRepository::GetWhere(
			database,
			fmt::format("file_from = '{}'", file_name)
		);

		if (game_objects.empty()) {
			c->Message(Chat::White, "There are no models for this zone...");
		}

		c->Message(Chat::White, "------------------------------------------------");
		c->Message(Chat::White, fmt::format("# Models from file name [{}]", file_name).c_str());
		c->Message(Chat::White, "------------------------------------------------");

		DisplayObjectResultToClient(c, game_objects);
	}
}

void DoorManipulation::CommandHeader(Client *c)
{
	c->Message(Chat::White, "------------------------------------------------");
	c->Message(Chat::White, "# Door Commands");
	c->Message(Chat::White, "------------------------------------------------");
}

void DoorManipulation::DisplayObjectResultToClient(
	Client *c,
	std::vector<ToolGameObjectsRepository::ToolGameObjects> game_objects
)
{
	std::vector<std::string> say_links;

	for (auto &g: game_objects) {
		say_links.emplace_back(
			fmt::format(
				"[{}] ",
				EQ::SayLinkEngine::GenerateQuestSaylink(
					fmt::format("#door model {}", g.object_name),
					false,
					g.object_name
				)
			)
		);
	}

	int                      character_length = 0;
	std::vector<std::string> buffered_links;

	for (auto &links: say_links) {
		buffered_links.emplace_back(links);
		character_length += links.length();

		// empty buffer
		if (character_length > MAX_CLIENT_MESSAGE_LENGTH) {
			std::string message_buffer;

			for (auto &buffered_link: buffered_links) {
				message_buffer += buffered_link;
			}

			c->Message(Chat::White, message_buffer.c_str());

			// reset
			character_length = 0;
			buffered_links   = {};
		}
	}

	if (!buffered_links.empty()) {
		c->Message(Chat::White, implode(" ", buffered_links).c_str());
	}
}

void DoorManipulation::DisplayModelsFromFileResults(
	Client *c,
	std::vector<ToolGameObjectsRepository::ToolGameObjects> game_objects
)
{
	std::vector<std::string> say_links;

	for (auto &g: game_objects) {
		say_links.emplace_back(
			fmt::format(
				"[{}] ",
				EQ::SayLinkEngine::GenerateQuestSaylink(
					fmt::format("#door showmodelsfromfile {}", g.file_from),
					false,
					g.file_from
				)
			)
		);
	}

	int                      character_length = 0;
	std::vector<std::string> buffered_links;

	for (auto &links: say_links) {
		buffered_links.emplace_back(links);
		character_length += links.length();

		// empty buffer
		if (character_length > MAX_CLIENT_MESSAGE_LENGTH) {
			std::string message_buffer;

			for (auto &buffered_link: buffered_links) {
				message_buffer += buffered_link;
			}

			c->Message(Chat::White, message_buffer.c_str());

			// reset
			character_length = 0;
			buffered_links   = {};
		}
	}

	if (!buffered_links.empty()) {
		c->Message(Chat::White, implode(" ", buffered_links).c_str());
	}
}
