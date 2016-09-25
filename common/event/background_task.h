#pragma once
#include <functional>
#include "event_loop.h"

namespace EQ {
	class BackgroundTask
	{
	public:
		typedef std::function<void(void)> BackgroundTaskFunction;
		struct BackgroundTaskBaton
		{
			BackgroundTaskFunction fn;
			BackgroundTaskFunction on_finish;
		};

		BackgroundTask(BackgroundTaskFunction fn, BackgroundTaskFunction on_finish) {
			uv_work_t *m_work = new uv_work_t;
			memset(m_work, 0, sizeof(uv_work_t));
			BackgroundTaskBaton *baton = new BackgroundTaskBaton();
			baton->fn = fn;
			baton->on_finish = on_finish;
		
			m_work->data = baton;
			uv_queue_work(EventLoop::Get().Handle(), m_work, [](uv_work_t* req) {
				BackgroundTaskBaton *baton = (BackgroundTaskBaton*)req->data;
				baton->fn();
			}, [](uv_work_t* req, int status) {
				BackgroundTaskBaton *baton = (BackgroundTaskBaton*)req->data;
				baton->on_finish();
				delete baton;
				delete req;
			});
		}

		~BackgroundTask() {

		}
	};
}
