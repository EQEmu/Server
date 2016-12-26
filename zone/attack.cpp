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

#include "../common/global_define.h"
#include "../common/eq_constants.h"
#include "../common/eq_packet_structs.h"
#include "../common/rulesys.h"
#include "../common/skills.h"
#include "../common/spdat.h"
#include "../common/string_util.h"
#include "../common/data_verification.h"
#include "queryserv.h"
#include "quest_parser_collection.h"
#include "string_ids.h"
#include "water_map.h"
#include "worldserver.h"
#include "zone.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef BOTS
#include "bot.h"
#endif

extern QueryServ* QServ;
extern WorldServer worldserver;

#ifdef _WINDOWS
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#endif

extern EntityList entity_list;
extern Zone* zone;

bool Mob::AttackAnimation(EQEmu::skills::SkillType &skillinuse, int Hand, const EQEmu::ItemInstance* weapon)
{
	// Determine animation
	int type = 0;
	if (weapon && weapon->IsClassCommon()) {
		const EQEmu::ItemData* item = weapon->GetItem();

		Log.Out(Logs::Detail, Logs::Attack, "Weapon skill : %i", item->ItemType);

		switch (item->ItemType) {
		case EQEmu::item::ItemType1HSlash: // 1H Slashing
			skillinuse = EQEmu::skills::Skill1HSlashing;
			type = anim1HWeapon;
			break;
		case EQEmu::item::ItemType2HSlash: // 2H Slashing
			skillinuse = EQEmu::skills::Skill2HSlashing;
			type = anim2HSlashing;
			break;
		case EQEmu::item::ItemType1HPiercing: // Piercing
			skillinuse = EQEmu::skills::Skill1HPiercing;
			type = anim1HPiercing;
			break;
		case EQEmu::item::ItemType1HBlunt: // 1H Blunt
			skillinuse = EQEmu::skills::Skill1HBlunt;
			type = anim1HWeapon;
			break;
		case EQEmu::item::ItemType2HBlunt: // 2H Blunt
			skillinuse = EQEmu::skills::Skill2HBlunt;
			type = anim2HSlashing; //anim2HWeapon
			break;
		case EQEmu::item::ItemType2HPiercing: // 2H Piercing
			if (IsClient() && CastToClient()->ClientVersion() < EQEmu::versions::ClientVersion::RoF2)
				skillinuse = EQEmu::skills::Skill1HPiercing;
			else
				skillinuse = EQEmu::skills::Skill2HPiercing;
			type = anim2HWeapon;
			break;
		case EQEmu::item::ItemTypeMartial:
			skillinuse = EQEmu::skills::SkillHandtoHand;
			type = animHand2Hand;
			break;
		default:
			skillinuse = EQEmu::skills::SkillHandtoHand;
			type = animHand2Hand;
			break;
		}// switch
	}
	else if(IsNPC()) {
		switch (skillinuse) {
		case EQEmu::skills::Skill1HSlashing: // 1H Slashing
			type = anim1HWeapon;
			break;
		case EQEmu::skills::Skill2HSlashing: // 2H Slashing
			type = anim2HSlashing;
			break;
		case EQEmu::skills::Skill1HPiercing: // Piercing
			type = anim1HPiercing;
			break;
		case EQEmu::skills::Skill1HBlunt: // 1H Blunt
			type = anim1HWeapon;
			break;
		case EQEmu::skills::Skill2HBlunt: // 2H Blunt
			type = anim2HSlashing; //anim2HWeapon
			break;
		case EQEmu::skills::Skill2HPiercing: // 2H Piercing
			type = anim2HWeapon;
			break;
		case EQEmu::skills::SkillHandtoHand:
			type = animHand2Hand;
			break;
		default:
			type = animHand2Hand;
			break;
		}// switch
	}
	else {
		skillinuse = EQEmu::skills::SkillHandtoHand;
		type = animHand2Hand;
	}

	// If we're attacking with the secondary hand, play the dual wield anim
	if (Hand == EQEmu::inventory::slotSecondary)	// DW anim
		type = animDualWield;

	DoAnim(type);
	return true;
}

int Mob::compute_tohit(EQEmu::skills::SkillType skillinuse)
{
	int tohit = GetSkill(EQEmu::skills::SkillOffense) + 7;
	tohit += GetSkill(skillinuse);
	if (IsNPC())
		tohit += CastToNPC()->GetAccuracyRating();
	if (IsClient()) {
		double reduction = CastToClient()->m_pp.intoxication / 2.0;
		if (reduction > 20.0) {
			reduction = std::min((110 - reduction) / 100.0, 1.0);
			tohit = reduction * static_cast<double>(tohit);
		} else if (IsBerserk()) {
			tohit += (GetLevel() * 2) / 5;
		}
	}
	return std::max(tohit, 1);
}

// based on dev quotes
// the AGI bonus has actually drastically changed from classic
int Mob::compute_defense()
{
	int defense = GetSkill(EQEmu::skills::SkillDefense) * 400 / 225;
	defense += (8000 * (GetAGI() - 40)) / 36000;
	if (IsClient())
		defense += CastToClient()->GetHeroicAGI() / 10;

	defense += itembonuses.AvoidMeleeChance; // item mod2
	if (IsNPC())
		defense += CastToNPC()->GetAvoidanceRating();

	if (IsClient()) {
		double reduction = CastToClient()->m_pp.intoxication / 2.0;
		if (reduction > 20.0) {
			reduction = std::min((110 - reduction) / 100.0, 1.0);
			defense = reduction * static_cast<double>(defense);
		}
	}

	return std::max(1, defense);
}

// called when a mob is attacked, does the checks to see if it's a hit
// and does other mitigation checks. 'this' is the mob being attacked.
bool Mob::CheckHitChance(Mob* other, EQEmu::skills::SkillType skillinuse, int chance_mod)
{
	Mob *attacker = other;
	Mob *defender = this;
	Log.Out(Logs::Detail, Logs::Attack, "CheckHitChance(%s) attacked by %s", defender->GetName(), attacker->GetName());

	// calculate defender's avoidance
	auto avoidance = defender->compute_defense() + 10; // add 10 in case the NPC's stats are fucked
	auto evasion_bonus = defender->spellbonuses.AvoidMeleeChanceEffect; // we check this first since it has a special case
	if (evasion_bonus <= -100)
		return true;
	if (evasion_bonus >= 10000) // some sort of auto avoid disc
		return false;
	// 172 Evasion aka SE_AvoidMeleeChance
	evasion_bonus += defender->itembonuses.AvoidMeleeChanceEffect + defender->aabonuses.AvoidMeleeChanceEffect; // item bonus here isn't mod2 avoidance

	Mob *owner = nullptr;
	if (defender->IsPet())
		owner = defender->GetOwner();
	else if (defender->IsNPC() && defender->CastToNPC()->GetSwarmOwner())
		owner = entity_list.GetMobID(defender->CastToNPC()->GetSwarmOwner());

	if (owner) // 215 Pet Avoidance % aka SE_PetAvoidance
		evasion_bonus += owner->aabonuses.PetAvoidance + owner->spellbonuses.PetAvoidance + owner->itembonuses.PetAvoidance;

	// Evasion is a percentage bonus according to AA descriptions
	if (evasion_bonus)
		avoidance = (avoidance * (100 + evasion_bonus)) / 100;

	if (chance_mod >= 10000) // override for stuff like SE_SkillAttack
		return true;

	// calculate attacker's accuracy
	auto accuracy = attacker->compute_tohit(skillinuse) + 10; // add 10 in case the NPC's stats are fucked
	if (chance_mod > 0) // multiplier
		accuracy *= chance_mod;

	// Torven parsed an apparent constant of 1.2 somewhere in here * 6 / 5 looks eqmathy to me!
	accuracy = accuracy * 6 / 5;

	// unsure on the stacking order of these effects, rather hard to parse
	// item mod2 accuracy isn't applied to range? Theory crafting and parses back it up I guess
	// mod2 accuracy -- flat bonus
	if (skillinuse != EQEmu::skills::SkillArchery && skillinuse != EQEmu::skills::SkillThrowing)
		accuracy += attacker->itembonuses.HitChance;

	// 216 Melee Accuracy Amt aka SE_Accuracy -- flat bonus
	accuracy += attacker->itembonuses.Accuracy[EQEmu::skills::HIGHEST_SKILL + 1] +
				attacker->aabonuses.Accuracy[EQEmu::skills::HIGHEST_SKILL + 1] +
				attacker->spellbonuses.Accuracy[EQEmu::skills::HIGHEST_SKILL + 1] +
				attacker->itembonuses.Accuracy[skillinuse] +
				attacker->aabonuses.Accuracy[skillinuse] +
				attacker->spellbonuses.Accuracy[skillinuse];

	// auto hit discs (and looks like there are some autohit AAs)
	if (attacker->spellbonuses.HitChanceEffect[skillinuse] >= 10000 || attacker->aabonuses.HitChanceEffect[skillinuse] >= 10000)
		return true;

	if (attacker->spellbonuses.HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1] >= 10000)
		return true;

	// 184 Accuracy % aka SE_HitChance -- percentage increase
	auto hit_bonus = attacker->itembonuses.HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1] +
					 attacker->aabonuses.HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1] +
					 attacker->spellbonuses.HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1] +
					 attacker->itembonuses.HitChanceEffect[skillinuse] +
					 attacker->aabonuses.HitChanceEffect[skillinuse] +
					 attacker->spellbonuses.HitChanceEffect[skillinuse];

	accuracy = (accuracy * (100 + hit_bonus)) / 100;

	// TODO: April 2003 added an archery/throwing PVP accuracy penalty while moving, should be in here some where,
	// but PVP is less important so I haven't tried parsing it at all

	// There is also 110 Ranger Archery Accuracy % which should probably be in here some where, but it's not in any spells/aas
	// Name implies it's a percentage increase, if one wishes to implement, do it like the hit_bonus above but limited to ranger archery

	// There is also 183 UNUSED - Skill Increase Chance which devs say isn't used at all in code, but some spells reference it
	// I do not recommend implementing this once since there are spells that use it which would make this not live-like with default spell files

	// so now we roll!
	// relevant dev quote:
	// Then your chance to simply avoid the attack is checked (defender's avoidance roll beat the attacker's accuracy roll.)
	int tohit_roll = zone->random.Roll0(accuracy);
	int avoid_roll = zone->random.Roll0(avoidance);
	Log.Out(Logs::Detail, Logs::Attack, "CheckHitChance accuracy(%d => %d) avoidance(%d => %d)", accuracy, tohit_roll, avoidance, avoid_roll);

	// tie breaker? Don't want to be biased any one way
	if (tohit_roll == avoid_roll)
		return zone->random.Roll(50);
	return tohit_roll > avoid_roll;
}

bool Mob::AvoidDamage(Mob *other, int32 &damage, int hand)
{
	/* called when a mob is attacked, does the checks to see if it's a hit
	* and does other mitigation checks. 'this' is the mob being attacked.
	*
	* special return values:
	* -1 - block
	* -2 - parry
	* -3 - riposte
	* -4 - dodge
	*
	*/

	/* Order according to current (SoF+?) dev quotes:
	 * https://forums.daybreakgames.com/eq/index.php?threads/test-update-06-10-15.223510/page-2#post-3261772
	 * https://forums.daybreakgames.com/eq/index.php?threads/test-update-06-10-15.223510/page-2#post-3268227
	 * Riposte 50, hDEX, must have weapon/fists, doesn't work on archery/throwing
	 * Block 25, hDEX, works on archery/throwing, behind block done here if back to attacker base1 is chance
	 * Parry 45, hDEX, doesn't work on throwing/archery, must be facing target
	 * Dodge 45, hAGI, works on archery/throwing, monks can dodge attacks from behind
	 * Shield Block, rand base1
	 * Staff Block, rand base1
	 *    regular strike through
	 *    avoiding the attack (CheckHitChance)
	 * As soon as one succeeds, none of the rest are checked
	 *
	 * Formula (all int math)
	 * (posted for parry, assume rest at the same)
	 * Chance = (((SKILL + 100) + [((SKILL+100) * SPA(175).Base1) / 100]) / 45) + [(hDex / 25) - min([hDex / 25], hStrikethrough)].
	 * hStrikethrough is a mob stat that was added to counter the bonuses of heroic stats
	 * Number rolled against 100, if the chance is greater than 100 it happens 100% of time
	 *
	 * Things with 10k accuracy mods can be avoided with these skills qq
	 */
	Mob *attacker = other;
	Mob *defender = this;

	bool InFront = attacker->InFrontMob(this, attacker->GetX(), attacker->GetY());

	/*
	This special ability adds a negative modifer to the defenders riposte/block/parry/chance
	therefore reducing the defenders chance to successfully avoid the melee attack. At present
	time this is the only way to fine tune counter these mods on players. This may
	ultimately end up being more useful as fields in npc_types.
	*/

	int counter_all = 0;
	int counter_riposte = 0;
	int counter_block = 0;
	int counter_parry = 0;
	int counter_dodge = 0;

	if (attacker->GetSpecialAbility(COUNTER_AVOID_DAMAGE)) {
		counter_all = attacker->GetSpecialAbilityParam(COUNTER_AVOID_DAMAGE, 0);
		counter_riposte = attacker->GetSpecialAbilityParam(COUNTER_AVOID_DAMAGE, 1);
		counter_block = attacker->GetSpecialAbilityParam(COUNTER_AVOID_DAMAGE, 2);
		counter_parry = attacker->GetSpecialAbilityParam(COUNTER_AVOID_DAMAGE, 3);
		counter_dodge = attacker->GetSpecialAbilityParam(COUNTER_AVOID_DAMAGE, 4);
	}

	// riposte -- it may seem crazy, but if the attacker has SPA 173 on them, they are immune to Ripo
	bool ImmuneRipo = attacker->aabonuses.RiposteChance || attacker->spellbonuses.RiposteChance || attacker->itembonuses.RiposteChance;
	// Need to check if we have something in MainHand to actually attack with (or fists)
	if (hand != EQEmu::inventory::slotRange && (CanThisClassRiposte() || IsEnraged()) && InFront && !ImmuneRipo) {
		if (IsEnraged()) {
			damage = -3;
			Log.Out(Logs::Detail, Logs::Combat, "I am enraged, riposting frontal attack.");
			return true;
		}
		if (IsClient())
			CastToClient()->CheckIncreaseSkill(EQEmu::skills::SkillRiposte, other, -10);
		// check auto discs ... I guess aa/items too :P
		if (spellbonuses.RiposteChance == 10000 || aabonuses.RiposteChance == 10000 || itembonuses.RiposteChance == 10000) {
			damage = -3;
			return true;
		}
		int chance = GetSkill(EQEmu::skills::SkillRiposte) + 100;
		chance += (chance * (aabonuses.RiposteChance + spellbonuses.RiposteChance + itembonuses.RiposteChance)) / 100;
		chance /= 50;
		chance += itembonuses.HeroicDEX / 25; // live has "heroic strickthrough" here to counter
		if (counter_riposte || counter_all) {
			float counter = (counter_riposte + counter_all) / 100.0f;
			chance -= chance * counter;
		}
		// AA Slippery Attacks
		if (hand == EQEmu::inventory::slotSecondary) {
			int slip = aabonuses.OffhandRiposteFail + itembonuses.OffhandRiposteFail + spellbonuses.OffhandRiposteFail;
			chance += chance * slip / 100;
		}
		if (chance > 0 && zone->random.Roll(chance)) { // could be <0 from offhand stuff
			damage = -3;
			return true;
		}
	}

	// block
	bool bBlockFromRear = false;

	// a successful roll on this does not mean a successful block is forthcoming. only that a chance to block
	// from a direction other than the rear is granted.

	int BlockBehindChance = aabonuses.BlockBehind + spellbonuses.BlockBehind + itembonuses.BlockBehind;

	if (BlockBehindChance && zone->random.Roll(BlockBehindChance))
		bBlockFromRear = true;

	if (CanThisClassBlock() && (InFront || bBlockFromRear)) {
		if (IsClient())
			CastToClient()->CheckIncreaseSkill(EQEmu::skills::SkillBlock, other, -10);
		// check auto discs ... I guess aa/items too :P
		if (spellbonuses.IncreaseBlockChance == 10000 || aabonuses.IncreaseBlockChance == 10000 ||
		    itembonuses.IncreaseBlockChance == 10000) {
			damage = -1;
			return true;
		}
		int chance = GetSkill(EQEmu::skills::SkillBlock) + 100;
		chance += (chance * (aabonuses.IncreaseBlockChance + spellbonuses.IncreaseBlockChance + itembonuses.IncreaseBlockChance)) / 100;
		chance /= 25;
		chance += itembonuses.HeroicDEX / 25; // live has "heroic strickthrough" here to counter
		if (counter_block || counter_all) {
			float counter = (counter_block + counter_all) / 100.0f;
			chance -= chance * counter;
		}
		if (zone->random.Roll(chance)) {
			damage = -1;
			return true;
		}
	}

	// parry
	if (CanThisClassParry() && InFront && hand != EQEmu::inventory::slotRange) {
		if (IsClient())
			CastToClient()->CheckIncreaseSkill(EQEmu::skills::SkillParry, other, -10);
		// check auto discs ... I guess aa/items too :P
		if (spellbonuses.ParryChance == 10000 || aabonuses.ParryChance == 10000 || itembonuses.ParryChance == 10000) {
			damage = -2;
			return true;
		}
		int chance = GetSkill(EQEmu::skills::SkillParry) + 100;
		chance += (chance * (aabonuses.ParryChance + spellbonuses.ParryChance + itembonuses.ParryChance)) / 100;
		chance /= 45;
		chance += itembonuses.HeroicDEX / 25; // live has "heroic strickthrough" here to counter
		if (counter_parry || counter_all) {
			float counter = (counter_parry + counter_all) / 100.0f;
			chance -= chance * counter;
		}
		if (zone->random.Roll(chance)) {
			damage = -2;
			return true;
		}
	}

	// dodge
	if (CanThisClassDodge() && (InFront || GetClass() == MONK) ) {
		if (IsClient())
			CastToClient()->CheckIncreaseSkill(EQEmu::skills::SkillDodge, other, -10);
		// check auto discs ... I guess aa/items too :P
		if (spellbonuses.DodgeChance == 10000 || aabonuses.DodgeChance == 10000 || itembonuses.DodgeChance == 10000) {
			damage = -4;
			return true;
		}
		int chance = GetSkill(EQEmu::skills::SkillDodge) + 100;
		chance += (chance * (aabonuses.DodgeChance + spellbonuses.DodgeChance + itembonuses.DodgeChance)) / 100;
		chance /= 45;
		chance += itembonuses.HeroicAGI / 25; // live has "heroic strickthrough" here to counter
		if (counter_dodge || counter_all) {
			float counter = (counter_dodge + counter_all) / 100.0f;
			chance -= chance * counter;
		}
		if (zone->random.Roll(chance)) {
			damage = -4;
			return true;
		}
	}

	// Try Shield Block OR TwoHandBluntBlockCheck
	if (HasShieldEquiped() && (aabonuses.ShieldBlock || spellbonuses.ShieldBlock || itembonuses.ShieldBlock) && (InFront || bBlockFromRear)) {
		int chance = aabonuses.ShieldBlock + spellbonuses.ShieldBlock + itembonuses.ShieldBlock;
		if (counter_block || counter_all) {
			float counter = (counter_block + counter_all) / 100.0f;
			chance -= chance * counter;
		}
		if (zone->random.Roll(chance)) {
			damage = -1;
			return true;
		}
	}

	if (HasTwoHandBluntEquiped() && (aabonuses.TwoHandBluntBlock || spellbonuses.TwoHandBluntBlock || itembonuses.TwoHandBluntBlock) && (InFront || bBlockFromRear)) {
		int chance = aabonuses.TwoHandBluntBlock + itembonuses.TwoHandBluntBlock + spellbonuses.TwoHandBluntBlock;
		if (counter_block || counter_all) {
			float counter = (counter_block + counter_all) / 100.0f;
			chance -= chance * counter;
		}
		if (zone->random.Roll(chance)) {
			damage = -1;
			return true;
		}
	}

	return false;
}

