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


	npc->NamePetOnSpellID(spell_id, petname);


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


	if (IsClient()) {
		CastToClient()->DoPetBagResync();
	}

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

void NPC::NamePetOnSpellID(uint16 spell_id, const char* static_name) {
	ValidatePetList();
    Mob* owner = GetOwner();
    if (!owner) {
        return;
    }

    std::vector<std::string> existing_pet_names;
    for (auto pet : owner->GetAllPets()) {
        existing_pet_names.push_back(pet->GetCleanName());
    }

	std::string tmp_lastname;
	std::string tmp_name = (static_name == nullptr) ? "" : static_name;

    auto getRandomBearName = [&]() -> std::string {
        std::vector<std::string> bearNames = {
            "Yogi", "Boo", "Pip", "Nugget", "Snick", "Pebble", "Fizz", "Munch", "Squirt", "Binky",
            "Tiny Grizzle", "Snugglepaws", "Honey Nibbles", "Bearly There", "Cuddlycub", "Fuzzlet",
            "Pint-Sized Paws", "Mini Growl", "Buttonbear", "Teacup Teddie",
            "Coco", "Bubba", "Milo", "Teddy", "Biscuit", "Frodo", "Gizmo", "Fluffy", "Mochi", "Waffles",
            "Bamboo", "Chomp", "Sprout", "Rolo", "Munchkin", "Pudding", "Pipsqueak", "Fuzzball", "Nibbles",
            "Pickles", "Popcorn", "Ziggy", "Sparky", "Scooter", "Whiskers", "Snickers", "Wiggles",
            "Bubbles", "Chubby", "Choco", "Snickerdoodle", "Cupcake", "Tootsie", "Doodle", "Muffin",
            "Peanut", "Buttons", "Truffles", "Brownie", "Gingersnap", "Poppy", "Puff", "Smores",
            "Marshmallow", "Cuddles", "Pumpkin", "Ruffles", "Tater", "Sprinkles", "Chewy", "Puffball",
            "Cupcake", "Fudge", "Chester", "Cosmo", "Clover", "Dobby", "Squeaky", "Nibbler", "Tater Tot",
            "Dumpling", "Wombat", "BoBo", "Churro", "Scooby", "Pudding", "Ducky", "Peaches", "Rascal",
            "Smidge", "Bean", "Scruffy", "Gus", "Rugrat", "Hobbit", "Beary_McBearface", "Paddington",
            "Bearlock Holmes", "Bearon von Growl", "Bearcules", "Winnie the Boo", "Grizzly Adams",
            "Bear Grylls", "Bearfoot", "Bearth Vader", "Bearin' Square", "Paw Bear",
            "Bearzooka", "Bear Hugz", "Bearister", "Gummy Bearson", "Bearalicious",
            "Robin Hoodbear", "Bearthoven", "Sir Growls-a-Lot", "Bearington",
            "Honeybear Hound", "Bearminator", "Bear Necessities", "Grizz Lee",
            "Polar Oppawsite", "Growlbert Einstein", "Bearoness", "Bearrific",
            "Bearcat", "Bearly Legal", "Unbearlievable", "Teddy Ruxbin", "Bear Hugger",
            "Bearoness von Snuggles", "Bearbie Doll", "Clawdia Pawlsen", "Grizzelda",
            "Fuzz Lightyear", "Pawdrey Hepbear", "Furrari", "Bearbados Slim", "Bearlin",
            "Furrnando", "Growlberto", "Bearloaf", "Bearianna Grande", "Bearon the Red",
            "Clawrence of Arabia", "Paddingpaw", "Pawtrick Swayze", "Bearami Brown",
            "Grizzabella", "Bearlentine", "Bearthday Boy", "Paw McCartney", "Clawdette",
            "Bearon Brando", "Beartholomew", "Bear Hugington", "Fluff Daddy", "Chewbearca",
            "Growldemort", "Bearicane", "Bearlosaurus Rex", "Bear-lenium Falcon", "Bearborator"
        };

        // Check existing bucket names first
        for (int i = 0; i < 10; i++) {
            std::string bucket_name = fmt::format("bear_name_{}", i);
            std::string ret_name = owner->GetBucket(bucket_name);
            if (!ret_name.empty()) {
                if (std::find(existing_pet_names.begin(), existing_pet_names.end(), ret_name) == existing_pet_names.end()) {
                    return ret_name;
                }
            }
        }

        // Generate a random bear name that is not already used
        int max = bearNames.size() - 1;
        std::string selected_name;
        for (int i = 0; i < bearNames.size(); ++i) {
            std::string random_name = bearNames[zone->random.Roll0(max)];
            if (std::find(existing_pet_names.begin(), existing_pet_names.end(), random_name) == existing_pet_names.end()) {
                selected_name = random_name;
                break;
            }
        }

        if (selected_name.empty()) {
            return "Unnamed Bear"; // Fallback if all names are taken
        }

        // Set the selected name into the first available bucket
        for (int i = 0; i < 10; i++) {
            std::string bucket_name = fmt::format("bear_name_{}", i);
            std::string ret_name = owner->GetBucket(bucket_name);
            if (ret_name.empty()) {  // Found the first unused bucket slot
                owner->SetBucket(bucket_name, selected_name); // Assuming SetBucket is the correct method to use
                break;
            }
        }

        return selected_name;
    };

	auto getRandomSkeletonName = [&]() -> std::string {
        std::vector<std::string> prefixes = {"Mor", "Skel", "Grim", "Varn", "Mar", "Karn", "Zor", "Gor", "Thal", "Tor", "Nar", "Thrax"};
        std::vector<std::string> middles = {"ak", "or", "th", "ar", "al", "ro", "im", "uth", "on", "an", "en", "ol", "amun"};
        std::vector<std::string> suffixes = {"rik", "thos", "nar", "grim", "thal", "ok", "ath", "ur", "mar", "oth", "ros", "ak", "dar"};

        // Check existing bucket names first
        for (int i = 1; i < 10; ++i) {
            std::string bucket_name = fmt::format("skeleton_name_{}", i);
            std::string ret_name = owner->GetBucket(bucket_name);
            if (!ret_name.empty()) {
                if (std::find(existing_pet_names.begin(), existing_pet_names.end(), ret_name) == existing_pet_names.end()) {
                    return ret_name;
                }
            }
        }

        // Generate a new skeleton name that is not already used
        std::string selected_name;
        do {
            std::string random_prefix = prefixes[zone->random.Roll0(prefixes.size() - 1)];
            std::string random_middle = middles[zone->random.Roll0(middles.size() - 1)];
            std::string random_suffix = suffixes[zone->random.Roll0(suffixes.size() - 1)];
            selected_name = random_prefix + random_middle + random_suffix;
            selected_name[0] = toupper(selected_name[0]);  // Capitalize the first letter
        } while (std::find(existing_pet_names.begin(), existing_pet_names.end(), selected_name) != existing_pet_names.end());

        // Set the selected name into the first available bucket
        for (int i = 1; i <= 100; ++i) {
            std::string bucket_name = fmt::format("skeleton_name_{}", i);
            if (owner->GetBucket(bucket_name).empty()) {  // Found the first unused bucket slot
                owner->SetBucket(bucket_name, selected_name);
                break;
            }
        }

        return selected_name;
    };

	auto getRandomWarderName = [&]() -> std::string {
        std::vector<std::string> prefixes = {
            "Gnar", "Krag", "Bru", "Vor", "Thok", "Dra", "Gar", "Zhar", "Kro", "Skaar", "Fang", "Ruk", "Grim",
            "Tharn", "Bar", "Krull", "Vorn", "Drak", "Krog", "Mar", "Groth", "Skorn", "Grak", "Harg",
            "Ruk", "Narz", "Vul", "Krath", "Rorg", "Tark", "Bruk", "Grimz", "Thrak", "Brak", "Mor", "Drak", "Kill",
            "Gnash", "Vrak", "Zur", "Grorn", "Koth", "Vorash", "Thrash", "Zorag", "Gruk", "Rak", "Vorn", "Goth"
        };
        std::vector<std::string> suffixes = {
            "fang", "claw", "tusk", "bite", "maw", "roar", "rend", "gore", "gnash", "bark", "slash",
            "snap", "rip", "tear", "thorn", "howl", "grunt", "wing", "quill", "tail", "fur",
            "king", "snout", "scale", "jaw", "hide", "horn", "talon",
            "hoof", "paw", "mane", "purr", "hiss", "sting", "snarl", "growl", "screech",
            "coil", "lunge", "scowl", "chomp", "gnarl", "gash", "whip", "bristle", "creep",
            "slink", "scratch", "gnaw", "rake", "squeal", "hiss", "snort",
            "rasp", "tread", "bound", "lunge", "lash", "slither", "thrash",
            "peck", "snip", "snatch",
            "bite", "shred", "gouge", "flinch", "grunt", "pierce",
            "clamp", "grind", "rake", "carve", "shred", "crunch",
            "batter", "crush", "mash", "snub", "dozer"
        };

        // Check existing bucket names first
        for (int i = 1; i <= 10; ++i) {
            std::string bucket_name = fmt::format("warder_name_{}", i);
            std::string ret_name = owner->GetBucket(bucket_name);
            if (!ret_name.empty()) {
                if (std::find(existing_pet_names.begin(), existing_pet_names.end(), ret_name) == existing_pet_names.end()) {
                    return ret_name;
                }
            }
        }

        // Generate a new warder name that is not already used
        std::string selected_name;
        do {
            std::string random_prefix = prefixes[zone->random.Roll0(prefixes.size() - 1)];
            std::string random_suffix = suffixes[zone->random.Roll0(suffixes.size() - 1)];
            selected_name = random_prefix + random_suffix;
            selected_name[0] = toupper(selected_name[0]);  // Capitalize the first letter
        } while (std::find(existing_pet_names.begin(), existing_pet_names.end(), selected_name) != existing_pet_names.end());

        // Set the selected name into the first available bucket
        for (int i = 1; i <= 10; ++i) {
            std::string bucket_name = fmt::format("warder_name_{}", i);
            if (owner->GetBucket(bucket_name).empty()) {  // Found the first unused bucket slot
                owner->SetBucket(bucket_name, selected_name);
                break;
            }
        }

        return selected_name;
    };

    auto getRandomSpiritName = [&]() -> std::string {
        std::vector<std::string> prefixes = {
            "Ancient", "Wise", "Shadow", "Mystic", "Spirit", "Ghost", "Phantom", "Ancestral", "Elder", "Sacred",
            "Thunder", "Moon", "Frost", "Blood", "Night", "Storm", "Silent", "Echo", "Fire", "Earth", "Sky",
            "Star", "Dark", "Silver", "Grim", "Fierce", "Wild", "Whisper", "Winter", "Steel", "Iron",
            "Noble", "Proud", "Fierce", "Glimmer", "Ember", "Savage", "Brave", "Noble", "Shimmer",
            "Golden", "Crimson", "Lone", "Eternal", "Wraith", "Stone"
        };

        std::vector<std::string> suffixes = {
            "Wolf", "Lupus", "Fang", "Howl", "Prowl", "Claw", "Eye", "Breath", "Maw", "Snarl", "Fur", "Tail", "Howler",
            "Bite", "Heart", "Shade", "Stalker", "Hunter", "Runner", "Roar", "Spirit", "Bane", "Fury",
            "Warden", "Shroud", "Shadow", "Ripper", "Guardian", "Strider", "Nightshade", "Sentry",
            "Whisper", "Fangblade", "Razorback", "Warg", "Sentinel", "Watcher", "Ghostwalker"
        };

        // Check existing bucket names first
        for (int i = 1; i <= 10; ++i) {
            std::string bucket_name = fmt::format("spirit_name_{}", i);
            std::string ret_name = owner->GetBucket(bucket_name);
            if (!ret_name.empty()) {
                if (std::find(existing_pet_names.begin(), existing_pet_names.end(), ret_name) == existing_pet_names.end()) {
                    return ret_name;
                }
            }
        }

        // Generate a new spirit name that is not already used
        std::string selected_name;
        do {
            std::string random_prefix = prefixes[zone->random.Roll0(prefixes.size() - 1)];
            std::string random_suffix = suffixes[zone->random.Roll0(suffixes.size() - 1)];
            selected_name = random_prefix + "_" + random_suffix;
        } while (std::find(existing_pet_names.begin(), existing_pet_names.end(), selected_name) != existing_pet_names.end());

        // Set the selected name into the first available bucket
        for (int i = 1; i <= 10; ++i) {
            std::string bucket_name = fmt::format("spirit_name_{}", i);
            if (owner->GetBucket(bucket_name).empty()) {  // Found the first unused bucket slot
                owner->SetBucket(bucket_name, selected_name);
                break;
            }
        }

        return selected_name;
    };

    auto handleElementalNaming = [&](const std::string& bucket_prefix) -> std::string {
        std::string pet_name;
        bool name_found = false;

        // Check existing bucket names first
        for (int i = 1; i <= 100; ++i) {
            std::string bucket_name = fmt::format("{}_name_{}", bucket_prefix, i);
            pet_name = owner->GetBucket(bucket_name);
            if (!pet_name.empty()) {
                if (std::find(existing_pet_names.begin(), existing_pet_names.end(), pet_name) == existing_pet_names.end()) {
                    return pet_name;
                }
            }
        }

        // If no existing name was found, use the elemental's default name
        if (!name_found) {
            pet_name = GetCleanName();  // Keep the elemental's default name unique
            for (int i = 1; i <= 100; ++i) {
                std::string bucket_name = fmt::format("{}_name_{}", bucket_prefix, i);
                if (owner->GetBucket(bucket_name).empty()) {  // Found the first unused bucket slot
                    owner->SetBucket(bucket_name, pet_name);
                    return pet_name;
                }
            }
        }
    };

    auto getRandomSpectreName = [&]() -> std::string {
        std::vector<std::string> prefixes = {"Shad", "Vel", "Mor", "Xyl", "Eld", "Zar", "Thar", "Lur", "Vor", "Dra", "Thrax", "Amun", "Grim"};
        std::vector<std::string> middles = {"rax", "drim", "vath", "ris", "ros", "vok", "nis", "rok", "rath", "lor", "amun"};
        std::vector<std::string> suffixes = {"thar", "is", "al", "ar", "os", "eth", "or", "ith", "as", "ok", "dar", "ra"};

        // Check existing bucket names first
        for (int i = 1; i <= 100; ++i) {
            std::string bucket_name = fmt::format("spectre_name_{}", i);
            std::string ret_name = owner->GetBucket(bucket_name);
            if (!ret_name.empty()) {
                if (std::find(existing_pet_names.begin(), existing_pet_names.end(), ret_name) == existing_pet_names.end()) {
                    return ret_name;
                }
            }
        }

        // Generate a new spectre name that is not already used
        std::string selected_name;
        do {
            std::string random_prefix = prefixes[zone->random.Roll0(prefixes.size() - 1)];
            std::string random_middle = middles[zone->random.Roll0(middles.size() - 1)];
            std::string random_suffix = suffixes[zone->random.Roll0(suffixes.size() - 1)];
            selected_name = random_prefix + random_middle + random_suffix;
            selected_name[0] = toupper(selected_name[0]);  // Capitalize the first letter
        } while (std::find(existing_pet_names.begin(), existing_pet_names.end(), selected_name) != existing_pet_names.end());

        // Set the selected name into the first available bucket
        for (int i = 1; i <= 100; ++i) {
            std::string bucket_name = fmt::format("spectre_name_{}", i);
            if (owner->GetBucket(bucket_name).empty()) {  // Found the first unused bucket slot
                owner->SetBucket(bucket_name, selected_name);
                return selected_name;
            }
        }
    };

	switch (spell_id) {
		// Enchanter Pets
		case 285: case 681: case 295: case 682: case 683: case 684: case 685: case 686:
		case 687: case 688: case 689: case 670: case 1723: case 3034: case 5505: case 10586:
			tmp_lastname = fmt::format("{}'s Animation", owner->GetCleanName());
			tmp_name 	 = (static_name == nullptr) ? spells[spell_id].name : static_name;
			break;
		// Beastlord Pets
		case 2612: case 2633: case 2614: case 2616: case 2618: case 2621: case 2623:
		case 2626: case 2627: case 2631: case 3457: case 3461: case 5531: case 5538:
		case 10379:
			tmp_lastname 	= fmt::format("{}'s Warder", owner->GetCleanName());
			tmp_name 	 	= (static_name == nullptr) ? getRandomWarderName() : static_name;
			// Resizing
			switch(owner->GetRace()) {
				case HUMAN:
					size *= 1.00;
				};
			break;
		// Shaman Pets
		case 164: case 577: case 165: case 166: case 1574: case 3377: case 5389: case 9983:
			tmp_lastname 	= fmt::format("{}'s Spirit", owner->GetCleanName());
			tmp_name 		= (static_name == nullptr) ? getRandomSpiritName() : static_name;
			size			= size * 0.50;
			break;
		// Necromancer Skeletons
		case 338: case 491: case 351: case 362: case 492: case 440: case 493: case 441:
		case 494: case 442: case 495: case 443: case 1621: case 1622:
			tmp_lastname 	= fmt::format("{}'s Skeleton", owner->GetCleanName());
			tmp_name 		= (static_name == nullptr) ? getRandomSkeletonName() : static_name;
			break;
		// Necromancer Spectres
		case 1623: case 3304: case 3310: case 3314: case 5431: case 5438: case 10506: case 10561:
			tmp_lastname = fmt::format("{}'s Spectre", owner->GetCleanName());
			tmp_name 		= (static_name == nullptr) ? getRandomSpectreName() : static_name;
			break;
		// Magician Pets
		case 3317: case 317: case 400: case 404: case 396: case 499: case 572: case 576:
		case 623: case 627: case 631: case 635: case 1674: case 1678: case 10695:
			tmp_lastname = fmt::format("{}'s Air Elemental", owner->GetCleanName());
			tmp_name 		= (static_name == nullptr)? handleElementalNaming("air_elemental") : static_name;
			break;
		case 3320: case 315: case 398: case 402: case 336: case 497: case 570: case 574:
		case 621: case 625: case 629: case 633: case 1672: case 1676: case 5480: case 10708:
			tmp_lastname = fmt::format("{}'s Water Elemental", owner->GetCleanName());
			tmp_name 		= (static_name == nullptr) ? handleElementalNaming("water_elemental") : static_name;
			break;
		case 3322: case 316: case 399: case 403: case 395: case 498: case 571: case 575:
		case 622: case 626: case 630: case 634: case 1673: case 1677: case 5485: case 10719:
			tmp_lastname = fmt::format("{}'s Fire Elemental", owner->GetCleanName());
			tmp_name 		= (static_name == nullptr) ? handleElementalNaming("fire_elemental") : static_name;
			break;
		case 3324: case 58: case 397: case 401: case 335: case 496: case 569: case 573:
		case 620: case 624: case 628: case 632: case 1671: case 1675: case 5495: case 10753:
			tmp_lastname = fmt::format("{}'s Earth Elemental", owner->GetCleanName());
			tmp_name 		= (static_name == nullptr) ? handleElementalNaming("earth_elemental") : static_name;
			break;
		case 1936:
			tmp_lastname = fmt::format("{}'s Elemental Avatar", owner->GetCleanName());
			break;
		case 1400: case 1402: case 1404: case 4888: case 10769:
			tmp_lastname = fmt::format("{}'s Summoned Monster", owner->GetCleanName());
			break;
		// Druid
		case 1475:
			tmp_lastname 	= fmt::format("{}'s Tiny Bear", owner->GetCleanName());
			tmp_name 	 	= (static_name == nullptr) ? getRandomBearName() : static_name;
			size			= size * 0.50;
			break;
		// Cleric
		case 1721: case 5256: case 11750: case 11751: case 11752:
			tmp_lastname = fmt::format("{}'s Holy Hammer", owner->GetCleanName());
			tmp_name 		= (static_name == nullptr) ? fmt::format("{}`s Holy Hammer", owner->GetCleanName()) : static_name;
			break;
		// Wizard
		case 1722: case 5460: case 10840:
			tmp_lastname 	= fmt::format("{}'s Animated Sword", owner->GetCleanName());
			tmp_name 		= (static_name == nullptr) ? fmt::format("{}`s {}", owner->GetCleanName(), spells[spell_id].name) : static_name;
			break;
		// Clockworks
		case 16601: case 16995:
			tmp_lastname = fmt::format("{}'s Clockwork Servant", owner->GetCleanName());
			if (spell_id = 16995) {
				tmp_name = (static_name == nullptr) ? fmt::format("{}`s Resupply Agent", owner->GetCleanName()) : static_name;
			} else {
				tmp_name = (static_name == nullptr) ? fmt::format("{}`s Clockwork Banker", owner->GetCleanName()) : static_name;
			}
	}

	if (tmp_lastname.size() < sizeof(lastname)) {
		strn0cpy(lastname, tmp_lastname.c_str(), sizeof(lastname));
	}

	if (!tmp_name.empty() && tmp_name.size() < sizeof(name)) {
		strn0cpy(name, tmp_name.c_str(), sizeof(name));
	}

	// Replace spaces with underscores
	for (char* p = name; *p; ++p) {
		if (*p == ' ') {
			*p = '_';
		}
	}

	for (char* p = lastname; *p; ++p) {
		if (*p == '\'') {
			*p = '`';
		}
	}
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

		if (pet_npc->GetTarget()) {
			auto tar_temp = pet_npc->GetTarget();
			pet_npc->SetTarget(nullptr, true);
			pet_npc->SetTarget(tar_temp, true);
		}

		safe_delete(outapp);
		safe_delete(outapp2);
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

bool Mob::IsPetAllowed(uint16 incoming_spell) {
    ValidatePetList();
    std::map<uint8, uint16> class_spell_map; // Maps class_id to spell_id
    std::set<uint8> assigned_classes; // Track assigned classes
    uint32 player_classes_bitmask = GetClassesBits();
    int pet_count = petids.size();

    // Check if the total number of pets exceeds the allowed limit
    if (pet_count + 1 > RuleI(Custom, AbsolutePetLimit)) {
        Message(Chat::SpellFailure, "You may not control any additional pets.");
        return false;
    }

    std::vector<uint16> spell_list;
    spell_list.push_back(incoming_spell);

    for (auto pet : GetAllPets()) {
        uint16 origin_spell = 0;

        auto pet_buffs = pet->GetBuffs();
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

        if (origin_spell == incoming_spell) {
            Message(Chat::SpellFailure, "You may not have more than one pet of the same type.");
            return false; // Never allow identical pets.
        }

        spell_list.push_back(origin_spell);
    }

    // Lambda function to check if a spell is unusable by any class the player has
    auto is_unusable_by_player_classes = [&](uint16 spell_id) -> bool {
        for (int i = Class::Warrior; i <= Class::Berserker; i++) {
            if (player_classes_bitmask & GetPlayerClassBit(i)) {
                if (GetSpellLevel(spell_id, i) < UINT8_MAX) {
                    return false;
                }
            }
        }
        return true;
    };

    // Process spells iteratively
    while (!spell_list.empty()) {
        uint16 spell_id = spell_list.back();
        spell_list.pop_back();

        std::map<uint8, uint16> class_levels;
        for (int i = Class::Warrior; i <= Class::Berserker; i++) {
            if (player_classes_bitmask & GetPlayerClassBit(i)) {
                uint16 level = GetSpellLevel(spell_id, i);
                if (level < UINT8_MAX) {
                    class_levels[i] = level;
                }
            }
        }

        bool assigned = false;

        // Assign to the lowest level class available among the player's classes
        for (auto &pair : class_levels) {
            uint8 class_id = pair.first;
            uint16 spell_level = pair.second;

            if (!assigned_classes.count(class_id)) {
                if (class_spell_map.count(class_id)) {
                    if (spell_level < GetSpellLevel(class_spell_map[class_id], class_id)) {
                        spell_list.push_back(class_spell_map[class_id]);
                        class_spell_map[class_id] = spell_id;
                        assigned_classes.insert(class_id);
                        assigned = true;
                        break;
                    }
                } else {
                    class_spell_map[class_id] = spell_id;
                    assigned_classes.insert(class_id);
                    assigned = true;
                    break;
                }
            }
        }

        // If the spell couldn't be assigned, check if it's unusable by any of the player's classes
        if (!assigned) {
            if (is_unusable_by_player_classes(spell_id)) {
                continue; // Allow it to pass if it's unusable by any of the player's classes
            } else {
				Message(Chat::SpellFailure, "You may not have more than one pet from the same class.");
                return false;
            }
        }
    }

    return true;
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
	ValidatePetList();
    // Iterate through the petids vector to find and remove the pet by its ID
    for (auto it = petids.begin(); it != petids.end(); ++it) {
        if (*it == pet_id) {
            // Retrieve the Mob associated with the pet ID
            auto pet = entity_list.GetMob(pet_id);
            if (pet) {
                pet->SetOwnerID(0);  // Detach the pet from its owner
            }
            petids.erase(it);        // Remove the pet ID from the vector
			ValidatePetList();
			if (IsClient()) {
				CastToClient()->DoPetBagResync();
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

    // Log the contents of the petids list before the final return
    LogDebug("Pet ID [{}] added to petids list for Mob [{}]. Current petids:", pet_id, GetCleanName());
    for (auto id : petids) {
        LogDebug("  - Pet ID: [{}]", id);
    }

	ConfigurePetWindow(focused_pet_id ? entity_list.GetNPCByID(focused_pet_id) : GetPet(0));

    return true;  // Return true to indicate the pet was successfully added
}

void Mob::ValidatePetList() {
    for (auto it = petids.begin(); it != petids.end(); ) {
		auto pet = entity_list.GetMob(*it);
        if (!pet || !pet->GetOwner()) {
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
