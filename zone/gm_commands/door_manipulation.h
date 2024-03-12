#ifndef EQEMU_DOOR_MANIPULATION_H
#define EQEMU_DOOR_MANIPULATION_H

#include "../client.h"
#include "../../common/repositories/tool_game_objects_repository.h"

class DoorManipulation {

public:
	static void CommandHandler(Client *c, const Seperator *sep);
	static void CommandHeader(Client *c);
	static void DisplayObjectResultToClient(
		Client *c,
		std::vector<ToolGameObjectsRepository::ToolGameObjects> game_objects
	);
	static void DisplayModelsFromFileResults(
		Client *c,
		std::vector<ToolGameObjectsRepository::ToolGameObjects> game_objects
	);
};


#endif //EQEMU_DOOR_MANIPULATION_H
