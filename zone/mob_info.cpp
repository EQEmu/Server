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
#include "npc_scale_manager.h"

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
	std::string entity_variable = "modify_stat_" + attribute;
	std::string scaling_modified;
	if (mob->GetEntityVariable(entity_variable.c_str())) {
		scaling_modified = " *";
	}

	if (attribute == "ac") {
		if (mob->GetEntityVariable(std::string("modify_stat_max_hp").c_str())) {
			scaling_modified = " *";
		}

		return commify(std::to_string(mob->GetAC())) + scaling_modified;
	}

	if (attribute == "atk") {
		return std::to_string(mob->GetATK()) + scaling_modified;
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
		if (mob->GetEntityVariable(std::string("modify_stat_max_hp").c_str())) {
			scaling_modified = " *";
		}

		return commify(std::to_string(mob->GetHP())) + scaling_modified + " / " +
			   commify(std::to_string(mob->GetMaxHP())) + " (" +
			   std::to_string((int) mob->GetHPRatio()) + "%)";
	}

	if (attribute == "mana") {
		return commify(std::to_string(mob->GetMana()));
	}

	if (attribute == "mp_min_max") {
		return commify(std::to_string(mob->GetMana())) + " / " + commify(std::to_string(mob->GetMaxMana())) + " (" +
			   std::to_string((int) mob->GetManaPercent()) + "%)";
	}

	if (attribute == "end_min_max") {
		return commify(std::to_string(mob->GetEndurance())) + " / " + commify(std::to_string(mob->GetMaxEndurance())) + " (" +
			   std::to_string((int)mob->GetEndurancePercent()) + "%)";
	}

	if (attribute == "str") {
		return commify(std::to_string(mob->GetSTR())) + scaling_modified + " / " +
			   commify(std::to_string(mob->GetMaxSTR())) + " +" +
			   commify(std::to_string(mob->GetHeroicSTR()));
	}

	if (attribute == "sta") {
		return commify(std::to_string(mob->GetSTA())) + scaling_modified + " / " +
			   commify(std::to_string(mob->GetMaxSTA())) + " +" +
			   commify(std::to_string(mob->GetHeroicSTA()));
	}

	if (attribute == "dex") {
		return commify(std::to_string(mob->GetDEX())) + scaling_modified + " / " +
			   commify(std::to_string(mob->GetMaxDEX())) + " +" +
			   commify(std::to_string(mob->GetHeroicDEX()));
	}

	if (attribute == "agi") {
		return commify(std::to_string(mob->GetAGI())) + scaling_modified + " / " +
			   commify(std::to_string(mob->GetMaxAGI())) + " +" +
			   commify(std::to_string(mob->GetHeroicAGI()));
	}

	if (attribute == "int") {
		return commify(std::to_string(mob->GetINT())) + scaling_modified + " / " +
			   commify(std::to_string(mob->GetMaxINT())) + " +" +
			   commify(std::to_string(mob->GetHeroicINT()));
	}

	if (attribute == "wis") {
		return commify(std::to_string(mob->GetWIS())) + scaling_modified + " / " +
			   commify(std::to_string(mob->GetMaxWIS())) + " +" +
			   commify(std::to_string(mob->GetHeroicWIS()));
	}

	if (attribute == "cha") {
		return commify(std::to_string(mob->GetCHA())) + scaling_modified + " / " +
			   commify(std::to_string(mob->GetMaxCHA())) + " +" +
			   commify(std::to_string(mob->GetHeroicCHA()));
	}

	if (attribute == "mr") {
		return commify(std::to_string(mob->GetMR())) + scaling_modified + " / " +
			   commify(std::to_string(mob->GetMaxMR())) + " +" +
			   commify(std::to_string(mob->GetHeroicMR()));
	}

	if (attribute == "cr") {
		return commify(std::to_string(mob->GetCR())) + scaling_modified + " / " +
			   commify(std::to_string(mob->GetMaxCR())) + " +" +
			   commify(std::to_string(mob->GetHeroicCR()));
	}

	if (attribute == "fr") {
		return commify(std::to_string(mob->GetFR())) + scaling_modified + " / " +
			   commify(std::to_string(mob->GetMaxFR())) + " +" +
			   commify(std::to_string(mob->GetHeroicFR()));
	}

	if (attribute == "pr") {
		return commify(std::to_string(mob->GetPR())) + scaling_modified + " / " +
			   commify(std::to_string(mob->GetMaxPR())) + " +" +
			   commify(std::to_string(mob->GetHeroicPR()));
	}

	if (attribute == "dr") {
		return commify(std::to_string(mob->GetDR())) + scaling_modified + " / " +
			   commify(std::to_string(mob->GetMaxDR())) + " +" +
			   commify(std::to_string(mob->GetHeroicDR()));
	}

	if (attribute == "cr") {
		return commify(std::to_string(mob->GetCR())) + scaling_modified + " / " +
			   commify(std::to_string(mob->GetMaxCR())) + " +" +
			   commify(std::to_string(mob->GetHeroicCR()));
	}

	if (attribute == "pr") {
		return commify(std::to_string(mob->GetPR())) + scaling_modified + " / " +
			   commify(std::to_string(mob->GetMaxPR())) + " +" +
			   commify(std::to_string(mob->GetHeroicPR()));
	}

	if (attribute == "cor") {
		return commify(std::to_string(mob->GetCorrup())) + scaling_modified;
	}

	if (attribute == "phy") {
		return commify(std::to_string(mob->GetPhR())) + scaling_modified;
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

	if (attribute == "tohit") {
		return std::to_string(mob->compute_tohit(EQ::skills::SkillHandtoHand));
	}

	if (attribute == "total_to_hit") {
		return std::to_string(mob->GetTotalToHit(EQ::skills::SkillHandtoHand, 0));
	}

	if (attribute == "defense") {
		return std::to_string(mob->compute_defense());
	}

	if (attribute == "total_defense") {
		return std::to_string(mob->GetTotalDefense());
	}

	if (attribute == "offense") {
		return std::to_string(mob->offense(EQ::skills::SkillHandtoHand));
	}

	if (attribute == "mitigation_ac") {
		return std::to_string(mob->GetMitigationAC());
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
			return std::to_string(npc->GetSpawnGroupId());
		}
		if (attribute == "grid") {
			return std::to_string(npc->GetGrid());
		}
		if (attribute == "emote") {
			return std::to_string(npc->GetEmoteID());
		}
		if (attribute == "see_invis") {
			return std::to_string(npc->SeeInvisible());
		}
		if (attribute == "see_invis_undead") {
			return std::to_string(npc->SeeInvisibleUndead());
		}
		if (attribute == "faction") {
			return std::to_string(npc->GetNPCFactionID());
		}
		if (attribute == "loottable") {
			return std::to_string(npc->GetLoottableID());
		}
		if (attribute == "prim_skill") {
			return std::to_string(npc->GetPrimSkill());
		}
		if (attribute == "sec_skill") {
			return std::to_string(npc->GetSecSkill());
		}
		if (attribute == "melee_texture_1") {
			return std::to_string(npc->GetMeleeTexture1());
		}
		if (attribute == "melee_texture_2") {
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
		if (attribute == "spells_id") {
			return std::to_string(npc->GetNPCSpellsID());
		}
		if (attribute == "roambox_min_x") {
			return std::to_string((int)npc->GetRoamboxMinX());
		}
		if (attribute == "roambox_max_x") {
			return std::to_string((int)npc->GetRoamboxMaxX());
		}
		if (attribute == "roambox_min_y") {
			return std::to_string((int)npc->GetRoamboxMinY());
		}
		if (attribute == "roambox_max_y") {
			return std::to_string((int)npc->GetRoamboxMaxY());
		}
		if (attribute == "roambox_min_delay") {
			return std::to_string((int)npc->GetRoamboxMinDelay());
		}
		if (attribute == "roambox_delay") {
			return std::to_string((int)npc->GetRoamboxDelay());
		}
		if (attribute == "roambox_distance") {
			return std::to_string((int)npc->GetRoamboxDistance());
		}
		if (attribute == "proximity_min_x") {
			return std::to_string((int)npc->GetProximityMinX());
		}
		if (attribute == "proximity_max_x") {
			return std::to_string((int)npc->GetProximityMaxX());
		}
		if (attribute == "proximity_min_y") {
			return std::to_string((int)npc->GetProximityMinY());
		}
		if (attribute == "proximity_max_y") {
			return std::to_string((int)npc->GetProximityMaxY());
		}
		if (attribute == "proximity_min_z") {
			return std::to_string((int)npc->GetProximityMinZ());
		}
		if (attribute == "proximity_max_z") {
			return std::to_string((int)npc->GetProximityMaxZ());
		}
		if (attribute == "accuracy") {
			return std::to_string((int)npc->GetAccuracyRating()) + scaling_modified;
		}
		if (attribute == "slow_mitigation") {
			if (mob->GetEntityVariable(std::string("modify_stat_slow_mitigation").c_str())) {
				scaling_modified = " *";
			}

			return std::to_string((int)npc->GetSlowMitigation()) + scaling_modified;
		}
		if (attribute == "min_hit") {
			if (mob->GetEntityVariable(std::string("modify_stat_min_hit").c_str())) {
				scaling_modified = " *";
			}

			return commify(std::to_string((int)npc->GetMinDMG())) + scaling_modified;
		}
		if (attribute == "max_hit") {
			if (mob->GetEntityVariable(std::string("modify_stat_max_hit").c_str())) {
				scaling_modified = " *";
			}

			return commify(std::to_string((int)npc->GetMaxDMG())) + scaling_modified;
		}
		if (attribute == "hp_regen") {
			if (mob->GetEntityVariable(std::string("modify_stat_hp_regen").c_str())) {
				scaling_modified = " *";
			}

			return commify(std::to_string((int)npc->GetHPRegen())) + scaling_modified;
		}
		if (attribute == "attack_delay") {
			if (mob->GetEntityVariable(std::string("modify_stat_attack_delay").c_str())) {
				scaling_modified = " *";
			}

			return commify(std::to_string(npc->GetAttackDelay())) + scaling_modified;
		}
		if (attribute == "spell_scale") {
			if (mob->GetEntityVariable(std::string("modify_stat_spell_scale").c_str())) {
				scaling_modified = " *";
			}

			return commify(std::to_string((int)npc->GetSpellScale())) + scaling_modified;
		}
		if (attribute == "heal_scale") {
			if (mob->GetEntityVariable(std::string("modify_stat_heal_scale").c_str())) {
				scaling_modified = " *";
			}

			return commify(std::to_string((int)npc->GetHealScale())) + scaling_modified;
		}
		if (attribute == "avoidance") {
			return commify(std::to_string((int)npc->GetAvoidanceRating())) + scaling_modified;
		}

		npc->GetNPCEmote(npc->GetEmoteID(), 0);
	}

	if (mob->IsClient()) {
		Client *client = mob->CastToClient();

		if (attribute == "shielding") {
			return commify(std::to_string((int)client->GetShielding())) + " / " +
				   commify(std::to_string((int) RuleI(Character, ItemShieldingCap)));
		}
		if (attribute == "spell_shielding") {
			return commify(std::to_string((int)client->GetSpellShield())) + " / " +
				   commify(std::to_string((int) RuleI(Character, ItemSpellShieldingCap)));
		}
		if (attribute == "dot_shielding") {
			return commify(std::to_string((int)client->GetDoTShield())) + " / " +
				   commify(std::to_string((int) RuleI(Character, ItemDoTShieldingCap)));
		}
		if (attribute == "stun_resist") {
			return commify(std::to_string((int)client->GetStunResist())) + " / " +
				   commify(std::to_string((int) RuleI(Character, ItemStunResistCap)));
		}
		if (attribute == "damage_shield") {
			return commify(std::to_string((int)client->GetDS())) + " / " +
				   commify(std::to_string((int) RuleI(Character, ItemDamageShieldCap)));
		}
		if (attribute == "avoidance") {
			return commify(std::to_string((int) client->GetAvoidance())) + " / " +
				   commify(std::to_string((int) RuleI(Character, ItemAvoidanceCap)));
		}
		if (attribute == "strikethrough") {
			return commify(std::to_string((int) client->GetStrikeThrough())) + " / " +
				   commify(std::to_string((int) RuleI(Character, ItemStrikethroughCap)));
		}
		if (attribute == "accuracy") {
			return commify(std::to_string((int) client->GetAccuracy())) + " / " +
				   commify(std::to_string((int) RuleI(Character, ItemAccuracyCap)));
		}
		if (attribute == "combat_effects") {
			return commify(std::to_string((int) client->GetCombatEffects())) + " / " +
				   commify(std::to_string((int) RuleI(Character, ItemCombatEffectsCap)));
		}
		if (attribute == "heal_amount") {
			return commify(std::to_string((int) client->GetHealAmt())) + " / " +
				   commify(std::to_string((int) RuleI(Character, ItemHealAmtCap)));
		}
		if (attribute == "spell_dmg") {
			return commify(std::to_string((int) client->GetSpellDmg())) + " / " +
				   commify(std::to_string((int) RuleI(Character, ItemSpellDmgCap)));
		}
		if (attribute == "clairvoyance") {
			return commify(std::to_string((int) client->GetClair())) + " / " +
				   commify(std::to_string((int) RuleI(Character, ItemClairvoyanceCap)));
		}
		if (attribute == "ds_mitigation") {
			return commify(std::to_string((int) client->GetDSMit())) + " / " +
				   commify(std::to_string((int) RuleI(Character, ItemDSMitigationCap)));
		}
		if (attribute == "hp_regen") {
			return commify(std::to_string((int) client->GetHPRegen())) + " / " +
				   commify(std::to_string((int) RuleI(Character, ItemHealthRegenCap)));
		}
		if (attribute == "mana_regen") {
			return commify(std::to_string((int) client->GetManaRegen())) + " / " +
				   commify(std::to_string((int) RuleI(Character, ItemManaRegenCap)));
		}
		if (attribute == "end_regen") {
			return commify(std::to_string((int) client->CalcEnduranceRegen())) + " / " +
				   commify(std::to_string((int) client->CalcEnduranceRegenCap()));
		}
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

		find_replace(attribute_name, "_min_max", std::string(""));

		/**
		 * Translate attribute names with underscores
		 *     "total_to_hit" = "Total To Hit"
		 */
		if (attribute_name.find('_') != std::string::npos) {
			std::vector<std::string> split_string = split(attribute_name, '_');
			std::string new_attribute_name;
			for (std::string &string_value : split_string) {
				new_attribute_name += ucfirst(string_value) + " ";
			}
			attribute_name = new_attribute_name;
		}

		/**
		 * Attribute names less than 4 characters get capitalized
		 *     "hp" = "HP"
		 */
		if (attribute_name.length() <= 3) {
			attribute_name = str_toupper(attribute_name);
		}

		/**
		 * Attribute names larger than 3 characters get capitalized first letter
		 *     "avoidance" = "Avoidance"
		 */
		if (attribute_name.length() > 3) {
			attribute_name = ucfirst(attribute_name);
		}

		find_replace(attribute_name, "Proximity", std::string(""));
		find_replace(attribute_name, "Roambox", std::string(""));

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
		menu_commands += "[" + EQ::SayLinkEngine::GenerateQuestSaylink("#grid show", false, "Grid Points") + "] ";
	}

	if (npc->GetEmoteID() > 0) {
		std::string saylink = StringFormat("#emotesearch %u", npc->GetEmoteID());
		menu_commands += "[" + EQ::SayLinkEngine::GenerateQuestSaylink(saylink, false, "Emotes") + "] ";
	}

	if (npc->GetLoottableID() > 0) {
		menu_commands += "[" + EQ::SayLinkEngine::GenerateQuestSaylink("#npcloot show", false, "Loot") + "] ";
	}

	if (npc->IsProximitySet()) {
		menu_commands += "[" + EQ::SayLinkEngine::GenerateQuestSaylink("#proximity show", false, "Proximity") + "] ";
	}

	if (menu_commands.length() > 0) {
		std::string dev_menu = "[" + EQ::SayLinkEngine::GenerateQuestSaylink("#devtools", false, "DevTools") + "] ";;
		client->Message(Chat::White, "| %s [Show Commands] %s", dev_menu.c_str(), menu_commands.c_str());
	}
}

