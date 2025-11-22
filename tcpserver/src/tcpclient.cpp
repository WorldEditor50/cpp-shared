#include <sys/socket.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include "tcpclient.h"
#include "packet.h"


void TcpClient::run()
{
    unsigned char buf[MAX_BUFFER_LEN];
    while (isRunning.load()) {
        memset(buf, 0, MAX_BUFFER_LEN);
        int len = socket.recvMessage(buf, MAX_BUFFER_LEN);
        if (len <= 0) {
            continue;
        }
        /* parse data */
        Packet *packet = (Packet*)buf;
        int type = packet->type;
        auto it = router.find(type);
        if (it != router.end()) {
            it->second(this, packet);
        }

    }
    return;
}

TcpClient::TcpClient():
    isRunning(1)
{
    router.insert(std::pair<int, FnTcpClientHandler>(TcpMessage_Show, TcpClient_Show));
}

TcpClient::~TcpClient()
{


}

int TcpClient::start(const char* host, int port)
{
    int ret = connectTo(host, port);
    if (ret != 0) {
        printf("failed to connect.\n");
        return -1;
    }
    recvThread = std::thread(&TcpClient::run, this);
    return 0;
}

void TcpClient::stop()
{
    if (isRunning.load()) {
        isRunning.store(0);
        recvThread.join();
    }
    socket.destroy();
    return;
}

void TcpClient::sendMessage(unsigned char* data, unsigned long dataSize)
{
    return socket.sendMessage(data, dataSize);
}

int TcpClient::connectTo(const char* host, int port)
{
    int ret = socket.tcp();
    if (ret < 0) {
        printf("failed to create tcp socket\n");
        return -1;
    }

    ret = socket.connectTo(host, port);
    if (ret != 0) {
        printf("failed to connect to host\n");
        return -2;
    }
    return 0;
}

