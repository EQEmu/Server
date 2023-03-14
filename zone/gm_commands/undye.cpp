#include "../client.h"

void command_undye(Client *c, const Seperator *sep)
{
	auto target = c;

	bool allowed=(c->GetGM() || c->GetTarget() == c);

	if (allowed) {
		target = c->GetTarget()->CastToClient();

		target->Undye();
		c->Message(
			Chat::White,
			fmt::format(
				"Undyed armor for {}.",
				c->GetTargetDescription(target)
			).c_str()
		);
	}
	else {
		c->Message(Chat::Red, "Only clients tagged as GM allowed to undye others");
	}
}
