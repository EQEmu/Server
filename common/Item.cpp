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

#include "debug.h"
#include "StringUtil.h"
#include "Item.h"
#include "database.h"
#include "misc.h"
#include "races.h"
#include "shareddb.h"
#include "classes.h"

#include <limits.h>

#include <sstream>
#include <iostream>

std::list<ItemInst*> dirty_inst;
int32 NextItemInstSerialNumber = 1;

static inline int32 GetNextItemInstSerialNumber() {

	// The Bazaar relies on each item a client has up for Trade having a unique
	// identifier. This 'SerialNumber' is sent in Serialized item packets and
	// is used in Bazaar packets to identify the item a player is buying or inspecting.
	//
	// E.g. A trader may have 3 Five dose cloudy potions, each with a different number of remaining charges
	// up for sale with different prices.
	//
	// NextItemInstSerialNumber is the next one to hand out.
	//
	// It is very unlikely to reach 2,147,483,647. Maybe we should call abort(), rather than wrapping back to 1.
	if(NextItemInstSerialNumber >= INT_MAX)
		NextItemInstSerialNumber = 1;
	else
		NextItemInstSerialNumber++;

	return NextItemInstSerialNumber;
}

ItemInst::ItemInst(const Item_Struct* item, int16 charges) {
	m_use_type = ItemInstNormal;
	m_item = item;
	m_charges = charges;
	m_price = 0;
	m_instnodrop = false;
	m_merchantslot = 0;
	if(m_item &&m_item->ItemClass == ItemClassCommon)
		m_color = m_item->Color;
	else
		m_color = 0;
	m_merchantcount = 1;
	m_SerialNumber = GetNextItemInstSerialNumber();

	m_exp = 0;
	m_evolveLvl = 0;
	m_activated = false;
	m_scaledItem = nullptr;
	m_evolveInfo = nullptr;
	m_scaling = false;
}

ItemInst::ItemInst(SharedDatabase *db, uint32 item_id, int16 charges) {
	m_use_type = ItemInstNormal;
	m_item = db->GetItem(item_id);
	m_charges = charges;
	m_price = 0;
	m_merchantslot = 0;
	m_instnodrop=false;
	if(m_item && m_item->ItemClass == ItemClassCommon)
		m_color = m_item->Color;
	else
		m_color = 0;
	m_merchantcount = 1;
	m_SerialNumber = GetNextItemInstSerialNumber();

	m_exp = 0;
	m_evolveLvl = 0;
	m_activated = false;
	m_scaledItem = nullptr;
	m_evolveInfo = nullptr;
	m_scaling = false;
}

ItemInstQueue::~ItemInstQueue() {
	iter_queue cur,end;
	cur = m_list.begin();
	end = m_list.end();
	for(; cur != end; ++cur) {
		ItemInst *tmp = * cur;
		safe_delete(tmp);
	}
	m_list.clear();
}

Inventory::~Inventory() {
	std::map<int16, ItemInst*>::iterator cur,end;

	cur = m_worn.begin();
	end = m_worn.end();
	for(; cur != end; ++cur) {
		ItemInst *tmp = cur->second;
		safe_delete(tmp);
	}
	m_worn.clear();

	cur = m_inv.begin();
	end = m_inv.end();
	for(; cur != end; ++cur) {
		ItemInst *tmp = cur->second;
		safe_delete(tmp);
	}
	m_inv.clear();

	cur = m_bank.begin();
	end = m_bank.end();
	for(; cur != end; ++cur) {
		ItemInst *tmp = cur->second;
		safe_delete(tmp);
	}
	m_bank.clear();

	cur = m_shbank.begin();
	end = m_shbank.end();
	for(; cur != end; ++cur) {
		ItemInst *tmp = cur->second;
		safe_delete(tmp);
	}
	m_shbank.clear();

	cur = m_trade.begin();
	end = m_trade.end();
	for(; cur != end; ++cur) {
		ItemInst *tmp = cur->second;
		safe_delete(tmp);
	}
	m_trade.clear();
}

// Make a copy of an ItemInst object
ItemInst::ItemInst(const ItemInst& copy)
{
	m_use_type=copy.m_use_type;
	m_item=copy.m_item;
	m_charges=copy.m_charges;
	m_price=copy.m_price;
	m_color=copy.m_color;
	m_merchantslot=copy.m_merchantslot;
	m_currentslot=copy.m_currentslot;
	m_instnodrop=copy.m_instnodrop;
	m_merchantcount=copy.m_merchantcount;
	// Copy container contents
	iter_contents it;
	for (it=copy.m_contents.begin(); it!=copy.m_contents.end(); ++it) {
		ItemInst* inst_old = it->second;
		ItemInst* inst_new = nullptr;

		if (inst_old) {
			inst_new = inst_old->Clone();
		}

		if (inst_new != nullptr) {
			m_contents[it->first] = inst_new;
		}
	}
	std::map<std::string, std::string>::const_iterator iter;
	for (iter = copy.m_custom_data.begin(); iter != copy.m_custom_data.end(); ++iter) {
		m_custom_data[iter->first] = iter->second;
	}
	m_SerialNumber = copy.m_SerialNumber;
	m_custom_data = copy.m_custom_data;
	m_timers = copy.m_timers;

	m_exp = copy.m_exp;
	m_evolveLvl = copy.m_evolveLvl;
	m_activated = copy.m_activated;
	if (copy.m_scaledItem)
		m_scaledItem = new Item_Struct(*copy.m_scaledItem);
	else
		m_scaledItem = nullptr;

	if(copy.m_evolveInfo)
		m_evolveInfo = new EvolveInfo(*copy.m_evolveInfo);
	else
		m_evolveInfo = nullptr;

	m_scaling = copy.m_scaling;
}

// Clean up container contents
ItemInst::~ItemInst()
{
	Clear();
	safe_delete(m_scaledItem);
	safe_delete(m_evolveInfo);
}

// Clone a type of ItemInst object
// c++ doesn't allow a polymorphic copy constructor,
// so we have to resort to a polymorphic Clone()
ItemInst* ItemInst::Clone() const
{
	// Pseudo-polymorphic copy constructor
	return new ItemInst(*this);
}

// Query item type
bool ItemInst::IsType(ItemClassTypes item_class) const
{
	// Check usage type
	if ((m_use_type == ItemInstWorldContainer) && (item_class == ItemClassContainer))

		return true;
	if (!m_item)
		return false;

	return (m_item->ItemClass == item_class);
}

// Is item stackable?
bool ItemInst::IsStackable() const
{
	return m_item->Stackable;
}

bool ItemInst::IsCharged() const
{
	if(m_item->MaxCharges > 1)
		return true;
	else
		return false;
}

// Can item be equipped?

bool ItemInst::IsEquipable(uint16 race, uint16 class_) const
{
	if (!m_item || (m_item->Slots == 0))
		return false;

	return m_item->IsEquipable(race, class_);
}

// Can equip at this slot?
bool ItemInst::IsEquipable(int16 slot_id) const
{
	if (!m_item)
		return false;

	if(slot_id == 9999) {
		slot_id = 22;
		uint32 slot_mask = (1 << slot_id);
		if (slot_mask & m_item->Slots)
			return true;
	}

	if (slot_id < 22) {
		uint32 slot_mask = (1 << slot_id);
		if (slot_mask & m_item->Slots)
			return true;
	}

	return false;
}

int8 ItemInst::AvailableAugmentSlot(int32 augtype) const
{
	if (m_item->ItemClass != ItemClassCommon || !m_item)
		return -1;

	int i;
	for (i=0;i<5;i++) {
		if (!GetItem(i)) {
			if (augtype==-1 || (m_item->AugSlotType[i] && ((1<<(m_item->AugSlotType[i]-1)) & augtype)))
				break;
		}

	}

	return (i<5) ? i : -1;
}

