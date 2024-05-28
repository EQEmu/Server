#include "../client.h"
#include "../worldserver.h"
#include "../../common/events/player_events.h"

extern WorldServer worldserver;

void command_parcels(Client* c, const Seperator* sep)
{
	const uint16 arguments = sep->argnum;
	if (!arguments) {
		SendParcelsSubCommands(c);
		return;
	}

	const bool is_add        = !strcasecmp(sep->arg[1], "add");
	const bool is_listdb     = !strcasecmp(sep->arg[1], "listdb");
	const bool is_listmemory = !strcasecmp(sep->arg[1], "listmemory");

	if (!is_add && !is_listdb && !is_listmemory) {
		SendParcelsSubCommands(c);
		return;
	}

	if (is_add) {
		if (arguments < 2 || !sep->IsNumber(4)) {
			SendParcelsSubCommands(c);
			return;
		}

		const std::string& to_name = sep->arg[2];
		uint32 item_id  = sep->IsNumber(3) ? Strings::ToUnsignedInt(sep->arg[3]) : 0;
		int16  quantity = static_cast<int16>(Strings::ToInt(sep->arg[4]));

		std::vector<uint32> augment_ids = { 0, 0, 0, 0, 0, 0 };

		if (!sep->IsNumber(3)) {
			const std::string& cmd_msg = sep->msg;

			const size_t link_open  = cmd_msg.find('\x12');
			const size_t link_close = cmd_msg.find_last_of('\x12');

			if (link_open != link_close && (cmd_msg.length() - link_open) > EQ::constants::SAY_LINK_BODY_SIZE) {
				EQ::SayLinkBody_Struct link_body;

				EQ::saylink::DegenerateLinkBody(
					link_body,
					cmd_msg.substr(link_open + 1, EQ::constants::SAY_LINK_BODY_SIZE)
				);

				item_id = link_body.item_id;

				augment_ids = {
					link_body.augment_1,
					link_body.augment_2,
					link_body.augment_3,
					link_body.augment_4,
					link_body.augment_5,
					link_body.augment_6
				};
			}
		}


		std::string note;
		uint16      note_index    = 5;
		uint16      augment_index = 0;

		if (arguments >= note_index) {
			while (sep->IsNumber(note_index)) {
				augment_ids[augment_index] = Strings::ToUnsignedInt(sep->arg[note_index]);

				augment_index++;
				note_index++;
			}

			note = sep->argplus[note_index];
		}

		auto send_to_client = CharacterParcelsRepository::GetParcelCountAndCharacterName(
			database,
			to_name
		);

		const std::string& character_name = send_to_client.front().character_name;
		const uint32 character_id = send_to_client.front().char_id;

		if (character_name.empty()) {
			c->MessageString(Chat::White, CANT_FIND_PLAYER, to_name.c_str());
			return;
		}

		const int free_slot = c->FindNextFreeParcelSlot(character_id);
		if (free_slot == INVALID_INDEX) {
			c->Message(
				Chat::White,
				fmt::format(
					"Unfortunately, {} cannot accept any more parcels at this time. Please try again later.",
					character_name
				).c_str()
			);
			return;
		}

		if (item_id == PARCEL_MONEY_ITEM_ID) {
			if (quantity > std::numeric_limits<int>::max()) {
				c->Message(
					Chat::White,
					fmt::format(
						"Your quantity of {} Copper was too large. Set to max quantity of {} Copper.",
						Strings::Commify(quantity),
						Strings::Commify(std::numeric_limits<int>::max())
					).c_str()
				);
				quantity = std::numeric_limits<int>::max();
			}

			auto item = database.GetItem(PARCEL_MONEY_ITEM_ID);
			if (!item) {
				c->Message(Chat::White, "Item ID {} does not exist.", item_id);
				return;
			}

			std::unique_ptr<EQ::ItemInstance> inst(database.CreateItem(item, 1));
			if (!inst) {
				c->Message(Chat::White, "Item ID {} does not exist.", item_id);
				return;
			}

			CharacterParcelsRepository::CharacterParcels p;
			p.from_name = c->GetName();
			p.note      = note;
			p.sent_date = time(nullptr);
			p.quantity  = quantity == 0 ? 1 : quantity;
			p.item_id   = PARCEL_MONEY_ITEM_ID;
			p.char_id   = character_id;
			p.slot_id   = free_slot;
			p.id        = 0;

			auto result = CharacterParcelsRepository::InsertOne(database, p);
			if (!result.id) {
				LogError(
					"Failed to add parcel to database. From {} to {} item {} with a quantity of {}.",
					p.from_name,
					character_name,
					p.item_id,
					p.quantity
				);
				c->Message(
					Chat::White,
					"Unable to save parcel to the database. Please contact an administrator."
				);
				return;
			}

			c->MessageString(
				Chat::White,
				PARCEL_DELIVERY,
				c->GetCleanName(),
				"Money",
				character_name.c_str()
			);

			if (player_event_logs.IsEventEnabled(PlayerEvent::PARCEL_SEND)) {
				PlayerEvent::ParcelSend e{ };
				e.from_player_name = p.from_name;
				e.to_player_name   = character_name;
				e.item_id          = p.item_id;
				e.quantity         = p.quantity;
				e.sent_date        = p.sent_date;

				RecordPlayerEventLogWithClient (c, PlayerEvent::PARCEL_SEND, e);
			}

			Parcel_Struct ps{ };
			ps.item_slot = p.slot_id;
			strn0cpy(ps.send_to, character_name.c_str(), sizeof(ps.send_to));

			c->SendParcelDeliveryToWorld(ps);
		} else {
			auto item = database.GetItem(item_id);
			if (!item) {
				c->Message(Chat::White, "Item ID {} does not exist.", item_id);
				return;
			}

			quantity = EQ::Clamp(
				quantity,
				std::numeric_limits<int16>::min(),
				std::numeric_limits<int16>::max()
			);

			std::unique_ptr<EQ::ItemInstance> inst(
				database.CreateItem(
					item,
					quantity,
					augment_ids[0],
					augment_ids[1],
					augment_ids[2],
					augment_ids[3],
					augment_ids[4],
					augment_ids[5]
				)
			);
			if (!inst) {
				c->Message(Chat::White, "Item ID {} does not exist.", item_id);
				return;
			}

			if (inst->IsStackable()) {
				quantity = EQ::ClampUpper(quantity, inst->GetItem()->StackSize);
			} else if (inst->GetItem()->MaxCharges > 0) {
				quantity = EQ::ClampUpper(quantity, inst->GetItem()->MaxCharges);
			}

			CharacterParcelsRepository::CharacterParcels p;
			p.from_name  = c->GetCleanName();
			p.note       = note;
			p.sent_date  = time(nullptr);
			p.quantity   = quantity;
			p.item_id    = item_id;
			p.aug_slot_1 = augment_ids[0];
			p.aug_slot_2 = augment_ids[1];
			p.aug_slot_3 = augment_ids[2];
			p.aug_slot_4 = augment_ids[3];
			p.aug_slot_5 = augment_ids[4];
			p.aug_slot_6 = augment_ids[5];
			p.char_id    = character_id;
			p.slot_id    = free_slot;
			p.id         = 0;

			auto result = CharacterParcelsRepository::InsertOne(database, p);
			if (!result.id) {
				const auto* item = database.CreateItem(
					p.item_id,
					p.quantity,
					p.aug_slot_1,
					p.aug_slot_2,
					p.aug_slot_3,
					p.aug_slot_4,
					p.aug_slot_5,
					p.aug_slot_6
				);
				if (!item) {
					c->Message(
						Chat::White,
						fmt::format(
							"Item ID {} in slot {} does not exist.",
							p.item_id,
							p.slot_id
						).c_str()
					);
					return;
				}

				EQ::SayLinkEngine linker;
				linker.SetLinkType(EQ::saylink::SayLinkItemInst);
				linker.SetItemInst(item);

				const std::string& item_link = linker.GenerateLink();

				LogError(
					"Failed to add parcel to database. From {} to {} item {} with a quantity of {}.",
					p.from_name,
					character_name,
					item_link,
					p.quantity
				);
				c->Message(
					Chat::White,
					"Unable to save parcel to the database. Please contact an administrator."
				);
				return;
			}

			c->MessageString(
				Chat::White,
				PARCEL_DELIVERY,
				c->GetCleanName(),
				inst->GetItem()->Name,
				character_name.c_str()
			);

			if (player_event_logs.IsEventEnabled(PlayerEvent::PARCEL_SEND)) {
				PlayerEvent::ParcelSend e{ };
				e.from_player_name = p.from_name;
				e.to_player_name   = character_name;
				e.item_id          = p.item_id;
				p.aug_slot_1       = augment_ids[0];
				p.aug_slot_2       = augment_ids[1];
				p.aug_slot_3       = augment_ids[2];
				p.aug_slot_4       = augment_ids[3];
				p.aug_slot_5       = augment_ids[4];
				p.aug_slot_6       = augment_ids[5];
				e.quantity         = p.quantity;
				e.sent_date        = p.sent_date;

				RecordPlayerEventLogWithClient(c, PlayerEvent::PARCEL_SEND, e);
			}

			Parcel_Struct ps{ };
			ps.item_slot = p.slot_id;
			strn0cpy(ps.send_to, character_name.c_str(), sizeof(ps.send_to));

			c->SendParcelDeliveryToWorld(ps);
		}
	} else if (is_listdb) {
		if (arguments < 2) {
			c->Message(Chat::White, "Usage: #parcels listdb [Character Name]");
			return;
		}

		const std::string& character_name = sep->arg[2];
		if (character_name.empty()) {
			c->Message(Chat::White, "You must provide a character name.");
			return;
		}

		auto player_id = CharacterParcelsRepository::GetParcelCountAndCharacterName(database, character_name);

		const uint32 character_id = player_id.at(0).char_id;
		if (!character_id) {
			c->MessageString(Chat::White, CANT_FIND_PLAYER, character_name.c_str());
			return;
		}

		auto results = CharacterParcelsRepository::GetWhere(
			database,
			fmt::format(
				"char_id = {} ORDER BY slot_id ASC",
				character_id
			)
		);

		if (results.empty()) {
			c->Message(
				Chat::White,
				fmt::format(
					"No parcels could be found for {}.",
					character_name
				).c_str()
			);
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Found {} parcel{} for {}.",
				results.size(),
				results.size() != 1 ? "s" : "",
				character_name
			).c_str()
		);

		for (auto const& p: results) {
			const auto* item = database.CreateItem(
				p.item_id,
				p.quantity,
				p.aug_slot_1,
				p.aug_slot_2,
				p.aug_slot_3,
				p.aug_slot_4,
				p.aug_slot_5,
				p.aug_slot_6
			);
			if (!item) {
				c->Message(
					Chat::White,
					fmt::format(
						"Item ID {} in slot {} does not exist.",
						p.item_id,
						p.slot_id
					).c_str()
				);
				return;
			}

			EQ::SayLinkEngine linker;
			linker.SetLinkType(EQ::saylink::SayLinkItemInst);
			linker.SetItemInst(item);

			const std::string& item_link = linker.GenerateLink();
			c->Message(
				Chat::White,
				fmt::format(
					"Slot {} contains {} with a quantity of {}.",
					p.slot_id,
					item_link,
					p.quantity
				).c_str()
			);
		}
	} else if (is_listmemory) {
		if (arguments < 2) {
			c->Message(Chat::White, "Usage: #parcels listmemory [Character Name] (Must be in the same zone)");
			return;
		}

		const std::string& character_name = sep->arg[2];
		Client           * character      = entity_list.GetClientByName(character_name.c_str());

		if (!character) {
			c->Message(
				Chat::White,
				fmt::format(
					"Player {} could not be found in this zone.  Ensure you are in the same zone as the player.",
					character_name
				).c_str()
			);
			return;
		}

		auto parcels = character->GetParcels();
		if (parcels.empty()) {
			c->Message(
				Chat::White,
				fmt::format(
					"No parcels could be found for {}.",
					character_name
				).c_str()
			);
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Found {} parcel{} for {}.",
				parcels.size(),
				parcels.size() != 1 ? "s" : "",
				character_name
			).c_str()
		);

		for (auto const& p: parcels) {
			const auto* item = database.CreateItem(
				p.second.item_id,
				p.second.quantity,
				p.second.aug_slot_1,
				p.second.aug_slot_2,
				p.second.aug_slot_3,
				p.second.aug_slot_4,
				p.second.aug_slot_5,
				p.second.aug_slot_6
			);
			if (!item) {
				c->Message(
					Chat::White,
					fmt::format(
						"Item ID {} in slot {} does not exist.",
						p.second.item_id,
						p.second.slot_id
					).c_str()
				);
				return;
			}

			EQ::SayLinkEngine linker;
			linker.SetLinkType(EQ::saylink::SayLinkItemInst);
			linker.SetItemInst(item);

			const std::string& item_link = linker.GenerateLink();
			c->Message(
				Chat::White,
				fmt::format(
					"Slot {} contains {} with a quantity of {}.",
					p.second.slot_id,
					item_link,
					p.second.quantity
				).c_str()
			);
		}
	}
}

