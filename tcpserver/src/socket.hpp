#ifndef SOCKET_HPP
#define SOCKET_HPP
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <cstring>
#include <cstdio>
#include <string>
#include <memory>
#include <stdexcept>

#define MAX_PAYLOAD_LEN 1024
class Socket;
using SocketPtr = std::shared_ptr<Socket>;

class Socket
{
public:
public:
    int fd;
    unsigned int port;
    char host[NI_MAXHOST];
public:
    Socket():fd(-1),port(0)
    {
        memset(host, 0, NI_MAXHOST);
    }

    explicit Socket(int newFD, const char* newHost, const char* newPort)
        :fd(newFD), port(std::atoi(newPort))
    {
        memcpy(host, newHost, NI_MAXHOST);
    }

    Socket(const Socket &r):fd(r.fd),port(r.port)
    {
        memcpy(host, r.host, NI_MAXHOST);
    }

    ~Socket(){}

    int tcp()
    {
        fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (fd < 0) {
            throw std::runtime_error("failed to create tcp socket");
            return -1;
        }
        return 0;
    }

    int udp()
    {
        fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (fd < 0) {
            throw std::runtime_error("failed to create udp socket");
            return -1;
        }
        return 0;
    }

    int icmp()
    {
        fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
        if (fd < 0) {
            throw std::runtime_error("failed to create icmp socket");
            return -1;
        }
        return 0;
    }

    int bindTo(int port)
    {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        return bind(fd, (sockaddr*)&addr, sizeof(struct sockaddr_in));
    }

    int setIoReuse(int opt)
    {
        return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }

    void setNonBlock(bool on)
    {
        int flags = fcntl(fd, F_GETFL, 0);
        if (on) {
            flags |= O_NONBLOCK;
        } else {
            flags = flags &~ O_NONBLOCK;
        }
        fcntl(fd, F_SETFL, flags);
        return;
    }

    int connectTo(const char* host, int port)
    {
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_aton(host, &addr.sin_addr);
        return connect(fd, (sockaddr*)&addr, sizeof(sockaddr_in));
    }

    int listening(int maxPortCount)
    {
        return listen(fd, maxPortCount);
    }

    void sendMessage(const std::string &content)
    {
        if (content.size() < MAX_PAYLOAD_LEN) {
            send(fd, content.c_str(), content.size(), 0);
            return;
        }
        const char *data = content.data();
        std::string::size_type total = content.size();
        std::string::size_type pos = 0;
        while (pos < total) {
            int len = send(fd, data + pos, MAX_PAYLOAD_LEN, 0);
            if (len < 0) {
                break;
            }
            pos += len;
        }
        return;
    }

    void sendMessage(unsigned char* data, unsigned long dataSize)
    {
        unsigned long pos = 0;
        unsigned long res = 0;
        int bufLen = 0;
        int len = 0;
        while (pos < dataSize) {
            res = dataSize - pos;
            bufLen = res >= MAX_PAYLOAD_LEN ? MAX_PAYLOAD_LEN : (MAX_PAYLOAD_LEN - res);
            int len = send(fd, data + pos, bufLen, 0);
            if (len > 0) {
                pos += len;
            }
        }
        return;
    }

    std::string recvMessage()
    {
        char buffer[MAX_PAYLOAD_LEN] = {0};
        ssize_t len = recv(fd, buffer, MAX_PAYLOAD_LEN, 0);
        if (len < 0) {
            return std::string();
        }
        return std::string(buffer, len);
    }

    ssize_t recvMessage(unsigned char* data, unsigned long dataSize)
    {
        return recv(fd, data, dataSize, 0);
    }

    SocketPtr accepting()
    {
        struct sockaddr_in addr;
        socklen_t len = sizeof(struct sockaddr_in);
        int newFD = accept(fd, (sockaddr*)&addr, &len);
        if (newFD < 0) {
            return nullptr;
        }
        char newHost[NI_MAXHOST] = {0};
        char newPort[NI_MAXSERV] = {0};
        int ret = getnameinfo((sockaddr*)&addr,
                              len,
                              newHost,
                              NI_MAXHOST,
                              newPort,
                              NI_MAXSERV,
                              NI_NUMERICHOST | NI_NUMERICSERV);
        if (ret != 0) {
            return nullptr;
        }
        printf("accept new connection, ip:%s, port:%s\n", newHost, newPort);
        return std::make_shared<Socket>(newFD, newHost, newPort);
    }

    void destroy()
    {
        if (fd > 0) {
            close(fd);
            fd = -1;
        }
        port = 0;
        memset(host, 0, NI_MAXHOST);
        return;
    }

    std::string getName()
    {
        return std::string(host) + ":" + std::to_string(port);
    }
};

#endif // SOCKET_HPP
