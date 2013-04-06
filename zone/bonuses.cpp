/*  EQEMu:  Everquest Server Emulator
Copyright (C) 2001-2004  EQEMu Development Team (http://eqemu.org)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	  You should have received a copy of the GNU General Public License
	  along with this program; if not, write to the Free Software
	  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "../common/debug.h"
#include "../common/spdat.h"
#include "masterentity.h"
#include "../common/packet_dump.h"
#include "../common/moremath.h"
#include "../common/Item.h"
#include "worldserver.h"
#include "../common/skills.h"
#include "../common/bodytypes.h"
#include "../common/classes.h"
#include "../common/rulesys.h"
#include "QuestParserCollection.h"
#include <math.h>
#include <assert.h>
#ifndef WIN32
#include <stdlib.h>
#include "../common/unix.h"
#endif

#include "StringIDs.h"

void Mob::CalcBonuses()
{	
	CalcSpellBonuses(&spellbonuses);
	CalcMaxHP();
	CalcMaxMana();
	SetAttackTimer();
	
	rooted = FindType(SE_Root);
}

void NPC::CalcBonuses()
{
	Mob::CalcBonuses();
	memset(&aabonuses, 0, sizeof(StatBonuses));

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
	_ZP(Client_CalcBonuses);
	memset(&itembonuses, 0, sizeof(StatBonuses));
	CalcItemBonuses(&itembonuses);
	CalcEdibleBonuses(&itembonuses);
	
	CalcSpellBonuses(&spellbonuses);

	_log(AA__BONUSES, "Calculating AA Bonuses for %s.", this->GetCleanName());
	CalcAABonuses(&aabonuses);	//we're not quite ready for this
	_log(AA__BONUSES, "Finished calculating AA Bonuses for %s.", this->GetCleanName());
	
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
	
	rooted = FindType(SE_Root);

	XPRate = 100 + spellbonuses.XPRateMod;
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

	unsigned int i;
	//should not include 21 (SLOT_AMMO)
	for (i=0; i<21; i++) {
		const ItemInst* inst = m_inv[i];
		if(inst == 0)
			continue;
		AddItemBonuses(inst, newbon);
	}
	
	//Power Source Slot
	if (GetClientVersion() >= EQClientSoF)
	{
		const ItemInst* inst = m_inv[9999];
		if(inst)
			AddItemBonuses(inst, newbon);
	}

	//tribute items
	for (i = 0; i < MAX_PLAYER_TRIBUTES; i++) {
		const ItemInst* inst = m_inv[TRIBUTE_SLOT_START + i];
		if(inst == 0)
			continue;
		AddItemBonuses(inst, newbon, false, true);
	}
	// Caps
	if(newbon->HPRegen > CalcHPRegenCap())
		newbon->HPRegen = CalcHPRegenCap();

	if(newbon->ManaRegen > CalcManaRegenCap())
		newbon->ManaRegen = CalcManaRegenCap();
	
	if(newbon->EnduranceRegen > CalcEnduranceRegenCap())
		newbon->EnduranceRegen = CalcEnduranceRegenCap();
			
	SetAttackTimer();
}
		
void Client::AddItemBonuses(const ItemInst *inst, StatBonuses* newbon, bool isAug, bool isTribute) {
	if(!inst || !inst->IsType(ItemClassCommon))
	{
		return;
	}

	if(inst->GetAugmentType()==0 && isAug == true)
	{
		return;
	}

	const Item_Struct *item = inst->GetItem();

	if(!isTribute && !inst->IsEquipable(GetBaseRace(),GetClass()))
	{
		if(item->ItemType != ItemTypeFood && item->ItemType != ItemTypeDrink)
			return;
	}

	if(GetLevel() < item->ReqLevel)
	{
		return;
	}

	if(GetLevel() >= item->RecLevel)
	{
		newbon->AC += item->AC;
		newbon->HP += item->HP;
		newbon->Mana += item->Mana;
		newbon->Endurance += item->Endur;
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
	else
	{
		int lvl = GetLevel();
		int reclvl = item->RecLevel;

		newbon->AC += CalcRecommendedLevelBonus( lvl, reclvl, item->AC );
		newbon->HP += CalcRecommendedLevelBonus( lvl, reclvl, item->HP );
		newbon->Mana += CalcRecommendedLevelBonus( lvl, reclvl, item->Mana );
		newbon->Endurance += CalcRecommendedLevelBonus( lvl, reclvl, item->Endur );
		newbon->STR += CalcRecommendedLevelBonus( lvl, reclvl, (item->AStr + item->HeroicStr) );
		newbon->STA += CalcRecommendedLevelBonus( lvl, reclvl, (item->ASta + item->HeroicSta) );
		newbon->DEX += CalcRecommendedLevelBonus( lvl, reclvl, (item->ADex + item->HeroicDex) );
		newbon->AGI += CalcRecommendedLevelBonus( lvl, reclvl, (item->AAgi + item->HeroicAgi) );
		newbon->INT += CalcRecommendedLevelBonus( lvl, reclvl, (item->AInt + item->HeroicInt) );
		newbon->WIS += CalcRecommendedLevelBonus( lvl, reclvl, (item->AWis + item->HeroicWis) );
		newbon->CHA += CalcRecommendedLevelBonus( lvl, reclvl, (item->ACha + item->HeroicCha) );

		newbon->MR += CalcRecommendedLevelBonus( lvl, reclvl, (item->MR + item->HeroicMR) );
		newbon->FR += CalcRecommendedLevelBonus( lvl, reclvl, (item->FR + item->HeroicFR) );
		newbon->CR += CalcRecommendedLevelBonus( lvl, reclvl, (item->CR + item->HeroicCR) );
		newbon->PR += CalcRecommendedLevelBonus( lvl, reclvl, (item->PR + item->HeroicPR) );
		newbon->DR += CalcRecommendedLevelBonus( lvl, reclvl, (item->DR + item->HeroicDR) );
		newbon->Corrup += CalcRecommendedLevelBonus( lvl, reclvl, (item->SVCorruption + item->HeroicSVCorrup) );

		newbon->STRCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicStr );
		newbon->STACapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicSta );
		newbon->DEXCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicDex );
		newbon->AGICapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicAgi );
		newbon->INTCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicInt );
		newbon->WISCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicWis );
		newbon->CHACapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicCha );
		newbon->MRCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicMR );
		newbon->CRCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicFR );
		newbon->FRCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicCR );
		newbon->PRCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicPR );
		newbon->DRCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicDR );
		newbon->CorrupCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicSVCorrup );

		newbon->HeroicSTR += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicStr );
		newbon->HeroicSTA += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicSta );
		newbon->HeroicDEX += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicDex );
		newbon->HeroicAGI += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicAgi );
		newbon->HeroicINT += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicInt );
		newbon->HeroicWIS += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicWis );
		newbon->HeroicCHA += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicCha );
		newbon->HeroicMR += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicMR );
		newbon->HeroicFR += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicFR );
		newbon->HeroicCR += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicCR );
		newbon->HeroicPR += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicPR );
		newbon->HeroicDR += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicDR );
		newbon->HeroicCorrup += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicSVCorrup );
	}
	
	//FatherNitwit: New style haste, shields, and regens
	if(newbon->haste < (int16)item->Haste) {
		newbon->haste = item->Haste;
	}
	if(item->Regen > 0)
		newbon->HPRegen += item->Regen;

	if(item->ManaRegen > 0)
		newbon->ManaRegen += item->ManaRegen;
	
	if(item->EnduranceRegen > 0)
		newbon->EnduranceRegen += item->EnduranceRegen;

	if(item->Attack > 0) {
		
		int cap = RuleI(Character, ItemATKCap);
		cap += itembonuses.ItemATKCap + spellbonuses.ItemATKCap + aabonuses.ItemATKCap; 

		if((newbon->ATK + item->Attack) > cap)
			newbon->ATK = RuleI(Character, ItemATKCap);
		else
			newbon->ATK += item->Attack;
	}
	if(item->DamageShield > 0) {
		if((newbon->DamageShield + item->DamageShield) > RuleI(Character, ItemDamageShieldCap))
			newbon->DamageShield = RuleI(Character, ItemDamageShieldCap);
		else
			newbon->DamageShield += item->DamageShield;
	}
	if(item->SpellShield > 0) {
		if((newbon->SpellShield + item->SpellShield) > RuleI(Character, ItemSpellShieldingCap))
			newbon->SpellShield = RuleI(Character, ItemSpellShieldingCap);
		else
			newbon->SpellShield += item->SpellShield;
	}
	if(item->Shielding > 0) {
		if((newbon->MeleeMitigation + item->Shielding) > RuleI(Character, ItemShieldingCap))
			newbon->MeleeMitigation = RuleI(Character, ItemShieldingCap);
		else
			newbon->MeleeMitigation += item->Shielding;
	}
	if(item->StunResist > 0) {
		if((newbon->StunResist + item->StunResist) > RuleI(Character, ItemStunResistCap))
			newbon->StunResist = RuleI(Character, ItemStunResistCap);
		else
			newbon->StunResist += item->StunResist;
	}
	if(item->StrikeThrough > 0) {
		if((newbon->StrikeThrough + item->StrikeThrough) > RuleI(Character, ItemStrikethroughCap))
			newbon->StrikeThrough = RuleI(Character, ItemStrikethroughCap);
		else
			newbon->StrikeThrough += item->StrikeThrough;
	}
	if(item->Avoidance > 0) {
		if((newbon->AvoidMeleeChance + item->Avoidance) > RuleI(Character, ItemAvoidanceCap))
			newbon->AvoidMeleeChance = RuleI(Character, ItemAvoidanceCap);
		else
			newbon->AvoidMeleeChance += item->Avoidance;
	}
	if(item->Accuracy > 0) {
		if((newbon->HitChance + item->Accuracy) > RuleI(Character, ItemAccuracyCap))
			newbon->HitChance = RuleI(Character, ItemAccuracyCap);
		else
			newbon->HitChance += item->Accuracy;
	}
	if(item->CombatEffects > 0) {
		if((newbon->ProcChance + item->CombatEffects) > RuleI(Character, ItemCombatEffectsCap))
			newbon->ProcChance = RuleI(Character, ItemCombatEffectsCap);
		else
			newbon->ProcChance += item->CombatEffects;
	}
	if(item->DotShielding > 0) {
		if((newbon->DoTShielding + item->DotShielding) > RuleI(Character, ItemDoTShieldingCap))
			newbon->DoTShielding = RuleI(Character, ItemDoTShieldingCap);
		else
			newbon->DoTShielding += item->DotShielding;
	}
	
	if(item->HealAmt > 0) {
		if((newbon->HealAmt + item->HealAmt) > RuleI(Character, ItemHealAmtCap))
			newbon->HealAmt = RuleI(Character, ItemHealAmtCap);
		else
			newbon->HealAmt += item->HealAmt;
	}
	if(item->SpellDmg > 0) {
		if((newbon->SpellDmg + item->SpellDmg) > RuleI(Character, ItemSpellDmgCap))
			newbon->SpellDmg = RuleI(Character, ItemSpellDmgCap);
		else
			newbon->SpellDmg += item->SpellDmg;
	}
	if(item->Clairvoyance > 0) {
		if((newbon->Clairvoyance + item->Clairvoyance) > RuleI(Character, ItemClairvoyanceCap))
			newbon->Clairvoyance = RuleI(Character, ItemClairvoyanceCap);
		else
			newbon->Clairvoyance += item->Clairvoyance;
	}
	
	if(item->DSMitigation > 0) {
		if((newbon->DSMitigation + item->DSMitigation) > RuleI(Character, ItemDSMitigationCap))
			newbon->DSMitigation = RuleI(Character, ItemDSMitigationCap);
		else
			newbon->DSMitigation += item->DSMitigation;
	}
	if (item->Worn.Effect>0 && (item->Worn.Type == ET_WornEffect)) { // latent effects
		ApplySpellsBonuses(item->Worn.Effect, item->Worn.Level, newbon, 0, true);
	}

	if (item->Focus.Effect>0 && (item->Focus.Type == ET_Focus)) { // focus effects
		ApplySpellsBonuses(item->Focus.Effect, item->Focus.Level, newbon, 0, true);
	}

	switch(item->BardType)
	{
	case 51: /* All (e.g. Singing Short Sword) */
		{
			if(item->BardValue > newbon->singingMod)
				newbon->singingMod = item->BardValue;
			if(item->BardValue > newbon->brassMod)
				newbon->brassMod = item->BardValue;
			if(item->BardValue > newbon->stringedMod)
				newbon->stringedMod = item->BardValue;
			if(item->BardValue > newbon->percussionMod)
				newbon->percussionMod = item->BardValue;
			if(item->BardValue > newbon->windMod)
				newbon->windMod = item->BardValue;
			break;
		}
	case 50: /* Singing */
		{
			if(item->BardValue > newbon->singingMod)
				newbon->singingMod = item->BardValue;
			break;
		}
	case 23: /* Wind */
		{
			if(item->BardValue > newbon->windMod)
				newbon->windMod = item->BardValue;
			break;
		}
	case 24: /* stringed */
		{
			if(item->BardValue > newbon->stringedMod)
				newbon->stringedMod = item->BardValue;
			break;
		}
	case 25: /* brass */
		{
			if(item->BardValue > newbon->brassMod)
				newbon->brassMod = item->BardValue;
			break;
		}
	case 26: /* Percussion */
		{
			if(item->BardValue > newbon->percussionMod)
				newbon->percussionMod = item->BardValue;
			break;
		}
	}
	
	if (item->SkillModValue != 0 && item->SkillModType <= HIGHEST_SKILL){
		if ((item->SkillModValue > 0 && newbon->skillmod[item->SkillModType] < item->SkillModValue) ||
			(item->SkillModValue < 0 && newbon->skillmod[item->SkillModType] > item->SkillModValue))
		{
			newbon->skillmod[item->SkillModType] = item->SkillModValue;
		}
	}

	// Add Item Faction Mods
	if (item->FactionMod1)
	{
		if (item->FactionAmt1 > 0 && item->FactionAmt1 > GetItemFactionBonus(item->FactionMod1))
		{
			AddItemFactionBonus(item->FactionMod1, item->FactionAmt1);
		}
		else if (item->FactionAmt1 < 0 && item->FactionAmt1 < GetItemFactionBonus(item->FactionMod1))
		{
			AddItemFactionBonus(item->FactionMod1, item->FactionAmt1);
		}
	}
	if (item->FactionMod2)
	{
		if (item->FactionAmt2 > 0 && item->FactionAmt2 > GetItemFactionBonus(item->FactionMod2))
		{
			AddItemFactionBonus(item->FactionMod2, item->FactionAmt2);
		}
		else if (item->FactionAmt2 < 0 && item->FactionAmt2 < GetItemFactionBonus(item->FactionMod2))
		{
			AddItemFactionBonus(item->FactionMod2, item->FactionAmt2);
		}
	}
	if (item->FactionMod3)
	{
		if (item->FactionAmt3 > 0 && item->FactionAmt3 > GetItemFactionBonus(item->FactionMod3))
		{
			AddItemFactionBonus(item->FactionMod3, item->FactionAmt3);
		}
		else if (item->FactionAmt3 < 0 && item->FactionAmt3 < GetItemFactionBonus(item->FactionMod3))
		{
			AddItemFactionBonus(item->FactionMod3, item->FactionAmt3);
		}
	}
	if (item->FactionMod4)
	{
		if (item->FactionAmt4 > 0 && item->FactionAmt4 > GetItemFactionBonus(item->FactionMod4))
		{
			AddItemFactionBonus(item->FactionMod4, item->FactionAmt4);
		}
		else if (item->FactionAmt4 < 0 && item->FactionAmt4 < GetItemFactionBonus(item->FactionMod4))
		{
			AddItemFactionBonus(item->FactionMod4, item->FactionAmt4);
		}
	}
	
	if (item->ExtraDmgSkill != 0 && item->ExtraDmgSkill <= HIGHEST_SKILL) {
		if((newbon->SkillDamageAmount[item->ExtraDmgSkill] + item->ExtraDmgAmt) > RuleI(Character, ItemExtraDmgCap))
			newbon->SkillDamageAmount[item->ExtraDmgSkill] = RuleI(Character, ItemExtraDmgCap);
		else
			newbon->SkillDamageAmount[item->ExtraDmgSkill] += item->ExtraDmgAmt;
	}

	if (!isAug)
	{
		int i;
		for(i = 0; i < MAX_AUGMENT_SLOTS; i++) {
			AddItemBonuses(inst->GetAugment(i),newbon,true);
		}
	}

}

