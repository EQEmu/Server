/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2018 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include "client.h"
#include "mob.h"
#include "../common/races.h"
#include "../common/say_link.h"

inline std::string GetMobAttributeByString(Mob *mob, const std::string &attribute)
{
	if (attribute == "ac") {
		return std::to_string(mob->GetAC());
	}

	if (attribute == "atk") {
		return std::to_string(mob->GetATK());
	}

	if (attribute == "end") {
		int endurance = 0;
		if (mob->IsClient()) {
			endurance = mob->CastToClient()->GetEndurance();
		}

		return std::to_string(endurance);
	}

	if (attribute == "hp") {
		return std::to_string(mob->GetHP());
	}

	if (attribute == "mana") {
		return std::to_string(mob->GetMana());
	}

	if (attribute == "str") {
		return std::to_string(mob->GetSTR());
	}

	if (attribute == "sta") {
		return std::to_string(mob->GetSTA());
	}

	if (attribute == "dex") {
		return std::to_string(mob->GetDEX());
	}

	if (attribute == "agi") {
		return std::to_string(mob->GetAGI());
	}

	if (attribute == "int") {
		return std::to_string(mob->GetINT());
	}

	if (attribute == "wis") {
		return std::to_string(mob->GetWIS());
	}

	if (attribute == "cha") {
		return std::to_string(mob->GetCHA());
	}

	if (attribute == "mr") {
		return std::to_string(mob->GetMR());
	}

	if (attribute == "cr") {
		return std::to_string(mob->GetCR());
	}

	if (attribute == "fr") {
		return std::to_string(mob->GetFR());
	}

	if (attribute == "pr") {
		return std::to_string(mob->GetPR());
	}

	if (attribute == "dr") {
		return std::to_string(mob->GetDR());
	}

	if (attribute == "cr") {
		return std::to_string(mob->GetCR());
	}

	if (attribute == "pr") {
		return std::to_string(mob->GetPR());
	}

	if (attribute == "cor") {
		return std::to_string(mob->GetCorrup());
	}

	if (attribute == "phy") {
		return std::to_string(mob->GetPhR());
	}

	if (attribute == "name") {
		return mob->GetCleanName();
	}

	if (attribute == "lastname") {
		return mob->GetLastName();
	}

	if (attribute == "race") {
		return GetRaceIDName(mob->GetRace());
	}

	if (attribute == "class") {
		return GetClassIDName(mob->GetClass(), 0);
	}

	if (attribute == "level") {
		return std::to_string(mob->GetLevel());
	}

	if (mob->IsNPC()) {
		NPC *npc = mob->CastToNPC();

		if (attribute == "npcid") {
			return std::to_string(npc->GetNPCTypeID());
		}
		if (attribute == "texture") {
			return std::to_string(npc->GetTexture());
		}
		if (attribute == "bodytype") {
			return std::to_string(npc->GetBodyType());
		}
		if (attribute == "gender") {
			return std::to_string(npc->GetGender());
		}
		if (attribute == "size") {
			return std::to_string(npc->GetSize());
		}
		if (attribute == "runspeed") {
			return std::to_string(npc->GetRunspeed());
		}
		if (attribute == "walkspeed") {
			return std::to_string(npc->GetWalkspeed());
		}
		if (attribute == "spawngroup") {
			return std::to_string(npc->GetSp2());
		}
		if (attribute == "grid") {
			return std::to_string(npc->GetGrid());
		}
		if (attribute == "emote") {
			return std::to_string(npc->GetEmoteID());
		}
		npc->GetNPCEmote(npc->GetEmoteID(), 0);
	}

	if (attribute == "type") {
		std::string entity_type = "Mob";

		if (mob->IsCorpse()) {
			entity_type = "Corpse";
		}

		if (mob->IsNPC()) {
			entity_type = "NPC";
		}

		if (mob->IsClient()) {
			entity_type = "Client";
		}

		return entity_type;
	}

	return "null";
}

