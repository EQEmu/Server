/*	EQEMu: Everquest Server Emulator
	
	Copyright (C) 2001-2020 EQEMu Development Team (http://eqemulator.net)

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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef METRIC_EVENT_H
#define METRIC_EVENT_H

#include "types.h"

namespace EQEmu
{
	class MetricEvent {
	public:

		enum class EventType {
			eventNone,
			eventNpcStatsMonitor
		};

		MetricEvent() {
			
			m_finalized = false;
			m_expired = false;
			m_event_id = 0;
		}

		virtual EventType GetEventType() const = 0;

		virtual void Process() = 0;
		virtual void Flush() = 0;

		virtual void Finalize() { m_finalized = true; } // invoke MetricEvent::Finalize() inside of derived class function if not handled locally
		bool IsFinalized() const { return m_finalized; }

		virtual void Expire() { m_finalized = true; m_expired = true; } // invoke MetricEvent::Expire() inside of derived class function if not handled locally
		bool IsExpired() const { return m_expired; }

		void SetEventId(int value) { if (m_event_id == 0) { m_event_id = value; } }
		int GetEventId() const { return m_event_id; }

	private:

		bool m_finalized;
		bool m_expired;
		int m_event_id;
	};

} // EQEmu

#endif // METRIC_EVENT_H
