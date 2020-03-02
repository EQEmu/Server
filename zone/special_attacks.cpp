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
#include "../common/string_util.h"

#include "client.h"
#include "entity.h"
#include "mob.h"
#include "string_ids.h"
#include "lua_parser.h"

#include <string.h>

extern double frame_time;

int Mob::GetBaseSkillDamage(EQEmu::skills::SkillType skill, Mob *target)
{
	int base = EQEmu::skills::GetBaseDamage(skill);
	auto skill_level = GetSkill(skill);
	switch (skill) {
	case EQEmu::skills::SkillDragonPunch:
	case EQEmu::skills::SkillEagleStrike:
	case EQEmu::skills::SkillTigerClaw:
	case EQEmu::skills::SkillRoundKick:
		if (skill_level >= 25)
			base++;
		if (skill_level >= 75)
			base++;
		if (skill_level >= 125)
			base++;
		if (skill_level >= 175)
			base++;
		return base;
	case EQEmu::skills::SkillFrenzy:
		if (IsClient() && CastToClient()->GetInv().GetItem(EQEmu::invslot::slotPrimary)) {
			if (GetLevel() > 15)
				base += GetLevel() - 15;
			if (base > 23)
				base = 23;
			if (GetLevel() > 50)
				base += 2;
			if (GetLevel() > 54)
				base++;
			if (GetLevel() > 59)
				base++;
		}
		return base;
	case EQEmu::skills::SkillFlyingKick: {
		float skill_bonus = skill_level / 9.0f;
		float ac_bonus = 0.0f;
		if (IsClient()) {
			auto inst = CastToClient()->GetInv().GetItem(EQEmu::invslot::slotFeet);
			if (inst)
				ac_bonus = inst->GetItemArmorClass(true) / 25.0f;
		}
		if (ac_bonus > skill_bonus)
			ac_bonus = skill_bonus;
		return static_cast<int>(ac_bonus + skill_bonus);
	}
	case EQEmu::skills::SkillKick: {
		// there is some base *= 4 case in here?
		float skill_bonus = skill_level / 10.0f;
		float ac_bonus = 0.0f;
		if (IsClient()) {
			auto inst = CastToClient()->GetInv().GetItem(EQEmu::invslot::slotFeet);
			if (inst)
				ac_bonus = inst->GetItemArmorClass(true) / 25.0f;
		}
		if (ac_bonus > skill_bonus)
			ac_bonus = skill_bonus;
		return static_cast<int>(ac_bonus + skill_bonus);
	}
	case EQEmu::skills::SkillBash: {
		float skill_bonus = skill_level / 10.0f;
		float ac_bonus = 0.0f;
		const EQEmu::ItemInstance *inst = nullptr;
		if (IsClient()) {
			if (HasShieldEquiped())
				inst = CastToClient()->GetInv().GetItem(EQEmu::invslot::slotSecondary);
			else if (HasTwoHanderEquipped())
				inst = CastToClient()->GetInv().GetItem(EQEmu::invslot::slotPrimary);
		}
		if (inst)
			ac_bonus = inst->GetItemArmorClass(true) / 25.0f;
		else
			return 0; // return 0 in cases where we don't have an item
		if (ac_bonus > skill_bonus)
			ac_bonus = skill_bonus;
		return static_cast<int>(ac_bonus + skill_bonus);
	}
	case EQEmu::skills::SkillBackstab: {
		float skill_bonus = static_cast<float>(skill_level) * 0.02f;
		base = 3; // There seems to be a base 3 for NPCs or some how BS w/o weapon?
		// until we get a better inv system for NPCs they get nerfed!
		if (IsClient()) {
			auto *inst = CastToClient()->GetInv().GetItem(EQEmu::invslot::slotPrimary);
			if (inst && inst->GetItem() && inst->GetItem()->ItemType == EQEmu::item::ItemType1HPiercing) {
				base = inst->GetItemBackstabDamage(true);
				if (!inst->GetItemBackstabDamage())
					base += inst->GetItemWeaponDamage(true);
				if (target) {
					if (inst->GetItemElementalFlag(true) && inst->GetItemElementalDamage(true))
						base += target->ResistElementalWeaponDmg(inst);
					if (inst->GetItemBaneDamageBody(true) || inst->GetItemBaneDamageRace(true))
						base += target->CheckBaneDamage(inst);
				}
			}
		} else if (IsNPC()) {
			auto *npc = CastToNPC();
			base = std::max(base, npc->GetBaseDamage());
			// parses show relatively low BS mods from lots of NPCs, so either their BS skill is super low
			// or their mod is divided again, this is probably not the right mod, but it's better
			skill_bonus /= 3.0f;
		}
		// ahh lets make sure everything is casted right :P ugly but w/e
		return static_cast<int>(static_cast<float>(base) * (skill_bonus + 2.0f));
	}
	default:
		return 0;
	}
}

void Mob::DoSpecialAttackDamage(Mob *who, EQEmu::skills::SkillType skill, int32 base_damage, int32 min_damage,
				int32 hate_override, int ReuseTime)
{
	// this really should go through the same code as normal melee damage to
	// pick up all the special behavior there

	if ((who == nullptr ||
	     ((IsClient() && CastToClient()->dead) || (who->IsClient() && who->CastToClient()->dead)) || HasDied() ||
	     (!IsAttackAllowed(who))))
		return;

	DamageHitInfo my_hit;
	my_hit.damage_done = 1; // min 1 dmg
	my_hit.base_damage = base_damage;
	my_hit.min_damage = min_damage;
	my_hit.skill = skill;

	if (my_hit.base_damage == 0)
		my_hit.base_damage = GetBaseSkillDamage(my_hit.skill);

	if (who->GetInvul() || who->GetSpecialAbility(IMMUNE_MELEE))
		my_hit.damage_done = DMG_INVULNERABLE;

	if (who->GetSpecialAbility(IMMUNE_MELEE_EXCEPT_BANE) && skill != EQEmu::skills::SkillBackstab)
		my_hit.damage_done = DMG_INVULNERABLE;

	uint32 hate = my_hit.base_damage;
	if (hate_override > -1)
		hate = hate_override;

	if (skill == EQEmu::skills::SkillBash) {
		if (IsClient()) {
			EQEmu::ItemInstance *item = CastToClient()->GetInv().GetItem(EQEmu::invslot::slotSecondary);
			if (item) {
				if (item->GetItem()->ItemType == EQEmu::item::ItemTypeShield) {
					hate += item->GetItem()->AC;
				}
				const EQEmu::ItemData *itm = item->GetItem();
				auto fbash = GetFuriousBash(itm->Focus.Effect);
				hate = hate * (100 + fbash) / 100;
				if (fbash)
					MessageString(Chat::Spells, GLOWS_RED, itm->Name);
			}
		}
	}

	my_hit.offense = offense(my_hit.skill);
	my_hit.tohit = GetTotalToHit(my_hit.skill, 0);

	my_hit.hand = EQEmu::invslot::slotPrimary; // Avoid checks hand for throwing/archery exclusion, primary should
						  // work for most
	if (skill == EQEmu::skills::SkillThrowing || skill == EQEmu::skills::SkillArchery)
		my_hit.hand = EQEmu::invslot::slotRange;

	DoAttack(who, my_hit);

	who->AddToHateList(this, hate, 0, false);
	if (my_hit.damage_done > 0 && aabonuses.SkillAttackProc[0] && aabonuses.SkillAttackProc[1] == skill &&
	    IsValidSpell(aabonuses.SkillAttackProc[2])) {
		float chance = aabonuses.SkillAttackProc[0] / 1000.0f;
		if (zone->random.Roll(chance))
			SpellFinished(aabonuses.SkillAttackProc[2], who, EQEmu::spells::CastingSlot::Item, 0, -1,
				      spells[aabonuses.SkillAttackProc[2]].ResistDiff);
	}

	who->Damage(this, my_hit.damage_done, SPELL_UNKNOWN, skill, false);

	// Make sure 'this' has not killed the target and 'this' is not dead (Damage shield ect).
	if (!GetTarget())
		return;
	if (HasDied())
		return;

	if (HasSkillProcs())
		TrySkillProc(who, skill, ReuseTime * 1000);

	if (my_hit.damage_done > 0 && HasSkillProcSuccess())
		TrySkillProc(who, skill, ReuseTime * 1000, true);
}

