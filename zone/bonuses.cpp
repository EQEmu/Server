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

	/* Fast walking NPC's are prone to disappear into walls/hills
		We set this here because NPC's can cast spells to change walkspeed/runspeed
	*/
	float get_walk_speed = static_cast<float>(0.025f * this->GetWalkspeed());
	rooted = FindType(SE_Root);
}

void NPC::CalcBonuses()
{
	memset(&itembonuses, 0, sizeof(StatBonuses));
	if (RuleB(NPC, UseItemBonusesForNonPets)) {
		memset(&itembonuses, 0, sizeof(StatBonuses));
		CalcItemBonuses(&itembonuses);
	}
	else {
		if (GetOwner()) {
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

	// hmm maybe a better way to do this
	int metabolism = spellbonuses.Metabolism + itembonuses.Metabolism + aabonuses.Metabolism;
	int timer = GetClass() == MONK ? CONSUMPTION_MNK_TIMER : CONSUMPTION_TIMER;
	timer = timer * (100 + metabolism) / 100;
	if (timer != consume_food_timer.GetTimerTime())
		consume_food_timer.SetTimer(timer);
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
	SetDuelWeaponsEquiped(false);

	unsigned int i;
	// Update: MainAmmo should only calc skill mods (TODO: Check for other cases)
	for (i = EQ::invslot::BONUS_BEGIN; i <= EQ::invslot::BONUS_SKILL_END; i++) {
		const EQ::ItemInstance* inst = m_inv[i];
		if(inst == 0)
			continue;
		AddItemBonuses(inst, newbon, false, false, 0, (i == EQ::invslot::slotAmmo));

		//These are given special flags due to how often they are checked for various spell effects.
		const EQ::ItemData *item = inst->GetItem();
		if (i == EQ::invslot::slotSecondary && (item && item->ItemType == EQ::item::ItemTypeShield))
			SetShieldEquiped(true);
		else if (i == EQ::invslot::slotPrimary && (item && item->ItemType == EQ::item::ItemType2HBlunt)) {
			SetTwoHandBluntEquiped(true);
			SetTwoHanderEquipped(true);
		}
		else if (i == EQ::invslot::slotPrimary && (item && (item->ItemType == EQ::item::ItemType2HSlash || item->ItemType == EQ::item::ItemType2HPiercing))) {
			SetTwoHanderEquipped(true);
		}
	}

	if (CanThisClassDualWield()) {
		SetDuelWeaponsEquiped(true);
	}

	//tribute items
	for (i = EQ::invslot::TRIBUTE_BEGIN; i <= EQ::invslot::TRIBUTE_END; i++) {
		const EQ::ItemInstance* inst = m_inv[i];
		if(inst == 0)
			continue;
		AddItemBonuses(inst, newbon, false, true);
	}

	//Optional ability to have worn effects calculate as an addititive bonus instead of highest value
	if (RuleI(Spells, AdditiveBonusWornType) && RuleI(Spells, AdditiveBonusWornType) != EQ::item::ItemEffectWorn){
		for (i = EQ::invslot::BONUS_BEGIN; i <= EQ::invslot::BONUS_STAT_END; i++) {
			const EQ::ItemInstance* inst = m_inv[i];
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
	if (IsValidSpell(SPELL_AVATAR_ST_PROC) && FindBuff(SPELL_AVATAR_ST_PROC)) {
		itembonuses.ATK += 100;
		spellbonuses.ATK -= 100;
	}

	itembonuses.ATK = std::min(itembonuses.ATK, CalcItemATKCap());
}

void Client::AddItemBonuses(const EQ::ItemInstance *inst, StatBonuses *newbon, bool isAug, bool isTribute, int rec_override, bool ammo_slot_item)
{
	if (!inst || !inst->IsClassCommon()) {
		return;
	}

	if (inst->GetAugmentType() == 0 && isAug == true) {
		return;
	}

	const EQ::ItemData *item = inst->GetItem();

	if (!isTribute && !inst->IsEquipable(GetBaseRace(), GetClass())) {
		if (item->ItemType != EQ::item::ItemTypeFood && item->ItemType != EQ::item::ItemTypeDrink)
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
		if (item->Worn.Effect > 0 && item->Worn.Type == EQ::item::ItemEffectWorn) { // latent effects
			ApplySpellsBonuses(item->Worn.Effect, item->Worn.Level, newbon, 0, item->Worn.Type);
		}

		if (item->Focus.Effect > 0 && (item->Focus.Type == EQ::item::ItemEffectFocus)) { // focus effects
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

		if (item->ExtraDmgSkill != 0 && item->ExtraDmgSkill <= EQ::skills::HIGHEST_SKILL) {
			if ((newbon->SkillDamageAmount[item->ExtraDmgSkill] + item->ExtraDmgAmt) >
				RuleI(Character, ItemExtraDmgCap))
				newbon->SkillDamageAmount[item->ExtraDmgSkill] = RuleI(Character, ItemExtraDmgCap);
			else
				newbon->SkillDamageAmount[item->ExtraDmgSkill] += item->ExtraDmgAmt;
		}
	}

	// Process when ammo_slot_item = true or false
	if (item->SkillModValue != 0 && item->SkillModType <= EQ::skills::HIGHEST_SKILL) {
		if ((item->SkillModValue > 0 && newbon->skillmod[item->SkillModType] < item->SkillModValue) ||
			(item->SkillModValue < 0 && newbon->skillmod[item->SkillModType] > item->SkillModValue)) {

			newbon->skillmod[item->SkillModType] = item->SkillModValue;
			newbon->skillmodmax[item->SkillModType] = item->SkillModMax;
		}
	}

	if (!isAug) {
		for (int i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++)
			AddItemBonuses(inst->GetAugment(i), newbon, true, false, rec_level, ammo_slot_item);
	}
}

void Client::AdditiveWornBonuses(const EQ::ItemInstance *inst, StatBonuses* newbon, bool isAug) {

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

	const EQ::ItemData *item = inst->GetItem();

	if(!inst->IsEquipable(GetBaseRace(),GetClass()))
		return;

	if(GetLevel() < item->ReqLevel)
		return;

	if (item->Worn.Effect > 0 && item->Worn.Type == RuleI(Spells, AdditiveBonusWornType))
		ApplySpellsBonuses(item->Worn.Effect, item->Worn.Level, newbon, 0, item->Worn.Type);// Non-live like - Addititive latent effects


	if (!isAug)
	{
		int i;
		for (i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
			AdditiveWornBonuses(inst->GetAugment(i),newbon,true);
		}
	}
}

void Client::CalcEdibleBonuses(StatBonuses* newbon) {
	uint32 i;

	bool food = false;
	bool drink = false;
	for (i = EQ::invslot::GENERAL_BEGIN; i <= EQ::invslot::GENERAL_END; i++)
	{
		if (food && drink)
			break;
		const EQ::ItemInstance* inst = GetInv().GetItem(i);
		if (inst && inst->GetItem() && inst->IsClassCommon()) {
			const EQ::ItemData *item = inst->GetItem();
			if (!food && item->ItemType == EQ::item::ItemTypeFood)
				food = true;
			else if (!drink && item->ItemType == EQ::item::ItemTypeDrink)
				drink = true;
			else
				continue;
			AddItemBonuses(inst, newbon);
		}
	}
	for (i = EQ::invbag::GENERAL_BAGS_BEGIN; i <= EQ::invbag::GENERAL_BAGS_END; i++)
	{
		if (food && drink)
			break;
		const EQ::ItemInstance* inst = GetInv().GetItem(i);
		if (inst && inst->GetItem() && inst->IsClassCommon()) {
			const EQ::ItemData *item = inst->GetItem();
			if (!food && item->ItemType == EQ::item::ItemTypeFood)
				food = true;
			else if (!drink && item->ItemType == EQ::item::ItemTypeDrink)
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
	int32 base_value = 0;
	int32 limit_value = 0; // only really used for SE_RaiseStatCap & SE_ReduceSkillTimer in aa_effects table
	uint32 slot = 0;

	for (const auto &e : rank.effects) {
		effect = e.effect_id;
		base_value = e.base_value;
		limit_value = e.limit_value;
		slot = e.slot;

		// we default to 0 (SE_CurrentHP) for the effect, so if there aren't any base1/2 values, we'll just skip it
		if (effect == 0 && base_value == 0 && limit_value == 0)
			continue;

		// IsBlankSpellEffect()
		if (effect == SE_Blank || (effect == SE_CHA && base_value == 0) || effect == SE_StackingCommand_Block ||
		    effect == SE_StackingCommand_Overwrite)
			continue;

		LogAA("Applying Effect [{}] from AA [{}] in slot [{}] (base1: [{}], base2: [{}]) on [{}]",
			effect, rank.id, slot, base_value, limit_value, GetCleanName());

		uint8 focus = IsFocusEffect(0, 0, true, effect);
		if (focus) {
			newbon->FocusEffects[focus] = effect;
			continue;
		}

		switch (effect) {
		case SE_ACv2:
		case SE_ArmorClass:
			newbon->AC += base_value;
			break;
		// Note: AA effects that use accuracy are skill limited, while spell effect is not.
		case SE_Accuracy:
			// Bad data or unsupported new skill
			if (limit_value > EQ::skills::HIGHEST_SKILL)
				break;
			if ((limit_value == ALL_SKILLS) && (newbon->Accuracy[EQ::skills::HIGHEST_SKILL + 1] < base_value))
				newbon->Accuracy[EQ::skills::HIGHEST_SKILL + 1] = base_value;
			else if (newbon->Accuracy[limit_value] < base_value)
				newbon->Accuracy[limit_value] += base_value;
			break;
		case SE_CurrentHP: // regens
			newbon->HPRegen += base_value;
			break;
		case SE_CurrentEndurance:
			newbon->EnduranceRegen += base_value;
			break;
		case SE_MovementSpeed:
			newbon->movementspeed += base_value; // should we let these stack?
			/*if (base1 > newbon->movementspeed)	//or should we use a total value?
				newbon->movementspeed = base1;*/
			break;
		case SE_STR:
			newbon->STR += base_value;
			break;
		case SE_DEX:
			newbon->DEX += base_value;
			break;
		case SE_AGI:
			newbon->AGI += base_value;
			break;
		case SE_STA:
			newbon->STA += base_value;
			break;
		case SE_INT:
			newbon->INT += base_value;
			break;
		case SE_WIS:
			newbon->WIS += base_value;
			break;
		case SE_CHA:
			newbon->CHA += base_value;
			break;
		case SE_WaterBreathing:
			// handled by client
			break;
		case SE_CurrentMana:
			newbon->ManaRegen += base_value;
			break;
		case SE_ManaPool:
			newbon->Mana += base_value;
			break;
		case SE_ItemManaRegenCapIncrease:
			newbon->ItemManaRegenCap += base_value;
			break;
		case SE_ResistFire:
			newbon->FR += base_value;
			break;
		case SE_ResistCold:
			newbon->CR += base_value;
			break;
		case SE_ResistPoison:
			newbon->PR += base_value;
			break;
		case SE_ResistDisease:
			newbon->DR += base_value;
			break;
		case SE_ResistMagic:
			newbon->MR += base_value;
			break;
		case SE_ResistCorruption:
			newbon->Corrup += base_value;
			break;
		case SE_IncreaseSpellHaste:
			break;
		case SE_IncreaseRange:
			break;
		case SE_MaxHPChange:
			newbon->MaxHP += base_value;
			break;
		case SE_Packrat:
			newbon->Packrat += base_value;
			break;
		case SE_TwoHandBash:
			break;
		case SE_SetBreathLevel:
			break;
		case SE_RaiseStatCap:
			switch (limit_value) {
			// are these #define'd somewhere?
			case 0: // str
				newbon->STRCapMod += base_value;
				break;
			case 1: // sta
				newbon->STACapMod += base_value;
				break;
			case 2: // agi
				newbon->AGICapMod += base_value;
				break;
			case 3: // dex
				newbon->DEXCapMod += base_value;
				break;
			case 4: // wis
				newbon->WISCapMod += base_value;
				break;
			case 5: // int
				newbon->INTCapMod += base_value;
				break;
			case 6: // cha
				newbon->CHACapMod += base_value;
				break;
			case 7: // mr
				newbon->MRCapMod += base_value;
				break;
			case 8: // cr
				newbon->CRCapMod += base_value;
				break;
			case 9: // fr
				newbon->FRCapMod += base_value;
				break;
			case 10: // pr
				newbon->PRCapMod += base_value;
				break;
			case 11: // dr
				newbon->DRCapMod += base_value;
				break;
			case 12: // corruption
				newbon->CorrupCapMod += base_value;
				break;
			}
			break;
		case SE_SpellSlotIncrease:
			break;
		case SE_MysticalAttune:
			newbon->BuffSlotIncrease += base_value;
			break;
		case SE_TotalHP:
			newbon->HP += base_value;
			break;
		case SE_StunResist:
			newbon->StunResist += base_value;
			break;
		case SE_SpellCritChance:
			newbon->CriticalSpellChance += base_value;
			break;
		case SE_SpellCritDmgIncrease:
			newbon->SpellCritDmgIncrease += base_value;
			break;
		case SE_DotCritDmgIncrease:
			newbon->DotCritDmgIncrease += base_value;
			break;
		case SE_ResistSpellChance:
			newbon->ResistSpellChance += base_value;
			break;
		case SE_CriticalHealChance:
			newbon->CriticalHealChance += base_value;
			break;
		case SE_CriticalHealOverTime:
			newbon->CriticalHealOverTime += base_value;
			break;
		case SE_CriticalDoTChance:
			newbon->CriticalDoTChance += base_value;
			break;
		case SE_ReduceSkillTimer:
			newbon->SkillReuseTime[limit_value] += base_value;
			break;
		case SE_Fearless:
			newbon->Fearless = true;
			break;
		case SE_PersistantCasting:
			newbon->PersistantCasting += base_value;
			break;
		case SE_DelayDeath:
			newbon->DelayDeath += base_value;
			break;
		case SE_FrontalStunResist:
			newbon->FrontalStunResist += base_value;
			break;
		case SE_ImprovedBindWound:
			newbon->BindWound += base_value;
			break;
		case SE_MaxBindWound:
			newbon->MaxBindWound += base_value;
			break;
		case SE_SeeInvis:
			newbon->SeeInvis = base_value;
			break;
		case SE_BaseMovementSpeed:
			newbon->BaseMovementSpeed += base_value;
			break;
		case SE_IncreaseRunSpeedCap:
			newbon->IncreaseRunSpeedCap += base_value;
			break;
		case SE_ConsumeProjectile:
			newbon->ConsumeProjectile += base_value;
			break;
		case SE_ForageAdditionalItems:
			newbon->ForageAdditionalItems += base_value;
			break;
		case SE_Salvage:
			newbon->SalvageChance += base_value;
			break;
		case SE_ArcheryDamageModifier:
			newbon->ArcheryDamageModifier += base_value;
			break;
		case SE_DoubleRangedAttack:
			newbon->DoubleRangedAttack += base_value;
			break;
		case SE_DamageShield:
			newbon->DamageShield += base_value;
			break;
		case SE_CharmBreakChance:
			newbon->CharmBreakChance += base_value;
			break;
		case SE_OffhandRiposteFail:
			newbon->OffhandRiposteFail += base_value;
			break;
		case SE_ItemAttackCapIncrease:
			newbon->ItemATKCap += base_value;
			break;
		case SE_GivePetGroupTarget:
			newbon->GivePetGroupTarget = true;
			break;
		case SE_ItemHPRegenCapIncrease:
			newbon->ItemHPRegenCap += base_value;
			break;
		case SE_Ambidexterity:
			newbon->Ambidexterity += base_value;
			break;
		case SE_PetMaxHP:
			newbon->PetMaxHP += base_value;
			break;
		case SE_AvoidMeleeChance:
			newbon->AvoidMeleeChanceEffect += base_value;
			break;
		case SE_CombatStability:
			newbon->CombatStability += base_value;
			break;
		case SE_AddSingingMod:
			switch (limit_value) {
			case EQ::item::ItemTypeWindInstrument:
				newbon->windMod += base_value;
				break;
			case EQ::item::ItemTypeStringedInstrument:
				newbon->stringedMod += base_value;
				break;
			case EQ::item::ItemTypeBrassInstrument:
				newbon->brassMod += base_value;
				break;
			case EQ::item::ItemTypePercussionInstrument:
				newbon->percussionMod += base_value;
				break;
			case EQ::item::ItemTypeSinging:
				newbon->singingMod += base_value;
				break;
			}
			break;
		case SE_SongModCap:
			newbon->songModCap += base_value;
			break;
		case SE_PetCriticalHit:
			newbon->PetCriticalHit += base_value;
			break;
		case SE_PetAvoidance:
			newbon->PetAvoidance += base_value;
			break;
		case SE_ShieldBlock:
			newbon->ShieldBlock += base_value;
			break;
		case SE_ShieldEquipDmgMod:
			newbon->ShieldEquipDmgMod += base_value;
			break;
		case SE_SecondaryDmgInc:
			newbon->SecondaryDmgInc = true;
			break;
		case SE_ChangeAggro:
			newbon->hatemod += base_value;
			break;
		case SE_EndurancePool:
			newbon->Endurance += base_value;
			break;
		case SE_ChannelChanceItems:
			newbon->ChannelChanceItems += base_value;
			break;
		case SE_ChannelChanceSpells:
			newbon->ChannelChanceSpells += base_value;
			break;
		case SE_DoubleSpecialAttack:
			newbon->DoubleSpecialAttack += base_value;
			break;
		case SE_TripleBackstab:
			newbon->TripleBackstab += base_value;
			break;
		case SE_FrontalBackstabMinDmg:
			newbon->FrontalBackstabMinDmg = true;
			break;
		case SE_FrontalBackstabChance:
			newbon->FrontalBackstabChance += base_value;
			break;
		case SE_Double_Backstab_Front:
			newbon->Double_Backstab_Front += base_value;
			break;
		case SE_BlockBehind:
			newbon->BlockBehind += base_value;
			break;
		case SE_StrikeThrough:
		case SE_StrikeThrough2:
			newbon->StrikeThrough += base_value;
			break;
		case SE_DoubleAttackChance:
			newbon->DoubleAttackChance += base_value;
			break;
		case SE_GiveDoubleAttack:
			newbon->GiveDoubleAttack += base_value;
			break;
		case SE_ProcChance:
			newbon->ProcChanceSPA += base_value;
			break;
		case SE_RiposteChance:
			newbon->RiposteChance += base_value;
			break;
		case SE_DodgeChance:
			newbon->DodgeChance += base_value;
			break;
		case SE_ParryChance:
			newbon->ParryChance += base_value;
			break;
		case SE_IncreaseBlockChance:
			newbon->IncreaseBlockChance += base_value;
			break;
		case SE_Flurry:
			newbon->FlurryChance += base_value;
			break;
		case SE_PetFlurry:
			newbon->PetFlurry += base_value;
			break;
		case SE_BardSongRange:
			newbon->SongRange += base_value;
			break;
		case SE_RootBreakChance:
			newbon->RootBreakChance += base_value;
			break;
		case SE_UnfailingDivinity:
			newbon->UnfailingDivinity += base_value;
			break;
		case SE_CrippBlowChance:
			newbon->CrippBlowChance += base_value;
			break;

		case SE_HitChance: {
			// Bad data or unsupported new skill
			if (limit_value > EQ::skills::HIGHEST_SKILL)
				break;
			if (limit_value == ALL_SKILLS)
				newbon->HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] += base_value;
			else
				newbon->HitChanceEffect[limit_value] += base_value;
		}

		case SE_ProcOnKillShot:
			for (int i = 0; i < MAX_SPELL_TRIGGER * 3; i += 3) {
				if (!newbon->SpellOnKill[i] ||
				    ((newbon->SpellOnKill[i] == limit_value) && (newbon->SpellOnKill[i + 1] < base_value))) {
					// base1 = chance, base2 = SpellID to be triggered, base3 = min npc level
					newbon->SpellOnKill[i] = limit_value;
					newbon->SpellOnKill[i + 1] = base_value;

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
					newbon->SpellOnDeath[i] = base_value;
					newbon->SpellOnDeath[i + 1] = limit_value;
					break;
				}
			}
			break;

		case SE_WeaponProc:
		case SE_AddMeleeProc:
			for (int i = 0; i < MAX_AA_PROCS; i += 4) {
				if (!newbon->SpellProc[i + SBIndex::COMBAT_PROC_ORIGIN_ID]) {
					newbon->SpellProc[i + SBIndex::COMBAT_PROC_ORIGIN_ID] = rank.id;   //aa rank id
					newbon->SpellProc[i + SBIndex::COMBAT_PROC_SPELL_ID] = base_value; //proc spell id
					newbon->SpellProc[i + SBIndex::COMBAT_PROC_RATE_MOD] = limit_value; //proc rate modifer
					newbon->SpellProc[i + SBIndex::COMBAT_PROC_REUSE_TIMER] = 0;	  //Lock out Timer
					break;
				}
			}
			break;

		case SE_RangedProc:
			for (int i = 0; i < MAX_AA_PROCS; i += 4) {
				if (!newbon->RangedProc[i + SBIndex::COMBAT_PROC_ORIGIN_ID]) {
					newbon->RangedProc[i + SBIndex::COMBAT_PROC_ORIGIN_ID] = rank.id;   //aa rank id
					newbon->RangedProc[i + SBIndex::COMBAT_PROC_SPELL_ID] = base_value; //proc spell id
					newbon->RangedProc[i + SBIndex::COMBAT_PROC_RATE_MOD] = limit_value; //proc rate modifer
					newbon->RangedProc[i + SBIndex::COMBAT_PROC_REUSE_TIMER] = 0;	   //Lock out Timer
					break;
				}
			}
			break;

		case SE_DefensiveProc:
			for (int i = 0; i < MAX_AA_PROCS; i += 4) {
				if (!newbon->DefensiveProc[i + SBIndex::COMBAT_PROC_ORIGIN_ID]) {
					newbon->DefensiveProc[i + SBIndex::COMBAT_PROC_ORIGIN_ID] = rank.id;   //aa rank id
					newbon->DefensiveProc[i + SBIndex::COMBAT_PROC_SPELL_ID] = base_value; //proc spell id
					newbon->DefensiveProc[i + SBIndex::COMBAT_PROC_RATE_MOD] = limit_value; //proc rate modifer
					newbon->DefensiveProc[i + SBIndex::COMBAT_PROC_REUSE_TIMER] = 0;	  //Lock out Timer
					break;
				}
			}
			break;

		case SE_Proc_Timer_Modifier: {
			/*
				AA can multiples of this in a single effect, proc should use the timer 
				that comes after the respective proc spell effect, thus rank.id will be already set
				when this is checked.
			*/

			newbon->Proc_Timer_Modifier = true;

			for (int i = 0; i < MAX_AA_PROCS; i += 4) {
				if (newbon->SpellProc[i + SBIndex::COMBAT_PROC_ORIGIN_ID] == rank.id) {
					if (!newbon->SpellProc[i + SBIndex::COMBAT_PROC_REUSE_TIMER]) {
						newbon->SpellProc[i + SBIndex::COMBAT_PROC_REUSE_TIMER] = limit_value;//Lock out Timer
						break;
					}
				}

				if (newbon->RangedProc[i + SBIndex::COMBAT_PROC_ORIGIN_ID] == rank.id) {
					if (!newbon->RangedProc[i + SBIndex::COMBAT_PROC_REUSE_TIMER]) {
						newbon->RangedProc[i + SBIndex::COMBAT_PROC_REUSE_TIMER] = limit_value;//Lock out Timer
						break;
					}
				}

				if (newbon->DefensiveProc[i + SBIndex::COMBAT_PROC_ORIGIN_ID] == rank.id) {
					if (!newbon->DefensiveProc[i + SBIndex::COMBAT_PROC_REUSE_TIMER]) {
						newbon->DefensiveProc[i + SBIndex::COMBAT_PROC_REUSE_TIMER] = limit_value;//Lock out Timer
						break;
					}
				}
			}
			break;
		}

		case SE_CriticalHitChance: {
			// Bad data or unsupported new skill
			if (limit_value > EQ::skills::HIGHEST_SKILL)
				break;
			if (limit_value == ALL_SKILLS)
				newbon->CriticalHitChance[EQ::skills::HIGHEST_SKILL + 1] += base_value;
			else
				newbon->CriticalHitChance[limit_value] += base_value;
		} break;

		case SE_CriticalDamageMob: {
			// Bad data or unsupported new skill
			if (limit_value > EQ::skills::HIGHEST_SKILL)
				break;
			// base1 = effect value, base2 = skill restrictions(-1 for all)
			if (limit_value == ALL_SKILLS)
				newbon->CritDmgMod[EQ::skills::HIGHEST_SKILL + 1] += base_value;
			else
				newbon->CritDmgMod[limit_value] += base_value;
			break;
		}

		case SE_Critical_Melee_Damage_Mod_Max:
		{
			// Bad data or unsupported new skill
			if (limit_value > EQ::skills::HIGHEST_SKILL)
				break;
			int skill = limit_value == ALL_SKILLS ? EQ::skills::HIGHEST_SKILL + 1 : limit_value;
			if (base_value < 0 && newbon->CritDmgModNoStack[skill] > base_value)
				newbon->CritDmgModNoStack[skill] = base_value;
			else if (base_value > 0 && newbon->CritDmgModNoStack[skill] < base_value)
				newbon->CritDmgModNoStack[skill] = base_value;
			break;
		}

		case SE_CriticalSpellChance: {
			newbon->CriticalSpellChance += base_value;

			if (limit_value > newbon->SpellCritDmgIncNoStack)
				newbon->SpellCritDmgIncNoStack = limit_value;

			break;
		}

		case SE_ResistFearChance: {
			if (base_value == 100) // If we reach 100% in a single spell/item then we should be immune to
					  // negative fear resist effects until our immunity is over
				newbon->Fearless = true;

			newbon->ResistFearChance += base_value; // these should stack
			break;
		}

		case SE_SkillDamageAmount: {
			// Bad data or unsupported new skill
			if (limit_value > EQ::skills::HIGHEST_SKILL)
				break;
			if (limit_value == ALL_SKILLS)
				newbon->SkillDamageAmount[EQ::skills::HIGHEST_SKILL + 1] += base_value;
			else
				newbon->SkillDamageAmount[limit_value] += base_value;
			break;
		}

		case SE_SkillAttackProc: {
			for (int i = 0; i < MAX_CAST_ON_SKILL_USE i += 3) {
				if (!newbon->SkillAttackProc[i + SBIndex::SKILLATK_PROC_SPELL_ID]) { // spell id
					newbon->SkillAttackProc[i + SBIndex::SKILLATK_PROC_SPELL_ID] = rank.spell; // spell to proc
					newbon->SkillAttackProc[i + SBIndex::SKILLATK_PROC_CHANCE] = base_value; // Chance base 1000 = 100% proc rate
					newbon->SkillAttackProc[i + SBIndex::SKILLATK_PROC_SKILL] = limit_value; // Skill to Proc Offr
					
					if (limit_value < EQ::skills::HIGHEST_SKILL) {
						newbon->HasSkillAttackProc[limit_value] = true; //check first before looking for any effects.
					}
					break;
				}
			}
		}

		case SE_DamageModifier: {
			// Bad data or unsupported new skill
			if (limit_value > EQ::skills::HIGHEST_SKILL)
				break;
			if (limit_value == ALL_SKILLS)
				newbon->DamageModifier[EQ::skills::HIGHEST_SKILL + 1] += base_value;
			else
				newbon->DamageModifier[limit_value] += base_value;
			break;
		}

		case SE_DamageModifier2: {
			// Bad data or unsupported new skill
			if (limit_value > EQ::skills::HIGHEST_SKILL)
				break;
			if (limit_value == ALL_SKILLS)
				newbon->DamageModifier2[EQ::skills::HIGHEST_SKILL + 1] += base_value;
			else
				newbon->DamageModifier2[limit_value] += base_value;
			break;
		}

		case SE_Skill_Base_Damage_Mod: {
			// Bad data or unsupported new skill
			if (limit_value > EQ::skills::HIGHEST_SKILL)
				break;
			if (limit_value == ALL_SKILLS)
				newbon->DamageModifier3[EQ::skills::HIGHEST_SKILL + 1] += base_value;
			else
				newbon->DamageModifier3[limit_value] += base_value;
			break;
		}

		case SE_SlayUndead: {
			if (newbon->SlayUndead[SBIndex::SLAYUNDEAD_DMG_MOD] < base_value)
				newbon->SlayUndead[SBIndex::SLAYUNDEAD_RATE_MOD] = base_value; // Rate
			newbon->SlayUndead[SBIndex::SLAYUNDEAD_DMG_MOD]      = limit_value;	 // Damage Modifier
			break;
		}

		case SE_DoubleRiposte: {
			newbon->DoubleRiposte += base_value;
			break;
		}

		case SE_GiveDoubleRiposte: {
			// 0=Regular Riposte 1=Skill Attack Riposte 2=Skill
			if (limit_value == 0) {
				if (newbon->GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_CHANCE] < base_value)
					newbon->GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_CHANCE] = base_value;
			}
			// Only for special attacks.
			else if (limit_value > 0 && (newbon->GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_SKILL_ATK_CHANCE] < base_value)) {
				newbon->GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_SKILL_ATK_CHANCE] = base_value;
				newbon->GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_SKILL]            = limit_value;
			}

			break;
		}

		// Physically raises skill cap ie if 55/55 it will raise to 55/60
		case SE_RaiseSkillCap: {

			if (limit_value > EQ::skills::HIGHEST_SKILL)
				break;

			if (newbon->RaiseSkillCap[limit_value] < base_value)
				newbon->RaiseSkillCap[limit_value] = base_value;
			break;
		}

		case SE_MasteryofPast: {
			if (newbon->MasteryofPast < base_value)
				newbon->MasteryofPast = base_value;
			break;
		}

		case SE_CastingLevel: {
			newbon->adjusted_casting_skill += base_value;
			break;
		}

		case SE_CastingLevel2: {
			newbon->effective_casting_level += base_value;
			break;
		}

		case SE_DivineSave: {
			if (newbon->DivineSaveChance[SBIndex::DIVINE_SAVE_CHANCE] < base_value) {
				newbon->DivineSaveChance[SBIndex::DIVINE_SAVE_CHANCE]           = base_value;
				newbon->DivineSaveChance[SBIndex::DIVINE_SAVE_SPELL_TRIGGER_ID] = limit_value;
			}
			break;
		}

		case SE_SpellEffectResistChance: {
			for (int e = 0; e < MAX_RESISTABLE_EFFECTS * 2; e += 2) {
				if (newbon->SEResist[e + 1] && (newbon->SEResist[e] == limit_value) &&
				    (newbon->SEResist[e + 1] < base_value)) {
					newbon->SEResist[e] = limit_value; // Spell Effect ID
					newbon->SEResist[e + 1] = base_value; // Resist Chance
					break;
				} else if (!newbon->SEResist[e + 1]) {
					newbon->SEResist[e] = limit_value; // Spell Effect ID
					newbon->SEResist[e + 1] = base_value; // Resist Chance
					break;
				}
			}
			break;
		}

		case SE_MitigateDamageShield: {
			
			//AA that increase mitigation are set to negative.
			if (base_value < 0) {
				base_value = base_value * (-1);
			}

			newbon->DSMitigationOffHand += base_value;
			break;
		}

		case SE_FinishingBlow: {
			// base1 = chance, base2 = damage
			if (newbon->FinishingBlow[SBIndex::FINISHING_EFFECT_DMG] < limit_value) {
				newbon->FinishingBlow[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = base_value;
				newbon->FinishingBlow[SBIndex::FINISHING_EFFECT_DMG]         = limit_value;
			}
			break;
		}

		case SE_FinishingBlowLvl: {
			// base1 = level, base2 = ??? (Set to 200 in AA data, possible proc rate mod?)
			if (newbon->FinishingBlowLvl[SBIndex::FINISHING_EFFECT_LEVEL_MAX] < base_value) {
				newbon->FinishingBlowLvl[SBIndex::FINISHING_EFFECT_LEVEL_MAX]          = base_value;
				newbon->FinishingBlowLvl[SBIndex::FINISHING_BLOW_LEVEL_HP_RATIO] = limit_value;
			}
			break;
		}

		case SE_StunBashChance:
			newbon->StunBashChance += base_value;
			break;

		case SE_IncreaseChanceMemwipe:
			newbon->IncreaseChanceMemwipe += base_value;
			break;

		case SE_CriticalMend:
			newbon->CriticalMend += base_value;
			break;

		case SE_HealRate:
			newbon->HealRate += base_value;
			break;

		case SE_MeleeLifetap: {

			if ((base_value < 0) && (newbon->MeleeLifetap > base_value))
				newbon->MeleeLifetap = base_value;

			else if (newbon->MeleeLifetap < base_value)
				newbon->MeleeLifetap = base_value;
			break;
		}

		case SE_Vampirism:
			newbon->Vampirism += base_value;
			break;

		case SE_FrenziedDevastation:
			newbon->FrenziedDevastation += limit_value;
			break;

		case SE_SpellProcChance:
			newbon->SpellProcChance += base_value;
			break;

		case SE_Berserk:
			newbon->BerserkSPA = true;
			break;

		case SE_Metabolism:
			newbon->Metabolism += base_value;
			break;

		case SE_ImprovedReclaimEnergy: {
			if ((base_value < 0) && (newbon->ImprovedReclaimEnergy > base_value))
				newbon->ImprovedReclaimEnergy = base_value;

			else if (newbon->ImprovedReclaimEnergy < base_value)
				newbon->ImprovedReclaimEnergy = base_value;
			break;
		}

		case SE_HeadShot: {
			if (newbon->HeadShot[SBIndex::FINISHING_EFFECT_DMG] < limit_value) {
				newbon->HeadShot[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = base_value;
				newbon->HeadShot[SBIndex::FINISHING_EFFECT_DMG]         = limit_value;
			}
			break;
		}

		case SE_HeadShotLevel: {
			if (newbon->HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX] < base_value)
				newbon->HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX]          = base_value;
				newbon->HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS] = limit_value;
			break;
		}

		case SE_Assassinate: {
			if (newbon->Assassinate[SBIndex::FINISHING_EFFECT_DMG] < limit_value) {
				newbon->Assassinate[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = base_value;
				newbon->Assassinate[SBIndex::FINISHING_EFFECT_DMG]         = limit_value;
			}
			break;
		}

		case SE_AssassinateLevel: {
			if (newbon->AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX] < base_value) {
				newbon->AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX]          = base_value;
				newbon->AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS] = limit_value;
			}
			break;
		}

		case SE_PetMeleeMitigation:
			newbon->PetMeleeMitigation += base_value;
			break;

		case SE_FactionModPct: {
			if ((base_value < 0) && (newbon->FactionModPct > base_value))
				newbon->FactionModPct = base_value;

			else if (newbon->FactionModPct < base_value)
				newbon->FactionModPct = base_value;
			break;
		}

		case SE_IllusionPersistence:
			newbon->IllusionPersistence = base_value;
			break;

		case SE_LimitToSkill: {

			// Bad data or unsupported new skill
			if (base_value > EQ::skills::HIGHEST_SKILL) {
				break;
			}
			if (base_value <= EQ::skills::HIGHEST_SKILL) {
				newbon->LimitToSkill[base_value] = true;
				newbon->LimitToSkill[EQ::skills::HIGHEST_SKILL + 3] = true; //Used as a general exists check
			}
			break;
		}

		case SE_SkillProcAttempt: {
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
			newbon->MeleeMitigationEffect += base_value;
			break;

		case SE_ATK:
			newbon->ATK += base_value;
			break;
		case SE_IncreaseExtTargetWindow:
			newbon->extra_xtargets += base_value;
			break;

		case SE_PC_Pet_Rampage: {
			newbon->PC_Pet_Rampage[SBIndex::PET_RAMPAGE_CHANCE] += base_value; //Chance to rampage
			if (newbon->PC_Pet_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] < limit_value)
				newbon->PC_Pet_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] = limit_value; //Damage modifer - take highest
			break;
		}

		case SE_PC_Pet_AE_Rampage: {
			newbon->PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_CHANCE] += base_value; //Chance to rampage
			if (newbon->PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] < limit_value)
				newbon->PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] = limit_value; //Damage modifer - take highest
			break;
		}

		case SE_PC_Pet_Flurry_Chance:
			newbon->PC_Pet_Flurry += base_value; //Chance to Flurry
			break;

		case SE_ShroudofStealth:
			newbon->ShroudofStealth = true;
			break;

		case SE_ReduceFallDamage:
			newbon->ReduceFallDamage += base_value;
			break;

		case SE_ReduceTradeskillFail:{

			if (limit_value > EQ::skills::HIGHEST_SKILL)
				break;

			newbon->ReduceTradeskillFail[limit_value] += base_value;
			break;
		}

		case SE_TradeSkillMastery:
			if (newbon->TradeSkillMastery < base_value)
				newbon->TradeSkillMastery = base_value;
			break;

		case SE_NoBreakAESneak:
			if (newbon->NoBreakAESneak < base_value)
				newbon->NoBreakAESneak = base_value;
			break;

		case SE_FeignedCastOnChance:
			if (newbon->FeignedCastOnChance < base_value)
				newbon->FeignedCastOnChance = base_value;
			break;

		case SE_AddPetCommand:
			if (base_value && limit_value < PET_MAXCOMMANDS)
				newbon->PetCommands[limit_value] = true;
			break;

		case SE_FeignedMinion:
			if (newbon->FeignedMinionChance < base_value)
				newbon->FeignedMinionChance = base_value;
			break;

		case SE_AdditionalAura:
			newbon->aura_slots += base_value;
			break;

		case SE_IncreaseTrapCount:
			newbon->trap_slots += base_value;
			break;

		case SE_ForageSkill:
			newbon->GrantForage += base_value;
			// we need to grant a skill point here
			// I'd rather not do this here, but whatever, probably fine
			if (IsClient()) {
				auto client = CastToClient();
				if (client->GetRawSkill(EQ::skills::SkillType::SkillForage) == 0)
					client->SetSkill(EQ::skills::SkillType::SkillForage, 1);
			}
			break;

		case SE_Attack_Accuracy_Max_Percent:
			newbon->Attack_Accuracy_Max_Percent += base_value;
			break;

		case SE_AC_Mitigation_Max_Percent:
			newbon->AC_Mitigation_Max_Percent += base_value;
			break;

		case SE_AC_Avoidance_Max_Percent:
			newbon->AC_Avoidance_Max_Percent += base_value;
			break;

		case SE_Damage_Taken_Position_Mod:
		{
			//Mitigate if damage taken from behind base2 = 0, from front base2 = 1
			if (limit_value < 0 || limit_value > 2)
				break;
			else if (base_value < 0 && newbon->Damage_Taken_Position_Mod[limit_value] > base_value)
				newbon->Damage_Taken_Position_Mod[limit_value] = base_value;
			else if (base_value > 0 && newbon->Damage_Taken_Position_Mod[limit_value] < base_value)
				newbon->Damage_Taken_Position_Mod[limit_value] = base_value;
			break;
		}

		case SE_Melee_Damage_Position_Mod:
		{
			if (limit_value < 0 || limit_value > 2)
				break;
			else if (base_value < 0 && newbon->Melee_Damage_Position_Mod[limit_value] > base_value)
				newbon->Melee_Damage_Position_Mod[limit_value] = base_value;
			else if (base_value > 0 && newbon->Melee_Damage_Position_Mod[limit_value] < base_value)
				newbon->Melee_Damage_Position_Mod[limit_value] = base_value;
			break;
		}

		case SE_Damage_Taken_Position_Amt:
		{
			//Mitigate if damage taken from behind base2 = 0, from front base2 = 1
			if (limit_value < 0 || limit_value > 2)
				break;

			newbon->Damage_Taken_Position_Amt[limit_value] += base_value;
			break;
		}

		case SE_Melee_Damage_Position_Amt:
		{
			//Mitigate if damage taken from behind base2 = 0, from front base2 = 1
			if (limit_value < 0 || limit_value > 2)
				break;

			newbon->Melee_Damage_Position_Amt[limit_value] += base_value;
			break;
		}

		case SE_DS_Mitigation_Amount:
			newbon->DS_Mitigation_Amount += base_value;
			break;

		case SE_DS_Mitigation_Percentage:
			newbon->DS_Mitigation_Percentage += base_value;
			break;

		case SE_Pet_Crit_Melee_Damage_Pct_Owner:
			newbon->Pet_Crit_Melee_Damage_Pct_Owner += base_value;
			break;

		case SE_Pet_Add_Atk:
			newbon->Pet_Add_Atk += base_value;
			break;

		case SE_Weapon_Stance:
		{
			if (IsValidSpell(base_value)) { //base1 is the spell_id of buff
				if (limit_value <= WEAPON_STANCE_TYPE_MAX) { //0=2H, 1=Shield, 2=DW
					if (IsValidSpell(newbon->WeaponStance[limit_value])) { //Check if we already a spell_id saved for this effect
						if (spells[newbon->WeaponStance[limit_value]].rank < spells[base_value].rank) { //If so, check if any new spellids with higher rank exist (live spells for this are ranked).
							newbon->WeaponStance[limit_value] = base_value; //Overwrite with new effect
							SetWeaponStanceEnabled(true);
						}
					}
					else {
						newbon->WeaponStance[limit_value] = base_value; //If no prior effect exists, then apply
						SetWeaponStanceEnabled(true);
					}
				}
			}
			break;
		}

		case SE_ExtraAttackChance:
		{
			if (newbon->ExtraAttackChance[SBIndex::EXTRA_ATTACK_CHANCE] < base_value) {
				newbon->ExtraAttackChance[SBIndex::EXTRA_ATTACK_CHANCE]   = base_value;
				newbon->ExtraAttackChance[SBIndex::EXTRA_ATTACK_NUM_ATKS] = limit_value ? limit_value : 1;
			}
			break;
		}

		case SE_AddExtraAttackPct_1h_Primary:
		{
			if (newbon->ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_CHANCE] < base_value) {
				newbon->ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_CHANCE]   = base_value;
				newbon->ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_NUM_ATKS] = limit_value ? limit_value : 1;
			}
			break;
		}

		case SE_AddExtraAttackPct_1h_Secondary:
		{

			if (newbon->ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_CHANCE] < base_value) {
				newbon->ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_CHANCE]   = base_value;
				newbon->ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_NUM_ATKS] = limit_value ? limit_value : 1;
			}
			break;
		}

		case SE_Double_Melee_Round:
		{
			if (newbon->DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_CHANCE] < base_value) {
				newbon->DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_CHANCE] = base_value;
				newbon->DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_DMG_BONUS] = limit_value;

			}
			break;
		}

		case SE_ExtendedShielding: 
		{
			if (newbon->ExtendedShielding < base_value) {
				newbon->ExtendedShielding = base_value;
			}
			break;
		}

		case SE_ShieldDuration: 
		{
			if (newbon->ShieldDuration < base_value) {
				newbon->ShieldDuration = base_value;
			}
			break;
		}

		case SE_Worn_Endurance_Regen_Cap:
			newbon->ItemEnduranceRegenCap += base_value;
			break;


		case SE_SecondaryForte:
			if (newbon->SecondaryForte < base_value) {
				newbon->SecondaryForte = base_value;
			}
			break;
        
		case SE_ZoneSuspendMinion:
			newbon->ZoneSuspendMinion = base_value;
			break;


		case SE_Reflect:

			if (newbon->reflect[SBIndex::REFLECT_CHANCE] < base_value) {
				newbon->reflect[SBIndex::REFLECT_CHANCE] = base_value;
			}
			if (newbon->reflect[SBIndex::REFLECT_RESISTANCE_MOD] < limit_value) {
				newbon->reflect[SBIndex::REFLECT_RESISTANCE_MOD] = limit_value;
			}
			break;

		case SE_SpellDamageShield:
			newbon->SpellDamageShield += base_value;
			break;

		case SE_Amplification:
			newbon->Amplification += base_value;
			break;

		case SE_MitigateSpellDamage:
		{
			newbon->MitigateSpellRune[SBIndex::MITIGATION_RUNE_PERCENT] += base_value;
			break;
		}

		case SE_MitigateDotDamage:
		{
			newbon->MitigateDotRune[SBIndex::MITIGATION_RUNE_PERCENT] += base_value;
			break;
		}

		case SE_TrapCircumvention:
			newbon->TrapCircumvention += base_value;
			break;

		// to do
		case SE_PetDiscipline:
			break;
		case SE_PotionBeltSlots:
			break;
		case SE_BandolierSlots:
			break;
		case SE_ReduceApplyPoisonTime:
			break;
		case SE_NimbleEvasion:
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
			LogAA("SPA [{}] not accounted for in AA [{}] ([{}])", effect, rank.base_ability->name.c_str(), rank.id);
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

			if (buffs[i].hit_number > 0)
				Numhits(true);
		}
	}

	//Applies any perma NPC spell bonuses from npc_spells_effects table.
	if (IsNPC())
		CastToNPC()->ApplyAISpellEffects(newbon);

	//Disables a specific spell effect bonus completely, can also be limited to negate only item, AA or spell bonuses.
	if (spellbonuses.NegateEffects){
		for(i = 0; i < buff_count; i++) {
			if( (buffs[i].spellid != SPELL_UNKNOWN) && (IsEffectInSpell(buffs[i].spellid, SE_NegateSpellEffect)) )
				NegateSpellEffectBonuses(buffs[i].spellid);
		}
	}

	if (GetClass() == BARD)
		newbon->ManaRegen = 0; // Bards do not get mana regen from spells.
}

