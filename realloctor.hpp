#ifndef REALLOCTOR_HPP
#define REALLOCTOR_HPP

#include <map>
#include <vector>
#include <mutex>

template <typename T>
class Reallocator
{
public:
    using Pointer = T*;
private:
    static std::map<std::size_t, std::vector<T*> > arrayPool;
    static std::vector<T*> elementPool;
    static std::mutex mutex_;
private:
    Reallocator() = default;
    ~Reallocator()
    {
        clear();
    }
public:

    static T* get(std::size_t size_)
    {
        if (size_ == 0) {
            return nullptr;
        }
        std::lock_guard<std::mutex> guard(mutex_);
        T* ptr = nullptr;
        if (arrayPool.find(size_) == arrayPool.end()) {
            ptr = new T[size_];
        } else {
            ptr = arrayPool[size_].back();
            arrayPool[size_].pop_back();
        }
        return ptr;
    }
    static void recycle(std::size_t size_, Pointer &ptr)
    {
        if (size_ == 0 || ptr == nullptr) {
            return;
        }
        std::lock_guard<std::mutex> guard(mutex_);
        arrayPool[size_].push_back(ptr);
        ptr = nullptr;
        return;
    }
    static T* get()
    {
        std::lock_guard<std::mutex> guard(mutex_);
        T* ptr = nullptr;
        if (elementPool.empty()) {
            ptr = new T;
        } else {
            ptr = elementPool.back();
            elementPool.pop_back();
        }
        return ptr;
    }
    static void recycle(Pointer &ptr)
    {
        if (ptr == nullptr) {
            return;
        }
        std::lock_guard<std::mutex> guard(mutex_);
        elementPool.push_back(ptr);
        ptr = nullptr;
        return;
    }
    void clear()
    {
        std::lock_guard<std::mutex> guard(mutex_);
        for (auto& block : arrayPool) {
            for (int i = 0; i < block.size(); i++) {
                T* ptr = block.at(i);
                delete [] ptr;
            }
            block.clear();
        }
        arrayPool.clear();
        for (int i = 0; i < elementPool.size(); i++) {
            T* ptr = elementPool.at(i);
            delete ptr;
        }
        elementPool.clear();
        return;
    }
};

template <typename T>
std::map<std::size_t, std::vector<T*> > Reallocator<T>::arrayPool;
template <typename T>
std::vector<T*> Reallocator<T>::elementPool;
template <typename T>
std::mutex Reallocator<T>::mutex_;
#endif // REALLOCTOR_HPP
