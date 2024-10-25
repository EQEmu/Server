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
#include "../common/strings.h"
#include "../common/eq_packet_structs.h"
#include "../common/inventory_profile.h"
#include "../common/rulesys.h"
#include <iostream>
#include <cstdlib>
#include <vector>
#include "sof_char_create_data.h"
#include "../common/repositories/character_instance_safereturns_repository.h"
#include "../common/repositories/inventory_repository.h"
#include "../common/repositories/criteria/content_filter_criteria.h"
#include "../common/zone_store.h"

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

	std::string character_list_query = fmt::format(
		SQL(
			SELECT
			`id`,
			`name`,
			`gender`,
			`race`,
			`class`,
			`level`,
			`deity`,
			`last_login`,
			`time_played`,
			`hair_color`,
			`beard_color`,
			`eye_color_1`,
			`eye_color_2`,
			`hair_style`,
			`beard`,
			`face`,
			`drakkin_heritage`,
			`drakkin_tattoo`,
			`drakkin_details`,
			`zone_id`
			FROM
			`character_data`
			WHERE
			`account_id` = {}
			AND
			`deleted_at` IS NULL
			ORDER BY `name`
			LIMIT {}
		),
		account_id,
		character_limit
	);

	auto results = database.QueryDatabase(character_list_query);
	size_t character_count = results.RowCount();
	if (character_count == 0) {
		*out_app = new EQApplicationPacket(OP_SendCharInfo, sizeof(CharacterSelect_Struct));
		CharacterSelect_Struct *cs = (CharacterSelect_Struct *) (*out_app)->pBuffer;
		cs->CharCount = 0;
		cs->TotalChars = character_limit;
		return;
	}

	size_t packet_size = sizeof(CharacterSelect_Struct) + (sizeof(CharacterSelectEntry_Struct) * character_count);
	*out_app = new EQApplicationPacket(OP_SendCharInfo, packet_size);

	unsigned char *buff_ptr = (*out_app)->pBuffer;
	CharacterSelect_Struct *cs = (CharacterSelect_Struct *) buff_ptr;

	cs->CharCount = character_count;
	cs->TotalChars = character_limit;

	buff_ptr += sizeof(CharacterSelect_Struct);
	for (auto row = results.begin(); row != results.end(); ++row) {
		CharacterSelectEntry_Struct *p_character_select_entry_struct = (CharacterSelectEntry_Struct *) buff_ptr;
		PlayerProfile_Struct        pp;
		EQ::InventoryProfile        inventory_profile;

		pp.SetPlayerProfileVersion(EQ::versions::ConvertClientVersionToMobVersion(client_version));
		inventory_profile.SetInventoryVersion(client_version);
		inventory_profile.SetGMInventory(true); // charsel can not interact with items..but, no harm in setting to full expansion support

		uint32 character_id = Strings::ToUnsignedInt(row[0]);
		uint8 has_home = 0;
		uint8 has_bind = 0;

		memset(&pp, 0, sizeof(PlayerProfile_Struct));
		memset(p_character_select_entry_struct->Name, 0, sizeof(p_character_select_entry_struct->Name));
		strcpy(p_character_select_entry_struct->Name, row[1]);
		p_character_select_entry_struct->Class = (uint8) Strings::ToUnsignedInt(row[4]);
		p_character_select_entry_struct->Race = (uint32) Strings::ToUnsignedInt(row[3]);
		p_character_select_entry_struct->Level = (uint8) Strings::ToUnsignedInt(row[5]);
		p_character_select_entry_struct->ShroudClass = p_character_select_entry_struct->Class;
		p_character_select_entry_struct->ShroudRace = p_character_select_entry_struct->Race;
		p_character_select_entry_struct->Zone = (uint16) Strings::ToUnsignedInt(row[19]);
		p_character_select_entry_struct->Instance = 0;
		p_character_select_entry_struct->Gender = (uint8) Strings::ToUnsignedInt(row[2]);
		p_character_select_entry_struct->Face = (uint8) Strings::ToUnsignedInt(row[15]);

		for (uint32 material_slot = 0; material_slot < EQ::textures::materialCount; material_slot++) {
			p_character_select_entry_struct->Equip[material_slot].Material = 0;
			p_character_select_entry_struct->Equip[material_slot].Unknown1 = 0;
			p_character_select_entry_struct->Equip[material_slot].EliteModel = 0;
			p_character_select_entry_struct->Equip[material_slot].HerosForgeModel = 0;
			p_character_select_entry_struct->Equip[material_slot].Unknown2 = 0;
			p_character_select_entry_struct->Equip[material_slot].Color = 0;
		}

		p_character_select_entry_struct->Unknown15 = 0xFF;
		p_character_select_entry_struct->Unknown19 = 0xFF;
		p_character_select_entry_struct->DrakkinTattoo = (uint32) Strings::ToInt(row[17]);
		p_character_select_entry_struct->DrakkinDetails = (uint32) Strings::ToInt(row[18]);
		p_character_select_entry_struct->Deity = (uint32) Strings::ToInt(row[6]);
		p_character_select_entry_struct->PrimaryIDFile = 0;                            // Processed Below
		p_character_select_entry_struct->SecondaryIDFile = 0;                        // Processed Below
		p_character_select_entry_struct->HairColor = (uint8) Strings::ToInt(row[9]);
		p_character_select_entry_struct->BeardColor = (uint8) Strings::ToInt(row[10]);
		p_character_select_entry_struct->EyeColor1 = (uint8) Strings::ToInt(row[11]);
		p_character_select_entry_struct->EyeColor2 = (uint8) Strings::ToInt(row[12]);
		p_character_select_entry_struct->HairStyle = (uint8) Strings::ToInt(row[13]);
		p_character_select_entry_struct->Beard = (uint8) Strings::ToInt(row[14]);
		p_character_select_entry_struct->GoHome = 0;                                // Processed Below
		p_character_select_entry_struct->Tutorial = 0;                                // Processed Below
		p_character_select_entry_struct->DrakkinHeritage = (uint32) Strings::ToInt(row[16]);
		p_character_select_entry_struct->Unknown1 = 0;
		p_character_select_entry_struct->Enabled = 1;
		p_character_select_entry_struct->LastLogin = (uint32) Strings::ToInt(row[7]);            // RoF2 value: 1212696584
		p_character_select_entry_struct->Unknown2 = 0;

		if (RuleB(World, EnableReturnHomeButton)) {
			int now = time(nullptr);
			if ((now - Strings::ToInt(row[7])) >= RuleI(World, MinOfflineTimeToReturnHome))
				p_character_select_entry_struct->GoHome = 1;
		}

		if (RuleB(World, EnableTutorialButton) && (p_character_select_entry_struct->Level <= RuleI(World, MaxLevelForTutorial)))
			p_character_select_entry_struct->Tutorial = 1;

		/**
		 * Bind
		 */
		character_list_query = fmt::format(
			SQL(
				SELECT
				`zone_id`, `instance_id`, `x`, `y`, `z`, `heading`, `slot`
				FROM
				`character_bind`
				WHERE
				`id` = {}
				LIMIT 5
			),
			character_id
		);
		auto results_bind = database.QueryDatabase(character_list_query);
		auto bind_count = results_bind.RowCount();
		for (auto row_b = results_bind.begin(); row_b != results_bind.end(); ++row_b) {
			if (row_b[6] && Strings::ToInt(row_b[6]) == 4) {
				has_home = 1;
				// If our bind count is less than 5, we need to actually make use of this data so lets parse it
				if (bind_count < 5) {
					pp.binds[4].zone_id     = Strings::ToInt(row_b[0]);
					pp.binds[4].instance_id = Strings::ToInt(row_b[1]);
					pp.binds[4].x           = Strings::ToFloat(row_b[2]);
					pp.binds[4].y           = Strings::ToFloat(row_b[3]);
					pp.binds[4].z           = Strings::ToFloat(row_b[4]);
					pp.binds[4].heading     = Strings::ToFloat(row_b[5]);
				}
			}

			if (row_b[6] && Strings::ToInt(row_b[6]) == 0)
				has_bind = 1;
		}

		if (has_home == 0 || has_bind == 0) {
			std::string character_list_query = fmt::format(
				SQL(
					SELECT
					`zone_id`, `bind_id`, `x`, `y`, `z`, `heading`
					FROM
					`start_zones`
					WHERE
					`player_class` = {}
					AND
					`player_deity` = {}
					AND
					`player_race` = {} {}
				),
				p_character_select_entry_struct->Class,
				p_character_select_entry_struct->Deity,
				p_character_select_entry_struct->Race,
				ContentFilterCriteria::apply().c_str()
			);
			auto results_bind = content_db.QueryDatabase(character_list_query);
			for (auto row_d = results_bind.begin(); row_d != results_bind.end(); ++row_d) {
				/* If a bind_id is specified, make them start there */
				if (Strings::ToInt(row_d[1]) != 0) {
					pp.binds[4].zone_id = (uint32) Strings::ToInt(row_d[1]);

					auto z = GetZone(pp.binds[4].zone_id);
					if (z) {
						pp.binds[4].x       = z->safe_x;
						pp.binds[4].y       = z->safe_y;
						pp.binds[4].z       = z->safe_z;
						pp.binds[4].heading = z->safe_heading;
					}
				}
					/* Otherwise, use the zone and coordinates given */
				else {
					pp.binds[4].zone_id = (uint32) Strings::ToInt(row_d[0]);
					float x = Strings::ToFloat(row_d[2]);
					float y = Strings::ToFloat(row_d[3]);
					float z = Strings::ToFloat(row_d[4]);
					float heading = Strings::ToFloat(row_d[5]);
					if (x == 0 && y == 0 && z == 0 && heading == 0) {
						auto zone = GetZone(pp.binds[4].zone_id);
						if (zone) {
							x       = zone->safe_x;
							y       = zone->safe_y;
							z       = zone->safe_z;
							heading = zone->safe_heading;
						}
					}
					pp.binds[4].x       = x;
					pp.binds[4].y       = y;
					pp.binds[4].z       = z;
					pp.binds[4].heading = heading;
				}
			}
			pp.binds[0] = pp.binds[4];
			/* If no home bind set, set it */
			if (has_home == 0) {
				std::string query = fmt::format(
					SQL(
						REPLACE INTO
						`character_bind`
						(`id`, `zone_id`, `instance_id`, `x`, `y`, `z`, `heading`, `slot`)
						VALUES ({}, {}, {}, {}, {}, {}, {}, {})
					),
					character_id,
					pp.binds[4].zone_id,
					0,
					pp.binds[4].x,
					pp.binds[4].y,
					pp.binds[4].z,
					pp.binds[4].heading,
					4
				);
				auto results_bset = QueryDatabase(query);
			}
			/* If no regular bind set, set it */
			if (has_bind == 0) {
				std::string query = fmt::format(
					SQL(
						REPLACE INTO
						`character_bind`
						(`id`, `zone_id`, `instance_id`, `x`, `y`, `z`, `heading`, `slot`)
						VALUES ({}, {}, {}, {}, {}, {}, {}, {})
					),
					character_id,
					pp.binds[0].zone_id,
					0,
					pp.binds[0].x,
					pp.binds[0].y,
					pp.binds[0].z,
					pp.binds[0].heading,
					0
				);
				auto results_bset = QueryDatabase(query);
			}
		}
		/* If our bind count is less than 5, then we have null data that needs to be filled in. */
		if (bind_count < 5) {
			// we know that home and main bind must be valid here, so we don't check those
			// we also use home to fill in the null data like live does.
			for (int i = 1; i < 4; i++) {
				if (pp.binds[i].zone_id != 0) // we assume 0 is the only invalid one ...
					continue;

				std::string query = fmt::format(
					SQL(
						REPLACE INTO
						`character_bind`
						(`id`, `zone_id`, `instance_id`, `x`, `y`, `z`, `heading`, `slot`)
						VALUES ({}, {}, {}, {}, {}, {}, {}, {})
					),
					character_id,
					pp.binds[4].zone_id,
					0,
					pp.binds[4].x,
					pp.binds[4].y,
					pp.binds[4].z,
					pp.binds[4].heading,
					i
				);
				auto results_bset = QueryDatabase(query);
			}
		}

		character_list_query = fmt::format(
			SQL(
				SELECT
				`slot`, `red`, `green`, `blue`, `use_tint`, `color`
				FROM
				`character_material`
				WHERE
				`id` = {}
			),
			character_id
		);
		auto results_b = database.QueryDatabase(character_list_query);
		uint8 slot = 0;
		for (auto row_b = results_b.begin(); row_b != results_b.end(); ++row_b) {
			slot = Strings::ToInt(row_b[0]);
			pp.item_tint.Slot[slot].Red     = Strings::ToInt(row_b[1]);
			pp.item_tint.Slot[slot].Green   = Strings::ToInt(row_b[2]);
			pp.item_tint.Slot[slot].Blue    = Strings::ToInt(row_b[3]);
			pp.item_tint.Slot[slot].UseTint = Strings::ToInt(row_b[4]);
		}

		if (GetCharSelInventory(account_id, p_character_select_entry_struct->Name, &inventory_profile)) {
			const EQ::ItemData *item = nullptr;
			const EQ::ItemInstance *inst = nullptr;
			int16 inventory_slot = 0;
			for (uint32 matslot = EQ::textures::textureBegin; matslot < EQ::textures::materialCount; matslot++) {
				inventory_slot = EQ::InventoryProfile::CalcSlotFromMaterial(matslot);
				if (inventory_slot == INVALID_INDEX) { continue; }
				inst = inventory_profile.GetItem(inventory_slot);
				if (inst == nullptr)
					continue;

				item = inst->GetItem();
				if (item == nullptr)
					continue;

				if (matslot > 6) {
					uint32 item_id_file = 0;
					// Weapon Models
					if (inst->GetOrnamentationIDFile() != 0) {
						item_id_file = inst->GetOrnamentationIDFile();
						p_character_select_entry_struct->Equip[matslot].Material = item_id_file;
					} else {
						if (strlen(item->IDFile) > 2) {
							item_id_file = Strings::ToInt(&item->IDFile[2]);
							p_character_select_entry_struct->Equip[matslot].Material = item_id_file;
						}
					}

					if (matslot == EQ::textures::weaponPrimary) {
						p_character_select_entry_struct->PrimaryIDFile = item_id_file;
					} else {
						p_character_select_entry_struct->SecondaryIDFile = item_id_file;
					}
				} else {
					// Armor Materials/Models
					uint32 color = (
						pp.item_tint.Slot[matslot].UseTint ?
						pp.item_tint.Slot[matslot].Color :
						inst->GetColor()
					);
					p_character_select_entry_struct->Equip[matslot].Material = item->Material;
					p_character_select_entry_struct->Equip[matslot].EliteModel = item->EliteMaterial;
					p_character_select_entry_struct->Equip[matslot].HerosForgeModel = inst->GetOrnamentHeroModel(matslot);
					p_character_select_entry_struct->Equip[matslot].Color = color;
				}
			}
		} else {
			printf("Error loading inventory for %s\n", p_character_select_entry_struct->Name);
		}
		buff_ptr += sizeof(CharacterSelectEntry_Struct);
	}
}

