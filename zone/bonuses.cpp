/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2004 EQEMu Development Team (http://eqemu.org)

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
#include "../common/classes.h"
#include "../common/global_define.h"
#include "../common/item_instance.h"
#include "../common/rulesys.h"
#include "../common/skills.h"
#include "../common/spdat.h"

#include "client.h"
#include "entity.h"
#include "mob.h"

#ifdef BOTS
#include "bot.h"
#endif

#include "quest_parser_collection.h"


#ifndef WIN32
#include <stdlib.h>
#include "../common/unix.h"
#endif


void Mob::CalcBonuses()
{
	CalcSpellBonuses(&spellbonuses);
	CalcAABonuses(&aabonuses);
	CalcMaxHP();
	CalcMaxMana();
	SetAttackTimer();
	CalcAC();

	rooted = FindType(SE_Root);
}

void NPC::CalcBonuses()
{
	memset(&itembonuses, 0, sizeof(StatBonuses));
	if(RuleB(NPC, UseItemBonusesForNonPets)){
		memset(&itembonuses, 0, sizeof(StatBonuses));
		CalcItemBonuses(&itembonuses);
	}
	else{
		if(GetOwner()){
			memset(&itembonuses, 0, sizeof(StatBonuses));
			CalcItemBonuses(&itembonuses);
		}
	}

	// This has to happen last, so we actually take the item bonuses into account.
	Mob::CalcBonuses();
}

void Client::CalcBonuses()
{
	memset(&itembonuses, 0, sizeof(StatBonuses));
	CalcItemBonuses(&itembonuses);
	CalcEdibleBonuses(&itembonuses);
	CalcSpellBonuses(&spellbonuses);
	CalcAABonuses(&aabonuses);

	ProcessItemCaps(); // caps that depend on spell/aa bonuses

	RecalcWeight();

	CalcAC();
	CalcATK();
	CalcHaste();

	CalcSTR();
	CalcSTA();
	CalcDEX();
	CalcAGI();
	CalcINT();
	CalcWIS();
	CalcCHA();

	CalcMR();
	CalcFR();
	CalcDR();
	CalcPR();
	CalcCR();
	CalcCorrup();

	CalcMaxHP();
	CalcMaxMana();
	CalcMaxEndurance();

	SetAttackTimer();

	rooted = FindType(SE_Root);

	XPRate = 100 + spellbonuses.XPRateMod;

	if (GetMaxXTargets() != 5 + aabonuses.extra_xtargets)
		SetMaxXTargets(5 + aabonuses.extra_xtargets);
}

int Client::CalcRecommendedLevelBonus(uint8 level, uint8 reclevel, int basestat)
{
	if( (reclevel > 0) && (level < reclevel) )
	{
		int32 statmod = (level * 10000 / reclevel) * basestat;

		if( statmod < 0 )
		{
			statmod -= 5000;
			return (statmod/10000);
		}
		else
		{
			statmod += 5000;
			return (statmod/10000);
		}
	}

	return 0;
}

void Client::CalcItemBonuses(StatBonuses* newbon) {
	//memset assumed to be done by caller.

	// Clear item faction mods
	ClearItemFactionBonuses();
	SetShieldEquiped(false);
	SetTwoHandBluntEquiped(false);
	SetTwoHanderEquipped(false);

	unsigned int i;
	// Update: MainAmmo should only calc skill mods (TODO: Check for other cases)
	for (i = EQEmu::inventory::slotCharm; i <= EQEmu::inventory::slotAmmo; i++) {
		const EQEmu::ItemInstance* inst = m_inv[i];
		if(inst == 0)
			continue;
		AddItemBonuses(inst, newbon, false, false, 0, (i == EQEmu::inventory::slotAmmo));

		//These are given special flags due to how often they are checked for various spell effects.
		const EQEmu::ItemData *item = inst->GetItem();
		if (i == EQEmu::inventory::slotSecondary && (item && item->ItemType == EQEmu::item::ItemTypeShield))
			SetShieldEquiped(true);
		else if (i == EQEmu::inventory::slotPrimary && (item && item->ItemType == EQEmu::item::ItemType2HBlunt)) {
			SetTwoHandBluntEquiped(true);
			SetTwoHanderEquipped(true);
		}
		else if (i == EQEmu::inventory::slotPrimary && (item && (item->ItemType == EQEmu::item::ItemType2HSlash || item->ItemType == EQEmu::item::ItemType2HPiercing)))
			SetTwoHanderEquipped(true);
	}

	//Power Source Slot
	if (ClientVersion() >= EQEmu::versions::ClientVersion::SoF)
	{
		const EQEmu::ItemInstance* inst = m_inv[EQEmu::inventory::slotPowerSource];
		if(inst)
			AddItemBonuses(inst, newbon);
	}

	//tribute items
	for (i = 0; i < EQEmu::legacy::TRIBUTE_SIZE; i++) {
		const EQEmu::ItemInstance* inst = m_inv[EQEmu::legacy::TRIBUTE_BEGIN + i];
		if(inst == 0)
			continue;
		AddItemBonuses(inst, newbon, false, true);
	}

	//Optional ability to have worn effects calculate as an addititive bonus instead of highest value
	if (RuleI(Spells, AdditiveBonusWornType) && RuleI(Spells, AdditiveBonusWornType) != EQEmu::item::ItemEffectWorn){
		for (i = EQEmu::inventory::slotCharm; i < EQEmu::inventory::slotAmmo; i++) {
			const EQEmu::ItemInstance* inst = m_inv[i];
			if(inst == 0)
				continue;
			AdditiveWornBonuses(inst, newbon);
		}
	}
}

// These item stat caps depend on spells/AAs so we process them after those are processed
void Client::ProcessItemCaps()
{
	itembonuses.HPRegen = std::min(itembonuses.HPRegen, CalcHPRegenCap());
	itembonuses.ManaRegen = std::min(itembonuses.ManaRegen, CalcManaRegenCap());
	itembonuses.EnduranceRegen = std::min(itembonuses.EnduranceRegen, CalcEnduranceRegenCap());

	// The Sleeper Tomb Avatar proc counts towards item ATK
	// The client uses a 100 here, so using a 100 here the client and server will agree
	// For example, if you set the effect to be 200 it will get 100 item ATK and 100 spell ATK
	if (IsValidSpell(2434) && FindBuff(2434)) {
		itembonuses.ATK += 100;
		spellbonuses.ATK -= 100;
	}

	itembonuses.ATK = std::min(itembonuses.ATK, CalcItemATKCap());
}

void Client::AddItemBonuses(const EQEmu::ItemInstance *inst, StatBonuses *newbon, bool isAug, bool isTribute, int rec_override, bool ammo_slot_item)
{
	if (!inst || !inst->IsClassCommon()) {
		return;
	}

	if (inst->GetAugmentType() == 0 && isAug == true) {
		return;
	}

	const EQEmu::ItemData *item = inst->GetItem();

	if (!isTribute && !inst->IsEquipable(GetBaseRace(), GetClass())) {
		if (item->ItemType != EQEmu::item::ItemTypeFood && item->ItemType != EQEmu::item::ItemTypeDrink)
			return;
	}

	if (GetLevel() < inst->GetItemRequiredLevel(true)) {
		return;
	}
	
	// So there isn't a very nice way to get the real rec level from the aug's inst, so we just pass it in, only
	// used for augs
	auto rec_level = isAug ? rec_override : inst->GetItemRecommendedLevel(true);

	if (!ammo_slot_item) {
		if (GetLevel() >= rec_level) {
			newbon->AC += item->AC;
			newbon->HP += item->HP;
			newbon->Mana += item->Mana;
			newbon->Endurance += item->Endur;
			newbon->ATK += item->Attack;
			newbon->STR += (item->AStr + item->HeroicStr);
			newbon->STA += (item->ASta + item->HeroicSta);
			newbon->DEX += (item->ADex + item->HeroicDex);
			newbon->AGI += (item->AAgi + item->HeroicAgi);
			newbon->INT += (item->AInt + item->HeroicInt);
			newbon->WIS += (item->AWis + item->HeroicWis);
			newbon->CHA += (item->ACha + item->HeroicCha);

			newbon->MR += (item->MR + item->HeroicMR);
			newbon->FR += (item->FR + item->HeroicFR);
			newbon->CR += (item->CR + item->HeroicCR);
			newbon->PR += (item->PR + item->HeroicPR);
			newbon->DR += (item->DR + item->HeroicDR);
			newbon->Corrup += (item->SVCorruption + item->HeroicSVCorrup);

			newbon->STRCapMod += item->HeroicStr;
			newbon->STACapMod += item->HeroicSta;
			newbon->DEXCapMod += item->HeroicDex;
			newbon->AGICapMod += item->HeroicAgi;
			newbon->INTCapMod += item->HeroicInt;
			newbon->WISCapMod += item->HeroicWis;
			newbon->CHACapMod += item->HeroicCha;
			newbon->MRCapMod += item->HeroicMR;
			newbon->CRCapMod += item->HeroicFR;
			newbon->FRCapMod += item->HeroicCR;
			newbon->PRCapMod += item->HeroicPR;
			newbon->DRCapMod += item->HeroicDR;
			newbon->CorrupCapMod += item->HeroicSVCorrup;

			newbon->HeroicSTR += item->HeroicStr;
			newbon->HeroicSTA += item->HeroicSta;
			newbon->HeroicDEX += item->HeroicDex;
			newbon->HeroicAGI += item->HeroicAgi;
			newbon->HeroicINT += item->HeroicInt;
			newbon->HeroicWIS += item->HeroicWis;
			newbon->HeroicCHA += item->HeroicCha;
			newbon->HeroicMR += item->HeroicMR;
			newbon->HeroicFR += item->HeroicFR;
			newbon->HeroicCR += item->HeroicCR;
			newbon->HeroicPR += item->HeroicPR;
			newbon->HeroicDR += item->HeroicDR;
			newbon->HeroicCorrup += item->HeroicSVCorrup;

		}
		else {
			int lvl = GetLevel();

			newbon->AC += CalcRecommendedLevelBonus(lvl, rec_level, item->AC);
			newbon->HP += CalcRecommendedLevelBonus(lvl, rec_level, item->HP);
			newbon->Mana += CalcRecommendedLevelBonus(lvl, rec_level, item->Mana);
			newbon->Endurance += CalcRecommendedLevelBonus(lvl, rec_level, item->Endur);
			newbon->ATK += CalcRecommendedLevelBonus(lvl, rec_level, item->Attack);
			newbon->STR += CalcRecommendedLevelBonus(lvl, rec_level, (item->AStr + item->HeroicStr));
			newbon->STA += CalcRecommendedLevelBonus(lvl, rec_level, (item->ASta + item->HeroicSta));
			newbon->DEX += CalcRecommendedLevelBonus(lvl, rec_level, (item->ADex + item->HeroicDex));
			newbon->AGI += CalcRecommendedLevelBonus(lvl, rec_level, (item->AAgi + item->HeroicAgi));
			newbon->INT += CalcRecommendedLevelBonus(lvl, rec_level, (item->AInt + item->HeroicInt));
			newbon->WIS += CalcRecommendedLevelBonus(lvl, rec_level, (item->AWis + item->HeroicWis));
			newbon->CHA += CalcRecommendedLevelBonus(lvl, rec_level, (item->ACha + item->HeroicCha));

			newbon->MR += CalcRecommendedLevelBonus(lvl, rec_level, (item->MR + item->HeroicMR));
			newbon->FR += CalcRecommendedLevelBonus(lvl, rec_level, (item->FR + item->HeroicFR));
			newbon->CR += CalcRecommendedLevelBonus(lvl, rec_level, (item->CR + item->HeroicCR));
			newbon->PR += CalcRecommendedLevelBonus(lvl, rec_level, (item->PR + item->HeroicPR));
			newbon->DR += CalcRecommendedLevelBonus(lvl, rec_level, (item->DR + item->HeroicDR));
			newbon->Corrup +=
				CalcRecommendedLevelBonus(lvl, rec_level, (item->SVCorruption + item->HeroicSVCorrup));

			newbon->STRCapMod += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicStr);
			newbon->STACapMod += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicSta);
			newbon->DEXCapMod += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicDex);
			newbon->AGICapMod += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicAgi);
			newbon->INTCapMod += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicInt);
			newbon->WISCapMod += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicWis);
			newbon->CHACapMod += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicCha);
			newbon->MRCapMod += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicMR);
			newbon->CRCapMod += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicFR);
			newbon->FRCapMod += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicCR);
			newbon->PRCapMod += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicPR);
			newbon->DRCapMod += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicDR);
			newbon->CorrupCapMod += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicSVCorrup);

			newbon->HeroicSTR += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicStr);
			newbon->HeroicSTA += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicSta);
			newbon->HeroicDEX += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicDex);
			newbon->HeroicAGI += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicAgi);
			newbon->HeroicINT += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicInt);
			newbon->HeroicWIS += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicWis);
			newbon->HeroicCHA += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicCha);
			newbon->HeroicMR += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicMR);
			newbon->HeroicFR += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicFR);
			newbon->HeroicCR += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicCR);
			newbon->HeroicPR += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicPR);
			newbon->HeroicDR += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicDR);
			newbon->HeroicCorrup += CalcRecommendedLevelBonus(lvl, rec_level, item->HeroicSVCorrup);
		}

		// FatherNitwit: New style haste, shields, and regens
		if (newbon->haste < (int32)item->Haste) {
			newbon->haste = item->Haste;
		}
		if (item->Regen > 0)
			newbon->HPRegen += item->Regen;

		if (item->ManaRegen > 0)
			newbon->ManaRegen += item->ManaRegen;

		if (item->EnduranceRegen > 0)
			newbon->EnduranceRegen += item->EnduranceRegen;

		if (item->DamageShield > 0) {
			if ((newbon->DamageShield + item->DamageShield) > RuleI(Character, ItemDamageShieldCap))
				newbon->DamageShield = RuleI(Character, ItemDamageShieldCap);
			else
				newbon->DamageShield += item->DamageShield;
		}
		if (item->SpellShield > 0) {
			if ((newbon->SpellShield + item->SpellShield) > RuleI(Character, ItemSpellShieldingCap))
				newbon->SpellShield = RuleI(Character, ItemSpellShieldingCap);
			else
				newbon->SpellShield += item->SpellShield;
		}
		if (item->Shielding > 0) {
			if ((newbon->MeleeMitigation + item->Shielding) > RuleI(Character, ItemShieldingCap))
				newbon->MeleeMitigation = RuleI(Character, ItemShieldingCap);
			else
				newbon->MeleeMitigation += item->Shielding;
		}
		if (item->StunResist > 0) {
			if ((newbon->StunResist + item->StunResist) > RuleI(Character, ItemStunResistCap))
				newbon->StunResist = RuleI(Character, ItemStunResistCap);
			else
				newbon->StunResist += item->StunResist;
		}
		if (item->StrikeThrough > 0) {
			if ((newbon->StrikeThrough + item->StrikeThrough) > RuleI(Character, ItemStrikethroughCap))
				newbon->StrikeThrough = RuleI(Character, ItemStrikethroughCap);
			else
				newbon->StrikeThrough += item->StrikeThrough;
		}
		if (item->Avoidance > 0) {
			if ((newbon->AvoidMeleeChance + item->Avoidance) > RuleI(Character, ItemAvoidanceCap))
				newbon->AvoidMeleeChance = RuleI(Character, ItemAvoidanceCap);
			else
				newbon->AvoidMeleeChance += item->Avoidance;
		}
		if (item->Accuracy > 0) {
			if ((newbon->HitChance + item->Accuracy) > RuleI(Character, ItemAccuracyCap))
				newbon->HitChance = RuleI(Character, ItemAccuracyCap);
			else
				newbon->HitChance += item->Accuracy;
		}
		if (item->CombatEffects > 0) {
			if ((newbon->ProcChance + item->CombatEffects) > RuleI(Character, ItemCombatEffectsCap))
				newbon->ProcChance = RuleI(Character, ItemCombatEffectsCap);
			else
				newbon->ProcChance += item->CombatEffects;
		}
		if (item->DotShielding > 0) {
			if ((newbon->DoTShielding + item->DotShielding) > RuleI(Character, ItemDoTShieldingCap))
				newbon->DoTShielding = RuleI(Character, ItemDoTShieldingCap);
			else
				newbon->DoTShielding += item->DotShielding;
		}

		if (item->HealAmt > 0) {
			if ((newbon->HealAmt + item->HealAmt) > RuleI(Character, ItemHealAmtCap))
				newbon->HealAmt = RuleI(Character, ItemHealAmtCap);
			else
				newbon->HealAmt += item->HealAmt;
		}
		if (item->SpellDmg > 0) {
			if ((newbon->SpellDmg + item->SpellDmg) > RuleI(Character, ItemSpellDmgCap))
				newbon->SpellDmg = RuleI(Character, ItemSpellDmgCap);
			else
				newbon->SpellDmg += item->SpellDmg;
		}
		if (item->Clairvoyance > 0) {
			if ((newbon->Clairvoyance + item->Clairvoyance) > RuleI(Character, ItemClairvoyanceCap))
				newbon->Clairvoyance = RuleI(Character, ItemClairvoyanceCap);
			else
				newbon->Clairvoyance += item->Clairvoyance;
		}

		if (item->DSMitigation > 0) {
			if ((newbon->DSMitigation + item->DSMitigation) > RuleI(Character, ItemDSMitigationCap))
				newbon->DSMitigation = RuleI(Character, ItemDSMitigationCap);
			else
				newbon->DSMitigation += item->DSMitigation;
		}
		if (item->Worn.Effect > 0 && item->Worn.Type == EQEmu::item::ItemEffectWorn) { // latent effects
			ApplySpellsBonuses(item->Worn.Effect, item->Worn.Level, newbon, 0, item->Worn.Type);
		}

		if (item->Focus.Effect > 0 && (item->Focus.Type == EQEmu::item::ItemEffectFocus)) { // focus effects
			ApplySpellsBonuses(item->Focus.Effect, item->Focus.Level, newbon, 0);
		}

		switch (item->BardType) {
		case 51: /* All (e.g. Singing Short Sword) */
			if (item->BardValue > newbon->singingMod)
				newbon->singingMod = item->BardValue;
			if (item->BardValue > newbon->brassMod)
				newbon->brassMod = item->BardValue;
			if (item->BardValue > newbon->stringedMod)
				newbon->stringedMod = item->BardValue;
			if (item->BardValue > newbon->percussionMod)
				newbon->percussionMod = item->BardValue;
			if (item->BardValue > newbon->windMod)
				newbon->windMod = item->BardValue;
			break;
		case 50: /* Singing */
			if (item->BardValue > newbon->singingMod)
				newbon->singingMod = item->BardValue;
			break;
		case 23: /* Wind */
			if (item->BardValue > newbon->windMod)
				newbon->windMod = item->BardValue;
			break;
		case 24: /* stringed */
			if (item->BardValue > newbon->stringedMod)
				newbon->stringedMod = item->BardValue;
			break;
		case 25: /* brass */
			if (item->BardValue > newbon->brassMod)
				newbon->brassMod = item->BardValue;
			break;
		case 26: /* Percussion */
			if (item->BardValue > newbon->percussionMod)
				newbon->percussionMod = item->BardValue;
			break;
		}
	
		// Add Item Faction Mods
		if (item->FactionMod1) {
			if (item->FactionAmt1 > 0 && item->FactionAmt1 > GetItemFactionBonus(item->FactionMod1)) {
				AddItemFactionBonus(item->FactionMod1, item->FactionAmt1);
			}
			else if (item->FactionAmt1 < 0 && item->FactionAmt1 < GetItemFactionBonus(item->FactionMod1)) {
				AddItemFactionBonus(item->FactionMod1, item->FactionAmt1);
			}
		}
		if (item->FactionMod2) {
			if (item->FactionAmt2 > 0 && item->FactionAmt2 > GetItemFactionBonus(item->FactionMod2)) {
				AddItemFactionBonus(item->FactionMod2, item->FactionAmt2);
			}
			else if (item->FactionAmt2 < 0 && item->FactionAmt2 < GetItemFactionBonus(item->FactionMod2)) {
				AddItemFactionBonus(item->FactionMod2, item->FactionAmt2);
			}
		}
		if (item->FactionMod3) {
			if (item->FactionAmt3 > 0 && item->FactionAmt3 > GetItemFactionBonus(item->FactionMod3)) {
				AddItemFactionBonus(item->FactionMod3, item->FactionAmt3);
			}
			else if (item->FactionAmt3 < 0 && item->FactionAmt3 < GetItemFactionBonus(item->FactionMod3)) {
				AddItemFactionBonus(item->FactionMod3, item->FactionAmt3);
			}
		}
		if (item->FactionMod4) {
			if (item->FactionAmt4 > 0 && item->FactionAmt4 > GetItemFactionBonus(item->FactionMod4)) {
				AddItemFactionBonus(item->FactionMod4, item->FactionAmt4);
			}
			else if (item->FactionAmt4 < 0 && item->FactionAmt4 < GetItemFactionBonus(item->FactionMod4)) {
				AddItemFactionBonus(item->FactionMod4, item->FactionAmt4);
			}
		}

		if (item->ExtraDmgSkill != 0 && item->ExtraDmgSkill <= EQEmu::skills::HIGHEST_SKILL) {
			if ((newbon->SkillDamageAmount[item->ExtraDmgSkill] + item->ExtraDmgAmt) >
				RuleI(Character, ItemExtraDmgCap))
				newbon->SkillDamageAmount[item->ExtraDmgSkill] = RuleI(Character, ItemExtraDmgCap);
			else
				newbon->SkillDamageAmount[item->ExtraDmgSkill] += item->ExtraDmgAmt;
		}
	}

	// Process when ammo_slot_item = true or false
	if (item->SkillModValue != 0 && item->SkillModType <= EQEmu::skills::HIGHEST_SKILL) {
		if ((item->SkillModValue > 0 && newbon->skillmod[item->SkillModType] < item->SkillModValue) ||
			(item->SkillModValue < 0 && newbon->skillmod[item->SkillModType] > item->SkillModValue)) {

			newbon->skillmod[item->SkillModType] = item->SkillModValue;
			newbon->skillmodmax[item->SkillModType] = item->SkillModMax;
		}
	}

	if (!isAug) {
		for (int i = EQEmu::inventory::socketBegin; i < EQEmu::inventory::SocketCount; i++)
			AddItemBonuses(inst->GetAugment(i), newbon, true, false, rec_level, ammo_slot_item);
	}
}