// We should probably refactor this to take the struct not the packet
void Client::OPCombatAbility(const CombatAbility_Struct *ca_atk)
{
	if (!GetTarget())
		return;
	// make sure were actually able to use such an attack.
	if (spellend_timer.Enabled() || IsFeared() || IsStunned() || IsMezzed() || DivineAura() || dead)
		return;

	pTimerType timer = pTimerCombatAbility;
	// RoF2+ Tiger Claw is unlinked from other monk skills, if they ever do that for other classes there will need
	// to be more checks here
	if (ClientVersion() >= EQEmu::versions::ClientVersion::RoF2 && ca_atk->m_skill == EQEmu::skills::SkillTigerClaw)
		timer = pTimerCombatAbility2;


	bool CanBypassSkillCheck = false;

	if (ca_atk->m_skill == EQEmu::skills::SkillBash) { // SLAM - Bash without a shield equipped
		switch (GetRace())
		{
		case OGRE:
		case TROLL:
		case BARBARIAN:
			CanBypassSkillCheck = true;
		default:
			break;
		}
	}

	/* Check to see if actually have skill */
	if (!MaxSkill(static_cast<EQEmu::skills::SkillType>(ca_atk->m_skill)) && !CanBypassSkillCheck)
		return;

	if (GetTarget()->GetID() != ca_atk->m_target)
		return; // invalid packet.

	if (!IsAttackAllowed(GetTarget()))
		return;

	// These two are not subject to the combat ability timer, as they
	// allready do their checking in conjunction with the attack timer
	// throwing weapons
	if (ca_atk->m_atk == EQEmu::invslot::slotRange) {
		if (ca_atk->m_skill == EQEmu::skills::SkillThrowing) {
			SetAttackTimer();
			ThrowingAttack(GetTarget());
			if (CheckDoubleRangedAttack())
				ThrowingAttack(GetTarget(), true);
			return;
		}
		// ranged attack (archery)
		if (ca_atk->m_skill == EQEmu::skills::SkillArchery) {
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

	int ReuseTime = 0;
	int ClientHaste = GetHaste();
	int HasteMod = 0;

	if (ClientHaste >= 0)
		HasteMod = (10000 / (100 + ClientHaste)); //+100% haste = 2x as many attacks
	else
		HasteMod = (100 - ClientHaste); //-100% haste = 1/2 as many attacks

	int32 dmg = 0;

	int32 skill_reduction = this->GetSkillReuseTime(ca_atk->m_skill);

	// not sure what the '100' indicates..if ->m_atk is not used as 'slot' reference, then change SlotRange above back to '11'
	if (ca_atk->m_atk == 100 &&
	    ca_atk->m_skill == EQEmu::skills::SkillBash) { // SLAM - Bash without a shield equipped
		if (GetTarget() != this) {

			CheckIncreaseSkill(EQEmu::skills::SkillBash, GetTarget(), 10);
			DoAnim(animTailRake, 0, false);

			int32 ht = 0;
			if (GetWeaponDamage(GetTarget(), GetInv().GetItem(EQEmu::invslot::slotSecondary)) <= 0 &&
			    GetWeaponDamage(GetTarget(), GetInv().GetItem(EQEmu::invslot::slotShoulders)) <= 0)
				dmg = -5;
			else
				ht = dmg = GetBaseSkillDamage(EQEmu::skills::SkillBash, GetTarget());

			ReuseTime = BashReuseTime - 1 - skill_reduction;
			ReuseTime = (ReuseTime * HasteMod) / 100;
			DoSpecialAttackDamage(GetTarget(), EQEmu::skills::SkillBash, dmg, 0, ht, ReuseTime);
			if (ReuseTime > 0)
				p_timers.Start(timer, ReuseTime);
		}
		return;
	}

	if (ca_atk->m_atk == 100 && ca_atk->m_skill == EQEmu::skills::SkillFrenzy) {
		CheckIncreaseSkill(EQEmu::skills::SkillFrenzy, GetTarget(), 10);
		int AtkRounds = 1;
		int32 max_dmg = GetBaseSkillDamage(EQEmu::skills::SkillFrenzy, GetTarget());
		DoAnim(anim2HSlashing, 0, false);

		max_dmg = mod_frenzy_damage(max_dmg);

		if (GetClass() == BERSERKER) {
			int chance = GetLevel() * 2 + GetSkill(EQEmu::skills::SkillFrenzy);
			if (zone->random.Roll0(450) < chance)
				AtkRounds++;
			if (zone->random.Roll0(450) < chance)
				AtkRounds++;
		}

		ReuseTime = FrenzyReuseTime - 1 - skill_reduction;
		ReuseTime = (ReuseTime * HasteMod) / 100;

		while (AtkRounds > 0) {
			if (GetTarget())
				DoSpecialAttackDamage(GetTarget(), EQEmu::skills::SkillFrenzy, max_dmg, 0, max_dmg, ReuseTime);
			AtkRounds--;
		}

		if (ReuseTime > 0)
			p_timers.Start(timer, ReuseTime);
		return;
	}

	switch (GetClass()) {
	case BERSERKER:
	case WARRIOR:
	case RANGER:
	case BEASTLORD:
		if (ca_atk->m_atk != 100 || ca_atk->m_skill != EQEmu::skills::SkillKick)
			break;
		if (GetTarget() != this) {
			CheckIncreaseSkill(EQEmu::skills::SkillKick, GetTarget(), 10);
			DoAnim(animKick, 0, false);

			int32 ht = 0;
			if (GetWeaponDamage(GetTarget(), GetInv().GetItem(EQEmu::invslot::slotFeet)) <= 0)
				dmg = -5;
			else
				ht = dmg = GetBaseSkillDamage(EQEmu::skills::SkillKick, GetTarget());

			ReuseTime = KickReuseTime - 1 - skill_reduction;
			DoSpecialAttackDamage(GetTarget(), EQEmu::skills::SkillKick, dmg, 0, ht, ReuseTime);
		}
		break;
	case MONK: {
		ReuseTime = MonkSpecialAttack(GetTarget(), ca_atk->m_skill) - 1 - skill_reduction;

		// Live AA - Technique of Master Wu
		int wuchance = itembonuses.DoubleSpecialAttack + spellbonuses.DoubleSpecialAttack + aabonuses.DoubleSpecialAttack;

		if (wuchance) {
			const int MonkSPA[5] = {
				EQEmu::skills::SkillFlyingKick,
				EQEmu::skills::SkillDragonPunch,
				EQEmu::skills::SkillEagleStrike,
				EQEmu::skills::SkillTigerClaw,
				EQEmu::skills::SkillRoundKick
			};
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
			// They didn't add a string ID for this.
			std::string msg = StringFormat("The spirit of Master Wu fills you!  You gain %d additional attack(s).", extra);
			// live uses 400 here -- not sure if it's the best for all clients though
			SendColoredText(400, msg);
			auto classic = RuleB(Combat, ClassicMasterWu);
			while (extra) {
				MonkSpecialAttack(GetTarget(), (classic ? MonkSPA[zone->random.Int(0, 4)] : ca_atk->m_skill));
				--extra;
			}
		}

		if (ReuseTime < 100) {
			// hackish... but we return a huge reuse time if this is an
			// invalid skill, otherwise, we can safely assume it is a
			// valid monk skill and just cast it to a SkillType
			CheckIncreaseSkill((EQEmu::skills::SkillType)ca_atk->m_skill, GetTarget(), 10);
		}
		break;
	}
	case ROGUE: {
		if (ca_atk->m_atk != 100 || ca_atk->m_skill != EQEmu::skills::SkillBackstab)
			break;
		ReuseTime = BackstabReuseTime-1 - skill_reduction;
		TryBackstab(GetTarget(), ReuseTime);
		break;
	}
	default:
		//they have no abilities... wtf? make em wait a bit
		ReuseTime = 9 - skill_reduction;
		break;
	}

	ReuseTime = (ReuseTime * HasteMod) / 100;
	if (ReuseTime > 0) {
		p_timers.Start(timer, ReuseTime);
	}
}

//returns the reuse time in sec for the special attack used.
int Mob::MonkSpecialAttack(Mob *other, uint8 unchecked_type)
{
	if (!other)
		return 0;

	int32 ndamage = 0;
	int32 max_dmg = 0;
	int32 min_dmg = 0;
	int reuse = 0;
	EQEmu::skills::SkillType skill_type; // to avoid casting... even though it "would work"
	uint8 itemslot = EQEmu::invslot::slotFeet;
	if (IsNPC()) {
		auto *npc = CastToNPC();
		min_dmg = npc->GetMinDamage();
	}

	switch (unchecked_type) {
	case EQEmu::skills::SkillFlyingKick:
		skill_type = EQEmu::skills::SkillFlyingKick;
		max_dmg = GetBaseSkillDamage(skill_type);
		min_dmg = 0; // revamped FK formula is missing the min mod?
		DoAnim(animFlyingKick, 0, false);
		reuse = FlyingKickReuseTime;
		break;
	case EQEmu::skills::SkillDragonPunch:
		skill_type = EQEmu::skills::SkillDragonPunch;
		max_dmg = GetBaseSkillDamage(skill_type);
		itemslot = EQEmu::invslot::slotHands;
		DoAnim(animTailRake, 0, false);
		reuse = TailRakeReuseTime;
		break;
	case EQEmu::skills::SkillEagleStrike:
		skill_type = EQEmu::skills::SkillEagleStrike;
		max_dmg = GetBaseSkillDamage(skill_type);
		itemslot = EQEmu::invslot::slotHands;
		DoAnim(animEagleStrike, 0, false);
		reuse = EagleStrikeReuseTime;
		break;
	case EQEmu::skills::SkillTigerClaw:
		skill_type = EQEmu::skills::SkillTigerClaw;
		max_dmg = GetBaseSkillDamage(skill_type);
		itemslot = EQEmu::invslot::slotHands;
		DoAnim(animTigerClaw, 0, false);
		reuse = TigerClawReuseTime;
		break;
	case EQEmu::skills::SkillRoundKick:
		skill_type = EQEmu::skills::SkillRoundKick;
		max_dmg = GetBaseSkillDamage(skill_type);
		DoAnim(animRoundKick, 0, false);
		reuse = RoundKickReuseTime;
		break;
	case EQEmu::skills::SkillKick:
		skill_type = EQEmu::skills::SkillKick;
		max_dmg = GetBaseSkillDamage(skill_type);
		DoAnim(animKick, 0, false);
		reuse = KickReuseTime;
		break;
	default:
		Log(Logs::Detail, Logs::Attack, "Invalid special attack type %d attempted", unchecked_type);
		return (1000); /* nice long delay for them, the caller depends on this! */
	}

	if (IsClient()) {
		if (GetWeaponDamage(other, CastToClient()->GetInv().GetItem(itemslot)) <= 0) {
			max_dmg = DMG_INVULNERABLE;
		}
	} else {
		if (GetWeaponDamage(other, (const EQEmu::ItemData *)nullptr) <= 0) {
			max_dmg = DMG_INVULNERABLE;
		}
	}

	int32 ht = 0;
	if (max_dmg > 0)
		ht = max_dmg;

	// This can potentially stack with changes to kick damage
	ht = ndamage = mod_monk_special_damage(ndamage, skill_type);

	DoSpecialAttackDamage(other, skill_type, max_dmg, min_dmg, ht, reuse);

	return reuse;
}

void Mob::TryBackstab(Mob *other, int ReuseTime) {
	if(!other)
		return;

	bool bIsBehind = false;
	bool bCanFrontalBS = false;

	//make sure we have a proper weapon if we are a client.
	if(IsClient()) {
		const EQEmu::ItemInstance *wpn = CastToClient()->GetInv().GetItem(EQEmu::invslot::slotPrimary);
		if (!wpn || (wpn->GetItem()->ItemType != EQEmu::item::ItemType1HPiercing)){
			MessageString(Chat::Red, BACKSTAB_WEAPON);
			return;
		}
	}

	//Live AA - Triple Backstab
	int tripleChance = itembonuses.TripleBackstab + spellbonuses.TripleBackstab + aabonuses.TripleBackstab;

	if (BehindMob(other, GetX(), GetY()))
		bIsBehind = true;

	else {
		//Live AA - Seized Opportunity
		int FrontalBSChance = itembonuses.FrontalBackstabChance + spellbonuses.FrontalBackstabChance + aabonuses.FrontalBackstabChance;

		if (FrontalBSChance && zone->random.Roll(FrontalBSChance))
			bCanFrontalBS = true;
	}

	if (bIsBehind || bCanFrontalBS || (IsNPC() && CanFacestab())) { // Player is behind other OR can do Frontal Backstab
		if (bCanFrontalBS && IsClient()) // I don't think there is any message ...
			CastToClient()->Message(Chat::White,"Your fierce attack is executed with such grace, your target did not see it coming!");

		RogueBackstab(other,false,ReuseTime);
		if (level > 54) {
			// TODO: 55-59 doesn't appear to match just checking double attack, 60+ does though
			if(IsClient() && CastToClient()->CheckDoubleAttack())
			{
				if(other->GetHP() > 0)
					RogueBackstab(other,false,ReuseTime);

				if (tripleChance && other->GetHP() > 0 && zone->random.Roll(tripleChance))
					RogueBackstab(other,false,ReuseTime);
			}
		}

		if(IsClient())
			CastToClient()->CheckIncreaseSkill(EQEmu::skills::SkillBackstab, other, 10);

	}
	//Live AA - Chaotic Backstab
	else if(aabonuses.FrontalBackstabMinDmg || itembonuses.FrontalBackstabMinDmg || spellbonuses.FrontalBackstabMinDmg) {
		m_specialattacks = eSpecialAttacks::ChaoticStab;

		//we can stab from any angle, we do min damage though.
		// chaotic backstab can't double etc Seized can, but that's because it's a chance to do normal BS
		// Live actually added SPA 473 which grants chance to double here when they revamped chaotic/seized
		RogueBackstab(other, true, ReuseTime);
		if(IsClient())
			CastToClient()->CheckIncreaseSkill(EQEmu::skills::SkillBackstab, other, 10);
		m_specialattacks = eSpecialAttacks::None;
	}
	else { //We do a single regular attack if we attack from the front without chaotic stab
		Attack(other, EQEmu::invslot::slotPrimary);
	}
}

//heko: backstab
void Mob::RogueBackstab(Mob* other, bool min_damage, int ReuseTime)
{
	if (!other)
		return;

	uint32 hate = 0;

	// make sure we can hit (bane, magical, etc)
	if (IsClient()) {
		const EQEmu::ItemInstance *wpn = CastToClient()->GetInv().GetItem(EQEmu::invslot::slotPrimary);
		if (!GetWeaponDamage(other, wpn))
			return;
	} else if (!GetWeaponDamage(other, (const EQEmu::ItemData*)nullptr)){
		return;
	}

	int base_damage = GetBaseSkillDamage(EQEmu::skills::SkillBackstab, other);
	hate = base_damage;

	DoSpecialAttackDamage(other, EQEmu::skills::SkillBackstab, base_damage, 0, hate, ReuseTime);
	DoAnim(anim1HPiercing, 0, false);
}

// assassinate [No longer used for regular assassinate 6-29-14]
void Mob::RogueAssassinate(Mob* other)
{
	//can you dodge, parry, etc.. an assassinate??
	//if so, use DoSpecialAttackDamage(other, BACKSTAB, 32000); instead
	if (GetWeaponDamage(other, IsClient() ? CastToClient()->GetInv().GetItem(EQEmu::invslot::slotPrimary) : (const EQEmu::ItemInstance*)nullptr) > 0){
		other->Damage(this, 32000, SPELL_UNKNOWN, EQEmu::skills::SkillBackstab);
	}else{
		other->Damage(this, -5, SPELL_UNKNOWN, EQEmu::skills::SkillBackstab);
	}
	DoAnim(anim1HPiercing, 0, false);	//piercing animation
}

void Client::RangedAttack(Mob* other, bool CanDoubleAttack) {
	//conditions to use an attack checked before we are called
	if (!other)
		return;
	//make sure the attack and ranged timers are up
	//if the ranged timer is disabled, then they have no ranged weapon and shouldent be attacking anyhow
	if(!CanDoubleAttack && ((attack_timer.Enabled() && !attack_timer.Check(false)) || (ranged_timer.Enabled() && !ranged_timer.Check()))) {
		LogCombat("Throwing attack canceled. Timer not up. Attack [{}], ranged [{}]", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		// The server and client timers are not exact matches currently, so this would spam too often if enabled
		//Message(0, "Error: Timer not up. Attack %d, ranged %d", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		return;
	}
	const EQEmu::ItemInstance* RangeWeapon = m_inv[EQEmu::invslot::slotRange];

	//locate ammo
	int ammo_slot = EQEmu::invslot::slotAmmo;
	const EQEmu::ItemInstance* Ammo = m_inv[EQEmu::invslot::slotAmmo];

	if (!RangeWeapon || !RangeWeapon->IsClassCommon()) {
		LogCombat("Ranged attack canceled. Missing or invalid ranged weapon ([{}]) in slot [{}]", GetItemIDAt(EQEmu::invslot::slotRange), EQEmu::invslot::slotRange);
		Message(0, "Error: Rangeweapon: GetItem(%i)==0, you have no bow!", GetItemIDAt(EQEmu::invslot::slotRange));
		return;
	}
	if (!Ammo || !Ammo->IsClassCommon()) {
		LogCombat("Ranged attack canceled. Missing or invalid ammo item ([{}]) in slot [{}]", GetItemIDAt(EQEmu::invslot::slotAmmo), EQEmu::invslot::slotAmmo);
		Message(0, "Error: Ammo: GetItem(%i)==0, you have no ammo!", GetItemIDAt(EQEmu::invslot::slotAmmo));
		return;
	}

	const EQEmu::ItemData* RangeItem = RangeWeapon->GetItem();
	const EQEmu::ItemData* AmmoItem = Ammo->GetItem();

	if (RangeItem->ItemType != EQEmu::item::ItemTypeBow) {
		LogCombat("Ranged attack canceled. Ranged item is not a bow. type [{}]", RangeItem->ItemType);
		Message(0, "Error: Rangeweapon: Item %d is not a bow.", RangeWeapon->GetID());
		return;
	}
	if (AmmoItem->ItemType != EQEmu::item::ItemTypeArrow) {
		LogCombat("Ranged attack canceled. Ammo item is not an arrow. type [{}]", AmmoItem->ItemType);
		Message(0, "Error: Ammo: type %d != %d, you have the wrong type of ammo!", AmmoItem->ItemType, EQEmu::item::ItemTypeArrow);
		return;
	}

	LogCombat("Shooting [{}] with bow [{}] ([{}]) and arrow [{}] ([{}])", other->GetName(), RangeItem->Name, RangeItem->ID, AmmoItem->Name, AmmoItem->ID);

	//look for ammo in inventory if we only have 1 left...
	if(Ammo->GetCharges() == 1) {
		//first look for quivers
		int r;
		bool found = false;
		for (r = EQEmu::invslot::GENERAL_BEGIN; r <= EQEmu::invslot::GENERAL_END; r++) {
			const EQEmu::ItemInstance *pi = m_inv[r];
			if (pi == nullptr || !pi->IsClassBag())
				continue;
			const EQEmu::ItemData* bagitem = pi->GetItem();
			if (!bagitem || bagitem->BagType != EQEmu::item::BagTypeQuiver)
				continue;

			//we found a quiver, look for the ammo in it
			int i;
			for (i = 0; i < bagitem->BagSlots; i++) {
				EQEmu::ItemInstance* baginst = pi->GetItem(i);
				if(!baginst)
					continue;	//empty
				if(baginst->GetID() == Ammo->GetID()) {
					//we found it... use this stack
					//the item wont change, but the instance does
					Ammo = baginst;
					ammo_slot = m_inv.CalcSlotId(r, i);
					found = true;
					LogCombat("Using ammo from quiver stack at slot [{}]. [{}] in stack", ammo_slot, Ammo->GetCharges());
					break;
				}
			}
			if(found)
				break;
		}

		if(!found) {
			//if we dont find a quiver, look through our inventory again
			//not caring if the thing is a quiver.
			int32 aslot = m_inv.HasItem(AmmoItem->ID, 1, invWherePersonal);
			if (aslot != INVALID_INDEX) {
				ammo_slot = aslot;
				Ammo = m_inv[aslot];
				LogCombat("Using ammo from inventory stack at slot [{}]. [{}] in stack", ammo_slot, Ammo->GetCharges());
			}
		}
	}

	float range = RangeItem->Range + AmmoItem->Range + GetRangeDistTargetSizeMod(GetTarget());
	LogCombat("Calculated bow range to be [{}]", range);
	range *= range;
	float dist = DistanceSquared(m_Position, other->GetPosition());
	if(dist > range) {
		LogCombat("Ranged attack out of range client should catch this. ([{}] > [{}]).\n", dist, range);
		MessageString(Chat::Red,TARGET_OUT_OF_RANGE);//Client enforces range and sends the message, this is a backup just incase.
		return;
	}
	else if(dist < (RuleI(Combat, MinRangedAttackDist)*RuleI(Combat, MinRangedAttackDist))){
		MessageString(Chat::Yellow,RANGED_TOO_CLOSE);//Client enforces range and sends the message, this is a backup just incase.
		return;
	}

	if(!IsAttackAllowed(other) ||
		IsCasting() ||
		IsSitting() ||
		(DivineAura() && !GetGM()) ||
		IsStunned() ||
		IsFeared() ||
		IsMezzed() ||
		(GetAppearance() == eaDead)){
		return;
	}

	//Shoots projectile and/or applies the archery damage
	DoArcheryAttackDmg(other, RangeWeapon, Ammo,0,0,0,0,0,0, AmmoItem, ammo_slot);

	//EndlessQuiver AA base1 = 100% Chance to avoid consumption arrow.
	int ChanceAvoidConsume = aabonuses.ConsumeProjectile + itembonuses.ConsumeProjectile + spellbonuses.ConsumeProjectile;

	if (RangeItem->ExpendableArrow || !ChanceAvoidConsume || (ChanceAvoidConsume < 100 && zone->random.Int(0,99) > ChanceAvoidConsume)){
		DeleteItemInInventory(ammo_slot, 1, true);
		LogCombat("Consumed one arrow from slot [{}]", ammo_slot);
	} else {
		LogCombat("Endless Quiver prevented ammo consumption");
	}

	CheckIncreaseSkill(EQEmu::skills::SkillArchery, GetTarget(), -15);
	CommonBreakInvisibleFromCombat();
}

void Mob::DoArcheryAttackDmg(Mob *other, const EQEmu::ItemInstance *RangeWeapon, const EQEmu::ItemInstance *Ammo,
			     uint16 weapon_damage, int16 chance_mod, int16 focus, int ReuseTime, uint32 range_id,
			     uint32 ammo_id, const EQEmu::ItemData *AmmoItem, int AmmoSlot, float speed)
{
	if ((other == nullptr ||
	     ((IsClient() && CastToClient()->dead) || (other->IsClient() && other->CastToClient()->dead)) ||
	     HasDied() || (!IsAttackAllowed(other)) || (other->GetInvul() || other->GetSpecialAbility(IMMUNE_MELEE)))) {
		return;
	}

	const EQEmu::ItemInstance *_RangeWeapon = nullptr;
	const EQEmu::ItemInstance *_Ammo = nullptr;
	const EQEmu::ItemData *ammo_lost = nullptr;

	/*
	If LaunchProjectile is false this function will do archery damage on target,
	otherwise it will shoot the projectile at the target, once the projectile hits target
	this function is then run again to do the damage portion
	*/
	bool LaunchProjectile = false;

	if (RuleB(Combat, ProjectileDmgOnImpact)) {
		if (AmmoItem) { // won't be null when we are firing the arrow
			LaunchProjectile = true;
		} else {
			/*
			Item sync check on projectile landing.
			Weapon damage is already calculated so this only affects procs!
			Ammo proc check will use database to find proc if you used up your last ammo.
			If you change range item mid projectile flight, you loose your chance to proc from bow (Deal
			with it!).
			*/

			if (!RangeWeapon && !Ammo && range_id && ammo_id) {
				if (IsClient()) {
					_RangeWeapon = CastToClient()->m_inv[EQEmu::invslot::slotRange];
					if (_RangeWeapon && _RangeWeapon->GetItem() &&
					    _RangeWeapon->GetItem()->ID == range_id)
						RangeWeapon = _RangeWeapon;

					_Ammo = CastToClient()->m_inv[AmmoSlot];
					if (_Ammo && _Ammo->GetItem() && _Ammo->GetItem()->ID == ammo_id)
						Ammo = _Ammo;
					else
						ammo_lost = database.GetItem(ammo_id);
				}
			}
		}
	} else if (AmmoItem) {
		SendItemAnimation(other, AmmoItem, EQEmu::skills::SkillArchery);
	}

	LogCombat("Ranged attack hit [{}]", other->GetName());

	uint32 hate = 0;
	int TotalDmg = 0;
	int WDmg = 0;
	int ADmg = 0;
	if (!weapon_damage) {
		WDmg = GetWeaponDamage(other, RangeWeapon);
		ADmg = GetWeaponDamage(other, Ammo);
	} else {
		WDmg = weapon_damage;
	}

	if (LaunchProjectile) { // 1: Shoot the Projectile once we calculate weapon damage.
		TryProjectileAttack(other, AmmoItem, EQEmu::skills::SkillArchery, (WDmg + ADmg), RangeWeapon,
				    Ammo, AmmoSlot, speed);
		return;
	}

	// unsure when this should happen
	if (focus) // From FcBaseEffects
		WDmg += WDmg * focus / 100;

	if (WDmg > 0 || ADmg > 0) {
		if (WDmg < 0)
			WDmg = 0;
		if (ADmg < 0)
			ADmg = 0;
		int MaxDmg = WDmg + ADmg;
		hate = ((WDmg + ADmg));

		if (RuleB(Combat, ProjectileDmgOnImpact)) {
			LogCombat("Bow and Arrow DMG [{}], Max Damage [{}]", WDmg, MaxDmg);
		}
		else {
			LogCombat("Bow DMG [{}], Arrow DMG [{}], Max Damage [{}]", WDmg, ADmg, MaxDmg);
		}

		if (MaxDmg == 0)
			MaxDmg = 1;

		DamageHitInfo my_hit;
		my_hit.base_damage = MaxDmg;
		my_hit.min_damage = 0;
		my_hit.damage_done = 1;

		my_hit.skill = EQEmu::skills::SkillArchery;
		my_hit.offense = offense(my_hit.skill);
		my_hit.tohit = GetTotalToHit(my_hit.skill, chance_mod);
		my_hit.hand = EQEmu::invslot::slotRange;

		DoAttack(other, my_hit);
		TotalDmg = my_hit.damage_done;
	} else {
		TotalDmg = DMG_INVULNERABLE;
	}

	if (IsClient() && !CastToClient()->GetFeigned())
		other->AddToHateList(this, hate, 0, false);

	other->Damage(this, TotalDmg, SPELL_UNKNOWN, EQEmu::skills::SkillArchery);

	// Skill Proc Success
	if (TotalDmg > 0 && HasSkillProcSuccess() && other && !other->HasDied()) {
		if (ReuseTime)
			TrySkillProc(other, EQEmu::skills::SkillArchery, ReuseTime);
		else
			TrySkillProc(other, EQEmu::skills::SkillArchery, 0, true, EQEmu::invslot::slotRange);
	}
	// end of old fuck

	if (LaunchProjectile)
		return; // Shouldn't reach this point durring initial launch phase, but just in case.

	// Weapon Proc
	if (RangeWeapon && other && !other->HasDied())
		TryWeaponProc(RangeWeapon, other, EQEmu::invslot::slotRange);

	// Ammo Proc
	if (ammo_lost)
		TryWeaponProc(nullptr, ammo_lost, other, EQEmu::invslot::slotRange);
	else if (Ammo && other && !other->HasDied())
		TryWeaponProc(Ammo, other, EQEmu::invslot::slotRange);

	// Skill Proc
	if (HasSkillProcs() && other && !other->HasDied()) {
		if (ReuseTime)
			TrySkillProc(other, EQEmu::skills::SkillArchery, ReuseTime);
		else
			TrySkillProc(other, EQEmu::skills::SkillArchery, 0, false, EQEmu::invslot::slotRange);
	}
}

bool Mob::TryProjectileAttack(Mob *other, const EQEmu::ItemData *item, EQEmu::skills::SkillType skillInUse,
			      uint16 weapon_dmg, const EQEmu::ItemInstance *RangeWeapon,
			      const EQEmu::ItemInstance *Ammo, int AmmoSlot, float speed)
{
	if (!other)
		return false;

	int slot = -1;

	// Make sure there is an avialable slot.
	for (int i = 0; i < MAX_SPELL_PROJECTILE; i++) {
		if (ProjectileAtk[i].target_id == 0) {
			slot = i;
			break;
		}
	}

	if (slot < 0)
		return false;

	float speed_mod = speed;

	float distance = other->CalculateDistance(GetX(), GetY(), GetZ());
	float hit =
	    1200.0f + (10 * distance / speed_mod); // Calcuation: 60 = Animation Lag, 1.8 = Speed modifier for speed of (4)

	ProjectileAtk[slot].increment = 1;
	ProjectileAtk[slot].hit_increment = static_cast<uint16>(hit); // This projected hit time if target does NOT MOVE
	ProjectileAtk[slot].target_id = other->GetID();
	ProjectileAtk[slot].wpn_dmg = weapon_dmg;
	ProjectileAtk[slot].origin_x = GetX();
	ProjectileAtk[slot].origin_y = GetY();
	ProjectileAtk[slot].origin_z = GetZ();

	if (RangeWeapon && RangeWeapon->GetItem())
		ProjectileAtk[slot].ranged_id = RangeWeapon->GetItem()->ID;

	if (Ammo && Ammo->GetItem())
		ProjectileAtk[slot].ammo_id = Ammo->GetItem()->ID;

	ProjectileAtk[slot].ammo_slot = 0;
	ProjectileAtk[slot].skill = skillInUse;
	ProjectileAtk[slot].speed_mod = speed_mod;

	SetProjectileAttack(true);

	if (item)
		SendItemAnimation(other, item, skillInUse, speed);
	// else if (IsNPC())
	// ProjectileAnimation(other, 0,false,speed,0,0,0,CastToNPC()->GetAmmoIDfile(),skillInUse);

	return true;
}

void Mob::ProjectileAttack()
{
	if (!HasProjectileAttack())
		return;

	Mob *target = nullptr;
	bool disable = true;

	for (int i = 0; i < MAX_SPELL_PROJECTILE; i++) {
		if (ProjectileAtk[i].increment == 0)
			continue;

		disable = false;
		Mob *target = entity_list.GetMobID(ProjectileAtk[i].target_id);

		if (target && target->IsMoving()) { // Only recalculate hit increment if target moving
			// Due to frequency that we need to check increment the targets position variables may not be
			// updated even if moving. Do a simple check before calculating distance.
			if (ProjectileAtk[i].tlast_x != target->GetX() || ProjectileAtk[i].tlast_y != target->GetY()) {
				ProjectileAtk[i].tlast_x = target->GetX();
				ProjectileAtk[i].tlast_y = target->GetY();
				float distance = target->CalculateDistance(
				    ProjectileAtk[i].origin_x, ProjectileAtk[i].origin_y, ProjectileAtk[i].origin_z);
				float hit = 1200.0f + (10 * distance / ProjectileAtk[i].speed_mod); // Calcuation: 60 =
											     // Animation Lag, 1.8 =
											     // Speed modifier for speed
											     // of (4)
				ProjectileAtk[i].hit_increment = static_cast<uint16>(hit);
			}
		}

		// We hit I guess?
		if (ProjectileAtk[i].hit_increment <= ProjectileAtk[i].increment) {
			if (target) {
				if (IsNPC()) {
					if (ProjectileAtk[i].skill == EQEmu::skills::SkillConjuration) {
						if (IsValidSpell(ProjectileAtk[i].wpn_dmg))
							SpellOnTarget(ProjectileAtk[i].wpn_dmg, target, false, true,
								      spells[ProjectileAtk[i].wpn_dmg].ResistDiff,
								      true);
					} else {
						CastToNPC()->DoRangedAttackDmg(
						    target, false, ProjectileAtk[i].wpn_dmg, 0,
						    static_cast<EQEmu::skills::SkillType>(ProjectileAtk[i].skill));
					}
				} else {
					if (ProjectileAtk[i].skill == EQEmu::skills::SkillArchery)
						DoArcheryAttackDmg(target, nullptr, nullptr, ProjectileAtk[i].wpn_dmg,
								   0, 0, 0, ProjectileAtk[i].ranged_id,
								   ProjectileAtk[i].ammo_id, nullptr,
								   ProjectileAtk[i].ammo_slot);
					else if (ProjectileAtk[i].skill == EQEmu::skills::SkillThrowing)
						DoThrowingAttackDmg(target, nullptr, nullptr, ProjectileAtk[i].wpn_dmg,
								    0, 0, 0, ProjectileAtk[i].ranged_id,
								    ProjectileAtk[i].ammo_slot);
					else if (ProjectileAtk[i].skill == EQEmu::skills::SkillConjuration &&
						 IsValidSpell(ProjectileAtk[i].wpn_dmg))
						SpellOnTarget(ProjectileAtk[i].wpn_dmg, target, false, true,
							      spells[ProjectileAtk[i].wpn_dmg].ResistDiff, true);
				}
			}

			ProjectileAtk[i].increment = 0;
			ProjectileAtk[i].target_id = 0;
			ProjectileAtk[i].wpn_dmg = 0;
			ProjectileAtk[i].origin_x = 0.0f;
			ProjectileAtk[i].origin_y = 0.0f;
			ProjectileAtk[i].origin_z = 0.0f;
			ProjectileAtk[i].tlast_x = 0.0f;
			ProjectileAtk[i].tlast_y = 0.0f;
			ProjectileAtk[i].ranged_id = 0;
			ProjectileAtk[i].ammo_id = 0;
			ProjectileAtk[i].ammo_slot = 0;
			ProjectileAtk[i].skill = 0;
			ProjectileAtk[i].speed_mod = 0.0f;
		} else {
			ProjectileAtk[i].increment += 1000 * frame_time;
		}
	}

	if (disable)
		SetProjectileAttack(false);
}

float Mob::GetRangeDistTargetSizeMod(Mob* other)
{
	/*
	Range is enforced client side, therefore these numbers do not need to be 100% accurate just close enough to
	prevent any exploitation. The range mod changes in some situations depending on if size is from spawn or from SendIllusionPacket changes.
	At present time only calculate from spawn (it is no consistent what happens to the calc when changing it after spawn).
	*/
	if (!other)
		return 0.0f;

	float tsize = other->GetSize();

	if (GetSize() > tsize)
		tsize = GetSize();

	float mod = 0.0f;
	/*These are correct numbers if mob size is changed via #size (Don't know why it matters but it does)
	if (tsize < 7)
		mod = 16.0f;
	else if (tsize >=7 && tsize <= 20)
		mod = 16.0f + (0.6f * (tsize - 6.0f));
	else if (tsize >=20 && tsize <= 60)
		mod = 25.0f + (1.25f * (tsize - 20.0f));
	else
		mod = 75.0f;
	*/

	if (tsize < 10)
		mod = 18.0f;
	else if (tsize >=10 && tsize < 15)
		mod = 20.0f + (4.0f * (tsize - 10.0f));
	else if (tsize >=15 && tsize <= 20)
		mod = 42.0f + (5.8f * (tsize - 15.0f));
	else
		mod = 75.0f;

	return (mod + 2.0f); //Add 2.0f as buffer to prevent any chance of failures, client enforce range check regardless.
}

void NPC::RangedAttack(Mob* other)
{
	if (!other)
		return;
	//make sure the attack and ranged timers are up
	//if the ranged timer is disabled, then they have no ranged weapon and shouldent be attacking anyhow
	if((attack_timer.Enabled() && !attack_timer.Check(false)) || (ranged_timer.Enabled() && !ranged_timer.Check())){
		LogCombat("Archery canceled. Timer not up. Attack [{}], ranged [{}]", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		return;
	}

	if(!CheckLosFN(other))
		return;

	int attacks = GetSpecialAbilityParam(SPECATK_RANGED_ATK, 0);
	attacks = attacks > 0 ? attacks : 1;
	for(int i = 0; i < attacks; ++i) {

		if(!GetSpecialAbility(SPECATK_RANGED_ATK))
			return;

		int sa_min_range = GetSpecialAbilityParam(SPECATK_RANGED_ATK, 4); //Min Range of NPC attack
		int sa_max_range = GetSpecialAbilityParam(SPECATK_RANGED_ATK, 1); //Max Range of NPC attack

		float min_range = static_cast<float>(RuleI(Combat, MinRangedAttackDist));
		float max_range = 250; // needs to be longer than 200(most spells)

		if (sa_max_range)
			max_range = static_cast<float>(sa_max_range);

		if (sa_min_range)
			min_range = static_cast<float>(sa_min_range);

		max_range *= max_range;
		if(DistanceSquared(m_Position, other->GetPosition()) > max_range)
			return;
		else if(DistanceSquared(m_Position, other->GetPosition()) < (min_range * min_range))
			return;

		if(!other || !IsAttackAllowed(other) ||
			IsCasting() ||
			DivineAura() ||
			IsStunned() ||
			IsFeared() ||
			IsMezzed() ||
			(GetAppearance() == eaDead)){
			return;
		}

		FaceTarget(other);

		DoRangedAttackDmg(other);

		CommonBreakInvisibleFromCombat();
	}
}

void NPC::DoRangedAttackDmg(Mob* other, bool Launch, int16 damage_mod, int16 chance_mod, EQEmu::skills::SkillType skill, float speed, const char *IDFile)
{
	if ((other == nullptr ||
		(other->HasDied())) ||
		HasDied() ||
		(!IsAttackAllowed(other)) ||
		(other->GetInvul() ||
		other->GetSpecialAbility(IMMUNE_MELEE)))
	{
		return;
	}

	EQEmu::skills::SkillType skillInUse = static_cast<EQEmu::skills::SkillType>(GetRangedSkill());

	if (skill != skillInUse)
		skillInUse = skill;

	if (Launch)
	{
		const char *ammo = "IT10";

		if (IDFile != nullptr)
			ammo = IDFile;
		else if (GetAmmoIDfile())
			ammo = GetAmmoIDfile();

		ProjectileAnimation(other, 0,false,speed,0,0,0,ammo,skillInUse);

		if (RuleB(Combat, ProjectileDmgOnImpact))
		{
			TryProjectileAttack(other, nullptr, skillInUse, damage_mod, nullptr, nullptr, 0, speed);
			return;
		}
	}

	if (!chance_mod)
		chance_mod = GetSpecialAbilityParam(SPECATK_RANGED_ATK, 2);

	int TotalDmg = 0;
	int MaxDmg = GetBaseDamage() * RuleR(Combat, ArcheryNPCMultiplier); // should add a field to npc_types
	int MinDmg = GetMinDamage() * RuleR(Combat, ArcheryNPCMultiplier);

	if (!damage_mod)
		 damage_mod = GetSpecialAbilityParam(SPECATK_RANGED_ATK, 3);//Damage modifier

	DamageHitInfo my_hit;
	my_hit.base_damage = MaxDmg;
	my_hit.min_damage = MinDmg;
	my_hit.damage_done = 1;

	my_hit.skill = skill;
	my_hit.offense = offense(my_hit.skill);
	my_hit.tohit = GetTotalToHit(my_hit.skill, chance_mod);
	my_hit.hand = EQEmu::invslot::slotRange;

	DoAttack(other, my_hit);

	TotalDmg = my_hit.damage_done;

	if (TotalDmg > 0) {
		TotalDmg += TotalDmg * damage_mod / 100;
		other->AddToHateList(this, TotalDmg, 0, false);
	} else {
		other->AddToHateList(this, 0, 0, false);
	}

	other->Damage(this, TotalDmg, SPELL_UNKNOWN, skillInUse);

	if (TotalDmg > 0 && HasSkillProcSuccess() && !other->HasDied())
		TrySkillProc(other, skillInUse, 0, true, EQEmu::invslot::slotRange);

	//try proc on hits and misses
	if(other && !other->HasDied())
		TrySpellProc(nullptr, (const EQEmu::ItemData*)nullptr, other, EQEmu::invslot::slotRange);

	if (HasSkillProcs() && other && !other->HasDied())
		TrySkillProc(other, skillInUse, 0, false, EQEmu::invslot::slotRange);
}

void Client::ThrowingAttack(Mob* other, bool CanDoubleAttack) { //old was 51
	//conditions to use an attack checked before we are called
	if (!other)
		return;
	//make sure the attack and ranged timers are up
	//if the ranged timer is disabled, then they have no ranged weapon and shouldent be attacking anyhow
	if((!CanDoubleAttack && (attack_timer.Enabled() && !attack_timer.Check(false)) || (ranged_timer.Enabled() && !ranged_timer.Check()))) {
		LogCombat("Throwing attack canceled. Timer not up. Attack [{}], ranged [{}]", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		// The server and client timers are not exact matches currently, so this would spam too often if enabled
		//Message(0, "Error: Timer not up. Attack %d, ranged %d", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		return;
	}

	int ammo_slot = EQEmu::invslot::slotRange;
	const EQEmu::ItemInstance* RangeWeapon = m_inv[EQEmu::invslot::slotRange];

	if (!RangeWeapon || !RangeWeapon->IsClassCommon()) {
		LogCombat("Ranged attack canceled. Missing or invalid ranged weapon ([{}]) in slot [{}]", GetItemIDAt(EQEmu::invslot::slotRange), EQEmu::invslot::slotRange);
		Message(0, "Error: Rangeweapon: GetItem(%i)==0, you have nothing to throw!", GetItemIDAt(EQEmu::invslot::slotRange));
		return;
	}

	const EQEmu::ItemData* item = RangeWeapon->GetItem();
	if (item->ItemType != EQEmu::item::ItemTypeLargeThrowing && item->ItemType != EQEmu::item::ItemTypeSmallThrowing) {
		LogCombat("Ranged attack canceled. Ranged item [{}] is not a throwing weapon. type [{}]", item->ItemType);
		Message(0, "Error: Rangeweapon: GetItem(%i)==0, you have nothing useful to throw!", GetItemIDAt(EQEmu::invslot::slotRange));
		return;
	}

	LogCombat("Throwing [{}] ([{}]) at [{}]", item->Name, item->ID, other->GetName());

	if(RangeWeapon->GetCharges() == 1) {
		//first check ammo
		const EQEmu::ItemInstance* AmmoItem = m_inv[EQEmu::invslot::slotAmmo];
		if(AmmoItem != nullptr && AmmoItem->GetID() == RangeWeapon->GetID()) {
			//more in the ammo slot, use it
			RangeWeapon = AmmoItem;
			ammo_slot = EQEmu::invslot::slotAmmo;
			LogCombat("Using ammo from ammo slot, stack at slot [{}]. [{}] in stack", ammo_slot, RangeWeapon->GetCharges());
		} else {
			//look through our inventory for more
			int32 aslot = m_inv.HasItem(item->ID, 1, invWherePersonal);
			if (aslot != INVALID_INDEX) {
				//the item wont change, but the instance does, not that it matters
				ammo_slot = aslot;
				RangeWeapon = m_inv[aslot];
				LogCombat("Using ammo from inventory slot, stack at slot [{}]. [{}] in stack", ammo_slot, RangeWeapon->GetCharges());
			}
		}
	}

	float range = item->Range + GetRangeDistTargetSizeMod(other);
	LogCombat("Calculated bow range to be [{}]", range);
	range *= range;
	float dist = DistanceSquared(m_Position, other->GetPosition());
	if(dist > range) {
		LogCombat("Throwing attack out of range client should catch this. ([{}] > [{}]).\n", dist, range);
		MessageString(Chat::Red,TARGET_OUT_OF_RANGE);//Client enforces range and sends the message, this is a backup just incase.
		return;
	}
	else if(dist < (RuleI(Combat, MinRangedAttackDist)*RuleI(Combat, MinRangedAttackDist))){
		MessageString(Chat::Yellow,RANGED_TOO_CLOSE);//Client enforces range and sends the message, this is a backup just incase.
		return;
	}

	if(!IsAttackAllowed(other) ||
		IsCasting() ||
		IsSitting() ||
		(DivineAura() && !GetGM()) ||
		IsStunned() ||
		IsFeared() ||
		IsMezzed() ||
		(GetAppearance() == eaDead)){
		return;
	}

	DoThrowingAttackDmg(other, RangeWeapon, item);

	//consume ammo
	DeleteItemInInventory(ammo_slot, 1, true);
	CommonBreakInvisibleFromCombat();
}

void Mob::DoThrowingAttackDmg(Mob *other, const EQEmu::ItemInstance *RangeWeapon, const EQEmu::ItemData *AmmoItem,
			      uint16 weapon_damage, int16 chance_mod, int16 focus, int ReuseTime, uint32 range_id,
			      int AmmoSlot, float speed)
{
	if ((other == nullptr ||
	     ((IsClient() && CastToClient()->dead) || (other->IsClient() && other->CastToClient()->dead)) ||
	     HasDied() || (!IsAttackAllowed(other)) || (other->GetInvul() || other->GetSpecialAbility(IMMUNE_MELEE)))) {
		return;
	}

	const EQEmu::ItemInstance *_RangeWeapon = nullptr;
	const EQEmu::ItemData *ammo_lost = nullptr;

	/*
	If LaunchProjectile is false this function will do archery damage on target,
	otherwise it will shoot the projectile at the target, once the projectile hits target
	this function is then run again to do the damage portion
	*/
	bool LaunchProjectile = false;

	if (RuleB(Combat, ProjectileDmgOnImpact)) {
		if (AmmoItem) {
			LaunchProjectile = true;
		} else {
			if (!RangeWeapon && range_id) {
				if (IsClient()) {
					_RangeWeapon = CastToClient()->m_inv[AmmoSlot];
					if (_RangeWeapon && _RangeWeapon->GetItem() &&
					    _RangeWeapon->GetItem()->ID != range_id)
						RangeWeapon = _RangeWeapon;
					else
						ammo_lost = database.GetItem(range_id);
				}
			}
		}
	} else if (AmmoItem) {
		SendItemAnimation(other, AmmoItem, EQEmu::skills::SkillThrowing);
	}

	LogCombat("Throwing attack hit [{}]", other->GetName());

	int WDmg = 0;

	if (!weapon_damage) {
		if (IsClient() && RangeWeapon)
			WDmg = GetWeaponDamage(other, RangeWeapon);
		else if (AmmoItem)
			WDmg = GetWeaponDamage(other, AmmoItem);

		if (LaunchProjectile) {
			TryProjectileAttack(other, AmmoItem, EQEmu::skills::SkillThrowing, WDmg, RangeWeapon,
					    nullptr, AmmoSlot, speed);
			return;
		}
	} else {
		WDmg = weapon_damage;
	}

	if (focus) // From FcBaseEffects
		WDmg += WDmg * focus / 100;

	int TotalDmg = 0;

	if (WDmg > 0) {
		DamageHitInfo my_hit;
		my_hit.base_damage = WDmg;
		my_hit.min_damage = 0;
		my_hit.damage_done = 1;

		my_hit.skill = EQEmu::skills::SkillThrowing;
		my_hit.offense = offense(my_hit.skill);
		my_hit.tohit = GetTotalToHit(my_hit.skill, chance_mod);
		my_hit.hand = EQEmu::invslot::slotRange;

		DoAttack(other, my_hit);
		TotalDmg = my_hit.damage_done;

		LogCombat("Item DMG [{}]. Hit for damage [{}]", WDmg, TotalDmg);
	} else {
		TotalDmg = DMG_INVULNERABLE;
	}

	if (IsClient() && !CastToClient()->GetFeigned())
		other->AddToHateList(this, WDmg, 0, false);

	other->Damage(this, TotalDmg, SPELL_UNKNOWN, EQEmu::skills::SkillThrowing);

	if (TotalDmg > 0 && HasSkillProcSuccess() && other && !other->HasDied()) {
		if (ReuseTime)
			TrySkillProc(other, EQEmu::skills::SkillThrowing, ReuseTime);
		else
			TrySkillProc(other, EQEmu::skills::SkillThrowing, 0, true, EQEmu::invslot::slotRange);
	}
	// end old shit

	if (LaunchProjectile)
		return;

	// Throwing item Proc
	if (ammo_lost)
		TryWeaponProc(nullptr, ammo_lost, other, EQEmu::invslot::slotRange);
	else if (RangeWeapon && other && !other->HasDied())
		TryWeaponProc(RangeWeapon, other, EQEmu::invslot::slotRange);

	if (HasSkillProcs() && other && !other->HasDied()) {
		if (ReuseTime)
			TrySkillProc(other, EQEmu::skills::SkillThrowing, ReuseTime);
		else
			TrySkillProc(other, EQEmu::skills::SkillThrowing, 0, false, EQEmu::invslot::slotRange);
	}
	if (IsClient()) {
		CastToClient()->CheckIncreaseSkill(EQEmu::skills::SkillThrowing, GetTarget());
	}
}

void Mob::SendItemAnimation(Mob *to, const EQEmu::ItemData *item, EQEmu::skills::SkillType skillInUse, float velocity) {
	auto outapp = new EQApplicationPacket(OP_SomeItemPacketMaybe, sizeof(Arrow_Struct));
	Arrow_Struct *as = (Arrow_Struct *) outapp->pBuffer;
	as->type = 1;
	as->src_x = GetX();
	as->src_y = GetY();
	as->src_z = GetZ();
	as->source_id = GetID();
	as->target_id = to->GetID();
	as->item_id = item->ID;

	as->item_type = item->ItemType;
	as->skill = (uint8)skillInUse;

	strn0cpy(as->model_name, item->IDFile, 16);


	/*
		The angular field affects how the object flies towards the target.
		A low angular (10) makes it circle the target widely, where a high
		angular (20000) makes it go straight at them.

		The tilt field causes the object to be tilted flying through the air
		and also seems to have an effect on how it behaves when circling the
		target based on the angular field.

		Arc causes the object to form an arc in motion. A value too high will
	*/
	as->velocity = velocity;

	//these angle and tilt used together seem to make the arrow/knife throw as straight as I can make it

	as->launch_angle = CalculateHeadingToTarget(to->GetX(), to->GetY());
	as->tilt = 125;
	as->arc = 50;


	//fill in some unknowns, we dont know their meaning yet
	//neither of these seem to change the behavior any
	as->unknown088 = 125;
	as->unknown092 = 16;

	entity_list.QueueCloseClients(this, outapp);
	safe_delete(outapp);
}

void Mob::ProjectileAnimation(Mob* to, int item_id, bool IsArrow, float speed, float angle, float tilt, float arc, const char *IDFile, EQEmu::skills::SkillType skillInUse) {
	if (!to)
		return;

	const EQEmu::ItemData* item = nullptr;
	uint8 item_type = 0;

	if(!item_id) {
		item = database.GetItem(8005); // Arrow will be default
	}
	else {
		item = database.GetItem(item_id); // Use the item input into the command
	}

	if(!item) {
		return;
	}
	if(IsArrow) {
		item_type = 27;
	}
	if(!item_type && !skillInUse) {
		item_type = item->ItemType;
	}
	else if (skillInUse)
		item_type = GetItemTypeBySkill(skillInUse);

	if(!speed) {
		speed = 4.0;
	}
	if(!angle) {
		angle = CalculateHeadingToTarget(to->GetX(), to->GetY()) * 2;
	}
	if(!tilt) {
		tilt = 125;
	}
	if(!arc) {
		arc = 50;
	}

	const char *item_IDFile = item->IDFile;

	if (IDFile && (strncmp(IDFile, "IT", 2) == 0))
		item_IDFile = IDFile;

	// See SendItemAnimation() for some notes on this struct
	auto outapp = new EQApplicationPacket(OP_SomeItemPacketMaybe, sizeof(Arrow_Struct));
	Arrow_Struct *as = (Arrow_Struct *) outapp->pBuffer;
	as->type = 1;
	as->src_x = GetX();
	as->src_y = GetY();
	as->src_z = GetZ();
	as->source_id = GetID();
	as->target_id = to->GetID();
	as->item_id = item->ID;
	as->item_type = item_type;
	as->skill = skillInUse;	// Doesn't seem to have any effect
	strn0cpy(as->model_name, item_IDFile, 16);
	as->velocity = speed;
	as->launch_angle = angle;
	as->tilt = tilt;
	as->arc = arc;
	as->unknown088 = 125;
	as->unknown092 = 16;

	entity_list.QueueCloseClients(this, outapp);
	safe_delete(outapp);

}

void NPC::DoClassAttacks(Mob *target) {
	if(target == nullptr)
		return;	//gotta have a target for all these

	bool taunt_time = taunt_timer.Check();
	bool ca_time = classattack_timer.Check(false);
	bool ka_time = knightattack_timer.Check(false);

	//only check attack allowed if we are going to do something
	if((taunt_time || ca_time || ka_time) && !IsAttackAllowed(target))
		return;

	if(ka_time){
		int knightreuse = 1000; //lets give it a small cooldown actually.

		switch(GetClass()){
			case SHADOWKNIGHT: case SHADOWKNIGHTGM:{
				if (CastSpell(SPELL_NPC_HARM_TOUCH, target->GetID())) {
					knightreuse = HarmTouchReuseTime * 1000;
					}
				break;
			}
			case PALADIN: case PALADINGM:{
				if(GetHPRatio() < 20) {
					if (CastSpell(SPELL_LAY_ON_HANDS, GetID())) {
						knightreuse = LayOnHandsReuseTime * 1000;
					}
				} else {
					knightreuse = 2000; //Check again in two seconds.
				}
				break;
			}
		}
		knightattack_timer.Start(knightreuse);
	}

	//general stuff, for all classes....
	//only gets used when their primary ability get used too
	if (taunting && HasOwner() && target->IsNPC() && target->GetBodyType() != BT_Undead && taunt_time) {
		this->GetOwner()->MessageString(Chat::PetResponse, PET_TAUNTING);
		Taunt(target->CastToNPC(), false);
	}

	if(!ca_time)
		return;

	float HasteModifier = GetHaste() * 0.01f;

	int level = GetLevel();
	int reuse = TauntReuseTime * 1000;	//make this very long since if they dont use it once, they prolly never will
	bool did_attack = false;
	//class specific stuff...
	switch(GetClass()) {
		case ROGUE: case ROGUEGM:
			if(level >= 10) {
				reuse = BackstabReuseTime * 1000;
				TryBackstab(target, reuse);
				did_attack = true;
			}
			break;
		case MONK: case MONKGM: {
			uint8 satype = EQEmu::skills::SkillKick;
			if (level > 29) { satype = EQEmu::skills::SkillFlyingKick; }
			else if (level > 24) { satype = EQEmu::skills::SkillDragonPunch; }
			else if (level > 19) { satype = EQEmu::skills::SkillEagleStrike; }
			else if (level > 9) { satype = EQEmu::skills::SkillTigerClaw; }
			else if (level > 4) { satype = EQEmu::skills::SkillRoundKick; }
			reuse = MonkSpecialAttack(target, satype);

			reuse *= 1000;
			did_attack = true;
			break;
		}
		case WARRIOR: case WARRIORGM:{
			if(level >= RuleI(Combat, NPCBashKickLevel)){
				if(zone->random.Roll(75)) { //tested on live, warrior mobs both kick and bash, kick about 75% of the time, casting doesn't seem to make a difference.
					DoAnim(animKick, 0, false);
					int32 dmg = GetBaseSkillDamage(EQEmu::skills::SkillKick);

					if (GetWeaponDamage(target, (const EQEmu::ItemData*)nullptr) <= 0)
						dmg = DMG_INVULNERABLE;

					reuse = (KickReuseTime + 3) * 1000;
					DoSpecialAttackDamage(target, EQEmu::skills::SkillKick, dmg, GetMinDamage(), -1, reuse);
					did_attack = true;
				}
				else {
					DoAnim(animTailRake, 0, false);
					int32 dmg = GetBaseSkillDamage(EQEmu::skills::SkillBash);

					if (GetWeaponDamage(target, (const EQEmu::ItemData*)nullptr) <= 0)
						dmg = DMG_INVULNERABLE;

					reuse = (BashReuseTime + 3) * 1000;
					DoSpecialAttackDamage(target, EQEmu::skills::SkillBash, dmg, GetMinDamage(), -1, reuse);
					did_attack = true;
				}
			}
			break;
		}
		case BERSERKER: case BERSERKERGM:{
			int AtkRounds = 1;
			int32 max_dmg = GetBaseSkillDamage(EQEmu::skills::SkillFrenzy);
			DoAnim(anim2HSlashing, 0, false);

			if (GetClass() == BERSERKER) {
				int chance = GetLevel() * 2 + GetSkill(EQEmu::skills::SkillFrenzy);
				if (zone->random.Roll0(450) < chance)
					AtkRounds++;
				if (zone->random.Roll0(450) < chance)
					AtkRounds++;
			}

			while (AtkRounds > 0) {
				if (GetTarget())
					DoSpecialAttackDamage(GetTarget(), EQEmu::skills::SkillFrenzy, max_dmg, GetMinDamage(), -1, reuse);
				AtkRounds--;
			}

			did_attack = true;
			break;
		}
		case RANGER: case RANGERGM:
		case BEASTLORD: case BEASTLORDGM: {
			//kick
			if(level >= RuleI(Combat, NPCBashKickLevel)){
				DoAnim(animKick, 0, false);
				int32 dmg = GetBaseSkillDamage(EQEmu::skills::SkillKick);

				if (GetWeaponDamage(target, (const EQEmu::ItemData*)nullptr) <= 0)
					dmg = DMG_INVULNERABLE;

				reuse = (KickReuseTime + 3) * 1000;
				DoSpecialAttackDamage(target, EQEmu::skills::SkillKick, dmg, GetMinDamage(), -1, reuse);
				did_attack = true;
			}
			break;
		}
		case CLERIC: case CLERICGM: //clerics can bash too.
		case SHADOWKNIGHT: case SHADOWKNIGHTGM:
		case PALADIN: case PALADINGM:{
			if(level >= RuleI(Combat, NPCBashKickLevel)){
				DoAnim(animTailRake, 0, false);
				int32 dmg = GetBaseSkillDamage(EQEmu::skills::SkillBash);

				if (GetWeaponDamage(target, (const EQEmu::ItemData*)nullptr) <= 0)
					dmg = DMG_INVULNERABLE;

				reuse = (BashReuseTime + 3) * 1000;
				DoSpecialAttackDamage(target, EQEmu::skills::SkillBash, dmg, GetMinDamage(), -1, reuse);
				did_attack = true;
			}
			break;
		}
	}

	classattack_timer.Start(reuse / HasteModifier);
}

// this should be refactored to generate an OP_CombatAbility struct and call OPCombatAbility
void Client::DoClassAttacks(Mob *ca_target, uint16 skill, bool IsRiposte)
{
	if(!ca_target)
		return;

	if(spellend_timer.Enabled() || IsFeared() || IsStunned() || IsMezzed() || DivineAura() || dead)
		return;

	if(!IsAttackAllowed(ca_target))
		return;

	//check range for all these abilities, they are all close combat stuff
	if(!CombatRange(ca_target)){
		return;
	}

	if(!IsRiposte && (!p_timers.Expired(&database, pTimerCombatAbility, false))) {
		return;
	}

	int ReuseTime = 0;
	float HasteMod = GetHaste() * 0.01f;

	uint16 skill_to_use = -1;

	if (skill == -1){
		switch(GetClass()){
		case WARRIOR:
		case RANGER:
		case BEASTLORD:
			skill_to_use = EQEmu::skills::SkillKick;
			break;
		case BERSERKER:
			skill_to_use = EQEmu::skills::SkillFrenzy;
			break;
		case SHADOWKNIGHT:
		case PALADIN:
			skill_to_use = EQEmu::skills::SkillBash;
			break;
		case MONK:
			if(GetLevel() >= 30)
			{
				skill_to_use = EQEmu::skills::SkillFlyingKick;
			}
			else if(GetLevel() >= 25)
			{
				skill_to_use = EQEmu::skills::SkillDragonPunch;
			}
			else if(GetLevel() >= 20)
			{
				skill_to_use = EQEmu::skills::SkillEagleStrike;
			}
			else if(GetLevel() >= 10)
			{
				skill_to_use = EQEmu::skills::SkillTigerClaw;
			}
			else if(GetLevel() >= 5)
			{
				skill_to_use = EQEmu::skills::SkillRoundKick;
			}
			else
			{
				skill_to_use = EQEmu::skills::SkillKick;
			}
			break;
		case ROGUE:
			skill_to_use = EQEmu::skills::SkillBackstab;
			break;
		}
	}

	else
		skill_to_use = skill;

	if(skill_to_use == -1)
		return;

	int dmg = GetBaseSkillDamage(static_cast<EQEmu::skills::SkillType>(skill_to_use), GetTarget());

	if (skill_to_use == EQEmu::skills::SkillBash) {
		if (ca_target!=this) {
			DoAnim(animTailRake, 0, false);

			if (GetWeaponDamage(ca_target, GetInv().GetItem(EQEmu::invslot::slotSecondary)) <= 0 && GetWeaponDamage(ca_target, GetInv().GetItem(EQEmu::invslot::slotShoulders)) <= 0)
				dmg = DMG_INVULNERABLE;

			ReuseTime = (BashReuseTime - 1) / HasteMod;

			DoSpecialAttackDamage(ca_target, EQEmu::skills::SkillBash, dmg, 0, -1, ReuseTime);

			if(ReuseTime > 0 && !IsRiposte) {
				p_timers.Start(pTimerCombatAbility, ReuseTime);
			}
		}
		return;
	}

	if (skill_to_use == EQEmu::skills::SkillFrenzy) {
		CheckIncreaseSkill(EQEmu::skills::SkillFrenzy, GetTarget(), 10);
		int AtkRounds = 1;
		DoAnim(anim2HSlashing, 0, false);

		ReuseTime = (FrenzyReuseTime - 1) / HasteMod;

		// bards can do riposte frenzy for some reason
		if (!IsRiposte && GetClass() == BERSERKER) {
			int chance = GetLevel() * 2 + GetSkill(EQEmu::skills::SkillFrenzy);
			if (zone->random.Roll0(450) < chance)
				AtkRounds++;
			if (zone->random.Roll0(450) < chance)
				AtkRounds++;
		}

		while(AtkRounds > 0) {
			if (ca_target!=this)
				DoSpecialAttackDamage(ca_target, EQEmu::skills::SkillFrenzy, dmg, 0, dmg, ReuseTime);
			AtkRounds--;
		}

		if(ReuseTime > 0 && !IsRiposte) {
			p_timers.Start(pTimerCombatAbility, ReuseTime);
		}
		return;
	}

	if (skill_to_use == EQEmu::skills::SkillKick){
		if(ca_target!=this){
			DoAnim(animKick, 0, false);

			if (GetWeaponDamage(ca_target, GetInv().GetItem(EQEmu::invslot::slotFeet)) <= 0)
				dmg = DMG_INVULNERABLE;

			ReuseTime = KickReuseTime-1;

			DoSpecialAttackDamage(ca_target, EQEmu::skills::SkillKick, dmg, 0, -1, ReuseTime);
		}
	}

	if (skill_to_use == EQEmu::skills::SkillFlyingKick || skill_to_use == EQEmu::skills::SkillDragonPunch || skill_to_use == EQEmu::skills::SkillEagleStrike || skill_to_use == EQEmu::skills::SkillTigerClaw || skill_to_use == EQEmu::skills::SkillRoundKick) {
		ReuseTime = MonkSpecialAttack(ca_target, skill_to_use) - 1;
		MonkSpecialAttack(ca_target, skill_to_use);

		if (IsRiposte)
			return;

		//Live AA - Technique of Master Wu
		int wuchance = itembonuses.DoubleSpecialAttack + spellbonuses.DoubleSpecialAttack + aabonuses.DoubleSpecialAttack;
		if (wuchance) {
			const int MonkSPA[5] = {EQEmu::skills::SkillFlyingKick, EQEmu::skills::SkillDragonPunch,
						EQEmu::skills::SkillEagleStrike, EQEmu::skills::SkillTigerClaw,
						EQEmu::skills::SkillRoundKick};
			int extra = 0;
			// always 1/4 of the double attack chance, 25% at rank 5 (100/4)
			while (wuchance > 0) {
				if (zone->random.Roll(wuchance))
					extra++;
				else
					break;
				wuchance /= 4;
			}
			// They didn't add a string ID for this.
			std::string msg = StringFormat(
			    "The spirit of Master Wu fills you!  You gain %d additional attack(s).", extra);
			// live uses 400 here -- not sure if it's the best for all clients though
			SendColoredText(400, msg);
			auto classic = RuleB(Combat, ClassicMasterWu);
			while (extra) {
				MonkSpecialAttack(GetTarget(),
						  classic ? MonkSPA[zone->random.Int(0, 4)] : skill_to_use);
				extra--;
			}
		}
	}

	if (skill_to_use == EQEmu::skills::SkillBackstab){
		ReuseTime = BackstabReuseTime-1;

		if (IsRiposte)
			ReuseTime=0;

		TryBackstab(ca_target,ReuseTime);
	}

	ReuseTime = ReuseTime / HasteMod;
	if(ReuseTime > 0 && !IsRiposte){
		p_timers.Start(pTimerCombatAbility, ReuseTime);
	}
}

void Mob::Taunt(NPC *who, bool always_succeed, int chance_bonus, bool FromSpell, int32 bonus_hate)
{
	if (who == nullptr)
		return;

	if (DivineAura())
		return;

	if (!FromSpell && !CombatRange(who))
		return;

	if (!always_succeed && IsClient())
		CastToClient()->CheckIncreaseSkill(EQEmu::skills::SkillTaunt, who, 10);

	Mob *hate_top = who->GetHateMost();

	int level_difference = GetLevel() - who->GetLevel();
	bool Success = false;

	// Support for how taunt worked pre 2000 on LIVE - Can not taunt NPC over your level.
	if ((RuleB(Combat, TauntOverLevel) == false) && (level_difference < 0) ||
	    who->GetSpecialAbility(IMMUNE_TAUNT)) {
		MessageString(Chat::SpellFailure, FAILED_TAUNT);
		return;
	}

	// All values used based on live parses after taunt was updated in 2006.
	if ((hate_top && hate_top->GetHPRatio() >= 20) || hate_top == nullptr || chance_bonus) {
		// SE_Taunt this is flat chance
		if (chance_bonus) {
			Success = zone->random.Roll(chance_bonus);
		} else {
			float tauntchance = 50.0f;

			if (always_succeed)
				tauntchance = 101.0f;

			else {

				if (level_difference < 0) {
					tauntchance += static_cast<float>(level_difference) * 3.0f;
					if (tauntchance < 20)
						tauntchance = 20.0f;
				}

				else {
					tauntchance += static_cast<float>(level_difference) * 5.0f;
					if (tauntchance > 65)
						tauntchance = 65.0f;
				}
			}

			// TauntSkillFalloff rate is not based on any real data. Default of 33% gives a reasonable
			// result.
			if (IsClient() && !always_succeed)
				tauntchance -= (RuleR(Combat, TauntSkillFalloff) *
						(CastToClient()->MaxSkill(EQEmu::skills::SkillTaunt) -
						 GetSkill(EQEmu::skills::SkillTaunt)));

			if (tauntchance < 1)
				tauntchance = 1.0f;

			tauntchance /= 100.0f;

			Success = tauntchance > zone->random.Real(0, 1);
		}

		if (Success) {
			if (hate_top && hate_top != this) {
				int newhate = (who->GetNPCHate(hate_top) - who->GetNPCHate(this)) + 1 + bonus_hate;
				who->CastToNPC()->AddToHateList(this, newhate);
				Success = true;
			} else {
				who->CastToNPC()->AddToHateList(this, 12);
			}

			if (who->CanTalk())
				who->SayString(SUCCESSFUL_TAUNT, GetCleanName());
		} else {
			MessageString(Chat::SpellFailure, FAILED_TAUNT);
		}
	} else {
		MessageString(Chat::SpellFailure, FAILED_TAUNT);
	}

	if (HasSkillProcs())
		TrySkillProc(who, EQEmu::skills::SkillTaunt, TauntReuseTime * 1000);

	if (Success && HasSkillProcSuccess())
		TrySkillProc(who, EQEmu::skills::SkillTaunt, TauntReuseTime * 1000, true);
}

void Mob::InstillDoubt(Mob *who) {
	//make sure we can use this skill
	/*int skill = GetSkill(INTIMIDATION);*/	//unused

	//make sure our target is an NPC
	if(!who || !who->IsNPC())
		return;

	if(DivineAura())
		return;

	//range check
	if(!CombatRange(who))
		return;

	if(IsClient()) {
		CastToClient()->CheckIncreaseSkill(EQEmu::skills::SkillIntimidation, who, 10);
	}

	//I think this formula needs work
	int value = 0;

	//user's bonus
	value += GetSkill(EQEmu::skills::SkillIntimidation) + GetCHA() / 4;

	//target's counters
	value -= target->GetLevel()*4 + who->GetWIS()/4;

	if (zone->random.Roll(value)) {
		//temporary hack...
		//cast fear on them... should prolly be a different spell
		//and should be un-resistable.
		SpellOnTarget(229, who, false, true, -2000);
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
}

int Mob::TryHeadShot(Mob *defender, EQEmu::skills::SkillType skillInUse)
{
	// Only works on YOUR target.
	if (defender && defender->GetBodyType() == BT_Humanoid && !defender->IsClient() &&
	    skillInUse == EQEmu::skills::SkillArchery && GetTarget() == defender) {
		uint32 HeadShot_Dmg = aabonuses.HeadShot[1] + spellbonuses.HeadShot[1] + itembonuses.HeadShot[1];
		uint8 HeadShot_Level = 0; // Get Highest Headshot Level
		HeadShot_Level = std::max({aabonuses.HSLevel[0], spellbonuses.HSLevel[0], itembonuses.HSLevel[0]});

		if (HeadShot_Dmg && HeadShot_Level && (defender->GetLevel() <= HeadShot_Level)) {
			int chance = GetDEX();
			chance = 100 * chance / (chance + 3500);
			if (IsClient())
				chance += CastToClient()->GetHeroicDEX() / 25;
			chance *= 10;
			int norm = aabonuses.HSLevel[1];
			if (norm > 0)
				chance = chance * norm / 100;
			chance += aabonuses.HeadShot[0] + spellbonuses.HeadShot[0] + itembonuses.HeadShot[0];
			if (zone->random.Int(1, 1000) <= chance) {
				entity_list.MessageCloseString(
					this, false, 200, Chat::MeleeCrit, FATAL_BOW_SHOT,
					GetName());
				return HeadShot_Dmg;
			}
		}
	}

	return 0;
}

int Mob::TryAssassinate(Mob *defender, EQEmu::skills::SkillType skillInUse)
{
	if (defender && (defender->GetBodyType() == BT_Humanoid) && !defender->IsClient() && GetLevel() >= 60 &&
	    (skillInUse == EQEmu::skills::SkillBackstab || skillInUse == EQEmu::skills::SkillThrowing)) {
		int chance = GetDEX();
		if (skillInUse == EQEmu::skills::SkillBackstab) {
			chance = 100 * chance / (chance + 3500);
			if (IsClient())
				chance += CastToClient()->GetHeroicDEX();
			chance *= 10;
			int norm = aabonuses.AssassinateLevel[1];
			if (norm > 0)
				chance = chance * norm / 100;
		} else if (skillInUse == EQEmu::skills::SkillThrowing) {
			if (chance > 255)
				chance = 260;
			else
				chance += 5;
		}

		chance += aabonuses.Assassinate[0] + spellbonuses.Assassinate[0] + itembonuses.Assassinate[0];

		uint32 Assassinate_Dmg =
		    aabonuses.Assassinate[1] + spellbonuses.Assassinate[1] + itembonuses.Assassinate[1];

		uint8 Assassinate_Level = 0; // Get Highest Headshot Level
		Assassinate_Level = std::max(
		    {aabonuses.AssassinateLevel[0], spellbonuses.AssassinateLevel[0], itembonuses.AssassinateLevel[0]});

		// revamped AAs require AA line I believe?
		if (!Assassinate_Level)
			return 0;

		if (Assassinate_Dmg && Assassinate_Level && (defender->GetLevel() <= Assassinate_Level)) {
			if (zone->random.Int(1, 1000) <= chance) {
				entity_list.MessageCloseString(
					this, false, 200, Chat::MeleeCrit, ASSASSINATES,
					GetName());
				return Assassinate_Dmg;
			}
		}
	}

	return 0;
}

void Mob::DoMeleeSkillAttackDmg(Mob *other, uint16 weapon_damage, EQEmu::skills::SkillType skillinuse, int16 chance_mod,
				int16 focus, bool CanRiposte, int ReuseTime)
{
	if (!CanDoSpecialAttack(other))
		return;

	/*
		For spells using skill value 98 (feral swipe ect) server sets this to 67 automatically.
		Kayen: This is unlikely to be completely accurate but use OFFENSE skill value for these effects.
		TODO: We need to stop moving skill 98, it's suppose to just be a dummy skill AFAIK
		Spells using offense should use the skill of your primary, if you can use it, otherwise h2h
	*/
	if (skillinuse == EQEmu::skills::SkillBegging)
		skillinuse = EQEmu::skills::SkillOffense;

	int damage = 0;
	uint32 hate = 0;
	if (hate == 0 && weapon_damage > 1)
		hate = weapon_damage;

	if (weapon_damage > 0) {
		if (focus) // From FcBaseEffects
			weapon_damage += weapon_damage * focus / 100;

		if (skillinuse == EQEmu::skills::SkillBash) {
			if (IsClient()) {
				EQEmu::ItemInstance *item =
				    CastToClient()->GetInv().GetItem(EQEmu::invslot::slotSecondary);
				if (item) {
					if (item->GetItem()->ItemType == EQEmu::item::ItemTypeShield) {
						hate += item->GetItem()->AC;
					}
					const EQEmu::ItemData *itm = item->GetItem();
					hate = hate * (100 + GetFuriousBash(itm->Focus.Effect)) / 100;
				}
			}
		}

		DamageHitInfo my_hit;
		my_hit.base_damage = weapon_damage;
		my_hit.min_damage = 0;
		my_hit.damage_done = 1;

		my_hit.skill = skillinuse;
		my_hit.offense = offense(my_hit.skill);
		my_hit.tohit = GetTotalToHit(my_hit.skill, chance_mod);
		// slot range exclude ripe etc ...
		my_hit.hand = CanRiposte ? EQEmu::invslot::slotRange : EQEmu::invslot::slotPrimary;

		if (IsNPC())
			my_hit.min_damage = CastToNPC()->GetMinDamage();

		DoAttack(other, my_hit);
		damage = my_hit.damage_done;
	} else {
		damage = DMG_INVULNERABLE;
	}

	bool CanSkillProc = true;
	if (skillinuse == EQEmu::skills::SkillOffense) {    // Hack to allow damage to display.
		skillinuse = EQEmu::skills::SkillTigerClaw; //'strike' your opponent - Arbitrary choice for message.
		CanSkillProc = false;			    // Disable skill procs
	}

	other->AddToHateList(this, hate, 0, false);
	if (damage > 0 && aabonuses.SkillAttackProc[0] && aabonuses.SkillAttackProc[1] == skillinuse &&
	    IsValidSpell(aabonuses.SkillAttackProc[2])) {
		float chance = aabonuses.SkillAttackProc[0] / 1000.0f;
		if (zone->random.Roll(chance))
			SpellFinished(aabonuses.SkillAttackProc[2], other, EQEmu::spells::CastingSlot::Item, 0, -1,
				      spells[aabonuses.SkillAttackProc[2]].ResistDiff);
	}

	other->Damage(this, damage, SPELL_UNKNOWN, skillinuse);

	if (HasDied())
		return;

	if (CanSkillProc && HasSkillProcs())
		TrySkillProc(other, skillinuse, ReuseTime);

	if (CanSkillProc && (damage > 0) && HasSkillProcSuccess())
		TrySkillProc(other, skillinuse, ReuseTime, true);
}

bool Mob::CanDoSpecialAttack(Mob *other) {
	//Make sure everything is valid before doing any attacks.
	if (!other) {
		SetTarget(nullptr);
		return false;
	}

	if(!GetTarget())
		SetTarget(other);

	if ((other == nullptr || ((IsClient() && CastToClient()->dead) || (other->IsClient() && other->CastToClient()->dead)) || HasDied() || (!IsAttackAllowed(other)))) {
		return false;
	}

	if(other->GetInvul() || other->GetSpecialAbility(IMMUNE_MELEE))
		return false;

	return true;
}
