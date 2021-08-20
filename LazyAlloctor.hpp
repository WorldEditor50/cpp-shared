#ifndef LAZYALLOCTOR_HPP
#define LAZYALLOCTOR_HPP

#include <map>
#include <vector>
#include <mutex>

template <typename T>
class LazyAllocator
{
private:
    class LazyAllocatorImpl
    {
    public:
        std::map<std::size_t, std::vector<T*> > storage;
        std::mutex mutex_;
        LazyAllocatorImpl() = default;
        T* allocate(std::size_t size_)
        {
            if (size_ == 0) {
                return nullptr;
            }
            std::lock_guard<std::mutex> guard(mutex_);
            T* ptr = nullptr;
            if (storage.find(size_) == storage.end()) {
                ptr = new T[size_];
            } else {
                ptr = storage[size_].back();
                storage[size_].pop_back();
            }
            return ptr;
        }
        void deallocate(std::size_t size_, T* &ptr)
        {
            if (size_ == 0 || ptr == nullptr) {
                return;
            }
            std::lock_guard<std::mutex> guard(mutex_);
            storage[size_].push_back(ptr);
            ptr = nullptr;
            return;
        }

        ~LazyAllocatorImpl()
        {
            std::lock_guard<std::mutex> guard(mutex_);
            for (auto& p : storage) {
                auto& elementVec = p.second;
                for (auto& e : elementVec) {
                    delete [] e;
                }
                elementVec.clear();
            }
            storage.clear();
            return;
        }
    };

    static LazyAllocatorImpl impl;
    std::map<std::size_t, std::vector<T*> > temp;
public:
    LazyAllocator() = default;
    ~LazyAllocator()
    {
        for (auto& p : temp) {
            for (auto& e : p.second) {
                impl.deallocate(p.first, e);
            }
        }
        temp.clear();
    }

    T* get(std::size_t size_)
    {
        T* ptr = impl.allocate(size_);
        if (ptr != nullptr) {
            temp[size_].push_back(ptr);
        }
        return ptr;
    }

};

template <typename T>
typename LazyAllocator<T>::LazyAllocatorImpl LazyAllocator<T>::impl;

#endif // LAZYALLOCTOR_HPP
