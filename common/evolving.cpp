/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2003 EQEMu Development Team (http://eqemulator.net)

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

#include "evolving.h"

#include "item_instance.h"
#include "rulesys.h"
#include "shareddb.h"
#include "events/player_event_logs.h"
#include "repositories/character_evolving_items_repository.h"

EvolvingItemsManager::EvolvingItemsManager()
{
	m_db         = nullptr;
	m_content_db = nullptr;
}

void EvolvingItemsManager::LoadEvolvingItems() const
{
	auto const& results = ItemsEvolvingDetailsRepository::All(*m_db);

	if (results.empty()) {
		return;
	}

	std::ranges::transform(results.begin(), results.end(),
				   std::inserter(evolving_items_manager.GetEvolvingItemsCache(), evolving_items_manager.GetEvolvingItemsCache().end()),
				   [](const ItemsEvolvingDetailsRepository::ItemsEvolvingDetails& x) {
					   return std::make_pair(x.item_id, x);
				   }
	);
}

void EvolvingItemsManager::SetDatabase(Database *db)
{
	m_db = db;
}

void EvolvingItemsManager::SetContentDatabase(Database *db)
{
	m_content_db = db;
}

double EvolvingItemsManager::CalculateProgression(const uint64 current_amount, const uint32 item_id)
{
	if (!evolving_items_manager.GetEvolvingItemsCache().contains(item_id)) {
		return 0;
	}

	return evolving_items_manager.GetEvolvingItemsCache().at(item_id).required_amount > 0
			   ? static_cast<double>(current_amount)
				 / static_cast<double>(evolving_items_manager.GetEvolvingItemsCache().at(item_id).required_amount) * 100
			   : 0;
}

void EvolvingItemsManager::DoLootChecks(const uint32 char_id, const uint16 slot_id, const EQ::ItemInstance& inst) const
{
	inst.SetEvolveEquiped(false);
	if (inst.IsEvolving() && slot_id <= EQ::invslot::EQUIPMENT_END && slot_id >= EQ::invslot::EQUIPMENT_BEGIN) {
		inst.SetEvolveEquiped(true);
	}

	if (!inst.IsEvolving()) {
		return;
	}

	if (!inst.GetEvolveUniqueID()) {
		auto e = CharacterEvolvingItemsRepository::NewEntity();

		e.char_id       = char_id;
		e.item_id       = inst.GetID();
		e.equiped       = inst.GetEvolveEquiped();
		e.final_item_id = evolving_items_manager.GetFinalItemID(inst);

		auto r = CharacterEvolvingItemsRepository::InsertOne(*m_db, e);
		e.id = r.id;

		inst.SetEvolveUniqueID(e.id);
		inst.SetEvolveCharID(e.char_id);
		inst.SetEvolveItemID(e.item_id);
		inst.SetEvolveFinalItemID(e.final_item_id);

		return;
	}

	CharacterEvolvingItemsRepository::SetEquiped(*m_db, inst.GetEvolveUniqueID(), inst.GetEvolveEquiped());
}

uint32 EvolvingItemsManager::GetFinalItemID(const EQ::ItemInstance& inst) const
{
	const auto start_iterator = std::ranges::find_if(
		evolving_items_manager.GetEvolvingItemsCache().cbegin(),
		evolving_items_manager.GetEvolvingItemsCache().cend(),
		[&](const std::pair<uint32, ItemsEvolvingDetailsRepository::ItemsEvolvingDetails> &a) {
			return a.second.item_evo_id == inst.GetEvolveLoreID();
		}
		);

	if (start_iterator == std::end(evolving_items_manager.GetEvolvingItemsCache())) {
		return 0;
	}

	const auto final_id = std::ranges::max_element(
		start_iterator,
		evolving_items_manager.GetEvolvingItemsCache().cend(),
		[&](const std::pair<uint32, ItemsEvolvingDetailsRepository::ItemsEvolvingDetails> &a,
		    const std::pair<uint32, ItemsEvolvingDetailsRepository::ItemsEvolvingDetails> &b) {
			return a.second.item_evo_id == b.second.item_evo_id &&
			       a.second.item_evolve_level < b.second.item_evolve_level;
		}
		);

	return final_id->first;
}

uint32 EvolvingItemsManager::GetNextEvolveItemID(const EQ::ItemInstance& inst) const
{
	int8 current_level = inst.GetEvolveLvl();

	const auto iterator = std::ranges::find_if(
		evolving_items_manager.GetEvolvingItemsCache().cbegin(),
		evolving_items_manager.GetEvolvingItemsCache().cend(),
		[&](const std::pair<uint32, ItemsEvolvingDetailsRepository::ItemsEvolvingDetails> &a) {
			return a.second.item_evo_id == inst.GetEvolveLoreID() &&
			       a.second.item_evolve_level == current_level + 1;
		}
		);

	if (iterator == std::end(evolving_items_manager.GetEvolvingItemsCache())) {
		return 0;
	}

	return iterator->first;
}

ItemsEvolvingDetailsRepository::ItemsEvolvingDetails EvolvingItemsManager::GetEvolveItemDetails(const uint64 id)
{
	if (GetEvolvingItemsCache().contains(id)) {
		return GetEvolvingItemsCache().at(id);
	}

	return ItemsEvolvingDetailsRepository::NewEntity();
}

std::vector<ItemsEvolvingDetailsRepository::ItemsEvolvingDetails> EvolvingItemsManager::GetEvolveIDItems(
	const uint32 evolve_id)
{
	std::vector<ItemsEvolvingDetailsRepository::ItemsEvolvingDetails> e{};

	for (auto const& [key, value] : GetEvolvingItemsCache()) {
		if (value.item_evo_id == evolve_id) {
			e.push_back(value);
		}
	}

	std::ranges::sort(e.begin(), e.end(),
	          [&](ItemsEvolvingDetailsRepository::ItemsEvolvingDetails const &a,
	              ItemsEvolvingDetailsRepository::ItemsEvolvingDetails const &b) {
		          return a.item_evolve_level < b.item_evolve_level;
	          }
	);

	return e;
}

uint64 EvolvingItemsManager::GetTotalEarnedXP(const EQ::ItemInstance &inst)
{
	if (!inst) {
		return 0;
	}

	uint64 xp                 = inst.GetEvolveCurrentAmount();
	auto evolve_id_item_cache = GetEvolveIDItems(inst.GetEvolveLoreID());
	auto current_level        = inst.GetEvolveLvl();

	for (auto const& i:evolve_id_item_cache) {
		if (i.item_evolve_level < current_level) {
			xp += i.required_amount;
		}
	}

	return xp;
}

EvolveTransfer2_Struct EvolvingItemsManager::GetNextItemByXP(const EQ::ItemInstance &inst_in, uint64 in_xp)
{
	EvolveTransfer2_Struct ets{};
	auto evolve_items = GetEvolveIDItems(inst_in.GetEvolveLoreID());
	int64 free_xp     = static_cast<int64>(in_xp);
	uint32 max_transfer_level = 0;

	for (auto const& e:evolve_items) {
		if (e.item_evolve_level < inst_in.GetEvolveLvl()) {
			continue;
		}

		if (e.item_evolve_level == inst_in.GetEvolveLvl()) {
			if (e.required_amount - inst_in.GetEvolveCurrentAmount() <= in_xp) {
				ets.item_id = e.item_id;
				ets.current_amount = e.required_amount;
				max_transfer_level = 0;
				free_xp -= e.required_amount - inst_in.GetEvolveCurrentAmount();
			} else {
				ets.item_id = e.item_id;
				ets.current_amount = inst_in.GetEvolveCurrentAmount() + in_xp;
				max_transfer_level = 0;
				free_xp = 0;
				return ets;
			}
		}
		else {
			if (e.required_amount - inst_in.GetEvolveCurrentAmount() <= in_xp) {
				ets.item_id = e.item_id;
				ets.current_amount += e.required_amount;
				max_transfer_level += 1;
				free_xp -= e.required_amount - inst_in.GetEvolveCurrentAmount();
			} else {
				ets.item_id = e.item_id;
				ets.current_amount += inst_in.GetEvolveCurrentAmount() + in_xp;
				max_transfer_level += 1;
				free_xp = 0;
				return ets;
			}
		}

		if (free_xp <= 0) {
			return ets;
		}
	}

	return ets;
}

EvolveTransfer_Struct EvolvingItemsManager::DetermineTransferResults(SharedDatabase &db, const EQ::ItemInstance& inst_from, const EQ::ItemInstance& inst_to)
{
	EvolveTransfer_Struct ets{};
	// EQ::ItemInstance *inst_from_new = nullptr;
	// EQ::ItemInstance *inst_to_new   = nullptr;

	uint32 compatibility = 20;
	uint32 max_transfer_level = 0;

	auto evolving_details_inst_from = evolving_items_manager.GetEvolveItemDetails(inst_from.GetID());
	auto evolving_details_inst_to   = evolving_items_manager.GetEvolveItemDetails(inst_to.GetID());
	if (!evolving_details_inst_from.id || !evolving_details_inst_to.id) {
		return ets;
	}

	if (evolving_details_inst_from.type == evolving_details_inst_to.type) {
		uint64 xp = 0;
		if (evolving_details_inst_from.sub_type == evolving_details_inst_to.sub_type) {
			compatibility = 100;
		}
		else {
			compatibility = 50;
		}

		xp                  = evolving_items_manager.GetTotalEarnedXP(inst_from) * compatibility / 100;
		auto new_item = evolving_items_manager.GetNextItemByXP(inst_to, xp);

		ets.item_from_id = new_item.item_id
		ets.inst_1             = inst_from_new;
		ets.inst_2             = inst_to_new;
		ets.compatibility      = compatibility;
		ets.max_transfer_level = max_transfer_level;

		// if (new_item.item_id == inst_to.GetID()) {
		// 	ets.item_from_id             = inst_from.GetID();
		// 	ets.item_from_current_amount = 0;
		// 	ets.item_to_id               = new_item.item_id;
		// 	ets.item_to_current_amount   = new_item.current_amount;
		// }
		// else {
		// 	inst_to_new        = db.CreateItem(new_item.item_id);
		// 	inst_from_new      = inst_from.Clone();
		// }

		// max_transfer_level = inst_from_new->GetEvolveLvl() - inst_from.GetEvolveLvl();
		// inst_to_new->SetEvolveAddToCurrentAmount(new_item.current_amount);
		// inst_to_new->SetEvolveProgression2();
		// inst_from_new->SetEvolveCurrentAmount(0);
		// inst_from_new->SetEvolveProgression2();
	}

	return ets;
}
