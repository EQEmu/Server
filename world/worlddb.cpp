/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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

#include "worlddb.h"
#include "../common/string_util.h"
#include "../common/eq_packet_structs.h"
#include "../common/inventory_profile.h"
#include "../common/rulesys.h"
#include <iostream>
#include <cstdlib>
#include <vector>
#include "sof_char_create_data.h"
#include "../common/repositories/criteria/content_filter_criteria.h"
#include "world_store.h"

WorldDatabase database;
WorldDatabase content_db;
extern std::vector<RaceClassAllocation> character_create_allocations;
extern std::vector<RaceClassCombos> character_create_race_class_combos;


/**
 * @param account_id
 * @param out_app
 * @param client_version_bit
 */
void WorldDatabase::GetCharSelectInfo(uint32 account_id, EQApplicationPacket **out_app, uint32 client_version_bit)
{
	EQ::versions::ClientVersion
		   client_version  = EQ::versions::ConvertClientVersionBitToClientVersion(client_version_bit);
	size_t character_limit = EQ::constants::StaticLookup(client_version)->CharacterCreationLimit;

	if (character_limit > EQ::constants::CHARACTER_CREATION_LIMIT) {
		character_limit = EQ::constants::CHARACTER_CREATION_LIMIT;
	}

	// Force Titanium clients to use '8'
	if (client_version == EQ::versions::ClientVersion::Titanium) {
		character_limit = 8;
	}

	std::string character_list_query = StringFormat(
		"SELECT                     "
		"`id`,                      "  // 0
		"name,                      "  // 1
		"gender,                    "  // 2
		"race,                      "  // 3
		"class,                     "  // 4
		"`level`,                   "  // 5
		"deity,                     "  // 6
		"last_login,                "  // 7
		"time_played,               "  // 8
		"hair_color,                "  // 9
		"beard_color,               "  // 10
		"eye_color_1,               "  // 11
		"eye_color_2,               "  // 12
		"hair_style,                "  // 13
		"beard,                     "  // 14
		"face,                      "  // 15
		"drakkin_heritage,          "  // 16
		"drakkin_tattoo,            "  // 17
		"drakkin_details,           "  // 18
		"zone_id		            "  // 19
		"FROM                       "
		"character_data             "
		"WHERE `account_id` = %i AND deleted_at IS NULL ORDER BY `name` LIMIT %u",
		account_id,
		character_limit
	);

	auto results = database.QueryDatabase(character_list_query);

	size_t character_count = results.RowCount();
	if (character_count == 0) {
		*out_app                   = new EQApplicationPacket(OP_SendCharInfo, sizeof(CharacterSelect_Struct));
		CharacterSelect_Struct *cs = (CharacterSelect_Struct *) (*out_app)->pBuffer;
		cs->CharCount  = 0;
		cs->TotalChars = character_limit;
		return;
	}

	size_t packet_size = sizeof(CharacterSelect_Struct) + (sizeof(CharacterSelectEntry_Struct) * character_count);
	*out_app = new EQApplicationPacket(OP_SendCharInfo, packet_size);

	unsigned char          *buff_ptr = (*out_app)->pBuffer;
	CharacterSelect_Struct *cs       = (CharacterSelect_Struct *) buff_ptr;

	cs->CharCount  = character_count;
	cs->TotalChars = character_limit;

	buff_ptr += sizeof(CharacterSelect_Struct);
	for (auto row = results.begin(); row != results.end(); ++row) {
		CharacterSelectEntry_Struct *p_character_select_entry_struct = (CharacterSelectEntry_Struct *) buff_ptr;
		PlayerProfile_Struct        player_profile_struct;
		EQ::InventoryProfile     inventory_profile;

		player_profile_struct.SetPlayerProfileVersion(EQ::versions::ConvertClientVersionToMobVersion(client_version));
		inventory_profile.SetInventoryVersion(client_version);
		inventory_profile.SetGMInventory(true); // charsel can not interact with items..but, no harm in setting to full expansion support

		uint32 character_id = (uint32) atoi(row[0]);
		uint8  has_home     = 0;
		uint8  has_bind     = 0;

		memset(&player_profile_struct, 0, sizeof(PlayerProfile_Struct));

		memset(p_character_select_entry_struct->Name, 0, sizeof(p_character_select_entry_struct->Name));
		strcpy(p_character_select_entry_struct->Name, row[1]);
		p_character_select_entry_struct->Class       = (uint8) atoi(row[4]);
		p_character_select_entry_struct->Race        = (uint32) atoi(row[3]);
		p_character_select_entry_struct->Level       = (uint8) atoi(row[5]);
		p_character_select_entry_struct->ShroudClass = p_character_select_entry_struct->Class;
		p_character_select_entry_struct->ShroudRace  = p_character_select_entry_struct->Race;
		p_character_select_entry_struct->Zone        = (uint16) atoi(row[19]);
		p_character_select_entry_struct->Instance    = 0;
		p_character_select_entry_struct->Gender      = (uint8) atoi(row[2]);
		p_character_select_entry_struct->Face        = (uint8) atoi(row[15]);

		for (uint32 material_slot = 0; material_slot < EQ::textures::materialCount; material_slot++) {
			p_character_select_entry_struct->Equip[material_slot].Material        = 0;
			p_character_select_entry_struct->Equip[material_slot].Unknown1        = 0;
			p_character_select_entry_struct->Equip[material_slot].EliteModel      = 0;
			p_character_select_entry_struct->Equip[material_slot].HerosForgeModel = 0;
			p_character_select_entry_struct->Equip[material_slot].Unknown2        = 0;
			p_character_select_entry_struct->Equip[material_slot].Color           = 0;
		}

		p_character_select_entry_struct->Unknown15       = 0xFF;
		p_character_select_entry_struct->Unknown19       = 0xFF;
		p_character_select_entry_struct->DrakkinTattoo   = (uint32) atoi(row[17]);
		p_character_select_entry_struct->DrakkinDetails  = (uint32) atoi(row[18]);
		p_character_select_entry_struct->Deity           = (uint32) atoi(row[6]);
		p_character_select_entry_struct->PrimaryIDFile   = 0;                            // Processed Below
		p_character_select_entry_struct->SecondaryIDFile = 0;                        // Processed Below
		p_character_select_entry_struct->HairColor       = (uint8) atoi(row[9]);
		p_character_select_entry_struct->BeardColor      = (uint8) atoi(row[10]);
		p_character_select_entry_struct->EyeColor1       = (uint8) atoi(row[11]);
		p_character_select_entry_struct->EyeColor2       = (uint8) atoi(row[12]);
		p_character_select_entry_struct->HairStyle       = (uint8) atoi(row[13]);
		p_character_select_entry_struct->Beard           = (uint8) atoi(row[14]);
		p_character_select_entry_struct->GoHome          = 0;                                // Processed Below
		p_character_select_entry_struct->Tutorial        = 0;                                // Processed Below
		p_character_select_entry_struct->DrakkinHeritage = (uint32) atoi(row[16]);
		p_character_select_entry_struct->Unknown1        = 0;
		p_character_select_entry_struct->Enabled         = 1;
		p_character_select_entry_struct->LastLogin       = (uint32) atoi(row[7]);            // RoF2 value: 1212696584
		p_character_select_entry_struct->Unknown2        = 0;

		if (RuleB(World, EnableReturnHomeButton)) {
			int now = time(nullptr);
			if ((now - atoi(row[7])) >= RuleI(World, MinOfflineTimeToReturnHome)) {
				p_character_select_entry_struct->GoHome = 1;
			}
		}

		if (RuleB(World, EnableTutorialButton) && (p_character_select_entry_struct->Level <= RuleI(World, MaxLevelForTutorial))) {
			p_character_select_entry_struct->Tutorial = 1;
		}

		/**
		 * Bind
		 */
		character_list_query = StringFormat(
			"SELECT `zone_id`, `instance_id`, `x`, `y`, `z`, `heading`, `slot` FROM `character_bind`  WHERE `id` = %i LIMIT 5",
			character_id
		);
		auto      results_bind = database.QueryDatabase(character_list_query);
		auto      bind_count   = results_bind.RowCount();
		for (auto row_b        = results_bind.begin(); row_b != results_bind.end(); ++row_b) {
			if (row_b[6] && atoi(row_b[6]) == 4) {
				has_home = 1;
				// If our bind count is less than 5, we need to actually make use of this data so lets parse it
				if (bind_count < 5) {
					player_profile_struct.binds[4].zoneId      = atoi(row_b[0]);
					player_profile_struct.binds[4].instance_id = atoi(row_b[1]);
					player_profile_struct.binds[4].x           = atof(row_b[2]);
					player_profile_struct.binds[4].y           = atof(row_b[3]);
					player_profile_struct.binds[4].z           = atof(row_b[4]);
					player_profile_struct.binds[4].heading     = atof(row_b[5]);
				}
			}
			if (row_b[6] && atoi(row_b[6]) == 0) { has_bind = 1; }
		}

		if (has_home == 0 || has_bind == 0) {
			character_list_query = StringFormat(
				"SELECT `zone_id`, `bind_id`, `x`, `y`, `z` FROM `start_zones` WHERE `player_class` = %i AND `player_deity` = %i AND `player_race` = %i %s",
				p_character_select_entry_struct->Class,
				p_character_select_entry_struct->Deity,
				p_character_select_entry_struct->Race,
				ContentFilterCriteria::apply().c_str()
			);
			auto      results_bind = content_db.QueryDatabase(character_list_query);
			for (auto row_d        = results_bind.begin(); row_d != results_bind.end(); ++row_d) {
				/* If a bind_id is specified, make them start there */
				if (atoi(row_d[1]) != 0) {
					player_profile_struct.binds[4].zoneId = (uint32) atoi(row_d[1]);
					content_db.GetSafePoints(
						ZoneName(player_profile_struct.binds[4].zoneId),
						0,
						&player_profile_struct.binds[4].x,
						&player_profile_struct.binds[4].y,
						&player_profile_struct.binds[4].z
					);
				}
					/* Otherwise, use the zone and coordinates given */
				else {
					player_profile_struct.binds[4].zoneId = (uint32) atoi(row_d[0]);
					float x = atof(row_d[2]);
					float y = atof(row_d[3]);
					float z = atof(row_d[4]);
					if (x == 0 && y == 0 && z == 0) {
						content_db.GetSafePoints(
							ZoneName(player_profile_struct.binds[4].zoneId),
							0,
							&x,
							&y,
							&z
						);
					}
					player_profile_struct.binds[4].x = x;
					player_profile_struct.binds[4].y = y;
					player_profile_struct.binds[4].z = z;
				}
			}
			player_profile_struct.binds[0] = player_profile_struct.binds[4];
			/* If no home bind set, set it */
			if (has_home == 0) {
				std::string query        = StringFormat(
					"REPLACE INTO `character_bind` (id, zone_id, instance_id, x, y, z, heading, slot)"
					" VALUES (%u, %u, %u, %f, %f, %f, %f, %i)",
					character_id,
					player_profile_struct.binds[4].zoneId,
					0,
					player_profile_struct.binds[4].x,
					player_profile_struct.binds[4].y,
					player_profile_struct.binds[4].z,
					player_profile_struct.binds[4].heading,
					4
				);
				auto        results_bset = QueryDatabase(query);
			}
			/* If no regular bind set, set it */
			if (has_bind == 0) {
				std::string query        = StringFormat(
					"REPLACE INTO `character_bind` (id, zone_id, instance_id, x, y, z, heading, slot)"
					" VALUES (%u, %u, %u, %f, %f, %f, %f, %i)",
					character_id,
					player_profile_struct.binds[0].zoneId,
					0,
					player_profile_struct.binds[0].x,
					player_profile_struct.binds[0].y,
					player_profile_struct.binds[0].z,
					player_profile_struct.binds[0].heading,
					0
				);
				auto        results_bset = QueryDatabase(query);
			}
		}
		/* If our bind count is less than 5, then we have null data that needs to be filled in. */
		if (bind_count < 5) {
			// we know that home and main bind must be valid here, so we don't check those
			// we also use home to fill in the null data like live does.
			for (int i = 1; i < 4; i++) {
				if (player_profile_struct.binds[i].zoneId != 0) { // we assume 0 is the only invalid one ...
					continue;
				}

				std::string query        = StringFormat(
					"REPLACE INTO `character_bind` (id, zone_id, instance_id, x, y, z, heading, slot)"
					" VALUES (%u, %u, %u, %f, %f, %f, %f, %i)",
					character_id,
					player_profile_struct.binds[4].zoneId,
					0,
					player_profile_struct.binds[4].x,
					player_profile_struct.binds[4].y,
					player_profile_struct.binds[4].z,
					player_profile_struct.binds[4].heading,
					i
				);
				auto        results_bset = QueryDatabase(query);
			}
		}

		character_list_query = StringFormat(
			"SELECT slot, red, green, blue, use_tint, color FROM `character_material` WHERE `id` = %u",
			character_id
		);
		auto      results_b = database.QueryDatabase(character_list_query);
		uint8     slot      = 0;
		for (auto row_b     = results_b.begin(); row_b != results_b.end(); ++row_b) {
			slot = atoi(row_b[0]);
			player_profile_struct.item_tint.Slot[slot].Red     = atoi(row_b[1]);
			player_profile_struct.item_tint.Slot[slot].Green   = atoi(row_b[2]);
			player_profile_struct.item_tint.Slot[slot].Blue    = atoi(row_b[3]);
			player_profile_struct.item_tint.Slot[slot].UseTint = atoi(row_b[4]);
		}

		if (GetCharSelInventory(account_id, p_character_select_entry_struct->Name, &inventory_profile)) {
			const EQ::ItemData     *item          = nullptr;
			const EQ::ItemInstance *inst          = nullptr;
			int16 inventory_slot = 0;

			for (uint32 matslot = EQ::textures::textureBegin; matslot < EQ::textures::materialCount; matslot++) {
				inventory_slot = EQ::InventoryProfile::CalcSlotFromMaterial(matslot);
				if (inventory_slot == INVALID_INDEX) { continue; }
				inst = inventory_profile.GetItem(inventory_slot);
				if (inst == nullptr) {
					continue;
				}
				item = inst->GetItem();
				if (item == nullptr) {
					continue;
				}

				if (matslot > 6) {
					uint32 item_id_file = 0;
					// Weapon Models
					if (inst->GetOrnamentationIDFile() != 0) {
						item_id_file = inst->GetOrnamentationIDFile();
						p_character_select_entry_struct->Equip[matslot].Material = item_id_file;
					}
					else {
						if (strlen(item->IDFile) > 2) {
							item_id_file = atoi(&item->IDFile[2]);
							p_character_select_entry_struct->Equip[matslot].Material = item_id_file;
						}
					}
					if (matslot == EQ::textures::weaponPrimary) {
						p_character_select_entry_struct->PrimaryIDFile = item_id_file;
					}
					else {
						p_character_select_entry_struct->SecondaryIDFile = item_id_file;
					}
				}
				else {
					uint32 color = 0;
					if (player_profile_struct.item_tint.Slot[matslot].UseTint) {
						color = player_profile_struct.item_tint.Slot[matslot].Color;
					}
					else {
						color = inst->GetColor();
					}

					// Armor Materials/Models
					p_character_select_entry_struct->Equip[matslot].Material        = item->Material;
					p_character_select_entry_struct->Equip[matslot].EliteModel      = item->EliteMaterial;
					p_character_select_entry_struct->Equip[matslot].HerosForgeModel = inst->GetOrnamentHeroModel(matslot);
					p_character_select_entry_struct->Equip[matslot].Color           = color;
				}
			}
		}
		else {
			printf("Error loading inventory for %s\n", p_character_select_entry_struct->Name);
		}

		buff_ptr += sizeof(CharacterSelectEntry_Struct);
	}
}

