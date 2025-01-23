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

	EQApplicationPacket* MakeTitlesPacket(Client* c);
	std::string GetPrefix(int title_id);
	std::string GetSuffix(int title_id);
	const std::vector<TitleEntry>& GetEligibleTitles(Client* c);
	bool IsNewAATitleAvailable(int aa_points, int class_id);
	bool IsNewTradeSkillTitleAvailable(int skill_id, int skill_value);
	void CreateNewPlayerTitle(Client* c, std::string title);
	void CreateNewPlayerSuffix(Client* c, std::string suffix);
	bool HasTitle(Client* c, uint32 title_id);
	inline const std::vector<TitleEntry>& GetTitles() { return titles; }

protected:
	std::vector<TitleEntry> titles;
};

extern TitleManager title_manager;

#endif

