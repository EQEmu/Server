#ifndef EQEMU_CHARACTER_EVOLVING_ITEMS_REPOSITORY_H
#define EQEMU_CHARACTER_EVOLVING_ITEMS_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_character_evolving_items_repository.h"

#include <boost/tuple/tuple_comparison.hpp>

class CharacterEvolvingItemsRepository: public BaseCharacterEvolvingItemsRepository {
public:
	// Custom extended repository methods here

	static CharacterEvolvingItems SetCurrentAmountAndProgression(Database& db, const uint64 id, const uint64 amount, const double progression)
	{
		auto e = FindOne(db, id);
		if (e.id == 0) {
			return NewEntity();
		}

		e.current_amount = amount;
		e.progression    = progression;
		e.deleted_at     = 0;
		UpdateOne(db, e);
		return e;
	}

	static CharacterEvolvingItems SetEquipped(Database& db, const uint64 id, const bool equipped)
	{
		auto e = FindOne(db, id);
		if (e.id == 0) {
			return NewEntity();
		}

		e.equipped   = equipped;
		e.deleted_at = 0;
		UpdateOne(db, e);
		return e;
	}

	static CharacterEvolvingItems SoftDelete(Database& db, const uint64 id)
	{
		auto e = FindOne(db, id);
		if (e.id == 0) {
			return NewEntity();
		}

		e.deleted_at = time(nullptr);
		UpdateOne(db, e);
		return e;
	}

	static bool UpdateCharID(Database &db, const uint64 id, const uint32 to_char_id)
	{
		auto e = FindOne(db, id);
		if (e.id == 0) {
			return false;
		}

		e.character_id = to_char_id;
		e.deleted_at   = 0;
		return UpdateOne(db, e);
	}
};

#endif //EQEMU_CHARACTER_EVOLVING_ITEMS_REPOSITORY_H
