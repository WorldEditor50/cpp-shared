#ifndef TCPCLIENT_H
#define TCPCLIENT_H
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <map>
#include <thread>
#include <atomic>
#include "tcphandler.h"
#include "socket.hpp"

class TcpClient
{
private:
    Socket socket;
    std::atomic<int> isRunning;
    std::thread recvThread;
    std::map<int, FnTcpClientHandler> router;
private:
    void run();
    int connectTo(const char* host, int port);
public:
    TcpClient();
    ~TcpClient();
    int start(const char* host, int port);
    void stop();
    void sendMessage(unsigned char* data, unsigned long dataSize);
    Socket& getSocket() {return socket;}
};

#endif // TCPCLIENT_H
