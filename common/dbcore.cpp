#ifdef _WINDOWS
#include <winsock2.h>
#endif

#include "misc_functions.h"
#include "eqemu_logsys.h"
#include "timer.h"

#include "dbcore.h"

#include <errmsg.h>
#include <fstream>
#include <iostream>
#include <mysqld_error.h>
#include <string.h>

#ifdef _WINDOWS
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#include <process.h>
#else

#include "unix.h"
#include <pthread.h>

#endif

#ifdef _EQDEBUG
#define DEBUG_MYSQL_QUERIES 0
#else
#define DEBUG_MYSQL_QUERIES 0
#endif

DBcore::DBcore()
{
	mysql_init(&mysql);
	pHost           = nullptr;
	pUser           = nullptr;
	pPassword       = nullptr;
	pDatabase       = nullptr;
	pCompress       = false;
	pSSL            = false;
	pStatus         = Closed;
}

DBcore::~DBcore()
{
	/**
	 * This prevents us from doing a double free in multi-tenancy setups where we
	 * are re-using the default database connection pointer when we dont have an
	 * external configuration setup ex: (content_database)
	 */
	std::string mysql_connection_host;
	if (mysql.host) {
		mysql_connection_host = mysql.host;
	}

	if (GetOriginHost() != mysql_connection_host) {
		return;
	}

	mysql_close(&mysql);
	safe_delete_array(pHost);
	safe_delete_array(pUser);
	safe_delete_array(pPassword);
	safe_delete_array(pDatabase);
}

// Sends the MySQL server a keepalive
void DBcore::ping()
{
	if (!MDatabase.trylock()) {
		// well, if's it's locked, someone's using it. If someone's using it, it doesnt need a keepalive
		return;
	}
	mysql_ping(&mysql);
	MDatabase.unlock();
}

MySQLRequestResult DBcore::QueryDatabase(std::string query, bool retryOnFailureOnce)
{
	return QueryDatabase(query.c_str(), query.length(), retryOnFailureOnce);
}

bool DBcore::DoesTableExist(std::string table_name)
{
	auto results = QueryDatabase(fmt::format("SHOW TABLES LIKE '{}'", table_name));

	return results.RowCount() > 0;
}

MySQLRequestResult DBcore::QueryDatabase(const char *query, uint32 querylen, bool retryOnFailureOnce)
{
	BenchTimer timer;
	timer.reset();

	LockMutex lock(&MDatabase);

	// Reconnect if we are not connected before hand.
	if (pStatus != Connected) {
		Open();
	}



	// request query. != 0 indicates some kind of error.
	if (mysql_real_query(&mysql, query, querylen) != 0) {
		unsigned int errorNumber = mysql_errno(&mysql);

		if (errorNumber == CR_SERVER_GONE_ERROR) {
			pStatus = Error;
		}

		// error appears to be a disconnect error, may need to try again.
		if (errorNumber == CR_SERVER_LOST || errorNumber == CR_SERVER_GONE_ERROR) {

			if (retryOnFailureOnce) {
				LogInfo("Database Error: Lost connection, attempting to recover");
				MySQLRequestResult requestResult = QueryDatabase(query, querylen, false);

				if (requestResult.Success()) {
					LogInfo("Reconnection to database successful");
					return requestResult;
				}

			}

			pStatus = Error;

			auto errorBuffer = new char[MYSQL_ERRMSG_SIZE];

			snprintf(errorBuffer, MYSQL_ERRMSG_SIZE, "#%i: %s", mysql_errno(&mysql), mysql_error(&mysql));

			return MySQLRequestResult(nullptr, 0, 0, 0, 0, (uint32) mysql_errno(&mysql), errorBuffer);
		}

		auto errorBuffer = new char[MYSQL_ERRMSG_SIZE];
		snprintf(errorBuffer, MYSQL_ERRMSG_SIZE, "#%i: %s", mysql_errno(&mysql), mysql_error(&mysql));

		/**
		 * Error logging
		 */
		if (mysql_errno(&mysql) > 0 && strlen(query) > 0) {
			LogMySQLError("[{}] [{}]\n[{}]", mysql_errno(&mysql), mysql_error(&mysql), query);
		}

		return MySQLRequestResult(nullptr, 0, 0, 0, 0, mysql_errno(&mysql), errorBuffer);
	}

	// successful query. get results.
	MYSQL_RES *res     = mysql_store_result(&mysql);
	uint32    rowCount = 0;

	if (res != nullptr) {
		rowCount = (uint32) mysql_num_rows(res);
	}

	MySQLRequestResult requestResult(
		res,
		(uint32) mysql_affected_rows(&mysql),
		rowCount,
		(uint32) mysql_field_count(&mysql),
		(uint32) mysql_insert_id(&mysql)
	);

	if (LogSys.log_settings[Logs::MySQLQuery].is_category_enabled == 1) {
		if ((strncasecmp(query, "select", 6) == 0)) {
			LogMySQLQuery(
				"{0}; -- ({1} row{2} returned) ({3}s)",
				query,
				requestResult.RowCount(),
				requestResult.RowCount() == 1 ? "" : "s",
				std::to_string(timer.elapsed())
			);
		}
		else {
			LogMySQLQuery(
				"{0}; -- ({1} row{2} affected) ({3}s)",
				query,
				requestResult.RowsAffected(),
				requestResult.RowsAffected() == 1 ? "" : "s",
				std::to_string(timer.elapsed())
			);
		}
	}

	return requestResult;
}

