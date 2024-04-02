#include "mqttClient.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int sockfd = createSocket(MY_IP, MY_PORT, QUEUESIZE);

    char answer[20];
    char message[500];
    char anssession[50];

    for (;;)
    {
        printf("select: connect, publish, subscribe, exit\n");
        memset(answer, 0, 20);
        scanf("%s", answer);
        printf("You selected: %s\n", answer);

        if (strcmp(answer, "connect") == 0)
        {
            readConnectPayload(sockfd);
        }
        else if (strcmp(answer, "publish") == 0)
        {
            // send the publish message to the server
            char *publishMessage = "PUBLISH";
            //send(sockfd, publishMessage, strlen(publishMessage), 0);
            printf("publish message sent\n");
        }
        else if (strcmp(answer, "subscribe") == 0)
        {
            // send the subscribe message to the server
            char *subscribeMessage = "SUBSCRIBE";
            //send(sockfd, subscribeMessage, strlen(subscribeMessage), 0);
            printf("subscribe message sent\n");
        }
        else if (strcmp(answer, "exit") == 0)
        {
            // send the exit message to the server
            char *exitMessage = "EXIT";
            //send(sockfd, exitMessage, strlen(exitMessage), 0);
            printf("exit message sent\n");
            break;
        }
    }

    close(sockfd);

    return 0;
}