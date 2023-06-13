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



#include "../common/eqemu_logsys.h"

#include "classes.h"
#include "spdat.h"
#include "../common/rulesys.h"
#include "../common/strings.h"

#ifndef WIN32
#include <stdlib.h>
#include "unix.h"
#endif

///////////////////////////////////////////////////////////////////////////////
// spell property testing functions

bool IsTargetableAESpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	return spells[spell_id].target_type == ST_AETarget;
}

bool IsSacrificeSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_Sacrifice);
}

bool IsLifetapSpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	const auto& spell = spells[spell_id];

	if (
		spell.target_type == ST_Tap ||
		spell.target_type == ST_TargetAETap ||
		spell_id == SPELL_ANCIENT_LIFEBANE
	) {
		return true;
	}

	return false;
}

bool IsMesmerizeSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_Mez);
}

bool IsStunSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_Stun);
}

bool IsSummonSpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	const auto& spell = spells[spell_id];

	for (int i = 0; i < EFFECT_COUNT; i++) {
		const auto effect_id = spell.effect_id[i];
		if (
			effect_id == SE_SummonPet ||
			effect_id == SE_SummonItem ||
			effect_id == SE_SummonPC
		) {
			return true;
		}
	}

	return false;
}

bool IsDamageSpell(uint16 spell_id)
{
	if (IsLifetapSpell(spell_id)) {
		return false;
	}

	const auto& spell = spells[spell_id];

	for (int i = 0; i < EFFECT_COUNT; i++) {
		const auto effect_id = spell.effect_id[i];
		if (
			spell.base_value[i] < 0 &&
			(
				effect_id == SE_CurrentHPOnce ||
				(
					effect_id == SE_CurrentHP &&
					spell.buff_duration < 1
				)
			)
		) {
			return true;
		}
	}

	return false;
}


bool IsFearSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_Fear);
}

bool IsCureSpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	auto has_cure_effect = false;

	const auto& spell = spells[spell_id];

	for (int i = 0; i < EFFECT_COUNT; i++) {
		if (
			spell.effect_id[i] == SE_DiseaseCounter ||
			spell.effect_id[i] == SE_PoisonCounter ||
			spell.effect_id[i] == SE_CurseCounter ||
			spell.effect_id[i] == SE_CorruptionCounter
		) {
			has_cure_effect = true;
		}
	}

	if (
		has_cure_effect &&
		IsBeneficialSpell(spell_id)
	) {
		return true;
	}

	return false;
}

bool IsSlowSpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	const auto& spell = spells[spell_id];

	for (int i = 0; i < EFFECT_COUNT; i++) {
		const auto effect_id = spell.effect_id[i];
		if (
			effect_id == SE_AttackSpeed4 ||
			(
				effect_id == SE_AttackSpeed &&
				spell.base_value[i] < 100
			)
		) {
			return true;
		}
	}

	return false;
}

bool IsHasteSpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	const auto& spell = spells[spell_id];

	for (int i = 0; i < EFFECT_COUNT; i++) {
		if (spell.effect_id[i] == SE_AttackSpeed) {
			return (spell.base_value[i] < 100);
		}
	}

	return false;
}

bool IsHarmonySpell(uint16 spell_id)
{
	return (
		IsEffectInSpell(spell_id, SE_ChangeFrenzyRad) ||
		IsEffectInSpell(spell_id, SE_Harmony) ||
		IsEffectInSpell(spell_id, SE_Lull)
	);
}

bool IsPercentalHealSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_PercentalHeal);
}

bool IsGroupOnlySpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	return spells[spell_id].good_effect == BENEFICIAL_EFFECT_GROUP_ONLY;
}

bool IsBeneficialSpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	// You'd think just checking goodEffect flag would be enough?
	if (spells[spell_id].good_effect == BENEFICIAL_EFFECT) {
		// If the target type is ST_Self or ST_Pet and is a SE_CancleMagic spell
		// it is not Beneficial
		const auto target_type = spells[spell_id].target_type;
		if (
			target_type != ST_Self &&
			target_type != ST_Pet &&
			IsEffectInSpell(spell_id, SE_CancelMagic)
		) {
			return false;
		}

		// When our targetarget_typeype is ST_Target, ST_AETarget, ST_Aniaml, ST_Undead, or ST_Pet
		// We need to check more things!
		if (
			target_type == ST_Target ||
			target_type == ST_AETarget ||
			target_type == ST_Animal ||
			target_type == ST_Undead ||
			target_type == ST_Pet
		) {
			const auto spell_affect_index = spells[spell_id].spell_affect_index;

			// If the resisttype is magic and SpellAffectIndex is Calm/memblur/dispell sight
			// it's not beneficial
			if (spells[spell_id].resist_type == RESIST_MAGIC) {
				// checking these SAI cause issues with the rng defensive proc line
				// So I guess instead of fixing it for real, just a quick hack :P
				if (
					spells[spell_id].effect_id[0] != SE_DefensiveProc &&
					(
						spell_affect_index == SAI_Calm ||
						spell_affect_index == SAI_Dispell_Sight ||
						spell_affect_index == SAI_Memory_Blur ||
						spell_affect_index == SAI_Calm_Song
					)
				) {
					return false;
				}
			} else {
				// If the resisttype is not magic and spell is Bind Sight or Cast Sight
				// It's not beneficial
				if (
					(
						spell_affect_index == SAI_Calm &&
						IsEffectInSpell(spell_id, SE_Harmony)
					) ||
					(
						spell_affect_index == SAI_Calm_Song &&
						IsEffectInSpell(spell_id, SE_BindSight)
					) ||
					(
						spell_affect_index == SAI_Dispell_Sight &&
						spells[spell_id].skill == EQ::skills::SkillDivination &&
						!IsEffectInSpell(spell_id, SE_VoiceGraft)
					)
				) {
					return false;
				}
			}
		}
	}

	// And finally, if goodEffect is not 0 or if it's a group spell it's beneficial
	return (
		spells[spell_id].good_effect != DETRIMENTAL_EFFECT ||
		IsGroupSpell(spell_id)
	);
}

bool IsDetrimentalSpell(uint16 spell_id)
{
	return !IsBeneficialSpell(spell_id);
}

bool IsInvisibleSpell(uint16 spell_id)
{
	return (
		IsEffectInSpell(spell_id, SE_Invisibility) ||
		IsEffectInSpell(spell_id, SE_Invisibility2) ||
		IsEffectInSpell(spell_id, SE_InvisVsUndead) ||
		IsEffectInSpell(spell_id, SE_InvisVsUndead2) ||
		IsEffectInSpell(spell_id, SE_InvisVsAnimals) ||
		IsEffectInSpell(spell_id, SE_ImprovedInvisAnimals)
	);
}

bool IsInvulnerabilitySpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_DivineAura);
}

bool IsCompleteHealDurationSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_CompleteHeal);
}

bool IsPoisonCounterSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_PoisonCounter);
}

bool IsDiseaseCounterSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_DiseaseCounter);
}

bool IsSummonItemSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_SummonItem);
}

bool IsSummonSkeletonSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_NecPet);
}

