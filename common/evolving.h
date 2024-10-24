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
	EvolveGetNextItem_Struct GetNextItemByXP(const EQ::ItemInstance &inst_in, int64 in_xp);
	std::map<uint32, ItemsEvolvingDetailsRepository::ItemsEvolvingDetails>& GetEvolvingItemsCache() { return evolving_items_cache; }
	std::vector<ItemsEvolvingDetailsRepository::ItemsEvolvingDetails> GetEvolveIDItems(uint32 evolve_id);

private:
	std::map<uint32, ItemsEvolvingDetailsRepository::ItemsEvolvingDetails> evolving_items_cache;
	Database *                                                             m_db;
	Database *                                                             m_content_db;
};

extern EvolvingItemsManager evolving_items_manager;

#endif //EVOLVING_H
