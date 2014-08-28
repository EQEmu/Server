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
#ifndef LAUNCHERLIST_H_
#define LAUNCHERLIST_H_

#include "../common/types.h"
#include <map>
#include <vector>
#include <string>

class LauncherLink;
class EmuTCPConnection;
class EQLConfig;

class LauncherList {
public:
	LauncherList();
	~LauncherList();

	void Process();

	void LoadList();
	EQLConfig *GetConfig(const char *name);
	void CreateLauncher(const char *name, uint8 dynamic_count);
	void Remove(const char *name);

	void Add(EmuTCPConnection *conn);
	LauncherLink *Get(const char *name);
	LauncherLink *FindByZone(const char *short_name);

	int GetLauncherCount();
	void GetLauncherNameList(std::vector<std::string> &list);

protected:
	std::map<std::string, EQLConfig *> m_configs;	//we own these objects
	std::map<std::string, LauncherLink *> m_launchers;	//we own these objects
//	std::map<std::string, EQLConfig *> m_configs;	//we own these objects
	std::vector<LauncherLink *> m_pendingLaunchers;	//we own these objects, have not yet identified themself
	int nextID;
};

#endif /*LAUNCHERLIST_H_*/