int WorldDatabase::MoveCharacterToBind(int CharID, uint8 bindnum)
{
	/*  if an invalid bind point is specified, use the primary bind */
	if (bindnum > 4)
	{
		bindnum = 0;
	}

	std::string query = StringFormat("SELECT zone_id, instance_id, x, y, z FROM character_bind WHERE id = %u AND slot = %u LIMIT 1", CharID, bindnum);
	auto results = database.QueryDatabase(query);
	if(!results.Success() || results.RowCount() == 0) {
		return 0;
	}

	int zone_id, instance_id;
	double x, y, z, heading;
	for (auto row = results.begin(); row != results.end(); ++row) {
		zone_id = atoi(row[0]);
		instance_id = atoi(row[1]);
		x = atof(row[2]);
		y = atof(row[3]);
		z = atof(row[4]);
		heading = atof(row[5]);
	}

	query = StringFormat("UPDATE character_data SET zone_id = '%d', zone_instance = '%d', x = '%f', y = '%f', z = '%f', heading = '%f' WHERE id = %u",
						 zone_id, instance_id, x, y, z, heading, CharID);

	results = database.QueryDatabase(query);
	if(!results.Success()) {
		return 0;
	}

	return zone_id;
}

bool WorldDatabase::GetStartZone(
	PlayerProfile_Struct *p_player_profile_struct,
	CharCreate_Struct *p_char_create_struct,
	bool is_titanium
)
{
	// SoF doesn't send the player_choice field in character creation, it now sends the real zoneID instead.
	//
	// For SoF, search for an entry in start_zones with a matching zone_id, class, race and deity.
	//
	// For now, if no row matching row is found, send them to Crescent Reach, as that is probably the most likely
	// reason for no match being found.
	//
	if (!p_player_profile_struct || !p_char_create_struct) {
		return false;
	}

	p_player_profile_struct->x                    = 0;
	p_player_profile_struct->y                    = 0;
	p_player_profile_struct->z                    = 0;
	p_player_profile_struct->heading              = 0;
	p_player_profile_struct->zone_id              = 0;
	p_player_profile_struct->binds[0].x           = 0;
	p_player_profile_struct->binds[0].y           = 0;
	p_player_profile_struct->binds[0].z           = 0;
	p_player_profile_struct->binds[0].zoneId      = 0;
	p_player_profile_struct->binds[0].instance_id = 0;

	// see if we have an entry for start_zone. We can support both titanium & SOF+ by having two entries per class/race/deity combo with different zone_ids
	std::string query;

	if (is_titanium) {
		// Titanium sends player choice (starting city) instead of a zone id
		query = StringFormat(
			"SELECT x, y, z, heading, start_zone, bind_id, bind_x, bind_y, bind_z FROM start_zones WHERE player_choice = %i "
			"AND player_class = %i AND player_deity = %i AND player_race = %i %s",
			p_char_create_struct->start_zone,
			p_char_create_struct->class_,
			p_char_create_struct->deity,
			p_char_create_struct->race,
			ContentFilterCriteria::apply().c_str()
		);
		LogInfo("Titanium Start zone query: [{}]\n", query.c_str());
	}
	else {
		query = StringFormat(
			"SELECT x, y, z, heading, start_zone, bind_id, bind_x, bind_y, bind_z FROM start_zones WHERE zone_id = %i "
			"AND player_class = %i AND player_deity = %i AND player_race = %i %s",
			p_char_create_struct->start_zone,
			p_char_create_struct->class_,
			p_char_create_struct->deity,
			p_char_create_struct->race,
			ContentFilterCriteria::apply().c_str()
		);
		LogInfo("SoF Start zone query: [{}]\n", query.c_str());
	}

    auto results = QueryDatabase(query);
	if(!results.Success()) {
		return false;
	}

	LogInfo("SoF Start zone query: [{}]\n", query.c_str());

	if (results.RowCount() == 0) {
		printf("No start_zones entry in database, using defaults\n");
		is_titanium ? SetTitaniumDefaultStartZone(p_player_profile_struct, p_char_create_struct) : SetSoFDefaultStartZone(p_player_profile_struct, p_char_create_struct);
    }
    else {
		LogInfo("Found starting location in start_zones");
		auto row = results.begin();
		p_player_profile_struct->x               = atof(row[0]);
		p_player_profile_struct->y               = atof(row[1]);
		p_player_profile_struct->z               = atof(row[2]);
		p_player_profile_struct->heading         = atof(row[3]);
		p_player_profile_struct->zone_id         = atoi(row[4]);
		p_player_profile_struct->binds[0].zoneId = atoi(row[5]);
		p_player_profile_struct->binds[0].x      = atof(row[6]);
		p_player_profile_struct->binds[0].y      = atof(row[7]);
		p_player_profile_struct->binds[0].z      = atof(row[8]);
	}

	if (p_player_profile_struct->x == 0 && p_player_profile_struct->y == 0 && p_player_profile_struct->z == 0) {
		content_db.GetSafePoints(
			ZoneName(p_player_profile_struct->zone_id),
			0,
			&p_player_profile_struct->x,
			&p_player_profile_struct->y,
			&p_player_profile_struct->z
		);
	}

	if (p_player_profile_struct->binds[0].x == 0 && p_player_profile_struct->binds[0].y == 0 &&
		p_player_profile_struct->binds[0].z == 0) {
		content_db.GetSafePoints(
			ZoneName(p_player_profile_struct->binds[0].zoneId),
			0,
			&p_player_profile_struct->binds[0].x,
			&p_player_profile_struct->binds[0].y,
			&p_player_profile_struct->binds[0].z
		);
	}

	return true;
}

