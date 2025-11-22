#ifndef TCPSERVER_H
#define TCPSERVER_H
#include <vector>
#include <map>
#include <atomic>
#include <thread>
#include <string>
#include <mutex>
#include "tcphandler.h"
#include "socket.hpp"

class TcpServer
{
private:
    Socket socket;
    std::mutex mutex;
    std::atomic<int> isRunning;
    std::map<std::string, SocketPtr> clients;
    std::map<int, FnTcpServerHandler> router;
private:
    int run();
    int init(int port, int maxListenCount);
public:
    TcpServer();
    ~TcpServer();
    int start(int port, int maxListenCount);
    int stop();
    void broadcast(unsigned char* data, unsigned long dataSize);
};

#endif // TCPSERVER_H
