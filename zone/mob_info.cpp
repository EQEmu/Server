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

std::string commify(const std::string &number)
{
	std::string temp_string;

	auto string_length = static_cast<int>(number.length());
	int i = 0;
	for (i = string_length - 3; i >= 0; i -= 3) {
		if (i > 0) {
			temp_string = "," + number.substr(static_cast<unsigned long>(i), 3) + temp_string;
		}
		else {
			temp_string = number.substr(static_cast<unsigned long>(i), 3) + temp_string;
		}
	}

	if (i < 0) {
		temp_string = number.substr(0, static_cast<unsigned long>(3 + i)) + temp_string;
	}

	return temp_string;
}

inline std::string GetMobAttributeByString(Mob *mob, const std::string &attribute)
{
	if (attribute == "ac") {
		return commify(std::to_string(mob->GetAC()));
	}

	if (attribute == "atk") {
		return std::to_string(mob->GetATK());
	}

	if (attribute == "end") {
		int endurance = 0;
		if (mob->IsClient()) {
			endurance = mob->CastToClient()->GetEndurance();
		}

		return commify(std::to_string(endurance));
	}

	if (attribute == "hp") {
		return commify(std::to_string(mob->GetHP()));
	}

	if (attribute == "hp_min_max") {
		return commify(std::to_string(mob->GetHP())) + " / " + commify(std::to_string(mob->GetMaxHP())) + " (" +
			   std::to_string((int)mob->GetHPRatio()) + "%)";
	}

	if (attribute == "mana") {
		return commify(std::to_string(mob->GetMana()));
	}

	if (attribute == "mp_min_max") {
		return commify(std::to_string(mob->GetMana())) + " / " + commify(std::to_string(mob->GetMaxMana())) + " (" +
			   std::to_string((int)mob->GetManaPercent()) + "%)";
	}

	if (attribute == "end_min_max") {
		return commify(std::to_string(mob->GetEndurance())) + " / " + commify(std::to_string(mob->GetMaxEndurance())) + " (" +
			   std::to_string((int)mob->GetEndurancePercent()) + "%)";
	}

	if (attribute == "str") {
		return commify(std::to_string(mob->GetSTR())) + " / " + commify(std::to_string(mob->GetMaxSTR())) + " +" +
			   commify(std::to_string(mob->GetHeroicSTR()));
	}

	if (attribute == "sta") {
		return commify(std::to_string(mob->GetSTA())) + " / " + commify(std::to_string(mob->GetMaxSTA())) + " +" +
			   commify(std::to_string(mob->GetHeroicSTA()));
	}

	if (attribute == "dex") {
		return commify(std::to_string(mob->GetDEX())) + " / " + commify(std::to_string(mob->GetMaxDEX())) + " +" +
			   commify(std::to_string(mob->GetHeroicDEX()));
	}

	if (attribute == "agi") {
		return commify(std::to_string(mob->GetAGI())) + " / " + commify(std::to_string(mob->GetMaxAGI())) + " +" +
			   commify(std::to_string(mob->GetHeroicAGI()));
	}

	if (attribute == "int") {
		return commify(std::to_string(mob->GetINT())) + " / " + commify(std::to_string(mob->GetMaxINT())) + " +" +
			   commify(std::to_string(mob->GetHeroicINT()));
	}

	if (attribute == "wis") {
		return commify(std::to_string(mob->GetWIS())) + " / " + commify(std::to_string(mob->GetMaxWIS())) + " +" +
			   commify(std::to_string(mob->GetHeroicWIS()));
	}

	if (attribute == "cha") {
		return commify(std::to_string(mob->GetCHA())) + " / " + commify(std::to_string(mob->GetMaxCHA())) + " +" +
			   commify(std::to_string(mob->GetHeroicCHA()));
	}

	if (attribute == "mr") {
		return commify(std::to_string(mob->GetMR())) + " / " + commify(std::to_string(mob->GetMaxMR())) + " +" +
			   commify(std::to_string(mob->GetHeroicMR()));
	}

	if (attribute == "cr") {
		return commify(std::to_string(mob->GetCR())) + " / " + commify(std::to_string(mob->GetMaxCR())) + " +" +
			   commify(std::to_string(mob->GetHeroicCR()));
	}

	if (attribute == "fr") {
		return commify(std::to_string(mob->GetFR())) + " / " + commify(std::to_string(mob->GetMaxFR())) + " +" +
			   commify(std::to_string(mob->GetHeroicFR()));
	}

	if (attribute == "pr") {
		return commify(std::to_string(mob->GetPR())) + " / " + commify(std::to_string(mob->GetMaxPR())) + " +" +
			   commify(std::to_string(mob->GetHeroicPR()));
	}

	if (attribute == "dr") {
		return commify(std::to_string(mob->GetDR())) + " / " + commify(std::to_string(mob->GetMaxDR())) + " +" +
			   commify(std::to_string(mob->GetHeroicDR()));
	}

	if (attribute == "cr") {
		return commify(std::to_string(mob->GetCR())) + " / " + commify(std::to_string(mob->GetMaxCR())) + " +" +
			   commify(std::to_string(mob->GetHeroicCR()));
	}

	if (attribute == "pr") {
		return commify(std::to_string(mob->GetPR())) + " / " + commify(std::to_string(mob->GetMaxPR())) + " +" +
			   commify(std::to_string(mob->GetHeroicPR()));
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

	if (attribute == "surname") {
		std::string last_name = mob->GetLastName();
		return (last_name.length() > 0 ? mob->GetLastName() : " ");
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

	if (attribute == "flymode") {
		return std::to_string(mob->GetFlyMode());
	}

	if (attribute == "maxbuffslots") {
		return std::to_string(mob->GetMaxBuffSlots());
	}

	if (attribute == "curbuffslots") {
		return std::to_string(mob->GetCurrentBuffSlots());
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
			return std::to_string((int)npc->GetSize());
		}
		if (attribute == "runspeed") {
			return std::to_string((int)npc->GetRunspeed());
		}
		if (attribute == "walkspeed") {
			return std::to_string((int)npc->GetWalkspeed());
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
		if (attribute == "seeInvis") {
			return std::to_string(npc->SeeInvisible());
		}
		if (attribute == "seeInvisUndead") {
			return std::to_string(npc->SeeInvisibleUndead());
		}
		if (attribute == "faction") {
			return std::to_string(npc->GetNPCFactionID());
		}
		if (attribute == "loottable") {
			return std::to_string(npc->GetLoottableID());
		}
		if (attribute == "primSkill") {
			return std::to_string(npc->GetPrimSkill());
		}
		if (attribute == "secSkill") {
			return std::to_string(npc->GetSecSkill());
		}
		if (attribute == "meleeTexture1") {
			return std::to_string(npc->GetMeleeTexture1());
		}
		if (attribute == "meleeTexture2") {
			return std::to_string(npc->GetMeleeTexture2());
		}
		if (attribute == "aggrorange") {
			return std::to_string((int)npc->GetAggroRange());
		}
		if (attribute == "assistrange") {
			return std::to_string((int)npc->GetAssistRange());
		}
		if (attribute == "findable") {
			return std::to_string(npc->IsFindable());
		}
		if (attribute == "trackable") {
			return std::to_string(npc->IsTrackable());
		}
		if (attribute == "spellsid") {
			return std::to_string(npc->GetNPCSpellsID());
		}
		if (attribute == "roamboxMinX") {
			return std::to_string((int)npc->GetRoamboxMinX());
		}
		if (attribute == "roamboxMaxX") {
			return std::to_string((int)npc->GetRoamboxMaxX());
		}
		if (attribute == "roamboxMinY") {
			return std::to_string((int)npc->GetRoamboxMinY());
		}
		if (attribute == "roamboxMaxY") {
			return std::to_string((int)npc->GetRoamboxMaxY());
		}
		if (attribute == "roamboxMinDelay") {
			return std::to_string((int)npc->GetRoamboxMinDelay());
		}
		if (attribute == "roamboxDelay") {
			return std::to_string((int)npc->GetRoamboxDelay());
		}
		if (attribute == "roamboxDistance") {
			return std::to_string((int)npc->GetRoamboxDistance());
		}
		if (attribute == "proximityMinX") {
			return std::to_string((int)npc->GetProximityMinX());
		}
		if (attribute == "proximityMaxX") {
			return std::to_string((int)npc->GetProximityMaxX());
		}
		if (attribute == "proximityMinY") {
			return std::to_string((int)npc->GetProximityMinY());
		}
		if (attribute == "proximityMaxY") {
			return std::to_string((int)npc->GetProximityMaxY());
		}
		if (attribute == "proximityMinZ") {
			return std::to_string((int)npc->GetProximityMinZ());
		}
		if (attribute == "proximityMaxZ") {
			return std::to_string((int)npc->GetProximityMaxZ());
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

		if (attribute_name.find('_') != std::string::npos) {
			std::vector<std::string> split_string = split(attribute_name, '_');
			attribute_name = split_string[0];
		}
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
		std::string window_text = "<c \"#FFFF66\">*Drag window open vertically to see all</c><br>";

		Client *client = this->CastToClient();

		std::vector<std::string> info_attributes = {
			"name",
			"race",
			"surname",
			"class",
		};
		window_text += WriteDisplayInfoSection(mob, "Info", info_attributes, 1, false);

		std::vector<std::string> basic_attributes = {
			"type",
			"level",
			"hp_min_max",
			"ac",
			"mp_min_max",
			"atk",
			"end_min_max",
		};
		window_text += WriteDisplayInfoSection(mob, "Main", basic_attributes, 1, false);

		std::vector<std::string> stat_attributes = {
			"str",
			"sta",
			"agi",
			"dex",
			"wis",
			"int",
			"cha",
		};
		window_text += WriteDisplayInfoSection(mob, "Statistics", stat_attributes, 1, false);

		std::vector<std::string> resist_attributes = {
			"pr",
			"mr",
			"dr",
			"fr",
			"cr",
			"cor",
			"phy",
		};
		window_text += WriteDisplayInfoSection(mob, "Resists", resist_attributes, 1, false);

		if (mob->IsNPC()) {
			NPC *npc = mob->CastToNPC();

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
				"seeInvis",
				"seeInvisUndead",
				"faction",
				"loottable",
				"primSkill",
				"secSkill",
				"meleeTexture1",
				"meleeTexture2",
				"aggrorange",
				"assistrange",
				"findable",
				"trackable",
				"flymode",
				"spellsid",
				"curbuffslots",
				"maxbuffslots",
			};

			window_text += WriteDisplayInfoSection(mob, "NPC Attributes", npc_attributes, 1, true);

			/**
			 * Print Roambox
			 */
			if (npc->GetRoamboxMaxX() != 0 && npc->GetRoamboxMinX() != 0) {
				std::vector<std::string> npc_roambox = {
					"roamboxMinX",
					"roamboxMaxX",
					"roamboxMinY",
					"roamboxMaxY",
					"roamboxMinDelay",
					"roamboxDelay",
					"roamboxDistance",
				};

				window_text += WriteDisplayInfoSection(mob, "Roambox", npc_roambox, 1, true);
			}

			if (npc->proximity != nullptr) {
				std::vector<std::string> npc_proximity = {
					"proximityMinX",
					"proximityMaxX",
					"proximityMinY",
					"proximityMaxY",
					"proximityMinZ",
					"proximityMaxZ",
				};

				window_text += WriteDisplayInfoSection(mob, "Proximity", npc_proximity, 1, true);
			}

			client->Message(0, " ");
			npc->QueryLoot(client);

			NPCCommandsMenu(client, npc);
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
				0,
				"Snooze",
				"OK"
			);
		}
	}
}
