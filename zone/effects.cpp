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
#include "../common/debug.h"
#include "masterentity.h"
#include "worldserver.h"
#include "zonedb.h"
#include "../common/spdat.h"
#include "../common/packet_dump.h"
#include "../common/packet_functions.h"
#include "petitions.h"
#include "../common/serverinfo.h"
#include "../common/ZoneNumbers.h"
#include "../common/moremath.h"
#include "../common/guilds.h"
#include "StringIDs.h"
#include "NpcAI.h"

float Client::GetActSpellRange(uint16 spell_id, float range, bool IsBard)
{
	float extrange = 100;

	extrange += GetFocusEffect(focusRange, spell_id);

	return (range * extrange) / 100;
}


int32 NPC::GetActSpellDamage(uint16 spell_id, int32 value,  Mob* target) {

	//Quest scale all NPC spell damage via $npc->SetSpellFocusDMG(value)
	//DoT Damage - Mob::DoBuffTic [spell_effects.cpp] / Direct Damage Mob::SpellEffect [spell_effects.cpp]

	int32 dmg = value;

	 if (target) {
		value += dmg*target->GetVulnerability(this, spell_id, 0)/100; 

		if (spells[spell_id].buffduration == 0)
			value -= target->GetFcDamageAmtIncoming(this, spell_id);
		else
			value -= target->GetFcDamageAmtIncoming(this, spell_id)/spells[spell_id].buffduration;
	 }
	  	 
	 value += dmg*SpellFocusDMG/100; 

	if (AI_HasSpellsEffects()){
		int16 chance = 0;
		int ratio = 0;

		if (spells[spell_id].buffduration == 0) {
		
			chance += spellbonuses.CriticalSpellChance + spellbonuses.FrenziedDevastation;
		
			if (chance && MakeRandomInt(1,100) <= chance){
			
				ratio += spellbonuses.SpellCritDmgIncrease + spellbonuses.SpellCritDmgIncNoStack;
				value += (value*ratio)/100;
				entity_list.MessageClose_StringID(this, true, 100, MT_SpellCrits, OTHER_CRIT_BLAST, GetCleanName(), itoa(-value));
			}
		}
		else {
			
			chance += spellbonuses.CriticalDoTChance;
		
			if (chance && MakeRandomInt(1,100) <= chance){
			
				ratio += spellbonuses.DotCritDmgIncrease;
				value += (value*ratio)/100;
			}
		}
	}

	return value;
}

