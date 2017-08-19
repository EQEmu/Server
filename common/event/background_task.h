#pragma once
#include <functional>
#include "../any.h"
#include "event_loop.h"

namespace EQ {
	class BackgroundTask
	{
	public:
		typedef std::function<void(EQEmu::Any&)> BackgroundTaskFunction;
		struct BackgroundTaskBaton
		{
			BackgroundTaskFunction fn;
			BackgroundTaskFunction on_finish;
			EQEmu::Any data;
		};

		BackgroundTask(BackgroundTaskFunction fn, BackgroundTaskFunction on_finish, EQEmu::Any data) {
			uv_work_t *m_work = new uv_work_t;
			memset(m_work, 0, sizeof(uv_work_t));
			BackgroundTaskBaton *baton = new BackgroundTaskBaton();
			baton->fn = fn;
			baton->on_finish = on_finish;
			baton->data = data;
		
			m_work->data = baton;
			uv_queue_work(EventLoop::Get().Handle(), m_work, [](uv_work_t* req) {
				BackgroundTaskBaton *baton = (BackgroundTaskBaton*)req->data;
				baton->fn(baton->data);
			}, [](uv_work_t* req, int status) {
				BackgroundTaskBaton *baton = (BackgroundTaskBaton*)req->data;
				baton->on_finish(baton->data);
				delete baton;
				delete req;
			});
		}

		~BackgroundTask() {

		}
	};
}
