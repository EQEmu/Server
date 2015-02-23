#include "item_container.h"
#include <map>
#include <utility>

struct EQEmu::ItemContainer::impl
{
	std::map<int, std::shared_ptr<ItemInstance>> items;
};

EQEmu::ItemContainer::ItemContainer()
{
	impl_ = new impl();
}

EQEmu::ItemContainer::~ItemContainer()
{
	if(impl_)
		delete impl_;
}

EQEmu::ItemContainer::ItemContainer(ItemContainer &&other) {
	impl_ = other.impl_;
	other.impl_ = nullptr;
}

std::shared_ptr<EQEmu::ItemInstance> EQEmu::ItemContainer::Get(const int slot_id) {
	auto iter = impl_->items.find(slot_id);
	if(iter != impl_->items.end()) {
		return iter->second;
	}

	return std::shared_ptr<EQEmu::ItemInstance>(nullptr);
}

bool EQEmu::ItemContainer::Put(const int slot_id, std::shared_ptr<ItemInstance> inst) {
	if(!inst)
		return false;

	auto iter = impl_->items.find(slot_id);
	if(iter == impl_->items.end()) {
		impl_->items[slot_id] = inst;
		//trigger insert in slot_id
		return true;
	}

	return false;
}

bool EQEmu::ItemContainer::Delete(const int slot_id) {
	auto iter = impl_->items.find(slot_id);
	if(iter == impl_->items.end()) {
		return false;
	} else {
		impl_->items.erase(iter);
		//trigger delete in slotid
		return true;
	}
}

bool EQEmu::ItemContainer::Serialize(MemoryBuffer &buf, int container_number) {
	if(impl_->items.size() == 0) {
		return false;
	}

	for(auto &iter : impl_->items) {
		buf.Write<int32>(container_number);
		buf.Write<int32>(iter.first);
		buf.Write<void*>(iter.second.get());
	}

	return true;
}