void Mob::MeleeMitigation(Mob *attacker, int32 &damage, int32 minhit, ExtraAttackOptions *opts)
{
	if (damage <= 0)
		return;

	Mob* defender = this;
	float aa_mit = (aabonuses.CombatStability + itembonuses.CombatStability +
			spellbonuses.CombatStability) / 100.0f;

	if (RuleB(Combat, UseIntervalAC)) {
		float softcap = (GetSkill(EQEmu::skills::SkillDefense) + GetLevel()) *
			RuleR(Combat, SoftcapFactor) * (1.0 + aa_mit);
		float mitigation_rating = 0.0;
		float attack_rating = 0.0;
		int shield_ac = 0;
		int armor = 0;
		float weight = 0.0;

		float monkweight = RuleI(Combat, MonkACBonusWeight);
		monkweight = mod_monk_weight(monkweight, attacker);

		if (IsClient()) {
			armor = CastToClient()->GetRawACNoShield(shield_ac);
			weight = (CastToClient()->CalcCurrentWeight() / 10.0);
		} else if (IsNPC()) {
			armor = CastToNPC()->GetRawAC();
			int PetACBonus = 0;

			if (!IsPet())
				armor = (armor / RuleR(Combat, NPCACFactor));

			Mob *owner = nullptr;
			if (IsPet())
				owner = GetOwner();
			else if ((CastToNPC()->GetSwarmOwner()))
				owner = entity_list.GetMobID(CastToNPC()->GetSwarmOwner());

			if (owner)
				PetACBonus = owner->aabonuses.PetMeleeMitigation + owner->itembonuses.PetMeleeMitigation + owner->spellbonuses.PetMeleeMitigation;

			armor += spellbonuses.AC + itembonuses.AC + PetACBonus + 1;
		}

		if (opts) {
			armor *= (1.0f - opts->armor_pen_percent);
			armor -= opts->armor_pen_flat;
		}

		if (RuleB(Combat, OldACSoftcapRules)) {
			if (GetClass() == WIZARD || GetClass() == MAGICIAN ||
					GetClass() == NECROMANCER || GetClass() == ENCHANTER)
				softcap = RuleI(Combat, ClothACSoftcap);
			else if (GetClass() == MONK && weight <= monkweight)
				softcap = RuleI(Combat, MonkACSoftcap);
			else if(GetClass() == DRUID || GetClass() == BEASTLORD || GetClass() == MONK)
				softcap = RuleI(Combat, LeatherACSoftcap);
			else if(GetClass() == SHAMAN || GetClass() == ROGUE ||
					GetClass() == BERSERKER || GetClass() == RANGER)
				softcap = RuleI(Combat, ChainACSoftcap);
			else
				softcap = RuleI(Combat, PlateACSoftcap);
		}

		softcap += shield_ac;
		armor += shield_ac;
		if (RuleB(Combat, OldACSoftcapRules))
			softcap += (softcap * (aa_mit * RuleR(Combat, AAMitigationACFactor)));
		if (armor > softcap) {
			int softcap_armor = armor - softcap;
			if (RuleB(Combat, OldACSoftcapRules)) {
				if (GetClass() == WARRIOR)
					softcap_armor = softcap_armor * RuleR(Combat, WarriorACSoftcapReturn);
				else if (GetClass() == SHADOWKNIGHT || GetClass() == PALADIN ||
						(GetClass() == MONK && weight <= monkweight))
					softcap_armor = softcap_armor * RuleR(Combat, KnightACSoftcapReturn);
				else if (GetClass() == CLERIC || GetClass() == BARD ||
						GetClass() == BERSERKER || GetClass() == ROGUE ||
						GetClass() == SHAMAN || GetClass() == MONK)
					softcap_armor = softcap_armor * RuleR(Combat, LowPlateChainACSoftcapReturn);
				else if (GetClass() == RANGER || GetClass() == BEASTLORD)
					softcap_armor = softcap_armor * RuleR(Combat, LowChainLeatherACSoftcapReturn);
				else if (GetClass() == WIZARD || GetClass() == MAGICIAN ||
						GetClass() == NECROMANCER || GetClass() == ENCHANTER ||
						GetClass() == DRUID)
					softcap_armor = softcap_armor * RuleR(Combat, CasterACSoftcapReturn);
				else
					softcap_armor = softcap_armor * RuleR(Combat, MiscACSoftcapReturn);
			} else {
				if (GetClass() == WARRIOR)
					softcap_armor *= RuleR(Combat, WarACSoftcapReturn);
				else if (GetClass() == PALADIN || GetClass() == SHADOWKNIGHT)
					softcap_armor *= RuleR(Combat, PalShdACSoftcapReturn);
				else if (GetClass() == CLERIC || GetClass() == RANGER ||
						GetClass() == MONK || GetClass() == BARD)
					softcap_armor *= RuleR(Combat, ClrRngMnkBrdACSoftcapReturn);
				else if (GetClass() == DRUID || GetClass() == NECROMANCER ||
						GetClass() == WIZARD || GetClass() == ENCHANTER ||
						GetClass() == MAGICIAN)
					softcap_armor *= RuleR(Combat, DruNecWizEncMagACSoftcapReturn);
				else if (GetClass() == ROGUE || GetClass() == SHAMAN ||
						GetClass() == BEASTLORD || GetClass() == BERSERKER)
					softcap_armor *= RuleR(Combat, RogShmBstBerACSoftcapReturn);
				else
					softcap_armor *= RuleR(Combat, MiscACSoftcapReturn);
			}
			armor = softcap + softcap_armor;
		}

		if (GetClass() == WIZARD || GetClass() == MAGICIAN ||
				GetClass() == NECROMANCER || GetClass() == ENCHANTER)
				mitigation_rating = ((GetSkill(EQEmu::skills::SkillDefense) + itembonuses.HeroicAGI / 10) / 4.0) + armor + 1;
		else
			mitigation_rating = ((GetSkill(EQEmu::skills::SkillDefense) + itembonuses.HeroicAGI / 10) / 3.0) + (armor * 1.333333) + 1;
		mitigation_rating *= 0.847;

		mitigation_rating = mod_mitigation_rating(mitigation_rating, attacker);

		if (attacker->IsClient())
			attack_rating = (attacker->CastToClient()->CalcATK() + ((attacker->GetSTR() - 66) * 0.9) + (attacker->GetSkill(EQEmu::skills::SkillOffense)*1.345));
		else
			attack_rating = (attacker->GetATK() + (attacker->GetSkill(EQEmu::skills::SkillOffense)*1.345) + ((attacker->GetSTR() - 66) * 0.9));

		attack_rating = attacker->mod_attack_rating(attack_rating, this);

		damage = GetMeleeMitDmg(attacker, damage, minhit, mitigation_rating, attack_rating);
	} else {
		////////////////////////////////////////////////////////
		// Scorpious2k: Include AC in the calculation
		// use serverop variables to set values
		int32 myac = GetAC();
		if(opts) {
			myac *= (1.0f - opts->armor_pen_percent);
			myac -= opts->armor_pen_flat;
		}

		if (damage > 0 && myac > 0) {
			int acfail=1000;
			std::string tmp;

			if (database.GetVariable("ACfail", tmp)) {
				acfail = (int) (atof(tmp.c_str()) * 100);
				if (acfail>100) acfail=100;
			}

			if (acfail<=0 || zone->random.Int(0, 100)>acfail) {
				float acreduction=1;
				int acrandom=300;
				if (database.GetVariable("ACreduction", tmp))
				{
					acreduction=atof(tmp.c_str());
					if (acreduction>100) acreduction=100;
				}

				if (database.GetVariable("ACrandom", tmp))
				{
					acrandom = (int) ((atof(tmp.c_str())+1) * 100);
					if (acrandom>10100) acrandom=10100;
				}

				if (acreduction>0) {
					damage -= (int32) (GetAC() * acreduction/100.0f);
				}
				if (acrandom>0) {
					damage -= (myac * zone->random.Int(0, acrandom) / 10000);
				}
				if (damage<1) damage=1;
				Log.Out(Logs::Detail, Logs::Combat, "AC Damage Reduction: fail chance %d%%. Failed. Reduction %.3f%%, random %d. Resulting damage %d.", acfail, acreduction, acrandom, damage);
			} else {
				Log.Out(Logs::Detail, Logs::Combat, "AC Damage Reduction: fail chance %d%%. Did not fail.", acfail);
			}
		}

		damage -= (aa_mit * damage);

		if(damage != 0 && damage < minhit)
			damage = minhit;
		//reduce the damage from shielding item and aa based on the min dmg
		//spells offer pure mitigation
		damage -= (minhit * defender->itembonuses.MeleeMitigation / 100);
		damage -= (damage * (defender->spellbonuses.MeleeMitigationEffect + defender->itembonuses.MeleeMitigationEffect + defender->aabonuses.MeleeMitigationEffect) / 100);
	}

	if (damage < 0)
		damage = 0;
}

// This is called when the Mob is the one being hit
int32 Mob::GetMeleeMitDmg(Mob *attacker, int32 damage, int32 minhit,
		float mit_rating, float atk_rating)
{
	float d = 10.0;
	float mit_roll = zone->random.Real(0, mit_rating);
	float atk_roll = zone->random.Real(0, atk_rating);

	if (atk_roll > mit_roll) {
		float a_diff = atk_roll - mit_roll;
		float thac0 = atk_rating * RuleR(Combat, ACthac0Factor);
		float thac0cap = attacker->GetLevel() * 9 + 20;
		if (thac0 > thac0cap)
			thac0 = thac0cap;

		d -= 10.0 * (a_diff / thac0);
	} else if (mit_roll > atk_roll) {
		float m_diff = mit_roll - atk_roll;
		float thac20 = mit_rating * RuleR(Combat, ACthac20Factor);
		float thac20cap = GetLevel() * 9 + 20;
		if (thac20 > thac20cap)
			thac20 = thac20cap;

		d += 10.0 * (m_diff / thac20);
	}

	if (d < 0.0)
		d = 0.0;
	else if (d > 20.0)
		d = 20.0;

	float interval = (damage - minhit) / 20.0;
	damage -= ((int)d * interval);

	damage -= (minhit * itembonuses.MeleeMitigation / 100);
	damage -= (damage *  (spellbonuses.MeleeMitigationEffect + itembonuses.MeleeMitigationEffect + aabonuses.MeleeMitigationEffect) / 100);
	return damage;
}

// This is called when the Client is the one being hit
int32 Client::GetMeleeMitDmg(Mob *attacker, int32 damage, int32 minhit,
		float mit_rating, float atk_rating)
{
	if (!attacker->IsNPC() || RuleB(Combat, UseOldDamageIntervalRules))
		return Mob::GetMeleeMitDmg(attacker, damage, minhit, mit_rating, atk_rating);
	int d = 10;
	// floats for the rounding issues
	float dmg_interval = (damage - minhit) / 19.0;
	float dmg_bonus = minhit - dmg_interval;
	float spellMeleeMit =  (spellbonuses.MeleeMitigationEffect + itembonuses.MeleeMitigationEffect + aabonuses.MeleeMitigationEffect) / 100.0;
	if (GetClass() == WARRIOR)
		spellMeleeMit += 0.05;
	dmg_bonus -= dmg_bonus * (itembonuses.MeleeMitigation / 100.0);
	dmg_interval -= dmg_interval * spellMeleeMit;

	float mit_roll = zone->random.Real(0, mit_rating);
	float atk_roll = zone->random.Real(0, atk_rating);

	if (atk_roll > mit_roll) {
		float a_diff = atk_roll - mit_roll;
		float thac0 = atk_rating * RuleR(Combat, ACthac0Factor);
		float thac0cap = attacker->GetLevel() * 9 + 20;
		if (thac0 > thac0cap)
			thac0 = thac0cap;

		d += 10 * (a_diff / thac0);
	} else if (mit_roll > atk_roll) {
		float m_diff = mit_roll - atk_roll;
		float thac20 = mit_rating * RuleR(Combat, ACthac20Factor);
		float thac20cap = GetLevel() * 9 + 20;
		if (thac20 > thac20cap)
			thac20 = thac20cap;

		d -= 10 * (m_diff / thac20);
	}

	if (d < 1)
		d = 1;
	else if (d > 20)
		d = 20;

	return static_cast<int32>((dmg_bonus + dmg_interval * d));
}

//Returns the weapon damage against the input mob
//if we cannot hit the mob with the current weapon we will get a value less than or equal to zero
//Else we know we can hit.
//GetWeaponDamage(mob*, const EQEmu::ItemData*) is intended to be used for mobs or any other situation where we do not have a client inventory item
//GetWeaponDamage(mob*, const EQEmu::ItemInstance*) is intended to be used for situations where we have a client inventory item
int Mob::GetWeaponDamage(Mob *against, const EQEmu::ItemData *weapon_item) {
	int dmg = 0;
	int banedmg = 0;

	//can't hit invulnerable stuff with weapons.
	if(against->GetInvul() || against->GetSpecialAbility(IMMUNE_MELEE)){
		return 0;
	}

	//check to see if our weapons or fists are magical.
	if(against->GetSpecialAbility(IMMUNE_MELEE_NONMAGICAL)){
		if(weapon_item){
			if(weapon_item->Magic){
				dmg = weapon_item->Damage;

				//this is more for non weapon items, ex: boots for kick
				//they don't have a dmg but we should be able to hit magical
				dmg = dmg <= 0 ? 1 : dmg;
			}
			else
				return 0;
		}
		else{
			if((GetClass() == MONK || GetClass() == BEASTLORD) && GetLevel() >= 30){
				dmg = GetHandToHandDamage();
			}
			else if(GetOwner() && GetLevel() >= RuleI(Combat, PetAttackMagicLevel)){
				//pets wouldn't actually use this but...
				//it gives us an idea if we can hit due to the dual nature of this function
				dmg = 1;
			}
			else if(GetSpecialAbility(SPECATK_MAGICAL))
			{
				dmg = 1;
			}
			else
				return 0;
		}
	}
	else{
		if(weapon_item){
			dmg = weapon_item->Damage;

			dmg = dmg <= 0 ? 1 : dmg;
		}
		else{
			dmg = GetHandToHandDamage();
		}
	}

	int eledmg = 0;
	if(!against->GetSpecialAbility(IMMUNE_MAGIC)){
		if(weapon_item && weapon_item->ElemDmgAmt){
			//we don't check resist for npcs here
			eledmg = weapon_item->ElemDmgAmt;
			dmg += eledmg;
		}
	}

	if(against->GetSpecialAbility(IMMUNE_MELEE_EXCEPT_BANE)){
		if(weapon_item){
			if(weapon_item->BaneDmgBody == against->GetBodyType()){
				banedmg += weapon_item->BaneDmgAmt;
			}

			if(weapon_item->BaneDmgRace == against->GetRace()){
				banedmg += weapon_item->BaneDmgRaceAmt;
			}
		}

		if(!banedmg){
			if(!GetSpecialAbility(SPECATK_BANE))
				return 0;
			else
				return 1;
		}
		else
			dmg += banedmg;
	}
	else{
		if(weapon_item){
			if(weapon_item->BaneDmgBody == against->GetBodyType()){
				banedmg += weapon_item->BaneDmgAmt;
			}

			if(weapon_item->BaneDmgRace == against->GetRace()){
				banedmg += weapon_item->BaneDmgRaceAmt;
			}
		}

		dmg += (banedmg + eledmg);
	}

	if(dmg <= 0){
		return 0;
	}
	else
		return dmg;
}

int Mob::GetWeaponDamage(Mob *against, const EQEmu::ItemInstance *weapon_item, uint32 *hate)
{
	int dmg = 0;
	int banedmg = 0;
	int x = 0;

	if (!against || against->GetInvul() || against->GetSpecialAbility(IMMUNE_MELEE))
		return 0;

	// check for items being illegally attained
	if (weapon_item) {
		if (!weapon_item->GetItem())
			return 0;

		if (weapon_item->GetItemRequiredLevel(true) > GetLevel())
			return 0;

		if (!weapon_item->IsEquipable(GetBaseRace(), GetClass()))
			return 0;
	}

	if (against->GetSpecialAbility(IMMUNE_MELEE_NONMAGICAL)) {
		if (weapon_item) {
			// check to see if the weapon is magic
			bool MagicWeapon = weapon_item->GetItemMagical(true) || spellbonuses.MagicWeapon || itembonuses.MagicWeapon;
			if (MagicWeapon) {
				auto rec_level = weapon_item->GetItemRecommendedLevel(true);
				if (IsClient() && GetLevel() < rec_level)
					dmg = CastToClient()->CalcRecommendedLevelBonus(
					    GetLevel(), rec_level, weapon_item->GetItemWeaponDamage(true));
				else
					dmg = weapon_item->GetItemWeaponDamage(true);
				dmg = dmg <= 0 ? 1 : dmg;
			} else {
				return 0;
			}
		} else {
			bool MagicGloves = false;
			if (IsClient()) {
				const EQEmu::ItemInstance *gloves = CastToClient()->GetInv().GetItem(EQEmu::inventory::slotHands);
				if (gloves)
					MagicGloves = gloves->GetItemMagical(true);
			}

			if (GetClass() == MONK || GetClass() == BEASTLORD) {
				if (MagicGloves || GetLevel() >= 30) {
					dmg = GetHandToHandDamage();
					if (hate)
						*hate += dmg;
				}
			} else if (GetOwner() &&
				   GetLevel() >=
				       RuleI(Combat, PetAttackMagicLevel)) { // pets wouldn't actually use this but...
				dmg = 1; // it gives us an idea if we can hit
			} else if (MagicGloves || GetSpecialAbility(SPECATK_MAGICAL)) {
				dmg = 1;
			} else
				return 0;
		}
	} else {
		if (weapon_item) {
			if (weapon_item->GetItem()) {
				auto rec_level = weapon_item->GetItemRecommendedLevel(true);
				if (IsClient() && GetLevel() < rec_level) {
					dmg = CastToClient()->CalcRecommendedLevelBonus(
					    GetLevel(), rec_level, weapon_item->GetItemWeaponDamage(true));
				} else {
					dmg = weapon_item->GetItemWeaponDamage(true);
				}

				dmg = dmg <= 0 ? 1 : dmg;
			}
		} else {
			dmg = GetHandToHandDamage();
			if (hate)
				*hate += dmg;
		}
	}

	int eledmg = 0;
	if (!against->GetSpecialAbility(IMMUNE_MAGIC)) {
		if (weapon_item && weapon_item->GetItem() && weapon_item->GetItemElementalFlag(true))
			// the client actually has the way this is done, it does not appear to check req!
			eledmg = against->ResistElementalWeaponDmg(weapon_item);
	}

	if (weapon_item && weapon_item->GetItem() &&
			(weapon_item->GetItemBaneDamageBody(true) || weapon_item->GetItemBaneDamageRace(true)))
		banedmg = against->CheckBaneDamage(weapon_item);

	if (against->GetSpecialAbility(IMMUNE_MELEE_EXCEPT_BANE)) {
		if (!banedmg) {
			if (!GetSpecialAbility(SPECATK_BANE))
				return 0;
			else
				return 1;
		} else {
			dmg += (banedmg + eledmg);
			if (hate)
				*hate += banedmg;
		}
	} else {
		dmg += (banedmg + eledmg);
		if (hate)
			*hate += banedmg;
	}

	return std::max(0, dmg);
}

//note: throughout this method, setting `damage` to a negative is a way to
//stop the attack calculations
// IsFromSpell added to allow spell effects to use Attack. (Mainly for the Rampage AA right now.)
bool Client::Attack(Mob* other, int Hand, bool bRiposte, bool IsStrikethrough, bool IsFromSpell, ExtraAttackOptions *opts, int special)
{
	if (!other) {
		SetTarget(nullptr);
		Log.Out(Logs::General, Logs::Error, "A null Mob object was passed to Client::Attack() for evaluation!");
		return false;
	}

	if(!GetTarget())
		SetTarget(other);

	Log.Out(Logs::Detail, Logs::Combat, "Attacking %s with hand %d %s", other?other->GetName():"(nullptr)", Hand, bRiposte?"(this is a riposte)":"");

	//SetAttackTimer();
	if (
		(IsCasting() && GetClass() != BARD && !IsFromSpell)
		|| other == nullptr
		|| ((IsClient() && CastToClient()->dead) || (other->IsClient() && other->CastToClient()->dead))
		|| (GetHP() < 0)
		|| (!IsAttackAllowed(other))
		) {
		Log.Out(Logs::Detail, Logs::Combat, "Attack canceled, invalid circumstances.");
		return false; // Only bards can attack while casting
	}

	if(DivineAura() && !GetGM()) {//cant attack while invulnerable unless your a gm
		Log.Out(Logs::Detail, Logs::Combat, "Attack canceled, Divine Aura is in effect.");
		Message_StringID(MT_DefaultText, DIVINE_AURA_NO_ATK);	//You can't attack while invulnerable!
		return false;
	}

	if (GetFeigned())
		return false; // Rogean: How can you attack while feigned? Moved up from Aggro Code.

	EQEmu::ItemInstance* weapon;
	if (Hand == EQEmu::inventory::slotSecondary){	// Kaiyodo - Pick weapon from the attacking hand
		weapon = GetInv().GetItem(EQEmu::inventory::slotSecondary);
		OffHandAtk(true);
	}
	else{
		weapon = GetInv().GetItem(EQEmu::inventory::slotPrimary);
		OffHandAtk(false);
	}

	if(weapon != nullptr) {
		if (!weapon->IsWeapon()) {
			Log.Out(Logs::Detail, Logs::Combat, "Attack canceled, Item %s (%d) is not a weapon.", weapon->GetItem()->Name, weapon->GetID());
			return(false);
		}
		Log.Out(Logs::Detail, Logs::Combat, "Attacking with weapon: %s (%d)", weapon->GetItem()->Name, weapon->GetID());
	} else {
		Log.Out(Logs::Detail, Logs::Combat, "Attacking without a weapon.");
	}

	// calculate attack_skill and skillinuse depending on hand and weapon
	// also send Packet to near clients
	EQEmu::skills::SkillType skillinuse;
	AttackAnimation(skillinuse, Hand, weapon);
	Log.Out(Logs::Detail, Logs::Combat, "Attacking with %s in slot %d using skill %d", weapon?weapon->GetItem()->Name:"Fist", Hand, skillinuse);

	/// Now figure out damage
	int damage = 0;
	uint8 mylevel = GetLevel() ? GetLevel() : 1;
	uint32 hate = 0;
	if (weapon) hate = weapon->GetItem()->Damage + weapon->GetItem()->ElemDmgAmt;
	int weapon_damage = GetWeaponDamage(other, weapon, &hate);
	if (hate == 0 && weapon_damage > 1) hate = weapon_damage;

	//if weapon damage > 0 then we know we can hit the target with this weapon
	//otherwise we cannot and we set the damage to -5 later on
	if(weapon_damage > 0){
		auto shield_inc = spellbonuses.ShieldEquipDmgMod + itembonuses.ShieldEquipDmgMod + aabonuses.ShieldEquipDmgMod;
		if (shield_inc > 0 && HasShieldEquiped() && Hand == EQEmu::inventory::slotPrimary) {
			weapon_damage = weapon_damage * (100 + shield_inc) / 100;
			hate = hate * (100 + shield_inc) / 100;
		}

		//Berserker Berserk damage bonus
		if(IsBerserk() && GetClass() == BERSERKER){
			int bonus = 3 + GetLevel()/10;		//unverified
			weapon_damage = weapon_damage * (100+bonus) / 100;
			Log.Out(Logs::Detail, Logs::Combat, "Berserker damage bonus increases DMG to %d", weapon_damage);
		}

		//try a finishing blow.. if successful end the attack
		if(TryFinishingBlow(other, skillinuse))
			return (true);

		int min_hit = 1;
		int max_hit = (2*weapon_damage*GetDamageTable(skillinuse)) / 100;

		if(GetLevel() < 10 && max_hit > RuleI(Combat, HitCapPre10))
			max_hit = (RuleI(Combat, HitCapPre10));
		else if(GetLevel() < 20 && max_hit > RuleI(Combat, HitCapPre20))
			max_hit = (RuleI(Combat, HitCapPre20));

		CheckIncreaseSkill(skillinuse, other, -15);
		CheckIncreaseSkill(EQEmu::skills::SkillOffense, other, -15);

		// ***************************************************************
		// *** Calculate the damage bonus, if applicable, for this hit ***
		// ***************************************************************

#ifndef EQEMU_NO_WEAPON_DAMAGE_BONUS

		// If you include the preprocessor directive "#define EQEMU_NO_WEAPON_DAMAGE_BONUS", that indicates that you do not
		// want damage bonuses added to weapon damage at all. This feature was requested by ChaosSlayer on the EQEmu Forums.
		//
		// This is not recommended for normal usage, as the damage bonus represents a non-trivial component of the DPS output
		// of weapons wielded by higher-level melee characters (especially for two-handed weapons).

		int ucDamageBonus = 0;

		if (Hand == EQEmu::inventory::slotPrimary && GetLevel() >= 28 && IsWarriorClass())
		{
			// Damage bonuses apply only to hits from the main hand (Hand == MainPrimary) by characters level 28 and above
			// who belong to a melee class. If we're here, then all of these conditions apply.

			ucDamageBonus = GetWeaponDamageBonus(weapon ? weapon->GetItem() : (const EQEmu::ItemData*) nullptr);

			min_hit += (int) ucDamageBonus;
			max_hit += (int) ucDamageBonus;
			hate += ucDamageBonus;
		}
#endif
		//Live AA - Sinister Strikes *Adds weapon damage bonus to offhand weapon.
		if (Hand == EQEmu::inventory::slotSecondary) {
			if (aabonuses.SecondaryDmgInc || itembonuses.SecondaryDmgInc || spellbonuses.SecondaryDmgInc){

				ucDamageBonus = GetWeaponDamageBonus(weapon ? weapon->GetItem() : (const EQEmu::ItemData*) nullptr, true);

				min_hit += (int) ucDamageBonus;
				max_hit += (int) ucDamageBonus;
				hate += ucDamageBonus;
			}
		}

		// this effect is actually a min cap that happens after the final damage is calculated
		min_hit += min_hit * GetMeleeMinDamageMod_SE(skillinuse) / 100;

		if(max_hit < min_hit)
			max_hit = min_hit;

		if(RuleB(Combat, UseIntervalAC))
			damage = max_hit;
		else
			damage = zone->random.Int(min_hit, max_hit);

		damage = mod_client_damage(damage, skillinuse, Hand, weapon, other);

		Log.Out(Logs::Detail, Logs::Combat, "Damage calculated to %d (min %d, max %d, str %d, skill %d, DMG %d, lv %d)",
			damage, min_hit, max_hit, GetSTR(), GetSkill(skillinuse), weapon_damage, mylevel);

		int hit_chance_bonus = 0;

		if(opts) {
			damage *= opts->damage_percent;
			damage += opts->damage_flat;
			hate *= opts->hate_percent;
			hate += opts->hate_flat;
			hit_chance_bonus += opts->hit_chance;
		}

		//check to see if we hit..
		if (other->AvoidDamage(this, damage, Hand)) {
			if (!bRiposte && !IsStrikethrough) {
				int strike_through = itembonuses.StrikeThrough + spellbonuses.StrikeThrough + aabonuses.StrikeThrough;
				if(strike_through && zone->random.Roll(strike_through)) {
					Message_StringID(MT_StrikeThrough, STRIKETHROUGH_STRING); // You strike through your opponents defenses!
					Attack(other, Hand, false, true); // Strikethrough only gives another attempted hit
					return false;
				}
				// I'm pretty sure you can riposte a riposte
				if (damage == -3 && !bRiposte) {
					DoRiposte(other);
					if (IsDead())
						return false;
				}
			}
			Log.Out(Logs::Detail, Logs::Combat, "Avoided damage with code %d", damage);
		} else {
			if (other->CheckHitChance(this, skillinuse, hit_chance_bonus)) {
				other->MeleeMitigation(this, damage, min_hit, opts);
				if (damage > 0)
					CommonOutgoingHitSuccess(other, damage, skillinuse,opts);
				Log.Out(Logs::Detail, Logs::Combat, "Final damage after all reductions: %d", damage);
			} else {
				Log.Out(Logs::Detail, Logs::Combat, "Attack missed. Damage set to 0.");
				damage = 0;
			}
		}
	} else {
		damage = -5;
	}

	// Hate Generation is on a per swing basis, regardless of a hit, miss, or block, its always the same.
	// If we are this far, this means we are atleast making a swing.

	other->AddToHateList(this, hate);

	///////////////////////////////////////////////////////////
	////// Send Attack Damage
	///////////////////////////////////////////////////////////
	if (damage > 0 && aabonuses.SkillAttackProc[0] && aabonuses.SkillAttackProc[1] == skillinuse &&
	    IsValidSpell(aabonuses.SkillAttackProc[2])) {
		float chance = aabonuses.SkillAttackProc[0] / 1000.0f;
		if (zone->random.Roll(chance))
			SpellFinished(aabonuses.SkillAttackProc[2], other, EQEmu::CastingSlot::Item, 0, -1,
				      spells[aabonuses.SkillAttackProc[2]].ResistDiff);
	}
	other->Damage(this, damage, SPELL_UNKNOWN, skillinuse, true, -1, false, special);

	if (IsDead()) return false;

	MeleeLifeTap(damage);

	if (damage > 0 && HasSkillProcSuccess() && other && other->GetHP() > 0)
		TrySkillProc(other, skillinuse, 0, true, Hand);

	CommonBreakInvisibleFromCombat();

	if(GetTarget())
		TriggerDefensiveProcs(other, Hand, true, damage);

	if (damage > 0)
		return true;

	else
		return false;
}