int32 Client::GetActSpellDamage(uint16 spell_id, int32 value, Mob* target) {

	if (spells[spell_id].targettype == ST_Self)
		return value;

	bool Critical = false;
	int32 value_BaseEffect = 0;

	value_BaseEffect = value + (value*GetFocusEffect(focusFcBaseEffects, spell_id)/100);

	// Need to scale HT damage differently after level 40! It no longer scales by the constant value in the spell file. It scales differently, instead of 10 more damage per level, it does 30 more damage per level. So we multiply the level minus 40 times 20 if they are over level 40.
	if ( (spell_id == SPELL_HARM_TOUCH || spell_id == SPELL_HARM_TOUCH2 || spell_id == SPELL_IMP_HARM_TOUCH ) && GetLevel() > 40)
		value -= (GetLevel() - 40) * 20;

	//This adds the extra damage from the AA Unholy Touch, 450 per level to the AA Improved Harm TOuch.
	if (spell_id == SPELL_IMP_HARM_TOUCH) //Improved Harm Touch
		value -= GetAA(aaUnholyTouch) * 450; //Unholy Touch

	int chance = RuleI(Spells, BaseCritChance); //Wizard base critical chance is 2% (Does not scale with level)
		chance += itembonuses.CriticalSpellChance + spellbonuses.CriticalSpellChance + aabonuses.CriticalSpellChance;

		chance += itembonuses.FrenziedDevastation + spellbonuses.FrenziedDevastation + aabonuses.FrenziedDevastation;

	if (chance > 0 || (GetClass() == WIZARD && GetLevel() >= RuleI(Spells, WizCritLevel))) {

		 int32 ratio = RuleI(Spells, BaseCritRatio); //Critical modifier is applied from spell effects only. Keep at 100 for live like criticals.

		//Improved Harm Touch is a guaranteed crit if you have at least one level of SCF.
		 if (spell_id == SPELL_IMP_HARM_TOUCH && (GetAA(aaSpellCastingFury) > 0) && (GetAA(aaUnholyTouch) > 0))
			 chance = 100;

		 if (MakeRandomInt(1,100) <= chance){
			Critical = true;
			ratio += itembonuses.SpellCritDmgIncrease + spellbonuses.SpellCritDmgIncrease + aabonuses.SpellCritDmgIncrease;
			ratio += itembonuses.SpellCritDmgIncNoStack + spellbonuses.SpellCritDmgIncNoStack + aabonuses.SpellCritDmgIncNoStack;
		}

		else if (GetClass() == WIZARD && (GetLevel() >= RuleI(Spells, WizCritLevel)) && (MakeRandomInt(1,100) <= RuleI(Spells, WizCritChance))) {
			ratio += MakeRandomInt(20,70); //Wizard innate critical chance is calculated seperately from spell effect and is not a set ratio. (20-70 is parse confirmed)
			Critical = true;
		}

		ratio += RuleI(Spells, WizCritRatio); //Default is zero

		if (Critical){

			value = value_BaseEffect*ratio/100;

			value += value_BaseEffect*GetFocusEffect(focusImprovedDamage, spell_id)/100;

			value += int(value_BaseEffect*GetFocusEffect(focusFcDamagePctCrit, spell_id)/100)*ratio/100;

			if (target) {
				value += int(value_BaseEffect*target->GetVulnerability(this, spell_id, 0)/100)*ratio/100;
				value -= target->GetFcDamageAmtIncoming(this, spell_id);
			}

			value -= GetFocusEffect(focusFcDamageAmtCrit, spell_id)*ratio/100;

			value -= GetFocusEffect(focusFcDamageAmt, spell_id);

			if(itembonuses.SpellDmg && spells[spell_id].classes[(GetClass()%16) - 1] >= GetLevel() - 5)
				value -= GetExtraSpellAmt(spell_id, itembonuses.SpellDmg, value)*ratio/100;

			entity_list.MessageClose_StringID(this, true, 100, MT_SpellCrits,
					OTHER_CRIT_BLAST, GetName(), itoa(-value));
			Message_StringID(MT_SpellCrits, YOU_CRIT_BLAST, itoa(-value));

			return value;
		}
	}

	value = value_BaseEffect;

	value += value_BaseEffect*GetFocusEffect(focusImprovedDamage, spell_id)/100;

	value += value_BaseEffect*GetFocusEffect(focusFcDamagePctCrit, spell_id)/100;

	if (target) {
		value += value_BaseEffect*target->GetVulnerability(this, spell_id, 0)/100;
		value -= target->GetFcDamageAmtIncoming(this, spell_id);
	}

	value -= GetFocusEffect(focusFcDamageAmtCrit, spell_id);

	value -= GetFocusEffect(focusFcDamageAmt, spell_id);

	if(itembonuses.SpellDmg && spells[spell_id].classes[(GetClass()%16) - 1] >= GetLevel() - 5)
		 value -= GetExtraSpellAmt(spell_id, itembonuses.SpellDmg, value);

	return value;
}

