/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemulator.net)

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

#include "../common/rulesys.h"
#include "../common/strings.h"

#include "client.h"
#include "entity.h"
#include "mob.h"
#include "string_ids.h"
#include "lua_parser.h"

#include <string.h>

extern double frame_time;

int Mob::GetBaseSkillDamage(uint16 skill_id, Mob* m)
{
	int          base        = Skill::GetBaseDamage(skill_id);
	const uint16 skill_level = GetSkill(skill_id);
	switch (skill_id) {
		case Skill::DragonPunch:
		case Skill::EagleStrike:
		case Skill::TigerClaw:
			if (skill_level >= 25) {
				base++;
			}

			if (skill_level >= 75) {
				base++;
			}

			if (skill_level >= 125) {
				base++;
			}

			if (skill_level >= 175) {
				base++;
			}

			if (RuleB(Character, ItemExtraSkillDamageCalcAsPercent) && GetSkillDmgAmt(skill_id) > 0) {
				base *= std::abs(GetSkillDmgAmt(skill_id) / 100);
			}

			return base;
		case Skill::Frenzy:
			if (IsClient() && CastToClient()->GetInv().GetItem(EQ::invslot::slotPrimary)) {
				if (GetLevel() > 15) {
					base += GetLevel() - 15;
				}

				if (base > 23) {
					base = 23;
				}

				if (GetLevel() > 50) {
					base += 2;
				}

				if (GetLevel() > 54) {
					base++;
				}

				if (GetLevel() > 59) {
					base++;
				}
			}

			if (RuleB(Character, ItemExtraSkillDamageCalcAsPercent) && GetSkillDmgAmt(skill_id) > 0) {
				base *= std::abs(GetSkillDmgAmt(skill_id) / 100);
			}

			return base;
		case Skill::FlyingKick: {
			float skill_bonus = skill_level / 9.0f;
			float ac_bonus    = 0.0f;
			if (IsClient()) {
				const auto* inst = CastToClient()->GetInv().GetItem(EQ::invslot::slotFeet);
				if (inst) {
					ac_bonus = inst->GetItemArmorClass(true) / 25.0f;
				}
			}

			if (ac_bonus > skill_bonus) {
				ac_bonus = skill_bonus;
			}

			if (RuleB(Character, ItemExtraSkillDamageCalcAsPercent) && GetSkillDmgAmt(skill_id) > 0) {
				return (
					static_cast<int>(ac_bonus + skill_bonus) *
					std::abs(GetSkillDmgAmt(skill_id) / 100)
				);
			}

			return static_cast<int>(ac_bonus + skill_bonus);
		}
		case Skill::Kick:
		case Skill::RoundKick: {
			// there is some base *= 4 case in here?
			float skill_bonus = skill_level / 10.0f;
			float ac_bonus    = 0.0f;
			if (IsClient()) {
				const auto* inst = CastToClient()->GetInv().GetItem(EQ::invslot::slotFeet);
				if (inst) {
					ac_bonus = inst->GetItemArmorClass(true) / 25.0f;
				}
			}

			if (ac_bonus > skill_bonus) {
				ac_bonus = skill_bonus;
			}

			if (skill_level >= 175) {
				base++;
			}

			if (RuleB(Character, ItemExtraSkillDamageCalcAsPercent) && GetSkillDmgAmt(skill_id) > 0) {
				return (
					static_cast<int>(ac_bonus + skill_bonus) *
					std::abs(GetSkillDmgAmt(skill_id) / 100)
				);
			}

			return static_cast<int>(ac_bonus + skill_bonus);
		}
		case Skill::Bash: {
			float skill_bonus = skill_level / 10.0f;
			float ac_bonus    = 0.0f;

			const EQ::ItemInstance* inst = nullptr;
			if (IsClient()) {
				if (HasShieldEquipped()) {
					inst = CastToClient()->GetInv().GetItem(EQ::invslot::slotSecondary);
				} else if (HasTwoHanderEquipped()) {
					inst = CastToClient()->GetInv().GetItem(EQ::invslot::slotPrimary);
				}
			}

			if (inst) {
				ac_bonus = inst->GetItemArmorClass(true) / 25.0f;
			} else {
				return 0;
			}

			if (ac_bonus > skill_bonus) {
				ac_bonus = skill_bonus;
			}

			if (RuleB(Character, ItemExtraSkillDamageCalcAsPercent) && GetSkillDmgAmt(skill_id) > 0) {
				return (
					static_cast<int>(ac_bonus + skill_bonus) *
					std::abs(GetSkillDmgAmt(skill_id) / 100)
				);
			}

			return static_cast<int>(ac_bonus + skill_bonus);
		}
		case Skill::Backstab: {
			float skill_bonus = static_cast<float>(skill_level) * 0.02f;
			base = 3;
			if (IsClient()) {
				const auto* inst = CastToClient()->GetInv().GetItem(EQ::invslot::slotPrimary);
				if (
					inst &&
					inst->GetItem() &&
					inst->GetItem()->ItemType == EQ::item::ItemType1HPiercing
				) {
					base = inst->GetItemBackstabDamage(true);
					if (!inst->GetItemBackstabDamage()) {
						base += inst->GetItemWeaponDamage(true);
					}

					if (m) {
						if (
							inst->GetItemElementalFlag(true) &&
							inst->GetItemElementalDamage(true) &&
							!RuleB(Combat, BackstabIgnoresElemental)
						) {
							base += m->ResistElementalWeaponDmg(inst);
						}

						if (
							(inst->GetItemBaneDamageBody(true) || inst->GetItemBaneDamageRace(true)) &&
							!RuleB(Combat, BackstabIgnoresBane)
						) {
							base += m->CheckBaneDamage(inst);
						}
					}
				}
			} else if (IsNPC()) {
				NPC* n = CastToNPC();
				base = round((n->GetMaxDMG() - n->GetMinDMG()) / RuleR(NPC, NPCBackstabMod));
				// parses show relatively low BS mods from lots of NPCs, so either their BS skill is super low
				// or their mod is divided again, this is probably not the right mod, but it's better
				skill_bonus /= 3.0f;
			}

			if (RuleB(Character, ItemExtraSkillDamageCalcAsPercent) && GetSkillDmgAmt(skill_id) > 0) {
				return (
					static_cast<int>(static_cast<float>(base) *
					(skill_bonus + 2.0f)) *
					std::abs(GetSkillDmgAmt(skill_id) / 100)
				);
			}

			return static_cast<int>(static_cast<float>(base) * (skill_bonus + 2.0f));
		}
		default: {
			return 0;
		}
	}
}

