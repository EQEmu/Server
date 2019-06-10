#include "tasks_service.h"
#include "../../common/eqemu_logsys.h"
#include "../../common/eqemu_config.h"

EQ::TasksService::TasksService()
	: EQ::Service("Tasks", 100, 1)
{

}

EQ::TasksService::~TasksService() {

}

void EQ::TasksService::OnStart() {
	Log(Logs::General, Logs::Status, "Connecting to database...");

	auto config = EQEmuConfig::get();

	m_db.reset(new TasksDatabase());

	auto r = m_db->Connect(
		config->DatabaseHost.c_str(),
		config->DatabaseUsername.c_str(),
		config->DatabasePassword.c_str(),
		config->DatabaseDB.c_str(),
		config->DatabasePort);

	if (false == r) {
		Log(Logs::General, Logs::Status, "Unable to connect to database.");
		Stop();
		return;
	}

	Log(Logs::General, Logs::Status, "Connected to database.");

	m_db->LoadLogSettings(LogSys.log_settings);
	LogSys.StartFileLogs();
}

void EQ::TasksService::OnStop() {
	m_db.release();
}

void EQ::TasksService::OnHeartbeat(double time_since_last) {

}

void EQ::TasksService::OnRoutedMessage(const std::string& filter, const std::string& identifier, const std::string& id, const EQ::Net::Packet& payload)
{
	LogF(Logs::General, Logs::Status, "Routed message from filter {0}, identifier {1}, id {2} with a payload of size {3}", filter, identifier, id, payload.Length());
}

EQRegisterService(EQ::TasksService);
