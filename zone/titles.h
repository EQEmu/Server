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
#ifndef TITLES_H
#define TITLES_H

#include <vector>

class Client;
class EQApplicationPacket;

struct TitleEntry
{
	int title_id;
	int skill_id;
	int min_skill_value;
	int max_skill_value;
	int min_aa_points;
	int max_aa_points;
	int class_id;
	int gender_id;
	int character_id;
	int status;
	int item_id;
	std::string prefix;
	std::string suffix;
	int titleset;
};

class TitleManager
{
public:
	TitleManager();

	bool LoadTitles();

	EQApplicationPacket *MakeTitlesPacket(Client *client);
	std::string GetPrefix(int title_id);
	std::string GetSuffix(int title_id);
	int NumberOfAvailableTitles(Client *client);
	bool IsClientEligibleForTitle(Client *client, TitleEntry title);
	bool IsNewAATitleAvailable(int aa_points, int class_id);
	bool IsNewTradeSkillTitleAvailable(int skill_id, int skill_value);
	void CreateNewPlayerTitle(Client *client, std::string title);
	void CreateNewPlayerSuffix(Client *client, std::string suffix);
	bool HasTitle(Client* client, uint32 title_id);

protected:
	std::vector<TitleEntry> titles;
};

extern TitleManager title_manager;

#endif

