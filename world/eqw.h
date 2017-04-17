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
#ifndef EQW_H_
#define EQW_H_

#include <string>
#include <vector>
#include <map>
#include "../common/types.h"

class EQLConfig;

//this is the main object exported to perl.
class EQW {
	EQW();
public:
	static EQW *Singleton() { return(&s_EQW); }

	void AppendOutput(const char *str);
	const std::string &GetOutput() const;
	void ClearOutput() { m_outputBuffer = ""; }

//BEGIN PERL EXPORT
	//NOTE: you must have a space after the * of a return value
	Const_char * GetConfig(Const_char *var_name);
	void LockWorld();
	void UnlockWorld();

	bool LSConnected();

	int CountZones();
	std::vector<std::string> ListBootedZones();	//returns an array of zone_refs (opaque)
	std::map<std::string,std::string> GetZoneDetails(Const_char *zone_ref);	//returns a hash ref of details

	int CountPlayers();
	std::vector<std::string> ListPlayers(Const_char *zone_name = "");	//returns an array of player refs (opaque)
	std::map<std::string,std::string> GetPlayerDetails(Const_char *player_ref);	//returns a hash ref of details

	int CountLaunchers(bool active_only);
//	vector<string> ListActiveLaunchers();	//returns an array of launcher names
	std::vector<std::string> ListLaunchers();	//returns an array of launcher names
	EQLConfig * GetLauncher(Const_char *launcher_name);	//returns the EQLConfig object for the specified launcher.
	void CreateLauncher(Const_char *launcher_name, int dynamic_count);
//	EQLConfig * FindLauncher(Const_char *zone_ref);

	//Guild routines, mostly wrappers around guild_mgr
	uint32	CreateGuild(const char* name, uint32 leader_char_id);
	bool	DeleteGuild(uint32 guild_id);
	bool	RenameGuild(uint32 guild_id, const char* name);
	bool	SetGuildMOTD(uint32 guild_id, const char* motd, const char *setter);
	bool	SetGuildLeader(uint32 guild_id, uint32 leader_char_id);
	bool	SetGuild(uint32 charid, uint32 guild_id, uint8 rank);
	bool	SetGuildRank(uint32 charid, uint8 rank);
	bool	SetBankerFlag(uint32 charid, bool is_banker);
	bool	SetTributeFlag(uint32 charid, bool enabled);
	bool	SetPublicNote(uint32 charid, const char *note);

	//bugs
	int CountBugs();
	std::vector<std::string> ListBugs(uint32 offset);	//returns an array of zone_refs (opaque)
	std::map<std::string,std::string> GetBugDetails(const char *id);
    void ResolveBug(const char *id);
	
	void SendMessage(uint32 type, const char *msg);
	void WorldShutDown(uint32 time, uint32 interval);
//END PERL EXPORT

protected:
	std::string m_outputBuffer;
	std::string m_returnBuffer;

	bool m_worldLocked;

private:
	static EQW s_EQW;
};

#endif /*EQW_H_*/
