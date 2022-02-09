#include "../client.h"
#include "../groups.h"

void command_iteminfo(Client *c, const Seperator *sep)
{
	auto inst = c->GetInv()[EQ::invslot::slotCursor];
	if (!inst) {
		c->Message(Chat::Red, "Error: You need an item on your cursor for this command");
		return;
	}
	auto item = inst->GetItem();
	if (!item) {
		LogInventory("([{}]) Command #iteminfo processed an item with no data pointer");
		c->Message(Chat::Red, "Error: This item has no data reference");
		return;
	}

	EQ::SayLinkEngine linker;
	linker.SetLinkType(EQ::saylink::SayLinkItemInst);
	linker.SetItemInst(inst);

	c->Message(Chat::White, "*** Item Info for [%s] ***", linker.GenerateLink().c_str());
	c->Message(Chat::White, ">> ID: %u, ItemUseType: %u, ItemClassType: %u", item->ID, item->ItemType, item->ItemClass);
	c->Message(Chat::White, ">> IDFile: '%s', IconID: %u", item->IDFile, item->Icon);
	c->Message(
		Chat::White,
		">> Size: %u, Weight: %u, Price: %u, LDoNPrice: %u",
		item->Size,
		item->Weight,
		item->Price,
		item->LDoNPrice
	);
	c->Message(
		Chat::White,
		">> Material: 0x%02X, Color: 0x%08X, Tint: 0x%08X, Light: 0x%02X",
		item->Material,
		item->Color,
		inst->GetColor(),
		item->Light
	);
	c->Message(
		Chat::White,
		">> IsLore: %s, LoreGroup: %u, Lore: '%s'",
		(item->LoreFlag ? "TRUE" : "FALSE"),
		item->LoreGroup,
		item->Lore
	);
	c->Message(
		Chat::White, ">> NoDrop: %u, NoRent: %u, NoPet: %u, NoTransfer: %u, FVNoDrop: %u",
		item->NoDrop, item->NoRent, (uint8) item->NoPet, (uint8) item->NoTransfer, item->FVNoDrop
	);

	if (item->IsClassBook()) {
		c->Message(Chat::White, "*** This item is a Book (filename:'%s') ***", item->Filename);
	}
	else if (item->IsClassBag()) {
		c->Message(Chat::White, "*** This item is a Container (%u slots) ***", item->BagSlots);
	}
	else {
		c->Message(Chat::White, "*** This item is Common ***");
		c->Message(Chat::White, ">> Classes: %u, Races: %u, Slots: %u", item->Classes, item->Races, item->Slots);
		c->Message(
			Chat::White,
			">> ReqSkill: %u, ReqLevel: %u, RecLevel: %u",
			item->RecSkill,
			item->ReqLevel,
			item->RecLevel
		);
		c->Message(Chat::White, ">> SkillModType: %u, SkillModValue: %i", item->SkillModType, item->SkillModValue);
		c->Message(
			Chat::White, ">> BaneRaceType: %u, BaneRaceDamage: %u, BaneBodyType: %u, BaneBodyDamage: %i",
			item->BaneDmgRace, item->BaneDmgRaceAmt, item->BaneDmgBody, item->BaneDmgAmt
		);
		c->Message(
			Chat::White,
			">> Magic: %s, SpellID: %i, ProcLevel: %u, Charges: %u, MaxCharges: %u",
			(item->Magic ? "TRUE" : "FALSE"),
			item->Click.Effect,
			item->Click.Level,
			inst->GetCharges(),
			item->MaxCharges
		);
		c->Message(
			Chat::White,
			">> EffectType: 0x%02X, CastTime: %.2f",
			(uint8) item->Click.Type,
			((double) item->CastTime / 1000));
	}

	if (c->Admin() >= AccountStatus::GMMgmt) {
		c->Message(Chat::White, ">> MinStatus: %u", item->MinStatus);
	}
}

