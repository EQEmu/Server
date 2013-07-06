/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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


/*

	General outline of spell casting process

	1.
		a)	Client clicks a spell bar gem, ability, or item. client_process.cpp
		gets the op and calls CastSpell() with all the relevant info including
		cast time.

		b) NPC does CastSpell() from AI

	2.
		a)	CastSpell() determines there is a cast time and sets some state keeping
		flags to be used to check the progress of casting and finish it later.

		b)	CastSpell() sees there's no cast time, and calls CastedSpellFinished()
		Go to step 4.

	3.
		SpellProcess() notices that the spell casting timer which was set by
		CastSpell() is expired, and calls CastedSpellFinished()

	4.
		CastedSpellFinished() checks some timed spell specific things, like
		wether to interrupt or not, due to movement or melee. If successful
		SpellFinished() is called.

	5.
		SpellFinished() checks some things like LoS, reagents, target and
		figures out what's going to get hit by this spell based on its type.

	6.
		a)	Single target spell, SpellOnTarget() is called.

		b)	AE spell, Entity::AESpell() is called.

		c)	Group spell, Group::CastGroupSpell()/SpellOnTarget() is called as
		needed.

	7.
		SpellOnTarget() may or may not call SpellEffect() to cause effects to
		the target

	8.
		If this was timed, CastedSpellFinished() will restore the client's
		spell bar gems.


	Most user code should call CastSpell(), with a 0 casting time if needed,
	and not SpellFinished().

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
#include "../common/StringUtil.h"
#include <math.h>
#include <assert.h>

#ifndef WIN32
    #include <stdlib.h>
    #include "../common/unix.h"
#endif

#ifdef _GOTFRAGS
	#include "../common/packet_dump_file.h"
#endif

#include "StringIDs.h"
#include "QuestParserCollection.h"

extern Zone* zone;
extern volatile bool ZoneLoaded;
extern WorldServer worldserver;

// this is run constantly for every mob
void Mob::SpellProcess()
{
	// check the rapid recast prevention timer
	if(delaytimer == true && spellend_timer.Check())
	{
		spellend_timer.Disable();
		delaytimer = false;
		return;
	}

	// a timed spell is finished casting
	if (casting_spell_id != 0 && spellend_timer.Check())
	{
		spellend_timer.Disable();
		delaytimer = false;
		CastedSpellFinished(casting_spell_id, casting_spell_targetid, casting_spell_slot,
			casting_spell_mana, casting_spell_inventory_slot, casting_spell_resist_adjust);
	}

}

void NPC::SpellProcess()
{
	Mob::SpellProcess();

	if(GetSwarmInfo()){
		Mob *swp_o = GetSwarmInfo()->GetOwner();
		if(!swp_o)
		{
			Depop();
		}

		if(GetSwarmInfo()->duration->Check(false))
		{
			Depop();
		}

		Mob *targMob = entity_list.GetMob(GetSwarmInfo()->target);
		if(GetSwarmInfo()->target != 0)
		{
			if(!targMob || (targMob && targMob->IsCorpse()))
				Depop();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// functions related to begin/finish casting, fizzling etc

//
// only CastSpell and DoCastSpell should be setting casting_spell_id.
// basically casting_spell_id is only set when casting a triggered spell from
// the spell bar gems, an ability, or an item. note that it's actually set
// even if it's a 0 cast time, but then the spell is finished right after and
// it's unset. this is ok, since the 0 cast time spell is still a triggered
// one.
// the rule is you can cast one triggered (usually timed) spell at a time
// but things like SpellFinished() can run concurrent with a triggered cast
// to allow procs to work
bool Mob::CastSpell(uint16 spell_id, uint16 target_id, uint16 slot,
	int32 cast_time, int32 mana_cost, uint32* oSpellWillFinish, uint32 item_slot,
	uint32 timer, uint32 timer_duration, uint32 type, int16 *resist_adjust)
{
	_ZP(Mob_CastSpell);

	mlog(SPELLS__CASTING, "CastSpell called for spell %s (%d) on entity %d, slot %d, time %d, mana %d, from item slot %d",
		spells[spell_id].name, spell_id, target_id, slot, cast_time, mana_cost, (item_slot==0xFFFFFFFF)?999:item_slot);

	if(casting_spell_id == spell_id)
		ZeroCastingVars();

	if
	(
		!IsValidSpell(spell_id) ||
		casting_spell_id ||
		delaytimer ||
		spellend_timer.Enabled() ||
		IsStunned() ||
		IsFeared() ||
		IsMezzed() ||
		(IsSilenced() && !IsDiscipline(spell_id)) ||
		(IsAmnesiad() && IsDiscipline(spell_id))
	)
	{
		mlog(SPELLS__CASTING_ERR, "Spell casting canceled: not able to cast now. Valid? %d, casting %d, waiting? %d, spellend? %d, stunned? %d, feared? %d, mezed? %d, silenced? %d, amnesiad? %d",
			IsValidSpell(spell_id), casting_spell_id, delaytimer, spellend_timer.Enabled(), IsStunned(), IsFeared(), IsMezzed(), IsSilenced(), IsAmnesiad() );
		if(IsSilenced() && !IsDiscipline(spell_id))
			Message_StringID(13, SILENCED_STRING);
		if(IsAmnesiad() && IsDiscipline(spell_id))
			Message_StringID(13, MELEE_SILENCE);
		if(IsClient())
			CastToClient()->SendSpellBarEnable(spell_id);
		if(casting_spell_id && IsNPC())
			CastToNPC()->AI_Event_SpellCastFinished(false, casting_spell_slot);
		return(false);
	}

	if(IsDetrimentalSpell(spell_id) && !zone->CanDoCombat()){
		Message_StringID(13, SPELL_WOULDNT_HOLD);
		if(IsClient())
			CastToClient()->SendSpellBarEnable(spell_id);
		if(casting_spell_id && IsNPC())
			CastToNPC()->AI_Event_SpellCastFinished(false, casting_spell_slot);
		return(false);
	}

	//cannot cast under divine aura
	if(DivineAura()) {
		mlog(SPELLS__CASTING_ERR, "Spell casting canceled: cannot cast while Divine Aura is in effect.");
		InterruptSpell(173, 0x121, false);
		return(false);
	}

	if(IsClient() && GetTarget() && IsHarmonySpell(spell_id))
	{
		for(int i = 0; i < EFFECT_COUNT; i++) {
			// not important to check limit on SE_Lull as it doesnt have one and if the other components won't land, then SE_Lull wont either
			if (spells[spell_id].effectid[i] == SE_ChangeFrenzyRad || spells[spell_id].effectid[i] == SE_Harmony) {
				if((spells[spell_id].max[i] != 0 && GetTarget()->GetLevel() > spells[spell_id].max[i]) || GetTarget()->GetSpecialAbility(IMMUNE_PACIFY)) {
					InterruptSpell(CANNOT_AFFECT_NPC, 0x121, spell_id);
					return(false);
				}
			}
		}
	}

	// check for fizzle
	// note that CheckFizzle itself doesn't let NPCs fizzle,
	// but this code allows for it.
	if(slot < MAX_PP_MEMSPELL && !CheckFizzle(spell_id))
	{
		int fizzle_msg = IsBardSong(spell_id) ? MISS_NOTE : SPELL_FIZZLE;
		InterruptSpell(fizzle_msg, 0x121, spell_id);

		uint32 use_mana = ((spells[spell_id].mana) / 4);
		mlog(SPELLS__CASTING_ERR, "Spell casting canceled: fizzled. %d mana has been consumed", use_mana);

		// fizzle 1/4 the mana away
		SetMana(GetMana() - use_mana);
		return(false);
	}

	if (HasActiveSong() && IsBardSong(spell_id)) {
		mlog(SPELLS__BARDS, "Casting a new song while singing a song. Killing old song %d.", bardsong);
		//Note: this does NOT tell the client
		_StopSong();
	}

	//Added to prevent MQ2 exploitation of equipping normally-unequippable/clickable items with effects and clicking them for benefits.
	if(item_slot && IsClient() && ((slot == USE_ITEM_SPELL_SLOT) || (slot == POTION_BELT_SPELL_SLOT)))
	{
		ItemInst *itm = CastToClient()->GetInv().GetItem(item_slot);
		int bitmask = 1;
		bitmask = bitmask << (CastToClient()->GetClass() - 1);
		if( itm && itm->GetItem()->Classes != 65535 ) {
			if ((itm->GetItem()->Click.Type == ET_EquipClick) && !(itm->GetItem()->Classes & bitmask)) {
				if (CastToClient()->GetClientVersion() < EQClientSoF) {
					// They are casting a spell from an item that requires equipping but shouldn't let them equip it
					LogFile->write(EQEMuLog::Error, "HACKER: %s (account: %s) attempted to click an equip-only effect on item %s (id: %d) which they shouldn't be able to equip!",
						CastToClient()->GetCleanName(), CastToClient()->AccountName(), itm->GetItem()->Name, itm->GetItem()->ID);
					database.SetHackerFlag(CastToClient()->AccountName(), CastToClient()->GetCleanName(), "Clicking equip-only item with an invalid class");
				}
				else {
					Message_StringID(13, MUST_EQUIP_ITEM);
				}
				return(false);
			}
			if ((itm->GetItem()->Click.Type == ET_ClickEffect2) && !(itm->GetItem()->Classes & bitmask)) {
				if (CastToClient()->GetClientVersion() < EQClientSoF) {
					// They are casting a spell from an item that they don't meet the race/class requirements to cast
					LogFile->write(EQEMuLog::Error, "HACKER: %s (account: %s) attempted to click a race/class restricted effect on item %s (id: %d) which they shouldn't be able to click!",
						CastToClient()->GetCleanName(), CastToClient()->AccountName(), itm->GetItem()->Name, itm->GetItem()->ID);
					database.SetHackerFlag(CastToClient()->AccountName(), CastToClient()->GetCleanName(), "Clicking race/class restricted item with an invalid class");
				}
				else {
					Message_StringID(13, CANNOT_USE_ITEM);
				}
				return(false);
			}
		}
		if( itm && (itm->GetItem()->Click.Type == ET_EquipClick) && !(item_slot < 22 || item_slot == 9999) ){
			if (CastToClient()->GetClientVersion() < EQClientSoF) {
				// They are attempting to cast a must equip clicky without having it equipped
				LogFile->write(EQEMuLog::Error, "HACKER: %s (account: %s) attempted to click an equip-only effect on item %s (id: %d) without equiping it!", CastToClient()->GetCleanName(), CastToClient()->AccountName(), itm->GetItem()->Name, itm->GetItem()->ID);
				database.SetHackerFlag(CastToClient()->AccountName(), CastToClient()->GetCleanName(), "Clicking equip-only item without equiping it");
			}
			else {
				Message_StringID(13, MUST_EQUIP_ITEM);
			}
			return(false);
		}
	}

	if(IsClient()) {
		char temp[64];
		sprintf(temp, "%d", spell_id);
		parse->EventPlayer(EVENT_CAST_BEGIN, CastToClient(), temp, 0);
	} else if(IsNPC()) {
		char temp[64];
		sprintf(temp, "%d", spell_id);
		parse->EventNPC(EVENT_CAST_BEGIN, CastToNPC(), nullptr, temp, 0);
	}

	if(resist_adjust)
	{
		return(DoCastSpell(spell_id, target_id, slot, cast_time, mana_cost, oSpellWillFinish, item_slot, timer, timer_duration, type, *resist_adjust));
	}
	else
	{
		return(DoCastSpell(spell_id, target_id, slot, cast_time, mana_cost, oSpellWillFinish, item_slot, timer, timer_duration, type, spells[spell_id].ResistDiff));
	}
}

//
// the order of things here is intentional and important. make sure you
// understand the whole spell casting process and the flags that are passed
// around if you're gonna modify this
//
// this is the 2nd phase of CastSpell, broken up like this to make it easier
// to repeat a spell for bard songs
//
bool Mob::DoCastSpell(uint16 spell_id, uint16 target_id, uint16 slot,
					int32 cast_time, int32 mana_cost, uint32* oSpellWillFinish,
					uint32 item_slot, uint32 timer, uint32 timer_duration, uint32 type,
					int16 resist_adjust)
{
	_ZP(Mob_DoCastSpell);

	Mob* pMob = nullptr;
	int32 orgcasttime;
	EQApplicationPacket *outapp = nullptr;

	if(!IsValidSpell(spell_id)) {
		InterruptSpell();
		return(false);
	}

	const SPDat_Spell_Struct &spell = spells[spell_id];

	mlog(SPELLS__CASTING, "DoCastSpell called for spell %s (%d) on entity %d, slot %d, time %d, mana %d, from item %d",
		spell.name, spell_id, target_id, slot, cast_time, mana_cost, item_slot==0xFFFFFFFF?999:item_slot);


	casting_spell_id = spell_id;
	casting_spell_slot = slot;
	casting_spell_inventory_slot = item_slot;
	if(casting_spell_timer != 0xFFFFFFFF)
	{
		casting_spell_timer = timer;
		casting_spell_timer_duration = timer_duration;
	}
	casting_spell_type = type;

	SaveSpellLoc();
	mlog(SPELLS__CASTING, "Casting %d Started at (%.3f,%.3f,%.3f)", spell_id, spell_x, spell_y, spell_z);

	// if this spell doesn't require a target, or if it's an optional target
	// and a target wasn't provided, then it's us; unless TGB is on and this
	// is a TGB compatible spell.
	if((IsGroupSpell(spell_id) ||
		spell.targettype == ST_Self ||
		spell.targettype == ST_AECaster ||
		spell.targettype == ST_TargetOptional) && target_id == 0)
	{
		mlog(SPELLS__CASTING, "Spell %d auto-targeted the caster. Group? %d, target type %d", spell_id, IsGroupSpell(spell_id), spell.targettype);
		target_id = GetID();
	}

	if(cast_time <= -1) {
		// save the non-reduced cast time to use in the packet
		cast_time = orgcasttime = spell.cast_time;
		// if there's a cast time, check if they have a modifier for it
		if(cast_time) {
			cast_time = GetActSpellCasttime(spell_id, cast_time);
		}
	}
	else
		orgcasttime = cast_time;

	// we checked for spells not requiring targets above
	if(target_id == 0) {
		mlog(SPELLS__CASTING_ERR, "Spell Error: no target. spell=%d\n", GetName(), spell_id);
		if(IsClient()) {
			//clients produce messages... npcs should not for this case
			Message_StringID(13, SPELL_NEED_TAR);
			InterruptSpell();
		} else {
			InterruptSpell(0, 0, 0);	//the 0 args should cause no messages
		}
		return(false);
	}

	// ok now we know the target
	casting_spell_targetid = target_id;

	if (mana_cost == -1) {
		mana_cost = spell.mana;
		mana_cost = GetActSpellCost(spell_id, mana_cost);
	}

	if(IsClient() && CastToClient()->CheckAAEffect(aaEffectMassGroupBuff) && spells[spell_id].can_mgb)
		mana_cost *= 2;

	// mana is checked for clients on the frontend. we need to recheck it for NPCs though
	// fix: items dont need mana :-/
	// If you're at full mana, let it cast even if you dont have enough mana

	// we calculated this above, now enforce it
	if(mana_cost > 0 && slot != 10)
	{
		int my_curmana = GetMana();
		int my_maxmana = GetMaxMana();
		if(my_curmana < spell.mana)	// not enough mana
		{
			//this is a special case for NPCs with no mana...
			if(IsNPC() && my_curmana == my_maxmana)
			{
				mana_cost = 0;
			} else {
				mlog(SPELLS__CASTING_ERR, "Spell Error not enough mana spell=%d mymana=%d cost=%d\n", GetName(), spell_id, my_curmana, mana_cost);
				if(IsClient()) {
					//clients produce messages... npcs should not for this case
					Message_StringID(13, INSUFFICIENT_MANA);
					InterruptSpell();
				} else {
					InterruptSpell(0, 0, 0);	//the 0 args should cause no messages
				}
				return(false);
			}
		}
	}

	if(mana_cost > GetMana())
		mana_cost = GetMana();

	// we know our mana cost now
	casting_spell_mana = mana_cost;

	casting_spell_resist_adjust = resist_adjust;

	mlog(SPELLS__CASTING, "Spell %d: Casting time %d (orig %d), mana cost %d", orgcasttime, cast_time, mana_cost);

	// cast time is 0, just finish it right now and be done with it
	if(cast_time == 0) {
		CastedSpellFinished(spell_id, target_id, slot, mana_cost, item_slot, resist_adjust);
		return(true);
	}

	cast_time = mod_cast_time(cast_time);

	// ok we know it has a cast time so we can start the timer now
	spellend_timer.Start(cast_time);

	if (IsAIControlled())
	{
		SetRunAnimSpeed(0);
		if(this != pMob)
			this->FaceTarget(pMob);
	}

	// if we got here we didn't fizzle, and are starting our cast
	if (oSpellWillFinish)
		*oSpellWillFinish = Timer::GetCurrentTime() + cast_time + 100;

	// now tell the people in the area
	outapp = new EQApplicationPacket(OP_BeginCast,sizeof(BeginCast_Struct));
	BeginCast_Struct* begincast = (BeginCast_Struct*)outapp->pBuffer;
	begincast->caster_id = GetID();
	begincast->spell_id = spell_id;
	begincast->cast_time = orgcasttime; // client calculates reduced time by itself
	outapp->priority = 3;
	entity_list.QueueCloseClients(this, outapp, false, 200, 0, true); //IsClient() ? FILTER_PCSPELLS : FILTER_NPCSPELLS);
	safe_delete(outapp);
	outapp = nullptr;
	return(true);
}

uint16 Mob::GetSpecializeSkillValue(uint16 spell_id) const {
	switch(spells[spell_id].skill) {
	case ABJURE:
		return(GetSkill(SPECIALIZE_ABJURE));
	case ALTERATION:
		return(GetSkill(SPECIALIZE_ALTERATION));
	case CONJURATION:
		return(GetSkill(SPECIALIZE_CONJURATION));
	case DIVINATION:
		return(GetSkill(SPECIALIZE_DIVINATION));
	case EVOCATION:
		return(GetSkill(SPECIALIZE_EVOCATION));
	default:
		//wtf...
		break;
	}
	return(0);
}

void Client::CheckSpecializeIncrease(uint16 spell_id) {
	switch(spells[spell_id].skill) {
	case ABJURE:
		CheckIncreaseSkill(SPECIALIZE_ABJURE, nullptr);
		break;
	case ALTERATION:
		CheckIncreaseSkill(SPECIALIZE_ALTERATION, nullptr);
		break;
	case CONJURATION:
		CheckIncreaseSkill(SPECIALIZE_CONJURATION, nullptr);
		break;
	case DIVINATION:
		CheckIncreaseSkill(SPECIALIZE_DIVINATION, nullptr);
		break;
	case EVOCATION:
		CheckIncreaseSkill(SPECIALIZE_EVOCATION, nullptr);
		break;
	default:
		//wtf...
		break;
	}
}

void Client::CheckSongSkillIncrease(uint16 spell_id){
	switch(spells[spell_id].skill)
	{
	case SINGING:
		CheckIncreaseSkill(SINGING, nullptr, -15);
		break;
	case PERCUSSION_INSTRUMENTS:
		if(this->itembonuses.percussionMod > 0) {
			if(GetRawSkill(PERCUSSION_INSTRUMENTS) > 0)	// no skill increases if not trained in the instrument
				CheckIncreaseSkill(PERCUSSION_INSTRUMENTS, nullptr, -15);
			else
				Message_StringID(13,NO_INSTRUMENT_SKILL);	// tell the client that they need instrument training
		}
		else
			CheckIncreaseSkill(SINGING, nullptr, -15);
		break;
	case STRINGED_INSTRUMENTS:
		if(this->itembonuses.stringedMod > 0) {
			if(GetRawSkill(STRINGED_INSTRUMENTS) > 0)
				CheckIncreaseSkill(STRINGED_INSTRUMENTS, nullptr, -15);
			else
				Message_StringID(13,NO_INSTRUMENT_SKILL);
		}
		else
			CheckIncreaseSkill(SINGING, nullptr, -15);
		break;
	case WIND_INSTRUMENTS:
		if(this->itembonuses.windMod > 0) {
			if(GetRawSkill(WIND_INSTRUMENTS) > 0)
				CheckIncreaseSkill(WIND_INSTRUMENTS, nullptr, -15);
			else
				Message_StringID(13,NO_INSTRUMENT_SKILL);
		}
		else
			CheckIncreaseSkill(SINGING, nullptr, -15);
		break;
	case BRASS_INSTRUMENTS:
		if(this->itembonuses.brassMod > 0) {
			if(GetRawSkill(BRASS_INSTRUMENTS) > 0)
				CheckIncreaseSkill(BRASS_INSTRUMENTS, nullptr, -15);
			else
				Message_StringID(13,NO_INSTRUMENT_SKILL);
		}
		else
			CheckIncreaseSkill(SINGING, nullptr, -15);
		break;
	default:
		break;
	}
}

/*
returns true if spell is successful, false if it fizzled.
only works for clients, npcs shouldn't be fizzling..
new algorithm thats closer to live eq (i hope)
TODO: Add aa skills, item mods, reduced the chance to fizzle
*/
bool Mob::CheckFizzle(uint16 spell_id)
{
	return(true);
}

