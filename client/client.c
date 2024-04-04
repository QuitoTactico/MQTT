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

        char a[100];
        a[0] = CONNECT;
        a[1] = 1;
        a[2] = 10;

        // VARIABLE HEADER
        a[3] = 1;
        a[4] = 4;
        a[5] = 'M';
        a[6] = 'Q';
        a[7] = 'T';
        a[8] = 'T';
        a[9] = 4;
        a[10] = 4;
        a[11] = 4;
        a[12] = 4;

        a[13] = 0;
        a[14] = 5;
        a[15] = 'a';
        a[16] = '1';
        a[17] = '3';
        a[18] = 'u';
        a[19] = '2';
        a[20] = 0;
        a[21] = 0;
        a[22] = 0;
        a[23] = 0;
        a[24] = 0;
        a[25] = 0;
        a[26] = 0;
        a[27] = 0;

        send(sockfd, &a, 100, 0);

        memset(a, 0, 100);

        send(sockfd, answer, strlen(answer), 0);
    }

    close(sockfd);

    return 0;
}