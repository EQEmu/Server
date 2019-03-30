#include "eqstream_concurrent.h"

struct EQ::Net::ConcurrentEQStreamManager::Impl
{

};

EQ::Net::ConcurrentEQStreamManager::ConcurrentEQStreamManager(const EQStreamManagerInterfaceOptions &options)
	: EQStreamManagerInterface(options)
{
	_impl.reset(new Impl());
}

EQ::Net::ConcurrentEQStreamManager::~ConcurrentEQStreamManager()
{
}

void EQ::Net::ConcurrentEQStreamManager::OnNewConnection(std::function<void(std::shared_ptr<ConcurrentEQStream>)> func)
{
}

void EQ::Net::ConcurrentEQStreamManager::OnConnectionStateChange(std::function<void(std::shared_ptr<ConcurrentEQStream>, DbProtocolStatus, DbProtocolStatus)> func)
{
}

void EQ::Net::ConcurrentEQStreamManager::DaybreakNewConnection(std::shared_ptr<DaybreakConnection> connection)
{
}

void EQ::Net::ConcurrentEQStreamManager::DaybreakConnectionStateChange(std::shared_ptr<DaybreakConnection> connection, DbProtocolStatus from, DbProtocolStatus to)
{
}

void EQ::Net::ConcurrentEQStreamManager::DaybreakPacketRecv(std::shared_ptr<DaybreakConnection> connection, const Packet &p)
{
}