bool IsSummonPetSpell(uint16 spell_id)
{
	return (
		IsEffectInSpell(spell_id, SE_SummonPet) ||
		IsEffectInSpell(spell_id, SE_SummonBSTPet) ||
		IsEffectInSpell(spell_id, SE_Familiar)
	);
}

bool IsPetSpell(uint16 spell_id)
{
	return (
		IsSummonPetSpell(spell_id) ||
		IsSummonSkeletonSpell(spell_id)
	);
}

bool IsSummonPCSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_SummonPC);
}

bool IsCharmSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_Charm);
}

bool IsBlindSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_Blind);
}

bool IsHealthSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_CurrentHP);
}

bool IsCastTimeReductionSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_IncreaseSpellHaste);
}

bool IsIncreaseDurationSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_IncreaseSpellDuration);
}

bool IsManaCostReductionSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_ReduceManaCost);
}

bool IsIncreaseRangeSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_IncreaseRange);
}

bool IsImprovedHealingSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_ImprovedHeal);
}

bool IsImprovedDamageSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_ImprovedDamage);
}

bool IsAEDurationSpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	const auto& spell = spells[spell_id];

	/*
		There are plenty of spells with aoe_duration set at single digit numbers, but these
		do not act as duration effects.
	*/
	if (
		spell.aoe_duration >= 2500 &&
		(
			spell.target_type == ST_AETarget ||
			spell.target_type == ST_UndeadAE ||
			spell.target_type == ST_AECaster ||
			spell.target_type == ST_Ring
		)
	) {
		return true;
	}

	return false;
}

bool IsPureNukeSpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	auto effect_count = 0;

	for (int i = 0; i < EFFECT_COUNT; i++) {
		if (!IsBlankSpellEffect(spell_id, i)) {
			effect_count++;
		}
	}

	if (
		effect_count == 1 &&
		IsEffectInSpell(spell_id, SE_CurrentHP) &&
		spells[spell_id].buff_duration == 0 &&
		IsDamageSpell(spell_id)
	) {
		return true;
	}

	return false;
}

bool IsAENukeSpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	if (
		IsPureNukeSpell(spell_id) &&
		spells[spell_id].aoe_range > 0
	) {
		return true;
	}

	return false;
}

bool IsPBAENukeSpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	const auto& spell = spells[spell_id];

	if (
		IsPureNukeSpell(spell_id) &&
		spell.aoe_range > 0 &&
		spell.target_type == ST_AECaster
	) {
		return true;
	}

	return false;
}

bool IsAERainNukeSpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	const auto& spell = spells[spell_id];

	if (
		IsPureNukeSpell(spell_id) &&
		spell.aoe_range > 0 &&
		spell.aoe_duration > 1000
	) {
		return true;
	}

	return false;
}

bool IsPartialResistableSpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	const auto& spell = spells[spell_id];

	if (spell.no_partial_resist) {
		return false;
	}

	// spell uses 600 (partial) scale if first effect is damage, else it uses 200 scale.
	// this includes DoTs.  no_partial_resist excludes spells like necro snares
	for (int o = 0; o < EFFECT_COUNT; o++) {
		const auto effect_id = spell.effect_id[o];

		if (IsBlankSpellEffect(spell_id, o)) {
			continue;
		}

		if (
			spell.base_value[o] < 0 &&
			(
				effect_id == SE_CurrentHPOnce ||
				effect_id == SE_CurrentHP
			)
		) {
			return true;
		}

		return false;
	}

	return false;
}

bool IsResistableSpell(uint16 spell_id)
{
	// for now only detrimental spells are resistable. later on i will
	// add specific exceptions for the beneficial spells that are resistable
	return IsDetrimentalSpell(spell_id);
}

// checks if this spell affects your group
bool IsGroupSpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	const auto& spell = spells[spell_id];

	return (
		spell.target_type == ST_AEBard ||
		spell.target_type == ST_Group ||
		spell.target_type == ST_GroupTeleport
	);
}

// checks if this spell can be targeted
bool IsTGBCompatibleSpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	if (
		!IsDetrimentalSpell(spell_id) &&
		spells[spell_id].buff_duration != 0 &&
		!IsBardSong(spell_id) &&
		!IsEffectInSpell(spell_id, SE_Illusion)
	) {
		return true;
	}

	return false;
}

bool IsBardSong(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	const auto& spell = spells[spell_id];

	if (
		spell.classes[BARD - 1] < UINT8_MAX &&
		!spell.is_discipline
	) {
		return true;
	}

	return false;
}

bool IsEffectInSpell(uint16 spell_id, int effect_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	const auto& spell = spells[spell_id];

	for (int i = 0; i < EFFECT_COUNT; i++) {
		if (spell.effect_id[i] == effect_id) {
			return true;
		}
	}

	return false;
}

uint16 GetSpellTriggerSpellID(uint16 spell_id, int effect_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	const auto& spell = spells[spell_id];

	for (int i = 0; i < EFFECT_COUNT; i++) {
		if (
			spell.effect_id[i] == SE_TriggerOnCast ||
			spell.effect_id[i] == SE_SpellTrigger ||
			spell.effect_id[i] == SE_ApplyEffect ||
			spell.effect_id[i] == SE_Trigger_Spell_Non_Item
		) {
			const auto trigger_spell_id = spell.limit_value[i];
			if (
				IsValidSpell(trigger_spell_id) &&
				IsEffectInSpell(trigger_spell_id, effect_id)
			) {
				return trigger_spell_id;
			}
		}
	}

	return 0;
}

// arguments are spell id and the index of the effect to check.
// this is used in loops that process effects inside a spell to skip
// the blanks
bool IsBlankSpellEffect(uint16 spell_id, int effect_index)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	const auto& spell = spells[spell_id];

	const auto effect     = spell.effect_id[effect_index];
	const auto base_value = spell.base_value[effect_index];
	const auto formula    = spell.formula[effect_index];

	// SE_CHA is "spacer"
	// SE_Stacking* are also considered blank where this is used
	if (
		effect == SE_Blank ||
		(
			effect == SE_CHA &&
			base_value == 0 &&
			formula == 100
		) ||
		effect == SE_StackingCommand_Block ||
		effect == SE_StackingCommand_Overwrite
	) {
		return true;
	}

	return false;
}

// checks some things about a spell id, to see if we can proceed
bool IsValidSpell(uint32 spell_id)
{
	if (
		SPDAT_RECORDS > 0 &&
		spell_id >= 2 &&
		spell_id != UINT32_MAX &&
		spell_id < SPDAT_RECORDS &&
		spells[spell_id].player_1[0]
	) {
		return true;
	}

	return false;
}

// returns the lowest level of any caster which can use the spell
uint8 GetSpellMinimumLevel(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return UINT8_MAX;
	}

	uint8 minimum_level = UINT8_MAX;

	const auto& spell = spells[spell_id];

	for (int i = 0; i < PLAYER_CLASS_COUNT; i++) {
		if (spell.classes[i] < minimum_level) {
			minimum_level = spell.classes[i];
		}
	}

	// if we can't cast the spell return 0
	// just so it wont screw up calculations used in other areas of the code
	// seen 127, 254, 255
	return minimum_level;
}

