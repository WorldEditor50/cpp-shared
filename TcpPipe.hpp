#ifndef TCPPIPE_HPP
#define TCPPIPE_HPP
#include <string>
#include <thread>
#include <string>
#include <vector>


class TcpServer
{
public:
    std::vector<std::string> messageCache;
public:
    TcpServer(){}
    void init(const std::string &host, int port);
    virtual void send(const std::string &message);
    virtual std::string recv();
    virtual void receiving();
};

class TcpClient
{
public:
    TcpClient(){}
    virtual void connect(const std::string &host, int port);
    void send(const std::string &message);
    std::string recv();

};

#endif // TCPPIPE_HPP
