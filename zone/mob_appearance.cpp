/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2018 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include "../common/eqemu_logsys.h"

#include "../common/misc_functions.h"
#include "../common/spdat.h"
#include "../common/string_util.h"

#include "mob.h"
#include "quest_parser_collection.h"
#include "zonedb.h"

#ifdef BOTS
#include "bot.h"
#endif

/**
 * Stores internal representation of mob texture by material slot
 *
 * @param material_slot
 * @param texture
 * @param color
 * @param hero_forge_model
 */
void Mob::SetMobTextureProfile(uint8 material_slot, uint16 texture, uint32 color, uint32 hero_forge_model)
{
	Log(Logs::Detail, Logs::MobAppearance,
		"Mob::SetMobTextureProfile [%s] material_slot: %u texture: %u color: %u hero_forge_model: %u",
		this->GetCleanName(),
		material_slot,
		texture,
		color,
		hero_forge_model
	);

	switch (material_slot) {
		case EQ::textures::armorHead:
			mob_texture_profile.Head.Material        = texture;
			mob_texture_profile.Head.HerosForgeModel = hero_forge_model;
			mob_texture_profile.Head.Color           = color;
			break;
		case EQ::textures::armorChest:
			mob_texture_profile.Chest.Material        = texture;
			mob_texture_profile.Chest.HerosForgeModel = hero_forge_model;
			mob_texture_profile.Chest.Color           = color;
			break;
		case EQ::textures::armorArms:
			mob_texture_profile.Arms.Material        = texture;
			mob_texture_profile.Arms.HerosForgeModel = hero_forge_model;
			mob_texture_profile.Arms.Color           = color;
			break;
		case EQ::textures::armorWrist:
			mob_texture_profile.Wrist.Material        = texture;
			mob_texture_profile.Wrist.HerosForgeModel = hero_forge_model;
			mob_texture_profile.Wrist.Color           = color;
			break;
		case EQ::textures::armorHands:
			mob_texture_profile.Hands.Material        = texture;
			mob_texture_profile.Hands.HerosForgeModel = hero_forge_model;
			mob_texture_profile.Hands.Color           = color;
			break;
		case EQ::textures::armorLegs:
			mob_texture_profile.Legs.Material        = texture;
			mob_texture_profile.Legs.HerosForgeModel = hero_forge_model;
			mob_texture_profile.Legs.Color           = color;
			break;
		case EQ::textures::armorFeet:
			mob_texture_profile.Feet.Material        = texture;
			mob_texture_profile.Feet.HerosForgeModel = hero_forge_model;
			mob_texture_profile.Feet.Color           = color;
			break;
		case EQ::textures::weaponPrimary:
			mob_texture_profile.Primary.Material        = texture;
			mob_texture_profile.Primary.HerosForgeModel = hero_forge_model;
			mob_texture_profile.Primary.Color           = color;
			break;
		case EQ::textures::weaponSecondary:
			mob_texture_profile.Secondary.Material        = texture;
			mob_texture_profile.Secondary.HerosForgeModel = hero_forge_model;
			mob_texture_profile.Secondary.Color           = color;
			break;
		default:
			return;
	}
}

/**
 * Returns internal representation of mob texture by material
 *
 * @param material_slot
 * @return
 */
int32 Mob::GetTextureProfileMaterial(uint8 material_slot) const
{
	switch (material_slot) {
		case EQ::textures::armorHead:
			return mob_texture_profile.Head.Material;
		case EQ::textures::armorChest:
			return mob_texture_profile.Chest.Material;
		case EQ::textures::armorArms:
			return mob_texture_profile.Arms.Material;
		case EQ::textures::armorWrist:
			return mob_texture_profile.Wrist.Material;
		case EQ::textures::armorHands:
			return mob_texture_profile.Hands.Material;
		case EQ::textures::armorLegs:
			return mob_texture_profile.Legs.Material;
		case EQ::textures::armorFeet:
			return mob_texture_profile.Feet.Material;
		case EQ::textures::weaponPrimary:
			return mob_texture_profile.Primary.Material;
		case EQ::textures::weaponSecondary:
			return mob_texture_profile.Secondary.Material;
		default:
			return 0;
	}
}

/**
 * Returns internal representation of mob texture by color
 *
 * @param material_slot
 * @return
 */
