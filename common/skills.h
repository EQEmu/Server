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

// Correct Skill Numbers as of 4-14-2002
typedef enum {
	_1H_BLUNT				= 0,
	_1H_SLASHING			= 1,
	_2H_BLUNT				= 2,
	_2H_SLASHING			= 3,
	ABJURE					= 4,
	ALTERATION				= 5,
	APPLY_POISON			= 6,
	ARCHERY					= 7,
	BACKSTAB				= 8,
	BIND_WOUND				= 9,
	BASH					= 10,
	BLOCKSKILL				= 11,
	BRASS_INSTRUMENTS		= 12,
	CHANNELING				= 13,
	CONJURATION				= 14,
	DEFENSE					= 15,
	DISARM					= 16,
	DISARM_TRAPS			= 17,
	DIVINATION				= 18,
	DODGE					= 19,
	DOUBLE_ATTACK			= 20,
	DRAGON_PUNCH			= 21	,	//aka Tail Rake
	DUAL_WIELD				= 22,
	EAGLE_STRIKE			= 23,
	EVOCATION				= 24,
	FEIGN_DEATH				= 25,
	FLYING_KICK				= 26,
	FORAGE					= 27,
	HAND_TO_HAND			= 28,
	HIDE					= 29,
	KICK					= 30,
	MEDITATE				= 31,
	MEND					= 32,
	OFFENSE					= 33,
	PARRY					= 34,
	PICK_LOCK				= 35,
	PIERCING				= 36,
	RIPOSTE					= 37,
	ROUND_KICK				= 38,
	SAFE_FALL				= 39,
	SENSE_HEADING			= 40,
	SINGING					= 41,
	SNEAK					= 42,
	SPECIALIZE_ABJURE		= 43,
	SPECIALIZE_ALTERATION	= 44,
	SPECIALIZE_CONJURATION	= 45,
	SPECIALIZE_DIVINATION	= 46,
	SPECIALIZE_EVOCATION	= 47,
	PICK_POCKETS			= 48,
	STRINGED_INSTRUMENTS	= 49,
	SWIMMING				= 50,
	THROWING				= 51,
	TIGER_CLAW				= 52,
	TRACKING				= 53,
	WIND_INSTRUMENTS		= 54,
	FISHING					= 55,
	MAKE_POISON				= 56,
	TINKERING				= 57,
	RESEARCH				= 58,
	ALCHEMY					= 59,
	BAKING					= 60,
	TAILORING				= 61,
	SENSE_TRAPS				= 62,
	BLACKSMITHING			= 63,
	FLETCHING				= 64,
	BREWING					= 65,
	ALCOHOL_TOLERANCE		= 66,
	BEGGING					= 67,
	JEWELRY_MAKING			= 68,
	POTTERY					= 69,
	PERCUSSION_INSTRUMENTS	= 70,
	INTIMIDATION			= 71,
	BERSERKING				= 72,
	TAUNT					= 73,
	FRENZY					= 74,
	GENERIC_TRADESKILL		= 75
} SkillType;

#define HIGHEST_SKILL	FRENZY

#endif

