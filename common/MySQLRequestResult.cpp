#include "MySQLRequestResult.h"


MySQLRequestResult::MySQLRequestResult()
	: m_CurrentRow(), m_OneBeyondRow()
{
	ZeroOut();
}

MySQLRequestResult::MySQLRequestResult(MYSQL_RES* result, uint32 rowsAffected, uint32 rowCount, uint32 columnCount, uint32 lastInsertedID, uint32 errorNumber, char *errorBuffer)
	: m_CurrentRow(result), m_OneBeyondRow()
{
	if (errorBuffer != nullptr)
		m_Success = false;
	else if (errorBuffer == nullptr && result == nullptr)
	{
		m_Success = false;
		
#ifdef _EQDEBUG
		std::cout << "DB Query Error: No Result" << std::endl;
#endif
		m_ErrorNumber = UINT_MAX;
		m_ErrorBuffer = new char[MYSQL_ERRMSG_SIZE];

		strcpy(m_ErrorBuffer, "DBcore::RunQuery: No Result");
	} 
	else
		m_Success = true;

	m_Result = result;
	m_ErrorBuffer = errorBuffer;
	m_RowsAffected = rowsAffected;
	m_RowCount = rowCount;
	m_ColumnCount = columnCount;
	m_LastInsertedID = lastInsertedID;
	m_ErrorNumber = errorNumber;
}

void MySQLRequestResult::FreeInternals()
{
	safe_delete_array(m_ErrorBuffer);

	if (m_Result != nullptr)
		mysql_free_result(m_Result);

	ZeroOut();
}

void MySQLRequestResult::ZeroOut()
{
	m_Success = false;
	m_Result = nullptr;
	m_ErrorBuffer = nullptr;
	m_RowCount = 0;
	m_RowsAffected = 0;
	m_LastInsertedID = 0;
}

MySQLRequestResult::~MySQLRequestResult()
{
	FreeInternals();
}

MySQLRequestResult::MySQLRequestResult(MySQLRequestResult&& moveItem)
	: m_CurrentRow(moveItem.m_CurrentRow), m_OneBeyondRow()
{
	m_Result = moveItem.m_Result;
	m_ErrorBuffer = moveItem.m_ErrorBuffer;
	m_Success = moveItem.m_Success;
	m_RowCount = moveItem.m_RowCount;
	m_RowsAffected = moveItem.m_RowsAffected;
	m_LastInsertedID = moveItem.m_LastInsertedID;

	// Keeps deconstructor from double freeing 
	// pre move instance.
	moveItem.ZeroOut();
}

MySQLRequestResult& MySQLRequestResult::operator=(MySQLRequestResult&& other)
{
	// Assigning something to itself?
	// Silly! (but happens)
	if (this == &other)
		return *this;

	FreeInternals();

	m_Success = other.m_Success;

	m_Result = other.m_Result;
	m_ErrorBuffer = other.m_ErrorBuffer;

	m_RowCount = other.m_RowCount;
	m_RowsAffected = other.m_RowsAffected;
	m_LastInsertedID = other.m_LastInsertedID;
	m_CurrentRow = other.m_CurrentRow;
	m_OneBeyondRow = other.m_OneBeyondRow;
	
	// Keeps deconstructor from double freeing 
	// pre move instance.
	other.ZeroOut();
	return *this;
}