int WorldDatabase::MoveCharacterToBind(int character_id, uint8 bind_number)
{
	/*  if an invalid bind point is specified, use the primary bind */
	if (bind_number > 4)
		bind_number = 0;

	std::string query = fmt::format(
		SQL(
			SELECT
			zone_id, instance_id, x, y, z, heading
			FROM
			character_bind
			WHERE
			id = {}
			AND
			slot = {}
			LIMIT 1
		),
		character_id,
		bind_number
	);
	auto results = database.QueryDatabase(query);
	if(!results.Success() || results.RowCount() == 0) {
		return 0;
	}

	int zone_id, instance_id;
	double x, y, z, heading;
	for (auto row = results.begin(); row != results.end(); ++row) {
		zone_id = Strings::ToInt(row[0]);
		instance_id = Strings::ToInt(row[1]);
		x = Strings::ToFloat(row[2]);
		y = Strings::ToFloat(row[3]);
		z = Strings::ToFloat(row[4]);
		heading = Strings::ToFloat(row[5]);
	}

	query = fmt::format(
		SQL(
			UPDATE
			`character_data`
			SET
			`zone_id` = {},
			`zone_instance` = {},
			`x` = {},
			`y` = {},
			`z` = {},
			`heading` = {}
			WHERE `id` = {}
		),
		zone_id,
		instance_id,
		x,
		y,
		z,
		heading,
		character_id
	);

	results = database.QueryDatabase(query);
	if(!results.Success()) {
		return 0;
	}

	return zone_id;
}

