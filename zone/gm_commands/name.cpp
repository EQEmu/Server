#include "../client.h"

void command_name(Client *c, const Seperator *sep)
{
	Client *target;

	if ((strlen(sep->arg[1]) == 0) || (!(c->GetTarget() && c->GetTarget()->IsClient()))) {
		c->Message(Chat::White, "Usage: #name newname (requires player target)");
	}
	else {
		target = c->GetTarget()->CastToClient();
		char *oldname = strdup(target->GetName());
		if (target->ChangeFirstName(sep->arg[1], c->GetName())) {
			c->Message(Chat::White, "Successfully renamed %s to %s", oldname, sep->arg[1]);
			// until we get the name packet working right this will work
			c->Message(Chat::White, "Sending player to char select.");
			target->Kick("Name was changed");
		}
		else {
			c->Message(
				Chat::Red,
				"ERROR: Unable to rename %s. Check that the new name '%s' isn't already taken.",
				oldname,
				sep->arg[2]
			);
		}
		free(oldname);
	}
}