void Client::AdditiveWornBonuses(const EQEmu::ItemInstance *inst, StatBonuses* newbon, bool isAug) {

	/*
	Powerful Non-live like option allows developers to add worn effects on items that
	can stack with other worn effects of the same spell effect type, instead of only taking the highest value.
	Ie Cleave I = 40 pct cleave - So if you equip 3 cleave I items you will have a 120 pct cleave bonus.
	To enable use RuleI(Spells, AdditiveBonusWornType)
	Setting value =  2  Will force all live items to automatically be calculated additivily
	Setting value to anything else will indicate the item 'worntype' that if set to the same, will cause the bonuses to use this calculation
	which will also stack with regular (worntype 2) effects. [Ie set rule = 3 and item worntype = 3]
	*/

	if (!inst || !inst->IsClassCommon())
		return;

	if(inst->GetAugmentType()==0 && isAug == true)
		return;

	const EQEmu::ItemData *item = inst->GetItem();

	if(!inst->IsEquipable(GetBaseRace(),GetClass()))
		return;

	if(GetLevel() < item->ReqLevel)
		return;

	if (item->Worn.Effect > 0 && item->Worn.Type == RuleI(Spells, AdditiveBonusWornType))
		ApplySpellsBonuses(item->Worn.Effect, item->Worn.Level, newbon, 0, item->Worn.Type);// Non-live like - Addititive latent effects


	if (!isAug)
	{
		int i;
		for (i = EQEmu::inventory::socketBegin; i < EQEmu::inventory::SocketCount; i++) {
			AdditiveWornBonuses(inst->GetAugment(i),newbon,true);
		}
	}
}

void Client::CalcEdibleBonuses(StatBonuses* newbon) {
	uint32 i;

	bool food = false;
	bool drink = false;
	for (i = EQEmu::legacy::GENERAL_BEGIN; i <= EQEmu::legacy::GENERAL_BAGS_BEGIN; i++)
	{
		if (food && drink)
			break;
		const EQEmu::ItemInstance* inst = GetInv().GetItem(i);
		if (inst && inst->GetItem() && inst->IsClassCommon()) {
			const EQEmu::ItemData *item = inst->GetItem();
			if (item->ItemType == EQEmu::item::ItemTypeFood && !food)
				food = true;
			else if (item->ItemType == EQEmu::item::ItemTypeDrink && !drink)
				drink = true;
			else
				continue;
			AddItemBonuses(inst, newbon);
		}
	}
	for (i = EQEmu::legacy::GENERAL_BAGS_BEGIN; i <= EQEmu::legacy::GENERAL_BAGS_END; i++)
	{
		if (food && drink)
			break;
		const EQEmu::ItemInstance* inst = GetInv().GetItem(i);
		if (inst && inst->GetItem() && inst->IsClassCommon()) {
			const EQEmu::ItemData *item = inst->GetItem();
			if (item->ItemType == EQEmu::item::ItemTypeFood && !food)
				food = true;
			else if (item->ItemType == EQEmu::item::ItemTypeDrink && !drink)
				drink = true;
			else
				continue;
			AddItemBonuses(inst, newbon);
		}
	}
}

void Mob::CalcAABonuses(StatBonuses *newbon)
{
	memset(newbon, 0, sizeof(StatBonuses)); // start fresh

	for (const auto &aa : aa_ranks) {
		auto ability_rank = zone->GetAlternateAdvancementAbilityAndRank(aa.first, aa.second.first);
		auto ability = ability_rank.first;
		auto rank = ability_rank.second;

		if(!ability) {
			continue;
		}

		// bad data or no effects
		if (rank->effects.empty())
			continue;

		ApplyAABonuses(*rank, newbon);
	}
}

