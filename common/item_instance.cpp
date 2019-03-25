/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

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

#include "inventory_profile.h"
//#include "classes.h"
//#include "global_define.h"
//#include "item_instance.h"
//#include "races.h"
#include "rulesys.h"
#include "shareddb.h"
#include "string_util.h"

//#include "../common/light_source.h"

#include <limits.h>

//#include <iostream>

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

//
// class EQEmu::ItemInstance
//
EQEmu::ItemInstance::ItemInstance(const ItemData* item, int16 charges) {
	m_use_type = ItemInstNormal;
	if(item) {
		m_item = new ItemData(*item);
	} else {
		m_item = nullptr;
	}
	m_charges = charges;
	m_price = 0;
	m_attuned = false;
	m_merchantslot = 0;
	if (m_item && m_item->IsClassCommon())
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
	m_ornamenticon = 0;
	m_ornamentidfile = 0;
	m_ornament_hero_model = 0;
	m_recast_timestamp = 0;
	m_new_id_file = 0;
}

EQEmu::ItemInstance::ItemInstance(SharedDatabase *db, uint32 item_id, int16 charges) {
	m_use_type = ItemInstNormal;
	m_item = db->GetItem(item_id);
	if(m_item) {
		m_item = new ItemData(*m_item);
	}
	else {
		m_item = nullptr;
	}

	m_charges = charges;
	m_price = 0;
	m_merchantslot = 0;
	m_attuned=false;
	if (m_item && m_item->IsClassCommon())
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
	m_ornamenticon = 0;
	m_ornamentidfile = 0;
	m_ornament_hero_model = 0;
	m_recast_timestamp = 0;
	m_new_id_file = 0;
}

EQEmu::ItemInstance::ItemInstance(ItemInstTypes use_type) {
	m_use_type = use_type;
	m_item = nullptr;
	m_charges = 0;
	m_price = 0;
	m_attuned = false;
	m_merchantslot = 0;
	m_color = 0;

	m_exp = 0;
	m_evolveLvl = 0;
	m_activated = false;
	m_scaledItem = nullptr;
	m_evolveInfo = nullptr;
	m_scaling = false;
	m_ornamenticon = 0;
	m_ornamentidfile = 0;
	m_ornament_hero_model = 0;
	m_recast_timestamp = 0;
	m_new_id_file = 0;
}

