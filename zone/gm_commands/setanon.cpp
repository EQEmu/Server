#include "../client.h"

void command_setanon(Client *c, const Seperator *sep)
{
	auto arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #setanon [Anonymous Flag]");
		c->Message(Chat::White, "Note: 0 = Not Anonymous, 1 = Anonymous, 2 = Roleplaying");
		return;
	}

	const uint8 anon_flag = static_cast<uint8>(std::stoul(sep->arg[1]));
	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
		t = c->GetTarget()->CastToClient();
	}

	std::string anon_setting;
	if (anon_flag == Anonymity::NotAnonymous) {
		anon_setting = "no longer Anonymous or Roleplaying";
	} else if (anon_flag == Anonymity::Anonymous) {
		anon_setting = "now Anonymous";
	} else if (anon_flag == Anonymity::Roleplaying) {
		anon_setting = "now Roleplaying";
	} else {
		c->Message(Chat::White, "Usage: #setanon [Anonymous Flag]");
		c->Message(Chat::White, "Note: 0 = Not Anonymous, 1 = Anonymous, 2 = Roleplaying");
		return;
	}

	c->SetAnon(anon_flag);

	c->Message(
		Chat::White,
		fmt::format(
			"{} {} {}.",
			c->GetTargetDescription(t, TargetDescriptionType::UCYou),
			c == t ? "are" : "is",
			anon_setting
		).c_str()
	);
}
