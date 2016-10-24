#pragma once

namespace EQ
{
	namespace Net
	{
		enum ServertalkPacketType
		{
			ServertalkClientHello = 1,
			ServertalkServerHello,
			ServertalkClientHandshake,
			ServertalkMessage,
		};
	}
}