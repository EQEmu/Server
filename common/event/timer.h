#pragma once
#include <functional>
#include "event_loop.h"

namespace EQ {
	class Timer
	{
	public:
		Timer(uint64_t duration_ms, bool repeats, std::function<void(void)> cb)
		{
			memset(&m_timer, 0, sizeof(uv_timer_t));
			m_cb = cb;
			m_duration_ms = duration_ms;
			m_repeats = repeats;
			m_attached = nullptr;
			Attach(EventLoop::Get().Handle());
		}

		~Timer()
		{
			Detach();
		}
	private:
		void Execute() {
			m_cb();
		}

		virtual void Attach(uv_loop_t *loop) {
			if (!m_attached) {
				uv_timer_init(loop, &m_timer);
				m_timer.data = this;

				if (m_repeats) {
					uv_timer_start(&m_timer, [](uv_timer_t *handle) {
						Timer *t = (Timer*)handle->data;
						t->Execute();
					}, m_duration_ms, m_duration_ms);
				}
				else {
					uv_timer_start(&m_timer, [](uv_timer_t *handle) {
						Timer *t = (Timer*)handle->data;
						t->Execute();
					}, m_duration_ms, 0);
				}

				m_attached = loop;
			}
		}

		virtual void Detach() {
			if (m_attached) {
				uv_timer_stop(&m_timer);
				m_attached = nullptr;
			}
		}

		uv_timer_t m_timer;
		std::function<void(void)> m_cb;
		uint64_t m_duration_ms;
		bool m_repeats;
		uv_loop_t *m_attached;
	};
}
