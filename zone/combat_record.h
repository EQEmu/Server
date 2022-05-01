#ifndef EQEMU_COMBAT_RECORD_H
#define EQEMU_COMBAT_RECORD_H

#include <ctime>
#include <string>

class CombatRecord {
public:
	void Start(std::string in_mob_name);
	void Stop();
	bool InCombat();
	void ProcessHPEvent(int hp, int current_hp);
	double TimeInCombat() const;
private:
	std::string mob_name;
	time_t      start_time      = 0;
	time_t      end_time        = 0;
	__int64_t   damage_received = 0;
	__int64_t   heal_received   = 0;
};

#endif //EQEMU_COMBAT_RECORD_H