//used by complete heal and #heal
void Mob::Heal()
{
	SetMaxHP();
	SendHPUpdate();
}

void Client::Damage(Mob* other, int32 damage, uint16 spell_id, EQEmu::skills::SkillType attack_skill, bool avoidable, int8 buffslot, bool iBuffTic, int special)
{
	if(dead || IsCorpse())
		return;

	if(spell_id==0)
		spell_id = SPELL_UNKNOWN;

	// cut all PVP spell damage to 2/3
	// Blasting ourselfs is considered PvP
	//Don't do PvP mitigation if the caster is damaging himself
	if(other && other->IsClient() && (other != this) && damage > 0) {
		int PvPMitigation = 100;
		if (attack_skill == EQEmu::skills::SkillArchery)
			PvPMitigation = 80;
		else
			PvPMitigation = 67;
		damage = (damage * PvPMitigation) / 100;
	}

	if(!ClientFinishedLoading())
		damage = -5;

	//do a majority of the work...
	CommonDamage(other, damage, spell_id, attack_skill, avoidable, buffslot, iBuffTic, special);

	if (damage > 0) {

		if (spell_id == SPELL_UNKNOWN)
			CheckIncreaseSkill(EQEmu::skills::SkillDefense, other, -15);
	}
}

bool Client::Death(Mob* killerMob, int32 damage, uint16 spell, EQEmu::skills::SkillType attack_skill)
{
	if(!ClientFinishedLoading())
		return false;

	if(dead)
		return false;	//cant die more than once...

	if(!spell)
		spell = SPELL_UNKNOWN;

	char buffer[48] = { 0 };
	snprintf(buffer, 47, "%d %d %d %d", killerMob ? killerMob->GetID() : 0, damage, spell, static_cast<int>(attack_skill));
	if(parse->EventPlayer(EVENT_DEATH, this, buffer, 0) != 0) {
		if(GetHP() < 0) {
			SetHP(0);
		}
		return false;
	}

	if(killerMob && killerMob->IsClient() && (spell != SPELL_UNKNOWN) && damage > 0) {
		char val1[20]={0};
		entity_list.MessageClose_StringID(this, false, 100, MT_NonMelee, HIT_NON_MELEE,
			killerMob->GetCleanName(), GetCleanName(), ConvertArray(damage, val1));
	}

	int exploss = 0;
	Log.Out(Logs::Detail, Logs::Combat, "Fatal blow dealt by %s with %d damage, spell %d, skill %d", killerMob ? killerMob->GetName() : "Unknown", damage, spell, attack_skill);

	/*
		#1: Send death packet to everyone
	*/
	uint8 killed_level = GetLevel();

	SendLogoutPackets();

	/* Make self become corpse packet */
	EQApplicationPacket app2(OP_BecomeCorpse, sizeof(BecomeCorpse_Struct));
	BecomeCorpse_Struct* bc = (BecomeCorpse_Struct*)app2.pBuffer;
	bc->spawn_id = GetID();
	bc->x = GetX();
	bc->y = GetY();
	bc->z = GetZ();
	QueuePacket(&app2);

	/* Make Death Packet */
	EQApplicationPacket app(OP_Death, sizeof(Death_Struct));
	Death_Struct* d = (Death_Struct*)app.pBuffer;
	d->spawn_id = GetID();
	d->killer_id = killerMob ? killerMob->GetID() : 0;
	d->corpseid=GetID();
	d->bindzoneid = m_pp.binds[0].zoneId;
	d->spell_id = spell == SPELL_UNKNOWN ? 0xffffffff : spell;
	d->attack_skill = spell != SPELL_UNKNOWN ? 0xe7 : attack_skill;
	d->damage = damage;
	app.priority = 6;
	entity_list.QueueClients(this, &app);

	/*
		#2: figure out things that affect the player dying and mark them dead
	*/

	InterruptSpell();
	SetPet(0);
	SetHorseId(0);
	dead = true;

	if(GetMerc()) {
		GetMerc()->Suspend();
	}

	if (killerMob != nullptr)
	{
		if (killerMob->IsNPC()) {
			parse->EventNPC(EVENT_SLAY, killerMob->CastToNPC(), this, "", 0);

			mod_client_death_npc(killerMob);

			uint16 emoteid = killerMob->GetEmoteID();
			if(emoteid != 0)
				killerMob->CastToNPC()->DoNPCEmote(KILLEDPC,emoteid);
			killerMob->TrySpellOnKill(killed_level,spell);
		}

		if(killerMob->IsClient() && (IsDueling() || killerMob->CastToClient()->IsDueling())) {
			SetDueling(false);
			SetDuelTarget(0);
			if (killerMob->IsClient() && killerMob->CastToClient()->IsDueling() && killerMob->CastToClient()->GetDuelTarget() == GetID())
			{
				//if duel opponent killed us...
				killerMob->CastToClient()->SetDueling(false);
				killerMob->CastToClient()->SetDuelTarget(0);
				entity_list.DuelMessage(killerMob,this,false);

				mod_client_death_duel(killerMob);

			} else {
				//otherwise, we just died, end the duel.
				Mob* who = entity_list.GetMob(GetDuelTarget());
				if(who && who->IsClient()) {
					who->CastToClient()->SetDueling(false);
					who->CastToClient()->SetDuelTarget(0);
				}
			}
		}
	}

	entity_list.RemoveFromTargets(this);
	hate_list.RemoveEntFromHateList(this);
	RemoveAutoXTargets();

	//remove ourself from all proximities
	ClearAllProximities();

	/*
		#3: exp loss and corpse generation
	*/

	// figure out if they should lose exp
	if(RuleB(Character, UseDeathExpLossMult)){
		float GetNum [] = {0.005f,0.015f,0.025f,0.035f,0.045f,0.055f,0.065f,0.075f,0.085f,0.095f,0.110f };
		int Num = RuleI(Character, DeathExpLossMultiplier);
		if((Num < 0) || (Num > 10))
			Num = 3;
		float loss = GetNum[Num];
		exploss=(int)((float)GetEXP() * (loss)); //loose % of total XP pending rule (choose 0-10)
	}

	if(!RuleB(Character, UseDeathExpLossMult)){
		exploss = (int)(GetLevel() * (GetLevel() / 18.0) * 12000);
	}

	if( (GetLevel() < RuleI(Character, DeathExpLossLevel)) || (GetLevel() > RuleI(Character, DeathExpLossMaxLevel)) || IsBecomeNPC() )
	{
		exploss = 0;
	}
	else if( killerMob )
	{
		if( killerMob->IsClient() )
		{
			exploss = 0;
		}
		else if( killerMob->GetOwner() && killerMob->GetOwner()->IsClient() )
		{
			exploss = 0;
		}
	}

	if(spell != SPELL_UNKNOWN)
	{
		uint32 buff_count = GetMaxTotalSlots();
		for(uint16 buffIt = 0; buffIt < buff_count; buffIt++)
		{
			if(buffs[buffIt].spellid == spell && buffs[buffIt].client)
			{
				exploss = 0;	// no exp loss for pvp dot
				break;
			}
		}
	}

	bool LeftCorpse = false;

	// now we apply the exp loss, unmem their spells, and make a corpse
	// unless they're a GM (or less than lvl 10
	if(!GetGM())
	{
		if(exploss > 0) {
			int32 newexp = GetEXP();
			if(exploss > newexp) {
				//lost more than we have... wtf..
				newexp = 1;
			} else {
				newexp -= exploss;
			}
			SetEXP(newexp, GetAAXP());
			//m_epp.perAA = 0;	//reset to no AA exp on death.
		}

		//this generates a lot of 'updates' to the client that the client does not need
		BuffFadeNonPersistDeath();
		if (RuleB(Character, UnmemSpellsOnDeath)) {
			if((ClientVersionBit() & EQEmu::versions::bit_SoFAndLater) && RuleB(Character, RespawnFromHover))
				UnmemSpellAll(true);
			else
				UnmemSpellAll(false);
		}

		if((RuleB(Character, LeaveCorpses) && GetLevel() >= RuleI(Character, DeathItemLossLevel)) || RuleB(Character, LeaveNakedCorpses))
		{
			// creating the corpse takes the cash/items off the player too
			auto new_corpse = new Corpse(this, exploss);

			std::string tmp;
			database.GetVariable("ServerType", tmp);
			if(tmp[0] == '1' && tmp[1] == '\0' && killerMob != nullptr && killerMob->IsClient()){
				database.GetVariable("PvPreward", tmp);
				int reward = atoi(tmp.c_str());
				if(reward==3){
					database.GetVariable("PvPitem", tmp);
					int pvpitem = atoi(tmp.c_str());
					if(pvpitem>0 && pvpitem<200000)
						new_corpse->SetPlayerKillItemID(pvpitem);
				}
				else if(reward==2)
					new_corpse->SetPlayerKillItemID(-1);
				else if(reward==1)
					new_corpse->SetPlayerKillItemID(1);
				else
					new_corpse->SetPlayerKillItemID(0);
				if(killerMob->CastToClient()->isgrouped) {
					Group* group = entity_list.GetGroupByClient(killerMob->CastToClient());
					if(group != 0)
					{
						for(int i=0;i<6;i++)
						{
							if(group->members[i] != nullptr)
							{
								new_corpse->AllowPlayerLoot(group->members[i],i);
							}
						}
					}
				}
			}

			entity_list.AddCorpse(new_corpse, GetID());
			SetID(0);

			//send the become corpse packet to everybody else in the zone.
			entity_list.QueueClients(this, &app2, true);

			LeftCorpse = true;
		}
	} else {
		BuffFadeDetrimental();
	}

	/*
		Finally, send em home

		We change the mob variables, not pp directly, because Save() will copy
		from these and overwrite what we set in pp anyway
	*/

	if(LeftCorpse && (ClientVersionBit() & EQEmu::versions::bit_SoFAndLater) && RuleB(Character, RespawnFromHover))
	{
		ClearDraggedCorpses();
		RespawnFromHoverTimer.Start(RuleI(Character, RespawnFromHoverTimer) * 1000);
		SendRespawnBinds();
	}
	else
	{
		if(isgrouped)
		{
			Group *g = GetGroup();
			if(g)
				g->MemberZoned(this);
		}

		Raid* r = entity_list.GetRaidByClient(this);

		if(r)
			r->MemberZoned(this);

		dead_timer.Start(5000, true);
		m_pp.zone_id = m_pp.binds[0].zoneId;
		m_pp.zoneInstance = m_pp.binds[0].instance_id;
		database.MoveCharacterToZone(this->CharacterID(), database.GetZoneName(m_pp.zone_id));
		Save();
		GoToDeath();
	}

	/* QS: PlayerLogDeaths */
	if (RuleB(QueryServ, PlayerLogDeaths)){
		const char * killer_name = "";
		if (killerMob && killerMob->GetCleanName()){ killer_name = killerMob->GetCleanName(); }
		std::string event_desc = StringFormat("Died in zoneid:%i instid:%i by '%s', spellid:%i, damage:%i", this->GetZoneID(), this->GetInstanceID(), killer_name, spell, damage);
		QServ->PlayerLogEvent(Player_Log_Deaths, this->CharacterID(), event_desc);
	}

	parse->EventPlayer(EVENT_DEATH_COMPLETE, this, buffer, 0);
	return true;
}

bool NPC::Attack(Mob* other, int Hand, bool bRiposte, bool IsStrikethrough, bool IsFromSpell, ExtraAttackOptions *opts, int special)
{
	int damage = 0;

	if (!other) {
		SetTarget(nullptr);
		Log.Out(Logs::General, Logs::Error, "A null Mob object was passed to NPC::Attack() for evaluation!");
		return false;
	}

	if(DivineAura())
		return(false);

	if(!GetTarget())
		SetTarget(other);

	//Check that we can attack before we calc heading and face our target
	if (!IsAttackAllowed(other)) {
		if (this->GetOwnerID())
			this->Say_StringID(NOT_LEGAL_TARGET);
		if(other) {
			if (other->IsClient())
				other->CastToClient()->RemoveXTarget(this, false);
			RemoveFromHateList(other);
			Log.Out(Logs::Detail, Logs::Combat, "I am not allowed to attack %s", other->GetName());
		}
		return false;
	}

	FaceTarget(GetTarget());

	EQEmu::skills::SkillType skillinuse = EQEmu::skills::SkillHandtoHand;
	if (Hand == EQEmu::inventory::slotPrimary) {
		skillinuse = static_cast<EQEmu::skills::SkillType>(GetPrimSkill());
		OffHandAtk(false);
	}
	if (Hand == EQEmu::inventory::slotSecondary) {
		skillinuse = static_cast<EQEmu::skills::SkillType>(GetSecSkill());
		OffHandAtk(true);
	}

	//figure out what weapon they are using, if any
	const EQEmu::ItemData* weapon = nullptr;
	if (Hand == EQEmu::inventory::slotPrimary && equipment[EQEmu::inventory::slotPrimary] > 0)
		weapon = database.GetItem(equipment[EQEmu::inventory::slotPrimary]);
	else if (equipment[EQEmu::inventory::slotSecondary])
		weapon = database.GetItem(equipment[EQEmu::inventory::slotSecondary]);

	//We dont factor much from the weapon into the attack.
	//Just the skill type so it doesn't look silly using punching animations and stuff while wielding weapons
	if(weapon) {
		Log.Out(Logs::Detail, Logs::Combat, "Attacking with weapon: %s (%d) (too bad im not using it for much)", weapon->Name, weapon->ID);

		if (Hand == EQEmu::inventory::slotSecondary && weapon->ItemType == EQEmu::item::ItemTypeShield){
			Log.Out(Logs::Detail, Logs::Combat, "Attack with shield canceled.");
			return false;
		}

		switch(weapon->ItemType) {
		case EQEmu::item::ItemType1HSlash:
			skillinuse = EQEmu::skills::Skill1HSlashing;
			break;
		case EQEmu::item::ItemType2HSlash:
			skillinuse = EQEmu::skills::Skill2HSlashing;
			break;
		case EQEmu::item::ItemType1HPiercing:
			skillinuse = EQEmu::skills::Skill1HPiercing;
			break;
		case EQEmu::item::ItemType2HPiercing:
			skillinuse = EQEmu::skills::Skill2HPiercing;
			break;
		case EQEmu::item::ItemType1HBlunt:
			skillinuse = EQEmu::skills::Skill1HBlunt;
			break;
		case EQEmu::item::ItemType2HBlunt:
			skillinuse = EQEmu::skills::Skill2HBlunt;
			break;
		case EQEmu::item::ItemTypeBow:
			skillinuse = EQEmu::skills::SkillArchery;
			break;
		case EQEmu::item::ItemTypeLargeThrowing:
		case EQEmu::item::ItemTypeSmallThrowing:
			skillinuse = EQEmu::skills::SkillThrowing;
			break;
		default:
			skillinuse = EQEmu::skills::SkillHandtoHand;
			break;
		}
	}

	int weapon_damage = GetWeaponDamage(other, weapon);

	//do attack animation regardless of whether or not we can hit below
	int16 charges = 0;
	EQEmu::ItemInstance weapon_inst(weapon, charges);
	AttackAnimation(skillinuse, Hand, &weapon_inst);

	//basically "if not immune" then do the attack
	if((weapon_damage) > 0) {

		//ele and bane dmg too
		//NPCs add this differently than PCs
		//if NPCs can't inheriently hit the target we don't add bane/magic dmg which isn't exactly the same as PCs
		uint32 eleBane = 0;
		if(weapon){
			if(weapon->BaneDmgBody == other->GetBodyType()){
				eleBane += weapon->BaneDmgAmt;
			}

			if(weapon->BaneDmgRace == other->GetRace()){
				eleBane += weapon->BaneDmgRaceAmt;
			}

			if(weapon->ElemDmgAmt){
				eleBane += (weapon->ElemDmgAmt * other->ResistSpell(weapon->ElemDmgType, 0, this) / 100);
			}
		}

		if(!RuleB(NPC, UseItemBonusesForNonPets)){
			if(!GetOwner()){
				eleBane = 0;
			}
		}

		uint8 otherlevel = other->GetLevel();
		uint8 mylevel = this->GetLevel();

		otherlevel = otherlevel ? otherlevel : 1;
		mylevel = mylevel ? mylevel : 1;

		//instead of calcing damage in floats lets just go straight to ints
		if(RuleB(Combat, UseIntervalAC))
			damage = (max_dmg+eleBane);
		else
			damage = zone->random.Int((min_dmg+eleBane),(max_dmg+eleBane));

		//check if we're hitting above our max or below it.
		if((min_dmg+eleBane) != 0 && damage < (min_dmg+eleBane)) {
			Log.Out(Logs::Detail, Logs::Combat, "Damage (%d) is below min (%d). Setting to min.", damage, (min_dmg+eleBane));
			damage = (min_dmg+eleBane);
		}
		if((max_dmg+eleBane) != 0 && damage > (max_dmg+eleBane)) {
			Log.Out(Logs::Detail, Logs::Combat, "Damage (%d) is above max (%d). Setting to max.", damage, (max_dmg+eleBane));
			damage = (max_dmg+eleBane);
		}

		damage = mod_npc_damage(damage, skillinuse, Hand, weapon, other);

		int32 hate = damage;
		if(IsPet())
		{
			hate = hate * 100 / GetDamageTable(skillinuse);
		}

		if(other->IsClient() && other->CastToClient()->IsSitting()) {
			Log.Out(Logs::Detail, Logs::Combat, "Client %s is sitting. Hitting for max damage (%d).", other->GetName(), (max_dmg+eleBane));
			damage = (max_dmg+eleBane);
			damage += (itembonuses.HeroicSTR / 10) + (damage * other->GetSkillDmgTaken(skillinuse, opts) / 100) + GetSkillDmgAmt(skillinuse);

			if(opts) {
				damage *= opts->damage_percent;
				damage += opts->damage_flat;
				hate *= opts->hate_percent;
				hate += opts->hate_flat;
			}

			Log.Out(Logs::Detail, Logs::Combat, "Generating hate %d towards %s", hate, GetName());
			// now add done damage to the hate list
			other->AddToHateList(this, hate);

		} else {

			int hit_chance_bonus = 0;

			if(opts) {
				damage *= opts->damage_percent;
				damage += opts->damage_flat;
				hate *= opts->hate_percent;
				hate += opts->hate_flat;
				hit_chance_bonus += opts->hit_chance;
			}

			if (other->AvoidDamage(this, damage, Hand)) {
				if (!bRiposte && damage == -3)
					DoRiposte(other);
			} else {
				if (other->CheckHitChance(this, skillinuse, hit_chance_bonus)) {
					other->MeleeMitigation(this, damage, min_dmg+eleBane, opts);
					CommonOutgoingHitSuccess(other, damage, skillinuse, opts);
				} else {
					damage = 0;
				}
			}
			other->AddToHateList(this, hate);
		}

		Log.Out(Logs::Detail, Logs::Combat, "Final damage against %s: %d", other->GetName(), damage);

		if(other->IsClient() && IsPet() && GetOwner()->IsClient()) {
			//pets do half damage to clients in pvp
			damage=damage/2;
		}
	}
	else
		damage = -5;

	if(GetHP() > 0 && !other->HasDied()) {
		other->Damage(this, damage, SPELL_UNKNOWN, skillinuse, true, -1, false, special); // Not avoidable client already had thier chance to Avoid
	} else
		return false;

	if (HasDied()) //killed by damage shield ect
		return false;

	MeleeLifeTap(damage);

	CommonBreakInvisibleFromCombat();

	//I doubt this works...
	if (!GetTarget())
		return true; //We killed them

	if(!bRiposte && !other->HasDied()) {
		TryWeaponProc(nullptr, weapon, other, Hand);	//no weapon

		if (!other->HasDied())
			TrySpellProc(nullptr, weapon, other, Hand);

		if (damage > 0 && HasSkillProcSuccess() && !other->HasDied())
			TrySkillProc(other, skillinuse, 0, true, Hand);
	}

	if(GetHP() > 0 && !other->HasDied())
		TriggerDefensiveProcs(other, Hand, true, damage);

	if (damage > 0)
		return true;

	else
		return false;
}

void NPC::Damage(Mob* other, int32 damage, uint16 spell_id, EQEmu::skills::SkillType attack_skill, bool avoidable, int8 buffslot, bool iBuffTic, int special) {
	if(spell_id==0)
		spell_id = SPELL_UNKNOWN;

	//handle EVENT_ATTACK. Resets after we have not been attacked for 12 seconds
	if(attacked_timer.Check())
	{
		Log.Out(Logs::Detail, Logs::Combat, "Triggering EVENT_ATTACK due to attack by %s", other ? other->GetName() : "nullptr");
		parse->EventNPC(EVENT_ATTACK, this, other, "", 0);
	}
	attacked_timer.Start(CombatEventTimer_expire);

	if (!IsEngaged())
		zone->AddAggroMob();

	if(GetClass() == LDON_TREASURE)
	{
		if(IsLDoNLocked() && GetLDoNLockedSkill() != LDoNTypeMechanical)
		{
			damage = -5;
		}
		else
		{
			if(IsLDoNTrapped())
			{
				Message_StringID(13, LDON_ACCIDENT_SETOFF2);
				SpellFinished(GetLDoNTrapSpellID(), other, EQEmu::CastingSlot::Item, 0, -1, spells[GetLDoNTrapSpellID()].ResistDiff, false);
				SetLDoNTrapSpellID(0);
				SetLDoNTrapped(false);
				SetLDoNTrapDetected(false);
			}
		}
	}

	//do a majority of the work...
	CommonDamage(other, damage, spell_id, attack_skill, avoidable, buffslot, iBuffTic, special);

	if(damage > 0) {
		//see if we are gunna start fleeing
		if(!IsPet()) CheckFlee();
	}
}

