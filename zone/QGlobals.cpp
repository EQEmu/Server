#include "../common/debug.h"
#include "../common/StringUtil.h"
#include "QGlobals.h"
#include "masterentity.h"
#include "zone.h"
#include "zonedb.h"

void QGlobalCache::AddGlobal(uint32 id, QGlobal global)
{
	global.id = id;
	qGlobalBucket.push_back(global);
}

void QGlobalCache::RemoveGlobal(std::string name, uint32 npcID, uint32 charID, uint32 zoneID)
{
	std::list<QGlobal>::iterator iter = qGlobalBucket.begin();
	while(iter != qGlobalBucket.end())
	{
		if(name.compare((*iter).name) == 0)
		{
			if((npcID == (*iter).npc_id || (*iter).npc_id == 0) &&
				(charID == (*iter).char_id || (*iter).char_id == 0) &&
				(zoneID == (*iter).zone_id || (*iter).zone_id == 0))
			{
				qGlobalBucket.erase(iter);
				return;
			}
		}
		++iter;
	}
}

void QGlobalCache::Combine(std::list<QGlobal> &cacheA, std::list<QGlobal> cacheB, uint32 npcID, uint32 charID, uint32 zoneID)
{
	std::list<QGlobal>::iterator iter = cacheB.begin();
	while(iter != cacheB.end())
	{
		QGlobal cur = (*iter);

		if((cur.npc_id == npcID || cur.npc_id == 0) && (cur.char_id == charID || cur.char_id == 0) &&
			(cur.zone_id == zoneID || cur.zone_id == 0))
		{
			if(Timer::GetTimeSeconds() < cur.expdate)
			{
				cacheA.push_back(cur);
			}
		}
		++iter;
	}
}

void QGlobalCache::GetQGlobals(std::list<QGlobal> &globals, NPC *n, Client *c, Zone *z) {
	globals.clear();
	
	QGlobalCache *npc_c = nullptr;
	QGlobalCache *char_c = nullptr;
	QGlobalCache *zone_c = nullptr;
	uint32 npc_id = 0;
	uint32 char_id = 0;
	uint32 zone_id = 0;

	if(n) {
		npc_id = n->GetNPCTypeID();
		npc_c = n->GetQGlobals();
	}

	if(c) {
		char_id = c->CharacterID();
		char_c = c->GetQGlobals();
	}

	if(z) {
		zone_id = z->GetZoneID();
		zone_c = z->GetQGlobals();
	}

	if(!npc_c && n) {
		npc_c = n->CreateQGlobals();
		npc_c->LoadByNPCID(npc_id);
	}

	if(!char_c && c) {
		char_c = c->CreateQGlobals();
		char_c->LoadByCharID(char_id);
	}

	if(!zone_c && z) {
		zone_c = z->CreateQGlobals();
		zone_c->LoadByZoneID(zone_id);
		zone_c->LoadByGlobalContext();
	}

	if(npc_c) {
		QGlobalCache::Combine(globals, npc_c->GetBucket(), npc_id, char_id, zone_id);
	}

	if(char_c) {
		QGlobalCache::Combine(globals, char_c->GetBucket(), npc_id, char_id, zone_id);
	}

	if(zone_c) {
		QGlobalCache::Combine(globals, zone_c->GetBucket(), npc_id, char_id, zone_id);
	}
}

bool QGlobalCache::GetQGlobal(QGlobal &g, std::string name, NPC *n, Client *c, Zone *z) {
	std::list<QGlobal> globals;
	QGlobalCache::GetQGlobals(globals, n, c, z);

	auto iter = globals.begin();
	while(iter != globals.end()) {
		if(iter->name.compare(name) == 0) {
			g = (*iter);
			return true;
		}
		++iter;
	}

	return false;
}

void QGlobalCache::PurgeExpiredGlobals()
{
	if(!qGlobalBucket.size())
		return;

	std::list<QGlobal>::iterator iter = qGlobalBucket.begin();
	while(iter != qGlobalBucket.end())
	{
		QGlobal cur = (*iter);
		if(Timer::GetTimeSeconds() > cur.expdate)
		{
			iter = qGlobalBucket.erase(iter);
			continue;
		}
		++iter;
	}
}

void QGlobalCache::LoadByNPCID(uint32 npcID)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (database.RunQuery(query, MakeAnyLenString(&query, "select name, charid, npcid, zoneid, value, expdate"
		" from quest_globals where npcid = %d", npcID), errbuf, &result))
	{
		while((row = mysql_fetch_row(result)))
		{
			AddGlobal(0, QGlobal(std::string(row[0]), atoi(row[1]), atoi(row[2]), atoi(row[3]), row[4], row[5]?atoi(row[5]):0xFFFFFFFF));
		}
		mysql_free_result(result);
	}
	safe_delete_array(query);
}

void QGlobalCache::LoadByCharID(uint32 charID)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (database.RunQuery(query, MakeAnyLenString(&query, "select name, charid, npcid, zoneid, value, expdate from"
		" quest_globals where charid = %d && npcid = 0", charID), errbuf, &result))
	{
		while((row = mysql_fetch_row(result)))
		{
			AddGlobal(0, QGlobal(std::string(row[0]), atoi(row[1]), atoi(row[2]), atoi(row[3]), row[4], row[5]?atoi(row[5]):0xFFFFFFFF));
		}
		mysql_free_result(result);
	}
	safe_delete_array(query);
}

void QGlobalCache::LoadByZoneID(uint32 zoneID)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (database.RunQuery(query, MakeAnyLenString(&query, "select name, charid, npcid, zoneid, value, expdate from quest_globals"
		" where zoneid = %d && npcid = 0 && charid = 0", zoneID), errbuf, &result))
	{
		while((row = mysql_fetch_row(result)))
		{
			AddGlobal(0, QGlobal(std::string(row[0]), atoi(row[1]), atoi(row[2]), atoi(row[3]), row[4], row[5]?atoi(row[5]):0xFFFFFFFF));
		}
		mysql_free_result(result);
	}
	safe_delete_array(query);
}
void QGlobalCache::LoadByGlobalContext()
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (database.RunQuery(query, MakeAnyLenString(&query, "select name, charid, npcid, zoneid, value, expdate from quest_globals"
		" where zoneid = 0 && npcid = 0 && charid = 0"), errbuf, &result))
	{
		while((row = mysql_fetch_row(result)))
		{
			AddGlobal(0, QGlobal(std::string(row[0]), atoi(row[1]), atoi(row[2]), atoi(row[3]), row[4], row[5]?atoi(row[5]):0xFFFFFFFF));
		}
		mysql_free_result(result);
	}
	safe_delete_array(query);
}
