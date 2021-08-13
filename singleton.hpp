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
    Singleton(const Singleton&) = delete;
    Singleton& operator = (const Singleton&) = delete;
public:
    Singleton(){}
    static std::shared_ptr<T> instance()
    {
        if (ptr == nullptr) {
            std::lock_guard<std::mutex> guard(Singleton::mutex);
            if (ptr == nullptr) {
                ptr = std::make_shared<T>();
            }
        }
        return ptr;
    }
};
template <typename T>
std::shared_ptr<T> Singleton<T>::ptr = nullptr;
template <typename T>
std::mutex Singleton<T>::mutex;
#endif // SINGLETON_HPP
