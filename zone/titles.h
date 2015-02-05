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
	int TitleID;
	int SkillID;
	int MinSkillValue;
	int MaxSkillValue;
	int MinAAPoints;
	int MaxAAPoints;
	int Class;
	int Gender;
	int CharID;
	int Status;
	int ItemID;
	std::string Prefix;
	std::string Suffix;
	int TitleSet;
};

class TitleManager
{
public:
	TitleManager();

	bool LoadTitles();

	EQApplicationPacket *MakeTitlesPacket(Client *c);
	std::string GetPrefix(int TitleID);
	std::string GetSuffix(int TitleID);
	int NumberOfAvailableTitles(Client *c);
	bool IsClientEligibleForTitle(Client *c, std::vector<TitleEntry>::iterator Title);
	bool IsNewAATitleAvailable(int AAPoints, int Class);
	bool IsNewTradeSkillTitleAvailable(int SkillID, int SkillValue);
	void CreateNewPlayerTitle(Client *c, const char *Title);
	void CreateNewPlayerSuffix(Client *c, const char *Suffix);

protected:
	std::vector<TitleEntry> Titles;
};

extern TitleManager title_manager;

#endif

