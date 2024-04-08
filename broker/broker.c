#include "mqttBroker.h"

int main(int argc, char *argv[])
{
    // create the socket file descriptor with the ip, port and queue size
    int sockfd = createSocket(NULL, MY_PORT, QUEUESIZE);

    if (sockfd == -1)
    {
        return 1;
    }

    thrd_t server;
    int res;

    thrd_create(&server, handleServer, 0);

    thrd_t t[QUEUESIZE];

    while (close_server())
    {
        for (int i = 0; i < QUEUESIZE; i++) {
            thrd_create(t + i, handleRecv, &sockfd);
        }

        for (int i = 0; i < QUEUESIZE; i++)
        {
            int res;
            thrd_join(t[i], &res);
            if (res == -1) printf("error");
        }
    }

    thrd_join(server, &res);

    close(sockfd);

    return 0;
}