uint8 GetSpellLevel(uint16 spell_id, uint8 class_id)
{
	if (!IsValidSpell(spell_id)) {
		return UINT8_MAX;
	}

	if (class_id >= PLAYER_CLASS_COUNT) {
		return UINT8_MAX;
	}

	return spells[spell_id].classes[class_id - 1];
}

// this will find the first occurrence of effect. this is handy
// for spells like mez and charm, but if the effect appears more than once
// in a spell this will just give back the first one.
int GetSpellEffectIndex(uint16 spell_id, int effect_id)
{
	if (!IsValidSpell(spell_id)) {
		return -1;
	}

	const auto& spell = spells[spell_id];

	for (int i = 0; i < EFFECT_COUNT; i++) {
		if (spell.effect_id[i] == effect_id) {
			return i;
		}
	}

	return -1;
}

int CalculatePoisonCounters(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return 0;
	}

	const auto& spell = spells[spell_id];

	auto counters = 0;

	for (int i = 0; i < EFFECT_COUNT; i++) {
		if (
			spell.effect_id[i] == SE_PoisonCounter &&
			spell.base_value[i] > 0
		) {
			counters += spell.base_value[i];
		}
	}

	return counters;
}

int CalculateDiseaseCounters(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return 0;
	}

	const auto& spell = spells[spell_id];

	auto counters = 0;

	for (int i = 0; i < EFFECT_COUNT; i++) {
		if (
			spell.effect_id[i] == SE_DiseaseCounter &&
			spell.base_value[i] > 0
		) {
			counters += spell.base_value[i];
		}
	}

	return counters;
}

int CalculateCurseCounters(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return 0;
	}

	const auto& spell = spells[spell_id];

	auto counters = 0;

	for (int i = 0; i < EFFECT_COUNT; i++) {
		if (
			spell.effect_id[i] == SE_CurseCounter &&
			spell.base_value[i] > 0
		) {
			counters += spell.base_value[i];
		}
	}

	return counters;
}

int CalculateCorruptionCounters(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return 0;
	}

	const auto& spell = spells[spell_id];

	auto counters = 0;

	for (int i = 0; i < EFFECT_COUNT; i++) {
		if (
			spell.effect_id[i] == SE_CorruptionCounter &&
			spell.base_value[i] > 0) {

			counters += spell.base_value[i];
		}
	}

	return counters;
}

int CalculateCounters(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return 0;
	}

	auto counter = CalculatePoisonCounters(spell_id);
	if (counter) {
		return counter;
	}

	counter = CalculateDiseaseCounters(spell_id);
	if (counter) {
		return counter;
	}

	counter = CalculateCurseCounters(spell_id);
	if (counter) {
		return counter;
	}

	counter = CalculateCorruptionCounters(spell_id);
	return counter;
}

bool IsDisciplineBuff(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	const auto& spell = spells[spell_id];

	if (
		spell.is_discipline &&
		spell.target_type == ST_Self
	) {
		return true;
	}

	return false;
}

bool IsDiscipline(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	const auto& spell = spells[spell_id];

	if (
		spell.mana == 0 &&
		(
			spell.endurance_cost ||
			spell.endurance_upkeep
		)
	) {
		return true;
	}

	return false;
}

bool IsCombatSkill(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	const auto& spell = spells[spell_id];

	//Check if Discipline
	if (
		spell.mana == 0 &&
		(
			spell.endurance_cost ||
			spell.endurance_upkeep
		)
	) {
		return true;
	}

	return false;
}

bool IsResurrectionEffects(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	// spell id 756 is Resurrection Effects spell
	if (
		spell_id == RuleI(Character, ResurrectionSicknessSpellID) ||
		spell_id == RuleI(Character, OldResurrectionSicknessSpellID)
	) {
		return true;
	}

	return false;
}

int8 GetSpellResurrectionSicknessCheck(uint16 spell_id_one, uint16 spell_id_two)
{
	if (
		!IsValidSpell(spell_id_one) ||
		!IsValidSpell(spell_id_two)
	) {
		return RuleI(Spells, ResurrectionEffectBlock);
	}

	const int8  rule_value = RuleI(Spells, ResurrectionEffectBlock);
	const auto& spell_one  = spells[spell_id_one];
	const auto& spell_two  = spells[spell_id_two];

	if (rule_value == RES_EFFECTS_BLOCK) {
		LogSpells(
			"[{}] is blocked by [{}]",
			spell_two.name,
			spell_one.name
		);
		return RES_EFFECTS_CANNOT_STACK;
	} else if (rule_value == RES_EFFECTS_BLOCK_WITH_BUFFS) {
		LogSpells(
			"[{}] is blocked by [{}], moving to empty slot if available",
			spell_one.name,
			spell_two.name
		);
		return MOVE_NEW_SLOT; // move to empty slot if available
	}

	return NO_RES_EFFECTS_BLOCK;
}

bool IsRuneSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_Rune);
}

bool IsMagicRuneSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_AbsorbMagicAtt);
}

bool IsManaTapSpell(uint16 spell_id)
{
	return (
		IsEffectInSpell(spell_id, SE_CurrentMana) &&
		spells[spell_id].target_type == ST_Tap
	);
}

bool IsAllianceSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_AddFaction);
}

bool IsDeathSaveSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_DeathSave);
}

// Deathsave spells with base of 1 are partial
bool IsPartialDeathSaveSpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	const auto& spell = spells[spell_id];

	for (int i = 0; i < EFFECT_COUNT; i++) {
		if (
			spell.effect_id[i] == SE_DeathSave &&
			spell.base_value[i] == PARTIAL_DEATH_SAVE
		) {
			return true;
		}
	}

	return false;
}

// Deathsave spells with base 2 are "full"
bool IsFullDeathSaveSpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	const auto& spell = spells[spell_id];

	for (int i = 0; i < EFFECT_COUNT; i++) {
		if (
			spell.effect_id[i] == SE_DeathSave &&
			spell.base_value[i] == FULL_DEATH_SAVE
		) {
			return true;
		}
	}

	return false;
}

bool IsShadowStepSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_ShadowStep);
}

bool IsSuccorSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_Succor);
}

bool IsTeleportSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_Teleport);
}

bool IsTranslocateSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_Translocate);
}

bool IsGateSpell(uint16 spell_id)
{
	return IsEffectInSpell(spell_id, SE_Gate);
}

bool IsIllusionSpell(uint16 spell_id)
{
	return (
		IsEffectInSpell(spell_id, SE_Illusion) &&
		spells[spell_id].target_type == ST_Self
	);
}

int GetSpellEffectDescriptionNumber(uint16 spell_id)
{
	if (IsValidSpell(spell_id)) {
		return spells[spell_id].effect_description_id;
	}

	return -1;
}