//A lot of the normal spell functions (IsBlankSpellEffect, etc) are set for just spells (in common/spdat.h).
//For now, we'll just put them directly into the code and comment with the corresponding normal function
//Maybe we'll fix it later? :-D
void Mob::ApplyAABonuses(const AA::Rank &rank, StatBonuses *newbon)
{
	if (rank.effects.empty()) // sanity check. why bother if no slots to fill?
		return;

	uint32 effect = 0;
	int32 base1 = 0;
	int32 base2 = 0; // only really used for SE_RaiseStatCap & SE_ReduceSkillTimer in aa_effects table
	uint32 slot = 0;

	for (const auto &e : rank.effects) {
		effect = e.effect_id;
		base1 = e.base1;
		base2 = e.base2;
		slot = e.slot;

		// we default to 0 (SE_CurrentHP) for the effect, so if there aren't any base1/2 values, we'll just skip it
		if (effect == 0 && base1 == 0 && base2 == 0)
			continue;

		// IsBlankSpellEffect()
		if (effect == SE_Blank || (effect == SE_CHA && base1 == 0) || effect == SE_StackingCommand_Block ||
		    effect == SE_StackingCommand_Overwrite)
			continue;

		Log.Out(Logs::Detail, Logs::AA, "Applying Effect %d from AA %u in slot %d (base1: %d, base2: %d) on %s",
			effect, rank.id, slot, base1, base2, GetCleanName());

		uint8 focus = IsFocusEffect(0, 0, true, effect);
		if (focus) {
			newbon->FocusEffects[focus] = static_cast<uint8>(effect);
			continue;
		}

		switch (effect) {
		case SE_ACv2:
		case SE_ArmorClass:
			newbon->AC += base1;
			break;
		// Note: AA effects that use accuracy are skill limited, while spell effect is not.
		case SE_Accuracy:
			// Bad data or unsupported new skill
			if (base2 > EQEmu::skills::HIGHEST_SKILL)
				break;
			if ((base2 == ALL_SKILLS) && (newbon->Accuracy[EQEmu::skills::HIGHEST_SKILL + 1] < base1))
				newbon->Accuracy[EQEmu::skills::HIGHEST_SKILL + 1] = base1;
			else if (newbon->Accuracy[base2] < base1)
				newbon->Accuracy[base2] += base1;
			break;
		case SE_CurrentHP: // regens
			newbon->HPRegen += base1;
			break;
		case SE_CurrentEndurance:
			newbon->EnduranceRegen += base1;
			break;
		case SE_MovementSpeed:
			newbon->movementspeed += base1; // should we let these stack?
			/*if (base1 > newbon->movementspeed)	//or should we use a total value?
				newbon->movementspeed = base1;*/
			break;
		case SE_STR:
			newbon->STR += base1;
			break;
		case SE_DEX:
			newbon->DEX += base1;
			break;
		case SE_AGI:
			newbon->AGI += base1;
			break;
		case SE_STA:
			newbon->STA += base1;
			break;
		case SE_INT:
			newbon->INT += base1;
			break;
		case SE_WIS:
			newbon->WIS += base1;
			break;
		case SE_CHA:
			newbon->CHA += base1;
			break;
		case SE_WaterBreathing:
			// handled by client
			break;
		case SE_CurrentMana:
			newbon->ManaRegen += base1;
			break;
		case SE_ManaPool:
			newbon->Mana += base1;
			break;
		case SE_ItemManaRegenCapIncrease:
			newbon->ItemManaRegenCap += base1;
			break;
		case SE_ResistFire:
			newbon->FR += base1;
			break;
		case SE_ResistCold:
			newbon->CR += base1;
			break;
		case SE_ResistPoison:
			newbon->PR += base1;
			break;
		case SE_ResistDisease:
			newbon->DR += base1;
			break;
		case SE_ResistMagic:
			newbon->MR += base1;
			break;
		case SE_ResistCorruption:
			newbon->Corrup += base1;
			break;
		case SE_IncreaseSpellHaste:
			break;
		case SE_IncreaseRange:
			break;
		case SE_MaxHPChange:
			newbon->MaxHP += base1;
			break;
		case SE_Packrat:
			newbon->Packrat += base1;
			break;
		case SE_TwoHandBash:
			break;
		case SE_SetBreathLevel:
			break;
		case SE_RaiseStatCap:
			switch (base2) {
			// are these #define'd somewhere?
			case 0: // str
				newbon->STRCapMod += base1;
				break;
			case 1: // sta
				newbon->STACapMod += base1;
				break;
			case 2: // agi
				newbon->AGICapMod += base1;
				break;
			case 3: // dex
				newbon->DEXCapMod += base1;
				break;
			case 4: // wis
				newbon->WISCapMod += base1;
				break;
			case 5: // int
				newbon->INTCapMod += base1;
				break;
			case 6: // cha
				newbon->CHACapMod += base1;
				break;
			case 7: // mr
				newbon->MRCapMod += base1;
				break;
			case 8: // cr
				newbon->CRCapMod += base1;
				break;
			case 9: // fr
				newbon->FRCapMod += base1;
				break;
			case 10: // pr
				newbon->PRCapMod += base1;
				break;
			case 11: // dr
				newbon->DRCapMod += base1;
				break;
			case 12: // corruption
				newbon->CorrupCapMod += base1;
				break;
			}
			break;
		case SE_SpellSlotIncrease:
			break;
		case SE_MysticalAttune:
			newbon->BuffSlotIncrease += base1;
			break;
		case SE_TotalHP:
			newbon->HP += base1;
			break;
		case SE_StunResist:
			newbon->StunResist += base1;
			break;
		case SE_SpellCritChance:
			newbon->CriticalSpellChance += base1;
			break;
		case SE_SpellCritDmgIncrease:
			newbon->SpellCritDmgIncrease += base1;
			break;
		case SE_DotCritDmgIncrease:
			newbon->DotCritDmgIncrease += base1;
			break;
		case SE_ResistSpellChance:
			newbon->ResistSpellChance += base1;
			break;
		case SE_CriticalHealChance:
			newbon->CriticalHealChance += base1;
			break;
		case SE_CriticalHealOverTime:
			newbon->CriticalHealOverTime += base1;
			break;
		case SE_CriticalDoTChance:
			newbon->CriticalDoTChance += base1;
			break;
		case SE_ReduceSkillTimer:
			newbon->SkillReuseTime[base2] += base1;
			break;
		case SE_Fearless:
			newbon->Fearless = true;
			break;
		case SE_PersistantCasting:
			newbon->PersistantCasting += base1;
			break;
		case SE_DelayDeath:
			newbon->DelayDeath += base1;
			break;
		case SE_FrontalStunResist:
			newbon->FrontalStunResist += base1;
			break;
		case SE_ImprovedBindWound:
			newbon->BindWound += base1;
			break;
		case SE_MaxBindWound:
			newbon->MaxBindWound += base1;
			break;
		case SE_ExtraAttackChance:
			newbon->ExtraAttackChance += base1;
			break;
		case SE_SeeInvis:
			newbon->SeeInvis = base1;
			break;
		case SE_BaseMovementSpeed:
			newbon->BaseMovementSpeed += base1;
			break;
		case SE_IncreaseRunSpeedCap:
			newbon->IncreaseRunSpeedCap += base1;
			break;
		case SE_ConsumeProjectile:
			newbon->ConsumeProjectile += base1;
			break;
		case SE_ForageAdditionalItems:
			newbon->ForageAdditionalItems += base1;
			break;
		case SE_Salvage:
			newbon->SalvageChance += base1;
			break;
		case SE_ArcheryDamageModifier:
			newbon->ArcheryDamageModifier += base1;
			break;
		case SE_DoubleRangedAttack:
			newbon->DoubleRangedAttack += base1;
			break;
		case SE_DamageShield:
			newbon->DamageShield += base1;
			break;
		case SE_CharmBreakChance:
			newbon->CharmBreakChance += base1;
			break;
		case SE_OffhandRiposteFail:
			newbon->OffhandRiposteFail += base1;
			break;
		case SE_ItemAttackCapIncrease:
			newbon->ItemATKCap += base1;
			break;
		case SE_GivePetGroupTarget:
			newbon->GivePetGroupTarget = true;
			break;
		case SE_ItemHPRegenCapIncrease:
			newbon->ItemHPRegenCap = +base1;
			break;
		case SE_Ambidexterity:
			newbon->Ambidexterity += base1;
			break;
		case SE_PetMaxHP:
			newbon->PetMaxHP += base1;
			break;
		case SE_AvoidMeleeChance:
			newbon->AvoidMeleeChanceEffect += base1;
			break;
		case SE_CombatStability:
			newbon->CombatStability += base1;
			break;
		case SE_AddSingingMod:
			switch (base2) {
			case EQEmu::item::ItemTypeWindInstrument:
				newbon->windMod += base1;
				break;
			case EQEmu::item::ItemTypeStringedInstrument:
				newbon->stringedMod += base1;
				break;
			case EQEmu::item::ItemTypeBrassInstrument:
				newbon->brassMod += base1;
				break;
			case EQEmu::item::ItemTypePercussionInstrument:
				newbon->percussionMod += base1;
				break;
			case EQEmu::item::ItemTypeSinging:
				newbon->singingMod += base1;
				break;
			}
			break;
		case SE_SongModCap:
			newbon->songModCap += base1;
			break;
		case SE_PetCriticalHit:
			newbon->PetCriticalHit += base1;
			break;
		case SE_PetAvoidance:
			newbon->PetAvoidance += base1;
			break;
		case SE_ShieldBlock:
			newbon->ShieldBlock += base1;
			break;
		case SE_ShieldEquipDmgMod:
			newbon->ShieldEquipDmgMod += base1;
			break;
		case SE_SecondaryDmgInc:
			newbon->SecondaryDmgInc = true;
			break;
		case SE_ChangeAggro:
			newbon->hatemod += base1;
			break;
		case SE_EndurancePool:
			newbon->Endurance += base1;
			break;
		case SE_ChannelChanceItems:
			newbon->ChannelChanceItems += base1;
			break;
		case SE_ChannelChanceSpells:
			newbon->ChannelChanceSpells += base1;
			break;
		case SE_DoubleSpecialAttack:
			newbon->DoubleSpecialAttack += base1;
			break;
		case SE_TripleBackstab:
			newbon->TripleBackstab += base1;
			break;
		case SE_FrontalBackstabMinDmg:
			newbon->FrontalBackstabMinDmg = true;
			break;
		case SE_FrontalBackstabChance:
			newbon->FrontalBackstabChance += base1;
			break;
		case SE_BlockBehind:
			newbon->BlockBehind += base1;
			break;

		case SE_StrikeThrough:
		case SE_StrikeThrough2:
			newbon->StrikeThrough += base1;
			break;
		case SE_DoubleAttackChance:
			newbon->DoubleAttackChance += base1;
			break;
		case SE_GiveDoubleAttack:
			newbon->GiveDoubleAttack += base1;
			break;
		case SE_ProcChance:
			newbon->ProcChanceSPA += base1;
			break;
		case SE_RiposteChance:
			newbon->RiposteChance += base1;
			break;
		case SE_Flurry:
			newbon->FlurryChance += base1;
			break;
		case SE_PetFlurry:
			newbon->PetFlurry += base1;
			break;
		case SE_BardSongRange:
			newbon->SongRange += base1;
			break;
		case SE_RootBreakChance:
			newbon->RootBreakChance += base1;
			break;
		case SE_UnfailingDivinity:
			newbon->UnfailingDivinity += base1;
			break;
		case SE_CrippBlowChance:
			newbon->CrippBlowChance += base1;
			break;

		case SE_HitChance: {
			// Bad data or unsupported new skill
			if (base2 > EQEmu::skills::HIGHEST_SKILL)
				break;
			if (base2 == ALL_SKILLS)
				newbon->HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1] += base1;
			else
				newbon->HitChanceEffect[base2] += base1;
		}

		case SE_ProcOnKillShot:
			for (int i = 0; i < MAX_SPELL_TRIGGER * 3; i += 3) {
				if (!newbon->SpellOnKill[i] ||
				    ((newbon->SpellOnKill[i] == base2) && (newbon->SpellOnKill[i + 1] < base1))) {
					// base1 = chance, base2 = SpellID to be triggered, base3 = min npc level
					newbon->SpellOnKill[i] = base2;
					newbon->SpellOnKill[i + 1] = base1;

					if (GetLevel() > 15)
						newbon->SpellOnKill[i + 2] =
						    GetLevel() - 15; // AA specifiy "non-trivial"
					else
						newbon->SpellOnKill[i + 2] = 0;

					break;
				}
			}
			break;

		case SE_SpellOnDeath:
			for (int i = 0; i < MAX_SPELL_TRIGGER * 2; i += 2) {
				if (!newbon->SpellOnDeath[i]) {
					// base1 = SpellID to be triggered, base2 = chance to fire
					newbon->SpellOnDeath[i] = base1;
					newbon->SpellOnDeath[i + 1] = base2;
					break;
				}
			}
			break;

		case SE_TriggerOnCast:

			for (int i = 0; i < MAX_SPELL_TRIGGER; i++) {
				if (newbon->SpellTriggers[i] == rank.id)
					break;

				if (!newbon->SpellTriggers[i]) {
					// Save the 'rank.id' of each triggerable effect to an array
					newbon->SpellTriggers[i] = rank.id;
					break;
				}
			}
			break;

		case SE_CriticalHitChance: {
			// Bad data or unsupported new skill
			if (base2 > EQEmu::skills::HIGHEST_SKILL)
				break;
			if (base2 == ALL_SKILLS)
				newbon->CriticalHitChance[EQEmu::skills::HIGHEST_SKILL + 1] += base1;
			else
				newbon->CriticalHitChance[base2] += base1;
		} break;

		case SE_CriticalDamageMob: {
			// Bad data or unsupported new skill
			if (base2 > EQEmu::skills::HIGHEST_SKILL)
				break;
			// base1 = effect value, base2 = skill restrictions(-1 for all)
			if (base2 == ALL_SKILLS)
				newbon->CritDmgMob[EQEmu::skills::HIGHEST_SKILL + 1] += base1;
			else
				newbon->CritDmgMob[base2] += base1;
			break;
		}

		case SE_CriticalSpellChance: {
			newbon->CriticalSpellChance += base1;

			if (base2 > newbon->SpellCritDmgIncNoStack)
				newbon->SpellCritDmgIncNoStack = base2;

			break;
		}

		case SE_ResistFearChance: {
			if (base1 == 100) // If we reach 100% in a single spell/item then we should be immune to
					  // negative fear resist effects until our immunity is over
				newbon->Fearless = true;

			newbon->ResistFearChance += base1; // these should stack
			break;
		}

		case SE_SkillDamageAmount: {
			// Bad data or unsupported new skill
			if (base2 > EQEmu::skills::HIGHEST_SKILL)
				break;
			if (base2 == ALL_SKILLS)
				newbon->SkillDamageAmount[EQEmu::skills::HIGHEST_SKILL + 1] += base1;
			else
				newbon->SkillDamageAmount[base2] += base1;
			break;
		}

		case SE_SkillAttackProc: {
			// You can only have one of these per client. [AA Dragon Punch]
			newbon->SkillAttackProc[0] = base1; // Chance base 1000 = 100% proc rate
			newbon->SkillAttackProc[1] = base2; // Skill to Proc Off
			newbon->SkillAttackProc[2] = rank.spell; // spell to proc
			break;
		}

		case SE_DamageModifier: {
			// Bad data or unsupported new skill
			if (base2 > EQEmu::skills::HIGHEST_SKILL)
				break;
			if (base2 == ALL_SKILLS)
				newbon->DamageModifier[EQEmu::skills::HIGHEST_SKILL + 1] += base1;
			else
				newbon->DamageModifier[base2] += base1;
			break;
		}

		case SE_DamageModifier2: {
			// Bad data or unsupported new skill
			if (base2 > EQEmu::skills::HIGHEST_SKILL)
				break;
			if (base2 == ALL_SKILLS)
				newbon->DamageModifier2[EQEmu::skills::HIGHEST_SKILL + 1] += base1;
			else
				newbon->DamageModifier2[base2] += base1;
			break;
		}

		case SE_SlayUndead: {
			if (newbon->SlayUndead[1] < base1)
				newbon->SlayUndead[0] = base1; // Rate
			newbon->SlayUndead[1] = base2;	 // Damage Modifier
			break;
		}

		case SE_DoubleRiposte: {
			newbon->DoubleRiposte += base1;
			break;
		}

		case SE_GiveDoubleRiposte: {
			// 0=Regular Riposte 1=Skill Attack Riposte 2=Skill
			if (base2 == 0) {
				if (newbon->GiveDoubleRiposte[0] < base1)
					newbon->GiveDoubleRiposte[0] = base1;
			}
			// Only for special attacks.
			else if (base2 > 0 && (newbon->GiveDoubleRiposte[1] < base1)) {
				newbon->GiveDoubleRiposte[1] = base1;
				newbon->GiveDoubleRiposte[2] = base2;
			}

			break;
		}

		// Physically raises skill cap ie if 55/55 it will raise to 55/60
		case SE_RaiseSkillCap: {

			if (base2 > EQEmu::skills::HIGHEST_SKILL)
				break;

			if (newbon->RaiseSkillCap[base2] < base1) 
				newbon->RaiseSkillCap[base2] = base1;
			break;
		}

		case SE_MasteryofPast: {
			if (newbon->MasteryofPast < base1)
				newbon->MasteryofPast = base1;
			break;
		}

		case SE_CastingLevel2:
		case SE_CastingLevel: {
			newbon->effective_casting_level += base1;
			break;
		}

		case SE_DivineSave: {
			if (newbon->DivineSaveChance[0] < base1) {
				newbon->DivineSaveChance[0] = base1;
				newbon->DivineSaveChance[1] = base2;
			}
			break;
		}

		case SE_SpellEffectResistChance: {
			for (int e = 0; e < MAX_RESISTABLE_EFFECTS * 2; e += 2) {
				if (newbon->SEResist[e + 1] && (newbon->SEResist[e] == base2) &&
				    (newbon->SEResist[e + 1] < base1)) {
					newbon->SEResist[e] = base2; // Spell Effect ID
					newbon->SEResist[e + 1] = base1; // Resist Chance
					break;
				} else if (!newbon->SEResist[e + 1]) {
					newbon->SEResist[e] = base2; // Spell Effect ID
					newbon->SEResist[e + 1] = base1; // Resist Chance
					break;
				}
			}
			break;
		}

		case SE_MitigateDamageShield: {
			if (base1 < 0)
				base1 = base1 * (-1);

			newbon->DSMitigationOffHand += base1;
			break;
		}

		case SE_FinishingBlow: {
			// base1 = chance, base2 = damage
			if (newbon->FinishingBlow[1] < base2) {
				newbon->FinishingBlow[0] = base1;
				newbon->FinishingBlow[1] = base2;
			}
			break;
		}

		case SE_FinishingBlowLvl: {
			// base1 = level, base2 = ??? (Set to 200 in AA data, possible proc rate mod?)
			if (newbon->FinishingBlowLvl[0] < base1) {
				newbon->FinishingBlowLvl[0] = base1;
				newbon->FinishingBlowLvl[1] = base2;
			}
			break;
		}

		case SE_StunBashChance:
			newbon->StunBashChance += base1;
			break;

		case SE_IncreaseChanceMemwipe:
			newbon->IncreaseChanceMemwipe += base1;
			break;

		case SE_CriticalMend:
			newbon->CriticalMend += base1;
			break;

		case SE_HealRate:
			newbon->HealRate += base1;
			break;

		case SE_MeleeLifetap: {

			if ((base1 < 0) && (newbon->MeleeLifetap > base1))
				newbon->MeleeLifetap = base1;

			else if (newbon->MeleeLifetap < base1)
				newbon->MeleeLifetap = base1;
			break;
		}

		case SE_Vampirism:
			newbon->Vampirism += base1;
			break;

		case SE_FrenziedDevastation:
			newbon->FrenziedDevastation += base2;
			break;

		case SE_SpellProcChance:
			newbon->SpellProcChance += base1;
			break;

		case SE_Berserk:
			newbon->BerserkSPA = true;
			break;

		case SE_Metabolism:
			newbon->Metabolism += base1;
			break;

		case SE_ImprovedReclaimEnergy: {
			if ((base1 < 0) && (newbon->ImprovedReclaimEnergy > base1))
				newbon->ImprovedReclaimEnergy = base1;

			else if (newbon->ImprovedReclaimEnergy < base1)
				newbon->ImprovedReclaimEnergy = base1;
			break;
		}

		case SE_HeadShot: {
			if (newbon->HeadShot[1] < base2) {
				newbon->HeadShot[0] = base1;
				newbon->HeadShot[1] = base2;
			}
			break;
		}

		case SE_HeadShotLevel: {
			if (newbon->HSLevel[0] < base1)
				newbon->HSLevel[0] = base1;
				newbon->HSLevel[1] = base2;
			break;
		}

		case SE_Assassinate: {
			if (newbon->Assassinate[1] < base2) {
				newbon->Assassinate[0] = base1;
				newbon->Assassinate[1] = base2;
			}
			break;
		}

		case SE_AssassinateLevel: {
			if (newbon->AssassinateLevel[0] < base1) {
				newbon->AssassinateLevel[0] = base1;
				newbon->AssassinateLevel[1] = base2;
			}
			break;
		}

		case SE_PetMeleeMitigation:
			newbon->PetMeleeMitigation += base1;
			break;

		case SE_FactionModPct: {
			if ((base1 < 0) && (newbon->FactionModPct > base1))
				newbon->FactionModPct = base1;

			else if (newbon->FactionModPct < base1)
				newbon->FactionModPct = base1;
			break;
		}

		case SE_IllusionPersistence:
			newbon->IllusionPersistence = true;
			break;

		case SE_LimitToSkill: {
			// Bad data or unsupported new skill
			if (base2 > EQEmu::skills::HIGHEST_SKILL)
				break;
			if (base1 <= EQEmu::skills::HIGHEST_SKILL)
				newbon->LimitToSkill[base1] = true;
			break;
		}

		case SE_SkillProc: {
			for (int e = 0; e < MAX_SKILL_PROCS; e++) {
				if (newbon->SkillProc[e] && newbon->SkillProc[e] == rank.id)
					break; // Do not use the same aa id more than once.

				else if (!newbon->SkillProc[e]) {
					newbon->SkillProc[e] = rank.id;
					break;
				}
			}
			break;
		}

		case SE_SkillProcSuccess: {

			for (int e = 0; e < MAX_SKILL_PROCS; e++) {
				if (newbon->SkillProcSuccess[e] && newbon->SkillProcSuccess[e] == rank.id)
					break; // Do not use the same spell id more than once.

				else if (!newbon->SkillProcSuccess[e]) {
					newbon->SkillProcSuccess[e] = rank.id;
					break;
				}
			}
			break;
		}

		case SE_MeleeMitigation:
			newbon->MeleeMitigationEffect += base1;
			break;

		case SE_ATK:
			newbon->ATK += base1;
			break;
		case SE_IncreaseExtTargetWindow:
			newbon->extra_xtargets += base1;
			break;

		case SE_PC_Pet_Rampage: {
			newbon->PC_Pet_Rampage[0] += base1; //Chance to rampage
			if (newbon->PC_Pet_Rampage[1] < base2)
				newbon->PC_Pet_Rampage[1] = base2; //Damage modifer - take highest
			break;
		}

		case SE_PC_Pet_Flurry_Chance: 
			newbon->PC_Pet_Flurry += base1; //Chance to Flurry
			break;

		case SE_ShroudofStealth:
			newbon->ShroudofStealth = true;
			break;

		case SE_ReduceFallDamage:
			newbon->ReduceFallDamage += base1;
			break;

		case SE_ReduceTradeskillFail:{

			if (base2 > EQEmu::skills::HIGHEST_SKILL)
				break;

			newbon->ReduceTradeskillFail[base2] += base1;
			break;
		}

		case SE_TradeSkillMastery:
			if (newbon->TradeSkillMastery < base1)
				newbon->TradeSkillMastery = base1;
			break;

		case SE_NoBreakAESneak:
			if (newbon->NoBreakAESneak < base1)
				newbon->NoBreakAESneak = base1;
			break;

		case SE_FeignedCastOnChance:
			if (newbon->FeignedCastOnChance < base1)
				newbon->FeignedCastOnChance = base1;
			break;

		// to do
		case SE_PetDiscipline:
			break;
		case SE_PetDiscipline2:
			break;
		case SE_PotionBeltSlots:
			break;
		case SE_BandolierSlots:
			break;
		case SE_ForageSkill:
			break;
		case SE_SecondaryForte:
			break;
		case SE_ExtendedShielding:
			break;
		case SE_ShieldDuration:
			break;
		case SE_ReduceApplyPoisonTime:
			break;
		case SE_NimbleEvasion:
			break;
		case SE_TrapCircumvention:
			break;
		case SE_FeignedMinion:
			break;

		// not handled here
		case SE_HastenedAASkill:
		// not handled here but don't want to clutter debug log -- these may need to be verified to ignore
		case SE_LimitMaxLevel:
		case SE_LimitResist:
		case SE_LimitTarget:
		case SE_LimitEffect:
		case SE_LimitSpellType:
		case SE_LimitMinDur:
		case SE_LimitInstant:
		case SE_LimitMinLevel:
		case SE_LimitCastTimeMin:
		case SE_LimitCastTimeMax:
		case SE_LimitSpell:
		case SE_LimitCombatSkills:
		case SE_LimitManaMin:
		case SE_LimitSpellGroup:
		case SE_LimitSpellClass:
		case SE_LimitSpellSubclass:
		case SE_LimitHPPercent:
		case SE_LimitManaPercent:
		case SE_LimitEndPercent:
		case SE_LimitClass:
		case SE_LimitRace:
		case SE_LimitCastingSkill:
		case SE_LimitUseMin:
		case SE_LimitUseType:
			break;

		default:
			Log.Out(Logs::Detail, Logs::AA, "SPA %d not accounted for in AA %s (%d)", effect, rank.base_ability->name.c_str(), rank.id);
			break;
		}

	}
}

void Mob::CalcSpellBonuses(StatBonuses* newbon)
{
	int i;

	memset(newbon, 0, sizeof(StatBonuses));
	newbon->AggroRange = -1;
	newbon->AssistRange = -1;

	int buff_count = GetMaxTotalSlots();
	for(i = 0; i < buff_count; i++) {
		if(buffs[i].spellid != SPELL_UNKNOWN){
			ApplySpellsBonuses(buffs[i].spellid, buffs[i].casterlevel, newbon, buffs[i].casterid, 0, buffs[i].ticsremaining, i, buffs[i].instrument_mod);

			if (buffs[i].numhits > 0)
				Numhits(true);
		}
	}

	//Applies any perma NPC spell bonuses from npc_spells_effects table.
	if (IsNPC())
		CastToNPC()->ApplyAISpellEffects(newbon);

	//Removes the spell bonuses that are effected by a 'negate' debuff.
	if (spellbonuses.NegateEffects){
		for(i = 0; i < buff_count; i++) {
			if( (buffs[i].spellid != SPELL_UNKNOWN) && (IsEffectInSpell(buffs[i].spellid, SE_NegateSpellEffect)) )
				NegateSpellsBonuses(buffs[i].spellid);
		}
	}

	if (GetClass() == BARD)
		newbon->ManaRegen = 0; // Bards do not get mana regen from spells.
}

