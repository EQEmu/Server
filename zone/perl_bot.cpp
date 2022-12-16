#ifdef BOTS
#include "../common/features.h"
#ifdef EMBPERL_XS_CLASSES
#include "../common/global_define.h"
#include "embperl.h"
#include "bot.h"

Mob* Perl_Bot_GetOwner(Bot* self) // @categories Script Utility, Bot
{
	return self->GetBotOwner();
}

// todo: should just take a hash instead of all these overloads
void Perl_Bot_AddBotItem(Bot* self, uint16 slot_id, uint32 item_id) // @categories Inventory and Items
{
	self->AddBotItem(slot_id, item_id);
}

void Perl_Bot_AddBotItem(Bot* self, uint16 slot_id, uint32 item_id, uint16 charges) // @categories Inventory and Items
{
	self->AddBotItem(slot_id, item_id, charges);
}

void Perl_Bot_AddBotItem(Bot* self, uint16 slot_id, uint32 item_id, uint16 charges, bool attuned) // @categories Inventory and Items
{
	self->AddBotItem(slot_id, item_id, charges, attuned);
}

void Perl_Bot_AddBotItem(Bot* self, uint16 slot_id, uint32 item_id, uint16 charges, bool attuned, uint32 aug1) // @categories Inventory and Items
{
	self->AddBotItem(slot_id, item_id, charges, attuned, aug1);
}

void Perl_Bot_AddBotItem(Bot* self, uint16 slot_id, uint32 item_id, uint16 charges, bool attuned, uint32 aug1, uint32 aug2) // @categories Inventory and Items
{
	self->AddBotItem(slot_id, item_id, charges, attuned, aug1, aug2);
}

void Perl_Bot_AddBotItem(Bot* self, uint16 slot_id, uint32 item_id, uint16 charges, bool attuned, uint32 aug1, uint32 aug2, uint32 aug3) // @categories Inventory and Items
{
	self->AddBotItem(slot_id, item_id, charges, attuned, aug1, aug2, aug3);
}

void Perl_Bot_AddBotItem(Bot* self, uint16 slot_id, uint32 item_id, uint16 charges, bool attuned, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4) // @categories Inventory and Items
{
	self->AddBotItem(slot_id, item_id, charges, attuned, aug1, aug2, aug3, aug4);
}

void Perl_Bot_AddBotItem(Bot* self, uint16 slot_id, uint32 item_id, uint16 charges, bool attuned, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5) // @categories Inventory and Items
{
	self->AddBotItem(slot_id, item_id, charges, attuned, aug1, aug2, aug3, aug4, aug5);
}

void Perl_Bot_AddBotItem(Bot* self, uint16 slot_id, uint32 item_id, uint16 charges, bool attuned, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5, uint32 aug6) // @categories Inventory and Items
{
	self->AddBotItem(slot_id, item_id, charges, attuned, aug1, aug2, aug3, aug4, aug5, aug6);
}

void Perl_Bot_ApplySpell(Bot* self, int spell_id)
{
	self->ApplySpell(spell_id);
}

void Perl_Bot_ApplySpell(Bot* self, int spell_id, int duration)
{
	self->ApplySpell(spell_id, duration);
}

void Perl_Bot_ApplySpell(Bot* self, int spell_id, int duration, bool allow_pets)
{
	self->ApplySpell(spell_id, duration, ApplySpellType::Solo, allow_pets);
}

void Perl_Bot_ApplySpellGroup(Bot* self, int spell_id)
{
	self->ApplySpell(spell_id, 0, ApplySpellType::Group);
}

void Perl_Bot_ApplySpellGroup(Bot* self, int spell_id, int duration)
{
	self->ApplySpell(spell_id, duration, ApplySpellType::Group);
}

void Perl_Bot_ApplySpellGroup(Bot* self, int spell_id, int duration, bool allow_pets)
{
	self->ApplySpell(spell_id, duration, ApplySpellType::Group, allow_pets);
}

uint32 Perl_Bot_CountBotItem(Bot* self, uint32 item_id)
{
	return self->CountBotItem(item_id);
}

