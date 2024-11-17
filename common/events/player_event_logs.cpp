#include "player_event_logs.h"
#include <cereal/archives/json.hpp>

#include "../platform.h"
#include "../rulesys.h"
#include "player_event_discord_formatter.h"
#include "../repositories/player_event_loot_items_repository.h"
#include "../repositories/player_event_merchant_sell_repository.h"
#include "../repositories/player_event_merchant_purchase_repository.h"
#include "../repositories/player_event_npc_handin_repository.h"
#include "../repositories/player_event_npc_handin_entries_repository.h"

const uint32 PROCESS_RETENTION_TRUNCATION_TIMER_INTERVAL = 60 * 60 * 1000; // 1 hour

// general initialization routine
void PlayerEventLogs::Init()
{

	m_process_batch_events_timer.SetTimer(RuleI(Logging, BatchPlayerEventProcessIntervalSeconds) * 1000);
	m_process_retention_truncation_timer.SetTimer(PROCESS_RETENTION_TRUNCATION_TIMER_INTERVAL);

	ValidateDatabaseConnection();

	// initialize settings array
	for (int i = PlayerEvent::GM_COMMAND; i != PlayerEvent::MAX; i++) {
		m_settings[i].id                 = i;
		m_settings[i].event_name         = PlayerEvent::EventName[i];
		m_settings[i].event_enabled      = 1;
		m_settings[i].retention_days     = 0;
		m_settings[i].discord_webhook_id = 0;
		m_settings[i].etl_enabled        = false;
	}

	SetSettingsDefaults();

	// initialize settings from database
	auto             s = PlayerEventLogSettingsRepository::All(*m_database);
	std::vector<int> db{};
	db.reserve(s.size());
	for (auto &e: s) {
		if (e.id >= PlayerEvent::MAX) {
			continue;
		}
		m_settings[e.id] = e;
		db.emplace_back(e.id);
	}

	std::vector<PlayerEventLogSettingsRepository::PlayerEventLogSettings> settings_to_insert{};

	// insert entries that don't exist in database
	for (int i = PlayerEvent::GM_COMMAND; i != PlayerEvent::MAX; i++) {
		bool is_in_database = std::find(db.begin(), db.end(), i) != db.end();
		bool is_deprecated  = Strings::Contains(PlayerEvent::EventName[i], "Deprecated");
		bool is_implemented = !Strings::Contains(PlayerEvent::EventName[i], "Unimplemented");

		// remove when deprecated
		if (is_deprecated && is_in_database) {
			LogInfo("[Deprecated] Removing PlayerEvent [{}] ({})", PlayerEvent::EventName[i], i);
			PlayerEventLogSettingsRepository::DeleteWhere(*m_database, fmt::format("id = {}", i));
		}
		// remove when unimplemented if present
		if (!is_implemented && is_in_database) {
			LogInfo("[Unimplemented] Removing PlayerEvent [{}] ({})", PlayerEvent::EventName[i], i);
			PlayerEventLogSettingsRepository::DeleteWhere(*m_database, fmt::format("id = {}", i));
		}

		bool is_missing_in_database = std::find(db.begin(), db.end(), i) == db.end();
		if (is_missing_in_database && is_implemented && !is_deprecated) {
			LogInfo("[New] PlayerEvent [{}] ({})", PlayerEvent::EventName[i], i);

			auto c           = PlayerEventLogSettingsRepository::NewEntity();
			c.id             = i;
			c.event_name     = PlayerEvent::EventName[i];
			c.event_enabled  = m_settings[i].event_enabled;
			c.retention_days = m_settings[i].retention_days;
			c.etl_enabled    = false;
			settings_to_insert.emplace_back(c);
		}
	}

	if (!settings_to_insert.empty()) {
		PlayerEventLogSettingsRepository::ReplaceMany(*m_database, settings_to_insert);
	}

	LoadEtlIds();

	bool processing_in_world = !RuleB(Logging, PlayerEventsQSProcess) && IsWorld();
	bool processing_in_qs    = RuleB(Logging, PlayerEventsQSProcess) && IsQueryServ();

	// on initial boot process truncation
	if (processing_in_world || processing_in_qs) {
		ProcessRetentionTruncation();
	}
}

// set the database object, during initialization
PlayerEventLogs *PlayerEventLogs::SetDatabase(Database *db)
{
	m_database = db;

	return this;
}

// validates whether the connection is valid or not, used in initialization
bool PlayerEventLogs::ValidateDatabaseConnection()
{
	if (!m_database) {
		LogError("No database connection");
		return false;
	}

	return true;
}

// determines if the passed in event is enabled or not
// this is used to gate logic or events from firing off
// this is used prior to building the events, we don't want to
// build the events, send them through the stack in a function call
// only to discard them immediately afterwards, very wasteful on resources
// the quest api currently does this
bool PlayerEventLogs::IsEventEnabled(PlayerEvent::EventType event)
{
	return m_settings[event].event_enabled ? m_settings[event].event_enabled : false;
}

