#pragma once

#include <string>
#include <memory>

#include "world_connection.h"
#include "net/packet.h"
#include "eqemu_logsys.h"
#include "platform.h"
#include "crash.h"

#define EQRegisterService(type) EQEmuLogSys LogSys; \
int main(int argc, char **argv) { \
	LogSys.LoadLogSettingsDefaults(); \
	set_exception_handler(); \
	type srv; \
	srv.Run(); \
	return 0; \
} \

namespace EQ
{
	class Service
	{
	public:
		Service(const std::string &identifier, size_t heartbeat_duration_ms, size_t sleep_duration);
		virtual ~Service();
		
		void Run();
		
	protected:
		virtual void OnStart() = 0;
		virtual void OnStop() = 0;
		virtual void OnHeartbeat(double time_since_last) = 0;
		virtual void OnRoutedMessage(const std::string& filter, const std::string& identifier, const std::string& id, const EQ::Net::Packet& payload) = 0;
		
		void RouteMessage(const std::string &filter, const std::string &id, const EQ::Net::Packet& p);
		void Stop();
	private:
		struct Impl;
		std::unique_ptr<Impl> _impl;
	};
}
