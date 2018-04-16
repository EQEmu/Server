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

#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"
#include "../common/spdat.h"
#include "../common/misc_functions.h"

#include "client.h"
#include "entity.h"
#include "mob.h"

#include "string_ids.h"
#include "worldserver.h"
#include "zonedb.h"
#include "position.h"

float Mob::GetActSpellRange(uint16 spell_id, float range, bool IsBard)
{
	float extrange = 100;

	extrange += GetFocusEffect(focusRange, spell_id);

	return (range * extrange) / 100;
}

int32 Mob::GetActSpellDamage(uint16 spell_id, int32 value, Mob* target) {

	if (spells[spell_id].targettype == ST_Self)
		return value;

	if (IsNPC())
		value += value*CastToNPC()->GetSpellFocusDMG()/100;

	bool Critical = false;
	int32 value_BaseEffect = 0;
	int chance = 0;

	value_BaseEffect = value + (value*GetFocusEffect(focusFcBaseEffects, spell_id)/100);

	// Need to scale HT damage differently after level 40! It no longer scales by the constant value in the spell file. It scales differently, instead of 10 more damage per level, it does 30 more damage per level. So we multiply the level minus 40 times 20 if they are over level 40.
	if ((spell_id == SPELL_HARM_TOUCH || spell_id == SPELL_HARM_TOUCH2 || spell_id == SPELL_IMP_HARM_TOUCH ) && GetLevel() > 40)
		value -= (GetLevel() - 40) * 20;

	//This adds the extra damage from the AA Unholy Touch, 450 per level to the AA Improved Harm TOuch.
	if (spell_id == SPELL_IMP_HARM_TOUCH && IsClient()) //Improved Harm Touch
		value -= GetAA(aaUnholyTouch) * 450; //Unholy Touch

		chance = RuleI(Spells, BaseCritChance); //Wizard base critical chance is 2% (Does not scale with level)
		chance += itembonuses.CriticalSpellChance + spellbonuses.CriticalSpellChance + aabonuses.CriticalSpellChance;
		chance += itembonuses.FrenziedDevastation + spellbonuses.FrenziedDevastation + aabonuses.FrenziedDevastation;

	//Crtical Hit Calculation pathway
	if (chance > 0 || (IsClient() && GetClass() == WIZARD && GetLevel() >= RuleI(Spells, WizCritLevel))) {

		 int32 ratio = RuleI(Spells, BaseCritRatio); //Critical modifier is applied from spell effects only. Keep at 100 for live like criticals.

		//Improved Harm Touch is a guaranteed crit if you have at least one level of SCF.
		if (spell_id == SPELL_IMP_HARM_TOUCH && IsClient() && (GetAA(aaSpellCastingFury) > 0) && (GetAA(aaUnholyTouch) > 0))
			 chance = 100;

		if (spells[spell_id].override_crit_chance > 0 && chance > spells[spell_id].override_crit_chance)
			chance = spells[spell_id].override_crit_chance;

		if (zone->random.Roll(chance)) {
			Critical = true;
			ratio += itembonuses.SpellCritDmgIncrease + spellbonuses.SpellCritDmgIncrease + aabonuses.SpellCritDmgIncrease;
			ratio += itembonuses.SpellCritDmgIncNoStack + spellbonuses.SpellCritDmgIncNoStack + aabonuses.SpellCritDmgIncNoStack;
		}

		else if ((IsClient() && GetClass() == WIZARD) || (IsMerc() && GetClass() == CASTERDPS)) {
			if ((GetLevel() >= RuleI(Spells, WizCritLevel)) && zone->random.Roll(RuleI(Spells, WizCritChance))){
				//Wizard innate critical chance is calculated seperately from spell effect and is not a set ratio. (20-70 is parse confirmed)
				ratio += zone->random.Int(20,70);
				Critical = true;
			}
		}

		if (IsClient() && GetClass() == WIZARD)
			ratio += RuleI(Spells, WizCritRatio); //Default is zero

		if (Critical){

			value = value_BaseEffect*ratio/100;

			value += value_BaseEffect*GetFocusEffect(focusImprovedDamage, spell_id)/100;
			value += value_BaseEffect*GetFocusEffect(focusImprovedDamage2, spell_id)/100;

			value += int(value_BaseEffect*GetFocusEffect(focusFcDamagePctCrit, spell_id)/100)*ratio/100;

			if (target) {
				value += int(value_BaseEffect*target->GetVulnerability(this, spell_id, 0)/100)*ratio/100;
				value -= target->GetFcDamageAmtIncoming(this, spell_id);
			}

			value -= GetFocusEffect(focusFcDamageAmtCrit, spell_id)*ratio/100;

			value -= GetFocusEffect(focusFcDamageAmt, spell_id);
			value -= GetFocusEffect(focusFcDamageAmt2, spell_id);

			if (RuleB(Spells, IgnoreSpellDmgLvlRestriction) && !spells[spell_id].no_heal_damage_item_mod && itembonuses.SpellDmg)
				value -= GetExtraSpellAmt(spell_id, itembonuses.SpellDmg, value)*ratio / 100;

			else if(!spells[spell_id].no_heal_damage_item_mod && itembonuses.SpellDmg && spells[spell_id].classes[(GetClass() % 17) - 1] >= GetLevel() - 5)
				value -= GetExtraSpellAmt(spell_id, itembonuses.SpellDmg, value)*ratio/100;

			else if (IsNPC() && CastToNPC()->GetSpellScale())
				value = int(static_cast<float>(value) * CastToNPC()->GetSpellScale() / 100.0f);

			entity_list.MessageClose_StringID(this, true, 100, MT_SpellCrits,
					OTHER_CRIT_BLAST, GetName(), itoa(-value));

			if (IsClient())
				Message_StringID(MT_SpellCrits, YOU_CRIT_BLAST, itoa(-value));

			return value;
		}
	}
	//Non Crtical Hit Calculation pathway
	value = value_BaseEffect;

	value += value_BaseEffect*GetFocusEffect(focusImprovedDamage, spell_id)/100;
	value += value_BaseEffect*GetFocusEffect(focusImprovedDamage2, spell_id)/100;

	value += value_BaseEffect*GetFocusEffect(focusFcDamagePctCrit, spell_id)/100;

	if (target) {
		value += value_BaseEffect*target->GetVulnerability(this, spell_id, 0)/100;
		value -= target->GetFcDamageAmtIncoming(this, spell_id);
	}

	value -= GetFocusEffect(focusFcDamageAmtCrit, spell_id);

	value -= GetFocusEffect(focusFcDamageAmt, spell_id);
	value -= GetFocusEffect(focusFcDamageAmt2, spell_id);

	if (RuleB(Spells, IgnoreSpellDmgLvlRestriction) && !spells[spell_id].no_heal_damage_item_mod && itembonuses.SpellDmg)
		value -= GetExtraSpellAmt(spell_id, itembonuses.SpellDmg, value);

	else if (!spells[spell_id].no_heal_damage_item_mod && itembonuses.SpellDmg && spells[spell_id].classes[(GetClass() % 17) - 1] >= GetLevel() - 5)
		 value -= GetExtraSpellAmt(spell_id, itembonuses.SpellDmg, value);

	if (IsNPC() && CastToNPC()->GetSpellScale())
		value = int(static_cast<float>(value) * CastToNPC()->GetSpellScale() / 100.0f);

	return value;
}