bool Perl_Bot_HasBotItem(Bot* self, uint32 item_id)
{
	return self->HasBotItem(item_id);
}

void Perl_Bot_RemoveBotItem(Bot* self, uint32 item_id)
{
	return self->RemoveBotItem(item_id);
}

EQ::ItemInstance* Perl_Bot_GetAugmentAt(Bot* self, uint32 slot, uint32 aug_slot)
{
	EQ::ItemInstance* inst = self->GetInv().GetItem(slot);
	if (inst)
	{
		return inst->GetAugment(aug_slot);
	}
	return nullptr;
}

int Perl_Bot_CountAugmentEquippedByID(Bot* self, uint32 item_id)
{
	return self->GetInv().CountAugmentEquippedByID(item_id);
}

bool Perl_Bot_HasAugmentEquippedByID(Bot* self, uint32 item_id)
{
	return self->GetInv().HasAugmentEquippedByID(item_id);
}

int Perl_Bot_CountItemEquippedByID(Bot* self, uint32 item_id)
{
	return self->GetInv().CountItemEquippedByID(item_id);
}

bool Perl_Bot_HasItemEquippedByID(Bot* self, uint32 item_id)
{
	return self->GetInv().HasItemEquippedByID(item_id);
}

int Perl_Bot_GetRawItemAC(Bot* self) // @categories Inventory and Items
{
	return self->GetRawItemAC();
}

EQ::ItemInstance* Perl_Bot_GetBotItem(Bot* self, uint16 slot_id)
{
	return self->GetBotItem(slot_id);
}

uint32 Perl_Bot_GetBotItemIDBySlot(Bot* self, uint16 slot_id)
{
	return self->GetBotItemBySlot(slot_id);
}

bool Perl_Bot_IsStanding(Bot* self) // @categories Account and Character
{
	return self->IsStanding();
}

void Perl_Bot_Sit(Bot* self)
{
	self->Sit();
}

bool Perl_Bot_IsSitting(Bot* self) // @categories Account and Character
{
	return self->IsSitting();
}

void Perl_Bot_SendSpellAnim(Bot* self, uint16 targetid, uint16 spell_id)
{
	self->SendSpellAnim(targetid, spell_id);
}

void Perl_Bot_Signal(Bot* self, int signal_id)
{
	self->Signal(signal_id);
}

void Perl_Bot_OwnerMessage(Bot* self, std::string message)
{
	self->OwnerMessage(message);
}

int Perl_Bot_GetExpansionBitmask(Bot* self)
{
	return self->GetExpansionBitmask();
}

void Perl_Bot_Escape(Bot* self) // @categories Account and Character, Skills and Recipes
{
	self->Escape();
}

void Perl_Bot_Fling(Bot* self, float target_x, float target_y, float target_z)
{
	self->Fling(0, target_x, target_y, target_z, false, false, true);
}

void Perl_Bot_Fling(Bot* self, float target_x, float target_y, float target_z, bool ignore_los)
{
	self->Fling(0, target_x, target_y, target_z, ignore_los, false, true);
}

void Perl_Bot_Fling(Bot* self, float target_x, float target_y, float target_z, bool ignore_los, bool clip_through_walls)
{
	self->Fling(0, target_x, target_y, target_z, ignore_los, clip_through_walls, true);
}

void Perl_Bot_Fling(Bot* self, float value, float target_x, float target_y, float target_z)
{
	self->Fling(value, target_x, target_y, target_z);
}

void Perl_Bot_Fling(Bot* self, float value, float target_x, float target_y, float target_z, bool ignore_los)
{
	self->Fling(value, target_x, target_y, target_z, ignore_los);
}

void Perl_Bot_Fling(Bot* self, float value, float target_x, float target_y, float target_z, bool ignore_los, bool clip_through_walls)
{
	self->Fling(value, target_x, target_y, target_z, ignore_los, clip_through_walls);
}

void Perl_Bot_Stand(Bot* self) // @categories Script Utility
{
	self->Stand();
}

int Perl_Bot_GetItemIDAt(Bot* self, int16 slot_id) // @categories Inventory and Items
{
	return self->GetItemIDAt(slot_id);
}

