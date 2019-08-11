/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2004 EQEMu Development Team (http://eqemu.org)

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
#include <iostream>
using namespace std;
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
	#ifdef _CRTDBG_MAP_ALLOC
		#undef new
	#endif
#include <fstream>
	#ifdef _CRTDBG_MAP_ALLOC
		#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
	#endif
using namespace std;
#ifdef WIN32
#include <conio.h>
#define snprintf	_snprintf
#define vsnprintf	_vsnprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#endif

bool spells_loaded = false;
volatile bool RunLoops = true;
extern volatile bool ZoneLoaded;
#ifdef SHAREMEM
	#include "../common/EMuShareMem.h"
	extern LoadEMuShareMemDLL EMuShareMemDLL;
	#ifndef WIN32
		#include <sys/types.h>
		#include <sys/ipc.h>
		#include <sys/sem.h>
		#include <sys/shm.h>
#ifndef FREEBSD
	union semun {
		int val;
		struct semid_ds *buf;
		ushort *array;
		struct seminfo *__buf;
		void *__pad;
	};
#endif
	#endif
#endif




#include "../common/queue.h"
#include "../common/timer.h"
#include "../common/eq_stream.h"
#include "../common/eq_packet_structs.h"
#include "../common/mutex.h"
#include "../common/version.h"
#include "../common/files.h"
#include "../common/eqemu_error.h"
#include "../common/packet_dump_file.h"

#include "masterentity.h"
#include "worldserver.h"
#include "net.h"
#include "spdat.h"
#include "zone.h"
#include "command.h"
#include "parser.h"
#include "embparser.h"


#ifndef NEW_LoadSPDat
void LoadSPDat(SPDat_Spell_Struct** SpellsPointer) {
	//FILE *fp;
	//cout << "Beginning Spells memset." << endl;
	int u;
	//for (u = 0; u < SPDAT_RECORDS; u++)
	//{	//cout << u << ' ';
	memset((char*) &spells,0,sizeof(SPDat_Spell_Struct)*SPDAT_RECORDS);
	//}
	//cout << "Memset finished\n";
	char temp=' ';
	int tempid=0;
	char token[64]="";
	int a = 0;
	char sep='^';
	LogFile->write(EQEMuLog::Normal, "If this is the last message you see, you forgot to move spells_en.txt from your EQ dir to this dir.");

#ifdef FREEBSD
#error ifstreams seem to break BSD...
#endif
	ifstream in;in.open(SPELLS_FILE);

	if(!in.is_open()){
		LogFile->write(EQEMuLog::Error, "File '%s' not found in same directory as zone.exe, spell loading FAILED!", SPELLS_FILE);
		return;
	}
	//while(!in.eof())
	//{in >> temp;}
	//for(int x =0; x< spellsen_size; x++)
	//	memset((char*) &spells[x],0,sizeof(SPDat_Spell_Struct));
	while(tempid <= SPDAT_RECORDS-1)
	{
		//if(tempid>3490)
		//{
		//	cout << "BLEH";
		//	getch();
		//}

		//in.getline(&temp, 624);
		in.get(temp);
		while(chrcmpI(&temp, &sep))
		{
			strncat(token,&temp,1);
			a++;//cout << temp<< ' ';
			in.get(temp);
		}
		tempid=atoi(token);
		if(tempid>=SPDAT_RECORDS)
			break;
		//cout << "TempID: " << tempid << endl;
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		strncpy(spells[tempid].name,token,a);
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;
		//cout << spells[tempid].name << '^';
		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		strncpy(spells[tempid].player_1,token,a);
		//cout << spells[tempid].player_1 << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		strncpy(spells[tempid].teleport_zone,token,a);
		//cout << spells[tempid].teleport_zone << '^';
		a=0;

		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		strncpy(spells[tempid].you_cast,token,a);
		//cout << spells[tempid].you_cast << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		strncpy(spells[tempid].other_casts,token,a);
		//cout << spells[tempid].other_casts << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;
		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		strncpy(spells[tempid].cast_on_you,token,a);
		//cout << spells[tempid].cast_on_you << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		strncpy(spells[tempid].cast_on_other,token,a);
		//cout << spells[tempid].cast_on_other << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		strncpy(spells[tempid].spell_fades,token,a);
		//cout << spells[tempid].spell_fades << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			in.get(temp);
		}
		spells[tempid].range=atof(token);
		//cout << spells[tempid].range << '^';
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].aoerange=atof(token);
		//cout << spells[tempid].aoerange << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].pushback=atof(token);
		//cout << spells[tempid].pushback << '^';
		a=0;

		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].pushup=atof(token);
		//cout << spells[tempid].pushup << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].cast_time=atoi(token);

		//cout << spells[tempid].cast_time << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].recovery_time=atoi(token);
		//cout << spells[tempid].recovery_time << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].recast_time=atoi(token);
		//cout << spells[tempid].recast_time << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].buffdurationformula=atoi(token);
		//cout << spells[tempid].buffdurationformula << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].buffduration=atoi(token);
		//cout << spells[tempid].buffduration << '^';

		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].ImpactDuration=atoi(token);
		//cout << spells[tempid].ImpactDuration<< '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].mana=atoi(token);
		//cout << spells[tempid].mana << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		int y;
		for(y=0; y< 12;y++)
		{
			in.get(temp);
			while(chrcmpI(&temp,&sep))
			{
				strncat(token,&temp,1);
				a++;
				in.get(temp);
			}
			spells[tempid].base[y]=atoi(token);
			//cout << spells[tempid].base[y] << '^';
			a=0;
			for(u=0;u<64;u++)
				token[u]=(char)0;


		}
		for(y=0; y< 12;y++)
		{
			in.get(temp);
			while(chrcmpI(&temp,&sep))
			{
				strncat(token,&temp,1);
				a++;
				in.get(temp);
			}
			spells[tempid].max[y]=atoi(token);
			//cout << spells[tempid].max[y] << '^';
			a=0;
			for(u=0;u<64;u++)
				token[u]=(char)0;

		}
		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].icon=atoi(token);
		//cout << spells[tempid].icon << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].memicon=atoi(token);
		//cout << spells[tempid].memicon << '^';

		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		for(y=0; y< 4;y++)
		{
			in.get(temp);
			while(chrcmpI(&temp,&sep))
			{
				strncat(token,&temp,1);
				a++;
				in.get(temp);
			}
			spells[tempid].components[y]=atoi(token);
			//cout << spells[tempid].components[y] << '^';
			a=0;
			for(u=0;u<64;u++)
				token[u]=(char)0;

		}
		for(y=0; y< 4;y++)
		{
			in.get(temp);
			while(chrcmpI(&temp,&sep))
			{
				strncat(token,&temp,1);
				a++;
				in.get(temp);
			}
			spells[tempid].component_counts[y]=atoi(token);//atoi(token);
			//cout << spells[tempid].component_counts[y] << '^';
			a=0;
			for(u=0;u<64;u++)
				token[u]=(char)0;
		}
		for(y=0; y< 4;y++)
		{
			in.get(temp);
			while(chrcmpI(&temp,&sep))
			{
				strncat(token,&temp,1);
				a++;
				in.get(temp);
			}
			spells[tempid].NoexpendReagent[y]=atoi(token); //NoExpend Reagent
			//cout << spells[tempid].NoexpendReagent[y] << '^';
			a=0;
			for(u=0;u<64;u++)
				token[u]=(char)0;

		}
		for(y=0; y< 12;y++)
		{
			in.get(temp);
			while(chrcmpI(&temp,&sep))
			{
				strncat(token,&temp,1);
				a++;
				in.get(temp);
			}
			spells[tempid].formula[y]=atoi(token);
			//cout << spells[tempid].formula[y] << '^';
			a=0;
			for(u=0;u<64;u++)
				token[u]=(char)0;

		}
		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}

		spells[tempid].LightType=atoi(token);
		//cout << spells[tempid].LightType << '^';
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].goodEffect=atoi(token);
		//cout << spells[tempid].goodEffect << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].Activated=atoi(token);
		//cout << spells[tempid].Activated << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);

		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].resisttype=atoi(token);
		//cout << spells[tempid].resisttype << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		for(y=0; y< 12;y++)
		{
			in.get(temp);
			while(chrcmpI(&temp,&sep))
			{
				strncat(token,&temp,1);
				a++;
				in.get(temp);
			}
			spells[tempid].effectid[y]=atoi(token);
			//cout << spells[tempid].effectid[y] << '^';
			a=0;
			for(u=0;u<64;u++)
				token[u]=(char)0;

		}
		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].targettype=atoi(token);
		//cout << spells[tempid].targettype << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].basediff=atoi(token);
		//cout << spells[tempid].basediff<< '^';
		a=0;
		for(u=0;u<64;u++)

			token[u]=(char)0;
		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].skill=atoi(token);
		//cout << spells[tempid].skill << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].zonetype=atoi(token);
		//cout << spells[tempid].zonetype << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].EnvironmentType=atoi(token);
		//cout << spells[tempid].EnvironmentType << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;

			in.get(temp);
		}
		spells[tempid].TimeOfDay=atoi(token);
		//cout << spells[tempid].TimeOfDay << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		for(y=0; y< 15;y++)
		{
			in.get(temp);
			while(chrcmpI(&temp,&sep))
			{
				strncat(token,&temp,1);
				a++;
				in.get(temp);
			}
			spells[tempid].classes[y]= atoi(token);
			//cout << spells[tempid].classes[y] << '^';
			a=0;
			for(u=0;u<64;u++)
				token[u]=(char)0;

		} //cout << "end class";
			/*for(y=0; y< 3;y++)
			{
			in.get(temp);
			while(chrcmpI(&temp,&sep))
			{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
			}
			spells[tempid].unknown1[y]=atoi(token);
			cout << spells[tempid].unknown1[y] << '^';
			a=0;
			for(u=0;u<64;u++)
			token[u]=(char)0;

			}
			in.get(temp);
			while(chrcmpI(&temp,&sep))
			{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
			}
			spells[tempid].unknown2=atoi(token);
			cout << spells[tempid].unknown2 << '^';
			a=0;
			for(u=0;u<64;u++)
			token[u]=(char)0;
		*/
		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].CastingAnim=atoi(token);
		//cout << spells[tempid].CastingAnim << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;
		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].TargetAnim=atoi(token);
		//cout << spells[tempid].TargetAnim << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;
		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].TravelType=atoi(token);
		//cout << spells[tempid].TravelType << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;
		in.get(temp);
		while(chrcmpI(&temp,&sep))
		{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
		}
		spells[tempid].SpellAffectIndex=atoi(token);
		//cout << spells[tempid].SpellAffectIndex << '^';

		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		for(y=0; y< 23;y++)
		{
			in.get(temp);
			while(chrcmpI(&temp,&sep))
			{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
			}
			spells[tempid].Spacing2[y]=atoi(token);
			//cout << spells[tempid].base[y] << '^';
			a=0;
			for(u=0;u<64;u++)
				token[u]=(char)0;
		}

		in.get(temp);
		while(chrcmpI(&temp,&sep))
			{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
			}
		spells[tempid].ResistDiff=atoi(token);
			//cout << spells[tempid].ResistDiff << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		in.get(temp);
		for(y=0; y< 2;y++)
			{
			in.get(temp);
			while(chrcmpI(&temp,&sep))
			{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
			}
			spells[tempid].Spacing3[y]=atoi(token);
			//cout << spells[tempid].base[y] << '^';
			a=0;
			for(u=0;u<64;u++)
			token[u]=(char)0;
			}

		in.get(temp);
		while(chrcmpI(&temp,&sep))
			{
			strncat(token,&temp,1);
			a++;
			in.get(temp);
			}
		spells[tempid].RecourseLink = atoi(token);
			//cout << spells[tempid].RecourseLink << '^';
		a=0;
		for(u=0;u<64;u++)
			token[u]=(char)0;

		while(temp!='\n')
			in.get(temp);

		//cout << endl;
		if(tempid==SPDAT_RECORDS-1) break;
	}
	//for(u=0;u< SPDAT_RECORDS;u++)
	// cout << u << ' ' << spells[u].name << '^';

	spells_loaded = true;
	cout << "Spells loaded.\n";
	in.close();

}
#endif


