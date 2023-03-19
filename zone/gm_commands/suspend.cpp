#include "../client.h"
#include "../worldserver.h"
#include "../../common/repositories/account_repository.h"

extern WorldServer worldserver;

void command_suspend(Client *c, const Seperator *sep)
{
	auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #suspend [Character Name] [Days] [Reason]");
		c->Message(Chat::White, "Note: Specify 0 days to lift a suspension");
		return;
	}

	const std::string character_name = Strings::ToLower(sep->arg[1]);
	auto days = std::stoul(sep->arg[2]);

	const std::string reason = sep->arg[3] ? sep->argplus[3] : "";

	auto l = AccountRepository::GetWhere(
		content_db,
		fmt::format(
			"LOWER(charname) = '{}'",
			Strings::Escape(character_name)
		)
	);

	if (l.empty()) {
		c->Message(
			Chat::White,
			fmt::format(
				"Character '{}' does not exist.",
				sep->arg[1]
			).c_str()
		);
		return;
	}

	l[0].status = -1;
	l[0].suspendeduntil = std::time(nullptr) + (days * 86400);
	l[0].suspend_reason = reason;

	if (!AccountRepository::UpdateOne(content_db, l[0])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to suspend {}.",
				character_name
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Account {} ({}) with the character {} {}.",
			l[0].name,
			l[0].id,
			character_name,
			(
				days ?
				fmt::format(
					"has been temporarily suspended for {} day{}.",
					days,
					days != 1 ? "s" : ""
				) :
				"is no longer suspended"
			)
		).c_str()
	);

	auto* b = entity_list.GetClientByName(character_name.c_str());

	if (b) {
		b->WorldKick();
		return;
	}

	auto pack = new ServerPacket(ServerOP_KickPlayer, sizeof(ServerKickPlayer_Struct));
	auto* k = (ServerKickPlayer_Struct *) pack->pBuffer;

	strn0cpy(k->adminname, c->GetName(), sizeof(k->adminname));
	strn0cpy(k->name, character_name.c_str(), sizeof(k->name));
	k->adminrank = c->Admin();

	worldserver.SendPacket(pack);

	safe_delete(pack);
}