bool ItemInst::AvailableWearSlot(uint32 aug_wear_slots) const
{
	if (m_item->ItemClass != ItemClassCommon || !m_item)
		return false;

	int i;
	for(i=0; i<23; i++) {
		if(m_item->Slots & (1<<i)) {
			if(aug_wear_slots & (1<<i))
				break;
		}
	}

	return (i<23) ? true : false;
}

uint32 ItemInst::GetAugmentItemID(uint8 slot) const
{
uint32 id=0;
	if (m_item->ItemClass == ItemClassCommon) {
		return GetItemID(slot);
	}

	return id;
}

uint32 ItemInst::GetItemID(uint8 slot) const
{
const ItemInst *item;
uint32 id=0;
	if ((item=GetItem(slot))!=nullptr)
		id= item->GetItem()->ID;

	return id;
}


// Has attack/delay?
bool ItemInst::IsWeapon() const
{
	if (!m_item || m_item->ItemClass != ItemClassCommon)
		return false;
	if(m_item->ItemType==ItemTypeArrow && m_item->Damage != 0)
		return true;
	else
		return ((m_item->Damage != 0) && (m_item->Delay != 0));
}

bool ItemInst::IsAmmo() const {

	if(!m_item) return false;

	if((m_item->ItemType == ItemTypeArrow) ||
		(m_item->ItemType == ItemTypeLargeThrowing) ||
		(m_item->ItemType == ItemTypeSmallThrowing))
		return true;

	return false;

}

// Retrieve augment inside item
ItemInst* ItemInst::GetAugment(uint8 slot) const
{
	if (m_item->ItemClass == ItemClassCommon)
		return GetItem(slot);

	return nullptr;
}

// Remove augment from item and destroy it
void ItemInst::DeleteAugment(uint8 index)
{
	if (m_item->ItemClass == ItemClassCommon)
		DeleteItem(index);
}

// Remove augment from item and return it
ItemInst* ItemInst::RemoveAugment(uint8 index)
{
	if (m_item->ItemClass == ItemClassCommon)
		return PopItem(index);

	return nullptr;
}

// Add an augment to the item
void ItemInst::PutAugment(uint8 slot, const ItemInst& augment)
{
	if (m_item->ItemClass == ItemClassCommon)
		PutItem(slot,augment);
}

void ItemInst::PutAugment(SharedDatabase *db, uint8 slot, uint32 item_id)
{
	if (item_id != 0) {
		const ItemInst* aug = db->CreateItem(item_id);
		if(aug)
		{
			PutAugment(slot,*aug);
			safe_delete(aug);
		}
	}
}

// Retrieve item inside container
ItemInst* ItemInst::GetItem(uint8 index) const
{
	iter_contents it = m_contents.find(index);
	if (it != m_contents.end()) {
		ItemInst* inst = it->second;
		return inst;
	}

	return nullptr;
}

void ItemInst::PutItem(uint8 index, const ItemInst& inst)
{
	// Clean up item already in slot (if exists)
	DeleteItem(index);


	// Delegate to internal method
	_PutItem(index, inst.Clone());
}

// Remove item inside container
void ItemInst::DeleteItem(uint8 index)
{
	ItemInst* inst = PopItem(index);
	safe_delete(inst);
}

// Remove all items from container
void ItemInst::Clear()
{
	// Destroy container contents
	iter_contents cur, end;
	cur = m_contents.begin();
	end = m_contents.end();
	for (; cur != end; ++cur) {
		ItemInst* inst = cur->second;
		safe_delete(inst);
	}
	m_contents.clear();
}

// Remove all items from container
void ItemInst::ClearByFlags(byFlagSetting is_nodrop, byFlagSetting is_norent)
{
	// Destroy container contents
	iter_contents cur, end, del;
	cur = m_contents.begin();
	end = m_contents.end();
	for (; cur != end;) {
		ItemInst* inst = cur->second;
		const Item_Struct* item = inst->GetItem();
		del = cur;
		++cur;

		switch(is_nodrop) {
		case byFlagSet:
			if (item->NoDrop == 0) {
				safe_delete(inst);
				m_contents.erase(del->first);
				continue;
			}
		case byFlagNotSet:
			if (item->NoDrop != 0) {
				safe_delete(inst);
				m_contents.erase(del->first);
				continue;
			}
		default:
			break;
		}

		switch(is_norent) {
		case byFlagSet:
			if (item->NoRent == 0) {
				safe_delete(inst);
				m_contents.erase(del->first);
				continue;
			}
		case byFlagNotSet:
			if (item->NoRent != 0) {
				safe_delete(inst);
				m_contents.erase(del->first);
				continue;
			}
		default:
			break;
		}
	}
}

// Remove item from container without memory delete
// Hands over memory ownership to client of this function call
ItemInst* ItemInst::PopItem(uint8 index)
{
	iter_contents it = m_contents.find(index);
	if (it != m_contents.end()) {
		ItemInst* inst = it->second;
		m_contents.erase(index);
		return inst;
	}

	// Return pointer that needs to be deleted (or otherwise managed)
	return nullptr;
}

// Put item onto back of queue
void ItemInstQueue::push(ItemInst* inst)
{
	m_list.push_back(inst);
}

// Put item onto front of queue
void ItemInstQueue::push_front(ItemInst* inst)
{
	m_list.push_front(inst);
}

// Remove item from front of queue
ItemInst* ItemInstQueue::pop()
{
	if (m_list.size() == 0)
		return nullptr;

	ItemInst* inst = m_list.front();
	m_list.pop_front();
	return inst;
}

// Look at item at front of queue
ItemInst* ItemInstQueue::peek_front() const
{
	return (m_list.size()==0) ? nullptr : m_list.front();
}

// Retrieve item at specified slot; returns false if item not found
ItemInst* Inventory::GetItem(int16 slot_id) const
{
	ItemInst* result = nullptr;

	// Cursor
	if (slot_id == SLOT_CURSOR) {
		// Cursor slot
		result = m_cursor.peek_front();
	}

	// Non bag slots
	else if (slot_id>=3000 && slot_id<=3007) {
		// Trade slots
		result = _GetItem(m_trade, slot_id);
	}
	else if (slot_id>=2500 && slot_id<=2501) {
		// Shared Bank slots
		result = _GetItem(m_shbank, slot_id);
	}
	else if (slot_id>=2000 && slot_id<=2023) {
		// Bank slots
		result = _GetItem(m_bank, slot_id);
	}
	else if ((slot_id>=22 && slot_id<=29)) {
		// Personal inventory slots
		result = _GetItem(m_inv, slot_id);
	}
	else if ((slot_id>=0 && slot_id<=21) || (slot_id >= 400 && slot_id<=404) || (slot_id == 9999)) {
		// Equippable slots (on body)
		result = _GetItem(m_worn, slot_id);
	}

	// Inner bag slots
	else if (slot_id>=3031 && slot_id<=3110) {
		// Trade bag slots
		ItemInst* inst = _GetItem(m_trade, Inventory::CalcSlotId(slot_id));
		if (inst && inst->IsType(ItemClassContainer)) {
			result = inst->GetItem(Inventory::CalcBagIdx(slot_id));
		}
	}
	else if (slot_id>=2531 && slot_id<=2550) {
		// Shared Bank bag slots
		ItemInst* inst = _GetItem(m_shbank, Inventory::CalcSlotId(slot_id));
		if (inst && inst->IsType(ItemClassContainer)) {
			result = inst->GetItem(Inventory::CalcBagIdx(slot_id));
		}
	}
	else if (slot_id>=2031 && slot_id<=2270) {
		// Bank bag slots
		ItemInst* inst = _GetItem(m_bank, Inventory::CalcSlotId(slot_id));
		if (inst && inst->IsType(ItemClassContainer)) {
			result = inst->GetItem(Inventory::CalcBagIdx(slot_id));
		}
	}
	else if (slot_id>=331 && slot_id<=340) {
		// Cursor bag slots
		ItemInst* inst = m_cursor.peek_front();
		if (inst && inst->IsType(ItemClassContainer)) {
			result = inst->GetItem(Inventory::CalcBagIdx(slot_id));
		}
	}
	else if (slot_id>=251 && slot_id<=330) {
		// Personal inventory bag slots
		ItemInst* inst = _GetItem(m_inv, Inventory::CalcSlotId(slot_id));
		if (inst && inst->IsType(ItemClassContainer)) {
			result = inst->GetItem(Inventory::CalcBagIdx(slot_id));
		}
	}

	return result;
}

