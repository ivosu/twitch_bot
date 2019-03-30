//
// Created by strejivo on 3/29/19.
//

#ifndef TWITCH_IRC_TST_QUEUE_H
#define TWITCH_IRC_TST_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template <typename T>
class tst_queue {
  public:

	void push(T&& val) {
		m_queue.push(val);
	}

	void push(const T& val) {
		m_queue.push(val);
	}

	template<typename... Args>
	void emplace(Args&&... args) {
		{
			std::lock_guard lock(m_mutex);
			m_queue.emplace(args...);
		}
		m_cond_var.notify_one();
	}

	template<typename _Rep, typename _Period>
	std::optional<T> pop(const std::chrono::duration<_Rep, _Period>& timeout) {
			std::unique_lock lock(m_mutex);
			if (!m_cond_var.wait_for(lock, timeout, [=]() -> bool { return !m_queue.empty(); }))
				return std::nullopt;
			else {
				T tmp(m_queue.front());
				m_queue.pop();
				return tmp;
			}
	}

	T pop() {
		std::unique_lock lock(m_mutex);
		m_cond_var.wait(lock, [=]() -> bool { return !m_queue.empty(); });
		T tmp(m_queue.front());
		m_queue.pop();
		return tmp;
	}

  private:
	std::queue<T> m_queue;
	std::mutex m_mutex;
	std::condition_variable m_cond_var;
};

#endif //TWITCH_IRC_TST_QUEUE_H
