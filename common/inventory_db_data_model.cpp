#include "inventory_db_data_model.h"
#include "shareddb.h"
#include "string_util.h"
#include <list>
#include <string>

enum DataEventTypes
{
	DB_Insert,
	DB_Delete
};

struct DataEvent
{
	DataEventTypes evt;
	EQEmu::InventorySlot slot;
	std::shared_ptr<EQEmu::ItemInstance> inst;
};

struct EQEmu::InventoryDatabaseDataModel::impl {
	SharedDatabase *db_;
	std::list<DataEvent> events_;
	uint32 char_id_;
};

EQEmu::InventoryDatabaseDataModel::InventoryDatabaseDataModel(SharedDatabase *db, uint32 char_id) {
	impl_ = new impl;
	impl_->db_ = db;
	impl_->char_id_ = char_id;
}

EQEmu::InventoryDatabaseDataModel::~InventoryDatabaseDataModel() {
	delete impl_;
}

void EQEmu::InventoryDatabaseDataModel::Begin() {
	impl_->db_->TransactionBegin();
	impl_->events_.clear();
}

bool EQEmu::InventoryDatabaseDataModel::Commit() {
	std::string base_insert = "INSERT INTO character_inventory(id, type, slot, bag_index, aug_index, "
		"item_id, charges, color, attuned, custom_data, ornament_icon, ornament_idfile, ornament_hero_model"
		", tracking_id) VALUES";

	std::string current_insert = base_insert;
	bool insert = false;
	for(auto iter : impl_->events_) {
		if(iter.evt == DB_Delete) {
			if(insert) {
				insert = false;

				//commit the current_insert
				auto res = impl_->db_->QueryDatabase(current_insert);
				if(!res.Success()) {
					Rollback();
					return false;
				}
				
				current_insert = base_insert;
			}

			std::string current_delete;
			if(iter.slot.BagIndex() > -1) {
				if(iter.slot.AugIndex() > -1) {
					current_delete = StringFormat("DELETE FROM character_inventory WHERE id=%u AND type=%u AND slot=%u AND bag_index=%u AND aug_index=%u",
												  impl_->char_id_, iter.slot.Type(), iter.slot.Slot(), iter.slot.BagIndex(), iter.slot.AugIndex());
				}
				else {
					current_delete = StringFormat("DELETE FROM character_inventory WHERE id=%u AND type=%u AND slot=%u AND bag_index=%u",
												  impl_->char_id_, iter.slot.Type(), iter.slot.Slot(), iter.slot.BagIndex());
				}
			}
			else if(iter.slot.AugIndex() > -1) {
				current_delete = StringFormat("DELETE FROM character_inventory WHERE id=%u AND type=%u AND slot=%u AND aug_index=%u",
											  impl_->char_id_, iter.slot.Type(), iter.slot.Slot(), iter.slot.AugIndex());
			}
			else {
				current_delete = StringFormat("DELETE FROM character_inventory WHERE id=%u AND type=%u AND slot=%u",
											  impl_->char_id_, iter.slot.Type(), iter.slot.Slot());
			}

			auto res = impl_->db_->QueryDatabase(current_delete);
			if(!res.Success()) {
				Rollback();
				return false;
			}

		} else {
			//insert
			if(!insert) {
				insert = true;
			} else {
				current_insert += ",";
			}

			current_insert += StringFormat("(%u, %i, %i, %i, %i, %u, %i, %u, %u, '%s', %u, %u, %u, %llu)", 
										   impl_->char_id_, 
										   iter.slot.Type(), 
										   iter.slot.Slot(), 
										   iter.slot.BagIndex(), 
										   iter.slot.AugIndex(),
										   iter.inst->GetBaseItem()->ID,
										   iter.inst->GetCharges(),
										   iter.inst->GetColor(),
										   iter.inst->GetAttuned(),
										   EscapeString(iter.inst->GetCustomData()).c_str(),
										   iter.inst->GetOrnamentIcon(),
										   iter.inst->GetOrnamentIDFile(),
										   iter.inst->GetOrnamentHeroModel(),
										   iter.inst->GetTrackingID());
		}
	}

	if(insert) {
		insert = false;

		//commit the current_insert
		auto res = impl_->db_->QueryDatabase(current_insert);
		if(!res.Success()) {
			Rollback();
			return false;
		}

		current_insert = base_insert;
	}

	impl_->db_->TransactionCommit();
	impl_->events_.clear();
	return true;
}

void EQEmu::InventoryDatabaseDataModel::Rollback() {
	impl_->db_->TransactionRollback();
	impl_->events_.clear();
}

void EQEmu::InventoryDatabaseDataModel::Insert(const InventorySlot &slot, std::shared_ptr<ItemInstance> inst) {
	DataEvent evt;
	evt.evt = DB_Insert;
	evt.inst = inst;
	evt.slot = slot;
	impl_->events_.push_back(evt);

	//insert current item
	if(slot.BagIndex() < 0 && slot.AugIndex() < 0) {
		//if bag put all bag contents in
		//if common put all augment contents in
		if(inst->GetBaseItem()->ItemClass == ItemClassContainer) {
			auto container = inst->GetContainer();
			auto iter = container->Begin();
			while(iter != container->End()) {
				DataEvent evt;
				evt.evt = DB_Insert;
				evt.inst = iter->second;
				evt.slot = InventorySlot(slot.Type(), slot.Slot(), iter->first, -1);
				impl_->events_.push_back(evt);

				//do augments here
				if(evt.inst->GetBaseItem()->ItemClass == ItemClassCommon) {
					auto inst_container = evt.inst->GetContainer();
					auto inst_iter = inst_container->Begin();
					while(inst_iter != inst_container->End()) {
						DataEvent evt;
						evt.evt = DB_Insert;
						evt.inst = inst_iter->second;
						evt.slot = InventorySlot(slot.Type(), slot.Slot(), iter->first, inst_iter->first);
						impl_->events_.push_back(evt);
						++inst_iter;
					}
				}

				++iter;
			}
		}
		else if(inst->GetBaseItem()->ItemClass == ItemClassCommon) {
			auto container = inst->GetContainer();
			auto iter = container->Begin();
			while(iter != container->End()) {
				DataEvent evt;
				evt.evt = DB_Insert;
				evt.inst = iter->second;
				evt.slot = InventorySlot(slot.Type(), slot.Slot(), -1, iter->first);
				impl_->events_.push_back(evt);

				++iter;
			}
		}
	}
	else if(slot.AugIndex() < 0 && inst->GetBaseItem()->ItemClass == ItemClassCommon) {
		//bag item that can have augs
		//if common put all augment contents in
		auto container = inst->GetContainer();
		auto iter = container->Begin();
		while(iter != container->End()) {
			DataEvent evt;
			evt.evt = DB_Insert;
			evt.inst = iter->second;
			evt.slot = InventorySlot(slot.Type(), slot.Slot(), slot.BagIndex(), iter->first);
			impl_->events_.push_back(evt);
		
			++iter;
		}
	}
}

void EQEmu::InventoryDatabaseDataModel::Delete(const InventorySlot &slot) {
	DataEvent evt;
	evt.evt = DB_Delete;
	evt.slot = slot;
	impl_->events_.push_back(evt);
}
