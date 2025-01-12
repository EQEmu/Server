/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2004 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "../common/global_define.h"
#include "../common/spdat.h"
#include "../common/strings.h"

#include "../common/repositories/pets_repository.h"
#include "../common/repositories/pets_beastlord_data_repository.h"
#include "../common/repositories/character_pet_name_repository.h"

#include "entity.h"
#include "client.h"
#include "mob.h"

#include "pets.h"
#include "zonedb.h"

#include <string>

#include "bot.h"

#ifndef WIN32
#include <stdlib.h>
#include "../common/unix.h"
#include "npc.h"
#endif


// need to pass in a char array of 64 chars
void Mob::GetRandPetName(char *name)
{
	std::string temp;
	temp.reserve(64);
	// note these orders are used to make the exclusions cheap :P
	static const char *part1[] = {"G", "J", "K", "L", "V", "X", "Z"};
	static const char *part2[] = {nullptr, "ab", "ar", "as", "eb", "en", "ib", "ob", "on"};
	static const char *part3[] = {nullptr, "an", "ar", "ek", "ob"};
	static const char *part4[] = {"er", "ab", "n", "tik"};

	const char *first = part1[zone->random.Int(0, (sizeof(part1) / sizeof(const char *)) - 1)];
	const char *second = part2[zone->random.Int(0, (sizeof(part2) / sizeof(const char *)) - 1)];
	const char *third = part3[zone->random.Int(0, (sizeof(part3) / sizeof(const char *)) - 1)];
	const char *fourth = part4[zone->random.Int(0, (sizeof(part4) / sizeof(const char *)) - 1)];

	// if both of these are empty, we would get an illegally short name
	if (second == nullptr && third == nullptr)
		fourth = part4[(sizeof(part4) / sizeof(const char *)) - 1];

	// "ektik" isn't allowed either I guess?
	if (third == part3[3] && fourth == part4[3])
		fourth = part4[zone->random.Int(0, (sizeof(part4) / sizeof(const char *)) - 2)];

	// "Laser" isn't allowed either I guess?
	if (first == part1[3] && second == part2[3] && third == nullptr && fourth == part4[0])
		fourth = part4[zone->random.Int(1, (sizeof(part4) / sizeof(const char *)) - 2)];

	temp += first;
	if (second != nullptr)
		temp += second;
	if (third != nullptr)
		temp += third;
	temp += fourth;

	strn0cpy(name, temp.c_str(), 64);
}

void Mob::MakePet(uint16 spell_id, const char* pettype, const char *petname) {
	// petpower of -1 is used to get the petpower based on whichever focus is currently
	// equipped. This should replicate the old functionality for the most part.
	MakePoweredPet(spell_id, pettype, -1, petname);
}

