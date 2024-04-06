#include "mqttClient.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
    char answer[20];
    char message[500];
    char anssession[50];

    printf("Connect to the server\n\n");

    createConnectPayload(message);

    int sockfd = connectSocket(MY_IP, MY_PORT);

    send(sockfd, &message, 500, 0);

    for (;;)
    {
        printf("select: publish, subscribe, exit\n");
        memset(answer, 0, 20);
        scanf("%s", answer);
        printf("You selected: %s\n", answer);
        memset(message, 0, 500);

        if (strcmp(answer, "publish") == 0)
        {
            printf("publish message sent\n");
        }
        else if (strcmp(answer, "subscribe") == 0)
        {

            printf("subscribe message sent\n");
        }
        else if (strcmp(answer, "exit") == 0)
        {
            send(sockfd, "q\0", 2, 0);
            printf("exiting the server\n");
            break;
        }


        send(sockfd, answer, strlen(answer), 0);
    }

    close(sockfd);

    return 0;
}