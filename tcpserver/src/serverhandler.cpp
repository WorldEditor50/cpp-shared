#include "serverhandler.h"
#include <sys/socket.h>
#include <cstring>
#include <cstdio>
#include <string>
#include "packet.h"
#include "tcpserver.h"

int TcpServer_Forward(TcpServer *server, Packet *packet)
{

    return 0;
}

int TcpServer_Show(TcpServer *server, Packet *packet)
{
    SocketPtr client = server->getClient(packet->src);
    if (!client) {
        printf("server recv data, but can't find client.\n");
        return -1;
    }
    char data[64] = {0};
    memcpy(data, packet->data, 64);
    memset(packet->data, 0, 1024);
    snprintf((char*)packet->data, 256,
            "client:%s, data:%s",
            packet->src,
            data);
    printf("server recv:%s\n", packet->data);
    client->sendMessage((unsigned char*)packet, sizeof(Packet));
    return 0;
}

