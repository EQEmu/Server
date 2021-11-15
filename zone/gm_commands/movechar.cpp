#include "../client.h"

void command_movechar(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0 || sep->arg[2][0] == 0) {
		c->Message(Chat::White, "Usage: #movechar [charactername] [zonename]");
	}
	else if (c->Admin() < commandMovecharToSpecials && strcasecmp(sep->arg[2], "cshome") == 0 ||
			 strcasecmp(sep->arg[2], "load") == 0 || strcasecmp(sep->arg[2], "load2") == 0) {
		c->Message(Chat::White, "Invalid zone name");
	}
	else {
		uint32 tmp = database.GetAccountIDByChar(sep->arg[1]);
		if (tmp) {
			if (c->Admin() >= commandMovecharSelfOnly || tmp == c->AccountID()) {
				if (!database.MoveCharacterToZone((char *) sep->arg[1], ZoneID(sep->arg[2]))) {
					c->Message(Chat::White, "Character Move Failed!");
				}
				else {
					c->Message(Chat::White, "Character has been moved.");
				}
			}
			else {
				c->Message(Chat::Red, "You cannot move characters that are not on your account.");
			}
		}
		else {
			c->Message(Chat::White, "Character Does Not Exist");
		}
	}
}

