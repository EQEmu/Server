#include "../client.h"

void command_dye(Client *c, const Seperator *sep)
{
	const uint16 arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1) || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #dye help");
		c->Message(Chat::White, "Usage: #dye [slot] [red] [green] [blue]");
		c->Message(Chat::White, "Note: Red, Green, and Blue go from 0 to 255.");

		std::vector<std::string> dye_slots = {
			"Helmet",
			"Chest",
			"Arms",
			"Wrist",
			"Hands",
			"Legs",
			"Feet"
		};

		int                      slot_id = 0;
		std::vector<std::string> slot_messages;

		for (const auto& slot : dye_slots) {
			slot_messages.emplace_back(
				fmt::format(
					"({}) {}",
					slot_id,
					slot
				)
			);
			slot_id++;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Slots are as follows: {}",
				Strings::Implode(", ", slot_messages)
			).c_str()
		);
		return;
	}

	uint8 slot     = sep->IsNumber(1) ? static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[1])) : 0;
	uint8 red      = sep->IsNumber(2) ? static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2])) : std::numeric_limits<uint8>::max();
	uint8 green    = sep->IsNumber(3) ? static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[3])) : std::numeric_limits<uint8>::max();
	uint8 blue     = sep->IsNumber(4) ? static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[4])) : std::numeric_limits<uint8>::max();

	if (RuleB(Command, DyeCommandRequiresDyes)) {
		if (c->CountItem(Item::ID::PrismaticDye) >= 1) {
			c->RemoveItem(Item::ID::PrismaticDye);
		} else {
			const std::string& item_link = database.CreateItemLink(Item::ID::PrismaticDye);
			c->Message(Chat::White, fmt::format("This command requires a {} to use.", item_link).c_str());
			return;
		}
	}

	c->DyeArmorBySlot(slot, red, green, blue, std::numeric_limits<uint8>::max());
}

