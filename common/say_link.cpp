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
#include <algorithm>

bool EQ::saylink::DegenerateLinkBody(SayLinkBody_Struct &say_link_body_struct, const std::string &say_link_body)
{
	memset(&say_link_body_struct, 0, sizeof(say_link_body_struct));
	if (say_link_body.length() != EQ::constants::SAY_LINK_BODY_SIZE) {
		return false;
	}

	say_link_body_struct.action_id     = (uint8) strtol(say_link_body.substr(0, 1).c_str(), nullptr, 16);
	say_link_body_struct.item_id       = (uint32) strtol(say_link_body.substr(1, 5).c_str(), nullptr, 16);
	say_link_body_struct.augment_1     = (uint32) strtol(say_link_body.substr(6, 5).c_str(), nullptr, 16);
	say_link_body_struct.augment_2     = (uint32) strtol(say_link_body.substr(11, 5).c_str(), nullptr, 16);
	say_link_body_struct.augment_3     = (uint32) strtol(say_link_body.substr(16, 5).c_str(), nullptr, 16);
	say_link_body_struct.augment_4     = (uint32) strtol(say_link_body.substr(21, 5).c_str(), nullptr, 16);
	say_link_body_struct.augment_5     = (uint32) strtol(say_link_body.substr(26, 5).c_str(), nullptr, 16);
	say_link_body_struct.augment_6     = (uint32) strtol(say_link_body.substr(31, 5).c_str(), nullptr, 16);
	say_link_body_struct.is_evolving   = (uint8) strtol(say_link_body.substr(36, 1).c_str(), nullptr, 16);
	say_link_body_struct.evolve_group  = (uint32) strtol(say_link_body.substr(37, 4).c_str(), nullptr, 16);
	say_link_body_struct.evolve_level  = (uint8) strtol(say_link_body.substr(41, 2).c_str(), nullptr, 16);
	say_link_body_struct.ornament_icon = (uint32) strtol(say_link_body.substr(43, 5).c_str(), nullptr, 16);
	say_link_body_struct.hash          = (uint32) strtol(say_link_body.substr(48, 8).c_str(), nullptr, 16);

	return true;
}

