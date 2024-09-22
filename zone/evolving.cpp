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
	auto out  = std::make_unique<EQApplicationPacket>(EmuOpcode::OP_EvolveItem, sizeof(EvolveItemToggle_Struct));
	auto data = reinterpret_cast<EvolveItemToggle_Struct *>(out->pBuffer);

	data->action     = action;
	data->unique_id  = item.id;
	data->percentage = item.progression;
	data->activated  = item.activated;

	QueuePacket(out.get());
}

void Client::ProcessEvolvingItem(const uint64 exp, const Mob *mob)
{
	std::vector<EQ::ItemInstance*> queue{};

	for (auto &[key, inst]: GetInv().GetWorn()) {
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

				inst->SetEvolveProgression2();

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

				if (inst->GetEvolveProgression() >= 100) {
					queue.push_back(inst);
				}

				break;
			}
			case EvolvingItems::Types::NUMBER_OF_KILLS: {
				if (mob && mob->GetRace() == sub_type) {
					inst->SetEvolveAddToCurrentAmount(1);
					inst->SetEvolveProgression2();

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

				if (inst->GetEvolveProgression() >= 100) {
					queue.push_back(inst);
				}

				break;
			}
			default: {}
		}
	}

	if (!queue.empty()) {
		for (auto const& i:queue) {
			DoEvolveCheckProgression(*i);
		}
	}
}

void Client::DoEvolveItemDisplayFinalResult(const EQApplicationPacket* app)
{
	const auto in = reinterpret_cast<EvolveItemToggle_Struct*>(app->pBuffer);

	const uint32 item_id = static_cast<uint32>(in->unique_id & 0xFFFFFFFF);
	std::unique_ptr<EQ::ItemInstance> const inst(database.CreateItem(item_id));

	inst->SetEvolveProgression(100);

	if (inst) {
		SendItemPacket(0, inst.get(), ItemPacketViewLink);
	}
}

bool Client::DoEvolveCheckProgression(const EQ::ItemInstance &inst)
{
	if (inst.GetEvolveProgression() < 100 || inst.GetEvolveLvl() == inst.GetMaxEvolveLvl()) {
		return false;
	}

	const auto new_item_id = evolving_items_manager.GetNextEvolveItemID(inst);
	if (!new_item_id) {
		return false;
	}

	std::unique_ptr<EQ::ItemInstance> const new_inst(database.CreateItem(new_item_id));

	RemoveItemBySerialNumber(inst.GetSerialNumber());
	PushItemOnCursor(*new_inst, true);

	MessageString(Chat::Yellow, EVOLVE_ITEM_EVOLVED, inst.GetItem()->Name);
	return true;
}

void Client::SendEvolveXPTransferWindow()
{
	auto out = std::make_unique<EQApplicationPacket>(OP_EvolveItem, sizeof(EvolveItemToggle_Struct));
	const auto data = reinterpret_cast<EvolveItemToggle_Struct*>(out->pBuffer);

	data->action = 1;

	QueuePacket(out.get());
}

void Client::SendEvolveXPWindowDetails(const EQApplicationPacket *app)
{
	const auto in = reinterpret_cast<EvolveXPWindowReceive_Struct*>(app->pBuffer);

	const auto item_1_slot = GetInv().HasEvolvingItem(in->item1_unique_id, 1, invWherePersonal | invWhereWorn | invWhereCursor);
	const auto item_2_slot = GetInv().HasEvolvingItem(in->item2_unique_id, 1, invWherePersonal | invWhereWorn | invWhereCursor);

	if (item_1_slot == INVALID_INDEX || item_2_slot == INVALID_INDEX) {
		return;
	}

	const auto inst_from = GetInv().GetItem(item_1_slot);
	const auto inst_to = GetInv().GetItem(item_2_slot);

	if (!inst_from || !inst_to) {
		return;
	}

	const auto results = evolving_items_manager.DetermineTransferResults(*inst_from, *inst_to);

	if (!results.item_from_id || !results.item_to_id) {
		return;
	}

	std::unique_ptr<EQ::ItemInstance> const inst_from_new (database.CreateItem(results.item_from_id));
	std::unique_ptr<EQ::ItemInstance> const inst_to_new (database.CreateItem(results.item_to_id));
	if (!inst_from_new || !inst_to_new) {
		return;
	}

	inst_from_new->SetEvolveCurrentAmount(results.item_from_current_amount);
	inst_from_new->SetEvolveProgression2();
	inst_to_new->SetEvolveCurrentAmount(results.item_to_current_amount);
	inst_to_new->SetEvolveProgression2();

	std::stringstream           ss;
	cereal::BinaryOutputArchive ar(ss);

	EvolveXPWindowSend_Struct e{};
	e.action             = EvolvingItems::Actions::TRANSFER_WINDOW_DETAILS;
	e.compatibility      = results.compatibility;
	e.item1_unique_id    = inst_from->GetEvolveUniqueID();
	e.item2_unique_id    = inst_to->GetEvolveUniqueID();
	e.max_transfer_level = results.max_transfer_level;
	e.unknown_028        = 1;
	e.unknown_029        = 1;
	e.serialize_item_1   = inst_from_new->Serialize(0);
	e.serialize_item_2   = inst_to_new->Serialize(0);

	{
		ar(e);
	}

	uint32 packet_size = sizeof(EvolveItemMessaging_Struct) + ss.str().length();

	std::unique_ptr<EQApplicationPacket> out(new EQApplicationPacket(OP_EvolveItem, packet_size));
	const auto data    = reinterpret_cast<EvolveItemMessaging_Struct *>(out->pBuffer);

	data->action = EvolvingItems::Actions::TRANSFER_WINDOW_DETAILS;
	memcpy(data->serialized_data, ss.str().data(), ss.str().length());

	QueuePacket(out.get());

	ss.str("");
	ss.clear();
}

void Client::DoEvolveTransferXP(const EQApplicationPacket* app)
{
	const auto in = reinterpret_cast<EvolveXPWindowReceive_Struct*>(app->pBuffer);

	const auto item_1_slot = GetInv().HasEvolvingItem(in->item1_unique_id, 1, invWherePersonal | invWhereWorn | invWhereCursor);
	const auto item_2_slot = GetInv().HasEvolvingItem(in->item2_unique_id, 1, invWherePersonal | invWhereWorn | invWhereCursor);

	if (item_1_slot == INVALID_INDEX || item_2_slot == INVALID_INDEX) {
		return;
	}

	const auto inst_from = GetInv().GetItem(item_1_slot);
	const auto inst_to = GetInv().GetItem(item_2_slot);

	if (!inst_from || !inst_to) {
		return;
	}

	const auto results = evolving_items_manager.DetermineTransferResults(*inst_from, *inst_to);

	if (!results.item_from_id || !results.item_to_id) {
		return;
	}

	std::unique_ptr<const EQ::ItemInstance> const inst_from_new(database.CreateItem(results.item_from_id));
	std::unique_ptr<const EQ::ItemInstance> const inst_to_new(database.CreateItem(results.item_to_id));

	if (!inst_from_new || !inst_to_new) {
		return;
	}

	inst_from_new->SetEvolveCurrentAmount(results.item_from_current_amount);
	inst_from_new->SetEvolveProgression2();
	inst_to_new->SetEvolveCurrentAmount(results.item_to_current_amount);
	inst_to_new->SetEvolveProgression2();

	RemoveItemBySerialNumber(inst_from->GetSerialNumber());
	PushItemOnCursor(*inst_from_new, true);

	RemoveItemBySerialNumber(inst_to->GetSerialNumber());
	PushItemOnCursor(*inst_to_new, true);
}