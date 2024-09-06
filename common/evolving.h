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

#include "item_instance.h"
#include "repositories/items_evolving_details_repository.h"

class EvolvingItemsManager
{
public:
	EvolvingItemsManager();
	void SetDatabase(Database *db);
	void SetContentDatabase(Database *db);

	void LoadEvolvingItems() const;
	double CalculateProgression(uint64 current_amount, uint32 item_id);
	const EQ::ItemInstance& DoEquipedChecks(Client *c, uint16 slot_id, const EQ::ItemInstance& inst) const;
	std::map<uint32, ItemsEvolvingDetailsRepository::ItemsEvolvingDetails>& GetEvolvingItemsCache() { return evolving_items_cache; }

private:
	std::map<uint32, ItemsEvolvingDetailsRepository::ItemsEvolvingDetails> evolving_items_cache;
	Database *                                                             m_db;
	Database *                                                             m_content_db;
};

extern EvolvingItemsManager evolving_items_manager;

#endif //EVOLVING_H