void Mob::ApplySpellsBonuses(uint16 spell_id, uint8 casterlevel, StatBonuses *new_bonus, uint16 casterId,
			     uint8 WornType, int32 ticsremaining, int buffslot, int instrument_mod,
			     bool IsAISpellEffect, uint16 effect_id, int32 se_base, int32 se_limit, int32 se_max)
{
	int i, effect_value, base2, max, effectid;
	bool AdditiveWornBonus = false;

	if(!IsAISpellEffect && !IsValidSpell(spell_id))
		return;

	for (i = 0; i < EFFECT_COUNT; i++)
	{
		//Buffs/Item effects
		if (!IsAISpellEffect) {

			if(IsBlankSpellEffect(spell_id, i))
				continue;

			uint8 focus = IsFocusEffect(spell_id, i);
			if (focus)
			{
				if (WornType){
					if (RuleB(Spells, UseAdditiveFocusFromWornSlot))
						new_bonus->FocusEffectsWorn[focus] += spells[spell_id].base[i];
				}

				else
					new_bonus->FocusEffects[focus] = static_cast<uint8>(spells[spell_id].effectid[i]);

				continue;
			}

			if (WornType && (RuleI(Spells, AdditiveBonusWornType) == WornType))
				AdditiveWornBonus = true;

			effectid = spells[spell_id].effectid[i];
			effect_value = CalcSpellEffectValue(spell_id, i, casterlevel, instrument_mod, nullptr, ticsremaining, casterId);
			base2 = spells[spell_id].base2[i];
			max = spells[spell_id].max[i];
		}
		//Use AISpellEffects
		else {
			effectid = effect_id;
			effect_value = se_base;
			base2 = se_limit;
			max = se_max;
			i = EFFECT_COUNT; //End the loop
		}

		switch (effectid)
		{
			case SE_CurrentHP: //regens
				if(effect_value > 0) {
					new_bonus->HPRegen += effect_value;
				}
				break;

			case SE_CurrentEndurance:
				new_bonus->EnduranceRegen += effect_value;
				break;

			case SE_ChangeFrenzyRad:
			{
				// redundant to have level check here
				if(new_bonus->AggroRange == -1 || effect_value < new_bonus->AggroRange)
				{
					new_bonus->AggroRange = static_cast<float>(effect_value);
				}
				break;
			}

			case SE_Harmony:
			{
				// Harmony effect as buff - kinda tricky
				// harmony could stack with a lull spell, which has better aggro range
				// take the one with less range in any case
				if(new_bonus->AssistRange == -1 || effect_value < new_bonus->AssistRange)
				{
					new_bonus->AssistRange = static_cast<float>(effect_value);
				}
				break;
			}

			case SE_AttackSpeed:
			{
				if ((effect_value - 100) > 0) { // Haste
					if (new_bonus->haste < 0) break; // Slowed - Don't apply haste
					if ((effect_value - 100) > new_bonus->haste) {
						new_bonus->haste = effect_value - 100;
					}
				}
				else if ((effect_value - 100) < 0) { // Slow
					int real_slow_value = (100 - effect_value) * -1;
					real_slow_value -= ((real_slow_value * GetSlowMitigation()/100));
					if (real_slow_value < new_bonus->haste)
						new_bonus->haste = real_slow_value;
				}
				break;
			}

			case SE_AttackSpeed2:
			{
				if ((effect_value - 100) > 0) { // Haste V2 - Stacks with V1 but does not Overcap
					if (new_bonus->hastetype2 < 0) break; //Slowed - Don't apply haste2
					if ((effect_value - 100) > new_bonus->hastetype2) {
						new_bonus->hastetype2 = effect_value - 100;
					}
				}
				else if ((effect_value - 100) < 0) { // Slow
					int real_slow_value = (100 - effect_value) * -1;
					real_slow_value -= ((real_slow_value * GetSlowMitigation()/100));
					if (real_slow_value < new_bonus->hastetype2)
						new_bonus->hastetype2 = real_slow_value;
				}
				break;
			}

			case SE_AttackSpeed3:
			{
				if (effect_value < 0){ //Slow
					effect_value -= ((effect_value * GetSlowMitigation()/100));
					if (effect_value < new_bonus->hastetype3)
						new_bonus->hastetype3 = effect_value;
				}

				else if (effect_value > 0) { // Haste V3 - Stacks and Overcaps
					if (effect_value > new_bonus->hastetype3) {
						new_bonus->hastetype3 = effect_value;
					}
				}
				break;
			}

			case SE_AttackSpeed4:
			{
				// These don't generate the IMMUNE_ATKSPEED message and the icon shows up
				// but have no effect on the mobs attack speed
				if (GetSpecialAbility(UNSLOWABLE))
					break;

				if (effect_value < 0) //A few spells use negative values(Descriptions all indicate it should be a slow)
					effect_value = effect_value * -1;

				if (effect_value > 0 && effect_value > new_bonus->inhibitmelee) {
					effect_value -= ((effect_value * GetSlowMitigation()/100));
					if (effect_value > new_bonus->inhibitmelee)
						new_bonus->inhibitmelee = effect_value;
				}

				break;
			}

			case SE_TotalHP:
			{
				new_bonus->HP += effect_value;
				break;
			}

			case SE_ManaRegen_v2:
			case SE_CurrentMana:
			{
				new_bonus->ManaRegen += effect_value;
				break;
			}

			case SE_ManaPool:
			{
				new_bonus->Mana += effect_value;
				break;
			}

			case SE_Stamina:
			{
				new_bonus->EnduranceReduction += effect_value;
				break;
			}

			case SE_ACv2:
			case SE_ArmorClass:
			{
				new_bonus->AC += effect_value;
				break;
			}

			case SE_ATK:
			{
				new_bonus->ATK += effect_value;
				break;
			}

			case SE_STR:
			{
				new_bonus->STR += effect_value;
				break;
			}

			case SE_DEX:
			{
				new_bonus->DEX += effect_value;
				break;
			}

			case SE_AGI:
			{
				new_bonus->AGI += effect_value;
				break;
			}

			case SE_STA:
			{
				new_bonus->STA += effect_value;
				break;
			}

			case SE_INT:
			{
				new_bonus->INT += effect_value;
				break;
			}

			case SE_WIS:
			{
				new_bonus->WIS += effect_value;
				break;
			}

			case SE_CHA:
			{
				if (spells[spell_id].base[i] != 0) {
					new_bonus->CHA += effect_value;
				}
				break;
			}

			case SE_AllStats:
			{
				new_bonus->STR += effect_value;
				new_bonus->DEX += effect_value;
				new_bonus->AGI += effect_value;
				new_bonus->STA += effect_value;
				new_bonus->INT += effect_value;
				new_bonus->WIS += effect_value;
				new_bonus->CHA += effect_value;
				break;
			}

			case SE_ResistFire:
			{
				new_bonus->FR += effect_value;
				break;
			}

			case SE_ResistCold:
			{
				new_bonus->CR += effect_value;
				break;
			}

			case SE_ResistPoison:
			{
				new_bonus->PR += effect_value;
				break;
			}

			case SE_ResistDisease:
			{
				new_bonus->DR += effect_value;
				break;
			}

			case SE_ResistMagic:
			{
				new_bonus->MR += effect_value;
				break;
			}

			case SE_ResistAll:
			{
				new_bonus->MR += effect_value;
				new_bonus->DR += effect_value;
				new_bonus->PR += effect_value;
				new_bonus->CR += effect_value;
				new_bonus->FR += effect_value;
				break;
			}

			case SE_ResistCorruption:
			{
				new_bonus->Corrup += effect_value;
				break;
			}

			case SE_RaiseStatCap:
			{
				switch(spells[spell_id].base2[i])
				{
					//are these #define'd somewhere?
					case 0: //str
						new_bonus->STRCapMod += effect_value;
						break;
					case 1: //sta
						new_bonus->STACapMod += effect_value;
						break;
					case 2: //agi
						new_bonus->AGICapMod += effect_value;
						break;
					case 3: //dex
						new_bonus->DEXCapMod += effect_value;
						break;
					case 4: //wis
						new_bonus->WISCapMod += effect_value;
						break;
					case 5: //int
						new_bonus->INTCapMod += effect_value;
						break;
					case 6: //cha
						new_bonus->CHACapMod += effect_value;
						break;
					case 7: //mr
						new_bonus->MRCapMod += effect_value;
						break;
					case 8: //cr
						new_bonus->CRCapMod += effect_value;
						break;
					case 9: //fr
						new_bonus->FRCapMod += effect_value;
						break;
					case 10: //pr
						new_bonus->PRCapMod += effect_value;
						break;
					case 11: //dr
						new_bonus->DRCapMod += effect_value;
						break;
					case 12: // corruption
						new_bonus->CorrupCapMod += effect_value;
						break;
				}
				break;
			}

			case SE_CastingLevel2:
			case SE_CastingLevel:	// Brilliance of Ro
			{
				new_bonus->effective_casting_level += effect_value;
				break;
			}

			case SE_MovementSpeed:
				new_bonus->movementspeed += effect_value;
				break;

			case SE_SpellDamageShield:
				new_bonus->SpellDamageShield += effect_value;
				break;

			case SE_DamageShield:
			{
				new_bonus->DamageShield += effect_value;
				new_bonus->DamageShieldSpellID = spell_id;
				//When using npc_spells_effects MAX value can be set to determine DS Type
				if (IsAISpellEffect && max)
					new_bonus->DamageShieldType = GetDamageShieldType(spell_id, max);
				else
					new_bonus->DamageShieldType = GetDamageShieldType(spell_id);

				break;
			}

			case SE_ReverseDS:
			{
				new_bonus->ReverseDamageShield += effect_value;
				new_bonus->ReverseDamageShieldSpellID = spell_id;

				if (IsAISpellEffect && max)
					new_bonus->ReverseDamageShieldType = GetDamageShieldType(spell_id, max);
				else
					new_bonus->ReverseDamageShieldType = GetDamageShieldType(spell_id);
				break;
			}

			case SE_Reflect:
				new_bonus->reflect_chance += effect_value;
				break;

			case SE_Amplification:
				new_bonus->Amplification += effect_value;
				break;

			case SE_ChangeAggro:
				new_bonus->hatemod += effect_value;
				break;

			case SE_MeleeMitigation:
				// This value is negative because it counteracts another SPA :P
				new_bonus->MeleeMitigationEffect += effect_value;
				break;

			case SE_CriticalHitChance:
			{
				// Bad data or unsupported new skill
				if (base2 > EQEmu::skills::HIGHEST_SKILL)
					break;
				if (AdditiveWornBonus) {
					if(base2 == ALL_SKILLS)
						new_bonus->CriticalHitChance[EQEmu::skills::HIGHEST_SKILL + 1] += effect_value;
					else
						new_bonus->CriticalHitChance[base2] += effect_value;
				}

				else if(effect_value < 0) {

					if (base2 == ALL_SKILLS && new_bonus->CriticalHitChance[EQEmu::skills::HIGHEST_SKILL + 1] > effect_value)
						new_bonus->CriticalHitChance[EQEmu::skills::HIGHEST_SKILL + 1] = effect_value;
					else if(base2 != ALL_SKILLS && new_bonus->CriticalHitChance[base2] > effect_value)
						new_bonus->CriticalHitChance[base2] = effect_value;
				}


				else if (base2 == ALL_SKILLS && new_bonus->CriticalHitChance[EQEmu::skills::HIGHEST_SKILL + 1] < effect_value)
					new_bonus->CriticalHitChance[EQEmu::skills::HIGHEST_SKILL + 1] = effect_value;
				else if(base2 != ALL_SKILLS && new_bonus->CriticalHitChance[base2] < effect_value)
						new_bonus->CriticalHitChance[base2] = effect_value;

				break;
			}

			case SE_CrippBlowChance:
			{
				if (AdditiveWornBonus)
					new_bonus->CrippBlowChance += effect_value;

				else if((effect_value < 0) && (new_bonus->CrippBlowChance > effect_value))
						new_bonus->CrippBlowChance = effect_value;

				else if(new_bonus->CrippBlowChance < effect_value)
					new_bonus->CrippBlowChance = effect_value;

				break;
			}

			case SE_AvoidMeleeChance:
			{
				if (AdditiveWornBonus)
					new_bonus->AvoidMeleeChanceEffect += effect_value;

				else if((effect_value < 0) && (new_bonus->AvoidMeleeChanceEffect > effect_value))
					new_bonus->AvoidMeleeChanceEffect = effect_value;

				else if(new_bonus->AvoidMeleeChanceEffect < effect_value)
					new_bonus->AvoidMeleeChanceEffect = effect_value;
				break;
			}

			case SE_RiposteChance:
			{
				if (AdditiveWornBonus)
					new_bonus->RiposteChance += effect_value;

				else if((effect_value < 0) && (new_bonus->RiposteChance > effect_value))
					new_bonus->RiposteChance = effect_value;

				else if(new_bonus->RiposteChance < effect_value)
					new_bonus->RiposteChance = effect_value;
				break;
			}

			case SE_DodgeChance:
			{
				if (AdditiveWornBonus)
					new_bonus->DodgeChance += effect_value;

				else if((effect_value < 0) && (new_bonus->DodgeChance > effect_value))
					new_bonus->DodgeChance = effect_value;

				if(new_bonus->DodgeChance < effect_value)
					new_bonus->DodgeChance = effect_value;
				break;
			}

			case SE_ParryChance:
			{
				if (AdditiveWornBonus)
					new_bonus->ParryChance += effect_value;

				else if((effect_value < 0) && (new_bonus->ParryChance > effect_value))
					new_bonus->ParryChance = effect_value;

				if(new_bonus->ParryChance < effect_value)
					new_bonus->ParryChance = effect_value;
				break;
			}

			case SE_DualWieldChance:
			{
				if (AdditiveWornBonus)
					new_bonus->DualWieldChance += effect_value;

				else if((effect_value < 0) && (new_bonus->DualWieldChance > effect_value))
					new_bonus->DualWieldChance = effect_value;

				if(new_bonus->DualWieldChance < effect_value)
					new_bonus->DualWieldChance = effect_value;
				break;
			}

			case SE_DoubleAttackChance:
			{

				if (AdditiveWornBonus)
					new_bonus->DoubleAttackChance += effect_value;

				else if((effect_value < 0) && (new_bonus->DoubleAttackChance > effect_value))
					new_bonus->DoubleAttackChance = effect_value;

				if(new_bonus->DoubleAttackChance < effect_value)
					new_bonus->DoubleAttackChance = effect_value;
				break;
			}

			case SE_TripleAttackChance:
			{

				if (AdditiveWornBonus)
					new_bonus->TripleAttackChance += effect_value;

				else if((effect_value < 0) && (new_bonus->TripleAttackChance > effect_value))
					new_bonus->TripleAttackChance = effect_value;

				if(new_bonus->TripleAttackChance < effect_value)
					new_bonus->TripleAttackChance = effect_value;
				break;
			}

			case SE_MeleeLifetap:
			{
				if (AdditiveWornBonus)
					new_bonus->MeleeLifetap += spells[spell_id].base[i];

				else if((effect_value < 0) && (new_bonus->MeleeLifetap > effect_value))
					new_bonus->MeleeLifetap = effect_value;

				else if(new_bonus->MeleeLifetap < effect_value)
					new_bonus->MeleeLifetap = effect_value;
				break;
			}

			case SE_Vampirism:
				new_bonus->Vampirism += effect_value;
				break;

			case SE_AllInstrumentMod:
			{
				if(effect_value > new_bonus->singingMod)
					new_bonus->singingMod = effect_value;
				if(effect_value > new_bonus->brassMod)
					new_bonus->brassMod = effect_value;
				if(effect_value > new_bonus->percussionMod)
					new_bonus->percussionMod = effect_value;
				if(effect_value > new_bonus->windMod)
					new_bonus->windMod = effect_value;
				if(effect_value > new_bonus->stringedMod)
					new_bonus->stringedMod = effect_value;
				break;
			}

			case SE_ResistSpellChance:
				new_bonus->ResistSpellChance += effect_value;
				break;

			case SE_ResistFearChance:
			{
				if(effect_value == 100) // If we reach 100% in a single spell/item then we should be immune to negative fear resist effects until our immunity is over
					new_bonus->Fearless = true;

				new_bonus->ResistFearChance += effect_value; // these should stack
				break;
			}

			case SE_Fearless:
				new_bonus->Fearless = true;
				break;

			case SE_HundredHands:
			{
				if (AdditiveWornBonus)
					new_bonus->HundredHands += effect_value;

				if (effect_value > 0 && effect_value > new_bonus->HundredHands)
					new_bonus->HundredHands = effect_value; //Increase Weapon Delay
				else if (effect_value < 0 && effect_value < new_bonus->HundredHands)
					new_bonus->HundredHands = effect_value; //Decrease Weapon Delay
				break;
			}

			case SE_MeleeSkillCheck:
			{
				if(new_bonus->MeleeSkillCheck < effect_value) {
					new_bonus->MeleeSkillCheck = effect_value;
					new_bonus->MeleeSkillCheckSkill = base2==ALL_SKILLS?255:base2;
				}
				break;
			}

			case SE_HitChance:
			{
				// Bad data or unsupported new skill
				if (base2 > EQEmu::skills::HIGHEST_SKILL)
					break;

				if (AdditiveWornBonus){
					if(base2 == ALL_SKILLS)
						new_bonus->HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1] += effect_value;
					else
						new_bonus->HitChanceEffect[base2] += effect_value;
				}

				else if(base2 == ALL_SKILLS){

					if ((effect_value < 0) && (new_bonus->HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1] > effect_value))
						new_bonus->HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1] = effect_value;

					else if (!new_bonus->HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1] ||
						((new_bonus->HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1] > 0) && (new_bonus->HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1] < effect_value)))
						new_bonus->HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1] = effect_value;
				}

				else {

					if ((effect_value < 0) && (new_bonus->HitChanceEffect[base2] > effect_value))
						new_bonus->HitChanceEffect[base2] = effect_value;

					else if (!new_bonus->HitChanceEffect[base2] ||
							((new_bonus->HitChanceEffect[base2] > 0) && (new_bonus->HitChanceEffect[base2] < effect_value)))
							new_bonus->HitChanceEffect[base2] = effect_value;
				}

				break;

			}

			case SE_DamageModifier:
			{
				// Bad data or unsupported new skill
				if (base2 > EQEmu::skills::HIGHEST_SKILL)
					break;
				int skill = base2 == ALL_SKILLS ? EQEmu::skills::HIGHEST_SKILL + 1 : base2;
				if (effect_value < 0 && new_bonus->DamageModifier[skill] > effect_value)
					new_bonus->DamageModifier[skill] = effect_value;
				else if (effect_value > 0 && new_bonus->DamageModifier[skill] < effect_value)
					new_bonus->DamageModifier[skill] = effect_value;
				break;
			}

			case SE_DamageModifier2:
			{
				// Bad data or unsupported new skill
				if (base2 > EQEmu::skills::HIGHEST_SKILL)
					break;
				int skill = base2 == ALL_SKILLS ? EQEmu::skills::HIGHEST_SKILL + 1 : base2;
				if (effect_value < 0 && new_bonus->DamageModifier2[skill] > effect_value)
					new_bonus->DamageModifier2[skill] = effect_value;
				else if (effect_value > 0 && new_bonus->DamageModifier2[skill] < effect_value)
					new_bonus->DamageModifier2[skill] = effect_value;
				break;
			}

			case SE_MinDamageModifier:
			{
				// Bad data or unsupported new skill
				if (base2 > EQEmu::skills::HIGHEST_SKILL)
					break;
				int skill = base2 == ALL_SKILLS ? EQEmu::skills::HIGHEST_SKILL + 1 : base2;
				if (effect_value < 0 && new_bonus->MinDamageModifier[skill] > effect_value)
					new_bonus->MinDamageModifier[skill] = effect_value;
				else if (effect_value > 0 && new_bonus->MinDamageModifier[skill] < effect_value)
					new_bonus->MinDamageModifier[skill] = effect_value;
				break;
			}

			case SE_StunResist:
			{
				if(new_bonus->StunResist < effect_value)
					new_bonus->StunResist = effect_value;
				break;
			}

			case SE_ProcChance:
			{
				if (AdditiveWornBonus)
					new_bonus->ProcChanceSPA += effect_value;

				else if((effect_value < 0) && (new_bonus->ProcChanceSPA > effect_value))
					new_bonus->ProcChanceSPA = effect_value;

				if(new_bonus->ProcChanceSPA < effect_value)
					new_bonus->ProcChanceSPA = effect_value;

				break;
			}

			case SE_ExtraAttackChance:
				new_bonus->ExtraAttackChance += effect_value;
				break;

			case SE_PercentXPIncrease:
			{
				if(new_bonus->XPRateMod < effect_value)
					new_bonus->XPRateMod = effect_value;
				break;
			}

			case SE_DeathSave:
			{
				if(new_bonus->DeathSave[0] < effect_value)
				{
					new_bonus->DeathSave[0] = effect_value; //1='Partial' 2='Full'
					new_bonus->DeathSave[1] = buffslot;
					//These are used in later expansion spell effects.
					new_bonus->DeathSave[2] = base2;//Min level for HealAmt
					new_bonus->DeathSave[3] = max;//HealAmt
				}
				break;
			}

			case SE_DivineSave:
			{
				if (AdditiveWornBonus) {
					new_bonus->DivineSaveChance[0] += effect_value;
					new_bonus->DivineSaveChance[1] = 0;
				}

				else if(new_bonus->DivineSaveChance[0] < effect_value)
				{
					new_bonus->DivineSaveChance[0] = effect_value;
					new_bonus->DivineSaveChance[1] = base2;
				}
				break;
			}

			case SE_Flurry:
				new_bonus->FlurryChance += effect_value;
				break;

			case SE_Accuracy:
			{
				// Bad data or unsupported new skill
				if (base2 > EQEmu::skills::HIGHEST_SKILL)
					break;
				if ((effect_value < 0) && (new_bonus->Accuracy[EQEmu::skills::HIGHEST_SKILL + 1] > effect_value))
					new_bonus->Accuracy[EQEmu::skills::HIGHEST_SKILL + 1] = effect_value;

				else if (!new_bonus->Accuracy[EQEmu::skills::HIGHEST_SKILL + 1] ||
					((new_bonus->Accuracy[EQEmu::skills::HIGHEST_SKILL + 1] > 0) && (new_bonus->Accuracy[EQEmu::skills::HIGHEST_SKILL + 1] < effect_value)))
					new_bonus->Accuracy[EQEmu::skills::HIGHEST_SKILL + 1] = effect_value;
				break;
			}

			case SE_MaxHPChange:
				new_bonus->MaxHPChange += effect_value;
				break;

			case SE_EndurancePool:
				new_bonus->Endurance += effect_value;
				break;

			case SE_HealRate:
				new_bonus->HealRate += effect_value;
				break;

			case SE_SkillDamageTaken:
			{
				// Bad data or unsupported new skill
				if (base2 > EQEmu::skills::HIGHEST_SKILL)
					break;
				//When using npc_spells_effects if MAX value set, use stackable quest based modifier.
				if (IsAISpellEffect && max){
					if(base2 == ALL_SKILLS)
						SkillDmgTaken_Mod[EQEmu::skills::HIGHEST_SKILL + 1] = effect_value;
					else
						SkillDmgTaken_Mod[base2] = effect_value;
				}
				else {

					if(base2 == ALL_SKILLS)
						new_bonus->SkillDmgTaken[EQEmu::skills::HIGHEST_SKILL + 1] += effect_value;
					else
						new_bonus->SkillDmgTaken[base2] += effect_value;

				}
				break;
			}

			case SE_TriggerOnCast:
			{
				for(int e = 0; e < MAX_SPELL_TRIGGER; e++)
				{
					if(!new_bonus->SpellTriggers[e])
					{
						new_bonus->SpellTriggers[e] = spell_id;
						break;
					}
				}
				break;
			}

			case SE_SpellCritChance:
				new_bonus->CriticalSpellChance += effect_value;
				break;

			case SE_CriticalSpellChance:
			{
				new_bonus->CriticalSpellChance += effect_value;

				if (base2 > new_bonus->SpellCritDmgIncNoStack)
					new_bonus->SpellCritDmgIncNoStack = base2;
				break;
			}

			case SE_SpellCritDmgIncrease:
				new_bonus->SpellCritDmgIncrease += effect_value;
				break;

			case SE_DotCritDmgIncrease:
				new_bonus->DotCritDmgIncrease += effect_value;
				break;

			case SE_CriticalHealChance:
				new_bonus->CriticalHealChance += effect_value;
				break;

			case SE_CriticalHealOverTime:
				new_bonus->CriticalHealOverTime += effect_value;
				break;

			case SE_CriticalHealDecay:
				new_bonus->CriticalHealDecay = true;
				break;

			case SE_CriticalRegenDecay:
				new_bonus->CriticalRegenDecay = true;
				break;

			case SE_CriticalDotDecay:
				new_bonus->CriticalDotDecay = true;
				break;

			case SE_MitigateDamageShield:
			{
				if (effect_value < 0)
					effect_value = effect_value*-1;

				new_bonus->DSMitigationOffHand += effect_value;
				break;
			}

			case SE_CriticalDoTChance:
				new_bonus->CriticalDoTChance += effect_value;
				break;

			case SE_ProcOnKillShot:
			{
				for(int e = 0; e < MAX_SPELL_TRIGGER*3; e+=3)
				{
					if(!new_bonus->SpellOnKill[e])
					{
						// Base2 = Spell to fire | Base1 = % chance | Base3 = min level
						new_bonus->SpellOnKill[e] = base2;
						new_bonus->SpellOnKill[e+1] = effect_value;
						new_bonus->SpellOnKill[e+2] = max;
						break;
					}
				}
				break;
			}

			case SE_SpellOnDeath:
			{
				for(int e = 0; e < MAX_SPELL_TRIGGER; e+=2)
				{
					if(!new_bonus->SpellOnDeath[e])
					{
						// Base2 = Spell to fire | Base1 = % chance
						new_bonus->SpellOnDeath[e] = base2;
						new_bonus->SpellOnDeath[e+1] = effect_value;
						break;
					}
				}
				break;
			}

			case SE_CriticalDamageMob:
			{
				// Bad data or unsupported new skill
				if (base2 > EQEmu::skills::HIGHEST_SKILL)
					break;
				if(base2 == ALL_SKILLS)
					new_bonus->CritDmgMob[EQEmu::skills::HIGHEST_SKILL + 1] += effect_value;
				else
					new_bonus->CritDmgMob[base2] += effect_value;
				break;
			}

			case SE_ReduceSkillTimer:
			{
				if(new_bonus->SkillReuseTime[base2] < effect_value)
					new_bonus->SkillReuseTime[base2] = effect_value;
				break;
			}

			case SE_SkillDamageAmount:
			{
				// Bad data or unsupported new skill
				if (base2 > EQEmu::skills::HIGHEST_SKILL)
					break;
				if(base2 == ALL_SKILLS)
					new_bonus->SkillDamageAmount[EQEmu::skills::HIGHEST_SKILL + 1] += effect_value;
				else
					new_bonus->SkillDamageAmount[base2] += effect_value;
				break;
			}

			case SE_GravityEffect:
				new_bonus->GravityEffect = 1;
				break;

			case SE_AntiGate:
				new_bonus->AntiGate = true;
				break;

			case SE_MagicWeapon:
				new_bonus->MagicWeapon = true;
				break;

			case SE_IncreaseBlockChance:
				new_bonus->IncreaseBlockChance += effect_value;
				break;

			case SE_PersistantCasting:
				new_bonus->PersistantCasting += effect_value;
				break;

			case SE_LimitHPPercent:
			{
				if(new_bonus->HPPercCap[0] != 0 && new_bonus->HPPercCap[0] > effect_value){
					new_bonus->HPPercCap[0] = effect_value;
					new_bonus->HPPercCap[1] = base2;
				}
				else if(new_bonus->HPPercCap[0] == 0){
					new_bonus->HPPercCap[0] = effect_value;
					new_bonus->HPPercCap[1] = base2;
				}
				break;
			}
			case SE_LimitManaPercent:
			{
				if(new_bonus->ManaPercCap[0] != 0 && new_bonus->ManaPercCap[0] > effect_value){
					new_bonus->ManaPercCap[0] = effect_value;
					new_bonus->ManaPercCap[1] = base2;
				}
				else if(new_bonus->ManaPercCap[0] == 0) {
					new_bonus->ManaPercCap[0] = effect_value;
					new_bonus->ManaPercCap[1] = base2;
				}

				break;
			}
			case SE_LimitEndPercent:
			{
				if(new_bonus->EndPercCap[0] != 0 && new_bonus->EndPercCap[0] > effect_value) {
					new_bonus->EndPercCap[0] = effect_value;
					new_bonus->EndPercCap[1] = base2;
				}

				else if(new_bonus->EndPercCap[0] == 0){
					new_bonus->EndPercCap[0] = effect_value;
					new_bonus->EndPercCap[1] = base2;
				}

				break;
			}

			case SE_BlockNextSpellFocus:
				new_bonus->BlockNextSpell = true;
				break;

			case SE_NegateSpellEffect:
				new_bonus->NegateEffects = true;
				break;

			case SE_ImmuneFleeing:
				new_bonus->ImmuneToFlee = true;
				if (currently_fleeing) // lets update shit now instead of next tick
					ProcessFlee();
				break;

			case SE_DelayDeath:
				new_bonus->DelayDeath += effect_value;
				break;

			case SE_SpellProcChance:
				new_bonus->SpellProcChance += effect_value;
				break;

			case SE_CharmBreakChance:
				new_bonus->CharmBreakChance += effect_value;
				break;

			case SE_BardSongRange:
				new_bonus->SongRange += effect_value;
				break;

			case SE_HPToMana:
			{
				//Lower the ratio the more favorable
				if((!new_bonus->HPToManaConvert) || (new_bonus->HPToManaConvert >= effect_value))
				new_bonus->HPToManaConvert = spells[spell_id].base[i];
				break;
			}

			case SE_SkillDamageAmount2:
			{
				// Bad data or unsupported new skill
				if (base2 > EQEmu::skills::HIGHEST_SKILL)
					break;
				if(base2 == ALL_SKILLS)
					new_bonus->SkillDamageAmount2[EQEmu::skills::HIGHEST_SKILL + 1] += effect_value;
				else
					new_bonus->SkillDamageAmount2[base2] += effect_value;
				break;
			}

			case SE_NegateAttacks:
			{
				if (!new_bonus->NegateAttacks[0] ||
					((new_bonus->NegateAttacks[0] && new_bonus->NegateAttacks[2]) && (new_bonus->NegateAttacks[2] < max))){
					new_bonus->NegateAttacks[0] = 1;
					new_bonus->NegateAttacks[1] = buffslot;
					new_bonus->NegateAttacks[2] = max;
				}
				break;
			}

			case SE_MitigateMeleeDamage:
			{
				if (new_bonus->MitigateMeleeRune[0] < effect_value){
					new_bonus->MitigateMeleeRune[0] = effect_value;
					new_bonus->MitigateMeleeRune[1] = buffslot;
					new_bonus->MitigateMeleeRune[2] = base2;
					new_bonus->MitigateMeleeRune[3] = max;
				}
				break;
			}


			case SE_MeleeThresholdGuard:
			{
				if (new_bonus->MeleeThresholdGuard[0] < effect_value){
					new_bonus->MeleeThresholdGuard[0] = effect_value;
					new_bonus->MeleeThresholdGuard[1] = buffslot;
					new_bonus->MeleeThresholdGuard[2] = base2;
				}
				break;
			}

			case SE_SpellThresholdGuard:
			{
				if (new_bonus->SpellThresholdGuard[0] < effect_value){
					new_bonus->SpellThresholdGuard[0] = effect_value;
					new_bonus->SpellThresholdGuard[1] = buffslot;
					new_bonus->SpellThresholdGuard[2] = base2;
				}
				break;
			}

			case SE_MitigateSpellDamage:
			{
				if (new_bonus->MitigateSpellRune[0] < effect_value){
					new_bonus->MitigateSpellRune[0] = effect_value;
					new_bonus->MitigateSpellRune[1] = buffslot;
					new_bonus->MitigateSpellRune[2] = base2;
					new_bonus->MitigateSpellRune[3] = max;
				}
				break;
			}

			case SE_MitigateDotDamage:
			{
				if (new_bonus->MitigateDotRune[0] < effect_value){
					new_bonus->MitigateDotRune[0] = effect_value;
					new_bonus->MitigateDotRune[1] = buffslot;
					new_bonus->MitigateDotRune[2] = base2;
					new_bonus->MitigateDotRune[3] = max;
				}
				break;
			}

			case SE_ManaAbsorbPercentDamage:
			{
				if (new_bonus->ManaAbsorbPercentDamage[0] < effect_value){
					new_bonus->ManaAbsorbPercentDamage[0] = effect_value;
					new_bonus->ManaAbsorbPercentDamage[1] = buffslot;
				}
				break;
			}

			case SE_TriggerMeleeThreshold:
				new_bonus->TriggerMeleeThreshold = true;
				break;

			case SE_TriggerSpellThreshold:
				new_bonus->TriggerSpellThreshold = true;
				break;

			case SE_ShieldBlock:
				new_bonus->ShieldBlock += effect_value;
				break;

			case SE_ShieldEquipDmgMod:
				new_bonus->ShieldEquipDmgMod += effect_value;
				break;

			case SE_BlockBehind:
				new_bonus->BlockBehind += effect_value;
				break;

			case SE_Blind:
				new_bonus->IsBlind = true;
				break;

			case SE_Fear:
				new_bonus->IsFeared = true;
				break;

			//AA bonuses - implemented broadly into spell/item effects
			case SE_FrontalStunResist:
				new_bonus->FrontalStunResist += effect_value;
				break;

			case SE_ImprovedBindWound:
				new_bonus->BindWound += effect_value;
				break;

			case SE_MaxBindWound:
				new_bonus->MaxBindWound += effect_value;
				break;

			case SE_BaseMovementSpeed:
				new_bonus->BaseMovementSpeed += effect_value;
				break;

			case SE_IncreaseRunSpeedCap:
				new_bonus->IncreaseRunSpeedCap += effect_value;
				break;

			case SE_DoubleSpecialAttack:
				new_bonus->DoubleSpecialAttack += effect_value;
				break;

			case SE_TripleBackstab:
				new_bonus->TripleBackstab += effect_value;
				break;

			case SE_FrontalBackstabMinDmg:
				new_bonus->FrontalBackstabMinDmg = true;
				break;

			case SE_FrontalBackstabChance:
				new_bonus->FrontalBackstabChance += effect_value;
				break;

			case SE_ConsumeProjectile:
				new_bonus->ConsumeProjectile += effect_value;
				break;

			case SE_ForageAdditionalItems:
				new_bonus->ForageAdditionalItems += effect_value;
				break;

			case SE_Salvage:
				new_bonus->SalvageChance += effect_value;
				break;

			case SE_ArcheryDamageModifier:
				new_bonus->ArcheryDamageModifier += effect_value;
				break;

			case SE_DoubleRangedAttack:
				new_bonus->DoubleRangedAttack += effect_value;
				break;

			case SE_SecondaryDmgInc:
				new_bonus->SecondaryDmgInc = true;
				break;

			case SE_StrikeThrough:
			case SE_StrikeThrough2:
				new_bonus->StrikeThrough += effect_value;
				break;

			case SE_GiveDoubleAttack:
				new_bonus->GiveDoubleAttack += effect_value;
				break;

			case SE_PetCriticalHit:
				new_bonus->PetCriticalHit += effect_value;
				break;

			case SE_CombatStability:
				new_bonus->CombatStability += effect_value;
				break;

			case SE_AddSingingMod:
				switch (base2) {
				case EQEmu::item::ItemTypeWindInstrument:
					new_bonus->windMod += effect_value;
					break;
				case EQEmu::item::ItemTypeStringedInstrument:
					new_bonus->stringedMod += effect_value;
					break;
				case EQEmu::item::ItemTypeBrassInstrument:
					new_bonus->brassMod += effect_value;
					break;
				case EQEmu::item::ItemTypePercussionInstrument:
					new_bonus->percussionMod += effect_value;
					break;
				case EQEmu::item::ItemTypeSinging:
					new_bonus->singingMod += effect_value;
					break;
				default:
					break;
				}
				break;

			case SE_SongModCap:
				new_bonus->songModCap += effect_value;
				break;

			case SE_PetAvoidance:
				new_bonus->PetAvoidance += effect_value;
				break;

			case SE_Ambidexterity:
				new_bonus->Ambidexterity += effect_value;
				break;

			case SE_PetMaxHP:
				new_bonus->PetMaxHP += effect_value;
				break;

			case SE_PetFlurry:
				new_bonus->PetFlurry += effect_value;
				break;

			case SE_GivePetGroupTarget:
				new_bonus->GivePetGroupTarget = true;
				break;

			case SE_RootBreakChance:
				new_bonus->RootBreakChance += effect_value;
				break;

			case SE_ChannelChanceItems:
				new_bonus->ChannelChanceItems += effect_value;
				break;

			case SE_ChannelChanceSpells:
				new_bonus->ChannelChanceSpells += effect_value;
				break;

			case SE_UnfailingDivinity:
				new_bonus->UnfailingDivinity += effect_value;
				break;


			case SE_ItemHPRegenCapIncrease:
				new_bonus->ItemHPRegenCap += effect_value;
				break;

			case SE_OffhandRiposteFail:
				new_bonus->OffhandRiposteFail += effect_value;
				break;

			case SE_ItemAttackCapIncrease:
				new_bonus->ItemATKCap += effect_value;
				break;

			case SE_TwoHandBluntBlock:
				new_bonus->TwoHandBluntBlock += effect_value;
				break;

			case SE_StunBashChance:
				new_bonus->StunBashChance += effect_value;
				break;

			case SE_IncreaseChanceMemwipe:
				new_bonus->IncreaseChanceMemwipe += effect_value;
				break;

			case SE_CriticalMend:
				new_bonus->CriticalMend += effect_value;
				break;

			case SE_SpellEffectResistChance:
			{
				for(int e = 0; e < MAX_RESISTABLE_EFFECTS*2; e+=2)
				{
					if(new_bonus->SEResist[e+1] && (new_bonus->SEResist[e] == base2) && (new_bonus->SEResist[e+1] < effect_value)){
						new_bonus->SEResist[e] = base2; //Spell Effect ID
						new_bonus->SEResist[e+1] = effect_value; //Resist Chance
						break;
					}
					else if (!new_bonus->SEResist[e+1]){
						new_bonus->SEResist[e] = base2; //Spell Effect ID
						new_bonus->SEResist[e+1] = effect_value; //Resist Chance
						break;
					}
				}
				break;
			}

			case SE_MasteryofPast:
			{
				if(new_bonus->MasteryofPast < effect_value)
					new_bonus->MasteryofPast = effect_value;
				break;
			}

			case SE_DoubleRiposte:
			{
				new_bonus->DoubleRiposte += effect_value;
			}

			case SE_GiveDoubleRiposte:
			{
				//Only allow for regular double riposte chance.
				if(new_bonus->GiveDoubleRiposte[base2] == 0){
					if(new_bonus->GiveDoubleRiposte[0] < effect_value)
						new_bonus->GiveDoubleRiposte[0] = effect_value;
				}
				break;
			}

			case SE_SlayUndead:
			{
				if(new_bonus->SlayUndead[1] < effect_value)
					new_bonus->SlayUndead[0] = effect_value; // Rate
					new_bonus->SlayUndead[1] = base2; // Damage Modifier
				break;
			}

			case SE_TriggerOnReqTarget:
			case SE_TriggerOnReqCaster:
				new_bonus->TriggerOnValueAmount = true;
				break;

			case SE_DivineAura:
				new_bonus->DivineAura = true;
				break;

			case SE_ImprovedTaunt:
				if (new_bonus->ImprovedTaunt[0] < effect_value) {
					new_bonus->ImprovedTaunt[0] = effect_value;
					new_bonus->ImprovedTaunt[1] = base2;
					new_bonus->ImprovedTaunt[2] = buffslot;
				}
				break;


			case SE_DistanceRemoval:
				new_bonus->DistanceRemoval = true;
				break;

			case SE_FrenziedDevastation:
				new_bonus->FrenziedDevastation += base2;
				break;

			case SE_Root:
				if (new_bonus->Root[0] && (new_bonus->Root[1] > buffslot)){
					new_bonus->Root[0] = 1;
					new_bonus->Root[1] = buffslot;
				}
				else if (!new_bonus->Root[0]){
					new_bonus->Root[0] = 1;
					new_bonus->Root[1] = buffslot;
				}
				break;

			case SE_Rune:

				if (new_bonus->MeleeRune[0] && (new_bonus->MeleeRune[1] > buffslot)){

					new_bonus->MeleeRune[0] = effect_value;
					new_bonus->MeleeRune[1] = buffslot;
				}
				else if (!new_bonus->MeleeRune[0]){
					new_bonus->MeleeRune[0] = effect_value;
					new_bonus->MeleeRune[1] = buffslot;
				}

				break;

			case SE_AbsorbMagicAtt:
				if (new_bonus->AbsorbMagicAtt[0] && (new_bonus->AbsorbMagicAtt[1] > buffslot)){
					new_bonus->AbsorbMagicAtt[0] = effect_value;
					new_bonus->AbsorbMagicAtt[1] = buffslot;
				}
				else if (!new_bonus->AbsorbMagicAtt[0]){
					new_bonus->AbsorbMagicAtt[0] = effect_value;
					new_bonus->AbsorbMagicAtt[1] = buffslot;
				}
				break;

			case SE_NegateIfCombat:
				new_bonus->NegateIfCombat = true;
				break;

			case SE_Screech:
				new_bonus->Screech = effect_value;
				break;

			case SE_AlterNPCLevel:

				if (IsNPC()){
					if (!new_bonus->AlterNPCLevel
					|| ((effect_value < 0) && (new_bonus->AlterNPCLevel > effect_value))
					|| ((effect_value > 0) && (new_bonus->AlterNPCLevel < effect_value))) {

						int tmp_lv =  GetOrigLevel() + effect_value;
						if (tmp_lv < 1)
							tmp_lv = 1;
						else if (tmp_lv > 255)
							tmp_lv = 255;
						if ((GetLevel() != tmp_lv)){
							new_bonus->AlterNPCLevel = effect_value;
							SetLevel(tmp_lv);
						}
					}
				}
				break;

			case SE_AStacker:
				new_bonus->AStacker[0] = 1;
				new_bonus->AStacker[1] = effect_value;
				break;

			case SE_BStacker:
				new_bonus->BStacker[0] = 1;
				new_bonus->BStacker[1] = effect_value;
				break;

			case SE_CStacker:
				new_bonus->CStacker[0] = 1;
				new_bonus->CStacker[1] = effect_value;
				break;

			case SE_DStacker:
				new_bonus->DStacker[0] = 1;
				new_bonus->DStacker[1] = effect_value;
				break;

			case SE_Berserk:
				new_bonus->BerserkSPA = true;
				break;


			case SE_Metabolism:
				new_bonus->Metabolism += effect_value;
				break;

			case SE_ImprovedReclaimEnergy:
			{
				if((effect_value < 0) && (new_bonus->ImprovedReclaimEnergy > effect_value))
					new_bonus->ImprovedReclaimEnergy = effect_value;

				else if(new_bonus->ImprovedReclaimEnergy < effect_value)
					new_bonus->ImprovedReclaimEnergy = effect_value;
				break;
			}

			case SE_HeadShot:
			{
				if(new_bonus->HeadShot[1] < base2){
					new_bonus->HeadShot[0] = effect_value;
					new_bonus->HeadShot[1] = base2;
				}
				break;
			}

			case SE_HeadShotLevel:
			{
				if(new_bonus->HSLevel[0] < effect_value) {
					new_bonus->HSLevel[0] = effect_value;
					new_bonus->HSLevel[1] = base2;
				}
				break;
			}

			case SE_Assassinate:
			{
				if(new_bonus->Assassinate[1] < base2){
					new_bonus->Assassinate[0] = effect_value;
					new_bonus->Assassinate[1] = base2;
				}
				break;
			}

			case SE_AssassinateLevel:
			{
				if(new_bonus->AssassinateLevel[0] < effect_value) {
					new_bonus->AssassinateLevel[0] = effect_value;
					new_bonus->AssassinateLevel[1] = base2;
				}
				break;
			}

			case SE_FinishingBlow:
			{
				//base1 = chance, base2 = damage
				if (new_bonus->FinishingBlow[1] < base2){
					new_bonus->FinishingBlow[0] = effect_value;
					new_bonus->FinishingBlow[1] = base2;
				}
				break;
			}

			case SE_FinishingBlowLvl:
			{
				//base1 = level, base2 = ??? (Set to 200 in AA data, possible proc rate mod?)
				if (new_bonus->FinishingBlowLvl[0] < effect_value){
					new_bonus->FinishingBlowLvl[0] = effect_value;
					new_bonus->FinishingBlowLvl[1] = base2;
				}
				break;
			}

			case SE_PetMeleeMitigation:
				new_bonus->PetMeleeMitigation += effect_value;
				break;

			case SE_Sanctuary:
				new_bonus->Sanctuary = true;
				break;

			case SE_FactionModPct:
			{
				if((effect_value < 0) && (new_bonus->FactionModPct > effect_value))
					new_bonus->FactionModPct = effect_value;

				else if(new_bonus->FactionModPct < effect_value)
					new_bonus->FactionModPct = effect_value;
				break;
			}

			case SE_IllusionPersistence:
				new_bonus->IllusionPersistence = true;
				break;

			case SE_LimitToSkill:{
				// Bad data or unsupported new skill
				if (base2 > EQEmu::skills::HIGHEST_SKILL)
					break;
				if (effect_value <= EQEmu::skills::HIGHEST_SKILL){
					new_bonus->LimitToSkill[effect_value] = true;
				}
				break;
			}

			case SE_SkillProc:{

				for(int e = 0; e < MAX_SKILL_PROCS; e++)
				{
					if(new_bonus->SkillProc[e] && new_bonus->SkillProc[e] == spell_id)
						break; //Do not use the same spell id more than once.

					else if(!new_bonus->SkillProc[e]){
						new_bonus->SkillProc[e] = spell_id;
						break;
					}
				}
				break;
			}

			case SE_SkillProcSuccess:{

				for(int e = 0; e < MAX_SKILL_PROCS; e++)
				{
					if(new_bonus->SkillProcSuccess[e] && new_bonus->SkillProcSuccess[e] == spell_id)
						break; //Do not use the same spell id more than once.

					else if(!new_bonus->SkillProcSuccess[e]){
						new_bonus->SkillProcSuccess[e] = spell_id;
						break;
					}
				}
				break;
			}

			case SE_PC_Pet_Rampage: {
				new_bonus->PC_Pet_Rampage[0] += effect_value; //Chance to rampage
				if (new_bonus->PC_Pet_Rampage[1] < base2)
					new_bonus->PC_Pet_Rampage[1] = base2; //Damage modifer - take highest
				break;
			}

			case SE_PC_Pet_Flurry_Chance: 
				new_bonus->PC_Pet_Flurry += effect_value; //Chance to Flurry
				break;

			case SE_ShroudofStealth:
				new_bonus->ShroudofStealth = true;
				break;

			case SE_ReduceFallDamage:
				new_bonus->ReduceFallDamage += effect_value;
				break;

			case SE_ReduceTradeskillFail:{

				if (base2 > EQEmu::skills::HIGHEST_SKILL)
					break;

				new_bonus->ReduceTradeskillFail[base2] += effect_value;
				break;
			}

			case SE_TradeSkillMastery:
				if (new_bonus->TradeSkillMastery < effect_value)
					new_bonus->TradeSkillMastery = effect_value;
				break;

			case SE_RaiseSkillCap: {
				if (base2 > EQEmu::skills::HIGHEST_SKILL)
					break;
				
				if (new_bonus->RaiseSkillCap[base2] < effect_value) 
					new_bonus->RaiseSkillCap[base2] = effect_value;
				break;
			}

			case SE_NoBreakAESneak:
				if (new_bonus->NoBreakAESneak < effect_value)
					new_bonus->NoBreakAESneak = effect_value;
				break;

			case SE_FeignedCastOnChance:
				if (new_bonus->FeignedCastOnChance < effect_value)
					new_bonus->FeignedCastOnChance = effect_value;
				break;
		
			//Special custom cases for loading effects on to NPC from 'npc_spels_effects' table
			if (IsAISpellEffect) {

				//Non-Focused Effect to modify incoming spell damage by resist type.
				case SE_FcSpellVulnerability:
					ModVulnerability(base2, effect_value);
				break;
			}
		}
	}
}

