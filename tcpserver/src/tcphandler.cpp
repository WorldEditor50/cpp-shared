#include "tcphandler.h"
#include <sys/socket.h>
#include <cstring>
#include <cstdio>
#include "packet.h"


int TcpServer_Forward(TcpServer *server, SocketPtr client, Packet *packet)
{

    return 0;
}

int TcpServer_Show(TcpServer *server, SocketPtr client, Packet *packet)
{
    char data[64] = {0};
    memcpy(data, packet->data, 64);
    memset(packet->data, 0, 1024);
    snprintf((char*)packet->data, 256,
            "client:%s, data:%s",
            client->getName().c_str(),
            data);
    printf("server recv:%s\n", packet->data);
    client->sendMessage((unsigned char*)packet, sizeof(Packet));
    return 0;
}

int TcpClient_Show(TcpClient *client, Packet *packet)
{
    printf("client recv:%s\n", (char*)packet->data);
    return 0;
}