bool NPC::Death(Mob* killer_mob, int32 damage, uint16 spell, EQEmu::skills::SkillType attack_skill)
{
	Log.Out(Logs::Detail, Logs::Combat, "Fatal blow dealt by %s with %d damage, spell %d, skill %d",
		((killer_mob) ? (killer_mob->GetName()) : ("[nullptr]")), damage, spell, attack_skill);

	Mob *oos = nullptr;
	if (killer_mob) {
		oos = killer_mob->GetOwnerOrSelf();

		char buffer[48] = { 0 };
		snprintf(buffer, 47, "%d %d %d %d", killer_mob->GetID(), damage, spell, static_cast<int>(attack_skill));

		if (parse->EventNPC(EVENT_DEATH, this, oos, buffer, 0) != 0) {
			if (GetHP() < 0) {
				SetHP(0);
			}
			return false;
		}

		if (killer_mob->IsClient() && (spell != SPELL_UNKNOWN) && damage > 0) {
			char val1[20] = { 0 };
			entity_list.MessageClose_StringID(this, false, 100, MT_NonMelee, HIT_NON_MELEE,
				killer_mob->GetCleanName(), GetCleanName(), ConvertArray(damage, val1));
		}
	}
	else {
		char buffer[48] = { 0 };
		snprintf(buffer, 47, "%d %d %d %d", 0, damage, spell, static_cast<int>(attack_skill));

		if (parse->EventNPC(EVENT_DEATH, this, nullptr, buffer, 0) != 0) {
			if (GetHP() < 0) {
				SetHP(0);
			}
			return false;
		}
	}

	if (IsEngaged()) {
		zone->DelAggroMob();
		Log.Out(Logs::Detail, Logs::Attack, "%s Mobs currently Aggro %i", __FUNCTION__, zone->MobsAggroCount());
	}

	SetHP(0);
	SetPet(0);

	if (GetSwarmOwner()) {
		Mob* owner = entity_list.GetMobID(GetSwarmOwner());
		if (owner)
			owner->SetTempPetCount(owner->GetTempPetCount() - 1);
	}

	Mob* killer = GetHateDamageTop(this);

	entity_list.RemoveFromTargets(this, p_depop);

	if (p_depop == true)
		return false;

	HasAISpellEffects = false;
	BuffFadeAll();
	uint8 killed_level = GetLevel();

	if (GetClass() == LDON_TREASURE) { // open chest
		auto outapp = new EQApplicationPacket(OP_Animation, sizeof(Animation_Struct));
		Animation_Struct* anim = (Animation_Struct*)outapp->pBuffer;
		anim->spawnid = GetID();
		anim->action = 0x0F;
		anim->speed = 10;
		entity_list.QueueCloseClients(this, outapp);
		safe_delete(outapp);
	}

	auto app = new EQApplicationPacket(OP_Death, sizeof(Death_Struct));
	Death_Struct* d = (Death_Struct*)app->pBuffer;
	d->spawn_id = GetID();
	d->killer_id = killer_mob ? killer_mob->GetID() : 0;
	d->bindzoneid = 0;
	d->spell_id = spell == SPELL_UNKNOWN ? 0xffffffff : spell;
	d->attack_skill = SkillDamageTypes[attack_skill];
	d->damage = damage;
	app->priority = 6;
	entity_list.QueueClients(killer_mob, app, false);

	safe_delete(app);

	if (respawn2) {
		respawn2->DeathReset(1);
	}

	if (killer_mob && GetClass() != LDON_TREASURE)
		hate_list.AddEntToHateList(killer_mob, damage);

	Mob *give_exp = hate_list.GetDamageTopOnHateList(this);

	if (give_exp == nullptr)
		give_exp = killer;

	if (give_exp && give_exp->HasOwner()) {

		bool ownerInGroup = false;
		if ((give_exp->HasGroup() && give_exp->GetGroup()->IsGroupMember(give_exp->GetUltimateOwner()))
			|| (give_exp->IsPet() && (give_exp->GetOwner()->IsClient()
			|| (give_exp->GetOwner()->HasGroup() && give_exp->GetOwner()->GetGroup()->IsGroupMember(give_exp->GetOwner()->GetUltimateOwner())))))
			ownerInGroup = true;

		give_exp = give_exp->GetUltimateOwner();

#ifdef BOTS
		if (!RuleB(Bots, BotGroupXP) && !ownerInGroup) {
			give_exp = nullptr;
		}
#endif //BOTS
	}

	if (give_exp && give_exp->IsTempPet() && give_exp->IsPetOwnerClient()) {
		if (give_exp->IsNPC() && give_exp->CastToNPC()->GetSwarmOwner()) {
			Mob* temp_owner = entity_list.GetMobID(give_exp->CastToNPC()->GetSwarmOwner());
			if (temp_owner)
				give_exp = temp_owner;
		}
	}

	int PlayerCount = 0; // QueryServ Player Counting

	Client *give_exp_client = nullptr;
	if (give_exp && give_exp->IsClient())
		give_exp_client = give_exp->CastToClient();

	//do faction hits even if we are a merchant, so long as a player killed us
	if (give_exp_client && !RuleB(NPC, EnableMeritBasedFaction))
		hate_list.DoFactionHits(GetNPCFactionID());

	bool IsLdonTreasure = (this->GetClass() == LDON_TREASURE);

	if (give_exp_client && !IsCorpse()) {
		Group *kg = entity_list.GetGroupByClient(give_exp_client);
		Raid *kr = entity_list.GetRaidByClient(give_exp_client);

		int32 finalxp = EXP_FORMULA;
		finalxp = give_exp_client->mod_client_xp(finalxp, this);

		if (kr) {
			if (!IsLdonTreasure && MerchantType == 0) {
				kr->SplitExp((finalxp), this);
				if (killer_mob && (kr->IsRaidMember(killer_mob->GetName()) || kr->IsRaidMember(killer_mob->GetUltimateOwner()->GetName())))
					killer_mob->TrySpellOnKill(killed_level, spell);
			}

			/* Send the EVENT_KILLED_MERIT event for all raid members */
			for (int i = 0; i < MAX_RAID_MEMBERS; i++) {
				if (kr->members[i].member != nullptr && kr->members[i].member->IsClient()) { // If Group Member is Client
					Client *c = kr->members[i].member;
					parse->EventNPC(EVENT_KILLED_MERIT, this, c, "killed", 0);

					if (RuleB(NPC, EnableMeritBasedFaction))
						c->SetFactionLevel(c->CharacterID(), GetNPCFactionID(), c->GetBaseClass(), c->GetBaseRace(), c->GetDeity());

					mod_npc_killed_merit(kr->members[i].member);

					if (RuleB(TaskSystem, EnableTaskSystem))
						kr->members[i].member->UpdateTasksOnKill(GetNPCTypeID());
					PlayerCount++;
				}
			}

			// QueryServ Logging - Raid Kills
			if (RuleB(QueryServ, PlayerLogNPCKills)) {
				auto pack =
				    new ServerPacket(ServerOP_QSPlayerLogNPCKills,
						     sizeof(QSPlayerLogNPCKill_Struct) +
							 (sizeof(QSPlayerLogNPCKillsPlayers_Struct) * PlayerCount));
				PlayerCount = 0;
				QSPlayerLogNPCKill_Struct* QS = (QSPlayerLogNPCKill_Struct*)pack->pBuffer;
				QS->s1.NPCID = this->GetNPCTypeID();
				QS->s1.ZoneID = this->GetZoneID();
				QS->s1.Type = 2; // Raid Fight
				for (int i = 0; i < MAX_RAID_MEMBERS; i++) {
					if (kr->members[i].member != nullptr && kr->members[i].member->IsClient()) { // If Group Member is Client
						Client *c = kr->members[i].member;
						QS->Chars[PlayerCount].char_id = c->CharacterID();
						PlayerCount++;
					}
				}
				worldserver.SendPacket(pack); // Send Packet to World
				safe_delete(pack);
			}
			// End QueryServ Logging

		}
		else if (give_exp_client->IsGrouped() && kg != nullptr) {
			if (!IsLdonTreasure && MerchantType == 0) {
				kg->SplitExp((finalxp), this);
				if (killer_mob && (kg->IsGroupMember(killer_mob->GetName()) || kg->IsGroupMember(killer_mob->GetUltimateOwner()->GetName())))
					killer_mob->TrySpellOnKill(killed_level, spell);
			}

			/* Send the EVENT_KILLED_MERIT event and update kill tasks
			* for all group members */
			for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
				if (kg->members[i] != nullptr && kg->members[i]->IsClient()) { // If Group Member is Client
					Client *c = kg->members[i]->CastToClient();
					parse->EventNPC(EVENT_KILLED_MERIT, this, c, "killed", 0);

					if (RuleB(NPC, EnableMeritBasedFaction))
						c->SetFactionLevel(c->CharacterID(), GetNPCFactionID(), c->GetBaseClass(), c->GetBaseRace(), c->GetDeity());

					mod_npc_killed_merit(c);

					if (RuleB(TaskSystem, EnableTaskSystem))
						c->UpdateTasksOnKill(GetNPCTypeID());

					PlayerCount++;
				}
			}

			// QueryServ Logging - Group Kills
			if (RuleB(QueryServ, PlayerLogNPCKills)) {
				auto pack =
				    new ServerPacket(ServerOP_QSPlayerLogNPCKills,
						     sizeof(QSPlayerLogNPCKill_Struct) +
							 (sizeof(QSPlayerLogNPCKillsPlayers_Struct) * PlayerCount));
				PlayerCount = 0;
				QSPlayerLogNPCKill_Struct* QS = (QSPlayerLogNPCKill_Struct*)pack->pBuffer;
				QS->s1.NPCID = this->GetNPCTypeID();
				QS->s1.ZoneID = this->GetZoneID();
				QS->s1.Type = 1; // Group Fight
				for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
					if (kg->members[i] != nullptr && kg->members[i]->IsClient()) { // If Group Member is Client
						Client *c = kg->members[i]->CastToClient();
						QS->Chars[PlayerCount].char_id = c->CharacterID();
						PlayerCount++;
					}
				}
				worldserver.SendPacket(pack); // Send Packet to World
				safe_delete(pack);
			}
			// End QueryServ Logging
		}
		else {
			if (!IsLdonTreasure && MerchantType == 0) {
				int conlevel = give_exp->GetLevelCon(GetLevel());
				if (conlevel != CON_GREEN) {
					if (!GetOwner() || (GetOwner() && !GetOwner()->IsClient())) {
						give_exp_client->AddEXP((finalxp), conlevel);
						if (killer_mob && (killer_mob->GetID() == give_exp_client->GetID() || killer_mob->GetUltimateOwner()->GetID() == give_exp_client->GetID()))
							killer_mob->TrySpellOnKill(killed_level, spell);
					}
				}
			}

			/* Send the EVENT_KILLED_MERIT event */
			parse->EventNPC(EVENT_KILLED_MERIT, this, give_exp_client, "killed", 0);

			if (RuleB(NPC, EnableMeritBasedFaction))
				give_exp_client->SetFactionLevel(give_exp_client->CharacterID(), GetNPCFactionID(), give_exp_client->GetBaseClass(),
				give_exp_client->GetBaseRace(), give_exp_client->GetDeity());

			mod_npc_killed_merit(give_exp_client);

			if (RuleB(TaskSystem, EnableTaskSystem))
				give_exp_client->UpdateTasksOnKill(GetNPCTypeID());

			// QueryServ Logging - Solo
			if (RuleB(QueryServ, PlayerLogNPCKills)) {
				auto pack = new ServerPacket(ServerOP_QSPlayerLogNPCKills,
							     sizeof(QSPlayerLogNPCKill_Struct) +
								 (sizeof(QSPlayerLogNPCKillsPlayers_Struct) * 1));
				QSPlayerLogNPCKill_Struct* QS = (QSPlayerLogNPCKill_Struct*)pack->pBuffer;
				QS->s1.NPCID = this->GetNPCTypeID();
				QS->s1.ZoneID = this->GetZoneID();
				QS->s1.Type = 0; // Solo Fight
				Client *c = give_exp_client;
				QS->Chars[0].char_id = c->CharacterID();
				PlayerCount++;
				worldserver.SendPacket(pack); // Send Packet to World
				safe_delete(pack);
			}
			// End QueryServ Logging
		}
	}

	if (!HasOwner() && !IsMerc() && class_ != MERCHANT && class_ != ADVENTUREMERCHANT && !GetSwarmInfo()
		&& MerchantType == 0 && ((killer && (killer->IsClient() || (killer->HasOwner() && killer->GetUltimateOwner()->IsClient()) ||
		(killer->IsNPC() && killer->CastToNPC()->GetSwarmInfo() && killer->CastToNPC()->GetSwarmInfo()->GetOwner() && killer->CastToNPC()->GetSwarmInfo()->GetOwner()->IsClient())))
		|| (killer_mob && IsLdonTreasure)))
	{
		if (killer != 0) {
			if (killer->GetOwner() != 0 && killer->GetOwner()->IsClient())
				killer = killer->GetOwner();

			if (killer->IsClient() && !killer->CastToClient()->GetGM())
				this->CheckMinMaxLevel(killer);
		}

		entity_list.RemoveFromAutoXTargets(this);
		uint16 emoteid = this->GetEmoteID();
		auto corpse = new Corpse(this, &itemlist, GetNPCTypeID(), &NPCTypedata,
					 level > 54 ? RuleI(NPC, MajorNPCCorpseDecayTimeMS)
						    : RuleI(NPC, MinorNPCCorpseDecayTimeMS));
		entity_list.LimitRemoveNPC(this);
		entity_list.AddCorpse(corpse, GetID());

		entity_list.UnMarkNPC(GetID());
		entity_list.RemoveNPC(GetID());
		this->SetID(0);

		if (killer != 0 && emoteid != 0)
			corpse->CastToNPC()->DoNPCEmote(AFTERDEATH, emoteid);
		if (killer != 0 && killer->IsClient()) {
			corpse->AllowPlayerLoot(killer, 0);
			if (killer->IsGrouped()) {
				Group* group = entity_list.GetGroupByClient(killer->CastToClient());
				if (group != 0) {
					for (int i = 0; i<6; i++) { // Doesnt work right, needs work
						if (group->members[i] != nullptr) {
							corpse->AllowPlayerLoot(group->members[i], i);
						}
					}
				}
			}
			else if (killer->IsRaidGrouped()) {
				Raid* r = entity_list.GetRaidByClient(killer->CastToClient());
				if (r) {
					int i = 0;
					for (int x = 0; x < MAX_RAID_MEMBERS; x++) {
						switch (r->GetLootType()) {
						case 0:
						case 1:
							if (r->members[x].member && r->members[x].IsRaidLeader) {
								corpse->AllowPlayerLoot(r->members[x].member, i);
								i++;
							}
							break;
						case 2:
							if (r->members[x].member && r->members[x].IsRaidLeader) {
								corpse->AllowPlayerLoot(r->members[x].member, i);
								i++;
							}
							else if (r->members[x].member && r->members[x].IsGroupLeader) {
								corpse->AllowPlayerLoot(r->members[x].member, i);
								i++;
							}
							break;
						case 3:
							if (r->members[x].member && r->members[x].IsLooter) {
								corpse->AllowPlayerLoot(r->members[x].member, i);
								i++;
							}
							break;
						case 4:
							if (r->members[x].member) {
								corpse->AllowPlayerLoot(r->members[x].member, i);
								i++;
							}
							break;
						}
					}
				}
			}
		}
		else if (killer_mob && IsLdonTreasure) {
			auto u_owner = killer_mob->GetUltimateOwner();
			if (u_owner->IsClient())
				corpse->AllowPlayerLoot(u_owner, 0);
		}

		if (zone && zone->adv_data) {
			ServerZoneAdventureDataReply_Struct *sr = (ServerZoneAdventureDataReply_Struct*)zone->adv_data;
			if (sr->type == Adventure_Kill) {
				zone->DoAdventureCountIncrease();
			}
			else if (sr->type == Adventure_Assassinate) {
				if (sr->data_id == GetNPCTypeID()) {
					zone->DoAdventureCountIncrease();
				}
				else {
					zone->DoAdventureAssassinationCountIncrease();
				}
			}
		}
	}
	else {
		entity_list.RemoveFromXTargets(this);
	}

	// Parse quests even if we're killed by an NPC
	if (oos) {
		mod_npc_killed(oos);

		uint16 emoteid = this->GetEmoteID();
		if (emoteid != 0)
			this->DoNPCEmote(ONDEATH, emoteid);
		if (oos->IsNPC()) {
			parse->EventNPC(EVENT_NPC_SLAY, oos->CastToNPC(), this, "", 0);
			uint16 emoteid = oos->GetEmoteID();
			if (emoteid != 0)
				oos->CastToNPC()->DoNPCEmote(KILLEDNPC, emoteid);
			killer_mob->TrySpellOnKill(killed_level, spell);
		}
	}

	WipeHateList();
	p_depop = true;

	if (killer_mob && killer_mob->GetTarget() == this) //we can kill things without having them targeted
		killer_mob->SetTarget(nullptr); //via AE effects and such..

	entity_list.UpdateFindableNPCState(this, true);

	char buffer[48] = { 0 };
	snprintf(buffer, 47, "%d %d %d %d", killer_mob ? killer_mob->GetID() : 0, damage, spell, static_cast<int>(attack_skill));
	parse->EventNPC(EVENT_DEATH_COMPLETE, this, oos, buffer, 0);

	/* Zone controller process EVENT_DEATH_ZONE (Death events) */
	if (RuleB(Zone, UseZoneController)) {
		if (entity_list.GetNPCByNPCTypeID(ZONE_CONTROLLER_NPC_ID) && this->GetNPCTypeID() != ZONE_CONTROLLER_NPC_ID) {
			char data_pass[100] = { 0 };
			snprintf(data_pass, 99, "%d %d %d %d %d", killer_mob ? killer_mob->GetID() : 0, damage, spell, static_cast<int>(attack_skill), this->GetNPCTypeID());
			parse->EventNPC(EVENT_DEATH_ZONE, entity_list.GetNPCByNPCTypeID(ZONE_CONTROLLER_NPC_ID)->CastToNPC(), nullptr, data_pass, 0);
		}
	}

	return true;
}

void Mob::AddToHateList(Mob* other, uint32 hate /*= 0*/, int32 damage /*= 0*/, bool iYellForHelp /*= true*/, bool bFrenzy /*= false*/, bool iBuffTic /*= false*/, uint16 spell_id)
{
	if(!other)
		return;

	if (other == this)
		return;

	if(damage < 0){
		hate = 1;
	}

	if (iYellForHelp)
		SetPrimaryAggro(true);
	else
		SetAssistAggro(true);

	bool wasengaged = IsEngaged();
	Mob* owner = other->GetOwner();
	Mob* mypet = this->GetPet();
	Mob* myowner = this->GetOwner();
	Mob* targetmob = this->GetTarget();

	if(other){
		bool on_hatelist = CheckAggro(other);
		AddRampage(other);
		if (on_hatelist) { // odd reason, if you're not on the hate list, subtlety etc don't apply!
			// Spell Casting Subtlety etc
			int hatemod = 100 + other->spellbonuses.hatemod + other->itembonuses.hatemod + other->aabonuses.hatemod;

			if(hatemod < 1)
				hatemod = 1;
			hate = ((hate * (hatemod))/100);
		} else {
			hate += 100; // 100 bonus initial aggro
		}
	}

	if(IsPet() && GetOwner() && GetOwner()->GetAA(aaPetDiscipline) && IsHeld() && !IsFocused()) { //ignore aggro if hold and !focus
		return;
	}

	if(IsPet() && GetOwner() && GetOwner()->GetAA(aaPetDiscipline) && IsHeld() && GetOwner()->GetAA(aaAdvancedPetDiscipline) >= 1 && IsFocused()) {
		if (!targetmob)
			return;
	}

	if (other->IsNPC() && (other->IsPet() || other->CastToNPC()->GetSwarmOwner() > 0))
		TryTriggerOnValueAmount(false, false, false, true);

	if(IsClient() && !IsAIControlled())
		return;

	if(IsFamiliar() || GetSpecialAbility(IMMUNE_AGGRO))
		return;

	if (spell_id != SPELL_UNKNOWN && NoDetrimentalSpellAggro(spell_id))
		return;

	if (other == myowner)
		return;

	if(other->GetSpecialAbility(IMMUNE_AGGRO_ON))
		return;

	if(GetSpecialAbility(NPC_TUNNELVISION)) {
		int tv_mod = GetSpecialAbilityParam(NPC_TUNNELVISION, 0);

		Mob *top = GetTarget();
		if(top && top != other) {
			if(tv_mod) {
				float tv = tv_mod / 100.0f;
				hate *= tv;
			} else {
				hate *= RuleR(Aggro, TunnelVisionAggroMod);
			}
		}
	}

	if(IsNPC() && CastToNPC()->IsUnderwaterOnly() && zone->HasWaterMap()) {
		if(!zone->watermap->InLiquid(glm::vec3(other->GetPosition()))) {
			return;
		}
	}
	// first add self

	// The damage on the hate list is used to award XP to the killer. This check is to prevent Killstealing.
	// e.g. Mob has 5000 hit points, Player A melees it down to 500 hp, Player B executes a headshot (10000 damage).
	// If we add 10000 damage, Player B would get the kill credit, so we only award damage credit to player B of the
	// amount of HP the mob had left.
	//
	if(damage > GetHP())
		damage = GetHP();

	if (spellbonuses.ImprovedTaunt[1] && (GetLevel() < spellbonuses.ImprovedTaunt[0])
		&& other &&  (buffs[spellbonuses.ImprovedTaunt[2]].casterid != other->GetID()))
		hate = (hate*spellbonuses.ImprovedTaunt[1])/100;

	hate_list.AddEntToHateList(other, hate, damage, bFrenzy, !iBuffTic);

	if(other->IsClient())
		other->CastToClient()->AddAutoXTarget(this);

#ifdef BOTS
	// if other is a bot, add the bots client to the hate list
	if(other->IsBot()) {
		if(other->CastToBot()->GetBotOwner() && other->CastToBot()->GetBotOwner()->CastToClient()->GetFeigned()) {
			AddFeignMemory(other->CastToBot()->GetBotOwner()->CastToClient());
		}
		else {
			if(!hate_list.IsEntOnHateList(other->CastToBot()->GetBotOwner()))
				hate_list.AddEntToHateList(other->CastToBot()->GetBotOwner(), 0, 0, false, true);
		}
	}
#endif //BOTS

	// if other is a merc, add the merc client to the hate list
	if(other->IsMerc()) {
		if(other->CastToMerc()->GetMercOwner() && other->CastToMerc()->GetMercOwner()->CastToClient()->GetFeigned()) {
			AddFeignMemory(other->CastToMerc()->GetMercOwner()->CastToClient());
		}
		else {
			if(!hate_list.IsEntOnHateList(other->CastToMerc()->GetMercOwner()))
				hate_list.AddEntToHateList(other->CastToMerc()->GetMercOwner(), 0, 0, false, true);
		}
	} //MERC

	// then add pet owner if there's one
	if (owner) { // Other is a pet, add him and it
		// EverHood 6/12/06
		// Can't add a feigned owner to hate list
		if(owner->IsClient() && owner->CastToClient()->GetFeigned()) {
			//they avoid hate due to feign death...
		} else {
			// cb:2007-08-17
			// owner must get on list, but he's not actually gained any hate yet
			if(!owner->GetSpecialAbility(IMMUNE_AGGRO))
			{
				hate_list.AddEntToHateList(owner, 0, 0, false, !iBuffTic);
				if(owner->IsClient())
					owner->CastToClient()->AddAutoXTarget(this);
			}
		}
	}

	if (mypet && (!(GetAA(aaPetDiscipline) && mypet->IsHeld()))) { // I have a pet, add other to it
		if(!mypet->IsFamiliar() && !mypet->GetSpecialAbility(IMMUNE_AGGRO))
			mypet->hate_list.AddEntToHateList(other, 0, 0, bFrenzy);
	} else if (myowner) { // I am a pet, add other to owner if it's NPC/LD
		if (myowner->IsAIControlled() && !myowner->GetSpecialAbility(IMMUNE_AGGRO))
			myowner->hate_list.AddEntToHateList(other, 0, 0, bFrenzy);
	}

	if (other->GetTempPetCount())
		entity_list.AddTempPetsToHateList(other, this, bFrenzy);

	if (!wasengaged) {
		if(IsNPC() && other->IsClient() && other->CastToClient())
			parse->EventNPC(EVENT_AGGRO, this->CastToNPC(), other, "", 0);
		AI_Event_Engaged(other, iYellForHelp);
	}
}

