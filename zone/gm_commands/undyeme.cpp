#include "../client.h"

void command_undyeme(Client *c, const Seperator *sep)
{
	c->Undye();
	c->Message(Chat::White, "Undyed armor for yourself.");
}