void Client::CalcEdibleBonuses(StatBonuses* newbon) {
#if EQDEBUG >= 11
    cout<<"Client::CalcEdibleBonuses(StatBonuses* newbon)"<<endl;
#endif
  // Search player slots for skill=14(food) and skill=15(drink)
  	uint32 i;
  	
	bool food = false;
	bool drink = false;
	for (i = 22; i <= 29; i++)
	{
		if (food && drink)
			break;
		const ItemInst* inst = GetInv().GetItem(i);
		if (inst && inst->GetItem() && inst->IsType(ItemClassCommon)) {
			const Item_Struct *item=inst->GetItem();
			if (item->ItemType == ItemTypeFood && !food)
				food = true;
			else if (item->ItemType == ItemTypeDrink && !drink)
				drink = true;
			else
				continue;
			AddItemBonuses(inst, newbon);
		}
	}
	for (i = 251; i <= 330; i++)
	{
		if (food && drink)
			break;
		const ItemInst* inst = GetInv().GetItem(i);
		if (inst && inst->GetItem() && inst->IsType(ItemClassCommon)) {
			const Item_Struct *item=inst->GetItem();
			if (item->ItemType == ItemTypeFood && !food)
				food = true;
			else if (item->ItemType == ItemTypeDrink && !drink)
				drink = true;
			else
				continue;
			AddItemBonuses(inst, newbon);
		}
	}
}

