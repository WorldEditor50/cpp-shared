#ifndef TCPHANDLER_H
#define TCPHANDLER_H
#include <functional>
#include "packet.h"
#include "socket.hpp"

enum TcpMessageType {
    TcpMessage_Forward = 0,
    TcpMessage_Show
};

class TcpServer;
using FnTcpServerHandler = std::function<int(TcpServer*, SocketPtr, Packet*)>;
class TcpClient;
using FnTcpClientHandler = std::function<int(TcpClient*, Packet *packet)>;

int TcpServer_Forward(TcpServer *server, SocketPtr client, Packet *packet);
int TcpServer_Show(TcpServer *server, SocketPtr client, Packet *packet);
int TcpClient_Show(TcpClient *client, Packet *packet);
#endif // TCPHANDLER_H

