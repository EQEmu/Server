#ifndef EQEMU_PLAYER_EVENTS_H
#define EQEMU_PLAYER_EVENTS_H

#include <string>
#include <cereal/cereal.hpp>
#include "../types.h"

namespace PlayerEvent {

	enum EventType {
		GM_COMMAND = 1,
		ZONING,
		AA_GAIN, // unimplemented
		AA_PURCHASE, // unimplemented
		FORAGE_SUCCESS, // unimplemented
		FORAGE_FAILURE, // unimplemented
		FISH_SUCCESS, // unimplemented
		FISH_FAILURE, // unimplemented
		ITEM_DESTROY, // unimplemented
		WENT_ONLINE, // unimplemented
		WENT_OFFLINE, // unimplemented
		LEVEL_GAIN, // unimplemented
		LEVEL_LOSS, // unimplemented
		LOOT_ITEM, // unimplemented
		MERCHANT_PURCHASE, // unimplemented
		MERCHANT_SELL, // unimplemented
		GROUP_JOIN, // unimplemented
		GROUP_LEAVE, // unimplemented
		RAID_JOIN, // unimplemented
		RAID_LEAVE, // unimplemented
		GROUNDSPAWN_PICKUP, // unimplemented
		NPC_HANDIN, // unimplemented
		SKILL_UP, // unimplemented
		TASK_ACCEPT, // unimplemented
		TASK_UPDATE, // unimplemented
		TASK_COMPLETE, // unimplemented
		TRADE, // unimplemented
		GIVE_ITEM, // unimplemented
		SAY, // unimplemented
		REZ_ACCEPTED, // unimplemented
		DEATH, // unimplemented
		COMBINE_FAILURE, // unimplemented
		COMBINE_SUCCESS, // unimplemented
		MAX // dont remove
	};

	// tied to Event
	static const char *EventName[PlayerEvent::MAX] = {
		"None",
		"GM Command",
		"Zoning",
		"AA Gain",
		"AA Purchase",
		"Forage Success",
		"Forage Failure",
		"Fish Success",
		"Fish Failure",
		"Item Destroy",
		"Went Online",
		"Went Offline",
		"Level Gain",
		"Level Loss",
		"Loot Item",
		"Merchant Purchase",
		"Merchant Sell",
		"Group Join",
		"Group Leave",
		"Raid Join",
		"Raid Leave",
		"Groundspawn Pickup",
		"NPC Handin",
		"Skill Up",
		"Task Accept",
		"Task Update",
		"Task Complete",
		"Trade",
		"Given Item",
		"Say",
		"Rez Accepted",
		"Death",
		"Combine Failure",
		"Combine Success",
	};

	// Generic struct used by all events
	struct PlayerEvent {
		int64       account_id;
		int64       character_id;
		std::string character_name;
		int         zone_id;
		int         instance_id;
		float       x;
		float       y;
		float       z;
		float       heading;
	};

	// used in Trade event
	struct TradeItem {
		int64       item_id;
		std::string item_name;
		int32       slot;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(slot)
			);
		}
	};

	// used in Trade event
	class TradeEntry {
	public:
		int64                  character_id;
		std::string            character_name;
		int32                  coin;
		std::vector<TradeItem> items;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(character_id),
				CEREAL_NVP(character_name),
				CEREAL_NVP(coin),
				CEREAL_NVP(items)
			);
		}
	};

	/**
	 * Events
	 */
	struct TradeEvent {
		std::vector<TradeEntry> entries;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(entries)
			);
		}
	};

	struct GMCommandEvent {
		std::string message;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(message)
			);
		}
	};

	struct ZoningEvent {
		int32 from_zone_id;
		int32 from_instance_id;
		int32 from_instance_version;
		int32 to_zone_id;
		int32 to_instance_id;
		int32 to_instance_version;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(from_zone_id),
				CEREAL_NVP(from_instance_id),
				CEREAL_NVP(from_instance_version),
				CEREAL_NVP(to_zone_id),
				CEREAL_NVP(to_instance_id),
				CEREAL_NVP(to_instance_version)
			);
		}
	};
}

#endif //EQEMU_PLAYER_EVENTS_H