/*void EntityList::SendAATimer(uint32 charid,UseAA_Struct* uaa){
	Client* client2=this->GetClientByCharID(charid);
	if(!client2){
		LogFile->write(EQEMuLog::Error, "Error in SendAATimer: Couldnt find character!");
		return;
	}
	client2->SendAATimer(uaa);
}

void ZoneDatabase::UpdateAndDeleteAATimers(uint32 charid){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	char *query2 = 0;

	if (!RunQuery(query, MakeAnyLenString(&query, "delete from aa_timers where charid=%i and UNIX_TIMESTAMP(now())>=end",charid), errbuf)) {
		LogFile->write(EQEMuLog::Error, "UpdateAATimers query '%s' %s", query, errbuf);
	}
	if (!RunQuery(query2, MakeAnyLenString(&query2, "update aa_timers set end=end-(UNIX_TIMESTAMP(now())-begin),begin=UNIX_TIMESTAMP(now()) where charid=%i",charid), errbuf)) {
		LogFile->write(EQEMuLog::Error, "UpdateAATimers query '%s' %s", query2, errbuf);
	}
	safe_delete_array(query);
	safe_delete_array(query2);
}

void ZoneDatabase::UpdateTimersClientConnected(uint32 charid){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	if (!RunQuery(query, MakeAnyLenString(&query, "update aa_timers set end=(UNIX_TIMESTAMP(now())+(end-begin)),begin=UNIX_TIMESTAMP(now()) where charid=%i",charid), errbuf)) {
		LogFile->write(EQEMuLog::Error, "UpdateAATimers query '%s' %s", query, errbuf);
	}
	safe_delete_array(query);
}

void ZoneDatabase::GetAATimers(uint32 charid){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT ability,begin,end from aa_timers WHERE charid=%i", charid), errbuf, &result)) {
		while( ( row = mysql_fetch_row(result) ) ){
			UseAA_Struct* uaa=new UseAA_Struct();
			uaa->ability=atoi(row[0]);
			uaa->begin=atoi(row[1]);
			uaa->end=atoi(row[2]);
			entity_list.SendAATimer(charid,uaa);
			safe_delete(uaa);
		}
		mysql_free_result(result);
	}
	else {
		LogFile->write(EQEMuLog::Error, "Database::GetAATimers query '%s' %s", query, errbuf);
	}
	safe_delete_array(query);
}

uint32 ZoneDatabase::GetTimerRemaining(uint32 charid,uint32 ability){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint32 remain=0;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT end-begin from aa_timers WHERE charid=%i and ability=%i", charid,ability), errbuf, &result)) {
		if((row=mysql_fetch_row(result))){
			remain=atoi(row[0]);
		}
		mysql_free_result(result);
	}
	else {
		LogFile->write(EQEMuLog::Error, "Database::GetTimerRemaining query '%s' %s", query, errbuf);
	}
	safe_delete_array(query);
	return remain;
}

void ZoneDatabase::UpdateAATimers(uint32 charid,uint32 endtime,uint32 begintime,uint32 ability){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	if(begintime==0){
		if (!RunQuery(query, MakeAnyLenString(&query, "replace into aa_timers (charid,end,begin,ability) values(%i,UNIX_TIMESTAMP(now())+%i,UNIX_TIMESTAMP(now()),%i)",charid,endtime,ability), errbuf)) {
			LogFile->write(EQEMuLog::Error, "UpdateAATimers query '%s' %s", query, errbuf);
		}
	}
	else{
		if (!RunQuery(query, MakeAnyLenString(&query, "replace into aa_timers (charid,end,begin,ability) values(%i,%i,%i,%i)",charid,endtime,begintime,ability), errbuf)) {
			LogFile->write(EQEMuLog::Error, "UpdateAATimers query '%s' %s", query, errbuf);
		}
	}
	safe_delete_array(query);
}*/