bool Client::CheckFizzle(uint16 spell_id)
{
	// GMs don't fizzle
	if (GetGM()) return(true);

	uint8 no_fizzle_level = 0;

	//Live AA - Spell Casting Expertise, Mastery of the Past
	no_fizzle_level = aabonuses.MasteryofPast + itembonuses.MasteryofPast + spellbonuses.MasteryofPast;

	if (spells[spell_id].classes[GetClass()-1] < no_fizzle_level)
		return true;

	//is there any sort of focus that affects fizzling?

	int par_skill;
	int act_skill;

	par_skill = spells[spell_id].classes[GetClass()-1] * 5 - 10;//IIRC even if you are lagging behind the skill levels you don't fizzle much
	if (par_skill > 235)
		par_skill = 235;

	par_skill += spells[spell_id].classes[GetClass()-1]; // maximum of 270 for level 65 spell

	act_skill = GetSkill(spells[spell_id].skill);
	act_skill += GetLevel(); // maximum of whatever the client can cheat

	//spell specialization
	float specialize = GetSpecializeSkillValue(spell_id);
	if(specialize > 0) {
		switch(GetAA(aaSpellCastingMastery)){
		case 1:
			specialize = specialize * 1.05;
			break;
		case 2:
			specialize = specialize * 1.15;
			break;
		case 3:
			specialize = specialize * 1.3;
			break;
		}
		if(((specialize/6.0f) + 15.0f) < MakeRandomFloat(0, 100)) {
			specialize *= SPECIALIZE_FIZZLE / 200;
		} else {
			specialize = 0.0f;
		}
	}

	// == 0 --> on par
	// > 0 --> skill is lower, higher chance of fizzle
	// < 0 --> skill is better, lower chance of fizzle
	// the max that diff can be is +- 235
	float diff = par_skill + spells[spell_id].basediff - act_skill;

	// if you have high int/wis you fizzle less, you fizzle more if you are stupid
	if(GetClass() == BARD)
	{
		diff -= (GetCHA() - 110) / 20.0;
	}
	else if (GetCasterClass() == 'W')
	{
		diff -= (GetWIS() - 125) / 20.0;
	}
	else if (GetCasterClass() == 'I')
	{
		diff -= (GetINT() - 125) / 20.0;
	}

	// base fizzlechance is lets say 5%, we can make it lower for AA skills or whatever
	float basefizzle = 10;
	float fizzlechance = basefizzle - specialize + diff / 5.0;

	// always at least 1% chance to fail or 5% to succeed
	fizzlechance = fizzlechance < 1 ? 1 : (fizzlechance > 95 ? 95 : fizzlechance);

	/*
	if(IsBardSong(spell_id))
	{
		//This was a channel chance modifier - no evidence for fizzle reduction
		fizzlechance -= GetAA(aaInternalMetronome) * 1.5f;
	}
	*/

	float fizzle_roll = MakeRandomFloat(0, 100);

	mlog(SPELLS__CASTING, "Check Fizzle %s  spell %d  fizzlechance: %0.2f%%   diff: %0.2f  roll: %0.2f", GetName(), spell_id, fizzlechance, diff, fizzle_roll);

	if(fizzle_roll > fizzlechance)
		return(true);
	return(false);
}

void Mob::ZeroCastingVars()
{
	// zero out the state keeping vars
	attacked_count = 0;
	spellend_timer.Disable();
	casting_spell_id = 0;
	casting_spell_targetid = 0;
	casting_spell_slot = 0;
	casting_spell_mana = 0;
	casting_spell_inventory_slot = 0;
	casting_spell_timer = 0;
	casting_spell_timer_duration = 0;
	casting_spell_type = 0;
	casting_spell_resist_adjust = 0;
	delaytimer = false;
}

void Mob::InterruptSpell(uint16 spellid)
{
	if (spellid == SPELL_UNKNOWN)
		spellid = casting_spell_id;

	InterruptSpell(0, 0x121, spellid);
}

// color not used right now
void Mob::InterruptSpell(uint16 message, uint16 color, uint16 spellid)
{
	EQApplicationPacket *outapp;
	uint16 message_other;
	bool bard_song_mode = false; //has the bard song gone to auto repeat mode
	if (spellid == SPELL_UNKNOWN) {
		if(bardsong) {
			spellid = bardsong;
			bard_song_mode = true;
		} else {
			spellid = casting_spell_id;
		}
	}

	if(casting_spell_id && IsNPC()) {
		CastToNPC()->AI_Event_SpellCastFinished(false, casting_spell_slot);
	}

	if(casting_spell_type == 1 && IsClient()) { //Rest AA Timer on failed cast
		CastToClient()->SendAATimer(casting_spell_timer - pTimerAAStart, 0, 0xFFFFFF);
		CastToClient()->Message_StringID(15,ABILITY_FAILED);
		CastToClient()->GetPTimers().Clear(&database, casting_spell_timer);
	}

	ZeroCastingVars();	// resets all the state keeping stuff

	mlog(SPELLS__CASTING, "Spell %d has been interrupted.", spellid);

	if(!spellid)
		return;

	if (bardsong || IsBardSong(casting_spell_id))
		_StopSong();

	if(bard_song_mode) {
		return;
	}

	if(!message)
		message = IsBardSong(spellid) ? SONG_ENDS_ABRUPTLY : INTERRUPT_SPELL;

	// clients need some packets
	if (IsClient() && message != SONG_ENDS)
	{
		// the interrupt message
		outapp = new EQApplicationPacket(OP_InterruptCast, sizeof(InterruptCast_Struct));
		InterruptCast_Struct* ic = (InterruptCast_Struct*) outapp->pBuffer;
		ic->messageid = message;
		ic->spawnid = GetID();
		outapp->priority = 5;
		CastToClient()->QueuePacket(outapp);
		safe_delete(outapp);

		SendSpellBarEnable(spellid);
	}

	// notify people in the area

	// first figure out what message others should get
	switch(message)
	{
		case SONG_ENDS:
			message_other = SONG_ENDS_OTHER;
			break;
		case SONG_ENDS_ABRUPTLY:
			message_other = SONG_ENDS_ABRUPTLY_OTHER;
			break;
		case MISS_NOTE:
			message_other = MISSED_NOTE_OTHER;
			break;
		case SPELL_FIZZLE:
			message_other = SPELL_FIZZLE_OTHER;
			break;
		default:
			message_other = INTERRUPT_SPELL_OTHER;
	}

	// this is the actual message, it works the same as a formatted message
	outapp = new EQApplicationPacket(OP_InterruptCast, sizeof(InterruptCast_Struct) + strlen(GetCleanName()) + 1);
	InterruptCast_Struct* ic = (InterruptCast_Struct*) outapp->pBuffer;
	ic->messageid = message_other;
	ic->spawnid = GetID();
	strcpy(ic->message, GetCleanName());
	entity_list.QueueCloseClients(this, outapp, true, 200, 0, true, IsClient() ? FilterPCSpells : FilterNPCSpells);
	safe_delete(outapp);

}

// this is called after the timer is up and the spell is finished
// casting. everything goes through here, including items with zero cast time
// only to be used from SpellProcess
// NOTE: do not put range checking, etc into this function. this should
// just check timed spell specific things before passing off to SpellFinished
// which figures out proper targets etc
void Mob::CastedSpellFinished(uint16 spell_id, uint32 target_id, uint16 slot,
							uint16 mana_used, uint32 inventory_slot, int16 resist_adjust)
{
	_ZP(Mob_CastedSpellFinished);

	bool IsFromItem = false;

	if(IsClient() && slot != USE_ITEM_SPELL_SLOT && slot != POTION_BELT_SPELL_SLOT && spells[spell_id].recast_time > 1000) { // 10 is item
		if(!CastToClient()->GetPTimers().Expired(&database, pTimerSpellStart + spell_id, false)) {
			//should we issue a message or send them a spell gem packet?
			Message_StringID(13, SPELL_RECAST);
			mlog(SPELLS__CASTING_ERR, "Casting of %d canceled: spell reuse timer not expired", spell_id);
			InterruptSpell();
			return;
		}
	}

	if(IsClient() && ((slot == USE_ITEM_SPELL_SLOT) || (slot == POTION_BELT_SPELL_SLOT)))
	{
		IsFromItem = true;
		ItemInst *itm = CastToClient()->GetInv().GetItem(inventory_slot);
		if(itm && itm->GetItem()->RecastDelay > 0)
		{
			if(!CastToClient()->GetPTimers().Expired(&database, (pTimerItemStart + itm->GetItem()->RecastType), false)) {
				Message_StringID(13, SPELL_RECAST);
				mlog(SPELLS__CASTING_ERR, "Casting of %d canceled: item spell reuse timer not expired", spell_id);
				InterruptSpell();
				return;
			}
		}
	}

	if(!IsValidSpell(spell_id))
	{
		mlog(SPELLS__CASTING_ERR, "Casting of %d canceled: invalid spell id", spell_id);
		InterruptSpell();
		return;
	}

	// prevent rapid recast - this can happen if somehow the spell gems
	// become desynced and the player casts again.
	if(IsClient())
	{
		if(delaytimer)
		{
			mlog(SPELLS__CASTING_ERR, "Casting of %d canceled: recast too quickly", spell_id);
			Message(13, "You are unable to focus.");
			InterruptSpell();
			return;
		}
	}

	// make sure they aren't somehow casting 2 timed spells at once
	if (casting_spell_id != spell_id)
	{
		mlog(SPELLS__CASTING_ERR, "Casting of %d canceled: already casting", spell_id);
		Message_StringID(13,ALREADY_CASTING);
		InterruptSpell();
		return;
	}

	bool bard_song_mode = false;
	bool regain_conc = false;
	Mob *spell_target = entity_list.GetMob(target_id);
	// here we do different things if this is a bard casting a bard song from
	// a spell bar slot
	if(GetClass() == BARD) // bard's can move when casting any spell...
	{
		if (IsBardSong(spell_id)) {
			if(spells[spell_id].buffduration == 0xFFFF || spells[spell_id].recast_time != 0) {
				mlog(SPELLS__BARDS, "Bard song %d not applying bard logic because duration or recast is wrong: dur=%d, recast=%d", spells[spell_id].buffduration, spells[spell_id].recast_time);
			} else {
				bardsong = spell_id;
				bardsong_slot = slot;
				//NOTE: theres a lot more target types than this to think about...
				if (spell_target == nullptr || (spells[spell_id].targettype != ST_Target && spells[spell_id].targettype != ST_AETarget))
					bardsong_target_id = GetID();
				else
					bardsong_target_id = spell_target->GetID();
				bardsong_timer.Start(6000);
				mlog(SPELLS__BARDS, "Bard song %d started: slot %d, target id %d", bardsong, bardsong_slot, bardsong_target_id);
				bard_song_mode = true;
			}
		}
	}
	else // not bard, check movement
	{
		// if has been attacked, or moved while casting
		// check for regain concentration
		if
		(
			attacked_count > 0 ||
			GetX() != GetSpellX() ||
			GetY() != GetSpellY()
		)
		{
			// modify the chance based on how many times they were hit
			// but cap it so it's not that large a factor
			if(attacked_count > 15) attacked_count = 15;

			float channelchance, distance_moved, d_x, d_y, distancemod;

			if(IsClient())
			{
				float channelbonuses = 0.0f;
				//AA that effect Spell channel chance are no longer on live. http://everquest.allakhazam.com/history/patches-2006-2.html
				//No harm in maintaining the effects regardless, since we do check for channel chance.
				if (IsFromItem)
					channelbonuses += spellbonuses.ChannelChanceItems + itembonuses.ChannelChanceItems + aabonuses.ChannelChanceItems;
				else
					channelbonuses += spellbonuses.ChannelChanceSpells + itembonuses.ChannelChanceSpells + aabonuses.ChannelChanceSpells;

				// max 93% chance at 252 skill
				channelchance = 30 + GetSkill(CHANNELING) / 400.0f * 100;
				channelchance -= attacked_count * 2;
				channelchance += channelchance * channelbonuses / 100.0f;
			}
#ifdef BOTS
			else if(IsBot()) {
				float channelbonuses = 0.0f;

				if (IsFromItem)
					channelbonuses += spellbonuses.ChannelChanceItems + itembonuses.ChannelChanceItems + aabonuses.ChannelChanceItems;
				else
					channelbonuses += spellbonuses.ChannelChanceSpells + itembonuses.ChannelChanceSpells + aabonuses.ChannelChanceSpells;

				// max 93% chance at 252 skill
				channelchance = 30 + GetSkill(CHANNELING) / 400.0f * 100;
				channelchance -= attacked_count * 2;
				channelchance += channelchance * channelbonuses / 100.0f;
			}
#endif //BOTS
			else {
				// NPCs are just hard to interrupt, otherwise they get pwned
				channelchance = 85;
				channelchance -= attacked_count;
			}

			// as you get farther from your casting location,
			// it gets squarely harder to regain concentration
			if(GetX() != GetSpellX() || GetY() != GetSpellY())
			{
				d_x = fabs(fabs(GetX()) - fabs(GetSpellX()));
				d_y = fabs(fabs(GetY()) - fabs(GetSpellY()));
				if(d_x < 5 && d_y < 5)
				{
					//avoid the square root...
					distance_moved = d_x * d_x + d_y * d_y;
					// if you moved 1 unit, that's 25% off your chance to regain.
					// if you moved 2, you lose 100% off your chance
					distancemod = distance_moved * 25;
					channelchance -= distancemod;
				}
				else
				{
					channelchance = 0;
				}
			}

			mlog(SPELLS__CASTING, "Checking Interruption: spell x: %f  spell y: %f  cur x: %f  cur y: %f channelchance %f channeling skill %d\n", GetSpellX(), GetSpellY(), GetX(), GetY(), channelchance, GetSkill(CHANNELING));

			if(MakeRandomFloat(0, 100) > channelchance) {
				mlog(SPELLS__CASTING_ERR, "Casting of %d canceled: interrupted.", spell_id);
				InterruptSpell();
				return;
			}
			// if we got here, we regained concentration
			regain_conc = true;
			Message_StringID(MT_Spells,REGAIN_AND_CONTINUE);
			entity_list.MessageClose_StringID(this, true, 200, MT_Spells, OTHER_REGAIN_CAST, this->GetCleanName());
		}
	}

	// Check for consumables and Reagent focus items
	// first check for component reduction
	if(IsClient()) {
		int reg_focus = CastToClient()->GetFocusEffect(focusReagentCost,spell_id);
		if(MakeRandomInt(1, 100) <= reg_focus) {
			mlog(SPELLS__CASTING, "Spell %d: Reagent focus item prevented reagent consumption (%d chance)", spell_id, reg_focus);
		} else {
			if(reg_focus > 0)
				mlog(SPELLS__CASTING, "Spell %d: Reagent focus item failed to prevent reagent consumption (%d chance)", spell_id, reg_focus);
			Client *c = this->CastToClient();
			int component, component_count, inv_slot_id;
			bool missingreags = false;
			for(int t_count = 0; t_count < 4; t_count++) {
				component = spells[spell_id].components[t_count];
				component_count = spells[spell_id].component_counts[t_count];

				if (component == -1)
					continue;

				// bard components are requirements for a certain instrument type, not a specific item
				if(bard_song_mode) {
					bool HasInstrument = true;
					int InstComponent = spells[spell_id].NoexpendReagent[0];

					switch (InstComponent) {
						case -1:
							continue;		// no instrument required, go to next component

						// percussion songs (13000 = hand drum)
						case 13000:
							if(itembonuses.percussionMod == 0) {			// check for the appropriate instrument type
								HasInstrument = false;
								c->Message_StringID(13, SONG_NEEDS_DRUM);	// send an error message if missing
							}
							break;

						// wind songs (13001 = wooden flute)
						case 13001:
							if(itembonuses.windMod == 0) {
								HasInstrument = false;
								c->Message_StringID(13, SONG_NEEDS_WIND);
							}
							break;

						// string songs (13011 = lute)
						case 13011:
							if(itembonuses.stringedMod == 0) {
								HasInstrument = false;
								c->Message_StringID(13, SONG_NEEDS_STRINGS);
							}
							break;

						// brass songs (13012 = horn)
						case 13012:
							if(itembonuses.brassMod == 0) {
								HasInstrument = false;
								c->Message_StringID(13, SONG_NEEDS_BRASS);
							}
							break;

						default:	// some non-instrument component. Let it go, but record it in the log
							mlog(SPELLS__CASTING_ERR, "Something odd happened: Song %d required component %s", spell_id, component);
					}

					if(!HasInstrument) {	// if the instrument is missing, log it and interrupt the song
						mlog(SPELLS__CASTING_ERR, "Song %d: Canceled. Missing required instrument %s", spell_id, component);
						if(c->GetGM())
							c->Message(0, "Your GM status allows you to finish casting even though you're missing a required instrument.");
						else {
							InterruptSpell();
							return;
						}
					}
				}	// end bard component section

				// handle the components for traditional casters
				else {
					if(c->GetInv().HasItem(component, component_count, invWhereWorn|invWherePersonal) == -1) // item not found
					{
						if (!missingreags)
						{
							c->Message_StringID(13, MISSING_SPELL_COMP);
							missingreags=true;
						}

						const Item_Struct *item = database.GetItem(component);
						if(item) {
							c->Message_StringID(13, MISSING_SPELL_COMP_ITEM, item->Name);
							mlog(SPELLS__CASTING_ERR, "Spell %d: Canceled. Missing required reagent %s (%d)", spell_id, item->Name, component);
						}
						else {
							char TempItemName[64];
							strcpy((char*)&TempItemName, "UNKNOWN");
							mlog(SPELLS__CASTING_ERR, "Spell %d: Canceled. Missing required reagent %s (%d)", spell_id, TempItemName, component);
						}
					}
				} // end bard/not bard ifs
			} // end reagent loop

			if (missingreags) {
				if(c->GetGM())
					c->Message(0, "Your GM status allows you to finish casting even though you're missing required components.");
				else {
					InterruptSpell();
					return;
				}
			}
			else if (!bard_song_mode)
			{
				for(int t_count = 0; t_count < 4; t_count++) {
					component = spells[spell_id].components[t_count];
					if (component == -1)
						continue;
					component_count = spells[spell_id].component_counts[t_count];
					mlog(SPELLS__CASTING_ERR, "Spell %d: Consuming %d of spell component item id %d", spell_id, component, component_count);
					// Components found, Deleting
					// now we go looking for and deleting the items one by one
					for(int s = 0; s < component_count; s++)
					{
						inv_slot_id = c->GetInv().HasItem(component, 1, invWhereWorn|invWherePersonal);
						if(inv_slot_id != -1)
						{
							c->DeleteItemInInventory(inv_slot_id, 1, true);
						}
						else
						{	// some kind of error in the code if this happens
							c->Message(13, "ERROR: reagent item disappeared while processing?");
						}
					}
				}
				} // end missingreags/consumption
			} // end `focus did not help us`
	} // end IsClient() for reagents

	// this is common to both bard and non bard

	// if this was cast from an inventory slot, check out the item that's there

	int16 DeleteChargeFromSlot = -1;

	if(IsClient() && ((slot == USE_ITEM_SPELL_SLOT) || (slot == POTION_BELT_SPELL_SLOT))
		&& inventory_slot != 0xFFFFFFFF)	// 10 is an item
	{
        bool fromaug = false;
        const ItemInst* inst = CastToClient()->GetInv()[inventory_slot];
        Item_Struct* augitem = 0;
        uint32 recastdelay = 0;
        uint32 recasttype = 0;

        for(int r = 0; r < MAX_AUGMENT_SLOTS; r++) {
            const ItemInst* aug_i = inst->GetAugment(r);

            if(!aug_i)
                continue;
            const Item_Struct* aug = aug_i->GetItem();
            if(!aug)
                continue;

            if ( aug->Click.Effect == spell_id )
            {
                recastdelay = aug_i->GetItem()->RecastDelay;
                recasttype = aug_i->GetItem()->RecastType;
                fromaug = true;
                break;
            }
        }

        //Test the aug recast delay
        if(IsClient() && fromaug && recastdelay > 0)
        {
            if(!CastToClient()->GetPTimers().Expired(&database, (pTimerItemStart + recasttype), false)) {
                Message_StringID(13, SPELL_RECAST);
                mlog(SPELLS__CASTING_ERR, "Casting of %d canceled: item spell reuse timer not expired", spell_id);
                InterruptSpell();  
                return;
            }
            else
            {
                //Can we start the timer here?  I don't see why not.
                CastToClient()->GetPTimers().Start((pTimerItemStart + recasttype), recastdelay);
            }
        }

		if (inst && inst->IsType(ItemClassCommon) && (inst->GetItem()->Click.Effect == spell_id) && inst->GetCharges() || fromaug)
		{
			//const Item_Struct* item = inst->GetItem();
			int16 charges = inst->GetItem()->MaxCharges;

            if(fromaug) { charges = -1; } //Don't destroy the parent item

			if(charges > -1) {	// charged item, expend a charge
				mlog(SPELLS__CASTING, "Spell %d: Consuming a charge from item %s (%d) which had %d/%d charges.", spell_id, inst->GetItem()->Name, inst->GetItem()->ID, inst->GetCharges(), inst->GetItem()->MaxCharges);
				DeleteChargeFromSlot = inventory_slot;
			} else {
				mlog(SPELLS__CASTING, "Spell %d: Cast from unlimited charge item %s (%d) (%d charges)", spell_id, inst->GetItem()->Name, inst->GetItem()->ID, inst->GetItem()->MaxCharges);
			}
		}
		else
		{
			mlog(SPELLS__CASTING_ERR, "Item used to cast spell %d was missing from inventory slot %d after casting!", spell_id, inventory_slot);
			Message(13, "Casting Error: Active casting item not found in inventory slot %i", inventory_slot);
			InterruptSpell();
			return;
		}
	}
	if(IsClient()) {
		uint32 buff_max = GetMaxTotalSlots();
		for (int buffSlot = 0; buffSlot < buff_max; buffSlot++) {
			if (buffs[buffSlot].spellid == 0 || buffs[buffSlot].spellid >= SPDAT_RECORDS)
				continue;

			if(spells[buffs[buffSlot].spellid].numhits > 0)
				CheckHitsRemaining(buffSlot, true);
		}

		TrySympatheticProc(target, spell_id);
	}

	TryTwincast(this, target, spell_id);

	TryTriggerOnCast(spell_id, 0);

	// we're done casting, now try to apply the spell
	if( !SpellFinished(spell_id, spell_target, slot, mana_used, inventory_slot, resist_adjust) )
	{
		mlog(SPELLS__CASTING_ERR, "Casting of %d canceled: SpellFinished returned false.", spell_id);
		InterruptSpell();
		return;
	}

	if(DeleteChargeFromSlot >= 0)
		CastToClient()->DeleteItemInInventory(DeleteChargeFromSlot, 1, true);

	//
	// at this point the spell has successfully been cast
	//

	if(IsClient()) {
		char temp[64];
		sprintf(temp, "%d", spell_id);
		parse->EventPlayer(EVENT_CAST, CastToClient(), temp, 0);
	} else if(IsNPC()) {
		char temp[64];
		sprintf(temp, "%d", spell_id);
		parse->EventNPC(EVENT_CAST, CastToNPC(), nullptr, temp, 0);
	}

	if(bard_song_mode)
	{
		if(IsClient())
		{
			this->CastToClient()->CheckSongSkillIncrease(spell_id);
			this->CastToClient()->MemorizeSpell(slot, spell_id, memSpellSpellbar);
		}
		mlog(SPELLS__CASTING, "Bard song %d should be started", spell_id);
	}
	else
	{
		if(IsClient())
		{
			Client *c = CastToClient();
			SendSpellBarEnable(spell_id);

			// this causes the delayed refresh of the spell bar gems
			c->MemorizeSpell(slot, spell_id, memSpellSpellbar);

			// this tells the client that casting may happen again
			SetMana(GetMana());

			// skills
			if(slot < MAX_PP_MEMSPELL)
			{
				c->CheckIncreaseSkill(spells[spell_id].skill, nullptr);

				// increased chance of gaining channel skill if you regained concentration
				c->CheckIncreaseSkill(CHANNELING, nullptr, regain_conc ? 5 : 0);

				c->CheckSpecializeIncrease(spell_id);
			}
		}
	}

	// there should be no casting going on now
	ZeroCastingVars();

	// set the rapid recast timer for next time around
	delaytimer = true;
	spellend_timer.Start(400,true);

	mlog(SPELLS__CASTING, "Spell casting of %d is finished.", spell_id);

}

