#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_zoneshutdown(Client *c, const Seperator *sep)
{
	const int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #zoneshutdown [Zone ID|Zone Short Name]");
		return;
	}

	if (!worldserver.Connected()) {
		c->Message(Chat::White, "World server disconnected.");
		return;
	}

	const uint32 zone_id = sep->IsNumber(1) ? Strings::ToUnsignedInt(sep->arg[1]) : ZoneID(sep->arg[1]);

	if (!zone_id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Zone '{}' does not exist.",
				sep->arg[1]
			).c_str()
		);
		return;
	}

	auto pack = new ServerPacket(ServerOP_ZoneShutdown, sizeof(ServerZoneStateChange_Struct));
	auto *s   = (ServerZoneStateChange_Struct *) pack->pBuffer;

	s->zone_id = zone_id;

	strn0cpy(s->admin_name, c->GetName(), sizeof(s->admin_name));

	worldserver.SendPacket(pack);
	safe_delete(pack);
}