std::string ItemInst::GetCustomDataString() const {
	std::string ret_val;
	std::map<std::string, std::string>::const_iterator iter = m_custom_data.begin();
	while(iter != m_custom_data.end()) {
		if(ret_val.length() > 0) {
			ret_val += "^";
		}
		ret_val += iter->first;
		ret_val += "^";
		ret_val += iter->second;
		++iter;

		if(ret_val.length() > 0) {
			ret_val += "^";
		}
	}
	return ret_val;
}

void ItemInst::SetCustomData(std::string identifier, std::string value) {
	DeleteCustomData(identifier);
	m_custom_data[identifier] = value;
}

void ItemInst::SetCustomData(std::string identifier, int value) {
	DeleteCustomData(identifier);
	std::stringstream ss;
	ss << value;
	m_custom_data[identifier] = ss.str();
}

void ItemInst::SetCustomData(std::string identifier, float value) {
	DeleteCustomData(identifier);
	std::stringstream ss;
	ss << value;
	m_custom_data[identifier] = ss.str();
}

void ItemInst::SetCustomData(std::string identifier, bool value) {
	DeleteCustomData(identifier);
	std::stringstream ss;
	ss << value;
	m_custom_data[identifier] = ss.str();
}

void ItemInst::DeleteCustomData(std::string identifier) {
	std::map<std::string, std::string>::iterator iter = m_custom_data.find(identifier);
	if(iter != m_custom_data.end()) {
		m_custom_data.erase(iter);
	}
}

std::string ItemInst::GetCustomData(std::string identifier) {
	std::map<std::string, std::string>::const_iterator iter = m_custom_data.find(identifier);
	if(iter != m_custom_data.end()) {
		return iter->second;
	}

	return "";
}

void ItemInst::SetTimer(std::string name, uint32 time) {
	Timer t(time);
	t.Start(time, false);
	m_timers[name] = t;
}

void ItemInst::StopTimer(std::string name) {
	auto iter = m_timers.find(name);
	if(iter != m_timers.end()) {
		m_timers.erase(iter);
	}
}

void ItemInst::ClearTimers() {
	m_timers.clear();
}

const Item_Struct* ItemInst::GetItem() const {
	if(!m_scaledItem)
		return m_item;
	else
		return m_scaledItem;
}

const Item_Struct* ItemInst::GetUnscaledItem() const {
	return m_item;
}

void ItemInst::Initialize(SharedDatabase *db) {
	// if there's no actual item, don't do anything
	if(!m_item)
		return;

	// initialize scaling items
	if (m_item->CharmFileID != 0) {
		m_scaling = true;
		ScaleItem();
	}

	// initialize evolving items
	else if ((db) && m_item->LoreGroup >= 1000 && m_item->LoreGroup != -1) {
		// not complete yet
	}
}

void ItemInst::ScaleItem() {
	if(m_scaledItem) {
		memcpy(m_scaledItem, m_item, sizeof(Item_Struct));
	} else {
		m_scaledItem = new Item_Struct(*m_item);
	}

	float Mult = (float)(GetExp())/10000;	// scaling is determined by exp, with 10,000 being full stats

	m_scaledItem->AStr = (int8)((float)m_item->AStr*Mult);
	m_scaledItem->ASta = (int8)((float)m_item->ASta*Mult);
	m_scaledItem->AAgi = (int8)((float)m_item->AAgi*Mult);
	m_scaledItem->ADex = (int8)((float)m_item->ADex*Mult);
	m_scaledItem->AInt = (int8)((float)m_item->AInt*Mult);
	m_scaledItem->AWis = (int8)((float)m_item->AWis*Mult);
	m_scaledItem->ACha = (int8)((float)m_item->ACha*Mult);

	m_scaledItem->MR = (int8)((float)m_item->MR*Mult);
	m_scaledItem->PR = (int8)((float)m_item->PR*Mult);
	m_scaledItem->DR = (int8)((float)m_item->DR*Mult);
	m_scaledItem->CR = (int8)((float)m_item->CR*Mult);
	m_scaledItem->FR = (int8)((float)m_item->FR*Mult);

	m_scaledItem->HP = (int32)((float)m_item->HP*Mult);
	m_scaledItem->Mana = (int32)((float)m_item->Mana*Mult);
	m_scaledItem->AC = (int32)((float)m_item->AC*Mult);

	m_scaledItem->SkillModValue = (int32)((float)m_item->SkillModValue*Mult);
	m_scaledItem->BaneDmgAmt = (int8)((float)m_item->BaneDmgAmt*Mult);
	m_scaledItem->BardValue = (int32)((float)m_item->BardValue*Mult);
	m_scaledItem->ElemDmgAmt = (uint8)((float)m_item->ElemDmgAmt*Mult);
	m_scaledItem->Damage = (uint32)((float)m_item->Damage*Mult);

	m_scaledItem->CombatEffects = (int8)((float)m_item->CombatEffects*Mult);
	m_scaledItem->Shielding = (int8)((float)m_item->Shielding*Mult);
	m_scaledItem->StunResist = (int8)((float)m_item->StunResist*Mult);
	m_scaledItem->StrikeThrough = (int8)((float)m_item->StrikeThrough*Mult);
	m_scaledItem->ExtraDmgAmt = (uint32)((float)m_item->ExtraDmgAmt*Mult);
	m_scaledItem->SpellShield = (int8)((float)m_item->SpellShield*Mult);
	m_scaledItem->Avoidance = (int8)((float)m_item->Avoidance*Mult);
	m_scaledItem->Accuracy = (int8)((float)m_item->Accuracy*Mult);

	m_scaledItem->FactionAmt1 = (int32)((float)m_item->FactionAmt1*Mult);
	m_scaledItem->FactionAmt2 = (int32)((float)m_item->FactionAmt2*Mult);
	m_scaledItem->FactionAmt3 = (int32)((float)m_item->FactionAmt3*Mult);
	m_scaledItem->FactionAmt4 = (int32)((float)m_item->FactionAmt4*Mult);

	m_scaledItem->Endur = (uint32)((float)m_item->Endur*Mult);
	m_scaledItem->DotShielding = (uint32)((float)m_item->DotShielding*Mult);
	m_scaledItem->Attack = (uint32)((float)m_item->Attack*Mult);
	m_scaledItem->Regen = (uint32)((float)m_item->Regen*Mult);
	m_scaledItem->ManaRegen = (uint32)((float)m_item->ManaRegen*Mult);
	m_scaledItem->EnduranceRegen = (uint32)((float)m_item->EnduranceRegen*Mult);
	m_scaledItem->Haste = (uint32)((float)m_item->Haste*Mult);
	m_scaledItem->DamageShield = (uint32)((float)m_item->DamageShield*Mult);

	m_scaledItem->Purity = (uint32)((float)m_item->Purity*Mult);
	m_scaledItem->BackstabDmg = (uint32)((float)m_item->BackstabDmg*Mult);
	m_scaledItem->DSMitigation = (uint32)((float)m_item->DSMitigation*Mult);
	m_scaledItem->HeroicStr = (int32)((float)m_item->HeroicStr*Mult);
	m_scaledItem->HeroicInt = (int32)((float)m_item->HeroicInt*Mult);
	m_scaledItem->HeroicWis = (int32)((float)m_item->HeroicWis*Mult);
	m_scaledItem->HeroicAgi = (int32)((float)m_item->HeroicAgi*Mult);
	m_scaledItem->HeroicDex = (int32)((float)m_item->HeroicDex*Mult);
	m_scaledItem->HeroicSta = (int32)((float)m_item->HeroicSta*Mult);
	m_scaledItem->HeroicCha = (int32)((float)m_item->HeroicCha*Mult);
	m_scaledItem->HeroicMR = (int32)((float)m_item->HeroicMR*Mult);
	m_scaledItem->HeroicFR = (int32)((float)m_item->HeroicFR*Mult);
	m_scaledItem->HeroicCR = (int32)((float)m_item->HeroicCR*Mult);
	m_scaledItem->HeroicDR = (int32)((float)m_item->HeroicDR*Mult);
	m_scaledItem->HeroicPR = (int32)((float)m_item->HeroicPR*Mult);
	m_scaledItem->HeroicSVCorrup = (int32)((float)m_item->HeroicSVCorrup*Mult);
	m_scaledItem->HealAmt = (int32)((float)m_item->HealAmt*Mult);
	m_scaledItem->SpellDmg = (int32)((float)m_item->SpellDmg*Mult);
	m_scaledItem->Clairvoyance = (uint32)((float)m_item->Clairvoyance*Mult);

	m_scaledItem->CharmFileID = 0;	// this stops the client from trying to scale the item itself.
}