bool Mob::DetermineSpellTargets(uint16 spell_id, Mob *&spell_target, Mob *&ae_center, CastAction_type &CastAction) {

/*
	The basic types of spells:

	Single target - some might be undead only, self only, etc, but these
	all affect the target of the caster.

	AE around caster - these affect entities close to the caster, and have
	no target.

	AE around target - these have a target, and affect the target as well as
	entities close to the target.

	AE on location - this is a tricky one that is cast on a mob target but
	has a special AE duration that keeps it recasting every 2.5 sec on the
	same location. These work the same as AE around target spells, except
	the target is a special beacon that's created when the spell is cast

	Group - the caster is always affected, but there's more
		targetgroupbuffs on - these affect the target and the target's group.
		targetgroupbuffs off - no target, affects the caster's group.

	Group Teleport - the caster plus his group are affected. these cannot
	be targeted.

	I think the string ID SPELL_NEED_TAR is wrong, it dosent seem to show up.
*/

	// during this switch, this variable gets set to one of these things
	// and that causes the spell to be executed differently

	bodyType target_bt = BT_Humanoid;
	SpellTargetType targetType = spells[spell_id].targettype;
	bodyType mob_body = spell_target ? spell_target->GetBodyType() : BT_Humanoid;

	if(IsPlayerIllusionSpell(spell_id)
		&& spell_target != nullptr // null ptr crash safeguard
		&& !spell_target->IsNPC() // still self only if NPC targetted
		&& IsClient()
		&& (IsGrouped() // still self only if not grouped
		|| IsRaidGrouped())
		&& CastToClient()->CheckAAEffect(aaEffectProjectIllusion)){
			mlog(AA__MESSAGE, "Project Illusion overwrote target caster: %s spell id: %d was ON", GetName(), spell_id);
			targetType = ST_GroupClientAndPet;
	}

	switch (targetType)
	{
// single target spells
		case ST_Self:
		{
			spell_target = this;
			CastAction = SingleTarget;
			break;
		}

		case ST_TargetOptional:
		{
			if(!spell_target)
				spell_target = this;
			CastAction = SingleTarget;
			break;
		}

		// target required for these
		case ST_Undead: {
			if(!spell_target || (
				spell_target->GetBodyType() != BT_SummonedUndead
				&& spell_target->GetBodyType() != BT_Undead
				&& spell_target->GetBodyType() != BT_Vampire
				)
			)
			{
				//invalid target
				mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target of body type %d (undead)", spell_id, spell_target->GetBodyType());
				Message_StringID(13,SPELL_NEED_TAR);
				return false;
			}
			CastAction = SingleTarget;
			break;
		}

		case ST_Summoned: {
			uint8 body_type = spell_target?spell_target->GetBodyType():0;
			if(!spell_target || (body_type != BT_Summoned && body_type != BT_Summoned2 && body_type != BT_Summoned3))
			{
				//invalid target
				mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target of body type %d (summoned)", spell_id, body_type);
				Message_StringID(13,SPELL_NEED_TAR);
				return false;
			}
			CastAction = SingleTarget;
			break;
		}

		case ST_SummonedPet:
		{
			uint8 body_type = spell_target ? spell_target->GetBodyType() : 0;
			if(!spell_target || (spell_target != GetPet()) ||
				(body_type != BT_Summoned && body_type != BT_Summoned2 && body_type != BT_Summoned3 && body_type != BT_Animal))
			{
				mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target of body type %d (summoned pet)",
							spell_id, body_type);

				Message_StringID(13, SPELL_NEED_TAR);

				return false;
			}
			CastAction = SingleTarget;
			break;
		}
		//single body type target spells...
		//this is a little hackish, but better than duplicating code IMO
		case ST_Plant: if(target_bt == BT_Humanoid) target_bt = BT_Plant;
		case ST_Dragon: if(target_bt == BT_Humanoid) target_bt = BT_Dragon;
		case ST_Giant: if(target_bt == BT_Humanoid) target_bt = BT_Giant;
		case ST_Animal: if(target_bt == BT_Humanoid) target_bt = BT_Animal;

		// check for special case body types (Velious dragons/giants)
		if(mob_body == BT_RaidGiant) mob_body = BT_Giant;
		if(mob_body == BT_VeliousDragon) mob_body = BT_Dragon;

		{
			if(!spell_target || mob_body != target_bt)
			{
				//invalid target
				mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target of body type %d (want body Type %d)", spell_id, spell_target->GetBodyType(), target_bt);
				if(!spell_target)
					Message_StringID(13,SPELL_NEED_TAR);
				else
					Message_StringID(13,CANNOT_AFFECT_NPC);
				return false;
			}
			CastAction = SingleTarget;
			break;
		}

		case ST_Tap:
		case ST_LDoNChest_Cursed:
		case ST_Target: {
			if(IsLDoNObjectSpell(spell_id))
			{
				if(!spell_target)
				{
					mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target (ldon object)", spell_id);
					Message_StringID(13,SPELL_NEED_TAR);
					return false;
				}
				else
				{
					if(!spell_target->IsNPC())
					{
						mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target (normal)", spell_id);
						Message_StringID(13,SPELL_NEED_TAR);
						return false;
					}

					if(spell_target->GetClass() != LDON_TREASURE)
					{
						mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target (normal)", spell_id);
						Message_StringID(13,SPELL_NEED_TAR);
						return false;
					}
				}
			}

			if(!spell_target)
			{
				mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target (normal)", spell_id);
				Message_StringID(13,SPELL_NEED_TAR);
				return false;	// can't cast these unless we have a target
			}
			CastAction = SingleTarget;
			break;
		}

		case ST_Corpse:
		{
			if(!spell_target || !spell_target->IsPlayerCorpse())
			{
				mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target (corpse)", spell_id);
				uint32 message = ONLY_ON_CORPSES;
				if(!spell_target) message = SPELL_NEED_TAR;
				else if(!spell_target->IsCorpse()) message = ONLY_ON_CORPSES;
				else if(!spell_target->IsPlayerCorpse()) message = CORPSE_NOT_VALID;
				Message_StringID(13, message);
				return false;
			}
			CastAction = SingleTarget;
			break;
		}
		case ST_Pet:
		{
			spell_target = GetPet();
			if(!spell_target)
			{
				mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target (no pet)", spell_id);
				Message_StringID(13,NO_PET);
				return false;	// can't cast these unless we have a target
			}
			CastAction = SingleTarget;
			break;
		}

		case ST_AEBard:
		case ST_AECaster:
		{
			spell_target = nullptr;
			ae_center = this;
			CastAction = AECaster;
			break;
		}

		case ST_HateList:
		{
			spell_target = nullptr;
			ae_center = this;
			CastAction = CAHateList;
			break;
		}

		case ST_UndeadAE:	//should only affect undead...
		case ST_AETarget:
		{
			if(!spell_target)
			{
				mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target (AOE)", spell_id);
				Message_StringID(13,SPELL_NEED_TAR);
				return false;
			}
			ae_center = spell_target;
			CastAction = AETarget;
			break;
		}

		// Group spells
		case ST_GroupTeleport:
		case ST_Group:
		{
			if(IsClient() && CastToClient()->TGB() && IsTGBCompatibleSpell(spell_id)) {
				if( (!target) ||
					(target->IsNPC() && !(target->GetOwner() && target->GetOwner()->IsClient())) ||
					(target->IsCorpse()) )
					spell_target = this;
				else
					spell_target = target;
			} else {
				spell_target = this;
			}
			CastAction = GroupSpell;
			break;
		}
		case ST_GroupClientAndPet:
		{
			if(!spell_target)
			{
				mlog(SPELLS__CASTING_ERR, "Spell %d canceled: invalid target (Group Required: Single Target)", spell_id);
				Message_StringID(13,SPELL_NEED_TAR);
				return false;
			}

			if(spell_target != this)
			{
				if(spell_target == GetPet())
				{
					CastAction = SingleTarget;
				}
				else if(spell_target == GetOwner())
				{
					CastAction = SingleTarget;
				}
				else
				{
					uint32 group_id_caster = 0;
					uint32 group_id_target = 0;
					if(IsClient())
					{
						if(IsGrouped())
						{
							group_id_caster = GetGroup()->GetID();
						}
						else if(IsRaidGrouped())
						{
							group_id_caster = (GetRaid()->GetGroup(CastToClient()) == 0xFFFF) ? 0 : (GetRaid()->GetGroup(CastToClient()) + 1);
						}
					}
					else if(IsPet())
					{
						Mob *owner = GetOwner();
						if(owner->IsGrouped())
						{
							group_id_caster = owner->GetGroup()->GetID();
						}
						else if(owner->IsRaidGrouped())
						{
							group_id_caster = (owner->GetRaid()->GetGroup(CastToClient()) == 0xFFFF) ? 0 : (owner->GetRaid()->GetGroup(CastToClient()) + 1);
						}
					}
#ifdef BOTS
					else if(IsBot())
					{
						if(IsGrouped())
						{
							group_id_caster = GetGroup()->GetID();
						}
						else if(IsRaidGrouped())
						{
							if(GetOwner())
								group_id_caster = (GetRaid()->GetGroup(GetOwner()->CastToClient()) == 0xFFFF) ? 0 : (GetRaid()->GetGroup(GetOwner()->CastToClient()) + 1);
						}
					}
#endif //BOTS

					if(spell_target->IsClient())
					{
						if(spell_target->IsGrouped())
						{
							group_id_target = spell_target->GetGroup()->GetID();
						}
						else if(spell_target->IsRaidGrouped())
						{
							group_id_target = (spell_target->GetRaid()->GetGroup(CastToClient()) == 0xFFFF) ? 0 : (spell_target->GetRaid()->GetGroup(CastToClient()) + 1);
						}
					}
					else if(spell_target->IsPet())
					{
						Mob *owner = spell_target->GetOwner();
						if(owner->IsGrouped())
						{
							group_id_target = owner->GetGroup()->GetID();
						}
						else if(owner->IsRaidGrouped())
						{
							group_id_target = (owner->GetRaid()->GetGroup(CastToClient()) == 0xFFFF) ? 0 : (owner->GetRaid()->GetGroup(CastToClient()) + 1);
						}
					}
#ifdef BOTS
					else if(spell_target->IsBot())
					{
						if(spell_target->IsGrouped())
						{
							group_id_target = spell_target->GetGroup()->GetID();
						}
						else if(spell_target->IsRaidGrouped())
						{
							if(spell_target->GetOwner())
								group_id_target = (spell_target->GetRaid()->GetGroup(spell_target->GetOwner()->CastToClient()) == 0xFFFF) ? 0 : (spell_target->GetRaid()->GetGroup(spell_target->GetOwner()->CastToClient()) + 1);
						}
					}
#endif //BOTS

					if(group_id_caster == 0 || group_id_target == 0)
					{
						mlog(SPELLS__CASTING_ERR, "Spell %d canceled: Attempted to cast a Single Target Group spell on a ungrouped member.", spell_id);
						Message_StringID(13, TARGET_GROUP_MEMBER);
						return false;
					}

					if(group_id_caster != group_id_target)
					{
						mlog(SPELLS__CASTING_ERR, "Spell %d canceled: Attempted to cast a Single Target Group spell on a ungrouped member.", spell_id);
						Message_StringID(13, TARGET_GROUP_MEMBER);
						return false;
					}

					CastAction = SingleTarget;
				}
			}
			else
			{
				CastAction = SingleTarget;
			}
			break;
		}

		case ST_Directional:
			CastAction = DirectionalAE;
			spell_target = nullptr;
			ae_center = nullptr;
			break;

		case ST_TargetsTarget:
		{
			Mob *spell_target_tot = spell_target ? spell_target->GetTarget() : nullptr;
			if(!spell_target_tot)
				return false;
			//Verfied from live - Target's Target needs to be in combat range to recieve the effect
			if (!this->CombatRange(spell_target))
				return false;

			spell_target = spell_target_tot;
			CastAction = SingleTarget;
			break;
		}

		default:
		{
			mlog(SPELLS__CASTING_ERR, "I dont know Target Type: %d   Spell: (%d) %s", spells[spell_id].targettype, spell_id, spells[spell_id].name);
			Message(0, "I dont know Target Type: %d   Spell: (%d) %s", spells[spell_id].targettype, spell_id, spells[spell_id].name);
			CastAction = CastActUnknown;
			break;
		}
	}
	return(true);
}

// only used from CastedSpellFinished, and procs
// we can't interrupt in this, or anything called from this!
// if you need to abort the casting, return false
bool Mob::SpellFinished(uint16 spell_id, Mob *spell_target, uint16 slot, uint16 mana_used,
						uint32 inventory_slot, int16 resist_adjust, bool isproc)
{
	_ZP(Mob_SpellFinished);

	//EQApplicationPacket *outapp = nullptr;
	Mob *ae_center = nullptr;

	if(!IsValidSpell(spell_id))
		return false;

	if( spells[spell_id].zonetype == 1 && !zone->CanCastOutdoor()){
		if(IsClient()){
				if(!CastToClient()->GetGM()){
					Message_StringID(13, CAST_OUTDOORS);
					return false;
				}
			}
		}

	if(IsEffectInSpell(spell_id, SE_Levitate) && !zone->CanLevitate()){
			if(IsClient()){
				if(!CastToClient()->GetGM()){
					Message(13, "You can't levitate in this zone.");
					return false;
				}
			}
		}

	if(IsClient() && !CastToClient()->GetGM()){

		if(zone->IsSpellBlocked(spell_id, GetX(), GetY(), GetZ())){
			const char *msg = zone->GetSpellBlockedMessage(spell_id, GetX(), GetY(), GetZ());
			if(msg){
				Message(13, msg);
				return false;
			}
			else{
				Message(13, "You can't cast this spell here.");
				return false;
			}

		}
	}

	if
	(
		this->IsClient() &&
		(zone->GetZoneID() == 183 || zone->GetZoneID() == 184) &&	// load
		CastToClient()->Admin() < 80
	)
	{
		if
		(
			IsEffectInSpell(spell_id, SE_Gate) ||
			IsEffectInSpell(spell_id, SE_Translocate) ||
			IsEffectInSpell(spell_id, SE_Teleport)
		)
		{
			Message(0, "The Gods brought you here, only they can send you away.");
			return false;
		}
	}


	//determine the type of spell target we have
	CastAction_type CastAction;
	if(!DetermineSpellTargets(spell_id, spell_target, ae_center, CastAction))
		return(false);

	mlog(SPELLS__CASTING, "Spell %d: target type %d, target %s, AE center %s", spell_id, CastAction, spell_target?spell_target->GetName():"NONE", ae_center?ae_center->GetName():"NONE");

	// if a spell has the AEDuration flag, it becomes an AE on target
	// spell that's recast every 2500 msec for AEDuration msec. There are
	// spells of all kinds of target types that do this, strangely enough
	// TODO: finish this
	if(IsAEDurationSpell(spell_id)) {
		// the spells are AE target, but we aim them on a beacon
		Mob *beacon_loc = spell_target ? spell_target : this;
		Beacon *beacon = new Beacon(beacon_loc, spells[spell_id].AEDuration);
		entity_list.AddBeacon(beacon);
		mlog(SPELLS__CASTING, "Spell %d: AE duration beacon created, entity id %d", spell_id, beacon->GetName());
		spell_target = nullptr;
		ae_center = beacon;
		CastAction = AECaster;
	}

	// check line of sight to target if it's a detrimental spell
	if(spell_target && IsDetrimentalSpell(spell_id) && !CheckLosFN(spell_target) && !IsHarmonySpell(spell_id))
	{
		mlog(SPELLS__CASTING, "Spell %d: cannot see target %s", spell_target->GetName());
		Message_StringID(13,CANT_SEE_TARGET);
		return false;
	}

	// check to see if target is a caster mob before performing a mana tap
	if(spell_target && IsManaTapSpell(spell_id)) {
		if(spell_target->GetCasterClass() == 'N') {
			Message_StringID(13, TARGET_NO_MANA);
			return false;
		}
	}

	//range check our target, if we have one and it is not us
	float range = spells[spell_id].range;
	if(IsClient() && CastToClient()->TGB() && IsTGBCompatibleSpell(spell_id) && IsGroupSpell(spell_id))
		range = spells[spell_id].aoerange;

	range = GetActSpellRange(spell_id, range);
	if(IsPlayerIllusionSpell(spell_id)
		&& IsClient()
		&& CastToClient()->CheckAAEffect(aaEffectProjectIllusion)){
		range = 100;
	}
	if(spell_target != nullptr && spell_target != this) {
		//casting a spell on somebody but ourself, make sure they are in range
		float dist2 = DistNoRoot(*spell_target);
		float range2 = range * range;
		if(dist2 > range2) {
			//target is out of range.
			mlog(SPELLS__CASTING, "Spell %d: Spell target is out of range (squared: %f > %f)", spell_id, dist2, range2);
			Message_StringID(13, TARGET_OUT_OF_RANGE);
			return(false);
		}
	}

	//
	// Switch #2 - execute the spell
	//
	switch(CastAction)
	{
		default:
		case CastActUnknown:
		case SingleTarget:
		{

#ifdef BOTS
			if(IsBot()) {
				bool StopLogic = false;
				if(!this->CastToBot()->DoFinishedSpellSingleTarget(spell_id, spell_target, slot, StopLogic))
					return false;
				if(StopLogic)
					break;
			}
#endif //BOTS

			if(spell_target == nullptr) {
				mlog(SPELLS__CASTING, "Spell %d: Targeted spell, but we have no target", spell_id);
				return(false);
			}
			if (isproc) {
				SpellOnTarget(spell_id, spell_target, false, true, resist_adjust, true);
			} else {
				SpellOnTarget(spell_id, spell_target, false, true, resist_adjust, false);
			}
			if(IsPlayerIllusionSpell(spell_id)
			&& IsClient()
			&& CastToClient()->CheckAAEffect(aaEffectProjectIllusion)){
				mlog(AA__MESSAGE, "Effect Project Illusion for %s on spell id: %d was ON", GetName(), spell_id);
				CastToClient()->DisableAAEffect(aaEffectProjectIllusion);
			}
			else{
				mlog(AA__MESSAGE, "Effect Project Illusion for %s on spell id: %d was OFF", GetName(), spell_id);
			}
			break;
		}

		case AECaster:
		case AETarget:
		{
#ifdef BOTS
			if(IsBot()) {
				bool StopLogic = false;
				if(!this->CastToBot()->DoFinishedSpellAETarget(spell_id, spell_target, slot, StopLogic))
					return false;
				if(StopLogic)
					break;
			}
#endif //BOTS

			// we can't cast an AE spell without something to center it on
			assert(ae_center != nullptr);

			if(ae_center->IsBeacon()) {
				// special ae duration spell
				ae_center->CastToBeacon()->AELocationSpell(this, spell_id, resist_adjust);
			} else {
				// regular PB AE or targeted AE spell - spell_target is null if PB
				if(spell_target)	// this must be an AETarget spell
				{
					// affect the target too
					SpellOnTarget(spell_id, spell_target, false, true, resist_adjust);
				}
				if(ae_center && ae_center == this && IsBeneficialSpell(spell_id))
					SpellOnTarget(spell_id, this);

				bool affect_caster = !IsNPC();	//NPC AE spells do not affect the NPC caster
				entity_list.AESpell(this, ae_center, spell_id, affect_caster, resist_adjust);
			}
			break;
		}

		case GroupSpell:
		{
#ifdef BOTS
			if(IsBot()) {
				bool StopLogic = false;
				if(!this->CastToBot()->DoFinishedSpellGroupTarget(spell_id, spell_target, slot, StopLogic))
					return false;
				if(StopLogic)
					break;
			}
#endif //BOTS

			if(spells[spell_id].can_mgb && IsClient() && CastToClient()->CheckAAEffect(aaEffectMassGroupBuff))
			{
				SpellOnTarget(spell_id, this);
				entity_list.MassGroupBuff(this, this, spell_id, true);
				CastToClient()->DisableAAEffect(aaEffectMassGroupBuff);
			}
			else
			{
				// at this point spell_target is a member of the other group, or the
				// caster if they're not using TGB
				// NOTE: this will always hit the caster, plus the target's group so
				// it can affect up to 7 people if the targeted group is not our own
				if(spell_target->IsGrouped())
				{
					Group *target_group = entity_list.GetGroupByMob(spell_target);
					if(target_group)
					{
						target_group->CastGroupSpell(this, spell_id);
					}
				}
				else if(spell_target->IsRaidGrouped() && spell_target->IsClient())
				{
					Raid *target_raid = entity_list.GetRaidByClient(spell_target->CastToClient());
					uint32 gid = 0xFFFFFFFF;
					if(target_raid){
						gid = target_raid->GetGroup(spell_target->GetName());
						if(gid < 12)
							target_raid->CastGroupSpell(this, spell_id, gid);
						else
							SpellOnTarget(spell_id, spell_target);
					}
				}
				else
				{
					// if target is grouped, CastGroupSpell will cast it on the caster
					// too, but if not then we have to do that here.

					if(spell_target != this){
						SpellOnTarget(spell_id, this);
	#ifdef GROUP_BUFF_PETS
						//pet too
						if (GetPet() && HasPetAffinity() && !GetPet()->IsCharmed())
							SpellOnTarget(spell_id, GetPet());
	#endif
					}

					SpellOnTarget(spell_id, spell_target);
	#ifdef GROUP_BUFF_PETS
					//pet too
					if (spell_target->GetPet() && HasPetAffinity() && !spell_target->GetPet()->IsCharmed())
						SpellOnTarget(spell_id, spell_target->GetPet());
	#endif
				}
			}
			break;
		}

		case CAHateList:
		{
			if(!IsClient())
			{
				hate_list.SpellCast(this, spell_id, spells[spell_id].range > spells[spell_id].aoerange ? spells[spell_id].range : spells[spell_id].aoerange);
			}
			break;
		}

		case DirectionalAE:
		{
			float angle_start = spells[spell_id].directional_start + (GetHeading() * 360.0f / 256.0f);
			float angle_end = spells[spell_id].directional_end + (GetHeading() * 360.0f / 256.0f);

			while(angle_start > 360.0f)
				angle_start -= 360.0f;

			while(angle_end > 360.0f)
				angle_end -= 360.0f;

			std::list<Mob*> targets_in_range;
			std::list<Mob*>::iterator iter;

			entity_list.GetTargetsForConeArea(this, spells[spell_id].aoerange, spells[spell_id].aoerange / 2, targets_in_range);
			iter = targets_in_range.begin();
			while(iter != targets_in_range.end())
			{
				float heading_to_target = (CalculateHeadingToTarget((*iter)->GetX(), (*iter)->GetY()) * 360.0f / 256.0f);
				while(heading_to_target < 0.0f)
					heading_to_target += 360.0f;

				while(heading_to_target > 360.0f)
					heading_to_target -= 360.0f;

				if(angle_start > angle_end)
				{
					if((heading_to_target >= angle_start && heading_to_target <= 360.0f) ||
						(heading_to_target >= 0.0f && heading_to_target <= angle_end))
					{
						if(CheckLosFN(spell_target))
							SpellOnTarget(spell_id, spell_target, false, true, resist_adjust);
					}
				}
				else
				{
					if(heading_to_target >= angle_start && heading_to_target <= angle_end)
					{
						if(CheckLosFN((*iter)))
							SpellOnTarget(spell_id, (*iter), false, true, resist_adjust);
					}
				}
				iter++;
			}
			break;
		}
	}

	DoAnim(spells[spell_id].CastingAnim, 0, true, IsClient() ? FilterPCSpells : FilterNPCSpells);

	// Set and send the nimbus effect if this spell has one
	int NimbusEffect = GetNimbusEffect(spell_id);
	if(NimbusEffect) {
		if(!IsNimbusEffectActive(NimbusEffect)) {
			SendSpellEffect(NimbusEffect, 500, 0, 1, 3000, true);
		}
	}

	// if this was a spell slot or an ability use up the mana for it
	// CastSpell already reduced the cost for it if we're a client with focus
	if(slot != USE_ITEM_SPELL_SLOT && slot != POTION_BELT_SPELL_SLOT && mana_used > 0)
	{
		mlog(SPELLS__CASTING, "Spell %d: consuming %d mana", spell_id, mana_used);
		if (!DoHPToManaCovert(mana_used))
			SetMana(GetMana() - mana_used);
	}

	//set our reuse timer on long ass reuse_time spells...
	if(IsClient())
	{
		if(spell_id == casting_spell_id && casting_spell_timer != 0xFFFFFFFF)
		{
			CastToClient()->GetPTimers().Start(casting_spell_timer, casting_spell_timer_duration);
			mlog(SPELLS__CASTING, "Spell %d: Setting custom reuse timer %d to %d", spell_id, casting_spell_timer, casting_spell_timer_duration);
		}
		else if(spells[spell_id].recast_time > 1000) {
			int recast = spells[spell_id].recast_time/1000;
			if (spell_id == SPELL_LAY_ON_HANDS)	//lay on hands
			{
				recast -= GetAA(aaFervrentBlessing) * 420;
			}
			else if (spell_id == SPELL_HARM_TOUCH || spell_id == SPELL_HARM_TOUCH2)	//harm touch
			{
				recast -= GetAA(aaTouchoftheWicked) * 420;
			}
			int reduction = CastToClient()->GetFocusEffect(focusReduceRecastTime, spell_id);
			if(reduction)
				recast -= reduction;

			mlog(SPELLS__CASTING, "Spell %d: Setting long reuse timer to %d s (orig %d)", spell_id, recast, spells[spell_id].recast_time);
			CastToClient()->GetPTimers().Start(pTimerSpellStart + spell_id, recast);
		}
	}

	if(IsClient() && ((slot == USE_ITEM_SPELL_SLOT) || (slot == POTION_BELT_SPELL_SLOT)))
	{
		ItemInst *itm = CastToClient()->GetInv().GetItem(inventory_slot);
		if(itm && itm->GetItem()->RecastDelay > 0){
			CastToClient()->GetPTimers().Start((pTimerItemStart + itm->GetItem()->RecastType), itm->GetItem()->RecastDelay);
		}
	}

	if(IsNPC())
		CastToNPC()->AI_Event_SpellCastFinished(true, slot);

	return true;
}

