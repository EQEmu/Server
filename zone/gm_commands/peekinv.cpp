#include "../client.h"
#include "../object.h"

void command_peekinv(Client *c, const Seperator *sep)
{
	// this can be cleaned up once inventory is cleaned up
	enum {
		peekNone       = 0x0000,
		peekEquip      = 0x0001,
		peekGen        = 0x0002,
		peekCursor     = 0x0004,
		peekLimbo      = 0x0008,
		peekTrib       = 0x0010,
		peekBank       = 0x0020,
		peekShBank     = 0x0040,
		peekTrade      = 0x0080,
		peekWorld      = 0x0100,
		peekOutOfScope = (peekWorld * 2) // less than
	};

	static const char *scope_prefix[] = {"equip", "gen", "cursor", "limbo", "trib", "bank", "shbank", "trade", "world"};

	static const int16 scope_range[][2] = {
		{EQ::invslot::EQUIPMENT_BEGIN,   EQ::invslot::EQUIPMENT_END},
		{EQ::invslot::GENERAL_BEGIN,     EQ::invslot::GENERAL_END},
		{EQ::invslot::slotCursor,        EQ::invslot::slotCursor},
		{EQ::invslot::SLOT_INVALID,      EQ::invslot::SLOT_INVALID},
		{EQ::invslot::TRIBUTE_BEGIN,     EQ::invslot::TRIBUTE_END},
		{EQ::invslot::BANK_BEGIN,        EQ::invslot::BANK_END},
		{EQ::invslot::SHARED_BANK_BEGIN, EQ::invslot::SHARED_BANK_END},
		{EQ::invslot::TRADE_BEGIN,       EQ::invslot::TRADE_END},
		{EQ::invslot::SLOT_BEGIN,        (EQ::invtype::WORLD_SIZE - 1)}
	};

	static const bool scope_bag[] = {false, true, true, true, false, true, true, true, true};

	if (!c) {
		return;
	}

	if (c->GetTarget() && !c->GetTarget()->IsClient()) {
		c->Message(Chat::White, "You must target a PC for this command.");
		return;
	}

	int scopeMask = peekNone;

	if (strcasecmp(sep->arg[1], "all") == 0) { scopeMask = (peekOutOfScope - 1); }
	else if (strcasecmp(sep->arg[1], "equip") == 0) { scopeMask |= peekEquip; }
	else if (strcasecmp(sep->arg[1], "gen") == 0) { scopeMask |= peekGen; }
	else if (strcasecmp(sep->arg[1], "cursor") == 0) { scopeMask |= peekCursor; }
	else if (strcasecmp(sep->arg[1], "poss") == 0) { scopeMask |= (peekEquip | peekGen | peekCursor); }
	else if (strcasecmp(sep->arg[1], "limbo") == 0) { scopeMask |= peekLimbo; }
	else if (strcasecmp(sep->arg[1], "curlim") == 0) { scopeMask |= (peekCursor | peekLimbo); }
	else if (strcasecmp(sep->arg[1], "trib") == 0) { scopeMask |= peekTrib; }
	else if (strcasecmp(sep->arg[1], "bank") == 0) { scopeMask |= peekBank; }
	else if (strcasecmp(sep->arg[1], "shbank") == 0) { scopeMask |= peekShBank; }
	else if (strcasecmp(sep->arg[1], "allbank") == 0) { scopeMask |= (peekBank | peekShBank); }
	else if (strcasecmp(sep->arg[1], "trade") == 0) { scopeMask |= peekTrade; }
	else if (strcasecmp(sep->arg[1], "world") == 0) { scopeMask |= peekWorld; }

	if (!scopeMask) {
		c->Message(
			Chat::White,
			"Usage: #peekinv [equip|gen|cursor|poss|limbo|curlim|trib|bank|shbank|allbank|trade|world|all]"
		);
		c->Message(Chat::White, "- Displays a portion of the targeted user's inventory");
		c->Message(Chat::White, "- Caution: 'all' is a lot of information!");
		return;
	}

	Client *targetClient = c;
	if (c->GetTarget()) {
		targetClient = c->GetTarget()->CastToClient();
	}

	const EQ::ItemInstance *inst_main = nullptr;
	const EQ::ItemInstance *inst_sub  = nullptr;
	const EQ::ItemInstance *inst_aug  = nullptr;
	const EQ::ItemData     *item_data = nullptr;

	EQ::SayLinkEngine linker;
	linker.SetLinkType(EQ::saylink::SayLinkItemInst);

	c->Message(Chat::White, "Displaying inventory for %s...", targetClient->GetName());

	Object *objectTradeskill = targetClient->GetTradeskillObject();

	bool itemsFound = false;

	for (int scopeIndex = 0, scopeBit = peekEquip; scopeBit < peekOutOfScope; ++scopeIndex, scopeBit <<= 1) {
		if (scopeBit & ~scopeMask) {
			continue;
		}

		if (scopeBit & peekWorld) {
			if (objectTradeskill == nullptr) {
				c->Message(Chat::Default, "No world tradeskill object selected...");
				continue;
			}
			else {
				c->Message(
					Chat::White,
					"[WorldObject DBID: %i (entityid: %i)]",
					objectTradeskill->GetDBID(),
					objectTradeskill->GetID());
			}
		}

		for (int16 indexMain = scope_range[scopeIndex][0]; indexMain <= scope_range[scopeIndex][1]; ++indexMain) {
			if (indexMain == EQ::invslot::SLOT_INVALID) {
				continue;
			}

			inst_main = ((scopeBit & peekWorld) ? objectTradeskill->GetItem(indexMain) : targetClient->GetInv().GetItem(
				indexMain
			));
			if (inst_main) {
				itemsFound = true;
				item_data  = inst_main->GetItem();
			}
			else {
				item_data = nullptr;
			}

			linker.SetItemInst(inst_main);

			c->Message(
				(item_data == nullptr),
				"%sSlot: %i, Item: %i (%s), Charges: %i",
				scope_prefix[scopeIndex],
				((scopeBit & peekWorld) ? (EQ::invslot::WORLD_BEGIN + indexMain) : indexMain),
				((item_data == nullptr) ? 0 : item_data->ID),
				linker.GenerateLink().c_str(),
				((inst_main == nullptr) ? 0 : inst_main->GetCharges())
			);

			if (inst_main && inst_main->IsClassCommon()) {
				for (uint8 indexAug = EQ::invaug::SOCKET_BEGIN; indexAug <= EQ::invaug::SOCKET_END; ++indexAug) {
					inst_aug = inst_main->GetItem(indexAug);
					if (!inst_aug) { // extant only
						continue;
					}

					item_data = inst_aug->GetItem();
					linker.SetItemInst(inst_aug);

					c->Message(
						(item_data == nullptr),
						".%sAugSlot: %i (Slot #%i, Aug idx #%i), Item: %i (%s), Charges: %i",
						scope_prefix[scopeIndex],
						INVALID_INDEX,
						((scopeBit & peekWorld) ? (EQ::invslot::WORLD_BEGIN + indexMain) : indexMain),
						indexAug,
						((item_data == nullptr) ? 0 : item_data->ID),
						linker.GenerateLink().c_str(),
						((inst_sub == nullptr) ? 0 : inst_sub->GetCharges())
					);
				}
			}

			if (!scope_bag[scopeIndex] || !(inst_main && inst_main->IsClassBag())) {
				continue;
			}

			for (uint8 indexSub = EQ::invbag::SLOT_BEGIN; indexSub <= EQ::invbag::SLOT_END; ++indexSub) {
				inst_sub = inst_main->GetItem(indexSub);
				if (!inst_sub) { // extant only
					continue;
				}

				item_data = inst_sub->GetItem();
				linker.SetItemInst(inst_sub);

				c->Message(
					(item_data == nullptr),
					"..%sBagSlot: %i (Slot #%i, Bag idx #%i), Item: %i (%s), Charges: %i",
					scope_prefix[scopeIndex],
					((scopeBit & peekWorld) ? INVALID_INDEX : EQ::InventoryProfile::CalcSlotId(indexMain, indexSub)),
					((scopeBit & peekWorld) ? (EQ::invslot::WORLD_BEGIN + indexMain) : indexMain),
					indexSub,
					((item_data == nullptr) ? 0 : item_data->ID),
					linker.GenerateLink().c_str(),
					((inst_sub == nullptr) ? 0 : inst_sub->GetCharges())
				);

				if (inst_sub->IsClassCommon()) {
					for (uint8 indexAug = EQ::invaug::SOCKET_BEGIN; indexAug <= EQ::invaug::SOCKET_END; ++indexAug) {
						inst_aug = inst_sub->GetItem(indexAug);
						if (!inst_aug) { // extant only
							continue;
						}

						item_data = inst_aug->GetItem();
						linker.SetItemInst(inst_aug);

						c->Message(
							(item_data == nullptr),
							"...%sAugSlot: %i (Slot #%i, Sub idx #%i, Aug idx #%i), Item: %i (%s), Charges: %i",
							scope_prefix[scopeIndex],
							INVALID_INDEX,
							((scopeBit & peekWorld) ? INVALID_INDEX : EQ::InventoryProfile::CalcSlotId(
								indexMain,
								indexSub
							)),
							indexSub,
							indexAug,
							((item_data == nullptr) ? 0 : item_data->ID),
							linker.GenerateLink().c_str(),
							((inst_sub == nullptr) ? 0 : inst_sub->GetCharges())
						);
					}
				}
			}
		}

		if (scopeBit & peekLimbo) {
			int       limboIndex = 0;
			for (auto it         = targetClient->GetInv().cursor_cbegin();
				(it != targetClient->GetInv().cursor_cend());
				++it, ++limboIndex) {
				if (it == targetClient->GetInv().cursor_cbegin()) {
					continue;
				}

				inst_main = *it;
				if (inst_main) {
					itemsFound = true;
					item_data  = inst_main->GetItem();
				}
				else {
					item_data = nullptr;
				}

				linker.SetItemInst(inst_main);

				c->Message(
					(item_data == nullptr),
					"%sSlot: %i, Item: %i (%s), Charges: %i",
					scope_prefix[scopeIndex],
					(8000 + limboIndex),
					((item_data == nullptr) ? 0 : item_data->ID),
					linker.GenerateLink().c_str(),
					((inst_main == nullptr) ? 0 : inst_main->GetCharges())
				);

				if (inst_main && inst_main->IsClassCommon()) {
					for (uint8 indexAug = EQ::invaug::SOCKET_BEGIN; indexAug <= EQ::invaug::SOCKET_END; ++indexAug) {
						inst_aug = inst_main->GetItem(indexAug);
						if (!inst_aug) { // extant only
							continue;
						}

						item_data = inst_aug->GetItem();
						linker.SetItemInst(inst_aug);

						c->Message(
							(item_data == nullptr),
							".%sAugSlot: %i (Slot #%i, Aug idx #%i), Item: %i (%s), Charges: %i",
							scope_prefix[scopeIndex],
							INVALID_INDEX,
							(8000 + limboIndex),
							indexAug,
							((item_data == nullptr) ? 0 : item_data->ID),
							linker.GenerateLink().c_str(),
							((inst_sub == nullptr) ? 0 : inst_sub->GetCharges())
						);
					}
				}

				if (!scope_bag[scopeIndex] || !(inst_main && inst_main->IsClassBag())) {
					continue;
				}

				for (uint8 indexSub = EQ::invbag::SLOT_BEGIN; indexSub <= EQ::invbag::SLOT_END; ++indexSub) {
					inst_sub = inst_main->GetItem(indexSub);
					if (!inst_sub) {
						continue;
					}

					item_data = (inst_sub == nullptr) ? nullptr : inst_sub->GetItem();

					linker.SetItemInst(inst_sub);

					c->Message(
						(item_data == nullptr),
						"..%sBagSlot: %i (Slot #%i, Bag idx #%i), Item: %i (%s), Charges: %i",
						scope_prefix[scopeIndex],
						INVALID_INDEX,
						(8000 + limboIndex),
						indexSub,
						((item_data == nullptr) ? 0 : item_data->ID),
						linker.GenerateLink().c_str(),
						((inst_sub == nullptr) ? 0 : inst_sub->GetCharges())
					);

					if (inst_sub->IsClassCommon()) {
						for (uint8 indexAug = EQ::invaug::SOCKET_BEGIN;
							indexAug <= EQ::invaug::SOCKET_END;
							++indexAug) {
							inst_aug = inst_sub->GetItem(indexAug);
							if (!inst_aug) { // extant only
								continue;
							}

							item_data = inst_aug->GetItem();
							linker.SetItemInst(inst_aug);

							c->Message(
								(item_data == nullptr),
								"...%sAugSlot: %i (Slot #%i, Sub idx #%i, Aug idx #%i), Item: %i (%s), Charges: %i",
								scope_prefix[scopeIndex],
								INVALID_INDEX,
								(8000 + limboIndex),
								indexSub,
								indexAug,
								((item_data == nullptr) ? 0 : item_data->ID),
								linker.GenerateLink().c_str(),
								((inst_sub == nullptr) ? 0 : inst_sub->GetCharges())
							);
						}
					}
				}
			}
		}
	}

	if (!itemsFound) {
		c->Message(Chat::White, "No items found.");
	}
}

