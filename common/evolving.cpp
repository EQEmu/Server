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
#include "events/player_event_logs.h"
#include "repositories/character_evolving_items_repository.h"
#include "../zone/client.h"

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

const EQ::ItemInstance* EvolvingItemsManager::DoLootChecks(Client *c, uint16 slot_id, const EQ::ItemInstance& inst) const
{
	auto inst_clone = inst.Clone();
	inst_clone->SetEvolveEquiped(false);
	if (inst_clone->IsEvolvingItem() && slot_id <= EQ::invslot::EQUIPMENT_END && slot_id >= EQ::invslot::EQUIPMENT_BEGIN) {
		inst_clone->SetEvolveEquiped(true);
		c->GetInv().PutItem(slot_id, *inst_clone);
		if (c->GetEvolvingItems().contains(inst_clone->GetID())) {
			c->GetEvolvingItems().at(inst_clone->GetID()).equiped = true;
			CharacterEvolvingItemsRepository::ReplaceOne(*m_db, c->GetEvolvingItems().at(inst_clone->GetID()));
		}
		else {
			auto e           = CharacterEvolvingItemsRepository::NewEntity();
			e.char_id        = c->CharacterID();
			e.equiped        = true;
			e.item_id        = inst_clone->GetID();

			auto result = CharacterEvolvingItemsRepository::InsertOne(*m_db, e);
			e.id = result.id;
			c->GetEvolvingItems().emplace(inst_clone->GetID(), e);
		}
	} else {
		c->GetInv().PutItem(slot_id, *inst_clone);
		if (c->GetEvolvingItems().contains(inst_clone->GetID())) {
			c->GetEvolvingItems().at(inst_clone->GetID()).equiped = false;
			CharacterEvolvingItemsRepository::ReplaceOne(*m_db, c->GetEvolvingItems().at(inst_clone->GetID()));
		}
		else {
			auto e           = CharacterEvolvingItemsRepository::NewEntity();
			e.char_id        = c->CharacterID();
			e.equiped        = false;
			e.item_id        = inst_clone->GetID();

			auto result = CharacterEvolvingItemsRepository::InsertOne(*m_db, e);
			e.id = result.id;
			c->GetEvolvingItems().emplace(inst_clone->GetID(), e);
		}
	}

	return inst_clone;
}