// Split from the basic MakePet to allow backward compatiblity with existing code while also
// making it possible for petpower to be retained without the focus item having to
// stay equipped when the character zones. petpower of -1 means that the currently equipped petfocus
// of a client is searched for and used instead.
void Mob::MakePoweredPet(uint16 spell_id, const char* pettype, int16 petpower,
		const char *petname, float in_size) {
	// Sanity and early out checking first.
	//if(HasPet() || pettype == nullptr)
	//	return;

	int16 act_power = 0; // The actual pet power we'll use.
	if (petpower == -1) {
		if (IsClient()) {
			act_power = CastToClient()->GetFocusEffect(focusPetPower, spell_id);//Client only
		}
		else if (IsBot())
			act_power = CastToBot()->GetFocusEffect(focusPetPower, spell_id);
	}
	else if (petpower > 0)
		act_power = petpower;

	// optional rule: classic style variance in pets. Achieve this by
	// adding a random 0-4 to pet power, since it only comes in increments
	// of five from focus effects.

	//lookup our pets table record for this type
	PetRecord record;
	if(!content_db.GetPoweredPetEntry(pettype, act_power, &record)) {
		Message(Chat::Red, "Unable to find data for pet %s", pettype);
		LogError("Unable to find data for pet [{}], check pets table", pettype);
		return;
	}

	//find the NPC data for the specified NPC type
	const NPCType *base = content_db.LoadNPCTypesData(record.npc_type);
	if(base == nullptr) {
		Message(Chat::Red, "Unable to load NPC data for pet %s", pettype);
		LogError("Unable to load NPC data for pet [{}] (NPC ID [{}]), check pets and npc_types tables", pettype, record.npc_type);
		return;
	}

	//we copy the npc_type data because we need to edit it a bit
	auto npc_type = new NPCType;
	memcpy(npc_type, base, sizeof(NPCType));

	// If pet power is set to -1 in the DB, use stat scaling
	if ((IsClient() || IsBot()) && record.petpower == -1)
	{
		float scale_power = (float)act_power / 100.0f;
		if(scale_power > 0)
		{
			npc_type->max_hp *= (1 + scale_power);
			npc_type->current_hp = npc_type->max_hp;
			npc_type->AC *= (1 + scale_power);
			npc_type->level += 1 + ((int)act_power / 25) > npc_type->level + RuleR(Pets, PetPowerLevelCap) ? RuleR(Pets, PetPowerLevelCap) : 1 + ((int)act_power / 25); // gains an additional level for every 25 pet power
			npc_type->min_dmg = (npc_type->min_dmg * (1 + (scale_power / 2)));
			npc_type->max_dmg = (npc_type->max_dmg * (1 + (scale_power / 2)));
			npc_type->size = npc_type->size * (1 + (scale_power / 2)) > npc_type->size * 3 ? npc_type->size * 3 : npc_type-> size * (1 + (scale_power / 2));
		}
		record.petpower = act_power;
	}

	//Live AA - Elemental Durability
	int64 MaxHP = aabonuses.PetMaxHP + itembonuses.PetMaxHP + spellbonuses.PetMaxHP;

	if (MaxHP){
		npc_type->max_hp += (npc_type->max_hp*MaxHP)/100;
		npc_type->current_hp = npc_type->max_hp;
	}

	//TODO: think about regen (engaged vs. not engaged)

	// Pet naming:
	// 0 - `s pet
	// 1 - `s familiar
	// 2 - `s Warder
	// 3 - Random name if client, `s pet for others
	// 4 - Keep DB name
	// 5 - `s ward

	if (petname != nullptr) {
		// Name was provided, use it.
		strn0cpy(npc_type->name, petname, 64);
		EntityList::RemoveNumbers(npc_type->name);
		entity_list.MakeNameUnique(npc_type->name);
	} else if (record.petnaming == 0) {
		strcpy(npc_type->name, GetCleanName());
		npc_type->name[25] = '\0';
		strcat(npc_type->name, "`s_pet");
	} else if (record.petnaming == 1) {
		strcpy(npc_type->name, GetName());
		npc_type->name[19] = '\0';
		strcat(npc_type->name, "`s_familiar");
	} else if (record.petnaming == 2) {
		strcpy(npc_type->name, GetName());
		npc_type->name[21] = 0;
		strcat(npc_type->name, "`s_Warder");
	} else if (record.petnaming == 4) {
		// Keep the DB name
	} else if (record.petnaming == 3 && IsClient()) {
		GetRandPetName(npc_type->name);
	} else if (record.petnaming == 5 && IsClient()) {
		strcpy(npc_type->name, GetName());
		npc_type->name[24] = '\0';
		strcat(npc_type->name, "`s_ward");
	} else {
		strcpy(npc_type->name, GetCleanName());
		npc_type->name[25] = '\0';
		strcat(npc_type->name, "`s_pet");
	}

	// Beastlord Pets
	if (record.petnaming == 2) {
		uint16 race_id = GetBaseRace();

		auto d = content_db.GetBeastlordPetData(race_id);

		npc_type->race        = d.race_id;
		npc_type->texture     = d.texture;
		npc_type->helmtexture = d.helm_texture;
		npc_type->gender      = d.gender;
		npc_type->luclinface  = d.face;

		npc_type->size *= d.size_modifier;
	}

	// handle monster summoning pet appearance
	if(record.monsterflag) {
		uint32 monsterid = 0;

		// get a random npc id from the spawngroups assigned to this zone
		auto query = StringFormat("SELECT npcID "
									"FROM (spawnentry INNER JOIN spawn2 ON spawn2.spawngroupID = spawnentry.spawngroupID) "
									"INNER JOIN npc_types ON npc_types.id = spawnentry.npcID "
									"WHERE spawn2.zone = '%s' AND npc_types.bodytype NOT IN (11, 33, 66, 67) "
									"AND npc_types.race NOT IN (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 44, "
									"55, 67, 71, 72, 73, 77, 78, 81, 90, 92, 93, 94, 106, 112, 114, 127, 128, "
									"130, 139, 141, 183, 236, 237, 238, 239, 254, 266, 329, 330, 378, 379, "
									"380, 381, 382, 383, 404, 522) "
									"ORDER BY RAND() LIMIT 1", zone->GetShortName());
		auto results = content_db.QueryDatabase(query);
		if (!results.Success()) {
			safe_delete(npc_type);
			return;
		}

		if (results.RowCount() != 0) {
			auto row = results.begin();
			monsterid = Strings::ToInt(row[0]);
		}

		// since we don't have any monsters, just make it look like an earth pet for now
		if (monsterid == 0)
			monsterid = 567;

		// give the summoned pet the attributes of the monster we found
		const NPCType* monster = content_db.LoadNPCTypesData(monsterid);
		if(monster) {
			npc_type->race = monster->race;
			npc_type->size = monster->size;
			npc_type->texture = monster->texture;
			npc_type->gender = monster->gender;
			npc_type->luclinface = monster->luclinface;
			npc_type->helmtexture = monster->helmtexture;
			npc_type->herosforgemodel = monster->herosforgemodel;
		} else
			LogError("Error loading NPC data for monster summoning pet (NPC ID [{}])", monsterid);
	}

	//this takes ownership of the npc_type data
	auto npc = new Pet(npc_type, this, (PetType)record.petcontrol, spell_id, record.petpower);

	if (IsClient()) {
		auto pn = CastToClient()->GetPetVanityName(npc->GetPetOriginClass());
		if (!pn.empty()) {
			npc->SetName(pn.c_str());
			npc->TempName(pn.c_str());
		}
	}

	// Now that we have an actual object to interact with, load
	// the base items for the pet. These are always loaded
	// so that a rank 1 suspend minion does not kill things
	// like the special back items some focused pets may receive.
	uint32 petinv[EQ::invslot::EQUIPMENT_COUNT];
	memset(petinv, 0, sizeof(petinv));
	const EQ::ItemData *item = nullptr;

	if (content_db.GetBasePetItems(record.equipmentset, petinv)) {
		for (int i = EQ::invslot::EQUIPMENT_BEGIN; i <= EQ::invslot::EQUIPMENT_END; i++)
			if (petinv[i]) {
				item = database.GetItem(petinv[i]);
				npc->AddLootDrop(item, LootdropEntriesRepository::NewNpcEntity(), true);
			}
	}

	npc->UpdateEquipmentLight();

	// finally, override size if one was provided
	if (in_size > 0.0f) {
		npc->size = in_size;
		LogDebug("Size was overwritten to: [{}]", in_size);
	}

	npc->SetOwnerID(GetID());
	entity_list.AddNPC(npc, true, true);
	AddPet(npc);
	// We need to handle PetType 5 (petHatelist), add the current target to the hatelist of the pet

	if (record.petcontrol == petTargetLock)
	{
		Mob* m_target = GetTarget();

		bool activiate_pet = false;
		if (m_target && m_target->GetID() != GetID()) {

			if (spells[spell_id].target_type == ST_Self) {
				float distance = CalculateDistance(m_target->GetX(), m_target->GetY(), m_target->GetZ());
				if (distance <= 200) { //Live distance on targetlock pets that self cast. No message is given if not in range.
					activiate_pet = true;
				}
			}
			else {
				activiate_pet = true;
			}
		}

		if (activiate_pet){
			npc->AddToHateList(m_target, 1);
			npc->SetPetTargetLockID(m_target->GetID());
			npc->SetSpecialAbility(SpecialAbility::AggroImmunity, 1);
		}
		else {
			npc->CastSpell(SPELL_UNSUMMON_SELF, npc->GetID()); //Live like behavior, damages self for 20K
			if (!npc->HasDied()) {
				npc->Kill(); //Ensure pet dies if over 20k HP.
			}
		}
	}

	if (IsClient()) {
		CastToClient()->DoPetBagResync(npc->GetPetOriginClass());

		auto inst = GetInv().GetItem(EQ::invslot::slotAmmo);
		if (!inst) {
			return;
		}

		auto wand = inst->GetItem();
		if (!wand || wand->Click.Effect <= 0) {
			return;
		}

		if (strncmp(wand->Name, "Petamorph Wand", 14) == 0) {
			npc->ApplySpellBuff(wand->Click.Effect);
		}
	}
}
void NPC::TryDepopTargetLockedPets(Mob* current_target) {

	if (!current_target || (current_target && (current_target->GetID() != GetPetTargetLockID()) || current_target->IsCorpse())) {

		//Use when swarmpets are set to auto lock from quest or rule
		if (GetSwarmInfo() && GetSwarmInfo()->target) {
			Mob* owner = entity_list.GetMobID(GetSwarmInfo()->owner_id);
			if (owner) {
				owner->SetTempPetCount(owner->GetTempPetCount() - 1);
			}
			Depop();
			return;
		}
		//Use when pets are given petype 5
		if (IsPet() && GetPetType() == petTargetLock && GetPetTargetLockID()) {
			CastSpell(SPELL_UNSUMMON_SELF, GetID()); //Live like behavior, damages self for 20K
			if (!HasDied()) {
				Kill(); //Ensure pet dies if over 20k HP.
			}
			return;
		}
	}
}

