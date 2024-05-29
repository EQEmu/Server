#include "../client.h"

void command_emptyinventory(Client *c, const Seperator *sep)
{
	Client* t = c;
	if (c->GetGM() && c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	EQ::ItemInstance* item = nullptr;

	int16 removed_count = 0;

	for (const auto& slot_id : EQ::InventoryProfile::GetInventorySlotIDs()) {
		item = t->GetInv().GetItem(slot_id);
		if (item) {
			int16 stack_size = std::max(item->GetCharges(), static_cast<int16>(1));
			removed_count += stack_size;
			t->DeleteItemInInventory(slot_id, 0, true);
		}
	}

	if (!removed_count) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} no items to delete.",
				c->GetTargetDescription(t, TargetDescriptionType::UCYou),
				c == t ? "have" : "has"
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Inventory cleared for {}, {} item{} deleted.",
			c->GetTargetDescription(t),
			removed_count,
			removed_count != 1 ? "s" : ""
		).c_str()
	);
}
