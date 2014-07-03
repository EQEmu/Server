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
#include "../common/debug.h"
#include "masterentity.h"
#include "worldserver.h"
#include "net.h"
#include "zonedb.h"
#include "../common/spdat.h"
#include "../common/packet_dump.h"
#include "../common/packet_functions.h"
#include "petitions.h"
#include "../common/serverinfo.h"
#include "../common/ZoneNumbers.h"
#include "../common/moremath.h"
#include "../common/guilds.h"
#include "../common/logsys.h"
#include "../common/StringUtil.h"
#include "StringIDs.h"
#include "NpcAI.h"
#include "QuestParserCollection.h"
extern WorldServer worldserver;

uint32 Client::NukeItem(uint32 itemnum, uint8 where_to_check) {
	if (itemnum == 0)
		return 0;
	uint32 x = 0;
	ItemInst *cur = nullptr;

	int i;
	if(where_to_check & invWhereWorn) {
		for (i = 0; i <= 21; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable)
					x += cur->GetCharges();
				else
					x++;
				DeleteItemInInventory(i, 0, true);
			}
		}
		if (GetItemIDAt(9999) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(9999) != INVALID_ID)) {
			cur = m_inv.GetItem(9999);
			if(cur && cur->GetItem()->Stackable)
				x += cur->GetCharges();
			else
				x++;
			DeleteItemInInventory(9999, 0, true);
		}
	}

	if(where_to_check & invWhereCursor) {
		if (GetItemIDAt(30) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(30) != INVALID_ID)) {
			cur = m_inv.GetItem(30);
			if(cur && cur->GetItem()->Stackable)
				x += cur->GetCharges();
			else
				x++;
			DeleteItemInInventory(30, 0, true);
		}

		for (i = 331; i <= 340; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable)
					x += cur->GetCharges();
				else
					x++;
				DeleteItemInInventory(i, 0, true);
			}
		}
	}

	if(where_to_check & invWherePersonal) {
		for (i = 22; i <= 29; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable)
					x += cur->GetCharges();
				else
					x++;
				DeleteItemInInventory(i, 0, true);
			}
		}

		for (i = 251; i <= 330; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable)
					x += cur->GetCharges();
				else
					x++;
				DeleteItemInInventory(i, 0, true);
			}
		}
	}

	if(where_to_check & invWhereBank) {
		for (i = 2000; i <= 2023; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable)
					x += cur->GetCharges();
				else
					x++;
				DeleteItemInInventory(i, 0, true);
			}
		}

		for (i = 2031; i <= 2270; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable)
					x += cur->GetCharges();
				else
					x++;
				DeleteItemInInventory(i, 0, true);
			}
		}
	}

	if(where_to_check & invWhereSharedBank) {
		for (i = 2500; i <= 2501; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable)
					x += cur->GetCharges();
				else
					x++;
				DeleteItemInInventory(i, 0, true);
			}
		}

		for (i = 2531; i <= 2550; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable)
					x += cur->GetCharges();
				else
					x++;
				DeleteItemInInventory(i, 0, true);
			}
		}
	}
	return x;
}

bool Client::CheckLoreConflict(const Item_Struct* item) {
	if (!item || !(item->LoreFlag))
		return false;

	if (item->LoreGroup == -1)
		return (m_inv.HasItem(item->ID, 0, ~invWhereSharedBank) != SLOT_INVALID);

	return (m_inv.HasItemByLoreGroup(item->LoreGroup, ~invWhereSharedBank) != SLOT_INVALID);
}

bool Client::SummonItem(uint32 item_id, int16 charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5, bool attuned, uint16 to_slot) {
	const Item_Struct* item = database.GetItem(item_id);

	if(item == nullptr) {
		Message(13, "Item %u does not exist.", item_id);
		mlog(INVENTORY__ERROR, "Player %s on account %s attempted to create an item with an invalid id.\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u)\n", GetName(), account_name, item_id, aug1, aug2, aug3, aug4, aug5);
		return false;
	}
	else if(CheckLoreConflict(item)) {
		Message(13, "You already have a lore %s (%i) in your inventory.", item->Name, item_id);
		return false;
	}
	else if(((item->ItemClass != ItemClassCommon) || (item->AugType > 0)) && (aug1 | aug2 | aug3 | aug4 | aug5)) {
		Message(13, "You can not augment an augment or a non-common class item.");
		mlog(INVENTORY__ERROR, "Player %s on account %s attempted to augment an augment or a non-common class item.\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u)\n", GetName(), account_name, item->ID, aug1, aug2, aug3, aug4, aug5);
		return false;
	}
	
	uint32 augments[MAX_AUGMENT_SLOTS] = {aug1, aug2, aug3, aug4, aug5};
	uint32 classes = item->Classes;
	uint32 races = item->Races;
	uint32 slots = item->Slots;

	bool enforcewear = RuleB(Inventory, EnforceAugmentWear);
	bool enforcerestr = RuleB(Inventory, EnforceAugmentRestriction);
	bool enforceusable = RuleB(Inventory, EnforceAugmentUsability);
	
	for(int iter = 0; iter < MAX_AUGMENT_SLOTS; ++iter) {
		const Item_Struct* augtest = database.GetItem(augments[iter]);

		if(augtest == nullptr) {
			if(augments[iter]) {
				Message(13, "Augment %u (Aug%i) does not exist.", augments[iter], iter + 1);
				mlog(INVENTORY__ERROR, "Player %s on account %s attempted to create an augment (Aug%i) with an invalid id.\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u)\n", GetName(), account_name, (iter + 1), item->ID, aug1, aug2, aug3, aug4, aug5);
				return false;
			}
		}
		else {
			if(CheckLoreConflict(augtest)) {
				Message(13, "You already have a lore %s (%u) in your inventory.", augtest->Name, augtest->ID);				
				return false;
			}
			else if(augtest->AugType == 0) {
				Message(13, "%s (%u) (Aug%i) is not an actual augment.", augtest->Name, augtest->ID, iter + 1);
				mlog(INVENTORY__ERROR, "Player %s on account %s attempted to use a non-augment item (Aug%i) as an augment.\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u)\n", GetName(), account_name, item->ID, (iter + 1), aug1, aug2, aug3, aug4, aug5);				
				return false;
			}
			
			if(enforcewear) {
				if((item->AugSlotType[iter] == AugTypeNone) || !(((uint32)1 << (item->AugSlotType[iter] - 1)) & augtest->AugType)) {
					Message(13, "Augment %u (Aug%i) is not acceptable wear on Item %u.", augments[iter], iter + 1, item->ID);
					mlog(INVENTORY__ERROR, "Player %s on account %s attempted to augment an item with an unacceptable augment type (Aug%i).\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u)\n", GetName(), account_name, (iter + 1), item->ID, aug1, aug2, aug3, aug4, aug5);
					return false;
				}

				if(item->AugSlotVisible[iter] == 0) {
					Message(13, "Item %u has not evolved enough to accept Augment %u (Aug%i).", item->ID, augments[iter], iter + 1);
					mlog(INVENTORY__ERROR, "Player %s on account %s attempted to augment an unevolved item with augment type (Aug%i).\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u)\n", GetName(), account_name, (iter + 1), item->ID, aug1, aug2, aug3, aug4, aug5);
					return false;
				}
			}

			if(enforcerestr) {
				bool restrictfail = false;
				uint8 it = item->ItemType;

				switch(augtest->AugRestrict) {
					case AugRestrAny:
						break;
					case AugRestrArmor:
						switch(it) {
						case ItemTypeArmor:
							break;
						default:
							restrictfail = true;
							break;
						}
						break;
					case AugRestrWeapons:
						switch(it) {
							case ItemType1HSlash:
							case ItemType1HBlunt:
							case ItemType1HPiercing:
							case ItemTypeMartial:
							case ItemType2HSlash:
							case ItemType2HBlunt:
							case ItemType2HPiercing:
							case ItemTypeBow:
								break;
							default:
								restrictfail = true;
								break;
						}
						break;
					case AugRestr1HWeapons:
						switch(it) {
							case ItemType1HSlash:
							case ItemType1HBlunt:
							case ItemType1HPiercing:
							case ItemTypeMartial:
								break;
							default:
								restrictfail = true;
								break;
							}
						break;
					case AugRestr2HWeapons:
						switch(it) {
							case ItemType2HSlash:
							case ItemType2HBlunt:
							case ItemType2HPiercing:
							case ItemTypeBow:
								break;
							default:
								restrictfail = true;
								break;
						}
						break;
					case AugRestr1HSlash:
						switch(it) {
							case ItemType1HSlash:
								break;
							default:
								restrictfail = true;
							break;
						}
						break;
					case AugRestr1HBlunt:
						switch(it) {
							case ItemType1HBlunt:
								break;
							default:
								restrictfail = true;
								break;
						}
						break;
					case AugRestrPiercing:
						switch(it) {
							case ItemType1HPiercing:
								break;
							default:
								restrictfail = true;
								break;
						}
						break;
					case AugRestrHandToHand:
						switch(it) {
							case ItemTypeMartial:
								break;
							default:
								restrictfail = true;
								break;
						}
						break;
					case AugRestr2HSlash:
						switch(it) {
							case ItemType2HSlash:
								break;
							default:
								restrictfail = true;
								break;
						}
						break;
					case AugRestr2HBlunt:
						switch(it) {
							case ItemType2HBlunt:
								break;
							default:
								restrictfail = true;
							break;
						}
						break;
					case AugRestr2HPierce:
						switch(it) {
							case ItemType2HPiercing:
								break;
							default:
								restrictfail = true;
								break;
						}
						break;
					case AugRestrBows:
						switch(it) {
							case ItemTypeBow:
								break;
							default:
								restrictfail = true;
								break;
						}
						break;
					case AugRestrShields:
						switch(it) {
							case ItemTypeShield:
								break;
							default:
								restrictfail = true;
								break;
						}
						break;
					case AugRestr1HSlash1HBluntOrHandToHand:
						switch(it) {
							case ItemType1HSlash:
							case ItemType1HBlunt:
							case ItemTypeMartial:
								break;
							default:
								restrictfail = true;
								break;
						}
						break;
					case AugRestr1HBluntOrHandToHand:
						switch(it) {
							case ItemType1HBlunt:
							case ItemTypeMartial:
								break;
							default:
								restrictfail = true;
								break;
						}
						break;
					case AugRestrUnknown1:
					case AugRestrUnknown2:
					case AugRestrUnknown3:
					default:
						restrictfail = true;
						break;
				}

				if(restrictfail) {
					Message(13, "Augment %u (Aug%i) is restricted from wear on Item %u.", augments[iter], (iter + 1), item->ID);
					mlog(INVENTORY__ERROR, "Player %s on account %s attempted to augment an item with a restricted augment (Aug%i).\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u)\n", GetName(), account_name, (iter + 1), item->ID, aug1, aug2, aug3, aug4, aug5);
					return false;
				}
			}

			if(enforceusable) {
				if(item->Classes && !(classes &= augtest->Classes)) {
					Message(13, "Augment %u (Aug%i) will result in an item not usable by any class.", augments[iter], (iter + 1));
					mlog(INVENTORY__ERROR, "Player %s on account %s attempted to create an item unusable by any class.\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u)\n", GetName(), account_name, item->ID, aug1, aug2, aug3, aug4, aug5);
					return false;
				}

				if(item->Races && !(races &= augtest->Races)) {
					Message(13, "Augment %u (Aug%i) will result in an item not usable by any race.", augments[iter], (iter + 1));
					mlog(INVENTORY__ERROR, "Player %s on account %s attempted to create an item unusable by any race.\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u)\n", GetName(), account_name, item->ID, aug1, aug2, aug3, aug4, aug5);
					return false;
				}

				if(item->Slots && !(slots &= augtest->Slots)) {
					Message(13, "Augment %u (Aug%i) will result in an item not usable in any slot.", augments[iter], (iter + 1));
					mlog(INVENTORY__ERROR, "Player %s on account %s attempted to create an item unusable in any slot.\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u)\n", GetName(), account_name, item->ID, aug1, aug2, aug3, aug4, aug5);
					return false;
				}
			}
		}
	}

	if(charges <= 0 && item->Stackable)
		charges = 1;

	else if(charges == -1)
		charges = item->MaxCharges;

	ItemInst* inst = database.CreateItem(item, charges);

	if(inst == nullptr) {
		Message(13, "An unknown server error has occurred and your item was not created.");
		LogFile->write(EQEMuLog::Error, "Player %s on account %s encountered an unknown item creation error.\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u)\n", GetName(), account_name, item->ID, aug1, aug2, aug3, aug4, aug5);
		return false;
	}

	for(int iter = 0; iter < MAX_AUGMENT_SLOTS; ++iter) {
		if(augments[iter])
			inst->PutAugment(&database, iter, augments[iter]);
	}

	if(attuned && inst->GetItem()->Attuneable)
		inst->SetInstNoDrop(true);

	if(enforceusable && (((to_slot >= 0) && (to_slot <= 21)) || (to_slot == 9999))) {
		uint32 slottest = (to_slot == 9999) ? 22 : to_slot;

		if(!(slots & ((uint32)1 << slottest))) {
			Message(0, "This item is not equipable at slot %u - moving to cursor.", to_slot);
			mlog(INVENTORY__ERROR, "Player %s on account %s attempted to equip an item unusable in slot %u - moved to cursor.\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u)\n", GetName(), account_name, to_slot, item->ID, aug1, aug2, aug3, aug4, aug5);
			to_slot = SLOT_CURSOR;
		}
	}

	if(to_slot == SLOT_CURSOR) {
		PushItemOnCursor(*inst);
		SendItemPacket(SLOT_CURSOR, inst, ItemPacketSummonItem);
	}
	else
		PutItemInInventory(to_slot, *inst, true);
	safe_delete(inst);
	
	if((RuleB(Character, EnableDiscoveredItems)) && !GetGM()) {
		if(!IsDiscovered(item_id))
			DiscoverItem(item_id);

		for(int iter = 0; iter < MAX_AUGMENT_SLOTS; ++iter) {
			if(augments[iter] && !IsDiscovered(augments[iter]))
				DiscoverItem(augments[iter]);
		}
	}

	return true;
}

