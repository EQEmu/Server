/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#ifdef _WINDOWS
#include <process.h>
#else
#include <pthread.h>
#endif

#ifdef _WINDOWS
	#define snprintf	_snprintf
	#define strncasecmp	_strnicmp
	#define strcasecmp	_stricmp
#endif
#include "../common/StringUtil.h"
#include "../common/packet_functions.h"
#include "../common/packet_dump.h"
#include "../common/packet_dump_file.h"
#include "../common/emu_opcodes.h"
#include "../common/eq_packet_structs.h"
#include "../common/servertalk.h"
#include "entity.h"
#include "masterentity.h"

#include "petitions.h"
#include "worldserver.h"

PetitionList petition_list;

extern WorldServer worldserver;


void Petition::SendPetitionToPlayer(Client* clientto) {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_PetitionCheckout,sizeof(Petition_Struct));
	Petition_Struct* pet = (Petition_Struct*) outapp->pBuffer;
	strcpy(pet->accountid,this->GetAccountName());
	strcpy(pet->lastgm,this->GetLastGM());
	strcpy(pet->charname,this->GetCharName());
	pet->petnumber = this->petid;
	pet->charclass = this->GetCharClass();
	pet->charlevel = this->GetCharLevel();
	pet->charrace = this->GetCharRace();
	pet->zone = this->GetZone();
	//strcpy(pet->zone,this->GetZone());
	strcpy(pet->petitiontext,this->GetPetitionText());
	pet->checkouts = this->GetCheckouts();
	pet->unavail = this->GetUnavails();
	pet->senttime = this->GetSentTime();
	//memset(pet->unknown5, 0, sizeof(pet->unknown5));
	//pet->unknown5[3] = 0x1f;
	pet->urgency = this->GetUrgency();
	strcpy(pet->gmtext, this->GetGMText());
	clientto->QueuePacket(outapp);
	safe_delete(outapp);
	return;
}

Petition::Petition(uint32 id)
{
	petid = id;
	charclass = 0;
	charrace = 0;
	charlevel = 0;
	checkouts = 0;
	unavailables = 0;
	urgency = 0;
	time(&senttime);
	ischeckedout = false;
	memset(accountname, 0, sizeof(accountname));
	memset(charname, 0, sizeof(charname));
	memset(lastgm, 0, sizeof(lastgm));
	memset(petitiontext, 0, sizeof(petitiontext));
	memset(gmtext, 0, sizeof(gmtext));

	//memset(this->zone, 0, sizeof(this->zone));
	zone = 1;
}
Petition* PetitionList::GetPetitionByID(uint32 id_in) {
	LinkedListIterator<Petition*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->GetID() == id_in)
			return iterator.GetData();
		iterator.Advance();
	}
	return 0;
}
uint32 PetitionList::GetTotalPetitions(){
	LinkedListIterator<Petition*> iterator(list);
	iterator.Reset();
	uint32 total=0;
	while(iterator.MoreElements()) {
		total++;
		iterator.Advance();
	}
	return total;
}
bool PetitionList::FindPetitionByAccountName(const char* acctname) {
	LinkedListIterator<Petition*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (!strcmp(acctname,iterator.GetData()->GetAccountName()))
			return true;
		iterator.Advance();
	}
	return false;
}
bool PetitionList::DeletePetitionByCharName(const char* charname) {
	LinkedListIterator<Petition*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (!strcmp(charname,iterator.GetData()->GetCharName())) {
			if(DeletePetition(iterator.GetData()->GetID())==0)
				return true;
			else
				return false;
		}
		iterator.Advance();
	}
	return false;
}
void PetitionList::UpdateZoneListQueue() {
	ServerPacket* pack = new ServerPacket(ServerOP_Petition, sizeof(ServerPetitionUpdate_Struct));
	ServerPetitionUpdate_Struct* pupdate = (ServerPetitionUpdate_Struct*) pack->pBuffer;
	pupdate->petid = 0x00;
	pupdate->status = 0x00;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void PetitionList::AddPetition(Petition* pet) {
	list.Insert(pet);
	return;
}

//Return Values: 0 = Ok ; -1 = Error deleting petition.
int PetitionList::DeletePetition(uint32 petnumber) {
	LinkedListIterator<Petition*> iterator(list);
	iterator.Reset();
	LockMutex lock(&PList_Mutex);
	while(iterator.MoreElements()) {
		if (iterator.GetData()->GetID() == petnumber) {
			database.DeletePetitionFromDB(iterator.GetData());
			iterator.RemoveCurrent();
			return 0;
			break;
		}
		else {
			iterator.Advance();
		}
	}
	return -1;
}

void PetitionList::UpdateGMQueue() {
	LinkedListIterator<Petition*> iterator(list);
	iterator.Reset();
	uint32 total=0;
	while(iterator.MoreElements()) {
		total++;
		entity_list.SendPetitionToAdmins(iterator.GetData());
		iterator.Advance();
	}
	if(total==0)
		entity_list.SendPetitionToAdmins();
	return;
}

void PetitionList::ClearPetitions() {
	//	entity_list.ClearClientPetitionQueue();
	LinkedListIterator<Petition*> iterator(list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		iterator.RemoveCurrent(true);
		iterator.Advance();
	}
	return;
}

void PetitionList::ReadDatabase() {
	LockMutex lock(&PList_Mutex);
	ClearPetitions();
	database.RefreshPetitionsFromDB();
	UpdateGMQueue();
	return;
}

void PetitionList::UpdatePetition(Petition* pet) {
	LockMutex lock(&PList_Mutex);
	database.UpdatePetitionToDB(pet);
	return;
}

void ZoneDatabase::DeletePetitionFromDB(Petition* wpet) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	uint32 affected_rows = 0;
	uint8 checkedout = 0;
	if (wpet->CheckedOut()) checkedout = 0;
	else checkedout = 1;
	if (!RunQuery(query, MakeAnyLenString(&query, "DELETE from petitions where petid = %i", wpet->GetID()), errbuf, 0, &affected_rows)) {
		LogFile->write(EQEMuLog::Error, "Error in DeletePetitionFromDB query '%s': %s", query, errbuf);
	}
	safe_delete_array(query);

	return;
}

