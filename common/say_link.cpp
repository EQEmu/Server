/*	EQEMu: Everquest Server Emulator
	
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "say_link.h"
#include "emu_constants.h"

#include "string_util.h"
#include "item_instance.h"
#include "item_data.h"
#include "../zone/zonedb.h"


bool EQEmu::saylink::DegenerateLinkBody(SayLinkBody_Struct& say_link_body_struct, const std::string& say_link_body)
{
	memset(&say_link_body_struct, 0, sizeof(say_link_body_struct));
	if (say_link_body.length() != EQEmu::legacy::TEXT_LINK_BODY_LENGTH)
		return false;

	say_link_body_struct.unknown_1 = (uint8)strtol(say_link_body.substr(0, 1).c_str(), nullptr, 16);
	say_link_body_struct.item_id = (uint32)strtol(say_link_body.substr(1, 5).c_str(), nullptr, 16);
	say_link_body_struct.augment_1 = (uint32)strtol(say_link_body.substr(6, 5).c_str(), nullptr, 16);
	say_link_body_struct.augment_2 = (uint32)strtol(say_link_body.substr(11, 5).c_str(), nullptr, 16);
	say_link_body_struct.augment_3 = (uint32)strtol(say_link_body.substr(16, 5).c_str(), nullptr, 16);
	say_link_body_struct.augment_4 = (uint32)strtol(say_link_body.substr(21, 5).c_str(), nullptr, 16);
	say_link_body_struct.augment_5 = (uint32)strtol(say_link_body.substr(26, 5).c_str(), nullptr, 16);
	say_link_body_struct.augment_6 = (uint32)strtol(say_link_body.substr(31, 5).c_str(), nullptr, 16);
	say_link_body_struct.is_evolving = (uint8)strtol(say_link_body.substr(36, 1).c_str(), nullptr, 16);
	say_link_body_struct.evolve_group = (uint32)strtol(say_link_body.substr(37, 4).c_str(), nullptr, 16);
	say_link_body_struct.evolve_level = (uint8)strtol(say_link_body.substr(41, 2).c_str(), nullptr, 16);
	say_link_body_struct.ornament_icon = (uint32)strtol(say_link_body.substr(43, 5).c_str(), nullptr, 16);
	say_link_body_struct.hash = (int)strtol(say_link_body.substr(48, 8).c_str(), nullptr, 16);

	return true;
}

bool EQEmu::saylink::GenerateLinkBody(std::string& say_link_body, const SayLinkBody_Struct& say_link_body_struct)
{
	say_link_body = StringFormat(
		"%1X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%1X" "%04X" "%02X" "%05X" "%08X",
		(0x0F & say_link_body_struct.unknown_1),
		(0x000FFFFF & say_link_body_struct.item_id),
		(0x000FFFFF & say_link_body_struct.augment_1),
		(0x000FFFFF & say_link_body_struct.augment_2),
		(0x000FFFFF & say_link_body_struct.augment_3),
		(0x000FFFFF & say_link_body_struct.augment_4),
		(0x000FFFFF & say_link_body_struct.augment_5),
		(0x000FFFFF & say_link_body_struct.augment_6),
		(0x0F & say_link_body_struct.is_evolving),
		(0x0000FFFF & say_link_body_struct.evolve_group),
		(0xFF & say_link_body_struct.evolve_level),
		(0x000FFFFF & say_link_body_struct.ornament_icon),
		(0xFFFFFFFF & say_link_body_struct.hash)
	);

	if (say_link_body.length() != EQEmu::legacy::TEXT_LINK_BODY_LENGTH)
		return false;

	return true;
}

EQEmu::SayLinkEngine::SayLinkEngine()
{
	Reset();
}

std::string EQEmu::SayLinkEngine::GenerateLink()
{
	m_Link.clear();
	m_LinkBody.clear();
	m_LinkText.clear();

	generate_body();
	generate_text();

	if ((m_LinkBody.length() == EQEmu::legacy::TEXT_LINK_BODY_LENGTH) && (m_LinkText.length() > 0)) {
		m_Link.push_back(0x12);
		m_Link.append(m_LinkBody);
		m_Link.append(m_LinkText);
		m_Link.push_back(0x12);
	}

	if ((m_Link.length() == 0) || (m_Link.length() > 250)) {
		m_Error = true;
		m_Link = "<LINKER ERROR>";
		Log.Out(Logs::General, Logs::Error, "TextLink::GenerateLink() failed to generate a useable text link (LinkType: %i, Lengths: {link: %u, body: %u, text: %u})",
			m_LinkType, m_Link.length(), m_LinkBody.length(), m_LinkText.length());
		Log.Out(Logs::General, Logs::Error, ">> LinkBody: %s", m_LinkBody.c_str());
		Log.Out(Logs::General, Logs::Error, ">> LinkText: %s", m_LinkText.c_str());
	}

	return m_Link;
}

void EQEmu::SayLinkEngine::Reset()
{
	m_LinkType = saylink::SayLinkBlank;
	m_ItemData = nullptr;
	m_LootData = nullptr;
	m_ItemInst = nullptr;
	m_Proxy_unknown_1 = 0;
	m_ProxyItemID = 0;
	m_ProxyAugment1ID = 0;
	m_ProxyAugment2ID = 0;
	m_ProxyAugment3ID = 0;
	m_ProxyAugment4ID = 0;
	m_ProxyAugment5ID = 0;
	m_ProxyAugment6ID = 0;
	m_ProxyIsEvolving = 0;
	m_ProxyEvolveGroup = 0;
	m_ProxyEvolveLevel = 0;
	m_ProxyOrnamentIcon = 0;
	m_ProxyHash = 0;
	m_ProxyText = nullptr;
	m_TaskUse = false;
	m_Link.clear();
	m_LinkBody.clear();
	m_LinkText.clear();
	m_Error = false;
}

void EQEmu::SayLinkEngine::generate_body()
{
	/*
	Current server mask: EQClientRoF2

	RoF2: "%1X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%1X" "%04X" "%02X" "%05X" "%08X" (56)
	RoF:  "%1X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%1X" "%04X" "%1X"  "%05X" "%08X" (55)
	SoF:  "%1X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X"        "%1X" "%04X" "%1X"  "%05X" "%08X" (50)
	6.2:  "%1X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X"        "%1X" "%04X" "%1X"         "%08X" (45)
	*/

	memset(&m_LinkBodyStruct, 0, sizeof(SayLinkBody_Struct));

	const EQEmu::ItemData* item_data = nullptr;

	switch (m_LinkType) {
	case saylink::SayLinkBlank:
		break;
	case saylink::SayLinkItemData:
		if (m_ItemData == nullptr) { break; }
		m_LinkBodyStruct.item_id = m_ItemData->ID;
		m_LinkBodyStruct.evolve_group = m_ItemData->LoreGroup; // this probably won't work for all items
		//m_LinkBodyStruct.evolve_level = m_ItemData->EvolvingLevel;
		// TODO: add hash call
		break;
	case saylink::SayLinkLootItem:
		if (m_LootData == nullptr) { break; }
		item_data = database.GetItem(m_LootData->item_id);
		if (item_data == nullptr) { break; }
		m_LinkBodyStruct.item_id = item_data->ID;
		m_LinkBodyStruct.augment_1 = m_LootData->aug_1;
		m_LinkBodyStruct.augment_2 = m_LootData->aug_2;
		m_LinkBodyStruct.augment_3 = m_LootData->aug_3;
		m_LinkBodyStruct.augment_4 = m_LootData->aug_4;
		m_LinkBodyStruct.augment_5 = m_LootData->aug_5;
		m_LinkBodyStruct.augment_6 = m_LootData->aug_6;
		m_LinkBodyStruct.evolve_group = item_data->LoreGroup; // see note above
		//m_LinkBodyStruct.evolve_level = item_data->EvolvingLevel;
		// TODO: add hash call
		break;
	case saylink::SayLinkItemInst:
		if (m_ItemInst == nullptr) { break; }
		if (m_ItemInst->GetItem() == nullptr) { break; }
		m_LinkBodyStruct.item_id = m_ItemInst->GetItem()->ID;
		m_LinkBodyStruct.augment_1 = m_ItemInst->GetAugmentItemID(0);
		m_LinkBodyStruct.augment_2 = m_ItemInst->GetAugmentItemID(1);
		m_LinkBodyStruct.augment_3 = m_ItemInst->GetAugmentItemID(2);
		m_LinkBodyStruct.augment_4 = m_ItemInst->GetAugmentItemID(3);
		m_LinkBodyStruct.augment_5 = m_ItemInst->GetAugmentItemID(4);
		m_LinkBodyStruct.augment_6 = m_ItemInst->GetAugmentItemID(5);
		m_LinkBodyStruct.is_evolving = (m_ItemInst->IsEvolving() ? 1 : 0);
		m_LinkBodyStruct.evolve_group = m_ItemInst->GetItem()->LoreGroup; // see note above
		m_LinkBodyStruct.evolve_level = m_ItemInst->GetEvolveLvl();
		m_LinkBodyStruct.ornament_icon = m_ItemInst->GetOrnamentationIcon();
		// TODO: add hash call
		break;
	default:
		break;
	}

	if (m_Proxy_unknown_1)
		m_LinkBodyStruct.unknown_1 = m_Proxy_unknown_1;
	if (m_ProxyItemID)
		m_LinkBodyStruct.item_id = m_ProxyItemID;
	if (m_ProxyAugment1ID)
		m_LinkBodyStruct.augment_1 = m_ProxyAugment1ID;
	if (m_ProxyAugment2ID)
		m_LinkBodyStruct.augment_2 = m_ProxyAugment2ID;
	if (m_ProxyAugment3ID)
		m_LinkBodyStruct.augment_3 = m_ProxyAugment3ID;
	if (m_ProxyAugment4ID)
		m_LinkBodyStruct.augment_4 = m_ProxyAugment4ID;
	if (m_ProxyAugment5ID)
		m_LinkBodyStruct.augment_5 = m_ProxyAugment5ID;
	if (m_ProxyAugment6ID)
		m_LinkBodyStruct.augment_6 = m_ProxyAugment6ID;
	if (m_ProxyIsEvolving)
		m_LinkBodyStruct.is_evolving = m_ProxyIsEvolving;
	if (m_ProxyEvolveGroup)
		m_LinkBodyStruct.evolve_group = m_ProxyEvolveGroup;
	if (m_ProxyEvolveLevel)
		m_LinkBodyStruct.evolve_level = m_ProxyEvolveLevel;
	if (m_ProxyOrnamentIcon)
		m_LinkBodyStruct.ornament_icon = m_ProxyOrnamentIcon;
	if (m_ProxyHash)
		m_LinkBodyStruct.hash = m_ProxyHash;


	if (m_TaskUse)
		m_LinkBodyStruct.hash = 0x14505DC2;

	m_LinkBody = StringFormat(
		"%1X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%1X" "%04X" "%02X" "%05X" "%08X",
		(0x0F & m_LinkBodyStruct.unknown_1),
		(0x000FFFFF & m_LinkBodyStruct.item_id),
		(0x000FFFFF & m_LinkBodyStruct.augment_1),
		(0x000FFFFF & m_LinkBodyStruct.augment_2),
		(0x000FFFFF & m_LinkBodyStruct.augment_3),
		(0x000FFFFF & m_LinkBodyStruct.augment_4),
		(0x000FFFFF & m_LinkBodyStruct.augment_5),
		(0x000FFFFF & m_LinkBodyStruct.augment_6),
		(0x0F & m_LinkBodyStruct.is_evolving),
		(0x0000FFFF & m_LinkBodyStruct.evolve_group),
		(0xFF & m_LinkBodyStruct.evolve_level),
		(0x000FFFFF & m_LinkBodyStruct.ornament_icon),
		(0xFFFFFFFF & m_LinkBodyStruct.hash)
	);
}

void EQEmu::SayLinkEngine::generate_text()
{
	if (m_ProxyText != nullptr) {
		m_LinkText = m_ProxyText;
		return;
	}

	const EQEmu::ItemData* item_data = nullptr;

	switch (m_LinkType) {
	case saylink::SayLinkBlank:
		break;
	case saylink::SayLinkItemData:
		if (m_ItemData == nullptr) { break; }
		m_LinkText = m_ItemData->Name;
		return;
	case saylink::SayLinkLootItem:
		if (m_LootData == nullptr) { break; }
		item_data = database.GetItem(m_LootData->item_id);
		if (item_data == nullptr) { break; }
		m_LinkText = item_data->Name;
		return;
	case saylink::SayLinkItemInst:
		if (m_ItemInst == nullptr) { break; }
		if (m_ItemInst->GetItem() == nullptr) { break; }
		m_LinkText = m_ItemInst->GetItem()->Name;
		return;
	default:
		break;
	}

	m_LinkText = "null";
}