void Mob::DisplayInfo(Mob *mob)
{
	if (!mob) {
		return;
	}

	if (this->IsClient()) {

		std::string window_text = "<c \"#FFFF66\">*Drag window open vertically to see all</c><br>";

		Client *client = this->CastToClient();

		if (!client->IsDevToolsEnabled()) {
			return;
		}

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

		std::vector<std::string> calculations = {
			"tohit",
			"total_to_hit",
			"defense",
			"total_defense",
			"offense",
			"mitigation_ac",
		};
		window_text += WriteDisplayInfoSection(mob, "Calculations", calculations, 1, true);

		if (mob->IsClient()) {
			std::vector<std::string> mods = {
				"hp_regen",
				"mana_regen",
				"end_regen",
				"heal_amount",
				"spell_dmg",
				"clairvoyance",
			};
			window_text += WriteDisplayInfoSection(mob, "Mods", mods, 1, true);

			std::vector<std::string> mod_defensive = {
				"shielding",
				"spell_shielding",
				"dot_shielding",
				"stun_resist",
				"damage_shield",
				"ds_mitigation",
				"avoidance",
			};

			window_text += WriteDisplayInfoSection(mob, "Mod Defensive", mod_defensive, 1, true);

			std::vector<std::string> mod_offensive = {
				"strikethrough",
				"accuracy",
				"combat_effects",
			};
			window_text += WriteDisplayInfoSection(mob, "Mod Offensive", mod_offensive, 1, true);
		}

		if (mob->IsNPC()) {
			NPC *npc = mob->CastToNPC();

			std::vector<std::string> npc_stats = {
				"accuracy",
				"slow_mitigation",
				"min_hit",
				"max_hit",
				"hp_regen",
				"attack_delay",
				"spell_scale",
				"heal_scale",
				"avoidance",
			};

			window_text += WriteDisplayInfoSection(mob, "NPC Stats", npc_stats, 1, true);

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
				"see_invis",
				"see_invis_undead",
				"faction",
				"loottable",
				"prim_skill",
				"sec_skill",
				"melee_texture_1",
				"melee_texture_2",
				"aggrorange",
				"assistrange",
				"findable",
				"trackable",
				"flymode",
				"spells_id",
				"curbuffslots",
				"maxbuffslots",
			};

			window_text += WriteDisplayInfoSection(mob, "NPC Attributes", npc_attributes, 1, true);

			/**
			 * Print Roambox
			 */
			if (npc->GetRoamboxMaxX() != 0 && npc->GetRoamboxMinX() != 0) {
				std::vector<std::string> npc_roambox = {
					"roambox_min_x",
					"roambox_max_x",
					"roambox_min_y",
					"roambox_max_y",
					"roambox_min_delay",
					"roambox_delay",
					"roambox_distance",
				};

				window_text += WriteDisplayInfoSection(mob, "Roambox", npc_roambox, 1, true);
			}

			if (npc->proximity != nullptr) {
				std::vector<std::string> npc_proximity = {
					"proximity_min_x",
					"proximity_max_x",
					"proximity_min_y",
					"proximity_max_y",
					"proximity_min_z",
					"proximity_max_z",
				};

				window_text += WriteDisplayInfoSection(mob, "Proximity", npc_proximity, 1, true);
			}

			int8        npc_type        = npc_scale_manager->GetNPCScalingType(npc);
			std::string npc_type_string = npc_scale_manager->GetNPCScalingTypeName(npc);

			client->Message(
				0,
				"| # Target: %s Type: %i (%s)",
				npc->GetCleanName(),
				npc_type,
				npc_type_string.c_str());

			NPCCommandsMenu(client, npc);
		}

		// std::cout << "Window Length: " << window_text.length() << std::endl;

		if (client->GetDisplayMobInfoWindow()) {
			client->SendFullPopup(
				"GM: Entity Info",
				window_text.c_str(),
				EQ::popupresponse::MOB_INFO_DISMISS,
				0,
				100,
				0,
				"Snooze",
				"OK"
			);
		}
	}
}
