#pragma once

#include "../../common/service.h"

namespace EQ
{
	class TasksService : public EQ::Service
	{
	public:
		TasksService();
		virtual ~TasksService();

	protected:
		virtual void OnStart();
		virtual void OnStop();
		virtual void OnHeartbeat(double time_since_last);
		virtual void OnRoutedMessage(const std::string& identifier, int type, const EQ::Net::Packet& p);
	};
}
