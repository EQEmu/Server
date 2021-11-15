#include "../client.h"

void command_zopp(Client *c, const Seperator *sep)
{ // - Owner only command..non-targetable to eliminate malicious or mischievious activities.
	if (!c) {
		return;
	}
	else if (sep->argnum < 3 || sep->argnum > 4) {
		c->Message(Chat::White, "Usage: #zopp [trade/summon] [slot id] [item id] [*charges]");
	}
	else if (!strcasecmp(sep->arg[1], "trade") == 0 && !strcasecmp(sep->arg[1], "t") == 0 &&
			 !strcasecmp(sep->arg[1], "summon") == 0 && !strcasecmp(sep->arg[1], "s") == 0) {
		c->Message(Chat::White, "Usage: #zopp [trade/summon] [slot id] [item id] [*charges]");
	}
	else if (!sep->IsNumber(2) || !sep->IsNumber(3) || (sep->argnum == 4 && !sep->IsNumber(4))) {
		c->Message(Chat::White, "Usage: #zopp [trade/summon] [slot id] [item id] [*charges]");
	}
	else {
		ItemPacketType packettype;

		if (strcasecmp(sep->arg[1], "trade") == 0 || strcasecmp(sep->arg[1], "t") == 0) {
			packettype = ItemPacketTrade;
		}
		else {
			packettype = ItemPacketLimbo;
		}

		int16  slotid  = atoi(sep->arg[2]);
		uint32 itemid  = atoi(sep->arg[3]);
		int16  charges = sep->argnum == 4 ? atoi(sep->arg[4]) : 1; // defaults to 1 charge if not specified

		const EQ::ItemData *FakeItem = database.GetItem(itemid);

		if (!FakeItem) {
			c->Message(Chat::Red, "Error: Item [%u] is not a valid item id.", itemid);
			return;
		}

		int16              item_status = 0;
		const EQ::ItemData *item       = database.GetItem(itemid);
		if (item) {
			item_status = static_cast<int16>(item->MinStatus);
		}
		if (item_status > c->Admin()) {
			c->Message(Chat::Red, "Error: Insufficient status to use this command.");
			return;
		}

		if (charges < 0 || charges > FakeItem->StackSize) {
			c->Message(Chat::Red, "Warning: The specified charge count does not meet expected criteria!");
			c->Message(Chat::White, "Processing request..results may cause unpredictable behavior.");
		}

		EQ::ItemInstance *FakeItemInst = database.CreateItem(FakeItem, charges);
		c->SendItemPacket(slotid, FakeItemInst, packettype);
		c->Message(
			Chat::White, "Sending zephyr op packet to client - [%s] %s (%u) with %i %s to slot %i.",
			packettype == ItemPacketTrade ? "Trade" : "Summon", FakeItem->Name, itemid, charges,
			std::abs(charges == 1) ? "charge" : "charges", slotid
		);
		safe_delete(FakeItemInst);
	}
}

