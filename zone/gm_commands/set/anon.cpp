#include "../../client.h"
#include "../../../common/repositories/character_data_repository.h"

void command_setanon(Client *c, const Seperator *sep)
{
	auto arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1) || arguments > 2) {
		c->Message(Chat::White, "Usage: #setanon [Anonymous Flag]");
		c->Message(Chat::White, "Usage: #setanon [Character ID] [Anonymous Flag]");
		c->Message(Chat::White, "Note: 0 = Not Anonymous, 1 = Anonymous, 2 = Roleplaying");
		return;
	}

	if (arguments == 1) {
		const uint8 anon_flag = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[1]));
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
			c->Message(Chat::White, "Usage: #setanon [Character ID] [Anonymous Flag]");
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
	} else if (arguments == 2) {
		const auto character_id = Strings::ToInt(sep->arg[1]);
		const uint8 anon_flag = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));

		auto e = CharacterDataRepository::FindOne(content_db, character_id);
		if (!e.id) {
			c->Message(
				Chat::White,
				fmt::format(
					"Character ID {} does not exist or is invalid.",
					character_id
				).c_str()
			);
			return;
		}

		e.anon = anon_flag;

		auto updated = CharacterDataRepository::UpdateOne(content_db, e);

		if (!updated) {
			c->Message(
				Chat::White,
				fmt::format(
					"Failed to change Anonymous Flag for {} ({}).",
					e.name,
					character_id
				).c_str()
			);
			return;
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
			c->Message(Chat::White, "Usage: #setanon [Character ID] [Anonymous Flag]");
			c->Message(Chat::White, "Note: 0 = Not Anonymous, 1 = Anonymous, 2 = Roleplaying");
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"{} ({}) is {}.",
				e.name,
				character_id,
				anon_setting
			).c_str()
		);
	}
}
