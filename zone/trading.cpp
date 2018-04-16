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
#include "../common/string_util.h"
#include "../common/misc_functions.h"

#include "client.h"
#include "entity.h"
#include "mob.h"

#include "quest_parser_collection.h"
#include "string_ids.h"
#include "worldserver.h"

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

void Trade::SetTradeCash(uint32 in_pp, uint32 in_gp, uint32 in_sp, uint32 in_cp)
{
	pp=in_pp; gp=in_gp; sp=in_sp; cp=in_cp;
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
		Mob* with = With();
		if (with)
			with->trade->Start(owner->GetID(), false);
	}
}

// Add item from a given slot to trade bucket (automatically does bag data too)
void Trade::AddEntity(uint16 trade_slot_id, uint32 stack_size) {
	// TODO: review for inventory saves / consider changing return type to bool so failure can be passed to desync handler

	if (!owner || !owner->IsClient()) {
		// This should never happen
		Log(Logs::General, Logs::None, "Programming error: NPC's should not call Trade::AddEntity()");
		return;
	}

	// If one party accepted the trade then an item was added, their state needs to be reset
	owner->trade->state = Trading;
	Mob* with = With();
	if (with)
		with->trade->state = Trading;

	// Item always goes into trade bucket from cursor
	Client* client = owner->CastToClient();
	EQEmu::ItemInstance* inst = client->GetInv().GetItem(EQEmu::inventory::slotCursor);

	if (!inst) {
		client->Message(13, "Error: Could not find item on your cursor!");
		return;
	}

	EQEmu::ItemInstance* inst2 = client->GetInv().GetItem(trade_slot_id);

	// it looks like the original code attempted to allow stacking...
	// (it just didn't handle partial stack move actions)
	if (stack_size > 0) {
		if (!inst->IsStackable() || !inst2 || !inst2->GetItem() || (inst->GetID() != inst2->GetID()) || (stack_size > inst->GetCharges())) {
			client->Kick();
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

		Log(Logs::Detail, Logs::Trading, "%s added partial item '%s' stack (qty: %i) to trade slot %i", owner->GetName(), inst->GetItem()->Name, stack_size, trade_slot_id);

		if (_stack_size > 0)
			inst->SetCharges(_stack_size);
		else
			client->DeleteItemInInventory(EQEmu::inventory::slotCursor);

		SendItemData(inst2, trade_slot_id);
	}
	else {
		if (inst2 && inst2->GetID()) {
			client->Kick();
			return;
		}

		SendItemData(inst, trade_slot_id);

		Log(Logs::Detail, Logs::Trading, "%s added item '%s' to trade slot %i", owner->GetName(), inst->GetItem()->Name, trade_slot_id);

		client->PutItemInInventory(trade_slot_id, *inst);
		client->DeleteItemInInventory(EQEmu::inventory::slotCursor);
	}
}

// Retrieve mob the owner is trading with
// Done like this in case 'with' mob goes LD and Mob* becomes invalid
Mob* Trade::With()
{
	return entity_list.GetMob(with_id);
}

// Private Method: Send item data for trade item to other person involved in trade
void Trade::SendItemData(const EQEmu::ItemInstance* inst, int16 dest_slot_id)
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
		with->SendItemPacket(dest_slot_id - EQEmu::legacy::TRADE_BEGIN, inst, ItemPacketTradeView);
		if (inst->GetItem()->ItemClass == 1) {
			for (uint16 i = EQEmu::inventory::containerBegin; i < EQEmu::inventory::ContainerCount; i++) {
				uint16 bagslot_id = EQEmu::InventoryProfile::CalcSlotId(dest_slot_id, i);
				const EQEmu::ItemInstance* bagitem = trader->GetInv().GetItem(bagslot_id);
				if (bagitem) {
					with->SendItemPacket(bagslot_id - EQEmu::legacy::TRADE_BEGIN, bagitem, ItemPacketTradeView);
				}
			}
		}

		//safe_delete(outapp);
	}
}

// Audit trade: The part logged is what travels owner -> with
void Trade::LogTrade()
{
	Mob* with = With();
	if (!owner->IsClient() || !with)
		return; // Should never happen

	Client* trader = owner->CastToClient();
	bool logtrade = false;
	int admin_level = 0;
	uint8 item_count = 0;

	if (zone->tradevar != 0) {
		for (uint16 i = EQEmu::legacy::TRADE_BEGIN; i <= EQEmu::legacy::TRADE_END; i++) {
			if (trader->GetInv().GetItem(i))
				item_count++;
		}

		if (((this->cp + this->sp + this->gp + this->pp)>0) || (item_count>0))
			admin_level = trader->Admin();
		else
			admin_level = 999;

		if (zone->tradevar == 7) {
			logtrade = true;
		}
		else if ((admin_level>=10) && (admin_level<20)) {
			if ((zone->tradevar<8) && (zone->tradevar>5))
				logtrade = true;
		}
		else if (admin_level<=20) {
			if ((zone->tradevar<8) && (zone->tradevar>4))
				logtrade = true;
		}
		else if (admin_level<=80) {
			if ((zone->tradevar<8) && (zone->tradevar>3))
				logtrade = true;
		}
		else if (admin_level<=100){
			if ((zone->tradevar<9) && (zone->tradevar>2))
				logtrade = true;
		}
		else if (admin_level<=150){
			if (((zone->tradevar<8) && (zone->tradevar>1)) || (zone->tradevar==9))
				logtrade = true;
		}
		else if (admin_level<=255){
			if ((zone->tradevar<8) && (zone->tradevar>0))
				logtrade = true;
		}
	}

	if (logtrade == true) {
		char logtext[1000] = {0};
		uint32 cash = 0;
		bool comma = false;

		// Log items offered by owner
		cash = this->cp + this->sp + this->gp + this->pp;
		if ((cash>0) || (item_count>0)) {
			sprintf(logtext, "%s gave %s ", trader->GetName(), with->GetName());

			if (item_count > 0) {
				strcat(logtext, "items {");

				for (uint16 i = EQEmu::legacy::TRADE_BEGIN; i <= EQEmu::legacy::TRADE_END; i++) {
					const EQEmu::ItemInstance* inst = trader->GetInv().GetItem(i);

					if (!comma)
						comma = true;
					else {
						if (inst)
							strcat(logtext, ",");
					}

					if (inst) {
						char item_num[15] = {0};
						sprintf(item_num, "%i", inst->GetItem()->ID);
						strcat(logtext, item_num);

						if (inst->IsClassBag()) {
							for (uint8 j = EQEmu::inventory::containerBegin; j < EQEmu::inventory::ContainerCount; j++) {
								inst = trader->GetInv().GetItem(i, j);
								if (inst) {
									strcat(logtext, ",");
									sprintf(item_num, "%i", inst->GetItem()->ID);
									strcat(logtext, item_num);
								}
							}
						}
					}
				}
			}

			if (cash > 0) {
				char money[100] = {0};
				sprintf(money, " %ipp, %igp, %isp, %icp", trader->trade->pp, trader->trade->gp, trader->trade->sp, trader->trade->cp);
				strcat(logtext, money);
			}

			database.logevents(trader->AccountName(), trader->AccountID(),
				trader->Admin(), trader->GetName(), with->GetName(), "Trade", logtext, 6);
		}
	}
}


void Trade::DumpTrade()
{
	Mob* with = With();
	Log(Logs::Detail, Logs::Trading, "Dumping trade data: '%s' in TradeState %i with '%s'",
		this->owner->GetName(), state, ((with==nullptr)?"(null)":with->GetName()));

	if (!owner->IsClient())
		return;

	Client* trader = owner->CastToClient();
	for (uint16 i = EQEmu::legacy::TRADE_BEGIN; i <= EQEmu::legacy::TRADE_END; i++) {
		const EQEmu::ItemInstance* inst = trader->GetInv().GetItem(i);

		if (inst) {
			Log(Logs::Detail, Logs::Trading, "Item %i (Charges=%i, Slot=%i, IsBag=%s)",
				inst->GetItem()->ID, inst->GetCharges(),
				i, ((inst->IsClassBag()) ? "True" : "False"));

			if (inst->IsClassBag()) {
				for (uint8 j = EQEmu::inventory::containerBegin; j < EQEmu::inventory::ContainerCount; j++) {
					inst = trader->GetInv().GetItem(i, j);
					if (inst) {
						Log(Logs::Detail, Logs::Trading, "\tBagItem %i (Charges=%i, Slot=%i)",
							inst->GetItem()->ID, inst->GetCharges(),
							EQEmu::InventoryProfile::CalcSlotId(i, j));
					}
				}
			}
		}
	}

	Log(Logs::Detail, Logs::Trading, "\tpp:%i, gp:%i, sp:%i, cp:%i", pp, gp, sp, cp);
}