// this is called from Damage() when 'this' is attacked by 'other.
// 'this' is the one being attacked
// 'other' is the attacker
// a damage shield causes damage (or healing) to whoever attacks the wearer
// a reverse ds causes damage to the wearer whenever it attack someone
// given this, a reverse ds must be checked each time the wearer is attacking
// and not when they're attacked
//a damage shield on a spell is a negative value but on an item it's a positive value so add the spell value and subtract the item value to get the end ds value
void Mob::DamageShield(Mob* attacker, bool spell_ds) {

	if(!attacker || this == attacker)
		return;

	int DS = 0;
	int rev_ds = 0;
	uint16 spellid = 0;

	if(!spell_ds)
	{
		DS = spellbonuses.DamageShield;
		rev_ds = attacker->spellbonuses.ReverseDamageShield;

		if(spellbonuses.DamageShieldSpellID != 0 && spellbonuses.DamageShieldSpellID != SPELL_UNKNOWN)
			spellid = spellbonuses.DamageShieldSpellID;
	}
	else {
		DS = spellbonuses.SpellDamageShield;
		rev_ds = 0;
		// This ID returns "you are burned", seemed most appropriate for spell DS
		spellid = 2166;
	}

	if(DS == 0 && rev_ds == 0)
		return;

	Log.Out(Logs::Detail, Logs::Combat, "Applying Damage Shield of value %d to %s", DS, attacker->GetName());

	//invert DS... spells yield negative values for a true damage shield
	if(DS < 0) {
		if(!spell_ds)	{

			DS += aabonuses.DamageShield; //Live AA - coat of thistles. (negative value)
			DS -= itembonuses.DamageShield; //+Damage Shield should only work when you already have a DS spell

			//Spell data for damage shield mitigation shows a negative value for spells for clients and positive
			//value for spells that effect pets. Unclear as to why. For now will convert all positive to be consistent.
			if (attacker->IsOffHandAtk()){
				int32 mitigation = attacker->itembonuses.DSMitigationOffHand +
									attacker->spellbonuses.DSMitigationOffHand +
									attacker->aabonuses.DSMitigationOffHand;
				DS -= DS*mitigation/100;
			}
			DS -= DS * attacker->itembonuses.DSMitigation / 100;
		}
		attacker->Damage(this, -DS, spellid, EQEmu::skills::SkillAbjuration/*hackish*/, false);
		//we can assume there is a spell now
		auto outapp = new EQApplicationPacket(OP_Damage, sizeof(CombatDamage_Struct));
		CombatDamage_Struct* cds = (CombatDamage_Struct*)outapp->pBuffer;
		cds->target = attacker->GetID();
		cds->source = GetID();
		cds->type = spellbonuses.DamageShieldType;
		cds->spellid = 0x0;
		cds->damage = DS;
		entity_list.QueueCloseClients(this, outapp);
		safe_delete(outapp);
	} else if (DS > 0 && !spell_ds) {
		//we are healing the attacker...
		attacker->HealDamage(DS);
		//TODO: send a packet???
	}

	//Reverse DS
	//this is basically a DS, but the spell is on the attacker, not the attackee
	//if we've gotten to this point, we know we know "attacker" hit "this" (us) for damage & we aren't invulnerable
	uint16 rev_ds_spell_id = SPELL_UNKNOWN;

	if(spellbonuses.ReverseDamageShieldSpellID != 0 && spellbonuses.ReverseDamageShieldSpellID != SPELL_UNKNOWN)
		rev_ds_spell_id = spellbonuses.ReverseDamageShieldSpellID;

	if(rev_ds < 0) {
		Log.Out(Logs::Detail, Logs::Combat, "Applying Reverse Damage Shield of value %d to %s", rev_ds, attacker->GetName());
		attacker->Damage(this, -rev_ds, rev_ds_spell_id, EQEmu::skills::SkillAbjuration/*hackish*/, false); //"this" (us) will get the hate, etc. not sure how this works on Live, but it'll works for now, and tanks will love us for this
		//do we need to send a damage packet here also?
	}
}

uint8 Mob::GetWeaponDamageBonus(const EQEmu::ItemData *weapon, bool offhand)
{
	// dev quote with old and new formulas
	// https://forums.daybreakgames.com/eq/index.php?threads/test-update-09-17-15.226618/page-5#post-3326194
	//
	// We assume that the level check is done before calling this function and sinister strikes is checked before
	// calling for offhand DB
	auto level = GetLevel();
	if (!weapon)
		return 1 + ((level - 28) / 3); // how does weaponless scale?

	auto delay = weapon->Delay;
	if (weapon->IsType1HWeapon() || weapon->ItemType == EQEmu::item::ItemTypeMartial) {
		// we assume sinister strikes is checked before calling here
		if (!offhand) {
			if (delay <= 39)
				return 1 + ((level - 28) / 3);
			else if (delay < 43)
				return 2 + ((level - 28) / 3) + ((delay - 40) / 3);
			else if (delay < 45)
				return 3 + ((level - 28) / 3) + ((delay - 40) / 3);
			else if (delay >= 45)
				return 4 + ((level - 28) / 3) + ((delay - 40) / 3);
		} else {
			return 1 + ((level - 40) / 3) * (delay / 30); // YOOO shit's useless waste of AAs
		}
	} else {
		// 2h damage bonus
		int damage_bonus = 1 + (level - 28) / 3;
		if (delay <= 27)
			return damage_bonus + 1;
		// Client isn't reflecting what the dev quoted, this matches better
		if (level > 29) {
			int level_bonus = (level - 30) / 5 + 1;
			if (level > 50) {
				level_bonus++;
				int level_bonus2 = level - 50;
				if (level > 67)
					level_bonus2 += 5;
				else if (level > 59)
					level_bonus2 += 4;
				else if (level > 58)
					level_bonus2 += 3;
				else if (level > 56)
					level_bonus2 += 2;
				else if (level > 54)
					level_bonus2++;
				level_bonus += level_bonus2 * delay / 40;
			}
			damage_bonus += level_bonus;
		}
		if (delay >= 40) {
			int delay_bonus = (delay - 40) / 3 + 1;
			if (delay >= 45)
				delay_bonus += 2;
			else if (delay >= 43)
				delay_bonus++;
			damage_bonus += delay_bonus;
		}
		return damage_bonus;
	}
}

int Mob::GetHandToHandDamage(void)
{
	if (RuleB(Combat, UseRevampHandToHand)) {
		// everyone uses this in the revamp!
		int skill = GetSkill(EQEmu::skills::SkillHandtoHand);
		int epic = 0;
		if (IsClient() && CastToClient()->GetItemIDAt(12) == 10652 && GetLevel() > 46)
			epic = 280;
		if (epic > skill)
			skill = epic;
		return skill / 15 + 3;
	}

	static uint8 mnk_dmg[] = {99,
				4, 4, 4, 4, 5, 5, 5, 5, 5, 6,           // 1-10
				6, 6, 6, 6, 7, 7, 7, 7, 7, 8,           // 11-20
				8, 8, 8, 8, 9, 9, 9, 9, 9, 10,          // 21-30
				10, 10, 10, 10, 11, 11, 11, 11, 11, 12, // 31-40
				12, 12, 12, 12, 13, 13, 13, 13, 13, 14, // 41-50
				14, 14, 14, 14, 14, 14, 14, 14, 14, 14, // 51-60
				14, 14};                                // 61-62
	static uint8 bst_dmg[] = {99,
				4, 4, 4, 4, 4, 5, 5, 5, 5, 5,        // 1-10
				5, 6, 6, 6, 6, 6, 6, 7, 7, 7,        // 11-20
				7, 7, 7, 8, 8, 8, 8, 8, 8, 9,        // 21-30
				9, 9, 9, 9, 9, 10, 10, 10, 10, 10,   // 31-40
				10, 11, 11, 11, 11, 11, 11, 12, 12}; // 41-49
	if (GetClass() == MONK) {
		if (IsClient() && CastToClient()->GetItemIDAt(12) == 10652 && GetLevel() > 50)
			return 9;
		if (level > 62)
			return 15;
		return mnk_dmg[level];
	} else if (GetClass() == BEASTLORD) {
		if (level > 49)
			return 13;
		return bst_dmg[level];
	}
	return 2;
}

int Mob::GetHandToHandDelay(void)
{
	if (RuleB(Combat, UseRevampHandToHand)) {
		// everyone uses this in the revamp!
		int skill = GetSkill(EQEmu::skills::SkillHandtoHand);
		int epic = 0;
		int iksar = 0;
		if (IsClient() && CastToClient()->GetItemIDAt(12) == 10652 && GetLevel() > 46)
			epic = 280;
		else if (GetRace() == IKSAR)
			iksar = 1;
		if (epic > skill)
			skill = epic;
		return iksar - skill / 21 + 38;
	}

	int delay = 35;
	static uint8 mnk_hum_delay[] = {99,
				35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 1-10
				35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 11-20
				35, 35, 35, 35, 35, 35, 35, 34, 34, 34, // 21-30
				34, 33, 33, 33, 33, 32, 32, 32, 32, 31, // 31-40
				31, 31, 31, 30, 30, 30, 30, 29, 29, 29, // 41-50
				29, 28, 28, 28, 28, 27, 27, 27, 27, 26, // 51-60
				24, 22};                                // 61-62
	static uint8 mnk_iks_delay[] = {99,
				35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 1-10
				35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 11-20
				35, 35, 35, 35, 35, 35, 35, 35, 35, 34, // 21-30
				34, 34, 34, 34, 34, 33, 33, 33, 33, 33, // 31-40
				33, 32, 32, 32, 32, 32, 32, 31, 31, 31, // 41-50
				31, 31, 31, 30, 30, 30, 30, 30, 30, 29, // 51-60
				25, 23};                                // 61-62
	static uint8 bst_delay[] = {99,
				35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 1-10
				35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 11-20
				35, 35, 35, 35, 35, 35, 35, 35, 34, 34, // 21-30
				34, 34, 34, 33, 33, 33, 33, 33, 32, 32, // 31-40
				32, 32, 32, 31, 31, 31, 31, 31, 30, 30, // 41-50
				30, 30, 30, 29, 29, 29, 29, 29, 28, 28, // 51-60
				28, 28, 28, 27, 27, 27, 27, 27, 26, 26, // 61-70
				26, 26, 26};                            // 71-73

	if (GetClass() == MONK) {
		// Have a look to see if we have epic fists on
		if (IsClient() && CastToClient()->GetItemIDAt(12) == 10652 && GetLevel() > 50)
			return 16;
		int level = GetLevel();
		if (level > 62)
			return GetRace() == IKSAR ? 21 : 20;
		return GetRace() == IKSAR ? mnk_iks_delay[level] : mnk_hum_delay[level];
	} else if (GetClass() == BEASTLORD) {
		int level = GetLevel();
		if (level > 73)
			return 25;
		return bst_delay[level];
	}
	return 35;
}

int32 Mob::ReduceDamage(int32 damage)
{
	if(damage <= 0)
		return damage;

	int32 slot = -1;
	bool DisableMeleeRune = false;

	if (spellbonuses.NegateAttacks[0]){
		slot = spellbonuses.NegateAttacks[1];
		if(slot >= 0) {
			if(--buffs[slot].numhits == 0) {

				if(!TryFadeEffect(slot))
					BuffFadeBySlot(slot , true);
			}

			if (spellbonuses.NegateAttacks[2] && (damage > spellbonuses.NegateAttacks[2]))
				damage -= spellbonuses.NegateAttacks[2];
			else
				return -6;
		}
	}

	//Only mitigate if damage is above the minimium specified.
	if (spellbonuses.MeleeThresholdGuard[0]){
		slot = spellbonuses.MeleeThresholdGuard[1];

		if (slot >= 0 && (damage > spellbonuses.MeleeThresholdGuard[2]))
		{
			DisableMeleeRune = true;
			int damage_to_reduce = damage * spellbonuses.MeleeThresholdGuard[0] / 100;
			if(damage_to_reduce >= buffs[slot].melee_rune)
			{
				Log.Out(Logs::Detail, Logs::Spells, "Mob::ReduceDamage SE_MeleeThresholdGuard %d damage negated, %d"
					" damage remaining, fading buff.", damage_to_reduce, buffs[slot].melee_rune);
				damage -= buffs[slot].melee_rune;
				if(!TryFadeEffect(slot))
					BuffFadeBySlot(slot);
			}
			else
			{
				Log.Out(Logs::Detail, Logs::Spells, "Mob::ReduceDamage SE_MeleeThresholdGuard %d damage negated, %d"
					" damage remaining.", damage_to_reduce, buffs[slot].melee_rune);
				buffs[slot].melee_rune = (buffs[slot].melee_rune - damage_to_reduce);
				damage -= damage_to_reduce;
			}
		}
	}

	if (spellbonuses.MitigateMeleeRune[0] && !DisableMeleeRune){
		slot = spellbonuses.MitigateMeleeRune[1];
		if(slot >= 0)
		{
			int damage_to_reduce = damage * spellbonuses.MitigateMeleeRune[0] / 100;

			if (spellbonuses.MitigateMeleeRune[2] && (damage_to_reduce > spellbonuses.MitigateMeleeRune[2]))
					damage_to_reduce = spellbonuses.MitigateMeleeRune[2];

			if(spellbonuses.MitigateMeleeRune[3] && (damage_to_reduce >= buffs[slot].melee_rune))
			{
				Log.Out(Logs::Detail, Logs::Spells, "Mob::ReduceDamage SE_MitigateMeleeDamage %d damage negated, %d"
					" damage remaining, fading buff.", damage_to_reduce, buffs[slot].melee_rune);
				damage -= buffs[slot].melee_rune;
				if(!TryFadeEffect(slot))
					BuffFadeBySlot(slot);
			}
			else
			{
				Log.Out(Logs::Detail, Logs::Spells, "Mob::ReduceDamage SE_MitigateMeleeDamage %d damage negated, %d"
					" damage remaining.", damage_to_reduce, buffs[slot].melee_rune);

				if (spellbonuses.MitigateMeleeRune[3])
					buffs[slot].melee_rune = (buffs[slot].melee_rune - damage_to_reduce);

				damage -= damage_to_reduce;
			}
		}
	}

	if(damage < 1)
		return -6;

	if (spellbonuses.MeleeRune[0] && spellbonuses.MeleeRune[1] >= 0)
		damage = RuneAbsorb(damage, SE_Rune);

	if(damage < 1)
		return -6;

	return(damage);
}

int32 Mob::AffectMagicalDamage(int32 damage, uint16 spell_id, const bool iBuffTic, Mob* attacker)
{
	if(damage <= 0)
		return damage;

	bool DisableSpellRune = false;
	int32 slot = -1;

	// See if we block the spell outright first
	if (!iBuffTic && spellbonuses.NegateAttacks[0]){
		slot = spellbonuses.NegateAttacks[1];
		if(slot >= 0) {
			if(--buffs[slot].numhits == 0) {

				if(!TryFadeEffect(slot))
					BuffFadeBySlot(slot , true);
			}

			if (spellbonuses.NegateAttacks[2] && (damage > spellbonuses.NegateAttacks[2]))
				damage -= spellbonuses.NegateAttacks[2];
			else
				return 0;
		}
	}

	// If this is a DoT, use DoT Shielding...
	if(iBuffTic) {
		damage -= (damage * itembonuses.DoTShielding / 100);

		if (spellbonuses.MitigateDotRune[0]){
			slot = spellbonuses.MitigateDotRune[1];
			if(slot >= 0)
			{
				int damage_to_reduce = damage * spellbonuses.MitigateDotRune[0] / 100;

				if (spellbonuses.MitigateDotRune[2] && (damage_to_reduce > spellbonuses.MitigateDotRune[2]))
					damage_to_reduce = spellbonuses.MitigateDotRune[2];

				if(spellbonuses.MitigateDotRune[3] && (damage_to_reduce >= buffs[slot].dot_rune))
				{
					damage -= buffs[slot].dot_rune;
					if(!TryFadeEffect(slot))
						BuffFadeBySlot(slot);
				}
				else
				{
					if (spellbonuses.MitigateDotRune[3])
						buffs[slot].dot_rune = (buffs[slot].dot_rune - damage_to_reduce);

					damage -= damage_to_reduce;
				}
			}
		}
	}

	// This must be a DD then so lets apply Spell Shielding and runes.
	else
	{
		// Reduce damage by the Spell Shielding first so that the runes don't take the raw damage.
		damage -= (damage * itembonuses.SpellShield / 100);

		//Only mitigate if damage is above the minimium specified.
		if (spellbonuses.SpellThresholdGuard[0]){
			slot = spellbonuses.SpellThresholdGuard[1];

			if (slot >= 0 && (damage > spellbonuses.MeleeThresholdGuard[2]))
			{
				DisableSpellRune = true;
				int damage_to_reduce = damage * spellbonuses.SpellThresholdGuard[0] / 100;
				if(damage_to_reduce >= buffs[slot].magic_rune)
				{
					damage -= buffs[slot].magic_rune;
					if(!TryFadeEffect(slot))
						BuffFadeBySlot(slot);
				}
				else
				{
					buffs[slot].melee_rune = (buffs[slot].magic_rune - damage_to_reduce);
					damage -= damage_to_reduce;
				}
			}
		}

		// Do runes now.
		if (spellbonuses.MitigateSpellRune[0] && !DisableSpellRune){
			slot = spellbonuses.MitigateSpellRune[1];
			if(slot >= 0)
			{
				int damage_to_reduce = damage * spellbonuses.MitigateSpellRune[0] / 100;

				if (spellbonuses.MitigateSpellRune[2] && (damage_to_reduce > spellbonuses.MitigateSpellRune[2]))
					damage_to_reduce = spellbonuses.MitigateSpellRune[2];

				if(spellbonuses.MitigateSpellRune[3] && (damage_to_reduce >= buffs[slot].magic_rune))
				{
					Log.Out(Logs::Detail, Logs::Spells, "Mob::ReduceDamage SE_MitigateSpellDamage %d damage negated, %d"
						" damage remaining, fading buff.", damage_to_reduce, buffs[slot].magic_rune);
					damage -= buffs[slot].magic_rune;
					if(!TryFadeEffect(slot))
						BuffFadeBySlot(slot);
				}
				else
				{
					Log.Out(Logs::Detail, Logs::Spells, "Mob::ReduceDamage SE_MitigateMeleeDamage %d damage negated, %d"
						" damage remaining.", damage_to_reduce, buffs[slot].magic_rune);

					if (spellbonuses.MitigateSpellRune[3])
						buffs[slot].magic_rune = (buffs[slot].magic_rune - damage_to_reduce);

					damage -= damage_to_reduce;
				}
			}
		}

		if(damage < 1)
			return 0;

		//Regular runes absorb spell damage (except dots) - Confirmed on live.
		if (spellbonuses.MeleeRune[0] && spellbonuses.MeleeRune[1] >= 0)
			damage = RuneAbsorb(damage, SE_Rune);

		if (spellbonuses.AbsorbMagicAtt[0] && spellbonuses.AbsorbMagicAtt[1] >= 0)
			damage = RuneAbsorb(damage, SE_AbsorbMagicAtt);

		if(damage < 1)
			return 0;
	}
	return damage;
}

int32 Mob::ReduceAllDamage(int32 damage)
{
	if(damage <= 0)
		return damage;

	if(spellbonuses.ManaAbsorbPercentDamage[0]) {
		int32 mana_reduced =  damage * spellbonuses.ManaAbsorbPercentDamage[0] / 100;
		if (GetMana() >= mana_reduced){
			damage -= mana_reduced;
			SetMana(GetMana() - mana_reduced);
			TryTriggerOnValueAmount(false, true);
		}
	}

	CheckNumHitsRemaining(NumHit::IncomingDamage);

	return(damage);
}

bool Mob::HasProcs() const
{
	for (int i = 0; i < MAX_PROCS; i++)
		if (PermaProcs[i].spellID != SPELL_UNKNOWN || SpellProcs[i].spellID != SPELL_UNKNOWN)
			return true;
	return false;
}

bool Mob::HasDefensiveProcs() const
{
	for (int i = 0; i < MAX_PROCS; i++)
		if (DefensiveProcs[i].spellID != SPELL_UNKNOWN)
			return true;
	return false;
}

bool Mob::HasSkillProcs() const
{

	for(int i = 0; i < MAX_SKILL_PROCS; i++){
		if (spellbonuses.SkillProc[i] || itembonuses.SkillProc[i] || aabonuses.SkillProc[i])
			return true;
	}
	return false;
}

bool Mob::HasSkillProcSuccess() const
{
	for(int i = 0; i < MAX_SKILL_PROCS; i++){
		if (spellbonuses.SkillProcSuccess[i] || itembonuses.SkillProcSuccess[i] || aabonuses.SkillProcSuccess[i])
			return true;
	}
	return false;
}

bool Mob::HasRangedProcs() const
{
	for (int i = 0; i < MAX_PROCS; i++)
		if (RangedProcs[i].spellID != SPELL_UNKNOWN)
			return true;
	return false;
}

bool Client::CheckDoubleAttack()
{
	int chance = 0;
	int skill = GetSkill(EQEmu::skills::SkillDoubleAttack);
	//Check for bonuses that give you a double attack chance regardless of skill (ie Bestial Frenzy/Harmonious Attack AA)
	int bonusGiveDA = aabonuses.GiveDoubleAttack + spellbonuses.GiveDoubleAttack + itembonuses.GiveDoubleAttack;
	if (skill > 0)
		chance = skill + GetLevel();
	else if (!bonusGiveDA)
		return false;

	if (bonusGiveDA)
		chance += bonusGiveDA / 100.0f * 500; // convert to skill value
	int per_inc = aabonuses.DoubleAttackChance + spellbonuses.DoubleAttackChance + itembonuses.DoubleAttackChance;
	if (per_inc)
		chance += chance * per_inc / 100;

	return zone->random.Int(1, 500) <= chance;
}

// Admittedly these parses were short, but this check worked for 3 toons across multiple levels
// with varying triple attack skill (1-3% error at least)
bool Client::CheckTripleAttack()
{
	int chance = GetSkill(EQEmu::skills::SkillTripleAttack);
	if (chance < 1)
		return false;

	int per_inc = aabonuses.TripleAttackChance + spellbonuses.TripleAttackChance + itembonuses.TripleAttackChance;
	if (per_inc)
		chance += chance * per_inc / 100;

	return zone->random.Int(1, 1000) <= chance;
}

bool Client::CheckDoubleRangedAttack() {
	int32 chance = spellbonuses.DoubleRangedAttack + itembonuses.DoubleRangedAttack + aabonuses.DoubleRangedAttack;

	if(chance && zone->random.Roll(chance))
		return true;

	return false;
}

bool Mob::CheckDoubleAttack()
{
	// Not 100% certain pets follow this or if it's just from pets not always
	// having the same skills as most mobs
	int chance = GetSkill(EQEmu::skills::SkillDoubleAttack);
	if (GetLevel() > 35)
		chance += GetLevel();

	int per_inc = aabonuses.DoubleAttackChance + spellbonuses.DoubleAttackChance + itembonuses.DoubleAttackChance;
	if (per_inc)
		chance += chance * per_inc / 100;

	return zone->random.Int(1, 500) <= chance;
}