bool ItemInst::EvolveOnAllKills() const {
	return (m_evolveInfo && m_evolveInfo->AllKills);
}

int8 ItemInst::GetMaxEvolveLvl() const {
	if(m_evolveInfo)
		return m_evolveInfo->MaxLvl;
	else
		return 0;
}

uint32 ItemInst::GetKillsNeeded(uint8 currentlevel) {
	uint32 kills = -1;	// default to -1 (max uint32 value) because this value is usually divided by, so we don't want to ever return zero.
	if (m_evolveInfo)
		if (currentlevel != m_evolveInfo->MaxLvl)
			kills = m_evolveInfo->LvlKills[currentlevel-1];

	if (kills == 0)
		kills = -1;

	return kills;
}

// Retrieve item at specified position within bag
ItemInst* Inventory::GetItem(int16 slot_id, uint8 bagidx) const
{
	return GetItem(Inventory::CalcSlotId(slot_id, bagidx));
}

int16 Inventory::PushCursor(const ItemInst& inst)
{
	m_cursor.push(inst.Clone());
	return SLOT_CURSOR;
}

// Put an item snto specified slot
int16 Inventory::PutItem(int16 slot_id, const ItemInst& inst)
{
	// Clean up item already in slot (if exists)
	DeleteItem(slot_id);

	if (!inst) {
		// User is effectively deleting the item
		// in the slot, why hold a null ptr in map<>?
		return slot_id;
	}

	// Delegate to internal method
	return _PutItem(slot_id, inst.Clone());
}

// Swap items in inventory
bool Inventory::SwapItem(int16 slot_a, int16 slot_b)
{
	// Temp holding areas for a and b
	ItemInst* inst_a = GetItem(slot_a);
	ItemInst* inst_b = GetItem(slot_b);

	if(inst_a) { if(!inst_a->IsSlotAllowed(slot_b)) { return false; } }
	if(inst_b) { if(!inst_b->IsSlotAllowed(slot_a)) { return false; } }

	_PutItem(slot_a, inst_b); // Copy b->a
	_PutItem(slot_b, inst_a); // Copy a->b

	return true;
}

// Checks that user has at least 'quantity' number of items in a given inventory slot
// Returns first slot it was found in, or SLOT_INVALID if not found

//This function has a flaw in that it only returns the last stack that it looked at
//when quantity is greater than 1 and not all of quantity can be found in 1 stack.

int16 Inventory::HasItem(uint32 item_id, uint8 quantity, uint8 where)
{
	int16 slot_id = SLOT_INVALID;

	//Altered by Father Nitwit to support a specification of
	//where to search, with a default value to maintain compatibility

	// Check each inventory bucket
	if(where & invWhereWorn) {
		slot_id = _HasItem(m_worn, item_id, quantity);
		if (slot_id != SLOT_INVALID)
			return slot_id;
	}

	if(where & invWherePersonal) {
		slot_id = _HasItem(m_inv, item_id, quantity);
		if (slot_id != SLOT_INVALID)
			return slot_id;
	}

	if(where & invWhereBank) {
		slot_id = _HasItem(m_bank, item_id, quantity);
		if (slot_id != SLOT_INVALID)
			return slot_id;
	}

	if(where & invWhereSharedBank) {
		slot_id = _HasItem(m_shbank, item_id, quantity);
		if (slot_id != SLOT_INVALID)
			return slot_id;
	}

	if(where & invWhereTrading) {
		slot_id = _HasItem(m_trade, item_id, quantity);
		if (slot_id != SLOT_INVALID)
			return slot_id;
	}

	if(where & invWhereCursor) {
		// Check cursor queue
		slot_id = _HasItem(m_cursor, item_id, quantity);
		if (slot_id != SLOT_INVALID)
			return slot_id;
	}

	return slot_id;
}

//this function has the same quantity flaw mentioned above in HasItem()

int16 Inventory::HasItemByUse(uint8 use, uint8 quantity, uint8 where)
{
	int16 slot_id = SLOT_INVALID;

	// Check each inventory bucket
	if(where & invWhereWorn) {
		slot_id = _HasItemByUse(m_worn, use, quantity);
		if (slot_id != SLOT_INVALID)
			return slot_id;
	}

	if(where & invWherePersonal) {
		slot_id = _HasItemByUse(m_inv, use, quantity);
		if (slot_id != SLOT_INVALID)
			return slot_id;
	}

	if(where & invWhereBank) {
		slot_id = _HasItemByUse(m_bank, use, quantity);
		if (slot_id != SLOT_INVALID)
			return slot_id;
	}

	if(where & invWhereSharedBank) {
		slot_id = _HasItemByUse(m_shbank, use, quantity);
		if (slot_id != SLOT_INVALID)
			return slot_id;
	}

	if(where & invWhereTrading) {
		slot_id = _HasItemByUse(m_trade, use, quantity);
		if (slot_id != SLOT_INVALID)
			return slot_id;
	}

	if(where & invWhereCursor) {
		// Check cursor queue
		slot_id = _HasItemByUse(m_cursor, use, quantity);
		if (slot_id != SLOT_INVALID)
			return slot_id;
	}

	return slot_id;
}

int16 Inventory::HasItemByLoreGroup(uint32 loregroup, uint8 where)
{
	int16 slot_id = SLOT_INVALID;

	// Check each inventory bucket
	if(where & invWhereWorn) {
		slot_id = _HasItemByLoreGroup(m_worn, loregroup);
		if (slot_id != SLOT_INVALID)
			return slot_id;
	}

	if(where & invWherePersonal) {
		slot_id = _HasItemByLoreGroup(m_inv, loregroup);
		if (slot_id != SLOT_INVALID)
			return slot_id;
	}

	if(where & invWhereBank) {
		slot_id = _HasItemByLoreGroup(m_bank, loregroup);
		if (slot_id != SLOT_INVALID)
			return slot_id;
	}

	if(where & invWhereSharedBank) {
		slot_id = _HasItemByLoreGroup(m_shbank, loregroup);
		if (slot_id != SLOT_INVALID)
			return slot_id;
	}

	if(where & invWhereTrading) {
		slot_id = _HasItemByLoreGroup(m_trade, loregroup);
		if (slot_id != SLOT_INVALID)
			return slot_id;
	}

	if(where & invWhereCursor) {
		// Check cursor queue
		slot_id = _HasItemByLoreGroup(m_cursor, loregroup);
		if (slot_id != SLOT_INVALID)
			return slot_id;
	}

	return slot_id;
}

