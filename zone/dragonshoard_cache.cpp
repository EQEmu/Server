#include "zone/dragonshoard_cache.h"

#include "common/eqemu_logsys.h"
#include "common/item_instance.h"
#include "common/repositories/character_evolving_items_repository.h"
#include "common/repositories/dragonhoard_items_repository.h"
#include "common/rulesys.h"
#include "common/shareddb.h"

#include <fmt/format.h>

#include <algorithm>

namespace DragonHoard {

HoardCache::HoardCache()
	: m_slot_limit(static_cast<uint32>(std::max(RuleI(Features, DragonsHoard), 0)))
{
}

HoardCache::~HoardCache()
{
	for (auto& [slot_id, inst] : m_slots) {
		safe_delete(inst);
	}
}

void HoardCache::EnsureLoaded(SharedDatabase& db, uint32 character_id)
{
	if (m_loaded) {
		return;
	}

	m_loaded = true;

	const auto rows = DragonhoardItemsRepository::GetWhere(
		db, fmt::format("character_id = {}", character_id));

	for (const auto& row : rows) {
		EQ::ItemInstance* inst = db.CreateItem(
			row.item_id, row.charges, 0, 0, 0, 0, 0, 0, row.attuned != 0, row.custom_data,
			row.ornament_icon, row.ornament_idfile, static_cast<uint32>(row.ornament_hero_model)
		);

		if (inst) {
			inst->SetColor(row.color);

			// The record is soft-deleted while the item is in the hoard. Clear deleted_at in the copy:
			// retrieve restores the record, and a later save of these details must not delete it again.
			if (row.evolve_id) {
				auto evolving = CharacterEvolvingItemsRepository::FindOne(db, row.evolve_id);
				if (evolving.id) {
					evolving.deleted_at = 0;
					inst->GetEvolvingDetails() = evolving;
					inst->CalculateEvolveProgression();
				}
			}
		} else {
			LogError(
				"Dragon's Hoard item [{}] in slot [{}] of character [{}] could not be created",
				row.item_id, row.slot_id, character_id
			);
		}

		m_slots[row.slot_id] = inst;
	}
}

uint32 HoardCache::FindFreeSlot(SharedDatabase& db, uint32 character_id)
{
	EnsureLoaded(db, character_id);

	uint32 free_slot = 0;
	for (const auto& [slot_id, inst] : m_slots) {
		if (slot_id != free_slot) {
			break;
		}

		++free_slot;
	}

	return free_slot;
}

bool HoardCache::Deposit(SharedDatabase& db, uint32 character_id, uint32 slot_id, EQ::ItemInstance* inst)
{
	EnsureLoaded(db, character_id);

	auto e = DragonhoardItemsRepository::NewEntity();
	e.character_id        = character_id;
	e.slot_id             = slot_id;
	e.item_id             = inst->GetItem()->ID;
	e.item_name           = inst->GetItem()->Name;
	e.charges             = inst->GetCharges();
	e.color               = inst->GetColor();
	e.attuned             = inst->IsAttuned() ? 1 : 0;
	e.ornament_icon       = inst->GetOrnamentationIcon();
	e.ornament_idfile     = inst->GetOrnamentationIDFile();
	e.ornament_hero_model = static_cast<int32_t>(inst->GetOrnamentHeroModel());
	e.custom_data         = inst->GetCustomDataString();
	e.evolve_id           = inst->GetEvolveUniqueID();

	// InsertOne reports success through the auto-increment id, which this table does not have.
	if (DragonhoardItemsRepository::InsertMany(db, {e}) == 0) {
		LogError(
			"Dragon's Hoard write failed for character [{}] slot [{}] item [{}]",
			character_id, slot_id, e.item_id
		);
		return false;
	}

	m_slots[slot_id] = inst;

	return true;
}

std::optional<uint32> HoardCache::FindSlot(SharedDatabase& db, uint32 character_id, int32 serial_number)
{
	EnsureLoaded(db, character_id);

	const auto it = std::find_if(m_slots.begin(), m_slots.end(), [serial_number](const auto& slot) {
		return slot.second && slot.second->GetSerialNumber() == serial_number;
	});

	if (it == m_slots.end()) {
		return std::nullopt;
	}

	return it->first;
}

const EQ::ItemInstance* HoardCache::Get(uint32 slot_id) const
{
	const auto it = m_slots.find(slot_id);

	return it != m_slots.end() ? it->second : nullptr;
}

EQ::ItemInstance* HoardCache::Retrieve(SharedDatabase& db, uint32 character_id, uint32 slot_id)
{
	const auto it = m_slots.find(slot_id);
	if (it == m_slots.end() || !it->second) {
		return nullptr;
	}

	const int deleted = DragonhoardItemsRepository::DeleteWhere(
		db, fmt::format("character_id = {} AND slot_id = {}", character_id, slot_id));

	if (deleted == 0) {
		LogError("Dragon's Hoard delete failed for character [{}] slot [{}]", character_id, slot_id);
		return nullptr;
	}

	EQ::ItemInstance* inst = it->second;
	m_slots.erase(it);

	return inst;
}

} // namespace DragonHoard