// this processes any current player events on the queue
void PlayerEventLogs::ProcessBatchQueue()
{
	m_batch_queue_lock.lock();
	if (m_record_batch_queue.empty()) {
		m_batch_queue_lock.unlock();
		return;
	}

	std::map<uint32, uint32> counter{};
	for (auto const& e: m_record_batch_queue) {
		counter[e.event_type_id]++;
	}


	BenchTimer benchmark;

	EtlQueues etl_queues{};
	etl_queues.trade.reserve(counter[PlayerEvent::TRADE] ? counter[PlayerEvent::TRADE] : 0);
	etl_queues.speech.reserve(counter[PlayerEvent::SPEECH] ? counter[PlayerEvent::SPEECH] : 0);
	etl_queues.loot_items.reserve(counter[PlayerEvent::LOOT_ITEM] ? counter[PlayerEvent::LOOT_ITEM] : 0);
	etl_queues.killed_npc.reserve(counter[PlayerEvent::KILLED_NPC] ? counter[PlayerEvent::KILLED_NPC] : 0);
	etl_queues.npc_handin.reserve(counter[PlayerEvent::NPC_HANDIN] ? counter[PlayerEvent::NPC_HANDIN] : 0);
	etl_queues.aa_purchase.reserve(counter[PlayerEvent::AA_PURCHASE] ? counter[PlayerEvent::AA_PURCHASE] : 0);
	etl_queues.merchant_sell.reserve(counter[PlayerEvent::MERCHANT_SELL] ? counter[PlayerEvent::MERCHANT_SELL] : 0);
	etl_queues.killed_raid_npc.reserve(counter[PlayerEvent::KILLED_RAID_NPC] ? counter[PlayerEvent::KILLED_RAID_NPC] : 0);
	etl_queues.killed_named_npc.reserve(counter[PlayerEvent::KILLED_NAMED_NPC] ? counter[PlayerEvent::KILLED_NAMED_NPC] : 0);
	etl_queues.merchant_purchase.reserve(counter[PlayerEvent::MERCHANT_PURCHASE] ? counter[PlayerEvent::MERCHANT_PURCHASE] : 0);

	for (auto &r:m_record_batch_queue) {
		if (m_settings[r.event_type_id].etl_enabled) {
			switch (r.event_type_id) {
				case PlayerEvent::EventType::LOOT_ITEM: {
					PlayerEvent::LootItemEvent in{};
					PlayerEventLootItemsRepository::PlayerEventLootItems out{};

					{
						std::stringstream ss;
						ss << r.event_data;
						cereal::JSONInputArchive ar(ss);
						in.serialize(ar);
					}

					out.charges     = in.charges;
					out.corpse_name = in.corpse_name;
					out.item_id     = in.item_id;
					out.item_name   = in.item_name;
					out.npc_id      = in.npc_id;
					out.created_at  = r.created_at;

					if (m_etl_settings.contains(PlayerEvent::EventType::LOOT_ITEM)) {
						r.etl_table_id = m_etl_settings.at(PlayerEvent::EventType::LOOT_ITEM).next_id;
						m_etl_settings.at(PlayerEvent::EventType::LOOT_ITEM).next_id++;
					}

					etl_queues.loot_items.push_back(out);
					break;
				}
				case PlayerEvent::EventType::MERCHANT_SELL: {
					PlayerEvent::MerchantSellEvent in{};
					PlayerEventMerchantSellRepository::PlayerEventMerchantSell out{};

					{
						std::stringstream ss;
						ss << r.event_data;
						cereal::JSONInputArchive ar(ss);
						in.serialize(ar);
					}

					out.npc_id                  = in.npc_id;
					out.merchant_name           = in.merchant_name;
					out.merchant_type           = in.merchant_type;
					out.item_id                 = in.item_id;
					out.item_name               = in.item_name;
					out.charges                 = in.charges;
					out.cost                    = in.cost;
					out.alternate_currency_id   = in.alternate_currency_id;
					out.player_money_balance    = in.player_money_balance;
					out.player_currency_balance = in.player_currency_balance;
					out.created_at              = r.created_at;

					if (m_etl_settings.contains(PlayerEvent::EventType::MERCHANT_SELL)) {
						r.etl_table_id = m_etl_settings.at(PlayerEvent::EventType::MERCHANT_SELL).next_id;
						m_etl_settings.at(PlayerEvent::EventType::MERCHANT_SELL).next_id++;
					}

					etl_queues.merchant_sell.push_back(out);
					break;
				}
				case PlayerEvent::EventType::MERCHANT_PURCHASE: {
					PlayerEvent::MerchantPurchaseEvent in{};
					PlayerEventMerchantPurchaseRepository::PlayerEventMerchantPurchase out{};

					{
						std::stringstream ss;
						ss << r.event_data;
						cereal::JSONInputArchive ar(ss);
						in.serialize(ar);
					}

					out.npc_id                  = in.npc_id;
					out.merchant_name           = in.merchant_name;
					out.merchant_type           = in.merchant_type;
					out.item_id                 = in.item_id;
					out.item_name               = in.item_name;
					out.charges                 = in.charges;
					out.cost                    = in.cost;
					out.alternate_currency_id   = in.alternate_currency_id;
					out.player_money_balance    = in.player_money_balance;
					out.player_currency_balance = in.player_currency_balance;
					out.created_at              = r.created_at;

					if (m_etl_settings.contains(PlayerEvent::EventType::MERCHANT_PURCHASE)) {
						r.etl_table_id = m_etl_settings.at(PlayerEvent::EventType::MERCHANT_PURCHASE).next_id;
						m_etl_settings.at(PlayerEvent::EventType::MERCHANT_PURCHASE).next_id++;
					}

					etl_queues.merchant_purchase.push_back(out);
					break;
				}
				case PlayerEvent::EventType::NPC_HANDIN: {
					PlayerEvent::HandinEvent in{};
					PlayerEventNpcHandinRepository::PlayerEventNpcHandin out{};
					PlayerEventNpcHandinEntriesRepository::PlayerEventNpcHandinEntries out_entries{};

					{
						std::stringstream ss;
						ss << r.event_data;
						cereal::JSONInputArchive ar(ss);
						in.serialize(ar);
					}


					out.npc_id          = in.npc_id;
					out.npc_name        = in.npc_name;
					out.handin_copper   = in.handin_money.copper;
					out.handin_silver   = in.handin_money.silver;
					out.handin_gold     = in.handin_money.gold;
					out.handin_platinum = in.handin_money.platinum;
					out.return_copper   = in.return_money.copper;
					out.return_silver   = in.return_money.silver;
					out.return_gold     = in.return_money.gold;
					out.return_platinum = in.return_money.platinum;
					out.is_quest_handin = in.is_quest_handin;
					out.created_at      = r.created_at;

					if (m_etl_settings.contains(PlayerEvent::EventType::NPC_HANDIN)) {
						r.etl_table_id = m_etl_settings.at(PlayerEvent::EventType::NPC_HANDIN).next_id;
						m_etl_settings.at(PlayerEvent::EventType::NPC_HANDIN).next_id++;
					}

					if (!in.handin_items.empty()) {
						etl_queues.npc_handin_entries.reserve(etl_queues.npc_handin_entries.size() + in.handin_items.size());
						for (auto const &i: in.handin_items) {
							out_entries.charges                    = i.charges;
							out_entries.evolve_amount              = 0;
							out_entries.evolve_level               = 0;
							out_entries.item_id                    = i.item_id;
							out_entries.player_event_npc_handin_id = r.etl_table_id;
							out_entries.type                       = 1;
							out.created_at                         = r.created_at;

							if (!i.augment_ids.empty()) {
								uint32 augments[6]{};
								for (int x = 0; x < i.augment_ids.size(); x++) {
									augments[x] = i.augment_ids[x];
								}
								out_entries.augment_1_id = augments[0];
								out_entries.augment_2_id = augments[1];
								out_entries.augment_3_id = augments[2];
								out_entries.augment_4_id = augments[3];
								out_entries.augment_5_id = augments[4];
								out_entries.augment_6_id = augments[5];
							}
							etl_queues.npc_handin_entries.push_back(out_entries);
						}
					}

					if (!in.return_items.empty()) {
						etl_queues.npc_handin_entries.reserve(etl_queues.npc_handin_entries.size() + in.return_items.size());
						for (auto const &i: in.handin_items) {
							out_entries.charges                    = i.charges;
							out_entries.evolve_amount              = 0;
							out_entries.evolve_level               = 0;
							out_entries.item_id                    = i.item_id;
							out_entries.player_event_npc_handin_id = r.etl_table_id;
							out_entries.type                       = 2;
							out.created_at                         = r.created_at;

							if (!i.augment_ids.empty()) {
								uint32 augments[6]{};
								for (int x = 0; x < i.augment_ids.size(); x++) {
									augments[x] = i.augment_ids[x];
								}
								out_entries.augment_1_id = augments[0];
								out_entries.augment_2_id = augments[1];
								out_entries.augment_3_id = augments[2];
								out_entries.augment_4_id = augments[3];
								out_entries.augment_5_id = augments[4];
								out_entries.augment_6_id = augments[5];
							}
							etl_queues.npc_handin_entries.push_back(out_entries);
						}
					}

					etl_queues.npc_handin.push_back(out);
					break;
				}
				case PlayerEvent::EventType::TRADE: {
					PlayerEvent::TradeEvent in{};
					PlayerEventTradeRepository::PlayerEventTrade out{};
					PlayerEventTradeEntriesRepository::PlayerEventTradeEntries out_entries{};

					{
						std::stringstream ss;
						ss << r.event_data;
						cereal::JSONInputArchive ar(ss);
						in.serialize(ar);
					}

					out.char1_id       = in.character_1_id;
					out.char1_platinum = in.character_1_give_money.platinum;
					out.char1_gold     = in.character_1_give_money.gold;
					out.char1_silver   = in.character_1_give_money.silver;
					out.char1_copper   = in.character_1_give_money.copper;
					out.char2_id       = in.character_2_id;
					out.char2_platinum = in.character_2_give_money.platinum;
					out.char2_gold     = in.character_2_give_money.gold;
					out.char2_silver   = in.character_2_give_money.silver;
					out.char2_copper   = in.character_2_give_money.copper;
					out.created_at     = r.created_at;

					if (m_etl_settings.contains(PlayerEvent::EventType::TRADE)) {
						r.etl_table_id = m_etl_settings.at(PlayerEvent::EventType::TRADE).next_id;
						m_etl_settings.at(PlayerEvent::EventType::TRADE).next_id++;
					}

					if (!in.character_1_give_items.empty()) {
						etl_queues.trade_entries.reserve(etl_queues.trade_entries.size() + in.character_1_give_items.size());
						for (auto const &i: in.character_1_give_items) {
							out_entries.char_id               = in.character_1_id;
							out_entries.charges               = i.charges;
							out_entries.slot                  = i.slot;
							out_entries.item_id               = i.item_id;
							out_entries.player_event_trade_id = r.etl_table_id;
							out_entries.in_bag                = i.in_bag;
							out.created_at                    = r.created_at;
							out_entries.augment_1_id          = i.aug_1_item_id;
							out_entries.augment_2_id          = i.aug_2_item_id;
							out_entries.augment_3_id          = i.aug_3_item_id;
							out_entries.augment_4_id          = i.aug_4_item_id;
							out_entries.augment_5_id          = i.aug_5_item_id;
							out_entries.augment_6_id          = i.aug_6_item_id;
							out_entries.created_at            = r.created_at;
						}
						etl_queues.trade_entries.push_back(out_entries);
					}

					if (!in.character_2_give_items.empty()) {
						etl_queues.trade_entries.reserve(etl_queues.trade_entries.size() + in.character_2_give_items.size());
						for (auto const &i: in.character_2_give_items) {
							out_entries.char_id               = in.character_2_id;
							out_entries.charges               = i.charges;
							out_entries.slot                  = i.slot;
							out_entries.item_id               = i.item_id;
							out_entries.player_event_trade_id = r.etl_table_id;
							out_entries.in_bag                = i.in_bag;
							out.created_at                    = r.created_at;
							out_entries.augment_1_id          = i.aug_1_item_id;
							out_entries.augment_2_id          = i.aug_2_item_id;
							out_entries.augment_3_id          = i.aug_3_item_id;
							out_entries.augment_4_id          = i.aug_4_item_id;
							out_entries.augment_5_id          = i.aug_5_item_id;
							out_entries.augment_6_id          = i.aug_6_item_id;
							out_entries.created_at            = r.created_at;
						}
						etl_queues.trade_entries.push_back(out_entries);
					}

					etl_queues.trade.push_back(out);
					break;
				}
				case PlayerEvent::EventType::SPEECH: {
					PlayerEvent::PlayerSpeech in{};
					PlayerEventSpeechRepository::PlayerEventSpeech out{};

					{
						std::stringstream ss;
						ss << r.event_data;
						cereal::JSONInputArchive ar(ss);
						in.serialize(ar);
					}

					out.from_char_id = in.from;
					out.to_char_id   = in.to;
					out.type         = in.type;
					out.min_status   = in.min_status;
					out.message      = in.message;
					out.guild_id     = in.guild_id;
					out.created_at   = r.created_at;

					if (m_etl_settings.contains(PlayerEvent::EventType::SPEECH)) {
						r.etl_table_id = m_etl_settings.at(PlayerEvent::EventType::SPEECH).next_id;
						m_etl_settings.at(PlayerEvent::EventType::SPEECH).next_id++;
					}

					etl_queues.speech.push_back(out);
					break;
				}
				case PlayerEvent::EventType::KILLED_NPC: {
					PlayerEvent::KilledNPCEvent in{};
					PlayerEventKilledNpcRepository::PlayerEventKilledNpc out{};

					{
						std::stringstream ss;
						ss << r.event_data;
						cereal::JSONInputArchive ar(ss);
						in.serialize(ar);
					}

					out.npc_id                        = in.npc_id;
					out.npc_name                      = in.npc_name;
					out.combat_time_seconds           = in.combat_time_seconds;
					out.total_damage_per_second_taken = in.total_damage_per_second_taken;
					out.total_heal_per_second_taken   = in.total_heal_per_second_taken;
					out.created_at                    = r.created_at;

					if (m_etl_settings.contains(PlayerEvent::EventType::KILLED_NPC)) {
						r.etl_table_id = m_etl_settings.at(PlayerEvent::EventType::KILLED_NPC).next_id;
						m_etl_settings.at(PlayerEvent::EventType::KILLED_NPC).next_id++;
					}

					etl_queues.killed_npc.push_back(out);
					break;
				}
				case PlayerEvent::EventType::KILLED_NAMED_NPC: {
					PlayerEvent::KilledNPCEvent in{};
					PlayerEventKilledNamedNpcRepository::PlayerEventKilledNamedNpc out{};

					{
						std::stringstream ss;
						ss << r.event_data;
						cereal::JSONInputArchive ar(ss);
						in.serialize(ar);
					}

					out.npc_id                        = in.npc_id;
					out.npc_name                      = in.npc_name;
					out.combat_time_seconds           = in.combat_time_seconds;
					out.total_damage_per_second_taken = in.total_damage_per_second_taken;
					out.total_heal_per_second_taken   = in.total_heal_per_second_taken;
					out.created_at                    = r.created_at;

					if (m_etl_settings.contains(PlayerEvent::EventType::KILLED_NAMED_NPC)) {
						r.etl_table_id = m_etl_settings.at(PlayerEvent::EventType::KILLED_NAMED_NPC).next_id;
						m_etl_settings.at(PlayerEvent::EventType::KILLED_NAMED_NPC).next_id++;
					}

					etl_queues.killed_named_npc.push_back(out);
					break;
				}
				case PlayerEvent::EventType::KILLED_RAID_NPC: {
					PlayerEvent::KilledNPCEvent in{};
					PlayerEventKilledRaidNpcRepository::PlayerEventKilledRaidNpc out{};

					{
						std::stringstream ss;
						ss << r.event_data;
						cereal::JSONInputArchive ar(ss);
						in.serialize(ar);
					}

					out.npc_id                        = in.npc_id;
					out.npc_name                      = in.npc_name;
					out.combat_time_seconds           = in.combat_time_seconds;
					out.total_damage_per_second_taken = in.total_damage_per_second_taken;
					out.total_heal_per_second_taken   = in.total_heal_per_second_taken;
					out.created_at                    = r.created_at;

					if (m_etl_settings.contains(PlayerEvent::EventType::KILLED_RAID_NPC)) {
						r.etl_table_id = m_etl_settings.at(PlayerEvent::EventType::KILLED_RAID_NPC).next_id;
						m_etl_settings.at(PlayerEvent::EventType::KILLED_RAID_NPC).next_id++;
					}

					etl_queues.killed_raid_npc.push_back(out);
					break;
				}
				case PlayerEvent::EventType::AA_PURCHASE: {
					PlayerEvent::AAPurchasedEvent in{};
					PlayerEventAaPurchaseRepository::PlayerEventAaPurchase out{};

					{
						std::stringstream ss;
						ss << r.event_data;
						cereal::JSONInputArchive ar(ss);
						in.serialize(ar);
					}

					out.aa_ability_id = in.aa_id;
					out.cost          = in.aa_cost;
					out.previous_id   = in.aa_previous_id;
					out.next_id       = in.aa_next_id;
					out.created_at    = r.created_at;

					if (m_etl_settings.contains(PlayerEvent::EventType::AA_PURCHASE)) {
						r.etl_table_id = m_etl_settings.at(PlayerEvent::EventType::AA_PURCHASE).next_id;
						m_etl_settings.at(PlayerEvent::EventType::AA_PURCHASE).next_id++;
					}

					etl_queues.aa_purchase.push_back(out);
					break;
				}
				default: {
					LogError("Non-Implemented ETL routing <red>[{}]", r.event_type_id);
				}
			}
		}
	}

	// flush many
	PlayerEventLogsRepository::InsertMany(*m_database, m_record_batch_queue);

	// flush detailed tables
	if (!etl_queues.loot_items.empty()) {
		PlayerEventLootItemsRepository::InsertMany(*m_database, etl_queues.loot_items);
		etl_queues.loot_items.clear();
	}

	if (!etl_queues.merchant_sell.empty()) {
		PlayerEventMerchantSellRepository::InsertMany(*m_database, etl_queues.merchant_sell);
		etl_queues.merchant_sell.clear();
	}

	if (!etl_queues.merchant_purchase.empty()) {
		PlayerEventMerchantPurchaseRepository::InsertMany(*m_database, etl_queues.merchant_purchase);
		etl_queues.merchant_purchase.clear();
	}

	if (!etl_queues.npc_handin.empty()) {
		PlayerEventNpcHandinRepository::InsertMany(*m_database, etl_queues.npc_handin);
		etl_queues.npc_handin.clear();
		if (!etl_queues.npc_handin_entries.empty()) {
			PlayerEventNpcHandinEntriesRepository::InsertMany(*m_database, etl_queues.npc_handin_entries);
			etl_queues.npc_handin_entries.clear();
		}
	}

	if (!etl_queues.trade.empty()) {
		PlayerEventTradeRepository::InsertMany(*m_database, etl_queues.trade);
		etl_queues.trade.clear();
		if (!etl_queues.trade_entries.empty()) {
			PlayerEventTradeEntriesRepository::InsertMany(*m_database, etl_queues.trade_entries);
			etl_queues.trade_entries.clear();
		}
	}

	if (!etl_queues.speech.empty()) {
		PlayerEventSpeechRepository::InsertMany(*m_database, etl_queues.speech);
		etl_queues.speech.clear();
	}

	if (!etl_queues.killed_npc.empty()) {
		PlayerEventKilledNpcRepository::InsertMany(*m_database, etl_queues.killed_npc);
		etl_queues.killed_npc.clear();
	}

	if (!etl_queues.killed_named_npc.empty()) {
		PlayerEventKilledNamedNpcRepository::InsertMany(*m_database, etl_queues.killed_named_npc);
		etl_queues.killed_named_npc.clear();
	}

	if (!etl_queues.killed_raid_npc.empty()) {
		PlayerEventKilledRaidNpcRepository::InsertMany(*m_database, etl_queues.killed_raid_npc);
		etl_queues.killed_raid_npc.clear();
	}

	if (!etl_queues.aa_purchase.empty()) {
		PlayerEventAaPurchaseRepository::InsertMany(*m_database, etl_queues.aa_purchase);
		etl_queues.aa_purchase.clear();
	}

	LogPlayerEventsDetail(
		"Processing batch player event log queue of [{}] took [{}]",
		m_record_batch_queue.size(),
		benchmark.elapsed()
	);

	// empty
	m_record_batch_queue.clear();
	m_batch_queue_lock.unlock();
}

