#include "../common/features.h"
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
	quest_manager.GetInitiator()->ChannelMessageSend(self->GetItem()->Name, 0, 8, 0, 100, text);
}

void Perl_QuestItem_ItemSay(EQ::ItemInstance* self, const char* text, int language_id) // @categories Inventory and Items
{
	quest_manager.GetInitiator()->ChannelMessageSend(self->GetItem()->Name, 0, 8, language_id, 100, text);
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

EQ::ItemInstance* Perl_QuestItem_GetAugment(EQ::ItemInstance* self, int slot_id) // @categories Inventory and Items
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

void perl_register_questitem()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_class<EQ::ItemInstance>("QuestItem");
	package.add("ContainsAugmentByID", &Perl_QuestItem_ContainsAugmentByID);
	package.add("CountAugmentByID", &Perl_QuestItem_CountAugmentByID);
	package.add("GetAugment", &Perl_QuestItem_GetAugment);
	package.add("GetCharges", &Perl_QuestItem_GetCharges);
	package.add("GetID", &Perl_QuestItem_GetID);
	package.add("GetName", &Perl_QuestItem_GetName);
	package.add("GetTaskDeliveredCount", &Perl_QuestItem_GetTaskDeliveredCount);
	package.add("IsAttuned", &Perl_QuestItem_IsAttuned);
	package.add("IsStackable", &Perl_QuestItem_IsStackable);
	package.add("IsType", &Perl_QuestItem_IsType);
	package.add("ItemSay", (void(*)(EQ::ItemInstance*, const char*))&Perl_QuestItem_ItemSay);
	package.add("ItemSay", (void(*)(EQ::ItemInstance*, const char*, int))&Perl_QuestItem_ItemSay);
	package.add("RemoveTaskDeliveredItems", &Perl_QuestItem_RemoveTaskDeliveredItems);
	package.add("SetCharges", &Perl_QuestItem_SetCharges);
	package.add("SetScale", &Perl_QuestItem_SetScale);
}

#endif //EMBPERL_XS_CLASSES
