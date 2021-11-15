#include "../client.h"

void command_goto(Client *c, const Seperator *sep)
{
	std::string arg1 = sep->arg[1];

	bool goto_via_target_no_args = sep->arg[1][0] == '\0' && c->GetTarget();
	bool goto_via_player_name    = !sep->IsNumber(1) && !arg1.empty();
	bool goto_via_x_y_z          = sep->IsNumber(1) && sep->IsNumber(2) && sep->IsNumber(3);

	if (goto_via_target_no_args) {
		c->MovePC(
			zone->GetZoneID(),
			zone->GetInstanceID(),
			c->GetTarget()->GetX(),
			c->GetTarget()->GetY(),
			c->GetTarget()->GetZ(),
			c->GetTarget()->GetHeading()
		);
	}
	else if (goto_via_player_name) {

		/**
		 * Find them in zone first
		 */
		const char  *player_name       = sep->arg[1];
		std::string player_name_string = sep->arg[1];
		Client      *client            = entity_list.GetClientByName(player_name);
		if (client) {
			c->MovePC(
				zone->GetZoneID(),
				zone->GetInstanceID(),
				client->GetX(),
				client->GetY(),
				client->GetZ(),
				client->GetHeading()
			);

			c->Message(Chat::Yellow, "Goto player '%s' same zone", player_name_string.c_str());
		}
		else if (c->GotoPlayer(player_name_string)) {
			c->Message(Chat::Yellow, "Goto player '%s' different zone", player_name_string.c_str());
		}
		else {
			c->Message(Chat::Yellow, "Player '%s' not found", player_name_string.c_str());
		}
	}
	else if (goto_via_x_y_z) {
		c->MovePC(
			zone->GetZoneID(),
			zone->GetInstanceID(),
			atof(sep->arg[1]),
			atof(sep->arg[2]),
			atof(sep->arg[3]),
			(sep->arg[4] ? atof(sep->arg[4]) : c->GetHeading())
		);
	}
	else {
		c->Message(Chat::White, "Usage: #goto [x y z] [h]");
		c->Message(Chat::White, "Usage: #goto [player_name]");
	}
}

