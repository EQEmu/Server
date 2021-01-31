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

#include "expedition_lockout_timer.h"
#include "../common/string_util.h"
#include "../common/rulesys.h"
#include "../common/util/uuid.h"
#include <fmt/format.h>

const char* const DZ_REPLAY_TIMER_NAME = "Replay Timer"; // see December 14, 2016 patch notes

ExpeditionLockoutTimer::ExpeditionLockoutTimer(
	const std::string& expedition_uuid, const std::string& expedition_name,
	const std::string& event_name, uint64_t expire_time, uint32_t duration
) :
	m_expedition_uuid(expedition_uuid),
	m_expedition_name(expedition_name),
	m_event_name(event_name),
	m_expire_time(std::chrono::system_clock::from_time_t(expire_time)),
	m_duration(duration)
{
	if (event_name == DZ_REPLAY_TIMER_NAME)
	{
		m_is_replay_timer = true;
	}
}

ExpeditionLockoutTimer ExpeditionLockoutTimer::CreateLockout(
	const std::string& expedition_name, const std::string& event_name, uint32_t seconds, std::string uuid)
{
	seconds = static_cast<uint32_t>(seconds * RuleR(Expedition, LockoutDurationMultiplier));

	if (uuid.empty())
	{
		uuid = EQ::Util::UUID::Generate().ToString();
	}

	ExpeditionLockoutTimer lockout{uuid, expedition_name, event_name, 0, seconds};
	lockout.Reset(); // sets expire time
	return lockout;
}

uint32_t ExpeditionLockoutTimer::GetSecondsRemaining() const
{
	auto now = std::chrono::system_clock::now();
	if (m_expire_time > now)
	{
		auto remaining = m_expire_time - now;
		return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(remaining).count());
	}
	return 0;
}

ExpeditionLockoutTimer::DaysHoursMinutes ExpeditionLockoutTimer::GetDaysHoursMinutesRemaining() const
{
	auto seconds = GetSecondsRemaining();
	return ExpeditionLockoutTimer::DaysHoursMinutes{
		fmt::format_int(seconds / 86400).str(),       // days
		fmt::format_int((seconds / 3600) % 24).str(), // hours
		fmt::format_int((seconds / 60) % 60).str()    // minutes
	};
}

bool ExpeditionLockoutTimer::IsSameLockout(const ExpeditionLockoutTimer& compare_lockout) const
{
	return compare_lockout.IsSameLockout(GetExpeditionName(), GetEventName());
}

bool ExpeditionLockoutTimer::IsSameLockout(
	const std::string& expedition_name, const std::string& event_name) const
{
	return GetExpeditionName() == expedition_name && GetEventName() == event_name;
}

void ExpeditionLockoutTimer::AddLockoutTime(int seconds)
{
	seconds = static_cast<uint32_t>(seconds * RuleR(Expedition, LockoutDurationMultiplier));

	auto new_duration = std::max(0, static_cast<int>(m_duration.count()) + seconds);

	auto start_time = m_expire_time - m_duration;
	m_duration = std::chrono::seconds(new_duration);
	m_expire_time = start_time + m_duration;
}
