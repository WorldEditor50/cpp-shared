#ifndef SERVERHANDLER_H
#define SERVERHANDLER_H
#include <functional>
#include "packet.h"

class TcpServer;
using FnTcpServerHandler = std::function<int(TcpServer*, Packet*)>;

int TcpServer_Forward(TcpServer *server, Packet *packet);
int TcpServer_Show(TcpServer *server, Packet *packet);
#endif // TCPHANDLER_H

