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
#include "../common/misc_functions.h"
#include "queryserv.h"
#include "quest_parser_collection.h"
#include "string_ids.h"
#include "water_map.h"
#include "worldserver.h"
#include "zone.h"
#include "lua_parser.h"

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

EQEmu::skills::SkillType Mob::AttackAnimation(int Hand, const EQEmu::ItemInstance* weapon, EQEmu::skills::SkillType skillinuse)
{
	// Determine animation
	int type = 0;
	if (weapon && weapon->IsClassCommon()) {
		const EQEmu::ItemData* item = weapon->GetItem();

		Log(Logs::Detail, Logs::Attack, "Weapon skill : %i", item->ItemType);

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
	else if (IsNPC()) {
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

	DoAnim(type, 0, false);

	return skillinuse;
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
		}
		else if (IsBerserk()) {
			tohit += (GetLevel() * 2) / 5;
		}
	}
	return std::max(tohit, 1);
}

// return -1 in cases that always hit
int Mob::GetTotalToHit(EQEmu::skills::SkillType skill, int chance_mod)
{
	if (chance_mod >= 10000) // override for stuff like SE_SkillAttack
		return -1;

	// calculate attacker's accuracy
	auto accuracy = compute_tohit(skill) + 10; // add 10 in case the NPC's stats are fucked
	if (chance_mod > 0) // multiplier
		accuracy *= chance_mod;

	// Torven parsed an apparent constant of 1.2 somewhere in here * 6 / 5 looks eqmathy to me!
	// new test clients have 121 / 100
	accuracy = (accuracy * 121) / 100;

	// unsure on the stacking order of these effects, rather hard to parse
	// item mod2 accuracy isn't applied to range? Theory crafting and parses back it up I guess
	// mod2 accuracy -- flat bonus
	if (skill != EQEmu::skills::SkillArchery && skill != EQEmu::skills::SkillThrowing)
		accuracy += itembonuses.HitChance;

	// 216 Melee Accuracy Amt aka SE_Accuracy -- flat bonus
	accuracy += itembonuses.Accuracy[EQEmu::skills::HIGHEST_SKILL + 1] +
		aabonuses.Accuracy[EQEmu::skills::HIGHEST_SKILL + 1] +
		spellbonuses.Accuracy[EQEmu::skills::HIGHEST_SKILL + 1] +
		itembonuses.Accuracy[skill] +
		aabonuses.Accuracy[skill] +
		spellbonuses.Accuracy[skill];

	// auto hit discs (and looks like there are some autohit AAs)
	if (spellbonuses.HitChanceEffect[skill] >= 10000 || aabonuses.HitChanceEffect[skill] >= 10000)
		return -1;

	if (spellbonuses.HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1] >= 10000)
		return -1;

	// 184 Accuracy % aka SE_HitChance -- percentage increase
	auto hit_bonus = itembonuses.HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1] +
		aabonuses.HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1] +
		spellbonuses.HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1] +
		itembonuses.HitChanceEffect[skill] +
		aabonuses.HitChanceEffect[skill] +
		spellbonuses.HitChanceEffect[skill];

	accuracy = (accuracy * (100 + hit_bonus)) / 100;

	// TODO: April 2003 added an archery/throwing PVP accuracy penalty while moving, should be in here some where,
	// but PVP is less important so I haven't tried parsing it at all

	// There is also 110 Ranger Archery Accuracy % which should probably be in here some where, but it's not in any spells/aas
	// Name implies it's a percentage increase, if one wishes to implement, do it like the hit_bonus above but limited to ranger archery

	// There is also 183 UNUSED - Skill Increase Chance which devs say isn't used at all in code, but some spells reference it
	// I do not recommend implementing this once since there are spells that use it which would make this not live-like with default spell files
	return accuracy;
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

// return -1 in cases that always miss
int Mob::GetTotalDefense()
{
	auto avoidance = compute_defense() + 10; // add 10 in case the NPC's stats are fucked
	auto evasion_bonus = spellbonuses.AvoidMeleeChanceEffect; // we check this first since it has a special case
	if (evasion_bonus >= 10000)
		return -1;
	//
	// 172 Evasion aka SE_AvoidMeleeChance
	evasion_bonus += itembonuses.AvoidMeleeChanceEffect + aabonuses.AvoidMeleeChanceEffect; // item bonus here isn't mod2 avoidance

	Mob *owner = nullptr;
	if (IsPet())
		owner = GetOwner();
	else if (IsNPC() && CastToNPC()->GetSwarmOwner())
		owner = entity_list.GetMobID(CastToNPC()->GetSwarmOwner());

	if (owner) // 215 Pet Avoidance % aka SE_PetAvoidance
		evasion_bonus += owner->aabonuses.PetAvoidance + owner->spellbonuses.PetAvoidance + owner->itembonuses.PetAvoidance;

	// Evasion is a percentage bonus according to AA descriptions
	if (evasion_bonus)
		avoidance = (avoidance * (100 + evasion_bonus)) / 100;

	return avoidance;
}

// called when a mob is attacked, does the checks to see if it's a hit
// and does other mitigation checks. 'this' is the mob being attacked.
bool Mob::CheckHitChance(Mob* other, DamageHitInfo &hit)
{
#ifdef LUA_EQEMU
	bool lua_ret = false;
	bool ignoreDefault = false;
	lua_ret = LuaParser::Instance()->CheckHitChance(this, other, hit, ignoreDefault);
	
	if(ignoreDefault) {
		return lua_ret;
	}
#endif

	Mob *attacker = other;
	Mob *defender = this;
	Log(Logs::Detail, Logs::Attack, "CheckHitChance(%s) attacked by %s", defender->GetName(), attacker->GetName());

	if (defender->IsClient() && defender->CastToClient()->IsSitting())
		return true;

	auto avoidance = defender->GetTotalDefense();
	if (avoidance == -1) // some sort of auto avoid disc
		return false;

	auto accuracy = hit.tohit;
	if (accuracy == -1)
		return true;

	// so now we roll!
	// relevant dev quote:
	// Then your chance to simply avoid the attack is checked (defender's avoidance roll beat the attacker's accuracy roll.)
	int tohit_roll = zone->random.Roll0(accuracy);
	int avoid_roll = zone->random.Roll0(avoidance);
	Log(Logs::Detail, Logs::Attack, "CheckHitChance accuracy(%d => %d) avoidance(%d => %d)", accuracy, tohit_roll, avoidance, avoid_roll);

	// tie breaker? Don't want to be biased any one way
	if (tohit_roll == avoid_roll)
		return zone->random.Roll(50);
	return tohit_roll > avoid_roll;
}