inline std::string WriteDisplayInfoSection(
	Mob *mob,
	const std::string &section_name,
	std::vector<std::string> attributes_list,
	int column_count = 3,
	bool display_section_name = false
)
{
	std::string text;

	if (display_section_name) {
		text += "<c \"#FFFF66\">" + section_name + "</c><br>";
	}

	text += "<table><tbody>";

	int  index     = 0;
	bool first_row = true;

	for (const auto &attribute : attributes_list) {
		if (index == 0) {
			if (first_row) {
				text += "<tr>\n";
				first_row = false;
			}
			else {
				text += "</tr><tr>\n";
			}

		}

		std::string attribute_name = attribute;
		if (attribute_name.length() <= 3) {
			attribute_name = str_toupper(attribute_name);
		}
		if (attribute_name.length() > 3) {
			attribute_name = ucfirst(attribute_name);
		}

		std::string attribute_value = GetMobAttributeByString(mob, attribute);

		if (attribute_value.length() <= 0) {
			continue;
		}

		text += "<td>" + attribute_name + "</td><td>" + GetMobAttributeByString(mob, attribute) + "</td>";

		if (index == column_count) {
			index = 0;
			continue;
		}

		index++;
	}

	text += "</tr></tbody></table>";

	return text;
}

inline void NPCCommandsMenu(Client* client, NPC* npc)
{
	std::string menu_commands;

	if (npc->GetGrid() > 0) {
		menu_commands += EQEmu::SayLinkEngine::GenerateQuestSaylink("#grid show", false, "Grid Points") + " ";
	}

	if (npc->GetEmoteID() > 0) {
		std::string saylink = StringFormat("#emotesearch %u", npc->GetEmoteID());
		menu_commands += EQEmu::SayLinkEngine::GenerateQuestSaylink(saylink, false, "Emotes") + " ";
	}

	if (menu_commands.length() > 0) {
		client->Message(0, "# Show Commmands");
		client->Message(0, " - %s", menu_commands.c_str());
	}
}

void Mob::DisplayInfo(Mob *mob)
{
	if (!this || !mob) {
		return;
	}

	// std::vector<std::string> general_stats = {
//
	// 	// "accuracy",
	// 	// "slow_mitigation",
	// 	// "atk",
	// 	// "min_hit",
	// 	// "max_hit",
	// 	// "hp_regen",
	// 	// "attack_delay",
	// 	// "special_abilities"
	// };

	if (this->IsClient()) {
		std::string window_text = "*Drag / Maximize Window to see all info<br><br>";

		Client *client = this->CastToClient();

		std::vector<std::string> who_attributes = {
			"name",
			"lastname",
		};
		window_text += WriteDisplayInfoSection(mob, "Who", who_attributes, 1, false);

		std::vector<std::string> type_attributes = {
			"race",
			"class",
			"type"
		};
		window_text += WriteDisplayInfoSection(mob, "Type", type_attributes, 3, true);

		std::vector<std::string> basic_attributes = {
			"level",
			"hp",
			"mana",
			"end",
			"ac",
			"atk"
		};
		window_text += WriteDisplayInfoSection(mob, "Main", basic_attributes, 7, true);

		std::vector<std::string> stat_attributes = {
			"str",
			"sta",
			"agi",
			"dex",
			"wis",
			"int",
			"cha",
		};
		window_text += WriteDisplayInfoSection(mob, "Statistics", stat_attributes, 7, true);

		std::vector<std::string> resist_attributes = {
			"pr",
			"mr",
			"dr",
			"fr",
			"cr",
			"cor",
			"phy",
		};
		window_text += WriteDisplayInfoSection(mob, "Resists", resist_attributes, 7, true);

		if (mob->IsNPC()) {
			std::vector<std::string> npc_attributes = {
				"npcid",
				"texture",
				"bodytype",
				"gender",
				"size",
				"runspeed",
				"walkspeed",
				"spawngroup",
				"grid",
				"emote",
			};
			window_text += WriteDisplayInfoSection(mob, "NPC Attributes", npc_attributes, 2, true);

			client->Message(0, " ");
			mob->CastToNPC()->QueryLoot(client);

			NPCCommandsMenu(client, mob->CastToNPC());
		}

		std::cout << "Window Length: " << window_text.length() << std::endl;
		// std::cout << "Window " << window_text << std::endl;

		if (client->GetDisplayMobInfoWindow()) {
			client->SendFullPopup(
				"GM: Entity Info",
				window_text.c_str(),
				EQEmu::popupresponse::MOB_INFO_DISMISS,
				0,
				100,
				10,
				"Snooze",
				"OK"
			);
		}
	}
}