// adds a player event to the queue
void PlayerEventLogs::AddToQueue(PlayerEventLogsRepository::PlayerEventLogs &log)
{
	m_batch_queue_lock.lock();
	m_record_batch_queue.emplace_back(log);
	m_batch_queue_lock.unlock();
}

// fills common event data in the SendEvent function
void PlayerEventLogs::FillPlayerEvent(
	const PlayerEvent::PlayerEvent &p,
	PlayerEventLogsRepository::PlayerEventLogs &n
)
{
	n.account_id   = p.account_id;
	n.character_id = p.character_id;
	n.zone_id      = p.zone_id;
	n.instance_id  = p.instance_id;
	n.x            = p.x;
	n.y            = p.y;
	n.z            = p.z;
	n.heading      = p.heading;
}

// builds the dynamic packet used to ship the player event over the wire
// supports serializing the struct so it can be rebuilt on the other end
std::unique_ptr<ServerPacket>
PlayerEventLogs::BuildPlayerEventPacket(const PlayerEvent::PlayerEventContainer &e)
{
	EQ::Net::DynamicPacket dyn_pack;
	dyn_pack.PutSerialize(0, e);
	auto pack_size = sizeof(ServerSendPlayerEvent_Struct) + dyn_pack.Length();
	auto pack      = std::make_unique<ServerPacket>(ServerOP_PlayerEvent, static_cast<uint32_t>(pack_size));
	auto buf       = reinterpret_cast<ServerSendPlayerEvent_Struct *>(pack->pBuffer);
	buf->cereal_size = static_cast<uint32_t>(dyn_pack.Length());
	memcpy(buf->cereal_data, dyn_pack.Data(), dyn_pack.Length());

	return pack;
}