void Client::CalcAABonuses(StatBonuses* newbon) {
	memset(newbon, 0, sizeof(StatBonuses));	//start fresh

	int i;
	uint32 slots = 0;
	uint32 aa_AA = 0;
	uint32 aa_value = 0;
	for (i = 0; i < MAX_PP_AA_ARRAY; i++) {	//iterate through all of the client's AAs
		if (this->aa[i]) {	// make sure aa exists or we'll crash zone
			aa_AA = this->aa[i]->AA;	//same as aaid from the aa_effects table
			aa_value = this->aa[i]->value;	//how many points in it
			if (aa_AA > 0 || aa_value > 0) {	//do we have the AA? if 1 of the 2 is set, we can assume we do
				//slots = database.GetTotalAALevels(aa_AA);	//find out how many effects from aa_effects table
				slots = zone->GetTotalAALevels(aa_AA);	//find out how many effects from aa_effects, which is loaded into memory
				if (slots > 0)	//and does it have any effects? may be able to put this above, not sure if it runs on each iteration
					ApplyAABonuses(aa_AA, slots, newbon);	//add the bonuses
			}
		}
	}
}


//A lot of the normal spell functions (IsBlankSpellEffect, etc) are set for just spells (in common/spdat.h). 
//For now, we'll just put them directly into the code and comment with the corresponding normal function
//Maybe we'll fix it later? :-D
void Client::ApplyAABonuses(uint32 aaid, uint32 slots, StatBonuses* newbon) 
{
	if(slots == 0)	//sanity check. why bother if no slots to fill?
		return;

	//from AA_Ability struct
	uint32 effect = 0;
	int32 base1 = 0;
	int32 base2 = 0;	//only really used for SE_RaiseStatCap & SE_ReduceSkillTimer in aa_effects table
	uint32 slot = 0;

	std::map<uint32, std::map<uint32, AA_Ability> >::const_iterator find_iter = aa_effects.find(aaid);
	if(find_iter == aa_effects.end())
	{
		return;
	}

	for (map<uint32, AA_Ability>::const_iterator iter = aa_effects[aaid].begin(); iter != aa_effects[aaid].end(); ++iter) {
		effect = iter->second.skill_id;
		base1 = iter->second.base1;
		base2 = iter->second.base2;
		slot = iter->second.slot;

		//we default to 0 (SE_CurrentHP) for the effect, so if there aren't any base1/2 values, we'll just skip it
		if (effect == 0 && base1 == 0 && base2 == 0)
			continue;

		//IsBlankSpellEffect()
		if (effect == SE_Blank || (effect == SE_CHA && base1 == 0) || effect == SE_StackingCommand_Block || effect == SE_StackingCommand_Overwrite)
			continue;

		_log(AA__BONUSES, "Applying Effect %d from AA %u in slot %d (base1: %d, base2: %d) on %s", effect, aaid, slot, base1, base2, this->GetCleanName());

		uint8 focus = IsFocusEffect(0, 0, true,effect);
		if (focus)
		{
			newbon->FocusEffects[focus] = effect;
			continue;
		}

		switch (effect)
		{
			//Note: AA effects that use accuracy are skill limited, while spell effect is not.
			case SE_Accuracy:
				if ((base2 == -1) && (newbon->Accuracy[HIGHEST_SKILL+1] < base1))
					newbon->Accuracy[HIGHEST_SKILL+1] = base1;
				else if (newbon->Accuracy[base2] < base1)
					newbon->Accuracy[base2] += base1;
				break;
			case SE_CurrentHP: //regens
				newbon->HPRegen += base1;
				break;
			case SE_CurrentEndurance: 
				newbon->EnduranceRegen += base1;
				break;
			case SE_MovementSpeed:
				newbon->movementspeed += base1;	//should we let these stack?
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
				//handled by client
				break;
			case SE_CurrentMana:
				newbon->ManaRegen += base1;
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
				switch(base2)
				{
					//are these #define'd somewhere?
					case 0: //str
						newbon->STRCapMod += base1;
						break;
					case 1: //sta
						newbon->STACapMod += base1;
						break;
					case 2: //agi
						newbon->AGICapMod += base1;
						break;
					case 3: //dex
						newbon->DEXCapMod += base1;
						break;
					case 4: //wis
						newbon->WISCapMod += base1;
						break;
					case 5: //int
						newbon->INTCapMod += base1;
						break;
					case 6: //cha
						newbon->CHACapMod += base1;
						break;
					case 7: //mr
						newbon->MRCapMod += base1;
						break;
					case 8: //cr
						newbon->CRCapMod += base1;
						break;
					case 9: //fr
						newbon->FRCapMod += base1;
						break;
					case 10: //pr
						newbon->PRCapMod += base1;
						break;
					case 11: //dr
						newbon->DRCapMod += base1;
						break;
					case 12: //corruption
						newbon->CorrupCapMod += base1;
						break;
				}
				break;
			case SE_PetDiscipline2:
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
			case SE_ArcheryDamageModifier:
				newbon->ArcheryDamageModifier += base1;
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
				newbon->AvoidMeleeChance += base1;
				break;
			case SE_CombatStability:
				newbon->CombatStability += base1;
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
				newbon->ProcChance += base1;
				break;
			case SE_RiposteChance:
				newbon->RiposteChance += base1;
				break;
			case SE_Flurry:
				newbon->FlurryChance += base1;
				break;
			case SE_PetFlurry:
				newbon->PetFlurry = base1;
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

			case SE_SpellOnKill:
				for(int i = 0; i < MAX_SPELL_TRIGGER*3; i+=3)
				{
					if(!newbon->SpellOnKill[i] || ((newbon->SpellOnKill[i] == base2) && (newbon->SpellOnKill[i+1] < base1)))
					{
						//base1 = chance, base2 = SpellID to be triggered,  base3 = min npc level
						newbon->SpellOnKill[i] = base2;
						newbon->SpellOnKill[i+1] = base1;
						
						if (GetLevel() > 15)
							newbon->SpellOnKill[i+2] = GetLevel() - 15; //AA specifiy "non-trivial"
						else
							newbon->SpellOnKill[i+2] = 0;

						break;
					}
				}
			break;
				
			case SE_SpellOnDeath:
				for(int i = 0; i < MAX_SPELL_TRIGGER*2; i+=2)
				{
					if(!newbon->SpellOnDeath[i])
					{
						// base1 = SpellID to be triggered, base2 = chance to fire
						newbon->SpellOnDeath[i] = base1;
						newbon->SpellOnDeath[i+1] = base2;
						break;
					}
				}
			break;

			case SE_TriggerOnCast:
			
				for(int i = 0; i < MAX_SPELL_TRIGGER; i++)
				{
					if (newbon->SpellTriggers[i] == aaid)
						break;

					if(!newbon->SpellTriggers[i])
					{
						//Save the 'aaid' of each triggerable effect to an array
						newbon->SpellTriggers[i] = aaid;
						break;
					}
				}
			break;
				
			case SE_CriticalHitChance:
			{
				if(base2 == -1)
					newbon->CriticalHitChance[HIGHEST_SKILL+1] += base1;
				else
					newbon->CriticalHitChance[base2] += base1;
			}
			break;

			case SE_CriticalDamageMob:
			{
				// base1 = effect value, base2 = skill restrictions(-1 for all)
				if(base2 == -1)
					newbon->CritDmgMob[HIGHEST_SKILL+1] += base1;
				else
					newbon->CritDmgMob[base2] += base1;
				break;
			}

			case SE_CriticalSpellChance:
			{
				newbon->CriticalSpellChance += base1;
				
				if (base2 > 100)
					newbon->SpellCritDmgIncrease += (base2 - 100);
				
				break;
			}

			case SE_ResistFearChance:
			{
				if(base1 == 100) // If we reach 100% in a single spell/item then we should be immune to negative fear resist effects until our immunity is over
					newbon->Fearless = true;
					
				newbon->ResistFearChance += base1; // these should stack
				break;
			}

			case SE_SkillDamageAmount:
			{
				if(base2 == -1)
					newbon->SkillDamageAmount[HIGHEST_SKILL+1] += base1;
				else
					newbon->SkillDamageAmount[base2] += base1;
				break;
			}

			case SE_SpecialAttackKBProc:
			{
				//You can only have one of these per client. [AA Dragon Punch]
				newbon->SpecialAttackKBProc[0] = base1; //Chance base 100 = 25% proc rate
				newbon->SpecialAttackKBProc[1] = base2; //Skill to KB Proc Off
				break;
			}

			case SE_DamageModifier:
			{
				if(base2 == -1)
					newbon->DamageModifier[HIGHEST_SKILL+1] += base1;
				else
					newbon->DamageModifier[base2] += base1;
				break;
			}

			case SE_SlayUndead:
			{
				if(newbon->SlayUndead[1] < base1)
					newbon->SlayUndead[0] = base1; // Rate
					newbon->SlayUndead[1] = base2; // Damage Modifier
				break;
			}

			case SE_GiveDoubleRiposte:
			{
				//0=Regular Riposte 1=Skill Attack Riposte 2=Skill
				if(base2 == 0){
					if(newbon->GiveDoubleRiposte[0] < base1)
						newbon->GiveDoubleRiposte[0] = base1;
				}
				//Only for special attacks.
				else if(base2 > 0 && (newbon->GiveDoubleRiposte[1] < base1)){
					newbon->GiveDoubleRiposte[1] = base1;
					newbon->GiveDoubleRiposte[2] = base2;
				}

				break;
			}

			//Kayen: Not sure best way to implement this yet.
			//Physically raises skill cap ie if 55/55 it will raise to 55/60
			case SE_RaiseSkillCap:
			{
				if(newbon->RaiseSkillCap[0] < base1){
					newbon->RaiseSkillCap[0] = base1; //value
					newbon->RaiseSkillCap[1] = base2; //skill
				}
				break;
			}

			case SE_MasteryofPast:
			{
				if(newbon->MasteryofPast < base1)
					newbon->MasteryofPast = base1;
				break;
			}

			case SE_CastingLevel2:
			case SE_CastingLevel:
			{
				newbon->effective_casting_level += base1;
				break;
			}


			case SE_DivineSave:
			{
				if(newbon->DivineSaveChance[0] < base1)
				{
					newbon->DivineSaveChance[0] = base1;
					newbon->DivineSaveChance[1] = base2;
				}
				break;
			}

			case SE_SpellEffectResistChance:
			{
				for(int e = 0; e < MAX_RESISTABLE_EFFECTS*2; e+=2)
				{
					if(!newbon->SEResist[e] || ((newbon->SEResist[e] = base2) && (newbon->SEResist[e+1] < base1)) ){
						newbon->SEResist[e] = base2;
						newbon->SEResist[e+1] = base1;
					break;
					}
				}
				break;
			}

			case SE_MitigateDamageShield:
			{
				if (base1 < 0)
					base1 = base1*(-1);

				newbon->DSMitigationOffHand += base1;
				break;
			}

			case SE_FinishingBlow:
			{
			
				//base1 = chance, base2 = damage
				if (newbon->FinishingBlow[1] < base2){
					newbon->FinishingBlow[0] = base1;
					newbon->FinishingBlow[1] = base2;
				}
				break;
			}

			case SE_FinishingBlowLvl:
			{
				//base1 = level, base2 = ??? (Set to 200 in AA data, possible proc rate mod?)
				if (newbon->FinishingBlowLvl[0] < base1){
					newbon->FinishingBlowLvl[0] = base1;
					newbon->FinishingBlowLvl[1] = base2;
				}
				break;
			}
		}
	}
}