int32 Mob::GetTextureProfileColor(uint8 material_slot) const
{
	switch (material_slot) {
		case EQ::textures::armorHead:
			return mob_texture_profile.Head.Color;
		case EQ::textures::armorChest:
			return mob_texture_profile.Chest.Color;
		case EQ::textures::armorArms:
			return mob_texture_profile.Arms.Color;
		case EQ::textures::armorWrist:
			return mob_texture_profile.Wrist.Color;
		case EQ::textures::armorHands:
			return mob_texture_profile.Hands.Color;
		case EQ::textures::armorLegs:
			return mob_texture_profile.Legs.Color;
		case EQ::textures::armorFeet:
			return mob_texture_profile.Feet.Color;
		case EQ::textures::weaponPrimary:
			return mob_texture_profile.Primary.Color;
		case EQ::textures::weaponSecondary:
			return mob_texture_profile.Secondary.Color;
		default:
			return 0;
	}
}

/**
 * Returns internal representation of mob texture by HerosForgeModel
 *
 * @param material_slot
 * @return
 */
int32 Mob::GetTextureProfileHeroForgeModel(uint8 material_slot) const
{
	switch (material_slot) {
		case EQ::textures::armorHead:
			return mob_texture_profile.Head.HerosForgeModel;
		case EQ::textures::armorChest:
			return mob_texture_profile.Chest.HerosForgeModel;
		case EQ::textures::armorArms:
			return mob_texture_profile.Arms.HerosForgeModel;
		case EQ::textures::armorWrist:
			return mob_texture_profile.Wrist.HerosForgeModel;
		case EQ::textures::armorHands:
			return mob_texture_profile.Hands.HerosForgeModel;
		case EQ::textures::armorLegs:
			return mob_texture_profile.Legs.HerosForgeModel;
		case EQ::textures::armorFeet:
			return mob_texture_profile.Feet.HerosForgeModel;
		case EQ::textures::weaponPrimary:
			return mob_texture_profile.Primary.HerosForgeModel;
		case EQ::textures::weaponSecondary:
			return mob_texture_profile.Secondary.HerosForgeModel;
		default:
			return 0;
	}
}

/**
 * Gets the material or texture for a slot (leather / plate etc.)
 *
 * @param material_slot
 * @return
 */
int32 Mob::GetEquipmentMaterial(uint8 material_slot) const
{
	uint32 equipment_material         = 0;
	int32  ornamentation_augment_type = RuleI(Character, OrnamentationAugmentType);

	int32 texture_profile_material = GetTextureProfileMaterial(material_slot);

	Log(Logs::Detail, Logs::MobAppearance,
		"Mob::GetEquipmentMaterial [%s] material_slot: %u texture_profile_material: %i",
		this->clean_name,
		material_slot,
		texture_profile_material
	);

	if (texture_profile_material > 0) {
		return texture_profile_material;
	}

	auto item = database.GetItem(GetEquippedItemFromTextureSlot(material_slot));

	if (item != nullptr) {

		/**
		 * Handle primary / secondary texture
		 */
		bool is_primary_or_secondary_weapon =
				 material_slot == EQ::textures::weaponPrimary ||
				 material_slot == EQ::textures::weaponSecondary;

		if (is_primary_or_secondary_weapon) {
			if (this->IsClient()) {

				int16 inventory_slot = EQ::InventoryProfile::CalcSlotFromMaterial(material_slot);
				if (inventory_slot == INVALID_INDEX) {
					return 0;
				}

				const EQ::ItemInstance *item_instance = CastToClient()->m_inv[inventory_slot];
				if (item_instance) {
					if (item_instance->GetOrnamentationAug(ornamentation_augment_type)) {
						item = item_instance->GetOrnamentationAug(ornamentation_augment_type)->GetItem();
						if (item && strlen(item->IDFile) > 2) {
							equipment_material = atoi(&item->IDFile[2]);
						}
					}
					else if (item_instance->GetOrnamentationIDFile()) {
						equipment_material = item_instance->GetOrnamentationIDFile();
					}
				}
			}

			if (equipment_material == 0 && strlen(item->IDFile) > 2) {
				equipment_material = atoi(&item->IDFile[2]);
			}
		}
		else {
			equipment_material = item->Material;
		}
	}

	return equipment_material;
}

/**
 * @param material_slot
 * @return
 */
