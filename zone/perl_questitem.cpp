#include "../common/features.h"
#include "../common/languages.h"
#include "client.h"

#ifdef EMBPERL_XS_CLASSES

#include "../common/global_define.h"
#include "embperl.h"

std::string Perl_QuestItem_GetName(EQ::ItemInstance* self) // @categories Inventory and Items
{
	return self->GetItem()->Name;
}

void Perl_QuestItem_SetScale(EQ::ItemInstance* self, float scale_multiplier) // @categories Inventory and Items
{
	if (self->IsScaling()) {
		self->SetExp((int) (scale_multiplier * 10000 + .5));
	}
}

void Perl_QuestItem_ItemSay(EQ::ItemInstance* self, const char* text) // @categories Inventory and Items
{
	quest_manager.GetInitiator()->ChannelMessageSend(self->GetItem()->Name, 0, ChatChannel_Say, LANG_COMMON_TONGUE, MAX_LANGUAGE_SKILL, text);
}

void Perl_QuestItem_ItemSay(EQ::ItemInstance* self, const char* text, uint8 language_id) // @categories Inventory and Items
{
	quest_manager.GetInitiator()->ChannelMessageSend(self->GetItem()->Name, 0, ChatChannel_Say, language_id, MAX_LANGUAGE_SKILL, text);
}

bool Perl_QuestItem_IsType(EQ::ItemInstance* self, int type) // @categories Inventory and Items
{
	return self->IsType(static_cast<EQ::item::ItemClass>(type));
}

bool Perl_QuestItem_IsAttuned(EQ::ItemInstance* self) // @categories Inventory and Items
{
	return self->IsAttuned();
}

int Perl_QuestItem_GetCharges(EQ::ItemInstance* self) // @categories Inventory and Items
{
	return self->GetCharges();
}

EQ::ItemInstance* Perl_QuestItem_GetAugment(EQ::ItemInstance* self, uint8 slot_id) // @categories Inventory and Items
{
	return self->GetAugment(slot_id);
}

uint32_t Perl_QuestItem_GetID(EQ::ItemInstance* self) // @categories Inventory and Items
{
	return self->GetItem()->ID;
}

bool Perl_QuestItem_ContainsAugmentByID(EQ::ItemInstance* self, uint32_t item_id) // @categories Inventory and Items
{
	return self->ContainsAugmentByID(item_id);
}

int Perl_QuestItem_CountAugmentByID(EQ::ItemInstance* self, uint32_t item_id) // @categories Inventory and Items
{
	return self->CountAugmentByID(item_id);
}

bool Perl_QuestItem_IsStackable(EQ::ItemInstance* self)
{
	return self->IsStackable();
}

void Perl_QuestItem_SetCharges(EQ::ItemInstance* self, int16_t charges)
{
	self->SetCharges(charges);
}

int Perl_QuestItem_GetTaskDeliveredCount(EQ::ItemInstance* self)
{
	return self->GetTaskDeliveredCount();
}

int Perl_QuestItem_RemoveTaskDeliveredItems(EQ::ItemInstance* self)
{
	return self->RemoveTaskDeliveredItems();
}

void Perl_QuestItem_AddEXP(EQ::ItemInstance* self, uint32 exp)
{
	self->AddExp(exp);
}

void Perl_QuestItem_ClearTimers(EQ::ItemInstance* self)
{
	self->ClearTimers();
}

EQ::ItemInstance* Perl_QuestItem_Clone(EQ::ItemInstance* self)
{
	return self->Clone();
}

void Perl_QuestItem_DeleteCustomData(EQ::ItemInstance* self, std::string identifier)
{
	self->DeleteCustomData(identifier);
}

uint32 Perl_QuestItem_GetAugmentItemID(EQ::ItemInstance* self, uint8 slot_id)
{
	return self->GetAugmentItemID(slot_id);
}

int Perl_QuestItem_GetAugmentType(EQ::ItemInstance* self)
{
	return self->GetAugmentType();
}

uint32 Perl_QuestItem_GetColor(EQ::ItemInstance* self)
{
	return self->GetColor();
}

