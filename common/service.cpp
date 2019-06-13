#include "service.h"
#include "event/event_loop.h"
#include "event/timer.h"
#include <thread>
#include <chrono>

struct EQ::Service::Impl
{
	bool running;
	std::string identifier;
	size_t heartbeat_duration_ms;
	size_t sleep_duration_ms;
	std::unique_ptr<EQ::WorldConnection> world_connection;
	std::unique_ptr<EQ::Timer> heartbeat_timer;
	std::chrono::steady_clock::time_point last_time;
};

EQ::Service::Service(const std::string &identifier, size_t heartbeat_duration_ms, size_t sleep_duration_ms)
{
	_impl.reset(new Impl());
	_impl->running = false;
	_impl->identifier = identifier;
	_impl->heartbeat_duration_ms = heartbeat_duration_ms;
	_impl->sleep_duration_ms = sleep_duration_ms;
}

EQ::Service::~Service()
{
}

void EQ::Service::Run()
{
	_impl->running = true;

	OnStart();

	//If start canceled our run then just quit, dont bother initializing everything else
	if (!_impl->running) {
		return;
	}

	_impl->world_connection.reset(new EQ::WorldConnection(_impl->identifier));
	_impl->world_connection->SetOnRoutedMessageHandler([this](const std::string& filter, const std::string& identifier, const std::string& id, const EQ::Net::Packet& payload) {
		OnRoutedMessage(filter, identifier, id, payload);
	});
	_impl->last_time = std::chrono::steady_clock::now();

	_impl->heartbeat_timer.reset(new EQ::Timer(_impl->heartbeat_duration_ms, true, [this](EQ::Timer *t) {
		auto now = std::chrono::steady_clock::now();
		auto time_since = std::chrono::duration_cast<std::chrono::duration<double>>(now - _impl->last_time);
		OnHeartbeat(time_since.count());
		_impl->last_time = now;
	}));

	auto &loop = EQ::EventLoop::Get();
	auto sleep_duration = _impl->sleep_duration_ms;
	while (_impl->running) {
		loop.Process();
		std::this_thread::sleep_for(std::chrono::milliseconds(sleep_duration));
	}

	_impl->heartbeat_timer.release();
	_impl->world_connection.release();

	OnStop();
}

void EQ::Service::RouteMessage(const std::string &filter, const std::string &id, const EQ::Net::Packet &p)
{
	_impl->world_connection->RouteMessage(filter, id, p);
}

void EQ::Service::Stop() {
	_impl->running = false;
}
