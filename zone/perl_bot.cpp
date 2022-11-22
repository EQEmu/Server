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

EQ::ItemInstance* Perl_Bot_GetBotItem(Bot* self, uint16 slot_id)
{
	return self->GetBotItem(slot_id);
}

uint32 Perl_Bot_GetBotItemIDBySlot(Bot* self, uint16 slot_id)
{
	return self->GetBotItemBySlot(slot_id);
}

void Perl_Bot_SignalBot(Bot* self, int signal_id)
{
	self->SignalBot(signal_id);
}

void Perl_Bot_OwnerMessage(Bot* self, std::string message)
{
	self->OwnerMessage(message);
}

int Perl_Bot_GetExpansionBitmask(Bot* self)
{
	return self->GetExpansionBitmask();
}

void Perl_Bot_SetExpansionBitmask(Bot* self, int expansion_bitmask)
{
	self->SetExpansionBitmask(expansion_bitmask);
}

void Perl_Bot_SetExpansionBitmask(Bot* self, int expansion_bitmask, bool save)
{
	self->SetExpansionBitmask(expansion_bitmask, save);
}

bool Perl_Bot_CheckBotSpellEntries(Bot* self, int spellid)
{
	return self->CheckBotSpellEntries(spellid);
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
	package.add("CountBotItem", &Perl_Bot_CountBotItem);
	package.add("GetBotItem", &Perl_Bot_GetBotItem);
	package.add("GetBotItemIDBySlot", &Perl_Bot_GetBotItemIDBySlot);
	package.add("GetExpansionBitmask", &Perl_Bot_GetExpansionBitmask);
	package.add("GetOwner", &Perl_Bot_GetOwner);
	package.add("HasBotItem", &Perl_Bot_HasBotItem);
	package.add("OwnerMessage", &Perl_Bot_OwnerMessage);
	package.add("RemoveBotItem", &Perl_Bot_RemoveBotItem);
	package.add("SetExpansionBitmask", (void(*)(Bot*, int))&Perl_Bot_SetExpansionBitmask);
	package.add("SetExpansionBitmask", (void(*)(Bot*, int, bool))&Perl_Bot_SetExpansionBitmask);
	package.add("SignalBot", &Perl_Bot_SignalBot);
	package.add("CheckBotSpellEntries", &Perl_Bot_CheckBotSpellEntries);
}

#endif //EMBPERL_XS_CLASSES
#endif //BOTS

