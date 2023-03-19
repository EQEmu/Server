/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2008 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

*/

#ifndef CHATSERVER_DATABASE_H
#define CHATSERVER_DATABASE_H

#define AUTHENTICATION_TIMEOUT	60
#define INVALID_ID				0xFFFFFFFF

#include "../common/global_define.h"
#include "../common/types.h"
#include "../common/database.h"
#include "../common/linked_list.h"
#include "../common/database.h"
#include "clientlist.h"
#include "chatchannel.h"
#include <string>
#include <vector>
#include <map>

//atoi is not uint32 or uint32 safe!!!!
#define atoul(str) strtoul(str, nullptr, 10)

class UCSDatabase : public Database {
public:
	int FindAccount(const char *CharacterName, Client *c);
	int FindCharacter(const char *CharacterName);
	bool VerifyMailKey(std::string CharacterName, int IPAddress, std::string MailKey);
	bool GetVariable(const char* varname, char* varvalue, uint16 varvalue_len);
	bool LoadChatChannels();
	void LoadReservedNamesFromDB();
	void LoadFilteredNamesFromDB();
	bool IsChatChannelInDB(const std::string& channel_name);
	bool CheckChannelNameFilter(const std::string& channel_name);
	void SaveChatChannel(const std::string& channel_name, const std::string& channel_owner, const std::string& channel_password, const uint16& min_status);
	void DeleteChatChannel(const std::string& channel_name);
	int CurrentPlayerChannelCount(const std::string& player_name);
	std::string CurrentPlayerChannels(const std::string& player_name);
	void GetAccountStatus(Client *c);
	void SetChannelPassword(const std::string& channel_name, const std::string& password);
	void SetChannelOwner(const std::string& channel_name, const std::string& owner);
	void SendHeaders(Client *c);
	void SendBody(Client *c, const int& message_number);
	bool SendMail(const std::string& recipient, const std::string& from, const std::string& subject, const std::string& body, const std::string& recipients_string);
	void SetMessageStatus(const int& message_number, const int& Status);
	void ExpireMail();
	void AddFriendOrIgnore(const int& char_id, const int& type, const std::string& name);
	void RemoveFriendOrIgnore(const int& char_id, const int& type, const std::string& name);
	void GetFriendsAndIgnore(const int& char_id, std::vector<std::string> &Friends, std::vector<std::string> &Ignorees);

protected:
	void HandleMysqlError(uint32 errnum);
private:
	void DBInitVars();

};

#endif

