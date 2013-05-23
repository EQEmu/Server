#include "../common/debug.h"
#include "../common/servertalk.h"
#include "../common/extprofile.h"
#include "../common/rulesys.h"
#include "../common/MiscFunctions.h"
#include "../common/StringUtil.h"
#include "Adventure.h"
#include "AdventureManager.h"
#include "worlddb.h"
#include "zonelist.h"
#include "clientlist.h"
#include "cliententry.h"

extern ZSList zoneserver_list;
extern ClientList client_list;
extern AdventureManager adventure_manager;

Adventure::Adventure(AdventureTemplate *t)
{
	adventure_template = t;
	status = AS_WaitingForZoneIn;
	current_timer = new Timer(1000 * t->zone_in_time);
	count = 0;
	assassination_count = 0;
	instance_id = 0;
}

Adventure::Adventure(AdventureTemplate *t, int count, int assassination_count, AdventureStatus status, uint16 instance_id, uint32 time_left)
{
	adventure_template = t;
	this->count = count;
	this->assassination_count = assassination_count;
	this->status = status;
	this->instance_id = instance_id;

	if(status == AS_Finished)
	{
		database.SetInstanceDuration(instance_id, time_left);
	}
	else
	{
		database.SetInstanceDuration(instance_id, time_left + 60);
	}

	current_timer = new Timer(1000 * time_left);
}

Adventure::~Adventure()
{
	safe_delete(current_timer);
}

void Adventure::AddPlayer(std::string character_name, bool add_client_to_instance)
{
	if(!PlayerExists(character_name))
	{
		int client_id = database.GetCharacterID(character_name.c_str());
		if(add_client_to_instance)
		{
			database.AddClientToInstance(instance_id, client_id);
		}
		players.push_back(character_name);
	}
}

void Adventure::RemovePlayer(std::string character_name)
{
	std::list<std::string>::iterator iter = players.begin();
	while(iter != players.end())
	{
		if((*iter).compare(character_name) == 0)
		{
			database.RemoveClientFromInstance(instance_id, database.GetCharacterID(character_name.c_str()));
			players.erase(iter);
			return;
		}
		iter++;
	}
}

bool Adventure::PlayerExists(std::string character_name)
{
	std::list<std::string>::iterator iter = players.begin();
	while(iter != players.end())
	{
		if(character_name.compare((*iter)) == 0)
		{
			return true;
		}
		iter++;
	}
	return false;
}

bool Adventure::IsActive()
{
	return (status != AS_Finished);
}

bool Adventure::Process()
{
	if(players.size() == 0)
	{
		return false;
	}

	if(current_timer->Check())
	{
		//Timer wore out while waiting for zone in.
		if(status == AS_WaitingForZoneIn)
		{
			MoveCorpsesToGraveyard();
			database.DeleteInstance(instance_id);
			return false;
		}
		else if(status == AS_WaitingForPrimaryEndTime)
		{
			//Do partial failure: send a message to the clients that they can only get a certain amount of points.
			SendAdventureMessage(13, "You failed to complete your adventure in time. Complete your adventure goal within 30 minutes to "
				"receive a lesser reward. This adventure will end in 30 minutes and your party will be ejected from the dungeon.");
			SetStatus(AS_WaitingForSecondaryEndTime);
		}
		else
		{
			if(count < GetTemplate()->type_count)
			{
				Finished(AWS_Lose);
			}

			MoveCorpsesToGraveyard();
			database.DeleteInstance(instance_id);
			return false;
		}
	}
	return true;
}

bool Adventure::CreateInstance()
{
	uint32 zone_id = database.GetZoneID(adventure_template->zone);
	if(!zone_id)
	{
		return false;
	}

	uint16 id = 0;
	if(!database.GetUnusedInstanceID(id))
	{
		return false;
	}

	if(!database.CreateInstance(id, zone_id, adventure_template->zone_version, adventure_template->zone_in_time + 60))
	{
		return false;
	}

	instance_id = id;
	return true;
}

