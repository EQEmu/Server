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
#ifndef TIMER_H
#define TIMER_H

#include "types.h"

// Disgrace: for windows compile
#ifdef _WINDOWS
	#include "debug.h"
	int gettimeofday (timeval *tp, ...);
#endif

class Timer
{
public:
	Timer(uint32 timer_time, bool iUseAcurateTiming = false);
	Timer(uint32 start, uint32 timer, bool iUseAcurateTiming);
	~Timer() { }

	bool Check(bool iReset = true);
	void Enable();
	void Disable();
	void Start(uint32 set_timer_time=0, bool ChangeResetTimer = true);
	void SetTimer(uint32 set_timer_time=0);
	uint32 GetRemainingTime();
	inline const uint32& GetTimerTime()		{ return timer_time; }
	inline const uint32& GetSetAtTrigger()	{ return set_at_trigger; }
	void Trigger();
	void SetAtTrigger(uint32 set_at_trigger, bool iEnableIfDisabled = false);

	inline bool Enabled() { return enabled; }
	inline uint32 GetStartTime() { return(start_time); }
	inline uint32 GetDuration() { return(timer_time); }

	static const uint32 SetCurrentTime();
	static const uint32 GetCurrentTime();
	static const uint32 GetTimeSeconds();

private:
	uint32	start_time;
	uint32	timer_time;
	bool	enabled;
	uint32	set_at_trigger;

	// Tells the timer to be more acurate about happening every X ms.
	// Instead of Check() setting the start_time = now,
	// it it sets it to start_time += timer_time
	bool	pUseAcurateTiming;

//	static uint32 current_time;
//	static uint32 last_time;
};

#endif
