#include "database.h"
#include "main.h"
#include "eq_player_structs.h"
#include <vector>

extern std::vector<player_entry> player_list;

EQEmuDatabase::EQEmuDatabase(std::string ServerName, std::string DatabaseName, std::string DBUsername, std::string DBPassword) {
	SetServerName(ServerName);
	SetDatabaseName(DatabaseName);
	SetDBUsername(DBUsername);
	SetDBPassword(DBPassword);

	_mysql = mysql_init(NULL);
	if(_mysql)
	{
		if (!mysql_real_connect(_mysql, GetServerName().c_str(), GetDBUsername().c_str(),
			GetDBPassword().c_str(), GetDatabaseName().c_str(), 0, NULL, 0))
		{
			cout << "MYSQL CONNECT FAILED: " << endl;
			cout << GetServerName() << endl;
			cout << GetDBUsername() << endl;
			cout << GetDBPassword() << endl;
			cout << GetDatabaseName() << endl;
			mysql_close(_mysql);
			_mysql = 0;
		}
	}
}

EQEmuDatabase::~EQEmuDatabase()
{
	if(_mysql)
	{
		mysql_close(_mysql);
	}
}

void EQEmuDatabase::GetPlayer(std::string name)
{
	bool result = false;
	if(!_mysql)
	{
		cout << "NOT CONNECTED TO MYQSL: " << endl;
		return;
	}

	MYSQL_RES *res;
	MYSQL_ROW row;
	char * mQuery = 0;
	MakeAnyLenString(&mQuery, "SELECT profile, id FROM character_ WHERE name='%s'", name.c_str());
	if (mysql_query(_mysql, mQuery)) {
		cout << "Query failed: " << mQuery << endl;
		return;
	}

	res = mysql_use_result(_mysql);
	if(res)
	{
		while ((row = mysql_fetch_row(res)) != NULL)
		{
			player_entry pe;
			pe.id = atoi(row[1]);
			pe.data = new char[sizeof(PlayerProfile_Struct)];
			memcpy(pe.data, row[0], sizeof(PlayerProfile_Struct));
			player_list.push_back(pe);

			PlayerProfile_Struct *m_pp = (PlayerProfile_Struct*)pe.data;
		}
		mysql_free_result(res);
	}
	delete[] mQuery;
	mQuery = 0;
}

void EQEmuDatabase::GetPlayers()
{
	bool result = false;
	if(!_mysql)
	{
		cout << "NOT CONNECTED TO MYQSL: " << endl;
		return;
	}

	MYSQL_RES *res;
	MYSQL_ROW row;
	char * mQuery = 0;
	MakeAnyLenString(&mQuery, "SELECT profile, id FROM character_");
	if (mysql_query(_mysql, mQuery)) {
		cout << "Query failed: " << mQuery << endl;
		return;
	}

	res = mysql_use_result(_mysql);
	if(res)
	{
		while ((row = mysql_fetch_row(res)) != NULL)
		{
			player_entry pe;
			pe.id = atoi(row[1]);
			pe.data = new char[sizeof(PlayerProfile_Struct)];
			memcpy(pe.data, row[0], sizeof(PlayerProfile_Struct));
			player_list.push_back(pe);

			PlayerProfile_Struct *m_pp = (PlayerProfile_Struct*)pe.data;
		}
		mysql_free_result(res);
	}
	delete[] mQuery;
	mQuery = 0;
}

void EQEmuDatabase::StorePlayer(int32 charid, char* data)
{
	if(!_mysql)
	{
		cout << "NOT CONNECTED TO MYQSL: " << endl;
		return;
	}

	char *outbuffer = new char[2*sizeof(PlayerProfile_Struct) + 512];
	char *bptr = outbuffer;
	bptr += snprintf(bptr, 128, "UPDATE character_ SET profile='");
	bptr += mysql_real_escape_string(_mysql, bptr, (const char *) data, sizeof(PlayerProfile_Struct));
	snprintf(bptr, 128, "' WHERE id=%lu", charid);

	if (mysql_query(_mysql, outbuffer)) {
		cout << "Query failed: " << outbuffer << endl;
	}

	delete[] outbuffer;
	outbuffer = 0;
}
