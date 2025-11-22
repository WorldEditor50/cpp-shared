#include "tcpserver.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <cstdio>
#include <cstring>
#include "packet.h"


int TcpServer::run()
{
    int maxFD;
    fd_set fdset;
    fd_set fdset_;
    unsigned char buf[MAX_BUFFER_LEN];
    FD_ZERO(&fdset);
    FD_SET(socket.fd, &fdset);
    maxFD = socket.fd;
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    while (isRunning.load()) {
        fdset_ = fdset;
        int num = select(maxFD + 1, &fdset_, nullptr, nullptr, &tv);
        if (num < 0) {
            //printf("select failed. fd count < 0\n");
            continue;
        } else if (num == 0) {
            // printf("select timeout\n");
            continue;
        }

        if (FD_ISSET(socket.fd, &fdset_)) {
            /* new connection */
            SocketPtr newSocket = socket.accepting();
            if (newSocket == nullptr) {
                return -1;
            }
            newSocket->setNonBlock(true);
            /* add fdset */
            FD_SET(newSocket->fd, &fdset);
            if (maxFD < newSocket->fd) {
                maxFD = newSocket->fd;
            }
            std::string name = newSocket->getName();
            clients.insert(std::pair<std::string, SocketPtr>(name, newSocket));
        }
        /* receive data */
        for (auto it = clients.begin(); it != clients.end(); it++) {
            SocketPtr clientSocket = it->second;
            if (clientSocket->fd < 0) {
                it = clients.erase(it);
                continue;
            }
            if (!FD_ISSET(clientSocket->fd, &fdset_)) {
                continue;
            }
            memset(buf, 0, MAX_BUFFER_LEN);
            int len = clientSocket->recvMessage(buf, MAX_BUFFER_LEN);
            if (len <= 0) {
                FD_CLR(clientSocket->fd, &fdset);
                clientSocket->destroy();
                continue;
            }
            /* parse data */
            Packet *packet = (Packet*)buf;
            int type = packet->type;
            auto handler = router.find(type);
            if (handler != router.end()) {
                handler->second(this, clientSocket, packet);
            }
        }
    }
    return 0;
}

TcpServer::TcpServer():
    isRunning(1)
{
    router.insert(std::pair<int, FnTcpServerHandler>(TcpMessage_Show, TcpServer_Show));
}

TcpServer::~TcpServer()
{

}

int TcpServer::init(int port, int maxListenCount)
{
    /* create socket */
    int ret = socket.tcp();
    if (ret < 0) {
        printf("fail to create socket\n");
        return -1;
    }
    /* set io reused */
    socket.setIoReuse(1);
    /* bind */
    ret = socket.bindTo(port);
    if (ret != 0) {
        printf("bind error, code=%d\n", ret);
        socket.destroy();
        return -2;
    }
    /* listen */
    ret = socket.listening(maxListenCount);
    if (ret != 0) {
        printf("Listen failed with error: %d\n", ret);
        socket.destroy();
        return -3;
    }
    return 0;
}

int TcpServer::start(int port, int maxListenCount)
{
    int ret = init(port, maxListenCount);
    if (ret != 0) {
        return -1;
    }
    return run();
}

int TcpServer::stop()
{
    if (isRunning.load()) {
        isRunning.store(0);
    }
    socket.destroy();
    for (auto it = clients.begin(); it != clients.end(); it++) {
        SocketPtr clientSocket = it->second;
        clientSocket->destroy();
    }
    return 0;
}

void TcpServer::broadcast(unsigned char* data, unsigned long dataSize)
{
    for (auto it = clients.begin(); it != clients.end(); it++) {
        SocketPtr clientSocket = it->second;
        clientSocket->sendMessage(data, dataSize);
    }
    return;
}
