#pragma once

#include "../servertalk.h"

namespace EQ
{
	namespace Net
	{
		enum ServertalkPacketType
		{
			ServertalkClientHandshake,
			ServertalkMessage,
		};
	}
}
