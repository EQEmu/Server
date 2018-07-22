#pragma once

#include "map.h"
#include <list>

class Client;
class Seperator;

class IPathfinder
{
public:
	struct IPathNode
	{
		IPathNode(const glm::vec3 &p) {
			pos = p;
			teleport = false;
		}

		IPathNode(bool tp) {
			teleport = tp;
		}

		glm::vec3 pos;
		bool teleport;
	};

	typedef std::list<IPathNode> IPath;

	IPathfinder() { }
	virtual ~IPathfinder() { }

	virtual IPath FindRoute(const glm::vec3 &start, const glm::vec3 &end, bool &partial, bool &stuck) = 0;
	virtual glm::vec3 GetRandomLocation() = 0;
	virtual void DebugCommand(Client *c, const Seperator *sep) = 0;

	static IPathfinder *Load(const std::string &zone);
};
