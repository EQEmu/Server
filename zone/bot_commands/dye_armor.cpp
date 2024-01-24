#include "../client.h"
#include "../bot_command.h"

void bot_command_dye_armor(Client *c, const Seperator *sep)
{
	// TODO: Trouble-shoot model update issue

	const std::string material_slot_message = fmt::format(
		"Material Slots: * (All), {} (Head), {} (Chest), {} (Arms), {} (Wrists), {} (Hands), {} (Legs), {} (Feet)",
		EQ::textures::armorHead,
		EQ::textures::armorChest,
		EQ::textures::armorArms,
		EQ::textures::armorWrist,
		EQ::textures::armorHands,
		EQ::textures::armorLegs,
		EQ::textures::armorFeet
	);

	if (helper_command_alias_fail(c, "bot_command_dye_armor", sep->arg[0], "botdyearmor")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1]) || !sep->arg[1] || (sep->arg[1] && !Strings::IsNumber(sep->arg[1]))) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} [Material Slot] [Red: 0-255] [Green: 0-255] [Blue: 0-255] ([actionable: target | byname | ownergroup | targetgroup | namesgroup | healrotation | spawned] ([actionable_name]))",
				sep->arg[0]
			).c_str()
		);
		c->Message(Chat::White, material_slot_message.c_str());
		return;
	}
	const int ab_mask = ActionableBots::ABM_NoFilter;

	uint8 material_slot = EQ::textures::materialInvalid;
	int16 slot_id = INVALID_INDEX;

	bool dye_all = (sep->arg[1][0] == '*');
	if (!dye_all) {
		material_slot = Strings::ToInt(sep->arg[1]);
		slot_id = EQ::InventoryProfile::CalcSlotFromMaterial(material_slot);

		if (!sep->IsNumber(1) || slot_id == INVALID_INDEX || material_slot > EQ::textures::LastTintableTexture) {
			c->Message(Chat::White, "Valid material slots for this command are:");
			c->Message(Chat::White, material_slot_message.c_str());
			return;
		}
	}

	if (!sep->IsNumber(2)) {
		c->Message(Chat::White, "Valid Red values for this command are 0 to 255.");
		return;
	}

	uint32 red_value = Strings::ToUnsignedInt(sep->arg[2]);
	if (red_value > 255) {
		red_value = 255;
	}

	if (!sep->IsNumber(3)) {
		c->Message(Chat::White, "Valid Green values for this command are 0 to 255.");
		return;
	}

	uint32 green_value = Strings::ToUnsignedInt(sep->arg[3]);
	if (green_value > 255) {
		green_value = 255;
	}

	if (!sep->IsNumber(4)) {
		c->Message(Chat::White, "Valid Blue values for this command are 0 to 255.");
		return;
	}

	uint32 blue_value = Strings::ToUnsignedInt(sep->arg[4]);
	if (blue_value > 255) {
		blue_value = 255;
	}

	uint32 rgb_value = ((uint32)red_value << 16) | ((uint32)green_value << 8) | ((uint32)blue_value);

	std::list<Bot*> sbl;
	auto ab_type = ActionableBots::PopulateSBL(c, sep->arg[5], sbl, ab_mask);
	if (ab_type == ActionableBots::ABT_None) {
		return;
	}

	for (auto bot_iter : sbl) {
		if (!bot_iter) {
			continue;
		}

		if (!bot_iter->DyeArmor(slot_id, rgb_value, dye_all, (ab_type != ActionableBots::ABT_All))) {
			c->Message(
				Chat::White,
				fmt::format(
					"Failed to change armor color for {} due to unknown cause.",
					bot_iter->GetCleanName()
				).c_str()
			);
			return;
		}
	}

	if (ab_type == ActionableBots::ABT_All) {
		if (dye_all) {
			if (!database.botdb.SaveAllArmorColors(c->CharacterID(), rgb_value)) {
				c->Message(
					Chat::White,
					fmt::format(
						"{}",
						BotDatabase::fail::SaveAllArmorColors()
					).c_str()
				);
			}
		} else {
			if (!database.botdb.SaveAllArmorColorBySlot(c->CharacterID(), slot_id, rgb_value)) {
				c->Message(
					Chat::White,
					fmt::format(
						"{}",
						BotDatabase::fail::SaveAllArmorColorBySlot()
					).c_str()
				);
			}
		}
	}
}
