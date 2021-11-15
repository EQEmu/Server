#include "../client.h"

void command_path(Client *c, const Seperator *sep)
{
	if (zone->pathing) {
		zone->pathing->DebugCommand(c, sep);
	}
}

void Client::Undye()
{
	for (int cur_slot = EQ::textures::textureBegin; cur_slot <= EQ::textures::LastTexture; cur_slot++) {
		uint8            slot2 = SlotConvert(cur_slot);
		EQ::ItemInstance *inst = m_inv.GetItem(slot2);

		if (inst != nullptr) {
			inst->SetColor(inst->GetItem()->Color);
			database.SaveInventory(CharacterID(), inst, slot2);
		}

		m_pp.item_tint.Slot[cur_slot].Color = 0;
		SendWearChange(cur_slot);
	}

	database.DeleteCharacterDye(this->CharacterID());
}

