#include "../client.h"

void command_giveitem(Client *c, const Seperator *sep)
{
	uint32      item_id       = 0;
	int16       charges       = -1;
	uint32      augment_one   = 0;
	uint32      augment_two   = 0;
	uint32      augment_three = 0;
	uint32      augment_four  = 0;
	uint32      augment_five  = 0;
	uint32      augment_six   = 0;
	int         arguments     = sep->argnum;
	std::string cmd_msg       = sep->msg;
	size_t      link_open     = cmd_msg.find('\x12');
	size_t      link_close    = cmd_msg.find_last_of('\x12');
	if (c->GetTarget()) {
		if (!c->GetTarget()->IsClient()) {
			c->Message(Chat::Red, "You can only give items to players with this command.");
			return;
		}

		if (link_open != link_close && (cmd_msg.length() - link_open) > EQ::constants::SAY_LINK_BODY_SIZE) {
			EQ::SayLinkBody_Struct link_body;
			EQ::saylink::DegenerateLinkBody(
				link_body,
				cmd_msg.substr(link_open + 1, EQ::constants::SAY_LINK_BODY_SIZE));
			item_id       = link_body.item_id;
			augment_one   = link_body.augment_1;
			augment_two   = link_body.augment_2;
			augment_three = link_body.augment_3;
			augment_four  = link_body.augment_4;
			augment_five  = link_body.augment_5;
			augment_six   = link_body.augment_6;
		}
		else if (sep->IsNumber(1)) {
			item_id = atoi(sep->arg[1]);
		}
		else if (!sep->IsNumber(1)) {
			c->Message(
				Chat::Red,
				"Usage: #giveitem [item id | link] [charges] [augment_one_id] [augment_two_id] [augment_three_id] [augment_four_id] [augment_five_id] [augment_six_id] (Charges are optional.)"
			);
			return;
		}

		Client             *client_target = c->GetTarget()->CastToClient();
		uint8              item_status    = 0;
		uint8              current_status = c->Admin();
		const EQ::ItemData *item          = database.GetItem(item_id);
		if (item) {
			item_status = item->MinStatus;
		}

		if (item_status > current_status) {
			c->Message(
				Chat::White,
				fmt::format(
					"Insufficient status to summon this item, current status is {}, required status is {}.",
					current_status,
					item_status
				).c_str()
			);
			return;
		}

		if (arguments >= 2 && sep->IsNumber(2)) {
			charges = atoi(sep->arg[2]);
		}

		if (arguments >= 3 && sep->IsNumber(3)) {
			augment_one = atoi(sep->arg[3]);
		}

		if (arguments >= 4 && sep->IsNumber(4)) {
			augment_two = atoi(sep->arg[4]);
		}

		if (arguments >= 5 && sep->IsNumber(5)) {
			augment_three = atoi(sep->arg[5]);
		}

		if (arguments >= 6 && sep->IsNumber(6)) {
			augment_four = atoi(sep->arg[6]);
		}

		if (arguments >= 7 && sep->IsNumber(7)) {
			augment_five = atoi(sep->arg[7]);
		}

		if (arguments == 8 && sep->IsNumber(8)) {
			augment_six = atoi(sep->arg[8]);
		}

		client_target->SummonItem(
			item_id,
			charges,
			augment_one,
			augment_two,
			augment_three,
			augment_four,
			augment_five,
			augment_six
		);
	}
	else {
		c->Message(Chat::Red, "You must target a client to give the item to.");
		return;
	}
}

