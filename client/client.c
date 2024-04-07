#include "mqttClient.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
    char answer[20];
    char message[500];
    char anssession[50];
    char broker_ip[30];

    printf("Enter the broker IP: ");
    scanf("%s", broker_ip);

    printf("Connecting to the broker\n\n");

    createConnect(message);

    int sockfd = connectSocket(broker_ip, BROKER_PORT);

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
            createPublish(message);
            send(sockfd, &message, 500, 0);
            printf("publish message sent\n");
        }
        else if (strcmp(answer, "subscribe") == 0)
        {
            createSubscribe(message);
            send(sockfd, &message, 500, 0);
            printf("subscribe message sent\n");
        }
        else if (strcmp(answer, "exit") == 0)
        {
            send(sockfd, "q\0", 2, 0);
            send(sockfd, &message, 500, 0);  //si se comenta, el broker igual señala un "####### client gone #######"
            printf("exiting the server\n");
            break;
        }
    }

    //close(sockfd);

    return 0;
}