uint32 Mob::GetEquipmentColor(uint8 material_slot) const
{
	const EQ::ItemData *item = nullptr;

	if (armor_tint.Slot[material_slot].Color) {
		return armor_tint.Slot[material_slot].Color;
	}

	item = database.GetItem(GetEquippedItemFromTextureSlot(material_slot));
	if (item != nullptr) {
		return item->Color;
	}

	return 0;
}

/**
 * @param material_slot
 * @return
 */
int32 Mob::GetHerosForgeModel(uint8 material_slot) const
{
	uint32 hero_model = 0;
	if (material_slot >= 0 && material_slot < EQ::textures::weaponPrimary) {
		uint32 ornamentation_aug_type = RuleI(Character, OrnamentationAugmentType);

		const EQ::ItemData *item = database.GetItem(GetEquippedItemFromTextureSlot(material_slot));
		int16 invslot = EQ::InventoryProfile::CalcSlotFromMaterial(material_slot);

		if (item != nullptr && invslot != INVALID_INDEX) {
			if (IsClient()) {
				const EQ::ItemInstance *inst = CastToClient()->m_inv[invslot];
				if (inst) {
					if (inst->GetOrnamentationAug(ornamentation_aug_type)) {
						item       = inst->GetOrnamentationAug(ornamentation_aug_type)->GetItem();
						hero_model = item->HerosForgeModel;
					}
					else if (inst->GetOrnamentHeroModel()) {
						hero_model = inst->GetOrnamentHeroModel();
					}
				}
			}

			if (hero_model == 0) {
				hero_model = item->HerosForgeModel;
			}
		}

		if (IsNPC()) {
			hero_model = CastToNPC()->GetHeroForgeModel();

			/**
			 * Robes require full model number, and should only be sent to chest, arms, wrists, and legs slots
			 */
			if (hero_model > 1000 && material_slot != 1 && material_slot != 2 && material_slot != 3 &&
				material_slot != 5) {
				hero_model = 0;
			}
		}
	}

	/**
	 * Auto-Convert Hero Model to match the slot
	 *
	 * Otherwise, use the exact Model if model is > 999
	 * Robes for example are 11607 to 12107 in RoF
	 */
	if (hero_model > 0 && hero_model < 1000) {
		hero_model *= 100;
		hero_model += material_slot;
	}

	return hero_model;
}

uint32 NPC::GetEquippedItemFromTextureSlot(uint8 material_slot) const
{
	if (material_slot > 8) {
		return 0;
	}

	int16 inventory_slot = EQ::InventoryProfile::CalcSlotFromMaterial(material_slot);
	if (inventory_slot == INVALID_INDEX) {
		return 0;
	}

	return equipment[inventory_slot];
}

/**
 * NPCs typically use this function for sending appearance
 * @param one_client
 */
void Mob::SendArmorAppearance(Client *one_client)
{
	/**
	 * one_client of 0 means sent to all clients
	 *
	 * Despite the fact that OP_NewSpawn and OP_ZoneSpawns include the
	 * armor being worn and its mats, the client doesn't update the display
	 * on arrival of these packets reliably.
	 *
	 * Send Wear changes if mob is a PC race and item is an armor slot.
	 * The other packets work for primary/secondary.
	 */

	Log(Logs::Detail, Logs::MobAppearance, "Mob::SendArmorAppearance [%s]",
		this->GetCleanName()
	);

	if (IsPlayerRace(race)) {
		if (!IsClient()) {
			for (uint8 i = 0; i <= EQ::textures::materialCount; ++i) {
				const EQ::ItemData *item = database.GetItem(GetEquippedItemFromTextureSlot(i));
				if (item != nullptr) {
					SendWearChange(i, one_client);
				}
			}
		}
	}

	for (uint8 i = 0; i <= EQ::textures::materialCount; ++i) {
		if (GetTextureProfileMaterial(i)) {
			SendWearChange(i, one_client);
		}
	}
}

/**
 * @param material_slot
 * @param one_client
 */
