/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#include "../common/debug.h"
#include "EQLConfig.h"
#include "worlddb.h"
#include "LauncherLink.h"
#include "LauncherList.h"
#include "../common/StringUtil.h"
#include <cstdlib>
#include <cstring>

extern LauncherList launcher_list;

EQLConfig::EQLConfig(const char *launcher_name)
: m_name(launcher_name)
{
	LoadSettings();
}

void EQLConfig::LoadSettings() {
	char errbuf[MYSQL_ERRMSG_SIZE];
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;
	LauncherZone tmp;

	std::string namebuf;
	database.DoEscapeString(namebuf, m_name.c_str(), m_name.length());	
	namebuf.resize(63); //limit len to 64

	StringFormat(query,"SELECT dynamics FROM launcher WHERE name='%s'",namebuf.c_str());

	if (database.RunQuery(query, errbuf, &result))
	{
		while ((row = mysql_fetch_row(result))) {
			m_dynamics = atoi(row[0]);
		}
		mysql_free_result(result);
	} else {
		LogFile->write(EQEMuLog::Error, "EQLConfig::LoadSettings: %s", errbuf);
	}

	StringFormat(query, "SELECT zone,port FROM launcher_zones WHERE launcher='%s'", namebuf.c_str());

	if (database.RunQuery(query, errbuf, &result))
	{
		LauncherZone zs;
		while ((row = mysql_fetch_row(result))) {
			zs.name = row[0];
			zs.port = atoi(row[1]);
			m_zones[zs.name] = zs;
		}
		mysql_free_result(result);
	} else {
		LogFile->write(EQEMuLog::Error, "EQLConfig::LoadSettings: %s", errbuf);
	}
}

EQLConfig *EQLConfig::CreateLauncher(const char *name, uint8 dynamic_count) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	std::string query;

	std::string namebuf;
	database.DoEscapeString(namebuf, name, strlen(name));	
	namebuf.resize(63); //limit len to 64

	StringFormat(query, "INSERT INTO launcher (name,dynamics) VALUES('%s', %d)",
						namebuf.c_str(), dynamic_count);

	if (!database.RunQuery(query, errbuf)) {
		LogFile->write(EQEMuLog::Error, "Error in CreateLauncher query: %s", errbuf);
		return nullptr;
	}

	return(new EQLConfig(name));
}

void EQLConfig::GetZones(std::vector<LauncherZone> &result) {
	std::map<std::string, LauncherZone>::iterator cur, end;
	cur = m_zones.begin();
	end = m_zones.end();
	for(; cur != end; cur++) {
		result.push_back(cur->second);
	}
}

std::vector<std::string> EQLConfig::ListZones() {
	LauncherLink *ll = launcher_list.Get(m_name.c_str());
	std::vector<std::string> res;
	if(ll == nullptr) {
		//if the launcher isnt connected, use the list from the database.
		std::map<std::string, LauncherZone>::iterator cur, end;
		cur = m_zones.begin();
		end = m_zones.end();
		for(; cur != end; cur++) {
			res.push_back(cur->first);
		}
	} else {
		//otherwise, use the zone list from the launcher link.
		ll->GetZoneList(res);
	}
	return(res);
}

void EQLConfig::DeleteLauncher() {

	launcher_list.Remove(m_name.c_str());

	char errbuf[MYSQL_ERRMSG_SIZE];
	std::string query;

	std::string namebuf;
	database.DoEscapeString(namebuf, m_name.c_str(), m_name.length());	
	namebuf.resize(63); //limit len to 64

	StringFormat(query, "DELETE FROM launcher WHERE name='%s'",namebuf.c_str());

	if (!database.RunQuery(query, errbuf)) {
		LogFile->write(EQEMuLog::Error, "Error in DeleteLauncher 1 query: %s", errbuf);
		return;
	}

	StringFormat(query, "DELETE FROM launcher_zones WHERE launcher='%s'",
						namebuf.c_str());

	if (!database.RunQuery(query, errbuf)) {
		LogFile->write(EQEMuLog::Error, "Error in DeleteLauncher 2 query: %s", errbuf);
		return;
	}
}

bool EQLConfig::IsConnected() const {
	LauncherLink *ll = launcher_list.Get(m_name.c_str());
	return(ll != nullptr);
}

void EQLConfig::RestartZone(Const_char *zone_ref) {
	LauncherLink *ll = launcher_list.Get(m_name.c_str());
	if(ll == nullptr)
		return;
	ll->RestartZone(zone_ref);
}

void EQLConfig::StopZone(Const_char *zone_ref) {
	LauncherLink *ll = launcher_list.Get(m_name.c_str());
	if(ll == nullptr)
		return;
	ll->StopZone(zone_ref);
}

void EQLConfig::StartZone(Const_char *zone_ref) {
	LauncherLink *ll = launcher_list.Get(m_name.c_str());
	if(ll == nullptr)
		return;
	ll->StartZone(zone_ref);
}

