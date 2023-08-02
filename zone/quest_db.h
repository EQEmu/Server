#pragma once

#include <memory>

class Database;

// Base class for quest apis to manage connection to a MySQL database
class QuestDB
{
public:
	enum class Connection { Default = 0, Content };

	// Throws std::runtime_error on connection failure
	QuestDB() : QuestDB(Connection::Default) {}
	QuestDB(Connection type);
	QuestDB(const char* host, const char* user, const char* pass, const char* db, uint32_t port);

protected:
	std::unique_ptr<Database> m_db;
};
