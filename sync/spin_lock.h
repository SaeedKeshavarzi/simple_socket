#ifndef _SPIN_LOCK_H_
#define _SPIN_LOCK_H_

#include <atomic>

class spin_lock
{
private:
	std::atomic_flag flag{ ATOMIC_FLAG_INIT };

public:
	spin_lock() = default;
	spin_lock(const spin_lock&) = delete;
	spin_lock& operator=(const spin_lock&) = delete;

	inline void lock()
	{
		while (flag.test_and_set(std::memory_order_acquire));
	}

	inline void unlock()
	{
		flag.clear(std::memory_order_release);
	}
};

#endif // !_SPIN_LOCK_H_
