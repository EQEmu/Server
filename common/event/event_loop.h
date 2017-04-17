#pragma once
#include <functional>
#include <uv.h>
#include <cstring>

namespace EQ
{
	class EventLoop
	{
	public:
		static EventLoop &Get() {
			static EventLoop inst;
			return inst;
		}

		~EventLoop() {
			uv_loop_close(&m_loop);
		}

		void Process() {
			uv_run(&m_loop, UV_RUN_NOWAIT);
		}

		uv_loop_t* Handle() { return &m_loop; }

	private:
		EventLoop() {
			memset(&m_loop, 0, sizeof(uv_loop_t));
			uv_loop_init(&m_loop);
		}
		
		EventLoop(const EventLoop&);
		EventLoop& operator=(const EventLoop&);
	
		uv_loop_t m_loop;
	};
}