DmgShieldType GetDamageShieldType(uint16 spell_id, int damage_shield_type)
{
	if (!IsValidSpell(spell_id)) {
		return DS_THORNS;
	}

	const auto& spell = spells[spell_id];

	// If we have a DamageShieldType for this spell from the damageshieldtypes table, return that,
	// else, make a guess, based on the resist type. Default return value is DS_THORNS
	LogSpells(
		"DamageShieldType for spell [{}] ([{}]) is [{}]",
		spell_id,
		spell.name,
		spell.damage_shield_type
	);

	if (spell.damage_shield_type) {
		return static_cast<DmgShieldType>(spell.damage_shield_type);
	}

	switch (spells[spell_id].resist_type) {
		case RESIST_COLD:
			return DS_TORMENT;
		case RESIST_FIRE:
			return DS_BURN;
		case RESIST_DISEASE:
			return DS_DECAY;
		default:
			return DS_THORNS;
	}

	if (damage_shield_type) {
		return static_cast<DmgShieldType>(damage_shield_type);
	}

	return DS_THORNS;
}

bool IsLDoNObjectSpell(uint16 spell_id)
{
	return (
		IsEffectInSpell(spell_id, SE_AppraiseLDonChest) ||
		IsEffectInSpell(spell_id, SE_DisarmLDoNTrap) ||
		IsEffectInSpell(spell_id, SE_UnlockLDoNChest)
	);
}

int GetSpellResistType(uint16 spell_id)
{
	return spells[spell_id].resist_type;
}

int GetSpellTargetType(uint16 spell_id)
{
	return static_cast<int>(spells[spell_id].target_type);
}

bool IsHealOverTimeSpell(uint16 spell_id)
{
	if (
		(
			IsEffectInSpell(spell_id, SE_HealOverTime) ||
			GetSpellTriggerSpellID(spell_id, SE_HealOverTime)
		) &&
		!IsGroupSpell(spell_id)
	) {
		return true;
	}

	return false;
}

bool IsCompleteHealSpell(uint16 spell_id)
{
	if (
		(
			spell_id == SPELL_COMPLETE_HEAL ||
			IsEffectInSpell(spell_id, SE_CompleteHeal) ||
			IsPercentalHealSpell(spell_id) ||
			GetSpellTriggerSpellID(spell_id, SE_CompleteHeal)
		) &&
		!IsGroupSpell(spell_id)
	) {
		return true;
	}

	return false;
}

bool IsFastHealSpell(uint16 spell_id)
{
	spell_id = (
		IsEffectInSpell(spell_id, SE_CurrentHP) ?
		spell_id :
		GetSpellTriggerSpellID(spell_id, SE_CurrentHP)
	);

	if (!spell_id) {
		spell_id = (
			IsEffectInSpell(spell_id, SE_CurrentHPOnce) ?
			spell_id :
			GetSpellTriggerSpellID(spell_id, SE_CurrentHPOnce)
		);
	}

	if (spell_id) {
		if (
			spells[spell_id].cast_time <= MAX_FAST_HEAL_CASTING_TIME &&
			spells[spell_id].good_effect &&
			!IsGroupSpell(spell_id)
		) {
			for (int i = 0; i < EFFECT_COUNT; i++) {
				if (
					spells[spell_id].base_value[i] > 0 &&
					(
						spells[spell_id].effect_id[i] == SE_CurrentHP ||
						spells[spell_id].effect_id[i] == SE_CurrentHPOnce
					)
				) {
					return true;
				}
			}
		}
	}

	return false;
}

bool IsVeryFastHealSpell(uint16 spell_id)
{
	spell_id = (
		IsEffectInSpell(spell_id, SE_CurrentHP) ?
		spell_id :
		GetSpellTriggerSpellID(spell_id, SE_CurrentHP)
	);

	if (!spell_id) {
		spell_id = (
			IsEffectInSpell(spell_id, SE_CurrentHPOnce) ?
			spell_id :
			GetSpellTriggerSpellID(spell_id, SE_CurrentHPOnce)
		);
	}

	if (spell_id) {
		if (
			spells[spell_id].cast_time <= MAX_VERY_FAST_HEAL_CASTING_TIME &&
			spells[spell_id].good_effect &&
			!IsGroupSpell(spell_id)
		) {
			for (int i = 0; i < EFFECT_COUNT; i++) {
				if (
					spells[spell_id].base_value[i] > 0 &&
					(
						spells[spell_id].effect_id[i] == SE_CurrentHP ||
						spells[spell_id].effect_id[i] == SE_CurrentHPOnce
					)
				) {
					return true;
				}
			}
		}
	}

	return false;
}

bool IsRegularSingleTargetHealSpell(uint16 spell_id)
{
	spell_id = (
		IsEffectInSpell(spell_id, SE_CurrentHP) ?
		spell_id :
		GetSpellTriggerSpellID(spell_id, SE_CurrentHP)
	);

	if (!spell_id) {
		spell_id = (
			IsEffectInSpell(spell_id, SE_CurrentHPOnce) ?
			spell_id :
			GetSpellTriggerSpellID(spell_id, SE_CurrentHPOnce)
		);
	}

	if (spell_id) {
		if (
			spells[spell_id].target_type == ST_Target &&
			!IsCompleteHealSpell(spell_id) &&
			!IsHealOverTimeSpell(spell_id) &&
			!IsGroupSpell(spell_id)
		) {
			for (int i = 0; i < EFFECT_COUNT; i++) {
				if (
					spells[spell_id].base_value[i] > 0 &&
					spells[spell_id].buff_duration == 0 &&
					(
						spells[spell_id].effect_id[i] == SE_CurrentHP ||
						spells[spell_id].effect_id[i] == SE_CurrentHPOnce
					)
				) {
					return true;
				}
			}
		}
	}

	return false;
}

bool IsRegularGroupHealSpell(uint16 spell_id)
{
	spell_id = (
		IsEffectInSpell(spell_id, SE_CurrentHP) ?
		spell_id :
		GetSpellTriggerSpellID(spell_id, SE_CurrentHP)
	);

	if (!spell_id) {
		spell_id = (
			IsEffectInSpell(spell_id, SE_CurrentHPOnce) ?
			spell_id :
			GetSpellTriggerSpellID(spell_id, SE_CurrentHPOnce)
		);
	}

	if (spell_id) {
		if (
			IsGroupSpell(spell_id) &&
			!IsCompleteHealSpell(spell_id) &&
			!IsHealOverTimeSpell(spell_id)
		) {
			for (int i = 0; i < EFFECT_COUNT; i++) {
				if (
					spells[spell_id].base_value[i] > 0 &&
					spells[spell_id].buff_duration == 0 &&
					(
						spells[spell_id].effect_id[i] == SE_CurrentHP ||
						spells[spell_id].effect_id[i] == SE_CurrentHPOnce
					)
				) {
					return true;
				}
			}
		}
	}

	return false;
}

bool IsGroupCompleteHealSpell(uint16 spell_id)
{
	if (
		IsGroupSpell(spell_id) &&
		IsCompleteHealSpell(spell_id)
	) {
		return true;
	}

	return false;
}

bool IsGroupHealOverTimeSpell(uint16 spell_id)
{
	if (
		IsGroupSpell(spell_id) &&
		IsHealOverTimeSpell(spell_id) &&
		spells[spell_id].buff_duration < 10
	) {
		return true;
	}

	return false;
}

