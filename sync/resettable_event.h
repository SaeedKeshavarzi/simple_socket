#ifndef _RESETTABLE_EVENT_H_
#define _RESETTABLE_EVENT_H_

#include <mutex>
#include <atomic>
#include <type_traits>
#include <condition_variable>

template<bool _AutoReset>
class resettable_event
{
public:
	static constexpr bool is_auto = _AutoReset;

	resettable_event(const resettable_event&) = delete;
	resettable_event& operator=(const resettable_event&) = delete;

	resettable_event(const bool initial_state = false) :
		state{ initial_state }
	{ }

	inline bool is_set() const
	{
		return state.load();
	}

	inline void set()
	{
		std::lock_guard<std::mutex> lock(guard);

		if (!state.exchange(true))
			cv.notify_all();
	}

	inline void reset()
	{
		std::lock_guard<std::mutex> lock(guard);
		state = false;
	}

	template<typename = std::enable_if_t<!is_auto, bool>>
	inline void wait() const
	{
		std::unique_lock<std::mutex> lock(guard);

		while (!state.load())
			cv.wait(lock);
	}

	template<class _Rep, class _Period, typename = std::enable_if_t<!is_auto, bool>>
	inline bool wait_for(const std::chrono::duration<_Rep, _Period> & rel_time)  const
	{
		std::unique_lock<std::mutex> lock(guard);

		while (!state.load())
		{
			if (cv.wait_for(lock, rel_time) == std::cv_status::timeout)
				return state.load();
		}

		return true;
	}

	template<class _Clock, class _Duration, typename = std::enable_if_t<!is_auto, bool>>
	inline bool wait_until(const std::chrono::time_point<_Clock, _Duration> & timeout_time)  const
	{
		std::unique_lock<std::mutex> lock(guard);

		while (!state.load())
		{
			if (cv.wait_until(lock, timeout_time) == std::cv_status::timeout)
				return state.load();
		}

		return true;
	}

	template<typename = std::enable_if_t<is_auto, bool>>
	inline void wait()
	{
		std::unique_lock<std::mutex> lock(guard);

		while (!state.exchange(false))
			cv.wait(lock);
	}

	template<class _Rep, class _Period, typename = std::enable_if_t<is_auto, bool>>
	inline bool wait_for(const std::chrono::duration<_Rep, _Period> & rel_time)
	{
		std::unique_lock<std::mutex> lock(guard);

		while (!state.exchange(false))
		{
			if (cv.wait_for(lock, rel_time) == std::cv_status::timeout)
				return state.exchange(false);
		}

		return true;
	}

	template<class _Clock, class _Duration, typename = std::enable_if_t<is_auto, bool>>
	inline bool wait_until(const std::chrono::time_point<_Clock, _Duration> & timeout_time)
	{
		std::unique_lock<std::mutex> lock(guard);

		while (!state.exchange(false))
		{
			if (cv.wait_until(lock, timeout_time) == std::cv_status::timeout)
				return state.exchange(false);
		}

		return true;
	}

protected:
	std::atomic_bool state;
	mutable std::mutex guard;
	mutable std::condition_variable cv;
};

#endif // !_RESETTABLE_EVENT_H_