bool Inventory::HasSpaceForItem(const Item_Struct *ItemToTry, int16 Quantity) {

	if(ItemToTry->Stackable) {

		for(int16 i = 22; i <= 29; i++) {

			ItemInst* InvItem = GetItem(i);

			if(InvItem && (InvItem->GetItem()->ID == ItemToTry->ID) && (InvItem->GetCharges() < InvItem->GetItem()->StackSize)) {

				int ChargeSlotsLeft = InvItem->GetItem()->StackSize - InvItem->GetCharges();

				if(Quantity <= ChargeSlotsLeft)
					return true;

				Quantity -= ChargeSlotsLeft;

			}
			if (InvItem && InvItem->IsType(ItemClassContainer)) {

				int16 BaseSlotID = Inventory::CalcSlotId(i, 0);
				uint8 BagSize=InvItem->GetItem()->BagSlots;
				for (uint8 BagSlot = 0; BagSlot < BagSize; BagSlot++) {

					InvItem = GetItem(BaseSlotID + BagSlot);

					if(InvItem && (InvItem->GetItem()->ID == ItemToTry->ID) &&
						(InvItem->GetCharges() < InvItem->GetItem()->StackSize)) {

						int ChargeSlotsLeft = InvItem->GetItem()->StackSize - InvItem->GetCharges();

						if(Quantity <= ChargeSlotsLeft)
							return true;

						Quantity -= ChargeSlotsLeft;
					}
				}
			}
		}
	}

	for (int16 i = 22; i <= 29; i++) {

		ItemInst* InvItem = GetItem(i);

		if (!InvItem) {

			if(!ItemToTry->Stackable) {

				if(Quantity == 1)
					return true;
				else
					Quantity--;
			}
			else {
				if(Quantity <= ItemToTry->StackSize)
					return true;
				else
					Quantity -= ItemToTry->StackSize;
			}

		}
		else if(InvItem->IsType(ItemClassContainer) && CanItemFitInContainer(ItemToTry, InvItem->GetItem())) {

			int16 BaseSlotID = Inventory::CalcSlotId(i, 0);

			uint8 BagSize=InvItem->GetItem()->BagSlots;

			for (uint8 BagSlot=0; BagSlot<BagSize; BagSlot++) {

				InvItem = GetItem(BaseSlotID + BagSlot);

				if(!InvItem) {
					if(!ItemToTry->Stackable) {

						if(Quantity == 1)
							return true;
						else
							Quantity--;
					}
					else {
						if(Quantity <= ItemToTry->StackSize)
							return true;
						else
							Quantity -= ItemToTry->StackSize;
					}
				}
			}
		}
	}

	return false;

}

// Remove item from inventory (with memory delete)
bool Inventory::DeleteItem(int16 slot_id, uint8 quantity)
{
	// Pop item out of inventory map (or queue)
	ItemInst* item_to_delete = PopItem(slot_id);

	// Determine if object should be fully deleted, or
	// just a quantity of charges of the item can be deleted
	if (item_to_delete && (quantity > 0)) {

		item_to_delete->SetCharges(item_to_delete->GetCharges() - quantity);

		// If there are no charges left on the item,
		if(item_to_delete->GetCharges() <= 0) {
			// If the item is stackable (e.g arrows), or
			// the item is not stackable, and is not a charged item, or is expendable, delete it
			if(item_to_delete->IsStackable() ||
				(!item_to_delete->IsStackable() &&
				((item_to_delete->GetItem()->MaxCharges == 0) || item_to_delete->IsExpendable()))) {
				// Item can now be destroyed
				Inventory::MarkDirty(item_to_delete);
				return true;
			}
		}

		// Charges still exist, or it is a charged item that is not expendable. Put back into inventory
		_PutItem(slot_id, item_to_delete);
		return false;
	}

	Inventory::MarkDirty(item_to_delete);
	return true;

}

// Checks All items in a bag for No Drop
bool Inventory::CheckNoDrop(int16 slot_id) {
	ItemInst* inst = GetItem(slot_id);
	if (!inst) return false;
	if (!inst->GetItem()->NoDrop) return true;
	if (inst->GetItem()->ItemClass == 1) {
		for (uint8 i = 0; i < 10; i++) {
			ItemInst* bagitem = GetItem(Inventory::CalcSlotId(slot_id, i));
			if (bagitem && !bagitem->GetItem()->NoDrop) return true;
		}
	}
	return false;
}

// Remove item from bucket without memory delete
// Returns item pointer if full delete was successful
ItemInst* Inventory::PopItem(int16 slot_id)
{
	ItemInst* p = nullptr;

	if (slot_id==SLOT_CURSOR) { // Cursor
		p = m_cursor.pop();
	}
	else if ((slot_id>=0 && slot_id<=21) || (slot_id >= 400 && slot_id<=404) || (slot_id == 9999)) { // Worn slots
		p = m_worn[slot_id];
		m_worn.erase(slot_id);
	}
	else if ((slot_id>=22 && slot_id<=29)) {
		p = m_inv[slot_id];
		m_inv.erase(slot_id);
	}
	else if (slot_id>=2000 && slot_id<=2023) { // Bank slots
		p = m_bank[slot_id];
		m_bank.erase(slot_id);
	}
	else if (slot_id>=2500 && slot_id<=2501) { // Shared bank slots
		p = m_shbank[slot_id];
		m_shbank.erase(slot_id);
	}
	else if (slot_id>=3000 && slot_id<=3007) { // Trade window slots
		p = m_trade[slot_id];
		m_trade.erase(slot_id);
	}
	else {
		// Is slot inside bag?
		ItemInst* baginst = GetItem(Inventory::CalcSlotId(slot_id));
		if (baginst != nullptr && baginst->IsType(ItemClassContainer)) {
			p = baginst->PopItem(Inventory::CalcBagIdx(slot_id));
		}
	}

	// Return pointer that needs to be deleted (or otherwise managed)
	return p;
}

// Locate an available inventory slot
// Returns slot_id when there's one available, else SLOT_INVALID
int16 Inventory::FindFreeSlot(bool for_bag, bool try_cursor, uint8 min_size, bool is_arrow)
{
	// Check basic inventory
	for (int16 i=22; i<=29; i++) {
		if (!GetItem(i))
			// Found available slot in personal inventory
			return i;
	}

	if (!for_bag) {
		for (int16 i=22; i<=29; i++) {
			const ItemInst* inst = GetItem(i);
			if (inst && inst->IsType(ItemClassContainer)
				&& inst->GetItem()->BagSize >= min_size)
			{
				if(inst->GetItem()->BagType == BagTypeQuiver && inst->GetItem()->ItemType != ItemTypeArrow)
				{
					continue;
				}

				int16 base_slot_id = Inventory::CalcSlotId(i, 0);

				uint8 slots=inst->GetItem()->BagSlots;
				uint8 j;
				for (j=0; j<slots; j++) {
					if (!GetItem(base_slot_id + j))
						// Found available slot within bag
						return (base_slot_id + j);
				}
			}
		}
	}

	if (try_cursor)
		// Always room on cursor (it's a queue)
		// (we may wish to cap this in the future)
		return SLOT_CURSOR;

	// No available slots
	return SLOT_INVALID;
}

void Inventory::dumpBagContents(ItemInst *inst, iter_inst *it) {
	iter_contents itb;

	if (!inst || !inst->IsType(ItemClassContainer))
		return;

	// Go through bag, if bag
	for (itb=inst->_begin(); itb!=inst->_end(); ++itb) {
		ItemInst* baginst = itb->second;
		if(!baginst || !baginst->GetItem())
			continue;

		std::string subSlot;
		StringFormat(subSlot,"	Slot %d: %s (%d)", Inventory::CalcSlotId((*it)->first, itb->first),
			baginst->GetItem()->Name, (baginst->GetCharges()<=0) ? 1 : baginst->GetCharges());
		std::cout << subSlot << std::endl;
	}

}

