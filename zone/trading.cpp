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

#include "client.h"
#include "entity.h"
#include "mob.h"

#include "quest_parser_collection.h"
#include "string_ids.h"
#include "worldserver.h"
#include "../common/bazaar.h"

class QueryServ;

extern WorldServer worldserver;
extern QueryServ* QServ;

// The maximum amount of a single bazaar/barter transaction expressed in copper.
// Equivalent to 2 Million plat
#define MAX_TRANSACTION_VALUE 2000000000
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
	for (auto const &i: inv->serial_number) {
		auto inst = FindTraderItemBySerialNumber(i);
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
		auto inst = FindTraderItemBySerialNumber(inv->serial_number[i]);
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
		else if (!in->serial_number[i]) {
			break;
		}
	}

	if (!trade_items_valid) {
		Message(Chat::Red, "You are not able to become a trader at this time.");
		TraderEndTrader();
		safe_delete(inv);
		return;
	}

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
	auto                   gis     = new GetItems_Struct;
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
					gis->items[ndx]         = item->GetItem()->ID;
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

	for (auto row = results.begin(); row != results.end(); ++row) {
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

void Client::SendBuyerResults(char* searchString, uint32 searchID) {

	// This method is called when a potential seller in the /barter window searches for matching buyers
	//
	LogDebug("[CLIENT] Client::SendBuyerResults [{}]\n", searchString);

	auto escSearchString = new char[strlen(searchString) * 2 + 1];
	database.DoEscapeString(escSearchString, searchString, strlen(searchString));

	std::string query = StringFormat("SELECT * FROM buyer WHERE itemname LIKE '%%%s%%' ORDER BY charid LIMIT %i",
							escSearchString, RuleI(Bazaar, MaxBarterSearchResults));
	safe_delete_array(escSearchString);
	auto results = database.QueryDatabase(query);
    if (!results.Success()) {
        return;
    }

    int numberOfRows = results.RowCount();

    if(numberOfRows == RuleI(Bazaar, MaxBarterSearchResults))
        Message(Chat::Yellow, "Your search found too many results; some are not displayed.");
    else if(strlen(searchString) == 0)
        Message(Chat::NPCQuestSay, "There are %i Buy Lines.", numberOfRows);
    else
        Message(Chat::NPCQuestSay, "There are %i Buy Lines that match the search string '%s'.", numberOfRows, searchString);

    if(numberOfRows == 0)
        return;

    uint32 lastCharID = 0;
	Client *buyer = nullptr;

	for (auto row = results.begin(); row != results.end(); ++row) {
        char itemName[64];

        uint32 charID = Strings::ToInt(row[0]);
		uint32 buySlot = Strings::ToInt(row[1]);
		uint32 itemID = Strings::ToInt(row[2]);
		strcpy(itemName, row[3]);
		uint32 quantity = Strings::ToInt(row[4]);
		uint32 price = Strings::ToInt(row[5]);

        // Each item in the search results is sent as a single fixed length packet, although the position of
		// the fields varies due to the use of variable length strings. The reason the packet is so big, is
		// to allow item compensation, e.g. a buyer could offer to buy a Blade Of Carnage for 10000pp plus
		// other items in exchange. Item compensation is not currently supported in EQEmu.
		//
		auto outapp = new EQApplicationPacket(OP_Barter, 940);

		char *buf = (char *)outapp->pBuffer;

		const EQ::ItemData* item = database.GetItem(itemID);

		if(!item) {
			safe_delete(outapp);
            continue;
		}

        // Save having to scan the client list when dealing with multiple buylines for the same Character.
		if(charID != lastCharID) {
			buyer = entity_list.GetClientByCharID(charID);
			lastCharID = charID;
		}

		if(!buyer) {
			safe_delete(outapp);
            continue;
		}

        VARSTRUCT_ENCODE_TYPE(uint32, buf, Barter_BuyerSearchResults);	// Command
		VARSTRUCT_ENCODE_TYPE(uint32, buf, searchID);			// Match up results with the request
		VARSTRUCT_ENCODE_TYPE(uint32, buf, buySlot);			// Slot in this Buyer's list
		VARSTRUCT_ENCODE_TYPE(uint8, buf, 0x01);				// Unknown - probably a flag field
		VARSTRUCT_ENCODE_TYPE(uint32, buf, itemID);			// ItemID
		VARSTRUCT_ENCODE_STRING(buf, itemName);			// Itemname
		VARSTRUCT_ENCODE_TYPE(uint32, buf, item->Icon);			// Icon
		VARSTRUCT_ENCODE_TYPE(uint32, buf, quantity);			// Quantity
		VARSTRUCT_ENCODE_TYPE(uint8, buf, 0x01);				// Unknown - probably a flag field
		VARSTRUCT_ENCODE_TYPE(uint32, buf, price);				// Price
		VARSTRUCT_ENCODE_TYPE(uint32, buf, buyer->GetID());		// Entity ID
		VARSTRUCT_ENCODE_TYPE(uint32, buf, 0);				// Flag for + Items , probably ItemCount
		VARSTRUCT_ENCODE_STRING(buf, buyer->GetName());		// Seller Name


		QueuePacket(outapp);
		safe_delete(outapp);
    }

}

void Client::ShowBuyLines(const EQApplicationPacket *app) {

	BuyerInspectRequest_Struct* bir = ( BuyerInspectRequest_Struct*)app->pBuffer;

	Client *Buyer = entity_list.GetClientByID(bir->BuyerID);

	if(!Buyer) {
		bir->Approval = 0; // Tell the client that the Buyer is unavailable
		QueuePacket(app);
		Message(Chat::Red, "The Buyer has gone away.");
		return;
	}

	bir->Approval = Buyer->WithCustomer(GetID());

	QueuePacket(app);

	if(bir->Approval == 0) {
		MessageString(Chat::Yellow, TRADER_BUSY);
		return;
	}

	const char *WelcomeMessagePointer = Buyer->GetBuyerWelcomeMessage();

	if(strlen(WelcomeMessagePointer) > 0)
		Message(Chat::NPCQuestSay, "%s greets you, '%s'.", Buyer->GetName(), WelcomeMessagePointer);

	auto outapp = new EQApplicationPacket(OP_Barter, sizeof(BuyerBrowsing_Struct));

	BuyerBrowsing_Struct* bb = (BuyerBrowsing_Struct*)outapp->pBuffer;

	// This packet produces the SoandSo is browsing your Buy Lines message
	bb->Action = Barter_SellerBrowsing;

	sprintf(bb->PlayerName, "%s", GetName());

	Buyer->QueuePacket(outapp);

	safe_delete(outapp);

    std::string query = StringFormat("SELECT * FROM buyer WHERE charid = %i", Buyer->CharacterID());
    auto results = database.QueryDatabase(query);
    if (!results.Success() || results.RowCount() == 0)
        return;

    for (auto row = results.begin(); row != results.end(); ++row) {
        char ItemName[64];
        uint32 BuySlot = Strings::ToInt(row[1]);
        uint32 ItemID = Strings::ToInt(row[2]);
		strcpy(ItemName, row[3]);
		uint32 Quantity = Strings::ToInt(row[4]);
		uint32 Price = Strings::ToInt(row[5]);

		auto outapp = new EQApplicationPacket(OP_Barter, 936);

		char *Buf = (char *)outapp->pBuffer;

		const EQ::ItemData* item = database.GetItem(ItemID);

		if(!item) {
			safe_delete(outapp);
            continue;
		}

        VARSTRUCT_ENCODE_TYPE(uint32, Buf, Barter_BuyerInspectWindow);
		VARSTRUCT_ENCODE_TYPE(uint32, Buf, BuySlot);
		VARSTRUCT_ENCODE_TYPE(uint8, Buf, 1);				// Flag
		VARSTRUCT_ENCODE_TYPE(uint32, Buf, ItemID);
		VARSTRUCT_ENCODE_STRING(Buf, ItemName);
		VARSTRUCT_ENCODE_TYPE(uint32, Buf, item->Icon);
		VARSTRUCT_ENCODE_TYPE(uint32, Buf, Quantity);
		VARSTRUCT_ENCODE_TYPE(uint8, Buf, 1);				// Flag
		VARSTRUCT_ENCODE_TYPE(uint32, Buf, Price);
		VARSTRUCT_ENCODE_TYPE(uint32, Buf, Buyer->GetID());
		VARSTRUCT_ENCODE_TYPE(uint32, Buf, 0);
		VARSTRUCT_ENCODE_STRING(Buf, Buyer->GetName());

		QueuePacket(outapp);
		safe_delete(outapp);
    }
}

void Client::SellToBuyer(const EQApplicationPacket *app) {

	char* Buf = (char *)app->pBuffer;

	char ItemName[64];

	/*uint32	Action		=*/ VARSTRUCT_SKIP_TYPE(uint32, Buf);	//unused
	uint32	Quantity	= VARSTRUCT_DECODE_TYPE(uint32, Buf);
	uint32	BuyerID		= VARSTRUCT_DECODE_TYPE(uint32, Buf);
	uint32	BuySlot		= VARSTRUCT_DECODE_TYPE(uint32, Buf);
	uint32	UnknownByte	= VARSTRUCT_DECODE_TYPE(uint8, Buf);
	uint32	ItemID		= VARSTRUCT_DECODE_TYPE(uint32, Buf);
	/* ItemName */		VARSTRUCT_DECODE_STRING(ItemName, Buf);
	/*uint32	Unknown2	=*/ VARSTRUCT_SKIP_TYPE(uint32, Buf);	//unused
	uint32	QtyBuyerWants	= VARSTRUCT_DECODE_TYPE(uint32, Buf);
		UnknownByte	= VARSTRUCT_DECODE_TYPE(uint8, Buf);
	uint32	Price		= VARSTRUCT_DECODE_TYPE(uint32, Buf);
	/*uint32	BuyerID2	=*/ VARSTRUCT_SKIP_TYPE(uint32, Buf);	//unused
	/*uint32	Unknown3	=*/ VARSTRUCT_SKIP_TYPE(uint32, Buf);	//unused

	const EQ::ItemData *item = database.GetItem(ItemID);

	if(!item || !Quantity || !Price || !QtyBuyerWants) return;

	if (m_inv.HasItem(ItemID, Quantity, invWhereWorn | invWherePersonal | invWhereCursor) == INVALID_INDEX) {
		Message(Chat::Red, "You do not have %i %s on you.", Quantity, item->Name);
		return;
	}


	Client *Buyer = entity_list.GetClientByID(BuyerID);

	if(!Buyer || !Buyer->IsBuyer()) {
		Message(Chat::Red, "The Buyer has gone away.");
		return;
	}

	// For Stackable items, HasSpaceForItem will try check if there is space to stack with existing stacks in
	// the buyer inventory.
	if(!(Buyer->GetInv().HasSpaceForItem(item, Quantity))) {
		Message(Chat::Red, "The Buyer does not have space for %i %s", Quantity, item->Name);
		return;
	}

	if((static_cast<uint64>(Quantity) * static_cast<uint64>(Price)) > MAX_TRANSACTION_VALUE) {
		Message(Chat::Red, "That would exceed the single transaction limit of %u platinum.", MAX_TRANSACTION_VALUE / 1000);
		return;
	}

	if(!Buyer->HasMoney(Quantity * Price)) {
		Message(Chat::Red, "The Buyer does not have sufficient money to purchase that quantity of %s.", item->Name);
		Buyer->Message(Chat::Red, "%s tried to sell you %i %s, but you have insufficient funds.", GetName(), Quantity, item->Name);
		return;
	}

	if(Buyer->CheckLoreConflict(item)) {
		Message(Chat::Red, "That item is LORE and the Buyer already has one.");
		Buyer->Message(Chat::Red, "%s tried to sell you %s but this item is LORE and you already have one.",
					GetName(), item->Name);
		return;
	}

	if(item->NoDrop == 0) {
		Message(Chat::Red, "That item is NODROP.");
		return;
	}

	if(item->IsClassBag()) {
		Message(Chat::Red, "That item is a Bag.");
		return;
	}

	if(!item->Stackable) {

		for(uint32 i = 0; i < Quantity; i++) {

			int16 SellerSlot = m_inv.HasItem(ItemID, 1, invWhereWorn|invWherePersonal|invWhereCursor);

			// This shouldn't happen, as we already checked there was space in the Buyer's inventory
			if (SellerSlot == INVALID_INDEX) {

				if(i > 0) {
					// Set the Quantity to the actual number we successfully transferred.
					Quantity = i;
					break;
				}
				LogError("Unexpected error while moving item from seller to buyer");
				Message(Chat::Red, "Internal error while processing transaction.");
				return;
			}

			EQ::ItemInstance* ItemToTransfer = m_inv.PopItem(SellerSlot);

			if(!ItemToTransfer || !Buyer->MoveItemToInventory(ItemToTransfer, true)) {
				LogError("Unexpected error while moving item from seller to buyer");
				Message(Chat::Red, "Internal error while processing transaction.");

				if(ItemToTransfer)
					safe_delete(ItemToTransfer);

				return;
			}

			database.SaveInventory(CharacterID(), 0, SellerSlot);

			safe_delete(ItemToTransfer);

			// Remove the item from inventory, clientside
			//
			auto outapp2 = new EQApplicationPacket(OP_MoveItem, sizeof(MoveItem_Struct));

			MoveItem_Struct* mis	= (MoveItem_Struct*)outapp2->pBuffer;
			mis->from_slot		= SellerSlot;
			mis->to_slot		= 0xFFFFFFFF;
			mis->number_in_stack	= 0xFFFFFFFF;

			QueuePacket(outapp2);
			safe_delete(outapp2);

		}
	}
	else {
		// Stackable
		//
		uint32 QuantityMoved = 0;

		while(QuantityMoved < Quantity) {

			// Find the slot on the seller that has a stack of at least 1 of the item
			int16 SellerSlot = m_inv.HasItem(ItemID, 1, invWhereWorn|invWherePersonal|invWhereCursor);

			if (SellerSlot == INVALID_INDEX) {
				LogError("Unexpected error while moving item from seller to buyer");
				Message(Chat::Red, "Internal error while processing transaction.");
				return;
			}

			EQ::ItemInstance* ItemToTransfer = m_inv.PopItem(SellerSlot);

			if(!ItemToTransfer) {
				LogError("Unexpected error while moving item from seller to buyer");
				Message(Chat::Red, "Internal error while processing transaction.");
				return;
			}

			// If the stack we found has less than the quantity we are selling ...
			if(ItemToTransfer->GetCharges() <= (Quantity - QuantityMoved)) {
				// Transfer the entire stack

				QuantityMoved += ItemToTransfer->GetCharges();

				if(!Buyer->MoveItemToInventory(ItemToTransfer, true)) {
					LogError("Unexpected error while moving item from seller to buyer");
					Message(Chat::Red, "Internal error while processing transaction.");
					safe_delete(ItemToTransfer);
					return;
				}
				// Delete the entire stack from the seller's inventory
				database.SaveInventory(CharacterID(), 0, SellerSlot);

				safe_delete(ItemToTransfer);

				// and tell the client to do the same.
				auto outapp2 = new EQApplicationPacket(OP_MoveItem, sizeof(MoveItem_Struct));

				MoveItem_Struct* mis	= (MoveItem_Struct*)outapp2->pBuffer;
				mis->from_slot		= SellerSlot;
				mis->to_slot		= 0xFFFFFFFF;
				mis->number_in_stack	= 0xFFFFFFFF;

				QueuePacket(outapp2);
				safe_delete(outapp2);
			}
			else {
				//Move the amount we need, and put the rest of the stack back in the seller's inventory
				//
				int QuantityToRemoveFromStack = Quantity - QuantityMoved;

				ItemToTransfer->SetCharges(ItemToTransfer->GetCharges() - QuantityToRemoveFromStack);

				m_inv.PutItem(SellerSlot, *ItemToTransfer);

				database.SaveInventory(CharacterID(), ItemToTransfer, SellerSlot);

				ItemToTransfer->SetCharges(QuantityToRemoveFromStack);

				if(!Buyer->MoveItemToInventory(ItemToTransfer, true)) {
					LogError("Unexpected error while moving item from seller to buyer");
					Message(Chat::Red, "Internal error while processing transaction.");
					safe_delete(ItemToTransfer);
					return;
				}

				safe_delete(ItemToTransfer);

				auto outapp2 = new EQApplicationPacket(OP_DeleteItem, sizeof(MoveItem_Struct));

				MoveItem_Struct* mis	= (MoveItem_Struct*)outapp2->pBuffer;
				mis->from_slot			= SellerSlot;
				mis->to_slot			= 0xFFFFFFFF;
				mis->number_in_stack	= 0xFFFFFFFF;

				for(int i = 0; i < QuantityToRemoveFromStack; i++)
					QueuePacket(outapp2);

				safe_delete(outapp2);

				QuantityMoved = Quantity;
			}
		}

	}

	Buyer->TakeMoneyFromPP(Quantity * Price);

	AddMoneyToPP(Quantity * Price);

	if(RuleB(Bazaar, AuditTrail))
		BazaarAuditTrail(GetName(), Buyer->GetName(), ItemName, Quantity, Quantity * Price, 1);

	// We now send a packet to the Seller, which causes it to display 'You have sold <Qty> <Item> to <Player> for <money>'
	//
	// The PacketLength of 1016 is from the only instance of this packet I have seen, which is from Live, November 2008
	// The Titanium/6.2 struct is slightly different in that it appears to use fixed length strings instead of variable
	// length as used on Live. The extra space in the packet is also likely to be used for Item compensation, if we ever
	// implement that.
	//
	uint32 PacketLength = 1016;

	auto outapp = new EQApplicationPacket(OP_Barter, PacketLength);

	Buf = (char *)outapp->pBuffer;

	VARSTRUCT_ENCODE_TYPE(uint32, Buf, Barter_SellerTransactionComplete);
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, Quantity);
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, Quantity * Price);

	if (ClientVersion() >= EQ::versions::ClientVersion::SoD)
	{
		VARSTRUCT_ENCODE_TYPE(uint32, Buf, 0);	// Think this is the upper 32 bits of a 64 bit price
	}

	sprintf(Buf, "%s", Buyer->GetName()); Buf += 64;

	VARSTRUCT_ENCODE_TYPE(uint32, Buf, 0x00);
	VARSTRUCT_ENCODE_TYPE(uint8, Buf, 0x01);
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, 0x00);

	sprintf(Buf, "%s", ItemName); Buf += 64;

	QueuePacket(outapp);

	// This next packet goes to the Buyer and produces the 'You've bought <Qty> <Item> from <Seller> for <money>'
	//

	Buf = (char *)outapp->pBuffer;

	VARSTRUCT_ENCODE_TYPE(uint32, Buf, Barter_BuyerTransactionComplete);
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, Quantity);
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, Quantity * Price);

	if (Buyer->ClientVersion() >= EQ::versions::ClientVersion::SoD)
	{
		VARSTRUCT_ENCODE_TYPE(uint32, Buf, 0);	// Think this is the upper 32 bits of a 64 bit price
	}

	sprintf(Buf, "%s", GetName()); Buf += 64;

	VARSTRUCT_ENCODE_TYPE(uint32, Buf, 0x00);
	VARSTRUCT_ENCODE_TYPE(uint8, Buf, 0x01);
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, 0x00);

	sprintf(Buf, "%s", ItemName); Buf += 64;

	Buyer->QueuePacket(outapp);

	safe_delete(outapp);

	// Next we update the buyer table in the database to reflect the reduced quantity the Buyer wants to buy.
	//
	database.UpdateBuyLine(Buyer->CharacterID(), BuySlot, QtyBuyerWants - Quantity);

	// Next we update the Seller's Barter Window to reflect the reduced quantity the Buyer is now looking to buy.
	//
	auto outapp3 = new EQApplicationPacket(OP_Barter, 936);

	Buf = (char *)outapp3->pBuffer;

	VARSTRUCT_ENCODE_TYPE(uint32, Buf, Barter_BuyerInspectWindow);
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, BuySlot);
	VARSTRUCT_ENCODE_TYPE(uint8, Buf, 1); // Unknown
	VARSTRUCT_ENCODE_TYPE(uint32, Buf,ItemID);
	VARSTRUCT_ENCODE_STRING(Buf, ItemName);
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, item->Icon);
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, QtyBuyerWants - Quantity);

	// If the amount we have just sold completely satisfies the quantity the Buyer was looking for,
	// setting the next byte to 0 will remove the item from the Barter Window.
	//
	if(QtyBuyerWants - Quantity > 0) {
		VARSTRUCT_ENCODE_TYPE(uint8, Buf, 1); // 0 = Toggle Off, 1 = Toggle On
	}
	else {
		VARSTRUCT_ENCODE_TYPE(uint8, Buf, 0); // 0 = Toggle Off, 1 = Toggle On
	}

	VARSTRUCT_ENCODE_TYPE(uint32, Buf, Price);
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, Buyer->GetID());
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, 0);

	VARSTRUCT_ENCODE_STRING(Buf, Buyer->GetName());

	QueuePacket(outapp3);
	safe_delete(outapp3);

	// The next packet updates the /buyer window with the reduced quantity, and toggles the buy line off if the
	// quantity they wanted to buy has been met.
	//
	auto outapp4 = new EQApplicationPacket(OP_Barter, 936);

	Buf = (char*)outapp4->pBuffer;

	VARSTRUCT_ENCODE_TYPE(uint32, Buf, Barter_BuyerItemUpdate);
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, BuySlot);
	VARSTRUCT_ENCODE_TYPE(uint8, Buf, 1);
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, ItemID);
	VARSTRUCT_ENCODE_STRING(Buf, ItemName);
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, item->Icon);
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, QtyBuyerWants - Quantity);

	if((QtyBuyerWants - Quantity) > 0) {

		VARSTRUCT_ENCODE_TYPE(uint8, Buf, 1); // 0 = Toggle Off, 1 = Toggle On
	}
	else {
		VARSTRUCT_ENCODE_TYPE(uint8, Buf, 0); // 0 = Toggle Off, 1 = Toggle On
	}

	VARSTRUCT_ENCODE_TYPE(uint32, Buf, Price);
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, 0x08f4); // Unknown
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, 0);
	VARSTRUCT_ENCODE_STRING(Buf, Buyer->GetName());

	Buyer->QueuePacket(outapp4);
	safe_delete(outapp4);

	return;
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

