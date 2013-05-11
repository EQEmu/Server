/*  EQEMu:  Everquest Server Emulator
	Copyright (C) 2001-2003  EQEMu Development Team (http://eqemulator.net)

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


int32 Client::Additional_SpellDmg(uint16 spell_id, bool bufftick) 
{
	int32 spell_dmg = 0;
	spell_dmg  += GetFocusEffect(focusFF_Damage_Amount, spell_id);
	spell_dmg  += GetFocusEffect(focusSpellDamage, spell_id); 

	//For DOTs you need to apply the damage over the duration of the dot to each tick (this is how live did it)
	if (bufftick){
		int duration = CalcBuffDuration(this, this, spell_id);
		if (duration > 0)
			return spell_dmg /= duration;
		else
			return 0;
	}
	return spell_dmg;
}

//Scale all NPC spell Damage via $npc->SetSpellFocusDMG(value)
//Direct Damage is checked in Mob::SpellEffect [spell_effects.cpp]
//DoT Damage is checked in Mob::DoBuffTic [spell_effects.cpp] (This was added for npcs in that routine)
int32 NPC::GetActSpellDamage(uint16 spell_id, int32 value) {
	
	int32 modifier = 100;

	modifier += SpellFocusDMG;

	return (value * modifier / 100);
}

int32 Client::GetActSpellDamage(uint16 spell_id, int32 value) {
	// Important variables:
	// value: the actual damage after resists, passed from Mob::SpellEffect
	// modifier: modifier to damage (from spells & focus effects?)
	// ratio: % of the modifier to apply (from AAs & natural bonus?)
	// chance: critital chance %
	
	int32 modifier = 100;
	int16 spell_dmg = 0;


	//Dunno if this makes sense:
	if (spells[spell_id].resisttype > 0)
		modifier += GetFocusEffect((focusType)(0-spells[spell_id].resisttype), spell_id);
		
	
	int tt = spells[spell_id].targettype;
	if (tt == ST_UndeadAE || tt == ST_Undead || tt == ST_Summoned) {
		//undead/summoned spells
		modifier += GetFocusEffect(focusImprovedUndeadDamage, spell_id);
    } else {
    	//damage spells.
		modifier += GetFocusEffect(focusImprovedDamage, spell_id);
		modifier += GetFocusEffect(focusSpellEffectiveness, spell_id);
		modifier += GetFocusEffect(focusImprovedDamage2, spell_id);
	}
	
	// Need to scale HT damage differently after level 40! It no longer scales by the constant value in the spell file. It scales differently, instead of 10 more damage per level, it does 30 more damage per level. So we multiply the level minus 40 times 20 if they are over level 40.
	if ( spell_id == SPELL_HARM_TOUCH || spell_id == SPELL_HARM_TOUCH2 || spell_id == SPELL_IMP_HARM_TOUCH ) {
		if (this->GetLevel() > 40)
			value -= (this->GetLevel() - 40) * 20;  
	}

	//This adds the extra damage from the AA Unholy Touch, 450 per level to the AA Improved Harm TOuch.
	if (spell_id == SPELL_IMP_HARM_TOUCH) {  //Improved Harm Touch
			value -= GetAA(aaUnholyTouch) * 450; //Unholy Touch
	}
   
	// This adds the extra damage for the AA's Consumption of the Soul and Improved Consumption of the Soul, 200 per level to the AA Leech Curse for Shadowknights.
	if (spell_id == SPELL_LEECH_TOUCH) {   //Leech Touch
		value -= GetAA(aaConsumptionoftheSoul) * 200; //Consumption of the Soul
		value -= GetAA(aaImprovedConsumptionofSoul) * 200; //Improved Consumption of the Soul
	}

	//spell crits, dont make sense if cast on self.
	if(tt != ST_Self) {
		// item SpellDmg bonus
		// Formula = SpellDmg * (casttime + recastime) / 7; Cant trigger off spell less than 5 levels below and cant cause more dmg than the spell itself.
		if(this->itembonuses.SpellDmg && spells[spell_id].classes[(GetClass()%16) - 1] >= GetLevel() - 5) {
			spell_dmg = this->itembonuses.SpellDmg * (spells[spell_id].cast_time + spells[spell_id].recast_time) / 7000;
			if(spell_dmg > -value)
				spell_dmg = -value;
		}
		
		// Spell-based SpellDmg adds directly but it restricted by focuses.
		spell_dmg += Additional_SpellDmg(spell_id);

		int chance = RuleI(Spells, BaseCritChance);
		int32 ratio = RuleI(Spells, BaseCritRatio);

		chance += itembonuses.CriticalSpellChance + spellbonuses.CriticalSpellChance + aabonuses.CriticalSpellChance;
		ratio += itembonuses.SpellCritDmgIncrease + spellbonuses.SpellCritDmgIncrease + aabonuses.SpellCritDmgIncrease;

		if(GetClass() == WIZARD) {
			if (GetLevel() >= RuleI(Spells, WizCritLevel)) {
				chance += RuleI(Spells, WizCritChance);
				ratio += RuleI(Spells, WizCritRatio);
			}
			if(aabonuses.SpellCritDmgIncrease > 0) // wizards get an additional bonus
				ratio +=  aabonuses.SpellCritDmgIncrease * 1.5; //108%, 115%, 124%, close to Graffe's 207%, 215%, & 225%
		}
		
		//Improved Harm Touch is a guaranteed crit if you have at least one level of SCF.
		if (spell_id == SPELL_IMP_HARM_TOUCH) {
			if ( (GetAA(aaSpellCastingFury) > 0) && (GetAA(aaUnholyTouch) > 0) )
				chance = 100;
		} 

		/*
		//Handled in aa_effects will focus spells from 'spellgroup=99'. (SK life tap from buff procs)
		//If you are using an older spell file table (Pre SOF)...
		//Use SQL optional_EnableSoulAbrasionAA to update your spells table to properly use the effect.
		//If you do not want to update your table then you may want to enable this.
		if(tt == ST_Tap) {
			if(spells[spell_id].classes[SHADOWKNIGHT-1] >= 254 && spell_id != SPELL_LEECH_TOUCH){
				if(ratio < 100)	//chance increase and ratio are made up, not confirmed
					ratio = 100;

				switch (GetAA(aaSoulAbrasion))
				{
				case 1:
					modifier += 100;
					break;
				case 2:
					modifier += 200;
					break;
				case 3:
					modifier += 300;
					break;
				}
			}
		}
		*/
		
		if (chance > 0) {
			mlog(SPELLS__CRITS, "Attempting spell crit. Spell: %s (%d), Value: %d, Modifier: %d, Chance: %d, Ratio: %d", spells[spell_id].name, spell_id, value, modifier, chance, ratio);
			if(MakeRandomInt(0,100) <= chance) {
				modifier += modifier*ratio/100;
				spell_dmg *= 2;
				mlog(SPELLS__CRITS, "Spell crit successful. Final damage modifier: %d, Final Damage: %d", modifier, (value * modifier / 100) - spell_dmg);
				entity_list.MessageClose(this, false, 100, MT_SpellCrits, "%s delivers a critical blast! (%d)", GetName(), (-value * modifier / 100) + spell_dmg);	
			} else 
				mlog(SPELLS__CRITS, "Spell crit failed. Final Damage Modifier: %d, Final Damage: %d", modifier, (value * modifier / 100) - spell_dmg);
		}
	}
	
	return ((value * modifier / 100) - spell_dmg);
}

