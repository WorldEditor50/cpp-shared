#include <signal.h>
#include <unistd.h>
#include <cstring>
#include "tcpserver.h"
#include "packet.h"

static TcpServer server;

static void signalHandle(int sigNo)
{
    if (sigNo == SIGINT || sigNo == SIGTERM) {
        server.stop();
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

    server.start(8090, 8);
    return 0;
}
