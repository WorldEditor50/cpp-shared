#ifndef PIPELINE_H
#define PIPELINE_H

#include <queue>
#include <map>
#include <memory>
#include <mutex>
#include <array>
#include <condition_variable>
#include <functional>
#include <thread>


template<typename T>
class Pipeline
{
public:
    enum State {
        STATE_NONE = 0,
        STATE_IDEL,
        STATE_READY,
        STATE_TERMINATE
    };
    using Type = typename T::Type;
    using Func = std::function<void(T&)>;
    constexpr static int max_thread_num = 4;
    constexpr static int max_queue_len = 256;
protected:
    std::queue<T> dataQueue;
    std::map<Type, Func> mapper;
    std::mutex mutex;
    std::array<std::thread, max_thread_num> threads;
    std::condition_variable condit;
    State state;
public:
    Pipeline():state(STATE_NONE){}
    ~Pipeline(){}
    void registerHandler(Type &type, Func func)
    {
        mapper.insert(std::pair<Type, Func>(type, func));
        return;
    }
    void dispatch(T &data)
    {
        std::unique_lock<std::mutex> locker(mutex);
        if (state == STATE_NONE || state == STATE_TERMINATE) {
            return;
        }
        dataQueue.push(data);
        state = STATE_READY;
        condit.notify_one();
        return;
    }
    void start()
    {
        {
            std::unique_lock<std::mutex> locker(mutex);
            if (state != STATE_NONE) {
                return;
            }
            state = STATE_IDEL;
            condit.notify_all();
        }
        for (std::size_t i = 0; i < threads.size(); i++) {
            threads[i] = std::thread(&Pipeline::impl, this);
        }
        return;
    }

    void stop()
    {
        {
            std::unique_lock<std::mutex> locker(mutex);
            state = STATE_TERMINATE;
            condit.notify_all();
        }
        for (std::size_t i = 0; i < threads.size(); i++) {
            threads[i].join();
        }
        state = STATE_NONE;
        return;
    }
protected:
    virtual void impl()
    {
        while (1) {
            T value;
            {
                std::unique_lock<std::mutex> locker(mutex);
                condit.wait(locker, [this](){
                    return state == STATE_READY || state == STATE_TERMINATE;
                });
                if (state == STATE_TERMINATE) {
                    break;
                }
                value = std::move(dataQueue.front());
                dataQueue.pop();
                if (dataQueue.empty() == true) {
                    state = STATE_IDEL;
                    condit.notify_all();
                }
            }
            auto it = mapper.find(value.type);
            if (it == mapper.end()) {
                continue;
            }
            it->second(value);
        }
        return;
    }
};

#endif // PIPELINE_H
