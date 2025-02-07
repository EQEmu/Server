#ifndef TITLES_H
#define TITLES_H

#include <vector>
#include "../common/repositories/titles_repository.h"

class Client;
class EQApplicationPacket;

class TitleManager
{
public:
	TitleManager();

	bool LoadTitles();

	EQApplicationPacket* MakeTitlesPacket(Client* c);
	std::string GetPrefix(int title_set);
	std::string GetSuffix(int title_set);
	std::vector<TitlesRepository::Titles> GetEligibleTitles(Client* c);
	bool IsNewAATitleAvailable(int aa_points, int class_id);
	bool IsNewTradeSkillTitleAvailable(int t, int skill_value);
	void CreateNewPlayerTitle(Client* c, std::string title);
	void CreateNewPlayerSuffix(Client* c, std::string suffix);
	bool HasTitle(Client* c, uint32 title_id);
	inline const std::vector<TitlesRepository::Titles>& GetTitles() { return titles; }

protected:
	std::vector<TitlesRepository::Titles> titles;
};

extern TitleManager title_manager;

#endif