void Client::DropItem(int16 slot_id) {
	if(GetInv().CheckNoDrop(slot_id) && RuleI(World, FVNoDropFlag) == 0 ||	RuleI(Character, MinStatusForNoDropExemptions) < Admin() && RuleI(World, FVNoDropFlag) == 2) {
		database.SetHackerFlag(this->AccountName(), this->GetCleanName(), "Tried to drop an item on the ground that was nodrop!");
		GetInv().DeleteItem(slot_id);
		return;
	}

	ItemInst *inst = m_inv.PopItem(slot_id);
	if(inst) {
		int i = parse->EventItem(EVENT_DROP_ITEM, this, inst, nullptr, "", 0);
		if(i != 0)
			safe_delete(inst);
	}
	else {
		Message(13, "Error: Item not found in slot %i", slot_id);
		return;
	}

	if(slot_id == SLOT_CURSOR) {
		std::list<ItemInst*>::const_iterator s=m_inv.cursor_begin(),e=m_inv.cursor_end();
		database.SaveCursor(CharacterID(), s, e);
	}
	else
		database.SaveInventory(CharacterID(), nullptr, slot_id);

	if(!inst)
		return;

	Object* object = new Object(this, inst);
	entity_list.AddObject(object, true);
	object->StartDecay();
	safe_delete(inst);
}

void Client::DropInst(const ItemInst* inst) {
	if (!inst) {
		Message(13, "Error: Item not found");
		return;
	}

	if (inst->GetItem()->NoDrop == 0) {
		Message(13, "This item is NO DROP. Deleting.");
		return;
	}
	Object* object = new Object(this, inst);
	entity_list.AddObject(object, true);
	object->StartDecay();
}

int32 Client::GetItemIDAt(int16 slot_id) {
	const ItemInst* inst = m_inv[slot_id];
	if (inst)
		return inst->GetItem()->ID;
	return INVALID_ID;
}

int32 Client::GetAugmentIDAt(int16 slot_id, uint8 augslot) {
	const ItemInst* inst = m_inv[slot_id];
	if (inst) {
		if (inst->GetAugmentItemID(augslot))
			return inst->GetAugmentItemID(augslot);
	}
	return INVALID_ID;
}

void Client::DeleteItemInInventory(int16 slot_id, int16 quantity, bool client_update, bool update_db) {
	#if (EQDEBUG >= 5)
		LogFile->write(EQEMuLog::Debug, "DeleteItemInInventory(%i, %i, %s)", slot_id, quantity, (client_update) ? "true":"false");
	#endif

	if(!m_inv[slot_id]) {
		if(client_update && IsValidSlot(slot_id)) {
			EQApplicationPacket* outapp;
			outapp = new EQApplicationPacket(OP_DeleteItem, sizeof(DeleteItem_Struct));
			DeleteItem_Struct* delitem	= (DeleteItem_Struct*)outapp->pBuffer;
			delitem->from_slot = slot_id;
			delitem->to_slot = 0xFFFFFFFF;
			delitem->number_in_stack = 0xFFFFFFFF;
			QueuePacket(outapp);
			safe_delete(outapp);
		}
		return;
	}

	if(RuleB(QueryServ, PlayerLogDeletes)) {
		uint16 delete_count = 0;
		if(m_inv[slot_id])
			delete_count += m_inv.GetItem(slot_id)->GetTotalItemCount();

		ServerPacket* qspack = new ServerPacket(ServerOP_QSPlayerLogDeletes, sizeof(QSPlayerLogDelete_Struct) + (sizeof(QSDeleteItems_Struct) * delete_count));
		QSPlayerLogDelete_Struct* qsaudit = (QSPlayerLogDelete_Struct*)qspack->pBuffer;
		uint16 parent_offset = 0;

		qsaudit->char_id = character_id;
		qsaudit->stack_size = quantity;
		qsaudit->char_count = delete_count;

		qsaudit->items[parent_offset].char_slot = slot_id;
		qsaudit->items[parent_offset].item_id = m_inv[slot_id]->GetID();
		qsaudit->items[parent_offset].charges = m_inv[slot_id]->GetCharges();
		qsaudit->items[parent_offset].aug_1 = m_inv[slot_id]->GetAugmentItemID(1);
		qsaudit->items[parent_offset].aug_2 = m_inv[slot_id]->GetAugmentItemID(2);
		qsaudit->items[parent_offset].aug_3 = m_inv[slot_id]->GetAugmentItemID(3);
		qsaudit->items[parent_offset].aug_4 = m_inv[slot_id]->GetAugmentItemID(4);
		qsaudit->items[parent_offset].aug_5 = m_inv[slot_id]->GetAugmentItemID(5);

		if(m_inv[slot_id]->IsType(ItemClassContainer)) {
			for(uint8 bag_idx = 0; bag_idx < m_inv[slot_id]->GetItem()->BagSlots; bag_idx++) {
				ItemInst* bagitem = m_inv[slot_id]->GetItem(bag_idx);
				if(bagitem) {
					int16 bagslot_id = Inventory::CalcSlotId(slot_id, bag_idx);
					qsaudit->items[++parent_offset].char_slot = bagslot_id;
					qsaudit->items[parent_offset].item_id = bagitem->GetID();
					qsaudit->items[parent_offset].charges = bagitem->GetCharges();
					qsaudit->items[parent_offset].aug_1 = bagitem->GetAugmentItemID(1);
					qsaudit->items[parent_offset].aug_2 = bagitem->GetAugmentItemID(2);
					qsaudit->items[parent_offset].aug_3 = bagitem->GetAugmentItemID(3);
					qsaudit->items[parent_offset].aug_4 = bagitem->GetAugmentItemID(4);
					qsaudit->items[parent_offset].aug_5 = bagitem->GetAugmentItemID(5);
				}
			}
		}

		qspack->Deflate();
		if(worldserver.Connected()) { worldserver.SendPacket(qspack); }
		safe_delete(qspack);
	}
	bool isDeleted = m_inv.DeleteItem(slot_id, quantity);
	const ItemInst* inst = nullptr;
	if (slot_id == SLOT_CURSOR) {
		std::list<ItemInst*>::const_iterator s = m_inv.cursor_begin(), e = m_inv.cursor_end();
		if(update_db)
			database.SaveCursor(character_id, s, e);
	}
	else {
		inst = m_inv[slot_id];
		if(update_db)
			database.SaveInventory(character_id, inst, slot_id);
	}

	if(client_update && IsValidSlot(slot_id)) {
		EQApplicationPacket* outapp;
		if(inst) {
			if(!inst->IsStackable() && !isDeleted)
				outapp = new EQApplicationPacket(OP_DeleteCharge, sizeof(MoveItem_Struct));
			else
				outapp = new EQApplicationPacket(OP_DeleteItem, sizeof(MoveItem_Struct));

				DeleteItem_Struct* delitem = (DeleteItem_Struct*)outapp->pBuffer;
				delitem->from_slot = slot_id;
				delitem->to_slot = 0xFFFFFFFF;
				delitem->number_in_stack = 0xFFFFFFFF;
				for(int loop = 0; loop < quantity; loop++)
					QueuePacket(outapp);
				safe_delete(outapp);
		}
		else {
			outapp = new EQApplicationPacket(OP_MoveItem, sizeof(MoveItem_Struct));
			MoveItem_Struct* delitem = (MoveItem_Struct*)outapp->pBuffer;
			delitem->from_slot = slot_id;
			delitem->to_slot = 0xFFFFFFFF;
			delitem->number_in_stack = 0xFFFFFFFF;
			QueuePacket(outapp);
			safe_delete(outapp);
		}
	}
}

bool Client::PushItemOnCursor(const ItemInst& inst, bool client_update) {
	mlog(INVENTORY__SLOTS, "Putting item %s (%d) on the cursor", inst.GetItem()->Name, inst.GetItem()->ID);
	m_inv.PushCursor(inst);

	if (client_update)
		SendItemPacket(SLOT_CURSOR, &inst, ItemPacketSummonItem);
		
	std::list<ItemInst*>::const_iterator s=m_inv.cursor_begin(),e=m_inv.cursor_end();
	return database.SaveCursor(CharacterID(), s, e);
}

