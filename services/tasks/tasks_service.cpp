#include "tasks_service.h"
#include "../../common/eqemu_logsys.h"

EQ::TasksService::TasksService()
	: EQ::Service("Tasks", 100, 1)
{

}

EQ::TasksService::~TasksService() {

}

void EQ::TasksService::OnStart() {

}

void EQ::TasksService::OnStop() {

}

void EQ::TasksService::OnHeartbeat(double time_since_last) {

}

void EQ::TasksService::OnRoutedMessage(const std::string& identifier, int type, const EQ::Net::Packet& p) 
{
	LogF(Logs::General, Logs::World_Server, "Routed message of type {0} with length {1}", type, p.Length());
}

EQRegisterService(EQ::TasksService);
