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
#ifndef WORLDDB_H_
#define WORLDDB_H_

#include "../common/shareddb.h"
#include "../common/zone_numbers.h"
#include "../common/eq_packet.h"

struct PlayerProfile_Struct;
struct CharCreate_Struct;
struct CharacterSelect_Struct;


class WorldDatabase : public SharedDatabase {
public:
	bool GetStartZone(PlayerProfile_Struct* p_player_profile_struct, CharCreate_Struct* p_char_create_struct, bool is_titanium);
	void GetCharSelectInfo(uint32 account_id, EQApplicationPacket **out_app, uint32 client_version_bit);
	int MoveCharacterToBind(int CharID, uint8 bindnum = 0);

	void GetLauncherList(std::vector<std::string> &result);
	bool GetCharacterLevel(const char *name, int &level);

	bool LoadCharacterCreateAllocations();
	bool LoadCharacterCreateCombos();

	bool StoreCharacter(
		uint32 account_id,
		PlayerProfile_Struct *p_player_profile_struct,
		EQ::InventoryProfile *p_inventory_profile
	);

private:
	void SetTitaniumDefaultStartZone(PlayerProfile_Struct* in_pp, CharCreate_Struct* in_cc);
	void SetSoFDefaultStartZone(PlayerProfile_Struct* in_pp, CharCreate_Struct* in_cc);

	bool GetCharSelInventory(uint32 account_id, char* name, EQ::InventoryProfile* inv);
};

extern WorldDatabase database;
extern WorldDatabase content_db;

#endif /*WORLDDB_H_*/