std::string Mob::GenerateDruidPetName() {
	std::vector<std::string> bearNames = {
		"Tiny_Grizzle", "Snugglepaws", "Honey_Nibbles", "Bearly_There", "Cuddlycub", "Fuzzlet",
		"Pint-Sized_Paws", "Mini_Growl", "Buttonbear", "Teacup_Teddie", "Beary_McBearface",
		"Bearlock_Holmes", "Bearon_von_Growl", "Bearcules", "Winnie_the_Boo", "Grizzly_Adams",
		"Bear_Grylls", "Bearfoot", "Bearth_Vader", "Bearin'_Square", "Paw_Bear",
		"Bearzooka", "Bear_Hugz", "Bearister", "Gummy_Bearson", "Bearalicious",
		"Robin_Hoodbear", "Bearthoven", "Sir_Growls-a-Lot", "Bearington",
		"Honeybear_Hound", "Bearminator", "Bear_Necessities", "Grizz_Lee",
		"Polar_Oppawsite", "Growlbert_Einstein", "Bearoness", "Bearrific",
		"Bearcat", "Bearly_Legal", "Unbearlievable", "Teddy_Ruxbin", "Bear_Hugger",
		"Bearoness_von_Snuggles", "Bearbie_Doll", "Clawdia_Pawlsen", "Grizzelda",
		"Fuzz_Lightyear", "Pawdrey_Hepbear", "Furrari", "Bearbados_Slim", "Bearlin",
		"Furrnando", "Growlberto", "Bearloaf", "Bearianna_Grande", "Bearon_the_Red",
		"Clawrence_of_Arabia", "Paddingpaw", "Pawtrick_Swayze", "Bearami_Brown",
		"Grizzabella", "Bearlentine", "Bearthday_Boy", "Paw_McCartney", "Clawdette",
		"Bearon_Brando", "Beartholomew", "Bear_Hugington", "Fluff_Daddy", "Chewbearca",
		"Growldemort", "Bearicane", "Bearlosaurus_Rex", "Bear-lenium_Falcon", "Bearborator",
		"Bear_Fury", "Polar_Prowler", "Cuddle_Champion", "Snuggle_Monarch", "Honey_Whiskers",
		"Growl_Mage", "Bear_Wizard", "Fuzzy_Duchess", "Bear_Sentinel", "Claw_Commander",
		"Snuggle_Baron", "Bear_Prince", "Furry_Beast", "Growl_Titan", "Bear_Knight",
		"Honey_Rider", "Bear_Admiral", "Cuddle_Legend", "Bear_Hero", "Snuggle_Duke",
		"Bear_Czar", "Growl_Lord", "Furry_Tyrant", "Honey_Emperor", "Bear_Prowler",
		"Claw_Pirate", "Bear_Paladin", "Fuzzy_Lord", "Growl_Baron", "Snuggle_Templar",
		"Bear_General", "Honey_Duchess", "Bear_Flint", "Cuddle_Scout", "Furry_Marshal",
		"Bear_Hurricane", "Polar_Hound", "Growly_Paw", "Bear_Shadow", "Snuggle_Beast",
		"Bear_Overlord", "Honey_Baroness", "Furry_Bandit", "Bear_Pathfinder", "Paw_Explorer",
		"Snuggle_Wizard", "Bear_Pioneer", "Growl_Scribe", "Polar_Squire", "Honey_Vanguard",
		"Bear_Aurora", "Grizzly_Crusader", "Bear_Brigadier", "Fuzz_Sorcerer", "Claw_Illusionist",
		"Bear_Vanguard", "Snuggle_Wanderer", "Fuzzy_Phoenix", "Bear_Bishop", "Growl_Warlord",
		"Bear_Fiend", "Polar_Vizier", "Cuddle_Curator", "Furry_Avenger", "Snuggle_Harbinger",
		"Bear_Warden", "Growl_Chancellor", "Bear_Fury", "Honey_Sorceress", "Bear_Treasure",
		"Paw_Pioneer", "Bear_Sovereign", "Fuzz_Champion", "Bear_Voyager", "Claw_Saint",
		"Bear_Princess", "Growl_Gladiator", "Bear_Monarch", "Snuggle_Baroness", "Polar_Ruler",
		"Honey_Explorer", "Bear_Pathmaster", "Claw_Sentinel", "Bear_Guardian", "Fuzzy_Hero",
		"Bear_Medic", "Polar_Watcher", "Snuggle_Warrior", "Bear_Blade", "Honey_Bard",
		"Bear_Bard", "Growly_Leader", "Bear_Captain", "Cuddle_Ruler", "Polar_Marshal",
		"Bear_Judge", "Snuggle_Whisperer", "Bear_Lightbringer", "Fuzzy_Aurora", "Growl_Vanguard",
		"Claw_and_Effect", "Bear_to_Dream", "Honey_Fluff", "Fur_Real", "Pawfect_Buddy",
		"Growly_McGraw", "Bear_Snapper", "Honey_Munchkin", "Bearly_Believable", "Claw_of_Paws",
		"Polar_Express", "Bearhug_Brigade", "Fur_Majesty", "Honey_Pawseidon", "Bear_Scape",
		"Snuggle_Overlord", "Growly_Knight", "Bearly_Magic", "Honey_Pawradise", "Furry_Captain",
		"Bear_Horizons", "Growly_Treasure", "Bear_Panther", "Honey_Wonderpaw", "Claw_Voyager",
		"Bear_Legend", "Growl_Drifter", "Honey_Cubcake", "Bear_Flare", "Snuggle_Fortress",
		"Fuzzy_Brigadier", "Bear_Frontier", "Polar_Pioneer", "Claw_Barista", "Honey_Pawsitive",
		"Bear_with_Me", "Fur_Eternity", "Growly_Pride", "Bearly_Bold", "Snuggle_Sentinel",
		"Bear_Moonlight", "Claw_Flash", "Honey_Pawnder", "Bear_Savior", "Furry_Rogue",
		"Paw_Brigadier", "Growly_Hero", "Snuggle_Scribe", "Honey_Frostbite", "Bearly_Pawsitive",
		"Fuzzy_Hero", "Growlstorm", "Bear_Shadowfluff", "Honeycomb_Crusader", "Polar_Sentinel",
		"Bearly_Brilliant", "Snuggle_Fox", "Bear_Dynamo", "Honey_Pawrtal", "Claw_Marauder",
		"Bear_Flow", "Snuggle_Pal", "Growly_Mystic", "Honey_Lightbearer", "Bear_Fizzle",
		"Polar_Pawprint", "Growly_Mayhem", "Bear_Prowler", "Honeybear_Claw", "Clawbear_Commander",
		"Bear_Stormbear", "Furrious_Marshal", "Snuggle_Shield", "Honey_Glacier",
		"Bear_Battlecry", "Paw_Strike", "Growly_Pawsson", "Snuggle_Cuddler", "Bear_Plum",
		"Honey_Rosepaw", "Claw_Mark", "Bear_Courage", "Snuggle_Mistress", "Growly_Tigerpaw",
		"Polar_Howler", "Bear_Fluffball", "Honey_Frostwing", "Snuggle_Bearkfast", "Bear_Virtue",
		"Growly_Apex", "Honey_Sweetpaw", "Bearly_Noticeable", "Polar_Bravery", "Growl_Chieftain",
		"Bear_Mountainpaw", "Snuggle_Merchant", "Honey_Peace", "Bearly_Fierce", "Clawful_Joker",
		"Bear_Jokester", "Honey_Snowpaw", "Snuggle_Blast", "Bear_Sparkle", "Growly_Dragon",
		"Honey_Raider", "Bearly_Popular", "Fuzzy_Skipper", "Bear_Shadowstrike", "Snuggle_Luminator",
		"Honey_Pursuit", "Bear_Explorer", "Claw_Lord", "Growly_McClaw", "Polar_Furrow",
		"Honey_Fuzzball", "Snuggle_Knight", "Bear_Wanderpaw", "Growl_Majestic", "Bearly_Twilight",
		"Honey_Brightpaw", "Bear_Champion", "Snuggle_Diver", "Polar_Guard", "Growly_Scribe",
		"Bearly_Marvelous", "Honey_Hopepaw", "Bear_Thunderfluff", "Furriest_Paw", "Snuggle_Delight",
		"Bear_Pawlace", "Growl_Seeker", "Honey_Trailblazer", "Bearly_Tangible", "Clawful_Princess",
		"Bear_Mystery", "Growl_Crusader", "Honey_Dreamer", "Snuggle_Harbinger", "Bear_Prophet",
		"Polar_Stormbear", "Bear_Spiritbear", "Fuzzy_Sentinel", "Growly_Overseer", "Honey_Frostfluff",
		"Bear_Rogue", "Snuggle_Fury", "Bearly_Roaring", "Growl_Commander", "Polar_Visionary",
		"Bearly_Majestic", "Honey_Pawtroller", "Snuggle_Hopebear", "Fur_Keeper", "Growl_Lorekeeper",
		"Bear_Honor", "Snuggle_Pawmaster", "Honey_Fluffington", "Bearly_Hopeful", "Growly_Custodian",
		"Honey_Bearister", "Claw_Majesty", "Bearly_Hope", "Growly_Highness", "Polar_Whiskers",
		"Honey_Pawssible", "Bear_Battlelord", "Snuggle_Gentleclaw", "Growly_Fangkeeper", "Bear_Willowpaw",
		"Honey_Mysticpaw", "Furry_Leader", "Bear_Howler", "Growl_Fluffmaster", "Polar_Majesty"
	};

	return bearNames[zone->random.Roll0(bearNames.size()-1)];
}

std::string Mob::GenerateUndeadPetName() {
    std::vector<std::string> prefixes = {
        "Mor", "Skel", "Grim", "Varn", "Mar", "Karn", "Zor", "Gor", "Thal", "Tor", "Nar", "Thrax",
        "Shad", "Vel", "Xyl", "Eld", "Zar", "Thar", "Lur", "Vor", "Dra", "Amun"
    };

    std::vector<std::string> middles = {
        "ak", "or", "th", "ar", "al", "ro", "im", "uth", "on", "an", "en", "ol", "amun",
        "rax", "drim", "vath", "ris", "ros", "vok", "nis", "rok", "rath", "lor", "", "", "", ""
    };

    std::vector<std::string> suffixes = {
        "rik", "thos", "nar", "grim", "thal", "ok", "ath", "ur", "mar", "oth", "ros", "ak", "dar",
        "thar", "is", "al", "ar", "os", "eth", "or", "ith", "as", "ra", "", "", "", ""
    };

    std::string name = prefixes[zone->random.Roll0(static_cast<int>(prefixes.size()))] +
                       middles[zone->random.Roll0(static_cast<int>(middles.size()))] +
                       suffixes[zone->random.Roll0(static_cast<int>(suffixes.size()))];

    return name;
}