void Client::ResetTrade() {
	AddMoneyToPP(trade->cp, trade->sp, trade->gp, trade->pp, true);

	// step 1: process bags
	for (int16 trade_slot = EQEmu::legacy::TRADE_BEGIN; trade_slot <= EQEmu::legacy::TRADE_END; ++trade_slot) {
		const EQEmu::ItemInstance* inst = m_inv[trade_slot];

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
	for (int16 trade_slot = EQEmu::legacy::TRADE_BEGIN; trade_slot <= EQEmu::legacy::TRADE_END; ++trade_slot) {
		EQEmu::ItemInstance* inst = GetInv().GetItem(trade_slot);

		if (inst && inst->IsStackable()) {
			while (true) {
				// there's no built-in safety check against an infinite loop..but, it should break on one of the conditional checks
				int16 free_slot = m_inv.FindFreeSlotForTradeItem(inst);

				if ((free_slot == EQEmu::inventory::slotCursor) || (free_slot == INVALID_INDEX))
					break;

				EQEmu::ItemInstance* partial_inst = GetInv().GetItem(free_slot);

				if (!partial_inst)
					break;

				if (partial_inst->GetID() != inst->GetID()) {
					Log(Logs::Detail, Logs::None, "[CLIENT] Client::ResetTrade() - an incompatible location reference was returned by Inventory::FindFreeSlotForTradeItem()");

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
	for (int16 trade_slot = EQEmu::legacy::TRADE_END; trade_slot >= EQEmu::legacy::TRADE_BEGIN; --trade_slot) {
		EQEmu::ItemInstance* inst = GetInv().GetItem(trade_slot);

		if (inst && inst->IsStackable()) {
			for (int16 bias_slot = EQEmu::legacy::TRADE_BEGIN; bias_slot <= EQEmu::legacy::TRADE_END; ++bias_slot) {
				if (bias_slot >= trade_slot)
					break;

				EQEmu::ItemInstance* bias_inst = GetInv().GetItem(bias_slot);

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
	for (int16 trade_slot = EQEmu::legacy::TRADE_BEGIN; trade_slot <= EQEmu::legacy::TRADE_END; ++trade_slot) {
		const EQEmu::ItemInstance* inst = m_inv[trade_slot];

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
		Client* other = tradingWith->CastToClient();
		QSPlayerLogTrade_Struct* qs_audit = nullptr;
		bool qs_log = false;

		if(other) {
			Log(Logs::Detail, Logs::Trading, "Finishing trade with client %s", other->GetName());

			this->AddMoneyToPP(other->trade->cp, other->trade->sp, other->trade->gp, other->trade->pp, true);

			// step 0: pre-processing
			// QS code
			if (RuleB(QueryServ, PlayerLogTrades) && event_entry && event_details) {
				qs_audit = (QSPlayerLogTrade_Struct*)event_entry;
				qs_log = true;

				if (finalizer) {
					qs_audit->char2_id = this->character_id;

					qs_audit->char2_money.platinum = this->trade->pp;
					qs_audit->char2_money.gold = this->trade->gp;
					qs_audit->char2_money.silver = this->trade->sp;
					qs_audit->char2_money.copper = this->trade->cp;
				}
				else {
					qs_audit->char1_id = this->character_id;

					qs_audit->char1_money.platinum = this->trade->pp;
					qs_audit->char1_money.gold = this->trade->gp;
					qs_audit->char1_money.silver = this->trade->sp;
					qs_audit->char1_money.copper = this->trade->cp;
				}
			}

			// step 1: process bags
			for (int16 trade_slot = EQEmu::legacy::TRADE_BEGIN; trade_slot <= EQEmu::legacy::TRADE_END; ++trade_slot) {
				const EQEmu::ItemInstance* inst = m_inv[trade_slot];

				if (inst && inst->IsClassBag()) {
					Log(Logs::Detail, Logs::Trading, "Giving container %s (%d) in slot %d to %s", inst->GetItem()->Name, inst->GetItem()->ID, trade_slot, other->GetName());

					// TODO: need to check bag items/augments for no drop..everything for attuned...
					if (inst->GetItem()->NoDrop != 0 || Admin() >= RuleI(Character, MinStatusForNoDropExemptions) || RuleI(World, FVNoDropFlag) == 1 || other == this) {
						int16 free_slot = other->GetInv().FindFreeSlotForTradeItem(inst);

						if (free_slot != INVALID_INDEX) {
							if (other->PutItemInInventory(free_slot, *inst, true)) {
								Log(Logs::Detail, Logs::Trading, "Container %s (%d) successfully transferred, deleting from trade slot.", inst->GetItem()->Name, inst->GetItem()->ID);
								if (qs_log) {
									auto detail = new QSTradeItems_Struct;

									detail->from_id = this->character_id;
									detail->from_slot = trade_slot;
									detail->to_id = other->CharacterID();
									detail->to_slot = free_slot;
									detail->item_id = inst->GetID();
									detail->charges = 1;
									detail->aug_1 = inst->GetAugmentItemID(1);
									detail->aug_2 = inst->GetAugmentItemID(2);
									detail->aug_3 = inst->GetAugmentItemID(3);
									detail->aug_4 = inst->GetAugmentItemID(4);
									detail->aug_5 = inst->GetAugmentItemID(5);

									event_details->push_back(detail);

									if (finalizer)
										qs_audit->char2_count += detail->charges;
									else
										qs_audit->char1_count += detail->charges;

									for (uint8 sub_slot = EQEmu::inventory::containerBegin; (sub_slot < EQEmu::inventory::ContainerCount); ++sub_slot) { // this is to catch ALL items
										const EQEmu::ItemInstance* bag_inst = inst->GetItem(sub_slot);

										if (bag_inst) {
											detail = new QSTradeItems_Struct;

											detail->from_id = this->character_id;
											detail->from_slot = EQEmu::InventoryProfile::CalcSlotId(trade_slot, sub_slot);
											detail->to_id = other->CharacterID();
											detail->to_slot = EQEmu::InventoryProfile::CalcSlotId(free_slot, sub_slot);
											detail->item_id = bag_inst->GetID();
											detail->charges = (!bag_inst->IsStackable() ? 1 : bag_inst->GetCharges());
											detail->aug_1 = bag_inst->GetAugmentItemID(1);
											detail->aug_2 = bag_inst->GetAugmentItemID(2);
											detail->aug_3 = bag_inst->GetAugmentItemID(3);
											detail->aug_4 = bag_inst->GetAugmentItemID(4);
											detail->aug_5 = bag_inst->GetAugmentItemID(5);

											event_details->push_back(detail);

											if (finalizer)
												qs_audit->char2_count += detail->charges;
											else
												qs_audit->char1_count += detail->charges;
										}
									}
								}
							}
							else {
								Log(Logs::Detail, Logs::Trading, "Transfer of container %s (%d) to %s failed, returning to giver.", inst->GetItem()->Name, inst->GetItem()->ID, other->GetName());
								PushItemOnCursor(*inst, true);
							}
						}
						else {
							Log(Logs::Detail, Logs::Trading, "%s's inventory is full, returning container %s (%d) to giver.", other->GetName(), inst->GetItem()->Name, inst->GetItem()->ID);
							PushItemOnCursor(*inst, true);
						}
					}
					else {
						Log(Logs::Detail, Logs::Trading, "Container %s (%d) is NoDrop, returning to giver.", inst->GetItem()->Name, inst->GetItem()->ID);
						PushItemOnCursor(*inst, true);
					}

					DeleteItemInInventory(trade_slot);
				}
			}

			// step 2a: process stackables
			for (int16 trade_slot = EQEmu::legacy::TRADE_BEGIN; trade_slot <= EQEmu::legacy::TRADE_END; ++trade_slot) {
				EQEmu::ItemInstance* inst = GetInv().GetItem(trade_slot);

				if (inst && inst->IsStackable()) {
					while (true) {
						// there's no built-in safety check against an infinite loop..but, it should break on one of the conditional checks
						int16 partial_slot = other->GetInv().FindFreeSlotForTradeItem(inst);

						if ((partial_slot == EQEmu::inventory::slotCursor) || (partial_slot == INVALID_INDEX))
							break;

						EQEmu::ItemInstance* partial_inst = other->GetInv().GetItem(partial_slot);

						if (!partial_inst)
							break;

						if (partial_inst->GetID() != inst->GetID()) {
							Log(Logs::Detail, Logs::Trading, "[CLIENT] Client::ResetTrade() - an incompatible location reference was returned by Inventory::FindFreeSlotForTradeItem()");
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

						Log(Logs::Detail, Logs::Trading, "Transferring partial stack %s (%d) in slot %d to %s", inst->GetItem()->Name, inst->GetItem()->ID, trade_slot, other->GetName());

						if (other->PutItemInInventory(partial_slot, *partial_inst, true)) {
							Log(Logs::Detail, Logs::Trading, "Partial stack %s (%d) successfully transferred, deleting %i charges from trade slot.",
								inst->GetItem()->Name, inst->GetItem()->ID, (old_charges - inst->GetCharges()));
							if (qs_log) {
								auto detail = new QSTradeItems_Struct;

								detail->from_id = this->character_id;
								detail->from_slot = trade_slot;
								detail->to_id = other->CharacterID();
								detail->to_slot = partial_slot;
								detail->item_id = inst->GetID();
								detail->charges = (old_charges - inst->GetCharges());
								detail->aug_1 = 0;
								detail->aug_2 = 0;
								detail->aug_3 = 0;
								detail->aug_4 = 0;
								detail->aug_5 = 0;

								event_details->push_back(detail);

								if (finalizer)
									qs_audit->char2_count += detail->charges;
								else
									qs_audit->char1_count += detail->charges;
							}
						}
						else {
							Log(Logs::Detail, Logs::Trading, "Transfer of partial stack %s (%d) to %s failed, returning %i charges to trade slot.",
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
			for (int16 trade_slot = EQEmu::legacy::TRADE_END; trade_slot >= EQEmu::legacy::TRADE_BEGIN; --trade_slot) {
				EQEmu::ItemInstance* inst = GetInv().GetItem(trade_slot);

				if (inst && inst->IsStackable()) {
					for (int16 bias_slot = EQEmu::legacy::TRADE_BEGIN; bias_slot <= EQEmu::legacy::TRADE_END; ++bias_slot) {
						if (bias_slot >= trade_slot)
							break;

						EQEmu::ItemInstance* bias_inst = GetInv().GetItem(bias_slot);

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
							auto detail = new QSTradeItems_Struct;

							detail->from_id = this->character_id;
							detail->from_slot = trade_slot;
							detail->to_id = this->character_id;
							detail->to_slot = bias_slot;
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
			for (int16 trade_slot = EQEmu::legacy::TRADE_BEGIN; trade_slot <= EQEmu::legacy::TRADE_END; ++trade_slot) {
				const EQEmu::ItemInstance* inst = m_inv[trade_slot];

				if (inst) {
					Log(Logs::Detail, Logs::Trading, "Giving item %s (%d) in slot %d to %s", inst->GetItem()->Name, inst->GetItem()->ID, trade_slot, other->GetName());

					// TODO: need to check bag items/augments for no drop..everything for attuned...
					if (inst->GetItem()->NoDrop != 0 || Admin() >= RuleI(Character, MinStatusForNoDropExemptions) || RuleI(World, FVNoDropFlag) == 1 || other == this) {
						int16 free_slot = other->GetInv().FindFreeSlotForTradeItem(inst);

						if (free_slot != INVALID_INDEX) {
							if (other->PutItemInInventory(free_slot, *inst, true)) {
								Log(Logs::Detail, Logs::Trading, "Item %s (%d) successfully transferred, deleting from trade slot.", inst->GetItem()->Name, inst->GetItem()->ID);
								if (qs_log) {
									auto detail = new QSTradeItems_Struct;

									detail->from_id = this->character_id;
									detail->from_slot = trade_slot;
									detail->to_id = other->CharacterID();
									detail->to_slot = free_slot;
									detail->item_id = inst->GetID();
									detail->charges = (!inst->IsStackable() ? 1 : inst->GetCharges());
									detail->aug_1 = inst->GetAugmentItemID(1);
									detail->aug_2 = inst->GetAugmentItemID(2);
									detail->aug_3 = inst->GetAugmentItemID(3);
									detail->aug_4 = inst->GetAugmentItemID(4);
									detail->aug_5 = inst->GetAugmentItemID(5);

									event_details->push_back(detail);

									if (finalizer)
										qs_audit->char2_count += detail->charges;
									else
										qs_audit->char1_count += detail->charges;

									// 'step 3' should never really see containers..but, just in case...
									for (uint8 sub_slot = EQEmu::inventory::containerBegin; (sub_slot < EQEmu::inventory::ContainerCount); ++sub_slot) { // this is to catch ALL items
										const EQEmu::ItemInstance* bag_inst = inst->GetItem(sub_slot);

										if (bag_inst) {
											detail = new QSTradeItems_Struct;

											detail->from_id = this->character_id;
											detail->from_slot = trade_slot;
											detail->to_id = other->CharacterID();
											detail->to_slot = free_slot;
											detail->item_id = bag_inst->GetID();
											detail->charges = (!bag_inst->IsStackable() ? 1 : bag_inst->GetCharges());
											detail->aug_1 = bag_inst->GetAugmentItemID(1);
											detail->aug_2 = bag_inst->GetAugmentItemID(2);
											detail->aug_3 = bag_inst->GetAugmentItemID(3);
											detail->aug_4 = bag_inst->GetAugmentItemID(4);
											detail->aug_5 = bag_inst->GetAugmentItemID(5);

											event_details->push_back(detail);

											if (finalizer)
												qs_audit->char2_count += detail->charges;
											else
												qs_audit->char1_count += detail->charges;
										}
									}
								}
							}
							else {
								Log(Logs::Detail, Logs::Trading, "Transfer of Item %s (%d) to %s failed, returning to giver.", inst->GetItem()->Name, inst->GetItem()->ID, other->GetName());
								PushItemOnCursor(*inst, true);
							}
						}
						else {
							Log(Logs::Detail, Logs::Trading, "%s's inventory is full, returning item %s (%d) to giver.", other->GetName(), inst->GetItem()->Name, inst->GetItem()->ID);
							PushItemOnCursor(*inst, true);
						}
					}
					else {
						Log(Logs::Detail, Logs::Trading, "Item %s (%d) is NoDrop, returning to giver.", inst->GetItem()->Name, inst->GetItem()->ID);
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
			for (int16 trade_slot = EQEmu::legacy::TRADE_BEGIN; trade_slot <= EQEmu::legacy::TRADE_NPC_END; ++trade_slot) {
				const EQEmu::ItemInstance* trade_inst = m_inv[trade_slot];

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
						for (uint8 sub_slot = EQEmu::inventory::containerBegin; sub_slot < trade_inst->GetItem()->BagSlots; ++sub_slot) {
							const EQEmu::ItemInstance* trade_baginst = trade_inst->GetItem(sub_slot);

							if(trade_baginst) {
								detail = new QSHandinItems_Struct;

								strcpy(detail->action_type, "HANDIN");

								detail->char_slot = EQEmu::InventoryProfile::CalcSlotId(trade_slot, sub_slot);
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
		if(parse->HasQuestSub(tradingWith->GetNPCTypeID(), EVENT_TRADE)) {
			// This is a quest NPC
			quest_npc = true;
		}

		std::vector<EQEmu::Any> item_list;
		std::list<EQEmu::ItemInstance*> items;
		for (int i = EQEmu::legacy::TRADE_BEGIN; i <= EQEmu::legacy::TRADE_NPC_END; ++i) {
			EQEmu::ItemInstance *inst = m_inv.GetItem(i);
			if(inst) {
				items.push_back(inst);
				item_list.push_back(inst);
			} else {
				item_list.push_back((EQEmu::ItemInstance*)nullptr);
				continue;
			}

			const EQEmu::ItemData* item = inst->GetItem();
			if(item && quest_npc == false) {
				bool isPetAndCanHaveNoDrop = (RuleB(Pets, CanTakeNoDrop) && 
					_CLIENTPET(tradingWith) && 
					tradingWith->GetPetType()<=petOther);
				// if it was not a NO DROP or Attuned item (or if a GM is trading), let the NPC have it
				if(GetGM() || (inst->IsAttuned() == false && 
					(item->NoDrop != 0 || isPetAndCanHaveNoDrop))) {
					// pets need to look inside bags and try to equip items found there
					if (item->IsClassBag() && item->BagSlots > 0) {
						for (int16 bslot = EQEmu::inventory::containerBegin; bslot < item->BagSlots; bslot++) {
							const EQEmu::ItemInstance* baginst = inst->GetItem(bslot);
							if (baginst) {
								const EQEmu::ItemData* bagitem = baginst->GetItem();
								if (bagitem && (GetGM() || (bagitem->NoDrop != 0 && baginst->IsAttuned() == false))) {
									tradingWith->CastToNPC()->AddLootDrop(bagitem, &tradingWith->CastToNPC()->itemlist,
										baginst->GetCharges(), 1, 127, true, true);
								}
								else if (RuleB(NPC, ReturnNonQuestNoDropItems)) {
									PushItemOnCursor(*baginst, true);
								}
							}
						}
					}

					tradingWith->CastToNPC()->AddLootDrop(item, &tradingWith->CastToNPC()->itemlist,
						inst->GetCharges(), 1, 127, true, true);
				}
				// Return NO DROP and Attuned items being handed into a non-quest NPC if the rule is true
				else if (RuleB(NPC, ReturnNonQuestNoDropItems)) {
					PushItemOnCursor(*inst, true);
					DeleteItemInInventory(i);
				}
			}
		}

		if(RuleB(TaskSystem, EnableTaskSystem)) {
			int Cash = trade->cp + (trade->sp * 10) + (trade->gp * 100) + (trade->pp * 1000);
			if(UpdateTasksOnDeliver(items, Cash, tradingWith->GetNPCTypeID())) {
				if(!tradingWith->IsMoving())
					tradingWith->FaceTarget(this);

				this->EVENT_ITEM_ScriptStopReturn();

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

		EQEmu::ItemInstance *insts[4] = { 0 };
		for (int i = EQEmu::legacy::TRADE_BEGIN; i <= EQEmu::legacy::TRADE_NPC_END; ++i) {
			insts[i - EQEmu::legacy::TRADE_BEGIN] = m_inv.PopItem(i);
			database.SaveInventory(CharacterID(), nullptr, i);
		}

		parse->EventNPC(EVENT_TRADE, tradingWith->CastToNPC(), this, "", 0, &item_list);

		for(int i = 0; i < 4; ++i) {
			if(insts[i]) {
				safe_delete(insts[i]);
			}
		}
	}
}

bool Client::CheckTradeLoreConflict(Client* other)
{
	if (!other)
		return true;

	for (int16 index = EQEmu::legacy::TRADE_BEGIN; index <= EQEmu::legacy::TRADE_END; ++index) {
		const EQEmu::ItemInstance* inst = m_inv[index];
		if (!inst || !inst->GetItem())
			continue;

		if (other->CheckLoreConflict(inst->GetItem()))
			return true;
	}

	for (int16 index = EQEmu::legacy::TRADE_BAGS_BEGIN; index <= EQEmu::legacy::TRADE_BAGS_END; ++index) {
		const EQEmu::ItemInstance* inst = m_inv[index];
		if (!inst || !inst->GetItem())
			continue;

		if (other->CheckLoreConflict(inst->GetItem()))
			return true;
	}

	return false;
}

bool Client::CheckTradeNonDroppable()
{
	for (int16 index = EQEmu::legacy::TRADE_BEGIN; index <= EQEmu::legacy::TRADE_END; ++index){
		const EQEmu::ItemInstance* inst = m_inv[index];
		if (!inst)
			continue;

		if (!inst->IsDroppable())
			return true;
	}

	return false;
}

void Client::Trader_ShowItems(){
	auto outapp = new EQApplicationPacket(OP_Trader, sizeof(Trader_Struct));

	Trader_Struct* outints = (Trader_Struct*)outapp->pBuffer;
	Trader_Struct* TraderItems = database.LoadTraderItem(this->CharacterID());

	for(int i = 0; i < 80; i++){
		outints->ItemCost[i] = TraderItems->ItemCost[i];
		outints->Items[i] = TraderItems->Items[i];
	}
	outints->Code = BazaarTrader_ShowItems;

	QueuePacket(outapp);
	safe_delete(outapp);
	safe_delete(TraderItems);
}

void Client::SendTraderPacket(Client* Trader, uint32 Unknown72)
{
	if(!Trader)
		return;

	auto outapp = new EQApplicationPacket(OP_BecomeTrader, sizeof(BecomeTrader_Struct));

	BecomeTrader_Struct* bts = (BecomeTrader_Struct*)outapp->pBuffer;

	bts->Code = BazaarTrader_StartTraderMode;

	bts->ID = Trader->GetID();

	strn0cpy(bts->Name, Trader->GetName(), sizeof(bts->Name));

	bts->Unknown072 = Unknown72;

	QueuePacket(outapp);


	safe_delete(outapp);
}

void Client::Trader_CustomerBrowsing(Client *Customer) {

	auto outapp = new EQApplicationPacket(OP_Trader, sizeof(Trader_ShowItems_Struct));

	Trader_ShowItems_Struct* sis = (Trader_ShowItems_Struct*)outapp->pBuffer;

	sis->Code = BazaarTrader_CustomerBrowsing;

	sis->TraderID = Customer->GetID();

	QueuePacket(outapp);
}


void Client::Trader_StartTrader() {

	Trader=true;

	auto outapp = new EQApplicationPacket(OP_Trader, sizeof(Trader_ShowItems_Struct));

	Trader_ShowItems_Struct* sis = (Trader_ShowItems_Struct*)outapp->pBuffer;

	sis->Code = BazaarTrader_StartTraderMode;

	sis->TraderID = this->GetID();

	QueuePacket(outapp);


	safe_delete(outapp);

	// Notify other clients we are now in trader mode

	outapp= new EQApplicationPacket(OP_BecomeTrader, sizeof(BecomeTrader_Struct));

	BecomeTrader_Struct* bts = (BecomeTrader_Struct*)outapp->pBuffer;

	bts->Code = 1;

	bts->ID = this->GetID();

	strn0cpy(bts->Name, GetName(), sizeof(bts->Name));

	entity_list.QueueClients(this, outapp, false);


	safe_delete(outapp);
}

void Client::Trader_EndTrader() {

	// If someone is looking at our wares, remove all the items from the window.
	//
	if(CustomerID) {
		Client* Customer = entity_list.GetClientByID(CustomerID);
		GetItems_Struct* gis=GetTraderItems();

		if(Customer && gis) {
			auto outapp = new EQApplicationPacket(OP_TraderDelItem, sizeof(TraderDelItem_Struct));
			TraderDelItem_Struct* tdis = (TraderDelItem_Struct*)outapp->pBuffer;

			tdis->Unknown000 = 0;
			tdis->TraderID = Customer->GetID();
			tdis->Unknown012 = 0;
			Customer->Message(13, "The Trader is no longer open for business");

			for(int i = 0; i < 80; i++) {
				if(gis->Items[i] != 0) {

					if (Customer->ClientVersion() >= EQEmu::versions::ClientVersion::RoF)
					{
						// RoF+ use Item IDs for now
						tdis->ItemID = gis->Items[i];
					}
					else
					{
						tdis->ItemID = gis->SerialNumber[i];
					}

					Customer->QueuePacket(outapp);
				}
			}

			safe_delete(outapp);
			safe_delete(gis);
		}
	}

	database.DeleteTraderItem(this->CharacterID());

	// Notify other clients we are no longer in trader mode.
	//
	auto outapp = new EQApplicationPacket(OP_BecomeTrader, sizeof(BecomeTrader_Struct));

	BecomeTrader_Struct* bts = (BecomeTrader_Struct*)outapp->pBuffer;

	bts->Code = 0;

	bts->ID = this->GetID();

	strn0cpy(bts->Name, GetName(), sizeof(bts->Name));

	entity_list.QueueClients(this, outapp, false);


	safe_delete(outapp);

	outapp= new EQApplicationPacket(OP_Trader, sizeof(Trader_ShowItems_Struct));

	Trader_ShowItems_Struct* sis = (Trader_ShowItems_Struct*)outapp->pBuffer;

	sis->Code = BazaarTrader_EndTraderMode;

	sis->TraderID = BazaarTrader_EndTraderMode;

	QueuePacket(outapp);

	safe_delete(outapp);

	WithCustomer(0);

	this->Trader = false;
}

void Client::SendTraderItem(uint32 ItemID, uint16 Quantity) {

	std::string Packet;
	int16 FreeSlotID=0;

	const EQEmu::ItemData* item = database.GetItem(ItemID);

	if(!item){
		Log(Logs::Detail, Logs::Trading, "Bogus item deleted in Client::SendTraderItem!\n");
		return;
	}

	EQEmu::ItemInstance* inst = database.CreateItem(item, Quantity);

	if (inst)
	{
		bool is_arrow = (inst->GetItem()->ItemType == EQEmu::item::ItemTypeArrow) ? true : false;
		FreeSlotID = m_inv.FindFreeSlot(false, true, inst->GetItem()->Size, is_arrow);

		PutItemInInventory(FreeSlotID, *inst);
		Save();

		SendItemPacket(FreeSlotID, inst, ItemPacketTrade);

		safe_delete(inst);
	}
}

void Client::SendSingleTraderItem(uint32 CharID, int SerialNumber) {

	EQEmu::ItemInstance* inst= database.LoadSingleTraderItem(CharID, SerialNumber);
	if(inst) {
		SendItemPacket(30, inst, ItemPacketMerchant); // MainCursor?
		safe_delete(inst);
	}

}

void Client::BulkSendTraderInventory(uint32 char_id) {
	const EQEmu::ItemData *item;

	TraderCharges_Struct* TraderItems = database.LoadTraderItemWithCharges(char_id);

	for (uint8 i = 0;i < 80; i++) {
		if((TraderItems->ItemID[i] == 0) || (TraderItems->ItemCost[i] <= 0)) {
			continue;
		}
		else
			item=database.GetItem(TraderItems->ItemID[i]);

		if (item && (item->NoDrop!=0)) {
			EQEmu::ItemInstance* inst = database.CreateItem(item);
			if (inst) {
				inst->SetSerialNumber(TraderItems->SerialNumber[i]);
				if(TraderItems->Charges[i] > 0)
					inst->SetCharges(TraderItems->Charges[i]);

				if(inst->IsStackable()) {
					inst->SetMerchantCount(TraderItems->Charges[i]);
					inst->SetMerchantSlot(TraderItems->SerialNumber[i]);
				}

				inst->SetPrice(TraderItems->ItemCost[i]);
				SendItemPacket(30, inst, ItemPacketMerchant); // MainCursor?
				safe_delete(inst);
			}
			else
				Log(Logs::Detail, Logs::Trading, "Client::BulkSendTraderInventory nullptr inst pointer");
		}
		else
			Log(Logs::Detail, Logs::Trading, "Client::BulkSendTraderInventory nullptr item pointer or item is NODROP %8X",item);
	}
	safe_delete(TraderItems);
}

uint32 Client::FindTraderItemSerialNumber(int32 ItemID) {

	EQEmu::ItemInstance* item = nullptr;
	uint16 SlotID = 0;
	for (int i = EQEmu::legacy::GENERAL_BEGIN; i <= EQEmu::legacy::GENERAL_END; i++){
		item = this->GetInv().GetItem(i);
		if (item && item->GetItem()->ID == 17899){ //Traders Satchel
			for (int x = EQEmu::inventory::containerBegin; x < EQEmu::inventory::ContainerCount; x++) {
				// we already have the parent bag and a contents iterator..why not just iterate the bag!??
				SlotID = EQEmu::InventoryProfile::CalcSlotId(i, x);
				item = this->GetInv().GetItem(SlotID);
				if (item) {
					if (item->GetID() == ItemID)
						return item->GetSerialNumber();
				}
			}
		}
	}
	Log(Logs::Detail, Logs::Trading, "Client::FindTraderItemSerialNumber Couldn't find item! Item ID %i", ItemID);

	return 0;
}

EQEmu::ItemInstance* Client::FindTraderItemBySerialNumber(int32 SerialNumber){

	EQEmu::ItemInstance* item = nullptr;
	uint16 SlotID = 0;
	for (int i = EQEmu::legacy::GENERAL_BEGIN; i <= EQEmu::legacy::GENERAL_END; i++){
		item = this->GetInv().GetItem(i);
		if(item && item->GetItem()->ID == 17899){ //Traders Satchel
			for (int x = EQEmu::inventory::containerBegin; x < EQEmu::inventory::ContainerCount; x++) {
				// we already have the parent bag and a contents iterator..why not just iterate the bag!??
				SlotID = EQEmu::InventoryProfile::CalcSlotId(i, x);
				item = this->GetInv().GetItem(SlotID);
				if(item) {
					if(item->GetSerialNumber() == SerialNumber)
						return item;
				}
			}
		}
	}
	Log(Logs::Detail, Logs::Trading, "Client::FindTraderItemBySerialNumber Couldn't find item! Serial No. was %i", SerialNumber);

	return nullptr;
}


GetItems_Struct* Client::GetTraderItems(){

	const EQEmu::ItemInstance* item = nullptr;
	uint16 SlotID = 0;

	auto gis = new GetItems_Struct;

	memset(gis,0,sizeof(GetItems_Struct));

	uint8 ndx = 0;

	for (int i = EQEmu::legacy::GENERAL_BEGIN; i <= EQEmu::legacy::GENERAL_END; i++) {
		item = this->GetInv().GetItem(i);
		if(item && item->GetItem()->ID == 17899){ //Traders Satchel
			for (int x = EQEmu::inventory::containerBegin; x < EQEmu::inventory::ContainerCount; x++) {
				SlotID = EQEmu::InventoryProfile::CalcSlotId(i, x);

				item = this->GetInv().GetItem(SlotID);

				if(item){
					gis->Items[ndx] = item->GetItem()->ID;
					gis->SerialNumber[ndx] = item->GetSerialNumber();
					gis->Charges[ndx] = item->GetCharges();
					ndx++;
				}
			}
		}
	}
	return gis;
}

uint16 Client::FindTraderItem(int32 SerialNumber, uint16 Quantity){

	const EQEmu::ItemInstance* item= nullptr;
	uint16 SlotID = 0;
	for (int i = EQEmu::legacy::GENERAL_BEGIN; i <= EQEmu::legacy::GENERAL_END; i++) {
		item = this->GetInv().GetItem(i);
		if(item && item->GetItem()->ID == 17899){ //Traders Satchel
			for (int x = EQEmu::inventory::containerBegin; x < EQEmu::inventory::ContainerCount; x++){
				SlotID = EQEmu::InventoryProfile::CalcSlotId(i, x);

				item = this->GetInv().GetItem(SlotID);

				if (item && item->GetSerialNumber() == SerialNumber &&
					(item->GetCharges() >= Quantity || (item->GetCharges() <= 0 && Quantity == 1)))
				{
					return SlotID;
				}
			}
		}
	}
	Log(Logs::Detail, Logs::Trading, "Could NOT find a match for Item: %i with a quantity of: %i on Trader: %s\n",
					SerialNumber , Quantity, this->GetName());

	return 0;
}

void Client::NukeTraderItem(uint16 Slot,int16 Charges,uint16 Quantity,Client* Customer,uint16 TraderSlot, int32 SerialNumber, int32 itemid) {

	if(!Customer)
		return;

	Log(Logs::Detail, Logs::Trading, "NukeTraderItem(Slot %i, Charges %i, Quantity %i", Slot, Charges, Quantity);

	if(Quantity < Charges)
	{
		Customer->SendSingleTraderItem(this->CharacterID(), SerialNumber);
		m_inv.DeleteItem(Slot, Quantity);
	}
	else
	{
		auto outapp = new EQApplicationPacket(OP_TraderDelItem, sizeof(TraderDelItem_Struct));
		TraderDelItem_Struct* tdis = (TraderDelItem_Struct*)outapp->pBuffer;

		tdis->Unknown000 = 0;
		tdis->TraderID = Customer->GetID();
		if (Customer->ClientVersion() >= EQEmu::versions::ClientVersion::RoF)
		{
			// RoF+ use Item IDs for now
			tdis->ItemID = itemid;
		}
		else
		{
			tdis->ItemID = SerialNumber;
		}
		tdis->Unknown012 = 0;


		Customer->QueuePacket(outapp);
		safe_delete(outapp);

		m_inv.DeleteItem(Slot);
	}
	// This updates the trader. Removes it from his trading bags.
	//
	const EQEmu::ItemInstance* Inst = m_inv[Slot];

	database.SaveInventory(CharacterID(), Inst, Slot);

	EQApplicationPacket* outapp2;

	if(Quantity < Charges)
		outapp2 = new EQApplicationPacket(OP_DeleteItem,sizeof(MoveItem_Struct));
	else
		outapp2 = new EQApplicationPacket(OP_MoveItem,sizeof(MoveItem_Struct));

	MoveItem_Struct* mis = (MoveItem_Struct*)outapp2->pBuffer;
	mis->from_slot = Slot;
	mis->to_slot = 0xFFFFFFFF;
	mis->number_in_stack = 0xFFFFFFFF;

	if(Quantity >= Charges)
		Quantity = 1;

	for(int i = 0; i < Quantity; i++) {

		this->QueuePacket(outapp2);
	}
	safe_delete(outapp2);

}
void Client::TraderUpdate(uint16 SlotID,uint32 TraderID){
	// This method is no longer used.

	auto outapp = new EQApplicationPacket(OP_TraderItemUpdate, sizeof(TraderItemUpdate_Struct));
	TraderItemUpdate_Struct* tus=(TraderItemUpdate_Struct*)outapp->pBuffer;
	tus->Charges = 0xFFFF;
	tus->FromSlot = SlotID;
	tus->ToSlot = 0xFF;
	tus->TraderID = TraderID;
	tus->Unknown000 = 0;
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::FindAndNukeTraderItem(int32 SerialNumber, uint16 Quantity, Client* Customer, uint16 TraderSlot){

	const EQEmu::ItemInstance* item= nullptr;
	bool Stackable = false;
	int16 Charges=0;

	uint16 SlotID = FindTraderItem(SerialNumber, Quantity);
	
	if(SlotID > 0) {

		item = this->GetInv().GetItem(SlotID);

		if (!item)
		{
			Log(Logs::Detail, Logs::Trading, "Could not find Item: %i on Trader: %s", SerialNumber, Quantity, this->GetName());
			return;
		}

		Charges = this->GetInv().GetItem(SlotID)->GetCharges();

		Stackable = item->IsStackable();

		if (!Stackable)
			Quantity = (Charges > 0) ? Charges : 1;

		Log(Logs::Detail, Logs::Trading, "FindAndNuke %s, Charges %i, Quantity %i", item->GetItem()->Name, Charges, Quantity);

		if (Charges <= Quantity || (Charges <= 0 && Quantity==1) || !Stackable)
		{
			this->DeleteItemInInventory(SlotID, Quantity);

			TraderCharges_Struct* TraderItems = database.LoadTraderItemWithCharges(this->CharacterID());

			uint8 Count = 0;

			bool TestSlot = true;

			for(int i = 0;i < 80;i++){

				if(TestSlot && TraderItems->SerialNumber[i] == SerialNumber)
				{
					database.DeleteTraderItem(this->CharacterID(),i);
					NukeTraderItem(SlotID, Charges, Quantity, Customer, TraderSlot, TraderItems->SerialNumber[i], TraderItems->ItemID[i]);
					TestSlot=false;
				}
				else if (TraderItems->ItemID[i] > 0)
				{
					Count++;
				}
			}
			if (Count == 0)
			{
				Trader_EndTrader();
			}

			return;
		}
		else
		{
			database.UpdateTraderItemCharges(this->CharacterID(), item->GetSerialNumber(), Charges-Quantity);

			NukeTraderItem(SlotID, Charges, Quantity, Customer, TraderSlot, item->GetSerialNumber(), item->GetID());

			return;

		}
	}
	Log(Logs::Detail, Logs::Trading, "Could NOT find a match for Item: %i with a quantity of: %i on Trader: %s\n",SerialNumber,
					Quantity,this->GetName());
}

void Client::ReturnTraderReq(const EQApplicationPacket* app, int16 TraderItemCharges, uint32 itemid){

	TraderBuy_Struct* tbs = (TraderBuy_Struct*)app->pBuffer;

	EQApplicationPacket* outapp = nullptr;

	if (ClientVersion() >= EQEmu::versions::ClientVersion::RoF)
	{
		outapp = new EQApplicationPacket(OP_TraderShop, sizeof(TraderBuy_Struct));
	}
	else
	{
		outapp = new EQApplicationPacket(OP_TraderBuy, sizeof(TraderBuy_Struct));
	}

	TraderBuy_Struct* outtbs = (TraderBuy_Struct*)outapp->pBuffer;
	memcpy(outtbs, tbs, app->size);

	if (ClientVersion() >= EQEmu::versions::ClientVersion::RoF)
	{
		// Convert Serial Number back to Item ID for RoF+
		outtbs->ItemID = itemid;
	}
	else
	{
		// RoF+ requires individual price, but older clients require total price
		outtbs->Price = (tbs->Price * static_cast<uint32>(TraderItemCharges));
	}

	outtbs->Quantity = TraderItemCharges;
	// This should probably be trader ID, not customer ID as it is below.
	outtbs->TraderID = this->GetID();
	outtbs->AlreadySold = 0;

	QueuePacket(outapp);

	safe_delete(outapp);
}

void Client::TradeRequestFailed(const EQApplicationPacket* app) {

	TraderBuy_Struct* tbs = (TraderBuy_Struct*)app->pBuffer;

	auto outapp = new EQApplicationPacket(OP_TraderBuy, sizeof(TraderBuy_Struct));

	TraderBuy_Struct* outtbs = (TraderBuy_Struct*)outapp->pBuffer;

	memcpy(outtbs, tbs, app->size);

	outtbs->AlreadySold = 0xFFFFFFFF;

	outtbs->TraderID = 0xFFFFFFFF;;

	QueuePacket(outapp);

	safe_delete(outapp);
}


static void BazaarAuditTrail(const char *seller, const char *buyer, const char *itemName, int quantity, int totalCost, int tranType) {

	std::string query = StringFormat("INSERT INTO `trader_audit` "
                                    "(`time`, `seller`, `buyer`, `itemname`, `quantity`, `totalcost`, `trantype`) "
                                    "VALUES (NOW(), '%s', '%s', '%s', %i, %i, %i)",
                                    seller, buyer, itemName, quantity, totalCost, tranType);
	database.QueryDatabase(query);
}

void Client::BuyTraderItem(TraderBuy_Struct* tbs, Client* Trader, const EQApplicationPacket* app){

	if(!Trader) return;

	if(!Trader->IsTrader()) {
		TradeRequestFailed(app);
		return;
	}

	auto outapp = new EQApplicationPacket(OP_Trader, sizeof(TraderBuy_Struct));

	TraderBuy_Struct* outtbs = (TraderBuy_Struct*)outapp->pBuffer;

	outtbs->ItemID = tbs->ItemID;

	const EQEmu::ItemInstance* BuyItem = nullptr;
	uint32 ItemID = 0;

	if (ClientVersion() >= EQEmu::versions::ClientVersion::RoF)
	{
		// Convert Item ID to Serial Number for RoF+
		ItemID = tbs->ItemID;
		tbs->ItemID = Trader->FindTraderItemSerialNumber(tbs->ItemID);
	}

	BuyItem = Trader->FindTraderItemBySerialNumber(tbs->ItemID);

	if(!BuyItem) {
		Log(Logs::Detail, Logs::Trading, "Unable to find item on trader.");
		TradeRequestFailed(app);
		safe_delete(outapp);
		return;
	}

	tbs->Price = BuyItem->GetPrice();

	Log(Logs::Detail, Logs::Trading, "Buyitem: Name: %s, IsStackable: %i, Requested Quantity: %i, Charges on Item %i",
					BuyItem->GetItem()->Name, BuyItem->IsStackable(), tbs->Quantity, BuyItem->GetCharges());
	// If the item is not stackable, then we can only be buying one of them.
	if(!BuyItem->IsStackable())
		outtbs->Quantity = 1; // normally you can't send more than 1 here
	else {
		// Stackable items, arrows, diamonds, etc
		int32 ItemCharges = BuyItem->GetCharges();
		// ItemCharges for stackables should not be <= 0
		if(ItemCharges <= 0)
			outtbs->Quantity = 1;
		// If the purchaser requested more than is in the stack, just sell them how many are actually in the stack.
		else if(static_cast<uint32>(ItemCharges) < tbs->Quantity)
			outtbs->Quantity = ItemCharges;
		else
			outtbs->Quantity = tbs->Quantity;
	}

	Log(Logs::Detail, Logs::Trading, "Actual quantity that will be traded is %i", outtbs->Quantity);

	if((tbs->Price * outtbs->Quantity) <= 0) {
		Message(13, "Internal error. Aborting trade. Please report this to the ServerOP. Error code is 1");
		Trader->Message(13, "Internal error. Aborting trade. Please report this to the ServerOP. Error code is 1");
		Log(Logs::General, Logs::Error, "Bazaar: Zero price transaction between %s and %s aborted."
						"Item: %s, Charges: %i, TBS: Qty %i, Price: %i",
						GetName(), Trader->GetName(),
						BuyItem->GetItem()->Name, BuyItem->GetCharges(), tbs->Quantity, tbs->Price);
		TradeRequestFailed(app);
		safe_delete(outapp);
		return;
	}

	uint64 TotalTransactionValue = static_cast<uint64>(tbs->Price) * static_cast<uint64>(outtbs->Quantity);

	if(TotalTransactionValue > MAX_TRANSACTION_VALUE) {
		Message(13, "That would exceed the single transaction limit of %u platinum.", MAX_TRANSACTION_VALUE / 1000);
		TradeRequestFailed(app);
		safe_delete(outapp);
		return;
	}

	// This cannot overflow assuming MAX_TRANSACTION_VALUE, checked above, is the default of 2000000000
	uint32 TotalCost = tbs->Price * outtbs->Quantity;

	if (Trader->ClientVersion() >= EQEmu::versions::ClientVersion::RoF)
	{
		// RoF+ uses individual item price where older clients use total price
		outtbs->Price = tbs->Price;
	}
	else
	{
		outtbs->Price = TotalCost;
	}

	if(!TakeMoneyFromPP(TotalCost)) {
		database.SetHackerFlag(account_name, name, "Attempted to buy something in bazaar but did not have enough money.");
		TradeRequestFailed(app);
		safe_delete(outapp);
		return;
	}

	Log(Logs::Detail, Logs::Trading, "Customer Paid: %d in Copper", TotalCost);

	uint32 platinum = TotalCost / 1000;
	TotalCost -= (platinum * 1000);
	uint32 gold = TotalCost / 100;
	TotalCost -= (gold * 100);
	uint32 silver = TotalCost / 10;
	TotalCost -= (silver * 10);
	uint32 copper = TotalCost;

	Trader->AddMoneyToPP(copper, silver, gold, platinum, true);

	Log(Logs::Detail, Logs::Trading, "Trader Received: %d Platinum, %d Gold, %d Silver, %d Copper", platinum, gold, silver, copper);

	ReturnTraderReq(app, outtbs->Quantity, ItemID);

	outtbs->TraderID = this->GetID();
	outtbs->Action = BazaarBuyItem;
	strn0cpy(outtbs->ItemName, BuyItem->GetItem()->Name, 64);

	int TraderSlot = 0;

	if(BuyItem->IsStackable())
		SendTraderItem(BuyItem->GetItem()->ID, outtbs->Quantity);
	else
		SendTraderItem(BuyItem->GetItem()->ID, BuyItem->GetCharges());

	TraderSlot = Trader->FindTraderItem(tbs->ItemID, outtbs->Quantity);

	if(RuleB(Bazaar, AuditTrail))
		BazaarAuditTrail(Trader->GetName(), GetName(), BuyItem->GetItem()->Name, outtbs->Quantity, outtbs->Price, 0);

	Trader->FindAndNukeTraderItem(tbs->ItemID, outtbs->Quantity, this, 0);

	if (ItemID > 0 && Trader->ClientVersion() >= EQEmu::versions::ClientVersion::RoF)
	{
		// Convert Serial Number back to ItemID for RoF+
		outtbs->ItemID = ItemID;
	}

	Trader->QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendBazaarWelcome()
{
	const std::string query = "SELECT COUNT(DISTINCT char_id), count(char_id) FROM trader";
	auto results = database.QueryDatabase(query);
	if (results.Success() && results.RowCount() == 1){
		auto row = results.begin();

		EQApplicationPacket* outapp = nullptr;
		if (ClientVersion() >= EQEmu::versions::ClientVersion::RoF)
		{
			outapp = new EQApplicationPacket(OP_TraderShop, sizeof(BazaarWelcome_Struct));
		}
		else
		{
			outapp = new EQApplicationPacket(OP_BazaarSearch, sizeof(BazaarWelcome_Struct));
		}

		memset(outapp->pBuffer,0,outapp->size);

		BazaarWelcome_Struct* bws = (BazaarWelcome_Struct*)outapp->pBuffer;

		bws->Beginning.Action = BazaarWelcome;

		bws->Traders = atoi(row[0]);
		bws->Items = atoi(row[1]);

		if (ClientVersion() >= EQEmu::versions::ClientVersion::RoF)
		{
			bws->Unknown012 = GetID();
		}

		QueuePacket(outapp);

		safe_delete(outapp);
	}

	const std::string buyerCountQuery = "SELECT COUNT(DISTINCT charid) FROM buyer";
	results = database.QueryDatabase(buyerCountQuery);
	if (!results.Success() || results.RowCount() != 1)
		return;

	auto row = results.begin();
	Message(10, "There are %i Buyers waiting to purchase your loot. Type /barter to search for them, "
				"or use /buyer to set up your own Buy Lines.", atoi(row[0]));
}

void Client::SendBazaarResults(uint32 TraderID, uint32 Class_, uint32 Race, uint32 ItemStat, uint32 Slot, uint32 Type,
					char Name[64], uint32 MinPrice, uint32 MaxPrice) {

	std::string searchValues = " COUNT(item_id), trader.*, items.name ";
	std::string searchCriteria = " WHERE trader.item_id = items.id ";

	if(TraderID > 0) {
		Client* trader = entity_list.GetClientByID(TraderID);

		if(trader)
			searchCriteria.append(StringFormat(" AND trader.char_id = %i", trader->CharacterID()));
	}

	if(MinPrice != 0)
		searchCriteria.append(StringFormat(" AND trader.item_cost >= %i", MinPrice));

	if(MaxPrice != 0)
		searchCriteria.append(StringFormat(" AND trader.item_cost <= %i", MaxPrice));

	if(strlen(Name) > 0) {
		char *safeName = RemoveApostrophes(Name);
		searchCriteria.append(StringFormat(" AND items.name LIKE '%%%s%%'", safeName));
		safe_delete_array(safeName);
	}

	if(Class_ != 0xFFFFFFFF)
		searchCriteria.append(StringFormat(" AND MID(REVERSE(BIN(items.classes)), %i, 1) = 1", Class_));

	if(Race != 0xFFFFFFFF)
		searchCriteria.append(StringFormat(" AND MID(REVERSE(BIN(items.races)), %i, 1) = 1", Race));

	if(Slot != 0xFFFFFFFF)
		searchCriteria.append(StringFormat(" AND MID(REVERSE(BIN(items.slots)), %i, 1) = 1", Slot + 1));

	switch(Type){
        case 0xFFFFFFFF:
            break;
        case 0:
            // 1H Slashing
            searchCriteria.append(" AND items.itemtype = 0 AND damage > 0");
            break;
        case 31:
            searchCriteria.append(" AND items.itemclass = 2");
            break;
        case 46:
            searchCriteria.append(" AND items.spellid > 0 AND items.spellid < 65000");
            break;
        case 47:
            searchCriteria.append(" AND items.spellid = 998");
            break;
        case 48:
            searchCriteria.append(" AND items.spellid >= 1298 AND items.spellid <= 1307");
            break;
        case 49:
            searchCriteria.append(" AND items.focuseffect > 0");
            break;

        default:
            searchCriteria.append(StringFormat(" AND items.itemtype = %i", Type));
    }

	switch(ItemStat) {

		case STAT_AC:
			searchCriteria.append(" AND items.ac > 0");
			searchValues.append(", items.ac");
			break;

		case STAT_AGI:
			searchCriteria.append(" AND items.aagi > 0");
			searchValues.append(", items.aagi");
			break;

		case STAT_CHA:
			searchCriteria.append(" AND items.acha > 0");
			searchValues.append(", items.acha");
			break;

		case STAT_DEX:
			searchCriteria.append(" AND items.adex > 0");
			searchValues.append(", items.adex");
			break;

		case STAT_INT:
			searchCriteria.append(" AND items.aint > 0");
			searchValues.append(", items.aint");
			break;

		case STAT_STA:
			searchCriteria.append(" AND items.asta > 0");
			searchValues.append(", items.asta");
			break;

		case STAT_STR:
			searchCriteria.append(" AND items.astr > 0");
			searchValues.append(", items.astr");
			break;

		case STAT_WIS:
			searchCriteria.append(" AND items.awis > 0");
			searchValues.append(", items.awis");
			break;

		case STAT_COLD:
			searchCriteria.append(" AND items.cr > 0");
			searchValues.append(", items.cr");
			break;

		case STAT_DISEASE:
			searchCriteria.append(" AND items.dr > 0");
			searchValues.append(", items.dr");
			break;

		case STAT_FIRE:
			searchCriteria.append(" AND items.fr > 0");
			searchValues.append(", items.fr");
			break;

		case STAT_MAGIC:
            searchCriteria.append(" AND items.mr > 0");
			searchValues.append(", items.mr");
			break;

		case STAT_POISON:
			searchCriteria.append(" AND items.pr > 0");
			searchValues.append(", items.pr");
			break;

		case STAT_HP:
			searchCriteria.append(" AND items.hp > 0");
			searchValues.append(", items.hp");
			break;

		case STAT_MANA:
			searchCriteria.append(" AND items.mana > 0");
			searchValues.append(", items.mana");
			break;

		case STAT_ENDURANCE:
			searchCriteria.append(" AND items.endur > 0");
			searchValues.append(", items.endur");
			break;

		case STAT_ATTACK:
			searchCriteria.append(" AND items.attack > 0");
			searchValues.append(", items.attack");
			break;

		case STAT_HP_REGEN:
			searchCriteria.append(" AND items.regen > 0");
			searchValues.append(", items.regen");
			break;

		case STAT_MANA_REGEN:
			searchCriteria.append(" AND items.manaregen > 0");
			searchValues.append(", items.manaregen");
			break;

		case STAT_HASTE:
			searchCriteria.append(" AND items.haste > 0");
			searchValues.append(", items.haste");
			break;

		case STAT_DAMAGE_SHIELD:
			searchCriteria.append(" AND items.damageshield > 0");
			searchValues.append(", items.damageshield");
			break;

		default:
			searchValues.append(", 0");
			break;
	}

    std::string query = StringFormat("SELECT %s, SUM(charges), items.stackable "
                                    "FROM trader, items %s GROUP BY items.id, charges, char_id LIMIT %i",
                                    searchValues.c_str(), searchCriteria.c_str(), RuleI(Bazaar, MaxSearchResults));
    auto results = database.QueryDatabase(query);
    if (!results.Success()) {
		return;
    }

    Log(Logs::Detail, Logs::Trading, "SRCH: %s", query.c_str());

    int Size = 0;
    uint32 ID = 0;

    if (results.RowCount() == static_cast<unsigned long>(RuleI(Bazaar, MaxSearchResults)))
			Message(15, "Your search reached the limit of %i results. Please narrow your search down by selecting more options.",
					RuleI(Bazaar, MaxSearchResults));

    if(results.RowCount() == 0) {
	    auto outapp2 = new EQApplicationPacket(OP_BazaarSearch, sizeof(BazaarReturnDone_Struct));
	    BazaarReturnDone_Struct *brds = (BazaarReturnDone_Struct *)outapp2->pBuffer;
	    brds->TraderID = ID;
	    brds->Type = BazaarSearchDone;
	    brds->Unknown008 = 0xFFFFFFFF;
	    brds->Unknown012 = 0xFFFFFFFF;
	    brds->Unknown016 = 0xFFFFFFFF;
	    this->QueuePacket(outapp2);
	    safe_delete(outapp2);
	    return;
	}

    Size = results.RowCount() * sizeof(BazaarSearchResults_Struct);
    auto buffer = new uchar[Size];
    uchar *bufptr = buffer;
    memset(buffer, 0, Size);

    int Action = BazaarSearchResults;
    uint32 Cost = 0;
    int32 SerialNumber = 0;
    char temp_buffer[64] = {0};
    int Count = 0;
    uint32 StatValue = 0;

    for (auto row = results.begin(); row != results.end(); ++row) {
	    VARSTRUCT_ENCODE_TYPE(uint32, bufptr, Action);
	    Count = atoi(row[0]);
	    VARSTRUCT_ENCODE_TYPE(uint32, bufptr, Count);
	    SerialNumber = atoi(row[3]);
	    VARSTRUCT_ENCODE_TYPE(int32, bufptr, SerialNumber);
	    Client *Trader2 = entity_list.GetClientByCharID(atoi(row[1]));
	    if (Trader2) {
		    ID = Trader2->GetID();
		    VARSTRUCT_ENCODE_TYPE(uint32, bufptr, ID);
	    } else {
		    Log(Logs::Detail, Logs::Trading, "Unable to find trader: %i\n", atoi(row[1]));
		    VARSTRUCT_ENCODE_TYPE(uint32, bufptr, 0);
	    }
	    Cost = atoi(row[5]);
	    VARSTRUCT_ENCODE_TYPE(uint32, bufptr, Cost);
	    StatValue = atoi(row[8]);
	    VARSTRUCT_ENCODE_TYPE(uint32, bufptr, StatValue);
	    bool Stackable = atoi(row[10]);
	    if (Stackable) {
		    int Charges = atoi(row[9]);
		    sprintf(temp_buffer, "%s(%i)", row[7], Charges);
	    } else
		    sprintf(temp_buffer, "%s(%i)", row[7], Count);

	    memcpy(bufptr, &temp_buffer, strlen(temp_buffer));

	    bufptr += 64;

	    // Extra fields for SoD+
	    //
	    if (Trader2)
		    sprintf(temp_buffer, "%s", Trader2->GetName());
	    else
		    sprintf(temp_buffer, "Unknown");

	    memcpy(bufptr, &temp_buffer, strlen(temp_buffer));

	    bufptr += 64;

	    VARSTRUCT_ENCODE_TYPE(uint32, bufptr, atoi(row[1])); // ItemID
    }

    auto outapp = new EQApplicationPacket(OP_BazaarSearch, Size);

    memcpy(outapp->pBuffer, buffer, Size);

    this->QueuePacket(outapp);

    safe_delete(outapp);
    safe_delete_array(buffer);

    auto outapp2 = new EQApplicationPacket(OP_BazaarSearch, sizeof(BazaarReturnDone_Struct));
    BazaarReturnDone_Struct *brds = (BazaarReturnDone_Struct *)outapp2->pBuffer;

    brds->TraderID = ID;
    brds->Type = BazaarSearchDone;

    brds->Unknown008 = 0xFFFFFFFF;
    brds->Unknown012 = 0xFFFFFFFF;
    brds->Unknown016 = 0xFFFFFFFF;

    this->QueuePacket(outapp2);

    safe_delete(outapp2);
}

static void UpdateTraderCustomerItemsAdded(uint32 CustomerID, TraderCharges_Struct* gis, uint32 ItemID) {

	// Send Item packets to the customer to update the Merchant window with the
	// new items for sale, and give them a message in their chat window.

	Client* Customer = entity_list.GetClientByID(CustomerID);

	if(!Customer) return;

	const EQEmu::ItemData *item = database.GetItem(ItemID);

	if(!item) return;

	EQEmu::ItemInstance* inst = database.CreateItem(item);

	if(!inst) return;

	Customer->Message(13, "The Trader has put up %s for sale.", item->Name);

	for(int i = 0; i < 80; i++) {

		if(gis->ItemID[i] == ItemID) {

			inst->SetCharges(gis->Charges[i]);

			inst->SetPrice(gis->ItemCost[i]);

			inst->SetSerialNumber(gis->SerialNumber[i]);

			inst->SetMerchantSlot(gis->SerialNumber[i]);

			if(inst->IsStackable())
				inst->SetMerchantCount(gis->Charges[i]);

			Log(Logs::Detail, Logs::Trading, "Sending price update for %s, Serial No. %i with %i charges",
							item->Name, gis->SerialNumber[i], gis->Charges[i]);

			Customer->SendItemPacket(30, inst, ItemPacketMerchant); // MainCursor?
		}
	}

	safe_delete(inst);
}

static void UpdateTraderCustomerPriceChanged(uint32 CustomerID, TraderCharges_Struct* gis, uint32 ItemID, int32 Charges, uint32 NewPrice) {

	// Send ItemPackets to update the customer's Merchant window with the new price (or remove the item if
	// the new price is 0) and inform them with a chat message.

	Client* Customer = entity_list.GetClientByID(CustomerID);

	if(!Customer) return;

	const EQEmu::ItemData *item = database.GetItem(ItemID);

	if(!item) return;

	if(NewPrice == 0) {
		// If the new price is 0, remove the item(s) from the window.
		auto outapp = new EQApplicationPacket(OP_TraderDelItem, sizeof(TraderDelItem_Struct));
		TraderDelItem_Struct* tdis = (TraderDelItem_Struct*)outapp->pBuffer;

		tdis->Unknown000 = 0;
		tdis->TraderID = Customer->GetID();
		tdis->Unknown012 = 0;
		Customer->Message(13, "The Trader has withdrawn the %s from sale.", item->Name);

		for(int i = 0; i < 80; i++) {

			if(gis->ItemID[i] == ItemID) {
				if (Customer->ClientVersion() >= EQEmu::versions::ClientVersion::RoF)
				{
					// RoF+ use Item IDs for now
					tdis->ItemID = gis->ItemID[i];
				}
				else
				{
					tdis->ItemID = gis->SerialNumber[i];
				}
				tdis->ItemID = gis->SerialNumber[i];
				Log(Logs::Detail, Logs::Trading, "Telling customer to remove item %i with %i charges and S/N %i",
								ItemID, Charges, gis->SerialNumber[i]);


				Customer->QueuePacket(outapp);
			}
		}

		safe_delete(outapp);
		return;
	}

	Log(Logs::Detail, Logs::Trading, "Sending price updates to customer %s", Customer->GetName());

	EQEmu::ItemInstance* inst = database.CreateItem(item);

	if(!inst) return;

	if(Charges > 0)
		inst->SetCharges(Charges);

	inst->SetPrice(NewPrice);

	if(inst->IsStackable())
		inst->SetMerchantCount(Charges);

	// Let the customer know the price in the window has suddenly just changed on them.
	Customer->Message(13, "The Trader has changed the price of %s.", item->Name);

	for(int i = 0; i < 80; i++) {
		if((gis->ItemID[i] != ItemID) ||
			((!item->Stackable) && (gis->Charges[i] != Charges)))
			continue;

		inst->SetSerialNumber(gis->SerialNumber[i]);

		inst->SetMerchantSlot(gis->SerialNumber[i]);

		Log(Logs::Detail, Logs::Trading, "Sending price update for %s, Serial No. %i with %i charges",
						item->Name, gis->SerialNumber[i], gis->Charges[i]);

		Customer->SendItemPacket(30, inst, ItemPacketMerchant); // MainCursor??
	}
	safe_delete(inst);
}

void Client::HandleTraderPriceUpdate(const EQApplicationPacket *app) {

	// Handle price updates from the Trader and update a customer browsing our stuff if necessary
	// This method also handles removing items from sale and adding them back up whilst still in
	// Trader mode.
	//
	TraderPriceUpdate_Struct* tpus = (TraderPriceUpdate_Struct*)app->pBuffer;

	Log(Logs::Detail, Logs::Trading, "Received Price Update for %s, Item Serial No. %i, New Price %i",
					GetName(), tpus->SerialNumber, tpus->NewPrice);

	// Pull the items this Trader currently has for sale from the trader table.
	//
	TraderCharges_Struct* gis = database.LoadTraderItemWithCharges(CharacterID());

	if(!gis) {
		Log(Logs::Detail, Logs::None, "[CLIENT] Error retrieving Trader items details to update price.");
		return;
	}

	// The client only sends a single update with the Serial Number of the item whose price has been updated.
	// We must update the price for all the Trader's items that are identical to that one item, i.e.
	// if it is a stackable item like arrows, update the price for all stacks. If it is not stackable, then
	// update the prices for all items that have the same number of charges.
	//
	uint32 IDOfItemToUpdate = 0;

	int32 ChargesOnItemToUpdate = 0;

	uint32 OldPrice = 0;

	for(int i = 0; i < 80; i++) {

		if((gis->ItemID[i] > 0) && (gis->SerialNumber[i] == tpus->SerialNumber)) {
			// We found the item that the Trader wants to change the price of (or add back up for sale).
			//
			Log(Logs::Detail, Logs::Trading, "ItemID is %i, Charges is %i", gis->ItemID[i], gis->Charges[i]);

			IDOfItemToUpdate = gis->ItemID[i];

			ChargesOnItemToUpdate = gis->Charges[i];

			OldPrice = gis->ItemCost[i];

			break;
		}
	}

	if(IDOfItemToUpdate == 0) {

		// If the item is not currently in the trader table for this Trader, then they must have removed it from sale while
		// still in Trader mode. Check if the item is in their Trader Satchels, and if so, put it back up.

		// Quick Sanity check. If the item is not currently up for sale, and the new price is zero, just ack the packet
		// and do nothing.
		if(tpus->NewPrice == 0) {
			tpus->SubAction = BazaarPriceChange_RemoveItem;
			QueuePacket(app);
			safe_delete(gis);
			return ;
		}

		Log(Logs::Detail, Logs::Trading, "Unable to find item to update price for. Rechecking trader satchels");

		// Find what is in their Trader Satchels
		GetItems_Struct* newgis=GetTraderItems();

		uint32 IDOfItemToAdd = 0;

		int32 ChargesOnItemToAdd = 0;

		for(int i = 0; i < 80; i++) {

			if((newgis->Items[i] > 0) && (newgis->SerialNumber[i] == tpus->SerialNumber)) {

				Log(Logs::Detail, Logs::Trading, "Found new Item to Add, ItemID is %i, Charges is %i", newgis->Items[i],
								newgis->Charges[i]);

				IDOfItemToAdd = newgis->Items[i];
				ChargesOnItemToAdd = newgis->Charges[i];

				break;
			}
		}


		const EQEmu::ItemData *item = 0;

		if(IDOfItemToAdd)
			item = database.GetItem(IDOfItemToAdd);

		if(!IDOfItemToAdd || !item) {

			Log(Logs::Detail, Logs::Trading, "Item not found in Trader Satchels either.");
			tpus->SubAction = BazaarPriceChange_Fail;
			QueuePacket(app);
			Trader_EndTrader();
			safe_delete(gis);
			safe_delete(newgis);
			return;
		}

		// It is a limitation of the client that if you have multiple of the same item, but with different charges,
		// although you can set different prices for them before entering Trader mode. If you Remove them and then
		// add them back whilst still in Trader mode, they all go up for the same price. We check for this situation
		// and give the Trader a warning message.
		//
		if(!item->Stackable) {

			bool SameItemWithDifferingCharges = false;

			for(int i = 0; i < 80; i++) {
				if((newgis->Items[i] == IDOfItemToAdd) && (newgis->Charges[i] != ChargesOnItemToAdd)) {

					SameItemWithDifferingCharges = true;
					break;
				}
			}

			if(SameItemWithDifferingCharges)
				Message(13, "Warning: You have more than one %s with different charges. They have all been added for sale "
						"at the same price.", item->Name);
		}

		// Now put all Items with a matching ItemID up for trade.
		//
		for(int i = 0; i < 80; i++) {

			if(newgis->Items[i] == IDOfItemToAdd) {

				database.SaveTraderItem(CharacterID(), newgis->Items[i], newgis->SerialNumber[i], newgis->Charges[i],
							tpus->NewPrice, i);

				gis->ItemID[i] = newgis->Items[i];
				gis->Charges[i] = newgis->Charges[i];
				gis->SerialNumber[i] = newgis->SerialNumber[i];
				gis->ItemCost[i] = tpus->NewPrice;

				Log(Logs::Detail, Logs::Trading, "Adding new item for %s. ItemID %i, SerialNumber %i, Charges %i, Price: %i, Slot %i",
							GetName(), newgis->Items[i], newgis->SerialNumber[i], newgis->Charges[i],
							tpus->NewPrice, i);
			}
		}

		// If we have a customer currently browsing, update them with the new items.
		//
		if(CustomerID)
			UpdateTraderCustomerItemsAdded(CustomerID, gis, IDOfItemToAdd);

		safe_delete(gis);
		safe_delete(newgis);

		// Acknowledge to the client.
		tpus->SubAction = BazaarPriceChange_AddItem;
		QueuePacket(app);

		return;
	}

	// This is a safeguard against a Trader increasing the price of an item while a customer is browsing and
	// unwittingly buying it at a higher price than they were expecting to.
	//
	if((OldPrice != 0) && (tpus->NewPrice > OldPrice) && CustomerID) {

		tpus->SubAction = BazaarPriceChange_Fail;
		QueuePacket(app);
		Trader_EndTrader();
		Message(13, "You must remove the item from sale before you can increase the price while a customer is browsing.");
		Message(13, "Click 'Begin Trader' to restart Trader mode with the increased price for this item.");
		safe_delete(gis);
		return;
	}

	// Send Acknowledgement back to the client.
	if(OldPrice == 0)
		tpus->SubAction = BazaarPriceChange_AddItem;
	else if(tpus->NewPrice != 0)
		tpus->SubAction = BazaarPriceChange_UpdatePrice;
	else
		tpus->SubAction = BazaarPriceChange_RemoveItem;

	QueuePacket(app);

	if(OldPrice == tpus->NewPrice) {
		Log(Logs::Detail, Logs::Trading, "The new price is the same as the old one.");
		safe_delete(gis);
		return;
	}
	// Update the price for all items we have for sale that have this ItemID and number of charges, or remove
	// them from the trader table if the new price is zero.
	//
	database.UpdateTraderItemPrice(CharacterID(), IDOfItemToUpdate, ChargesOnItemToUpdate, tpus->NewPrice);

	// If a customer is browsing our goods, send them the updated prices / remove the items from the Merchant window
	if(CustomerID)
		UpdateTraderCustomerPriceChanged(CustomerID, gis, IDOfItemToUpdate, ChargesOnItemToUpdate, tpus->NewPrice);

	safe_delete(gis);

}

void Client::SendBuyerResults(char* searchString, uint32 searchID) {

	// This method is called when a potential seller in the /barter window searches for matching buyers
	//
	Log(Logs::Detail, Logs::None, "[CLIENT] Client::SendBuyerResults %s\n", searchString);

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
        Message(15, "Your search found too many results; some are not displayed.");
    else if(strlen(searchString) == 0)
        Message(10, "There are %i Buy Lines.", numberOfRows);
    else
        Message(10, "There are %i Buy Lines that match the search string '%s'.", numberOfRows, searchString);

    if(numberOfRows == 0)
        return;

    uint32 lastCharID = 0;
	Client *buyer = nullptr;

	for (auto row = results.begin(); row != results.end(); ++row) {
        char itemName[64];

        uint32 charID = atoi(row[0]);
		uint32 buySlot = atoi(row[1]);
		uint32 itemID = atoi(row[2]);
		strcpy(itemName, row[3]);
		uint32 quantity = atoi(row[4]);
		uint32 price = atoi(row[5]);

        // Each item in the search results is sent as a single fixed length packet, although the position of
		// the fields varies due to the use of variable length strings. The reason the packet is so big, is
		// to allow item compensation, e.g. a buyer could offer to buy a Blade Of Carnage for 10000pp plus
		// other items in exchange. Item compensation is not currently supported in EQEmu.
		//
		auto outapp = new EQApplicationPacket(OP_Barter, 940);

		char *buf = (char *)outapp->pBuffer;

		const EQEmu::ItemData* item = database.GetItem(itemID);

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
		Message(13, "The Buyer has gone away.");
		return;
	}

	bir->Approval = Buyer->WithCustomer(GetID());

	QueuePacket(app);

	if(bir->Approval == 0) {
		Message_StringID(clientMessageYellow, TRADER_BUSY);
		return;
	}

	const char *WelcomeMessagePointer = Buyer->GetBuyerWelcomeMessage();

	if(strlen(WelcomeMessagePointer) > 0)
		Message(10, "%s greets you, '%s'.", Buyer->GetName(), WelcomeMessagePointer);

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
        uint32 BuySlot = atoi(row[1]);
        uint32 ItemID = atoi(row[2]);
		strcpy(ItemName, row[3]);
		uint32 Quantity = atoi(row[4]);
		uint32 Price = atoi(row[5]);

		auto outapp = new EQApplicationPacket(OP_Barter, 936);

		char *Buf = (char *)outapp->pBuffer;

		const EQEmu::ItemData* item = database.GetItem(ItemID);

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

	const EQEmu::ItemData *item = database.GetItem(ItemID);

	if(!item || !Quantity || !Price || !QtyBuyerWants) return;

	if (m_inv.HasItem(ItemID, Quantity, invWhereWorn | invWherePersonal | invWhereCursor) == INVALID_INDEX) {
		Message(13, "You do not have %i %s on you.", Quantity, item->Name);
		return;
	}


	Client *Buyer = entity_list.GetClientByID(BuyerID);

	if(!Buyer || !Buyer->IsBuyer()) {
		Message(13, "The Buyer has gone away.");
		return;
	}

	// For Stackable items, HasSpaceForItem will try check if there is space to stack with existing stacks in
	// the buyer inventory.
	if(!(Buyer->GetInv().HasSpaceForItem(item, Quantity))) {
		Message(13, "The Buyer does not have space for %i %s", Quantity, item->Name);
		return;
	}

	if((static_cast<uint64>(Quantity) * static_cast<uint64>(Price)) > MAX_TRANSACTION_VALUE) {
		Message(13, "That would exceed the single transaction limit of %u platinum.", MAX_TRANSACTION_VALUE / 1000);
		return;
	}

	if(!Buyer->HasMoney(Quantity * Price)) {
		Message(13, "The Buyer does not have sufficient money to purchase that quantity of %s.", item->Name);
		Buyer->Message(13, "%s tried to sell you %i %s, but you have insufficient funds.", GetName(), Quantity, item->Name);
		return;
	}

	if(Buyer->CheckLoreConflict(item)) {
		Message(13, "That item is LORE and the Buyer already has one.");
		Buyer->Message(13, "%s tried to sell you %s but this item is LORE and you already have one.",
					GetName(), item->Name);
		return;
	}

	if(item->NoDrop == 0) {
		Message(13, "That item is NODROP.");
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
				Log(Logs::General, Logs::Error, "Unexpected error while moving item from seller to buyer.");
				Message(13, "Internal error while processing transaction.");
				return;
			}

			EQEmu::ItemInstance* ItemToTransfer = m_inv.PopItem(SellerSlot);

			if(!ItemToTransfer || !Buyer->MoveItemToInventory(ItemToTransfer, true)) {
				Log(Logs::General, Logs::Error, "Unexpected error while moving item from seller to buyer.");
				Message(13, "Internal error while processing transaction.");

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
				Log(Logs::General, Logs::Error, "Unexpected error while moving item from seller to buyer.");
				Message(13, "Internal error while processing transaction.");
				return;
			}

			EQEmu::ItemInstance* ItemToTransfer = m_inv.PopItem(SellerSlot);

			if(!ItemToTransfer) {
				Log(Logs::General, Logs::Error, "Unexpected error while moving item from seller to buyer.");
				Message(13, "Internal error while processing transaction.");
				return;
			}

			// If the stack we found has less than the quantity we are selling ...
			if(ItemToTransfer->GetCharges() <= (Quantity - QuantityMoved)) {
				// Transfer the entire stack

				QuantityMoved += ItemToTransfer->GetCharges();

				if(!Buyer->MoveItemToInventory(ItemToTransfer, true)) {
					Log(Logs::General, Logs::Error, "Unexpected error while moving item from seller to buyer.");
					Message(13, "Internal error while processing transaction.");
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
					Log(Logs::General, Logs::Error, "Unexpected error while moving item from seller to buyer.");
					Message(13, "Internal error while processing transaction.");
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

	AddMoneyToPP(Quantity * Price, false);

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

	if (ClientVersion() >= EQEmu::versions::ClientVersion::SoD)
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

	if (Buyer->ClientVersion() >= EQEmu::versions::ClientVersion::SoD)
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
		CustomerID = 0;
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

	const EQEmu::ItemData *item = database.GetItem(ItemID);

	if(!item) return;

	bool LoreConflict = CheckLoreConflict(item);

	Log(Logs::Detail, Logs::Trading, "UpdateBuyLine: Char: %s BuySlot: %i ItemID %i %s Quantity %i Toggle: %i Price %i ItemCount %i LoreConflict %i",
					GetName(), BuySlot, ItemID, item->Name, Quantity, ToggleOnOff, Price, ItemCount, LoreConflict);

	if((item->NoDrop != 0) && !LoreConflict && (Quantity > 0) && HasMoney(Quantity * Price) && ToggleOnOff && (ItemCount == 0)) {
		Log(Logs::Detail, Logs::Trading, "Adding to database");
		database.AddBuyLine(CharacterID(), BuySlot, ItemID, ItemName, Quantity, Price);
		QueuePacket(app);
	}
	else {
		if(ItemCount > 0)
			Message(13, "Buy line %s disabled as Item Compensation is not currently supported.", ItemName);

		else if(Quantity <= 0)
			Message(13, "Buy line %s disabled as the quantity is invalid.", ItemName);

		else if(LoreConflict)
			Message(13, "Buy line %s disabled as the item is LORE and you have one already.", ItemName);

		else if(item->NoDrop == 0)
			Message(13, "Buy line %s disabled as the item is NODROP.", ItemName);

		else if(ToggleOnOff)
			Message(13, "Buy line %s disabled due to insufficient funds.", ItemName);

		else
			database.RemoveBuyLine(CharacterID(), BuySlot);

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

	const EQEmu::ItemData* item = 0;

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
		Message(15, "Your search returned more than %i results. Only the first %i are displayed.",
				MAX_BUYER_ITEMSEARCH_RESULTS, MAX_BUYER_ITEMSEARCH_RESULTS);

	bisr->Action = Barter_BuyerSearch;
	bisr->ResultCount = Count;

	QueuePacket(outapp);
	safe_delete(outapp);
}