bool Mob::AvoidDamage(Mob *other, DamageHitInfo &hit)
{
#ifdef LUA_EQEMU
	bool lua_ret = false;
	bool ignoreDefault = false;
	lua_ret = LuaParser::Instance()->AvoidDamage(this, other, hit, ignoreDefault);
	
	if (ignoreDefault) {
		return lua_ret;
	}
#endif

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
	bool ImmuneRipo = attacker->aabonuses.RiposteChance || attacker->spellbonuses.RiposteChance || attacker->itembonuses.RiposteChance || attacker->IsEnraged();
	// Need to check if we have something in MainHand to actually attack with (or fists)
	if (hit.hand != EQEmu::inventory::slotRange && (CanThisClassRiposte() || IsEnraged()) && InFront && !ImmuneRipo) {
		if (IsEnraged()) {
			hit.damage_done = DMG_RIPOSTED;
			Log(Logs::Detail, Logs::Combat, "I am enraged, riposting frontal attack.");
			return true;
		}
		if (IsClient())
			CastToClient()->CheckIncreaseSkill(EQEmu::skills::SkillRiposte, other, -10);
		// check auto discs ... I guess aa/items too :P
		if (spellbonuses.RiposteChance == 10000 || aabonuses.RiposteChance == 10000 || itembonuses.RiposteChance == 10000) {
			hit.damage_done = DMG_RIPOSTED;
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
		if (hit.hand == EQEmu::inventory::slotSecondary) {
			int slip = aabonuses.OffhandRiposteFail + itembonuses.OffhandRiposteFail + spellbonuses.OffhandRiposteFail;
			chance += chance * slip / 100;
		}
		if (chance > 0 && zone->random.Roll(chance)) { // could be <0 from offhand stuff
			hit.damage_done = DMG_RIPOSTED;
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
			hit.damage_done = DMG_BLOCKED;
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
			hit.damage_done = DMG_BLOCKED;
			return true;
		}
	}

	// parry
	if (CanThisClassParry() && InFront && hit.hand != EQEmu::inventory::slotRange) {
		if (IsClient())
			CastToClient()->CheckIncreaseSkill(EQEmu::skills::SkillParry, other, -10);
		// check auto discs ... I guess aa/items too :P
		if (spellbonuses.ParryChance == 10000 || aabonuses.ParryChance == 10000 || itembonuses.ParryChance == 10000) {
			hit.damage_done = DMG_PARRIED;
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
			hit.damage_done = DMG_PARRIED;
			return true;
		}
	}

	// dodge
	if (CanThisClassDodge() && (InFront || GetClass() == MONK)) {
		if (IsClient())
			CastToClient()->CheckIncreaseSkill(EQEmu::skills::SkillDodge, other, -10);
		// check auto discs ... I guess aa/items too :P
		if (spellbonuses.DodgeChance == 10000 || aabonuses.DodgeChance == 10000 || itembonuses.DodgeChance == 10000) {
			hit.damage_done = DMG_DODGED;
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
			hit.damage_done = DMG_DODGED;
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
			hit.damage_done = DMG_BLOCKED;
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
			hit.damage_done = DMG_BLOCKED;
			return true;
		}
	}

	return false;
}

int Mob::GetACSoftcap()
{
	// from test server Resources/ACMitigation.txt
	static int war_softcaps[] = {
		312, 314, 316, 318, 320, 322, 324, 326, 328, 330, 332, 334, 336, 338, 340, 342, 344, 346, 348, 350, 352,
		354, 356, 358, 360, 362, 364, 366, 368, 370, 372, 374, 376, 378, 380, 382, 384, 386, 388, 390, 392, 394,
		396, 398, 400, 402, 404, 406, 408, 410, 412, 414, 416, 418, 420, 422, 424, 426, 428, 430, 432, 434, 436,
		438, 440, 442, 444, 446, 448, 450, 452, 454, 456, 458, 460, 462, 464, 466, 468, 470, 472, 474, 476, 478,
		480, 482, 484, 486, 488, 490, 492, 494, 496, 498, 500, 502, 504, 506, 508, 510, 512, 514, 516, 518, 520
	};

	static int clrbrdmnk_softcaps[] = {
		274, 276, 278, 278, 280, 282, 284, 286, 288, 290, 292, 292, 294, 296, 298, 300, 302, 304, 306, 308, 308,
		310, 312, 314, 316, 318, 320, 322, 322, 324, 326, 328, 330, 332, 334, 336, 336, 338, 340, 342, 344, 346,
		348, 350, 352, 352, 354, 356, 358, 360, 362, 364, 366, 366, 368, 370, 372, 374, 376, 378, 380, 380, 382,
		384, 386, 388, 390, 392, 394, 396, 396, 398, 400, 402, 404, 406, 408, 410, 410, 412, 414, 416, 418, 420,
		422, 424, 424, 426, 428, 430, 432, 434, 436, 438, 440, 440, 442, 444, 446, 448, 450, 452, 454, 454, 456
	};

	static int palshd_softcaps[] = {
		298, 300, 302, 304, 306, 308, 310, 312, 314, 316, 318, 320, 322, 324, 326, 328, 330, 332, 334, 336, 336,
		338, 340, 342, 344, 346, 348, 350, 352, 354, 356, 358, 360, 362, 364, 366, 368, 370, 372, 374, 376, 378,
		380, 382, 384, 384, 386, 388, 390, 392, 394, 396, 398, 400, 402, 404, 406, 408, 410, 412, 414, 416, 418,
		420, 422, 424, 426, 428, 430, 432, 432, 434, 436, 438, 440, 442, 444, 446, 448, 450, 452, 454, 456, 458,
		460, 462, 464, 466, 468, 470, 472, 474, 476, 478, 480, 480, 482, 484, 486, 488, 490, 492, 494, 496, 498
	};

	static int rng_softcaps[] = {
		286, 288, 290, 292, 294, 296, 298, 298, 300, 302, 304, 306, 308, 310, 312, 314, 316, 318, 320, 322, 322,
		324, 326, 328, 330, 332, 334, 336, 338, 340, 342, 344, 344, 346, 348, 350, 352, 354, 356, 358, 360, 362,
		364, 366, 368, 368, 370, 372, 374, 376, 378, 380, 382, 384, 386, 388, 390, 390, 392, 394, 396, 398, 400,
		402, 404, 406, 408, 410, 412, 414, 414, 416, 418, 420, 422, 424, 426, 428, 430, 432, 434, 436, 436, 438,
		440, 442, 444, 446, 448, 450, 452, 454, 456, 458, 460, 460, 462, 464, 466, 468, 470, 472, 474, 476, 478
	};

	static int dru_softcaps[] = {
		254, 256, 258, 260, 262, 264, 264, 266, 268, 270, 272, 272, 274, 276, 278, 280, 282, 282, 284, 286, 288,
		290, 290, 292, 294, 296, 298, 300, 300, 302, 304, 306, 308, 308, 310, 312, 314, 316, 318, 318, 320, 322,
		324, 326, 328, 328, 330, 332, 334, 336, 336, 338, 340, 342, 344, 346, 346, 348, 350, 352, 354, 354, 356,
		358, 360, 362, 364, 364, 366, 368, 370, 372, 372, 374, 376, 378, 380, 382, 382, 384, 386, 388, 390, 390,
		392, 394, 396, 398, 400, 400, 402, 404, 406, 408, 410, 410, 412, 414, 416, 418, 418, 420, 422, 424, 426
	};

	static int rogshmbstber_softcaps[] = {
		264, 266, 268, 270, 272, 272, 274, 276, 278, 280, 282, 282, 284, 286, 288, 290, 292, 294, 294, 296, 298,
		300, 302, 304, 306, 306, 308, 310, 312, 314, 316, 316, 318, 320, 322, 324, 326, 328, 328, 330, 332, 334,
		336, 338, 340, 340, 342, 344, 346, 348, 350, 350, 352, 354, 356, 358, 360, 362, 362, 364, 366, 368, 370,
		372, 374, 374, 376, 378, 380, 382, 384, 384, 386, 388, 390, 392, 394, 396, 396, 398, 400, 402, 404, 406,
		408, 408, 410, 412, 414, 416, 418, 418, 420, 422, 424, 426, 428, 430, 430, 432, 434, 436, 438, 440, 442
	};

	static int necwizmagenc_softcaps[] = {
		248, 250, 252, 254, 256, 256, 258, 260, 262, 264, 264, 266, 268, 270, 272, 272, 274, 276, 278, 280, 280,
		282, 284, 286, 288, 288, 290, 292, 294, 296, 296, 298, 300, 302, 304, 304, 306, 308, 310, 312, 312, 314,
		316, 318, 320, 320, 322, 324, 326, 328, 328, 330, 332, 334, 336, 336, 338, 340, 342, 344, 344, 346, 348,
		350, 352, 352, 354, 356, 358, 360, 360, 362, 364, 366, 368, 368, 370, 372, 374, 376, 376, 378, 380, 382,
		384, 384, 386, 388, 390, 392, 392, 394, 396, 398, 400, 400, 402, 404, 406, 408, 408, 410, 412, 414, 416
	};

	int level = std::min(105, static_cast<int>(GetLevel())) - 1;

	switch (GetClass()) {
	case WARRIOR:
		return war_softcaps[level];
	case CLERIC:
	case BARD:
	case MONK:
		return clrbrdmnk_softcaps[level];
	case PALADIN:
	case SHADOWKNIGHT:
		return palshd_softcaps[level];
	case RANGER:
		return rng_softcaps[level];
	case DRUID:
		return dru_softcaps[level];
	case ROGUE:
	case SHAMAN:
	case BEASTLORD:
	case BERSERKER:
		return rogshmbstber_softcaps[level];
	case NECROMANCER:
	case WIZARD:
	case MAGICIAN:
	case ENCHANTER:
		return necwizmagenc_softcaps[level];
	default:
		return 350;
	}
}

double Mob::GetSoftcapReturns()
{
	// These are based on the dev post, they seem to be correct for every level
	// AKA no more hard caps
	switch (GetClass()) {
	case WARRIOR:
		return 0.35;
	case CLERIC:
	case BARD:
	case MONK:
		return 0.3;
	case PALADIN:
	case SHADOWKNIGHT:
		return 0.33;
	case RANGER:
		return 0.315;
	case DRUID:
		return 0.265;
	case ROGUE:
	case SHAMAN:
	case BEASTLORD:
	case BERSERKER:
		return 0.28;
	case NECROMANCER:
	case WIZARD:
	case MAGICIAN:
	case ENCHANTER:
		return 0.25;
	default:
		return 0.3;
	}
}

int Mob::GetClassRaceACBonus()
{
	int ac_bonus = 0;
	auto level = GetLevel();
	if (GetClass() == MONK) {
		int hardcap = 30;
		int softcap = 14;
		if (level > 99) {
			hardcap = 58;
			softcap = 35;
		}
		else if (level > 94) {
			hardcap = 57;
			softcap = 34;
		}
		else if (level > 89) {
			hardcap = 56;
			softcap = 33;
		}
		else if (level > 84) {
			hardcap = 55;
			softcap = 32;
		}
		else if (level > 79) {
			hardcap = 54;
			softcap = 31;
		}
		else if (level > 74) {
			hardcap = 53;
			softcap = 30;
		}
		else if (level > 69) {
			hardcap = 53;
			softcap = 28;
		}
		else if (level > 64) {
			hardcap = 53;
			softcap = 26;
		}
		else if (level > 63) {
			hardcap = 50;
			softcap = 24;
		}
		else if (level > 61) {
			hardcap = 47;
			softcap = 24;
		}
		else if (level > 59) {
			hardcap = 45;
			softcap = 24;
		}
		else if (level > 54) {
			hardcap = 40;
			softcap = 20;
		}
		else if (level > 50) {
			hardcap = 38;
			softcap = 18;
		}
		else if (level > 44) {
			hardcap = 36;
			softcap = 17;
		}
		else if (level > 29) {
			hardcap = 34;
			softcap = 16;
		}
		else if (level > 14) {
			hardcap = 32;
			softcap = 15;
		}
		int weight = IsClient() ? CastToClient()->CalcCurrentWeight() : 0;
		if (weight < hardcap - 1) {
			int temp = level + 5;
			if (weight > softcap) {
				double redux = (weight - softcap) * 6.66667;
				redux = (100.0 - std::min(100.0, redux)) * 0.01;
				temp = std::max(0, static_cast<int>(temp * redux));
			}
			ac_bonus = (4 * temp) / 3;
		}
		else if (weight > hardcap + 1) {
			int temp = level + 5;
			double multiplier = std::min(1.0, (weight - (hardcap - 10.0)) / 100.0);
			temp = (4 * temp) / 3;
			ac_bonus -= static_cast<int>(temp * multiplier);
		}
	}

	if (GetClass() == ROGUE) {
		int level_scaler = level - 26;
		if (GetAGI() < 80)
			ac_bonus = level_scaler / 4;
		else if (GetAGI() < 85)
			ac_bonus = (level_scaler * 2) / 4;
		else if (GetAGI() < 90)
			ac_bonus = (level_scaler * 3) / 4;
		else if (GetAGI() < 100)
			ac_bonus = (level_scaler * 4) / 4;
		else if (GetAGI() >= 100)
			ac_bonus = (level_scaler * 5) / 4;
		if (ac_bonus > 12)
			ac_bonus = 12;
	}

	if (GetClass() == BEASTLORD) {
		int level_scaler = level - 6;
		if (GetAGI() < 80)
			ac_bonus = level_scaler / 5;
		else if (GetAGI() < 85)
			ac_bonus = (level_scaler * 2) / 5;
		else if (GetAGI() < 90)
			ac_bonus = (level_scaler * 3) / 5;
		else if (GetAGI() < 100)
			ac_bonus = (level_scaler * 4) / 5;
		else if (GetAGI() >= 100)
			ac_bonus = (level_scaler * 5) / 5;
		if (ac_bonus > 16)
			ac_bonus = 16;
	}

	if (GetRace() == IKSAR)
		ac_bonus += EQEmu::Clamp(static_cast<int>(level), 10, 35);

	return ac_bonus;
}

int Mob::ACSum()
{
	int ac = 0; // this should be base AC whenever shrouds come around
	ac += itembonuses.AC; // items + food + tribute
	int shield_ac = 0;
	if (HasShieldEquiped() && IsClient()) {
		auto client = CastToClient();
		auto inst = client->GetInv().GetItem(EQEmu::inventory::slotSecondary);
		if (inst) {
			if (inst->GetItemRecommendedLevel(true) <= GetLevel())
				shield_ac = inst->GetItemArmorClass(true);
			else
				shield_ac = client->CalcRecommendedLevelBonus(GetLevel(), inst->GetItemRecommendedLevel(true), inst->GetItemArmorClass(true));
		}
		shield_ac += client->GetHeroicSTR() / 10;
	}
	// EQ math
	ac = (ac * 4) / 3;
	// anti-twink
	if (IsClient() && GetLevel() < 50)
		ac = std::min(ac, 25 + 6 * GetLevel());
	ac = std::max(0, ac + GetClassRaceACBonus());
	if (IsNPC()) {
		// This is the developer tweaked number
		// for the VAST amount of NPCs in EQ this number didn't exceed 600 until recently (PoWar)
		// According to the guild hall Combat Dummies, a level 50 classic EQ mob it should be ~115
		// For a 60 PoP mob ~120, 70 OoW ~120
		ac += GetAC();
		Mob *owner = nullptr;
		if (IsPet())
			owner = GetOwner();
		else if (CastToNPC()->GetSwarmOwner())
			owner = entity_list.GetMobID(CastToNPC()->GetSwarmOwner());
		if (owner)
			ac += owner->aabonuses.PetAvoidance + owner->spellbonuses.PetAvoidance + owner->itembonuses.PetAvoidance;
		auto spell_aa_ac = aabonuses.AC + spellbonuses.AC;
		ac += GetSkill(EQEmu::skills::SkillDefense) / 5;
		if (EQEmu::ValueWithin(static_cast<int>(GetClass()), NECROMANCER, ENCHANTER))
			ac += spell_aa_ac / 3;
		else
			ac += spell_aa_ac / 4;
	}
	else { // TODO: so we can't set NPC skills ... so the skill bonus ends up being HUGE so lets nerf them a bit
		auto spell_aa_ac = aabonuses.AC + spellbonuses.AC;
		if (EQEmu::ValueWithin(static_cast<int>(GetClass()), NECROMANCER, ENCHANTER))
			ac += GetSkill(EQEmu::skills::SkillDefense) / 2 + spell_aa_ac / 3;
		else
			ac += GetSkill(EQEmu::skills::SkillDefense) / 3 + spell_aa_ac / 4;
	}

	if (GetAGI() > 70)
		ac += GetAGI() / 20;
	if (ac < 0)
		ac = 0;

	if (IsClient()
#ifdef BOTS
		|| IsBot()
#endif
		) {
		auto softcap = GetACSoftcap();
		auto returns = GetSoftcapReturns();
		int total_aclimitmod = aabonuses.CombatStability + itembonuses.CombatStability + spellbonuses.CombatStability;
		if (total_aclimitmod)
			softcap = (softcap * (100 + total_aclimitmod)) / 100;
		softcap += shield_ac;
		if (ac > softcap) {
			auto over_cap = ac - softcap;
			ac = softcap + (over_cap * returns);
		}
		Log(Logs::Detail, Logs::Combat, "ACSum ac %d softcap %d returns %f", ac, softcap, returns);
	}
	else {
		Log(Logs::Detail, Logs::Combat, "ACSum ac %d", ac);
	}
	return ac;
}

int Mob::offense(EQEmu::skills::SkillType skill)
{
	int offense = GetSkill(skill);
	int stat_bonus = 0;
	if (skill == EQEmu::skills::SkillArchery || skill == EQEmu::skills::SkillThrowing)
		stat_bonus = GetDEX();
	else
		stat_bonus = GetSTR();
	if (stat_bonus >= 75)
		offense += (2 * stat_bonus - 150) / 3;
	offense += GetATK();
	return offense;
}

// this assumes "this" is the defender
// this returns between 0.1 to 2.0
double Mob::RollD20(int offense, int mitigation)
{
	static double mods[] = {
		0.1, 0.2, 0.3, 0.4, 0.5,
		0.6, 0.7, 0.8, 0.9, 1.0,
		1.1, 1.2, 1.3, 1.4, 1.5,
		1.6, 1.7, 1.8, 1.9, 2.0
	};

	if (IsClient() && CastToClient()->IsSitting())
		return mods[19];

	auto atk_roll = zone->random.Roll0(offense + 5);
	auto def_roll = zone->random.Roll0(mitigation + 5);

	int avg = (offense + mitigation + 10) / 2;
	int index = std::max(0, (atk_roll - def_roll) + (avg / 2));

	index = EQEmu::Clamp((index * 20) / avg, 0, 19);

	return mods[index];
}

void Mob::MeleeMitigation(Mob *attacker, DamageHitInfo &hit, ExtraAttackOptions *opts)
{
#ifdef LUA_EQEMU
	bool ignoreDefault = false;
	LuaParser::Instance()->MeleeMitigation(this, attacker, hit, opts, ignoreDefault);
	
	if (ignoreDefault) {
		return;
	}
#endif

	if (hit.damage_done < 0 || hit.base_damage == 0)
		return;

	Mob* defender = this;
	auto mitigation = defender->GetMitigationAC();
	if (IsClient() && attacker->IsClient())
		mitigation = mitigation * 80 / 100; // 2004 PvP changes

	if (opts) {
		mitigation *= (1.0f - opts->armor_pen_percent);
		mitigation -= opts->armor_pen_flat;
	}

	auto roll = RollD20(hit.offense, mitigation);

	// +0.5 for rounding, min to 1 dmg
	hit.damage_done = std::max(static_cast<int>(roll * static_cast<double>(hit.base_damage) + 0.5), 1);

	Log(Logs::Detail, Logs::Attack, "mitigation %d vs offense %d. base %d rolled %f damage %d", mitigation, hit.offense, hit.base_damage, roll, hit.damage_done);
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
	if (against->GetInvul() || against->GetSpecialAbility(IMMUNE_MELEE)) {
		return 0;
	}

	//check to see if our weapons or fists are magical.
	if (against->GetSpecialAbility(IMMUNE_MELEE_NONMAGICAL)) {
		if (weapon_item) {
			if (weapon_item->Magic) {
				dmg = weapon_item->Damage;

				//this is more for non weapon items, ex: boots for kick
				//they don't have a dmg but we should be able to hit magical
				dmg = dmg <= 0 ? 1 : dmg;
			}
			else
				return 0;
		}
		else {
			if ((GetClass() == MONK || GetClass() == BEASTLORD) && GetLevel() >= 30) {
				dmg = GetHandToHandDamage();
			}
			else if (GetOwner() && GetLevel() >= RuleI(Combat, PetAttackMagicLevel)) {
				//pets wouldn't actually use this but...
				//it gives us an idea if we can hit due to the dual nature of this function
				dmg = 1;
			}
			else if (GetSpecialAbility(SPECATK_MAGICAL))
			{
				dmg = 1;
			}
			else
				return 0;
		}
	}
	else {
		if (weapon_item) {
			dmg = weapon_item->Damage;

			dmg = dmg <= 0 ? 1 : dmg;
		}
		else {
			dmg = GetHandToHandDamage();
		}
	}

	int eledmg = 0;
	if (!against->GetSpecialAbility(IMMUNE_MAGIC)) {
		if (weapon_item && weapon_item->ElemDmgAmt) {
			//we don't check resist for npcs here
			eledmg = weapon_item->ElemDmgAmt;
			dmg += eledmg;
		}
	}

	if (against->GetSpecialAbility(IMMUNE_MELEE_EXCEPT_BANE)) {
		if (weapon_item) {
			if (weapon_item->BaneDmgBody == against->GetBodyType()) {
				banedmg += weapon_item->BaneDmgAmt;
			}

			if (weapon_item->BaneDmgRace == against->GetRace()) {
				banedmg += weapon_item->BaneDmgRaceAmt;
			}
		}

		if (!banedmg) {
			if (!GetSpecialAbility(SPECATK_BANE))
				return 0;
			else
				return 1;
		}
		else
			dmg += banedmg;
	}
	else {
		if (weapon_item) {
			if (weapon_item->BaneDmgBody == against->GetBodyType()) {
				banedmg += weapon_item->BaneDmgAmt;
			}

			if (weapon_item->BaneDmgRace == against->GetRace()) {
				banedmg += weapon_item->BaneDmgRaceAmt;
			}
		}

		dmg += (banedmg + eledmg);
	}

	if (dmg <= 0) {
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
			}
			else {
				return 0;
			}
		}
		else {
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
			}
			else if (GetOwner() &&
				GetLevel() >=
				RuleI(Combat, PetAttackMagicLevel)) { // pets wouldn't actually use this but...
				dmg = 1; // it gives us an idea if we can hit
			}
			else if (MagicGloves || GetSpecialAbility(SPECATK_MAGICAL)) {
				dmg = 1;
			}
			else
				return 0;
		}
	}
	else {
		if (weapon_item) {
			if (weapon_item->GetItem()) {
				auto rec_level = weapon_item->GetItemRecommendedLevel(true);
				if (IsClient() && GetLevel() < rec_level) {
					dmg = CastToClient()->CalcRecommendedLevelBonus(
						GetLevel(), rec_level, weapon_item->GetItemWeaponDamage(true));
				}
				else {
					dmg = weapon_item->GetItemWeaponDamage(true);
				}

				dmg = dmg <= 0 ? 1 : dmg;
			}
		}
		else {
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
		}
		else {
			dmg += (banedmg + eledmg);
			if (hate)
				*hate += banedmg;
		}
	}
	else {
		dmg += (banedmg + eledmg);
		if (hate)
			*hate += banedmg;
	}

	return std::max(0, dmg);
}

int Client::DoDamageCaps(int base_damage)
{
	// this is based on a client function that caps melee base_damage
	auto level = GetLevel();
	auto stop_level = RuleI(Combat, LevelToStopDamageCaps);
	if (stop_level && stop_level <= level)
		return base_damage;
	int cap = 0;
	if (level >= 125) {
		cap = 7 * level;
	}
	else if (level >= 110) {
		cap = 6 * level;
	}
	else if (level >= 90) {
		cap = 5 * level;
	}
	else if (level >= 70) {
		cap = 4 * level;
	}
	else if (level >= 40) {
		switch (GetClass()) {
		case CLERIC:
		case DRUID:
		case SHAMAN:
			cap = 80;
			break;
		case NECROMANCER:
		case WIZARD:
		case MAGICIAN:
		case ENCHANTER:
			cap = 40;
			break;
		default:
			cap = 200;
			break;
		}
	}
	else if (level >= 30) {
		switch (GetClass()) {
		case CLERIC:
		case DRUID:
		case SHAMAN:
			cap = 26;
			break;
		case NECROMANCER:
		case WIZARD:
		case MAGICIAN:
		case ENCHANTER:
			cap = 18;
			break;
		default:
			cap = 60;
			break;
		}
	}
	else if (level >= 20) {
		switch (GetClass()) {
		case CLERIC:
		case DRUID:
		case SHAMAN:
			cap = 20;
			break;
		case NECROMANCER:
		case WIZARD:
		case MAGICIAN:
		case ENCHANTER:
			cap = 12;
			break;
		default:
			cap = 30;
			break;
		}
	}
	else if (level >= 10) {
		switch (GetClass()) {
		case CLERIC:
		case DRUID:
		case SHAMAN:
			cap = 12;
			break;
		case NECROMANCER:
		case WIZARD:
		case MAGICIAN:
		case ENCHANTER:
			cap = 10;
			break;
		default:
			cap = 14;
			break;
		}
	}
	else {
		switch (GetClass()) {
		case CLERIC:
		case DRUID:
		case SHAMAN:
			cap = 9;
			break;
		case NECROMANCER:
		case WIZARD:
		case MAGICIAN:
		case ENCHANTER:
			cap = 6;
			break;
		default:
			cap = 10; // this is where the 20 damage cap comes from
			break;
		}
	}

	return std::min(cap, base_damage);
}

void Mob::DoAttack(Mob *other, DamageHitInfo &hit, ExtraAttackOptions *opts)
{
	if (!other)
		return;
	Log(Logs::Detail, Logs::Combat, "%s::DoAttack vs %s base %d min %d offense %d tohit %d skill %d", GetName(),
		other->GetName(), hit.base_damage, hit.min_damage, hit.offense, hit.tohit, hit.skill);

	// check to see if we hit..
	if (other->AvoidDamage(this, hit)) {
		int strike_through = itembonuses.StrikeThrough + spellbonuses.StrikeThrough + aabonuses.StrikeThrough;
		if (strike_through && zone->random.Roll(strike_through)) {
			Message_StringID(MT_StrikeThrough,
				STRIKETHROUGH_STRING); // You strike through your opponents defenses!
			hit.damage_done = 1;			// set to one, we will check this to continue
		}
		// I'm pretty sure you can riposte a riposte
		if (hit.damage_done == DMG_RIPOSTED) {
			DoRiposte(other);
			//if (IsDead())
			return;
		}
		Log(Logs::Detail, Logs::Combat, "Avoided/strikethrough damage with code %d", hit.damage_done);
	}

	if (hit.damage_done >= 0) {
		if (other->CheckHitChance(this, hit)) {
			other->MeleeMitigation(this, hit, opts);
			if (hit.damage_done > 0) {
				ApplyDamageTable(hit);
				CommonOutgoingHitSuccess(other, hit, opts);
			}
			Log(Logs::Detail, Logs::Combat, "Final damage after all reductions: %d", hit.damage_done);
		}
		else {
			Log(Logs::Detail, Logs::Combat, "Attack missed. Damage set to 0.");
			hit.damage_done = 0;
		}
	}
}