/*
 * handle bard song pulses...
 *
 * we make several assumptions that SpellFinished does not:
 *	- there are no AEDuration (beacon) bard songs
 *	- there are no recourse spells on bard songs
 *	- there is no long recast delay on bard songs
 *
 * return false to stop the song
 */
bool Mob::ApplyNextBardPulse(uint16 spell_id, Mob *spell_target, uint16 slot) {
	if(slot == USE_ITEM_SPELL_SLOT) {
		//bard songs should never come from items...
		mlog(SPELLS__BARDS, "Bard Song Pulse %d: Supposidly cast from an item. Killing song.", spell_id);
		return(false);
	}

	//determine the type of spell target we have
	Mob *ae_center = nullptr;
	CastAction_type CastAction;
	if(!DetermineSpellTargets(spell_id, spell_target, ae_center, CastAction)) {
		mlog(SPELLS__BARDS, "Bard Song Pulse %d: was unable to determine target. Stopping.", spell_id);
		return(false);
	}

	if(ae_center != nullptr && ae_center->IsBeacon()) {
		mlog(SPELLS__BARDS, "Bard Song Pulse %d: Unsupported Beacon NPC AE spell", spell_id);
		return(false);
	}

	//use mana, if this spell has a mana cost
	int mana_used = spells[spell_id].mana;
	if(mana_used > 0) {
		if(mana_used > GetMana()) {
			//ran out of mana... this calls StopSong() for us
			mlog(SPELLS__BARDS, "Ran out of mana while singing song %d", spell_id);
			return(false);
		}

		mlog(SPELLS__CASTING, "Bard Song Pulse %d: consuming %d mana (have %d)", spell_id, mana_used, GetMana());
		SetMana(GetMana() - mana_used);
	}


	// check line of sight to target if it's a detrimental spell
	if(spell_target && IsDetrimentalSpell(spell_id) && !CheckLosFN(spell_target))
	{
		mlog(SPELLS__CASTING, "Bard Song Pulse %d: cannot see target %s", spell_target->GetName());
		Message_StringID(13, CANT_SEE_TARGET);
		return(false);
	}

	//range check our target, if we have one and it is not us
	float range = 0.00f;

	range = GetActSpellRange(spell_id, spells[spell_id].range, true);
	if(spell_target != nullptr && spell_target != this) {
		//casting a spell on somebody but ourself, make sure they are in range
		float dist2 = DistNoRoot(*spell_target);
		float range2 = range * range;
		if(dist2 > range2) {
			//target is out of range.
			mlog(SPELLS__BARDS, "Bard Song Pulse %d: Spell target is out of range (squared: %f > %f)", spell_id, dist2, range2);
			Message_StringID(13, TARGET_OUT_OF_RANGE);
			return(false);
		}
	}

	//
	// Switch #2 - execute the spell
	//
	switch(CastAction)
	{
		default:
		case CastActUnknown:
		case SingleTarget:
		{
			if(spell_target == nullptr) {
				mlog(SPELLS__BARDS, "Bard Song Pulse %d: Targeted spell, but we have no target", spell_id);
				return(false);
			}
			mlog(SPELLS__BARDS, "Bard Song Pulse: Targeted. spell %d, target %s", spell_id, spell_target->GetName());
			spell_target->BardPulse(spell_id, this);
			break;
		}

		case AECaster:
		{
			if(IsBeneficialSpell(spell_id))
				SpellOnTarget(spell_id, this);

			bool affect_caster = !IsNPC();	//NPC AE spells do not affect the NPC caster
			entity_list.AEBardPulse(this, this, spell_id, affect_caster);
			break;
		}
		case AETarget:
		{
			// we can't cast an AE spell without something to center it on
			if(ae_center == nullptr) {
				mlog(SPELLS__BARDS, "Bard Song Pulse %d: AE Targeted spell, but we have no target", spell_id);
				return(false);
			}

			// regular PB AE or targeted AE spell - spell_target is null if PB
			if(spell_target) {	// this must be an AETarget spell
				// affect the target too
				spell_target->BardPulse(spell_id, this);
				mlog(SPELLS__BARDS, "Bard Song Pulse: spell %d, AE target %s", spell_id, spell_target->GetName());
			} else {
				mlog(SPELLS__BARDS, "Bard Song Pulse: spell %d, AE with no target", spell_id);
			}
			bool affect_caster = !IsNPC();	//NPC AE spells do not affect the NPC caster
			entity_list.AEBardPulse(this, ae_center, spell_id, affect_caster);
			break;
		}

		case GroupSpell:
		{
			if(spell_target->IsGrouped()) {
				mlog(SPELLS__BARDS, "Bard Song Pulse: spell %d, Group targeting group of %s", spell_id, spell_target->GetName());
				Group *target_group = entity_list.GetGroupByMob(spell_target);
				if(target_group)
					target_group->GroupBardPulse(this, spell_id);
			}
			else if(spell_target->IsRaidGrouped() && spell_target->IsClient()) {
				mlog(SPELLS__BARDS, "Bard Song Pulse: spell %d, Raid group targeting raid group of %s", spell_id, spell_target->GetName());
				Raid *r = entity_list.GetRaidByClient(spell_target->CastToClient());
				if(r){
					uint32 gid = r->GetGroup(spell_target->GetName());
					if(gid < 12){
						r->GroupBardPulse(this, spell_id, gid);
					}
					else{
						BardPulse(spell_id, this);
#ifdef GROUP_BUFF_PETS
						if (GetPet() && HasPetAffinity() && !GetPet()->IsCharmed())
							GetPet()->BardPulse(spell_id, this);
#endif
					}
				}
			}
			else {
				mlog(SPELLS__BARDS, "Bard Song Pulse: spell %d, Group target without group. Affecting caster.", spell_id);
				BardPulse(spell_id, this);
#ifdef GROUP_BUFF_PETS
				if (GetPet() && HasPetAffinity() && !GetPet()->IsCharmed())
					GetPet()->BardPulse(spell_id, this);
#endif
			}
			break;
		}
	}

	//do we need to do this???
	DoAnim(spells[spell_id].CastingAnim, 0, true, IsClient() ? FilterPCSpells : FilterNPCSpells);
	if(IsClient())
		CastToClient()->CheckSongSkillIncrease(spell_id);

	return(true);
}