void Adventure::SetStatus(AdventureStatus new_status)
{
	if(new_status == AS_WaitingForPrimaryEndTime)
	{
		status = new_status;
		safe_delete(current_timer);
		current_timer = new Timer(adventure_template->duration * 1000);
		database.SetInstanceDuration(instance_id, adventure_template->duration + 60);
		ServerPacket *pack = new ServerPacket(ServerOP_InstanceUpdateTime, sizeof(ServerInstanceUpdateTime_Struct));
		ServerInstanceUpdateTime_Struct *ut = (ServerInstanceUpdateTime_Struct*)pack->pBuffer;
		ut->instance_id = instance_id;
		ut->new_duration = adventure_template->duration + 60;

		pack->Deflate();
		zoneserver_list.SendPacket(0, instance_id, pack);
		safe_delete(pack);
	}
	else if(new_status == AS_WaitingForSecondaryEndTime)
	{
		status = new_status;
		safe_delete(current_timer);
		current_timer = new Timer(1800000);
		database.SetInstanceDuration(instance_id, 1860);
		ServerPacket *pack = new ServerPacket(ServerOP_InstanceUpdateTime, sizeof(ServerInstanceUpdateTime_Struct));
		ServerInstanceUpdateTime_Struct *ut = (ServerInstanceUpdateTime_Struct*)pack->pBuffer;
		ut->instance_id = instance_id;
		ut->new_duration = 1860;

		pack->Deflate();
		zoneserver_list.SendPacket(0, instance_id, pack);
		safe_delete(pack);
	}
	else if(new_status == AS_Finished)
	{
		status = new_status;
		safe_delete(current_timer);
		current_timer = new Timer(1800000);
		database.SetInstanceDuration(instance_id, 1800);
		ServerPacket *pack = new ServerPacket(ServerOP_InstanceUpdateTime, sizeof(ServerInstanceUpdateTime_Struct));
		ServerInstanceUpdateTime_Struct *ut = (ServerInstanceUpdateTime_Struct*)pack->pBuffer;
		ut->instance_id = instance_id;
		ut->new_duration = 1860;

		pack->Deflate();
		zoneserver_list.SendPacket(0, instance_id, pack);
		safe_delete(pack);
	}
	else
	{
		return;
	}

	std::list<std::string>::iterator iter = players.begin();
	while(iter != players.end())
	{
		adventure_manager.GetAdventureData((*iter).c_str());
		iter++;
	}
}

void Adventure::SendAdventureMessage(uint32 type, const char *msg)
{
	ServerPacket *pack = new ServerPacket(ServerOP_EmoteMessage, sizeof(ServerEmoteMessage_Struct) + strlen(msg) + 1);
	ServerEmoteMessage_Struct *sms = (ServerEmoteMessage_Struct*)pack->pBuffer;
	sms->type = type;
	strcpy(sms->message, msg);
	std::list<std::string>::iterator iter = players.begin();
	while(iter != players.end())
	{
		ClientListEntry *current = client_list.FindCharacter((*iter).c_str());
		if(current)
		{
			strcpy(sms->to, (*iter).c_str());
			zoneserver_list.SendPacket(current->zone(), current->instance(), pack);
		}
		iter++;
	}
	delete pack;
}

void Adventure::IncrementCount()
{
	const AdventureTemplate *at = GetTemplate();
	if(count >= at->type_count)
	{
		return;
	}

	if(status == AS_WaitingForPrimaryEndTime)
	{
		count++;
		if(count == at->type_count)
		{
			SetStatus(AS_Finished);
			Finished(AWS_Win);
		}
	}
	else if(status == AS_WaitingForSecondaryEndTime)
	{
		count++;
		if(count == at->type_count)
		{
			SetStatus(AS_Finished);
			Finished(AWS_SecondPlace);
		}
	}
}

void Adventure::IncrementAssassinationCount()
{
	if(count >= RuleI(Adventure, NumberKillsForBossSpawn))
	{
		return;
	}

	assassination_count++;
}