void NPC::CalcItemBonuses(StatBonuses *newbon)
{
	if(newbon){

		for (int i = 0; i < EQEmu::legacy::EQUIPMENT_SIZE; i++){
			const EQEmu::ItemData *cur = database.GetItem(equipment[i]);
			if(cur){
				//basic stats
				newbon->AC += cur->AC;
				newbon->HP += cur->HP;
				newbon->Mana += cur->Mana;
				newbon->Endurance += cur->Endur;
				newbon->STR += (cur->AStr + cur->HeroicStr);
				newbon->STA += (cur->ASta + cur->HeroicSta);
				newbon->DEX += (cur->ADex + cur->HeroicDex);
				newbon->AGI += (cur->AAgi + cur->HeroicAgi);
				newbon->INT += (cur->AInt + cur->HeroicInt);
				newbon->WIS += (cur->AWis + cur->HeroicWis);
				newbon->CHA += (cur->ACha + cur->HeroicCha);
				newbon->MR += (cur->MR + cur->HeroicMR);
				newbon->FR += (cur->FR + cur->HeroicFR);
				newbon->CR += (cur->CR + cur->HeroicCR);
				newbon->PR += (cur->PR + cur->HeroicPR);
				newbon->DR += (cur->DR + cur->HeroicDR);
				newbon->Corrup += (cur->SVCorruption + cur->HeroicSVCorrup);

				//more complex stats
				if(cur->Regen > 0) {
					newbon->HPRegen += cur->Regen;
				}
				if(cur->ManaRegen > 0) {
					newbon->ManaRegen += cur->ManaRegen;
				}
				if(cur->Attack > 0) {
					newbon->ATK += cur->Attack;
				}
				if(cur->DamageShield > 0) {
					newbon->DamageShield += cur->DamageShield;
				}
				if(cur->SpellShield > 0) {
					newbon->SpellDamageShield += cur->SpellShield;
				}
				if(cur->Shielding > 0) {
					newbon->MeleeMitigation += cur->Shielding;
				}
				if(cur->StunResist > 0) {
					newbon->StunResist += cur->StunResist;
				}
				if(cur->StrikeThrough > 0) {
					newbon->StrikeThrough += cur->StrikeThrough;
				}
				if(cur->Avoidance > 0) {
					newbon->AvoidMeleeChance += cur->Avoidance;
				}
				if(cur->Accuracy > 0) {
					newbon->HitChance += cur->Accuracy;
				}
				if(cur->CombatEffects > 0) {
					newbon->ProcChance += cur->CombatEffects;
				}
				if (cur->Worn.Effect>0 && (cur->Worn.Type == EQEmu::item::ItemEffectWorn)) { // latent effects
					ApplySpellsBonuses(cur->Worn.Effect, cur->Worn.Level, newbon, 0, cur->Worn.Type);
				}

				if (RuleB(Spells, NPC_UseFocusFromItems)){
					if (cur->Focus.Effect>0 && (cur->Focus.Type == EQEmu::item::ItemEffectFocus)){  // focus effects
						ApplySpellsBonuses(cur->Focus.Effect, cur->Focus.Level, newbon);
					}
				}

				if (cur->Haste > newbon->haste)
					newbon->haste = cur->Haste;
			}
		}

	}
}

