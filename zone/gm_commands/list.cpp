#include "../client.h"
#include "../corpse.h"
#include "../object.h"
#include "../doors.h"

void command_list(Client *c, const Seperator *sep)
{
	std::string search_type;
	if (strcasecmp(sep->arg[1], "npcs") == 0) {
		search_type = "npcs";
	}

	if (strcasecmp(sep->arg[1], "players") == 0) {
		search_type = "players";
	}

	if (strcasecmp(sep->arg[1], "corpses") == 0) {
		search_type = "corpses";
	}

	if (strcasecmp(sep->arg[1], "doors") == 0) {
		search_type = "doors";
	}

	if (strcasecmp(sep->arg[1], "objects") == 0) {
		search_type = "objects";
	}

	if (search_type.length() > 0) {

		int entity_count = 0;
		int found_count  = 0;

		std::string search_string;

		if (sep->arg[2]) {
			search_string = sep->arg[2];
		}

		/**
		 * NPC
		 */
		if (search_type.find("npcs") != std::string::npos) {
			auto &entity_list_search = entity_list.GetMobList();

			for (auto &itr : entity_list_search) {
				Mob *entity = itr.second;
				if (!entity->IsNPC()) {
					continue;
				}

				entity_count++;

				std::string entity_name = entity->GetName();

				/**
				 * Filter by name
				 */
				if (search_string.length() > 0 && entity_name.find(search_string) == std::string::npos) {
					continue;
				}

				std::string saylink = StringFormat(
					"#goto %.0f %0.f %.0f",
					entity->GetX(),
					entity->GetY(),
					entity->GetZ() + (entity->IsBoat() ? 50 : 0));

				c->Message(
					0,
					"| %s | ID %5d | %s | x %.0f | y %0.f | z %.0f",
					EQ::SayLinkEngine::GenerateQuestSaylink(saylink, false, "Goto").c_str(),
					entity->GetID(),
					entity->GetName(),
					entity->GetX(),
					entity->GetY(),
					entity->GetZ()
				);

				found_count++;
			}
		}

		/**
		 * Client
		 */
		if (search_type.find("players") != std::string::npos) {
			auto &entity_list_search = entity_list.GetClientList();

			for (auto &itr : entity_list_search) {
				Client *entity = itr.second;

				entity_count++;

				std::string entity_name = entity->GetName();

				/**
				 * Filter by name
				 */
				if (search_string.length() > 0 && entity_name.find(search_string) == std::string::npos) {
					continue;
				}

				std::string saylink = StringFormat(
					"#goto %.0f %0.f %.0f",
					entity->GetX(),
					entity->GetY(),
					entity->GetZ());

				c->Message(
					0,
					"| %s | ID %5d | %s | x %.0f | y %0.f | z %.0f",
					EQ::SayLinkEngine::GenerateQuestSaylink(saylink, false, "Goto").c_str(),
					entity->GetID(),
					entity->GetName(),
					entity->GetX(),
					entity->GetY(),
					entity->GetZ()
				);

				found_count++;
			}
		}

		/**
		 * Corpse
		 */
		if (search_type.find("corpses") != std::string::npos) {
			auto &entity_list_search = entity_list.GetCorpseList();

			for (auto &itr : entity_list_search) {
				Corpse *entity = itr.second;

				entity_count++;

				std::string entity_name = entity->GetName();

				/**
				 * Filter by name
				 */
				if (search_string.length() > 0 && entity_name.find(search_string) == std::string::npos) {
					continue;
				}

				std::string saylink = StringFormat(
					"#goto %.0f %0.f %.0f",
					entity->GetX(),
					entity->GetY(),
					entity->GetZ());

				c->Message(
					0,
					"| %s | ID %5d | %s | x %.0f | y %0.f | z %.0f",
					EQ::SayLinkEngine::GenerateQuestSaylink(saylink, false, "Goto").c_str(),
					entity->GetID(),
					entity->GetName(),
					entity->GetX(),
					entity->GetY(),
					entity->GetZ()
				);

				found_count++;
			}
		}

		/**
		 * Doors
		 */
		if (search_type.find("doors") != std::string::npos) {
			auto &entity_list_search = entity_list.GetDoorsList();

			for (auto &itr : entity_list_search) {
				Doors *entity = itr.second;

				entity_count++;

				std::string entity_name = entity->GetDoorName();

				/**
				 * Filter by name
				 */
				if (search_string.length() > 0 && entity_name.find(search_string) == std::string::npos) {
					continue;
				}

				std::string saylink = StringFormat(
					"#goto %.0f %0.f %.0f",
					entity->GetX(),
					entity->GetY(),
					entity->GetZ());

				c->Message(
					0,
					"| %s | Entity ID %5d | Door ID %i | %s | x %.0f | y %0.f | z %.0f",
					EQ::SayLinkEngine::GenerateQuestSaylink(saylink, false, "Goto").c_str(),
					entity->GetID(),
					entity->GetDoorID(),
					entity->GetDoorName(),
					entity->GetX(),
					entity->GetY(),
					entity->GetZ()
				);

				found_count++;
			}
		}

		/**
		 * Objects
		 */
		if (search_type.find("objects") != std::string::npos) {
			auto &entity_list_search = entity_list.GetObjectList();

			for (auto &itr : entity_list_search) {
				Object *entity = itr.second;

				entity_count++;

				std::string entity_name = entity->GetModelName();

				/**
				 * Filter by name
				 */
				if (search_string.length() > 0 && entity_name.find(search_string) == std::string::npos) {
					continue;
				}

				std::string saylink = StringFormat(
					"#goto %.0f %0.f %.0f",
					entity->GetX(),
					entity->GetY(),
					entity->GetZ());

				c->Message(
					0,
					"| %s | Entity ID %5d | Object DBID %i | %s | x %.0f | y %0.f | z %.0f",
					EQ::SayLinkEngine::GenerateQuestSaylink(saylink, false, "Goto").c_str(),
					entity->GetID(),
					entity->GetDBID(),
					entity->GetModelName(),
					entity->GetX(),
					entity->GetY(),
					entity->GetZ()
				);

				found_count++;
			}
		}

		if (found_count) {
			c->Message(
				0, "Found (%i) of type (%s) in zone (%i) total",
				found_count,
				search_type.c_str(),
				entity_count
			);
		}
	}
	else {
		c->Message(Chat::White, "Usage of #list");
		c->Message(Chat::White, "- #list [npcs|players|corpses|doors|objects] [search]");
		c->Message(Chat::White, "- Example: #list npcs (Blank for all)");
	}
}