void WorldDatabase::SetSoFDefaultStartZone(PlayerProfile_Struct* in_pp, CharCreate_Struct* in_cc){
	if (in_cc->start_zone == RuleI(World, TutorialZoneID)) {
		in_pp->zone_id = in_cc->start_zone;
	}
	else {
		in_pp->x = in_pp->binds[0].x = -51;
		in_pp->y = in_pp->binds[0].y = -20;
		in_pp->z = in_pp->binds[0].z = 0.79;
		in_pp->zone_id = in_pp->binds[0].zoneId = 394; // Crescent Reach.
	}
}

void WorldDatabase::SetTitaniumDefaultStartZone(PlayerProfile_Struct* in_pp, CharCreate_Struct* in_cc)
{
	switch (in_cc->start_zone)
	{
		case 0:
		{
			if (in_cc->deity == 203) // Cazic-Thule Erudites go to Paineel
			{
				in_pp->zone_id = 75; // paineel
				in_pp->binds[0].zoneId = 75;
			}
			else
			{
				in_pp->zone_id = 24;	// erudnext
				in_pp->binds[0].zoneId = 38;	// tox
			}
			break;
		}
		case 1:
		{
			in_pp->zone_id = 2;	// qeynos2
			in_pp->binds[0].zoneId = 2;	// qeynos2
			break;
		}
		case 2:
		{
			in_pp->zone_id = 29;	// halas
			in_pp->binds[0].zoneId = 30;	// everfrost
			break;
		}
		case 3:
		{
			in_pp->zone_id = 19;	// rivervale
			in_pp->binds[0].zoneId = 20;	// kithicor
			break;
		}
		case 4:
		{
			in_pp->zone_id = 9;	// freportw
			in_pp->binds[0].zoneId = 9;	// freportw
			break;
		}
		case 5:
		{
			in_pp->zone_id = 40;	// neriaka
			in_pp->binds[0].zoneId = 25;	// nektulos
			break;
		}
		case 6:
		{
			in_pp->zone_id = 52;	// gukta
			in_pp->binds[0].zoneId = 46;	// innothule
			break;
		}
		case 7:
		{
			in_pp->zone_id = 49;	// oggok
			in_pp->binds[0].zoneId = 47;	// feerrott
			break;
		}
		case 8:
		{
			in_pp->zone_id = 60;	// kaladima
			in_pp->binds[0].zoneId = 68;	// butcher
			break;
		}
		case 9:
		{
			in_pp->zone_id = 54;	// gfaydark
			in_pp->binds[0].zoneId = 54;	// gfaydark
			break;
		}
		case 10:
		{
			in_pp->zone_id = 61;	// felwithea
			in_pp->binds[0].zoneId = 54;	// gfaydark
			break;
		}
		case 11:
		{
			in_pp->zone_id = 55;	// akanon
			in_pp->binds[0].zoneId = 56;	// steamfont
			break;
		}
		case 12:
		{
			in_pp->zone_id = 82;	// cabwest
			in_pp->binds[0].zoneId = 78;	// fieldofbone
			break;
		}
		case 13:
		{
			in_pp->zone_id = 155;	// sharvahl
			in_pp->binds[0].zoneId = 155;	// sharvahl
			break;
		}
	}
}