//note: throughout this method, setting `damage` to a negative is a way to
//stop the attack calculations
// IsFromSpell added to allow spell effects to use Attack. (Mainly for the Rampage AA right now.)
bool Client::Attack(Mob* other, int Hand, bool bRiposte, bool IsStrikethrough, bool IsFromSpell, ExtraAttackOptions *opts)
{
	if (!other) {
		SetTarget(nullptr);
		Log(Logs::General, Logs::Error, "A null Mob object was passed to Client::Attack() for evaluation!");
		return false;
	}

	if (!GetTarget())
		SetTarget(other);

	Log(Logs::Detail, Logs::Combat, "Attacking %s with hand %d %s", other ? other->GetName() : "(nullptr)", Hand, bRiposte ? "(this is a riposte)" : "");

	//SetAttackTimer();
	if (
		(IsCasting() && GetClass() != BARD && !IsFromSpell)
		|| other == nullptr
		|| ((IsClient() && CastToClient()->dead) || (other->IsClient() && other->CastToClient()->dead))
		|| (GetHP() < 0)
		|| (!IsAttackAllowed(other))
		) {
		Log(Logs::Detail, Logs::Combat, "Attack canceled, invalid circumstances.");
		return false; // Only bards can attack while casting
	}

	if (DivineAura() && !GetGM()) {//cant attack while invulnerable unless your a gm
		Log(Logs::Detail, Logs::Combat, "Attack canceled, Divine Aura is in effect.");
		Message_StringID(MT_DefaultText, DIVINE_AURA_NO_ATK);	//You can't attack while invulnerable!
		return false;
	}

	if (GetFeigned())
		return false; // Rogean: How can you attack while feigned? Moved up from Aggro Code.

	EQEmu::ItemInstance* weapon = nullptr;
	if (Hand == EQEmu::inventory::slotSecondary) {	// Kaiyodo - Pick weapon from the attacking hand
		weapon = GetInv().GetItem(EQEmu::inventory::slotSecondary);
		OffHandAtk(true);
	}
	else {
		weapon = GetInv().GetItem(EQEmu::inventory::slotPrimary);
		OffHandAtk(false);
	}

	if (weapon != nullptr) {
		if (!weapon->IsWeapon()) {
			Log(Logs::Detail, Logs::Combat, "Attack canceled, Item %s (%d) is not a weapon.", weapon->GetItem()->Name, weapon->GetID());
			return(false);
		}
		Log(Logs::Detail, Logs::Combat, "Attacking with weapon: %s (%d)", weapon->GetItem()->Name, weapon->GetID());
	}
	else {
		Log(Logs::Detail, Logs::Combat, "Attacking without a weapon.");
	}

	DamageHitInfo my_hit;
	// calculate attack_skill and skillinuse depending on hand and weapon
	// also send Packet to near clients
	my_hit.skill = AttackAnimation(Hand, weapon);
	Log(Logs::Detail, Logs::Combat, "Attacking with %s in slot %d using skill %d", weapon ? weapon->GetItem()->Name : "Fist", Hand, my_hit.skill);

	// Now figure out damage
	my_hit.damage_done = 1;
	my_hit.min_damage = 0;
	uint8 mylevel = GetLevel() ? GetLevel() : 1;
	uint32 hate = 0;
	if (weapon)
		hate = (weapon->GetItem()->Damage + weapon->GetItem()->ElemDmgAmt);

	my_hit.base_damage = GetWeaponDamage(other, weapon, &hate);
	if (hate == 0 && my_hit.base_damage > 1)
		hate = my_hit.base_damage;

	//if weapon damage > 0 then we know we can hit the target with this weapon
	//otherwise we cannot and we set the damage to -5 later on
	if (my_hit.base_damage > 0) {
		// if we revamp this function be more general, we will have to make sure this isn't
		// executed for anything BUT normal melee damage weapons from auto attack
		if (Hand == EQEmu::inventory::slotPrimary || Hand == EQEmu::inventory::slotSecondary)
			my_hit.base_damage = DoDamageCaps(my_hit.base_damage);
		auto shield_inc = spellbonuses.ShieldEquipDmgMod + itembonuses.ShieldEquipDmgMod + aabonuses.ShieldEquipDmgMod;
		if (shield_inc > 0 && HasShieldEquiped() && Hand == EQEmu::inventory::slotPrimary) {
			my_hit.base_damage = my_hit.base_damage * (100 + shield_inc) / 100;
			hate = hate * (100 + shield_inc) / 100;
		}

		CheckIncreaseSkill(my_hit.skill, other, -15);
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

			my_hit.min_damage = ucDamageBonus;
			hate += ucDamageBonus;
		}
#endif
		//Live AA - Sinister Strikes *Adds weapon damage bonus to offhand weapon.
		if (Hand == EQEmu::inventory::slotSecondary) {
			if (aabonuses.SecondaryDmgInc || itembonuses.SecondaryDmgInc || spellbonuses.SecondaryDmgInc) {

				ucDamageBonus = GetWeaponDamageBonus(weapon ? weapon->GetItem() : (const EQEmu::ItemData*) nullptr, true);

				my_hit.min_damage = ucDamageBonus;
				hate += ucDamageBonus;
			}
		}

		// damage = mod_client_damage(damage, skillinuse, Hand, weapon, other);

		Log(Logs::Detail, Logs::Combat, "Damage calculated: base %d min damage %d skill %d", my_hit.base_damage, my_hit.min_damage, my_hit.skill);

		int hit_chance_bonus = 0;
		my_hit.offense = offense(my_hit.skill); // we need this a few times
		my_hit.hand = Hand;

		if (opts) {
			my_hit.base_damage *= opts->damage_percent;
			my_hit.base_damage += opts->damage_flat;
			hate *= opts->hate_percent;
			hate += opts->hate_flat;
			hit_chance_bonus += opts->hit_chance;
		}

		my_hit.tohit = GetTotalToHit(my_hit.skill, hit_chance_bonus);

		DoAttack(other, my_hit, opts);
	}
	else {
		my_hit.damage_done = DMG_INVULNERABLE;
	}

	// Hate Generation is on a per swing basis, regardless of a hit, miss, or block, its always the same.
	// If we are this far, this means we are atleast making a swing.

	other->AddToHateList(this, hate);

	///////////////////////////////////////////////////////////
	////// Send Attack Damage
	///////////////////////////////////////////////////////////
	if (my_hit.damage_done > 0 && aabonuses.SkillAttackProc[0] && aabonuses.SkillAttackProc[1] == my_hit.skill &&
		IsValidSpell(aabonuses.SkillAttackProc[2])) {
		float chance = aabonuses.SkillAttackProc[0] / 1000.0f;
		if (zone->random.Roll(chance))
			SpellFinished(aabonuses.SkillAttackProc[2], other, EQEmu::CastingSlot::Item, 0, -1,
				spells[aabonuses.SkillAttackProc[2]].ResistDiff);
	}
	other->Damage(this, my_hit.damage_done, SPELL_UNKNOWN, my_hit.skill, true, -1, false, m_specialattacks);

	if (IsDead()) return false;

	MeleeLifeTap(my_hit.damage_done);

	if (my_hit.damage_done > 0 && HasSkillProcSuccess() && other && other->GetHP() > 0)
		TrySkillProc(other, my_hit.skill, 0, true, Hand);

	CommonBreakInvisibleFromCombat();

	if (GetTarget())
		TriggerDefensiveProcs(other, Hand, true, my_hit.damage_done);

	if (my_hit.damage_done > 0)
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