bool EQ::saylink::GenerateLinkBody(std::string &say_link_body, const SayLinkBody_Struct &say_link_body_struct)
{
	say_link_body = StringFormat(
		"%1X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%1X" "%04X" "%02X" "%05X" "%08X",
		(0x0F & say_link_body_struct.action_id),
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

	if (say_link_body.length() != EQ::constants::SAY_LINK_BODY_SIZE) {
		return false;
	}

	return true;
}

EQ::SayLinkEngine::SayLinkEngine()
{
	Reset();
}

const std::string &EQ::SayLinkEngine::GenerateLink()
{
	m_Link.clear();
	m_LinkBody.clear();
	m_LinkText.clear();

	generate_body();
	generate_text();

	if ((m_LinkBody.length() == EQ::constants::SAY_LINK_BODY_SIZE) && (m_LinkText.length() > 0)) {
		m_Link.push_back(0x12);
		m_Link.append(m_LinkBody);
		m_Link.append(m_LinkText);
		m_Link.push_back(0x12);
	}

	if ((m_Link.length() == 0) || (m_Link.length() > (EQ::constants::SAY_LINK_MAXIMUM_SIZE))) {
		m_Error = true;
		m_Link  = "<LINKER ERROR>";
		LogError("SayLinkEngine::GenerateLink() failed to generate a useable say link");
		LogError(">> LinkType: {}, Lengths: [link: {}({}), body: {}({}), text: {}({})]",
				 m_LinkType,
				 m_Link.length(),
				 EQ::constants::SAY_LINK_MAXIMUM_SIZE,
				 m_LinkBody.length(),
				 EQ::constants::SAY_LINK_BODY_SIZE,
				 m_LinkText.length(),
				 EQ::constants::SAY_LINK_TEXT_SIZE
		);
		LogError(">> LinkBody: {}", m_LinkBody.c_str());
		LogError(">> LinkText: {}", m_LinkText.c_str());
	}

	return m_Link;
}

void EQ::SayLinkEngine::Reset()
{
	m_LinkType = saylink::SayLinkBlank;
	m_ItemData = nullptr;
	m_LootData = nullptr;
	m_ItemInst = nullptr;

	memset(&m_LinkBodyStruct, 0, sizeof(SayLinkBody_Struct));
	memset(&m_LinkProxyStruct, 0, sizeof(SayLinkProxy_Struct));

	m_TaskUse = false;
	m_Link.clear();
	m_LinkBody.clear();
	m_LinkText.clear();
	m_Error = false;
}

void EQ::SayLinkEngine::generate_body()
{
	/*
	Current server mask: EQClientRoF2

	RoF2: "%1X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%1X" "%04X" "%02X" "%05X" "%08X" (56)
	RoF:  "%1X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%1X" "%04X" "%1X"  "%05X" "%08X" (55)
	SoF:  "%1X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X"        "%1X" "%04X" "%1X"  "%05X" "%08X" (50)
	6.2:  "%1X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X"        "%1X" "%04X" "%1X"         "%08X" (45)
	*/

	memset(&m_LinkBodyStruct, 0, sizeof(SayLinkBody_Struct));

	const EQ::ItemData *item_data = nullptr;

	switch (m_LinkType) {
		case saylink::SayLinkBlank:
			break;
		case saylink::SayLinkItemData:
			if (m_ItemData == nullptr) { break; }
			m_LinkBodyStruct.item_id      = m_ItemData->ID;
			m_LinkBodyStruct.evolve_group = m_ItemData->LoreGroup; // this probably won't work for all items
			//m_LinkBodyStruct.evolve_level = m_ItemData->EvolvingLevel;
			// TODO: add hash call
			break;
		case saylink::SayLinkLootItem:
			if (m_LootData == nullptr) { break; }
			item_data = database.GetItem(m_LootData->item_id);
			if (item_data == nullptr) { break; }
			m_LinkBodyStruct.item_id      = item_data->ID;
			m_LinkBodyStruct.augment_1    = m_LootData->aug_1;
			m_LinkBodyStruct.augment_2    = m_LootData->aug_2;
			m_LinkBodyStruct.augment_3    = m_LootData->aug_3;
			m_LinkBodyStruct.augment_4    = m_LootData->aug_4;
			m_LinkBodyStruct.augment_5    = m_LootData->aug_5;
			m_LinkBodyStruct.augment_6    = m_LootData->aug_6;
			m_LinkBodyStruct.evolve_group = item_data->LoreGroup; // see note above
			//m_LinkBodyStruct.evolve_level = item_data->EvolvingLevel;
			// TODO: add hash call
			break;
		case saylink::SayLinkItemInst:
			if (m_ItemInst == nullptr) { break; }
			if (m_ItemInst->GetItem() == nullptr) { break; }
			m_LinkBodyStruct.item_id       = m_ItemInst->GetItem()->ID;
			m_LinkBodyStruct.augment_1     = m_ItemInst->GetAugmentItemID(0);
			m_LinkBodyStruct.augment_2     = m_ItemInst->GetAugmentItemID(1);
			m_LinkBodyStruct.augment_3     = m_ItemInst->GetAugmentItemID(2);
			m_LinkBodyStruct.augment_4     = m_ItemInst->GetAugmentItemID(3);
			m_LinkBodyStruct.augment_5     = m_ItemInst->GetAugmentItemID(4);
			m_LinkBodyStruct.augment_6     = m_ItemInst->GetAugmentItemID(5);
			m_LinkBodyStruct.is_evolving   = (m_ItemInst->IsEvolving() ? 1 : 0);
			m_LinkBodyStruct.evolve_group  = m_ItemInst->GetItem()->LoreGroup; // see note above
			m_LinkBodyStruct.evolve_level  = m_ItemInst->GetEvolveLvl();
			m_LinkBodyStruct.ornament_icon = m_ItemInst->GetOrnamentationIcon();
			// TODO: add hash call
			break;
		default:
			break;
	}

	if (m_LinkProxyStruct.action_id) {
		m_LinkBodyStruct.action_id = m_LinkProxyStruct.action_id;
	}
	if (m_LinkProxyStruct.item_id) {
		m_LinkBodyStruct.item_id = m_LinkProxyStruct.item_id;
	}
	if (m_LinkProxyStruct.augment_1) {
		m_LinkBodyStruct.augment_1 = m_LinkProxyStruct.augment_1;
	}
	if (m_LinkProxyStruct.augment_2) {
		m_LinkBodyStruct.augment_2 = m_LinkProxyStruct.augment_2;
	}
	if (m_LinkProxyStruct.augment_3) {
		m_LinkBodyStruct.augment_3 = m_LinkProxyStruct.augment_3;
	}
	if (m_LinkProxyStruct.augment_4) {
		m_LinkBodyStruct.augment_4 = m_LinkProxyStruct.augment_4;
	}
	if (m_LinkProxyStruct.augment_5) {
		m_LinkBodyStruct.augment_5 = m_LinkProxyStruct.augment_5;
	}
	if (m_LinkProxyStruct.augment_6) {
		m_LinkBodyStruct.augment_6 = m_LinkProxyStruct.augment_6;
	}
	if (m_LinkProxyStruct.is_evolving) {
		m_LinkBodyStruct.is_evolving = m_LinkProxyStruct.is_evolving;
	}
	if (m_LinkProxyStruct.evolve_group) {
		m_LinkBodyStruct.evolve_group = m_LinkProxyStruct.evolve_group;
	}
	if (m_LinkProxyStruct.evolve_level) {
		m_LinkBodyStruct.evolve_level = m_LinkProxyStruct.evolve_level;
	}
	if (m_LinkProxyStruct.ornament_icon) {
		m_LinkBodyStruct.ornament_icon = m_LinkProxyStruct.ornament_icon;
	}
	if (m_LinkProxyStruct.hash) {
		m_LinkBodyStruct.hash = m_LinkProxyStruct.hash;
	}


	if (m_TaskUse) {
		m_LinkBodyStruct.hash = 0x14505DC2;
	}

	m_LinkBody = StringFormat(
		"%1X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%1X" "%04X" "%02X" "%05X" "%08X",
		(0x0F & m_LinkBodyStruct.action_id),
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

void EQ::SayLinkEngine::generate_text()
{
	if (m_LinkProxyStruct.text != nullptr) {
		m_LinkText = m_LinkProxyStruct.text;
		return;
	}

	const EQ::ItemData *item_data = nullptr;

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

std::string EQ::SayLinkEngine::GenerateQuestSaylink(std::string saylink_text, bool silent, std::string link_name)
{
	uint32 saylink_id = 0;

	/**
	 * Query for an existing phrase and id in the saylink table
	 */
	std::string query = StringFormat(
		"SELECT `id` FROM `saylink` WHERE `phrase` = '%s' LIMIT 1",
		EscapeString(saylink_text).c_str());

	auto results = database.QueryDatabase(query);

	if (results.Success()) {
		if (results.RowCount() >= 1) {
			for (auto row = results.begin(); row != results.end(); ++row)
				saylink_id = static_cast<uint32>(atoi(row[0]));
		}
		else {
			std::string insert_query = StringFormat(
				"INSERT INTO `saylink` (`phrase`) VALUES ('%s')",
				EscapeString(saylink_text).c_str());

			results = database.QueryDatabase(insert_query);
			if (!results.Success()) {
				LogError("Error in saylink phrase queries {}", results.ErrorMessage().c_str());
			}
			else {
				saylink_id = results.LastInsertedID();
			}
		}
	}

	/**
	 * Generate the actual link
	 */
	EQ::SayLinkEngine linker;
	linker.SetProxyItemID(SAYLINK_ITEM_ID);
	if (silent) {
		linker.SetProxyAugment2ID(saylink_id);
	}
	else {
		linker.SetProxyAugment1ID(saylink_id);
	}

	linker.SetProxyText(link_name.c_str());

	return linker.GenerateLink();
}

std::string EQ::SayLinkEngine::InjectSaylinksIfNotExist(const char *message)
{
	std::string              new_message       = message;
	int                      link_index        = 0;
	int                      saylink_index     = 0;
	std::vector<std::string> links             = {};
	std::vector<std::string> saylinks          = {};
	int                      saylink_length    = 50;
	std::string              saylink_separator = "\u0012";
	std::string              saylink_partial   = "00000";

	LogSaylinkDetail("new_message pre pass 1 [{}]", new_message);

	// first pass - strip existing saylinks by putting placeholder anchors on them
	for (auto &saylink: split_string(new_message, saylink_separator)) {
		if (!saylink.empty() && saylink.length() > saylink_length &&
			saylink.find(saylink_partial) != std::string::npos) {
			saylinks.emplace_back(saylink);

			LogSaylinkDetail("Found saylink [{}]", saylink);

			// replace with anchor
			find_replace(
				new_message,
				fmt::format("{}", saylink),
				fmt::format("<saylink:{}>", saylink_index)
			);

			saylink_index++;
		}
	}

	LogSaylinkDetail("new_message post pass 1 [{}]", new_message);

	LogSaylinkDetail("saylink separator count [{}]", std::count(new_message.begin(), new_message.end(), '\u0012'));

	// loop through brackets until none exist
	if (new_message.find('[') != std::string::npos) {
		for (auto &b: split_string(new_message, "[")) {
			if (!b.empty() && b.find(']') != std::string::npos) {
				std::vector<std::string> right_split = split_string(b, "]");
				if (!right_split.empty()) {
					std::string bracket_message = trim(right_split[0]);

					// we shouldn't see a saylink fragment here, ignore this bracket
					if (bracket_message.find(saylink_partial) != std::string::npos) {
						continue;
					}

					// skip where multiple saylinks are within brackets
					if (bracket_message.find(saylink_separator) != std::string::npos &&
						std::count(bracket_message.begin(), bracket_message.end(), '\u0012') > 1) {
						continue;
					}

					// if non empty bracket contents
					if (!bracket_message.empty()) {
						LogSaylinkDetail("Found bracket_message [{}]", bracket_message);

						// already a saylink
						// todo: improve this later
						if (!bracket_message.empty() &&
							(bracket_message.length() > saylink_length ||
							 bracket_message.find(saylink_separator) != std::string::npos)) {
							links.emplace_back(bracket_message);
						}
						else {
							links.emplace_back(
								EQ::SayLinkEngine::GenerateQuestSaylink(
									bracket_message,
									false,
									bracket_message
								)
							);
						}

						// replace with anchor
						find_replace(
							new_message,
							fmt::format("[{}]", bracket_message),
							fmt::format("<prelink:{}>", link_index)
						);

						link_index++;
					}
				}
			}
		}
	}

	LogSaylinkDetail("new_message post pass 2 (post brackets) [{}]", new_message);

	// strip any current delimiters of saylinks
	find_replace(new_message, saylink_separator, "");

	// pop links onto anchors
	link_index = 0;
	for (auto &link: links) {

		// strip any current delimiters of saylinks
		find_replace(link, saylink_separator, "");

		find_replace(
			new_message,
			fmt::format("<prelink:{}>", link_index),
			fmt::format("[\u0012{}\u0012]", link)
		);
		link_index++;
	}

	LogSaylinkDetail("new_message post pass 3 (post prelink anchor pop) [{}]", new_message);

	// pop links onto anchors
	saylink_index = 0;
	for (auto &link: saylinks) {
		// strip any current delimiters of saylinks
		find_replace(link, saylink_separator, "");

		// check to see if we did a double anchor pass (existing saylink that was also inside brackets)
		// this means we found a saylink and we're checking to see if we're already encoded before double encoding
		if (new_message.find(fmt::format("\u0012<saylink:{}>\u0012", saylink_index)) != std::string::npos) {
			LogSaylinkDetail("Found encoded saylink at index [{}]", saylink_index);

			find_replace(
				new_message,
				fmt::format("\u0012<saylink:{}>\u0012", saylink_index),
				fmt::format("\u0012{}\u0012", link)
			);
			saylink_index++;
			continue;
		}

		find_replace(
			new_message,
			fmt::format("<saylink:{}>", saylink_index),
			fmt::format("\u0012{}\u0012", link)
		);
		saylink_index++;
	}

	LogSaylinkDetail("new_message post pass 4 (post saylink anchor pop) [{}]", new_message);

	return new_message;
}
