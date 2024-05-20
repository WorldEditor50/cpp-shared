#ifndef LAZYALLOCTOR_HPP
#define LAZYALLOCTOR_HPP

#include <map>
#include <vector>
#include <mutex>
#include <condition_variable>

template<typename T>
class LazyAllocator
{
public:
    enum State {
        STATE_ALLOCATING = 0,
        STATE_DEALLOCATING,
        STATE_EMPTING
    };
private:
    constexpr static std::size_t capacity = 256;
    std::mutex mutex;
    std::condition_variable condit;
    std::multimap<std::size_t, T*> container;
public:
    LazyAllocator(){}
    ~LazyAllocator()
    {
        clear();
    }

    T* pop(std::size_t size)
    {
        std::lock_guard<std::mutex> guard(mutex);
        T* ptr = nullptr;
        std::size_t totalSize = size;
        if (totalSize&0x3ff) {
            totalSize = ((totalSize >> 10) + 1)<<10;
        }
        auto it = container.find(totalSize);
        if (it == container.end()) {
            ptr = new T[totalSize];
        } else {
            ptr = it->second;
            container.erase(it);
        }
        return ptr;
    }

    void push(std::size_t size, T* &ptr)
    {
        if (ptr == nullptr || size == 0) {
            return;
        }

        if (container.size() < capacity) {
            std::lock_guard<std::mutex> guard(mutex);
            std::size_t totalSize = size;
            if (totalSize&0x3ff) {
                totalSize = ((totalSize >> 10) + 1)<<10;
            }
            container.insert(std::pair<std::size_t, T*>(totalSize, ptr));
        } else {
            delete [] ptr;
            ptr = nullptr;
        }
        return;
    }

    void clear()
    {
        std::lock_guard<std::mutex> guard(mutex);
        for (auto it = container.begin(); it != container.end(); it++) {
            T* ptr = it->second;
            delete [] ptr;
            ptr = nullptr;
        }
        container.clear();
        return;
    }
};

#endif // LAZYALLOCTOR_HPP