std::string Perl_QuestItem_GetCustomData(EQ::ItemInstance* self, std::string identifier)
{
	return self->GetCustomData(identifier);
}

std::string Perl_QuestItem_GetCustomDataString(EQ::ItemInstance* self)
{
	return self->GetCustomDataString();
}

uint32 Perl_QuestItem_GetEXP(EQ::ItemInstance* self)
{
	return self->GetExp();
}

EQ::ItemInstance* Perl_QuestItem_GetItem(EQ::ItemInstance* self, uint8 slot_id)
{
	return self->GetItem(slot_id);
}

uint32 Perl_QuestItem_GetItemID(EQ::ItemInstance* self, uint8 slot_id)
{
	return self->GetItemID(slot_id);
}

uint32 Perl_QuestItem_GetItemScriptID(EQ::ItemInstance* self)
{
	return self->GetItemScriptID();
}

uint32 Perl_QuestItem_GetKillsNeeded(EQ::ItemInstance* self, uint8 current_level)
{
	return self->GetKillsNeeded(current_level);
}

int8 Perl_QuestItem_GetMaxEvolveLevel(EQ::ItemInstance* self)
{
	return self->GetMaxEvolveLvl();
}

uint32 Perl_QuestItem_GetPrice(EQ::ItemInstance* self)
{
	return self->GetPrice();
}

uint8 Perl_QuestItem_GetTotalItemCount(EQ::ItemInstance* self)
{
	return self->GetTotalItemCount();
}

bool Perl_QuestItem_IsAmmo(EQ::ItemInstance* self)
{
	return self->IsAmmo();
}

bool Perl_QuestItem_IsAugmentable(EQ::ItemInstance* self)
{
	return self->IsAugmentable();
}

bool Perl_QuestItem_IsAugmented(EQ::ItemInstance* self)
{
	return self->IsAugmented();
}

bool Perl_QuestItem_IsEquipable(EQ::ItemInstance* self, int16 slot_id)
{
	return self->IsEquipable(slot_id);
}

bool Perl_QuestItem_IsEquipable(EQ::ItemInstance* self, uint16 race_bitmask, uint16 class_bitmask)
{
	return self->IsEquipable(race_bitmask, class_bitmask);
}

bool Perl_QuestItem_IsExpendable(EQ::ItemInstance* self)
{
	return self->IsExpendable();
}

bool Perl_QuestItem_IsWeapon(EQ::ItemInstance* self)
{
	return self->IsWeapon();
}

void Perl_QuestItem_SetAttuned(EQ::ItemInstance* self, bool is_attuned)
{
	self->SetAttuned(is_attuned);
}

void Perl_QuestItem_SetColor(EQ::ItemInstance* self, uint32 color)
{
	self->SetColor(color);
}

void Perl_QuestItem_SetEXP(EQ::ItemInstance* self, uint32 exp)
{
	self->SetExp(exp);
}

void Perl_QuestItem_SetPrice(EQ::ItemInstance* self, uint32 price)
{
	self->SetPrice(price);
}

void Perl_QuestItem_SetScaling(EQ::ItemInstance* self, bool is_scaling)
{
	self->SetScaling(is_scaling);
}

void Perl_QuestItem_SetTimer(EQ::ItemInstance* self, std::string timer_name, uint32 timer)
{
	self->SetTimer(timer_name, timer);
}

void Perl_QuestItem_StopTimer(EQ::ItemInstance* self, std::string timer_name)
{
	self->StopTimer(timer_name);
}