void Client::CalcItemScale() {
	bool changed = false;

	// MainAmmo excluded in helper function below
	if (CalcItemScale(EQEmu::legacy::EQUIPMENT_BEGIN, EQEmu::legacy::EQUIPMENT_END)) // original coding excluded MainAmmo (< 21)
		changed = true;

	if (CalcItemScale(EQEmu::legacy::GENERAL_BEGIN, EQEmu::legacy::GENERAL_END)) // original coding excluded MainCursor (< 30)
		changed = true;

	// I excluded cursor bag slots here because cursor was excluded above..if this is incorrect, change 'slot_y' here to CURSOR_BAG_END
	// and 'slot_y' above to CURSOR from GENERAL_END above - or however it is supposed to be...
	if (CalcItemScale(EQEmu::legacy::GENERAL_BAGS_BEGIN, EQEmu::legacy::GENERAL_BAGS_END)) // (< 341)
		changed = true;

	if (CalcItemScale(EQEmu::legacy::TRIBUTE_BEGIN, EQEmu::legacy::TRIBUTE_END)) // (< 405)
		changed = true;

	//Power Source Slot
	if (ClientVersion() >= EQEmu::versions::ClientVersion::SoF)
	{
		if (CalcItemScale(EQEmu::inventory::slotPowerSource, EQEmu::inventory::slotPowerSource))
			changed = true;
	}

	if(changed)
	{
		CalcBonuses();
	}
}

bool Client::CalcItemScale(uint32 slot_x, uint32 slot_y) {
	// behavior change: 'slot_y' is now [RANGE]_END and not [RANGE]_END + 1
	bool changed = false;
	uint32 i;
	for (i = slot_x; i <= slot_y; i++) {
		if (i == EQEmu::inventory::slotAmmo) // moved here from calling procedure to facilitate future range changes where MainAmmo may not be the last slot
			continue;

		EQEmu::ItemInstance* inst = m_inv.GetItem(i);

		if(inst == nullptr)
			continue;

		// TEST CODE: test for bazaar trader crashing with charm items
		if (Trader)
			if (i >= EQEmu::legacy::GENERAL_BAGS_BEGIN && i <= EQEmu::legacy::GENERAL_BAGS_END) {
				EQEmu::ItemInstance* parent_item = m_inv.GetItem(EQEmu::InventoryProfile::CalcSlotId(i));
				if (parent_item && parent_item->GetItem()->ID == 17899) // trader satchel
					continue;
			}

		bool update_slot = false;
		if(inst->IsScaling())
		{
			uint16 oldexp = inst->GetExp();
			parse->EventItem(EVENT_SCALE_CALC, this, inst, nullptr, "", 0);

			if (inst->GetExp() != oldexp) {	// if the scaling factor changed, rescale the item and update the client
				inst->ScaleItem();
				changed = true;
				update_slot = true;
			}
		}

		//iterate all augments
		for (int x = EQEmu::inventory::socketBegin; x < EQEmu::inventory::SocketCount; ++x)
		{
			EQEmu::ItemInstance * a_inst = inst->GetAugment(x);
			if(!a_inst)
				continue;

			if(a_inst->IsScaling())
			{
				uint16 oldexp = a_inst->GetExp();
				parse->EventItem(EVENT_SCALE_CALC, this, a_inst, nullptr, "", 0);

				if (a_inst->GetExp() != oldexp)
				{
					a_inst->ScaleItem();
					changed = true;
					update_slot = true;
				}
			}
		}

		if(update_slot)
		{
			SendItemPacket(i, inst, ItemPacketCharmUpdate);
		}
	}
	return changed;
}

void Client::DoItemEnterZone() {
	bool changed = false;

	// MainAmmo excluded in helper function below
	if (DoItemEnterZone(EQEmu::legacy::EQUIPMENT_BEGIN, EQEmu::legacy::EQUIPMENT_END)) // original coding excluded MainAmmo (< 21)
		changed = true;

	if (DoItemEnterZone(EQEmu::legacy::GENERAL_BEGIN, EQEmu::legacy::GENERAL_END)) // original coding excluded MainCursor (< 30)
		changed = true;

	// I excluded cursor bag slots here because cursor was excluded above..if this is incorrect, change 'slot_y' here to CURSOR_BAG_END
	// and 'slot_y' above to CURSOR from GENERAL_END above - or however it is supposed to be...
	if (DoItemEnterZone(EQEmu::legacy::GENERAL_BAGS_BEGIN, EQEmu::legacy::GENERAL_BAGS_END)) // (< 341)
		changed = true;

	if (DoItemEnterZone(EQEmu::legacy::TRIBUTE_BEGIN, EQEmu::legacy::TRIBUTE_END)) // (< 405)
		changed = true;

	//Power Source Slot
	if (ClientVersion() >= EQEmu::versions::ClientVersion::SoF)
	{
		if (DoItemEnterZone(EQEmu::inventory::slotPowerSource, EQEmu::inventory::slotPowerSource))
			changed = true;
	}

	if(changed)
	{
		CalcBonuses();
	}
}

bool Client::DoItemEnterZone(uint32 slot_x, uint32 slot_y) {
	// behavior change: 'slot_y' is now [RANGE]_END and not [RANGE]_END + 1
	bool changed = false;
	for(uint32 i = slot_x; i <= slot_y; i++) {
		if (i == EQEmu::inventory::slotAmmo) // moved here from calling procedure to facilitate future range changes where MainAmmo may not be the last slot
			continue;

		EQEmu::ItemInstance* inst = m_inv.GetItem(i);

		if(!inst)
			continue;

		// TEST CODE: test for bazaar trader crashing with charm items
		if (Trader)
			if (i >= EQEmu::legacy::GENERAL_BAGS_BEGIN && i <= EQEmu::legacy::GENERAL_BAGS_END) {
				EQEmu::ItemInstance* parent_item = m_inv.GetItem(EQEmu::InventoryProfile::CalcSlotId(i));
				if (parent_item && parent_item->GetItem()->ID == 17899) // trader satchel
					continue;
			}

		bool update_slot = false;
		if(inst->IsScaling())
		{
			uint16 oldexp = inst->GetExp();

			parse->EventItem(EVENT_ITEM_ENTER_ZONE, this, inst, nullptr, "", 0);
			if (i <= EQEmu::inventory::slotAmmo || i == EQEmu::inventory::slotPowerSource) {
				parse->EventItem(EVENT_EQUIP_ITEM, this, inst, nullptr, "", i);
			}

			if (inst->GetExp() != oldexp) {	// if the scaling factor changed, rescale the item and update the client
				inst->ScaleItem();
				changed = true;
				update_slot = true;
			}
		} else {
			if (i <= EQEmu::inventory::slotAmmo || i == EQEmu::inventory::slotPowerSource) {
				parse->EventItem(EVENT_EQUIP_ITEM, this, inst, nullptr, "", i);
			}

			parse->EventItem(EVENT_ITEM_ENTER_ZONE, this, inst, nullptr, "", 0);
		}

		//iterate all augments
		for (int x = EQEmu::inventory::socketBegin; x < EQEmu::inventory::SocketCount; ++x)
		{
			EQEmu::ItemInstance *a_inst = inst->GetAugment(x);
			if(!a_inst)
				continue;

			if(a_inst->IsScaling())
			{
				uint16 oldexp = a_inst->GetExp();

				parse->EventItem(EVENT_ITEM_ENTER_ZONE, this, a_inst, nullptr, "", 0);

				if (a_inst->GetExp() != oldexp)
				{
					a_inst->ScaleItem();
					changed = true;
					update_slot = true;
				}
			} else {
				parse->EventItem(EVENT_ITEM_ENTER_ZONE, this, a_inst, nullptr, "", 0);
			}
		}

		if(update_slot)
		{
			SendItemPacket(i, inst, ItemPacketCharmUpdate);
		}
	}
	return changed;
}

uint8 Mob::IsFocusEffect(uint16 spell_id,int effect_index, bool AA,uint32 aa_effect)
{
	uint16 effect = 0;

	if (!AA)
		effect = spells[spell_id].effectid[effect_index];
	else
		effect = aa_effect;

	switch (effect)
	{
		case SE_ImprovedDamage:
			return focusImprovedDamage;
		case SE_ImprovedDamage2:
			return focusImprovedDamage2;
		case SE_ImprovedHeal:
			return focusImprovedHeal;
		case SE_ReduceManaCost:
			return focusManaCost;
		case SE_IncreaseSpellHaste:
			return focusSpellHaste;
		case SE_IncreaseSpellDuration:
			return focusSpellDuration;
		case SE_SpellDurationIncByTic:
			return focusSpellDurByTic;
		case SE_SwarmPetDuration:
			return focusSwarmPetDuration;
		case SE_IncreaseRange:
			return focusRange;
		case SE_ReduceReagentCost:
			return focusReagentCost;
		case SE_PetPowerIncrease:
			return focusPetPower;
		case SE_SpellResistReduction:
			return focusResistRate;
		case SE_SpellHateMod:
			return focusSpellHateMod;
		case SE_ReduceReuseTimer:
			return focusReduceRecastTime;
		case SE_TriggerOnCast:
			//return focusTriggerOnCast;
			return 0; //This is calculated as an actual bonus
		case SE_FcSpellVulnerability:
			return focusSpellVulnerability;
		case SE_BlockNextSpellFocus:
			//return focusBlockNextSpell;
			return 0; //This is calculated as an actual bonus
		case SE_FcTwincast:
			return focusTwincast;
		case SE_SympatheticProc:
			return focusSympatheticProc;
		case SE_FcDamageAmt:
			return focusFcDamageAmt;
		case SE_FcDamageAmt2:
			return focusFcDamageAmt2;
		case SE_FcDamageAmtCrit:
			return focusFcDamageAmtCrit;
		case SE_FcDamagePctCrit:
			return focusFcDamagePctCrit;
		case SE_FcDamageAmtIncoming:
			return focusFcDamageAmtIncoming;
		case SE_FcHealAmtIncoming:
			return focusFcHealAmtIncoming;
		case SE_FcHealPctIncoming:
			return focusFcHealPctIncoming;
		case SE_FcBaseEffects:
			return focusFcBaseEffects;
		case SE_FcIncreaseNumHits:
			return focusIncreaseNumHits;
		case SE_FcLimitUse:
			return focusFcLimitUse;
		case SE_FcMute:
			return focusFcMute;
		case SE_FcTimerRefresh:
			return focusFcTimerRefresh;
		case SE_FcStunTimeMod:
			return focusFcStunTimeMod;
		case SE_FcHealPctCritIncoming:
			return focusFcHealPctCritIncoming;
		case  SE_FcHealAmt:
			return focusFcHealAmt;
		case SE_FcHealAmtCrit:
			return focusFcHealAmtCrit;
	}
	return 0;
}

