#pragma once
#include <functional>
#include "event_loop.h"

namespace EQ {
	class Timer
	{
	public:
		Timer(EventLoop &loop, std::function<void(Timer *)> cb) : _loop(loop)
		{
			_timer = nullptr;
			_cb = cb;
		}

		Timer(EventLoop &loop, uint64_t duration_ms, bool repeats, std::function<void(Timer *)> cb) : _loop(loop)
		{
			_timer = nullptr;
			_cb = cb;
			Start(duration_ms, repeats);
		}

		Timer(std::function<void(Timer *)> cb) : _loop(EventLoop::GetDefault())
		{
			_timer = nullptr;
			_cb = cb;
		}

		Timer(uint64_t duration_ms, bool repeats, std::function<void(Timer *)> cb) : _loop(EventLoop::GetDefault())
		{
			_timer = nullptr;
			_cb = cb;
			Start(duration_ms, repeats);
		}
	
		~Timer()
		{
			Stop();
		}

		void Start(uint64_t duration_ms, bool repeats) {
			auto loop = EventLoop::GetDefault().Handle();
			if (!_timer) {
				_timer = new uv_timer_t;
				memset(_timer, 0, sizeof(uv_timer_t));
				uv_timer_init(loop, _timer);
				_timer->data = this;

				if (repeats) {
					uv_timer_start(_timer, [](uv_timer_t *handle) {
						Timer *t = (Timer*)handle->data;
						t->Execute();
					}, duration_ms, duration_ms);
				}
				else {
					uv_timer_start(_timer, [](uv_timer_t *handle) {
						Timer *t = (Timer*)handle->data;
						t->Stop();
						t->Execute();
					}, duration_ms, 0);
				}
			}
		}

		void Stop() {
			if (_timer) {
				uv_close((uv_handle_t*)_timer, [](uv_handle_t* handle) {
					delete handle;
				});
				_timer = nullptr;
			}
		}
	private:
		void Execute() {
			_cb(this);
		}
	
		EventLoop &_loop;
		uv_timer_t *_timer;
		std::function<void(Timer*)> _cb;
	};
}
