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

#include "metric_manager.h"
#include "metric_event.h"

EQEmu::MetricManager::MetricManager() {

	m_current_event_id = 0;
}

void EQEmu::MetricManager::Process() {

	for (std::list<MetricEvent*>::iterator iter = m_event_list.begin(); iter != m_event_list.end(); ) {

		if ((*iter)->IsExpired()) {

			iter = m_event_list.erase(iter);
			continue;
		}

		(*iter)->Process();
		++iter;
	}
}

int EQEmu::MetricManager::RegisterEvent(MetricEvent* event_object) {

	if (event_object == nullptr) {
		return 0;
	}

	event_object->SetEventId(get_next_event_id());
	m_event_list.push_back(event_object);

	return event_object->GetEventId();
}

void EQEmu::MetricManager::FlushAll() {

	for (auto iter : m_event_list) {
		iter->Flush();
	}
}

void EQEmu::MetricManager::FlushById(int event_id) {

	for (auto iter : m_event_list) {
		if (iter->GetEventId() == event_id) {

			iter->Flush();
			break;
		}
	}
}

void EQEmu::MetricManager::FinalizeAll() {

	for (auto iter : m_event_list) {
		iter->Finalize();
	}
}

void EQEmu::MetricManager::FinalizeById(int event_id) {

	for (auto iter : m_event_list) {
		if (iter->GetEventId() == event_id) {

			iter->Finalize();
			break;
		}
	}
}

void EQEmu::MetricManager::ExpireAll() {

	for (auto iter : m_event_list) {
		iter->Expire();
	}
}

void EQEmu::MetricManager::ExpireById(int event_id) {

	for (auto iter : m_event_list) {
		if (iter->GetEventId() == event_id) {

			iter->Expire();
			break;
		}
	}
}

int EQEmu::MetricManager::get_next_event_id() {

	return ++m_current_event_id;
}
