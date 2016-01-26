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

#include "types.h"
#include "skills.h"

bool EQEmu::IsTradeskill(SkillUseTypes skill)
{
	switch (skill) {
	case SkillFishing:
	case SkillMakePoison:
	case SkillTinkering:
	case SkillResearch:
	case SkillAlchemy:
	case SkillBaking:
	case SkillTailoring:
	case SkillBlacksmithing:
	case SkillFletching:
	case SkillBrewing:
	case SkillPottery:
	case SkillJewelryMaking:
		return true;
	default:
		return false;
	}
}

bool EQEmu::IsSpecializedSkill(SkillUseTypes skill)
{
	// this could be a simple if, but if this is more portable if any IDs change (probably won't)
	// or any other specialized are added (also unlikely)
	switch (skill) {
	case SkillSpecializeAbjure:
	case SkillSpecializeAlteration:
	case SkillSpecializeConjuration:
	case SkillSpecializeDivination:
	case SkillSpecializeEvocation:
		return true;
	default:
		return false;
	}
}

float EQEmu::GetSkillMeleePushForce(SkillUseTypes skill)
{
	// This is the force/magnitude of the push from an attack of this skill type
	// You can find these numbers in the clients skill struct
	switch (skill) {
	case Skill1HBlunt:
	case Skill1HSlashing:
	case SkillHandtoHand:
	case SkillThrowing:
		return 0.1f;
	case Skill2HBlunt:
	case Skill2HSlashing:
	case SkillEagleStrike:
	case SkillKick:
	case SkillTigerClaw:
	case Skill2HPiercing:
		return 0.2f;
	case SkillArchery:
		return 0.15f;
	case SkillBackstab:
	case SkillBash:
		return 0.3f;
	case SkillDragonPunch:
	case SkillRoundKick:
		return 0.25f;
	case SkillFlyingKick:
		return 0.4f;
	case Skill1HPiercing:
	case SkillFrenzy:
		return 0.05f;
	case SkillIntimidation:
		return 2.5f;
	default:
		return 0.0f;
	}
}

bool EQEmu::IsBardInstrumentSkill(SkillUseTypes skill)
{
	switch (skill) {
	case SkillBrassInstruments:
	case SkillSinging:
	case SkillStringedInstruments:
	case SkillWindInstruments:
	case SkillPercussionInstruments:
		return true;
	default:
		return false;
	}
}
