#include "item_container_default_serialization.h"

bool EQEmu::ItemContainerDefaultSerialization::Serialize(MemoryBuffer &buf, const int container_number, const std::map<int, std::shared_ptr<ItemInstance>>& items) {
	if(items.size() == 0) {
		return false;
	}

	bool ret = false;
	for(auto &iter : items) {
		buf.Write<int32>(container_number);
		buf.Write<int32>(iter.first);
		buf.Write<int32>(-1);
		buf.Write<int32>(-1);
		buf.Write<void*>(iter.second.get());
		ret = true;
	}

	return ret;
}