void DBcore::TransactionBegin()
{
	QueryDatabase("START TRANSACTION");
}

void DBcore::TransactionCommit()
{
	QueryDatabase("COMMIT");
}

void DBcore::TransactionRollback()
{
	QueryDatabase("ROLLBACK");
}

uint32 DBcore::DoEscapeString(char *tobuf, const char *frombuf, uint32 fromlen)
{
//	No good reason to lock the DB, we only need it in the first place to check char encoding.
//	LockMutex lock(&MDatabase);
	return mysql_real_escape_string(&mysql, tobuf, frombuf, fromlen);
}

bool DBcore::Open(
	const char *iHost,
	const char *iUser,
	const char *iPassword,
	const char *iDatabase,
	uint32 iPort,
	uint32 *errnum,
	char *errbuf,
	bool iCompress,
	bool iSSL
)
{
	LockMutex lock(&MDatabase);
	safe_delete_array(pHost);
	safe_delete_array(pUser);
	safe_delete_array(pPassword);
	safe_delete_array(pDatabase);
	pHost     = strcpy(new char[strlen(iHost) + 1], iHost);
	pUser     = strcpy(new char[strlen(iUser) + 1], iUser);
	pPassword = strcpy(new char[strlen(iPassword) + 1], iPassword);
	pDatabase = strcpy(new char[strlen(iDatabase) + 1], iDatabase);
	pCompress = iCompress;
	pPort     = iPort;
	pSSL      = iSSL;
	return Open(errnum, errbuf);
}

bool DBcore::Open(uint32 *errnum, char *errbuf)
{
	if (errbuf) {
		errbuf[0] = 0;
	}
	LockMutex lock(&MDatabase);
	if (GetStatus() == Connected) {
		return true;
	}
	if (GetStatus() == Error) {
		mysql_close(&mysql);
		mysql_init(&mysql);        // Initialize structure again
	}
	if (!pHost) {
		return false;
	}
	/*
	Added CLIENT_FOUND_ROWS flag to the connect
	otherwise DB update calls would say 0 rows affected when the value already equalled
	what the function was tring to set it to, therefore the function would think it failed
	*/
	uint32 flags = CLIENT_FOUND_ROWS;
	if (pCompress) {
		flags |= CLIENT_COMPRESS;
	}
	if (pSSL) {
		flags |= CLIENT_SSL;
	}
	if (mysql_real_connect(&mysql, pHost, pUser, pPassword, pDatabase, pPort, 0, flags)) {
		pStatus = Connected;

		std::string connected_origin_host = pHost;
		SetOriginHost(connected_origin_host);

		return true;
	}
	else {
		if (errnum) {
			*errnum = mysql_errno(&mysql);
		}
		if (errbuf) {
			snprintf(errbuf, MYSQL_ERRMSG_SIZE, "#%i: %s", mysql_errno(&mysql), mysql_error(&mysql));
		}
		pStatus = Error;
		return false;
	}
}

void DBcore::SetMysql(MYSQL *mysql)
{
	DBcore::mysql = *mysql;
}

const std::string &DBcore::GetOriginHost() const
{
	return origin_host;
}

void DBcore::SetOriginHost(const std::string &origin_host)
{
	DBcore::origin_host = origin_host;
}