bool Client::PutItemInInventory(int16 slot_id, const ItemInst& inst, bool client_update) {
	mlog(INVENTORY__SLOTS, "Putting item %s (%d) into slot %d", inst.GetItem()->Name, inst.GetItem()->ID, slot_id);
	if (slot_id==SLOT_CURSOR)
		return PushItemOnCursor(inst,client_update);
	else
		m_inv.PutItem(slot_id, inst);

	if (client_update)
		SendItemPacket(slot_id, &inst, (slot_id==SLOT_CURSOR)?ItemPacketSummonItem:ItemPacketTrade);

	if (slot_id==SLOT_CURSOR) {
		std::list<ItemInst*>::const_iterator s=m_inv.cursor_begin(),e=m_inv.cursor_end();
		return database.SaveCursor(this->CharacterID(), s, e);
	}
	else
		return database.SaveInventory(this->CharacterID(), &inst, slot_id);

	CalcBonuses();
}

void Client::PutLootInInventory(int16 slot_id, const ItemInst &inst, ServerLootItem_Struct** bag_item_data) {
	mlog(INVENTORY__SLOTS, "Putting loot item %s (%d) into slot %d", inst.GetItem()->Name, inst.GetItem()->ID, slot_id);
	m_inv.PutItem(slot_id, inst);

	SendLootItemInPacket(&inst, slot_id);

	if (slot_id==SLOT_CURSOR) {
		std::list<ItemInst*>::const_iterator s=m_inv.cursor_begin(),e=m_inv.cursor_end();
		database.SaveCursor(this->CharacterID(), s, e);
	}
	else
		database.SaveInventory(this->CharacterID(), &inst, slot_id);

	if(bag_item_data) {
		int16 interior_slot;
		for(int i = 0; i < 10; i++) {
			if(bag_item_data[i] == nullptr)
				continue;
			const ItemInst *bagitem = database.CreateItem(bag_item_data[i]->item_id, bag_item_data[i]->charges, bag_item_data[i]->aug1, bag_item_data[i]->aug2, bag_item_data[i]->aug3, bag_item_data[i]->aug4, bag_item_data[i]->aug5);
			interior_slot = Inventory::CalcSlotId(slot_id, i);
			mlog(INVENTORY__SLOTS, "Putting bag loot item %s (%d) into slot %d (bag slot %d)", inst.GetItem()->Name, inst.GetItem()->ID, interior_slot, i);
			PutLootInInventory(interior_slot, *bagitem);
			safe_delete(bagitem);
		}
	}

	CalcBonuses();
}
bool Client::TryStacking(ItemInst* item, uint8 type, bool try_worn, bool try_cursor) {
	if(!item || !item->IsStackable() || item->GetCharges()>=item->GetItem()->StackSize)
		return false;
	int16 i;
	uint32 item_id = item->GetItem()->ID;
	for (i = 22; i <= 29; i++) {
		ItemInst* tmp_inst = m_inv.GetItem(i);
		if(tmp_inst && tmp_inst->GetItem()->ID == item_id && tmp_inst->GetCharges() < tmp_inst->GetItem()->StackSize){
			MoveItemCharges(*item, i, type);
			CalcBonuses();
			if(item->GetCharges())
				return AutoPutLootInInventory(*item, try_worn, try_cursor, 0);
			return true;
		}
	}
	for (i = 22; i <= 29; i++) {
		for (uint8 j = 0; j < 10; j++) {
			uint16 slotid = Inventory::CalcSlotId(i, j);
			ItemInst* tmp_inst = m_inv.GetItem(slotid);
			if(tmp_inst && tmp_inst->GetItem()->ID == item_id && tmp_inst->GetCharges() < tmp_inst->GetItem()->StackSize){
				MoveItemCharges(*item, slotid, type);
				CalcBonuses();
				if(item->GetCharges())
					return AutoPutLootInInventory(*item, try_worn, try_cursor, 0);
				return true;
			}
		}
	}
	return false;
}

bool Client::AutoPutLootInInventory(ItemInst& inst, bool try_worn, bool try_cursor, ServerLootItem_Struct** bag_item_data) {
	if (try_worn && inst.IsEquipable(GetBaseRace(), GetClass()) && inst.GetItem()->ReqLevel<=level && !inst.GetItem()->Attuneable && inst.GetItem()->ItemType != ItemTypeAugmentation) {
		for (int16 i = 0; i < 9999; i++) {
			if (i == 22)
				i = 9999;

			if (!m_inv[i]) {
				if(i == SLOT_PRIMARY && inst.IsWeapon()) {
					if((inst.GetItem()->ItemType == ItemType2HSlash) || (inst.GetItem()->ItemType == ItemType2HBlunt) || (inst.GetItem()->ItemType == ItemType2HPiercing)) {
						if(m_inv[SLOT_SECONDARY])
							continue;
					}
				}
				if(i == SLOT_SECONDARY && m_inv[SLOT_PRIMARY]) {
					uint8 use = m_inv[SLOT_PRIMARY]->GetItem()->ItemType;
					if(use == ItemType2HSlash || use == ItemType2HBlunt || use == ItemType2HPiercing)
						continue;
				}
				if(i == SLOT_SECONDARY && inst.IsWeapon() && !CanThisClassDualWield())
					continue;

				if (inst.IsEquipable(i)) {
					PutLootInInventory(i, inst);
					uint8 worn_slot_material = Inventory::CalcMaterialFromSlot(i);
					if(worn_slot_material != 0xFF)
						SendWearChange(worn_slot_material);					
					parse->EventItem(EVENT_EQUIP_ITEM, this, &inst, nullptr, "", i);
					return true;
				}
			}
		}
	}

	if (inst.IsStackable()) {
		if(TryStacking(&inst, ItemPacketTrade, try_worn, try_cursor))
			return true;
	}

	bool is_arrow = (inst.GetItem()->ItemType == ItemTypeArrow) ? true : false;
	int16 slot_id = m_inv.FindFreeSlot(inst.IsType(ItemClassContainer), try_cursor, inst.GetItem()->Size, is_arrow);
	if (slot_id != SLOT_INVALID) {
		PutLootInInventory(slot_id, inst, bag_item_data);
		return true;
	}

	return false;
}

void Client::MoveItemCharges(ItemInst &from, int16 to_slot, uint8 type) {
	ItemInst *tmp_inst = m_inv.GetItem(to_slot);
	if(tmp_inst && tmp_inst->GetCharges() < tmp_inst->GetItem()->StackSize) {
		int charge_slots_left = tmp_inst->GetItem()->StackSize - tmp_inst->GetCharges();
		int charges_to_move = from.GetCharges() < charge_slots_left ? from.GetCharges() : charge_slots_left;
		tmp_inst->SetCharges(tmp_inst->GetCharges() + charges_to_move);
		from.SetCharges(from.GetCharges() - charges_to_move);
		SendLootItemInPacket(tmp_inst, to_slot);
		if (to_slot==SLOT_CURSOR) {
			std::list<ItemInst*>::const_iterator s=m_inv.cursor_begin(),e=m_inv.cursor_end();
			database.SaveCursor(this->CharacterID(), s, e);
		}
		else
			database.SaveInventory(this->CharacterID(), tmp_inst, to_slot);
	}
}

bool Client::MakeItemLink(char* &ret_link, const ItemInst *inst) {
	if (!inst)
		return false;

	const Item_Struct* item = inst->GetItem();
	uint8 evolving = 0;
	uint16 loregroup = 0;
	uint8 evolvedlevel = 0;
	int hash = 0;
	if (GetClientVersion() >= EQClientRoF)
		MakeAnyLenString(&ret_link, "%1X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%1X" "%04X" "%1X" "%05X" "%08X", 0, item->ID, inst->GetAugmentItemID(0), inst->GetAugmentItemID(1), inst->GetAugmentItemID(2), inst->GetAugmentItemID(3), inst->GetAugmentItemID(4), inst->GetAugmentItemID(5), evolving, loregroup, evolvedlevel, 0, hash);
	else if (GetClientVersion() >= EQClientSoF)
		MakeAnyLenString(&ret_link, "%1X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%1X" "%04X" "%1X" "%05X" "%08X", 0, item->ID, inst->GetAugmentItemID(0), inst->GetAugmentItemID(1), inst->GetAugmentItemID(2), inst->GetAugmentItemID(3), inst->GetAugmentItemID(4), evolving, loregroup, evolvedlevel, 0, hash);
	else
		MakeAnyLenString(&ret_link, "%1X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%1X" "%04X" "%1X" "%08X", 0, item->ID, inst->GetAugmentItemID(0), inst->GetAugmentItemID(1), inst->GetAugmentItemID(2), inst->GetAugmentItemID(3), inst->GetAugmentItemID(4), evolving, loregroup, evolvedlevel, hash);

	return true;
}

int Client::GetItemLinkHash(const ItemInst* inst) {
	if (!inst)
		return 0;

	const Item_Struct* item = inst->GetItem();
	char* hash_str = 0;
	int hash = 0;

	if (item->ItemClass == 0 && item->CharmFileID)
		MakeAnyLenString(&hash_str, "%d%s-1-1-1-1-1%d %d %d %d %d %d %d %d %d", item->ID, item->Name, item->Light, item->Icon, item->Price, item->Size, item->Weight, item->ItemClass, item->ItemType, item->Favor, item->GuildFavor);
	else if (item->ItemClass == 2)
		MakeAnyLenString(&hash_str, "%d%s%d%d%09X", item->ID, item->Name, item->Weight, item->BookType, item->Price);
	else if (item->ItemClass == 1)
		MakeAnyLenString(&hash_str, "%d%s%x%d%09X%d", item->ID, item->Name, item->BagSlots, item->BagWR, item->Price, item->Weight);
	else
		MakeAnyLenString(&hash_str, "%d%s-1-1-1-1-1%d %d %d %d %d %d %d %d %d %d %d %d %d", item->ID, item->Name, item->Mana, item->HP, item->Favor, item->Light, item->Icon, item->Price, item->Weight, item->ReqLevel, item->Size, item->ItemClass, item->ItemType, item->AC, item->GuildFavor);

	safe_delete_array(hash_str);
	return hash;
}