void ZoneDatabase::UpdatePetitionToDB(Petition* wpet) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	uint32 affected_rows = 0;
	uint8 checkedout = 0;
	if (wpet->CheckedOut()) checkedout = 1;
	else checkedout = 0;
	if (!RunQuery(query, MakeAnyLenString(&query, "UPDATE petitions set gmtext = '%s', lastgm = '%s', urgency = %i, checkouts = %i, unavailables = %i, ischeckedout = %i where petid = %i", wpet->GetGMText(), wpet->GetLastGM(), wpet->GetUrgency(), wpet->GetCheckouts(), wpet->GetUnavails(), checkedout, wpet->GetID()), errbuf, 0, &affected_rows)) {
		LogFile->write(EQEMuLog::Error, "Error in UpdatePetitionToDB query '%s': %s", query, errbuf);
	}
	safe_delete_array(query);
	return;
}



void ZoneDatabase::InsertPetitionToDB(Petition* wpet)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	uint32 affected_rows = 0;
	uint8 checkedout = 0;
	if (wpet->CheckedOut())
		checkedout = 1;
	else
		checkedout = 0;

	uint32 len = strlen(wpet->GetPetitionText());
	char* petitiontext = new char[2*len+1];
	memset(petitiontext, 0, 2*len+1);
	DoEscapeString(petitiontext, wpet->GetPetitionText(), len);
	if (!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO petitions (petid, charname, accountname, lastgm, petitiontext, zone, urgency, charclass, charrace, charlevel, checkouts, unavailables, ischeckedout, senttime, gmtext) values (%i,'%s','%s','%s','%s',%i,%i,%i,%i,%i,%i,%i,%i,%i, '%s')", wpet->GetID(), wpet->GetCharName(), wpet->GetAccountName(), wpet->GetLastGM(), petitiontext, wpet->GetZone(), wpet->GetUrgency(), wpet->GetCharClass(), wpet->GetCharRace(), wpet->GetCharLevel(), wpet->GetCheckouts(), wpet->GetUnavails(), checkedout, wpet->GetSentTime(), wpet->GetGMText()), errbuf, 0, &affected_rows)) {
		LogFile->write(EQEMuLog::Error, "Error in InsertPetitionToDB query '%s': %s", query, errbuf);
	}

	safe_delete_array(petitiontext);
	safe_delete_array(query);
#if EQDEBUG >= 5
		LogFile->write(EQEMuLog::Debug, "New petition created");
#endif
	return;
}

void ZoneDatabase::RefreshPetitionsFromDB()
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	Petition* newpet;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT petid, charname, accountname, lastgm, petitiontext, zone, urgency, charclass, charrace, charlevel, checkouts, unavailables, ischeckedout, senttime, gmtext from petitions order by petid"), errbuf, &result))
	{
		safe_delete_array(query);
		while ((row = mysql_fetch_row(result))) {
			newpet = new Petition(atoi(row[0]));
			newpet->SetCName(row[1]);
			newpet->SetAName(row[2]);
			newpet->SetLastGM(row[3]);
			newpet->SetPetitionText(row[4]);
			newpet->SetZone(atoi(row[5]));
			newpet->SetUrgency(atoi(row[6]));
			newpet->SetClass(atoi(row[7]));
			newpet->SetRace(atoi(row[8]));
			newpet->SetLevel(atoi(row[9]));
			newpet->SetCheckouts(atoi(row[10]));
			newpet->SetUnavails(atoi(row[11]));
			newpet->SetSentTime2(atol(row[13]));
			newpet->SetGMText(row[14]);
			std::cout << "Petition " << row[0] << " pettime = " << newpet->GetSentTime() << std::endl;
			if (atoi(row[12]) == 1) newpet->SetCheckedOut(true);
			else newpet->SetCheckedOut(false);
			petition_list.AddPetition(newpet);
		}
		mysql_free_result(result);
	}
	else {
		LogFile->write(EQEMuLog::Error, "Error in RefreshPetitionsFromDB query '%s': %s", query, errbuf);
		safe_delete_array(query);
		return;
	}

	return;
}
