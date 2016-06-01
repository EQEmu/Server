/*	EQEMu: Everquest Server Emulator
	
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef COMMON_TEXTURES_H
#define COMMON_TEXTURES_H

#include "types.h"


namespace EQEmu
{
	//class InventorySlot;
	
	namespace textures {
		//enum : int { TextureInvalid = -1, TextureBegin };
		enum : uint8 { TextureInvalid = 255, TextureBegin = 0 };

		//enum TextureSlot : int {
		enum TextureSlot : uint8 {
			TextureHead = TextureBegin,
			TextureChest,
			TextureArms,
			TextureWrist,
			TextureHands,
			TextureLegs,
			TextureFeet,
			TexturePrimary,
			TextureSecondary,
			TextureCount
		};

		const int LastTexture = TextureSecondary;
		const int LastTintableTexture = TextureFeet;

		//extern int ConvertEquipmentSlotToTextureSlot(int equipment_slot);
		//extern int ConvertEquipmentSlotToTextureSlot(const InventorySlot &equipment_slot);
		//extern InventorySlot ConvertTextureSlotToEquipmentSlot(int texture_slot);

	} /*textures*/

	struct TextureProfile {
		union {
			struct {
				uint32 Head;
				uint32 Chest;
				uint32 Arms;
				uint32 Wrist;
				uint32 Hands;
				uint32 Legs;
				uint32 Feet;
				uint32 Primary;
				uint32 Secondary;
			};
			uint32 Texture[textures::TextureCount];
		};

		TextureProfile();
	};

	struct Tint_Struct {
		union {
			struct {
				uint8 Blue;
				uint8 Green;
				uint8 Red;
				uint8 UseTint;	// if there's a tint this is FF
			};
			uint32 Color;
		};

		//Tint_Struct();
	};

	struct TintProfile {
		union {
			struct {
				Tint_Struct Head;
				Tint_Struct Chest;
				Tint_Struct Arms;
				Tint_Struct Wrist;
				Tint_Struct Hands;
				Tint_Struct Legs;
				Tint_Struct Feet;
				Tint_Struct Primary;
				Tint_Struct Secondary;
			};
			Tint_Struct Slot[textures::TextureCount];
		};

		TintProfile();
	};

} /*EQEmu*/

#endif /*COMMON_TEXTURES_H*/