int WorldDatabase::MoveCharacterToInstanceSafeReturn(
	int character_id,
	int instance_zone_id,
	int instance_id
)
{
	int zone_id = 0;

	// only moves if safe return is for specified zone instance
	auto entries = CharacterInstanceSafereturnsRepository::GetWhere(
		database,
		fmt::format(
			"character_id = {} AND instance_zone_id = {} AND instance_id = {} AND safe_zone_id > 0",
			character_id, instance_zone_id, instance_id
		)
	);

	if (!entries.empty()) {
		auto entry = entries.front();

		auto results = QueryDatabase(
			fmt::format(
				SQL(
					UPDATE character_data
					SET zone_id = {}, zone_instance = 0, x = {}, y = {}, z = {}, heading = {}
						WHERE id = {};
				),
				entry.safe_zone_id,
				entry.safe_x,
				entry.safe_y,
				entry.safe_z,
				entry.safe_heading,
				character_id
			)
		);

		if (results.Success() && results.RowsAffected() > 0) {
			zone_id = entry.safe_zone_id;
		}
	}

	if (zone_id == 0) {
		zone_id = MoveCharacterToBind(character_id);
	}

	return zone_id;
}

bool WorldDatabase::GetStartZone(
	PlayerProfile_Struct *pp,
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
	if (!pp || !p_char_create_struct) {
		return false;
	}

	pp->x                    = 0;
	pp->y                    = 0;
	pp->z                    = 0;
	pp->heading              = 0;
	pp->zone_id              = 0;
	pp->binds[0].x           = 0;
	pp->binds[0].y           = 0;
	pp->binds[0].z           = 0;
	pp->binds[0].zone_id     = 0;
	pp->binds[0].instance_id = 0;

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
		is_titanium ? SetTitaniumDefaultStartZone(pp, p_char_create_struct) : SetSoFDefaultStartZone(pp, p_char_create_struct);
    }
    else {
		LogInfo("Found starting location in start_zones");
		auto row = results.begin();
		pp->x                = Strings::ToFloat(row[0]);
		pp->y                = Strings::ToFloat(row[1]);
		pp->z                = Strings::ToFloat(row[2]);
		pp->heading          = Strings::ToFloat(row[3]);
		pp->zone_id          = Strings::ToInt(row[4]);
		pp->binds[0].zone_id = Strings::ToInt(row[5]);
		pp->binds[0].x       = Strings::ToFloat(row[6]);
		pp->binds[0].y       = Strings::ToFloat(row[7]);
		pp->binds[0].z       = Strings::ToFloat(row[8]);
		pp->binds[0].heading = Strings::ToFloat(row[3]);
	}

	if (
		pp->x == 0 &&
		pp->y == 0 &&
		pp->z == 0 &&
		pp->heading == 0
	) {
		auto zone = GetZone(pp->zone_id);
		if (zone) {
			pp->x       = zone->safe_x;
			pp->y       = zone->safe_y;
			pp->z       = zone->safe_z;
			pp->heading = zone->safe_heading;
		}
	}

	if (
		pp->binds[0].x == 0 &&
		pp->binds[0].y == 0 &&
		pp->binds[0].z == 0 &&
		pp->binds[0].heading == 0
	) {
		auto zone = GetZone(pp->binds[0].zone_id);
		if (zone) {
			pp->binds[0].x       = zone->safe_x;
			pp->binds[0].y       = zone->safe_y;
			pp->binds[0].z       = zone->safe_z;
			pp->binds[0].heading = zone->safe_heading;
		}
	}

	return true;
}

