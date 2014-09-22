#include "mysql_request_result.h"


MySQLRequestResult::MySQLRequestResult()
	: m_CurrentRow(), m_OneBeyondRow()
{
	ZeroOut();
}

MySQLRequestResult::MySQLRequestResult(MYSQL_RES* result, uint32 rowsAffected, uint32 rowCount, uint32 columnCount, uint32 lastInsertedID, uint32 errorNumber, char *errorBuffer)
	: m_CurrentRow(result), m_OneBeyondRow()
{
	m_Result = result;
	m_RowsAffected = rowsAffected;
	m_RowCount = rowCount;
	m_ColumnCount = columnCount;
	m_LastInsertedID = lastInsertedID;

	// If we actually need the column length / fields it will be
	// requested at that time, no need to pull it in just to cache it.
	// Normal usage would have it as nullptr most likely anyways.
	m_ColumnLengths = nullptr;
	m_Fields = nullptr;

	m_Success = true;
	if (errorBuffer != nullptr)
		m_Success = false;

	m_ErrorNumber = errorNumber;
	m_ErrorBuffer = errorBuffer;
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
	m_ColumnLengths = nullptr;
	m_Fields = nullptr;
	m_RowCount = 0;
	m_RowsAffected = 0;
	m_LastInsertedID = 0;
}

MySQLRequestResult::~MySQLRequestResult()
{
	FreeInternals();
}

uint32 MySQLRequestResult::LengthOfColumn(int columnIndex)
{
	if (m_ColumnLengths == nullptr && m_Result != nullptr)
		m_ColumnLengths = mysql_fetch_lengths(m_Result);

	// If someone screws up and tries to get the length of a
	// column when no result occured (check Success! argh!)
	// then we always return 0. Also applies if mysql screws
	// up and can't get the column lengths for whatever reason.
	if (m_ColumnLengths == nullptr)
		return 0;

	// Want to index check to be sure we don't read passed
	// the end of the array. Just default to 0 in that case.
	// We *shouldn't* need this or the previous checks if all
	// interface code is correctly written.
	if (columnIndex >= m_ColumnCount)
		return 0;

	return m_ColumnLengths[columnIndex];
}

const std::string MySQLRequestResult::FieldName(int columnIndex)
{
	if (columnIndex >= m_ColumnCount || m_Result == nullptr)
		return std::string();

	if (m_Fields == nullptr)
		m_Fields = mysql_fetch_fields(m_Result);

	return std::string(m_Fields[columnIndex].name);
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
	m_ColumnLengths = moveItem.m_ColumnLengths;
	m_ColumnCount = moveItem.m_ColumnCount;
	m_Fields = moveItem.m_Fields;

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
	m_ColumnLengths = other.m_ColumnLengths;
	m_ColumnCount = other.m_ColumnCount;
	m_Fields = other.m_Fields;

	// Keeps deconstructor from double freeing
	// pre move instance.
	other.ZeroOut();
	return *this;
}
