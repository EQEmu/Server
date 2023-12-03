#ifndef EQEMU_OBJECT_MANIPULATION_H
#define EQEMU_OBJECT_MANIPULATION_H

#include "../client.h"

class ObjectManipulation {

public:
	static void CommandHandler(Client *c, const Seperator *sep);
	static void CommandHeader(Client *c);
	static void SendSubcommands(Client *c);
};


#endif //EQEMU_OBJECT_MANIPULATION_H
