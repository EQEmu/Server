#include "quest_db.h"
#include "zonedb.h"
#include "zone_config.h"

// New connections avoid concurrency issues and allow use of unbuffered results
// with prepared statements. Using zone connections w/o buffering would cause
// "Commands out of sync" errors if any queries occur before results consumed.
QuestDB::QuestDB(Connection type)
	: m_db(std::make_unique<Database>())
{
	const auto config = EQEmuConfig::get();

	if (type == Connection::Default || type == Connection::Content && config->ContentDbHost.empty())
	{
		m_db->Connect(config->DatabaseHost, config->DatabaseUsername, config->DatabasePassword,
		              config->DatabaseDB, config->DatabasePort, "questdb");
	}
	else if (type == Connection::Content)
	{
		m_db->Connect(config->ContentDbHost, config->ContentDbUsername, config->ContentDbPassword,
		              config->ContentDbName, config->ContentDbPort, "questdb");
	}

	if (m_db->GetStatus() != DBcore::Connected)
	{
		throw std::runtime_error(fmt::format("Failed to connect to db type [{}]", static_cast<int>(type)));
	}
}

QuestDB::QuestDB(const char* host, const char* user, const char* pass, const char* db, uint32_t port)
	: m_db(std::make_unique<Database>())
{
	if (!m_db->Connect(host, user, pass, db, port, "questdb"))
	{
		throw std::runtime_error(fmt::format("Failed to connect to db [{}:{}]", host, port));
	}
}