int32 Client::GetActDoTDamage(uint16 spell_id, int32 value, Mob* target) {

	if (target == nullptr)
		return value;

	int32 value_BaseEffect = 0;
	int32 extra_dmg = 0;
	int16 chance = 0;
	chance += itembonuses.CriticalDoTChance + spellbonuses.CriticalDoTChance + aabonuses.CriticalDoTChance;

	if (spellbonuses.CriticalDotDecay)
			chance += GetDecayEffectValue(spell_id, SE_CriticalDotDecay);
	
	value_BaseEffect = value + (value*GetFocusEffect(focusFcBaseEffects, spell_id)/100);

	if (chance > 0 && (MakeRandomInt(1, 100) <= chance)) {
	
		int32 ratio = 200;
		ratio += itembonuses.DotCritDmgIncrease + spellbonuses.DotCritDmgIncrease + aabonuses.DotCritDmgIncrease;

		value = value_BaseEffect*ratio/100;  

		value += int(value_BaseEffect*GetFocusEffect(focusImprovedDamage, spell_id)/100)*ratio/100; 

		value += int(value_BaseEffect*GetFocusEffect(focusFcDamagePctCrit, spell_id)/100)*ratio/100;
	
		value += int(value_BaseEffect*target->GetVulnerability(this, spell_id, 0)/100)*ratio/100; 

		extra_dmg = target->GetFcDamageAmtIncoming(this, spell_id) + 
					int(GetFocusEffect(focusFcDamageAmtCrit, spell_id)*ratio/100) +
					GetFocusEffect(focusFcDamageAmt, spell_id);

		if (extra_dmg) {
			int duration = CalcBuffDuration(this, this, spell_id);
			if (duration > 0)
				extra_dmg /= duration; 
		}

		value -= extra_dmg;

		return value;
	}


	value = value_BaseEffect;

	value += value_BaseEffect*GetFocusEffect(focusImprovedDamage, spell_id)/100; 

	value += value_BaseEffect*GetFocusEffect(focusFcDamagePctCrit, spell_id)/100; 

	value += value_BaseEffect*target->GetVulnerability(this, spell_id, 0)/100; 
	
	extra_dmg = target->GetFcDamageAmtIncoming(this, spell_id) + 
				GetFocusEffect(focusFcDamageAmtCrit, spell_id) +
				GetFocusEffect(focusFcDamageAmt, spell_id); 

	if (extra_dmg) {
		int duration = CalcBuffDuration(this, this, spell_id);
		if (duration > 0)
			extra_dmg /= duration; 
	} 
	
	value -= extra_dmg;

	return value;
}

int32 Mob::GetExtraSpellAmt(uint16 spell_id, int32 extra_spell_amt, int32 base_spell_dmg)
{
	int total_cast_time = 0;

	if (spells[spell_id].recast_time >= spells[spell_id].recovery_time)
			total_cast_time = spells[spell_id].recast_time + spells[spell_id].cast_time;
	else
		total_cast_time = spells[spell_id].recovery_time + spells[spell_id].cast_time;

	if (total_cast_time > 0 && total_cast_time <= 2500)
		extra_spell_amt = extra_spell_amt*25/100; 
	 else if (total_cast_time > 2500 && total_cast_time < 7000) 
		 extra_spell_amt = extra_spell_amt*(0.167*((total_cast_time - 1000)/1000)); 
	 else 
		 extra_spell_amt = extra_spell_amt * total_cast_time / 7000; 

		if(extra_spell_amt*2 < base_spell_dmg)
			return 0;

		return extra_spell_amt;
}


int32 NPC::GetActSpellHealing(uint16 spell_id, int32 value, Mob* target) {

	//Scale all NPC spell healing via SetSpellFocusHeal(value)

	value += value*SpellFocusHeal/100; 

	 if (target) {
		value += target->GetFocusIncoming(focusFcHealAmtIncoming, SE_FcHealAmtIncoming, this, spell_id); 
		value += value*target->GetHealRate(spell_id, this)/100;
	 }

	 //Allow for critical heal chance if NPC is loading spell effect bonuses.
	 if (AI_HasSpellsEffects()){

		if(spells[spell_id].buffduration < 1) {

			if(spellbonuses.CriticalHealChance && (MakeRandomInt(0,99) < spellbonuses.CriticalHealChance)) {
				value = value*2;	 			
				entity_list.MessageClose_StringID(this, true, 100, MT_SpellCrits, OTHER_CRIT_HEAL, GetCleanName(), itoa(value));
			}
		}
		else if(spellbonuses.CriticalHealOverTime && (MakeRandomInt(0,99) < spellbonuses.CriticalHealOverTime)) {
				value = value*2;	 			
		}
	 }
	 
	return value;
}

