#pragma once

#include "../servertalk.h"

namespace EQ
{
	namespace Net
	{
		enum ServertalkPacketType
		{
			ServertalkClientHello = 1,
			ServertalkServerHello = 2,
			ServertalkClientHandshake = 3,
			ServertalkMessage = 5,
		};
	}
}