int Inventory::GetSlotByItemInst(ItemInst *inst) {
	if(!inst)
		return -1;

	int i = GetSlotByItemInstCollection(m_worn, inst);
	if(i != -1) {
		return i;
	}

	i = GetSlotByItemInstCollection(m_inv, inst);
	if(i != -1) {
		return i;
	}

	i = GetSlotByItemInstCollection(m_bank, inst);
	if(i != -1) {
		return i;
	}

	i = GetSlotByItemInstCollection(m_shbank, inst);
	if(i != -1) {
		return i;
	}

	i = GetSlotByItemInstCollection(m_trade, inst);
	if(i != -1) {
		return i;
	}

	if(m_cursor.peek_front() == inst) {
		return SLOT_CURSOR;
	}

	return -1;
}

int Inventory::GetSlotByItemInstCollection(const std::map<int16, ItemInst*> &collection, ItemInst *inst) {
	for(auto iter = collection.begin(); iter != collection.end(); ++iter) {
		ItemInst *t_inst = iter->second;
		if(t_inst == inst) {
			return iter->first;
		}

		if(t_inst && !t_inst->IsType(ItemClassContainer)) {
			for(auto b_iter = t_inst->_begin(); b_iter != t_inst->_end(); ++b_iter) {
				if(b_iter->second == inst) {
					return Inventory::CalcSlotId(iter->first, b_iter->first);
				}
			}
		}
	}

	return -1;
}

void Inventory::dumpItemCollection(const std::map<int16, ItemInst*> &collection) {
	iter_inst it;
	iter_contents itb;
	ItemInst* inst = nullptr;
	
	for (it=collection.begin(); it!=collection.end(); ++it) {
		inst = it->second;
		if(!inst || !inst->GetItem())
			continue;
		
		std::string slot;
		StringFormat(slot, "Slot %d: %s (%d)",it->first, it->second->GetItem()->Name, (inst->GetCharges()<=0) ? 1 : inst->GetCharges());
		std::cout << slot << std::endl;		

		dumpBagContents(inst, &it);
	}
}

void Inventory::dumpWornItems() {
	std::cout << "Worn items:" << std::endl;
	dumpItemCollection(m_worn);
}

void Inventory::dumpInventory() {
	std::cout << "Inventory items:" << std::endl;
	dumpItemCollection(m_inv);
}

void Inventory::dumpBankItems() {
	
	std::cout << "Bank items:" << std::endl;
	dumpItemCollection(m_bank);
}

void Inventory::dumpSharedBankItems() {
	
	std::cout << "Shared Bank items:" << std::endl;
	dumpItemCollection(m_shbank);
}

void Inventory::dumpEntireInventory() {

	dumpWornItems();
	dumpInventory();
	dumpBankItems();
	dumpSharedBankItems();
	
	std::cout << std::endl;
}

// Internal Method: Retrieves item within an inventory bucket
ItemInst* Inventory::_GetItem(const std::map<int16, ItemInst*>& bucket, int16 slot_id) const
{
	iter_inst it = bucket.find(slot_id);
	if (it != bucket.end()) {
		return it->second;
	}

	// Not found!
	return nullptr;
}

// Internal Method: "put" item into bucket, without regard for what is currently in bucket
// Assumes item has already been allocated
int16 Inventory::_PutItem(int16 slot_id, ItemInst* inst)
{
	// If putting a nullptr into slot, we need to remove slot without memory delete
	if (inst == nullptr) {
		//Why do we not delete the poped item here????
		PopItem(slot_id);
		return slot_id;
	}

	int16 result = SLOT_INVALID;

	if (slot_id==SLOT_CURSOR) { // Cursor
		// Replace current item on cursor, if exists
		m_cursor.pop(); // no memory delete, clients of this function know what they are doing
		m_cursor.push_front(inst);
		result = slot_id;
	}
	else if ((slot_id>=0 && slot_id<=21) || (slot_id >= 400 && slot_id<=404) || (slot_id == 9999)) { // Worn slots
		m_worn[slot_id] = inst;
		result = slot_id;
	}
	else if ((slot_id>=22 && slot_id<=29)) {
		m_inv[slot_id] = inst;
		result = slot_id;
	}
	else if (slot_id>=2000 && slot_id<=2023) { // Bank slots
		m_bank[slot_id] = inst;
		result = slot_id;
	}
	else if (slot_id>=2500 && slot_id<=2501) { // Shared bank slots
		m_shbank[slot_id] = inst;
		result = slot_id;
	}
	else if (slot_id>=3000 && slot_id<=3007) { // Trade window slots
		m_trade[slot_id] = inst;
		result = slot_id;
	}
	else {
		// Slot must be within a bag
		ItemInst* baginst = GetItem(Inventory::CalcSlotId(slot_id)); // Get parent bag
		if (baginst && baginst->IsType(ItemClassContainer)) {
			baginst->_PutItem(Inventory::CalcBagIdx(slot_id), inst);
			result = slot_id;
		}
	}

	if (result == SLOT_INVALID) {
		LogFile->write(EQEMuLog::Error, "Inventory::_PutItem: Invalid slot_id specified (%i)", slot_id);
		Inventory::MarkDirty(inst); // Slot not found, clean up
	}

	return result;
}

// Internal Method: Checks an inventory bucket for a particular item
int16 Inventory::_HasItem(std::map<int16, ItemInst*>& bucket, uint32 item_id, uint8 quantity)
{
	iter_inst it;
	iter_contents itb;
	ItemInst* inst = nullptr;
	uint8 quantity_found = 0;

	// Check item: After failed checks, check bag contents (if bag)
	for (it=bucket.begin(); it!=bucket.end(); ++it) {
		inst = it->second;
		if (inst) {
			if (inst->GetID() == item_id) {
				quantity_found += (inst->GetCharges()<=0) ? 1 : inst->GetCharges();
				if (quantity_found >= quantity)
					return it->first;
			}

			for(int i = 0; i < MAX_AUGMENT_SLOTS; i++) {
				if (inst->GetAugmentItemID(i) == item_id && quantity <= 1)
					return SLOT_AUGMENT; // Only one augment per slot.
			}
		}
		// Go through bag, if bag
		if (inst && inst->IsType(ItemClassContainer)) {

			for (itb=inst->_begin(); itb!=inst->_end(); ++itb) {
				ItemInst* baginst = itb->second;
				if (baginst->GetID() == item_id) {
					quantity_found += (baginst->GetCharges()<=0) ? 1 : baginst->GetCharges();
					if (quantity_found >= quantity)
						return Inventory::CalcSlotId(it->first, itb->first);
				}
				for(int i = 0; i < MAX_AUGMENT_SLOTS; i++) {
					if (baginst->GetAugmentItemID(i) == item_id && quantity <= 1)
						return SLOT_AUGMENT; // Only one augment per slot.
				}
			}
		}
	}

	// Not found
	return SLOT_INVALID;
}

// Internal Method: Checks an inventory queue type bucket for a particular item
int16 Inventory::_HasItem(ItemInstQueue& iqueue, uint32 item_id, uint8 quantity)
{
	iter_queue it;
	iter_contents itb;
	uint8 quantity_found = 0;

	// Read-only iteration of queue
	for (it=iqueue.begin(); it!=iqueue.end(); ++it) {
		ItemInst* inst = *it;
		if (inst)
		{
			if (inst->GetID() == item_id) {
				quantity_found += (inst->GetCharges()<=0) ? 1 : inst->GetCharges();
				if (quantity_found >= quantity)
					return SLOT_CURSOR;
			}
			for(int i = 0; i < MAX_AUGMENT_SLOTS; i++) {
				if (inst->GetAugmentItemID(i) == item_id && quantity <= 1)
					return SLOT_AUGMENT; // Only one augment per slot.
			}
		}
		// Go through bag, if bag
		if (inst && inst->IsType(ItemClassContainer)) {

			for (itb=inst->_begin(); itb!=inst->_end(); ++itb) {
				ItemInst* baginst = itb->second;
				if (baginst->GetID() == item_id) {
					quantity_found += (baginst->GetCharges()<=0) ? 1 : baginst->GetCharges();
					if (quantity_found >= quantity)
						return Inventory::CalcSlotId(SLOT_CURSOR, itb->first);
				}
				for(int i = 0; i < MAX_AUGMENT_SLOTS; i++) {
					if (baginst->GetAugmentItemID(i) == item_id && quantity <= 1)
						return SLOT_AUGMENT; // Only one augment per slot.
				}

			}
		}
	}

	// Not found
	return SLOT_INVALID;
}