void Client::ToggleBuyerMode(bool TurnOn) {

	auto outapp = new EQApplicationPacket(OP_Barter, 13 + strlen(GetName()));

	char* Buf = (char*)outapp->pBuffer;

	VARSTRUCT_ENCODE_TYPE(uint32, Buf, Barter_BuyerAppearance);
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, GetID());

	if(TurnOn) {
		VARSTRUCT_ENCODE_TYPE(uint32, Buf, 0x01);
	}
	else {
		VARSTRUCT_ENCODE_TYPE(uint32, Buf, 0x00);
		database.DeleteBuyLines(CharacterID());
		SetCustomerID(0);
	}

	VARSTRUCT_ENCODE_STRING(Buf, GetName());

	entity_list.QueueClients(this, outapp, false);

	safe_delete(outapp);

	Buyer = TurnOn;
}

void Client::UpdateBuyLine(const EQApplicationPacket *app) {

	// This method is called when:
	//
	// /buyer mode is first turned on, once for each item
	// A BuyLine is toggled on or off in the/buyer window.
	//
	char* Buf = (char*)app->pBuffer;

	char ItemName[64];

	/*uint32 Action		=*/ VARSTRUCT_SKIP_TYPE(uint32, Buf);	//unused
	uint32 BuySlot		= VARSTRUCT_DECODE_TYPE(uint32, Buf);
	uint8 Unknown009	= VARSTRUCT_DECODE_TYPE(uint8, Buf);
	uint32 ItemID		= VARSTRUCT_DECODE_TYPE(uint32, Buf);
	/* ItemName */		VARSTRUCT_DECODE_STRING(ItemName, Buf);
	uint32 Icon		= VARSTRUCT_DECODE_TYPE(uint32, Buf);
	uint32 Quantity		= VARSTRUCT_DECODE_TYPE(uint32, Buf);
	uint8 ToggleOnOff	= VARSTRUCT_DECODE_TYPE(uint8, Buf);
	uint32 Price		= VARSTRUCT_DECODE_TYPE(uint32, Buf);
	/*uint32 UnknownZ		=*/ VARSTRUCT_SKIP_TYPE(uint32, Buf);	//unused
	uint32 ItemCount	= VARSTRUCT_DECODE_TYPE(uint32, Buf);

	const EQ::ItemData *item = database.GetItem(ItemID);

	if(!item) return;

	bool LoreConflict = CheckLoreConflict(item);

	LogTrading("UpdateBuyLine: Char: [{}] BuySlot: [{}] ItemID [{}] [{}] Quantity [{}] Toggle: [{}] Price [{}] ItemCount [{}] LoreConflict [{}]",
					GetName(), BuySlot, ItemID, item->Name, Quantity, ToggleOnOff, Price, ItemCount, LoreConflict);

	if((item->NoDrop != 0) && (!item->IsClassBag())&& !LoreConflict && (Quantity > 0) && HasMoney(Quantity * Price) && ToggleOnOff && (ItemCount == 0)) {
		LogTrading("Adding to database");
		database.AddBuyLine(CharacterID(), BuySlot, ItemID, ItemName, Quantity, Price);
		QueuePacket(app);
	}
	else {
		if(ItemCount > 0) {
			Message(Chat::Red, "Buy line %s disabled as Item Compensation is not currently supported.", ItemName);
		} else if(Quantity <= 0) {
			Message(Chat::Red, "Buy line %s disabled as the quantity is invalid.", ItemName);
		} else if(LoreConflict) {
			Message(Chat::Red, "Buy line %s disabled as the item is LORE and you have one already.", ItemName);
		} else if(item->NoDrop == 0) {
			Message(Chat::Red, "Buy line %s disabled as the item is NODROP.", ItemName);
		} else if(item->IsClassBag()) {
			Message(Chat::Red, "Buy line %s disabled as the item is a Bag.", ItemName);
		} else if(ToggleOnOff) {
			Message(Chat::Red, "Buy line %s disabled due to insufficient funds.", ItemName);
		} else {
			database.RemoveBuyLine(CharacterID(), BuySlot);
		}

		auto outapp = new EQApplicationPacket(OP_Barter, 936);

		Buf = (char*)outapp->pBuffer;

		VARSTRUCT_ENCODE_TYPE(uint32, Buf, Barter_BuyerItemUpdate);
		VARSTRUCT_ENCODE_TYPE(uint32, Buf, BuySlot);
		VARSTRUCT_ENCODE_TYPE(uint8, Buf, Unknown009);
		VARSTRUCT_ENCODE_TYPE(uint32, Buf, ItemID);
		VARSTRUCT_ENCODE_STRING(Buf, ItemName);
		VARSTRUCT_ENCODE_TYPE(uint32, Buf, Icon);
		VARSTRUCT_ENCODE_TYPE(uint32, Buf, Quantity);
		VARSTRUCT_ENCODE_TYPE(uint8, Buf, 0);				// Toggle the Buy Line off in the client
		VARSTRUCT_ENCODE_TYPE(uint32, Buf, Price);
		VARSTRUCT_ENCODE_TYPE(uint32, Buf, 0x08f4);			// Unknown
		VARSTRUCT_ENCODE_TYPE(uint32, Buf, 0);
		VARSTRUCT_ENCODE_STRING(Buf, GetName());

		QueuePacket(outapp);
		safe_delete(outapp);
	}

}

