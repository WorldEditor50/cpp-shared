#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H
#include <functional>
#include "packet.h"

class TcpClient;
using FnTcpClientHandler = std::function<int(TcpClient*, Packet*)>;

int TcpClient_Show(TcpClient *client, Packet *packet);
#endif // TCPHANDLER_H