// Make a copy of an EQEmu::ItemInstance object
EQEmu::ItemInstance::ItemInstance(const ItemInstance& copy)
{
	m_use_type=copy.m_use_type;
	if(copy.m_item)
		m_item = new ItemData(*copy.m_item);
	else
		m_item = nullptr;

	m_charges=copy.m_charges;
	m_price=copy.m_price;
	m_color=copy.m_color;
	m_merchantslot=copy.m_merchantslot;
	m_currentslot=copy.m_currentslot;
	m_attuned=copy.m_attuned;
	m_merchantcount=copy.m_merchantcount;
	// Copy container contents
	for (auto it = copy.m_contents.begin(); it != copy.m_contents.end(); ++it) {
		ItemInstance* inst_old = it->second;
		ItemInstance* inst_new = nullptr;

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
		m_scaledItem = new ItemData(*copy.m_scaledItem);
	else
		m_scaledItem = nullptr;

	if(copy.m_evolveInfo)
		m_evolveInfo = new EvolveInfo(*copy.m_evolveInfo);
	else
		m_evolveInfo = nullptr;

	m_scaling = copy.m_scaling;
	m_ornamenticon = copy.m_ornamenticon;
	m_ornamentidfile = copy.m_ornamentidfile;
	m_ornament_hero_model = copy.m_ornament_hero_model;
	m_recast_timestamp = copy.m_recast_timestamp;
	m_new_id_file = copy.m_new_id_file;
}

// Clean up container contents
EQEmu::ItemInstance::~ItemInstance()
{
	Clear();
	safe_delete(m_item);
	safe_delete(m_scaledItem);
	safe_delete(m_evolveInfo);
}

// Query item type
bool EQEmu::ItemInstance::IsType(item::ItemClass item_class) const
{
	// IsType(<ItemClassTypes>) does not protect against 'm_item = nullptr'
	
	// Check usage type
	if ((m_use_type == ItemInstWorldContainer) && (item_class == item::ItemClassBag))
		return true;

	if (!m_item)
		return false;

	return (m_item->ItemClass == item_class);
}

bool EQEmu::ItemInstance::IsClassCommon() const
{
	return (m_item && m_item->IsClassCommon());
}

bool EQEmu::ItemInstance::IsClassBag() const
{
	return (m_item && m_item->IsClassBag());
}

bool EQEmu::ItemInstance::IsClassBook() const
{
	return (m_item && m_item->IsClassBook());
}

// Is item stackable?
bool EQEmu::ItemInstance::IsStackable() const
{
	if (!m_item)
		return false;
	
	return m_item->Stackable;
}

bool EQEmu::ItemInstance::IsCharged() const
{
	if (!m_item)
		return false;
	
	if (m_item->MaxCharges > 1)
		return true;
	else
		return false;
}

// Can item be equipped?
bool EQEmu::ItemInstance::IsEquipable(uint16 race, uint16 class_) const
{
	if (!m_item || (m_item->Slots == 0))
		return false;

	return m_item->IsEquipable(race, class_);
}

// Can equip at this slot?
bool EQEmu::ItemInstance::IsEquipable(int16 slot_id) const
{
	if (!m_item)
		return false;

	if (slot_id < EQEmu::invslot::EQUIPMENT_BEGIN || slot_id > EQEmu::invslot::EQUIPMENT_END)
		return false;

	return ((m_item->Slots & (1 << slot_id)) != 0);
}

bool EQEmu::ItemInstance::IsAugmentable() const
{
	if (!m_item)
		return false;

	for (int index = invaug::SOCKET_BEGIN; index <= invaug::SOCKET_END; ++index) {
		if (m_item->AugSlotType[index] != 0)
			return true;
	}

	return false;
}

bool EQEmu::ItemInstance::AvailableWearSlot(uint32 aug_wear_slots) const {
	if (!m_item || !m_item->IsClassCommon())
		return false;

	int index = invslot::EQUIPMENT_BEGIN;
	for (; index <= invslot::EQUIPMENT_END; ++index) {
		if (m_item->Slots & (1 << index)) {
			if (aug_wear_slots & (1 << index))
				break;
		}
	}

	return (index <= EQEmu::invslot::EQUIPMENT_END);
}

int8 EQEmu::ItemInstance::AvailableAugmentSlot(int32 augtype) const
{
	if (!m_item || !m_item->IsClassCommon())
		return INVALID_INDEX;

	int index = invaug::SOCKET_BEGIN;
	for (; index <= invaug::SOCKET_END; ++index) {
		if (GetItem(index)) { continue; }
		if (augtype == -1 || (m_item->AugSlotType[index] && ((1 << (m_item->AugSlotType[index] - 1)) & augtype)))
			break;
	}

	return (index <= invaug::SOCKET_END) ? index : INVALID_INDEX;
}

bool EQEmu::ItemInstance::IsAugmentSlotAvailable(int32 augtype, uint8 slot) const
{
	if (!m_item || !m_item->IsClassCommon())
		 return false;

	if ((!GetItem(slot) && m_item->AugSlotVisible[slot]) && augtype == -1 || (m_item->AugSlotType[slot] && ((1 << (m_item->AugSlotType[slot] - 1)) & augtype))) {
		return true;
	}
		return false;
}

// Retrieve item inside container
EQEmu::ItemInstance* EQEmu::ItemInstance::GetItem(uint8 index) const
{
	auto it = m_contents.find(index);
	if (it != m_contents.end()) {
		return it->second;
	}

	return nullptr;
}

uint32 EQEmu::ItemInstance::GetItemID(uint8 slot) const
{
	ItemInstance *item = GetItem(slot);
	if (item)
		return item->GetID();

	return 0;
}

void EQEmu::ItemInstance::PutItem(uint8 index, const ItemInstance& inst)
{
	// Clean up item already in slot (if exists)
	DeleteItem(index);

	// Delegate to internal method
	_PutItem(index, inst.Clone());
}

// Remove item inside container
void EQEmu::ItemInstance::DeleteItem(uint8 index)
{
	ItemInstance* inst = PopItem(index);
	safe_delete(inst);
}

// Remove item from container without memory delete
// Hands over memory ownership to client of this function call
EQEmu::ItemInstance* EQEmu::ItemInstance::PopItem(uint8 index)
{
	auto iter = m_contents.find(index);
	if (iter != m_contents.end()) {
		ItemInstance* inst = iter->second;
		m_contents.erase(index);
		return inst; // Return pointer that needs to be deleted (or otherwise managed)
	}
	
	return nullptr;
}

// Remove all items from container
void EQEmu::ItemInstance::Clear()
{
	// Destroy container contents
	for (auto iter = m_contents.begin(); iter != m_contents.end(); ++iter) {
		safe_delete(iter->second);
	}
	m_contents.clear();
}

// Remove all items from container
void EQEmu::ItemInstance::ClearByFlags(byFlagSetting is_nodrop, byFlagSetting is_norent)
{
	// TODO: This needs work...

	// Destroy container contents
	std::map<uint8, ItemInstance*>::const_iterator cur, end, del;
	cur = m_contents.begin();
	end = m_contents.end();
	for (; cur != end;) {
		ItemInstance* inst = cur->second;
		if (inst == nullptr) {
			cur = m_contents.erase(cur);
			continue;
		}

		const ItemData* item = inst->GetItem();
		if (item == nullptr) {
			cur = m_contents.erase(cur);
			continue;
		}

		del = cur;
		++cur;

		switch (is_nodrop) {
		case byFlagSet:
			if (item->NoDrop == 0) {
				safe_delete(inst);
				m_contents.erase(del->first);
				continue;
			}
			// no 'break;' deletes 'byFlagNotSet' type - can't add at the moment because it really *breaks* the process somewhere
		case byFlagNotSet:
			if (item->NoDrop != 0) {
				safe_delete(inst);
				m_contents.erase(del->first);
				continue;
			}
		default:
			break;
		}

		switch (is_norent) {
		case byFlagSet:
			if (item->NoRent == 0) {
				safe_delete(inst);
				m_contents.erase(del->first);
				continue;
			}
			// no 'break;' deletes 'byFlagNotSet' type - can't add at the moment because it really *breaks* the process somewhere
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

uint8 EQEmu::ItemInstance::FirstOpenSlot() const
{
	if (!m_item)
		return INVALID_INDEX;

	uint8 slots = m_item->BagSlots, i;
	for (i = invbag::SLOT_BEGIN; i < slots; i++) {
		if (!GetItem(i))
			break;
	}

	return (i < slots) ? i : INVALID_INDEX;
}

uint8 EQEmu::ItemInstance::GetTotalItemCount() const
{
	if (!m_item)
		return 0;
	
	uint8 item_count = 1;

	if (m_item && !m_item->IsClassBag()) { return item_count; }

	for (int index = invbag::SLOT_BEGIN; index < m_item->BagSlots; ++index) { if (GetItem(index)) { ++item_count; } }

	return item_count;
}

bool EQEmu::ItemInstance::IsNoneEmptyContainer()
{
	if (!m_item || !m_item->IsClassBag())
		return false;

	for (int index = invbag::SLOT_BEGIN; index < m_item->BagSlots; ++index) {
		if (GetItem(index))
			return true;
	}

	return false;
}

// Retrieve augment inside item
EQEmu::ItemInstance* EQEmu::ItemInstance::GetAugment(uint8 slot) const
{
	if (m_item && m_item->IsClassCommon())
		return GetItem(slot);

	return nullptr;
}

EQEmu::ItemInstance* EQEmu::ItemInstance::GetOrnamentationAug(int32 ornamentationAugtype) const
{
	if (!m_item || !m_item->IsClassCommon()) { return nullptr; }
	if (ornamentationAugtype == 0) { return nullptr; }

	for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; i++)
	{
		if (GetAugment(i) && m_item->AugSlotType[i] == ornamentationAugtype)
		{
			const char *item_IDFile = GetAugment(i)->GetItem()->IDFile;
			if (
				(strncmp(item_IDFile, "IT64", strlen(item_IDFile)) == 0
				|| strncmp(item_IDFile, "IT63", strlen(item_IDFile)) == 0)
				&& GetAugment(i)->GetItem()->HerosForgeModel == 0
				)
			{
				continue;
			}
			return this->GetAugment(i);
		}
	}

	return nullptr;
}

uint32 EQEmu::ItemInstance::GetOrnamentHeroModel(int32 material_slot) const {
	// Not a Hero Forge item.
	if (m_ornament_hero_model == 0 || material_slot < 0)
		return 0;

	// Item is using an explicit Hero Forge ID
	if (m_ornament_hero_model >= 1000)
		return m_ornament_hero_model;

	// Item is using a shorthand ID
	return (m_ornament_hero_model * 100) + material_slot;
}

bool EQEmu::ItemInstance::UpdateOrnamentationInfo() {
	if (!m_item || !m_item->IsClassCommon())
		return false;
	
	bool ornamentSet = false;

	int32 ornamentationAugtype = RuleI(Character, OrnamentationAugmentType);
	if (GetOrnamentationAug(ornamentationAugtype))
	{
		const ItemData* ornamentItem;
		ornamentItem = GetOrnamentationAug(ornamentationAugtype)->GetItem();
		if (ornamentItem != nullptr)
		{
			SetOrnamentIcon(ornamentItem->Icon);
			SetOrnamentHeroModel(ornamentItem->HerosForgeModel);
			if (strlen(ornamentItem->IDFile) > 2)
			{
				SetOrnamentationIDFile(atoi(&ornamentItem->IDFile[2]));
			}
			else
			{
				SetOrnamentationIDFile(0);
			}
			ornamentSet = true;
		}
	}
	else
	{
		SetOrnamentIcon(0);
		SetOrnamentHeroModel(0);
		SetOrnamentationIDFile(0);
	}

	return ornamentSet;
}

bool EQEmu::ItemInstance::CanTransform(const ItemData *ItemToTry, const ItemData *Container, bool AllowAll) {
	if (!ItemToTry || !Container) return false;

	if (ItemToTry->ItemType == item::ItemTypeArrow || strnlen(Container->CharmFile, 30) == 0)
		return false;

	if (AllowAll && strncasecmp(Container->CharmFile, "ITEMTRANSFIGSHIELD", 18) && strncasecmp(Container->CharmFile, "ITEMTransfigBow", 15)) {
		switch (ItemToTry->ItemType) {
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 35:
			case 45:
				return true;
		}
	}

	static std::map<std::string, int> types;
	types["itemtransfig1hp"] = 2;
	types["itemtransfig1hs"] = 0;
	types["itemtransfig2hb"] = 4;
	types["itemtransfig2hp"] = 35;
	types["itemtransfig2hs"] = 1;
	types["itemtransfigblunt"] = 3;
	types["itemtransfig1hb"] = 3;
	types["itemtransfigbow"] = 5;
	types["itemtransfighth"] = 45;
	types["itemtransfigshield"] = 8;
	types["itemtransfigslashing"] = 0;

	auto i = types.find(MakeLowerString(Container->CharmFile));
	if (i != types.end() && i->second == ItemToTry->ItemType)
		return true;

	static std::map<std::string, int> typestwo;
	typestwo["itemtransfigblunt"] = 4;
	typestwo["itemtransfigslashing"] = 1;

	i = typestwo.find(MakeLowerString(Container->CharmFile));
	if (i != typestwo.end() && i->second == ItemToTry->ItemType)
		return true;

	return false;
}

uint32 EQEmu::ItemInstance::GetAugmentItemID(uint8 slot) const
{
	if (!m_item || !m_item->IsClassCommon())
		return 0;

	return GetItemID(slot);
}

// Add an augment to the item
void EQEmu::ItemInstance::PutAugment(uint8 slot, const ItemInstance& augment)
{
	if (!m_item || !m_item->IsClassCommon())
		return;
	
	PutItem(slot, augment);
}

void EQEmu::ItemInstance::PutAugment(SharedDatabase *db, uint8 slot, uint32 item_id)
{
	if (item_id == 0) { return; }
	if (db == nullptr) { return; /* TODO: add log message for nullptr */ }

	const ItemInstance* aug = db->CreateItem(item_id);
	if (aug) {
		PutAugment(slot, *aug);
		safe_delete(aug);
	}	
}

// Remove augment from item and destroy it
void EQEmu::ItemInstance::DeleteAugment(uint8 index)
{
	if (!m_item || !m_item->IsClassCommon())
		return;
	
	DeleteItem(index);
}

// Remove augment from item and return it
EQEmu::ItemInstance* EQEmu::ItemInstance::RemoveAugment(uint8 index)
{
	if (!m_item || !m_item->IsClassCommon())
		return nullptr;
	
	return PopItem(index);
}

bool EQEmu::ItemInstance::IsAugmented()
{
	if (!m_item || !m_item->IsClassCommon())
		return false;
	
	for (int index = invaug::SOCKET_BEGIN; index <= invaug::SOCKET_END; ++index) {
		if (GetAugmentItemID(index))
			return true;
	}

	return false;
}

// Has attack/delay?
bool EQEmu::ItemInstance::IsWeapon() const
{
	if (!m_item || !m_item->IsClassCommon())
		return false;

	if (m_item->ItemType == item::ItemTypeArrow && m_item->Damage != 0)
		return true;
	else
		return ((m_item->Damage != 0) && (m_item->Delay != 0));
}

bool EQEmu::ItemInstance::IsAmmo() const
{
	if (!m_item)
		return false;

	if ((m_item->ItemType == item::ItemTypeArrow) ||
		(m_item->ItemType == item::ItemTypeLargeThrowing) ||
		(m_item->ItemType == item::ItemTypeSmallThrowing)
		) {
		return true;
	}

	return false;

}

const EQEmu::ItemData* EQEmu::ItemInstance::GetItem() const
{
	if (!m_item)
		return nullptr;

	if (m_scaledItem)
		return m_scaledItem;

	return m_item;
}

const EQEmu::ItemData* EQEmu::ItemInstance::GetUnscaledItem() const
{
	// No operator calls and defaults to nullptr
	return m_item;
}

std::string EQEmu::ItemInstance::GetCustomDataString() const {
	std::string ret_val;
	auto iter = m_custom_data.begin();
	while (iter != m_custom_data.end()) {
		if (ret_val.length() > 0) {
			ret_val += "^";
		}
		ret_val += iter->first;
		ret_val += "^";
		ret_val += iter->second;
		++iter;

		if (ret_val.length() > 0) {
			ret_val += "^";
		}
	}
	return ret_val;
}

std::string EQEmu::ItemInstance::GetCustomData(std::string identifier) {
	std::map<std::string, std::string>::const_iterator iter = m_custom_data.find(identifier);
	if (iter != m_custom_data.end()) {
		return iter->second;
	}

	return "";
}

void EQEmu::ItemInstance::SetCustomData(std::string identifier, std::string value) {
	DeleteCustomData(identifier);
	m_custom_data[identifier] = value;
}

void EQEmu::ItemInstance::SetCustomData(std::string identifier, int value) {
	DeleteCustomData(identifier);
	std::stringstream ss;
	ss << value;
	m_custom_data[identifier] = ss.str();
}

void EQEmu::ItemInstance::SetCustomData(std::string identifier, float value) {
	DeleteCustomData(identifier);
	std::stringstream ss;
	ss << value;
	m_custom_data[identifier] = ss.str();
}

void EQEmu::ItemInstance::SetCustomData(std::string identifier, bool value) {
	DeleteCustomData(identifier);
	std::stringstream ss;
	ss << value;
	m_custom_data[identifier] = ss.str();
}

void EQEmu::ItemInstance::DeleteCustomData(std::string identifier) {
	auto iter = m_custom_data.find(identifier);
	if (iter != m_custom_data.end()) {
		m_custom_data.erase(iter);
	}
}

// Clone a type of EQEmu::ItemInstance object
// c++ doesn't allow a polymorphic copy constructor,
// so we have to resort to a polymorphic Clone()
EQEmu::ItemInstance* EQEmu::ItemInstance::Clone() const
{
	// Pseudo-polymorphic copy constructor
	return new ItemInstance(*this);
}

bool EQEmu::ItemInstance::IsSlotAllowed(int16 slot_id) const {
	if (!m_item) { return false; }
	else if (InventoryProfile::SupportsContainers(slot_id)) { return true; }
	else if (m_item->Slots & (1 << slot_id)) { return true; }
	else if (slot_id > invslot::EQUIPMENT_END) { return true; } // why do we call 'InventoryProfile::SupportsContainers' with this here?
	else { return false; }
}

bool EQEmu::ItemInstance::IsDroppable(bool recurse) const
{
	if (!m_item)
		return false;
	/*if (m_ornamentidfile) // not implemented
		return false;*/
	if (m_attuned)
		return false;
	/*if (m_item->FVNoDrop != 0) // not implemented
		return false;*/
	if (m_item->NoDrop == 0)
		return false;

	if (recurse) {
		for (auto iter : m_contents) {
			if (!iter.second)
				continue;

			if (!iter.second->IsDroppable(recurse))
				return false;
		}
	}
	
	return true;
}

void EQEmu::ItemInstance::Initialize(SharedDatabase *db) {
	// if there's no actual item, don't do anything
	if (!m_item)
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

void EQEmu::ItemInstance::ScaleItem() {
	if (!m_item)
		return;

	if (m_scaledItem) {
		memcpy(m_scaledItem, m_item, sizeof(ItemData));
	}
	else {
		m_scaledItem = new ItemData(*m_item);
	}

	float Mult = (float)(GetExp()) / 10000;	// scaling is determined by exp, with 10,000 being full stats

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

	// check these..some may not need to be modified (really need to check all stats/bonuses)
	//m_scaledItem->SkillModValue = (int32)((float)m_item->SkillModValue*Mult);
	//m_scaledItem->BaneDmgAmt = (int8)((float)m_item->BaneDmgAmt*Mult);	// watch (10 entries with charmfileid)
	m_scaledItem->BardValue = (int32)((float)m_item->BardValue*Mult);		// watch (no entries with charmfileid)
	m_scaledItem->ElemDmgAmt = (uint8)((float)m_item->ElemDmgAmt*Mult);		// watch (no entries with charmfileid)
	m_scaledItem->Damage = (uint32)((float)m_item->Damage*Mult);			// watch

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

bool EQEmu::ItemInstance::EvolveOnAllKills() const {
	return (m_evolveInfo && m_evolveInfo->AllKills);
}

int8 EQEmu::ItemInstance::GetMaxEvolveLvl() const {
	if (m_evolveInfo)
		return m_evolveInfo->MaxLvl;
	else
		return 0;
}

uint32 EQEmu::ItemInstance::GetKillsNeeded(uint8 currentlevel) {
	uint32 kills = -1;	// default to -1 (max uint32 value) because this value is usually divided by, so we don't want to ever return zero.
	if (m_evolveInfo)
		if (currentlevel != m_evolveInfo->MaxLvl)
			kills = m_evolveInfo->LvlKills[currentlevel - 1];

	if (kills == 0)
		kills = -1;

	return kills;
}

void EQEmu::ItemInstance::SetTimer(std::string name, uint32 time) {
	Timer t(time);
	t.Start(time, false);
	m_timers[name] = t;
}

void EQEmu::ItemInstance::StopTimer(std::string name) {
	auto iter = m_timers.find(name);
	if(iter != m_timers.end()) {
		m_timers.erase(iter);
	}
}

void EQEmu::ItemInstance::ClearTimers() {
	m_timers.clear();
}

int EQEmu::ItemInstance::GetItemArmorClass(bool augments) const
{
	int ac = 0;
	const auto item = GetItem();
	if (item) {
		ac = item->AC;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					ac += GetAugment(i)->GetItemArmorClass();
	}
	return ac;
}

int EQEmu::ItemInstance::GetItemElementalDamage(int &magic, int &fire, int &cold, int &poison, int &disease, int &chromatic, int &prismatic, int &physical, int &corruption, bool augments) const
{
	const auto item = GetItem();
	if (item) {
		switch (item->ElemDmgType) {
		case RESIST_MAGIC:
			magic += item->ElemDmgAmt;
			break;
		case RESIST_FIRE:
			fire += item->ElemDmgAmt;
			break;
		case RESIST_COLD:
			cold += item->ElemDmgAmt;
			break;
		case RESIST_POISON:
			poison += item->ElemDmgAmt;
			break;
		case RESIST_DISEASE:
			disease += item->ElemDmgAmt;
			break;
		case RESIST_CHROMATIC:
			chromatic += item->ElemDmgAmt;
			break;
		case RESIST_PRISMATIC:
			prismatic += item->ElemDmgAmt;
			break;
		case RESIST_PHYSICAL:
			physical += item->ElemDmgAmt;
			break;
		case RESIST_CORRUPTION:
			corruption += item->ElemDmgAmt;
			break;
		}

		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					GetAugment(i)->GetItemElementalDamage(magic, fire, cold, poison, disease, chromatic, prismatic, physical, corruption);
	}
	return magic + fire + cold + poison + disease + chromatic + prismatic + physical + corruption;
}

int EQEmu::ItemInstance::GetItemElementalFlag(bool augments) const
{
	int flag = 0;
	const auto item = GetItem();
	if (item) {
		flag = item->ElemDmgType;
		if (flag)
			return flag;

		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i) {
				if (GetAugment(i))
					flag = GetAugment(i)->GetItemElementalFlag();
				if (flag)
					return flag;
			}
		}
	}
	return flag;
}

int EQEmu::ItemInstance::GetItemElementalDamage(bool augments) const
{
	int damage = 0;
	const auto item = GetItem();
	if (item) {
		damage = item->ElemDmgAmt;
		if (damage)
			return damage;

		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i) {
				if (GetAugment(i))
					damage = GetAugment(i)->GetItemElementalDamage();
				if (damage)
					return damage;
			}
		}
	}
	return damage;
}

int EQEmu::ItemInstance::GetItemRecommendedLevel(bool augments) const
{
	int level = 0;
	const auto item = GetItem();
	if (item) {
		level = item->RecLevel;

		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i) {
				int temp = 0;
				if (GetAugment(i)) {
					temp = GetAugment(i)->GetItemRecommendedLevel();
					if (temp > level)
						level = temp;
				}
			}
		}
	}

	return level;
}

