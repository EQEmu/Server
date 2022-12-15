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
		LOOT_ITEM,
		MERCHANT_PURCHASE,
		MERCHANT_SELL,
		GROUP_JOIN, // unimplemented
		GROUP_LEAVE, // unimplemented
		RAID_JOIN, // unimplemented
		RAID_LEAVE, // unimplemented
		GROUNDSPAWN_PICKUP,
		NPC_HANDIN, // unimplemented
		SKILL_UP,
		TASK_ACCEPT,
		TASK_UPDATE,
		TASK_COMPLETE,
		TRADE, // unimplemented
		GIVE_ITEM, // unimplemented
		SAY,
		REZ_ACCEPTED,
		DEATH, // unimplemented
		COMBINE_FAILURE,
		COMBINE_SUCCESS,
		DROPPED_ITEM,
		SPLIT_MONEY, // unimplemented
		DZ_JOIN, // unimplemented
		DZ_LEAVE, // unimplemented
		TRADER_PURCHASE, // unimplemented
		TRADER_SELL, // unimplemented
		BANDOLIER_CREATE, // unimplemented
		BANDOLIER_SWAP, // unimplemented
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
		"Dropped Item",
		"Split Money",
		"DZ Join",
		"DZ Leave",
		"Trader Purchase",
		"Trader Sell",
		"Bandolier Create",
		"Bandolier Swap",
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

	struct LootItemEvent {
		uint32      item_id;
		std::string item_name;
		int16       charges;
		std::string corpse_name;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(charges),
				CEREAL_NVP(corpse_name)
			);
		}
	};

	struct MerchantPurchaseEvent {
		uint32      npc_id;
		std::string merchant_name;
		uint32      merchant_type;
		uint32      item_id;
		std::string item_name;
		int16       charges;
		uint32      cost;
		uint32      alternate_currency_id;
		uint64      player_money_balance;
		uint64      player_currency_balance;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(npc_id),
				CEREAL_NVP(merchant_name),
				CEREAL_NVP(merchant_type),
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(charges),
				CEREAL_NVP(cost),
				CEREAL_NVP(alternate_currency_id),
				CEREAL_NVP(player_money_balance),
				CEREAL_NVP(player_currency_balance)
			);
		}
	};

	struct MerchantSellEvent {
		uint32      npc_id;
		std::string merchant_name;
		uint32      merchant_type;
		uint32      item_id;
		std::string item_name;
		int16       charges;
		uint32      cost;
		uint32      alternate_currency_id;
		uint64      player_money_balance;
		uint64      player_currency_balance;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(npc_id),
				CEREAL_NVP(merchant_name),
				CEREAL_NVP(merchant_type),
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(charges),
				CEREAL_NVP(cost),
				CEREAL_NVP(alternate_currency_id),
				CEREAL_NVP(player_money_balance),
				CEREAL_NVP(player_currency_balance)
			);
		}
	};

	struct SkillUpEvent {
		uint32      skill_id;
		int         value;
		int16       max_skill;
		std::string against_who;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(skill_id),
				CEREAL_NVP(value),
				CEREAL_NVP(max_skill),
				CEREAL_NVP(against_who)
			);
		}
	};

	struct TaskAcceptEvent {
		uint32      npc_id;
		std::string npc_name;
		uint32      task_id;
		std::string task_name;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(npc_id),
				CEREAL_NVP(npc_name),
				CEREAL_NVP(task_id),
				CEREAL_NVP(task_name)
			);
		}
	};

	struct TaskUpdateEvent {
		uint32      task_id;
		std::string task_name;
		uint32      activity_id;
		uint32      done_count;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(task_id),
				CEREAL_NVP(task_name),
				CEREAL_NVP(activity_id),
				CEREAL_NVP(done_count)
			);
		}
	};

	struct TaskCompleteEvent {
		uint32      task_id;
		std::string task_name;
		uint32      activity_id;
		uint32      done_count;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(task_id),
				CEREAL_NVP(task_name),
				CEREAL_NVP(activity_id),
				CEREAL_NVP(done_count)
			);
		}
	};

	struct GroundSpawnPickupEvent {
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

	struct SayEvent {
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

	struct ResurrectAcceptEvent {
		std::string resurrecter_name;
		uint32      spell_id;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(resurrecter_name)
			);
		}
	};

	struct CombineEvent {
		uint32      recipe_id;
		std::string recipe_name;
		uint32      made_count;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(recipe_id),
				CEREAL_NVP(recipe_name),
				CEREAL_NVP(made_count)
			);
		}
	};

	struct DroppedItemEvent {
		uint32      item_id;
		std::string item_name;
		int16       slot_id;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(slot_id)
			);
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

#define RecordPlayerEventLogWithClient(c, event_type, event_data) do {\
    if (player_event_logs.IsEventEnabled(event_type)) {\
        worldserver.SendPacket(\
            player_event_logs.RecordEvent(\
                event_type,\
                c->GetPlayerEvent(),\
                event_data\
            ).get()\
        );\
    }\
} while (0)
