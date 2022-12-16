#include <cereal/archives/json.hpp>
#include "player_event_logs.h"
#include "../timer.h"
#include "player_event_discord_formatter.h"

// general initialization routine
void PlayerEventLogs::Init()
{
	ValidateDatabaseConnection();

	// initialize settings array
	for (int i = PlayerEvent::GM_COMMAND; i != PlayerEvent::MAX; i++) {
		m_settings[i].id                 = i;
		m_settings[i].event_name         = PlayerEvent::EventName[i];
		m_settings[i].event_enabled      = 1;
		m_settings[i].retention_days     = 0;
		m_settings[i].discord_webhook_id = 0;
	}

	// initialize settings from database
	auto             s = PlayerEventLogSettingsRepository::All(*m_database);
	std::vector<int> db_settings{};
	db_settings.reserve(s.size());
	for (auto &e: s) {
		m_settings[e.id] = e;
		db_settings.emplace_back(e.id);
	}

	// insert entries that don't exist in database
	for (int i = PlayerEvent::GM_COMMAND; i != PlayerEvent::MAX; i++) {
		bool is_missing_in_database = std::find(db_settings.begin(), db_settings.end(), i) == db_settings.end();
		if (is_missing_in_database) {
			LogInfo(
				"[New] PlayerEvent [{}] ({})",
				PlayerEvent::EventName[i],
				i
			);

			auto c = PlayerEventLogSettingsRepository::NewEntity();
			c.id            = i;
			c.event_name    = PlayerEvent::EventName[i];
			c.event_enabled = m_settings[i].event_enabled;
			PlayerEventLogSettingsRepository::InsertOne(*m_database, c);
		}
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
		LogError("[PlayerEventLogs::ValidateDatabaseConnection] No database connection");
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
	if (m_record_batch_queue.empty()) {
		return;
	}

	BenchTimer benchmark;

	// flush many
	PlayerEventLogsRepository::InsertMany(*m_database, m_record_batch_queue);
	LogInfo(
		"Processing batch player event log queue of [{}] took [{}]",
		m_record_batch_queue.size(),
		benchmark.elapsed()
	);

	// empty
	m_record_batch_queue = {};
}

// adds a player event to the queue
void PlayerEventLogs::AddToQueue(const PlayerEventLogsRepository::PlayerEventLogs &log)
{
	m_record_batch_queue.emplace_back(log);
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
PlayerEventLogs::BuildPlayerEventPacket(const BasePlayerEventLogsRepository::PlayerEventLogs &e)
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
	if (event_type_id > PlayerEvent::EventType::MAX) {
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
	if (event_type_id > PlayerEvent::EventType::MAX) {
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

std::string PlayerEventLogs::GetDiscordPayloadFromEvent(const PlayerEventLogsRepository::PlayerEventLogs &e)
{
	std::string payload;

	switch (e.event_type_id) {
		case PlayerEvent::SAY:
			PlayerEvent::SayEvent n;
			std::stringstream     ss;
			{
				ss << e.event_data;
				cereal::JSONInputArchive ar(ss);
				n.serialize(ar);
			}
			payload = PlayerEventDiscordFormatter::FormatEventSay(e, n);
			break;
	}

	return payload;
}