void Client::SendItemLink(const ItemInst* inst, bool send_to_all) {
	if (!inst)
		return;

	const Item_Struct* item = inst->GetItem();
	const char* name2 = &item->Name[0];
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_ItemLinkText,strlen(name2)+68);
	char buffer2[135] = {0};
	char itemlink[135] = {0};
	sprintf(itemlink,"%c0%06u0%05u-%05u-%05u-%05u-%05u00000000%c", 0x12, item->ID, inst->GetAugmentItemID(0), inst->GetAugmentItemID(1), inst->GetAugmentItemID(2), inst->GetAugmentItemID(3), inst->GetAugmentItemID(4), 0x12);
	sprintf(buffer2,"%c%c%c%c%c%c%c%c%c%c%c%c%s",0x00,0x00,0x00,0x00,0xD3,0x01,0x00,0x00,0x1E,0x01,0x00,0x00,itemlink);
	memcpy(outapp->pBuffer,buffer2,outapp->size);
	QueuePacket(outapp);
	safe_delete(outapp);
	if (send_to_all == false)
		return;
	const char* charname = this->GetName();
	outapp = new EQApplicationPacket(OP_ItemLinkText,strlen(itemlink)+14+strlen(charname));
	char buffer3[150] = {0};
	sprintf(buffer3,"%c%c%c%c%c%c%c%c%c%c%c%c%6s%c%s",0x00,0x00,0x00,0x00,0xD2,0x01,0x00,0x00,0x00,0x00,0x00,0x00,charname,0x00,itemlink);
	memcpy(outapp->pBuffer,buffer3,outapp->size);
	entity_list.QueueCloseClients(this->CastToMob(),outapp,true,200,0,false);
	safe_delete(outapp);
}

void Client::SendLootItemInPacket(const ItemInst* inst, int16 slot_id) {
	SendItemPacket(slot_id,inst, ItemPacketTrade);
}

bool Client::IsValidSlot(uint32 slot) {
	if((slot == (uint32)SLOT_INVALID) || (slot >= 0 && slot <= 30) || (slot >= 251 && slot <= 340) || (slot >= 400 && slot <= 404) || (slot >= 2000 && slot <= 2023) || (slot >= 2031 && slot <= 2270) || (slot >= 2500 && slot <= 2501) || (slot >= 2531 && slot <= 2550) || (slot >= 3000 && slot <= 3007) || (slot >= 4000 && slot <= 4009) || (slot == 9999))
		return true;
	else
		return false;
}

bool Client::IsBankSlot(uint32 slot) {
	if ((slot >= 2000 && slot <= 2023) || (slot >= 2031 && slot <= 2270) || (slot >= 2500 && slot <= 2501) || (slot >= 2531 && slot <= 2550))
		return true;
	return false;
}

bool Client::SwapItem(MoveItem_Struct* move_in) {
	uint32 src_slot_check = move_in->from_slot;
	uint32 dst_slot_check = move_in->to_slot;
	uint32 stack_count_check = move_in->number_in_stack;

	if(!IsValidSlot(src_slot_check)){
		if(src_slot_check < 2147483647)
			Message(13, "Warning: Invalid slot move from slot %u to slot %u with %u charges!", src_slot_check, dst_slot_check, stack_count_check);
		mlog(INVENTORY__SLOTS, "Invalid slot move from slot %u to slot %u with %u charges!", src_slot_check, dst_slot_check, stack_count_check);
		return false;
	}

	if(!IsValidSlot(dst_slot_check)) {
		if(src_slot_check < 2147483647)
			Message(13, "Warning: Invalid slot move from slot %u to slot %u with %u charges!", src_slot_check, dst_slot_check, stack_count_check);
		mlog(INVENTORY__SLOTS, "Invalid slot move from slot %u to slot %u with %u charges!", src_slot_check, dst_slot_check, stack_count_check);
		return false;
	}

	if (move_in->from_slot == move_in->to_slot) {
		if(RuleB(QueryServ, PlayerLogMoves))
			QSSwapItemAuditor(move_in);
		return true;
	}

	if (move_in->to_slot == (uint32)SLOT_INVALID) {
		if(move_in->from_slot == (uint32)SLOT_CURSOR) {
			mlog(INVENTORY__SLOTS, "Client destroyed item from cursor slot %d", move_in->from_slot);
			if(RuleB(QueryServ, PlayerLogMoves))
				QSSwapItemAuditor(move_in);

			ItemInst *inst = m_inv.GetItem(SLOT_CURSOR);
			if(inst)
				parse->EventItem(EVENT_DESTROY_ITEM, this, inst, nullptr, "", 0);

			DeleteItemInInventory(move_in->from_slot);
			return true;
		}
		else {
			mlog(INVENTORY__SLOTS, "Deleted item from slot %d as a result of an inventory container tradeskill combine.", move_in->from_slot);
			if(RuleB(QueryServ, PlayerLogMoves))
				QSSwapItemAuditor(move_in);
			DeleteItemInInventory(move_in->from_slot);
			return true;
		}
	}
	if(auto_attack && (move_in->from_slot == SLOT_PRIMARY || move_in->from_slot == SLOT_SECONDARY || move_in->from_slot == SLOT_RANGE))
		SetAttackTimer();
	else if(auto_attack && (move_in->to_slot == SLOT_PRIMARY || move_in->to_slot == SLOT_SECONDARY || move_in->to_slot == SLOT_RANGE))
		SetAttackTimer();
	int16 src_slot_id = (int16)move_in->from_slot;
	int16 dst_slot_id = (int16)move_in->to_slot;

	if(IsBankSlot(src_slot_id) || IsBankSlot(dst_slot_id) || IsBankSlot(src_slot_check) || IsBankSlot(dst_slot_check)) {
		uint32 distance = 0;
		NPC *banker = entity_list.GetClosestBanker(this, distance);

		if(!banker || distance > USE_NPC_RANGE2) {
			char *hacked_string = nullptr;
			MakeAnyLenString(&hacked_string, "Player tried to make use of a banker(items) but %s is non-existant or too far away (%u units).", banker ? banker->GetName() : "UNKNOWN NPC", distance);
			database.SetMQDetectionFlag(AccountName(), GetName(), hacked_string, zone->GetShortName());
			safe_delete_array(hacked_string);
			Kick();
			return false;
		}
	}
	
	uint32 srcitemid = 0;
	uint32 dstitemid = 0;
	ItemInst* src_inst = m_inv.GetItem(src_slot_id);
	ItemInst* dst_inst = m_inv.GetItem(dst_slot_id);
	if (src_inst){
		mlog(INVENTORY__SLOTS, "Src slot %d has item %s (%d) with %d charges in it.", src_slot_id, src_inst->GetItem()->Name, src_inst->GetItem()->ID, src_inst->GetCharges());
		srcitemid = src_inst->GetItem()->ID;
		if (src_inst->GetCharges() > 0 && (src_inst->GetCharges() < (int16)move_in->number_in_stack || move_in->number_in_stack > src_inst->GetItem()->StackSize)) {
			Message(13,"Error: Insufficent number in stack.");
			return false;
		}
	}
	if (dst_inst) {
		mlog(INVENTORY__SLOTS, "Dest slot %d has item %s (%d) with %d charges in it.", dst_slot_id, dst_inst->GetItem()->Name, dst_inst->GetItem()->ID, dst_inst->GetCharges());
		dstitemid = dst_inst->GetItem()->ID;
	}
	if (Trader && srcitemid > 0) {
		ItemInst* srcbag;
		ItemInst* dstbag;
		uint32 srcbagid = 0;
		uint32 dstbagid = 0;
		if (src_slot_id >= 250 && src_slot_id < 330){
			srcbag=m_inv.GetItem(((int)(src_slot_id / 10)) - 3);
			if(srcbag)
				srcbagid=srcbag->GetItem()->ID;
		}
		if (dst_slot_id >= 250 && dst_slot_id < 330){
			dstbag=m_inv.GetItem(((int)(dst_slot_id / 10)) - 3);
			if(dstbag)
				dstbagid=dstbag->GetItem()->ID;
		}
		if (srcitemid == 17899 || srcbagid == 17899 || dstitemid == 17899 || dstbagid == 17899){
			this->Trader_EndTrader();
			this->Message(13,"You cannot move your Trader Satchels, or items inside them, while Trading.");
		}
	}

	if (!src_inst && (src_slot_id < 4000 || src_slot_id > 4009)) {
		if (dst_inst) {
			move_in->from_slot = dst_slot_check;
			move_in->to_slot = src_slot_check;
			move_in->number_in_stack = dst_inst->GetCharges();
			if(!SwapItem(move_in)) { mlog(INVENTORY__ERROR, "Recursive SwapItem call failed due to non-existent destination item (charid: %i, fromslot: %i, toslot: %i)", CharacterID(), src_slot_id, dst_slot_id); }
		}

		return false;
	}
	if(src_inst && src_slot_id >= 2500 && src_slot_id <= 2550) {
		if(!database.VerifyInventory(account_id, src_slot_id, src_inst)) {
			LogFile->write(EQEMuLog::Error, "Player %s on account %s was found exploiting the shared bank.\n", GetName(), account_name);
			DeleteItemInInventory(dst_slot_id,0,true);
			return(false);
		}
		if(src_slot_id >= 2500 && src_slot_id <= 2501 && src_inst->IsType(ItemClassContainer)){
			for (uint8 idx = 0; idx < 10; idx++) {
				const ItemInst* baginst = src_inst->GetItem(idx);
				if(baginst && !database.VerifyInventory(account_id, Inventory::CalcSlotId(src_slot_id, idx), baginst)){
					DeleteItemInInventory(Inventory::CalcSlotId(src_slot_id, idx),0,false);
				}
			}
		}
	}
	if(dst_inst && dst_slot_id >= 2500 && dst_slot_id <= 2550) {
		if(!database.VerifyInventory(account_id, dst_slot_id, dst_inst)) {
			LogFile->write(EQEMuLog::Error, "Player %s on account %s was found exploting the shared bank.\n", GetName(), account_name);
			DeleteItemInInventory(src_slot_id,0,true);
			return(false);
		}
		if(dst_slot_id >= 2500 && dst_slot_id <= 2501 && dst_inst->IsType(ItemClassContainer)){
			for (uint8 idx = 0; idx < 10; idx++) {
				const ItemInst* baginst = dst_inst->GetItem(idx);
				if(baginst && !database.VerifyInventory(account_id, Inventory::CalcSlotId(dst_slot_id, idx), baginst))
					DeleteItemInInventory(Inventory::CalcSlotId(dst_slot_id, idx),0,false);
			}
		}
	}

	Mob* with = trade->With();
	if (((with && with->IsClient() && dst_slot_id >= 3000 && dst_slot_id <= 3007) || (dst_slot_id >= 2500 && dst_slot_id <= 2550)) && GetInv().CheckNoDrop(src_slot_id) && RuleI(World, FVNoDropFlag) == 0 || RuleI(Character, MinStatusForNoDropExemptions) < Admin() && RuleI(World, FVNoDropFlag) == 2) {
		DeleteItemInInventory(src_slot_id);
		WorldKick();
		return false;
	}

	if(m_tradeskill_object != nullptr) {
		if (src_slot_id >= 4000 && src_slot_id <= 4009) {
			ItemInst* inst = m_tradeskill_object->PopItem(Inventory::CalcBagIdx(src_slot_id));
			if (inst) {
				PutItemInInventory(dst_slot_id, *inst, false);
				safe_delete(inst);
			}

			if(RuleB(QueryServ, PlayerLogMoves))
				QSSwapItemAuditor(move_in, true);

			return true;
		}
		else if (dst_slot_id >= 4000 && dst_slot_id <= 4009) {
			uint8 world_idx = Inventory::CalcBagIdx(dst_slot_id);
			ItemInst* world_inst = m_tradeskill_object->PopItem(world_idx);
			
			if (world_inst == nullptr) {
				m_tradeskill_object->PutItem(world_idx, src_inst);
				m_inv.DeleteItem(src_slot_id);
			}
			else {
				const Item_Struct* world_item = world_inst->GetItem();
				const Item_Struct* src_item = src_inst->GetItem();
				if (world_item && src_item) {
					if ((world_item->ID == src_item->ID) && src_inst->IsStackable()) {
						int16 world_charges = world_inst->GetCharges();
						int16 src_charges = src_inst->GetCharges();

						world_charges += src_charges;
						if (world_charges > world_inst->GetItem()->StackSize) {
							src_charges = world_charges - world_inst->GetItem()->StackSize;
							world_charges = world_inst->GetItem()->StackSize;
						}
						else
							src_charges = 0;

						world_inst->SetCharges(world_charges);
						m_tradeskill_object->Save();

						if (src_charges == 0)
							m_inv.DeleteItem(src_slot_id);
						else
							src_inst->SetCharges(src_charges);
					}
					else {
						ItemInst* inv_inst = m_inv.PopItem(src_slot_id);
						m_tradeskill_object->PutItem(world_idx, inv_inst);
						m_inv.PutItem(src_slot_id, *world_inst);
						safe_delete(inv_inst);
					}
				}
			}

			safe_delete(world_inst);
			if (src_slot_id==SLOT_CURSOR) {
				std::list<ItemInst*>::const_iterator s=m_inv.cursor_begin(),e=m_inv.cursor_end();
				database.SaveCursor(character_id, s, e);
			}
			else
				database.SaveInventory(character_id, m_inv[src_slot_id], src_slot_id);

			if(RuleB(QueryServ, PlayerLogMoves))
				QSSwapItemAuditor(move_in, true);

			return true;
		}
	}

	if (dst_slot_id >= 3000 && dst_slot_id <= 3007) {
		if (src_slot_id != SLOT_CURSOR) {
			Kick();
			return false;
		}
		if (with) {
			mlog(INVENTORY__SLOTS, "Trade item move from slot %d to slot %d (trade with %s)", src_slot_id, dst_slot_id, with->GetName());
			if (!m_inv[SLOT_CURSOR]) {
				Message(13, "Error: Cursor item not located on server!");
				return false;
			}

			if(RuleB(QueryServ, PlayerLogMoves))
				QSSwapItemAuditor(move_in);

			trade->AddEntity(src_slot_id, dst_slot_id);

			return true;
		}
		else {
			if(RuleB(QueryServ, PlayerLogMoves))
				QSSwapItemAuditor(move_in);

			SummonItem(src_inst->GetID(), src_inst->GetCharges());
			DeleteItemInInventory(SLOT_CURSOR);

			return true;
		}
	}
	
	if (move_in->number_in_stack > 0) {
		if(src_inst && !src_inst->IsStackable()) {
			mlog(INVENTORY__ERROR, "Move from %d to %d with stack size %d. %s is not a stackable item. (charname: %s)", src_slot_id, dst_slot_id, move_in->number_in_stack, src_inst->GetItem()->Name, GetName());
			return false;
		}

		if (dst_inst) {
			if(src_inst->GetID() != dst_inst->GetID()) {
				mlog(INVENTORY__ERROR, "Move from %d to %d with stack size %d. Incompatible item types: %d != %d", src_slot_id, dst_slot_id, move_in->number_in_stack, src_inst->GetID(), dst_inst->GetID());
				return(false);
			}
			if(dst_inst->GetCharges() < dst_inst->GetItem()->StackSize) {
				mlog(INVENTORY__SLOTS, "Move from %d to %d with stack size %d. dest has %d/%d charges", src_slot_id, dst_slot_id, move_in->number_in_stack, dst_inst->GetCharges(), dst_inst->GetItem()->StackSize);
				uint16 usedcharges = dst_inst->GetItem()->StackSize - dst_inst->GetCharges();
				if (usedcharges > move_in->number_in_stack)
					usedcharges = move_in->number_in_stack;

				dst_inst->SetCharges(dst_inst->GetCharges() + usedcharges);
				src_inst->SetCharges(src_inst->GetCharges() - usedcharges);
				
				if (src_inst->GetCharges() < 1) {
					mlog(INVENTORY__SLOTS, "Dest (%d) now has %d charges, source (%d) was entirely consumed. (%d moved)", dst_slot_id, dst_inst->GetCharges(), src_slot_id, usedcharges);
					database.SaveInventory(CharacterID(),nullptr,src_slot_id);
					m_inv.DeleteItem(src_slot_id);
				}
				else
					mlog(INVENTORY__SLOTS, "Dest (%d) now has %d charges, source (%d) has %d (%d moved)", dst_slot_id, dst_inst->GetCharges(), src_slot_id, src_inst->GetCharges(), usedcharges);
			}
			else {
				mlog(INVENTORY__ERROR, "Move from %d to %d with stack size %d. Exceeds dest maximum stack size: %d/%d", src_slot_id, dst_slot_id, move_in->number_in_stack, (src_inst->GetCharges()+dst_inst->GetCharges()), dst_inst->GetItem()->StackSize);
				return false;
			}
		}
		else {
			if ((int16)move_in->number_in_stack >= src_inst->GetCharges()) {
				if(!m_inv.SwapItem(src_slot_id, dst_slot_id))
					return false;
				mlog(INVENTORY__SLOTS, "Move entire stack from %d to %d with stack size %d. Dest empty.", src_slot_id, dst_slot_id, move_in->number_in_stack);
			}
			else {
				src_inst->SetCharges(src_inst->GetCharges() - move_in->number_in_stack);
				mlog(INVENTORY__SLOTS, "Split stack of %s (%d) from slot %d to %d with stack size %d. Src keeps %d.", src_inst->GetItem()->Name, src_inst->GetItem()->ID, src_slot_id, dst_slot_id, move_in->number_in_stack, src_inst->GetCharges());
				ItemInst* inst = database.CreateItem(src_inst->GetItem(), move_in->number_in_stack);
				m_inv.PutItem(dst_slot_id, *inst);
				safe_delete(inst);
			}
		}
	}
	else {
		if(src_inst && (dst_slot_id < 22 || dst_slot_id == 9999) && dst_slot_id >= 0) {
			if (src_inst->GetItem()->Attuneable)
				src_inst->SetInstNoDrop(true);
			if (src_inst->IsAugmented()) {
				for(int i = 0; i < MAX_AUGMENT_SLOTS; i++) {
					if (src_inst->GetAugment(i)) {
						if (src_inst->GetAugment(i)->GetItem()->Attuneable)
							src_inst->GetAugment(i)->SetInstNoDrop(true);
					}
				}
			}
			SetMaterial(dst_slot_id,src_inst->GetItem()->ID);
		}
		if(!m_inv.SwapItem(src_slot_id, dst_slot_id)) { return false; }
		mlog(INVENTORY__SLOTS, "Moving entire item from slot %d to slot %d", src_slot_id, dst_slot_id);

		if(src_slot_id < 22 || src_slot_id == 9999) {
			if(src_inst)
				parse->EventItem(EVENT_UNEQUIP_ITEM, this, src_inst, nullptr, "", src_slot_id);

			if(dst_inst)
				parse->EventItem(EVENT_EQUIP_ITEM, this, dst_inst, nullptr, "", src_slot_id);
		}

		if(dst_slot_id < 22 || dst_slot_id == 9999) {
			if(dst_inst)
				parse->EventItem(EVENT_UNEQUIP_ITEM, this, dst_inst, nullptr, "", dst_slot_id);

			if(src_inst)
				parse->EventItem(EVENT_EQUIP_ITEM, this, src_inst, nullptr, "", dst_slot_id);
		}
	}

	int matslot = SlotConvert2(dst_slot_id);
	if (dst_slot_id < 22 && matslot != 0)
		SendWearChange(matslot);

	if (src_slot_id == SLOT_CURSOR){
		std::list<ItemInst*>::const_iterator s = m_inv.cursor_begin(), e = m_inv.cursor_end();
		database.SaveCursor(character_id, s, e);
	}
	else
		database.SaveInventory(character_id, m_inv.GetItem(src_slot_id), src_slot_id);
	if (dst_slot_id==SLOT_CURSOR) {
		std::list<ItemInst*>::const_iterator s = m_inv.cursor_begin(), e = m_inv.cursor_end();
		database.SaveCursor(character_id, s, e);
	}
	else
		database.SaveInventory(character_id, m_inv.GetItem(dst_slot_id), dst_slot_id);

	if(RuleB(QueryServ, PlayerLogMoves))
		QSSwapItemAuditor(move_in, true);

	CalcBonuses();
	return true;
}