int32 Client::GetActSpellHealing(uint16 spell_id, int32 value, Mob* target) {

	if (target == nullptr)
		target = this;

	int32 value_BaseEffect = 0;
	int16 chance = 0;
	int8 modifier = 1;
	bool Critical = false;

	value_BaseEffect = value + (value*GetFocusEffect(focusFcBaseEffects, spell_id)/100);

	value = value_BaseEffect;

	value += int(value_BaseEffect*GetFocusEffect(focusImprovedHeal, spell_id)/100);

	// Instant Heals
	if(spells[spell_id].buffduration < 1) {

		chance += itembonuses.CriticalHealChance + spellbonuses.CriticalHealChance + aabonuses.CriticalHealChance;

		chance += target->GetFocusIncoming(focusFcHealPctCritIncoming, SE_FcHealPctCritIncoming, this, spell_id);

		if (spellbonuses.CriticalHealDecay)
			chance += GetDecayEffectValue(spell_id, SE_CriticalHealDecay);

		if(chance && (MakeRandomInt(0,99) < chance)) {
			Critical = true;
			modifier = 2; //At present time no critical heal amount modifier SPA exists.
		}

		value *= modifier;
		value += GetFocusEffect(focusFcHealAmtCrit, spell_id) * modifier;
		value += GetFocusEffect(focusFcHealAmt, spell_id);
		value += target->GetFocusIncoming(focusFcHealAmtIncoming, SE_FcHealAmtIncoming, this, spell_id);

		if(itembonuses.HealAmt && spells[spell_id].classes[(GetClass()%16) - 1] >= GetLevel() - 5)
			value += GetExtraSpellAmt(spell_id, itembonuses.HealAmt, value) * modifier;

		value += value*target->GetHealRate(spell_id, this)/100;

		if (Critical) {
			entity_list.MessageClose_StringID(this, true, 100, MT_SpellCrits,
					OTHER_CRIT_HEAL, GetName(), itoa(value));
			Message_StringID(MT_SpellCrits, YOU_CRIT_HEAL, itoa(value));
		}

		return value;
	}

	//Heal over time spells. [Heal Rate and Additional Healing effects do not increase this value]
	else {

		chance = itembonuses.CriticalHealOverTime + spellbonuses.CriticalHealOverTime + aabonuses.CriticalHealOverTime;

		chance += target->GetFocusIncoming(focusFcHealPctCritIncoming, SE_FcHealPctCritIncoming, this, spell_id);

		if (spellbonuses.CriticalRegenDecay)
			chance += GetDecayEffectValue(spell_id, SE_CriticalRegenDecay);

		if(chance && (MakeRandomInt(0,99) < chance))
			return (value * 2);
	}

	return value;
}


int32 Client::GetActSpellCost(uint16 spell_id, int32 cost)
{
	//FrenziedDevastation doubles mana cost of all DD spells
	int16 FrenziedDevastation = itembonuses.FrenziedDevastation + spellbonuses.FrenziedDevastation + aabonuses.FrenziedDevastation;

	if (FrenziedDevastation && IsPureNukeSpell(spell_id))
		cost *= 2;	
	
	// Formula = Unknown exact, based off a random percent chance up to mana cost(after focuses) of the cast spell
	if(this->itembonuses.Clairvoyance && spells[spell_id].classes[(GetClass()%16) - 1] >= GetLevel() - 5)
	{
		int16 mana_back = this->itembonuses.Clairvoyance * MakeRandomInt(1, 100) / 100;
		// Doesnt generate mana, so best case is a free spell
		if(mana_back > cost)
			mana_back = cost;

		cost -= mana_back;
	}

	// This formula was derived from the following resource:
	// http://www.eqsummoners.com/eq1/specialization-library.html
	// WildcardX
	float PercentManaReduction = 0;
	float SpecializeSkill = GetSpecializeSkillValue(spell_id);
	int SuccessChance = MakeRandomInt(0, 100);

	float bonus = 1.0;
	switch(GetAA(aaSpellCastingMastery))
	{
	case 1:
		bonus += 0.05;
		break;
	case 2:
		bonus += 0.15;
		break;
	case 3:
		bonus += 0.30;
		break;
	}

	bonus += 0.05 * GetAA(aaAdvancedSpellCastingMastery);

	if(SuccessChance <= (SpecializeSkill * 0.3 * bonus))
	{
		PercentManaReduction = 1 + 0.05 * SpecializeSkill;
		switch(GetAA(aaSpellCastingMastery))
		{
		case 1:
			PercentManaReduction += 2.5;
			break;
		case 2:
			PercentManaReduction += 5.0;
			break;
		case 3:
			PercentManaReduction += 10.0;
			break;
		}

		switch(GetAA(aaAdvancedSpellCastingMastery))
		{
		case 1:
			PercentManaReduction += 2.5;
			break;
		case 2:
			PercentManaReduction += 5.0;
			break;
		case 3:
			PercentManaReduction += 10.0;
			break;
		}
	}

	int16 focus_redux = GetFocusEffect(focusManaCost, spell_id);

	if(focus_redux > 0)
	{
		PercentManaReduction += MakeRandomFloat(1, (double)focus_redux);
	}

	cost -= (cost * (PercentManaReduction / 100));

	// Gift of Mana - reduces spell cost to 1 mana
	if(focus_redux >= 100) {
		uint32 buff_max = GetMaxTotalSlots();
		for (int buffSlot = 0; buffSlot < buff_max; buffSlot++) {
			if (buffs[buffSlot].spellid == 0 || buffs[buffSlot].spellid >= SPDAT_RECORDS)
				continue;

			if(IsEffectInSpell(buffs[buffSlot].spellid, SE_ReduceManaCost)) {
				if(CalcFocusEffect(focusManaCost, buffs[buffSlot].spellid, spell_id) == 100)
					cost = 1;
			}
		}
	}

	if(cost < 0)
		cost = 0;

	return cost;
}

