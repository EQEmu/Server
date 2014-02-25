/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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
#include "../common/debug.h"
// Disgrace: for windows compile
#ifndef WIN32
	#include <sys/time.h>
#else
	#include <sys/timeb.h>
#endif

#include <iostream>

#include "timer.h"

uint32 current_time = 0;
uint32 base_time;

Timer::Timer() {
	trigger_time = 0;
	start_time = current_time;
	reset_interval = 0;
	pUseAcurateTiming = false;
	enabled = false;
}

Timer::Timer(uint32 in_timer_time, bool iUseAcurateTiming) {
	start_time = current_time;
	trigger_time = start_time + in_timer_time;
	reset_interval = in_timer_time;
	pUseAcurateTiming = iUseAcurateTiming;
	enabled = (in_timer_time > 0);
}

Timer::Timer(uint32 start, uint32 timer, bool iUseAcurateTiming) {
	trigger_time = start + timer;
	start_time = start;
	reset_interval = timer;
	pUseAcurateTiming = iUseAcurateTiming;
	enabled = (timer > 0);
}

/* Reimplemented for MSVC - Bounce */
#ifdef _WINDOWS
int gettimeofday (timeval *tp, ...)
{
	timeb tb;

	ftime (&tb);

	tp->tv_sec = tb.time;
	tp->tv_usec = tb.millitm * 1000;

	return 0;
}
#endif

/* This function checks if the timer triggered */
bool Timer::Check(bool iReset)
{
	//time check is usually false, enabled is almost always true - check time first
	if (current_time >= trigger_time && enabled) {
		if (iReset) {
			if (pUseAcurateTiming)
				start_time = trigger_time;
			else
				start_time = current_time;
			trigger_time = start_time + reset_interval;
		}
		return true;
	}

	return false;
}

/* This function set the timer and restart it */
void Timer::Start(uint32 set_timer_time, bool ChangeResetTimer) {
	start_time = current_time;
	enabled = true;
	if (set_timer_time != 0)
	{
		trigger_time = start_time + set_timer_time;
		if (ChangeResetTimer)
			reset_interval = set_timer_time;
	}
	else
	{
		trigger_time = start_time + reset_interval;
	}
}

/* This timer updates the timer without restarting it */
void Timer::SetTimer(uint32 set_timer_time) {
	/* If we were disabled before => restart the timer */
	if (!enabled) {
		start_time = current_time;
		enabled = true;
	}
	trigger_time = current_time + set_timer_time;
	reset_interval = set_timer_time;
}

uint32 Timer::GetRemainingTime() {
	if (enabled) {
		if (current_time >= trigger_time)
			return 0;
		else
			return trigger_time - current_time;
	}
	else {
		return 0xFFFFFFFF;
	}
}

void Timer::SetAtTrigger(uint32 in_set_at_trigger, bool iEnableIfDisabled) {
	reset_interval = in_set_at_trigger;
	if (!enabled && iEnableIfDisabled) {
		enabled = true;
	}
}

void Timer::Trigger()
{
	enabled = true;
	trigger_time = current_time;
	start_time = current_time;
}

const uint32 Timer::GetCurrentTime()
{
	return current_time;
}

//just to keep all time related crap in one place... not really related to timers.
const uint32 Timer::GetTimeSeconds() {
#ifdef WIN32
	timeb tb;
	ftime(&tb);
	return tb.time;
#else
	timeval read_time;
	gettimeofday(&read_time,0);
	return read_time.tv_sec;
#endif
}

uint32 Timer::GetTimeMilliseconds()
{
#ifdef WIN32
	timeb tb;
	ftime(&tb);
	return tb.time * 1000 + tb.millitm;
#else
	timeval read_time;
	gettimeofday(&read_time,0);
	return read_time.tv_sec * 1000 + read_time.tv_usec * 0.001;
#endif
}

void Timer::SetCurrentTime()
{
	current_time = GetTimeMilliseconds() - base_time;
}

void Timer::InitBaseTime()
{
	base_time = GetTimeMilliseconds();
}