int32 Client::GetActDoTDamage(uint16 spell_id, int32 value) {

	int32 modifier = 100;
	int16 spell_dmg = 0;
	int16 critChance = 0;
	int32 ratio = 0;

	modifier += GetFocusEffect(focusImprovedDamage, spell_id);
	critChance += itembonuses.CriticalDoTChance + spellbonuses.CriticalDoTChance + aabonuses.CriticalDoTChance;
	ratio += itembonuses.DotCritDmgIncrease + spellbonuses.DotCritDmgIncrease + aabonuses.DotCritDmgIncrease;
	spell_dmg += Additional_SpellDmg(spell_id,true);
	
	// since DOTs are the Necromancer forte, give an innate bonus (Kayen: Is this a real bonus?)
	// however, no chance to crit unless they've trained atleast one level in the AA first
	if (GetClass() == NECROMANCER && critChance > 0)
		critChance += 5;
	
	if (critChance > 0){
		if (MakeRandomInt(0, 99) < critChance){
			modifier += modifier*ratio/100;
			return (((value*modifier/100)-spell_dmg)*2);
		}
	}
	
	return ((value*modifier/100)-spell_dmg);

}

//Scale all NPC spell healing via SetSpellFocusHeal(value)
int32 NPC::GetActSpellHealing(uint16 spell_id, int32 value) {

	int32 modifier = 100;
	modifier += SpellFocusHeal;
	
		// Check for buffs that affect the healrate of the target
		if(this->GetTarget())
		{
			value += value * GetHealRate(spell_id) / 100; 
		}

	return (value * modifier / 100);
}

int32 Client::Additional_Heal(uint16 spell_id) 
{
	int32 heal_amt = 0;

	heal_amt  += GetFocusEffect(focusAdditionalHeal, spell_id);
	heal_amt  += GetFocusEffect(focusAdditionalHeal2, spell_id);

	if (heal_amt){
		int duration = CalcBuffDuration(this, this, spell_id);
		if (duration > 0)
			return heal_amt /= duration;
	}
	
	return heal_amt;
}