void perl_register_questitem()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_class<EQ::ItemInstance>("QuestItem");
	package.add("AddEXP", &Perl_QuestItem_AddEXP);
	package.add("ClearTimers", &Perl_QuestItem_ClearTimers);
	package.add("Clone", &Perl_QuestItem_Clone);
	package.add("ContainsAugmentByID", &Perl_QuestItem_ContainsAugmentByID);
	package.add("CountAugmentByID", &Perl_QuestItem_CountAugmentByID);
	package.add("DeleteCustomData", &Perl_QuestItem_DeleteCustomData);
	package.add("GetAugment", &Perl_QuestItem_GetAugment);
	package.add("GetAugmentItemID", &Perl_QuestItem_GetAugmentItemID);
	package.add("GetAugmentType", &Perl_QuestItem_GetAugmentType);
	package.add("GetCharges", &Perl_QuestItem_GetCharges);
	package.add("GetColor", &Perl_QuestItem_GetColor);
	package.add("GetCustomData", &Perl_QuestItem_GetCustomData);
	package.add("GetCustomDataString", &Perl_QuestItem_GetCustomDataString);
	package.add("GetEXP", &Perl_QuestItem_GetEXP);
	package.add("GetID", &Perl_QuestItem_GetID);
	package.add("GetItem", &Perl_QuestItem_GetItem);
	package.add("GetItemID", &Perl_QuestItem_GetItemID);
	package.add("GetItemScriptID", &Perl_QuestItem_GetItemScriptID);
	package.add("GetKillsNeeded", &Perl_QuestItem_GetKillsNeeded);
	package.add("GetMaxEvolveLevel", &Perl_QuestItem_GetMaxEvolveLevel);
	package.add("GetName", &Perl_QuestItem_GetName);
	package.add("GetPrice", &Perl_QuestItem_GetPrice);
	package.add("GetTaskDeliveredCount", &Perl_QuestItem_GetTaskDeliveredCount);
	package.add("GetTotalItemCount", &Perl_QuestItem_GetTotalItemCount);
	package.add("IsAmmo", &Perl_QuestItem_IsAmmo);
	package.add("IsAttuned", &Perl_QuestItem_IsAttuned);
	package.add("IsAugmentable", &Perl_QuestItem_IsAugmentable);
	package.add("IsAugmented", &Perl_QuestItem_IsAugmented);
	package.add("IsEquipable", (bool(*)(EQ::ItemInstance*, int16))&Perl_QuestItem_IsEquipable);
	package.add("IsEquipable", (bool(*)(EQ::ItemInstance*, uint16, uint16))&Perl_QuestItem_IsEquipable);
	package.add("IsExpendable", &Perl_QuestItem_IsExpendable);
	package.add("IsInstanceNoDrop", &Perl_QuestItem_IsAttuned);
	package.add("IsStackable", &Perl_QuestItem_IsStackable);
	package.add("IsType", &Perl_QuestItem_IsType);
	package.add("IsWeapon", &Perl_QuestItem_IsWeapon);
	package.add("ItemSay", (void(*)(EQ::ItemInstance*, const char*))&Perl_QuestItem_ItemSay);
	package.add("ItemSay", (void(*)(EQ::ItemInstance*, const char*, uint8))&Perl_QuestItem_ItemSay);
	package.add("RemoveTaskDeliveredItems", &Perl_QuestItem_RemoveTaskDeliveredItems);
	package.add("SetAttuned", &Perl_QuestItem_SetAttuned);
	package.add("SetCharges", &Perl_QuestItem_SetCharges);
	package.add("SetColor", &Perl_QuestItem_SetColor);
	package.add("SetCustomData", (void(*)(EQ::ItemInstance*, std::string, bool))&Perl_QuestItem_SetColor);
	package.add("SetCustomData", (void(*)(EQ::ItemInstance*, std::string, float))&Perl_QuestItem_SetColor);
	package.add("SetCustomData", (void(*)(EQ::ItemInstance*, std::string, int))&Perl_QuestItem_SetColor);
	package.add("SetCustomData", (void(*)(EQ::ItemInstance*, std::string, std::string))&Perl_QuestItem_SetColor);
	package.add("SetEXP", &Perl_QuestItem_SetEXP);
	package.add("SetInstanceNoDrop", &Perl_QuestItem_SetAttuned);
	package.add("SetPrice", &Perl_QuestItem_SetPrice);
	package.add("SetScale", &Perl_QuestItem_SetScale);
	package.add("SetScaling", &Perl_QuestItem_SetScaling);
	package.add("SetTimer", &Perl_QuestItem_SetTimer);
	package.add("StopTimer", &Perl_QuestItem_StopTimer);
}

#endif //EMBPERL_XS_CLASSES