int32 Client::GetActSpellDuration(uint16 spell_id, int32 duration)
{
	if (spells[spell_id].not_extendable)
		return duration;

	int increase = 100;
	increase += GetFocusEffect(focusSpellDuration, spell_id);
	int tic_inc = 0;
	tic_inc = GetFocusEffect(focusSpellDurByTic, spell_id);

	// Only need this for clients, since the change was for bard songs, I assume we should keep non bard songs getting +1
	// However if its bard or not and is mez, charm or fear, we need to add 1 so that client is in sync
	if (!(IsShortDurationBuff(spell_id) && IsBardSong(spell_id)) ||
			IsFearSpell(spell_id) ||
			IsCharmSpell(spell_id) ||
			IsMezSpell(spell_id) ||
			IsBlindSpell(spell_id))
		tic_inc += 1;

	return (((duration * increase) / 100) + tic_inc);
}

int32 Client::GetActSpellCasttime(uint16 spell_id, int32 casttime)
{
	int32 cast_reducer = 0;
	cast_reducer += GetFocusEffect(focusSpellHaste, spell_id);

	//this function loops through the effects of spell_id many times
	//could easily be consolidated.

	if (GetLevel() >= 51 && casttime >= 3000 && !BeneficialSpell(spell_id)
		&& (GetClass() == SHADOWKNIGHT || GetClass() == RANGER
			|| GetClass() == PALADIN || GetClass() == BEASTLORD ))
		cast_reducer += (GetLevel()-50)*3;

	//LIVE AA SpellCastingDeftness, QuickBuff, QuickSummoning, QuickEvacuation, QuickDamage

	if (cast_reducer > RuleI(Spells, MaxCastTimeReduction))
		cast_reducer = RuleI(Spells, MaxCastTimeReduction);

	casttime = (casttime*(100 - cast_reducer)/100);

	return casttime;
}