bool IsDebuffSpell(uint16 spell_id)
{
	if (
		IsBeneficialSpell(spell_id) ||
		IsHealthSpell(spell_id) ||
		IsStunSpell(spell_id) ||
		IsMesmerizeSpell(spell_id) ||
		IsCharmSpell(spell_id) ||
		IsSlowSpell(spell_id) ||
		IsEffectInSpell(spell_id, SE_Root) ||
		IsEffectInSpell(spell_id, SE_CancelMagic) ||
		IsEffectInSpell(spell_id, SE_MovementSpeed) ||
		IsFearSpell(spell_id) ||
		IsEffectInSpell(spell_id, SE_InstantHate)
	) {
		return false;
	}

	return true;
}

bool IsResistDebuffSpell(uint16 spell_id)
{
	if (
		!IsBeneficialSpell(spell_id) &&
		(
			IsEffectInSpell(spell_id, SE_ResistFire) ||
			IsEffectInSpell(spell_id, SE_ResistCold) ||
			IsEffectInSpell(spell_id, SE_ResistPoison) ||
			IsEffectInSpell(spell_id, SE_ResistDisease) ||
			IsEffectInSpell(spell_id, SE_ResistMagic) ||
			IsEffectInSpell(spell_id, SE_ResistAll) ||
			IsEffectInSpell(spell_id, SE_ResistCorruption)
		)
	) {
		return true;
	}

	return false;
}

bool IsSelfConversionSpell(uint16 spell_id)
{
	if (
		GetSpellTargetType(spell_id) == ST_Self &&
		IsEffectInSpell(spell_id, SE_CurrentMana) &&
		IsEffectInSpell(spell_id, SE_CurrentHP) &&
		spells[spell_id].base_value[GetSpellEffectIndex(spell_id, SE_CurrentMana)] > 0 &&
		spells[spell_id].base_value[GetSpellEffectIndex(spell_id, SE_CurrentHP)] < 0
	) {
		return true;
	}

	return false;
}

// returns true for both detrimental and beneficial buffs
bool IsBuffSpell(uint16 spell_id)
{
	if (
		IsValidSpell(spell_id) &&
		(
			spells[spell_id].buff_duration ||
			spells[spell_id].buff_duration_formula
		)
	) {
		return true;
	}

	return false;
}

bool IsPersistDeathSpell(uint16 spell_id)
{
	if (
		IsValidSpell(spell_id) &&
		spells[spell_id].persist_death
	) {
		return true;
	}

	return false;
}

bool IsSuspendableSpell(uint16 spell_id)
{
	if (
		IsValidSpell(spell_id) &&
		spells[spell_id].suspendable
	) {
		return true;
	}

	return false;
}

bool IsCastOnFadeDurationSpell(uint16 spell_id)
{
	for (int i = 0; i < EFFECT_COUNT; ++i) {
		if (
			spells[spell_id].effect_id[i] == SE_CastOnFadeEffect ||
			spells[spell_id].effect_id[i] == SE_CastOnFadeEffectNPC ||
			spells[spell_id].effect_id[i] == SE_CastOnFadeEffectAlways
		) {
			return true;
		}
	}

	return false;
}

bool IsDistanceModifierSpell(uint16 spell_id)
{
	if (
		IsValidSpell(spell_id) &&
		spells[spell_id].max_distance > spells[spell_id].min_distance &&
		(
			spells[spell_id].max_distance_mod ||
			spells[spell_id].min_distance_mod
		)
	) {
		return true;
	}

	return false;
}

int GetSpellPartialMeleeRuneReduction(uint16 spell_id)
{
	for (int i = 0; i < EFFECT_COUNT; ++i) {
		if (spells[spell_id].effect_id[i] == SE_MitigateMeleeDamage) {
			return spells[spell_id].base_value[i];
		}
	}

	return 0;
}

int GetSpellPartialMagicRuneReduction(uint16 spell_id)
{
	for (int i = 0; i < EFFECT_COUNT; ++i) {
		if (spells[spell_id].effect_id[i] == SE_MitigateSpellDamage) {
			return spells[spell_id].base_value[i];
		}
	}

	return 0;
}

int GetSpellPartialMeleeRuneAmount(uint16 spell_id)
{
	for (int i = 0; i < EFFECT_COUNT; ++i) {
		if (spells[spell_id].effect_id[i] == SE_MitigateMeleeDamage) {
			return spells[spell_id].max_value[i];
		}
	}

	return 0;
}

int GetSpellPartialMagicRuneAmount(uint16 spell_id)
{
	for (int i = 0; i < EFFECT_COUNT; ++i) {
		if (spells[spell_id].effect_id[i] == SE_MitigateSpellDamage) {
			return spells[spell_id].max_value[i];
		}
	}

	return 0;
}

bool IsRestAllowedSpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	return spells[spell_id].allow_rest;
}

bool IsNoDetrimentalSpellAggroSpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	return spells[spell_id].no_detrimental_spell_aggro;
}

bool IsStackableDOT(uint16 spell_id)
{
	// rules according to client
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	const auto &spell = spells[spell_id];

	if (
		spell.unstackable_dot ||
		spell.good_effect ||
		!spell.buff_duration_formula) {
		return false;
	}

	return (
		IsEffectInSpell(spell_id, SE_CurrentHP) ||
		IsEffectInSpell(spell_id, SE_GravityEffect)
	);
}

bool IsBardOnlyStackEffect(int effect_id)
{
	switch (effect_id) {
		case SE_BardAEDot:
			return true;
		default:
			return false;
	}
}

bool IsCastWhileInvisibleSpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	const auto &spell = spells[spell_id];

	if (
		spell.cast_not_standing ||
		spell.sneak
	) {
		return true;
	}

	return false;
}

