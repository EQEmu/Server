#ifndef ADVENTURE_MANAGER_H
#define ADVENTURE_MANAGER_H

#include "../common/debug.h"
#include "../common/types.h"
#include "../common/timer.h"
#include "Adventure.h"
#include "AdventureTemplate.h"
#include <map>
#include <list>

using namespace std;

class AdventureManager
{
public:
	AdventureManager();
	~AdventureManager();

	void Process();

	bool LoadAdventureTemplates();
	bool LoadAdventureEntries();
	void LoadLeaderboardInfo();

	void CalculateAdventureRequestReply(const char *data);
	void PlayerClickedDoor(const char *player, int zone_id, int door_id);
	void TryAdventureCreate(const char *data);
	void GetAdventureData(Adventure *adv);
	void GetAdventureData(const char *name);
	void LeaveAdventure(const char *name);
	void IncrementCount(uint16 instance_id);
	void IncrementAssassinationCount(uint16 instance_id);
	void DoLeaderboardRequest(const char* player, uint8 type);
	void SendAdventureFinish(AdventureFinishEvent fe);
	void AddFinishedEvent(AdventureFinishEvent fe) { finished_list.push_back(fe); Save(); }
	bool PopFinishedEvent(const char *name, AdventureFinishEvent &fe);
	void Save();
	void Load();

	Adventure **GetFinishedAdventures(const char *player, int &count);
	Adventure *GetActiveAdventure(const char *player);
	AdventureTemplate *GetAdventureTemplate(int theme, int id);
	AdventureTemplate *GetAdventureTemplate(int id);
	void GetZoneData(uint16 instance_id);
protected:
	bool IsInExcludedZoneList(list<AdventureZones> excluded_zones, string zone_name, int version);
	bool IsInExcludedZoneInList(list<AdventureZoneIn> excluded_zone_ins, int zone_id, int door_object);
	void DoLeaderboardRequestWins(const char* player);
	void DoLeaderboardRequestPercentage(const char* player);
	void DoLeaderboardRequestWinsGuk(const char* player);
	void DoLeaderboardRequestPercentageGuk(const char* player);
	void DoLeaderboardRequestWinsMir(const char* player);
	void DoLeaderboardRequestPercentageMir(const char* player);
	void DoLeaderboardRequestWinsMmc(const char* player);
	void DoLeaderboardRequestPercentageMmc(const char* player);
	void DoLeaderboardRequestWinsRuj(const char* player);
	void DoLeaderboardRequestPercentageRuj(const char* player);
	void DoLeaderboardRequestWinsTak(const char* player);
	void DoLeaderboardRequestPercentageTak(const char* player);

	map<uint32, AdventureTemplate*> adventure_templates;
	map<uint32, list<AdventureTemplate*> > adventure_entries;
	list<Adventure*> adventure_list;
	list<AdventureFinishEvent> finished_list;
	list<LeaderboardInfo> leaderboard_info_wins;
	list<LeaderboardInfo> leaderboard_info_percentage;
	list<LeaderboardInfo> leaderboard_info_wins_guk;
	list<LeaderboardInfo> leaderboard_info_percentage_guk;
	list<LeaderboardInfo> leaderboard_info_wins_mir;
	list<LeaderboardInfo> leaderboard_info_percentage_mir;
	list<LeaderboardInfo> leaderboard_info_wins_mmc;
	list<LeaderboardInfo> leaderboard_info_percentage_mmc;
	list<LeaderboardInfo> leaderboard_info_wins_ruj;
	list<LeaderboardInfo> leaderboard_info_percentage_ruj;
	list<LeaderboardInfo> leaderboard_info_wins_tak;
	list<LeaderboardInfo> leaderboard_info_percentage_tak;
	bool leaderboard_sorted_wins;
	bool leaderboard_sorted_percentage;
	bool leaderboard_sorted_wins_guk;
	bool leaderboard_sorted_percentage_guk;
	bool leaderboard_sorted_wins_mir;
	bool leaderboard_sorted_percentage_mir;
	bool leaderboard_sorted_wins_mmc;
	bool leaderboard_sorted_percentage_mmc;
	bool leaderboard_sorted_wins_ruj;
	bool leaderboard_sorted_percentage_ruj;
	bool leaderboard_sorted_wins_tak;
	bool leaderboard_sorted_percentage_tak;
	Timer *process_timer;
	Timer *save_timer;
	Timer *leaderboard_info_timer;
};

#endif