int32 Mob::GetActDoTDamage(uint16 spell_id, int32 value, Mob* target) {

	if (target == nullptr)
		return value;

	if (IsNPC())
		value += value*CastToNPC()->GetSpellFocusDMG()/100;

	int32 value_BaseEffect = 0;
	int32 extra_dmg = 0;
	int16 chance = 0;
	chance += itembonuses.CriticalDoTChance + spellbonuses.CriticalDoTChance + aabonuses.CriticalDoTChance;

	if (spellbonuses.CriticalDotDecay)
		chance += GetDecayEffectValue(spell_id, SE_CriticalDotDecay);

	if (spells[spell_id].override_crit_chance > 0 && chance > spells[spell_id].override_crit_chance)
		chance = spells[spell_id].override_crit_chance;

	value_BaseEffect = value + (value*GetFocusEffect(focusFcBaseEffects, spell_id)/100);

	if (chance > 0 && (zone->random.Roll(chance))) {
		int32 ratio = 200;
		ratio += itembonuses.DotCritDmgIncrease + spellbonuses.DotCritDmgIncrease + aabonuses.DotCritDmgIncrease;
		value = value_BaseEffect*ratio/100;
		value += int(value_BaseEffect*GetFocusEffect(focusImprovedDamage, spell_id)/100)*ratio/100;
		value += int(value_BaseEffect*GetFocusEffect(focusImprovedDamage2, spell_id)/100)*ratio/100;
		value += int(value_BaseEffect*GetFocusEffect(focusFcDamagePctCrit, spell_id)/100)*ratio/100;
		value += int(value_BaseEffect*target->GetVulnerability(this, spell_id, 0)/100)*ratio/100;
		extra_dmg = target->GetFcDamageAmtIncoming(this, spell_id) +
					int(GetFocusEffect(focusFcDamageAmtCrit, spell_id)*ratio/100) +
					GetFocusEffect(focusFcDamageAmt, spell_id) +
					GetFocusEffect(focusFcDamageAmt2, spell_id);

		if (extra_dmg) {
			int duration = CalcBuffDuration(this, this, spell_id);
			if (duration > 0)
				extra_dmg /= duration;
		}

		value -= extra_dmg;
	}
	else {

		value = value_BaseEffect;
		value += value_BaseEffect*GetFocusEffect(focusImprovedDamage, spell_id)/100;
		value += value_BaseEffect*GetFocusEffect(focusImprovedDamage2, spell_id)/100;
		value += value_BaseEffect*GetFocusEffect(focusFcDamagePctCrit, spell_id)/100;
		value += value_BaseEffect*target->GetVulnerability(this, spell_id, 0)/100;
		extra_dmg = target->GetFcDamageAmtIncoming(this, spell_id) +
					GetFocusEffect(focusFcDamageAmtCrit, spell_id) +
					GetFocusEffect(focusFcDamageAmt, spell_id) +
					GetFocusEffect(focusFcDamageAmt2, spell_id);

		if (extra_dmg) {
			int duration = CalcBuffDuration(this, this, spell_id);
			if (duration > 0)
				extra_dmg /= duration;
		}

		value -= extra_dmg;
	}

	if (IsNPC() && CastToNPC()->GetSpellScale())
		value = int(static_cast<float>(value) * CastToNPC()->GetSpellScale() / 100.0f);

	return value;
}

