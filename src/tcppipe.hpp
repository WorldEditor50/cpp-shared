#ifndef TCPPIPE_HPP
#define TCPPIPE_HPP
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <atomic>
#include <memory>
#include <iostream>
#include <regex>
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

#ifdef __linux__
class UnixSocket //: public Socket
{
public:
    using SocketPtr = std::shared_ptr<UnixSocket>;
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
    int tcp()
    {
        fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (fd < 0) {
            throw std::runtime_error("failed to create tcp socket");
            return fd;
        }
        return OK;
    }
    int udp()
    {
        fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (fd < 0) {
            throw std::runtime_error("failed to create udp socket");
            return fd;
        }
        return OK;
    }

    int icmp()
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

    int setIOReused(bool on)
    {
        int opt = on == true ? 1 : 0;
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

    std::shared_ptr<UnixSocket> accepting()
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
#endif

class WinSocket
{
public:
    using SocketPtr = std::shared_ptr<WinSocket>;
};
class Protocol
{
public:
    static std::map<std::string, std::string> parse(const std::string &message)
    {
        std::map<std::string, std::string> data;
        std::regex re(",");
        std::vector<std::string> dataVec(std::sregex_token_iterator(message.begin(),
                                                                    message.end(),
                                                                    re,
                                                                    -1),
                                         std::sregex_token_iterator());
        int start = dataVec[0].find("service:");
        data["service"] = dataVec[0].substr(start);
        start = dataVec[1].find("from:");
        data["from"] = dataVec[1].substr(start);
        start = dataVec[2].find("to:");
        data["to"] = dataVec[2].substr(start);
        start = dataVec[2].find("content:");
        data["content"] = dataVec[2].substr(start);
        return data;
    }
    template<typename ...T>
    static std::string append(const T& ...t)
    {
        std::string message;
        return message;
    }
    static std::string build(const std::vector<std::string> &data)
    {
        std::string message;
        message += "service:" + data[0] + ",";
        message += "from:" + data[1] + ",";
        message += "to:" + data[2] + ",";
        message += "content:" + data[3];
        return message;
    }
};

#define MAX_EVENT_NUM 2048
template<typename TSocket>
class AbstractServer
{
public:
    using SocketPtr = typename TSocket::SocketPtr;
    TSocket socket;
    std::atomic_bool isRunning;
    std::map<std::string, SocketPtr> socketMap;
public:
    AbstractServer():isRunning(false){}
    ~AbstractServer()
    {
        socket.destroy();
        for (auto it = socketMap.begin(); it != socketMap.end(); it++) {
             it->second->destroy();
        }
    }
    AbstractServer& start(const std::string &host, int port)
    {
        if (isRunning.load()) {
            return *this;
        }
        /* create socket */
        int fd = socket.tcp();
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
};

template<typename TSocket>
class Select : public AbstractServer<TSocket>
{
public:
    using SocketPtr = typename TSocket::SocketPtr;
    using Server = AbstractServer<TSocket>;
    std::thread recvThread;
public:
    Select(){}
    ~Select()
    {
        Server::isRunning.store(false);
        recvThread.join();
    }
    void run()
    {
        if (Server::isRunning.load()) {
            return;
        }
        Server::isRunning.store(true);
        recvThread = std::thread(&Select::receive, this);
        while (Server::isRunning.load()) {
            fd_set fdSet;
            FD_ZERO(&fdSet);
            FD_SET(Server::socket.fd, &fdSet);
            int fdCount = select(Server::socket.fd + 1, &fdSet, nullptr, nullptr, nullptr);
            if (fdCount < 0) {
                std::runtime_error("select failed. fd count < 0");
                return;
            }
            if (fdCount == 0) {
                std::runtime_error("select:zero count.");
                continue;
            }
            auto newSocket = Server::socket.accepting();
            if (newSocket != nullptr) {
                if (Server::socketMap.size() > FD_SETSIZE) {
                    newSocket->destroy();
                    std::runtime_error("select full.");
                    return;
                }
                std::cout<<"new connection:"<<newSocket->getAddress()<<std::endl;
                newSocket->setNonBlock(true);
                Server::socketMap.insert(std::pair<std::string, SocketPtr>(newSocket->getAddress(), newSocket));
            }
        }
        std::cout<<"accept leave"<<std::endl;
    }

    void receive()
    {
        while (Server::isRunning.load()) {
            for (auto it = Server::socketMap.begin(); it != Server::socketMap.end(); it++) {
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
};
#ifdef __linux__
template<typename TSocket>
class Epoll : public AbstractServer<TSocket>
{
public:
    using SocketPtr = typename TSocket::SocketPtr;
    using Server = AbstractServer<TSocket>;
    int epfd;
    std::map<int, SocketPtr> fd2Socket;
    epoll_event eventList[MAX_EVENT_NUM];
public:
    Epoll(){}
    ~Epoll(){}
    void run()
    {
        if (Server::isRunning.load()) {
            return;
        }
        Server::isRunning.store(true);
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
        ev.data.fd = Server::socket.fd;
        ev.events = EPOLLIN;
        int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, Server::socket.fd, &ev);
        if (ret < 0) {
            close(epfd);
            std::runtime_error("epoll_ctl failed.");
            return;
        }
        while (Server::isRunning.load()) {
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
                if (eventList[i].data.fd == Server::socket.fd) {
                    /* accept */
                    SocketPtr newSocket = Server::socket.accepting();
                    if (newSocket == nullptr) {
                        continue;
                    }
                    std::cout<<"new connection:"<<newSocket->getAddress()<<std::endl;
                    newSocket->setNonBlock(false);
                    Server::socketMap.insert(std::pair<std::string, SocketPtr>(newSocket->getAddress(), newSocket));
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
            ev.data.fd = Server::socket.fd;
            ev.events = EPOLLIN;
            epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
            fd2Socket[fd]->destroy();
            auto it = fd2Socket.find(fd);
            fd2Socket.erase(it);
        }
        /* routing */
    }
};
#endif

template <typename TSocket, template<typename> class IO, typename TProtocol = Protocol>
class TcpServer : public IO<TSocket>
{
public:
    using StringVec = std::vector<std::string>;
    std::thread ioThread;
public:
    TcpServer(){}
    ~TcpServer()
    {
        ioThread.join();
    }
    TcpServer& start(const std::string &host, int port)
    {
        IO<TSocket>::Server::start(host, port);
        return *this;
    }
    void run()
    {
        if (IO<TSocket>::Server::isRunning.load()) {
            return;
        }
        ioThread = std::thread(&IO<TSocket>::run, this);
    }
};

template<typename TSocket, typename TProtocol = Protocol>
class TcpClient
{
public:
    using StringVec = std::vector<std::string>;
public:
    TSocket socket;
    std::atomic_bool isRunning;
    std::thread recvThread;
    std::map<std::string, StringVec> mailBox;
public:
    TcpClient(){}
    ~TcpClient()
    {
        isRunning.store(false);
        recvThread.join();
    }

    TcpClient& connectHost(const std::string &host, int port)
    {
        /* create socket */
        int fd = socket.tcp();
        if (fd < 0) {
            return *this;
        }
        /* connect */
        int ret = socket.connectTo(host, port);
        if (ret != 0) {
            socket.destroy();
            return *this;
        }
        isRunning.store(true);
        std::cout<<"client start"<<std::endl;
        recvThread = std::thread(&TcpClient::receive, this);
        return *this;
    }

    void send(const std::string &destination, const std::string &data)
    {
        std::string msg = TProtocol::build(std::vector<std::string>{"forward",
                                           socket.getAddress(),
                                           destination,
                                           data});
        socket.sendMessage(msg);
    }

    void send(const std::string &data)
    {
        socket.sendMessage(data);
    }

    void receive()
    {
        std::cout<<"client enter"<<std::endl;
        while (isRunning.load()) {
            std::string content = socket.recvMessage();
            if (content.empty() == false) {
                std::cout<<"message:"<<content<<std::endl;
            }
        }
        std::cout<<"client leave"<<std::endl;
        return;
    }
};

template <typename TSocket>
class TcpPipe : protected TcpClient<TSocket>
{
public:
    static std::string host;
    static int port;
#ifdef __linux__
    static TcpServer<TSocket, Epoll> server;
#endif
#ifdef WIN32
    static TcpServer<TSocket, Select> server;
#endif
    TcpPipe()
    {
        server.start(host, port).run();
        TcpClient<TSocket>::connectHost(host, port);
    }
    void push(const std::string &message) {TcpClient<TSocket>::send(message);}
};
template<typename TSocket>
std::string TcpPipe<TSocket>::host = "127.0.0.1";
template<typename TSocket>
int TcpPipe<TSocket>::port = 8081;
#ifdef __linux__
template<typename TSocket>
TcpServer<TSocket, Epoll> TcpPipe<TSocket>::server;
#endif

#ifdef WIN32
template<typename TSocket>
TcpServer<TSocket, Select> TcpPipe<TSocket>::server;
#endif
#endif // TCPPIPE_HPP