bool Client::TrainDiscipline(uint32 itemid) {

	//get the item info
	const Item_Struct *item = database.GetItem(itemid);
	if(item == nullptr) {
		Message(13, "Unable to find the tome you turned in!");
		LogFile->write(EQEMuLog::Error, "Unable to find turned in tome id %lu\n", (unsigned long)itemid);
		return(false);
	}

	if(item->ItemClass != ItemClassCommon || item->ItemType != ItemTypeSpell) {
		Message(13, "Invalid item type, you cannot learn from this item.");
		//summon them the item back...
		SummonItem(itemid);
		return(false);
	}

	//Need a way to determine the difference between a spell and a tome
	//so they cant turn in a spell and get it as a discipline
	//this is kinda a hack:
	if(!(
		item->Name[0] == 'T' &&
		item->Name[1] == 'o' &&
		item->Name[2] == 'm' &&
		item->Name[3] == 'e' &&
		item->Name[4] == ' '
		) && !(
		item->Name[0] == 'S' &&
		item->Name[1] == 'k' &&
		item->Name[2] == 'i' &&
		item->Name[3] == 'l' &&
		item->Name[4] == 'l' &&
		item->Name[5] == ':' &&
		item->Name[6] == ' '
		)) {
		Message(13, "This item is not a tome.");
		//summon them the item back...
		SummonItem(itemid);
		return(false);
	}

	int myclass = GetClass();
	if(myclass == WIZARD || myclass == ENCHANTER || myclass == MAGICIAN || myclass == NECROMANCER) {
		Message(13, "Your class cannot learn from this tome.");
		//summon them the item back...
		SummonItem(itemid);
		return(false);
	}

	//make sure we can train this...
	//can we use the item?
	uint32 cbit = 1 << (myclass-1);
	if(!(item->Classes & cbit)) {
		Message(13, "Your class cannot learn from this tome.");
		//summon them the item back...
		SummonItem(itemid);
		return(false);
	}

	uint32 spell_id = item->Scroll.Effect;
	if(!IsValidSpell(spell_id)) {
		Message(13, "This tome contains invalid knowledge.");
		return(false);
	}

	//can we use the spell?
	const SPDat_Spell_Struct &spell = spells[spell_id];
	uint8 level_to_use = spell.classes[myclass - 1];
	if(level_to_use == 255) {
		Message(13, "Your class cannot learn from this tome.");
		//summon them the item back...
		SummonItem(itemid);
		return(false);
	}

	if(level_to_use > GetLevel()) {
		Message(13, "You must be at least level %d to learn this discipline.", level_to_use);
		//summon them the item back...
		SummonItem(itemid);
		return(false);
	}

	//add it to PP.
	int r;
	for(r = 0; r < MAX_PP_DISCIPLINES; r++) {
		if(m_pp.disciplines.values[r] == spell_id) {
			Message(13, "You already know this discipline.");
			//summon them the item back...
			SummonItem(itemid);
			return(false);
		} else if(m_pp.disciplines.values[r] == 0) {
			m_pp.disciplines.values[r] = spell_id;
			SendDisciplineUpdate();
			Message(0, "You have learned a new discipline!");
			return(true);
		}
	}
	Message(13, "You have learned too many disciplines and can learn no more.");
	return(false);
}

void Client::SendDisciplineUpdate() {
	//this dosent seem to work right now

	EQApplicationPacket app(OP_DisciplineUpdate, sizeof(Disciplines_Struct));
	Disciplines_Struct *d = (Disciplines_Struct*)app.pBuffer;
	//dunno why I dont just send the one from m_pp
	memcpy(d, &m_pp.disciplines, sizeof(m_pp.disciplines));

	QueuePacket(&app);
}

bool Client::UseDiscipline(uint32 spell_id, uint32 target) {
	// Dont let client waste a reuse timer if they can't use the disc
	if (IsStunned() || IsFeared() || IsMezzed() || IsAmnesiad() || IsPet())
	{
		return(false);
	}

	//make sure we have the spell...
	int r;
	for(r = 0; r < MAX_PP_DISCIPLINES; r++) {
		if(m_pp.disciplines.values[r] == spell_id)
			break;
	}
	if(r == MAX_PP_DISCIPLINES)
		return(false);	//not found.

	//Check the disc timer
	pTimerType DiscTimer = pTimerDisciplineReuseStart + spells[spell_id].EndurTimerIndex;
	if(!p_timers.Expired(&database, DiscTimer)) {
		/*char val1[20]={0};*/	//unused
		/*char val2[20]={0};*/	//unused
		uint32 remain = p_timers.GetRemainingTime(DiscTimer);
		//Message_StringID(0, DISCIPLINE_CANUSEIN, ConvertArray((remain)/60,val1), ConvertArray(remain%60,val2));
		Message(0, "You can use this discipline in %d minutes %d seconds.", ((remain)/60), (remain%60));
		return(false);
	}

	//make sure we can use it..
	if(!IsValidSpell(spell_id)) {
		Message(13, "This tome contains invalid knowledge.");
		return(false);
	}

	//can we use the spell?
	const SPDat_Spell_Struct &spell = spells[spell_id];
	uint8 level_to_use = spell.classes[GetClass() - 1];
	if(level_to_use == 255) {
		Message(13, "Your class cannot learn from this tome.");
		//should summon them a new one...
		return(false);
	}

	if(level_to_use > GetLevel()) {
		Message_StringID(13, DISC_LEVEL_USE_ERROR);
		//should summon them a new one...
		return(false);
	}

	if(GetEndurance() > spell.EndurCost) {
		SetEndurance(GetEndurance() - spell.EndurCost);
		TryTriggerOnValueAmount(false, false, true);
	} else {
		Message(11, "You are too fatigued to use this skill right now.");
		return(false);
	}

	if(spell.recast_time > 0)
	{
		uint32 reduced_recast = spell.recast_time / 1000;
		reduced_recast -= CastToClient()->GetFocusEffect(focusReduceRecastTime, spell_id);
		if(reduced_recast < 0)
			reduced_recast = 0;

		CastSpell(spell_id, target, DISCIPLINE_SPELL_SLOT, -1, -1, 0, -1, (uint32)DiscTimer, reduced_recast);
		if(spells[spell_id].EndurTimerIndex < MAX_DISCIPLINE_TIMERS)
		{
			EQApplicationPacket *outapp = new EQApplicationPacket(OP_DisciplineTimer, sizeof(DisciplineTimer_Struct));
			DisciplineTimer_Struct *dts = (DisciplineTimer_Struct *)outapp->pBuffer;
			dts->TimerID = spells[spell_id].EndurTimerIndex;
			dts->Duration = reduced_recast;
			QueuePacket(outapp);
			safe_delete(outapp);
		}
	}
	else
	{
		CastSpell(spell_id, target, DISCIPLINE_SPELL_SLOT);
	}
	return(true);
}