void Mob::CommonDamage(Mob* attacker, int32 &damage, const uint16 spell_id, const EQEmu::skills::SkillType skill_used, bool &avoidable, const int8 buffslot, const bool iBuffTic, int special) {
	// This method is called with skill_used=ABJURE for Damage Shield damage.
	bool FromDamageShield = (skill_used == EQEmu::skills::SkillAbjuration);
	bool ignore_invul = false;
	if (IsValidSpell(spell_id))
		ignore_invul = spell_id == 982 || spells[spell_id].cast_not_standing; // cazic touch

	Log.Out(Logs::Detail, Logs::Combat, "Applying damage %d done by %s with skill %d and spell %d, avoidable? %s, is %sa buff tic in slot %d",
		damage, attacker?attacker->GetName():"NOBODY", skill_used, spell_id, avoidable?"yes":"no", iBuffTic?"":"not ", buffslot);

	if (!ignore_invul && (GetInvul() || DivineAura())) {
		Log.Out(Logs::Detail, Logs::Combat, "Avoiding %d damage due to invulnerability.", damage);
		damage = -5;
	}

	if( spell_id != SPELL_UNKNOWN || attacker == nullptr )
		avoidable = false;

	// only apply DS if physical damage (no spell damage)
	// damage shield calls this function with spell_id set, so its unavoidable
	if (attacker && damage > 0 && spell_id == SPELL_UNKNOWN && skill_used != EQEmu::skills::SkillArchery && skill_used != EQEmu::skills::SkillThrowing) {
		DamageShield(attacker);
	}

	if (spell_id == SPELL_UNKNOWN && skill_used) {
		CheckNumHitsRemaining(NumHit::IncomingHitAttempts);

		if (attacker)
			attacker->CheckNumHitsRemaining(NumHit::OutgoingHitAttempts);
	}

	if(attacker){
		if(attacker->IsClient()){
			if(!RuleB(Combat, EXPFromDmgShield)) {
			// Damage shield damage shouldn't count towards who gets EXP
				if(!attacker->CastToClient()->GetFeigned() && !FromDamageShield)
					AddToHateList(attacker, 0, damage, true, false, iBuffTic, spell_id);
			}
			else {
				if(!attacker->CastToClient()->GetFeigned())
					AddToHateList(attacker, 0, damage, true, false, iBuffTic, spell_id);
			}
		}
		else
			AddToHateList(attacker, 0, damage, true, false, iBuffTic, spell_id);
	}

	if(damage > 0) {
		//if there is some damage being done and theres an attacker involved
		if(attacker) {
			// if spell is lifetap add hp to the caster
			if (spell_id != SPELL_UNKNOWN && IsLifetapSpell( spell_id )) {
				int healed = damage;

				healed = attacker->GetActSpellHealing(spell_id, healed);
				Log.Out(Logs::Detail, Logs::Combat, "Applying lifetap heal of %d to %s", healed, attacker->GetName());
				attacker->HealDamage(healed);

				//we used to do a message to the client, but its gone now.
				// emote goes with every one ... even npcs
				entity_list.MessageClose(this, true, 300, MT_Emote, "%s beams a smile at %s", attacker->GetCleanName(), this->GetCleanName() );
			}
		}	//end `if there is some damage being done and theres anattacker person involved`

		Mob *pet = GetPet();
		if (pet && !pet->IsFamiliar() && !pet->GetSpecialAbility(IMMUNE_AGGRO) && !pet->IsEngaged() && attacker && attacker != this && !attacker->IsCorpse())
		{
			if (!pet->IsHeld()) {
				Log.Out(Logs::Detail, Logs::Aggro, "Sending pet %s into battle due to attack.", pet->GetName());
				pet->AddToHateList(attacker, 1,0, true, false, false, spell_id);
				pet->SetTarget(attacker);
				Message_StringID(10, PET_ATTACKING, pet->GetCleanName(), attacker->GetCleanName());
			}
		}

		//see if any runes want to reduce this damage
		if(spell_id == SPELL_UNKNOWN) {
			damage = ReduceDamage(damage);
			Log.Out(Logs::Detail, Logs::Combat, "Melee Damage reduced to %d", damage);
			damage = ReduceAllDamage(damage);
			TryTriggerThreshHold(damage, SE_TriggerMeleeThreshold, attacker);

			if (skill_used)
				CheckNumHitsRemaining(NumHit::IncomingHitSuccess);

		} else {
			int32 origdmg = damage;
			damage = AffectMagicalDamage(damage, spell_id, iBuffTic, attacker);
			if (origdmg != damage && attacker && attacker->IsClient()) {
				if(attacker->CastToClient()->GetFilter(FilterDamageShields) != FilterHide)
					attacker->Message(15, "The Spellshield absorbed %d of %d points of damage", origdmg - damage, origdmg);
			}
			if (damage == 0 && attacker && origdmg != damage && IsClient()) {
				//Kayen: Probably need to add a filter for this - Not sure if this msg is correct but there should be a message for spell negate/runes.
				Message(263, "%s tries to cast on YOU, but YOUR magical skin absorbs the spell.",attacker->GetCleanName());
			}
			damage = ReduceAllDamage(damage);
			TryTriggerThreshHold(damage, SE_TriggerSpellThreshold, attacker);
		}

		if(IsClient() && CastToClient()->sneaking){
			CastToClient()->sneaking = false;
			SendAppearancePacket(AT_Sneak, 0);
		}
		if(attacker && attacker->IsClient() && attacker->CastToClient()->sneaking){
			attacker->CastToClient()->sneaking = false;
			attacker->SendAppearancePacket(AT_Sneak, 0);
		}

		//final damage has been determined.

		SetHP(GetHP() - damage);

		if (IsClient())
			this->CastToClient()->SendHPUpdateMarquee();

		if(HasDied()) {
			bool IsSaved = false;

			if(TryDivineSave())
				IsSaved = true;

			if(!IsSaved && !TrySpellOnDeath()) {
				SetHP(-500);

				if(Death(attacker, damage, spell_id, skill_used)) {
					return;
				}
			}
		}
		else{
			if(GetHPRatio() < 16)
				TryDeathSave();
		}

		TryTriggerOnValueAmount(true);

		//fade mez if we are mezzed
		if (IsMezzed() && attacker) {
			Log.Out(Logs::Detail, Logs::Combat, "Breaking mez due to attack.");
			entity_list.MessageClose_StringID(this, true, 100, MT_WornOff,
					HAS_BEEN_AWAKENED, GetCleanName(), attacker->GetCleanName());
			BuffFadeByEffect(SE_Mez);
		}

		// broken up for readability
		// This is based on what the client is doing
		// We had a bunch of stuff like BaseImmunityLevel checks, which I think is suppose to just be for spells
		// This is missing some merc checks, but those mostly just skipped the spell bonuses I think ...
		bool can_stun = false;
		int stunbash_chance = 0; // bonus
		if (attacker) {
			if (skill_used == EQEmu::skills::SkillBash) {
				can_stun = true;
				if (attacker->IsClient())
					stunbash_chance = attacker->spellbonuses.StunBashChance +
							  attacker->itembonuses.StunBashChance +
							  attacker->aabonuses.StunBashChance;
			} else if (skill_used == EQEmu::skills::SkillKick &&
				   (attacker->GetLevel() > 55 || attacker->IsNPC()) && GetClass() == WARRIOR) {
				can_stun = true;
			}

			if ((GetBaseRace() == OGRE || GetBaseRace() == OGGOK_CITIZEN) &&
			    !attacker->BehindMob(this, attacker->GetX(), attacker->GetY()))
				can_stun = false;
			if (GetSpecialAbility(UNSTUNABLE))
				can_stun = false;
		}
		if (can_stun) {
			int bashsave_roll = zone->random.Int(0, 100);
			if (bashsave_roll > 98 || bashsave_roll > (55 - stunbash_chance)) {
				// did stun -- roll other resists
				// SE_FrontalStunResist description says any angle now a days
				int stun_resist2 = spellbonuses.FrontalStunResist + itembonuses.FrontalStunResist +
						   aabonuses.FrontalStunResist;
				if (zone->random.Int(1, 100) > stun_resist2) {
					// stun resist 2 failed
					// time to check SE_StunResist and mod2 stun resist
					int stun_resist =
					    spellbonuses.StunResist + itembonuses.StunResist + aabonuses.StunResist;
					if (zone->random.Int(0, 100) >= stun_resist) {
						// did stun
						// nothing else to check!
						Stun(2000); // straight 2 seconds every time
					} else {
						// stun resist passed!
						if (IsClient())
							Message_StringID(MT_Stun, SHAKE_OFF_STUN);
					}
				} else {
					// stun resist 2 passed!
					if (IsClient())
						Message_StringID(MT_Stun, AVOID_STUNNING_BLOW);
				}
			} else {
				// main stun failed -- extra interrupt roll
				if (IsCasting() &&
				    !EQEmu::ValueWithin(casting_spell_id, 859, 1023)) // these spells are excluded
					// 90% chance >< -- stun immune won't reach this branch though :(
					if (zone->random.Int(0, 9) > 1)
						InterruptSpell();
			}
		}

		if(spell_id != SPELL_UNKNOWN && !iBuffTic) {
			//see if root will break
			if (IsRooted() && !FromDamageShield)  // neotoyko: only spells cancel root
				TryRootFadeByDamage(buffslot, attacker);
		}
		else if(spell_id == SPELL_UNKNOWN)
		{
			//increment chances of interrupting
			if(IsCasting()) { //shouldnt interrupt on regular spell damage
				attacked_count++;
				Log.Out(Logs::Detail, Logs::Combat, "Melee attack while casting. Attack count %d", attacked_count);
			}
		}

		//send an HP update if we are hurt
		if(GetHP() < GetMaxHP())
			SendHPUpdate(!iBuffTic); // the OP_Damage actually updates the client in these cases, so we skip the HP update for them
	}	//end `if damage was done`

	//send damage packet...
	if(!iBuffTic) { //buff ticks do not send damage, instead they just call SendHPUpdate(), which is done above
		auto outapp = new EQApplicationPacket(OP_Damage, sizeof(CombatDamage_Struct));
		CombatDamage_Struct* a = (CombatDamage_Struct*)outapp->pBuffer;
		a->target = GetID();
		if (attacker == nullptr)
			a->source = 0;
		else if (attacker->IsClient() && attacker->CastToClient()->GMHideMe())
			a->source = 0;
		else
			a->source = attacker->GetID();
		a->type = SkillDamageTypes[skill_used]; // was 0x1c
		a->damage = damage;
		a->spellid = spell_id;
		a->special = special;
		a->meleepush_xy = attacker ? attacker->GetHeading() * 2.0f : 0.0f;
		if (RuleB(Combat, MeleePush) && damage > 0 && !IsRooted() &&
		    (IsClient() || zone->random.Roll(RuleI(Combat, MeleePushChance)))) {
			a->force = EQEmu::skills::GetSkillMeleePushForce(skill_used);
			// update NPC stuff
			auto new_pos = glm::vec3(m_Position.x + (a->force * std::sin(a->meleepush_xy) + m_Delta.x),
						   m_Position.y + (a->force * std::cos(a->meleepush_xy) + m_Delta.y), m_Position.z);
			if (zone->zonemap && zone->zonemap->CheckLoS(glm::vec3(m_Position), new_pos)) { // If we have LoS on the new loc it should be reachable.
				if (IsNPC()) {
					// Is this adequate?
					Teleport(new_pos);
					SendPosUpdate();
				}
			} else {
				a->force = 0.0f; // we couldn't move there, so lets not
			}
		}

		//Note: if players can become pets, they will not receive damage messages of their own
		//this was done to simplify the code here (since we can only effectively skip one mob on queue)
		eqFilterType filter;
		Mob *skip = attacker;
		if(attacker && attacker->GetOwnerID()) {
			//attacker is a pet, let pet owners see their pet's damage
			Mob* owner = attacker->GetOwner();
			if (owner && owner->IsClient()) {
				if (((spell_id != SPELL_UNKNOWN) || (FromDamageShield)) && damage>0) {
					//special crap for spell damage, looks hackish to me
					char val1[20]={0};
					owner->Message_StringID(MT_NonMelee,OTHER_HIT_NONMELEE,GetCleanName(),ConvertArray(damage,val1));
				} else {
					if(damage > 0) {
						if(spell_id != SPELL_UNKNOWN)
							filter = iBuffTic ? FilterDOT : FilterSpellDamage;
						else
							filter = FilterPetHits;
					} else if(damage == -5)
						filter = FilterNone;	//cant filter invulnerable
					else
						filter = FilterPetMisses;

					if(!FromDamageShield)
						owner->CastToClient()->QueuePacket(outapp,true,CLIENT_CONNECTED,filter);
				}
			}
			skip = owner;
		} else {
			//attacker is not a pet, send to the attacker

			//if the attacker is a client, try them with the correct filter
			if(attacker && attacker->IsClient()) {
				if ((spell_id != SPELL_UNKNOWN || FromDamageShield) && damage > 0) {
					//special crap for spell damage, looks hackish to me
					char val1[20] = {0};
					if (FromDamageShield) {
						if (attacker->CastToClient()->GetFilter(FilterDamageShields) != FilterHide)
							attacker->Message_StringID(MT_DS,OTHER_HIT_NONMELEE, GetCleanName(), ConvertArray(damage, val1));
					} else {
						entity_list.MessageClose_StringID(this, true, 100, MT_NonMelee, HIT_NON_MELEE, attacker->GetCleanName(), GetCleanName(), ConvertArray(damage, val1));
					}
				} else {
					if(damage > 0) {
						if(spell_id != SPELL_UNKNOWN)
							filter = iBuffTic ? FilterDOT : FilterSpellDamage;
						else
							filter = FilterNone;	//cant filter our own hits
					} else if(damage == -5)
						filter = FilterNone;	//cant filter invulnerable
					else
						filter = FilterMyMisses;

					attacker->CastToClient()->QueuePacket(outapp, true, CLIENT_CONNECTED, filter);
				}
			}
			skip = attacker;
		}

		//send damage to all clients around except the specified skip mob (attacker or the attacker's owner) and ourself
		if(damage > 0) {
			if(spell_id != SPELL_UNKNOWN)
				filter = iBuffTic ? FilterDOT : FilterSpellDamage;
			else
				filter = FilterOthersHit;
		} else if(damage == -5)
			filter = FilterNone;	//cant filter invulnerable
		else
			filter = FilterOthersMiss;
		//make attacker (the attacker) send the packet so we can skip them and the owner
		//this call will send the packet to `this` as well (using the wrong filter) (will not happen until PC charm works)
		// If this is Damage Shield damage, the correct OP_Damage packets will be sent from Mob::DamageShield, so
		// we don't send them here.
		if(!FromDamageShield) {
			entity_list.QueueCloseClients(this, outapp, true, 200, skip, true, filter);
			//send the damage to ourself if we are a client
			if(IsClient()) {
				//I dont think any filters apply to damage affecting us
				CastToClient()->QueuePacket(outapp);
			}
		}

		safe_delete(outapp);
	} else {
		//else, it is a buff tic...
		// So we can see our dot dmg like live shows it.
		if(spell_id != SPELL_UNKNOWN && damage > 0 && attacker && attacker != this && attacker->IsClient()) {
			//might filter on (attack_skill>200 && attack_skill<250), but I dont think we need it
			attacker->FilteredMessage_StringID(attacker, MT_DoTDamage, FilterDOT,
					YOUR_HIT_DOT, GetCleanName(), itoa(damage), spells[spell_id].name);
			// older clients don't have the below String ID, but it will be filtered
			entity_list.FilteredMessageClose_StringID(attacker, true, 200,
					MT_DoTDamage, FilterDOT, OTHER_HIT_DOT, GetCleanName(),
					itoa(damage), attacker->GetCleanName(), spells[spell_id].name);
		}
	} //end packet sending

}

void Mob::HealDamage(uint32 amount, Mob *caster, uint16 spell_id)
{
	int32 maxhp = GetMaxHP();
	int32 curhp = GetHP();
	uint32 acthealed = 0;

	if (amount > (maxhp - curhp))
		acthealed = (maxhp - curhp);
	else
		acthealed = amount;

	if (acthealed > 100) {
		if (caster) {
			if (IsBuffSpell(spell_id)) { // hots
				// message to caster
				if (caster->IsClient() && caster == this) {
					if (caster->CastToClient()->ClientVersionBit() & EQEmu::versions::bit_SoFAndLater)
						FilteredMessage_StringID(caster, MT_NonMelee, FilterHealOverTime,
								HOT_HEAL_SELF, itoa(acthealed), spells[spell_id].name);
					else
						FilteredMessage_StringID(caster, MT_NonMelee, FilterHealOverTime,
								YOU_HEALED, GetCleanName(), itoa(acthealed));
				} else if (caster->IsClient() && caster != this) {
					if (caster->CastToClient()->ClientVersionBit() & EQEmu::versions::bit_SoFAndLater)
						caster->FilteredMessage_StringID(caster, MT_NonMelee, FilterHealOverTime,
								HOT_HEAL_OTHER, GetCleanName(), itoa(acthealed),
								spells[spell_id].name);
					else
						caster->FilteredMessage_StringID(caster, MT_NonMelee, FilterHealOverTime,
								YOU_HEAL, GetCleanName(), itoa(acthealed));
				}
				// message to target
				if (IsClient() && caster != this) {
					if (CastToClient()->ClientVersionBit() & EQEmu::versions::bit_SoFAndLater)
						FilteredMessage_StringID(this, MT_NonMelee, FilterHealOverTime,
								HOT_HEALED_OTHER, caster->GetCleanName(),
								itoa(acthealed), spells[spell_id].name);
					else
						FilteredMessage_StringID(this, MT_NonMelee, FilterHealOverTime,
								YOU_HEALED, caster->GetCleanName(), itoa(acthealed));
				}
			} else { // normal heals
				FilteredMessage_StringID(caster, MT_NonMelee, FilterSpellDamage,
						YOU_HEALED, caster->GetCleanName(), itoa(acthealed));
				if (caster != this)
					caster->FilteredMessage_StringID(caster, MT_NonMelee, FilterSpellDamage,
							YOU_HEAL, GetCleanName(), itoa(acthealed));
			}
		} else {
			Message(MT_NonMelee, "You have been healed for %d points of damage.", acthealed);
		}
	}

	if (curhp < maxhp) {
		if ((curhp + amount) > maxhp)
			curhp = maxhp;
		else
			curhp += amount;
		SetHP(curhp);

		SendHPUpdate();
	}
}

//proc chance includes proc bonus
float Mob::GetProcChances(float ProcBonus, uint16 hand)
{
	int mydex = GetDEX();
	float ProcChance = 0.0f;

	uint32 weapon_speed = GetWeaponSpeedbyHand(hand);

	if (RuleB(Combat, AdjustProcPerMinute)) {
		ProcChance = (static_cast<float>(weapon_speed) *
				RuleR(Combat, AvgProcsPerMinute) / 60000.0f); // compensate for weapon_speed being in ms
		ProcBonus += static_cast<float>(mydex) * RuleR(Combat, ProcPerMinDexContrib);
		ProcChance += ProcChance * ProcBonus / 100.0f;
	} else {
		ProcChance = RuleR(Combat, BaseProcChance) +
			static_cast<float>(mydex) / RuleR(Combat, ProcDexDivideBy);
		ProcChance += ProcChance * ProcBonus / 100.0f;
	}

	Log.Out(Logs::Detail, Logs::Combat, "Proc chance %.2f (%.2f from bonuses)", ProcChance, ProcBonus);
	return ProcChance;
}

float Mob::GetDefensiveProcChances(float &ProcBonus, float &ProcChance, uint16 hand, Mob* on) {

	if (!on)
		return ProcChance;

	int myagi = on->GetAGI();
	ProcBonus = 0;
	ProcChance = 0;

	uint32 weapon_speed = GetWeaponSpeedbyHand(hand);

	ProcChance = (static_cast<float>(weapon_speed) * RuleR(Combat, AvgDefProcsPerMinute) / 60000.0f); // compensate for weapon_speed being in ms
	ProcBonus += static_cast<float>(myagi) * RuleR(Combat, DefProcPerMinAgiContrib) / 100.0f;
	ProcChance = ProcChance + (ProcChance * ProcBonus);

	Log.Out(Logs::Detail, Logs::Combat, "Defensive Proc chance %.2f (%.2f from bonuses)", ProcChance, ProcBonus);
	return ProcChance;
}

// argument 'weapon' not used
void Mob::TryDefensiveProc(Mob *on, uint16 hand) {

	if (!on) {
		SetTarget(nullptr);
		Log.Out(Logs::General, Logs::Error, "A null Mob object was passed to Mob::TryDefensiveProc for evaluation!");
		return;
	}

	if (!HasDefensiveProcs())
		return;

	if (!on->HasDied() && on->GetHP() > 0){

		float ProcChance, ProcBonus;
		on->GetDefensiveProcChances(ProcBonus, ProcChance, hand , this);

		if (hand != EQEmu::inventory::slotPrimary)
			ProcChance /= 2;

		int level_penalty = 0;
		int level_diff = GetLevel() - on->GetLevel();
		if (level_diff > 6)//10% penalty per level if > 6 levels over target.
			level_penalty = (level_diff - 6) * 10;

		ProcChance -= ProcChance*level_penalty/100;

		if (ProcChance < 0)
			return;

		for (int i = 0; i < MAX_PROCS; i++) {
			if (IsValidSpell(DefensiveProcs[i].spellID)) {
				float chance = ProcChance * (static_cast<float>(DefensiveProcs[i].chance)/100.0f);
				if (zone->random.Roll(chance)) {
					ExecWeaponProc(nullptr, DefensiveProcs[i].spellID, on);
					CheckNumHitsRemaining(NumHit::DefensiveSpellProcs, 0,DefensiveProcs[i].base_spellID);
				}
			}
		}
	}
}

void Mob::TryWeaponProc(const EQEmu::ItemInstance* weapon_g, Mob *on, uint16 hand) {
	if(!on) {
		SetTarget(nullptr);
		Log.Out(Logs::General, Logs::Error, "A null Mob object was passed to Mob::TryWeaponProc for evaluation!");
		return;
	}

	if (!IsAttackAllowed(on)) {
		Log.Out(Logs::Detail, Logs::Combat, "Preventing procing off of unattackable things.");
		return;
	}

	if (DivineAura()) {
		Log.Out(Logs::Detail, Logs::Combat, "Procs canceled, Divine Aura is in effect.");
		return;
	}

	if(!weapon_g) {
		TrySpellProc(nullptr, (const EQEmu::ItemData*)nullptr, on);
		return;
	}

	if (!weapon_g->IsClassCommon()) {
		TrySpellProc(nullptr, (const EQEmu::ItemData*)nullptr, on);
		return;
	}

	// Innate + aug procs from weapons
	// TODO: powersource procs -- powersource procs are on invis augs, so shouldn't need anything extra
	TryWeaponProc(weapon_g, weapon_g->GetItem(), on, hand);
	// Procs from Buffs and AA both melee and range
	TrySpellProc(weapon_g, weapon_g->GetItem(), on, hand);

	return;
}

