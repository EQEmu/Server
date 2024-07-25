/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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

#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"
#include "../common/rulesys.h"
#include "../common/strings.h"
#include "../common/eq_packet_structs.h"
#include "../common/misc_functions.h"
#include "../common/events/player_event_logs.h"
#include "../common/repositories/trader_repository.h"
#include "../common/repositories/buyer_repository.h"
#include "../common/repositories/buyer_buy_lines_repository.h"

#include "client.h"
#include "entity.h"
#include "mob.h"

#include "quest_parser_collection.h"
#include "string_ids.h"
#include "worldserver.h"
#include "../common/bazaar.h"
#include <numeric>

class QueryServ;

extern WorldServer worldserver;
extern QueryServ* QServ;

// The maximum amount of a single bazaar/barter transaction expressed in copper.
// Equivalent to 2 Million plat
constexpr auto MAX_TRANSACTION_VALUE = 2000000000;
// ##########################################
// Trade implementation
// ##########################################

Trade::Trade(Mob* in_owner)
{
	owner = in_owner;
	Reset();
}

Trade::~Trade()
{
	Reset();
}

void Trade::Reset()
{
	state = TradeNone;
	with_id = 0;
	pp=0; gp=0; sp=0; cp=0;
}

// Initiate a trade with another mob
// initiate_with specifies whether to start trade with other mob as well
void Trade::Start(uint32 mob_id, bool initiate_with)
{
	Reset();
	state = Trading;
	with_id = mob_id;

	// Autostart on other mob?
	if (initiate_with) {
		Mob *with = With();
		if (with) {
			with->trade->Start(owner->GetID(), false);
		}
	}
}

// Add item from a given slot to trade bucket (automatically does bag data too)
void Trade::AddEntity(uint16 trade_slot_id, uint32 stack_size) {
	// TODO: review for inventory saves / consider changing return type to bool so failure can be passed to desync handler

	if (!owner || !owner->IsClient()) {
		// This should never happen
		LogDebug("Programming error: NPC's should not call Trade::AddEntity()");
		return;
	}

	// If one party accepted the trade then an item was added, their state needs to be reset
	owner->trade->state = Trading;
	Mob* with = With();
	if (with)
		with->trade->state = Trading;

	// Item always goes into trade bucket from cursor
	Client* client = owner->CastToClient();
	EQ::ItemInstance* inst = client->GetInv().GetItem(EQ::invslot::slotCursor);

	if (!inst) {
		client->Message(Chat::Red, "Error: Could not find item on your cursor!");
		return;
	}

	EQ::ItemInstance* inst2 = client->GetInv().GetItem(trade_slot_id);

	// it looks like the original code attempted to allow stacking...
	// (it just didn't handle partial stack move actions)
	if (stack_size > 0) {
		if (!inst->IsStackable() || !inst2 || !inst2->GetItem() || (inst->GetID() != inst2->GetID()) || (stack_size > inst->GetCharges())) {
			client->Kick("Error stacking item in trade");
			return;
		}

		uint32 _stack_size = 0;

		if ((stack_size + inst2->GetCharges()) > inst2->GetItem()->StackSize) {
			_stack_size = (stack_size + inst2->GetCharges()) - inst->GetItem()->StackSize;
			inst2->SetCharges(inst2->GetItem()->StackSize);
		}
		else {
			_stack_size = inst->GetCharges() - stack_size;
			inst2->SetCharges(stack_size + inst2->GetCharges());
		}

		LogTrading("[{}] added partial item [{}] stack (qty: [{}]) to trade slot [{}]", owner->GetName(), inst->GetItem()->Name, stack_size, trade_slot_id);

		if (_stack_size > 0)
			inst->SetCharges(_stack_size);
		else
			client->DeleteItemInInventory(EQ::invslot::slotCursor);

		SendItemData(inst2, trade_slot_id);
	}
	else {
		if (inst2 && inst2->GetID()) {
			client->Kick("Attempting to add null item to trade");
			return;
		}

		SendItemData(inst, trade_slot_id);

		LogTrading("[{}] added item [{}] to trade slot [{}]", owner->GetName(), inst->GetItem()->Name, trade_slot_id);

		client->PutItemInInventory(trade_slot_id, *inst);
		client->DeleteItemInInventory(EQ::invslot::slotCursor);
	}
}

// Retrieve mob the owner is trading with
// Done like this in case 'with' mob goes LD and Mob* becomes invalid
Mob* Trade::With()
{
	return entity_list.GetMob(with_id);
}

// Private Method: Send item data for trade item to other person involved in trade
void Trade::SendItemData(const EQ::ItemInstance* inst, int16 dest_slot_id)
{
	if (inst == nullptr)
		return;

	// @merth: This needs to be redone with new item classes
	Mob* mob = With();
	if (!mob->IsClient())
		return; // Not sending packets to NPCs!

	Client* with = mob->CastToClient();
	Client* trader = owner->CastToClient();
	if (with && with->IsClient()) {
		with->SendItemPacket(dest_slot_id - EQ::invslot::TRADE_BEGIN, inst, ItemPacketTradeView);
		if (inst->GetItem()->ItemClass == 1) {
			for (uint16 i = EQ::invbag::SLOT_BEGIN; i <= EQ::invbag::SLOT_END; i++) {
				uint16 bagslot_id = EQ::InventoryProfile::CalcSlotId(dest_slot_id, i);
				const EQ::ItemInstance* bagitem = trader->GetInv().GetItem(bagslot_id);
				if (bagitem) {
					with->SendItemPacket(bagslot_id - EQ::invslot::TRADE_BEGIN, bagitem, ItemPacketTradeView);
				}
			}
		}

		//safe_delete(outapp);
	}
}

Mob *Trade::GetOwner() const
{
	return owner;
}


void Client::ResetTrade() {
	AddMoneyToPP(trade->cp, trade->sp, trade->gp, trade->pp, true);

	// step 1: process bags
	for (int16 trade_slot = EQ::invslot::TRADE_BEGIN; trade_slot <= EQ::invslot::TRADE_END; ++trade_slot) {
		const EQ::ItemInstance* inst = m_inv[trade_slot];

		if (inst && inst->IsClassBag()) {
			int16 free_slot = m_inv.FindFreeSlotForTradeItem(inst);

			if (free_slot != INVALID_INDEX) {
				PutItemInInventory(free_slot, *inst);
				SendItemPacket(free_slot, inst, ItemPacketTrade);
			}
			else {
				DropInst(inst);
			}

			DeleteItemInInventory(trade_slot);
		}
	}

	// step 2a: process stackables
	for (int16 trade_slot = EQ::invslot::TRADE_BEGIN; trade_slot <= EQ::invslot::TRADE_END; ++trade_slot) {
		EQ::ItemInstance* inst = GetInv().GetItem(trade_slot);

		if (inst && inst->IsStackable()) {
			while (true) {
				// there's no built-in safety check against an infinite loop..but, it should break on one of the conditional checks
				int16 free_slot = m_inv.FindFreeSlotForTradeItem(inst);

				if ((free_slot == EQ::invslot::slotCursor) || (free_slot == INVALID_INDEX))
					break;

				EQ::ItemInstance* partial_inst = GetInv().GetItem(free_slot);

				if (!partial_inst)
					break;

				if (partial_inst->GetID() != inst->GetID()) {
					LogDebug("[CLIENT] Client::ResetTrade() - an incompatible location reference was returned by Inventory::FindFreeSlotForTradeItem()");

					break;
				}

				if ((partial_inst->GetCharges() + inst->GetCharges()) > partial_inst->GetItem()->StackSize) {
					int16 new_charges = (partial_inst->GetCharges() + inst->GetCharges()) - partial_inst->GetItem()->StackSize;

					partial_inst->SetCharges(partial_inst->GetItem()->StackSize);
					inst->SetCharges(new_charges);
				}
				else {
					partial_inst->SetCharges(partial_inst->GetCharges() + inst->GetCharges());
					inst->SetCharges(0);
				}

				PutItemInInventory(free_slot, *partial_inst);
				SendItemPacket(free_slot, partial_inst, ItemPacketTrade);

				if (inst->GetCharges() == 0) {
					DeleteItemInInventory(trade_slot);

					break;
				}
			}
		}
	}

	// step 2b: adjust trade stack bias
	// (if any partial stacks exist before the final stack, FindFreeSlotForTradeItem() will return that slot in step 3 and an overwrite will occur)
	for (int16 trade_slot = EQ::invslot::TRADE_END; trade_slot >= EQ::invslot::TRADE_BEGIN; --trade_slot) {
		EQ::ItemInstance* inst = GetInv().GetItem(trade_slot);

		if (inst && inst->IsStackable()) {
			for (int16 bias_slot = EQ::invslot::TRADE_BEGIN; bias_slot <= EQ::invslot::TRADE_END; ++bias_slot) {
				if (bias_slot >= trade_slot)
					break;

				EQ::ItemInstance* bias_inst = GetInv().GetItem(bias_slot);

				if (!bias_inst || (bias_inst->GetID() != inst->GetID()) || (bias_inst->GetCharges() >= bias_inst->GetItem()->StackSize))
					continue;

				if ((bias_inst->GetCharges() + inst->GetCharges()) > bias_inst->GetItem()->StackSize) {
					int16 new_charges = (bias_inst->GetCharges() + inst->GetCharges()) - bias_inst->GetItem()->StackSize;

					bias_inst->SetCharges(bias_inst->GetItem()->StackSize);
					inst->SetCharges(new_charges);
				}
				else {
					bias_inst->SetCharges(bias_inst->GetCharges() + inst->GetCharges());
					inst->SetCharges(0);
				}

				if (inst->GetCharges() == 0) {
					DeleteItemInInventory(trade_slot);

					break;
				}
			}
		}
	}

	// step 3: process everything else
	for (int16 trade_slot = EQ::invslot::TRADE_BEGIN; trade_slot <= EQ::invslot::TRADE_END; ++trade_slot) {
		const EQ::ItemInstance* inst = m_inv[trade_slot];

		if (inst) {
			int16 free_slot = m_inv.FindFreeSlotForTradeItem(inst);

			if (free_slot != INVALID_INDEX) {
				PutItemInInventory(free_slot, *inst);
				SendItemPacket(free_slot, inst, ItemPacketTrade);
			}
			else {
				DropInst(inst);
			}

			DeleteItemInInventory(trade_slot);
		}
	}
}

