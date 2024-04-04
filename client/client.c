#include "mqttClient.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int sockfd = connectSocket(MY_IP, MY_PORT, QUEUESIZE);

    //send(sockfd, "halo", 4, 0);

    char answer[20];
    char message[500];
    char anssession[50];

    for (;;)
    {
        printf("select: connect, publish, subscribe, exit\n");
        memset(answer, 0, 20);
        scanf("%s", answer);
        printf("You selected: %s\n", answer);
        memset(message, 0, 500);

        if (strcmp(answer, "connect") == 0)
        {
            readConnectPayload(sockfd, message);
            // send the publish message to the server
            send(sockfd, message, strlen(answer), 0);
        }
        else if (strcmp(answer, "publish") == 0)
        {
            // send the publish message to the server
            char *publishMessage = "PUBLISH";
            send(sockfd, publishMessage, strlen(publishMessage), 0);
            printf("publish message sent\n");
        }
        else if (strcmp(answer, "subscribe") == 0)
        {
            // send the subscribe message to the server
            char *subscribeMessage = "SUBSCRIBE";
            send(sockfd, subscribeMessage, strlen(subscribeMessage), 0);
            printf("subscribe message sent\n");
        }
        else if (strcmp(answer, "exit") == 0)
        {
            // send the exit message to the server
            char *exitMessage = "EXIT";
            //send(sockfd, exitMessage, strlen(exitMessage), 0);
            send(sockfd, "q", 1, 0);
            printf("exit message sent\n");
            break;
        }

        uint8_t a = 10;

        send(sockfd, &a, 1, 0);

        send(sockfd, answer, strlen(answer), 0);
    }

    close(sockfd);

    return 0;
}