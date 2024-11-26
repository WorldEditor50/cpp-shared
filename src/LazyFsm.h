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

class iAction
{
public:
    virtual int exec() = 0;
};

class Event
{
public:
    using ActionPtr = std::shared_ptr<iAction>;
public:
    int id;
    int state;
    int nextState;
    ActionPtr action;
public:
    Event():id(0), state(0), nextState(0), action(nullptr){}
    Event(int id_, int state_, int nextSate_, ActionPtr action_)
        :id(id_),state(state_),nextState(nextSate_),action(action_){}
    Event(const Event &r)
        :id(r.id),state(r.state),nextState(r.nextState),action(r.action){}
    virtual ~Event(){}
};

class LazyFSM
{
public:
    enum Status {
        FSM_NONE = 0,
        FSM_READY,
        FSM_WAIT,
        FSM_TERMINATE
    };
private:
    int status;
    int currentEventID;
    int currentState;
    std::thread listenThread;
    std::mutex mutex;
    std::condition_variable condit;
    std::map<int, Event> events;
public:
    LazyFSM():status(FSM_NONE){}
    ~LazyFSM()
    {
        stop();
    }
    void registerEvent(int id, int state, int nextSate, Event::ActionPtr action)
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
        if (status != FSM_READY) {
            status = FSM_READY;
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
                    status = FSM_WAIT;
                }
                break;
            }
        }
        return id;
    }

    void run()
    {
        while (true) {
            std::unique_lock<std::mutex> locker(mutex);
            condit.wait(locker, [this](){
                return status == FSM_READY;
            });
            if (status == FSM_TERMINATE) {
                status = FSM_NONE;
                break;
            }
            int ret = events[currentEventID].action->exec();
            if (ret == 0) {
                currentEventID = transit(events[currentEventID].nextState);
            } else {
                status = FSM_WAIT;
            }
        }
        return;
    }

    void start(int state)
    {
        if (status != FSM_NONE) {
            return;
        }
        currentState = state;
        status = FSM_READY;
        listenThread = std::thread(&LazyFSM::run, this);
        return;
    }
    void stop()
    {
        if (status == FSM_NONE) {
            return;
        }
        while (status != FSM_NONE) {
            std::unique_lock<std::mutex> lock(mutex);
            status = FSM_TERMINATE;
            listenThread.join();
        }
        return;
    }

};
#endif // LAZYFSM_H