int32 Mob::GetExtraSpellAmt(uint16 spell_id, int32 extra_spell_amt, int32 base_spell_dmg)
{
	if (RuleB(Spells, FlatItemExtraSpellAmt))
		return extra_spell_amt;

	int total_cast_time = 0;

	if (spells[spell_id].recast_time >= spells[spell_id].recovery_time)
			total_cast_time = spells[spell_id].recast_time + spells[spell_id].cast_time;
	else
		total_cast_time = spells[spell_id].recovery_time + spells[spell_id].cast_time;

	if (total_cast_time > 0 && total_cast_time <= 2500)
		extra_spell_amt = extra_spell_amt*25/100;
	 else if (total_cast_time > 2500 && total_cast_time < 7000)
		 extra_spell_amt = extra_spell_amt*(167*((total_cast_time - 1000)/1000)) / 1000;
	 else
		 extra_spell_amt = extra_spell_amt * total_cast_time / 7000;

		if(extra_spell_amt*2 < base_spell_dmg)
			return 0;

		return extra_spell_amt;
}

int32 Mob::GetActSpellHealing(uint16 spell_id, int32 value, Mob* target) {

	if (target == nullptr)
		target = this;

	if (IsNPC())
		value += value*CastToNPC()->GetSpellFocusHeal()/100;

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

		if (spells[spell_id].override_crit_chance > 0 && chance > spells[spell_id].override_crit_chance)
			chance = spells[spell_id].override_crit_chance;

		if(chance && (zone->random.Roll(chance))) {
			Critical = true;
			modifier = 2; //At present time no critical heal amount modifier SPA exists.
		}

		value *= modifier;
		value += GetFocusEffect(focusFcHealAmtCrit, spell_id) * modifier;
		value += GetFocusEffect(focusFcHealAmt, spell_id);
		value += target->GetFocusIncoming(focusFcHealAmtIncoming, SE_FcHealAmtIncoming, this, spell_id);

		if(!spells[spell_id].no_heal_damage_item_mod && itembonuses.HealAmt && spells[spell_id].classes[(GetClass()%17) - 1] >= GetLevel() - 5)
			value += GetExtraSpellAmt(spell_id, itembonuses.HealAmt, value) * modifier;

		value += value*target->GetHealRate(spell_id, this)/100;

		if (IsNPC() && CastToNPC()->GetHealScale())
			value = int(static_cast<float>(value) * CastToNPC()->GetHealScale() / 100.0f);

		if (Critical) {
			entity_list.MessageClose_StringID(this, true, 100, MT_SpellCrits,
					OTHER_CRIT_HEAL, GetName(), itoa(value));

			if (IsClient())
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

		if(chance && zone->random.Roll(chance))
			value *= 2;
	}

	if (IsNPC() && CastToNPC()->GetHealScale())
		value = int(static_cast<float>(value) * CastToNPC()->GetHealScale() / 100.0f);

	return value;
}


