#pragma once

#include "../../common/service.h"

namespace EQ
{
	class Test2Service : public EQ::Service
	{
	public:
		Test2Service();
		virtual ~Test2Service();

	protected:
		virtual void OnStart();
		virtual void OnStop();
		virtual void OnHeartbeat(double time_since_last);
		virtual void OnRoutedMessage(const std::string& filter, const std::string& identifier, const std::string& id, const EQ::Net::Packet& payload);

	private:
		size_t bytes;
		size_t packets;
	};
}
