#include "test1_service.h"
#include "../../common/eqemu_logsys.h"
#include "../../common/eqemu_config.h"

EQ::Test1Service::Test1Service()
	: EQ::Service("Test1", 1, 1)
{

}

struct TestPacket
{
	int64_t f1;
	int64_t f2;
	int64_t f3;
	int64_t f4;
	int64_t f5;
	int64_t f6;
	int64_t f7;
	char f8[4092];

	template <class Archive>
	void serialize(Archive &ar)
	{
		ar(f1, f2, f3, f4, f5, f6, f7, f8);
	}
};

EQ::Test1Service::~Test1Service() {

}

void EQ::Test1Service::OnStart() {
	
}

void EQ::Test1Service::OnStop() {
}

void EQ::Test1Service::OnHeartbeat(double time_since_last) {
	TestPacket p;
	p.f1 = 33;
	p.f2 = 43;
	p.f3 = 56;
	p.f4 = 90;
	
	EQ::Net::DynamicPacket out;
	out.PutInt32(0, 1234);
	out.PutSerialize(4, p);
	
	for (int i = 0; i < 250; ++i) {
		RouteMessage("Test2", "", out);
	}
}

void EQ::Test1Service::OnRoutedMessage(const std::string& filter, const std::string& identifier, const std::string& id, const EQ::Net::Packet& payload)
{

}

EQRegisterService(EQ::Test1Service);
