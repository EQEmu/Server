#ifndef EQEMU_PLAYER_EVENTS_H
#define EQEMU_PLAYER_EVENTS_H

#include <string>
#include <cereal/cereal.hpp>
#include "../types.h"

namespace PlayerEvent {

	enum Event {
		GM_COMMAND = 1,
		ZONING,
		MAX // dont remove
	};

	// tied to Event
	static const char *EventName[PlayerEvent::MAX] = {
		"None",
		"GM Command",
		"Zoning",
	};

	/**
	 * Generic struct used by all events
	 */
	class PlayerEvent {
	public:
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
	class TradeItem {
	public:
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
	class TradeEvent {
	public:
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

	class GMCommandEvent {
	public:
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

	class ZoningEvent {
	public:
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
