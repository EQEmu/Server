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

#include "../common/evolving.h"

#include "../common/global_define.h"
#include "../common/rulesys.h"

#include "../common/eq_constants.h"
#include "../common/events/player_event_logs.h"
#include "../common/repositories/character_evolving_items_repository.h"

void Client::DoEvolveItemToggle(const EQApplicationPacket* app)
{
	auto in      = reinterpret_cast<Evolve_Item_Toggle_Struct *>(app->pBuffer);
	auto results = CharacterEvolvingItemsRepository::GetWhere(
		database, fmt::format("`char_id` = '{}' AND `unique_id` = '{}' LIMIT 1;", CharacterID(), in->unique_id)
		);

	if (results.empty()) {
		return;
	}

	auto item      = results.front();
	item.activated = in->activated;

	// update client in memory status
	auto client_evolving_items = GetEvolvingItems();
	if (client_evolving_items.contains(item.item_id)) {
		client_evolving_items.at(item.item_id).activated = in->activated;
	}

	// update db
	CharacterEvolvingItemsRepository::ReplaceOne(database, item);

	// send update to client
	SendEvolvingPacket(EvolvingItems::Actions::UPDATE_ITEMS, item);
}

void Client::SendEvolvingPacket(int8 action, CharacterEvolvingItemsRepository::CharacterEvolvingItems item)
{
	auto out  = std::make_unique<EQApplicationPacket>(OP_EvolveItem, sizeof(Evolve_Item_Toggle_Struct));
	auto data = reinterpret_cast<Evolve_Item_Toggle_Struct *>(out->pBuffer);

	data->action     = action;
	data->unique_id  = item.id;
	data->percentage = item.progression;
	data->activated  = item.activated;

	QueuePacket(out.get());
}

void Client::ProcessEvolvingItem(uint64 exp, Mob* mob)
{
	for (auto& [key, value] : GetEvolvingItems()) {
		if (value.equiped) {
			if (!evolving_items_manager.GetEvolvingItemsCache().contains(value.item_id)) {
				return;
			}

			auto type     = evolving_items_manager.GetEvolvingItemsCache().at(value.item_id).type;
			auto sub_type = evolving_items_manager.GetEvolvingItemsCache().at(value.item_id).sub_type;
			switch (type) {
				case EvolvingItems::Types::AMOUNT_OF_EXP: {
					std::unique_ptr<EQ::ItemInstance> const inst(database.CreateItem(value.item_id));
					if (!inst) {
						return;
					}

					if (sub_type == EvolvingItems::SubTypes::ALL_EXP ||
						(sub_type == EvolvingItems::SubTypes::GROUP_EXP && IsGrouped())
					) {
						value.current_amount += exp * RuleR(EvolvingItems, PercentOfGroupExperience) / 100;
					}
					else if (sub_type == EvolvingItems::SubTypes::ALL_EXP ||
						(sub_type == EvolvingItems::SubTypes::RAID_EXP && IsRaidGrouped())
					) {
						value.current_amount += exp * RuleR(EvolvingItems, PercentOfRaidExperience) / 100;
					}
					else if(sub_type == EvolvingItems::SubTypes::ALL_EXP ||
						sub_type == EvolvingItems::SubTypes::SOLO_EXP
					) {
						value.current_amount += exp * RuleR(EvolvingItems, PercentOfSoloExperience) / 100;;
					}

					value.progression = evolving_items_manager.CalculateProgression(value.current_amount, value.item_id);
					inst->SetEvolveProgression(value.progression);

					CharacterEvolvingItemsRepository::ReplaceOne(database, value);
					SendEvolvingPacket(EvolvingItems::Actions::UPDATE_ITEMS, value);

					LogInfo(
						"ProcessEvolvingItem: - Type 1 Amount of EXP - SubType ({}) for item {}  Assigned {} of exp to {}",
						sub_type,
						inst->GetItem()->Name,
						exp * 0.001,
						GetName()
					);
					break;
				}
				case EvolvingItems::Types::NUMBER_OF_KILLS: {
					std::unique_ptr<EQ::ItemInstance> const inst(database.CreateItem(value.item_id));
					if (!inst) {
						return;
					}

					if (mob && mob->GetRace() == sub_type) {
						value.current_amount += 1;
//						value.progression                       = inst->GetEvolvingInfo()->CalcEvolvingProgression(value.item_id);
						value.progression                       = evolving_items_manager.CalculateProgression(value.current_amount, value.item_id);
						inst->SetEvolveProgression(value.progression);

						CharacterEvolvingItemsRepository::ReplaceOne(database, value);
						SendEvolvingPacket(EvolvingItems::Actions::UPDATE_ITEMS, value);

						LogInfo(
							"ProcessEvolvingItem: - Type 2 Number of Kills - SubType ({}) for item {}  Increased count by 1 for {}",
							sub_type,
							inst->GetItem()->Name,
							GetName()
							);
					}

					break;
				}
				default: {

				}
			}
		}
	}
}