int EQEmu::ItemInstance::GetItemRequiredLevel(bool augments) const
{
	int level = 0;
	const auto item = GetItem();
	if (item) {
		level = item->ReqLevel;

		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i) {
				int temp = 0;
				if (GetAugment(i)) {
					temp = GetAugment(i)->GetItemRequiredLevel();
					if (temp > level)
						level = temp;
				}
			}
		}
	}

	return level;
}

int EQEmu::ItemInstance::GetItemWeaponDamage(bool augments) const
{
	int damage = 0;
	const auto item = GetItem();
	if (item) {
		damage = item->Damage;

		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					damage += GetAugment(i)->GetItemWeaponDamage();
		}
	}
	return damage;
}

int EQEmu::ItemInstance::GetItemBackstabDamage(bool augments) const
{
	int damage = 0;
	const auto item = GetItem();
	if (item) {
		damage = item->BackstabDmg;

		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					damage += GetAugment(i)->GetItemBackstabDamage();
		}
	}
	return damage;
}

int EQEmu::ItemInstance::GetItemBaneDamageBody(bool augments) const
{
	int body = 0;
	const auto item = GetItem();
	if (item) {
		body = item->BaneDmgBody;
		if (body)
			return body;

		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i)) {
					body = GetAugment(i)->GetItemBaneDamageBody();
					if (body)
						return body;
				}
		}
	}
	return body;
}