int32 Client::GetActSpellHealing(uint16 spell_id, int32 value) {

	int32 modifier = 100;
	int16 heal_amt = 0;
	modifier += GetFocusEffect(focusImprovedHeal, spell_id);
	modifier += GetFocusEffect(focusSpellEffectiveness, spell_id);
	heal_amt += Additional_Heal(spell_id);
	int chance = 0;
	
	// Instant Heals					
	if(spells[spell_id].buffduration < 1) 
	{
		// Formula = HealAmt * (casttime + recastime) / 7; Cant trigger off spell less than 5 levels below and cant heal more than the spell itself.
		if(this->itembonuses.HealAmt && spells[spell_id].classes[(GetClass()%16) - 1] >= GetLevel() - 5) {
			heal_amt = this->itembonuses.HealAmt * (spells[spell_id].cast_time + spells[spell_id].recast_time) / 7000;
			if(heal_amt > value)
				heal_amt = value;
		}

		// Check for buffs that affect the healrate of the target and critical heal rate of target
		if(GetTarget()){
			value += value * GetHealRate(spell_id) / 100;
			chance += GetCriticalHealRate(spell_id);
		}
		
		//Live AA - Healing Gift, Theft of Life
		chance += itembonuses.CriticalHealChance + spellbonuses.CriticalHealChance + aabonuses.CriticalHealChance;
		
		if(MakeRandomInt(0,99) < chance) {
			entity_list.MessageClose(this, false, 100, MT_SpellCrits, "%s performs an exceptional heal! (%d)", GetName(), ((value * modifier / 50) + heal_amt*2));		
			return ((value * modifier / 50) + heal_amt*2);
		}
		else{
			return ((value * modifier / 100) + heal_amt);
		}		
	}
	// Hots
	else {
		chance += itembonuses.CriticalHealChance + spellbonuses.CriticalHealChance + aabonuses.CriticalHealChance;
		if(MakeRandomInt(0,99) < chance) 
			return ((value * modifier / 50) + heal_amt*2);
	}
	return ((value * modifier / 100) + heal_amt);
}

int32 Client::GetActSpellCost(uint16 spell_id, int32 cost)
{
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
	int increase = 100;
	increase += GetFocusEffect(focusSpellDuration, spell_id);
	int tic_inc = 0;
	tic_inc = GetFocusEffect(focusSpellDurByTic, spell_id);
	
	if(IsBeneficialSpell(spell_id))
	{
		switch(GetAA(aaSpellCastingReinforcement)) {
		case 1:
			increase += 5;
			break;
		case 2:
			increase += 15;
			break;
		case 3:
			increase += 30;
			if (GetAA(aaSpellCastingReinforcementMastery) == 1)
				increase += 20;
			break;
		}
	}

	if(IsMezSpell(spell_id)) {
		tic_inc += GetAA(aaMesmerizationMastery);
	}
	
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

void EntityList::AETaunt(Client* taunter, float range) {
	LinkedListIterator<NPC*> iterator(npc_list);
	
	if(range == 0) {
		range = 100;		//arbitrary default...
	}
	
	range = range * range;
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		NPC * them = iterator.GetData();
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
		iterator.Advance();
	}
}

// solar: causes caster to hit every mob within dist range of center with
// spell_id.
// NPC spells will only affect other NPCs with compatible faction
void EntityList::AESpell(Mob *caster, Mob *center, uint16 spell_id, bool affect_caster, int16 resist_adjust)
{
	LinkedListIterator<Mob*> iterator(mob_list);
	Mob *curmob;
	
	float dist = caster->GetAOERange(spell_id);
	float dist2 = dist * dist;
	
	bool bad = IsDetrimentalSpell(spell_id);
	bool isnpc = caster->IsNPC();
	const int MAX_TARGETS_ALLOWED = 4;
	int iCounter = 0;
	
	for(iterator.Reset(); iterator.MoreElements(); iterator.Advance())
	{
		curmob = iterator.GetData();
		if(curmob == center)	//do not affect center
			continue;
		if(curmob == caster && !affect_caster)	//watch for caster too
			continue;
		if(center->DistNoRoot(*curmob) > dist2)	//make sure they are in range
			continue;
		if(isnpc && curmob->IsNPC()) {	//check npc->npc casting
			FACTION_VALUE f = curmob->GetReverseFactionCon(caster);
			if(bad) {
				//affect mobs that are on our hate list, or
				//which have bad faction with us
				if( ! (caster->CheckAggro(curmob) || f == FACTION_THREATENLY || f == FACTION_SCOWLS) )
					continue;
			} else {
				//only affect mobs we would assist.
				if( ! (f <= FACTION_AMIABLE))
					continue;
			}
		}
		//finally, make sure they are within range
		if(bad) {
			if(!caster->IsAttackAllowed(curmob, true))
				continue;
			if(!center->CheckLosFN(curmob))
				continue;
		}
		else { // check to stop casting beneficial ae buffs (to wit: bard songs) on enemies...
			// This does not check faction for beneficial AE buffs..only agro and attackable.
			// I've tested for spells that I can find without problem, but a faction-based
			// check may still be needed. Any changes here should also reflect in BardAEPulse() -U
			if(caster->IsAttackAllowed(curmob, true))
				continue;
			if(caster->CheckAggro(curmob))
				continue;
		}

		//if we get here... cast the spell.
		if(IsTargetableAESpell(spell_id) && bad) 
		{
			if(iCounter < MAX_TARGETS_ALLOWED)
			{
				caster->SpellOnTarget(spell_id, curmob, false, true, resist_adjust);
			}
		}
		else
		{
			caster->SpellOnTarget(spell_id, curmob, false, true, resist_adjust);
		}

		if(!isnpc) //npcs are not target limited...
			iCounter++;
	}	
}

