#include "../client.h"
#include "../titles.h"

void command_title(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0) {
		c->Message(
			Chat::White,
			"Usage: #title [remove|text] [1 = Create row in title table] - remove or set title to 'text'"
		);
	}
	else {
		bool Save = (atoi(sep->arg[2]) == 1);

		Mob *target_mob = c->GetTarget();
		if (!target_mob) {
			target_mob = c;
		}
		if (!target_mob->IsClient()) {
			c->Message(Chat::Red, "#title only works on players.");
			return;
		}
		Client *t = target_mob->CastToClient();

		if (strlen(sep->arg[1]) > 31) {
			c->Message(Chat::Red, "Title must be 31 characters or less.");
			return;
		}

		bool removed = false;
		if (!strcasecmp(sep->arg[1], "remove")) {
			t->SetAATitle("");
			removed = true;
		}
		else {
			for (unsigned int i = 0; i < strlen(sep->arg[1]); i++)
				if (sep->arg[1][i] == '_') {
					sep->arg[1][i] = ' ';
				}
			if (!Save) {
				t->SetAATitle(sep->arg[1]);
			}
			else {
				title_manager.CreateNewPlayerTitle(t, sep->arg[1]);
			}
		}

		t->Save();

		if (removed) {
			c->Message(Chat::Red, "%s's title has been removed.", t->GetName(), sep->arg[1]);
			if (t != c) {
				t->Message(Chat::Red, "Your title has been removed.", sep->arg[1]);
			}
		}
		else {
			c->Message(Chat::Red, "%s's title has been changed to '%s'.", t->GetName(), sep->arg[1]);
			if (t != c) {
				t->Message(Chat::Red, "Your title has been changed to '%s'.", sep->arg[1]);
			}
		}
	}
}