int EQEmu::ItemInstance::GetItemBaneDamageRace(bool augments) const
{
	int race = 0;
	const auto item = GetItem();
	if (item) {
		race = item->BaneDmgRace;
		if (race)
			return race;

		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i)) {
					race = GetAugment(i)->GetItemBaneDamageRace();
					if (race)
						return race;
				}
		}
	}
	return race;
}

int EQEmu::ItemInstance::GetItemBaneDamageBody(bodyType against, bool augments) const
{
	int damage = 0;
	const auto item = GetItem();
	if (item) {
		if (item->BaneDmgBody == against)
			damage += item->BaneDmgAmt;

		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					damage += GetAugment(i)->GetItemBaneDamageBody(against);
		}
	}
	return damage;
}

int EQEmu::ItemInstance::GetItemBaneDamageRace(uint16 against, bool augments) const
{
	int damage = 0;
	const auto item = GetItem();
	if (item) {
		if (item->BaneDmgRace == against)
			damage += item->BaneDmgRaceAmt;

		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					damage += GetAugment(i)->GetItemBaneDamageRace(against);
		}
	}
	return damage;
}

int EQEmu::ItemInstance::GetItemMagical(bool augments) const
{
	const auto item = GetItem();
	if (item) {
		if (item->Magic)
			return 1;

		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i) && GetAugment(i)->GetItemMagical())
					return 1;
		}
	}
	return 0;
}

