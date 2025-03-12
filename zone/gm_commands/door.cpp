#include "../client.h"
#include "door_manipulation.h"

void command_door(Client *c, const Seperator *sep)
{
	DoorManipulation::CommandHandler(c, sep);
}