void Client::SwapItemResync(MoveItem_Struct* move_slots) {
	mlog(INVENTORY__ERROR, "Inventory desyncronization. (charname: %s, source: %i, destination: %i)", GetName(), move_slots->from_slot, move_slots->to_slot);
	Message(15, "Inventory Desyncronization detected: Resending slot data...");

	if((move_slots->from_slot >= 0 && move_slots->from_slot <= 340) || move_slots->from_slot == 9999) {
		int16 resync_slot = (Inventory::CalcSlotId(move_slots->from_slot) == SLOT_INVALID) ? move_slots->from_slot : Inventory::CalcSlotId(move_slots->from_slot);
		if(IsValidSlot(resync_slot) && resync_slot != SLOT_INVALID) {
			const Item_Struct* token_struct = database.GetItem(22292);
			ItemInst* token_inst = database.CreateItem(token_struct, 1);

			SendItemPacket(resync_slot, token_inst, ItemPacketTrade);

			if(m_inv[resync_slot])
				SendItemPacket(resync_slot, m_inv[resync_slot], ItemPacketTrade);
			else {
				EQApplicationPacket* outapp = new EQApplicationPacket(OP_DeleteItem, sizeof(DeleteItem_Struct));
				DeleteItem_Struct* delete_slot = (DeleteItem_Struct*)outapp->pBuffer;
				delete_slot->from_slot = resync_slot;
				delete_slot->to_slot = 0xFFFFFFFF;
				delete_slot->number_in_stack = 0xFFFFFFFF;
				QueuePacket(outapp);
				safe_delete(outapp);
			}
			Message(14, "Source slot %i resyncronized.", move_slots->from_slot);
		}
		else
			Message(13, "Could not resyncronize source slot %i.", move_slots->from_slot);
	}
	else {
		int16 resync_slot = (Inventory::CalcSlotId(move_slots->from_slot) == SLOT_INVALID) ? move_slots->from_slot : Inventory::CalcSlotId(move_slots->from_slot);
		if(IsValidSlot(resync_slot) && resync_slot != SLOT_INVALID) {
			if(m_inv[resync_slot]) {
				const Item_Struct* token_struct = database.GetItem(22292);
				ItemInst* token_inst = database.CreateItem(token_struct, 1);
				SendItemPacket(resync_slot, token_inst, ItemPacketTrade);
				SendItemPacket(resync_slot, m_inv[resync_slot], ItemPacketTrade);
				Message(14, "Source slot %i resyncronized.", move_slots->from_slot);
			}
			else
				Message(13, "Could not resyncronize source slot %i.", move_slots->from_slot);
		}
		else
			Message(13, "Could not resyncronize source slot %i.", move_slots->from_slot);
	}

	if((move_slots->to_slot >= 0 && move_slots->to_slot <= 340) || move_slots->to_slot == 9999) {
		int16 resync_slot = (Inventory::CalcSlotId(move_slots->to_slot) == SLOT_INVALID) ? move_slots->to_slot : Inventory::CalcSlotId(move_slots->to_slot);
		if(IsValidSlot(resync_slot) && resync_slot != SLOT_INVALID) {
			const Item_Struct* token_struct = database.GetItem(22292);
			ItemInst* token_inst = database.CreateItem(token_struct, 1);

			SendItemPacket(resync_slot, token_inst, ItemPacketTrade);

			if(m_inv[resync_slot]) { SendItemPacket(resync_slot, m_inv[resync_slot], ItemPacketTrade); }
			else {
				EQApplicationPacket* outapp = new EQApplicationPacket(OP_DeleteItem, sizeof(DeleteItem_Struct));
				DeleteItem_Struct* delete_slot = (DeleteItem_Struct*)outapp->pBuffer;
				delete_slot->from_slot = resync_slot;
				delete_slot->to_slot = 0xFFFFFFFF;
				delete_slot->number_in_stack = 0xFFFFFFFF;
				QueuePacket(outapp);
				safe_delete(outapp);
			}
			Message(14, "Destination slot %i resyncronized.", move_slots->to_slot);
		}
		else
			Message(13, "Could not resyncronize destination slot %i.", move_slots->to_slot);
	}
	else {
		int16 resync_slot = (Inventory::CalcSlotId(move_slots->to_slot) == SLOT_INVALID) ? move_slots->to_slot : Inventory::CalcSlotId(move_slots->to_slot);
		if(IsValidSlot(resync_slot) && resync_slot != SLOT_INVALID) {
			if(m_inv[resync_slot]) {
				const Item_Struct* token_struct = database.GetItem(22292);
				ItemInst* token_inst = database.CreateItem(token_struct, 1);
				SendItemPacket(resync_slot, token_inst, ItemPacketTrade);
				SendItemPacket(resync_slot, m_inv[resync_slot], ItemPacketTrade);

				Message(14, "Destination slot %i resyncronized.", move_slots->to_slot);
			}
			else
				Message(13, "Could not resyncronize destination slot %i.", move_slots->to_slot);
		}
		else
			Message(13, "Could not resyncronize destination slot %i.", move_slots->to_slot);
	}
}