int EQEmu::ItemInstance::GetItemHP(bool augments) const
{
	int hp = 0;
	const auto item = GetItem();
	if (item) {
		hp = item->HP;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					hp += GetAugment(i)->GetItemHP();
	}
	return hp;
}

int EQEmu::ItemInstance::GetItemMana(bool augments) const
{
	int mana = 0;
	const auto item = GetItem();
	if (item) {
		mana = item->Mana;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					mana += GetAugment(i)->GetItemMana();
	}
	return mana;
}

int EQEmu::ItemInstance::GetItemEndur(bool augments) const
{
	int endur = 0;
	const auto item = GetItem();
	if (item) {
		endur = item->Endur;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					endur += GetAugment(i)->GetItemEndur();
	}
	return endur;
}

int EQEmu::ItemInstance::GetItemAttack(bool augments) const
{
	int atk = 0;
	const auto item = GetItem();
	if (item) {
		atk = item->Attack;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					atk += GetAugment(i)->GetItemAttack();
	}
	return atk;
}

int EQEmu::ItemInstance::GetItemStr(bool augments) const
{
	int str = 0;
	const auto item = GetItem();
	if (item) {
		str = item->AStr;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					str += GetAugment(i)->GetItemStr();
	}
	return str;
}

int EQEmu::ItemInstance::GetItemSta(bool augments) const
{
	int sta = 0;
	const auto item = GetItem();
	if (item) {
		sta = item->ASta;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					sta += GetAugment(i)->GetItemSta();
	}
	return sta;
}

