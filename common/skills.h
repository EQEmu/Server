/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemulator.org)

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
#ifndef SKILLS_H
#define SKILLS_H

#include <string>
#include <map>

/*
**	Skill use types
**
**	(ref: eqstr_us.txt [05-10-2013])
*/
enum SkillUseTypes
{
/*13855*/	Skill1HBlunt = 0,
/*13856*/	Skill1HSlashing,
/*13857*/	Skill2HBlunt,
/*13858*/	Skill2HSlashing,
/*13859*/	SkillAbjuration,
/*13861*/	SkillAlteration, // 5
/*13862*/	SkillApplyPoison,
/*13863*/	SkillArchery,
/*13864*/	SkillBackstab,
/*13866*/	SkillBindWound,
/*13867*/	SkillBash, // 10
/*13871*/	SkillBlock,
/*13872*/	SkillBrassInstruments,
/*13874*/	SkillChanneling,
/*13875*/	SkillConjuration,
/*13876*/	SkillDefense, // 15
/*13877*/	SkillDisarm,
/*13878*/	SkillDisarmTraps,
/*13879*/	SkillDivination,
/*13880*/	SkillDodge,
/*13881*/	SkillDoubleAttack, // 20
/*13882*/	SkillDragonPunch,
/*13924*/	SkillTailRake = SkillDragonPunch, // Iksar Monk equivilent
/*13883*/	SkillDualWield,
/*13884*/	SkillEagleStrike,
/*13885*/	SkillEvocation,
/*13886*/	SkillFeignDeath, // 25
/*13888*/	SkillFlyingKick,
/*13889*/	SkillForage,
/*13890*/	SkillHandtoHand,
/*13891*/	SkillHide,
/*13893*/	SkillKick, // 30
/*13894*/	SkillMeditate,
/*13895*/	SkillMend,
/*13896*/	SkillOffense,
/*13897*/	SkillParry,
/*13899*/	SkillPickLock, // 35
/*13900*/	Skill1HPiercing,				// Changed in RoF2(05-10-2013)
/*13903*/	SkillRiposte,
/*13904*/	SkillRoundKick,
/*13905*/	SkillSafeFall,
/*13906*/	SkillSenseHeading, // 40
/*13908*/	SkillSinging,
/*13909*/	SkillSneak,
/*13910*/	SkillSpecializeAbjure,			// No idea why they truncated this one..especially when there are longer ones...
/*13911*/	SkillSpecializeAlteration,
/*13912*/	SkillSpecializeConjuration, // 45
/*13913*/	SkillSpecializeDivination,
/*13914*/	SkillSpecializeEvocation,
/*13915*/	SkillPickPockets,
/*13916*/	SkillStringedInstruments,
/*13917*/	SkillSwimming, // 50
/*13919*/	SkillThrowing,
/*13920*/	SkillTigerClaw,
/*13921*/	SkillTracking,
/*13923*/	SkillWindInstruments,
/*13854*/	SkillFishing, // 55
/*13853*/	SkillMakePoison,
/*13852*/	SkillTinkering,
/*13851*/	SkillResearch,
/*13850*/	SkillAlchemy,
/*13865*/	SkillBaking, // 60
/*13918*/	SkillTailoring,
/*13907*/	SkillSenseTraps,
/*13870*/	SkillBlacksmithing,
/*13887*/	SkillFletching,
/*13873*/	SkillBrewing, // 65
/*13860*/	SkillAlcoholTolerance,
/*13868*/	SkillBegging,
/*13892*/	SkillJewelryMaking,
/*13901*/	SkillPottery,
/*13898*/	SkillPercussionInstruments, // 70
/*13922*/	SkillIntimidation,
/*13869*/	SkillBerserking,
/*13902*/	SkillTaunt,
/*05837*/	SkillFrenzy, // 74				// This appears to be the only listed one not grouped with the others

// SoF+ specific skills
/*03670*/	SkillRemoveTraps, // 75
/*13049*/	SkillTripleAttack,

// RoF2+ specific skills
/*00789*/	Skill2HPiercing, // 77
// /*01216*/	SkillNone,						// This needs to move down as new skills are added

/*00000*/	_EmuSkillCount					// move to last position of active enumeration labels

// Skill Counts
// /*-----*/	_SkillCount_62 = 75,			// use for Ti and earlier max skill checks
// /*-----*/	_SkillCount_SoF = 77,			// use for SoF thru RoF1 max skill checks
// /*-----*/	_SkillCount_RoF2 = 78,			// use for RoF2 max skill checks

// Support values
// /*-----*/	_SkillServerArraySize = _SkillCount_RoF2,	// Should reflect last client '_SkillCount'

// Superfluous additions to SkillUseTypes..server-use only
// /*-----*/	ExtSkillGenericTradeskill = 100

/*					([EQClientVersion]	[0] - Unknown, [3] - SoF, [7] - RoF2[05-10-2013])
	[Skill]			[index]	|	[0]		[1]		[2]		[3]		[4]		[5]		[6]		[7]
	Frenzy			(05837)	|	---		074		074		074		074		074		074		074
	Remove Traps	(03670)	|	---		---		---		075		075		075		075		075
	Triple Attack	(13049)	|	---		---		---		076		076		076		076		076
	2H Piercing		(00789)	|	---		---		---		---		---		---		---		077
*/

/*
	[SkillCaps.txt] (SoF+)
	a^b^c^d(^e)	(^e is RoF+, but cursory glance appears to be all zeros)

	a - Class
	b - Skill
	c - Level
	d - Max Value
	(e - Unknown)
*/

/*
	NOTE: Disregard this until it is sorted out

	I changed (tradeskill==75) to ExtSkillGenericTradeskill in tradeskills.cpp for both instances. 	If it's a pseudo-enumeration of
	an AA ability, then use the 'ExtSkill' ('ExtendedSkill') prefix with a value >= 100. (current implementation)

	We probably need to recode ALL of the skill checks to use the new Skill2HPiercing and ensure that the animation value is
	properly changed in the patch files. As far as earlier clients using this new skill, it can be done, but we just need to ensure
	that skill address is not inadvertently passed to the client..and we can just send an actual message for the skill-up. Use of a
	command can tell the player what that particular skill value is.

	Nothing on SkillTripleAttack just yet..haven't looked into its current implementation.

	In addition to the above re-coding, we're probably going to need to rework the database pp blob to reserve space for the current
	100-dword buffer allocation. This way, we can just add new ones without having to rework it each time.
	(Wasn't done for this in particular..but, thanks Akkadius!)

	-U
*/
};

// temporary until it can be sorted out...
#define HIGHEST_SKILL	Skill2HPiercing
// Spell Effects use this value to determine if an effect applies to all skills.
#define ALL_SKILLS	-1

// server profile does not reflect this yet..so, prefixed with 'PACKET_'
#define PACKET_SKILL_ARRAY_SIZE 100


// for skill related helper functions
namespace EQEmu {
	bool IsTradeskill(SkillUseTypes skill);
	bool IsSpecializedSkill(SkillUseTypes skill);
	float GetSkillMeleePushForce(SkillUseTypes skill);
	bool IsBardInstrumentSkill(SkillUseTypes skill);

	const std::map<SkillUseTypes, std::string>& GetSkillUseTypesMap();
}

#endif
