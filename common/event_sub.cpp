#include "event_sub.h"
#include <string.h>

void EventSubscriptionWatcher::Subscribe(const std::string &event_name)
{
	m_subs[event_name] = 1;
}

void EventSubscriptionWatcher::Unsubscribe(const std::string &event_name)
{
	m_subs[event_name] = 0;
}

bool EventSubscriptionWatcher::IsSubscribed(const std::string &event_name) const
{
	auto iter = m_subs.find(event_name);
	if (iter != m_subs.end()) {
		return iter->second;
	}

	return false;
}
