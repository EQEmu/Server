#pragma once

#include "map.h"
#include <list>

class Client;
class Seperator;

class IPathfinder
{
public:
	typedef std::list<glm::vec3> IPath;

	IPathfinder() { }
	virtual ~IPathfinder() { }

	virtual IPath FindRoute(const glm::vec3 &start, const glm::vec3 &end) = 0;
	virtual glm::vec3 GetRandomLocation() = 0;
	virtual void DebugCommand(Client *c, const Seperator *sep) = 0;

	static IPathfinder *Load(const std::string &zone);
};