void EntityList::AETaunt(Client* taunter, float range)
{
	if (range == 0)
		range = 100;		//arbitrary default...

	range = range * range;

	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		NPC *them = it->second;
		float zdiff = taunter->GetZ() - them->GetZ();
		if (zdiff < 0)
			zdiff *= -1;
		if (zdiff < 10
				&& taunter->IsAttackAllowed(them)
				&& taunter->DistNoRootNoZ(*them) <= range) {
			if (taunter->CheckLosFN(them)) {
				taunter->Taunt(them, true);
			}
		}
		++it;
	}
}

// solar: causes caster to hit every mob within dist range of center with
// spell_id.
// NPC spells will only affect other NPCs with compatible faction
void EntityList::AESpell(Mob *caster, Mob *center, uint16 spell_id, bool affect_caster, int16 resist_adjust)
{
	Mob *curmob;

	float dist = caster->GetAOERange(spell_id);
	float dist2 = dist * dist;

	bool bad = IsDetrimentalSpell(spell_id);
	bool isnpc = caster->IsNPC();
	const int MAX_TARGETS_ALLOWED = 4;
	int iCounter = 0;

	for (auto it = mob_list.begin(); it != mob_list.end(); ++it) {
		curmob = it->second;
		// test to fix possible cause of random zone crashes..external methods accessing client properties before they're initialized
		if (curmob->IsClient() && !curmob->CastToClient()->ClientFinishedLoading())
			continue;
		if (curmob == center)	//do not affect center
			continue;
		if (curmob == caster && !affect_caster)	//watch for caster too
			continue;
		if (center->DistNoRoot(*curmob) > dist2)	//make sure they are in range
			continue;
		if (isnpc && curmob->IsNPC()) {	//check npc->npc casting
			FACTION_VALUE f = curmob->GetReverseFactionCon(caster);
			if (bad) {
				//affect mobs that are on our hate list, or
				//which have bad faction with us
				if (!(caster->CheckAggro(curmob) || f == FACTION_THREATENLY || f == FACTION_SCOWLS) )
					continue;
			} else {
				//only affect mobs we would assist.
				if (!(f <= FACTION_AMIABLE))
					continue;
			}
		}
		//finally, make sure they are within range
		if (bad) {
			if (!caster->IsAttackAllowed(curmob, true))
				continue;
			if (!center->CheckLosFN(curmob))
				continue;
		} else { // check to stop casting beneficial ae buffs (to wit: bard songs) on enemies...
			// This does not check faction for beneficial AE buffs..only agro and attackable.
			// I've tested for spells that I can find without problem, but a faction-based
			// check may still be needed. Any changes here should also reflect in BardAEPulse() -U
			if (caster->IsAttackAllowed(curmob, true))
				continue;
			if (caster->CheckAggro(curmob))
				continue;
		}

		//if we get here... cast the spell.
		if (IsTargetableAESpell(spell_id) && bad) {
			if (iCounter < MAX_TARGETS_ALLOWED) {
				caster->SpellOnTarget(spell_id, curmob, false, true, resist_adjust);
			}
		} else {
			caster->SpellOnTarget(spell_id, curmob, false, true, resist_adjust);
		}

		if (!isnpc) //npcs are not target limited...
			iCounter++;
	}
}