std::string Mob::GenerateElementalPetName() {
    std::vector<std::string> part1 = {"G", "J", "K", "L", "V", "X", "Z", "T", "N", "M", "R", "S"};
    std::vector<std::string> part2 = {"", "ab", "ar", "as", "eb", "en", "ib", "ob", "on", "ul", "ix", "al"};
    std::vector<std::string> part3 = {"", "an", "ar", "ek", "ob", "or", "us", "al", "is", "um"};
    std::vector<std::string> part4 = {"er", "ab", "n", "tik", "eth", "os", "ar", "ir", "is"};

    const std::string& first = part1[zone->random.Roll0(part1.size())];
    const std::string& second = part2[zone->random.Roll0(part2.size())];
    const std::string& third = part3[zone->random.Roll0(part3.size())];
    const std::string& fourth = part4[zone->random.Roll0(part4.size())];

    std::string name = first + second + third + fourth;

    return name;
}

std::string Mob::GenerateBeastlordPetName() {
    std::map<uint16_t, std::vector<std::string>> prefixes = {
        {Race::Human, {"Prowl", "Claw", "Fang", "Swift", "Shadow", "Mane", "Stalk", "Hunter", "Stride", "Velvet",
                       "Night", "Steel", "Strike", "Bold", "Fleet", "Track", "Glide", "Chase", "Pace", "Wind"}},
        {Race::Erudite, {"Kob", "Snarl", "Growl", "Blood", "Fang", "Gnash", "Rend", "Dark", "Howl", "Lurk",
                         "Shade", "Savage", "Whisper", "Veil", "Shroud", "Fury", "Bite", "Roar", "Ash", "Net"}},
        {Race::WoodElf, {"Verd", "Green", "Leaf", "Glen", "Sylv", "Forest", "Lush", "Canopy", "Wild", "Briar",
                         "Spring", "Meadow", "Moss", "Fern", "Blossom", "Dew", "Glade", "Thicket", "Twig", "Sap"}},
        {Race::HighElf, {"Azure", "Sky", "Mist", "Blue", "Gale", "Wind", "Celest", "Cloud", "Breeze", "Rain",
                         "Ray", "Glow", "Beam", "Auror", "Star", "Frost", "Shine", "Sun", "Zeph", "Lumin"}},
        {Race::DarkElf, {"Night", "Shade", "Venom", "Spider", "Web", "Dus", "Dark", "Fang", "Shadow", "Silk",
                         "Poison", "Gloom", "Twilight", "Abyss", "Dagger", "Grim", "Obsid", "Murk", "Rift", "Haze"}},
        {Race::HalfElf, {"Flare", "Crimson", "Scarlet", "Ruby", "Red", "Flame", "Burn", "Spark", "Ember", "Blaze",
                         "Cinder", "Gleam", "Radiant", "Inferno", "Ignis", "Fire", "Light", "Dawn", "Torch", "Ray"}},
        {Race::Dwarf, {"Gob", "Rock", "Iron", "Stone", "Smash", "Forge", "Ore", "Granite", "Hammer", "Anvil",
                       "Crush", "Might", "Hard", "Rough", "Bold", "Peak", "Mine", "Steel", "Grind", "Brawn"}},
        {Race::Halfling, {"Slink", "Viper", "Serp", "Coil", "Hiss", "Slith", "Fang", "Venom", "Scale", "Rept",
                          "Twist", "Loop", "Slide", "Slime", "Slink", "Wrap", "Shadow", "Tail", "Spine", "Crawl"}},
        {Race::Troll, {"Croak", "Swamp", "Marsh", "Scale", "Snap", "River", "Mud", "Grim", "Bog", "Reek",
                       "Creak", "Drip", "Mire", "Weed", "Rot", "Snag", "Tide", "Crust", "Filth", "Silt"}},
        {Race::Ogre, {"Grizz", "Bear", "Bruin", "Growl", "Fur", "Mighty", "Tusker", "Grumble", "Rumble", "Boulder",
                      "Fang", "Beast", "Mass", "Brute", "Tough", "Broad", "Heavy", "Thump", "Claw", "Shag"}},
        {Race::Barbarian, {"Wolf", "Howl", "Prowl", "Moon", "Lup", "Snow", "Fang", "Frost", "Alpha", "Winter",
                           "Pack", "Storm", "White", "Night", "Hunter", "Trail", "Ice", "Cold", "Tundra", "Steel"}},
        {Race::Gnome, {"Junk", "Rust", "Bolt", "Gear", "Scrap", "Crank", "Cobb", "Tink", "Gadget", "Grind",
                       "Sprock", "Screw", "Weld", "Nut", "Spark", "Churn", "Whirl", "Clink", "Tick", "Whisk"}},
        {Race::Froglok, {"Spore", "Glow", "Mold", "Sprout", "Fung", "Shroom", "Cap", "Lumin", "Moss", "Puff",
                         "Damp", "Myc", "Glare", "Slime", "Water", "Lagoon", "Lush", "Wet", "Marsh", "Trill"}},
        {Race::Drakkin, {"Gold", "Aur", "Shin", "Gleam", "Drak", "Solar", "Dawn", "Flame", "Cinder", "Sun",
                         "Bright", "Flash", "Fire", "Blaze", "Spark", "Infer", "Radiant", "Star", "Flare", "Lume"}},
        {Race::VahShir, {"Stripe", "Tiger", "Claw", "Roa", "Fang", "Savage", "Wild", "Pred", "Stalk", "Hunter",
                         "Prowl", "Rage", "Chase", "Leap", "Bound", "Jungle", "Pack", "Slash", "Growl", "Rush"}},
        {Race::Iksar, {"Scale", "Wyrm", "Slink", "Slith", "Lash", "Crawl", "Spine", "Hiss", "Venom", "Rept",
                       "Glide", "Drake", "Slime", "Spire", "Tail", "Drag", "Shade", "Quill", "Shadow", "Fang"}}
    };

	std::map<uint16_t, std::vector<std::string>> suffixes = {
		{Race::Human, {"paw", "stalker", "runner", "prowl", "mane", "stride", "shadow", "step", "fang", "",
					"dash", "mark", "pace", "hunt", "trail", "roam", "", "rush", "path", ""}},
		{Race::Erudite, {"fang", "howl", "snap", "jaw", "shadow", "bite", "blood", "hunter", "lash", "",
						"scar", "strike", "shade", "", "venom", "creep", "", "prowler", "", ""}},
		{Race::WoodElf, {"wing", "leaf", "glen", "glade", "whisper", "shade", "tree", "branch", "bough", "",
						"song", "root", "sap", "", "bark", "twig", "", "stem", "verdant", ""}},
		{Race::HighElf, {"breeze", "cloud", "wind", "song", "ray", "light", "beam", "sky", "rain", "",
						"mist", "shine", "gleam", "", "haze", "dawn", "", "flare", "aura", ""}},
		{Race::DarkElf, {"fang", "silk", "shadow", "creep", "web", "strike", "shade", "gloom", "venom", "",
						"shroud", "rift", "dusk", "", "scar", "", "night", "veil", "abyss", ""}},
		{Race::HalfElf, {"flare", "wing", "burn", "spark", "scale", "dash", "flame", "singe", "ember", "",
						"ray", "light", "gleam", "", "blaze", "fire", "", "torch", "inferno", ""}},
		{Race::Dwarf, {"smash", "stone", "strike", "fist", "crag", "forge", "iron", "hammer", "boulder", "",
					"peak", "ore", "cliff", "", "rock", "steel", "", "anvil", "grind", ""}},
		{Race::Halfling, {"coil", "hiss", "venom", "slither", "scale", "serp", "strike", "slide", "lash", "",
						"spine", "fang", "tail", "", "wrap", "loop", "", "shadow", "slime", ""}},
		{Race::Troll, {"snap", "tooth", "scale", "clamp", "jaw", "bite", "bog", "swamp", "grim", "",
					"mire", "rot", "weed", "", "silt", "mud", "", "creek", "snag", ""}},
		{Race::Ogre, {"paw", "growl", "bite", "claw", "stalk", "shadow", "roar", "strike", "maw", "",
					"crush", "rage", "bulk", "", "thump", "shag", "", "beast", "brute", ""}},
		{Race::Barbarian, {"howl", "moon", "prowler", "hunter", "fang", "shadow", "snow", "frost", "alpha", "",
						"lup", "storm", "trail", "", "tundra", "ice", "", "roam", "wolf", ""}},
		{Race::Gnome, {"bot", "snap", "crank", "rivet", "clank", "weld", "cog", "bolt", "gear", "",
					"grind", "churn", "whirl", "", "tick", "spark", "", "crash", "whir", ""}},
		{Race::Froglok, {"spore", "bloom", "fungus", "cap", "puff", "shroom", "light", "sprout", "moss", "",
						"glow", "damp", "slime", "", "marsh", "drip", "", "lagoon", "pond", ""}},
		{Race::Drakkin, {"wing", "ray", "shine", "glow", "fire", "dawn", "flame", "scale", "gold", "",
						"flare", "light", "star", "", "spark", "aura", "", "sun", "bright", ""}},
		{Race::VahShir, {"stripe", "claw", "prowl", "stalker", "roar", "fang", "wild", "savage", "hunter", "",
						"pounce", "leap", "slash", "", "growl", "rush", "", "bound", "pack", ""}},
		{Race::Iksar, {"scale", "fang", "lash", "strike", "slink", "venom", "hiss", "shadow", "coil", "",
					"crawl", "shade", "drake", "", "tail", "quill", "", "glide", "spire", ""}}
	};

    // Random fallback
    std::vector<std::string> fallbackPrefixes = {"Shadow", "Dark", "Night", "Swift", "Fang", "Blood", "Venom", "Prowl", "Claw", "Savage"};
    std::vector<std::string> fallbackSuffixes = {"stalker", "fang", "runner", "strike", "lash", "hunter", "shade", "prowler", "scar", ""};

    uint16_t race = GetBaseRace();
    const std::vector<std::string>& racePrefixes = prefixes.count(race) ? prefixes[race] : fallbackPrefixes;
    const std::vector<std::string>& raceSuffixes = suffixes.count(race) ? suffixes[race] : fallbackSuffixes;

    return racePrefixes[zone->random.Roll0(racePrefixes.size())] + raceSuffixes[zone->random.Roll0(raceSuffixes.size())];
}

