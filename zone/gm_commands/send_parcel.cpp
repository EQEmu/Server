#include "../client.h"
#include "../npc_scale_manager.h"

void command_send_parcel(Client *c, const Seperator *sep)
{
	const uint16 arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #send_parcel [Character ID|Character Name] [Item ID|Item Link] [Quantity] [Augment One] [Augment Two] [Augment Three] [Augment Four] [Augment Five] [Augment Six]");
		return;
	}

	std::string name         = sep->IsNumber(1) ? std::string() : sep->arg[1];
	uint32      character_id = sep->IsNumber(1) ? Strings::ToUnsignedInt(sep->arg[1]) : 0;
	std::string cmd_msg      = sep->msg;
	size_t      link_open    = cmd_msg.find('\x12');
	size_t      link_close   = cmd_msg.find_last_of('\x12');

	if (character_id) {
		const std::string& character_name = database.GetCharName(character_id);
		if (character_name.empty()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Character ID {} could not be found.",
					character_id
				).c_str()
			);
			return;
		}

		name = character_name;
	} else {
		const auto& v = CharacterParcelsRepository::GetParcelCountAndCharacterName(database, name);
		if (v.at(0).character_name.empty()) {
			return;
		}

		character_id = v.at(0).char_id;
	}

	if (!character_id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Character '{}' does not exist.",
				name
			).c_str()
		);
		return;
	}

	const int next_parcel_slot = CharacterParcelsRepository::GetNextFreeParcelSlot(database, character_id, RuleI(Parcel, ParcelMaxItems));
	if (next_parcel_slot == INVALID_INDEX) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} has no Parcel slots available.",
				name
			).c_str()
		);
		return;
	}

	uint32 item_id       = sep->IsNumber(2) ? Strings::ToUnsignedInt(sep->arg[2]): 0;
	int16  quantity      = sep->IsNumber(3) ? static_cast<int16>(Strings::ToInt(sep->arg[3])) : 1;
	uint32 augment_one   = sep->IsNumber(4) ? Strings::ToUnsignedInt(sep->arg[4]): 0;
	uint32 augment_two   = sep->IsNumber(5) ? Strings::ToUnsignedInt(sep->arg[5]): 0;
	uint32 augment_three = sep->IsNumber(6) ? Strings::ToUnsignedInt(sep->arg[6]): 0;
	uint32 augment_four  = sep->IsNumber(7) ? Strings::ToUnsignedInt(sep->arg[7]): 0;
	uint32 augment_five  = sep->IsNumber(8) ? Strings::ToUnsignedInt(sep->arg[8]): 0;
	uint32 augment_six   = sep->IsNumber(9) ? Strings::ToUnsignedInt(sep->arg[9]): 0;

	if (link_open != link_close && (cmd_msg.length() - link_open) > EQ::constants::SAY_LINK_BODY_SIZE) {
		EQ::SayLinkBody_Struct link_body;
		EQ::saylink::DegenerateLinkBody(link_body, cmd_msg.substr(link_open + 1, EQ::constants::SAY_LINK_BODY_SIZE));
		item_id       = link_body.item_id;
		augment_one   = link_body.augment_1;
		augment_two   = link_body.augment_2;
		augment_three = link_body.augment_3;
		augment_four  = link_body.augment_4;
		augment_five  = link_body.augment_5;
		augment_six   = link_body.augment_6;
	}

	auto e = CharacterParcelsRepository::NewEntity();

	e.char_id    = character_id;
	e.item_id    = item_id;
	e.aug_slot_1 = augment_one;
	e.aug_slot_2 = augment_two;
	e.aug_slot_3 = augment_three;
	e.aug_slot_4 = augment_four;
	e.aug_slot_5 = augment_five;
	e.aug_slot_6 = augment_six;
	e.slot_id    = next_parcel_slot;
	e.quantity   = quantity;
	e.from_name  = c->GetCleanName();
	e.sent_date  = std::time(nullptr);

	e = CharacterParcelsRepository::InsertOne(database, e);

	const auto* item = database.CreateItem(
		item_id,
		quantity,
		augment_one,
		augment_two,
		augment_three,
		augment_four,
		augment_five,
		augment_six
	);

	EQ::SayLinkEngine linker;
	linker.SetLinkType(EQ::saylink::SayLinkItemInst);
	linker.SetItemInst(item);

	const std::string& item_link = linker.GenerateLink();

	if (!e.id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to send {} to {}.",
				item_link,
				name
			).c_str()
		);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Successfully sent {} to {}.",
			item_link,
			name
		).c_str()
	);
}