void Client::FinishTrade(Mob* tradingWith, bool finalizer, void* event_entry, std::list<void*>* event_details) {
	if(tradingWith && tradingWith->IsClient()) {
		Client                * other    = tradingWith->CastToClient();
		PlayerLogTrade_Struct * qs_audit = nullptr;
		bool qs_log = false;

		if(other) {
			LogTrading("Finishing trade with client [{}]", other->GetName());

			AddMoneyToPP(other->trade->cp, other->trade->sp, other->trade->gp, other->trade->pp, true);

			// step 0: pre-processing
			// QS code
			if (RuleB(QueryServ, PlayerLogTrades) && event_entry && event_details) {
				qs_audit = (PlayerLogTrade_Struct*)event_entry;
				qs_log = true;

				if (finalizer) {
					qs_audit->character_2_id = character_id;

					qs_audit->character_2_money.platinum = trade->pp;
					qs_audit->character_2_money.gold     = trade->gp;
					qs_audit->character_2_money.silver   = trade->sp;
					qs_audit->character_2_money.copper   = trade->cp;
				}
				else {
					qs_audit->character_1_id = character_id;

					qs_audit->character_1_money.platinum = trade->pp;
					qs_audit->character_1_money.gold     = trade->gp;
					qs_audit->character_1_money.silver   = trade->sp;
					qs_audit->character_1_money.copper   = trade->cp;
				}
			}

			// step 1: process bags
			for (int16 trade_slot = EQ::invslot::TRADE_BEGIN; trade_slot <= EQ::invslot::TRADE_END; ++trade_slot) {
				const EQ::ItemInstance* inst = m_inv[trade_slot];

				if (inst && inst->IsClassBag()) {
					LogTrading("Giving container [{}] ([{}]) in slot [{}] to [{}]", inst->GetItem()->Name, inst->GetItem()->ID, trade_slot, other->GetName());

					// TODO: need to check bag items/augments for no drop..everything for attuned...
					if (
						inst->GetItem()->NoDrop != 0 ||
						CanTradeFVNoDropItem() ||
						other == this
					) {
						int16 free_slot = other->GetInv().FindFreeSlotForTradeItem(inst);

						if (free_slot != INVALID_INDEX) {
							if (other->PutItemInInventory(free_slot, *inst, true)) {
								LogTrading("Container [{}] ([{}]) successfully transferred, deleting from trade slot", inst->GetItem()->Name, inst->GetItem()->ID);
								if (qs_log) {
									auto detail = new PlayerLogTradeItemsEntry_Struct;

									detail->from_character_id = character_id;
									detail->from_slot       = trade_slot;
									detail->to_character_id = other->CharacterID();
									detail->to_slot         = free_slot;
									detail->item_id = inst->GetID();
									detail->charges = 1;
									detail->aug_1 = inst->GetAugmentItemID(1);
									detail->aug_2 = inst->GetAugmentItemID(2);
									detail->aug_3 = inst->GetAugmentItemID(3);
									detail->aug_4 = inst->GetAugmentItemID(4);
									detail->aug_5 = inst->GetAugmentItemID(5);

									event_details->push_back(detail);

									if (finalizer)
										qs_audit->character_2_item_count += detail->charges;
									else
										qs_audit->character_1_item_count += detail->charges;

									for (uint8 sub_slot = EQ::invbag::SLOT_BEGIN; (sub_slot <= EQ::invbag::SLOT_END); ++sub_slot) { // this is to catch ALL items
										const EQ::ItemInstance* bag_inst = inst->GetItem(sub_slot);

										if (bag_inst) {
											detail = new PlayerLogTradeItemsEntry_Struct;

											detail->from_character_id = character_id;
											detail->from_slot       = EQ::InventoryProfile::CalcSlotId(trade_slot, sub_slot);
											detail->to_character_id = other->CharacterID();
											detail->to_slot         = EQ::InventoryProfile::CalcSlotId(free_slot, sub_slot);
											detail->item_id = bag_inst->GetID();
											detail->charges = (!bag_inst->IsStackable() ? 1 : bag_inst->GetCharges());
											detail->aug_1 = bag_inst->GetAugmentItemID(1);
											detail->aug_2 = bag_inst->GetAugmentItemID(2);
											detail->aug_3 = bag_inst->GetAugmentItemID(3);
											detail->aug_4 = bag_inst->GetAugmentItemID(4);
											detail->aug_5 = bag_inst->GetAugmentItemID(5);

											event_details->push_back(detail);

											if (finalizer)
												qs_audit->character_2_item_count += detail->charges;
											else
												qs_audit->character_1_item_count += detail->charges;
										}
									}
								}
							}
							else {
								LogTrading("Transfer of container [{}] ([{}]) to [{}] failed, returning to giver", inst->GetItem()->Name, inst->GetItem()->ID, other->GetName());
								PushItemOnCursor(*inst, true);
							}
						}
						else {
							LogTrading("[{}]'s inventory is full, returning container [{}] ([{}]) to giver", other->GetName(), inst->GetItem()->Name, inst->GetItem()->ID);
							PushItemOnCursor(*inst, true);
						}
					}
					else {
						LogTrading("Container [{}] ([{}]) is NoDrop, returning to giver", inst->GetItem()->Name, inst->GetItem()->ID);
						PushItemOnCursor(*inst, true);
					}

					DeleteItemInInventory(trade_slot);
				}
			}

			// step 2a: process stackables
			for (int16 trade_slot = EQ::invslot::TRADE_BEGIN; trade_slot <= EQ::invslot::TRADE_END; ++trade_slot) {
				EQ::ItemInstance* inst = GetInv().GetItem(trade_slot);

				if (inst && inst->IsStackable()) {
					while (true) {
						// there's no built-in safety check against an infinite loop..but, it should break on one of the conditional checks
						int16 partial_slot = other->GetInv().FindFreeSlotForTradeItem(inst);

						if ((partial_slot == EQ::invslot::slotCursor) || (partial_slot == INVALID_INDEX))
							break;

						EQ::ItemInstance* partial_inst = other->GetInv().GetItem(partial_slot);

						if (!partial_inst)
							break;

						if (partial_inst->GetID() != inst->GetID()) {
							LogTrading("[CLIENT] Client::ResetTrade() - an incompatible location reference was returned by Inventory::FindFreeSlotForTradeItem()");
							break;
						}

						int16 old_charges = inst->GetCharges();
						int16 partial_charges = partial_inst->GetCharges();

						if ((partial_inst->GetCharges() + inst->GetCharges()) > partial_inst->GetItem()->StackSize) {
							int16 new_charges = (partial_inst->GetCharges() + inst->GetCharges()) - partial_inst->GetItem()->StackSize;

							partial_inst->SetCharges(partial_inst->GetItem()->StackSize);
							inst->SetCharges(new_charges);
						}
						else {
							partial_inst->SetCharges(partial_inst->GetCharges() + inst->GetCharges());
							inst->SetCharges(0);
						}

						LogTrading("Transferring partial stack [{}] ([{}]) in slot [{}] to [{}]", inst->GetItem()->Name, inst->GetItem()->ID, trade_slot, other->GetName());

						if (other->PutItemInInventory(partial_slot, *partial_inst, true)) {
							LogTrading("Partial stack [{}] ([{}]) successfully transferred, deleting [{}] charges from trade slot",
								inst->GetItem()->Name, inst->GetItem()->ID, (old_charges - inst->GetCharges()));
							if (qs_log) {
								auto detail = new PlayerLogTradeItemsEntry_Struct;

								detail->from_character_id = character_id;
								detail->from_slot       = trade_slot;
								detail->to_character_id = other->CharacterID();
								detail->to_slot         = partial_slot;
								detail->item_id = inst->GetID();
								detail->charges = (old_charges - inst->GetCharges());
								detail->aug_1 = 0;
								detail->aug_2 = 0;
								detail->aug_3 = 0;
								detail->aug_4 = 0;
								detail->aug_5 = 0;

								event_details->push_back(detail);

								if (finalizer)
									qs_audit->character_2_item_count += detail->charges;
								else
									qs_audit->character_1_item_count += detail->charges;
							}
						}
						else {
							LogTrading("Transfer of partial stack [{}] ([{}]) to [{}] failed, returning [{}] charges to trade slot",
								inst->GetItem()->Name, inst->GetItem()->ID, other->GetName(), (old_charges - inst->GetCharges()));

							inst->SetCharges(old_charges);
							partial_inst->SetCharges(partial_charges);
							break;
						}

						if (inst->GetCharges() == 0) {
							DeleteItemInInventory(trade_slot);
							break;
						}
					}
				}
			}

			// step 2b: adjust trade stack bias
			// (if any partial stacks exist before the final stack, FindFreeSlotForTradeItem() will return that slot in step 3 and an overwrite will occur)
			for (int16 trade_slot = EQ::invslot::TRADE_END; trade_slot >= EQ::invslot::TRADE_BEGIN; --trade_slot) {
				EQ::ItemInstance* inst = GetInv().GetItem(trade_slot);

				if (inst && inst->IsStackable()) {
					for (int16 bias_slot = EQ::invslot::TRADE_BEGIN; bias_slot <= EQ::invslot::TRADE_END; ++bias_slot) {
						if (bias_slot >= trade_slot)
							break;

						EQ::ItemInstance* bias_inst = GetInv().GetItem(bias_slot);

						if (!bias_inst || (bias_inst->GetID() != inst->GetID()) || (bias_inst->GetCharges() >= bias_inst->GetItem()->StackSize))
							continue;

						int16 old_charges = inst->GetCharges();

						if ((bias_inst->GetCharges() + inst->GetCharges()) > bias_inst->GetItem()->StackSize) {
							int16 new_charges = (bias_inst->GetCharges() + inst->GetCharges()) - bias_inst->GetItem()->StackSize;

							bias_inst->SetCharges(bias_inst->GetItem()->StackSize);
							inst->SetCharges(new_charges);
						}
						else {
							bias_inst->SetCharges(bias_inst->GetCharges() + inst->GetCharges());
							inst->SetCharges(0);
						}

						if (qs_log) {
							auto detail = new PlayerLogTradeItemsEntry_Struct;

							detail->from_character_id = character_id;
							detail->from_slot       = trade_slot;
							detail->to_character_id = character_id;
							detail->to_slot         = bias_slot;
							detail->item_id = inst->GetID();
							detail->charges = (old_charges - inst->GetCharges());
							detail->aug_1 = 0;
							detail->aug_2 = 0;
							detail->aug_3 = 0;
							detail->aug_4 = 0;
							detail->aug_5 = 0;

							event_details->push_back(detail);
						}

						if (inst->GetCharges() == 0) {
							DeleteItemInInventory(trade_slot);
							break;
						}
					}
				}
			}

			// step 3: process everything else
			for (int16 trade_slot = EQ::invslot::TRADE_BEGIN; trade_slot <= EQ::invslot::TRADE_END; ++trade_slot) {
				const EQ::ItemInstance* inst = m_inv[trade_slot];

				if (inst) {
					LogTrading("Giving item [{}] ([{}]) in slot [{}] to [{}]", inst->GetItem()->Name, inst->GetItem()->ID, trade_slot, other->GetName());

					// TODO: need to check bag items/augments for no drop..everything for attuned...
					if (inst->GetItem()->NoDrop != 0 || CanTradeFVNoDropItem() || other == this) {
						int16 free_slot = other->GetInv().FindFreeSlotForTradeItem(inst);

						if (free_slot != INVALID_INDEX) {
							if (other->PutItemInInventory(free_slot, *inst, true)) {
								LogTrading("Item [{}] ([{}]) successfully transferred, deleting from trade slot", inst->GetItem()->Name, inst->GetItem()->ID);
								if (qs_log) {
									auto detail = new PlayerLogTradeItemsEntry_Struct;

									detail->from_character_id = character_id;
									detail->from_slot       = trade_slot;
									detail->to_character_id = other->CharacterID();
									detail->to_slot         = free_slot;
									detail->item_id = inst->GetID();
									detail->charges = (!inst->IsStackable() ? 1 : inst->GetCharges());
									detail->aug_1 = inst->GetAugmentItemID(1);
									detail->aug_2 = inst->GetAugmentItemID(2);
									detail->aug_3 = inst->GetAugmentItemID(3);
									detail->aug_4 = inst->GetAugmentItemID(4);
									detail->aug_5 = inst->GetAugmentItemID(5);

									event_details->push_back(detail);

									if (finalizer)
										qs_audit->character_2_item_count += detail->charges;
									else
										qs_audit->character_1_item_count += detail->charges;

									// 'step 3' should never really see containers..but, just in case...
									for (uint8 sub_slot = EQ::invbag::SLOT_BEGIN; (sub_slot <= EQ::invbag::SLOT_END); ++sub_slot) { // this is to catch ALL items
										const EQ::ItemInstance* bag_inst = inst->GetItem(sub_slot);

										if (bag_inst) {
											detail = new PlayerLogTradeItemsEntry_Struct;

											detail->from_character_id = character_id;
											detail->from_slot       = trade_slot;
											detail->to_character_id = other->CharacterID();
											detail->to_slot         = free_slot;
											detail->item_id = bag_inst->GetID();
											detail->charges = (!bag_inst->IsStackable() ? 1 : bag_inst->GetCharges());
											detail->aug_1 = bag_inst->GetAugmentItemID(1);
											detail->aug_2 = bag_inst->GetAugmentItemID(2);
											detail->aug_3 = bag_inst->GetAugmentItemID(3);
											detail->aug_4 = bag_inst->GetAugmentItemID(4);
											detail->aug_5 = bag_inst->GetAugmentItemID(5);

											event_details->push_back(detail);

											if (finalizer)
												qs_audit->character_2_item_count += detail->charges;
											else
												qs_audit->character_1_item_count += detail->charges;
										}
									}
								}
							}
							else {
								LogTrading("Transfer of Item [{}] ([{}]) to [{}] failed, returning to giver", inst->GetItem()->Name, inst->GetItem()->ID, other->GetName());
								PushItemOnCursor(*inst, true);
							}
						}
						else {
							LogTrading("[{}]'s inventory is full, returning item [{}] ([{}]) to giver", other->GetName(), inst->GetItem()->Name, inst->GetItem()->ID);
							PushItemOnCursor(*inst, true);
						}
					}
					else {
						LogTrading("Item [{}] ([{}]) is NoDrop, returning to giver", inst->GetItem()->Name, inst->GetItem()->ID);
						PushItemOnCursor(*inst, true);
					}

					DeleteItemInInventory(trade_slot);
				}
			}

			//Do not reset the trade here, done by the caller.
		}
	}
	else if(tradingWith && tradingWith->IsNPC()) {
		QSPlayerLogHandin_Struct* qs_audit = nullptr;
		bool qs_log = false;

		// QS code
		if(RuleB(QueryServ, PlayerLogTrades) && event_entry && event_details) {
			// Currently provides only basic functionality. Calling method will also
			// need to be modified before item returns and rewards can be logged.
			qs_audit = (QSPlayerLogHandin_Struct*)event_entry;
			qs_log = true;

			qs_audit->quest_id = 0;
			qs_audit->char_id = character_id;
			qs_audit->char_money.platinum = trade->pp;
			qs_audit->char_money.gold = trade->gp;
			qs_audit->char_money.silver = trade->sp;
			qs_audit->char_money.copper = trade->cp;
			qs_audit->char_count = 0;
			qs_audit->npc_id = tradingWith->GetNPCTypeID();
			qs_audit->npc_money.platinum = 0;
			qs_audit->npc_money.gold = 0;
			qs_audit->npc_money.silver = 0;
			qs_audit->npc_money.copper = 0;
			qs_audit->npc_count = 0;
		}

		if(qs_log) { // This can be incorporated below when revisions are made
			for (int16 trade_slot = EQ::invslot::TRADE_BEGIN; trade_slot <= EQ::invslot::TRADE_NPC_END; ++trade_slot) {
				const EQ::ItemInstance* trade_inst = m_inv[trade_slot];

				if(trade_inst) {
					auto detail = new QSHandinItems_Struct;

					strcpy(detail->action_type, "HANDIN");

					detail->char_slot = trade_slot;
					detail->item_id = trade_inst->GetID();
					detail->charges = (!trade_inst->IsStackable() ? 1 : trade_inst->GetCharges());
					detail->aug_1 = trade_inst->GetAugmentItemID(1);
					detail->aug_2 = trade_inst->GetAugmentItemID(2);
					detail->aug_3 = trade_inst->GetAugmentItemID(3);
					detail->aug_4 = trade_inst->GetAugmentItemID(4);
					detail->aug_5 = trade_inst->GetAugmentItemID(5);

					event_details->push_back(detail);
					qs_audit->char_count += detail->charges;

					if (trade_inst->IsClassBag()) {
						for (uint8 sub_slot = EQ::invbag::SLOT_BEGIN; sub_slot < trade_inst->GetItem()->BagSlots; ++sub_slot) {
							const EQ::ItemInstance* trade_baginst = trade_inst->GetItem(sub_slot);

							if(trade_baginst) {
								detail = new QSHandinItems_Struct;

								strcpy(detail->action_type, "HANDIN");

								detail->char_slot = EQ::InventoryProfile::CalcSlotId(trade_slot, sub_slot);
								detail->item_id = trade_baginst->GetID();
								detail->charges = (!trade_inst->IsStackable() ? 1 : trade_inst->GetCharges());
								detail->aug_1 = trade_baginst->GetAugmentItemID(1);
								detail->aug_2 = trade_baginst->GetAugmentItemID(2);
								detail->aug_3 = trade_baginst->GetAugmentItemID(3);
								detail->aug_4 = trade_baginst->GetAugmentItemID(4);
								detail->aug_5 = trade_baginst->GetAugmentItemID(5);

								event_details->push_back(detail);
								qs_audit->char_count += detail->charges;
							}
						}
					}
				}
			}
		}

		bool quest_npc = false;
		if (parse->HasQuestSub(tradingWith->GetNPCTypeID(), EVENT_TRADE)) {
			// This is a quest NPC
			quest_npc = true;
		}

		// take ownership of all trade slot items
		EQ::ItemInstance* insts[4] = { 0 };
		for (int i = EQ::invslot::TRADE_BEGIN; i <= EQ::invslot::TRADE_NPC_END; ++i) {
			insts[i - EQ::invslot::TRADE_BEGIN] = m_inv.PopItem(i);
			database.SaveInventory(CharacterID(), nullptr, i);
		}

		// copy to be filtered by task updates, null trade slots preserved for quest event arg
		std::vector<EQ::ItemInstance*> items(insts, insts + std::size(insts));

		if (RuleB(TaskSystem, EnableTaskSystem)) {
			if (UpdateTasksOnDeliver(items, *trade, tradingWith->CastToNPC())) {
				if (!tradingWith->IsMoving())
					tradingWith->FaceTarget(this);

				EVENT_ITEM_ScriptStopReturn();

			}
		}

		// Regardless of quest or non-quest NPC - No in combat trade completion
		// is allowed.
		if (tradingWith->CheckAggro(this))
		{
			for (EQ::ItemInstance* inst : items) {
				if (!inst || !inst->GetItem()) {
					continue;
				}

				tradingWith->SayString(TRADE_BACK, GetCleanName());
				PushItemOnCursor(*inst, true);
			}
		}
		// Only enforce trade rules if the NPC doesn't have an EVENT_TRADE
		// subroutine.  That overrides all.
		else if (!quest_npc)
		{
			for (EQ::ItemInstance* inst : items) {
				if (!inst || !inst->GetItem()) {
					continue;
				}

				// remove delivered task items
				if (RuleB(TaskSystem, EnableTaskSystem) && inst->GetTaskDeliveredCount() > 0) {
					int remaining = inst->RemoveTaskDeliveredItems();
					if (remaining <= 0) {
						inst = nullptr;
						continue; // all items in trade slot consumed by task update
					}
				}

				const EQ::ItemData* item = inst->GetItem();
				const bool is_pet = _CLIENTPET(tradingWith) && tradingWith->GetPetType()<=petOther;
				const bool is_quest_npc = tradingWith->CastToNPC()->IsQuestNPC();
				const bool restrict_quest_items_to_quest_npc = RuleB(NPC, ReturnQuestItemsFromNonQuestNPCs);
				const bool pets_can_take_quest_items = RuleB(Pets, CanTakeQuestItems);
				const bool is_pet_and_can_have_nodrop_items = (RuleB(Pets, CanTakeNoDrop) &&	is_pet);
				const bool is_pet_and_can_have_quest_items = (pets_can_take_quest_items &&	is_pet);
				// if it was not a NO DROP or Attuned item (or if a GM is trading), let the NPC have it
				if (GetGM() ||
					(!restrict_quest_items_to_quest_npc || (is_quest_npc && item->IsQuestItem()) || !item->IsQuestItem()) && // If rule is enabled, return any quest items given to non-quest NPCs
					(((item->NoDrop != 0 && !inst->IsAttuned()) || is_pet_and_can_have_nodrop_items) &&
					((!item->IsQuestItem() || is_pet_and_can_have_quest_items || !is_pet)))) {
					// pets need to look inside bags and try to equip items found there
					if (item->IsClassBag() && item->BagSlots > 0) {
						for (int16 bslot = EQ::invbag::SLOT_BEGIN; bslot < item->BagSlots; bslot++) {
							const EQ::ItemInstance *baginst = inst->GetItem(bslot);
							if (baginst) {
								const EQ::ItemData *bagitem = baginst->GetItem();
								if (bagitem && (GetGM() ||
									(!restrict_quest_items_to_quest_npc ||
									(is_quest_npc && bagitem->IsQuestItem()) || !bagitem->IsQuestItem()) &&
									// If rule is enabled, return any quest items given to non-quest NPCs (inside bags)
									(bagitem->NoDrop != 0 && !baginst->IsAttuned()) &&
									((is_pet && (!bagitem->IsQuestItem() || pets_can_take_quest_items) ||
									!is_pet)))) {

									if (GetGM()) {
										const std::string& item_link = database.CreateItemLink(bagitem->ID);
										Message(
											Chat::White,
											fmt::format(
												"Your GM flag allows you to give {} to {}.",
												item_link,
												GetTargetDescription(tradingWith)
											).c_str()
										);
									}

									auto loot_drop_entry = LootdropEntriesRepository::NewNpcEntity();
									loot_drop_entry.equip_item = 1;
									loot_drop_entry.item_charges = static_cast<int8>(baginst->GetCharges());

									tradingWith->CastToNPC()->AddLootDrop(
										bagitem,
										loot_drop_entry,
										true
									);
									// Return quest items being traded to non-quest NPC when the rule is true
								} else if (restrict_quest_items_to_quest_npc && (!is_quest_npc && bagitem->IsQuestItem())) {
									tradingWith->SayString(TRADE_BACK, GetCleanName());
									PushItemOnCursor(*baginst, true);
									Message(Chat::Red, "You can only trade quest items to quest NPCs.");
									// Return quest items being traded to player pet when not allowed
								} else if (is_pet && bagitem->IsQuestItem() && !pets_can_take_quest_items) {
									tradingWith->SayString(TRADE_BACK, GetCleanName());
									PushItemOnCursor(*baginst, true);
									Message(Chat::Red, "You cannot trade quest items with your pet.");
								} else if (RuleB(NPC, ReturnNonQuestNoDropItems)) {
									tradingWith->SayString(TRADE_BACK, GetCleanName());
									PushItemOnCursor(*baginst, true);
								}
							}
						}
					}

					auto new_loot_drop_entry = LootdropEntriesRepository::NewNpcEntity();
					new_loot_drop_entry.equip_item = 1;
					new_loot_drop_entry.item_charges = static_cast<int8>(inst->GetCharges());

					tradingWith->CastToNPC()->AddLootDrop(
						item,
						new_loot_drop_entry,
						true
					);
				}
				// Return quest items being traded to non-quest NPC when the rule is true
				else if (restrict_quest_items_to_quest_npc && (!is_quest_npc && item->IsQuestItem())) {
					tradingWith->SayString(TRADE_BACK, GetCleanName());
					PushItemOnCursor(*inst, true);
					Message(Chat::Red, "You can only trade quest items to quest NPCs.");
				}
				// Return quest items being traded to player pet when not allowed
				else if (is_pet && item->IsQuestItem()) {
					tradingWith->SayString(TRADE_BACK, GetCleanName());
					PushItemOnCursor(*inst, true);
					Message(Chat::Red, "You cannot trade quest items with your pet.");
				}
				// Return NO DROP and Attuned items being handed into a non-quest NPC if the rule is true
				else if (RuleB(NPC, ReturnNonQuestNoDropItems)) {
					tradingWith->SayString(TRADE_BACK, GetCleanName());
					PushItemOnCursor(*inst, true);
				}
			}
		}

		char temp1[100] = { 0 };
		char temp2[100] = { 0 };
		snprintf(temp1, 100, "copper.%d", tradingWith->GetNPCTypeID());
		snprintf(temp2, 100, "%u", trade->cp);
		parse->AddVar(temp1, temp2);
		snprintf(temp1, 100, "silver.%d", tradingWith->GetNPCTypeID());
		snprintf(temp2, 100, "%u", trade->sp);
		parse->AddVar(temp1, temp2);
		snprintf(temp1, 100, "gold.%d", tradingWith->GetNPCTypeID());
		snprintf(temp2, 100, "%u", trade->gp);
		parse->AddVar(temp1, temp2);
		snprintf(temp1, 100, "platinum.%d", tradingWith->GetNPCTypeID());
		snprintf(temp2, 100, "%u", trade->pp);
		parse->AddVar(temp1, temp2);

		if(tradingWith->GetAppearance() != eaDead) {
			tradingWith->FaceTarget(this);
		}

		if (parse->HasQuestSub(tradingWith->GetNPCTypeID(), EVENT_TRADE)) {
			std::vector<std::any> item_list(items.begin(), items.end());
			parse->EventNPC(EVENT_TRADE, tradingWith->CastToNPC(), this, "", 0, &item_list);
		}

		for(int i = 0; i < 4; ++i) {
			if(insts[i]) {
				safe_delete(insts[i]);
			}
		}
	}
}

bool Client::CheckTradeLoreConflict(Client* other)
{
	if (!other) {
		return true;
	}

	bool has_lore_item = false;
	std::vector<uint32> lore_item_ids;

	for (int16 index = EQ::invslot::TRADE_BEGIN; index <= EQ::invslot::TRADE_END; ++index) {
		const auto inst = m_inv[index];
		if (!inst || !inst->GetItem()) {
			continue;
		}

		if (other->CheckLoreConflict(inst->GetItem())) {
			lore_item_ids.emplace_back(inst->GetItem()->ID);

			has_lore_item = true;
		}
	}

	for (int16 index = EQ::invbag::TRADE_BAGS_BEGIN; index <= EQ::invbag::TRADE_BAGS_END; ++index) {
		const auto inst = m_inv[index];
		if (!inst || !inst->GetItem()) {
			continue;
		}

		if (other->CheckLoreConflict(inst->GetItem())) {
			lore_item_ids.emplace_back(inst->GetItem()->ID);

			has_lore_item = true;
		}
	}

	if (has_lore_item && RuleB(Character, PlayerTradingLoreFeedback)) {
		for (const uint32 lore_item_id : lore_item_ids) {
			Message(
				Chat::Red,
				fmt::format(
					"{} already has a lore {} in their inventory.",
					other->GetCleanName(),
					database.CreateItemLink(lore_item_id)
				).c_str()
			);
		}
	}

	return has_lore_item;
}

bool Client::CheckTradeNonDroppable()
{
	for (int16 index = EQ::invslot::TRADE_BEGIN; index <= EQ::invslot::TRADE_END; ++index){
		const EQ::ItemInstance* inst = m_inv[index];
		if (!inst)
			continue;

		if (!inst->IsDroppable())
			return true;
	}

	return false;
}

