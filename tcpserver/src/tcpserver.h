#ifndef TCPSERVER_H
#define TCPSERVER_H
#include <vector>
#include <map>
#include <atomic>
#include <thread>
#include <string>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <list>
#include "serverhandler.h"
#include "socket.hpp"


class ObjectPool
{
private:
    int maxObjectCount;
    std::mutex mutex;
    std::list<Packet*> pool;
public:
    ObjectPool():maxObjectCount(1024){}
    explicit ObjectPool(int count):maxObjectCount(count){}
    ~ObjectPool()
    {
        clear();
    }

    Packet* get()
    {
        std::lock_guard<std::mutex> guard(mutex);
        Packet* obj = nullptr;
        if (pool.size() > maxObjectCount) {
            obj = new Packet();
        } else {
            if (pool.empty()) {
                obj = new Packet();
            } else {
                obj = std::move(pool.front());
                pool.pop_front();
            }
        }
        memset((void*)obj, 0, sizeof(Packet));
        return obj;
    }

    void push(Packet* &obj)
    {
        if (obj == nullptr) {
            return;
        }
        std::lock_guard<std::mutex> guard(mutex);
        if (pool.size() > maxObjectCount) {
            delete obj;
            obj = nullptr;
        } else {
            pool.push_back(obj);
        }
        return;
    }

    void clear()
    {
        std::lock_guard<std::mutex> guard(mutex);
        for (auto it = pool.begin(); it != pool.end(); it++) {
            Packet* obj = *it;
            delete [] obj;
        }
        return;
    }
};

class TcpServer
{
public:
    enum State {
        STATE_NONE = 0,
        STATE_IDEL,
        STATE_DATAREADY,
        STATE_HANDLING,
        STATE_TERMINATE
    };
private:
    Socket socket;
    std::atomic<int> isRunning;
    std::map<std::string, SocketPtr> clients;
    std::map<int, FnTcpServerHandler> router;
    int state;
    std::mutex mutex;
    std::condition_variable condit;
    std::thread processThread;
    std::queue<Packet*> dataQueue;
    ObjectPool objectPool;
private:
    int run();
    void process();
    int init(int port, int maxListenCount);
public:
    TcpServer();
    ~TcpServer();
    int start(int port, int maxListenCount);
    int stop();
    void broadcast(unsigned char* data, unsigned long dataSize);
    SocketPtr getClient(const char *name);
};

#endif // TCPSERVER_H
