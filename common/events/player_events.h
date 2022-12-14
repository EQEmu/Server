#ifndef EQEMU_PLAYER_EVENTS_H
#define EQEMU_PLAYER_EVENTS_H

#include <string>
#include <cereal/cereal.hpp>
#include "../types.h"

namespace PlayerEvent {

	enum EventType {
		GM_COMMAND = 1,
		ZONING,
		AA_GAIN,
		AA_PURCHASE,
		FORAGE_SUCCESS,
		FORAGE_FAILURE,
		FISH_SUCCESS,
		FISH_FAILURE,
		ITEM_DESTROY,
		WENT_ONLINE,
		WENT_OFFLINE,
		LEVEL_GAIN,
		LEVEL_LOSS,
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

	// used in events with no extra data
	struct EmptyEvent {
		std::string noop; // noop, gets discard upstream

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(noop)
			);
		}
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

	struct AAGainedEvent {
		uint32 aa_gained;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(CEREAL_NVP(aa_gained));
		}
	};

	struct AAPurchasedEvent {
		int32 aa_id;
		int32 aa_cost;
		int32 aa_previous_id;
		int32 aa_next_id;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(aa_id),
				CEREAL_NVP(aa_cost),
				CEREAL_NVP(aa_previous_id),
				CEREAL_NVP(aa_next_id)
			);
		}
	};

	struct ForageSuccessEvent {
		uint32      item_id;
		std::string item_name;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name)
			);
		}
	};

	struct FishSuccessEvent {
		uint32      item_id;
		std::string item_name;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name)
			);
		}
	};

	struct DestroyItemEvent {
		uint32      item_id;
		std::string item_name;
		int16       charges;
		std::string reason;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(reason),
				CEREAL_NVP(charges)
			);
		}
	};

	struct LevelGainedEvent {
		int levels_gained;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(CEREAL_NVP(levels_gained));
		}
	};

	struct LevelLostEvent {
		int levels_lost;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(CEREAL_NVP(levels_lost));
		}
	};
}

#endif //EQEMU_PLAYER_EVENTS_H

#define RecordPlayerEventLog(event_type, event_data) do {\
    if (player_event_logs.IsEventEnabled(event_type)) {\
        worldserver.SendPacket(\
            player_event_logs.RecordEvent(\
                event_type,\
                GetPlayerEvent(),\
                event_data\
            ).get()\
        );\
    }\
} while (0)
