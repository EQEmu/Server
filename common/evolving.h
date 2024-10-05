/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2005 EQEMu Development Team (http://eqemulator.net)

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
#ifndef EVOLVING_H
#define EVOLVING_H

#include "shareddb.h"
#include "events/player_events.h"
#include "repositories/items_evolving_details_repository.h"

namespace EQ {
	class ItemInstance;
}

class EvolvingItemsManager
{
public:
	EvolvingItemsManager();
	void SetDatabase(Database *db);
	void SetContentDatabase(Database *db);

	void LoadEvolvingItems() const;
	void DoLootChecks(uint32 char_id, uint16 slot_id, const EQ::ItemInstance &inst) const;
	uint32 GetFinalItemID(const EQ::ItemInstance &inst) const;
	uint32 GetNextEvolveItemID(const EQ::ItemInstance &inst) const;
	uint32 GetFirstItemInLoreGroup(uint32 lore_id);
	uint32 GetFirstItemInLoreGroupByItemID(uint32 item_id);
	uint64 GetTotalEarnedXP(const EQ::ItemInstance &inst);
	static double CalculateProgression(uint64 current_amount, uint32 item_id);
	static void LoadPlayerEvent(const EQ::ItemInstance &inst, PlayerEvent::EvolveItem &e);
	ItemsEvolvingDetailsRepository::ItemsEvolvingDetails GetEvolveItemDetails(uint64 id);
	EvolveTransfer_Struct DetermineTransferResults(const EQ::ItemInstance& inst_from, const EQ::ItemInstance& inst_to);
	EvolveTransfer2_Struct GetNextItemByXP(const EQ::ItemInstance &inst_in, int64 in_xp);
	std::map<uint32, ItemsEvolvingDetailsRepository::ItemsEvolvingDetails>& GetEvolvingItemsCache() { return evolving_items_cache; }
	std::vector<ItemsEvolvingDetailsRepository::ItemsEvolvingDetails> GetEvolveIDItems(uint32 evolve_id);

private:
	std::map<uint32, ItemsEvolvingDetailsRepository::ItemsEvolvingDetails> evolving_items_cache;
	Database *                                                             m_db;
	Database *                                                             m_content_db;
};

extern EvolvingItemsManager evolving_items_manager;

#endif //EVOLVING_H
