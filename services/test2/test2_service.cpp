#include "test2_service.h"
#include "../../common/eqemu_logsys.h"
#include "../../common/eqemu_config.h"

EQ::Test2Service::Test2Service()
	: EQ::Service("Test2", 3000, 1)
{

}

EQ::Test2Service::~Test2Service() {

}

void EQ::Test2Service::OnStart() {
	bytes = 0;
	packets = 0;
}

void EQ::Test2Service::OnStop() {
}

void EQ::Test2Service::OnHeartbeat(double time_since_last) {
	
	auto bytes_per_sec = bytes / time_since_last;
	auto packets_per_sec = packets / time_since_last;
	printf("Transfer rate %.2f KB/sec %.2f Packets/sec\n", bytes_per_sec / 1000.0, packets_per_sec);

	bytes = 0;
	packets = 0;
}

void EQ::Test2Service::OnRoutedMessage(const std::string& filter, const std::string& identifier, const std::string& id, const EQ::Net::Packet& payload)
{
	bytes += sizeof(RouteToMessage);
	bytes += payload.Length();
	packets++;
}

EQRegisterService(EQ::Test2Service);