const PlayerEventLogSettingsRepository::PlayerEventLogSettings *PlayerEventLogs::GetSettings() const
{
	return m_settings;
}

bool PlayerEventLogs::IsEventDiscordEnabled(int32_t event_type_id)
{
	// out of bounds check
	if (event_type_id >= PlayerEvent::EventType::MAX) {
		return false;
	}

	// make sure webhook id is set
	if (m_settings[event_type_id].discord_webhook_id == 0) {
		return false;
	}

	// ensure there is a matching webhook to begin with
	if (!LogSys.GetDiscordWebhooks()[m_settings[event_type_id].discord_webhook_id].webhook_url.empty()) {
		return true;
	}

	return false;
}

std::string PlayerEventLogs::GetDiscordWebhookUrlFromEventType(int32_t event_type_id)
{
	// out of bounds check
	if (event_type_id >= PlayerEvent::EventType::MAX) {
		return "";
	}

	// make sure webhook id is set
	if (m_settings[event_type_id].discord_webhook_id == 0) {
		return "";
	}

	// ensure there is a matching webhook to begin with
	if (!LogSys.GetDiscordWebhooks()[m_settings[event_type_id].discord_webhook_id].webhook_url.empty()) {
		return LogSys.GetDiscordWebhooks()[m_settings[event_type_id].discord_webhook_id].webhook_url;
	}

	return "";
}

