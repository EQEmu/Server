#ifndef EQEMU_COMBAT_RECORD_H
#define EQEMU_COMBAT_RECORD_H

#include <ctime>
#include <string>
#include "../common/types.h"

class CombatRecord {
public:
	void Start(std::string in_mob_name);
	void Stop();
	bool InCombat();
	void ProcessHPEvent(int64 hp, int64 current_hp);
	double TimeInCombat() const;
private:
	std::string mob_name;
	time_t      start_time      = 0;
	time_t      end_time        = 0;
	int64       damage_received = 0;
	int64       heal_received   = 0;
};

#endif //EQEMU_COMBAT_RECORD_H
