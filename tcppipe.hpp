#ifndef TCPPIPE_HPP
#define TCPPIPE_HPP
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <atomic>
#include <memory>
#include <iostream>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

enum Error {
    OK = 0,
    SOCKET_ERR,
    BIND_ERR,
    CONNECT_ERR,

};
class UnixSocket //: public Socket
{
public:
    constexpr static int maxBufferLen = 65532;
    int fd;
    int port_;
    std::string host_;
public:
    UnixSocket():fd(-1), port_(8080), host_("127.0.0.1"){}
    UnixSocket(int newFD, const std::string &host, const std::string &port)
        :fd(newFD), port_(std::atoi(port.c_str())), host_(host) {}
    UnixSocket(const UnixSocket &sock):fd(sock.fd), port_(sock.port_), host_(sock.host_){}
    ~UnixSocket(){}
    inline int getFD() const {return fd;}
    inline int getPort() const {return port_;}
    inline std::string getHost() const {return host_;}
    inline std::string getAddress() const
    {
        return host_ + ":" + std::to_string(port_);
    }
    int tcpSocket()
    {
        fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (fd < 0) {
            throw std::runtime_error("failed to create tcp socket");
            return fd;
        }
        return OK;
    }
    int udpSocket()
    {
        fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (fd < 0) {
            throw std::runtime_error("failed to create udp socket");
            return fd;
        }
        return OK;
    }

    int icmpSocket()
    {
        fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
        if (fd < 0) {
            throw std::runtime_error("failed to create icmp socket");
            return fd;
        }
        return OK;
    }

    int bindTo(const std::string &host, int port)
    {
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_aton(host.c_str(), &addr.sin_addr);
        return bind(fd, (sockaddr*)&addr, sizeof(sockaddr_in));
    }

    int setIOReused(int opt)
    {
        return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }

    void setNonBlock(bool on)
    {
        int flags = fcntl(fd, F_GETFL, 0);
        if (on == true) {
            flags |= O_NONBLOCK;
        } else {
            flags = flags &~ O_NONBLOCK;
        }
        fcntl(fd, F_SETFL, flags);
        return;
    }

    int connectTo(const std::string &host, int port)
    {
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_aton(host.c_str(), &addr.sin_addr);
        return connect(fd, (sockaddr*)&addr, sizeof(sockaddr_in));
    }
    int listening(int maxPortCount)
    {
        return listen(fd, maxPortCount);
    }
    bool sendMessage(const std::string &content)
    {
        if (content.size() < maxBufferLen) {
            return send(fd, content.c_str(), content.size(), 0) == content.size();
        }
        const char *data = content.data();
        std::string::size_type total = content.size();
        std::string::size_type pos = 0;
        while (pos < total) {
            ssize_t len = send(fd, data + pos, maxBufferLen, 0);
            if (len < 0) {
                break;
            }
            pos += len;
        }
        return pos == total;
    }

    std::string recvMessage()
    {
        char buffer[maxBufferLen] = {0};
        ssize_t len = recv(fd, buffer, maxBufferLen, 0);
        if (len < 0) {
            return std::string();
        }
        return std::string(buffer, len);
    }

    std::shared_ptr<UnixSocket> acceptSocket()
    {
        sockaddr_in addr;
        socklen_t len = sizeof(sockaddr_in);
        int newFD = accept(fd, (sockaddr*)&addr, &len);
        if (newFD < 0) {
            return nullptr;
        }
        char acHost[NI_MAXHOST] = {0};
        char acPort[NI_MAXSERV] = {0};
        int ret = getnameinfo((sockaddr*)&addr,
                              len,
                              acHost,
                              NI_MAXHOST,
                              acPort,
                              NI_MAXSERV,
                              NI_NUMERICHOST | NI_NUMERICSERV);
        if (ret != 0) {
            return nullptr;
        }
        return std::make_shared<UnixSocket>(newFD, acHost, acPort);
    }
    void destroy()
    {
        close(fd);
        fd = -1;
    }
};

class WinSocket
{
public:

};

#define MAX_EVENT_NUM 2048
template <typename TSocket>
class TcpPipe
{
public:
    enum IOType {
        MODEL_SELECT = 0,
        MODEL_EPOLL
    };
    using StringVec = std::vector<std::string>;
    using SocketPtr = std::shared_ptr<TSocket>;
private:
    IOType ioType;
    int epfd;
    epoll_event eventList[MAX_EVENT_NUM];
    TSocket socket;
    std::string address_;
    std::atomic_bool isRunning;
    std::thread recvThread;
    std::map<std::string, StringVec> mailBox;
    std::map<std::string, SocketPtr> socketMap;
    std::map<int, SocketPtr> fd2Socket;
public:
    TcpPipe(){}
    ~TcpPipe()
    {
        isRunning.store(false);
        if (ioType == MODEL_SELECT) {
            recvThread.join();
        }
        for (auto it = socketMap.begin(); it != socketMap.end(); it++) {
             it->second->destroy();
        }
    }

    TcpPipe& sever(const std::string &host, int port)
    {
        /* create socket */
        int fd = socket.tcpSocket();
        if (fd < 0) {
            std::runtime_error("fail to create socket");
            return *this;
        }
        /* set io reused */
        socket.setIOReused(1);
        /* bind */
        int ret = socket.bindTo(host, port);
        if (ret != 0) {
            socket.destroy();
            std::runtime_error("fail to bind");
            return *this;
        }
        /* listen */
        ret = socket.listening(1024);
        if (ret != 0) {
            socket.destroy();
            std::runtime_error("fail to listen");
        }
        return *this;
    }

