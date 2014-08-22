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
#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Disgrace: for windows compile
#ifdef WIN32
	#include <windows.h>
	#define snprintf	_snprintf
#else
	#include "../common/unix.h"
#endif

#include "quest.h"

pquest_entry	Quest::m_pQuests;
int				Quest::m_nQuests;

Quest::Quest()
{
	m_pQuests = NULL;
	m_nQuests = 0;
}

Quest::~Quest()
{
	for( int i=0;i<m_nQuests;i++ )
	{
		delete m_pQuests[ i ].m_pQuestName;
		delete m_pQuests[ i ].m_pQuestText;
		delete m_pQuests[ i ].m_pQuestEnd;
	}
	delete[] m_pQuests;
}

bool ZoneDatabase::OpenQuery(char* zonename) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES* result;
	MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT name, text, end, npcID, questobject, priceobject, cash, exp FROM quest WHERE zone='%s'", zonename), errbuf, &result)) {
		delete[] query;
		Quest::m_nQuests = mysql_num_rows( result );
		if( Quest::m_nQuests )
		{
		int l_cnt = 0;
			Quest::m_pQuests = new quest_entry[ Quest::m_nQuests ];
			while(row = mysql_fetch_row(result)) {
				Quest::m_pQuests[ l_cnt ].m_pQuestName = new char[ strlen( row[0] )+1 ];
				strcpy( Quest::m_pQuests[ l_cnt ].m_pQuestName, row[0] );
				Quest::m_pQuests[ l_cnt ].m_pQuestText = new char[ strlen( row[1] )+1 ];
				strcpy( Quest::m_pQuests[ l_cnt ].m_pQuestText, row[1] );
				Quest::m_pQuests[ l_cnt ].m_pQuestEnd = new char[ strlen( row[2] )+1 ];
				strcpy( Quest::m_pQuests[ l_cnt ].m_pQuestEnd, row[2] );

				Quest::m_pQuests[ l_cnt ].m_iNpcId		= atoi( row[3] );
				Quest::m_pQuests[ l_cnt ].m_iQuestObject= atoi( row[4] );
				Quest::m_pQuests[ l_cnt ].m_iQuestPrice	= atoi( row[5] );
				Quest::m_pQuests[ l_cnt ].m_iQuestCash	= atoi( row[6] );
				Quest::m_pQuests[ l_cnt ].m_iQuestExp	= atoi( row[7] );
				cerr << "Quests '" << Quest::m_pQuests[ l_cnt ].m_pQuestName << "' , NPCID " << Quest::m_pQuests[ l_cnt ].m_iNpcId << endl;
				l_cnt++;
			}
			mysql_free_result(result);
			return true;
		}
		mysql_free_result(result);
	}
	cerr << "Error in ZoneDatabase::OpenQuest query '" << query << "' " << errbuf << endl;
	delete[] query;

return false;
}

pquest_entry	Quest::Test( int NpcId, int QuestObject )
{
	for( int i=0;i<m_nQuests;i++ )
		if( m_pQuests[ i ].m_iNpcId == NpcId && m_pQuests[ i ].m_iQuestObject	== QuestObject )
			return &m_pQuests[ i ];
return NULL;
}