void Mob::CalcSpellBonuses(StatBonuses* newbon)
{
	int i;

	memset(newbon, 0, sizeof(StatBonuses));
	newbon->AggroRange = -1;
	newbon->AssistRange = -1;

	uint32 buff_count = GetMaxTotalSlots();
	for(i = 0; i < buff_count; i++) {
		if(buffs[i].spellid != SPELL_UNKNOWN)
			ApplySpellsBonuses(buffs[i].spellid, buffs[i].casterlevel, newbon, buffs[i].casterid, false, buffs[i].ticsremaining,i);
	}

	//Removes the spell bonuses that are effected by a 'negate' debuff.
	if (spellbonuses.NegateEffects){
		for(i = 0; i < buff_count; i++) {
			if( (buffs[i].spellid != SPELL_UNKNOWN) && (IsEffectInSpell(buffs[i].spellid, SE_NegateSpellEffect)) )
				NegateSpellsBonuses(buffs[i].spellid);
		}
	}
	//this prolly suffer from roundoff error slightly...
	newbon->AC = newbon->AC * 10 / 34;	//ratio determined impirically from client.
}

void Mob::ApplySpellsBonuses(uint16 spell_id, uint8 casterlevel, StatBonuses* newbon, uint16 casterId, bool item_bonus, uint32 ticsremaining, int buffslot)
{
	int i, effect_value;
	Mob *caster = NULL;
	
	if(!IsValidSpell(spell_id))
		return;
	
	if(casterId > 0)
		caster = entity_list.GetMob(casterId);

	for (i = 0; i < EFFECT_COUNT; i++)
	{
		if(IsBlankSpellEffect(spell_id, i))
			continue;

		uint8 focus = IsFocusEffect(spell_id, i);
		if (focus)
		{
			newbon->FocusEffects[focus] = spells[spell_id].effectid[i];
			continue;
		}
		
		effect_value = CalcSpellEffectValue(spell_id, i, casterlevel, caster, ticsremaining);

		switch (spells[spell_id].effectid[i])
		{
			case SE_CurrentHP: //regens
				if(effect_value > 0) {
					newbon->HPRegen += effect_value;
				}
				break;

			case SE_CurrentEndurance: 
				newbon->EnduranceRegen += effect_value;
				break;
		
			case SE_ChangeFrenzyRad:
			{
				// redundant to have level check here
				if(newbon->AggroRange == -1 || effect_value < newbon->AggroRange)
				{
					newbon->AggroRange = effect_value;
				}
				break;
			}

			case SE_Harmony:
			{
				// neotokyo: Harmony effect as buff - kinda tricky
				// harmony could stack with a lull spell, which has better aggro range
				// take the one with less range in any case
				if(newbon->AssistRange == -1 || effect_value < newbon->AssistRange)
				{
					newbon->AssistRange = effect_value;
				}
				break;
			}

			case SE_AttackSpeed:
			{
				if ((effect_value - 100) > 0) { // Haste
					if (newbon->haste < 0) break; // Slowed - Don't apply haste
					if ((effect_value - 100) > newbon->haste) {
						newbon->haste = effect_value - 100;
					}
				} 
				else if ((effect_value - 100) < 0) { // Slow
					//Slow Mitigation works by taking the amount that would be slowed, and adding a multiplied version of the difference.
					int real_slow_value = (100 - effect_value) * -1;
					if (slow_mitigation){
						int new_effect_value = SlowMitigation(false,caster,real_slow_value);
						if (new_effect_value < newbon->haste) {
							newbon->haste = new_effect_value;
							SlowMitigation(true,caster);
						}
					}
					else {
						if (real_slow_value < newbon->haste) 
							newbon->haste = real_slow_value;
					}
				}
				break;
			}

 			case SE_AttackSpeed2:
			{
				if ((effect_value - 100) > 0) { // Haste V2 - Stacks with V1 but does not Overcap
					if ((effect_value - 100) > newbon->hastetype2) {
						newbon->hastetype2 = effect_value - 100;
					}
				}
				break;
 			}
 
 			case SE_AttackSpeed3:
 			{
				if (effect_value > 0) { // Haste V3 - Stacks and Overcaps
					if (effect_value > newbon->hastetype3) {
						newbon->hastetype3 = effect_value;
					}
				}
				break;
 			}

			case SE_AttackSpeed4:
			{
				if (effect_value > 0) {
					if (slow_mitigation){
						int new_effect_value = SlowMitigation(false,caster,effect_value);
						if (new_effect_value > newbon->inhibitmelee) {
								newbon->inhibitmelee = new_effect_value;
								SlowMitigation(true,caster);
						}
					}
					else if (effect_value > newbon->inhibitmelee) {
								newbon->inhibitmelee = effect_value;
					}
				}
				break;
			}

			case SE_TotalHP:
			{
				newbon->HP += effect_value;
				break;
			}
			
			case SE_ManaRegen_v2:
			case SE_CurrentMana:
			{
				newbon->ManaRegen += effect_value;
				break;
			}

			case SE_ManaPool:
			{
				newbon->Mana += effect_value;
				break;
			}

			case SE_Stamina:
			{
				newbon->EnduranceReduction += effect_value;
				break;
			}
			
			case SE_ACv2:	
			case SE_ArmorClass:
			{
				newbon->AC += effect_value;
				break;
			}

			case SE_ATK:
			{
				newbon->ATK += effect_value;
				break;
			}

			case SE_STR:
			{
				newbon->STR += effect_value;
				break;
			}

			case SE_DEX:
			{
				newbon->DEX += effect_value;
				break;
			}

			case SE_AGI:
			{
				newbon->AGI += effect_value;
				break;
			}

			case SE_STA:
			{
				newbon->STA += effect_value;
				break;
			}

			case SE_INT:
			{
				newbon->INT += effect_value;
				break;
			}

			case SE_WIS:
			{
				newbon->WIS += effect_value;
				break;
			}

			case SE_CHA:
			{
				if (spells[spell_id].base[i] != 0) {
					newbon->CHA += effect_value;
				}
				break;
			}

			case SE_AllStats:
			{
				newbon->STR += effect_value;
				newbon->DEX += effect_value;
				newbon->AGI += effect_value;
				newbon->STA += effect_value;
				newbon->INT += effect_value;
				newbon->WIS += effect_value;
				newbon->CHA += effect_value;
				break;
			}

			case SE_ResistFire:
			{
				newbon->FR += effect_value;
				break;
			}

			case SE_ResistCold:
			{
				newbon->CR += effect_value;
				break;
			}

			case SE_ResistPoison:
			{
				newbon->PR += effect_value;
				break;
			}

			case SE_ResistDisease:
			{
				newbon->DR += effect_value;
				break;
			}

			case SE_ResistMagic:
			{
				newbon->MR += effect_value;
				break;
			}

			case SE_ResistAll:
			{
				newbon->MR += effect_value;
				newbon->DR += effect_value;
				newbon->PR += effect_value;
				newbon->CR += effect_value;
				newbon->FR += effect_value;
				break;
			}

			case SE_ResistCorruption:
			{
				newbon->Corrup += effect_value;
				break;
			}
			
			case SE_RaiseStatCap:
			{
				switch(spells[spell_id].base2[i])
				{
					//are these #define'd somewhere?
					case 0: //str
						newbon->STRCapMod += effect_value;
						break;
					case 1: //sta
						newbon->STACapMod += effect_value;
						break;
					case 2: //agi
						newbon->AGICapMod += effect_value;
						break;
					case 3: //dex
						newbon->DEXCapMod += effect_value;
						break;
					case 4: //wis
						newbon->WISCapMod += effect_value;
						break;
					case 5: //int
						newbon->INTCapMod += effect_value;
						break;
					case 6: //cha
						newbon->CHACapMod += effect_value;
						break;
					case 7: //mr
						newbon->MRCapMod += effect_value;
						break;
					case 8: //cr
						newbon->CRCapMod += effect_value;
						break;
					case 9: //fr
						newbon->FRCapMod += effect_value;
						break;
					case 10: //pr
						newbon->PRCapMod += effect_value;
						break;
					case 11: //dr
						newbon->DRCapMod += effect_value;
						break;
					case 12: // corruption
						newbon->CorrupCapMod += effect_value;
						break;
				}
				break;
			}

			case SE_CastingLevel2:
			case SE_CastingLevel:	// Brilliance of Ro
			{
				newbon->effective_casting_level += effect_value;
				break;
			}

			case SE_MovementSpeed:
				newbon->movementspeed += effect_value;
				break;
			
			case SE_SpellDamageShield:
				newbon->SpellDamageShield += effect_value;
				break;

			case SE_DamageShield:
			{
				newbon->DamageShield += effect_value;
				newbon->DamageShieldSpellID = spell_id;
				newbon->DamageShieldType = GetDamageShieldType(spell_id);
				break;
			}
			
			case SE_ReverseDS:
			{
				newbon->ReverseDamageShield += effect_value;
				newbon->ReverseDamageShieldSpellID = spell_id;
				newbon->ReverseDamageShieldType = GetDamageShieldType(spell_id);
				break;
			}

			case SE_Reflect:
				newbon->reflect_chance += effect_value;
				break;

			case SE_SingingSkill:
			{
				if(effect_value > newbon->singingMod)
					newbon->singingMod = effect_value;
				break;
			}
			
			case SE_ChangeAggro:
				newbon->hatemod += effect_value;
				break;

			case SE_MeleeMitigation:
				//for some reason... this value is negative for increased mitigation
				newbon->MeleeMitigation -= effect_value;
				break;
			
			case SE_CriticalHitChance:
			{

				if (RuleB(Spells, AdditiveBonusValues) && item_bonus) {
					if(spells[spell_id].base2[i] == -1)
						newbon->CriticalHitChance[HIGHEST_SKILL+1] += effect_value;
					else
						newbon->CriticalHitChance[spells[spell_id].base2[i]] += effect_value;
				}
				
				else if(effect_value < 0) {
					
					if(spells[spell_id].base2[i] == -1 && newbon->CriticalHitChance[HIGHEST_SKILL+1] > effect_value)
						newbon->CriticalHitChance[HIGHEST_SKILL+1] = effect_value;
					else if(spells[spell_id].base2[i] != -1 && newbon->CriticalHitChance[spells[spell_id].base2[i]] > effect_value)
						newbon->CriticalHitChance[spells[spell_id].base2[i]] = effect_value;
				}


				else if(spells[spell_id].base2[i] == -1 && newbon->CriticalHitChance[HIGHEST_SKILL+1] < effect_value)
						newbon->CriticalHitChance[HIGHEST_SKILL+1] = effect_value;
				else if(spells[spell_id].base2[i] != -1 && newbon->CriticalHitChance[spells[spell_id].base2[i]] < effect_value)
						newbon->CriticalHitChance[spells[spell_id].base2[i]] = effect_value;
				break;
			}
				
			case SE_CrippBlowChance:
			{
				if (RuleB(Spells, AdditiveBonusValues) && item_bonus) 
					newbon->CrippBlowChance += effect_value;

				else if((effect_value < 0) && (newbon->CrippBlowChance > effect_value))
						newbon->CrippBlowChance = effect_value;

				else if(newbon->CrippBlowChance < effect_value)
					newbon->CrippBlowChance = effect_value;
					
				break;
			}
				
			case SE_AvoidMeleeChance:
			{
				//multiplier is to be compatible with item effects, watching for overflow too
				effect_value = effect_value<3000? effect_value * 10 : 30000;
				if (RuleB(Spells, AdditiveBonusValues) && item_bonus) 
					newbon->AvoidMeleeChance += effect_value;

				else if((effect_value < 0) && (newbon->AvoidMeleeChance  > effect_value))
					newbon->AvoidMeleeChance = effect_value;
				
				else if(newbon->AvoidMeleeChance  < effect_value)
					newbon->AvoidMeleeChance = effect_value;
				break;
			}
				
			case SE_RiposteChance:
			{
				if (RuleB(Spells, AdditiveBonusValues) && item_bonus) 
					newbon->RiposteChance += effect_value;

				else if((effect_value < 0) && (newbon->RiposteChance > effect_value))
					newbon->RiposteChance = effect_value;
				
				else if(newbon->RiposteChance < effect_value)
					newbon->RiposteChance = effect_value;
				break;
			}
				
			case SE_DodgeChance:
			{
				if (RuleB(Spells, AdditiveBonusValues) && item_bonus) 
					newbon->DodgeChance += effect_value;

				else if((effect_value < 0) && (newbon->DodgeChance > effect_value))
					newbon->DodgeChance  = effect_value;

				if(newbon->DodgeChance < effect_value)
					newbon->DodgeChance = effect_value;
				break;
			}
				
			case SE_ParryChance:
			{
				if (RuleB(Spells, AdditiveBonusValues) && item_bonus) 
					newbon->ParryChance += effect_value;

				else if((effect_value < 0) && (newbon->ParryChance > effect_value))
					newbon->ParryChance  = effect_value;

				if(newbon->ParryChance < effect_value)
					newbon->ParryChance = effect_value;
				break;
			}
				
			case SE_DualWieldChance:
			{
				if (RuleB(Spells, AdditiveBonusValues) && item_bonus) 
					newbon->DualWieldChance += effect_value;

				else if((effect_value < 0) && (newbon->DualWieldChance > effect_value))
					newbon->DualWieldChance  = effect_value;

				if(newbon->DualWieldChance < effect_value)
					newbon->DualWieldChance = effect_value;
				break;
			}
				
			case SE_DoubleAttackChance:
			{

				if (RuleB(Spells, AdditiveBonusValues) && item_bonus) 
					newbon->DoubleAttackChance += effect_value;

				else if((effect_value < 0) && (newbon->DoubleAttackChance > effect_value))
					newbon->DoubleAttackChance  = effect_value;

				if(newbon->DoubleAttackChance < effect_value)
					newbon->DoubleAttackChance = effect_value;
				break;
			}

			case SE_TripleAttackChance:
			{

				if (RuleB(Spells, AdditiveBonusValues) && item_bonus) 
					newbon->TripleAttackChance  += effect_value;

				else if((effect_value < 0) && (newbon->TripleAttackChance  > effect_value))
					newbon->TripleAttackChance = effect_value;

				if(newbon->TripleAttackChance < effect_value)
					newbon->TripleAttackChance = effect_value;
				break;
			}
				
			case SE_MeleeLifetap:
			{
				if (RuleB(Spells, AdditiveBonusValues) && item_bonus) 
					newbon->MeleeLifetap += spells[spell_id].base[i];

				else if((effect_value < 0) && (newbon->MeleeLifetap  > effect_value))
					newbon->MeleeLifetap = spells[spell_id].base[i];
				
				if(newbon->MeleeLifetap < spells[spell_id].base[i])
					newbon->MeleeLifetap = spells[spell_id].base[i];
				break;
			}
				
			case SE_AllInstrumentMod:
			{
				if(effect_value > newbon->singingMod)
					newbon->singingMod = effect_value;
				if(effect_value > newbon->brassMod)
					newbon->brassMod = effect_value;
				if(effect_value > newbon->percussionMod)
					newbon->percussionMod = effect_value;
				if(effect_value > newbon->windMod)
					newbon->windMod = effect_value;
				if(effect_value > newbon->stringedMod)
					newbon->stringedMod = effect_value;
				break;
			}
				
			case SE_ResistSpellChance:
				newbon->ResistSpellChance += effect_value;
				break;

			case SE_ResistFearChance:
			{
				if(effect_value == 100) // If we reach 100% in a single spell/item then we should be immune to negative fear resist effects until our immunity is over
					newbon->Fearless = true;
					
				newbon->ResistFearChance += effect_value; // these should stack
				break;
			}
			
			case SE_Fearless:
				newbon->Fearless = true;  
				break;

 			case SE_HundredHands:
 			{
				if (RuleB(Spells, AdditiveBonusValues) && item_bonus) 
					newbon->HundredHands += spells[spell_id].base[i];

				if (effect_value > 0 && effect_value > newbon->HundredHands)
					newbon->HundredHands = effect_value; //Increase Weapon Delay
				else if (effect_value < 0 && effect_value < newbon->HundredHands)
					newbon->HundredHands = effect_value; //Decrease Weapon Delay
				break;
 			}
				
			case SE_MeleeSkillCheck:
			{
				if(newbon->MeleeSkillCheck < effect_value) {
					newbon->MeleeSkillCheck = effect_value;
					newbon->MeleeSkillCheckSkill = spells[spell_id].base2[i]==-1?255:spells[spell_id].base2[i];
				}
				break;
			}
				
			case SE_HitChance:
			{

				if (RuleB(Spells, AdditiveBonusValues) && item_bonus){ 
					if(spells[spell_id].base2[i] == -1)
						newbon->HitChanceEffect[HIGHEST_SKILL+1] += effect_value;
					else
						newbon->HitChanceEffect[spells[spell_id].base2[i]] += effect_value;
				}

				else if(spells[spell_id].base2[i] == -1){

					if ((effect_value < 0) && (newbon->HitChanceEffect[HIGHEST_SKILL+1] > effect_value))
						newbon->HitChanceEffect[HIGHEST_SKILL+1] = effect_value;

					else if (!newbon->HitChanceEffect[HIGHEST_SKILL+1] || 
							((newbon->HitChanceEffect[HIGHEST_SKILL+1] > 0) && (newbon->HitChanceEffect[HIGHEST_SKILL+1] < effect_value)))
							newbon->HitChanceEffect[HIGHEST_SKILL+1] = effect_value;
				}
				
				else {

					if ((effect_value < 0) && (newbon->HitChanceEffect[spells[spell_id].base2[i]] > effect_value))
						newbon->HitChanceEffect[spells[spell_id].base2[i]] = effect_value;

					else if (!newbon->HitChanceEffect[spells[spell_id].base2[i]] || 
							((newbon->HitChanceEffect[spells[spell_id].base2[i]] > 0) && (newbon->HitChanceEffect[spells[spell_id].base2[i]] < effect_value)))
							newbon->HitChanceEffect[spells[spell_id].base2[i]] = effect_value;
				}

				break;
				
			}
				
			case SE_DamageModifier:
			{
				if(spells[spell_id].base2[i] == -1)
					newbon->DamageModifier[HIGHEST_SKILL+1] += effect_value;
				else
					newbon->DamageModifier[spells[spell_id].base2[i]] += effect_value;
				break;
			}
				
			case SE_MinDamageModifier:
			{
				if(spells[spell_id].base2[i] == -1)
					newbon->MinDamageModifier[HIGHEST_SKILL+1] += effect_value;
				else
					newbon->MinDamageModifier[spells[spell_id].base2[i]] += effect_value;
				break;
			}
				
			case SE_StunResist:
			{
				if(newbon->StunResist < effect_value)
					newbon->StunResist = effect_value;
				break;
			}
				
			case SE_ProcChance:
			{
				//multiplier is to be compatible with item effects,watching for overflow too
				effect_value = effect_value<3000? effect_value : 3000;

				if (RuleB(Spells, AdditiveBonusValues) && item_bonus) 
					newbon->ProcChance += effect_value;

				else if((effect_value < 0) && (newbon->DoubleAttackChance > effect_value))
					newbon->ProcChance  = effect_value;

				if(newbon->ProcChance < effect_value)
					newbon->ProcChance = effect_value;

				break;
			}
				
			case SE_ExtraAttackChance:
				newbon->ExtraAttackChance += effect_value;
				break;
			
			case SE_PercentXPIncrease:
			{
				if(newbon->XPRateMod < effect_value)
					newbon->XPRateMod = effect_value;
				break;
			}

			case SE_DeathSave:
			{
				if(newbon->DeathSave[0] < effect_value)
				{
					newbon->DeathSave[0] = effect_value; //1='Partial' 2='Full'
					newbon->DeathSave[1] = buffslot;	
					//These are used in later expansion spell effects.
					newbon->DeathSave[2] = spells[spell_id].base2[i];//Min level for HealAmt
					newbon->DeathSave[3] = spells[spell_id].max[i];//HealAmt
				}
				break;
			}

			case SE_DivineSave:
			{
				if (RuleB(Spells, AdditiveBonusValues) && item_bonus) {
					newbon->DivineSaveChance[0] += effect_value;
					newbon->DivineSaveChance[1] = 0;
				}

				else if(newbon->DivineSaveChance[0] < effect_value)
				{
					newbon->DivineSaveChance[0] = effect_value;
					newbon->DivineSaveChance[1] = spells[spell_id].base2[i];
					//SetDeathSaveChance(true);
				}
				break;
			}

			case SE_Flurry:
				newbon->FlurryChance += effect_value;
				break;

			case SE_Accuracy:
			{
				if ((effect_value < 0) && (newbon->Accuracy[HIGHEST_SKILL+1] > effect_value))
						newbon->Accuracy[HIGHEST_SKILL+1] = effect_value;

				else if (!newbon->Accuracy[HIGHEST_SKILL+1] || 
						((newbon->Accuracy[HIGHEST_SKILL+1] > 0) && (newbon->Accuracy[HIGHEST_SKILL+1] < effect_value)))
							newbon->Accuracy[HIGHEST_SKILL+1] = effect_value;
				break;
			}

			case SE_MaxHPChange:
				newbon->MaxHPChange += effect_value;
				break;

			case SE_EndurancePool:
				newbon->Endurance += effect_value;
				break;

			case SE_HealRate:
				newbon->HealRate += effect_value;
				break;

			case SE_SkillDamageTaken:
			{
				if(spells[spell_id].base2[i] == -1)
					newbon->SkillDmgTaken[HIGHEST_SKILL+1] += effect_value;
				else
					newbon->SkillDmgTaken[spells[spell_id].base2[i]] += effect_value;
				break;
			}
			
			case SE_TriggerOnCast:
			{
				for(int e = 0; e < MAX_SPELL_TRIGGER; e++)
				{
					if(!newbon->SpellTriggers[e])
					{
						newbon->SpellTriggers[e] = spell_id;
						break;
					}
				}
				break;
			}

			case SE_SpellCritChance:
				newbon->CriticalSpellChance += effect_value;
				break;
			
			case SE_CriticalSpellChance:
			{
				newbon->CriticalSpellChance += effect_value;
				if (spells[spell_id].base2[i] > 100)
					newbon->SpellCritDmgIncrease += (spells[spell_id].base2[i] - 100);
				break;
			}

			case SE_SpellCritDmgIncrease:
				newbon->SpellCritDmgIncrease += effect_value;
				break;

			case SE_DotCritDmgIncrease:
				newbon->DotCritDmgIncrease += effect_value;
				break;

			case SE_CriticalHealChance2:
			case SE_CriticalHealChance:
				newbon->CriticalHealChance += effect_value;
				break;
			
			case SE_CriticalHealOverTime2:
			case SE_CriticalHealOverTime:
				newbon->CriticalHealOverTime += effect_value;
				break;
				
			case SE_MitigateDamageShield:
			{
				if (effect_value < 0)
					effect_value = effect_value*-1;
			
				newbon->DSMitigationOffHand += effect_value;
				break;
			}
			
			case SE_CriticalDoTChance:
				newbon->CriticalDoTChance += effect_value;
				break;
	
			case SE_SpellOnKill:
			{
				for(int e = 0; e < MAX_SPELL_TRIGGER*3; e+=3)
				{
					if(!newbon->SpellOnKill[e])
					{
						// Base2 = Spell to fire | Base1 = % chance | Base3 = min level
						newbon->SpellOnKill[e] = spells[spell_id].base2[i];
						newbon->SpellOnKill[e+1] = effect_value;
						newbon->SpellOnKill[e+2] = spells[spell_id].max[i];
						break;
					}
				}
				break;
			}
			
			case SE_SpellOnDeath:
			{
				for(int e = 0; e < MAX_SPELL_TRIGGER; e+=2)
				{
					if(!newbon->SpellOnDeath[e])
					{
						// Base2 = Spell to fire | Base1 = % chance
						newbon->SpellOnDeath[e] = spells[spell_id].base2[i];
						newbon->SpellOnDeath[e+1] = effect_value;
						break;
					}
				}
				break;
			}

			case SE_CriticalDamageMob:
			{
				if(spells[spell_id].base2[i] == -1)
					newbon->CritDmgMob[HIGHEST_SKILL+1] += effect_value;
				else
					newbon->CritDmgMob[spells[spell_id].base2[i]] += effect_value;
				break;
			}

			case SE_ReduceSkillTimer:
			{
				if(newbon->SkillReuseTime[spells[spell_id].base2[i]] < effect_value)
					newbon->SkillReuseTime[spells[spell_id].base2[i]] = effect_value;
				break;
			}

			case SE_SkillDamageAmount:
			{
				if(spells[spell_id].base2[i] == -1)
					newbon->SkillDamageAmount[HIGHEST_SKILL+1] += effect_value;
				else
					newbon->SkillDamageAmount[spells[spell_id].base2[i]] += effect_value;
				break;
			}

			case SE_GravityEffect:
				newbon->GravityEffect = 1;
				break;

			case SE_AntiGate:
				newbon->AntiGate = true;
				break;

			case SE_MagicWeapon:
				newbon->MagicWeapon = true;
				break;

			case SE_IncreaseBlockChance:
				newbon->IncreaseBlockChance += effect_value;
				break;

			case SE_PersistantCasting:
				newbon->PersistantCasting += effect_value;
				break;

			case SE_LimitHPPercent:
			{
				if(newbon->HPPercCap != 0 && newbon->HPPercCap > effect_value)
					newbon->HPPercCap = effect_value;
				else if(newbon->HPPercCap == 0)
					newbon->HPPercCap = effect_value;
					
				break;
			}
			case SE_LimitManaPercent:
			{
				if(newbon->ManaPercCap != 0 && newbon->ManaPercCap > effect_value)
					newbon->ManaPercCap = effect_value;
				else if(newbon->ManaPercCap == 0)
					newbon->ManaPercCap = effect_value;

				break;
			}
			case SE_LimitEndPercent:
			{
				if(newbon->EndPercCap != 0 && newbon->EndPercCap > effect_value)
					newbon->EndPercCap = effect_value;
				else if(newbon->EndPercCap == 0)
					newbon->EndPercCap = effect_value;

				break;
			}

			case SE_BlockNextSpellFocus:
				newbon->BlockNextSpell = true;
				break;

			case SE_NegateSpellEffect:
				newbon->NegateEffects = true;
				break;

			case SE_ImmuneFleeing:
				newbon->ImmuneToFlee = true;
				break;

			case SE_DelayDeath:
				newbon->DelayDeath += effect_value;
				break;

			case SE_SpellProcChance:
				newbon->SpellProcChance += effect_value;
				break;
			
			case SE_CharmBreakChance:
				newbon->CharmBreakChance += effect_value;
				break;

			case SE_BardSongRange:
				newbon->SongRange += effect_value;
				break;
			
			case SE_HPToMana:
			{
				//Lower the ratio the more favorable
				if((!newbon->HPToManaConvert) || (newbon->HPToManaConvert >= effect_value)) 
				newbon->HPToManaConvert = spells[spell_id].base[i];
				break;
			}

			case SE_SkillDamageAmount2:
			{
				if(spells[spell_id].base2[i] == -1)
					newbon->SkillDamageAmount2[HIGHEST_SKILL+1] += effect_value;
				else
					newbon->SkillDamageAmount2[spells[spell_id].base2[i]] += effect_value;
				break;
			}

			case SE_NegateAttacks:
			{
				if (!newbon->NegateAttacks[0]){
					newbon->NegateAttacks[0] = 1;
					newbon->NegateAttacks[1] = buffslot;
				}
				break;
			}

			case SE_MitigateMeleeDamage:
			{
				if (newbon->MitigateMeleeRune[0] < effect_value){
					newbon->MitigateMeleeRune[0] = effect_value;
					newbon->MitigateMeleeRune[1] = buffslot;
				}
				break;
			}

			case SE_MitigateSpellDamage:
			{
				if (newbon->MitigateSpellRune[0] < effect_value){
					newbon->MitigateSpellRune[0] = effect_value;
					newbon->MitigateSpellRune[1] = buffslot;
				}
				break;
			}

			case SE_ManaAbsorbPercentDamage:
			{
				if (newbon->ManaAbsorbPercentDamage[0] < effect_value){
					newbon->ManaAbsorbPercentDamage[0] = effect_value;
					newbon->ManaAbsorbPercentDamage[1] = buffslot;
				}
				break;
			}

			case SE_ShieldBlock:
				newbon->ShieldBlock += effect_value;
				break;

			case SE_BlockBehind:
				newbon->BlockBehind += effect_value;
				break;

			case SE_Fear:
				newbon->IsFeared = true;  
				break;

			//AA bonuses - implemented broadly into spell/item effects
			case SE_FrontalStunResist:
				newbon->FrontalStunResist += effect_value;
				break;

			case SE_ImprovedBindWound:
				newbon->BindWound += effect_value;
				break;

			case SE_MaxBindWound:
				newbon->MaxBindWound += effect_value;
				break;

			case SE_BaseMovementSpeed:
				newbon->BaseMovementSpeed += effect_value; 
				break;

			case SE_IncreaseRunSpeedCap:
				newbon->IncreaseRunSpeedCap += effect_value;
				break;

			case SE_DoubleSpecialAttack:
				newbon->DoubleSpecialAttack += effect_value;
				break;
			
			case SE_TripleBackstab:
				newbon->TripleBackstab += effect_value;
				break;
			
			case SE_FrontalBackstabMinDmg:
				newbon->FrontalBackstabMinDmg = true;
				break;
		
			case SE_FrontalBackstabChance:
				newbon->FrontalBackstabChance += effect_value;
				break;
						
			case SE_ConsumeProjectile:
				newbon->ConsumeProjectile += effect_value;
				break;

            case SE_ForageAdditionalItems:
                newbon->ForageAdditionalItems += effect_value;
                break;

			case SE_ArcheryDamageModifier:
				newbon->ArcheryDamageModifier += effect_value;
				break;

			case SE_SecondaryDmgInc:
				newbon->SecondaryDmgInc = true;
				break;

			case SE_StrikeThrough2:
				newbon->StrikeThrough += effect_value;	
				break;

			case SE_GiveDoubleAttack:
				newbon->GiveDoubleAttack += effect_value;
				break;
			
			case SE_PetCriticalHit:
				newbon->PetCriticalHit += effect_value;
				break;

			case SE_CombatStability:
				newbon->CombatStability += effect_value;
				break;

			case SE_PetAvoidance:
				newbon->PetAvoidance += effect_value;
				break;

			case SE_Ambidexterity:
				newbon->Ambidexterity += effect_value;
				break;

			case SE_PetMaxHP:
				newbon->PetMaxHP += effect_value;
				break;

			case SE_PetFlurry:
				newbon->PetFlurry += effect_value;
				break;
			
			case SE_GivePetGroupTarget:
				newbon->GivePetGroupTarget = true;
				break;

			case SE_RootBreakChance:
				newbon->RootBreakChance += effect_value;
				break;
			
			case SE_ChannelChanceItems:
				newbon->ChannelChanceItems += effect_value;
				break;
			
			case SE_ChannelChanceSpells:
				newbon->ChannelChanceSpells += effect_value;
				break;
			
			case SE_UnfailingDivinity:
				newbon->UnfailingDivinity += effect_value;
				break;
			

			case SE_ItemHPRegenCapIncrease:
				newbon->ItemHPRegenCap += effect_value;
				break;
			
			case SE_OffhandRiposteFail:
				newbon->OffhandRiposteFail += effect_value;
				break;

			case SE_ItemAttackCapIncrease:
				newbon->ItemATKCap += effect_value;
				break;
			
			case SE_TwoHandBluntBlock:
				newbon->TwoHandBluntBlock += effect_value;
				break;

			case SE_SpellEffectResistChance:
			{
				for(int e = 0; e < MAX_RESISTABLE_EFFECTS*2; e+=2)
				{
					if(!newbon->SEResist[e] && 
						((newbon->SEResist[e] = spells[spell_id].base2[i]) && (newbon->SEResist[e+1] < effect_value)) ){
						newbon->SEResist[e] = spells[spell_id].base2[i];
						newbon->SEResist[e+1] = effect_value;
						break;
					}
				}
				break;
			}

			case SE_MasteryofPast:
			{
				if(newbon->MasteryofPast < effect_value)
					newbon->MasteryofPast = effect_value;
				break;
			}

			case SE_GiveDoubleRiposte:
			{
				//Only allow for regular double riposte chance.
				if(newbon->GiveDoubleRiposte[spells[spell_id].base2[i]] == 0){
					if(newbon->GiveDoubleRiposte[0] < effect_value)
						newbon->GiveDoubleRiposte[0] = effect_value;
				}
				break;
			}

			case SE_SlayUndead:
			{
				if(newbon->SlayUndead[1] < effect_value)
					newbon->SlayUndead[0] = effect_value; // Rate
					newbon->SlayUndead[1] = spells[spell_id].base2[i]; // Damage Modifier
				break;
			}


		}
	}
}