bool EQLConfig::BootStaticZone(Const_char *short_name, uint16 port) {
	//make sure the short name is valid.
	if(database.GetZoneID(short_name) == 0)
		return(false);

	//database update
	char errbuf[MYSQL_ERRMSG_SIZE];
	std::string query;

	std::string namebuf;
	database.DoEscapeString(namebuf, m_name.c_str(), m_name.length());	
	namebuf.resize(63); //limit len to 64
	
	std::string zonebuf;
	database.DoEscapeString(zonebuf, short_name, strlen(short_name));	
	zonebuf.resize(15); //limit len to 16

	StringFormat(query, "INSERT INTO launcher_zones (launcher,zone,port) VALUES('%s', '%s', %d)",
						namebuf.c_str(), zonebuf.c_str(), port);

	if (!database.RunQuery(query, errbuf)) {
		LogFile->write(EQEMuLog::Error, "Error in BootStaticZone query: %s", errbuf);
		return false;
	}

	//update our internal state.
	LauncherZone lz;
	lz.name = short_name;
	lz.port = port;
	m_zones[lz.name] = lz;

	//if the launcher is connected, update it.
	LauncherLink *ll = launcher_list.Get(m_name.c_str());
	if(ll != nullptr) {
		ll->BootZone(short_name, port);
	}

	return(true);
}

bool EQLConfig::ChangeStaticZone(Const_char *short_name, uint16 port) {
	//make sure the short name is valid.
	if(database.GetZoneID(short_name) == 0)
		return(false);

	//check internal state
	std::map<std::string, LauncherZone>::iterator res;
	res = m_zones.find(short_name);
	if(res == m_zones.end()) {
		//not found.
		LogFile->write(EQEMuLog::Error, "Update for unknown zone %s", short_name);
		return(false);
	}


	//database update
	char errbuf[MYSQL_ERRMSG_SIZE];
	std::string query;

	std::string namebuf;
	database.DoEscapeString(namebuf, m_name.c_str(), m_name.length());	
	namebuf.resize(63); //limit len to 64
	
	std::string zonebuf;
	database.DoEscapeString(zonebuf, short_name, strlen(short_name));	//limit len to 16
	zonebuf.resize(15);

	StringFormat(query, "UPDATE launcher_zones SET port=%d WHERE launcher='%s' AND zone='%s'",
						port, namebuf.c_str(), zonebuf.c_str());

	if (!database.RunQuery(query, errbuf)) {
		LogFile->write(EQEMuLog::Error, "Error in ChangeStaticZone query: %s", errbuf);
		return false;
	}

	//update internal state
	res->second.port = port;

	//if the launcher is connected, update it.
	LauncherLink *ll = launcher_list.Get(m_name.c_str());
	if(ll != nullptr) {
		ll->RestartZone(short_name);
	}

	return(true);
}

bool EQLConfig::DeleteStaticZone(Const_char *short_name) {
	//check internal state
	std::map<std::string, LauncherZone>::iterator res;
	res = m_zones.find(short_name);
	if(res == m_zones.end()) {
		//not found.
		LogFile->write(EQEMuLog::Error, "Update for unknown zone %s", short_name);
		return(false);
	}

	//database update
	char errbuf[MYSQL_ERRMSG_SIZE];
	std::string query;

	std::string namebuf;
	database.DoEscapeString(namebuf, m_name.c_str(), m_name.length());	
	namebuf.resize(63);//limit len to 64
	
	std::string zonebuf;
	database.DoEscapeString(zonebuf, short_name, strlen(short_name));	
	zonebuf.resize(15); //limit len to 16

	StringFormat(query, "DELETE FROM launcher_zones WHERE launcher='%s' AND zone='%s'",
						namebuf.c_str(), zonebuf.c_str());

	if (!database.RunQuery(query, errbuf)) {
		LogFile->write(EQEMuLog::Error, "Error in DeleteStaticZone query: %s", errbuf);
		return false;
	}

	//internal update.
	m_zones.erase(res);

	//if the launcher is connected, update it.
	LauncherLink *ll = launcher_list.Get(m_name.c_str());
	if(ll != nullptr) {
		ll->StopZone(short_name);
	}

	return true;
}

bool EQLConfig::SetDynamicCount(int count) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	std::string query;

	std::string namebuf;
	database.DoEscapeString(namebuf, m_name.c_str(), m_name.length());	
	namebuf.resize(63);//limit len to 64

	StringFormat(query, "UPDATE launcher SET dynamics=%d WHERE name='%s'",
						count, namebuf.c_str());

	if (!database.RunQuery(query, errbuf)) {
		LogFile->write(EQEMuLog::Error, "Error in SetDynamicCount query: %s", errbuf);
		return false;
	}

	//update in-memory version.
	m_dynamics = count;

	//if the launcher is connected, update it.
	LauncherLink *ll = launcher_list.Get(m_name.c_str());
	if(ll != nullptr) {
		ll->BootDynamics(count);
	}

	return(false);
}

int EQLConfig::GetDynamicCount() const {
	return(m_dynamics);
}

std::map<std::string,std::string> EQLConfig::GetZoneDetails(Const_char *zone_ref) {
	std::map<std::string,std::string> res;

	LauncherLink *ll = launcher_list.Get(m_name.c_str());
	if(ll == nullptr) {
		res["name"] = zone_ref;
		res["up"] = "0";
		res["starts"] = "0";
		res["port"] = "0";
	} else {
		ll->GetZoneDetails(zone_ref, res);
	}

	return(res);
}

