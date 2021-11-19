#include "../client.h"
#include "door_manipulation.h"
#include "../doors.h"

void command_door(Client *c, const Seperator *sep)
{
	DoorManipulation::CommandHandler(c, sep);
}