void Client::TraderShowItems()
{
	auto outapp = new EQApplicationPacket(OP_Trader, sizeof(Trader_Struct));
	auto data   = (Trader_Struct *) outapp->pBuffer;

	auto   trader_items = TraderRepository::GetWhere(database, fmt::format("`char_id` = '{}'", CharacterID()));
	uint32 item_limit   = trader_items.size() >= GetInv().GetLookup()->InventoryTypeSize.Bazaar ?
		GetInv().GetLookup()->InventoryTypeSize.Bazaar :
		trader_items.size();

	for (int i = 0; i < item_limit; i++) {
		data->item_cost[i] = trader_items.at(i).item_cost;
		data->items[i]     = ClientVersion() == EQ::versions::ClientVersion::RoF2 ? trader_items.at(i).item_sn
			: trader_items.at(i).item_id;
	}

	data->action = ListTraderItems;

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendTraderPacket(Client* Trader, uint32 Unknown72)
{
	if(!Trader)
		return;

	auto outapp = new EQApplicationPacket(OP_BecomeTrader, sizeof(BecomeTrader_Struct));

	BecomeTrader_Struct* bts = (BecomeTrader_Struct*)outapp->pBuffer;

	bts->action = BazaarTrader_StartTraderMode;

	bts->trader_id = Trader->CharacterID();
	bts->entity_id = Trader->GetID();
	strn0cpy(bts->trader_name, Trader->GetName(), sizeof(bts->trader_name));

	QueuePacket(outapp);


	safe_delete(outapp);
}

void Client::Trader_CustomerBrowsing(Client *Customer)
{

	auto outapp = new EQApplicationPacket(OP_Trader, sizeof(Trader_ShowItems_Struct));
	auto sis    = (Trader_ShowItems_Struct *) outapp->pBuffer;

	sis->action    = CustomerBrowsing;
	sis->entity_id = Customer->GetID();

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::TraderStartTrader(const EQApplicationPacket *app)
{
	uint32                                max_items         = GetInv().GetLookup()->InventoryTypeSize.Bazaar;
	auto                                  in                = (ClickTrader_Struct *) app->pBuffer;
	auto                                  inv               = GetTraderItems();
	bool                                  trade_items_valid = true;
	std::vector<TraderRepository::Trader> trader_items{};

	//Check inventory for no-trade items
	for (auto i = 0; i < max_items; i++) {
		if (inv->items[i] == 0 || inv->serial_number[i] == 0) {
			continue;
		}

		auto inst = FindTraderItemBySerialNumber(inv->serial_number[i]);
		if (inst) {
			if (inst->GetItem() && inst->GetItem()->NoDrop == 0) {
				Message(
					Chat::Red,
					fmt::format(
						"Item: {} is NODROP and found in a Trader's Satchel. Please remove and restart trader mode",
						inst->GetItem()->Name
					).c_str()
				);
				TraderEndTrader();
				safe_delete(inv);
				return;
			}
		}
	}

	for (uint32 i = 0; i < max_items; i++) {
		if (inv->serial_number[i] <= 0) {
			continue;
		}

		auto inst = FindTraderItemBySerialNumber(inv->serial_number[i]);
		if (!inst) {
			trade_items_valid = false;
			break;
		}

		auto it   = std::find(std::begin(in->serial_number), std::end(in->serial_number), inv->serial_number[i]);
		if (inst && it != std::end(in->serial_number)) {
			inst->SetPrice(in->item_cost[i]);
			TraderRepository::Trader trader_item{};

			trader_item.id             = 0;
			trader_item.char_entity_id = GetID();
			trader_item.char_id        = CharacterID();
			trader_item.char_zone_id   = GetZoneID();
			trader_item.item_charges   = inst->GetCharges() == 0 ? 1 : inst->GetCharges();
			trader_item.item_cost      = inst->GetPrice();
			trader_item.item_id        = inst->GetID();
			trader_item.item_sn        = in->serial_number[i];
			trader_item.slot_id        = i;
			if (inst->IsAugmented()) {
				auto augs              = inst->GetAugmentIDs();
				trader_item.aug_slot_1 = augs.at(0);
				trader_item.aug_slot_2 = augs.at(1);
				trader_item.aug_slot_3 = augs.at(2);
				trader_item.aug_slot_4 = augs.at(3);
				trader_item.aug_slot_5 = augs.at(4);
				trader_item.aug_slot_6 = augs.at(5);
			}

			trader_items.emplace_back(trader_item);
			continue;
		}
		else if (inst) {
			Message(
				Chat::Red,
				fmt::format(
					"Item: {} has no price set. Please set a price and try again.",
					inst->GetItem()->Name
				).c_str()
			);
			trade_items_valid = false;
			continue;
		}
	}

	if (!trade_items_valid) {
		Message(Chat::Red, "You are not able to become a trader at this time.  Invalid item found.");
		TraderEndTrader();
		safe_delete(inv);
		return;
	}

	TraderRepository::DeleteWhere(database, fmt::format("`char_id` = '{}';", CharacterID()));
	TraderRepository::ReplaceMany(database, trader_items);
	safe_delete(inv);

	// This refreshes the Trader window to display the End Trader button
	if (ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		auto outapp = new EQApplicationPacket(OP_Trader, sizeof(TraderStatus_Struct));
		auto data   = (TraderStatus_Struct *) outapp->pBuffer;
		data->Code = TraderAck2;
		QueuePacket(outapp);
		safe_delete(outapp);
	}

	MessageString(Chat::Yellow, TRADER_MODE_ON);
	SetTrader(true);
	SendTraderMode(TraderOn);
	SendBecomeTraderToWorld(this, TraderOn);
	LogTrading("Trader Mode ON for Player [{}] with client version {}.", GetCleanName(), (uint32) ClientVersion());
}

void Client::TraderEndTrader()
{
	if (IsThereACustomer()) {
		auto customer = entity_list.GetClientByID(GetCustomerID());
		if (customer) {
			auto end_session = new EQApplicationPacket(OP_ShopEnd);
			customer->FastQueuePacket(&end_session);
		}
	}

	TraderRepository::DeleteWhere(database, fmt::format("`char_id` = '{}'", CharacterID()));

	SendBecomeTraderToWorld(this, TraderOff);
	SendTraderMode(TraderOff);

	WithCustomer(0);
	SetTrader(false);
}

void Client::SendTraderItem(uint32 ItemID, uint16 Quantity, TraderRepository::Trader &t) {

	std::string Packet;
	int16 FreeSlotID=0;

	const EQ::ItemData* item = database.GetItem(ItemID);

	if(!item){
		LogTrading("Bogus item deleted in Client::SendTraderItem!\n");
		return;
	}

	std::unique_ptr<EQ::ItemInstance> inst(
		database.CreateItem(
			item,
			Quantity,
			t.aug_slot_1,
			t.aug_slot_2,
			t.aug_slot_3,
			t.aug_slot_4,
			t.aug_slot_5,
			t.aug_slot_6
		)
	);

	if (inst)
	{
		bool is_arrow = (inst->GetItem()->ItemType == EQ::item::ItemTypeArrow) ? true : false;
		FreeSlotID = m_inv.FindFreeSlot(false, true, inst->GetItem()->Size, is_arrow);

		if (TryStacking(inst.get(), ItemPacketTrade, true, false)) {
		}
		else {
			PutItemInInventory(FreeSlotID, *inst);
			SendItemPacket(FreeSlotID, inst.get(), ItemPacketTrade);
		}
		Save();
	}
}

void Client::SendSingleTraderItem(uint32 char_id, int serial_number)
{
	auto inst = database.LoadSingleTraderItem(char_id, serial_number);
	if (inst) {
		SendItemPacket(EQ::invslot::slotCursor, inst.get(), ItemPacketMerchant); // MainCursor?
	}
}

void Client::BulkSendTraderInventory(uint32 char_id)
{
	const EQ::ItemData *item;

	auto   trader_items = TraderRepository::GetWhere(database, fmt::format("`char_id` = '{}'", char_id));
	uint32 item_limit   = trader_items.size() >= GetInv().GetLookup()->InventoryTypeSize.Bazaar ?
		GetInv().GetLookup()->InventoryTypeSize.Bazaar :
		trader_items.size();

	for (uint32 i = 0; i < item_limit; i++) {
		if ((trader_items.at(i).item_id == 0) || (trader_items.at(i).item_cost == 0)) {
			continue;
		}
		else {
			item = database.GetItem(trader_items.at(i).item_id);
		}

		if (item && (item->NoDrop != 0)) {
			std::unique_ptr<EQ::ItemInstance> inst(
				database.CreateItem(
					trader_items.at(i).item_id,
					trader_items.at(i).item_charges,
					trader_items.at(i).aug_slot_1,
					trader_items.at(i).aug_slot_2,
					trader_items.at(i).aug_slot_3,
					trader_items.at(i).aug_slot_4,
					trader_items.at(i).aug_slot_5,
					trader_items.at(i).aug_slot_6
				)
			);
			if (inst) {
				inst->SetSerialNumber(trader_items.at(i).item_sn);
				if (trader_items.at(i).item_charges > 0) {
					inst->SetCharges(trader_items.at(i).item_charges);
				}

				if (inst->IsStackable()) {
					inst->SetMerchantCount(trader_items.at(i).item_charges);
					inst->SetMerchantSlot(trader_items.at(i).item_sn);
				}

				inst->SetPrice(trader_items.at(i).item_cost);
				SendItemPacket(EQ::invslot::slotCursor, inst.get(), ItemPacketMerchant);
//				safe_delete(inst);
			}
			else
				LogTrading("Client::BulkSendTraderInventory nullptr inst pointer");
		}
	}
}

uint32 Client::FindTraderItemSerialNumber(int32 ItemID) {

	EQ::ItemInstance* item = nullptr;
	uint16 SlotID = 0;
	for (int i = EQ::invslot::GENERAL_BEGIN; i <= EQ::invslot::GENERAL_END; i++){
		item = GetInv().GetItem(i);
		if (item && item->GetItem()->BagType == EQ::item::BagTypeTradersSatchel){
			for (int x = EQ::invbag::SLOT_BEGIN; x <= EQ::invbag::SLOT_END; x++) {
				// we already have the parent bag and a contents iterator..why not just iterate the bag!??
				SlotID = EQ::InventoryProfile::CalcSlotId(i, x);
				item = GetInv().GetItem(SlotID);
				if (item) {
					if (item->GetID() == ItemID)
						return item->GetSerialNumber();
				}
			}
		}
	}
	LogTrading("Client::FindTraderItemSerialNumber Couldn't find item! Item ID [{}]", ItemID);

	return 0;
}

EQ::ItemInstance *Client::FindTraderItemBySerialNumber(int32 SerialNumber)
{
	EQ::ItemInstance *item   = nullptr;
	int16            slot_id = 0;

	for (int16 i = EQ::invslot::GENERAL_BEGIN; i <= EQ::invslot::GENERAL_END; i++) {
		item = GetInv().GetItem(i);
		if (item && item->GetItem()->BagType == EQ::item::BagTypeTradersSatchel) {
			for (int16 x = EQ::invbag::SLOT_BEGIN; x <= EQ::invbag::SLOT_END; x++) {
				// we already have the parent bag and a contents iterator..why not just iterate the bag!??
				slot_id = EQ::InventoryProfile::CalcSlotId(i, x);
				item    = GetInv().GetItem(slot_id);
				if (item) {
					if (item->GetSerialNumber() == SerialNumber) {
						return item;
					}
				}
			}
		}
	}

	LogTrading("Couldn't find item! Serial No. was [{}]", SerialNumber);

	return nullptr;
}


GetItems_Struct *Client::GetTraderItems()
{
	const EQ::ItemInstance *item   = nullptr;
	int16                  slot_id = INVALID_INDEX;
	auto                   gis     = new GetItems_Struct{0};
	uint8                  ndx     = 0;

	for (int16 i = EQ::invslot::GENERAL_BEGIN; i <= EQ::invslot::GENERAL_END; i++) {
		if (ndx >= GetInv().GetLookup()->InventoryTypeSize.Bazaar) {
			break;
		}
		item = GetInv().GetItem(i);
		if (item && item->GetItem()->BagType == EQ::item::BagTypeTradersSatchel) {
			for (int x = EQ::invbag::SLOT_BEGIN; x <= EQ::invbag::SLOT_END; x++) {
				if (ndx >= GetInv().GetLookup()->InventoryTypeSize.Bazaar) {
					break;
				}

				slot_id = EQ::InventoryProfile::CalcSlotId(i, x);
				item    = GetInv().GetItem(slot_id);

				if (item) {
					gis->items[ndx]         = item->GetID();
					gis->serial_number[ndx] = item->GetSerialNumber();
					gis->charges[ndx]       = item->GetCharges() == 0 ? 1 : item->GetCharges();
					ndx++;
				}
			}
		}
	}
	return gis;
}

uint16 Client::FindTraderItem(int32 SerialNumber, uint16 Quantity){

	const EQ::ItemInstance* item= nullptr;
	uint16 SlotID = 0;
	for (int i = EQ::invslot::GENERAL_BEGIN; i <= EQ::invslot::GENERAL_END; i++) {
		item = GetInv().GetItem(i);
		if (item && item->GetItem()->BagType == EQ::item::BagTypeTradersSatchel){
			for (int x = EQ::invbag::SLOT_BEGIN; x <= EQ::invbag::SLOT_END; x++){
				SlotID = EQ::InventoryProfile::CalcSlotId(i, x);

				item = GetInv().GetItem(SlotID);

				if (item && item->GetSerialNumber() == SerialNumber &&
					(item->GetCharges() >= Quantity || (item->GetCharges() <= 0 && Quantity == 1)))
				{
					return SlotID;
				}
			}
		}
	}
	LogTrading("Could NOT find a match for Item: [{}] with a quantity of: [{}] on Trader: [{}]\n",
					SerialNumber , Quantity, GetName());

	return 0;
}

void Client::NukeTraderItem(
	uint16 slot,
	int16 charges,
	int16 quantity,
	Client *customer,
	uint16 trader_slot,
	int32 serial_number,
	int32 item_id
)
{
	if (!customer) {
		return;
	}

	LogTrading("NukeTraderItem(Slot <green>[{}] Charges <green>[{}] Quantity <green>[{}]", slot, charges, quantity);
	if (quantity < charges) {
		customer->SendSingleTraderItem(CharacterID(), serial_number);
		m_inv.DeleteItem(slot, quantity);
	}
	else {
		auto outapp = new EQApplicationPacket(OP_TraderDelItem, sizeof(TraderDelItem_Struct));
		auto tdis   = (TraderDelItem_Struct *) outapp->pBuffer;

		tdis->unknown_000 = 0;
		tdis->trader_id   = customer->GetID();
		tdis->item_id     = serial_number;
		tdis->unknown_012 = 0;
		customer->QueuePacket(outapp);
		safe_delete(outapp);

		m_inv.DeleteItem(slot);
	}
	// This updates the trader. Removes it from his trading bags.
	//
	const EQ::ItemInstance *Inst = m_inv[slot];
	database.SaveInventory(CharacterID(), Inst, slot);

	EQApplicationPacket *outapp2;

	if (quantity < charges) {
		outapp2 = new EQApplicationPacket(OP_DeleteItem, sizeof(MoveItem_Struct));
	}
	else {
		outapp2 = new EQApplicationPacket(OP_MoveItem, sizeof(MoveItem_Struct));
	}

	auto mis = (MoveItem_Struct *) outapp2->pBuffer;
	mis->from_slot       = slot;
	mis->to_slot         = 0xFFFFFFFF;
	mis->number_in_stack = 0xFFFFFFFF;

	if (quantity >= charges) {
		quantity = 1;
	}

	for (int i = 0; i < quantity; i++) {
		QueuePacket(outapp2);
	}

	safe_delete(outapp2);
}

void Client::FindAndNukeTraderItem(int32 serial_number, int16 quantity, Client *customer, uint16 trader_slot)
{
	const EQ::ItemInstance *item     = nullptr;
	bool                   stackable = false;
	int16                  charges   = 0;
	uint16                 slot_id   = FindTraderItem(serial_number, quantity);

	if (slot_id > 0) {
		item = GetInv().GetItem(slot_id);
		if (!item) {
			LogTrading("Could not find Item: [{}] on Trader: [{}]", serial_number, quantity, GetName());
			return;
		}

		charges   = GetInv().GetItem(slot_id)->GetCharges();
		stackable = item->IsStackable();
		if (!stackable) {
			quantity = (charges > 0) ? charges : 1;
		}

		LogTrading("FindAndNuke <green>[{}] charges <green>[{}] quantity <green>[{}]",
				   item->GetItem()->Name,
				   charges,
				   quantity
		);

		if (charges <= quantity || (charges <= 0 && quantity == 1) || !stackable) {
			DeleteItemInInventory(slot_id, quantity);
			auto   trader_items = TraderRepository::GetWhere(database, fmt::format("`char_id` = '{}'", CharacterID()));
			uint32 item_limit   = trader_items.size() >= GetInv().GetLookup()->InventoryTypeSize.Bazaar ?
				GetInv().GetLookup()->InventoryTypeSize.Bazaar :
				trader_items.size();
			uint8  count        = 0;
			bool   test_slot    = true;

			std::vector<TraderRepository::Trader> delete_queue{};
			for (int i = 0; i < item_limit; i++) {
				if (test_slot && trader_items.at(i).item_sn == serial_number) {
					delete_queue.push_back(trader_items.at(i));
					NukeTraderItem(
						slot_id,
						charges,
						quantity,
						customer,
						trader_slot,
						trader_items.at(i).item_sn,
						trader_items.at(i).item_id
					);
					test_slot = false;
				}
				else if (trader_items.at(i).item_id > 0) {
					count++;
				}
			}

			TraderRepository::DeleteMany(database, delete_queue);
			if (count == 0) {
				TraderEndTrader();
			}

			return;
		}
		else {
			TraderRepository::UpdateQuantity(database, CharacterID(), item->GetSerialNumber(), charges - quantity);
			NukeTraderItem(slot_id, charges, quantity, customer, trader_slot, item->GetSerialNumber(), item->GetID());
			return;
		}
	}
	LogTrading("Could NOT find a match for Item: <red>[{}] with a quantity of: <red>[{}] on Trader: <red>[{}]\n",
			   serial_number,
			   quantity,
			   GetName()
	);
}

void Client::ReturnTraderReq(const EQApplicationPacket *app, int16 trader_item_charges, uint32 item_id)
{
	auto tbs    = (TraderBuy_Struct *) app->pBuffer;
	auto outapp = new EQApplicationPacket(OP_TraderBuy, sizeof(TraderBuy_Struct));
	auto outtbs = (TraderBuy_Struct *) outapp->pBuffer;

	memcpy(outtbs, tbs, app->size);
	if (ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		// Convert Serial Number back to Item ID for RoF+
		outtbs->item_id = item_id;
	}
	else {
		// RoF+ requires individual price, but older clients require total price
		outtbs->price = (tbs->price * static_cast<uint32>(trader_item_charges));
	}

	outtbs->quantity     = trader_item_charges;
	// This should probably be trader ID, not customer ID as it is below.
	outtbs->trader_id    = GetID();
	outtbs->already_sold = 0;

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::TradeRequestFailed(const EQApplicationPacket *app)
{
	auto tbs    = (TraderBuy_Struct *) app->pBuffer;
	auto outapp = new EQApplicationPacket(OP_TraderBuy, sizeof(TraderBuy_Struct));
	auto outtbs = (TraderBuy_Struct *) outapp->pBuffer;

	memcpy(outtbs, tbs, app->size);
	outtbs->already_sold = 0xFFFFFFFF;
	outtbs->trader_id    = 0xFFFFFFFF;

	QueuePacket(outapp);
	safe_delete(outapp);
}

static void BazaarAuditTrail(const char *seller, const char *buyer, const char *itemName, int quantity, int totalCost, int tranType) {

	const std::string& query = fmt::format(
		"INSERT INTO `trader_audit` "
        	"(`time`, `seller`, `buyer`, `itemname`, `quantity`, `totalcost`, `trantype`) "
		"VALUES (NOW(), '{}', '{}', '{}', {}, {}, {})",
		seller,
		buyer,
		Strings::Escape(itemName),
		quantity,
		totalCost,
		tranType
	);
	database.QueryDatabase(query);
}

void Client::BuyTraderItem(TraderBuy_Struct *tbs, Client *Trader, const EQApplicationPacket *app)
{
	if (!Trader) {
		return;
	}

	if (!Trader->IsTrader()) {
		TradeRequestFailed(app);
		return;
	}

	auto in                          = (TraderBuy_Struct *) app->pBuffer;
	auto outapp                      = std::make_unique<EQApplicationPacket>(OP_Trader, sizeof(TraderBuy_Struct));
	auto outtbs                      = (TraderBuy_Struct *) outapp->pBuffer;
	outtbs->item_id                  = tbs->item_id;
	const EQ::ItemInstance *buy_item = nullptr;
	uint32 item_id                   = 0;

	if (ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		tbs->item_id = Strings::ToUnsignedBigInt(tbs->serial_number);
	}

	buy_item = Trader->FindTraderItemBySerialNumber(tbs->item_id);

	if (!buy_item) {
		LogTrading("Unable to find item id <red>[{}] item_sn <red>[{}] on trader", tbs->item_id, tbs->serial_number);
		TradeRequestFailed(app);
		return;
	}

	LogTrading(
		"Name: <green>[{}] IsStackable: <green>[{}] Requested Quantity: <green>[{}] Charges on Item <green>[{}]",
		buy_item->GetItem()->Name,
		buy_item->IsStackable(),
		tbs->quantity,
		buy_item->GetCharges()
	);
	// If the item is not stackable, then we can only be buying one of them.
	if (!buy_item->IsStackable()) {
		outtbs->quantity = 1; // normally you can't send more than 1 here
	}
	else {
		// Stackable items, arrows, diamonds, etc
		int32 item_charges = buy_item->GetCharges();
		// ItemCharges for stackables should not be <= 0
		if (item_charges <= 0) {
			outtbs->quantity = 1;
			// If the purchaser requested more than is in the stack, just sell them how many are actually in the stack.
		}
		else if (static_cast<uint32>(item_charges) < tbs->quantity) {
			outtbs->quantity = item_charges;
		}
		else {
			outtbs->quantity = tbs->quantity;
		}
	}

	LogTrading("Actual quantity that will be traded is <green>[{}]", outtbs->quantity);

	if ((tbs->price * outtbs->quantity) <= 0) {
        Message(Chat::Red, "Internal error. Aborting trade. Please report this to the ServerOP. Error code is 1");
        Trader->Message(
            Chat::Red,
            "Internal error. Aborting trade. Please report this to the ServerOP. Error code is 1"
        );
        LogError(
            "Bazaar: Zero price transaction between <red>[{}] and <red>[{}] aborted. Item: <red>[{}] Charges: "
            "<red>[{}] Qty <red>[{}] Price: <red>[{}]",
            GetName(),
            Trader->GetName(),
            buy_item->GetItem()->Name,
            buy_item->GetCharges(),
            tbs->quantity,
            tbs->price
        );
        TradeRequestFailed(app);
        return;
    }

	uint64 total_transaction_value = static_cast<uint64>(tbs->price) * static_cast<uint64>(outtbs->quantity);

	if (total_transaction_value > MAX_TRANSACTION_VALUE) {
		Message(
			Chat::Red,
			"That would exceed the single transaction limit of %u platinum.",
			MAX_TRANSACTION_VALUE / 1000
		);
		TradeRequestFailed(app);
		return;
	}

	// This cannot overflow assuming MAX_TRANSACTION_VALUE, checked above, is the default of 2000000000
	uint32 total_cost = tbs->price * outtbs->quantity;

	if (Trader->ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		// RoF+ uses individual item price where older clients use total price
		outtbs->price = tbs->price;
	}
	else {
		outtbs->price = total_cost;
	}

	if (!TakeMoneyFromPP(total_cost)) {
        RecordPlayerEventLog(
            PlayerEvent::POSSIBLE_HACK,
            PlayerEvent::PossibleHackEvent{
                .message = "Attempted to buy something in bazaar but did not have enough money."
            }
        );
        TradeRequestFailed(app);
        return;
    }

	LogTrading("Customer Paid: <green>[{}] in Copper", total_cost);

	uint32 platinum = total_cost / 1000;
	total_cost     -= (platinum * 1000);
	uint32 gold     = total_cost / 100;
	total_cost     -= (gold * 100);
	uint32 silver   = total_cost / 10;
	total_cost     -= (silver * 10);
	uint32 copper   = total_cost;

	Trader->AddMoneyToPP(copper, silver, gold, platinum, true);

	if (player_event_logs.IsEventEnabled(PlayerEvent::TRADER_PURCHASE)) {
        auto e = PlayerEvent::TraderPurchaseEvent{
            .item_id              = buy_item->GetID(),
            .item_name            = buy_item->GetItem()->Name,
            .trader_id            = Trader->CharacterID(),
            .trader_name          = Trader->GetCleanName(),
            .price                = tbs->price,
            .charges              = outtbs->quantity,
            .total_cost           = (tbs->price * outtbs->quantity),
            .player_money_balance = GetCarriedMoney(),
        };

        RecordPlayerEventLog(PlayerEvent::TRADER_PURCHASE, e);
    }

	if (player_event_logs.IsEventEnabled(PlayerEvent::TRADER_SELL)) {
        auto e = PlayerEvent::TraderSellEvent{
            .item_id              = buy_item->GetID(),
            .item_name            = buy_item->GetItem()->Name,
            .buyer_id             = CharacterID(),
            .buyer_name           = GetCleanName(),
            .price                = tbs->price,
            .charges              = outtbs->quantity,
            .total_cost           = (tbs->price * outtbs->quantity),
            .player_money_balance = Trader->GetCarriedMoney(),
        };

        RecordPlayerEventLogWithClient(Trader, PlayerEvent::TRADER_SELL, e);
    }

	LogTrading("Trader Received: [{}] Platinum, [{}] Gold, [{}] Silver, [{}] Copper", platinum, gold, silver, copper);
    ReturnTraderReq(app, outtbs->quantity, item_id);

    outtbs->trader_id = GetID();
    outtbs->action    = BazaarBuyItem;
	strn0cpy(outtbs->seller_name, Trader->GetCleanName(), sizeof(outtbs->seller_name));
	strn0cpy(outtbs->buyer_name, GetCleanName(), sizeof(outtbs->buyer_name));
	strn0cpy(outtbs->item_name, buy_item->GetItem()->Name, sizeof(outtbs->item_name));
	strn0cpy(
		outtbs->serial_number,
		fmt::format("{:016}", buy_item->GetSerialNumber()).c_str(),
		sizeof(outtbs->serial_number)
	);

	TraderRepository::Trader t{};
	t.item_charges = buy_item->IsStackable() ? outtbs->quantity : buy_item->GetCharges();
	t.item_id      = buy_item->GetItem()->ID;
	t.aug_slot_1   = buy_item->GetAugmentItemID(0);
	t.aug_slot_2   = buy_item->GetAugmentItemID(1);
	t.aug_slot_3   = buy_item->GetAugmentItemID(2);
	t.aug_slot_4   = buy_item->GetAugmentItemID(3);
	t.aug_slot_5   = buy_item->GetAugmentItemID(4);
	t.aug_slot_6   = buy_item->GetAugmentItemID(5);
	t.char_id      = CharacterID();
	t.slot_id      = FindNextFreeParcelSlot(CharacterID());

	SendTraderItem(
		buy_item->GetItem()->ID,
		buy_item->IsStackable() ? outtbs->quantity : buy_item->GetCharges(),
		t
	);

	if (RuleB(Bazaar, AuditTrail)) {
		BazaarAuditTrail(Trader->GetName(), GetName(), buy_item->GetItem()->Name, outtbs->quantity, outtbs->price, 0);
	}

	Trader->FindAndNukeTraderItem(tbs->item_id, outtbs->quantity, this, 0);

	if (item_id > 0 && Trader->ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		// Convert Serial Number back to ItemID for RoF+
		outtbs->item_id = item_id;
	}

	Trader->QueuePacket(outapp.get());
}

void Client::SendBazaarWelcome()
{
	const auto results = TraderRepository::GetWelcomeData(database);
	auto       outapp  = std::make_unique<EQApplicationPacket>(OP_BazaarSearch, sizeof(BazaarWelcome_Struct));
	auto       data    = (BazaarWelcome_Struct *) outapp->pBuffer;

	data->action  = BazaarWelcome;
	data->traders = results.count_of_traders;
	data->items   = results.count_of_items;

	QueuePacket(outapp.get());
}

void Client::SendBarterWelcome()
{
	const auto results = BuyerBuyLinesRepository::GetWelcomeData(database);
	MessageString(Chat::White, BUYER_WELCOME, std::to_string(results.count_of_buyers).c_str());
}

void Client::DoBazaarSearch(BazaarSearchCriteria_Struct search_criteria)
{
	auto results = Bazaar::GetSearchResults(database, search_criteria, GetZoneID());
	if (results.empty()) {
		SendBazaarDone(GetID());
		return;
	}

	std::stringstream           ss{};
	cereal::BinaryOutputArchive ar(ss);
	ar(results);

	uint32 packet_size = ss.str().length() + sizeof(BazaarSearchMessaging_Struct);
	auto   out         = new EQApplicationPacket(OP_BazaarSearch, packet_size);
	auto   data        = (BazaarSearchMessaging_Struct *) out->pBuffer;

	data->action = BazaarSearch;
	memcpy(data->payload, ss.str().data(), ss.str().length());
	FastQueuePacket(&out);

	SendBazaarDone(GetID());
	SendBazaarDeliveryCosts();
}

void Client::SendBazaarResults(
	uint32 trader_id,
	uint32 in_class,
	uint32 in_race,
	uint32 item_stat,
	uint32 item_slot,
	uint32 item_type,
	char item_name[64],
	uint32 min_price,
	uint32 max_price
)
{
	std::string search_values   = " COUNT(item_id), trader.*, items.name ";
	std::string search_criteria = " WHERE trader.item_id = items.id ";

	if (trader_id > 0) {
		Client *trader = entity_list.GetClientByID(trader_id);

		if (trader) {
			search_criteria.append(StringFormat(" AND trader.char_id = %i", trader->CharacterID()));
		}
	}

	if (min_price != 0) {
		search_criteria.append(StringFormat(" AND trader.item_cost >= %i", min_price));
	}

	if (max_price != 0) {
		search_criteria.append(StringFormat(" AND trader.item_cost <= %i", max_price));
	}

	if (strlen(item_name) > 0) {
		char *safeName = RemoveApostrophes(item_name);
		search_criteria.append(StringFormat(" AND items.name LIKE '%%%s%%'", safeName));
		safe_delete_array(safeName);
	}

	if (in_class != 0xFFFFFFFF) {
		search_criteria.append(StringFormat(" AND MID(REVERSE(BIN(items.classes)), %i, 1) = 1", in_class));
	}

	if (in_race != 0xFFFFFFFF) {
		search_criteria.append(StringFormat(" AND MID(REVERSE(BIN(items.races)), %i, 1) = 1", in_race));
	}

	if (item_slot != 0xFFFFFFFF) {
		search_criteria.append(StringFormat(" AND MID(REVERSE(BIN(items.slots)), %i, 1) = 1", item_slot + 1));
	}

	switch (item_type) {
		case 0xFFFFFFFF:
			break;
		case 0:
			// 1H Slashing
			search_criteria.append(" AND items.itemtype = 0 AND damage > 0");
			break;
		case 31:
			search_criteria.append(" AND items.itemclass = 2");
			break;
		case 46:
			search_criteria.append(" AND items.scrolleffect > 0 AND items.scrolleffect < 65000");
			break;
		case 47:
			search_criteria.append(" AND items.worneffect = 998");
			break;
		case 48:
			search_criteria.append(" AND items.worneffect >= 1298 AND items.worneffect <= 1307");
			break;
		case 49:
			search_criteria.append(" AND items.focuseffect > 0");
			break;

		default:
			search_criteria.append(StringFormat(" AND items.itemtype = %i", item_type));
	}

	switch (item_stat) {

		case STAT_AC:
			search_criteria.append(" AND items.ac > 0");
			search_values.append(", items.ac");
			break;

		case STAT_AGI:
			search_criteria.append(" AND items.aagi > 0");
			search_values.append(", items.aagi");
			break;

		case STAT_CHA:
			search_criteria.append(" AND items.acha > 0");
			search_values.append(", items.acha");
			break;

		case STAT_DEX:
			search_criteria.append(" AND items.adex > 0");
			search_values.append(", items.adex");
			break;

		case STAT_INT:
			search_criteria.append(" AND items.aint > 0");
			search_values.append(", items.aint");
			break;

		case STAT_STA:
			search_criteria.append(" AND items.asta > 0");
			search_values.append(", items.asta");
			break;

		case STAT_STR:
			search_criteria.append(" AND items.astr > 0");
			search_values.append(", items.astr");
			break;

		case STAT_WIS:
			search_criteria.append(" AND items.awis > 0");
			search_values.append(", items.awis");
			break;

		case STAT_COLD:
			search_criteria.append(" AND items.cr > 0");
			search_values.append(", items.cr");
			break;

		case STAT_DISEASE:
			search_criteria.append(" AND items.dr > 0");
			search_values.append(", items.dr");
			break;

		case STAT_FIRE:
			search_criteria.append(" AND items.fr > 0");
			search_values.append(", items.fr");
			break;

		case STAT_MAGIC:
			search_criteria.append(" AND items.mr > 0");
			search_values.append(", items.mr");
			break;

		case STAT_POISON:
			search_criteria.append(" AND items.pr > 0");
			search_values.append(", items.pr");
			break;

		case STAT_HP:
			search_criteria.append(" AND items.hp > 0");
			search_values.append(", items.hp");
			break;

		case STAT_MANA:
			search_criteria.append(" AND items.mana > 0");
			search_values.append(", items.mana");
			break;

		case STAT_ENDURANCE:
			search_criteria.append(" AND items.endur > 0");
			search_values.append(", items.endur");
			break;

		case STAT_ATTACK:
			search_criteria.append(" AND items.attack > 0");
			search_values.append(", items.attack");
			break;

		case STAT_HP_REGEN:
			search_criteria.append(" AND items.regen > 0");
			search_values.append(", items.regen");
			break;

		case STAT_MANA_REGEN:
			search_criteria.append(" AND items.manaregen > 0");
			search_values.append(", items.manaregen");
			break;

		case STAT_HASTE:
			search_criteria.append(" AND items.haste > 0");
			search_values.append(", items.haste");
			break;

		case STAT_DAMAGE_SHIELD:
			search_criteria.append(" AND items.damageshield > 0");
			search_values.append(", items.damageshield");
			break;

		default:
			search_values.append(", 0");
			break;
	}

	std::string query = StringFormat(
		"SELECT %s, SUM(charges), items.stackable "
		"FROM trader, items %s GROUP BY items.id, charges, char_id LIMIT %i",
		search_values.c_str(),
		search_criteria.c_str(),
		RuleI(Bazaar, MaxSearchResults)
	);

	auto results = database.QueryDatabase(query);

	if (!results.Success()) {
		return;
	}

	LogTrading("SRCH: [{}]", query.c_str());

	int    Size = 0;
	uint32 ID   = 0;

	if (results.RowCount() == static_cast<unsigned long>(RuleI(Bazaar, MaxSearchResults))) {
		Message(
			Chat::Yellow,
			"Your search reached the limit of %i results. Please narrow your search down by selecting more options.",
			RuleI(Bazaar, MaxSearchResults));
	}

	if (results.RowCount() == 0) {
		auto                    outapp2 = new EQApplicationPacket(OP_BazaarSearch, sizeof(BazaarReturnDone_Struct));
		BazaarReturnDone_Struct *brds   = (BazaarReturnDone_Struct *) outapp2->pBuffer;
		brds->TraderID   = ID;
		brds->Type       = BazaarSearchDone;
		brds->Unknown008 = 0xFFFFFFFF;
		brds->Unknown012 = 0xFFFFFFFF;
		brds->Unknown016 = 0xFFFFFFFF;
		QueuePacket(outapp2);
		safe_delete(outapp2);
		return;
	}

	Size = results.RowCount() * sizeof(BazaarSearchResults_Struct);
	auto  buffer  = new uchar[Size];
	uchar *bufptr = buffer;
	memset(buffer, 0, Size);

	int    Action          = BazaarSearchResults;
	uint32 Cost            = 0;
	int32  SerialNumber    = 0;
	char   temp_buffer[64] = {0};
	int    Count           = 0;
	uint32 StatValue       = 0;

	for (auto &row = results.begin(); row != results.end(); ++row) {
		VARSTRUCT_ENCODE_TYPE(uint32, bufptr, Action);
		Count = Strings::ToInt(row[0]);
		VARSTRUCT_ENCODE_TYPE(uint32, bufptr, Count);
		SerialNumber = Strings::ToInt(row[3]);
		VARSTRUCT_ENCODE_TYPE(int32, bufptr, SerialNumber);
		Client *Trader2 = entity_list.GetClientByCharID(Strings::ToInt(row[1]));
		if (Trader2) {
			ID = Trader2->GetID();
			VARSTRUCT_ENCODE_TYPE(uint32, bufptr, ID);
		}
		else {
			LogTrading("Unable to find trader: [{}]\n", Strings::ToInt(row[1]));
			VARSTRUCT_ENCODE_TYPE(uint32, bufptr, 0);
		}
		Cost = Strings::ToInt(row[5]);
		VARSTRUCT_ENCODE_TYPE(uint32, bufptr, Cost);
		StatValue = Strings::ToInt(row[8]);
		VARSTRUCT_ENCODE_TYPE(uint32, bufptr, StatValue);
		bool Stackable = Strings::ToInt(row[10]);
		if (Stackable) {
			int Charges = Strings::ToInt(row[9]);
			sprintf(temp_buffer, "%s(%i)", row[7], Charges);
		}
		else {
			sprintf(temp_buffer, "%s(%i)", row[7], Count);
		}

		memcpy(bufptr, &temp_buffer, strlen(temp_buffer));

		bufptr += 64;

		// Extra fields for SoD+
		//
		if (Trader2) {
			sprintf(temp_buffer, "%s", Trader2->GetName());
		}
		else {
			sprintf(temp_buffer, "Unknown");
		}

		memcpy(bufptr, &temp_buffer, strlen(temp_buffer));

		bufptr += 64;

		VARSTRUCT_ENCODE_TYPE(uint32, bufptr, Strings::ToInt(row[1])); // ItemID
	}

	auto outapp = new EQApplicationPacket(OP_BazaarSearch, Size);

	memcpy(outapp->pBuffer, buffer, Size);

	QueuePacket(outapp);

	safe_delete(outapp);
	safe_delete_array(buffer);

	auto                    outapp2 = new EQApplicationPacket(OP_BazaarSearch, sizeof(BazaarReturnDone_Struct));
	BazaarReturnDone_Struct *brds   = (BazaarReturnDone_Struct *) outapp2->pBuffer;

	brds->TraderID = ID;
	brds->Type     = BazaarSearchDone;

	brds->Unknown008 = 0xFFFFFFFF;
	brds->Unknown012 = 0xFFFFFFFF;
	brds->Unknown016 = 0xFFFFFFFF;

	QueuePacket(outapp2);

	safe_delete(outapp2);
}

static void UpdateTraderCustomerItemsAdded(
	uint32 customer_id,
	std::vector<BaseTraderRepository::Trader> trader_items,
	uint32 item_id,
	uint32 item_limit
)
{
	// Send Item packets to the customer to update the Merchant window with the
	// new items for sale, and give them a message in their chat window.
	auto customer = entity_list.GetClientByID(customer_id);
	if (!customer) {
		return;
	}

	const EQ::ItemData *item = database.GetItem(item_id);
	if (!item) {
		return;
	}

	customer->Message(Chat::Red, "The Trader has put up %s for sale.", item->Name);

	for (auto const &i: trader_items) {
		if (i.item_id == item_id) {
			std::unique_ptr<EQ::ItemInstance> inst(
				database.CreateItem(
					i.item_id,
					i.item_charges,
					i.aug_slot_1,
					i.aug_slot_2,
					i.aug_slot_3,
					i.aug_slot_4,
					i.aug_slot_5,
					i.aug_slot_6
				)
			);
			if (!inst) {
				return;
			}

			inst->SetCharges(i.item_charges);
			inst->SetPrice(i.item_cost);
			inst->SetSerialNumber(i.item_sn);
			inst->SetMerchantSlot(i.item_sn);
			if (inst->IsStackable()) {
				inst->SetMerchantCount(i.item_charges);
			}

			customer->SendItemPacket(EQ::invslot::slotCursor, inst.get(), ItemPacketMerchant); // MainCursor?
			LogTrading("Sending price update for [{}], Serial No. [{}] with [{}] charges",
					   item->Name, i.item_sn, i.item_charges);
		}
	}
}

static void UpdateTraderCustomerPriceChanged(
	uint32 customer_id,
	std::vector<BaseTraderRepository::Trader> trader_items,
	uint32 item_id,
	int32 charges,
	uint32 new_price,
	uint32 item_limit
)
{
	// Send ItemPackets to update the customer's Merchant window with the new price (or remove the item if
	// the new price is 0) and inform them with a chat message.
	auto customer = entity_list.GetClientByID(customer_id);

	if (!customer) {
		return;
	}

	const EQ::ItemData *item = database.GetItem(item_id);

	if (!item) {
		return;
	}

	if (new_price == 0) {
		// If the new price is 0, remove the item(s) from the window.
		auto outapp = new EQApplicationPacket(OP_TraderDelItem, sizeof(TraderDelItem_Struct));
		auto tdis   = (TraderDelItem_Struct *) outapp->pBuffer;

		tdis->unknown_000 = 0;
		tdis->trader_id   = customer->GetID();
		tdis->unknown_012 = 0;
		customer->Message(Chat::Red, "The Trader has withdrawn the %s from sale.", item->Name);

		for (int i = 0; i < item_limit; i++) {
			if (trader_items.at(i).item_id == item_id) {
				if (customer->ClientVersion() >= EQ::versions::ClientVersion::RoF) {
					// RoF+ use Item IDs for now
					tdis->item_id = trader_items.at(i).item_id;
				}
				else {
					tdis->item_id = trader_items.at(i).item_sn;
				}
				tdis->item_id = trader_items.at(i).item_sn;
				LogTrading("Telling customer to remove item [{}] with [{}] charges and S/N [{}]",
						   item_id, charges, trader_items.at(i).item_sn);

				customer->QueuePacket(outapp);
			}
		}

		safe_delete(outapp);
		return;
	}

	LogTrading("Sending price updates to customer [{}]", customer->GetName());

	auto it = std::find_if(trader_items.begin(), trader_items.end(), [&](TraderRepository::Trader x){ return x.item_id == item->ID;});
	std::unique_ptr<EQ::ItemInstance> inst(
		database.CreateItem(
			it->item_id,
			it->item_charges,
			it->aug_slot_1,
			it->aug_slot_2,
			it->aug_slot_3,
			it->aug_slot_4,
			it->aug_slot_5,
			it->aug_slot_6
		)
	);
	if (!inst) {
		return;
	}

	if (charges > 0) {
		inst->SetCharges(charges);
	}

	inst->SetPrice(new_price);
	if (inst->IsStackable()) {
		inst->SetMerchantCount(charges);
	}

	// Let the customer know the price in the window has suddenly just changed on them.
	customer->Message(Chat::Red, "The Trader has changed the price of %s.", item->Name);

	for (int i = 0; i < item_limit; i++) {
		if ((trader_items.at(i).item_id != item_id) ||
			((!item->Stackable) && (trader_items.at(i).item_charges != charges))) {
			continue;
		}

		inst->SetSerialNumber(trader_items.at(i).item_sn);
		inst->SetMerchantSlot(trader_items.at(i).item_sn);

		LogTrading("Sending price update for [{}], Serial No. [{}] with [{}] charges",
				   item->Name, trader_items.at(i).item_sn, trader_items.at(i).item_charges);

		customer->SendItemPacket(EQ::invslot::slotCursor, inst.get(), ItemPacketMerchant); // MainCursor??
	}
//	safe_delete(inst);
}

void Client::SendBuyerResults(BarterSearchRequest_Struct& bsr)
{
	if (ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		std::string search_string(bsr.search_string);
		BuyerLineSearch_Struct results{};

		SetBarterTime();

		if (bsr.search_scope == 1) {
			// Local Buyers
			results = BuyerBuyLinesRepository::SearchBuyLines(database, search_string, 0, GetZoneID(), GetInstanceID());
		}
		else if (bsr.buyer_id) {
			// Specific Buyer
			results = BuyerBuyLinesRepository::SearchBuyLines(database, search_string, bsr.buyer_id);
		} else {
			// All Buyers
			results = BuyerBuyLinesRepository::SearchBuyLines(database, search_string);
		}

		if (results.buy_line.empty()) {
			Message(Chat::White, "No buylines could be found.");
			return;
		}

		std::string buyer_name = "ID {} not in zone.";
		if (search_string.empty()) {
			search_string = "*";
		}

		results.search_string  = std::move(search_string);
		results.transaction_id = bsr.transaction_id;
		std::stringstream ss{};
		cereal::BinaryOutputArchive ar(ss);

		{ ar(results); }

		auto packet = std::make_unique<EQApplicationPacket>(OP_BuyerItems, ss.str().length() + sizeof(BuyerGeneric_Struct));
		auto emu    = (BuyerGeneric_Struct *) packet->pBuffer;

		emu->action = Barter_BuyerSearch;
		memcpy(emu->payload, ss.str().data(), ss.str().length());

		QueuePacket(packet.get());

		ss.str("");
		ss.clear();

	}
}

void Client::ShowBuyLines(const EQApplicationPacket *app)
{
	auto bir   = (BuyerInspectRequest_Struct *) app->pBuffer;
	auto buyer = entity_list.GetClientByID(bir->buyer_id);

	if (!buyer || buyer->GetCustomerID()) {
		bir->approval = 0; // Tell the client that the Buyer is unavailable
		QueuePacket(app);
		MessageString(Chat::Yellow, TRADER_BUSY);
		return;
	}

	if (ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		SetBarterTime();
		bir->approval = buyer->WithCustomer(GetID());
		QueuePacket(app);

		auto results  = BuyerBuyLinesRepository::GetBuyLines(database, buyer->CharacterID());
		auto greeting = BuyerRepository::GetWelcomeMessage(database, buyer->GetBuyerID());

		if (greeting.length() == 0) {
			greeting = "Welcome!";
		}

		MessageString(Chat::NPCQuestSay, BUYER_GREETING, buyer->GetName(), greeting.c_str());
		const std::string name(GetName());
		buyer->SendSellerBrowsing(name);

		std::stringstream           ss{};
		cereal::BinaryOutputArchive ar(ss);

		for (auto l : results) {
			const EQ::ItemData *item = database.GetItem(l.item_id);
			l.enabled     = 1;
			l.item_icon   = item->Icon;
			l.item_toggle = 1;

			{ ar(l); }

			auto packet = std::make_unique<EQApplicationPacket>(OP_BuyerItems, ss.str().length() + sizeof(BuyerGeneric_Struct));
			auto emu    = (BuyerGeneric_Struct *) packet->pBuffer;

			emu->action = Barter_BuyerInspectBegin;
			memcpy(emu->payload, ss.str().data(), ss.str().length());

			QueuePacket(packet.get());

			ss.str("");
			ss.clear();
		}
		
		return;
	}
}

void Client::SellToBuyer(const EQApplicationPacket *app)
{
	if (ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		BuyerLineSellItem_Struct     sell_line{};
		auto                         in = (BuyerGeneric_Struct *) app->pBuffer;
		EQ::Util::MemoryStreamReader ss_in(
			reinterpret_cast<char *>(in->payload),
			app->size - sizeof(BuyerGeneric_Struct));
		cereal::BinaryInputArchive   ar(ss_in);
		ar(sell_line);

		sell_line.seller_name = GetCleanName();

		switch (sell_line.purchase_method) {
			case BarterInBazaar:
			case BarterByVendor: {
				auto buyer = entity_list.GetClientByID(sell_line.buyer_entity_id);
				if (!buyer) {
					SendBarterBuyerClientMessage(
						sell_line,
						Barter_SellerTransactionComplete,
						Barter_Failure,
						Barter_Failure
					);
					break;
				}

				if (!DoBarterBuyerChecks(sell_line)) {
					return;
				};

				if (!DoBarterSellerChecks(sell_line)) {
					return;
				};

				BuyerRepository::UpdateTransactionDate(database, sell_line.buyer_id, time(nullptr));

				if (!FindNumberOfFreeInventorySlotsWithSizeCheck(sell_line.trade_items)) {
					LogTradingDetail("Seller {} has insufficient inventory space for {} compensation items.",
									 GetCleanName(),
									 sell_line.trade_items.size()
					);
					Message(Chat::Red, "Insufficient inventory space for the compensation items.");
					SendBarterBuyerClientMessage(
						sell_line,
						Barter_SellerTransactionComplete,
						Barter_Failure,
						Barter_Failure
					);
					return;
				}

				for (auto const &ti: sell_line.trade_items) {
					std::unique_ptr<EQ::ItemInstance> inst(
						database.CreateItem(
							ti.item_id,
							ti.item_quantity *
							sell_line.seller_quantity
						)
					);

					if (inst.get()->GetItem()) {
						buyer->RemoveItem(ti.item_id, ti.item_quantity * sell_line.seller_quantity);
						if (!PutItemInInventoryWithStacking(inst.get())) {
							Message(Chat::Red, "Error putting item in your inventory.");
							buyer->PutItemInInventoryWithStacking(inst.get());
							SendBarterBuyerClientMessage(
								sell_line,
								Barter_SellerTransactionComplete,
								Barter_Failure,
								Barter_Failure
							);
							return;
						}
					}
				}

				std::unique_ptr<EQ::ItemInstance> buy_inst(
					database.CreateItem(
						sell_line.item_id,
						sell_line.seller_quantity
					)
				);
				RemoveItem(sell_line.item_id, sell_line.seller_quantity);
				if (buy_inst->IsStackable()) {
					if (!buyer->PutItemInInventoryWithStacking(buy_inst.get())) {
						buyer->Message(Chat::Red, "Error putting item in your inventory.");
						PutItemInInventoryWithStacking(buy_inst.get());
						SendBarterBuyerClientMessage(
							sell_line,
							Barter_SellerTransactionComplete,
							Barter_Failure,
							Barter_Failure
						);
						return;
					}
				}
				else {
					for (int i = 1; i <= sell_line.seller_quantity; i++) {
						buy_inst->SetCharges(1);
						if (!buyer->PutItemInInventoryWithStacking(buy_inst.get())) {
							buyer->Message(Chat::Red, "Error putting item in your inventory.");
							PutItemInInventoryWithStacking(buy_inst.get());
							SendBarterBuyerClientMessage(
							sell_line,
								Barter_SellerTransactionComplete,
								Barter_Failure,
								Barter_Failure
							);
							return;
						}
					}
				}

				uint64 total_cost = (uint64) sell_line.item_cost * (uint64) sell_line.seller_quantity;
				AddMoneyToPPWithOverflow(total_cost, false);
				buyer->TakeMoneyFromPPWithOverFlow(total_cost, false);

				if (player_event_logs.IsEventEnabled(PlayerEvent::BARTER_TRANSACTION)) {
					PlayerEvent::BarterTransaction e{};
					e.status        = "Successful Barter Transaction";
					e.item_id       = sell_line.item_id;
					e.item_quantity = sell_line.seller_quantity;
					e.item_name     = sell_line.item_name;
					e.trade_items   = sell_line.trade_items;
					for (auto &t: e.trade_items) {
						t *= sell_line.seller_quantity;
					}
					e.total_cost  = total_cost;
					e.buyer_name  = buyer->GetCleanName();
					e.seller_name = GetCleanName();
					RecordPlayerEventLog(PlayerEvent::BARTER_TRANSACTION, e);
				}

				SendWindowUpdatesToSellerAndBuyer(sell_line);
				SendBarterBuyerClientMessage(
					sell_line,
					Barter_SellerTransactionComplete,
					Barter_Success,
					Barter_Success
				);
				buyer->SendBarterBuyerClientMessage(
					sell_line,
					Barter_BuyerTransactionComplete,
					Barter_Success,
					Barter_Success
				);
				break;
			}
			case BarterOutsideBazaar: {
				bool seller_error = false;
				auto buyer_time   = BuyerRepository::GetTransactionDate(database, sell_line.buyer_id);

				if (buyer_time > GetBarterTime()) {
					SendBarterBuyerClientMessage(
						sell_line,
						Barter_SellerTransactionComplete,
						Barter_Failure,
						Barter_DataOutOfDate
					);
					return;
				}

				if (sell_line.trade_items.size() > 0) {
					Message(Chat::Red, "You must visit the buyer directly when receiving compensation items.");
					seller_error = true;
				}

				auto buy_item_slot_id = GetInv().HasItem(
					sell_line.item_id,
					sell_line.seller_quantity,
					invWherePersonal
				);
				auto buy_item = buy_item_slot_id == INVALID_INDEX ? nullptr : GetInv().GetItem(buy_item_slot_id);
				if (!buy_item) {
					SendBarterBuyerClientMessage(
						sell_line,
						Barter_SellerTransactionComplete,
						Barter_Failure,
						Barter_SellerDoesNotHaveItem
					);
					break;
				}

				if (seller_error) {
					LogTradingDetail("Seller Error <red>[{}]  Sell/Buy Transaction Failed.",
									 seller_error
					);
					SendBarterBuyerClientMessage(
						sell_line,
						Barter_SellerTransactionComplete,
						Barter_Failure,
						Barter_Failure
					);
					return;
				}

				BuyerRepository::UpdateTransactionDate(database, sell_line.buyer_id, time(nullptr));

				auto server_packet = std::make_unique<ServerPacket>(
					ServerOP_BuyerMessaging,
					sizeof(BuyerMessaging_Struct)
				);

				auto data = (BuyerMessaging_Struct *) server_packet->pBuffer;

				data->action           = Barter_SellItem;
				data->buyer_entity_id  = sell_line.buyer_entity_id;
				data->buyer_id         = sell_line.buyer_id;
				data->seller_entity_id = GetID();
				data->buy_item_id      = sell_line.item_id;
				data->buy_item_qty     = sell_line.item_quantity;
				data->buy_item_cost    = sell_line.item_cost;
				data->buy_item_icon    = sell_line.item_icon;
				data->zone_id          = GetZoneID();
				data->slot             = sell_line.slot;
				data->seller_quantity  = sell_line.seller_quantity;
				strn0cpy(data->item_name, sell_line.item_name, sizeof(data->item_name));
				strn0cpy(data->buyer_name, sell_line.buyer_name.c_str(), sizeof(data->buyer_name));
				strn0cpy(data->seller_name, GetCleanName(), sizeof(data->seller_name));

				worldserver.SendPacket(server_packet.get());

				break;
			}
		}
	}
}

void Client::SendBuyerPacket(Client* Buyer) {

	// This is the Buyer Appearance packet. This method is called for each Buyer when a Client connects to the zone.
	//
	auto outapp = new EQApplicationPacket(OP_Barter, 13 + strlen(GetName()));

	char* Buf = (char*)outapp->pBuffer;

	VARSTRUCT_ENCODE_TYPE(uint32, Buf, Barter_BuyerAppearance);
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, Buyer->GetID());
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, 0x01);
	VARSTRUCT_ENCODE_STRING(Buf, GetName());

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::ToggleBuyerMode(bool status)
{
	auto outapp = std::make_unique<EQApplicationPacket>(OP_Barter, sizeof(BuyerSetAppearance_Struct));
	auto data   = (BuyerSetAppearance_Struct *) outapp->pBuffer;

	data->action    = Barter_BuyerAppearance;
	data->entity_id = GetID();

	if (status && IsInBuyerSpace()) {
		SetBuyerID(CharacterID());

		BuyerRepository::Buyer b{};
		b.id                    = 0;
		b.char_id               = GetBuyerID();
		b.char_entity_id        = GetID();
		b.char_zone_id          = GetZoneID();
		b.char_zone_instance_id = GetInstanceID();
		b.char_name             = GetCleanName();
		b.transaction_date      = time(nullptr);
		BuyerRepository::DeleteBuyer(database, GetBuyerID());
		BuyerRepository::InsertOne(database, b);

		data->status = BuyerBarter::On;
		SetCustomerID(0);
		SendBuyerMode(true);
		SendBuyerToBarterWindow(this, Barter_AddToBarterWindow);
		Message(Chat::Yellow, "Barter Mode ON.");
	}
	else {
		data->status = BuyerBarter::Off;
		BuyerRepository::DeleteBuyer(database, GetBuyerID());
		SetCustomerID(0);
		SendBuyerToBarterWindow(this, Barter_RemoveFromBarterWindow);
		SendBuyerMode(false);
		SetBuyerID(0);
		if (!IsInBuyerSpace()) {
			Message(Chat::Red, "You must be in a Barter Stall to start Barter Mode.");
		}
		Message(Chat::Yellow, fmt::format("Barter Mode OFF. Buy lines deactivated.").c_str());
	}

	entity_list.QueueClients(this, outapp.get(), false);
}

void Client::ModifyBuyLine(const EQApplicationPacket *app)
{
	if (ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		BuyerBuyLines_Struct         bl{};
		auto                         in = (BuyerGeneric_Struct *) app->pBuffer;
		EQ::Util::MemoryStreamReader ss_in(
			reinterpret_cast<char *>(in->payload),
			app->size - sizeof(BuyerGeneric_Struct)
		);
		cereal::BinaryInputArchive   ar(ss_in);
		ar(bl);

		if (bl.buy_lines.empty()) {
			return;
		}

		BuyerRepository::UpdateTransactionDate(database, GetBuyerID(), time(nullptr));
		int64 current_total_cost = 0;
		bool  pass               = false;

		auto current_buy_lines = BuyerBuyLinesRepository::GetBuyLines(database, CharacterID());

		std::map<uint32, BuylineItemDetails_Struct> item_map;
		BuildBuyLineMapFromVector(item_map, current_buy_lines);

		current_total_cost = ValidateBuyLineCost(item_map);

		auto buy_line = bl.buy_lines.front();
		auto it       = std::find_if(
			current_buy_lines.cbegin(),
			current_buy_lines.cend(),
			[&](BuyerLineItems_Struct bl) {
				return bl.slot == buy_line.slot;
			}
		);

		if (buy_line.item_toggle) {
			current_total_cost += buy_line.item_cost * buy_line.item_quantity;
			if (it != std::end(current_buy_lines)) {
				current_total_cost -= it->item_cost * it->item_quantity;
				if (current_total_cost > GetCarriedMoney()) {
					buy_line.item_cost     = it->item_cost;
					buy_line.item_quantity = it->item_quantity;
					Message(
						Chat::Red,
						fmt::format(
							"You currently do not have sufficient funds to support your buy lines. You have {} and need {}",
							DetermineMoneyString(GetCarriedMoney()),
							DetermineMoneyString(current_total_cost)).c_str()
					);
					SendBuyLineUpdate(buy_line);
					return;
				}
				else {
					RemoveItemFromBuyLineMap(item_map, *it);
					BuildBuyLineMapFromVector(item_map, bl.buy_lines);
				}
			}
			else {
				BuildBuyLineMapFromVector(item_map, bl.buy_lines);
			}
		}
		else {
			current_total_cost -= static_cast<int64>(buy_line.item_cost) * static_cast<int64>(buy_line.item_quantity);
			std::map<uint32, BuylineItemDetails_Struct> item_map_tmp;
			BuildBuyLineMapFromVector(item_map_tmp, bl.buy_lines);
			if (ValidateBuyLineItems(item_map_tmp)) {
				pass = true;
			}
		}

		if (current_total_cost > static_cast<int64>(GetCarriedMoney())) {
			Message(
				Chat::Red,
				fmt::format(
					"You currently do not have sufficient funds to support your buy lines. You have {} and need {}",
					DetermineMoneyString(GetCarriedMoney()),
					DetermineMoneyString(current_total_cost)).c_str()
			);
			buy_line.item_toggle = 0;
			SendBuyLineUpdate(buy_line);
			return;
		}

		bool buyer_error = false;

		if (!ValidateBuyLineItems(item_map)) {
			buy_line.item_toggle = 0;
		}

		buy_line.item_icon = database.GetItem(buy_line.item_id)->Icon;
		if ((buy_line.item_toggle && it != std::end(current_buy_lines)) || pass) {
			BuyerBuyLinesRepository::ModifyBuyLine(database, buy_line, GetBuyerID());
			Message(Chat::Yellow, fmt::format("Buy line for {} modified.", buy_line.item_name).c_str());
		}
		else if (buy_line.item_toggle && it == std::end(current_buy_lines)) {
			BuyerBuyLinesRepository::CreateBuyLine(database, buy_line, GetBuyerID());
			Message(Chat::Yellow, fmt::format("Buy line for {} enabled.", buy_line.item_name).c_str());
		}
		else if (!buy_line.item_toggle) {
			BuyerBuyLinesRepository::DeleteBuyLine(database, GetBuyerID(), buy_line.slot);
			Message(Chat::Yellow, fmt::format("Buy line for {} disabled.", buy_line.item_name).c_str());
		}
		else {
			BuyerBuyLinesRepository::DeleteBuyLine(database, GetBuyerID(), buy_line.slot);
			Message(
				Chat::Yellow,
				fmt::format("Unhandled modification.  Buy line for {} disabled.", buy_line.item_name).c_str());
		}

		SendBuyLineUpdate(buy_line);

		if (IsThereACustomer()) {
			auto customer = entity_list.GetClientByID(GetCustomerID());
			if (!customer) {
				return;
			}

			auto it = std::find_if(
				current_buy_lines.cbegin(),
				current_buy_lines.cend(),
				[&](BuyerLineItems_Struct bl) {
					return bl.slot == buy_line.slot;
				}
			);
			if (it == std::end(current_buy_lines) && !buy_line.item_toggle) {
				return;
			}

			std::stringstream           ss_customer{};
			cereal::BinaryOutputArchive ar_customer(ss_customer);

			BuyerLineItems_Struct blis{};
			blis.enabled       = buy_line.enabled;
			blis.item_cost     = buy_line.item_cost;
			blis.item_icon     = buy_line.item_icon;
			blis.item_id       = buy_line.item_id;
			blis.item_quantity = buy_line.item_quantity;
			blis.item_toggle   = buy_line.item_toggle;
			blis.slot          = buy_line.slot;
			blis.item_name     = buy_line.item_name;
			for (auto const &i: buy_line.trade_items) {
				BuyerLineTradeItems_Struct bltis{};
				bltis.item_icon     = i.item_icon;
				bltis.item_id       = i.item_id;
				bltis.item_quantity = i.item_quantity;
				bltis.item_name     = i.item_name;
				blis.trade_items.push_back(bltis);
			}

			{ ar_customer(blis); }

			auto packet = std::make_unique<EQApplicationPacket>(
				OP_BuyerItems,
				ss_customer.str().length() +
				sizeof(BuyerGeneric_Struct)
			);
			auto emu    = (BuyerGeneric_Struct *) packet->pBuffer;

			emu->action = Barter_BuyerInspectBegin;
			memcpy(emu->payload, ss_customer.str().data(), ss_customer.str().length());

			customer->QueuePacket(packet.get());

			ss_customer.str("");
			ss_customer.clear();
		}
	}
	return;
}

void Client::BuyerItemSearch(const EQApplicationPacket *app)
{
	auto               bis   = (BuyerItemSearch_Struct *) app->pBuffer;
	const EQ::ItemData *item = 0;
	uint32             it    = 0;

	BuyerItemSearchResults_Struct bisr{};

	while ((item = database.IterateItems(&it)) && bisr.results.size() < RuleI(Bazaar, MaxBuyerInventorySearchResults)) {
		if (!item->NoDrop) {
			continue;
		}

		auto item_name_match = std::strstr(
			Strings::ToLower(item->Name).c_str(),
			Strings::ToLower(bis->search_string).c_str()
		);

		if (item_name_match) {
			BuyerItemSearchResultEntry_Struct bisre{};
			bisre.item_id   = item->ID;
			bisre.item_icon = item->Icon;
			strn0cpy(bisre.item_name, item->Name, sizeof(bisre.item_name));
			bisr.results.push_back(bisre);
		}
	}

	bisr.action       = Barter_BuyerSearchResults;
	bisr.result_count = bisr.results.size();

	std::stringstream           ss{};
	cereal::BinaryOutputArchive ar(ss);
	{ ar(bisr); }

	uint32 packet_size = sizeof(BuyerGeneric_Struct) + ss.str().length();
	auto   outapp      = std::make_unique<EQApplicationPacket>(OP_Barter, packet_size);
	auto   emu         = (BuyerGeneric_Struct *) outapp->pBuffer;

	emu->action = Barter_BuyerSearchResults;
	memcpy(emu->payload, ss.str().data(), ss.str().length());

	QueuePacket(outapp.get());

	ss.str("");
	ss.clear();
}

const std::string &Client::GetMailKeyFull() const
{
	return m_mail_key_full;
}

const std::string &Client::GetMailKey() const
{
	return m_mail_key;
}

void Client::SendBecomeTraderToWorld(Client *trader, BazaarTraderBarterActions action)
{
	auto outapp = new ServerPacket(ServerOP_TraderMessaging, sizeof(TraderMessaging_Struct));
	auto data   = (TraderMessaging_Struct *) outapp->pBuffer;

	data->action    = action;
	data->entity_id = trader->GetID();
	data->trader_id = trader->CharacterID();
	data->zone_id   = trader->GetZoneID();
	strn0cpy(data->trader_name, trader->GetName(), sizeof(data->trader_name));

	worldserver.SendPacket(outapp);
	safe_delete(outapp);
}

void Client::SendBecomeTrader(BazaarTraderBarterActions action, uint32 entity_id)
{
	if (entity_id <= 0) {
		return;
	}

	auto trader = entity_list.GetClientByID(entity_id);
	if (!trader) {
		return;
	}

	auto outapp = new EQApplicationPacket(OP_BecomeTrader, sizeof(BecomeTrader_Struct));
	auto data   = (BecomeTrader_Struct *) outapp->pBuffer;

	data->action    = action;
	data->entity_id = trader->GetID();
	data->trader_id = trader->CharacterID();
	strn0cpy(data->trader_name, trader->GetCleanName(), sizeof(data->trader_name));

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendTraderMode(BazaarTraderBarterActions status)
{
	auto outapp = new EQApplicationPacket(OP_Trader, sizeof(Trader_ShowItems_Struct));
	auto data   = (Trader_ShowItems_Struct *) outapp->pBuffer;

	data->action    = status;
	data->entity_id = GetID();

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::TraderPriceUpdate(const EQApplicationPacket *app)
{
	// Handle price updates from the Trader and update a customer browsing our stuff if necessary
	// This method also handles removing items from sale and adding them back up whilst still in
	// Trader mode.
	//
	auto tpus = (TraderPriceUpdate_Struct *) app->pBuffer;

	LogTrading(
		"Received Price Update for <green>[{}] Item Serial No. <green>[{}] New Price <green>[{}]",
		GetName(),
		tpus->SerialNumber,
		tpus->NewPrice
	);

	// Pull the items this Trader currently has for sale from the trader table.
	//
	auto   trader_items = TraderRepository::GetWhere(database, fmt::format("`char_id` = '{}'", CharacterID()));
	uint32 item_limit   = trader_items.size() >= GetInv().GetLookup()->InventoryTypeSize.Bazaar
		? GetInv().GetLookup()->InventoryTypeSize.Bazaar
		: trader_items.size();

	// The client only sends a single update with the Serial Number of the item whose price has been updated.
	// We must update the price for all the Trader's items that are identical to that one item, i.e.
	// if it is a stackable item like arrows, update the price for all stacks. If it is not stackable, then
	// update the prices for all items that have the same number of charges.
	//
	uint32 id_of_item_to_update      = 0;
	int32  charges_on_item_to_update = 0;
	uint32 old_price                 = 0;

	for (int i = 0; i < item_limit; i++) {
		if ((trader_items.at(i).item_id > 0) && (trader_items.at(i).item_sn == tpus->SerialNumber)) {
			// We found the item that the Trader wants to change the price of (or add back up for sale).
			//
			id_of_item_to_update      = trader_items.at(i).item_id;
			charges_on_item_to_update = trader_items.at(i).item_charges;
			old_price                 = trader_items.at(i).item_cost;

			LogTrading(
				"ItemID is <green>[{}] Charges is <green>[{}]",
				trader_items.at(i).item_id,
				trader_items.at(i).item_charges
			);
			break;
		}
	}

	if (id_of_item_to_update == 0) {
		// If the item is not currently in the trader table for this Trader, then they must have removed it from sale while
		// still in Trader mode. Check if the item is in their Trader Satchels, and if so, put it back up.
		// Quick Sanity check. If the item is not currently up for sale, and the new price is zero, just ack the packet
		// and do nothing.
		if (tpus->NewPrice == 0) {
			tpus->SubAction = BazaarPriceChange_RemoveItem;
			QueuePacket(app);
			return;
		}

		LogTrading("Unable to find item to update price for. Rechecking trader satchels");

		// Find what is in their Trader Satchels
		auto   newgis                 = GetTraderItems();
		uint32 id_of_item_to_add      = 0;
		int32  charges_on_item_to_add = 0;

		for (int i = 0; i < GetInv().GetLookup()->InventoryTypeSize.Bazaar; i++) {
			if ((newgis->items[i] > 0) && (newgis->serial_number[i] == tpus->SerialNumber)) {
				id_of_item_to_add      = newgis->items[i];
				charges_on_item_to_add = newgis->charges[i];

				LogTrading(
					"Found new Item to Add, ItemID is <green>[{}] Charges is <green>[{}]",
					newgis->items[i],
					newgis->charges[i]
				);
				break;
			}
		}

		const EQ::ItemData *item = nullptr;
		if (id_of_item_to_add) {
			item = database.GetItem(id_of_item_to_add);
		}

		if (!id_of_item_to_add || !item) {
			tpus->SubAction = BazaarPriceChange_Fail;
			QueuePacket(app);
			TraderEndTrader();
			safe_delete(newgis);

			LogTrading("Item not found in Trader Satchels either");
			return;
		}

		// It is a limitation of the client that if you have multiple of the same item, but with different charges,
		// although you can set different prices for them before entering Trader mode. If you Remove them and then
		// add them back whilst still in Trader mode, they all go up for the same price. We check for this situation
		// and give the Trader a warning message.
		//
		if (!item->Stackable) {
			bool same_item_with_differing_charges = false;

			for (int i = 0; i < GetInv().GetLookup()->InventoryTypeSize.Bazaar; i++) {
				if ((newgis->items[i] == id_of_item_to_add) && (newgis->charges[i] != charges_on_item_to_add)) {
					same_item_with_differing_charges = true;
					break;
				}
			}

			if (same_item_with_differing_charges) {
				Message(
					Chat::Red,
					"Warning: You have more than one %s with different charges. They have all been added for sale "
					"at the same price.",
					item->Name
				);
			}
		}

		// Now put all Items with a matching ItemID up for trade.
		//
		for (int i = 0; i < GetInv().GetLookup()->InventoryTypeSize.Bazaar; i++) {
			if (newgis->items[i] == id_of_item_to_add) {
				auto item_detail = FindTraderItemBySerialNumber(newgis->serial_number[i]);

				TraderRepository::Trader trader_item{};
				trader_item.id             = 0;
				trader_item.char_entity_id = GetID();
				trader_item.char_id        = CharacterID();
				trader_item.char_zone_id   = GetZoneID();
				trader_item.item_charges   = newgis->charges[i];
				trader_item.item_cost      = tpus->NewPrice;
				trader_item.item_id        = newgis->items[i];
				trader_item.item_sn        = newgis->serial_number[i];
				if (item_detail->IsAugmented()) {
					auto augs              = item_detail->GetAugmentIDs();
					trader_item.aug_slot_1 = augs.at(0);
					trader_item.aug_slot_2 = augs.at(1);
					trader_item.aug_slot_3 = augs.at(2);
					trader_item.aug_slot_4 = augs.at(3);
					trader_item.aug_slot_5 = augs.at(4);
					trader_item.aug_slot_6 = augs.at(5);
				}
				trader_item.slot_id = i;

				TraderRepository::ReplaceOne(database, trader_item);

				trader_items.push_back(trader_item);

				LogTrading(
					"Adding new item for <green>[{}] ItemID <green>[{}] SerialNumber <green>[{}] Charges <green>[{}] "
					"Price: <green>[{}] Slot <green>[{}]",
					GetName(),
					newgis->items[i],
					newgis->serial_number[i],
					newgis->charges[i],
					tpus->NewPrice,
					i
				);
			}
		}

		// If we have a customer currently browsing, update them with the new items.
		//
		if (GetCustomerID()) {
			UpdateTraderCustomerItemsAdded(
				GetCustomerID(),
				trader_items,
				id_of_item_to_add,
				GetInv().GetLookup()->InventoryTypeSize.Bazaar
			);
		}

		safe_delete(newgis);

		// Acknowledge to the client.
		tpus->SubAction = BazaarPriceChange_AddItem;
		QueuePacket(app);

		return;
	}

	// This is a safeguard against a Trader increasing the price of an item while a customer is browsing and
	// unwittingly buying it at a higher price than they were expecting to.
	//
	if ((old_price != 0) && (tpus->NewPrice > old_price) && GetCustomerID()) {
		tpus->SubAction = BazaarPriceChange_Fail;
		QueuePacket(app);
		TraderEndTrader();
		Message(
			Chat::Red,
			"You must remove the item from sale before you can increase the price while a customer is browsing."
		);
		Message(Chat::Red, "Click 'Begin Trader' to restart Trader mode with the increased price for this item.");
		return;
	}

	// Send Acknowledgement back to the client.
	if (old_price == 0) {
		tpus->SubAction = BazaarPriceChange_AddItem;
	}
	else if (tpus->NewPrice != 0) {
		tpus->SubAction = BazaarPriceChange_UpdatePrice;
	}
	else {
		tpus->SubAction = BazaarPriceChange_RemoveItem;
	}

	QueuePacket(app);

	if (old_price == tpus->NewPrice) {
		LogTrading("The new price is the same as the old one");
		return;
	}
	// Update the price for all items we have for sale that have this ItemID and number of charges, or remove
	// them from the trader table if the new price is zero.
	//
	database.UpdateTraderItemPrice(CharacterID(), id_of_item_to_update, charges_on_item_to_update, tpus->NewPrice);

	// If a customer is browsing our goods, send them the updated prices / remove the items from the Merchant window
	if (GetCustomerID()) {
		UpdateTraderCustomerPriceChanged(
			GetCustomerID(),
			trader_items,
			id_of_item_to_update,
			charges_on_item_to_update,
			tpus->NewPrice,
			item_limit
		);
	}
}

void Client::SendBazaarDone(uint32 trader_id)
{
	auto outapp2 = new EQApplicationPacket(OP_BazaarSearch, sizeof(BazaarReturnDone_Struct));
	auto brds    = (BazaarReturnDone_Struct *) outapp2->pBuffer;

	brds->TraderID   = trader_id;
	brds->Type       = BazaarSearchDone;
	brds->Unknown008 = 0xFFFFFFFF;
	brds->Unknown012 = 0xFFFFFFFF;
	brds->Unknown016 = 0xFFFFFFFF;

	QueuePacket(outapp2);
	safe_delete(outapp2);
}

void Client::SendBulkBazaarTraders()
{
	if (ClientVersion() < EQ::versions::ClientVersion::RoF2) {
		return;
	}

	auto  results = TraderRepository::GetDistinctTraders(database);
	auto  p_size  = 4 + 12 * results.count + results.name_length;
	auto  buffer  = std::make_unique<char[]>(p_size);
	memset(buffer.get(), 0, p_size);
	char *bufptr  = buffer.get();

	VARSTRUCT_ENCODE_TYPE(uint32, bufptr, results.count);

	for (auto t : results.traders) {
		VARSTRUCT_ENCODE_TYPE(uint32, bufptr, t.zone_id);
		VARSTRUCT_ENCODE_TYPE(uint32, bufptr, t.trader_id);
		VARSTRUCT_ENCODE_TYPE(uint32, bufptr, t.entity_id);
		VARSTRUCT_ENCODE_STRING(bufptr, t.trader_name.c_str());
	}

	auto outapp = std::make_unique<EQApplicationPacket>(OP_TraderBulkSend, p_size);
	memcpy(outapp->pBuffer, buffer.get(), p_size);

	QueuePacket(outapp.get());
}

void Client::DoBazaarInspect(const BazaarInspect_Struct &in)
{
	auto items = TraderRepository::GetWhere(database, fmt::format("item_sn = {}", in.serial_number));
	if (items.empty()) {
		LogInfo("Failed to find item with serial number [{}]", in.serial_number);
		return;
	}

	auto &item = items.front();

	std::unique_ptr<EQ::ItemInstance> inst(
		database.CreateItem(
			item.item_id,
			item.item_charges,
			item.aug_slot_1,
			item.aug_slot_2,
			item.aug_slot_3,
			item.aug_slot_4,
			item.aug_slot_5,
			item.aug_slot_6
		)
	);

	if (inst) {
		SendItemPacket(0, inst.get(), ItemPacketViewLink);
	}
}

void Client::SendBazaarDeliveryCosts()
{
	auto outapp = std::make_unique<EQApplicationPacket>(OP_BazaarSearch, sizeof(BazaarDeliveryCost_Struct));
	auto data   = (BazaarDeliveryCost_Struct *) outapp->pBuffer;

	data->action                = DeliveryCostUpdate;
	data->voucher_delivery_cost = RuleI(Bazaar, VoucherDeliveryCost);
	data->parcel_deliver_cost   = RuleR(Bazaar, ParcelDeliveryCostMod);

	QueuePacket(outapp.get());
}

std::string Client::DetermineMoneyString(uint64 cp)
{
	uint32 plat   = cp / 1000;
	uint32 gold   = (cp - plat * 1000) / 100;
	uint32 silver = (cp - plat * 1000 - gold * 100) / 10;
	uint32 copper = (cp - plat * 1000 - gold * 100 - silver * 10);

	if (!plat && !gold && !silver && !copper) {
		return std::string("No Money");
	}

	std::string money {};
	if (plat) {
		money += fmt::format("{}p ", plat);
	}
	if (gold) {
		money += fmt::format("{}g ", gold);
	}
	if (silver) {
		money += fmt::format("{}s ", silver);
	}
	if (copper) {
		money += fmt::format("{}c", copper);
	}

	return fmt::format("{}", money);
}

void Client::BuyTraderItemOutsideBazaar(TraderBuy_Struct *tbs, const EQApplicationPacket *app)
{
	auto in          = (TraderBuy_Struct *) app->pBuffer;
	auto trader_item = TraderRepository::GetItemBySerialNumber(database, tbs->serial_number);
	if (!trader_item.id) {
		LogTrading("Attempt to purchase an item outside of the Bazaar trader_id <red>[{}] item serial_number "
				   "<red>[{}] The Traders data was outdated.",
				   tbs->trader_id,
				   tbs->serial_number
		);
		in->method     = BazaarByParcel;
		in->sub_action = DataOutDated;
		TradeRequestFailed(app);
		return;
	}

	if (trader_item.active_transaction) {
		LogTrading("Attempt to purchase an item outside of the Bazaar trader_id <red>[{}] item serial_number "
				   "<red>[{}] The item is already within an active transaction.",
				   tbs->trader_id,
				   tbs->serial_number
		);
		in->method     = BazaarByParcel;
		in->sub_action = DataOutDated;
		TradeRequestFailed(app);
		return;
	}

	TraderRepository::UpdateActiveTransaction(database, trader_item.id, true);

	std::unique_ptr<EQ::ItemInstance> buy_item(
		database.CreateItem(
			trader_item.item_id,
			trader_item.item_charges,
			trader_item.aug_slot_1,
			trader_item.aug_slot_2,
			trader_item.aug_slot_3,
			trader_item.aug_slot_4,
			trader_item.aug_slot_5,
			trader_item.aug_slot_6
		)
	);

	if (!buy_item) {
		LogTrading("Unable to find item id <red>[{}] item_sn <red>[{}] on trader",
				   trader_item.item_id,
				   trader_item.item_sn
		);
		in->method     = BazaarByParcel;
		in->sub_action = Failed;
		TraderRepository::UpdateActiveTransaction(database, trader_item.id, false);
		TradeRequestFailed(app);
		return;
	}

	LogTrading(
		"Name: <green>[{}] IsStackable: <green>[{}] Requested Quantity: <green>[{}] Charges on Item <green>[{}]",
		buy_item->GetItem()->Name,
		buy_item->IsStackable(),
		tbs->quantity,
		buy_item->GetCharges()
	);

	// Determine the actual quantity for the purchase
	if (!buy_item->IsStackable()) {
		tbs->quantity = 1;
	}
	else {
		int32 item_charges = buy_item->GetCharges();
		if (item_charges <= 0) {
			tbs->quantity = 1;
		}
		else if (static_cast<uint32>(item_charges) < tbs->quantity) {
			tbs->quantity = item_charges;
		}
	}

	LogTrading("Actual quantity that will be traded is <green>[{}]", tbs->quantity);

	uint64 total_transaction_value = static_cast<uint64>(tbs->price) * static_cast<uint64>(tbs->quantity);
	if (total_transaction_value > MAX_TRANSACTION_VALUE) {
		Message(
			Chat::Red,
			"That would exceed the single transaction limit of %u platinum.",
			MAX_TRANSACTION_VALUE / 1000
		);
		TraderRepository::UpdateActiveTransaction(database, trader_item.id, false);
		TradeRequestFailed(app);
		return;
	}

	uint32 total_cost = tbs->price * tbs->quantity;
	uint32 fee        = static_cast<uint32>(std::round((uint32) total_cost * RuleR(Bazaar, ParcelDeliveryCostMod)));
	if (!TakeMoneyFromPP(total_cost + fee)) {
		RecordPlayerEventLog(
			PlayerEvent::POSSIBLE_HACK,
			PlayerEvent::PossibleHackEvent{
				.message = fmt::format(
					"{} attempted to buy {} in bazaar but did not have enough money.",
					GetCleanName(),
					buy_item->GetItem()->Name
				)
			}
		);
		in->method     = BazaarByParcel;
		in->sub_action = InsufficientFunds;
		TraderRepository::UpdateActiveTransaction(database, trader_item.id, false);
		TradeRequestFailed(app);
		return;
	}

	Message(Chat::Red, fmt::format("You paid {} for the parcel delivery.", DetermineMoneyString(fee)).c_str());
	LogTrading("Customer <green>[{}] Paid: <green>[{}] in Copper", CharacterID(), total_cost);

	if (player_event_logs.IsEventEnabled(PlayerEvent::TRADER_PURCHASE)) {
		auto e = PlayerEvent::TraderPurchaseEvent{
			.item_id              = buy_item->GetID(),
			.item_name            = buy_item->GetItem()->Name,
			.trader_id            = tbs->trader_id,
			.trader_name          = tbs->seller_name,
			.price                = tbs->price,
			.charges              = tbs->quantity,
			.total_cost           = total_cost,
			.player_money_balance = GetCarriedMoney(),
		};

		RecordPlayerEventLog(PlayerEvent::TRADER_PURCHASE, e);
	}

	CharacterParcelsRepository::CharacterParcels parcel_out{};
	auto next_slot = FindNextFreeParcelSlot(CharacterID());
	if (next_slot == INVALID_INDEX) {
		LogTrading(
			"{} attempted to purchase {} from the bazaar with parcel delivery.  Unfortunately their parcel limit was reached.  "
			"Purchase unsuccessful.",
			GetCleanName(),
			buy_item->GetItem()->Name
		);
		in->method     = BazaarByParcel;
		in->sub_action = TooManyParcels;
		TraderRepository::UpdateActiveTransaction(database, trader_item.id, false);
		TradeRequestFailed(app);
		return;
	}
	parcel_out.from_name  = tbs->seller_name;
	parcel_out.note       = "Delivered from a Bazaar Purchase";
	parcel_out.sent_date  = time(nullptr);
	parcel_out.quantity   = buy_item->IsStackable() ? tbs->quantity : buy_item->GetCharges();
	parcel_out.item_id    = buy_item->GetItem()->ID;
	parcel_out.aug_slot_1 = buy_item->GetAugmentItemID(0);
	parcel_out.aug_slot_2 = buy_item->GetAugmentItemID(1);
	parcel_out.aug_slot_3 = buy_item->GetAugmentItemID(2);
	parcel_out.aug_slot_4 = buy_item->GetAugmentItemID(3);
	parcel_out.aug_slot_5 = buy_item->GetAugmentItemID(4);
	parcel_out.aug_slot_6 = buy_item->GetAugmentItemID(5);
	parcel_out.char_id    = CharacterID();
	parcel_out.slot_id    = next_slot;
	parcel_out.id         = 0;

	auto result = CharacterParcelsRepository::InsertOne(database, parcel_out);
	if (!result.id) {
		LogError("Failed to add parcel to database.  From {} to {} item {} quantity {}",
				 parcel_out.from_name,
				 GetCleanName(),
				 parcel_out.item_id,
				 parcel_out.quantity
		);
		Message(Chat::Yellow, "Unable to save parcel to the database. Please contact an administrator.");
		in->method     = BazaarByParcel;
		in->sub_action = Failed;
		TraderRepository::UpdateActiveTransaction(database, trader_item.id, false);
		TradeRequestFailed(app);
		return;
	}

	ReturnTraderReq(app, tbs->quantity, buy_item->GetID());
	if (player_event_logs.IsEventEnabled(PlayerEvent::PARCEL_SEND)) {
		PlayerEvent::ParcelSend e{};
		e.from_player_name = parcel_out.from_name;
		e.to_player_name   = GetCleanName();
		e.item_id          = parcel_out.item_id;
		e.aug_slot_1       = parcel_out.aug_slot_1;
		e.aug_slot_2       = parcel_out.aug_slot_2;
		e.aug_slot_3       = parcel_out.aug_slot_3;
		e.aug_slot_4       = parcel_out.aug_slot_4;
		e.aug_slot_5       = parcel_out.aug_slot_5;
		e.aug_slot_6       = parcel_out.aug_slot_6;
		e.quantity         = parcel_out.quantity;
		e.sent_date        = parcel_out.sent_date;

		RecordPlayerEventLog(PlayerEvent::PARCEL_SEND, e);
	}

	Parcel_Struct ps{};
	ps.item_slot = parcel_out.slot_id;
	strn0cpy(ps.send_to, GetCleanName(), sizeof(ps.send_to));

	if (trader_item.item_charges == tbs->quantity) {
		TraderRepository::DeleteOne(database, trader_item.id);
	} else {
		TraderRepository::UpdateQuantity(
			database,
			trader_item.char_id,
			trader_item.item_sn,
			trader_item.item_charges - tbs->quantity
		);
	}

	SendParcelDeliveryToWorld(ps);

	if (RuleB(Bazaar, AuditTrail)) {
		BazaarAuditTrail(tbs->seller_name, GetName(), buy_item->GetItem()->Name, tbs->quantity, tbs->price, 0);
	}

	auto out_server = std::make_unique<ServerPacket>(ServerOP_BazaarPurchase, sizeof(BazaarPurchaseMessaging_Struct));
	auto out_data   = (BazaarPurchaseMessaging_Struct *) out_server->pBuffer;

	out_data->trader_buy_struct       = *tbs;
	out_data->buyer_id                = CharacterID();
	out_data->item_aug_1              = buy_item->GetAugmentItemID(0);
	out_data->item_aug_2              = buy_item->GetAugmentItemID(1);
	out_data->item_aug_3              = buy_item->GetAugmentItemID(2);
	out_data->item_aug_4              = buy_item->GetAugmentItemID(3);
	out_data->item_aug_5              = buy_item->GetAugmentItemID(4);
	out_data->item_aug_6              = buy_item->GetAugmentItemID(5);
	out_data->item_quantity_available = trader_item.item_charges;
	out_data->id                      = trader_item.id;
	strn0cpy(out_data->trader_buy_struct.buyer_name, GetCleanName(), sizeof(out_data->trader_buy_struct.buyer_name));

	worldserver.SendPacket(out_server.release());
}

void Client::SetBuyerWelcomeMessage(const char *welcome_message)
{
	BuyerRepository::UpdateWelcomeMessage(database, CharacterID(), welcome_message);
}

void Client::SendBuyerGreeting(uint32 buyer_id)
{
	auto buyer = BuyerRepository::GetWhere(database, fmt::format("`char_id` = '{}'", buyer_id));
	if (buyer.empty()) {
		Message(Chat::White, "Welcome!");
		return;
	}
	Message(Chat::White, buyer.front().welcome_message.c_str());
}

void Client::SendSellerBrowsing(const std::string &browser)
{
	auto outapp = std::make_unique<EQApplicationPacket>(OP_Barter, sizeof(BuyerBrowsing_Struct));
	auto eq     = (BuyerBrowsing_Struct *) outapp->pBuffer;

	eq->action = Barter_SellerBrowsing;
	strn0cpy(eq->char_name, browser.c_str(), sizeof(eq->char_name));

	QueuePacket(outapp.get());
}

void Client::SendBuyerMode(bool status)
{
	auto outapp = std::make_unique<EQApplicationPacket>(OP_Barter, 4);
	auto emu    = (BuyerGeneric_Struct *) outapp->pBuffer;

	emu->action = status ? Barter_BuyerModeOn : Barter_BuyerModeOff;

	QueuePacket(outapp.get());
}

bool Client::IsInBuyerSpace()
{
#define BUYER_DOOR_ARC_RADIUS_HIGH    91
#define BUYER_DOOR_ARC_RADIUS_LOW     71
#define BUYER_DOOR_OPEN_TYPE         155
#define TRADER_DOOR_OPEN_TYPE        153

	struct BuyerDoorDataStruct {
		uint32 door_id;
		uint32 arc_offset;
	};

	std::vector<BuyerDoorDataStruct> buyer_door_data = {
		{.door_id = 2}, {.arc_offset = 90},{.door_id = 3} ,{.arc_offset = 0} ,{.door_id = 4},  {.arc_offset = 0},
		{.door_id = 5}, {.arc_offset = 0} ,{.door_id = 6} ,{.arc_offset = 90},{.door_id = 7},  {.arc_offset = 0},
		{.door_id = 8}, {.arc_offset = 0} ,{.door_id = 9} ,{.arc_offset = 0} ,{.door_id = 10}, {.arc_offset = 0},
		{.door_id = 11},{.arc_offset = 0} ,{.door_id = 12},{.arc_offset = 0} ,{.door_id = 13}, {.arc_offset = 0},
		{.door_id = 14},{.arc_offset = 0} ,{.door_id = 15},{.arc_offset = 0} ,{.door_id = 16}, {.arc_offset = 90},
		{.door_id = 17},{.arc_offset = 0} ,{.door_id = 18},{.arc_offset = 0} ,{.door_id = 19}, {.arc_offset = 0},
		{.door_id = 20},{.arc_offset = 0} ,{.door_id = 21},{.arc_offset = 0} ,{.door_id = 22}, {.arc_offset = 0},
		{.door_id = 23},{.arc_offset = 0} ,{.door_id = 24},{.arc_offset = 0} ,{.door_id = 25}, {.arc_offset = 0},
		{.door_id = 26},{.arc_offset = 0} ,{.door_id = 27},{.arc_offset = 0} ,{.door_id = 28}, {.arc_offset = 0},
		{.door_id = 29},{.arc_offset = 90},{.door_id = 30},{.arc_offset = 0} ,{.door_id = 31}, {.arc_offset = 0},
		{.door_id = 32},{.arc_offset = 0} ,{.door_id = 33},{.arc_offset = 0} ,{.door_id = 34}, {.arc_offset = 0},
		{.door_id = 35},{.arc_offset = 0} ,{.door_id = 36},{.arc_offset = 90},{.door_id = 37}, {.arc_offset = 0},
		{.door_id = 38},{.arc_offset = 0} ,{.door_id = 39},{.arc_offset = 0} ,{.door_id = 40}, {.arc_offset = 0},
		{.door_id = 41},{.arc_offset = 0} ,{.door_id = 42},{.arc_offset = 0} ,{.door_id = 43}, {.arc_offset = 90},
		{.door_id = 44},{.arc_offset = 0} ,{.door_id = 45},{.arc_offset = 0} ,{.door_id = 46}, {.arc_offset = 0},
		{.door_id = 47},{.arc_offset = 0} ,{.door_id = 48},{.arc_offset = 0} ,{.door_id = 49}, {.arc_offset = 0},
		{.door_id = 50},{.arc_offset = 90},{.door_id = 51},{.arc_offset = 90},{.door_id = 52}, {.arc_offset = 0},
		{.door_id = 53},{.arc_offset = 0} ,{.door_id = 54},{.arc_offset = 0}, {.door_id = 55}, {.arc_offset = 0},
		{.door_id = 56},{.arc_offset = 0} ,{.door_id = 57},{.arc_offset = 0}, {.door_id = 122},{.arc_offset = 0}
	};

	auto m_location = GetPosition();

	for (auto const &d: buyer_door_data) {
		auto door = entity_list.GetDoorsByDoorID(d.door_id);
		if (door && IsWithinCircularArc(
			door->GetPosition(),
			m_location,
			d.arc_offset,
			BUYER_DOOR_ARC_RADIUS_HIGH,
			BUYER_DOOR_ARC_RADIUS_LOW
		)
			) {
			return true;
		}
	}

	for (auto const& d:entity_list.GetDoorsList()) {
		if (d.second->GetOpenType() == DoorType::BuyerStall) {
			if (IsWithinSquare(d.second->GetPosition(), d.second->GetSize(), GetPosition())) {
				return true;
			}
		}
	}

	return false;
}

void Client::CreateStartingBuyLines(const EQApplicationPacket *app)
{
	if (ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		BuyerBuyLines_Struct         bl{};
		auto                         in = (BuyerGeneric_Struct *) app->pBuffer;
		EQ::Util::MemoryStreamReader ss_in(
			reinterpret_cast<char *>(in->payload),
			app->size - sizeof(BuyerGeneric_Struct));
		cereal::BinaryInputArchive   ar(ss_in);
		ar(bl);

		if (bl.buy_lines.empty()) {
			return;
		}

		std::map<uint32, BuylineItemDetails_Struct> item_map{};

		if (!BuildBuyLineMap(item_map, bl)) {
			ToggleBuyerMode(false);
			return;
		}

		auto proposed_total_cost = ValidateBuyLineCost(item_map);
		if (proposed_total_cost == 0) {
			ToggleBuyerMode(false);
			return;
		}

		if (!ValidateBuyLineItems(item_map)) {
			ToggleBuyerMode(false);
			return;
		}

		std::stringstream           ss_out{};
		cereal::BinaryOutputArchive ar_out(ss_out);

		for (auto &b: bl.buy_lines) {
			BuyerBuyLinesRepository::CreateBuyLine(database, b, CharacterID());

			{ ar_out(b); }

			uint32 packet_size = ss_out.str().length() + sizeof(BuyerGeneric_Struct);
			auto   out         = std::make_unique<EQApplicationPacket>(OP_BuyerItems, packet_size);
			auto   data        = (BazaarSearchMessaging_Struct *) out->pBuffer;

			data->action = Barter_BuyerItemUpdate;
			memcpy(data->payload, ss_out.str().data(), ss_out.str().length());
			QueuePacket(out.get());

			ss_out.str("");
			ss_out.clear();
		}

		Message(Chat::Yellow, fmt::format("{} buy lines enabled.", bl.buy_lines.size()).c_str());
	}
}

void Client::SendBuyLineUpdate(const BuyerLineItems_Struct &buy_line)
{
	std::stringstream           ss_out{};
	cereal::BinaryOutputArchive ar_out(ss_out);

	{ ar_out(buy_line); }

	uint32 packet_size = ss_out.str().length() + sizeof(BuyerGeneric_Struct);
	auto   out         = std::make_unique<EQApplicationPacket>(OP_BuyerItems, packet_size);
	auto   data        = (BazaarSearchMessaging_Struct *) out->pBuffer;

	data->action = Barter_BuyerItemUpdate;
	memcpy(data->payload, ss_out.str().data(), ss_out.str().length());
	QueuePacket(out.get());

	ss_out.str("");
	ss_out.clear();
}

void Client::CheckIfMovedItemIsPartOfBuyLines(uint32 item_id)
{
	auto b_trade_items = BuyerTradeItemsRepository::GetTradeItems(database, GetBuyerID());
	if (b_trade_items.empty()) {
		return;
	}

	auto it = std::find_if(
		b_trade_items.cbegin(),
		b_trade_items.cend(),
		[&](const BaseBuyerTradeItemsRepository::BuyerTradeItems bti) {
			return bti.item_id == item_id;
		}
	);
	if (it != std::end(b_trade_items)) {
		auto item = GetInv().GetItem(GetInv().HasItem(item_id, 1, invWherePersonal));
		if (!item) {
			return;
		}

		Message(
			Chat::Red,
			fmt::format(
				"You moved an item ({}) that is part of an active buy line.",
				item->GetItem()->Name
			).c_str()
		);
		ToggleBuyerMode(false);
	}
}

void Client::SendWindowUpdatesToSellerAndBuyer(BuyerLineSellItem_Struct &blsi)
{
	auto buyer  = entity_list.GetClientByID(blsi.buyer_entity_id);
	auto seller = this;
	if (!buyer || !seller) {
		return;
	}

	if (blsi.item_quantity - blsi.seller_quantity <= 0) {
		auto outapp = std::make_unique<EQApplicationPacket>(
			OP_BuyerItems,
			sizeof(BuyerRemoveItemFromMerchantWindow_Struct)
		);
		auto data   = (BuyerRemoveItemFromMerchantWindow_Struct *) outapp->pBuffer;

		data->action      = Barter_RemoveFromMerchantWindow;
		data->buy_slot_id = blsi.slot;
		QueuePacket(outapp.get());

		std::stringstream           ss{};
		cereal::BinaryOutputArchive ar(ss);

		BuyerLineItems_Struct bl{};
		bl.enabled       = 0;
		bl.item_cost     = blsi.item_cost;
		bl.item_icon     = blsi.item_icon;
		bl.item_id       = blsi.item_id;
		bl.item_quantity = blsi.item_quantity - blsi.seller_quantity;
		bl.item_name     = blsi.item_name;
		bl.item_toggle   = 0;
		bl.slot          = blsi.slot;

		for (auto const &b: blsi.trade_items) {
			BuyerLineTradeItems_Struct blti{};
			blti.item_icon     = b.item_icon;
			blti.item_id       = b.item_id;
			blti.item_quantity = b.item_quantity;
			blti.item_name     = b.item_name;
			bl.trade_items.push_back(blti);
		}

		{ ar(bl); }

		uint32 packet_size = ss.str().length() + sizeof(BuyerGeneric_Struct);
		outapp = std::make_unique<EQApplicationPacket>(OP_BuyerItems, packet_size);
		auto emu = (BuyerGeneric_Struct *) outapp->pBuffer;

		emu->action = Barter_BuyerItemUpdate;
		memcpy(emu->payload, ss.str().data(), ss.str().length());

		buyer->QueuePacket(outapp.get());
		BuyerBuyLinesRepository::DeleteBuyLine(database, buyer->CharacterID(), blsi.slot);
	}
	else {
		std::stringstream           ss{};
		cereal::BinaryOutputArchive ar(ss);

		BuyerLineItems_Struct bli{};
		bli.enabled       = 1;
		bli.item_cost     = blsi.item_cost;
		bli.item_icon     = blsi.item_icon;
		bli.item_id       = blsi.item_id;
		bli.item_quantity = blsi.item_quantity - blsi.seller_quantity;
		bli.item_toggle   = 1;
		bli.slot          = blsi.slot;
		bli.item_name     = blsi.item_name;
		for (auto const &b: blsi.trade_items) {
			BuyerLineTradeItems_Struct blti{};
			blti.item_id       = b.item_id;
			blti.item_icon     = b.item_icon;
			blti.item_quantity = b.item_quantity;
			blti.item_name     = b.item_name;
			bli.trade_items.push_back(blti);
		}
		{ ar(bli); }

		uint32 packet_size = ss.str().length() + sizeof(BuyerGeneric_Struct);
		auto   outapp      = std::make_unique<EQApplicationPacket>(OP_BuyerItems, packet_size);
		auto   emu         = (BuyerGeneric_Struct *) outapp->pBuffer;

		emu->action = Barter_BuyerInspectBegin;
		memcpy(emu->payload, ss.str().data(), ss.str().length());

		QueuePacket(outapp.get());

		outapp = std::make_unique<EQApplicationPacket>(OP_BuyerItems, packet_size);
		emu    = (BuyerGeneric_Struct *) outapp->pBuffer;

		emu->action = Barter_BuyerItemUpdate;
		memcpy(emu->payload, ss.str().data(), ss.str().length());

		buyer->QueuePacket(outapp.get());

		BuyerBuyLinesRepository::ModifyBuyLine(database, bli, buyer->GetBuyerID());
	}
}

void Client::SendBuyerToBarterWindow(Client *buyer, uint32 action)
{
	auto server_packet = std::make_unique<ServerPacket>(
		ServerOP_BuyerMessaging,
		sizeof(BuyerMessaging_Struct)
	);
	auto data          = (BuyerMessaging_Struct *) server_packet->pBuffer;

	data->action          = action;
	data->zone_id         = buyer->GetZoneID();
	data->buyer_id        = buyer->GetBuyerID();
	data->buyer_entity_id = buyer->GetID();
	strn0cpy(data->buyer_name, buyer->GetCleanName(), sizeof(data->buyer_name));

	worldserver.SendPacket(server_packet.get());
}

void Client::SendBulkBazaarBuyers()
{
	auto results = BuyerRepository::All(database);

	if (results.empty()) {
		return;
	}

	auto outapp = std::make_unique<EQApplicationPacket>(OP_Barter, sizeof(BuyerAddBuyertoBarterWindow_Struct));
	auto emu    = (BuyerAddBuyertoBarterWindow_Struct *) outapp->pBuffer;

	for (auto const &b: results) {
		auto buyer = entity_list.GetClientByCharID(b.char_id);
		emu->action          = Barter_AddToBarterWindow;
		emu->buyer_id        = b.char_id;
		emu->buyer_entity_id = buyer ? buyer->GetID() : 0;
		emu->zone_id         = buyer ? buyer->GetZoneID() : 0;
		strn0cpy(emu->buyer_name, b.char_name.c_str(), sizeof(emu->buyer_name));

		QueuePacket(outapp.get());
	}
}

void Client::SendBarterBuyerClientMessage(
	BuyerLineSellItem_Struct &blsi,
	BarterBuyerActions action,
	BarterBuyerSubActions sub_action,
	BarterBuyerSubActions error_code
)
{
	std::stringstream           ss{};
	cereal::BinaryOutputArchive ar(ss);

	blsi.sub_action = sub_action;
	blsi.error_code = error_code;

	{ ar(blsi); }

	uint32 packet_size = ss.str().length() + sizeof(BuyerGeneric_Struct);
	auto   outapp      = std::make_unique<EQApplicationPacket>(OP_BuyerItems, packet_size);
	auto   emu         = (BuyerGeneric_Struct *) outapp->pBuffer;

	emu->action = action;
	memcpy(emu->payload, ss.str().data(), ss.str().length());

	QueuePacket(outapp.get());
}

bool Client::BuildBuyLineMap(std::map<uint32, BuylineItemDetails_Struct> &item_map, BuyerBuyLines_Struct &bl)
{
	bool buyer_error = false;

	for (auto const &b: bl.buy_lines) {
		if (item_map.contains(b.item_id) && item_map[b.item_id].item_cost > 0) {
			Message(
				Chat::Red,
				fmt::format(
					"You cannot have two buy lines for the same item {}. Buy line not possible.",
					b.item_name
				).c_str()
			);
			buyer_error = true;
			break;
		}
		BuylineItemDetails_Struct t = {b.item_quantity * b.item_cost, b.item_quantity};
		item_map.emplace(b.item_id, t);
		for (auto const &i: b.trade_items) {
			if (item_map.contains(i.item_id) && item_map[i.item_id].item_cost > 0) {
				Message(
					Chat::Red,
					fmt::format(
						"You cannot buy {} and offer the same item as compensation. Buy line not possible.",
						i.item_name
					).c_str()
				);
				buyer_error = true;
				break;
			}
			if (item_map.contains(i.item_id)) {
				item_map[i.item_id].item_quantity += i.item_quantity * b.item_quantity;
				continue;
			}
			t = {0, i.item_quantity * b.item_quantity};
			item_map.emplace(i.item_id, t);
		}
	}

	if (buyer_error) {
		return false;
	}

	return true;
}

bool Client::BuildBuyLineMapFromVector(
	std::map<uint32, BuylineItemDetails_Struct> &item_map,
	std::vector<BuyerLineItems_Struct> &bl
)
{

	bool buyer_error = false;

	for (auto const &b: bl) {
		if (item_map.contains(b.item_id) && item_map[b.item_id].item_cost > 0) {
			Message(
				Chat::Red,
				fmt::format(
					"You cannot have two buy lines for the same item {}. Buy line not possible.",
					b.item_name
				).c_str()
			);
			buyer_error = true;
			break;
		}
		BuylineItemDetails_Struct t = {b.item_quantity * b.item_cost, b.item_quantity};
		item_map.emplace(b.item_id, t);
		for (auto const &i: b.trade_items) {
			if (item_map.contains(i.item_id) && item_map[i.item_id].item_cost > 0) {
				Message(
					Chat::Red,
					fmt::format(
						"You cannot buy {} and offer the same item as compensation. Buy line not possible.",
						i.item_name
					).c_str()
				);
				buyer_error = true;
				break;
			}
			if (item_map.contains(i.item_id)) {
				item_map[i.item_id].item_quantity += i.item_quantity * b.item_quantity;
				continue;
			}
			t = {0, i.item_quantity * b.item_quantity};
			item_map.emplace(i.item_id, t);
		}
	}

	if (buyer_error) {
		return false;
	}

	return true;
}

void
Client::RemoveItemFromBuyLineMap(std::map<uint32, BuylineItemDetails_Struct> &item_map, const BuyerLineItems_Struct &bl)
{
	if (item_map.contains(bl.item_id) && item_map[bl.item_id].item_cost > 0) {
		item_map.erase(bl.item_id);
	}

	for (auto const &i: bl.trade_items) {
		if (item_map.contains(i.item_id) &&
			(item_map[i.item_id].item_quantity - (i.item_quantity * bl.item_quantity)) == 0) {
			item_map.erase(i.item_id);
		}
		else if (item_map.contains(i.item_id)) {
			item_map[i.item_id].item_quantity -= i.item_quantity * bl.item_quantity;
		}
	}
}

bool Client::ValidateBuyLineItems(std::map<uint32, BuylineItemDetails_Struct> &item_map)
{
	bool buyer_error = false;

	for (auto const &i: item_map) {
		auto item = database.GetItem(i.first);
		if (!item) {
			buyer_error = true;
			break;
		}

		if (i.second.item_cost > 0) {
			auto buy_item_slot_id = GetInv().HasItem(i.first, i.second.item_quantity, invWherePersonal);
			auto buy_item         = buy_item_slot_id == INVALID_INDEX ? nullptr : GetInv().GetItem(buy_item_slot_id);
			if (buy_item && CheckLoreConflict(buy_item->GetItem())) {
				Message(
					Chat::Red,
					fmt::format(
						"You already have a {}. Purchasing another will cause a lore conflict. Buy line not possible.",
						buy_item->GetItem()->Name
					).c_str()
				);
				buyer_error = true;
				break;
			}
		}
		if (i.second.item_cost == 0) {
			if (i.second.item_quantity > 1 && CheckLoreConflict(item)) {
				Message(
					Chat::Red,
					fmt::format(
						"Your buy line requires {} {}s however the item is LORE. Buy line not possible.",
						i.second.item_quantity,
						item->Name
					).c_str()
				);
				buyer_error = true;
				break;
			}

			auto buy_item_slot_id = GetInv().HasItem(i.first, i.second.item_quantity, invWherePersonal);
			auto buy_item         = buy_item_slot_id == INVALID_INDEX ? nullptr : GetInv().GetItem(buy_item_slot_id);

			if (!buy_item) {
				Message(
					Chat::Red,
					fmt::format(
						"Your buy line(s) require a total of {} {}{} which could not be found. Buy line not possible.",
						i.second.item_quantity,
						item->Name,
						i.second.item_quantity > 1 ? "s" : ""
					).c_str()
				);
				buyer_error = true;
				break;
			}

			if (buy_item->IsAugmentable() && buy_item->IsAugmented()) {
				Message(
					Chat::Red,
					fmt::format(
						"You cannot offer {} because it is augmented. Buy line not possible.",
						buy_item->GetItem()->Name
					).c_str()
				);
				buyer_error = true;
				break;
			}

			if (!buy_item->IsDroppable()) {
				Message(
					Chat::Red,
					fmt::format(
						"You cannot offer {} because it is NoTrade. Buy line not possible.",
						buy_item->GetItem()->Name
					).c_str());
				buyer_error = true;
				break;
			}

			buyer_error = false;
		}
	}

	return !buyer_error;
}

int64 Client::ValidateBuyLineCost(std::map<uint32, BuylineItemDetails_Struct> &item_map)
{
	int64 proposed_total_cost = std::accumulate(
		item_map.cbegin(),
		item_map.cend(),
		0,
		[](auto prev_sum, const std::pair<uint32, BuylineItemDetails_Struct> &x) {
			return prev_sum + x.second.item_cost;
		}
	);

	if (proposed_total_cost > GetCarriedMoney()) {
		Message(
			Chat::Red,
			fmt::format(
				"You currently do not have sufficient funds to support your buy lines. You have {} and need {}",
				DetermineMoneyString(GetCarriedMoney()),
				DetermineMoneyString(proposed_total_cost)).c_str()
		);
		return 0;
	}

	return proposed_total_cost;
}

bool Client::DoBarterBuyerChecks(BuyerLineSellItem_Struct &sell_line)
{
	bool buyer_error = false;
	auto buyer       = entity_list.GetClientByID(sell_line.buyer_entity_id);

	if (!buyer) {
		return false;
	}

	auto buyer_time = BuyerRepository::GetTransactionDate(database, buyer->CharacterID());
	if (buyer_time > GetBarterTime()) {
		if (sell_line.purchase_method == BarterByVendor) {
			SendBarterBuyerClientMessage(
				sell_line,
				Barter_SellerTransactionComplete,
				Barter_Success,
				Barter_DataOutOfDate
			);
			return false;
		}
		SendBarterBuyerClientMessage(sell_line, Barter_SellerTransactionComplete, Barter_Failure, Barter_DataOutOfDate);
		return false;
	}

	for (auto const &ti: sell_line.trade_items) {
		auto ti_slot_id = buyer->GetInv().HasItem(
			ti.item_id,
			ti.item_quantity * sell_line.seller_quantity,
			invWherePersonal
		);
		if (ti_slot_id == INVALID_INDEX) {
			LogTradingDetail(
				"Seller attempting to sell item <green>[{}] to buyer <green>[{}] though buyer no longer has compensation item <red>[{}]",
				sell_line.item_name,
				buyer->GetCleanName(),
				ti.item_name
			);
			buyer->Message(
				Chat::Red,
				fmt::format(
					"{} wanted to sell you {} however you no longer have compensation item {}",
					sell_line.seller_name,
					sell_line.item_name,
					ti.item_name
				).c_str());
			buyer_error = true;
			break;
		}
	}

	uint64 total_cost = (uint64) sell_line.item_cost * (uint64) sell_line.seller_quantity;
	if (!buyer->HasMoney(total_cost)) {
		LogTradingDetail(
			"Seller attempting to sell item <green>[{}] to buyer <green>[{}] though buyer does not have enough money <red>[{}]",
			sell_line.item_name,
			buyer->GetCleanName(),
			total_cost
		);
		buyer->Message(
			Chat::Red,
			fmt::format(
				"{} wanted to sell you {} however you have insufficient funds.",
				sell_line.seller_name,
				sell_line.item_name
			).c_str()
		);
		buyer_error = true;
	}

	auto buy_item_slot_id = buyer->GetInv().HasItem(
		sell_line.item_id,
		sell_line.seller_quantity,
		invWherePersonal
	);
	auto buy_item         = buy_item_slot_id == INVALID_INDEX ? nullptr : buyer->GetInv().GetItem(buy_item_slot_id);
	if (buy_item && buyer->CheckLoreConflict(buy_item->GetItem())) {
		LogTradingDetail(
			"Seller attempting to sell item <green>[{}] to buyer <green>[{}] though buyer already has the item which is LORE.",
			sell_line.item_name,
			buyer->GetCleanName()
		);
		buyer->Message(
			Chat::Red,
			fmt::format(
				"{} wanted to sell you {} however you already have the LORE item.",
				sell_line.seller_name,
				sell_line.item_name
			).c_str()
		);
		buyer_error = true;
	}

	if (buyer_error) {
		LogTradingDetail("Buyer error <red>[{}] Barter Sell/Buy Transaction Failed.", buyer_error);
		SendBarterBuyerClientMessage(sell_line, Barter_SellerTransactionComplete, Barter_Failure, Barter_Failure);
		return false;
	}

	return true;
}

bool Client::DoBarterSellerChecks(BuyerLineSellItem_Struct &sell_line)
{
	bool seller_error = false;
	auto sell_item_slot_id = GetInv().HasItem(sell_line.item_id, sell_line.seller_quantity, invWherePersonal);
	auto sell_item = sell_item_slot_id == INVALID_INDEX ? nullptr : GetInv().GetItem(sell_item_slot_id);
	if (!sell_item) {
		seller_error = true;
		LogTradingDetail("Seller no longer has item <red>[{}] to sell to buyer <red>[{}]",
						 sell_line.item_name,
						 sell_line.buyer_name
		);
		SendBarterBuyerClientMessage(
			sell_line,
			Barter_SellerTransactionComplete,
			Barter_Failure,
			Barter_SellerDoesNotHaveItem
		);
	}

	if (sell_item && sell_item->IsAugmentable() && sell_item->IsAugmented()) {
		seller_error = true;
		LogTradingDetail("Seller item <red>[{}] is augmented therefore cannot be sold.",
						 sell_line.item_name
		);
		Message(Chat::Red, "The item that you are trying to sell is augmented. Please remove augments first");
	}

	if (seller_error) {
		LogTradingDetail("Seller Error <red>[{}]  Barter Sell/Buy Transaction Failed.", seller_error);
		SendBarterBuyerClientMessage(sell_line, Barter_SellerTransactionComplete, Barter_Failure, Barter_Failure);
		return false;
	}

	return true;
}