// Internal Method: Checks an inventory bucket for a particular item
int16 Inventory::_HasItemByUse(std::map<int16, ItemInst*>& bucket, uint8 use, uint8 quantity)
{
	iter_inst it;
	iter_contents itb;
	ItemInst* inst = nullptr;
	uint8 quantity_found = 0;

	// Check item: After failed checks, check bag contents (if bag)
	for (it=bucket.begin(); it!=bucket.end(); ++it) {
		inst = it->second;
		if (inst && inst->IsType(ItemClassCommon) && inst->GetItem()->ItemType == use) {
			quantity_found += (inst->GetCharges()<=0) ? 1 : inst->GetCharges();
			if (quantity_found >= quantity)
				return it->first;
		}

		// Go through bag, if bag
		if (inst && inst->IsType(ItemClassContainer)) {

			for (itb=inst->_begin(); itb!=inst->_end(); itb++) {
				ItemInst* baginst = itb->second;
				if (baginst && baginst->IsType(ItemClassCommon) && baginst->GetItem()->ItemType == use) {
					quantity_found += (baginst->GetCharges()<=0) ? 1 : baginst->GetCharges();
					if (quantity_found >= quantity)
						return Inventory::CalcSlotId(it->first, itb->first);
				}
			}
		}
	}

	// Not found
	return SLOT_INVALID;
}

// Internal Method: Checks an inventory queue type bucket for a particular item
int16 Inventory::_HasItemByUse(ItemInstQueue& iqueue, uint8 use, uint8 quantity)
{
	iter_queue it;
	iter_contents itb;
	uint8 quantity_found = 0;

	// Read-only iteration of queue
	for (it=iqueue.begin(); it!=iqueue.end(); ++it) {
		ItemInst* inst = *it;
		if (inst && inst->IsType(ItemClassCommon) && inst->GetItem()->ItemType == use) {
			quantity_found += (inst->GetCharges()<=0) ? 1 : inst->GetCharges();
			if (quantity_found >= quantity)
				return SLOT_CURSOR;
		}

		// Go through bag, if bag
		if (inst && inst->IsType(ItemClassContainer)) {

			for (itb=inst->_begin(); itb!=inst->_end(); ++itb) {
				ItemInst* baginst = itb->second;
				if (baginst && baginst->IsType(ItemClassCommon) && baginst->GetItem()->ItemType == use) {
					quantity_found += (baginst->GetCharges()<=0) ? 1 : baginst->GetCharges();
					if (quantity_found >= quantity)
						return Inventory::CalcSlotId(SLOT_CURSOR, itb->first);
				}
			}
		}
	}

	// Not found
	return SLOT_INVALID;
}

int16 Inventory::_HasItemByLoreGroup(std::map<int16, ItemInst*>& bucket, uint32 loregroup)
{
	iter_inst it;
	iter_contents itb;
	ItemInst* inst = nullptr;

	// Check item: After failed checks, check bag contents (if bag)
	for (it=bucket.begin(); it!=bucket.end(); ++it) {
		inst = it->second;
		if (inst) {
			if (inst->GetItem()->LoreGroup == loregroup)
				return it->first;

			ItemInst* Aug;
			for(int i = 0; i < MAX_AUGMENT_SLOTS; i++) {
				Aug = inst->GetAugment(i);
				if (Aug && Aug->GetItem()->LoreGroup == loregroup)
					return SLOT_AUGMENT; // Only one augment per slot.
			}
		}
		// Go through bag, if bag
		if (inst && inst->IsType(ItemClassContainer)) {

			for (itb=inst->_begin(); itb!=inst->_end(); ++itb) {
				ItemInst* baginst = itb->second;
				if (baginst && baginst->IsType(ItemClassCommon)&& baginst->GetItem()->LoreGroup == loregroup)
					return Inventory::CalcSlotId(it->first, itb->first);

				ItemInst* Aug2;
				for(int i = 0; i < MAX_AUGMENT_SLOTS; i++) {
					Aug2 = baginst->GetAugment(i);
					if (Aug2 && Aug2->GetItem()->LoreGroup == loregroup)
						return SLOT_AUGMENT; // Only one augment per slot.
				}
			}
		}
	}

	// Not found
	return SLOT_INVALID;
}

// Internal Method: Checks an inventory queue type bucket for a particular item
int16 Inventory::_HasItemByLoreGroup(ItemInstQueue& iqueue, uint32 loregroup)
{
	iter_queue it;
	iter_contents itb;

	// Read-only iteration of queue
	for (it=iqueue.begin(); it!=iqueue.end(); ++it) {
		ItemInst* inst = *it;
		if (inst)
		{
			if (inst->GetItem()->LoreGroup == loregroup)
				return SLOT_CURSOR;

			ItemInst* Aug;
			for(int i = 0; i < MAX_AUGMENT_SLOTS; i++) {
				Aug = inst->GetAugment(i);
				if (Aug && Aug->GetItem()->LoreGroup == loregroup)
					return SLOT_AUGMENT; // Only one augment per slot.
			}
		}
		// Go through bag, if bag
		if (inst && inst->IsType(ItemClassContainer)) {

			for (itb=inst->_begin(); itb!=inst->_end(); ++itb) {
				ItemInst* baginst = itb->second;
				if (baginst && baginst->IsType(ItemClassCommon)&& baginst->GetItem()->LoreGroup == loregroup)
					return Inventory::CalcSlotId(SLOT_CURSOR, itb->first);


				ItemInst* Aug2;
				for(int i = 0; i < MAX_AUGMENT_SLOTS; i++) {
					Aug2 = baginst->GetAugment(i);
					if (Aug2 && Aug2->GetItem()->LoreGroup == loregroup)
						return SLOT_AUGMENT; // Only one augment per slot.
				}

			}
		}
	}

	// Not found
	return SLOT_INVALID;
}

bool ItemInst::IsSlotAllowed(int16 slot_id) const {
	// 'SupportsContainers' and 'slot_id > 21' previously saw the reassigned PowerSource slot (9999 to 22) as valid -U
	if(!m_item) { return false; }
	else if(Inventory::SupportsContainers(slot_id)) { return true; }
	else if(m_item->Slots & (1 << slot_id)) { return true; }
	else if(slot_id == 9999 && (m_item->Slots & (1 << 22))) { return true; }
	else if(slot_id != 9999 && slot_id > 21) { return true; }
	else { return false; }
}

uint8 ItemInst::FirstOpenSlot() const
{
	uint8 slots=m_item->BagSlots,i;
	for(i=0;i<slots;i++) {
		if (!GetItem(i))
			break;
	}

	return (i<slots) ? i : 0xff;
}

uint8 ItemInst::GetTotalItemCount() const
{
	uint8 item_count = 1;

	if(m_item->ItemClass != ItemClassContainer) { return item_count; }

	for(int idx = 0; idx < m_item->BagSlots; idx++) { if(GetItem(idx)) { item_count++; } }

	return item_count;
}

bool ItemInst::IsNoneEmptyContainer()
{
	if(m_item->ItemClass != ItemClassContainer)
		return false;

	for(int i = 0; i < m_item->BagSlots; ++i)
		if(GetItem(i))
			return true;

	return false;
}

bool ItemInst::IsAugmented()
{
	for(int i = 0; i < MAX_AUGMENT_SLOTS; ++i)
		if (GetAugmentItemID(i))
			return true;

	return false;
}

