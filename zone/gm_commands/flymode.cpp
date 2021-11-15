#include "../client.h"

void command_flymode(Client *c, const Seperator *sep)
{
	Mob *t = c;

	if (strlen(sep->arg[1]) == 1 && sep->IsNumber(1) && atoi(sep->arg[1]) >= 0 && atoi(sep->arg[1]) <= 5) {
		if (c->GetTarget()) {
			t = c->GetTarget();
		}

		int fm = atoi(sep->arg[1]);

		t->SetFlyMode(static_cast<GravityBehavior>(fm));
		t->SendAppearancePacket(AT_Levitate, fm);
		if (sep->arg[1][0] == '0') {
			c->Message(Chat::White, "Setting %s to Grounded", t->GetName());
		}
		else if (sep->arg[1][0] == '1') {
			c->Message(Chat::White, "Setting %s to Flying", t->GetName());
		}
		else if (sep->arg[1][0] == '2') {
			c->Message(Chat::White, "Setting %s to Levitating", t->GetName());
		}
		else if (sep->arg[1][0] == '3') {
			c->Message(Chat::White, "Setting %s to In Water", t->GetName());
		}
		else if (sep->arg[1][0] == '4') {
			c->Message(Chat::White, "Setting %s to Floating(Boat)", t->GetName());
		}
		else if (sep->arg[1][0] == '5') {
			c->Message(Chat::White, "Setting %s to Levitating While Running", t->GetName());
		}
	}
	else {
		c->Message(Chat::White, "#flymode [0/1/2/3/4/5]");
	}
}