void Mob::BardPulse(uint16 spell_id, Mob *caster) {
	int buffs_i;
	uint32 buff_count = GetMaxTotalSlots();
	for (buffs_i = 0; buffs_i < buff_count; buffs_i++) {
		if(buffs[buffs_i].spellid != spell_id)
			continue;
		if(buffs[buffs_i].casterid != caster->GetID()) {
			mlog(SPELLS__BARDS, "Bard Pulse for %d: found buff from caster %d and we are pulsing for %d... are there two bards playing the same song???", spell_id, buffs[buffs_i].casterid, caster->GetID());
			return;
		}
		//extend the spell if it will expire before the next pulse
		if(buffs[buffs_i].ticsremaining <= 3) {
			buffs[buffs_i].ticsremaining += 3;
			mlog(SPELLS__BARDS, "Bard Song Pulse %d: extending duration in slot %d to %d tics", spell_id, buffs_i, buffs[buffs_i].ticsremaining);
		}

		//should we send this buff update to the client... seems like it would
		//be a lot of traffic for no reason...
//this may be the wrong packet...
		if(IsClient()) {
			EQApplicationPacket *packet = new EQApplicationPacket(OP_Action, sizeof(Action_Struct));

			Action_Struct* action = (Action_Struct*) packet->pBuffer;
			action->source = caster->GetID();
			action->target = GetID();
			action->spell = spell_id;
			action->sequence = (uint32) (GetHeading() * 2);	// just some random number
			action->instrument_mod = caster->GetInstrumentMod(spell_id);
			action->buff_unknown = 0;
			action->level = buffs[buffs_i].casterlevel;
			action->type = SpellDamageType;
			entity_list.QueueCloseClients(this, packet, false, 200, 0, true, IsClient() ? FilterPCSpells : FilterNPCSpells);

			action->buff_unknown = 4;

			if(IsEffectInSpell(spell_id, SE_TossUp))
			{
				action->buff_unknown = 0;
			}
			else if(spells[spell_id].pushback > 0 || spells[spell_id].pushup > 0)
			{
				if(IsClient())
				{
					if(HasBuffIcon(caster, this, spell_id) == false)
					{
						CastToClient()->SetKnockBackExemption(true);

						action->buff_unknown = 0;
						EQApplicationPacket* outapp_push = new EQApplicationPacket(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
						PlayerPositionUpdateServer_Struct* spu = (PlayerPositionUpdateServer_Struct*)outapp_push->pBuffer;

						double look_heading = caster->CalculateHeadingToTarget(GetX(), GetY());
						look_heading /= 256;
						look_heading *= 360;
						if(look_heading > 360)
							look_heading -= 360;

						//x and y are crossed mkay
						double new_x = spells[spell_id].pushback * sin(double(look_heading * 3.141592 / 180.0));
						double new_y = spells[spell_id].pushback * cos(double(look_heading * 3.141592 / 180.0));

						spu->spawn_id	= GetID();
						spu->x_pos		= FloatToEQ19(GetX());
						spu->y_pos		= FloatToEQ19(GetY());
						spu->z_pos		= FloatToEQ19(GetZ());
						spu->delta_x	= NewFloatToEQ13(new_x);
						spu->delta_y	= NewFloatToEQ13(new_y);
						spu->delta_z	= NewFloatToEQ13(spells[spell_id].pushup);
						spu->heading	= FloatToEQ19(GetHeading());
						spu->padding0002	=0;
						spu->padding0006	=7;
						spu->padding0014	=0x7f;
						spu->padding0018	=0x5df27;
						spu->animation = 0;
						spu->delta_heading = NewFloatToEQ13(0);
						outapp_push->priority = 6;
						entity_list.QueueClients(this, outapp_push, true);
						CastToClient()->FastQueuePacket(&outapp_push);
					}
				}
			}

			if(IsClient() && IsEffectInSpell(spell_id, SE_ShadowStep))
			{
				CastToClient()->SetShadowStepExemption(true);
			}

			if(!IsEffectInSpell(spell_id, SE_BindAffinity))
			{
				CastToClient()->QueuePacket(packet);
			}

			EQApplicationPacket *message_packet = new EQApplicationPacket(OP_Damage, sizeof(CombatDamage_Struct));
			CombatDamage_Struct *cd = (CombatDamage_Struct *)message_packet->pBuffer;
			cd->target = action->target;
			cd->source = action->source;
			cd->type = SpellDamageType;
			cd->spellid = action->spell;
			cd->sequence = action->sequence;
			cd->damage = 0;
			if(!IsEffectInSpell(spell_id, SE_BindAffinity))
			{
				entity_list.QueueCloseClients(this, message_packet, false, 200, 0, true, IsClient() ? FilterPCSpells : FilterNPCSpells);
			}
			safe_delete(message_packet);
			safe_delete(packet);

		}
		//we are done...
		return;
	}
	mlog(SPELLS__BARDS, "Bard Song Pulse %d: Buff not found, reapplying spell.", spell_id);
	//this spell is not affecting this mob, apply it.
	caster->SpellOnTarget(spell_id, this);
}

///////////////////////////////////////////////////////////////////////////////
// buff related functions

// returns how many _ticks_ the buff will last.
// a tick is 6 seconds
// this is the place to figure out random duration buffs like fear and charm.
// both the caster and target mobs are passed in, so different behavior can
// even be created depending on the types of mobs involved
//
// right now this is just an outline, working on this..
int Mob::CalcBuffDuration(Mob *caster, Mob *target, uint16 spell_id, int32 caster_level_override)
{
	int formula, duration;

	if(!IsValidSpell(spell_id) || (!caster && !target))
		return 0;

	if(!caster && !target)
		return 0;

	// if we have at least one, we can make do, we'll just pretend they're the same
	if(!caster)
		caster = target;
	if(!target)
		target = caster;

	formula = spells[spell_id].buffdurationformula;
	duration = spells[spell_id].buffduration;

	int castlevel = caster->GetCasterLevel(spell_id);
	if(caster_level_override > 0)
		castlevel = caster_level_override;

	int res = CalcBuffDuration_formula(castlevel, formula, duration);
	// Only need this for clients, since the change was for bard songs, I assume we should keep non bard songs getting +1
	// However if its bard or not and is mez, charm or fear, we need to add 1 so that client is in sync
	if(!IsShortDurationBuff(spell_id) ||
		IsFearSpell(spell_id) ||
		IsCharmSpell(spell_id) ||
		IsMezSpell(spell_id) ||
		IsBlindSpell(spell_id))
		res += 1;
	
	res = mod_buff_duration(res, caster, target, spell_id);

	mlog(SPELLS__CASTING, "Spell %d: Casting level %d, formula %d, base_duration %d: result %d",
		spell_id, castlevel, formula, duration, res);

	return(res);
}

// the generic formula calculations
int CalcBuffDuration_formula(int level, int formula, int duration)
{
	int i;	// temp variable

	switch(formula)
	{
		case 0:	// not a buff
			return 0;

		case 1:
			i = (int)ceil(level / 2.0f);
			return i < duration ? (i < 1 ? 1 : i) : duration;

		case 2:
			i = (int)ceil(duration / 5.0f * 3);
			return i < duration ? (i < 1 ? 1 : i) : duration;

		case 3:
			i = level * 30;
			return i < duration ? (i < 1 ? 1 : i) : duration;

		case 4:	// only used by 'LowerElement'
			return ((duration != 0) ? duration : 50);

		case 5:
			i = duration;
			// 0 value results in a 3 tick spell, else its between 1-3 ticks.
			return i < 3 ? (i < 1 ? 3 : i) : 3;

		case 6:
			i = (int)ceil(level / 2.0f);
			return i < duration ? (i < 1 ? 1 : i) : duration;

		case 7:
			i = level;
			return (duration == 0) ? (i < 1 ? 1 : i) : duration;

		case 8:
			i = level + 10;
			return i < duration ? (i < 1 ? 1 : i) : duration;

		case 9:
			i = level * 2 + 10;
			return i < duration ? (i < 1 ? 1 : i) : duration;

		case 10:
			i = level * 3 + 10;
			return i < duration ? (i < 1 ? 1 : i) : duration;

		case 11:
			return duration;

		case 12:
			return duration;

		case 15:	// Don't know what the real formula for this should be. Used by Skinspikes potion.
			return duration;

		case 50:	// lucy says this is unlimited?
			return 72000;	// 5 days

		case 3600:
			return duration ? duration : 3600;

		default:
			LogFile->write(EQEMuLog::Debug, "CalcBuffDuration_formula: unknown formula %d", formula);
			return 0;
	}
}

// helper function for AddBuff to determine stacking
// spellid1 is the spell already worn, spellid2 is the one trying to be cast
// returns:
// 0 if not the same type, no action needs to be taken
// 1 if spellid1 should be removed (overwrite)
// -1 if they can't stack and spellid2 should be stopped
//currently, a spell will not land if it would overwrite a better spell on any effect
//if all effects are better or the same, we overwrite, else we do nothing
int Mob::CheckStackConflict(uint16 spellid1, int caster_level1, uint16 spellid2, int caster_level2, Mob* caster1, Mob* caster2)
{
	const SPDat_Spell_Struct &sp1 = spells[spellid1];
	const SPDat_Spell_Struct &sp2 = spells[spellid2];

	int i, effect1, effect2, sp1_value, sp2_value;
	int blocked_effect, blocked_below_value, blocked_slot;
	int overwrite_effect, overwrite_below_value, overwrite_slot;

	mlog(SPELLS__STACKING, "Check Stacking on old %s (%d) @ lvl %d (by %s) vs. new %s (%d) @ lvl %d (by %s)", sp1.name, spellid1, caster_level1, (caster1==nullptr)?"Nobody":caster1->GetName(), sp2.name, spellid2, caster_level2, (caster2==nullptr)?"Nobody":caster2->GetName());

	if(((spellid1 == spellid2) && (spellid1 == 2751)) || //special case spells that will block each other no matter what
		((spellid1 == spellid2) && (spellid1 == 2755)) //manaburn / lifeburn
		){
			mlog(SPELLS__STACKING, "Blocking spell because manaburn/lifeburn does not stack with itself");
			return -1;
		}

	int modval = mod_spell_stack(spellid1, caster_level1, caster1, spellid2, caster_level2, caster2);
	if(modval < 2) { return(modval); }

	//resurrection effects wont count for overwrite/block stacking
	switch(spellid1)
	{
	case 756:
	case 757:
	case 5249:
		return (0);
	}

	switch (spellid2)
	{
	case 756:
	case 757:
	case 5249:
		return (0);
	}

	/*
	One of these is a bard song and one isn't and they're both beneficial so they should stack.
	*/
	if(IsBardSong(spellid1) != IsBardSong(spellid2))
	{
		if(!IsDetrimentalSpell(spellid1) && !IsDetrimentalSpell(spellid2))
		{
			mlog(SPELLS__STACKING, "%s and %s are beneficial, and one is a bard song, no action needs to be taken", sp1.name, sp2.name);
			return (0);
		}
	}


	// check for special stacking block command in spell1 against spell2
	for(i = 0; i < EFFECT_COUNT; i++)
	{
		effect1 = sp1.effectid[i];
		if(effect1 == SE_StackingCommand_Block)
		{
			/*
			The logic here is if you're comparing the same spells they can't block each other
			from refreshing
			*/
			if(spellid1 == spellid2)
				continue;

			blocked_effect = sp1.base[i];
			blocked_slot = sp1.formula[i] - 201;	//they use base 1 for slots, we use base 0
			blocked_below_value = sp1.max[i];

			if(sp2.effectid[blocked_slot] == blocked_effect)
			{
				sp2_value = CalcSpellEffectValue(spellid2, blocked_slot, caster_level2);

				mlog(SPELLS__STACKING, "%s (%d) blocks effect %d on slot %d below %d. New spell has value %d on that slot/effect. %s.",
					sp1.name, spellid1, blocked_effect, blocked_slot, blocked_below_value, sp2_value, (sp2_value < blocked_below_value)?"Blocked":"Not blocked");

				if(sp2_value < blocked_below_value)
				{
					mlog(SPELLS__STACKING, "Blocking spell because sp2_value < blocked_below_value");
					return -1;	// blocked
				}
			} else {
				mlog(SPELLS__STACKING, "%s (%d) blocks effect %d on slot %d below %d, but we do not have that effect on that slot. Ignored.",
					sp1.name, spellid1, blocked_effect, blocked_slot, blocked_below_value);
			}
		}
	}

	// check for special stacking overwrite in spell2 against effects in spell1
	for(i = 0; i < EFFECT_COUNT; i++)
	{
		effect2 = sp2.effectid[i];
		if(effect2 == SE_StackingCommand_Overwrite)
		{
			overwrite_effect = sp2.base[i];
			overwrite_slot = sp2.formula[i] - 201;	//they use base 1 for slots, we use base 0
			overwrite_below_value = sp2.max[i];
			if(sp1.effectid[overwrite_slot] == overwrite_effect)
			{
				sp1_value = CalcSpellEffectValue(spellid1, overwrite_slot, caster_level1);

				mlog(SPELLS__STACKING, "%s (%d) overwrites existing spell if effect %d on slot %d is below %d. Old spell has value %d on that slot/effect. %s.",
					sp2.name, spellid2, overwrite_effect, overwrite_slot, overwrite_below_value, sp1_value, (sp1_value < overwrite_below_value)?"Overwriting":"Not overwriting");

				if(sp1_value < overwrite_below_value)
				{
					mlog(SPELLS__STACKING, "Overwrite spell because sp1_value < overwrite_below_value");
					return 1;			// overwrite spell if its value is less
				}
			} else {
				mlog(SPELLS__STACKING, "%s (%d) overwrites existing spell if effect %d on slot %d is below %d, but we do not have that effect on that slot. Ignored.",
					sp2.name, spellid2, overwrite_effect, overwrite_slot, overwrite_below_value);

			}
		}
	}

	bool sp1_detrimental = IsDetrimentalSpell(spellid1);
	bool sp2_detrimental = IsDetrimentalSpell(spellid2);
	bool sp_det_mismatch;

	if(sp1_detrimental == sp2_detrimental)
		sp_det_mismatch = false;
	else
		sp_det_mismatch = true;

	// now compare matching effects
	// arbitration takes place if 2 spells have the same effect at the same
	// effect slot, otherwise they're stackable, even if it's the same effect
	bool will_overwrite = false;
	for(i = 0; i < EFFECT_COUNT; i++)
	{
		if(IsBlankSpellEffect(spellid1, i) || IsBlankSpellEffect(spellid2, i))
			continue;

		effect1 = sp1.effectid[i];
		effect2 = sp2.effectid[i];

		//Effects which really aren't going to affect stacking.
		if(effect1 == SE_CurrentHPOnce ||
			effect1 == SE_CurseCounter	||
			effect1 == SE_DiseaseCounter ||
			effect1 == SE_PoisonCounter){
			continue;
			}

		/*
		Quick check, are the effects the same, if so then
		keep going else ignore it for stacking purposes.
		*/
		if(effect1 != effect2)
			continue;

		/*
		Skip check if effect is SE_Limit*
		skip checking effect2 since we know they are equal
		*/
		if(effect1 == SE_LimitMaxLevel ||
			effect1 == SE_LimitResist ||
			effect1 == SE_LimitTarget ||
			effect1 == SE_LimitEffect ||
			effect1 == SE_LimitSpellType ||
			effect1 == SE_LimitSpell ||
			effect1 == SE_LimitMinDur ||
			effect1 == SE_LimitInstant ||
			effect1 == SE_LimitMinLevel ||
			effect1 == SE_LimitCastTime)
			continue;

		/*
		If target is a npc and caster1 and caster2 exist
		If Caster1 isn't the same as Caster2 and the effect is a DoT then ignore it.
		*/
		if(IsNPC() && caster1 && caster2 && caster1 != caster2) {
			if(effect1 == SE_CurrentHP && sp1_detrimental && sp2_detrimental) {
				continue;
				mlog(SPELLS__STACKING, "Both casters exist and are not the same, the effect is a detrimental dot, moving on");
			}
		}

		if(effect1 == SE_CompleteHeal){ //SE_CompleteHeal never stacks or overwrites ever, always block.
			mlog(SPELLS__STACKING, "Blocking spell because complete heal never stacks or overwries");
			return (-1);
		}

		/*
		If the effects are the same and
		sp1 = beneficial & sp2 = detrimental or
		sp1 = detrimental & sp2 = beneficial
		Then this effect should be ignored for stacking purposes.
		*/
		if(sp_det_mismatch)
		{
			mlog(SPELLS__STACKING, "The effects are the same but the spell types are not, passing the effect");
			continue;
		}

		/*
		If the spells aren't the same
		and the effect is a dot we can go ahead and stack it
		*/
		if(effect1 == SE_CurrentHP && spellid1 != spellid2 && sp1_detrimental && sp2_detrimental) {
			mlog(SPELLS__STACKING, "The spells are not the same and it is a detrimental dot, passing");
			continue;
		}

		sp1_value = CalcSpellEffectValue(spellid1, i, caster_level1);
		sp2_value = CalcSpellEffectValue(spellid2, i, caster_level2);

		// some spells are hard to compare just on value. attack speed spells
		// have a value that's a percentage for instance
		if
		(
			effect1 == SE_AttackSpeed ||
			effect1 == SE_AttackSpeed2 ||
			effect1 == SE_AttackSpeed3
		)
		{
			sp1_value -= 100;
			sp2_value -= 100;
		}

		if(sp1_value < 0)
			sp1_value = 0 - sp1_value;
		if(sp2_value < 0)
			sp2_value = 0 - sp2_value;

		if(sp2_value < sp1_value) {
			mlog(SPELLS__STACKING, "Spell %s (value %d) is not as good as %s (value %d). Rejecting %s.",
				sp2.name, sp2_value, sp1.name, sp1_value, sp2.name);
			return -1;	// can't stack
		}
		//we dont return here... a better value on this one effect dosent mean they are
		//all better...

		mlog(SPELLS__STACKING, "Spell %s (value %d) is not as good as %s (value %d). We will overwrite %s if there are no other conflicts.",
			sp1.name, sp1_value, sp2.name, sp2_value, sp1.name);
		will_overwrite = true;
	}

	//if we get here, then none of the values on the new spell are "worse"
	//so now we see if this new spell is any better, or if its not related at all
	if(will_overwrite) {
		mlog(SPELLS__STACKING, "Stacking code decided that %s should overwrite %s.", sp2.name, sp1.name);
		return(1);
	}

	mlog(SPELLS__STACKING, "Stacking code decided that %s is not affected by %s.", sp2.name, sp1.name);
	return 0;
}


// returns the slot the buff was added to, -1 if it wasn't added due to
// stacking problems, and -2 if this is not a buff
// if caster is null, the buff will be added with the caster level being
// the level of the mob
int Mob::AddBuff(Mob *caster, uint16 spell_id, int duration, int32 level_override)
{

	int buffslot, ret, caster_level, emptyslot = -1;
	bool will_overwrite = false;
	std::vector<int> overwrite_slots;

	if(level_override > 0)
		caster_level = level_override;
	else
		caster_level = caster ? caster->GetCasterLevel(spell_id) : GetCasterLevel(spell_id);

	if(duration == 0)
	{
		duration = CalcBuffDuration(caster, this, spell_id);

		if(caster)
			duration = caster->GetActSpellDuration(spell_id, duration);
	}

	if(duration == 0) {
		mlog(SPELLS__BUFFS, "Buff %d failed to add because its duration came back as 0.", spell_id);
		return -2;	// no duration? this isn't a buff
	}

	mlog(SPELLS__BUFFS, "Trying to add buff %d cast by %s (cast level %d) with duration %d",
		spell_id, caster?caster->GetName():"UNKNOWN", caster_level, duration);

	// first we loop through everything checking that the spell
	// can stack with everything. this is to avoid stripping the spells
	// it would overwrite, and then hitting a buff we can't stack with.
	// we also check if overwriting will occur. this is so after this loop
	// we can determine if there will be room for this buff
	uint32 buff_count = GetMaxTotalSlots();
	uint32 start_slot = 0;
	uint32 end_slot = 0;
	if(IsDisciplineBuff(spell_id))
	{
		start_slot = GetMaxBuffSlots() + GetMaxSongSlots();
		end_slot = start_slot + GetCurrentDiscSlots();
	}
	else if(spells[spell_id].short_buff_box)
	{
		start_slot = GetMaxBuffSlots();
		end_slot = start_slot + GetCurrentSongSlots();
	}
	else
	{
		start_slot = 0;
		end_slot = GetCurrentBuffSlots();
	}

	for(buffslot = 0; buffslot < buff_count; buffslot++)
	{
		const Buffs_Struct &curbuf = buffs[buffslot];

		if(curbuf.spellid != SPELL_UNKNOWN)
		{
			// there's a buff in this slot
			ret = CheckStackConflict(curbuf.spellid, curbuf.casterlevel, spell_id, caster_level, entity_list.GetMobID(curbuf.casterid), caster);
			if(ret == -1) {	// stop the spell
				mlog(SPELLS__BUFFS, "Adding buff %d failed: stacking prevented by spell %d in slot %d with caster level %d", spell_id, curbuf.spellid, buffslot, curbuf.casterlevel);
				return -1;
			}
			if(ret == 1) {	// set a flag to indicate that there will be overwriting
				mlog(SPELLS__BUFFS, "Adding buff %d will overwrite spell %d in slot %d with caster level %d", spell_id, curbuf.spellid, buffslot, curbuf.casterlevel);
				will_overwrite = true;
				overwrite_slots.push_back(buffslot);
			}
		}
		else
		{
			if(emptyslot == -1)
			{
				if(buffslot >= start_slot && buffslot < end_slot)
				{
					emptyslot = buffslot;
				}
			}
		}
	}

	// we didn't find an empty slot to put it in, and it's not overwriting
	// anything so there must not be any room left.
	if(emptyslot == -1 && !will_overwrite)
	//	return -1;
	{
		if(IsDetrimentalSpell(spell_id)) //Sucks to be you, bye bye one of your buffs
		{
			for(buffslot = 0; buffslot < buff_count; buffslot++)
			{
				const Buffs_Struct &curbuf = buffs[buffslot];
				if(IsBeneficialSpell(curbuf.spellid))
				{
					mlog(SPELLS__BUFFS, "No slot for detrimental buff %d, so we are overwriting a beneficial buff %d in slot %d", spell_id, curbuf.spellid, buffslot);
					BuffFadeBySlot(buffslot,false);
					emptyslot = buffslot;
					break;
				}
			}
			if(emptyslot == -1) {
				mlog(SPELLS__BUFFS, "Unable to find a buff slot for detrimental buff %d", spell_id);
				return(-1);
			}
		}
		else {
			mlog(SPELLS__BUFFS, "Unable to find a buff slot for beneficial buff %d", spell_id);
			return -1;
		}
	}

	// at this point we know that this buff will stick, but we have
	// to remove some other buffs already worn if will_overwrite is true
	if(will_overwrite)
	{
		std::vector<int>::iterator cur, end;
		cur = overwrite_slots.begin();
		end = overwrite_slots.end();
		for(; cur != end; cur++) {
			// strip spell
			BuffFadeBySlot(*cur, false);

			// if we hadn't found a free slot before, or if this is earlier
			// we use it
			if(emptyslot == -1 || *cur < emptyslot)
				emptyslot = *cur;
		}
	}

	// now add buff at emptyslot
	assert(buffs[emptyslot].spellid == SPELL_UNKNOWN);	// sanity check

	buffs[emptyslot].spellid = spell_id;
	buffs[emptyslot].casterlevel = caster_level;
	if(caster && caster->IsClient()) {
		strcpy(buffs[emptyslot].caster_name, caster->GetName());
	} else {
		memset(buffs[emptyslot].caster_name, 0, 64);
	}
	buffs[emptyslot].casterid = caster ? caster->GetID() : 0;
	buffs[emptyslot].ticsremaining = duration;
	buffs[emptyslot].counters = CalculateCounters(spell_id);
	buffs[emptyslot].numhits = spells[spell_id].numhits;
	buffs[emptyslot].client = caster ? caster->IsClient() : 0;
	buffs[emptyslot].persistant_buff = 0;
	buffs[emptyslot].deathsaveCasterAARank = 0;
	buffs[emptyslot].deathSaveSuccessChance = 0;

	if(level_override > 0)
	{
		buffs[emptyslot].UpdateClient = true;
	}
	else{
		if(buffs[emptyslot].ticsremaining > (1+CalcBuffDuration_formula(caster_level, spells[spell_id].buffdurationformula, spells[spell_id].buffduration)))
			buffs[emptyslot].UpdateClient = true;
	}

	mlog(SPELLS__BUFFS, "Buff %d added to slot %d with caster level %d", spell_id, emptyslot, caster_level);
	if(IsPet() && GetOwner() && GetOwner()->IsClient()) {
		SendPetBuffsToClient();
	}


	if((IsClient() && !CastToClient()->GetPVP()) || (IsPet() && GetOwner() && GetOwner()->IsClient() && !GetOwner()->CastToClient()->GetPVP()) ||
				(IsMerc() && GetOwner() && GetOwner()->IsClient() && !GetOwner()->CastToClient()->GetPVP()))
	{
		EQApplicationPacket *outapp = MakeBuffsPacket();

		entity_list.QueueClientsByTarget(this, outapp, false, nullptr, true, false, BIT_SoDAndLater);

		if(GetTarget() == this) {
			CastToClient()->QueuePacket(outapp);
		}

		safe_delete(outapp);
	}

	// recalculate bonuses since we stripped/added buffs
	CalcBonuses();

	return emptyslot;
}

// used by some MobAI stuff
// NOT USED BY SPELL CODE
// note that this should not be used for determining which slot to place a
// buff into
// returns -1 on stack failure, -2 if all slots full, the slot number if the buff should overwrite another buff, or a free buff slot
int Mob::CanBuffStack(uint16 spellid, uint8 caster_level, bool iFailIfOverwrite)
{
	int i, ret, firstfree = -2;

	mlog(AI__BUFFS, "Checking if buff %d cast at level %d can stack on me.%s", spellid, caster_level, iFailIfOverwrite?" failing if we would overwrite something":"");

	uint32 buff_count = GetMaxTotalSlots();
	for (i=0; i < buff_count; i++)
	{
		const Buffs_Struct &curbuf = buffs[i];

		// no buff in this slot
		if (curbuf.spellid == SPELL_UNKNOWN)
		{
			// if we haven't found a free slot, this is the first one so save it
			if(firstfree == -2)
				firstfree = i;
			continue;
		}

		if(curbuf.spellid == spellid)
			return(-1);	//do not recast a buff we already have on, we recast fast enough that we dont need to refresh our buffs

		// there's a buff in this slot
		ret = CheckStackConflict(curbuf.spellid, curbuf.casterlevel, spellid, caster_level);
		if(ret == 1) {
			// should overwrite current slot
			if(iFailIfOverwrite) {
				mlog(AI__BUFFS, "Buff %d would overwrite %d in slot %d, reporting stack failure", spellid, curbuf.spellid, i);
				return(-1);
			}
			if(firstfree == -2)
				firstfree = i;
		}
		if(ret == -1) {
			mlog(AI__BUFFS, "Buff %d would conflict with %d in slot %d, reporting stack failure", spellid, curbuf.spellid, i);
			return -1;	// stop the spell, can't stack it
		}
	}

	mlog(AI__BUFFS, "Reporting that buff %d could successfully be placed into slot %d", spellid, firstfree);

	return firstfree;
}

///////////////////////////////////////////////////////////////////////////////
// spell effect related functions
//
// this is actually applying a spell cast from 'this' on 'spelltar'
// it performs pvp checking and applies resists, etc then it
// passes it to SpellEffect which causes effects to the target
//
// this is called by these functions:
// Mob::SpellFinished
// Entity::AESpell (called by Mob::SpellFinished)
// Group::CastGroupSpell (called by Mob::SpellFinished)
//
// also note you can't interrupt the spell here. at this point it's going
// and if you don't want effects just return false. interrupting here will
// break stuff
//
bool Mob::SpellOnTarget(uint16 spell_id, Mob* spelltar, bool reflect, bool use_resist_adjust, int16 resist_adjust, bool isproc)
{

	// well we can't cast a spell on target without a target
	if(!spelltar)
	{
		mlog(SPELLS__CASTING_ERR, "Unable to apply spell %d without a target", spell_id);
		Message(13, "SOT: You must have a target for this spell.");
		return false;
	}

	if(spelltar->IsClient() && spelltar->CastToClient()->IsHoveringForRespawn())
		return false;

	if(IsDetrimentalSpell(spell_id) && !IsAttackAllowed(spelltar) && !IsResurrectionEffects(spell_id)) {
		if(!IsClient() || !CastToClient()->GetGM()) {
			Message_StringID(MT_Shout, SPELL_NO_HOLD);
			return false;
		}
	}

	if(RuleB(Spells, EnableBlockedBuffs))
	{
		if(spelltar->IsBlockedBuff(spell_id))
		{
			mlog(SPELLS__BUFFS, "Spell %i not applied to %s as it is a Blocked Buff.", spell_id, spelltar->GetName());
			return false;
		}

		if(spelltar->IsPet() && spelltar->GetOwner() && spelltar->GetOwner()->IsBlockedPetBuff(spell_id))
		{
			mlog(SPELLS__BUFFS, "Spell %i not applied to %s (%s's pet) as it is a Pet Blocked Buff.", spell_id, spelltar->GetName(),
						spelltar->GetOwner()->GetName());
			return false;
		}
	}

	EQApplicationPacket *action_packet, *message_packet;
	float spell_effectiveness;

	if(!IsValidSpell(spell_id))
		return false;

	uint16 caster_level = GetCasterLevel(spell_id);

	mlog(SPELLS__CASTING, "Casting spell %d on %s with effective caster level %d", spell_id, spelltar->GetName(), caster_level);

	// Actual cast action - this causes the caster animation and the particles
	// around the target
	// we do this first, that way we get the particles even if the spell
	// doesn't land due to pvp protection
	// note: this packet is sent again if the spell is successful, with a flag
	// set
	action_packet = new EQApplicationPacket(OP_Action, sizeof(Action_Struct));
	Action_Struct* action = (Action_Struct*) action_packet->pBuffer;

	// select source
	if(IsClient() && CastToClient()->GMHideMe())
	{
		action->source = spelltar->GetID();
	}
	else
	{
		action->source = GetID();
		// this is a hack that makes detrimental buffs work client to client
		// TODO figure out how to do this right
		if
		(
			IsDetrimentalSpell(spell_id) &&
			IsClient() &&
			spelltar->IsClient()
		)
		{
			action->source = spelltar->GetID();
		}
	}

	// select target
	if	// Bind Sight line of spells
	(
		spell_id == 500 ||	// bind sight
		spell_id == 407		// cast sight
	)
	{
		action->target = GetID();
	}
	else
	{
		action->target = spelltar->GetID();
	}

	action->level = caster_level;	// caster level, for animation only
	action->type = 231;	// 231 means a spell
	action->spell = spell_id;
	action->sequence = (uint32) (GetHeading() * 2);	// just some random number
	action->instrument_mod = GetInstrumentMod(spell_id);
	action->buff_unknown = 0;

	if(spelltar != this && spelltar->IsClient())	// send to target
		spelltar->CastToClient()->QueuePacket(action_packet);
	if(IsClient())	// send to caster
		CastToClient()->QueuePacket(action_packet);
	// send to people in the area, ignoring caster and target
	entity_list.QueueCloseClients(spelltar, action_packet, true, 200, this, true, spelltar->IsClient() ? FilterPCSpells : FilterNPCSpells);


	/* Send the EVENT_CAST_ON event */
	if(spelltar->IsNPC())
	{
		char temp1[100];
		sprintf(temp1, "%d", spell_id);
		parse->EventNPC(EVENT_CAST_ON, spelltar->CastToNPC(), this, temp1, 0);
	}

	mod_spell_cast(spell_id, spelltar, reflect, use_resist_adjust, resist_adjust, isproc);

	// now check if the spell is allowed to land

	// invuln mobs can't be affected by any spells, good or bad
	if(spelltar->GetInvul() || spelltar->DivineAura()) {
		mlog(SPELLS__CASTING_ERR, "Casting spell %d on %s aborted: they are invulnerable.", spell_id, spelltar->GetName());
		safe_delete(action_packet);
		return false;
	}

	//cannot hurt untargetable mobs
	bodyType bt = spelltar->GetBodyType();
	if(bt == BT_NoTarget || bt == BT_NoTarget2) {
		if (RuleB(Pets, UnTargetableSwarmPet)) {
			if (spelltar->IsNPC()) {
				if (!spelltar->CastToNPC()->GetSwarmOwner()) {
					mlog(SPELLS__CASTING_ERR, "Casting spell %d on %s aborted: they are untargetable", spell_id, spelltar->GetName());
					safe_delete(action_packet);
					return(false);
				}
			} else {
				mlog(SPELLS__CASTING_ERR, "Casting spell %d on %s aborted: they are untargetable", spell_id, spelltar->GetName());
				safe_delete(action_packet);
				return(false);
			}
		} else {
			mlog(SPELLS__CASTING_ERR, "Casting spell %d on %s aborted: they are untargetable", spell_id, spelltar->GetName());
			safe_delete(action_packet);
			return(false);
		}
	}

	// Prevent double invising, which made you uninvised
	// Not sure if all 3 should be stacking
	if(IsEffectInSpell(spell_id, SE_Invisibility))
	{
		if(spelltar->invisible)
		{
			spelltar->Message_StringID(MT_Shout, ALREADY_INVIS, GetCleanName());
			safe_delete(action_packet);
			return false;
		}
	}

	if(IsEffectInSpell(spell_id, SE_InvisVsUndead))
	{
		if(spelltar->invisible_undead)
		{
			spelltar->Message_StringID(MT_Shout, ALREADY_INVIS, GetCleanName());
			safe_delete(action_packet);
			return false;
		}
	}

	if(IsEffectInSpell(spell_id, SE_InvisVsAnimals))
	{
		if(spelltar->invisible_animals)
		{
			spelltar->Message_StringID(MT_Shout, ALREADY_INVIS, GetCleanName());
			safe_delete(action_packet);
			return false;
		}
	}

	if(!(IsClient() && CastToClient()->GetGM()) && !IsHarmonySpell(spell_id))	// GMs can cast on anything
	{
		// Beneficial spells check
		if(IsBeneficialSpell(spell_id))
		{
			if(IsClient() &&	//let NPCs do beneficial spells on anybody if they want, should be the job of the AI, not the spell code to prevent this from going wrong
				spelltar != this)
			{

				Client* pClient = 0;
				Raid* pRaid = 0;
				Group* pBasicGroup = 0;
				uint32 nGroup = 0; //raid group

				Client* pClientTarget = 0;
				Raid* pRaidTarget = 0;
				Group* pBasicGroupTarget = 0;
				uint32 nGroupTarget = 0; //raid group

				Client* pClientTargetPet = 0;
				Raid* pRaidTargetPet = 0;
				Group* pBasicGroupTargetPet = 0;
				uint32 nGroupTargetPet = 0; //raid group

				const uint32 cnWTF = 0xFFFFFFFF + 1; //this should be zero unless on 64bit? forced uint64?

				//Caster client pointers
				pClient = this->CastToClient();
				pRaid = entity_list.GetRaidByClient(pClient);
				pBasicGroup = entity_list.GetGroupByMob(this);
				if(pRaid)
					nGroup = pRaid->GetGroup(pClient) + 1;

				//Target client pointers
				if(spelltar->IsClient())
				{
					pClientTarget = spelltar->CastToClient();
					pRaidTarget = entity_list.GetRaidByClient(pClientTarget);
					pBasicGroupTarget = entity_list.GetGroupByMob(spelltar);
					if(pRaidTarget)
						nGroupTarget = pRaidTarget->GetGroup(pClientTarget) + 1;
				}

				if(spelltar->IsPet())
				{
					Mob *owner = spelltar->GetOwner();
					if(owner->IsClient())
					{
						pClientTargetPet = owner->CastToClient();
						pRaidTargetPet = entity_list.GetRaidByClient(pClientTargetPet);
						pBasicGroupTargetPet = entity_list.GetGroupByMob(owner);
						if(pRaidTargetPet)
							nGroupTargetPet = pRaidTargetPet->GetGroup(pClientTargetPet) + 1;
					}

				}


				if(!IsBeneficialAllowed(spelltar) ||
					(IsGroupOnlySpell(spell_id) &&
						!(
							(pBasicGroup && ((pBasicGroup == pBasicGroupTarget) || (pBasicGroup == pBasicGroupTargetPet))) || //Basic Group

							((nGroup != cnWTF) && ((nGroup == nGroupTarget) || (nGroup == nGroupTargetPet))) || //Raid group

							(spelltar == GetPet()) //should be able to cast grp spells on self and pet despite grped status.
						)
					)
				)
				{
					if(spells[spell_id].targettype == ST_AEBard) {
						//if it was a beneficial AE bard song don't spam the window that it would not hold
						mlog(SPELLS__CASTING_ERR, "Beneficial ae bard song %d can't take hold %s -> %s, IBA? %d", spell_id, GetName(), spelltar->GetName(), IsBeneficialAllowed(spelltar));
					} else {
						mlog(SPELLS__CASTING_ERR, "Beneficial spell %d can't take hold %s -> %s, IBA? %d", spell_id, GetName(), spelltar->GetName(), IsBeneficialAllowed(spelltar));
						Message_StringID(MT_Shout, SPELL_NO_HOLD);
					}
					safe_delete(action_packet);
					return false;
				}
			}
		}
		else if	( !IsAttackAllowed(spelltar, true) && !IsResurrectionEffects(spell_id)) // Detrimental spells - PVP check
		{
			mlog(SPELLS__CASTING_ERR, "Detrimental spell %d can't take hold %s -> %s", spell_id, GetName(), spelltar->GetName());
			spelltar->Message_StringID(MT_Shout, YOU_ARE_PROTECTED, GetCleanName());
			safe_delete(action_packet);
			return false;
		}
	}

	// ok at this point the spell is permitted to affect the target,
	// but we need to check special cases and resists


	// check immunities
	if(spelltar->IsImmuneToSpell(spell_id, this))
	{
		//the above call does the message to the client if needed
		mlog(SPELLS__RESISTS, "Spell %d can't take hold due to immunity %s -> %s", spell_id, GetName(), spelltar->GetName());
		safe_delete(action_packet);
		return false;
	}

	//check for AE_Undead
	if(spells[spell_id].targettype == ST_UndeadAE){
		if(spelltar->GetBodyType() != BT_SummonedUndead &&
			spelltar->GetBodyType() != BT_Undead &&
			spelltar->GetBodyType() != BT_Vampire)
		{
			return false;
		}
	}
	// Block next spell effect should be used up first(since its blocking the next spell)
	if(CanBlockSpell()) {
		uint32 buff_count = GetMaxTotalSlots();
		int focus = 0;
		for (int b=0; b < buff_count; b++) {
			if(IsEffectInSpell(buffs[b].spellid, SE_BlockNextSpellFocus)) {
				focus = CalcFocusEffect(focusBlockNextSpell, buffs[b].spellid, spell_id);
				if(focus) {
					CheckHitsRemaining(b);
					Message_StringID(MT_Shout, SPELL_WOULDNT_HOLD);
					return false;
				}
			}
		}
	}
	// Reflect
	if(TryReflectSpell(spell_id) && spelltar && !reflect && IsDetrimentalSpell(spell_id) && this != spelltar) {
		int reflect_chance = 0;
		switch(RuleI(Spells, ReflectType))
		{
			case 0:
				break;

			case 1:
			{
				if(spells[spell_id].targettype == ST_Target) {
					for(int y = 0; y < 16; y++) {
						if(spells[spell_id].classes[y] < 255)
							reflect_chance = 1;
					}
				}
				break;
			}
			case 2:
			{
				for(int y = 0; y < 16; y++) {
					if(spells[spell_id].classes[y] < 255)
						reflect_chance = 1;
				}
				break;
			}
			case 3:
			{
				if(spells[spell_id].targettype == ST_Target)
					reflect_chance = 1;

				break;
			}
			case 4:
				reflect_chance = 1;

			default:
				break;
		}
		if(reflect_chance) {
			Message_StringID(MT_Spells, SPELL_REFLECT, GetCleanName(), spelltar->GetCleanName());
			SpellOnTarget(spell_id, this, true, use_resist_adjust, resist_adjust);
			return false;
		}
	}

	// resist check - every spell can be resisted, beneficial or not
	// add: ok this isn't true, eqlive's spell data is fucked up, buffs are
	// not all unresistable, so changing this to only check certain spells
	if(IsResistableSpell(spell_id))
	{
		spell_effectiveness = spelltar->ResistSpell(spells[spell_id].resisttype, spell_id, this, use_resist_adjust, resist_adjust);
		if(spell_effectiveness < 100)
		{
			if(spell_effectiveness == 0 || !IsPartialCapableSpell(spell_id) )
			{
				mlog(SPELLS__RESISTS, "Spell %d was completely resisted by %s", spell_id, spelltar->GetName());
				Message_StringID(MT_SpellFailure, TARGET_RESISTED, spells[spell_id].name);
				spelltar->Message_StringID(MT_SpellFailure, YOU_RESIST, spells[spell_id].name);

				if(spelltar->IsAIControlled()){
					int32 aggro = CheckAggroAmount(spell_id);
					if(aggro > 0) {
						if(!IsHarmonySpell(spell_id))
						spelltar->AddToHateList(this, aggro);
						else
							if(!PassCharismaCheck(this, spelltar, spell_id))
								spelltar->AddToHateList(this, aggro);
					}
					else{
						int32 newhate = spelltar->GetHateAmount(this) + aggro;
						if (newhate < 1) {
							spelltar->SetHate(this,1);
						} else {
							spelltar->SetHate(this,newhate);
						}
					}
				}

				safe_delete(action_packet);
				return false;
			}
		}
	}
	else
	{
		spell_effectiveness = 100;
	}

		// Recourse means there is a spell linked to that spell in that the recourse spell will
		// be automatically casted on the casters group or the caster only depending on Targettype
		// this is for things like dark empathy, shadow vortex
		int recourse_spell=0;
		recourse_spell = spells[spell_id].RecourseLink;
		if(recourse_spell)
		{
			if(spells[recourse_spell].targettype == ST_Group || spells[recourse_spell].targettype == ST_GroupTeleport)
			{
				if(IsGrouped())
				{
					Group *g = entity_list.GetGroupByMob(this);
					if(g)
						g->CastGroupSpell(this, recourse_spell);
					else{
						SpellOnTarget(recourse_spell, this);
#ifdef GROUP_BUFF_PETS
						if (GetPet())
							SpellOnTarget(recourse_spell, GetPet());
#endif
					}
				}
				else if(IsRaidGrouped() && IsClient())
				{
					Raid *r = entity_list.GetRaidByClient(CastToClient());
					uint32 gid = 0xFFFFFFFF;
					if(r)
						gid = r->GetGroup(GetName());
					else
						gid = 13;	// Forces ungrouped spell casting

					if(gid < 12)
					{
						r->CastGroupSpell(this, recourse_spell, gid);
					}
					else{
						SpellOnTarget(recourse_spell, this);
#ifdef GROUP_BUFF_PETS
						if (GetPet())
							SpellOnTarget(recourse_spell, GetPet());
#endif
					}
				}
				else if(HasOwner())
				{
					if(GetOwner()->IsGrouped())
					{
						Group *g = entity_list.GetGroupByMob(GetOwner());
						if(g)
							g->CastGroupSpell(this, recourse_spell);
						else{
							SpellOnTarget(recourse_spell, GetOwner());
							SpellOnTarget(recourse_spell, this);
						}
					}
					else if(GetOwner()->IsRaidGrouped() && GetOwner()->IsClient())
					{
						Raid *r = entity_list.GetRaidByClient(GetOwner()->CastToClient());
						uint32 gid = 0xFFFFFFFF;
						if(r)
							gid = r->GetGroup(GetOwner()->GetName());
						else
							gid = 13;	// Forces ungrouped spell casting

						if(gid < 12)
						{
							r->CastGroupSpell(this, recourse_spell, gid);
						}
						else
						{
							SpellOnTarget(recourse_spell, GetOwner());
							SpellOnTarget(recourse_spell, this);
						}
					}
					else
					{
						SpellOnTarget(recourse_spell, GetOwner());
						SpellOnTarget(recourse_spell, this);
					}
				}
				else
				{
					SpellOnTarget(recourse_spell, this);
#ifdef GROUP_BUFF_PETS
					if (GetPet())
						SpellOnTarget(recourse_spell, GetPet());
#endif
				}

			}
			else
			{
				SpellOnTarget(recourse_spell, this);
			}
		}

		if(spelltar->spellbonuses.SpellDamageShield && IsDetrimentalSpell(spell_id)){
			spelltar->DamageShield(this, true);
			spelltar->CheckHitsRemaining(0, false, false, SE_DamageShield);
		}

	TrySpellTrigger(spelltar, spell_id);
	TryApplyEffect(spelltar, spell_id);

	if (spelltar->IsAIControlled() && IsDetrimentalSpell(spell_id) && !IsHarmonySpell(spell_id)) {
		int32 aggro_amount = CheckAggroAmount(spell_id, isproc);
		mlog(SPELLS__CASTING, "Spell %d cast on %s generated %d hate", spell_id, spelltar->GetName(), aggro_amount);
		if(aggro_amount > 0)
			spelltar->AddToHateList(this, aggro_amount);		else{
			int32 newhate = spelltar->GetHateAmount(this) + aggro_amount;
			if (newhate < 1) {
				spelltar->SetHate(this,1);
			} else {
				spelltar->SetHate(this,newhate);
			}
		}
	}
	else if (IsBeneficialSpell(spell_id) && !IsSummonPCSpell(spell_id))
		entity_list.AddHealAggro(spelltar, this, CheckHealAggroAmount(spell_id, (spelltar->GetMaxHP() - spelltar->GetHP())));

	// cause the effects to the target
	if(!spelltar->SpellEffect(this, spell_id, spell_effectiveness))
	{
		// if SpellEffect returned false there's a problem applying the
		// spell. It's most likely a buff that can't stack.
		mlog(SPELLS__CASTING_ERR, "Spell %d could not apply its effects %s -> %s\n", spell_id, GetName(), spelltar->GetName());
		Message_StringID(MT_Shout, SPELL_NO_HOLD);
		safe_delete(action_packet);
		return false;
	}

	// send the action packet again now that the spell is successful
	// NOTE: this is what causes the buff icon to appear on the client, if
	// this is a buff - but it sortof relies on the first packet.
	// the complete sequence is 2 actions and 1 damage message
	action->buff_unknown = 0x04;	// this is a success flag

	if(IsEffectInSpell(spell_id, SE_TossUp))
	{
		action->buff_unknown = 0;
	}
	else if(spells[spell_id].pushback > 0 || spells[spell_id].pushup > 0)
	{
		if(spelltar->IsClient())
		{
			if(HasBuffIcon(this, spelltar, spell_id) == false)
			{
				spelltar->CastToClient()->SetKnockBackExemption(true);

				action->buff_unknown = 0;
				EQApplicationPacket* outapp_push = new EQApplicationPacket(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
				PlayerPositionUpdateServer_Struct* spu = (PlayerPositionUpdateServer_Struct*)outapp_push->pBuffer;

				double look_heading = CalculateHeadingToTarget(spelltar->GetX(), spelltar->GetY());
				look_heading /= 256;
				look_heading *= 360;
				if(look_heading > 360)
					look_heading -= 360;

				//x and y are crossed mkay
				double new_x = spells[spell_id].pushback * sin(double(look_heading * 3.141592 / 180.0));
				double new_y = spells[spell_id].pushback * cos(double(look_heading * 3.141592 / 180.0));

				spu->spawn_id	= spelltar->GetID();
				spu->x_pos		= FloatToEQ19(spelltar->GetX());
				spu->y_pos		= FloatToEQ19(spelltar->GetY());
				spu->z_pos		= FloatToEQ19(spelltar->GetZ());
				spu->delta_x	= NewFloatToEQ13(new_x);
				spu->delta_y	= NewFloatToEQ13(new_y);
				spu->delta_z	= NewFloatToEQ13(spells[spell_id].pushup);
				spu->heading	= FloatToEQ19(spelltar->GetHeading());
				spu->padding0002	=0;
				spu->padding0006	=7;
				spu->padding0014	=0x7f;
				spu->padding0018	=0x5df27;
				spu->animation = 0;
				spu->delta_heading = NewFloatToEQ13(0);
				outapp_push->priority = 6;
				entity_list.QueueClients(this, outapp_push, true);
				spelltar->CastToClient()->FastQueuePacket(&outapp_push);
			}
		}
	}

	if(spelltar->IsClient() && IsEffectInSpell(spell_id, SE_ShadowStep))
	{
		spelltar->CastToClient()->SetShadowStepExemption(true);
	}

	if(!IsEffectInSpell(spell_id, SE_BindAffinity))
	{
		if(spelltar != this && spelltar->IsClient())	// send to target
			spelltar->CastToClient()->QueuePacket(action_packet);
		if(IsClient())	// send to caster
			CastToClient()->QueuePacket(action_packet);
	}
	// send to people in the area, ignoring caster and target
	//live dosent send this to anybody but the caster
	//entity_list.QueueCloseClients(spelltar, action_packet, true, 200, this, true, spelltar->IsClient() ? FILTER_PCSPELLS : FILTER_NPCSPELLS);

	// TEMPORARY - this is the message for the spell.
	// double message on effects that use ChangeHP - working on this
	message_packet = new EQApplicationPacket(OP_Damage, sizeof(CombatDamage_Struct));
	CombatDamage_Struct *cd = (CombatDamage_Struct *)message_packet->pBuffer;
	cd->target = action->target;
	cd->source = action->source;
	cd->type = action->type;
	cd->spellid = action->spell;
	cd->sequence = action->sequence;
	cd->damage = 0;
	if(!IsEffectInSpell(spell_id, SE_BindAffinity))
	{
		entity_list.QueueCloseClients(spelltar, message_packet, false, 200, 0, true, spelltar->IsClient() ? FilterPCSpells : FilterNPCSpells);
	}
	safe_delete(action_packet);
	safe_delete(message_packet);

	mlog(SPELLS__CASTING, "Cast of %d by %s on %s complete successfully.", spell_id, GetName(), spelltar->GetName());

	return true;
}

void Corpse::CastRezz(uint16 spellid, Mob* Caster)
{
	_log(SPELLS__REZ, "Corpse::CastRezz spellid %i, Rezzed() is %i, rezzexp is %i", spellid,Rezzed(),rezzexp);

	if(Rezzed()){
		if(Caster && Caster->IsClient())
			Caster->Message(13,"This character has already been resurrected.");

		return;
	}
	/*
	if(!can_rez) {
		if(Caster && Caster->IsClient())
			Caster->Message_StringID(0, CORPSE_TOO_OLD);
		return;
	}
	*/

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_RezzRequest, sizeof(Resurrect_Struct));
	Resurrect_Struct* rezz = (Resurrect_Struct*) outapp->pBuffer;
	// Why are we truncating these names to 30 characters ?
	memcpy(rezz->your_name,this->orgname,30);
	memcpy(rezz->corpse_name,this->name,30);
	memcpy(rezz->rezzer_name,Caster->GetName(),30);
	rezz->zone_id = zone->GetZoneID();
	rezz->instance_id = zone->GetInstanceID();
	rezz->spellid = spellid;
	rezz->x = this->x_pos;
	rezz->y = this->y_pos;
	rezz->z = this->z_pos;
	rezz->unknown000 = 0x00000000;
	rezz->unknown020 = 0x00000000;
	rezz->unknown088 = 0x00000000;
	// We send this to world, because it needs to go to the player who may not be in this zone.
	worldserver.RezzPlayer(outapp, rezzexp, dbid, OP_RezzRequest);
	_pkt(SPELLS__REZ, outapp);
	safe_delete(outapp);
}

bool Mob::FindBuff(uint16 spellid)
{
	int i;

	uint32 buff_count = GetMaxTotalSlots();
	for(i = 0; i < buff_count; i++)
		if(buffs[i].spellid == spellid)
			return true;

	return false;
}

// removes all buffs
void Mob::BuffFadeAll()
{
	uint32 buff_count = GetMaxTotalSlots();
	for (int j = 0; j < buff_count; j++) {
		if(buffs[j].spellid != SPELL_UNKNOWN)
			BuffFadeBySlot(j, false);
	}
	//we tell BuffFadeBySlot not to recalc, so we can do it only once when were done
	CalcBonuses();
}

void Mob::BuffFadeDetrimental() {
	uint32 buff_count = GetMaxTotalSlots();
	for (int j = 0; j < buff_count; j++) {
		if(buffs[j].spellid != SPELL_UNKNOWN) {
			if(IsDetrimentalSpell(buffs[j].spellid))
				BuffFadeBySlot(j, false);
		}
	}
}

void Mob::BuffFadeDetrimentalByCaster(Mob *caster)
{
	if(!caster)
		return;

	uint32 buff_count = GetMaxTotalSlots();
	for (int j = 0; j < buff_count; j++) {
		if(buffs[j].spellid != SPELL_UNKNOWN) {
			if(IsDetrimentalSpell(buffs[j].spellid))
			{
				//this is a pretty terrible way to do this but
				//there really isn't another way till I rewrite the basics
				Mob * c = entity_list.GetMob(buffs[j].casterid);
				if(c && c == caster)
					BuffFadeBySlot(j, false);
			}
		}
	}
}

void Mob::BuffFadeBySitModifier()
{
	bool r_bonus = false;
	uint32 buff_count = GetMaxTotalSlots();
	for(uint32 j = 0; j < buff_count; ++j)
	{
		if(buffs[j].spellid != SPELL_UNKNOWN)
		{
			if(spells[buffs[j].spellid].disallow_sit)
			{
				BuffFadeBySlot(j, false);
				r_bonus = true;
			}
		}
	}

	if(r_bonus)
	{
		CalcBonuses();
	}
}

// removes the buff matching spell_id
void Mob::BuffFadeBySpellID(uint16 spell_id)
{
	uint32 buff_count = GetMaxTotalSlots();
	for (int j = 0; j < buff_count; j++)
	{
		if (buffs[j].spellid == spell_id)
			BuffFadeBySlot(j, false);
	}

	//we tell BuffFadeBySlot not to recalc, so we can do it only once when were done
	CalcBonuses();
}

// removes buffs containing effectid, skipping skipslot
void Mob::BuffFadeByEffect(int effectid, int skipslot)
{
	int i;

	uint32 buff_count = GetMaxTotalSlots();
	for(i = 0; i < buff_count; i++)
	{
		if(buffs[i].spellid == SPELL_UNKNOWN)
			continue;
		if(IsEffectInSpell(buffs[i].spellid, effectid) && i != skipslot)
			BuffFadeBySlot(i, false);
	}

	//we tell BuffFadeBySlot not to recalc, so we can do it only once when were done
	CalcBonuses();
}

// checks if 'this' can be affected by spell_id from caster
// returns true if the spell should fail, false otherwise
bool Mob::IsImmuneToSpell(uint16 spell_id, Mob *caster)
{
	_ZP(Mob_IsImmuneToSpell);
	int effect_index;

	if(caster == nullptr)
		return(false);

	//TODO: this function loops through the effect list for
	//this spell like 10 times, this could easily be consolidated
	//into one loop through with a switch statement.

	mlog(SPELLS__RESISTS, "Checking to see if we are immune to spell %d cast by %s", spell_id, caster->GetName());

	if(!IsValidSpell(spell_id))
		return true;

	if(IsBeneficialSpell(spell_id) && (caster->GetNPCTypeID())) //then skip the rest, stop NPCs aggroing each other with buff spells. 2013-03-05
		return false;

	if(IsMezSpell(spell_id))
	{
		if(GetSpecialAbility(UNMEZABLE)) {
			mlog(SPELLS__RESISTS, "We are immune to Mez spells.");
			caster->Message_StringID(MT_Shout, CANNOT_MEZ);
			int32 aggro = CheckAggroAmount(spell_id);
			if(aggro > 0) {
				AddToHateList(caster, aggro);
			} else {
				AddToHateList(caster, 1);
			}
			return true;
		}

		// check max level for spell
		effect_index = GetSpellEffectIndex(spell_id, SE_Mez);
		assert(effect_index >= 0);
		// NPCs get to ignore the max level
		if((GetLevel() > spells[spell_id].max[effect_index]) &&
			(!caster->IsNPC() || (caster->IsNPC() && !RuleB(Spells, NPCIgnoreBaseImmunity))))
		{
			mlog(SPELLS__RESISTS, "Our level (%d) is higher than the limit of this Mez spell (%d)", GetLevel(), spells[spell_id].max[effect_index]);
			caster->Message_StringID(MT_Shout, CANNOT_MEZ_WITH_SPELL);
			return true;
		}
	}

	// slow and haste spells
	if(GetSpecialAbility(UNSLOWABLE) && IsEffectInSpell(spell_id, SE_AttackSpeed))
	{
		mlog(SPELLS__RESISTS, "We are immune to Slow spells.");
		caster->Message_StringID(MT_Shout, IMMUNE_ATKSPEED);
		int32 aggro = CheckAggroAmount(spell_id);
		if(aggro > 0) {
			AddToHateList(caster, aggro);
		} else {
			AddToHateList(caster, 1);
		}
		return true;
	}

	// client vs client fear
	if(IsEffectInSpell(spell_id, SE_Fear))
	{
		effect_index = GetSpellEffectIndex(spell_id, SE_Fear);
		if(GetSpecialAbility(UNFEARABLE)) {
			mlog(SPELLS__RESISTS, "We are immune to Fear spells.");
			caster->Message_StringID(MT_Shout, IMMUNE_FEAR);
			int32 aggro = CheckAggroAmount(spell_id);
			if(aggro > 0) {
				AddToHateList(caster, aggro);
			} else {
				AddToHateList(caster, 1);
			}
			return true;
		} else if(IsClient() && caster->IsClient() && (caster->CastToClient()->GetGM() == false))
		{
			mlog(SPELLS__RESISTS, "Clients cannot fear eachother!");
			caster->Message_StringID(MT_Shout, IMMUNE_FEAR);
			return true;
		}
		else if(GetLevel() > spells[spell_id].max[effect_index] && spells[spell_id].max[effect_index] != 0)
		{
			mlog(SPELLS__RESISTS, "Level is %d, cannot be feared by this spell.", GetLevel());
			caster->Message_StringID(MT_Shout, SPELL_NO_EFFECT);
			return true;
		}

		else if (IsClient() && CastToClient()->CheckAAEffect(aaEffectWarcry))
		{
			Message(13, "Your are immune to fear.");
			mlog(SPELLS__RESISTS, "Clients has WarCry effect, immune to fear!");
			caster->Message_StringID(MT_Shout, IMMUNE_FEAR);
			return true;
		}
	}

	if(IsCharmSpell(spell_id))
	{
		if(GetSpecialAbility(UNCHARMABLE))
		{
			mlog(SPELLS__RESISTS, "We are immune to Charm spells.");
			caster->Message_StringID(MT_Shout, CANNOT_CHARM);
			int32 aggro = CheckAggroAmount(spell_id);
			if(aggro > 0) {
				AddToHateList(caster, aggro);
			} else {
				AddToHateList(caster, 1);
			}
			return true;
		}

		if(this == caster)
		{
			mlog(SPELLS__RESISTS, "You are immune to your own charms.");
			caster->Message(MT_Shout, "You cannot charm yourself.");
			return true;
		}

		//let npcs cast whatever charm on anyone
		if(!caster->IsNPC())
		{
			// check level limit of charm spell
			effect_index = GetSpellEffectIndex(spell_id, SE_Charm);
			assert(effect_index >= 0);
			if(GetLevel() > spells[spell_id].max[effect_index] && spells[spell_id].max[effect_index] != 0)
			{
				mlog(SPELLS__RESISTS, "Our level (%d) is higher than the limit of this Charm spell (%d)", GetLevel(), spells[spell_id].max[effect_index]);
				caster->Message_StringID(MT_Shout, CANNOT_CHARM_YET);
				return true;
			}
		}
	}

	if
	(
		IsEffectInSpell(spell_id, SE_Root) ||
		IsEffectInSpell(spell_id, SE_MovementSpeed)
	)
	{
		if(GetSpecialAbility(UNSNAREABLE)) {
			mlog(SPELLS__RESISTS, "We are immune to Snare spells.");
			caster->Message_StringID(MT_Shout, IMMUNE_MOVEMENT);
			int32 aggro = CheckAggroAmount(spell_id);
			if(aggro > 0) {
				AddToHateList(caster, aggro);
			} else {
				AddToHateList(caster, 1);
			}
			return true;
		}
	}

	if(IsLifetapSpell(spell_id))
	{
		if(this == caster)
		{
			mlog(SPELLS__RESISTS, "You cannot lifetap yourself.");
			caster->Message_StringID(MT_Shout, CANT_DRAIN_SELF);
			return true;
		}
	}

	if(IsSacrificeSpell(spell_id))
	{
		if(this == caster)
		{
			mlog(SPELLS__RESISTS, "You cannot sacrifice yourself.");
			caster->Message_StringID(MT_Shout, CANNOT_SAC_SELF);
			return true;
		}
	}

	mlog(SPELLS__RESISTS, "No immunities to spell %d found.", spell_id);

	return false;
}

//
// Spell resists:
// returns an effectiveness index from 0 to 100. for most spells, 100 means
// it landed, and anything else means it was resisted; however there are some
// spells that can be partially effective, and this value can be used there.
// TODO: we need to figure out how the following pvp values work and implement them
// pvp_resist_base
// pvp_resist_calc
// pvp_resist_cap
float Mob::ResistSpell(uint8 resist_type, uint16 spell_id, Mob *caster, bool use_resist_override, int resist_override, bool CharismaCheck)
{

	if(!caster)
	{
		return 100;
	}

	if(spell_id != 0 && !IsValidSpell(spell_id))
	{
		return 0;
	}

	if(GetSpecialAbility(IMMUNE_CASTING_FROM_RANGE))
	{
		if(!caster->CombatRange(this))
		{
			return(0);
		}
	}

	if(GetSpecialAbility(IMMUNE_MAGIC))
	{
		mlog(SPELLS__RESISTS, "We are immune to magic, so we fully resist the spell %d", spell_id);
		return(0);
	}

	//Get resist modifier and adjust it based on focus 2 resist about eq to 1% resist chance
	int resist_modifier = (use_resist_override) ? resist_override : spells[spell_id].ResistDiff;
	if(caster->IsClient())
	{
		if(IsValidSpell(spell_id))
		{
			int focus_resist = caster->CastToClient()->GetFocusEffect(focusResistRate, spell_id);
			resist_modifier -= 2 * focus_resist;
		}
	}

	//Check for fear resist
	if(IsFearSpell(spell_id))
	{
		int fear_resist_bonuses = CalcFearResistChance();
		if(MakeRandomInt(0, 99) < fear_resist_bonuses)
		{
			mlog(SPELLS__RESISTS, "Resisted spell in fear resistance, had %d chance to resist", fear_resist_bonuses);
			return 0;
		}
	}


	if (!CharismaCheck){

		//Check for Spell Effect specific resistance chances (ie AA Mental Fortitude)
		int se_resist_bonuses = GetSpellEffectResistChance(spell_id);
		if(se_resist_bonuses && (MakeRandomInt(0, 99) < se_resist_bonuses))
		{
			return 0;
		}

		// Check for Chance to Resist Spell bonuses (ie Sanctification Discipline)
		int resist_bonuses = CalcResistChanceBonus();
		if(resist_bonuses && (MakeRandomInt(0, 99) < resist_bonuses))
		{
			mlog(SPELLS__RESISTS, "Resisted spell in sanctification, had %d chance to resist", resist_bonuses);
			return 0;
		}
	}

	//Get the resist chance for the target
	if(resist_type == RESIST_NONE)
	{
		mlog(SPELLS__RESISTS, "Spell was unresistable");
		return 100;
	}

	int target_resist;
	switch(resist_type)
	{
	case RESIST_FIRE:
		target_resist = GetFR();
		break;
	case RESIST_COLD:
		target_resist = GetCR();
		break;
	case RESIST_MAGIC:
		target_resist = GetMR();
		break;
	case RESIST_DISEASE:
		target_resist = GetDR();
		break;
	case RESIST_POISON:
		target_resist = GetPR();
		break;
	case RESIST_CORRUPTION:
		target_resist = GetCorrup();
		break;
	case RESIST_PRISMATIC:
		target_resist = (GetFR() + GetCR() + GetMR() + GetDR() + GetPR()) / 5;
		break;
	case RESIST_CHROMATIC:
		{
			target_resist = GetFR();
			int temp = GetCR();
			if(temp < target_resist)
			{
				target_resist = temp;
			}

			temp = GetMR();
			if(temp < target_resist)
			{
				target_resist = temp;
			}

			temp = GetDR();
			if(temp < target_resist)
			{
				target_resist = temp;
			}

			temp = GetPR();
			if(temp < target_resist)
			{
				target_resist = temp;
			}
		}
		break;
	case RESIST_PHYSICAL:
	default:
		//This is guessed but the others are right
		target_resist = (GetSTA() / 4);
	}

	//Setup our base resist chance.
	//Lulls have a slightly higher chance to resist than normal 15/200 or ~ 7.5%
	int resist_chance;
	if(IsHarmonySpell(spell_id))
	{
		resist_chance = 15;
	}
	else
	{
		resist_chance = 0;
	}

    //Adjust our resist chance based on level modifiers
    int temp_level_diff = GetLevel() - caster->GetLevel();
    if(IsNPC() && GetLevel() >= RuleI(Casting,ResistFalloff))
    {
        int a = (RuleI(Casting,ResistFalloff)-1) - caster->GetLevel();
        if(a > 0)
        {
            temp_level_diff = a;
        }
        else
        {
            temp_level_diff = 0;
        }
    }

    if(IsClient() && GetLevel() >= 21 && temp_level_diff > 15)
    {
        temp_level_diff = 15;
    }

    if(IsNPC() && temp_level_diff < -9)
    {
        temp_level_diff = -9;
    }

    int level_mod = temp_level_diff * temp_level_diff / 2;
    if(temp_level_diff < 0)
    {
        level_mod = -level_mod;
    }

    if(IsNPC() && (caster->GetLevel() - GetLevel()) < -20)
    {
        level_mod = 1000;
    }

    //Even more level stuff this time dealing with damage spells
    if(IsNPC() && IsDamageSpell(spell_id) && GetLevel() >= 17)
    {
        int level_diff;
        if(GetLevel() >= RuleI(Casting,ResistFalloff))
        {
            level_diff = (RuleI(Casting,ResistFalloff)-1) - caster->GetLevel();
            if(level_diff < 0)
            {
                level_diff = 0;
            }
        }
        else
        {
            level_diff = GetLevel() - caster->GetLevel();
        }
        level_mod += (2 * level_diff);
    }

	if (CharismaCheck)
	{
		//For charm chance to break checks, Default 10 CHA = -1 resist mod.
		int16 cha_resist_modifier = 0;
		cha_resist_modifier	= caster->GetCHA()/RuleI(Spells, CharismaEffectiveness);
		resist_modifier -= cha_resist_modifier;
	}

	//Add our level, resist and -spell resist modifier to our roll chance
	resist_chance += level_mod;
	resist_chance += resist_modifier;
	resist_chance += target_resist;

	resist_chance = mod_spell_resist(resist_chance, level_mod, resist_modifier, target_resist, resist_type, spell_id, caster);

	//Do our min and max resist checks.
	if(resist_chance > spells[spell_id].MaxResist && spells[spell_id].MaxResist != 0)
	{
		resist_chance = spells[spell_id].MaxResist;
	}

	if(resist_chance < spells[spell_id].MinResist && spells[spell_id].MinResist != 0)
	{
		resist_chance = spells[spell_id].MinResist;
	}

	//Finally our roll
	int roll = MakeRandomInt(0, 200);
	if(roll > resist_chance)
	{
		return 100;
	}
	else
	{
		//This is confusing but it's basically right
		//It skews partial resists up over 100 more often than not
		if(!IsPartialCapableSpell(spell_id))
		{
			return 0;
		}
		else
		{
			resist_chance -= roll;
			if(resist_chance < 1)
			{
				resist_chance = 1;
			}

			int partial_modifier = ((150 * (roll - resist_chance)) / resist_chance);

			if(IsNPC())
			{
				if(GetLevel() > caster->GetLevel() && GetLevel() >= 17 && caster->GetLevel() <= 50)
				{
					partial_modifier += 5;
				}

				if(GetLevel() >= 30 && caster->GetLevel() < 50)
				{
					partial_modifier += (caster->GetLevel() - 25);
				}

				if(GetLevel() < 15)
				{
					partial_modifier -= 5;
				}
			}

			if(caster->IsNPC())
			{
				if((GetLevel() - caster->GetLevel()) >= 20)
				{
					partial_modifier += (GetLevel() - caster->GetLevel()) * 1.5;
				}
			}

			if(partial_modifier < 0)
			{
				return 0;
			}

			if(partial_modifier > 100)
			{
				return 100;
			}

			return partial_modifier;
		}
	}
}

int16 Mob::CalcResistChanceBonus()
{
	int resistchance = spellbonuses.ResistSpellChance + itembonuses.ResistSpellChance;

	if(IsClient())
		resistchance += aabonuses.ResistSpellChance;

	if (spellbonuses.ResistSpellChance)
		CheckHitsRemaining(0, false, false, SE_ResistSpellChance);

	return resistchance;
}

int16 Mob::CalcFearResistChance()
{
	int resistchance = spellbonuses.ResistFearChance + itembonuses.ResistFearChance;
	if(this->IsClient()) {
		resistchance += aabonuses.ResistFearChance;
		if(aabonuses.Fearless == true)
			resistchance = 100;
	}
	if(spellbonuses.Fearless == true || itembonuses.Fearless == true)
		resistchance = 100;

	return resistchance;
}

float Mob::GetAOERange(uint16 spell_id) {
	float range;

	range = spells[spell_id].aoerange;
	if(range == 0)	//for TGB spells, they prolly do not have an aoe range
		range = spells[spell_id].range;
	if(range == 0)
		range = 10;	//something....

	if (IsClient()) {

		if(IsBardSong(spell_id) && IsBeneficialSpell(spell_id)) {
			//Live AA - Extended Notes, SionachiesCrescendo
			float song_bonus = aabonuses.SongRange + spellbonuses.SongRange + itembonuses.SongRange;
			range += range*song_bonus /100.0f;
		}

		range = CastToClient()->GetActSpellRange(spell_id, range);
	}

	return(range);
}

///////////////////////////////////////////////////////////////////////////////
// 'other' functions

void Mob::Spin() {
	if(IsClient()) {
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_Action, sizeof(Action_Struct));
		outapp->pBuffer[0] = 0x0B;
		outapp->pBuffer[1] = 0x0A;
		outapp->pBuffer[2] = 0x0B;
		outapp->pBuffer[3] = 0x0A;
		outapp->pBuffer[4] = 0xE7;
		outapp->pBuffer[5] = 0x00;
		outapp->pBuffer[6] = 0x4D;
		outapp->pBuffer[7] = 0x04;
		outapp->pBuffer[8] = 0x00;
		outapp->pBuffer[9] = 0x00;
		outapp->pBuffer[10] = 0x00;
		outapp->pBuffer[11] = 0x00;
		outapp->pBuffer[12] = 0x00;
		outapp->pBuffer[13] = 0x00;
		outapp->pBuffer[14] = 0x00;
		outapp->pBuffer[15] = 0x00;
		outapp->pBuffer[16] = 0x00;
		outapp->pBuffer[17] = 0x00;
		outapp->pBuffer[18] = 0xD4;
		outapp->pBuffer[19] = 0x43;
		outapp->pBuffer[20] = 0x00;
		outapp->pBuffer[21] = 0x00;
		outapp->pBuffer[22] = 0x00;
		outapp->priority = 5;
		CastToClient()->QueuePacket(outapp);
		safe_delete(outapp);
	}
	else {
		GMMove(GetX(), GetY(), GetZ(), GetHeading()+5);
	}
}