// GM_COMMAND           | [x] Implemented Formatter
// ZONING               | [x] Implemented Formatter
// AA_GAIN              | [x] Implemented Formatter
// AA_PURCHASE          | [x] Implemented Formatter
// FORAGE_SUCCESS       | [x] Implemented Formatter
// FORAGE_FAILURE       | [x] Implemented Formatter
// FISH_SUCCESS         | [x] Implemented Formatter
// FISH_FAILURE         | [x] Implemented Formatter
// ITEM_DESTROY         | [x] Implemented Formatter
// WENT_ONLINE          | [x] Implemented Formatter
// WENT_OFFLINE         | [x] Implemented Formatter
// LEVEL_GAIN           | [x] Implemented Formatter
// LEVEL_LOSS           | [x] Implemented Formatter
// LOOT_ITEM            | [x] Implemented Formatter
// MERCHANT_PURCHASE    | [x] Implemented Formatter
// MERCHANT_SELL        | [x] Implemented Formatter
// GROUP_JOIN           | [] Implemented Formatter
// GROUP_LEAVE          | [] Implemented Formatter
// RAID_JOIN            | [] Implemented Formatter
// RAID_LEAVE           | [] Implemented Formatter
// GROUNDSPAWN_PICKUP   | [x] Implemented Formatter
// NPC_HANDIN           | [x] Implemented Formatter
// SKILL_UP             | [x] Implemented Formatter
// TASK_ACCEPT          | [x] Implemented Formatter
// TASK_UPDATE          | [x] Implemented Formatter
// TASK_COMPLETE        | [x] Implemented Formatter
// TRADE                | [] Implemented Formatter
// GIVE_ITEM            | [] Implemented Formatter
// SAY                  | [x] Implemented Formatter
// REZ_ACCEPTED         | [x] Implemented Formatter
// DEATH                | [x] Implemented Formatter
// COMBINE_FAILURE      | [x] Implemented Formatter
// COMBINE_SUCCESS      | [x] Implemented Formatter
// DROPPED_ITEM         | [x] Implemented Formatter
// SPLIT_MONEY          | [x] Implemented Formatter
// DZ_JOIN              | [] Implemented Formatter
// DZ_LEAVE             | [] Implemented Formatter
// TRADER_PURCHASE      | [x] Implemented Formatter
// TRADER_SELL          | [x] Implemented Formatter
// BANDOLIER_CREATE     | [] Implemented Formatter
// BANDOLIER_SWAP       | [] Implemented Formatter
// DISCOVER_ITEM        | [X] Implemented Formatter

