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
    void init(const std::string &host, int port)
    {
        /* create socket */

        /* bind */

        /* listen */

        /* accept */

    }
    void send(const std::string &message)
    {

    }
    std::string recv()
    {

    }
    static void receiving()
    {

    }
};

class TcpClient
{
public:
    TcpClient(){}
    void connect(const std::string &host, int port)
    {

    }
    void send(const std::string &message)
    {

    }
    std::string recv()
    {

    }

};

#endif // TCPPIPE_HPP
