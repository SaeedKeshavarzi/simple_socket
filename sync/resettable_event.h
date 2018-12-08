#ifndef _RESETTABLE_EVENT_H_
#define _RESETTABLE_EVENT_H_

#include <condition_variable>
#include <atomic>

#include "spin_lock.h"

class resettable_event
{
protected:
	std::condition_variable_any cv;
	std::atomic<bool> state;
	spin_lock sync;

public:
	resettable_event(const bool initial_state) : state{ initial_state } { }

	inline bool is_set() const
	{
		return state.load();
	}

	inline void set()
	{
		std::lock_guard<spin_lock> lock(sync);

		if (!state.exchange(true))
		{
			cv.notify_all();
		}
	}

	inline void reset()
	{
		std::lock_guard<spin_lock> lock(sync);
		state = false;
	}

	virtual inline void wait() = 0;

	inline bool wait_for(const std::chrono::milliseconds& rel_time)
	{
		return this->wait_until(std::chrono::steady_clock::now() + rel_time);
	}

	virtual inline bool wait_until(const std::chrono::time_point<std::chrono::steady_clock>& /*timeout_time*/) = 0;
};

class manual_reset_event : public resettable_event
{
public:
	manual_reset_event(const manual_reset_event&) = delete;
	manual_reset_event& operator=(const manual_reset_event&) = delete;

	explicit manual_reset_event(const bool initial_state = false) : resettable_event{ initial_state } { }

	inline void wait()
	{
		std::unique_lock<spin_lock> lock(sync);

		while (!state.load())
		{
			cv.wait(lock);
		}
	}

	inline bool wait_until(const std::chrono::time_point<std::chrono::steady_clock>& timeout_time)
	{
		std::unique_lock<spin_lock> lock(sync);

		while (!state.load())
		{
			if (cv.wait_until(lock, timeout_time) == std::cv_status::timeout)
			{
				return state.load();
			}
		}

		return true;
	}
};

class auto_reset_event : public resettable_event
{
public:
	auto_reset_event(const auto_reset_event&) = delete;
	auto_reset_event& operator=(const auto_reset_event&) = delete;

	explicit auto_reset_event(const bool initial_state = false) : resettable_event{ initial_state } { }

	inline void wait()
	{
		std::unique_lock<spin_lock> lock(sync);

		while (!state.exchange(false))
		{
			cv.wait(lock);
		}
	}

	inline bool wait_until(const std::chrono::time_point<std::chrono::steady_clock>& timeout_time)
	{
		std::unique_lock<spin_lock> lock(sync);

		while (!state.exchange(false))
		{
			if (cv.wait_until(lock, timeout_time) == std::cv_status::timeout)
			{
				return state.exchange(false);
			}
		}

		return true;
	}
};

#endif // !_RESETTABLE_EVENT_H_