std::string PlayerEventLogs::GetDiscordPayloadFromEvent(const PlayerEvent::PlayerEventContainer &e)
{
	std::string payload;
	switch (e.player_event_log.event_type_id) {
		case PlayerEvent::AA_GAIN: {
			PlayerEvent::AAGainedEvent n{};
			std::stringstream          ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatAAGainedEvent(e, n);
			break;
		}
		case PlayerEvent::AA_PURCHASE: {
			PlayerEvent::AAPurchasedEvent n{};
			std::stringstream             ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatAAPurchasedEvent(e, n);
			break;
		}
		case PlayerEvent::COMBINE_FAILURE:
		case PlayerEvent::COMBINE_SUCCESS: {
			PlayerEvent::CombineEvent n{};
			std::stringstream         ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatCombineEvent(e, n);
			break;
		}
		case PlayerEvent::DEATH: {
			PlayerEvent::DeathEvent n{};
			std::stringstream       ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatDeathEvent(e, n);
			break;
		}
		case PlayerEvent::DISCOVER_ITEM: {
			PlayerEvent::DiscoverItemEvent n{};
			std::stringstream              ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatDiscoverItemEvent(e, n);
			break;
		}
		case PlayerEvent::DROPPED_ITEM: {
			PlayerEvent::DroppedItemEvent n{};
			std::stringstream             ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatDroppedItemEvent(e, n);
			break;
		}
		case PlayerEvent::FISH_FAILURE:
		case PlayerEvent::FORAGE_FAILURE:
		case PlayerEvent::WENT_ONLINE:
		case PlayerEvent::WENT_OFFLINE: {
			payload = PlayerEventDiscordFormatter::FormatWithNodata(e);
			break;
		}
		case PlayerEvent::FISH_SUCCESS: {
			PlayerEvent::FishSuccessEvent n{};
			std::stringstream             ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatFishSuccessEvent(e, n);
			break;
		}
		case PlayerEvent::FORAGE_SUCCESS: {
			PlayerEvent::ForageSuccessEvent n{};
			std::stringstream               ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatForageSuccessEvent(e, n);
			break;
		}
		case PlayerEvent::ITEM_DESTROY: {
			PlayerEvent::DestroyItemEvent n{};
			std::stringstream             ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatDestroyItemEvent(e, n);
			break;
		}
		case PlayerEvent::LEVEL_GAIN: {
			PlayerEvent::LevelGainedEvent n{};
			std::stringstream             ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatLevelGainedEvent(e, n);
			break;
		}
		case PlayerEvent::LEVEL_LOSS: {
			PlayerEvent::LevelLostEvent n{};
			std::stringstream           ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatLevelLostEvent(e, n);
			break;
		}
		case PlayerEvent::LOOT_ITEM: {
			PlayerEvent::LootItemEvent n{};
			std::stringstream          ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatLootItemEvent(e, n);
			break;
		}
		case PlayerEvent::GROUNDSPAWN_PICKUP: {
			PlayerEvent::GroundSpawnPickupEvent n{};
			std::stringstream                   ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatGroundSpawnPickupEvent(e, n);
			break;
		}
		case PlayerEvent::NPC_HANDIN: {
			PlayerEvent::HandinEvent n{};
			std::stringstream        ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatNPCHandinEvent(e, n);
			break;
		}
		case PlayerEvent::SAY: {
			PlayerEvent::SayEvent n{};
			std::stringstream     ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatEventSay(e, n);
			break;
		}
		case PlayerEvent::GM_COMMAND: {
			PlayerEvent::GMCommandEvent n{};
			std::stringstream           ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatGMCommand(e, n);
			break;
		}
		case PlayerEvent::SKILL_UP: {
			PlayerEvent::SkillUpEvent n{};
			std::stringstream         ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatSkillUpEvent(e, n);
			break;
		}
		case PlayerEvent::SPLIT_MONEY: {
			PlayerEvent::SplitMoneyEvent n{};
			std::stringstream            ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatSplitMoneyEvent(e, n);
			break;
		}
		case PlayerEvent::TASK_ACCEPT: {
			PlayerEvent::TaskAcceptEvent n{};
			std::stringstream            ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatTaskAcceptEvent(e, n);
			break;
		}
		case PlayerEvent::TASK_COMPLETE: {
			PlayerEvent::TaskCompleteEvent n{};
			std::stringstream              ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatTaskCompleteEvent(e, n);
			break;
		}
		case PlayerEvent::TASK_UPDATE: {
			PlayerEvent::TaskUpdateEvent n{};
			std::stringstream            ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatTaskUpdateEvent(e, n);
			break;
		}
		case PlayerEvent::TRADE: {
			PlayerEvent::TradeEvent n{};
			std::stringstream       ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatTradeEvent(e, n);
			break;
		}
		case PlayerEvent::TRADER_PURCHASE: {
			PlayerEvent::TraderPurchaseEvent n{};
			std::stringstream                ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatTraderPurchaseEvent(e, n);
			break;
		}
		case PlayerEvent::TRADER_SELL: {
			PlayerEvent::TraderSellEvent n{};
			std::stringstream            ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatTraderSellEvent(e, n);
			break;
		}
		case PlayerEvent::REZ_ACCEPTED: {
			PlayerEvent::ResurrectAcceptEvent n{};
			std::stringstream                 ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatResurrectAcceptEvent(e, n);
			break;
		}
		case PlayerEvent::MERCHANT_PURCHASE: {
			PlayerEvent::MerchantPurchaseEvent n{};
			std::stringstream                  ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}

			payload = PlayerEventDiscordFormatter::FormatMerchantPurchaseEvent(e, n);
			break;
		}
		case PlayerEvent::MERCHANT_SELL: {
			PlayerEvent::MerchantSellEvent n{};
			std::stringstream              ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}

			payload = PlayerEventDiscordFormatter::FormatMerchantSellEvent(e, n);
			break;
		}
		case PlayerEvent::ZONING: {
			PlayerEvent::ZoningEvent n{};
			std::stringstream        ss;
			{
				ss << e.player_event_log.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}

			payload = PlayerEventDiscordFormatter::FormatZoningEvent(e, n);
			break;
		}
		default: {
			LogInfo(
				"Player event [{}] ({}) Discord formatter not implemented",
				e.player_event_log.event_type_name,
				e.player_event_log.event_type_id
			);
		}
	}

	return payload;
}

// general process function, used in world or QS depending on rule Logging:PlayerEventsQSProcess
void PlayerEventLogs::Process()
{
	if (m_process_batch_events_timer.Check() || m_record_batch_queue.size() >= RuleI(Logging, BatchPlayerEventProcessChunkSize)) {
		ProcessBatchQueue();
	}

	if (m_process_retention_truncation_timer.Check()) {
		ProcessRetentionTruncation();
	}
}

