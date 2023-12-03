#include "../client.h"
#include "object_manipulation.h"

void command_object(Client *c, const Seperator *sep)
{
	ObjectManipulation::CommandHandler(c, sep);
}
