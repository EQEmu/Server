#include "zone.h"

BaseDataRepository::BaseData Zone::GetBaseData(uint8 level, uint8 class_id)
{
	for (const auto& e : m_base_data) {
		if (e.level == level && e.class_ == class_id) {
			return e;
		}
	}

	return BaseDataRepository::NewEntity();
}

void Zone::LoadBaseData()
{
	const auto& l = BaseDataRepository::All(content_db);

	if (l.empty()) {
		return;
	}

	for (const auto& e : l) {
		if (e.level < 1 || !IsPlayerClass(e.class_)) {
			continue;
		}

		m_base_data.emplace_back(e);
	}
}

void Zone::ReloadBaseData()
{
	ClearBaseData();
	LoadBaseData();
}