void Mob::DoSpecialAttackDamage(
	Mob* m,
	uint16 skill_id,
	int32 base_damage,
	int32 min_damage,
	int32 hate_override,
	int reuse
)
{
	// this really should go through the same code as normal melee damage to
	// pick up all the special behavior there
	if (
		HasDied() ||
		!m ||
		!IsAttackAllowed(m) ||
		(
			(IsClient() && CastToClient()->dead) ||
			(m->IsClient() && m->CastToClient()->dead)
		)
	) {
		return;
	}

	DamageHitInfo my_hit;

	my_hit.damage_done = 1;
	my_hit.base_damage = base_damage;
	my_hit.min_damage  = min_damage;
	my_hit.skill_id    = skill_id;

	if (my_hit.base_damage == Damage::None) {
		my_hit.base_damage = GetBaseSkillDamage(my_hit.skill_id);
	}

	if (base_damage == Damage::Invulnerable) {
		my_hit.damage_done = Damage::Invulnerable;
	}

	if (m->GetInvul() || m->GetSpecialAbility(IMMUNE_MELEE)) {
		my_hit.damage_done = Damage::Invulnerable;
	}

	if (m->GetSpecialAbility(IMMUNE_MELEE_EXCEPT_BANE) && skill_id != Skill::Backstab) {
		my_hit.damage_done = Damage::Invulnerable;
	}

	int64 hate = hate_override > -1 ? hate_override : my_hit.base_damage;

	if (skill_id == Skill::Bash) {
		if (IsClient()) {
			const auto* inst = CastToClient()->GetInv().GetItem(EQ::invslot::slotSecondary);
			if (inst) {
				if (inst->GetItem()->ItemType == EQ::item::ItemTypeShield) {
					hate += inst->GetItem()->AC;
				}

				const auto* item = inst->GetItem();
				const int furious_bash = GetSpellFuriousBash(item->Focus.Effect);
				hate = hate * (100 + furious_bash) / 100;
				if (furious_bash) {
					MessageString(Chat::FocusEffect, GLOWS_RED, item->Name);
				}
			}
		}
	}

	my_hit.offense = offense(my_hit.skill_id);
	my_hit.tohit   = GetTotalToHit(my_hit.skill_id, 0);

	// Rogue Backstab Haste Correction
	// Haste should only provide a max of a 2 s reduction to Backstab cooldown, but it seems that while Skill::Reuse::Backstabr can be reduced, there is another timer (repop on the button)
	// that is controlling the actual cooldown.  I'm not sure how this is implemented, but it is impacted by spell haste (including bard v2 and v3), but not worn haste.
	// This code applies an adjustment to backstab accuracy to compensate for this so that Rogue DPS doesn't significantly outclass other classes.

	if (
		RuleB(Combat, RogueBackstabHasteCorrection) &&
		skill_id == Skill::Backstab &&
		GetHaste() > 100
	) {
		int haste_spell = (
			spellbonuses.haste -
			spellbonuses.inhibitmelee +
			spellbonuses.hastetype2 +
			spellbonuses.hastetype3
		);
		int haste_worn = itembonuses.haste;

		// Compute Intended Cooldown.  100% Spell = 1 s reduction (max), 40% Worn = 1 s reduction (max).
		int   reduction_intended_spell = haste_spell > 100 ? 100 : haste_spell;
		int   reduction_intended_worn  = 2.5 * (haste_worn > 40 ? 40 : haste_worn);
		int16 intended_cooldown        = 1000 - reduction_intended_spell - reduction_intended_worn;

		// Compute Actual Cooldown.  Actual only impacted by spell haste ( + v2 + v3), and is 10 s / (100 + haste)
		int actual_cooldown = 100000 / (100 + haste_spell);

		// Compute Accuracy Adjustment
		int backstab_accuracy_adjust = actual_cooldown * 1000 / intended_cooldown;

		// orig_accuracy = my_hit.tohit;
		int adjusted_accuracy = my_hit.tohit * backstab_accuracy_adjust / 1000;

		my_hit.tohit = adjusted_accuracy;
	}

	my_hit.hand = EQ::invslot::slotPrimary; // Avoid checks hand for throwing/archery exclusion, primary should work for most
	if (skill_id == Skill::Throwing || skill_id == Skill::Archery) {
		my_hit.hand = EQ::invslot::slotRange;

	DoAttack(m, my_hit);

	m->AddToHateList(this, hate, 0);
	m->Damage(this, my_hit.damage_done, SPELL_UNKNOWN, skill_id, false);

	// Make sure 'this' has not killed the target and 'this' is not dead (Damage shield ect).
	if (!GetTarget())
		return;
	if (HasDied())
		return;

	TryCastOnSkillUse(m, skill_id);

	if (HasSkillProcs()) {
		TrySkillProc(m, skill_id, reuse * 1000);
	}

	if (my_hit.damage_done > Damage::None && HasSkillProcSuccess()) {
		TrySkillProc(m, skill_id, reuse * 1000, true);
	}
}

// We should probably refactor this to take the struct not the packet
void Client::OPCombatAbility(const CombatAbility_Struct *ca_atk)
{
	if (!GetTarget())
		return;
	// make sure were actually able to use such an attack. (Bards can throw while casting. ~Kayen confirmed on live 1/22)
	if ((spellend_timer.Enabled() && GetClass() != BARD)|| IsFeared() || IsStunned() || IsMezzed() || DivineAura() || dead)
		return;

	pTimerType timer = pTimerCombatAbility;
	// RoF2+ Tiger Claw is unlinked from other monk skills, if they ever do that for other classes there will need
	// to be more checks here
	if (ClientVersion() >= EQ::versions::ClientVersion::RoF2 && ca_atk->m_skill == Skill::TigerClaw) {
		timer = pTimerCombatAbility2;

	bool CanBypassSkillCheck = false;

	if (ca_atk->m_skill == Skill::Bash) { // SLAM - Bash without a shield equipped
		switch (GetRace()) {
			case OGRE:
			case TROLL:
			case BARBARIAN:
				bypass_skill_check = true;
			default:
				break;
		}
	}

	// Check to see if actually have skill
	if (!MaxSkill(ca_atk->m_skill) && !bypass_skill_check) {
		return;

	if (GetTarget()->GetID() != ca_atk->m_target)
		return; // invalid packet.

	if (!IsAttackAllowed(GetTarget()))
		return;

	// These two are not subject to the combat ability timer, as they
	// allready do their checking in conjunction with the attack timer
	// throwing weapons
	if (ca_atk->m_atk == EQ::invslot::slotRange) {
		if (ca_atk->m_skill == Skill::Throwing) {
			SetAttackTimer();
			ThrowingAttack(GetTarget());
			if (CheckDoubleRangedAttack())
				ThrowingAttack(GetTarget(), true);
			return;
		}
		// ranged attack (archery)
		if (ca_atk->m_skill == Skill::Archery) {
			SetAttackTimer();
			RangedAttack(GetTarget());
			if (CheckDoubleRangedAttack())
				RangedAttack(GetTarget(), true);
			return;
		}
		// could we return here? Im not sure is m_atk 11 is used for real specials
	}

	// check range for all these abilities, they are all close combat stuff
	if (!CombatRange(GetTarget()))
		return;

	if (!p_timers.Expired(&database, timer, false)) {
		Message(Chat::Red, "Ability recovery time not yet met.");
		return;
	}

	int reuse          = 0;
	int haste          = GetHaste();
	int haste_modifier = 0;

	if (ClientHaste >= 0)
		HasteMod = (10000 / (100 + ClientHaste)); //+100% haste = 2x as many attacks
	else
		HasteMod = (100 - ClientHaste); //-100% haste = 1/2 as many attacks

	int64 damage          = 0;
	int   skill_reduction = GetSkillReuseTime(ca_atk->m_skill);

	// not sure what the '100' indicates, if ->m_atk is not used as 'slot' reference, then change SlotRange above back to '11'
	if (
		ca_atk->m_atk == 100 &&
		ca_atk->m_skill == Skill::Bash
	) { // SLAM - Bash without a shield equipped
		if (GetTarget() != this) {
			CheckIncreaseSkill(Skill::Bash, GetTarget(), 10);
			DoAnim(AttackAnimation::TailRake, 0, false);

			int32 ht = 0;
			if (GetWeaponDamage(GetTarget(), GetInv().GetItem(EQ::invslot::slotSecondary)) <= 0 &&
			    GetWeaponDamage(GetTarget(), GetInv().GetItem(EQ::invslot::slotShoulders)) <= 0)
				dmg = -5;
			else
				ht = dmg = GetBaseSkillDamage(EQ::skills::SkillBash, GetTarget());

			if (
				GetWeaponDamage(GetTarget(), GetInv().GetItem(EQ::invslot::slotSecondary)) <= 0 &&
				GetWeaponDamage(GetTarget(), GetInv().GetItem(EQ::invslot::slotShoulders)) <= 0
			) {
				damage = Damage::Invulnerable;
			} else {
				hate_override = damage = GetBaseSkillDamage(Skill::Bash, GetTarget());
			}

			reuse = Skill::Reuse::Bash - 1 - skill_reduction;
			reuse = (reuse * haste_modifier) / 100;
			DoSpecialAttackDamage(GetTarget(), Skill::Bash, damage, 0, hate_override, reuse);

			if (reuse) {
				p_timers.Start(timer, reuse);
			}
		}

		return;
	}

	if (ca_atk->m_atk == 100 && ca_atk->m_skill == Skill::Frenzy) {
		int attack_rounds = 1;
		int max_dmg       = GetBaseSkillDamage(Skill::Frenzy, GetTarget());

		CheckIncreaseSkill(Skill::Frenzy, GetTarget(), 10);
		DoAnim(AttackAnimation::OneHandWeapon, 0, false);

		if (GetClass() == Class::Berserker) {
			int chance = GetLevel() * 2 + GetSkill(Skill::Frenzy);

			if (zone->random.Roll0(450) < chance) {
				attack_rounds++;
			}

		ReuseTime = FrenzyReuseTime - 1 - skill_reduction;
		ReuseTime = (ReuseTime * HasteMod) / 100;

		reuse = Skill::Reuse::Frenzy - 1 - skill_reduction;
		reuse = (reuse * haste_modifier) / 100;

		const EQ::ItemInstance* primary_in_use = GetInv().GetItem(EQ::invslot::slotPrimary);
		if (primary_in_use && GetWeaponDamage(GetTarget(), primary_in_use) <= 0) {
			max_dmg = Damage::Invulnerable;
		}

		while (attack_rounds > 0) {
			if (GetTarget()) {
				DoSpecialAttackDamage(GetTarget(), Skill::Frenzy, max_dmg, 0, max_dmg, reuse);
			}

			attack_rounds--;
		}

		if (reuse) {
			p_timers.Start(timer, reuse);
		}

		return;
	}

	const uint8 class_id = GetClass();

	// Warrior, Ranger, Monk, Beastlord, and Berserker can kick always
	const uint32 allowed_kick_classes = RuleI(Combat, ExtraAllowedKickClassesBitmask);

	const bool can_kick = (
		class_id == Class::Warrior ||
		class_id == Class::Ranger ||
		class_id == Class::Monk ||
		class_id == Class::Beastlord ||
		class_id == Class::Berserker ||
		allowed_kick_classes & GetPlayerClassBit(class_id)
	);

	bool found_skill = false;

	if (
		ca_atk->m_atk == 100 &&
		ca_atk->m_skill == Skill::Kick &&
			can_kick
	) {
		if (GetTarget() != this) {
			CheckIncreaseSkill(Skill::Kick, GetTarget(), 10);
			DoAnim(AttackAnimation::Kick, 0, false);

			int hate_override = 0;
			if (GetWeaponDamage(GetTarget(), GetInv().GetItem(EQ::invslot::slotFeet)) <= 0) {
				damage = Damage::Invulnerable;
			} else {
				hate_override = damage = GetBaseSkillDamage(Skill::Kick, GetTarget());
			}

			reuse = Skill::Reuse::Kick - 1 - skill_reduction;
			DoSpecialAttackDamage(GetTarget(), Skill::Kick, damage, 0, hate_override, reuse);

			found_skill = true;
		}
		break;
	case MONK: {
		ReuseTime = MonkSpecialAttack(GetTarget(), ca_atk->m_skill) - 1 - skill_reduction;

	if (class_id == Class::Monk) {
		reuse = MonkSpecialAttack(GetTarget(), ca_atk->m_skill) - 1 - skill_reduction;

		// Live AA - Technique of Master Wu
		int wuchance = itembonuses.DoubleSpecialAttack + spellbonuses.DoubleSpecialAttack + aabonuses.DoubleSpecialAttack;

		if (wu_chance) {
			int extra = 0;

			// always 1/4 of the double attack chance, 25% at rank 5 (100/4)
			while (wuchance > 0) {
				if (zone->random.Roll(wuchance)) {
					++extra;
				}
				else {
					break;
				}
				wuchance /= 4;
			}
			if (extra) {
				SendColoredText(
					Chat::MasterWu,
					fmt::format(
						"The spirit of Master Wu fills you! You gain {} additional attack{}.",
						extra,
						extra != 1 ? "s" : ""
					)
				);
			}

			const bool is_classic = RuleB(Combat, ClassicMasterWu);
			while (extra) {
				MonkSpecialAttack(
					GetTarget(),
					(is_classic ? monk_skills[zone->random.Int(0, 4)] : ca_atk->m_skill)
				);
				--extra;
			}
		}

		if (reuse < 100) {
			// hackish... but we return a huge reuse time if this is an
			// invalid skill, otherwise, we can safely assume it is a
			// valid monk skill and just cast it to a uint16
			CheckIncreaseSkill((uint16) ca_atk->m_skill, GetTarget(), 10);
		}
		break;
	}

	if (
		ca_atk->m_atk == 100 &&
		ca_atk->m_skill == Skill::Backstab &&
		class_id == Class::Rogue
	) {
		reuse = Skill::Reuse::Backstab - 1 - skill_reduction;
		TryBackstab(GetTarget(), reuse);
		found_skill = true;
	}

	if (!found_skill) {
		reuse = 9 - skill_reduction;
	}

	reuse = (reuse * haste_modifier) / 100;

	reuse = EQ::Clamp(reuse, 0, reuse);

	if (reuse) {
		p_timers.Start(timer, reuse);
	}
}

//returns the reuse time in sec for the special attack used.
int Mob::MonkSpecialAttack(Mob* m, uint8 skill_id)
{
	if (!m) {
		return 0;
	}

	int32  max_damage = Damage::None;
	int32  min_damage = Damage::None;
	int    reuse      = 0;
	uint8  slot_id    = EQ::invslot::slotFeet;
	uint16 skill_to_use;

	if (IsNPC()) {
		NPC* n = CastToNPC();
		min_damage = n->GetMinDamage();
	}

	switch (skill_id) {
		case Skill::DragonPunch:
			skill_to_use = Skill::DragonPunch;
			max_damage   = GetBaseSkillDamage(skill_to_use);
			slot_id      = EQ::invslot::slotHands;

			DoAnim(AttackAnimation::TailRake, 0, false);

			reuse = Skill::Reuse::TailRake;
			break;
		case Skill::EagleStrike:
			skill_to_use = Skill::EagleStrike;
			max_damage   = GetBaseSkillDamage(skill_to_use);
			slot_id      = EQ::invslot::slotHands;

			DoAnim(AttackAnimation::EagleStrike, 0, false);

			reuse = Skill::Reuse::EagleStrike;
			break;
		case Skill::FlyingKick:
			skill_to_use = Skill::FlyingKick;
			max_damage   = GetBaseSkillDamage(skill_to_use);
			min_damage   = 0;

			DoAnim(AttackAnimation::FlyingKick, 0, false);

			reuse = Skill::Reuse::FlyingKick;
			break;
		case Skill::Kick:
			skill_to_use = Skill::Kick;
			max_damage   = GetBaseSkillDamage(skill_to_use);

			DoAnim(AttackAnimation::Kick, 0, false);

			reuse = Skill::Reuse::Kick;
			break;
		case Skill::RoundKick:
			skill_to_use = Skill::RoundKick;
			max_damage   = GetBaseSkillDamage(skill_to_use);

			DoAnim(AttackAnimation::RoundKick, 0, false);

			reuse = Skill::Reuse::RoundKick;
			break;
		case Skill::TigerClaw:
			skill_to_use = Skill::TigerClaw;
			max_damage   = GetBaseSkillDamage(skill_to_use);
			slot_id      = EQ::invslot::slotHands;

			DoAnim(AttackAnimation::TigerClaw, 0, false);

			reuse = Skill::Reuse::TigerClaw;
			break;
		default:
			LogAttackDetail("Invalid special attack type {} attempted.", skill_id);
			return 1000;
	}

	if (IsClient()) {
		if (GetWeaponDamage(m, CastToClient()->GetInv().GetItem(slot_id)) <= 0) {
			max_damage = Damage::Invulnerable;
		}
	} else {
		if (GetWeaponDamage(m, (const EQ::ItemData*)nullptr) <= 0) {
			max_damage = Damage::Invulnerable;
		}
	}

	int32 hate = 0;
	if (max_damage > Damage::None) {
		hate = max_damage;
	}

	// aggro should never be negative else it does massive aggro
	if (hate < 0) {
		hate = 0;
	}

	DoSpecialAttackDamage(m, skill_to_use, max_damage, min_damage, hate, reuse);

	return reuse;
}

void Mob::TryBackstab(Mob* m, int reuse)
{
	if (!m) {
		return;
	}

	bool is_behind            = false;
	bool can_frontal_backstab = false;

	//make sure we have a proper weapon if we are a client.
	if (IsClient()) {
		const auto* weapon = CastToClient()->GetInv().GetItem(EQ::invslot::slotPrimary);
		if (!weapon || (weapon->GetItem()->ItemType != EQ::item::ItemType1HPiercing)) {
			MessageString(Chat::Red, BACKSTAB_WEAPON);
			return;
		}
	}

	//Live AA - Triple Backstab
	int triple_backstab_chance = (
		itembonuses.TripleBackstab +
		spellbonuses.TripleBackstab +
		aabonuses.TripleBackstab
	);

	if (BehindMob(m, GetX(), GetY())) {
		is_behind = true;
	} else {
		//Live AA - Seized Opportunity
		int frontal_backstab_chance = (
			itembonuses.FrontalBackstabChance +
			spellbonuses.FrontalBackstabChance +
			aabonuses.FrontalBackstabChance
		);

		if (frontal_backstab_chance && zone->random.Roll(frontal_backstab_chance)) {
			can_frontal_backstab = true;
		}
	}

	if (
		is_behind ||
		can_frontal_backstab ||
		(IsNPC() && CanFacestab())
	) { // Player is behind other OR can do Frontal Backstab
		if (can_frontal_backstab && IsClient()) { // I don't think there is any message ...
			CastToClient()->Message(
				Chat::White,
				"Your fierce attack is executed with such grace, your target did not see it coming!"
			);
		}

		RogueBackstab(m, false, reuse);
		if (level >= RuleI(Combat, DoubleBackstabLevelRequirement)) {
			// TODO: 55-59 doesn't appear to match just checking double attack, 60+ does though
			if (IsClient() && CastToClient()->CheckDoubleAttack()) {
				if (m->GetHP() > 0) {
					RogueBackstab(m, false, reuse);
				}

				if (triple_backstab_chance && m->GetHP() > 0 && zone->random.Roll(triple_backstab_chance)) {
					RogueBackstab(m, false, reuse);
				}
			}
		}

		if (IsClient()) {
			CastToClient()->CheckIncreaseSkill(Skill::Backstab, m, 10);
		}
	} else if (
		aabonuses.FrontalBackstabMinDmg ||
		itembonuses.FrontalBackstabMinDmg ||
		spellbonuses.FrontalBackstabMinDmg
	) {
		//Live AA - Chaotic Backstab
		m_specialattacks = eSpecialAttacks::ChaoticStab;

		//we can stab from any angle, we do min damage though.
		// chaotic backstab can't double etc Seized can, but that's because it's a chance to do normal BS
		// Live actually added SPA 473 which grants chance to double here when they revamped chaotic/seized
		RogueBackstab(m, true, reuse);

		if (IsClient()) {
			CastToClient()->CheckIncreaseSkill(Skill::Backstab, m, 10);
		}

		m_specialattacks = eSpecialAttacks::None;

		int double_front_backstab_chance = (
			aabonuses.Double_Backstab_Front +
			itembonuses.Double_Backstab_Front +
			spellbonuses.Double_Backstab_Front
		);
		if (double_front_backstab_chance && m->GetHP() > 0 && zone->random.Roll(double_front_backstab_chance)) {
			RogueBackstab(m, false, reuse);
		}
	} else { //We do a single regular attack if we attack from the front without chaotic stab
		Attack(m, EQ::invslot::slotPrimary);
	}
}

void Mob::RogueBackstab(Mob* m, bool min_damage, int reuse)
{
	if (!m) {
		return;
	}

	int64 hate = 0;

	// make sure we can hit (bane, magical, etc)
	if (IsClient()) {
		const auto* weapon = CastToClient()->GetInv().GetItem(EQ::invslot::slotPrimary);
		if (!GetWeaponDamage(m, weapon)) {
			return;
		}
	} else if (!GetWeaponDamage(m, (const EQ::ItemData*)nullptr)){
		return;
	}

	int base_damage = GetBaseSkillDamage(Skill::Backstab, m);

	hate = base_damage;

	DoSpecialAttackDamage(m, Skill::Backstab, base_damage, 0, hate, reuse);
	DoAnim(AttackAnimation::OneHandPiercing, 0, false);
}

// assassinate [No longer used for regular assassinate 6-29-14]
void Mob::RogueAssassinate(Mob* m)
{
	//can you dodge, parry, etc.. an assassinate??
	//if so, use DoSpecialAttackDamage(other, BACKSTAB, 32000); instead
	const auto* inst = IsClient() ? CastToClient()->GetInv().GetItem(EQ::invslot::slotPrimary) : nullptr;

	if (GetWeaponDamage(m, inst) > 0) {
		m->Damage(this, 32000, SPELL_UNKNOWN, Skill::Backstab);
	} else {
		m->Damage(this, -5, SPELL_UNKNOWN, Skill::Backstab);
	}

	DoAnim(AttackAnimation::OneHandPiercing, 0, false);	//piercing animation
}

void Client::RangedAttack(Mob* m, bool can_double_attack)
{
	if (!m || m == this) {
		return;
	}

	//make sure the attack and ranged timers are up
	//if the ranged timer is disabled, then they have no ranged weapon and shouldent be attacking anyhow
	if (
		!can_double_attack &&
		(
			(attack_timer.Enabled() && !attack_timer.Check(false)) ||
			(ranged_timer.Enabled() && !ranged_timer.Check())
		)
	) {
		LogCombat(
			"Throwing attack canceled. Timer not up. Attack [{}], ranged [{}]",
			attack_timer.GetRemainingTime(),
			ranged_timer.GetRemainingTime()
		);
		return;
	}

	const auto* range_weapon = m_inv[EQ::invslot::slotRange];

	int ammo_slot = EQ::invslot::slotAmmo;
	const auto* ammo = m_inv[EQ::invslot::slotAmmo];

	if (!range_weapon || !range_weapon->IsClassCommon()) {
		LogCombat(
			"Ranged attack canceled. Missing or invalid ranged weapon ([{}]) in slot [{}]",
			GetItemIDAt(EQ::invslot::slotRange),
			EQ::invslot::slotRange
		);
		Message(Chat::White, "You have no bow!");
		return;
	}
	if (!ammo || !ammo->IsClassCommon()) {
		LogCombat(
			"Ranged attack canceled. Missing or invalid ammo item ([{}]) in slot [{}]",
			GetItemIDAt(EQ::invslot::slotAmmo),
			EQ::invslot::slotAmmo
		);
		Message(Chat::White, "You have no ammo!");
		return;
	}

	const auto* range_item = range_weapon->GetItem();
	const auto* ammo_item  = ammo->GetItem();

	if (range_item->ItemType != EQ::item::ItemTypeBow) {
		const std::string& item_link = database.CreateItemLink(range_weapon->GetID());
		LogCombat("Ranged attack canceled. Ranged item is not a bow. type [{}]", range_item->ItemType);
		Message(
			Chat::White,
			fmt::format(
				"{} is not a bow.",
				item_link
			).c_str()
		);
		return;
	}

	if (ammo_item->ItemType != EQ::item::ItemTypeArrow) {
		const std::string& item_link = database.CreateItemLink(range_weapon->GetID());
		LogCombat("Ranged attack canceled. Ammo item is not an arrow. type [{}]", ammo_item->ItemType);
		Message(
			Chat::White,
			fmt::format(
				"{} is not an arrow.",
				item_link
			).c_str()
		);
		return;
	}

	LogCombat(
		"Shooting [{}] with bow [{}] ([{}]) and arrow [{}] ([{}])",
		m->GetName(),
		range_item->Name,
		range_item->ID,
		ammo_item->Name,
		ammo_item->ID
	);

	//look for ammo in inventory if we only have 1 left...
	if (ammo->GetCharges() == 1) {
		//first look for quivers
		bool found = false;
		for (int slot_id = EQ::invslot::GENERAL_BEGIN; slot_id <= EQ::invslot::GENERAL_END; slot_id++) {
			const auto* inst = m_inv[slot_id];
			if (!inst || !inst->IsClassBag()) {
				continue;
			}

			const auto* item = inst->GetItem();
			if (!item || item->BagType != EQ::item::BagTypeQuiver) {
				continue;
			}

			//we found a quiver, look for the ammo in it
			for (int bag_slot_id = 0; bag_slot_id < item->BagSlots; bag_slot_id++) {
				const auto* bag_inst = inst->GetItem(bag_slot_id);
				if (!bag_inst) {
					continue;
				}

				if (bag_inst->GetID() == ammo->GetID()) {
					ammo      = bag_inst;
					ammo_slot = EQ::InventoryProfile::CalcSlotId(slot_id, bag_slot_id);
					found     = true;
					LogCombat(
						"Using ammo from quiver stack at slot [{}]. [{}] in stack",
						ammo_slot,
						ammo->GetCharges()
					);
					break;
				}
			}

			if (found) {
				break;
			}
		}

		if (!found) {
			//if we dont find a quiver, look through our inventory again
			//not caring if the thing is a quiver.
			const int16 ammo_slot_id = m_inv.HasItem(ammo_item->ID, 1, invWherePersonal);
			if (ammo_slot_id != INVALID_INDEX) {
				ammo_slot = ammo_slot_id;
				ammo      = m_inv[ammo_slot_id];
				LogCombat("Using ammo from inventory stack at slot [{}]. [{}] in stack", ammo_slot, ammo->GetCharges());
			}
		}
	}

	float range = range_item->Range + ammo_item->Range + GetRangeDistTargetSizeMod(GetTarget());

	LogCombat("Calculated bow range to be [{}]", range);

	range *= range;

	const float dist = DistanceSquared(m_Position, m->GetPosition());

	if (dist > range) {
		LogCombat("Ranged attack out of range client should catch this. ([{}] > [{}]).", dist, range);
		MessageString(
			Chat::Red,
			TARGET_OUT_OF_RANGE
		);
		return;
	} else if (dist < (RuleI(Combat, MinRangedAttackDist) * RuleI(Combat, MinRangedAttackDist))) {
		MessageString(
			Chat::Yellow,
			RANGED_TOO_CLOSE
		);
		return;
	}

	if (
		!IsAttackAllowed(m) ||
		IsCasting() ||
		IsSitting() ||
		(DivineAura() && !GetGM()) ||
		IsStunned() ||
		IsFeared() ||
		IsMezzed() ||
		GetAppearance() == eaDead
	) {
		return;
	}

	//Shoots projectile and/or applies the archery damage
	DoArcheryAttackDmg(m, range_weapon, ammo, 0, 0, 0, 0, 0, 0, ammo_item, ammo_slot);

	//EndlessQuiver AA base1 = 100% Chance to avoid consumption arrow.
	int chance_avoid_consume = (
		aabonuses.ConsumeProjectile +
		itembonuses.ConsumeProjectile +
		spellbonuses.ConsumeProjectile
	);

	// Consume Ammo, unless Ammo Consumption is disabled or player has Endless Quiver
	const bool consumes_ammo = RuleB(Combat, ArcheryConsumesAmmo);
	if (
		consumes_ammo &&
		(
			range_item->ExpendableArrow ||
			!chance_avoid_consume ||
			(chance_avoid_consume < 100 && zone->random.Int(0, 99) > chance_avoid_consume)
		)
	) {
		DeleteItemInInventory(ammo_slot, 1, true);
		LogCombat("Consumed Archery Ammo from slot {}.", ammo_slot);
	} else if (!consumes_ammo) {
		LogCombat("Archery Ammo Consumption is disabled.");
	} else {
		LogCombat("Endless Quiver prevented Ammo Consumption.");
	}

	CheckIncreaseSkill(Skill::Archery, GetTarget(), -15);
	CommonBreakInvisibleFromCombat();
}

void Mob::DoArcheryAttackDmg(
	Mob* m,
	const EQ::ItemInstance* range_weapon,
	const EQ::ItemInstance* ammo,
	int32 weapon_damage,
	int16 chance_mod,
	int16 focus,
	int reuse,
	uint32 range_id,
	uint32 ammo_id,
	const EQ::ItemData* ammo_item,
	int ammo_slot_id,
	float speed,
	bool disable_procs
)
{
	if (
		HasDied() ||
		!m ||
		!IsAttackAllowed(m) ||
		m->GetInvul() ||
		m->GetSpecialAbility(IMMUNE_MELEE) ||
		(
			(IsClient() && CastToClient()->dead) ||
			(m->IsClient() && m->CastToClient()->dead)
		)
	) {
		return;
	}

	const EQ::ItemInstance* current_range_weapon = nullptr;
	const EQ::ItemInstance* current_ammo         = nullptr;

	const EQ::ItemData* last_ammo_used = nullptr;

	/*
	If LaunchProjectile is false this function will do archery damage on target,
	otherwise it will shoot the projectile at the target, once the projectile hits target
	this function is then run again to do the damage portion
	*/
	bool launch_projectile = false;

	if (RuleB(Combat, ProjectileDmgOnImpact)) {
		if (ammo_item) { // won't be null when we are firing the arrow
			launch_projectile = true;
		} else {
			/*
			Item sync check on projectile landing.
			Weapon damage is already calculated so this only affects procs!
			Ammo proc check will use database to find proc if you used up your last ammo.
			If you change range item mid projectile flight, you loose your chance to proc from bow (Deal
			with it!).
			*/

			if (!range_weapon && !ammo && range_id && ammo_id) {
				if (IsClient()) {
					current_range_weapon = CastToClient()->m_inv[EQ::invslot::slotRange];
					if (
						current_range_weapon &&
						current_range_weapon->GetItem() &&
						current_range_weapon->GetItem()->ID == range_id
					) {
						range_weapon = current_range_weapon;
					}

					current_ammo = CastToClient()->m_inv[ammo_slot_id];

					if (current_ammo && current_ammo->GetItem() && current_ammo->GetItem()->ID == ammo_id) {
						ammo = current_ammo;
					} else {
						last_ammo_used = database.GetItem(ammo_id);
					}
				}
			}
		}
	} else if (ammo_item) {
		SendItemAnimation(m, ammo_item, Skill::Archery);
	}

	LogCombat("Ranged attack hit [{}]", m->GetName());

	int64 hate                  = 0;
	int64 total_damage          = 0;
	int   current_weapon_damage = Damage::None;
	int   current_ammo_damage   = 0;

	if (!weapon_damage) {
		current_weapon_damage = GetWeaponDamage(m, range_weapon);
		current_ammo_damage   = GetWeaponDamage(m, ammo);
	} else {
		current_weapon_damage = weapon_damage;
	}

	if (launch_projectile) { // 1: Shoot the Projectile once we calculate weapon damage.
		TryProjectileAttack(
			m,
			ammo_item,
			Skill::Archery,
			(current_weapon_damage + current_ammo_damage),
			range_weapon,
			ammo,
			ammo_slot_id,
			speed,
			disable_procs
		);
		return;
	}

	if (focus) {
		current_weapon_damage += current_weapon_damage * focus / 100;
	}

	if (current_weapon_damage > Damage::None || current_ammo_damage > Damage::None) {
		if (current_weapon_damage < Damage::None) {
			current_weapon_damage = Damage::None;
		}

		if (current_ammo_damage < Damage::None) {
			current_ammo_damage = Damage::None;
		}

		int max_damage = current_weapon_damage + current_ammo_damage;
		hate = ((current_weapon_damage + current_ammo_damage));

		if (RuleB(Combat, ProjectileDmgOnImpact)) {
			LogCombat("Bow and Arrow DMG [{}], Max Damage [{}]", current_weapon_damage, max_damage);
		} else {
			LogCombat("Bow DMG [{}], Arrow DMG [{}], Max Damage [{}]", current_weapon_damage, current_ammo_damage, max_damage);
		}

		if (max_damage == Damage::None) {
			max_damage = 1;
		}

		DamageHitInfo my_hit {};

		my_hit.base_damage = max_damage;
		my_hit.min_damage  = 0;
		my_hit.damage_done = 1;
		my_hit.skill_id    = Skill::Archery;
		my_hit.offense     = offense(my_hit.skill_id);
		my_hit.tohit       = GetTotalToHit(my_hit.skill_id, chance_mod);
		my_hit.hand        = EQ::invslot::slotRange;

		DoAttack(m, my_hit);
		total_damage = my_hit.damage_done;
	} else {
		total_damage = Damage::Invulnerable;
	}

	if (IsClient() && !CastToClient()->GetFeigned()) {
		m->AddToHateList(this, hate, 0);
	}

	m->Damage(this, total_damage, SPELL_UNKNOWN, Skill::Archery);


	if (!disable_procs) {
		// Weapon Proc
		if (range_weapon && m && !m->HasDied()) {
			TryCombatProcs(range_weapon, m, EQ::invslot::slotRange);
		}

		// Ammo Proc, do not try spell procs if from ammo.
		if (last_ammo_used) {
			TryWeaponProc(nullptr, last_ammo_used, m, EQ::invslot::slotRange);
		} else if (ammo && m && !m->HasDied()) {
			TryWeaponProc(ammo, ammo->GetItem(), m, EQ::invslot::slotRange);
		}
	}

	TryCastOnSkillUse(m, Skill::Archery);

	if (!disable_procs) {
		// Skill Proc Attempt
		if (HasSkillProcs() && m && !m->HasDied()) {
			if (reuse) {
				TrySkillProc(m, Skill::Archery, reuse);
			} else {
				TrySkillProc(m, Skill::Archery, 0, false, EQ::invslot::slotRange);
			}
		}

		// Skill Proc Success ... can proc off hits OR misses
		if (HasSkillProcSuccess() && m && !m->HasDied()) {
			if (reuse) {
				TrySkillProc(m, Skill::Archery, reuse, true);
			} else {
				TrySkillProc(m, Skill::Archery, 0, true, EQ::invslot::slotRange);
			}
		}
	}
}

bool Mob::TryProjectileAttack(
	Mob* m,
	const EQ::ItemData* item,
	uint16 skill_id,
	uint64 weapon_dmg,
	const EQ::ItemInstance* range_weapon,
	const EQ::ItemInstance* ammo,
	int ammo_slot_id,
	float speed,
	bool disable_procs
)
{
	if (!m) {
		return false;
	}

	int slot_id = -1;

	// Make sure there is an avialable slot.
	for (int i = 0; i < MAX_SPELL_PROJECTILE; i++) {
		if (ProjectileAtk[i].target_id == 0) {
			slot_id = i;
			break;
		}
	}

	if (slot_id < 0) {
		return false;
	}

	float distance_mod = 0.0f;
	float distance     = m->CalculateDistance(GetX(), GetY(), GetZ());

	/*
	New Distance Mod constant (7/25/21 update), modifier is needed to adjust slower speeds to have correct impact times at short distances.
	We use archery 4.0 speed as a baseline for the forumla.  At speed 1.5 at 50 pct distance mod is needed, where as speed 4.0 there is no modifer.
	Therefore, we derive out our modifer as follows. distance_mod = (speed - 4) * ((50 - 0)/(1.5-4)). The ratio there is -20.0f. distance_mod = (speed - 4) * -20.0f
	For distances >125 we use different modifier, this was all meticulously tested by eye to get the best possible outcome for projectile impact times. Not perfect though.
	*/

	if (distance <= 125.0f) {
		if (speed != 4.0f) { //Standard functions will always be 4.0f for archery.
			distance_mod = (speed - 4.0f) * -20.0f;
			distance += distance * distance_mod / 100.0f;
		}
	} else if (distance > 125.0f && distance <= 200.0f) {
		distance = 3.14f * (distance / 2.0f); //Get distance of arc to better reflect projectile path length
	} else if (distance > 200.0f) {
		distance = distance * 1.30f; //Add 30% to base distance if over 200 range to tighten up hit timing.
		distance = 3.14f * (distance / 2.0f); //Get distance of arc to better reflect projectile path length
	}

	float hit = 1200.0f + (10 * distance / speed);

	ProjectileAtk[slot_id].increment     = 1;
	ProjectileAtk[slot_id].hit_increment = static_cast<uint16>(hit); // This projected hit time if target does NOT MOVE
	ProjectileAtk[slot_id].target_id     = m->GetID();
	ProjectileAtk[slot_id].wpn_dmg       = weapon_dmg;
	ProjectileAtk[slot_id].origin_x      = GetX();
	ProjectileAtk[slot_id].origin_y      = GetY();
	ProjectileAtk[slot_id].origin_z      = GetZ();

	if (range_weapon && range_weapon->GetItem()) {
		ProjectileAtk[slot_id].ranged_id = range_weapon->GetItem()->ID;
	}

	if (ammo && ammo->GetItem()) {
		ProjectileAtk[slot_id].ammo_id = ammo->GetItem()->ID;
	}

	ProjectileAtk[slot_id].ammo_slot     = ammo_slot_id;
	ProjectileAtk[slot_id].skill         = skill_id;
	ProjectileAtk[slot_id].speed_mod     = speed;
	ProjectileAtk[slot_id].disable_procs = disable_procs;

	SetProjectileAttack(true);

	if (item) {
		SendItemAnimation(m, item, skill_id, speed);
	}

	return true;
}

void Mob::ProjectileAttack()
{
	if (!HasProjectileAttack()) {
		return;
	}

	Mob* m = nullptr;
	bool disable = true;

	for (int slot_id = 0; slot_id < MAX_SPELL_PROJECTILE; slot_id++) {
		if (ProjectileAtk[slot_id].increment == 0) {
			continue;
		}

		disable = false;

		m = entity_list.GetMobID(ProjectileAtk[slot_id].target_id);

		if (m && m->IsMoving()) {
			/*
			Only recalculate hit increment if target is moving.
			Due to frequency that we need to check increment the targets position variables may not be
			updated even if moving. Do a simple check before calculating distance.
			*/
			if (ProjectileAtk[slot_id].tlast_x != m->GetX() || ProjectileAtk[slot_id].tlast_y != m->GetY()) {

				ProjectileAtk[slot_id].tlast_x = m->GetX();
				ProjectileAtk[slot_id].tlast_y = m->GetY();

				//Recalculate from the original location the projectile was fired in relation to the current targets location.
				float distance = m->CalculateDistance(
					ProjectileAtk[slot_id].origin_x,
					ProjectileAtk[slot_id].origin_y,
					ProjectileAtk[slot_id].origin_z
				);
				float distance_mod = 0.0f;

				if (distance <= 125.0f) {
					distance_mod = (ProjectileAtk[slot_id].speed_mod - 4.0f) * -20.0f;
					distance += distance * distance_mod / 100.0f;
				} else if (distance > 125.0f && distance <= 200.0f) {
					distance = 3.14f * (distance / 2.0f); //Get distance of arc to better reflect projectile path length
				} else if (distance > 200.0f) {
					distance = distance * 1.30f; //Add 30% to base distance if over 200 range to tighten up hit timing.
					distance = 3.14f * (distance / 2.0f); //Get distance of arc to better reflect projectile path length
				}

				float hit = 1200.0f + (10 * distance / ProjectileAtk[slot_id].speed_mod);

				ProjectileAtk[slot_id].hit_increment = static_cast<uint16>(hit);
			}
		}

		// Check if we hit.
		if (ProjectileAtk[slot_id].hit_increment <= ProjectileAtk[slot_id].increment) {
			if (m) {
				if (IsNPC()) {
					if (ProjectileAtk[slot_id].skill == Skill::Conjuration) {
						if (IsValidSpell(ProjectileAtk[slot_id].wpn_dmg)) {
							SpellOnTarget(
								ProjectileAtk[slot_id].wpn_dmg,
								m,
								false,
								true,
								spells[ProjectileAtk[slot_id].wpn_dmg].resist_difficulty,
								true
							);
						}
					} else {
						CastToNPC()->DoRangedAttackDmg(
							m,
							false,
							ProjectileAtk[slot_id].wpn_dmg,
							0,
							ProjectileAtk[slot_id].skill
						);
					}
				} else {
					if (ProjectileAtk[slot_id].skill == Skill::Archery) {
						DoArcheryAttackDmg(
							m,
							nullptr,
							nullptr,
							ProjectileAtk[slot_id].wpn_dmg,
							0,
							0,
							0,
							ProjectileAtk[slot_id].ranged_id,
							ProjectileAtk[slot_id].ammo_id,
							nullptr,
							ProjectileAtk[slot_id].ammo_slot,
							4.0f,
							ProjectileAtk[slot_id].disable_procs
						);
					} else if (ProjectileAtk[slot_id].skill == Skill::Throwing) {
						DoThrowingAttackDmg(
							m,
							nullptr,
							nullptr,
							ProjectileAtk[slot_id].wpn_dmg,
							0,
							0,
							0,
							ProjectileAtk[slot_id].ranged_id,
							ProjectileAtk[slot_id].ammo_slot,
							4.0f,
							ProjectileAtk[slot_id].disable_procs
						);
					} else if (
						ProjectileAtk[slot_id].skill == Skill::Conjuration &&
						IsValidSpell(ProjectileAtk[slot_id].wpn_dmg)
					) {
						SpellOnTarget(
							ProjectileAtk[slot_id].wpn_dmg,
							m,
							false,
							true,
							spells[ProjectileAtk[slot_id].wpn_dmg].resist_difficulty,
							true
						);
					}
				}
			}

			ProjectileAtk[slot_id].increment = 0;
			ProjectileAtk[slot_id].target_id = 0;
			ProjectileAtk[slot_id].wpn_dmg   = 0;
			ProjectileAtk[slot_id].origin_x  = 0.0f;
			ProjectileAtk[slot_id].origin_y  = 0.0f;
			ProjectileAtk[slot_id].origin_z  = 0.0f;
			ProjectileAtk[slot_id].tlast_x   = 0.0f;
			ProjectileAtk[slot_id].tlast_y   = 0.0f;
			ProjectileAtk[slot_id].ranged_id = 0;
			ProjectileAtk[slot_id].ammo_id   = 0;
			ProjectileAtk[slot_id].ammo_slot = 0;
			ProjectileAtk[slot_id].skill     = 0;
			ProjectileAtk[slot_id].speed_mod = 0.0f;
		} else {
			ProjectileAtk[slot_id].increment += 1000 * frame_time;
		}
	}

	if (disable) {
		SetProjectileAttack(false);
	}
}

float Mob::GetRangeDistTargetSizeMod(Mob* m)
{
	/*
	Range is enforced client side, therefore these numbers do not need to be 100% accurate just close enough to
	prevent any exploitation. The range mod changes in some situations depending on if size is from spawn or from SendIllusionPacket changes.
	At present time only calculate from spawn (it is no consistent what happens to the calc when changing it after spawn).
	*/
	if (!m) {
		return 0.0f;
	}

	float target_size = m->GetSize();

	if (GetSize() > target_size) {
		target_size = GetSize();
	}

	float size_modifier = 0.0f;

	if (target_size < 10) {
		size_modifier = 18.0f;
	} else if (EQ::ValueWithin(target_size, 10, 14)) {
		size_modifier = 20.0f + (4.0f * (target_size - 10.0f));
	} else if (EQ::ValueWithin(target_size, 15, 20)) {
		size_modifier = 42.0f + (5.8f * (target_size - 15.0f));
	} else {
		size_modifier = 75.0f;
	}

	return (size_modifier + 2.0f); //Add 2.0f as buffer to prevent any chance of failures, client enforce range check regardless.
}

void NPC::RangedAttack(Mob* m)
{
	if (!m) {
		return;
	}

	// make sure the attack and ranged timers are up
	// if the ranged timer is disabled, then they have no ranged weapon and shouldent be attacking anyhow
	if (
		(attack_timer.Enabled() && !attack_timer.Check(false)) ||
		(ranged_timer.Enabled() && !ranged_timer.Check())
	) {
		LogCombat(
			"Archery canceled. Timer not up. Attack [{}], ranged [{}]",
			attack_timer.GetRemainingTime(),
			ranged_timer.GetRemainingTime()
		);
		return;
	}

	if (!HasBowAndArrowEquipped() && !GetSpecialAbility(SPECATK_RANGED_ATK)) {
		return;
	}

	if (!CheckLosFN(m)) {
		return;
	}

	int attacks = 1;
	float min_range = static_cast<float>(RuleI(Combat, MinRangedAttackDist));
	float max_range = 250.0f; // needs to be longer than 200(most spells)

	if (GetSpecialAbility(SPECATK_RANGED_ATK)) {
		int temp_attacks = GetSpecialAbilityParam(SPECATK_RANGED_ATK, 0);
		attacks = temp_attacks > 0 ? temp_attacks : 1;

		int temp_min_range = GetSpecialAbilityParam(SPECATK_RANGED_ATK, 4); // Min Range of NPC attack
		int temp_max_range = GetSpecialAbilityParam(SPECATK_RANGED_ATK, 1); // Max Range of NPC attack

		if (temp_max_range) {
			max_range = static_cast<float>(temp_max_range);
		}

		if (temp_min_range) {
			min_range = static_cast<float>(temp_min_range);
		}
	}

	max_range *= max_range;
	min_range *= min_range;

	for (int i = 0; i < attacks; ++i) {
		if (DistanceSquared(m_Position, m->GetPosition()) > max_range) {
			return;
		} else if (DistanceSquared(m_Position, m->GetPosition()) < min_range) {
			return;
		}

		if (
			!m ||
			!IsAttackAllowed(m) ||
			IsCasting() ||
			DivineAura() ||
			IsStunned() ||
			IsFeared() ||
			IsMezzed() ||
			GetAppearance() == eaDead
		) {
			return;
		}

		FaceTarget(m);

		DoRangedAttackDmg(m);

		CommonBreakInvisibleFromCombat();
	}
}

void NPC::DoRangedAttackDmg(
	Mob* m,
	bool launch,
	int16 damage_mod,
	int16 chance_mod,
	uint16 skill_id,
	float speed,
	const std::string& idfile
)
{
	if (
		HasDied() ||
		!m ||
		m->HasDied() ||
		!IsAttackAllowed(m) ||
		m->GetInvul() ||
		m->GetSpecialAbility(IMMUNE_MELEE)
	) {
		return;
	}

	const uint16 skill_to_use = skill_id != GetRangedSkill() ? skill_id : GetRangedSkill();

	if (launch) {
		ProjectileAnimation(
			m,
			0,
			false,
			speed,
			0,
			0,
			0,
			idfile.empty() ? GetAmmoIDfile() : idfile.c_str(),
			skill_to_use
		);

		if (RuleB(Combat, ProjectileDmgOnImpact)) {
			TryProjectileAttack(m, nullptr, skill_to_use, damage_mod, nullptr, nullptr, 0, speed);
			return;
		}
	}

	if (!chance_mod) {
		chance_mod = GetSpecialAbilityParam(SPECATK_RANGED_ATK, 2);
	}

	int total_damage = Damage::None;
	int max_damage   = GetBaseDamage() * RuleR(Combat, ArcheryNPCMultiplier); // should add a field to npc_types
	int min_damage   = GetMinDamage() * RuleR(Combat, ArcheryNPCMultiplier);

	if (!damage_mod) {
		damage_mod = GetSpecialAbilityParam(SPECATK_RANGED_ATK, 3);
	}

	DamageHitInfo my_hit;

	my_hit.base_damage = max_damage;
	my_hit.min_damage  = min_damage;
	my_hit.damage_done = 1;
	my_hit.skill_id    = skill_id;
	my_hit.offense     = offense(my_hit.skill_id);
	my_hit.tohit       = GetTotalToHit(my_hit.skill_id, chance_mod);
	my_hit.hand        = EQ::invslot::slotRange;

	DoAttack(m, my_hit);

	total_damage = my_hit.damage_done;

	if (total_damage > Damage::None) {
		total_damage += total_damage * damage_mod / 100;
		m->AddToHateList(this, total_damage, 0);
	} else {
		m->AddToHateList(this, 0, 0);
	}

	m->Damage(this, total_damage, SPELL_UNKNOWN, skill_to_use);

	//try proc on hits and misses
	if (m && !m->HasDied()) {
		TrySpellProc(nullptr, (const EQ::ItemData*)nullptr, m, EQ::invslot::slotRange);
	}

	TryCastOnSkillUse(m, skill_to_use);

	if (HasSkillProcs() && m && !m->HasDied()) {
		TrySkillProc(m, skill_to_use, 0, false, EQ::invslot::slotRange);
	}

	if (HasSkillProcSuccess() && m && !m->HasDied()) {
		TrySkillProc(m, skill_to_use, 0, true, EQ::invslot::slotRange);
	}
}

void Client::ThrowingAttack(Mob* m, bool can_double_attack)
{
	if (!m) {
		return;
	}

	//make sure the attack and ranged timers are up
	//if the ranged timer is disabled, then they have no ranged weapon and shouldent be attacking anyhow
	if (
		!can_double_attack &&
		(
			(attack_timer.Enabled() && !attack_timer.Check(false)) ||
			(ranged_timer.Enabled() && !ranged_timer.Check())
		)
	) {
		LogCombat(
			"Throwing attack canceled. Timer not up. Attack [{}], ranged [{}]",
			attack_timer.GetRemainingTime(),
			ranged_timer.GetRemainingTime()
		);
		return;
	}

	int ammo_slot = EQ::invslot::slotRange;
	const auto* range_weapon = m_inv[EQ::invslot::slotRange];

	if (!range_weapon || !range_weapon->IsClassCommon()) {
		const std::string& item_link = database.CreateItemLink(range_weapon->GetID());
		LogCombat("Ranged attack canceled. Ranged item [{}] is not a throwing weapon.", range_weapon->GetID());
		Message(
			Chat::White,
			fmt::format(
				"{} is not a throwing weapon.",
				item_link
			).c_str()
		);
		return;
	}

	const auto* item = range_weapon->GetItem();
	if (item->ItemType != EQ::item::ItemTypeLargeThrowing && item->ItemType != EQ::item::ItemTypeSmallThrowing) {
		const std::string& item_link = database.CreateItemLink(range_weapon->GetID());
		LogCombat("Ranged attack canceled. Ranged item [{}] is not a throwing weapon. type [{}]", item->ID, item->ItemType);
		Message(
			Chat::White,
			fmt::format(
				"{} is not a throwing weapon.",
				item_link
			).c_str()
		);
		return;
	}

	LogCombat("Throwing [{}] ([{}]) at [{}]", item->Name, item->ID, m->GetName());

	if (range_weapon->GetCharges() == 1) {
		const auto* ammo_item = m_inv[EQ::invslot::slotAmmo];
		if (ammo_item && ammo_item->GetID() == range_weapon->GetID()) {
			range_weapon = ammo_item;
			ammo_slot    = EQ::invslot::slotAmmo;
			LogCombat("Using ammo from ammo slot, stack at slot [{}]. [{}] in stack", ammo_slot, range_weapon->GetCharges());
		} else {
			const int16 ammo_slot_id = m_inv.HasItem(item->ID, 1, invWherePersonal);
			if (ammo_slot_id != INVALID_INDEX) {
				ammo_slot    = ammo_slot_id;
				range_weapon = m_inv[ammo_slot_id];
				LogCombat("Using ammo from inventory slot, stack at slot [{}]. [{}] in stack", ammo_slot, range_weapon->GetCharges());
			}
		}
	}

	float range = item->Range + GetRangeDistTargetSizeMod(m);

	LogCombat("Calculated bow range to be [{}]", range);

	range *= range;

	float dist = DistanceSquared(m_Position, m->GetPosition());
	if (dist > range) {
		LogCombat("Throwing attack out of range client should catch this. ([{}] > [{}]).\n", dist, range);
		MessageString(
			Chat::Red,
			TARGET_OUT_OF_RANGE
		);
		return;
	} else if (dist < (RuleI(Combat, MinRangedAttackDist) * RuleI(Combat, MinRangedAttackDist))) {
		MessageString(
			Chat::Yellow,
			RANGED_TOO_CLOSE
		);
		return;
	}

	if (
		!IsAttackAllowed(m) ||
		(IsCasting() && GetClass() != Class::Bard) ||
		IsSitting() ||
		(DivineAura() && !GetGM()) ||
		IsStunned() ||
		IsFeared() ||
		IsMezzed() ||
		GetAppearance() == eaDead
	) {
		return;
	}

	DoThrowingAttackDmg(m, range_weapon, item, 0, 0, 0, 0, 0, ammo_slot);

	// Consume Ammo, unless Ammo Consumption is disabled
	if (RuleB(Combat, ThrowingConsumesAmmo)) {
		DeleteItemInInventory(ammo_slot, 1, true);
		LogCombat("Consumed Throwing Ammo from slot {}.", ammo_slot);
	} else {
		LogCombat("Throwing Ammo Consumption is disabled.");
	}

	CommonBreakInvisibleFromCombat();
}

void Mob::DoThrowingAttackDmg(
	Mob* m,
	const EQ::ItemInstance* range_weapon,
	const EQ::ItemData* ammo_item,
	int32 weapon_damage,
	int16 chance_mod,
	int16 focus,
	int reuse,
	uint32 range_id,
	int ammo_slot_id,
	float speed,
	bool disable_procs
)
{
	if (
		HasDied() ||
		!m ||
		!IsAttackAllowed(m) ||
		m->GetInvul() ||
		m->GetSpecialAbility(IMMUNE_MELEE) ||
		(
			(IsClient() && CastToClient()->dead) ||
			(m->IsClient() && m->CastToClient()->dead)
		)
	) {
		return;
	}

	const EQ::ItemInstance* current_range_weapon = nullptr;

	const EQ::ItemData* last_ammo_used = nullptr;

	/*
	If LaunchProjectile is false this function will do archery damage on target,
	otherwise it will shoot the projectile at the target, once the projectile hits target
	this function is then run again to do the damage portion
	*/
	bool launch = false;

	if (RuleB(Combat, ProjectileDmgOnImpact)) {
		if (ammo_item) {
			launch = true;
		} else {
			if (!range_weapon && range_id) {
				if (IsClient()) {
					current_range_weapon = CastToClient()->m_inv[ammo_slot_id];

					if (current_range_weapon && current_range_weapon->GetItem() && current_range_weapon->GetItem()->ID == range_id) {
						range_weapon = current_range_weapon;
					} else {
						last_ammo_used = database.GetItem(range_id);
					}
				}
			}
		}
	} else if (ammo_item) {
		SendItemAnimation(m, ammo_item, Skill::Throwing);
	}

	LogCombat("Throwing attack hit [{}]", m->GetName());

	int current_weapon_damage = Damage::None;

	if (!weapon_damage) {
		if (IsOfClientBot() && range_weapon) {
			current_weapon_damage = GetWeaponDamage(m, range_weapon);
		} else if (ammo_item) {
			current_weapon_damage = GetWeaponDamage(m, ammo_item);
		}

		if (launch) {
			TryProjectileAttack(
				m,
				ammo_item,
				Skill::Throwing,
				current_weapon_damage,
				range_weapon,
				nullptr,
				ammo_slot_id,
				speed
			);
			return;
		}
	} else {
		current_weapon_damage = weapon_damage;
	}

	if (focus) { // no longer used, keep for quests
		current_weapon_damage += current_weapon_damage * focus / 100;
	}

	int total_damage = Damage::None;

	if (current_weapon_damage > Damage::None) {
		DamageHitInfo my_hit{ };

		my_hit.base_damage = current_weapon_damage;
		my_hit.min_damage  = 0;
		my_hit.damage_done = 1;
		my_hit.skill_id    = Skill::Throwing;
		my_hit.offense     = offense(my_hit.skill_id);
		my_hit.tohit       = GetTotalToHit(my_hit.skill_id, chance_mod);
		my_hit.hand        = EQ::invslot::slotRange;

		DoAttack(m, my_hit);
		total_damage = my_hit.damage_done;

		LogCombat("Item DMG [{}]. Hit for damage [{}]", current_weapon_damage, total_damage);
	} else {
		total_damage = Damage::Invulnerable;
	}

	if (IsClient() && !CastToClient()->GetFeigned()) {
		m->AddToHateList(this, current_weapon_damage, 0);
	}

	m->Damage(this, total_damage, SPELL_UNKNOWN, Skill::Throwing);

	if (!disable_procs && m && !m->HasDied()) {
		TryCombatProcs(range_weapon, m, EQ::invslot::slotRange, last_ammo_used);
	}

	TryCastOnSkillUse(m, Skill::Throwing);

	if (!disable_procs) {
		if (HasSkillProcs() && m && !m->HasDied()) {
			if (reuse) {
				TrySkillProc(m, Skill::Throwing, reuse);
			} else {
				TrySkillProc(m, Skill::Throwing, 0, false, EQ::invslot::slotRange);
			}
		}

		if (HasSkillProcSuccess() && m && !m->HasDied()) {
			if (reuse) {
				TrySkillProc(m, Skill::Throwing, reuse, true);
			} else {
				TrySkillProc(m, Skill::Throwing, 0, true, EQ::invslot::slotRange);
			}
		}
	}

	if (IsClient()) {
		CastToClient()->CheckIncreaseSkill(Skill::Throwing, GetTarget());
	}
}

void Mob::SendItemAnimation(Mob* m, const EQ::ItemData* item, uint16 skill_id, float velocity)
{
	auto outapp = new EQApplicationPacket(OP_SomeItemPacketMaybe, sizeof(Arrow_Struct));
	auto* a = (Arrow_Struct*) outapp->pBuffer;

	a->type      = 1;
	a->src_x     = GetX();
	a->src_y     = GetY();
	a->src_z     = GetZ();
	a->source_id = GetID();
	a->target_id = m->GetID();
	a->item_id   = item->ID;
	a->item_type = item->ItemType;
	a->skill     = static_cast<uint8>(skill_id);

	strn0cpy(a->model_name, item->IDFile, 16);


	/*
		The angular field affects how the object flies towards the target.
		A low angular (10) makes it circle the target widely, where a high
		angular (20000) makes it go straight at them.

		The tilt field causes the object to be tilted flying through the air
		and also seems to have an effect on how it behaves when circling the
		target based on the angular field.

		Arc causes the object to form an arc in motion. A value too high will
	*/
	a->velocity = velocity;

	//these angle and tilt used together seem to make the arrow/knife throw as straight as I can make it

	a->launch_angle = CalculateHeadingToTarget(m->GetX(), m->GetY());
	a->tilt         = 125;
	a->arc          = 50;


	//fill in some unknowns, we dont know their meaning yet
	//neither of these seem to change the behavior any
	a->unknown088 = 125;
	a->unknown092 = 16;

	entity_list.QueueCloseClients(this, outapp);
	safe_delete(outapp);
}

void Mob::ProjectileAnimation(
	Mob* m,
	int item_id,
	bool is_arrow,
	float speed,
	float angle,
	float tilt,
	float arc,
	const std::string& idfile,
	uint16 skill_id
)
{
	if (!m) {
		return;
	}

	const auto* item = database.GetItem(item_id ? item_id : Item::ID::Arrow);
	uint8 item_type = is_arrow ? EQ::item::ItemType::ItemTypeArrow : EQ::item::ItemType::ItemType1HSlash;

	if (!item) {
		return;
	}

	if (!item_type && !skill_id) {
		item_type = item->ItemType;
	} else if (skill_id) {
		item_type = GetItemTypeBySkill(skill_id);
	}

	if (!speed) {
		speed = 4.0f;
	}

	if (!angle) {
		angle = CalculateHeadingToTarget(m->GetX(), m->GetY());
	}

	if (!tilt) {
		tilt = 125.0f;
	}

	if (!arc) {
		arc = 50.0f;
	}

	const std::string& item_idfile = !idfile.empty() && Strings::BeginsWith(idfile, "IT") ? idfile : item->IDFile;

	// See SendItemAnimation() for some notes on this struct
	auto outapp = new EQApplicationPacket(OP_SomeItemPacketMaybe, sizeof(Arrow_Struct));
	auto* a = (Arrow_Struct*) outapp->pBuffer;

	a->type         = 1;
	a->src_x        = GetX();
	a->src_y        = GetY();
	a->src_z        = GetZ();
	a->source_id    = GetID();
	a->target_id    = m->GetID();
	a->item_id      = item->ID;
	a->item_type    = item_type;
	a->skill        = skill_id;
	a->velocity     = speed;
	a->launch_angle = angle;
	a->tilt         = tilt;
	a->arc          = arc;
	a->unknown088   = 125;
	a->unknown092   = 16;

	strn0cpy(a->model_name, item_idfile.c_str(), sizeof(a->model_name));

	entity_list.QueueCloseClients(this, outapp);
	safe_delete(outapp);

}

void NPC::DoClassAttacks(Mob* m)
{
	if (!m) {
		return;
	}

	bool taunt_time         = taunt_timer.Check();
	bool class_attack_time  = classattack_timer.Check(false);
	bool knight_attack_time = knightattack_timer.Check(false);

	const auto* boots = database.GetItem(equipment[EQ::invslot::slotFeet]);

	//only check attack allowed if we are going to do something
	if ((taunt_time || class_attack_time || knight_attack_time) && !IsAttackAllowed(m)) {
		return;
	}

	if (knight_attack_time) {
		int knight_reuse = 1000;

		switch (GetClass()) {
			case Class::ShadowKnight:
			case Class::ShadowKnightGM: {
				if (CastSpell(SPELL_NPC_HARM_TOUCH, m->GetID())) {
					knight_reuse = Skill::Reuse::HarmTouch * 1000;
				}

				break;
			}
			case PALADIN: case PALADINGM:{
				if(GetHPRatio() < 20) {
					if (CastSpell(SPELL_LAY_ON_HANDS, GetID())) {
						knight_reuse = Skill::Reuse::LayOnHands * 1000;
					}
				} else {
					knight_reuse = 2000; //Check again in two seconds.
				}

				break;
			}
		}

		knightattack_timer.Start(knight_reuse);
	}

	//general stuff, for all classes....
	//only gets used when their primary ability get used too
	if (
		IsTaunting() &&
		HasOwner() &&
		m->IsNPC() &&
		m->GetBodyType() != BT_Undead &&
		taunt_time &&
		type_of_pet &&
		type_of_pet != petTargetLock &&
		DistanceSquared(GetPosition(), m->GetPosition()) <= (RuleI(Pets, PetTauntRange) * RuleI(Pets, PetTauntRange))
	) {
		GetOwner()->MessageString(Chat::PetResponse, PET_TAUNTING);
		Taunt(m->CastToNPC(), false);
	}

	if (!class_attack_time) {
		return;

	float haste_modifier = GetHaste() * 0.01f;

	int  level      = GetLevel();
	int  reuse      = Skill::Reuse::Taunt * 1000;    //make this very long since if they dont use it once, they prolly never will
	bool did_attack = false;

	switch (GetClass()) {
		case Class::Rogue:
		case Class::RogueGM:
			if (level >= 10) {
				reuse = Skill::Reuse::Backstab * 1000;
				TryBackstab(m, reuse);
				did_attack = true;
			}

			break;
		case Class::Monk:
		case Class::MonkGM: {
			uint16 monk_skill_id = Skill::Kick;

			if (level > 29) {
				monk_skill_id = Skill::FlyingKick;
			} else if (level > 24) {
				monk_skill_id = Skill::DragonPunch;
			} else if (level > 19) {
				monk_skill_id = Skill::EagleStrike;
			} else if (level > 9) {
				monk_skill_id = Skill::TigerClaw;
			} else if (level > 4) {
				monk_skill_id = Skill::RoundKick;
			}

			reuse      = MonkSpecialAttack(m, monk_skill_id) * 1000;
			did_attack = true;
			break;
		}
		case Class::Warrior:
		case Class::WarriorGM: {
			if (level >= RuleI(Combat, NPCBashKickLevel)) {
				if (zone->random.Roll(75)) { //tested on live, warrior mobs both kick and bash, kick about 75% of the time, casting doesn't seem to make a difference.
					DoAnim(AttackAnimation::Kick, 0, false);

					int64 damage = GetBaseSkillDamage(Skill::Kick);

					if (GetWeaponDamage(m, boots) <= 0) {
						damage = Damage::Invulnerable;
					}

					reuse = (Skill::Reuse::Kick + 3) * 1000;
					DoSpecialAttackDamage(m, Skill::Kick, damage, GetMinDamage(), -1, reuse);

					did_attack = true;
				} else {
					DoAnim(AttackAnimation::TailRake, 0, false);

					int64 damage = GetBaseSkillDamage(Skill::Bash);

					if (GetWeaponDamage(m, (const EQ::ItemData*) nullptr) <= 0) {
						damage = Damage::Invulnerable;
					}

					reuse = (Skill::Reuse::Bash + 3) * 1000;
					DoSpecialAttackDamage(m, Skill::Bash, damage, GetMinDamage(), -1, reuse);

					did_attack = true;
				}
			}
			break;
		}
		case Class::Berserker:
		case Class::BerserkerGM: {
			int attack_rounds = 1;
			int max_damage    = GetBaseSkillDamage(Skill::Frenzy);

			DoAnim(AttackAnimation::TwoHandSlashing, 0, false);

			if (GetClass() == Class::Berserker) {
				int chance = GetLevel() * 2 + GetSkill(Skill::Frenzy);

				if (zone->random.Roll0(450) < chance) {
					attack_rounds++;
				}

				if (zone->random.Roll0(450) < chance) {
					attack_rounds++;
				}
			}

			while (attack_rounds > 0) {
				if (GetTarget()) {
					DoSpecialAttackDamage(GetTarget(), Skill::Frenzy, max_damage, GetMinDamage(), -1, reuse);
				}

				attack_rounds--;
			}

			did_attack = true;
			break;
		}
		case Class::Ranger:
		case Class::RangerGM:
		case Class::Beastlord:
		case Class::BeastlordGM: {
			if (level >= RuleI(Combat, NPCBashKickLevel)) {
				DoAnim(AttackAnimation::Kick, 0, false);

				int64 damage = GetBaseSkillDamage(Skill::Kick);

				if (GetWeaponDamage(m, boots) <= 0) {
					damage = Damage::Invulnerable;
				}

				reuse = (Skill::Reuse::Kick + 3) * 1000;
				DoSpecialAttackDamage(m, Skill::Kick, damage, GetMinDamage(), -1, reuse);

				did_attack = true;
			}
			break;
		}
		case Class::Cleric:
		case Class::ClericGM: //clerics can bash too.
		case Class::ShadowKnight:
		case Class::ShadowKnightGM:
		case Class::Paladin:
		case Class::PaladinGM: {
			if (level >= RuleI(Combat, NPCBashKickLevel)) {
				DoAnim(AttackAnimation::TailRake, 0, false);

				int64 damage = GetBaseSkillDamage(Skill::Bash);

				if (GetWeaponDamage(m, (const EQ::ItemData*) nullptr) <= 0) {
					damage = Damage::Invulnerable;
				}

				reuse = (Skill::Reuse::Bash + 3) * 1000;
				DoSpecialAttackDamage(m, Skill::Bash, damage, GetMinDamage(), -1, reuse);

				did_attack = true;
			}
			break;
		}
	}

	classattack_timer.Start(reuse / haste_modifier);
}

// this should be refactored to generate an OP_CombatAbility struct and call OPCombatAbility
void Client::DoClassAttacks(Mob* m, uint16 skill_id, bool is_riposte)
{
	if (!m) {
		return;
	}

	if (
		spellend_timer.Enabled() ||
		IsFeared() ||
		IsStunned() ||
		IsMezzed() ||
		DivineAura() ||
		dead
	) {
		return;
	}

	if (!IsAttackAllowed(m)) {
		return;
	}

	//check range for all these abilities, they are all close combat stuff
	if (!CombatRange(m)) {
		return;
	}

	if (!is_riposte && !p_timers.Expired(&database, pTimerCombatAbility, false)) {
		return;
	}

	int    reuse          = 0;
	float  haste_modifier = GetHaste() * 0.01f;
	uint16 skill_to_use   = Skill::All;

	if (skill_id == Skill::All) {
		switch (GetClass()) {
			case Class::Warrior:
			case Class::Ranger:
			case Class::Beastlord:
				skill_to_use = Skill::Kick;
				break;
			case Class::Berserker:
				skill_to_use = Skill::Frenzy;
				break;
			case Class::ShadowKnight:
			case Class::Paladin:
				skill_to_use = Skill::Bash;
				break;
			case Class::Monk:
				if (GetLevel() >= 30) {
					skill_to_use = Skill::FlyingKick;
				} else if (GetLevel() >= 25) {
					skill_to_use = Skill::DragonPunch;
				} else if (GetLevel() >= 20) {
					skill_to_use = Skill::EagleStrike;
				} else if (GetLevel() >= 10) {
					skill_to_use = Skill::TigerClaw;
				} else if (GetLevel() >= 5) {
					skill_to_use = Skill::RoundKick;
				} else {
					skill_to_use = Skill::Kick;
				}

				break;
			case Class::Rogue:
				skill_to_use = Skill::Backstab;
				break;
		}
	} else {
		skill_to_use = skill_id;
	}

	if (skill_to_use == Skill::All) {
		return;
	}

	int64 damage = GetBaseSkillDamage(skill_to_use, GetTarget());

	if (skill_to_use == Skill::Bash) {
		if (m != this) {
			DoAnim(AttackAnimation::TailRake, 0, false);

			if (
				GetWeaponDamage(m, GetInv().GetItem(EQ::invslot::slotSecondary)) <= 0 &&
				GetWeaponDamage(m, GetInv().GetItem(EQ::invslot::slotShoulders)) <= 0
			) {
				damage = Damage::Invulnerable;
			}

			reuse = (Skill::Reuse::Bash - 1) / haste_modifier;

			DoSpecialAttackDamage(m, Skill::Bash, damage, 0, -1, reuse);

			if(reuse > 0 && !is_riposte) {
				p_timers.Start(pTimerCombatAbility, reuse);
			}
		}
		return;
	}

	if (skill_to_use == Skill::Frenzy) {
		CheckIncreaseSkill(Skill::Frenzy, GetTarget(), 10);

		int attack_rounds = 1;

		DoAnim(AttackAnimation::OneHandWeapon, 0, false);

		reuse = (Skill::Reuse::Frenzy - 1) / haste_modifier;

		// bards can do riposte frenzy for some reason
		if (!is_riposte && GetClass() == Class::Berserker) {
			int chance = GetLevel() * 2 + GetSkill(Skill::Frenzy);

			if (zone->random.Roll0(450) < chance) {
				attack_rounds++;
			}

			if (zone->random.Roll0(450) < chance) {
				attack_rounds++;
			}
		}

		while(attack_rounds > 0) {
			if (m != this) {
				DoSpecialAttackDamage(m, Skill::Frenzy, damage, 0, damage, reuse);
			}

			attack_rounds--;
		}

		if (reuse > 0 && !is_riposte) {
			p_timers.Start(pTimerCombatAbility, reuse);
		}

		return;
	}

	if (skill_to_use == Skill::Kick) {
		if (m != this) {
			DoAnim(AttackAnimation::Kick, 0, false);

			if (GetWeaponDamage(m, GetInv().GetItem(EQ::invslot::slotFeet)) <= 0) {
				damage = Damage::Invulnerable;
			}

			reuse = Skill::Reuse::Kick - 1;

			DoSpecialAttackDamage(m, Skill::Kick, damage, 0, -1, reuse);
		}
	}

	if (Skill::IsMonk(skill_to_use)) {
		reuse = MonkSpecialAttack(m, skill_to_use) - 1;

		MonkSpecialAttack(m, skill_to_use);

		if (is_riposte) {
			return;
		}

		//Live AA - Technique of Master Wu
		int wu_chance = (
			itembonuses.DoubleSpecialAttack +
			spellbonuses.DoubleSpecialAttack +
			aabonuses.DoubleSpecialAttack
		);
		if (wu_chance) {
			int extra = 0;

			// always 1/4 of the double attack chance, 25% at rank 5 (100/4)
			while (wu_chance > 0) {
				if (zone->random.Roll(wu_chance)) {
					extra++;
				} else {
					break;
				}

				wu_chance /= 4;
			}

			SendColoredText(
				Chat::MasterWu,
				fmt::format(
					"The spirit of Master Wu fills you!  You gain {} additional attack{}.",
					extra,
					extra != 1 ? "s" : ""
				).c_str()
			);

			const bool is_classic = RuleB(Combat, ClassicMasterWu);
			while (extra) {
				MonkSpecialAttack(GetTarget(), is_classic ? monk_skills[zone->random.Int(0, 4)] : skill_to_use);
				extra--;
			}
		}
	}

	if (skill_to_use == Skill::Backstab) {
		reuse = Skill::Reuse::Backstab - 1;

		if (is_riposte) {
			reuse = 0;
		}

		TryBackstab(m, reuse);
	}

	reuse = reuse / haste_modifier;

	if (reuse > 0 && !is_riposte) {
		p_timers.Start(pTimerCombatAbility, reuse);
	}
}

/* Classic Taunt Methodology
* This is not how Sony did it.  This is a guess that fits the very limited data available.
* Low level players with maxed taunt for their level taunted about 50% on white cons.
* A 65 ranger with 150 taunt skill (max) taunted about 50% on level 60 and under NPCs.
* A 65 warrior with maxed taunt (230) was taunting around 50% on SSeru NPCs.		*/

/* Rashere in 2006: "your taunt skill was irrelevant if you were above level 60 and taunting
* something that was also above level 60."
* Also: "The chance to taunt an NPC higher level than yourself dropped off at double the rate
* if you were above level 60 than if you were below level 60 making it very hard to taunt creature
* higher level than yourself if you were above level 60."
*
* See http://www.elitegamerslounge.com/home/soearchive/viewtopic.php?t=81156 */
void Mob::Taunt(NPC* n, bool always_succeed, int chance_bonus, bool from_spell, int32 bonus_hate)
{
	if (
		!n ||
		DivineAura() ||
		(!from_spell && !CombatRange(n)) ||
		(IsNPC() && IsCharmed())
	) {
		return;

	if (!always_succeed && IsClient()) {
		CastToClient()->CheckIncreaseSkill(Skill::Taunt, n, 10);
	}

	Mob* hate_top         = n->GetHateMost();
	int  level_difference = GetLevel() - n->GetLevel();
	bool success          = false;
	int  taunt_chance     = 0;

	// Support for how taunt worked pre 2000 on LIVE - Can not taunt NPC over your level.
	if (
		!RuleB(Combat, TauntOverLevel) &&
		level_difference < 0 ||
		n->GetSpecialAbility(IMMUNE_TAUNT)
	) {
		MessageString(Chat::SpellFailure, FAILED_TAUNT);
		return;
	}

	if (always_succeed) {
		taunt_chance = 100;
	}

	// Modern Taunt
	if (!RuleB(Combat, ClassicTauntSystem)) {
		if (
			(hate_top && hate_top->GetHPRatio() >= 20) ||
			!hate_top ||
			chance_bonus
		) {
			taunt_chance = chance_bonus ? chance_bonus : 50;
		} else {
			if (level_difference < 0) {
				taunt_chance += level_difference * 3;

				if (taunt_chance < 20) {
					taunt_chance = 20;
				}
			} else {
				taunt_chance += level_difference * 5;

				if (taunt_chance > 65) {
					taunt_chance = 65;
				}
			}

			if (IsClient() && !always_succeed) {
				taunt_chance -= (
					RuleR(Combat, TauntSkillFalloff) *
					(CastToClient()->MaxSkill(Skill::Taunt) - GetSkill(Skill::Taunt))
				);
			}

			if (taunt_chance < 1) {
				taunt_chance = 1;
			}
		}
	} else { // Classic Taunt
		if (GetLevel() >= 60 && level_difference < 0) {
			if (level_difference < -5) {
				taunt_chance = 0;
			} else if (level_difference == -5) {
				taunt_chance = 10;
			} else {
				taunt_chance = 50 + level_difference * 10;
			}
		} else {
			// this will make the skill difference between the tank classes actually affect success rates
			// but only for NPCs near the player's level.  Mid to low blues will start to taunt at 50%
			// even with lower skill
			taunt_chance = 50 * GetSkill(Skill::Taunt) / (n->GetLevel() * 5 + 5);
			taunt_chance += level_difference * 5;

			tauntchance /= 100.0f;

			success = tauntchance > zone->random.Real(0, 1);

	//success roll
	success = zone->random.Roll(taunt_chance);

	// Log result
	LogHate(
		"Taunter mob [{}] target npc [{}] taunt_chance [{}] success [{}] hate_top [{}]",
		GetMobDescription(),
		n->GetMobDescription(),
		taunt_chance,
		success ? "true" : "false",
		hate_top ? hate_top->GetMobDescription() : "not found"
	);

	// Actual Taunting
	if (success) {
		if (hate_top && hate_top != this) {
			int64 new_hate = (
				(n->GetNPCHate(hate_top) - n->GetNPCHate(this)) +
				bonus_hate +
				RuleI(Combat, TauntOverAggro) +
				1
			);

			LogHate(
				"Not Top Hate - Taunter [{}] Target [{}] Hated Top [{}] Hate Top Amt [{}] This Character Amt [{}] Bonus_Hate Amt [{}] TauntOverAggro Amt [{}] - Total [{}]",
				GetMobDescription(),
				n->GetMobDescription(),
				hate_top->GetMobDescription(),
				n->GetNPCHate(hate_top),
				n->GetNPCHate(this),
				bonus_hate,
				RuleI(Combat, TauntOverAggro),
				new_hate
			);

			n->CastToNPC()->AddToHateList(this, new_hate);
		} else {
			LogHate("Already Hate Top");
			n->CastToNPC()->AddToHateList(this, 12);
		}

		if (n->CanTalk()) {
			n->SayString(SUCCESSFUL_TAUNT, GetCleanName());
		}
	} else {
		MessageString(Chat::SpellFailure, FAILED_TAUNT);
	}

	// Modern Abilities
	if (!RuleB(Combat, ClassicTauntSystem)) {
		TryCastOnSkillUse(n, Skill::Taunt);

		if (HasSkillProcs()) {
			TrySkillProc(n, Skill::Taunt, Skill::Reuse::Taunt * 1000);
		}

		if (success && HasSkillProcSuccess()) {
			TrySkillProc(n, Skill::Taunt, Skill::Reuse::Taunt * 1000, true);
		}
	}
}

void Mob::InstillDoubt(Mob* m)
{
	if (!m || !m->IsNPC()) {
		return;
	}

	if (DivineAura()) {
		return;
	}

	//range check
	if (!CombatRange(m)) {
		return;
	}

	if (IsClient()) {
		CastToClient()->CheckIncreaseSkill(Skill::Intimidation, m, 10);
	}

	//I think this formula needs work
	int  value   = 0;
	bool success = false;

	//user's bonus
	value += GetSkill(Skill::Intimidation) + GetCHA() / 4;

	//target's counters
	value -= target->GetLevel()*4 + m->GetWIS() / 4;

	if (zone->random.Roll(value)) {
		//temporary hack...
		//cast fear on them... should prolly be a different spell
		//and should be un-resistable.
		SpellOnTarget(229, m, 0, true, -2000);
		success = true;
		//is there a success message?
	} else {
		MessageString(Chat::LightBlue,NOT_SCARING);
		//Idea from WR:
		/* if (target->IsNPC() && zone->random.Int(0,99) < 10 ) {
			entity_list.MessageClose(target, false, 50, Chat::NPCRampage, "%s lashes out in anger!",target->GetName());
			//should we actually do this? and the range is completely made up, unconfirmed
			entity_list.AEAttack(target, 50);
		}*/
	}

	TryCastOnSkillUse(m, Skill::Intimidation);

	if (HasSkillProcs()) {
		TrySkillProc(m, Skill::Intimidation, Skill::Reuse::InstillDoubt * 1000);
	}

	if (success && HasSkillProcSuccess()) {
		TrySkillProc(m, Skill::Intimidation, Skill::Reuse::InstillDoubt * 1000, true);
	}
}

int Mob::TryHeadShot(Mob* m, uint16 skill_id)
{
	// Only works on YOUR target.
	if (
		m &&
		!m->IsClient() &&
		skill_id == Skill::Archery &&
		GetTarget() == m &&
		(m->GetBodyType() == BT_Humanoid || !RuleB(Combat, HeadshotOnlyHumanoids)) &&
		!m->GetSpecialAbility(IMMUNE_HEADSHOT)
	) {
		const uint32 headshot_damage = (
			aabonuses.HeadShot[SBIndex::FINISHING_EFFECT_DMG] +
			spellbonuses.HeadShot[SBIndex::FINISHING_EFFECT_DMG] +
			itembonuses.HeadShot[SBIndex::FINISHING_EFFECT_DMG]
		);
		const uint8 headshot_level  = std::max(
			{
				aabonuses.HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX],
				spellbonuses.HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX],
				itembonuses.HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX]
			}
		);

		if (
			headshot_damage &&
			headshot_level &&
			m->GetLevel() <= headshot_level
		) {
			int chance = GetDEX();
			chance = 100 * chance / (chance + 3500);

			if (IsClient() || IsBot()) {
				chance += GetHeroicDEX() / 25;
			}

			chance *= 10;

			int headshot_level_bonus = aabonuses.HSLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS];
			if (headshot_level_bonus > 0) {
				chance = chance * headshot_level_bonus / 100;
			}

			chance += (
				aabonuses.HeadShot[SBIndex::FINISHING_EFFECT_PROC_CHANCE] +
				spellbonuses.HeadShot[SBIndex::FINISHING_EFFECT_PROC_CHANCE] +
				itembonuses.HeadShot[SBIndex::FINISHING_EFFECT_PROC_CHANCE]
			);

			if (zone->random.Int(1, 1000) <= chance) {
				entity_list.MessageCloseString(this, false, 200, Chat::MeleeCrit, FATAL_BOW_SHOT, GetName());
				return headshot_damage;
			}
		}
	}

	return 0;
}

int Mob::TryAssassinate(Mob* m, uint16 skill_id)
{
	if (
		m &&
		!m->IsClient() &&
		GetLevel() >= RuleI(Combat, AssassinateLevelRequirement) &&
		(skill_id == Skill::Backstab || skill_id == Skill::Throwing) &&
		(m->GetBodyType() == BT_Humanoid || !RuleB(Combat, AssassinateOnlyHumanoids)) &&
		!m->GetSpecialAbility(IMMUNE_ASSASSINATE)
	) {
		int chance = GetDEX();

		if (skill_id == Skill::Backstab) {
			chance = 100 * chance / (chance + 3500);

			if (IsClient() || IsBot()) {
				chance += GetHeroicDEX();
			}

			chance *= 10;
			int assassinate_level_bonus = aabonuses.AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_CHANCE_BONUS];
			if (assassinate_level_bonus > 0) {
				chance = chance * assassinate_level_bonus / 100;
			}
		} else if (skill_id == Skill::Throwing) {
			if (chance > 255) {
				chance = 260;
			} else {
				chance += 5;
			}
		}

		chance += (
			aabonuses.Assassinate[SBIndex::FINISHING_EFFECT_PROC_CHANCE] +
			spellbonuses.Assassinate[SBIndex::FINISHING_EFFECT_PROC_CHANCE] +
			itembonuses.Assassinate[SBIndex::FINISHING_EFFECT_PROC_CHANCE]
		);

		uint32 assassinate_damage_bonus = (
			aabonuses.Assassinate[SBIndex::FINISHING_EFFECT_DMG] +
			spellbonuses.Assassinate[SBIndex::FINISHING_EFFECT_DMG] +
			itembonuses.Assassinate[SBIndex::FINISHING_EFFECT_DMG]
		);

		uint8 assassinate_level = std::max(
			{
				aabonuses.AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX],
				spellbonuses.AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX],
				itembonuses.AssassinateLevel[SBIndex::FINISHING_EFFECT_LEVEL_MAX]
			}
		);

		// revamped AAs require AA line I believe?
		if (!assassinate_level) {
			return 0;
		}

		if (assassinate_damage_bonus && assassinate_level && (m->GetLevel() <= assassinate_level)) {
			if (zone->random.Int(1, 1000) <= chance) {
				entity_list.MessageCloseString(this, false, 200, Chat::MeleeCrit, ASSASSINATES, GetName());
				return assassinate_damage_bonus;
			}
		}
	}

	return 0;
}

void Mob::DoMeleeSkillAttackDmg(
	Mob* m,
	int32 weapon_damage,
	uint16 skill_id,
	int16 chance_mod,
	int16 focus,
	bool can_riposte,
	int reuse_time
)
{
	if (!CanDoSpecialAttack(m)) {
		return;
	}

	/*
		For spells using skill value 98 (feral swipe ect) server sets this to 67 automatically.
		Kayen: This is unlikely to be completely accurate but use OFFENSE skill value for these effects.
		TODO: We need to stop moving skill 98, it's suppose to just be a dummy skill AFAIK
		Spells using offense should use the skill of your primary, if you can use it, otherwise h2h
	*/
	if (skill_id == Skill::Begging) {
		skill_id = Skill::Offense;
	}

	int64 damage = Damage::None;
	int64 hate   = 0;
	if (hate == 0 && weapon_damage > 1) {
		hate = weapon_damage;
	}

	if (weapon_damage > Damage::None) {
		if (focus) {
			weapon_damage += weapon_damage * focus / 100;
		}

		if (skill_id == Skill::Bash && IsClient()) {
			EQ::ItemInstance *item =
				CastToClient()->GetInv().GetItem(EQ::invslot::slotSecondary);
			if (item) {
				if (item->GetItem()->ItemType == EQ::item::ItemTypeShield) {
					hate += item->GetItem()->AC;
				}
				const EQ::ItemData *itm = item->GetItem();
				hate = hate * (100 + GetFuriousBash(itm->Focus.Effect)) / 100;
			}
		}

		DamageHitInfo my_hit {};
		my_hit.base_damage = weapon_damage;
		my_hit.min_damage = 0;
		my_hit.damage_done = 1;
		my_hit.skill_id    = skill_id;
		my_hit.offense     = offense(my_hit.skill_id);
		my_hit.tohit       = GetTotalToHit(my_hit.skill_id, chance_mod);
		my_hit.hand        = can_riposte ? EQ::invslot::slotPrimary : EQ::invslot::slotRange;

		my_hit.skill = skillinuse;
		my_hit.offense = offense(my_hit.skill);
		my_hit.tohit = GetTotalToHit(my_hit.skill, chance_mod);
		// slot range exclude ripe etc ...
		my_hit.hand = CanRiposte ? EQ::invslot::slotRange : EQ::invslot::slotPrimary;

		if (IsNPC()) {
			my_hit.min_damage = CastToNPC()->GetMinDamage();
		}

		DoAttack(m, my_hit);
		damage = my_hit.damage_done;
	} else {
		damage = Damage::Invulnerable;
	}

	if (skill_id == Skill::Offense) {    // Hack to allow damage to display.
		skill_id = Skill::TigerClaw; //'strike' your opponent - Arbitrary choice for message.
	}

	m->AddToHateList(this, hate, 0);
	m->Damage(this, damage, SPELL_UNKNOWN, skill_id);

	if (HasDied()) {
		return;
	}

	TryCastOnSkillUse(m, skill_id);
}

bool Mob::CanDoSpecialAttack(Mob* m)
{
	if (!m) {
		SetTarget(nullptr);
		return false;
	}

	if (!GetTarget()) {
		SetTarget(m);
	}

	if (
		(
			HasDied() ||
			!m ||
			!IsAttackAllowed(m) ||
			(
				(IsClient() && CastToClient()->dead) ||
				(m->IsClient() && m->CastToClient()->dead)
			)
		)
	) {
		return false;
	}

	return !m->GetInvul() && !m->GetSpecialAbility(IMMUNE_MELEE);
}
