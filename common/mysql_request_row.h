#ifndef MYSQL_REQUEST_ROW_H
#define MYSQL_REQUEST_ROW_H

#ifdef _WINDOWS
	#include <winsock2.h>
	#include <windows.h>
#endif

#include <mysql.h>
#include <iterator>
#include "types.h"

class MySQLRequestRow
{
public:
	using iterator_category = std::input_iterator_tag;
	using value_type = MYSQL_ROW;
	using difference_type = std::ptrdiff_t;
	using pointer = MYSQL_ROW*;
	using reference = MYSQL_ROW&;

private:
	MYSQL_RES* m_Result;
	MYSQL_ROW m_MySQLRow;

public:

	MySQLRequestRow();
	MySQLRequestRow(MYSQL_RES *result);
	MySQLRequestRow(const MySQLRequestRow& row);
	MySQLRequestRow(MySQLRequestRow&& moveItem);
	MySQLRequestRow& operator=(MySQLRequestRow& moveItem);
	MySQLRequestRow& operator++();
	MySQLRequestRow operator++(int);
	bool operator==(const MySQLRequestRow& rhs);
	bool operator!=(const MySQLRequestRow& rhs);
	MySQLRequestRow operator*();
	char* operator[](int index);

};



#endif