int Perl_Bot_GetAugmentIDAt(Bot* self, int16 slot_id, uint8 aug_slot) // @categories Inventory and Items
{
	return self->GetAugmentIDAt(slot_id, aug_slot);
}

int Perl_Bot_GetBaseSTR(Bot* self) // @categories Stats and Attributes
{
	return self->GetBaseSTR();
}

int Perl_Bot_GetBaseSTA(Bot* self) // @categories Stats and Attributes
{
	return self->GetBaseSTA();
}

int Perl_Bot_GetBaseCHA(Bot* self) // @categories Stats and Attributes
{
	return self->GetBaseCHA();
}

int Perl_Bot_GetBaseDEX(Bot* self) // @categories Stats and Attributes
{
	return self->GetBaseDEX();
}

int Perl_Bot_GetBaseINT(Bot* self) // @categories Stats and Attributes
{
	return self->GetBaseINT();
}

int Perl_Bot_GetBaseAGI(Bot* self) // @categories Stats and Attributes
{
	return self->GetBaseAGI();
}

int Perl_Bot_GetBaseWIS(Bot* self) // @categories Stats and Attributes
{
	return self->GetBaseWIS();
}

Group* Perl_Bot_GetGroup(Bot* self) // @categories Account and Character, Group
{
	return self->GetGroup();
}

int Perl_Bot_GetHealAmount(Bot* self)
{
	return self->GetHealAmt();
}

int Perl_Bot_GetSpellDamage(Bot* self)
{
	return self->GetSpellDmg();
}

int Perl_Bot_GetInstrumentMod(Bot* self, uint16 spell_id) // @categories Spells and Disciplines
{
	return self->GetInstrumentMod(spell_id);
}

EQ::ItemInstance* Perl_Bot_GetItemAt(Bot* self, uint32 slot) // @categories Inventory and Items
{
	return self->GetInv().GetItem(slot);
}

bool Perl_Bot_IsGrouped(Bot* self) // @categories Account and Character, Group
{
	return self->IsGrouped();
}

void Perl_Bot_SetExpansionBitmask(Bot* self, int expansion_bitmask)
{
	self->SetExpansionBitmask(expansion_bitmask);
}

void Perl_Bot_SetExpansionBitmask(Bot* self, int expansion_bitmask, bool save)
{
	self->SetExpansionBitmask(expansion_bitmask, save);
}

void Perl_Bot_SetSpellDuration(Bot* self, int spell_id)
{
	self->SetSpellDuration(spell_id);
}

void Perl_Bot_SetSpellDuration(Bot* self, int spell_id, int duration)
{
	self->SetSpellDuration(spell_id, duration);
}

void Perl_Bot_SetSpellDuration(Bot* self, int spell_id, int duration, bool allow_pets)
{
	self->SetSpellDuration(spell_id, duration, ApplySpellType::Solo, allow_pets);
}

void Perl_Bot_SetSpellDurationGroup(Bot* self, int spell_id)
{
	self->SetSpellDuration(spell_id, 0, ApplySpellType::Group);
}

void Perl_Bot_SetSpellDurationGroup(Bot* self, int spell_id, int duration)
{
	self->SetSpellDuration(spell_id, duration, ApplySpellType::Group);
}

void Perl_Bot_SetSpellDurationGroup(Bot* self, int spell_id, int duration, bool allow_pets)
{
	self->SetSpellDuration(spell_id, duration, ApplySpellType::Group, allow_pets);
}

bool Perl_Bot_ReloadBotDataBuckets(Bot* self)
{
	return self->GetBotDataBuckets();
}

bool Perl_Bot_ReloadBotOwnerDataBuckets(Bot* self)
{
	return self->GetBotOwnerDataBuckets();
}

bool Perl_Bot_ReloadBotSpells(Bot* self)
{
	return self->AI_AddBotSpells(self->GetBotSpellID());
}

void Perl_Bot_ReloadBotSpellSettings(Bot* self)
{
	self->LoadBotSpellSettings();
}