void Mob::TryWeaponProc(const EQEmu::ItemInstance *inst, const EQEmu::ItemData *weapon, Mob *on, uint16 hand)
{

	if (!weapon)
		return;
	uint16 skillinuse = 28;
	int ourlevel = GetLevel();
	float ProcBonus = static_cast<float>(aabonuses.ProcChanceSPA +
			spellbonuses.ProcChanceSPA + itembonuses.ProcChanceSPA);
	ProcBonus += static_cast<float>(itembonuses.ProcChance) / 10.0f; // Combat Effects
	float ProcChance = GetProcChances(ProcBonus, hand);

	if (hand != EQEmu::inventory::slotPrimary) //Is Archery intened to proc at 50% rate?
		ProcChance /= 2;

	// Try innate proc on weapon
	// We can proc once here, either weapon or one aug
	bool proced = false; // silly bool to prevent augs from going if weapon does
	skillinuse = GetSkillByItemType(weapon->ItemType);
	if (weapon->Proc.Type == EQEmu::item::ItemEffectCombatProc && IsValidSpell(weapon->Proc.Effect)) {
		float WPC = ProcChance * (100.0f + // Proc chance for this weapon
				static_cast<float>(weapon->ProcRate)) / 100.0f;
		if (zone->random.Roll(WPC)) {	// 255 dex = 0.084 chance of proc. No idea what this number should be really.
			if (weapon->Proc.Level > ourlevel) {
				Log.Out(Logs::Detail, Logs::Combat,
						"Tried to proc (%s), but our level (%d) is lower than required (%d)",
						weapon->Name, ourlevel, weapon->Proc.Level);
				if (IsPet()) {
					Mob *own = GetOwner();
					if (own)
						own->Message_StringID(13, PROC_PETTOOLOW);
				} else {
					Message_StringID(13, PROC_TOOLOW);
				}
			} else {
				Log.Out(Logs::Detail, Logs::Combat,
						"Attacking weapon (%s) successfully procing spell %d (%.2f percent chance)",
						weapon->Name, weapon->Proc.Effect, WPC * 100);
				ExecWeaponProc(inst, weapon->Proc.Effect, on);
				proced = true;
			}
		}
	}
	//If OneProcPerWeapon is not enabled, we reset the try for that weapon regardless of if we procced or not.
	//This is for some servers that may want to have as many procs triggering from weapons as possible in a single round.
	if(!RuleB(Combat, OneProcPerWeapon))
		proced = false;

	if (!proced && inst) {
		for (int r = EQEmu::inventory::socketBegin; r < EQEmu::inventory::SocketCount; r++) {
			const EQEmu::ItemInstance *aug_i = inst->GetAugment(r);
			if (!aug_i) // no aug, try next slot!
				continue;
			const EQEmu::ItemData *aug = aug_i->GetItem();
			if (!aug)
				continue;

			if (aug->Proc.Type == EQEmu::item::ItemEffectCombatProc && IsValidSpell(aug->Proc.Effect)) {
				float APC = ProcChance * (100.0f + // Proc chance for this aug
					static_cast<float>(aug->ProcRate)) / 100.0f;
				if (zone->random.Roll(APC)) {
					if (aug->Proc.Level > ourlevel) {
						if (IsPet()) {
							Mob *own = GetOwner();
							if (own)
								own->Message_StringID(13, PROC_PETTOOLOW);
						} else {
							Message_StringID(13, PROC_TOOLOW);
						}
					} else {
						ExecWeaponProc(aug_i, aug->Proc.Effect, on);
						if (RuleB(Combat, OneProcPerWeapon))
							break;
					}
				}
			}
		}
	}
	// TODO: Powersource procs -- powersource procs are from augs so shouldn't need anything extra

	return;
}

void Mob::TrySpellProc(const EQEmu::ItemInstance *inst, const EQEmu::ItemData *weapon, Mob *on, uint16 hand)
{
	float ProcBonus = static_cast<float>(spellbonuses.SpellProcChance +
			itembonuses.SpellProcChance + aabonuses.SpellProcChance);
	float ProcChance = 0.0f;
	ProcChance = GetProcChances(ProcBonus, hand);

	if (hand != EQEmu::inventory::slotPrimary) //Is Archery intened to proc at 50% rate?
		ProcChance /= 2;

	bool rangedattk = false;
	if (weapon && hand == EQEmu::inventory::slotRange) {
		if (weapon->ItemType == EQEmu::item::ItemTypeArrow ||
			weapon->ItemType == EQEmu::item::ItemTypeLargeThrowing ||
			weapon->ItemType == EQEmu::item::ItemTypeSmallThrowing ||
			weapon->ItemType == EQEmu::item::ItemTypeBow) {
			rangedattk = true;
		}
	}

	if (!weapon && hand == EQEmu::inventory::slotRange && GetSpecialAbility(SPECATK_RANGED_ATK))
		rangedattk = true;

	for (uint32 i = 0; i < MAX_PROCS; i++) {
		if (IsPet() && hand != EQEmu::inventory::slotPrimary) //Pets can only proc spell procs from their primay hand (ie; beastlord pets)
			continue; // If pets ever can proc from off hand, this will need to change

		// Not ranged
		if (!rangedattk) {
			// Perma procs (AAs)
			if (PermaProcs[i].spellID != SPELL_UNKNOWN) {
				if (zone->random.Roll(PermaProcs[i].chance)) { // TODO: Do these get spell bonus?
					Log.Out(Logs::Detail, Logs::Combat,
							"Permanent proc %d procing spell %d (%d percent chance)",
							i, PermaProcs[i].spellID, PermaProcs[i].chance);
					ExecWeaponProc(nullptr, PermaProcs[i].spellID, on);
				} else {
					Log.Out(Logs::Detail, Logs::Combat,
							"Permanent proc %d failed to proc %d (%d percent chance)",
							i, PermaProcs[i].spellID, PermaProcs[i].chance);
				}
			}

			// Spell procs (buffs)
			if (SpellProcs[i].spellID != SPELL_UNKNOWN) {
				float chance = ProcChance * (static_cast<float>(SpellProcs[i].chance) / 100.0f);
				if (zone->random.Roll(chance)) {
					Log.Out(Logs::Detail, Logs::Combat,
							"Spell proc %d procing spell %d (%.2f percent chance)",
							i, SpellProcs[i].spellID, chance);
					auto outapp = new EQApplicationPacket(OP_BeginCast,sizeof(BeginCast_Struct));
					BeginCast_Struct* begincast = (BeginCast_Struct*)outapp->pBuffer;
					begincast->caster_id = GetID();
					begincast->spell_id = SpellProcs[i].spellID;
					begincast->cast_time = 0;
					outapp->priority = 3;
					entity_list.QueueCloseClients(this, outapp, false, 200, 0, true);
					safe_delete(outapp);
					ExecWeaponProc(nullptr, SpellProcs[i].spellID, on, SpellProcs[i].level_override);
					CheckNumHitsRemaining(NumHit::OffensiveSpellProcs, 0,
								  SpellProcs[i].base_spellID);
				} else {
					Log.Out(Logs::Detail, Logs::Combat,
							"Spell proc %d failed to proc %d (%.2f percent chance)",
							i, SpellProcs[i].spellID, chance);
				}
			}
		} else if (rangedattk) { // ranged only
			// ranged spell procs (buffs)
			if (RangedProcs[i].spellID != SPELL_UNKNOWN) {
				float chance = ProcChance * (static_cast<float>(RangedProcs[i].chance) / 100.0f);
					if (zone->random.Roll(chance)) {
					Log.Out(Logs::Detail, Logs::Combat,
							"Ranged proc %d procing spell %d (%.2f percent chance)",
							i, RangedProcs[i].spellID, chance);
					ExecWeaponProc(nullptr, RangedProcs[i].spellID, on);
					CheckNumHitsRemaining(NumHit::OffensiveSpellProcs, 0,
								  RangedProcs[i].base_spellID);
				} else {
					Log.Out(Logs::Detail, Logs::Combat,
							"Ranged proc %d failed to proc %d (%.2f percent chance)",
							i, RangedProcs[i].spellID, chance);
				}
			}
		}
	}

	if (HasSkillProcs() && hand != EQEmu::inventory::slotRange){ //We check ranged skill procs within the attack functions.
		uint16 skillinuse = 28;
		if (weapon)
			skillinuse = GetSkillByItemType(weapon->ItemType);

		TrySkillProc(on, skillinuse, 0, false, hand);
	}

	return;
}

void Mob::TryPetCriticalHit(Mob *defender, uint16 skill, int32 &damage)
{
	if(damage < 1)
		return;

	//Allows pets to perform critical hits.
	//Each rank adds an additional 1% chance for any melee hit (primary, secondary, kick, bash, etc) to critical,
	//dealing up to 63% more damage. http://www.magecompendium.com/aa-short-library.html

	Mob *owner = nullptr;
	float critChance = 0.0f;
	critChance += RuleI(Combat, MeleeBaseCritChance);
	uint32 critMod = 163;

	if (damage < 1) //We can't critical hit if we don't hit.
		return;

	if (IsPet())
		owner = GetOwner();
	else if ((IsNPC() && CastToNPC()->GetSwarmOwner()))
		owner = entity_list.GetMobID(CastToNPC()->GetSwarmOwner());
	else
		return;

	if (!owner)
		return;

	int32 CritPetChance = owner->aabonuses.PetCriticalHit + owner->itembonuses.PetCriticalHit + owner->spellbonuses.PetCriticalHit;
	int32 CritChanceBonus = GetCriticalChanceBonus(skill);

	if (CritPetChance || critChance) {

		//For pets use PetCriticalHit for base chance, pets do not innately critical with without it
		//even if buffed with a CritChanceBonus effects.
		critChance += CritPetChance;
		critChance += critChance*CritChanceBonus/100.0f;
	}

	if(critChance > 0){

		critChance /= 100;

		if(zone->random.Roll(critChance))
		{
			critMod += GetCritDmgMob(skill) * 2; // To account for base crit mod being 200 not 100
			damage = (damage * critMod) / 100;
			entity_list.FilteredMessageClose_StringID(this, false, 200,
					MT_CritMelee, FilterMeleeCrits, CRITICAL_HIT,
					GetCleanName(), itoa(damage));
		}
	}
}

void Mob::TryCriticalHit(Mob *defender, uint16 skill, int32 &damage, ExtraAttackOptions *opts)
{
	if(damage < 1 || !defender)
		return;

	// decided to branch this into it's own function since it's going to be duplicating a lot of the
	// code in here, but could lead to some confusion otherwise
	if ((IsPet() && GetOwner()->IsClient()) || (IsNPC() && CastToNPC()->GetSwarmOwner())) {
		TryPetCriticalHit(defender,skill,damage);
		return;
	}

#ifdef BOTS
	if (this->IsPet() && this->GetOwner() && this->GetOwner()->IsBot()) {
		this->TryPetCriticalHit(defender,skill,damage);
		return;
	}
#endif //BOTS

	float critChance = 0.0f;
	bool IsBerskerSPA = false;

	//1: Try Slay Undead
	if (defender->GetBodyType() == BT_Undead ||
				defender->GetBodyType() == BT_SummonedUndead || defender->GetBodyType() == BT_Vampire) {
		int32 SlayRateBonus = aabonuses.SlayUndead[0] + itembonuses.SlayUndead[0] + spellbonuses.SlayUndead[0];
		if (SlayRateBonus) {
			float slayChance = static_cast<float>(SlayRateBonus) / 10000.0f;
			if (zone->random.Roll(slayChance)) {
				int32 SlayDmgBonus = aabonuses.SlayUndead[1] + itembonuses.SlayUndead[1] + spellbonuses.SlayUndead[1];
				damage = (damage * SlayDmgBonus * 2.25) / 100;
				if (GetGender() == 1) // female
					entity_list.FilteredMessageClose_StringID(this, false, 200,
							MT_CritMelee, FilterMeleeCrits, FEMALE_SLAYUNDEAD,
							GetCleanName(), itoa(damage));
				else // males and neuter I guess
					entity_list.FilteredMessageClose_StringID(this, false, 200,
							MT_CritMelee, FilterMeleeCrits, MALE_SLAYUNDEAD,
							GetCleanName(), itoa(damage));
				return;
			}
		}
	}

	//2: Try Melee Critical

	//Base critical rate for all classes is dervived from DEX stat, this rate is then augmented
	//by item,spell and AA bonuses allowing you a chance to critical hit. If the following rules
	//are defined you will have an innate chance to hit at Level 1 regardless of bonuses.
	//Warning: Do not define these rules if you want live like critical hits.
	critChance += RuleI(Combat, MeleeBaseCritChance);

	if (IsClient()) {
		critChance  += RuleI(Combat, ClientBaseCritChance);

		if (spellbonuses.BerserkSPA || itembonuses.BerserkSPA || aabonuses.BerserkSPA)
				IsBerskerSPA = true;

		if (((GetClass() == WARRIOR || GetClass() == BERSERKER) && GetLevel() >= 12)  || IsBerskerSPA) {
			if (IsBerserk() || IsBerskerSPA)
				critChance += RuleI(Combat, BerserkBaseCritChance);
			else
				critChance += RuleI(Combat, WarBerBaseCritChance);
		}
	}

	int deadlyChance = 0;
	int deadlyMod = 0;
	if (skill == EQEmu::skills::SkillArchery && GetClass() == RANGER && GetSkill(EQEmu::skills::SkillArchery) >= 65)
		critChance += 6;

	if (skill == EQEmu::skills::SkillThrowing && GetClass() == ROGUE && GetSkill(EQEmu::skills::SkillThrowing) >= 65) {
		critChance += RuleI(Combat, RogueCritThrowingChance);
		deadlyChance = RuleI(Combat, RogueDeadlyStrikeChance);
		deadlyMod = RuleI(Combat, RogueDeadlyStrikeMod);
	}

	int CritChanceBonus = GetCriticalChanceBonus(skill);

	if (CritChanceBonus || critChance) {

		//Get Base CritChance from Dex. (200 = ~1.6%, 255 = ~2.0%, 355 = ~2.20%) Fall off rate > 255
		//http://giline.versus.jp/shiden/su.htm , http://giline.versus.jp/shiden/damage_e.htm
		if (GetDEX() <= 255)
			critChance += (float(GetDEX()) / 125.0f);
		else if (GetDEX() > 255)
			critChance += (float(GetDEX()-255)/ 500.0f) + 2.0f;
		critChance += critChance*(float)CritChanceBonus /100.0f;
	}

	if(opts) {
		critChance *= opts->crit_percent;
		critChance += opts->crit_flat;
	}

	if(critChance > 0) {

		critChance /= 100;

		if(zone->random.Roll(critChance))
		{
			uint32 critMod = 200;
			bool crip_success = false;
			int32 CripplingBlowChance = GetCrippBlowChance();

			//Crippling Blow Chance: The percent value of the effect is applied
			//to the your Chance to Critical. (ie You have 10% chance to critical and you
			//have a 200% Chance to Critical Blow effect, therefore you have a 20% Chance to Critical Blow.
			if (CripplingBlowChance || (IsBerserk() || IsBerskerSPA)) {
				if (!IsBerserk() && !IsBerskerSPA)
					critChance *= float(CripplingBlowChance)/100.0f;

				if ((IsBerserk() || IsBerskerSPA) || zone->random.Roll(critChance)) {
					critMod = 400;
					crip_success = true;
				}
			}

			critMod += GetCritDmgMob(skill) * 2; // To account for base crit mod being 200 not 100
			damage = damage * critMod / 100;

			bool deadlySuccess = false;
			if (deadlyChance && zone->random.Roll(static_cast<float>(deadlyChance) / 100.0f)) {
				if (BehindMob(defender, GetX(), GetY())) {
					damage *= deadlyMod;
					deadlySuccess = true;
				}
			}

			if (crip_success) {
				entity_list.FilteredMessageClose_StringID(this, false, 200,
						MT_CritMelee, FilterMeleeCrits, CRIPPLING_BLOW,
						GetCleanName(), itoa(damage));
				// Crippling blows also have a chance to stun
				//Kayen: Crippling Blow would cause a chance to interrupt for npcs < 55, with a staggers message.
				if (defender->GetLevel() <= 55 && !defender->GetSpecialAbility(IMMUNE_STUN)){
					defender->Emote("staggers.");
					defender->Stun(0);
				}
			} else if (deadlySuccess) {
				entity_list.FilteredMessageClose_StringID(this, false, 200,
						MT_CritMelee, FilterMeleeCrits, DEADLY_STRIKE,
						GetCleanName(), itoa(damage));
			} else {
				entity_list.FilteredMessageClose_StringID(this, false, 200,
						MT_CritMelee, FilterMeleeCrits, CRITICAL_HIT,
						GetCleanName(), itoa(damage));
			}
		}
	}
}

bool Mob::TryFinishingBlow(Mob *defender, EQEmu::skills::SkillType skillinuse)
{
	if (defender && !defender->IsClient() && defender->GetHPRatio() < 10){

		uint32 FB_Dmg = aabonuses.FinishingBlow[1] + spellbonuses.FinishingBlow[1] + itembonuses.FinishingBlow[1];

		uint32 FB_Level = 0;
		FB_Level = aabonuses.FinishingBlowLvl[0];
		if (FB_Level < spellbonuses.FinishingBlowLvl[0])
			FB_Level = spellbonuses.FinishingBlowLvl[0];
		else if (FB_Level < itembonuses.FinishingBlowLvl[0])
			FB_Level = itembonuses.FinishingBlowLvl[0];

		//Proc Chance value of 500 = 5%
		uint32 ProcChance = (aabonuses.FinishingBlow[0] + spellbonuses.FinishingBlow[0] + spellbonuses.FinishingBlow[0])/10;

		if(FB_Level && FB_Dmg && (defender->GetLevel() <= FB_Level) && (ProcChance >= zone->random.Int(0, 1000))){
			entity_list.MessageClose_StringID(this, false, 200, MT_CritMelee, FINISHING_BLOW, GetName());
			DoSpecialAttackDamage(defender, skillinuse, FB_Dmg, 1, -1, 10, false, false);
			return true;
		}
	}
	return false;
}

void Mob::DoRiposte(Mob *defender)
{
	Log.Out(Logs::Detail, Logs::Combat, "Preforming a riposte");

	if (!defender)
		return;

	defender->Attack(this, EQEmu::inventory::slotPrimary, true);
	if (HasDied())
		return;

	// this effect isn't used on live? See no AAs or spells
	int32 DoubleRipChance = defender->aabonuses.DoubleRiposte + defender->spellbonuses.DoubleRiposte +
				defender->itembonuses.DoubleRiposte;

	if (DoubleRipChance && zone->random.Roll(DoubleRipChance)) {
		Log.Out(Logs::Detail, Logs::Combat,
			"Preforming a double riposted from SE_DoubleRiposte (%d percent chance)", DoubleRipChance);
		defender->Attack(this, EQEmu::inventory::slotPrimary, true);
		if (HasDied())
			return;
	}

	DoubleRipChance = defender->aabonuses.GiveDoubleRiposte[0] + defender->spellbonuses.GiveDoubleRiposte[0] +
			  defender->itembonuses.GiveDoubleRiposte[0];

	// Live AA - Double Riposte
	if (DoubleRipChance && zone->random.Roll(DoubleRipChance)) {
		Log.Out(Logs::Detail, Logs::Combat,
			"Preforming a double riposted from SE_GiveDoubleRiposte base1 == 0 (%d percent chance)",
			DoubleRipChance);
		defender->Attack(this, EQEmu::inventory::slotPrimary, true);
		if (HasDied())
			return;
	}

	// Double Riposte effect, allows for a chance to do RIPOSTE with a skill specific special attack (ie Return Kick).
	// Coded narrowly: Limit to one per client. Limit AA only. [1 = Skill Attack Chance, 2 = Skill]

	DoubleRipChance = defender->aabonuses.GiveDoubleRiposte[1];

	if (DoubleRipChance && zone->random.Roll(DoubleRipChance)) {
		Log.Out(Logs::Detail, Logs::Combat, "Preforming a return SPECIAL ATTACK (%d percent chance)",
			DoubleRipChance);

		if (defender->GetClass() == MONK)
			defender->MonkSpecialAttack(this, defender->aabonuses.GiveDoubleRiposte[2]);
		else if (defender->IsClient() && defender->CastToClient()->HasSkill((EQEmu::skills::SkillType)defender->aabonuses.GiveDoubleRiposte[2]))
			defender->CastToClient()->DoClassAttacks(this, defender->aabonuses.GiveDoubleRiposte[2], true);
	}
}

void Mob::ApplyMeleeDamageBonus(uint16 skill, int32 &damage,ExtraAttackOptions *opts){

	if(!RuleB(Combat, UseIntervalAC)){
		if(IsNPC()){ //across the board NPC damage bonuses.
			//only account for STR here, assume their base STR was factored into their DB damages
			int dmgbonusmod = 0;
			dmgbonusmod += (100*(itembonuses.STR + spellbonuses.STR))/3;
			dmgbonusmod += (100*(spellbonuses.ATK + itembonuses.ATK))/5;
			Log.Out(Logs::Detail, Logs::Combat, "Damage bonus: %d percent from ATK and STR bonuses.", (dmgbonusmod/100));
			damage += (damage*dmgbonusmod/10000);
		}
	}

	int dmgbonusmod = 0;
	
	dmgbonusmod += GetMeleeDamageMod_SE(skill);
	if (opts)
		dmgbonusmod += opts->melee_damage_bonus_flat;

	damage += damage * dmgbonusmod / 100;
}

bool Mob::HasDied() {
	bool Result = false;
	int32 hp_below = 0;

	hp_below = (GetDelayDeath() * -1);

	if((GetHP()) <= (hp_below))
		Result = true;

	return Result;
}

uint16 Mob::GetDamageTable(EQEmu::skills::SkillType skillinuse)
{
	if(GetLevel() <= 51)
	{
		uint32 ret_table = 0;
		int str_over_75 = 0;
		if(GetSTR() > 75)
			str_over_75 = GetSTR() - 75;
		if(str_over_75 > 255)
			ret_table = (GetSkill(skillinuse)+255)/2;
		else
			ret_table = (GetSkill(skillinuse)+str_over_75)/2;

		if(ret_table < 100)
			return 100;

		return ret_table;
	}
	else if(GetLevel() >= 90)
	{
		if(GetClass() == MONK)
			return 379;
		else
			return 345;
	}
	else
	{
		uint32 dmg_table[] = {
			275, 275, 275, 275, 275,
			280, 280, 280, 280,	285,
			285, 285, 290, 290, 295,
			295, 300, 300, 300, 305,
			305, 305, 310, 310, 315,
			315, 320, 320, 320, 325,
			325, 325, 330, 330, 335,
			335, 340, 340, 340,
		};
		if(GetClass() == MONK)
			return (dmg_table[GetLevel()-51]*(100+RuleI(Combat,MonkDamageTableBonus))/100);
		else
			return dmg_table[GetLevel()-51];
	}
}

