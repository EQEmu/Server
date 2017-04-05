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

#ifndef COMMON_SAY_LINK_H
#define COMMON_SAY_LINK_H

#include "types.h"

#include <string>


struct ServerLootItem_Struct;

namespace EQEmu
{
	struct ItemData;
	class ItemInstance;
	struct SayLinkBody_Struct;

	namespace saylink {
		enum SayLinkType {
			SayLinkBlank = 0,
			SayLinkItemData,
			SayLinkLootItem,
			SayLinkItemInst
		};

		extern bool DegenerateLinkBody(SayLinkBody_Struct& say_Link_body_struct, const std::string& say_link_body);
		extern bool GenerateLinkBody(std::string& say_link_body, const SayLinkBody_Struct& say_link_body_struct);

	} /*saylink*/

	struct SayLinkBody_Struct {
		uint8 unknown_1;		/* %1X */
		uint32 item_id;			/* %05X */
		uint32 augment_1;		/* %05X */
		uint32 augment_2;		/* %05X */
		uint32 augment_3;		/* %05X */
		uint32 augment_4;		/* %05X */
		uint32 augment_5;		/* %05X */
		uint32 augment_6;		/* %05X */
		uint8 is_evolving;		/* %1X */
		uint32 evolve_group;	/* %05X */
		uint8 evolve_level;		/* %02X */
		uint32 ornament_icon;	/* %05X */
		int hash;				/* %08X */
	};

	class SayLinkEngine {
	public:
		SayLinkEngine();

		void SetLinkType(saylink::SayLinkType link_type) { m_LinkType = link_type; }
		void SetItemData(const EQEmu::ItemData* item_data) { m_ItemData = item_data; }
		void SetLootData(const ServerLootItem_Struct* loot_data) { m_LootData = loot_data; }
		void SetItemInst(const ItemInstance* item_inst) { m_ItemInst = item_inst; }

		// mainly for saylinks..but, not limited to
		void SetProxyUnknown1(uint8 proxy_unknown_1) { m_Proxy_unknown_1 = proxy_unknown_1; }
		void SetProxyItemID(uint32 proxy_item_id) { m_ProxyItemID = proxy_item_id; }
		void SetProxyAugment1ID(uint32 proxy_augment_id) { m_ProxyAugment1ID = proxy_augment_id; }
		void SetProxyAugment2ID(uint32 proxy_augment_id) { m_ProxyAugment2ID = proxy_augment_id; }
		void SetProxyAugment3ID(uint32 proxy_augment_id) { m_ProxyAugment3ID = proxy_augment_id; }
		void SetProxyAugment4ID(uint32 proxy_augment_id) { m_ProxyAugment4ID = proxy_augment_id; }
		void SetProxyAugment5ID(uint32 proxy_augment_id) { m_ProxyAugment5ID = proxy_augment_id; }
		void SetProxyAugment6ID(uint32 proxy_augment_id) { m_ProxyAugment6ID = proxy_augment_id; }
		void SetProxyIsEvolving(uint8 proxy_is_evolving) { m_ProxyIsEvolving = proxy_is_evolving; }
		void SetProxyEvolveGroup(uint32 proxy_evolve_group) { m_ProxyEvolveGroup = proxy_evolve_group; }
		void SetProxyEvolveLevel(uint8 proxy_evolve_level) { m_ProxyEvolveLevel = proxy_evolve_level; }
		void SetProxyOrnamentIcon(uint32 proxy_ornament_icon) { m_ProxyOrnamentIcon = proxy_ornament_icon; }
		void SetProxyHash(int proxy_hash) { m_ProxyHash = proxy_hash; }

		void SetProxyText(const char* proxy_text) { m_ProxyText = proxy_text; } // overrides standard text use
		void SetTaskUse() { m_TaskUse = true; }

		std::string GenerateLink();
		bool LinkError() { return m_Error; }

		std::string Link() { return m_Link; }			// contains full string format: '/12x' '<LinkBody>' '<LinkText>' '/12x'
		std::string LinkBody() { return m_LinkBody; }	// contains string format: '<LinkBody>'
		std::string LinkText() { return m_LinkText; }	// contains string format: '<LinkText>'

		void Reset();

	private:
		void generate_body();
		void generate_text();

		int m_LinkType;
		const ItemData* m_ItemData;
		const ServerLootItem_Struct* m_LootData;
		const ItemInstance* m_ItemInst;

		uint8 m_Proxy_unknown_1;
		uint32 m_ProxyItemID;
		uint32 m_ProxyAugment1ID;
		uint32 m_ProxyAugment2ID;
		uint32 m_ProxyAugment3ID;
		uint32 m_ProxyAugment4ID;
		uint32 m_ProxyAugment5ID;
		uint32 m_ProxyAugment6ID;
		uint8 m_ProxyIsEvolving;
		uint32 m_ProxyEvolveGroup;
		uint8 m_ProxyEvolveLevel;
		uint32 m_ProxyOrnamentIcon;
		int m_ProxyHash;
		const char* m_ProxyText;
		bool m_TaskUse;
		SayLinkBody_Struct m_LinkBodyStruct;
		std::string m_Link;
		std::string m_LinkBody;
		std::string m_LinkText;
		bool m_Error;
	};

} /*EQEmu*/

#endif /*COMMON_SAY_LINK_H*/