/*
uint16 Client::GetCombinedAC_TEST() {
	int ac1;

	ac1 = GetRawItemAC();
	if (m_pp.class_ != WIZARD && m_pp.class_ != MAGICIAN && m_pp.class_ != NECROMANCER && m_pp.class_ != ENCHANTER) {
		ac1 = ac1*4/3;
	}
	ac1 += GetSkill(DEFENSE)/3;
	if (GetAGI() > 70) {
		ac1 += GetAGI()/20;
	}

	int ac2;

	ac2 = GetRawItemAC();
	if (m_pp.class_ != WIZARD && m_pp.class_ != MAGICIAN && m_pp.class_ != NECROMANCER && m_pp.class_ != ENCHANTER) {
		ac2 = ac2*4/3;
	}
	ac2 += GetSkill(DEFENSE)*400/255;

	int combined_ac = (ac1+ac2)*1000/847;
	return combined_ac;
	float combined_ac = ((float)ac1+(float)ac2)*1000.0f/847.0f;
	return (uint16) combined_ac;//*10.0f)-10;
}
*/


/*bool Client::GetIncreaseSpellDurationItem(uint16 &spell_id, char *itemname)
{
	for (int i=0; i<22; i++) {
		const ItemInst* inst = m_inv[i];
		if (!inst || !inst->IsType(ItemTypeCommon))
			continue;

		const Item_Struct* item = inst->GetItem();
		if (item->FocusId && (item->FocusId != 0xFFFF)) {
			if (IsIncreaseDurationSpell(item->FocusId)) {
				spell_id = item->FocusId;
				if (itemname)
					strcpy(itemname, item->Name);
				return true;
			}
		}
	}
	return false;
}

bool Client::GetReduceManaCostItem(uint16 &spell_id, char *itemname)
{
	for (int i=0; i<22; i++) {
		const ItemInst* inst = m_inv[i];
		if (!inst || !inst->IsType(ItemTypeCommon))
			continue;

		const Item_Struct* item = inst->GetItem();
		if (item->FocusId && (item->FocusId != 0xFFFF)) {
			if (IsReduceManaSpell(item->FocusId)) {
				spell_id = item->FocusId;
				if (itemname)
					strcpy(itemname, item->Name);
				return true;
			}
		}
	}
	return false;
}

bool Client::GetReduceCastTimeItem(uint16 &spell_id, char *itemname)
{
	for (int i=0; i<22; i++) {
		const ItemInst* inst = m_inv[i];
		if (!inst || !inst->IsType(ItemTypeCommon))
			continue;

		const Item_Struct* item = inst->GetItem();
		if (item->FocusId && (item->FocusId != 0xFFFF)) {
			if (IsReduceCastTimeSpell(item->FocusId)) {
				spell_id = item->FocusId;
				if (itemname)
					strcpy(itemname, item->Name);
				return true;
			}
		}
	}
	return false;
}

bool Client::GetExtendedRangeItem(uint16 &spell_id, char *itemname)
{
	for (int i=0; i<22; i++) {
		const ItemInst* inst = m_inv[i];
		if (!inst || !inst->IsType(ItemTypeCommon))
			continue;

		const Item_Struct* item = inst->GetItem();
		if (item->FocusId && (item->FocusId != 0xFFFF)) {
			if (IsExtRangeSpell(item->FocusId)) {
				spell_id = item->FocusId;
				if (itemname)
					strcpy(itemname, item->Name);
				return true;
			}
		}
	}
	return false;
}

bool Client::GetImprovedHealingItem(uint16 &spell_id, char *itemname)
{
	for (int i=0; i<22; i++) {
		const ItemInst* inst = m_inv[i];
		if (!inst || !inst->IsType(ItemTypeCommon))
			continue;

		const Item_Struct* item = inst->GetItem();
		if (item->FocusId && (item->FocusId != 0xFFFF)) {
			if (IsImprovedHealingSpell(item->FocusId)) {
				spell_id = item->FocusId;
				if (itemname)
					strcpy(itemname, item->Name);
				return true;
			}
		}
	}
	return false;
}

bool Client::GetImprovedDamageItem(uint16 &spell_id, char *itemname)
{
	for (int i=0; i<22; i++) {
		const ItemInst* inst = m_inv[i];
		if (!inst || !inst->IsType(ItemTypeCommon))
			continue;

		const Item_Struct* item = inst->GetItem();
		if (item->FocusId && (item->FocusId != 0xFFFF)) {
			if (IsImprovedDamageSpell(item->FocusId)) {
				spell_id = item->FocusId;
				if (itemname)
					strcpy(itemname, item->Name);
				return true;
			}
		}
	}
	return false;
}

int32 Client::GenericFocus(uint16 spell_id, uint16 modspellid)
{
	int modifier = 100, i;
	const SPDat_Spell_Struct &spell = spells[spell_id];
	const SPDat_Spell_Struct &modspell = spells[modspellid];

	for (i = 0; i < EFFECT_COUNT; i++)
	{
		if(IsBlankSpellEffect(modspellid, i))
			continue;
		switch( spells[modspellid].effectid[i] )
		{
			case SE_LimitMaxLevel:
				if (spell.classes[(GetClass()%17) - 1] > modspell.base[i])
					return 100;
				break;
			case SE_LimitMinLevel:
				if (spell.classes[(GetClass()%17) - 1] < modspell.base[i])
					return 100;
				break;
			case SE_IncreaseRange:
				modifier += modspell.base[i];
				break;
			case SE_IncreaseSpellHaste:
				modifier -= modspell.base[i];
				break;
			case SE_IncreaseSpellDuration:
				modifier += modspell.base[i];
				break;
			case SE_LimitSpell:
				// negative sign means exclude
				// positive sign means include
				if (modspell.base[i] < 0)
				{
					if (modspell.base[i] * (-1) == spell_id)
						return 100;
				}
				else
				{
					if (spells[modspellid].base[i] != spell_id)
						return 100;
				}
				break;
			case SE_LimitEffect:
				switch( spells[modspellid].base[i] )
				{
					case -147:
						if (IsPercentalHealSpell(spell_id))
							return 100;
						break;
					case -101:
						if (IsCHDurationSpell(spell_id))
							return 100;
						break;
					case -40:
						if (IsInvulnerabilitySpell(spell_id))
							return 100;
						break;
					case -32:
						if (IsSummonItemSpell(spell_id))
							return 100;
						break;
					case 0:
						if (!IsEffectHitpointsSpell(spell_id))
							return 100;
						break;
					case 33:
						if (!IsSummonPetSpell(spell_id))
							return 100;
						break;
					case 36:
						if (!IsPoisonCounterSpell(spell_id))
							return 100;
						break;
					case 71:
						if (!IsSummonSkeletonSpell(spell_id))
							return 100;
						break;
					default:
						LogFile->write(EQEMuLog::Normal, "GenericFocus: unknown limit effect %d", spells[modspellid].base[i]);
				}
				break;
			case SE_LimitCastTime:
				if (modspell.base[i] > (int16)spell.cast_time)
					return 100;
				break;
			case SE_LimitSpellType:
				switch( spells[modspellid].base[i] )
				{
					case 0:
						if (!IsDetrimentalSpell(spell_id))
							return 100;
						break;
					case 1:
						if (!IsBeneficialSpell(spell_id))
							return 100;
						break;
					default:
						LogFile->write(EQEMuLog::Normal, "GenericFocus: unknown limit spelltype %d", spells[modspellid].base[i]);
				}
				break;
			case SE_LimitMinDur:
				if (modspell.base[i] > CalcBuffDuration_formula(GetLevel(), spell.buffdurationformula, spell.buffduration))
					return 100;
				break;
			case SE_ImprovedDamage:
			case SE_ImprovedHeal:
				modifier += modspell.base[i];
				break;
			case SE_ReduceManaCost:
				modifier -= modspell.base[i];
				break;
			default:
				LogFile->write(EQEMuLog::Normal, "GenericFocus: unknown effectid %d", modspell.effectid[i]);
		}
	}

	return modifier;
}
*/