void PlayerEventLogs::ProcessRetentionTruncation()
{
	LogPlayerEvents("Running truncation");

	for (int i = PlayerEvent::GM_COMMAND; i != PlayerEvent::MAX; i++) {
		if (m_settings[i].retention_days > 0) {
			uint32 deleted_count;
			if (m_settings[i].etl_enabled) {
				switch (m_settings[i].id) {
					case PlayerEvent::LOOT_ITEM: {
						deleted_count = PlayerEventLootItemsRepository::DeleteWhere(
							*m_database,
							fmt::format(
								"created_at < (NOW() - INTERVAL {} DAY)",
								m_settings[i].retention_days));
						break;
					}
					case PlayerEvent::MERCHANT_SELL: {
						deleted_count = PlayerEventMerchantSellRepository::DeleteWhere(
							*m_database,
							fmt::format(
								"created_at < (NOW() - INTERVAL {} DAY)",
								m_settings[i].retention_days));
						break;
					}
					case PlayerEvent::MERCHANT_PURCHASE: {
						deleted_count = PlayerEventMerchantPurchaseRepository::DeleteWhere(
							*m_database,
							fmt::format(
								"created_at < (NOW() - INTERVAL {} DAY)",
								m_settings[i].retention_days));
						break;
					}
					case PlayerEvent::NPC_HANDIN: {
						deleted_count = PlayerEventNpcHandinRepository::DeleteWhere(
							*m_database,
							fmt::format(
								"created_at < (NOW() - INTERVAL {} DAY)",
								m_settings[i].retention_days));
						deleted_count += PlayerEventNpcHandinEntriesRepository::DeleteWhere(
							*m_database,
							fmt::format(
								"created_at < (NOW() - INTERVAL {} DAY)",
								m_settings[i].retention_days));
						break;
					}
					case PlayerEvent::TRADE: {
						deleted_count = PlayerEventTradeRepository::DeleteWhere(
							*m_database,
							fmt::format(
								"created_at < (NOW() - INTERVAL {} DAY)",
								m_settings[i].retention_days));
						deleted_count += PlayerEventTradeEntriesRepository::DeleteWhere(
							*m_database,
							fmt::format(
								"created_at < (NOW() - INTERVAL {} DAY)",
								m_settings[i].retention_days));
						break;
					}
					case PlayerEvent::SPEECH: {
						deleted_count = PlayerEventSpeechRepository::DeleteWhere(
							*m_database,
							fmt::format(
								"created_at < (NOW() - INTERVAL {} DAY)",
								m_settings[i].retention_days));
						break;
					}
					case PlayerEvent::KILLED_NPC: {
						deleted_count = PlayerEventKilledNpcRepository::DeleteWhere(
							*m_database,
							fmt::format(
								"created_at < (NOW() - INTERVAL {} DAY)",
								m_settings[i].retention_days));
						break;
					}
					case PlayerEvent::KILLED_NAMED_NPC: {
						deleted_count = PlayerEventKilledNamedNpcRepository::DeleteWhere(
							*m_database,
							fmt::format(
								"created_at < (NOW() - INTERVAL {} DAY)",
								m_settings[i].retention_days));
						break;
					}
					case PlayerEvent::KILLED_RAID_NPC: {
						deleted_count = PlayerEventKilledRaidNpcRepository::DeleteWhere(
							*m_database,
							fmt::format(
								"created_at < (NOW() - INTERVAL {} DAY)",
								m_settings[i].retention_days));
						break;
					}
					case PlayerEvent::AA_PURCHASE: {
						deleted_count = PlayerEventAaPurchaseRepository::DeleteWhere(
							*m_database,
							fmt::format(
								"created_at < (NOW() - INTERVAL {} DAY)",
								m_settings[i].retention_days));
						break;
					}
					default: {
						LogError("NonImplemented ETL Event Type <red>[{}] ", static_cast<uint32>(m_settings[i].id));
					}
				}
			}
			else {
				deleted_count = PlayerEventLogsRepository::DeleteWhere(
					*m_database,
					fmt::format(
						"event_type_id = {} AND created_at < (NOW() - INTERVAL {} DAY)",
						i,
						m_settings[i].retention_days
					)
				);
			}

			if (deleted_count > 0) {
				LogInfo(
					"Truncated [{}] events of type [{}] ({}) older than [{}] days",
					deleted_count,
					PlayerEvent::EventName[i],
					i,
					m_settings[i].retention_days
				);
			}
		}
	}

	LoadEtlIds();
}

void PlayerEventLogs::ReloadSettings()
{
	for (auto &e: PlayerEventLogSettingsRepository::All(*m_database)) {
		if (e.id >= PlayerEvent::MAX || e.id < 0) {
			continue;
		}

		m_settings[e.id] = e;
	}
}

const int32_t RETENTION_DAYS_DEFAULT = 7;

void PlayerEventLogs::SetSettingsDefaults()
{
	m_settings[PlayerEvent::GM_COMMAND].event_enabled                = 1;
	m_settings[PlayerEvent::ZONING].event_enabled                    = 1;
	m_settings[PlayerEvent::AA_GAIN].event_enabled                   = 1;
	m_settings[PlayerEvent::AA_PURCHASE].event_enabled               = 1;
	m_settings[PlayerEvent::FORAGE_SUCCESS].event_enabled            = 0;
	m_settings[PlayerEvent::FORAGE_FAILURE].event_enabled            = 0;
	m_settings[PlayerEvent::FISH_SUCCESS].event_enabled              = 0;
	m_settings[PlayerEvent::FISH_FAILURE].event_enabled              = 0;
	m_settings[PlayerEvent::ITEM_DESTROY].event_enabled              = 1;
	m_settings[PlayerEvent::WENT_ONLINE].event_enabled               = 0;
	m_settings[PlayerEvent::WENT_OFFLINE].event_enabled              = 0;
	m_settings[PlayerEvent::LEVEL_GAIN].event_enabled                = 1;
	m_settings[PlayerEvent::LEVEL_LOSS].event_enabled                = 1;
	m_settings[PlayerEvent::LOOT_ITEM].event_enabled                 = 1;
	m_settings[PlayerEvent::MERCHANT_PURCHASE].event_enabled         = 1;
	m_settings[PlayerEvent::MERCHANT_SELL].event_enabled             = 1;
	m_settings[PlayerEvent::GROUP_JOIN].event_enabled                = 0;
	m_settings[PlayerEvent::GROUP_LEAVE].event_enabled               = 0;
	m_settings[PlayerEvent::RAID_JOIN].event_enabled                 = 0;
	m_settings[PlayerEvent::RAID_LEAVE].event_enabled                = 0;
	m_settings[PlayerEvent::GROUNDSPAWN_PICKUP].event_enabled        = 1;
	m_settings[PlayerEvent::NPC_HANDIN].event_enabled                = 1;
	m_settings[PlayerEvent::SKILL_UP].event_enabled                  = 0;
	m_settings[PlayerEvent::TASK_ACCEPT].event_enabled               = 1;
	m_settings[PlayerEvent::TASK_UPDATE].event_enabled               = 1;
	m_settings[PlayerEvent::TASK_COMPLETE].event_enabled             = 1;
	m_settings[PlayerEvent::TRADE].event_enabled                     = 1;
	m_settings[PlayerEvent::GIVE_ITEM].event_enabled                 = 1;
	m_settings[PlayerEvent::SAY].event_enabled                       = 0;
	m_settings[PlayerEvent::REZ_ACCEPTED].event_enabled              = 1;
	m_settings[PlayerEvent::DEATH].event_enabled                     = 1;
	m_settings[PlayerEvent::COMBINE_FAILURE].event_enabled           = 1;
	m_settings[PlayerEvent::COMBINE_SUCCESS].event_enabled           = 1;
	m_settings[PlayerEvent::DROPPED_ITEM].event_enabled              = 1;
	m_settings[PlayerEvent::SPLIT_MONEY].event_enabled               = 1;
	m_settings[PlayerEvent::DZ_JOIN].event_enabled                   = 1;
	m_settings[PlayerEvent::DZ_LEAVE].event_enabled                  = 1;
	m_settings[PlayerEvent::TRADER_PURCHASE].event_enabled           = 1;
	m_settings[PlayerEvent::TRADER_SELL].event_enabled               = 1;
	m_settings[PlayerEvent::BANDOLIER_CREATE].event_enabled          = 0;
	m_settings[PlayerEvent::BANDOLIER_SWAP].event_enabled            = 0;
	m_settings[PlayerEvent::DISCOVER_ITEM].event_enabled             = 1;
	m_settings[PlayerEvent::POSSIBLE_HACK].event_enabled             = 1;
	m_settings[PlayerEvent::KILLED_NPC].event_enabled                = 0;
	m_settings[PlayerEvent::KILLED_NAMED_NPC].event_enabled          = 1;
	m_settings[PlayerEvent::KILLED_RAID_NPC].event_enabled           = 1;
	m_settings[PlayerEvent::ITEM_CREATION].event_enabled             = 1;
	m_settings[PlayerEvent::GUILD_TRIBUTE_DONATE_ITEM].event_enabled = 1;
	m_settings[PlayerEvent::GUILD_TRIBUTE_DONATE_PLAT].event_enabled = 1;
	m_settings[PlayerEvent::PARCEL_SEND].event_enabled               = 1;
	m_settings[PlayerEvent::PARCEL_RETRIEVE].event_enabled           = 1;
	m_settings[PlayerEvent::PARCEL_DELETE].event_enabled             = 1;
	m_settings[PlayerEvent::BARTER_TRANSACTION].event_enabled        = 1;
	m_settings[PlayerEvent::EVOLVE_ITEM].event_enabled               = 1;
	m_settings[PlayerEvent::SPEECH].event_enabled                    = 1;

	for (int i = PlayerEvent::GM_COMMAND; i != PlayerEvent::MAX; i++) {
		m_settings[i].retention_days = RETENTION_DAYS_DEFAULT;
	}
}

