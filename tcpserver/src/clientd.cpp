#include <signal.h>
#include <unistd.h>
#include <cstring>
#include "tcpclient.h"
#include "packet.h"

static volatile sig_atomic_t isRunning = 1;

static void signalHandle(int sigNo)
{
    if (sigNo == SIGINT || sigNo == SIGTERM) {
        isRunning = 0;
    }
    return;
}

int main()
{
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = signalHandle;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);


    TcpClient client;
    int ret = client.start("127.0.0.1", 8090);
    if (ret != 0) {
        return -1;
    }
    Packet packet;
    packet.signature = 0xAF0E;
    packet.type = TcpMessage_Show;
    char buf[64] = "hello world!";
    packet.datasize = strlen(buf);
    memcpy(packet.data, buf, packet.datasize);
    while (isRunning) {
        client.sendMessage((unsigned char*)&packet, sizeof(Packet));
        usleep(2000000);
    }
    client.stop();
    return 0;
}