void Client::QSSwapItemAuditor(MoveItem_Struct* move_in, bool postaction_call) {
	int16 from_slot_id = static_cast<int16>(move_in->from_slot);
	int16 to_slot_id = static_cast<int16>(move_in->to_slot);
	int16 move_amount = static_cast<int16>(move_in->number_in_stack);

	if(!m_inv[from_slot_id] && !m_inv[to_slot_id])
		return;

	uint16 move_count = 0;

	if(m_inv[from_slot_id])
		move_count += m_inv[from_slot_id]->GetTotalItemCount();
	if(to_slot_id != from_slot_id) { 
		if(m_inv[to_slot_id])
			move_count += m_inv[to_slot_id]->GetTotalItemCount();
	}

	ServerPacket* qspack = new ServerPacket(ServerOP_QSPlayerLogMoves, sizeof(QSPlayerLogMove_Struct) + (sizeof(QSMoveItems_Struct) * move_count));
	QSPlayerLogMove_Struct* qsaudit = (QSPlayerLogMove_Struct*)qspack->pBuffer;

	qsaudit->char_id = character_id;
	qsaudit->stack_size = move_amount;
	qsaudit->char_count = move_count;
	qsaudit->postaction = postaction_call;
	qsaudit->from_slot = from_slot_id;
	qsaudit->to_slot = to_slot_id;

	move_count = 0;

	const ItemInst* from_inst = m_inv[postaction_call?to_slot_id:from_slot_id];

	if(from_inst) {
		qsaudit->items[move_count].from_slot = from_slot_id;
		qsaudit->items[move_count].to_slot = to_slot_id;
		qsaudit->items[move_count].item_id = from_inst->GetID();
		qsaudit->items[move_count].charges = from_inst->GetCharges();
		qsaudit->items[move_count].aug_1 = from_inst->GetAugmentItemID(1);
		qsaudit->items[move_count].aug_2 = from_inst->GetAugmentItemID(2);
		qsaudit->items[move_count].aug_3 = from_inst->GetAugmentItemID(3);
		qsaudit->items[move_count].aug_4 = from_inst->GetAugmentItemID(4);
		qsaudit->items[move_count++].aug_5 = from_inst->GetAugmentItemID(5);

		if(from_inst->IsType(ItemClassContainer)) {
			for(uint8 bag_idx = 0; bag_idx < from_inst->GetItem()->BagSlots; bag_idx++) {
				const ItemInst* from_baginst = from_inst->GetItem(bag_idx);

				if(from_baginst) {
					qsaudit->items[move_count].from_slot = Inventory::CalcSlotId(from_slot_id, bag_idx);
					qsaudit->items[move_count].to_slot = Inventory::CalcSlotId(to_slot_id, bag_idx);
					qsaudit->items[move_count].item_id = from_baginst->GetID();
					qsaudit->items[move_count].charges = from_baginst->GetCharges();
					qsaudit->items[move_count].aug_1 = from_baginst->GetAugmentItemID(1);
					qsaudit->items[move_count].aug_2 = from_baginst->GetAugmentItemID(2);
					qsaudit->items[move_count].aug_3 = from_baginst->GetAugmentItemID(3);
					qsaudit->items[move_count].aug_4 = from_baginst->GetAugmentItemID(4);
					qsaudit->items[move_count++].aug_5 = from_baginst->GetAugmentItemID(5);
				}
			}
		}
	}

	if(to_slot_id != from_slot_id) {
		const ItemInst* to_inst = m_inv[postaction_call?from_slot_id:to_slot_id];

		if(to_inst) {
			qsaudit->items[move_count].from_slot = to_slot_id;
			qsaudit->items[move_count].to_slot = from_slot_id;
			qsaudit->items[move_count].item_id = to_inst->GetID();
			qsaudit->items[move_count].charges = to_inst->GetCharges();
			qsaudit->items[move_count].aug_1 = to_inst->GetAugmentItemID(1);
			qsaudit->items[move_count].aug_2 = to_inst->GetAugmentItemID(2);
			qsaudit->items[move_count].aug_3 = to_inst->GetAugmentItemID(3);
			qsaudit->items[move_count].aug_4 = to_inst->GetAugmentItemID(4);
			qsaudit->items[move_count++].aug_5 = to_inst->GetAugmentItemID(5);

			if(to_inst->IsType(ItemClassContainer)) {
				for(uint8 bag_idx = 0; bag_idx < to_inst->GetItem()->BagSlots; bag_idx++) {
					const ItemInst* to_baginst = to_inst->GetItem(bag_idx);

					if(to_baginst) {
						qsaudit->items[move_count].from_slot = Inventory::CalcSlotId(to_slot_id, bag_idx);
						qsaudit->items[move_count].to_slot = Inventory::CalcSlotId(from_slot_id, bag_idx);
						qsaudit->items[move_count].item_id = to_baginst->GetID();
						qsaudit->items[move_count].charges = to_baginst->GetCharges();
						qsaudit->items[move_count].aug_1 = to_baginst->GetAugmentItemID(1);
						qsaudit->items[move_count].aug_2 = to_baginst->GetAugmentItemID(2);
						qsaudit->items[move_count].aug_3 = to_baginst->GetAugmentItemID(3);
						qsaudit->items[move_count].aug_4 = to_baginst->GetAugmentItemID(4);
						qsaudit->items[move_count++].aug_5 = to_baginst->GetAugmentItemID(5);
					}
				}
			}
		}
	}

	if(move_count && worldserver.Connected()) {
		qspack->Deflate();
		worldserver.SendPacket(qspack);
	}

	safe_delete(qspack);
}

void Client::DyeArmor(DyeStruct* dye){
	int16 slot=0;
	for(int i = 0;i < 7;i++) {
		if(m_pp.item_tint[i].rgb.blue!=dye->dye[i].rgb.blue || m_pp.item_tint[i].rgb.red!=dye->dye[i].rgb.red || m_pp.item_tint[i].rgb.green != dye->dye[i].rgb.green) {
			slot = m_inv.HasItem(32557, 1, invWherePersonal);
			if(slot != SLOT_INVALID) {
				DeleteItemInInventory(slot,1,true);
				uint8 slot2=SlotConvert(i);
				ItemInst* inst = this->m_inv.GetItem(slot2);
				if(inst) {
					inst->SetColor((dye->dye[i].rgb.red * 65536) + (dye->dye[i].rgb.green * 256) + (dye->dye[i].rgb.blue));
					database.SaveInventory(CharacterID(),inst,slot2);
					if(dye->dye[i].rgb.use_tint)
						m_pp.item_tint[i].rgb.use_tint = 0xFF;
					else
						m_pp.item_tint[i].rgb.use_tint = 0x00;
				}
				m_pp.item_tint[i].rgb.blue=dye->dye[i].rgb.blue;
				m_pp.item_tint[i].rgb.red=dye->dye[i].rgb.red;
				m_pp.item_tint[i].rgb.green=dye->dye[i].rgb.green;
				SendWearChange(i);
			}
			else {
				Message(13,"Could not locate A Vial of Prismatic Dye.");
				return;
			}
		}
	}
	EQApplicationPacket* outapp=new EQApplicationPacket(OP_Dye,0);
	QueuePacket(outapp);
	safe_delete(outapp);
	Save();
}

bool Client::DecreaseByID(uint32 type, uint8 amt) {
	const Item_Struct* TempItem = 0;
	ItemInst* ins;
	int x;
	int num = 0;
	for(x = 0; x < 331; x++) {
		if (x == 31)
			x = 251;
		TempItem = 0;
		ins = GetInv().GetItem(x);
		if (ins)
			TempItem = ins->GetItem();
		if (TempItem && TempItem->ID == type) {
			num += ins->GetCharges();
			if (num >= amt)
				break;
		}
	}
	if (num < amt)
		return false;
	for(x = 0; x < 331; x++) {
		if (x == 31)
			x = 251;
		TempItem = 0;
		ins = GetInv().GetItem(x);
		if (ins)
			TempItem = ins->GetItem();
		if (TempItem && TempItem->ID == type) {
			if (ins->GetCharges() < amt) {
				amt -= ins->GetCharges();
				DeleteItemInInventory(x,amt,true);
			}
			else {
				DeleteItemInInventory(x,amt,true);
				amt = 0;
			}
			if (amt < 1)
				break;
		}
	}
	return true;
}