void EntityList::MassGroupBuff(Mob *caster, Mob *center, uint16 spell_id, bool affect_caster)
{
	Mob *curmob;

	float dist = caster->GetAOERange(spell_id);
	float dist2 = dist * dist;

	bool bad = IsDetrimentalSpell(spell_id);

	for (auto it = mob_list.begin(); it != mob_list.end(); ++it) {
		curmob = it->second;
		if (curmob == center)	//do not affect center
			continue;
		if (curmob == caster && !affect_caster)	//watch for caster too
			continue;
		if (center->DistNoRoot(*curmob) > dist2)	//make sure they are in range
			continue;

		//Only npcs mgb should hit are client pets...
		if (curmob->IsNPC()) {
			Mob *owner = curmob->GetOwner();
			if (owner) {
				if (!owner->IsClient()) {
					continue;
				}
			} else {
				continue;
			}
		}

		if (bad) {
			continue;
		}

		caster->SpellOnTarget(spell_id, curmob);
	}
}

// solar: causes caster to hit every mob within dist range of center with
// a bard pulse of spell_id.
// NPC spells will only affect other NPCs with compatible faction
void EntityList::AEBardPulse(Mob *caster, Mob *center, uint16 spell_id, bool affect_caster)
{
	Mob *curmob;

	float dist = caster->GetAOERange(spell_id);
	float dist2 = dist * dist;

	bool bad = IsDetrimentalSpell(spell_id);
	bool isnpc = caster->IsNPC();

	for (auto it = mob_list.begin(); it != mob_list.end(); ++it) {
		curmob = it->second;
		if (curmob == center)	//do not affect center
			continue;
		if (curmob == caster && !affect_caster)	//watch for caster too
			continue;
		if (center->DistNoRoot(*curmob) > dist2)	//make sure they are in range
			continue;
		if (isnpc && curmob->IsNPC()) {	//check npc->npc casting
			FACTION_VALUE f = curmob->GetReverseFactionCon(caster);
			if (bad) {
				//affect mobs that are on our hate list, or
				//which have bad faction with us
				if (!(caster->CheckAggro(curmob) || f == FACTION_THREATENLY || f == FACTION_SCOWLS) )
					continue;
			} else {
				//only affect mobs we would assist.
				if (!(f <= FACTION_AMIABLE))
					continue;
			}
		}
		//finally, make sure they are within range
		if (bad) {
			if (!center->CheckLosFN(curmob))
				continue;
		} else { // check to stop casting beneficial ae buffs (to wit: bard songs) on enemies...
			// See notes in AESpell() above for more info. 
			if (caster->IsAttackAllowed(curmob, true))
				continue;
			if (caster->CheckAggro(curmob))
				continue;
		}

		//if we get here... cast the spell.
		curmob->BardPulse(spell_id, caster);
	}
	if (caster->IsClient())
		caster->CastToClient()->CheckSongSkillIncrease(spell_id);
}

//Dook- Rampage and stuff for clients.
//NPCs handle it differently in Mob::Rampage
void EntityList::AEAttack(Mob *attacker, float dist, int Hand, int count, bool IsFromSpell) {
//Dook- Will need tweaking, currently no pets or players or horses
	Mob *curmob;

	float dist2 = dist * dist;

	int hit = 0;

	for (auto it = mob_list.begin(); it != mob_list.end(); ++it) {
		curmob = it->second;
		if (curmob->IsNPC()
				&& curmob != attacker //this is not needed unless NPCs can use this
				&&(attacker->IsAttackAllowed(curmob))
				&& curmob->GetRace() != 216 && curmob->GetRace() != 472 /* dont attack horses */
				&& (curmob->DistNoRoot(*attacker) <= dist2)
		) {
			attacker->Attack(curmob, Hand, false, false, IsFromSpell);
			hit++;
			if (count != 0 && hit >= count)
				return;
		}
	}
}


