#pragma once

#include "map.h"
#include <list>

class Client;
class Seperator;

enum PathingPolyFlags
{
	PathingNormal = 1,
	PathingWater = 2,
	PathingLava = 4,
	PathingZoneLine = 8,
	PathingPvP = 16,
	PathingSlime = 32,
	PathingIce = 64,
	PathingVWater = 128,
	PathingGeneralArea = 256,
	PathingPortal = 512,
	PathingPrefer = 1024,
	PathingDisabled = 2048,
	PathingAll = 65535,
	PathingNotDisabled = PathingAll ^ PathingDisabled
};

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

	virtual IPath FindRoute(const glm::vec3 &start, const glm::vec3 &end, bool &partial, bool &stuck, int flags = PathingNotDisabled) = 0;
	virtual glm::vec3 GetRandomLocation(const glm::vec3 &start) = 0;
	virtual void DebugCommand(Client *c, const Seperator *sep) = 0;

	static IPathfinder *Load(const std::string &zone);
};