void Mob::SendWearChange(uint8 material_slot, Client *one_client)
{
	auto packet       = new EQApplicationPacket(OP_WearChange, sizeof(WearChange_Struct));
	auto *wear_change = (WearChange_Struct *) packet->pBuffer;

	Log(Logs::Detail, Logs::MobAppearance, "Mob::SendWearChange [%s]",
		this->GetCleanName()
	);

	wear_change->spawn_id         = GetID();
	wear_change->material         = static_cast<uint32>(GetEquipmentMaterial(material_slot));
	wear_change->elite_material   = IsEliteMaterialItem(material_slot);
	wear_change->hero_forge_model = static_cast<uint32>(GetHerosForgeModel(material_slot));

#ifdef BOTS
	if (IsBot()) {
		auto item_inst = CastToBot()->GetBotItem(EQ::InventoryProfile::CalcSlotFromMaterial(material_slot));
		if (item_inst)
			wear_change->color.Color = item_inst->GetColor();
		else
			wear_change->color.Color = 0;
	}
	else {
		wear_change->color.Color = GetEquipmentColor(material_slot);
	}
#else
	wear_change->color.Color = GetEquipmentColor(material_slot);
#endif

	wear_change->wear_slot_id = material_slot;

	if (!one_client) {
		entity_list.QueueClients(this, packet);
	}
	else {
		one_client->QueuePacket(packet, false, Client::CLIENT_CONNECTED);
	}

	safe_delete(packet);
}

/**
 *
 * @param slot
 * @param texture
 * @param hero_forge_model
 * @param elite_material
 * @param unknown06
 * @param unknown18
 */
void Mob::SendTextureWC(
	uint8 slot,
	uint16 texture,
	uint32 hero_forge_model,
	uint32 elite_material,
	uint32 unknown06,
	uint32 unknown18
)
{
	auto outapp       = new EQApplicationPacket(OP_WearChange, sizeof(WearChange_Struct));
	auto *wear_change = (WearChange_Struct *) outapp->pBuffer;

	if (this->IsClient()) {
		wear_change->color.Color = GetEquipmentColor(slot);
	}
	else {
		wear_change->color.Color = this->GetArmorTint(slot);
	}

	wear_change->spawn_id         = this->GetID();
	wear_change->material         = texture;
	wear_change->wear_slot_id     = slot;
	wear_change->unknown06        = unknown06;
	wear_change->elite_material   = elite_material;
	wear_change->hero_forge_model = hero_forge_model;
	wear_change->unknown18        = unknown18;

	SetMobTextureProfile(slot, texture, wear_change->color.Color, hero_forge_model);

	entity_list.QueueClients(this, outapp);
	safe_delete(outapp);
}

/**
 * @param material_slot
 * @param red_tint
 * @param green_tint
 * @param blue_tint
 */
void Mob::SetSlotTint(uint8 material_slot, uint8 red_tint, uint8 green_tint, uint8 blue_tint)
{
	uint32 color;
	color = (red_tint & 0xFF) << 16;
	color |= (green_tint & 0xFF) << 8;
	color |= (blue_tint & 0xFF);
	color |= (color) ? (0xFF << 24) : 0;
	armor_tint.Slot[material_slot].Color = color;

	auto outapp = new EQApplicationPacket(OP_WearChange, sizeof(WearChange_Struct));
	auto *wc    = (WearChange_Struct *) outapp->pBuffer;

	wc->spawn_id         = this->GetID();
	wc->material         = GetEquipmentMaterial(material_slot);
	wc->hero_forge_model = GetHerosForgeModel(material_slot);
	wc->color.Color      = color;
	wc->wear_slot_id     = material_slot;

	SetMobTextureProfile(material_slot, texture, color);

	entity_list.QueueClients(this, outapp);
	safe_delete(outapp);
}

/**
 * @param material_slot
 * @param texture
 * @param color
 * @param hero_forge_model
 */
void Mob::WearChange(uint8 material_slot, uint16 texture, uint32 color, uint32 hero_forge_model)
{
	armor_tint.Slot[material_slot].Color = color;

	/**
	 * Change internal values
	 */
	SetMobTextureProfile(material_slot, texture, color, hero_forge_model);

	/**
	 * Packet update
	 */
	auto outapp       = new EQApplicationPacket(OP_WearChange, sizeof(WearChange_Struct));
	auto *wear_change = (WearChange_Struct *) outapp->pBuffer;

	wear_change->spawn_id         = this->GetID();
	wear_change->material         = texture;
	wear_change->hero_forge_model = hero_forge_model;
	wear_change->color.Color      = color;
	wear_change->wear_slot_id     = material_slot;

	entity_list.QueueClients(this, outapp);
	safe_delete(outapp);
}