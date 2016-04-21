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

#ifndef COMMON_LIGHT_SOURCE_H
#define COMMON_LIGHT_SOURCE_H

#include "types.h"


namespace EQEmu
{
	namespace lightsource {
		enum LightType {
			LightTypeNone = 0,
			LightTypeCandle,
			LightTypeTorch,
			LightTypeTinyGlowingSkull,
			LightTypeSmallLantern,
			LightTypeSteinOfMoggok, // 5
			LightTypeLargeLantern,
			LightTypeFlamelessLantern,
			LightTypeGlobeOfStars,
			LightTypeLightGlobe,
			LightTypeLightstone, // 10
			LightTypeGreaterLightstone,
			LightTypeFireBeetleEye,
			LightTypeColdlight,
			LightTypeUnknown1,
			LightTypeUnknown2, // 15
			LightTypeCount
		};

		enum LightLevel {
			LightLevelUnlit = 0,
			LightLevelCandle,
			LightLevelTorch,
			LightLevelSmallMagic,
			LightLevelRedLight,
			LightLevelBlueLight, // 5
			LightLevelSmallLantern,
			LightLevelMagicLantern,
			LightLevelLargeLantern,
			LightLevelLargeMagic,
			LightLevelBrilliant, // 10
			LightLevelCount
		};

		struct LightSourceProfile {
			/*
			Current criteria (light types):
			Equipment:	{ 0 .. 15 }
			General:	{ 9 .. 13 }

			Notes:
			- Initial character load and item movement updates use different light source update behaviors
			-- Server procedure matches the item movement behavior since most updates occur post-character load
			- MainAmmo is not considered when determining light sources
			- No 'Sub' or 'Aug' items are recognized as light sources
			- Light types '< 9' and '> 13' are not considered for general (carried) light sources
			- If values > 0x0F are valid, then assignment limiters will need to be removed
			- MainCursor 'appears' to be a valid light source update slot..but, have not experienced updates during debug sessions
			- All clients have a bug regarding stackable items (light and sound updates are not processed when picking up an item)
			-- The timer-based update cancels out the invalid light source
			*/
			LightSourceProfile();

			void Clear();

			// Light types (classifications)
			struct {
				uint8 Innate;		// Defined by db field `npc_types`.`light` - where appropriate
				uint8 Equipment;	// Item_Struct::light value of worn/carried equipment
				uint8 Spell;		// Set value of any light-producing spell (can be modded to mimic equip_light behavior)
				uint8 Active;		// Highest value of all light sources
			} Type;

			// Light levels (intensities) - used to determine which light source should be active
			struct {
				uint8 Innate;
				uint8 Equipment;
				uint8 Spell;
				uint8 Active;
			} Level;
		};

		extern uint8 TypeToLevel(uint8 light_type);
		extern bool IsLevelGreater(uint8 left_type, uint8 right_type);
	};
}

#endif /* COMMON_LIGHT_SOURCE_H */
