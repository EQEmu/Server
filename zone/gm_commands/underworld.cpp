#include "../client.h"

void command_underworld(Client *c, const Seperator *sep) {
	entity_list.GetUnderworldMobs(c);
}