/*void Client::Discipline(ClientDiscipline_Struct* disc_in, Mob* tar) {
Message(0, "Disc packet id=%d, %x,%x,%x", disc_in->disc_id, disc_in->unknown3[0], disc_in->unknown3[1], disc_in->unknown3[2]);
	if (!p_timers.Expired(&database, pTimerDisciplineReuse)) {
		char val1[20]={0};
		char val2[20]={0};
		uint32 remain = p_timers.GetRemainingTime(pTimerDisciplineReuse);
		MessageString(Chat::White,DISCIPLINE_CANUSEIN,ConvertArray((remain)/60,val1),ConvertArray(remain%60,val2));
		//Message(0,"You can use a new discipline in %i minutes %i seconds.", (disc_timer.GetRemainingTime()/1000)/60,	disc_timer.GetRemainingTime()/1000%60);
		return;
	}

	//reuse times are a little messes up, they should scale down somehow
	//as you gain in levels, but im not sure how, so its just a lvl 60 bonus right now

	//should change this to check classes better.

	//both in seconds, converted at the end.
	uint32 duration = 0;
	uint32 reuse = 0;

	switch(disc_in->disc_id){
	// Shared?
	case discResistant: { // Resistant
		// 1 minute duration
		// 1 hour reuse
		// +3 to +10 to resists
		if (GetLevel()<=29)
			return;
		duration = 60;
		reuse = 60*60;
		entity_list.MessageClose(this, false, 100, 0, "%s has become more resistant!", GetName());
		break;
	}
	case discFearless: { // Fearless
		// 11 second duration
		// 1 hour reuse
		// 100% fear immunity
		if (GetLevel()<=39)
			return;
		duration = 11;
		reuse = 60*60;
		entity_list.MessageCloseString(this, false, 100, 0, DISCIPLINE_FEARLESS, GetName());
		//entity_list.MessageClose(this, false, 100, 0, "%s becomes fearless!", GetName());
		break;
	}
	case discWhirlwind: { // Counterattack/Whirlwind/Furious
		// warrior level 56
		// rogue/monk level 53
		// 9 second duration
		// 1 hour reuse
		if ((GetClass() == WARRIOR && GetLevel() <= 56)
			||(GetLevel() <= 53)
			) return;
		duration = 9;
		reuse = 60*60;
		entity_list.MessageClose(this, false, 100, 0, "%s\'s face becomes twisted with fury!", GetName());
		break;
	}
	case discFellstrike: { // Duelist/Innerflame/Fellstrike
		// monk level 56
		// rogue level 59
		// warrior level 58
		// 12 second duration
		// 30 minute reuse
		// min 4*base hand/weapon damage
		if ((GetClass() == MONK && GetLevel() <= 55)
			||(GetClass() == WARRIOR && GetLevel() <= 58)
			||(GetClass() == ROGUE && GetLevel() <= 59)
			) return;
		duration = 12;
		reuse = 60*30;
		entity_list.MessageClose(this, false, 100, 0, "%s\'s muscles bulge with force of will!", GetName());
		break;
	}
	case discBlindingSpeed: { // Blindingspeed/Hundredfist
		// rogue level 58
		// monk level 57
		// 15 second duration
		// 30 minute reuse
		if ((GetClass() == MONK && GetLevel() <= 58)
			||(GetClass() == ROGUE && GetLevel() <= 57)
			) return;
		//disc_timer.Start(1000*60*30);
		//disc_elapse.Start(1000*15);
		duration = 15;
		reuse = 60*30;
		Message(0, "This discipline not implemented..");
		break;
	}
	case discDeadeye: { // Deadeye/Charge
		// warrior level 53
		// rogue level 54
		// 14 second duration
		// 30 minute reuse
		if ((GetClass() == WARRIOR && GetLevel() <= 53)
			||(GetClass() == ROGUE && GetLevel() <= 54)
			) return;
		duration = 14;
		reuse = 60*30;
		entity_list.MessageClose(this, false, 100, 0, "%s feels unstopable!", GetName());
		break;
	}
	// Warrior
	case discEvasive: { // Evasive
		// level 52
		// 3 minute duration
		// 15 minute reuse
		// +35% avoidance
		// -15% out
		duration = 3*60;
		reuse = 15*60;
		break;
	}
	case discMightystrike: { // Mightystrike
		// level 54
		// 10 second duration
		// 1 hour reuse
		// Auto crit
		duration = 10;
		reuse = 60*60;
		break;
	}
	case discDefensive: { // Defensive
		// level 55
		// 3 minute duration
		// 15 minute reuse, 10 after 60
		// +35% mitigation
		// -15% out
		duration = 3*60;
		if(level > 59)
			reuse = 10*60;
		else
			reuse = 15*60;
		break;
	}
	case discPrecise: { // Precise
		// level 57
		// 3 minute duration
		// 30 minute reuse
		// -15% avoidance
		// +35% out
		duration = 3*60;
		reuse = 30*60;
		break;
	}
	case discAggressive: { // Aggressive
		// level 60
		// 3 minute duration
		// 27 minute reuse
		// -15% mitigation
		// +35% out
		duration = 3*60;
		reuse = 27*60;
		break;
	}
	// Monk
	case discStonestance: { // Stonestance
		// level 51
		duration = 12;
		if(level > 59)
			reuse = 3*60;
		else
			reuse = 12*60;
		break;
	}
	case discThunderkick: { // Thunderkick
		// level 52
		if(level > 59)
			reuse = 1*60;
		else
			reuse = 7*60;
		duration = 5*60;		//hack for now, checked in combat and expired once used.
		break;
	}
	case discVoidance: { // Voidance
		// level 54
		if(level > 59)
			reuse = 54*60;
		else
			reuse = 60*60;
		duration = 8;
		break;
	}
	case discSilentfist: { // Silentfist
		// level 59
		// 6-7 minute reuse
		// Dragon punch damage bonus
		// Chance to stun
		if(level > 59)
			reuse = 6*60;
		else
			reuse = 7*60;
		duration = 5*60;		//hack for now, checked in combat and expired once used.
		break;
	}
	case discAshenhand: { // Ashenhand
		// level 60
		// 72 minute reuse
		// Eagle Strike damage bonus
		// Chance to slay
		reuse = 72*60;
		duration = 5*60;		//hack for now, checked in combat and expired once used.
		break;
	}
	// Rogue
	case discNimble: { // Nimble
		// level 55
		// 12 second duration
		// 30 minute reuse
		// Auto dodge
		if(level > 59)
			reuse = 25*60;
		else
			reuse = 30*60;
		duration = 12;
		break;
	}
	case discKinesthetics: { // Kinesthetics
		// level 57
		// 18 second duration
		// 30,27 minute reuse
		// Auto dualwield
		// Auto double attack
		if(level > 59)
			reuse = 27*60;
		else
			reuse = 30*60;
		duration = 18;
		break;
	}
	// Paladin
	case discHolyforge: { // Holyforge
		// level 55
		// 2 minute duration
		// 72 minute reuse
		// Crit/Crip undead
		// +15% to crit chance
		if(level > 59)
			reuse = 65*60;
		else
			reuse = 72*60;
		duration = 5*60;
		break;
	}
	case discSanctification: { // Sanctification
		// level 60
		// 10 second duration
		// 72 minute reuse
		// Spell immunity
		reuse = 72*60;
		duration = 15;
		break;
	}
	// Ranger
	case discTrueshot: { // Trueshot
		// level 55
		// 2 minute duration
		// 72 minute reuse
		// Max to two times max bow damage
		// +15% to hit
		if(level > 59)
			reuse = 67*60;
		else
			reuse = 72*60;
		duration = 2*60;
		break;
	}
	case discWeaponshield: { // Weaponshield
		// level 60
		// 15 second duration
		// 72 minute reuse
		// auto parry
		reuse = 72*60;
		duration = 20;
		break;
	}
	// Bard
	case discDeftdance: { // Deftdance
		// level 55
		// 10 second duration
		// 72 minute reuse
		// auto dodge
		// auto dualwield
		if(level > 59)
			reuse = 67*60;
		else
			reuse = 72*60;
		duration = 10;
		break;
	}
	case discPuretone: { // Puretone
		// level 60
		// 2 minute duration
		// 72 minute reuse
		// Auto instrument
		reuse = 72*60;
		duration = 4*60;
		break;
	}
	// Shadow knight
	case discUnholyAura: { // Unholy
		// level 55
		// 72 minute reuse
		// +25% to harmtouch
		// -300 to resist
		if(level > 59)
			reuse = 67*60;
		else
			reuse = 72*60;
		duration = 5*60;		//hack for now, checked in combat and expired once used.
		break;
	}
	case discLeechCurse: { // Leech curse
		// level 60
		// 15 second duration
		// 72 minute reuse
		// Heal self for each point of melee damage done
		reuse = 72*60;
		duration = 15;
		break;
	}
	// Default
	case 0:{ // Timer request
		break;
	}
	default:
		LogFile->write(EQEMuLog::Error, "Unknown Discipline requested by client: %s class: %i Disciline:%i", GetName(), class_,disc_in->disc_id);
		return;
	}

	if(reuse != 0) {
		p_timers.Start(pTimerDisciplineReuse, reuse);
		//nonpersistent timer for the 'discipline ready' message
		disc_timer.Start(1000*reuse);
	}
	if(duration != 0)
		disc_elapse.Start(1000*duration);

	disc_inuse = disc_in->disc_id;
}*/

