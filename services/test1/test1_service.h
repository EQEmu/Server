#pragma once

#include "../../common/service.h"

namespace EQ
{
	class Test1Service : public EQ::Service
	{
	public:
		Test1Service();
		virtual ~Test1Service();

	protected:
		virtual void OnStart();
		virtual void OnStop();
		virtual void OnHeartbeat(double time_since_last);
		virtual void OnRoutedMessage(const std::string& filter, const std::string& identifier, const std::string& id, const EQ::Net::Packet& payload);
	};
}
