/*	EQEMu:  Everquest Server Emulator
	
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "light_source.h"


EQEmu::lightsource::LightSourceProfile::LightSourceProfile()
{
	Clear();
}

void EQEmu::lightsource::LightSourceProfile::Clear()
{
	Type.Innate = 0;
	Type.Equipment = 0;
	Type.Spell = 0;
	Type.Active = 0;

	Level.Innate = 0;
	Level.Equipment = 0;
	Level.Spell = 0;
	Level.Active = 0;
}

uint8 EQEmu::lightsource::TypeToLevel(uint8 light_type)
{
	switch (light_type) {
	case LightTypeGlobeOfStars:
		return LightLevelBrilliant;		// 10
	case LightTypeFlamelessLantern:
	case LightTypeGreaterLightstone:
		return LightLevelLargeMagic;	// 9
	case LightTypeLargeLantern:
		return LightLevelLargeLantern;	// 8
	case LightTypeSteinOfMoggok:
	case LightTypeLightstone:
		return LightLevelMagicLantern;	// 7
	case LightTypeSmallLantern:
		return LightLevelSmallLantern;	// 6
	case LightTypeColdlight:
	case LightTypeUnknown2:
		return LightLevelBlueLight;		// 5
	case LightTypeFireBeetleEye:
	case LightTypeUnknown1:
		return LightLevelRedLight;		// 4
	case LightTypeTinyGlowingSkull:
	case LightTypeLightGlobe:
		return LightLevelSmallMagic;	// 3
	case LightTypeTorch:
		return LightLevelTorch;			// 2
	case LightTypeCandle:
		return LightLevelCandle;		// 1
	default:
		return LightLevelUnlit;			// 0
	}
}

bool EQEmu::lightsource::IsLevelGreater(uint8 left_type, uint8 right_type)
{
	static const uint8 light_levels[LightTypeCount] = {
		LightLevelUnlit,			/* LightTypeNone */
		LightLevelCandle,			/* LightTypeCandle */
		LightLevelTorch,			/* LightTypeTorch */
		LightLevelSmallMagic,		/* LightTypeTinyGlowingSkull */
		LightLevelSmallLantern,		/* LightTypeSmallLantern */
		LightLevelMagicLantern,		/* LightTypeSteinOfMoggok */
		LightLevelLargeLantern,		/* LightTypeLargeLantern */
		LightLevelLargeMagic,		/* LightTypeFlamelessLantern */
		LightLevelBrilliant,		/* LightTypeGlobeOfStars */
		LightLevelSmallMagic,		/* LightTypeLightGlobe */
		LightLevelMagicLantern,		/* LightTypeLightstone */
		LightLevelLargeMagic,		/* LightTypeGreaterLightstone */
		LightLevelRedLight,			/* LightTypeFireBeetleEye */
		LightLevelBlueLight,		/* LightTypeColdlight */
		LightLevelRedLight,			/* LightTypeUnknown1 */
		LightLevelBlueLight			/* LightTypeUnknown2 */
	};

	if (left_type >= LightTypeCount) { left_type = LightTypeNone; }
	if (right_type >= LightTypeCount) { right_type = LightTypeNone; }

	return (light_levels[left_type] > light_levels[right_type]);
}
