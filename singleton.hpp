#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#include <iostream>
#include <memory>
#include <mutex>

template <typename T>
class Singleton
{
private:
    static std::shared_ptr<T> ptr;
    static std::mutex mutex;
    class Collect
    {
    public:
        ~Collect()
        {
            std::lock_guard<std::mutex> guard(Singleton::mutex);
            if (ptr != nullptr) {
                delete ptr;
                ptr = nullptr;
            }
        }
    };
    static Collect collect;
    Singleton(const Singleton&) = delete;
    Singleton& operator = (const Singleton&) = delete;
public:
    Singleton(){}
    static std::shared_ptr<T> instance()
    {
        std::lock_guard<std::mutex> guard(Singleton::mutex);
        if (ptr == nullptr) {
            ptr.reset(new T);
        }
        return ptr;
    }
};
template <typename T>
std::shared_ptr<T> Singleton<T>::ptr = nullptr;
template <typename T>
std::mutex Singleton<T>::mutex;
template <typename T>
typename Singleton<T>::Collect Singleton<T>::collect;
#endif // SINGLETON_HPP