// Calculate slot_id for an item within a bag
int16 Inventory::CalcSlotId(int16 bagslot_id, uint8 bagidx)
{
	if (!Inventory::SupportsContainers(bagslot_id)) {
		return SLOT_INVALID;
	}

	int16 slot_id = SLOT_INVALID;

	if (bagslot_id==SLOT_CURSOR || bagslot_id==8000) // Cursor
		slot_id = IDX_CURSOR_BAG + bagidx;
	else if (bagslot_id>=22 && bagslot_id<=29) // Inventory slots
		slot_id = IDX_INV_BAG + (bagslot_id-22)*MAX_ITEMS_PER_BAG + bagidx;
	else if (bagslot_id>=2000 && bagslot_id<=2023) // Bank slots
		slot_id = IDX_BANK_BAG + (bagslot_id-2000)*MAX_ITEMS_PER_BAG + bagidx;
	else if (bagslot_id>=2500 && bagslot_id<=2501) // Shared bank slots
		slot_id = IDX_SHBANK_BAG + (bagslot_id-2500)*MAX_ITEMS_PER_BAG + bagidx;
	else if (bagslot_id>=3000 && bagslot_id<=3007) // Trade window slots
		slot_id = IDX_TRADE_BAG + (bagslot_id-3000)*MAX_ITEMS_PER_BAG + bagidx;

	return slot_id;
}

// Opposite of above: Get parent bag slot_id from a slot inside of bag
int16 Inventory::CalcSlotId(int16 slot_id)
{
	int16 parent_slot_id = SLOT_INVALID;

	if (slot_id>=251 && slot_id<=330)
		parent_slot_id = IDX_INV + (slot_id-251) / MAX_ITEMS_PER_BAG;
	else if (slot_id>=331 && slot_id<=340)
		parent_slot_id = SLOT_CURSOR;
	else if (slot_id>=2000 && slot_id<=2023)
		parent_slot_id = IDX_BANK + (slot_id-2000) / MAX_ITEMS_PER_BAG;
	else if (slot_id>=2031 && slot_id<=2270)
		parent_slot_id = IDX_BANK + (slot_id-2031) / MAX_ITEMS_PER_BAG;
	else if (slot_id>=2531 && slot_id<=2550)
		parent_slot_id = IDX_SHBANK + (slot_id-2531) / MAX_ITEMS_PER_BAG;
	else if (slot_id>=3100 && slot_id<=3179)
		parent_slot_id = IDX_TRADE + (slot_id-3100) / MAX_ITEMS_PER_BAG;

	return parent_slot_id;
}

uint8 Inventory::CalcBagIdx(int16 slot_id)
{
	uint8 index = 0;

	if (slot_id>=251 && slot_id<=330)
		index = (slot_id-251) % MAX_ITEMS_PER_BAG;
	else if (slot_id>=331 && slot_id<=340)
		index = (slot_id-331) % MAX_ITEMS_PER_BAG;
	else if (slot_id>=2000 && slot_id<=2023)
		index = (slot_id-2000) % MAX_ITEMS_PER_BAG;
	else if (slot_id>=2031 && slot_id<=2270)
		index = (slot_id-2031) % MAX_ITEMS_PER_BAG;
	else if (slot_id>=2531 && slot_id<=2550)
		index = (slot_id-2531) % MAX_ITEMS_PER_BAG;
	else if (slot_id>=3100 && slot_id<=3179)
		index = (slot_id-3100) % MAX_ITEMS_PER_BAG;
	else if (slot_id>=4000 && slot_id<=4009)
		index = (slot_id-4000) % MAX_ITEMS_PER_BAG;

	return index;
}

int16 Inventory::CalcSlotFromMaterial(uint8 material)
{
	switch(material)
	{
		case MaterialHead:
			return SLOT_HEAD;
		case MaterialChest:
			return SLOT_CHEST;
		case MaterialArms:
			return SLOT_ARMS;
		case MaterialWrist:
			return SLOT_BRACER01;	// there's 2 bracers, only one bracer material
		case MaterialHands:
			return SLOT_HANDS;
		case MaterialLegs:
			return SLOT_LEGS;
		case MaterialFeet:
			return SLOT_FEET;
		case MaterialPrimary:
			return SLOT_PRIMARY;
		case MaterialSecondary:
			return SLOT_SECONDARY;
		default:
			return SLOT_INVALID;
	}
}

uint8 Inventory::CalcMaterialFromSlot(int16 equipslot)
{
	switch(equipslot)
	{
		case SLOT_HEAD:
			return MaterialHead;
		case SLOT_CHEST:
			return MaterialChest;
		case SLOT_ARMS:
			return MaterialArms;
		case SLOT_BRACER01:
		case SLOT_BRACER02:
			return MaterialWrist;
		case SLOT_HANDS:
			return MaterialHands;
		case SLOT_LEGS:
			return MaterialLegs;
		case SLOT_FEET:
			return MaterialFeet;
		case SLOT_PRIMARY:
			return MaterialPrimary;
		case SLOT_SECONDARY:
			return MaterialSecondary;
		default:
			return _MaterialInvalid;
	}
}


// Test whether a given slot can support a container item
bool Inventory::SupportsContainers(int16 slot_id)
{
	if ((slot_id>=22 && slot_id<=30) ||		// Personal inventory slots
		(slot_id>=2000 && slot_id<=2023) ||	// Bank slots
		(slot_id>=2500 && slot_id<=2501) ||	// Shared bank slots
		(slot_id==SLOT_CURSOR) ||			// Cursor
		(slot_id>=3000 && slot_id<=3007))	// Trade window
		return true;
	return false;
}


bool Inventory::CanItemFitInContainer(const Item_Struct *ItemToTry, const Item_Struct *Container) {

	if(!ItemToTry || !Container) return false;

	if(ItemToTry->Size > Container->BagSize) return false;

	if((Container->BagType == BagTypeQuiver) && (ItemToTry->ItemType != ItemTypeArrow)) return false;

	if((Container->BagType == BagTypeBandolier) && (ItemToTry->ItemType != ItemTypeSmallThrowing)) return false;

	return true;
}

EvolveInfo::EvolveInfo() {
	// nothing here yet
}

EvolveInfo::EvolveInfo(uint32 first, uint8 max, bool allkills, uint32 L2, uint32 L3, uint32 L4, uint32 L5, uint32 L6, uint32 L7, uint32 L8, uint32 L9, uint32 L10) {
	FirstItem = first;
	MaxLvl = max;
	AllKills = allkills;
	LvlKills[0] = L2;
	LvlKills[1] = L3;
	LvlKills[2] = L4;
	LvlKills[3] = L5;
	LvlKills[4] = L6;
	LvlKills[5] = L7;
	LvlKills[6] = L8;
	LvlKills[7] = L9;
	LvlKills[8] = L10;
}

EvolveInfo::~EvolveInfo() {
}

bool Item_Struct::IsEquipable(uint16 Race, uint16 Class_) const
{
	bool IsRace = false;
	bool IsClass = false;

	uint32 Classes_ = Classes;

	uint32 Races_ = Races;

	uint32 Race_ = GetArrayRace(Race);

	for (int CurrentClass = 1; CurrentClass <= PLAYER_CLASS_COUNT; ++CurrentClass)
	{
		if (Classes_ % 2 == 1)
		{
			if (CurrentClass == Class_)
			{
					IsClass = true;
				break;
			}
		}
		Classes_ >>= 1;
	}

	Race_ = (Race_ == 18 ? 16 : Race_);

	for (unsigned int CurrentRace = 1; CurrentRace <= PLAYER_RACE_COUNT; ++CurrentRace)
	{
		if (Races_ % 2 == 1)
		{
				if (CurrentRace == Race_)
			{
					IsRace = true;
				break;
			}
		}
		Races_ >>= 1;
	}
	return (IsRace && IsClass);
}

void Inventory::CleanDirty() {
	auto iter = dirty_inst.begin();
	while(iter != dirty_inst.end()) {
		delete (*iter);
		++iter;
	}
	dirty_inst.clear();
}

void Inventory::MarkDirty(ItemInst *inst) {
	if(inst) {
		dirty_inst.push_back(inst);
	}
}