bool IsEffectIgnoredInStacking(int effect_id)
{
	// this should match RoF2
	switch (effect_id) {
		case SE_SeeInvis:
		case SE_DiseaseCounter:
		case SE_PoisonCounter:
		case SE_Levitate:
		case SE_InfraVision:
		case SE_UltraVision:
		case SE_CurrentHPOnce:
		case SE_CurseCounter:
		case SE_ImprovedDamage:
		case SE_ImprovedHeal:
		case SE_SpellResistReduction:
		case SE_IncreaseSpellHaste:
		case SE_IncreaseSpellDuration:
		case SE_IncreaseRange:
		case SE_SpellHateMod:
		case SE_ReduceReagentCost:
		case SE_ReduceManaCost:
		case SE_FcStunTimeMod:
		case SE_LimitMaxLevel:
		case SE_LimitResist:
		case SE_LimitTarget:
		case SE_LimitEffect:
		case SE_LimitSpellType:
		case SE_LimitSpell:
		case SE_LimitMinDur:
		case SE_LimitInstant:
		case SE_LimitMinLevel:
		case SE_LimitCastTimeMin:
		case SE_LimitCastTimeMax:
		case SE_StackingCommand_Block:
		case SE_StackingCommand_Overwrite:
		case SE_PetPowerIncrease:
		case SE_SkillDamageAmount:
		case SE_ChannelChanceSpells:
		case SE_Blank:
		case SE_FcDamageAmt:
		case SE_SpellDurationIncByTic:
		case SE_FcSpellVulnerability:
		case SE_FcDamageAmtIncoming:
		case SE_FcDamagePctCrit:
		case SE_FcDamageAmtCrit:
		case SE_ReduceReuseTimer:
		case SE_LimitCombatSkills:
		case SE_BlockNextSpellFocus:
		case SE_SpellTrigger:
		case SE_LimitManaMin:
		case SE_CorruptionCounter:
		case SE_ApplyEffect:
		case SE_NegateSpellEffect:
		case SE_LimitSpellGroup:
		case SE_LimitManaMax:
		case SE_FcHealAmt:
		case SE_FcHealPctIncoming:
		case SE_FcHealAmtIncoming:
		case SE_FcHealPctCritIncoming:
		case SE_FcHealAmtCrit:
		case SE_LimitClass:
		case SE_LimitRace:
		case SE_FcBaseEffects:
		case SE_FFItemClass:
		case SE_SkillDamageAmount2:
		case SE_FcLimitUse:
		case SE_FcIncreaseNumHits:
		case SE_LimitUseMin:
		case SE_LimitUseType:
		case SE_GravityEffect:
		case SE_Display:
			//Spell effects implemented after ROF2, following same pattern, lets assume these should go here.
		case SE_Fc_Spell_Damage_Pct_IncomingPC:
		case SE_Fc_Spell_Damage_Amt_IncomingPC:
		case SE_Ff_CasterClass:
		case SE_Ff_Same_Caster:
		case SE_Proc_Timer_Modifier:
		case SE_Weapon_Stance:
		case SE_TwinCastBlocker:
		case SE_Fc_CastTimeAmt:
		case SE_Fc_CastTimeMod2:
		case SE_Ff_DurationMax:
		case SE_Ff_Endurance_Max:
		case SE_Ff_Endurance_Min:
		case SE_Ff_ReuseTimeMin:
		case SE_Ff_ReuseTimeMax:
		case SE_Ff_Value_Min:
		case SE_Ff_Value_Max:
		case SE_Ff_FocusTimerMin:
			return true;
		default:
			return false;
	}
}

bool IsFocusLimit(int effect_id)
{
	switch (effect_id) {
		case SE_LimitMaxLevel:
		case SE_LimitResist:
		case SE_LimitTarget:
		case SE_LimitEffect:
		case SE_LimitSpellType:
		case SE_LimitSpell:
		case SE_LimitMinDur:
		case SE_LimitInstant:
		case SE_LimitMinLevel:
		case SE_LimitCastTimeMin:
		case SE_LimitCastTimeMax:
		case SE_LimitCombatSkills:
		case SE_LimitManaMin:
		case SE_LimitSpellGroup:
		case SE_LimitManaMax:
		case SE_LimitSpellClass:
		case SE_LimitSpellSubclass:
		case SE_LimitClass:
		case SE_LimitRace:
		case SE_LimitCastingSkill:
		case SE_LimitUseMin:
		case SE_LimitUseType:
		case SE_Ff_Override_NotFocusable:
		case SE_Ff_CasterClass:
		case SE_Ff_Same_Caster:
		case SE_Ff_DurationMax:
		case SE_Ff_Endurance_Max:
		case SE_Ff_Endurance_Min:
		case SE_Ff_ReuseTimeMin:
		case SE_Ff_ReuseTimeMax:
		case SE_Ff_Value_Min:
		case SE_Ff_Value_Max:
		case SE_Ff_FocusTimerMin:
		case SE_FFItemClass:
			return true;
		default:
			return false;
	}
}

int GetSpellNimbusEffect(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return 0;
	}

	return spells[spell_id].nimbus_effect;
}

int GetSpellFuriousBash(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return 0;
	}

	bool found_effect_limit = false;
	auto mod                = 0;

	for (int i = 0; i < EFFECT_COUNT; ++i) {
		if (spells[spell_id].effect_id[i] == SE_SpellHateMod) {
			mod = spells[spell_id].base_value[i];
		} else if (
			spells[spell_id].effect_id[i] == SE_LimitEffect &&
			spells[spell_id].base_value[i] == 999
		) {
			found_effect_limit = true;
		}
	}

	if (found_effect_limit) {
		return mod;
	}

	return 0;
}

bool IsShortDurationBuff(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	return spells[spell_id].short_buff_box != 0;
}

bool IsSpellUsableInThisZoneType(uint16 spell_id, uint8 zone_type)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	//check if spell can be cast in any zone (-1 or 255), then if spell zonetype matches zone's zonetype
	// || spells[spell_id].zonetype == 255 comparing signed 8 bit int to 255 is always false
	if (
		spells[spell_id].zone_type == -1 ||
		spells[spell_id].zone_type == zone_type
	) {
		return true;
	}

	return false;
}

const char* GetSpellName(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return "INVALID SPELL ID IN GETSPELLNAME";
	}

	return spells[spell_id].name;
}

bool IsTargetRequiredForSpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	const auto& spell = spells[spell_id];

	if (
		spell.target_type == ST_AEClientV1 ||
		spell.target_type == ST_Self ||
		spell.target_type == ST_AECaster ||
		spell.target_type == ST_Ring ||
		spell.target_type == ST_Beam
	) {
		return false;
	}

	return true;
}

bool IsInstrumentModifierAppliedToSpellEffect(uint16 spell_id, int effect_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	//Effects that are verified modifiable by bard instrument/singing mods, or highly likely due to similiar type of effect.
	switch (effect_id) {
		//Only modify instant endurance or mana effects (Ie. Mana drain, Crescendo line)
		case SE_CurrentEndurance:
		case SE_CurrentMana: {
			return spells[spell_id].buff_duration == 0;
		}

		case SE_CurrentHP:
		case SE_ArmorClass:
		case SE_ACv2:
		case SE_MovementSpeed:
		case SE_ATK:
		case SE_STR:
		case SE_DEX:
		case SE_AGI:
		case SE_STA:
		case SE_INT:
		case SE_WIS:
		case SE_CHA:
		case SE_AllStats:
		case SE_ResistFire:
		case SE_ResistCold:
		case SE_ResistPoison:
		case SE_ResistDisease:
		case SE_ResistMagic:
		case SE_ResistAll:
		case SE_ResistCorruption:
		case SE_Rune:
		case SE_AbsorbMagicAtt:
		case SE_DamageShield:
		case SE_MitigateDamageShield:
		case SE_Amplification: //On live Amplification is modified by singing mods, including itself.
		case SE_TripleAttackChance:
		case SE_Flurry:
		case SE_DamageModifier:
		case SE_DamageModifier2:
		case SE_MinDamageModifier:
		case SE_ProcChance:
		case SE_PetFlurry: // ? Need verified
		case SE_DiseaseCounter:
		case SE_PoisonCounter:
		case SE_CurseCounter:
		case SE_CorruptionCounter:
			return true;

		/*
			Following are confirmed NOT modifiable by instrument/singing mods.
			Focus Effects, Proc Effects, Spell Triggers are not modified but handled elsewhere, not neccessary to checked here.
		*/

		case SE_AttackSpeed: //(Haste AND Slow not modifiable)
		case SE_AttackSpeed2:
		case SE_AttackSpeed3:
		case SE_Lull:
		case SE_ChangeFrenzyRad:
		case SE_Harmony:
		case SE_AddFaction:
		//case SE_CurrentMana: // duration only
		case SE_ManaRegen_v2:
		//case SE_CurrentEndurance: // duration only
		case SE_PersistentEffect:
		case SE_ReduceReuseTimer:
		case SE_Stun:
		case SE_Mez:
		case SE_WipeHateList: //?
		case SE_CancelMagic:
		case SE_ManaAbsorbPercentDamage:
		case SE_ResistSpellChance:
		case SE_Reflect:
		case SE_MitigateSpellDamage:
		case SE_MitigateMeleeDamage:
		case SE_AllInstrumentMod:
		case SE_AddSingingMod:
		case SE_SongModCap:
		case SE_BardSongRange:
		case SE_TemporaryPets:
		case SE_SpellOnDeath:
		case SE_Invisibility:
		case SE_Invisibility2:
		case SE_InvisVsUndead:
		case SE_InvisVsUndead2:
		case SE_InvisVsAnimals:
		case SE_ImprovedInvisAnimals:
		case SE_SeeInvis:
		case SE_Levitate:
		case SE_WaterBreathing:
		case SE_ModelSize:
		case SE_ChangeHeight:
		case SE_MakeDrunk:
			return false;
		default:
			return true;
	}
	//Allowing anything not confirmed to be restricted / allowed to receive modifiers, as to not inhbit anyone making custom bard songs.
}