void Client::BuyerItemSearch(const EQApplicationPacket *app) {

	BuyerItemSearch_Struct* bis = (BuyerItemSearch_Struct*)app->pBuffer;

	auto outapp = new EQApplicationPacket(OP_Barter, sizeof(BuyerItemSearchResults_Struct));

	BuyerItemSearchResults_Struct* bisr = (BuyerItemSearchResults_Struct*)outapp->pBuffer;

	const EQ::ItemData* item = 0;

	int Count=0;

	char Name[64];
	char Criteria[255];

	strn0cpy(Criteria, bis->SearchString, sizeof(Criteria));

	strupr(Criteria);

	char* pdest;

	uint32 it = 0;

	while ((item = database.IterateItems(&it))) {

		strn0cpy(Name, item->Name, sizeof(Name));

		strupr(Name);

		pdest = strstr(Name, Criteria);

		if (pdest != nullptr) {
			sprintf(bisr->Results[Count].ItemName, "%s", item->Name);
			bisr->Results[Count].ItemID = item->ID;
			bisr->Results[Count].Unknown068 = item->Icon;
			bisr->Results[Count].Unknown072 = 0x00000000;
			Count++;
		}
		if (Count == MAX_BUYER_ITEMSEARCH_RESULTS)
			break;
	}
	if (Count == MAX_BUYER_ITEMSEARCH_RESULTS)
		Message(Chat::Yellow, "Your search returned more than %i results. Only the first %i are displayed.",
				MAX_BUYER_ITEMSEARCH_RESULTS, MAX_BUYER_ITEMSEARCH_RESULTS);

	bisr->Action = Barter_BuyerSearch;
	bisr->ResultCount = Count;

	QueuePacket(outapp);
	safe_delete(outapp);
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

	auto item = items.front();

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
		in->method     = ByParcel;
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
		in->method     = ByParcel;
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
		in->method     = ByParcel;
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
		in->method     = ByParcel;
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
		in->method     = ByParcel;
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
		in->method     = ByParcel;
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
