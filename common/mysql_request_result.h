#ifndef MYSQL_REQUEST_RESULT_H
#define MYSQL_REQUEST_RESULT_H

#ifdef _WINDOWS
	#include <winsock2.h>
	#include <windows.h>
#endif

#include <mysql.h>
#include "types.h"
#include "mysql_request_row.h"

#ifdef __FreeBSD__
	#include <string>
	#include <sstream>
	#include <iostream>
#endif

class MySQLRequestResult {
private:
	MYSQL_RES* m_Result;
	MYSQL_FIELD* m_Fields;
	char* m_ErrorBuffer;
	unsigned long* m_ColumnLengths;
	MySQLRequestRow m_CurrentRow;
	MySQLRequestRow m_OneBeyondRow;

	bool m_Success;
	uint32 m_RowsAffected;
	uint32 m_RowCount;
	uint32 m_ColumnCount;
	uint32 m_LastInsertedID;
	uint32 m_ErrorNumber;


public:

	MySQLRequestResult(MYSQL_RES* result, uint32 rowsAffected = 0, uint32 rowCount = 0, uint32 columnCount = 0, uint32 lastInsertedID = 0, uint32 errorNumber = 0, char *errorBuffer = nullptr);
	MySQLRequestResult();	
	MySQLRequestResult(MySQLRequestResult&& moveItem);
	~MySQLRequestResult();

	MySQLRequestResult& operator=(MySQLRequestResult&& other);

	bool Success() const { return m_Success;}
	std::string ErrorMessage() const {return m_ErrorBuffer ? std::string(m_ErrorBuffer) : std::string("");}
	uint32 ErrorNumber() const {return m_ErrorNumber;}
	uint32 RowsAffected() const {return m_RowsAffected;}
	uint32 RowCount() const {return m_RowCount;}
	uint32 ColumnCount() const {return m_ColumnCount;}
	uint32 LastInsertedID() const {return m_LastInsertedID;}
	// default to 0 index since we mostly use it that way anyways.
	uint32 LengthOfColumn(int columnIndex = 0); 
	const std::string FieldName(int columnIndex);

	MySQLRequestRow& begin() { return m_CurrentRow; }
	MySQLRequestRow& end() { return m_OneBeyondRow; }

private:
	void FreeInternals();
	void ZeroOut();
};


#endif

