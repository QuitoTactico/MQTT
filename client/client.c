#include "mqttClient.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
    char message[500];
    int brokerSockfd;

    char broker_ip[30];
    printf("Enter the broker IP: ");
    scanf("%s", broker_ip);

    int connectAccepted = 0;
    while (connectAccepted != 1)
    {
        char connack[4];

        int connectQos = createConnect(message);

        brokerSockfd = connectSocket(broker_ip, BROKER_PORT);

        send(brokerSockfd, &message, 500, 0);

        // receive connack
        recv(brokerSockfd, connack, 4, 0);

        printf("Connecting to the broker\n\n");
        connectAccepted = handleConnack(connack);
    }

    thrd_t recvThread;

    thrd_create(&recvThread, handleRecv, &brokerSockfd);

    char answer[20];

    for (;;)
    {
        printf("select: publish, subscribe, exit\n");
        memset(answer, 0, 20);
        scanf("%s", answer);
        printf("You selected: %s\n", answer);
        memset(message, 0, 500);
        
        if (strcmp(answer, "publish") == 0)
        {
            createPublish(message);
            send(brokerSockfd, &message, 500, 0);
            printf("publish message sent\n");
        }
        else if (strcmp(answer, "subscribe") == 0)
        {
            createSubscribe(message);
            send(brokerSockfd, &message, 500, 0);
            printf("subscribe message sent\n");
        }
        else if (strcmp(answer, "exit") == 0)
        {
            send(brokerSockfd, "q\0", 2, 0);
            printf("exiting the server\n");
            break;
        }
    }

    int res;
    thrd_join(recvThread, &res);

    // close(brokerSockfd);

    return 0;
}