int32 Client::GetActSpellCost(uint16 spell_id, int32 cost)
{
	//FrenziedDevastation doubles mana cost of all DD spells
	int16 FrenziedDevastation = itembonuses.FrenziedDevastation + spellbonuses.FrenziedDevastation + aabonuses.FrenziedDevastation;

	if (FrenziedDevastation && IsPureNukeSpell(spell_id))
		cost *= 2;

	// Formula = Unknown exact, based off a random percent chance up to mana cost(after focuses) of the cast spell
	if(itembonuses.Clairvoyance && spells[spell_id].classes[(GetClass()%17) - 1] >= GetLevel() - 5)
	{
		int16 mana_back = itembonuses.Clairvoyance * zone->random.Int(1, 100) / 100;
		// Doesnt generate mana, so best case is a free spell
		if(mana_back > cost)
			mana_back = cost;

		cost -= mana_back;
	}

	int spec = GetSpecializeSkillValue(spell_id);
	int PercentManaReduction = 0;
	if (spec)
		PercentManaReduction = 1 + spec / 20; // there seems to be some non-obvious rounding here, let's truncate for now.

	int16 focus_redux = GetFocusEffect(focusManaCost, spell_id);
	PercentManaReduction += focus_redux;

	cost -= cost * PercentManaReduction / 100;

	// Gift of Mana - reduces spell cost to 1 mana
	if(focus_redux >= 100) {
		int buff_max = GetMaxTotalSlots();
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

int32 Mob::GetActSpellDuration(uint16 spell_id, int32 duration)
{
	int increase = 100;
	increase += GetFocusEffect(focusSpellDuration, spell_id);
	int tic_inc = 0;
	tic_inc = GetFocusEffect(focusSpellDurByTic, spell_id);

	float focused = ((duration * increase) / 100.0f) + tic_inc;
	int ifocused = static_cast<int>(focused);

	// 7.6 is rounded to 7, 8.6 is rounded to 9
	// 6 is 6, etc
	if (FCMP(focused, ifocused) || ifocused % 2) // equal or odd
		return ifocused;
	else // even and not equal round to odd
		return ifocused + 1;
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
	const EQEmu::ItemData *item = database.GetItem(itemid);
	if(item == nullptr) {
		Message(13, "Unable to find the tome you turned in!");
		Log(Logs::General, Logs::Error, "Unable to find turned in tome id %lu\n", (unsigned long)itemid);
		return(false);
	}

	if (!item->IsClassCommon() || item->ItemType != EQEmu::item::ItemTypeSpell) {
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
			database.SaveCharacterDisc(this->CharacterID(), r, spell_id);
			SendDisciplineUpdate();
			Message(0, "You have learned a new discipline!");
			return(true);
		}
	}
	Message(13, "You have learned too many disciplines and can learn no more.");
	return(false);
}

void Client::TrainDiscBySpellID(int32 spell_id)
{
	int i;
	for(i = 0; i < MAX_PP_DISCIPLINES; i++) {
		if(m_pp.disciplines.values[i] == 0) {
			m_pp.disciplines.values[i] = spell_id;
			database.SaveCharacterDisc(this->CharacterID(), i, spell_id);
			SendDisciplineUpdate();
			Message(15, "You have learned a new combat ability!");
			return;
		}
	}
}

int Client::GetDiscSlotBySpellID(int32 spellid)
{
	int i;

	for(i = 0; i < MAX_PP_DISCIPLINES; i++)
	{
		if(m_pp.disciplines.values[i] == spellid)
			return i;
	}
	
	return -1;
}

void Client::SendDisciplineUpdate() {
	EQApplicationPacket app(OP_DisciplineUpdate, sizeof(Disciplines_Struct));
	Disciplines_Struct *d = (Disciplines_Struct*)app.pBuffer;
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

	if(GetEndurance() < spell.EndurCost) {
		Message(11, "You are too fatigued to use this skill right now.");
		return(false);
	}

	// sneak attack discs require you to be hidden for 4 seconds before use
	if (spell.sneak && (!hidden || (hidden && (Timer::GetCurrentTime() - tmHidden) < 4000))) {
		Message_StringID(MT_SpellFailure, SNEAK_RESTRICT);
		return false;
	}

	// the client does this check before calling CastSpell, should prevent discs being eaten
	if (spell.buffdurationformula != 0 && spell.targettype == ST_Self && HasDiscBuff())
		return false;

	//Check the disc timer
	pTimerType DiscTimer = pTimerDisciplineReuseStart + spell.EndurTimerIndex;
	if(!p_timers.Expired(&database, DiscTimer, false)) { // lets not set the reuse timer in case CastSpell fails (or we would have to turn off the timer, but CastSpell will set it as well)
		/*char val1[20]={0};*/	//unused
		/*char val2[20]={0};*/	//unused
		uint32 remain = p_timers.GetRemainingTime(DiscTimer);
		//Message_StringID(0, DISCIPLINE_CANUSEIN, ConvertArray((remain)/60,val1), ConvertArray(remain%60,val2));
		Message(0, "You can use this discipline in %d minutes %d seconds.", ((remain)/60), (remain%60));
		return(false);
	}

	if(spell.recast_time > 0)
	{
		uint32 reduced_recast = spell.recast_time / 1000;
		auto focus = GetFocusEffect(focusReduceRecastTime, spell_id);
		// do stupid stuff because custom servers.
		// we really should be able to just do the -= focus but since custom servers could have shorter reuse timers
		// we have to make sure we don't underflow the uint32 ...
		// and yes, the focus effect can be used to increase the durations (spell 38944)
		if (focus > reduced_recast) {
			reduced_recast = 0;
			if (GetPTimers().Enabled((uint32)DiscTimer))
				GetPTimers().Clear(&database, (uint32)DiscTimer);
		} else {
			reduced_recast -= focus;
		}

		if (reduced_recast > 0)
			CastSpell(spell_id, target, EQEmu::CastingSlot::Discipline, -1, -1, 0, -1, (uint32)DiscTimer, reduced_recast);
		else{
			CastSpell(spell_id, target, EQEmu::CastingSlot::Discipline);
			return true;
		}

		SendDisciplineTimer(spells[spell_id].EndurTimerIndex, reduced_recast);
	}
	else
	{
		CastSpell(spell_id, target, EQEmu::CastingSlot::Discipline);
	}
	return(true);
}

void Client::SendDisciplineTimer(uint32 timer_id, uint32 duration)
{
	if (timer_id < MAX_DISCIPLINE_TIMERS)
	{
		auto outapp = new EQApplicationPacket(OP_DisciplineTimer, sizeof(DisciplineTimer_Struct));
		DisciplineTimer_Struct *dts = (DisciplineTimer_Struct *)outapp->pBuffer;
		dts->TimerID = timer_id;
		dts->Duration = duration;
		QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void EntityList::AETaunt(Client* taunter, float range, int32 bonus_hate)
{
	if (range == 0)
		range = 40;		//Live AE taunt range - Hardcoded.

	range = range * range;

	auto it = npc_list.begin();
	while (it != npc_list.end()) {
		NPC *them = it->second;
		float zdiff = taunter->GetZ() - them->GetZ();
		if (zdiff < 0)
			zdiff *= -1;
		if (zdiff < 10
				&& taunter->IsAttackAllowed(them)
				&& DistanceSquaredNoZ(taunter->GetPosition(), them->GetPosition()) <= range) {
			if (taunter->CheckLosFN(them)) {
				taunter->Taunt(them, true,0,true,bonus_hate);
			}
		}
		++it;
	}
}

// causes caster to hit every mob within dist range of center with
// spell_id.
// NPC spells will only affect other NPCs with compatible faction
void EntityList::AESpell(Mob *caster, Mob *center, uint16 spell_id, bool affect_caster, int16 resist_adjust, int *max_targets)
{
	Mob *curmob = nullptr;

	float dist = caster->GetAOERange(spell_id);
	float dist2 = dist * dist;
	float min_range2 = spells[spell_id].min_range * spells[spell_id].min_range;
	float dist_targ = 0;

	const auto &position = spells[spell_id].targettype == ST_Ring ? caster->GetTargetRingLocation() : static_cast<glm::vec3>(center->GetPosition());
	glm::vec2 min = { position.x - dist, position.y - dist };
	glm::vec2 max = { position.x + dist, position.y + dist };

	bool bad = IsDetrimentalSpell(spell_id);
	bool isnpc = caster->IsNPC();

	if (RuleB(Spells, OldRainTargets))
		max_targets = nullptr; // ignore it!

	// if we have a passed in value, use it, otherwise default to data
	// detrimental Target AEs have a default value of 4 for PCs and unlimited for NPCs
	int max_targets_allowed = 0; // unlimited
	if (max_targets) // rains pass this in since they need to preserve the count through waves
		max_targets_allowed = *max_targets;
	else if (spells[spell_id].aemaxtargets)
		max_targets_allowed = spells[spell_id].aemaxtargets;
	else if (IsTargetableAESpell(spell_id) && bad && !isnpc)
		max_targets_allowed = 4;

	int iCounter = 0;

	for (auto it = mob_list.begin(); it != mob_list.end(); ++it) {
		curmob = it->second;
		// test to fix possible cause of random zone crashes..external methods accessing client properties before they're initialized
		if (curmob->IsClient() && !curmob->CastToClient()->ClientFinishedLoading())
			continue;
		if (curmob == caster && !affect_caster)	//watch for caster too
			continue;
		if (spells[spell_id].targettype == ST_TargetAENoPlayersPets && curmob->IsPetOwnerClient())
			continue;
		if (spells[spell_id].targettype == ST_AreaClientOnly && !curmob->IsClient())
			continue;
		if (spells[spell_id].targettype == ST_AreaNPCOnly && !curmob->IsNPC())
			continue;
		// check PC/NPC only flag 1 = PCs, 2 = NPCs
		if (spells[spell_id].pcnpc_only_flag == 1 && !curmob->IsClient() && !curmob->IsMerc())
			continue;
		if (spells[spell_id].pcnpc_only_flag == 2 && (curmob->IsClient() || curmob->IsMerc()))
			continue;
		if (!IsWithinAxisAlignedBox(static_cast<glm::vec2>(curmob->GetPosition()), min, max))
			continue;

		dist_targ = DistanceSquared(curmob->GetPosition(), position);

		if (dist_targ > dist2)	//make sure they are in range
			continue;
		if (dist_targ < min_range2)	//make sure they are in range
			continue;
		if (isnpc && curmob->IsNPC() && spells[spell_id].targettype != ST_AreaNPCOnly) {	//check npc->npc casting
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
			if (center && !spells[spell_id].npc_no_los && !center->CheckLosFN(curmob))
				continue;
			if (!center && !spells[spell_id].npc_no_los && !caster->CheckLosFN(caster->GetTargetRingX(), caster->GetTargetRingY(), caster->GetTargetRingZ(), curmob->GetSize()))
				continue;
		} else { // check to stop casting beneficial ae buffs (to wit: bard songs) on enemies...
			// This does not check faction for beneficial AE buffs..only agro and attackable.
			// I've tested for spells that I can find without problem, but a faction-based
			// check may still be needed. Any changes here should also reflect in BardAEPulse()
			if (caster->IsAttackAllowed(curmob, true))
				continue;
			if (caster->CheckAggro(curmob))
				continue;
		}

		curmob->CalcSpellPowerDistanceMod(spell_id, dist_targ);
		caster->SpellOnTarget(spell_id, curmob, false, true, resist_adjust);

		if (max_targets_allowed) { // if we have a limit, increment count
			iCounter++;
			if (iCounter >= max_targets_allowed) // we done
				break;
		}
	}

	if (max_targets && max_targets_allowed)
		*max_targets = *max_targets - iCounter;
}

void EntityList::MassGroupBuff(Mob *caster, Mob *center, uint16 spell_id, bool affect_caster)
{
	Mob *curmob = nullptr;

	float dist = caster->GetAOERange(spell_id);
	float dist2 = dist * dist;

	bool bad = IsDetrimentalSpell(spell_id);

	for (auto it = mob_list.begin(); it != mob_list.end(); ++it) {
		curmob = it->second;
		if (curmob == center)	//do not affect center
			continue;
		if (curmob == caster && !affect_caster)	//watch for caster too
			continue;
		if (DistanceSquared(center->GetPosition(), curmob->GetPosition()) > dist2)	//make sure they are in range
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

// causes caster to hit every mob within dist range of center with
// a bard pulse of spell_id.
// NPC spells will only affect other NPCs with compatible faction
void EntityList::AEBardPulse(Mob *caster, Mob *center, uint16 spell_id, bool affect_caster)
{
	Mob *curmob = nullptr;

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
		if (DistanceSquared(center->GetPosition(), curmob->GetPosition()) > dist2)	//make sure they are in range
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

// Rampage and stuff for clients. Normal and Duration rampages
//NPCs handle it differently in Mob::Rampage
void EntityList::AEAttack(Mob *attacker, float dist, int Hand, int count, bool IsFromSpell) {
//Dook- Will need tweaking, currently no pets or players or horses
	Mob *curmob = nullptr;

	float dist2 = dist * dist;

	int hit = 0;

	for (auto it = mob_list.begin(); it != mob_list.end(); ++it) {
		curmob = it->second;
		if (curmob->IsNPC()
				&& curmob != attacker //this is not needed unless NPCs can use this
				&&(attacker->IsAttackAllowed(curmob))
				&& curmob->GetRace() != 216 && curmob->GetRace() != 472 /* dont attack horses */
				&& (DistanceSquared(curmob->GetPosition(), attacker->GetPosition()) <= dist2)
		) {
			if (!attacker->IsClient() || attacker->GetClass() == MONK || attacker->GetClass() == RANGER)
				attacker->Attack(curmob, Hand, false, false, IsFromSpell);
			else
				attacker->CastToClient()->DoAttackRounds(curmob, Hand, IsFromSpell);
			hit++;
			if (count != 0 && hit >= count)
				return;
		}
	}
}