#if 0	// solar: this is old code
/*void EntityList::AESpell(Mob* caster, Mob* center, float dist, uint16 spell_id, bool group)
{
	LinkedListIterator<Mob*> iterator(mob_list);
	iterator.Reset();
	while(iterator.MoreElements()) {
		Mob* mob = iterator.GetData();
		if (group){
				// Client casting group spell with out target group buffs enabled
				// Skip non group members
				if (caster->IsClient()
					&& !caster->CastToClient()->TGB()
					&& GetGroupByMob(mob) != 0
					&& !GetGroupByMob(mob)->IsGroupMember(caster)
					) {
					LogFile->write(EQEMuLog::Debug, "Group spell skipping %s", mob->GetName());
						iterator.Advance();
						continue;
				}
				// Client casting group spell with target group buffs enabled
				else if (caster->IsClient()
						&& caster->CastToClient()->TGB()
						&& GetGroupByMob(mob) != 0
						&& GetGroupByMob(mob)->IsGroupMember(caster)
						){
						LogFile->write(EQEMuLog::Debug, "Group spell TGB on %s's Group", mob->GetName());
						GetGroupByMob(mob)->CastGroupSpell(caster, spell_id);
						iterator.Advance();
						continue;
				}
				else if (caster->IsClient()
						&& caster->CastToClient()->TGB()
						&& GetGroupByMob(mob) == 0
						&& mob == center
						){
						LogFile->write(EQEMuLog::Debug, "Group spell TGB on %s", mob->GetName());
						caster->SpellOnTarget(spell_id, mob);
						return;
				}
		}
		if (
			mob->DistNoZ(*center) <= dist
			&& !(mob->IsClient() && mob->CastToClient()->GMHideMe())
			&& !mob->IsCorpse()
			) {
			//cout << "AE Spell Hit: t=" << iterator.GetData()->GetName() << ", d=" << iterator.GetData()->CastToMob()->DistNoRoot(center) << ", x=" << iterator.GetData()->CastToMob()->GetX() << ", y=" << iterator.GetData()->CastToMob()->GetY() << endl;
			if (caster == mob) {
				// Caster gets the first hit, already handled in spells.cpp
			}
		#ifdef IPC
			else if(caster->IsNPC() && !caster->CastToNPC()->IsInteractive()) {
		#else
			else if(caster->IsNPC()) {
		#endif
			// Npc
				if (caster->IsAttackAllowed(mob) && spells[spell_id].targettype != ST_AEBard) {
					caster->SpellOnTarget(spell_id, mob);
				}
				else if (mob->IsAIControlled() && spells[spell_id].targettype == ST_AEBard) {
					caster->SpellOnTarget(spell_id, mob);
				}
				else {
				}
			}
		#ifdef IPC
			else if(caster->IsNPC() && caster->CastToNPC()->IsInteractive()) {
				// Interactive npc
				if (caster->IsAttackAllowed(mob) && spells[spell_id].targettype != ST_AEBard && spells[spell_id].targettype != ST_GroupTeleport) {
					caster->SpellOnTarget(spell_id, mob);
				}
				else if (!mob->IsAIControlled() && (spells[spell_id].targettype == ST_AEBard||group) && mob->CastToClient()->GetPVP() == caster->CastToClient()->GetPVP()) {
						if (group && GetGroupByMob(mob) != GetGroupByMob(caster)) {
								iterator.Advance();
								continue;
					}
				caster->SpellOnTarget(spell_id, mob);
				}
				else {
				}
			}
		#endif
			else if (caster->IsClient() && !(caster->CastToClient()->IsBecomeNPC())) {
				// Client
				if (caster->IsAttackAllowed(mob) && spells[spell_id].targettype != ST_AEBard){
					caster->SpellOnTarget(spell_id, mob);
				}
				else if(spells[spell_id].targettype == ST_GroupTeleport && mob->IsClient() && mob->isgrouped && caster->isgrouped && entity_list.GetGroupByMob(caster))
				{
					Group* caster_group = entity_list.GetGroupByMob(caster);
					if(caster_group != 0 && caster_group->IsGroupMember(mob))
						caster->SpellOnTarget(spell_id,mob);
				}
				else if (mob->IsClient() && (spells[spell_id].targettype == ST_AEBard||group) && mob->CastToClient()->GetPVP() == caster->CastToClient()->GetPVP()) {
					if (group && GetGroupByMob(mob) != GetGroupByMob(caster)) {
						iterator.Advance();
						continue;
					}
					else if (mob->IsClient() && spells[spell_id].targettype == ST_AEBard && mob->CastToClient()->GetPVP() == caster->CastToClient()->GetPVP())
						caster->SpellOnTarget(spell_id, mob);
				#ifdef IPC
					else if (mob->IsNPC() && mob->CastToNPC()->IsInteractive()) {
						if (group && GetGroupByMob(mob) != GetGroupByMob(caster))
							continue;
						caster->SpellOnTarget(spell_id, mob);
					}
				#endif
				}
			}
			else if (caster->IsClient()) {
				// Client BecomeNPC
				caster->SpellOnTarget(spell_id, mob);
			}
		}
		iterator.Advance();
	}
}*/
#endif	// solar: old code