void Mob::SendSpellBarDisable()
{
	if (!IsClient())
		return;

	CastToClient()->MemorizeSpell(0, SPELLBAR_UNLOCK, memSpellSpellbar);
}

// this puts the spell bar back into a usable state fast
void Mob::SendSpellBarEnable(uint16 spell_id)
{
	if(!IsClient())
		return;

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_ManaChange, sizeof(ManaChange_Struct));
	ManaChange_Struct* manachange = (ManaChange_Struct*)outapp->pBuffer;
	manachange->new_mana = GetMana();
	manachange->spell_id = spell_id;
	manachange->stamina = CastToClient()->GetEndurance();
	outapp->priority = 6;
	CastToClient()->QueuePacket(outapp);
	safe_delete(outapp);
}

void Mob::Stun(int duration)
{
	//make sure a shorter stun does not overwrite a longer one.
	if(stunned && stunned_timer.GetRemainingTime() > uint32(duration))
		return;

	if(casting_spell_id) {
		int persistent_casting = spellbonuses.PersistantCasting + itembonuses.PersistantCasting;
		if(IsClient())
			persistent_casting += aabonuses.PersistantCasting;

		if(MakeRandomInt(1,99) > persistent_casting)
			InterruptSpell();
	}

	if(duration > 0)
	{
		stunned = true;
		stunned_timer.Start(duration);
	}
}