void WorldDatabase::GetLauncherList(std::vector<std::string> &rl) {
	rl.clear();

    const std::string query = "SELECT name FROM launcher";
    auto results = QueryDatabase(query);
    if (!results.Success()) {
        LogError("WorldDatabase::GetLauncherList: {}", results.ErrorMessage().c_str());
        return;
    }

    for (auto row = results.begin(); row != results.end(); ++row)
        rl.push_back(row[0]);

}

bool WorldDatabase::GetCharacterLevel(const char *name, int &level)
{
	std::string query = StringFormat("SELECT level FROM character_data WHERE name = '%s'", name);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
        LogError("WorldDatabase::GetCharacterLevel: {}", results.ErrorMessage().c_str());
        return false;
	}

	if (results.RowCount() == 0)
        return false;

    auto row = results.begin();
    level = atoi(row[0]);

    return true;
}

bool WorldDatabase::LoadCharacterCreateAllocations()
{
	character_create_allocations.clear();

	std::string query = "SELECT * FROM char_create_point_allocations ORDER BY id";
	auto results = QueryDatabase(query);
	if (!results.Success())
        return false;

    for (auto row = results.begin(); row != results.end(); ++row) {
        RaceClassAllocation allocate;
		allocate.Index = atoi(row[0]);
		allocate.BaseStats[0] = atoi(row[1]);
		allocate.BaseStats[3] = atoi(row[2]);
		allocate.BaseStats[1] = atoi(row[3]);
		allocate.BaseStats[2] = atoi(row[4]);
		allocate.BaseStats[4] = atoi(row[5]);
		allocate.BaseStats[5] = atoi(row[6]);
		allocate.BaseStats[6] = atoi(row[7]);
		allocate.DefaultPointAllocation[0] = atoi(row[8]);
		allocate.DefaultPointAllocation[3] = atoi(row[9]);
		allocate.DefaultPointAllocation[1] = atoi(row[10]);
		allocate.DefaultPointAllocation[2] = atoi(row[11]);
		allocate.DefaultPointAllocation[4] = atoi(row[12]);
		allocate.DefaultPointAllocation[5] = atoi(row[13]);
		allocate.DefaultPointAllocation[6] = atoi(row[14]);

		character_create_allocations.push_back(allocate);
    }

	return true;
}

