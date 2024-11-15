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
	QuestDB(Connection type) : QuestDB(type, false) {}
	QuestDB(Connection type, bool connect);
	QuestDB(const char* host, const char* user, const char* pass, const char* db, uint32_t port);

protected:
	// allow optional ownership of pointer to support using zone db connections
	struct Deleter
	{
		Deleter() : owner(true) {}
		Deleter(bool owner_) : owner(owner_) {}
		bool owner = true;
		void operator()(Database* ptr) noexcept;
	};

	std::unique_ptr<Database, Deleter> m_db;
};