void Mob::TrySkillProc(Mob *on, uint16 skill, uint16 ReuseTime, bool Success, uint16 hand, bool IsDefensive)
{

	if (!on) {
		SetTarget(nullptr);
		Log.Out(Logs::General, Logs::Error, "A null Mob object was passed to Mob::TrySkillProc for evaluation!");
		return;
	}

	if (!spellbonuses.LimitToSkill[skill] && !itembonuses.LimitToSkill[skill] && !aabonuses.LimitToSkill[skill])
		return;

	/*Allow one proc from each (Spell/Item/AA)
	Kayen: Due to limited avialability of effects on live it is too difficult
	to confirm how they stack at this time, will adjust formula when more data is avialablle to test.*/
	bool CanProc = true;

	uint16 base_spell_id = 0;
	uint16 proc_spell_id = 0;
	float ProcMod = 0;
	float chance = 0;

	if (IsDefensive)
		chance = on->GetSkillProcChances(ReuseTime, hand);
	else
		chance = GetSkillProcChances(ReuseTime, hand);

	if (spellbonuses.LimitToSkill[skill]){

		for(int e = 0; e < MAX_SKILL_PROCS; e++){
			if (CanProc &&
				((!Success && spellbonuses.SkillProc[e] && IsValidSpell(spellbonuses.SkillProc[e]))
				|| (Success && spellbonuses.SkillProcSuccess[e] && IsValidSpell(spellbonuses.SkillProcSuccess[e])))) {

				if (Success)
					base_spell_id = spellbonuses.SkillProcSuccess[e];
				else
					base_spell_id = spellbonuses.SkillProc[e];

				proc_spell_id = 0;
				ProcMod = 0;

				for (int i = 0; i < EFFECT_COUNT; i++) {

					if (spells[base_spell_id].effectid[i] == SE_SkillProc || spells[base_spell_id].effectid[i] == SE_SkillProcSuccess) {
						proc_spell_id = spells[base_spell_id].base[i];
						ProcMod = static_cast<float>(spells[base_spell_id].base2[i]);
					}

					else if (spells[base_spell_id].effectid[i] == SE_LimitToSkill && spells[base_spell_id].base[i] <= EQEmu::skills::HIGHEST_SKILL) {

						if (CanProc && spells[base_spell_id].base[i] == skill && IsValidSpell(proc_spell_id)) {
							float final_chance = chance * (ProcMod / 100.0f);
							if (zone->random.Roll(final_chance)) {
								ExecWeaponProc(nullptr, proc_spell_id, on);
								CheckNumHitsRemaining(NumHit::OffensiveSpellProcs, 0,
											  base_spell_id);
								CanProc = false;
								break;
							}
						}
					}
					else {
						//Reset and check for proc in sequence
						proc_spell_id = 0;
						ProcMod = 0;
					}
				}
			}
		}
	}

	if (itembonuses.LimitToSkill[skill]){
		CanProc = true;
		for(int e = 0; e < MAX_SKILL_PROCS; e++){
			if (CanProc &&
				((!Success && itembonuses.SkillProc[e] && IsValidSpell(itembonuses.SkillProc[e]))
				|| (Success && itembonuses.SkillProcSuccess[e] && IsValidSpell(itembonuses.SkillProcSuccess[e])))) {

				if (Success)
					base_spell_id = itembonuses.SkillProcSuccess[e];
				else
					base_spell_id = itembonuses.SkillProc[e];

				proc_spell_id = 0;
				ProcMod = 0;

				for (int i = 0; i < EFFECT_COUNT; i++) {
					if (spells[base_spell_id].effectid[i] == SE_SkillProc || spells[base_spell_id].effectid[i] == SE_SkillProcSuccess) {
						proc_spell_id = spells[base_spell_id].base[i];
						ProcMod = static_cast<float>(spells[base_spell_id].base2[i]);
					}

					else if (spells[base_spell_id].effectid[i] == SE_LimitToSkill && spells[base_spell_id].base[i] <= EQEmu::skills::HIGHEST_SKILL) {

						if (CanProc && spells[base_spell_id].base[i] == skill && IsValidSpell(proc_spell_id)) {
							float final_chance = chance * (ProcMod / 100.0f);
							if (zone->random.Roll(final_chance)) {
								ExecWeaponProc(nullptr, proc_spell_id, on);
								CanProc = false;
								break;
							}
						}
					}
					else {
						proc_spell_id = 0;
						ProcMod = 0;
					}
				}
			}
		}
	}

	if (IsClient() && aabonuses.LimitToSkill[skill]){

		CanProc = true;
		uint32 effect_id = 0;
		int32 base1 = 0;
		int32 base2 = 0;
		uint32 slot = 0;

		for(int e = 0; e < MAX_SKILL_PROCS; e++){
			if (CanProc &&
				((!Success && aabonuses.SkillProc[e])
				|| (Success && aabonuses.SkillProcSuccess[e]))) {
				int aaid = 0;

				if (Success)
					base_spell_id = aabonuses.SkillProcSuccess[e];
				else
					base_spell_id = aabonuses.SkillProc[e];

				proc_spell_id = 0;
				ProcMod = 0;

				for(auto &rank_info : aa_ranks) {
					auto ability_rank = zone->GetAlternateAdvancementAbilityAndRank(rank_info.first, rank_info.second.first);
					auto ability = ability_rank.first;
					auto rank = ability_rank.second;

					if(!ability) {
						continue;
					}

					for(auto &effect : rank->effects) {
						effect_id = effect.effect_id;
						base1 = effect.base1;
						base2 = effect.base2;
						slot = effect.slot;

						if(effect_id == SE_SkillProc || effect_id == SE_SkillProcSuccess) {
							proc_spell_id = base1;
							ProcMod = static_cast<float>(base2);
						}
						else if (effect_id == SE_LimitToSkill && base1 <= EQEmu::skills::HIGHEST_SKILL) {

							if (CanProc && base1 == skill && IsValidSpell(proc_spell_id)) {
								float final_chance = chance * (ProcMod / 100.0f);

								if (zone->random.Roll(final_chance)) {
									ExecWeaponProc(nullptr, proc_spell_id, on);
									CanProc = false;
									break;
								}
							}
						}
						else {
							proc_spell_id = 0;
							ProcMod = 0;
						}
					}
				}
			}
		}
	}
}

float Mob::GetSkillProcChances(uint16 ReuseTime, uint16 hand) {

	uint32 weapon_speed;
	float ProcChance = 0;

	if (!ReuseTime && hand) {
		weapon_speed = GetWeaponSpeedbyHand(hand);
		ProcChance = static_cast<float>(weapon_speed) * (RuleR(Combat, AvgProcsPerMinute) / 60000.0f);
		if (hand != EQEmu::inventory::slotPrimary)
			ProcChance /= 2;
	}

	else
		ProcChance = static_cast<float>(ReuseTime) * (RuleR(Combat, AvgProcsPerMinute) / 60000.0f);

	return ProcChance;
}

bool Mob::TryRootFadeByDamage(int buffslot, Mob* attacker) {

	/*Dev Quote 2010: http://forums.station.sony.com/eq/posts/list.m?topic_id=161443
	The Viscid Roots AA does the following: Reduces the chance for root to break by X percent.
	There is no distinction of any kind between the caster inflicted damage, or anyone
	else's damage. There is also no distinction between Direct and DOT damage in the root code.

	General Mechanics
	- Check buffslot to make sure damage from a root does not cancel the root
	- If multiple roots on target, always and only checks first root slot and if broken only removes that slots root.
	- Only roots on determental spells can be broken by damage.
	- Root break chance values obtained from live parses.
	*/

	if (!attacker || !spellbonuses.Root[0] || spellbonuses.Root[1] < 0)
		return false;

	if (IsDetrimentalSpell(spellbonuses.Root[1]) && spellbonuses.Root[1] != buffslot){
		int BreakChance = RuleI(Spells, RootBreakFromSpells);

		BreakChance -= BreakChance*buffs[spellbonuses.Root[1]].RootBreakChance/100;
		int level_diff = attacker->GetLevel() - GetLevel();

		//Use baseline if level difference <= 1 (ie. If target is (1) level less than you, or equal or greater level)

		if (level_diff == 2)
			BreakChance = (BreakChance * 80) /100; //Decrease by 20%;

		else if (level_diff >= 3 && level_diff <= 20)
			BreakChance = (BreakChance * 60) /100; //Decrease by 40%;

		else if (level_diff > 21)
			BreakChance = (BreakChance * 20) /100; //Decrease by 80%;

		if (BreakChance < 1)
			BreakChance = 1;

		if (zone->random.Roll(BreakChance)) {

			if (!TryFadeEffect(spellbonuses.Root[1])) {
				BuffFadeBySlot(spellbonuses.Root[1]);
				Log.Out(Logs::Detail, Logs::Combat, "Spell broke root! BreakChance percent chance");
				return true;
			}
		}
	}

	Log.Out(Logs::Detail, Logs::Combat, "Spell did not break root. BreakChance percent chance");
	return false;
}

int32 Mob::RuneAbsorb(int32 damage, uint16 type)
{
	uint32 buff_max = GetMaxTotalSlots();
	if (type == SE_Rune){
		for(uint32 slot = 0; slot < buff_max; slot++) {
			if(slot == spellbonuses.MeleeRune[1] && spellbonuses.MeleeRune[0] && buffs[slot].melee_rune && IsValidSpell(buffs[slot].spellid)){
				int melee_rune_left = buffs[slot].melee_rune;

				if(melee_rune_left > damage)
				{
					melee_rune_left -= damage;
					buffs[slot].melee_rune = melee_rune_left;
					return -6;
				}

				else
				{
					if(melee_rune_left > 0)
						damage -= melee_rune_left;

					if(!TryFadeEffect(slot))
						BuffFadeBySlot(slot);
				}
			}
		}
	}

	else{
		for(uint32 slot = 0; slot < buff_max; slot++) {
			if(slot == spellbonuses.AbsorbMagicAtt[1] && spellbonuses.AbsorbMagicAtt[0] && buffs[slot].magic_rune && IsValidSpell(buffs[slot].spellid)){
				int magic_rune_left = buffs[slot].magic_rune;
				if(magic_rune_left > damage)
				{
					magic_rune_left -= damage;
					buffs[slot].magic_rune = magic_rune_left;
					return 0;
				}

				else
				{
					if(magic_rune_left > 0)
						damage -= magic_rune_left;

					if(!TryFadeEffect(slot))
						BuffFadeBySlot(slot);
				}
			}
		}
	}

	return damage;
}

void Mob::CommonOutgoingHitSuccess(Mob* defender, int32 &damage, EQEmu::skills::SkillType skillInUse, ExtraAttackOptions *opts)
{
	if (!defender)
		return;

	ApplyMeleeDamageBonus(skillInUse, damage, opts);
	damage += (damage * defender->GetSkillDmgTaken(skillInUse, opts) / 100) + (GetSkillDmgAmt(skillInUse) + defender->GetFcDamageAmtIncoming(this, 0, true, skillInUse));
	TryCriticalHit(defender, skillInUse, damage,opts);
	CheckNumHitsRemaining(NumHit::OutgoingHitSuccess);
}

void Mob::CommonBreakInvisibleFromCombat()
{
	//break invis when you attack
	if(invisible) {
		Log.Out(Logs::Detail, Logs::Combat, "Removing invisibility due to melee attack.");
		BuffFadeByEffect(SE_Invisibility);
		BuffFadeByEffect(SE_Invisibility2);
		invisible = false;
	}
	if(invisible_undead) {
		Log.Out(Logs::Detail, Logs::Combat, "Removing invisibility vs. undead due to melee attack.");
		BuffFadeByEffect(SE_InvisVsUndead);
		BuffFadeByEffect(SE_InvisVsUndead2);
		invisible_undead = false;
	}
	if(invisible_animals){
		Log.Out(Logs::Detail, Logs::Combat, "Removing invisibility vs. animals due to melee attack.");
		BuffFadeByEffect(SE_InvisVsAnimals);
		invisible_animals = false;
	}

	CancelSneakHide();

	if (spellbonuses.NegateIfCombat)
		BuffFadeByEffect(SE_NegateIfCombat);

	hidden = false;
	improved_hidden = false;
}

/* Dev quotes:
 * Old formula
 *	 Final delay = (Original Delay / (haste mod *.01f)) + ((Hundred Hands / 100) * Original Delay)
 * New formula
 *	 Final delay = (Original Delay / (haste mod *.01f)) + ((Hundred Hands / 1000) * (Original Delay / (haste mod *.01f))
 * Base Delay	  20			  25			  30			  37
 * Haste		   2.25			2.25			2.25			2.25
 * HHE (old)	  -17			 -17			 -17			 -17
 * Final Delay	 5.488888889	 6.861111111	 8.233333333	 10.15444444
 *
 * Base Delay	  20			  25			  30			  37
 * Haste		   2.25			2.25			2.25			2.25
 * HHE (new)	  -383			-383			-383			-383
 * Final Delay	 5.484444444	 6.855555556	 8.226666667	 10.14622222
 *
 * Difference	 -0.004444444   -0.005555556   -0.006666667   -0.008222222
 *
 * These times are in 10th of a second
 */

void Mob::SetAttackTimer()
{
	attack_timer.SetAtTrigger(4000, true);
}

void Client::SetAttackTimer()
{
	float haste_mod = GetHaste() * 0.01f;

	//default value for attack timer in case they have
	//an invalid weapon equipped:
	attack_timer.SetAtTrigger(4000, true);

	Timer *TimerToUse = nullptr;

	for (int i = EQEmu::inventory::slotRange; i <= EQEmu::inventory::slotSecondary; i++) {
		//pick a timer
		if (i == EQEmu::inventory::slotPrimary)
			TimerToUse = &attack_timer;
		else if (i == EQEmu::inventory::slotRange)
			TimerToUse = &ranged_timer;
		else if (i == EQEmu::inventory::slotSecondary)
			TimerToUse = &attack_dw_timer;
		else	//invalid slot (hands will always hit this)
			continue;

		const EQEmu::ItemData *ItemToUse = nullptr;

		//find our item
		EQEmu::ItemInstance *ci = GetInv().GetItem(i);
		if (ci)
			ItemToUse = ci->GetItem();

		//special offhand stuff
		if (i == EQEmu::inventory::slotSecondary) {
			//if we cant dual wield, skip it
			if (!CanThisClassDualWield() || HasTwoHanderEquipped()) {
				attack_dw_timer.Disable();
				continue;
			}
		}

		//see if we have a valid weapon
		if (ItemToUse != nullptr) {
			//check type and damage/delay
			if (!ItemToUse->IsClassCommon()
				|| ItemToUse->Damage == 0
				|| ItemToUse->Delay == 0) {
				//no weapon
				ItemToUse = nullptr;
			}
			// Check to see if skill is valid
			else if ((ItemToUse->ItemType > EQEmu::item::ItemTypeLargeThrowing) &&
				(ItemToUse->ItemType != EQEmu::item::ItemTypeMartial) &&
				(ItemToUse->ItemType != EQEmu::item::ItemType2HPiercing)) {
				//no weapon
				ItemToUse = nullptr;
			}
		}

		int hhe = itembonuses.HundredHands + spellbonuses.HundredHands;
		int speed = 0;
		int delay = 3500;

		//if we have no weapon..
		if (ItemToUse == nullptr)
			delay = 100 * GetHandToHandDelay();
		else
			//we have a weapon, use its delay
			delay = 100 * ItemToUse->Delay;

		speed = delay / haste_mod;

		if (ItemToUse && ItemToUse->ItemType == EQEmu::item::ItemTypeBow) {
			// Live actually had a bug here where they would return the non-modified attack speed
			// rather than the cap ...
			speed = std::max(speed - GetQuiverHaste(speed), RuleI(Combat, QuiverHasteCap));
		} else {
			if (RuleB(Spells, Jun182014HundredHandsRevamp))
				speed = static_cast<int>(speed + ((hhe / 1000.0f) * speed));
			else
				speed = static_cast<int>(speed + ((hhe / 100.0f) * delay));
		}
		TimerToUse->SetAtTrigger(std::max(RuleI(Combat, MinHastedDelay), speed), true, true);
	}
}

void NPC::SetAttackTimer()
{
	float haste_mod = GetHaste() * 0.01f;

	//default value for attack timer in case they have
	//an invalid weapon equipped:
	attack_timer.SetAtTrigger(4000, true);

	Timer *TimerToUse = nullptr;
	int hhe = itembonuses.HundredHands + spellbonuses.HundredHands;

	// Technically NPCs should do some logic for weapons, but the effect is minimal
	// What they do is take the lower of their set delay and the weapon's
	// ex. Mob's delay set to 20, weapon set to 19, delay 19
	// Mob's delay set to 20, weapon set to 21, delay 20
	int speed = 0;
	if (RuleB(Spells, Jun182014HundredHandsRevamp))
		speed = static_cast<int>(((attack_delay / haste_mod) + ((hhe / 1000.0f) * (attack_delay / haste_mod))) * 100);
	else
		speed = static_cast<int>(((attack_delay / haste_mod) + ((hhe / 100.0f) * attack_delay)) * 100);

	for (int i = EQEmu::inventory::slotRange; i <= EQEmu::inventory::slotSecondary; i++) {
		//pick a timer
		if (i == EQEmu::inventory::slotPrimary)
			TimerToUse = &attack_timer;
		else if (i == EQEmu::inventory::slotRange)
			TimerToUse = &ranged_timer;
		else if (i == EQEmu::inventory::slotSecondary)
			TimerToUse = &attack_dw_timer;
		else	//invalid slot (hands will always hit this)
			continue;

		//special offhand stuff
		if (i == EQEmu::inventory::slotSecondary) {
			// SPECATK_QUAD is uncheesable
			if(!CanThisClassDualWield() || (HasTwoHanderEquipped() && !GetSpecialAbility(SPECATK_QUAD))) {
				attack_dw_timer.Disable();
				continue;
			}
		}

		TimerToUse->SetAtTrigger(std::max(RuleI(Combat, MinHastedDelay), speed), true, true);
	}
}

void Client::DoAttackRounds(Mob *target, int hand, bool IsFromSpell)
{
	if (!target)
		return;

	Attack(target, hand, false, false, IsFromSpell);

	bool candouble = CanThisClassDoubleAttack();
	// extra off hand non-sense, can only double with skill of 150 or above
	// or you have any amount of GiveDoubleAttack
	if (candouble && hand == EQEmu::inventory::slotSecondary)
		candouble = GetSkill(EQEmu::skills::SkillDoubleAttack) > 149 || (aabonuses.GiveDoubleAttack + spellbonuses.GiveDoubleAttack + itembonuses.GiveDoubleAttack) > 0;

	if (candouble) {
		CheckIncreaseSkill(EQEmu::skills::SkillDoubleAttack, target, -10);
		if (CheckDoubleAttack()) {
			Attack(target, hand, false, false, IsFromSpell);
			// you can only triple from the main hand
			if (hand == EQEmu::inventory::slotPrimary && CanThisClassTripleAttack()) {
				CheckIncreaseSkill(EQEmu::skills::SkillTripleAttack, target, -10);
				if (CheckTripleAttack())
					Attack(target, hand, false, false, IsFromSpell);
			}
		}
	}

	if (hand == EQEmu::inventory::slotPrimary) {
		// According to http://www.monkly-business.net/forums/showpost.php?p=312095&postcount=168 a dev told them flurry isn't dependant on triple attack
		// the parses kind of back that up and all of my parses seemed to be 4 or 5 attacks in the round which would work out to be
		// doubles or triples with 2 from flurries or triple with 1 or 2 flurries ... Going with the "dev quote" I guess like we've always had it
		auto flurrychance = aabonuses.FlurryChance + spellbonuses.FlurryChance + itembonuses.FlurryChance;
		if (flurrychance && zone->random.Roll(flurrychance)) {
			Attack(target, hand, false, false, IsFromSpell);
			Attack(target, hand, false, false, IsFromSpell);
			Message_StringID(MT_NPCFlurry, YOU_FLURRY);
		}
		// I haven't parsed where this guy happens, but it's not part of the normal chain above so this is fine
		auto extraattackchance = aabonuses.ExtraAttackChance + spellbonuses.ExtraAttackChance + itembonuses.ExtraAttackChance;
		if (extraattackchance && HasTwoHanderEquipped() && zone->random.Roll(extraattackchance))
			Attack(target, hand, false, false, IsFromSpell);
	}
}

bool Mob::CheckDualWield()
{
	// Pets /might/ follow a slightly different progression
	// although it could all be from pets having different skills than most mobs
	int chance = GetSkill(EQEmu::skills::SkillDualWield);
	if (GetLevel() > 35)
		chance += GetLevel();

	chance += aabonuses.Ambidexterity + spellbonuses.Ambidexterity + itembonuses.Ambidexterity;
	int per_inc = spellbonuses.DualWieldChance + aabonuses.DualWieldChance + itembonuses.DualWieldChance;
	if (per_inc)
		chance += chance * per_inc / 100;

	return zone->random.Int(1, 375) <= chance;
}

bool Client::CheckDualWield()
{
	int chance = GetSkill(EQEmu::skills::SkillDualWield) + GetLevel();

	chance += aabonuses.Ambidexterity + spellbonuses.Ambidexterity + itembonuses.Ambidexterity;
	int per_inc = spellbonuses.DualWieldChance + aabonuses.DualWieldChance + itembonuses.DualWieldChance;
	if (per_inc)
		chance += chance * per_inc / 100;

	return zone->random.Int(1, 375) <= chance;
}

void Mob::DoMainHandAttackRounds(Mob *target, ExtraAttackOptions *opts, int special)
{
	if (!target)
		return;

	if (RuleB(Combat, UseLiveCombatRounds)) {
		// A "quad" on live really is just a successful dual wield where both double attack
		// The mobs that could triple lost the ability to when the triple attack skill was added in
		Attack(target, EQEmu::inventory::slotPrimary, false, false, false, opts, special);
		if (CanThisClassDoubleAttack() && CheckDoubleAttack()){
			Attack(target, EQEmu::inventory::slotPrimary, false, false, false, opts, special);
								
			if ((IsPet() || IsTempPet()) && IsPetOwnerClient()){
				int chance = spellbonuses.PC_Pet_Flurry + itembonuses.PC_Pet_Flurry + aabonuses.PC_Pet_Flurry;
				if (chance && zone->random.Roll(chance))
					Flurry(nullptr);
			}
		}
		return;
	}

	if (IsNPC()) {
		int16 n_atk = CastToNPC()->GetNumberOfAttacks();
		if (n_atk <= 1) {
			Attack(target, EQEmu::inventory::slotPrimary, false, false, false, opts, special);
		} else {
			for (int i = 0; i < n_atk; ++i) {
				Attack(target, EQEmu::inventory::slotPrimary, false, false, false, opts, special);
			}
		}
	} else {
		Attack(target, EQEmu::inventory::slotPrimary, false, false, false, opts, special);
	}

	// we use this random value in three comparisons with different
	// thresholds, and if its truely random, then this should work
	// out reasonably and will save us compute resources.
	int32 RandRoll = zone->random.Int(0, 99);
	if ((CanThisClassDoubleAttack() || GetSpecialAbility(SPECATK_TRIPLE) || GetSpecialAbility(SPECATK_QUAD))
	    // check double attack, this is NOT the same rules that clients use...
	    &&
	    RandRoll < (GetLevel() + NPCDualAttackModifier)) {
		Attack(target, EQEmu::inventory::slotPrimary, false, false, false, opts, special);
		// lets see if we can do a triple attack with the main hand
		// pets are excluded from triple and quads...
		if ((GetSpecialAbility(SPECATK_TRIPLE) || GetSpecialAbility(SPECATK_QUAD)) && !IsPet() &&
		    RandRoll < (GetLevel() + NPCTripleAttackModifier)) {
			Attack(target, EQEmu::inventory::slotPrimary, false, false, false, opts, special);
			// now lets check the quad attack
			if (GetSpecialAbility(SPECATK_QUAD) && RandRoll < (GetLevel() + NPCQuadAttackModifier)) {
				Attack(target, EQEmu::inventory::slotPrimary, false, false, false, opts, special);
			}
		}
	}
}

void Mob::DoOffHandAttackRounds(Mob *target, ExtraAttackOptions *opts, int special)
{
	if (!target)
		return;
	// Mobs will only dual wield w/ the flag or have a secondary weapon
	// For now, SPECATK_QUAD means innate DW when Combat:UseLiveCombatRounds is true
	if ((GetSpecialAbility(SPECATK_INNATE_DW) ||
	     (RuleB(Combat, UseLiveCombatRounds) && GetSpecialAbility(SPECATK_QUAD))) ||
		 GetEquipment(EQEmu::textures::weaponSecondary) != 0) {
		if (CheckDualWield()) {
			Attack(target, EQEmu::inventory::slotSecondary, false, false, false, opts, special);
			if (CanThisClassDoubleAttack() && GetLevel() > 35 && CheckDoubleAttack()){
				Attack(target, EQEmu::inventory::slotSecondary, false, false, false, opts, special);

				if ((IsPet() || IsTempPet()) && IsPetOwnerClient()){
					int chance = spellbonuses.PC_Pet_Flurry + itembonuses.PC_Pet_Flurry + aabonuses.PC_Pet_Flurry;
					if (chance && zone->random.Roll(chance))
						Flurry(nullptr);
				}
			}
		}
	}
}
