#ifndef DBCORE_H
#define DBCORE_H

#ifdef _WINDOWS
	#include <winsock.h>
	#include <windows.h>
	//#include <winsock.h>
#endif
#include <mysql.h>
#include "../common/types.h"
#include "../common/Mutex.h"
#include "../common/linked_list.h"
#include "../common/queue.h"
#include "../common/timer.h"
#include "../common/Condition.h"

class DBcore {
public:
	enum eStatus { Closed, Connected, Error };

	DBcore();
	~DBcore();
	eStatus	GetStatus() { return pStatus; }
	bool	RunQuery(const char* query, uint32 querylen, char* errbuf = 0, MYSQL_RES** result = 0, uint32* affected_rows = 0, uint32* last_insert_id = 0, uint32* errnum = 0, bool retry = true);
	uint32	DoEscapeString(char* tobuf, const char* frombuf, uint32 fromlen);
	void	ping();
	MYSQL*	getMySQL(){ return &mysql; }

protected:
	bool	Open(const char* iHost, const char* iUser, const char* iPassword, const char* iDatabase, uint32 iPort, uint32* errnum = 0, char* errbuf = 0, bool iCompress = false, bool iSSL = false);
private:
	bool	Open(uint32* errnum = 0, char* errbuf = 0);

	MYSQL	mysql;
	Mutex	MDatabase;
	eStatus pStatus;

	char*	pHost;
	char*	pUser;
	char*	pPassword;
	char*	pDatabase;
	bool	pCompress;
	uint32	pPort;
	bool	pSSL;

};


#endif

