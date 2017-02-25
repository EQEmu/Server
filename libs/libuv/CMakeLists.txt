cmake_minimum_required(VERSION 2.8)
project(libuv)

set(libuv_sources
	src/fs-poll.c
	src/inet.c
	src/threadpool.c
	src/uv-common.c
	src/version.c
)

set(libuv_headers
	src/heap-inl.h
	src/queue.h
	src/uv-common.h
)

if(NOT WIN32)
	find_package (Threads REQUIRED)
	
	list(APPEND libuv_sources
		src/unix/async.c
		src/unix/core.c
		src/unix/dl.c
		src/unix/fs.c
		src/unix/getaddrinfo.c
		src/unix/getnameinfo.c
		src/unix/loop-watcher.c
		src/unix/loop.c
		src/unix/pipe.c
		src/unix/poll.c
		src/unix/process.c
		src/unix/signal.c
		src/unix/stream.c
		src/unix/tcp.c
		src/unix/thread.c
		src/unix/timer.c
		src/unix/tty.c
		src/unix/udp.c
	)
	
	list(APPEND libuv_headers
		src/unix/atomic-ops.h
		src/unix/internal.h
		src/unix/spinlock.h
	)
	
	if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
		add_definitions(-D_GNU_SOURCE)
		list(APPEND libuv_sources
			src/unix/linux-core.c
			src/unix/linux-inotify.c
			src/unix/linux-syscalls.c
			src/unix/proctitle.c
		)
		
		list(APPEND libuv_headers
			src/unix/linux-syscalls.h
		)
	endif()
	
	if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
		add_definitions(-D_DARWIN_USE_64_BIT_INODE=1)
		add_definitions(-D_DARWIN_UNLIMITED_SELECT=1)
		list(APPEND libuv_sources
			src/unix/darwin.c
			src/unix/darwin-proctitle.c
			src/unix/fsevents.c
			src/unix/kqueue.c
			src/unix/proctitle.c
		)
	endif()

else()
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	add_definitions(-D_WIN32_WINNT=0x0600)
	
	list(APPEND libuv_sources
		src/win/async.c
		src/win/core.c
		src/win/detect-wakeup.c
		src/win/dl.c
		src/win/error.c
		src/win/fs-event.c
		src/win/fs.c
		src/win/getaddrinfo.c
		src/win/getnameinfo.c
		src/win/handle.c
		src/win/loop-watcher.c
		src/win/pipe.c
		src/win/poll.c
		src/win/process-stdio.c
		src/win/process.c
		src/win/req.c
		src/win/signal.c
		src/win/stream.c
		src/win/tcp.c
		src/win/thread.c
		src/win/timer.c
		src/win/tty.c
		src/win/udp.c
		src/win/util.c
		src/win/winapi.c
		src/win/winsock.c
	)
	
	list(APPEND libuv_headers
		src/win/atomicops-inl.h
		src/win/handle-inl.h
		src/win/internal.h
		src/win/req-inl.h
		src/win/stream-inl.h
		src/win/winapi.h
		src/win/winsock.h
	)
endif()

add_library(libuv ${libuv_sources} ${libuv_headers})