bool Perl_Bot_HasBotSpellEntry(Bot* self, uint16 spellid)
{
	return self->HasBotSpellEntry(spellid);
}

void Perl_Bot_SendPayload(Bot* self, int payload_id) // @categories Script Utility
{
	self->SendPayload(payload_id);
}

void Perl_Bot_SendPayload(Bot* self, int payload_id, std::string payload_value) // @categories Script Utility
{
	self->SendPayload(payload_id, payload_value);
}

void perl_register_bot()
{
	perl::interpreter state(PERL_GET_THX);

	auto package = state.new_class<Bot>("Bot");
	package.add_base_class("NPC");
	package.add("AddBotItem", (void(*)(Bot*, uint16, uint32))&Perl_Bot_AddBotItem);
	package.add("AddBotItem", (void(*)(Bot*, uint16, uint32, uint16))&Perl_Bot_AddBotItem);
	package.add("AddBotItem", (void(*)(Bot*, uint16, uint32, uint16, bool))&Perl_Bot_AddBotItem);
	package.add("AddBotItem", (void(*)(Bot*, uint16, uint32, uint16, bool, uint32))&Perl_Bot_AddBotItem);
	package.add("AddBotItem", (void(*)(Bot*, uint16, uint32, uint16, bool, uint32, uint32))&Perl_Bot_AddBotItem);
	package.add("AddBotItem", (void(*)(Bot*, uint16, uint32, uint16, bool, uint32, uint32, uint32))&Perl_Bot_AddBotItem);
	package.add("AddBotItem", (void(*)(Bot*, uint16, uint32, uint16, bool, uint32, uint32, uint32, uint32))&Perl_Bot_AddBotItem);
	package.add("AddBotItem", (void(*)(Bot*, uint16, uint32, uint16, bool, uint32, uint32, uint32, uint32, uint32))&Perl_Bot_AddBotItem);
	package.add("AddBotItem", (void(*)(Bot*, uint16, uint32, uint16, bool, uint32, uint32, uint32, uint32, uint32, uint32))&Perl_Bot_AddBotItem);
	package.add("ApplySpell", (void(*)(Bot*, int))&Perl_Bot_ApplySpell);
	package.add("ApplySpell", (void(*)(Bot*, int, int))&Perl_Bot_ApplySpell);
	package.add("ApplySpell", (void(*)(Bot*, int, int, bool))&Perl_Bot_ApplySpell);
	package.add("ApplySpellGroup", (void(*)(Bot*, int))&Perl_Bot_ApplySpellGroup);
	package.add("ApplySpellGroup", (void(*)(Bot*, int, int))&Perl_Bot_ApplySpellGroup);
	package.add("ApplySpellGroup", (void(*)(Bot*, int, int, bool))&Perl_Bot_ApplySpellGroup);
	package.add("CountAugmentEquippedByID", &Perl_Bot_CountAugmentEquippedByID);
	package.add("CountBotItem", &Perl_Bot_CountBotItem); 
	package.add("CountItemEquippedByID", &Perl_Bot_CountItemEquippedByID);
	package.add("Escape", &Perl_Bot_Escape);
	package.add("Fling", (void(*)(Bot*, float, float, float))&Perl_Bot_Fling);
	package.add("Fling", (void(*)(Bot*, float, float, float, bool))&Perl_Bot_Fling);
	package.add("Fling", (void(*)(Bot*, float, float, float, bool, bool))&Perl_Bot_Fling);
	package.add("Fling", (void(*)(Bot*, float, float, float, float))&Perl_Bot_Fling);
	package.add("Fling", (void(*)(Bot*, float, float, float, float, bool))&Perl_Bot_Fling);
	package.add("Fling", (void(*)(Bot*, float, float, float, float, bool, bool))&Perl_Bot_Fling);
	package.add("GetAugmentAt", &Perl_Bot_GetAugmentAt);
	package.add("GetAugmentIDAt", &Perl_Bot_GetAugmentIDAt);
	package.add("GetBaseAGI", &Perl_Bot_GetBaseAGI);
	package.add("GetBaseCHA", &Perl_Bot_GetBaseCHA);
	package.add("GetBaseDEX", &Perl_Bot_GetBaseDEX);
	package.add("GetBaseINT", &Perl_Bot_GetBaseINT);
	package.add("GetBaseSTA", &Perl_Bot_GetBaseSTA);
	package.add("GetBaseSTR", &Perl_Bot_GetBaseSTR);
	package.add("GetBaseWIS", &Perl_Bot_GetBaseWIS);
	package.add("GetBotItem", &Perl_Bot_GetBotItem);
	package.add("GetBotItemIDBySlot", &Perl_Bot_GetBotItemIDBySlot);
	package.add("GetExpansionBitmask", &Perl_Bot_GetExpansionBitmask);
	package.add("GetGroup", &Perl_Bot_GetGroup);
	package.add("GetHealAmount", &Perl_Bot_GetHealAmount);
	package.add("GetInstrumentMod", &Perl_Bot_GetInstrumentMod);
	package.add("GetItemAt", &Perl_Bot_GetItemAt);
	package.add("GetItemIDAt", &Perl_Bot_GetItemIDAt);
	package.add("GetOwner", &Perl_Bot_GetOwner);
	package.add("GetRawItemAC", &Perl_Bot_GetRawItemAC);
	package.add("GetSpellDamage", &Perl_Bot_GetSpellDamage);
	package.add("HasAugmentEquippedByID", &Perl_Bot_HasAugmentEquippedByID);
	package.add("HasBotItem", &Perl_Bot_HasBotItem);
	package.add("HasBotSpellEntry", &Perl_Bot_HasBotSpellEntry);
	package.add("HasItemEquippedByID", &Perl_Bot_HasItemEquippedByID);
	package.add("IsGrouped", &Perl_Bot_IsGrouped);
	package.add("IsSitting", &Perl_Bot_IsSitting);
	package.add("IsStanding", &Perl_Bot_IsStanding);
	package.add("OwnerMessage", &Perl_Bot_OwnerMessage);
	package.add("ReloadBotDataBuckets", &Perl_Bot_ReloadBotDataBuckets);
	package.add("ReloadBotOwnerDataBuckets", &Perl_Bot_ReloadBotOwnerDataBuckets);
	package.add("ReloadBotSpells", &Perl_Bot_ReloadBotSpells);
	package.add("ReloadBotSpellSettings", &Perl_Bot_ReloadBotSpellSettings);
	package.add("RemoveBotItem", &Perl_Bot_RemoveBotItem);
	package.add("SendPayload", (void(*)(Bot*, int))&Perl_Bot_SendPayload);
	package.add("SendPayload", (void(*)(Bot*, int, std::string))&Perl_Bot_SendPayload);
	package.add("SendSpellAnim", &Perl_Bot_SendSpellAnim);
	package.add("SetExpansionBitmask", (void(*)(Bot*, int))&Perl_Bot_SetExpansionBitmask);
	package.add("SetExpansionBitmask", (void(*)(Bot*, int, bool))&Perl_Bot_SetExpansionBitmask);
	package.add("SetSpellDuration", (void(*)(Bot*, int))&Perl_Bot_SetSpellDuration);
	package.add("SetSpellDuration", (void(*)(Bot*, int, int))&Perl_Bot_SetSpellDuration);
	package.add("SetSpellDuration", (void(*)(Bot*, int, int, bool))&Perl_Bot_SetSpellDuration);
	package.add("SetSpellDurationGroup", (void(*)(Bot*, int))&Perl_Bot_SetSpellDurationGroup);
	package.add("SetSpellDurationGroup", (void(*)(Bot*, int, int))&Perl_Bot_SetSpellDurationGroup);
	package.add("SetSpellDurationGroup", (void(*)(Bot*, int, int, bool))&Perl_Bot_SetSpellDurationGroup);
	package.add("Signal", &Perl_Bot_Signal);
	package.add("Sit", &Perl_Bot_Sit);
	package.add("Stand", &Perl_Bot_Stand);
}

#endif //EMBPERL_XS_CLASSES
#endif //BOTS

