#pragma once

#include "../../common/service.h"
#include "tasks_database.h"

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
		virtual void OnRoutedMessage(const std::string& filter, const std::string& identifier, const std::string& id, const EQ::Net::Packet& payload);

	private:
		std::unique_ptr<TasksDatabase> m_db;
	};
}