bool WorldDatabase::LoadCharacterCreateCombos()
{
	character_create_race_class_combos.clear();

	std::string query = "SELECT * FROM char_create_combinations ORDER BY race, class, deity, start_zone";
	auto results = QueryDatabase(query);
	if (!results.Success())
        return false;

	for (auto row = results.begin(); row != results.end(); ++row) {
		RaceClassCombos combo;
		combo.AllocationIndex = atoi(row[0]);
		combo.Race = atoi(row[1]);
		combo.Class = atoi(row[2]);
		combo.Deity = atoi(row[3]);
		combo.Zone = atoi(row[4]);
		combo.ExpansionRequired = atoi(row[5]);

		character_create_race_class_combos.push_back(combo);
	}

	return true;
}

// this is a slightly modified version of SharedDatabase::GetInventory(...) for character select use-only
bool WorldDatabase::GetCharSelInventory(uint32 account_id, char *name, EQ::InventoryProfile *inv)
{
	if (!account_id || !name || !inv)
		return false;

	std::string query = StringFormat(
		"SELECT"
		" slotid,"
		" itemid,"
		" charges,"
		" color,"
		" augslot1,"
		" augslot2,"
		" augslot3,"
		" augslot4,"
		" augslot5,"
		" augslot6,"
		" instnodrop,"
		" custom_data,"
		" ornamenticon,"
		" ornamentidfile,"
		" ornament_hero_model "
		"FROM"
		" inventory "
		"INNER JOIN"
		" character_data ch "
		"ON"
		" ch.id = charid "
		"WHERE"
		" ch.name = '%s' "
		"AND"
		" ch.account_id = %i "
		"AND"
		" slotid >= %i "
		"AND"
		" slotid <= %i",
		name,
		account_id,
		EQ::invslot::slotHead,
		EQ::invslot::slotFeet
	);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	for (auto row = results.begin(); row != results.end(); ++row) {
		int16 slot_id = atoi(row[0]);

		switch (slot_id) {
		case EQ::invslot::slotFace:
		case EQ::invslot::slotEar2:
		case EQ::invslot::slotNeck:
		case EQ::invslot::slotShoulders:
		case EQ::invslot::slotBack:
		case EQ::invslot::slotFinger1:
		case EQ::invslot::slotFinger2:
			continue;
		default:
			break;
		}

		uint32 item_id = atoi(row[1]);
		int8 charges = atoi(row[2]);
		uint32 color = atoul(row[3]);

		uint32 aug[EQ::invaug::SOCKET_COUNT];
		aug[0] = (uint32)atoi(row[4]);
		aug[1] = (uint32)atoi(row[5]);
		aug[2] = (uint32)atoi(row[6]);
		aug[3] = (uint32)atoi(row[7]);
		aug[4] = (uint32)atoi(row[8]);
		aug[5] = (uint32)atoi(row[9]);

		bool instnodrop = ((row[10] && (uint16)atoi(row[10])) ? true : false);
		uint32 ornament_icon = (uint32)atoul(row[12]);
		uint32 ornament_idfile = (uint32)atoul(row[13]);
		uint32 ornament_hero_model = (uint32)atoul(row[14]);

		const EQ::ItemData *item = content_db.GetItem(item_id);
		if (!item)
			continue;

		EQ::ItemInstance *inst = content_db.CreateBaseItem(item, charges);

		if (inst == nullptr)
			continue;

		inst->SetAttuned(instnodrop);

		if (row[11]) {
			std::string data_str(row[11]);
			std::string idAsString;
			std::string value;
			bool use_id = true;

			for (int i = 0; i < data_str.length(); ++i) {
				if (data_str[i] == '^') {
					if (!use_id) {
						inst->SetCustomData(idAsString, value);
						idAsString.clear();
						value.clear();
					}

					use_id = !use_id;
					continue;
				}

				char v = data_str[i];
				if (use_id)
					idAsString.push_back(v);
				else
					value.push_back(v);
			}
		}

		inst->SetOrnamentIcon(ornament_icon);
		inst->SetOrnamentationIDFile(ornament_idfile);
		inst->SetOrnamentHeroModel(item->HerosForgeModel);

		if (color > 0)
			inst->SetColor(color);

		inst->SetCharges(charges);

		if (item->IsClassCommon()) {
			for (int i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
				if (aug[i])
					inst->PutAugment(this, i, aug[i]);
			}
		}

		inv->PutItem(slot_id, *inst);

		safe_delete(inst);
	}

	return true;
}
