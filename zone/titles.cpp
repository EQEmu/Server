/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2005 EQEMu Development Team (http://eqemulator.net)

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

#include "../common/eq_packet_structs.h"
#include "../common/string_util.h"
#include "../common/misc_functions.h"

#include "client.h"
#include "entity.h"
#include "mob.h"

#include "titles.h"
#include "worldserver.h"

extern WorldServer worldserver;

TitleManager::TitleManager() {
}

bool TitleManager::LoadTitles()
{
	Titles.clear();

	std::string query = "SELECT `id`, `skill_id`, `min_skill_value`, `max_skill_value`, "
                        "`min_aa_points`, `max_aa_points`, `class`, `gender`, `char_id`, "
                        "`status`, `item_id`, `prefix`, `suffix`, `title_set` FROM titles";
    auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
        TitleEntry Title;
		Title.TitleID = atoi(row[0]);
		Title.SkillID = (EQEmu::skills::SkillType) atoi(row[1]);
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

	return true;
}

EQApplicationPacket *TitleManager::MakeTitlesPacket(Client *c)
{
	std::vector<TitleEntry>::iterator Iterator;

	std::vector<TitleEntry> AvailableTitles;

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

	auto outapp = new EQApplicationPacket(OP_SendTitleList, Length);

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

	std::vector<TitleEntry>::iterator Iterator;

	Iterator = Titles.begin();

	while(Iterator != Titles.end())
	{
		if(IsClientEligibleForTitle(c, Iterator))
			++Count;

		++Iterator;
	}

	return Count;
}

std::string TitleManager::GetPrefix(int TitleID)
{
	std::vector<TitleEntry>::iterator Iterator;

	Iterator = Titles.begin();

	while(Iterator != Titles.end())
	{
		if((*Iterator).TitleID == TitleID)
			return (*Iterator).Prefix;

		++Iterator;
	}

	return "";
}

std::string TitleManager::GetSuffix(int TitleID)
{
	std::vector<TitleEntry>::iterator Iterator;

	Iterator = Titles.begin();

	while(Iterator != Titles.end())
	{
		if((*Iterator).TitleID == TitleID)
			return (*Iterator).Suffix;

		++Iterator;
	}

	return "";
}

bool TitleManager::IsClientEligibleForTitle(Client *c, std::vector<TitleEntry>::iterator Title)
{
		if((Title->CharID >= 0) && (c->CharacterID() != static_cast<uint32>(Title->CharID)))
			return false;

		if((Title->Status >= 0) && (c->Admin() < Title->Status))
			return false;

		if((Title->Gender >= 0) && (c->GetBaseGender() != Title->Gender))
			return false;

		if((Title->Class >= 0) && (c->GetBaseClass() != Title->Class))
			return false;

		if((Title->MinAAPoints >= 0) && (c->GetSpentAA() < static_cast<uint32>(Title->MinAAPoints)))
			return false;

		if((Title->MaxAAPoints >= 0) && (c->GetSpentAA() > static_cast<uint32>(Title->MaxAAPoints)))
			return false;

		if(Title->SkillID >= 0)
		{
			if ((Title->MinSkillValue >= 0) && (c->GetRawSkill(static_cast<EQEmu::skills::SkillType>(Title->SkillID)) < static_cast<uint32>(Title->MinSkillValue)))
				return false;

			if ((Title->MaxSkillValue >= 0) && (c->GetRawSkill(static_cast<EQEmu::skills::SkillType>(Title->SkillID)) > static_cast<uint32>(Title->MaxSkillValue)))
				return false;

		}

		if ((Title->ItemID >= 1) && (c->GetInv().HasItem(Title->ItemID, 0, 0xFF) == INVALID_INDEX))
			return false;

		if((Title->TitleSet > 0) && (!c->CheckTitle(Title->TitleSet)))
			return false;

		return true;
}

bool TitleManager::IsNewAATitleAvailable(int AAPoints, int Class)
{
	std::vector<TitleEntry>::iterator Iterator;

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
	std::vector<TitleEntry>::iterator Iterator;

	Iterator = Titles.begin();

	while(Iterator != Titles.end())
	{
		if(((*Iterator).SkillID == SkillID) && ((*Iterator).MinSkillValue == SkillValue))
			return true;

		++Iterator;
	}

	return false;
}

void TitleManager::CreateNewPlayerTitle(Client *client, const char *title)
{
	if(!client || !title)
		return;

	auto escTitle = new char[strlen(title) * 2 + 1];

	client->SetAATitle(title);

	database.DoEscapeString(escTitle, title, strlen(title));
    auto query = StringFormat("SELECT `id` FROM titles "
                            "WHERE `prefix` = '%s' AND char_id = %i",
                            escTitle, client->CharacterID());
    auto results = database.QueryDatabase(query);
	if (results.Success() && results.RowCount() > 0){
        safe_delete_array(escTitle);
        return;
	}

	query = StringFormat("INSERT INTO titles (`char_id`, `prefix`) VALUES(%i, '%s')",
							client->CharacterID(), escTitle);
    safe_delete_array(escTitle);
    results = database.QueryDatabase(query);
	if(!results.Success()) {
        return;
    }

    auto pack = new ServerPacket(ServerOP_ReloadTitles, 0);
    worldserver.SendPacket(pack);
    safe_delete(pack);
}

void TitleManager::CreateNewPlayerSuffix(Client *client, const char *suffix)
{
	if(!client || !suffix)
		return;

    client->SetTitleSuffix(suffix);

    auto escSuffix = new char[strlen(suffix) * 2 + 1];
    database.DoEscapeString(escSuffix, suffix, strlen(suffix));

    std::string query = StringFormat("SELECT `id` FROM titles "
                                    "WHERE `suffix` = '%s' AND char_id = %i",
                                    escSuffix, client->CharacterID());
    auto results = database.QueryDatabase(query);
	if (results.Success() && results.RowCount() > 0) {
			safe_delete_array(escSuffix);
			return;
    }

    query = StringFormat("INSERT INTO titles (`char_id`, `suffix`) VALUES(%i, '%s')",
                        client->CharacterID(), escSuffix);
    safe_delete_array(escSuffix);
    results = database.QueryDatabase(query);
	if(!results.Success()) {
        return;
    }

    auto pack = new ServerPacket(ServerOP_ReloadTitles, 0);
    worldserver.SendPacket(pack);
    safe_delete(pack);
}

void Client::SetAATitle(const char *Title)
{
	strn0cpy(m_pp.title, Title, sizeof(m_pp.title));

	auto outapp = new EQApplicationPacket(OP_SetTitleReply, sizeof(SetTitleReply_Struct));

	SetTitleReply_Struct *strs = (SetTitleReply_Struct *)outapp->pBuffer;

	strn0cpy(strs->title, Title, sizeof(strs->title));

	strs->entity_id = GetID();

	entity_list.QueueClients(this, outapp, false);

	safe_delete(outapp);
}

void Client::SetTitleSuffix(const char *Suffix)
{
	strn0cpy(m_pp.suffix, Suffix, sizeof(m_pp.suffix));

	auto outapp = new EQApplicationPacket(OP_SetTitleReply, sizeof(SetTitleReply_Struct));

	SetTitleReply_Struct *strs = (SetTitleReply_Struct *)outapp->pBuffer;

	strs->is_suffix = 1;

	strn0cpy(strs->title, Suffix, sizeof(strs->title));

	strs->entity_id = GetID();

	entity_list.QueueClients(this, outapp, false);

	safe_delete(outapp);
}

void Client::EnableTitle(int titleSet) {

	if (CheckTitle(titleSet))
		return;

	std::string query = StringFormat("INSERT INTO player_titlesets "
                                    "(char_id, title_set) VALUES (%i, %i)",
                                    CharacterID(), titleSet);
    auto results = database.QueryDatabase(query);
	if(!results.Success())
		Log(Logs::General, Logs::Error, "Error in EnableTitle query for titleset %i and charid %i", titleSet, CharacterID());

}

bool Client::CheckTitle(int titleSet) {

	std::string query = StringFormat("SELECT `id` FROM player_titlesets "
                                    "WHERE `title_set`=%i AND `char_id`=%i LIMIT 1",
                                    titleSet, CharacterID());
    auto results = database.QueryDatabase(query);
	if (!results.Success()) {
        return false;
	}

	if (results.RowCount() == 0)
        return false;

	return true;
}

void Client::RemoveTitle(int titleSet) {

	if (!CheckTitle(titleSet))
		return;

	std::string query = StringFormat("DELETE FROM player_titlesets "
                                    "WHERE `title_set` = %i AND `char_id` = %i",
                                    titleSet, CharacterID());
   database.QueryDatabase(query);
}

