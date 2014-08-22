#include "../common/debug.h"

#ifdef _WINDOWS
#include <winsock2.h>
#endif

#include <iostream>
#include <errmsg.h>
#include <mysqld_error.h>
#include <limits.h>
#include "dbcore.h"
#include <string.h>
#include "../common/misc_functions.h"
#include <cstdlib>

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

DBcore::DBcore() {
	mysql_init(&mysql);
	pHost = 0;
	pUser = 0;
	pPassword = 0;
	pDatabase = 0;
	pCompress = false;
	pSSL = false;
	pStatus = Closed;
}

DBcore::~DBcore() {
	mysql_close(&mysql);
	safe_delete_array(pHost);
	safe_delete_array(pUser);
	safe_delete_array(pPassword);
	safe_delete_array(pDatabase);
}

// Sends the MySQL server a keepalive
void DBcore::ping() {
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

MySQLRequestResult DBcore::QueryDatabase(const char* query, uint32 querylen, bool retryOnFailureOnce)
{
	LockMutex lock(&MDatabase);

	// Reconnect if we are not connected before hand.
	if (pStatus != Connected)
		Open();

	// request query. != 0 indicates some kind of error.
	if (mysql_real_query(&mysql, query, querylen) != 0)
	{
		unsigned int errorNumber = mysql_errno(&mysql);

		if (errorNumber == CR_SERVER_GONE_ERROR)
			pStatus = Error;

		// error appears to be a disconnect error, may need to try again.
		if (errorNumber == CR_SERVER_LOST || errorNumber == CR_SERVER_GONE_ERROR)
		{

			if (retryOnFailureOnce)
			{
				std::cout << "Database Error: Lost connection, attempting to recover...." << std::endl;
				MySQLRequestResult requestResult = QueryDatabase(query, querylen, false);

				if (requestResult.Success())
				{
					std::cout << "Reconnection to database successful." << std::endl;
					return requestResult;
				}

			}

			pStatus = Error;

			char *errorBuffer = new char[MYSQL_ERRMSG_SIZE];

			snprintf(errorBuffer, MYSQL_ERRMSG_SIZE, "#%i: %s", mysql_errno(&mysql), mysql_error(&mysql));

			std::cout << "DB Query Error #" << mysql_errno(&mysql) << ": " << mysql_error(&mysql) << std::endl;

			return MySQLRequestResult(nullptr, 0, 0, 0, 0, (uint32)mysql_errno(&mysql), errorBuffer);
		}

		char *errorBuffer = new char[MYSQL_ERRMSG_SIZE];
		snprintf(errorBuffer, MYSQL_ERRMSG_SIZE, "#%i: %s", mysql_errno(&mysql), mysql_error(&mysql));

#ifdef _EQDEBUG
		std::cout << "DB Query Error #" << mysql_errno(&mysql) << ": " << mysql_error(&mysql) << std::endl;
#endif
		return MySQLRequestResult(nullptr, 0, 0, 0, 0, mysql_errno(&mysql),errorBuffer);

	}

	// successful query. get results.
	MYSQL_RES* res = mysql_store_result(&mysql);
	uint32 rowCount = 0;

	if (res != nullptr)
        rowCount = (uint32)mysql_num_rows(res);

	MySQLRequestResult requestResult(res, (uint32)mysql_affected_rows(&mysql), rowCount, (uint32)mysql_field_count(&mysql), (uint32)mysql_insert_id(&mysql));


#if DEBUG_MYSQL_QUERIES >= 1
	if (requestResult.Success())
	{
		std::cout << "query successful";
		if (requestResult.Result())
			std::cout << ", " << (int) mysql_num_rows(requestResult.Result()) << " rows returned";

		std::cout << ", " << requestResult.RowCount() << " rows affected";
		std::cout<< std::endl;
	}
	else {
		std::cout << "QUERY: query FAILED" << std::endl;
	}
#endif

	return requestResult;
}

bool DBcore::RunQuery(const char* query, uint32 querylen, char* errbuf, MYSQL_RES** result, uint32* affected_rows, uint32* last_insert_id, uint32* errnum, bool retry) {
	if (errnum)
		*errnum = 0;
	if (errbuf)
		errbuf[0] = 0;
	bool ret = false;
	LockMutex lock(&MDatabase);
	if (pStatus != Connected)
		Open();

	if (mysql_real_query(&mysql, query, querylen)) {
		if (mysql_errno(&mysql) == CR_SERVER_GONE_ERROR)
			pStatus = Error;
		if (mysql_errno(&mysql) == CR_SERVER_LOST || mysql_errno(&mysql) == CR_SERVER_GONE_ERROR) {
			if (retry) {
				std::cout << "Database Error: Lost connection, attempting to recover...." << std::endl;
				ret = RunQuery(query, querylen, errbuf, result, affected_rows, last_insert_id, errnum, false);
				if (ret)
					std::cout << "Reconnection to database successful." << std::endl;
			}
			else {
				pStatus = Error;
				if (errnum)
					*errnum = mysql_errno(&mysql);
				if (errbuf)
					snprintf(errbuf, MYSQL_ERRMSG_SIZE, "#%i: %s", mysql_errno(&mysql), mysql_error(&mysql));
				std::cout << "DB Query Error #" << mysql_errno(&mysql) << ": " << mysql_error(&mysql) << std::endl;
				ret = false;
			}
		}
		else {
			if (errnum)
				*errnum = mysql_errno(&mysql);
			if (errbuf)
				snprintf(errbuf, MYSQL_ERRMSG_SIZE, "#%i: %s", mysql_errno(&mysql), mysql_error(&mysql));
#ifdef _EQDEBUG
			std::cout << "DB Query Error #" << mysql_errno(&mysql) << ": " << mysql_error(&mysql) << std::endl;
#endif
			ret = false;
		}
	}
	else {
		if (result && mysql_field_count(&mysql)) {
			*result = mysql_store_result(&mysql);
#ifdef _EQDEBUG
			DBMemLeak::Alloc(*result, query);
#endif
		}
		else if (result)
			*result = 0;
		if (affected_rows)
			*affected_rows = mysql_affected_rows(&mysql);
		if (last_insert_id)
			*last_insert_id = (uint32)mysql_insert_id(&mysql);
		if (result) {
			if (*result) {
				ret = true;
			}
			else {
#ifdef _EQDEBUG
				std::cout << "DB Query Error: No Result" << std::endl;
#endif
				if (errnum)
					*errnum = UINT_MAX;
				if (errbuf)
					strcpy(errbuf, "DBcore::RunQuery: No Result");
				ret = false;
			}
		}
		else {
			ret = true;
		}
	}
#if DEBUG_MYSQL_QUERIES >= 1
	if (ret) {
		std::cout << "query successful";
		if (result && (*result))
			std::cout << ", " << (int) mysql_num_rows(*result) << " rows returned";
		if (affected_rows)
			std::cout << ", " << (*affected_rows) << " rows affected";
		std::cout<< std::endl;
	}
	else {
		std::cout << "QUERY: query FAILED" << std::endl;
	}
#endif
	return ret;
}

uint32 DBcore::DoEscapeString(char* tobuf, const char* frombuf, uint32 fromlen) {
//	No good reason to lock the DB, we only need it in the first place to check char encoding.
//	LockMutex lock(&MDatabase);
	return mysql_real_escape_string(&mysql, tobuf, frombuf, fromlen);
}

bool DBcore::Open(const char* iHost, const char* iUser, const char* iPassword, const char* iDatabase,uint32 iPort, uint32* errnum, char* errbuf, bool iCompress, bool iSSL) {
	LockMutex lock(&MDatabase);
	safe_delete(pHost);
	safe_delete(pUser);
	safe_delete(pPassword);
	safe_delete(pDatabase);
	pHost = strcpy(new char[strlen(iHost) + 1], iHost);
	pUser = strcpy(new char[strlen(iUser) + 1], iUser);
	pPassword = strcpy(new char[strlen(iPassword) + 1], iPassword);
	pDatabase = strcpy(new char[strlen(iDatabase) + 1], iDatabase);
	pCompress = iCompress;
	pPort = iPort;
	pSSL = iSSL;
	return Open(errnum, errbuf);
}

bool DBcore::Open(uint32* errnum, char* errbuf) {
	if (errbuf)
		errbuf[0] = 0;
	LockMutex lock(&MDatabase);
	if (GetStatus() == Connected)
		return true;
	if (GetStatus() == Error) {
		mysql_close(&mysql);
		mysql_init(&mysql);		// Initialize structure again
	}
	if (!pHost)
		return false;
	/*
	Added CLIENT_FOUND_ROWS flag to the connect
	otherwise DB update calls would say 0 rows affected when the value already equalled
	what the function was tring to set it to, therefore the function would think it failed
	*/
	uint32 flags = CLIENT_FOUND_ROWS;
	if (pCompress)
		flags |= CLIENT_COMPRESS;
	if (pSSL)
		flags |= CLIENT_SSL;
	if (mysql_real_connect(&mysql, pHost, pUser, pPassword, pDatabase, pPort, 0, flags)) {
		pStatus = Connected;
		return true;
	}
	else {
		if (errnum)
			*errnum = mysql_errno(&mysql);
		if (errbuf)
			snprintf(errbuf, MYSQL_ERRMSG_SIZE, "#%i: %s", mysql_errno(&mysql), mysql_error(&mysql));
		pStatus = Error;
		return false;
	}
}






