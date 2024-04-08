#include "mqttClient.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
    char answer[20];
    char message[500];
    char anssession[50];
    char broker_ip[30];
    char connack[500];

    printf("Enter the broker IP: ");
    scanf("%s", broker_ip);

    printf("Connecting to the broker\n\n");

    int connectQos = createConnect(message);

    int sockfd = connectSocket(broker_ip, BROKER_PORT);

    send(sockfd, &message, 500, 0); 
    
    // receive connack
    recv(sockfd, &connack, 500, 0);

    printf("connack received\n");

    if(connack[1] == 0x00)
    {
        printf("Connection accepted\n");
    }
    else
    {
        printf("Connection refused\n");
        printf("Response code: %02X\n", (unsigned char)connack[1]);
        return 1;
    }
    for (size_t i = 0; i < 8*4; i++) // 8 bits * 4 bytes. 4 bytes is the size of the connack message
    {
        printf("%02X ", (unsigned char)connack[i]); // Cast char to unsigned char for correct output
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
            int publishQos = createPublish(message);
            send(sockfd, &message, 500, 0);
            printf("publish message sent\n");
            if(publishQos){
                recv(sockfd, &message, 500, 0);
            }
            
        }
        else if (strcmp(answer, "subscribe") == 0)
        {
            int subscribeqos = createSubscribe(message);
            send(sockfd, &message, 500, 0);
            printf("subscribe message sent\n");
            if (subscribeqos){
                recv(sockfd, &message, 500, 0);
            }
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