void Client::RemoveNoRent(bool client_update) {
	int16 slot_id;
	for(slot_id = 0; slot_id <= 30; slot_id++) {
		const ItemInst* inst = m_inv[slot_id];
		if(inst && !inst->GetItem()->NoRent) {
			mlog(INVENTORY__SLOTS, "NoRent Timer Lapse: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
			DeleteItemInInventory(slot_id, 0, client_update);
		}
	}

	const ItemInst* inst = m_inv[9999];
	if(inst && !inst->GetItem()->NoRent) {
		mlog(INVENTORY__SLOTS, "NoRent Timer Lapse: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
		DeleteItemInInventory(9999, 0, client_update);
	}

	for(slot_id = 251; slot_id <= 340; slot_id++) {
		const ItemInst* inst = m_inv[slot_id];
		if(inst && !inst->GetItem()->NoRent) {
			mlog(INVENTORY__SLOTS, "NoRent Timer Lapse: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
			DeleteItemInInventory(slot_id, 0, client_update);
		}
	}
	
	for(slot_id = 2000; slot_id <= 2023; slot_id++) {
		const ItemInst* inst = m_inv[slot_id];
		if(inst && !inst->GetItem()->NoRent) {
			mlog(INVENTORY__SLOTS, "NoRent Timer Lapse: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
			DeleteItemInInventory(slot_id, 0, false);
		}
	}

	for(slot_id = 2031; slot_id <= 2270; slot_id++) {
		const ItemInst* inst = m_inv[slot_id];
		if(inst && !inst->GetItem()->NoRent) {
			mlog(INVENTORY__SLOTS, "NoRent Timer Lapse: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
			DeleteItemInInventory(slot_id, 0, false);
		}
	}

	for(slot_id = 2500; slot_id <= 2501; slot_id++) {
		const ItemInst* inst = m_inv[slot_id];
		if(inst && !inst->GetItem()->NoRent) {
			mlog(INVENTORY__SLOTS, "NoRent Timer Lapse: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
			DeleteItemInInventory(slot_id, 0, false);
		}
	}

	for(slot_id = 2531; slot_id <= 2550; slot_id++) {
		const ItemInst* inst = m_inv[slot_id];
		if(inst && !inst->GetItem()->NoRent) {
			mlog(INVENTORY__SLOTS, "NoRent Timer Lapse: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
			DeleteItemInInventory(slot_id, 0, false);
		}
	}
}

void Client::RemoveDuplicateLore(bool client_update) {
	int16 slot_id;
	for(slot_id = 0; slot_id <= 30; slot_id++) {
		ItemInst* inst = m_inv.PopItem(slot_id);
		if(inst) {
			if(CheckLoreConflict(inst->GetItem())) {
				mlog(INVENTORY__ERROR, "Lore Duplication Error: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
				database.SaveInventory(character_id, nullptr, slot_id);
			}
			else
				m_inv.PutItem(slot_id, *inst);
			safe_delete(inst);
		}
	}

	ItemInst* inst = m_inv.PopItem(9999);
	if(inst) {
		if(CheckLoreConflict(inst->GetItem())) {
			mlog(INVENTORY__ERROR, "Lore Duplication Error: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
			database.SaveInventory(character_id, nullptr, 9999);
		}
		else
			m_inv.PutItem(9999, *inst);
		safe_delete(inst);
	}

	for(slot_id = 251; slot_id <= 340; slot_id++) {
		ItemInst* inst = m_inv.PopItem(slot_id);
		if(inst) {
			if(CheckLoreConflict(inst->GetItem())) {
				mlog(INVENTORY__ERROR, "Lore Duplication Error: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
				database.SaveInventory(character_id, nullptr, slot_id);
			}
			else
				m_inv.PutItem(slot_id, *inst);
			safe_delete(inst);
		}
	}

	for(slot_id = 2000; slot_id <= 2023; slot_id++) {
		ItemInst* inst = m_inv.PopItem(slot_id);
		if(inst) {
			if(CheckLoreConflict(inst->GetItem())) {
				mlog(INVENTORY__ERROR, "Lore Duplication Error: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
				database.SaveInventory(character_id, nullptr, slot_id);
			}
			else
				m_inv.PutItem(slot_id, *inst);
			safe_delete(inst);
		}
	}

	for(slot_id = 2031; slot_id <= 2270; slot_id++) {
		ItemInst* inst = m_inv.PopItem(slot_id);
		if(inst) {
			if(CheckLoreConflict(inst->GetItem())) {
				mlog(INVENTORY__ERROR, "Lore Duplication Error: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
				database.SaveInventory(character_id, nullptr, slot_id);
			}
			else
				m_inv.PutItem(slot_id, *inst);
			safe_delete(inst);
		}
	}
}

void Client::MoveSlotNotAllowed(bool client_update) {
	int16 slot_id;
	for(slot_id = 0; slot_id <= 21; slot_id++) {
		if(m_inv[slot_id] && !m_inv[slot_id]->IsSlotAllowed(slot_id)) {
			ItemInst* inst = m_inv.PopItem(slot_id);
			bool is_arrow = (inst->GetItem()->ItemType == ItemTypeArrow) ? true : false;
			int16 free_slot_id = m_inv.FindFreeSlot(inst->IsType(ItemClassContainer), true, inst->GetItem()->Size, is_arrow);
			mlog(INVENTORY__ERROR, "Slot Assignment Error: Moving %s from slot %i to %i", inst->GetItem()->Name, slot_id, free_slot_id);
			PutItemInInventory(free_slot_id, *inst, client_update);
			database.SaveInventory(character_id, nullptr, slot_id);
			safe_delete(inst);
		}
	}
	
	slot_id = 9999;
	if(m_inv[slot_id] && !m_inv[slot_id]->IsSlotAllowed(slot_id)) {
		ItemInst* inst = m_inv.PopItem(slot_id);
		bool is_arrow = (inst->GetItem()->ItemType == ItemTypeArrow) ? true : false;
		int16 free_slot_id = m_inv.FindFreeSlot(inst->IsType(ItemClassContainer), true, inst->GetItem()->Size, is_arrow);
		mlog(INVENTORY__ERROR, "Slot Assignment Error: Moving %s from slot %i to %i", inst->GetItem()->Name, slot_id, free_slot_id);
		PutItemInInventory(free_slot_id, *inst, (GetClientVersion() >= EQClientSoF) ? client_update : false);
		database.SaveInventory(character_id, nullptr, slot_id);
		safe_delete(inst);
	}
}

uint32 Client::GetEquipment(uint8 material_slot) const {
	int invslot;
	const ItemInst *item;

	if(material_slot > 8)
		return 0;

	invslot = Inventory::CalcSlotFromMaterial(material_slot);
	if(invslot == -1)
		return 0;

	item = m_inv.GetItem(invslot);

	if(item && item->GetItem())
		return item->GetItem()->ID;

	return 0;
}

uint32 Client::GetEquipmentColor(uint8 material_slot) const {
	const Item_Struct *item;

	if(material_slot > 8)
		return 0;

	item = database.GetItem(GetEquipment(material_slot));
	if(item != 0)
		return m_pp.item_tint[material_slot].rgb.use_tint ? m_pp.item_tint[material_slot].color : item->Color;

	return 0;
}

void Client::SendItemPacket(int16 slot_id, const ItemInst* inst, ItemPacketType packet_type) {
	if (!inst)
		return;

	std::string packet = inst->Serialize(slot_id);
	EmuOpcode opcode = OP_Unknown;
	EQApplicationPacket* outapp = nullptr;
	ItemPacket_Struct* itempacket = nullptr;
	opcode = (packet_type == ItemPacketViewLink) ? OP_ItemLinkResponse : OP_ItemPacket;
	outapp = new EQApplicationPacket(opcode, packet.length()+sizeof(ItemPacket_Struct));
	itempacket = (ItemPacket_Struct*)outapp->pBuffer;
	memcpy(itempacket->SerializedItem, packet.c_str(), packet.length());
	itempacket->PacketType = packet_type;

#if EQDEBUG >= 9
		DumpPacket(outapp);
#endif
	FastQueuePacket(&outapp);
}

EQApplicationPacket* Client::ReturnItemPacket(int16 slot_id, const ItemInst* inst, ItemPacketType packet_type) {
	if (!inst)
		return 0;

	std::string packet = inst->Serialize(slot_id);
	EmuOpcode opcode = OP_Unknown;
	EQApplicationPacket* outapp = nullptr;
	BulkItemPacket_Struct* itempacket = nullptr;
	opcode = OP_ItemPacket;
	outapp = new EQApplicationPacket(opcode, packet.length()+1);
	itempacket = (BulkItemPacket_Struct*)outapp->pBuffer;
	memcpy(itempacket->SerializedItem, packet.c_str(), packet.length());

#if EQDEBUG >= 9
		DumpPacket(outapp);
#endif

	return outapp;
}

static int16 BandolierSlotToWeaponSlot(int BandolierSlot) {
	switch(BandolierSlot) {
		case bandolierMainHand:
			return SLOT_PRIMARY;
		case bandolierOffHand:
			return SLOT_SECONDARY;
		case bandolierRange:
			return SLOT_RANGE;
		default:
			return SLOT_AMMO;
	}
}

void Client::CreateBandolier(const EQApplicationPacket *app) {
	BandolierCreate_Struct *bs = (BandolierCreate_Struct*)app->pBuffer;
	_log(INVENTORY__BANDOLIER, "Char: %s Creating Bandolier Set %i, Set Name: %s", GetName(), bs->number, bs->name);
	strcpy(m_pp.bandoliers[bs->number].name, bs->name);
	const ItemInst* InvItem;
	const Item_Struct *BaseItem;
	int16 WeaponSlot;
	for(int BandolierSlot = bandolierMainHand; BandolierSlot <= bandolierAmmo; BandolierSlot++) {
		WeaponSlot = BandolierSlotToWeaponSlot(BandolierSlot);
		InvItem = GetInv()[WeaponSlot];
		if(InvItem) {
			BaseItem = InvItem->GetItem();
			_log(INVENTORY__BANDOLIER, "Char: %s adding item %s to slot %i", GetName(),BaseItem->Name, WeaponSlot);
			m_pp.bandoliers[bs->number].items[BandolierSlot].item_id = BaseItem->ID;
			m_pp.bandoliers[bs->number].items[BandolierSlot].icon = BaseItem->Icon;
		}
		else {
			_log(INVENTORY__BANDOLIER, "Char: %s no item in slot %i", GetName(), WeaponSlot);
			m_pp.bandoliers[bs->number].items[BandolierSlot].item_id = 0;
			m_pp.bandoliers[bs->number].items[BandolierSlot].icon = 0;
		}
	}
	Save();
}

void Client::RemoveBandolier(const EQApplicationPacket *app) {
	BandolierDelete_Struct *bds = (BandolierDelete_Struct*)app->pBuffer;
	_log(INVENTORY__BANDOLIER, "Char: %s removing set", GetName(), bds->number);
	memset(m_pp.bandoliers[bds->number].name, 0, 32);
	for(int i = bandolierMainHand; i <= bandolierAmmo; i++) {
		m_pp.bandoliers[bds->number].items[i].item_id = 0;
		m_pp.bandoliers[bds->number].items[i].icon = 0;
	}
	Save();
}

void Client::SetBandolier(const EQApplicationPacket *app) {
	BandolierSet_Struct *bss = (BandolierSet_Struct*)app->pBuffer;
	_log(INVENTORY__BANDOLIER, "Char: %s activating set %i", GetName(), bss->number);
	int16 slot;
	int16 WeaponSlot;
	ItemInst *BandolierItems[4];
	for(int BandolierSlot = bandolierMainHand; BandolierSlot <= bandolierAmmo; BandolierSlot++) {
		if(m_pp.bandoliers[bss->number].items[BandolierSlot].item_id) {
			WeaponSlot = BandolierSlotToWeaponSlot(BandolierSlot);
			slot = m_inv.HasItem(m_pp.bandoliers[bss->number].items[BandolierSlot].item_id, 1, invWhereWorn|invWherePersonal);
			if (slot == SLOT_INVALID) {
				if (m_inv.GetItem(SLOT_CURSOR)) {
					if (m_inv.GetItem(SLOT_CURSOR)->GetItem()->ID == m_pp.bandoliers[bss->number].items[BandolierSlot].item_id && m_inv.GetItem(SLOT_CURSOR)->GetCharges() >= 1)
							slot = SLOT_CURSOR;
					else if (m_inv.GetItem(SLOT_CURSOR)->GetItem()->ItemClass == 1) {
						for(int16 CursorBagSlot = 331; CursorBagSlot <= 340; CursorBagSlot++) {
							if (m_inv.GetItem(CursorBagSlot)) {
								if (m_inv.GetItem(CursorBagSlot)->GetItem()->ID == m_pp.bandoliers[bss->number].items[BandolierSlot].item_id && m_inv.GetItem(CursorBagSlot)->GetCharges() >= 1) {
										slot = CursorBagSlot;
										break;
								}
							}
						}
					}
				}
			}

			if(slot != SLOT_INVALID) {
				BandolierItems[BandolierSlot] = m_inv.PopItem(slot);
				if(((BandolierSlot == bandolierAmmo) || (BandolierSlot == bandolierRange)) && BandolierItems[BandolierSlot] && BandolierItems[BandolierSlot]->IsStackable()) {
					int Charges = BandolierItems[BandolierSlot]->GetCharges();
					if(Charges > 1) {
						BandolierItems[BandolierSlot]->SetCharges(Charges - 1);
						m_inv.PutItem(slot, *BandolierItems[BandolierSlot]);
						database.SaveInventory(character_id, BandolierItems[BandolierSlot], slot);
						BandolierItems[BandolierSlot]->SetCharges(1);
					}
					else
						database.SaveInventory(character_id, 0, slot);
				}
				else
					database.SaveInventory(character_id, 0, slot);
			}
			else {
				BandolierItems[BandolierSlot] = 0;
				if(slot == SLOT_INVALID) {
					_log(INVENTORY__BANDOLIER, "Character does not have required bandolier item for slot %i", WeaponSlot);
					ItemInst *InvItem = m_inv.PopItem(WeaponSlot);
					if(InvItem) {
						_log(INVENTORY__BANDOLIER, "returning item %s in weapon slot %i to inventory", InvItem->GetItem()->Name, WeaponSlot);
						if(MoveItemToInventory(InvItem))
							database.SaveInventory(character_id, 0, WeaponSlot);
						else
							_log(INVENTORY__BANDOLIER, "Char: %s, ERROR returning %s to inventory", GetName(), InvItem->GetItem()->Name);
						safe_delete(InvItem);
					}

				}
			}
		}
	}

	for(int BandolierSlot = bandolierMainHand; BandolierSlot <= bandolierAmmo; BandolierSlot++) {
		WeaponSlot = BandolierSlotToWeaponSlot(BandolierSlot);
		if(m_pp.bandoliers[bss->number].items[BandolierSlot].item_id) {
			if(BandolierItems[BandolierSlot]) {
				ItemInst *InvItem = m_inv.PopItem(WeaponSlot);
				m_inv.PutItem(WeaponSlot, *BandolierItems[BandolierSlot]);
				safe_delete(BandolierItems[BandolierSlot]);
				database.SaveInventory(character_id, m_inv.GetItem(WeaponSlot), WeaponSlot);

				if(InvItem) {
					if(!MoveItemToInventory(InvItem))
						_log(INVENTORY__BANDOLIER, "Char: %s, ERROR returning %s to inventory", GetName(), InvItem->GetItem()->Name);
					safe_delete(InvItem);
				}
			}
		}
		else {
			ItemInst *InvItem = m_inv.PopItem(WeaponSlot);
			if(InvItem) {
				_log(INVENTORY__BANDOLIER, "Bandolier has no item for slot %i, returning item %s to inventory", WeaponSlot, InvItem->GetItem()->Name);
				if(MoveItemToInventory(InvItem))
					database.SaveInventory(character_id, 0, WeaponSlot);
				else
					_log(INVENTORY__BANDOLIER, "Char: %s, ERROR returning %s to inventory", GetName(), InvItem->GetItem()->Name);
				safe_delete(InvItem);
			}
		}
	}
	CalcBonuses();
}

bool Client::MoveItemToInventory(ItemInst *ItemToReturn, bool UpdateClient) {
	if(!ItemToReturn)
		return false;

	_log(INVENTORY__SLOTS,"Char: %s Returning %s to inventory", GetName(), ItemToReturn->GetItem()->Name);

	uint32 ItemID = ItemToReturn->GetItem()->ID;

	if(ItemToReturn->IsStackable()) {
		for (int16 i=22; i<=30; i++) {
			ItemInst* InvItem = m_inv.GetItem(i);
			if(InvItem && (InvItem->GetItem()->ID == ItemID) && (InvItem->GetCharges() < InvItem->GetItem()->StackSize)) {
				int ChargeSlotsLeft = InvItem->GetItem()->StackSize - InvItem->GetCharges();
				int ChargesToMove = ItemToReturn->GetCharges() < ChargeSlotsLeft ? ItemToReturn->GetCharges() : ChargeSlotsLeft;
				InvItem->SetCharges(InvItem->GetCharges() + ChargesToMove);
				if(UpdateClient)
					SendItemPacket(i, InvItem, ItemPacketTrade);

				database.SaveInventory(character_id, m_inv.GetItem(i), i);
				ItemToReturn->SetCharges(ItemToReturn->GetCharges() - ChargesToMove);
				if(!ItemToReturn->GetCharges())
					return true;
			}
			if (InvItem && InvItem->IsType(ItemClassContainer)) {
				int16 BaseSlotID = Inventory::CalcSlotId(i, 0);
				uint8 BagSize = InvItem->GetItem()->BagSlots;
				uint8 BagSlot;
				for (BagSlot = 0; BagSlot < BagSize; BagSlot++) {
					InvItem = m_inv.GetItem(BaseSlotID + BagSlot);
					if (InvItem && (InvItem->GetItem()->ID == ItemID) &&
						(InvItem->GetCharges() < InvItem->GetItem()->StackSize)) {
						int ChargeSlotsLeft = InvItem->GetItem()->StackSize - InvItem->GetCharges();
						int ChargesToMove = ItemToReturn->GetCharges() < ChargeSlotsLeft ? ItemToReturn->GetCharges() : ChargeSlotsLeft;
						InvItem->SetCharges(InvItem->GetCharges() + ChargesToMove);

						if(UpdateClient)
							SendItemPacket(BaseSlotID + BagSlot, m_inv.GetItem(BaseSlotID + BagSlot), ItemPacketTrade);

						database.SaveInventory(character_id, m_inv.GetItem(BaseSlotID + BagSlot), BaseSlotID + BagSlot);
						ItemToReturn->SetCharges(ItemToReturn->GetCharges() - ChargesToMove);
						if(!ItemToReturn->GetCharges())
							return true;
					}
				}
			}
		}
	}

	for (int16 i = 22; i <= 30; i++) {
		ItemInst* InvItem = m_inv.GetItem(i);
		if (!InvItem) {
			m_inv.PutItem(i, *ItemToReturn);
			if(UpdateClient)
				SendItemPacket(i, ItemToReturn, ItemPacketTrade);

			database.SaveInventory(character_id, m_inv.GetItem(i), i);
			_log(INVENTORY__SLOTS, "Char: %s Storing in main inventory slot %i", GetName(), i);
			return true;
		}
		if(InvItem->IsType(ItemClassContainer) && Inventory::CanItemFitInContainer(ItemToReturn->GetItem(), InvItem->GetItem())) {
			int16 BaseSlotID = Inventory::CalcSlotId(i, 0);
			uint8 BagSize=InvItem->GetItem()->BagSlots;
			for (uint8 BagSlot=0; BagSlot<BagSize; BagSlot++) {
				InvItem = m_inv.GetItem(BaseSlotID + BagSlot);
				if (!InvItem) {
					m_inv.PutItem(BaseSlotID + BagSlot, *ItemToReturn);
					if(UpdateClient)
						SendItemPacket(BaseSlotID + BagSlot, ItemToReturn, ItemPacketTrade);

					database.SaveInventory(character_id, m_inv.GetItem(BaseSlotID + BagSlot), BaseSlotID + BagSlot);
					_log(INVENTORY__SLOTS, "Char: %s Storing in bag slot %i", GetName(), BaseSlotID + BagSlot);
					return true;
				}
			}
		}
	}
	_log(INVENTORY__SLOTS, "Char: %s No space, putting on the cursor", GetName());
	PushItemOnCursor(*ItemToReturn, UpdateClient);
	return true;
}

void Inventory::SetCustomItemData(uint32 character_id, int16 slot_id, std::string identifier, std::string value) {
	ItemInst *inst = GetItem(slot_id);
	if(inst) {
		inst->SetCustomData(identifier, value);
		database.SaveInventory(character_id, inst, slot_id);
	}
}

void Inventory::SetCustomItemData(uint32 character_id, int16 slot_id, std::string identifier, int value) {
	ItemInst *inst = GetItem(slot_id);
	if(inst) {
		inst->SetCustomData(identifier, value);
		database.SaveInventory(character_id, inst, slot_id);
	}
}

void Inventory::SetCustomItemData(uint32 character_id, int16 slot_id, std::string identifier, float value) {
	ItemInst *inst = GetItem(slot_id);
	if(inst) {
		inst->SetCustomData(identifier, value);
		database.SaveInventory(character_id, inst, slot_id);
	}
}

void Inventory::SetCustomItemData(uint32 character_id, int16 slot_id, std::string identifier, bool value) {
	ItemInst *inst = GetItem(slot_id);
	if(inst) {
		inst->SetCustomData(identifier, value);
		database.SaveInventory(character_id, inst, slot_id);
	}
}

std::string Inventory::GetCustomItemData(int16 slot_id, std::string identifier) {
	ItemInst *inst = GetItem(slot_id);
	if(inst)
		return inst->GetCustomData(identifier);
	return "";
}