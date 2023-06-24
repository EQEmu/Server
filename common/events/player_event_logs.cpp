#include <cereal/archives/json.hpp>
#include "player_event_logs.h"
#include "player_event_discord_formatter.h"
#include "../platform.h"
#include "../rulesys.h"

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
			LogInfo(
				"[New] PlayerEvent [{}] ({})",
				PlayerEvent::EventName[i],
				i
			);

			auto c = PlayerEventLogSettingsRepository::NewEntity();
			c.id             = i;
			c.event_name     = PlayerEvent::EventName[i];
			c.event_enabled  = m_settings[i].event_enabled;
			c.retention_days = m_settings[i].retention_days;
			PlayerEventLogSettingsRepository::InsertOne(*m_database, c);
		}
	}

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

	BenchTimer benchmark;

	// flush many
	PlayerEventLogsRepository::InsertMany(*m_database, m_record_batch_queue);
	LogPlayerEventsDetail(
		"Processing batch player event log queue of [{}] took [{}]",
		m_record_batch_queue.size(),
		benchmark.elapsed()
	);

	// empty
	m_record_batch_queue = {};
	m_batch_queue_lock.unlock();
}

// adds a player event to the queue
void PlayerEventLogs::AddToQueue(const PlayerEventLogsRepository::PlayerEventLogs &log)
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
	LogInfo("Running truncation");

	for (int i = PlayerEvent::GM_COMMAND; i != PlayerEvent::MAX; i++) {
		if (m_settings[i].retention_days > 0) {
			int deleted_count = PlayerEventLogsRepository::DeleteWhere(
				*m_database,
				fmt::format(
					"event_type_id = {} AND created_at < (NOW() - INTERVAL {} DAY)",
					i,
					m_settings[i].retention_days
				)
			);

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
}

void PlayerEventLogs::ReloadSettings()
{
	for (auto &e: PlayerEventLogSettingsRepository::All(*m_database)) {
		m_settings[e.id] = e;
	}
}

const int32_t RETENTION_DAYS_DEFAULT = 7;

void PlayerEventLogs::SetSettingsDefaults()
{
	m_settings[PlayerEvent::GM_COMMAND].event_enabled         = 1;
	m_settings[PlayerEvent::ZONING].event_enabled             = 1;
	m_settings[PlayerEvent::AA_GAIN].event_enabled            = 1;
	m_settings[PlayerEvent::AA_PURCHASE].event_enabled        = 1;
	m_settings[PlayerEvent::FORAGE_SUCCESS].event_enabled     = 0;
	m_settings[PlayerEvent::FORAGE_FAILURE].event_enabled     = 0;
	m_settings[PlayerEvent::FISH_SUCCESS].event_enabled       = 0;
	m_settings[PlayerEvent::FISH_FAILURE].event_enabled       = 0;
	m_settings[PlayerEvent::ITEM_DESTROY].event_enabled       = 1;
	m_settings[PlayerEvent::WENT_ONLINE].event_enabled        = 0;
	m_settings[PlayerEvent::WENT_OFFLINE].event_enabled       = 0;
	m_settings[PlayerEvent::LEVEL_GAIN].event_enabled         = 1;
	m_settings[PlayerEvent::LEVEL_LOSS].event_enabled         = 1;
	m_settings[PlayerEvent::LOOT_ITEM].event_enabled          = 1;
	m_settings[PlayerEvent::MERCHANT_PURCHASE].event_enabled  = 1;
	m_settings[PlayerEvent::MERCHANT_SELL].event_enabled      = 1;
	m_settings[PlayerEvent::GROUP_JOIN].event_enabled         = 0;
	m_settings[PlayerEvent::GROUP_LEAVE].event_enabled        = 0;
	m_settings[PlayerEvent::RAID_JOIN].event_enabled          = 0;
	m_settings[PlayerEvent::RAID_LEAVE].event_enabled         = 0;
	m_settings[PlayerEvent::GROUNDSPAWN_PICKUP].event_enabled = 1;
	m_settings[PlayerEvent::NPC_HANDIN].event_enabled         = 1;
	m_settings[PlayerEvent::SKILL_UP].event_enabled           = 0;
	m_settings[PlayerEvent::TASK_ACCEPT].event_enabled        = 1;
	m_settings[PlayerEvent::TASK_UPDATE].event_enabled        = 1;
	m_settings[PlayerEvent::TASK_COMPLETE].event_enabled      = 1;
	m_settings[PlayerEvent::TRADE].event_enabled              = 1;
	m_settings[PlayerEvent::GIVE_ITEM].event_enabled          = 1;
	m_settings[PlayerEvent::SAY].event_enabled                = 0;
	m_settings[PlayerEvent::REZ_ACCEPTED].event_enabled       = 1;
	m_settings[PlayerEvent::DEATH].event_enabled              = 1;
	m_settings[PlayerEvent::COMBINE_FAILURE].event_enabled    = 1;
	m_settings[PlayerEvent::COMBINE_SUCCESS].event_enabled    = 1;
	m_settings[PlayerEvent::DROPPED_ITEM].event_enabled       = 1;
	m_settings[PlayerEvent::SPLIT_MONEY].event_enabled        = 1;
	m_settings[PlayerEvent::DZ_JOIN].event_enabled            = 1;
	m_settings[PlayerEvent::DZ_LEAVE].event_enabled           = 1;
	m_settings[PlayerEvent::TRADER_PURCHASE].event_enabled    = 1;
	m_settings[PlayerEvent::TRADER_SELL].event_enabled        = 1;
	m_settings[PlayerEvent::BANDOLIER_CREATE].event_enabled   = 0;
	m_settings[PlayerEvent::BANDOLIER_SWAP].event_enabled     = 0;
	m_settings[PlayerEvent::DISCOVER_ITEM].event_enabled      = 1;
	m_settings[PlayerEvent::POSSIBLE_HACK].event_enabled      = 1;
	m_settings[PlayerEvent::KILLED_NPC].event_enabled         = 0;
	m_settings[PlayerEvent::KILLED_NAMED_NPC].event_enabled   = 1;
	m_settings[PlayerEvent::KILLED_RAID_NPC].event_enabled    = 1;
	m_settings[PlayerEvent::ITEM_CREATION].event_enabled      = 1;

	for (int i = PlayerEvent::GM_COMMAND; i != PlayerEvent::MAX; i++) {
		m_settings[i].retention_days = RETENTION_DAYS_DEFAULT;
	}
}