/*#if 0
// Queries the loottable: adds item & coin to the npc
void ZoneDatabase::AddLootTableToNPC(uint32 loottable_id, ItemList* itemlist, uint32* copper, uint32* silver, uint32* gold, uint32* plat) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	*copper = 0;
	*silver = 0;
	*gold = 0;
	*plat = 0;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT id, mincash, maxcash, avgcoin FROM loottable WHERE id=%i", loottable_id), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			uint32 mincash = atoi(row[1]);
			uint32 maxcash = atoi(row[2]);
			if (mincash > maxcash) {
				cerr << "Error in loottable #" << row[0] << ": mincash > maxcash" << endl;
			}
			else if (maxcash != 0) {
				uint32 cash = 0;
				if (mincash == maxcash)
					cash = mincash;
				else
					cash = (rand() % (maxcash - mincash)) + mincash;
				if (cash != 0) {
					uint32 coinavg = atoi(row[3]);
					if (coinavg != 0) {
						uint32 mincoin = (uint32) (coinavg * 0.75 + 1);
						uint32 maxcoin = (uint32) (coinavg * 1.25 + 1);
						*copper = (rand() % (maxcoin - mincoin)) + mincoin - 1;
						*silver = (rand() % (maxcoin - mincoin)) + mincoin - 1;
						*gold = (rand() % (maxcoin - mincoin)) + mincoin - 1;
						cash -= *copper;
						cash -= *silver * 10;
						cash -= *gold * 10;
					}
					*plat = cash / 1000;
					cash -= *plat * 1000;
					uint32 gold2 = cash / 100;
					cash -= gold2 * 100;
					uint32 silver2 = cash / 10;
					cash -= silver2 * 10;
					*gold += gold2;
					*silver += silver2;
					*copper += cash;
				}
			}
		}
		else {
			mysql_free_result(result);
			return;
		}
		mysql_free_result(result);
	}
	else
	{
		cerr << "Error in AddLootTableToNPC get coin query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return;
	}

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT loottable_id, lootdrop_id, multiplier, probability FROM loottable_entries WHERE loottable_id=%i", loottable_id), errbuf, &result)) {
		safe_delete_array(query);
		while ((row = mysql_fetch_row(result))) {
			int multiplier = atoi(row[2]);
			for (int i = 1; i <= multiplier; i++) {
				if ( ((rand()%1)*100) < atoi(row[3])) {
					AddLootDropToNPC(atoi(row[1]), itemlist);
				}
			}
		}
		mysql_free_result(result);
	}
	else {
		cerr << "Error in AddLootTableToNPC get items query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return;
	}

	return;
}

// Called by AddLootTableToNPC
// maxdrops = size of the array npcd
void ZoneDatabase::AddLootDropToNPC(uint32 lootdrop_id, ItemList* itemlist) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

// This is Wiz's updated Pool Looting functionality. Eventually, the database format should be moved over to use this
// or implemented to support both methods. (A unique identifier in lootable_entries indicates to roll for a pool item
// in another table.
#ifdef POOLLOOTING
	uint32 chancepool = 0;
	uint32 items[50];
	uint32 itemchance[50];
	uint16 itemcharges[50];
	uint8 i = 0;

	for (int m=0;m < 50;m++)
	{
		items[m]=0;
		itemchance[m]=0;
		itemcharges[m]=0;
	}

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT lootdrop_id, item_id, item_charges, equip_item, chance FROM lootdrop_entries WHERE lootdrop_id=%i order by chance desc", lootdrop_id), errbuf, &result))
	{
		safe_delete_array(query);
		while (row = mysql_fetch_row(result))
		{
			items[i] = atoi(row[1]);
			itemchance[i] = atoi(row[4]) + chancepool;
			itemcharges[i] = atoi(row[2]);
			chancepool += atoi(row[4]);
			i++;
		}
		uint32 res;
		i = 0;

		if (chancepool!=0) //avoid divide by zero if some mobs have 0 for chancepool
		{
			res = rand()%chancepool;
		}
		else
		{
			res = 0;
		}

		while (items[i] != 0)
		{
			if (res <= itemchance[i])
				break;
			else
				i++;
		}
		const Item_Struct* dbitem = database.GetItem(items[i]);
		if (dbitem == 0)
		{
			LogFile->write(EQEMuLog::Error, "AddLootDropToNPC: dbitem=0, item#=%i, lootdrop_id=%i", items[i], lootdrop_id);
		}
		else
		{
			//cout << "Adding item to Mob" << endl;
			ServerLootItem_Struct* item = new ServerLootItem_Struct;
			item->item_id = dbitem->ItemNumber;
			item->charges = itemcharges[i];
			item->equipSlot = 0;
			(*itemlist).Append(item);
		}
		mysql_free_result(result);
	}
#else
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT lootdrop_id, item_id, item_charges, equip_item, chance FROM lootdrop_entries WHERE lootdrop_id=%i order by chance desc", lootdrop_id), errbuf, &result))
	{
		safe_delete_array(query);
		while ((row = mysql_fetch_row(result)))
		{
			uint8 LootDropMod=1; // place holder till I put it in a database variable to make it configurable.
			if( (rand()%100) < ((atoi(row[4]) * LootDropMod)) )
			{
				uint32 itemid = atoi(row[1]);
				const Item_Struct* dbitem = database.GetItem(itemid);
				if (dbitem == 0)
				{
					LogFile->write(EQEMuLog::Error, "AddLootDropToNPC: dbitem=0, item#=%i, lootdrop_id=%i", itemid, lootdrop_id);
				}
				else
				{
					printf("Adding item: %i",item->ItemNumber);
					ServerLootItem_Struct* item = new ServerLootItem_Struct;
					item->item_id = dbitem->item_id;
					item->charges = atoi(row[2]);
					item->equipSlot = 0;
					(*itemlist).Append(item);
				}

				//mysql_free_result(result);
				//return;
			}
		}
		mysql_free_result(result);
	}
#endif
	else
	{
		LogFile->write(EQEMuLog::Error, "Error in AddLootDropToNPC query '%s' %s", query, errbuf);
		safe_delete_array(query);
		return;
	}

	return;
}
#endif*/