std::string Mob::GenerateShamanPetName() {
    static const std::vector<std::string> prefixes = {
        "Spirit", "Ethereal", "Ghost", "Ancestor", "Shadow", "Phantom", "Mystic", "Spectral",
        "Echo", "Soul", "Wraith", "Haunted", "Silent", "Wisdom", "Moon", "Pale", "Frost", "Dusk"
    };

    static const std::vector<std::string> middles = {
        "Howl", "Fang", "Hunter", "Paw", "Prowler", "Alpha", "Wolf", "Pack", "Shade",
        "Walker", "Guardian", "Sentinel", "Stalker", "Bound", "Claw", "Spirit", "Hound", "Watcher"
    };

    static const std::vector<std::string> suffixes = {
        "", "of Echoes", "of Shadows", "of the Moon", "of Frost", "of Ancestors", "of Night",
        "of Silence", "of Whispers", "of Spirits", "of the Hunt", "of Warding", "of the Pack",
        "of the Alpha", "of Wisdom", "of the Ether", "of Dusk", "of Winter"
    };

    const std::string& prefix = prefixes[zone->random.Roll0(prefixes.size())];
    const std::string& middle = middles[zone->random.Roll0(middles.size())];
    const std::string& suffix = suffixes[zone->random.Roll0(suffixes.size())];

    std::string name = prefix + " " + middle;
    if (!suffix.empty()) {
        name += " " + suffix;
    }

    return name;
}

std::string Mob::GenerateEnchanterPetName() {
    static const std::vector<std::string> enchanterAnimationNames = {
        "Phantom_Fencer", "Spectral_Blade", "Haunting_Slasher", "Ghostly_Swordsman",
        "Ethereal_Duelist", "Shadow_Slicer", "Arcane_Prowler", "Wraith_Cutlass",
        "Runed_Slicer", "Spellbound_Fencer", "Floating_Blade", "Void_Saber",
        "Mystic_Flurry", "Enigma_Slasher", "Hexed_Swordsman", "Wraith_Slasher",
        "Charmed_Duelist", "Telekinetic_Flurry", "Spectral_Saber", "Spirit_Blade",
        "Haunted_Dagger", "Cursed_Sword", "Hexblade", "Void_Cutter", "Soulbound_Flurry",
        "Runed_Edge", "Ethereal_Cleaver", "Shrouded_Strike", "Invisible_Slasher",
        "Silent_Cutlass", "Phantom_Dagger", "Spirit_Cleaver", "Arcane_Saber",

        "Stabby_McStabface", "Floaty_Bois", "Slashy_McSlashface", "Pokey_McPokerson",
        "Cutty_McCutface", "Blade_Boi", "Bonky_Buddy", "Bouncy_Slasher", "Clangy_McClang",
        "Whirly_McWhirlerson", "Spin_to_Win", "Pokey_McFloaty", "Swingy_Boi",
        "Boop_Stick", "Spooky_Slasher", "Whacky_Floater", "The_Floating_Brigade",

        "Dancing_Swords", "Floating_Fury", "Invisible_Blade", "Cursed_Edge",
        "Runed_Strike", "Ethereal_Shiv", "Void_Whisper", "Arcane_Warrior",
        "Phantom_Flurry", "Spectral_Strike", "Silent_Blade", "Enchanted_Edge",
        "Shrouded_Cleaver", "Illusory_Striker", "Ghostblade", "Mystic_Slicer",

        "Clanging_Specter", "Twilight_Blade", "Bound_Slicer", "Ethereal_Warrior",
        "Spirit_Slicer", "Haunted_Slasher", "Runic_Whirlwind", "Shimmering_Saber",
        "Floating_Strike", "Shadow_Blade", "Poltergeist_Slasher", "Void_Sentinel",
        "Phantom_Striker", "Glowing_Cleaver", "Haunting_Shiv"
    };

    // Return a random name from the static list
    return enchanterAnimationNames[zone->random.Roll0(enchanterAnimationNames.size())];
}

/* This is why the pets ghost - pets were being spawned too far away from its npc owner and some
into walls or objects (+10), this sometimes creates the "ghost" effect. I changed to +2 (as close as I
could get while it still looked good). I also noticed this can happen if an NPC is spawned on the same spot of another or in a related bad spot.*/
Pet::Pet(NPCType *type_data, Mob *owner, PetType type, uint16 spell_id, int16 power)
: NPC(type_data, 0, owner->GetPosition() + glm::vec4(2.0f, 2.0f, 0.0f, 0.0f), GravityBehavior::Water)
{
	GiveNPCTypeData(type_data);
	SetPetType(type);
	SetPetPower(power);
	SetOwnerID(owner ? owner->GetID() : 0);
	SetPetSpellID(spell_id);

	// All pets start at false on newer clients. The client
	// turns it on and tracks the state.
	SetTaunting(false);

	// Older clients didn't track state, and default taunting is on (per @mackal)
	// Familiar and animation pets don't get taunt until an AA.
	if (owner && owner->IsClient()) {
		if (!(owner->CastToClient()->ClientVersionBit() & EQ::versions::maskUFAndLater)) {
			if (
				(GetPetType() != petFamiliar && GetPetType() != petAnimation) ||
				aabonuses.PetCommands[PET_TAUNT]
			) {
				SetTaunting(true);
			}
		}
	}



	// Class should use npc constructor to set light properties
}

bool ZoneDatabase::GetPetEntry(const std::string& pet_type, PetRecord *p)
{
	return GetPoweredPetEntry(pet_type, 0, p);
}

