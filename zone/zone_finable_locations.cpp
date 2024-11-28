#include "zone.h"

void Zone::LoadFindableLocations() {
	m_findable_locations = content_db.LoadFindableLocations(GetShortName(), GetInstanceVersion());
	if (m_findable_locations.empty()) {
		LogInfo("No findable loacations loaded");
		return;
	}
}

void Zone::ReloadFindableLocations() {
	ClearFindableLocations();
	LoadFindableLocations();
}

void Zone::ClearFindableLocations() {
	m_findable_locations.clear();
}

void Zone::SendFindableLocations(Client* client) {
	if (m_findable_locations.empty()) {
		return;
	}
	
	size_t size = sizeof(uint32) + (m_findable_locations.size() * sizeof(FindableLocation_Struct));
	auto outapp = new EQApplicationPacket(OP_SendFindableLocations, size);

	outapp->WriteUInt32(m_findable_locations.size());

	for (auto& location : m_findable_locations) {
		outapp->WriteUInt32(location.type);
		outapp->WriteSInt32(location.findable_id);
		outapp->WriteSInt32(location.findable_sub_id);
		outapp->WriteSInt32(location.zone_id);
		outapp->WriteSInt32(location.zone_id_index);
		outapp->WriteFloat(location.y);
		outapp->WriteFloat(location.x);
		outapp->WriteFloat(location.z);
	}

	client->FastQueuePacket(&outapp);
}
