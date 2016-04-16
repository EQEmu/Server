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


void EQEmu::LightSource::impl::Clear()
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

uint8 EQEmu::LightSource::TypeToLevel(uint8 light_type)
{
	switch (light_type) {
	case TypeGlobeOfStars:
		return LevelBrilliant;		// 10
	case TypeFlamelessLantern:
	case TypeGreaterLightstone:
		return LevelLargeMagic;		// 9
	case TypeLargeLantern:
		return LevelLargeLantern;	// 8
	case TypeSteinOfMoggok:
	case TypeLightstone:
		return LevelMagicLantern;	// 7
	case TypeSmallLantern:
		return LevelSmallLantern;	// 6
	case TypeColdlight:
	case TypeUnknown2:
		return LevelBlueLight;		// 5
	case TypeFireBeetleEye:
	case TypeUnknown1:
		return LevelRedLight;		// 4
	case TypeTinyGlowingSkull:
	case TypeLightGlobe:
		return LevelSmallMagic;		// 3
	case TypeTorch:
		return LevelTorch;			// 2
	case TypeCandle:
		return LevelCandle;			// 1
	default:
		return LevelUnlit;			// 0
	}
}

bool EQEmu::LightSource::IsLevelGreater(uint8 left_type, uint8 right_type)
{
	static const uint8 light_levels[TypeCount] = {
		LevelUnlit,				/* TypeNone */
		LevelCandle,			/* TypeCandle */
		LevelTorch,				/* TypeTorch */
		LevelSmallMagic,		/* TypeTinyGlowingSkull */
		LevelSmallLantern,		/* TypeSmallLantern */
		LevelMagicLantern,		/* TypeSteinOfMoggok */
		LevelLargeLantern,		/* TypeLargeLantern */
		LevelLargeMagic,		/* TypeFlamelessLantern */
		LevelBrilliant,			/* TypeGlobeOfStars */
		LevelSmallMagic,		/* TypeLightGlobe */
		LevelMagicLantern,		/* TypeLightstone */
		LevelLargeMagic,		/* TypeGreaterLightstone */
		LevelRedLight,			/* TypeFireBeetleEye */
		LevelBlueLight,			/* TypeColdlight */
		LevelRedLight,			/* TypeUnknown1 */
		LevelBlueLight			/* TypeUnknown2 */
	};

	if (left_type >= TypeCount) { left_type = TypeNone; }
	if (right_type >= TypeCount) { right_type = TypeNone; }

	return (light_levels[left_type] > light_levels[right_type]);
}