void Adventure::Finished(AdventureWinStatus ws)
{
	std::list<std::string>::iterator iter = players.begin();
	while(iter != players.end())
	{
		ClientListEntry *current = client_list.FindCharacter((*iter).c_str());
		if(current)
		{
			if(current->Online() == CLE_Status_InZone)
			{
				//We can send our packets only.
				ServerPacket *pack = new ServerPacket(ServerOP_AdventureFinish, sizeof(ServerAdventureFinish_Struct));
				ServerAdventureFinish_Struct *af = (ServerAdventureFinish_Struct*)pack->pBuffer;
				strcpy(af->player, (*iter).c_str());
				af->theme = GetTemplate()->theme;
				if(ws == AWS_Win)
				{
					af->win = true;
					af->points = GetTemplate()->win_points;
				}
				else if(ws == AWS_SecondPlace)
				{
					af->win = true;
					af->points = GetTemplate()->lose_points;
				}
				else
				{
					af->win = false;
					af->points = 0;
				}
				pack->Deflate();
				zoneserver_list.SendPacket(current->zone(), current->instance(), pack);
				database.UpdateAdventureStatsEntry(database.GetCharacterID((*iter).c_str()), GetTemplate()->theme, (ws != AWS_Lose) ? true : false);
				delete pack;
			}
			else
			{
				AdventureFinishEvent afe;
				afe.name = (*iter);
				if(ws == AWS_Win)
				{
					afe.theme = GetTemplate()->theme;
					afe.points = GetTemplate()->win_points;
					afe.win = true;
				}
				else if(ws == AWS_SecondPlace)
				{
					afe.theme = GetTemplate()->theme;
					afe.points = GetTemplate()->lose_points;
					afe.win = true;
				}
				else
				{
					afe.win = false;
					afe.points = 0;
				}
				adventure_manager.AddFinishedEvent(afe);
				database.UpdateAdventureStatsEntry(database.GetCharacterID((*iter).c_str()), GetTemplate()->theme, (ws != AWS_Lose) ? true : false);
			}
		}
		else
		{
			AdventureFinishEvent afe;
			afe.name = (*iter);
			if(ws == AWS_Win)
			{
				afe.theme = GetTemplate()->theme;
				afe.points = GetTemplate()->win_points;
				afe.win = true;
			}
			else if(ws == AWS_SecondPlace)
			{
				afe.theme = GetTemplate()->theme;
				afe.points = GetTemplate()->lose_points;
				afe.win = true;
			}
			else
			{
				afe.win = false;
				afe.points = 0;
			}
			adventure_manager.AddFinishedEvent(afe);
			database.UpdateAdventureStatsEntry(database.GetCharacterID((*iter).c_str()), GetTemplate()->theme, (ws != AWS_Lose) ? true : false);
		}
		iter++;
	}
	adventure_manager.GetAdventureData(this);
}

void Adventure::MoveCorpsesToGraveyard()
{
	if(GetTemplate()->graveyard_zone_id == 0)
	{
		return;
	}

	std::list<uint32> dbid_list;
	std::list<uint32> charid_list;
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if(database.RunQuery(query,MakeAnyLenString(&query,"SELECT id, charid FROM player_corpses WHERE instanceid=%d", GetInstanceID()), errbuf, &result))
	{
		while((row = mysql_fetch_row(result)))
		{
			dbid_list.push_back(atoi(row[0]));
			charid_list.push_back(atoi(row[1]));
		}
		mysql_free_result(result);
		safe_delete_array(query);
	}
	else
	{
		LogFile->write(EQEMuLog::Error, "Error in AdventureManager:::MoveCorpsesToGraveyard: %s (%s)", query, errbuf);
		safe_delete_array(query);
	}

	std::list<uint32>::iterator iter = dbid_list.begin();
	while(iter != dbid_list.end())
	{
		float x = GetTemplate()->graveyard_x + MakeRandomFloat(-GetTemplate()->graveyard_radius, GetTemplate()->graveyard_radius);
		float y = GetTemplate()->graveyard_y + MakeRandomFloat(-GetTemplate()->graveyard_radius, GetTemplate()->graveyard_radius);
		float z = GetTemplate()->graveyard_z;
		if(database.RunQuery(query,MakeAnyLenString(&query, "UPDATE player_corpses SET zoneid=%d, instanceid=0, x=%f, y=%f, z=%f WHERE instanceid=%d",
			GetTemplate()->graveyard_zone_id, x, y, z, GetInstanceID()), errbuf))
		{
			safe_delete_array(query);
		}
		else
		{
			LogFile->write(EQEMuLog::Error, "Error in AdventureManager:::MoveCorpsesToGraveyard: %s (%s)", query, errbuf);
			safe_delete_array(query);
		}
		iter++;
	}

	iter = dbid_list.begin();
	std::list<uint32>::iterator c_iter = charid_list.begin();
	while(iter != dbid_list.end())
	{
		ServerPacket* pack = new ServerPacket(ServerOP_DepopAllPlayersCorpses, sizeof(ServerDepopAllPlayersCorpses_Struct));
		ServerDepopAllPlayersCorpses_Struct *dpc = (ServerDepopAllPlayersCorpses_Struct*)pack->pBuffer;
		dpc->CharacterID = (*c_iter);
		dpc->InstanceID = 0;
		dpc->ZoneID = GetTemplate()->graveyard_zone_id;
		zoneserver_list.SendPacket(0, GetInstanceID(), pack);
		delete pack;

		pack = new ServerPacket(ServerOP_SpawnPlayerCorpse, sizeof(SpawnPlayerCorpse_Struct));
		SpawnPlayerCorpse_Struct* spc = (SpawnPlayerCorpse_Struct*)pack->pBuffer;
		spc->player_corpse_id = (*iter);
		spc->zone_id = GetTemplate()->graveyard_zone_id;

		zoneserver_list.SendPacket(spc->zone_id, 0, pack);
		delete pack;
		iter++;
		c_iter++;
	}
}

