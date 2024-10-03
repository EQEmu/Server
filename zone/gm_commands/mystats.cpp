#include "../client.h"

void command_mystats(Client *c, const Seperator *sep)
{

	for (Mob* pet : c->GetAllPets()) {
		c->Message(Chat::White, "----- Displaying Stats & Inventory for [%s] -----", pet->GetCleanName());
		pet->ShowStats(c);

		// Ugly hack to look at pet equipment

		for (int i = EQ::invslot::EQUIPMENT_BEGIN; i <= EQ::invslot::EQUIPMENT_END; i++) {

			const EQ::ItemInstance *inst_main = nullptr;
			const EQ::ItemInstance *inst_sub  = nullptr;
			const EQ::ItemInstance *inst_aug  = nullptr;
			const EQ::ItemData     *item_data = nullptr;

			EQ::SayLinkEngine linker;
			linker.SetLinkType(EQ::saylink::SayLinkItemInst);

			inst_main = pet->GetInvPublic().GetItem(i);

			if (inst_main) {
				item_data  = inst_main->GetItem();
				linker.SetItemInst(inst_main);
			} else {
				item_data = nullptr;
			}
			if (i != EQ::invslot::slotCharm && i != EQ::invslot::slotPowerSource && i != EQ::invslot::slotAmmo) {
				std::string padded_string = fmt::format("[{:>10}]", EQ::invslot::GetInvPossessionsSlotName(i));
				std::replace(padded_string.begin(), padded_string.end(), ' ', '-');
				if (item_data) {
					padded_string = fmt::format("{}[{}]", padded_string, linker.GenerateLink());
				} else {
					padded_string = fmt::format("{}[{}]", padded_string, "<Empty>");
				}

				c->Message(
					Chat::White, padded_string.c_str()
				);
			}
		}
	}

	c->Message(Chat::White, "----- Displaying Stats & Inventory for [%s] -----", c->GetCleanName());
	c->ShowStats(c);
}