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
#ifndef __QUEST_H__
#define __QUEST_H__

#include "zonedb.h"
#include "zone_store.h"

typedef struct _tag_quest_entry{
	char	*m_pQuestName;
	char	*m_pQuestText;
	char	*m_pQuestEnd;
	int		m_iNpcId;
	int		m_iQuestObject;
	int		m_iQuestPrice;
	int		m_iQuestCash;
	int		m_iQuestExp;
}quest_entry,*pquest_entry;


class Quest{
public:
	Quest();
	~Quest();
	static pquest_entry	Test(int NpcId, int QuestObject);

	static pquest_entry	m_pQuests;
	static int			m_nQuests;
};

#endif