void Client::Damage(Mob* other, int32 damage, uint16 spell_id, EQEmu::skills::SkillType attack_skill, bool avoidable, int8 buffslot, bool iBuffTic, eSpecialAttacks special)
{
	if (dead || IsCorpse())
		return;

	if (spell_id == 0)
		spell_id = SPELL_UNKNOWN;

	// cut all PVP spell damage to 2/3
	// Blasting ourselfs is considered PvP
	//Don't do PvP mitigation if the caster is damaging himself
	//should this be applied to all damage? comments sound like some is for spell DMG
	//patch notes on PVP reductions only mention archery/throwing ... not normal dmg
	if (other && other->IsClient() && (other != this) && damage > 0) {
		int PvPMitigation = 100;
		if (attack_skill == EQEmu::skills::SkillArchery || attack_skill == EQEmu::skills::SkillThrowing)
			PvPMitigation = 80;
		else
			PvPMitigation = 67;
		damage = std::max((damage * PvPMitigation) / 100, 1);
	}

	if (!ClientFinishedLoading())
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
	if (!ClientFinishedLoading())
		return false;

	if (dead)
		return false;	//cant die more than once...

	if (!spell)
		spell = SPELL_UNKNOWN;

	char buffer[48] = { 0 };
	snprintf(buffer, 47, "%d %d %d %d", killerMob ? killerMob->GetID() : 0, damage, spell, static_cast<int>(attack_skill));
	if (parse->EventPlayer(EVENT_DEATH, this, buffer, 0) != 0) {
		if (GetHP() < 0) {
			SetHP(0);
		}
		return false;
	}

	if (killerMob && killerMob->IsClient() && (spell != SPELL_UNKNOWN) && damage > 0) {
		char val1[20] = { 0 };

		entity_list.MessageClose_StringID(
			this, /* Sender */
			false, /* Skip Sender */
			RuleI(Range, DamageMessages),
			MT_NonMelee, /* 283 */
			HIT_NON_MELEE, /* %1 hit %2 for %3 points of non-melee damage. */
			killerMob->GetCleanName(), /* Message1 */
			GetCleanName(), /* Message2 */
			ConvertArray(damage, val1)/* Message3 */
			);
	}

	int exploss = 0;
	Log(Logs::Detail, Logs::Combat, "Fatal blow dealt by %s with %d damage, spell %d, skill %d", killerMob ? killerMob->GetName() : "Unknown", damage, spell, attack_skill);

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
	d->corpseid = GetID();
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

	if (GetMerc()) {
		GetMerc()->Suspend();
	}

	if (killerMob != nullptr)
	{
		if (killerMob->IsNPC()) {
			parse->EventNPC(EVENT_SLAY, killerMob->CastToNPC(), this, "", 0);

			mod_client_death_npc(killerMob);

			uint16 emoteid = killerMob->GetEmoteID();
			if (emoteid != 0)
				killerMob->CastToNPC()->DoNPCEmote(KILLEDPC, emoteid);
			killerMob->TrySpellOnKill(killed_level, spell);
		}

		if (killerMob->IsClient() && (IsDueling() || killerMob->CastToClient()->IsDueling())) {
			SetDueling(false);
			SetDuelTarget(0);
			if (killerMob->IsClient() && killerMob->CastToClient()->IsDueling() && killerMob->CastToClient()->GetDuelTarget() == GetID())
			{
				//if duel opponent killed us...
				killerMob->CastToClient()->SetDueling(false);
				killerMob->CastToClient()->SetDuelTarget(0);
				entity_list.DuelMessage(killerMob, this, false);

				mod_client_death_duel(killerMob);

			}
			else {
				//otherwise, we just died, end the duel.
				Mob* who = entity_list.GetMob(GetDuelTarget());
				if (who && who->IsClient()) {
					who->CastToClient()->SetDueling(false);
					who->CastToClient()->SetDuelTarget(0);
				}
			}
		}
	}

	entity_list.RemoveFromTargets(this, true);
	hate_list.RemoveEntFromHateList(this);
	RemoveAutoXTargets();

	//remove ourself from all proximities
	ClearAllProximities();

	/*
	#3: exp loss and corpse generation
	*/

	// figure out if they should lose exp
	if (RuleB(Character, UseDeathExpLossMult)) {
		float GetNum[] = { 0.005f,0.015f,0.025f,0.035f,0.045f,0.055f,0.065f,0.075f,0.085f,0.095f,0.110f };
		int Num = RuleI(Character, DeathExpLossMultiplier);
		if ((Num < 0) || (Num > 10))
			Num = 3;
		float loss = GetNum[Num];
		exploss = (int)((float)GetEXP() * (loss)); //loose % of total XP pending rule (choose 0-10)
	}

	if (!RuleB(Character, UseDeathExpLossMult)) {
		exploss = (int)(GetLevel() * (GetLevel() / 18.0) * 12000);
	}

	if ((GetLevel() < RuleI(Character, DeathExpLossLevel)) || (GetLevel() > RuleI(Character, DeathExpLossMaxLevel)) || IsBecomeNPC())
	{
		exploss = 0;
	}
	else if (killerMob)
	{
		if (killerMob->IsClient())
		{
			exploss = 0;
		}
		else if (killerMob->GetOwner() && killerMob->GetOwner()->IsClient())
		{
			exploss = 0;
		}
	}

	if (spell != SPELL_UNKNOWN)
	{
		uint32 buff_count = GetMaxTotalSlots();
		for (uint16 buffIt = 0; buffIt < buff_count; buffIt++)
		{
			if (buffs[buffIt].spellid == spell && buffs[buffIt].client)
			{
				exploss = 0;	// no exp loss for pvp dot
				break;
			}
		}
	}

	bool LeftCorpse = false;

	// now we apply the exp loss, unmem their spells, and make a corpse
	// unless they're a GM (or less than lvl 10
	if (!GetGM())
	{
		if (exploss > 0) {
			int32 newexp = GetEXP();
			if (exploss > newexp) {
				//lost more than we have... wtf..
				newexp = 1;
			}
			else {
				newexp -= exploss;
			}
			SetEXP(newexp, GetAAXP());
			//m_epp.perAA = 0;	//reset to no AA exp on death.
		}

		//this generates a lot of 'updates' to the client that the client does not need
		BuffFadeNonPersistDeath();
		if (RuleB(Character, UnmemSpellsOnDeath)) {
			if ((ClientVersionBit() & EQEmu::versions::bit_SoFAndLater) && RuleB(Character, RespawnFromHover))
				UnmemSpellAll(true);
			else
				UnmemSpellAll(false);
		}

		if ((RuleB(Character, LeaveCorpses) && GetLevel() >= RuleI(Character, DeathItemLossLevel)) || RuleB(Character, LeaveNakedCorpses))
		{
			// creating the corpse takes the cash/items off the player too
			auto new_corpse = new Corpse(this, exploss);

			std::string tmp;
			database.GetVariable("ServerType", tmp);
			if (tmp[0] == '1' && tmp[1] == '\0' && killerMob != nullptr && killerMob->IsClient()) {
				database.GetVariable("PvPreward", tmp);
				int reward = atoi(tmp.c_str());
				if (reward == 3) {
					database.GetVariable("PvPitem", tmp);
					int pvpitem = atoi(tmp.c_str());
					if (pvpitem>0 && pvpitem<200000)
						new_corpse->SetPlayerKillItemID(pvpitem);
				}
				else if (reward == 2)
					new_corpse->SetPlayerKillItemID(-1);
				else if (reward == 1)
					new_corpse->SetPlayerKillItemID(1);
				else
					new_corpse->SetPlayerKillItemID(0);
				if (killerMob->CastToClient()->isgrouped) {
					Group* group = entity_list.GetGroupByClient(killerMob->CastToClient());
					if (group != 0)
					{
						for (int i = 0; i<6; i++)
						{
							if (group->members[i] != nullptr)
							{
								new_corpse->AllowPlayerLoot(group->members[i], i);
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
	}
	else {
		BuffFadeDetrimental();
	}

	/*
	Finally, send em home

	We change the mob variables, not pp directly, because Save() will copy
	from these and overwrite what we set in pp anyway
	*/

	if (LeftCorpse && (ClientVersionBit() & EQEmu::versions::bit_SoFAndLater) && RuleB(Character, RespawnFromHover))
	{
		ClearDraggedCorpses();
		RespawnFromHoverTimer.Start(RuleI(Character, RespawnFromHoverTimer) * 1000);
		SendRespawnBinds();
	}
	else
	{
		if (isgrouped)
		{
			Group *g = GetGroup();
			if (g)
				g->MemberZoned(this);
		}

		Raid* r = entity_list.GetRaidByClient(this);

		if (r)
			r->MemberZoned(this);

		dead_timer.Start(5000, true);
		m_pp.zone_id = m_pp.binds[0].zoneId;
		m_pp.zoneInstance = m_pp.binds[0].instance_id;
		database.MoveCharacterToZone(this->CharacterID(), database.GetZoneName(m_pp.zone_id));
		Save();
		GoToDeath();
	}

	/* QS: PlayerLogDeaths */
	if (RuleB(QueryServ, PlayerLogDeaths)) {
		const char * killer_name = "";
		if (killerMob && killerMob->GetCleanName()) { killer_name = killerMob->GetCleanName(); }
		std::string event_desc = StringFormat("Died in zoneid:%i instid:%i by '%s', spellid:%i, damage:%i", this->GetZoneID(), this->GetInstanceID(), killer_name, spell, damage);
		QServ->PlayerLogEvent(Player_Log_Deaths, this->CharacterID(), event_desc);
	}

	parse->EventPlayer(EVENT_DEATH_COMPLETE, this, buffer, 0);
	return true;
}

bool NPC::Attack(Mob* other, int Hand, bool bRiposte, bool IsStrikethrough, bool IsFromSpell, ExtraAttackOptions *opts)
{
	if (!other) {
		SetTarget(nullptr);
		Log(Logs::General, Logs::Error, "A null Mob object was passed to NPC::Attack() for evaluation!");
		return false;
	}

	if (DivineAura())
		return(false);

	if (!GetTarget())
		SetTarget(other);

	//Check that we can attack before we calc heading and face our target
	if (!IsAttackAllowed(other)) {
		if (this->GetOwnerID())
			this->Say_StringID(NOT_LEGAL_TARGET);
		if (other) {
			if (other->IsClient())
				other->CastToClient()->RemoveXTarget(this, false);
			RemoveFromHateList(other);
			Log(Logs::Detail, Logs::Combat, "I am not allowed to attack %s", other->GetName());
		}
		return false;
	}

	FaceTarget(GetTarget());

	DamageHitInfo my_hit;
	my_hit.skill = EQEmu::skills::SkillHandtoHand;
	my_hit.hand = Hand;
	my_hit.damage_done = 1;
	if (Hand == EQEmu::inventory::slotPrimary) {
		my_hit.skill = static_cast<EQEmu::skills::SkillType>(GetPrimSkill());
		OffHandAtk(false);
	}
	if (Hand == EQEmu::inventory::slotSecondary) {
		my_hit.skill = static_cast<EQEmu::skills::SkillType>(GetSecSkill());
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
	if (weapon) {
		Log(Logs::Detail, Logs::Combat, "Attacking with weapon: %s (%d) (too bad im not using it for much)", weapon->Name, weapon->ID);

		if (Hand == EQEmu::inventory::slotSecondary && weapon->ItemType == EQEmu::item::ItemTypeShield) {
			Log(Logs::Detail, Logs::Combat, "Attack with shield canceled.");
			return false;
		}

		switch (weapon->ItemType) {
		case EQEmu::item::ItemType1HSlash:
			my_hit.skill = EQEmu::skills::Skill1HSlashing;
			break;
		case EQEmu::item::ItemType2HSlash:
			my_hit.skill = EQEmu::skills::Skill2HSlashing;
			break;
		case EQEmu::item::ItemType1HPiercing:
			my_hit.skill = EQEmu::skills::Skill1HPiercing;
			break;
		case EQEmu::item::ItemType2HPiercing:
			my_hit.skill = EQEmu::skills::Skill2HPiercing;
			break;
		case EQEmu::item::ItemType1HBlunt:
			my_hit.skill = EQEmu::skills::Skill1HBlunt;
			break;
		case EQEmu::item::ItemType2HBlunt:
			my_hit.skill = EQEmu::skills::Skill2HBlunt;
			break;
		case EQEmu::item::ItemTypeBow:
			my_hit.skill = EQEmu::skills::SkillArchery;
			break;
		case EQEmu::item::ItemTypeLargeThrowing:
		case EQEmu::item::ItemTypeSmallThrowing:
			my_hit.skill = EQEmu::skills::SkillThrowing;
			break;
		default:
			my_hit.skill = EQEmu::skills::SkillHandtoHand;
			break;
		}
	}

	int weapon_damage = GetWeaponDamage(other, weapon);

	//do attack animation regardless of whether or not we can hit below
	int16 charges = 0;
	EQEmu::ItemInstance weapon_inst(weapon, charges);
	my_hit.skill = AttackAnimation(Hand, &weapon_inst, my_hit.skill);

	//basically "if not immune" then do the attack
	if (weapon_damage > 0) {

		//ele and bane dmg too
		//NPCs add this differently than PCs
		//if NPCs can't inheriently hit the target we don't add bane/magic dmg which isn't exactly the same as PCs
		int eleBane = 0;
		if (weapon) {
			if (weapon->BaneDmgBody == other->GetBodyType()) {
				eleBane += weapon->BaneDmgAmt;
			}

			if (weapon->BaneDmgRace == other->GetRace()) {
				eleBane += weapon->BaneDmgRaceAmt;
			}

			if (weapon->ElemDmgAmt) {
				eleBane += (weapon->ElemDmgAmt * other->ResistSpell(weapon->ElemDmgType, 0, this) / 100);
			}
		}

		if (!RuleB(NPC, UseItemBonusesForNonPets)) {
			if (!GetOwner()) {
				eleBane = 0;
			}
		}

		uint8 otherlevel = other->GetLevel();
		uint8 mylevel = this->GetLevel();

		otherlevel = otherlevel ? otherlevel : 1;
		mylevel = mylevel ? mylevel : 1;

		//damage = mod_npc_damage(damage, skillinuse, Hand, weapon, other);

		my_hit.base_damage = GetBaseDamage() + eleBane;
		my_hit.min_damage = GetMinDamage();
		int32 hate = my_hit.base_damage + my_hit.min_damage;

		int hit_chance_bonus = 0;

		if (opts) {
			my_hit.base_damage *= opts->damage_percent;
			my_hit.base_damage += opts->damage_flat;
			hate *= opts->hate_percent;
			hate += opts->hate_flat;
			hit_chance_bonus += opts->hit_chance;
		}

		my_hit.offense = offense(my_hit.skill);
		my_hit.tohit = GetTotalToHit(my_hit.skill, hit_chance_bonus);

		DoAttack(other, my_hit, opts);

		other->AddToHateList(this, hate);

		Log(Logs::Detail, Logs::Combat, "Final damage against %s: %d", other->GetName(), my_hit.damage_done);

		if (other->IsClient() && IsPet() && GetOwner()->IsClient()) {
			//pets do half damage to clients in pvp
			my_hit.damage_done /= 2;
			if (my_hit.damage_done < 1)
				my_hit.damage_done = 1;
		}
	}
	else {
		my_hit.damage_done = DMG_INVULNERABLE;
	}

	if (GetHP() > 0 && !other->HasDied()) {
		other->Damage(this, my_hit.damage_done, SPELL_UNKNOWN, my_hit.skill, true, -1, false, m_specialattacks); // Not avoidable client already had thier chance to Avoid
	}
	else
		return false;

	if (HasDied()) //killed by damage shield ect
		return false;

	MeleeLifeTap(my_hit.damage_done);

	CommonBreakInvisibleFromCombat();

	//I doubt this works...
	if (!GetTarget())
		return true; //We killed them

	if (!bRiposte && !other->HasDied()) {
		TryWeaponProc(nullptr, weapon, other, Hand);	//no weapon

		if (!other->HasDied())
			TrySpellProc(nullptr, weapon, other, Hand);

		if (my_hit.damage_done > 0 && HasSkillProcSuccess() && !other->HasDied())
			TrySkillProc(other, my_hit.skill, 0, true, Hand);
	}

	if (GetHP() > 0 && !other->HasDied())
		TriggerDefensiveProcs(other, Hand, true, my_hit.damage_done);

	if (my_hit.damage_done > 0)
		return true;

	else
		return false;
}

void NPC::Damage(Mob* other, int32 damage, uint16 spell_id, EQEmu::skills::SkillType attack_skill, bool avoidable, int8 buffslot, bool iBuffTic, eSpecialAttacks special) {
	if (spell_id == 0)
		spell_id = SPELL_UNKNOWN;

	//handle EVENT_ATTACK. Resets after we have not been attacked for 12 seconds
	if (attacked_timer.Check())
	{
		Log(Logs::Detail, Logs::Combat, "Triggering EVENT_ATTACK due to attack by %s", other ? other->GetName() : "nullptr");
		parse->EventNPC(EVENT_ATTACK, this, other, "", 0);
	}
	attacked_timer.Start(CombatEventTimer_expire);

	if (!IsEngaged())
		zone->AddAggroMob();

	if (GetClass() == LDON_TREASURE)
	{
		if (IsLDoNLocked() && GetLDoNLockedSkill() != LDoNTypeMechanical)
		{
			damage = -5;
		}
		else
		{
			if (IsLDoNTrapped())
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

	if (damage > 0) {
		//see if we are gunna start fleeing
		if (!IsPet()) CheckFlee();
	}
}

bool NPC::Death(Mob* killer_mob, int32 damage, uint16 spell, EQEmu::skills::SkillType attack_skill)
{
	Log(Logs::Detail, Logs::Combat, "Fatal blow dealt by %s with %d damage, spell %d, skill %d",
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

			entity_list.MessageClose_StringID(
				this, /* Sender */
				false, /* Skip Sender */
				RuleI(Range, DamageMessages),
				MT_NonMelee, /* 283 */
				HIT_NON_MELEE, /* %1 hit %2 for %3 points of non-melee damage. */
				killer_mob->GetCleanName(), /* Message1 */
				GetCleanName(), /* Message2 */
				ConvertArray(damage, val1) /* Message3 */
				);
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
		Log(Logs::Detail, Logs::Attack, "%s Mobs currently Aggro %i", __FUNCTION__, zone->MobsAggroCount());
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

		int32 finalxp = give_exp_client->GetExperienceForKill(this);
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
				if (conlevel != CON_GRAY) {
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

void Mob::AddToHateList(Mob* other, uint32 hate /*= 0*/, int32 damage /*= 0*/, bool iYellForHelp /*= true*/, bool bFrenzy /*= false*/, bool iBuffTic /*= false*/, uint16 spell_id, bool pet_command)
{
	if (!other)
		return;

	if (other == this)
		return;

	if (damage < 0) {
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
	bool on_hatelist = CheckAggro(other);

	if (other) {
		AddRampage(other);
		if (on_hatelist) { // odd reason, if you're not on the hate list, subtlety etc don't apply!
						   // Spell Casting Subtlety etc
			int hatemod = 100 + other->spellbonuses.hatemod + other->itembonuses.hatemod + other->aabonuses.hatemod;

			if (hatemod < 1)
				hatemod = 1;
			hate = ((hate * (hatemod)) / 100);
		}
		else {
			hate += 100; // 100 bonus initial aggro
		}
	}

	// Pet that is /pet hold on will not add to their hate list if they're not engaged
	// Pet that is /pet hold on and /pet focus on will not add others to their hate list
	// Pet that is /pet ghold on will never add to their hate list unless /pet attack or /pet qattack

	// we skip these checks if it's forced through a pet command
	if (!pet_command) {
		if (IsPet()) {
			if ((IsGHeld() || (IsHeld() && IsFocused())) && !on_hatelist) // we want them to be able to climb the hate list
				return;
			if ((IsHeld() || IsPetStop() || IsPetRegroup()) && !wasengaged) // not 100% sure on stop/regroup kind of hard to test, but regroup is like "classic hold"
				return;
		}
	}

	if (other->IsNPC() && (other->IsPet() || other->CastToNPC()->GetSwarmOwner() > 0))
		TryTriggerOnValueAmount(false, false, false, true);

	if (IsClient() && !IsAIControlled())
		return;

	if (IsFamiliar() || GetSpecialAbility(IMMUNE_AGGRO))
		return;

	if (spell_id != SPELL_UNKNOWN && NoDetrimentalSpellAggro(spell_id))
		return;

	if (other == myowner)
		return;

	if (other->GetSpecialAbility(IMMUNE_AGGRO_ON))
		return;

	if (GetSpecialAbility(NPC_TUNNELVISION)) {
		int tv_mod = GetSpecialAbilityParam(NPC_TUNNELVISION, 0);

		Mob *top = GetTarget();
		if (top && top != other) {
			if (tv_mod) {
				float tv = tv_mod / 100.0f;
				hate *= tv;
			}
			else {
				hate *= RuleR(Aggro, TunnelVisionAggroMod);
			}
		}
	}

	if (IsNPC() && CastToNPC()->IsUnderwaterOnly() && zone->HasWaterMap()) {
		if (!zone->watermap->InLiquid(glm::vec3(other->GetPosition()))) {
			return;
		}
	}
	// first add self

	// The damage on the hate list is used to award XP to the killer. This check is to prevent Killstealing.
	// e.g. Mob has 5000 hit points, Player A melees it down to 500 hp, Player B executes a headshot (10000 damage).
	// If we add 10000 damage, Player B would get the kill credit, so we only award damage credit to player B of the
	// amount of HP the mob had left.
	//
	if (damage > GetHP())
		damage = GetHP();

	if (spellbonuses.ImprovedTaunt[1] && (GetLevel() < spellbonuses.ImprovedTaunt[0])
		&& other && (buffs[spellbonuses.ImprovedTaunt[2]].casterid != other->GetID()))
		hate = (hate*spellbonuses.ImprovedTaunt[1]) / 100;

	hate_list.AddEntToHateList(other, hate, damage, bFrenzy, !iBuffTic);

	if (other->IsClient() && !on_hatelist)
		other->CastToClient()->AddAutoXTarget(this);

#ifdef BOTS
	// if other is a bot, add the bots client to the hate list
	if (other->IsBot()) {
		if (other->CastToBot()->GetBotOwner() && other->CastToBot()->GetBotOwner()->CastToClient()->GetFeigned()) {
			AddFeignMemory(other->CastToBot()->GetBotOwner()->CastToClient());
		}
		else {
			if (!hate_list.IsEntOnHateList(other->CastToBot()->GetBotOwner())) {
				hate_list.AddEntToHateList(other->CastToBot()->GetBotOwner(), 0, 0, false, true);
				other->CastToBot()->GetBotOwner()->CastToClient()->AddAutoXTarget(this);
			}
		}
	}
#endif //BOTS

	// if other is a merc, add the merc client to the hate list
	if (other->IsMerc()) {
		if (other->CastToMerc()->GetMercOwner() && other->CastToMerc()->GetMercOwner()->CastToClient()->GetFeigned()) {
			AddFeignMemory(other->CastToMerc()->GetMercOwner()->CastToClient());
		}
		else {
			if (!hate_list.IsEntOnHateList(other->CastToMerc()->GetMercOwner()))
				hate_list.AddEntToHateList(other->CastToMerc()->GetMercOwner(), 0, 0, false, true);
		}
	} //MERC

	  // then add pet owner if there's one
	if (owner) { // Other is a pet, add him and it
				 // EverHood 6/12/06
				 // Can't add a feigned owner to hate list
		if (owner->IsClient() && owner->CastToClient()->GetFeigned()) {
			//they avoid hate due to feign death...
		}
		else {
			// cb:2007-08-17
			// owner must get on list, but he's not actually gained any hate yet
			if (!owner->GetSpecialAbility(IMMUNE_AGGRO))
			{
				hate_list.AddEntToHateList(owner, 0, 0, false, !iBuffTic);
				if (owner->IsClient() && !CheckAggro(owner))
					owner->CastToClient()->AddAutoXTarget(this);
			}
		}
	}

	if (mypet && !mypet->IsHeld() && !mypet->IsPetStop()) { // I have a pet, add other to it
		if (!mypet->IsFamiliar() && !mypet->GetSpecialAbility(IMMUNE_AGGRO))
			mypet->hate_list.AddEntToHateList(other, 0, 0, bFrenzy);
	}
	else if (myowner) { // I am a pet, add other to owner if it's NPC/LD
		if (myowner->IsAIControlled() && !myowner->GetSpecialAbility(IMMUNE_AGGRO))
			myowner->hate_list.AddEntToHateList(other, 0, 0, bFrenzy);
	}

	if (other->GetTempPetCount())
		entity_list.AddTempPetsToHateList(other, this, bFrenzy);

	if (!wasengaged) {
		if (IsNPC() && other->IsClient() && other->CastToClient())
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

	if (!attacker || this == attacker)
		return;

	int DS = 0;
	int rev_ds = 0;
	uint16 spellid = 0;

	if (!spell_ds)
	{
		DS = spellbonuses.DamageShield;
		rev_ds = attacker->spellbonuses.ReverseDamageShield;

		if (spellbonuses.DamageShieldSpellID != 0 && spellbonuses.DamageShieldSpellID != SPELL_UNKNOWN)
			spellid = spellbonuses.DamageShieldSpellID;
	}
	else {
		DS = spellbonuses.SpellDamageShield;
		rev_ds = 0;
		// This ID returns "you are burned", seemed most appropriate for spell DS
		spellid = 2166;
	}

	if (DS == 0 && rev_ds == 0)
		return;

	Log(Logs::Detail, Logs::Combat, "Applying Damage Shield of value %d to %s", DS, attacker->GetName());

	//invert DS... spells yield negative values for a true damage shield
	if (DS < 0) {
		if (!spell_ds) {

			DS += aabonuses.DamageShield; //Live AA - coat of thistles. (negative value)
			DS -= itembonuses.DamageShield; //+Damage Shield should only work when you already have a DS spell

											//Spell data for damage shield mitigation shows a negative value for spells for clients and positive
											//value for spells that effect pets. Unclear as to why. For now will convert all positive to be consistent.
			if (attacker->IsOffHandAtk()) {
				int32 mitigation = attacker->itembonuses.DSMitigationOffHand +
					attacker->spellbonuses.DSMitigationOffHand +
					attacker->aabonuses.DSMitigationOffHand;
				DS -= DS*mitigation / 100;
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
	}
	else if (DS > 0 && !spell_ds) {
		//we are healing the attacker...
		attacker->HealDamage(DS);
		//TODO: send a packet???
	}

	//Reverse DS
	//this is basically a DS, but the spell is on the attacker, not the attackee
	//if we've gotten to this point, we know we know "attacker" hit "this" (us) for damage & we aren't invulnerable
	uint16 rev_ds_spell_id = SPELL_UNKNOWN;

	if (spellbonuses.ReverseDamageShieldSpellID != 0 && spellbonuses.ReverseDamageShieldSpellID != SPELL_UNKNOWN)
		rev_ds_spell_id = spellbonuses.ReverseDamageShieldSpellID;

	if (rev_ds < 0) {
		Log(Logs::Detail, Logs::Combat, "Applying Reverse Damage Shield of value %d to %s", rev_ds, attacker->GetName());
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
		}
		else {
			return 1 + ((level - 40) / 3) * (delay / 30); // YOOO shit's useless waste of AAs
		}
	}
	else {
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

	return 0;
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

	static uint8 mnk_dmg[] = { 99,
		4, 4, 4, 4, 5, 5, 5, 5, 5, 6,           // 1-10
		6, 6, 6, 6, 7, 7, 7, 7, 7, 8,           // 11-20
		8, 8, 8, 8, 9, 9, 9, 9, 9, 10,          // 21-30
		10, 10, 10, 10, 11, 11, 11, 11, 11, 12, // 31-40
		12, 12, 12, 12, 13, 13, 13, 13, 13, 14, // 41-50
		14, 14, 14, 14, 14, 14, 14, 14, 14, 14, // 51-60
		14, 14 };                                // 61-62
	static uint8 bst_dmg[] = { 99,
		4, 4, 4, 4, 4, 5, 5, 5, 5, 5,        // 1-10
		5, 6, 6, 6, 6, 6, 6, 7, 7, 7,        // 11-20
		7, 7, 7, 8, 8, 8, 8, 8, 8, 9,        // 21-30
		9, 9, 9, 9, 9, 10, 10, 10, 10, 10,   // 31-40
		10, 11, 11, 11, 11, 11, 11, 12, 12 }; // 41-49
	if (GetClass() == MONK) {
		if (IsClient() && CastToClient()->GetItemIDAt(12) == 10652 && GetLevel() > 50)
			return 9;
		if (level > 62)
			return 15;
		return mnk_dmg[level];
	}
	else if (GetClass() == BEASTLORD) {
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
	static uint8 mnk_hum_delay[] = { 99,
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 1-10
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 11-20
		35, 35, 35, 35, 35, 35, 35, 34, 34, 34, // 21-30
		34, 33, 33, 33, 33, 32, 32, 32, 32, 31, // 31-40
		31, 31, 31, 30, 30, 30, 30, 29, 29, 29, // 41-50
		29, 28, 28, 28, 28, 27, 27, 27, 27, 26, // 51-60
		24, 22 };                                // 61-62
	static uint8 mnk_iks_delay[] = { 99,
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 1-10
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 11-20
		35, 35, 35, 35, 35, 35, 35, 35, 35, 34, // 21-30
		34, 34, 34, 34, 34, 33, 33, 33, 33, 33, // 31-40
		33, 32, 32, 32, 32, 32, 32, 31, 31, 31, // 41-50
		31, 31, 31, 30, 30, 30, 30, 30, 30, 29, // 51-60
		25, 23 };                                // 61-62
	static uint8 bst_delay[] = { 99,
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 1-10
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 11-20
		35, 35, 35, 35, 35, 35, 35, 35, 34, 34, // 21-30
		34, 34, 34, 33, 33, 33, 33, 33, 32, 32, // 31-40
		32, 32, 32, 31, 31, 31, 31, 31, 30, 30, // 41-50
		30, 30, 30, 29, 29, 29, 29, 29, 28, 28, // 51-60
		28, 28, 28, 27, 27, 27, 27, 27, 26, 26, // 61-70
		26, 26, 26 };                            // 71-73

	if (GetClass() == MONK) {
		// Have a look to see if we have epic fists on
		if (IsClient() && CastToClient()->GetItemIDAt(12) == 10652 && GetLevel() > 50)
			return 16;
		int level = GetLevel();
		if (level > 62)
			return GetRace() == IKSAR ? 21 : 20;
		return GetRace() == IKSAR ? mnk_iks_delay[level] : mnk_hum_delay[level];
	}
	else if (GetClass() == BEASTLORD) {
		int level = GetLevel();
		if (level > 73)
			return 25;
		return bst_delay[level];
	}
	return 35;
}

int32 Mob::ReduceDamage(int32 damage)
{
	if (damage <= 0)
		return damage;

	int32 slot = -1;
	bool DisableMeleeRune = false;

	if (spellbonuses.NegateAttacks[0]) {
		slot = spellbonuses.NegateAttacks[1];
		if (slot >= 0) {
			if (--buffs[slot].numhits == 0) {

				if (!TryFadeEffect(slot))
					BuffFadeBySlot(slot, true);
			}

			if (spellbonuses.NegateAttacks[2] && (damage > spellbonuses.NegateAttacks[2]))
				damage -= spellbonuses.NegateAttacks[2];
			else
				return DMG_RUNE;
		}
	}

	//Only mitigate if damage is above the minimium specified.
	if (spellbonuses.MeleeThresholdGuard[0]) {
		slot = spellbonuses.MeleeThresholdGuard[1];

		if (slot >= 0 && (damage > spellbonuses.MeleeThresholdGuard[2]))
		{
			DisableMeleeRune = true;
			int damage_to_reduce = damage * spellbonuses.MeleeThresholdGuard[0] / 100;
			if (damage_to_reduce >= buffs[slot].melee_rune)
			{
				Log(Logs::Detail, Logs::Spells, "Mob::ReduceDamage SE_MeleeThresholdGuard %d damage negated, %d"
					" damage remaining, fading buff.", damage_to_reduce, buffs[slot].melee_rune);
				damage -= buffs[slot].melee_rune;
				if (!TryFadeEffect(slot))
					BuffFadeBySlot(slot);
			}
			else
			{
				Log(Logs::Detail, Logs::Spells, "Mob::ReduceDamage SE_MeleeThresholdGuard %d damage negated, %d"
					" damage remaining.", damage_to_reduce, buffs[slot].melee_rune);
				buffs[slot].melee_rune = (buffs[slot].melee_rune - damage_to_reduce);
				damage -= damage_to_reduce;
			}
		}
	}

	if (spellbonuses.MitigateMeleeRune[0] && !DisableMeleeRune) {
		slot = spellbonuses.MitigateMeleeRune[1];
		if (slot >= 0)
		{
			int damage_to_reduce = damage * spellbonuses.MitigateMeleeRune[0] / 100;

			if (spellbonuses.MitigateMeleeRune[2] && (damage_to_reduce > spellbonuses.MitigateMeleeRune[2]))
				damage_to_reduce = spellbonuses.MitigateMeleeRune[2];

			if (spellbonuses.MitigateMeleeRune[3] && (damage_to_reduce >= buffs[slot].melee_rune))
			{
				Log(Logs::Detail, Logs::Spells, "Mob::ReduceDamage SE_MitigateMeleeDamage %d damage negated, %d"
					" damage remaining, fading buff.", damage_to_reduce, buffs[slot].melee_rune);
				damage -= buffs[slot].melee_rune;
				if (!TryFadeEffect(slot))
					BuffFadeBySlot(slot);
			}
			else
			{
				Log(Logs::Detail, Logs::Spells, "Mob::ReduceDamage SE_MitigateMeleeDamage %d damage negated, %d"
					" damage remaining.", damage_to_reduce, buffs[slot].melee_rune);

				if (spellbonuses.MitigateMeleeRune[3])
					buffs[slot].melee_rune = (buffs[slot].melee_rune - damage_to_reduce);

				damage -= damage_to_reduce;
			}
		}
	}

	if (damage < 1)
		return DMG_RUNE;

	if (spellbonuses.MeleeRune[0] && spellbonuses.MeleeRune[1] >= 0)
		damage = RuneAbsorb(damage, SE_Rune);

	if (damage < 1)
		return DMG_RUNE;

	return(damage);
}

int32 Mob::AffectMagicalDamage(int32 damage, uint16 spell_id, const bool iBuffTic, Mob* attacker)
{
	if (damage <= 0)
		return damage;

	bool DisableSpellRune = false;
	int32 slot = -1;

	// See if we block the spell outright first
	if (!iBuffTic && spellbonuses.NegateAttacks[0]) {
		slot = spellbonuses.NegateAttacks[1];
		if (slot >= 0) {
			if (--buffs[slot].numhits == 0) {

				if (!TryFadeEffect(slot))
					BuffFadeBySlot(slot, true);
			}

			if (spellbonuses.NegateAttacks[2] && (damage > spellbonuses.NegateAttacks[2]))
				damage -= spellbonuses.NegateAttacks[2];
			else
				return 0;
		}
	}

	// If this is a DoT, use DoT Shielding...
	if (iBuffTic) {
		damage -= (damage * itembonuses.DoTShielding / 100);

		if (spellbonuses.MitigateDotRune[0]) {
			slot = spellbonuses.MitigateDotRune[1];
			if (slot >= 0)
			{
				int damage_to_reduce = damage * spellbonuses.MitigateDotRune[0] / 100;

				if (spellbonuses.MitigateDotRune[2] && (damage_to_reduce > spellbonuses.MitigateDotRune[2]))
					damage_to_reduce = spellbonuses.MitigateDotRune[2];

				if (spellbonuses.MitigateDotRune[3] && (damage_to_reduce >= buffs[slot].dot_rune))
				{
					damage -= buffs[slot].dot_rune;
					if (!TryFadeEffect(slot))
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
		if (spellbonuses.SpellThresholdGuard[0]) {
			slot = spellbonuses.SpellThresholdGuard[1];

			if (slot >= 0 && (damage > spellbonuses.MeleeThresholdGuard[2]))
			{
				DisableSpellRune = true;
				int damage_to_reduce = damage * spellbonuses.SpellThresholdGuard[0] / 100;
				if (damage_to_reduce >= buffs[slot].magic_rune)
				{
					damage -= buffs[slot].magic_rune;
					if (!TryFadeEffect(slot))
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
		if (spellbonuses.MitigateSpellRune[0] && !DisableSpellRune) {
			slot = spellbonuses.MitigateSpellRune[1];
			if (slot >= 0)
			{
				int damage_to_reduce = damage * spellbonuses.MitigateSpellRune[0] / 100;

				if (spellbonuses.MitigateSpellRune[2] && (damage_to_reduce > spellbonuses.MitigateSpellRune[2]))
					damage_to_reduce = spellbonuses.MitigateSpellRune[2];

				if (spellbonuses.MitigateSpellRune[3] && (damage_to_reduce >= buffs[slot].magic_rune))
				{
					Log(Logs::Detail, Logs::Spells, "Mob::ReduceDamage SE_MitigateSpellDamage %d damage negated, %d"
						" damage remaining, fading buff.", damage_to_reduce, buffs[slot].magic_rune);
					damage -= buffs[slot].magic_rune;
					if (!TryFadeEffect(slot))
						BuffFadeBySlot(slot);
				}
				else
				{
					Log(Logs::Detail, Logs::Spells, "Mob::ReduceDamage SE_MitigateMeleeDamage %d damage negated, %d"
						" damage remaining.", damage_to_reduce, buffs[slot].magic_rune);

					if (spellbonuses.MitigateSpellRune[3])
						buffs[slot].magic_rune = (buffs[slot].magic_rune - damage_to_reduce);

					damage -= damage_to_reduce;
				}
			}
		}

		if (damage < 1)
			return 0;

		//Regular runes absorb spell damage (except dots) - Confirmed on live.
		if (spellbonuses.MeleeRune[0] && spellbonuses.MeleeRune[1] >= 0)
			damage = RuneAbsorb(damage, SE_Rune);

		if (spellbonuses.AbsorbMagicAtt[0] && spellbonuses.AbsorbMagicAtt[1] >= 0)
			damage = RuneAbsorb(damage, SE_AbsorbMagicAtt);

		if (damage < 1)
			return 0;
	}
	return damage;
}

int32 Mob::ReduceAllDamage(int32 damage)
{
	if (damage <= 0)
		return damage;

	if (spellbonuses.ManaAbsorbPercentDamage[0]) {
		int32 mana_reduced = damage * spellbonuses.ManaAbsorbPercentDamage[0] / 100;
		if (GetMana() >= mana_reduced) {
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

	for (int i = 0; i < MAX_SKILL_PROCS; i++) {
		if (spellbonuses.SkillProc[i] || itembonuses.SkillProc[i] || aabonuses.SkillProc[i])
			return true;
	}
	return false;
}

bool Mob::HasSkillProcSuccess() const
{
	for (int i = 0; i < MAX_SKILL_PROCS; i++) {
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

	int inc = aabonuses.TripleAttackChance + spellbonuses.TripleAttackChance + itembonuses.TripleAttackChance;
	chance = static_cast<int>(chance * (1 + inc / 100.0f));
	chance = (chance * 100) / (chance + 800);

	return zone->random.Int(1, 100) <= chance;
}

bool Client::CheckDoubleRangedAttack() {
	int32 chance = spellbonuses.DoubleRangedAttack + itembonuses.DoubleRangedAttack + aabonuses.DoubleRangedAttack;

	if (chance && zone->random.Roll(chance))
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

void Mob::CommonDamage(Mob* attacker, int &damage, const uint16 spell_id, const EQEmu::skills::SkillType skill_used, bool &avoidable, const int8 buffslot, const bool iBuffTic, eSpecialAttacks special) {
	// This method is called with skill_used=ABJURE for Damage Shield damage.
	bool FromDamageShield = (skill_used == EQEmu::skills::SkillAbjuration);
	bool ignore_invul = false;
	if (IsValidSpell(spell_id))
		ignore_invul = spell_id == 982 || spells[spell_id].cast_not_standing; // cazic touch

	if (!ignore_invul && (GetInvul() || DivineAura())) {
		Log(Logs::Detail, Logs::Combat, "Avoiding %d damage due to invulnerability.", damage);
		damage = DMG_INVULNERABLE;
	}

	if (spell_id != SPELL_UNKNOWN || attacker == nullptr)
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

	if (attacker) {
		if (attacker->IsClient()) {
			if (!RuleB(Combat, EXPFromDmgShield)) {
				// Damage shield damage shouldn't count towards who gets EXP
				if (!attacker->CastToClient()->GetFeigned() && !FromDamageShield)
					AddToHateList(attacker, 0, damage, true, false, iBuffTic, spell_id);
			}
			else {
				if (!attacker->CastToClient()->GetFeigned())
					AddToHateList(attacker, 0, damage, true, false, iBuffTic, spell_id);
			}
		}
		else
			AddToHateList(attacker, 0, damage, true, false, iBuffTic, spell_id);
	}

	if (damage > 0) {
		//if there is some damage being done and theres an attacker involved
		if (attacker) {
			// if spell is lifetap add hp to the caster
			if (spell_id != SPELL_UNKNOWN && IsLifetapSpell(spell_id)) {
				int healed = damage;

				healed = attacker->GetActSpellHealing(spell_id, healed);
				Log(Logs::Detail, Logs::Combat, "Applying lifetap heal of %d to %s", healed, attacker->GetName());
				attacker->HealDamage(healed);

				//we used to do a message to the client, but its gone now.
				// emote goes with every one ... even npcs
				entity_list.MessageClose(this, true, RuleI(Range, SpellMessages), MT_Emote, "%s beams a smile at %s", attacker->GetCleanName(), this->GetCleanName());
			}
		}	//end `if there is some damage being done and theres anattacker person involved`

		Mob *pet = GetPet();
		// pets that have GHold will never automatically add NPCs
		// pets that have Hold and no Focus will add NPCs if they're engaged
		// pets that have Hold and Focus will not add NPCs
		if (pet && !pet->IsFamiliar() && !pet->GetSpecialAbility(IMMUNE_AGGRO) && !pet->IsEngaged() && attacker && attacker != this && !attacker->IsCorpse() && !pet->IsGHeld())
		{
			if (!pet->IsHeld()) {
				Log(Logs::Detail, Logs::Aggro, "Sending pet %s into battle due to attack.", pet->GetName());
				pet->AddToHateList(attacker, 1, 0, true, false, false, spell_id);
				pet->SetTarget(attacker);
				Message_StringID(10, PET_ATTACKING, pet->GetCleanName(), attacker->GetCleanName());
			}
		}

		//see if any runes want to reduce this damage
		if (spell_id == SPELL_UNKNOWN) {
			damage = ReduceDamage(damage);
			Log(Logs::Detail, Logs::Combat, "Melee Damage reduced to %d", damage);
			damage = ReduceAllDamage(damage);
			TryTriggerThreshHold(damage, SE_TriggerMeleeThreshold, attacker);

			if (skill_used)
				CheckNumHitsRemaining(NumHit::IncomingHitSuccess);

		}
		else {
			int32 origdmg = damage;
			damage = AffectMagicalDamage(damage, spell_id, iBuffTic, attacker);
			if (origdmg != damage && attacker && attacker->IsClient()) {
				if (attacker->CastToClient()->GetFilter(FilterDamageShields) != FilterHide)
					attacker->Message(15, "The Spellshield absorbed %d of %d points of damage", origdmg - damage, origdmg);
			}
			if (damage == 0 && attacker && origdmg != damage && IsClient()) {
				//Kayen: Probably need to add a filter for this - Not sure if this msg is correct but there should be a message for spell negate/runes.
				Message(263, "%s tries to cast on YOU, but YOUR magical skin absorbs the spell.", attacker->GetCleanName());
			}
			damage = ReduceAllDamage(damage);
			TryTriggerThreshHold(damage, SE_TriggerSpellThreshold, attacker);
		}

		if (IsClient() && CastToClient()->sneaking) {
			CastToClient()->sneaking = false;
			SendAppearancePacket(AT_Sneak, 0);
		}
		if (attacker && attacker->IsClient() && attacker->CastToClient()->sneaking) {
			attacker->CastToClient()->sneaking = false;
			attacker->SendAppearancePacket(AT_Sneak, 0);
		}

		//final damage has been determined.

		SetHP(GetHP() - damage);


		if (HasDied()) {
			bool IsSaved = false;

			if (TryDivineSave())
				IsSaved = true;

			if (!IsSaved && !TrySpellOnDeath()) {
				SetHP(-500);

				if (Death(attacker, damage, spell_id, skill_used)) {
					return;
				}
			}
		}
		else {
			if (GetHPRatio() < 16)
				TryDeathSave();
		}

		TryTriggerOnValueAmount(true);

		//fade mez if we are mezzed
		if (IsMezzed() && attacker) {
			Log(Logs::Detail, Logs::Combat, "Breaking mez due to attack.");
			entity_list.MessageClose_StringID(
				this, /* Sender */
				true,  /* Skip Sender */
				RuleI(Range, SpellMessages),
				MT_WornOff, /* 284 */
				HAS_BEEN_AWAKENED, // %1 has been awakened by %2.
				GetCleanName(), /* Message1 */
				attacker->GetCleanName() /* Message2 */
				);
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
			}
			else if (skill_used == EQEmu::skills::SkillKick &&
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
					}
					else {
						// stun resist passed!
						if (IsClient())
							Message_StringID(MT_Stun, SHAKE_OFF_STUN);
					}
				}
				else {
					// stun resist 2 passed!
					if (IsClient())
						Message_StringID(MT_Stun, AVOID_STUNNING_BLOW);
				}
			}
			else {
				// main stun failed -- extra interrupt roll
				if (IsCasting() &&
					!EQEmu::ValueWithin(casting_spell_id, 859, 1023)) // these spells are excluded
																	  // 90% chance >< -- stun immune won't reach this branch though :(
					if (zone->random.Int(0, 9) > 1)
						InterruptSpell();
			}
		}

		if (spell_id != SPELL_UNKNOWN && !iBuffTic) {
			//see if root will break
			if (IsRooted() && !FromDamageShield)  // neotoyko: only spells cancel root
				TryRootFadeByDamage(buffslot, attacker);
		}
		else if (spell_id == SPELL_UNKNOWN)
		{
			//increment chances of interrupting
			if (IsCasting()) { //shouldnt interrupt on regular spell damage
				attacked_count++;
				Log(Logs::Detail, Logs::Combat, "Melee attack while casting. Attack count %d", attacked_count);
			}
		}

		//send an HP update if we are hurt
		if (GetHP() < GetMaxHP())
			SendHPUpdate(!iBuffTic); // the OP_Damage actually updates the client in these cases, so we skip the HP update for them
	}	//end `if damage was done`

		//send damage packet...
	if (!iBuffTic) { //buff ticks do not send damage, instead they just call SendHPUpdate(), which is done above
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
		if (special == eSpecialAttacks::AERampage)
			a->special = 1;
		else if (special == eSpecialAttacks::Rampage)
			a->special = 2;
		else
			a->special = 0;
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
					if (position_update_melee_push_timer.Check()) {
						SendPositionUpdate();
					}
				}
			}
			else {
				a->force = 0.0f; // we couldn't move there, so lets not
			}
		}

		//Note: if players can become pets, they will not receive damage messages of their own
		//this was done to simplify the code here (since we can only effectively skip one mob on queue)
		eqFilterType filter;
		Mob *skip = attacker;
		if (attacker && attacker->GetOwnerID()) {
			//attacker is a pet, let pet owners see their pet's damage
			Mob* owner = attacker->GetOwner();
			if (owner && owner->IsClient()) {
				if (((spell_id != SPELL_UNKNOWN) || (FromDamageShield)) && damage>0) {
					//special crap for spell damage, looks hackish to me
					char val1[20] = { 0 };
					owner->Message_StringID(MT_NonMelee, OTHER_HIT_NONMELEE, GetCleanName(), ConvertArray(damage, val1));
				}
				else {
					if (damage > 0) {
						if (spell_id != SPELL_UNKNOWN)
							filter = iBuffTic ? FilterDOT : FilterSpellDamage;
						else
							filter = FilterPetHits;
					}
					else if (damage == -5)
						filter = FilterNone;	//cant filter invulnerable
					else
						filter = FilterPetMisses;

					if (!FromDamageShield)
						owner->CastToClient()->QueuePacket(outapp, true, CLIENT_CONNECTED, filter);
				}
			}
			skip = owner;
		}
		else {
			//attacker is not a pet, send to the attacker

			//if the attacker is a client, try them with the correct filter
			if (attacker && attacker->IsClient()) {
				if ((spell_id != SPELL_UNKNOWN || FromDamageShield) && damage > 0) {
					//special crap for spell damage, looks hackish to me
					char val1[20] = { 0 };
					if (FromDamageShield) {
						if (attacker->CastToClient()->GetFilter(FilterDamageShields) != FilterHide)
							attacker->Message_StringID(MT_DS, OTHER_HIT_NONMELEE, GetCleanName(), ConvertArray(damage, val1));
					}
					else {
						entity_list.MessageClose_StringID(
							this, /* Sender */
							true, /* Skip Sender */
							RuleI(Range, SpellMessages),
							MT_NonMelee, /* 283 */
							HIT_NON_MELEE, /* %1 hit %2 for %3 points of non-melee damage. */
							attacker->GetCleanName(), /* Message1 */
							GetCleanName(), /* Message2 */
							ConvertArray(damage, val1) /* Message3 */
							);
					}
				}
				else {
					if (damage > 0) {
						if (spell_id != SPELL_UNKNOWN)
							filter = iBuffTic ? FilterDOT : FilterSpellDamage;
						else
							filter = FilterNone;	//cant filter our own hits
					}
					else if (damage == -5)
						filter = FilterNone;	//cant filter invulnerable
					else
						filter = FilterMyMisses;

					attacker->CastToClient()->QueuePacket(outapp, true, CLIENT_CONNECTED, filter);
				}
			}
			skip = attacker;
		}

		//send damage to all clients around except the specified skip mob (attacker or the attacker's owner) and ourself
		if (damage > 0) {
			if (spell_id != SPELL_UNKNOWN)
				filter = iBuffTic ? FilterDOT : FilterSpellDamage;
			else
				filter = FilterOthersHit;
		}
		else if (damage == -5)
			filter = FilterNone;	//cant filter invulnerable
		else
			filter = FilterOthersMiss;
		//make attacker (the attacker) send the packet so we can skip them and the owner
		//this call will send the packet to `this` as well (using the wrong filter) (will not happen until PC charm works)
		// If this is Damage Shield damage, the correct OP_Damage packets will be sent from Mob::DamageShield, so
		// we don't send them here.
		if (!FromDamageShield) {

			entity_list.QueueCloseClients(
				this, /* Sender */
				outapp, /* packet */
				true, /* Skip Sender */
				RuleI(Range, SpellMessages),
				skip, /* Skip this mob */
				true, /* Packet ACK */
				filter /* eqFilterType filter */
				);

			//send the damage to ourself if we are a client
			if (IsClient()) {
				//I dont think any filters apply to damage affecting us
				CastToClient()->QueuePacket(outapp);
			}
		}

		safe_delete(outapp);
	}
	else {
		//else, it is a buff tic...
		// So we can see our dot dmg like live shows it.
		if (spell_id != SPELL_UNKNOWN && damage > 0 && attacker && attacker != this && attacker->IsClient()) {
			//might filter on (attack_skill>200 && attack_skill<250), but I dont think we need it
			attacker->FilteredMessage_StringID(attacker, MT_DoTDamage, FilterDOT,
				YOUR_HIT_DOT, GetCleanName(), itoa(damage), spells[spell_id].name);

			/* older clients don't have the below String ID, but it will be filtered */
			entity_list.FilteredMessageClose_StringID(
				attacker, /* Sender */
				true, /* Skip Sender */
				RuleI(Range, SpellMessages),
				MT_DoTDamage, /* Type: 325 */
				FilterDOT, /* FilterType: 19 */
				OTHER_HIT_DOT,  /* MessageFormat: %1 has taken %2 damage from %3 by %4. */
				GetCleanName(), /* Message1 */
				itoa(damage), /* Message2 */
				attacker->GetCleanName(), /* Message3 */
				spells[spell_id].name /* Message4 */
				);
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
				}
				else if (caster->IsClient() && caster != this) {
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
			}
			else { // normal heals
				FilteredMessage_StringID(caster, MT_NonMelee, FilterSpellDamage,
					YOU_HEALED, caster->GetCleanName(), itoa(acthealed));
				if (caster != this)
					caster->FilteredMessage_StringID(caster, MT_NonMelee, FilterSpellDamage,
						YOU_HEAL, GetCleanName(), itoa(acthealed));
			}
		}
		else {
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
	}
	else {
		ProcChance = RuleR(Combat, BaseProcChance) +
			static_cast<float>(mydex) / RuleR(Combat, ProcDexDivideBy);
		ProcChance += ProcChance * ProcBonus / 100.0f;
	}

	Log(Logs::Detail, Logs::Combat, "Proc chance %.2f (%.2f from bonuses)", ProcChance, ProcBonus);
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

	Log(Logs::Detail, Logs::Combat, "Defensive Proc chance %.2f (%.2f from bonuses)", ProcChance, ProcBonus);
	return ProcChance;
}

// argument 'weapon' not used
void Mob::TryDefensiveProc(Mob *on, uint16 hand) {

	if (!on) {
		SetTarget(nullptr);
		Log(Logs::General, Logs::Error, "A null Mob object was passed to Mob::TryDefensiveProc for evaluation!");
		return;
	}

	if (!HasDefensiveProcs())
		return;

	if (!on->HasDied() && on->GetHP() > 0) {

		float ProcChance, ProcBonus;
		on->GetDefensiveProcChances(ProcBonus, ProcChance, hand, this);

		if (hand != EQEmu::inventory::slotPrimary)
			ProcChance /= 2;

		int level_penalty = 0;
		int level_diff = GetLevel() - on->GetLevel();
		if (level_diff > 6)//10% penalty per level if > 6 levels over target.
			level_penalty = (level_diff - 6) * 10;

		ProcChance -= ProcChance*level_penalty / 100;

		if (ProcChance < 0)
			return;

		for (int i = 0; i < MAX_PROCS; i++) {
			if (IsValidSpell(DefensiveProcs[i].spellID)) {
				float chance = ProcChance * (static_cast<float>(DefensiveProcs[i].chance) / 100.0f);
				if (zone->random.Roll(chance)) {
					ExecWeaponProc(nullptr, DefensiveProcs[i].spellID, on);
					CheckNumHitsRemaining(NumHit::DefensiveSpellProcs, 0, DefensiveProcs[i].base_spellID);
				}
			}
		}
	}
}

void Mob::TryWeaponProc(const EQEmu::ItemInstance* weapon_g, Mob *on, uint16 hand) {
	if (!on) {
		SetTarget(nullptr);
		Log(Logs::General, Logs::Error, "A null Mob object was passed to Mob::TryWeaponProc for evaluation!");
		return;
	}

	if (!IsAttackAllowed(on)) {
		Log(Logs::Detail, Logs::Combat, "Preventing procing off of unattackable things.");
		return;
	}

	if (DivineAura()) {
		Log(Logs::Detail, Logs::Combat, "Procs canceled, Divine Aura is in effect.");
		return;
	}

	if (!weapon_g) {
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
				Log(Logs::Detail, Logs::Combat,
					"Tried to proc (%s), but our level (%d) is lower than required (%d)",
					weapon->Name, ourlevel, weapon->Proc.Level);
				if (IsPet()) {
					Mob *own = GetOwner();
					if (own)
						own->Message_StringID(13, PROC_PETTOOLOW);
				}
				else {
					Message_StringID(13, PROC_TOOLOW);
				}
			}
			else {
				Log(Logs::Detail, Logs::Combat,
					"Attacking weapon (%s) successfully procing spell %d (%.2f percent chance)",
					weapon->Name, weapon->Proc.Effect, WPC * 100);
				ExecWeaponProc(inst, weapon->Proc.Effect, on);
				proced = true;
			}
		}
	}
	//If OneProcPerWeapon is not enabled, we reset the try for that weapon regardless of if we procced or not.
	//This is for some servers that may want to have as many procs triggering from weapons as possible in a single round.
	if (!RuleB(Combat, OneProcPerWeapon))
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
						}
						else {
							Message_StringID(13, PROC_TOOLOW);
						}
					}
					else {
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
					Log(Logs::Detail, Logs::Combat,
						"Permanent proc %d procing spell %d (%d percent chance)",
						i, PermaProcs[i].spellID, PermaProcs[i].chance);
					ExecWeaponProc(nullptr, PermaProcs[i].spellID, on);
				}
				else {
					Log(Logs::Detail, Logs::Combat,
						"Permanent proc %d failed to proc %d (%d percent chance)",
						i, PermaProcs[i].spellID, PermaProcs[i].chance);
				}
			}

			// Spell procs (buffs)
			if (SpellProcs[i].spellID != SPELL_UNKNOWN) {
				float chance = ProcChance * (static_cast<float>(SpellProcs[i].chance) / 100.0f);
				if (zone->random.Roll(chance)) {
					Log(Logs::Detail, Logs::Combat,
							"Spell proc %d procing spell %d (%.2f percent chance)",
							i, SpellProcs[i].spellID, chance);
					SendBeginCast(SpellProcs[i].spellID, 0);
					ExecWeaponProc(nullptr, SpellProcs[i].spellID, on, SpellProcs[i].level_override);
					CheckNumHitsRemaining(NumHit::OffensiveSpellProcs, 0,
						SpellProcs[i].base_spellID);
				}
				else {
					Log(Logs::Detail, Logs::Combat,
						"Spell proc %d failed to proc %d (%.2f percent chance)",
						i, SpellProcs[i].spellID, chance);
				}
			}
		}
		else if (rangedattk) { // ranged only
							   // ranged spell procs (buffs)
			if (RangedProcs[i].spellID != SPELL_UNKNOWN) {
				float chance = ProcChance * (static_cast<float>(RangedProcs[i].chance) / 100.0f);
				if (zone->random.Roll(chance)) {
					Log(Logs::Detail, Logs::Combat,
						"Ranged proc %d procing spell %d (%.2f percent chance)",
						i, RangedProcs[i].spellID, chance);
					ExecWeaponProc(nullptr, RangedProcs[i].spellID, on);
					CheckNumHitsRemaining(NumHit::OffensiveSpellProcs, 0,
						RangedProcs[i].base_spellID);
				}
				else {
					Log(Logs::Detail, Logs::Combat,
						"Ranged proc %d failed to proc %d (%.2f percent chance)",
						i, RangedProcs[i].spellID, chance);
				}
			}
		}
	}

	if (HasSkillProcs() && hand != EQEmu::inventory::slotRange) { //We check ranged skill procs within the attack functions.
		uint16 skillinuse = 28;
		if (weapon)
			skillinuse = GetSkillByItemType(weapon->ItemType);

		TrySkillProc(on, skillinuse, 0, false, hand);
	}

	return;
}

void Mob::TryPetCriticalHit(Mob *defender, DamageHitInfo &hit)
{
	if (hit.damage_done < 1)
		return;

	// Allows pets to perform critical hits.
	// Each rank adds an additional 1% chance for any melee hit (primary, secondary, kick, bash, etc) to critical,
	// dealing up to 63% more damage. http://www.magecompendium.com/aa-short-library.html
	// appears to be 70% damage, unsure if changed or just bad info before

	Mob *owner = nullptr;
	int critChance = 0;
	critChance += RuleI(Combat, PetBaseCritChance); // 0 by default
	int critMod = 170;

	if (IsPet())
		owner = GetOwner();
	else if ((IsNPC() && CastToNPC()->GetSwarmOwner()))
		owner = entity_list.GetMobID(CastToNPC()->GetSwarmOwner());
	else
		return;

	if (!owner)
		return;

	int CritPetChance =
		owner->aabonuses.PetCriticalHit + owner->itembonuses.PetCriticalHit + owner->spellbonuses.PetCriticalHit;

	if (CritPetChance || critChance)
		// For pets use PetCriticalHit for base chance, pets do not innately critical with without it
		critChance += CritPetChance;

	if (critChance > 0) {
		if (zone->random.Roll(critChance)) {
			critMod += GetCritDmgMod(hit.skill);
			hit.damage_done += 5;
			hit.damage_done = (hit.damage_done * critMod) / 100;

			entity_list.FilteredMessageClose_StringID(
				this, /* Sender */
				false,  /* Skip Sender */
				RuleI(Range, CriticalDamage),
				MT_CritMelee, /* Type: 301 */
				FilterMeleeCrits, /* FilterType: 12 */
				CRITICAL_HIT, /* MessageFormat: %1 scores a critical hit! (%2) */
				GetCleanName(), /* Message1 */
				itoa(hit.damage_done + hit.min_damage) /* Message2 */
				);

		}
	}
}

void Mob::TryCriticalHit(Mob *defender, DamageHitInfo &hit, ExtraAttackOptions *opts)
{
#ifdef LUA_EQEMU
	bool ignoreDefault = false;
	LuaParser::Instance()->TryCriticalHit(this, defender, hit, opts, ignoreDefault);

	if (ignoreDefault) {
		return;
	}
#endif

	if (hit.damage_done < 1 || !defender)
		return;

	// decided to branch this into it's own function since it's going to be duplicating a lot of the
	// code in here, but could lead to some confusion otherwise
	if ((IsPet() && GetOwner()->IsClient()) || (IsNPC() && CastToNPC()->GetSwarmOwner())) {
		TryPetCriticalHit(defender, hit);
		return;
	}

#ifdef BOTS
	if (this->IsPet() && this->GetOwner() && this->GetOwner()->IsBot()) {
		this->TryPetCriticalHit(defender, hit);
		return;
	}
#endif // BOTS

	if (IsNPC() && !RuleB(Combat, NPCCanCrit))
		return;

	// 1: Try Slay Undead
	if (defender->GetBodyType() == BT_Undead || defender->GetBodyType() == BT_SummonedUndead ||
		defender->GetBodyType() == BT_Vampire) {
		int SlayRateBonus = aabonuses.SlayUndead[0] + itembonuses.SlayUndead[0] + spellbonuses.SlayUndead[0];
		if (SlayRateBonus) {
			float slayChance = static_cast<float>(SlayRateBonus) / 10000.0f;
			if (zone->random.Roll(slayChance)) {
				int SlayDmgBonus = std::max(
				{ aabonuses.SlayUndead[1], itembonuses.SlayUndead[1], spellbonuses.SlayUndead[1] });
				hit.damage_done = std::max(hit.damage_done, hit.base_damage) + 5;
				hit.damage_done = (hit.damage_done * SlayDmgBonus) / 100;

				/* Female */
				if (GetGender() == 1) {
					entity_list.FilteredMessageClose_StringID(
						this, /* Sender */
						false, /* Skip Sender */
						RuleI(Range, CriticalDamage),
						MT_CritMelee, /* Type: 301 */
						FilterMeleeCrits, /* FilterType: 12 */
						FEMALE_SLAYUNDEAD, /* MessageFormat: %1's holy blade cleanses her target!(%2) */
						GetCleanName(), /* Message1 */
						itoa(hit.damage_done + hit.min_damage) /* Message2 */
						);
				}
				/* Males and Neuter */
				else {
					entity_list.FilteredMessageClose_StringID(
						this, /* Sender */
						false, /* Skip Sender */
						RuleI(Range, CriticalDamage),
						MT_CritMelee, /* Type: 301 */
						FilterMeleeCrits, /* FilterType: 12 */
						MALE_SLAYUNDEAD, /* MessageFormat: %1's holy blade cleanses his target!(%2)  */
						GetCleanName(), /* Message1 */
						itoa(hit.damage_done + hit.min_damage) /* Message2 */
						);
				}
				return;
			}
		}
	}

	// 2: Try Melee Critical
	// a lot of good info: http://giline.versus.jp/shiden/damage_e.htm, http://giline.versus.jp/shiden/su.htm

	// We either require an innate crit chance or some SPA 169 to crit
	bool innate_crit = false;
	int crit_chance = GetCriticalChanceBonus(hit.skill);
	if ((GetClass() == WARRIOR || GetClass() == BERSERKER) && GetLevel() >= 12)
		innate_crit = true;
	else if (GetClass() == RANGER && GetLevel() >= 12 && hit.skill == EQEmu::skills::SkillArchery)
		innate_crit = true;
	else if (GetClass() == ROGUE && GetLevel() >= 12 && hit.skill == EQEmu::skills::SkillThrowing)
		innate_crit = true;

	// we have a chance to crit!
	if (innate_crit || crit_chance) {
		int difficulty = 0;
		if (hit.skill == EQEmu::skills::SkillArchery)
			difficulty = RuleI(Combat, ArcheryCritDifficulty);
		else if (hit.skill == EQEmu::skills::SkillThrowing)
			difficulty = RuleI(Combat, ThrowingCritDifficulty);
		else
			difficulty = RuleI(Combat, MeleeCritDifficulty);
		int roll = zone->random.Int(1, difficulty);

		int dex_bonus = GetDEX();
		if (dex_bonus > 255)
			dex_bonus = 255 + ((dex_bonus - 255) / 5);
		dex_bonus += 45; // chances did not match live without a small boost

						 // so if we have an innate crit we have a better chance, except for ber throwing
		if (!innate_crit || (GetClass() == BERSERKER && hit.skill == EQEmu::skills::SkillThrowing))
			dex_bonus = dex_bonus * 3 / 5;

		if (crit_chance)
			dex_bonus += dex_bonus * crit_chance / 100;

		// check if we crited
		if (roll < dex_bonus) {
			// step 1: check for finishing blow
			if (TryFinishingBlow(defender, hit.damage_done))
				return;

			// step 2: calculate damage
			hit.damage_done = std::max(hit.damage_done, hit.base_damage) + 5;
			int og_damage = hit.damage_done;
			int crit_mod = 170 + GetCritDmgMod(hit.skill);
			if (crit_mod < 100) {
				crit_mod = 100;
			}

			hit.damage_done = hit.damage_done * crit_mod / 100;
			Log(Logs::Detail, Logs::Combat,
				"Crit success roll %d dex chance %d og dmg %d crit_mod %d new dmg %d", roll, dex_bonus,
				og_damage, crit_mod, hit.damage_done);

			// step 3: check deadly strike
			if (GetClass() == ROGUE && hit.skill == EQEmu::skills::SkillThrowing) {
				if (BehindMob(defender, GetX(), GetY())) {
					int chance = GetLevel() * 12;
					if (zone->random.Int(1, 1000) < chance) {
						// step 3a: check assassinate
						int assdmg = TryAssassinate(defender, hit.skill); // I don't think this is right
						if (assdmg) {
							hit.damage_done = assdmg;
							return;
						}
						hit.damage_done = hit.damage_done * 200 / 100;

						entity_list.FilteredMessageClose_StringID(
							this, /* Sender */
							false, /* Skip Sender */
							RuleI(Range, CriticalDamage),
							MT_CritMelee, /* Type: 301 */
							FilterMeleeCrits, /* FilterType: 12 */
							DEADLY_STRIKE, /* MessageFormat: %1 scores a Deadly Strike!(%2) */
							GetCleanName(), /* Message1 */
							itoa(hit.damage_done + hit.min_damage) /* Message2 */
							);
						return;
					}
				}
			}

			// step 4: check crips
			// this SPA was reused on live ...
			bool berserk = spellbonuses.BerserkSPA || itembonuses.BerserkSPA || aabonuses.BerserkSPA;
			if (!berserk) {
				if (zone->random.Roll(GetCrippBlowChance())) {
					berserk = true;
				} // TODO: Holyforge is suppose to have an innate extra undead chance? 1/5 which matches the SPA crip though ...
			}

			if (IsBerserk() || berserk) {
				hit.damage_done += og_damage * 119 / 100;
				Log(Logs::Detail, Logs::Combat, "Crip damage %d", hit.damage_done);

				entity_list.FilteredMessageClose_StringID(
					this, /* Sender */
					false, /* Skip Sender */
					RuleI(Range, CriticalDamage),
					MT_CritMelee, /* Type: 301 */
					FilterMeleeCrits, /* FilterType: 12 */
					CRIPPLING_BLOW, /* MessageFormat: %1 lands a Crippling Blow!(%2) */
					GetCleanName(), /* Message1 */
					itoa(hit.damage_done + hit.min_damage) /* Message2 */
					);

				// Crippling blows also have a chance to stun
				// Kayen: Crippling Blow would cause a chance to interrupt for npcs < 55, with a
				// staggers message.
				if (defender->GetLevel() <= 55 && !defender->GetSpecialAbility(UNSTUNABLE)) {
					defender->Emote("staggers.");
					defender->Stun(2000);
				}
				return;
			}

			/* Normal Critical hit message */
			entity_list.FilteredMessageClose_StringID(
				this, /* Sender */
				false, /* Skip Sender */
				RuleI(Range, CriticalDamage),
				MT_CritMelee, /* Type: 301 */
				FilterMeleeCrits, /* FilterType: 12 */
				CRITICAL_HIT, /* MessageFormat: %1 scores a critical hit! (%2) */
				GetCleanName(), /* Message1 */
				itoa(hit.damage_done + hit.min_damage) /* Message2 */
				);
		}
	}
}

bool Mob::TryFinishingBlow(Mob *defender, int &damage)
{
	// base2 of FinishingBlowLvl is the HP limit (cur / max) * 1000, 10% is listed as 100
	if (defender && !defender->IsClient() && defender->GetHPRatio() < 10) {

		uint32 FB_Dmg =
			aabonuses.FinishingBlow[1] + spellbonuses.FinishingBlow[1] + itembonuses.FinishingBlow[1];

		uint32 FB_Level = 0;
		FB_Level = aabonuses.FinishingBlowLvl[0];
		if (FB_Level < spellbonuses.FinishingBlowLvl[0])
			FB_Level = spellbonuses.FinishingBlowLvl[0];
		else if (FB_Level < itembonuses.FinishingBlowLvl[0])
			FB_Level = itembonuses.FinishingBlowLvl[0];

		// modern AA description says rank 1 (500) is 50% chance
		int ProcChance =
			aabonuses.FinishingBlow[0] + spellbonuses.FinishingBlow[0] + spellbonuses.FinishingBlow[0];

		if (FB_Level && FB_Dmg && (defender->GetLevel() <= FB_Level) &&
			(ProcChance >= zone->random.Int(1, 1000))) {

			/* Finishing Blow Critical Message */
			entity_list.FilteredMessageClose_StringID(
				this, /* Sender */
				false, /* Skip Sender */
				RuleI(Range, CriticalDamage),
				MT_CritMelee, /* Type: 301 */
				FilterMeleeCrits, /* FilterType: 12 */
				FINISHING_BLOW, /* MessageFormat: %1 scores a Finishing Blow!!) */
				GetCleanName() /* Message1 */
				);

			damage = FB_Dmg;
			return true;
		}
	}
	return false;
}

void Mob::DoRiposte(Mob *defender)
{
	Log(Logs::Detail, Logs::Combat, "Preforming a riposte");

	if (!defender)
		return;

	defender->Attack(this, EQEmu::inventory::slotPrimary, true);
	if (HasDied())
		return;

	// this effect isn't used on live? See no AAs or spells
	int32 DoubleRipChance = defender->aabonuses.DoubleRiposte + defender->spellbonuses.DoubleRiposte +
		defender->itembonuses.DoubleRiposte;

	if (DoubleRipChance && zone->random.Roll(DoubleRipChance)) {
		Log(Logs::Detail, Logs::Combat,
			"Preforming a double riposted from SE_DoubleRiposte (%d percent chance)", DoubleRipChance);
		defender->Attack(this, EQEmu::inventory::slotPrimary, true);
		if (HasDied())
			return;
	}

	DoubleRipChance = defender->aabonuses.GiveDoubleRiposte[0] + defender->spellbonuses.GiveDoubleRiposte[0] +
		defender->itembonuses.GiveDoubleRiposte[0];

	// Live AA - Double Riposte
	if (DoubleRipChance && zone->random.Roll(DoubleRipChance)) {
		Log(Logs::Detail, Logs::Combat,
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
		Log(Logs::Detail, Logs::Combat, "Preforming a return SPECIAL ATTACK (%d percent chance)",
			DoubleRipChance);

		if (defender->GetClass() == MONK)
			defender->MonkSpecialAttack(this, defender->aabonuses.GiveDoubleRiposte[2]);
		else if (defender->IsClient()) // so yeah, even if you don't have the skill you can still do the attack :P (and we don't crash anymore)
			defender->CastToClient()->DoClassAttacks(this, defender->aabonuses.GiveDoubleRiposte[2], true);
	}
}

void Mob::ApplyMeleeDamageMods(uint16 skill, int &damage, Mob *defender, ExtraAttackOptions *opts)
{
	int dmgbonusmod = 0;

	dmgbonusmod += GetMeleeDamageMod_SE(skill);
	if (opts)
		dmgbonusmod += opts->melee_damage_bonus_flat;

	if (defender) {
		if (defender->IsClient() && defender->GetClass() == WARRIOR)
			dmgbonusmod -= 5;
		// 168 defensive
		dmgbonusmod += (defender->spellbonuses.MeleeMitigationEffect + itembonuses.MeleeMitigationEffect + aabonuses.MeleeMitigationEffect);
	}

	damage += damage * dmgbonusmod / 100;
}

bool Mob::HasDied() {
	bool Result = false;
	int32 hp_below = 0;

	hp_below = (GetDelayDeath() * -1);

	if ((GetHP()) <= (hp_below))
		Result = true;

	return Result;
}

const DamageTable &Mob::GetDamageTable() const
{
	static const DamageTable dmg_table[] = {
		{ 210, 49, 105 }, // 1-50
		{ 245, 35,  80 }, // 51
		{ 245, 35,  80 }, // 52
		{ 245, 35,  80 }, // 53
		{ 245, 35,  80 }, // 54
		{ 245, 35,  80 }, // 55
		{ 265, 28,  70 }, // 56
		{ 265, 28,  70 }, // 57
		{ 265, 28,  70 }, // 58
		{ 265, 28,  70 }, // 59
		{ 285, 23,  65 }, // 60
		{ 285, 23,  65 }, // 61
		{ 285, 23,  65 }, // 62
		{ 290, 21,  60 }, // 63
		{ 290, 21,  60 }, // 64
		{ 295, 19,  55 }, // 65
		{ 295, 19,  55 }, // 66
		{ 300, 19,  55 }, // 67
		{ 300, 19,  55 }, // 68
		{ 300, 19,  55 }, // 69
		{ 305, 19,  55 }, // 70
		{ 305, 19,  55 }, // 71
		{ 310, 17,  50 }, // 72
		{ 310, 17,  50 }, // 73
		{ 310, 17,  50 }, // 74
		{ 315, 17,  50 }, // 75
		{ 315, 17,  50 }, // 76
		{ 325, 17,  45 }, // 77
		{ 325, 17,  45 }, // 78
		{ 325, 17,  45 }, // 79
		{ 335, 17,  45 }, // 80
		{ 335, 17,  45 }, // 81
		{ 345, 17,  45 }, // 82
		{ 345, 17,  45 }, // 83
		{ 345, 17,  45 }, // 84
		{ 355, 17,  45 }, // 85
		{ 355, 17,  45 }, // 86
		{ 365, 17,  45 }, // 87
		{ 365, 17,  45 }, // 88
		{ 365, 17,  45 }, // 89
		{ 375, 17,  45 }, // 90
		{ 375, 17,  45 }, // 91
		{ 380, 17,  45 }, // 92
		{ 380, 17,  45 }, // 93
		{ 380, 17,  45 }, // 94
		{ 385, 17,  45 }, // 95
		{ 385, 17,  45 }, // 96
		{ 390, 17,  45 }, // 97
		{ 390, 17,  45 }, // 98
		{ 390, 17,  45 }, // 99
		{ 395, 17,  45 }, // 100
		{ 395, 17,  45 }, // 101
		{ 400, 17,  45 }, // 102
		{ 400, 17,  45 }, // 103
		{ 400, 17,  45 }, // 104
		{ 405, 17,  45 }  // 105
	};

	static const DamageTable mnk_table[] = {
		{ 220, 45, 100 }, // 1-50
		{ 245, 35,  80 }, // 51
		{ 245, 35,  80 }, // 52
		{ 245, 35,  80 }, // 53
		{ 245, 35,  80 }, // 54
		{ 245, 35,  80 }, // 55
		{ 285, 23,  65 }, // 56
		{ 285, 23,  65 }, // 57
		{ 285, 23,  65 }, // 58
		{ 285, 23,  65 }, // 59
		{ 290, 21,  60 }, // 60
		{ 290, 21,  60 }, // 61
		{ 290, 21,  60 }, // 62
		{ 295, 19,  55 }, // 63
		{ 295, 19,  55 }, // 64
		{ 300, 17,  50 }, // 65
		{ 300, 17,  50 }, // 66
		{ 310, 17,  50 }, // 67
		{ 310, 17,  50 }, // 68
		{ 310, 17,  50 }, // 69
		{ 320, 17,  50 }, // 70
		{ 320, 17,  50 }, // 71
		{ 325, 15,  45 }, // 72
		{ 325, 15,  45 }, // 73
		{ 325, 15,  45 }, // 74
		{ 330, 15,  45 }, // 75
		{ 330, 15,  45 }, // 76
		{ 335, 15,  40 }, // 77
		{ 335, 15,  40 }, // 78
		{ 335, 15,  40 }, // 79
		{ 345, 15,  40 }, // 80
		{ 345, 15,  40 }, // 81
		{ 355, 15,  40 }, // 82
		{ 355, 15,  40 }, // 83
		{ 355, 15,  40 }, // 84
		{ 365, 15,  40 }, // 85
		{ 365, 15,  40 }, // 86
		{ 375, 15,  40 }, // 87
		{ 375, 15,  40 }, // 88
		{ 375, 15,  40 }, // 89
		{ 385, 15,  40 }, // 90
		{ 385, 15,  40 }, // 91
		{ 390, 15,  40 }, // 92
		{ 390, 15,  40 }, // 93
		{ 390, 15,  40 }, // 94
		{ 395, 15,  40 }, // 95
		{ 395, 15,  40 }, // 96
		{ 400, 15,  40 }, // 97
		{ 400, 15,  40 }, // 98
		{ 400, 15,  40 }, // 99
		{ 405, 15,  40 }, // 100
		{ 405, 15,  40 }, // 101
		{ 410, 15,  40 }, // 102
		{ 410, 15,  40 }, // 103
		{ 410, 15,  40 }, // 104
		{ 415, 15,  40 }, // 105
	};

	bool monk = GetClass() == MONK;
	bool melee = IsWarriorClass();
	// tables caped at 105 for now -- future proofed for a while at least :P
	int level = std::min(static_cast<int>(GetLevel()), 105);

	if (!melee || (!monk && level < 51))
		return dmg_table[0];

	if (monk && level < 51)
		return mnk_table[0];

	auto &which = monk ? mnk_table : dmg_table;
	return which[level - 50];
}

void Mob::ApplyDamageTable(DamageHitInfo &hit)
{
#ifdef LUA_EQEMU
	bool ignoreDefault = false;
	LuaParser::Instance()->ApplyDamageTable(this, hit, ignoreDefault);
	
	if (ignoreDefault) {
		return;
	}
#endif

	// someone may want to add this to custom servers, can remove this if that's the case
	if (!IsClient()
#ifdef BOTS
		&& !IsBot()
#endif
		)
		return;
	// this was parsed, but we do see the min of 10 and the normal minus factor is 105, so makes sense
	if (hit.offense < 115)
		return;

	// things that come out to 1 dmg seem to skip this (ex non-bash slam classes)
	if (hit.damage_done < 2)
		return;

	auto &damage_table = GetDamageTable();

	if (zone->random.Roll(damage_table.chance))
		return;

	int basebonus = hit.offense - damage_table.minusfactor;
	basebonus = std::max(10, basebonus / 2);
	int extrapercent = zone->random.Roll0(basebonus);
	int percent = std::min(100 + extrapercent, damage_table.max_extra);
	hit.damage_done = (hit.damage_done * percent) / 100;

	if (IsWarriorClass() && GetLevel() > 54)
		hit.damage_done++;
	Log(Logs::Detail, Logs::Attack, "Damage table applied %d (max %d)", percent, damage_table.max_extra);
}

void Mob::TrySkillProc(Mob *on, uint16 skill, uint16 ReuseTime, bool Success, uint16 hand, bool IsDefensive)
{

	if (!on) {
		SetTarget(nullptr);
		Log(Logs::General, Logs::Error, "A null Mob object was passed to Mob::TrySkillProc for evaluation!");
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

	if (spellbonuses.LimitToSkill[skill]) {

		for (int e = 0; e < MAX_SKILL_PROCS; e++) {
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

	if (itembonuses.LimitToSkill[skill]) {
		CanProc = true;
		for (int e = 0; e < MAX_SKILL_PROCS; e++) {
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

	if (IsClient() && aabonuses.LimitToSkill[skill]) {

		CanProc = true;
		uint32 effect_id = 0;
		int32 base1 = 0;
		int32 base2 = 0;
		uint32 slot = 0;

		for (int e = 0; e < MAX_SKILL_PROCS; e++) {
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

				for (auto &rank_info : aa_ranks) {
					auto ability_rank = zone->GetAlternateAdvancementAbilityAndRank(rank_info.first, rank_info.second.first);
					auto ability = ability_rank.first;
					auto rank = ability_rank.second;

					if (!ability) {
						continue;
					}

					for (auto &effect : rank->effects) {
						effect_id = effect.effect_id;
						base1 = effect.base1;
						base2 = effect.base2;
						slot = effect.slot;

						if (effect_id == SE_SkillProc || effect_id == SE_SkillProcSuccess) {
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

	if (IsDetrimentalSpell(spellbonuses.Root[1]) && spellbonuses.Root[1] != buffslot) {
		int BreakChance = RuleI(Spells, RootBreakFromSpells);

		BreakChance -= BreakChance*buffs[spellbonuses.Root[1]].RootBreakChance / 100;
		int level_diff = attacker->GetLevel() - GetLevel();

		//Use baseline if level difference <= 1 (ie. If target is (1) level less than you, or equal or greater level)

		if (level_diff == 2)
			BreakChance = (BreakChance * 80) / 100; //Decrease by 20%;

		else if (level_diff >= 3 && level_diff <= 20)
			BreakChance = (BreakChance * 60) / 100; //Decrease by 40%;

		else if (level_diff > 21)
			BreakChance = (BreakChance * 20) / 100; //Decrease by 80%;

		if (BreakChance < 1)
			BreakChance = 1;

		if (zone->random.Roll(BreakChance)) {

			if (!TryFadeEffect(spellbonuses.Root[1])) {
				BuffFadeBySlot(spellbonuses.Root[1]);
				Log(Logs::Detail, Logs::Combat, "Spell broke root! BreakChance percent chance");
				return true;
			}
		}
	}

	Log(Logs::Detail, Logs::Combat, "Spell did not break root. BreakChance percent chance");
	return false;
}

int32 Mob::RuneAbsorb(int32 damage, uint16 type)
{
	uint32 buff_max = GetMaxTotalSlots();
	if (type == SE_Rune) {
		for (uint32 slot = 0; slot < buff_max; slot++) {
			if (slot == spellbonuses.MeleeRune[1] && spellbonuses.MeleeRune[0] && buffs[slot].melee_rune && IsValidSpell(buffs[slot].spellid)) {
				int melee_rune_left = buffs[slot].melee_rune;

				if (melee_rune_left > damage)
				{
					melee_rune_left -= damage;
					buffs[slot].melee_rune = melee_rune_left;
					return -6;
				}

				else
				{
					if (melee_rune_left > 0)
						damage -= melee_rune_left;

					if (!TryFadeEffect(slot))
						BuffFadeBySlot(slot);
				}
			}
		}
	}

	else {
		for (uint32 slot = 0; slot < buff_max; slot++) {
			if (slot == spellbonuses.AbsorbMagicAtt[1] && spellbonuses.AbsorbMagicAtt[0] && buffs[slot].magic_rune && IsValidSpell(buffs[slot].spellid)) {
				int magic_rune_left = buffs[slot].magic_rune;
				if (magic_rune_left > damage)
				{
					magic_rune_left -= damage;
					buffs[slot].magic_rune = magic_rune_left;
					return 0;
				}

				else
				{
					if (magic_rune_left > 0)
						damage -= magic_rune_left;

					if (!TryFadeEffect(slot))
						BuffFadeBySlot(slot);
				}
			}
		}
	}

	return damage;
}

void Mob::CommonOutgoingHitSuccess(Mob* defender, DamageHitInfo &hit, ExtraAttackOptions *opts)
{
	if (!defender)
		return;

#ifdef LUA_EQEMU
	bool ignoreDefault = false;
	LuaParser::Instance()->CommonOutgoingHitSuccess(this, defender, hit, opts, ignoreDefault);

	if (ignoreDefault) {
		return;
	}
#endif

	// BER weren't parsing the halving
	if (hit.skill == EQEmu::skills::SkillArchery ||
		(hit.skill == EQEmu::skills::SkillThrowing && GetClass() != BERSERKER))
		hit.damage_done /= 2;

	if (hit.damage_done < 1)
		hit.damage_done = 1;

	if (hit.skill == EQEmu::skills::SkillArchery) {
		int bonus = aabonuses.ArcheryDamageModifier + itembonuses.ArcheryDamageModifier + spellbonuses.ArcheryDamageModifier;
		hit.damage_done += hit.damage_done * bonus / 100;
		int headshot = TryHeadShot(defender, hit.skill);
		if (headshot > 0) {
			hit.damage_done = headshot;
		}
		else if (GetClass() == RANGER && GetLevel() > 50) { // no double dmg on headshot
			if (defender->IsNPC() && !defender->IsMoving() && !defender->IsRooted()) {
				hit.damage_done *= 2;
				Message_StringID(MT_CritMelee, BOW_DOUBLE_DAMAGE);
			}
		}
	}

	int extra_mincap = 0;
	int min_mod = hit.base_damage * GetMeleeMinDamageMod_SE(hit.skill) / 100;
	if (hit.skill == EQEmu::skills::SkillBackstab) {
		extra_mincap = GetLevel() < 7 ? 7 : GetLevel();
		if (GetLevel() >= 60)
			extra_mincap = GetLevel() * 2;
		else if (GetLevel() > 50)
			extra_mincap = GetLevel() * 3 / 2;
		if (IsSpecialAttack(eSpecialAttacks::ChaoticStab)) {
			hit.damage_done = extra_mincap;
		}
		else {
			int ass = TryAssassinate(defender, hit.skill);
			if (ass > 0)
				hit.damage_done = ass;
		}
	}
	else if (hit.skill == EQEmu::skills::SkillFrenzy && GetClass() == BERSERKER && GetLevel() > 50) {
		extra_mincap = 4 * GetLevel() / 5;
	}

	// this has some weird ordering
	// Seems the crit message is generated before some of them :P

	// worn item +skill dmg, SPA 220, 418. Live has a normalized version that should be here too
	hit.min_damage += GetSkillDmgAmt(hit.skill);

	// shielding mod2
	if (defender->itembonuses.MeleeMitigation)
		hit.min_damage -= hit.min_damage * defender->itembonuses.MeleeMitigation / 100;

	ApplyMeleeDamageMods(hit.skill, hit.damage_done, defender, opts);
	min_mod = std::max(min_mod, extra_mincap);
	if (min_mod && hit.damage_done < min_mod) // SPA 186
		hit.damage_done = min_mod;

	TryCriticalHit(defender, hit, opts);

	hit.damage_done += hit.min_damage;
	if (IsClient()) {
		int extra = 0;
		switch (hit.skill) {
		case EQEmu::skills::SkillThrowing:
		case EQEmu::skills::SkillArchery:
			extra = CastToClient()->GetHeroicDEX() / 10;
			break;
		default:
			extra = CastToClient()->GetHeroicSTR() / 10;
			break;
		}
		hit.damage_done += extra;
	}

	// this appears where they do special attack dmg mods
	int spec_mod = 0;
	if (IsSpecialAttack(eSpecialAttacks::Rampage)) {
		int mod = GetSpecialAbilityParam(SPECATK_RAMPAGE, 2);
		if (mod > 0)
			spec_mod = mod;
		if ((IsPet() || IsTempPet()) && IsPetOwnerClient()) {
			int spell = spellbonuses.PC_Pet_Rampage[1] + itembonuses.PC_Pet_Rampage[1] + aabonuses.PC_Pet_Rampage[1];
			if (spell > spec_mod)
				spec_mod = spell;
		}
	}
	else if (IsSpecialAttack(eSpecialAttacks::AERampage)) {
		int mod = GetSpecialAbilityParam(SPECATK_AREA_RAMPAGE, 2);
		if (mod > 0)
			spec_mod = mod;
	}
	if (spec_mod > 0)
		hit.damage_done = (hit.damage_done * spec_mod) / 100;

	hit.damage_done += (hit.damage_done * defender->GetSkillDmgTaken(hit.skill, opts) / 100) + (defender->GetFcDamageAmtIncoming(this, 0, true, hit.skill));

	CheckNumHitsRemaining(NumHit::OutgoingHitSuccess);
}

void Mob::CommonBreakInvisibleFromCombat()
{
	//break invis when you attack
	if (invisible) {
		Log(Logs::Detail, Logs::Combat, "Removing invisibility due to melee attack.");
		BuffFadeByEffect(SE_Invisibility);
		BuffFadeByEffect(SE_Invisibility2);
		invisible = false;
	}
	if (invisible_undead) {
		Log(Logs::Detail, Logs::Combat, "Removing invisibility vs. undead due to melee attack.");
		BuffFadeByEffect(SE_InvisVsUndead);
		BuffFadeByEffect(SE_InvisVsUndead2);
		invisible_undead = false;
	}
	if (invisible_animals) {
		Log(Logs::Detail, Logs::Combat, "Removing invisibility vs. animals due to melee attack.");
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
		}
		else {
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
		speed = static_cast<int>((attack_delay / haste_mod) + ((hhe / 1000.0f) * (attack_delay / haste_mod)));
	else
		speed = static_cast<int>((attack_delay / haste_mod) + ((hhe / 100.0f) * attack_delay));

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
			if (!CanThisClassDualWield() || (HasTwoHanderEquipped() && !GetSpecialAbility(SPECATK_QUAD))) {
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
				if (CheckTripleAttack()) {
					Attack(target, hand, false, false, IsFromSpell);
					auto flurrychance = aabonuses.FlurryChance + spellbonuses.FlurryChance +
						itembonuses.FlurryChance;
					if (flurrychance && zone->random.Roll(flurrychance)) {
						Attack(target, hand, false, false, IsFromSpell);
						if (zone->random.Roll(flurrychance))
							Attack(target, hand, false, false, IsFromSpell);
						Message_StringID(MT_NPCFlurry, YOU_FLURRY);
					}
				}
			}
		}
	}

	if (hand == EQEmu::inventory::slotPrimary) {
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

void Mob::DoMainHandAttackRounds(Mob *target, ExtraAttackOptions *opts)
{
	if (!target)
		return;

	if (RuleB(Combat, UseLiveCombatRounds)) {
		// A "quad" on live really is just a successful dual wield where both double attack
		// The mobs that could triple lost the ability to when the triple attack skill was added in
		Attack(target, EQEmu::inventory::slotPrimary, false, false, false, opts);
		if (CanThisClassDoubleAttack() && CheckDoubleAttack()) {
			Attack(target, EQEmu::inventory::slotPrimary, false, false, false, opts);
			if ((IsPet() || IsTempPet()) && IsPetOwnerClient()) {
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
			Attack(target, EQEmu::inventory::slotPrimary, false, false, false, opts);
		}
		else {
			for (int i = 0; i < n_atk; ++i) {
				Attack(target, EQEmu::inventory::slotPrimary, false, false, false, opts);
			}
		}
	}
	else {
		Attack(target, EQEmu::inventory::slotPrimary, false, false, false, opts);
	}

	// we use this random value in three comparisons with different
	// thresholds, and if its truely random, then this should work
	// out reasonably and will save us compute resources.
	int32 RandRoll = zone->random.Int(0, 99);
	if ((CanThisClassDoubleAttack() || GetSpecialAbility(SPECATK_TRIPLE) || GetSpecialAbility(SPECATK_QUAD))
		// check double attack, this is NOT the same rules that clients use...
		&&
		RandRoll < (GetLevel() + NPCDualAttackModifier)) {
		Attack(target, EQEmu::inventory::slotPrimary, false, false, false, opts);
		// lets see if we can do a triple attack with the main hand
		// pets are excluded from triple and quads...
		if ((GetSpecialAbility(SPECATK_TRIPLE) || GetSpecialAbility(SPECATK_QUAD)) && !IsPet() &&
			RandRoll < (GetLevel() + NPCTripleAttackModifier)) {
			Attack(target, EQEmu::inventory::slotPrimary, false, false, false, opts);
			// now lets check the quad attack
			if (GetSpecialAbility(SPECATK_QUAD) && RandRoll < (GetLevel() + NPCQuadAttackModifier)) {
				Attack(target, EQEmu::inventory::slotPrimary, false, false, false, opts);
			}
		}
	}
}

void Mob::DoOffHandAttackRounds(Mob *target, ExtraAttackOptions *opts)
{
	if (!target)
		return;
	// Mobs will only dual wield w/ the flag or have a secondary weapon
	// For now, SPECATK_QUAD means innate DW when Combat:UseLiveCombatRounds is true
	if ((GetSpecialAbility(SPECATK_INNATE_DW) ||
		(RuleB(Combat, UseLiveCombatRounds) && GetSpecialAbility(SPECATK_QUAD))) ||
		GetEquipment(EQEmu::textures::weaponSecondary) != 0) {
		if (CheckDualWield()) {
			Attack(target, EQEmu::inventory::slotSecondary, false, false, false, opts);
			if (CanThisClassDoubleAttack() && GetLevel() > 35 && CheckDoubleAttack()) {
				Attack(target, EQEmu::inventory::slotSecondary, false, false, false, opts);

				if ((IsPet() || IsTempPet()) && IsPetOwnerClient()) {
					int chance = spellbonuses.PC_Pet_Flurry + itembonuses.PC_Pet_Flurry + aabonuses.PC_Pet_Flurry;
					if (chance && zone->random.Roll(chance))
						Flurry(nullptr);
				}
			}
		}
	}
}
