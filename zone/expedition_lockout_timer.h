/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef EXPEDITION_LOCKOUT_TIMER_H
#define EXPEDITION_LOCKOUT_TIMER_H

#include <string>

extern const char* const DZ_REPLAY_TIMER_NAME;

class ExpeditionLockoutTimer
{
public:
	ExpeditionLockoutTimer() {}
	ExpeditionLockoutTimer(std::string expedition_name, std::string event_name, uint64_t expire_time, uint32_t duration, bool inherited = false);

	struct DaysHoursMinutes
	{
		std::string days;
		std::string hours;
		std::string mins;
	};

	uint32_t GetDuration() const { return m_duration; }
	uint64_t GetExpireTime() const { return m_expire_time; }
	uint32_t GetSecondsRemaining() const;
	DaysHoursMinutes GetDaysHoursMinutesRemaining() const;
	const std::string& GetExpeditionName() const { return m_expedition_name; }
	const std::string& GetEventName() const { return m_event_name; }
	void SetExpireTime(uint64_t expire_time) { m_expire_time = expire_time; }
	void SetInherited(bool is_inherited) { m_is_inherited = is_inherited; }
	bool IsExpired() const { return GetSecondsRemaining() == 0; }
	bool IsInherited() const { return m_is_inherited; }
	bool IsReplayTimer() const { return m_is_replay_timer; }
	bool IsSameLockout(const ExpeditionLockoutTimer& compare_lockout) const;
	bool IsSameLockout(const std::string& expedition_name, const std::string& event_name) const;

private:
	std::string m_expedition_name;
	std::string m_event_name;
	uint64_t    m_expire_time     = 0;
	uint32_t    m_duration        = 0;
	bool        m_is_inherited    = false; // inherited from expedition leader
	bool        m_is_replay_timer = false;
};

#endif
