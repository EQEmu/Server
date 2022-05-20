#include "../client.h"
#include "../npc_scale_manager.h"

void command_scale(Client *c, const Seperator *sep)
{
	if (sep->argnum == 0) {
		c->Message(Chat::Yellow, "# Usage # ");
		c->Message(Chat::Yellow, "#scale [static/dynamic] (With targeted NPC)");
		c->Message(Chat::Yellow, "#scale [npc_name_search] [static/dynamic] (To make zone-wide changes)");
		c->Message(Chat::Yellow, "#scale all [static/dynamic]");
		return;
	}

	/**
	 * Targeted changes
	 */
	if (c->GetTarget() && c->GetTarget()->IsNPC() && sep->argnum < 2) {
		NPC *npc = c->GetTarget()->CastToNPC();

		bool apply_status = false;
		if (strcasecmp(sep->arg[1], "dynamic") == 0) {
			c->Message(Chat::Yellow, "Applying global base scaling to npc dynamically (All stats set to zeroes)...");
			apply_status = npc_scale_manager->ApplyGlobalBaseScalingToNPCDynamically(npc);
		}
		else if (strcasecmp(sep->arg[1], "static") == 0) {
			c->Message(Chat::Yellow, "Applying global base scaling to npc statically (Copying base stats onto NPC)...");
			apply_status = npc_scale_manager->ApplyGlobalBaseScalingToNPCStatically(npc);
		}
		else {
			return;
		}

		if (apply_status) {
			c->Message(Chat::Yellow, "Applied to NPC '%s' successfully!", npc->GetName());
		}
		else {
			c->Message(
				Chat::Yellow, "Failed to load scaling data from the database "
							  "for this npc / type, see 'NPCScaling' log for more info"
			);
		}
	}
	else if (c->GetTarget() && sep->argnum < 2) {
		c->Message(Chat::Yellow, "Target must be an npc!");
	}

	/**
	 * Zonewide
	 */
	if (sep->argnum > 1) {

		std::string scale_type;
		if (strcasecmp(sep->arg[2], "dynamic") == 0) {
			scale_type = "dynamic";
		}
		else if (strcasecmp(sep->arg[2], "static") == 0) {
			scale_type = "static";
		}

		if (scale_type.length() <= 0) {
			c->Message(
				Chat::Yellow,
				"You must first set if you intend on using static versus dynamic for these changes"
			);
			c->Message(Chat::Yellow, "#scale [npc_name_search] [static/dynamic]");
			c->Message(Chat::Yellow, "#scale all [static/dynamic]");
			return;
		}

		std::string search_string = sep->arg[1];

		auto &entity_list_search = entity_list.GetNPCList();

		int       found_count = 0;
		for (auto &itr : entity_list_search) {
			NPC *entity = itr.second;

			std::string entity_name = entity->GetName();

			/**
			 * Filter by name
			 */
			if (search_string.length() > 0 && entity_name.find(search_string) == std::string::npos &&
				strcasecmp(sep->arg[1], "all") != 0) {
				continue;
			}

			std::string status = "(Searching)";

			if (strcasecmp(sep->arg[3], "apply") == 0) {
				status = "(Applying)";

				if (strcasecmp(sep->arg[2], "dynamic") == 0) {
					npc_scale_manager->ApplyGlobalBaseScalingToNPCDynamically(entity);
				}
				if (strcasecmp(sep->arg[2], "static") == 0) {
					npc_scale_manager->ApplyGlobalBaseScalingToNPCStatically(entity);
				}
			}

			c->Message(
				15,
				"| ID %5d | %s | x %.0f | y %0.f | z %.0f | DBID %u %s",
				entity->GetID(),
				entity->GetName(),
				entity->GetX(),
				entity->GetY(),
				entity->GetZ(),
				entity->GetNPCTypeID(),
				status.c_str()
			);

			found_count++;
		}

		if (strcasecmp(sep->arg[3], "apply") == 0) {
			c->Message(Chat::Yellow, "%s scaling applied against (%i) NPC's", sep->arg[2], found_count);
		}
		else {

			std::string saylink = StringFormat(
				"#scale %s %s apply",
				sep->arg[1],
				sep->arg[2]
			);

			c->Message(Chat::Yellow, "Found (%i) NPC's that match this search...", found_count);
			c->Message(
				Chat::Yellow, "To apply these changes, click <%s> or type %s",
				EQ::SayLinkEngine::GenerateQuestSaylink(saylink, false, "Apply").c_str(),
				saylink.c_str()
			);
		}
	}
}

