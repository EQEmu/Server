#pragma once

#include "../eqemu_logsys.h"
#include <vector>
#include <array>
#include <atomic>
#include <memory>
#include <optional>
#include <mutex>
#include <uv.h>
#include <iostream>

// forward declaration
namespace EQ { namespace Net { class TCPConnection; } }

//constexpr size_t TCP_BUFFER_SIZE = 1024;
constexpr size_t TCP_BUFFER_SIZE = 8192;

struct TCPWriteReq {
	uv_write_t req{};
	std::array<char, TCP_BUFFER_SIZE> buffer{};
	size_t buffer_index{};
	EQ::Net::TCPConnection* connection{};
	uint32_t magic = 0xC0FFEE;
};

class WriteReqPool {
public:
	explicit WriteReqPool(size_t initial_capacity = 512)
		: m_capacity(initial_capacity), m_head(0) {
		m_reqs.reserve(m_capacity);
		m_locks = std::make_unique<std::atomic_bool[]>(m_capacity);

		for (size_t i = 0; i < m_capacity; ++i) {
			m_reqs.emplace_back(std::make_unique<TCPWriteReq>());
			m_locks[i].store(false, std::memory_order_relaxed);
		}
	}

	std::optional<TCPWriteReq*> acquire() {
		size_t start = m_head.fetch_add(1, std::memory_order_relaxed) % m_capacity;

		for (size_t i = 0; i < m_capacity; ++i) {
			size_t index = (start + i) % m_capacity;

			bool expected = false;
			if (m_locks[index].compare_exchange_strong(expected, true, std::memory_order_acquire)) {
				LogNetTCPDetail("[WriteReqPool] Acquired buffer index [{}]", index);
				return m_reqs[index].get();
			}
		}

		grow();
		return acquireAfterGrow();
	}

	void release(TCPWriteReq* req) {
		for (size_t i = 0; i < m_capacity; ++i) {
			if (m_reqs[i].get() == req) {
				m_locks[i].store(false, std::memory_order_release);
				LogNetTCPDetail("[WriteReqPool] Released buffer index [{}]", i);
				return;
			}
		}
		std::cerr << "WriteReqPool::release - Invalid or stale pointer\n";
	}

private:
	std::vector<std::unique_ptr<TCPWriteReq>> m_reqs;
	std::unique_ptr<std::atomic_bool[]> m_locks;
	std::atomic<size_t> m_head;
	size_t m_capacity;
	std::mutex m_grow_mutex;

	void grow() {
		std::lock_guard<std::mutex> lock(m_grow_mutex);

		size_t old_capacity = m_capacity;
		size_t new_capacity = old_capacity * 2;

		m_reqs.reserve(new_capacity);
		for (size_t i = old_capacity; i < new_capacity; ++i) {
			m_reqs.emplace_back(std::make_unique<TCPWriteReq>());
		}

		auto new_locks = std::make_unique<std::atomic_bool[]>(new_capacity);
		for (size_t i = 0; i < old_capacity; ++i) {
			new_locks[i].store(m_locks[i].load(std::memory_order_acquire));
		}
		for (size_t i = old_capacity; i < new_capacity; ++i) {
			new_locks[i].store(false, std::memory_order_relaxed);
		}

		m_locks = std::move(new_locks);
		m_capacity = new_capacity;

		LogNetTCP("[WriteReqPool] Grew pool to capacity [{}] from [{}]", m_capacity, old_capacity);
	}

	std::optional<TCPWriteReq*> acquireAfterGrow() {
		// Try every slot once again after growth
		for (size_t i = 0; i < m_capacity; ++i) {
			bool expected = false;
			if (m_locks[i].compare_exchange_strong(expected, true, std::memory_order_acquire)) {
				LogNetTCP("[WriteReqPool] Acquired buffer index [{}] after grow", i);
				return m_reqs[i].get();
			}
		}
		return std::nullopt;
	}
};