bool ZoneDatabase::GetPoweredPetEntry(const std::string& pet_type, int16 pet_power, PetRecord* r)
{
	const auto& l = PetsRepository::GetWhere(
		content_db,
		fmt::format(
			"`type` = '{}' AND `petpower` <= {} ORDER BY `petpower` DESC LIMIT 1",
			pet_type,
			pet_power <= 0 ? 0 : pet_power
		)
	);

	if (l.empty()) {
		return false;
	}

	auto &e = l.front();

	r->npc_type     = e.npcID;
	r->temporary    = e.temp;
	r->petpower     = e.petpower;
	r->petcontrol   = e.petcontrol;
	r->petnaming    = e.petnaming;
	r->monsterflag  = e.monsterflag;
	r->equipmentset = e.equipmentset;

	return true;
}

void Mob::ConfigurePetWindow(Mob* selected_pet) {
	if (!IsClient()) {
		return;
	}
	if (selected_pet && selected_pet->GetOwnerID() == GetID()) {
		auto this_client = CastToClient();
		auto pet_npc	 = selected_pet->CastToNPC();

		auto outapp = new EQApplicationPacket;
		auto outapp2 = new EQApplicationPacket;

		focused_pet_id = pet_npc->GetID();

		// Handle SPO? Maybe just Follow\Guard
		switch (pet_npc->GetPetOrder()) {
			case SPO_Follow:
				pet_npc->SetEntityVariable("IgnoreNextFollowCommand", "true");
				break;
			case SPO_Guard:
				pet_npc->SetEntityVariable("IgnoreNextGuardCommand", "true");
				break;
			case SPO_Sit:
				pet_npc->SetEntityVariable("IgnoreNextSitCommand", "true");
				break;
		}

		pet_npc->CreateDespawnPacket(outapp, false);
		pet_npc->CreateSpawnPacket(outapp2, this);

		this_client->QueuePacket(outapp);
		this_client->QueuePacket(outapp2);

		pet_npc->SendAppearancePacket(AppearanceType::Pet, GetID(), true, true);

		for (auto pet_iter : GetAllPets()) {
			if (pet_iter->GetID() != pet_npc->GetID()) {
				switch (pet_npc->GetPetOrder()) {
					case SPO_Follow:
						pet_iter->SetEntityVariable("IgnoreNextFollowCommand", "true");
						break;
					case SPO_Guard:
						pet_iter->SetEntityVariable("IgnoreNextGuardCommand", "true");
						break;
					case SPO_Sit:
						pet_iter->SetEntityVariable("IgnoreNextSitCommand", "true");
						break;
				}
				pet_iter->SendAppearancePacket(AppearanceType::Pet, GetID(), true, true);
			}
		}
		if (GetTarget() && GetTarget()->GetID() == pet_npc->GetID()) { pet_npc->SendBuffsToClient(this_client); }
		pet_npc->SendPetBuffsToClient();

		if (pet_npc->IsHeld()) { pet_npc->SetEntityVariable("IgnoreNextHoldCommand", "true"); }
		this_client->SetPetCommandState(PET_BUTTON_HOLD, pet_npc->IsHeld());

		if (pet_npc->IsTaunting()) { pet_npc->SetEntityVariable("IgnoreNextTauntCommand", "true"); }
		this_client->SetPetCommandState(PET_BUTTON_TAUNT, pet_npc->IsTaunting());

		if (pet_npc->IsGHeld()) { pet_npc->SetEntityVariable("IgnoreNextGHoldCommand", "true"); }
		this_client->SetPetCommandState(PET_BUTTON_GHOLD, pet_npc->IsGHeld());

		if (pet_npc->IsFocused()) { pet_npc->SetEntityVariable("IgnoreNextFocusCommand", "true"); }
		this_client->SetPetCommandState(PET_BUTTON_FOCUS, pet_npc->IsFocused());

		if (pet_npc->IsNoCast()) { pet_npc->SetEntityVariable("IgnoreNextSpellholdCommand", "true"); }
		this_client->SetPetCommandState(PET_BUTTON_SPELLHOLD, pet_npc->IsNoCast());

		if (pet_npc->GetPetOrder() == SPO_Follow) { pet_npc->SetEntityVariable("IgnoreNextFollowCommand", "true"); }
		this_client->SetPetCommandState(PET_BUTTON_FOLLOW, pet_npc->GetPetOrder() == SPO_Follow);

		if (pet_npc->GetPetOrder() == SPO_Guard) { pet_npc->SetEntityVariable("IgnoreNextGuardCommand", "true"); }
		this_client->SetPetCommandState(PET_BUTTON_GUARD, pet_npc->GetPetOrder() == SPO_Guard);

		if (pet_npc->GetPetOrder() == SPO_Sit) { pet_npc->SetEntityVariable("IgnoreNextSitCommand", "true"); }
		this_client->SetPetCommandState(PET_BUTTON_SIT, pet_npc->GetPetOrder() == SPO_Sit);

		safe_delete(outapp);
		safe_delete(outapp2);

		auto pet_target = GetTarget();
		if (pet_target) {
			auto app = new EQApplicationPacket(OP_PetHoTT, sizeof(ClientTarget_Struct));
			auto ct = (ClientTarget_Struct *)app->pBuffer;
			ct->new_target = pet_npc->GetTarget() ? pet_npc->GetTarget()->GetID() : 0;
			this_client->FastQueuePacket(&app);
		}
	}
}

// Get the ID of the pet at the given index
uint16 Mob::GetPetID(uint8 idx) const {
    // Check if the index is greater than or equal to the pet limit
    if (idx >= RuleI(Custom, AbsolutePetLimit)) {
        return 0;  // Return 0 if the index exceeds the pet limit
    }

    // If the index is within the bounds of the petids vector, return the pet ID at that index
    if (idx < petids.size()) {
        return petids[idx];
    } else {
        return 0;  // Return 0 if the index is out of bounds
    }
}

bool Mob::IsPetAllowed(uint16 spell_id) {
    ValidatePetList();
    int pet_count = petids.size();
    int cumulative_bitmask = 0;

    // Check if the total number of pets exceeds the allowed limit
    if (pet_count >= RuleI(Custom, AbsolutePetLimit)) {
        Message(Chat::SpellFailure, "You may not control any additional pets.");
        return false;
    }

    for (auto pet : GetAllPets()) {
        if (!pet) {
            continue;
        }

        uint16 origin_spell = 0;
        auto pet_buffs = pet->GetBuffs();

        // Find the spell that created the current pet
        for (int i = 0; i < pet->GetMaxTotalSlots(); i++) {
            if (IsCharmSpell(pet_buffs[i].spellid)) {
                origin_spell = pet_buffs[i].spellid;
                LogDebug("Found Charm Spell: [{}]", origin_spell);
                break;
            }
        }

        if (!origin_spell && pet->IsNPC()) {
            origin_spell = pet->CastToNPC()->GetPetSpellID();
        }

		// Check if this spell matches the origin spell of an existing pet
        if (origin_spell == spell_id) {
            Message(Chat::SpellFailure, "You may not control any additional pets of this type (%s).", spells[spell_id].name);
            return false;
        }

        // Aggregate the bitmask of the classes that can summon this pet
        for (int i = Class::Warrior; i <= Class::Berserker; i++) {
            if (GetSpellLevel(origin_spell, i) < UINT8_MAX) {
                cumulative_bitmask |= (1 << i); // Mark class `i` as having a pet
            }
        }
    }

    // Check if the new spell's classes overlap with the bitmask of existing pet classes
    for (int i = Class::Warrior; i <= Class::Berserker; i++) {
        if (GetSpellLevel(spell_id, i) < UINT8_MAX) { // This class can cast the spell
            if (cumulative_bitmask & (1 << i)) { // This class already has a pet
                Message(Chat::SpellFailure, "You may not control any additional pets for this class (%s).", GetClassIDName(i));
                return false;
            }
        }
    }

    return true; // Allow the pet
}

// Get the Mob instance of the pet at the given index
Mob* Mob::GetPet(uint8 idx) {
    // Check if the index is greater than or equal to the pet limit
    if (idx >= RuleI(Custom, AbsolutePetLimit)) {
        return nullptr;  // Return nullptr if the index exceeds the pet limit
    }

    // Check if the pet ID at the given index is valid
    if (!GetPetID(idx)) {
        return nullptr;  // Return nullptr if there is no pet ID at this index
    }

    // Retrieve the Mob associated with the pet ID
    const auto m = entity_list.GetMob(GetPetID(idx));
    if (!m) {
        RemovePet(idx);  // Remove the pet from the list if it no longer exists
        return nullptr;  // Return nullptr if the pet does not exist
    }

    // Verify that the retrieved Mob is still owned by this Mob
    if (m->GetOwnerID() != GetID()) {
        RemovePet(idx);  // Remove the pet from the list if it is no longer owned by this Mob
        return nullptr;  // Return nullptr if the ownership does not match
    }

    return m;  // Return the Mob instance of the pet if all checks pass
}