void Mob::ApplySpellsBonuses(uint16 spell_id, uint8 casterlevel, StatBonuses *new_bonus, uint16 casterId,
			     uint8 WornType, int32 ticsremaining, int buffslot, int instrument_mod,
			     bool IsAISpellEffect, uint16 effect_id, int32 se_base, int32 se_limit, int32 se_max)
{
	int i, effect_value, limit_value, max_value, spell_effect_id;
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
					if (RuleB(Spells, UseAdditiveFocusFromWornSlot)) {
						new_bonus->FocusEffectsWorn[focus] += spells[spell_id].base_value[i];
					}
				}
				else {
					new_bonus->FocusEffects[focus] = spells[spell_id].effect_id[i];
				}
				continue;
			}

			if (WornType && (RuleI(Spells, AdditiveBonusWornType) == WornType)) {
				AdditiveWornBonus = true;
			}

			spell_effect_id = spells[spell_id].effect_id[i];
			effect_value = CalcSpellEffectValue(spell_id, i, casterlevel, instrument_mod, nullptr, ticsremaining, casterId);
			limit_value = spells[spell_id].limit_value[i];
			max_value = spells[spell_id].max_value[i];
		}
		//Use AISpellEffects
		else {
			spell_effect_id = effect_id;
			effect_value = se_base;
			limit_value = se_limit;
			max_value = se_max;
			i = EFFECT_COUNT; //End the loop
		}

		switch (spell_effect_id)
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
				if (max_value != 0 && GetLevel() > max_value)
					break;

				if(new_bonus->AggroRange == -1 || effect_value < new_bonus->AggroRange)
				{
					new_bonus->AggroRange = static_cast<float>(effect_value);
				}
				break;
			}

			case SE_Harmony:
			{
				if (max_value != 0 && GetLevel() > max_value)
					break;
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
				if (spells[spell_id].base_value[i] != 0) {
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
				switch(spells[spell_id].limit_value[i])
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

			case SE_CastingLevel:	// Brilliance of Ro
			{
				new_bonus->adjusted_casting_skill += effect_value;
				break;
			}

			case SE_CastingLevel2:
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
				if (IsAISpellEffect && max_value)
					new_bonus->DamageShieldType = GetDamageShieldType(spell_id, max_value);
				else
					new_bonus->DamageShieldType = GetDamageShieldType(spell_id);

				break;
			}

			case SE_ReverseDS:
			{
				new_bonus->ReverseDamageShield += effect_value;
				new_bonus->ReverseDamageShieldSpellID = spell_id;

				if (IsAISpellEffect && max_value)
					new_bonus->ReverseDamageShieldType = GetDamageShieldType(spell_id, max_value);
				else
					new_bonus->ReverseDamageShieldType = GetDamageShieldType(spell_id);
				break;
			}

			case SE_Reflect:

				if (AdditiveWornBonus) {
					new_bonus->reflect[SBIndex::REFLECT_CHANCE] += effect_value;
				}

				else if (new_bonus->reflect[SBIndex::REFLECT_CHANCE] < effect_value) {
					new_bonus->reflect[SBIndex::REFLECT_CHANCE] = effect_value;
					new_bonus->reflect[SBIndex::REFLECT_RESISTANCE_MOD] = limit_value;
					new_bonus->reflect[SBIndex::REFLECT_DMG_EFFECTIVENESS] = max_value;
				}
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
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;
				if (AdditiveWornBonus) {
					if(limit_value == ALL_SKILLS)
						new_bonus->CriticalHitChance[EQ::skills::HIGHEST_SKILL + 1] += effect_value;
					else
						new_bonus->CriticalHitChance[limit_value] += effect_value;
				}

				else if(effect_value < 0) {

					if (limit_value == ALL_SKILLS && new_bonus->CriticalHitChance[EQ::skills::HIGHEST_SKILL + 1] > effect_value)
						new_bonus->CriticalHitChance[EQ::skills::HIGHEST_SKILL + 1] = effect_value;
					else if(limit_value != ALL_SKILLS && new_bonus->CriticalHitChance[limit_value] > effect_value)
						new_bonus->CriticalHitChance[limit_value] = effect_value;
				}


				else if (limit_value == ALL_SKILLS && new_bonus->CriticalHitChance[EQ::skills::HIGHEST_SKILL + 1] < effect_value)
					new_bonus->CriticalHitChance[EQ::skills::HIGHEST_SKILL + 1] = effect_value;
				else if(limit_value != ALL_SKILLS && new_bonus->CriticalHitChance[limit_value] < effect_value)
						new_bonus->CriticalHitChance[limit_value] = effect_value;

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
					new_bonus->MeleeLifetap += spells[spell_id].base_value[i];

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
					new_bonus->MeleeSkillCheckSkill = limit_value==ALL_SKILLS?255:limit_value;
				}
				break;
			}

			case SE_HitChance:
			{
				// Bad data or unsupported new skill
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;

				if (AdditiveWornBonus){
					if(limit_value == ALL_SKILLS)
						new_bonus->HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] += effect_value;
					else
						new_bonus->HitChanceEffect[limit_value] += effect_value;
				}

				else if(limit_value == ALL_SKILLS){

					if ((effect_value < 0) && (new_bonus->HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] > effect_value))
						new_bonus->HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] = effect_value;

					else if (!new_bonus->HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] ||
						((new_bonus->HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] > 0) && (new_bonus->HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] < effect_value)))
						new_bonus->HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] = effect_value;
				}

				else {

					if ((effect_value < 0) && (new_bonus->HitChanceEffect[limit_value] > effect_value))
						new_bonus->HitChanceEffect[limit_value] = effect_value;

					else if (!new_bonus->HitChanceEffect[limit_value] ||
							((new_bonus->HitChanceEffect[limit_value] > 0) && (new_bonus->HitChanceEffect[limit_value] < effect_value)))
							new_bonus->HitChanceEffect[limit_value] = effect_value;
				}

				break;

			}

			case SE_DamageModifier:
			{
				// Bad data or unsupported new skill
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;
				int skill = limit_value == ALL_SKILLS ? EQ::skills::HIGHEST_SKILL + 1 : limit_value;
				if (effect_value < 0 && new_bonus->DamageModifier[skill] > effect_value)
					new_bonus->DamageModifier[skill] = effect_value;
				else if (effect_value > 0 && new_bonus->DamageModifier[skill] < effect_value)
					new_bonus->DamageModifier[skill] = effect_value;
				break;
			}

			case SE_DamageModifier2:
			{
				// Bad data or unsupported new skill
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;
				int skill = limit_value == ALL_SKILLS ? EQ::skills::HIGHEST_SKILL + 1 : limit_value;
				if (effect_value < 0 && new_bonus->DamageModifier2[skill] > effect_value)
					new_bonus->DamageModifier2[skill] = effect_value;
				else if (effect_value > 0 && new_bonus->DamageModifier2[skill] < effect_value)
					new_bonus->DamageModifier2[skill] = effect_value;
				break;
			}

			case SE_Skill_Base_Damage_Mod:
			{
				// Bad data or unsupported new skill
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;
				int skill = limit_value == ALL_SKILLS ? EQ::skills::HIGHEST_SKILL + 1 : limit_value;
				if (effect_value < 0 && new_bonus->DamageModifier3[skill] > effect_value)
					new_bonus->DamageModifier3[skill] = effect_value;
				else if (effect_value > 0 && new_bonus->DamageModifier3[skill] < effect_value)
					new_bonus->DamageModifier3[skill] = effect_value;
				break;
			}

			case SE_MinDamageModifier:
			{
				// Bad data or unsupported new skill
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;
				int skill = limit_value == ALL_SKILLS ? EQ::skills::HIGHEST_SKILL + 1 : limit_value;
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
			{
				if (AdditiveWornBonus) {
					new_bonus->ExtraAttackChance[SBIndex::EXTRA_ATTACK_CHANCE] += effect_value;
					new_bonus->ExtraAttackChance[SBIndex::EXTRA_ATTACK_NUM_ATKS] = limit_value ? limit_value : 1;
				}
				if (new_bonus->ExtraAttackChance[SBIndex::EXTRA_ATTACK_CHANCE] < effect_value) {
					new_bonus->ExtraAttackChance[SBIndex::EXTRA_ATTACK_CHANCE]   = effect_value;
					new_bonus->ExtraAttackChance[SBIndex::EXTRA_ATTACK_NUM_ATKS] = limit_value ? limit_value : 1;
				}
				break;
			}

			case SE_AddExtraAttackPct_1h_Primary:
			{
				if (AdditiveWornBonus) {
					new_bonus->ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_CHANCE] += effect_value;
					new_bonus->ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_NUM_ATKS] = limit_value ? limit_value : 1;
				}

				if (new_bonus->ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_CHANCE] < effect_value) {
					new_bonus->ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_CHANCE]   = effect_value;
					new_bonus->ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_NUM_ATKS] = limit_value ? limit_value : 1;
				}
				break;
			}

			case SE_AddExtraAttackPct_1h_Secondary:
			{
				if (AdditiveWornBonus) {
					new_bonus->ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_CHANCE] += effect_value;
					new_bonus->ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_NUM_ATKS] = limit_value ? limit_value : 1;
				}

				if (new_bonus->ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_CHANCE] < effect_value) {
					new_bonus->ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_CHANCE]   = effect_value;
					new_bonus->ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_NUM_ATKS] = limit_value ? limit_value : 1;
				}
				break;
			}

			case SE_Double_Melee_Round:
			{
				if (AdditiveWornBonus) {
					new_bonus->DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_CHANCE] += effect_value;
					new_bonus->DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_DMG_BONUS] += limit_value;
				}

				if (new_bonus->DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_CHANCE] < effect_value) {
					new_bonus->DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_CHANCE] = effect_value;
					new_bonus->DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_DMG_BONUS] = limit_value;
				}
				break;
			}

			case SE_PercentXPIncrease:
			{
				if(new_bonus->XPRateMod < effect_value)
					new_bonus->XPRateMod = effect_value;
				break;
			}

			case SE_DeathSave:
			{
				if(new_bonus->DeathSave[SBIndex::DEATH_SAVE_TYPE] < effect_value)
				{
					new_bonus->DeathSave[SBIndex::DEATH_SAVE_TYPE]               = effect_value; //1='Partial' 2='Full'
					new_bonus->DeathSave[SBIndex::DEATH_SAVE_BUFFSLOT]           = buffslot;
					//These are used in later expansion spell effects.
					new_bonus->DeathSave[SBIndex::DEATH_SAVE_MIN_LEVEL_FOR_HEAL] = limit_value;//Min level for HealAmt
					new_bonus->DeathSave[SBIndex::DEATH_SAVE_HEAL_AMT]           = max_value;//HealAmt
				}
				break;
			}

			case SE_DivineSave:
			{
				if (AdditiveWornBonus) {
					new_bonus->DivineSaveChance[SBIndex::DIVINE_SAVE_CHANCE] += effect_value;
					new_bonus->DivineSaveChance[SBIndex::DIVINE_SAVE_SPELL_TRIGGER_ID] = 0;
				}

				else if(new_bonus->DivineSaveChance[SBIndex::DIVINE_SAVE_CHANCE] < effect_value)
				{
					new_bonus->DivineSaveChance[SBIndex::DIVINE_SAVE_CHANCE]           = effect_value;
					new_bonus->DivineSaveChance[SBIndex::DIVINE_SAVE_SPELL_TRIGGER_ID] = limit_value;
				}
				break;
			}

			case SE_Flurry:
				new_bonus->FlurryChance += effect_value;
				break;

			case SE_Accuracy:
			{
				// Bad data or unsupported new skill
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;
				if ((effect_value < 0) && (new_bonus->Accuracy[EQ::skills::HIGHEST_SKILL + 1] > effect_value))
					new_bonus->Accuracy[EQ::skills::HIGHEST_SKILL + 1] = effect_value;

				else if (!new_bonus->Accuracy[EQ::skills::HIGHEST_SKILL + 1] ||
					((new_bonus->Accuracy[EQ::skills::HIGHEST_SKILL + 1] > 0) && (new_bonus->Accuracy[EQ::skills::HIGHEST_SKILL + 1] < effect_value)))
					new_bonus->Accuracy[EQ::skills::HIGHEST_SKILL + 1] = effect_value;
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
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;
				//When using npc_spells_effects if MAX value set, use stackable quest based modifier.
				if (IsAISpellEffect && max_value){
					if(limit_value == ALL_SKILLS)
						SkillDmgTaken_Mod[EQ::skills::HIGHEST_SKILL + 1] = effect_value;
					else
						SkillDmgTaken_Mod[limit_value] = effect_value;
				}
				else {

					if(limit_value == ALL_SKILLS)
						new_bonus->SkillDmgTaken[EQ::skills::HIGHEST_SKILL + 1] += effect_value;
					else
						new_bonus->SkillDmgTaken[limit_value] += effect_value;

				}
				break;
			}

			case SE_SpellCritChance:
				new_bonus->CriticalSpellChance += effect_value;
				break;

			case SE_CriticalSpellChance:
			{
				new_bonus->CriticalSpellChance += effect_value;

				if (limit_value > new_bonus->SpellCritDmgIncNoStack)
					new_bonus->SpellCritDmgIncNoStack = limit_value;
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
				/*
				Bard songs have identical negative base value and positive max
				The effect for the songs should increase mitigation. There are
				spells that do decrease the mitigation with just negative base values.
				To be consistent all values that increase mitigation will be set to positives
				*/
				if (max_value > 0 && effect_value < 0) {
					effect_value = max_value;
				}

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
						new_bonus->SpellOnKill[e] = limit_value;
						new_bonus->SpellOnKill[e+1] = effect_value;
						new_bonus->SpellOnKill[e+2] = max_value;
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
						new_bonus->SpellOnDeath[e] = limit_value;
						new_bonus->SpellOnDeath[e+1] = effect_value;
						break;
					}
				}
				break;
			}

			case SE_CriticalDamageMob:
			{
				// Bad data or unsupported new skill
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;
				if(limit_value == ALL_SKILLS)
					new_bonus->CritDmgMod[EQ::skills::HIGHEST_SKILL + 1] += effect_value;
				else
					new_bonus->CritDmgMod[limit_value] += effect_value;
				break;
			}

			case SE_Critical_Melee_Damage_Mod_Max:
			{
				// Bad data or unsupported new skill
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;
				int skill = limit_value == ALL_SKILLS ? EQ::skills::HIGHEST_SKILL + 1 : limit_value;
				if (effect_value < 0 && new_bonus->CritDmgModNoStack[skill] > effect_value)
					new_bonus->CritDmgModNoStack[skill] = effect_value;
				else if (effect_value > 0 && new_bonus->CritDmgModNoStack[skill] < effect_value) {
					new_bonus->CritDmgModNoStack[skill] = effect_value;
				}
				break;
			}

			case SE_ReduceSkillTimer:
			{
				if(new_bonus->SkillReuseTime[limit_value] < effect_value)
					new_bonus->SkillReuseTime[limit_value] = effect_value;
				break;
			}

			case SE_SkillDamageAmount:
			{
				// Bad data or unsupported new skill
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;
				if(limit_value == ALL_SKILLS)
					new_bonus->SkillDamageAmount[EQ::skills::HIGHEST_SKILL + 1] += effect_value;
				else
					new_bonus->SkillDamageAmount[limit_value] += effect_value;
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

			case SE_Hunger:
				new_bonus->hunger = true;
				break;

			case SE_IncreaseBlockChance:
				if (AdditiveWornBonus)
					new_bonus->IncreaseBlockChance += effect_value;
				else if (effect_value < 0 && new_bonus->IncreaseBlockChance > effect_value)
					new_bonus->IncreaseBlockChance = effect_value;
				else if (new_bonus->IncreaseBlockChance < effect_value)
					new_bonus->IncreaseBlockChance = effect_value;
				break;

			case SE_PersistantCasting:
				new_bonus->PersistantCasting += effect_value;
				break;

			case SE_LimitHPPercent:
			{
				if(new_bonus->HPPercCap[SBIndex::RESOURCE_PERCENT_CAP] != 0 && new_bonus->HPPercCap[SBIndex::RESOURCE_PERCENT_CAP] > effect_value){
					new_bonus->HPPercCap[SBIndex::RESOURCE_PERCENT_CAP] = effect_value;
					new_bonus->HPPercCap[SBIndex::RESOURCE_AMOUNT_CAP]  = limit_value;
				}
				else if(new_bonus->HPPercCap[SBIndex::RESOURCE_PERCENT_CAP] == 0){
					new_bonus->HPPercCap[SBIndex::RESOURCE_PERCENT_CAP] = effect_value;
					new_bonus->HPPercCap[SBIndex::RESOURCE_AMOUNT_CAP]  = limit_value;
				}
				break;
			}
			case SE_LimitManaPercent:
			{
				if(new_bonus->ManaPercCap[SBIndex::RESOURCE_PERCENT_CAP] != 0 && new_bonus->ManaPercCap[SBIndex::RESOURCE_PERCENT_CAP] > effect_value){
					new_bonus->ManaPercCap[SBIndex::RESOURCE_PERCENT_CAP] = effect_value;
					new_bonus->ManaPercCap[SBIndex::RESOURCE_AMOUNT_CAP]  = limit_value;
				}
				else if(new_bonus->ManaPercCap[SBIndex::RESOURCE_PERCENT_CAP] == 0) {
					new_bonus->ManaPercCap[SBIndex::RESOURCE_PERCENT_CAP] = effect_value;
					new_bonus->ManaPercCap[SBIndex::RESOURCE_AMOUNT_CAP]  = limit_value;
				}

				break;
			}
			case SE_LimitEndPercent:
			{
				if(new_bonus->EndPercCap[SBIndex::RESOURCE_PERCENT_CAP] != 0 && new_bonus->EndPercCap[SBIndex::RESOURCE_PERCENT_CAP] > effect_value) {
					new_bonus->EndPercCap[SBIndex::RESOURCE_PERCENT_CAP] = effect_value;
					new_bonus->EndPercCap[SBIndex::RESOURCE_AMOUNT_CAP]  = limit_value;
				}

				else if(new_bonus->EndPercCap[SBIndex::RESOURCE_PERCENT_CAP] == 0){
					new_bonus->EndPercCap[SBIndex::RESOURCE_PERCENT_CAP] = effect_value;
					new_bonus->EndPercCap[SBIndex::RESOURCE_AMOUNT_CAP]  = limit_value;
				}

				break;
			}

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
				new_bonus->HPToManaConvert = spells[spell_id].base_value[i];
				break;
			}

			case SE_SkillDamageAmount2:
			{
				// Bad data or unsupported new skill
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;
				if(limit_value == ALL_SKILLS)
					new_bonus->SkillDamageAmount2[EQ::skills::HIGHEST_SKILL + 1] += effect_value;
				else
					new_bonus->SkillDamageAmount2[limit_value] += effect_value;
				break;
			}

			case SE_NegateAttacks:
			{
				if (!new_bonus->NegateAttacks[SBIndex::NEGATE_ATK_EXISTS] ||
					((new_bonus->NegateAttacks[SBIndex::NEGATE_ATK_EXISTS] && new_bonus->NegateAttacks[SBIndex::NEGATE_ATK_MAX_DMG_ABSORB_PER_HIT]) && (new_bonus->NegateAttacks[SBIndex::NEGATE_ATK_MAX_DMG_ABSORB_PER_HIT] < max_value))){
					new_bonus->NegateAttacks[SBIndex::NEGATE_ATK_EXISTS]                 = 1;
					new_bonus->NegateAttacks[SBIndex::NEGATE_ATK_BUFFSLOT]               = buffslot;
					new_bonus->NegateAttacks[SBIndex::NEGATE_ATK_MAX_DMG_ABSORB_PER_HIT] = max_value;
				}
				break;
			}

			case SE_MitigateMeleeDamage:
			{
				if (new_bonus->MitigateMeleeRune[SBIndex::MITIGATION_RUNE_PERCENT] < effect_value){
					new_bonus->MitigateMeleeRune[SBIndex::MITIGATION_RUNE_PERCENT]                = effect_value;
					new_bonus->MitigateMeleeRune[SBIndex::MITIGATION_RUNE_BUFFSLOT]               = buffslot;
					new_bonus->MitigateMeleeRune[SBIndex::MITIGATION_RUNE_MAX_DMG_ABSORB_PER_HIT] = limit_value;
					new_bonus->MitigateMeleeRune[SBIndex::MITIGATION_RUNE_MAX_HP_AMT]             = max_value;
				}
				break;
			}


			case SE_MeleeThresholdGuard:
			{
				if (new_bonus->MeleeThresholdGuard[SBIndex::THRESHOLDGUARD_MITIGATION_PERCENT] < effect_value){
					new_bonus->MeleeThresholdGuard[SBIndex::THRESHOLDGUARD_MITIGATION_PERCENT]  = effect_value;
					new_bonus->MeleeThresholdGuard[SBIndex::THRESHOLDGUARD_BUFFSLOT]           = buffslot;
					new_bonus->MeleeThresholdGuard[SBIndex::THRESHOLDGUARD_MIN_DMG_TO_TRIGGER] = limit_value;
				}
				break;
			}

			case SE_SpellThresholdGuard:
			{
				if (new_bonus->SpellThresholdGuard[SBIndex::THRESHOLDGUARD_MITIGATION_PERCENT] < effect_value){
					new_bonus->SpellThresholdGuard[SBIndex::THRESHOLDGUARD_MITIGATION_PERCENT]  = effect_value;
					new_bonus->SpellThresholdGuard[SBIndex::THRESHOLDGUARD_BUFFSLOT]           = buffslot;
					new_bonus->SpellThresholdGuard[SBIndex::THRESHOLDGUARD_MIN_DMG_TO_TRIGGER] = limit_value;
				}
				break;
			}

			case SE_MitigateSpellDamage:
			{
				if (WornType) {
					new_bonus->MitigateSpellRune[SBIndex::MITIGATION_RUNE_PERCENT] += effect_value;
				}

				if (new_bonus->MitigateSpellRune[SBIndex::MITIGATION_RUNE_PERCENT] < effect_value){
					new_bonus->MitigateSpellRune[SBIndex::MITIGATION_RUNE_PERCENT]                = effect_value;
					new_bonus->MitigateSpellRune[SBIndex::MITIGATION_RUNE_BUFFSLOT]               = buffslot;
					new_bonus->MitigateSpellRune[SBIndex::MITIGATION_RUNE_MAX_DMG_ABSORB_PER_HIT] = limit_value;
					new_bonus->MitigateSpellRune[SBIndex::MITIGATION_RUNE_MAX_HP_AMT]             = max_value;
				}
				break;
			}

			case SE_MitigateDotDamage:
			{
				if (WornType) {
					new_bonus->MitigateDotRune[SBIndex::MITIGATION_RUNE_PERCENT] += effect_value;
				}

				if (new_bonus->MitigateDotRune[SBIndex::MITIGATION_RUNE_PERCENT] < effect_value){
					new_bonus->MitigateDotRune[SBIndex::MITIGATION_RUNE_PERCENT]                = effect_value;
					new_bonus->MitigateDotRune[SBIndex::MITIGATION_RUNE_BUFFSLOT]               = buffslot;
					new_bonus->MitigateDotRune[SBIndex::MITIGATION_RUNE_MAX_DMG_ABSORB_PER_HIT] = limit_value;
					new_bonus->MitigateDotRune[SBIndex::MITIGATION_RUNE_MAX_HP_AMT]             = max_value;
				}
				break;
			}

			case SE_ManaAbsorbPercentDamage:
			{
				if (new_bonus->ManaAbsorbPercentDamage < effect_value){
					new_bonus->ManaAbsorbPercentDamage = effect_value;
				}
				break;
			}

			case SE_Endurance_Absorb_Pct_Damage:
			{
				if (new_bonus->EnduranceAbsorbPercentDamage[SBIndex::ENDURANCE_ABSORD_MITIGIATION] < effect_value) {
					new_bonus->EnduranceAbsorbPercentDamage[SBIndex::ENDURANCE_ABSORD_MITIGIATION]  = effect_value;
					new_bonus->EnduranceAbsorbPercentDamage[SBIndex::ENDURANCE_ABSORD_DRAIN_PER_HP] = limit_value;
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

			case SE_Double_Backstab_Front:
				new_bonus->Double_Backstab_Front += effect_value;
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
				switch (limit_value) {
				case EQ::item::ItemTypeWindInstrument:
					new_bonus->windMod += effect_value;
					break;
				case EQ::item::ItemTypeStringedInstrument:
					new_bonus->stringedMod += effect_value;
					break;
				case EQ::item::ItemTypeBrassInstrument:
					new_bonus->brassMod += effect_value;
					break;
				case EQ::item::ItemTypePercussionInstrument:
					new_bonus->percussionMod += effect_value;
					break;
				case EQ::item::ItemTypeSinging:
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
					if(new_bonus->SEResist[e+1] && (new_bonus->SEResist[e] == limit_value) && (new_bonus->SEResist[e+1] < effect_value)){
						new_bonus->SEResist[e] = limit_value; //Spell Effect ID
						new_bonus->SEResist[e+1] = effect_value; //Resist Chance
						break;
					}
					else if (!new_bonus->SEResist[e+1]){
						new_bonus->SEResist[e] = limit_value; //Spell Effect ID
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
				if(new_bonus->GiveDoubleRiposte[limit_value] == 0){
					if(new_bonus->GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_CHANCE] < effect_value)
						new_bonus->GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_CHANCE] = effect_value;
				}
				break;
			}

			case SE_SlayUndead:
			{
				if(new_bonus->SlayUndead[SBIndex::SLAYUNDEAD_DMG_MOD] < effect_value)
					new_bonus->SlayUndead[SBIndex::SLAYUNDEAD_RATE_MOD] = effect_value; // Rate
					new_bonus->SlayUndead[SBIndex::SLAYUNDEAD_DMG_MOD]  = limit_value; // Damage Modifier
				break;
			}

			case SE_TriggerOnReqTarget:
			case SE_TriggerOnReqCaster:
				new_bonus->TriggerOnCastRequirement = true;
				break;

			case SE_DivineAura:
				new_bonus->DivineAura = true;
				break;

			case SE_ImprovedTaunt:
				if (new_bonus->ImprovedTaunt[SBIndex::IMPROVED_TAUNT_MAX_LVL] < effect_value) {
					new_bonus->ImprovedTaunt[SBIndex::IMPROVED_TAUNT_MAX_LVL]   = effect_value;
					new_bonus->ImprovedTaunt[SBIndex::IMPROVED_TAUNT_AGGRO_MOD] = limit_value;
					new_bonus->ImprovedTaunt[SBIndex::IMPROVED_TAUNT_BUFFSLOT]  = buffslot;
				}
				break;


			case SE_DistanceRemoval:
				new_bonus->DistanceRemoval = true;
				break;

			case SE_FrenziedDevastation:
				new_bonus->FrenziedDevastation += limit_value;
				break;

			case SE_Root:
				if (new_bonus->Root[SBIndex::ROOT_EXISTS] && (new_bonus->Root[SBIndex::ROOT_BUFFSLOT] > buffslot)){
					new_bonus->Root[SBIndex::ROOT_EXISTS]   = 1;
					new_bonus->Root[SBIndex::ROOT_BUFFSLOT] = buffslot;
				}
				else if (!new_bonus->Root[SBIndex::ROOT_EXISTS]){
					new_bonus->Root[SBIndex::ROOT_EXISTS]   = 1;
					new_bonus->Root[SBIndex::ROOT_BUFFSLOT] = buffslot;
				}
				break;

			case SE_Rune:

				if (new_bonus->MeleeRune[SBIndex::RUNE_AMOUNT] && (new_bonus->MeleeRune[SBIndex::RUNE_BUFFSLOT] > buffslot)){

					new_bonus->MeleeRune[SBIndex::RUNE_AMOUNT]   = effect_value;
					new_bonus->MeleeRune[SBIndex::RUNE_BUFFSLOT] = buffslot;
				}
				else if (!new_bonus->MeleeRune[SBIndex::RUNE_AMOUNT]){
					new_bonus->MeleeRune[SBIndex::RUNE_AMOUNT]   = effect_value;
					new_bonus->MeleeRune[SBIndex::RUNE_BUFFSLOT] = buffslot;
				}

				break;

			case SE_AbsorbMagicAtt:
				if (new_bonus->AbsorbMagicAtt[SBIndex::RUNE_AMOUNT] && (new_bonus->AbsorbMagicAtt[SBIndex::RUNE_BUFFSLOT] > buffslot)){
					new_bonus->AbsorbMagicAtt[SBIndex::RUNE_AMOUNT]   = effect_value;
					new_bonus->AbsorbMagicAtt[SBIndex::RUNE_BUFFSLOT] = buffslot;
				}
				else if (!new_bonus->AbsorbMagicAtt[SBIndex::RUNE_AMOUNT]){
					new_bonus->AbsorbMagicAtt[SBIndex::RUNE_AMOUNT]   = effect_value;
					new_bonus->AbsorbMagicAtt[SBIndex::RUNE_BUFFSLOT] = buffslot;
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
				new_bonus->AStacker[SBIndex::BUFFSTACKER_EXISTS] = 1;
				new_bonus->AStacker[SBIndex::BUFFSTACKER_VALUE]  = effect_value;
				break;

			case SE_BStacker:
				new_bonus->BStacker[SBIndex::BUFFSTACKER_EXISTS] = 1;
				new_bonus->BStacker[SBIndex::BUFFSTACKER_VALUE]  = effect_value;
				break;

			case SE_CStacker:
				new_bonus->CStacker[SBIndex::BUFFSTACKER_EXISTS] = 1;
				new_bonus->CStacker[SBIndex::BUFFSTACKER_VALUE]  = effect_value;
				break;

			case SE_DStacker:
				new_bonus->DStacker[SBIndex::BUFFSTACKER_EXISTS] = 1;
				new_bonus->DStacker[SBIndex::BUFFSTACKER_VALUE]  = effect_value;
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
				if(new_bonus->HeadShot[SBIndex::FINISHING_EFFECT_DMG] < limit_value){
					new_bonus->HeadShot[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value;
					new_bonus->HeadShot[SBIndex::FINISHING_EFFECT_DMG]         = limit_value;
				}
				break;
			}

			case SE_HeadShotLevel:
			{
				if(new_bonus->HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX] < effect_value) {
					new_bonus->HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX]          = effect_value;
					new_bonus->HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS] = limit_value;
				}
				break;
			}

			case SE_Assassinate:
			{
				if(new_bonus->Assassinate[SBIndex::FINISHING_EFFECT_DMG] < limit_value){
					new_bonus->Assassinate[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value;
					new_bonus->Assassinate[SBIndex::FINISHING_EFFECT_DMG]         = limit_value;
				}
				break;
			}

			case SE_AssassinateLevel:
			{
				if(new_bonus->AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX] < effect_value) {
					new_bonus->AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX]          = effect_value;
					new_bonus->AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS] = limit_value;
				}
				break;
			}

			case SE_FinishingBlow:
			{
				//base1 = chance, base2 = damage
				if (new_bonus->FinishingBlow[SBIndex::FINISHING_EFFECT_DMG] < limit_value){
					new_bonus->FinishingBlow[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value;
					new_bonus->FinishingBlow[SBIndex::FINISHING_EFFECT_DMG]         = limit_value;
				}
				break;
			}

			case SE_FinishingBlowLvl:
			{
				if (new_bonus->FinishingBlowLvl[SBIndex::FINISHING_EFFECT_LEVEL_MAX] < effect_value){
					new_bonus->FinishingBlowLvl[SBIndex::FINISHING_EFFECT_LEVEL_MAX]    = effect_value;
					new_bonus->FinishingBlowLvl[SBIndex::FINISHING_BLOW_LEVEL_HP_RATIO] = limit_value;
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
				new_bonus->IllusionPersistence = effect_value;
				break;

			case SE_LimitToSkill:{
				// Bad data or unsupported new skill
				if (effect_value > EQ::skills::HIGHEST_SKILL) {
					break;
				}
				if (effect_value <= EQ::skills::HIGHEST_SKILL){
					new_bonus->LimitToSkill[effect_value] = true;
					new_bonus->LimitToSkill[EQ::skills::HIGHEST_SKILL + 3] = true; //Used as a general exists check
				}
				break;
			}

			case SE_SkillProcAttempt:{

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

			case SE_SkillAttackProc: {
				for (int i = 0; i < MAX_CAST_ON_SKILL_USE i += 3) { 
					if (!new_bonus->SkillAttackProc[i + SBIndex::SKILLATK_PROC_SPELL_ID]) { // spell id
						new_bonus->SkillAttackProc[i + SBIndex::SKILLATK_PROC_SPELL_ID] = max_value; // spell to proc
						new_bonus->SkillAttackProc[i + SBIndex::SKILLATK_PROC_CHANCE] = effect_value; // Chance base 1000 = 100% proc rate
						new_bonus->SkillAttackProc[i + SBIndex::SKILLATK_PROC_SKILL] = limit_value; // Skill to Proc Offr
						
						if (limit_value < EQ::skills::HIGHEST_SKILL) {
							new_bonus->HasSkillAttackProc[limit_value] = true; //check first before looking for any effects.
						}
						break;
					}
				}
			}

			case SE_PC_Pet_Rampage: {
				new_bonus->PC_Pet_Rampage[SBIndex::PET_RAMPAGE_CHANCE] += effect_value; //Chance to rampage
				if (new_bonus->PC_Pet_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] < limit_value)
					new_bonus->PC_Pet_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] = limit_value; //Damage modifer - take highest
				break;
			}

			case SE_PC_Pet_AE_Rampage: {
				new_bonus->PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_CHANCE] += effect_value; //Chance to rampage
				if (new_bonus->PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] < limit_value)
					new_bonus->PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] = limit_value; //Damage modifer - take highest
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

				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;

				new_bonus->ReduceTradeskillFail[limit_value] += effect_value;
				break;
			}

			case SE_TradeSkillMastery:
				if (new_bonus->TradeSkillMastery < effect_value)
					new_bonus->TradeSkillMastery = effect_value;
				break;

			case SE_RaiseSkillCap: {
				if (limit_value > EQ::skills::HIGHEST_SKILL)
					break;

				if (new_bonus->RaiseSkillCap[limit_value] < effect_value)
					new_bonus->RaiseSkillCap[limit_value] = effect_value;
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

			case SE_AdditionalAura:
				if (new_bonus->aura_slots < effect_value)
					new_bonus->aura_slots = effect_value;
				break;

			case SE_IncreaseTrapCount:
				if (new_bonus->trap_slots < effect_value)
					new_bonus->trap_slots = effect_value;
				break;

			case SE_Attack_Accuracy_Max_Percent:
				new_bonus->Attack_Accuracy_Max_Percent += effect_value;
				break;


			case SE_AC_Mitigation_Max_Percent:
				new_bonus->AC_Mitigation_Max_Percent += effect_value;
				break;

			case SE_AC_Avoidance_Max_Percent:
				new_bonus->AC_Avoidance_Max_Percent += effect_value;
				break;

			case SE_Damage_Taken_Position_Mod:
			{
				//Mitigate if damage taken from behind base2 = 0, from front base2 = 1
				if (limit_value < 0 || limit_value > 2)
					break;
				if (AdditiveWornBonus)
					new_bonus->Damage_Taken_Position_Mod[limit_value] += effect_value;
				else if (effect_value < 0 && new_bonus->Damage_Taken_Position_Mod[limit_value] > effect_value)
					new_bonus->Damage_Taken_Position_Mod[limit_value] = effect_value;
				else if (effect_value > 0 && new_bonus->Damage_Taken_Position_Mod[limit_value] < effect_value)
					new_bonus->Damage_Taken_Position_Mod[limit_value] = effect_value;
				break;
			}

			case SE_Melee_Damage_Position_Mod:
			{
				//Increase damage by percent from behind base2 = 0, from front base2 = 1
				if (limit_value < 0 || limit_value > 2)
					break;
				if (AdditiveWornBonus)
					new_bonus->Melee_Damage_Position_Mod[limit_value] += effect_value;
				else if (effect_value < 0 && new_bonus->Melee_Damage_Position_Mod[limit_value] > effect_value)
					new_bonus->Melee_Damage_Position_Mod[limit_value] = effect_value;
				else if (effect_value > 0 && new_bonus->Melee_Damage_Position_Mod[limit_value] < effect_value)
					new_bonus->Melee_Damage_Position_Mod[limit_value] = effect_value;
				break;
			}

			case SE_Damage_Taken_Position_Amt:
			{
				//Mitigate if damage taken from behind base2 = 0, from front base2 = 1
				if (limit_value < 0 || limit_value > 2)
					break;

				new_bonus->Damage_Taken_Position_Amt[limit_value] += effect_value;
				break;
			}

			case SE_Melee_Damage_Position_Amt:
			{
				//Mitigate if damage taken from behind base2 = 0, from front base2 = 1
				if (limit_value < 0 || limit_value > 2)
					break;

				new_bonus->Melee_Damage_Position_Amt[limit_value] += effect_value;
				break;
			}

			case SE_DS_Mitigation_Amount:
				new_bonus->DS_Mitigation_Amount += effect_value;
				break;

			case SE_DS_Mitigation_Percentage:
				new_bonus->DS_Mitigation_Percentage += effect_value;
				break;

			case SE_Pet_Crit_Melee_Damage_Pct_Owner:
				new_bonus->Pet_Crit_Melee_Damage_Pct_Owner += effect_value;
				break;

			case SE_Pet_Add_Atk:
				new_bonus->Pet_Add_Atk += effect_value;
				break;

			case SE_ExtendedShielding:
			{
				if (AdditiveWornBonus) {
					new_bonus->ExtendedShielding += effect_value;
				}
				else if (effect_value < 0 && new_bonus->ExtendedShielding > effect_value){
					new_bonus->ExtendedShielding = effect_value;
				}
				else if (effect_value > 0 && new_bonus->ExtendedShielding < effect_value){
					new_bonus->ExtendedShielding = effect_value;
				}
				break;
			}

			case SE_ShieldDuration:
			{
				if (AdditiveWornBonus) {
					new_bonus->ShieldDuration += effect_value;
				}
				else if (effect_value < 0 && new_bonus->ShieldDuration > effect_value){
					new_bonus->ShieldDuration = effect_value;
				}
				else if (effect_value > 0 && new_bonus->ShieldDuration < effect_value){
					new_bonus->ShieldDuration = effect_value;
				}
				break;
			}

			case SE_Worn_Endurance_Regen_Cap:
				new_bonus->ItemEnduranceRegenCap += effect_value;
				break;

			case SE_ItemManaRegenCapIncrease:
				new_bonus->ItemManaRegenCap += effect_value;
				break;

			case SE_Weapon_Stance: {
				if (IsValidSpell(effect_value)) { //base1 is the spell_id of buff
					if (limit_value <= WEAPON_STANCE_TYPE_MAX) { //0=2H, 1=Shield, 2=DW
						if (IsValidSpell(new_bonus->WeaponStance[limit_value])) { //Check if we already a spell_id saved for this effect
							if (spells[new_bonus->WeaponStance[limit_value]].rank < spells[effect_value].rank) { //If so, check if any new spellids with higher rank exist (live spells for this are ranked).
								new_bonus->WeaponStance[limit_value] = effect_value; //Overwrite with new effect
								SetWeaponStanceEnabled(true);
								
								if (WornType) {
									weaponstance.itembonus_enabled = true;
								}
								else {
									weaponstance.spellbonus_enabled = true;
								}
							}
						}
						else {
							new_bonus->WeaponStance[limit_value] = effect_value; //If no prior effect exists, then apply
							SetWeaponStanceEnabled(true);

							if (WornType) {
								weaponstance.itembonus_enabled = true;
							}
							else {
								weaponstance.spellbonus_enabled = true;
							}
						}
					}
				}
				break;
			}

			case SE_ZoneSuspendMinion:
				new_bonus->ZoneSuspendMinion = effect_value;
				break;

			case SE_CompleteHeal:
				new_bonus->CompleteHealBuffBlocker = true;
				break;

			case SE_TrapCircumvention:
				new_bonus->TrapCircumvention += effect_value;
				break;

			//Special custom cases for loading effects on to NPC from 'npc_spels_effects' table
			if (IsAISpellEffect) {

				//Non-Focused Effect to modify incoming spell damage by resist type.
				case SE_FcSpellVulnerability:
					ModVulnerability(limit_value, effect_value);
				break;
			}
		}
	}
}

