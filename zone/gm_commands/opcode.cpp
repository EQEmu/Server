#include "../client.h"
#include "../../common/patches/patches.h"

void command_opcode(Client *c, const Seperator *sep)
{
	ReloadAllPatches();
	c->Message(Chat::White, "Opcodes for all patches have been reloaded");
}

