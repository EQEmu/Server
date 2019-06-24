#pragma once
#include <functional>
#include <uv.h>
#include <cstring>

namespace EQ
{
	class EventLoop
	{
	public:
		EventLoop() {
			memset(&m_loop, 0, sizeof(uv_loop_t));
			uv_loop_init(&m_loop);
		}

		~EventLoop() {
			uv_loop_close(&m_loop);
		}

		EventLoop(const EventLoop&) = delete;
		EventLoop& operator=(const EventLoop&) = delete;
		
		static EventLoop &GetDefault() {
			static thread_local EventLoop inst;
			return inst;
		}

		void Process() {
			uv_run(&m_loop, UV_RUN_NOWAIT);
		}

		uv_loop_t* Handle() { return &m_loop; }

	private:	
		uv_loop_t m_loop;
	};
}
