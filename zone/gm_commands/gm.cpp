#include "../client.h"

void command_gm(Client *c, const Seperator *sep)
{
	bool   state = atobool(sep->arg[1]);
	Client *t    = c;

	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	if (sep->arg[1][0] != 0) {
		t->SetGM(state);
		c->Message(Chat::White, "%s is %s a GM.", t->GetName(), state ? "now" : "no longer");
	}
	else {
		c->Message(Chat::White, "Usage: #gm [on/off]");
	}
}

// there's no need for this, as /summon already takes care of it
// this command is here for reference but it is not added to the
// list above

//To whoever wrote the above: And what about /kill, /zone, /zoneserver, etc?
//There is a reason for the # commands: so that admins can specifically enable certain
//commands for their users. Some might want users to #summon but not to /kill. Cant do that if they are a GM