void SendParcelsSubCommands(Client* c)
{
	c->Message(Chat::White, "#parcels listdb [Character Name]");
	c->Message(Chat::White, "#parcels listmemory [Character Name] (Must be in the same zone)");
	c->Message(Chat::White, "#parcels add [Character Name] [Item ID] [Quantity] [Note]");
	c->Message(Chat::White, "#parcels add [Character Name] [Item Link] [Quantity] [Note]");
	c->Message(Chat::White, "#parcels add [Character Name] [Item ID] [Quantity] [Augment One] [Note]");
	c->Message(Chat::White, "#parcels add [Character Name] [Item ID] [Quantity] [Augment One] [Augment Two] [Note]");
	c->Message(
		Chat::White,
		"#parcels add [Character Name] [Item ID] [Quantity] [Augment One] [Augment Two] [Augment Three] [Note]"
	);
	c->Message(
		Chat::White,
		"#parcels add [Character Name] [Item ID] [Quantity] [Augment One] [Augment Two] [Augment Three] [Augment Four] [Note]"
	);
	c->Message(
		Chat::White,
		"#parcels add [Character Name] [Item ID] [Quantity] [Augment One] [Augment Two] [Augment Three] [Augment Four] [Augment Five] [Note]"
	);
	c->Message(
		Chat::White,
		"#parcels add [Character Name] [Item ID] [Quantity] [Augment One] [Augment Two] [Augment Three] [Augment Four] [Augment Five] [Augment Six] [Note]"
	);
	c->Message(
		Chat::White,
		"Note: To send money use Item ID 99990. Quantity is valid for stackable items, charges on an item, or amount of Copper."
	);
	c->Message(
		Chat::White,
		"Note: If you want some augment slots empty and not others, you must provide a 0 for that augment slot's item ID"
	);
}
