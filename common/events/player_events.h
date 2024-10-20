#ifndef EQEMU_PLAYER_EVENTS_H
#define EQEMU_PLAYER_EVENTS_H

#include <string>
#include <cereal/cereal.hpp>
#include "../types.h"
#include "../repositories/player_event_logs_repository.h"

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
		NPC_HANDIN,
		SKILL_UP,
		TASK_ACCEPT,
		TASK_UPDATE,
		TASK_COMPLETE,
		TRADE,
		GIVE_ITEM, // unimplemented
		SAY,
		REZ_ACCEPTED,
		DEATH,
		COMBINE_FAILURE,
		COMBINE_SUCCESS,
		DROPPED_ITEM,
		SPLIT_MONEY,
		DZ_JOIN, // unimplemented
		DZ_LEAVE, // unimplemented
		TRADER_PURCHASE,
		TRADER_SELL,
		BANDOLIER_CREATE, // unimplemented
		BANDOLIER_SWAP, // unimplemented
		DISCOVER_ITEM,
		POSSIBLE_HACK,
		KILLED_NPC,
		KILLED_NAMED_NPC,
		KILLED_RAID_NPC,
		ITEM_CREATION,
		GUILD_TRIBUTE_DONATE_ITEM,
		GUILD_TRIBUTE_DONATE_PLAT,
		PARCEL_SEND,
		PARCEL_RETRIEVE,
		PARCEL_DELETE,
		BARTER_TRANSACTION,
		EVOLVE_ITEM,
		MAX // dont remove
	};

	// Don't ever remove items, even if they are deprecated
	// If event is deprecated just tag (Deprecated) in the name
	// If event is unimplemented just tag (Unimplemented) in the name
	// Events don't get saved to the database if unimplemented or deprecated
	// Events tagged as deprecated will get automatically removed
	static const char *EventName[EventType::MAX] = {
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
		"Group Join (Unimplemented)",
		"Group Leave (Unimplemented)",
		"Raid Join (Unimplemented)",
		"Raid Leave (Unimplemented)",
		"Groundspawn Pickup",
		"NPC Handin",
		"Skill Up",
		"Task Accept",
		"Task Update",
		"Task Complete",
		"Trade",
		"Given Item (Unimplemented)",
		"Say",
		"Rez Accepted",
		"Death",
		"Combine Failure",
		"Combine Success",
		"Dropped Item",
		"Split Money",
		"DZ Join (Unimplemented)",
		"DZ Leave (Unimplemented)",
		"Trader Purchase",
		"Trader Sell",
		"Bandolier Create (Unimplemented)",
		"Bandolier Swap (Unimplemented)",
		"Discover Item",
		"Possible Hack",
		"Killed NPC",
		"Killed Named NPC",
		"Killed Raid NPC",
		"Item Creation",
		"Guild Tribute Donate Item",
		"Guild Tribute Donate Platinum",
		"Parcel Item Sent",
		"Parcel Item Retrieved",
		"Parcel Prune Routine",
		"Barter Transaction",
		"Evolve Item Update"
	};

	// Generic struct used by all events
	struct PlayerEvent {
		int64       account_id;
		std::string account_name;
		int64       character_id;
		std::string character_name;
		int64       guild_id;
		std::string guild_name;
		int         zone_id;
		std::string zone_short_name;
		std::string zone_long_name;
		int         instance_id;
		float       x;
		float       y;
		float       z;
		float       heading;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(account_id),
				CEREAL_NVP(account_name),
				CEREAL_NVP(character_id),
				CEREAL_NVP(character_name),
				CEREAL_NVP(guild_id),
				CEREAL_NVP(guild_name),
				CEREAL_NVP(zone_id),
				CEREAL_NVP(zone_short_name),
				CEREAL_NVP(zone_long_name),
				CEREAL_NVP(instance_id),
				CEREAL_NVP(x),
				CEREAL_NVP(y),
				CEREAL_NVP(z),
				CEREAL_NVP(heading)
			);
		}
	};

	// contains metadata in use for things like log/discord formatters
	// along with the actual event to be persisted
	struct PlayerEventContainer {
		PlayerEvent                                player_event;
		PlayerEventLogsRepository::PlayerEventLogs player_event_log;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(player_event),
				CEREAL_NVP(player_event_log)
			);
		}
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
	struct ItemCreationEvent {
		int64       item_id;
		std::string item_name;
		uint16      to_slot;
		int16       charges;
		uint32      aug1;
		uint32      aug2;
		uint32      aug3;
		uint32      aug4;
		uint32      aug5;
		uint32      aug6;
		bool        attuned;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(to_slot),
				CEREAL_NVP(charges),
				CEREAL_NVP(aug1),
				CEREAL_NVP(aug2),
				CEREAL_NVP(aug3),
				CEREAL_NVP(aug4),
				CEREAL_NVP(aug5),
				CEREAL_NVP(aug6),
				CEREAL_NVP(attuned)
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
	class TradeItemEntry {
	public:
		uint16      slot;
		uint32      item_id;
		std::string item_name;
		uint16      charges;
		uint32      aug_1_item_id;
		std::string aug_1_item_name;
		uint32      aug_2_item_id;
		std::string aug_2_item_name;
		uint32      aug_3_item_id;
		std::string aug_3_item_name;
		uint32      aug_4_item_id;
		std::string aug_4_item_name;
		uint32      aug_5_item_id;
		std::string aug_5_item_name;
		uint32      aug_6_item_id;
		std::string aug_6_item_name;
		bool        in_bag;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(slot),
				CEREAL_NVP(item_id),
				CEREAL_NVP(charges),
				CEREAL_NVP(aug_1_item_id),
				CEREAL_NVP(aug_2_item_id),
				CEREAL_NVP(aug_3_item_id),
				CEREAL_NVP(aug_4_item_id),
				CEREAL_NVP(aug_5_item_id),
				CEREAL_NVP(in_bag)
			);
		}
	};

	/**
	 * Events
	 */
	struct Money {
		int32 platinum;
		int32 gold;
		int32 silver;
		int32 copper;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(platinum),
				CEREAL_NVP(gold),
				CEREAL_NVP(silver),
				CEREAL_NVP(copper)
			);
		}
	};

	struct TradeEvent {
		uint32                      character_1_id;
		std::string                 character_1_name;
		uint32                      character_2_id;
		std::string                 character_2_name;
		Money                       character_1_give_money;
		Money                       character_2_give_money;
		std::vector<TradeItemEntry> character_1_give_items;
		std::vector<TradeItemEntry> character_2_give_items;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(character_1_id),
				CEREAL_NVP(character_1_name),
				CEREAL_NVP(character_2_id),
				CEREAL_NVP(character_2_name),
				CEREAL_NVP(character_1_give_money),
				CEREAL_NVP(character_2_give_money),
				CEREAL_NVP(character_1_give_items),
				CEREAL_NVP(character_2_give_items)
			);
		}
	};

	struct GMCommandEvent {
		std::string message;
		std::string target;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(message),
				CEREAL_NVP(target)
			);
		}
	};

	struct ZoningEvent {
		std::string from_zone_long_name;
		std::string from_zone_short_name;
		int32       from_zone_id;
		int32       from_instance_id;
		int32       from_instance_version;
		std::string to_zone_long_name;
		std::string to_zone_short_name;
		int32       to_zone_id;
		int32       to_instance_id;
		int32       to_instance_version;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(from_zone_long_name),
				CEREAL_NVP(from_zone_short_name),
				CEREAL_NVP(from_zone_id),
				CEREAL_NVP(from_instance_id),
				CEREAL_NVP(from_instance_version),
				CEREAL_NVP(to_zone_long_name),
				CEREAL_NVP(to_zone_short_name),
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
		uint32 aa_id;
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
		uint32 from_level;
		uint8  to_level;
		int    levels_gained;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(from_level),
				CEREAL_NVP(to_level),
				CEREAL_NVP(levels_gained)
			);
		}
	};

	struct LevelLostEvent {
		uint32 from_level;
		uint8  to_level;
		int    levels_lost;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(from_level),
				CEREAL_NVP(to_level),
				CEREAL_NVP(levels_lost)
			);
		}
	};

	struct LootItemEvent {
		uint32      item_id;
		std::string item_name;
		int16       charges;
		uint32      npc_id;
		std::string corpse_name;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(charges),
				CEREAL_NVP(npc_id),
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
		std::string target;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(message),
				CEREAL_NVP(target)
			);
		}
	};

	struct ResurrectAcceptEvent {
		std::string resurrecter_name;
		std::string spell_name;
		uint32      spell_id;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(resurrecter_name),
				CEREAL_NVP(spell_name),
				CEREAL_NVP(spell_id)
			);
		}
	};

	struct CombineEvent {
		uint32      recipe_id;
		std::string recipe_name;
		uint32      made_count;
		uint32      tradeskill_id;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(recipe_id),
				CEREAL_NVP(recipe_name),
				CEREAL_NVP(made_count),
				CEREAL_NVP(tradeskill_id)
			);
		}
	};

	struct DroppedItemEvent {
		uint32      item_id;
		std::string item_name;
		int16       slot_id;
		uint32      charges;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(slot_id),
				CEREAL_NVP(charges)
			);
		}
	};

	struct DeathEvent {
		uint32      killer_id;
		std::string killer_name;
		int64       damage;
		uint32      spell_id;
		std::string spell_name;
		int         skill_id;
		std::string skill_name;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(killer_id),
				CEREAL_NVP(killer_name),
				CEREAL_NVP(damage),
				CEREAL_NVP(spell_id),
				CEREAL_NVP(spell_name),
				CEREAL_NVP(skill_id),
				CEREAL_NVP(skill_name)
			);
		}
	};

	struct SplitMoneyEvent {
		uint32 copper;
		uint32 silver;
		uint32 gold;
		uint32 platinum;
		uint64 player_money_balance;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(copper),
				CEREAL_NVP(silver),
				CEREAL_NVP(gold),
				CEREAL_NVP(platinum),
				CEREAL_NVP(player_money_balance)
			);
		}
	};

	struct TraderPurchaseEvent {
		uint32      item_id;
		std::string item_name;
		uint32      trader_id;
		std::string trader_name;
		uint32      price;
		uint32      charges;
		uint32      total_cost;
		uint64      player_money_balance;


		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(trader_id),
				CEREAL_NVP(trader_name),
				CEREAL_NVP(price),
				CEREAL_NVP(charges),
				CEREAL_NVP(total_cost),
				CEREAL_NVP(player_money_balance)
			);
		}
	};

	struct TraderSellEvent {
		uint32      item_id;
		std::string item_name;
		uint32      buyer_id;
		std::string buyer_name;
		uint32      price;
		uint32      charges;
		uint32      total_cost;
		uint64      player_money_balance;


		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(buyer_id),
				CEREAL_NVP(buyer_name),
				CEREAL_NVP(price),
				CEREAL_NVP(charges),
				CEREAL_NVP(total_cost),
				CEREAL_NVP(player_money_balance)
			);
		}
	};

	struct DiscoverItemEvent {
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

	class HandinEntry {
	public:
		uint32                   item_id;
		std::string              item_name;
		std::vector<uint32>      augment_ids;
		std::vector<std::string> augment_names;
		uint16                   charges;
		bool                     attuned;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(augment_ids),
				CEREAL_NVP(augment_names),
				CEREAL_NVP(charges),
				CEREAL_NVP(attuned)
			);
		}
	};

	class HandinMoney {
	public:
		uint32 copper;
		uint32 silver;
		uint32 gold;
		uint32 platinum;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(copper),
				CEREAL_NVP(silver),
				CEREAL_NVP(gold),
				CEREAL_NVP(platinum)
			);
		}
	};

	struct HandinEvent {
		uint32                   npc_id;
		std::string              npc_name;
		std::vector<HandinEntry> handin_items;
		HandinMoney              handin_money;
		std::vector<HandinEntry> return_items;
		HandinMoney              return_money;
		bool                     is_quest_handin;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(npc_id),
				CEREAL_NVP(npc_name),
				CEREAL_NVP(handin_items),
				CEREAL_NVP(handin_money),
				CEREAL_NVP(return_items),
				CEREAL_NVP(return_money),
				CEREAL_NVP(is_quest_handin)
			);
		}
	};

	struct PossibleHackEvent {
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

	struct KilledNPCEvent {
		uint32      npc_id;
		std::string npc_name;
		uint32      combat_time_seconds;
		uint64      total_damage_per_second_taken;
		uint64      total_heal_per_second_taken;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(npc_id),
				CEREAL_NVP(npc_name),
				CEREAL_NVP(combat_time_seconds),
				CEREAL_NVP(total_damage_per_second_taken),
				CEREAL_NVP(total_heal_per_second_taken)
			);
		}
	};

	struct GuildTributeDonateItem {
		uint32      item_id;
		uint32 		guild_favor;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(guild_favor)
			);
		}
	};

	struct GuildTributeDonatePlat {
		uint32      plat;
		uint32 		guild_favor;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(plat),
				CEREAL_NVP(guild_favor)
			);
		}
	};

	struct ParcelRetrieve {
		uint32      item_id;
		uint32      quantity;
		std::string from_player_name;
		uint32      sent_date;
		uint32      aug_slot_1;
		uint32      aug_slot_2;
		uint32      aug_slot_3;
		uint32      aug_slot_4;
		uint32      aug_slot_5;
		uint32      aug_slot_6;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(quantity),
				CEREAL_NVP(from_player_name),
				CEREAL_NVP(sent_date),
				CEREAL_NVP(aug_slot_1),
				CEREAL_NVP(aug_slot_2),
				CEREAL_NVP(aug_slot_3),
				CEREAL_NVP(aug_slot_4),
				CEREAL_NVP(aug_slot_5),
				CEREAL_NVP(aug_slot_6)
			);
		}
	};

	struct ParcelSend {
		uint32      item_id;
		uint32      quantity;
		std::string from_player_name;
		std::string to_player_name;
		uint32      sent_date;
		uint32      aug_slot_1;
		uint32      aug_slot_2;
		uint32      aug_slot_3;
		uint32      aug_slot_4;
		uint32      aug_slot_5;
		uint32      aug_slot_6;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(quantity),
				CEREAL_NVP(from_player_name),
				CEREAL_NVP(to_player_name),
				CEREAL_NVP(sent_date),
				CEREAL_NVP(aug_slot_1),
				CEREAL_NVP(aug_slot_2),
				CEREAL_NVP(aug_slot_3),
				CEREAL_NVP(aug_slot_4),
				CEREAL_NVP(aug_slot_5),
				CEREAL_NVP(aug_slot_6)
			);
		}
	};

	struct ParcelDelete {
		uint32      item_id;
		uint32      quantity;
		uint32      char_id;
		std::string from_name;
		std::string note;
		uint32      sent_date;
		uint32      aug_slot_1;
		uint32      aug_slot_2;
		uint32      aug_slot_3;
		uint32      aug_slot_4;
		uint32      aug_slot_5;
		uint32      aug_slot_6;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(item_id),
				CEREAL_NVP(quantity),
				CEREAL_NVP(char_id),
				CEREAL_NVP(from_name),
				CEREAL_NVP(note),
				CEREAL_NVP(sent_date),
				CEREAL_NVP(aug_slot_1),
				CEREAL_NVP(aug_slot_2),
				CEREAL_NVP(aug_slot_3),
				CEREAL_NVP(aug_slot_4),
				CEREAL_NVP(aug_slot_5),
				CEREAL_NVP(aug_slot_6)
				);
		}
	};

	struct BarterTransaction {
		std::string                             status;
		uint32                                  item_id;
		uint32                                  item_quantity;
		std::string                             item_name;
		std::vector<BuyerLineTradeItems_Struct> trade_items;
		std::string                             buyer_name;
		std::string                             seller_name;
		uint64                                  total_cost;
		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(status),
				CEREAL_NVP(item_id),
				CEREAL_NVP(item_quantity),
				CEREAL_NVP(item_name),
				CEREAL_NVP(trade_items),
				CEREAL_NVP(buyer_name),
				CEREAL_NVP(seller_name),
				CEREAL_NVP(total_cost)
			);
		}
	};

	struct EvolveItem {
		std::string status;
		uint32      item_id;
		uint64      unique_id;
		std::string item_name;
		uint32      level;
		double      progression;

		// cereal
		template<class Archive>
		void serialize(Archive &ar)
		{
			ar(
				CEREAL_NVP(status),
				CEREAL_NVP(item_id),
				CEREAL_NVP(unique_id),
				CEREAL_NVP(item_name),
				CEREAL_NVP(level),
				CEREAL_NVP(progression)
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
                (c)->GetPlayerEvent(),\
                event_data\
            ).get()\
        );\
    }\
} while (0)
