#ifndef TCPPIPE_HPP
#define TCPPIPE_HPP
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <atomic>
#include <memory>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#endif
class Socket
{
public:
    enum Error {
        OK = 0,
        SOCKET_ERR,
        BIND_ERR,
        CONNECT_ERR,

    };
public:
    Socket(){}
    virtual ~Socket() = 0;
    virtual int getFD() const  = 0;
    virtual int getPort() const  = 0;
    virtual std::string getHost() const  = 0;
    virtual std::string getAddress() const = 0;
    virtual int createSocket() = 0;
    virtual int bindTo(const std::string &host, int port) = 0;
    virtual int connectTo(const std::string &host, int port) = 0;
    virtual int listening(int maxPortCount) = 0;
    virtual bool sendMessage(const std::string &content) = 0;
    virtual std::string recvMessage() = 0;
    virtual std::shared_ptr<Socket> acceptSocket() = 0;
    virtual void clear() = 0;
};

class UnixSocket : public Socket
{
public:
    constexpr static int maxBufferLen = 65532;
private:
    int fd;
    int port_;
    std::string host_;
public:
    UnixSocket():fd(-1), port_(8080), host_("127.0.0.1"){}
    UnixSocket(int newFD, const std::string &host, const std::string &port)
        :fd(newFD), port_(std::atoi(port.c_str())), host_(host) {}
    UnixSocket(const UnixSocket &sock):fd(sock.fd), port_(sock.port_), host_(sock.host_){}
    inline int getFD() const override {return fd;}
    inline int getPort() const override {return port_;}
    inline std::string getHost() const override {return host_;}
    inline std::string getAddress() const override
    {
        return host_ + ":" + std::to_string(port_);
    }
    int createSocket() override
    {
        fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            throw std::runtime_error("failed to create socket");
            return fd;
        }
        return OK;
    }
    int bindTo(const std::string &host, int port) override
    {
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_aton(host.c_str(), &addr.sin_addr);
        return bind(fd, (sockaddr*)&addr, sizeof(sockaddr_in));
    }
    int connectTo(const std::string &host, int port) override
    {
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_aton(host.c_str(), &addr.sin_addr);
        return connect(fd, (sockaddr*)&addr, sizeof(sockaddr_in));
    }
    int listening(int maxPortCount) override
    {
        return listen(fd, maxPortCount);
    }
    bool sendMessage(const std::string &content) override
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
    std::string recvMessage() override
    {
        char buffer[maxBufferLen] = {0};
        ssize_t len = recv(fd, buffer, maxBufferLen, 0);
        if (len < 0) {
            return std::string();
        }
        return std::string(buffer, len);
    }
    std::shared_ptr<Socket> acceptSocket() override
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
    void clear() override
    {
        close(fd);
        fd = -1;
    }
};

class WinSocket : public Socket
{
public:

};


template <typename TSocket>
class TcpPipe
{
public:
    using StringVec = std::vector<std::string>;
    using SocketPtr = std::shared_ptr<TSocket>;
private:
    TSocket socket;
    std::string address_;
    std::atomic_bool isRunning;
    std::map<std::string, StringVec> mailBox;
    std::map<std::string, SocketPtr> socketMap;
public:
    TcpPipe(const std::string &ip, int port)
    {
        address_ = ip + std::to_string(port);
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
    void connect(const std::string &address)
    {

    }
private:
    void accept()
    {
        while (isRunning.load()) {
            SocketPtr newSocket = socket.acceptSocket();
            if (newSocket != nullptr) {
                socketMap.insert(std::make_pair<std::string, SocketPtr>(newSocket.getAddress(), newSocket));
            }
        }
        return;
    }
    void recv()
    {
        while (isRunning.load()) {
            for (auto it = socketMap.begin(); it != socketMap.end(); it++) {
                std::string content = it->recvMessage();
                if (content.empty() == false) {
                    mailBox[it->first].push_back(content);
                }
            }
        }
        return;
    }
};

#endif // TCPPIPE_HPP
