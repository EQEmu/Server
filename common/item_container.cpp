#include "item_container.h"
#include "item_container_default_serialization.h"
#include <utility>

struct EQEmu::ItemContainer::impl
{
	std::map<int, std::shared_ptr<ItemInstance>> items_;
	ItemContainerSerializationStrategy *serialize_strat_;
};

EQEmu::ItemContainer::ItemContainer()
{
	impl_ = new impl();
	impl_->serialize_strat_ = new ItemContainerDefaultSerialization();
}

EQEmu::ItemContainer::ItemContainer(ItemContainerSerializationStrategy *strategy) {
	impl_ = new impl();
	impl_->serialize_strat_ = strategy;
}

EQEmu::ItemContainer::~ItemContainer()
{
	if(impl_) {
		delete impl_->serialize_strat_;
		delete impl_;
	}
}

EQEmu::ItemContainer::ItemContainer(ItemContainer &&other) {
	impl_ = other.impl_;
	other.impl_ = nullptr;
}

EQEmu::ItemContainer& EQEmu::ItemContainer::operator=(ItemContainer &&other) {
	if(this == &other)
		return *this;

	impl_ = other.impl_;
	other.impl_ = nullptr;
	return *this;
}

std::shared_ptr<EQEmu::ItemInstance> EQEmu::ItemContainer::Get(const int slot_id) {
	auto iter = impl_->items_.find(slot_id);
	if(iter != impl_->items_.end()) {
		return iter->second;
	}

	return std::shared_ptr<EQEmu::ItemInstance>(nullptr);
}

bool EQEmu::ItemContainer::Put(const int slot_id, std::shared_ptr<ItemInstance> inst) {
	if(!inst) {
		impl_->items_.erase(slot_id);
		return true;
	} else {
		auto iter = impl_->items_.find(slot_id);
		if(iter == impl_->items_.end()) {
			impl_->items_[slot_id] = inst;
			return true;
		}
	}

	return false;
}

bool EQEmu::ItemContainer::HasItem(uint32 item_id) {
	for(auto &item : impl_->items_) {
		if(item.second->GetBaseItem()->ID == item_id) {
			return true;
		}
	}

	return false;
}

bool EQEmu::ItemContainer::HasItemByLoreGroup(uint32 loregroup) {
	if(loregroup == 0xFFFFFFFF)
		return false;

	for(auto &item : impl_->items_) {
		if(item.second->GetBaseItem()->LoreGroup == loregroup) {
			return true;
		}
	}

	return false;
}

uint32 EQEmu::ItemContainer::Size() {
	return (uint32)impl_->items_.size();
}

uint32 EQEmu::ItemContainer::Size() const {
	return (uint32)impl_->items_.size();
}

bool EQEmu::ItemContainer::Delete(const int slot_id) {
	auto iter = impl_->items_.find(slot_id);
	if(iter == impl_->items_.end()) {
		return false;
	} else {
		impl_->items_.erase(iter);
		return true;
	}
}

bool EQEmu::ItemContainer::Serialize(MemoryBuffer &buf, int container_number) {
	if(impl_->serialize_strat_) {
		return impl_->serialize_strat_->Serialize(buf, container_number, impl_->items_);
	}

	return false;
}

EQEmu::ItemContainer::ItemContainerIter EQEmu::ItemContainer::Begin() {
	return impl_->items_.begin();
}

EQEmu::ItemContainer::ItemContainerIter EQEmu::ItemContainer::End() {
	return impl_->items_.end();
}

void EQEmu::ItemContainer::Interrogate(int level) {
	char buffer[16] = { 0 };
	for(int i = 0; i < level; ++i) {
		buffer[i] = '\t';
	}

	for(auto &iter : impl_->items_) {
		printf("%s%u: (%u)%s (%u)\n", buffer, iter.first, iter.second->GetBaseItem()->ID, iter.second->GetBaseItem()->Name, iter.second->GetCharges());
		iter.second->Interrogate(level + 1);
	}
}