/*  EQEMu:  Everquest Server Emulator
	Copyright (C) 2001-2005  EQEMu Development Team (http://eqemulator.net)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	  You should have received a copy of the GNU General Public License
	  along with this program; if not, write to the Free Software
	  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "../common/debug.h"
#include "../common/eq_packet_structs.h"
#include "masterentity.h"
#include "titles.h"
#include "../common/MiscFunctions.h"
#include "worldserver.h"

extern WorldServer worldserver;

TitleManager::TitleManager() {
}

bool TitleManager::LoadTitles()
{
	Titles.clear();

	TitleEntry Title;

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = nullptr;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (!database.RunQuery(query, MakeAnyLenString(&query,
		"SELECT `id`, `skill_id`, `min_skill_value`, `max_skill_value`, `min_aa_points`, `max_aa_points`, `class`, `gender`, "
		"`char_id`, `status`, `item_id`, `prefix`, `suffix`, `title_set` from titles"), errbuf, &result))
	{
		LogFile->write(EQEMuLog::Error, "Unable to load titles: %s : %s", query, errbuf);
		safe_delete_array(query);
		return(false);
	}

	safe_delete_array(query);

	while ((row = mysql_fetch_row(result))) {
		Title.TitleID = atoi(row[0]);
		Title.SkillID = (SkillType) atoi(row[1]);
		Title.MinSkillValue = atoi(row[2]);
		Title.MaxSkillValue = atoi(row[3]);
		Title.MinAAPoints = atoi(row[4]);
		Title.MaxAAPoints = atoi(row[5]);
		Title.Class = atoi(row[6]);
		Title.Gender = atoi(row[7]);
		Title.CharID = atoi(row[8]);
		Title.Status = atoi(row[9]);
		Title.ItemID = atoi(row[10]);
		Title.Prefix = row[11];
		Title.Suffix = row[12];
		Title.TitleSet = atoi(row[13]);
		Titles.push_back(Title);
	}
	mysql_free_result(result);

	return(true);
}

EQApplicationPacket *TitleManager::MakeTitlesPacket(Client *c)
{
	vector<TitleEntry>::iterator Iterator;

	vector<TitleEntry> AvailableTitles;

	uint32 Length = 4;

	Iterator = Titles.begin();

	while(Iterator != Titles.end())
	{
		if(!IsClientEligibleForTitle(c, Iterator))
		{
			++Iterator;
			continue;
		}

		AvailableTitles.push_back((*Iterator));

		Length += Iterator->Prefix.length() + Iterator->Suffix.length() + 6;

		++Iterator;

	}

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_SendTitleList, Length);

	char *Buffer = (char *)outapp->pBuffer;

	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, AvailableTitles.size());

	Iterator = AvailableTitles.begin();

	while(Iterator != AvailableTitles.end())
	{
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, Iterator->TitleID);

		VARSTRUCT_ENCODE_STRING(Buffer, Iterator->Prefix.c_str());

		VARSTRUCT_ENCODE_STRING(Buffer, Iterator->Suffix.c_str());

		++Iterator;
	}
	return(outapp);
}

int TitleManager::NumberOfAvailableTitles(Client *c)
{
	int Count = 0;

	vector<TitleEntry>::iterator Iterator;

	Iterator = Titles.begin();

	while(Iterator != Titles.end())
	{
		if(IsClientEligibleForTitle(c, Iterator))
			++Count;

		++Iterator;
	}

	return Count;
}

string TitleManager::GetPrefix(int TitleID)
{
	vector<TitleEntry>::iterator Iterator;

	Iterator = Titles.begin();

	while(Iterator != Titles.end())
	{
		if((*Iterator).TitleID == TitleID)
			return (*Iterator).Prefix;

		++Iterator;
	}

	return "";
}

string TitleManager::GetSuffix(int TitleID)
{
	vector<TitleEntry>::iterator Iterator;

	Iterator = Titles.begin();

	while(Iterator != Titles.end())
	{
		if((*Iterator).TitleID == TitleID)
			return (*Iterator).Suffix;

		Iterator++;
	}

	return "";
}

bool TitleManager::IsClientEligibleForTitle(Client *c, vector<TitleEntry>::iterator Title)
{
		if((Title->CharID >= 0) && (c->CharacterID() != static_cast<uint32>(Title->CharID)))
			return false;

		if((Title->Status >= 0) && (c->Admin() < Title->Status))
			return false;

		if((Title->Gender >= 0) && (c->GetBaseGender() != Title->Gender))
			return false;

		if((Title->Class >= 0) && (c->GetBaseClass() != Title->Class))
			return false;

		if((Title->MinAAPoints >= 0) && (c->GetAAPointsSpent() < static_cast<uint32>(Title->MinAAPoints)))
			return false;

		if((Title->MaxAAPoints >= 0) && (c->GetAAPointsSpent() > static_cast<uint32>(Title->MaxAAPoints)))
			return false;

		if(Title->SkillID >= 0)
		{
			if((Title->MinSkillValue >= 0)
			   && (c->GetRawSkill(static_cast<SkillType>(Title->SkillID)) < static_cast<uint32>(Title->MinSkillValue)))
				return false;

			if((Title->MaxSkillValue >= 0)
			   && (c->GetRawSkill(static_cast<SkillType>(Title->SkillID)) > static_cast<uint32>(Title->MaxSkillValue)))
				return false;

		}

		if((Title->ItemID >= 1) && (c->GetInv().HasItem(Title->ItemID, 0, 0xFF) == SLOT_INVALID))
			return false;

      	if((Title->TitleSet > 0) && (!c->CheckTitle(Title->TitleSet)))
         	return false;

		return true;
}

bool TitleManager::IsNewAATitleAvailable(int AAPoints, int Class)
{
	vector<TitleEntry>::iterator Iterator;

	Iterator = Titles.begin();

	while(Iterator != Titles.end())
	{
		if((((*Iterator).Class == -1) || ((*Iterator).Class == Class)) && ((*Iterator).MinAAPoints == AAPoints))
			return true;

		++Iterator;
	}

	return false;
}

bool TitleManager::IsNewTradeSkillTitleAvailable(int SkillID, int SkillValue)
{
	vector<TitleEntry>::iterator Iterator;

	Iterator = Titles.begin();

	while(Iterator != Titles.end())
	{
		if(((*Iterator).SkillID == SkillID) && ((*Iterator).MinSkillValue == SkillValue))
			return true;

		++Iterator;
	}

	return false;
}

void TitleManager::CreateNewPlayerTitle(Client *c, const char *Title)
{
	if(!c || !Title)
		return;

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = nullptr;
	MYSQL_RES *result;

	char *EscTitle = new char[strlen(Title) * 2 + 1];

	c->SetAATitle(Title);

	database.DoEscapeString(EscTitle, Title, strlen(Title));

	if (database.RunQuery(query, MakeAnyLenString(&query,
		"SELECT `id` from titles where `prefix` = '%s' and char_id = %i", EscTitle, c->CharacterID()), errbuf, &result))
	{
		if(mysql_num_rows(result) > 0)
		{
			mysql_free_result(result);
			safe_delete_array(query);
			safe_delete_array(EscTitle);
			return;
		}
		mysql_free_result(result);
	}

	safe_delete_array(query);

	if(!database.RunQuery(query,MakeAnyLenString(&query, "INSERT into titles (`char_id`, `prefix`) VALUES(%i, '%s')",
						    c->CharacterID(), EscTitle), errbuf))
		LogFile->write(EQEMuLog::Error, "Error adding title: %s %s", query, errbuf);
	else
	{
		ServerPacket* pack = new ServerPacket(ServerOP_ReloadTitles, 0);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
	safe_delete_array(query);
	safe_delete_array(EscTitle);

}

void TitleManager::CreateNewPlayerSuffix(Client *c, const char *Suffix)
{
	if(!c || !Suffix)
		return;

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = nullptr;
	MYSQL_RES *result;

	char *EscSuffix = new char[strlen(Suffix) * 2 + 1];

	c->SetTitleSuffix(Suffix);

	database.DoEscapeString(EscSuffix, Suffix, strlen(Suffix));

	if (database.RunQuery(query, MakeAnyLenString(&query,
		"SELECT `id` from titles where `suffix` = '%s' and char_id = %i", EscSuffix, c->CharacterID()), errbuf, &result))
	{
		if(mysql_num_rows(result) > 0)
		{
			mysql_free_result(result);
			safe_delete_array(query);
			safe_delete_array(EscSuffix);
			return;
		}
		mysql_free_result(result);
	}

	safe_delete_array(query);

	if(!database.RunQuery(query,MakeAnyLenString(&query, "INSERT into titles (`char_id`, `suffix`) VALUES(%i, '%s')",
						    c->CharacterID(), EscSuffix), errbuf))
		LogFile->write(EQEMuLog::Error, "Error adding title suffix: %s %s", query, errbuf);
	else
	{
		ServerPacket* pack = new ServerPacket(ServerOP_ReloadTitles, 0);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
	safe_delete_array(query);
	safe_delete_array(EscSuffix);

}

void Client::SetAATitle(const char *Title)
{
	strn0cpy(m_pp.title, Title, sizeof(m_pp.title));

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_SetTitleReply, sizeof(SetTitleReply_Struct));

	SetTitleReply_Struct *strs = (SetTitleReply_Struct *)outapp->pBuffer;

	strn0cpy(strs->title, Title, sizeof(strs->title));

	strs->entity_id = GetID();

	entity_list.QueueClients(this, outapp, false);

	safe_delete(outapp);
}

void Client::SetTitleSuffix(const char *Suffix)
{
	strn0cpy(m_pp.suffix, Suffix, sizeof(m_pp.suffix));

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_SetTitleReply, sizeof(SetTitleReply_Struct));

	SetTitleReply_Struct *strs = (SetTitleReply_Struct *)outapp->pBuffer;

	strs->is_suffix = 1;

	strn0cpy(strs->title, Suffix, sizeof(strs->title));

	strs->entity_id = GetID();

	entity_list.QueueClients(this, outapp, false);

	safe_delete(outapp);
}

void Client::EnableTitle(int titleset) {

   if (CheckTitle(titleset)) {
      return;
   }

   char errbuf[MYSQL_ERRMSG_SIZE];
   char *query = 0;

   if(!database.RunQuery(query,MakeAnyLenString(&query, "INSERT INTO player_titlesets (char_id, title_set) VALUES (%i, %i)", CharacterID(), titleset), errbuf)) {
      LogFile->write(EQEMuLog::Error, "Error in EnableTitle query for titleset %i and charid %i", titleset, CharacterID());
      safe_delete_array(query);
      return;
   }
   else {
      safe_delete_array(query);
      return;
   }
}

bool Client::CheckTitle(int titleset) {

   char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;

   if (database.RunQuery(query, MakeAnyLenString(&query, "SELECT `id` FROM player_titlesets WHERE `title_set`=%i AND `char_id`=%i LIMIT 1", titleset, CharacterID()), errbuf, &result)) {
      safe_delete_array(query);
      if (mysql_num_rows(result) >= 1) {
         mysql_free_result(result);
         return(true);
      }
         mysql_free_result(result);
   }

   else {
      LogFile->write(EQEMuLog::Error, "Error in CheckTitle query '%s': %s", query,  errbuf);
      safe_delete_array(query);
   }

   return(false);
}

void Client::RemoveTitle(int titleset) {

   if (!CheckTitle(titleset)) {
      return;
   }

   char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;

   if (database.RunQuery(query, MakeAnyLenString(&query, "DELETE FROM player_titlesets WHERE `title_set`=%i AND `char_id`=%i", titleset, CharacterID()), errbuf)) {
      safe_delete_array(query);
   }

   else {
      LogFile->write(EQEMuLog::Error, "Error in RemoveTitle query '%s': %s", query,  errbuf);
      safe_delete_array(query);
   }

   return;
}