void NPC::CalcItemBonuses(StatBonuses *newbon)
{
	if(newbon){

		for (int i = EQ::invslot::BONUS_BEGIN; i <= EQ::invslot::BONUS_STAT_END; i++){
			const EQ::ItemData *cur = database.GetItem(equipment[i]);
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
					newbon->SpellShield += cur->SpellShield;
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
				if (cur->Worn.Effect>0 && (cur->Worn.Type == EQ::item::ItemEffectWorn)) { // latent effects
					ApplySpellsBonuses(cur->Worn.Effect, cur->Worn.Level, newbon, 0, cur->Worn.Type);
				}

				if (RuleB(Spells, NPC_UseFocusFromItems)){
					if (cur->Focus.Effect>0 && (cur->Focus.Type == EQ::item::ItemEffectFocus)){  // focus effects
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
	if (CalcItemScale(EQ::invslot::EQUIPMENT_BEGIN, EQ::invslot::EQUIPMENT_END)) // original coding excluded MainAmmo (< 21)
		changed = true;

	if (CalcItemScale(EQ::invslot::GENERAL_BEGIN, EQ::invslot::GENERAL_END)) // original coding excluded MainCursor (< 30)
		changed = true;

	// I excluded cursor bag slots here because cursor was excluded above..if this is incorrect, change 'slot_y' here to CURSOR_BAG_END
	// and 'slot_y' above to CURSOR from GENERAL_END above - or however it is supposed to be...
	if (CalcItemScale(EQ::invbag::GENERAL_BAGS_BEGIN, EQ::invbag::GENERAL_BAGS_END)) // (< 341)
		changed = true;

	if (CalcItemScale(EQ::invslot::TRIBUTE_BEGIN, EQ::invslot::TRIBUTE_END)) // (< 405)
		changed = true;

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
		if (i == EQ::invslot::slotAmmo) // moved here from calling procedure to facilitate future range changes where MainAmmo may not be the last slot
			continue;

		EQ::ItemInstance* inst = m_inv.GetItem(i);

		if(inst == nullptr)
			continue;

		// TEST CODE: test for bazaar trader crashing with charm items
		if (Trader)
			if (i >= EQ::invbag::GENERAL_BAGS_BEGIN && i <= EQ::invbag::GENERAL_BAGS_END) {
				EQ::ItemInstance* parent_item = m_inv.GetItem(EQ::InventoryProfile::CalcSlotId(i));
				if (parent_item && parent_item->GetItem()->BagType == EQ::item::BagTypeTradersSatchel)
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
		for (int x = EQ::invaug::SOCKET_BEGIN; x <= EQ::invaug::SOCKET_END; ++x)
		{
			EQ::ItemInstance * a_inst = inst->GetAugment(x);
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
	if (DoItemEnterZone(EQ::invslot::EQUIPMENT_BEGIN, EQ::invslot::EQUIPMENT_END)) // original coding excluded MainAmmo (< 21)
		changed = true;

	if (DoItemEnterZone(EQ::invslot::GENERAL_BEGIN, EQ::invslot::GENERAL_END)) // original coding excluded MainCursor (< 30)
		changed = true;

	// I excluded cursor bag slots here because cursor was excluded above..if this is incorrect, change 'slot_y' here to CURSOR_BAG_END
	// and 'slot_y' above to CURSOR from GENERAL_END above - or however it is supposed to be...
	if (DoItemEnterZone(EQ::invbag::GENERAL_BAGS_BEGIN, EQ::invbag::GENERAL_BAGS_END)) // (< 341)
		changed = true;

	if (DoItemEnterZone(EQ::invslot::TRIBUTE_BEGIN, EQ::invslot::TRIBUTE_END)) // (< 405)
		changed = true;

	if(changed)
	{
		CalcBonuses();
	}
}

bool Client::DoItemEnterZone(uint32 slot_x, uint32 slot_y) {
	// behavior change: 'slot_y' is now [RANGE]_END and not [RANGE]_END + 1
	bool changed = false;
	for(uint32 i = slot_x; i <= slot_y; i++) {
		if (i == EQ::invslot::slotAmmo) // moved here from calling procedure to facilitate future range changes where MainAmmo may not be the last slot
			continue;

		EQ::ItemInstance* inst = m_inv.GetItem(i);

		if(!inst)
			continue;

		// TEST CODE: test for bazaar trader crashing with charm items
		if (Trader)
			if (i >= EQ::invbag::GENERAL_BAGS_BEGIN && i <= EQ::invbag::GENERAL_BAGS_END) {
				EQ::ItemInstance* parent_item = m_inv.GetItem(EQ::InventoryProfile::CalcSlotId(i));
				if (parent_item && parent_item->GetItem()->BagType == EQ::item::BagTypeTradersSatchel)
					continue;
			}

		bool update_slot = false;
		if(inst->IsScaling())
		{
			uint16 oldexp = inst->GetExp();

			parse->EventItem(EVENT_ITEM_ENTER_ZONE, this, inst, nullptr, "", 0);
			if (i <= EQ::invslot::EQUIPMENT_END) {
				parse->EventItem(EVENT_EQUIP_ITEM, this, inst, nullptr, "", i);
			}

			if (inst->GetExp() != oldexp) {	// if the scaling factor changed, rescale the item and update the client
				inst->ScaleItem();
				changed = true;
				update_slot = true;
			}
		} else {
			if (i <= EQ::invslot::EQUIPMENT_END) {
				parse->EventItem(EVENT_EQUIP_ITEM, this, inst, nullptr, "", i);
			}

			parse->EventItem(EVENT_ITEM_ENTER_ZONE, this, inst, nullptr, "", 0);
		}

		//iterate all augments
		for (int x = EQ::invaug::SOCKET_BEGIN; x <= EQ::invaug::SOCKET_END; ++x)
		{
			EQ::ItemInstance *a_inst = inst->GetAugment(x);
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
		effect = spells[spell_id].effect_id[effect_index];
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
		case SE_Fc_ResistIncoming:
			focusFcResistIncoming;
		case SE_Fc_Amplify_Mod:
			focusFcAmplifyMod;
		case SE_Fc_Amplify_Amt:
			focusFcAmplifyAmt;
		case SE_SpellHateMod:
			return focusSpellHateMod;
		case SE_ReduceReuseTimer:
			return focusReduceRecastTime;
		case SE_TriggerOnCast:
			return focusTriggerOnCast;
		case SE_FcSpellVulnerability:
			return focusSpellVulnerability;
		case SE_Fc_Spell_Damage_Pct_IncomingPC:
			return focusFcSpellDamagePctIncomingPC;
		case SE_BlockNextSpellFocus:
			return focusBlockNextSpell;
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
		case SE_Fc_Spell_Damage_Amt_IncomingPC:
			return focusFcSpellDamageAmtIncomingPC;
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
		case SE_FcTimerLockout:
			return focusFcTimerLockout;
		case SE_Fc_Cast_Spell_On_Land:
			return focusFcCastSpellOnLand;
		case SE_FcStunTimeMod:
			return focusFcStunTimeMod;
		case SE_Fc_CastTimeMod2:
			return focusFcCastTimeMod2;
		case SE_Fc_CastTimeAmt:
			return focusFcCastTimeAmt;
		case SE_FcHealPctCritIncoming:
			return focusFcHealPctCritIncoming;
		case  SE_FcHealAmt:
			return focusFcHealAmt;
		case SE_FcHealAmtCrit:
			return focusFcHealAmtCrit;

	}
	return 0;
}

void Mob::NegateSpellEffectBonuses(uint16 spell_id)
{
	if (!IsValidSpell(spell_id))
		return;

	int effect_value = 0;

	for (int i = 0; i < EFFECT_COUNT; i++)
	{
		bool negate_spellbonus = false;
		bool negate_aabonus = false;
		bool negate_itembonus = false;

		if (spells[spell_id].effect_id[i] == SE_NegateSpellEffect) {

			//Set negate types
			switch (spells[spell_id].base_value[i])
			{
				case NEGATE_SPA_ALL_BONUSES:
					negate_spellbonus = true;
					negate_aabonus = true;
					negate_itembonus = true;
					break;

				case NEGATE_SPA_SPELLBONUS:
					negate_spellbonus = true;
					break;

				case NEGATE_SPA_ITEMBONUS:
					negate_itembonus = true;
					break;

				case NEGATE_SPA_AABONUS:
					negate_aabonus = true;
					break;

				case NEGATE_SPA_ITEMBONUS_AND_AABONUS:
					negate_aabonus = true;
					negate_itembonus = true;
					break;

				case NEGATE_SPA_SPELLBONUS_AND_AABONUS:
					negate_aabonus = true;
					negate_spellbonus = true;
					break;

				case NEGATE_SPA_SPELLBONUS_AND_ITEMBONUS:
					negate_spellbonus = true;
					negate_itembonus = true;
					break;
			}

			//Negate focus effects
			for (int e = 0; e < HIGHEST_FOCUS + 1; e++)
			{
				if (spellbonuses.FocusEffects[e] == spells[spell_id].limit_value[i])
				{
					if (negate_spellbonus) { spellbonuses.FocusEffects[e] = effect_value; }
					continue;
				}
			}

			//Negate bonuses
			switch (spells[spell_id].limit_value[i])
			{
				case SE_CurrentHP:
					if (negate_spellbonus) { spellbonuses.HPRegen = effect_value; }
					if (negate_aabonus) { aabonuses.HPRegen = effect_value; }
					if (negate_itembonus) { itembonuses.HPRegen = effect_value; }
					break;

				case SE_CurrentEndurance:
					if (negate_spellbonus) { spellbonuses.EnduranceRegen = effect_value; }
					if (negate_aabonus) { aabonuses.EnduranceRegen = effect_value; }
					if (negate_itembonus) { itembonuses.EnduranceRegen = effect_value; }
					break;

				case SE_ChangeFrenzyRad:
					if (negate_spellbonus) { spellbonuses.AggroRange = static_cast<float>(effect_value); }
					if (negate_aabonus) { aabonuses.AggroRange = static_cast<float>(effect_value); }
					if (negate_itembonus) { itembonuses.AggroRange = static_cast<float>(effect_value); }
					break;

				case SE_Harmony:
					if (negate_spellbonus) { spellbonuses.AssistRange = static_cast<float>(effect_value); }
					if (negate_aabonus) { aabonuses.AssistRange = static_cast<float>(effect_value); }
					if (negate_itembonus) { itembonuses.AssistRange = static_cast<float>(effect_value); }
					break;

				case SE_AttackSpeed:
					if (negate_spellbonus) { spellbonuses.haste = effect_value; }
					if (negate_aabonus) { aabonuses.haste = effect_value; }
					if (negate_itembonus) { itembonuses.haste = effect_value; }
					break;

				case SE_AttackSpeed2:
					if (negate_spellbonus) { spellbonuses.hastetype2 = effect_value; }
					if (negate_aabonus) { aabonuses.hastetype2 = effect_value; }
					if (negate_itembonus) { itembonuses.hastetype2 = effect_value; }
					break;

				case SE_AttackSpeed3:
				{
					if (effect_value > 0) {
						if (negate_spellbonus) { spellbonuses.hastetype3 = effect_value; }
						if (negate_aabonus) { aabonuses.hastetype3 = effect_value; }
						if (negate_itembonus) { itembonuses.hastetype3 = effect_value; }

					}
					break;
				}

				case SE_AttackSpeed4:
					if (negate_spellbonus) { spellbonuses.inhibitmelee = effect_value; }
					if (negate_aabonus) { aabonuses.inhibitmelee = effect_value; }
					if (negate_itembonus) { itembonuses.inhibitmelee = effect_value; }
					break;

				case SE_TotalHP:
					if (negate_spellbonus) { spellbonuses.HP = effect_value; }
					if (negate_aabonus) { aabonuses.HP = effect_value; }
					if (negate_itembonus) { itembonuses.HP = effect_value; }
					break;

				case SE_ManaRegen_v2:
				case SE_CurrentMana:
					if (negate_spellbonus) { spellbonuses.ManaRegen = effect_value; }
					if (negate_aabonus) { aabonuses.ManaRegen = effect_value; }
					if (negate_itembonus) { itembonuses.ManaRegen = effect_value; }
					break;

				case SE_ManaPool:
					if (negate_spellbonus) { spellbonuses.Mana = effect_value; }
					if (negate_itembonus) { itembonuses.Mana = effect_value; }
					if (negate_aabonus) { aabonuses.Mana = effect_value; }
					break;

				case SE_Stamina:
					if (negate_spellbonus) { spellbonuses.EnduranceReduction = effect_value; }
					if (negate_itembonus) { itembonuses.EnduranceReduction = effect_value; }
					if (negate_aabonus) { aabonuses.EnduranceReduction = effect_value; }
					break;

				case SE_ACv2:
				case SE_ArmorClass:
					if (negate_spellbonus) { spellbonuses.AC = effect_value; }
					if (negate_aabonus) { aabonuses.AC = effect_value; }
					if (negate_itembonus) { itembonuses.AC = effect_value; }
					break;

				case SE_ATK:
					if (negate_spellbonus) { spellbonuses.ATK = effect_value; }
					if (negate_aabonus) { aabonuses.ATK = effect_value; }
					if (negate_itembonus) { itembonuses.ATK = effect_value; }
					break;

				case SE_STR:
					if (negate_spellbonus) { spellbonuses.STR = effect_value; }
					if (negate_itembonus) { itembonuses.STR = effect_value; }
					if (negate_aabonus) { aabonuses.STR = effect_value; }
					break;

				case SE_DEX:
					if (negate_spellbonus) { spellbonuses.DEX = effect_value; }
					if (negate_aabonus) { aabonuses.DEX = effect_value; }
					if (negate_itembonus) { itembonuses.DEX = effect_value; }
					break;

				case SE_AGI:
					if (negate_itembonus) { itembonuses.AGI = effect_value; }
					if (negate_aabonus) { aabonuses.AGI = effect_value; }
					if (negate_spellbonus) { spellbonuses.AGI = effect_value; }
					break;

				case SE_STA:
					if (negate_spellbonus) { spellbonuses.STA = effect_value; }
					if (negate_itembonus) { itembonuses.STA = effect_value; }
					if (negate_aabonus) { aabonuses.STA = effect_value; }
					break;

				case SE_INT:
					if (negate_spellbonus) { spellbonuses.INT = effect_value; }
					if (negate_aabonus) { aabonuses.INT = effect_value; }
					if (negate_itembonus) { itembonuses.INT = effect_value; }
					break;

				case SE_WIS:
					if (negate_spellbonus) { spellbonuses.WIS = effect_value; }
					if (negate_aabonus) { aabonuses.WIS = effect_value; }
					if (negate_itembonus) { itembonuses.WIS = effect_value; }
					break;

				case SE_CHA:
					if (negate_itembonus) { itembonuses.CHA = effect_value; }
					if (negate_spellbonus) { spellbonuses.CHA = effect_value; }
					if (negate_aabonus) { aabonuses.CHA = effect_value; }
					break;

				case SE_AllStats:
				{
					if (negate_spellbonus) { spellbonuses.STR = effect_value; }
					if (negate_spellbonus) { spellbonuses.DEX = effect_value; }
					if (negate_spellbonus) { spellbonuses.AGI = effect_value; }
					if (negate_spellbonus) { spellbonuses.STA = effect_value; }
					if (negate_spellbonus) { spellbonuses.INT = effect_value; }
					if (negate_spellbonus) { spellbonuses.WIS = effect_value; }
					if (negate_spellbonus) { spellbonuses.CHA = effect_value; }

					if (negate_itembonus) { itembonuses.STR = effect_value; }
					if (negate_itembonus) { itembonuses.DEX = effect_value; }
					if (negate_itembonus) { itembonuses.AGI = effect_value; }
					if (negate_itembonus) { itembonuses.STA = effect_value; }
					if (negate_itembonus) { itembonuses.INT = effect_value; }
					if (negate_itembonus) { itembonuses.WIS = effect_value; }
					if (negate_itembonus) { itembonuses.CHA = effect_value; }

					if (negate_aabonus) { aabonuses.STR = effect_value; }
					if (negate_aabonus) { aabonuses.DEX = effect_value; }
					if (negate_aabonus) { aabonuses.AGI = effect_value; }
					if (negate_aabonus) { aabonuses.STA = effect_value; }
					if (negate_aabonus) { aabonuses.INT = effect_value; }
					if (negate_aabonus) { aabonuses.WIS = effect_value; }
					if (negate_aabonus) { aabonuses.CHA = effect_value; }
					break;
				}

				case SE_ResistFire:
					if (negate_spellbonus) { spellbonuses.FR = effect_value; }
					if (negate_itembonus) { itembonuses.FR = effect_value; }
					if (negate_aabonus) { aabonuses.FR = effect_value; }
					break;

				case SE_ResistCold:
					if (negate_spellbonus) { spellbonuses.CR = effect_value; }
					if (negate_aabonus) { aabonuses.CR = effect_value; }
					if (negate_itembonus) { itembonuses.CR = effect_value; }
					break;

				case SE_ResistPoison:
					if (negate_spellbonus) { spellbonuses.PR = effect_value; }
					if (negate_aabonus) { aabonuses.PR = effect_value; }
					if (negate_itembonus) { itembonuses.PR = effect_value; }
					break;

				case SE_ResistDisease:
					if (negate_spellbonus) { spellbonuses.DR = effect_value; }
					if (negate_itembonus) { itembonuses.DR = effect_value; }
					if (negate_aabonus) { aabonuses.DR = effect_value; }
					break;

				case SE_ResistMagic:
					if (negate_spellbonus) { spellbonuses.MR = effect_value; }
					if (negate_aabonus) { aabonuses.MR = effect_value; }
					if (negate_itembonus) { itembonuses.MR = effect_value; }
					break;

				case SE_ResistAll:
				{
					if (negate_spellbonus) { spellbonuses.MR = effect_value; }
					if (negate_spellbonus) { spellbonuses.DR = effect_value; }
					if (negate_spellbonus) { spellbonuses.PR = effect_value; }
					if (negate_spellbonus) { spellbonuses.CR = effect_value; }
					if (negate_spellbonus) { spellbonuses.FR = effect_value; }

					if (negate_aabonus) { aabonuses.MR = effect_value; }
					if (negate_aabonus) { aabonuses.DR = effect_value; }
					if (negate_aabonus) { aabonuses.PR = effect_value; }
					if (negate_aabonus) { aabonuses.CR = effect_value; }
					if (negate_aabonus) { aabonuses.FR = effect_value; }

					if (negate_itembonus) { itembonuses.MR = effect_value; }
					if (negate_itembonus) { itembonuses.DR = effect_value; }
					if (negate_itembonus) { itembonuses.PR = effect_value; }
					if (negate_itembonus) { itembonuses.CR = effect_value; }
					if (negate_itembonus) { itembonuses.FR = effect_value; }
					break;
				}

				case SE_ResistCorruption:
					if (negate_spellbonus) { spellbonuses.Corrup = effect_value; }
					if (negate_itembonus) { itembonuses.Corrup = effect_value; }
					if (negate_aabonus) { aabonuses.Corrup = effect_value; }
					break;

				case SE_CastingLevel:	// Brilliance of Ro
					if (negate_spellbonus) { spellbonuses.adjusted_casting_skill = effect_value; }
					if (negate_aabonus) { aabonuses.adjusted_casting_skill = effect_value; }
					if (negate_itembonus) { itembonuses.adjusted_casting_skill = effect_value; }
					break;

				case SE_CastingLevel2:
					if (negate_spellbonus) { spellbonuses.effective_casting_level = effect_value; }
					if (negate_aabonus) { aabonuses.effective_casting_level = effect_value; }
					if (negate_itembonus) { itembonuses.effective_casting_level = effect_value; }
					break;


				case SE_MovementSpeed:
					if (negate_spellbonus) { spellbonuses.movementspeed = effect_value; }
					if (negate_aabonus) { aabonuses.movementspeed = effect_value; }
					if (negate_itembonus) { itembonuses.movementspeed = effect_value; }
					break;

				case SE_SpellDamageShield:
					if (negate_spellbonus) { spellbonuses.SpellDamageShield = effect_value; }
					if (negate_aabonus) { aabonuses.SpellDamageShield = effect_value; }
					if (negate_itembonus) { itembonuses.SpellDamageShield = effect_value; }
					break;

				case SE_DamageShield:
					if (negate_spellbonus) { spellbonuses.DamageShield = effect_value; }
					if (negate_aabonus) { aabonuses.DamageShield = effect_value; }
					if (negate_itembonus) { itembonuses.DamageShield = effect_value; }
					break;

				case SE_ReverseDS:
					if (negate_spellbonus) { spellbonuses.ReverseDamageShield = effect_value; }
					if (negate_aabonus) { aabonuses.ReverseDamageShield = effect_value; }
					if (negate_itembonus) { itembonuses.ReverseDamageShield = effect_value; }
					break;

				case SE_Reflect:
					if (negate_spellbonus) { spellbonuses.reflect[SBIndex::REFLECT_CHANCE] = effect_value; }
					if (negate_aabonus) { aabonuses.reflect[SBIndex::REFLECT_CHANCE] = effect_value; }
					if (negate_itembonus) { itembonuses.reflect[SBIndex::REFLECT_CHANCE] = effect_value; }
					break;

				case SE_Amplification:
					if (negate_spellbonus) { spellbonuses.Amplification = effect_value; }
					if (negate_itembonus) { itembonuses.Amplification = effect_value; }
					if (negate_aabonus) { aabonuses.Amplification = effect_value; }
					break;

				case SE_ChangeAggro:
					if (negate_spellbonus) { spellbonuses.hatemod = effect_value; }
					if (negate_itembonus) { itembonuses.hatemod = effect_value; }
					if (negate_aabonus) { aabonuses.hatemod = effect_value; }
					break;

				case SE_MeleeMitigation:
					if (negate_spellbonus) { spellbonuses.MeleeMitigationEffect = effect_value; }
					if (negate_itembonus) { itembonuses.MeleeMitigationEffect = effect_value; }
					if (negate_aabonus) { aabonuses.MeleeMitigationEffect = effect_value; }
					break;

				case SE_CriticalHitChance:
				{
					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_spellbonus) { spellbonuses.CriticalHitChance[e] = effect_value; }
						if (negate_aabonus) { aabonuses.CriticalHitChance[e] = effect_value; }
						if (negate_itembonus) { itembonuses.CriticalHitChance[e] = effect_value; }
					}
				}

				case SE_CrippBlowChance:
					if (negate_spellbonus) { spellbonuses.CrippBlowChance = effect_value; }
					if (negate_aabonus) { aabonuses.CrippBlowChance = effect_value; }
					if (negate_itembonus) { itembonuses.CrippBlowChance = effect_value; }
					break;

				case SE_AvoidMeleeChance:
					if (negate_spellbonus) { spellbonuses.AvoidMeleeChanceEffect = effect_value; }
					if (negate_aabonus) { aabonuses.AvoidMeleeChanceEffect = effect_value; }
					if (negate_itembonus) { itembonuses.AvoidMeleeChanceEffect = effect_value; }
					break;

				case SE_RiposteChance:
					if (negate_spellbonus) { spellbonuses.RiposteChance = effect_value; }
					if (negate_aabonus) { aabonuses.RiposteChance = effect_value; }
					if (negate_itembonus) { itembonuses.RiposteChance = effect_value; }
					break;

				case SE_DodgeChance:
					if (negate_spellbonus) { spellbonuses.DodgeChance = effect_value; }
					if (negate_aabonus) { aabonuses.DodgeChance = effect_value; }
					if (negate_itembonus) { itembonuses.DodgeChance = effect_value; }
					break;

				case SE_ParryChance:
					if (negate_spellbonus) { spellbonuses.ParryChance = effect_value; }
					if (negate_aabonus) { aabonuses.ParryChance = effect_value; }
					if (negate_itembonus) { itembonuses.ParryChance = effect_value; }
					break;

				case SE_DualWieldChance:
					if (negate_spellbonus) { spellbonuses.DualWieldChance = effect_value; }
					if (negate_aabonus) { aabonuses.DualWieldChance = effect_value; }
					if (negate_itembonus) { itembonuses.DualWieldChance = effect_value; }
					break;

				case SE_DoubleAttackChance:
					if (negate_spellbonus) { spellbonuses.DoubleAttackChance = effect_value; }
					if (negate_aabonus) { aabonuses.DoubleAttackChance = effect_value; }
					if (negate_itembonus) { itembonuses.DoubleAttackChance = effect_value; }
					break;

				case SE_TripleAttackChance:
					if (negate_spellbonus) { spellbonuses.TripleAttackChance = effect_value; }
					if (negate_aabonus) { aabonuses.TripleAttackChance = effect_value; }
					if (negate_itembonus) { itembonuses.TripleAttackChance = effect_value; }
					break;

				case SE_MeleeLifetap:
					if (negate_spellbonus) { spellbonuses.MeleeLifetap = effect_value; }
					if (negate_aabonus) { aabonuses.MeleeLifetap = effect_value; }
					if (negate_itembonus) { itembonuses.MeleeLifetap = effect_value; }
					break;

				case SE_AllInstrumentMod:
				{
					if (negate_spellbonus) { spellbonuses.singingMod = effect_value; }
					if (negate_spellbonus) { spellbonuses.brassMod = effect_value; }
					if (negate_spellbonus) { spellbonuses.percussionMod = effect_value; }
					if (negate_spellbonus) { spellbonuses.windMod = effect_value; }
					if (negate_spellbonus) { spellbonuses.stringedMod = effect_value; }

					if (negate_itembonus) { itembonuses.singingMod = effect_value; }
					if (negate_itembonus) { itembonuses.brassMod = effect_value; }
					if (negate_itembonus) { itembonuses.percussionMod = effect_value; }
					if (negate_itembonus) { itembonuses.windMod = effect_value; }
					if (negate_itembonus) { itembonuses.stringedMod = effect_value; }

					if (negate_aabonus) { aabonuses.singingMod = effect_value; }
					if (negate_aabonus) { aabonuses.brassMod = effect_value; }
					if (negate_aabonus) { aabonuses.percussionMod = effect_value; }
					if (negate_aabonus) { aabonuses.windMod = effect_value; }
					if (negate_aabonus) { aabonuses.stringedMod = effect_value; }
					break;
				}

				case SE_ResistSpellChance:
					if (negate_spellbonus) { spellbonuses.ResistSpellChance = effect_value; }
					if (negate_aabonus) { aabonuses.ResistSpellChance = effect_value; }
					if (negate_itembonus) { itembonuses.ResistSpellChance = effect_value; }
					break;

				case SE_ResistFearChance:
					if (negate_spellbonus) { spellbonuses.Fearless = false; }
					if (negate_spellbonus) { spellbonuses.ResistFearChance = effect_value; }
					if (negate_aabonus) { aabonuses.ResistFearChance = effect_value; }
					if (negate_itembonus) { itembonuses.ResistFearChance = effect_value; }
					break;

				case SE_Fearless:
					if (negate_spellbonus) { spellbonuses.Fearless = false; }
					if (negate_aabonus) { aabonuses.Fearless = false; }
					if (negate_itembonus) { itembonuses.Fearless = false; }
					break;

				case SE_HundredHands:
					if (negate_spellbonus) { spellbonuses.HundredHands = effect_value; }
					if (negate_aabonus) { aabonuses.HundredHands = effect_value; }
					if (negate_itembonus) { itembonuses.HundredHands = effect_value; }
					break;

				case SE_MeleeSkillCheck:
				{
					if (negate_spellbonus) { spellbonuses.MeleeSkillCheck = effect_value; }
					if (negate_spellbonus) { spellbonuses.MeleeSkillCheckSkill = effect_value; }
					break;
				}

				case SE_HitChance:
				{
					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_spellbonus) { spellbonuses.HitChanceEffect[e] = effect_value; }
						if (negate_aabonus) { aabonuses.HitChanceEffect[e] = effect_value; }
						if (negate_itembonus) { itembonuses.HitChanceEffect[e] = effect_value; }
					}
					break;
				}

				case SE_DamageModifier:
				{
					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_spellbonus) { spellbonuses.DamageModifier[e] = effect_value; }
						if (negate_aabonus) { aabonuses.DamageModifier[e] = effect_value; }
						if (negate_itembonus) { itembonuses.DamageModifier[e] = effect_value; }
					}
					break;
				}

				case SE_DamageModifier2:
				{
					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_spellbonus) { spellbonuses.DamageModifier2[e] = effect_value; }
						if (negate_aabonus) { aabonuses.DamageModifier2[e] = effect_value; }
						if (negate_itembonus) { itembonuses.DamageModifier2[e] = effect_value; }
					}
					break;
				}

				case SE_Skill_Base_Damage_Mod:
				{
					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_spellbonus) { spellbonuses.DamageModifier3[e] = effect_value; }
						if (negate_aabonus) { aabonuses.DamageModifier3[e] = effect_value; }
						if (negate_itembonus) { itembonuses.DamageModifier3[e] = effect_value; }
					}
					break;
				}


				case SE_MinDamageModifier:
				{
					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_spellbonus) { spellbonuses.MinDamageModifier[e] = effect_value; }
						if (negate_aabonus) { aabonuses.MinDamageModifier[e] = effect_value; }
						if (negate_itembonus) { itembonuses.MinDamageModifier[e] = effect_value; }
					}
					break;
				}

				case SE_StunResist:
					if (negate_spellbonus) { spellbonuses.StunResist = effect_value; }
					if (negate_aabonus) { aabonuses.StunResist = effect_value; }
					if (negate_itembonus) { itembonuses.StunResist = effect_value; }
					break;

				case SE_ProcChance:
					if (negate_spellbonus) { spellbonuses.ProcChanceSPA = effect_value; }
					if (negate_aabonus) { aabonuses.ProcChanceSPA = effect_value; }
					if (negate_itembonus) { itembonuses.ProcChanceSPA = effect_value; }
					break;

				case SE_ExtraAttackChance:
					if (negate_spellbonus) { spellbonuses.ExtraAttackChance[SBIndex::EXTRA_ATTACK_CHANCE] = effect_value; }
					if (negate_aabonus) { aabonuses.ExtraAttackChance[SBIndex::EXTRA_ATTACK_CHANCE] = effect_value; }
					if (negate_itembonus) { itembonuses.ExtraAttackChance[SBIndex::EXTRA_ATTACK_CHANCE] = effect_value; }
					break;

				case SE_AddExtraAttackPct_1h_Primary:
					if (negate_spellbonus) { spellbonuses.ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_CHANCE] = effect_value; }
					if (negate_aabonus) { aabonuses.ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_CHANCE] = effect_value; }
					if (negate_itembonus) { itembonuses.ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_CHANCE] = effect_value; }
					break;

				case SE_AddExtraAttackPct_1h_Secondary:
					if (negate_spellbonus) { spellbonuses.ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_CHANCE] = effect_value; }
					if (negate_aabonus) { aabonuses.ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_CHANCE] = effect_value; }
					if (negate_itembonus) { itembonuses.ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_CHANCE] = effect_value; }
					break;

				case SE_Double_Melee_Round:
					if (negate_spellbonus) { spellbonuses.DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_CHANCE] = effect_value; }
					if (negate_aabonus) { aabonuses.DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_CHANCE] = effect_value; }
					if (negate_itembonus) { itembonuses.DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_CHANCE] = effect_value; }
					break;

				case SE_PercentXPIncrease:
					if (negate_spellbonus) { spellbonuses.XPRateMod = effect_value; }
					if (negate_aabonus) { aabonuses.XPRateMod = effect_value; }
					if (negate_itembonus) { itembonuses.XPRateMod = effect_value; }
					break;

				case SE_Flurry:
					if (negate_spellbonus) { spellbonuses.FlurryChance = effect_value; }
					if (negate_aabonus) { aabonuses.FlurryChance = effect_value; }
					if (negate_itembonus) { itembonuses.FlurryChance = effect_value; }
					break;

				case SE_Accuracy:
				{
					if (negate_spellbonus) { spellbonuses.Accuracy[EQ::skills::HIGHEST_SKILL + 1] = effect_value; }
					if (negate_itembonus) { itembonuses.Accuracy[EQ::skills::HIGHEST_SKILL + 1] = effect_value; }

					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_aabonus) { aabonuses.Accuracy[e] = effect_value; }
					}
					break;
				}

				case SE_MaxHPChange:
					if (negate_spellbonus) { spellbonuses.MaxHPChange = effect_value; }
					if (negate_aabonus) { aabonuses.MaxHPChange = effect_value; }
					if (negate_itembonus) { itembonuses.MaxHPChange = effect_value; }
					break;

				case SE_EndurancePool:
					if (negate_spellbonus) { spellbonuses.Endurance = effect_value; }
					if (negate_aabonus) { aabonuses.Endurance = effect_value; }
					if (negate_itembonus) { itembonuses.Endurance = effect_value; }
					break;

				case SE_HealRate:
					if (negate_spellbonus) { spellbonuses.HealRate = effect_value; }
					if (negate_aabonus) { aabonuses.HealRate = effect_value; }
					if (negate_itembonus) { itembonuses.HealRate = effect_value; }
					break;

				case SE_SkillDamageTaken:
				{
					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_spellbonus) { spellbonuses.SkillDmgTaken[e] = effect_value; }
						if (negate_aabonus) { aabonuses.SkillDmgTaken[e] = effect_value; }
						if (negate_itembonus) { itembonuses.SkillDmgTaken[e] = effect_value; }

					}
					break;
				}

				case SE_SpellCritChance:
					if (negate_spellbonus) { spellbonuses.CriticalSpellChance = effect_value; }
					if (negate_aabonus) { aabonuses.CriticalSpellChance = effect_value; }
					if (negate_itembonus) { itembonuses.CriticalSpellChance = effect_value; }
					break;

				case SE_CriticalSpellChance:
					if (negate_spellbonus) { spellbonuses.CriticalSpellChance = effect_value; }
					if (negate_spellbonus) { spellbonuses.SpellCritDmgIncrease = effect_value; }
					if (negate_aabonus) { aabonuses.CriticalSpellChance = effect_value; }
					if (negate_aabonus) { aabonuses.SpellCritDmgIncrease = effect_value; }
					if (negate_itembonus) { itembonuses.CriticalSpellChance = effect_value; }
					if (negate_itembonus) { itembonuses.SpellCritDmgIncrease = effect_value; }
					break;

				case SE_SpellCritDmgIncrease:
					if (negate_spellbonus) { spellbonuses.SpellCritDmgIncrease = effect_value; }
					if (negate_aabonus) { aabonuses.SpellCritDmgIncrease = effect_value; }
					if (negate_itembonus) { itembonuses.SpellCritDmgIncrease = effect_value; }
					break;

				case SE_DotCritDmgIncrease:
					if (negate_spellbonus) { spellbonuses.DotCritDmgIncrease = effect_value; }
					if (negate_aabonus) { aabonuses.DotCritDmgIncrease = effect_value; }
					if (negate_itembonus) { itembonuses.DotCritDmgIncrease = effect_value; }
					break;

				case SE_CriticalHealChance:
					if (negate_spellbonus) { spellbonuses.CriticalHealChance = effect_value; }
					if (negate_aabonus) { aabonuses.CriticalHealChance = effect_value; }
					if (negate_itembonus) { itembonuses.CriticalHealChance = effect_value; }
					break;

				case SE_CriticalHealOverTime:
					if (negate_spellbonus) { spellbonuses.CriticalHealOverTime = effect_value; }
					if (negate_aabonus) { aabonuses.CriticalHealOverTime = effect_value; }
					if (negate_itembonus) { itembonuses.CriticalHealOverTime = effect_value; }
					break;

				case SE_MitigateDamageShield:
					if (negate_spellbonus) { spellbonuses.DSMitigationOffHand = effect_value; }
					if (negate_itembonus) { itembonuses.DSMitigationOffHand = effect_value; }
					if (negate_aabonus) { aabonuses.DSMitigationOffHand = effect_value; }
					break;

				case SE_CriticalDoTChance:
					if (negate_spellbonus) { spellbonuses.CriticalDoTChance = effect_value; }
					if (negate_aabonus) { aabonuses.CriticalDoTChance = effect_value; }
					if (negate_itembonus) { itembonuses.CriticalDoTChance = effect_value; }
					break;

				case SE_ProcOnKillShot:
				{
					for (int e = 0; e < MAX_SPELL_TRIGGER * 3; e = 3)
					{
						if (negate_spellbonus) { spellbonuses.SpellOnKill[e] = effect_value; }
						if (negate_spellbonus) { spellbonuses.SpellOnKill[e + 1] = effect_value; }
						if (negate_spellbonus) { spellbonuses.SpellOnKill[e + 2] = effect_value; }

						if (negate_aabonus) { aabonuses.SpellOnKill[e] = effect_value; }
						if (negate_aabonus) { aabonuses.SpellOnKill[e + 1] = effect_value; }
						if (negate_aabonus) { aabonuses.SpellOnKill[e + 2] = effect_value; }

						if (negate_itembonus) { itembonuses.SpellOnKill[e] = effect_value; }
						if (negate_itembonus) { itembonuses.SpellOnKill[e + 1] = effect_value; }
						if (negate_itembonus) { itembonuses.SpellOnKill[e + 2] = effect_value; }
					}
					break;
				}

				/*
				case SE_SpellOnDeath:
				{
					for(int e = 0; e < MAX_SPELL_TRIGGER; e=2)
					{
						if  (negate_spellbonus) { spellbonuses.SpellOnDeath[e] = SPELL_UNKNOWN;
						if  (negate_spellbonus) { spellbonuses.SpellOnDeath[e+1] = effect_value; }
					}
					break;
				}
				*/

				case SE_CriticalDamageMob:
				{
					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_spellbonus) { spellbonuses.CritDmgMod[e] = effect_value; }
						if (negate_aabonus) { aabonuses.CritDmgMod[e] = effect_value; }
						if (negate_itembonus) { itembonuses.CritDmgMod[e] = effect_value; }
					}
					break;
				}

				case SE_Critical_Melee_Damage_Mod_Max:
				{
					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_spellbonus) { spellbonuses.CritDmgModNoStack[e] = effect_value; }
						if (negate_aabonus) { aabonuses.CritDmgModNoStack[e] = effect_value; }
						if (negate_itembonus) { itembonuses.CritDmgModNoStack[e] = effect_value; }
					}
					break;
				}

				case SE_SkillDamageAmount:
				{
					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_spellbonus) { spellbonuses.SkillDamageAmount[e] = effect_value; }
						if (negate_aabonus) { aabonuses.SkillDamageAmount[e] = effect_value; }
						if (negate_itembonus) { itembonuses.SkillDamageAmount[e] = effect_value; }
					}
					break;
				}

				case SE_IncreaseBlockChance:
					if (negate_spellbonus) { spellbonuses.IncreaseBlockChance = effect_value; }
					if (negate_aabonus) { aabonuses.IncreaseBlockChance = effect_value; }
					if (negate_itembonus) { itembonuses.IncreaseBlockChance = effect_value; }
					break;

				case SE_PersistantCasting:
					if (negate_spellbonus) { spellbonuses.PersistantCasting = effect_value; }
					if (negate_itembonus) { itembonuses.PersistantCasting = effect_value; }
					if (negate_aabonus) { aabonuses.PersistantCasting = effect_value; }
					break;

				case SE_ImmuneFleeing:
					if (negate_spellbonus) { spellbonuses.ImmuneToFlee = false; }
					break;

				case SE_DelayDeath:
					if (negate_spellbonus) { spellbonuses.DelayDeath = effect_value; }
					if (negate_aabonus) { aabonuses.DelayDeath = effect_value; }
					if (negate_itembonus) { itembonuses.DelayDeath = effect_value; }
					break;

				case SE_SpellProcChance:
					if (negate_spellbonus) { spellbonuses.SpellProcChance = effect_value; }
					if (negate_itembonus) { itembonuses.SpellProcChance = effect_value; }
					if (negate_aabonus) { aabonuses.SpellProcChance = effect_value; }
					break;

				case SE_CharmBreakChance:
					if (negate_spellbonus) { spellbonuses.CharmBreakChance = effect_value; }
					if (negate_aabonus) { aabonuses.CharmBreakChance = effect_value; }
					if (negate_itembonus) { itembonuses.CharmBreakChance = effect_value; }
					break;

				case SE_BardSongRange:
					if (negate_spellbonus) { spellbonuses.SongRange = effect_value; }
					if (negate_aabonus) { aabonuses.SongRange = effect_value; }
					if (negate_itembonus) { itembonuses.SongRange = effect_value; }
					break;

				case SE_SkillDamageAmount2:
				{
					for (int e = 0; e < EQ::skills::HIGHEST_SKILL + 1; e++)
					{
						if (negate_spellbonus) { spellbonuses.SkillDamageAmount2[e] = effect_value; }
						if (negate_aabonus) { aabonuses.SkillDamageAmount2[e] = effect_value; }
						if (negate_itembonus) { itembonuses.SkillDamageAmount2[e] = effect_value; }
					}
					break;
				}

				case SE_NegateAttacks:
					if (negate_spellbonus) { spellbonuses.NegateAttacks[SBIndex::NEGATE_ATK_EXISTS] = effect_value; }
					if (negate_spellbonus) { spellbonuses.NegateAttacks[SBIndex::NEGATE_ATK_BUFFSLOT] = effect_value; }
					break;

				case SE_MitigateMeleeDamage:
					if (negate_spellbonus) { spellbonuses.MitigateMeleeRune[SBIndex::MITIGATION_RUNE_PERCENT] = effect_value; }
					if (negate_spellbonus) { spellbonuses.MitigateMeleeRune[SBIndex::MITIGATION_RUNE_BUFFSLOT] = -1; }
					break;

				case SE_MeleeThresholdGuard:
					if (negate_spellbonus) { spellbonuses.MeleeThresholdGuard[SBIndex::THRESHOLDGUARD_MITIGATION_PERCENT] = effect_value; }
					if (negate_spellbonus) { spellbonuses.MeleeThresholdGuard[SBIndex::THRESHOLDGUARD_BUFFSLOT] = -1; }
					if (negate_spellbonus) { spellbonuses.MeleeThresholdGuard[SBIndex::THRESHOLDGUARD_BUFFSLOT] = effect_value; }
					break;

				case SE_SpellThresholdGuard:
					if (negate_spellbonus) { spellbonuses.SpellThresholdGuard[SBIndex::THRESHOLDGUARD_MITIGATION_PERCENT] = effect_value; }
					if (negate_spellbonus) { spellbonuses.SpellThresholdGuard[SBIndex::THRESHOLDGUARD_BUFFSLOT] = -1; }
					if (negate_spellbonus) { spellbonuses.SpellThresholdGuard[SBIndex::THRESHOLDGUARD_BUFFSLOT] = effect_value; }
					break;

				case SE_MitigateSpellDamage:
					if (negate_spellbonus) { spellbonuses.MitigateSpellRune[SBIndex::MITIGATION_RUNE_PERCENT] = effect_value; }
					if (negate_spellbonus) { spellbonuses.MitigateSpellRune[SBIndex::MITIGATION_RUNE_BUFFSLOT] = -1; }
					break;

				case SE_MitigateDotDamage:
					if (negate_spellbonus) { spellbonuses.MitigateDotRune[SBIndex::MITIGATION_RUNE_PERCENT] = effect_value; }
					if (negate_spellbonus) { spellbonuses.MitigateDotRune[SBIndex::MITIGATION_RUNE_BUFFSLOT] = -1; }
					break;

				case SE_ManaAbsorbPercentDamage:
					if (negate_spellbonus) { spellbonuses.ManaAbsorbPercentDamage = effect_value; }
					break;

				case SE_Endurance_Absorb_Pct_Damage:
					if (negate_spellbonus) { spellbonuses.EnduranceAbsorbPercentDamage[SBIndex::ENDURANCE_ABSORD_MITIGIATION] = effect_value; }
					if (negate_spellbonus) { spellbonuses.EnduranceAbsorbPercentDamage[SBIndex::ENDURANCE_ABSORD_DRAIN_PER_HP] = effect_value; }
					break;

				case SE_ShieldBlock:
					if (negate_spellbonus) { spellbonuses.ShieldBlock = effect_value; }
					if (negate_aabonus) { aabonuses.ShieldBlock = effect_value; }
					if (negate_itembonus) { itembonuses.ShieldBlock = effect_value; }

				case SE_BlockBehind:
					if (negate_spellbonus) { spellbonuses.BlockBehind = effect_value; }
					if (negate_aabonus) { aabonuses.BlockBehind = effect_value; }
					if (negate_itembonus) { itembonuses.BlockBehind = effect_value; }
					break;

				case SE_Blind:
					if (negate_spellbonus) { spellbonuses.IsBlind = false; }
					break;

				case SE_Fear:
					if (negate_spellbonus) { spellbonuses.IsFeared = false; }
					break;

				case SE_FrontalStunResist:
					if (negate_spellbonus) { spellbonuses.FrontalStunResist = effect_value; }
					if (negate_aabonus) { aabonuses.FrontalStunResist = effect_value; }
					if (negate_itembonus) { itembonuses.FrontalStunResist = effect_value; }
					break;

				case SE_ImprovedBindWound:
					if (negate_aabonus) { aabonuses.BindWound = effect_value; }
					if (negate_itembonus) { itembonuses.BindWound = effect_value; }
					if (negate_spellbonus) { spellbonuses.BindWound = effect_value; }
					break;

				case SE_MaxBindWound:
					if (negate_spellbonus) { spellbonuses.MaxBindWound = effect_value; }
					if (negate_aabonus) { aabonuses.MaxBindWound = effect_value; }
					if (negate_itembonus) { itembonuses.MaxBindWound = effect_value; }
					break;

				case SE_BaseMovementSpeed:
					if (negate_spellbonus) { spellbonuses.BaseMovementSpeed = effect_value; }
					if (negate_aabonus) { aabonuses.BaseMovementSpeed = effect_value; }
					if (negate_itembonus) { itembonuses.BaseMovementSpeed = effect_value; }
					break;

				case SE_IncreaseRunSpeedCap:
					if (negate_itembonus) { itembonuses.IncreaseRunSpeedCap = effect_value; }
					if (negate_aabonus) { aabonuses.IncreaseRunSpeedCap = effect_value; }
					if (negate_spellbonus) { spellbonuses.IncreaseRunSpeedCap = effect_value; }
					break;

				case SE_DoubleSpecialAttack:
					if (negate_spellbonus) { spellbonuses.DoubleSpecialAttack = effect_value; }
					if (negate_aabonus) { aabonuses.DoubleSpecialAttack = effect_value; }
					if (negate_itembonus) { itembonuses.DoubleSpecialAttack = effect_value; }
					break;

				case SE_TripleBackstab:
					if (negate_spellbonus) { spellbonuses.TripleBackstab = effect_value; }
					if (negate_aabonus) { aabonuses.TripleBackstab = effect_value; }
					if (negate_itembonus) { itembonuses.TripleBackstab = effect_value; }
					break;

				case SE_FrontalBackstabMinDmg:
					if (negate_spellbonus) { spellbonuses.FrontalBackstabMinDmg = false; }
					break;

				case SE_FrontalBackstabChance:
					if (negate_spellbonus) { spellbonuses.FrontalBackstabChance = effect_value; }
					if (negate_aabonus) { aabonuses.FrontalBackstabChance = effect_value; }
					if (negate_itembonus) { itembonuses.FrontalBackstabChance = effect_value; }
					break;

				case SE_Double_Backstab_Front:
					if (negate_spellbonus) { spellbonuses.Double_Backstab_Front = effect_value; }
					if (negate_aabonus) { aabonuses.Double_Backstab_Front = effect_value; }
					if (negate_itembonus) { itembonuses.Double_Backstab_Front = effect_value; }
					break;

				case SE_ConsumeProjectile:
					if (negate_spellbonus) { spellbonuses.ConsumeProjectile = effect_value; }
					if (negate_aabonus) { aabonuses.ConsumeProjectile = effect_value; }
					if (negate_itembonus) { itembonuses.ConsumeProjectile = effect_value; }
					break;

				case SE_ForageAdditionalItems:
					if (negate_spellbonus) { spellbonuses.ForageAdditionalItems = effect_value; }
					if (negate_aabonus) { aabonuses.ForageAdditionalItems = effect_value; }
					if (negate_itembonus) { itembonuses.ForageAdditionalItems = effect_value; }
					break;

				case SE_Salvage:
					if (negate_spellbonus) { spellbonuses.SalvageChance = effect_value; }
					if (negate_aabonus) { aabonuses.SalvageChance = effect_value; }
					if (negate_itembonus) { itembonuses.SalvageChance = effect_value; }
					break;

				case SE_ArcheryDamageModifier:
					if (negate_spellbonus) { spellbonuses.ArcheryDamageModifier = effect_value; }
					if (negate_aabonus) { aabonuses.ArcheryDamageModifier = effect_value; }
					if (negate_itembonus) { itembonuses.ArcheryDamageModifier = effect_value; }
					break;

				case SE_SecondaryDmgInc:
					if (negate_spellbonus) { spellbonuses.SecondaryDmgInc = false; }
					if (negate_aabonus) { aabonuses.SecondaryDmgInc = false; }
					if (negate_itembonus) { itembonuses.SecondaryDmgInc = false; }
					break;

				case SE_StrikeThrough:
					if (negate_spellbonus) { spellbonuses.StrikeThrough = effect_value; }
					if (negate_aabonus) { aabonuses.StrikeThrough = effect_value; }
					if (negate_itembonus) { itembonuses.StrikeThrough = effect_value; }
					break;

				case SE_StrikeThrough2:
					if (negate_spellbonus) { spellbonuses.StrikeThrough = effect_value; }
					if (negate_aabonus) { aabonuses.StrikeThrough = effect_value; }
					if (negate_itembonus) { itembonuses.StrikeThrough = effect_value; }
					break;

				case SE_GiveDoubleAttack:
					if (negate_spellbonus) { spellbonuses.GiveDoubleAttack = effect_value; }
					if (negate_aabonus) { aabonuses.GiveDoubleAttack = effect_value; }
					if (negate_itembonus) { itembonuses.GiveDoubleAttack = effect_value; }
					break;

				case SE_PetCriticalHit:
					if (negate_spellbonus) { spellbonuses.PetCriticalHit = effect_value; }
					if (negate_aabonus) { aabonuses.PetCriticalHit = effect_value; }
					if (negate_itembonus) { itembonuses.PetCriticalHit = effect_value; }
					break;

				case SE_CombatStability:
					if (negate_spellbonus) { spellbonuses.CombatStability = effect_value; }
					if (negate_aabonus) { aabonuses.CombatStability = effect_value; }
					if (negate_itembonus) { itembonuses.CombatStability = effect_value; }
					break;

				case SE_PetAvoidance:
					if (negate_spellbonus) { spellbonuses.PetAvoidance = effect_value; }
					if (negate_aabonus) { aabonuses.PetAvoidance = effect_value; }
					if (negate_itembonus) { itembonuses.PetAvoidance = effect_value; }
					break;

				case SE_Ambidexterity:
					if (negate_spellbonus) { spellbonuses.Ambidexterity = effect_value; }
					if (negate_aabonus) { aabonuses.Ambidexterity = effect_value; }
					if (negate_itembonus) { itembonuses.Ambidexterity = effect_value; }
					break;

				case SE_PetMaxHP:
					if (negate_spellbonus) { spellbonuses.PetMaxHP = effect_value; }
					if (negate_aabonus) { aabonuses.PetMaxHP = effect_value; }
					if (negate_itembonus) { itembonuses.PetMaxHP = effect_value; }
					break;

				case SE_PetFlurry:
					if (negate_spellbonus) { spellbonuses.PetFlurry = effect_value; }
					if (negate_aabonus) { aabonuses.PetFlurry = effect_value; }
					if (negate_itembonus) { itembonuses.PetFlurry = effect_value; }
					break;

				case SE_GivePetGroupTarget:
					if (negate_spellbonus) { spellbonuses.GivePetGroupTarget = false; }
					if (negate_aabonus) { aabonuses.GivePetGroupTarget = false; }
					if (negate_itembonus) { itembonuses.GivePetGroupTarget = false; }
					break;

				case SE_PetMeleeMitigation:
					if (negate_spellbonus) { spellbonuses.PetMeleeMitigation = effect_value; }
					if (negate_itembonus) { itembonuses.PetMeleeMitigation = effect_value; }
					if (negate_aabonus) { aabonuses.PetMeleeMitigation = effect_value; }
					break;

				case SE_RootBreakChance:
					if (negate_spellbonus) { spellbonuses.RootBreakChance = effect_value; }
					if (negate_aabonus) { aabonuses.RootBreakChance = effect_value; }
					if (negate_itembonus) { itembonuses.RootBreakChance = effect_value; }
					break;

				case SE_ChannelChanceItems:
					if (negate_spellbonus) { spellbonuses.ChannelChanceItems = effect_value; }
					if (negate_aabonus) { aabonuses.ChannelChanceItems = effect_value; }
					if (negate_itembonus) { itembonuses.ChannelChanceItems = effect_value; }
					break;

				case SE_ChannelChanceSpells:
					if (negate_spellbonus) { spellbonuses.ChannelChanceSpells = effect_value; }
					if (negate_aabonus) { aabonuses.ChannelChanceSpells = effect_value; }
					if (negate_itembonus) { itembonuses.ChannelChanceSpells = effect_value; }
					break;

				case SE_UnfailingDivinity:
					if (negate_spellbonus) { spellbonuses.UnfailingDivinity = effect_value; }
					if (negate_aabonus) { aabonuses.UnfailingDivinity = effect_value; }
					if (negate_itembonus) { itembonuses.UnfailingDivinity = effect_value; }
					break;

				case SE_ItemHPRegenCapIncrease:
					if (negate_spellbonus) { spellbonuses.ItemHPRegenCap = effect_value; }
					if (negate_aabonus) { aabonuses.ItemHPRegenCap = effect_value; }
					if (negate_itembonus) { itembonuses.ItemHPRegenCap = effect_value; }
					break;

				case SE_Worn_Endurance_Regen_Cap:
					if (negate_spellbonus) { spellbonuses.ItemEnduranceRegenCap = effect_value; }
					if (negate_aabonus) { aabonuses.ItemEnduranceRegenCap = effect_value; }
					if (negate_itembonus) { itembonuses.ItemEnduranceRegenCap = effect_value; }
					break;

				case SE_OffhandRiposteFail:
					if (negate_spellbonus) { spellbonuses.OffhandRiposteFail = effect_value; }
					if (negate_aabonus) { aabonuses.OffhandRiposteFail = effect_value; }
					if (negate_itembonus) { itembonuses.OffhandRiposteFail = effect_value; }
					break;

				case SE_ItemAttackCapIncrease:
					if (negate_aabonus) { aabonuses.ItemATKCap = effect_value; }
					if (negate_itembonus) { itembonuses.ItemATKCap = effect_value; }
					if (negate_spellbonus) { spellbonuses.ItemATKCap = effect_value; }
					break;

				case SE_SpellEffectResistChance:
				{
					for (int e = 0; e < MAX_RESISTABLE_EFFECTS * 2; e += 2)
					{
						if (negate_spellbonus) { spellbonuses.SEResist[e] = effect_value; }
						if (negate_spellbonus) { spellbonuses.SEResist[e + 1] = effect_value; }
					}
					break;
				}

				case SE_MasteryofPast:
					if (negate_spellbonus) { spellbonuses.MasteryofPast = effect_value; }
					if (negate_aabonus) { aabonuses.MasteryofPast = effect_value; }
					if (negate_itembonus) { itembonuses.MasteryofPast = effect_value; }
					break;

				case SE_DoubleRiposte:
					if (negate_spellbonus) { spellbonuses.DoubleRiposte = effect_value; }
					if (negate_itembonus) { itembonuses.DoubleRiposte = effect_value; }
					if (negate_aabonus) { aabonuses.DoubleRiposte = effect_value; }
					break;

				case SE_GiveDoubleRiposte:
					if (negate_spellbonus) { spellbonuses.GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_CHANCE] = effect_value; }
					if (negate_itembonus) { itembonuses.GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_CHANCE] = effect_value; }
					if (negate_aabonus) { aabonuses.GiveDoubleRiposte[SBIndex::DOUBLE_RIPOSTE_CHANCE] = effect_value; }
					break;

				case SE_SlayUndead:
					if (negate_spellbonus) { spellbonuses.SlayUndead[SBIndex::SLAYUNDEAD_RATE_MOD] = effect_value; }
					if (negate_spellbonus) { spellbonuses.SlayUndead[SBIndex::SLAYUNDEAD_DMG_MOD] = effect_value; }
					if (negate_itembonus) { itembonuses.SlayUndead[SBIndex::SLAYUNDEAD_RATE_MOD] = effect_value; }
					if (negate_itembonus) { itembonuses.SlayUndead[SBIndex::SLAYUNDEAD_DMG_MOD] = effect_value; }
					if (negate_aabonus) { aabonuses.SlayUndead[SBIndex::SLAYUNDEAD_RATE_MOD] = effect_value; }
					if (negate_aabonus) { aabonuses.SlayUndead[SBIndex::SLAYUNDEAD_DMG_MOD] = effect_value; }
					break;

				case SE_DoubleRangedAttack:
					if (negate_spellbonus) { spellbonuses.DoubleRangedAttack = effect_value; }
					if (negate_aabonus) { aabonuses.DoubleRangedAttack = effect_value; }
					if (negate_itembonus) { itembonuses.DoubleRangedAttack = effect_value; }
					break;

				case SE_ShieldEquipDmgMod:
					if (negate_spellbonus) { spellbonuses.ShieldEquipDmgMod = effect_value; }
					if (negate_aabonus) { aabonuses.ShieldEquipDmgMod = effect_value; }
					if (negate_itembonus) { itembonuses.ShieldEquipDmgMod = effect_value; }
					break;

				case SE_TriggerMeleeThreshold:
					if (negate_spellbonus) { spellbonuses.TriggerMeleeThreshold = false; }
					break;

				case SE_TriggerSpellThreshold:
					if (negate_spellbonus) { spellbonuses.TriggerSpellThreshold = false; }
					break;

				case SE_DivineAura:
					if (negate_spellbonus) { spellbonuses.DivineAura = false; }
					break;

				case SE_StunBashChance:
					if (negate_spellbonus) { spellbonuses.StunBashChance = effect_value; }
					if (negate_itembonus) { itembonuses.StunBashChance = effect_value; }
					if (negate_aabonus) { aabonuses.StunBashChance = effect_value; }
					break;

				case SE_IncreaseChanceMemwipe:
					if (negate_spellbonus) { spellbonuses.IncreaseChanceMemwipe = effect_value; }
					if (negate_itembonus) { itembonuses.IncreaseChanceMemwipe = effect_value; }
					if (negate_aabonus) { aabonuses.IncreaseChanceMemwipe = effect_value; }
					break;

				case SE_CriticalMend:
					if (negate_spellbonus) { spellbonuses.CriticalMend = effect_value; }
					if (negate_itembonus) { itembonuses.CriticalMend = effect_value; }
					if (negate_aabonus) { aabonuses.CriticalMend = effect_value; }
					break;

				case SE_DistanceRemoval:
					if (negate_spellbonus) { spellbonuses.DistanceRemoval = false; }
					break;

				case SE_ImprovedTaunt:
					if (negate_spellbonus) { spellbonuses.ImprovedTaunt[SBIndex::IMPROVED_TAUNT_MAX_LVL] = effect_value; }
					if (negate_spellbonus) { spellbonuses.ImprovedTaunt[SBIndex::IMPROVED_TAUNT_AGGRO_MOD] = effect_value; }
					if (negate_spellbonus) { spellbonuses.ImprovedTaunt[SBIndex::IMPROVED_TAUNT_BUFFSLOT] = -1; }
					break;

				case SE_FrenziedDevastation:
					if (negate_spellbonus) { spellbonuses.FrenziedDevastation = effect_value; }
					if (negate_aabonus) { aabonuses.FrenziedDevastation = effect_value; }
					if (negate_itembonus) { itembonuses.FrenziedDevastation = effect_value; }
					break;

				case SE_Root:
					if (negate_spellbonus) { spellbonuses.Root[SBIndex::ROOT_EXISTS] = effect_value; }
					if (negate_spellbonus) { spellbonuses.Root[SBIndex::ROOT_BUFFSLOT] = -1; }
					break;

				case SE_Rune:
					if (negate_spellbonus) { spellbonuses.MeleeRune[SBIndex::RUNE_AMOUNT] = effect_value; }
					if (negate_spellbonus) { spellbonuses.MeleeRune[SBIndex::RUNE_BUFFSLOT] = -1; }
					break;

				case SE_AbsorbMagicAtt:
					if (negate_spellbonus) { spellbonuses.AbsorbMagicAtt[SBIndex::RUNE_AMOUNT] = effect_value; }
					if (negate_spellbonus) { spellbonuses.AbsorbMagicAtt[SBIndex::RUNE_BUFFSLOT] = -1; }
					break;

				case SE_Berserk:
					if (negate_spellbonus) { spellbonuses.BerserkSPA = false; }
					if (negate_aabonus) { aabonuses.BerserkSPA = false; }
					if (negate_itembonus) { itembonuses.BerserkSPA = false; }
					break;

				case SE_Vampirism:
					if (negate_spellbonus) { spellbonuses.Vampirism = effect_value; }
					if (negate_aabonus) { aabonuses.Vampirism = effect_value; }
					if (negate_itembonus) { itembonuses.Vampirism = effect_value; }
					break;

				case SE_Metabolism:
					if (negate_spellbonus) { spellbonuses.Metabolism = effect_value; }
					if (negate_aabonus) { aabonuses.Metabolism = effect_value; }
					if (negate_itembonus) { itembonuses.Metabolism = effect_value; }
					break;

				case SE_ImprovedReclaimEnergy:
					if (negate_spellbonus) { spellbonuses.ImprovedReclaimEnergy = effect_value; }
					if (negate_aabonus) { aabonuses.ImprovedReclaimEnergy = effect_value; }
					if (negate_itembonus) { itembonuses.ImprovedReclaimEnergy = effect_value; }
					break;

				case SE_HeadShot:
					if (negate_spellbonus) { spellbonuses.HeadShot[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value; }
					if (negate_aabonus) { aabonuses.HeadShot[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value; }
					if (negate_itembonus) { itembonuses.HeadShot[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value; }
					if (negate_spellbonus) { spellbonuses.HeadShot[SBIndex::FINISHING_EFFECT_DMG] = effect_value; }
					if (negate_aabonus) { aabonuses.HeadShot[SBIndex::FINISHING_EFFECT_DMG] = effect_value; }
					if (negate_itembonus) { itembonuses.HeadShot[SBIndex::FINISHING_EFFECT_DMG] = effect_value; }
					break;

				case SE_HeadShotLevel:
					if (negate_spellbonus) { spellbonuses.HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX] = effect_value; }
					if (negate_aabonus) { aabonuses.HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX] = effect_value; }
					if (negate_itembonus) { itembonuses.HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX] = effect_value; }
					if (negate_spellbonus) { spellbonuses.HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS] = effect_value; }
					if (negate_aabonus) { aabonuses.HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS] = effect_value; }
					if (negate_itembonus) { itembonuses.HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS] = effect_value; }
					break;

				case SE_Assassinate:
					if (negate_spellbonus) { spellbonuses.Assassinate[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value; }
					if (negate_aabonus) { aabonuses.Assassinate[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value; }
					if (negate_itembonus) { itembonuses.Assassinate[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value; }
					if (negate_spellbonus) { spellbonuses.Assassinate[SBIndex::FINISHING_EFFECT_DMG] = effect_value; }
					if (negate_aabonus) { aabonuses.Assassinate[SBIndex::FINISHING_EFFECT_DMG] = effect_value; }
					if (negate_itembonus) { itembonuses.Assassinate[SBIndex::FINISHING_EFFECT_DMG] = effect_value; }
					break;

				case SE_AssassinateLevel:
					if (negate_spellbonus) { spellbonuses.AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX] = effect_value; }
					if (negate_aabonus) { aabonuses.AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX] = effect_value; }
					if (negate_itembonus) { itembonuses.AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX] = effect_value; }
					if (negate_spellbonus) { spellbonuses.AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS] = effect_value; }
					if (negate_aabonus) { aabonuses.AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS] = effect_value; }
					if (negate_itembonus) { itembonuses.AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS] = effect_value; }
					break;

				case SE_FinishingBlow:
					if (negate_spellbonus) { spellbonuses.FinishingBlow[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value; }
					if (negate_aabonus) { aabonuses.FinishingBlow[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value; }
					if (negate_itembonus) { itembonuses.FinishingBlow[SBIndex::FINISHING_EFFECT_PROC_CHANCE] = effect_value; }
					if (negate_spellbonus) { spellbonuses.FinishingBlow[SBIndex::FINISHING_EFFECT_DMG] = effect_value; }
					if (negate_aabonus) { aabonuses.FinishingBlow[SBIndex::FINISHING_EFFECT_DMG] = effect_value; }
					if (negate_itembonus) { itembonuses.FinishingBlow[SBIndex::FINISHING_EFFECT_DMG] = effect_value; }
					break;

				case SE_FinishingBlowLvl:
					if (negate_spellbonus) { spellbonuses.FinishingBlowLvl[SBIndex::FINISHING_EFFECT_LEVEL_MAX] = effect_value; }
					if (negate_aabonus) { aabonuses.FinishingBlowLvl[SBIndex::FINISHING_EFFECT_LEVEL_MAX] = effect_value; }
					if (negate_itembonus) { itembonuses.FinishingBlowLvl[SBIndex::FINISHING_EFFECT_LEVEL_MAX] = effect_value; }
					if (negate_spellbonus) { spellbonuses.FinishingBlowLvl[SBIndex::FINISHING_BLOW_LEVEL_HP_RATIO] = effect_value; }
					if (negate_aabonus) { aabonuses.FinishingBlowLvl[SBIndex::FINISHING_BLOW_LEVEL_HP_RATIO] = effect_value; }
					if (negate_itembonus) { itembonuses.FinishingBlowLvl[SBIndex::FINISHING_BLOW_LEVEL_HP_RATIO] = effect_value; }
					break;

				case SE_Sanctuary:
					if (negate_spellbonus) { spellbonuses.Sanctuary = false; }
					break;

				case SE_FactionModPct:
					if (negate_spellbonus) { spellbonuses.FactionModPct = effect_value; }
					if (negate_itembonus) { itembonuses.FactionModPct = effect_value; }
					if (negate_aabonus) { aabonuses.FactionModPct = effect_value; }
					break;

				case SE_IllusionPersistence:
					if (negate_spellbonus) { spellbonuses.IllusionPersistence = effect_value; }
					if (negate_itembonus) { itembonuses.IllusionPersistence = effect_value; }
					if (negate_aabonus) { aabonuses.IllusionPersistence = effect_value; }
					break;

				case SE_Attack_Accuracy_Max_Percent:
					if (negate_spellbonus) { spellbonuses.Attack_Accuracy_Max_Percent = effect_value; }
					if (negate_itembonus) { itembonuses.Attack_Accuracy_Max_Percent = effect_value; }
					if (negate_aabonus) { aabonuses.Attack_Accuracy_Max_Percent = effect_value; }
					break;


				case SE_AC_Mitigation_Max_Percent:
					if (negate_spellbonus) { spellbonuses.AC_Mitigation_Max_Percent = effect_value; }
					if (negate_itembonus) { itembonuses.AC_Mitigation_Max_Percent = effect_value; }
					if (negate_aabonus) { aabonuses.AC_Mitigation_Max_Percent = effect_value; }
					break;

				case SE_AC_Avoidance_Max_Percent:
					if (negate_spellbonus) { spellbonuses.AC_Avoidance_Max_Percent = effect_value; }
					if (negate_itembonus) { itembonuses.AC_Avoidance_Max_Percent = effect_value; }
					if (negate_aabonus) { aabonuses.AC_Avoidance_Max_Percent = effect_value; }
					break;

				case SE_Melee_Damage_Position_Mod:
					if (negate_spellbonus) { spellbonuses.Melee_Damage_Position_Mod[SBIndex::POSITION_BACK] = effect_value; }
					if (negate_aabonus) { aabonuses.Melee_Damage_Position_Mod[SBIndex::POSITION_BACK] = effect_value; }
					if (negate_itembonus) { itembonuses.Melee_Damage_Position_Mod[SBIndex::POSITION_BACK] = effect_value; }
					if (negate_spellbonus) { spellbonuses.Melee_Damage_Position_Mod[SBIndex::POSITION_FRONT] = effect_value; }
					if (negate_aabonus) { aabonuses.Melee_Damage_Position_Mod[SBIndex::POSITION_FRONT] = effect_value; }
					if (negate_itembonus) { itembonuses.Melee_Damage_Position_Mod[SBIndex::POSITION_FRONT] = effect_value; }
					break;

				case SE_Damage_Taken_Position_Mod:
					if (negate_spellbonus) { spellbonuses.Damage_Taken_Position_Mod[SBIndex::POSITION_BACK] = effect_value; }
					if (negate_aabonus) { aabonuses.Damage_Taken_Position_Mod[SBIndex::POSITION_BACK] = effect_value; }
					if (negate_itembonus) { itembonuses.Damage_Taken_Position_Mod[SBIndex::POSITION_BACK] = effect_value; }
					if (negate_spellbonus) { spellbonuses.Damage_Taken_Position_Mod[SBIndex::POSITION_FRONT] = effect_value; }
					if (negate_aabonus) { aabonuses.Damage_Taken_Position_Mod[SBIndex::POSITION_FRONT] = effect_value; }
					if (negate_itembonus) { itembonuses.Damage_Taken_Position_Mod[SBIndex::POSITION_FRONT] = effect_value; }
					break;

				case SE_Melee_Damage_Position_Amt:
					if (negate_spellbonus) { spellbonuses.Melee_Damage_Position_Amt[SBIndex::POSITION_BACK] = effect_value; }
					if (negate_aabonus) { aabonuses.Melee_Damage_Position_Amt[SBIndex::POSITION_BACK] = effect_value; }
					if (negate_itembonus) { itembonuses.Melee_Damage_Position_Amt[SBIndex::POSITION_BACK] = effect_value; }
					if (negate_spellbonus) { spellbonuses.Melee_Damage_Position_Amt[SBIndex::POSITION_FRONT] = effect_value; }
					if (negate_aabonus) { aabonuses.Melee_Damage_Position_Amt[SBIndex::POSITION_FRONT] = effect_value; }
					if (negate_itembonus) { itembonuses.Melee_Damage_Position_Amt[SBIndex::POSITION_FRONT] = effect_value; }
					break;

				case SE_Damage_Taken_Position_Amt:
					if (negate_spellbonus) { spellbonuses.Damage_Taken_Position_Amt[SBIndex::POSITION_BACK] = effect_value; }
					if (negate_aabonus) { aabonuses.Damage_Taken_Position_Amt[SBIndex::POSITION_BACK] = effect_value; }
					if (negate_itembonus) { itembonuses.Damage_Taken_Position_Amt[SBIndex::POSITION_BACK] = effect_value; }
					if (negate_spellbonus) { spellbonuses.Damage_Taken_Position_Amt[SBIndex::POSITION_FRONT] = effect_value; }
					if (negate_aabonus) { aabonuses.Damage_Taken_Position_Amt[SBIndex::POSITION_FRONT] = effect_value; }
					if (negate_itembonus) { itembonuses.Damage_Taken_Position_Amt[SBIndex::POSITION_FRONT] = effect_value; }
					break;


				case SE_DS_Mitigation_Amount:
					if (negate_spellbonus) { spellbonuses.DS_Mitigation_Amount = effect_value; }
					if (negate_itembonus) { itembonuses.DS_Mitigation_Amount = effect_value; }
					if (negate_aabonus) { aabonuses.DS_Mitigation_Amount = effect_value; }
					break;

				case SE_DS_Mitigation_Percentage:
					if (negate_spellbonus) { spellbonuses.DS_Mitigation_Percentage = effect_value; }
					if (negate_itembonus) { itembonuses.DS_Mitigation_Percentage = effect_value; }
					if (negate_aabonus) { aabonuses.DS_Mitigation_Percentage = effect_value; }
					break;

				case SE_Pet_Crit_Melee_Damage_Pct_Owner:
					if (negate_spellbonus) { spellbonuses.Pet_Crit_Melee_Damage_Pct_Owner = effect_value; }
					if (negate_itembonus) { itembonuses.Pet_Crit_Melee_Damage_Pct_Owner = effect_value; }
					if (negate_aabonus) { aabonuses.Pet_Crit_Melee_Damage_Pct_Owner = effect_value; }
					break;

				case SE_Pet_Add_Atk:
					if (negate_spellbonus) { spellbonuses.Pet_Add_Atk = effect_value; }
					if (negate_itembonus) { itembonuses.Pet_Add_Atk = effect_value; }
					if (negate_aabonus) { aabonuses.Pet_Add_Atk = effect_value; }
					break;

				case SE_PC_Pet_Rampage:
					if (negate_spellbonus) { spellbonuses.PC_Pet_Rampage[SBIndex::PET_RAMPAGE_CHANCE] = effect_value; }
					if (negate_itembonus) { itembonuses.PC_Pet_Rampage[SBIndex::PET_RAMPAGE_CHANCE] = effect_value; }
					if (negate_aabonus) { aabonuses.PC_Pet_Rampage[SBIndex::PET_RAMPAGE_CHANCE] = effect_value; }
					if (negate_spellbonus) { spellbonuses.PC_Pet_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] = effect_value; }
					if (negate_itembonus) { itembonuses.PC_Pet_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] = effect_value; }
					if (negate_aabonus) { aabonuses.PC_Pet_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] = effect_value; }
					break;

				case SE_PC_Pet_AE_Rampage:
					if (negate_spellbonus) { spellbonuses.PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_CHANCE] = effect_value; }
					if (negate_itembonus) { itembonuses.PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_CHANCE] = effect_value; }
					if (negate_aabonus) { aabonuses.PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_CHANCE] = effect_value; }
					if (negate_spellbonus) { spellbonuses.PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] = effect_value; }
					if (negate_itembonus) { itembonuses.PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] = effect_value; }
					if (negate_aabonus) { aabonuses.PC_Pet_AE_Rampage[SBIndex::PET_RAMPAGE_DMG_MOD] = effect_value; }
					break;


				case SE_SkillProcSuccess: {
					for (int e = 0; e < MAX_SKILL_PROCS; e++)
					{
						if (negate_spellbonus) { spellbonuses.SkillProcSuccess[e] = effect_value; }
						if (negate_itembonus) { itembonuses.SkillProcSuccess[e] = effect_value; }
						if (negate_aabonus) { aabonuses.SkillProcSuccess[e] = effect_value; }
					}
				}

				case SE_SkillProcAttempt: {
					for (int e = 0; e < MAX_SKILL_PROCS; e++)
					{
						if (negate_spellbonus) { spellbonuses.SkillProc[e] = effect_value; }
						if (negate_itembonus) { itembonuses.SkillProc[e] = effect_value; }
						if (negate_aabonus) { aabonuses.SkillProc[e] = effect_value; }
					}
				}
			}
		}
	}
}