std::vector<Mob*> Mob::GetAllPets() {
	ValidatePetList();
    std::vector<Mob*> pets;

    for (uint16 pet_id : petids) {
        auto pet = entity_list.GetMob(pet_id);
        if (pet) {
            pets.push_back(pet);
        }
    }

    return pets;
}

Mob* Mob::GetPetByID(uint16 id) {
	ValidatePetList();
    // Iterate through the list of pet IDs
    for (uint16 pet_id : petids) {
        if (pet_id == id) {  // Check if the current pet ID matches the provided ID
            // Retrieve the Mob associated with the pet ID
            Mob* pet = entity_list.GetMob(pet_id);
            if (pet && pet->GetOwnerID() == GetID()) {  // Ensure the pet is still owned by this Mob
                return pet;  // Return the pet if it is owned by this Mob and matches the ID
            } else {
                // If the pet exists but isn't owned by this Mob, consider removing it from the list
                RemovePet(pet_id);  // Clean up the pet list if needed
            }
        }
    }

    return nullptr;  // Return nullptr if no pet with the given ID is found
}

// Remove the pet at the given index from the pet list
bool Mob::RemovePetByIndex(uint8 idx /*= 0*/) {
    // Check if the index is within the bounds of the petids vector
    if (idx >= petids.size()) {
        return false;  // Return false if the index is out of bounds
    }

    // Retrieve the Mob associated with the pet ID
    auto m = entity_list.GetMob(GetPetID(idx));
    if (m) {
        m->SetOwnerID(0);  // Detach the pet from its owner if the Mob exists
    }

    // Remove the pet ID from the vector
    petids.erase(petids.begin() + idx);

    return true;  // Return true to indicate successful removal
}

bool Mob::RemovePet(Mob* pet) {
    if (!pet) {
        return false;  // Return false if the provided Mob pointer is null
    }

    // Get the ID of the Mob to remove and delegate to RemovePet by ID
    return RemovePet(pet->GetID());
}

bool Mob::RemovePet(uint16 pet_id) {
    // Iterate through the petids vector to find and remove the pet by its ID
    for (auto it = petids.begin(); it != petids.end(); ++it) {
        if (*it == pet_id) {
            // Retrieve the Mob associated with the pet ID
            auto pet = entity_list.GetMob(pet_id);
            if (pet) {

				pet->SetOwnerID(0);  // Detach the pet from its owner
				pet->SendAppearancePacket(AppearanceType::Pet, 0, true, true);
				pet->SetPetType(petNone);
				pet->SetHeld(false);
				pet->SetGHeld(false);
				pet->SetNoCast(false);
				pet->SetFocused(false);
				pet->SetPetStop(false);
				pet->SetPetRegroup(false);
            }
            petids.erase(it);        // Remove the pet ID from the vector
			ValidatePetList();

			if (pet_id == focused_pet_id) {
				focused_pet_id = 0;
			}

            return true;             // Return true to indicate successful removal
        }
    }

    return false;  // Return false if the pet was not found in the list
}

// Remove all pets from the pet list
void Mob::RemoveAllPets() {
    // Iterate over each pet ID in the petids vector
    for (auto pet_id : petids) {
        // Retrieve the Mob associated with the pet ID
        auto pet = entity_list.GetMob(pet_id);
        if (pet) {
            pet->SetOwnerID(0);  // Detach the pet from its owner if the Mob exists
        }
    }

    // Clear the petids vector to remove all pet IDs
    petids.clear();
}

// Check if there is a valid pet at the given index
bool Mob::HasPet(uint8 idx) const {
    // Check if the petids vector is empty or the index is out of bounds
    if (petids.empty() || idx >= petids.size()) {
        return false;  // Return false if there are no pets or the index is invalid
    }

    // Retrieve the Mob instance of the pet at the given index
    auto m = entity_list.GetMob(petids[idx]);

    return m != nullptr;  // Return true if the pet exists, false otherwise
}

// Add a new pet to the pet list by Mob object; returns true if pet is on the list at the end of this operation
bool Mob::AddPet(Mob* newpet) {
    // Delegate to the AddPet by ID function
    return AddPet(newpet->GetID());
}

// Add a new pet to the pet list by pet ID; returns true if pet is on the list at the end of this operation
bool Mob::AddPet(uint16 pet_id) {
	ValidatePetList();
    // Retrieve the Mob associated with the given pet ID
    auto newpet = entity_list.GetMob(pet_id);
    if (!newpet) {
        return false;  // Return false if the Mob with the given ID does not exist
    }

    // Check if we have reached the absolute pet limit
    if (petids.size() >= RuleI(Custom, AbsolutePetLimit)) {
        return false;  // Return false if the limit is reached
    }

    // Check if this pet ID is already in the petids list
    for (auto id : petids) {
        if (id == pet_id) {
            return true;  // Return true if the pet is already in the list
        }
    }

    // Add the new pet's ID to the petids list and set its owner ID
    petids.push_back(pet_id);
    newpet->SetOwnerID(GetID());  // Set the owner ID to this mob's ID

	focused_pet_id = pet_id;
	ConfigurePetWindow(newpet);
	if (newpet->IsNPC() && IsClient() && !IsEffectInSpell(newpet->CastToNPC()->GetPetSpellID(), SE_Charm)) {
		CastToClient()->DoPetBagResync(newpet->CastToNPC()->GetPetOriginClass());
		CastToClient()->Save();
	}

    return true;  // Return true to indicate the pet was successfully added
}

void Mob::ValidatePetList() {
    for (auto it = petids.begin(); it != petids.end(); ) {
		auto pet = entity_list.GetMob(*it);
        if (!pet || !pet->GetOwner() && entity_list.GetNPCList().size() > 0) {
            LogDebug("Removing invalid pet ID [{}] from petids list for Mob [{}].", *it, GetCleanName());
            it = petids.erase(it);  // Remove invalid pet ID and advance the iterator
        } else {
            ++it;  // Advance the iterator if the ID is valid
        }
    }
}

// Set a pet into the given index location using a Mob object
bool Mob::SetPet(Mob* newpet, uint8 idx) {
    // Delegate to the SetPet by ID function
    return SetPet(idx, newpet->GetID());
}

// Set a pet into the given index location using a pet ID
bool Mob::SetPet(uint16 pet_id, uint8 idx) {
	ValidatePetList();
    // Retrieve the Mob associated with the given pet ID
    auto newpet = entity_list.GetMob(pet_id);
    if (!newpet) {
        return false;  // Return false if the Mob with the given ID does not exist
    }

    // Check if the index is within the allowed pet limit
    if (idx >= RuleI(Custom, AbsolutePetLimit)) {
        return false;  // Return false if the index exceeds the pet limit
    }

    // Ensure the petids vector is large enough to accommodate the index
    if (idx >= petids.size()) {
        // Resize the petids vector if necessary, filling new slots with 0 (invalid ID)
        petids.resize(idx + 1, 0);
    }

    // Check if this pet ID is already in the petids list
    for (auto id : petids) {
        if (id == pet_id) {
            return true;  // Return true if the pet is already in the list
        }
    }

    // Set the new pet ID at the given index
    petids[idx] = pet_id;
    newpet->SetOwnerID(GetID());  // Set the owner ID to this mob's ID

    return true;  // Return true to indicate the pet was successfully set
}

