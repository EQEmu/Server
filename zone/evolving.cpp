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
	auto in   = reinterpret_cast<EvolveItemToggle_Struct *>(app->pBuffer);
	auto item = CharacterEvolvingItemsRepository::FindOne(database, in->unique_id);

	if (!item.id) {
		return;
	}

	item.activated = in->activated;

	auto inst = GetInv().GetItem(GetInv().HasItem(item.item_id));
	inst->SetEvolveActivated(item.activated ? true : false);

	// update db
	CharacterEvolvingItemsRepository::ReplaceOne(database, item);

	// send update to client
	SendEvolvingPacket(EvolvingItems::Actions::UPDATE_ITEMS, item);
}

void Client::SendEvolvingPacket(int8 action, CharacterEvolvingItemsRepository::CharacterEvolvingItems item)
{
	auto out  = std::make_unique<EQApplicationPacket>(OP_EvolveItem, sizeof(EvolveItemToggle_Struct));
	auto data = reinterpret_cast<EvolveItemToggle_Struct *>(out->pBuffer);

	data->action     = action;
	data->unique_id  = item.id;
	data->percentage = item.progression;
	data->activated  = item.activated;

	QueuePacket(out.get());
}

void Client::ProcessEvolvingItem(const uint64 exp, const Mob *mob)
{
	for (auto const &[key, inst]: GetInv().GetWorn()) {
		if (!inst->IsEvolving() || !inst->GetEvolveActivated()) {
			continue;
		}

		auto type     = evolving_items_manager.GetEvolvingItemsCache().at(inst->GetID()).type;
		auto sub_type = evolving_items_manager.GetEvolvingItemsCache().at(inst->GetID()).sub_type;
		switch (type) {
			case EvolvingItems::Types::AMOUNT_OF_EXP: {
				if (sub_type == EvolvingItems::SubTypes::ALL_EXP ||
				    (sub_type == EvolvingItems::SubTypes::GROUP_EXP && IsGrouped())
				) {
					inst->SetEvolveAddToCurrentAmount(exp * RuleR(EvolvingItems, PercentOfGroupExperience) / 100);
				}
				else if (sub_type == EvolvingItems::SubTypes::ALL_EXP ||
				         (sub_type == EvolvingItems::SubTypes::RAID_EXP && IsRaidGrouped())
				) {
					inst->SetEvolveAddToCurrentAmount(exp * RuleR(EvolvingItems, PercentOfRaidExperience) / 100);
				}
				else if (sub_type == EvolvingItems::SubTypes::ALL_EXP ||
				         sub_type == EvolvingItems::SubTypes::SOLO_EXP
				) {
					inst->SetEvolveAddToCurrentAmount(exp * RuleR(EvolvingItems, PercentOfSoloExperience) / 100);
				}

				inst->SetEvolveProgression(
					evolving_items_manager.CalculateProgression(inst->GetEvolveCurrentAmount(), inst->GetID()));

				auto e = CharacterEvolvingItemsRepository::SetCurrentAmountAndProgression(database, inst->GetEvolveUniqueID(), inst->GetEvolveCurrentAmount(), inst->GetEvolveProgression());
				if (!e.id) {
					break;
				}

				SendEvolvingPacket(EvolvingItems::Actions::UPDATE_ITEMS, e);

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
				if (mob && mob->GetRace() == sub_type) {
					inst->SetEvolveAddToCurrentAmount(1);
					inst->SetEvolveProgression(
						evolving_items_manager.CalculateProgression(inst->GetEvolveCurrentAmount(), inst->GetID()));

					auto e = CharacterEvolvingItemsRepository::SetCurrentAmountAndProgression(database, inst->GetEvolveUniqueID(), inst->GetEvolveCurrentAmount(), inst->GetEvolveProgression());
					if (!e.id) {
						break;
					}

					SendEvolvingPacket(EvolvingItems::Actions::UPDATE_ITEMS, e);

					LogInfo(
						"ProcessEvolvingItem: - Type 2 Number of Kills - SubType ({}) for item {}  Increased count by 1 for {}",
						sub_type,
						inst->GetItem()->Name,
						GetName()
						);
				}

				break;
			}
			default: {}
		}
	}
}