#ifndef LAZYFSM_H
#define LAZYFSM_H
#include <map>
#include <functional>
#include <tuple>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include <condition_variable>

class Action
{
public:
    virtual int exec() = 0;
};

class Event
{
public:
    int id;
    int state;
    int nextState;
    Action *action;
public:
    Event():id(0), state(0), nextState(0), action(nullptr){}
    Event(int id_,
          int state_,
          int nextSate_,
          Action* action_):
        id(id_),
        state(state_),
        nextState(nextSate_),
        action(action_){}
    virtual ~Event(){}
};

class LazyFSM
{
private:
    enum Status {
        RUNNING = 0,
        WAIT,
        SHUTDOWN
    };
    std::map<int, Event> events;
    int currentEventID;
    int currentState;
    std::thread listenThread;
    std::mutex mutex;
    std::condition_variable condit;
    Status status;
public:
    void registerEvent(int id,
                       int state,
                       int nextSate,
                       Action* action)
    {
        events.insert(std::pair<int, Event>(id, Event(id, state, nextSate, action)));
        return;
    }

    void setEvent(int id)
    {
        if (events.find(id) == events.end()) {
            return;
        }
        std::unique_lock<std::mutex> lock(mutex);
        currentEventID = id;
        if (status != RUNNING) {
            status = RUNNING;
            condit.notify_all();
        }
        return;
    }

    int transit(int nextState)
    {
        int id = 0;
        for (auto& event : events) {
            if (event.second.state == nextState) {
                id = event.first;
                currentState = nextState;
                if (currentState == 0) {
                    status = WAIT;
                }
                break;
            }
        }
        return id;
    }

    void run()
    {
        while (true) {
            std::unique_lock<std::mutex> lock(mutex);
            while (status == WAIT) {
                condit.wait(lock);
            }
            if (status == SHUTDOWN) {
                break;
            }
            int ret = events[currentEventID].action->exec();
            if (ret == 0) {
                currentEventID = transit(events[currentEventID].nextState);
            } else {
                status = WAIT;
            }
        }
        return;
    }

    void start(int state)
    {
        currentState = state;
        status = RUNNING;
        listenThread = std::thread(&LazyFSM::run, this);
        return;
    }
    void stop()
    {
        if (status != SHUTDOWN) {
            std::unique_lock<std::mutex> lock(mutex);
            status = SHUTDOWN;
            listenThread.join();
        }
        return;
    }
    LazyFSM(){}
    ~LazyFSM()
    {
        stop();
    }
};
#endif // LAZYFSM_H