void NPC::CalcItemBonuses(StatBonuses *newbon)
{
	if(newbon){

		for(int i = 0; i < MAX_WORN_INVENTORY; i++){
			const Item_Struct *cur = database.GetItem(equipment[i]);
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
				if (cur->Worn.Effect>0 && (cur->Worn.Type == ET_WornEffect)) { // latent effects
					ApplySpellsBonuses(cur->Worn.Effect, cur->Worn.Level, newbon);
				}
				if (cur->Haste > newbon->haste)
					newbon->haste = cur->Haste;
			}
		}
	
	}
}

void Client::CalcItemScale(bool login)
{
	bool changed = false;

	if(CalcItemScale(0, 21, login))
		changed = true;

	if(CalcItemScale(22, 30, login))
		changed = true;

	if(CalcItemScale(251, 341, login))
		changed = true;

	if(CalcItemScale(400, 405, login))
		changed = true;

	//Power Source Slot
	if (GetClientVersion() >= EQClientSoF)
	{
		if(CalcItemScale(9999, 10000, login))
			changed = true;
	}

	if(changed)
	{
		CalcBonuses();
	}
}

bool Client::CalcItemScale(uint32 slot_x, uint32 slot_y, bool login)
{
	bool changed = false;
	int i;
	for (i = slot_x; i < slot_y; i++) {
		const ItemInst* inst = m_inv[i];
		if(inst == 0)
			continue;
		
		bool update_slot = false;
		if(inst->IsScaling())
		{
			EvoItemInst* e_inst = (EvoItemInst*)inst;
			uint16 oldexp = e_inst->GetExp();

			if(login) {
                parse->EventItem(EVENT_ITEM_ENTERZONE, this, e_inst, e_inst->GetID(), 0);
            }
            parse->EventItem(EVENT_SCALE_CALC, this, e_inst, e_inst->GetID(), 0);

			if (e_inst->GetExp() != oldexp) {	// if the scaling factor changed, rescale the item and update the client
				e_inst->ScaleItem();
				changed = true;
				update_slot = true;
			}
		}

		//iterate all augments
		for(int x = 0; x < MAX_AUGMENT_SLOTS; ++x) 
		{
			ItemInst * a_inst = inst->GetAugment(x);
			if(!a_inst)
				continue;

			if(a_inst->IsScaling())
			{
				EvoItemInst* e_inst = (EvoItemInst*)a_inst;
				uint16 oldexp = e_inst->GetExp();

                if(login) {
                    parse->EventItem(EVENT_ITEM_ENTERZONE, this, e_inst, e_inst->GetID(), 0);
                }
                parse->EventItem(EVENT_SCALE_CALC, this, e_inst, e_inst->GetID(), 0);

				if (e_inst->GetExp() != oldexp) 
				{
					e_inst->ScaleItem();
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
		case SE_SpellVulnerability:
			return focusSpellVulnerability;
		case SE_BlockNextSpellFocus:
			return focusBlockNextSpell;
		case SE_Twincast:
			return focusTwincast;
		case SE_SympatheticProc:
			return focusSympatheticProc;
		case SE_SpellDamage:
			return focusSpellDamage;
		case SE_FF_Damage_Amount:
			return focusFF_Damage_Amount;
		case SE_ImprovedDamage2:
			return focusImprovedDamage2;
		case SE_Empathy:
			return focusAdditionalDamage;
		case SE_HealRate2:
			return focusHealRate;
		case SE_IncreaseSpellPower:
			return focusSpellEffectiveness;
		case SE_IncreaseNumHits:
			return focusIncreaseNumHits;
		case SE_CriticalHealRate:
			return focusCriticalHealRate;
		case SE_AdditionalHeal2:
			return focusAdditionalHeal2;
		case SE_AdditionalHeal:
			return focusAdditionalHeal;
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
					spellbonuses.AggroRange = effect_value;
					aabonuses.AggroRange = effect_value;
					itembonuses.AggroRange = effect_value;
					break;

				case SE_Harmony:
					spellbonuses.AssistRange = effect_value;
					aabonuses.AssistRange = effect_value;
					itembonuses.AssistRange = effect_value;
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

				case SE_SingingSkill:
					spellbonuses.singingMod = effect_value;
					itembonuses.singingMod = effect_value;
					aabonuses.singingMod = effect_value;
					break;
				
				case SE_ChangeAggro:
					spellbonuses.hatemod = effect_value;
					itembonuses.hatemod = effect_value;
					aabonuses.hatemod = effect_value;
					break;
				
				case SE_MeleeMitigation:
					spellbonuses.MeleeMitigation = effect_value;
					itembonuses.MeleeMitigation = effect_value;
					aabonuses.MeleeMitigation = effect_value;
					break;

				case SE_CriticalHitChance:
				{
					for(int e = 0; e < HIGHEST_SKILL+1; e++)
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
					spellbonuses.AvoidMeleeChance = effect_value;
					aabonuses.AvoidMeleeChance = effect_value;
					itembonuses.AvoidMeleeChance = effect_value;
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
					for(int e = 0; e < HIGHEST_SKILL+1; e++)
					{
						spellbonuses.HitChanceEffect[e] = effect_value;
						aabonuses.HitChanceEffect[e] = effect_value;
						itembonuses.HitChanceEffect[e] = effect_value;
					}
					break;
				}
					
				case SE_DamageModifier:
				{
					for(int e = 0; e < HIGHEST_SKILL+1; e++)
					{
						spellbonuses.DamageModifier[e] = effect_value;
						aabonuses.DamageModifier[e] = effect_value;
						itembonuses.DamageModifier[e] = effect_value;
					}
					break;
				}
					
				case SE_MinDamageModifier:
				{
					for(int e = 0; e < HIGHEST_SKILL+1; e++)
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
					spellbonuses.ProcChance = effect_value;
					aabonuses.ProcChance = effect_value;
					itembonuses.ProcChance = effect_value;
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
					spellbonuses.Accuracy[HIGHEST_SKILL+1] = effect_value;
					itembonuses.Accuracy[HIGHEST_SKILL+1] = effect_value;

						for(int e = 0; e < HIGHEST_SKILL+1; e++)
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
					for(int e = 0; e < HIGHEST_SKILL+1; e++)
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

				case SE_CriticalHealChance2:
				case SE_CriticalHealChance:
					spellbonuses.CriticalHealChance = effect_value;
					aabonuses.CriticalHealChance = effect_value;
					itembonuses.CriticalHealChance = effect_value;
					break;
				
				case SE_CriticalHealOverTime2:
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

				case SE_SpellOnKill:
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
					for(int e = 0; e < HIGHEST_SKILL+1; e++)
					{
						spellbonuses.CritDmgMob[e] = effect_value;
						aabonuses.CritDmgMob[e] = effect_value;
						itembonuses.CritDmgMob[e] = effect_value;
					}
					break;
				}

				case SE_SkillDamageAmount:
				{
					for(int e = 0; e < HIGHEST_SKILL+1; e++)
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
					for(int e = 0; e < HIGHEST_SKILL+1; e++)
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

				case SE_MitigateSpellDamage:
					spellbonuses.MitigateSpellRune[0] = effect_value;
					spellbonuses.MitigateSpellRune[1] = -1;
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

			}
		}
	}
}
