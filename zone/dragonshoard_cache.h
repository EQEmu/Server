#pragma once

#include "common/types.h"

#include <map>
#include <optional>

class SharedDatabase;

namespace EQ {
	class ItemInstance;
}

namespace DragonHoard {

// A character's Dragon's Hoard contents. Rows are read from dragonhoard_items on first use and
// served from memory afterwards; a mutation changes the cache only after its row is written.
class HoardCache {
public:
	HoardCache();
	~HoardCache();

	HoardCache(const HoardCache&) = delete;
	HoardCache& operator=(const HoardCache&) = delete;

	// Slot capacity, read from the rule when the character enters the zone and kept for the zone
	// session, so a rule reload does not change it under a connected client.
	uint32 SlotLimit() const { return m_slot_limit; }

	// Lowest unused slot id. The caller enforces the slot limit.
	uint32 FindFreeSlot(SharedDatabase& db, uint32 character_id);

	// Writes the row, then takes ownership of inst. Returns false, leaving inst with the caller,
	// if the write failed.
	bool Deposit(SharedDatabase& db, uint32 character_id, uint32 slot_id, EQ::ItemInstance* inst);

	// Slot of the stored item with this serial number.
	std::optional<uint32> FindSlot(SharedDatabase& db, uint32 character_id, int32 serial_number);

	// The item stored in a slot, or nullptr. The cache keeps ownership.
	const EQ::ItemInstance* Get(uint32 slot_id) const;

	// Deletes the row, then hands the item to the caller. Returns nullptr if the slot is empty or
	// the delete failed.
	EQ::ItemInstance* Retrieve(SharedDatabase& db, uint32 character_id, uint32 slot_id);

	// Calls f(uint32 slot_id, const EQ::ItemInstance* inst) for each stored item, in slot order.
	template <typename F>
	void ForEach(SharedDatabase& db, uint32 character_id, F&& f)
	{
		EnsureLoaded(db, character_id);

		for (const auto& [slot_id, inst] : m_slots) {
			if (inst) {
				f(slot_id, inst);
			}
		}
	}

private:
	using SlotMap = std::map<uint32, EQ::ItemInstance*>;

	void EnsureLoaded(SharedDatabase& db, uint32 character_id);

	// slot id -> owned item. A null entry reserves a slot whose row could not be loaded.
	SlotMap      m_slots;
	const uint32 m_slot_limit;
	bool         m_loaded = false;
};

} // namespace DragonHoard
