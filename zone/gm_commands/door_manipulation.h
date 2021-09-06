#ifndef EQEMU_DOOR_MANIPULATION_H
#define EQEMU_DOOR_MANIPULATION_H

#include "../client.h"

class DoorManipulation {

public:
	static void CommandHandler(Client *c, const Seperator *sep);
	static void CommandHeader(Client *c);
};


#endif //EQEMU_DOOR_MANIPULATION_H
