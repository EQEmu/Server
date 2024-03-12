#include "../client.h"

void command_emptyinventory(Client *c, const Seperator *sep)
{
	auto target = c;
	if (c->GetGM() && c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	EQ::ItemInstance *item = nullptr;
	static const int16 slots[][2] = {
		{ EQ::invslot::POSSESSIONS_BEGIN, EQ::invslot::POSSESSIONS_END },
		{ EQ::invbag::GENERAL_BAGS_BEGIN, EQ::invbag::GENERAL_BAGS_END },
		{ EQ::invbag::CURSOR_BAG_BEGIN, EQ::invbag::CURSOR_BAG_END},
		{ EQ::invslot::BANK_BEGIN, EQ::invslot::BANK_END },
		{ EQ::invbag::BANK_BAGS_BEGIN, EQ::invbag::BANK_BAGS_END },
		{ EQ::invslot::SHARED_BANK_BEGIN, EQ::invslot::SHARED_BANK_END },
		{ EQ::invbag::SHARED_BANK_BAGS_BEGIN, EQ::invbag::SHARED_BANK_BAGS_END },
	};
	int removed_count = 0;
	const size_t size = sizeof(slots) / sizeof(slots[0]);
	for (int slot_index = 0; slot_index < size; ++slot_index) {
		for (int slot_id = slots[slot_index][0]; slot_id <= slots[slot_index][1]; ++slot_id) {
			item = target->GetInv().GetItem(slot_id);
			if (item) {
				int stack_size = std::max(static_cast<int>(item->GetCharges()), 1);
				removed_count += stack_size;
				target->DeleteItemInInventory(slot_id, 0, true);
			}
		}
	}

	if (!removed_count) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} no items to delete.",
				c->GetTargetDescription(target, TargetDescriptionType::UCYou),
				c == target ? "have" : "has"
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Inventory cleared for {}, {} item{} deleted.",
			c->GetTargetDescription(target),
			removed_count,
			removed_count != 1 ? "s" : ""
		).c_str()
	);
}