void Mob::UnStun() {
	if(stunned && stunned_timer.Enabled()) {
		stunned = false;
		stunned_timer.Disable();
	}
}

// Stuns "this"
void Client::Stun(int duration)
{
	Mob::Stun(duration);

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_Stun, sizeof(Stun_Struct));
	Stun_Struct* stunon = (Stun_Struct*) outapp->pBuffer;
	stunon->duration = duration;
	outapp->priority = 5;
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::UnStun() {
	Mob::UnStun();

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_Stun, sizeof(Stun_Struct));
	Stun_Struct* stunon = (Stun_Struct*) outapp->pBuffer;
	stunon->duration = 0;
	outapp->priority = 5;
	QueuePacket(outapp);
	safe_delete(outapp);
}

void NPC::Stun(int duration) {
	Mob::Stun(duration);
	SetRunAnimSpeed(0);
	SendPosition();
}

void NPC::UnStun() {
	Mob::UnStun();
	SetRunAnimSpeed(this->GetRunspeed());
	SendPosition();
}

void Mob::Mesmerize()
{
	mezzed = true;

	if (casting_spell_id)
		InterruptSpell();

	SendPosition();
/* this stuns the client for max time, with no way to break it
	if (this->IsClient()){
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_Stun, sizeof(Stun_Struct));
		Stun_Struct* stunon = (Stun_Struct*) outapp->pBuffer;
		stunon->duration = 0xFFFF;
		this->CastToClient()->QueuePacket(outapp);
		safe_delete(outapp);
	} else {
		SetRunAnimSpeed(0);
	}
*/
}

void Client::MakeBuffFadePacket(uint16 spell_id, int slot_id, bool send_message)
{
	EQApplicationPacket* outapp;

	outapp = new EQApplicationPacket(OP_Buff, sizeof(SpellBuffFade_Struct));
	SpellBuffFade_Struct* sbf = (SpellBuffFade_Struct*) outapp->pBuffer;

	sbf->entityid=GetID();
	// i dont know why but this works.. for now
	sbf->slot=2;
//	sbf->slot=m_pp.buffs[slot_id].slotid;
//	sbf->level=m_pp.buffs[slot_id].level;
//	sbf->effect=m_pp.buffs[slot_id].effect;
	sbf->spellid=spell_id;
	sbf->slotid=slot_id;
	sbf->bufffade = 1;
#if EQDEBUG >= 11
	printf("Sending SBF 1 from server:\n");
	DumpPacket(outapp);
#endif
	QueuePacket(outapp);

/*
	sbf->effect=0;
	sbf->level=0;
	sbf->slot=0;
*/
	sbf->spellid=0xffffffff;
#if EQDEBUG >= 11
	printf("Sending SBF 2 from server:\n");
	DumpPacket(outapp);
#endif
	QueuePacket(outapp);
	safe_delete(outapp);

	if(send_message)
	{
		const char *fadetext = spells[spell_id].spell_fades;
		outapp = new EQApplicationPacket(OP_BuffFadeMsg, sizeof(BuffFadeMsg_Struct) + strlen(fadetext));
		BuffFadeMsg_Struct *bfm = (BuffFadeMsg_Struct *) outapp->pBuffer;
		bfm->color = MT_Spells;
		memcpy(bfm->msg, fadetext, strlen(fadetext));
		QueuePacket(outapp);
		safe_delete(outapp);
	}

}

