#include "combat_record.h"
#include "../common/eqemu_logsys.h"
#include "../common/string_util.h"

void CombatRecord::Start(std::string in_mob_name)
{
	start_time      = std::time(nullptr);
	end_time        = 0;
	damage_received = 0;
	heal_received   = 0;
	mob_name        = in_mob_name;
}

void CombatRecord::Stop()
{
	end_time = std::time(nullptr);

	double time_in_combat = TimeInCombat();

	LogCombatRecord(
		"[Summary] Mob [{}] [Received] DPS [{:.0f}] Heal/s [{:.0f}] Duration [{}] ({}s)",
		mob_name,
		time_in_combat > 0 ? (damage_received / time_in_combat) : damage_received,
		time_in_combat > 0 ? (heal_received / time_in_combat) : heal_received,
		time_in_combat > 0 ? ConvertSecondsToTime(time_in_combat) : "",
		time_in_combat
	);
}

bool CombatRecord::InCombat()
{
	return start_time > 0;
}

void CombatRecord::ProcessHPEvent(int64 hp, int64 current_hp)
{
	// damage
	if (hp < current_hp) {
		damage_received = damage_received + std::llabs(current_hp - hp);
	}

	// heal
	if (hp > current_hp && current_hp > 0) {
		heal_received = heal_received + std::llabs(current_hp - hp);
	}

	LogCombatRecordDetail(
		"damage_received [{}] heal_received [{}] current_hp [{}] hp [{}] calc [{}]",
		damage_received,
		heal_received,
		current_hp,
		hp,
		std::llabs(current_hp - hp)
	);
}

double CombatRecord::TimeInCombat() const
{
	return difftime(end_time, start_time);
}
