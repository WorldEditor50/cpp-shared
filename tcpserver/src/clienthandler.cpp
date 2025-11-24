#include "clienthandler.h"
#include <sys/socket.h>
#include <cstring>
#include <cstdio>
#include <string>
#include "packet.h"
#include "tcpclient.h"

int TcpClient_Show(TcpClient *client, Packet *packet)
{
    printf("client recv:%s\n", (char*)packet->data);
    return 0;
}
