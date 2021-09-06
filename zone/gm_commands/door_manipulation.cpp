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
		Doors       *door = entity_list.GetDoorsByID(c->GetDoorToolEntityId());
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

		c->Message(Chat::White, fmt::format("Creating door entity_id [{}] with model [{}]", entity_id, model).c_str());
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
