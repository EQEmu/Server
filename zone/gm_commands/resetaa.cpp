#include "../client.h"

void command_resetaa(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsClient()) {
		c->Message(Chat::White, "You must target a player to use this command.");
		return;
	}

	const uint16 arguments = sep->argnum;

	const bool is_aa         = !strcasecmp(sep->arg[1], "aa");
	const bool is_leadership = !strcasecmp(sep->arg[1], "leadership");

	if (!is_aa && !is_leadership) {
		c->Message(Chat::White, "Usage: #resetaa aa - Resets and refunds a player's AAs");
		c->Message(Chat::White, "Usage: #resetaa leadership - Resets a player's Leadership AAs");
		return;
	}

	Client* t = c->GetTarget()->CastToClient();

	if (is_aa) {
		t->ResetAA();
	} else if (is_leadership) {
		t->ResetLeadershipAA();
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Successfully reset all{} AAs for {}.",
			is_aa ? "" : " Leadership",
			c->GetTargetDescription(t)
		).c_str()
	);
}
