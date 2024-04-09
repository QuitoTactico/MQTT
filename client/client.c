#include "mqttClient.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
    char answer[20];
    char message[500];
    char anssession[50];
    char broker_ip[30];
    char connack[4];
    char puback[4];
    char suback[4];

    printf("Enter the broker IP: ");
    scanf("%s", broker_ip);

    printf("Connecting to the broker\n\n");
    int sockfd;
    int connectAccepted = 0;
    while (connectAccepted != 1)
    {
        int connectQos = createConnect(message);

        sockfd = connectSocket(broker_ip, BROKER_PORT);

        send(sockfd, &message, 500, 0);

        // receive connack
        recv(sockfd, connack, 4, 0);

        connectAccepted = handleconnack(connack);

        if(connectAccepted == 0){
            close(sockfd);
        }
    }

    for (;;)
    {
        printf("select: publish, subscribe, exit\n");
        memset(answer, 0, 20);
        scanf("%s", answer);
        printf("You selected: %s\n", answer);
        memset(message, 0, 500);

        if (strcmp(answer, "publish") == 0)
        {
            Resultpub publishResponse = createPublish(message);
            send(sockfd, &message, 500, 0);
            printf("publish message sent\n");
            if (publishResponse.resQos)
            {
                recv(sockfd, puback, 4, 0);
                handlepuback(puback, publishResponse.id);
            }
        }
        else if (strcmp(answer, "subscribe") == 0)
        {
            Result subscribeqos = createSubscribe(message);
            send(sockfd, &message, 500, 0);
            printf("subscribe message sent\n");
            if (subscribeqos.resQos)
            {
                recv(sockfd, &message, 500, 0);
                handlesuback(suback, subscribeqos.counter, subscribeqos.id);
            }
        }
        else if (strcmp(answer, "exit") == 0)
        {
            send(sockfd, "q\0", 2, 0);
            send(sockfd, &message, 500, 0); // si se comenta, el broker igual señala un "####### client gone #######"
            printf("exiting the server\n");
            break;
        }
    }

    // close(sockfd);

    return 0;
}