bool IsPulsingBardSong(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	if (
		spells[spell_id].buff_duration == 0xFFFF ||
		spells[spell_id].recast_time > 0 ||
		spells[spell_id].mana > 0 ||
		IsEffectInSpell(spell_id, SE_TemporaryPets) ||
		IsEffectInSpell(spell_id, SE_Familiar)
	) {
		return false;
	}

	return true;
}

int GetSpellStatValue(uint16 spell_id, const char* stat_identifier, uint8 slot)
{
	if (!IsValidSpell(spell_id)) {
		return 0;
	}

	if (!stat_identifier) {
		return 0;
	}

	if (slot > 0) {
		slot -= 1;
	}

	const auto id = Strings::ToLower(stat_identifier);

	if (slot < 16) {
		if (id == "classes") { return spells[spell_id].classes[slot]; }
		else if (id == "deities") { return spells[spell_id].deities[slot]; }
	}

	if (slot < 12) {
		if (id == "base") { return spells[spell_id].base_value[slot]; }
		else if (id == "base2") { return spells[spell_id].limit_value[slot]; }
		else if (id == "max") { return spells[spell_id].max_value[slot]; }
		else if (id == "formula") { return spells[spell_id].formula[slot]; }
		else if (id == "effectid") { return spells[spell_id].effect_id[slot]; }
	}

	if (slot < 4) {
		if (id == "components") { return spells[spell_id].component[slot]; }
		else if (id == "component_counts") { return spells[spell_id].component_count[slot]; }
		else if (id == "noexpendreagent") { return spells[spell_id].no_expend_reagent[slot]; }
	}

	if (id == "range") { return static_cast<int32>(spells[spell_id].range); }
	else if (id == "aoe_range") { return static_cast<int32>(spells[spell_id].aoe_range); }
	else if (id == "push_back") { return static_cast<int32>(spells[spell_id].push_back); }
	else if (id == "push_up") { return static_cast<int32>(spells[spell_id].push_up); }
	else if (id == "cast_time") { return spells[spell_id].cast_time; }
	else if (id == "recovery_time") { return spells[spell_id].recovery_time; }
	else if (id == "recast_time") { return spells[spell_id].recast_time; }
	else if (id == "buff_duration_formula") { return spells[spell_id].buff_duration_formula; }
	else if (id == "buff_duration") { return spells[spell_id].buff_duration; }
	else if (id == "aeduration") { return spells[spell_id].aoe_duration; }
	else if (id == "mana") { return spells[spell_id].mana; }
	//else if (id == "LightType") {stat = spells[spell_id].LightType; } - Not implemented
	else if (id == "goodeffect") { return spells[spell_id].good_effect; }
	else if (id == "activated") { return spells[spell_id].activated; }
	else if (id == "resisttype") { return spells[spell_id].resist_type; }
	else if (id == "targettype") { return spells[spell_id].target_type; }
	else if (id == "basediff") { return spells[spell_id].base_difficulty; }
	else if (id == "skill") { return spells[spell_id].skill; }
	else if (id == "zonetype") { return spells[spell_id].zone_type; }
	else if (id == "environmenttype") { return spells[spell_id].environment_type; }
	else if (id == "timeofday") { return spells[spell_id].time_of_day; }
	else if (id == "castinganim") { return spells[spell_id].casting_animation; }
	else if (id == "spellaffectindex") { return spells[spell_id].spell_affect_index; }
	else if (id == "disallow_sit") { return spells[spell_id].disallow_sit; }
	//else if (id == "spellanim") {stat = spells[spell_id].spellanim; } - Not implemented
	else if (id == "uninterruptable") { return spells[spell_id].uninterruptable; }
	else if (id == "resistdiff") { return spells[spell_id].resist_difficulty; }
	else if (id == "dot_stacking_exempt") { return spells[spell_id].unstackable_dot; }
	else if (id == "recourselink") { return spells[spell_id].recourse_link; }
	else if (id == "no_partial_resist") { return spells[spell_id].no_partial_resist; }
	else if (id == "short_buff_box") { return spells[spell_id].short_buff_box; }
	else if (id == "descnum") { return spells[spell_id].description_id; }
	else if (id == "effectdescnum") { return spells[spell_id].effect_description_id; }
	else if (id == "npc_no_los") { return spells[spell_id].npc_no_los; }
	else if (id == "feedbackable") { return spells[spell_id].feedbackable; }
	else if (id == "reflectable") { return spells[spell_id].reflectable; }
	else if (id == "bonushate") { return spells[spell_id].bonus_hate; }
	else if (id == "endurcost") { return spells[spell_id].endurance_cost; }
	else if (id == "endurtimerindex") { return spells[spell_id].timer_id; }
	else if (id == "isdisciplinebuff") { return spells[spell_id].is_discipline; }
	else if (id == "hateadded") { return spells[spell_id].hate_added; }
	else if (id == "endurupkeep") { return spells[spell_id].endurance_upkeep; }
	else if (id == "numhitstype") { return spells[spell_id].hit_number_type; }
	else if (id == "numhits") { return spells[spell_id].hit_number; }
	else if (id == "pvpresistbase") { return spells[spell_id].pvp_resist_base; }
	else if (id == "pvpresistcalc") { return spells[spell_id].pvp_resist_per_level; }
	else if (id == "pvpresistcap") { return spells[spell_id].pvp_resist_cap; }
	else if (id == "spell_category") { return spells[spell_id].spell_category; }
	else if (id == "can_mgb") { return spells[spell_id].can_mgb; }
	else if (id == "dispel_flag") { return spells[spell_id].dispel_flag; }
	else if (id == "minresist") { return spells[spell_id].min_resist; }
	else if (id == "maxresist") { return spells[spell_id].max_resist; }
	else if (id == "viral_targets") { return spells[spell_id].viral_targets; }
	else if (id == "viral_timer") { return spells[spell_id].viral_timer; }
	else if (id == "nimbuseffect") { return spells[spell_id].nimbus_effect; }
	else if (id == "directional_start") { return static_cast<int32>(spells[spell_id].directional_start); }
	else if (id == "directional_end") { return static_cast<int32>(spells[spell_id].directional_end); }
	else if (id == "not_focusable") { return spells[spell_id].not_focusable; }
	else if (id == "suspendable") { return spells[spell_id].suspendable; }
	else if (id == "viral_range") { return spells[spell_id].viral_range; }
	else if (id == "spellgroup") { return spells[spell_id].spell_group; }
	else if (id == "rank") { return spells[spell_id].rank; }
	else if (id == "no_resist") { return spells[spell_id].no_resist; }
	else if (id == "castrestriction") { return spells[spell_id].cast_restriction; }
	else if (id == "allowrest") { return spells[spell_id].allow_rest; }
	else if (id == "incombat") { return spells[spell_id].can_cast_in_combat; }
	else if (id == "outofcombat") { return spells[spell_id].can_cast_out_of_combat; }
	else if (id == "aemaxtargets") { return spells[spell_id].aoe_max_targets; }
	else if (id == "no_heal_damage_item_mod") { return spells[spell_id].no_heal_damage_item_mod; }
	else if (id == "persistdeath") { return spells[spell_id].persist_death; }
	else if (id == "min_dist") { return static_cast<int32>(spells[spell_id].min_distance); }
	else if (id == "min_dist_mod") { return static_cast<int32>(spells[spell_id].min_distance_mod); }
	else if (id == "max_dist") { return static_cast<int32>(spells[spell_id].max_distance); }
	else if (id == "min_range") { return static_cast<int32>(spells[spell_id].min_range); }
	else if (id == "damageshieldtype") { return spells[spell_id].damage_shield_type; }

	return 0;
}