void WorldDatabase::SetSoFDefaultStartZone(PlayerProfile_Struct* in_pp, CharCreate_Struct* in_cc){
	int sof_start_zone_id = RuleI(World, SoFStartZoneID);
	if (sof_start_zone_id > 0) {
			in_pp->zone_id = sof_start_zone_id;
			in_cc->start_zone = in_pp->zone_id;
	}
	else if (in_cc->start_zone == RuleI(World, TutorialZoneID)) {
		in_pp->zone_id = in_cc->start_zone;
	}
	else {
		in_pp->x = in_pp->binds[0].x = -51.0f;
		in_pp->y = in_pp->binds[0].y = -20.0f;
		in_pp->z = in_pp->binds[0].z = 0.79f;
		in_pp->heading = in_pp->binds[0].heading = 0.0f;
		in_pp->zone_id = in_pp->binds[0].zone_id = Zones::CRESCENT; // Crescent Reach.
	}
}

void WorldDatabase::SetTitaniumDefaultStartZone(PlayerProfile_Struct* in_pp, CharCreate_Struct* in_cc)
{
	int titanium_start_zone_id = RuleI(World, TitaniumStartZoneID);
	if (titanium_start_zone_id > 0) {
		in_pp->zone_id = titanium_start_zone_id;
		in_pp->binds[0].zone_id = titanium_start_zone_id;
	} else {
		switch (in_cc->start_zone)
		{
			case StartZoneIndex::Odus:
			{
				if (in_cc->deity == Deity::CazicThule) // Cazic-Thule Erudites go to Paineel
				{
					in_pp->zone_id = Zones::PAINEEL; // paineel
					in_pp->binds[0].zone_id = Zones::PAINEEL;
				}
				else
				{
					in_pp->zone_id = Zones::ERUDNEXT;	// erudnext
					in_pp->binds[0].zone_id = Zones::TOX;	// tox
				}
				break;
			}
			case StartZoneIndex::Qeynos:
			{
				in_pp->zone_id = Zones::QEYNOS2;	// qeynos2
				in_pp->binds[0].zone_id = Zones::QEYNOS2;	// qeynos2
				break;
			}
			case StartZoneIndex::Halas:
			{
				in_pp->zone_id = Zones::HALAS;	// halas
				in_pp->binds[0].zone_id = Zones::EVERFROST;	// everfrost
				break;
			}
			case StartZoneIndex::Rivervale:
			{
				in_pp->zone_id = Zones::RIVERVALE;	// rivervale
				in_pp->binds[0].zone_id = Zones::KITHICOR;	// kithicor
				break;
			}
			case StartZoneIndex::Freeport:
			{
				in_pp->zone_id = Zones::FREPORTW;	// freportw
				in_pp->binds[0].zone_id = Zones::FREPORTW;	// freportw
				break;
			}
			case StartZoneIndex::Neriak:
			{
				in_pp->zone_id = Zones::NERIAKA;	// neriaka
				in_pp->binds[0].zone_id = Zones::NEKTULOS;	// nektulos
				break;
			}
			case StartZoneIndex::Grobb:
			{
				in_pp->zone_id = Zones::GROBB;	// grobb
				in_pp->binds[0].zone_id = Zones::INNOTHULE;	// innothule
				break;
			}
			case StartZoneIndex::Oggok:
			{
				in_pp->zone_id = Zones::OGGOK;	// oggok
				in_pp->binds[0].zone_id = Zones::FEERROTT;	// feerrott
				break;
			}
			case StartZoneIndex::Kaladim:
			{
				in_pp->zone_id = Zones::KALADIMA;	// kaladima
				in_pp->binds[0].zone_id = Zones::BUTCHER;	// butcher
				break;
			}
			case StartZoneIndex::GreaterFaydark:
			{
				in_pp->zone_id = Zones::GFAYDARK;	// gfaydark
				in_pp->binds[0].zone_id = Zones::GFAYDARK;	// gfaydark
				break;
			}
			case StartZoneIndex::Felwithe:
			{
				in_pp->zone_id = Zones::FELWITHEA;	// felwithea
				in_pp->binds[0].zone_id = Zones::GFAYDARK;	// gfaydark
				break;
			}
			case StartZoneIndex::Akanon:
			{
				in_pp->zone_id = Zones::AKANON;	// akanon
				in_pp->binds[0].zone_id = Zones::STEAMFONT;	// steamfont
				break;
			}
			case StartZoneIndex::Cabilis:
			{
				in_pp->zone_id = Zones::CABWEST;	// cabwest
				in_pp->binds[0].zone_id = Zones::FIELDOFBONE;	// fieldofbone
				break;
			}
			case StartZoneIndex::SharVahl:
			{
				in_pp->zone_id = Zones::SHARVAHL;	// sharvahl
				in_pp->binds[0].zone_id = Zones::SHARVAHL;	// sharvahl
				break;
			}
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
    level = Strings::ToInt(row[0]);

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
		allocate.Index = Strings::ToInt(row[0]);
		allocate.BaseStats[0] = Strings::ToInt(row[1]);
		allocate.BaseStats[3] = Strings::ToInt(row[2]);
		allocate.BaseStats[1] = Strings::ToInt(row[3]);
		allocate.BaseStats[2] = Strings::ToInt(row[4]);
		allocate.BaseStats[4] = Strings::ToInt(row[5]);
		allocate.BaseStats[5] = Strings::ToInt(row[6]);
		allocate.BaseStats[6] = Strings::ToInt(row[7]);
		allocate.DefaultPointAllocation[0] = Strings::ToInt(row[8]);
		allocate.DefaultPointAllocation[3] = Strings::ToInt(row[9]);
		allocate.DefaultPointAllocation[1] = Strings::ToInt(row[10]);
		allocate.DefaultPointAllocation[2] = Strings::ToInt(row[11]);
		allocate.DefaultPointAllocation[4] = Strings::ToInt(row[12]);
		allocate.DefaultPointAllocation[5] = Strings::ToInt(row[13]);
		allocate.DefaultPointAllocation[6] = Strings::ToInt(row[14]);

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
		combo.AllocationIndex = Strings::ToInt(row[0]);
		combo.Race = Strings::ToInt(row[1]);
		combo.Class = Strings::ToInt(row[2]);
		combo.Deity = Strings::ToInt(row[3]);
		combo.Zone = Strings::ToInt(row[4]);
		combo.ExpansionRequired = Strings::ToInt(row[5]);

		character_create_race_class_combos.push_back(combo);
	}

	return true;
}

// this is a slightly modified version of SharedDatabase::GetInventory(...) for character select use-only
bool WorldDatabase::GetCharSelInventory(uint32 account_id, char *name, EQ::InventoryProfile *inv)
{
	if (!account_id || !name || !inv) {
		return false;
	}

	const uint32 character_id = GetCharacterID(name);

	if (!character_id) {
		return false;
	}

	const auto& l = InventoryRepository::GetWhere(
		*this,
		fmt::format(
			"`character_id` = {} AND `slot_id` BETWEEN {} AND {}",
			character_id,
			EQ::invslot::slotHead,
			EQ::invslot::slotFeet
		)
	);

	if (l.empty()) {
		return true;
	}

	for (const auto& e : l) {
		switch (e.slot_id) {
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


		uint32 augment_ids[EQ::invaug::SOCKET_COUNT] = {
			e.augment_one,
			e.augment_two,
			e.augment_three,
			e.augment_four,
			e.augment_five,
			e.augment_six
		};

		const EQ::ItemData* item = content_db.GetItem(e.item_id);
		if (!item) {
			continue;
		}

		EQ::ItemInstance *inst = content_db.CreateBaseItem(item, e.charges);

		if (!inst) {
			continue;
		}

		inst->SetCharges(e.charges);

		if (e.color > 0) {
			inst->SetColor(e.color);
		}

		if (item->IsClassCommon()) {
			for (int i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
				if (augment_ids[i]) {
					inst->PutAugment(this, i, augment_ids[i]);
				}
			}
		}

		inst->SetAttuned(e.instnodrop);

		if (!e.custom_data.empty()) {
			inst->SetCustomDataString(e.custom_data);
		}

		inst->SetOrnamentIcon(e.ornament_icon);
		inst->SetOrnamentationIDFile(e.ornament_idfile);
		inst->SetOrnamentHeroModel(e.ornament_hero_model);

		inv->PutItem(e.slot_id, *inst);

		safe_delete(inst);
	}

	return true;
}