    TcpPipe& client(const std::string& host, int port)
    {
        /* create socket */
        int fd = socket.tcpSocket();
        if (fd < 0) {
            return *this;
        }
        /* connect */
        int ret = socket.connectTo(host, port);
        if (ret != 0) {
            socket.destroy();
        }
        return *this;
    }

    void post(const std::string &address, const std::string &content)
    {
        if (socketMap.find(address) == socketMap.end()) {
            return;
        }
        socketMap[address]->sendMessage(content);
        return;
    }

    StringVec get(const std::string &address)
    {
        return mailBox[address];
    }

    void syncRun()
    {
        ioType = MODEL_SELECT;
        isRunning.store(true);
        recvThread = std::thread(&TcpPipe::syncReceive, this);
        while (isRunning.load()) {
            fd_set fdSet;
            FD_ZERO(&fdSet);
            FD_SET(socket.fd, &fdSet);
            int fdCount = select(socket.fd + 1, &fdSet, nullptr, nullptr, nullptr);
            if (fdCount < 0) {
                std::runtime_error("select failed. fd count < 0");
                return;
            }
            if (fdCount == 0) {
                std::runtime_error("select:zero count.");
                continue;
            }
            SocketPtr newSocket = socket.acceptSocket();
            if (newSocket != nullptr) {
                if (socketMap.size() > FD_SETSIZE) {
                    newSocket->destroy();
                    std::runtime_error("select full.");
                    return;
                }
                std::cout<<"new connection:"<<newSocket->getAddress()<<std::endl;
                newSocket->setNonBlock(true);
                socketMap.insert(std::pair<std::string, SocketPtr>(newSocket->getAddress(), newSocket));
            }
        }
        std::cout<<"accept leave"<<std::endl;
        return;
    }
    void syncReceive()
    {
        while (isRunning.load()) {
            for (auto it = socketMap.begin(); it != socketMap.end(); it++) {
                fd_set fdSet;
                FD_ZERO(&fdSet);
                FD_SET(it->second->fd, &fdSet);
                struct timeval tv;
                tv.tv_sec = 1;
                tv.tv_usec = 500;
                int fdCount = select(it->second->fd + 1, &fdSet, nullptr, nullptr, &tv);
                if (fdCount < 0) {
                    std::runtime_error("select failed. fd count < 0");
                    return;
                }
                if (fdCount == 0) {
                    std::runtime_error("select:zero count.");
                    continue;
                }
                std::string content = it->second->recvMessage();
                if (content.empty() == false) {
                    std::cout<<"from:"<<it->first<<"message:"<<content<<std::endl;
                    //mailBox[it->first].push_back(content);
                } else {
                    it->second->destroy();
                }
                /* routing */
            }
        }
        return;
    }

    void asyncRun()
    {
        ioType = MODEL_EPOLL;
        isRunning.store(true);
        std::cout<<"epoll_create"<<std::endl;
        /* create */
        epfd = epoll_create1(0);
        if (epfd < 0) {
            std::runtime_error("epoll_create failed.");
            return;
        }
        std::cout<<"epoll_ctl"<<std::endl;
        /* add sever fd to epoll */
        struct epoll_event ev;
        ev.data.fd = socket.fd;
        ev.events = EPOLLIN;
        int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, socket.fd, &ev);
        if (ret < 0) {
            close(epfd);
            std::runtime_error("epoll_ctl failed.");
            return;
        }
        while (isRunning.load()) {
            std::cout<<"epoll wait"<<std::endl;
            int fdCount = epoll_wait(epfd, eventList, MAX_EVENT_NUM, -1);
            if (fdCount < 0) {
                if (errno == EINTR) {
                    continue;
                }
                break;
            } else if (fdCount == 0) {
                continue;
            }
            for (int i = 0; i < fdCount; i++) {
                if (eventList[i].events != EPOLLIN) {
                    continue;
                }
                if (eventList[i].data.fd == socket.fd) {
                    /* accept */
                    SocketPtr newSocket = socket.acceptSocket();
                    if (newSocket == nullptr) {
                        continue;
                    }
                    std::cout<<"new connection:"<<newSocket->getAddress()<<std::endl;
                    newSocket->setNonBlock(false);
                    socketMap.insert(std::pair<std::string, SocketPtr>(newSocket->getAddress(), newSocket));
                    fd2Socket.insert(std::pair<int, SocketPtr>(newSocket->fd, newSocket));
                    /* add client fd to epoll */
                    struct epoll_event ev;
                    ev.data.fd = newSocket->fd;
                    ev.events = EPOLLIN;
                    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, newSocket->fd, &ev);
                    if (ret < 0) {
                        close(epfd);
                        std::runtime_error("epoll_ctl failed.");
                    }
                } else {
                    /* receive */
                    receive(i);
                }
            }
        }
        return;
    }

    void receive(int index)
    {
        int fd = eventList[index].data.fd;
        std::string content = fd2Socket[fd]->recvMessage();
        if (content.empty() == false) {
            std::cout<<"message:"<<content<<std::endl;
        } else {
            struct epoll_event ev;
            ev.data.fd = socket.fd;
            ev.events = EPOLLIN;
            epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
            fd2Socket[fd]->destroy();
            auto it = fd2Socket.find(fd);
            fd2Socket.erase(it);
        }
        /* routing */
    }
};

#endif // TCPPIPE_HPP
