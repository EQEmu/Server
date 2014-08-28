/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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
#ifndef __CONDITION_H
#define __CONDITION_H

#include "debug.h"
#ifndef WIN32
#include <pthread.h>
#endif

//Sombody, someday needs to figure out how to implement a condition
//system on windows...


class Condition {
	private:
#ifdef WIN32
		enum {
			SignalEvent = 0,
			BroadcastEvent,
			_eventCount
		};

		HANDLE m_events[_eventCount];
		uint32 m_waiters;
		CRITICAL_SECTION CSMutex;
#else
		pthread_cond_t cond;
		pthread_mutex_t mutex;
#endif
	public:
		Condition();
		void Signal();
		void SignalAll();
		void Wait();
//		bool TimedWait(unsigned long usec);
		~Condition();
};

#endif

