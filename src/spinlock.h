#ifndef SPINLOCK_H
#define SPINLOCK_H
#include <atomic>

class SpinLock
{
private:
    std::atomic_flag _lock = ATOMIC_FLAG_INIT;
public:
    SpinLock(){}
    ~SpinLock(){}
    void lock()
    {
        while (_lock.test_and_set(std::memory_order_acquire)) {

        }
    }

    void unlock()
    {
        _lock.clear(std::memory_order_release);
    }
};
#endif // SPINLOCK_H
