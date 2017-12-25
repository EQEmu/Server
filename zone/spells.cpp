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

#include "../common/bodytypes.h"
#include "../common/classes.h"
#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"
#include "../common/item_instance.h"
#include "../common/rulesys.h"
#include "../common/skills.h"
#include "../common/spdat.h"
#include "../common/string_util.h"
#include "../common/data_verification.h"
#include "../common/misc_functions.h"

#include "quest_parser_collection.h"
#include "string_ids.h"
#include "worldserver.h"

#include <assert.h>
#include <math.h>
#include <algorithm>

#ifndef WIN32
	#include <stdlib.h>
	#include "../common/unix.h"
#endif

#ifdef _GOTFRAGS
	#include "../common/packet_dump_file.h"
#endif

#ifdef BOTS
#include "bot.h"
#endif



extern Zone* zone;
extern volatile bool is_zone_loaded;
extern WorldServer worldserver;

using EQEmu::CastingSlot;

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
	if (casting_spell_id != 0 && casting_spell_checks && spellend_timer.Check())
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
	if (swarm_timer.Check()) {
		DepopSwarmPets();
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
bool Mob::CastSpell(uint16 spell_id, uint16 target_id, CastingSlot slot,
	int32 cast_time, int32 mana_cost, uint32* oSpellWillFinish, uint32 item_slot,
	uint32 timer, uint32 timer_duration, int16 *resist_adjust,
	uint32 aa_id)
{
	Log(Logs::Detail, Logs::Spells, "CastSpell called for spell %s (%d) on entity %d, slot %d, time %d, mana %d, from item slot %d",
		(IsValidSpell(spell_id))?spells[spell_id].name:"UNKNOWN SPELL", spell_id, target_id, static_cast<int>(slot), cast_time, mana_cost, (item_slot==0xFFFFFFFF)?999:item_slot);

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
		Log(Logs::Detail, Logs::Spells, "Spell casting canceled: not able to cast now. Valid? %d, casting %d, waiting? %d, spellend? %d, stunned? %d, feared? %d, mezed? %d, silenced? %d, amnesiad? %d",
			IsValidSpell(spell_id), casting_spell_id, delaytimer, spellend_timer.Enabled(), IsStunned(), IsFeared(), IsMezzed(), IsSilenced(), IsAmnesiad() );
		if(IsSilenced() && !IsDiscipline(spell_id))
			Message_StringID(13, SILENCED_STRING);
		if(IsAmnesiad() && IsDiscipline(spell_id))
			Message_StringID(13, MELEE_SILENCE);
		if(IsClient())
			CastToClient()->SendSpellBarEnable(spell_id);
		if(casting_spell_id && IsNPC())
			CastToNPC()->AI_Event_SpellCastFinished(false, static_cast<uint16>(casting_spell_slot));
		return(false);
	}
	//It appears that the Sanctuary effect is removed by a check on the client side (keep this however for redundancy)
	if (spellbonuses.Sanctuary && (spells[spell_id].targettype != ST_Self && GetTarget() != this) || IsDetrimentalSpell(spell_id))
		BuffFadeByEffect(SE_Sanctuary);

	if(IsClient()){
		int chance = CastToClient()->GetFocusEffect(focusFcMute, spell_id);//Client only

		if (zone->random.Roll(chance)) {
			Message_StringID(13, SILENCED_STRING);
			if(IsClient())
				CastToClient()->SendSpellBarEnable(spell_id);
			return(false);
		}
	}

	if(IsDetrimentalSpell(spell_id) && !zone->CanDoCombat()){
		Message_StringID(13, SPELL_WOULDNT_HOLD);
		if(IsClient())
			CastToClient()->SendSpellBarEnable(spell_id);
		if(casting_spell_id && IsNPC())
			CastToNPC()->AI_Event_SpellCastFinished(false, static_cast<uint16>(casting_spell_slot));
		return(false);
	}

	//cannot cast under divine aura
	if(DivineAura()) {
		Log(Logs::Detail, Logs::Spells, "Spell casting canceled: cannot cast while Divine Aura is in effect.");
		InterruptSpell(173, 0x121, false);
		return(false);
	}

	if (spellbonuses.NegateIfCombat)
		BuffFadeByEffect(SE_NegateIfCombat);

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

	if (HasActiveSong() && IsBardSong(spell_id)) {
		Log(Logs::Detail, Logs::Spells, "Casting a new song while singing a song. Killing old song %d.", bardsong);
		//Note: this does NOT tell the client
		_StopSong();
	}

	//Added to prevent MQ2 exploitation of equipping normally-unequippable/clickable items with effects and clicking them for benefits.
	if(item_slot && IsClient() && (slot == CastingSlot::Item || slot == CastingSlot::PotionBelt))
	{
		EQEmu::ItemInstance *itm = CastToClient()->GetInv().GetItem(item_slot);
		int bitmask = 1;
		bitmask = bitmask << (CastToClient()->GetClass() - 1);
		if( itm && itm->GetItem()->Classes != 65535 ) {
			if ((itm->GetItem()->Click.Type == EQEmu::item::ItemEffectEquipClick) && !(itm->GetItem()->Classes & bitmask)) {
				if (CastToClient()->ClientVersion() < EQEmu::versions::ClientVersion::SoF) {
					// They are casting a spell from an item that requires equipping but shouldn't let them equip it
					Log(Logs::General, Logs::Error, "HACKER: %s (account: %s) attempted to click an equip-only effect on item %s (id: %d) which they shouldn't be able to equip!",
						CastToClient()->GetCleanName(), CastToClient()->AccountName(), itm->GetItem()->Name, itm->GetItem()->ID);
					database.SetHackerFlag(CastToClient()->AccountName(), CastToClient()->GetCleanName(), "Clicking equip-only item with an invalid class");
				}
				else {
					Message_StringID(13, MUST_EQUIP_ITEM);
				}
				return(false);
			}
			if ((itm->GetItem()->Click.Type == EQEmu::item::ItemEffectClick2) && !(itm->GetItem()->Classes & bitmask)) {
				if (CastToClient()->ClientVersion() < EQEmu::versions::ClientVersion::SoF) {
					// They are casting a spell from an item that they don't meet the race/class requirements to cast
					Log(Logs::General, Logs::Error, "HACKER: %s (account: %s) attempted to click a race/class restricted effect on item %s (id: %d) which they shouldn't be able to click!",
						CastToClient()->GetCleanName(), CastToClient()->AccountName(), itm->GetItem()->Name, itm->GetItem()->ID);
					database.SetHackerFlag(CastToClient()->AccountName(), CastToClient()->GetCleanName(), "Clicking race/class restricted item with an invalid class");
				}
				else {
					if (CastToClient()->ClientVersion() >= EQEmu::versions::ClientVersion::RoF)
					{
						// Line 181 in eqstr_us.txt was changed in RoF+
						Message(15, "Your race, class, or deity cannot use this item.");
					}
					else
					{
						Message_StringID(13, CANNOT_USE_ITEM);
					}
				}
				return(false);
			}
		}
		if (itm && (itm->GetItem()->Click.Type == EQEmu::item::ItemEffectEquipClick) && !(item_slot <= EQEmu::inventory::slotAmmo || item_slot == EQEmu::inventory::slotPowerSource)){
			if (CastToClient()->ClientVersion() < EQEmu::versions::ClientVersion::SoF) {
				// They are attempting to cast a must equip clicky without having it equipped
				Log(Logs::General, Logs::Error, "HACKER: %s (account: %s) attempted to click an equip-only effect on item %s (id: %d) without equiping it!", CastToClient()->GetCleanName(), CastToClient()->AccountName(), itm->GetItem()->Name, itm->GetItem()->ID);
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

	//To prevent NPC ghosting when spells are cast from scripts
	if (IsNPC() && IsMoving() && cast_time > 0)
		SendPosition();

	if(resist_adjust)
	{
		return(DoCastSpell(spell_id, target_id, slot, cast_time, mana_cost, oSpellWillFinish, item_slot, timer, timer_duration, *resist_adjust, aa_id));
	}
	else
	{
		return(DoCastSpell(spell_id, target_id, slot, cast_time, mana_cost, oSpellWillFinish, item_slot, timer, timer_duration, spells[spell_id].ResistDiff, aa_id));
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
bool Mob::DoCastSpell(uint16 spell_id, uint16 target_id, CastingSlot slot,
					int32 cast_time, int32 mana_cost, uint32* oSpellWillFinish,
					uint32 item_slot, uint32 timer, uint32 timer_duration,
					int16 resist_adjust, uint32 aa_id)
{
	Mob* pMob = nullptr;
	int32 orgcasttime;

	if(!IsValidSpell(spell_id)) {
		InterruptSpell();
		return(false);
	}

	const SPDat_Spell_Struct &spell = spells[spell_id];

	Log(Logs::Detail, Logs::Spells, "DoCastSpell called for spell %s (%d) on entity %d, slot %d, time %d, mana %d, from item %d",
		spell.name, spell_id, target_id, static_cast<int>(slot), cast_time, mana_cost, item_slot==0xFFFFFFFF?999:item_slot);

	casting_spell_id = spell_id;
	casting_spell_slot = slot;
	casting_spell_inventory_slot = item_slot;
	if(casting_spell_timer != 0xFFFFFFFF)
	{
		casting_spell_timer = timer;
		casting_spell_timer_duration = timer_duration;
	}
	casting_spell_aa_id = aa_id;

	// check for fizzle
	// note that CheckFizzle itself doesn't let NPCs fizzle,
	// but this code allows for it.
	if (slot < CastingSlot::MaxGems && !CheckFizzle(spell_id)) {
		int fizzle_msg = IsBardSong(spell_id) ? MISS_NOTE : SPELL_FIZZLE;

		uint32 use_mana = ((spells[spell_id].mana) / 4);
		Log(Logs::Detail, Logs::Spells, "Spell casting canceled: fizzled. %d mana has been consumed", use_mana);

		// fizzle 1/4 the mana away
		Mob::SetMana(GetMana() - use_mana); // We send StopCasting which will update mana
		StopCasting();

		Message_StringID(MT_SpellFailure, fizzle_msg);

		/* Song Failure Messages */
		entity_list.FilteredMessageClose_StringID(
			this, /* Sender */
			true, /* Skip Sender */
			RuleI(Range, SpellMessages), 
			MT_SpellFailure, /* Type: 289 */
			(IsClient() ? FilterPCSpells : FilterNPCSpells), /* FilterType: 8 or 9 depending on client/npc */
			(fizzle_msg == MISS_NOTE ? MISSED_NOTE_OTHER : SPELL_FIZZLE_OTHER), 
			/* 
				MessageFormat: You miss a note, bringing your song to a close! (if missed note)
				MessageFormat: A missed note brings %1's song to a close!
				MessageFormat: %1's spell fizzles!
			*/
			GetName() /* Message1 */
		);

		TryTriggerOnValueAmount(false, true);
		return(false);
	}

	SaveSpellLoc();
	Log(Logs::Detail, Logs::Spells, "Casting %d Started at (%.3f,%.3f,%.3f)", spell_id, m_SpellLocation.x, m_SpellLocation.y, m_SpellLocation.z);

	// if this spell doesn't require a target, or if it's an optional target
	// and a target wasn't provided, then it's us; unless TGB is on and this
	// is a TGB compatible spell.
	if((IsGroupSpell(spell_id) ||
		spell.targettype == ST_AEClientV1 ||
		spell.targettype == ST_Self ||
		spell.targettype == ST_AECaster ||
		spell.targettype == ST_Ring ||
		spell.targettype == ST_Beam ||
		spell.targettype == ST_TargetOptional) && target_id == 0)
	{
		Log(Logs::Detail, Logs::Spells, "Spell %d auto-targeted the caster. Group? %d, target type %d", spell_id, IsGroupSpell(spell_id), spell.targettype);
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
		Log(Logs::Detail, Logs::Spells, "Spell Error: no target. spell=%d", spell_id);
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

	// We don't get actual mana cost here, that's done when we consume the mana
	if (mana_cost == -1)
		mana_cost = spell.mana;

	// mana is checked for clients on the frontend. we need to recheck it for NPCs though
	// If you're at full mana, let it cast even if you dont have enough mana

	// we calculated this above, now enforce it
	if(mana_cost > 0 && slot != CastingSlot::Item)
	{
		int my_curmana = GetMana();
		int my_maxmana = GetMaxMana();
		if(my_curmana < mana_cost)	// not enough mana
		{
			//this is a special case for NPCs with no mana...
			if(IsNPC() && my_curmana == my_maxmana)
			{
				mana_cost = 0;
			} else {
				Log(Logs::Detail, Logs::Spells, "Spell Error not enough mana spell=%d mymana=%d cost=%d\n", GetName(), spell_id, my_curmana, mana_cost);
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

	Log(Logs::Detail, Logs::Spells, "Spell %d: Casting time %d (orig %d), mana cost %d",
			spell_id, cast_time, orgcasttime, mana_cost);

	// now tell the people in the area -- we ALWAYS want to send this, even instant cast spells.
	// The only time this is skipped is for NPC innate procs and weapon procs. Procs from buffs
	// oddly still send this. Since those cases don't reach here, we don't need to check them
	if (slot != CastingSlot::Discipline)
		SendBeginCast(spell_id, orgcasttime);

	// cast time is 0, just finish it right now and be done with it
	if(cast_time == 0) {
		if (!DoCastingChecks()) {
			StopCasting();
			return false;
		}
		CastedSpellFinished(spell_id, target_id, slot, mana_cost, item_slot, resist_adjust);
		return(true);
	}

	cast_time = mod_cast_time(cast_time);

	// ok we know it has a cast time so we can start the timer now
	spellend_timer.Start(cast_time);

	if (IsAIControlled())
	{
		SetRunAnimSpeed(0);
		pMob = entity_list.GetMob(target_id);
		if (pMob && this != pMob)
			FaceTarget(pMob);
	}

	// if we got here we didn't fizzle, and are starting our cast
	if (oSpellWillFinish)
		*oSpellWillFinish = Timer::GetCurrentTime() + cast_time + 100;

	if (IsClient() && slot == CastingSlot::Item && item_slot != 0xFFFFFFFF) {
		auto item = CastToClient()->GetInv().GetItem(item_slot);
		if (item && item->GetItem())
			Message_StringID(MT_Spells, BEGINS_TO_GLOW, item->GetItem()->Name);
	}

	if (!DoCastingChecks()) {
		StopCasting();
		return false;
	}

	return(true);
}

void Mob::SendBeginCast(uint16 spell_id, uint32 casttime)
{
	auto outapp = new EQApplicationPacket(OP_BeginCast, sizeof(BeginCast_Struct));
	auto begincast = (BeginCast_Struct *)outapp->pBuffer;

	begincast->caster_id = GetID();
	begincast->spell_id = spell_id;
	begincast->cast_time = casttime; // client calculates reduced time by itself

	outapp->priority = 3;

	entity_list.QueueCloseClients(
		this, /* Sender */
		outapp, /* Packet */
		false, /* Ignore Sender */
		RuleI(Range, BeginCast),
		0, /* Skip this Mob */
		true /* Packet ACK */
	); //IsClient() ? FILTER_PCSPELLS : FILTER_NPCSPELLS);

	safe_delete(outapp);
}

/*
 * Some failures should be caught before the spell finishes casting
 * This is especially helpful to clients when they cast really long things
 * If this passes it sets casting_spell_checks to true which is checked in
 * SpellProcess(), if a situation ever arises where a spell is delayed by these
 * it's probably doing something wrong.
 */

bool Mob::DoCastingChecks()
{
	if (!IsClient() || (IsClient() && CastToClient()->GetGM())) {
		casting_spell_checks = true;
		return true;
	}

	uint16 spell_id = casting_spell_id;
	Mob *spell_target = entity_list.GetMob(casting_spell_targetid);

	if (RuleB(Spells, BuffLevelRestrictions)) {
		// casting_spell_targetid is guaranteed to be what we went, check for ST_Self for now should work though
		if (spell_target && spells[spell_id].targettype != ST_Self && !spell_target->CheckSpellLevelRestriction(spell_id)) {
			Log(Logs::Detail, Logs::Spells, "Spell %d failed: recipient did not meet the level restrictions", spell_id);
			if (!IsBardSong(spell_id))
				Message_StringID(MT_SpellFailure, SPELL_TOO_POWERFUL);
			return false;
		}
	}

	if (spells[spell_id].zonetype == 1 && !zone->CanCastOutdoor()) {
		Message_StringID(13, CAST_OUTDOORS);
		return false;
	}

	if (IsEffectInSpell(spell_id, SE_Levitate) && !zone->CanLevitate()) {
		Message(13, "You can't levitate in this zone.");
		return false;
	}

	if(zone->IsSpellBlocked(spell_id, glm::vec3(GetPosition()))) {
		const char *msg = zone->GetSpellBlockedMessage(spell_id, glm::vec3(GetPosition()));
		if (msg) {
			Message(13, msg);
			return false;
		} else {
			Message(13, "You can't cast this spell here.");
			return false;
		}
	}

	if (IsClient() && spells[spell_id].EndurTimerIndex > 0 && casting_spell_slot < CastingSlot::MaxGems)
		if (!CastToClient()->IsLinkedSpellReuseTimerReady(spells[spell_id].EndurTimerIndex))
			return false;

	casting_spell_checks = true;
	return true;
}

uint16 Mob::GetSpecializeSkillValue(uint16 spell_id) const {
	switch(spells[spell_id].skill) {
	case EQEmu::skills::SkillAbjuration:
		return(GetSkill(EQEmu::skills::SkillSpecializeAbjure));
	case EQEmu::skills::SkillAlteration:
		return(GetSkill(EQEmu::skills::SkillSpecializeAlteration));
	case EQEmu::skills::SkillConjuration:
		return(GetSkill(EQEmu::skills::SkillSpecializeConjuration));
	case EQEmu::skills::SkillDivination:
		return(GetSkill(EQEmu::skills::SkillSpecializeDivination));
	case EQEmu::skills::SkillEvocation:
		return(GetSkill(EQEmu::skills::SkillSpecializeEvocation));
	default:
		//wtf...
		break;
	}
	return(0);
}

void Client::CheckSpecializeIncrease(uint16 spell_id) {
	// These are not active because CheckIncreaseSkill() already does so.
	// It's such a rare occurance that adding them here is wasted..(ref only)
	/*
	if (IsDead() || IsUnconscious())
		return;
	if (IsAIControlled())
		return;
	*/

	switch(spells[spell_id].skill) {
	case EQEmu::skills::SkillAbjuration:
		CheckIncreaseSkill(EQEmu::skills::SkillSpecializeAbjure, nullptr);
		break;
	case EQEmu::skills::SkillAlteration:
		CheckIncreaseSkill(EQEmu::skills::SkillSpecializeAlteration, nullptr);
		break;
	case EQEmu::skills::SkillConjuration:
		CheckIncreaseSkill(EQEmu::skills::SkillSpecializeConjuration, nullptr);
		break;
	case EQEmu::skills::SkillDivination:
		CheckIncreaseSkill(EQEmu::skills::SkillSpecializeDivination, nullptr);
		break;
	case EQEmu::skills::SkillEvocation:
		CheckIncreaseSkill(EQEmu::skills::SkillSpecializeEvocation, nullptr);
		break;
	default:
		//wtf...
		break;
	}
}

void Client::CheckSongSkillIncrease(uint16 spell_id){
	// These are not active because CheckIncreaseSkill() already does so.
	// It's such a rare occurance that adding them here is wasted..(ref only)
	/*
	if (IsDead() || IsUnconscious())
		return;
	if (IsAIControlled())
		return;
	*/

	switch(spells[spell_id].skill)
	{
	case EQEmu::skills::SkillSinging:
		CheckIncreaseSkill(EQEmu::skills::SkillSinging, nullptr, -15);
		break;
	case EQEmu::skills::SkillPercussionInstruments:
		if(this->itembonuses.percussionMod > 0) {
			if (GetRawSkill(EQEmu::skills::SkillPercussionInstruments) > 0)	// no skill increases if not trained in the instrument
				CheckIncreaseSkill(EQEmu::skills::SkillPercussionInstruments, nullptr, -15);
			else
				Message_StringID(13,NO_INSTRUMENT_SKILL);	// tell the client that they need instrument training
		}
		else
			CheckIncreaseSkill(EQEmu::skills::SkillSinging, nullptr, -15);
		break;
	case EQEmu::skills::SkillStringedInstruments:
		if(this->itembonuses.stringedMod > 0) {
			if (GetRawSkill(EQEmu::skills::SkillStringedInstruments) > 0)
				CheckIncreaseSkill(EQEmu::skills::SkillStringedInstruments, nullptr, -15);
			else
				Message_StringID(13,NO_INSTRUMENT_SKILL);
		}
		else
			CheckIncreaseSkill(EQEmu::skills::SkillSinging, nullptr, -15);
		break;
	case EQEmu::skills::SkillWindInstruments:
		if(this->itembonuses.windMod > 0) {
			if (GetRawSkill(EQEmu::skills::SkillWindInstruments) > 0)
				CheckIncreaseSkill(EQEmu::skills::SkillWindInstruments, nullptr, -15);
			else
				Message_StringID(13,NO_INSTRUMENT_SKILL);
		}
		else
			CheckIncreaseSkill(EQEmu::skills::SkillSinging, nullptr, -15);
		break;
	case EQEmu::skills::SkillBrassInstruments:
		if(this->itembonuses.brassMod > 0) {
			if (GetRawSkill(EQEmu::skills::SkillBrassInstruments) > 0)
				CheckIncreaseSkill(EQEmu::skills::SkillBrassInstruments, nullptr, -15);
			else
				Message_StringID(13,NO_INSTRUMENT_SKILL);
		}
		else
			CheckIncreaseSkill(EQEmu::skills::SkillSinging, nullptr, -15);
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
		if(((specialize/6.0f) + 15.0f) < zone->random.Real(0, 100)) {
			specialize *= SPECIALIZE_FIZZLE / 200.0f;
		} else {
			specialize = 0.0f;
		}
	}

	// == 0 --> on par
	// > 0 --> skill is lower, higher chance of fizzle
	// < 0 --> skill is better, lower chance of fizzle
	// the max that diff can be is +- 235
	float diff = par_skill + static_cast<float>(spells[spell_id].basediff) - act_skill;

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

	float fizzle_roll = zone->random.Real(0, 100);

	Log(Logs::Detail, Logs::Spells, "Check Fizzle %s  spell %d  fizzlechance: %0.2f%%   diff: %0.2f  roll: %0.2f", GetName(), spell_id, fizzlechance, diff, fizzle_roll);

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
	casting_spell_slot = CastingSlot::Gem1;
	casting_spell_mana = 0;
	casting_spell_inventory_slot = 0;
	casting_spell_timer = 0;
	casting_spell_timer_duration = 0;
	casting_spell_resist_adjust = 0;
	casting_spell_checks = false;
	casting_spell_aa_id = 0;
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
	EQApplicationPacket *outapp = nullptr;
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
		CastToNPC()->AI_Event_SpellCastFinished(false, static_cast<uint16>(casting_spell_slot));
	}

	if(casting_spell_aa_id && IsClient()) { //Rest AA Timer on failed cast
		CastToClient()->Message_StringID(MT_SpellFailure, ABILITY_FAILED);
		CastToClient()->ResetAlternateAdvancementTimer(casting_spell_aa_id);
	}

	ZeroCastingVars();	// resets all the state keeping stuff

	Log(Logs::Detail, Logs::Spells, "Spell %d has been interrupted.", spellid);

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
	entity_list.QueueCloseClients(this, outapp, true, RuleI(Range, SongMessages), 0, true, IsClient() ? FilterPCSpells : FilterNPCSpells);
	safe_delete(outapp);

}

// this is like interrupt, just it doesn't spam interrupt packets to everyone
// There are a few cases where this is what live does :P
void Mob::StopCasting()
{
	if (casting_spell_id && IsNPC()) {
		CastToNPC()->AI_Event_SpellCastFinished(false, static_cast<uint16>(casting_spell_slot));
	}

	if (IsClient()) {
		auto c = CastToClient();
		if (casting_spell_aa_id) { //Rest AA Timer on failed cast
			c->Message_StringID(MT_SpellFailure, ABILITY_FAILED);
			c->ResetAlternateAdvancementTimer(casting_spell_aa_id);
		}

		auto outapp = new EQApplicationPacket(OP_ManaChange, sizeof(ManaChange_Struct));
		auto mc = (ManaChange_Struct *)outapp->pBuffer;
		mc->new_mana = GetMana();
		mc->stamina = GetEndurance();
		mc->spell_id = casting_spell_id;
		mc->keepcasting = 0;
		c->FastQueuePacket(&outapp);
	}
	ZeroCastingVars();
}

// this is called after the timer is up and the spell is finished
// casting. everything goes through here, including items with zero cast time
// only to be used from SpellProcess
// NOTE: do not put range checking, etc into this function. this should
// just check timed spell specific things before passing off to SpellFinished
// which figures out proper targets etc
void Mob::CastedSpellFinished(uint16 spell_id, uint32 target_id, CastingSlot slot,
							uint16 mana_used, uint32 inventory_slot, int16 resist_adjust)
{
	bool IsFromItem = false;

	if(IsClient() && slot != CastingSlot::Item && slot != CastingSlot::PotionBelt && spells[spell_id].recast_time > 1000) { // 10 is item
		if(!CastToClient()->GetPTimers().Expired(&database, pTimerSpellStart + spell_id, false)) {
			//should we issue a message or send them a spell gem packet?
			Message_StringID(13, SPELL_RECAST);
			Log(Logs::Detail, Logs::Spells, "Casting of %d canceled: spell reuse timer not expired", spell_id);
			StopCasting();
			return;
		}
	}

	if(IsClient() && (slot == CastingSlot::Item || slot == CastingSlot::PotionBelt))
	{
		IsFromItem = true;
		EQEmu::ItemInstance *itm = CastToClient()->GetInv().GetItem(inventory_slot);
		if(itm && itm->GetItem()->RecastDelay > 0)
		{
			if(!CastToClient()->GetPTimers().Expired(&database, (pTimerItemStart + itm->GetItem()->RecastType), false)) {
				Message_StringID(13, SPELL_RECAST);
				Log(Logs::Detail, Logs::Spells, "Casting of %d canceled: item spell reuse timer not expired", spell_id);
				StopCasting();
				return;
			}
		}
	}

	if(!IsValidSpell(spell_id))
	{
		Log(Logs::Detail, Logs::Spells, "Casting of %d canceled: invalid spell id", spell_id);
		InterruptSpell();
		return;
	}

	// prevent rapid recast - this can happen if somehow the spell gems
	// become desynced and the player casts again.
	if(IsClient())
	{
		if(delaytimer)
		{
			Log(Logs::Detail, Logs::Spells, "Casting of %d canceled: recast too quickly", spell_id);
			Message(13, "You are unable to focus.");
			InterruptSpell();
			return;
		}
	}

	// make sure they aren't somehow casting 2 timed spells at once
	if (casting_spell_id != spell_id)
	{
		Log(Logs::Detail, Logs::Spells, "Casting of %d canceled: already casting", spell_id);
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
			if(spells[spell_id].buffduration == 0xFFFF) {
				Log(Logs::Detail, Logs::Spells, "Bard song %d not applying bard logic because duration. dur=%d, recast=%d", spells[spell_id].buffduration);
			} else {
				// So long recast bard songs need special bard logic, although the effects don't repulse like other songs
				// This is basically a hack to get that effect
				// You can hold down the long recast spells, but you only get the effects once
				// TODO fuck bards.
				if (spells[spell_id].recast_time == 0) {
					bardsong = spell_id;
					bardsong_slot = slot;
					//NOTE: theres a lot more target types than this to think about...
					if (spell_target == nullptr || (spells[spell_id].targettype != ST_Target && spells[spell_id].targettype != ST_AETarget))
						bardsong_target_id = GetID();
					else
						bardsong_target_id = spell_target->GetID();
					bardsong_timer.Start(6000);
				}
				Log(Logs::Detail, Logs::Spells, "Bard song %d started: slot %d, target id %d", bardsong, bardsong_slot, bardsong_target_id);
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
				channelchance = 30 + GetSkill(EQEmu::skills::SkillChanneling) / 400.0f * 100;
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
				channelchance = 30 + GetSkill(EQEmu::skills::SkillChanneling) / 400.0f * 100;
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
				d_x = std::abs(std::abs(GetX()) - std::abs(GetSpellX()));
				d_y = std::abs(std::abs(GetY()) - std::abs(GetSpellY()));
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

			Log(Logs::Detail, Logs::Spells, "Checking Interruption: spell x: %f  spell y: %f  cur x: %f  cur y: %f channelchance %f channeling skill %d\n", GetSpellX(), GetSpellY(), GetX(), GetY(), channelchance, GetSkill(EQEmu::skills::SkillChanneling));

			if(!spells[spell_id].uninterruptable && zone->random.Real(0, 100) > channelchance) {
				Log(Logs::Detail, Logs::Spells, "Casting of %d canceled: interrupted.", spell_id);
				InterruptSpell();
				return;
			}
			// if we got here, we regained concentration
			regain_conc = true;
			Message_StringID(MT_Spells,REGAIN_AND_CONTINUE);
			entity_list.MessageClose_StringID(this, true, RuleI(Range, SpellMessages), MT_Spells, OTHER_REGAIN_CAST, this->GetCleanName());
		}
	}

	// Check for consumables and Reagent focus items
	// first check for component reduction
	if(IsClient()) {
		int reg_focus = CastToClient()->GetFocusEffect(focusReagentCost,spell_id);//Client only
		if(zone->random.Roll(reg_focus)) {
			Log(Logs::Detail, Logs::Spells, "Spell %d: Reagent focus item prevented reagent consumption (%d chance)", spell_id, reg_focus);
		} else {
			if(reg_focus > 0)
				Log(Logs::Detail, Logs::Spells, "Spell %d: Reagent focus item failed to prevent reagent consumption (%d chance)", spell_id, reg_focus);
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
							Log(Logs::Detail, Logs::Spells, "Something odd happened: Song %d required component %d", spell_id, component);
					}

					if(!HasInstrument) {	// if the instrument is missing, log it and interrupt the song
						Log(Logs::Detail, Logs::Spells, "Song %d: Canceled. Missing required instrument %d", spell_id, component);
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

						const EQEmu::ItemData *item = database.GetItem(component);
						if(item) {
							c->Message_StringID(13, MISSING_SPELL_COMP_ITEM, item->Name);
							Log(Logs::Detail, Logs::Spells, "Spell %d: Canceled. Missing required reagent %s (%d)", spell_id, item->Name, component);
						}
						else {
							char TempItemName[64];
							strcpy((char*)&TempItemName, "UNKNOWN");
							Log(Logs::Detail, Logs::Spells, "Spell %d: Canceled. Missing required reagent %s (%d)", spell_id, TempItemName, component);
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
				int noexpend;
				for(int t_count = 0; t_count < 4; t_count++) {
					component = spells[spell_id].components[t_count];
					noexpend = spells[spell_id].NoexpendReagent[t_count];
					if (component == -1 || noexpend == component)
						continue;
					component_count = spells[spell_id].component_counts[t_count];
					Log(Logs::Detail, Logs::Spells, "Spell %d: Consuming %d of spell component item id %d", spell_id, component_count, component);
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

	if(IsClient() && (slot == CastingSlot::Item || slot == CastingSlot::PotionBelt)
		&& inventory_slot != 0xFFFFFFFF)	// 10 is an item
	{
		bool fromaug = false;
		const EQEmu::ItemInstance* inst = CastToClient()->GetInv()[inventory_slot];
		EQEmu::ItemData* augitem = nullptr;
		uint32 recastdelay = 0;
		uint32 recasttype = 0;

		while (true) {
			if (inst == nullptr)
				break;

			for (int r = EQEmu::inventory::socketBegin; r < EQEmu::inventory::SocketCount; r++) {
				const EQEmu::ItemInstance* aug_i = inst->GetAugment(r);

				if (!aug_i)
					continue;
				const EQEmu::ItemData* aug = aug_i->GetItem();
				if (!aug)
					continue;

				if (aug->Click.Effect == spell_id)
				{
					recastdelay = aug_i->GetItem()->RecastDelay;
					recasttype = aug_i->GetItem()->RecastType;
					fromaug = true;
					break;
				}
			}

			break;
		}

		//Test the aug recast delay
		if(IsClient() && fromaug && recastdelay > 0)
		{
			if(!CastToClient()->GetPTimers().Expired(&database, (pTimerItemStart + recasttype), false)) {
				Message_StringID(13, SPELL_RECAST);
				Log(Logs::Detail, Logs::Spells, "Casting of %d canceled: item spell reuse timer not expired", spell_id);
				StopCasting();
				return;
			}
			else
			{
				//Can we start the timer here?  I don't see why not.
				CastToClient()->GetPTimers().Start((pTimerItemStart + recasttype), recastdelay);
				database.UpdateItemRecastTimestamps(CastToClient()->CharacterID(), recasttype,
								CastToClient()->GetPTimers().Get(pTimerItemStart + recasttype)->GetReadyTimestamp());
			}
		}

		if (inst && inst->IsClassCommon() && (inst->GetItem()->Click.Effect == spell_id) && inst->GetCharges() || fromaug)
		{
			//const ItemData* item = inst->GetItem();
			int16 charges = inst->GetItem()->MaxCharges;

			if(fromaug) { charges = -1; } //Don't destroy the parent item

			if(charges > -1) {	// charged item, expend a charge
				Log(Logs::Detail, Logs::Spells, "Spell %d: Consuming a charge from item %s (%d) which had %d/%d charges.", spell_id, inst->GetItem()->Name, inst->GetItem()->ID, inst->GetCharges(), inst->GetItem()->MaxCharges);
				DeleteChargeFromSlot = inventory_slot;
			} else {
				Log(Logs::Detail, Logs::Spells, "Spell %d: Cast from unlimited charge item %s (%d) (%d charges)", spell_id, inst->GetItem()->Name, inst->GetItem()->ID, inst->GetItem()->MaxCharges);
			}
		}
		else
		{
			Log(Logs::Detail, Logs::Spells, "Item used to cast spell %d was missing from inventory slot %d after casting!", spell_id, inventory_slot);
			Message(13, "Casting Error: Active casting item not found in inventory slot %i", inventory_slot);
			InterruptSpell();
			return;
		}
	}

	// we're done casting, now try to apply the spell
	if( !SpellFinished(spell_id, spell_target, slot, mana_used, inventory_slot, resist_adjust) )
	{
		Log(Logs::Detail, Logs::Spells, "Casting of %d canceled: SpellFinished returned false.", spell_id);
		// most of the cases we return false have a message already or are logic errors that shouldn't happen
		// if there are issues I guess we can do something else, but this should work
		StopCasting();
		return;
	}

	if(IsClient()) {
		CheckNumHitsRemaining(NumHit::MatchingSpells);
		TrySympatheticProc(target, spell_id);
	}

	TryTwincast(this, target, spell_id);

	TryTriggerOnCast(spell_id, 0);

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
			Client *c = CastToClient();
			c->CheckSongSkillIncrease(spell_id);
			if (spells[spell_id].EndurTimerIndex > 0 && slot < CastingSlot::MaxGems)
				c->SetLinkedSpellReuseTimer(spells[spell_id].EndurTimerIndex, spells[spell_id].recast_time / 1000);
			c->MemorizeSpell(static_cast<uint32>(slot), spell_id, memSpellSpellbar);
		}
		Log(Logs::Detail, Logs::Spells, "Bard song %d should be started", spell_id);
	}
	else
	{
		if(IsClient())
		{
			Client *c = CastToClient();
			SendSpellBarEnable(spell_id);

			// this causes the delayed refresh of the spell bar gems
			if (spells[spell_id].EndurTimerIndex > 0 && slot < CastingSlot::MaxGems)
				c->SetLinkedSpellReuseTimer(spells[spell_id].EndurTimerIndex, spells[spell_id].recast_time / 1000);
			c->MemorizeSpell(static_cast<uint32>(slot), spell_id, memSpellSpellbar);

			// this tells the client that casting may happen again
			SetMana(GetMana());

			// skills
			if (EQEmu::skills::IsCastingSkill(spells[spell_id].skill)) {
				c->CheckIncreaseSkill(spells[spell_id].skill, nullptr);

				// increased chance of gaining channel skill if you regained concentration
				c->CheckIncreaseSkill(EQEmu::skills::SkillChanneling, nullptr, regain_conc ? 5 : 0);

				c->CheckSpecializeIncrease(spell_id);
			}
		}
	}

	// there should be no casting going on now
	ZeroCastingVars();

	// set the rapid recast timer for next time around
	// Why do we have this? It mostly just causes issues when things are working correctly
	// It also needs to be <users's ping to not cause issues
	delaytimer = true;
	spellend_timer.Start(10, true);

	Log(Logs::Detail, Logs::Spells, "Spell casting of %d is finished.", spell_id);

}

bool Mob::DetermineSpellTargets(uint16 spell_id, Mob *&spell_target, Mob *&ae_center, CastAction_type &CastAction, CastingSlot slot, bool isproc)
{
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
		&& (HasProjectIllusion())){
			Log(Logs::Detail, Logs::AA, "Project Illusion overwrote target caster: %s spell id: %d was ON", GetName(), spell_id);
			targetType = ST_GroupClientAndPet;
	}

	// NPC innate procs override the target type to single target.
	// Yes. This code will cause issues if they have the proc as innate AND on a weapon. Oh well.
	if (isproc && IsNPC() && CastToNPC()->GetInnateProcSpellID() == spell_id)
		targetType = ST_Target;

	if (spell_target && !spell_target->PassCastRestriction(true, spells[spell_id].CastRestriction)){
		Message_StringID(13,SPELL_NEED_TAR);
		return false;
	}

	//Must be out of combat. (If Beneficial checks casters combat state, Deterimental checks targets)
	if (!spells[spell_id].InCombat && spells[spell_id].OutofCombat) {
		if (IsDetrimentalSpell(spell_id)) {
			if (spell_target &&
			    ((spell_target->IsNPC() && spell_target->IsEngaged()) ||
			     (spell_target->IsClient() && spell_target->CastToClient()->GetAggroCount()))) {
				Message_StringID(13, SPELL_NO_EFFECT); // Unsure correct string
				return false;
			}
		}

		else if (IsBeneficialSpell(spell_id)) {
			if ((IsNPC() && IsEngaged()) || (IsClient() && CastToClient()->GetAggroCount())) {
				if (IsDiscipline(spell_id))
					Message_StringID(13, NO_ABILITY_IN_COMBAT);
				else
					Message_StringID(13, NO_CAST_IN_COMBAT);

				return false;
			}
		}
	}

	// Must be in combat. (If Beneficial checks casters combat state, Deterimental checks targets)
	else if (spells[spell_id].InCombat && !spells[spell_id].OutofCombat) {
		if (IsDetrimentalSpell(spell_id)) {
			if (spell_target &&
			    ((spell_target->IsNPC() && !spell_target->IsEngaged()) ||
			     (spell_target->IsClient() && !spell_target->CastToClient()->GetAggroCount()))) {
				Message_StringID(13, SPELL_NO_EFFECT); // Unsure correct string
				return false;
			}
		}

		else if (IsBeneficialSpell(spell_id)) {
			if ((IsNPC() && !IsEngaged()) || (IsClient() && !CastToClient()->GetAggroCount())) {
				if (IsDiscipline(spell_id))
					Message_StringID(13, NO_ABILITY_OUT_OF_COMBAT);
				else
					Message_StringID(13, NO_CAST_OUT_OF_COMBAT);

				return false;
			}
		}
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
				mob_body != BT_SummonedUndead
				&& mob_body != BT_Undead
				&& mob_body != BT_Vampire
				)
			)
			{
				//invalid target
				Log(Logs::Detail, Logs::Spells, "Spell %d canceled: invalid target of body type %d (undead)", spell_id, mob_body);
				if(!spell_target)
					Message_StringID(13,SPELL_NEED_TAR);
				else
					Message_StringID(13,CANNOT_AFFECT_NPC);
				return false;
			}
			CastAction = SingleTarget;
			break;
		}

		case ST_Summoned: {
			if(!spell_target || (mob_body != BT_Summoned && mob_body != BT_Summoned2 && mob_body != BT_Summoned3))
			{
				//invalid target
				Log(Logs::Detail, Logs::Spells, "Spell %d canceled: invalid target of body type %d (summoned)", spell_id, mob_body);
				Message_StringID(13,SPELL_NEED_TAR);
				return false;
			}
			CastAction = SingleTarget;
			break;
		}

		case ST_SummonedPet:
		{
			if(!spell_target || (spell_target != GetPet()) ||
				(mob_body != BT_Summoned && mob_body != BT_Summoned2 && mob_body != BT_Summoned3 && mob_body != BT_Animal))
			{
				Log(Logs::Detail, Logs::Spells, "Spell %d canceled: invalid target of body type %d (summoned pet)",
							spell_id, mob_body);

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
				Log(Logs::Detail, Logs::Spells, "Spell %d canceled: invalid target of body type %d (want body Type %d)", spell_id, mob_body, target_bt);
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
					Log(Logs::Detail, Logs::Spells, "Spell %d canceled: invalid target (ldon object)", spell_id);
					Message_StringID(13,SPELL_NEED_TAR);
					return false;
				}
				else
				{
					if(!spell_target->IsNPC())
					{
						Log(Logs::Detail, Logs::Spells, "Spell %d canceled: invalid target (normal)", spell_id);
						Message_StringID(13,SPELL_NEED_TAR);
						return false;
					}

					if(spell_target->GetClass() != LDON_TREASURE)
					{
						Log(Logs::Detail, Logs::Spells, "Spell %d canceled: invalid target (normal)", spell_id);
						Message_StringID(13,SPELL_NEED_TAR);
						return false;
					}
				}
			}

			if(!spell_target)
			{
				Log(Logs::Detail, Logs::Spells, "Spell %d canceled: invalid target (normal)", spell_id);
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
				Log(Logs::Detail, Logs::Spells, "Spell %d canceled: invalid target (corpse)", spell_id);
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
				Log(Logs::Detail, Logs::Spells, "Spell %d canceled: invalid target (no pet)", spell_id);
				Message_StringID(13,NO_PET);
				return false;	// can't cast these unless we have a target
			}
			CastAction = SingleTarget;
			break;
		}

		case ST_AEBard:
		case ST_AECaster:
		case ST_AEClientV1:
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

		case ST_AETargetHateList:
		{
			if (spells[spell_id].range > 0)
			{
				if(!spell_target)
					return false;

				ae_center = spell_target;
				CastAction = AETarget;
			}
			else {
				spell_target = nullptr;
				ae_center = this;
				CastAction = CAHateList;
			}
			break;
		}

		case ST_AreaClientOnly:
		case ST_AreaNPCOnly:
		{
			if (spells[spell_id].range > 0)
			{
				if(!spell_target)
					return false;

				ae_center = spell_target;
				CastAction = AETarget;
			}
			else {
				spell_target = nullptr;
				ae_center = this;
				CastAction = AECaster;
			}
			break;
		}

		case ST_UndeadAE:	//should only affect undead...
		case ST_SummonedAE:
		case ST_TargetAETap:
		case ST_AETarget:
		case ST_TargetAENoPlayersPets:
		{
			if(!spell_target)
			{
				Log(Logs::Detail, Logs::Spells, "Spell %d canceled: invalid target (AOE)", spell_id);
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
		case ST_GroupNoPets:
		{
			if(IsClient() && CastToClient()->TGB() && IsTGBCompatibleSpell(spell_id) && (slot != CastingSlot::Item || RuleB(Spells, AllowItemTGB))) {
				if( (!target) ||
					(target->IsNPC() && !(target->GetOwner() && target->GetOwner()->IsClient())) ||
					(target->IsCorpse()) )
					spell_target = this;
				else
					spell_target = target;
			} else {
				spell_target = this;
			}

			if (spell_target && spell_target->IsPet() && spells[spell_id].targettype == ST_GroupNoPets){
				Message_StringID(13,NO_CAST_ON_PET);
				return false;
			}

			CastAction = GroupSpell;
			break;
		}
		case ST_GroupClientAndPet:
		{
			if(!spell_target)
			{
				Log(Logs::Detail, Logs::Spells, "Spell %d canceled: invalid target (Group Required: Single Target)", spell_id);
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
							if (Group* group = GetGroup()) {
								group_id_caster = group->GetID();
							}
						}
						else if(IsRaidGrouped())
						{
							if (Raid* raid = GetRaid()) {
								uint32 group_id = raid->GetGroup(CastToClient());
								group_id_caster = (group_id == 0xFFFFFFFF) ? 0 : (group_id + 1);
							}
						}
					}
					else if(IsPet())
					{
						Mob *owner = GetOwner();
						if(owner->IsGrouped())
						{
							if (Group* group = owner->GetGroup()) {
								group_id_caster = group->GetID();
							}
						}
						else if(owner->IsRaidGrouped())
						{
							if (Raid* raid = owner->GetRaid()) {
								uint32 group_id = raid->GetGroup(owner->CastToClient());
								group_id_caster = (group_id == 0xFFFFFFFF) ? 0 : (group_id + 1);
							}
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
							if (Group* group = spell_target->GetGroup()) {
								group_id_target = group->GetID();
							}
						}
						else if(spell_target->IsRaidGrouped())
						{
							if (Raid* raid = spell_target->GetRaid()) {
								uint32 group_id = raid->GetGroup(spell_target->CastToClient());
								group_id_target = (group_id == 0xFFFFFFFF) ? 0 : (group_id + 1);
							}
						}
					}
					else if(spell_target->IsPet())
					{
						Mob *owner = spell_target->GetOwner();
						if(owner->IsGrouped())
						{
							if (Group* group = owner->GetGroup()) {
								group_id_target = group->GetID();
							}
						}
						else if(owner->IsRaidGrouped())
						{
							if (Raid* raid = owner->GetRaid()) {
								uint32 group_id = raid->GetGroup(owner->CastToClient());
								group_id_target = (group_id == 0xFFFFFFFF) ? 0 : (group_id + 1);
							}
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
						Log(Logs::Detail, Logs::Spells, "Spell %d canceled: Attempted to cast a Single Target Group spell on a ungrouped member.", spell_id);
						Message_StringID(13, TARGET_GROUP_MEMBER);
						return false;
					}

					if(group_id_caster != group_id_target)
					{
						Log(Logs::Detail, Logs::Spells, "Spell %d canceled: Attempted to cast a Single Target Group spell on a ungrouped member.", spell_id);
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

		case ST_PetMaster:
		{

			Mob *owner = nullptr;

			if (IsPet())
				owner = GetOwner();
			else if ((IsNPC() && CastToNPC()->GetSwarmOwner()))
				owner = entity_list.GetMobID(CastToNPC()->GetSwarmOwner());

			if (!owner)
				return false;

			spell_target = owner;
			CastAction = SingleTarget;
			break;
		}

		case ST_Beam:
		{
			CastAction = Beam;
			spell_target = nullptr;
			ae_center = nullptr;
			break;
		}

		case ST_Ring:
		{
			CastAction = TargetRing;
			spell_target = nullptr;
			ae_center = nullptr;
			break;
		}

		default:
		{
			Log(Logs::Detail, Logs::Spells, "I dont know Target Type: %d   Spell: (%d) %s", spells[spell_id].targettype, spell_id, spells[spell_id].name);
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
bool Mob::SpellFinished(uint16 spell_id, Mob *spell_target, CastingSlot slot, uint16 mana_used,
						uint32 inventory_slot, int16 resist_adjust, bool isproc, int level_override)
{
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

		if(zone->IsSpellBlocked(spell_id, glm::vec3(GetPosition()))){
			const char *msg = zone->GetSpellBlockedMessage(spell_id, glm::vec3(GetPosition()));
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

	if (IsClient() && CastToClient()->GetGM()){
		if (zone->IsSpellBlocked(spell_id, glm::vec3(GetPosition()))){
			Log(Logs::Detail, Logs::Spells, "GM Cast Blocked Spell: %s (ID %i)", GetSpellName(spell_id), spell_id);
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
	if(!DetermineSpellTargets(spell_id, spell_target, ae_center, CastAction, slot, isproc))
		return(false);

	Log(Logs::Detail, Logs::Spells, "Spell %d: target type %d, target %s, AE center %s", spell_id, CastAction, spell_target?spell_target->GetName():"NONE", ae_center?ae_center->GetName():"NONE");

	// if a spell has the AEDuration flag, it becomes an AE on target
	// spell that's recast every 2500 msec for AEDuration msec. There are
	// spells of all kinds of target types that do this, strangely enough
	// TODO: finish this
	if(IsAEDurationSpell(spell_id)) {
		// the spells are AE target, but we aim them on a beacon
		Mob *beacon_loc = spell_target ? spell_target : this;
		auto beacon = new Beacon(beacon_loc, spells[spell_id].AEDuration);
		entity_list.AddBeacon(beacon);
		Log(Logs::Detail, Logs::Spells, "Spell %d: AE duration beacon created, entity id %d", spell_id, beacon->GetName());
		spell_target = nullptr;
		ae_center = beacon;
		CastAction = AECaster;
	}

	// check line of sight to target if it's a detrimental spell
	if(!spells[spell_id].npc_no_los && spell_target && IsDetrimentalSpell(spell_id) && !CheckLosFN(spell_target) && !IsHarmonySpell(spell_id) && spells[spell_id].targettype != ST_TargetOptional)
	{
		Log(Logs::Detail, Logs::Spells, "Spell %d: cannot see target %s", spell_id, spell_target->GetName());
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
	float range = spells[spell_id].range + GetRangeDistTargetSizeMod(spell_target);
	if(IsClient() && CastToClient()->TGB() && IsTGBCompatibleSpell(spell_id) && IsGroupSpell(spell_id))
		range = spells[spell_id].aoerange;

	range = GetActSpellRange(spell_id, range);
	if(IsPlayerIllusionSpell(spell_id)
		&& IsClient()
		&& (HasProjectIllusion())){
		range = 100;
	}
	if(spell_target != nullptr && spell_target != this) {
		//casting a spell on somebody but ourself, make sure they are in range
		float dist2 = DistanceSquared(m_Position, spell_target->GetPosition());
		float range2 = range * range;
		float min_range2 = spells[spell_id].min_range * spells[spell_id].min_range;
		if(dist2 > range2) {
			//target is out of range.
			Log(Logs::Detail, Logs::Spells, "Spell %d: Spell target is out of range (squared: %f > %f)", spell_id, dist2, range2);
			Message_StringID(13, TARGET_OUT_OF_RANGE);
			return(false);
		}
		else if (dist2 < min_range2){
			//target is too close range.
			Log(Logs::Detail, Logs::Spells, "Spell %d: Spell target is too close (squared: %f < %f)", spell_id, dist2, min_range2);
			Message_StringID(13, TARGET_TOO_CLOSE);
			return(false);
		}

		spell_target->CalcSpellPowerDistanceMod(spell_id, dist2);
	}
	//AE Duration spells were ignoring distance check from item clickies
	if(ae_center != nullptr && ae_center != this) {
		//casting a spell on somebody but ourself, make sure they are in range
		float dist2 = DistanceSquared(m_Position, ae_center->GetPosition());
		float range2 = range * range;
		float min_range2 = spells[spell_id].min_range * spells[spell_id].min_range;
		if(dist2 > range2) {
			//target is out of range.
			Log(Logs::Detail, Logs::Spells, "Spell %d: Spell target is out of range (squared: %f > %f)", spell_id, dist2, range2);
			Message_StringID(13, TARGET_OUT_OF_RANGE);
			return(false);
		}
		else if (dist2 < min_range2){
			//target is too close range.
			Log(Logs::Detail, Logs::Spells, "Spell %d: Spell target is too close (squared: %f < %f)", spell_id, dist2, min_range2);
			Message_StringID(13, TARGET_TOO_CLOSE);
			return(false);
		}

		ae_center->CalcSpellPowerDistanceMod(spell_id, dist2);
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
				Log(Logs::Detail, Logs::Spells, "Spell %d: Targeted spell, but we have no target", spell_id);
				return(false);
			}
			if (isproc) {
				SpellOnTarget(spell_id, spell_target, false, true, resist_adjust, true, level_override);
			} else {
				if (spells[spell_id].targettype == ST_TargetOptional){
					if (!TrySpellProjectile(spell_target, spell_id))
						return false;
				}

				else if(!SpellOnTarget(spell_id, spell_target, false, true, resist_adjust, false, level_override)) {
					if(IsBuffSpell(spell_id) && IsBeneficialSpell(spell_id)) {
						// Prevent mana usage/timers being set for beneficial buffs
						if(casting_spell_aa_id)
							InterruptSpell();
						return false;
					}
				}
			}

			if(IsPlayerIllusionSpell(spell_id)
			&& IsClient()
			&& (HasProjectIllusion())){
				Log(Logs::Detail, Logs::AA, "Effect Project Illusion for %s on spell id: %d was ON", GetName(), spell_id);
				SetProjectIllusion(false);
			}
			else{
				Log(Logs::Detail, Logs::AA, "Effect Project Illusion for %s on spell id: %d was OFF", GetName(), spell_id);
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
				// unsure if we actually need this? Need to find some spell examples
				if(ae_center && ae_center == this && IsBeneficialSpell(spell_id))
					SpellOnTarget(spell_id, this);

				// NPCs should never be affected by an AE they cast. PB AEs shouldn't affect caster either
				// I don't think any other cases that get here matter
				bool affect_caster = !IsNPC() && spells[spell_id].targettype != ST_AECaster;

				if (spells[spell_id].targettype == ST_AETargetHateList)
					hate_list.SpellCast(this, spell_id, spells[spell_id].aoerange, ae_center);
				else
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

			// We hold off turning MBG off so we can still use it to calc the mana cost
			if(spells[spell_id].can_mgb && HasMGB())
			{
				SpellOnTarget(spell_id, this);
				entity_list.MassGroupBuff(this, this, spell_id, true);
			}
			else
			{
				// at this point spell_target is a member of the other group, or the
				// caster if they're not using TGB
				// NOTE: this will always hit the caster, plus the target's group so
				// it can affect up to 7 people if the targeted group is not our own

				// Allow pets who cast group spells to affect the group.
				if (spell_target->IsPetOwnerClient() && IsPetOwnerClient()){
					Mob* owner =  spell_target->GetOwner();

					if (owner)
						spell_target = owner;
				}

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
						if (spells[spell_id].targettype != ST_GroupNoPets && GetPet() && HasPetAffinity() && !GetPet()->IsCharmed())
							SpellOnTarget(spell_id, GetPet());
	#endif
					}

					SpellOnTarget(spell_id, spell_target);
	#ifdef GROUP_BUFF_PETS
					//pet too
					if (spells[spell_id].targettype != ST_GroupNoPets && spell_target->GetPet() && HasPetAffinity() && !spell_target->GetPet()->IsCharmed())
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
			ConeDirectional(spell_id, resist_adjust);
			break;
		}

		case Beam:
		{
			BeamDirectional(spell_id, resist_adjust);
 			break;
 		}

		case TargetRing:
		{
			entity_list.AESpell(this, nullptr, spell_id, false, resist_adjust);
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

	bool mgb = HasMGB() && spells[spell_id].can_mgb;
	// if this was a spell slot or an ability use up the mana for it
	if(slot != CastingSlot::Item && slot != CastingSlot::PotionBelt && mana_used > 0)
	{
		mana_used = GetActSpellCost(spell_id, mana_used);
		if (mgb) {
			mana_used *= 2;
		}
		// clamp if we some how got focused above our current mana
		if (GetMana() < mana_used)
			mana_used = GetMana();
		Log(Logs::Detail, Logs::Spells, "Spell %d: consuming %d mana", spell_id, mana_used);
		if (!DoHPToManaCovert(mana_used)) {
			SetMana(GetMana() - mana_used);
			TryTriggerOnValueAmount(false, true);
		}
	}
	// one may want to check if this is a disc or not, but we actually don't, there are non disc stuff that have end cost
	// lets not consume end for custom items that have disc procs.
	// One might also want to filter out USE_ITEM_SPELL_SLOT, but DISCIPLINE_SPELL_SLOT are both #defined to the same thing ...
	if (spells[spell_id].EndurCost && !isproc) {
		auto end_cost = spells[spell_id].EndurCost;
		if (mgb)
			end_cost *= 2;
		SetEndurance(GetEndurance() - EQEmu::ClampUpper(end_cost, GetEndurance()));
		TryTriggerOnValueAmount(false, false, true);
	}
	if (mgb)
		SetMGB(false);

	//set our reuse timer on long ass reuse_time spells...
	if(IsClient() && !isproc)
	{
		if(casting_spell_aa_id) {
			AA::Rank *rank = zone->GetAlternateAdvancementRank(casting_spell_aa_id);

			if(rank && rank->base_ability) {
				ExpendAlternateAdvancementCharge(rank->base_ability->id);
			}
		}
		else if(spell_id == casting_spell_id && casting_spell_timer != 0xFFFFFFFF)
		{
			//aa new todo: aa expendable charges here
			CastToClient()->GetPTimers().Start(casting_spell_timer, casting_spell_timer_duration);
			Log(Logs::Detail, Logs::Spells, "Spell %d: Setting custom reuse timer %d to %d", spell_id, casting_spell_timer, casting_spell_timer_duration);
		}
		else if(spells[spell_id].recast_time > 1000 && !spells[spell_id].IsDisciplineBuff) {
			int recast = spells[spell_id].recast_time/1000;
			if (spell_id == SPELL_LAY_ON_HANDS)	//lay on hands
			{
				recast -= GetAA(aaFervrentBlessing) * 420;
			}
			else if (spell_id == SPELL_HARM_TOUCH || spell_id == SPELL_HARM_TOUCH2)	//harm touch
			{
				recast -= GetAA(aaTouchoftheWicked) * 420;
			}
			int reduction = CastToClient()->GetFocusEffect(focusReduceRecastTime, spell_id);//Client only
			if(reduction)
				recast -= reduction;

			Log(Logs::Detail, Logs::Spells, "Spell %d: Setting long reuse timer to %d s (orig %d)", spell_id, recast, spells[spell_id].recast_time);
			CastToClient()->GetPTimers().Start(pTimerSpellStart + spell_id, recast);
		}
	}

	if(IsClient() && (slot == CastingSlot::Item || slot == CastingSlot::PotionBelt))
	{
		EQEmu::ItemInstance *itm = CastToClient()->GetInv().GetItem(inventory_slot);
		if(itm && itm->GetItem()->RecastDelay > 0){
			auto recast_type = itm->GetItem()->RecastType;
			CastToClient()->GetPTimers().Start((pTimerItemStart + recast_type), itm->GetItem()->RecastDelay);
			database.UpdateItemRecastTimestamps(
			    CastToClient()->CharacterID(), recast_type,
			    CastToClient()->GetPTimers().Get(pTimerItemStart + recast_type)->GetReadyTimestamp());
			auto outapp = new EQApplicationPacket(OP_ItemRecastDelay, sizeof(ItemRecastDelay_Struct));
			ItemRecastDelay_Struct *ird = (ItemRecastDelay_Struct *)outapp->pBuffer;
			ird->recast_delay = itm->GetItem()->RecastDelay;
			ird->recast_type = recast_type;
			CastToClient()->QueuePacket(outapp);
			safe_delete(outapp);
		}
	}

	if(IsNPC())
		CastToNPC()->AI_Event_SpellCastFinished(true, static_cast<uint16>(slot));

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
bool Mob::ApplyNextBardPulse(uint16 spell_id, Mob *spell_target, CastingSlot slot) {
	if(slot == CastingSlot::Item) {
		//bard songs should never come from items...
		Log(Logs::Detail, Logs::Spells, "Bard Song Pulse %d: Supposidly cast from an item. Killing song.", spell_id);
		return(false);
	}

	//determine the type of spell target we have
	Mob *ae_center = nullptr;
	CastAction_type CastAction;
	if(!DetermineSpellTargets(spell_id, spell_target, ae_center, CastAction, slot)) {
		Log(Logs::Detail, Logs::Spells, "Bard Song Pulse %d: was unable to determine target. Stopping.", spell_id);
		return(false);
	}

	if(ae_center != nullptr && ae_center->IsBeacon()) {
		Log(Logs::Detail, Logs::Spells, "Bard Song Pulse %d: Unsupported Beacon NPC AE spell", spell_id);
		return(false);
	}

	//use mana, if this spell has a mana cost
	int mana_used = spells[spell_id].mana;
	if(mana_used > 0) {
		if(mana_used > GetMana()) {
			//ran out of mana... this calls StopSong() for us
			Log(Logs::Detail, Logs::Spells, "Ran out of mana while singing song %d", spell_id);
			return(false);
		}

		Log(Logs::Detail, Logs::Spells, "Bard Song Pulse %d: consuming %d mana (have %d)", spell_id, mana_used, GetMana());
		SetMana(GetMana() - mana_used);
	}

	// check line of sight to target if it's a detrimental spell
	if(spell_target && IsDetrimentalSpell(spell_id) && !CheckLosFN(spell_target))
	{
		Log(Logs::Detail, Logs::Spells, "Bard Song Pulse %d: cannot see target %s", spell_target->GetName());
		Message_StringID(13, CANT_SEE_TARGET);
		return(false);
	}

	//range check our target, if we have one and it is not us
	float range = 0.00f;

	range = GetActSpellRange(spell_id, spells[spell_id].range, true);
	if(spell_target != nullptr && spell_target != this) {
		//casting a spell on somebody but ourself, make sure they are in range
		float dist2 = DistanceSquared(m_Position, spell_target->GetPosition());
		float range2 = range * range;
		if(dist2 > range2) {
			//target is out of range.
			Log(Logs::Detail, Logs::Spells, "Bard Song Pulse %d: Spell target is out of range (squared: %f > %f)", spell_id, dist2, range2);
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
				Log(Logs::Detail, Logs::Spells, "Bard Song Pulse %d: Targeted spell, but we have no target", spell_id);
				return(false);
			}
			Log(Logs::Detail, Logs::Spells, "Bard Song Pulse: Targeted. spell %d, target %s", spell_id, spell_target->GetName());
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
				Log(Logs::Detail, Logs::Spells, "Bard Song Pulse %d: AE Targeted spell, but we have no target", spell_id);
				return(false);
			}

			// regular PB AE or targeted AE spell - spell_target is null if PB
			if(spell_target) {	// this must be an AETarget spell
				// affect the target too
				spell_target->BardPulse(spell_id, this);
				Log(Logs::Detail, Logs::Spells, "Bard Song Pulse: spell %d, AE target %s", spell_id, spell_target->GetName());
			} else {
				Log(Logs::Detail, Logs::Spells, "Bard Song Pulse: spell %d, AE with no target", spell_id);
			}
			bool affect_caster = !IsNPC();	//NPC AE spells do not affect the NPC caster
			entity_list.AEBardPulse(this, ae_center, spell_id, affect_caster);
			break;
		}

		case GroupSpell:
		{
			if(spell_target->IsGrouped()) {
				Log(Logs::Detail, Logs::Spells, "Bard Song Pulse: spell %d, Group targeting group of %s", spell_id, spell_target->GetName());
				Group *target_group = entity_list.GetGroupByMob(spell_target);
				if(target_group)
					target_group->GroupBardPulse(this, spell_id);
			}
			else if(spell_target->IsRaidGrouped() && spell_target->IsClient()) {
				Log(Logs::Detail, Logs::Spells, "Bard Song Pulse: spell %d, Raid group targeting raid group of %s", spell_id, spell_target->GetName());
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
				Log(Logs::Detail, Logs::Spells, "Bard Song Pulse: spell %d, Group target without group. Affecting caster.", spell_id);
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
	int buff_count = GetMaxTotalSlots();
	for (buffs_i = 0; buffs_i < buff_count; buffs_i++) {
		if(buffs[buffs_i].spellid != spell_id)
			continue;
		if(buffs[buffs_i].casterid != caster->GetID()) {
			Log(Logs::Detail, Logs::Spells, "Bard Pulse for %d: found buff from caster %d and we are pulsing for %d... are there two bards playing the same song???", spell_id, buffs[buffs_i].casterid, caster->GetID());
			return;
		}
		//extend the spell if it will expire before the next pulse
		if(buffs[buffs_i].ticsremaining <= 3) {
			buffs[buffs_i].ticsremaining += 3;
			Log(Logs::Detail, Logs::Spells, "Bard Song Pulse %d: extending duration in slot %d to %d tics", spell_id, buffs_i, buffs[buffs_i].ticsremaining);
		}

		//should we send this buff update to the client... seems like it would
		//be a lot of traffic for no reason...
//this may be the wrong packet...
		if(IsClient()) {
			auto packet = new EQApplicationPacket(OP_Action, sizeof(Action_Struct));

			Action_Struct* action = (Action_Struct*) packet->pBuffer;
			action->source = caster->GetID();
			action->target = GetID();
			action->spell = spell_id;
			action->sequence = (uint32) (GetHeading() * 2);	// just some random number
			action->instrument_mod = caster->GetInstrumentMod(spell_id);
			action->buff_unknown = 0;
			action->level = buffs[buffs_i].casterlevel;
			action->type = DamageTypeSpell;
			entity_list.QueueCloseClients(this, packet, false, RuleI(Range, SongMessages), 0, true, IsClient() ? FilterPCSpells : FilterNPCSpells);

			action->buff_unknown = 4;

			if(IsEffectInSpell(spell_id, SE_TossUp))
			{
				action->buff_unknown = 0;
			}
			else if(spells[spell_id].pushback > 0 || spells[spell_id].pushup > 0)
			{
				if(IsClient())
				{
					if(!IsBuffSpell(spell_id))
					{
						CastToClient()->SetKnockBackExemption(true);

						action->buff_unknown = 0;
						auto outapp_push = new EQApplicationPacket(
						    OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
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

			auto message_packet = new EQApplicationPacket(OP_Damage, sizeof(CombatDamage_Struct));
			CombatDamage_Struct *cd = (CombatDamage_Struct *)message_packet->pBuffer;
			cd->target = action->target;
			cd->source = action->source;
			cd->type = DamageTypeSpell;
			cd->spellid = action->spell;
			cd->meleepush_xy = action->sequence;
			cd->damage = 0;
			if(!IsEffectInSpell(spell_id, SE_BindAffinity))
			{
				entity_list.QueueCloseClients(this, message_packet, false, RuleI(Range, SongMessages), 0, true, IsClient() ? FilterPCSpells : FilterNPCSpells);
			}
			safe_delete(message_packet);
			safe_delete(packet);

		}
		//we are done...
		return;
	}
	Log(Logs::Detail, Logs::Spells, "Bard Song Pulse %d: Buff not found, reapplying spell.", spell_id);
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
	if (caster == target && (target->aabonuses.IllusionPersistence || target->spellbonuses.IllusionPersistence ||
				 target->itembonuses.IllusionPersistence) &&
	    spell_id != 287 && spell_id != 601 && IsEffectInSpell(spell_id, SE_Illusion))
		res = 10000; // ~16h override

	res = mod_buff_duration(res, caster, target, spell_id);

	Log(Logs::Detail, Logs::Spells, "Spell %d: Casting level %d, formula %d, base_duration %d: result %d",
		spell_id, castlevel, formula, duration, res);

	return res;
}

// the generic formula calculations
int CalcBuffDuration_formula(int level, int formula, int duration)
{
	int temp;

	switch (formula) {
	case 1:
		temp = level > 3 ? level / 2 : 1;
		break;
	case 2:
		temp = level > 3 ? level / 2 + 5 : 6;
		break;
	case 3:
		temp = 30 * level;
		break;
	case 4: // only used by 'LowerElement'
		temp = 50;
		break;
	case 5:
		temp = 2;
		break;
	case 6:
		temp = level / 2 + 2;
		break;
	case 7:
		temp = level;
		break;
	case 8:
		temp = level + 10;
		break;
	case 9:
		temp = 2 * level + 10;
		break;
	case 10:
		temp = 3 * level + 10;
		break;
	case 11:
		temp = 30 * (level + 3);
		break;
	case 12:
		temp = level > 7 ? level / 4 : 1;
		break;
	case 13:
		temp = 4 * level + 10;
		break;
	case 14:
		temp = 5 * (level + 2);
		break;
	case 15:
		temp = 10 * (level + 10);
		break;
	case 50: // Permanent. Cancelled by casting/combat for perm invis, non-lev zones for lev, curing poison/curse
		 // counters, etc.
		return -1;
	case 51: // Permanent. Cancelled when out of range of aura.
		return -4;
	default:
		// the client function has another bool parameter that if true returns -2 -- unsure
		if (formula < 200)
			return 0;
		temp = formula;
		break;
	}
	if (duration && duration < temp)
		temp = duration;
	return temp;
}

// helper function for AddBuff to determine stacking
// spellid1 is the spell already worn, spellid2 is the one trying to be cast
// returns:
// 0 if not the same type, no action needs to be taken
// 1 if spellid1 should be removed (overwrite)
// -1 if they can't stack and spellid2 should be stopped
//currently, a spell will not land if it would overwrite a better spell on any effect
//if all effects are better or the same, we overwrite, else we do nothing
int Mob::CheckStackConflict(uint16 spellid1, int caster_level1, uint16 spellid2, int caster_level2, Mob* caster1, Mob* caster2, int buffslot)
{
	const SPDat_Spell_Struct &sp1 = spells[spellid1];
	const SPDat_Spell_Struct &sp2 = spells[spellid2];

	int i, effect1, effect2, sp1_value, sp2_value;
	int blocked_effect, blocked_below_value, blocked_slot;
	int overwrite_effect, overwrite_below_value, overwrite_slot;

	Log(Logs::Detail, Logs::Spells, "Check Stacking on old %s (%d) @ lvl %d (by %s) vs. new %s (%d) @ lvl %d (by %s)", sp1.name, spellid1, caster_level1, (caster1==nullptr)?"Nobody":caster1->GetName(), sp2.name, spellid2, caster_level2, (caster2==nullptr)?"Nobody":caster2->GetName());

	if (spellid1 == spellid2 ) {
		if (!IsStackableDot(spellid1) && !IsEffectInSpell(spellid1, SE_ManaBurn)) { // mana burn spells we need to use the stacking command blocks live actually checks those first, we should probably rework to that too
			if (caster_level1 > caster_level2) { // cur buff higher level than new
				if (IsEffectInSpell(spellid1, SE_ImprovedTaunt)) {
					Log(Logs::Detail, Logs::Spells, "SE_ImprovedTaunt level exception, overwriting.");
					return 1;
				} else {
					Log(Logs::Detail, Logs::Spells, "Spells the same but existing is higher level, stopping.");
					return -1;
				}
			} else {
				Log(Logs::Detail, Logs::Spells, "Spells the same but newer is higher or equal level, overwriting.");
				return 1;
			}
		} else if (spellid1 == 2751) {
			Log(Logs::Detail, Logs::Spells, "Blocking spell because manaburn does not stack with itself.");
			return -1;
		}
	}

	int modval = mod_spell_stack(spellid1, caster_level1, caster1, spellid2, caster_level2, caster2);
	if(modval < 2) { return(modval); }

	/*
	One of these is a bard song and one isn't and they're both beneficial so they should stack.
	*/
	if(IsBardSong(spellid1) != IsBardSong(spellid2))
	{
		if(!IsDetrimentalSpell(spellid1) && !IsDetrimentalSpell(spellid2))
		{
			Log(Logs::Detail, Logs::Spells, "%s and %s are beneficial, and one is a bard song, no action needs to be taken", sp1.name, sp2.name);
			return (0);
		}
	}

	bool effect_match = true; // Figure out if we're identical in effects on all slots.
	if (spellid1 != spellid2) {
		for (i = 0; i < EFFECT_COUNT; i++) {
			// we don't want this optimization for mana burns
			if (sp1.effectid[i] != sp2.effectid[i] || sp1.effectid[i] == SE_ManaBurn) {
				effect_match = false;
				break;
			}
		}
	} else if (IsEffectInSpell(spellid1, SE_ManaBurn)) {
		Log(Logs::Detail, Logs::Spells, "We have a Mana Burn spell that is the same, they won't stack");
		return -1;
	}

	// check for special stacking overwrite in spell2 against effects in spell1
	// If all of the effects match they are the same line and shouldn't care for these checks
	if (!effect_match) {
		for(i = 0; i < EFFECT_COUNT; i++)
		{
			effect1 = sp1.effectid[i];
			effect2 = sp2.effectid[i];

			if (spellbonuses.Screech == 1) {
				if (effect2 == SE_Screech && sp2.base[i] == -1) {
					Message_StringID(MT_SpellFailure, SCREECH_BUFF_BLOCK, sp2.name);
					return -1;
				}
			}

			/*Buff stacking prevention spell effects (446 - 449) works as follows... If B prevent A, if C prevent B, if D prevent C.
			If checking same type ie A vs A, which ever effect base value is higher will take hold.
			Special check is added to make sure the buffs stack properly when applied from fade on duration effect, since the buff
			is not fully removed at the time of the trgger*/
			if (spellbonuses.AStacker[0]) {
				if ((effect2 == SE_AStacker) && (sp2.effectid[i] <= spellbonuses.AStacker[1]))
					return -1;
			}

			if (spellbonuses.BStacker[0]) {
				if ((effect2 == SE_BStacker) && (sp2.effectid[i] <= spellbonuses.BStacker[1]))
					return -1;
				if ((effect2 == SE_AStacker) && (!IsCastonFadeDurationSpell(spellid1) && buffs[buffslot].ticsremaining != 1 && IsEffectInSpell(spellid1, SE_BStacker)))
					return -1;
			}

			if (spellbonuses.CStacker[0]) {
				if ((effect2 == SE_CStacker) && (sp2.effectid[i] <= spellbonuses.CStacker[1]))
					return -1;
				if ((effect2 == SE_BStacker) && (!IsCastonFadeDurationSpell(spellid1) && buffs[buffslot].ticsremaining != 1 && IsEffectInSpell(spellid1, SE_CStacker)))
					return -1;
			}

			if (spellbonuses.DStacker[0]) {
				if ((effect2 == SE_DStacker) && (sp2.effectid[i] <= spellbonuses.DStacker[1]))
					return -1;
				if ((effect2 == SE_CStacker) && (!IsCastonFadeDurationSpell(spellid1) && buffs[buffslot].ticsremaining != 1 && IsEffectInSpell(spellid1, SE_DStacker)))
					return -1;
			}

			if(effect2 == SE_StackingCommand_Overwrite)
			{
				overwrite_effect = sp2.base[i];
				overwrite_slot = sp2.formula[i] - 201;	//they use base 1 for slots, we use base 0
				overwrite_below_value = sp2.max[i];
				if(sp1.effectid[overwrite_slot] == overwrite_effect)
				{
					sp1_value = CalcSpellEffectValue(spellid1, overwrite_slot, caster_level1);

					Log(Logs::Detail, Logs::Spells, "%s (%d) overwrites existing spell if effect %d on slot %d is below %d. Old spell has value %d on that slot/effect. %s.",
						sp2.name, spellid2, overwrite_effect, overwrite_slot, overwrite_below_value, sp1_value, (sp1_value < overwrite_below_value)?"Overwriting":"Not overwriting");

					if(sp1_value < overwrite_below_value)
					{
						Log(Logs::Detail, Logs::Spells, "Overwrite spell because sp1_value < overwrite_below_value");
						return 1;			// overwrite spell if its value is less
					}
				} else {
					Log(Logs::Detail, Logs::Spells, "%s (%d) overwrites existing spell if effect %d on slot %d is below %d, but we do not have that effect on that slot. Ignored.",
						sp2.name, spellid2, overwrite_effect, overwrite_slot, overwrite_below_value);

				}
			} else if (effect1 == SE_StackingCommand_Block)
			{
				blocked_effect = sp1.base[i];
				blocked_slot = sp1.formula[i] - 201;
				blocked_below_value = sp1.max[i];

				if (sp2.effectid[blocked_slot] == blocked_effect)
				{
					sp2_value = CalcSpellEffectValue(spellid2, blocked_slot, caster_level2);

					Log(Logs::Detail, Logs::Spells, "%s (%d) blocks effect %d on slot %d below %d. New spell has value %d on that slot/effect. %s.",
						sp1.name, spellid1, blocked_effect, blocked_slot, blocked_below_value, sp2_value, (sp2_value < blocked_below_value)?"Blocked":"Not blocked");

					if (sp2_value < blocked_below_value)
					{
						Log(Logs::Detail, Logs::Spells, "Blocking spell because sp2_Value < blocked_below_value");
						return -1;		//blocked
					}
				} else {
					Log(Logs::Detail, Logs::Spells, "%s (%d) blocks effect %d on slot %d below %d, but we do not have that effect on that slot. Ignored.",
						sp1.name, spellid1, blocked_effect, blocked_slot, blocked_below_value);
				}
			}
		}
	} else {
		Log(Logs::Detail, Logs::Spells, "%s (%d) and %s (%d) appear to be in the same line, skipping Stacking Overwrite/Blocking checks",
				sp1.name, spellid1, sp2.name, spellid2);
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
	bool values_equal = true;
	for(i = 0; i < EFFECT_COUNT; i++)
	{
		if(IsBlankSpellEffect(spellid1, i) || IsBlankSpellEffect(spellid2, i))
			continue;

		effect1 = sp1.effectid[i];
		effect2 = sp2.effectid[i];

		/*
		Quick check, are the effects the same, if so then
		keep going else ignore it for stacking purposes.
		*/
		if(effect1 != effect2)
			continue;

		// big ol' list according to the client, wasn't that nice!
		if (IsEffectIgnoredInStacking(effect1))
			continue;

		// negative AC affects are skipped. Ex. Sun's Corona and Glacier Breath should stack
		// There may be more SPAs we need to add here ....
		// The client does just check base rather than calculating the affect change value.
		if ((effect1 == SE_ArmorClass || effect1 == SE_ACv2) && sp2.base[i] < 0)
			continue;

		/*
		If target is a npc and caster1 and caster2 exist
		If Caster1 isn't the same as Caster2 and the effect is a DoT then ignore it.
		*/
		if(IsNPC() && caster1 && caster2 && caster1 != caster2) {
			if(effect1 == SE_CurrentHP && sp1_detrimental && sp2_detrimental) {
				Log(Logs::Detail, Logs::Spells, "Both casters exist and are not the same, the effect is a detrimental dot, moving on");
				continue;
			}
		}

		if(effect1 == SE_CompleteHeal){ //SE_CompleteHeal never stacks or overwrites ever, always block.
			Log(Logs::Detail, Logs::Spells, "Blocking spell because complete heal never stacks or overwries");
			return (-1);
		}

		/*
		If the spells aren't the same
		and the effect is a dot we can go ahead and stack it
		*/
		if(effect1 == SE_CurrentHP && spellid1 != spellid2 && sp1_detrimental && sp2_detrimental) {
			Log(Logs::Detail, Logs::Spells, "The spells are not the same and it is a detrimental dot, passing");
			continue;
		}

		sp1_value = CalcSpellEffectValue(spellid1, i, caster_level1);
		sp2_value = CalcSpellEffectValue(spellid2, i, caster_level2);

		// some spells are hard to compare just on value. attack speed spells
		// have a value that's a percentage for instance
		if
		(
			effect1 == SE_AttackSpeed ||
			effect1 == SE_AttackSpeed2
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
			Log(Logs::Detail, Logs::Spells, "Spell %s (value %d) is not as good as %s (value %d). Rejecting %s.",
				sp2.name, sp2_value, sp1.name, sp1_value, sp2.name);
			return -1;	// can't stack
		}
		if (sp2_value != sp1_value)
			values_equal = false;
		//we dont return here... a better value on this one effect dosent mean they are
		//all better...

		Log(Logs::Detail, Logs::Spells, "Spell %s (value %d) is not as good as %s (value %d). We will overwrite %s if there are no other conflicts.",
			sp1.name, sp1_value, sp2.name, sp2_value, sp1.name);
		will_overwrite = true;
	}

	//if we get here, then none of the values on the new spell are "worse"
	//so now we see if this new spell is any better, or if its not related at all
	if(will_overwrite) {
		if (values_equal && effect_match && !IsGroupSpell(spellid2) && IsGroupSpell(spellid1)) {
			Log(Logs::Detail, Logs::Spells, "%s (%d) appears to be the single target version of %s (%d), rejecting",
					sp2.name, spellid2, sp1.name, spellid1);
			return -1;
		}
		Log(Logs::Detail, Logs::Spells, "Stacking code decided that %s should overwrite %s.", sp2.name, sp1.name);
		return(1);
	}

	Log(Logs::Detail, Logs::Spells, "Stacking code decided that %s is not affected by %s.", sp2.name, sp1.name);
	return 0;
}

// Check Spell Level Restrictions
// returns true if they meet the restrictions, false otherwise
// derived from http://samanna.net/eq.general/buffs.shtml
// spells 1-50: no restrictons
// 51-65: SpellLevel/2+15
// 66+ Group Spells 62, Single Target 61
bool Mob::CheckSpellLevelRestriction(uint16 spell_id)
{
	return true;
}

bool Client::CheckSpellLevelRestriction(uint16 spell_id)
{
	int SpellLevel = GetMinLevel(spell_id);

	// Only check for beneficial buffs
	if (IsBuffSpell(spell_id) && IsBeneficialSpell(spell_id)) {
		if (SpellLevel > 65) {
			if (IsGroupSpell(spell_id) && GetLevel() < 62)
				return false;
			else if (GetLevel() < 61)
				return false;
		} else if (SpellLevel > 50) { // 51-65
			if (GetLevel() < (SpellLevel / 2 + 15))
				return false;
		}
	}

	return true;
}

uint32 Mob::GetFirstBuffSlot(bool disc, bool song)
{
	return 0;
}

uint32 Mob::GetLastBuffSlot(bool disc, bool song)
{
	return GetCurrentBuffSlots();
}

uint32 Client::GetFirstBuffSlot(bool disc, bool song)
{
	if (song)
		return GetMaxBuffSlots();
	if (disc)
		return GetMaxBuffSlots() + GetMaxSongSlots();
	return 0;
}

uint32 Client::GetLastBuffSlot(bool disc, bool song)
{
	if (song)
		return GetMaxBuffSlots() + GetCurrentSongSlots();
	if (disc)
		return GetMaxBuffSlots() + GetMaxSongSlots() + GetCurrentDiscSlots();
	return GetCurrentBuffSlots();
}

bool Mob::HasDiscBuff()
{
	int slot = GetFirstBuffSlot(true, false);
	return buffs[slot].spellid != SPELL_UNKNOWN;
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

	if (level_override > 0)
		caster_level = level_override;
	else
		caster_level = caster ? caster->GetCasterLevel(spell_id) : GetCasterLevel(spell_id);

	if (duration == 0) {
		duration = CalcBuffDuration(caster, this, spell_id);

		if (caster && duration > 0) // negatives are perma buffs
			duration = caster->GetActSpellDuration(spell_id, duration);
	}

	if (duration == 0) {
		Log(Logs::Detail, Logs::Spells, "Buff %d failed to add because its duration came back as 0.", spell_id);
		return -2;	// no duration? this isn't a buff
	}

	Log(Logs::Detail, Logs::Spells, "Trying to add buff %d cast by %s (cast level %d) with duration %d",
		spell_id, caster?caster->GetName():"UNKNOWN", caster_level, duration);

	// first we loop through everything checking that the spell
	// can stack with everything. this is to avoid stripping the spells
	// it would overwrite, and then hitting a buff we can't stack with.
	// we also check if overwriting will occur. this is so after this loop
	// we can determine if there will be room for this buff
	int buff_count = GetMaxTotalSlots();
	uint32 start_slot = GetFirstBuffSlot(IsDisciplineBuff(spell_id), spells[spell_id].short_buff_box);
	uint32 end_slot = GetLastBuffSlot(IsDisciplineBuff(spell_id), spells[spell_id].short_buff_box);

	for (buffslot = 0; buffslot < buff_count; buffslot++) {
		const Buffs_Struct &curbuf = buffs[buffslot];

		if (curbuf.spellid != SPELL_UNKNOWN) {
			// there's a buff in this slot
			ret = CheckStackConflict(curbuf.spellid, curbuf.casterlevel, spell_id,
					caster_level, entity_list.GetMobID(curbuf.casterid), caster, buffslot);
			if (ret == -1) {	// stop the spell
				Log(Logs::Detail, Logs::Spells, "Adding buff %d failed: stacking prevented by spell %d in slot %d with caster level %d",
						spell_id, curbuf.spellid, buffslot, curbuf.casterlevel);
				if (caster && caster->IsClient() && RuleB(Client, UseLiveBlockedMessage)) {
					caster->Message(13, "Your %s did not take hold on %s. (Blocked by %s.)", spells[spell_id].name, this->GetName(), spells[curbuf.spellid].name);
				}
				return -1;
			}
			if (ret == 1) {	// set a flag to indicate that there will be overwriting
				Log(Logs::Detail, Logs::Spells, "Adding buff %d will overwrite spell %d in slot %d with caster level %d",
						spell_id, curbuf.spellid, buffslot, curbuf.casterlevel);
				// If this is the first buff it would override, use its slot
				if (!will_overwrite && !IsDisciplineBuff(spell_id))
					emptyslot = buffslot;
				will_overwrite = true;
				overwrite_slots.push_back(buffslot);
			}
		} else {
			if (emptyslot == -1) {
				if (buffslot >= start_slot && buffslot < end_slot) {
					emptyslot = buffslot;
				}
			}
		}
	}

	// we didn't find an empty slot to put it in, and it's not overwriting
	// anything so there must not be any room left.
	if (emptyslot == -1 && !will_overwrite) {
		if (IsDetrimentalSpell(spell_id))  {//Sucks to be you, bye bye one of your buffs
			for (buffslot = 0; buffslot < buff_count; buffslot++) {
				const Buffs_Struct &curbuf = buffs[buffslot];
				if (IsBeneficialSpell(curbuf.spellid)) {
					Log(Logs::Detail, Logs::Spells, "No slot for detrimental buff %d, so we are overwriting a beneficial buff %d in slot %d",
							spell_id, curbuf.spellid, buffslot);
					BuffFadeBySlot(buffslot, false);
					emptyslot = buffslot;
					break;
				}
			}
			if(emptyslot == -1) {
				Log(Logs::Detail, Logs::Spells, "Unable to find a buff slot for detrimental buff %d", spell_id);
				return -1;
			}
		} else {
			Log(Logs::Detail, Logs::Spells, "Unable to find a buff slot for beneficial buff %d", spell_id);
			return -1;
		}
	}

	// at this point we know that this buff will stick, but we have
	// to remove some other buffs already worn if will_overwrite is true
	if (will_overwrite) {
		std::vector<int>::iterator cur, end;
		cur = overwrite_slots.begin();
		end = overwrite_slots.end();
		for (; cur != end; ++cur) {
			// strip spell
			BuffFadeBySlot(*cur, false);

			// if we hadn't found a free slot before, or if this is earlier
			// we use it
			if (emptyslot == -1 || (*cur < emptyslot && !IsDisciplineBuff(spell_id)))
				emptyslot = *cur;
		}
	}

	// now add buff at emptyslot
	assert(buffs[emptyslot].spellid == SPELL_UNKNOWN);	// sanity check

	buffs[emptyslot].spellid = spell_id;
	buffs[emptyslot].casterlevel = caster_level;
	if (caster && !caster->IsAura()) // maybe some other things we don't want to ...
		strcpy(buffs[emptyslot].caster_name, caster->GetCleanName());
	else
		memset(buffs[emptyslot].caster_name, 0, 64);
	buffs[emptyslot].casterid = caster ? caster->GetID() : 0;
	buffs[emptyslot].ticsremaining = duration;
	buffs[emptyslot].counters = CalculateCounters(spell_id);
	buffs[emptyslot].numhits = spells[spell_id].numhits;
	buffs[emptyslot].client = caster ? caster->IsClient() : 0;
	buffs[emptyslot].persistant_buff = 0;
	buffs[emptyslot].caston_x = 0;
	buffs[emptyslot].caston_y = 0;
	buffs[emptyslot].caston_z = 0;
	buffs[emptyslot].dot_rune = 0;
	buffs[emptyslot].ExtraDIChance = 0;
	buffs[emptyslot].RootBreakChance = 0;
	buffs[emptyslot].instrument_mod = caster ? caster->GetInstrumentMod(spell_id) : 10;

	if (level_override > 0) {
		buffs[emptyslot].UpdateClient = true;
	} else {
		if (buffs[emptyslot].ticsremaining > (1 + CalcBuffDuration_formula(caster_level, spells[spell_id].buffdurationformula, spells[spell_id].buffduration)))
			buffs[emptyslot].UpdateClient = true;
	}

	Log(Logs::Detail, Logs::Spells, "Buff %d added to slot %d with caster level %d", spell_id, emptyslot, caster_level);
	if (IsPet() && GetOwner() && GetOwner()->IsClient())
		SendPetBuffsToClient();

	if((IsClient() && !CastToClient()->GetPVP()) ||
		(IsPet() && GetOwner() && GetOwner()->IsClient() && !GetOwner()->CastToClient()->GetPVP()) ||
#ifdef BOTS
		(IsBot() && GetOwner() && GetOwner()->IsClient() && !GetOwner()->CastToClient()->GetPVP()) ||
#endif
		(IsMerc() && GetOwner() && GetOwner()->IsClient() && !GetOwner()->CastToClient()->GetPVP()))
	{
		EQApplicationPacket *outapp = MakeBuffsPacket();

		entity_list.QueueClientsByTarget(this, outapp, false, nullptr, true, false, EQEmu::versions::bit_SoDAndLater);

		if(IsClient() && GetTarget() == this)
			CastToClient()->QueuePacket(outapp);

		safe_delete(outapp);
	}

	if (IsNPC()) {
		EQApplicationPacket *outapp = MakeBuffsPacket();
		entity_list.QueueClientsByTarget(this, outapp, false, nullptr, true, false, EQEmu::versions::bit_SoDAndLater, true);
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

	Log(Logs::Detail, Logs::AI, "Checking if buff %d cast at level %d can stack on me.%s", spellid, caster_level, iFailIfOverwrite?" failing if we would overwrite something":"");

	int buff_count = GetMaxTotalSlots();
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
		ret = CheckStackConflict(curbuf.spellid, curbuf.casterlevel, spellid, caster_level, nullptr, nullptr, i);
		if(ret == 1) {
			// should overwrite current slot
			if(iFailIfOverwrite) {
				Log(Logs::Detail, Logs::AI, "Buff %d would overwrite %d in slot %d, reporting stack failure", spellid, curbuf.spellid, i);
				return(-1);
			}
			if(firstfree == -2)
				firstfree = i;
		}
		if(ret == -1) {
			
			Log(Logs::Detail, Logs::AI, "Buff %d would conflict with %d in slot %d, reporting stack failure", spellid, curbuf.spellid, i);
			return -1;	// stop the spell, can't stack it
		}
	}

	Log(Logs::Detail, Logs::AI, "Reporting that buff %d could successfully be placed into slot %d", spellid, firstfree);

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
bool Mob::SpellOnTarget(uint16 spell_id, Mob *spelltar, bool reflect, bool use_resist_adjust, int16 resist_adjust,
			bool isproc, int level_override)
{

	// well we can't cast a spell on target without a target
	if(!spelltar)
	{
		Log(Logs::Detail, Logs::Spells, "Unable to apply spell %d without a target", spell_id);
		Message(13, "SOT: You must have a target for this spell.");
		return false;
	}

	if(spelltar->IsClient() && spelltar->CastToClient()->IsHoveringForRespawn())
		return false;

	if(IsDetrimentalSpell(spell_id) && !IsAttackAllowed(spelltar, true) && !IsResurrectionEffects(spell_id)) {
		if(!IsClient() || !CastToClient()->GetGM()) {
			Message_StringID(MT_SpellFailure, SPELL_NO_HOLD);
			return false;
		}
	}

	EQApplicationPacket *action_packet = nullptr, *message_packet = nullptr;
	float spell_effectiveness;

	if(!IsValidSpell(spell_id))
		return false;

	// these target types skip pcnpc only check (according to dev quotes)
	// other AE spells this is redundant, oh well
	// 1 = PCs, 2 = NPCs
	if (spells[spell_id].pcnpc_only_flag && spells[spell_id].targettype != ST_AETargetHateList &&
	    spells[spell_id].targettype != ST_HateList) {
		if (spells[spell_id].pcnpc_only_flag == 1 && !spelltar->IsClient() && !spelltar->IsMerc())
			return false;
		else if (spells[spell_id].pcnpc_only_flag == 2 && (spelltar->IsClient() || spelltar->IsMerc()))
			return false;
	}

	uint16 caster_level = level_override > 0 ? level_override : GetCasterLevel(spell_id);

	Log(Logs::Detail, Logs::Spells, "Casting spell %d on %s with effective caster level %d", spell_id, spelltar->GetName(), caster_level);

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
	entity_list.QueueCloseClients(
		spelltar, /* Sender */
		action_packet, /* Packet */
		true, /* Ignore Sender */
		RuleI(Range, SpellMessages), 
		this, /* Skip this Mob */
		true, /* Packet ACK */
		(spelltar->IsClient() ? FilterPCSpells : FilterNPCSpells) /* EQ Filter Type: (8 or 9) */
	);

	/* Send the EVENT_CAST_ON event */
	if(spelltar->IsNPC())
	{
		char temp1[100];
		sprintf(temp1, "%d", spell_id);
		parse->EventNPC(EVENT_CAST_ON, spelltar->CastToNPC(), this, temp1, 0);
	}
	else if (spelltar->IsClient())
	{
		char temp1[100];
		sprintf(temp1, "%d", spell_id);
		parse->EventPlayer(EVENT_CAST_ON, spelltar->CastToClient(),temp1, 0);
	}

	mod_spell_cast(spell_id, spelltar, reflect, use_resist_adjust, resist_adjust, isproc);

	// now check if the spell is allowed to land
	if (RuleB(Spells, EnableBlockedBuffs)) {
		// We return true here since the caster's client should act like normal
		if (spelltar->IsBlockedBuff(spell_id)) {
			Log(Logs::Detail, Logs::Spells, "Spell %i not applied to %s as it is a Blocked Buff.",
					spell_id, spelltar->GetName());
			safe_delete(action_packet);
			return true;
		}

		if (spelltar->IsPet() && spelltar->GetOwner() &&
				spelltar->GetOwner()->IsBlockedPetBuff(spell_id)) {
			Log(Logs::Detail, Logs::Spells, "Spell %i not applied to %s (%s's pet) as it is a Pet Blocked Buff.",
					spell_id, spelltar->GetName(), spelltar->GetOwner()->GetName());
			safe_delete(action_packet);
			return true;
		}
	}

	// invuln mobs can't be affected by any spells, good or bad
	if(spelltar->GetInvul() || spelltar->DivineAura()) {
		Log(Logs::Detail, Logs::Spells, "Casting spell %d on %s aborted: they are invulnerable.", spell_id, spelltar->GetName());
		safe_delete(action_packet);
		return false;
	}

	//cannot hurt untargetable mobs
	bodyType bt = spelltar->GetBodyType();
	if(bt == BT_NoTarget || bt == BT_NoTarget2) {
		if (RuleB(Pets, UnTargetableSwarmPet)) {
			if (spelltar->IsNPC()) {
				if (!spelltar->CastToNPC()->GetSwarmOwner()) {
					Log(Logs::Detail, Logs::Spells, "Casting spell %d on %s aborted: they are untargetable", spell_id, spelltar->GetName());
					safe_delete(action_packet);
					return(false);
				}
			} else {
				Log(Logs::Detail, Logs::Spells, "Casting spell %d on %s aborted: they are untargetable", spell_id, spelltar->GetName());
				safe_delete(action_packet);
				return(false);
			}
		} else {
			Log(Logs::Detail, Logs::Spells, "Casting spell %d on %s aborted: they are untargetable", spell_id, spelltar->GetName());
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
			spelltar->Message_StringID(MT_SpellFailure, ALREADY_INVIS, GetCleanName());
			safe_delete(action_packet);
			return false;
		}
	}

	if(IsEffectInSpell(spell_id, SE_InvisVsUndead))
	{
		if(spelltar->invisible_undead)
		{
			spelltar->Message_StringID(MT_SpellFailure, ALREADY_INVIS, GetCleanName());
			safe_delete(action_packet);
			return false;
		}
	}

	if(IsEffectInSpell(spell_id, SE_InvisVsAnimals))
	{
		if(spelltar->invisible_animals)
		{
			spelltar->Message_StringID(MT_SpellFailure, ALREADY_INVIS, GetCleanName());
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

				Client* pClient = nullptr;
				Raid* pRaid = nullptr;
				Group* pBasicGroup = nullptr;
				uint32 nGroup = 0; //raid group

				Client* pClientTarget = nullptr;
				Raid* pRaidTarget = nullptr;
				Group* pBasicGroupTarget = nullptr;
				uint32 nGroupTarget = 0; //raid group

				Client* pClientTargetPet = nullptr;
				Raid* pRaidTargetPet = nullptr;
				Group* pBasicGroupTargetPet = nullptr;
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

				if((!IsAllianceSpellLine(spell_id) && !IsBeneficialAllowed(spelltar)) ||
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
						Log(Logs::Detail, Logs::Spells, "Beneficial ae bard song %d can't take hold %s -> %s, IBA? %d", spell_id, GetName(), spelltar->GetName(), IsBeneficialAllowed(spelltar));
					} else {
						Log(Logs::Detail, Logs::Spells, "Beneficial spell %d can't take hold %s -> %s, IBA? %d", spell_id, GetName(), spelltar->GetName(), IsBeneficialAllowed(spelltar));
						Message_StringID(MT_SpellFailure, SPELL_NO_HOLD);
					}
					safe_delete(action_packet);
					return false;
				}
			}
		}
		else if	( !IsAttackAllowed(spelltar, true) && !IsResurrectionEffects(spell_id)) // Detrimental spells - PVP check
		{
			Log(Logs::Detail, Logs::Spells, "Detrimental spell %d can't take hold %s -> %s", spell_id, GetName(), spelltar->GetName());
			spelltar->Message_StringID(MT_SpellFailure, YOU_ARE_PROTECTED, GetCleanName());
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
		Log(Logs::Detail, Logs::Spells, "Spell %d can't take hold due to immunity %s -> %s", spell_id, GetName(), spelltar->GetName());
		safe_delete(action_packet);
		return false;
	}

	//check for AE_Undead
	if(spells[spell_id].targettype == ST_UndeadAE){
		if(spelltar->GetBodyType() != BT_SummonedUndead &&
			spelltar->GetBodyType() != BT_Undead &&
			spelltar->GetBodyType() != BT_Vampire)
		{
			safe_delete(action_packet);
			return false;
		}
	}
	// Block next spell effect should be used up first(since its blocking the next spell)
	if(CanBlockSpell()) {
		int buff_count = GetMaxTotalSlots();
		int focus = 0;
		for (int b=0; b < buff_count; b++) {
			if(IsEffectInSpell(buffs[b].spellid, SE_BlockNextSpellFocus)) {
				focus = CalcFocusEffect(focusBlockNextSpell, buffs[b].spellid, spell_id);
				if(focus) {
					CheckNumHitsRemaining(NumHit::MatchingSpells, b);
					Message_StringID(MT_SpellFailure, SPELL_WOULDNT_HOLD);
					safe_delete(action_packet);
					return false;
				}
			}
		}
	}
	// Reflect
	if(spelltar && spelltar->TryReflectSpell(spell_id) && !reflect && IsDetrimentalSpell(spell_id) && this != spelltar) {
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
		if (reflect_chance) {

			if (RuleB(Spells, ReflectMessagesClose)) {
				entity_list.MessageClose_StringID(
					this, /* Sender */
					false, /* Skip Sender */
					RuleI(Range, SpellMessages), /* Range */
					MT_Spells, /* Type */
					SPELL_REFLECT, /* String ID */
					GetCleanName(), /* Message 1 */
					spelltar->GetCleanName() /* Message 2 */
				);
			}
			else {
				Message_StringID(MT_Spells, SPELL_REFLECT, GetCleanName(), spelltar->GetCleanName());
			}

			CheckNumHitsRemaining(NumHit::ReflectSpell);
			// caster actually appears to change
			// ex. During OMM fight you click your reflect mask and you get the recourse from the reflected
			// spell
			spelltar->SpellOnTarget(spell_id, this, true, use_resist_adjust, resist_adjust);
			safe_delete(action_packet);
			return false;
		}
	}

	// resist check - every spell can be resisted, beneficial or not
	// add: ok this isn't true, eqlive's spell data is fucked up, buffs are
	// not all unresistable, so changing this to only check certain spells
	if(IsResistableSpell(spell_id))
	{
		spelltar->BreakInvisibleSpells(); //Any detrimental spell cast on you will drop invisible (can be AOE, non damage ect).

		if (IsCharmSpell(spell_id) || IsMezSpell(spell_id) || IsFearSpell(spell_id))
			spell_effectiveness = spelltar->ResistSpell(spells[spell_id].resisttype, spell_id, this, use_resist_adjust, resist_adjust, true, false, false, level_override);
		else
			spell_effectiveness = spelltar->ResistSpell(spells[spell_id].resisttype, spell_id, this, use_resist_adjust, resist_adjust, false, false, false, level_override);

		if(spell_effectiveness < 100)
		{
			if(spell_effectiveness == 0 || !IsPartialCapableSpell(spell_id) )
			{
				Log(Logs::Detail, Logs::Spells, "Spell %d was completely resisted by %s", spell_id, spelltar->GetName());

				if (spells[spell_id].resisttype == RESIST_PHYSICAL){
					Message_StringID(MT_SpellFailure, PHYSICAL_RESIST_FAIL,spells[spell_id].name);
					spelltar->Message_StringID(MT_SpellFailure, YOU_RESIST, spells[spell_id].name);
				}
				else {
					Message_StringID(MT_SpellFailure, TARGET_RESISTED, spells[spell_id].name);
					spelltar->Message_StringID(MT_SpellFailure, YOU_RESIST, spells[spell_id].name);
				}

				if (spelltar->IsAIControlled()) {
					int32 aggro = CheckAggroAmount(spell_id, spelltar);
					if (aggro > 0) {
						if (!IsHarmonySpell(spell_id))
							spelltar->AddToHateList(this, aggro);
						else if (!spelltar->PassCharismaCheck(this, spell_id))
							spelltar->AddToHateList(this, aggro);
					} else {
						int newhate = spelltar->GetHateAmount(this) + aggro;
						spelltar->SetHateAmountOnEnt(this, std::max(1, newhate));
					}
				}

				if (spelltar->IsClient()){
					spelltar->CastToClient()->BreakSneakWhenCastOn(this, true);
					spelltar->CastToClient()->BreakFeignDeathWhenCastOn(true);
				}
				
				spelltar->CheckNumHitsRemaining(NumHit::IncomingSpells);
				CheckNumHitsRemaining(NumHit::OutgoingSpells);

				safe_delete(action_packet);
				return false;
			}
		}

		if (spelltar->IsClient()){
			spelltar->CastToClient()->BreakSneakWhenCastOn(this, false);
			spelltar->CastToClient()->BreakFeignDeathWhenCastOn(false);
		}
	}
	else
	{
		spell_effectiveness = 100;
	}

	if(spelltar->spellbonuses.SpellDamageShield && IsDetrimentalSpell(spell_id))
		spelltar->DamageShield(this, true);

	if (spelltar->IsAIControlled() && IsDetrimentalSpell(spell_id) && !IsHarmonySpell(spell_id)) {
		int32 aggro_amount = CheckAggroAmount(spell_id, spelltar, isproc);
		Log(Logs::Detail, Logs::Spells, "Spell %d cast on %s generated %d hate", spell_id,
			spelltar->GetName(), aggro_amount);
		if (aggro_amount > 0) {
			spelltar->AddToHateList(this, aggro_amount);
		} else {
			int32 newhate = spelltar->GetHateAmount(this) + aggro_amount;
			spelltar->SetHateAmountOnEnt(this, std::max(newhate, 1));
		}
	} else if (IsBeneficialSpell(spell_id) && !IsSummonPCSpell(spell_id)) {
		entity_list.AddHealAggro(
		    spelltar, this,
		    CheckHealAggroAmount(spell_id, spelltar, (spelltar->GetMaxHP() - spelltar->GetHP())));
	}

	// make sure spelltar is high enough level for the buff
	if(RuleB(Spells, BuffLevelRestrictions) && !spelltar->CheckSpellLevelRestriction(spell_id))
	{
		Log(Logs::Detail, Logs::Spells, "Spell %d failed: recipient did not meet the level restrictions", spell_id);
		if(!IsBardSong(spell_id))
			Message_StringID(MT_SpellFailure, SPELL_TOO_POWERFUL);
		safe_delete(action_packet);
		return false;
	}

	// cause the effects to the target
	if(!spelltar->SpellEffect(this, spell_id, spell_effectiveness, level_override))
	{
		// if SpellEffect returned false there's a problem applying the
		// spell. It's most likely a buff that can't stack.
		Log(Logs::Detail, Logs::Spells, "Spell %d could not apply its effects %s -> %s\n", spell_id, GetName(), spelltar->GetName());
		if(casting_spell_aa_id)
			Message_StringID(MT_SpellFailure, SPELL_NO_HOLD);
		safe_delete(action_packet);
		return false;
	}

	if (IsValidSpell(spells[spell_id].RecourseLink) && spells[spell_id].RecourseLink != spell_id)
		SpellFinished(spells[spell_id].RecourseLink, this, CastingSlot::Item, 0, -1, spells[spells[spell_id].RecourseLink].ResistDiff);

	if (IsDetrimentalSpell(spell_id)) {

		CheckNumHitsRemaining(NumHit::OutgoingSpells);

		if (spelltar)
			spelltar->CheckNumHitsRemaining(NumHit::IncomingSpells);
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
			if(!IsBuffSpell(spell_id))
			{
				spelltar->CastToClient()->SetKnockBackExemption(true);

				action->buff_unknown = 0;
				auto outapp_push =
				    new EQApplicationPacket(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
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
	cd->meleepush_xy = action->sequence;
	cd->damage = 0;
	if(!IsEffectInSpell(spell_id, SE_BindAffinity)){
		entity_list.QueueCloseClients(
			spelltar, /* Sender */
			message_packet, /* Packet */
			false, /* Ignore Sender */
			RuleI(Range, SpellMessages), 
			0, /* Skip this mob */
			true, /* Packet ACK */
			(spelltar->IsClient() ? FilterPCSpells : FilterNPCSpells) /* Message Filter Type: (8 or 9) */
		);
	}
	safe_delete(action_packet);
	safe_delete(message_packet);

	Log(Logs::Detail, Logs::Spells, "Cast of %d by %s on %s complete successfully.", spell_id, GetName(), spelltar->GetName());

	return true;
}

void Corpse::CastRezz(uint16 spellid, Mob* Caster)
{
	Log(Logs::Detail, Logs::Spells, "Corpse::CastRezz spellid %i, Rezzed() is %i, rezzexp is %i", spellid,IsRezzed(),rez_experience);

	if(IsRezzed()){
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

	auto outapp = new EQApplicationPacket(OP_RezzRequest, sizeof(Resurrect_Struct));
	Resurrect_Struct* rezz = (Resurrect_Struct*) outapp->pBuffer;
	// Why are we truncating these names to 30 characters ?
	memcpy(rezz->your_name,this->corpse_name,30);
	memcpy(rezz->corpse_name,this->name,30);
	memcpy(rezz->rezzer_name,Caster->GetName(),30);
	rezz->zone_id = zone->GetZoneID();
	rezz->instance_id = zone->GetInstanceID();
	rezz->spellid = spellid;
	rezz->x = this->m_Position.x;
	rezz->y = this->m_Position.y;
	rezz->z = this->m_Position.z;
	rezz->unknown000 = 0x00000000;
	rezz->unknown020 = 0x00000000;
	rezz->unknown088 = 0x00000000;
	// We send this to world, because it needs to go to the player who may not be in this zone.
	worldserver.RezzPlayer(outapp, rez_experience, corpse_db_id, OP_RezzRequest);
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
	int buff_count = GetMaxTotalSlots();
	for (int j = 0; j < buff_count; j++) {
		if(buffs[j].spellid != SPELL_UNKNOWN)
			BuffFadeBySlot(j, false);
	}
	//we tell BuffFadeBySlot not to recalc, so we can do it only once when were done
	CalcBonuses();
}

void Mob::BuffFadeNonPersistDeath()
{
	int buff_count = GetMaxTotalSlots();
	for (int j = 0; j < buff_count; j++) {
		if (buffs[j].spellid != SPELL_UNKNOWN && !IsPersistDeathSpell(buffs[j].spellid))
			BuffFadeBySlot(j, false);
	}
	//we tell BuffFadeBySlot not to recalc, so we can do it only once when were done
	CalcBonuses();
}

void Mob::BuffFadeDetrimental() {
	int buff_count = GetMaxTotalSlots();
	for (int j = 0; j < buff_count; j++) {
		if(buffs[j].spellid != SPELL_UNKNOWN) {
			if(IsDetrimentalSpell(buffs[j].spellid))
				BuffFadeBySlot(j, false);
		}
	}
	//we tell BuffFadeBySlot not to recalc, so we can do it only once when were done
	CalcBonuses();
}

void Mob::BuffFadeDetrimentalByCaster(Mob *caster)
{
	if(!caster)
		return;

	int buff_count = GetMaxTotalSlots();
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
	//we tell BuffFadeBySlot not to recalc, so we can do it only once when were done
	CalcBonuses();
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
	int buff_count = GetMaxTotalSlots();
	for (int j = 0; j < buff_count; j++)
	{
		if (buffs[j].spellid == spell_id)
			BuffFadeBySlot(j, false);
	}

	//we tell BuffFadeBySlot not to recalc, so we can do it only once when were done
	CalcBonuses();
}

void Mob::BuffFadeBySpellIDAndCaster(uint16 spell_id, uint16 caster_id)
{
	bool recalc_bonus = false;
	auto buff_count = GetMaxTotalSlots();
	for (int i = 0; i < buff_count; ++i) {
		if (buffs[i].spellid == spell_id && buffs[i].casterid == caster_id) {
			BuffFadeBySlot(i, false);
			recalc_bonus = true;
		}
	}

	if (recalc_bonus)
		CalcBonuses();
}

// removes buffs containing effectid, skipping skipslot
void Mob::BuffFadeByEffect(int effectid, int skipslot)
{
	int i;

	int buff_count = GetMaxTotalSlots();
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

bool Mob::IsAffectedByBuff(uint16 spell_id)
{
	int buff_count = GetMaxTotalSlots();
	for (int i = 0; i < buff_count; ++i)
		if (buffs[i].spellid == spell_id)
			return true;

	return false;
}

bool Mob::IsAffectedByBuffByGlobalGroup(GlobalGroup group)
{
	int buff_count = GetMaxTotalSlots();
	for (int i = 0; i < buff_count; ++i) {
		if (buffs[i].spellid == SPELL_UNKNOWN)
			continue;
		if (spells[buffs[i].spellid].spell_category == static_cast<int>(group))
			return true;
	}

	return false;
}

// checks if 'this' can be affected by spell_id from caster
// returns true if the spell should fail, false otherwise
bool Mob::IsImmuneToSpell(uint16 spell_id, Mob *caster)
{
	int effect_index;

	if(caster == nullptr)
		return(false);

	//TODO: this function loops through the effect list for
	//this spell like 10 times, this could easily be consolidated
	//into one loop through with a switch statement.

	Log(Logs::Detail, Logs::Spells, "Checking to see if we are immune to spell %d cast by %s", spell_id, caster->GetName());

	if(!IsValidSpell(spell_id))
		return true;

	if(IsBeneficialSpell(spell_id) && (caster->GetNPCTypeID())) //then skip the rest, stop NPCs aggroing each other with buff spells. 2013-03-05
		return false;

	if(IsMezSpell(spell_id))
	{
		if(GetSpecialAbility(UNMEZABLE)) {
			Log(Logs::Detail, Logs::Spells, "We are immune to Mez spells.");
			caster->Message_StringID(MT_Shout, CANNOT_MEZ);
			int32 aggro = caster->CheckAggroAmount(spell_id, this);
			if(aggro > 0) {
				AddToHateList(caster, aggro);
			} else {
				AddToHateList(caster, 1,0,true,false,false,spell_id);
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
			Log(Logs::Detail, Logs::Spells, "Our level (%d) is higher than the limit of this Mez spell (%d)", GetLevel(), spells[spell_id].max[effect_index]);
			caster->Message_StringID(MT_Shout, CANNOT_MEZ_WITH_SPELL);
			return true;
		}
	}

	// slow and haste spells
	if(GetSpecialAbility(UNSLOWABLE) && IsEffectInSpell(spell_id, SE_AttackSpeed))
	{
		Log(Logs::Detail, Logs::Spells, "We are immune to Slow spells.");
		caster->Message_StringID(MT_Shout, IMMUNE_ATKSPEED);
		int32 aggro = caster->CheckAggroAmount(spell_id, this);
		if(aggro > 0) {
			AddToHateList(caster, aggro);
		} else {
			AddToHateList(caster, 1,0,true,false,false,spell_id);
		}
		return true;
	}

	// client vs client fear
	if(IsEffectInSpell(spell_id, SE_Fear))
	{
		effect_index = GetSpellEffectIndex(spell_id, SE_Fear);
		if(GetSpecialAbility(UNFEARABLE)) {
			Log(Logs::Detail, Logs::Spells, "We are immune to Fear spells.");
			caster->Message_StringID(MT_Shout, IMMUNE_FEAR);
			int32 aggro = caster->CheckAggroAmount(spell_id, this);
			if(aggro > 0) {
				AddToHateList(caster, aggro);
			} else {
				AddToHateList(caster, 1,0,true,false,false,spell_id);
			}
			return true;
		} else if(IsClient() && caster->IsClient() && (caster->CastToClient()->GetGM() == false))
		{
			Log(Logs::Detail, Logs::Spells, "Clients cannot fear eachother!");
			caster->Message_StringID(MT_Shout, IMMUNE_FEAR);
			return true;
		}
		else if(GetLevel() > spells[spell_id].max[effect_index] && spells[spell_id].max[effect_index] != 0)
		{
			Log(Logs::Detail, Logs::Spells, "Level is %d, cannot be feared by this spell.", GetLevel());
			caster->Message_StringID(MT_Shout, FEAR_TOO_HIGH);
			int32 aggro = caster->CheckAggroAmount(spell_id, this);
			if (aggro > 0) {
				AddToHateList(caster, aggro);
			} else {
				AddToHateList(caster, 1,0,true,false,false,spell_id);
			}
			return true;
		}
		else if (CheckAATimer(aaTimerWarcry))
		{
			Message(13, "Your are immune to fear.");
			Log(Logs::Detail, Logs::Spells, "Clients has WarCry effect, immune to fear!");
			caster->Message_StringID(MT_Shout, IMMUNE_FEAR);
			return true;
		}
	}

	if(IsCharmSpell(spell_id))
	{
		if(GetSpecialAbility(UNCHARMABLE))
		{
			Log(Logs::Detail, Logs::Spells, "We are immune to Charm spells.");
			caster->Message_StringID(MT_Shout, CANNOT_CHARM);
			int32 aggro = caster->CheckAggroAmount(spell_id, this);
			if(aggro > 0) {
				AddToHateList(caster, aggro);
			} else {
				AddToHateList(caster, 1,0,true,false,false,spell_id);
			}
			return true;
		}

		if(this == caster)
		{
			Log(Logs::Detail, Logs::Spells, "You are immune to your own charms.");
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
				Log(Logs::Detail, Logs::Spells, "Our level (%d) is higher than the limit of this Charm spell (%d)", GetLevel(), spells[spell_id].max[effect_index]);
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
			Log(Logs::Detail, Logs::Spells, "We are immune to Snare spells.");
			caster->Message_StringID(MT_Shout, IMMUNE_MOVEMENT);
			int32 aggro = caster->CheckAggroAmount(spell_id, this);
			if(aggro > 0) {
				AddToHateList(caster, aggro);
			} else {
				AddToHateList(caster, 1,0,true,false,false,spell_id);
			}
			return true;
		}
	}

	if(IsLifetapSpell(spell_id))
	{
		if(this == caster)
		{
			Log(Logs::Detail, Logs::Spells, "You cannot lifetap yourself.");
			caster->Message_StringID(MT_Shout, CANT_DRAIN_SELF);
			return true;
		}
	}

	if(IsSacrificeSpell(spell_id))
	{
		if(this == caster)
		{
			Log(Logs::Detail, Logs::Spells, "You cannot sacrifice yourself.");
			caster->Message_StringID(MT_Shout, CANNOT_SAC_SELF);
			return true;
		}
	}

	Log(Logs::Detail, Logs::Spells, "No immunities to spell %d found.", spell_id);

	return false;
}

int Mob::GetResist(uint8 resist_type)
{
	switch(resist_type)
	{
	case RESIST_FIRE:
		return GetFR();
	case RESIST_COLD:
		return GetCR();
	case RESIST_MAGIC:
		return GetMR();
	case RESIST_DISEASE:
		return GetDR();
	case RESIST_POISON:
		return GetPR();
	case RESIST_CORRUPTION:
		return GetCorrup();
	case RESIST_PRISMATIC:
		return (GetFR() + GetCR() + GetMR() + GetDR() + GetPR()) / 5;
	case RESIST_CHROMATIC:
		return std::min({GetFR(), GetCR(), GetMR(), GetDR(), GetPR()});
	case RESIST_PHYSICAL:
		if (IsNPC())
			return GetPhR();
		else
			return 0;
	default:
		return 0;
	}
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
float Mob::ResistSpell(uint8 resist_type, uint16 spell_id, Mob *caster, bool use_resist_override, int resist_override, bool CharismaCheck, bool CharmTick, bool IsRoot, int level_override)
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
		Log(Logs::Detail, Logs::Spells, "We are immune to magic, so we fully resist the spell %d", spell_id);
		return(0);
	}

	//Get resist modifier and adjust it based on focus 2 resist about eq to 1% resist chance
	int resist_modifier = (use_resist_override) ? resist_override : spells[spell_id].ResistDiff;

	if(caster->GetSpecialAbility(CASTING_RESIST_DIFF))
		resist_modifier += caster->GetSpecialAbilityParam(CASTING_RESIST_DIFF, 0);

	int focus_resist = caster->GetFocusEffect(focusResistRate, spell_id);
	resist_modifier -= 2 * focus_resist;

	//Check for fear resist
	bool IsFear = false;
	if(IsFearSpell(spell_id))
	{
		IsFear = true;
		int fear_resist_bonuses = CalcFearResistChance();
		if(zone->random.Roll(fear_resist_bonuses))
		{
			Log(Logs::Detail, Logs::Spells, "Resisted spell in fear resistance, had %d chance to resist", fear_resist_bonuses);
			return 0;
		}
	}

	if (!CharmTick){

		//Check for Spell Effect specific resistance chances (ie AA Mental Fortitude)
		int se_resist_bonuses = GetSpellEffectResistChance(spell_id);
		if(se_resist_bonuses && zone->random.Roll(se_resist_bonuses))
		{
			return 0;
		}

		// Check for Chance to Resist Spell bonuses (ie Sanctification Discipline)
		int resist_bonuses = CalcResistChanceBonus();
		if(resist_bonuses && zone->random.Roll(resist_bonuses))
		{
			Log(Logs::Detail, Logs::Spells, "Resisted spell in sanctification, had %d chance to resist", resist_bonuses);
			return 0;
		}
	}

	//Get the resist chance for the target
	if(resist_type == RESIST_NONE || spells[spell_id].no_resist)
	{
		Log(Logs::Detail, Logs::Spells, "Spell was unresistable");
		return 100;
	}

	int target_resist = GetResist(resist_type);

	// JULY 24, 2002 changes
	int level = GetLevel();
	if (IsPetOwnerClient() && caster->IsNPC() && !caster->IsPetOwnerClient()) {
		auto owner = GetOwner();
		if (owner != nullptr) {
			target_resist = std::max(target_resist, owner->GetResist(resist_type));
			level = owner->GetLevel();
		}
	}

	//Setup our base resist chance.
	int resist_chance = 0;
	int level_mod = 0;

	//Adjust our resist chance based on level modifiers
	uint8 caster_level = level_override > 0 ? level_override : caster->GetLevel();
	int temp_level_diff = level - caster_level;

	//Physical Resists are calclated using their own formula derived from extensive parsing.
	if (resist_type == RESIST_PHYSICAL) {
		level_mod = ResistPhysical(temp_level_diff, caster_level);
	}

	else {

		if(IsNPC() && level >= RuleI(Casting,ResistFalloff))
		{
			int a = (RuleI(Casting,ResistFalloff)-1) - caster_level;
			if(a > 0)
			{
				temp_level_diff = a;
			}
			else
			{
				temp_level_diff = 0;
			}
		}

		if(IsClient() && level >= 21 && temp_level_diff > 15)
		{
			temp_level_diff = 15;
		}

		if(IsNPC() && temp_level_diff < -9)
		{
			temp_level_diff = -9;
		}

		level_mod = temp_level_diff * temp_level_diff / 2;
		if(temp_level_diff < 0)
		{
			level_mod = -level_mod;
		}

		if(IsNPC() && (caster_level - level) < -20)
		{
			level_mod = 1000;
		}

		//Even more level stuff this time dealing with damage spells
		if(IsNPC() && IsDamageSpell(spell_id) && level >= 17)
		{
			int level_diff;
			if(level >= RuleI(Casting,ResistFalloff))
			{
				level_diff = (RuleI(Casting,ResistFalloff)-1) - caster_level;
				if(level_diff < 0)
				{
					level_diff = 0;
				}
			}
			else
			{
				level_diff = level - caster_level;
			}
			level_mod += (2 * level_diff);
		}
	}

	if (CharismaCheck)
	{
		/*
		Charisma ONLY effects the initial resist check when charm is cast with 10 CHA = -1 Resist mod up to 255 CHA (min ~ 75 cha)
		Charisma less than ~ 75 gives a postive modifier to resist checks at approximate ratio of -10 CHA = +6 Resist.
		Mez spells do same initial resist check as a above.
		Lull spells only check charisma if inital cast is resisted to see if mob will aggro, same modifier/cap as above.
		Charisma DOES NOT extend charm durations.
		Fear resist chance is given a -20 resist modifier if CHA is < 100, from 100-255 it progressively reduces the negative mod to 0.
		Fears verse undead DO NOT apply a charisma modifer. (Note: unknown Base1 values defined in undead fears do not effect duration).
		*/
		int16 charisma = caster->GetCHA();

		if (IsFear && (spells[spell_id].targettype != ST_Undead)){

			if (charisma < 100)
				resist_modifier -= 20;

			else if (charisma <= 255)
				resist_modifier += (charisma - 100)/8;
		}

		else {

			if (charisma >= 75){

				if (charisma > RuleI(Spells, CharismaEffectivenessCap))
					charisma = RuleI(Spells, CharismaEffectivenessCap);

				resist_modifier -= (charisma - 75)/RuleI(Spells, CharismaEffectiveness);
			}
			else
				resist_modifier += ((75 - charisma)/10) * 6; //Increase Resist Chance
		}

	}


	//Lull spells DO NOT use regular resists on initial cast, instead they use a flat +15 modifier. Live parses confirm this.
	//Regular resists are used when checking if mob will aggro off of a lull resist.
	if(!CharismaCheck && IsHarmonySpell(spell_id))
		target_resist = 15;

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

	//Average charm duration agianst mobs with 0% chance to resist on LIVE is ~ 68 ticks.
	//Minimum resist chance should be caclulated factoring in the RuleI(Spells, CharmBreakCheckChance)
	if (CharmTick) {

		float min_charmbreakchance = ((100.0f/static_cast<float>(RuleI(Spells, CharmBreakCheckChance)))/66.0f * 100.0f)*2.0f;
		if (resist_chance < static_cast<int>(min_charmbreakchance))
			resist_chance = min_charmbreakchance;
	}

	//Average root duration agianst mobs with 0% chance to resist on LIVE is ~ 22 ticks (6% resist chance).
	//Minimum resist chance should be caclulated factoring in the RuleI(Spells, RootBreakCheckChance)
	if (IsRoot) {

		float min_rootbreakchance = ((100.0f/static_cast<float>(RuleI(Spells, RootBreakCheckChance)))/22.0f * 100.0f)*2.0f;

		if (resist_chance < static_cast<int>(min_rootbreakchance))
			resist_chance = min_rootbreakchance;
	}

	//Finally our roll
	int roll = zone->random.Int(0, 200);
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
			if(resist_chance < 1)
			{
				resist_chance = 1;
			}

			int partial_modifier = ((150 * (resist_chance - roll)) / resist_chance);

			if(IsNPC())
			{
				if(level > caster_level && level >= 17 && caster_level <= 50)
				{
					partial_modifier += 5;
				}

				if(level >= 30 && caster_level < 50)
				{
					partial_modifier += (caster_level - 25);
				}

				if(level < 15)
				{
					partial_modifier -= 5;
				}
			}

			if(caster->IsNPC())
			{
				if((level - caster_level) >= 20)
				{
					partial_modifier += (level - caster_level) * 1.5;
				}
			}

			if(partial_modifier <= 0)
			{
				return 100;
			}
			else if(partial_modifier >= 100)
			{
				return 0;
			}

			return (100.0f - partial_modifier);
		}
	}
}

int Mob::ResistPhysical(int level_diff, uint8 caster_level)
{
	/*	Physical resists use the standard level mod calculation in
	conjunction with a resist fall off formula that greatly prevents you
	from landing abilities on mobs that are higher level than you.
	After level 12, every 4 levels gained the max level you can hit
	your target without a sharp resist penalty is raised by 1.
	Extensive parsing confirms this, along with baseline phyiscal resist rates used.
	*/


	if (level_diff == 0)
		return level_diff;

	int level_mod = 0;

	if (level_diff > 0) {

		int ResistFallOff = 0;

		if (caster_level <= 12)
			ResistFallOff = 3;
		else
			ResistFallOff = caster_level/4;

		if (level_diff > ResistFallOff || level_diff >= 15)
			level_mod = ((level_diff * 10) + level_diff)*2;
		else
			level_mod = level_diff * level_diff / 2;
	}

	else
		level_mod = -(level_diff * level_diff / 2);

	return level_mod;
}

int16 Mob::CalcResistChanceBonus()
{
	int resistchance = spellbonuses.ResistSpellChance + itembonuses.ResistSpellChance;

	if(IsClient())
		resistchance += aabonuses.ResistSpellChance;

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

	if(IsBardSong(spell_id) && IsBeneficialSpell(spell_id)) {
		//Live AA - Extended Notes, SionachiesCrescendo
		float song_bonus = static_cast<float>(aabonuses.SongRange + spellbonuses.SongRange + itembonuses.SongRange);
		range += range*song_bonus /100.0f;
	}

	range = GetActSpellRange(spell_id, range);

	return(range);
}

///////////////////////////////////////////////////////////////////////////////
// 'other' functions

void Mob::Spin() {
	if(IsClient()) {
		auto outapp = new EQApplicationPacket(OP_Action, sizeof(Action_Struct));
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

	auto outapp = new EQApplicationPacket(OP_ManaChange, sizeof(ManaChange_Struct));
	ManaChange_Struct* manachange = (ManaChange_Struct*)outapp->pBuffer;
	manachange->new_mana = GetMana();
	manachange->spell_id = spell_id;
	manachange->stamina = CastToClient()->GetEndurance();
	manachange->keepcasting = 0;
	outapp->priority = 6;
	CastToClient()->QueuePacket(outapp);
	safe_delete(outapp);
}

void Mob::Stun(int duration)
{
	//make sure a shorter stun does not overwrite a longer one.
	if(stunned && stunned_timer.GetRemainingTime() > uint32(duration))
		return;

	if(IsValidSpell(casting_spell_id) && !spells[casting_spell_id].uninterruptable) {
		int persistent_casting = spellbonuses.PersistantCasting + itembonuses.PersistantCasting + aabonuses.PersistantCasting;

		if(zone->random.Int(0,99) > persistent_casting)
			InterruptSpell();
	}

	if(duration > 0)
	{
		stunned = true;
		stunned_timer.Start(duration);
		SendAddPlayerState(PlayerState::Stunned);
	}
}

void Mob::UnStun() {
	if(stunned && stunned_timer.Enabled()) {
		stunned = false;
		stunned_timer.Disable();
		SendRemovePlayerState(PlayerState::Stunned);
	}
}

// Stuns "this"
void Client::Stun(int duration)
{
	Mob::Stun(duration);

	auto outapp = new EQApplicationPacket(OP_Stun, sizeof(Stun_Struct));
	Stun_Struct* stunon = (Stun_Struct*) outapp->pBuffer;
	stunon->duration = duration;
	outapp->priority = 5;
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::UnStun() {
	Mob::UnStun();

	auto outapp = new EQApplicationPacket(OP_Stun, sizeof(Stun_Struct));
	Stun_Struct* stunon = (Stun_Struct*) outapp->pBuffer;
	stunon->duration = 0;
	outapp->priority = 5;
	QueuePacket(outapp);
	safe_delete(outapp);
}

void NPC::Stun(int duration) {
	Mob::Stun(duration);
	SetCurrentSpeed(0);
}

void NPC::UnStun() {
	Mob::UnStun();
	SetCurrentSpeed(GetRunspeed());
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
	EQApplicationPacket* outapp = nullptr;

	outapp = new EQApplicationPacket(OP_Buff, sizeof(SpellBuffPacket_Struct));
	SpellBuffPacket_Struct* sbf = (SpellBuffPacket_Struct*) outapp->pBuffer;

	sbf->entityid = GetID();
	// i dont know why but this works.. for now
	sbf->buff.effect_type = 2;
//	sbf->slot=m_pp.buffs[slot_id].slotid;
//	sbf->level=m_pp.buffs[slot_id].level;
//	sbf->effect=m_pp.buffs[slot_id].effect;
	sbf->buff.spellid = spell_id;
	sbf->slotid = slot_id;
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
	sbf->buff.spellid = 0xffffffff;
#if EQDEBUG >= 11
	printf("Sending SBF 2 from server:\n");
	DumpPacket(outapp);
#endif
	QueuePacket(outapp);
	safe_delete(outapp);

	if(send_message)
	{
		const char *fadetext = spells[spell_id].spell_fades;
		outapp = new EQApplicationPacket(OP_ColoredText, sizeof(ColoredText_Struct) + strlen(fadetext));
		ColoredText_Struct *bfm = (ColoredText_Struct *) outapp->pBuffer;
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
	Log(Logs::Detail, Logs::Spells, "Spell %d memorized into slot %d", spell_id, slot);

	database.SaveCharacterMemorizedSpell(this->CharacterID(), m_pp.mem_spells[slot], slot);

	if(update_client)
	{
		MemorizeSpell(slot, spell_id, memSpellMemorize);
	}
}

void Client::UnmemSpell(int slot, bool update_client)
{
	if(slot > MAX_PP_MEMSPELL || slot < 0)
		return;

	Log(Logs::Detail, Logs::Spells, "Spell %d forgotten from slot %d", m_pp.mem_spells[slot], slot);
	m_pp.mem_spells[slot] = 0xFFFFFFFF;

	database.DeleteCharacterMemorizedSpell(this->CharacterID(), m_pp.mem_spells[slot], slot);

	if(update_client)
	{
		MemorizeSpell(slot, m_pp.mem_spells[slot], memSpellForget);
	}
}

void Client::UnmemSpellBySpellID(int32 spell_id)
{
	for(int i = 0; i < MAX_PP_MEMSPELL; i++) {
		if(m_pp.mem_spells[i] == spell_id) {
			UnmemSpell(i, true);
			break;
		}
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
	database.SaveCharacterSpell(this->CharacterID(), spell_id, slot);
	Log(Logs::Detail, Logs::Spells, "Spell %d scribed into spell book slot %d", spell_id, slot);

	if(update_client)
	{
		MemorizeSpell(slot, spell_id, memSpellScribing);
	}
}

void Client::UnscribeSpell(int slot, bool update_client)
{
	if(slot >= MAX_PP_SPELLBOOK || slot < 0)
		return;

	Log(Logs::Detail, Logs::Spells, "Spell %d erased from spell book slot %d", m_pp.spell_book[slot], slot);
	m_pp.spell_book[slot] = 0xFFFFFFFF;

	database.DeleteCharacterSpell(this->CharacterID(), m_pp.spell_book[slot], slot);
	if(update_client)
	{
		auto outapp = new EQApplicationPacket(OP_DeleteSpell, sizeof(DeleteSpell_Struct));
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

	Log(Logs::Detail, Logs::Spells, "Discipline %d untrained from slot %d", m_pp.disciplines.values[slot], slot);
	m_pp.disciplines.values[slot] = 0;
	database.DeleteCharacterDisc(this->CharacterID(), slot);

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

bool Client::SpellGlobalCheck(uint16 spell_ID, uint32 char_ID) {

	std::string spell_Global_Name;
	int spell_Global_Value;
	int global_Value;

	std::string query = StringFormat("SELECT qglobal, value FROM spell_globals "
                                    "WHERE spellid = %i", spell_ID);
    auto results = database.QueryDatabase(query);
    if (!results.Success()) {
		return false; // Query failed, so prevent spell from scribing just in case
    }

    if (results.RowCount() != 1)
        return true; // Spell ID isn't listed in the spells_global table, so it is not restricted from scribing

    auto row = results.begin();
    spell_Global_Name = row[0];
	spell_Global_Value = atoi(row[1]);

	if (spell_Global_Name.empty())
        return true; // If the entry in the spell_globals table has nothing set for the qglobal name

    query = StringFormat("SELECT value FROM quest_globals "
                        "WHERE charid = %i AND name = '%s'",
                        char_ID, spell_Global_Name.c_str());
    results = database.QueryDatabase(query);
    if (!results.Success()) {
        Log(Logs::General, Logs::Error, "Spell ID %i query of spell_globals with Name: '%s' Value: '%i' failed", spell_ID, spell_Global_Name.c_str(), spell_Global_Value);
        return false;
    }

    if (results.RowCount() != 1) {
        Log(Logs::General, Logs::Error, "Char ID: %i does not have the Qglobal Name: '%s' for Spell ID %i", char_ID, spell_Global_Name.c_str(), spell_ID);
        return false;
    }

    row = results.begin();

    global_Value = atoi(row[0]);

    if (global_Value == spell_Global_Value)
        return true; // If the values match from both tables, allow the spell to be scribed
    else if (global_Value > spell_Global_Value)
        return true; // Check if the qglobal value is greater than the require spellglobal value

    // If no matching result found in qglobals, don't scribe this spell
    Log(Logs::General, Logs::Error, "Char ID: %i Spell_globals Name: '%s' Value: '%i' did not match QGlobal Value: '%i' for Spell ID %i", char_ID, spell_Global_Name.c_str(), spell_Global_Value, global_Value, spell_ID);
    return false;
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
	int buff_count = GetMaxTotalSlots();
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
						Log(Logs::General, Logs::Normal,
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

bool Mob::IsCombatProc(uint16 spell_id) {

	if (RuleB(Spells, FocusCombatProcs))
		return false;

	if(spell_id == SPELL_UNKNOWN)
		return(false);

	if ((spells[spell_id].cast_time == 0) && (spells[spell_id].recast_time == 0) && (spells[spell_id].recovery_time == 0))
	{

		for (int i = 0; i < MAX_PROCS; i++){
			if (PermaProcs[i].spellID == spell_id || SpellProcs[i].spellID == spell_id
				 || RangedProcs[i].spellID == spell_id){
				return true;
			}
		}
	}

	return false;
}

bool Mob::AddProcToWeapon(uint16 spell_id, bool bPerma, uint16 iChance, uint16 base_spell_id, int level_override) {
	if(spell_id == SPELL_UNKNOWN)
		return(false);

	int i;
	if (bPerma) {
		for (i = 0; i < MAX_PROCS; i++) {
			if (PermaProcs[i].spellID == SPELL_UNKNOWN) {
				PermaProcs[i].spellID = spell_id;
				PermaProcs[i].chance = iChance;
				PermaProcs[i].base_spellID = base_spell_id;
				PermaProcs[i].level_override = level_override;
				Log(Logs::Detail, Logs::Spells, "Added permanent proc spell %d with chance %d to slot %d", spell_id, iChance, i);

				return true;
			}
		}
		Log(Logs::Detail, Logs::Spells, "Too many perma procs for %s", GetName());
	} else {
		for (i = 0; i < MAX_PROCS; i++) {
			if (SpellProcs[i].spellID == SPELL_UNKNOWN) {
				SpellProcs[i].spellID = spell_id;
				SpellProcs[i].chance = iChance;
				SpellProcs[i].base_spellID = base_spell_id;;
				SpellProcs[i].level_override = level_override;
				Log(Logs::Detail, Logs::Spells, "Added spell-granted proc spell %d with chance %d to slot %d", spell_id, iChance, i);
				return true;
			}
		}
		Log(Logs::Detail, Logs::Spells, "Too many procs for %s", GetName());
	}
	return false;
}

bool Mob::RemoveProcFromWeapon(uint16 spell_id, bool bAll) {
	for (int i = 0; i < MAX_PROCS; i++) {
		if (bAll || SpellProcs[i].spellID == spell_id) {
			SpellProcs[i].spellID = SPELL_UNKNOWN;
			SpellProcs[i].chance = 0;
			SpellProcs[i].base_spellID = SPELL_UNKNOWN;
			SpellProcs[i].level_override = -1;
			Log(Logs::Detail, Logs::Spells, "Removed proc %d from slot %d", spell_id, i);
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
			Log(Logs::Detail, Logs::Spells, "Added spell-granted defensive proc spell %d with chance %d to slot %d", spell_id, iChance, i);
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
			Log(Logs::Detail, Logs::Spells, "Removed defensive proc %d from slot %d", spell_id, i);
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
			Log(Logs::Detail, Logs::Spells, "Added spell-granted ranged proc spell %d with chance %d to slot %d", spell_id, iChance, i);
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
			Log(Logs::Detail, Logs::Spells, "Removed ranged proc %d from slot %d", spell_id, i);
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
		slot = static_cast<int>(casting_spell_slot);

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
	Log(Logs::Detail, Logs::Spells, "Determined effective casting level %d+%d+%d=%d", GetLevel(), spellbonuses.effective_casting_level, itembonuses.effective_casting_level, level);
	return std::max(1, level);
}

//this method does NOT tell the client to stop singing the song.
//this is NOT the right way to stop a mob from singing, use InterruptSpell
//you should really know what your doing before you call this
void Mob::_StopSong()
{
	bardsong = 0;
	bardsong_target_id = 0;
	bardsong_slot = CastingSlot::Gem1;
	bardsong_timer.Disable();
}

//This member function sets the buff duration on the client
//however it does not work if sent quickly after an action packets, which is what one might perfer to do
//Thus I use this in the buff process to update the correct duration once after casting
//this allows AAs and focus effects that increase buff duration to work correctly, but could probably
//be used for other things as well
void Client::SendBuffDurationPacket(Buffs_Struct &buff, int slot)
{
	EQApplicationPacket* outapp = nullptr;
	outapp = new EQApplicationPacket(OP_Buff, sizeof(SpellBuffPacket_Struct));
	SpellBuffPacket_Struct* sbf = (SpellBuffPacket_Struct*) outapp->pBuffer;

	sbf->entityid = GetID();

	sbf->buff.effect_type = 2;

	sbf->buff.level = buff.casterlevel > 0 ? buff.casterlevel : GetLevel();
	sbf->buff.bard_modifier = buff.instrument_mod;
	sbf->buff.spellid = buff.spellid;
	sbf->buff.duration = buff.ticsremaining;
	if (buff.dot_rune)
		sbf->buff.counters = buff.dot_rune;
	else if (buff.magic_rune)
		sbf->buff.counters = buff.magic_rune;
	else if (buff.melee_rune)
		sbf->buff.counters = buff.melee_rune;
	else if (buff.counters)
		sbf->buff.counters = buff.counters;
	sbf->buff.player_id = buff.casterid;
	sbf->buff.num_hits = buff.numhits;
	sbf->buff.y = buff.caston_y;
	sbf->buff.x = buff.caston_x;
	sbf->buff.z = buff.caston_z;

	sbf->slotid = slot;
	sbf->bufffade = 0;
	FastQueuePacket(&outapp);
}

void Client::SendBuffNumHitPacket(Buffs_Struct &buff, int slot)
{
	// UF+ use this packet
	if (ClientVersion() < EQEmu::versions::ClientVersion::UF)
		return;
	EQApplicationPacket *outapp = nullptr;
	outapp = new EQApplicationPacket(OP_BuffCreate, sizeof(BuffIcon_Struct) + sizeof(BuffIconEntry_Struct));
	BuffIcon_Struct *bi = (BuffIcon_Struct *)outapp->pBuffer;
	bi->entity_id = GetID();
	bi->count = 1;
	bi->all_buffs = 0;
	bi->tic_timer = tic_timer.GetRemainingTime();

	bi->entries[0].buff_slot = slot;
	bi->entries[0].spell_id = buff.spellid;
	bi->entries[0].tics_remaining = buff.ticsremaining;
	bi->entries[0].num_hits = buff.numhits;
	strn0cpy(bi->entries[0].caster, buff.caster_name, 64);
	bi->name_lengths = strlen(bi->entries[0].caster);
	FastQueuePacket(&outapp);
}

void Mob::SendPetBuffsToClient()
{
	// Don't really need this check, as it should be checked before this method is called, but it doesn't hurt
	// too much to check again.
	if(!(GetOwner() && GetOwner()->IsClient()))
		return;

	int PetBuffCount = 0;

	auto outapp = new EQApplicationPacket(OP_PetBuffWindow, sizeof(PetBuff_Struct));
	PetBuff_Struct* pbs=(PetBuff_Struct*)outapp->pBuffer;
	memset(outapp->pBuffer,0,outapp->size);
	pbs->petid=GetID();

	int MaxSlots = GetMaxTotalSlots();

	if(MaxSlots > PET_BUFF_COUNT)
		MaxSlots = PET_BUFF_COUNT;

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

	if (c->ClientVersionBit() & EQEmu::versions::bit_SoDAndLater)
	{
		EQApplicationPacket *outapp = MakeBuffsPacket();
		c->FastQueuePacket(&outapp);
	}
}

EQApplicationPacket *Mob::MakeBuffsPacket(bool for_target)
{
	uint32 count = 0;
	// for self we want all buffs, for target, we want to skip song window buffs
	// since NPCs and pets don't have a song window, we still see it for them :P
	uint32 buff_count = for_target ? GetMaxBuffSlots() : GetMaxTotalSlots();
	for(int i = 0; i < buff_count; ++i)
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
	buff->all_buffs = 1;
	buff->tic_timer = tic_timer.GetRemainingTime();
	// there are more types, the client doesn't seem to really care though. The others are also currently hard to fill in here ...
	// (see comment in common/eq_packet_structs.h)
	if (for_target)
		buff->type = IsClient() ? 5 : 7;
	else
		buff->type = 0;

	buff->name_lengths = 0; // hacky shit
	uint32 index = 0;
	for(int i = 0; i < buff_count; ++i)
	{
		if(buffs[i].spellid != SPELL_UNKNOWN)
		{
			buff->entries[index].buff_slot = i;
			buff->entries[index].spell_id = buffs[i].spellid;
			buff->entries[index].tics_remaining = buffs[i].ticsremaining;
			buff->entries[index].num_hits = buffs[i].numhits;
			strn0cpy(buff->entries[index].caster, buffs[i].caster_name, 64);
			buff->name_lengths += strlen(buff->entries[index].caster);
			++index;
		}
	}

	return outapp;
}

void Mob::BuffModifyDurationBySpellID(uint16 spell_id, int32 newDuration)
{
	int buff_count = GetMaxTotalSlots();
	for(int i = 0; i < buff_count; ++i)
	{
		if (buffs[i].spellid == spell_id)
		{
			buffs[i].ticsremaining = newDuration;
			if(IsClient())
			{
				CastToClient()->SendBuffDurationPacket(buffs[i], i);
			}
		}
	}
}

int Client::GetCurrentBuffSlots() const
{
	int numbuffs = 15;
	// client does check spells and items
	numbuffs += aabonuses.BuffSlotIncrease + spellbonuses.BuffSlotIncrease + itembonuses.BuffSlotIncrease;
	if (GetLevel() > 70)
		numbuffs++;
	if (GetLevel() > 74)
		numbuffs++;
	return EQEmu::ClampUpper(numbuffs, EQEmu::constants::Lookup(m_ClientVersion)->LongBuffs);
}

int Client::GetCurrentSongSlots() const
{
	return EQEmu::constants::Lookup(m_ClientVersion)->ShortBuffs; // AAs dont affect this
}

void Client::InitializeBuffSlots()
{
	int max_slots = GetMaxTotalSlots();
	buffs = new Buffs_Struct[max_slots];
	for(int x = 0; x < max_slots; ++x)
	{
		buffs[x].spellid = SPELL_UNKNOWN;
		buffs[x].UpdateClient = false;
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
		buffs[x].UpdateClient = false;
	}
	current_buff_count = 0;
}

void NPC::UninitializeBuffSlots()
{
	safe_delete_array(buffs);
}

void Client::SendSpellAnim(uint16 targetid, uint16 spell_id)
{
	if (!targetid || !IsValidSpell(spell_id))
		return;

	EQApplicationPacket app(OP_Action, sizeof(Action_Struct));
	Action_Struct* a = (Action_Struct*)app.pBuffer;
	a->target = targetid;
	a->source = this->GetID();
	a->type = 231;
	a->spell = spell_id;
	a->sequence = 231;

	app.priority = 1;
	entity_list.QueueCloseClients(this, &app, false, RuleI(Range, SpellParticles));
}

void Mob::CalcDestFromHeading(float heading, float distance, float MaxZDiff, float StartX, float StartY, float &dX, float &dY, float &dZ)
{
	if (!distance) { return; }
	if (!MaxZDiff) { MaxZDiff = 5; }

	float ReverseHeading = 256 - heading;
	float ConvertAngle = ReverseHeading * 1.40625f;
	if (ConvertAngle <= 270)
		ConvertAngle = ConvertAngle + 90;
	else
		ConvertAngle = ConvertAngle - 270;

	float Radian = ConvertAngle * (3.1415927f / 180.0f);

	float CircleX = distance * cos(Radian);
	float CircleY = distance * sin(Radian);
	dX = CircleX + StartX;
	dY = CircleY + StartY;
	dZ = FindGroundZ(dX, dY, MaxZDiff);
}

void Mob::BeamDirectional(uint16 spell_id, int16 resist_adjust)
{
	int maxtarget_count = 0;
	bool beneficial_targets = false;

	if (IsBeneficialSpell(spell_id) && IsClient())
		beneficial_targets = true;

	std::list<Mob *> targets_in_range;

	entity_list.GetTargetsForConeArea(this, spells[spell_id].min_range, spells[spell_id].range,
					  spells[spell_id].range / 2, spells[spell_id].pcnpc_only_flag, targets_in_range);
	auto iter = targets_in_range.begin();

	float dX = 0;
	float dY = 0;
	float dZ = 0;

	CalcDestFromHeading(GetHeading(), spells[spell_id].range, 5, GetX(), GetY(), dX, dY, dZ);
	dZ = GetZ();

	// FIND SLOPE: Put it into the form y = mx + b
	float m = (dY - GetY()) / (dX - GetX());
	float b = (GetY() * dX - dY * GetX()) / (dX - GetX());

	while (iter != targets_in_range.end()) {
		if (!(*iter) || (beneficial_targets && ((*iter)->IsNPC() && !(*iter)->IsPetOwnerClient())) ||
		    (*iter)->BehindMob(this, (*iter)->GetX(), (*iter)->GetY())) {
			++iter;
			continue;
		}

		if (IsNPC() && (*iter)->IsNPC()) {
			auto fac = (*iter)->GetReverseFactionCon(this);
			if (beneficial_targets) {
				// only affect mobs we would assist.
				if (!(fac <= FACTION_AMIABLE)) {
					++iter;
					continue;
				}
			} else {
				// affect mobs that are on our hate list, or which have bad faction with us
				if (!(CheckAggro(*iter) || fac == FACTION_THREATENLY || fac == FACTION_SCOWLS)) {
					++iter;
					continue;
				}
			}
		}

		//# shortest distance from line to target point
		float d = std::abs((*iter)->GetY() - m * (*iter)->GetX() - b) / sqrt(m * m + 1);

		if (d <= spells[spell_id].aoerange) {
			if (CheckLosFN((*iter)) || spells[spell_id].npc_no_los) {
				(*iter)->CalcSpellPowerDistanceMod(spell_id, 0, this);
				SpellOnTarget(spell_id, (*iter), false, true, resist_adjust);
				maxtarget_count++;
			}

			if (maxtarget_count >= spells[spell_id].aemaxtargets)
				return;
		}
		++iter;
	}
}

void Mob::ConeDirectional(uint16 spell_id, int16 resist_adjust)
{
	int maxtarget_count = 0;
	bool beneficial_targets = false;

	if (IsBeneficialSpell(spell_id) && IsClient())
		beneficial_targets = true;

	float angle_start = spells[spell_id].directional_start + (GetHeading() * 360.0f / 256.0f);
	float angle_end = spells[spell_id].directional_end + (GetHeading() * 360.0f / 256.0f);

	while (angle_start > 360.0f)
		angle_start -= 360.0f;

	while (angle_end > 360.0f)
		angle_end -= 360.0f;

	std::list<Mob *> targets_in_range;

	entity_list.GetTargetsForConeArea(this, spells[spell_id].min_range, spells[spell_id].aoerange,
					  spells[spell_id].aoerange / 2, spells[spell_id].pcnpc_only_flag, targets_in_range);
	auto iter = targets_in_range.begin();

	while (iter != targets_in_range.end()) {
		if (!(*iter) || (beneficial_targets && ((*iter)->IsNPC() && !(*iter)->IsPetOwnerClient()))) {
			++iter;
			continue;
		}

		float heading_to_target =
		    (CalculateHeadingToTarget((*iter)->GetX(), (*iter)->GetY()) * 360.0f / 256.0f);

		while (heading_to_target < 0.0f)
			heading_to_target += 360.0f;

		while (heading_to_target > 360.0f)
			heading_to_target -= 360.0f;

		if (IsNPC() && (*iter)->IsNPC()) {
			auto fac = (*iter)->GetReverseFactionCon(this);
			if (beneficial_targets) {
				// only affect mobs we would assist.
				if (!(fac <= FACTION_AMIABLE)) {
					++iter;
					continue;
				}
			} else {
				// affect mobs that are on our hate list, or which have bad faction with us
				if (!(CheckAggro(*iter) || fac == FACTION_THREATENLY || fac == FACTION_SCOWLS)) {
					++iter;
					continue;
				}
			}
		}

		if (angle_start > angle_end) {
			if ((heading_to_target >= angle_start && heading_to_target <= 360.0f) ||
			    (heading_to_target >= 0.0f && heading_to_target <= angle_end)) {
				if (CheckLosFN((*iter)) || spells[spell_id].npc_no_los) {
					(*iter)->CalcSpellPowerDistanceMod(spell_id, 0, this);
					SpellOnTarget(spell_id, (*iter), false, true, resist_adjust);
					maxtarget_count++;
				}
			}
		} else {
			if (heading_to_target >= angle_start && heading_to_target <= angle_end) {
				if (CheckLosFN((*iter)) || spells[spell_id].npc_no_los) {
					(*iter)->CalcSpellPowerDistanceMod(spell_id, 0, this);
					SpellOnTarget(spell_id, (*iter), false, true, resist_adjust);
					maxtarget_count++;
				}
			}
		}

		if (maxtarget_count >= spells[spell_id].aemaxtargets)
			return;

		++iter;
	}
}

// duration in seconds
void Client::SetLinkedSpellReuseTimer(uint32 timer_id, uint32 duration)
{
	if (timer_id > 19)
		return;
	Log(Logs::Detail, Logs::Spells, "Setting Linked Spell Reuse %d for %d", timer_id, duration);
	GetPTimers().Start(pTimerLinkedSpellReuseStart + timer_id, duration);
	auto outapp = new EQApplicationPacket(OP_LinkedReuse, sizeof(LinkedSpellReuseTimer_Struct));
	auto lr = (LinkedSpellReuseTimer_Struct *)outapp->pBuffer;
	lr->timer_id = timer_id;
	lr->start_time = Timer::GetCurrentTime() / 1000;
	lr->end_time = lr->start_time + duration;
	FastQueuePacket(&outapp);
}

bool Client::IsLinkedSpellReuseTimerReady(uint32 timer_id)
{
	if (timer_id > 19)
		return true;
	return GetPTimers().Expired(&database, pTimerLinkedSpellReuseStart + timer_id, false);
}

