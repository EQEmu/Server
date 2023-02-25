#ifndef DBCORE_H
#define DBCORE_H

#ifdef _WINDOWS
#include <winsock2.h>
#include <windows.h>
#endif

#include "../common/mutex.h"
#include "../common/mysql_request_result.h"
#include "../common/types.h"

#include <mysql.h>
#include <string.h>
#include <mutex>

class DBcore {
public:
	enum eStatus {
		Closed, Connected, Error
	};

	DBcore();
	~DBcore();
	eStatus GetStatus() { return pStatus; }
	MySQLRequestResult QueryDatabase(const char *query, uint32 querylen, bool retryOnFailureOnce = true);
	MySQLRequestResult QueryDatabase(std::string query, bool retryOnFailureOnce = true);
	void TransactionBegin();
	void TransactionCommit();
	void TransactionRollback();
	std::string Escape(const std::string& s);
	uint32 DoEscapeString(char *tobuf, const char *frombuf, uint32 fromlen);
	void ping();

	const std::string &GetOriginHost() const;
	void SetOriginHost(const std::string &origin_host);

	bool DoesTableExist(std::string table_name);

	void SetMySQL(const DBcore &o)
	{
		mysql      = o.mysql;
		mysqlOwner = false;
	}
	void SetMutex(Mutex *mutex);

protected:
	bool Open(
		const char *iHost,
		const char *iUser,
		const char *iPassword,
		const char *iDatabase,
		uint32 iPort,
		uint32 *errnum = 0,
		char *errbuf = 0,
		bool iCompress = false,
		bool iSSL = false
	);

private:
	bool Open(uint32 *errnum = nullptr, char *errbuf = nullptr);

	MYSQL*  mysql;
	bool    mysqlOwner;
	Mutex   *m_mutex;
	eStatus pStatus;

	std::mutex m_query_lock{};

	std::string origin_host;

	char   *pHost;
	char   *pUser;
	char   *pPassword;
	char   *pDatabase;
	bool   pCompress;
	uint32 pPort;
	bool   pSSL;

};


#endif

