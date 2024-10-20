#ifndef EQEMU_PLAYER_EVENT_LOGS_H
#define EQEMU_PLAYER_EVENT_LOGS_H

#include <any>
#include <cereal/archives/json.hpp>
#include <mutex>
#include <random>
#include "../json/json_archive_single_line.h"
#include "../repositories/player_event_log_settings_repository.h"
#include "../repositories/player_event_logs_repository.h"
#include "../servertalk.h"
#include "../timer.h"
#include "../repositories/player_event_loot_items_repository.h"
#include "../repositories/player_event_merchant_sell_repository.h"
#include "../repositories/player_event_merchant_purchase_repository.h"

class PlayerEventLogs {
public:
	void Init();
	void ReloadSettings();
	void LoadETLIDs();
	void IncrementDetailTableIDCache(PlayerEvent::EventType event_type) { m_last_id_cache_detail_tables[event_type]++; }
	PlayerEventLogs *SetDatabase(Database *db);
	bool ValidateDatabaseConnection();
	bool IsEventEnabled(PlayerEvent::EventType event);
	std::map<PlayerEvent::EventType, uint64>& GetETLIDCache() { return m_last_id_cache_detail_tables; }

	void Process();

	// batch queue
	void AddToQueue(PlayerEventLogsRepository::PlayerEventLogs &logs);

	// main event record generic function
	// can ingest any struct event types
	template<typename T>
	std::unique_ptr<ServerPacket> RecordEvent(
		PlayerEvent::EventType t,
		const PlayerEvent::PlayerEvent &p,
		T e
	)
	{
		auto n = PlayerEventLogsRepository::NewEntity();
		FillPlayerEvent(p, n);
		n.event_type_id = t;

		std::stringstream ss;
		{
			cereal::JSONOutputArchiveSingleLine ar(ss);
			e.serialize(ar);
		}

		n.event_type_name = PlayerEvent::EventName[t];
		n.event_data      = Strings::Contains(ss.str(), "noop") ? "{}" : ss.str();
		n.created_at      = std::time(nullptr);

		auto c = PlayerEvent::PlayerEventContainer{
			.player_event = p,
			.player_event_log = n
		};

		return BuildPlayerEventPacket(c);
	}

	[[nodiscard]] const PlayerEventLogSettingsRepository::PlayerEventLogSettings *GetSettings() const;
	bool IsEventDiscordEnabled(int32_t event_type_id);
	std::string GetDiscordWebhookUrlFromEventType(int32_t event_type_id);

	static std::string GetDiscordPayloadFromEvent(const PlayerEvent::PlayerEventContainer &e);

private:
	Database                                                 *m_database; // reference to database
	PlayerEventLogSettingsRepository::PlayerEventLogSettings m_settings[PlayerEvent::EventType::MAX]{};
	std::map<PlayerEvent::EventType, uint64> m_last_id_cache_detail_tables{};

	// batch queue is used to record events in batch
	std::vector<PlayerEventLogsRepository::PlayerEventLogs> m_record_batch_queue{};
	static void FillPlayerEvent(const PlayerEvent::PlayerEvent &p, PlayerEventLogsRepository::PlayerEventLogs &n);
	static std::unique_ptr<ServerPacket>
	BuildPlayerEventPacket(const PlayerEvent::PlayerEventContainer &e);
	std::map<PlayerEvent::EventType, std::any> m_record_etl_queue{};

	// timers
	Timer m_process_batch_events_timer; // events processing timer
	Timer m_process_retention_truncation_timer; // timer for truncating events based on retention settings

	// processing
	std::mutex m_batch_queue_lock{};
	void ProcessBatchQueue();
	void ProcessRetentionTruncation();
	void SetSettingsDefaults();

public:
	struct EtlInfo_Struct {
		std::string                                                   etl_table_name;
		std::vector<std::any>                                         etl_queue;
		std::function<int(Database &, const std::vector<std::any> &)> etl_load_func_ptr;
	};

	std::map<PlayerEvent::EventType, EtlInfo_Struct> m_etl_data = {
		{
			PlayerEvent::LOOT_ITEM,
			{
			  .etl_table_name    = "player_event_loot_items",
			  .etl_queue         = {},
			  .etl_load_func_ptr = { &PlayerEventLootItemsRepository::InsertManyFromStdAny },
			}
		},
		{
			PlayerEvent::MERCHANT_SELL,
			{
			  .etl_table_name    = "player_event_merchant_sell",
			  .etl_queue         = {},
			  .etl_load_func_ptr = { &PlayerEventMerchantSellRepository::InsertManyFromStdAny },
			}
		},
		{
			PlayerEvent::MERCHANT_PURCHASE,
			{
				.etl_table_name    = "player_event_merchant_purchase",
				.etl_queue         = {},
				.etl_load_func_ptr = { &PlayerEventMerchantPurchaseRepository::InsertManyFromStdAny },
			  }
		}

	};

};

extern PlayerEventLogs player_event_logs;

#endif //EQEMU_PLAYER_EVENT_LOGS_H
