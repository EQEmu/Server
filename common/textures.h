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

	struct Texture_Struct {
		uint32 Material;
		uint32 Unknown1;
		uint32 EliteMaterial;
		uint32 HeroForgeModel;
		uint32 Material2;	// Same as material?
	};

	struct TextureProfile {
		union {
			struct {
				Texture_Struct Head;
				Texture_Struct Chest;
				Texture_Struct Arms;
				Texture_Struct Wrist;
				Texture_Struct Hands;
				Texture_Struct Legs;
				Texture_Struct Feet;
				Texture_Struct Primary;
				Texture_Struct Secondary;
			};
			Texture_Struct Slot[textures::TextureCount];
		};

		TextureProfile();
	};

	struct TextureShort_Struct {
		uint32 Material;
	};

	struct TextureShortProfile {
		union {
			struct {
				TextureShort_Struct Head;
				TextureShort_Struct Chest;
				TextureShort_Struct Arms;
				TextureShort_Struct Wrist;
				TextureShort_Struct Hands;
				TextureShort_Struct Legs;
				TextureShort_Struct Feet;
				TextureShort_Struct Primary;
				TextureShort_Struct Secondary;
			};
			TextureShort_Struct Slot[textures::TextureCount];
		};

		TextureShortProfile();
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