void Client::MemSpell(uint16 spell_id, int slot, bool update_client)
{
	if(slot >= MAX_PP_MEMSPELL || slot < 0)
		return;

	if(update_client)
	{
		if(m_pp.mem_spells[slot] != 0xFFFFFFFF)
			UnmemSpell(slot, update_client);
	}

	m_pp.mem_spells[slot] = spell_id;
	mlog(CLIENT__SPELLS, "Spell %d memorized into slot %d", spell_id, slot);

	if(update_client)
	{
		MemorizeSpell(slot, spell_id, memSpellMemorize);
	}
}

void Client::UnmemSpell(int slot, bool update_client)
{
	if(slot > MAX_PP_MEMSPELL || slot < 0)
		return;

	mlog(CLIENT__SPELLS, "Spell %d forgotten from slot %d", m_pp.mem_spells[slot], slot);
	m_pp.mem_spells[slot] = 0xFFFFFFFF;

	if(update_client)
	{
		MemorizeSpell(slot, m_pp.mem_spells[slot], memSpellForget);
	}
}

void Client::UnmemSpellAll(bool update_client)
{
	int i;

	for(i = 0; i < MAX_PP_MEMSPELL; i++)
		if(m_pp.mem_spells[i] != 0xFFFFFFFF)
			UnmemSpell(i, update_client);
}

void Client::ScribeSpell(uint16 spell_id, int slot, bool update_client)
{
	if(slot >= MAX_PP_SPELLBOOK || slot < 0)
		return;

	if(update_client)
	{
		if(m_pp.spell_book[slot] != 0xFFFFFFFF)
			UnscribeSpell(slot, update_client);
	}

	m_pp.spell_book[slot] = spell_id;
	mlog(CLIENT__SPELLS, "Spell %d scribed into spell book slot %d", spell_id, slot);

	if(update_client)
	{
		MemorizeSpell(slot, spell_id, memSpellScribing);
	}
}

void Client::UnscribeSpell(int slot, bool update_client)
{
	if(slot >= MAX_PP_SPELLBOOK || slot < 0)
		return;

	mlog(CLIENT__SPELLS, "Spell %d erased from spell book slot %d", m_pp.spell_book[slot], slot);
	m_pp.spell_book[slot] = 0xFFFFFFFF;

	if(update_client)
	{
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_DeleteSpell, sizeof(DeleteSpell_Struct));
		DeleteSpell_Struct* del = (DeleteSpell_Struct*)outapp->pBuffer;
		del->spell_slot = slot;
		del->success = 1;
		QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Client::UnscribeSpellAll(bool update_client)
{
	int i;

	for(i = 0; i < MAX_PP_SPELLBOOK; i++)
	{
		if(m_pp.spell_book[i] != 0xFFFFFFFF)
			UnscribeSpell(i, update_client);
	}
}

void Client::UntrainDisc(int slot, bool update_client)
{
	if(slot >= MAX_PP_DISCIPLINES || slot < 0)
		return;

	mlog(CLIENT__SPELLS, "Discipline %d untrained from slot %d", m_pp.disciplines.values[slot], slot);
	m_pp.disciplines.values[slot] = 0;

	if(update_client)
	{
		SendDisciplineUpdate();
	}
}

void Client::UntrainDiscAll(bool update_client)
{
	int i;

	for(i = 0; i < MAX_PP_DISCIPLINES; i++)
	{
		if(m_pp.disciplines.values[i] != 0)
			UntrainDisc(i, update_client);
	}
}

int Client::GetNextAvailableSpellBookSlot(int starting_slot) {
	for (int i = starting_slot; i < MAX_PP_SPELLBOOK; i++) {	//using starting_slot should help speed this up when we're iterating through a bunch of spells
		if (!IsValidSpell(GetSpellByBookSlot(i)))
			return i;
	}

	return -1;	//default
}

int Client::FindSpellBookSlotBySpellID(uint16 spellid) {
	for(int i = 0; i < MAX_PP_SPELLBOOK; i++) {
		if(m_pp.spell_book[i] == spellid)
			return i;
	}

	return -1;	//default
}

bool Client::SpellGlobalCheck(uint16 Spell_ID, uint16 Char_ID) {

	std::string Spell_Global_Name;
	int Spell_Global_Value;
	int Global_Value;

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (database.RunQuery(query,MakeAnyLenString(&query, "SELECT qglobal, value FROM spell_globals WHERE spellid=%i", Spell_ID), errbuf, &result)) {
		safe_delete_array(query);

		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			Spell_Global_Name = row[0];
			Spell_Global_Value = atoi(row[1]);

			mysql_free_result(result);

			if (Spell_Global_Name.empty()) { // If the entry in the spell_globals table has nothing set for the qglobal name
				return true;
			}
			else if (database.RunQuery(query,MakeAnyLenString(&query, "SELECT value FROM quest_globals WHERE charid=%i AND name='%s'", Char_ID, Spell_Global_Name.c_str()), errbuf, &result)) {
				safe_delete_array(query);

				if (mysql_num_rows(result) == 1) {
					row = mysql_fetch_row(result);

					Global_Value = atoi(row[0]);
					mysql_free_result(result);
					if (Global_Value == Spell_Global_Value) { // If the values match from both tables, allow the spell to be scribed
						return true;
					}
					else if (Global_Value > Spell_Global_Value) { // Check if the qglobal value is greater than the require spellglobal value
						return true;
					}
					else // If no matching result found in qglobals, don't scribe this spell
					{
						LogFile->write(EQEMuLog::Error, "Char ID: %i Spell_globals Name: '%s' Value: '%i' did not match QGlobal Value: '%i' for Spell ID %i", Char_ID, Spell_Global_Name.c_str(), Spell_Global_Value, Global_Value, Spell_ID);
						return false;
					}
				}
				else
					LogFile->write(EQEMuLog::Error, "Char ID: %i does not have the Qglobal Name: '%s' for Spell ID %i", Char_ID, Spell_Global_Name.c_str(), Spell_ID);
					safe_delete_array(query);
			}
			else
				LogFile->write(EQEMuLog::Error, "Spell ID %i query of spell_globals with Name: '%s' Value: '%i' failed", Spell_ID, Spell_Global_Name.c_str(), Spell_Global_Value);
		}
		else {
			return true; // Spell ID isn't listed in the spells_global table, so it is not restricted from scribing
		}
		mysql_free_result(result);
	}
	else {
		LogFile->write(EQEMuLog::Error, "Error while querying Spell ID %i spell_globals table query '%s': %s", Spell_ID, query, errbuf);
		safe_delete_array(query);
		return false; // Query failed, so prevent spell from scribing just in case
	}
	return false; // Default is false
}

// TODO get rid of this
int16 Mob::GetBuffSlotFromType(uint16 type) {
	uint32 buff_count = GetMaxTotalSlots();
	for (int i = 0; i < buff_count; i++) {
		if (buffs[i].spellid != SPELL_UNKNOWN) {
			for (int j = 0; j < EFFECT_COUNT; j++) {
				if (spells[buffs[i].spellid].effectid[j] == type )
					return i;
			}
		}
	}
	return -1;
}

uint16 Mob::GetSpellIDFromSlot(uint8 slot)
{
	if (buffs[slot].spellid != SPELL_UNKNOWN)
		return buffs[slot].spellid;
	return 0;
}


bool Mob::FindType(uint16 type, bool bOffensive, uint16 threshold) {
	uint32 buff_count = GetMaxTotalSlots();
	for (int i = 0; i < buff_count; i++) {
		if (buffs[i].spellid != SPELL_UNKNOWN) {

			for (int j = 0; j < EFFECT_COUNT; j++) {
				// adjustments necessary for offensive npc casting behavior
				if (bOffensive) {
					if (spells[buffs[i].spellid].effectid[j] == type) {
						int16 value =
								CalcSpellEffectValue_formula(spells[buffs[i].spellid].buffdurationformula,
											spells[buffs[i].spellid].base[j],
											spells[buffs[i].spellid].max[j],
											buffs[i].casterlevel, buffs[i].spellid);
						LogFile->write(EQEMuLog::Normal,
								"FindType: type = %d; value = %d; threshold = %d",
								type, value, threshold);
						if (value < threshold)
							return true;
					}
				} else {
					if (spells[buffs[i].spellid].effectid[j] == type )
						return true;
				}
			}
		}
	}
	return false;
}

bool Mob::AddProcToWeapon(uint16 spell_id, bool bPerma, uint16 iChance) {
	if(spell_id == SPELL_UNKNOWN)
		return(false);

	int i;
	if (bPerma) {
		for (i = 0; i < MAX_PROCS; i++) {
			if (PermaProcs[i].spellID == SPELL_UNKNOWN) {
				PermaProcs[i].spellID = spell_id;
				PermaProcs[i].chance = iChance;
				PermaProcs[i].base_spellID = SPELL_UNKNOWN;
				mlog(SPELLS__PROCS, "Added permanent proc spell %d with chance %d to slot %d", spell_id, iChance, i);

				return true;
			}
		}
		mlog(SPELLS__PROCS, "Too many perma procs for %s", GetName());
	} else {
		for (i = 0; i < MAX_PROCS; i++) {
			if (SpellProcs[i].spellID == SPELL_UNKNOWN) {
				SpellProcs[i].spellID = spell_id;
				SpellProcs[i].chance = iChance;
				SpellProcs[i].base_spellID = SPELL_UNKNOWN;;
				mlog(SPELLS__PROCS, "Added spell-granted proc spell %d with chance %d to slot %d", spell_id, iChance, i);
				return true;
			}
		}
		mlog(SPELLS__PROCS, "Too many procs for %s", GetName());
	}
	return false;
}

bool Mob::RemoveProcFromWeapon(uint16 spell_id, bool bAll) {
	for (int i = 0; i < MAX_PROCS; i++) {
		if (bAll || SpellProcs[i].spellID == spell_id) {
			SpellProcs[i].spellID = SPELL_UNKNOWN;
			SpellProcs[i].chance = 0;
			SpellProcs[i].base_spellID = SPELL_UNKNOWN;
			mlog(SPELLS__PROCS, "Removed proc %d from slot %d", spell_id, i);
		}
	}
	return true;
}

bool Mob::AddDefensiveProc(uint16 spell_id, uint16 iChance, uint16 base_spell_id)
{
	if(spell_id == SPELL_UNKNOWN)
		return(false);

	int i;
	for (i = 0; i < MAX_PROCS; i++) {
		if (DefensiveProcs[i].spellID == SPELL_UNKNOWN) {
			DefensiveProcs[i].spellID = spell_id;
			DefensiveProcs[i].chance = iChance;
			DefensiveProcs[i].base_spellID = base_spell_id;
			mlog(SPELLS__PROCS, "Added spell-granted defensive proc spell %d with chance %d to slot %d", spell_id, iChance, i);
			return true;
		}
	}

	return false;
}

bool Mob::RemoveDefensiveProc(uint16 spell_id, bool bAll)
{
	for (int i = 0; i < MAX_PROCS; i++) {
		if (bAll || DefensiveProcs[i].spellID == spell_id) {
			DefensiveProcs[i].spellID = SPELL_UNKNOWN;
			DefensiveProcs[i].chance = 0;
			DefensiveProcs[i].base_spellID = SPELL_UNKNOWN;
			mlog(SPELLS__PROCS, "Removed defensive proc %d from slot %d", spell_id, i);
		}
	}
	return true;
}

bool Mob::AddSkillProc(uint16 spell_id, uint16 iChance, uint16 base_spell_id)
{
	if(spell_id == SPELL_UNKNOWN)
		return(false);

	int i;
	for (i = 0; i < MAX_PROCS; i++) {
		if (SkillProcs[i].spellID == SPELL_UNKNOWN) {
			SkillProcs[i].spellID = spell_id;
			SkillProcs[i].chance = iChance;
			SkillProcs[i].base_spellID = base_spell_id;
			mlog(SPELLS__PROCS, "Added spell-granted skill proc spell %d with chance %d to slot %d", spell_id, iChance, i);
			return true;
		}
	}
	return false;
}

bool Mob::RemoveSkillProc(uint16 spell_id, bool bAll)
{
	for (int i = 0; i < MAX_PROCS; i++) {
		if (bAll || SkillProcs[i].spellID == spell_id) {
			SkillProcs[i].spellID = SPELL_UNKNOWN;
			SkillProcs[i].chance = 0;
			SkillProcs[i].base_spellID = SPELL_UNKNOWN;
			mlog(SPELLS__PROCS, "Removed Skill proc %d from slot %d", spell_id, i);
		}
	}
	return true;
}

bool Mob::AddRangedProc(uint16 spell_id, uint16 iChance, uint16 base_spell_id)
{
	if(spell_id == SPELL_UNKNOWN)
		return(false);

	int i;
	for (i = 0; i < MAX_PROCS; i++) {
		if (RangedProcs[i].spellID == SPELL_UNKNOWN) {
			RangedProcs[i].spellID = spell_id;
			RangedProcs[i].chance = iChance;
			RangedProcs[i].base_spellID = base_spell_id;
			mlog(SPELLS__PROCS, "Added spell-granted ranged proc spell %d with chance %d to slot %d", spell_id, iChance, i);
			return true;
		}
	}

	return false;
}

bool Mob::RemoveRangedProc(uint16 spell_id, bool bAll)
{
	for (int i = 0; i < MAX_PROCS; i++) {
		if (bAll || RangedProcs[i].spellID == spell_id) {
			RangedProcs[i].spellID = SPELL_UNKNOWN;
			RangedProcs[i].chance = 0;
			RangedProcs[i].base_spellID = SPELL_UNKNOWN;;
			mlog(SPELLS__PROCS, "Removed ranged proc %d from slot %d", spell_id, i);
		}
	}
	return true;
}

// this is checked in a few places to decide wether special bard
// behavior should be used.
bool Mob::UseBardSpellLogic(uint16 spell_id, int slot)
{
	if(spell_id == SPELL_UNKNOWN)
		spell_id = casting_spell_id;

	if(slot == -1)
		slot = casting_spell_slot;

	// should we treat this as a bard singing?
	return
	(
		spell_id != 0 &&
		spell_id != SPELL_UNKNOWN &&
		slot != -1 &&
		GetClass() == BARD &&
		slot <= MAX_PP_MEMSPELL &&
		IsBardSong(spell_id)
	);
}

int Mob::GetCasterLevel(uint16 spell_id) {
	int level = GetLevel();
	level += itembonuses.effective_casting_level + spellbonuses.effective_casting_level + aabonuses.effective_casting_level;
	mlog(SPELLS__CASTING, "Determined effective casting level %d+%d+%d=%d", GetLevel(), spellbonuses.effective_casting_level, itembonuses.effective_casting_level, level);
	return(level);
}


//this method does NOT tell the client to stop singing the song.
//this is NOT the right way to stop a mob from singing, use InterruptSpell
//you should really know what your doing before you call this
void Mob::_StopSong()
{
	bardsong = 0;
	bardsong_target_id = 0;
	bardsong_slot = 0;
	bardsong_timer.Disable();
}

//This member function sets the buff duration on the client
//however it does not work if sent quickly after an action packets, which is what one might perfer to do
//Thus I use this in the buff process to update the correct duration once after casting
//this allows AAs and focus effects that increase buff duration to work correctly, but could probably
//be used for other things as well
void Client::SendBuffDurationPacket(uint16 spell_id, int duration, int inlevel)
{
	EQApplicationPacket* outapp;
	outapp = new EQApplicationPacket(OP_Buff, sizeof(SpellBuffFade_Struct));
	SpellBuffFade_Struct* sbf = (SpellBuffFade_Struct*) outapp->pBuffer;

	sbf->entityid = GetID();
	sbf->slot=2;
	sbf->spellid=spell_id;
	sbf->slotid=0;
	sbf->effect = inlevel > 0 ? inlevel : GetLevel();
	sbf->level = inlevel > 0 ? inlevel : GetLevel();
	sbf->bufffade = 0;
	sbf->duration = duration;
	FastQueuePacket(&outapp);
}

void Mob::SendPetBuffsToClient()
{
	// Don't really need this check, as it should be checked before this method is called, but it doesn't hurt
	// too much to check again.
	if(!(GetOwner() && GetOwner()->IsClient()))
		return;

	int PetBuffCount = 0;


	EQApplicationPacket* outapp = new EQApplicationPacket(OP_PetBuffWindow,sizeof(PetBuff_Struct));
	PetBuff_Struct* pbs=(PetBuff_Struct*)outapp->pBuffer;
	memset(outapp->pBuffer,0,outapp->size);
	pbs->petid=GetID();

	int MaxSlots = GetMaxTotalSlots();

	if(MaxSlots > BUFF_COUNT)
		MaxSlots = BUFF_COUNT;

	for(int buffslot = 0; buffslot < MaxSlots; buffslot++)
	{
		if(buffs[buffslot].spellid != SPELL_UNKNOWN) {
			pbs->spellid[buffslot] = buffs[buffslot].spellid;
			pbs->ticsremaining[buffslot] = buffs[buffslot].ticsremaining;
			PetBuffCount++;
		}
	}

	pbs->buffcount=PetBuffCount;
	GetOwner()->CastToClient()->QueuePacket(outapp);
	safe_delete(outapp);
}

void Mob::SendBuffsToClient(Client *c)
{
	if(!c)
		return;

	if(c->GetClientVersionBit() & BIT_SoDAndLater)
	{
		EQApplicationPacket *outapp = MakeBuffsPacket();
		c->FastQueuePacket(&outapp);
	}
}

EQApplicationPacket *Mob::MakeBuffsPacket(bool for_target)
{
	uint32 count = 0;
	uint32 buff_count = GetMaxTotalSlots();
	for(unsigned int i = 0; i < buff_count; ++i)
	{
		if(buffs[i].spellid != SPELL_UNKNOWN)
		{
			++count;
		}
	}

	EQApplicationPacket* outapp = nullptr;

	//Create it for a targeting window, else create it for a create buff packet.
	if(for_target)
	{
		outapp = new EQApplicationPacket(OP_TargetBuffs, sizeof(BuffIcon_Struct) + sizeof(BuffIconEntry_Struct) * count);
	}
	else
	{
		outapp = new EQApplicationPacket(OP_BuffCreate, sizeof(BuffIcon_Struct) + sizeof(BuffIconEntry_Struct) * count);
	}
	BuffIcon_Struct *buff = (BuffIcon_Struct*)outapp->pBuffer;
	buff->entity_id = GetID();
	buff->count = count;

	uint32 index = 0;
	for(unsigned int i = 0; i < buff_count; ++i)
	{
		if(buffs[i].spellid != SPELL_UNKNOWN)
		{
			buff->entries[index].buff_slot = i;
			buff->entries[index].spell_id = buffs[i].spellid;
			buff->entries[index].tics_remaining = buffs[i].ticsremaining;
			++index;
		}
	}

	return outapp;
}



void Mob::BuffModifyDurationBySpellID(uint16 spell_id, int32 newDuration)
{
	uint32 buff_count = GetMaxTotalSlots();
	for(int i = 0; i < buff_count; ++i)
	{
		if (buffs[i].spellid == spell_id)
		{
			buffs[i].ticsremaining = newDuration;
			if(IsClient())
			{
				CastToClient()->SendBuffDurationPacket(buffs[i].spellid, buffs[i].ticsremaining, buffs[i].casterlevel);
			}
		}
	}
}
void Mob::UpdateRuneFlags()
{
	bool Has_SE_Rune = false, Has_SE_AbsorbMagicAtt = false, Has_SE_MitigateMeleeDamage = false, Has_SE_MitigateSpellDamage = false;
	uint32 buff_count = GetMaxTotalSlots();
	for (unsigned int i = 0; i < buff_count; ++i)
	{
		if (buffs[i].spellid != SPELL_UNKNOWN)
		{
			for (int j = 0; j < EFFECT_COUNT; ++j)
			{
				switch(spells[buffs[i].spellid].effectid[j])
				{
					case SE_Rune:
					{
						Has_SE_Rune = true;
						break;
					}
					case SE_AbsorbMagicAtt:
					{
						Has_SE_AbsorbMagicAtt = true;
						break;
					}
					case SE_MitigateMeleeDamage:
					{
						Has_SE_MitigateMeleeDamage = true;
						break;
					}
					case SE_MitigateSpellDamage:
					{
						Has_SE_MitigateSpellDamage = true;
						break;
					}

					default:
						break;
				}
			}
		}
	}

	SetHasRune(Has_SE_Rune);
	SetHasSpellRune(Has_SE_AbsorbMagicAtt);
	SetHasPartialMeleeRune(Has_SE_MitigateMeleeDamage);
	SetHasPartialSpellRune(Has_SE_MitigateSpellDamage);
}

int Client::GetCurrentBuffSlots() const
{
	if(15 + aabonuses.BuffSlotIncrease > 25)
		return 25;
	else
		return 15 + aabonuses.BuffSlotIncrease;
}

int Client::GetCurrentSongSlots() const
{
	return 12; // AAs dont affect this
}

void Client::InitializeBuffSlots()
{
	int max_slots = GetMaxTotalSlots();
	buffs = new Buffs_Struct[max_slots];
	for(int x = 0; x < max_slots; ++x)
	{
		buffs[x].spellid = SPELL_UNKNOWN;
	}
	current_buff_count = 0;
}

void Client::UninitializeBuffSlots()
{
	safe_delete_array(buffs);
}

void NPC::InitializeBuffSlots()
{
	int max_slots = GetMaxTotalSlots();
	buffs = new Buffs_Struct[max_slots];
	for(int x = 0; x < max_slots; ++x)
	{
		buffs[x].spellid = SPELL_UNKNOWN;
	}
	current_buff_count = 0;
}

void NPC::UninitializeBuffSlots()
{
	safe_delete_array(buffs);
}

