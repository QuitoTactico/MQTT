#include "mqttClient.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
    char answer[20];
    unsigned char message[500];
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

        uint8_t a[100];

        // fixed header
        a[0] = CONNECT;
        a[1] = 1;
        a[2] = 10;

        // VARIABLE HEADER
        a[3] = 0;
        a[4] = 4;
        a[5] = 'M';
        a[6] = 'Q';
        a[7] = 'T';
        a[8] = 'T';
        // version
        a[9] = 4;
        // flags
        a[10] = 0;
        // keep alive
        a[11] = 4;
        a[12] = 4;

        // id
        a[13] = 0;
        a[14] = 5;
        a[15] = 'a';
        a[16] = '1';
        a[17] = '3';
        a[18] = 'u';
        a[19] = '2';
        // will top
        a[20] = 0;
        a[21] = 1;
        a[22] = 'n';
        // will mess
        a[23] = 0;
        a[24] = 2;
        a[25] = 'b';
        a[26] = 'c';
        // user
        a[27] = 0;
        a[28] = 1;
        a[29] = 'a';
        // pass
        a[30] = 0;
        a[31] = 1;
        a[31] = 'b';

        send(sockfd, &a, 100, 0);

        memset(a, 0, 100);

        send(sockfd, answer, strlen(answer), 0);
    }

    close(sockfd);

    return 0;
}