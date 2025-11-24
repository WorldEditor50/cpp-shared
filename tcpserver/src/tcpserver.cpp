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
            Packet *packet = objectPool.get();
            int len = clientSocket->recvMessage((unsigned char*)packet, MAX_BUFFER_LEN);
            if (len <= 0) {
                FD_CLR(clientSocket->fd, &fdset);
                clientSocket->destroy();
                continue;
            }
            //printf("Get data:%s\n", (char*)packet->data);

            memset(packet->src, 0, 32);
            std::string name = clientSocket->getName();
            memcpy(packet->src, name.c_str(), name.size());
            /* queue message */
            std::unique_lock<std::mutex> locker(mutex);
            if (state != STATE_TERMINATE) {
                dataQueue.push(packet);
                state = STATE_DATAREADY;
                condit.notify_one();
            }

        }
    }
    return 0;
}

void TcpServer::process()
{
    while (isRunning.load()) {
        Packet* packet = nullptr;
        {
            std::unique_lock<std::mutex> locker(mutex);
            if (condit.wait_for(locker, std::chrono::milliseconds(3000), [this](){
                        return state == STATE_DATAREADY || state == STATE_TERMINATE;
                        })) {
                packet = std::move(dataQueue.front());
                dataQueue.pop();
                if (dataQueue.empty()) {
                    state = STATE_IDEL;
                    condit.notify_all();
                }
            }
            if (state == STATE_TERMINATE) {
                objectPool.push(packet);
                state = STATE_NONE;
                break;
            }
        }
        if (packet == nullptr) {
            continue;
        }

        /* handle data */
        int type = packet->type;
        auto handler = router.find(type);
        if (handler != router.end()) {
            handler->second(this, packet);
        }

        objectPool.push(packet);
    }
    return;
}

TcpServer::TcpServer():
    isRunning(1),state(STATE_NONE)
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
    state = STATE_IDEL;
    processThread = std::thread(&TcpServer::process, this);
    return run();
}

int TcpServer::stop()
{
    if (isRunning.load()) {
        isRunning.store(0);
        processThread.join();
    }

    socket.destroy();
    for (auto it = clients.begin(); it != clients.end(); it++) {
        SocketPtr clientSocket = it->second;
        clientSocket->destroy();
    }

    while (!dataQueue.empty()) {
        Packet *packet = std::move(dataQueue.front());
        dataQueue.pop();
        objectPool.push(packet);
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

SocketPtr TcpServer::getClient(const char* name)
{
    auto it = clients.find(name);
    if (it == clients.end()) {
        return nullptr;
    }
    return it->second;
}