void Mob::NegateSpellsBonuses(uint16 spell_id)
{
	if(!IsValidSpell(spell_id))
		return;

	int effect_value = 0;

	for (int i = 0; i < EFFECT_COUNT; i++)
	{
		if (spells[spell_id].effectid[i] == SE_NegateSpellEffect){

			//Negate focus effects
			for(int e = 0; e < HIGHEST_FOCUS+1; e++)
			{
				if (spellbonuses.FocusEffects[e] == spells[spell_id].base2[i])
				{
					spellbonuses.FocusEffects[e] = effect_value;
					continue;
				}
			}

			//Negate bonuses
			switch (spells[spell_id].base2[i])
			{
				case SE_CurrentHP:
					if(spells[spell_id].base[i] == 1) {
						spellbonuses.HPRegen = effect_value;
						aabonuses.HPRegen = effect_value;
						itembonuses.HPRegen = effect_value;
					}
					break;

				case SE_CurrentEndurance:
					spellbonuses.EnduranceRegen = effect_value;
					aabonuses.EnduranceRegen = effect_value;
					itembonuses.EnduranceRegen = effect_value;
					break;

				case SE_ChangeFrenzyRad:
					spellbonuses.AggroRange = static_cast<float>(effect_value);
					aabonuses.AggroRange = static_cast<float>(effect_value);
					itembonuses.AggroRange = static_cast<float>(effect_value);
					break;

				case SE_Harmony:
					spellbonuses.AssistRange = static_cast<float>(effect_value);
					aabonuses.AssistRange = static_cast<float>(effect_value);
					itembonuses.AssistRange = static_cast<float>(effect_value);
					break;

				case SE_AttackSpeed:
					spellbonuses.haste = effect_value;
					aabonuses.haste = effect_value;
					itembonuses.haste = effect_value;
					break;

				case SE_AttackSpeed2:
					spellbonuses.hastetype2 = effect_value;
					aabonuses.hastetype2 = effect_value;
					itembonuses.hastetype2 = effect_value;
					break;

				case SE_AttackSpeed3:
				{
					if (effect_value > 0) {
						spellbonuses.hastetype3 = effect_value;
						aabonuses.hastetype3 = effect_value;
						itembonuses.hastetype3 = effect_value;

					}
					break;
				}

				case SE_AttackSpeed4:
					spellbonuses.inhibitmelee = effect_value;
					aabonuses.inhibitmelee = effect_value;
					itembonuses.inhibitmelee = effect_value;
					break;

				case SE_TotalHP:
					spellbonuses.HP = effect_value;
					aabonuses.HP = effect_value;
					itembonuses.HP = effect_value;
					break;

				case SE_ManaRegen_v2:
				case SE_CurrentMana:
					spellbonuses.ManaRegen = effect_value;
					aabonuses.ManaRegen = effect_value;
					itembonuses.ManaRegen = effect_value;
					break;

				case SE_ManaPool:
					spellbonuses.Mana = effect_value;
					itembonuses.Mana = effect_value;
					aabonuses.Mana = effect_value;
					break;

				case SE_Stamina:
					spellbonuses.EnduranceReduction = effect_value;
					itembonuses.EnduranceReduction = effect_value;
					aabonuses.EnduranceReduction = effect_value;
					break;

				case SE_ACv2:
				case SE_ArmorClass:
					spellbonuses.AC = effect_value;
					aabonuses.AC = effect_value;
					itembonuses.AC = effect_value;
					break;

				case SE_ATK:
					spellbonuses.ATK = effect_value;
					aabonuses.ATK = effect_value;
					itembonuses.ATK = effect_value;
					break;

				case SE_STR:
					spellbonuses.STR = effect_value;
					itembonuses.STR = effect_value;
					aabonuses.STR = effect_value;
					break;

				case SE_DEX:
					spellbonuses.DEX = effect_value;
					aabonuses.DEX = effect_value;
					itembonuses.DEX = effect_value;
					break;

				case SE_AGI:
					itembonuses.AGI = effect_value;
					aabonuses.AGI = effect_value;
					spellbonuses.AGI = effect_value;
					break;

				case SE_STA:
					spellbonuses.STA = effect_value;
					itembonuses.STA = effect_value;
					aabonuses.STA = effect_value;
					break;

				case SE_INT:
					spellbonuses.INT = effect_value;
					aabonuses.INT = effect_value;
					itembonuses.INT = effect_value;
					break;

				case SE_WIS:
					spellbonuses.WIS = effect_value;
					aabonuses.WIS = effect_value;
					itembonuses.WIS = effect_value;
					break;

				case SE_CHA:
					itembonuses.CHA = effect_value;
					spellbonuses.CHA = effect_value;
					aabonuses.CHA = effect_value;
					break;

				case SE_AllStats:
				{
					spellbonuses.STR = effect_value;
					spellbonuses.DEX = effect_value;
					spellbonuses.AGI = effect_value;
					spellbonuses.STA = effect_value;
					spellbonuses.INT = effect_value;
					spellbonuses.WIS = effect_value;
					spellbonuses.CHA = effect_value;

					itembonuses.STR = effect_value;
					itembonuses.DEX = effect_value;
					itembonuses.AGI = effect_value;
					itembonuses.STA = effect_value;
					itembonuses.INT = effect_value;
					itembonuses.WIS = effect_value;
					itembonuses.CHA = effect_value;

					aabonuses.STR = effect_value;
					aabonuses.DEX = effect_value;
					aabonuses.AGI = effect_value;
					aabonuses.STA = effect_value;
					aabonuses.INT = effect_value;
					aabonuses.WIS = effect_value;
					aabonuses.CHA = effect_value;
					break;
				}

				case SE_ResistFire:
					spellbonuses.FR = effect_value;
					itembonuses.FR = effect_value;
					aabonuses.FR = effect_value;
					break;

				case SE_ResistCold:
					spellbonuses.CR = effect_value;
					aabonuses.CR = effect_value;
					itembonuses.CR = effect_value;
					break;

				case SE_ResistPoison:
					spellbonuses.PR = effect_value;
					aabonuses.PR = effect_value;
					itembonuses.PR = effect_value;
					break;

				case SE_ResistDisease:
					spellbonuses.DR = effect_value;
					itembonuses.DR = effect_value;
					aabonuses.DR = effect_value;
					break;

				case SE_ResistMagic:
					spellbonuses.MR = effect_value;
					aabonuses.MR = effect_value;
					itembonuses.MR = effect_value;
					break;

				case SE_ResistAll:
				{
					spellbonuses.MR = effect_value;
					spellbonuses.DR = effect_value;
					spellbonuses.PR = effect_value;
					spellbonuses.CR = effect_value;
					spellbonuses.FR = effect_value;

					aabonuses.MR = effect_value;
					aabonuses.DR = effect_value;
					aabonuses.PR = effect_value;
					aabonuses.CR = effect_value;
					aabonuses.FR = effect_value;

					itembonuses.MR = effect_value;
					itembonuses.DR = effect_value;
					itembonuses.PR = effect_value;
					itembonuses.CR = effect_value;
					itembonuses.FR = effect_value;
					break;
				}

				case SE_ResistCorruption:
					spellbonuses.Corrup = effect_value;
					itembonuses.Corrup = effect_value;
					aabonuses.Corrup = effect_value;
					break;

				case SE_CastingLevel2:
				case SE_CastingLevel:	// Brilliance of Ro
					spellbonuses.effective_casting_level = effect_value;
					aabonuses.effective_casting_level = effect_value;
					itembonuses.effective_casting_level = effect_value;
					break;


				case SE_MovementSpeed:
					spellbonuses.movementspeed = effect_value;
					aabonuses.movementspeed = effect_value;
					itembonuses.movementspeed = effect_value;
					break;

				case SE_SpellDamageShield:
					spellbonuses.SpellDamageShield = effect_value;
					aabonuses.SpellDamageShield = effect_value;
					itembonuses.SpellDamageShield = effect_value;
					break;

				case SE_DamageShield:
					spellbonuses.DamageShield = effect_value;
					aabonuses.DamageShield = effect_value;
					itembonuses.DamageShield = effect_value;
					break;

				case SE_ReverseDS:
					spellbonuses.ReverseDamageShield = effect_value;
					aabonuses.ReverseDamageShield = effect_value;
					itembonuses.ReverseDamageShield = effect_value;
					break;

				case SE_Reflect:
					spellbonuses.reflect_chance = effect_value;
					aabonuses.reflect_chance = effect_value;
					itembonuses.reflect_chance = effect_value;
					break;

				case SE_Amplification:
					spellbonuses.Amplification = effect_value;
					itembonuses.Amplification = effect_value;
					aabonuses.Amplification = effect_value;
					break;

				case SE_ChangeAggro:
					spellbonuses.hatemod = effect_value;
					itembonuses.hatemod = effect_value;
					aabonuses.hatemod = effect_value;
					break;

				case SE_MeleeMitigation:
					spellbonuses.MeleeMitigationEffect = effect_value;
					itembonuses.MeleeMitigationEffect = effect_value;
					aabonuses.MeleeMitigationEffect = effect_value;
					break;

				case SE_CriticalHitChance:
				{
					for (int e = 0; e < EQEmu::skills::HIGHEST_SKILL + 1; e++)
					{
						spellbonuses.CriticalHitChance[e] = effect_value;
						aabonuses.CriticalHitChance[e] = effect_value;
						itembonuses.CriticalHitChance[e] = effect_value;
					}
				}

				case SE_CrippBlowChance:
					spellbonuses.CrippBlowChance = effect_value;
					aabonuses.CrippBlowChance = effect_value;
					itembonuses.CrippBlowChance = effect_value;
					break;

				case SE_AvoidMeleeChance:
					spellbonuses.AvoidMeleeChanceEffect = effect_value;
					aabonuses.AvoidMeleeChanceEffect = effect_value;
					itembonuses.AvoidMeleeChanceEffect = effect_value;
					break;

				case SE_RiposteChance:
					spellbonuses.RiposteChance = effect_value;
					aabonuses.RiposteChance = effect_value;
					itembonuses.RiposteChance = effect_value;
					break;

				case SE_DodgeChance:
					spellbonuses.DodgeChance = effect_value;
					aabonuses.DodgeChance = effect_value;
					itembonuses.DodgeChance = effect_value;
					break;

				case SE_ParryChance:
					spellbonuses.ParryChance = effect_value;
					aabonuses.ParryChance = effect_value;
					itembonuses.ParryChance = effect_value;
					break;

				case SE_DualWieldChance:
					spellbonuses.DualWieldChance = effect_value;
					aabonuses.DualWieldChance = effect_value;
					itembonuses.DualWieldChance = effect_value;
					break;

				case SE_DoubleAttackChance:
					spellbonuses.DoubleAttackChance = effect_value;
					aabonuses.DoubleAttackChance = effect_value;
					itembonuses.DoubleAttackChance = effect_value;
					break;

				case SE_TripleAttackChance:
					spellbonuses.TripleAttackChance = effect_value;
					aabonuses.TripleAttackChance = effect_value;
					itembonuses.TripleAttackChance = effect_value;
					break;

				case SE_MeleeLifetap:
					spellbonuses.MeleeLifetap = effect_value;
					aabonuses.MeleeLifetap = effect_value;
					itembonuses.MeleeLifetap = effect_value;
					break;

				case SE_AllInstrumentMod:
				{
					spellbonuses.singingMod = effect_value;
					spellbonuses.brassMod = effect_value;
					spellbonuses.percussionMod = effect_value;
					spellbonuses.windMod = effect_value;
					spellbonuses.stringedMod = effect_value;

					itembonuses.singingMod = effect_value;
					itembonuses.brassMod = effect_value;
					itembonuses.percussionMod = effect_value;
					itembonuses.windMod = effect_value;
					itembonuses.stringedMod = effect_value;

					aabonuses.singingMod = effect_value;
					aabonuses.brassMod = effect_value;
					aabonuses.percussionMod = effect_value;
					aabonuses.windMod = effect_value;
					aabonuses.stringedMod = effect_value;
					break;
				}

				case SE_ResistSpellChance:
					spellbonuses.ResistSpellChance = effect_value;
					aabonuses.ResistSpellChance = effect_value;
					itembonuses.ResistSpellChance = effect_value;
					break;

				case SE_ResistFearChance:
					spellbonuses.Fearless = false;
					spellbonuses.ResistFearChance = effect_value;
					aabonuses.ResistFearChance = effect_value;
					itembonuses.ResistFearChance = effect_value;
					break;

				case SE_Fearless:
					spellbonuses.Fearless = false;
					aabonuses.Fearless = false;
					itembonuses.Fearless = false;
					break;

				case SE_HundredHands:
					spellbonuses.HundredHands = effect_value;
					aabonuses.HundredHands = effect_value;
					itembonuses.HundredHands = effect_value;
					break;

				case SE_MeleeSkillCheck:
				{
					spellbonuses.MeleeSkillCheck = effect_value;
					spellbonuses.MeleeSkillCheckSkill = effect_value;
					break;
				}

				case SE_HitChance:
				{
					for (int e = 0; e < EQEmu::skills::HIGHEST_SKILL + 1; e++)
					{
						spellbonuses.HitChanceEffect[e] = effect_value;
						aabonuses.HitChanceEffect[e] = effect_value;
						itembonuses.HitChanceEffect[e] = effect_value;
					}
					break;
				}

				case SE_DamageModifier:
				{
					for (int e = 0; e < EQEmu::skills::HIGHEST_SKILL + 1; e++)
					{
						spellbonuses.DamageModifier[e] = effect_value;
						aabonuses.DamageModifier[e] = effect_value;
						itembonuses.DamageModifier[e] = effect_value;
					}
					break;
				}

				case SE_DamageModifier2:
				{
					for (int e = 0; e < EQEmu::skills::HIGHEST_SKILL + 1; e++)
					{
						spellbonuses.DamageModifier2[e] = effect_value;
						aabonuses.DamageModifier2[e] = effect_value;
						itembonuses.DamageModifier2[e] = effect_value;
					}
					break;
				}

				case SE_MinDamageModifier:
				{
					for (int e = 0; e < EQEmu::skills::HIGHEST_SKILL + 1; e++)
					{
						spellbonuses.MinDamageModifier[e] = effect_value;
						aabonuses.MinDamageModifier[e] = effect_value;
						itembonuses.MinDamageModifier[e] = effect_value;
					}
					break;
				}

				case SE_StunResist:
					spellbonuses.StunResist = effect_value;
					aabonuses.StunResist = effect_value;
					itembonuses.StunResist = effect_value;
					break;

				case SE_ProcChance:
					spellbonuses.ProcChanceSPA = effect_value;
					aabonuses.ProcChanceSPA = effect_value;
					itembonuses.ProcChanceSPA = effect_value;
					break;

				case SE_ExtraAttackChance:
					spellbonuses.ExtraAttackChance = effect_value;
					aabonuses.ExtraAttackChance = effect_value;
					itembonuses.ExtraAttackChance = effect_value;
					break;

				case SE_PercentXPIncrease:
					spellbonuses.XPRateMod = effect_value;
					aabonuses.XPRateMod = effect_value;
					itembonuses.XPRateMod = effect_value;
					break;

				case SE_Flurry:
					spellbonuses.FlurryChance = effect_value;
					aabonuses.FlurryChance = effect_value;
					itembonuses.FlurryChance = effect_value;
					break;

				case SE_Accuracy:
				{
					spellbonuses.Accuracy[EQEmu::skills::HIGHEST_SKILL + 1] = effect_value;
					itembonuses.Accuracy[EQEmu::skills::HIGHEST_SKILL + 1] = effect_value;

					for (int e = 0; e < EQEmu::skills::HIGHEST_SKILL + 1; e++)
						{
							aabonuses.Accuracy[e] = effect_value;
						}
					break;
				}

				case SE_MaxHPChange:
					spellbonuses.MaxHPChange = effect_value;
					aabonuses.MaxHPChange = effect_value;
					itembonuses.MaxHPChange = effect_value;
					break;

				case SE_EndurancePool:
					spellbonuses.Endurance = effect_value;
					aabonuses.Endurance = effect_value;
					itembonuses.Endurance = effect_value;
					break;

				case SE_HealRate:
					spellbonuses.HealRate = effect_value;
					aabonuses.HealRate = effect_value;
					itembonuses.HealRate = effect_value;
					break;

				case SE_SkillDamageTaken:
				{
					for (int e = 0; e < EQEmu::skills::HIGHEST_SKILL + 1; e++)
					{
						spellbonuses.SkillDmgTaken[e] = effect_value;
						aabonuses.SkillDmgTaken[e] = effect_value;
						itembonuses.SkillDmgTaken[e] = effect_value;

					}
					break;
				}

				case SE_TriggerOnCast:
				{
					for(int e = 0; e < MAX_SPELL_TRIGGER; e++)
					{
						spellbonuses.SpellTriggers[e] = effect_value;
						aabonuses.SpellTriggers[e] = effect_value;
						itembonuses.SpellTriggers[e] = effect_value;
					}
					break;
				}

				case SE_SpellCritChance:
					spellbonuses.CriticalSpellChance = effect_value;
					aabonuses.CriticalSpellChance = effect_value;
					itembonuses.CriticalSpellChance = effect_value;
					break;

				case SE_CriticalSpellChance:
					spellbonuses.CriticalSpellChance = effect_value;
					spellbonuses.SpellCritDmgIncrease = effect_value;
					aabonuses.CriticalSpellChance = effect_value;
					aabonuses.SpellCritDmgIncrease = effect_value;
					itembonuses.CriticalSpellChance = effect_value;
					itembonuses.SpellCritDmgIncrease = effect_value;
					break;

				case SE_SpellCritDmgIncrease:
					spellbonuses.SpellCritDmgIncrease = effect_value;
					aabonuses.SpellCritDmgIncrease = effect_value;
					itembonuses.SpellCritDmgIncrease = effect_value;
					break;

				case SE_DotCritDmgIncrease:
					spellbonuses.DotCritDmgIncrease = effect_value;
					aabonuses.DotCritDmgIncrease = effect_value;
					itembonuses.DotCritDmgIncrease = effect_value;
					break;

				case SE_CriticalHealChance:
					spellbonuses.CriticalHealChance = effect_value;
					aabonuses.CriticalHealChance = effect_value;
					itembonuses.CriticalHealChance = effect_value;
					break;

				case SE_CriticalHealOverTime:
					spellbonuses.CriticalHealOverTime = effect_value;
					aabonuses.CriticalHealOverTime = effect_value;
					itembonuses.CriticalHealOverTime = effect_value;
					break;

				case SE_MitigateDamageShield:
					spellbonuses.DSMitigationOffHand = effect_value;
					itembonuses.DSMitigationOffHand = effect_value;
					aabonuses.DSMitigationOffHand = effect_value;
					break;

				case SE_CriticalDoTChance:
					spellbonuses.CriticalDoTChance = effect_value;
					aabonuses.CriticalDoTChance = effect_value;
					itembonuses.CriticalDoTChance = effect_value;
					break;

				case SE_ProcOnKillShot:
				{
					for(int e = 0; e < MAX_SPELL_TRIGGER*3; e=3)
					{
						spellbonuses.SpellOnKill[e] = effect_value;
						spellbonuses.SpellOnKill[e+1] = effect_value;
						spellbonuses.SpellOnKill[e+2] = effect_value;

						aabonuses.SpellOnKill[e] = effect_value;
						aabonuses.SpellOnKill[e+1] = effect_value;
						aabonuses.SpellOnKill[e+2] = effect_value;

						itembonuses.SpellOnKill[e] = effect_value;
						itembonuses.SpellOnKill[e+1] = effect_value;
						itembonuses.SpellOnKill[e+2] = effect_value;
					}
					break;
				}

				/*
				case SE_SpellOnDeath:
				{
					for(int e = 0; e < MAX_SPELL_TRIGGER; e=2)
					{
						spellbonuses.SpellOnDeath[e] = SPELL_UNKNOWN;
						spellbonuses.SpellOnDeath[e+1] = effect_value;
					}
					break;
				}
				*/

				case SE_CriticalDamageMob:
				{
					for (int e = 0; e < EQEmu::skills::HIGHEST_SKILL + 1; e++)
					{
						spellbonuses.CritDmgMob[e] = effect_value;
						aabonuses.CritDmgMob[e] = effect_value;
						itembonuses.CritDmgMob[e] = effect_value;
					}
					break;
				}

				case SE_SkillDamageAmount:
				{
					for (int e = 0; e < EQEmu::skills::HIGHEST_SKILL + 1; e++)
					{
						spellbonuses.SkillDamageAmount[e] = effect_value;
						aabonuses.SkillDamageAmount[e] = effect_value;
						itembonuses.SkillDamageAmount[e] = effect_value;
					}
					break;
				}

				case SE_IncreaseBlockChance:
					spellbonuses.IncreaseBlockChance = effect_value;
					aabonuses.IncreaseBlockChance = effect_value;
					itembonuses.IncreaseBlockChance = effect_value;
					break;

				case SE_PersistantCasting:
					spellbonuses.PersistantCasting = effect_value;
					itembonuses.PersistantCasting = effect_value;
					aabonuses.PersistantCasting = effect_value;
					break;

				case SE_ImmuneFleeing:
					spellbonuses.ImmuneToFlee = false;
					break;

				case SE_DelayDeath:
					spellbonuses.DelayDeath = effect_value;
					aabonuses.DelayDeath = effect_value;
					itembonuses.DelayDeath = effect_value;
					break;

				case SE_SpellProcChance:
					spellbonuses.SpellProcChance = effect_value;
					itembonuses.SpellProcChance = effect_value;
					aabonuses.SpellProcChance = effect_value;
					break;

				case SE_CharmBreakChance:
					spellbonuses.CharmBreakChance = effect_value;
					aabonuses.CharmBreakChance = effect_value;
					itembonuses.CharmBreakChance = effect_value;
					break;

				case SE_BardSongRange:
					spellbonuses.SongRange = effect_value;
					aabonuses.SongRange = effect_value;
					itembonuses.SongRange = effect_value;
					break;

				case SE_SkillDamageAmount2:
				{
					for (int e = 0; e < EQEmu::skills::HIGHEST_SKILL + 1; e++)
					{
						spellbonuses.SkillDamageAmount2[e] = effect_value;
						aabonuses.SkillDamageAmount2[e] = effect_value;
						itembonuses.SkillDamageAmount2[e] = effect_value;
					}
					break;
				}

				case SE_NegateAttacks:
					spellbonuses.NegateAttacks[0] = effect_value;
					spellbonuses.NegateAttacks[1] = effect_value;
					break;

				case SE_MitigateMeleeDamage:
					spellbonuses.MitigateMeleeRune[0] = effect_value;
					spellbonuses.MitigateMeleeRune[1] = -1;
					break;

				case SE_MeleeThresholdGuard:
					spellbonuses.MeleeThresholdGuard[0] = effect_value;
					spellbonuses.MeleeThresholdGuard[1] = -1;
					spellbonuses.MeleeThresholdGuard[1] = effect_value;
					break;

				case SE_SpellThresholdGuard:
					spellbonuses.SpellThresholdGuard[0] = effect_value;
					spellbonuses.SpellThresholdGuard[1] = -1;
					spellbonuses.SpellThresholdGuard[1] = effect_value;
					break;

				case SE_MitigateSpellDamage:
					spellbonuses.MitigateSpellRune[0] = effect_value;
					spellbonuses.MitigateSpellRune[1] = -1;
					break;

				case SE_MitigateDotDamage:
					spellbonuses.MitigateDotRune[0] = effect_value;
					spellbonuses.MitigateDotRune[1] = -1;
					break;

				case SE_ManaAbsorbPercentDamage:
					spellbonuses.ManaAbsorbPercentDamage[0] = effect_value;
					spellbonuses.ManaAbsorbPercentDamage[1] = -1;
					break;

				case SE_ShieldBlock:
					spellbonuses.ShieldBlock = effect_value;
					aabonuses.ShieldBlock = effect_value;
					itembonuses.ShieldBlock = effect_value;

				case SE_BlockBehind:
					spellbonuses.BlockBehind = effect_value;
					aabonuses.BlockBehind = effect_value;
					itembonuses.BlockBehind = effect_value;
					break;

				case SE_Blind:
					spellbonuses.IsBlind = false;
					break;

				case SE_Fear:
					spellbonuses.IsFeared = false;
					break;

				case SE_FrontalStunResist:
					spellbonuses.FrontalStunResist = effect_value;
					aabonuses.FrontalStunResist = effect_value;
					itembonuses.FrontalStunResist = effect_value;
					break;

				case SE_ImprovedBindWound:
					aabonuses.BindWound = effect_value;
					itembonuses.BindWound = effect_value;
					spellbonuses.BindWound = effect_value;
					break;

				case SE_MaxBindWound:
					spellbonuses.MaxBindWound = effect_value;
					aabonuses.MaxBindWound = effect_value;
					itembonuses.MaxBindWound = effect_value;
					break;

				case SE_BaseMovementSpeed:
					spellbonuses.BaseMovementSpeed = effect_value;
					aabonuses.BaseMovementSpeed = effect_value;
					itembonuses.BaseMovementSpeed = effect_value;
					break;

				case SE_IncreaseRunSpeedCap:
					itembonuses.IncreaseRunSpeedCap = effect_value;
					aabonuses.IncreaseRunSpeedCap = effect_value;
					spellbonuses.IncreaseRunSpeedCap = effect_value;
					break;

				case SE_DoubleSpecialAttack:
					spellbonuses.DoubleSpecialAttack = effect_value;
					aabonuses.DoubleSpecialAttack = effect_value;
					itembonuses.DoubleSpecialAttack = effect_value;
					break;

				case SE_TripleBackstab:
					spellbonuses.TripleBackstab = effect_value;
					aabonuses.TripleBackstab = effect_value;
					itembonuses.TripleBackstab = effect_value;
					break;

				case SE_FrontalBackstabMinDmg:
					spellbonuses.FrontalBackstabMinDmg = false;
					break;

				case SE_FrontalBackstabChance:
					spellbonuses.FrontalBackstabChance = effect_value;
					aabonuses.FrontalBackstabChance = effect_value;
					itembonuses.FrontalBackstabChance = effect_value;
					break;

				case SE_ConsumeProjectile:
					spellbonuses.ConsumeProjectile = effect_value;
					aabonuses.ConsumeProjectile = effect_value;
					itembonuses.ConsumeProjectile = effect_value;
					break;

				case SE_ForageAdditionalItems:
					spellbonuses.ForageAdditionalItems = effect_value;
					aabonuses.ForageAdditionalItems = effect_value;
					itembonuses.ForageAdditionalItems = effect_value;
					break;

				case SE_Salvage:
					spellbonuses.SalvageChance = effect_value;
					aabonuses.SalvageChance = effect_value;
					itembonuses.SalvageChance = effect_value;
					break;

				case SE_ArcheryDamageModifier:
					spellbonuses.ArcheryDamageModifier = effect_value;
					aabonuses.ArcheryDamageModifier = effect_value;
					itembonuses.ArcheryDamageModifier = effect_value;
					break;

				case SE_SecondaryDmgInc:
					spellbonuses.SecondaryDmgInc = false;
					aabonuses.SecondaryDmgInc = false;
					itembonuses.SecondaryDmgInc = false;
					break;

				case SE_StrikeThrough:
					spellbonuses.StrikeThrough = effect_value;
					aabonuses.StrikeThrough = effect_value;
					itembonuses.StrikeThrough = effect_value;
					break;

				case SE_StrikeThrough2:
					spellbonuses.StrikeThrough = effect_value;
					aabonuses.StrikeThrough = effect_value;
					itembonuses.StrikeThrough = effect_value;
					break;

				case SE_GiveDoubleAttack:
					spellbonuses.GiveDoubleAttack = effect_value;
					aabonuses.GiveDoubleAttack = effect_value;
					itembonuses.GiveDoubleAttack = effect_value;
					break;

				case SE_PetCriticalHit:
					spellbonuses.PetCriticalHit = effect_value;
					aabonuses.PetCriticalHit = effect_value;
					itembonuses.PetCriticalHit = effect_value;
					break;

				case SE_CombatStability:
					spellbonuses.CombatStability = effect_value;
					aabonuses.CombatStability = effect_value;
					itembonuses.CombatStability = effect_value;
					break;

				case SE_PetAvoidance:
					spellbonuses.PetAvoidance = effect_value;
					aabonuses.PetAvoidance = effect_value;
					itembonuses.PetAvoidance = effect_value;
					break;

				case SE_Ambidexterity:
					spellbonuses.Ambidexterity = effect_value;
					aabonuses.Ambidexterity = effect_value;
					itembonuses.Ambidexterity = effect_value;
					break;

				case SE_PetMaxHP:
					spellbonuses.PetMaxHP = effect_value;
					aabonuses.PetMaxHP = effect_value;
					itembonuses.PetMaxHP = effect_value;
					break;

				case SE_PetFlurry:
					spellbonuses.PetFlurry = effect_value;
					aabonuses.PetFlurry = effect_value;
					itembonuses.PetFlurry = effect_value;
					break;

				case SE_GivePetGroupTarget:
					spellbonuses.GivePetGroupTarget = false;
					aabonuses.GivePetGroupTarget = false;
					itembonuses.GivePetGroupTarget = false;
					break;

				case SE_PetMeleeMitigation:
					spellbonuses.PetMeleeMitigation = effect_value;
					itembonuses.PetMeleeMitigation = effect_value;
					aabonuses.PetMeleeMitigation = effect_value;
					break;

				case SE_RootBreakChance:
					spellbonuses.RootBreakChance = effect_value;
					aabonuses.RootBreakChance = effect_value;
					itembonuses.RootBreakChance = effect_value;
					break;

				case SE_ChannelChanceItems:
					spellbonuses.ChannelChanceItems = effect_value;
					aabonuses.ChannelChanceItems = effect_value;
					itembonuses.ChannelChanceItems = effect_value;
					break;

				case SE_ChannelChanceSpells:
					spellbonuses.ChannelChanceSpells = effect_value;
					aabonuses.ChannelChanceSpells = effect_value;
					itembonuses.ChannelChanceSpells = effect_value;
					break;

				case SE_UnfailingDivinity:
					spellbonuses.UnfailingDivinity = effect_value;
					aabonuses.UnfailingDivinity = effect_value;
					itembonuses.UnfailingDivinity = effect_value;
					break;

				case SE_ItemHPRegenCapIncrease:
					spellbonuses.ItemHPRegenCap = effect_value;
					aabonuses.ItemHPRegenCap = effect_value;
					itembonuses.ItemHPRegenCap = effect_value;
					break;

				case SE_OffhandRiposteFail:
					spellbonuses.OffhandRiposteFail = effect_value;
					aabonuses.OffhandRiposteFail = effect_value;
					itembonuses.OffhandRiposteFail = effect_value;
					break;

				case SE_ItemAttackCapIncrease:
					aabonuses.ItemATKCap = effect_value;
					itembonuses.ItemATKCap = effect_value;
					spellbonuses.ItemATKCap = effect_value;
					break;

				case SE_SpellEffectResistChance:
				{
					for(int e = 0; e < MAX_RESISTABLE_EFFECTS*2; e+=2)
					{
						spellbonuses.SEResist[e] = effect_value;
						spellbonuses.SEResist[e+1] = effect_value;
					}
					break;
				}

				case SE_MasteryofPast:
					spellbonuses.MasteryofPast = effect_value;
					aabonuses.MasteryofPast = effect_value;
					itembonuses.MasteryofPast = effect_value;
					break;

				case SE_DoubleRiposte:
					spellbonuses.DoubleRiposte = effect_value;
					itembonuses.DoubleRiposte = effect_value;
					aabonuses.DoubleRiposte = effect_value;
					break;

				case SE_GiveDoubleRiposte:
					spellbonuses.GiveDoubleRiposte[0] = effect_value;
					itembonuses.GiveDoubleRiposte[0] = effect_value;
					aabonuses.GiveDoubleRiposte[0] = effect_value;
					break;

				case SE_SlayUndead:
					spellbonuses.SlayUndead[0] = effect_value;
					spellbonuses.SlayUndead[1] = effect_value;
					itembonuses.SlayUndead[0] = effect_value;
					itembonuses.SlayUndead[1] = effect_value;
					aabonuses.SlayUndead[0] = effect_value;
					aabonuses.SlayUndead[1] = effect_value;
					break;

				case SE_DoubleRangedAttack:
					spellbonuses.DoubleRangedAttack = effect_value;
					aabonuses.DoubleRangedAttack = effect_value;
					itembonuses.DoubleRangedAttack = effect_value;
					break;

				case SE_ShieldEquipDmgMod:
					spellbonuses.ShieldEquipDmgMod = effect_value;
					aabonuses.ShieldEquipDmgMod = effect_value;
					itembonuses.ShieldEquipDmgMod = effect_value;
					break;

				case SE_TriggerMeleeThreshold:
					spellbonuses.TriggerMeleeThreshold = false;
					break;

				case SE_TriggerSpellThreshold:
					spellbonuses.TriggerSpellThreshold = false;
					break;

				case SE_DivineAura:
					spellbonuses.DivineAura = false;
					break;

				case SE_StunBashChance:
					spellbonuses.StunBashChance = effect_value;
					itembonuses.StunBashChance = effect_value;
					aabonuses.StunBashChance = effect_value;
					break;

				case SE_IncreaseChanceMemwipe:
					spellbonuses.IncreaseChanceMemwipe = effect_value;
					itembonuses.IncreaseChanceMemwipe = effect_value;
					aabonuses.IncreaseChanceMemwipe = effect_value;
					break;

				case SE_CriticalMend:
					spellbonuses.CriticalMend = effect_value;
					itembonuses.CriticalMend = effect_value;
					aabonuses.CriticalMend = effect_value;
					break;

				case SE_DistanceRemoval:
					spellbonuses.DistanceRemoval = false;
					break;

				case SE_ImprovedTaunt:
					spellbonuses.ImprovedTaunt[0] = effect_value;
					spellbonuses.ImprovedTaunt[1] = effect_value;
					spellbonuses.ImprovedTaunt[2] = -1;
					break;

				case SE_FrenziedDevastation:
					spellbonuses.FrenziedDevastation = effect_value;
					aabonuses.FrenziedDevastation = effect_value;
					itembonuses.FrenziedDevastation = effect_value;
					break;

				case SE_Root:
					spellbonuses.Root[0] = effect_value;
					spellbonuses.Root[1] = -1;
					break;

				case SE_Rune:
					spellbonuses.MeleeRune[0] = effect_value;
					spellbonuses.MeleeRune[1] = -1;
					break;

				case SE_AbsorbMagicAtt:
					spellbonuses.AbsorbMagicAtt[0] = effect_value;
					spellbonuses.AbsorbMagicAtt[1] = -1;
					break;

				case SE_Berserk:
					spellbonuses.BerserkSPA = false;
					aabonuses.BerserkSPA = false;
					itembonuses.BerserkSPA = false;
					break;

				case SE_Vampirism:
					spellbonuses.Vampirism = effect_value;
					aabonuses.Vampirism = effect_value;
					itembonuses.Vampirism = effect_value;
					break;

				case SE_Metabolism:
					spellbonuses.Metabolism = effect_value;
					aabonuses.Metabolism = effect_value;
					itembonuses.Metabolism = effect_value;
					break;

				case SE_ImprovedReclaimEnergy:
					spellbonuses.ImprovedReclaimEnergy = effect_value;
					aabonuses.ImprovedReclaimEnergy = effect_value;
					itembonuses.ImprovedReclaimEnergy = effect_value;
					break;

				case SE_HeadShot:
					spellbonuses.HeadShot[0] = effect_value;
					aabonuses.HeadShot[0] = effect_value;
					itembonuses.HeadShot[0] = effect_value;
					spellbonuses.HeadShot[1] = effect_value;
					aabonuses.HeadShot[1] = effect_value;
					itembonuses.HeadShot[1] = effect_value;
					break;

				case SE_HeadShotLevel:
					spellbonuses.HSLevel[0] = effect_value;
					aabonuses.HSLevel[0] = effect_value;
					itembonuses.HSLevel[0] = effect_value;
					spellbonuses.HSLevel[1] = effect_value;
					aabonuses.HSLevel[1] = effect_value;
					itembonuses.HSLevel[1] = effect_value;
					break;

				case SE_Assassinate:
					spellbonuses.Assassinate[0] = effect_value;
					aabonuses.Assassinate[0] = effect_value;
					itembonuses.Assassinate[0] = effect_value;
					spellbonuses.Assassinate[1] = effect_value;
					aabonuses.Assassinate[1] = effect_value;
					itembonuses.Assassinate[1] = effect_value;
					break;

				case SE_AssassinateLevel:
					spellbonuses.AssassinateLevel[0] = effect_value;
					aabonuses.AssassinateLevel[0] = effect_value;
					itembonuses.AssassinateLevel[0] = effect_value;
					spellbonuses.AssassinateLevel[1] = effect_value;
					aabonuses.AssassinateLevel[1] = effect_value;
					itembonuses.AssassinateLevel[1] = effect_value;
					break;

				case SE_FinishingBlow:
					spellbonuses.FinishingBlow[0] = effect_value;
					aabonuses.FinishingBlow[0] = effect_value;
					itembonuses.FinishingBlow[0] = effect_value;
					spellbonuses.FinishingBlow[1] = effect_value;
					aabonuses.FinishingBlow[1] = effect_value;
					itembonuses.FinishingBlow[1] = effect_value;
					break;

				case SE_FinishingBlowLvl:
					spellbonuses.FinishingBlowLvl[0] = effect_value;
					aabonuses.FinishingBlowLvl[0] = effect_value;
					itembonuses.FinishingBlowLvl[0] = effect_value;
					spellbonuses.FinishingBlowLvl[1] = effect_value;
					aabonuses.FinishingBlowLvl[1] = effect_value;
					itembonuses.FinishingBlowLvl[1] = effect_value;
					break;

				case SE_Sanctuary:
					spellbonuses.Sanctuary = false;
					break;

				case SE_FactionModPct:
					spellbonuses.FactionModPct = effect_value;
					itembonuses.FactionModPct = effect_value;
					aabonuses.FactionModPct = effect_value;
					break;

				case SE_IllusionPersistence:
					spellbonuses.IllusionPersistence = false;
					itembonuses.IllusionPersistence = false;
					aabonuses.IllusionPersistence = false;
					break;

				case SE_SkillProcSuccess:{
					for(int e = 0; e < MAX_SKILL_PROCS; e++)
					{
						spellbonuses.SkillProcSuccess[e] = effect_value;
						itembonuses.SkillProcSuccess[e] = effect_value;
						aabonuses.SkillProcSuccess[e] = effect_value;
					}
				 }

				case SE_SkillProc:{
					for(int e = 0; e < MAX_SKILL_PROCS; e++)
					{
						spellbonuses.SkillProc[e] = effect_value;
						itembonuses.SkillProc[e] = effect_value;
						aabonuses.SkillProc[e] = effect_value;
					}
				 }
			}
		}
	}
}