void NPC::GetPetState(SpellBuff_Struct *pet_buffs, uint32 *items, char *name) {
	//save the pet name
	strn0cpy(name, GetName(), 64);

	//save their items, we only care about what they are actually wearing
	memcpy(items, equipment, sizeof(uint32) * EQ::invslot::EQUIPMENT_COUNT);

	//save their buffs.
	for (int i=EQ::invslot::EQUIPMENT_BEGIN; i < GetPetMaxTotalSlots(); i++) {
		if (IsValidSpell(buffs[i].spellid)) {
			pet_buffs[i].spellid = buffs[i].spellid;
			pet_buffs[i].effect_type = i+1;
			pet_buffs[i].duration = buffs[i].ticsremaining;
			pet_buffs[i].level = buffs[i].casterlevel;
			pet_buffs[i].bard_modifier = 10;
			pet_buffs[i].counters = buffs[i].counters;
			pet_buffs[i].bard_modifier = buffs[i].instrument_mod;
			strcpy(pet_buffs[i].caster_name, buffs[i].caster_name);
		}
		else {
			pet_buffs[i].spellid = SPELL_UNKNOWN;
			pet_buffs[i].duration = 0;
			pet_buffs[i].level = 0;
			pet_buffs[i].bard_modifier = 10;
			pet_buffs[i].counters = 0;
		}
	}
}

void NPC::SetPetState(SpellBuff_Struct *pet_buffs, uint32 *items) {
	//restore their buffs...

	int i;
	for (i = 0; i < GetPetMaxTotalSlots(); i++) {
		for(int z = 0; z < GetPetMaxTotalSlots(); z++) {
		// check for duplicates
			if(IsValidSpell(buffs[z].spellid) && buffs[z].spellid == pet_buffs[i].spellid) {
				buffs[z].spellid = SPELL_UNKNOWN;
				pet_buffs[i].spellid = 0xFFFFFFFF;
			}
		}

		if (pet_buffs[i].spellid <= (uint32)SPDAT_RECORDS && pet_buffs[i].spellid != 0 && (pet_buffs[i].duration > 0 || pet_buffs[i].duration == -1)) {
			if(pet_buffs[i].level == 0 || pet_buffs[i].level > 100)
				pet_buffs[i].level = 1;
			buffs[i].spellid			= pet_buffs[i].spellid;
			buffs[i].ticsremaining		= pet_buffs[i].duration;
			buffs[i].casterlevel		= pet_buffs[i].level;
			buffs[i].casterid			= 0;
			buffs[i].counters			= pet_buffs[i].counters;
			buffs[i].hit_number			= spells[pet_buffs[i].spellid].hit_number;
			buffs[i].instrument_mod		= pet_buffs[i].bard_modifier;
			strcpy(buffs[i].caster_name, pet_buffs[i].caster_name);
		}
		else {
			buffs[i].spellid = SPELL_UNKNOWN;
			pet_buffs[i].spellid = 0xFFFFFFFF;
			pet_buffs[i].effect_type = 0;
			pet_buffs[i].level = 0;
			pet_buffs[i].duration = 0;
			pet_buffs[i].bard_modifier = 0;
		}
	}
	for (int j1=0; j1 < GetPetMaxTotalSlots(); j1++) {
		if (buffs[j1].spellid <= (uint32)SPDAT_RECORDS) {
			for (int x1=0; x1 < EFFECT_COUNT; x1++) {
				switch (spells[buffs[j1].spellid].effect_id[x1]) {
					case SE_AddMeleeProc:
					case SE_WeaponProc:
						// We need to reapply buff based procs
						// We need to do this here so suspended pets also regain their procs.
						AddProcToWeapon(GetProcID(buffs[j1].spellid,x1), false, 100+spells[buffs[j1].spellid].limit_value[x1], buffs[j1].spellid, buffs[j1].casterlevel, GetSpellProcLimitTimer(buffs[j1].spellid, ProcType::MELEE_PROC));
						break;
					case SE_DefensiveProc:
						AddDefensiveProc(GetProcID(buffs[j1].spellid, x1), 100 + spells[buffs[j1].spellid].limit_value[x1], buffs[j1].spellid, GetSpellProcLimitTimer(buffs[j1].spellid, ProcType::DEFENSIVE_PROC));
						break;
					case SE_RangedProc:
						AddRangedProc(GetProcID(buffs[j1].spellid, x1), 100 + spells[buffs[j1].spellid].limit_value[x1], buffs[j1].spellid, GetSpellProcLimitTimer(buffs[j1].spellid, ProcType::RANGED_PROC));
						break;
					case SE_Charm:
					case SE_Rune:
					case SE_NegateAttacks:
					case SE_Illusion:
						buffs[j1].spellid = SPELL_UNKNOWN;
						pet_buffs[j1].spellid = SPELLBOOK_UNKNOWN;
						pet_buffs[j1].effect_type = 0;
						pet_buffs[j1].level = 0;
						pet_buffs[j1].duration = 0;
						pet_buffs[j1].bard_modifier = 0;
						x1 = EFFECT_COUNT;
						break;
					// We can't send appearance packets yet, put down at CompleteConnect
				}
			}
		}
	}

	//restore their equipment...
	for (i = EQ::invslot::EQUIPMENT_BEGIN; i <= EQ::invslot::EQUIPMENT_END; i++) {
		if (items[i] == 0) {
			continue;
		}

		const EQ::ItemData *item2 = database.GetItem(items[i]);

		if (item2) {
			bool noDrop           = (item2->NoDrop == 0); // Field is reverse logic
			bool petCanHaveNoDrop = (RuleB(Pets, CanTakeNoDrop) && _CLIENTPET(this) && GetPetType() <= petOther);

			if (!noDrop || petCanHaveNoDrop) {
				AddLootDrop(item2, LootdropEntriesRepository::NewNpcEntity(), true);
			}
		}
	}
}

// Load the equipmentset from the DB. Might be worthwhile to load these into
// shared memory at some point due to the number of queries needed to load a
// nested set.
bool ZoneDatabase::GetBasePetItems(int32 equipmentset, uint32 *items) {
	if (equipmentset < 0 || items == nullptr)
		return false;

	// Equipment sets can be nested. We start with the top-most one and
	// add all items in it to the items array. Referenced equipmentsets
	// are loaded after that, up to a max depth of 5. (Arbitrary limit
	// so we don't go into an endless loop if the DB data is cyclic for
	// some reason.)
	// A slot will only get an item put in it if it is empty. That way
	// an equipmentset can overload a slot for the set(s) it includes.

	int depth = 0;
	int32 curset = equipmentset;
	int32 nextset = -1;
	uint32 slot;

	// outline:
	// get equipmentset from DB. (Mainly check if we exist and get the
	// nested ID)
	// query pets_equipmentset_entries with the set_id and loop over
	// all of the result rows. Check if we have something in the slot
	// already. If no, add the item id to the equipment array.
	while (curset >= 0 && depth < 5) {
		std::string  query = StringFormat("SELECT nested_set FROM pets_equipmentset WHERE set_id = '%d'", curset);
		auto results = QueryDatabase(query);
		if (!results.Success()) {
			return false;
		}

		if (results.RowCount() != 1) {
			// invalid set reference, it doesn't exist
			LogError("Error in GetBasePetItems equipment set [{}] does not exist", curset);
			return false;
		}

		auto row = results.begin();
		nextset = Strings::ToInt(row[0]);

		query = StringFormat("SELECT slot, item_id FROM pets_equipmentset_entries WHERE set_id='%d'", curset);
		results = QueryDatabase(query);
		if (results.Success()) {
			for (row = results.begin(); row != results.end(); ++row)
			{
				slot = Strings::ToInt(row[0]);

				if (slot > EQ::invslot::EQUIPMENT_END)
					continue;

				if (items[slot] == 0)
					items[slot] = Strings::ToInt(row[1]);
			}
		}

		curset = nextset;
		depth++;
	}

	return true;
}

bool Pet::CheckSpellLevelRestriction(Mob *caster, uint16 spell_id)
{
	auto owner = GetOwner();
	if (owner)
		return owner->CheckSpellLevelRestriction(caster, spell_id);
	return true;
}

BeastlordPetData::PetStruct ZoneDatabase::GetBeastlordPetData(uint16 race_id) {
	BeastlordPetData::PetStruct d;

	const auto& e = PetsBeastlordDataRepository::FindOne(*this, race_id);

	if (!e.player_race) {
		return d;
	}

	d.race_id       = e.pet_race;
	d.texture       = e.texture;
	d.helm_texture  = e.helm_texture;
	d.gender        = e.gender;
	d.size_modifier = e.size_modifier;
	d.face          = e.face;

	return d;
}