void PlayerEventLogs::LoadEtlIds()
{
	auto e = [&](auto p) -> bool {
		for (PlayerEventLogSettingsRepository::PlayerEventLogSettings const& c : m_settings) {
			if(c.id == p) {
				return c.etl_enabled ? true : false;
			}
		}

		return false;
	};

	m_etl_settings.clear();
	m_etl_settings = {
		{
			PlayerEvent::LOOT_ITEM,
			{
				.enabled = e(PlayerEvent::LOOT_ITEM),
				.table_name = "player_event_loot_items",
				.next_id = PlayerEventLootItemsRepository::GetNextAutoIncrementId(*m_database)
			}
		},
		{
			PlayerEvent::MERCHANT_SELL,
			{
				.enabled = e(PlayerEvent::MERCHANT_SELL),
				.table_name = "player_event_merchant_sell",
				.next_id = PlayerEventMerchantSellRepository::GetNextAutoIncrementId(*m_database)
			}
		},
		{
			PlayerEvent::MERCHANT_PURCHASE,
			{
				.enabled = e(PlayerEvent::MERCHANT_PURCHASE),
				.table_name = "player_event_merchant_purchase",
				.next_id = PlayerEventMerchantPurchaseRepository::GetNextAutoIncrementId(*m_database)
			}
		},
		{
			PlayerEvent::NPC_HANDIN,
			{
				.enabled = e(PlayerEvent::NPC_HANDIN),
				.table_name = "player_event_npc_handin",
				.next_id = PlayerEventNpcHandinRepository::GetNextAutoIncrementId(*m_database)
			}
		},
		{
			PlayerEvent::TRADE,
			{
				.enabled = e(PlayerEvent::TRADE),
				.table_name = "player_event_trade",
				.next_id = PlayerEventTradeRepository::GetNextAutoIncrementId(*m_database)
			}
		},
		{
			PlayerEvent::SPEECH,
			{
				.enabled = e(PlayerEvent::SPEECH),
				.table_name = "player_event_speech",
				.next_id = PlayerEventSpeechRepository::GetNextAutoIncrementId(*m_database)
			}
		},
		{
			PlayerEvent::KILLED_NPC,
			{
				.enabled = e(PlayerEvent::KILLED_NPC),
				.table_name = "player_event_killed_npc",
				.next_id = PlayerEventKilledNpcRepository::GetNextAutoIncrementId(*m_database)
			}
		},
		{
			PlayerEvent::KILLED_NAMED_NPC,
			{
				.enabled = e(PlayerEvent::KILLED_NAMED_NPC),
				.table_name = "player_event_killed_named_npc",
				.next_id = PlayerEventKilledNamedNpcRepository::GetNextAutoIncrementId(*m_database)
			}
		},
		{
			PlayerEvent::KILLED_RAID_NPC,
			{
				.enabled = e(PlayerEvent::KILLED_RAID_NPC),
				.table_name = "player_event_killed_raid_npc",
				.next_id = PlayerEventKilledRaidNpcRepository::GetNextAutoIncrementId(*m_database)
			}
		},
		{
			PlayerEvent::AA_PURCHASE,
			{
				.enabled = e(PlayerEvent::AA_PURCHASE),
				.table_name = "player_event_aa_purchase",
				.next_id = PlayerEventAaPurchaseRepository::GetNextAutoIncrementId(*m_database)
			}
		}
	};
}

bool PlayerEventLogs::LoadDatabaseConnection()
{
	const auto c = EQEmuConfig::get();

	LogInfo(
		"Connecting to MySQL for PlayerEvents [{}]@[{}]:[{}]",
		c->DatabaseUsername.c_str(),
		c->DatabaseHost.c_str(),
		c->DatabasePort
	);

	if (!player_event_database.Connect(
		c->DatabaseHost.c_str(),
		c->DatabaseUsername.c_str(),
		c->DatabasePassword.c_str(),
		c->DatabaseDB.c_str(),
		c->DatabasePort
	)) {
		LogError("Cannot continue without a database connection for player events.");
		return false;
	}

	SetDatabase(&player_event_database);
	return true;
}