bool IsVirusSpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	return (
		GetSpellViralMinimumSpreadTime(spell_id) &&
		GetSpellViralMaximumSpreadTime(spell_id) &&
		GetSpellViralSpreadRange(spell_id)
	);
}

int GetSpellViralMinimumSpreadTime(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return 0;
	}

	return spells[spell_id].viral_targets;
}

int GetSpellViralMaximumSpreadTime(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return 0;
	}

	return spells[spell_id].viral_timer;
}

int GetSpellViralSpreadRange(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return 0;
	}

	return spells[spell_id].viral_range;
}

int GetSpellProcLimitTimer(uint16 spell_id, int proc_type)
{
	if (!IsValidSpell(spell_id)) {
		return 0;
	}

	//This allows for support for effects that may have multiple different proc types and timers.
	auto use_next_timer = false;

	for (int i = 0; i < EFFECT_COUNT; ++i) {

		if (proc_type == ProcType::MELEE_PROC) {
			if (
				spells[spell_id].effect_id[i] == SE_WeaponProc ||
				spells[spell_id].effect_id[i] == SE_AddMeleeProc
			) {
				use_next_timer = true;
			}
		}

		if (proc_type == ProcType::RANGED_PROC) {
			if (spells[spell_id].effect_id[i] == SE_RangedProc) {
				use_next_timer = true;
			}
		}

		if (proc_type == ProcType::DEFENSIVE_PROC) {
			if (spells[spell_id].effect_id[i] == SE_DefensiveProc) {
				use_next_timer = true;
			}
		}

		if (
			use_next_timer &&
			spells[spell_id].effect_id[i] == SE_Proc_Timer_Modifier
		) {
			return spells[spell_id].limit_value[i];
		}
	}

	return 0;
}

bool IsCastRestrictedSpell(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	switch (spell_id) {
		case SPELL_TOUCH_OF_VINITRAS:
		case SPELL_DESPERATE_HOPE:
		case SPELL_CHARM:
		case SPELL_METAMORPHOSIS65:
		case SPELL_JT_BUFF:
		case SPELL_CAN_O_WHOOP_ASS:
		case SPELL_PHOENIX_CHARM:
		case SPELL_CAZIC_TOUCH:
		case SPELL_AVATAR_KNOCKBACK:
		case SPELL_SHAPECHANGE65:
		case SPELL_SUNSET_HOME1218:
		case SPELL_SUNSET_HOME819:
		case SPELL_SHAPECHANGE75:
		case SPELL_SHAPECHANGE80:
		case SPELL_SHAPECHANGE85:
		case SPELL_SHAPECHANGE90:
		case SPELL_SHAPECHANGE95:
		case SPELL_SHAPECHANGE100:
		case SPELL_SHAPECHANGE25:
		case SPELL_SHAPECHANGE30:
		case SPELL_SHAPECHANGE35:
		case SPELL_SHAPECHANGE40:
		case SPELL_SHAPECHANGE45:
		case SPELL_SHAPECHANGE50:
		case SPELL_NPC_AEGOLISM:
		case SPELL_SHAPECHANGE55:
		case SPELL_SHAPECHANGE60:
		case SPELL_COMMAND_OF_DRUZZIL:
		case SPELL_SHAPECHANGE70:
		case SPELL_GMHP25K:
		case SPELL_GMHP50K:
		case SPELL_GMHP100K:
		case SPELL_GMHP225K:
		case SPELL_GMHP475K:
		case SPELL_GMHP925K:
		case SPELL_GMHP2M:
		case SPELL_GMHP3M:
		case SPELL_GMHP5M:
		case SPELL_GUIDE_ACTING_ONE:
		case SPELL_GUIDE_ALLIANCE_ONE:
		case SPELL_GUIDE_CANCEL_MAGIC_ONE:
		case SPELL_GUIDE_JOURNEY_ONE:
		case SPELL_GUIDE_VISION_ONE:
		case SPELL_GUIDE_HEALTH_ONE:
		case SPELL_GUIDE_INVULNERABILITY_ONE:
		case SPELL_GUIDE_BOLT_ONE:
		case SPELL_GUIDE_MEMORY_BLUR_ONE:
		case SPELL_GUIDE_ACTING_TWO:
		case SPELL_GUIDE_CANCEL_MAGIC_TWO:
		case SPELL_GUIDE_JOURNEY_TWO:
		case SPELL_GUIDE_VISION_TWO:
		case SPELL_GUIDE_HEALTH_TWO:
		case SPELL_GUIDE_INVULNERABILITY_TWO:
		case SPELL_GUIDE_BOLT_TWO:
		case SPELL_GUIDE_MEMORY_BLUR_TWO:
		case SPELL_GUIDE_ALLIANCE_TWO:
		case SPELL_GUIDE_EVACUATION:
		case SPELL_GUIDE_LEVITATION:
		case SPELL_GUIDE_SPELL_HASTE:
		case SPELL_GUIDE_HASTE:
			return true;
		default:
			return false;
	}
}

bool IsCastNotStandingSpell(uint16 spell_id) {
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	/*
		field 'cast_not_standing' allows casting when sitting, stunned, mezed, Divine Aura, through SPA 343 Interrupt casting
		Likely also allows for casting while feared, but need to confirm. Possibly also while charmed.
		This field also allows for damage to ignore DA immunity.
	*/
	return spells[spell_id].cast_not_standing;
}