int EQEmu::ItemInstance::GetItemDex(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->ADex;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemDex();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemAgi(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->AAgi;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemAgi();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemInt(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->AInt;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemInt();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemWis(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->AWis;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemWis();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemCha(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->ACha;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemCha();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemMR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->MR;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemMR();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemFR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->FR;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemFR();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemCR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->CR;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemCR();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemPR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->PR;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemPR();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemDR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->DR;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemDR();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemCorrup(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->SVCorruption;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemCorrup();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemHeroicStr(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicStr;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicStr();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemHeroicSta(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicSta;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicSta();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemHeroicDex(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicDex;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicDex();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemHeroicAgi(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicAgi;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicAgi();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemHeroicInt(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicInt;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicInt();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemHeroicWis(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicWis;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicWis();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemHeroicCha(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicCha;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicCha();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemHeroicMR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicMR;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicMR();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemHeroicFR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicFR;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicFR();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemHeroicCR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicCR;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicCR();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemHeroicPR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicPR;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicPR();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemHeroicDR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicDR;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicDR();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemHeroicCorrup(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicSVCorrup;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicCorrup();
	}
	return total;
}

int EQEmu::ItemInstance::GetItemHaste(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->Haste;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i)) {
					int temp = GetAugment(i)->GetItemHaste();
					if (temp > total)
						total = temp;
				}
	}
	return total;
}

//
// class EvolveInfo
//
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