void EntityList::MassGroupBuff(Mob *caster, Mob *center, uint16 spell_id, bool affect_caster)
{
	LinkedListIterator<Mob*> iterator(mob_list);
	Mob *curmob;
	
	float dist = caster->GetAOERange(spell_id);
	float dist2 = dist * dist;
	
	bool bad = IsDetrimentalSpell(spell_id);

	for(iterator.Reset(); iterator.MoreElements(); iterator.Advance())
	{
		curmob = iterator.GetData();
		if(curmob == center)	//do not affect center
			continue;
		if(curmob == caster && !affect_caster)	//watch for caster too
			continue;
		if(center->DistNoRoot(*curmob) > dist2)	//make sure they are in range
			continue;

		//Only npcs mgb should hit are client pets...
		if(curmob->IsNPC())
		{
			Mob *owner = curmob->GetOwner();
			if(owner)
			{
				if(!owner->IsClient())
				{
					continue;
				}
			}
			else
			{
				continue;
			}
		}

		if(bad) 
		{
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
	LinkedListIterator<Mob*> iterator(mob_list);
	Mob *curmob;
	
	float dist = caster->GetAOERange(spell_id);
	float dist2 = dist * dist;
	
	bool bad = IsDetrimentalSpell(spell_id);
	bool isnpc = caster->IsNPC();
	
	for(iterator.Reset(); iterator.MoreElements(); iterator.Advance())
	{
		curmob = iterator.GetData();
		if(curmob == center)	//do not affect center
			continue;
		if(curmob == caster && !affect_caster)	//watch for caster too
			continue;
		if(center->DistNoRoot(*curmob) > dist2)	//make sure they are in range
			continue;
		if(isnpc && curmob->IsNPC()) {	//check npc->npc casting
			FACTION_VALUE f = curmob->GetReverseFactionCon(caster);
			if(bad) {
				//affect mobs that are on our hate list, or
				//which have bad faction with us
				if( ! (caster->CheckAggro(curmob) || f == FACTION_THREATENLY || f == FACTION_SCOWLS) )
					continue;
			} else {
				//only affect mobs we would assist.
				if( ! (f <= FACTION_AMIABLE))
					continue;
			}
		}
		//finally, make sure they are within range
		if(bad) {
			if(!center->CheckLosFN(curmob))
				continue;
		}
		else { // check to stop casting beneficial ae buffs (to wit: bard songs) on enemies...
			// See notes in AESpell() above for more info. 
			if(caster->IsAttackAllowed(curmob, true))
				continue;
			if(caster->CheckAggro(curmob))
				continue;
		}

		//if we get here... cast the spell.
		curmob->BardPulse(spell_id, caster);
	}
	if(caster->IsClient())
		caster->CastToClient()->CheckSongSkillIncrease(spell_id);
}

//Dook- Rampage and stuff for clients.
//NPCs handle it differently in Mob::Rampage
void EntityList::AEAttack(Mob *attacker, float dist, int Hand, int count, bool IsFromSpell) {
//Dook- Will need tweaking, currently no pets or players or horses 
	LinkedListIterator<Mob*> iterator(mob_list); 
	Mob *curmob; 
	
	float dist2 = dist * dist;
	
	int hit = 0;
	
	for(iterator.Reset(); iterator.MoreElements(); iterator.Advance()) { 
		curmob = iterator.GetData(); 
		if(curmob->IsNPC()
			&& curmob != attacker //this is not needed unless NPCs can use this
			&&(attacker->IsAttackAllowed(curmob))
			&& curmob->GetRace() != 216 && curmob->GetRace() != 472 /* dont attack horses */
			&& (curmob->DistNoRoot(*attacker) <= dist2)
		) {
			attacker->Attack(curmob, Hand, false, false, IsFromSpell); 
			hit++;
			if(count != 0 && hit >= count)
				return;
		}
	}    
} 


